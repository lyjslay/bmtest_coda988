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

struct sensor_info imx290_info = {
	.dev = 0x1A,
	.alen = 2,
	.dlen = 1,
	.gain =  { .cur = 0, .min = 0, .max = 0xf0 },
	.sgain =  { .cur = 0, .min = 0, .max = 0xf0 },
	.shsl =  { .cur = 0, .min = 0, .max = 0 },
	.shss =  { .cur = 0, .min = 0, .max = 0 },
	.ops = &imx290_ops,
};

struct sensor_mode imx290_mode[] = {
	[IMX290_MODE_720P60] =
	{
		.name = "720p60",
		.bit_depth = 10,
		.bayer = SENSOR_BAYER_GR,
		.active_w = 1280,
		.active_h = 720,
		.total_w = 1308,
		.total_h = 736,
		.fp_w = 12,
		.bp_w = 16,
		.fp_h = 10,
		.bp_h = 6,	// in spec it shall be 5 but csi-bdg report frame err.
	},
	[IMX290_MODE_720P30] =
	{
		.name = "720p30",
		.bit_depth = 10,
		.bayer = SENSOR_BAYER_GR,
		.active_w = 1280,
		.active_h = 720,
		.total_w = 1308,
		.total_h = 736,
		.fp_w = 12,
		.bp_w = 16,
		.fp_h = 10,
		.bp_h = 6,	// in spec it shall be 5 but csi-bdg report frame err.
	},
	[IMX290_MODE_720P15_HDR1] =
	{
		.name = "720p15_hdr1",
		.bit_depth = 10,
		.bayer = SENSOR_BAYER_GR,
		.active_w = 1280,
		.active_h = 720,
		.total_w = 1308,
		.total_h = 736,
		.fp_w = 12,
		.bp_w = 16,
		.fp_h = 10,
		.bp_h = 6,	// in spec it shall be 5 but csi-bdg report frame err.
	},
	[IMX290_MODE_720P15_HDR2] =
	{
		.name = "720p15_hdr2",
		.bit_depth = 10,
		.bayer = SENSOR_BAYER_GR,
		.active_w = 1280,
		.active_h = 720,
		.total_w = 1308,
		.total_h = 735,
		.fp_w = 12,
		.bp_w = 16,
		.fp_h = 19,
		.bp_h = 6,	// in spec it shall be 5 but csi-bdg report frame err.
	},
};

extern int cli_readline(char *prompt, char *cmdbuf);

/********************************************************************
 *   APIs to replace bmtest's standard APIs
 ********************************************************************/

static void imx290_get_gain(void *priv, struct sensor_prop *prop)
{
	struct sensor_info *sinf = (struct sensor_info *)priv;

	*prop = sinf->gain;
	sinf->adpt->read(sinf->adpt, sinf->dev, 0x3014, sinf->alen, (uint8_t*)&(prop->cur), 1);

	return;
}

static void imx290_set_gain(void *priv, uint16_t gain)
{
	struct sensor_info *sinf = (struct sensor_info *)priv;

	sinf->adpt->write(sinf->adpt, sinf->dev, 0x3014, sinf->alen, (uint8_t*)&(gain), 1);
	sinf->gain.cur = gain;

	return;
}

static void imx290_get_sgain(void *priv, struct sensor_prop *prop)
{
	struct sensor_info *sinf = (struct sensor_info *)priv;

	*prop = sinf->sgain;
	sinf->adpt->read(sinf->adpt, sinf->dev, 0x30F2, sinf->alen, (uint8_t*)&(prop->cur), 1);

	return;
}

static void imx290_set_sgain(void *priv, uint16_t gain)
{
	struct sensor_info *sinf = (struct sensor_info *)priv;

	sinf->adpt->write(sinf->adpt, sinf->dev, 0x30F2, sinf->alen, (uint8_t*)&(gain), 1);
	sinf->sgain.cur = gain;

	return;
}

static void imx290_get_shsl(void *priv, struct sensor_prop *prop)
{
	struct sensor_info *sinf = (struct sensor_info *)priv;
	const uint16_t addr[][3] = {{0x3020, 0x3021, 0x3022}, {0x3024, 0x3025, 0x3026}};

	*prop = sinf->shsl;
	sinf->adpt->read(sinf->adpt, sinf->dev, addr[sinf->ishdr][0], sinf->alen, (uint8_t*)&(prop->cur), 1);
	sinf->adpt->read(sinf->adpt, sinf->dev, addr[sinf->ishdr][1], sinf->alen, ((uint8_t*)&(prop->cur)) + 1, 1);
	sinf->adpt->read(sinf->adpt, sinf->dev, addr[sinf->ishdr][2], sinf->alen, ((uint8_t*)&(prop->cur)) + 2, 1);

	return;
}

static void imx290_set_shsl(void *priv, uint32_t shs)
{
	struct sensor_info *sinf = (struct sensor_info *)priv;
	const uint16_t addr[][3] = {{0x3020, 0x3021, 0x3022}, {0x3024, 0x3025, 0x3026}};

	sinf->adpt->write(sinf->adpt, sinf->dev, addr[sinf->ishdr][0], sinf->alen, (uint8_t*)&(shs), 1);
	sinf->adpt->write(sinf->adpt, sinf->dev, addr[sinf->ishdr][1], sinf->alen, ((uint8_t*)&(shs)) + 1, 1);
	sinf->adpt->write(sinf->adpt, sinf->dev, addr[sinf->ishdr][2], sinf->alen, ((uint8_t*)&(shs)) + 2, 1);
	sinf->shsl.cur = shs;

	return;
}

static void imx290_get_shss(void *priv, struct sensor_prop *prop)
{
	struct sensor_info *sinf = (struct sensor_info *)priv;

	*prop = sinf->shss;
	sinf->adpt->read(sinf->adpt, sinf->dev, 0x3020, sinf->alen, (uint8_t*)&(prop->cur), 1);
	sinf->adpt->read(sinf->adpt, sinf->dev, 0x3021, sinf->alen, ((uint8_t*)&(prop->cur)) + 1, 1);
	sinf->adpt->read(sinf->adpt, sinf->dev, 0x3022, sinf->alen, ((uint8_t*)&(prop->cur)) + 2, 1);

	return;
}

static void imx290_set_shss(void *priv, uint32_t shs)
{
	struct sensor_info *sinf = (struct sensor_info *)priv;

	sinf->adpt->write(sinf->adpt, sinf->dev, 0x3020, sinf->alen, (uint8_t*)&(shs), 1);
	sinf->adpt->write(sinf->adpt, sinf->dev, 0x3021, sinf->alen, ((uint8_t*)&(shs)) + 1, 1);
	sinf->adpt->write(sinf->adpt, sinf->dev, 0x3022, sinf->alen, ((uint8_t*)&(shs)) + 2, 1);
	sinf->shss.cur = shs;

	return;
}

struct sensor_ops imx290_ops = {
	.get_gain = imx290_get_gain,
	.set_gain = imx290_set_gain,
	.get_sgain = imx290_get_sgain,
	.set_sgain = imx290_set_sgain,
	.get_shsl = imx290_get_shsl,
	.set_shsl = imx290_set_shsl,
	.get_shss = imx290_get_shss,
	.set_shss = imx290_set_shss,
};

#define IMX290_TEST_PATTERN_REG        0x308C
#define IMX290_TEST_PTN_V_COLORBAR     0x21

struct sensor_setting imx290_init_720p60_4lane[] = {
	{0x3000, 0x01}, // STANDBY
	{SNR_MARKER_DELAY, 3000}, // delay
	{0x3005, 0x00}, // ADBIT 10bit
	{0x3007, 0x10}, // VREVERS, ...
	{0x3009, 0x01}, // Frame rate select 0-30, 1-60
	{0x300A, 0x3C}, // BLKLEVEL
	{0x300C, 0x00}, // WDMODE [0] 0:Normal, 1:DOL, WDSEL [5:4] 1:DOL 2 frames
	{0x300F, 0x00},
	{0x3010, 0x21},
	{0x3012, 0x64},
	{0x3014, 0x16}, // GAIN, 0x2A=>12.6dB TBD
	{0x3016, 0x09},
	{0x3018, 0xEE}, // VMAX[7:0]
	{0x3019, 0x02}, // VMAX[15:8]
	{0x301A, 0x00}, // VMAX[17:16]:=:0x301A[1:0]
	{0x301C, 0xE4}, // HMAX[7:0], TBD
	{0x301D, 0x1C}, // HMAX[15:8]
	{0x3020, 0x8C}, // SHS[7:0], TBD
	{0x3021, 0x01}, // SHS[15:8]
	{0x3022, 0x00}, // SHS[19:16]
	{0x3024, 0x00}, // SHS2[7:0], TBD
	{0x3025, 0x00}, // SHS2[15:8]
	{0x3026, 0x00}, // SHS2[19:16]
	{0x3030, 0x00}, // RHS1[7:0], TBD
	{0x3031, 0x00}, // RHS1[15:8]
	{0x3032, 0x00}, // RHS1[19:16]
	{0x3045, 0x00}, // DOLSCDEN [0] 1: pattern1 0: pattern2
	// DOLSYDINFOEN[1] 1: embed the id code into 4th of sync
	// code. 0: disable
	// HINFOEN [2] 1: insert id code after 4th sync code 0: disable
	//{0x3020, 0x30}, // SHS[7:0], TBD
	//{0x3021, 0x00}, // SHS[15:8]
	//{0x3022, 0x00}, // SHS[19:16]
	{0x3046, 0xE0}, // Lane number D-2, E-4, F-8
	{0x304B, 0x0A},
	{0x305C, 0x20}, // INCKSEL1
	{0x305D, 0x00}, // INCKSEL2
	{0x305E, 0x20}, // INCKSEL3
	{0x305F, 0x01}, // INCKSEL4
	{0x3070, 0x02},
	{0x3071, 0x11},
	/*new module check*/{0x309B, 0x10},
	/*new module check*/{0x309C, 0x22},
	/*new module check*/{0x30A2, 0x02},
	{0x30A6, 0x20},
	{0x30A8, 0x20},
	{0x30AA, 0x20},
	{0x30AC, 0x20},
	/*new module check*/{0x30B0, 0x43},
	{0x3106, 0x00}, //DOLHBFIXEN[7] 0: pattern1 1: pattern2
	/*new module check {0x310B, 0x00}*/
	{0x3119, 0x9E},
	/*new module check*/{0x311C, 0x1E},
	{0x311E, 0x08},
	{0x3128, 0x05},
	/*new module check*/{0x3129, 0x1D},
	/*new module check {0x3134, 0x0F}*/
	/*new module check {0x313B, 0x50}*/
	{0x313D, 0x83},
	/*new module check*/{0x3150, 0x03},
	{0x315E, 0x1A},
	{0x3164, 0x1A},
	/*new module check {0x317C, 0x00}*/{0x317C, 0x12},
	/*new module check*/{0x317E, 0x00},
	/*new module check {0x317F, 0x00}*/
	/*new module check*/{0x31EC, 0x37},
	{0x32B8, 0x50},
	{0x32B9, 0x10},
	{0x32BA, 0x00},
	{0x32BB, 0x04},
	{0x32C8, 0x50},
	{0x32C9, 0x10},
	{0x32CA, 0x00},
	{0x32CB, 0x04},
	{0x332C, 0xD3},
	{0x332D, 0x10},
	{0x332E, 0x0D},
	{0x3358, 0x06},
	{0x3359, 0xE1},
	{0x335A, 0x11},
	{0x3360, 0x1E},
	{0x3361, 0x61},
	{0x3362, 0x10},

	/* pattern gen */
	//{IMX290_TEST_PATTERN_REG, 0x21},
	{IMX290_TEST_PATTERN_REG, 0x00},

	/*new module check {0x33B0, 0x08}*/{0x33B0, 0x50},
	/*new module check {0x33B1, 0x30}*/
	/*new module check*/{0x33B2, 0x1A},
	{0x33B3, 0x04},
	/*new module check*/{0x3480, 0x49}, // INCKSEL7
	{0x3000, 0x00}, // STANDBY
	{SNR_MARKER_DELAY, 5000}, // delay
	{0x3002, 0x00}, // XMSTA
};

struct sensor_setting imx290_init_720p30_2lane[] = {
	{0x3000, 0x01}, // STANDBY
	{SNR_MARKER_DELAY, 3000}, // delay
	{0x3005, 0x00}, // ADBIT 10bit
	{0x3007, 0x10}, // VREVERS, ...
	{0x3009, 0x02}, //
	{0x300A, 0x3C}, // BLKLEVEL
	{0x300C, 0x00}, // WDMODE [0] 0:Normal, 1:DOL, WDSEL [5:4] 1:DOL 2 frames
	{0x300F, 0x00},
	{0x3010, 0x21},
	{0x3012, 0x64},
	{0x3014, 0x16}, // GAIN, 0x2A=>12.6dB TBD
	{0x3016, 0x09},
	{0x3018, 0xEE}, // VMAX[7:0]
	{0x3019, 0x02}, // VMAX[15:8]
	{0x301A, 0x00}, // VMAX[17:16]:=:0x301A[1:0]
	{0x301C, 0xC8}, // HMAX[7:0], TBD
	{0x301D, 0x19}, // HMAX[15:8]
	{0x3020, 0x8C}, // SHS[7:0], TBD
	{0x3021, 0x01}, // SHS[15:8]
	{0x3022, 0x00}, // SHS[19:16]
	{0x3024, 0x00}, // SHS2[7:0], TBD
	{0x3025, 0x00}, // SHS2[15:8]
	{0x3026, 0x00}, // SHS2[19:16]
	{0x3030, 0x00}, // RHS1[7:0], TBD
	{0x3031, 0x00}, // RHS1[15:8]
	{0x3032, 0x00}, // RHS1[19:16]
	{0x3045, 0x00}, // DOLSCDEN [0] 1: pattern1 0: pattern2
	// DOLSYDINFOEN[1] 1: embed the id code into 4th of sync
	// code. 0: disable
	// HINFOEN [2] 1: insert id code after 4th sync code 0: disable
	{0x3046, 0xD0}, // Lane number D-2, E-4, F-8
	{0x304B, 0x0A},
	{0x305C, 0x20}, // INCKSEL1
	{0x305D, 0x00}, // INCKSEL2
	{0x305E, 0x20}, // INCKSEL3
	{0x305F, 0x01}, // INCKSEL4
	{0x3070, 0x02},
	{0x3071, 0x11},
	/*new module check*/{0x309B, 0x10},
	/*new module check*/{0x309C, 0x22},
	/*new module check*/{0x30A2, 0x02},
	{0x30A6, 0x20},
	{0x30A8, 0x20},
	{0x30AA, 0x20},
	{0x30AC, 0x20},
	/*new module check*/{0x30B0, 0x43},
	{0x3106, 0x00}, //DOLHBFIXEN[7] 0: pattern1 1: pattern2
	/*new module check {0x310B, 0x00}*/
	{0x3119, 0x9E},
	/*new module check*/{0x311C, 0x1E},
	{0x311E, 0x08},
	{0x3128, 0x05},
	/*new module check*/{0x3129, 0x1D},
	/*new module check {0x3134, 0x0F}*/
	/*new module check {0x313B, 0x50}*/
	{0x313D, 0x83},
	/*new module check*/{0x3150, 0x03},
	{0x315E, 0x1A},
	{0x3164, 0x1A},
	/*new module check {0x317C, 0x00}*/{0x317C, 0x12},
	/*new module check*/{0x317E, 0x00},
	/*new module check {0x317F, 0x00}*/
	/*new module check*/{0x31EC, 0x37},
	{0x32B8, 0x50},
	{0x32B9, 0x10},
	{0x32BA, 0x00},
	{0x32BB, 0x04},
	{0x32C8, 0x50},
	{0x32C9, 0x10},
	{0x32CA, 0x00},
	{0x32CB, 0x04},
	{0x332C, 0xD3},
	{0x332D, 0x10},
	{0x332E, 0x0D},
	{0x3358, 0x06},
	{0x3359, 0xE1},
	{0x335A, 0x11},
	{0x3360, 0x1E},
	{0x3361, 0x61},
	{0x3362, 0x10},

	/* pattern gen */
	//{IMX290_TEST_PATTERN_REG, 0x21},
	{IMX290_TEST_PATTERN_REG, 0x00},

	/*new module check {0x33B0, 0x08}*/{0x33B0, 0x50},
	/*new module check {0x33B1, 0x30}*/
	/*new module check*/{0x33B2, 0x1A},
	{0x33B3, 0x04},
	/*new module check*/{0x3480, 0x49}, // INCKSEL7
	{0x3000, 0x00}, // STANDBY
	{SNR_MARKER_DELAY, 5000}, // delay
	{0x3002, 0x00}, // XMSTA
};

struct sensor_setting imx290_init_720p30_4lane[] = {
	{0x3000, 0x01}, // STANDBY
	{SNR_MARKER_DELAY, 3000}, // delay
	{0x3005, 0x00}, // ADBIT 10bit
	{0x3007, 0x10}, // VREVERS, ...
	{0x3009, 0x02}, //
	{0x300A, 0x3C}, // BLKLEVEL
	{0x300C, 0x00}, // WDMODE [0] 0:Normal, 1:DOL, WDSEL [5:4] 1:DOL 2 frames
	{0x300F, 0x00},
	{0x3010, 0x21},
	{0x3012, 0x64},
	{0x3014, 0x16}, // GAIN, 0x2A=>12.6dB TBD
	{0x3016, 0x09},
	{0x3018, 0xEE}, // VMAX[7:0]
	{0x3019, 0x02}, // VMAX[15:8]
	{0x301A, 0x00}, // VMAX[17:16]:=:0x301A[1:0]
	{0x301C, 0xC8}, // HMAX[7:0], TBD
	{0x301D, 0x19}, // HMAX[15:8]
	{0x3020, 0x8C}, // SHS[7:0], TBD
	{0x3021, 0x01}, // SHS[15:8]
	{0x3022, 0x00}, // SHS[19:16]
	{0x3024, 0x00}, // SHS2[7:0], TBD
	{0x3025, 0x00}, // SHS2[15:8]
	{0x3026, 0x00}, // SHS2[19:16]
	{0x3030, 0x00}, // RHS1[7:0], TBD
	{0x3031, 0x00}, // RHS1[15:8]
	{0x3032, 0x00}, // RHS1[19:16]
	{0x3045, 0x00}, // DOLSCDEN [0] 1: pattern1 0: pattern2
	// DOLSYDINFOEN[1] 1: embed the id code into 4th of sync
	// code. 0: disable
	// HINFOEN [2] 1: insert id code after 4th sync code 0: disable
	{0x3046, 0xE0}, // Lane number D-2, E-4, F-8
	{0x304B, 0x0A},
	{0x305C, 0x20}, // INCKSEL1
	{0x305D, 0x00}, // INCKSEL2
	{0x305E, 0x20}, // INCKSEL3
	{0x305F, 0x01}, // INCKSEL4
	{0x3070, 0x02},
	{0x3071, 0x11},
	/*new module check*/{0x309B, 0x10},
	/*new module check*/{0x309C, 0x22},
	/*new module check*/{0x30A2, 0x02},
	{0x30A6, 0x20},
	{0x30A8, 0x20},
	{0x30AA, 0x20},
	{0x30AC, 0x20},
	/*new module check*/{0x30B0, 0x43},
	{0x3106, 0x00}, //DOLHBFIXEN[7] 0: pattern1 1: pattern2
	/*new module check {0x310B, 0x00}*/
	{0x3119, 0x9E},
	/*new module check*/{0x311C, 0x1E},
	{0x311E, 0x08},
	{0x3128, 0x05},
	/*new module check*/{0x3129, 0x1D},
	/*new module check {0x3134, 0x0F}*/
	/*new module check {0x313B, 0x50}*/
	{0x313D, 0x83},
	/*new module check*/{0x3150, 0x03},
	{0x315E, 0x1A},
	{0x3164, 0x1A},
	/*new module check {0x317C, 0x00}*/{0x317C, 0x12},
	/*new module check*/{0x317E, 0x00},
	/*new module check {0x317F, 0x00}*/
	/*new module check*/{0x31EC, 0x37},
	{0x32B8, 0x50},
	{0x32B9, 0x10},
	{0x32BA, 0x00},
	{0x32BB, 0x04},
	{0x32C8, 0x50},
	{0x32C9, 0x10},
	{0x32CA, 0x00},
	{0x32CB, 0x04},
	{0x332C, 0xD3},
	{0x332D, 0x10},
	{0x332E, 0x0D},
	{0x3358, 0x06},
	{0x3359, 0xE1},
	{0x335A, 0x11},
	{0x3360, 0x1E},
	{0x3361, 0x61},
	{0x3362, 0x10},

	/* pattern gen */
	//{IMX290_TEST_PATTERN_REG, 0x21},
	{IMX290_TEST_PATTERN_REG, 0x00},

	/*new module check {0x33B0, 0x08}*/{0x33B0, 0x50},
	/*new module check {0x33B1, 0x30}*/
	/*new module check*/{0x33B2, 0x1A},
	{0x33B3, 0x04},
	/*new module check*/{0x3480, 0x49}, // INCKSEL7
	{0x3000, 0x00}, // STANDBY
	{SNR_MARKER_DELAY, 5000}, // delay
	{0x3002, 0x00}, // XMSTA
};

#define IMX290_HDR1_VMAX 0x30C
#define IMX290_HDR1_RHS 0x45
struct sensor_setting imx290_init_720p30_2lane_hdr1[] = {
	{0x3000, 0x01}, // STANDBY
	{SNR_MARKER_DELAY, 3000}, // delay
	{0x3005, 0x00}, // ADBIT 10bit
	{0x3007, 0x10}, // VREVERS, ...
	{0x3009, 0x02}, //
	{0x300A, 0x3C}, // BLKLEVEL
	{0x300C, 0x11}, // WDMODE [0] 0:Normal, 1:DOL, WDSEL [5:4] 1:DOL 2 frames
	{0x300F, 0x00},
	{0x3010, 0x61}, // 0x21:Normal, 0x61 Each frame gain adjustment
	{0x3012, 0x64},
	{0x3014, 0x16}, // GAIN, 0x2A=>12.6dB (LEF)
	{0x3016, 0x09},
	{0x3018, IMX290_HDR1_VMAX & 0xff}, // VMAX[7:0]
	{0x3019, (IMX290_HDR1_VMAX >> 8) & 0xff}, // VMAX[15:8]
	{0x301A, (IMX290_HDR1_VMAX >> 16) & 0xff}, // VMAX[17:16]:=:0x301A[1:0]
	{0x301C, 0xC8}, // HMAX[7:0], TBD
	{0x301D, 0x19}, // HMAX[15:8]
	{0x3020, 0x02}, // SHS1[7:0], TBD
	{0x3021, 0x00}, // SHS1[15:8]
	{0x3022, 0x00}, // SHS1[19:16]
	{0x3024, 0xDB}, // SHS2[7:0], TBD
	{0x3025, 0x02}, // SHS2[15:8]
	{0x3026, 0x00}, // SHS2[19:16]
	{0x3030, IMX290_HDR1_RHS & 0xff}, // RHS1[7:0], TBD
	{0x3031, (IMX290_HDR1_RHS >> 8) & 0xff}, // RHS1[15:8]
	{0x3032, (IMX290_HDR1_RHS >> 16) & 0xff}, // RHS1[19:16]
	{0x3045, 0x03}, // DOLSCDEN [0] 1: pattern1 0: pattern2
	// DOLSYDINFOEN [1] 1: embed the id code into 4th of sync
	// code. 0: disable
	// HINFOEN [2] 1: insert id code after 4th sync code 0: disable
	{0x3046, 0xD0}, // Lane number D-2, E-4, F-8
	{0x304B, 0x0A},
	{0x305C, 0x20}, // INCKSEL1
	{0x305D, 0x00}, // INCKSEL2
	{0x305E, 0x20}, // INCKSEL3
	{0x305F, 0x01}, // INCKSEL4
	{0x3070, 0x02},
	{0x3071, 0x11},
	/*new module check*/{0x309B, 0x10},
	/*new module check*/{0x309C, 0x22},
	/*new module check*/{0x30A2, 0x02},
	{0x30A6, 0x20},
	{0x30A8, 0x20},
	{0x30AA, 0x20},
	{0x30AC, 0x20},
	/*new module check*/{0x30B0, 0x43},
	{0x30F0, 0x64},
	/*new module check {0x310B, 0x00}*/
	//{0x3106, 0x11}, //DOLHBFIXEN[7] 0: pattern1 1: pattern2
	{0x3106, 0x11}, //DOLHBFIXEN[7] 0: pattern1 1: pattern2
	{0x3119, 0x9E},
	/*new module check*/{0x311C, 0x1E},
	{0x311E, 0x08},
	{0x3128, 0x05},
	/*new module check*/{0x3129, 0x1D},
	/*new module check {0x3134, 0x0F}*/
	/*new module check {0x313B, 0x50}*/
	{0x313D, 0x83},
	/*new module check*/{0x3150, 0x03},
	{0x315E, 0x1A},
	{0x3164, 0x1A},
	/*new module check {0x317C, 0x00}*/{0x317C, 0x12},
	/*new module check*/{0x317E, 0x00},
	/*new module check {0x317F, 0x00}*/
	/*new module check*/{0x31EC, 0x37},
	{0x32B8, 0x50},
	{0x32B9, 0x10},
	{0x32BA, 0x00},
	{0x32BB, 0x04},
	{0x32C8, 0x50},
	{0x32C9, 0x10},
	{0x32CA, 0x00},
	{0x32CB, 0x04},
	{0x332C, 0xD3},
	{0x332D, 0x10},
	{0x332E, 0x0D},
	{0x3358, 0x06},
	{0x3359, 0xE1},
	{0x335A, 0x11},
	{0x3360, 0x1E},
	{0x3361, 0x61},
	{0x3362, 0x10},

	/* pattern gen */
	//{IMX290_TEST_PATTERN_REG, 0x21},
	{IMX290_TEST_PATTERN_REG, 0x00},

	/*new module check {0x33B0, 0x08}*/{0x33B0, 0x50},
	/*new module check {0x33B1, 0x30}*/
	/*new module check*/{0x33B2, 0x1A},
	{0x33B3, 0x04},
	/*new module check*/{0x3480, 0x49}, // INCKSEL7
	{0x3000, 0x00}, // STANDBY
	{SNR_MARKER_DELAY, 5000}, // delay
	{0x3002, 0x00}, // XMSTA
};

struct sensor_setting imx290_init_720p30_2lane_hdr2[] = {
	{0x3000, 0x01}, // STANDBY
	{SNR_MARKER_DELAY, 3000}, // delay
	{0x3005, 0x00}, // ADBIT 10bit
	{0x3007, 0x10}, // VREVERS, ...
	{0x3009, 0x02}, //
	{0x300A, 0x3C}, // BLKLEVEL
	{0x300C, 0x11}, // WDMODE [0] 0:Normal, 1:DOL, WDSEL [5:4] 1:DOL 2 frames
	{0x300F, 0x00},
	{0x3010, 0x61}, // 0x21:Normal, 0x61 Each frame gain adjustment
	{0x3012, 0x64},
	{0x3014, 0x16}, // GAIN, 0x2A=>12.6dB (LEF)
	{0x3016, 0x09},
	//{0x3018, 0xEE}, // VMAX[7:0]
	//{0x3019, 0x02}, // VMAX[15:8]
	//{0x301A, 0x00}, // VMAX[17:16]:=:0x301A[1:0]
	{0x3018, IMX290_HDR1_VMAX & 0xff}, // VMAX[7:0]
	{0x3019, (IMX290_HDR1_VMAX >> 8) & 0xff}, // VMAX[15:8]
	{0x301A, (IMX290_HDR1_VMAX >> 16) & 0xff}, // VMAX[17:16]:=:0x301A[1:0]
	{0x301C, 0xC8}, // HMAX[7:0], TBD
	{0x301D, 0x19}, // HMAX[15:8]
	{0x3020, 0x02}, // SHS1[7:0], TBD
	{0x3021, 0x00}, // SHS1[15:8]
	{0x3022, 0x00}, // SHS1[19:16]
	{0x3024, 0xDB}, // SHS2[7:0], TBD
	{0x3025, 0x02}, // SHS2[15:8]
	{0x3026, 0x00}, // SHS2[19:16]
	{0x3030, IMX290_HDR1_RHS & 0xff}, // RHS1[7:0], TBD
	{0x3031, (IMX290_HDR1_RHS >> 8) & 0xff}, // RHS1[15:8]
	{0x3032, (IMX290_HDR1_RHS >> 16) & 0xff}, // RHS1[19:16]
	//{0x3030, 0x09}, // RHS1[7:0], TBD
	//{0x3031, 0x00}, // RHS1[15:8]
	//{0x3032, 0x00}, // RHS1[19:16]
	{0x3045, 0x02}, // DOLSCDEN [0] 1: pattern1 0: pattern2
	// DOLSYDINFOEN[1] 1: embed the id code into 4th of sync
	// code. 0: disable
	// HINFOEN [2] 1: insert id code after 4th sync code 0: disable
	{0x3046, 0xD0}, // Lane number D-2, E-4, F-8
	{0x304B, 0x0A},
	{0x305C, 0x20}, // INCKSEL1
	{0x305D, 0x00}, // INCKSEL2
	{0x305E, 0x20}, // INCKSEL3
	{0x305F, 0x01}, // INCKSEL4
	{0x3070, 0x02},
	{0x3071, 0x11},
	/*new module check*/{0x309B, 0x10},
	/*new module check*/{0x309C, 0x22},
	/*new module check*/{0x30A2, 0x02},
	{0x30A6, 0x20},
	{0x30A8, 0x20},
	{0x30AA, 0x20},
	{0x30AC, 0x20},
	/*new module check*/{0x30B0, 0x43},
	{0x30F0, 0x64},
	/*new module check {0x310B, 0x00}*/
	{0x3106, 0x91}, //DOLHBFIXEN[7] 0: pattern1 1: pattern2
	{0x3119, 0x9E},
	/*new module check*/{0x311C, 0x1E},
	{0x311E, 0x08},
	{0x3128, 0x05},
	/*new module check*/{0x3129, 0x1D},
	/*new module check {0x3134, 0x0F}*/
	/*new module check {0x313B, 0x50}*/
	{0x313D, 0x83},
	/*new module check*/{0x3150, 0x03},
	{0x315E, 0x1A},
	{0x3164, 0x1A},
	/*new module check {0x317C, 0x00}*/{0x317C, 0x12},
	/*new module check*/{0x317E, 0x00},
	{0x31A0, 0x34}, // HBLANK1 720P 10bit 0x534
	{0x31A1, 0x05},
	/*new module check {0x317F, 0x00}*/
	/*new module check*/{0x31EC, 0x37},
	{0x32B8, 0x50},
	{0x32B9, 0x10},
	{0x32BA, 0x00},
	{0x32BB, 0x04},
	{0x32C8, 0x50},
	{0x32C9, 0x10},
	{0x32CA, 0x00},
	{0x32CB, 0x04},
	{0x332C, 0xD3},
	{0x332D, 0x10},
	{0x332E, 0x0D},
	{0x3358, 0x06},
	{0x3359, 0xE1},
	{0x335A, 0x11},
	{0x3360, 0x1E},
	{0x3361, 0x61},
	{0x3362, 0x10},

	/* pattern gen */
	//{IMX290_TEST_PATTERN_REG, 0x21},
	{IMX290_TEST_PATTERN_REG, 0x00},

	/*new module check {0x33B0, 0x08}*/{0x33B0, 0x50},
	/*new module check {0x33B1, 0x30}*/
	/*new module check*/{0x33B2, 0x1A},
	{0x33B3, 0x04},
	/*new module check*/{0x3480, 0x49}, // INCKSEL7
	{0x3000, 0x00}, // STANDBY
	{SNR_MARKER_DELAY, 5000}, // delay
	{0x3002, 0x00}, // XMSTA
};

int imx290_poweron(void *priv)
{
	pattern_ctx *ctx = (pattern_ctx *)priv;
	struct sensor_info *sinf = (struct sensor_info *)ctx->ptn->priv;
	struct snr_i2c_adapter *adpt = NULL;
	struct sensor_setting *sensor_set = imx290_init_720p60_4lane;
	int setting_num = ARRAY_SIZE(imx290_init_720p60_4lane);
	struct sensor_setting *pset;
	struct sensor_mode *mode = &imx290_mode[IMX290_MODE_720P60];
	char cmdbuf[8];
	int i, r, test_pn = 0;
	uint32_t value;

#if defined(BOARD_ASIC) && defined(CIF_DPHY_ENABLE)
	// Switch the CLK Select
	value = readl(0x0A0D0004);
	value |= (0x8<<8);
	writel(0x0A0D0004, value);
	// Config the PHYD lane enable
	value = readl(0x0A0D0024);
	value &= ~0xFF0FFFFF;
	writel(0x0A0D0024, value);
	// PHYD lane enable
	value |= 0x0F000000;
	// CLK INV
	value |= (0xF<<20);
	value |= 0x10243;
	writel(0x0A0D0024, value);
	// PHYA lane enable
	value = readl(0x0A0D0000);
	value &= ~(0x1<<12);
	value &= ~(0x1F<<16);
	writel(0x0A0D0000, value);

	// Switch the PHY type to SUBLVDS
	value = readl(0x0A0D00F0);
	value &= 0xFFFFFF00;
	writel(0x0A0D00F0, value);
	value |= 0x37;
	writel(0x0A0D00F0, value);
#endif

	if (sensor_mode) {
		switch (sensor_mode) {
		case 1:
			mode = &imx290_mode[IMX290_MODE_720P30];
			sensor_set = imx290_init_720p30_2lane;
			setting_num = ARRAY_SIZE(imx290_init_720p30_2lane);
			uartlog("use 720p30 2 lanes\n");
			sinf->ishdr = false;
			sinf->shsl.min = 1;
			sinf->shsl.max = 0x2EC;
			break;
		case 2:
			mode = &imx290_mode[IMX290_MODE_720P30];
			sensor_set = imx290_init_720p30_4lane;
			setting_num = ARRAY_SIZE(imx290_init_720p30_4lane);
			uartlog("use 720p30 4 lanes\n");
			sinf->ishdr = false;
			sinf->shsl.min = 1;
			sinf->shsl.max = 0x2EC;
			break;
		case 3:
			mode = &imx290_mode[IMX290_MODE_720P15_HDR1];
			sensor_set = imx290_init_720p30_2lane_hdr1;
			setting_num = ARRAY_SIZE(imx290_init_720p30_2lane_hdr1);
			uartlog("use 720p30 2 lanes HDR Pattern 1\n");
			sinf->ishdr = true;
			sinf->shsl.min = IMX290_HDR1_RHS + 2;
			sinf->shsl.max = IMX290_HDR1_VMAX*2 - 2;
			sinf->shss.min = 2;
			sinf->shss.max = IMX290_HDR1_RHS - 2;
			break;
		case 4:
			mode = &imx290_mode[IMX290_MODE_720P15_HDR2];
			sensor_set = imx290_init_720p30_2lane_hdr2;
			setting_num = ARRAY_SIZE(imx290_init_720p30_2lane_hdr2);
			uartlog("use 720p30 2 lanes HDR Pattern 2\n");
			sinf->ishdr = true;
			sinf->shsl.min = IMX290_HDR1_RHS + 2;
			sinf->shsl.max = IMX290_HDR1_VMAX*2 - 2;
			sinf->shss.min = 2;
			sinf->shss.max = IMX290_HDR1_RHS - 2;
			break;
		default:
			uartlog("use 720p60 4 lanes\n");
			break;
		}
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
		if (pset->addr == IMX290_TEST_PATTERN_REG) {
			if (test_pn)
				pset->val = IMX290_TEST_PTN_V_COLORBAR;
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
	// Switch the PHY type to SUBLVDS
	value = readl(0x0A0D00F0);
	value &= 0xFFFFFF00;
	writel(0x0A0D00F0, value);
	value |= 0x37;
	writel(0x0A0D00F0, value);
#endif
	uartlog("streaming start\n");

	cmdbuf[0] = cmdbuf[1] = '0';
	cli_readline("run sensor to dram flow? [n/y]", cmdbuf);
	if (cmdbuf[0] == 'y') {
		uartlog("dump %d x %d \n", mode->total_w, mode->total_h);
		config_csibdg(0, mode->total_w, mode->total_h, ctx,
			      (sensor_mode == 3 || sensor_mode == 4), 0);
	}

	return 0;
}

int peri_up_0(void *priv)
{
#define CAM0PLL_SYNC_SET		406720388UL
#define CAM0PLL_DIV_SEL			12
#define CAM0PLL_POST_DIV_SEL		1
#define CAM0PLL_ICTRL			0
#define CAM0PLL_SEL_MODE		1
#define CAM0PLL_PRE_DIV_SEL		1
#define CMA0PLL_CLK_DIV			32

#define PWRPIN1V8_FORCE_LOW_BY_GPIO         (0)

	char cmdbuf[8];
#if !defined(FLOW_AFTER_ECO)
	unsigned int *fpga_pwr_reg;
	uint32_t value;
#endif
	uint32_t tmp;

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

	cmdbuf[0] = '0';
	cli_readline("### enter to start power-on seqeuence. s) to skip : ", cmdbuf);
	if (cmdbuf[0] == 's')
		return 0;

	bm_get_num("output 720P60-4L(default-0), 720P30-2L(1), 720P30-4L(2), 720P30-2LHDR1(3), 720P30-2LHDR2(4):", sensor_mode);
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

#if (PWRPIN1V8_FORCE_LOW_BY_GPIO == 1)
	// switch to gpio
	writel(HARD_REG_I2C4_SCL, 0x4030);
	writel(HARD_REG_I2C4_SDA, 0x4030);
	// gpio output enable
	writel(HARD_REG_GPIO1_OE, 0x6);
	writel(HARD_REG_GPIO1_DAT, 0);

	cli_readline("### switched i2c to gpio to pull low", cmdbuf);
#endif

#if !defined(FLOW_AFTER_ECO)
	fpga_pwr_reg = (unsigned int *)HARD_REG_FPGA_PWRPIN;
	// for lane chk
	tmp = (sensor_mode == 1 ||
	       sensor_mode == 3 ||
	       sensor_mode == 4) ? 0x3000 : 0xf000;
	// power sequence
	writel(fpga_pwr_reg, tmp);
	uartlog("power-on delay\n");
	udelay(5000);
	writel(fpga_pwr_reg, tmp | 0x08);
	uartlog("power-on delay\n");
	udelay(5000);
	writel(fpga_pwr_reg, tmp | 0x18);
	uartlog("power-on delay\n");
	udelay(5000);
	writel(fpga_pwr_reg, tmp | 0x38);
	uartlog("power-on delay\n");
	udelay(5000);
#endif
	// Switch the PHY type select
	writel(0x0a0c2000, 0x00000f02);
	if (sensor_mode == 1) { // 2-lane
		// config sub-LVDS front porch
		writel(0x0a0c2200, 0x000A0103);
		writel(0x0a0c2230, 0x00000011);
	} else if (sensor_mode == 3) { // 2-lane HDR pattern 1
		// config sub-LVDS front porch
		writel(0x0a0c2200, 0x000F1103);
		writel(0x0a0c2218, 0x00040760);
		writel(0x0a0c221C, 0x000801D4);
		writel(0x0a0c2220, 0x040401D8);
		writel(0x0a0c2224, 0x040805D4);
		writel(0x0a0c2228, 0x000005D8);
		writel(0x0a0c2230, 0x00000011);
	} else if (sensor_mode == 4) { // 2-lane HDR pattern 2
		// remove info line, only work for single and hdr pattern 2.
		// must change the line count.
		writel(0x0a0c2048, 0x00010001);
		// config sub-LVDS front porch
		writel(0x0a0c2200, 0x00043103);
		writel(0x0a0c2218, 0x00040760);
		writel(0x0a0c221C, 0x000801D4);
		writel(0x0a0c2220, 0x040401D8);
		writel(0x0a0c2224, 0x040805D4);
		writel(0x0a0c2228, 0x000005D8);
		writel(0x0a0c2250, 0x01DC000C);
		writel(0x0a0c2254, 0x05DC040C);
		// [29:16] HBLANK(per lane) [13:0] HSIZE
		writel(0x0a0c2258, 0x029A028E);
		writel(0x0a0c2230, 0x00000011);
	} else {	// 4-lane
		// config sub-LVDS front porch
		writel(0x0a0c2200, 0x0006010f);
		writel(0x0a0c2230, 0x00060013);
	}
#if !defined(FLOW_AFTER_ECO)
	// enable RX PHY and debug
	writel(fpga_pwr_reg, tmp | 0x07100039);
#endif
	uartlog("power-on delay\n");

#if (PWRPIN1V8_FORCE_LOW_BY_GPIO == 1)
	// release output enable
	writel(HARD_REG_GPIO1_OE, 0x0);
	// switch to i2c_4
	writel(HARD_REG_I2C4_SCL, 0x4000);
	writel(HARD_REG_I2C4_SDA, 0x4000);
#endif

	uartlog("### device poweron\n");

	return 0;
}

int peri_down_0(void *priv)
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
		if (sensor_mode == 1 ||
		    sensor_mode == 3 ||
		    sensor_mode == 4) { // 2-lane
			writel(fpga_pwr_reg, 0x3018);
			udelay(5000);
			writel(fpga_pwr_reg, 0x3008);
			udelay(5000);
			writel(fpga_pwr_reg, 0x3000);
			udelay(5000);
		} else {
			writel(fpga_pwr_reg, 0x0018);
			udelay(5000);
			writel(fpga_pwr_reg, 0x0008);
			udelay(5000);
			writel(fpga_pwr_reg, 0x0000);
			udelay(5000);
		}
#endif
	}

	return 0;
}

