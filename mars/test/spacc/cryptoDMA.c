#include <stdio.h>
#include <stdlib.h>
#include <common.h>
#include <string.h>

#include "cache.h"
#include "mmio.h"
#include "system_common.h"
#include "timer.h"

#include "cryptoDMA/platform_def.h"
#include "cryptoDMA/cryptoDMA_internal.h"
#include "cryptoDMA/cryptoDMA.h"

#define DEBUG 0

static inline void setup_ctrl(uint32_t *dma_descriptor, cryptoDMA_ctrl_s *ctrl)
{
	uint32_t des_ctrl = 0;

	if (ctrl->key_src == CRYPTODMA_KEY_SOURCE_OTP) {
		des_ctrl |= (0x1 << 27);
	} else {
		des_ctrl |= DES_USE_DESCRIPTOR_KEY;
	}

	if (ctrl->is_end) {
		des_ctrl |= 0xF;
	} else {
		des_ctrl |= 0xD;
		dma_descriptor[0x2] =
			(uint32_t)(ctrl->next_descriptor & 0xFFFFFFFF);
		dma_descriptor[0x3] = (uint32_t)(ctrl->next_descriptor >> 32);
	}

	dma_descriptor[CRYPTODMA_CTRL] |= des_ctrl;
}

static inline void setup_mode(uint32_t *dma_descriptor,
			      CRYPTODMA_ALGO_MODE_E mode, unsigned char *iv)
{
	switch (mode) {
	case CRYPTODMA_ALGO_MODE_CBC:
		dma_descriptor[CRYPTODMA_CTRL] |= DES_USE_DESCRIPTOR_IV;
		dma_descriptor[CRYPTODMA_CIPHER] = 0x1 << 1;
		memcpy(&dma_descriptor[CRYPTODMA_IV], iv, 16);
		break;
	case CRYPTODMA_ALGO_MODE_CTR:
		dma_descriptor[CRYPTODMA_CTRL] |= DES_USE_DESCRIPTOR_IV;
		dma_descriptor[CRYPTODMA_CIPHER] = 0x1 << 2;
		memcpy(&dma_descriptor[CRYPTODMA_IV], iv, 16);
		break;
	case CRYPTODMA_ALGO_MODE_OFB:
		dma_descriptor[CRYPTODMA_CTRL] |= DES_USE_DESCRIPTOR_IV;
		dma_descriptor[CRYPTODMA_CIPHER] = 0x1 << 3;
		memcpy(&dma_descriptor[CRYPTODMA_IV], iv, 16);
		break;
	case CRYPTODMA_ALGO_MODE_ECB:
	default:
		break;
	}
}

static inline void setup_key_size(uint32_t *dma_descriptor,
				  CRYPTODMA_KEY_SIZE_E size, unsigned char *key)
{
	switch (size) {
	case CRYPTODMA_KEY_SIZE_64BITS:
		memcpy(&dma_descriptor[CRYPTODMA_KEY], key, 8);
		break;
	case CRYPTODMA_KEY_SIZE_128BITS:
		dma_descriptor[CRYPTODMA_CIPHER] |= (0x1 << 5);
		memcpy(&dma_descriptor[CRYPTODMA_KEY], key, 16);
		break;
	case CRYPTODMA_KEY_SIZE_192BITS:
		dma_descriptor[CRYPTODMA_CIPHER] |= (0x1 << 4);
		memcpy(&dma_descriptor[CRYPTODMA_KEY], key, 24);
		break;
	case CRYPTODMA_KEY_SIZE_256BITS:
		dma_descriptor[CRYPTODMA_CIPHER] |= (0x1 << 3);
		memcpy(&dma_descriptor[CRYPTODMA_KEY], key, 32);
		break;
	default:
		break;
	}
}

static inline void setup_src(uint32_t *dma_descriptor, uintptr_t src,
			     uint32_t len)
{
	flush_dcache_range((unsigned long)src, len);

	dma_descriptor[CRYPTODMA_SRC_ADDR_L] =
		(uint32_t)((uint64_t)src & 0xFFFFFFFF);
	dma_descriptor[CRYPTODMA_SRC_ADDR_H] = (uint32_t)((uint64_t)src >> 32);

	dma_descriptor[CRYPTODMA_SRC_LEN] = len;
}

// some algo like sha256/sha1/sm3 needn't dst
#define setup_dst(dst)                                                         \
	do {                                                                   \
		dma_descriptor[CRYPTODMA_DST_ADDR_L] =                         \
			(uint32_t)((uint64_t)dst & 0xFFFFFFFF);                \
		dma_descriptor[CRYPTODMA_DST_ADDR_H] =                         \
			(uint32_t)((uint64_t)dst >> 32);                       \
	} while (0)

static inline void setup_src_and_dst(uint32_t *dma_descriptor, uintptr_t src,
				     uint32_t len, uintptr_t dst)
{
	setup_src(dma_descriptor, src, len);
	setup_dst(dst);
}

static inline void setup_action(uint32_t *dma_descriptor,
				CRYPTODMA_ACTION_E action)
{
	if (action == CRYPTODMA_ACTION_ENCRYPTION) {
		dma_descriptor[CRYPTODMA_CIPHER] |= 0x1;
	}
}

int trigger_cryptodma_work_and_wait_finish(uint32_t *dma_descriptor,
					   int irq_enable)
{
	uint32_t data;

	// set exec dma descriptor addr
	mmio_write_32(SEC_CRYPTODMA_BASE + CRYPTODMA_DES_BASE_L,
		      (uint32_t)((uint64_t)dma_descriptor & 0xFFFFFFFF));
	mmio_write_32(SEC_CRYPTODMA_BASE + CRYPTODMA_DES_BASE_H,
		      (uint32_t)((uint64_t)dma_descriptor >> 32));

	// Set cryptodma dma control
	//mmio_write_32(SEC_CRYPTODMA_BASE + CRYPTODMA_INT_MASK, 0x7);
	mmio_write_32(SEC_CRYPTODMA_BASE + CRYPTODMA_INT_MASK, 0x0);

	// Clear interrupt
	mmio_write_32(SEC_CRYPTODMA_BASE + CRYPTODMA_WR_INT, 0x7);

	// Trigger cryptodma engine
	mmio_write_32(SEC_CRYPTODMA_BASE + CRYPTODMA_DMA_CTRL,
		      DMA_WRITE_MAX_BURST << 24 | DMA_READ_MAX_BURST << 16 |
			      DMA_DESCRIPTOR_MODE << 1 | DMA_ENABLE);

	do {
		data = mmio_read_32(SEC_CRYPTODMA_BASE + CRYPTODMA_WR_INT);
#if DEBUG
		NOTICE("INT status 0x%x\n", data);
#endif
	} while (data == 0);

	return 0;
}

int plat_cryptodma_sha(uintptr_t src, uint32_t len,
		       cryptoDMA_sha_config_s *config)
{
	u64 timer_meter_start_cnt = timer_get_tick();
	__attribute__((aligned(64))) uint32_t dma_descriptor[32] = { 0 };
	uint32_t i;
	uint32_t result_size = 5;

	// must mark DES_USE_DESCRIPTOR_KEY flag
	dma_descriptor[CRYPTODMA_CTRL] =
		DES_USE_DESCRIPTOR_KEY | DES_USE_SHA | 0xF;
	if (config->algo == CRYPTODMA_SHA_SHA256) {
		dma_descriptor[CRYPTODMA_CIPHER] = (0x1 << 1);
		result_size = 8;
	}

	dma_descriptor[CRYPTODMA_CIPHER] |= 0x1;

	for (i = 0; i < result_size; i++) {
		dma_descriptor[CRYPTODMA_KEY + i] = config->state[i];
	}

	setup_src(dma_descriptor, src, len);
	flush_dcache_range((unsigned long)dma_descriptor,
			   32 * sizeof(uint32_t));

	if (trigger_cryptodma_work_and_wait_finish(dma_descriptor, 0)) {
		return -1;
	}

	for (i = 0; i < result_size; i++) {
		config->state[i] = mmio_read_32(SEC_CRYPTODMA_BASE +
						CRYPTODMA_SHA_PARA + i * 4);
	}

	timer_meter_start_cnt =
		((timer_get_tick() - timer_meter_start_cnt) / TIMER_CNT_US);
	NOTICE("Duaring : %ld\n", timer_meter_start_cnt);

	return 0;
}

int plat_cryptodma_sm3(uintptr_t src, uint32_t len,
		       cryptoDMA_sm3_config_s *config)
{
	u64 timer_meter_start_cnt = timer_get_tick();
	__attribute__((aligned(64))) uint32_t dma_descriptor[32] = { 0 };
	uint32_t i;

	// must mark DES_USE_DESCRIPTOR_KEY flag
	dma_descriptor[CRYPTODMA_CTRL] =
		DES_USE_DESCRIPTOR_KEY | DES_USE_SM3 | 0xF;
	dma_descriptor[CRYPTODMA_CIPHER] = 0x1;

	for (i = 0; i < 8; i++) {
		dma_descriptor[CRYPTODMA_KEY + i] = config->state[i];
	}

	setup_src(dma_descriptor, src, len);
	flush_dcache_range((unsigned long)dma_descriptor,
			   32 * sizeof(uint32_t));

	if (trigger_cryptodma_work_and_wait_finish(dma_descriptor, 0)) {
		return -1;
	}

	for (i = 0; i < 8; i++) {
		config->state[i] = mmio_read_32(SEC_CRYPTODMA_BASE +
						CRYPTODMA_SM3_PARA + i * 4);
	}

	timer_meter_start_cnt =
		((timer_get_tick() - timer_meter_start_cnt) / TIMER_CNT_US);
	NOTICE("Duaring : %ld\n", timer_meter_start_cnt);

	return 0;
}

int plat_cryptodma_base64(uintptr_t src, uint32_t len, uint32_t customer_code,
			  CRYPTODMA_ACTION_E ation)
{
	__attribute__((aligned(64))) uint32_t dma_descriptor[32] = { 0 };
	int result_len;

	dma_descriptor[CRYPTODMA_CTRL] = DES_USE_BASE64 | 0xF;
	if (ation == CRYPTODMA_ACTION_ENCRYPTION) {
		dma_descriptor[CRYPTODMA_CIPHER] = 0x1;
		result_len = (len + (3 - 1)) / 3 * 4;
	} else {
		result_len = (len / 4) * 3;
	}
	dma_descriptor[CRYPTODMA_DST_LEN] = result_len;

	setup_src(dma_descriptor, src, len);
	setup_dst(src);
	flush_dcache_range((unsigned long)dma_descriptor,
			   32 * sizeof(uint32_t));

	if (0 != customer_code) {
		mmio_write_32(SEC_CRYPTODMA_BASE +
				      CRYPTODMA_BASE64_CUSTOMIZE_CODE,
			      customer_code);
	}

	if (trigger_cryptodma_work_and_wait_finish(dma_descriptor, 0)) {
		return -1;
	}

	invalidate_dcache_range(src, result_len);
	return 0;
}

//test access speed
int plat_cryptodma_bypass(uintptr_t src, uint32_t len, uintptr_t dst)
{
	__attribute__((aligned(64))) uint32_t dma_descriptor[32] = { 0 };

	dma_descriptor[CRYPTODMA_CTRL] = DES_USE_BYPASS | 0xF;

	setup_src_and_dst(dma_descriptor, src, len, dst);
	flush_dcache_range((unsigned long)dma_descriptor,
			   32 * sizeof(uint32_t));

	if (trigger_cryptodma_work_and_wait_finish(dma_descriptor, 0)) {
		return -1;
	}

	invalidate_dcache_range(dst, len);
	return 0;
}

int plat_cryptodma_sm4(uintptr_t src, uint32_t len,
		       cryptoDMA_sm4_config_s *config)
{
	__attribute__((aligned(64))) uint32_t dma_descriptor[32] = { 0 };

	dma_descriptor[CRYPTODMA_CTRL] =
		DES_USE_DESCRIPTOR_KEY | DES_USE_SM4 | 0xF;

	setup_mode(dma_descriptor, config->mode, config->iv);
	setup_key_size(dma_descriptor, config->size, config->key);
	setup_action(dma_descriptor, config->action);

	setup_src_and_dst(dma_descriptor, src, len, src);
	flush_dcache_range((unsigned long)dma_descriptor,
			   32 * sizeof(uint32_t));

	if (trigger_cryptodma_work_and_wait_finish(dma_descriptor, 0)) {
		return -1;
	}

	invalidate_dcache_range(src, len);
	return 0;
}

int plat_cryptodma_aes(uintptr_t src, uint32_t len,
		       cryptoDMA_aes_config_s *config)
{
	u64 timer_meter_start_cnt = timer_get_tick();
	__attribute__((aligned(64))) uint32_t dma_descriptor[32] = { 0 };

	dma_descriptor[CRYPTODMA_CTRL] =
		DES_USE_DESCRIPTOR_KEY | DES_USE_AES | 0xF;

	setup_mode(dma_descriptor, config->mode, config->iv);
	setup_key_size(dma_descriptor, config->size, config->key);
	setup_action(dma_descriptor, config->action);
	setup_src(dma_descriptor, src, len);
	setup_dst(src);
	flush_dcache_range((unsigned long)dma_descriptor,
			   32 * sizeof(uint32_t));

	if (trigger_cryptodma_work_and_wait_finish(dma_descriptor, 0)) {
		//puts("-\n");
		return -1;
	}

	invalidate_dcache_range(src, len);

	timer_meter_start_cnt =
		((timer_get_tick() - timer_meter_start_cnt) / TIMER_CNT_US);
	NOTICE("Duaring : %ld\n", timer_meter_start_cnt);

	return 0;
}

uint32_t *plat_cryptodma_create_aes_descriptor(uintptr_t src, uint32_t len,
					       cryptoDMA_aes_config_s *config,
					       cryptoDMA_ctrl_s *ctrl)
{
	uint32_t *dma_descriptor = NULL;

	dma_descriptor = (uint32_t *)calloc(sizeof(uint32_t), 32);
	if (NULL == dma_descriptor) {
		printf("calloc dma descriptor failed\n");
		return NULL;
	}

	printf("dma_descriptor : %p\n", dma_descriptor);
	dma_descriptor[CRYPTODMA_CTRL] = DES_USE_DESCRIPTOR_KEY | DES_USE_AES;

	setup_ctrl(dma_descriptor, ctrl);
	setup_mode(dma_descriptor, config->mode, config->iv);
	setup_key_size(dma_descriptor, config->size, config->key);
	setup_action(dma_descriptor, config->action);
	setup_src_and_dst(dma_descriptor, src, len, src);

	flush_dcache_range((unsigned long)dma_descriptor,
			   32 * sizeof(uint32_t));

	return dma_descriptor;
}
