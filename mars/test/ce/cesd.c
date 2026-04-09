#include "system_common.h"
#include "timer.h"
#include <ce.h>
#include <cache.h>
#include <ce_port.h>

static void *dev = (void *)SPACC_BASE;
static long long throughput;

static unsigned long long counter_get(void);
static unsigned long long counter_hz(void);

#define wait_for_done(func, arg) \
	({ \
		int err; \
		unsigned long long counter = 0; \
		err = func(dev, arg); \
		if (err == 0) { \
			counter = counter_get(); \
			while (ce_status(dev)) \
				; \
			counter = counter_get() - counter; \
			ce_int_clear(dev); \
			throughput = 0; \
			if (counter) \
				throughput = \
				arg->len * counter_hz() / counter; \
			else \
				ce_warn("data too short\n"); \
		} \
		err; \
	 })

static unsigned long long counter_get(void)
{
	return timer_get_tick();
}

static unsigned long long counter_hz(void)
{
	return CONFIG_TIMER_FREQ;
}

static int power(int power)
{
#if 0
	if (power) {
		TOP_CLOCK_ENABLE_1 |= 1 << 17;		//enable clock
		TOP_SOFT_RESET_1 |= 1 << 16;		//release reset
	} else {
		TOP_CLOCK_ENABLE_1 &= ~(1 << 17);	//disable clock
		TOP_SOFT_RESET_1 &= ~(1 << 16);		//deactive reset
	}
#endif
	return 0;
}

int cesd_init(void)
{
	power(1);
	return ce_init(dev);
}
int cesd_destroy(void)
{
	ce_destroy(dev);
	power(0);
	return 0;
}

int cesd_cipher_do(struct ce_cipher *cipher)
{
	int err;

	flush_dcache_range((unsigned long)cipher->src,
			   (unsigned long)cipher->src + cipher->len);
	/* invalidate_dcache_range in bmtest environment means
	 * 1. flush cache line to main memory
	 * 2. invalidate this cache line
	 * for none cache line size aligned start address or end address
	 * it will
	 * 1. flush from an aligned address less than start address
	 * 2. end up with an aligned address greater than end address
	 *
	 * so we need invalidate output buffer before dma trasaction
	 * then invalidate output buffer again after dma trascation
	 */
	invalidate_dcache_range((unsigned long)cipher->dst,
				(unsigned long)cipher->dst + cipher->len);
	err = wait_for_done(ce_cipher_do, cipher);
	invalidate_dcache_range((unsigned long)cipher->dst,
				(unsigned long)cipher->dst + cipher->len);
	ce_save_iv(dev, cipher);
	return err;
}
int cesd_hash_do(struct ce_hash *hash)
{
	int err;
	flush_dcache_range((unsigned long)hash->src,
			   (unsigned long)hash->src + hash->len);
	err = wait_for_done(ce_hash_do, hash);
	ce_save_hash(dev, hash);
	return err;
}
int cesd_base_do(struct ce_base *base)
{
	int err;

	flush_dcache_range((unsigned long)base->src,
			   (unsigned long)base->src + base->len);
	/* invalidate_dcache_range in bmtest environment means
	 * 1. flush cache line to main memory
	 * 2. invalidate this cache line
	 * for none cache line size aligned start address or end address
	 * it will
	 * 1. flush from an aligned address less than start address
	 * 2. end up with an aligned address greater than end address
	 *
	 * so we need invalidate output buffer before dma trasaction
	 * then invalidate output buffer again after dma trascation
	 */
	invalidate_dcache_range((unsigned long)base->dst,
				(unsigned long)base->dst + ce_base_len(base));
	err = wait_for_done(ce_base_do, base);
	invalidate_dcache_range((unsigned long)base->dst,
				(unsigned long)base->dst + ce_base_len(base));
	return err;
}
int cesd_is_secure_key_valid(void)
{
	return ce_is_secure_key_valid(dev);
}
unsigned long long cesd_throughput(void)
{
	return throughput;
}

int cesd_self_test()
{
	unsigned char key[] = {
		0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
	};
	char *plaintext = "BE-HAPPY";
	unsigned char cipher[] = {
		0x29, 0x8f, 0x68, 0x62, 0xc4, 0x87, 0xd5, 0xb7,
	};
	char output[8] = {0};

	cesd_init();

	struct ce_cipher ce_cipher = {
		.alg = CE_DES_ECB,
		.enc = 1,
		.iv = NULL,
		.key = key,
		.src = plaintext,
		.dst = output,
		.len = 8,
		.iv_out = NULL,
	};

	ce_cipher.enc = 1;
	cesd_cipher_do(&ce_cipher);
	if (memcmp(output, cipher, 8)) {
		ce_err("encryption failed\n");
		while (1)
			;
	}
	ce_info("encryption self test pass\n");

	ce_cipher.enc = 0;
	ce_cipher.src = cipher;
	cesd_cipher_do(&ce_cipher);
	if (memcmp(output, plaintext, 8)) {
		ce_err("decryption failed\n");
		while (1)
			;
	}
	ce_info("descryption self test pass\n");

	cesd_destroy();

	return 0;
}

