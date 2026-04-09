#ifndef _TESTCASE_EFUSE_H_
#define _TESTCASE_EFUSE_H_
#define csi_error_t int
#define CSI_OK      0
#define CSI_ERROR   -1
#define EFUSE_BUSY (1 << 0)

#define EFUSE_BASE 0x03050000
#define SE_SYS_CTL_FOR_EFUSE_BASE	0x020C0000

#define EFUSE_MODE    0x000
#define EFUSE_ADR     0x004
#define EFUSE_DIR_CMD 0x008
#define EFUSE_RD_DATA 0x00c
#define EFUSE_STATUS  0x010
#define EFUSE_ONE_WAY     0x014
#define EFUSE_OP_PSSWD_ENTRY 0x024

// #define EFUSE_FTSN0   0x100
// #define EFUSE_FTSN1   0x104
// #define EFUSE_SW_INFO 0x12c
// #define EFUSE_W_LOCK0 0x198
// #define EFUSE_W_LOCK1 0x19c
// #define SCS_config    0x1a0
/*
 * eFuse definitions
 */
#define EFUSE_NUM_ADDRESS_BITS 7
#define EFUSE_MAGIC_BEGIN 0x0b0b0b0b
#define EFUSE_MAGIC_END 0x0e0e0e0e
#define SPIFP_MAGIC_BEGIN 0x1b1b1b1b
#define SPIFP_MAGIC_END 0x1e1e1e1e
#define USERCONF_MAGIC_BEGIN 0x2b2b2b2b
#define USERCONF_MAGIC_END 0x2e2e2e2e

/*
*hardmacro
*/
#define eFuse_sw_info 11
#define eFuse_rompatch_addr0 33
#define eFuse_rompatch_data0 34

#define eFuse_w_lock0 38
#define eFuse_w_lock1 39

#define secureRegionCTRL 62

/*
 * sec eFuse
*/
#define OP_PORT 0x00
#define OP_PASSWD 0x04
#define SEC_INFO0 0x08
#define SEC_INFO1 0x0c
#endif /* _TESTCASE_EFUSE_H_ */
