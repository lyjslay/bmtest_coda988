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

typedef enum NC_VIVO_CH_FORMATDEF_E
{
    NC_VIVO_CH_FORMATDEF_UNKNOWN = 0,

    SD_H720_NT = 1,
    SD_H720_PAL,
    AHD_720P_30P,
    AHD_720P_25P,
    AHD_960P_30P,
    AHD_960P_25P,
    AHD_1080P_30P,
    AHD_1080P_25P,
    AHD_1080P_60P,
    AHD_1080P_50P,
    AHD_4M_30P,
    AHD_4M_25P,
    AHD_8M_15P,
    AHD_8M_12_5P,

    NC_VIVO_CH_FORMATDEF_MAX,

} NC_VIVO_CH_FORMATDEF_E;


typedef enum _n5_outmode_sel
{
	N5_OUTMODE_1MUX_SD = 0,
	N5_OUTMODE_1MUX_HD,
	N5_OUTMODE_1MUX_FHD,
	N5_OUTMODE_1MUX_FHD_HALF,
	N5_OUTMODE_2MUX_SD,
	N5_OUTMODE_2MUX_HD,
	N5_OUTMODE_2MUX_FHD,
	N5_OUTMODE_1MUX_BT1120S,
	N5_OUTMODE_2MUX_BT1120S_720P,
	N5_OUTMODE_2MUX_BT1120S_1080P,
	N5_OUTMODE_BUTT
}N5_OUTMODE_SEL;


unsigned int chn0 = AHD_720P_25P; //refer to NC_VIVO_CH_FORMATDEF_E
unsigned int chn1 = AHD_720P_30P; //refer to NC_VIVO_CH_FORMATDEF_E
unsigned int bt601 = 0; //0: bt656/bt1120 1:bt601

unsigned int ntpal = 1; //1:PAL or 25p, 0:ntsc or 30p

unsigned char chn_mode[2]={0xEE, 0xEE};

#undef uartlog
#define uartlog(fmt, args...) \
{ \
    printf(fmt, ##args); \
    fflush(stdout); \
}

#define CIF_DPHY_ENABLE

struct sensor_info n5_info = {
	.dev = 0x32,
	.alen = 1,
	.dlen = 1,
	.gain =  { .cur = 0, .min = 0, .max = 0xf0 },
	.sgain =  { .cur = 0, .min = 0, .max = 0xf0 },
	.shsl =  { .cur = 0, .min = 0, .max = 0 },
	.shss =  { .cur = 0, .min = 0, .max = 0 },
	.ops = &n5_ops,
};

// sensor mode for n5
struct sensor_mode n5_mode[] = {
	[N5_MODE_720P30] =
	{
		.name = "720p25",
		.bit_depth = 10,
		.bayer = SENSOR_BAYER_GR,
		.active_w = 1280,
		.active_h = 720,
		.total_w = 1280,
		.total_h = 720,
		.fp_w = 12,
		.bp_w = 16,
		.fp_h = 10,
		.bp_h = 6,	// in spec it shall be 5 but csi-bdg report frame err.
	},
};

static void n5_get_gain(void *priv, struct sensor_prop *prop)
{
	return;
}

static void n5_set_shsl(void *priv, uint32_t shs);

static void n5_set_gain(void *priv, uint16_t gain)
{
	return;
}

static void n5_get_sgain(void *priv, struct sensor_prop *prop)
{
	return;
}

static void n5_set_sgain(void *priv, uint16_t gain)
{
	return;
}

static void n5_get_shsl(void *priv, struct sensor_prop *prop)
{
	return;
}

static void n5_set_shsl(void *priv, uint32_t shs)
{
	return;
}

static void n5_get_shss(void *priv, struct sensor_prop *prop)
{
	return;
}

static void n5_set_shss(void *priv, uint32_t shs)
{
	return;
}

static void n5_test_mode(void *priv, uint32_t value)
{
	return;
}

struct sensor_ops n5_ops = {
	.get_gain = n5_get_gain,
	.set_gain = n5_set_gain,
	.get_sgain = n5_get_sgain,
	.set_sgain = n5_set_sgain,
	.get_shsl = n5_get_shsl,
	.set_shsl = n5_set_shsl,
	.get_shss = n5_get_shss,
	.set_shss = n5_set_shss,
	.test_mode = n5_test_mode,
};

static void n5_i2c_write(uint8_t dev, uint16_t reg, uint16_t data)
{
	struct sensor_info *sinf = &n5_info;
	uint16_t tmp = data;
	int r;

	if (!sinf->adpt)
		return;

	r = sinf->adpt->write(sinf->adpt, dev, reg, sinf->alen, (uint8_t*)&tmp, 1);
	if (r < 0)
		uartlog("i2c_write fail addr[0x%x]\n", reg);
}

static uint8_t n5_i2c_read(uint8_t dev, uint16_t reg)
{
	struct sensor_info *sinf = &n5_info;
	uint16_t tmp;
	int r;

	if (!sinf->adpt)
		return 0;

	r = sinf->adpt->read(sinf->adpt, dev, reg, sinf->alen, (uint8_t*)&tmp, 1);
	if (r < 0)
		uartlog("i2c_write fail addr[0x%x]\n", reg);

	return (uint8_t)tmp;
}

int n5_intr(void *priv)
{
	return 0;
}

void n5_common_setting(unsigned char dev)
{
	n5_i2c_write(dev, 0xff, 0x00);
	n5_i2c_write(dev, 0x00, 0x10);
	n5_i2c_write(dev, 0x01, 0x10);
	n5_i2c_write(dev, 0x18, 0x3f);
	n5_i2c_write(dev, 0x19, 0x3f);
	n5_i2c_write(dev, 0x22, 0x0b);
	n5_i2c_write(dev, 0x23, 0x41);
	n5_i2c_write(dev, 0x26, 0x0b);
	n5_i2c_write(dev, 0x27, 0x41);
	n5_i2c_write(dev, 0x54, 0x00);
	n5_i2c_write(dev, 0xa0, 0x05);
	n5_i2c_write(dev, 0xa1, 0x05);
	n5_i2c_write(dev, 0xff, 0x01);
	n5_i2c_write(dev, 0x97, 0x00);		
	n5_i2c_write(dev, 0x97, 0x0f);
	n5_i2c_write(dev, 0x7A, 0x0f);
	n5_i2c_write(dev, 0xff, 0x05);
	n5_i2c_write(dev, 0x00, 0xd0);
	n5_i2c_write(dev, 0x01, 0x22);
	n5_i2c_write(dev, 0x05, 0x04);
	n5_i2c_write(dev, 0x08, 0x55);
	n5_i2c_write(dev, 0x1b, 0x08);
	n5_i2c_write(dev, 0x25, 0xdc);
	n5_i2c_write(dev, 0x28, 0x80);
	n5_i2c_write(dev, 0x2f, 0x00);
	n5_i2c_write(dev, 0x30, 0xe0);
	n5_i2c_write(dev, 0x31, 0x43);
	n5_i2c_write(dev, 0x32, 0xa2);
	n5_i2c_write(dev, 0x57, 0x00);
	n5_i2c_write(dev, 0x58, 0x77);
	n5_i2c_write(dev, 0x5b, 0x41);
	n5_i2c_write(dev, 0x5c, 0x7C);
	n5_i2c_write(dev, 0x5f, 0x00);
	n5_i2c_write(dev, 0x7b, 0x11);
	n5_i2c_write(dev, 0x7c, 0x01);
	n5_i2c_write(dev, 0x7d, 0x80);
	n5_i2c_write(dev, 0x80, 0x00);
	n5_i2c_write(dev, 0x90, 0x01);
	n5_i2c_write(dev, 0xa9, 0x00);
	n5_i2c_write(dev, 0xb8, 0x39);
	n5_i2c_write(dev, 0xb9, 0x72);
	n5_i2c_write(dev, 0xd1, 0x00);
	n5_i2c_write(dev, 0xd5, 0x80);
	n5_i2c_write(dev, 0xff, 0x06);
	n5_i2c_write(dev, 0x00, 0xd0);
	n5_i2c_write(dev, 0x01, 0x22);
	n5_i2c_write(dev, 0x05, 0x04);
	n5_i2c_write(dev, 0x08, 0x55);
	n5_i2c_write(dev, 0x1b, 0x08);
	n5_i2c_write(dev, 0x25, 0xdc);
	n5_i2c_write(dev, 0x28, 0x80);
	n5_i2c_write(dev, 0x2f, 0x00);
	n5_i2c_write(dev, 0x30, 0xe0);
	n5_i2c_write(dev, 0x31, 0x43);
	n5_i2c_write(dev, 0x32, 0xa2);
	n5_i2c_write(dev, 0x57, 0x00);
	n5_i2c_write(dev, 0x58, 0x77);
	n5_i2c_write(dev, 0x5b, 0x41);
	n5_i2c_write(dev, 0x5c, 0x7C);
	n5_i2c_write(dev, 0x5f, 0x00);
	n5_i2c_write(dev, 0x7b, 0x11);
	n5_i2c_write(dev, 0x7c, 0x01);
	n5_i2c_write(dev, 0x7d, 0x80);
	n5_i2c_write(dev, 0x80, 0x00);
	n5_i2c_write(dev, 0x90, 0x01);
	n5_i2c_write(dev, 0xa9, 0x00);
	n5_i2c_write(dev, 0xb8, 0x39);
	n5_i2c_write(dev, 0xb9, 0x72);
	n5_i2c_write(dev, 0xd1, 0x00);
	n5_i2c_write(dev, 0xd5, 0x80);
	n5_i2c_write(dev, 0xff, 0x09);
	n5_i2c_write(dev, 0x50, 0x30);
	n5_i2c_write(dev, 0x51, 0x6f);
	n5_i2c_write(dev, 0x52, 0x67);
	n5_i2c_write(dev, 0x53, 0x48);
	n5_i2c_write(dev, 0x54, 0x30);
	n5_i2c_write(dev, 0x55, 0x6f);
	n5_i2c_write(dev, 0x56, 0x67);
	n5_i2c_write(dev, 0x57, 0x48);
	n5_i2c_write(dev, 0x96, 0x00);
	n5_i2c_write(dev, 0x9e, 0x00);
	n5_i2c_write(dev, 0xb6, 0x00);
	n5_i2c_write(dev, 0xbe, 0x00);
	n5_i2c_write(dev, 0xff, 0x0a);
	n5_i2c_write(dev, 0x25, 0x10);
	n5_i2c_write(dev, 0x27, 0x1e);
	n5_i2c_write(dev, 0x30, 0xac);
	n5_i2c_write(dev, 0x31, 0x78);
	n5_i2c_write(dev, 0x32, 0x17);
	n5_i2c_write(dev, 0x33, 0xc1);
	n5_i2c_write(dev, 0x34, 0x40);
	n5_i2c_write(dev, 0x35, 0x00);
	n5_i2c_write(dev, 0x36, 0xc3);
	n5_i2c_write(dev, 0x37, 0x0a);
	n5_i2c_write(dev, 0x38, 0x00);
	n5_i2c_write(dev, 0x39, 0x02);
	n5_i2c_write(dev, 0x3a, 0x00);
	n5_i2c_write(dev, 0x3b, 0xb2);
	n5_i2c_write(dev, 0xa5, 0x10);
	n5_i2c_write(dev, 0xa7, 0x1e);
	n5_i2c_write(dev, 0xb0, 0xac);
	n5_i2c_write(dev, 0xb1, 0x78);
	n5_i2c_write(dev, 0xb2, 0x17);
	n5_i2c_write(dev, 0xb3, 0xc1);
	n5_i2c_write(dev, 0xb4, 0x40);
	n5_i2c_write(dev, 0xb5, 0x00);
	n5_i2c_write(dev, 0xb6, 0xc3);
	n5_i2c_write(dev, 0xb7, 0x0a);
	n5_i2c_write(dev, 0xb8, 0x00);
	n5_i2c_write(dev, 0xb9, 0x02);
	n5_i2c_write(dev, 0xba, 0x00);
	n5_i2c_write(dev, 0xbb, 0xb2);
	n5_i2c_write(dev, 0x77, 0x8F);
	n5_i2c_write(dev, 0xF7, 0x8F);
	n5_i2c_write(dev, 0xff, 0x13);
	n5_i2c_write(dev, 0x07, 0x47);
	n5_i2c_write(dev, 0x12, 0x04);
	n5_i2c_write(dev, 0x1e, 0x1f);
	n5_i2c_write(dev, 0x1f, 0x27);
	n5_i2c_write(dev, 0x2e, 0x10);
	n5_i2c_write(dev, 0x2f, 0xc8);
	n5_i2c_write(dev, 0x30, 0x00);
	n5_i2c_write(dev, 0x31, 0xff);
	n5_i2c_write(dev, 0x32, 0x00);
	n5_i2c_write(dev, 0x33, 0x00);
	n5_i2c_write(dev, 0x3a, 0xff);	
	n5_i2c_write(dev, 0x3b, 0xff);	
	n5_i2c_write(dev, 0x3c, 0xff);	
	n5_i2c_write(dev, 0x3d, 0xff);	
	n5_i2c_write(dev, 0x3e, 0xff);	
	n5_i2c_write(dev, 0x3f, 0x0f);	
	n5_i2c_write(dev, 0x70, 0x00);	
	n5_i2c_write(dev, 0x72, 0x05);	
	n5_i2c_write(dev, 0x7A, 0xf0);
	n5_i2c_write(dev, 0xff, 0x00); //8x8 color block test pattern
	n5_i2c_write(dev, 0x78, 0xba);
	n5_i2c_write(dev, 0xff, 0x05);
	n5_i2c_write(dev, 0x2c, 0x08);
	n5_i2c_write(dev, 0x6a, 0x80);
	n5_i2c_write(dev, 0xff, 0x06);
	n5_i2c_write(dev, 0x2c, 0x08);
	n5_i2c_write(dev, 0x6a, 0x80);
}

void n5_set_chn_720h_ntsc(unsigned char dev, unsigned char chn)
{
	uartlog("%s chn=%d\n", __func__, chn);
	n5_i2c_write(dev, 0xff, 0x00);
	n5_i2c_write(dev, 0x08+chn, 0xa0);
	n5_i2c_write(dev, 0x34+chn, 0x00);
	n5_i2c_write(dev, 0x81+chn, 0x60);
	n5_i2c_write(dev, 0x85+chn, 0x00);
	n5_i2c_write(dev, 0x54, n5_i2c_read(dev, 0x54)|(0x10<<chn));
	n5_i2c_write(dev, 0x18+chn, 0x08);
	n5_i2c_write(dev, 0x58+chn, 0x90);
	n5_i2c_write(dev, 0x5c+chn, 0xbc);
	n5_i2c_write(dev, 0x64+chn, 0xa1);
	n5_i2c_write(dev, 0x89+chn, 0x10);
	n5_i2c_write(dev, chn+0x8e, 0x2f);
	n5_i2c_write(dev, 0x30+chn, 0x12);
	n5_i2c_write(dev, 0xa0+chn, 0x05);

	n5_i2c_write(dev, 0xff, 0x01);
	n5_i2c_write(dev, 0x84+chn, 0x06);
	n5_i2c_write(dev, 0x8c+chn, 0x86);
	n5_i2c_write(dev, 0xed, n5_i2c_read(dev, 0xed)&(~(0x01<<chn)));

	n5_i2c_write(dev, 0xff, 0x05+chn);
	n5_i2c_write(dev, 0x20, 0x90);
	n5_i2c_write(dev, 0x25, 0xdc);
	n5_i2c_write(dev, 0x2b, 0x78);
	n5_i2c_write(dev, 0x47, 0x04);
	n5_i2c_write(dev, 0x50, 0x84);
	n5_i2c_write(dev, 0x69, 0x00);
	n5_i2c_write(dev, 0x7b, 0x00);
	n5_i2c_write(dev, 0xb8, 0xb9);	
}
void n5_set_chn_720h_pal(unsigned char dev, unsigned char chn)
{
	uartlog("%s chn=%d\n", __func__, chn);
	n5_i2c_write(dev, 0xff, 0x00);
	n5_i2c_write(dev, 0x08+chn, 0xdd);
	n5_i2c_write(dev, 0x34+chn, 0x00);
	n5_i2c_write(dev, 0x81+chn, 0x70);
	n5_i2c_write(dev, 0x85+chn, 0x00);
	n5_i2c_write(dev, 0x54, n5_i2c_read(dev, 0x54)&(~(0x10<<chn)));
	n5_i2c_write(dev, 0x18+chn, 0x08);
	n5_i2c_write(dev, 0x58+chn, 0x80);
	n5_i2c_write(dev, 0x5c+chn, 0xbe);
	n5_i2c_write(dev, 0x64+chn, 0xa0);
	n5_i2c_write(dev, 0x89+chn, 0x10);
	n5_i2c_write(dev, chn+0x8e, 0x2e);
	n5_i2c_write(dev, 0x30+chn, 0x12);
	n5_i2c_write(dev, 0xa0+chn, 0x05);

	n5_i2c_write(dev, 0xff, 0x01);
	n5_i2c_write(dev, 0x84+chn, 0x06);
	n5_i2c_write(dev, 0x8c+chn, 0x86);
	n5_i2c_write(dev, 0xed, n5_i2c_read(dev, 0xed)&(~(0x01<<chn)));

	n5_i2c_write(dev, 0xff, 0x05+chn);
	n5_i2c_write(dev, 0x20, 0x90);
	n5_i2c_write(dev, 0x25, 0xcc);
	n5_i2c_write(dev, 0x2b, 0x78);
	n5_i2c_write(dev, 0x47, 0x04);
	n5_i2c_write(dev, 0x50, 0x84);
	n5_i2c_write(dev, 0x69, 0x00);
	n5_i2c_write(dev, 0x7b, 0x00);
	n5_i2c_write(dev, 0xb8, 0xb9);	
}

void n5_set_chn_1920h_ntsc(unsigned char dev, unsigned char chn)
{
	uartlog("%s chn=%d\n", __func__, chn);
	n5_i2c_write(dev, 0xff, 0x00);
	n5_i2c_write(dev, 0x08+chn, 0xa0);
	n5_i2c_write(dev, 0x34+chn, 0x00);
	n5_i2c_write(dev, 0x81+chn, 0xa0);
	n5_i2c_write(dev, 0x85+chn, 0x00);
	n5_i2c_write(dev, 0x54, n5_i2c_read(dev, 0x54)|(0x10<<chn));
	n5_i2c_write(dev, 0x18+chn, 0x08);
	n5_i2c_write(dev, 0x58+chn, 0x2a);
	n5_i2c_write(dev, 0x5c+chn, 0xbc);
	n5_i2c_write(dev, 0x64+chn, 0x81);
	n5_i2c_write(dev, 0x89+chn, 0x00);
	n5_i2c_write(dev, chn+0x8e, 0x00);
	n5_i2c_write(dev, 0x30+chn, 0x12);
	n5_i2c_write(dev, 0xa0+chn, 0x05);

	n5_i2c_write(dev, 0xff, 0x01);
	n5_i2c_write(dev, 0x84+chn, 0x06);
	n5_i2c_write(dev, 0x8c+chn, 0x06);
	n5_i2c_write(dev, 0xed, n5_i2c_read(dev, 0xed)&(~(0x01<<chn)));

	n5_i2c_write(dev, 0xff, 0x05+chn);
	n5_i2c_write(dev, 0x20, 0x90);
	n5_i2c_write(dev, 0x25, 0xdc);
	n5_i2c_write(dev, 0x2b, 0x78);
	n5_i2c_write(dev, 0x47, 0x04);
	n5_i2c_write(dev, 0x50, 0x84);
	n5_i2c_write(dev, 0x69, 0x00);
	n5_i2c_write(dev, 0x7b, 0x00);
	n5_i2c_write(dev, 0xb8, 0xb9);	
}
void n5_set_chn_1920h_pal(unsigned char dev, unsigned char chn)
{
	uartlog("%s chn=%d\n", __func__, chn);
	n5_i2c_write(dev, 0xff, 0x00);
	n5_i2c_write(dev, 0x08+chn, 0xdd);
	n5_i2c_write(dev, 0x34+chn, 0x00);
	n5_i2c_write(dev, 0x81+chn, 0xb0);
	n5_i2c_write(dev, 0x85+chn, 0x00);
	n5_i2c_write(dev, 0x54, n5_i2c_read(dev, 0x54)&(~(0x10<<chn)));
	n5_i2c_write(dev, 0x18+chn, 0x08);
	n5_i2c_write(dev, 0x58+chn, 0x42);
	n5_i2c_write(dev, 0x5c+chn, 0xbe);
	n5_i2c_write(dev, 0x64+chn, 0xa0);
	n5_i2c_write(dev, 0x89+chn, 0x00);
	n5_i2c_write(dev, chn+0x8e, 0x00);
	n5_i2c_write(dev, 0x30+chn, 0x12);
	n5_i2c_write(dev, 0xa0+chn, 0x05);

	n5_i2c_write(dev, 0xff, 0x01);
	n5_i2c_write(dev, 0x84+chn, 0x06);
	n5_i2c_write(dev, 0x8c+chn, 0x06);
	n5_i2c_write(dev, 0xed, n5_i2c_read(dev, 0xed)&(~(0x01<<chn)));

	n5_i2c_write(dev, 0xff, 0x05+chn);
	n5_i2c_write(dev, 0x20, 0x90);
	n5_i2c_write(dev, 0x25, 0xcc);
	n5_i2c_write(dev, 0x2b, 0x78);
	n5_i2c_write(dev, 0x47, 0x04);
	n5_i2c_write(dev, 0x50, 0x84);
	n5_i2c_write(dev, 0x69, 0x00);
	n5_i2c_write(dev, 0x7b, 0x00);
	n5_i2c_write(dev, 0xb8, 0xb9);	
}


void n5_set_chn_720p_30(unsigned char dev, unsigned char chn)
{
	uartlog("%s chn=%d\n", __func__, chn);
	n5_i2c_write(dev, 0xff, 0x00);
	n5_i2c_write(dev, 0x08+chn, 0x00);
	n5_i2c_write(dev, 0x34+chn, 0x00);
	n5_i2c_write(dev, 0x81+chn, 0x06);
	n5_i2c_write(dev, 0x85+chn, 0x00);
	n5_i2c_write(dev, 0x54, n5_i2c_read(dev, 0x54)&(~(0x10<<chn)));
	n5_i2c_write(dev, 0x18+chn, 0x3f);
	n5_i2c_write(dev, 0x58+chn, 0x80);
	n5_i2c_write(dev, 0x5c+chn, 0x80);
	n5_i2c_write(dev, 0x64+chn, 0x01);
	n5_i2c_write(dev, 0x89+chn, 0x00);
	n5_i2c_write(dev, chn+0x8e, 0x00);
	n5_i2c_write(dev, 0x30+chn, 0x12);
	n5_i2c_write(dev, 0xa0+chn, 0x05);
	
	n5_i2c_write(dev, 0xff, 0x01);
	n5_i2c_write(dev, 0x84+chn, 0x08);
	n5_i2c_write(dev, 0x8c+chn, 0x08);
	n5_i2c_write(dev, 0xed, n5_i2c_read(dev, 0xed)&(~(0x01<<chn)));
	
	n5_i2c_write(dev, 0xff, 0x05+chn);
	n5_i2c_write(dev, 0x20, 0x84);
	n5_i2c_write(dev, 0x25, 0xdc);
	n5_i2c_write(dev, 0x2b, 0xa8);
	n5_i2c_write(dev, 0x47, 0xee);
	n5_i2c_write(dev, 0x50, 0xc6);
	n5_i2c_write(dev, 0x69, 0x00);
	n5_i2c_write(dev, 0x7b, 0x11);
	n5_i2c_write(dev, 0xb8, 0x39);	
}

void n5_set_chn_720p_25(unsigned char dev, unsigned char chn)
{
	uartlog("%s chn=%d\n", __func__, chn);
	n5_i2c_write(dev, 0xff, 0x00);
	n5_i2c_write(dev, 0x08+chn, 0x00);
	n5_i2c_write(dev, 0x34+chn, 0x00);
	n5_i2c_write(dev, 0x81+chn, 0x07);
	n5_i2c_write(dev, 0x85+chn, 0x00);
	n5_i2c_write(dev, 0x54, n5_i2c_read(dev, 0x54)&(~(0x10<<chn)));
	n5_i2c_write(dev, 0x18+chn, 0x3f);
	n5_i2c_write(dev, 0x58+chn, 0x80);
	n5_i2c_write(dev, 0x5c+chn, 0x80);
	n5_i2c_write(dev, 0x64+chn, 0x01);
	n5_i2c_write(dev, 0x89+chn, 0x00);
	n5_i2c_write(dev, chn+0x8e, 0x00);
	n5_i2c_write(dev, 0x30+chn, 0x12);
	n5_i2c_write(dev, 0xa0+chn, 0x05);

	n5_i2c_write(dev, 0xff, 0x01);
	n5_i2c_write(dev, 0x84+chn, 0x08);
	n5_i2c_write(dev, 0x8c+chn, 0x08);
	n5_i2c_write(dev, 0xed, n5_i2c_read(dev, 0xed)&(~(0x01<<chn)));

	n5_i2c_write(dev, 0xff, 0x05+chn);
	n5_i2c_write(dev, 0x20, 0x84);
	n5_i2c_write(dev, 0x25, 0xdc);
	n5_i2c_write(dev, 0x2b, 0xa8);
	n5_i2c_write(dev, 0x47, 0xee);
	n5_i2c_write(dev, 0x50, 0xc6);
	n5_i2c_write(dev, 0x69, 0x00);
	n5_i2c_write(dev, 0x7b, 0x11);
	n5_i2c_write(dev, 0xb8, 0x39);	
}

void n5_set_chn_1080p_30(unsigned char dev, unsigned char chn)
{
	uartlog("%s chn=%d\n", __func__, chn);
	n5_i2c_write(dev, 0xff, 0x00);
	n5_i2c_write(dev, 0x08+chn, 0x00);
	n5_i2c_write(dev, 0x34+chn, 0x00);
	n5_i2c_write(dev, 0x81+chn, 0x02);
	n5_i2c_write(dev, 0x85+chn, 0x00);
	n5_i2c_write(dev, 0x54, n5_i2c_read(dev, 0x54)&(~(0x10<<chn)));
	n5_i2c_write(dev, 0x18+chn, 0x3f);
	n5_i2c_write(dev, 0x58+chn, 0x78);
	n5_i2c_write(dev, 0x5c+chn, 0x80);
	n5_i2c_write(dev, 0x64+chn, 0x01);
	n5_i2c_write(dev, 0x89+chn, 0x10);
	n5_i2c_write(dev, chn+0x8e, 0x00);
	n5_i2c_write(dev, 0x30+chn, 0x12);
	n5_i2c_write(dev, 0xa0+chn, 0x05);

	n5_i2c_write(dev, 0xff, 0x01);
	n5_i2c_write(dev, 0x84+chn, 0x00);
	n5_i2c_write(dev, 0x8c+chn, 0x40);
	n5_i2c_write(dev, 0xed, n5_i2c_read(dev, 0xed)&(~(0x01<<chn)));

	n5_i2c_write(dev, 0xff, 0x05+chn);
	n5_i2c_write(dev, 0x20, 0x84);
	n5_i2c_write(dev, 0x25, 0xdc);
	n5_i2c_write(dev, 0x2b, 0xa8);
	n5_i2c_write(dev, 0x47, 0xee);
	n5_i2c_write(dev, 0x50, 0xc6);
	n5_i2c_write(dev, 0x69, 0x00);
	n5_i2c_write(dev, 0x7b, 0x11);
	n5_i2c_write(dev, 0xb8, 0x39);
	
	//n5_i2c_write(dev, 0xff, 0x13);
	//n5_i2c_write(dev, 0x70, n5_i2c_read(dev, 0x70)|(0x01<<chn));
	
}

void n5_set_chn_1080p_25(unsigned char dev, unsigned char chn)
{
	uartlog("%s chn=%d\n", __func__, chn);
	n5_i2c_write(dev, 0xff, 0x00);
	n5_i2c_write(dev, 0x08+chn, 0x00);
	n5_i2c_write(dev, 0x34+chn, 0x00);
	n5_i2c_write(dev, 0x81+chn, 0x03);
	n5_i2c_write(dev, 0x85+chn, 0x00);
	n5_i2c_write(dev, 0x54, n5_i2c_read(dev, 0x54)&(~(0x10<<chn)));
	n5_i2c_write(dev, 0x18+chn, 0x3f);
	n5_i2c_write(dev, 0x58+chn, 0x82);
	n5_i2c_write(dev, 0x5c+chn, 0x80);
	n5_i2c_write(dev, 0x64+chn, 0x01);
	n5_i2c_write(dev, 0x89+chn, 0x10);
	n5_i2c_write(dev, chn+0x8e, 0x00);
	n5_i2c_write(dev, 0x30+chn, 0x12);
	n5_i2c_write(dev, 0xa0+chn, 0x05);

	n5_i2c_write(dev, 0xff, 0x01);
	n5_i2c_write(dev, 0x84+chn, 0x00);
	n5_i2c_write(dev, 0x8c+chn, 0x40);
	n5_i2c_write(dev, 0xed, n5_i2c_read(dev, 0xed)&(~(0x01<<chn)));

	n5_i2c_write(dev, 0xff, 0x05+chn);
	n5_i2c_write(dev, 0x20, 0x84);
	n5_i2c_write(dev, 0x25, 0xdc);
	n5_i2c_write(dev, 0x2b, 0xa8);
	n5_i2c_write(dev, 0x47, 0xee);
	n5_i2c_write(dev, 0x50, 0xc6);
	n5_i2c_write(dev, 0x69, 0x00);
	n5_i2c_write(dev, 0x7b, 0x11);
	n5_i2c_write(dev, 0xb8, 0x39);	
}

void n5_set_chn_1080p_60(unsigned char dev, unsigned char chn)
{
	uartlog("%s chn=%d\n", __func__, chn);
	n5_i2c_write(dev, 0xff, 0x00);
	n5_i2c_write(dev, 0x08+chn, 0x00);
	n5_i2c_write(dev, 0x34+chn, 0x00);
	n5_i2c_write(dev, 0x81+chn, 0x02);
	n5_i2c_write(dev, 0x85+chn, 0x0b);
	n5_i2c_write(dev, 0x54, n5_i2c_read(dev, 0x54)&(~(0x10<<chn)));
	n5_i2c_write(dev, 0x18+chn, 0x3f);
	n5_i2c_write(dev, 0x58+chn, 0x78);
	n5_i2c_write(dev, 0x5c+chn, 0x80);
	n5_i2c_write(dev, 0x64+chn, 0x01);
	n5_i2c_write(dev, 0x89+chn, 0x10);
	n5_i2c_write(dev, chn+0x8e, 0x00);
	n5_i2c_write(dev, 0x30+chn, 0x12);
	n5_i2c_write(dev, 0xa0+chn, 0x05);

	n5_i2c_write(dev, 0xff, 0x01);
	n5_i2c_write(dev, 0x84+chn, 0x04);
	n5_i2c_write(dev, 0x8c+chn, 0x44);
	n5_i2c_write(dev, 0xed, n5_i2c_read(dev, 0xed)|(0x01<<chn));

	n5_i2c_write(dev, 0xff, 0x05+chn);
	n5_i2c_write(dev, 0x20, 0x84);
	n5_i2c_write(dev, 0x25, 0xdc);
	n5_i2c_write(dev, 0x2b, 0xa8);
	n5_i2c_write(dev, 0x47, 0xee);
	n5_i2c_write(dev, 0x50, 0xc6);
	n5_i2c_write(dev, 0x69, 0x00);
	n5_i2c_write(dev, 0x7b, 0x00);
	n5_i2c_write(dev, 0xb8, 0x39);
		
}

void n5_set_chn_1080p_50(unsigned char dev, unsigned char chn)
{
	uartlog("%s chn=%d\n", __func__, chn);
	n5_i2c_write(dev, 0xff, 0x00);
	n5_i2c_write(dev, 0x08+chn, 0x00);
	n5_i2c_write(dev, 0x34+chn, 0x00);
	n5_i2c_write(dev, 0x81+chn, 0x03);
	n5_i2c_write(dev, 0x85+chn, 0x0b);
	n5_i2c_write(dev, 0x54, n5_i2c_read(dev, 0x54)&(~(0x10<<chn)));
	n5_i2c_write(dev, 0x18+chn, 0x3f);
	n5_i2c_write(dev, 0x58+chn, 0x82);
	n5_i2c_write(dev, 0x5c+chn, 0x80);
	n5_i2c_write(dev, 0x64+chn, 0x00);
	n5_i2c_write(dev, 0x89+chn, 0x00);
	n5_i2c_write(dev, chn+0x8e, 0x00);
	n5_i2c_write(dev, 0x30+chn, 0x12);
	n5_i2c_write(dev, 0xa0+chn, 0x05);

	n5_i2c_write(dev, 0xff, 0x01);
	n5_i2c_write(dev, 0x84+chn, 0x04);
	n5_i2c_write(dev, 0x8c+chn, 0x44);
	n5_i2c_write(dev, 0xed, n5_i2c_read(dev, 0xed)|(0x01<<chn));

	n5_i2c_write(dev, 0xff, 0x05+chn);
	n5_i2c_write(dev, 0x20, 0x84);
	n5_i2c_write(dev, 0x25, 0xdc);
	n5_i2c_write(dev, 0x2b, 0xa8);
	n5_i2c_write(dev, 0x47, 0xee);
	n5_i2c_write(dev, 0x50, 0xc6);
	n5_i2c_write(dev, 0x69, 0x00);
	n5_i2c_write(dev, 0x7b, 0x00);
	n5_i2c_write(dev, 0xb8, 0x39);	
}

void n5_set_chn_960p_30(unsigned char dev, unsigned char chn)
{
	uartlog("%s chn=%d\n", __func__, chn);
	n5_i2c_write(dev, 0xff, 0x00);
	n5_i2c_write(dev, 0x08+chn, 0x00);
	n5_i2c_write(dev, 0x34+chn, 0x00);
	n5_i2c_write(dev, 0x81+chn, 0x06);
	n5_i2c_write(dev, 0x85+chn, 0x0b);
	n5_i2c_write(dev, 0x54, n5_i2c_read(dev, 0x54)&(~(0x10<<chn)));
	n5_i2c_write(dev, 0x18+chn, 0x3f);
	n5_i2c_write(dev, 0x58+chn, 0x48);
	n5_i2c_write(dev, 0x5c+chn, 0x80);
	n5_i2c_write(dev, 0x64+chn, 0x00);
	n5_i2c_write(dev, 0x89+chn, 0x00);
	n5_i2c_write(dev, chn+0x8e, 0x00);
	n5_i2c_write(dev, 0x30+chn, 0x12);
	n5_i2c_write(dev, 0xa0+chn, 0x05);

	n5_i2c_write(dev, 0xff, 0x01);
	n5_i2c_write(dev, 0x84+chn, 0x00);
	n5_i2c_write(dev, 0x8c+chn, 0x40);
	n5_i2c_write(dev, 0xed, n5_i2c_read(dev, 0xed)&(~(0x01<<chn)));

	n5_i2c_write(dev, 0xff, 0x05+chn);
	n5_i2c_write(dev, 0x20, 0x84);
	n5_i2c_write(dev, 0x25, 0xdc);
	n5_i2c_write(dev, 0x2b, 0xa8);
	n5_i2c_write(dev, 0x47, 0xee);
	n5_i2c_write(dev, 0x50, 0xc6);
	n5_i2c_write(dev, 0x69, 0x00);
	n5_i2c_write(dev, 0x7b, 0x11);
	n5_i2c_write(dev, 0xb8, 0x39);	
}

void n5_set_chn_960p_25(unsigned char dev, unsigned char chn)
{
	uartlog("%s chn=%d\n", __func__, chn);
	n5_i2c_write(dev, 0xff, 0x00);
	n5_i2c_write(dev, 0x08+chn, 0x00);
	n5_i2c_write(dev, 0x34+chn, 0x00);
	n5_i2c_write(dev, 0x81+chn, 0x07);
	n5_i2c_write(dev, 0x85+chn, 0x0b);
	n5_i2c_write(dev, 0x54, n5_i2c_read(dev, 0x54)&(~(0x10<<chn)));
	n5_i2c_write(dev, 0x18+chn, 0x3f);
	n5_i2c_write(dev, 0x58+chn, 0x40);
	n5_i2c_write(dev, 0x5c+chn, 0x80);
	n5_i2c_write(dev, 0x64+chn, 0x00);
	n5_i2c_write(dev, 0x89+chn, 0x00);
	n5_i2c_write(dev, chn+0x8e, 0x00);
	n5_i2c_write(dev, 0x30+chn, 0x12);
	n5_i2c_write(dev, 0xa0+chn, 0x05);

	n5_i2c_write(dev, 0xff, 0x01);
	n5_i2c_write(dev, 0x84+chn, 0x00);
	n5_i2c_write(dev, 0x8c+chn, 0x40);
	n5_i2c_write(dev, 0xed, n5_i2c_read(dev, 0xed)&(~(0x01<<chn)));

	n5_i2c_write(dev, 0xff, 0x05+chn);
	n5_i2c_write(dev, 0x20, 0x84);
	n5_i2c_write(dev, 0x25, 0xdc);
	n5_i2c_write(dev, 0x2b, 0xa8);
	n5_i2c_write(dev, 0x47, 0xee);
	n5_i2c_write(dev, 0x50, 0xc6);
	n5_i2c_write(dev, 0x69, 0x00);
	n5_i2c_write(dev, 0x7b, 0x11);
	n5_i2c_write(dev, 0xb8, 0x39);	
}

void n5_set_chn_2k_30(unsigned char dev, unsigned char chn)
{
	uartlog("%s chn=%d\n", __func__, chn);
	n5_i2c_write(dev, 0xff, 0x00);
	n5_i2c_write(dev, 0x08+chn, 0x00);
	n5_i2c_write(dev, 0x34+chn, 0x00);
	n5_i2c_write(dev, 0x81+chn, 0x0e);
	n5_i2c_write(dev, 0x85+chn, 0x00);
	n5_i2c_write(dev, 0x54, n5_i2c_read(dev, 0x54)&(~(0x10<<chn)));
	n5_i2c_write(dev, 0x18+chn, 0x1f);
	n5_i2c_write(dev, 0x58+chn, 0x82);
	n5_i2c_write(dev, 0x5c+chn, 0x80);
	n5_i2c_write(dev, 0x64+chn, 0x00);
	n5_i2c_write(dev, 0x89+chn, 0x00);
	n5_i2c_write(dev, chn+0x8e, 0x00);
	n5_i2c_write(dev, 0x30+chn, 0x12);
	n5_i2c_write(dev, 0xa0+chn, 0x05);

	n5_i2c_write(dev, 0xff, 0x01);
	n5_i2c_write(dev, 0x84+chn, 0x04);
	n5_i2c_write(dev, 0x8c+chn, 0x44);
	n5_i2c_write(dev, 0xed, n5_i2c_read(dev, 0xed)|(0x01<<chn));

	n5_i2c_write(dev, 0xff, 0x05+chn);
	n5_i2c_write(dev, 0x20, 0x84);
	n5_i2c_write(dev, 0x25, 0xdc);
	n5_i2c_write(dev, 0x2b, 0xa8);
	n5_i2c_write(dev, 0x47, 0xee);
	n5_i2c_write(dev, 0x50, 0xc6);
	n5_i2c_write(dev, 0x69, 0x10);
	n5_i2c_write(dev, 0x7b, 0x00);
	n5_i2c_write(dev, 0xb8, 0x39);
}


void n5_set_chn_2k_25(unsigned char dev, unsigned char chn)
{
	uartlog("%s chn=%d\n", __func__, chn);
	n5_i2c_write(dev, 0xff, 0x00);
	n5_i2c_write(dev, 0x08+chn, 0x00);
	n5_i2c_write(dev, 0x34+chn, 0x00);
	n5_i2c_write(dev, 0x81+chn, 0x0f);
	n5_i2c_write(dev, 0x85+chn, 0x00);
	n5_i2c_write(dev, 0x54, n5_i2c_read(dev, 0x54)&(~(0x10<<chn)));
	n5_i2c_write(dev, 0x18+chn, 0x1f);
	n5_i2c_write(dev, 0x58+chn, 0x84);
	n5_i2c_write(dev, 0x5c+chn, 0x80);
	n5_i2c_write(dev, 0x64+chn, 0x00);
	n5_i2c_write(dev, 0x89+chn, 0x00);
	n5_i2c_write(dev, chn+0x8e, 0x00);
	n5_i2c_write(dev, 0x30+chn, 0x12);
	n5_i2c_write(dev, 0xa0+chn, 0x05);

	n5_i2c_write(dev, 0xff, 0x01);
	n5_i2c_write(dev, 0x84+chn, 0x04);
	n5_i2c_write(dev, 0x8c+chn, 0x44);
	n5_i2c_write(dev, 0xed, n5_i2c_read(dev, 0xed)|(0x01<<chn));

	n5_i2c_write(dev, 0xff, 0x05+chn);
	n5_i2c_write(dev, 0x20, 0x84);
	n5_i2c_write(dev, 0x25, 0xdc);
	n5_i2c_write(dev, 0x2b, 0xa8);
	n5_i2c_write(dev, 0x47, 0xee);
	n5_i2c_write(dev, 0x50, 0xc6);
	n5_i2c_write(dev, 0x69, 0x10);
	n5_i2c_write(dev, 0x7b, 0x00);
	n5_i2c_write(dev, 0xb8, 0x39);	
}

void n5_set_chn_4k_15(unsigned char dev, unsigned char chn)
{
	uartlog("%s chn=%d\n", __func__, chn);
	n5_i2c_write(dev, 0xff, 0x00);
	n5_i2c_write(dev, 0x08+chn, 0x00);
	n5_i2c_write(dev, 0x34+chn, 0x00);
	n5_i2c_write(dev, 0x81+chn, 0x02);
	n5_i2c_write(dev, 0x85+chn, 0x08);
	n5_i2c_write(dev, 0x54, n5_i2c_read(dev, 0x54)&(~(0x10<<chn)));
	n5_i2c_write(dev, 0x18+chn, 0x1f);
	n5_i2c_write(dev, 0x58+chn, 0x82);
	n5_i2c_write(dev, 0x5c+chn, 0x80);
	n5_i2c_write(dev, 0x64+chn, 0x00);
	n5_i2c_write(dev, 0x89+chn, 0x00);
	n5_i2c_write(dev, chn+0x8e, 0x00);
	n5_i2c_write(dev, 0x30+chn, 0x12);
	n5_i2c_write(dev, 0xa0+chn, 0x05);

	n5_i2c_write(dev, 0xff, 0x01);
	n5_i2c_write(dev, 0x84+chn, 0x04);
	n5_i2c_write(dev, 0x8c+chn, 0x44);
	n5_i2c_write(dev, 0xed, n5_i2c_read(dev, 0xed)|(0x01<<chn));

	n5_i2c_write(dev, 0xff, 0x05+chn);
	n5_i2c_write(dev, 0x20, 0x84);
	n5_i2c_write(dev, 0x25, 0xdc);
	n5_i2c_write(dev, 0x2b, 0xa8);
	n5_i2c_write(dev, 0x47, 0xee);
	n5_i2c_write(dev, 0x50, 0xc6);
	n5_i2c_write(dev, 0x69, 0x10);
	n5_i2c_write(dev, 0x7b, 0x00);
	n5_i2c_write(dev, 0xb8, 0x39);
}


void n5_set_chn_4k_12_5(unsigned char dev, unsigned char chn)
{
	n5_i2c_write(dev, 0xff, 0x00);
	n5_i2c_write(dev, 0x08+chn, 0x00);
	n5_i2c_write(dev, 0x34+chn, 0x00);
	n5_i2c_write(dev, 0x81+chn, 0x01);
	n5_i2c_write(dev, 0x85+chn, 0x08);
	n5_i2c_write(dev, 0x54, n5_i2c_read(dev, 0x54)&(~(0x10<<chn)));
	n5_i2c_write(dev, 0x18+chn, 0x1f);
	n5_i2c_write(dev, 0x58+chn, 0x7a);
	n5_i2c_write(dev, 0x5c+chn, 0x80);
	n5_i2c_write(dev, 0x64+chn, 0x00);
	n5_i2c_write(dev, 0x89+chn, 0x00);
	n5_i2c_write(dev, chn+0x8e, 0x00);
	n5_i2c_write(dev, 0x30+chn, 0x12);
	n5_i2c_write(dev, 0xa0+chn, 0x05);

	n5_i2c_write(dev, 0xff, 0x01);
	n5_i2c_write(dev, 0x84+chn, 0x04);
	n5_i2c_write(dev, 0x8c+chn, 0x44);
	n5_i2c_write(dev, 0xed, n5_i2c_read(dev, 0xed)|(0x01<<chn));

	n5_i2c_write(dev, 0xff, 0x05+chn);
	n5_i2c_write(dev, 0x20, 0x84);
	n5_i2c_write(dev, 0x25, 0xdc);
	n5_i2c_write(dev, 0x2b, 0xa8);
	n5_i2c_write(dev, 0x47, 0xee);
	n5_i2c_write(dev, 0x50, 0xc6);
	n5_i2c_write(dev, 0x69, 0x10);
	n5_i2c_write(dev, 0x7b, 0x00);
	n5_i2c_write(dev, 0xb8, 0x39);	
}

void n5_set_chnmode(unsigned char dev, unsigned char chn, unsigned char chnmode)
{
	uartlog("%s %d,chn=%d, chnmode=%d\n", __func__, __LINE__, chn, chnmode);
	switch(chnmode)
	{
		case SD_H720_NT:
			n5_set_chn_720h_ntsc(dev, chn);
		break;
		case SD_H720_PAL:
			n5_set_chn_720h_pal(dev, chn);
		break;
		case AHD_720P_30P:
			n5_set_chn_720p_30(dev, chn);
		break;
		case AHD_720P_25P:
			n5_set_chn_720p_25(dev, chn);
		break;
		case AHD_1080P_30P:
			n5_set_chn_1080p_30(dev, chn);
		break;
		case AHD_1080P_25P:
			n5_set_chn_1080p_25(dev, chn);
		break;
		case AHD_960P_30P:
			n5_set_chn_960p_30(dev, chn);
		break;
		case AHD_960P_25P:
			n5_set_chn_960p_25(dev, chn);
		break;
		case AHD_4M_30P:
			n5_set_chn_2k_30(dev, chn);
		break;
		case AHD_4M_25P:
			n5_set_chn_2k_25(dev, chn);
		break;
		case AHD_8M_15P:
			n5_set_chn_4k_15(dev, chn);
		break;
		case AHD_8M_12_5P:
			n5_set_chn_4k_12_5(dev, chn);
		break;
		default:
			n5_set_chn_1080p_25(dev, chn);
		break;
	}	
	chn_mode[chn] = chnmode;
}

//port:0 or 1
//muxmode: 1->1mux(default), 2->2mux
void n5_set_portmode(unsigned char dev, unsigned char port, unsigned char muxmode, unsigned char is_bt601)
{
	unsigned char val_1xc8, val_1xca, val_0x54;
	// add delay for MUX2
	//unsigned char clk_freq_array[4] = {0x83,0x03,0x43,0x63}; //clk_freq: 0~3:37.125M/74.25M/148.5M/297M
	unsigned char clk_freq_array[4] = {0x83,0x03,0x4f,0x63}; //clk_freq: 0~3:37.125M/74.25M/148.5M/297M
	n5_i2c_write(dev, 0xff, 0x00);
	val_0x54 = n5_i2c_read(dev, 0x54);
	if( (N5_OUTMODE_2MUX_SD == muxmode)  || (N5_OUTMODE_2MUX_HD == muxmode) || 
		(N5_OUTMODE_2MUX_FHD == muxmode) || (N5_OUTMODE_2MUX_BT1120S_720P == muxmode) ||
		(N5_OUTMODE_2MUX_BT1120S_1080P == muxmode) )
		val_0x54 |= 0x01;
	else
		val_0x54 &= 0xFE;
	n5_i2c_write(dev, 0x54, val_0x54);
	
	n5_i2c_write(dev, 0xff, 0x01);
	val_1xc8 = n5_i2c_read(dev, 0xc8);
	switch(muxmode)
	{
		case N5_OUTMODE_1MUX_SD:
			n5_i2c_write(dev, 0xA0+port, 0x00);
			n5_i2c_write(dev, 0xC0, 0x00);
			n5_i2c_write(dev, 0xC2, 0x11);
			val_1xc8 &= (1==port?0x0F:0xF0);
			n5_i2c_write(dev, 0xC8, val_1xc8);
			n5_i2c_write(dev, 0xCC+port, clk_freq_array[0]);
		break;	
		case N5_OUTMODE_1MUX_HD:
			n5_i2c_write(dev, 0xA0+port, 0x00);
			n5_i2c_write(dev, 0xC0, 0x00);
			n5_i2c_write(dev, 0xC2, 0x11);
			val_1xc8 &= (1==port?0x0F:0xF0);
			n5_i2c_write(dev, 0xC8, val_1xc8);
			n5_i2c_write(dev, 0xCC+port, clk_freq_array[1]);
		break;
		case N5_OUTMODE_1MUX_FHD:
			n5_i2c_write(dev, 0xA0+port, 0x00);
			n5_i2c_write(dev, 0xC0, 0x00);
			n5_i2c_write(dev, 0xC2, 0x11);
			val_1xc8 &= (1==port?0x0F:0xF0);
			n5_i2c_write(dev, 0xC8, val_1xc8);
			n5_i2c_write(dev, 0xCC+port, clk_freq_array[2]);
		break;
		case N5_OUTMODE_1MUX_FHD_HALF:
			n5_i2c_write(dev, 0xA0+port, 0x00);
			n5_i2c_write(dev, 0xC0, 0x88);
			n5_i2c_write(dev, 0xC2, 0x99);
			val_1xc8 &= (1==port?0x0F:0xF0);
			n5_i2c_write(dev, 0xC8, val_1xc8);
			n5_i2c_write(dev, 0xCC+port, clk_freq_array[1]);
		break;
		case N5_OUTMODE_2MUX_SD:
			n5_i2c_write(dev, 0xA0+port, 0x20);
			n5_i2c_write(dev, 0xC0, 0x10);
			n5_i2c_write(dev, 0xC2, 0x10);
			val_1xc8 &= (1==port?0x0F:0xF0);
			val_1xc8 |= (1==port?0x20:0x02);
			n5_i2c_write(dev, 0xC8, val_1xc8);
			n5_i2c_write(dev, 0xCC+port, clk_freq_array[1]);
		break;
		case N5_OUTMODE_2MUX_HD:
			n5_i2c_write(dev, 0xA0+port, 0x20);
			n5_i2c_write(dev, 0xC0, 0x10);
			n5_i2c_write(dev, 0xC2, 0x10);
			val_1xc8 &= (1==port?0x0F:0xF0);
			val_1xc8 |= (1==port?0x20:0x02);
			n5_i2c_write(dev, 0xC8, val_1xc8);
			n5_i2c_write(dev, 0xCC+port, clk_freq_array[2]);
		break;
		case N5_OUTMODE_2MUX_FHD:
			n5_i2c_write(dev, 0xA0+port, 0x00);
			n5_i2c_write(dev, 0xC0, 0x10);
			n5_i2c_write(dev, 0xC2, 0x10);
			val_1xc8 &= (1==port?0x0F:0xF0);
			val_1xc8 |= (1==port?0x20:0x02);
			n5_i2c_write(dev, 0xC8, val_1xc8);
			n5_i2c_write(dev, 0xCC+port, clk_freq_array[2]);
		break;
		case N5_OUTMODE_1MUX_BT1120S:
			n5_i2c_write(dev, 0xA0, 0x00);
			n5_i2c_write(dev, 0xA1, 0x00);
			n5_i2c_write(dev, 0xC0, 0xCC);
			n5_i2c_write(dev, 0xC1, 0xCC);
			n5_i2c_write(dev, 0xC2, 0x44);
			n5_i2c_write(dev, 0xC3, 0x44);
			n5_i2c_write(dev, 0xC8, 0x00);
			n5_i2c_write(dev, 0xCA, 0x33); //two ports are enabled
			n5_i2c_write(dev, 0xCC, clk_freq_array[2]);
		break;
		case N5_OUTMODE_2MUX_BT1120S_720P:
			n5_i2c_write(dev, 0xA0, 0x00);
			n5_i2c_write(dev, 0xA1, 0x00);
			n5_i2c_write(dev, 0xC0, 0xDC);  //C data
			n5_i2c_write(dev, 0xC1, 0xDC);
			n5_i2c_write(dev, 0xC2, 0x54); //Y data
			n5_i2c_write(dev, 0xC3, 0x54);
			n5_i2c_write(dev, 0xC8, 0x22);
			n5_i2c_write(dev, 0xCA, 0x33); //two ports are enabled
			n5_i2c_write(dev, 0xCC, clk_freq_array[1]);
		break;
		case N5_OUTMODE_2MUX_BT1120S_1080P:
			n5_i2c_write(dev, 0xA0, 0x20);
			n5_i2c_write(dev, 0xA1, 0x20);
			n5_i2c_write(dev, 0xC0, 0xDC);  //C data
			n5_i2c_write(dev, 0xC1, 0xDC);
			n5_i2c_write(dev, 0xC2, 0x54); //Y data
			n5_i2c_write(dev, 0xC3, 0x54);
			n5_i2c_write(dev, 0xC8, 0x22);
			n5_i2c_write(dev, 0xCA, 0x33); //two ports are enabled
			n5_i2c_write(dev, 0xCC, clk_freq_array[1]);
		break;
	}
	if(1 == is_bt601)
	{
		n5_i2c_write(dev, 0xA8+port, 0x90+(port*0x10));	//h/v0 sync enabled
		//n5_i2c_write(dev, 0xA9, 0xA0); 	//h/v1 sync enabled
		//n5_i2c_write(dev, 0xBC, 0x10);	//h/v0 swap enabled
		//n5_i2c_write(dev, 0xBD, 0x10); 
		//n5_i2c_write(dev, 0xBE, 0x10);	//h/v1 swap enabled
		//n5_i2c_write(dev, 0xBF, 0x10); 
	}
	else
	{
		n5_i2c_write(dev, 0xA8, 0x00);
		//n5_i2c_write(dev, 0xA9, 0x00);  //h/v sync disable.
	}
	
	if(N5_OUTMODE_2MUX_BT1120S_720P == muxmode)
	{
		n5_i2c_write(dev, 0xE4, 0x11);
		n5_i2c_write(dev, 0xE5, 0x11);
	}
	else
	{
		n5_i2c_write(dev, 0xE4, 0x00);
		n5_i2c_write(dev, 0xE5, 0x00);
	}	
	
	val_1xca = n5_i2c_read(dev, 0xca);
	val_1xca |= (0x11<<port); //enable port
	n5_i2c_write(dev, 0xCA, val_1xca);
}

int n5_poweron(void *priv)
{
	pattern_ctx *ctx = (pattern_ctx *)priv;
	struct sensor_info *sinf = (struct sensor_info *)ctx->ptn->priv;
	struct snr_i2c_adapter *adpt = NULL;
	struct sensor_mode *mode = &n5_mode[N5_MODE_720P30];
	char cmdbuf[8];
	uint32_t tmp;
	unsigned int test_mode;
	unsigned int mux;

	disable_irq(CSIMAC0_INTR_NUM);

	// i2c config here
	snr_bm_i2c_get_adap((void *)&adpt);
	sinf->adpt = adpt;

	adpt->slaveaddr = sinf->dev;
	adpt->speed = 400000;
	snr_bm_i2c_init(adpt);

	// start rx init.
	n5_common_setting(sinf->dev);

	bm_get_num("single(0), 2mux(1)", mux);

	if (mux) {
		n5_set_chnmode(sinf->dev, 0, chn0);
		n5_set_chnmode(sinf->dev, 1, chn1);
		n5_set_portmode(sinf->dev, 0, N5_OUTMODE_2MUX_HD, bt601);
	} else {
		n5_set_chnmode(sinf->dev, 0, chn0);
		//n5_set_chnmode(sinf->dev, 1, chn1);
		n5_set_portmode(sinf->dev, 0, N5_OUTMODE_1MUX_HD, bt601);
	}

	bm_get_num("normal(0), test_mode(1)", test_mode);
	if (test_mode) {
		n5_i2c_write(sinf->dev, 0xFF, 0x5);
		n5_i2c_write(sinf->dev, 0x2c, 0x8);
		n5_i2c_write(sinf->dev, 0xFF, 0x6);
		n5_i2c_write(sinf->dev, 0x2c, 0x8);
		n5_i2c_write(sinf->dev, 0xFF, 0x7);
		n5_i2c_write(sinf->dev, 0x2c, 0x8);
		n5_i2c_write(sinf->dev, 0xFF, 0x8);
		n5_i2c_write(sinf->dev, 0x2c, 0x8);
		n5_i2c_write(sinf->dev, 0xFF, 0x0);
		n5_i2c_write(sinf->dev, 0x78, 0xaa);
		n5_i2c_write(sinf->dev, 0x79, 0xaa);
	}

	// [TODO] Switch the MAC type to TTL.
	writel(0x0a0c2000, (readl(0x0a0c2000) & ~0x7)
			| 0x00000004);
	if (mux) {
		writel(0x0a0c2080, 0x00010002);
		writel(0x0a0c2090, 0x00000f0f);
		writel(0x0a0c2094, 0x000000ff);
		writel(0x0a0c2098, 0xb090a080);
		writel(0x0a0c209c, 0xb191a181);
		writel(0x0a0c20a0, 0xb292a282);
		writel(0x0a0c20a4, 0xb393a383);
		writel(0x0a0c20a8, 0x0000000f);
	} else {
		writel(0x0a0c2080, 0x00000002);
		writel(0x0a0c2090, 0x00000f0f);
		writel(0x0a0c2094, 0x000000ff);
		writel(0x0a0c2098, 0xb69dab80);
		writel(0x0a0c20a8, 0x0000000f);
	}
	sinf->ishdr = false;

	uartlog("streaming start\n");

	cmdbuf[0] = cmdbuf[1] = '0';
	cli_readline("run sensor to dram flow? [n/y]", cmdbuf);
	if (cmdbuf[0] == 'y') {
		uartlog("dump %d x %d \n", mode->total_w, mode->total_h);
		config_csibdg(0, mode->total_w, mode->total_h, ctx,
			      (mux==0) ? 0:3, 1);
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

int peri_up_n5(void *priv)
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

	cmdbuf[0] = '0';
	cli_readline("### enter to start power-on seqeuence. s) to skip : ", cmdbuf);
	if (cmdbuf[0] == 's')
		return 0;

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

#if !defined(FLOW_AFTER_ECO)
	uartlog("clear reset!\n");
	fpga_pwr_reg = (unsigned int *)HARD_REG_FPGA_PWRPIN;
	// clear reset and enable phy rx
	writel(fpga_pwr_reg, 0x32100000);
#endif
	uartlog("### device poweron\n");

	return 0;
}

int peri_down_n5(void *priv)
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

