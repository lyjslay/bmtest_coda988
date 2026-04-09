#ifndef __CESD_H__
#define __CESD_H__

#include <ce.h>

int cesd_init(void);
int cesd_destroy(void);
int cesd_power(int power);
int cesd_cipher_do(struct ce_cipher *cipher);
int cesd_hash_do(struct ce_hash *hash);
int cesd_base_do(struct ce_base *base);
int cesd_is_secure_key_valid(void);
unsigned long long cesd_throughput(void);

#endif
