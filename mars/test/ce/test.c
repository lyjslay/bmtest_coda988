#include "boot_test.h"
#include "uart.h"
#include "cli.h"
#include "command.h"
#include <ce_port.h>
#include <cesd.h>
#include <golden.h>
#include <xrand.h>
#include <ce.h>
#include <base64.h>

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(array)	(sizeof(array)/sizeof(array[0]))
#endif

/*
 * eFuse base
 */
#define EFUSE_BASE			0x50028000
/*
 * eFuse controller definitions
 */
#define REG_EFUSE_MODE			0x0
#define REG_EFUSE_ADR			0x4
#define REG_EFUSE_RD_DATA		0xc
#define REG_EFUSE_ECCSRAM_ADR		0x10
#define REG_EFUSE_ECCSRAM_RDPORT	0x14
#define EFUSE_NUM_ADDRESS_BITS		7
#define EFUSE_MAGIC_BEGIN		0x0b0b0b0b
#define EFUSE_MAGIC_END			0x0e0e0e0e
#define SPIFP_MAGIC_BEGIN		0x1b1b1b1b
#define SPIFP_MAGIC_END			0x1e1e1e1e

#include <mmio.h>

void plat_bm_efuse_init(void)
{
	uint32_t reg_val;
	//int loop = 100;

	// SRAM loading, do it once when bootup
	while (mmio_read_32(EFUSE_BASE + REG_EFUSE_MODE) & 0x3) {
#if 0
		if (loop-- > 0)
			mdelay(10);
		else
			break;
#endif
	}
	reg_val = mmio_read_32(EFUSE_BASE + REG_EFUSE_MODE);
	reg_val = reg_val | (0x3 << 30) | (0x1 << 28);
	mmio_write_32(EFUSE_BASE + REG_EFUSE_MODE, reg_val);
	//loop = 100;
	while (mmio_read_32(EFUSE_BASE + REG_EFUSE_MODE) & (1 << 31)) {
#if 0
		if (loop-- > 0)
			mdelay(10);
		else
			break;
#endif
	}
}

static uint32_t efuse_raw_read(uint32_t address)
{
	uint32_t reg_val;
	int loop = 100;

	if (address > ((1 << EFUSE_NUM_ADDRESS_BITS) - 1))
		return 0;

	if (address == 0 || address == 1) { // embedded read
		mmio_write_32(EFUSE_BASE + REG_EFUSE_ADR, address);
		reg_val = mmio_read_32(EFUSE_BASE + REG_EFUSE_MODE);
		reg_val &= ~0x3;
		reg_val |= 0x2;
		mmio_write_32(EFUSE_BASE + REG_EFUSE_MODE, reg_val);
		while (mmio_read_32(EFUSE_BASE + REG_EFUSE_MODE) & 0x3) {
#if 0
			if (loop-- > 0)
				mdelay(10);
			else
				break;
#endif
		}
		reg_val = mmio_read_32(EFUSE_BASE + REG_EFUSE_RD_DATA);
	} else { // sram read
		mmio_write_32(EFUSE_BASE + REG_EFUSE_ECCSRAM_ADR, address - 2);
		reg_val = mmio_read_32(EFUSE_BASE + REG_EFUSE_ECCSRAM_RDPORT);
	}
	return reg_val;
}

static void efuse_set_bit(uint32_t address, uint32_t bit_i)
{
	const uint32_t address_mask = (1 << EFUSE_NUM_ADDRESS_BITS) - 1;
	uint32_t adr_val;
	int loop = 100;

	if (address > ((1 << EFUSE_NUM_ADDRESS_BITS) - 1))
		return;

	// embedded write
	adr_val = (address & address_mask) |
		((bit_i & 0x1f) << EFUSE_NUM_ADDRESS_BITS);
	mmio_write_32(EFUSE_BASE + REG_EFUSE_ADR, adr_val);
	mmio_setbits_32(EFUSE_BASE + REG_EFUSE_MODE, 0x3);
	while (mmio_read_32(EFUSE_BASE + REG_EFUSE_MODE) & 0x3) {
#if 0
		if (loop-- > 0)
			mdelay(10);
		else
			break;
#endif
	}
}

static void efuse_raw_write(uint32_t address, uint32_t val)
{
	int i;

	for (i = 0; i < 32; i++) {
		if ((val >> i) & 1)
			efuse_set_bit(address, i);
	}
	plat_bm_efuse_init();
}

void efuse_dump(uint32_t address, unsigned long l)
{
	unsigned long i;

	for (i = 0; i < l; ++i) {
		if (i % 4 == 0)
			printf("\n %04lx: ", i);
		printf("%08x ", efuse_raw_read(address + i));
	}
	printf("\n");
}

static int is_secure_key_enable(void)
{
	unsigned int tmp = efuse_raw_read(1);
	int b0 = tmp & 1;
	int b1 = (tmp >> 1) & 1;
	if (b0 == b1) {
		return b0;
	} else {
		ce_err("eFuse double bit not equal\n");
		return 0;
	}
}

static int program_secure_key_enable(void)
{
	efuse_raw_write(1, 3);
	return is_secure_key_enable();
}

static uint32_t key2cell(void *key)
{
	unsigned char *k = key;
#if 1
	/* little endian */
	uint32_t x = k[0] | (k[1] << 8) | (k[2] << 16) | (k[3] << 24);
#else
	/* big endian */
	uint32_t x = k[3] | (k[2] << 8) | (k[1] << 16) | (k[0] << 24);
#endif
	return x;
}

static int program_secure_key(void *key)
{
	int i;
	uint32_t k[8];

	for (i = 0; i < ARRAY_SIZE(k); ++i)
		k[i] = key2cell(((unsigned char *)key) + i * 4);

	for (i = 0; i < ARRAY_SIZE(k); ++i) {
		/* write secure key and its backup bank */
		efuse_raw_write(2 + i, k[i]);
		efuse_raw_write(2 + ARRAY_SIZE(k) + i, k[i]);
	}

	return 0;
}


#define TEST_DATA_LEN	(1024)

static unsigned char input[TEST_DATA_LEN];
static unsigned char ce_out[TEST_DATA_LEN];
static unsigned char gdn_out[TEST_DATA_LEN];

#define TEST_MSG_LEN	(1024)
static unsigned char msg[TEST_MSG_LEN + 64];

#define TEST_KEY_LEN	(32)
static unsigned char key[TEST_KEY_LEN];
static unsigned char secure_key[TEST_KEY_LEN] = {
	0xf9, 0x66, 0x3b, 0x6a, 0xda, 0xe5, 0x7f, 0x6e,
	0xeb, 0xc2, 0xb3, 0x6d, 0x1c, 0x6f, 0x25, 0xb2,
	0xd8, 0x37, 0x05, 0x9d, 0xd3, 0x35, 0x86, 0xf7,
	0x3c, 0x4c, 0x80, 0x88, 0x4a, 0x73, 0x4d, 0x5a,
};
#define TEST_IV_LEN	(32)
static unsigned char iv[TEST_IV_LEN];
static unsigned char ce_iv_out[TEST_IV_LEN];
static unsigned char gdn_iv_out[TEST_IV_LEN];
#define TEST_HASH_LEN	(32)
static unsigned char ce_dgst[TEST_HASH_LEN];
static unsigned char gdn_dgst[TEST_HASH_LEN];

static int test_check_title(char *title)
{
	printf("%-20s", title);
	return 0;
}

enum {
	CHECK_DEFAULT	= 1<<0,
	CHECK_IVOUT	= 1<<1,
};

static int test_check_result_base(void)
{
	int res = 0;
	unsigned int i;

	for (i = 0; i < TEST_DATA_LEN; ++i) {
		if (gdn_out[i] != ce_out[i]) {
			printf(
		"%dth byte not match, golden is [%02x] but test is [%02x]\n"
			       , i, gdn_out[i], ce_out[i]);
			res |= CHECK_DEFAULT;
			break;
		}
	}
	printf("[%s]\n", res?"FAIL":"PASS");
	return res;
}

static int test_check_result_cipher(struct ce_cipher *cipher)
{
	int res = 0;
	unsigned int i;

	for (i = 0; i < TEST_DATA_LEN; ++i) {
		if (gdn_out[i] != ce_out[i]) {
			printf(
		"%dth byte not match, golden is [%02x] but test is [%02x]\n"
			       , i, gdn_out[i], ce_out[i]);
			res |= CHECK_DEFAULT;
			break;
		}
	}
	unsigned int block_len = ce_cipher_block_len(cipher->alg);

	if (cipher->iv_out)
		if (memcmp(ce_iv_out, gdn_iv_out, block_len)) {
			res |= CHECK_IVOUT;
			printf("expected output iv\n");
			dump(gdn_iv_out, block_len);
			printf("calculated output iv\n");
			dump(ce_iv_out, block_len);
			while (1)
				;
		}
	printf("[%s] \t%lld B/s\n", res ? "FAIL" : "PASS",
	       cesd_throughput());
	return res;
}

static int test_check_result_hash(struct ce_hash *hash)
{
	unsigned int len = ce_hash_hash_len(hash->alg);
	int res = 0;
	unsigned int i;

	for (i = 0; i < len; ++i) {
		if (gdn_dgst[i] != ce_dgst[i]) {
			printf(
		"%dth byte not match, golden is [%02x] but test is [%02x]\n"
			       , i, gdn_dgst[i], ce_dgst[i]);
			res = 1;
			break;
		}
	}
	printf("[%s]\n", res?"FAIL":"PASS");
	return res;
}

static int test_des_ecb(void)
{
	int ret = 0;
	struct ce_cipher ce_cipher = {
		.alg = CE_DES_ECB,
		.enc = 1,
		.iv = NULL,
		.key = key,
		.src = input,
		.dst = ce_out,
		.len = TEST_DATA_LEN,
		.iv_out = NULL,
	};

	test_check_title("des-ecb-enc");
	gdn_ecb("des", input, gdn_out, TEST_DATA_LEN, key, 8, 1);

	ce_cipher.enc = 1;
	cesd_cipher_do(&ce_cipher);
	ret |= test_check_result_cipher(&ce_cipher);


	test_check_title("des-ecb-dec");
	gdn_ecb("des", input, gdn_out, TEST_DATA_LEN, key, 8, 0);
	ce_cipher.enc = 0;
	cesd_cipher_do(&ce_cipher);
	ret |= test_check_result_cipher(&ce_cipher);
	return ret;
}

static int test_des_cbc(void)
{
	int ret = 0;
	struct ce_cipher ce_cipher = {
		.alg = CE_DES_CBC,
		.enc = 1,
		.iv = iv,
		.key = key,
		.src = input,
		.dst = ce_out,
		.len = TEST_DATA_LEN,
		.iv_out = ce_iv_out,
	};

	test_check_title("des-cbc-enc");
	gdn_cbc("des", input, gdn_out,
		TEST_DATA_LEN, key, 8, 1, iv, gdn_iv_out);

	ce_cipher.enc = 1;
	cesd_cipher_do(&ce_cipher);
	ret |= test_check_result_cipher(&ce_cipher);


	test_check_title("des-cbc-dec");
	gdn_cbc("des", input, gdn_out,
		TEST_DATA_LEN, key, 8, 0, iv, gdn_iv_out);
	ce_cipher.enc = 0;
	cesd_cipher_do(&ce_cipher);
	ret |= test_check_result_cipher(&ce_cipher);
	return ret;
}

static int test_des_ctr(void)
{
	int ret = 0;
	struct ce_cipher ce_cipher = {
		.alg = CE_DES_CTR,
		.enc = 1,
		.iv = iv,
		.key = key,
		.src = input,
		.dst = ce_out,
		.len = TEST_DATA_LEN,
		.iv_out = ce_iv_out,
	};

	test_check_title("des-ctr-enc");
	gdn_ctr("des", input, gdn_out,
		TEST_DATA_LEN, key, 8, 1, iv, gdn_iv_out);

	ce_cipher.enc = 1;
	cesd_cipher_do(&ce_cipher);
	ret |= test_check_result_cipher(&ce_cipher);


	test_check_title("des-ctr-dec");
	gdn_ctr("des", input, gdn_out,
		TEST_DATA_LEN, key, 8, 0, iv, gdn_iv_out);
	ce_cipher.enc = 0;
	cesd_cipher_do(&ce_cipher);
	ret |= test_check_result_cipher(&ce_cipher);
	return ret;
}

static int test_des_ede3_ecb(void)
{
	int ret = 0;
	struct ce_cipher ce_cipher = {
		.alg = CE_DES_EDE3_ECB,
		.enc = 1,
		.iv = NULL,
		.key = key,
		.src = input,
		.dst = ce_out,
		.len = TEST_DATA_LEN,
		.iv_out = NULL,
	};

	test_check_title("des-ede3-ecb-enc");
	gdn_ecb("3des", input, gdn_out, TEST_DATA_LEN, key, 24, 1);

	ce_cipher.enc = 1;
	cesd_cipher_do(&ce_cipher);
	ret |= test_check_result_cipher(&ce_cipher);


	test_check_title("des-ede3-ecb-dec");
	gdn_ecb("3des", input, gdn_out, TEST_DATA_LEN, key, 24, 0);
	ce_cipher.enc = 0;
	cesd_cipher_do(&ce_cipher);
	ret |= test_check_result_cipher(&ce_cipher);
	return ret;
}

static int test_des_ede3_cbc(void)
{
	int ret = 0;
	struct ce_cipher ce_cipher = {
		.alg = CE_DES_EDE3_CBC,
		.enc = 1,
		.iv = iv,
		.key = key,
		.src = input,
		.dst = ce_out,
		.len = TEST_DATA_LEN,
		.iv_out = ce_iv_out,
	};

	test_check_title("des-ede3-cbc-enc");
	gdn_cbc("3des", input, gdn_out,
		TEST_DATA_LEN, key, 24, 1, iv, gdn_iv_out);

	ce_cipher.enc = 1;
	cesd_cipher_do(&ce_cipher);
	ret |= test_check_result_cipher(&ce_cipher);


	test_check_title("des-ede3-cbc-dec");
	gdn_cbc("3des", input, gdn_out,
		TEST_DATA_LEN, key, 24, 0, iv, gdn_iv_out);
	ce_cipher.enc = 0;
	cesd_cipher_do(&ce_cipher);
	ret |= test_check_result_cipher(&ce_cipher);
	return ret;
}

static int test_des_ede3_ctr(void)
{
	int ret = 0;
	struct ce_cipher ce_cipher = {
		.alg = CE_DES_EDE3_CTR,
		.enc = 1,
		.iv = iv,
		.key = key,
		.src = input,
		.dst = ce_out,
		.len = TEST_DATA_LEN,
		.iv_out = ce_iv_out,
	};

	test_check_title("des-ede3-ctr-enc");
	gdn_ctr("3des", input, gdn_out,
		TEST_DATA_LEN, key, 24, 1, iv, gdn_iv_out);

	ce_cipher.enc = 1;
	cesd_cipher_do(&ce_cipher);
	ret |= test_check_result_cipher(&ce_cipher);


	test_check_title("des-ede3-ctr-dec");
	gdn_ctr("3des", input, gdn_out,
		TEST_DATA_LEN, key, 24, 0, iv, gdn_iv_out);
	ce_cipher.enc = 0;
	cesd_cipher_do(&ce_cipher);
	ret |= test_check_result_cipher(&ce_cipher);
	return ret;
}

static int test_aes128_ecb(void)
{
	int ret = 0;
	struct ce_cipher ce_cipher = {
		.alg = CE_AES_128_ECB,
		.enc = 1,
		.iv = NULL,
		.key = key,
		.src = input,
		.dst = ce_out,
		.len = TEST_DATA_LEN,
		.iv_out = NULL,
	};

	test_check_title("aes-128-ecb-enc");
	gdn_ecb("aes", input, gdn_out, TEST_DATA_LEN, key, 16, 1);

	ce_cipher.enc = 1;
	cesd_cipher_do(&ce_cipher);
	ret |= test_check_result_cipher(&ce_cipher);


	test_check_title("aes-128-ecb-dec");
	gdn_ecb("aes", input, gdn_out, TEST_DATA_LEN, key, 16, 0);
	ce_cipher.enc = 0;
	cesd_cipher_do(&ce_cipher);
	ret |= test_check_result_cipher(&ce_cipher);
	return ret;
}

static int test_aes128_cbc(void)
{
	int ret = 0;
	struct ce_cipher ce_cipher = {
		.alg = CE_AES_128_CBC,
		.enc = 1,
		.iv = iv,
		.key = key,
		.src = input,
		.dst = ce_out,
		.len = TEST_DATA_LEN,
		.iv_out = ce_iv_out,
	};

	test_check_title("aes-128-cbc-enc");
	gdn_cbc("aes", input, gdn_out,
		TEST_DATA_LEN, key, 16, 1, iv, gdn_iv_out);

	ce_cipher.enc = 1;
	cesd_cipher_do(&ce_cipher);
	ret |= test_check_result_cipher(&ce_cipher);


	test_check_title("aes-128-cbc-dec");
	gdn_cbc("aes", input, gdn_out,
		TEST_DATA_LEN, key, 16, 0, iv, gdn_iv_out);
	ce_cipher.enc = 0;
	cesd_cipher_do(&ce_cipher);
	ret |= test_check_result_cipher(&ce_cipher);
	return ret;
}

static int test_aes128_ctr(void)
{
	int ret = 0;
	struct ce_cipher ce_cipher = {
		.alg = CE_AES_128_CTR,
		.enc = 1,
		.iv = iv,
		.key = key,
		.src = input,
		.dst = ce_out,
		.len = TEST_DATA_LEN,
		.iv_out = ce_iv_out,
	};

	test_check_title("aes-128-ctr-enc");
	gdn_ctr("aes", input, gdn_out,
		TEST_DATA_LEN, key, 16, 1, iv, gdn_iv_out);

	ce_cipher.enc = 1;
	cesd_cipher_do(&ce_cipher);
	ret |= test_check_result_cipher(&ce_cipher);


	test_check_title("aes-128-ctr-dec");
	gdn_ctr("aes", input, gdn_out,
		TEST_DATA_LEN, key, 16, 0, iv, gdn_iv_out);
	ce_cipher.enc = 0;
	cesd_cipher_do(&ce_cipher);
	ret |= test_check_result_cipher(&ce_cipher);
	return ret;
}

static int test_aes192_ecb(void)
{
	int ret = 0;
	struct ce_cipher ce_cipher = {
		.alg = CE_AES_192_ECB,
		.enc = 1,
		.iv = NULL,
		.key = key,
		.src = input,
		.dst = ce_out,
		.len = TEST_DATA_LEN,
		.iv_out = NULL,
	};

	test_check_title("aes-192-ecb-enc");
	gdn_ecb("aes", input, gdn_out, TEST_DATA_LEN, key, 24, 1);

	ce_cipher.enc = 1;
	cesd_cipher_do(&ce_cipher);
	ret |= test_check_result_cipher(&ce_cipher);


	test_check_title("aes-192-ecb-dec");
	gdn_ecb("aes", input, gdn_out, TEST_DATA_LEN, key, 24, 0);
	ce_cipher.enc = 0;
	cesd_cipher_do(&ce_cipher);
	ret |= test_check_result_cipher(&ce_cipher);
	return ret;
}

static int test_aes192_cbc(void)
{
	int ret = 0;
	struct ce_cipher ce_cipher = {
		.alg = CE_AES_192_CBC,
		.enc = 1,
		.iv = iv,
		.key = key,
		.src = input,
		.dst = ce_out,
		.len = TEST_DATA_LEN,
		.iv_out = ce_iv_out,
	};

	test_check_title("aes-192-cbc-enc");
	gdn_cbc("aes", input, gdn_out,
		TEST_DATA_LEN, key, 24, 1, iv, gdn_iv_out);

	ce_cipher.enc = 1;
	cesd_cipher_do(&ce_cipher);
	ret |= test_check_result_cipher(&ce_cipher);


	test_check_title("aes-192-cbc-dec");
	gdn_cbc("aes", input, gdn_out,
		TEST_DATA_LEN, key, 24, 0, iv, gdn_iv_out);
	ce_cipher.enc = 0;
	cesd_cipher_do(&ce_cipher);
	ret |= test_check_result_cipher(&ce_cipher);
	return ret;
}

static int test_aes192_ctr(void)
{
	int ret = 0;
	struct ce_cipher ce_cipher = {
		.alg = CE_AES_192_CTR,
		.enc = 1,
		.iv = iv,
		.key = key,
		.src = input,
		.dst = ce_out,
		.len = TEST_DATA_LEN,
		.iv_out = ce_iv_out,
	};

	test_check_title("aes-192-ctr-enc");
	gdn_ctr("aes", input, gdn_out,
		TEST_DATA_LEN, key, 24, 1, iv, gdn_iv_out);

	ce_cipher.enc = 1;
	cesd_cipher_do(&ce_cipher);
	ret |= test_check_result_cipher(&ce_cipher);


	test_check_title("aes-192-ctr-dec");
	gdn_ctr("aes", input, gdn_out,
		TEST_DATA_LEN, key, 24, 0, iv, gdn_iv_out);
	ce_cipher.enc = 0;
	cesd_cipher_do(&ce_cipher);
	ret |= test_check_result_cipher(&ce_cipher);
	return ret;
}

static int test_aes256_ecb(void)
{
	int ret = 0;
	struct ce_cipher ce_cipher = {
		.alg = CE_AES_256_ECB,
		.enc = 1,
		.iv = NULL,
		.key = key,
		.src = input,
		.dst = ce_out,
		.len = TEST_DATA_LEN,
		.iv_out = NULL,
	};

	test_check_title("aes-256-ecb-enc");
	gdn_ecb("aes", input, gdn_out, TEST_DATA_LEN, key, 32, 1);

	ce_cipher.enc = 1;
	cesd_cipher_do(&ce_cipher);
	ret |= test_check_result_cipher(&ce_cipher);


	test_check_title("aes-256-ecb-dec");
	gdn_ecb("aes", input, gdn_out, TEST_DATA_LEN, key, 32, 0);
	ce_cipher.enc = 0;
	cesd_cipher_do(&ce_cipher);
	ret |= test_check_result_cipher(&ce_cipher);
	return ret;
}

static int test_aes256_cbc(void)
{
	int ret = 0;
	struct ce_cipher ce_cipher = {
		.alg = CE_AES_256_CBC,
		.enc = 1,
		.iv = iv,
		.key = key,
		.src = input,
		.dst = ce_out,
		.len = TEST_DATA_LEN,
		.iv_out = ce_iv_out,
	};

	test_check_title("aes-256-cbc-enc");
	gdn_cbc("aes", input, gdn_out,
		TEST_DATA_LEN, key, 32, 1, iv, gdn_iv_out);

	ce_cipher.enc = 1;
	cesd_cipher_do(&ce_cipher);
	ret |= test_check_result_cipher(&ce_cipher);


	test_check_title("aes-256-cbc-dec");
	gdn_cbc("aes", input, gdn_out,
		TEST_DATA_LEN, key, 32, 0, iv, gdn_iv_out);
	ce_cipher.enc = 0;
	cesd_cipher_do(&ce_cipher);
	ret |= test_check_result_cipher(&ce_cipher);
	return ret;
}

static int test_aes256_ctr(void)
{
	int ret = 0;
	struct ce_cipher ce_cipher = {
		.alg = CE_AES_256_CTR,
		.enc = 1,
		.iv = iv,
		.key = key,
		.src = input,
		.dst = ce_out,
		.len = TEST_DATA_LEN,
		.iv_out = ce_iv_out,
	};

	test_check_title("aes-256-ctr-enc");
	gdn_ctr("aes", input, gdn_out,
		TEST_DATA_LEN, key, 32, 1, iv, gdn_iv_out);

	ce_cipher.enc = 1;
	cesd_cipher_do(&ce_cipher);
	ret |= test_check_result_cipher(&ce_cipher);


	test_check_title("aes-256-ctr-dec");
	gdn_ctr("aes", input, gdn_out,
		TEST_DATA_LEN, key, 32, 0, iv, gdn_iv_out);
	ce_cipher.enc = 0;
	cesd_cipher_do(&ce_cipher);
	ret |= test_check_result_cipher(&ce_cipher);
	return ret;
}

static int test_sm4_ecb(void)
{
	int ret = 0;
	struct ce_cipher ce_cipher = {
		.alg = CE_SM4_ECB,
		.enc = 1,
		.iv = NULL,
		.key = key,
		.src = input,
		.dst = ce_out,
		.len = TEST_DATA_LEN,
		.iv_out = NULL,
	};

	test_check_title("sm4-ecb-enc");
	gdn_ecb("sm4", input, gdn_out, TEST_DATA_LEN, key, 16, 1);

	ce_cipher.enc = 1;
	cesd_cipher_do(&ce_cipher);
	ret |= test_check_result_cipher(&ce_cipher);


	test_check_title("sm4-ecb-dec");
	gdn_ecb("sm4", input, gdn_out, TEST_DATA_LEN, key, 16, 0);
	ce_cipher.enc = 0;
	cesd_cipher_do(&ce_cipher);
	ret |= test_check_result_cipher(&ce_cipher);
	return ret;
}

static int test_sm4_cbc(void)
{
	int ret = 0;
	struct ce_cipher ce_cipher = {
		.alg = CE_SM4_CBC,
		.enc = 1,
		.iv  = iv,
		.key = key,
		.src = input,
		.dst = ce_out,
		.len = TEST_DATA_LEN,
		.iv_out = ce_iv_out,
	};

	test_check_title("sm4-cbc-enc");
	gdn_cbc("sm4", input, gdn_out,
		TEST_DATA_LEN, key, 16, 1, iv, gdn_iv_out);

	ce_cipher.enc = 1;
	cesd_cipher_do(&ce_cipher);
	ret |= test_check_result_cipher(&ce_cipher);


	test_check_title("sm4-cbc-dec");
	gdn_cbc("sm4", input, gdn_out,
		TEST_DATA_LEN, key, 16, 0, iv, gdn_iv_out);
	ce_cipher.enc = 0;
	cesd_cipher_do(&ce_cipher);
	ret |= test_check_result_cipher(&ce_cipher);
	return ret;
}

static int test_sm4_ctr(void)
{
	int ret = 0;
	struct ce_cipher ce_cipher = {
		.alg = CE_SM4_CTR,
		.enc = 1,
		.iv = iv,
		.key = key,
		.src = input,
		.dst = ce_out,
		.len = TEST_DATA_LEN,
		.iv_out = ce_iv_out,
	};

	test_check_title("sm4-ctr-enc");
	gdn_ctr("sm4", input, gdn_out,
		TEST_DATA_LEN, key, 16, 1, iv, gdn_iv_out);

	ce_cipher.enc = 1;
	cesd_cipher_do(&ce_cipher);
	ret |= test_check_result_cipher(&ce_cipher);


	test_check_title("sm4-ctr-dec");
	gdn_ctr("sm4", input, gdn_out,
		TEST_DATA_LEN, key, 16, 0, iv, gdn_iv_out);
	ce_cipher.enc = 0;
	cesd_cipher_do(&ce_cipher);
	ret |= test_check_result_cipher(&ce_cipher);
	return ret;
}

#define SHA_CHUNK_SIZE  (512 / 8)
#define ROUND_UP(x, n)   (((long)(x)+(n - 1))/n * n)
#define ROUND_DOWN(x, n) (((long)(x))/n * n)

static void *sha_padding(
			  void *msg,
			  unsigned long size,
			  unsigned long *output_size)
{
	//count in bytes
	unsigned char *p = msg + size;
	unsigned long new_size;

	if (size % SHA_CHUNK_SIZE + (8 + 64) / 8 > SHA_CHUNK_SIZE) {
		//2 extra chunk is needed
		new_size =
			ROUND_DOWN(size, SHA_CHUNK_SIZE) + 2 * SHA_CHUNK_SIZE;
	} else {
		//1 extra chunk is enough
		new_size = ROUND_DOWN(size, SHA_CHUNK_SIZE)+SHA_CHUNK_SIZE;
	}
	unsigned char *q = msg + new_size - 64 / 8;

	*p++ = 0x80; //0b 1000 - 0000
	while (p < q)
		*p++ = 0x00;
	//padding message size
	//unsigned long 64bit to big endian bit stream
	ce_u64_t tmp = size * 8;
	*p++ = (tmp>>56) & 0xff;
	*p++ = (tmp>>48) & 0xff;
	*p++ = (tmp>>40) & 0xff;
	*p++ = (tmp>>32) & 0xff;
	*p++ = (tmp>>24) & 0xff;
	*p++ = (tmp>>16) & 0xff;
	*p++ = (tmp>>8) & 0xff;
	*p = (tmp>>0) & 0xff;
	if (output_size) {
		*output_size = new_size;
		ce_assert(*output_size % SHA_CHUNK_SIZE == 0);
	}
	return msg;
}

static int test_sha1(void)
{
	unsigned long output_size;

	sha_padding(msg, TEST_MSG_LEN, &output_size);
	unsigned char sha1_init_param[] = {
		0x67, 0x45, 0x23, 0x01,
		0xEF, 0xCD, 0xAB, 0x89,
		0x98, 0xBA, 0xDC, 0xFE,
		0x10, 0x32, 0x54, 0x76,
		0xC3, 0xD2, 0xE1, 0xF0,
	};

	int ret = 0;
	struct ce_hash ce_hash = {
		.alg	= CE_SHA1,
		.init	= sha1_init_param,
		.src	= msg,
		.len	= output_size,
		.hash	= ce_dgst,
	};

	test_check_title("sha1");
	cesd_hash_do(&ce_hash);
	gdn_sha1(msg, TEST_MSG_LEN, gdn_dgst);
	ret |= test_check_result_hash(&ce_hash);
	return ret;
}

static int test_sha256(void)
{
	unsigned long output_size;

	sha_padding(msg, TEST_MSG_LEN, &output_size);
	unsigned char sha256_init_param[] = {
		0x6a, 0x09, 0xe6, 0x67,
		0xbb, 0x67, 0xae, 0x85,
		0x3c, 0x6e, 0xf3, 0x72,
		0xa5, 0x4f, 0xf5, 0x3a,
		0x51, 0x0e, 0x52, 0x7f,
		0x9b, 0x05, 0x68, 0x8c,
		0x1f, 0x83, 0xd9, 0xab,
		0x5b, 0xe0, 0xcd, 0x19,
	};

	int ret = 0;
	struct ce_hash ce_hash = {
		.alg	= CE_SHA256,
		.init	= sha256_init_param,
		.src	= msg,
		.len	= output_size,
		.hash	= ce_dgst,
	};

	test_check_title("sha256");
	cesd_hash_do(&ce_hash);
	gdn_sha256(msg, TEST_MSG_LEN, gdn_dgst);
	ret |= test_check_result_hash(&ce_hash);
	return ret;
}

static int test_base64(void)
{
	unsigned long original_len = TEST_DATA_LEN / 4 * 3;
	unsigned long encoded_len = original_len / 3 * 4;
	ce_assert(original_len <= TEST_DATA_LEN);
	ce_assert(encoded_len <= TEST_DATA_LEN);
	int ret = 0;
	struct ce_base ce_base = {
		.alg = CE_BASE64,
		.enc = 1,
		.src = input,
		.dst = ce_out,
		.len = original_len,
	};
	memset(gdn_out, 0x00, TEST_DATA_LEN);
	memset(ce_out, 0x00, TEST_DATA_LEN);

	test_check_title("base64-encode");
	ce_base.enc = 1;
	cesd_base_do(&ce_base);
	base64_encode(gdn_out, input, original_len, base64std);
	ret |= test_check_result_base();


	memcpy(input, ce_out, TEST_DATA_LEN);
	memset(gdn_out, 0x00, TEST_DATA_LEN);
	memset(ce_out, 0x00, TEST_DATA_LEN);

	test_check_title("base64-decode");
	ce_base.enc = 0;
	ce_base.len = encoded_len;
	cesd_base_do(&ce_base);
	base64_decode(gdn_out, input, encoded_len, base64std);
	ret |= test_check_result_base();

	//refill input with random
	xfill(input, TEST_DATA_LEN);

	return ret;
}

static int test_secure_key(void)
{
	int ret = 0;
	struct ce_cipher ce_cipher = {
		.alg = CE_AES_256_ECB,
		.enc = 1,
		.iv = NULL,
		.key = NULL,
		.src = input,
		.dst = ce_out,
		.len = TEST_DATA_LEN,
		.iv_out = NULL,
	};

	int secure_key_valid = cesd_is_secure_key_valid();

	if (!secure_key_valid) {
		printf("programming secure key...\n");
		int key_valid_program_success;
		int key_program_success;
		key_valid_program_success = program_secure_key_enable();
		program_secure_key(secure_key);

		if (!key_valid_program_success) {
			printf("program failed\n");
			efuse_dump(1, 1);
			while(1)
				;
		}

		printf("secure key program done, reboot and run again\n");
		printf("MAKE SURE YOUR PLATFORM WILL NOT RESET EFUSE DATA\n"
		       "AFTER CHIP RESET\n");

		while(1)
			;
	}


	test_check_title("secure-key-enc");
	if (secure_key_valid) {
		gdn_ecb("aes", input, gdn_out, TEST_DATA_LEN, secure_key,
			32, 1);

		ce_cipher.enc = 1;
		cesd_cipher_do(&ce_cipher);
		ret |= test_check_result_cipher(&ce_cipher);
	} else {
		printf("[%s]\t%s\n", "*NA*", "SECURE KEY INVALID");
	}


	test_check_title("secure-key-dec");
	if (secure_key_valid) {
		gdn_ecb("aes", input, gdn_out, TEST_DATA_LEN, secure_key,
			32, 0);
		ce_cipher.enc = 0;
		cesd_cipher_do(&ce_cipher);
		ret |= test_check_result_cipher(&ce_cipher);
	} else {
		printf("[%s]\t%s\n", "*NA*", "SECURE KEY INVALID");
	}
	return ret;
}

typedef int (*test_func_t)(void);

static test_func_t test_func_table[] = {
	/*test_secure_key, */
	test_des_ecb,
	test_des_cbc,
	test_des_ctr,
	test_des_ede3_ecb,
	test_des_ede3_cbc,
	test_des_ede3_ctr,
	test_aes128_ecb,
	test_aes128_cbc,
	test_aes128_ctr,
	test_aes192_ecb,
	test_aes192_cbc,
	test_aes192_ctr,
	test_aes256_ecb,
	test_aes256_cbc,
	test_aes256_ctr,
	test_sm4_ecb,
	test_sm4_cbc,
	test_sm4_ctr,
	test_sha1,
	test_sha256,
	test_base64,
};

extern int cesd_self_test(void);

#ifndef BUILD_TEST_CASE_all
int testcase_main()
{
	int ret = 0;

	printf("crypto engine test\n");
	printf("Press any key to continue\n"); //get a random seed
	int ch = uart_getc();

	printf("crypto engine performing self testing now\n");
	if (cesd_self_test()) {
		printf("crypto engine self test failed\n");
		while (1)
			;
	} else
		printf("crypto engine self test success, continue\n");

	unsigned int tick = timer_get_tick();
	unsigned int seed = tick^((ch<<24)|(ch<<16)|(ch<<8)|ch);

	xseed(seed);
	unsigned int xr = xrand();

	printf("RANDOM CHAR:[%08x] TICK:[%08x] SEED:[0x%08x] RAND:[0x%08x]\n"
	       , ch, tick, seed, xr);
	unsigned char tmp[128];

	memset(tmp, 0x00, sizeof(tmp));
	xfill(tmp, sizeof(tmp));
	//init random
	xfill(input, TEST_DATA_LEN);
	memset(gdn_out, 0x00, TEST_DATA_LEN);
	memset(ce_out, 0x00, TEST_DATA_LEN);
	xfill(key, TEST_KEY_LEN);
	xfill(iv, TEST_IV_LEN);
	//padding message
	xfill(msg, TEST_MSG_LEN);
	gdn_init();
	//init crypto engine

	unsigned int i;

	printf("\n");
	ret = 0;
	for (i = 0;
	     i < ARRAY_SIZE(test_func_table);
	     ++i) {
		//open it every time for clock-gate & reset test
		cesd_init();
		ret |= test_func_table[i]();
		cesd_destroy();
	}
	if (i > 1) {
		test_check_title("clock-gate-reset");
		printf("[%s]\n", ret ? "FAIL" : "PASS");
	}
	printf("\n");
	test_check_title("FINAL TEST RESULT");
	printf("[%s]\n", ret ? "FAIL" : "PASS");

	cesd_destroy();
	return ret;
}
#endif
