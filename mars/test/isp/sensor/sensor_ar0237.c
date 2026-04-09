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

struct sensor_mode ar0237_mode[] = {
	[AR0237_MODE_720P30] =
	{
		.name = "720p30",
		.bit_depth = 10,
		.bayer = SENSOR_BAYER_GB,
		.active_w = 1280,
		.active_h = 720,
		.total_w = 1280,
		.total_h = 720, // 744 for hdr pkt-sp
		.fp_w = 0,
		.bp_w = 0,
		.fp_h = 0,
		.bp_h = 6,	// in spec it shall be 5 but csi-bdg report frame err.
	},
	[AR0237_MODE_720P30_PKT_SP_HDR] =
	{
		.name = "720p30_hdr",
		.bit_depth = 10,
		.bayer = SENSOR_BAYER_GB,
		.active_w = 1280,
		.active_h = 720,
		.total_w = 1280,
		.total_h = 720, // 744 for hdr pkt-sp
		.fp_w = 0,
		.bp_w = 0,
		.fp_h = 0,
		.bp_h = 6,	// in spec it shall be 5 but csi-bdg report frame err.
	},
};

struct sensor_info ar0237_info = {
	.dev = 0x10, //0x30,
	.alen = 2,
	.dlen = 2,
	.gain =  { .cur = 0, .min = 0, .max = 0xf0 },
	.sgain =  { .cur = 0, .min = 0, .max = 0xf0 },
	.shsl =  { .cur = 0, .min = 0, .max = 0 },
	.shss =  { .cur = 0, .min = 0, .max = 0 },
	.ops = &ar0237_ops,
};

static void ar0237_set_shsl(void *priv, uint32_t shs)
{
	struct sensor_info *sinf = (struct sensor_info *)priv;

	shs = ((shs&0xFF)<<8) | ((shs>>8)&0xFF);
	sinf->adpt->write(sinf->adpt, sinf->dev, 0x3012, sinf->alen, (uint8_t*)&(shs), 2);
	sinf->shsl.cur = shs;

	return;
}

static void ar0237_set_shss(void *priv, uint32_t shs)
{
	struct sensor_info *sinf = (struct sensor_info *)priv;

	shs = ((shs&0xFF)<<8) | ((shs>>8)&0xFF);
	sinf->adpt->write(sinf->adpt, sinf->dev, 0x3212, sinf->alen, (uint8_t*)&(shs), 2);
	sinf->shsl.cur = shs;

	return;
}

static void ar0237_set_gain(void *priv, uint16_t gain)
{
	struct sensor_info *sinf = (struct sensor_info *)priv;
	const uint16_t addr[2] = {0x3060, 0x312C};
	uint16_t value = sinf->ishdr?0x0504:0x0004;

	value = ((value&0xFF)<<8) | ((value>>8)&0xFF);
	gain = ((gain&0xFF)<<8) | ((gain>>8)&0xFF);
	sinf->adpt->write(sinf->adpt, sinf->dev, 0x3100, sinf->alen, (uint8_t*)&(value), 2);
	sinf->adpt->write(sinf->adpt, sinf->dev, addr[sinf->ishdr], sinf->alen, (uint8_t*)&(gain), 2);
	sinf->gain.cur = gain;

	return;
}

static void ar0237_set_sgain(void *priv, uint16_t gain)
{
	struct sensor_info *sinf = (struct sensor_info *)priv;
	uint16_t value = 0x0504;

	value = ((value&0xFF)<<8) | ((value>>8)&0xFF);
	gain = ((gain&0xFF)<<8) | ((gain>>8)&0xFF);
	sinf->adpt->write(sinf->adpt, sinf->dev, 0x3100, sinf->alen, (uint8_t*)&(value), 2);
	sinf->adpt->write(sinf->adpt, sinf->dev, 0x3060, sinf->alen, (uint8_t*)&(gain), 2);
	sinf->gain.cur = gain;

	return;
}

struct sensor_ops ar0237_ops = {
	.set_gain = ar0237_set_gain,
	.set_sgain = ar0237_set_sgain,
	.set_shsl = ar0237_set_shsl,
	.set_shss = ar0237_set_shss,
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

#define DATA_SHORT_INVERT(x)           (((x>>8) & 0xFF) | ((x&0xFF)<<8))

#define AR0237_TEST_PATTERN_REG        0x3070
#define AR0237_TEST_PTN_SLOID          0x0001
#define AR0237_TEST_PTN_V_COLORBAR     0x0002
#define AR0237_TEST_PTN_GRAY           0x0003

#define AR0237_HISPI_CONTROL_REG       0x31C6
#define AR0237_HISPI_CTL_PKT_SP_LSB    0x0400
#define AR0237_HISPI_CTL_STM_SP_LSB    0x0404

#define AR0237_HISPI_DATA_PATH_SEL_REG 0x306E
#define AR0237_HISPI_PK_SP_MODE        0x9218

struct sensor_setting ar0237_init_720p_4lane[] = {
	{0x301A, 0x0001}, // RESET_REGISTER
	{SNR_MARKER_DELAY, 200}, /* delay */

//Linear_mode_sequencer
	{0x3088, 0x8000}, // SEQ_CTRL_PORT
	{0x3086, 0x4558}, // SEQ_DATA_PORT
	{0x3086, 0x72A6}, // SEQ_DATA_PORT
	{0x3086, 0x4A31}, // SEQ_DATA_PORT
	{0x3086, 0x4342}, // SEQ_DATA_PORT
	{0x3086, 0x8E03}, // SEQ_DATA_PORT
	{0x3086, 0x2A14}, // SEQ_DATA_PORT
	{0x3086, 0x4578}, // SEQ_DATA_PORT
	{0x3086, 0x7B3D}, // SEQ_DATA_PORT
	{0x3086, 0xFF3D}, // SEQ_DATA_PORT
	{0x3086, 0xFF3D}, // SEQ_DATA_PORT
	{0x3086, 0xEA2A}, // SEQ_DATA_PORT
	{0x3086, 0x043D}, // SEQ_DATA_PORT
	{0x3086, 0x102A}, // SEQ_DATA_PORT
	{0x3086, 0x052A}, // SEQ_DATA_PORT
	{0x3086, 0x1535}, // SEQ_DATA_PORT
	{0x3086, 0x2A05}, // SEQ_DATA_PORT
	{0x3086, 0x3D10}, // SEQ_DATA_PORT
	{0x3086, 0x4558}, // SEQ_DATA_PORT
	{0x3086, 0x2A04}, // SEQ_DATA_PORT
	{0x3086, 0x2A14}, // SEQ_DATA_PORT
	{0x3086, 0x3DFF}, // SEQ_DATA_PORT
	{0x3086, 0x3DFF}, // SEQ_DATA_PORT
	{0x3086, 0x3DEA}, // SEQ_DATA_PORT
	{0x3086, 0x2A04}, // SEQ_DATA_PORT
	{0x3086, 0x622A}, // SEQ_DATA_PORT
	{0x3086, 0x288E}, // SEQ_DATA_PORT
	{0x3086, 0x0036}, // SEQ_DATA_PORT
	{0x3086, 0x2A08}, // SEQ_DATA_PORT
	{0x3086, 0x3D64}, // SEQ_DATA_PORT
	{0x3086, 0x7A3D}, // SEQ_DATA_PORT
	{0x3086, 0x0444}, // SEQ_DATA_PORT
	{0x3086, 0x2C4B}, // SEQ_DATA_PORT
	{0x3086, 0xA403}, // SEQ_DATA_PORT
	{0x3086, 0x430D}, // SEQ_DATA_PORT
	{0x3086, 0x2D46}, // SEQ_DATA_PORT
	{0x3086, 0x4316}, // SEQ_DATA_PORT
	{0x3086, 0x2A90}, // SEQ_DATA_PORT
	{0x3086, 0x3E06}, // SEQ_DATA_PORT
	{0x3086, 0x2A98}, // SEQ_DATA_PORT
	{0x3086, 0x5F16}, // SEQ_DATA_PORT
	{0x3086, 0x530D}, // SEQ_DATA_PORT
	{0x3086, 0x1660}, // SEQ_DATA_PORT
	{0x3086, 0x3E4C}, // SEQ_DATA_PORT
	{0x3086, 0x2904}, // SEQ_DATA_PORT
	{0x3086, 0x2984}, // SEQ_DATA_PORT
	{0x3086, 0x8E03}, // SEQ_DATA_PORT
	{0x3086, 0x2AFC}, // SEQ_DATA_PORT
	{0x3086, 0x5C1D}, // SEQ_DATA_PORT
	{0x3086, 0x5754}, // SEQ_DATA_PORT
	{0x3086, 0x495F}, // SEQ_DATA_PORT
	{0x3086, 0x5305}, // SEQ_DATA_PORT
	{0x3086, 0x5307}, // SEQ_DATA_PORT
	{0x3086, 0x4D2B}, // SEQ_DATA_PORT
	{0x3086, 0xF810}, // SEQ_DATA_PORT
	{0x3086, 0x164C}, // SEQ_DATA_PORT
	{0x3086, 0x0955}, // SEQ_DATA_PORT
	{0x3086, 0x562B}, // SEQ_DATA_PORT
	{0x3086, 0xB82B}, // SEQ_DATA_PORT
	{0x3086, 0x984E}, // SEQ_DATA_PORT
	{0x3086, 0x1129}, // SEQ_DATA_PORT
	{0x3086, 0x9460}, // SEQ_DATA_PORT
	{0x3086, 0x5C19}, // SEQ_DATA_PORT
	{0x3086, 0x5C1B}, // SEQ_DATA_PORT
	{0x3086, 0x4548}, // SEQ_DATA_PORT
	{0x3086, 0x4508}, // SEQ_DATA_PORT
	{0x3086, 0x4588}, // SEQ_DATA_PORT
	{0x3086, 0x29B6}, // SEQ_DATA_PORT
	{0x3086, 0x8E01}, // SEQ_DATA_PORT
	{0x3086, 0x2AF8}, // SEQ_DATA_PORT
	{0x3086, 0x3E02}, // SEQ_DATA_PORT
	{0x3086, 0x2AFA}, // SEQ_DATA_PORT
	{0x3086, 0x3F09}, // SEQ_DATA_PORT
	{0x3086, 0x5C1B}, // SEQ_DATA_PORT
	{0x3086, 0x29B2}, // SEQ_DATA_PORT
	{0x3086, 0x3F0C}, // SEQ_DATA_PORT
	{0x3086, 0x3E03}, // SEQ_DATA_PORT
	{0x3086, 0x3E15}, // SEQ_DATA_PORT
	{0x3086, 0x5C13}, // SEQ_DATA_PORT
	{0x3086, 0x3F11}, // SEQ_DATA_PORT
	{0x3086, 0x3E0F}, // SEQ_DATA_PORT
	{0x3086, 0x5F2B}, // SEQ_DATA_PORT
	{0x3086, 0x902B}, // SEQ_DATA_PORT
	{0x3086, 0x803E}, // SEQ_DATA_PORT
	{0x3086, 0x062A}, // SEQ_DATA_PORT
	{0x3086, 0xF23F}, // SEQ_DATA_PORT
	{0x3086, 0x103E}, // SEQ_DATA_PORT
	{0x3086, 0x0160}, // SEQ_DATA_PORT
	{0x3086, 0x29A2}, // SEQ_DATA_PORT
	{0x3086, 0x29A3}, // SEQ_DATA_PORT
	{0x3086, 0x5F4D}, // SEQ_DATA_PORT
	{0x3086, 0x1C2A}, // SEQ_DATA_PORT
	{0x3086, 0xFA29}, // SEQ_DATA_PORT
	{0x3086, 0x8345}, // SEQ_DATA_PORT
	{0x3086, 0xA83E}, // SEQ_DATA_PORT
	{0x3086, 0x072A}, // SEQ_DATA_PORT
	{0x3086, 0xFB3E}, // SEQ_DATA_PORT
	{0x3086, 0x2945}, // SEQ_DATA_PORT
	{0x3086, 0x8824}, // SEQ_DATA_PORT
	{0x3086, 0x3E08}, // SEQ_DATA_PORT
	{0x3086, 0x2AFA}, // SEQ_DATA_PORT
	{0x3086, 0x5D29}, // SEQ_DATA_PORT
	{0x3086, 0x9288}, // SEQ_DATA_PORT
	{0x3086, 0x102B}, // SEQ_DATA_PORT
	{0x3086, 0x048B}, // SEQ_DATA_PORT
	{0x3086, 0x1686}, // SEQ_DATA_PORT
	{0x3086, 0x8D48}, // SEQ_DATA_PORT
	{0x3086, 0x4D4E}, // SEQ_DATA_PORT
	{0x3086, 0x2B80}, // SEQ_DATA_PORT
	{0x3086, 0x4C0B}, // SEQ_DATA_PORT
	{0x3086, 0x3F36}, // SEQ_DATA_PORT
	{0x3086, 0x2AF2}, // SEQ_DATA_PORT
	{0x3086, 0x3F10}, // SEQ_DATA_PORT
	{0x3086, 0x3E01}, // SEQ_DATA_PORT
	{0x3086, 0x6029}, // SEQ_DATA_PORT
	{0x3086, 0x8229}, // SEQ_DATA_PORT
	{0x3086, 0x8329}, // SEQ_DATA_PORT
	{0x3086, 0x435C}, // SEQ_DATA_PORT
	{0x3086, 0x155F}, // SEQ_DATA_PORT
	{0x3086, 0x4D1C}, // SEQ_DATA_PORT
	{0x3086, 0x2AFA}, // SEQ_DATA_PORT
	{0x3086, 0x4558}, // SEQ_DATA_PORT
	{0x3086, 0x8E00}, // SEQ_DATA_PORT
	{0x3086, 0x2A98}, // SEQ_DATA_PORT
	{0x3086, 0x3F0A}, // SEQ_DATA_PORT
	{0x3086, 0x4A0A}, // SEQ_DATA_PORT
	{0x3086, 0x4316}, // SEQ_DATA_PORT
	{0x3086, 0x0B43}, // SEQ_DATA_PORT
	{0x3086, 0x168E}, // SEQ_DATA_PORT
	{0x3086, 0x032A}, // SEQ_DATA_PORT
	{0x3086, 0x9C45}, // SEQ_DATA_PORT
	{0x3086, 0x783F}, // SEQ_DATA_PORT
	{0x3086, 0x072A}, // SEQ_DATA_PORT
	{0x3086, 0x9D3E}, // SEQ_DATA_PORT
	{0x3086, 0x305D}, // SEQ_DATA_PORT
	{0x3086, 0x2944}, // SEQ_DATA_PORT
	{0x3086, 0x8810}, // SEQ_DATA_PORT
	{0x3086, 0x2B04}, // SEQ_DATA_PORT
	{0x3086, 0x530D}, // SEQ_DATA_PORT
	{0x3086, 0x4558}, // SEQ_DATA_PORT
	{0x3086, 0x3E08}, // SEQ_DATA_PORT
	{0x3086, 0x8E01}, // SEQ_DATA_PORT
	{0x3086, 0x2A98}, // SEQ_DATA_PORT
	{0x3086, 0x8E00}, // SEQ_DATA_PORT
	{0x3086, 0x76A7}, // SEQ_DATA_PORT
	{0x3086, 0x77A7}, // SEQ_DATA_PORT
	{0x3086, 0x4644}, // SEQ_DATA_PORT
	{0x3086, 0x1616}, // SEQ_DATA_PORT
	{0x3086, 0xA57A}, // SEQ_DATA_PORT
	{0x3086, 0x1244}, // SEQ_DATA_PORT
	{0x3086, 0x4B18}, // SEQ_DATA_PORT
	{0x3086, 0x4A04}, // SEQ_DATA_PORT
	{0x3086, 0x4316}, // SEQ_DATA_PORT
	{0x3086, 0x0643}, // SEQ_DATA_PORT
	{0x3086, 0x1605}, // SEQ_DATA_PORT
	{0x3086, 0x4316}, // SEQ_DATA_PORT
	{0x3086, 0x0743}, // SEQ_DATA_PORT
	{0x3086, 0x1658}, // SEQ_DATA_PORT
	{0x3086, 0x4316}, // SEQ_DATA_PORT
	{0x3086, 0x5A43}, // SEQ_DATA_PORT
	{0x3086, 0x1645}, // SEQ_DATA_PORT
	{0x3086, 0x588E}, // SEQ_DATA_PORT
	{0x3086, 0x032A}, // SEQ_DATA_PORT
	{0x3086, 0x9C45}, // SEQ_DATA_PORT
	{0x3086, 0x787B}, // SEQ_DATA_PORT
	{0x3086, 0x3F07}, // SEQ_DATA_PORT
	{0x3086, 0x2A9D}, // SEQ_DATA_PORT
	{0x3086, 0x530D}, // SEQ_DATA_PORT
	{0x3086, 0x8B16}, // SEQ_DATA_PORT
	{0x3086, 0x863E}, // SEQ_DATA_PORT
	{0x3086, 0x2345}, // SEQ_DATA_PORT
	{0x3086, 0x5825}, // SEQ_DATA_PORT
	{0x3086, 0x3E10}, // SEQ_DATA_PORT
	{0x3086, 0x8E01}, // SEQ_DATA_PORT
	{0x3086, 0x2A98}, // SEQ_DATA_PORT
	{0x3086, 0x8E00}, // SEQ_DATA_PORT
	{0x3086, 0x3E10}, // SEQ_DATA_PORT
	{0x3086, 0x8D60}, // SEQ_DATA_PORT
	{0x3086, 0x1244}, // SEQ_DATA_PORT
	{0x3086, 0x4BB9}, // SEQ_DATA_PORT
	{0x3086, 0x2C2C}, // SEQ_DATA_PORT
	{0x3086, 0x2C2C}, // SEQ_DATA_PORT
	{0x3086, 0x2C2C}, // SEQ_DATA_PORT

//PLL_settings
	{0x31AC, 0x0A0A}, // DATA_FORMAT_BITS
	{0x302A, 0x000A}, // VT_PIX_CLK_DIV
	{0x302C, 0x0001}, // VT_SYS_CLK_DIV
	{0x302E, 0x000A}, // PRE_PLL_CLK_DIV
	{0x3030, 0x00A0}, // PLL_MULTIPLIER
	{0x3036, 0x000A}, // OP_PIX_CLK_DIV
	{0x3038, 0x0002}, // OP_SYS_CLK_DIV

//Output resolution
	{0x3002, 0x00B8}, //Y_ADDR_START = 184
	{0x3004, 0x014C}, //X_ADDR_START = 332
	{0x3006, 0x0387}, //Y_ADDR_END = 903
	{0x3008, 0x064B}, //X_ADDR_END = 1611

//Timing
	{0x300A, 0x04BA}, // FRAME_LENGTH_LINES
	{0x300C, 0x0898}, // LINE_LENGTH_PCK ori:4C8
	{0x3012, 0x0200}, // COARSE_INTEGRATION_TIME
	{0x30A2, 0x0001}, // X_ODD_INC
	{0x30A6, 0x0001}, // Y_ODD_INC
	{0x30AE, 0x0001}, // X_ODD_INC_CB
	{0x30A8, 0x0001}, // Y_ODD_INC_CB
	{0x3040, 0x0000}, // READ_MODE
	{0x3082, 0x0009}, // OPERATION_MODE_CTRL
	{0x3100, 0x0000}, // AECTRLREG
	{0x31D0, 0x0000}, // COMPANDING

//Default_Settings_5_Oct
	{0x3064, 0x1802}, // SMIA_TEST
	{0x3EEE, 0xA0AA}, // DAC_LD_34_35
	{0x3F4A, 0x0F70}, // DELTA_DK_PIX_THRES
	{0x309E, 0x016C}, // HIDY_PROG_START_ADDR
	{0x3092, 0x006F}, // ROW_NOISE_CONTROL
	{0x3EE4, 0x9937}, // DAC_LD_24_25
	{0x3EE6, 0x3863}, // DAC_LD_26_27
	{0x3EEC, 0x3B0C}, // DAC_LD_32_33
	{0x30B0, 0x1A3A}, // DIGITAL_TEST
	{0x30BA, 0x762C}, // DIGITAL_CTRL
	{0x3EEA, 0x283C}, // DAC_LD_30_31
	{0x3ECC, 0x4E2D}, // DAC_LD_0_1
	{0x3ED2, 0xFEA6}, // DAC_LD_6_7
	{0x3ED6, 0x2CB3}, // DAC_LD_10_11
	{0x3EEA, 0x2819}, // DAC_LD_30_31

	{0x31AE, 0x0304}, // SERIAL_FORMAT 302: 2 lanes, 304: 4lanes
	{AR0237_HISPI_CONTROL_REG, AR0237_HISPI_CTL_PKT_SP_LSB}, // HISPI_CONTROL_STATUS [1] :0 - LSB, 1 - MSB
	{0x306E, 0x9210}, // DATAPATH_SELECT

	{AR0237_TEST_PATTERN_REG, 0}, // Test Pattern

	{0x301A, 0x005C}, // RESET_REGISTER
	{}
};

struct sensor_setting ar0237_init_720p_hdr_4lane[] = {
	{0x301A, 0x0001}, // RESET_REGISTER
	{SNR_MARKER_DELAY, 200}, /* delay */
//PLL_settings
	{0x31AC, 0x0A0A}, // DATA_FORMAT_BITS
	{0x302A, 0x000A}, // VT_PIX_CLK_DIV
	{0x302C, 0x0001}, // VT_SYS_CLK_DIV
	{0x302E, 0x000A}, // PRE_PLL_CLK_DIV
	{0x3030, 0x00A0}, // PLL_MULTIPLIER
	{0x3036, 0x000A}, // OP_PIX_CLK_DIV
	{0x3038, 0x0002}, // OP_SYS_CLK_DIV

//Output resolution
	{0x3002, 0x00B8}, //Y_ADDR_START = 184
	{0x3004, 0x014C}, //X_ADDR_START = 332
	{0x3006, 0x0387}, //Y_ADDR_END = 903
	{0x3008, 0x064B}, //X_ADDR_END = 1611

//Timing
	{0x300A, 0x04BA}, // FRAME_LENGTH_LINES
	{0x300C, 0x1100}, // LINE_LENGTH_PCK ori:4C8
	//{0x300C, 0x1400}, // LINE_LENGTH_PCK ori:4C8
	{0x3012, 0x0416}, // COARSE_INTEGRATION_TIME
	{0x30A2, 0x0001}, // X_ODD_INC
	{0x30A6, 0x0001}, // Y_ODD_INC
	{0x30AE, 0x0001}, // X_ODD_INC_CB
	{0x30A8, 0x0001}, // Y_ODD_INC_CB
	{0x3040, 0x0000}, // READ_MODE
	{0x3082, 0x0009}, // OPERATION_MODE_CTRL
	{0x3100, 0x0400}, // AECTRLREG
	{0x31D0, 0x0000}, // COMPANDING

//Default_Settings_5_Oct
	{0x3064, 0x1802}, // SMIA_TEST
	{0x3EEE, 0xA0AA}, // DAC_LD_34_35
	{0x3F4A, 0x0F70}, // DELTA_DK_PIX_THRES
	{0x309E, 0x016C}, // HIDY_PROG_START_ADDR
	{0x3092, 0x006F}, // ROW_NOISE_CONTROL
	{0x3EE4, 0x9937}, // DAC_LD_24_25
	{0x3EE6, 0x3863}, // DAC_LD_26_27
	{0x3EEC, 0x3B0C}, // DAC_LD_32_33
	{0x30B0, 0x1A3A}, // DIGITAL_TEST
	{0x30BA, 0x762C}, // DIGITAL_CTRL
	{0x3EEA, 0x283C}, // DAC_LD_30_31
	{0x3ECC, 0x4E2D}, // DAC_LD_0_1
	{0x3ED2, 0xFEA6}, // DAC_LD_6_7
	{0x3ED6, 0x2CB3}, // DAC_LD_10_11
	{0x3EEA, 0x2819}, // DAC_LD_30_31

	{0x31AE, 0x0304}, // SERIAL_FORMAT 302: 2 lanes, 304: 4lanes
	{AR0237_HISPI_CONTROL_REG, AR0237_HISPI_CTL_PKT_SP_LSB}, // HISPI_CONTROL_STATUS [1] :0 - LSB, 1 - MSB
	{0x306E, 0x9210}, // DATAPATH_SELECT

	{0x31AC, 0x100A},
	{0x318E, 0x1000},
	{0x3082, 0x0800}, // [1:0] image mode. [3:2] fixed exp ratio. [11] seperately T1, T2.
	{0x30B0, 0x0A3A},
	{0x3212, 300}, // COARSE_INTEGRATION_TIME2
	{AR0237_TEST_PATTERN_REG, 0}, // Test Pattern

	{0x301A, 0x005C}, // RESET_REGISTER
	{}
};

int ar0237_poweron(void *priv)
{
	pattern_ctx *ctx = (pattern_ctx *)priv;
	struct sensor_info *sinf = (struct sensor_info *)ctx->ptn->priv;
	struct snr_i2c_adapter *adpt = NULL;
	struct sensor_setting *sensor_set = ar0237_init_720p_4lane;
	int setting_num = ARRAY_SIZE(ar0237_init_720p_4lane);
	struct sensor_setting *pset;
	struct sensor_mode *mode = &ar0237_mode[AR0237_MODE_720P30];
	int i, r, test_pn = 0, stm_sp = 0, hdr = 0;
	char cmdbuf[8];
	uint32_t value;

	// i2c config here
	snr_bm_i2c_get_adap((void *)&adpt);
	sinf->adpt = adpt;

	adpt->slaveaddr = sinf->dev;
	snr_bm_i2c_init(adpt);

	cmdbuf[0] = cmdbuf[1] = '0';
	cli_readline("enable test pattern? [n/y]", cmdbuf);
	if (cmdbuf[0] == 'y') {
		test_pn = 1;
	}

	cmdbuf[0] = cmdbuf[1] = '0';
	cli_readline("Enable HDR? [n/y]", cmdbuf);
	if (cmdbuf[0] == 'y') {
		sensor_set = ar0237_init_720p_hdr_4lane;
		setting_num = ARRAY_SIZE(ar0237_init_720p_hdr_4lane);
		hdr = 1;
	} else {
		sensor_set = ar0237_init_720p_4lane;
		setting_num = ARRAY_SIZE(ar0237_init_720p_4lane);
		hdr = 0;
	}
	// convert sensor data
	sensor_data_convert(sensor_set, setting_num);
	cmdbuf[0] = cmdbuf[1] = '0';
	cli_readline("HISPI- [0 Packetized-SP/1 Streaming-SP]: ", cmdbuf);
	if (cmdbuf[0] == '1') {
		stm_sp = 1;
		// [TODO] Config Stream-SP SOV
		writel(0x0a0c2208, 0x09000000);
		writel(0x0a0c2258, (readl(0x0a0c2258) & ~0x3FFF) | ((mode->total_w>>2) & 0x3FFF));
		writel(0x0a0c2260, readl(0x0a0c2260) | 0x02);
		if (hdr) {
			// Config MAC HDR VSIZE and SHIFT(T2)
			writel(0x0a0c2044, (720<<16) | 302);
			// Enable HDR manual mode, hdr output inverse and S-SP HDR mode.
			writel(0x0a0c2040, 0x101);
		} else {
			writel(0x0a0c2040, 0x0);
		}
	} else {
		writel(0x0a0c2260, readl(0x0a0c2260) & ~0x02);
		if (hdr) {
			mode = &ar0237_mode[AR0237_MODE_720P30_PKT_SP_HDR];
			value = readl(0x0a0c2260);
			value |= 0x10;
			writel(0x0a0c2260, value);
		}
	}

	// i2c initial sequence
	for (i = 0; i < setting_num; i++) {
		pset = &(sensor_set[i]);

		if (pset->addr == SNR_MARKER_DELAY) {
			uartlog("sensor wr delay\n");
			udelay(pset->val);
			continue;
		}
		if (pset->addr == AR0237_TEST_PATTERN_REG) {
			if (test_pn)
				pset->val = DATA_SHORT_INVERT(AR0237_TEST_PTN_GRAY);
			else
				pset->val = 0;
		}
		if (pset->addr == AR0237_HISPI_CONTROL_REG) {
			if (stm_sp)
				pset->val = DATA_SHORT_INVERT(AR0237_HISPI_CTL_STM_SP_LSB);
			else
				pset->val = DATA_SHORT_INVERT(AR0237_HISPI_CTL_PKT_SP_LSB);
		}
		if (pset->addr == AR0237_HISPI_DATA_PATH_SEL_REG) {
			if (hdr && !stm_sp)
				pset->val = DATA_SHORT_INVERT(AR0237_HISPI_PK_SP_MODE);
		}
		r = sinf->adpt->write(sinf->adpt, sinf->dev, pset->addr, sinf->alen, (uint8_t*)&(pset->val), 2);
		if (r < 0)
			uartlog("%s i2c_write fail addr[0x%x]\n", ctx->ptn->name, pset->addr);
	}

	uartlog("Read back i2c registers:\n");
	for (i = 0; i < setting_num; i++) {
		uint16_t val = 0;

		pset = &(sensor_set[i]);
		// skip SEQ_DATA_PORT
		if (pset->addr == 0x3086)
			continue;
		r = sinf->adpt->read(sinf->adpt, sinf->dev, pset->addr, sinf->alen, (uint8_t*)&val, 2);

		uartlog("reg[0x%x]=0x%x(0x%x)\n", pset->addr, val, pset->val);
	}

#if defined(CIF_DPHY_ENABLE)
	// Switch the PHY type to HISPI
	value = readl(0x0A0D00F0);
	value &= 0xFFFFFF00;
	writel(0x0A0D00F0, value);
	value |= 0xF7;
	writel(0x0A0D00F0, value);
#endif
	uartlog("streaming start\n");

	cmdbuf[0] = cmdbuf[1] = '0';
	cli_readline("run sensor to dram flow? [n/y]", cmdbuf);
	if (cmdbuf[0] == 'y') {
		uartlog("dump %d x %d \n", mode->total_w, mode->total_h);
		config_csibdg(0, mode->total_w, mode->total_h, ctx, !!hdr, 0);
	}

	// restore sensor data
	sensor_data_convert(sensor_set, setting_num);

	return 0;
}

int peri_up_2(void *priv)
{

	char cmdbuf[8];
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
	writel(fpga_pwr_reg, 0x27100039);
	uartlog("### device poweron\n");
#endif

	// Switch the PHY type select
	writel(0x0a0c2000, 0x00000F02);
	// HISPI mode enable
	writel(0x0a0c2260, 0x00000001);
	// config sub-LVDS 4 lanes, bit reverse, front porch
	writel(0x0a0c2200, 0x0000050f);
	writel(0x0a0c2230, 0x00000013);
	return 0;
}

int peri_down_2(void *priv)
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

