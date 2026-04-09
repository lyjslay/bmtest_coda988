#ifndef _TESTCASE_I2C_H_
#define _TESTCASE_I2C_H_

#define BASE_I2C0       0x5001A000
#define BASE_I2C1       0x5001C000
#define BASE_I2C2       0x5001E000
#define BASE_I2C3       0x50020000
#define BASE_I2C4       0x50032000
#define BASE_PINMUX     0x50010400

#define REG_CON         0x0
#define REG_TAR         0x4
#define REG_SAR         0x8
#define REG_DATA_CMD    0x10
#define REG_SS_HCNT     0x14
#define REG_SS_LCNT     0x18
#define REG_FS_HCNT     0x1C
#define REG_FS_LCNT     0x20
#define REG_HS_HCNT     0x24
#define REG_HS_LCNT     0x28
#define REG_INT_STS     0x2C
#define REG_INT_MASK    0x30
#define REG_RAW_INT_STS 0x34
#define REG_RX_TL       0x38
#define REG_TX_TL       0x3C
#define REG_ENABLE      0x6C
#define REG_STATUS      0x70

#define ADDR_MODE_32    0x23
#define ADDR_MODE_40    0x24

#define RX_FULL         (1<<2)
#define TX_EMPTY        (1<<4)
#define RFNE            (1<<3)

#endif  /* _TESTCASE_I2C_H_ */
