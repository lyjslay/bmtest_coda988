#include <stdint.h>
#include <common.h>
#include <stdlib.h>
#include <string.h>
#include "boot_test.h"
#include "uart.h"
#include "testcase_spacc.h"
#include "cli.h"
#include "utils_def.h"
#include "command.h"

#include "mmio.h"
#include "system_common.h"

#include "cache.h"
#include "efuse.h"
#include "data/1KB.h"
#include "data/10KB.h"
#include "data/10KB_hash.h"
#include "data/100KB.h"
#include "data/1MB.h"
#include "data/16B.h"
#include "cryptoDMA/cryptoDMA.h"

#define DEBUG 0

static int is_irq_enable = 0;
static int is_c906irq_enable = 0;
static int irq_done = 0;

unsigned char key[32] = { 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61,
			  0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61,
			  0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68,
			  0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61 };
unsigned char iv[16] = { 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61,
			 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61 };
unsigned char gdn_iv_out[16] = { 0 };
extern void icache_enable(void);
extern void dcache_enable(void);
static inline void dump_data(unsigned char *data, size_t size)
{
	int i;

	for (i = 0; i < size; i++) {
		uartlog("%x ", data[i]);
	}

	uartlog("\n");
}

// static void print_src_key_iv(unsigned char *src, size_t size,
// 			     unsigned char *key, size_t key_size,
// 			     unsigned char *iv, size_t iv_size)
// {
// 	printf("src :");
// 	dump_data(src, size);
// 	printf("key :");
// 	dump_data(key, key_size);
// 	if (iv_size) {
// 		printf("iv :");
// 		dump_data(iv, iv_size);
// 	}
// }

static void print_src_key_iv(unsigned char *result, size_t size)
{
	printf("result :");
	dump_data(result, size);
	printf("----------------------------------------------------\n");
}

static int irq_handler(int irqn, void *priv)
{
	uartlog("INT: recv interrupt\n");
	mdelay(20);
	irq_done = 1;
	mmio_write_32(SPACC_BASE + CRYPTODMA_WR_INT, 0x7);

	return 0;
}

void write_efuse_key()
{
	// uint32_t *data; cutmoster key1
	cvi_efuse_power_on();
	cvi_efuse_program_word(54, 0x61);
	cvi_efuse_power_on();
	cvi_efuse_program_word(55, 0x61);
	cvi_efuse_power_on();
	cvi_efuse_program_word(56, 0x61);
	cvi_efuse_power_on();
	cvi_efuse_program_word(57, 0x61);

	cvi_efuse_power_on();
	cvi_efuse_program_word(58, 0x61);
	cvi_efuse_power_on();
	cvi_efuse_program_word(59, 0x61);
	cvi_efuse_power_on();
	cvi_efuse_program_word(60, 0x61);
	cvi_efuse_power_on();
	cvi_efuse_program_word(61, 0x61);

	test_get_sec_shadow();

	printf("SECURE_KEY_VALID:%x\n",
	       mmio_read_32(GetSpaccAddr(CRYPTODMA_SECURE_KEY_VALID)));
}

int check_result(const unsigned char *src, const unsigned char *output, size_t src_len) {
    if (memcmp(src, output, src_len) != 0) {
        printf("Encryption and decryption results do not match.\n");
        return -1;
    }

    return 0;
}
int plat_cryptodma_do(int isEncrypt, uintptr_t in, uintptr_t out, uint64_t len,
		      unsigned char *key, E_KEY_MODE keyMode, unsigned char *iv,
		      E_ALGO a, E_MODE b, uint32_t *state)
{
	uint32_t data;
	int i;
	uint32_t des_ctrl = 0;
	__attribute__((aligned(64))) uint32_t dma_descriptor[32] = { 0 };
	uint64_t src = (uintptr_t)in;
	uint64_t des = (uintptr_t)out;


#if DEBUG
	NOTICE("plat_cryptodma, src=%p, des=%p, image_size=0x%x\n", (void *)in,
	       (void *)out, (int)len);
#endif

	if (is_irq_enable && !is_c906irq_enable) {
		irq_done = 0;
		printf("a53 request_irq\n");
		request_irq(SPACC_IRQ, irq_handler, 0, "SPACC_IRQ int", NULL);
	}

	u64 timer_meter_start_cnt = timer_get_tick();

	switch (a) {
	case AES: {
		des_ctrl = DES_USE_DESCRIPTOR_IV | DES_USE_AES | 0xF |
			   DES_USE_DESCRIPTOR_KEY;
		dma_descriptor[CRYPTODMA_CTRL] = des_ctrl;
		switch (b) {
		case ECB:
			break;
		case CBC:
			dma_descriptor[CRYPTODMA_CIPHER] = CBC_ENABLE << 1;
			break;
		case CTR:
			dma_descriptor[CRYPTODMA_CIPHER] = 0x1 << 2;
			break;
		default:
			break;
		}

		switch (keyMode) {
		case KEY_128BITS:
			dma_descriptor[CRYPTODMA_CIPHER] |= (0x4 << 3);
			break;
		case KEY_192BITS:
			dma_descriptor[CRYPTODMA_CIPHER] |= (0x2 << 3);
			break;
		case KEY_256BITS:
			dma_descriptor[CRYPTODMA_CIPHER] |= (0x1 << 3);
			break;
		default:
			break;
		}
		break;
	}
	case SM3: {
		dma_descriptor[CRYPTODMA_CTRL] = DES_USE_DESCRIPTOR_IV |
						 DES_USE_DESCRIPTOR_KEY |
						 DES_USE_SM3 | 0xF;
		dma_descriptor[CRYPTODMA_CIPHER] = 0x1;
		break;
	}
	case SM4: {
		dma_descriptor[CRYPTODMA_CTRL] = DES_USE_DESCRIPTOR_IV |
						 DES_USE_DESCRIPTOR_KEY |
						 DES_USE_SM4 | 0xF;
		switch (b) {
		case ECB:
			break;
		case CBC:
			dma_descriptor[CRYPTODMA_CIPHER] = CBC_ENABLE << 1;
			break;
		case CTR:
			dma_descriptor[CRYPTODMA_CIPHER] = 0x1 << 2;
			break;
		case OFB:
			dma_descriptor[CRYPTODMA_CIPHER] = 0x1 << 3;
			break;
		default:
			break;
		}
		break;
	}
	case BYPASS: {
		dma_descriptor[CRYPTODMA_CTRL] = DES_USE_BYPASS | 0xF;
		break;
	}
	case SHA256:
		dma_descriptor[CRYPTODMA_CIPHER] = (0x1 << 1);
	case SHA1: {
		dma_descriptor[CRYPTODMA_CTRL] = DES_USE_DESCRIPTOR_KEY |
						 DES_USE_DESCRIPTOR_IV |
						 DES_USE_SHA | 0xF;
		dma_descriptor[CRYPTODMA_CIPHER] |= 0x1;
		break;
	}
	case BASE64_CUSTOMER:
	case BASE64: {
		dma_descriptor[CRYPTODMA_CTRL] = DES_USE_BASE64 | 0xF;
		if (isEncrypt) {
			dma_descriptor[BASE64_SIZE] = (len + (3 - 1)) / 3 * 4;
		} else {
			dma_descriptor[BASE64_SIZE] = (len / 4) * 3;
		}
		break;
	}
	case TDES:
		dma_descriptor[CRYPTODMA_CIPHER] = (0x1 << 3);
	case DES: {
		dma_descriptor[CRYPTODMA_CTRL] = DES_USE_DESCRIPTOR_IV |
						 DES_USE_DESCRIPTOR_KEY |
						 DES_USE_DES | 0xF;
		switch (b) {
		case ECB:
			break;
		case CBC:
			dma_descriptor[CRYPTODMA_CIPHER] |= (CBC_ENABLE << 1);
			break;
		case CTR:
			dma_descriptor[CRYPTODMA_CIPHER] |= (0x1 << 2);
			break;
		default:
			break;
		}

		break;
	}
	default:
		break;
	}

	if (isEncrypt)
		dma_descriptor[CRYPTODMA_CIPHER] |= 0x1;

	dma_descriptor[CRYPTODMA_SRC_ADDR_L] =
		(uint32_t)((uint64_t)src & 0xFFFFFFFF);
	dma_descriptor[CRYPTODMA_SRC_ADDR_H] = (uint32_t)((uint64_t)src >> 32);

	if (a != SHA256 && a != SHA1) {
		dma_descriptor[CRYPTODMA_DST_ADDR_L] =
			(uint32_t)((uint64_t)des & 0xFFFFFFFF);
		dma_descriptor[CRYPTODMA_DST_ADDR_H] =
			(uint32_t)((uint64_t)des >> 32);
	}

	dma_descriptor[CRYPTODMA_DATA_AMOUNT_L] =
		(uint32_t)((uint64_t)len & 0xFFFFFFFF);
	dma_descriptor[CRYPTODMA_DATA_AMOUNT_H] =
		(uint32_t)((uint64_t)len >> 32);

	if (a == SHA256) {
		// Clear SHA output first
		for (i = 0; i < 8; i++)
			mmio_write_32(SPACC_BASE + CRYPTODMA_SHA_PARA + i * 4,
				      0x12345678);

		for (i = 0; i < 8; i++)
			dma_descriptor[CRYPTODMA_KEY + i] = state[i];
	} else if (a == SM3) {
		for (i = 0; i < 8; i++)
			mmio_write_32(SPACC_BASE + CRYPTODMA_SM3_PARA + i * 4,
				      0x12345678);

		for (i = 0; i < 8; i++)
			dma_descriptor[CRYPTODMA_KEY + i] = state[i];
	} else if (a == SHA1) {
		// Clear SHA output first
		for (i = 0; i < 5; i++)
			mmio_write_32(SPACC_BASE + CRYPTODMA_SHA_PARA + i * 4,
				      0x12345678);

		for (i = 0; i < 5; i++)
			dma_descriptor[CRYPTODMA_KEY + i] = state[i];
	} else if ((a != BASE64) && (a != BASE64_CUSTOMER)) {
		switch (keyMode) {
		case KEY_128BITS:
			memcpy(&dma_descriptor[CRYPTODMA_KEY], key, 16);
			break;
		case KEY_192BITS:
			memcpy(&dma_descriptor[CRYPTODMA_KEY], key, 24);
			break;
		case KEY_256BITS:
			memcpy(&dma_descriptor[CRYPTODMA_KEY], key, 32);
			break;
		default:
			break;
		}

		memcpy(&dma_descriptor[CRYPTODMA_IV], iv, 16);
#if DEBUG
		for (i = 0; i < 16; i++) {
			NOTICE("key[%d] %x\n", i, key[i]);
			NOTICE("iv[%d] %x\n", i, iv[i]);
		}
#endif
	}
	if (is_irq_enable) {
		mmio_write_32(SPACC_BASE + CRYPTODMA_INT_MASK, 0x7);
	} else {
		mmio_write_32(SPACC_BASE + CRYPTODMA_INT_MASK, 0x0);
	}

	// Set cryptodma dma control
	mmio_write_32(SPACC_BASE + CRYPTODMA_DES_BASE_L,
		      (uint32_t)((uint64_t)dma_descriptor & 0xFFFFFFFF));
	mmio_write_32(SPACC_BASE + CRYPTODMA_DES_BASE_H,
		      (uint32_t)((uint64_t)dma_descriptor >> 32));

	flush_dcache_range((unsigned long)dma_descriptor,
			   sizeof(dma_descriptor));
	flush_dcache_range((unsigned long)src, len);

#if 0
	for (i = 0; i < 22; i++) {
		NOTICE("dma_descriptor[%d] = %x\n", i, dma_descriptor[i]);
	}
	INFO("dma_descriptor address: %p\n", dma_descriptor);
#endif

	// Clear interrupt
	// mmio_write_32(SPACC_BASE + CRYPTODMA_WR_INT, 0x7);

	if (a == BASE64_CUSTOMER) {
		uint32_t value = 0;
		// '/'->'!';'+'->'#'.'+'=0x2b '/'=0x2f
		value = ('!' << 8) | '#';
		mmio_write_32(SPACC_BASE + CRYPTODMA_BASE64_CUSTOMIZE_CODE,
			      value);
	}
	// Trigger cryptodma engine
	mmio_write_32(SPACC_BASE + CRYPTODMA_DMA_CTRL,
		      DMA_WRITE_MAX_BURST << 24 | DMA_READ_MAX_BURST << 16 |
			      DMA_DESCRIPTOR_MODE << 1 | DMA_ENABLE);

	// NOTICE("SPAAC DMA_CTRL: 0x%x\n",
	//        mmio_read_32(SPACC_BASE + CRYPTODMA_DMA_CTRL));
	
	
	if (is_irq_enable) {
		while (irq_done == 0) {
			mdelay(1);
		}
	} else {
		do {
			data = mmio_read_32(SPACC_BASE + CRYPTODMA_WR_INT);
#if DEBUG
			NOTICE("INT status 0x%x\n", data);
#endif
		} while (data == 0);
	}

	//#if DEBUG
	timer_meter_start_cnt =
		((timer_get_tick() - timer_meter_start_cnt) / TIMER_CNT_US);
	printf("Duaring : %ld us, %ld KBps\n", timer_meter_start_cnt,
	       ((1000000 / timer_meter_start_cnt) * len) / 1024);
	//#endif

	if (a == SHA256) {
		for (i = 0; i < 8; i++) {
			uintptr_t a = SPACC_BASE + CRYPTODMA_SHA_PARA + i * 4;
			state[i] = mmio_read_32(a);
			printf("i : %x, 0x%x \n", i, state[i]);
		}
	} else if (a == SHA1) {
		for (i = 0; i < 5; i++) {
			uintptr_t a = SPACC_BASE + CRYPTODMA_SHA_PARA + i * 4;
			state[i] = mmio_read_32(a);
			printf("i : %x, 0x%x \n", i, state[i]);
		}
	} else if (a == SM3) {
		for (i = 0; i < 8; i++) {
			uintptr_t a = SPACC_BASE + CRYPTODMA_SM3_PARA + i * 4;
			state[i] = mmio_read_32(a);
			printf("i : %x, 0x%x \n", i, state[i]);
		}
	} else {
		invalidate_dcache_range(des, len);
	}

	return 0;
}
void testcase_bypass_dram_to_dram(void)
{
	//DDR addr
	uint8_t *src = (uint8_t *)0x8f000000;
	uint8_t *result = (uint8_t *)0x8f010000;
	memcpy(src, "hello", 5);

	uartlog("%s\n", __func__);

	u64 timer_meter_start_cnt = timer_get_tick();

	plat_cryptodma_bypass((uintptr_t)src, 10 * 1024, (uintptr_t)result);
	plat_cryptodma_do(1, (uintptr_t)src, (uintptr_t)result, 10 * 1024, key,
			  KEY_ZERO, iv, BYPASS, XXX, NULL);

	NOTICE("Duaring : %ld\n",
	       ((timer_get_tick() - timer_meter_start_cnt) / TIMER_CNT_US));

	if (memcmp(src, result, 16)) {
		printf("%s failed\n", __func__);
		dump_data(result, 16);
	}
}

void testcase_bypass_tpu_sram_to_tpu_sram(void)
{
	//TPUSYS_TPU0_REGMEM_REGION0
	uint8_t *src = (uint8_t *)0x0C083000;
	uint8_t *result = (uint8_t *)0x0C085000;

	memcpy(src, "hello", 5);

	uartlog("%s\n", __func__);

	//plat_cryptodma_do(1, (uintptr_t)src, (uintptr_t)result, 10*1024, key, KEY_ZERO, iv, BYPASS, XXX, NULL);

	u64 timer_meter_start_cnt = timer_get_tick();

	plat_cryptodma_bypass((uintptr_t)src, 10 * 1024, (uintptr_t)result);

	NOTICE("Duaring : %ld\n",
	       ((timer_get_tick() - timer_meter_start_cnt) / TIMER_CNT_US));

	if (memcmp(src, result, 16)) {
		printf("%s failed\n", __func__);
		dump_data(result, 16);
	}
}

static uint8_t is_little_endian()
{
	static uint32_t _endian_x_ = 1;
	return ((const uint8_t *)(&_endian_x_))[0];
}

static uint8_t is_big_endian()
{
	return !is_little_endian();
}
//reverse byte order
static uint32_t reverse_32bit(uint32_t data)
{
	data = (data >> 16) | (data << 16);
	return ((data & 0xff00ff00UL) >> 8) | ((data & 0x00ff00ffUL) << 8);
}

//host byte order to big endian
static uint32_t os_htobe32(uint32_t data)
{
	if (is_big_endian()) {
		return data;
	}
	return reverse_32bit(data);
}

static inline uint64_t reverse_64bit(uint64_t data)
{
	data = (data >> 32) | (data << 32);
	data = ((data & 0xff00ff00ff00ff00ULL) >> 8) |
	       ((data & 0x00ff00ff00ff00ffULL) << 8);

	return ((data & 0xffff0000ffff0000ULL) >> 16) |
	       ((data & 0x0000ffff0000ffffULL) << 16);
}

//host to big endian
static uint64_t os_htobe64(uint64_t data)
{
	if (is_big_endian()) {
		return data;
	}

	return reverse_64bit(data);
}

int testcase_spacc_sha256(void)
{
	__aligned(64) uint8_t data[64] = { 0 };
	uint32_t state[8];

	/* expect result
    i : 0, 0xba4df22c 
    i : 1, 0xea3b05f 
    i : 2, 0x2a3be826 
    i : 3, 0x9ee2b9c5 
    i : 4, 0x5c1e161b 
    i : 5, 0x5e42a71f 
    i : 6, 0x62330473 
    i : 7, 0x24988b93
  */
	state[0] = os_htobe32(0x6A09E667);
	state[1] = os_htobe32(0xBB67AE85);
	state[2] = os_htobe32(0x3C6EF372);
	state[3] = os_htobe32(0xA54FF53A);
	state[4] = os_htobe32(0x510E527F);
	state[5] = os_htobe32(0x9B05688C);
	state[6] = os_htobe32(0x1F83D9AB);
	state[7] = os_htobe32(0x5BE0CD19);

	memcpy(data, "hello", 5);
	data[5] = 0x80;

	uint64_t bits_len_be = os_htobe64(40);
	memcpy(&data[56], &bits_len_be, 8);

	printf("bits_len_be: %lx\n", bits_len_be);

	printf("SHA256\n");
	plat_cryptodma_do(1, (uintptr_t)data, (uintptr_t)data, 64, key,
			  KEY_ZERO, iv, SHA256, XXX, state);

	cryptoDMA_sha_config_s config = {
		.algo = CRYPTODMA_SHA_SHA256,
	};

	config.state[0] = os_htobe32(0x6A09E667);
	config.state[1] = os_htobe32(0xBB67AE85);
	config.state[2] = os_htobe32(0x3C6EF372);
	config.state[3] = os_htobe32(0xA54FF53A);
	config.state[4] = os_htobe32(0x510E527F);
	config.state[5] = os_htobe32(0x9B05688C);
	config.state[6] = os_htobe32(0x1F83D9AB);
	config.state[7] = os_htobe32(0x5BE0CD19);

	// plat_cryptodma_sha((uintptr_t)__10KB_hash_bin, __10KB_hash_bin_len, &config);

	dump_data((unsigned char *)config.state, 32);

	return 0;
}

int testcase_spacc_sha1(void)
{
	__aligned(64) uint8_t data[64] = { 0 };

	uint32_t state[5];

	/*
    i : 0, 0x1dc6f4aa 
    i : 1, 0xa2e8c5dc 
    i : 2, 0xfdebeda 
    i : 3, 0xd92c483b 
    i : 4, 0x4d43a9ae 
  */
	state[0] = os_htobe32(0x67452301);
	state[1] = os_htobe32(0xEFCDAB89);
	state[2] = os_htobe32(0x98BADCFE);
	state[3] = os_htobe32(0x10325476);
	state[4] = os_htobe32(0xC3D2E1F0);

	memcpy(data, "hello", 5);
	data[5] = 0x80;

	uint64_t bits_len_be = os_htobe64(40);
	memcpy(&data[56], &bits_len_be, 8);

	printf("SHA1\n");
	plat_cryptodma_do(1, (uintptr_t)data, (uintptr_t)data, 64, key,
			  KEY_ZERO, iv, SHA1, XXX, state);

	cryptoDMA_sha_config_s config = {
		.algo = CRYPTODMA_SHA_SHA1,
	};

	config.state[0] = os_htobe32(0x67452301);
	config.state[1] = os_htobe32(0xEFCDAB89);
	config.state[2] = os_htobe32(0x98BADCFE);
	config.state[3] = os_htobe32(0x10325476);
	config.state[4] = os_htobe32(0xC3D2E1F0);

	//plat_cryptodma_sha((uintptr_t)__10KB_hash_bin, __10KB_hash_bin_len, &config);

	dump_data((unsigned char *)config.state, 20);

	return 0;
}

int testcase_spacc_sm3(void)
{
	__aligned(64) uint8_t data[64] = { 0 };

	uint32_t state[8];
#if 0
  unsigned char except_result[] = {
    0xBE,0xCB,0xBF,0xAA,0xE6,0x54,0x8B,0x8B,
    0xF0,0xCF,0xCA,0xD5,0xA2,0x71,0x83,0xCD,
    0x1B,0xE6,0x09,0x3B,0x1C,0xCE,0xCC,0xC3,
    0x03,0xD9,0xC6,0x1D,0x0A,0x64,0x52,0x68
  };
#endif
	state[0] = 0x7380166f;
	state[1] = 0x4914b2b9;
	state[2] = 0x172442d7;
	state[3] = 0xda8a0600;
	state[4] = 0xa96f30bc;
	state[5] = 0x163138aa;
	state[6] = 0xe38dee4d;
	state[7] = 0xb0fb0e4e;

	memcpy(data, "hello", 5);
	data[5] = 0x80;

	uint64_t bits_len_be = os_htobe64(40);
	memcpy(&data[56], &bits_len_be, 8);

	printf("SM3\n");
	plat_cryptodma_do(1, (uintptr_t)data, (uintptr_t)data, 64, key,
			  KEY_ZERO, iv, SM3, XXX, state);

	cryptoDMA_sm3_config_s config = { 0 };

	config.state[0] = 0x7380166f;
	config.state[1] = 0x4914b2b9;
	config.state[2] = 0x172442d7;
	config.state[3] = 0xda8a0600;
	config.state[4] = 0xa96f30bc;
	config.state[5] = 0x163138aa;
	config.state[6] = 0xe38dee4d;
	config.state[7] = 0xb0fb0e4e;

	// plat_cryptodma_sm3((uintptr_t)__10KB_hash_bin, __10KB_hash_bin_len, &config);

	// dump_data((unsigned char *)config.state, 32);

	return 0;
}

int testcase_base64_normal(void)
{
	int ret = 0;

	// __attribute__((aligned(64))) uint8_t src[64] = { 0 };
	//__attribute__((aligned(64))) uint8_t result[64] = {0};

	uint8_t *src = (uint8_t *)0x8a800000;
	size_t src_len = 3*1024*1024; //1*1024*1024;
	size_t result_len = (src_len + 2) / 3 * 4;
#if 1	
	int i;

	//memset(src, 1, 1*1024*1024);
	for (i = 0; i < src_len; i += 5) {
		memcpy((char *)(src + i), "hello", 5);
	}
  printf("memcpy done\n");
#else
	memcpy(src, "hello", 5);
#endif

	printf("BASE64 encode\n");
	// printf("src: %s\n", ((char *)src));
	//plat_cryptodma_base64((uintptr_t)src, 5, 0, CRYPTODMA_ACTION_ENCRYPTION);
	plat_cryptodma_do(1, (uintptr_t)src, (uintptr_t)src, src_len, key,
			  KEY_ZERO, iv, BASE64, XXX, NULL);
	// src[result_len] = 0;
	// printf("result: %s\n", ((char *)src));

	//for (i = 0; i < 1*1024*1024; i++) {
	//	uartlog("%d\n", *(src + i));
	//}

	//memcpy((char *)src, "aGVsbG8=", 8);
	//plat_cryptodma_base64((uintptr_t)src, 8, 0, CRYPTODMA_ACTION_DECRYPT);
	printf("BASE64 decode\n");
	// printf("src: %s\n", ((char *)src));
	plat_cryptodma_do(0, (uintptr_t)src, (uintptr_t)src, result_len, key,
			  KEY_ZERO, iv, BASE64, XXX, NULL);
	// src[src_len] = 0;
	// printf("result: %s\n", ((char *)src));

	return ret;
}

int testcase_base64_with_customer_code(void)
{
	int ret = 0;

	__attribute__((aligned(64))) uint8_t src[64] = { 0 };
	__attribute__((aligned(64))) uint8_t result[64] = { 0 };

	src[0] = 0xFB;
	src[1] = 0xEF;
	src[2] = 0xBE;

	//uint32_t customer_code = ('!' << 8) | '#';
	//plat_cryptodma_base64((uintptr_t)src, 3, customer_code, CRYPTODMA_ACTION_ENCRYPTION);
	//src[4] = 0;
	//uartlog("%s\n", ((char *)src));
	printf("BASE64 with customer code encode\n");
	plat_cryptodma_do(1, (uintptr_t)src, (uintptr_t)result, 3, key,
			  KEY_ZERO, iv, BASE64_CUSTOMER, XXX, NULL);
	uartlog("%s\n", ((char *)result));

	memcpy((char *)src, "####", 4);
	printf("BASE64 with customer code decode\n");
	//plat_cryptodma_base64((uintptr_t)src, 4, customer_code, CRYPTODMA_ACTION_DECRYPT);
	plat_cryptodma_do(0, (uintptr_t)src, (uintptr_t)result, 4, key,
			  KEY_ZERO, iv, BASE64_CUSTOMER, XXX, NULL);
	dump_data(result, 3);

	return ret;
}


static int do_aes(int argc, char **argv)
{
#if 1
	//__attribute__((aligned(64))) uint8_t result[10*1024] = {0};
	uint8_t *src = __10KB_bin;
	unsigned int src_len = __10KB_bin_len;
	uint8_t soft_src[src_len];
	unsigned char output[src_len];
	memset(output, 0, src_len);
	memcpy(soft_src, src, src_len);
#else
	uint8_t *src = (uint8_t *)0x05200000;
	uint8_t *result = (uint8_t *)0x05205000;

	memcpy(src, __10KB_bin, 10 * 1024);
#endif

#if 0
  cryptoDMA_aes_config_s config = {
    .mode = CRYPTODMA_ALGO_MODE_ECB,
    .size = CRYPTODMA_KEY_SIZE_128BITS,
    .action = CRYPTODMA_ACTION_ENCRYPTION,
  };

  memset(config.key, 'a', 16);
  memset(config.iv, 'a', 16);

  u64 timer_meter_start_cnt = timer_get_tick();
  plat_cryptodma_aes((uintptr_t)src, src_len, &config);
  timer_meter_start_cnt = ((timer_get_tick() - timer_meter_start_cnt) / TIMER_CNT_US);
  NOTICE("total Duaring : %ld , %ld\n", timer_meter_start_cnt, ((1000000/timer_meter_start_cnt)*(src_len/1024))/1024);
#endif

	printf("is_irq_enable is %d \n", is_irq_enable);

	printf("SPACC AES ECB 128 bits key encrypt\n");
	// //print_src_key_iv(src, src_len, key, 16, iv, 0);
	plat_cryptodma_do(1, (uintptr_t)src, (uintptr_t)src, src_len, key,
			  KEY_128BITS, iv, AES, ECB, NULL);
	// //print_src_key_iv(src, src_len);
#if 0
  config.action = CRYPTODMA_ACTION_DECRYPT;
  timer_meter_start_cnt = timer_get_tick();
  plat_cryptodma_aes((uintptr_t)src, src_len, &config);
  timer_meter_start_cnt = ((timer_get_tick() - timer_meter_start_cnt) / TIMER_CNT_US);
  NOTICE("Duaring : %ld , %ld\n", timer_meter_start_cnt, ((1000000/timer_meter_start_cnt)*(src_len/1024))/1024);
  dump_data(src, 16);
#endif
	//memset(src, 0, 16);

	printf("AES ECB 128 bits key decrypt\n");
	////print_src_key_iv(src, src_len, key, 16, iv, 0);
	plat_cryptodma_do(0, (uintptr_t)src, (uintptr_t)src, src_len, key,
			  KEY_128BITS, iv, AES, ECB, NULL);
	//print_src_key_iv(src, src_len);
	
	printf("AES ECB 192 bits key encrypt\n");
	////print_src_key_iv(src, src_len, key, 24, iv, 0);
	plat_cryptodma_do(1, (uintptr_t)src, (uintptr_t)src, src_len, key,
			  KEY_192BITS, iv, AES, ECB, NULL);
	//print_src_key_iv(src, src_len);

	printf("AES ECB 192 bits key decrypt\n");
	////print_src_key_iv(src, src_len, key, 24, iv, 0);
	plat_cryptodma_do(0, (uintptr_t)src, (uintptr_t)src, src_len, key,
			  KEY_192BITS, iv, AES, ECB, NULL);
	//print_src_key_iv(src, src_len);

	printf("AES ECB 256 bits key encrypt\n");
	////print_src_key_iv(src, src_len, key, 32, iv, 0);
	plat_cryptodma_do(1, (uintptr_t)src, (uintptr_t)src, src_len, key,
			  KEY_256BITS, iv, AES, ECB, NULL);
	//print_src_key_iv(src, src_len);

	//memset(src, 0, 16);
	printf("AES ECB 256 bits key decrypt\n");
	////print_src_key_iv(src, src_len, key, 32, iv, 0);
	plat_cryptodma_do(0, (uintptr_t)src, (uintptr_t)src, src_len, key,
			  KEY_256BITS, iv, AES, ECB, NULL);
	//print_src_key_iv(src, src_len);

	printf("AES CBC 128 bits key encrypt\n");
	////print_src_key_iv(src, src_len, key, 16, iv, 16);
	plat_cryptodma_do(1, (uintptr_t)src, (uintptr_t)src, src_len, key,
			  KEY_128BITS, iv, AES, CBC, NULL);
	//print_src_key_iv(src, src_len);

	//memset(src, 0, 16);
	printf("AES CBC 128 bits key decrypt\n");
	////print_src_key_iv(src, src_len, key, 16, iv, 16);
	plat_cryptodma_do(0, (uintptr_t)src, (uintptr_t)src, src_len, key,
			  KEY_128BITS, iv, AES, CBC, NULL);
	//print_src_key_iv(src, src_len);

	printf("AES CBC 192 bits key encrypt\n");
	////print_src_key_iv(src, src_len, key, 24, iv, 16);
	plat_cryptodma_do(1, (uintptr_t)src, (uintptr_t)src, src_len, key,
			  KEY_192BITS, iv, AES, CBC, NULL);
	//print_src_key_iv(src, src_len);

	//memset(src, 0, 16);
	printf("AES CBC 192 bits key decrypt\n");
	////print_src_key_iv(src, src_len, key, 24, iv, 16);
	plat_cryptodma_do(0, (uintptr_t)src, (uintptr_t)src, src_len, key,
			  KEY_192BITS, iv, AES, CBC, NULL);
	//print_src_key_iv(src, src_len);

	printf("AES CBC 256 bits key encrypt\n");
	////print_src_key_iv(src, src_len, key, 32, iv, 16);
	plat_cryptodma_do(1, (uintptr_t)src, (uintptr_t)src, src_len, key,
			  KEY_256BITS, iv, AES, CBC, NULL);
	//print_src_key_iv(src, src_len);

	//memset(src, 0, 16);
	printf("AES CBC 256 bits key decrypt\n");
	////print_src_key_iv(src, src_len, key, 32, iv, 16);
	plat_cryptodma_do(0, (uintptr_t)src, (uintptr_t)src, src_len, key,
			  KEY_256BITS, iv, AES, CBC, NULL);
	//print_src_key_iv(src, src_len);

	// ctr
	printf("AES CTR 128 bits key encrypt\n");
	////print_src_key_iv(src, src_len, key, 16, iv, 16);
	plat_cryptodma_do(1, (uintptr_t)src, (uintptr_t)src, src_len, key,
			  KEY_128BITS, iv, AES, CTR, NULL);
	//print_src_key_iv(src, src_len);

	//memset(src, 0, 16);
	printf("AES CTR 128 bits key decrypt\n");
	////print_src_key_iv(src, src_len, key, 16, iv, 16);
	plat_cryptodma_do(0, (uintptr_t)src, (uintptr_t)src, src_len, key,
			  KEY_128BITS, iv, AES, CTR, NULL);
	//print_src_key_iv(src, src_len);

	printf("AES CTR 192 bits key encrypt\n");
	////print_src_key_iv(src, src_len, key, 24, iv, 16);
	plat_cryptodma_do(1, (uintptr_t)src, (uintptr_t)src, src_len, key,
			  KEY_192BITS, iv, AES, CTR, NULL);
	//print_src_key_iv(src, src_len);

	//memset(src, 0, 16);
	printf("AES CTR 192 bits key decrypt\n");
	////print_src_key_iv(src, src_len, key, 24, iv, 16);
	plat_cryptodma_do(0, (uintptr_t)src, (uintptr_t)src, src_len, key,
			  KEY_192BITS, iv, AES, CTR, NULL);
	//print_src_key_iv(src, src_len);

	printf("AES CTR 256 bits key encrypt\n");
	////print_src_key_iv(src, src_len, key, 32, iv, 16);
	plat_cryptodma_do(1, (uintptr_t)src, (uintptr_t)src, src_len, key,
			  KEY_256BITS, iv, AES, CTR, NULL);
	//print_src_key_iv(src, src_len);

	//memset(src, 0, 16);
	printf("AES CTR 256 bits key decrypt\n");
	////print_src_key_iv(src, src_len, key, 32, iv, 16);
	plat_cryptodma_do(0, (uintptr_t)src, (uintptr_t)src, src_len, key,
			  KEY_256BITS, iv, AES, CTR, NULL);
	//print_src_key_iv(src, src_len);

	return 0;
}

static int do_des(int argc, char **argv)
{
	int ret = 0;
	int i;

	uint8_t *src = __10KB_bin;
  unsigned int src_len = __10KB_bin_len;
	//__attribute__((aligned(64))) uint8_t result[10*1024] = {0};

	// __attribute__((aligned(64))) uint8_t src[64] = { 0 };
	//__attribute__((aligned(64))) uint8_t result[64] = {0};
	// unsigned int src_len = 8;

	

	printf("DES ECB 128 bits key encrypt\n");
	////print_src_key_iv(src, src_len, key, 16, iv, 0);
	plat_cryptodma_do(1, (uintptr_t)src, (uintptr_t)src, src_len, key,
			  KEY_128BITS, iv, DES, ECB, NULL);
	//print_src_key_iv(src, src_len);

	printf("DES ECB 128 bits key decrypt\n");
	////print_src_key_iv(src, src_len, key, 16, iv, 0);
	plat_cryptodma_do(0, (uintptr_t)src, (uintptr_t)src, src_len, key,
			  KEY_128BITS, iv, DES, ECB, NULL);
	//print_src_key_iv(src, src_len);

	printf("DES CBC 128 bits key encrypt\n");
	////print_src_key_iv(src, src_len, key, 16, iv, 16);
	plat_cryptodma_do(1, (uintptr_t)src, (uintptr_t)src, src_len, key,
			  KEY_128BITS, iv, DES, CBC, NULL);
	//print_src_key_iv(src, src_len);

	printf("DES CBC 128 bits key decrypt\n");
	////print_src_key_iv(src, src_len, key, 16, iv, 16);
	plat_cryptodma_do(0, (uintptr_t)src, (uintptr_t)src, src_len, key,
			  KEY_128BITS, iv, DES, CBC, NULL);
	//print_src_key_iv(src, src_len);

	printf("DES CTR 128 bits key encrypt\n");
	////print_src_key_iv(src, src_len, key, 16, iv, 16);
	plat_cryptodma_do(1, (uintptr_t)src, (uintptr_t)src, src_len, key,
			  KEY_128BITS, iv, DES, CTR, NULL);
	//print_src_key_iv(src, src_len);

	printf("DES CTR 128 bits key decrypt\n");
	////print_src_key_iv(src, src_len, key, 16, iv, 16);
	plat_cryptodma_do(0, (uintptr_t)src, (uintptr_t)src, src_len, key,
			  KEY_128BITS, iv, DES, CTR, NULL);
	//print_src_key_iv(src, src_len);

	// TDES
	printf("Three DES ECB 192 bits key encrypt\n");
	////print_src_key_iv(src, src_len, key, 24, iv, 0);
	plat_cryptodma_do(1, (uintptr_t)src, (uintptr_t)src, src_len, key,
			  KEY_192BITS, iv, TDES, ECB, NULL);
	//print_src_key_iv(src, src_len);

	printf("Three DES ECB 192 bits key decrypt\n");
	////print_src_key_iv(src, src_len, key, 24, iv, 0);
	plat_cryptodma_do(0, (uintptr_t)src, (uintptr_t)src, src_len, key,
			  KEY_192BITS, iv, TDES, ECB, NULL);
	//print_src_key_iv(src, src_len);

	printf("Three DES CBC 192 bits key encrypt\n");
	////print_src_key_iv(src, src_len, key, 24, iv, 16);
	plat_cryptodma_do(1, (uintptr_t)src, (uintptr_t)src, src_len, key,
			  KEY_192BITS, iv, TDES, CBC, NULL);
	//print_src_key_iv(src, src_len);

	printf("Three DES CBC 192 bits key decrypt\n");
	////print_src_key_iv(src, src_len, key, 24, iv, 16);
	plat_cryptodma_do(0, (uintptr_t)src, (uintptr_t)src, src_len, key,
			  KEY_192BITS, iv, TDES, CBC, NULL);
	//print_src_key_iv(src, src_len);

	printf("Three DES CTR 192 bits key encrypt\n");
	////print_src_key_iv(src, src_len, key, 24, iv, 16);
	plat_cryptodma_do(1, (uintptr_t)src, (uintptr_t)src, src_len, key,
			  KEY_192BITS, iv, TDES, CTR, NULL);
	//print_src_key_iv(src, src_len);

	printf("Three DES CTR 192 bits key decrypt\n");
	////print_src_key_iv(src, src_len, key, 24, iv, 16);
	plat_cryptodma_do(0, (uintptr_t)src, (uintptr_t)src, src_len, key,
			  KEY_192BITS, iv, TDES, CTR, NULL);
	//print_src_key_iv(src, src_len);

	return ret;
}

static int do_sm4(int argc, char **argv)
{
	uint8_t *src = __10KB_bin;
	unsigned int src_len = __10KB_bin_len;
	//__attribute__((aligned(64))) uint8_t result[10*1024] = {0};

	cryptoDMA_sm4_config_s config = {
		.mode = CRYPTODMA_ALGO_MODE_ECB,
		.size = CRYPTODMA_KEY_SIZE_128BITS,
		.action = CRYPTODMA_ACTION_ENCRYPTION,
	};

	memset(config.key, 'a', 16);
	memset(config.iv, 'a', 16);

	//plat_cryptodma_sm4((uintptr_t)src, src_len, &config);

	printf("SM4 ECB 128 bits key encrypt\n");
	//print_src_key_iv(src, src_len, key, 16, iv, 0);
	plat_cryptodma_do(1, (uintptr_t)src, (uintptr_t)src, src_len, key,
			  KEY_128BITS, iv, SM4, ECB, NULL);
	//print_src_key_iv(src, src_len);

	//config.action = CRYPTODMA_ACTION_DECRYPT;
	//plat_cryptodma_sm4((uintptr_t)src, src_len, &config);

	//memset(src, 0, 16);
	printf("SM4 ECB 128 bits key decrypt\n");
	//print_src_key_iv(src, src_len, key, 16, iv, 0);
	plat_cryptodma_do(0, (uintptr_t)src, (uintptr_t)src, src_len, key,
			  KEY_128BITS, iv, SM4, ECB, NULL);
	//print_src_key_iv(src, src_len);

	//config.mode = CRYPTODMA_ALGO_MODE_CBC;
	//config.action = CRYPTODMA_ACTION_ENCRYPTION;

	//plat_cryptodma_sm4((uintptr_t)src, src_len, &config);
	printf("SM4 CBC 128 bits key encrypt\n");
	//print_src_key_iv(src, src_len, key, 16, iv, 16);
	plat_cryptodma_do(1, (uintptr_t)src, (uintptr_t)src, src_len, key,
			  KEY_128BITS, iv, SM4, CBC, NULL);
	//print_src_key_iv(src, src_len);

	//config.action = CRYPTODMA_ACTION_DECRYPT;
	//plat_cryptodma_sm4((uintptr_t)src, src_len, &config);
	//memset(src, 0, 16);
	printf("SM4 CBC 128 bits key decrypt\n");
	//print_src_key_iv(src, src_len, key, 16, iv, 16);
	plat_cryptodma_do(0, (uintptr_t)src, (uintptr_t)src, src_len, key,
			  KEY_128BITS, iv, SM4, CBC, NULL);
	//print_src_key_iv(src, src_len);

	//config.mode = CRYPTODMA_ALGO_MODE_CTR;
	//config.action = CRYPTODMA_ACTION_ENCRYPTION;

	//plat_cryptodma_sm4((uintptr_t)src, src_len, &config);
	printf("SM4 CTR 128 bits key encrypt\n");
	//print_src_key_iv(src, src_len, key, 16, iv, 16);
	plat_cryptodma_do(1, (uintptr_t)src, (uintptr_t)src, src_len, key,
			  KEY_128BITS, iv, SM4, CTR, NULL);
	//print_src_key_iv(src, src_len);

	//config.action = CRYPTODMA_ACTION_DECRYPT;
	//plat_cryptodma_sm4((uintptr_t)src, src_len, &config);
	//memset(src, 0, 16);
	printf("SM4 CTR 128 bits key decrypt\n");
	//print_src_key_iv(src, src_len, key, 16, iv, 16);
	plat_cryptodma_do(0, (uintptr_t)src, (uintptr_t)src, src_len, key,
			  KEY_128BITS, iv, SM4, CTR, NULL);
	//print_src_key_iv(src, src_len);

	//config.mode = CRYPTODMA_ALGO_MODE_OFB;
	//config.action = CRYPTODMA_ACTION_ENCRYPTION;

	//plat_cryptodma_sm4((uintptr_t)src, src_len, &config);
	printf("SM4 OFB 128 bits key encrypt\n");
	//print_src_key_iv(src, src_len, key, 16, iv, 16);
	plat_cryptodma_do(1, (uintptr_t)src, (uintptr_t)src, src_len, key,
			  KEY_128BITS, iv, SM4, OFB, NULL);
	//print_src_key_iv(src, src_len);

	//memset(src, 0, 16);
	printf("SM4 OFB 128 bits key decrypt\n");
	//print_src_key_iv(src, src_len, key, 16, iv, 16);
	plat_cryptodma_do(0, (uintptr_t)src, (uintptr_t)src, src_len, key,
			  KEY_128BITS, iv, SM4, OFB, NULL);
	//config.action = CRYPTODMA_ACTION_DECRYPT;
	//plat_cryptodma_sm4((uintptr_t)src, src_len, &config);
	//print_src_key_iv(src, src_len);

	return 0;
}

static int do_sha1(int argc, char **argv)
{
	testcase_spacc_sha1();

	return 0;
}

static int do_sha256(int argc, char **argv)
{
	testcase_spacc_sha256();

	return 0;
}

static int do_sm3(int argc, char **argv)
{
	testcase_spacc_sm3();

	return 0;
}

static int do_base64(int argc, char **argv)
{
	testcase_base64_normal();
	// testcase_base64_with_customer_code();

	return 0;
}

static int do_bypass(int argc, char **argv)
{
	testcase_bypass_dram_to_dram();
	testcase_bypass_tpu_sram_to_tpu_sram();

	return 0;
}

//DMA descriptor Chain mode configures DMA descriptors that perform two or more operations into one chain
static int do_chain(int argc, char **argv)
{
	uint32_t *dma_descriptor = NULL;
	uint32_t *dma_descriptor_1 = NULL;
	uint8_t *src = __10KB_bin;
	unsigned int src_len = __10KB_bin_len, i;
	int ret = 0;

	__attribute__((aligned(64))) uint8_t src_1[64] = { 0 };
	char *p = (char *)src_1;
	p[0] = 'h';
	p[1] = 'e';
	p[2] = 'o';
	p[3] = 'o';
	p[4] = 'o';

	//Padding data
	for (i = 5; i < 16; i++) {
		p[i] = 0xb;
	}

	cryptoDMA_aes_config_s config = {
		.mode = CRYPTODMA_ALGO_MODE_ECB,
		.size = CRYPTODMA_KEY_SIZE_128BITS,
		.action = CRYPTODMA_ACTION_ENCRYPTION,
	};

	memset(config.key, 'a', 16);
	memset(config.iv, 'a', 16);

	cryptoDMA_ctrl_s ctrl = {
		.key_src = CRYPTODMA_KEY_SOURCE_DESCRIPTOR,
		.is_end = 1,
	};

	dma_descriptor = plat_cryptodma_create_aes_descriptor(
		(uintptr_t)src, src_len, &config, &ctrl);

	ctrl.is_end = 0;
	ctrl.next_descriptor = (uintptr_t)dma_descriptor;
	dma_descriptor_1 = plat_cryptodma_create_aes_descriptor(
		(uintptr_t)src_1, src_len, &config, &ctrl);

	ret = trigger_cryptodma_work_and_wait_finish(dma_descriptor_1, 0);
	if (ret != 0) {
		printf("work failed\n");
		return -1;
	}

	invalidate_dcache_range((uintptr_t)src, src_len);
	invalidate_dcache_range((uintptr_t)src_1, src_len);
	dump_data(src, 16);
	dump_data(src_1, 16);

	return 0;
}

int cryptodma_aes(uintptr_t src, uintptr_t dest, uint64_t len,
		  unsigned char *iv, uint8_t isEncrypt)
{
	__attribute__((aligned(64))) uint32_t dma_descriptor[32] = { 0 };

	uint32_t status;

	INFO("AES/0x%lx/0x%lx/0x%lx\n", src, dest, len);

	// Prepare descriptor
	dma_descriptor[CRYPTODMA_CTRL] = 0x0980020f;
	if (isEncrypt == 1) {
		dma_descriptor[CRYPTODMA_CIPHER] =
			0x1 << 3 | CBC_ENABLE << 1 | 1;
	} else {
		dma_descriptor[CRYPTODMA_CIPHER] = 0x1 << 3 | CBC_ENABLE << 1;
	}
	dma_descriptor[CRYPTODMA_SRC_ADDR_L] = (uint32_t)(src & 0xFFFFFFFF);
	dma_descriptor[CRYPTODMA_SRC_ADDR_H] = (uint32_t)(src >> 32);

	dma_descriptor[CRYPTODMA_DST_ADDR_L] = (uint32_t)(dest & 0xFFFFFFFF);
	dma_descriptor[CRYPTODMA_DST_ADDR_H] = (uint32_t)(dest >> 32);

	dma_descriptor[CRYPTODMA_DATA_AMOUNT_L] = (uint32_t)(len & 0xFFFFFFFF);
	dma_descriptor[CRYPTODMA_DATA_AMOUNT_H] = (uint32_t)(len >> 32);
	memcpy(&dma_descriptor[CRYPTODMA_IV], iv, 16);

	// Set cryptodma control
	mmio_write_32(SEC_CRYPTODMA_BASE + CRYPTODMA_INT_MASK, 0x3);
	mmio_write_32(SEC_CRYPTODMA_BASE + CRYPTODMA_DES_BASE_L,
		      (uint32_t)((uint64_t)dma_descriptor & 0xFFFFFFFF));
	mmio_write_32(SEC_CRYPTODMA_BASE + CRYPTODMA_DES_BASE_H,
		      (uint32_t)((uint64_t)dma_descriptor >> 32));

	status = mmio_read_32(SEC_CRYPTODMA_BASE + CRYPTODMA_DMA_CTRL);

	flush_dcache_range((unsigned long)dma_descriptor,
			   sizeof(dma_descriptor));
	flush_dcache_range((uintptr_t)src, len);
	flush_dcache_range((uintptr_t)dest, len);
	// Clear interrupt
	mmio_write_32(SEC_CRYPTODMA_BASE + CRYPTODMA_WR_INT, 0x3);
	// Trigger cryptodma engine
	mmio_write_32(SEC_CRYPTODMA_BASE + CRYPTODMA_DMA_CTRL,
		      DMA_WRITE_MAX_BURST << 24 | DMA_READ_MAX_BURST << 16 |
			      DMA_DESCRIPTOR_MODE << 1 | DMA_ENABLE);
#if DEBUG
	printf("dma_descriptor[CRYPTODMA_CTRL]:%x\n",
	       dma_descriptor[CRYPTODMA_CTRL]);
	printf("dma_descriptor[CRYPTODMA_CIPHER]:%x\n",
	       dma_descriptor[CRYPTODMA_CIPHER]);
	printf("SEC_CRYPTODMA_BASE[%x]:%x\n", SEC_CRYPTODMA_BASE + 0x14,
	       mmio_read_32(SEC_CRYPTODMA_BASE + 0x14));
	printf("SEC_CRYPTODMA_BASE[CTRL]%x:%x\n",
	       SEC_CRYPTODMA_BASE + CRYPTODMA_DMA_CTRL,
	       mmio_read_32(SEC_CRYPTODMA_BASE + CRYPTODMA_DMA_CTRL));
#endif
	do {
		status = mmio_read_32(SEC_CRYPTODMA_BASE + CRYPTODMA_WR_INT);
		INFO("INT status 0x%x\n", status);
	} while (status == 0);

	return 0;
}
static int do_use_efuse_key(int argc, char **argv)
{
	write_efuse_key();
	uint8_t *src = __16B_bin;
	unsigned int src_len = __16B_bin_len;
	is_irq_enable = 1;

	printf("cryptodma_aes encrypt=========================\n");
	//print_src_key_iv(src, src_len, key, 32, iv, 16);
	cryptodma_aes((uintptr_t)src, (uintptr_t)src, src_len, iv, 1);
	//print_src_key_iv(src, src_len);

	printf("cryptodma_aes decrypt=========================\n");
	//print_src_key_iv(src, src_len, key, 16, iv, 0);
	cryptodma_aes((uintptr_t)src, (uintptr_t)src, src_len, iv, 0);
	//print_src_key_iv(src, src_len);

	return 0;
}

static int do_reset_ip(int argc, char **argv)
{
	__attribute__((aligned(64))) uint32_t dma_descriptor[32] = { 0 };
	uint32_t value;

	mmio_write_32(SPACC_BASE + CRYPTODMA_INT_MASK, 0x7);
	mmio_write_32(SPACC_BASE + CRYPTODMA_DES_BASE_L,
		      (uint32_t)((uint64_t)dma_descriptor & 0xFFFFFFFF));
	mmio_write_32(SPACC_BASE + CRYPTODMA_DES_BASE_H,
		      (uint32_t)((uint64_t)dma_descriptor >> 32));
	value = mmio_read_32(SPACC_BASE + CRYPTODMA_INT_MASK);
	uartlog("ini mask : %x\n", value);
	value = mmio_read_32(SPACC_BASE + CRYPTODMA_DES_BASE_L);
	uartlog("des base L : %x\n", value);
	value = mmio_read_32(SPACC_BASE + CRYPTODMA_DES_BASE_H);
	uartlog("des base H : %x\n", value);

	// enable soft reset x system
	mmio_write_32(0x28100000 + 0x8, (0x1 << 5));

	uartlog("default reset register : %x\n", mmio_read_32(0x3003000 + 0x8));
	mmio_clrbits_32(0x3003000 + 0x8, 0x1 << 21);
	uartlog("after clr bit reset register : %x\n",
		mmio_read_32(0x3003000 + 0x8));
	mdelay(2);
	mmio_setbits_32(0x3003000 + 0x8, 0x1 << 21);
	value = mmio_read_32(0x3003000 + 0x8);
	uartlog("after set bit reset register : %x\n", value);
	value = mmio_read_32(SPACC_BASE + CRYPTODMA_INT_MASK);
	uartlog("ini mask : %x\n", value);
	value = mmio_read_32(SPACC_BASE + CRYPTODMA_DES_BASE_L);
	uartlog("des base L : %x\n", value);
	value = mmio_read_32(SPACC_BASE + CRYPTODMA_DES_BASE_H);
	uartlog("des base H : %x\n", value);

	return 0;
}

static int do_c906_intr_test(int argc, char **argv)
{
	uint8_t *src = __10KB_bin;
	unsigned int src_len = __10KB_bin_len;
	irq_done = 1;
	is_c906irq_enable = 1;
	is_irq_enable = 1;
#ifdef CONFIG_C906
	bringup_c906l(0x88000000);
#endif
	//FPGA:mdelay(5000)
	mdelay(20);
	printf("AES ECB 128 bits key encrypt\n");
	//print_src_key_iv(src, src_len, key, 16, iv, 0);
	plat_cryptodma_do(1, (uintptr_t)src, (uintptr_t)src, src_len, key,
			  KEY_128BITS, iv, AES, ECB, NULL);
	//print_src_key_iv(src, src_len);

	// printf("read plic irq %0x\n", mmio_read_32(0x70001000));
	return 0;
}

static int do_clk_disable(int argc, char **argv)
{
	uint32_t value = mmio_read_32(SEC_TOP_BASE);
	uartlog("SYS CTRL REG : 0x%x\n", value);

	mmio_setbits_32(SEC_TOP_BASE, 0x1);

	// can't access
	mmio_read_32(SPACC_BASE + CRYPTODMA_INT_MASK);
	mmio_write_32(SPACC_BASE + CRYPTODMA_INT_MASK, 0x7);
	uartlog("SPACC_BASE + CRYPTODMA_INT_MASK : 0x%x\n",
		mmio_read_32(SPACC_BASE + CRYPTODMA_INT_MASK));
	return 0;
}

static int do_clk_re_enable(int argc, char **argv)
{
	mmio_setbits_32(SEC_TOP_BASE, 0x1);
	mdelay(1);
	mmio_clrbits_32(SEC_TOP_BASE, 0x1);

	testcase_bypass_dram_to_dram();

	return 0;
}

static int do_auto_run(int argc, char **argv)
{
	do_aes(0, NULL);
	do_des(0, NULL);
	do_sm4(0, NULL);
	do_sha1(0, NULL);
	do_sha256(0, NULL);
	do_sm3(0, NULL);
	do_bypass(0, NULL);

	return 0;
}

static int do_tick(int argc, char **argv)
{
	u64 timer_meter_start_cnt = timer_get_tick();
	u64 tmp;

	while (1) {
		tmp = timer_get_tick();
		if ((tmp - timer_meter_start_cnt) > 25000000) {
			timer_meter_start_cnt = tmp;
			printf("1s\n");
		}
	}

	return 0;
}

// static int do_enable_cache(int argc, char **argv)
// {
// 	// Fixme : haug when bmtest run in tpu sram
// 	icache_enable();
// 	dcache_enable();
// 	return 0;
// }
static int do_irq_test(int argc, char **argv)
{
	is_irq_enable = strtoul(argv[1], NULL, 0);
	is_c906irq_enable = 0;
	printf("is_irq_enable is %d \n", is_irq_enable);
	// do_auto_run(0, NULL);

	return 0;
}
static int do_mcu_intr_test(int argc, char **argv)
{
	uint8_t *src = __16B_bin;
	unsigned int src_len = __16B_bin_len;
	is_irq_enable = 1;
	//for enable muc intr
	mmio_write_32(0x03000248, 0x1);
	mmio_write_32(0x05025020, 0x05200080);
	mmio_write_32(0x05025024, 0x05205000);
	mmio_write_32(0x05025018, 0x1fffff);
	mmio_write_32(0x0502601c, 0x3);
	mmio_write_32(0x0502601c, 0x8);
	printf("is_irq_enable is %d \n", is_irq_enable);

	printf("AES ECB 128 bits key encrypt\n");
	//print_src_key_iv(src, src_len, key, 16, iv, 0);
	plat_cryptodma_do(1, (uintptr_t)src, (uintptr_t)src, src_len, key,
			  KEY_128BITS, iv, AES, ECB, NULL);
	//print_src_key_iv(src, src_len);

	return 0;
}
static struct cmd_entry spacc_cmds[] __attribute__((unused)) = {
	//test encrytped
	{ "aes", do_aes, 0, ": aes algo" },
	{ "des", do_des, 0, ": des algo" },
	{ "sm4", do_sm4, 0, ": sm4 algo" },
	// for hash
	{ "sha1", do_sha1, 0, ": sha1 algo" },
	{ "sha256", do_sha256, 0, ": sha256 algo" },
	{ "sm3", do_sm3, 0, ": sm3 algo" },
	//base64
	{ "base64", do_base64, 0, ": base64 algo" },
	//test other function
	{ "bypass", do_bypass, 0, ": bypass" }, //undo anying,test access speed
	{ "chain", do_chain, 0, ": chain" },
	{ "use_efuse_key", do_use_efuse_key, 0, ": use efuse key" },
	{ "reset_ip", do_reset_ip, 0, ": reset spacc" },
	// for clk test
	{ "clk_disable", do_clk_disable, 0,
	  ": disable clk,test read and write" }, //hang up
	{ "clk_re_enable", do_clk_re_enable, 0,
	  ": disable clk and then enable clk,test bypass" },
	// auto run
	{ "auto_run", do_auto_run, 0, ": for auto run with irq mode" },
	// { "enable_cache", do_enable_cache, 0, ": for time cost" },
	//irq_mode
	{ "ap_intr", do_irq_test, 1,
	  "ca53 intr_test,0 means polling mode,1 means irq mode" },
	{ "c906_irq", do_c906_intr_test, 0, "for irq test" },
	{ "mcu_irq", do_mcu_intr_test, 0, "for irq test" },
};

#ifndef BUILD_TEST_CASE_all
int testcase_main(void)
{
	int i;
	NOTICE("start spacc bmtest !\n");
	for (i = 0; i < ARRAY_SIZE(spacc_cmds); i++) {
		command_add(&spacc_cmds[i]);
	}

	cli_simple_loop();

	return 0;
}
#endif
