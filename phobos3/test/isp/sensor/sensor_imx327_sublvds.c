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

#if defined(BOARD_ASIC) && defined(CIF_DPHY_ENABLE)
static uint8_t lane_map[5] = {3, 4, 2, 0, 1};
#endif

struct sensor_info imx327lvds_info = {
	.dev = 0x1A,
	.alen = 2,
	.dlen = 1,
	.gain =  { .cur = 0, .min = 0, .max = 0xf0 },
	.sgain =  { .cur = 0, .min = 0, .max = 0xf0 },
	.shsl =  { .cur = 0, .min = 0, .max = 0 },
	.shss =  { .cur = 0, .min = 0, .max = 0 },
	.ops = &imx327lvds_ops,
};

// sensor mode for imx327lvds
struct sensor_mode imx327lvds_mode[] = {
	[IMX327LVDS_MODE_1080P30] =
	{
		.name = "1080p30",
		.bit_depth = 12,
		.bayer = SENSOR_BAYER_GR,
		.active_w = 1920,
		.active_h = 1080,
		.total_w = 1948,
		.total_h = 1110,
		.fp_w = 0,
		.bp_w = 0,
		.fp_h = 0,
		.bp_h = 0,	// in spec it shall be 5 but csi-bdg report frame err.
	},
	[IMX327LVDS_MODE_1080P30_HDR1] =
	{
		.name = "1080p30_hdr1",
		.bit_depth = 12,
		.bayer = SENSOR_BAYER_GR,
		.active_w = 1920,
		.active_h = 1080,
		.total_w = 1948,
		.total_h = 1110,
		.fp_w = 0,
		.bp_w = 0,
		.fp_h = 0,
		.bp_h = 0,	// in spec it shall be 5 but csi-bdg report frame err.
	},
};

static void imx327lvds_get_gain(void *priv, struct sensor_prop *prop)
{
	struct sensor_info *sinf = (struct sensor_info *)priv;

	*prop = sinf->gain;
	sinf->adpt->read(sinf->adpt, sinf->dev, 0x3014, sinf->alen, (uint8_t*)&(prop->cur), 1);

	return;
}

static void imx327lvds_set_gain(void *priv, uint16_t gain)
{
	struct sensor_info *sinf = (struct sensor_info *)priv;

	sinf->adpt->write(sinf->adpt, sinf->dev, 0x3014, sinf->alen, (uint8_t*)&(gain), 1);
	sinf->gain.cur = gain;

	return;
}

static void imx327lvds_get_sgain(void *priv, struct sensor_prop *prop)
{
	struct sensor_info *sinf = (struct sensor_info *)priv;

	*prop = sinf->sgain;
	sinf->adpt->read(sinf->adpt, sinf->dev, 0x30F2, sinf->alen, (uint8_t*)&(prop->cur), 1);

	return;
}

static void imx327lvds_set_sgain(void *priv, uint16_t gain)
{
	struct sensor_info *sinf = (struct sensor_info *)priv;

	sinf->adpt->write(sinf->adpt, sinf->dev, 0x30F2, sinf->alen, (uint8_t*)&(gain), 1);
	sinf->sgain.cur = gain;

	return;
}

static void imx327lvds_get_shsl(void *priv, struct sensor_prop *prop)
{
	struct sensor_info *sinf = (struct sensor_info *)priv;
	const uint16_t addr[][3] = {{0x3020, 0x3021, 0x3022}, {0x3024, 0x3025, 0x3026}};

	*prop = sinf->shsl;
	sinf->adpt->read(sinf->adpt, sinf->dev, addr[sinf->ishdr][0], sinf->alen, (uint8_t*)&(prop->cur), 1);
	sinf->adpt->read(sinf->adpt, sinf->dev, addr[sinf->ishdr][1], sinf->alen, ((uint8_t*)&(prop->cur)) + 1, 1);
	sinf->adpt->read(sinf->adpt, sinf->dev, addr[sinf->ishdr][2], sinf->alen, ((uint8_t*)&(prop->cur)) + 2, 1);

	return;
}

static void imx327lvds_set_shsl(void *priv, uint32_t shs)
{
	struct sensor_info *sinf = (struct sensor_info *)priv;
	const uint16_t addr[][3] = {{0x3020, 0x3021, 0x3022}, {0x3024, 0x3025, 0x3026}};

	sinf->adpt->write(sinf->adpt, sinf->dev, addr[sinf->ishdr][0], sinf->alen, (uint8_t*)&(shs), 1);
	sinf->adpt->write(sinf->adpt, sinf->dev, addr[sinf->ishdr][1], sinf->alen, ((uint8_t*)&(shs)) + 1, 1);
	sinf->adpt->write(sinf->adpt, sinf->dev, addr[sinf->ishdr][2], sinf->alen, ((uint8_t*)&(shs)) + 2, 1);
	sinf->shsl.cur = shs;

	return;
}

static void imx327lvds_get_shss(void *priv, struct sensor_prop *prop)
{
	struct sensor_info *sinf = (struct sensor_info *)priv;

	*prop = sinf->shss;
	sinf->adpt->read(sinf->adpt, sinf->dev, 0x3020, sinf->alen, (uint8_t*)&(prop->cur), 1);
	sinf->adpt->read(sinf->adpt, sinf->dev, 0x3021, sinf->alen, ((uint8_t*)&(prop->cur)) + 1, 1);
	sinf->adpt->read(sinf->adpt, sinf->dev, 0x3022, sinf->alen, ((uint8_t*)&(prop->cur)) + 2, 1);

	return;
}

static void imx327lvds_set_shss(void *priv, uint32_t shs)
{
	struct sensor_info *sinf = (struct sensor_info *)priv;

	sinf->adpt->write(sinf->adpt, sinf->dev, 0x3020, sinf->alen, (uint8_t*)&(shs), 1);
	sinf->adpt->write(sinf->adpt, sinf->dev, 0x3021, sinf->alen, ((uint8_t*)&(shs)) + 1, 1);
	sinf->adpt->write(sinf->adpt, sinf->dev, 0x3022, sinf->alen, ((uint8_t*)&(shs)) + 2, 1);
	sinf->shss.cur = shs;

	return;
}

struct sensor_ops imx327lvds_ops = {
	.get_gain = imx327lvds_get_gain,
	.set_gain = imx327lvds_set_gain,
	.get_sgain = imx327lvds_get_sgain,
	.set_sgain = imx327lvds_set_sgain,
	.get_shsl = imx327lvds_get_shsl,
	.set_shsl = imx327lvds_set_shsl,
	.get_shss = imx327lvds_get_shss,
	.set_shss = imx327lvds_set_shss,
};

#define IMX327LVDS_TEST_PATTERN_REG        0x308C
#define IMX327LVDS_TEST_PTN_V_COLORBAR     0x21

struct sensor_setting imx327lvds_init_1080p30_4lane[] = {
	{0x3003, 0x01}, // SW Reset
	{SNR_MARKER_DELAY, 3000}, // delay
	{0x3000, 0x01}, // STANDBY
	{0x3001, 0x00}, // HOLD
	{0x3002, 0x00}, // XMSTA
	{0x3005, 0x01}, // ADBIT 10bit
	{0x3007, 0x00}, // VREVERS, ...
	{0x3009, 0x12}, //
	{0x300A, 0xF0}, // BLKLEVEL
	//{0x300C, 0x00}, // WDMODE [0] 0:Normal, 1:DOL, WDSEL [5:4] 1:DOL 2 frames
	{0x3011, 0x02},
	{0x3014, 0xB4}, // GAIN, 0x2A=>12.6dB TBD
	{0x3018, 0x65}, // VMAX[7:0]
	{0x3019, 0x04}, // VMAX[15:8]
	{0x301A, 0x00}, // VMAX[17:16]:=:0x301A[1:0]
	{0x301C, 0x30}, // HMAX[7:0], TBD
	{0x301D, 0x11}, // HMAX[15:8]
	{0x3020, 0x01}, // SHS[7:0], TBD
	{0x3021, 0x00}, // SHS[15:8]
	{0x3022, 0x00}, // SHS[19:16]
	//{0x3024, 0x00}, // SHS2[7:0], TBD
	//{0x3025, 0x00}, // SHS2[15:8]
	//{0x3026, 0x00}, // SHS2[19:16]
	//{0x3030, 0x00}, // RHS1[7:0], TBD
	//{0x3031, 0x00}, // RHS1[15:8]
	//{0x3032, 0x00}, // RHS1[19:16]
	//{0x3045, 0x00}, // DOLSCDEN [0] 1: pattern1 0: pattern2
	// DOLSYDINFOEN[1] 1: embed the id code into 4th of sync
	// code. 0: disable
	// HINFOEN [2] 1: insert id code after 4th sync code 0: disable
	{0x3046, 0xE1},
	{0x304B, 0x0A},
	{0x305C, 0x18}, // INCKSEL1
	{0x305D, 0x00}, // INCKSEL2
	{0x305E, 0x20}, // INCKSEL3
	{0x305F, 0x01}, // INCKSEL4
	{0x309E, 0x4A},
	{0x309F, 0x4A},
	{0x30D2, 0x19},
	{0x30D7, 0x03},
	//{0x3106, 0x00}, //DOLHBFIXEN[7] 0: pattern1 1: pattern2
	{0x3129, 0x00},
	{0x313B, 0x61},
	{0x315E, 0x1A},
	{0x3164, 0x1A},
	{0x317C, 0x00},
	{0x31EC, 0x0E},
	{0x3480, 0x49}, // incksel7
	/* pattern gen */
	//{IMX290_TEST_PATTERN_REG, 0x21},
	//{IMX290_TEST_PATTERN_REG, 0x00},
	{SNR_MARKER_DELAY, 3000}, // delay
	{0x3000, 0x00}, // STANDBY
};

struct sensor_setting imx327lvds_init_1080p30_4lane_hdr1[] = {
	{0x3003, 0x01}, // SW Reset
	{SNR_MARKER_DELAY, 3000}, // delay
	{0x3000, 0x01}, // STANDBY
	{0x3001, 0x00}, // HOLD
	{0x3002, 0x00}, // XMSTA
	{0x3005, 0x01}, // ADBIT
	{0x3007, 0x00}, // VREVERS, ...
	{0x3009, 0x01}, //
	{0x300A, 0xF0}, // BLKLEVEL
	{0x300C, 0x11}, // WDMODE [0] 0:Normal, 1:DOL, WDSEL [5:4] 1:DOL 2 frames
	{0x3011, 0x02},
	{0x3014, 0x16}, // GAIN, 0x2A=>12.6dB TBD
	{0x3018, 0x65}, // VMAX[7:0]
	{0x3019, 0x04}, // VMAX[15:8]
	{0x301A, 0x00}, // VMAX[17:16]:=:0x301A[1:0]
	{0x301C, 0x98}, // HMAX[7:0], TBD
	{0x301D, 0x08}, // HMAX[15:8]
	{0x3020, 0x02}, // SHS[7:0], TBD
	{0x3021, 0x00}, // SHS[15:8]
	{0x3022, 0x00}, // SHS[19:16]
	{0x3024, 0xC9}, // SHS2[7:0], TBD
	{0x3025, 0x07}, // SHS2[15:8]
	{0x3026, 0x00}, // SHS2[19:16]
	{0x3030, 0x0B}, // RHS1[7:0], TBD
	{0x3031, 0x00}, // RHS1[15:8]
	{0x3032, 0x00}, // RHS1[19:16]
	{0x3045, 0x03}, // DOLSCDEN [0] 1: pattern1 0: pattern2
	// DOLSYDINFOEN[1] 1: embed the id code into 4th of sync
	// code. 0: disable
	// HINFOEN [2] 1: insert id code after 4th sync code 0: disable
	{0x3046, 0xE1},
	{0x304B, 0x0A},
	{0x305C, 0x18}, // INCKSEL1
	{0x305D, 0x00}, // INCKSEL2
	{0x305E, 0x20}, // INCKSEL3
	{0x305F, 0x01}, // INCKSEL4
	{0x309E, 0x4A},
	{0x309F, 0x4A},
	{0x30D2, 0x19},
	{0x30D7, 0x03},
	{0x3106, 0x11}, //DOLHBFIXEN[7] 0: pattern1 1: pattern2
	{0x3129, 0x00},
	{0x313B, 0x61},
	{0x315E, 0x1A},
	{0x3164, 0x1A},
	{0x317C, 0x00},
	{0x31EC, 0x0E},
	{0x3415, 0x00}, // NULL0_SIZE_V, set to 00h when DOL
	{0x3480, 0x49},
	{SNR_MARKER_DELAY, 3000}, // delay
	{0x3000, 0x00}, // STANDBY
};

int imx327lvds_poweron(void *priv)
{
	pattern_ctx *ctx = (pattern_ctx *)priv;
	struct sensor_info *sinf = (struct sensor_info *)ctx->ptn->priv;
	struct snr_i2c_adapter *adpt = NULL;
	struct sensor_setting *sensor_set = imx327lvds_init_1080p30_4lane;
	int setting_num = ARRAY_SIZE(imx327lvds_init_1080p30_4lane);
	struct sensor_setting *pset;
	struct sensor_mode *mode = &imx327lvds_mode[IMX327LVDS_MODE_1080P30];
	char cmdbuf[8];
	int i, r, test_pn = 0;
	uint32_t hdr = 0, tmp;
#if defined(BOARD_ASIC) && defined(CIF_DPHY_ENABLE)
	uint32_t value;
#endif

	disable_irq(CSIMAC0_INTR_NUM);
	bm_get_num("HDR OFF(0), HDR-P1(1):", hdr);

	writel(0x0a0c2000, 0x00000f01);
	switch (hdr) {
		case 0:
			// linear
			// config sub-LVDS front porch
			writel(0x0a0c2200, 0x0006020f);
			writel(0x0a0c2230, 0x00060013);
			mode = &imx327lvds_mode[IMX327LVDS_MODE_1080P30];
			sensor_set = imx327lvds_init_1080p30_4lane;
			setting_num = ARRAY_SIZE(imx327lvds_init_1080p30_4lane);
			uartlog("use 1080p30 4 lanes\n");
			sinf->ishdr = false;
			sinf->shsl.min = 1;
			sinf->shsl.max = 0x2EC;
			break;
		case 1:
			// config sub-LVDS front porch
			writel(0x0a0c2200, 0x000F120f);
#if 0
			writel(0x0a0c2218, 0x00040760);
			writel(0x0a0c221C, 0x000801D4);
			writel(0x0a0c2220, 0x040401D8);
			writel(0x0a0c2224, 0x040805D4);
			writel(0x0a0c2228, 0x000005D8);
#endif
			writel(0x0a0c2230, 0x00000013);

			mode = &imx327lvds_mode[IMX327LVDS_MODE_1080P30_HDR1];
			sensor_set = imx327lvds_init_1080p30_4lane_hdr1;
			setting_num = ARRAY_SIZE(imx327lvds_init_1080p30_4lane_hdr1);
			uartlog("use 1080p30 4 lanes HDR\n");
			sinf->ishdr = true;
			break;
		default:
			uartlog("not support mode\n");
			return -1;
	}
	cmdbuf[0] = cmdbuf[1] = '0';
	cli_readline("enable test pattern? [n/y]", cmdbuf);
	if (cmdbuf[0] == 'y') {
		test_pn = 1;
	}
	// i2c config here
	snr_bm_i2c_get_adap((void *)&adpt);
	sinf->adpt = adpt;

	adpt->slaveaddr = sinf->dev;
	snr_bm_i2c_init(adpt);

	// i2c initial sequence
	for (i = 0; i < setting_num; i++) {
		pset = &(sensor_set[i]);

		if (pset->addr == SNR_MARKER_DELAY) {
			uartlog("sensor wr delay\n");
			udelay(pset->val);
			continue;
		}
		if (pset->addr == IMX327LVDS_TEST_PATTERN_REG) {
			if (test_pn)
				pset->val = IMX327LVDS_TEST_PTN_V_COLORBAR;
			else
				pset->val = 0;
		}
		r = sinf->adpt->write(sinf->adpt, sinf->dev, pset->addr, sinf->alen, (uint8_t*)&(pset->val), 1);
		if (r < 0)
			uartlog("%s i2c_write fail addr[0x%x]\n", ctx->ptn->name, pset->addr);
	}

	uartlog("Read back i2c registers:\n");
	for (i = 0; i < setting_num; i++) {
		uint8_t val[2] = {0xFF};

		pset = &(sensor_set[i]);

		r = sinf->adpt->read(sinf->adpt, sinf->dev, pset->addr, sinf->alen, (uint8_t*)&(val[0]), 1);

		uartlog("reg[0x%x]=0x%x(0x%x)\n", pset->addr, val[0], pset->val);
	}

#if defined(BOARD_ASIC) && defined(CIF_DPHY_ENABLE)
	// Switch the CLK Select
	value = readl(0x0300B004);
	value |= ((1<<lane_map[0])<<8);
	writel(0x0300B004, value);
	// Config the PHYD lane enable
	value = readl(0x0300B028);
	value &= ~0xFF0FFFFF;
	writel(0x0300B028, value);
	// PHYD lane enable
	value |= 0x0F000000;
	// CLK INV
	value |= (0xF<<20);
	for (i=0; i<5; i++) {
		value |= (lane_map[i]&0xF)<<(i*4);
	}
	writel(0x0300B028, value);
	// PHYA lane enable
	value = readl(0x0300B000);
	value &= ~(0x1<<26);
	value &= ~(0x1F<<27);
	writel(0x0300B000, value);

	// Switch the PHY type to SUBLVDS
	value = readl(0x0300B0F0);
	value &= 0xFFFF0000;
	writel(0x0300B0F0, value);
	value |= 0xFB;
	writel(0x0300B0F0, value);
#endif

	uartlog("streaming start\n");

	cmdbuf[0] = cmdbuf[1] = '0';
	cli_readline("run sensor to dram flow? [n/y]", cmdbuf);
	if (cmdbuf[0] == 'y') {
		uartlog("dump %d x %d \n", mode->total_w, mode->total_h);
		config_csibdg(0, mode->total_w, mode->total_h, ctx,
			      (hdr == 1 || hdr == 2), 0);
	}
	//enable_irq(CSIMAC0_INTR_NUM);

	return 0;
}

int peri_up_imx327lvds(void *priv)
{
#define CAM0PLL_SYNC_SET		406720388UL
#define CAM0PLL_DIV_SEL			12
#define CAM0PLL_POST_DIV_SEL		1
#define CAM0PLL_ICTRL			0
#define CAM0PLL_SEL_MODE		1
#define CAM0PLL_PRE_DIV_SEL		1
#define CMA0PLL_CLK_DIV			32
	char cmdbuf[8];
#if defined(BOARD_ASIC) && defined(CIF_DPHY_ENABLE)
	uint32_t value;
#endif

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
	writel(HARD_REG_GPIOD7_DAT, 0x00);
	writel(HARD_REG_GPIOD7_OE, 0x80);
	// wait 500ns, wait INCK stable
	udelay(1);
	// XCLR = 1
	writel(HARD_REG_GPIOD7_DAT, 0x80);
	// wait 20us
	udelay(20);
#endif

	uartlog("### device poweron\n");

	return 0;
}

int peri_down_imx327lvds(void *priv)
{
	return 0;
}

