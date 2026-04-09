#ifndef _CRYPTODMA_H_
#define _CRYPTODMA_H_

typedef enum CRYPTODMA_ALGO_MODE {
	CRYPTODMA_ALGO_MODE_ECB,
	CRYPTODMA_ALGO_MODE_CBC,
	CRYPTODMA_ALGO_MODE_CTR,
	CRYPTODMA_ALGO_MODE_OFB,
} CRYPTODMA_ALGO_MODE_E;

typedef enum CRYPTODMA_KEY_SIZE {
	CRYPTODMA_KEY_SIZE_64BITS,
	CRYPTODMA_KEY_SIZE_128BITS,
	CRYPTODMA_KEY_SIZE_192BITS,
	CRYPTODMA_KEY_SIZE_256BITS,
} CRYPTODMA_KEY_SIZE_E;

typedef enum CRYPTODMA_ACTION {
	CRYPTODMA_ACTION_ENCRYPTION,
	CRYPTODMA_ACTION_DECRYPT,
} CRYPTODMA_ACTION_E;

typedef enum CRYPTODMA_KEY_SOURCE {
	CRYPTODMA_KEY_SOURCE_DESCRIPTOR,
	CRYPTODMA_KEY_SOURCE_OTP, // key from otp
} CRYPTODMA_KEY_SOURCE_E;

typedef struct cryptoDMA_ctrl {
	CRYPTODMA_KEY_SOURCE_E key_src;
	int is_end;
	uint64_t next_descriptor;
} cryptoDMA_ctrl_s;

typedef struct cryptoDMA_base64_action {
	CRYPTODMA_ACTION_E action;
} cryptoDMA_base64_action_s;

typedef struct cryptoDMA_aes_config {
	unsigned char key[32];
	unsigned char iv[16];
	CRYPTODMA_ALGO_MODE_E mode;
	CRYPTODMA_KEY_SIZE_E size;
	CRYPTODMA_ACTION_E action;
} cryptoDMA_aes_config_s;

typedef struct cryptoDMA_des_config {
	unsigned char key[24];
	unsigned char iv[16];
	CRYPTODMA_ALGO_MODE_E mode;
	CRYPTODMA_KEY_SIZE_E action;
} cryptoDMA_des_config_s;

typedef enum CRYPTODMA_SHA {
	CRYPTODMA_SHA_SHA1,
	CRYPTODMA_SHA_SHA256,
} CRYPTODMA_SHA_E;

typedef struct cryptoDMA_sha_config {
	CRYPTODMA_SHA_E algo;
	uint32_t state[8];
	unsigned char result[32];
} cryptoDMA_sha_config_s;

typedef struct cryptoDMA_sm3_config {
	uint32_t state[8];
} cryptoDMA_sm3_config_s;

typedef cryptoDMA_aes_config_s cryptoDMA_sm4_config_s;
typedef cryptoDMA_des_config_s cryptoDMA_tdes_config_s;
int plat_cryptodma_base64(uintptr_t src, uint32_t len, uint32_t customer_code,
			  CRYPTODMA_ACTION_E ation);
int plat_cryptodma_bypass(uintptr_t src, uint32_t len, uintptr_t dst);
int plat_cryptodma_sm4(uintptr_t src, uint32_t len,
		       cryptoDMA_sm4_config_s *config);
int plat_cryptodma_aes(uintptr_t src, uint32_t len,
		       cryptoDMA_aes_config_s *config);

int plat_cryptodma_sha(uintptr_t src, uint32_t len,
		       cryptoDMA_sha_config_s *config);
int plat_cryptodma_sm3(uintptr_t src, uint32_t len,
		       cryptoDMA_sm3_config_s *config);

uint32_t *plat_cryptodma_create_aes_descriptor(uintptr_t src, uint32_t len,
					       cryptoDMA_aes_config_s *config,
					       cryptoDMA_ctrl_s *ctrl);
int trigger_cryptodma_work_and_wait_finish(uint32_t *dma_descriptor,
					   int irq_enable);

#endif
