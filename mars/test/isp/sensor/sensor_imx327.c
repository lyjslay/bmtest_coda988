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

static uint32_t	use_4L = 0;

struct sensor_info imx327_info = {
	.dev = 0x1A,
	.alen = 2,
	.dlen = 1,
	.gain =  { .cur = 0, .min = 0, .max = 0xf0 },
	.sgain =  { .cur = 0, .min = 0, .max = 0xf0 },
	.shsl =  { .cur = 0, .min = 0, .max = 0 },
	.shss =  { .cur = 0, .min = 0, .max = 0 },
	.ops = &imx327_ops,
};

// sensor mode for imx327
struct sensor_mode imx327_mode[] = {
	[IMX327_MODE_720P30] =
	{
		.name = "720p30",
		.bit_depth = 10,
		.bayer = SENSOR_BAYER_GR,
		.active_w = 1280,
		.active_h = 720,
		.total_w = 1308,
		.total_h = 729,
		.fp_w = 12,
		.bp_w = 16,
		.fp_h = 10,
		.bp_h = 6,	// in spec it shall be 5 but csi-bdg report frame err.
	},
	[IMX327_MODE_720P15_HDR1] =
	{
		.name = "720p15_hdr1",
		.bit_depth = 10,
		.bayer = SENSOR_BAYER_GR,
		.active_w = 1280,
		.active_h = 720,
		.total_w = 1308,
		.total_h = 729,
		.fp_w = 12,
		.bp_w = 16,
		.fp_h = 10,
		.bp_h = 6,	// in spec it shall be 5 but csi-bdg report frame err.
	},
	[IMX327_MODE_1080P30] =
	{
		.name = "1080p30",
		.bit_depth = 12,
		.bayer = SENSOR_BAYER_GR,
		.active_w = 1920,
		.active_h = 1080,
		.total_w = 1948,
		.total_h = 1097,
		.fp_w = 0,
		.bp_w = 0,
		.fp_h = 0,
		.bp_h = 0,	// in spec it shall be 5 but csi-bdg report frame err.
	},
	[IMX327_MODE_1080P30_HDR1] =
	{
		.name = "1080p30_hdr1",
		.bit_depth = 12,
		.bayer = SENSOR_BAYER_GR,
		.active_w = 1920,
		.active_h = 1080,
		.total_w = 1952,
		.total_h = 1114,
		.fp_w = 0,
		.bp_w = 0,
		.fp_h = 0,
		.bp_h = 0,	// in spec it shall be 5 but csi-bdg report frame err.
	},
	[IMX327_MODE_1080P60_2L] =
	{
		.name = "1080p60-2L",
		.bit_depth = 12,
		.bayer = SENSOR_BAYER_GR,
		.active_w = 1920,
		.active_h = 1080,
		.total_w = 1948,
		.total_h = 1097,
		.fp_w = 0,
		.bp_w = 0,
		.fp_h = 0,
		.bp_h = 0,	// in spec it shall be 5 but csi-bdg report frame err.
	},
};

static void imx327_get_gain(void *priv, struct sensor_prop *prop)
{
	struct sensor_info *sinf = (struct sensor_info *)priv;

	*prop = sinf->gain;
	sinf->adpt->read(sinf->adpt, sinf->dev, 0x3014, sinf->alen, (uint8_t*)&(prop->cur), 1);

	return;
}

static void imx327_set_shsl(void *priv, uint32_t shs);

static void imx327_set_gain(void *priv, uint16_t gain)
{
	struct sensor_info *sinf = (struct sensor_info *)priv;
	sinf->adpt->write(sinf->adpt, sinf->dev, 0x3014, sinf->alen, (uint8_t*)&(gain), 1);
	sinf->gain.cur = gain;

	return;
}

static void imx327_get_sgain(void *priv, struct sensor_prop *prop)
{
	struct sensor_info *sinf = (struct sensor_info *)priv;

	*prop = sinf->sgain;
	sinf->adpt->read(sinf->adpt, sinf->dev, 0x30F2, sinf->alen, (uint8_t*)&(prop->cur), 1);

	return;
}

static void imx327_set_sgain(void *priv, uint16_t gain)
{
	struct sensor_info *sinf = (struct sensor_info *)priv;

	sinf->adpt->write(sinf->adpt, sinf->dev, 0x30F2, sinf->alen, (uint8_t*)&(gain), 1);
	sinf->sgain.cur = gain;

	return;
}

static void imx327_get_shsl(void *priv, struct sensor_prop *prop)
{
	struct sensor_info *sinf = (struct sensor_info *)priv;
	const uint16_t addr[][3] = {{0x3020, 0x3021, 0x3022}, {0x3024, 0x3025, 0x3026}};

	*prop = sinf->shsl;
	sinf->adpt->read(sinf->adpt, sinf->dev, addr[sinf->ishdr][0], sinf->alen, (uint8_t*)&(prop->cur), 1);
	sinf->adpt->read(sinf->adpt, sinf->dev, addr[sinf->ishdr][1], sinf->alen, ((uint8_t*)&(prop->cur)) + 1, 1);
	//sinf->adpt->read(sinf->adpt, sinf->dev, addr[sinf->ishdr][2], sinf->alen, ((uint8_t*)&(prop->cur)) + 2, 1);

	return;
}

static void imx327_set_shsl(void *priv, uint32_t shs)
{
	struct sensor_info *sinf = (struct sensor_info *)priv;
	const uint16_t addr[][3] = {{0x3020, 0x3021, 0x3022}, {0x3024, 0x3025, 0x3026}};

	sinf->adpt->write(sinf->adpt, sinf->dev, addr[sinf->ishdr][0], sinf->alen, (uint8_t*)&(shs), 1);
	sinf->adpt->write(sinf->adpt, sinf->dev, addr[sinf->ishdr][1], sinf->alen, ((uint8_t*)&(shs)) + 1, 1);
	//sinf->adpt->write(sinf->adpt, sinf->dev, addr[sinf->ishdr][2], sinf->alen, ((uint8_t*)&(shs)) + 2, 1);
	sinf->shsl.cur = shs;

	return;
}

static void imx327_get_shss(void *priv, struct sensor_prop *prop)
{
	struct sensor_info *sinf = (struct sensor_info *)priv;

	*prop = sinf->shss;
	sinf->adpt->read(sinf->adpt, sinf->dev, 0x3020, sinf->alen, (uint8_t*)&(prop->cur), 1);
	sinf->adpt->read(sinf->adpt, sinf->dev, 0x3021, sinf->alen, ((uint8_t*)&(prop->cur)) + 1, 1);
	sinf->adpt->read(sinf->adpt, sinf->dev, 0x3022, sinf->alen, ((uint8_t*)&(prop->cur)) + 2, 1);

	return;
}

static void imx327_set_shss(void *priv, uint32_t shs)
{
	struct sensor_info *sinf = (struct sensor_info *)priv;

	sinf->adpt->write(sinf->adpt, sinf->dev, 0x3020, sinf->alen, (uint8_t*)&(shs), 1);
	sinf->adpt->write(sinf->adpt, sinf->dev, 0x3021, sinf->alen, ((uint8_t*)&(shs)) + 1, 1);
	sinf->adpt->write(sinf->adpt, sinf->dev, 0x3022, sinf->alen, ((uint8_t*)&(shs)) + 2, 1);
	sinf->shss.cur = shs;

	return;
}

static void imx327_test_mode(void *priv, uint32_t value)
{
	struct sensor_info *sinf = (struct sensor_info *)priv;
	//uint8_t shs1;
	uint8_t tmp;
	uint8_t hcg;
	uint16_t gain;
	static uint8_t test_mode = 0;

	if (value == 0) {
		udelay(35);
		//udelay(800);
		tmp = 1;
		sinf->adpt->write(sinf->adpt, sinf->dev, 0x3001, sinf->alen, (uint8_t*)&(tmp), 1);
		gain = 0x16;
		imx327_set_gain(priv, gain);
		//udelay(400);
		hcg = 0x2;
		sinf->adpt->write(sinf->adpt, sinf->dev, 0x3009, sinf->alen, (uint8_t*)&(hcg), 1);
		tmp = 0;
		sinf->adpt->write(sinf->adpt, sinf->dev, 0x3001, sinf->alen, (uint8_t*)&(tmp), 1);
#if 0
		if (test_mode) {
			shs1 = 0x5a;
			gain = 0x21;
		} else {
			shs1 = 0x442;
			gain = 0x0;
		}
		tmp = 1;
		sinf->adpt->write(sinf->adpt, sinf->dev, 0x3001, sinf->alen, (uint8_t*)&(tmp), 1);
		if (1)
			imx327_set_shsl(priv, shs1);
		imx327_set_gain(priv, gain);
		tmp = 0;
		sinf->adpt->write(sinf->adpt, sinf->dev, 0x3001, sinf->alen, (uint8_t*)&(tmp), 1);
#endif
	} else if (value == 1) {
		if (test_mode)
			hcg = 0x12;
		else
			hcg = 0x02;
		test_mode ^= 1;
		tmp = 1;
		if (0)
			sinf->adpt->write(sinf->adpt, sinf->dev, 0x3001, sinf->alen, (uint8_t*)&(tmp), 1);
		sinf->adpt->write(sinf->adpt, sinf->dev, 0x3009, sinf->alen, (uint8_t*)&(hcg), 1);
		tmp = 0;
		if (0)
			sinf->adpt->write(sinf->adpt, sinf->dev, 0x3001, sinf->alen, (uint8_t*)&(tmp), 1);
	}
	return;
}

struct sensor_ops imx327_ops = {
	.get_gain = imx327_get_gain,
	.set_gain = imx327_set_gain,
	.get_sgain = imx327_get_sgain,
	.set_sgain = imx327_set_sgain,
	.get_shsl = imx327_get_shsl,
	.set_shsl = imx327_set_shsl,
	.get_shss = imx327_get_shss,
	.set_shss = imx327_set_shss,
	.test_mode = imx327_test_mode,
};

int imx327_intr(void *priv)
{
	uint32_t value;

	value = readl(0x0a0c2460);
	//uartlog("csi status intr 0x%x\n", value);
	// clear the interrupt status
	value = readl(0x0a0c2404) | ((value&0xFF)<<8);
	writel(0x0a0c2404, value);

	return 0;
}

#define IMX327_TEST_PATTERN_REG        0x308C
#define IMX327_TEST_PTN_V_COLORBAR     0x21
struct sensor_setting imx327_init_720p30_2lane[] = {
	{0x3003, 0x01}, // SW Reset
	{SNR_MARKER_DELAY, 3000}, // delay
	{0x3000, 0x01}, // STANDBY
	{0x3001, 0x00}, // HOLD
	{0x3002, 0x00}, // XMSTA
	{0x3005, 0x00}, // ADBIT 10bit
	{0x3007, 0x10}, // VREVERS, ...
	{0x3009, 0x02}, //
	{0x300A, 0x3C}, // BLKLEVEL
	//{0x300C, 0x00}, // WDMODE [0] 0:Normal, 1:DOL, WDSEL [5:4] 1:DOL 2 frames
	{0x3011, 0x02},
	{0x3014, 0x16}, // GAIN, 0x2A=>12.6dB TBD
	//{0x3016, 0x08}, // [TODO] sony's secrect register?
	{0x3018, 0xEE}, // VMAX[7:0]
	{0x3019, 0x02}, // VMAX[15:8]
	{0x301A, 0x00}, // VMAX[17:16]:=:0x301A[1:0]
	{0x301C, 0xC8}, // HMAX[7:0], TBD
	{0x301D, 0x19}, // HMAX[15:8]
	{0x3020, 0x8C}, // SHS[7:0], TBD
	{0x3021, 0x01}, // SHS[15:8]
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
	{0x3046, 0x00},
	{0x304B, 0x0A},
	{0x305C, 0x20}, // INCKSEL1
	{0x305D, 0x00}, // INCKSEL2
	{0x305E, 0x20}, // INCKSEL3
	{0x305F, 0x01}, // INCKSEL4
	{0x309E, 0x4A},
	{0x309F, 0x4A},
	{0x30D2, 0x19},
	{0x30D7, 0x03},
	//{0x3106, 0x00}, //DOLHBFIXEN[7] 0: pattern1 1: pattern2
	{0x3129, 0x1D},
	{0x313B, 0x61},
	{0x315E, 0x1A},
	{0x3164, 0x1A},
	{0x317C, 0x12},
	{0x31EC, 0x37},
	{0x3405, 0x10}, // Repetition
	{0x3407, 0x01}, // physical_lane_nl
	{0x3414, 0x04}, // opb_size_v
	{0x3418, 0xD9}, // y_out_size
	{0x3419, 0x02}, // y_out_size
	{0x3441, 0x0A}, // csi_dt_fmt
	{0x3442, 0x0A}, // csi_dt_fmt
	{0x3443, 0x01}, // csi_lane_mode
	{0x3444, 0x20}, // extck_freq
	{0x3445, 0x25}, // extck_freq
	{0x3446, 0x4F}, // tclkpost
	{0x3447, 0x00}, // tclkpost
	{0x3448, 0x80}, // thszero
	{0x3449, 0x00}, // thszero
	{0x344A, 0x17}, // thsprepare
	{0x344B, 0x00}, // thsprepare
	{0x344C, 0x17}, // tclktrail
	{0x344D, 0x00}, // tclktrail
	{0x344E, 0x80}, // thstrail
	{0x344F, 0x00}, // thstrail
	{0x3450, 0x57}, // tclkzero
	{0x3451, 0x00}, // tclkzero
	{0x3452, 0x17}, // tclkprepare
	{0x3453, 0x00}, // tckkprepare
	{0x3454, 0x17}, // tlpx
	{0x3455, 0x00}, // tlpx
	{0x3472, 0x1C}, // x_out_size
	{0x3473, 0x05}, // x_out_size
	{0x3480, 0x49}, // incksel7
	/* pattern gen */
	//{IMX290_TEST_PATTERN_REG, 0x21},
	//{IMX290_TEST_PATTERN_REG, 0x00},
	{SNR_MARKER_DELAY, 20000}, // delay
	{0x3000, 0x00}, // STANDBY
};
#define IMX327_HDR1_VMAX 0x2EE
#define IMX327_HDR1_RHS 0x09

struct sensor_setting imx327_init_720p30_2lane_hdr1[] = {
	{0x3003, 0x01}, // SW Reset
	{SNR_MARKER_DELAY, 3000}, // delay
	{0x3000, 0x01}, // STANDBY
	{0x3001, 0x00}, // HOLD
	{0x3002, 0x00}, // XMSTA
	{0x3005, 0x00}, // ADBIT
	{0x3007, 0x10}, // VREVERS, ...
	{0x3009, 0x02}, //
	{0x300A, 0x3C}, // BLKLEVEL
	{0x300C, 0x11}, // WDMODE [0] 0:Normal, 1:DOL, WDSEL [5:4] 1:DOL 2 frames
	{0x3011, 0x02},
	{0x3014, 0x16}, // GAIN, 0x2A=>12.6dB TBD
	//{0x3016, 0x08}, // [TODO] sony's secrect register?
	{0x3018, 0xEE}, // VMAX[7:0]
	{0x3019, 0x02}, // VMAX[15:8]
	{0x301A, 0x00}, // VMAX[17:16]:=:0x301A[1:0]
	{0x301C, 0xC8}, // HMAX[7:0], TBD
	{0x301D, 0x19}, // HMAX[15:8]
	{0x3020, 0x02}, // SHS[7:0], TBD
	{0x3021, 0x00}, // SHS[15:8]
	{0x3022, 0x00}, // SHS[19:16]
	{0x3024, 0x1B}, // SHS2[7:0], TBD
	{0x3025, 0x05}, // SHS2[15:8]
	{0x3026, 0x00}, // SHS2[19:16]
	{0x3030, 0x09}, // RHS1[7:0], TBD
	{0x3031, 0x00}, // RHS1[15:8]
	{0x3032, 0x00}, // RHS1[19:16]
	{0x3045, 0x05}, // DOLSCDEN [0] 1: pattern1 0: pattern2
	// DOLSYDINFOEN[1] 1: embed the id code into 4th of sync
	// code. 0: disable
	// HINFOEN [2] 1: insert id code after 4th sync code 0: disable
	{0x3046, 0x00},
	{0x304B, 0x0A},
	{0x305C, 0x20}, // INCKSEL1
	{0x305D, 0x00}, // INCKSEL2
	{0x305E, 0x20}, // INCKSEL3
	{0x305F, 0x01}, // INCKSEL4
	{0x309E, 0x4A},
	{0x309F, 0x4A},
	{0x30D2, 0x19},
	{0x30D7, 0x03},
	{0x3106, 0x11}, //DOLHBFIXEN[7] 0: pattern1 1: pattern2
	{0x3129, 0x1D},
	{0x313B, 0x61},
	{0x315E, 0x1A},
	{0x3164, 0x1A},
	{0x317C, 0x12},
	{0x31EC, 0x37},
	{0x3405, 0x10}, // Repetition
	{0x3407, 0x01}, // physical_lane_nl
	{0x3414, 0x04}, // opb_size_v
	{0x3415, 0x00}, // NULL0_SIZE_V, set to 00h when DOL
	{0x3418, 0xC6}, // y_out_size
	{0x3419, 0x05}, // y_out_size
	{0x3441, 0x0A}, // csi_dt_fmt
	{0x3442, 0x0A}, // csi_dt_fmt
	{0x3443, 0x01}, // csi_lane_mode
	{0x3444, 0x20}, // extck_freq
	{0x3445, 0x25}, // extck_freq
	{0x3446, 0x4F}, // tclkpost
	{0x3447, 0x00}, // tclkpost
	{0x3448, 0x80}, // thszero
	{0x3449, 0x00}, // thszero
	{0x344A, 0x17}, // thsprepare
	{0x344B, 0x00}, // thsprepare
	{0x344C, 0x17}, // tclktrail
	{0x344D, 0x00}, // tclktrail
	{0x344E, 0x80}, // thstrail
	{0x344F, 0x00}, // thstrail
	{0x3450, 0x57}, // tclkzero
	{0x3451, 0x00}, // tclkzero
	{0x3452, 0x17}, // tclkprepare
	{0x3453, 0x00}, // tckkprepare
	{0x3454, 0x17}, // tlpx
	{0x3455, 0x00}, // tlpx
	{0x3472, 0x20}, // x_out_size
	{0x3473, 0x05}, // x_out_size
	{0x347B, 0x23}, //
	{0x3480, 0x49}, // incksel7
	{SNR_MARKER_DELAY, 20000}, // delay
	{0x3000, 0x00}, // STANDBY
};

struct sensor_setting imx327_init_1080p30_4lane[] = {
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
	{0x3014, 0x0e}, // GAIN, 0x2A=>12.6dB TBD
	{0x3016, 0x08}, // [TODO] sony's secrect register?
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
	{0x3046, 0x01},
	{0x304B, 0x0A},
	{0x305C, 0x18}, // INCKSEL1
	{0x305D, 0x03}, // INCKSEL2
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
	{0x3405, 0x20}, // Repetition
	{0x3407, 0x03}, // physical_lane_nl
	{0x3414, 0x0A}, // opb_size_v
	{0x3418, 0x49}, // y_out_size
	{0x3419, 0x04}, // y_out_size
	{0x3441, 0x0C}, // csi_dt_fmt
	{0x3442, 0x0C}, // csi_dt_fmt
	{0x3443, 0x03}, // csi_lane_mode
	{0x3444, 0x20}, // extck_freq
	{0x3445, 0x25}, // extck_freq
	{0x3446, 0x47}, // tclkpost
	{0x3447, 0x00}, // tclkpost
	{0x3448, 0x80}, // thszero
	{0x3449, 0x00}, // thszero
	{0x344A, 0x17}, // thsprepare
	{0x344B, 0x00}, // thsprepare
	{0x344C, 0x0F}, // tclktrail
	{0x344D, 0x00}, // tclktrail
	{0x344E, 0x80}, // thstrail
	{0x344F, 0x00}, // thstrail
	{0x3450, 0x47}, // tclkzero
	{0x3451, 0x00}, // tclkzero
	{0x3452, 0x0F}, // tclkprepare
	{0x3453, 0x00}, // tckkprepare
	{0x3454, 0x0F}, // tlpx
	{0x3455, 0x00}, // tlpx
	{0x3472, 0x9C}, // x_out_size
	{0x3473, 0x07}, // x_out_size
	{0x3480, 0x49}, // incksel7
	/* pattern gen */
	//{IMX290_TEST_PATTERN_REG, 0x21},
	//{IMX290_TEST_PATTERN_REG, 0x00},
	{SNR_MARKER_DELAY, 20000}, // delay
	{0x3000, 0x00}, // STANDBY
};

struct sensor_setting imx327_init_1080p30_4lane_hdr1[] = {
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
	//{0x3016, 0x08}, // [TODO] sony's secrect register?
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
	{0x3045, 0x05}, // DOLSCDEN [0] 1: pattern1 0: pattern2
	// DOLSYDINFOEN[1] 1: embed the id code into 4th of sync
	// code. 0: disable
	// HINFOEN [2] 1: insert id code after 4th sync code 0: disable
	{0x3046, 0x01},
	{0x304B, 0x0A},
	{0x305C, 0x18}, // INCKSEL1
	{0x305D, 0x03}, // INCKSEL2
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
	{0x3405, 0x10}, // Repetition
	{0x3407, 0x03}, // physical_lane_nl
	{0x3414, 0x0A}, // opb_size_v
	{0x3415, 0x00}, // NULL0_SIZE_V, set to 00h when DOL
	{0x3418, 0xB4}, // y_out_size
	{0x3419, 0x08}, // y_out_size
	{0x3441, 0x0C}, // csi_dt_fmt
	{0x3442, 0x0C}, // csi_dt_fmt
	{0x3443, 0x03}, // csi_lane_mode
	{0x3444, 0x20}, // extck_freq
	{0x3445, 0x25}, // extck_freq
	{0x3446, 0x57}, // tclkpost
	{0x3447, 0x00}, // tclkpost
	{0x3448, 0x80}, // thszero
	{0x3449, 0x00}, // thszero
	{0x344A, 0x1F}, // thsprepare
	{0x344B, 0x00}, // thsprepare
	{0x344C, 0x1F}, // tclktrail
	{0x344D, 0x00}, // tclktrail
	{0x344E, 0x80}, // thstrail
	{0x344F, 0x00}, // thstrail
	{0x3450, 0x77}, // tclkzero
	{0x3451, 0x00}, // tclkzero
	{0x3452, 0x1F}, // tclkprepare
	{0x3453, 0x00}, // tckkprepare
	{0x3454, 0x17}, // tlpx
	{0x3455, 0x00}, // tlpx
	{0x3472, 0xA0}, // x_out_size
	{0x3473, 0x07}, // x_out_size
	{0x347B, 0x23}, //
	{0x3480, 0x49}, // incksel7
	{SNR_MARKER_DELAY, 20000}, // delay
	{0x3000, 0x00}, // STANDBY
};

struct sensor_setting imx327_init_1080p60_2lane[] = {
	{0x3003, 0x01}, // SW Reset
	{SNR_MARKER_DELAY, 3000}, // delay
	{0x3000, 0x01}, // STANDBY
	{0x3001, 0x00}, // HOLD
	{0x3002, 0x00}, // XMSTA
	{0x3005, 0x01}, // ADBIT 10bit
	{0x3007, 0x00}, // VREVERS, ...
	{0x3009, 0x01}, //
	{0x300A, 0xF0}, // BLKLEVEL
	//{0x300C, 0x00}, // WDMODE [0] 0:Normal, 1:DOL, WDSEL [5:4] 1:DOL 2 frames
	{0x3011, 0x02},
	{0x3014, 0x16}, // GAIN, 0x2A=>12.6dB TBD
	//{0x3016, 0x08}, // [TODO] sony's secrect register?
	{0x3018, 0x65}, // VMAX[7:0]
	{0x3019, 0x04}, // VMAX[15:8]
	{0x301A, 0x00}, // VMAX[17:16]:=:0x301A[1:0]
	{0x301C, 0x98}, // HMAX[7:0], TBD
	{0x301D, 0x08}, // HMAX[15:8]
	{0x3020, 0x8C}, // SHS[7:0], TBD
	{0x3021, 0x01}, // SHS[15:8]
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
	{0x3046, 0x01},
	{0x304B, 0x0A},
	{0x305C, 0x18}, // INCKSEL1
	{0x305D, 0x03}, // INCKSEL2
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
	{0x3405, 0x00}, // Repetition
	{0x3407, 0x01}, // physical_lane_nl
	{0x3414, 0x0A}, // opb_size_v
	{0x3418, 0x49}, // y_out_size
	{0x3419, 0x04}, // y_out_size
	{0x3441, 0x0C}, // csi_dt_fmt
	{0x3442, 0x0C}, // csi_dt_fmt
	{0x3443, 0x01}, // csi_lane_mode
	{0x3444, 0x20}, // extck_freq
	{0x3445, 0x25}, // extck_freq
	{0x3446, 0x77}, // tclkpost
	{0x3447, 0x00}, // tclkpost
	{0x3448, 0x67}, // thszero
	{0x3449, 0x00}, // thszero
	{0x344A, 0x47}, // thsprepare
	{0x344B, 0x00}, // thsprepare
	{0x344C, 0x37}, // tclktrail
	{0x344D, 0x00}, // tclktrail
	{0x344E, 0x3F}, // thstrail
	{0x344F, 0x00}, // thstrail
	{0x3450, 0xFF}, // tclkzero
	{0x3451, 0x00}, // tclkzero
	{0x3452, 0x3F}, // tclkprepare
	{0x3453, 0x00}, // tckkprepare
	{0x3454, 0x37}, // tlpx
	{0x3455, 0x00}, // tlpx
	{0x3472, 0x9C}, // x_out_size
	{0x3473, 0x07}, // x_out_size
	{0x3480, 0x49}, // incksel7
	/* pattern gen */
	//{IMX290_TEST_PATTERN_REG, 0x21},
	//{IMX290_TEST_PATTERN_REG, 0x00},
	{SNR_MARKER_DELAY, 20000}, // delay
	{0x3000, 0x00}, // STANDBY
};

int imx327_poweron(void *priv)
{
	pattern_ctx *ctx = (pattern_ctx *)priv;
	struct sensor_info *sinf = (struct sensor_info *)ctx->ptn->priv;
	struct snr_i2c_adapter *adpt = NULL;
	struct sensor_setting *sensor_set = imx327_init_720p30_2lane;
	int setting_num = ARRAY_SIZE(imx327_init_720p30_2lane);
	struct sensor_setting *pset;
	struct sensor_mode *mode = &imx327_mode[IMX327_MODE_720P30];
	char cmdbuf[8];
	int i, r, test_pn = 0;
	uint32_t value, hdr = 0, tmp, fmt = 0;

	disable_irq(CSIMAC0_INTR_NUM);
	bm_get_num("720P30(0), 1080P30(1), 1080P60-2L(2)", fmt);
	if (fmt == 2)
		hdr = 0;
	else
		bm_get_num("HDR OFF(0), HDR-P1(1):", hdr);


#if defined(BOARD_ASIC) && defined(CIF_DPHY_ENABLE)
	// Switch the PHY type to MIPI-CSI
	// uartlog("start phya reset\n");
	value = readl(0x0A0D00F0);
	value &= ~0x1;
	writel(0x0A0D00F0, value);
	// Switch the CLK Select
	value = readl(0x0A0D00004);
	value |= (0x8<<8);
	writel(0x0A0D00004, value);
	// Config the PHYD lane enable
	value = readl(0x0A0D0024);
	value &= ~0xFF0FFFFF;
	writel(0x0A0D0024, value);
	// PHYD lane enable
	value |= (use_4L ? 0x0F000000 :0x03000000);
	// CLK INV
	value |= (use_4L ? (0xF<<20) : (0x3<<20));
	value |= 0x10243;
	writel(0x0A0D0024, value);
	// PHYA lane enable
	value = readl(0x0A0D0000);
	value &= ~(0x1<<12);
	value &= ~(0x1F<<16);
	writel(0x0A0D0000, value);
#endif

	switch (hdr) {
		case 0:
			// linear
			writel(0x0a0c2040, 0x0);
			if (fmt == 0) {
				mode = &imx327_mode[IMX327_MODE_720P30];
				sensor_set = imx327_init_720p30_2lane;
				setting_num = ARRAY_SIZE(imx327_init_720p30_2lane);
				uartlog("use 720p30 2 lanes\n");
			} else if (fmt == 1) {
				mode = &imx327_mode[IMX327_MODE_1080P30];
				sensor_set = imx327_init_1080p30_4lane;
				setting_num = ARRAY_SIZE(imx327_init_1080p30_4lane);
				uartlog("use 1080p30 4 lanes\n");
			} else if (fmt == 2) {
				mode = &imx327_mode[IMX327_MODE_1080P30];
				sensor_set = imx327_init_1080p60_2lane;
				setting_num = ARRAY_SIZE(imx327_init_1080p60_2lane);
				uartlog("use 1080p60 2 lanes\n");
			}
			sinf->ishdr = false;
			sinf->shsl.min = 1;
			sinf->shsl.max = 0x2EC;
			break;
		case 1:
			if (fmt) {
				mode = &imx327_mode[IMX327_MODE_1080P30_HDR1];
				sensor_set = imx327_init_1080p30_4lane_hdr1;
				setting_num = ARRAY_SIZE(imx327_init_1080p30_4lane_hdr1);
				uartlog("use 1080p30 4 lanes HDR\n");
				sinf->ishdr = true;
				// Config MAC HDR VSIZE and SHIFT(T2)
				// Top line is 8, VSIZE = 729 + 8/2 = 733.
				// Shift = dummy line + 8/2 = 4 + 4 = 8.
				writel(0x0a0c2044, (0x1FFF<<16) | 0);
				// Enable HDR manual mode, hdr output inverse and S-SP HDR mode.
				writel(0x0a0c2040, 0x0111);
				//writel(0x0a0c2048, 0x0);
		
			} else {
				mode = &imx327_mode[IMX327_MODE_720P15_HDR1];
				sensor_set = imx327_init_720p30_2lane_hdr1;
				setting_num = ARRAY_SIZE(imx327_init_720p30_2lane_hdr1);
				uartlog("use 720p30 2 lanes HDR\n");
				sinf->ishdr = true;
				writel(0x0a0c2404, 0x000F0000);
				// Config MAC HDR VSIZE and SHIFT(T2)
				// Top line is 8, VSIZE = 729 + 8/2 = 733.
				// Shift = dummy line + 8/2 = 4 + 4 = 8.
				// writel(0x0a0c2044, (739<<16) | 0);
				// Enable HDR manual mode, hdr output inverse and S-SP HDR mode.
				// writel(0x0a0c2040, 0x0111);
				// writel(0x0a0c2048, 0x0);
			}
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
	adpt->speed = 400000;
	snr_bm_i2c_init(adpt);

	// i2c initial sequence
	for (i = 0; i < setting_num; i++) {
		pset = &(sensor_set[i]);

		if (pset->addr == SNR_MARKER_DELAY) {
			uartlog("sensor wr delay\n");
			udelay(pset->val);
			continue;
		}
		if (pset->addr == IMX327_TEST_PATTERN_REG) {
			if (test_pn)
				pset->val = IMX327_TEST_PTN_V_COLORBAR;
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

#if defined(BOARD_FPGA) && defined(CIF_DPHY_ENABLE)
	// Switch the PHY type to MIPI-CSI
	// uartlog("start phya reset\n");
	value = readl(0x0A0D00F0);
	value &= ~0x1;
	writel(0x0A0D00F0, value);
	// Switch the CLK Select
	value = readl(0x0A0D0004);
	value |= (0x1<<8);
	writel(0x0A0D0004, value);
	// config the auto scan
	writel(0x0A0D0054, 0x05010501);
	// Config the PHYD lane enable
	value = readl(0x0A0D0024);
	value &= ~0xFF0FFFFF;
	writel(0x0A0D0024, value);
	// PHYD lane enable
	value |= (use_4L ? 0x0F000000 :0x03000000);
	// CLK INV
	value |= (use_4L ? (0xF<<20) : (0x3<<20));
	value |= 0x43210;
	writel(0x0A0D0024, value);
	// PHYA lane enable
	value = readl(0x0A0D0000);
	value &= ~(0x1<<12);
	value &= ~(0x1F<<16);
	writel(0x0A0D0000, value);
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

#if defined(BOARD_ASIC)
static void enable_chip_power(void)
{
#define CLK_BY0_CAM0		(1UL<<28)
#define CLK_BY0_CAM1		(1UL<<29)
#define CLK_BY0_SRC_VIP_SYS0	(1UL<<23)
#define CLK_BY0_SRC_VIP_SYS1	(1UL<<24)
#define CLK_BY1_SRC_VIP_SYS2	(1UL<<1)
	unsigned int value;

	writel(0x03002040, 0x00010009);
	writel(0x03002048, 0x00020009);
	writel(0x03002054, 0x00010009);
	writel(0x0300205C, 0x00020009);
	value = readl(0x03002030);
	value &= ~(CLK_BY0_CAM0 | CLK_BY0_CAM1 | CLK_BY0_SRC_VIP_SYS0
			| CLK_BY0_SRC_VIP_SYS1);
	writel(0x03002030, value);

	value = readl(0x03002034);
	value &= ~(CLK_BY1_SRC_VIP_SYS2);
	writel(0x03002034, value);
}
#endif

int peri_up_6(void *priv)
{
#define CAM0PLL_SYNC_SET		406720388UL
#define CAM0PLL_DIV_SEL			12
#define CAM0PLL_POST_DIV_SEL		1
#define CAM0PLL_ICTRL			0
#define CAM0PLL_SEL_MODE		1
#define CAM0PLL_PRE_DIV_SEL		1
#define CMA0PLL_CLK_DIV			32
	char cmdbuf[8];
#if !defined(FLOW_AFTER_ECO)
	unsigned int *fpga_pwr_reg;
#endif
	uint32_t value, tmp;

	cmdbuf[0] = '0';
	cli_readline("### enter to start power-on seqeuence. s) to skip : ", cmdbuf);
	if (cmdbuf[0] == 's')
		return 0;

	bm_get_num("720P30/1080P60-2L(0), 1080P30-4L(1):", use_4L);
#if defined(BOARD_ASIC)
	enable_chip_power();

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
	// Switch the MAC type to MIPI-CSI
	writel(0x0a0c2000, 0x00000071);
	value = readl(0x0a0c2400);
	// config lane number
	value &= ~0x07;
	value |= ((use_4L == 0) ? 1 : 3);
	// config force video format
	// value |= (0x8<<16) | (0x01<<13);
	// Enable SOL
	//value |= (0x1<<12);
	writel(0x0a0c2400, value);
	// config the vs gen mode.
	writel(0x0a0c2470, 0x00);

#if !defined(FLOW_AFTER_ECO)
	fpga_pwr_reg = (unsigned int *)HARD_REG_FPGA_PWRPIN;
	// clear reset and enable phy rx
	writel(fpga_pwr_reg, 0x02100000);
#endif
	uartlog("### device poweron\n");

	return 0;
}

int peri_down_6(void *priv)
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

