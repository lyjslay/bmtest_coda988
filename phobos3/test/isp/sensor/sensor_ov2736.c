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

static uint32_t	      sensor_mode;
#if defined(BOARD_ASIC) && defined(CIF_DPHY_ENABLE)
static uint32_t	use_4L = 0;
static uint8_t lane_map[5] = {4, 0, 3, 2, 1};
#endif

struct sensor_mode ov2736_mode[] = {
	{
		.name = "1920x1080",
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

struct sensor_info ov2736_info = {
	.dev = 0x36, //0x10,
	.alen = 2,
	.dlen = 1,
	.gain =  { .cur = 0, .min = 0, .max = 0xf0 },
	.sgain =  { .cur = 0, .min = 0, .max = 0xf0 },
	.shsl =  { .cur = 0, .min = 0, .max = 0 },
	.shss =  { .cur = 0, .min = 0, .max = 0 },
	.ops = &ov2736_ops,
};

static void ov2736_set_shsl(void *priv, uint32_t shs)
{
	struct sensor_info *sinf = (struct sensor_info *)priv;

	sinf->adpt->write(sinf->adpt, sinf->dev, 0x3502, sinf->alen, (uint8_t*)&(shs), 1);
	sinf->adpt->write(sinf->adpt, sinf->dev, 0x3501, sinf->alen, ((uint8_t*)&(shs)) + 1, 1);
	sinf->shsl.cur = shs;

	return;
}

static void ov2736_set_shss(void *priv, uint32_t shs)
{
	struct sensor_info *sinf = (struct sensor_info *)priv;

	sinf->adpt->write(sinf->adpt, sinf->dev, 0x3512, sinf->alen, (uint8_t*)&(shs), 1);
	sinf->adpt->write(sinf->adpt, sinf->dev, 0x3511, sinf->alen, ((uint8_t*)&(shs)) + 1, 1);
	sinf->shsl.cur = shs;

	return;
}

static void ov2736_set_gain(void *priv, uint16_t gain)
{
	struct sensor_info *sinf = (struct sensor_info *)priv;

	sinf->adpt->write(sinf->adpt, sinf->dev, 0x3509, sinf->alen, (uint8_t*)&(gain), 1);
	sinf->adpt->write(sinf->adpt, sinf->dev, 0x3508, sinf->alen, ((uint8_t*)&(gain)) + 1, 1);
	sinf->gain.cur = gain;

	return;
}

static void ov2736_set_sgain(void *priv, uint16_t gain)
{
	struct sensor_info *sinf = (struct sensor_info *)priv;

	sinf->adpt->write(sinf->adpt, sinf->dev, 0x350D, sinf->alen, (uint8_t*)&(gain), 1);
	sinf->adpt->write(sinf->adpt, sinf->dev, 0x350C, sinf->alen, ((uint8_t*)&(gain)) + 1, 1);
	sinf->shsl.cur = gain;

	return;
}

struct sensor_ops ov2736_ops = {
	.set_gain = ov2736_set_gain,
	.set_sgain = ov2736_set_sgain,
	.set_shsl = ov2736_set_shsl,
	.set_shss = ov2736_set_shss,
};

#define OV2736_TEST_PATTERN_REG       0x5081
#define OV2736_TEST_PTN_DISABLE       0x00
//#define OV2736_TEST_PTN_COLOR_BAR     0x80
#define OV2736_TEST_PTN_COLOR_BAR     0x92
#define OV2736_DEFAULT_SHS		0x1F

struct sensor_setting ov2736_init_1920x1080_2lane[] = {
	{0x0103, 0x01},
	{0x303, 0x03},
	{0x305, 0x48},
	{0x307, 0x00},
	{0x308, 0x03},
	{0x309, 0x03},
	{0x323, 0x03},
	{0x325, 0x48},
	{0x327, 0x07},
	{0x3016, 0x32},
	{0x3000, 0x00},
	{0x3001, 0x00},
	{0x3002, 0x00},
	{0x3013, 0x00},
	{0x301f, 0xf0},
	{0x3023, 0xf0},
	{0x3020, 0x9b},
	{0x3022, 0x51},
	{0x3106, 0x11},
	{0x3107, 0x01},
	{0x3500, 0x00},
	{0x3501, 0x40},
	{0x3502, 0x00},
	{0x3503, 0x88},
	{0x3505, 0x83},
	{0x3508, 0x01},
	{0x3509, 0x80},
	{0x350a, 0x04},
	{0x350b, 0x00},
	{0x350c, 0x00},
	{0x350d, 0x80},
	{0x350e, 0x04},
	{0x350f, 0x00},
	{0x3510, 0x00},
	{0x3511, 0x00},
	{0x3512, 0x20},
	{0x3600, 0x55},
	{0x3601, 0x52},
	{0x3612, 0xb5},
	{0x3613, 0xb3},
	{0x3616, 0x83},
	{0x3621, 0x00},
	{0x3624, 0x06},
	{0x3642, 0x88},
	{0x3660, 0x00},
	{0x3661, 0x00},
	{0x366a, 0x64},
	{0x366c, 0x00},
	{0x366e, 0xff},
	{0x366f, 0xff},
	{0x3677, 0x11},
	{0x3678, 0x11},
	{0x3679, 0x0c},
	{0x3680, 0xff},
	{0x3681, 0x16},
	{0x3682, 0x16},
	{0x3683, 0x90},
	{0x3684, 0x90},
	{0x3768, 0x04},
	{0x3769, 0x20},
	{0x376a, 0x04},
	{0x376b, 0x20},
	{0x3620, 0x80},
	{0x3662, 0x10},
	{0x3663, 0x24},
	{0x3665, 0xa0},
	{0x3667, 0xa6},
	{0x3674, 0x01},
	{0x373d, 0x24},
	{0x3741, 0x28},
	{0x3743, 0x28},
	{0x3745, 0x28},
	{0x3747, 0x28},
	{0x3748, 0x00},
	{0x3749, 0x78},
	{0x374a, 0x00},
	{0x374b, 0x78},
	{0x374c, 0x00},
	{0x374d, 0x78},
	{0x374e, 0x00},
	{0x374f, 0x78},
	{0x3766, 0x12},
	{0x37e0, 0x00},
	{0x37e6, 0x04},
	{0x37e5, 0x04},
	{0x37e1, 0x04},
	{0x3737, 0x04},
	{0x37d0, 0x0a},
	{0x37d8, 0x04},
	{0x37e2, 0x08},
	{0x3739, 0x10},
	{0x37e4, 0x18},
	{0x37e3, 0x04},
	{0x37d9, 0x10},
	{0x4040, 0x04},
	{0x4041, 0x0f},
	{0x4008, 0x00},
	{0x4009, 0x0d},
	{0x37a1, 0x14},
	{0x37a8, 0x16},
	{0x37ab, 0x10},
	{0x37c2, 0x04},
	{0x3705, 0x00},
	{0x3706, 0x28},
	{0x370a, 0x00},
	{0x370b, 0x78},
	{0x3714, 0x24},
	{0x371a, 0x1e},
	{0x372a, 0x03},
	{0x3756, 0x00},
	{0x3757, 0x0e},
	{0x377b, 0x00},
	{0x377c, 0x0c},
	{0x377d, 0x20},
	{0x3790, 0x28},
	{0x3791, 0x78},
	{0x3800, 0x00},
	{0x3801, 0x00},
	{0x3802, 0x00},
	{0x3803, 0x04},
	{0x3804, 0x07},
	{0x3805, 0x8f},
	{0x3806, 0x04},
	{0x3807, 0x43},
	{0x3808, 0x07},
	{0x3809, 0x80},
	{0x380a, 0x04},
	{0x380b, 0x38},
	{0x380c, 0x02},
	{0x380d, 0x78},
	{0x380e, 0x04},
	{0x380f, 0xa0},
	{0x3811, 0x06},
	{0x3813, 0x04},
	{0x3814, 0x01},
	{0x3815, 0x01},
	{0x3816, 0x01},
	{0x3817, 0x01},
	{0x381d, 0x40},
	{0x381e, 0x02},
	{0x3820, 0x88},
	{0x3821, 0x00},
	{0x3822, 0x04},
	{0x3835, 0x00},
	{0x4303, 0x19},
	{0x4304, 0x19},
	{0x4305, 0x03},
	{0x4306, 0x81},
	{0x4503, 0x00},
	{0x4508, 0x14},
	{0x450a, 0x00},
	{0x450b, 0x40},
	{0x4833, 0x08},
	{0x5001, 0x09},
	{0x3b00, 0x00},
	{0x3b02, 0x00},
	{0x3b03, 0x00},
	{0x3c80, 0x08},
	{0x3c82, 0x00},
	{0x3c83, 0xb1},
	{0x3c87, 0x08},
	{0x3c8c, 0x10},
	{0x3c8d, 0x00},
	{0x3c90, 0x00},
	{0x3c91, 0x00},
	{0x3c92, 0x00},
	{0x3c93, 0x00},
	{0x3c94, 0x00},
	{0x3c95, 0x00},
	{0x3c96, 0x00},
	{0x3c97, 0x00},
	{0x3c98, 0x00},
	{0x4000, 0xf3},
	{0x4001, 0x60},
	{0x4002, 0x00},
	{0x4003, 0x40},
	{0x4090, 0x14},
	{0x4601, 0x10},
	{0x4701, 0x00},
	{0x4708, 0x09},
	{0x470a, 0x00},
	{0x470b, 0x40},
	{0x470c, 0x81},
	{0x480c, 0x12},
	{0x4710, 0x06},
	{0x4711, 0x00},
	{0x4837, 0x12},
	{0x4800, 0x00},
	{0x4c01, 0x00},
	{0x5036, 0x00},
	{0x5037, 0x00},
	{0x580b, 0x0f},
	{0x4903, 0x80},
	{0x484b, 0x05},
	{0x400a, 0x00},
	{0x400b, 0x90},
	{0x5000, 0x81},
	{0x5200, 0x18},
	{0x4837, 0x16},
	{0x380e, 0x04},
	{0x380f, 0xa0},
	{0x3500, 0x00},
	{0x3501, 0x49},
	{0x3502, 0x80},
	{0x3508, 0x02},
	{0x3509, 0x80},
	{0x3d8c, 0x11},
	{0x3d8d, 0xf0},
	{0x5180, 0x00},
	{0x5181, 0x10},
	{0x36a0, 0x16},
	{0x36a1, 0x50},
	{0x36a2, 0x60},
	{0x36a3, 0x80},
	{0x36a4, 0x00},
	{0x36a5, 0xa0},
	{0x36a6, 0x00},
	{0x36a7, 0x50},
	{0x36a8, 0x00},
	{0x36a9, 0x50},
	{0x36aa, 0x00},
	{0x36ab, 0x50},
	{0x36ac, 0x00},
	{0x36ad, 0x50},
	{0x36ae, 0x00},
	{0x36af, 0x50},
	{0x36b0, 0x00},
	{0x36b1, 0x50},
	{0x36b2, 0x00},
	{0x36b3, 0x50},
	{0x36b4, 0x00},
	{0x36b5, 0x50},
	{0x36b9, 0xee},
	{0x36ba, 0xee},
	{0x36bb, 0xee},
	{0x36bc, 0xee},
	{0x36bd, 0x0e},
	{0x36b6, 0x08},
	{0x36b7, 0x08},
	{0x36b8, 0x10},
	{0x3808, 0x04},
	{0x3809, 0x38},
	{0x3810, 0x01},
	{0x3811, 0xaa},
	{0x380c, 0x04},
	{0x380d, 0xf0},
	{0x0100, 0x01},
};

int ov2736_poweron(void *priv)
{
	pattern_ctx *ctx = (pattern_ctx *)priv;
	struct sensor_info *sinf = (struct sensor_info *)ctx->ptn->priv;
	struct snr_i2c_adapter *adpt = NULL;
	struct sensor_setting *sensor_set = ov2736_init_1920x1080_2lane;
	int setting_num = ARRAY_SIZE(ov2736_init_1920x1080_2lane);
	struct sensor_setting *pset;
	char cmdbuf[8];
	struct sensor_mode *mode = &ov2736_mode[OV2736_MODE_1920_1080];
	int i, r, test_pn = 0;
	uint32_t value;
	uint32_t hdr = 0;

#if defined(BOARD_ASIC) && defined(CIF_DPHY_ENABLE)
	// Switch the PHY type to MIPI-CSI
	// uartlog("start phya reset\n");
	value = readl(0x0300B0F0);
	value &= ~0x1;
	writel(0x0300B0F0, value);
	// Switch the CLK Select
	value = readl(0x0300B004);
	value |= ((1<<lane_map[0])<<8);
	writel(0x0300B004, value);
	// Config the PHYD lane enable
	value = readl(0x0300B028);
	value &= ~0xFF0FFFFF;
	writel(0x0300B028, value);
	// PHYD lane enable
	value |= (use_4L ? 0x0F000000 :0x03000000);
	// CLK INV
	value |= (use_4L ? (0xF<<20) : (0x3<<20));
	for (i=0; i<5; i++) {
		value |= (lane_map[i]&0xF)<<(i*4);
	}
	writel(0x0300B028, value);
	// PHYA lane enable
	value = readl(0x0300B000);
	value &= ~(0x1<<26);
	value &= ~(0x1F<<27);
	writel(0x0300B000, value);
#endif

	// i2c config here
	snr_bm_i2c_get_adap((void *)&adpt);
	sinf->adpt = adpt;

	adpt->slaveaddr = sinf->dev;
	snr_bm_i2c_init(adpt);

	// linear
	uartlog("1920x1080 2 lanes\n");
	sensor_set = ov2736_init_1920x1080_2lane;
	setting_num = ARRAY_SIZE(ov2736_init_1920x1080_2lane);
	mode = &ov2736_mode[OV2736_MODE_1920_1080];
	writel(0x0a0c2040, 0x0);

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

	value = test_pn ? OV2736_TEST_PTN_COLOR_BAR : OV2736_TEST_PTN_DISABLE;
	r = sinf->adpt->write(sinf->adpt, sinf->dev, OV2736_TEST_PATTERN_REG,
			      sinf->alen, (uint8_t*)&value, 1);
	if (r < 0)
		uartlog("%s i2c_write fail addr[0x%x]\n", ctx->ptn->name, OV2736_TEST_PATTERN_REG);


	uartlog("Read back i2c registers:\n");
	for (i = 0; i < setting_num; i++) {
		uint16_t val = 0;

		pset = &(sensor_set[i]);
		r = sinf->adpt->read(sinf->adpt, sinf->dev, pset->addr, sinf->alen, (uint8_t*)&val, 1);

		uartlog("reg[0x%x]=0x%x(0x%x)\n", pset->addr, val, pset->val);
	}

#if defined(BOARD_FPGA) && defined(CIF_DPHY_ENABLE)
	// Switch the PHY type to MIPI-CSI
	value = readl(0x0300B0F0);
	value &= ~0x1;
	writel(0x0300B0F0, value);
	// Config the PHYD lane enable
	value = readl(0x0300B028);
	value &= ~0xFF000000;
	writel(0x0300B028, value);
	value |= 0x0f000000;
	writel(0x0300B028, value);
#endif

	uartlog("streaming start\n");

	cmdbuf[0] = cmdbuf[1] = '0';
	cli_readline("run sensor to dram flow? [n/y]", cmdbuf);
	if (cmdbuf[0] == 'y') {
		unsigned int lines;
		
		if (sensor_mode == 0)
			lines = hdr==1?mode->total_h+sinf->shsl.cur+49:mode->total_h;
		else
			lines = hdr==1?mode->total_h+sinf->shsl.cur+57:mode->total_h;
		uartlog("dump %d x %d \n", mode->total_w, lines);
		config_csibdg(0, mode->total_w,
				lines,
				ctx, !!hdr, 0);
	}
	return 0;
}

int ov2736_intr(void *priv)
{
	uint32_t value;

	value = readl(0x0a0c2460);
	uartlog("csi status intr 0x%x\n", value);
	// clear the interrupt status
	value = readl(0x0a0c2404) | ((value&0xFF)<<8);
	writel(0x0a0c2404, value);

	return 0;
}

int peri_up_ov2736(void *priv)
{
#define CAM0PLL_SYNC_SET		393705325UL
#define CAM0PLL_DIV_SEL			11
#define CAM0PLL_POST_DIV_SEL		1
#define CAM0PLL_ICTRL			0
#define CAM0PLL_SEL_MODE		1
#define CAM0PLL_PRE_DIV_SEL		1
#define CMA0PLL_CLK_DIV			45

	char cmdbuf[8];
#if !defined(FLOW_AFTER_ECO)
	unsigned int *fpga_pwr_reg;
#endif
	uint32_t value;

	cmdbuf[0] = '0';
	cli_readline("### enter to start power-on seqeuence. s) to skip : ", cmdbuf);
	if (cmdbuf[0] == 's')
		return 0;
#if defined(BOARD_ASIC)
	// power on/reset
	// 1.2V = 1, 1.8V = 1, 2.9V = 1
	// INCK on
	// set the vip to full speec
	writel(0x0A0C8030, 0x21084210);
	writel(0x0A0C8034, 0x00084210);
	// select the source of mac0 to vip_sys_2
	value = readl(0x0a0c801C);
	value &= ~(0x03<<4);
	value |= (0x2<<4);
	writel(0x0a0c801C, value);
	// set vip_sys_2 dividor
	writel(0x03002110, (2<<16)|0x09);
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
	//writel(HARD_REG_GPIOD7_DAT, 0x80);
	//writel(HARD_REG_GPIOD7_OE, 0x80);
	// wait 500ns, wait INCK stable
	//udelay(1);
	// XCLR = 1
	//writel(HARD_REG_GPIOD7_DAT, 0x00);
	// wait 20us
	udelay(2000000);
#endif

#if !defined(FLOW_AFTER_ECO)
	// enable display clk select
	value = readl(0x0a0c8018);
	value |= 0x02;
	writel(0x0a0c8018, value);
	udelay(5000);
#endif

	// Switch the MAC type to MIPI-CSI
	writel(0x0a0c2000, 0x000000F0);
	value = readl(0x0a0c2400);
	// config lane number
	value |= 0x03;
	// config force video format
	// value |= (0x8<<16) | (0x01<<13);
	// Enable SOL
	//value |= (0x1<<12);
	writel(0x0a0c2400, value);
	// config the vs gen mode.
	writel(0x0a0c2470, 0x00);
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

int peri_down_ov2736(void *priv)
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

