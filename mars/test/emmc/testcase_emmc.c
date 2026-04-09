#include <stdint.h>
#include <assert.h>
#include <string.h>
#include "boot_test.h"
#include "uart.h"
#include "system_common.h"
#include "mmio.h"
#include "bm_sdhci.h"
#include "mmc.h"

typedef int (*mmc_thandler_t)(struct mmc_host *host);

struct mmc_testcase {
	mmc_thandler_t handler;
	char *name;
};

static int mmc_phy_init(struct mmc_host *host);
static int mmc_pio_test(struct mmc_host *host);
static int mmc_sdma_test(struct mmc_host *host);
static int mmc_adma2_test(struct mmc_host *host);
static int mmc_adma3_test(struct mmc_host *host);
static int mmc_prog_boot_partition(struct mmc_host *host);
static int mmc_prog_boot_fip(struct mmc_host *host);
static int mmc_interrupt_test(struct mmc_host *host);
static int mmc_tuning_test(struct mmc_host *host);
static int mmc_1_4_line_transfer_test(struct mmc_host *host);
static int mmc_hs200_test(struct mmc_host *host);
static int mmc_identify_test(struct mmc_host *host);
static int mmc_single_blk_rwe_test(struct mmc_host *host);
static int mmc_multi_blk_rwe_test(struct mmc_host *host);
static int mmc_register_rw_test(struct mmc_host *host);

char rbuf[65535]__attribute__((aligned (SD_BLOCK_SIZE)));
static struct mmc_testcase emmc_testcases[] = {
#ifndef BUILD_TEST_CASE_SLT_emmc
	{ .handler = mmc_register_rw_test, .name = "Register read/write test" },
	{ .handler = mmc_phy_init, .name = "PHY initialization test2" },
#endif /* ndef BUILD_TEST_CASE_SLT_emmc */
	{ .handler = mmc_identify_test, .name = "Card identification mode test" },
	{ .handler = mmc_1_4_line_transfer_test, .name = "1 and 4 dat lines transfer test" },
	{ .handler = mmc_pio_test, .name = "PIO test" },
	{ .handler = mmc_sdma_test, .name = "SDMA test" },
	{ .handler = mmc_adma2_test, .name = "ADMA2 test" },
	{ .handler = mmc_adma3_test, .name = "ADMA3 test" },
	{ .handler = mmc_prog_boot_partition, .name = "prog fip.bin to emmc boot partition" },

#ifndef BUILD_TEST_CASE_SLT_emmc
	{ .handler = mmc_interrupt_test, .name = "Interrupt test" },
#endif /* ndef BUILD_TEST_CASE_SLT_emmc */
#ifdef BOARD_ASIC
//	{ .handler = mmc_tuning_test, .name = "Tuning test" },
//	{ .handler = mmc_hs200_test, .name = "HS200 test" },
#endif
	{ .handler = mmc_single_blk_rwe_test, .name = "Single block read/write test" },
	{ .handler = mmc_multi_blk_rwe_test, .name = "Multiple block read/write test" },
	{ .handler = mmc_prog_boot_fip, .name = "prog fip.bin to emmc boot partition" },
};
static void dump_hex(char *desc, void *addr, int len)
{
	int i;
	unsigned char buff[17];
	unsigned char *pc = (unsigned char *)addr;

	/* Output description if given. */
	if (desc != NULL)
		printf("%s:\n", desc);

	/* Process every byte in the data. */
	for (i = 0; i < len; i++) {
		/* Multiple of 16 means new line (with line offset). */
		if ((i % 16) == 0) {
			/* Just don't print ASCII for the zeroth line. */
			if (i != 0)
				printf("  %s\n", buff);

			/* Output the offset. */
			printf("  %x ", i);
		}

		/* Now the hex code for the specific character. */
		printf(" %x", pc[i]);

		/* And store a printable ASCII character for later. */
		if ((pc[i] < 0x20) || (pc[i] > 0x7e))
			buff[i % 16] = '.';
		else
			buff[i % 16] = pc[i];
		buff[(i % 16) + 1] = '\0';
	}

	/* Pad out last line if not exactly 16 characters. */
	while ((i % 16) != 0) {
		printf("   ");
		i++;
	}

	/* And print the final ASCII bit. */
	printf("  %s\n", buff);
}

static int bm_mmc_get_clk()
{
	int clk;
#ifdef BOARD_FPGA
	printf("FPGA xtal pll setting 0x%x\n", mmio_read_32(0x3002030));

	clk = 25 * 1000 * 1000;
	printf("eMMC CLK is %d in FPGA\n", clk);

	return clk;
#elif defined(BOARD_PALLADIUM)
	mmio_write_32(0x3002030, 0);
	printf("PXP xtal pll setting 0x%x\n", mmio_read_32(0x3002030));

	mmio_write_32(0x3002064, 0x40009);

	clk = 375 * 1000 * 1000;
	printf("eMMC CLK is %d in PXP\n", clk);

	return clk;
#elif defined(BOARD_ASIC)
	mmio_write_32(0x3002030, 0);
	printf("ASIC xtal pll setting 0x%x\n", mmio_read_32(0x3002030));

	mmio_write_32(0x3002064, 0x40009);

	clk = 375 * 1000 * 1000;
	printf("eMMC CLK is %d in ASIC\n", clk);

	return clk;
#else
#error "BOARD type not defined"
#endif
}

#define EMMC_INIT_FREQ				(400 * 1000)


static void bm_emmc_hw_init(void)
{
	uintptr_t base, vendor_base;

	base = EMMC_BASE;
//    vendor_base = base + (mmio_read_16(base + P_VENDOR_SPECIFIC_AREA) & ((1<<12)-1));
//    bm_params.vendor_base = vendor_base;

    // deasset reset of phy
    /* mmio_write_32(base + SDHCI_P_PHY_CNFG, mmio_read_32(base + SDHCI_P_PHY_CNFG) | (1 << PHY_CNFG_PHY_RSTN)); */

    // reset data & cmd
    mmio_write_8(base + SDHCI_SOFTWARE_RESET, 0x6);

    // init common parameters
    mmio_write_8(base + SDHCI_PWR_CONTROL, SDHCI_BUS_VOL_VDD1_1_8V);
    mmio_write_8(base + SDHCI_TOUT_CTRL, 0xe);  // for TMCLK 50Khz
    mmio_write_16(base + SDHCI_HOST_CONTROL2,
                  mmio_read_16(base + SDHCI_HOST_CONTROL2) | 1<<11);  // set cmd23 support
//    mmio_write_16(vendor_base + VENDOR_EMMC_CTRL,
//                  mmio_read_16(vendor_base + VENDOR_EMMC_CTRL) | 0x1);

    // set clk.Toshiba/Scandisk eMMC don't support 400khz.
//    bm_emmc_set_clk(EMMC_INIT_FREQ);

    // set host version 4 parameters
    mmio_write_16(base + SDHCI_HOST_CONTROL2,
                  mmio_read_16(base + SDHCI_HOST_CONTROL2) | 0x1<<12); // set HOST_VER4_ENABLE
    if (mmio_read_32(base + SDHCI_CAPABILITIES1) & 0x1<<27) {
        mmio_write_16(base + SDHCI_HOST_CONTROL2,
                      mmio_read_16(base + SDHCI_HOST_CONTROL2) | 0x1<<13); // set 64bit addressing
    }

    // eMMc card setup
    mmio_write_16(base + SDHCI_HOST_CONTROL2,
                  mmio_read_16(base + SDHCI_HOST_CONTROL2) & ~(0x1<<8)); // clr UHS2_IF_ENABLE
    mmio_write_8(base + SDHCI_PWR_CONTROL,
                 mmio_read_8(base + SDHCI_PWR_CONTROL) | 0x1); // set SD_BUS_PWR_VDD1
    mmio_write_16(base + SDHCI_HOST_CONTROL2,
                  mmio_read_16(base + SDHCI_HOST_CONTROL2) & ~0x7); // clr UHS_MODE_SEL

    mmio_write_16(base + SDHCI_HOST_CONTROL2,
                  mmio_read_16(base + SDHCI_HOST_CONTROL2) | (0x1<<3)); // set 1.8v signal

	mmio_write_16(base + SDHCI_CLK_CTRL,
                  mmio_read_16(base + SDHCI_CLK_CTRL) | 0x1<<2); // set SD_CLK_EN
    udelay(400); // wait for voltage ramp up time at least 74 cycle,1/200k*80=400us

    //we enable all interrupt status here for testing
    mmio_write_16(base + SDHCI_INT_STATUS_EN, mmio_read_16(base + SDHCI_INT_STATUS_EN) | 0xFFFF);
    mmio_write_16(base + SDHCI_ERR_INT_STATUS_EN,mmio_read_16(base + SDHCI_ERR_INT_STATUS_EN) | 0xFFFF);

    INFO("eMMC init done\n");
}


#if defined(BOARD_FPGA)
#define MMC_FORCE_TRANS_FREQ	(13 * 1000 * 1000)
#elif defined(BOARD_PALLADIUM)
#define MMC_FORCE_TRANS_FREQ	(13 * 1000 * 1000)
#elif defined(BOARD_ASIC)
#define MMC_FORCE_TRANS_FREQ	(13 * 1000 * 1000)
#else
#error "BOARD type not defined"
#endif

#define MMC_TEST_MULI_BLOCK_SZ	(SD_BLOCK_SIZE * 2)
static uint8_t g_buf[MMC_TEST_MULI_BLOCK_SZ] __attribute__((aligned (SD_BLOCK_SIZE)));
static int mmc_ewr_test(struct mmc_host *host, unsigned int start_lba, unsigned int size, uint8_t *buf);
static int emmc_prg_fip_bin(struct mmc_host *host, uint32_t lba, uintptr_t *buf, uintptr_t *rd_buf, uint32_t size);
static int mmc_phy_init(struct mmc_host *host)
{
	struct bm_mmc_params params;
	printf("mmc_phy_init start\n");
	bm_emmc_hw_init();

	params.reg_base = EMMC_BASE;
	params.f_max = bm_mmc_get_clk();
	params.f_min = MMC_INIT_FREQ;
	params.flags = MMC_FLAG_SDMA_TRANS | MMC_FLAG_EMMC_SDR26 | MMC_FLAG_EMMC;
	params.force_trans_freq = MMC_FORCE_TRANS_FREQ;

	bm_mmc_setup_host(&params, host);
	bm_mmc_phy_init(host);

	if (mmc_rescan(host)) {
		return -1;
	}

	if (mmc_ewr_test(host, 0, sizeof(g_buf), g_buf)) {
		return -1;
	}

	mmc_destory(host);

	return 0;
}

static int mmc_pio_test(struct mmc_host *host)
{
	struct bm_mmc_params params;

	params.reg_base = EMMC_BASE;
	params.f_max = bm_mmc_get_clk();
	params.f_min = MMC_INIT_FREQ;
	params.flags = MMC_FLAG_SDMA_TRANS | MMC_FLAG_EMMC_SDR26 | MMC_FLAG_EMMC;
	params.force_trans_freq = 0;
#ifdef BOARD_FPGA
	params.force_trans_freq = 5 * 1000 * 1000;
#endif

	bm_mmc_setup_host(&params, host);
	bm_mmc_phy_init(host);
	if (mmc_rescan(host)) {
		return -1;
	}

	if (mmc_ewr_test(host, 0, sizeof(g_buf), g_buf)) {
		return -1;
	}

	mmc_destory(host);

	return 0;
}

static int mmc_sdma_test(struct mmc_host *host)
{
	struct bm_mmc_params params;

	params.reg_base = EMMC_BASE;
	params.f_max = bm_mmc_get_clk();
	params.f_min = MMC_INIT_FREQ;
	params.flags = MMC_FLAG_SDMA_TRANS | MMC_FLAG_EMMC_SDR26 | MMC_FLAG_EMMC;
	params.force_trans_freq = MMC_FORCE_TRANS_FREQ;

	bm_mmc_setup_host(&params, host);
	bm_mmc_phy_init(host);
	if (mmc_rescan(host)) {
		return -1;
	}

	if (mmc_ewr_test(host, 0, sizeof(g_buf), g_buf)) {
		return -1;
	}

	mmc_destory(host);

	return 0;
}

static int mmc_adma2_test(struct mmc_host *host)
{
	struct bm_mmc_params params;

	params.reg_base = EMMC_BASE;
	params.f_max = bm_mmc_get_clk();
	params.f_min = MMC_INIT_FREQ;
	params.flags = MMC_FLAG_ADMA2_TRANS | MMC_FLAG_EMMC_SDR26 | MMC_FLAG_EMMC;
	params.force_trans_freq = MMC_FORCE_TRANS_FREQ;

	bm_mmc_setup_host(&params, host);
	bm_mmc_phy_init(host);
	if (mmc_rescan(host)) {
		return -1;
	}

	if (mmc_ewr_test(host, 0, sizeof(g_buf), g_buf)) {
		return -1;
	}

	mmc_destory(host);

	return 0;
}

static int mmc_adma3_test(struct mmc_host *host)
{
	struct bm_mmc_params params;

	params.reg_base = EMMC_BASE;
	params.f_max = bm_mmc_get_clk();
	params.f_min = MMC_INIT_FREQ;
	params.flags = MMC_FLAG_ADMA3_TRANS | MMC_FLAG_EMMC_SDR26 | MMC_FLAG_EMMC;
	params.force_trans_freq = MMC_FORCE_TRANS_FREQ;

	bm_mmc_setup_host(&params, host);
	bm_mmc_phy_init(host);
	if (mmc_rescan(host)) {
		return -1;
	}

	if (mmc_ewr_test(host, 0, sizeof(g_buf), g_buf)) {
		return -1;
	}

	mmc_destory(host);

	return 0;
}

static int mmc_prog_boot_partition(struct mmc_host *host)
{
	struct bm_mmc_params params;

	params.reg_base = EMMC_BASE;
	params.f_max = bm_mmc_get_clk();
	params.f_min = MMC_INIT_FREQ;
	params.flags = MMC_FLAG_ADMA3_TRANS | MMC_FLAG_EMMC_SDR26 | MMC_FLAG_EMMC;
	params.force_trans_freq = MMC_FORCE_TRANS_FREQ;

	bm_mmc_setup_host(&params, host);
	bm_mmc_phy_init(host);
	if (mmc_rescan(host)) {
		return -1;
	}

	if (mmc_ewr_test(host, 0, sizeof(g_buf), g_buf)) {
		return -1;
	}

	mmc_destory(host);

	return 0;
}

static int mmc_prog_boot_fip(struct mmc_host *host)
{
	struct bm_mmc_params params;

	params.reg_base = EMMC_BASE;
	params.f_max = bm_mmc_get_clk();
	params.f_min = MMC_INIT_FREQ;
	params.flags = MMC_FLAG_ADMA3_TRANS | MMC_FLAG_EMMC_SDR26 | MMC_FLAG_EMMC;
	params.force_trans_freq = MMC_FORCE_TRANS_FREQ;
	uintptr_t fip_ptr = 0xA0000000;
	size_t fip_size=1024*1024;
	bm_mmc_setup_host(&params, host);
	bm_mmc_phy_init(host);
	if (mmc_rescan(host)) {
		return -1;
	}

	if (emmc_prg_fip_bin(host,0,fip_ptr,rbuf,fip_size)) {
		return -1;
	}
	if (emmc_prg_fip_bin(host,0x800,fip_ptr,rbuf,fip_size)) {
		return -1;
	}
	mmc_destory(host);

	return 0;
}

static int mmc_interrupt_test(struct mmc_host *host)
{
	struct bm_mmc_params params;

	params.reg_base = EMMC_BASE;
	params.f_max = bm_mmc_get_clk();
	params.f_min = MMC_INIT_FREQ;
	params.flags = MMC_FLAG_SDMA_TRANS | MMC_FLAG_EMMC_SDR26 | MMC_FLAG_EMMC | MMC_FLAG_SUPPORT_IRQ;
	params.force_trans_freq = MMC_FORCE_TRANS_FREQ;

	bm_mmc_setup_host(&params, host);
	bm_mmc_phy_init(host);
	if (mmc_rescan(host)) {
		return -1;
	}

	if (mmc_ewr_test(host, 0, sizeof(g_buf), g_buf)) {
		return -1;
	}

	mmc_destory(host);
	return 0;
}

static int mmc_tuning_test(struct mmc_host *host)
{
	struct bm_mmc_params params;

	params.reg_base = EMMC_BASE;
	params.f_max = bm_mmc_get_clk();
	params.f_min = MMC_INIT_FREQ;
	params.flags = MMC_FLAG_SDMA_TRANS | MMC_FLAG_EMMC_HS200 | MMC_FLAG_EMMC;
	params.force_trans_freq = 0;

	bm_mmc_setup_host(&params, host);
	bm_mmc_phy_init(host);
	if (mmc_rescan(host)) {
		return -1;
	}

	mmc_ewr_test(host, 0, sizeof(g_buf), g_buf);

	if (mmc_execute_tuning(host->card)) {
		return -1;
	}

	if (mmc_ewr_test(host, 0, sizeof(g_buf), g_buf)) {
		return -1;
	}

	mmc_destory(host);
	return 0;
}

static int mmc_1_4_line_transfer_test(struct mmc_host *host)
{
	struct bm_mmc_params params;

	params.reg_base = EMMC_BASE;
	params.f_max = bm_mmc_get_clk();
	params.f_min = MMC_INIT_FREQ;
	params.flags = MMC_FLAG_SDMA_TRANS | MMC_FLAG_EMMC_SDR26 | MMC_FLAG_EMMC | MMC_FLAG_BUS_WIDTH_1;
	params.force_trans_freq = MMC_FORCE_TRANS_FREQ;

	bm_mmc_setup_host(&params, host);
	bm_mmc_phy_init(host);
	if (mmc_rescan(host)) {
		return -1;
	}

	if (mmc_ewr_test(host, 0, sizeof(g_buf), g_buf)) {
		return -1;
	}
	mmc_destory(host);

	params.flags = MMC_FLAG_SDMA_TRANS | MMC_FLAG_EMMC_SDR26 | MMC_FLAG_EMMC;

	bm_mmc_setup_host(&params, host);
	bm_mmc_phy_init(host);
	if (mmc_rescan(host)) {
		return -1;
	}

	if (mmc_ewr_test(host, 0, sizeof(g_buf), g_buf)) {
		return -1;
	}

	mmc_destory(host);
	return 0;
}

static int mmc_hs200_test(struct mmc_host *host)
{
	struct bm_mmc_params params;
    static int cnt=0;
    /* if(cnt) */
        /* mmio_write_16(EMMC_BASE+0x24c,mmio_read_16(EMMC_BASE+0x24c)|0x1); */
    /* printf("%p :%08x \n",(void*)(EMMC_BASE+0x24c), mmio_read_32(EMMC_BASE+0x24c)); */
	/* cnt++; */
	params.reg_base = EMMC_BASE;
	params.f_max = bm_mmc_get_clk();
	params.f_min = MMC_INIT_FREQ;
	params.flags = MMC_FLAG_SDMA_TRANS | MMC_FLAG_EMMC_SDR52 | MMC_FLAG_EMMC;
	params.force_trans_freq = 0;

	bm_mmc_setup_host(&params, host);
	bm_mmc_phy_init(host);
	if (mmc_rescan(host)) {
		return -1;
	}

	if (mmc_ewr_test(host, 0, sizeof(g_buf), g_buf)) {
		return -1;
	}
	mmc_destory(host);
	return 0;
}

static int mmc_identify_test(struct mmc_host *host)
{
	struct bm_mmc_params params;

	params.reg_base = EMMC_BASE;
	params.f_max = bm_mmc_get_clk();
	params.f_min = MMC_INIT_FREQ;
	params.flags = MMC_FLAG_SDMA_TRANS | MMC_FLAG_EMMC_SDR26 | MMC_FLAG_EMMC;
	params.force_trans_freq = MMC_FORCE_TRANS_FREQ;

	bm_mmc_setup_host(&params, host);
	bm_mmc_phy_init(host);
	if (mmc_rescan(host)) {
		return -1;
	}

	printf("manfid:     0x%x\n", host->card->cid.manfid);
	printf("prod_name:  %s\n", host->card->cid.prod_name);
	printf("prv:        0x%x\n", host->card->cid.prv);
	printf("serial:     0x%x\n", host->card->cid.serial);
	printf("oemid:      0x%x\n", host->card->cid.oemid);
	printf("year:       0x%x\n", host->card->cid.year);
	printf("month:      0x%x\n", host->card->cid.month);
	printf("hwrev:      0x%x\n", host->card->cid.hwrev);
	printf("fwrev:      0x%x\n", host->card->cid.fwrev);

	mmc_destory(host);
	return 0;
}

static int mmc_single_blk_rwe_test(struct mmc_host *host)
{
	struct bm_mmc_params params;

	params.reg_base = EMMC_BASE;
	params.f_max = bm_mmc_get_clk();
	params.f_min = MMC_INIT_FREQ;
	params.flags = MMC_FLAG_SDMA_TRANS | MMC_FLAG_EMMC_SDR26 | MMC_FLAG_EMMC | MMC_FLAG_SUPPORT_IRQ;
	params.force_trans_freq = MMC_FORCE_TRANS_FREQ;

	bm_mmc_setup_host(&params, host);
	bm_mmc_phy_init(host);
	if (mmc_rescan(host)) {
		return -1;
	}

	if (mmc_ewr_test(host, 0, 0x200, g_buf)) {
		return -1;
	}

	mmc_destory(host);

	return 0;
}

static int mmc_multi_blk_rwe_test(struct mmc_host *host)
{
	struct bm_mmc_params params;

	params.reg_base = EMMC_BASE;
	params.f_max = bm_mmc_get_clk();
	params.f_min = MMC_INIT_FREQ;
	params.flags = MMC_FLAG_SDMA_TRANS | MMC_FLAG_EMMC_SDR26 | MMC_FLAG_EMMC | MMC_FLAG_SUPPORT_IRQ;
	params.force_trans_freq = MMC_FORCE_TRANS_FREQ;

	bm_mmc_setup_host(&params, host);
	bm_mmc_phy_init(host);
	if (mmc_rescan(host)) {
		return -1;
	}

	if (mmc_ewr_test(host, 0, sizeof(g_buf), g_buf)) {
		return -1;
	}
	if (mmc_ewr_test(host, 1024, sizeof(g_buf), g_buf)) {
		return -1;
	}
	if (mmc_ewr_test(host, 2048, sizeof(g_buf), g_buf)) {
		return -1;
	}
	mmc_destory(host);

	return 0;
}

static int mmc_register_rw_test(struct mmc_host *host)
{
	uint32_t base = EMMC_BASE;
	uint32_t addr = base;
	uint32_t count = 20;
	int i;

	mmio_write_16(base + SDHCI_CLK_CTRL, 0x3);
	mmio_write_32(base, 0x12345678);
	for (i = 0; i < count; i++) {
		if (i % 4 == 0) {
			if (i != 0)
				printf("\n");
			printf("0x%08x\t", addr);
			addr += 0x10;
		}
		printf("%08x ", mmio_read_32(base + i * 4));
	}
	printf("\n");

	return 0;
}


int testcase_emmc(void)
{
	int i;
	struct mmc_host host;

	printf("\n------------ emmc test start ------------\n\n");

    pinmux_config(PINMUX_EMMC);
	for (i = 0; i < sizeof(emmc_testcases) / sizeof(emmc_testcases[0]); ++i) {
		printf("[%d] %s start\n", i, emmc_testcases[i].name);
		if (emmc_testcases[i].handler(&host)) {
			printf("[%d] %s failed\n", i, emmc_testcases[i].name);
			return -1;
		}
		printf("[%d] %s done\n\n", i, emmc_testcases[i].name);
	}

	printf("\n------------ emmc test done ------------\n");

	return 0;
}

#ifndef BUILD_TEST_CASE_all
int testcase_main()
{
    pinmux_config(PINMUX_EMMC);

    return testcase_emmc();
}
#endif


static int mmc_ewr_test(struct mmc_host *host, unsigned int start_lba, unsigned int size, uint8_t *buf)
{
	int i;
	uint8_t pattern = 0x5a;

	pr_debug("\r\n================erase lba=0x%x size=%d buf:%p================== \r\n", start_lba, size, buf);
//	mmc_erase_blocks(host, start_lba, size);
//	mmc_read_blocks(host, start_lba, (uintptr_t)buf, size);
//	for (i = 0; i < size; ++i) {
//		if (buf[i] != 0x0 && buf[i] != 0xff) {
//			printf("erase failed at i=%d 0x%x != 0x%x\n", i, buf[i], pattern);
//			assert(0);
//		}
//	}

	pr_debug("\r\n================write/read lba=0x%x size=%d buf:%p================== \r\n", start_lba, size, buf);
	memset(buf, pattern, size);
	mmc_write_blocks(host, start_lba, (uintptr_t)buf, size);
	memset(buf, 0x0, size);
	mmc_read_blocks(host, start_lba, (uintptr_t)buf, size);

	for (i = 0; i < size; ++i) {
		if (buf[i] != pattern) {
			printf("write failed at i=%d 0x%x != 0x%x\n", i, buf[i], pattern);
			assert(0);
		}
	}


	pr_debug("\r\n======================== end ========================= \r\n");

	return 0;
}

#define TRUNK_SIZE 65536
#define ROUND_UP_DIV(a, b) (((a) + (b)-1) / (b))
#define PTR_INC(base, offset) (void *)((uint8_t *)(base) + (offset))
#define EMMC_BLOCK_SIZE 512
static int emmc_prg_fip_bin(struct mmc_host *host, uint32_t lba, uintptr_t *buf, uintptr_t *rd_buf, uint32_t size)
{
	// unsigned long , addr, size;
	int i;
	int count;
	uintptr_t tmp_buf;
	uintptr_t tmp_rd_buf;
	int ret=0;


	count = size % TRUNK_SIZE == 0 ? size / TRUNK_SIZE : size / TRUNK_SIZE + 1;


	for (i = 0; i < count; i++) {

		tmp_buf = (uintptr_t)PTR_INC(buf, i * TRUNK_SIZE);
		tmp_rd_buf = rd_buf;

		INFO("lba 0x%x, i %d\n", lba, i);

		#if INJECT_1ST_FIP_CRC_ERROR
		if (lba == 0 && i == 1)
		{
			INFO("emmc INJECT_1ST_FIP_CRC_ERROR, modify buf address\n");
			tmp_buf = (uintptr_t)0x4000000;
		}
		#endif


		// Switch to boot partition(emmc_rw_enable)
		mmc_cmd_t cmd;
		mmc_ops_t *ops = host->ops;
		zeromem(&cmd, sizeof(mmc_cmd_t));
		cmd.cmd_idx = MMC_CMD6;
		cmd.cmd_arg = EXTCSD_WRITE_BYTES | EXTCMMC_CMD(CMD_EXTCSD_PARTITION_CONFIG) |
				  EXTCSD_VALUE(PART_CFG_BOOT_PARTITION1_ENABLE|PART_CFG_PARTITION_BOOT1_ACCESS) | 1;
		ret=ops->send_cmd(host,&cmd);
		if(ret){
			ERROR("Swtich partition failed\n");
			return -1;
		}
		mmc_write_blocks(host, lba+ i * (TRUNK_SIZE / EMMC_BLOCK_SIZE), tmp_buf, TRUNK_SIZE);
		mmc_read_blocks(host, lba+ (i * TRUNK_SIZE / EMMC_BLOCK_SIZE),tmp_rd_buf, TRUNK_SIZE);

		if(memcmp(tmp_buf,tmp_rd_buf,TRUNK_SIZE)){
			ERROR("fip compare failed at lba:%d \n",i);
			return -1;
		}
		else{
			INFO("FIP lba:%d finished\n",i);
		}
		// emmc_rw_disable
		cmd.cmd_idx = MMC_CMD6;
		cmd.cmd_arg = EXTCSD_WRITE_BYTES | EXTCMMC_CMD(CMD_EXTCSD_PARTITION_CONFIG) |
			EXTCSD_VALUE(PART_CFG_BOOT_PARTITION1_ENABLE) | 1;
		ops->send_cmd(host,&cmd);

	}

	return 0;
}

module_testcase("1", testcase_emmc);
