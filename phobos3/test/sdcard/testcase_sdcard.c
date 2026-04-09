#include <stdint.h>
#include <string.h>

#include "boot_test.h"
#include "debug.h"
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
static int mmc_interrupt_test(struct mmc_host *host);
#ifndef BOARD_FPGA
static int mmc_tuning_test(struct mmc_host *host);
static int mmc_uhs_test(struct mmc_host *host);
#endif
static int mmc_1_4_line_transfer_test(struct mmc_host *host);
#if defined(TEST_SD0)
static int mmc_card_detect_test(struct mmc_host *host);
#endif
static int mmc_card_lock_test(struct mmc_host *host);
static int mmc_identify_test(struct mmc_host *host);
static int mmc_single_blk_rwe_test(struct mmc_host *host);
static int mmc_multi_blk_rwe_test(struct mmc_host *host);
static int mmc_register_rw_test(struct mmc_host *host);

static struct mmc_testcase mmc_testcases[] = {
	{ .handler = mmc_register_rw_test, .name = "Register read/write test" },
#if defined(TEST_SD0)
	{ .handler = mmc_card_detect_test, .name = "Card detect test" },
#endif
	{ .handler = mmc_card_lock_test, .name = "Card lock test" },
	{ .handler = mmc_phy_init, .name = "PHY initialization test" },
	{ .handler = mmc_identify_test, .name = "Card identification mode test" },
	{ .handler = mmc_1_4_line_transfer_test, .name = "1 and 4 dat lines transfer test" },
	{ .handler = mmc_pio_test, .name = "PIO test" },
	{ .handler = mmc_sdma_test, .name = "SDMA test" },
	{ .handler = mmc_adma2_test, .name = "ADMA2 test" },
	{ .handler = mmc_adma3_test, .name = "ADMA3 test" },
	{ .handler = mmc_interrupt_test, .name = "Interrupt test" },
#ifndef BOARD_FPGA
	{ .handler = mmc_tuning_test, .name = "Tuning test" },
	{ .handler = mmc_uhs_test, .name = "UHS[SDR50/DDR50/SDR104] test" },
#endif
	{ .handler = mmc_single_blk_rwe_test, .name = "Single block read/write test" },
	{ .handler = mmc_multi_blk_rwe_test, .name = "Multiple block read/write test" },
#ifndef BOARD_FPGA
	{ .handler = mmc_uhs_test, .name = "UHS[SDR50/DDR50/SDR104] test" },
#endif
};

static int bm_mmc_get_clk()
{
	int clk;
#ifdef BOARD_FPGA
#if 1
		printf("fpga_asic\n");
	
//		printf("xtal pll setting 0x%x\n", mmio_read_32(0x3002030));
//		mmio_clrbits_32(0x3002030, BIT(6));
	//	mmio_write_32(0x3002030, 0);
		printf("xtal pll setting2 0x%x\n", mmio_read_32(0x3002030));
	
		// for 1882, the initial clock is from xtal, which is 25Mhz
		// for 1882, atfer switch to pll, the default pll clock is 375Mhz
//		mmio_write_32(0x3002064, 0x40009);
	
		clk = 25 * 1000 * 1000;
		printf("SD CLK is %d in FPGA_ASIC\n", clk);
	
		return clk;

#else
		printf("fpga_asic\n");
	
		printf("xtal pll setting 0x%x\n", mmio_read_32(0x3002030));
		mmio_clrbits_32(0x3002030, BIT(6));
	//	mmio_write_32(0x3002030, 0);
		printf("xtal pll setting2 0x%x\n", mmio_read_32(0x3002030));
	
		// for 1882, the initial clock is from xtal, which is 25Mhz
		// for 1882, atfer switch to pll, the default pll clock is 375Mhz
		mmio_write_32(0x3002064, 0x40009);
	
		clk = 375 * 1000 * 1000;
		printf("SD CLK is %d in FPGA_ASIC\n", clk);
	
		return clk;
	
#endif


#elif defined(BOARD_PALLADIUM)
	printf("PXP\n");

	printf("xtal pll setting 0x%x\n", mmio_read_32(0x3002030));
//	mmio_write_32(0x3002030, 0);
	printf("xtal pll setting2 0x%x\n", mmio_read_32(0x3002030));

	// for 1882, the initial clock is from xtal, which is 25Mhz
	// for 1882, atfer switch to pll, the default pll clock is 375Mhz
	mmio_write_32(0x3002064, 0x40009);

	clk = 375 * 1000 * 1000;
	printf("eMMC CLK is %d in PXP\n", clk);

	return clk;
#elif defined(BOARD_ASIC)

	printf("ASIC xtal pll setting 0x%x\n", mmio_read_32(0x3002030));
	mmio_write_32(0x3002030, 0);
	printf("ASIC xtal pll setting2 0x%x\n", mmio_read_32(0x3002030));

	// for 1882, the initial clock is from xtal, which is 25Mhz
	// for 1882, atfer switch to pll, the default pll clock is 375Mhz
	mmio_write_32(0x3002064, 0x40009);

	clk = 375 * 1000 * 1000;
	printf("eMMC CLK is %d in ASIC\n", clk);

	return clk;
#else
#error "BOARD type not defined"
#endif
}


#if defined(BOARD_FPGA)
#define MMC_FORCE_TRANS_FREQ	(12 * 1000 * 1000)
#elif defined(BOARD_PALLADIUM)
#define MMC_FORCE_TRANS_FREQ	(12 * 1000 * 1000)
#elif defined(BOARD_ASIC)
#define MMC_FORCE_TRANS_FREQ	(12 * 1000 * 1000)
#else
#error "board type not defined"
#endif

#define MMC_TEST_MULI_BLOCK_SZ	(SD_BLOCK_SIZE * 2)
static uint8_t g_buf[MMC_TEST_MULI_BLOCK_SZ] __attribute__((aligned (SD_BLOCK_SIZE)));
static int mmc_ewr_test(struct mmc_host *host, unsigned int start_lba, unsigned int size, uint8_t *buf);
static int __attribute((__unused__)) mmc_phy_init(struct mmc_host *host)
{
	struct bm_mmc_params params;

	params.reg_base = SDIO_BASE;
	params.f_max = bm_mmc_get_clk();
	params.f_min = MMC_INIT_FREQ;
	params.flags = MMC_FLAG_SDMA_TRANS | MMC_FLAG_SD_HIGHSPEED | MMC_FLAG_SDCARD;
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

static int __attribute((__unused__)) mmc_pio_test(struct mmc_host *host)
{
	struct bm_mmc_params params;

	params.reg_base = SDIO_BASE;
	params.f_max = bm_mmc_get_clk();
	params.f_min = MMC_INIT_FREQ;
	params.flags = MMC_FLAG_PIO_TRANS | MMC_FLAG_SD_HIGHSPEED |
					MMC_FLAG_SDCARD;
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

static int __attribute((__unused__)) mmc_sdma_test(struct mmc_host *host)
{
	struct bm_mmc_params params;

	params.reg_base = SDIO_BASE;
	params.f_max = bm_mmc_get_clk();
	params.f_min = MMC_INIT_FREQ;
	params.flags = MMC_FLAG_SDMA_TRANS | MMC_FLAG_SD_HIGHSPEED | MMC_FLAG_SDCARD;
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

static int __attribute((__unused__)) mmc_adma2_test(struct mmc_host *host)
{
	struct bm_mmc_params params;

	params.reg_base = SDIO_BASE;
	params.f_max = bm_mmc_get_clk();
	params.f_min = MMC_INIT_FREQ;
	params.flags = MMC_FLAG_ADMA2_TRANS | MMC_FLAG_SD_HIGHSPEED | MMC_FLAG_SDCARD;
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

static int __attribute((__unused__)) mmc_adma3_test(struct mmc_host *host)
{
	struct bm_mmc_params params;

	params.reg_base = SDIO_BASE;
	params.f_max = bm_mmc_get_clk();
	params.f_min = MMC_INIT_FREQ;
	params.flags = MMC_FLAG_ADMA3_TRANS | MMC_FLAG_SD_HIGHSPEED | MMC_FLAG_SDCARD;
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

static int __attribute((__unused__)) mmc_interrupt_test(struct mmc_host *host)
{
	struct bm_mmc_params params;

	params.reg_base = SDIO_BASE;
	params.f_max = bm_mmc_get_clk();
	params.f_min = MMC_INIT_FREQ;
	params.flags = MMC_FLAG_SDMA_TRANS | MMC_FLAG_SD_HIGHSPEED | MMC_FLAG_SUPPORT_IRQ | MMC_FLAG_SDCARD;
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

#ifndef BOARD_FPGA
static int mmc_tuning_test(struct mmc_host *host)
{
	struct bm_mmc_params params;

	params.reg_base = SDIO_BASE;
	params.f_max = bm_mmc_get_clk();
	params.f_min = MMC_INIT_FREQ;
	params.flags = MMC_FLAG_SDMA_TRANS | MMC_FLAG_SD_SDR104 | MMC_FLAG_SDCARD;
	params.force_trans_freq = MMC_FORCE_TRANS_FREQ;

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

static int mmc_uhs_test(struct mmc_host *host)
{
	struct bm_mmc_params params;
    static int cnt=0;
    if(cnt)
        mmio_write_16(SDIO_BASE+0x24c,mmio_read_16(SDIO_BASE+0x24c)|0x1);
    printf("%p :%08x ",(void*)(SDIO_BASE+0x24c), mmio_read_32(SDIO_BASE+0x24c));
	params.reg_base = SDIO_BASE;
	params.f_max = bm_mmc_get_clk();
	params.f_min = MMC_INIT_FREQ;
	params.flags = MMC_FLAG_SDMA_TRANS | MMC_FLAG_SD_SDR104 | MMC_FLAG_SDCARD;
	params.force_trans_freq = MMC_FORCE_TRANS_FREQ;

	bm_mmc_setup_host(&params, host);
	bm_mmc_phy_init(host);
    cnt++;
	if (mmc_rescan(host)) {
		return -1;
	}

	if (mmc_ewr_test(host, 0, sizeof(g_buf), g_buf)) {
		return -1;
	}

	mmc_destory(host);

	return 0;
}

#endif

static int __attribute((__unused__)) mmc_1_4_line_transfer_test(struct mmc_host *host)
{
	struct bm_mmc_params params;

	params.reg_base = SDIO_BASE;
	params.f_max = bm_mmc_get_clk();
	params.f_min = MMC_INIT_FREQ;
	params.flags = MMC_FLAG_SDMA_TRANS | MMC_FLAG_SD_HIGHSPEED | MMC_FLAG_SD_DFLTSPEED | MMC_FLAG_SDCARD;
	params.force_trans_freq = MMC_FORCE_TRANS_FREQ;

	bm_mmc_setup_host(&params, host);
	bm_mmc_phy_init(host);
	if (mmc_rescan(host)) {
		return -1;
	}

	/* Set 1-bit bus width */
	if (mmc_app_set_bus_width(host->card, MMC_BUS_WIDTH_1)) {
		return -1;
	}
	mmc_set_bus_width(host, MMC_BUS_WIDTH_1);

	if (mmc_ewr_test(host, 0, sizeof(g_buf), g_buf)) {
		return -1;
	}

	/* Set 4-bit bus width */
	if (mmc_app_set_bus_width(host->card, MMC_BUS_WIDTH_4)) {
		return -1;
	}
	mmc_set_bus_width(host, MMC_BUS_WIDTH_4);

	if (mmc_ewr_test(host, 0, sizeof(g_buf), g_buf)) {
		return -1;
	}

	mmc_destory(host);
	return 0;
}
#if defined(TEST_SD0)
static int mmc_card_detect_test(struct mmc_host *host)
{
	struct bm_mmc_params params;

	params.reg_base = SDIO_BASE;
	params.f_max = bm_mmc_get_clk();
	params.f_min = MMC_INIT_FREQ;
	params.flags = MMC_FLAG_SDMA_TRANS | MMC_FLAG_SDCARD;
	params.force_trans_freq = MMC_FORCE_TRANS_FREQ;

	bm_mmc_setup_host(&params, host);

	if (bm_sd_card_detect(host)) {
		printf("card detected\n");
	} else {
		printf("card not detected\n");
	}

	return 0;
}
#endif
static int __attribute((__unused__)) mmc_card_lock_test(struct mmc_host *host)
{
	struct bm_mmc_params params;

	params.reg_base = SDIO_BASE;
	params.f_max = bm_mmc_get_clk();
	params.f_min = MMC_INIT_FREQ;
	params.flags = MMC_FLAG_SDMA_TRANS | MMC_FLAG_SDCARD;

	bm_mmc_setup_host(&params, host);

	if (bm_sd_card_lock(host)) {
		printf("card locked\n");
	} else {
		printf("card not locked\n");
	}

	return 0;
}

static int __attribute((__unused__)) mmc_identify_test(struct mmc_host *host)
{
	struct bm_mmc_params params;

	params.reg_base = SDIO_BASE;
	params.f_max = bm_mmc_get_clk();
	params.f_min = MMC_INIT_FREQ;
	params.flags = MMC_FLAG_SDMA_TRANS | MMC_FLAG_SD_DFLTSPEED | MMC_FLAG_SDCARD;
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

static int __attribute((__unused__)) mmc_single_blk_rwe_test(struct mmc_host *host)
{
	struct bm_mmc_params params;

	params.reg_base = SDIO_BASE;
	params.f_max = bm_mmc_get_clk();
	params.f_min = MMC_INIT_FREQ;
	params.flags = MMC_FLAG_SDMA_TRANS | MMC_FLAG_SD_HIGHSPEED | MMC_FLAG_SDCARD | MMC_FLAG_SUPPORT_IRQ;
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

static int __attribute((__unused__)) mmc_multi_blk_rwe_test(struct mmc_host *host)
{
	struct bm_mmc_params params;

	params.reg_base = SDIO_BASE;
	params.f_max = bm_mmc_get_clk();
	params.f_min = MMC_INIT_FREQ;
	params.flags = MMC_FLAG_SDMA_TRANS | MMC_FLAG_SD_HIGHSPEED | MMC_FLAG_SDCARD | MMC_FLAG_SUPPORT_IRQ;
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

static int mmc_register_rw_test(struct mmc_host *host)
{
	uint32_t base = SDIO_BASE;
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


int testcase_sdcard(void)
{
	int i;
	struct mmc_host host;

	printf("\n------------ mmc test start ------------\n\n");

#if defined(TEST_SD0)
    pinmux_config(PINMUX_SDIO0);
#elif defined(TEST_SD1)
    pinmux_config(PINMUX_SDIO1);
#endif
	for (i = 0; i < sizeof(mmc_testcases) / sizeof(mmc_testcases[0]); ++i) {
		printf("[%d] %s start\n", i, mmc_testcases[i].name);
		if (mmc_testcases[i].handler(&host)) {
			printf("[%d] %s failed\n", i, mmc_testcases[i].name);
			return -1;
		}
		printf("[%d] %s done\n\n", i, mmc_testcases[i].name);
	}

	printf("\n------------ mmc test done ------------\n");

	return 0;
}

#ifndef BUILD_TEST_CASE_all
int testcase_main()
{
    return testcase_sdcard();
}
#endif

static void __attribute((__unused__)) dump_hex(char *desc, void *addr, int len)
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


static int mmc_ewr_test(struct mmc_host *host, unsigned int start_lba, unsigned int size, uint8_t *buf)
{
	int i;
	uint8_t pattern = 0x5a;

#if 0
	pr_debug("\r\n================erase lba=0x%x size=%d buf:%p================== \r\n", start_lba, size, buf);
	mmc_erase_blocks(host, start_lba, size);
	memset(buf, 0x12, size);
	mmc_read_blocks(host, start_lba, (uintptr_t)buf, size);
	for (i = 0; i < size; ++i) {
		if (buf[i] != 0x0 && buf[i] != 0xff) {
			printf("erase failed at i=%d 0x%x != 0x%x\n", i, buf[i], pattern);
			ASSERT(0);
		}
	}
#endif

	pr_debug("\r\n================write/read lba=0x%x size=%d buf:%p================== \r\n", start_lba, size, buf);
	memset(buf, pattern, size);
	mmc_write_blocks(host, start_lba, (uintptr_t)buf, size);
	memset(buf, 0x0, size);
	mmc_read_blocks(host, start_lba, (uintptr_t)buf, size);

	for (i = 0; i < size; ++i) {
		if (buf[i] != pattern) {
			printf("write failed at i=%d 0x%x != 0x%x\n", i, buf[i], pattern);
			ASSERT(0);
		}
	}

	pr_debug("\r\n======================== end ========================= \r\n");

	return 0;
}

module_testcase("1", testcase_sdcard);

