#ifndef _TESTCASE_EFUSE_H_
#define _TESTCASE_EFUSE_H_

#define EFUSE_BASE			0x50028000

/*
 * eFuse definitions
 */
#define REG_EFUSE_MODE			0x0
#define REG_EFUSE_ADR			0x4
#define REG_EFUSE_RD_DATA		0xc
#define EFUSE_NUM_ADDRESS_BITS		7
#define EFUSE_MAGIC_BEGIN		0x0b0b0b0b
#define EFUSE_MAGIC_END			0x0e0e0e0e
#define SPIFP_MAGIC_BEGIN		0x1b1b1b1b
#define SPIFP_MAGIC_END			0x1e1e1e1e
#define USERCONF_MAGIC_BEGIN		0x2b2b2b2b
#define USERCONF_MAGIC_END		0x2e2e2e2e

#endif  /* _TESTCASE_EFUSE_H_ */
