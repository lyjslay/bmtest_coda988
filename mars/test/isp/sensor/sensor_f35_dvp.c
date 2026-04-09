#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#ifdef ENV_HOSTPC
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include "cli/command.h"
#include "cli/cli.h"
#else
#include "command.h"
#include "cli.h"
#include "system_common.h"
#include "module_testcase.h"
#include "uart.h"
#include "timer.h"
#endif

#include "../../vip/reg.h"
#include "../inc/isp_reg.h"

#include "../sensorlist.h"
#include "../vip_common.h"


#define HARD_REG_GPIOD7_DAT      (0x03023000)
#define HARD_REG_GPIOD7_OE       (0x03023004)

#undef uartlog
#define uartlog(fmt, args...) \
{ \
    printf(fmt, ##args); \
    fflush(stdout); \
}

#define CIF_DPHY_ENABLE
#define F35_DVP_DEFAULT_SHS			0x80
#define F35_DVP_TEST_PATTERN_REG		0x0C
#define F35_DVP_TEST_PTN_DISABLE		0x00
#define F35_DVP_TEST_PTN_WALK_ONE		0x01

struct sensor_mode f35_dvp_mode[] = {
	{
		.name = "1080P30",
		.bit_depth = 10,
		.bayer = SENSOR_BAYER_BG,
		.active_w = 1920,
		.active_h = 1080,
		.total_w = 1920,
		.total_h = 1080,
		.fp_w = 0,
		.bp_w = 0,
		.fp_h = 0,
		.bp_h = 0,	// in spec it shall be 5 but csi-bdg report frame err.
	},
};

struct sensor_info f35_dvp_info = {
	.dev = 0x40,
	.alen = 1,
	.dlen = 1,
	.gain =  { .cur = 0, .min = 0, .max = 0x3F },
	.shsl =  { .cur = 0, .min = 0, .max = 0 },
	.shss =  { .cur = 0, .min = 0, .max = 0 },
	.ops = &f35_dvp_ops,
};

static void f35_dvp_set_shsl(void *priv, uint32_t shs)
{
	struct sensor_info *sinf = (struct sensor_info *)priv;

	sinf->adpt->write(sinf->adpt, sinf->dev, 0x01, sinf->alen, (uint8_t*)&(shs), 1);
	sinf->adpt->write(sinf->adpt, sinf->dev, 0x02, sinf->alen, ((uint8_t*)&(shs)) + 1, 1);
	sinf->shsl.cur = shs;

	return;
}

static void f35_dvp_set_shss(void *priv, uint32_t shs)
{
	struct sensor_info *sinf = (struct sensor_info *)priv;

	sinf->adpt->write(sinf->adpt, sinf->dev, 0x05, sinf->alen, (uint8_t*)&(shs), 1);
	sinf->shsl.cur = shs;

	return;
}

static void f35_dvp_set_gain(void *priv, uint16_t gain)
{
	struct sensor_info *sinf = (struct sensor_info *)priv;

	sinf->adpt->write(sinf->adpt, sinf->dev, 0x00, sinf->alen, (uint8_t*)&(gain), 1);
	sinf->gain.cur = gain;

	return;
}

struct sensor_ops f35_dvp_ops = {
	.set_gain = f35_dvp_set_gain,
	.set_shsl = f35_dvp_set_shsl,
	.set_shss = f35_dvp_set_shss,
};

struct sensor_setting f35_dvp_init_1080p30[] = {
	{0x12, 0x70},
	{0x48, 0x85},
	{0x48, 0x05},
	{0x0E, 0x11},
	{0x0F, 0x14},
	{0x10, 0x40},
	{0x11, 0x80},
	{0x0D, 0xF0},
	{0x5F, 0x42},
	{0x60, 0x2B},
	{0x58, 0x18},
	{0x57, 0x60},
	{0x64, 0xE0},
	{0x20, 0x00},
	{0x21, 0x05},
	{0x22, 0x65},
	{0x23, 0x04},
	{0x24, 0xC0},
	{0x25, 0x38},
	{0x26, 0x43},
	{0x27, 0x0C},
	{0x28, 0x15},
	{0x29, 0x02},
	{0x2A, 0x00},
	{0x2B, 0x12},
	{0x2C, 0x00},
	{0x2D, 0x00},
	{0x2E, 0x14},
	{0x2F, 0x44},
	{0x41, 0xC4},
	{0x42, 0x13},
	{0x46, 0x01},
	{0x76, 0x60},
	{0x77, 0x09},
	{0x80, 0x46},
	{0x1D, 0xFF},
	{0x1E, 0x1F},
	{0x6C, 0x90},
	{0x32, 0x4F},
	{0x33, 0x58},
	{0x34, 0x5F},
	{0x35, 0x5F},
	{0x3A, 0xAF},
	{0x3B, 0x00},
	{0x3C, 0x70},
	{0x3D, 0x8F},
	{0x3E, 0xFF},
	{0x3F, 0x85},
	{0x40, 0xFF},
	{0x56, 0x32},
	{0x59, 0x67},
	{0x85, 0x3C},
	{0x8A, 0x04},
	{0x91, 0x10},
	{0x9C, 0xE1},
	{0x5A, 0x09},
	{0x5C, 0x4C},
	{0x5D, 0xF4},
	{0x5E, 0x1E},
	{0x62, 0x04},
	{0x63, 0x0F},
	{0x66, 0x04},
	{0x67, 0x30},
	{0x6A, 0x12},
	{0x7A, 0xA0},
	{0x9D, 0x10},
	{0x4A, 0x05},
	{0x7E, 0xCD},
	{0x50, 0x02},
	{0x49, 0x10},
	{0x47, 0x02},
	{0x7B, 0x4A},
	{0x7C, 0x0C},
	{0x7F, 0x57},
	{0x8F, 0x80},
	{0x90, 0x00},
	{0x8C, 0xFF},
	{0x8D, 0xC7},
	{0x8E, 0x00},
	{0x8B, 0x01},
	{0x0C, 0x00},
	{0x69, 0x74},
	{0x65, 0x02},
	{0x81, 0x74},
	{0x19, 0x20},
	{0x12, 0x30},
};

int f35_dvp_poweron(void *priv)
{
	pattern_ctx *ctx = (pattern_ctx *)priv;
	struct sensor_info *sinf = (struct sensor_info *)ctx->ptn->priv;
	struct snr_i2c_adapter *adpt = NULL;
	struct sensor_setting *sensor_set = f35_dvp_init_1080p30;
	int setting_num = ARRAY_SIZE(f35_dvp_init_1080p30);
	struct sensor_setting *pset;
	char cmdbuf[8];
	struct sensor_mode *mode = &f35_dvp_mode[F35_DVP_MODE_1080P30];
	int i, r, test_pn = 0;
	uint32_t value;

	// i2c config here
	snr_bm_i2c_get_adap((void *)&adpt);
	sinf->adpt = adpt;

	adpt->slaveaddr = sinf->dev;
	snr_bm_i2c_init(adpt);

	// linear
	uartlog("1920x1080 dvp\n");
	sensor_set = f35_dvp_init_1080p30;
	setting_num = ARRAY_SIZE(f35_dvp_init_1080p30);
	mode = &f35_dvp_mode[F35_DVP_MODE_1080P30];

	cmdbuf[0] = cmdbuf[1] = '0';
	cli_readline("enable test pattern? [n/y]", cmdbuf);
	if (cmdbuf[0] == 'y') {
		test_pn = 1;
	}

	// i2c initial sequence
	for (i = 0; i < setting_num; i++) {
		pset = &(sensor_set[i]);

		if (pset->addr == SNR_MARKER_DELAY) {
			uartlog("sensor wr delay\n");
			udelay(pset->val);
			continue;
		}
		r = sinf->adpt->write(sinf->adpt, sinf->dev, pset->addr, sinf->alen, (uint8_t*)&(pset->val), 1);
		if (r < 0)
			uartlog("%s i2c_write fail addr[0x%x]\n", ctx->ptn->name, pset->addr);
	}

	value = test_pn ? F35_DVP_TEST_PTN_WALK_ONE : F35_DVP_TEST_PTN_DISABLE;
	r = sinf->adpt->write(sinf->adpt, sinf->dev, F35_DVP_TEST_PATTERN_REG,
			      sinf->alen, (uint8_t*)&value, 1);
	if (r < 0)
		uartlog("%s i2c_write fail addr[0x%x]\n", ctx->ptn->name, F35_DVP_TEST_PATTERN_REG);


	uartlog("Read back i2c registers:\n");
	for (i = 0; i < setting_num; i++) {
		uint16_t val = 0;

		pset = &(sensor_set[i]);
		r = sinf->adpt->read(sinf->adpt, sinf->dev, pset->addr, sinf->alen, (uint8_t*)&val, 1);

		uartlog("reg[0x%x]=0x%x(0x%x)\n", pset->addr, val, pset->val);
	}

	uartlog("streaming start\n");

	cmdbuf[0] = cmdbuf[1] = '0';
	cli_readline("run sensor to dram flow? [n/y]", cmdbuf);
	if (cmdbuf[0] == 'y') {
		uartlog("dump %d x %d \n", mode->total_w, mode->total_h);
		config_csibdg(0, mode->total_w,
				mode->total_h,
				ctx, 0, 0);
	}
	return 0;
}

int peri_up_f35_dvp(void *priv)
{
#define CAM0PLL_SYNC_SET		406720388UL
#define CAM0PLL_DIV_SEL			12
#define CAM0PLL_POST_DIV_SEL		1
#define CAM0PLL_ICTRL			0
#define CAM0PLL_SEL_MODE		1
#define CAM0PLL_PRE_DIV_SEL		1
#define CMA0PLL_CLK_DIV			44

	char cmdbuf[8];
	uint32_t value;
#if !defined(FLOW_AFTER_ECO)
	unsigned int *fpga_pwr_reg;
#endif

	config_pinmux_raw0();

	cmdbuf[0] = '0';
	cli_readline("### enter to start power-on seqeuence. s) to skip : ", cmdbuf);
	if (cmdbuf[0] == 's')
		return 0;

#if defined(BOARD_ASIC)
	// power on/reset
	// 1.2V = 1, 1.8V = 1, 2.9V = 1
	// INCK on
	// enable cam0pll clk source
	value = readl(0x03002030);
	value &= ~(1<<28);
	writel(0x03002030, value);
	// cam0pll dividor factor
	writel(0x030020F4, (CMA0PLL_CLK_DIV<<16)|0x09);
	// set sync source en
	value = readl(0x03002840);
	value |= (1<<4);
	writel(0x03002840, value);
	// set sync set
	writel(0x03002874, CAM0PLL_SYNC_SET);
	// set sync set sw up
	value = readl(0x03002870);
	value ^= 0x01;
	writel(0x03002870, value);
	// set csr
	value = CAM0PLL_PRE_DIV_SEL | (CAM0PLL_POST_DIV_SEL << 8) | (CAM0PLL_SEL_MODE << 15) | (CAM0PLL_DIV_SEL << 17)
		| (CAM0PLL_ICTRL << 24);
	writel(0x03002814, value);
	// clear pwd
	value = readl(0x03002800);
	value &= ~(1<<12);
	writel(0x03002800, value);
	udelay(100);
	// XCLR = 0
	writel(HARD_REG_GPIOD7_DAT, 0x80);
	writel(HARD_REG_GPIOD7_OE, 0x80);
	// wait 500us, wait INCK stable
	udelay(5000);
	// XCLR = 1
	writel(HARD_REG_GPIOD7_DAT, 0x00);
	// wait 20ms
	udelay(20000);
	writel(HARD_REG_GPIOD7_DAT, 0x80);
	udelay(20000);
#endif

#if !defined(FLOW_AFTER_ECO)
	// enable display clk select
	value = readl(0x0a0c8018);
	value |= 0x02;
	writel(0x0a0c8018, value);
	udelay(5000);
#endif

	// Switch the MAC type to TTL
	writel(0x0a0c2000, 0x00000002);
	// configure TTL [0] ttl enable [2:1] bit mode
	writel(0x0a0c2010, 0x00000E03);
	writel(0x0a0c2030, 0x00000022);

#if !defined(FLOW_AFTER_ECO)
	fpga_pwr_reg = (unsigned int *)HARD_REG_FPGA_PWRPIN;
	// power sequence
	writel(fpga_pwr_reg, 0);
	uartlog("power-on delay\n");
	udelay(5000);
	writel(fpga_pwr_reg, 0x08);
	uartlog("power-on delay\n");
	udelay(5000);
	writel(fpga_pwr_reg, 0x18);
	uartlog("power-on delay\n");
	udelay(5000);
	writel(fpga_pwr_reg, 0x38);
	uartlog("power-on delay\n");
	udelay(5000);
	// clear reset and enable phy rx
	writel(fpga_pwr_reg, 0x32100039);
#endif
	uartlog("### device poweron\n");

	return 0;
}

int peri_down_f35_dvp(void *priv)
{
	char cmdbuf[8] = {0};

	cli_readline("### power-off? y/n", cmdbuf);

	if (cmdbuf[0] == 'y') {
#if !defined(FLOW_AFTER_ECO)
		unsigned int *fpga_pwr_reg;

		fpga_pwr_reg = (unsigned int *)HARD_REG_FPGA_PWRPIN;
		// INCLK = 0
		// XCLR = 0
		// POWER-OFF sequence
		uartlog("power-off");
		writel(fpga_pwr_reg, 0x18);
		udelay(5000);
		writel(fpga_pwr_reg, 0x08);
		udelay(5000);
		writel(fpga_pwr_reg, 0);
		udelay(5000);
#endif
	}

	return 0;
}

