#ifndef _TESTCASE_TRNG_H_
#define _TESTCASE_TRNG_H_
#define BIT(n) (UINT32_C(1) << (n)) 

#define TRNG_RAND0_REG_ADDR 0x24
#define TRNG_DATA_WIDTH 32
#define TRNG_ADDR_WIDTH 8 

#define GetTrngAddr(addrOff)  (addrOff+TRNG_BASE)


int testcase_trng(void);

#endif
