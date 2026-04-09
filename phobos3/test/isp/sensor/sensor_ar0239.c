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



#undef uartlog
#define uartlog(fmt, args...) \
{ \
    printf(fmt, ##args); \
    fflush(stdout); \
}

#define CIF_DPHY_ENABLE

struct sensor_mode ar0239_mode[] = {
	{
		.name = "720p30",
		.bit_depth = 12,
		.bayer = SENSOR_BAYER_GR,
		.active_w = 1280,
		.active_h = 720,
		.total_w = 1280,
		.total_h = 720,
		.fp_w = 0,
		.bp_w = 0,
		.fp_h = 0,
		.bp_h = 0,	// in spec it shall be 5 but csi-bdg report frame err.
	},
};

struct sensor_info ar0239_info = {
	.dev = 0x10, //0x30,
	.alen = 2,
	.dlen = 2,
	.gain =  { .cur = 0, .min = 0, .max = 0xf0 },
	.sgain =  { .cur = 0, .min = 0, .max = 0xf0 },
	.shsl =  { .cur = 0, .min = 0, .max = 0 },
	.shss =  { .cur = 0, .min = 0, .max = 0 },
	.ops = &ar0239_ops,
};

static void ar0239_set_shsl(void *priv, uint32_t shs)
{
	struct sensor_info *sinf = (struct sensor_info *)priv;

	shs = ((shs&0xFF)<<8) | ((shs>>8)&0xFF);
	sinf->adpt->write(sinf->adpt, sinf->dev, 0x3012, sinf->alen, (uint8_t*)&(shs), 2);
	sinf->shsl.cur = shs;

	return;
}

static void ar0239_set_gain(void *priv, uint16_t gain)
{
	struct sensor_info *sinf = (struct sensor_info *)priv;
	uint16_t value = 0x0004;

	value = ((value&0xFF)<<8) | ((value>>8)&0xFF);
	gain = ((gain&0xFF)<<8) | ((gain>>8)&0xFF);
	sinf->adpt->write(sinf->adpt, sinf->dev, 0x3100, sinf->alen, (uint8_t*)&(value), 2);
	sinf->adpt->write(sinf->adpt, sinf->dev, 0x3060, sinf->alen, (uint8_t*)&(gain), 2);
	sinf->gain.cur = gain;

	return;
}

struct sensor_ops ar0239_ops = {
	.set_gain = ar0239_set_gain,
	.set_shsl = ar0239_set_shsl,
};

static void sensor_data_convert(struct sensor_setting *data, int size)
{
	struct sensor_setting *s;
	int i;

	for (i = 0, s = data; i < size; i++, s++) {
		uint16_t tmp = ((s->val & 0xFF) << 8) | (s->val >> 8);
		s->val = tmp;
	}
}

struct sensor_setting ar0239_init_720p[] = {
	{0x301A, 0x0001}, // RESET_REGISTER
	{SNR_MARKER_DELAY, 200}, /* delay */
//Linear Mode Setup
	{0x3088, 0x8000}, // SEQ_CTRL_PORT
	{0x3086, 0x4558}, // SEQ_DATA_PORT
	{0x3086, 0x72A6}, // SEQ_DATA_PORT
	{0x3086, 0x4A31}, // SEQ_DATA_PORT
	{0x3086, 0x43C5}, // SEQ_DATA_PORT
	{0x3086, 0x2A08}, // SEQ_DATA_PORT
	{0x3086, 0x42CE}, // SEQ_DATA_PORT
	{0x3086, 0xCB8E}, // SEQ_DATA_PORT
	{0x3086, 0x0345}, // SEQ_DATA_PORT
	{0x3086, 0x782A}, // SEQ_DATA_PORT
	{0x3086, 0x1C7B}, // SEQ_DATA_PORT
	{0x3086, 0xCA3D}, // SEQ_DATA_PORT
	{0x3086, 0xFF3D}, // SEQ_DATA_PORT
	{0x3086, 0xFF3D}, // SEQ_DATA_PORT
	{0x3086, 0xEA2A}, // SEQ_DATA_PORT
	{0x3086, 0x0C3D}, // SEQ_DATA_PORT
	{0x3086, 0x102A}, // SEQ_DATA_PORT
	{0x3086, 0x0D2A}, // SEQ_DATA_PORT
	{0x3086, 0x1D35}, // SEQ_DATA_PORT
	{0x3086, 0x2A0D}, // SEQ_DATA_PORT
	{0x3086, 0x3D10}, // SEQ_DATA_PORT
	{0x3086, 0x4558}, // SEQ_DATA_PORT
	{0x3086, 0x2A0C}, // SEQ_DATA_PORT
	{0x3086, 0x8E03}, // SEQ_DATA_PORT
	{0x3086, 0x2A1C}, // SEQ_DATA_PORT
	{0x3086, 0x3DFF}, // SEQ_DATA_PORT
	{0x3086, 0x3DFF}, // SEQ_DATA_PORT
	{0x3086, 0x3DEA}, // SEQ_DATA_PORT
	{0x3086, 0x2A0C}, // SEQ_DATA_PORT
	{0x3086, 0x62CC}, // SEQ_DATA_PORT
	{0x3086, 0xCF2A}, // SEQ_DATA_PORT
	{0x3086, 0x288E}, // SEQ_DATA_PORT
	{0x3086, 0x0036}, // SEQ_DATA_PORT
	{0x3086, 0x2A08}, // SEQ_DATA_PORT
	{0x3086, 0x3D64}, // SEQ_DATA_PORT
	{0x3086, 0x7ACD}, // SEQ_DATA_PORT
	{0x3086, 0x3D04}, // SEQ_DATA_PORT
	{0x3086, 0x442A}, // SEQ_DATA_PORT
	{0x3086, 0x982C}, // SEQ_DATA_PORT
	{0x3086, 0x4BA4}, // SEQ_DATA_PORT
	{0x3086, 0xC257}, // SEQ_DATA_PORT
	{0x3086, 0x0043}, // SEQ_DATA_PORT
	{0x3086, 0x0C2D}, // SEQ_DATA_PORT
	{0x3086, 0x6343}, // SEQ_DATA_PORT
	{0x3086, 0x5FC4}, // SEQ_DATA_PORT
	{0x3086, 0xC52A}, // SEQ_DATA_PORT
	{0x3086, 0xFCC8}, // SEQ_DATA_PORT
	{0x3086, 0xC553}, // SEQ_DATA_PORT
	{0x3086, 0x0553}, // SEQ_DATA_PORT
	{0x3086, 0x078E}, // SEQ_DATA_PORT
	{0x3086, 0x034D}, // SEQ_DATA_PORT
	{0x3086, 0x2BF8}, // SEQ_DATA_PORT
	{0x3086, 0x2924}, // SEQ_DATA_PORT
	{0x3086, 0x29AE}, // SEQ_DATA_PORT
	{0x3086, 0x3F07}, // SEQ_DATA_PORT
	{0x3086, 0x45DC}, // SEQ_DATA_PORT
	{0x3086, 0x5C19}, // SEQ_DATA_PORT
	{0x3086, 0x494C}, // SEQ_DATA_PORT
	{0x3086, 0x095C}, // SEQ_DATA_PORT
	{0x3086, 0x1B2A}, // SEQ_DATA_PORT
	{0x3086, 0xFE8E}, // SEQ_DATA_PORT
	{0x3086, 0x0F3F}, // SEQ_DATA_PORT
	{0x3086, 0x032B}, // SEQ_DATA_PORT
	{0x3086, 0xB82B}, // SEQ_DATA_PORT
	{0x3086, 0x984E}, // SEQ_DATA_PORT
	{0x3086, 0x29BA}, // SEQ_DATA_PORT
	{0x3086, 0x6016}, // SEQ_DATA_PORT
	{0x3086, 0x45CC}, // SEQ_DATA_PORT
	{0x3086, 0x1645}, // SEQ_DATA_PORT
	{0x3086, 0x8C3F}, // SEQ_DATA_PORT
	{0x3086, 0x088E}, // SEQ_DATA_PORT
	{0x3086, 0x0D5C}, // SEQ_DATA_PORT
	{0x3086, 0x0B2A}, // SEQ_DATA_PORT
	{0x3086, 0xFA3F}, // SEQ_DATA_PORT
	{0x3086, 0x325C}, // SEQ_DATA_PORT
	{0x3086, 0x033F}, // SEQ_DATA_PORT
	{0x3086, 0x233E}, // SEQ_DATA_PORT
	{0x3086, 0x105F}, // SEQ_DATA_PORT
	{0x3086, 0x2AF2}, // SEQ_DATA_PORT
	{0x3086, 0x2B90}, // SEQ_DATA_PORT
	{0x3086, 0x2B80}, // SEQ_DATA_PORT
	{0x3086, 0xC73F}, // SEQ_DATA_PORT
	{0x3086, 0x0860}, // SEQ_DATA_PORT
	{0x3086, 0x29AA}, // SEQ_DATA_PORT
	{0x3086, 0xD029}, // SEQ_DATA_PORT
	{0x3086, 0x835F}, // SEQ_DATA_PORT
	{0x3086, 0x4D19}, // SEQ_DATA_PORT
	{0x3086, 0x45AC}, // SEQ_DATA_PORT
	{0x3086, 0x2AFA}, // SEQ_DATA_PORT
	{0x3086, 0xC63E}, // SEQ_DATA_PORT
	{0x3086, 0x052A}, // SEQ_DATA_PORT
	{0x3086, 0xFB2A}, // SEQ_DATA_PORT
	{0x3086, 0xFB5D}, // SEQ_DATA_PORT
	{0x3086, 0x8810}, // SEQ_DATA_PORT
	{0x3086, 0x3F06}, // SEQ_DATA_PORT
	{0x3086, 0x4588}, // SEQ_DATA_PORT
	{0x3086, 0x2992}, // SEQ_DATA_PORT
	{0x3086, 0x212B}, // SEQ_DATA_PORT
	{0x3086, 0x0485}, // SEQ_DATA_PORT
	{0x3086, 0x3F04}, // SEQ_DATA_PORT
	{0x3086, 0x2AFA}, // SEQ_DATA_PORT
	{0x3086, 0x8E09}, // SEQ_DATA_PORT
	{0x3086, 0x8D4D}, // SEQ_DATA_PORT
	{0x3086, 0x4E48}, // SEQ_DATA_PORT
	{0x3086, 0x2B80}, // SEQ_DATA_PORT
	{0x3086, 0x4C0C}, // SEQ_DATA_PORT
	{0x3086, 0x603F}, // SEQ_DATA_PORT
	{0x3086, 0x30C7}, // SEQ_DATA_PORT
	{0x3086, 0xC33F}, // SEQ_DATA_PORT
	{0x3086, 0x052A}, // SEQ_DATA_PORT
	{0x3086, 0xF23F}, // SEQ_DATA_PORT
	{0x3086, 0x083F}, // SEQ_DATA_PORT
	{0x3086, 0x0829}, // SEQ_DATA_PORT
	{0x3086, 0x82C6}, // SEQ_DATA_PORT
	{0x3086, 0xD0D2}, // SEQ_DATA_PORT
	{0x3086, 0x4558}, // SEQ_DATA_PORT
	{0x3086, 0xD35F}, // SEQ_DATA_PORT
	{0x3086, 0x4D19}, // SEQ_DATA_PORT
	{0x3086, 0x2AFA}, // SEQ_DATA_PORT
	{0x3086, 0xC92A}, // SEQ_DATA_PORT
	{0x3086, 0x988E}, // SEQ_DATA_PORT
	{0x3086, 0x0012}, // SEQ_DATA_PORT
	{0x3086, 0x444A}, // SEQ_DATA_PORT
	{0x3086, 0x0443}, // SEQ_DATA_PORT
	{0x3086, 0x1605}, // SEQ_DATA_PORT
	{0x3086, 0x4316}, // SEQ_DATA_PORT
	{0x3086, 0x5843}, // SEQ_DATA_PORT
	{0x3086, 0x165A}, // SEQ_DATA_PORT
	{0x3086, 0x4316}, // SEQ_DATA_PORT
	{0x3086, 0x0643}, // SEQ_DATA_PORT
	{0x3086, 0x1607}, // SEQ_DATA_PORT
	{0x3086, 0x4316}, // SEQ_DATA_PORT
	{0x3086, 0x8E03}, // SEQ_DATA_PORT
	{0x3086, 0x4578}, // SEQ_DATA_PORT
	{0x3086, 0x3F05}, // SEQ_DATA_PORT
	{0x3086, 0x2A9D}, // SEQ_DATA_PORT
	{0x3086, 0x5D88}, // SEQ_DATA_PORT
	{0x3086, 0x112B}, // SEQ_DATA_PORT
	{0x3086, 0x0453}, // SEQ_DATA_PORT
	{0x3086, 0x0D86}, // SEQ_DATA_PORT
	{0x3086, 0x2944}, // SEQ_DATA_PORT
	{0x3086, 0x8D4D}, // SEQ_DATA_PORT
	{0x3086, 0x2B64}, // SEQ_DATA_PORT
	{0x3086, 0x3F20}, // SEQ_DATA_PORT
	{0x3086, 0x4558}, // SEQ_DATA_PORT
	{0x3086, 0x253F}, // SEQ_DATA_PORT
	{0x3086, 0x058E}, // SEQ_DATA_PORT
	{0x3086, 0x002A}, // SEQ_DATA_PORT
	{0x3086, 0x9812}, // SEQ_DATA_PORT
	{0x3086, 0x444B}, // SEQ_DATA_PORT
	{0x3086, 0xC1A6}, // SEQ_DATA_PORT
	{0x3086, 0xB92C}, // SEQ_DATA_PORT
	{0x3086, 0x2C2C}, // SEQ_DATA_PORT
	{0x3086, 0x2C2C}, // SEQ_DATA_PORT

//rev 2 Initialize setting-Parallel Linear
	{0x3EFC, 0xE0FF}, // DAC_LD_48_49
	{0x30AE, 0x0003}, // X_ODD_INC_CB
	{0x3092, 0x086F}, // ROW_NOISE_CONTROL
	{0x30BA, 0x762C}, // DIGITAL_CTRL
	{0x317E, 0x8F70}, // DELTA_DK_CLIP
	{0x3180, 0x8040}, // DELTA_DK_CONTROL
	{0x3ED2, 0x0099}, // DAC_LD_6_7
	{0x3ED6, 0x14A1}, // DAC_LD_10_11
	{0x3ED8, 0xF9EE}, // DAC_LD_12_13
	{0x3EDA, 0x6E40}, // DAC_LD_14_15
	{0x3EDC, 0x08C5}, // DAC_LD_16_17
	{0x3EDE, 0x11A1}, // DAC_LD_18_19
	{0x3EE0, 0x118A}, // DAC_LD_20_21
	{0x3EE2, 0x31C6}, // DAC_LD_22_23
	{0x3EE4, 0x0D77}, // DAC_LD_24_25
	{0x3EFE, 0x96F2}, // DAC_LD_50_51
	{0x3F04, 0x0C18}, // DAC_LD_56_57
	{0x3ED0, 0xC400}, // DAC_LD_4_5
	{0x3ED4, 0x03CF}, // DAC_LD_8_9
	{0x3ECE, 0x40F3}, // DAC_LD_2_3
	{0x3F02, 0x2E1C}, // DAC_LD_54_55
	{0x3EF4, 0x5805}, // DAC_LD_40_41
	{0x3EF2, 0x1465}, // DAC_LD_38_39
	{0x3EF2, 0x9465}, // DAC_LD_38_39
	{0x3EF2, 0x9464}, // DAC_LD_38_39
	{0x3EF4, 0x5805}, // DAC_LD_40_41
	{0x3EF4, 0x5805}, // DAC_LD_40_41
	{0x3EF4, 0x5805}, // DAC_LD_40_41
	{0x3EF6, 0x1252}, // DAC_LD_42_43
	{0x3EF6, 0x5252}, // DAC_LD_42_43
	{0x3EF6, 0x5253}, // DAC_LD_42_43
	{0x3EF8, 0x14F2}, // DAC_LD_44_45
	{0x3EF8, 0x54F2}, // DAC_LD_44_45
	{0x3EF8, 0x54F4}, // DAC_LD_44_45
	{0x3F4C, 0x4B3C}, // PIX_DEF_1D_DDC_LO_DEF
	{0x3F4C, 0x52BC}, // PIX_DEF_1D_DDC_LO_DEF
	{0x3F4E, 0x573C}, // PIX_DEF_1D_DDC_HI_DEF
	{0x3F4E, 0x3E3C}, // PIX_DEF_1D_DDC_HI_DEF
	{0x3F50, 0x400B}, // PIX_DEF_1D_DDC_EDGE
	{0x3F50, 0x380B}, // PIX_DEF_1D_DDC_EDGE
	{0x31E0, 0x000F}, // PIX_DEF_ID
	{0x3324, 0x001C}, // ATTENUATION_CTRL
	{0x30B4, 0x01C1}, // TEMPSENS_CTRL_REG
	{0x3ED8, 0xFAEE}, // DAC_LD_12_13

//PLL_PIXCLK_44.86MHz
	{0x31AC, 0x0C0C}, // DATA_FORMAT_BITS
#if 0
	{0x302A, 0x0008}, //VT_PIX_CLK_DIV = 8
	{0x302C, 0x0001}, //VT_SYS_CLK_DIV = 1
	{0x302E, 0x0009}, //PRE_PLL_CLK_DIV = 9
	{0x3030, 0x0057}, //A0 //PLL_MULTIPLIER = 87
	{0x3036, 0x000C}, //OP_PIX_CLK_DIV = 12
	{0x3038, 0x0001}, //OP_SYS_CLK_DIV = 1
#endif
	{0x302A, 0x0008}, //VT_PIX_CLK_DIV = 8
	{0x302C, 0x0001}, //VT_SYS_CLK_DIV = 1
	{0x302E, 0x0008}, //PRE_PLL_CLK_DIV = 8
	{0x3030, 0x0072}, //A0 //PLL_MULTIPLIER = 114
	{0x3036, 0x000C}, //OP_PIX_CLK_DIV = 12
	{0x3038, 0x0001}, //OP_SYS_CLK_DIV = 1
//Operation Mode
	{0x3082, 0x0001}, // OPERATION_MODE_CTRL
	{0x318E, 0x0000}, // HDR_MC_CTRL3
	{0x30B0, 0x123A}, // DIGITAL_TEST
	{0x31AC, 0x0C0C}, // DATA_FORMAT_BITS
	{0x31D0, 0x0000}, // COMPANDING
	{0x3004, 0x0148}, //X_ADDR_START = 328
	{0x3008, 0x0647}, //X_ADDR_END = 1607
	{0x3002, 0x00EA}, //Y_ADDR_START = 234
	{0x3006, 0x03B9}, //Y_ADDR_END = 953
	{0x300C, 0x01BC}, //LINE_LENGTH_PCK = 444
	{0x300A, 0x034A}, //FRAME_LENGTH_LINES = 842
	{0x3012, 0x0346}, //COARSE_INTEGRATION_TIME

//Parallel setting
	{0x31AE, 0x0001}, // SERIAL_FORMAT
	{0x301A, 0x10D8}, // RESET_REGISTER
	{0x3ED8, 0xFAEE}, // DAC_LD_12_13


	{0x3064, 0x1802}, // SMIA_TEST
	{0x301A, 0x10DC}, // RESET_REGISTER
	{}
};

int ar0239_poweron(void *priv)
{
	pattern_ctx *ctx = (pattern_ctx *)priv;
	struct sensor_info *sinf = (struct sensor_info *)ctx->ptn->priv;
	struct snr_i2c_adapter *adpt = NULL;
	struct sensor_setting *pset;
	char cmdbuf[8];
	struct sensor_mode *mode = &ar0239_mode[AR0239_MODE_720P30];
	int i, r;

	// convert sensor data
	sensor_data_convert(ar0239_init_720p, ARRAY_SIZE(ar0239_init_720p));
	// i2c config here
	snr_bm_i2c_get_adap((void *)&adpt);
	sinf->adpt = adpt;

	adpt->slaveaddr = sinf->dev;
	snr_bm_i2c_init(adpt);

	// i2c initial sequence
	for (i = 0; i < ARRAY_SIZE(ar0239_init_720p); i++) {
		pset = &(ar0239_init_720p[i]);

		if (pset->addr == SNR_MARKER_DELAY) {
			uartlog("sensor wr delay\n");
			udelay(pset->val);
			continue;
		}
		r = sinf->adpt->write(sinf->adpt, sinf->dev, pset->addr, sinf->alen, (uint8_t*)&(pset->val), 2);
		if (r < 0)
			uartlog("%s i2c_write fail addr[0x%x]\n", ctx->ptn->name, pset->addr);
	}

	uartlog("Read back i2c registers:\n");
	for (i = 0; i < ARRAY_SIZE(ar0239_init_720p); i++) {
		uint16_t val = 0;

		pset = &(ar0239_init_720p[i]);
		// skip SEQ_DATA_PORT
		if (pset->addr == 0x3086)
			continue;
		r = sinf->adpt->read(sinf->adpt, sinf->dev, pset->addr, sinf->alen, (uint8_t*)&val, 2);

		uartlog("reg[0x%x]=0x%x(0x%x)\n", pset->addr, val, pset->val);
	}

	uartlog("streaming start\n");

	cmdbuf[0] = cmdbuf[1] = '0';
	cli_readline("run sensor to dram flow? [n/y]", cmdbuf);
	if (cmdbuf[0] == 'y') {
		uartlog("dump %d x %d \n", mode->total_w, mode->total_h);
		config_csibdg(0, mode->total_w, mode->total_h, ctx, 0, 0);
	}
	// restore sensor data
	sensor_data_convert(ar0239_init_720p, ARRAY_SIZE(ar0239_init_720p));
	return 0;
}

int peri_up_3(void *priv)
{

	char cmdbuf[8];
	uint32_t pinmux;
#if !defined(FLOW_AFTER_ECO)
	unsigned int *fpga_pwr_reg;
	uint32_t value;
#endif

	cmdbuf[0] = '0';
	cli_readline("### enter to start power-on seqeuence. s) to skip : ", cmdbuf);
	if (cmdbuf[0] == 's')
		return 0;

	// power down/reset
	// 1.2V = 1, 1.8V = 1, 2.9V = 1
	// INCK on
	// XCLR = 0
	// wait 500ns, wait INCK stable
	// XCLR = 1
	// wait 20us

#if !defined(FLOW_AFTER_ECO)
	// enable display clk select
	value = readl(0x0a0c8018);
	value |= 0x02;
	writel(0x0a0c8018, value);
	udelay(5000);
#endif

	// Switch the MAC type to TTL
	writel(0x0a0c2000, 0x00000003);
	// configure TTL [0] ttl enable [2:1] bit mode
	writel(0x0a0c2010, 0x00000C05);
	// configure the raw mode, inverse the clock.
	writel(0x0a0c2030, 0x00000011);
	// configure pinmux, vde/hde
	pinmux = (0x0c << 16) | (0x0d << 24);
	writel(0x0a0c2060, pinmux);
	// configure pinmux data 0~11
	pinmux = 0x0 | (0x01 << 8) | (0x02 << 16) | (0x03 << 24);
	writel(0x0a0c2064, pinmux);
	pinmux = 0x4 | (0x05 << 8) | (0x06 << 16) | (0x07 << 24);
	writel(0x0a0c2068, pinmux);
	pinmux = 0x8 | (0x09 << 8) | (0x0A << 16) | (0x0B << 24);
	writel(0x0a0c206C, pinmux);
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
	writel(fpga_pwr_reg, 0x30100039);
#endif
	uartlog("### device poweron\n");

	return 0;
}

int peri_down_3(void *priv)
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

