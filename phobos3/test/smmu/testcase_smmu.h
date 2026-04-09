#ifndef _TESTCASE_UART_H_
#define _TESTCASE_UART_H_

int testcase_smmu(void);

#define SUBSYS_OFFSET (0x08000000)
#define SMMU_BASE_ADDRESS (SUBSYS_OFFSET)
#define MMU_PTW_OFFSET (0x00020000)
#define MMU_LAT1_OFFSET (0x00021000)
#define MMU_LAT2_OFFSET (0x00022000)
/* not used // #define MMU_LAT3_OFFSET (0x00023000) */
#define MMU_LAT4_OFFSET (0x00024000) //TPU
#define MMU_LAT5_OFFSET (0x00025000)

// SMMU BASE ADDRESS
#define SMMU_PTW_BASE  (SMMU_BASE_ADDRESS + MMU_PTW_OFFSET)
#define SMMU_LAT1_BASE (SMMU_BASE_ADDRESS + MMU_LAT1_OFFSET)
#define SMMU_LAT2_BASE (SMMU_BASE_ADDRESS + MMU_LAT2_OFFSET)
#define SMMU_LAT4_BASE (SMMU_BASE_ADDRESS + MMU_LAT4_OFFSET)
#define SMMU_LAT5_BASE (SMMU_BASE_ADDRESS + MMU_LAT5_OFFSET)

// SMMU register defintion
// MMU_PTW
#define PTW_VERSION             0x0100
#define PTW_CONTROL             0x0104
    #define PTW_EN_BIT                  0x00000001
    #define PTW_INV_ALL_BIT             0x00000002
    #define PTW_PI_START_BIT            0x00000010
    #define PTW_PI_DONE_BIT             0x00000020
    #define PTW_PI_CLEAR_BIT            0x00000040
    #define PTW_PREFETCH_DIS_BIT        0x00000100
#define PTW_TTBR                0x0108
#define PTW_PI_START_VPN        0x0180
#define PTW_PI_END_VPN          0x0184

#define PTW_PMU_CTRL            0x0300
    #define PTW_PMU_START_BIT           0x00000001
    #define PTW_PMU_CLEAR_BIT           0x00000002
    #define PTW_PMU_PMU0SEL             0x000000F0
    #define PTW_PMU_PMU1SEL             0x00000F00
#define PTW_PMU0_RESULT         0x0304
#define PTW_PMU1_RESULT         0x0308

// MMU_LAT
#define LAT_VERSION             0x0000
#define LAT_CONTROL             0x0004
    #define PTW_LAT_START_BIT           0x00000001
    #define PTW_LAT_CLEAR_BIT           0x00000002
#define LAT_BYPASS_ID_0         0x0010
#define LAT_BYPASS_ID_0_MASK    0x0014
    #define LAT_EN_BIT                  0x00000001
    #define LAT_UTLB_INV_BIT            0x00000002
    #define LAT_BPSS_EN_BIT             0x00000004
    #define LAT_PI_START_BIT            0x00000010
    #define LAT_MASTER_ID_BITS          0xF0000000
#define LAT_PI_START_VPN        0x0080
#define LAT_PI_END_VPN          0x0084
#define LAT_PMU_CONTROL         0x0100
#define LAT_PMU0_RESULT         0x0104
#define LAT_PMU1_RESULT         0x0108
#define LAT_AR_UTLB_ENTRY_VALID 0x0140
#define LAT_AW_UTLB_ENTRY_VALID 0x0144

struct SmmuPageTable
{
    unsigned char byte0;
    unsigned char byte1;
    unsigned char byte2;
    unsigned char byte3;
    unsigned char byte4;
    unsigned char byte5;
    unsigned char byte6;
    unsigned char byte7;
/*
unsigned bValid : 1;
unsigned bReadable : 1;
unsigned bWriteable : 1;
unsigned bExecutable : 1;
unsigned bUserModeAccessible : 1;
unsigned bGlobal : 1;
unsigned bA : 1;
unsigned bD : 1;
unsigned Reserved : 2;
unsigned PFN0_9bits : 9;
unsigned PFN1_15bits : 15;
unsigned Reserved_30bits : 1;
*/
};

#endif
