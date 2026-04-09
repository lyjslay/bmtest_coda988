/**********************************************************************
 * Copyright (C) 2014-2015 Cadence Design Systems, Inc. * All rights reserved worldwide.
 ***********************************************************************
 * main.c
 *
 * Cadence USB Core Driver
 * main component function
 ***********************************************************************/

#include <stdint.h>
#include <system_common.h>
#include <tests.h>
#include <mmio.h>
#include <timer.h>
#include <log.h>
#include <hpnfc_core.h>
#include <string.h>
#include <assert.h>
#include <string.h>
#include "boot_test.h"
#include "uart.h"
#include "system_common.h"
#include "mmio.h"
#include "bm_nfc.h"
#include "mmc.h"

extern const char fw_version_info[];
/*
    0x70
        BIT[31:16] : nr pages in a single block, dd_page_per_block
        BIT[15:00] : pages size, dd_page_size

    0x74
        BIT[31:16] :	ECC sector size
        BIT[15:12] :	Rsvd
        BIT[11] :		1 to ignore CRC checking after reading of parameter page during DD
        BIT[10] :		1 for 4 address cycle
        BIT[9:6] :		number of LUNs
        BIT[5] :		dd_ack
        BIT[4] :		discovery_inhibit
        BIT[3:1] :		bot_ecc_corr_str
        BIT[0] :		boot_ecc_enable

typedef struct _nand_support_para_t {
    uint8_t id[8];

    uint32_t dd_reg_70_value;
    uint32_t dd_reg_74_value;

    uint16_t mainSize;
    uint16_t spareSize;
    uint16_t lunNumber;
    uint16_t pagesPerBlock;
    uint16_t blocksPerLun;
    uint16_t numberOfPlanes;
    uint16_t blockShift;
} nand_support_para_t;
*/
nand_support_para_t nand_support_list[] =
{
    /*  pg cnt 64, pg sz 2048.  spare sz 64, 4 addr cycle */
    {{0xec, 0xf1, 0x00, 0x95, 0x42, 0, 0, 0}, 0x400800, 0x2000C61, 2048, 64, 1, 64, 1024, 1, 6},  // K9F1G08U0F, id done, not verified
    /*  pg cnt 64, pg sz 2048.  spare sz 64, 5 addr cycle */
    {{0xec, 0xda, 0x10, 0x95, 0x46, 0, 0, 0}, 0x400800, 0x2000861, 2048, 64, 1, 64, 2048, 1, 6},  // K9F2G08U0D, id done
    /*  pg cnt 64, pg sz 2048.  spare sz 64, 5 addr cycle */
    {{0xec, 0xdc, 0x10, 0x95, 0x56, 0, 0, 0}, 0x400800, 0x2000861, 2048, 64, 1, 64, 4096, 1, 6},  // K9F4G08U0F, id done

    /*  pg cnt 64, pg sz 2048. spare sz 128, 4 addr cycle */
    {{0x98, 0xf1, 0x80, 0x15, 0x72, 0, 0, 0}, 0x400800, 0x2000C61, 2048, 128, 1, 64, 1024, 1, 6}, // TC58NVG0S3HTA00, id done, verified done
    /*  pg cnt 64, pg sz 2048. spare sz 128, 5 addr cycle */
    {{0x98, 0xda, 0x90, 0x15, 0x76, 0, 0, 0}, 0x400800, 0x2000861, 2048, 128, 1, 64, 2048, 1, 6}, // TC58NVG1S3HTA00, id done, not verified
    /*  pg cnt 64, pg sz 4096. spare sz 256, 5 addr cycle, block count 2048 !!!!!!!!!!!!!!!!!!!!!!!!!!! */
    // {{0x98, 0xdc, 0x90, 0x26, 0x76, 0, 0, 0}, 0x400800, 0x2000861, 4096, 256, 1, 64, 2048, 1, 6}, // TC58NVG2S0HTA00, id done
    // {{0x18, 0xdc, 0x90, 0x26, 0x76, 0, 0, 0}, 0x400800, 0x2000861, 4096, 256, 1, 64, 2048, 1, 6}, // TC58NVG2S0HTA00, id done, testing change id for not matching

    /*  pg cnt 64, pg sz 2048.  spare sz 64, 5 addr cycle, block count 2048*/
    {{0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff}, 0x400800, 0x2000861, 2048, 64, 1, 64, 1024, 1, 6},  // general setting
};

/* level, counter, state: */
uint32_t g_dbg_log_lvl = DBG_CRIT;
uint32_t g_dbg_log_cnt = 0;
uint32_t g_dbg_state = 0;
//static uint32_t dbg = 1;

static uint8_t nand_id[8];

static void enable_access_ddr(uint32_t enable);

static void grab_nand_id(void)
{
    uint32_t tmp;

    tmp = mmio_read_32(TOP_REG_NFC_ID_VALUE);
    printf("top reg 78 0x%x\n", tmp);
    //nand_id = ((tmp & 0xff0000) << 16) | (tmp & 0xFF000000);
    nand_id[0] = (tmp & 0xff0000) >> 16;
    nand_id[1] = (tmp & 0xff000000) >> 24;

    tmp = mmio_read_32(TOP_REG_NFC_MISC);
    printf("top reg 7C 0x%x\n", tmp);
    //nand_id |= (tmp & 0xff00) << 8 | (tmp & 0xFF0000) >> 8  | (tmp & 0xFF000000) >> 24;
    nand_id[2] = (tmp & 0xff00) >> 8;
    nand_id[3] = (tmp & 0xff0000) >> 16;
    nand_id[4] = (tmp & 0xff000000) >> 24;

    printf("NAND ID : ");
    for (uint32_t i = 0; i < NAND_ID_NUM; i++)
    {
        printf("%x ", nand_id[i]);
    }

    printf("nr entry %ld\n", sizeof(nand_support_list) / sizeof(nand_support_list[0]));

    for (uint32_t i = 0; i <sizeof(nand_support_list) / sizeof(nand_support_list[0]); i++)
    {
        if (memcmp(nand_id, nand_support_list[i].id, 8) == 0)
        {
            printf("match %d\n", i);
        }
    }
    //while (dbg) {}
}

static void modify_device_layout(void)
{
    HPNFC_DeviceLayout *dev = get_dev_layout();

    if (dev->pagesPerBlock != 0 && dev->blocksPerLun != 0) {
        return;
    }

    printf("NAND ID : ");
    for (uint32_t i = 0; i < NAND_ID_NUM; i++)
    {
        printf("%x ", nand_id[i]);
    }
    printf("\n");

    uint32_t nr_entry = sizeof(nand_support_list) / sizeof(nand_support_list[0]);
    uint32_t i;

    for (i = 0; i < nr_entry; i++)
    {
        if (memcmp(nand_id, nand_support_list[i].id, 8) == 0)
        {
            nand_support_para_t *nand = &nand_support_list[i];
            printf("match %d\n", i);
            dev->blocksPerLun = nand->blocksPerLun;
            dev->lunNumber = nand->lunNumber;
            dev->mainSize= nand->mainSize;
            dev->numberOfPlanes= nand->numberOfPlanes;
            dev->pagesPerBlock= nand->pagesPerBlock;
            dev->spareSize = nand->spareSize;
            dev->blockShift = nand->blockShift;
            break;
        }
    }

    if (i == nr_entry) {
        printf("NAND not found, use default value\n");
        nand_support_para_t *nand = &nand_support_list[nr_entry-1];
        dev->blocksPerLun = nand->blocksPerLun;
        dev->lunNumber = nand->lunNumber;
        dev->mainSize= nand->mainSize;
        dev->numberOfPlanes= nand->numberOfPlanes;
        dev->pagesPerBlock= nand->pagesPerBlock;
        dev->spareSize = nand->spareSize;
        dev->blockShift = nand->blockShift;
    }

    print_device_layout();
}

static void print_dd_pins(void)
{
    uint32_t tmp;
    tmp = mmio_read_32(TOP_REG_NFC_DD_PINS);
    printf("top 70 0x%x\n", tmp);

    tmp = mmio_read_32(TOP_REG_NFC_BOOT_REG);
    printf("top 74 0x%x\n", tmp);

    tmp = mmio_read_32(TOP_REG_NFC_ID_VALUE);
    printf("top 78 0x%x\n", tmp);

    tmp = mmio_read_32(TOP_REG_NFC_MISC);
    printf("top 7c 0x%x\n", tmp);
}

static void init_dd_pins(void)
{
    uint32_t tmp;
    printf("[ACT]Set dd pins\n");
    tmp = mmio_read_32(TOP_REG_NFC_DD_PINS);

    mmio_write_32(TOP_REG_NFC_DD_PINS, 0x400800);
    mmio_write_32(TOP_REG_NFC_BOOT_REG, 0x2000861);
    tmp = mmio_read_32(TOP_REG_NFC_DD_PINS);
    printf("2 top 70 0x%x\n", tmp);
    tmp = mmio_read_32(TOP_REG_NFC_BOOT_REG);
    printf("2 top 74 0x%x\n", tmp);
}

void *g_private_data;

static int irq_handler(int irqn, void *priv) {
    HPNFC_Core_Isr(g_private_data);
    return 0;
}

static void enable_access_ddr(uint32_t enable)
{
	uint32_t tmp = mmio_read_32(TOP_REG_NFC_REMAP39_TO_32);
	INFO("org TOP_REG_NFC_REMAP39_TO_32 0x%x\n", tmp);

	tmp &= 0xFFFFFEFF;

	if (enable)
	{
		tmp |= 0x1 << 8;
		mmio_write_32(TOP_REG_NFC_REMAP39_TO_32, tmp);
	}
	else
	{
		mmio_write_32(TOP_REG_NFC_REMAP39_TO_32, tmp);
	}
	INFO("new TOP_REG_NFC_REMAP39_TO_32 0x%x\n", tmp);

	return;
}

int testcase_nfc()
{
	uint32_t delay_cnt = 0;
    int rbs;// = mmio_read_32(0x500C1004);
    int ctrls;// = mmio_read_32(NFC_REG_CTRL_STATUS);
    //uint32_t addr = TOP_RESET_REG;

    printf("fw_version_info: %s\n", fw_version_info);
    printf("built time: %s %s\n", __DATE__, __TIME__);

    request_irq(NFC_INTR, irq_handler, 0, NULL, NULL);

    printf("testcase nfc \n");

    printf("enable ddr access \n");
    enable_access_ddr(1);

#if defined(BOARD_FPGA)
    printf("BOARD_FPGA\n");
    mmio_clrbits_32(TOP_RESET_REG, BIT(12)); /* NFC IP reset low*/
    timer_mdelay(1);
    pinmux_config(PINMUX_NAND);
    timer_mdelay(1);
    mmio_setbits_32(TOP_RESET_REG, BIT(12)); /* NFC IP reset high*/
    timer_mdelay(1);
#elif defined(BOARD_PALLADIUM)
    printf("BOARD_PALLADIUM\n");
    mmio_clrbits_32(TOP_RESET_REG, BIT(12)); /* NFC IP reset low*/
    timer_udelay(1);
    pinmux_config(PINMUX_NAND);
    timer_udelay(1);
    mmio_setbits_32(TOP_RESET_REG, BIT(12)); /* NFC IP reset high*/
#elif
    printf("error platform setting\n");
#endif

    printf("After reset IP and set pinmux\n");
    rbs = mmio_read_32(NFC_REG_RNB_STATUS);
    printf("rbs 0x%x\n", rbs);
    ctrls = mmio_read_32(NFC_REG_CTRL_STATUS);
    printf("ctrls 0x%x\n", ctrls);

	while (ctrls & NFC_REG_CTRL_STATUS_BUSY) {
		if (delay_cnt++ > MAX_NAND_PROBE_DELAY_COUNT) {
			break;
		}
		udelay(NAND_PROBE_DELAY_TIME);
		ctrls = mmio_read_32(NFC_REG_CTRL_STATUS);
	}

	printf("delay_cnt %d\n", delay_cnt);

    if (ctrls & NFC_REG_CTRL_STATUS_BUSY)
    {
        printf("Auto discover fail, try to manually init \n");
        init_dd_pins();
        print_dd_pins();
    }

    void *privateData = InitDriver(NFC_REG_BASE, NFC_SDMA_REG_BASE);
    g_private_data = privateData;
    grab_nand_id();
    modify_device_layout();

    int result = 0;

#if 1
    result = test_plan1_verify_cdma_mode(privateData);
    printf("test_plan1_verify_cdma_mode result %d\n", result);

    result = test_plan2_verify_pio_mode(privateData);
    printf("test_plan2_verify_pio_mode result %d\n", result);

    result = test_plan3_verify_generic_work_mode(privateData, 40);
    printf("test_plan3_verify_generic_work_mode %d\n", result);

    result = test_plan4_verify_uecc_error_report(privateData);
    printf("test_plan4_verify_uecc_error_report result %d\n", result);

    result = test_plan5_verify_desc_error(privateData);
    printf("test_plan5_verify_desc_error %d\n", result);

    result = test_plan9_verify_ecc_engine(privateData);
    printf("test_plan9_verify_ecc_engine result %d\n", result);

    /* NAND boot can't be auto test need to by tested manually */
//    result = test_plan10_verify_nand_boot(privateData);
//    printf("test_plan10_verify_nand_boot result %d\n", result);

    result = test_plan11_verify_status_polling(privateData);
    printf("test_plan11_verify_status_polling result %d\n", result);

    /* test plan12 is testing intr and included in previous test case */

    /* Timing can't be auto test need to by tested manually */
//    result = test_plan13_verify_timing_regs(privateData);
//    printf("test_plan13_verify_timing_regs result %d\n", result);

    /* test plan14 is testing sdma and included in previous test case */
    /* test plan15 can't be auto test need to by tested manually */
#endif

#if 1
    /* test plan6, 7 and 8 is merged here */
    result = test_plan6_verify_reset_type0(privateData);
    printf("test_plan6_verify_reset_type0 result %d\n", result);
    mmio_clrbits_32(TOP_RESET_REG, BIT(12)); /* NFC IP reset low*/
    timer_mdelay(1);
    pinmux_config(4);
    timer_mdelay(1);
    mmio_setbits_32(TOP_RESET_REG, BIT(12)); /* NFC IP reset high*/
    timer_mdelay(1);
    InitDriver(NFC_REG_BASE, NFC_SDMA_REG_BASE);

    timer_mdelay(30);
    result = test_plan1_verify_cdma_mode(privateData);
    printf("test_plan1_verify_cdma_mode result %d\n", result);
#endif

    /* Below functions are for misc use, not in test plan */
//    result = test_plan16_prg_vec_and_fip(privateData);
//    printf("test_plan16_prg_vec_and_fip result %d\n", result);

//    result = test_plan17_prg_all_nand(privateData);
//    printf("test_plan17_prg_all_nand result %d\n", result);

//    result = test_plan18_kernel_related_testing(privateData);
//    printf("test_plan18_kernel_related_testing result %d\n", result);

    return 0;
}
module_testcase("1", testcase_nfc);
