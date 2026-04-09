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

#define _MIPI_PORT0_
//#define _MIPI_PORT1_


#ifdef _MIPI_PORT0_
#define _MAR_BANK_ 0x20
#define _MTX_BANK_ 0x23
#else
#define _MAR_BANK_ 0x30
#define _MTX_BANK_ 0x33
#endif

#define CH0_NO		0
#define CH1_NO		1
#define NTPAL		0

#define VFMT_NTSC 		0
#define VFMT_PAL 		1

#undef uartlog
#define uartlog(fmt, args...) \
{ \
    printf(fmt, ##args); \
    fflush(stdout); \
}
static unsigned int adet=0;
static unsigned int fmt = 1; //0:960h 1:720P 2:1080P
//static unsigned int ntpal = 0; //0:ntsc/30p 1:pal/25p
static unsigned int mclk = 1; //0:1458 1:756

#define CIF_DPHY_ENABLE

static uint32_t	use_4L = 1;

struct sensor_info n6_info = {
	.dev = 0x31,
	.alen = 1,
	.dlen = 1,
	.gain =  { .cur = 0, .min = 0, .max = 0xf0 },
	.sgain =  { .cur = 0, .min = 0, .max = 0xf0 },
	.shsl =  { .cur = 0, .min = 0, .max = 0 },
	.shss =  { .cur = 0, .min = 0, .max = 0 },
	.ops = &n6_ops,
};

// sensor mode for n6
struct sensor_mode n6_mode[] = {
	[N6_MODE_720P30] =
	{
		.name = "720p30",
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

static void n6_get_gain(void *priv, struct sensor_prop *prop)
{
	return;
}

static void n6_set_shsl(void *priv, uint32_t shs);

static void n6_set_gain(void *priv, uint16_t gain)
{
	return;
}

static void n6_get_sgain(void *priv, struct sensor_prop *prop)
{
	return;
}

static void n6_set_sgain(void *priv, uint16_t gain)
{
	return;
}

static void n6_get_shsl(void *priv, struct sensor_prop *prop)
{
	return;
}

static void n6_set_shsl(void *priv, uint32_t shs)
{
	return;
}

static void n6_get_shss(void *priv, struct sensor_prop *prop)
{
	return;
}

static void n6_set_shss(void *priv, uint32_t shs)
{
	return;
}

static void n6_test_mode(void *priv, uint32_t value)
{
	return;
}

struct sensor_ops n6_ops = {
	.get_gain = n6_get_gain,
	.set_gain = n6_set_gain,
	.get_sgain = n6_get_sgain,
	.set_sgain = n6_set_sgain,
	.get_shsl = n6_get_shsl,
	.set_shsl = n6_set_shsl,
	.get_shss = n6_get_shss,
	.set_shss = n6_set_shss,
	.test_mode = n6_test_mode,
};

static void n6_i2c_write(uint8_t dev, uint16_t reg, uint16_t data)
{
	struct sensor_info *sinf = &n6_info;
	uint16_t tmp = data;
	int r;

	if (!sinf->adpt)
		return;

	r = sinf->adpt->write(sinf->adpt, dev, reg, sinf->alen, (uint8_t*)&tmp, 1);
	if (r < 0)
		uartlog("i2c_write fail addr[0x%x]\n", reg);
}

static uint8_t n6_i2c_read(uint8_t dev, uint16_t reg)
{
	struct sensor_info *sinf = &n6_info;
	uint16_t tmp;
	int r;

	if (!sinf->adpt)
		return 0;

	r = sinf->adpt->read(sinf->adpt, dev, reg, sinf->alen, (uint8_t*)&tmp, 1);
	if (r < 0)
		uartlog("i2c_write fail addr[0x%x]\n", reg);

	return (uint8_t)tmp;
}

int n6_intr(void *priv)
{
	return 0;
}

void n6_common_setting(unsigned char dev)
{
	unsigned char ch;
	uartlog("[%s::%d]\n", __func__, __LINE__);
	n6_i2c_write(dev, 0xff, 0x00);
	n6_i2c_write(dev, 0x80, 0x0f); 
	n6_i2c_write(dev, 0x00, 0x10);
	n6_i2c_write(dev, 0x01, 0x10);
	n6_i2c_write(dev, 0x02, 0x10);
	n6_i2c_write(dev, 0x03, 0x10);
	n6_i2c_write(dev, 0x22, 0x0b);
	n6_i2c_write(dev, 0x23, 0x41);
	n6_i2c_write(dev, 0x26, 0x0b);
	n6_i2c_write(dev, 0x27, 0x41);
	n6_i2c_write(dev, 0x2a, 0x0b);
	n6_i2c_write(dev, 0x2b, 0x41);
	n6_i2c_write(dev, 0x2e, 0x0b);
	n6_i2c_write(dev, 0x2f, 0x41);

	n6_i2c_write(dev, 0xff, 0x01);
	n6_i2c_write(dev, 0x98, 0x30);
	n6_i2c_write(dev, 0xed, 0x00);

	for(ch=0;ch<4;ch++)
	{
		n6_i2c_write(dev, 0xff, 0x05+ch);
		n6_i2c_write(dev, 0x00, 0xd0);
		n6_i2c_write(dev, 0x01, 0x22);
		n6_i2c_write(dev, 0x47, 0xee);
		n6_i2c_write(dev, 0x50, 0xc6);
		n6_i2c_write(dev, 0x57, 0x00);
		n6_i2c_write(dev, 0x58, 0x77);
		n6_i2c_write(dev, 0x5b, 0x41);
		n6_i2c_write(dev, 0x5c, 0x78);
		n6_i2c_write(dev, 0xB8, 0xB8);
	}
	

	n6_i2c_write(dev, 0xff, 0x09);
	n6_i2c_write(dev, 0x50, 0x30);
	n6_i2c_write(dev, 0x51, 0x6f);
	n6_i2c_write(dev, 0x52, 0x67);
	n6_i2c_write(dev, 0x53, 0x48);
	n6_i2c_write(dev, 0x54, 0x30);
	n6_i2c_write(dev, 0x55, 0x6f);
	n6_i2c_write(dev, 0x56, 0x67);
	n6_i2c_write(dev, 0x57, 0x48);
	n6_i2c_write(dev, 0x58, 0x30);
	n6_i2c_write(dev, 0x59, 0x6f);
	n6_i2c_write(dev, 0x5a, 0x67);
	n6_i2c_write(dev, 0x5b, 0x48);
	n6_i2c_write(dev, 0x5c, 0x30);
	n6_i2c_write(dev, 0x5d, 0x6f);
	n6_i2c_write(dev, 0x5e, 0x67);
	n6_i2c_write(dev, 0x5f, 0x48);
	
	n6_i2c_write(dev, 0xff, 0x0a);
	n6_i2c_write(dev, 0x25, 0x10);
	n6_i2c_write(dev, 0x27, 0x1e);
	n6_i2c_write(dev, 0x30, 0xac);
	n6_i2c_write(dev, 0x31, 0x78);
	n6_i2c_write(dev, 0x32, 0x17);
	n6_i2c_write(dev, 0x33, 0xc1);
	n6_i2c_write(dev, 0x34, 0x40);
	n6_i2c_write(dev, 0x35, 0x00);
	n6_i2c_write(dev, 0x36, 0xc3);
	n6_i2c_write(dev, 0x37, 0x0a);
	n6_i2c_write(dev, 0x38, 0x00);
	n6_i2c_write(dev, 0x39, 0x02);
	n6_i2c_write(dev, 0x3a, 0x00);
	n6_i2c_write(dev, 0x3b, 0xb2);
	n6_i2c_write(dev, 0xa5, 0x10);
	n6_i2c_write(dev, 0xa7, 0x1e);
	n6_i2c_write(dev, 0xb0, 0xac);
	n6_i2c_write(dev, 0xb1, 0x78);
	n6_i2c_write(dev, 0xb2, 0x17);
	n6_i2c_write(dev, 0xb3, 0xc1);
	n6_i2c_write(dev, 0xb4, 0x40);
	n6_i2c_write(dev, 0xb5, 0x00);
	n6_i2c_write(dev, 0xb6, 0xc3);
	n6_i2c_write(dev, 0xb7, 0x0a);
	n6_i2c_write(dev, 0xb8, 0x00);
	n6_i2c_write(dev, 0xb9, 0x02);
	n6_i2c_write(dev, 0xba, 0x00);
	n6_i2c_write(dev, 0xbb, 0xb2);
	n6_i2c_write(dev, 0xff, 0x0b);
	n6_i2c_write(dev, 0x25, 0x10);
	n6_i2c_write(dev, 0x27, 0x1e);
	n6_i2c_write(dev, 0x30, 0xac);
	n6_i2c_write(dev, 0x31, 0x78);
	n6_i2c_write(dev, 0x32, 0x17);
	n6_i2c_write(dev, 0x33, 0xc1);
	n6_i2c_write(dev, 0x34, 0x40);
	n6_i2c_write(dev, 0x35, 0x00);
	n6_i2c_write(dev, 0x36, 0xc3);
	n6_i2c_write(dev, 0x37, 0x0a);
	n6_i2c_write(dev, 0x38, 0x00);
	n6_i2c_write(dev, 0x39, 0x02);
	n6_i2c_write(dev, 0x3a, 0x00);
	n6_i2c_write(dev, 0x3b, 0xb2);
	n6_i2c_write(dev, 0xa5, 0x10);
	n6_i2c_write(dev, 0xa7, 0x1e);
	n6_i2c_write(dev, 0xb0, 0xac);
	n6_i2c_write(dev, 0xb1, 0x78);
	n6_i2c_write(dev, 0xb2, 0x17);
	n6_i2c_write(dev, 0xb3, 0xc1);
	n6_i2c_write(dev, 0xb4, 0x40);
	n6_i2c_write(dev, 0xb5, 0x00);
	n6_i2c_write(dev, 0xb6, 0xc3);
	n6_i2c_write(dev, 0xb7, 0x0a);
	n6_i2c_write(dev, 0xb8, 0x00);
	n6_i2c_write(dev, 0xb9, 0x02);
	n6_i2c_write(dev, 0xba, 0x00);
	n6_i2c_write(dev, 0xbb, 0xb2);

	n6_i2c_write(dev, 0xff, 0x13);
	n6_i2c_write(dev, 0x05, 0xa0);
	n6_i2c_write(dev, 0x31, 0xff);
	n6_i2c_write(dev, 0x07, 0x47);
	n6_i2c_write(dev, 0x12, 0x04);
	n6_i2c_write(dev, 0x1e, 0x1f);
	n6_i2c_write(dev, 0x1f, 0x27);
	n6_i2c_write(dev, 0x2e, 0x10);
	n6_i2c_write(dev, 0x2f, 0xc8);
	n6_i2c_write(dev, 0x31, 0xff);
	n6_i2c_write(dev, 0x32, 0x00);
	n6_i2c_write(dev, 0x33, 0x00);
	n6_i2c_write(dev, 0x72, 0x05);
	n6_i2c_write(dev, 0x7a, 0xf0);
	
	n6_i2c_write(dev, 0xff, _MAR_BANK_);
	n6_i2c_write(dev, 0x10, 0xff);
	n6_i2c_write(dev, 0x11, 0xff);
	if(1 == mclk)
	{
	n6_i2c_write(dev, 0x30, 0x0f);
	n6_i2c_write(dev, 0x32, 0x92);
	n6_i2c_write(dev, 0x34, 0xcd);
	n6_i2c_write(dev, 0x36, 0x04);
	n6_i2c_write(dev, 0x38, 0x58);
	}
	else
	{
	n6_i2c_write(dev, 0x30, 0x0f);
	n6_i2c_write(dev, 0x32, 0xff);
	n6_i2c_write(dev, 0x34, 0xcd);
	n6_i2c_write(dev, 0x36, 0x04);
	n6_i2c_write(dev, 0x38, 0xff);
	}
	n6_i2c_write(dev, 0x3c, 0x01);
	n6_i2c_write(dev, 0x3d, 0x11);
	n6_i2c_write(dev, 0x3e, 0x11);
	n6_i2c_write(dev, 0x45, 0x60);
	n6_i2c_write(dev, 0x46, 0x49);

	n6_i2c_write(dev, 0xff, _MTX_BANK_);
	n6_i2c_write(dev, 0xe9, 0x03);
	n6_i2c_write(dev, 0x03, 0x02);
	n6_i2c_write(dev, 0x01, 0xe4);
	n6_i2c_write(dev, 0x00, 0x7d);
	n6_i2c_write(dev, 0x01, 0xe0);
	n6_i2c_write(dev, 0x02, 0xa0);
	n6_i2c_write(dev, 0x20, 0x1e);
	n6_i2c_write(dev, 0x20, 0x1f);
	if(1 == mclk)
	{
	n6_i2c_write(dev, 0x04, 0x38);
	n6_i2c_write(dev, 0x45, 0xc4);
	n6_i2c_write(dev, 0x46, 0x01);
	n6_i2c_write(dev, 0x47, 0x1b);
	n6_i2c_write(dev, 0x48, 0x08);
	n6_i2c_write(dev, 0x65, 0xc4);
	n6_i2c_write(dev, 0x66, 0x01);
	n6_i2c_write(dev, 0x67, 0x1b);
	n6_i2c_write(dev, 0x68, 0x08);
	n6_i2c_write(dev, 0x85, 0xc4);
	n6_i2c_write(dev, 0x86, 0x01);
	n6_i2c_write(dev, 0x87, 0x1b);
	n6_i2c_write(dev, 0x88, 0x08);
	n6_i2c_write(dev, 0xa5, 0xc4);
	n6_i2c_write(dev, 0xa6, 0x01);
	n6_i2c_write(dev, 0xa7, 0x1b);
	n6_i2c_write(dev, 0xa8, 0x08);
	n6_i2c_write(dev, 0xc5, 0xc4);
	n6_i2c_write(dev, 0xc6, 0x01);	
	n6_i2c_write(dev, 0xc7, 0x1b);	
	n6_i2c_write(dev, 0xc8, 0x08);
	}
	else
	{
	n6_i2c_write(dev, 0x04, 0x6c);
	n6_i2c_write(dev, 0x45, 0xcd);
	n6_i2c_write(dev, 0x46, 0x42);
	n6_i2c_write(dev, 0x47, 0x36);
	n6_i2c_write(dev, 0x48, 0x0f);
	n6_i2c_write(dev, 0x65, 0xcd);
	n6_i2c_write(dev, 0x66, 0x42);
	n6_i2c_write(dev, 0x67, 0x0e);
	n6_i2c_write(dev, 0x68, 0x0f);
	n6_i2c_write(dev, 0x85, 0xcd);
	n6_i2c_write(dev, 0x86, 0x42);
	n6_i2c_write(dev, 0x87, 0x0e);
	n6_i2c_write(dev, 0x88, 0x0f);
	n6_i2c_write(dev, 0xa5, 0xcd);
	n6_i2c_write(dev, 0xa6, 0x42);
	n6_i2c_write(dev, 0xa7, 0x0e);
	n6_i2c_write(dev, 0xa8, 0x0f);
	n6_i2c_write(dev, 0xc5, 0xcd);
	n6_i2c_write(dev, 0xc6, 0x42);	
	n6_i2c_write(dev, 0xc7, 0x0e);	
	n6_i2c_write(dev, 0xc8, 0x0f);
	}
	n6_i2c_write(dev, 0xeb, 0x8d);

	n6_i2c_write(dev, 0xff, _MAR_BANK_);
	n6_i2c_write(dev, 0x00, 0xff);
	n6_i2c_write(dev, 0x40, 0x01);
	n6_i2c_write(dev, 0x40, 0x00);
	n6_i2c_write(dev, 0xff, 0x01);
	n6_i2c_write(dev, 0x97, 0x00);
	n6_i2c_write(dev, 0x97, 0x0f);

	n6_i2c_write(dev, 0xff, 0x00);  //test pattern
	n6_i2c_write(dev, 0x78, 0xba);
	n6_i2c_write(dev, 0x79, 0xac);
	n6_i2c_write(dev, 0xff, 0x05);
	n6_i2c_write(dev, 0x2c, 0x08);
	n6_i2c_write(dev, 0x6a, 0x80);
	n6_i2c_write(dev, 0xff, 0x06);
	n6_i2c_write(dev, 0x2c, 0x08);
	n6_i2c_write(dev, 0x6a, 0x80);
	n6_i2c_write(dev, 0xff, 0x07);
	n6_i2c_write(dev, 0x2c, 0x08);
	n6_i2c_write(dev, 0x6a, 0x80);
	n6_i2c_write(dev, 0xff, 0x08);
	n6_i2c_write(dev, 0x2c, 0x08);
	n6_i2c_write(dev, 0x6a, 0x80);
}

void n6_set_chn_960h(unsigned char dev, unsigned char ch, unsigned char ntpal)
{
	unsigned char val_0x54, val_20x01;
	uartlog("[%s::%d]\n", __func__, __LINE__);
	n6_i2c_write(dev, 0xff, 0x00);
	n6_i2c_write(dev, 0x08+ch, ntpal?0xdd:0xa0);
	n6_i2c_write(dev, 0x18+ch, 0x08);
	n6_i2c_write(dev, 0x22+ch*4, 0x0b);
	n6_i2c_write(dev, 0x23+ch*4, 0x41);
	n6_i2c_write(dev, 0x30+ch, 0x12);
	n6_i2c_write(dev, 0x34+ch, 0x01);
	val_0x54 = n6_i2c_read(dev, 0x54);
	if(ntpal)
		val_0x54 &= ~(0x10<<ch);
	else
		val_0x54 |= (0x10<<ch);
	n6_i2c_write(dev, 0x54, val_0x54);
	n6_i2c_write(dev, 0x58+ch, ntpal?0x80:0x90);
	n6_i2c_write(dev, 0x5c+ch, ntpal?0xbe:0xbc);
	n6_i2c_write(dev, 0x64+ch, ntpal?0xa0:0x81);
	n6_i2c_write(dev, 0x81+ch, ntpal?0xf0:0xe0);
	n6_i2c_write(dev, 0x85+ch, 0x00);
	n6_i2c_write(dev, 0x89+ch, 0x00);
	n6_i2c_write(dev, ch+0x8e, 0x00);
	n6_i2c_write(dev, 0xa0+ch, 0x05);

	n6_i2c_write(dev, 0xff, 0x01);
	n6_i2c_write(dev, 0x84+ch, 0x02);
	n6_i2c_write(dev, 0x88+ch, 0x00);
	n6_i2c_write(dev, 0x8c+ch, 0x40);
	n6_i2c_write(dev, 0xa0+ch, 0x20);
	n6_i2c_write(dev, 0xed, 0x00);

	n6_i2c_write(dev, 0xff, 0x05+ch);
	n6_i2c_write(dev, 0x01, 0x22);
	n6_i2c_write(dev, 0x05, 0x00);
	n6_i2c_write(dev, 0x08, 0x55);
	n6_i2c_write(dev, 0x25, 0xdc);
	n6_i2c_write(dev, 0x28, 0x80);
	n6_i2c_write(dev, 0x2f, 0x00);
	n6_i2c_write(dev, 0x30, 0xe0);
	n6_i2c_write(dev, 0x31, 0x43);
	n6_i2c_write(dev, 0x32, 0xa2);
	n6_i2c_write(dev, 0x47, 0x04);
	n6_i2c_write(dev, 0x50, 0x84);
	n6_i2c_write(dev, 0x57, 0x00);
	n6_i2c_write(dev, 0x58, 0x77);
	n6_i2c_write(dev, 0x5b, 0x43);
	n6_i2c_write(dev, 0x5c, 0x78);
	n6_i2c_write(dev, 0x5f, 0x00);
	n6_i2c_write(dev, 0x62, 0x20);
	n6_i2c_write(dev, 0x7b, 0x00);
	n6_i2c_write(dev, 0x7c, 0x01);
	n6_i2c_write(dev, 0x7d, 0x80);
	n6_i2c_write(dev, 0x80, 0x00);
	n6_i2c_write(dev, 0x90, 0x01);
	n6_i2c_write(dev, 0xa9, 0x00);
	n6_i2c_write(dev, 0xb5, 0x00);
	n6_i2c_write(dev, 0xb8, 0xb9);
	n6_i2c_write(dev, 0xb9, 0x72);
	n6_i2c_write(dev, 0xd1, 0x00);
	n6_i2c_write(dev, 0xd5, 0x80);

	n6_i2c_write(dev, 0xff, 0x09);
	n6_i2c_write(dev, 0x96+ch*0x20, 0x10);
	n6_i2c_write(dev, 0x98+ch*0x20, ntpal?0xc0:0xe0);
	n6_i2c_write(dev, ch*0x20+0x9e, 0x00);
	
	n6_i2c_write(dev, 0xff, _MAR_BANK_);
	val_20x01 = n6_i2c_read(dev, 0x01);
	val_20x01 &=(~(0x03<<(ch*2)));
	val_20x01 |=(0x02<<(ch*2));
	n6_i2c_write(dev, 0x01, val_20x01);
	n6_i2c_write(dev, 0x12+ch*2, 0xe0);
	n6_i2c_write(dev, 0x13+ch*2, 0x01);
	
}

void n6_set_chn_720p(unsigned char dev, unsigned char ch, unsigned char ntpal)
{
	unsigned char val_0x54, val_20x01;
	uartlog("[%s::%d]\n", __func__, __LINE__);
	n6_i2c_write(dev, 0xff, 0x00);
	n6_i2c_write(dev, 0x08+ch, 0x00);
	n6_i2c_write(dev, 0x18+ch, 0x3f);
	n6_i2c_write(dev, 0x30+ch, 0x12);
	n6_i2c_write(dev, 0x34+ch, 0x00);
	val_0x54 = n6_i2c_read(dev, 0x54);
	val_0x54 &= ~(0x10<<ch);
	n6_i2c_write(dev, 0x54, val_0x54);
	n6_i2c_write(dev, 0x58+ch, ntpal?0x80:0x80);
	n6_i2c_write(dev, 0x5c+ch, ntpal?0x00:0x00);
	n6_i2c_write(dev, 0x64+ch, ntpal?0x01:0x01);
	n6_i2c_write(dev, 0x81+ch, ntpal?0x0d:0x0c);
	n6_i2c_write(dev, 0x85+ch, 0x00);
	n6_i2c_write(dev, 0x89+ch, 0x00);
	n6_i2c_write(dev, ch+0x8e, 0x00);
	n6_i2c_write(dev, 0xa0+ch, 0x05);

	n6_i2c_write(dev, 0xff, 0x01);
	n6_i2c_write(dev, 0x84+ch, 0x02);
	n6_i2c_write(dev, 0x88+ch, 0x00);
	n6_i2c_write(dev, 0x8c+ch, 0x40);
	n6_i2c_write(dev, 0xa0+ch, 0x20);

	n6_i2c_write(dev, 0xff, 0x05+ch);
	n6_i2c_write(dev, 0x01, 0x22);
	n6_i2c_write(dev, 0x05, 0x04);
	n6_i2c_write(dev, 0x08, 0x55);
	n6_i2c_write(dev, 0x25, 0xdc);
	n6_i2c_write(dev, 0x28, 0x80);
	n6_i2c_write(dev, 0x2f, 0x00);
	n6_i2c_write(dev, 0x30, 0xe0);
	n6_i2c_write(dev, 0x31, 0x43);
	n6_i2c_write(dev, 0x32, 0xa2);
	n6_i2c_write(dev, 0x47, 0xee);
	n6_i2c_write(dev, 0x50, 0xc6);
	n6_i2c_write(dev, 0x57, 0x00);
	n6_i2c_write(dev, 0x58, 0x77);
	n6_i2c_write(dev, 0x5b, 0x41);
	n6_i2c_write(dev, 0x5c, 0x7C);
	n6_i2c_write(dev, 0x5f, 0x00);
	n6_i2c_write(dev, 0x62, 0x20);
	n6_i2c_write(dev, 0x7b, 0x11);
	n6_i2c_write(dev, 0x7c, 0x01);
	n6_i2c_write(dev, 0x7d, 0x80);
	n6_i2c_write(dev, 0x80, 0x00);
	n6_i2c_write(dev, 0x90, 0x01);
	n6_i2c_write(dev, 0xa9, 0x00);
	n6_i2c_write(dev, 0xb5, 0x40);
	n6_i2c_write(dev, 0xb8, 0x39);
	n6_i2c_write(dev, 0xb9, 0x72);
	n6_i2c_write(dev, 0xd1, 0x00);
	n6_i2c_write(dev, 0xd5, 0x80);

	n6_i2c_write(dev, 0xff, 0x09);
	n6_i2c_write(dev, 0x96+ch*0x20, 0x00);
	n6_i2c_write(dev, 0x98+ch*0x20, 0x00);
	n6_i2c_write(dev, ch*0x20+0x9e, 0x00);
	
	n6_i2c_write(dev, 0xff, _MAR_BANK_);
	val_20x01 = n6_i2c_read(dev, 0x01);
	val_20x01 &=(~(0x03<<(ch*2)));
	val_20x01 |=(0x01<<(ch*2));
	n6_i2c_write(dev, 0x01, val_20x01);
	n6_i2c_write(dev, 0x12+ch*2, 0x80);
	n6_i2c_write(dev, 0x13+ch*2, 0x02);
	
}

/*
1280x960p
dev:0x60 / 0x62 / 0x64 / 0x66
ch : 0 ~ 3
ntpal: 1:25p, 0:30p
*/
void n6_set_chn_960p(unsigned char dev, unsigned char ch, unsigned char ntpal)
{
	unsigned char val_0x54, val_20x01;
	uartlog("[%s::%d]\n", __func__, __LINE__);
	n6_i2c_write(dev, 0xff, 0x00);
	n6_i2c_write(dev, 0x08+ch, 0x00);
	n6_i2c_write(dev, 0x18+ch, 0x0f);
	n6_i2c_write(dev, 0x30+ch, 0x12);
	n6_i2c_write(dev, 0x34+ch, 0x00);
	val_0x54 = n6_i2c_read(dev, 0x54);
	val_0x54 &= ~(0x10<<ch);
	n6_i2c_write(dev, 0x54, val_0x54);
	n6_i2c_write(dev, 0x58+ch, ntpal?0x40:0x48);
	n6_i2c_write(dev, 0x5c+ch, ntpal?0x80:0x80);
	n6_i2c_write(dev, 0x64+ch, ntpal?0x28:0x28);
	n6_i2c_write(dev, 0x81+ch, ntpal?0x07:0x06);
	n6_i2c_write(dev, 0x85+ch, 0x0b);
	n6_i2c_write(dev, 0x89+ch, 0x00);
	n6_i2c_write(dev, ch+0x8e, 0x00);
	n6_i2c_write(dev, 0xa0+ch, 0x05);

	n6_i2c_write(dev, 0xff, 0x01);
	n6_i2c_write(dev, 0x84+ch, 0x02);
	n6_i2c_write(dev, 0x88+ch, 0x00);
	n6_i2c_write(dev, 0x8c+ch, 0x40);
	n6_i2c_write(dev, 0xa0+ch, 0x20);

	n6_i2c_write(dev, 0xff, 0x05+ch);
	n6_i2c_write(dev, 0x01, 0x22);
	n6_i2c_write(dev, 0x05, 0x04);
	n6_i2c_write(dev, 0x08, 0x55);
	n6_i2c_write(dev, 0x25, 0xdc);
	n6_i2c_write(dev, 0x28, 0x80);
	n6_i2c_write(dev, 0x2f, 0x00);
	n6_i2c_write(dev, 0x30, 0xe0);
	n6_i2c_write(dev, 0x31, 0x43);
	n6_i2c_write(dev, 0x32, 0xa2);
	n6_i2c_write(dev, 0x47, 0xee);
	n6_i2c_write(dev, 0x50, 0xc6);
	n6_i2c_write(dev, 0x57, 0x00);
	n6_i2c_write(dev, 0x58, 0x77);
	n6_i2c_write(dev, 0x5b, 0x41);
	n6_i2c_write(dev, 0x5c, 0x7C);
	n6_i2c_write(dev, 0x5f, 0x00);
	n6_i2c_write(dev, 0x62, 0x20);
	n6_i2c_write(dev, 0x7b, 0x11);
	n6_i2c_write(dev, 0x7c, 0x01);
	n6_i2c_write(dev, 0x7d, 0x80);
	n6_i2c_write(dev, 0x80, 0x00);
	n6_i2c_write(dev, 0x90, 0x01);
	n6_i2c_write(dev, 0xa9, 0x00);
	n6_i2c_write(dev, 0xb5, 0x40);
	n6_i2c_write(dev, 0xb8, 0x39);
	n6_i2c_write(dev, 0xb9, 0x72);
	n6_i2c_write(dev, 0xd1, 0x00);
	n6_i2c_write(dev, 0xd5, 0x80);

	n6_i2c_write(dev, 0xff, 0x09);
	n6_i2c_write(dev, 0x96+ch*0x20, 0x00);
	n6_i2c_write(dev, 0x98+ch*0x20, 0x00);
	n6_i2c_write(dev, ch*0x20+0x9e, 0x00);
	
	n6_i2c_write(dev, 0xff, _MAR_BANK_);
	val_20x01 = n6_i2c_read(dev, 0x01);
	val_20x01 &=(~(0x03<<(ch*2)));
	//val_20x01 |=(0x01<<(ch*2));
	n6_i2c_write(dev, 0x01, val_20x01);
	n6_i2c_write(dev, 0x12+ch*2, 0x80);
	n6_i2c_write(dev, 0x13+ch*2, 0x02);
	
}

void n6_set_chn_1080p(unsigned char dev, unsigned char ch, unsigned char ntpal)
{
	unsigned char val_0x54, val_20x01;
	uartlog("[%s::%d]\n", __func__, __LINE__);
	n6_i2c_write(dev, 0xff, 0x00);
	n6_i2c_write(dev, 0x08+ch, 0x00);
	n6_i2c_write(dev, 0x18+ch, 0x3f);
	n6_i2c_write(dev, 0x30+ch, 0x12);
	n6_i2c_write(dev, 0x34+ch, 0x00);
	val_0x54 = n6_i2c_read(dev, 0x54);
	val_0x54 &= ~(0x10<<ch);
	n6_i2c_write(dev, 0x54, val_0x54);
	n6_i2c_write(dev, 0x58+ch, ntpal?0x80:0x80);
	n6_i2c_write(dev, 0x5c+ch, ntpal?0x00:0x00);
	n6_i2c_write(dev, 0x64+ch, ntpal?0x01:0x01);
	n6_i2c_write(dev, 0x81+ch, ntpal?0x03:0x02);
	n6_i2c_write(dev, 0x85+ch, 0x00);
	n6_i2c_write(dev, 0x89+ch, 0x10);
	n6_i2c_write(dev, ch+0x8e, 0x00);
	n6_i2c_write(dev, 0xa0+ch, 0x05);

	n6_i2c_write(dev, 0xff, 0x01);
	n6_i2c_write(dev, 0x84+ch, 0x02);
	n6_i2c_write(dev, 0x88+ch, 0x00);
	n6_i2c_write(dev, 0x8c+ch, 0x40);
	n6_i2c_write(dev, 0xa0+ch, 0x20);

	n6_i2c_write(dev, 0xff, 0x05+ch);
	n6_i2c_write(dev, 0x01, 0x22);
	n6_i2c_write(dev, 0x05, 0x04);
	n6_i2c_write(dev, 0x08, 0x55);
	n6_i2c_write(dev, 0x25, 0xdc);
	n6_i2c_write(dev, 0x28, 0x80);
	n6_i2c_write(dev, 0x2f, 0x00);
	n6_i2c_write(dev, 0x30, 0xe0);
	n6_i2c_write(dev, 0x31, 0x41);
	n6_i2c_write(dev, 0x32, 0xa2);
	n6_i2c_write(dev, 0x47, 0xee);
	n6_i2c_write(dev, 0x50, 0xc6);
	n6_i2c_write(dev, 0x57, 0x00);
	n6_i2c_write(dev, 0x58, 0x77);
	n6_i2c_write(dev, 0x5b, 0x41);
	n6_i2c_write(dev, 0x5c, 0x7C);
	n6_i2c_write(dev, 0x5f, 0x00);
	n6_i2c_write(dev, 0x62, 0x20);
	n6_i2c_write(dev, 0x7b, 0x11);
	n6_i2c_write(dev, 0x7c, 0x01);
	n6_i2c_write(dev, 0x7d, 0x80);
	n6_i2c_write(dev, 0x80, 0x00);
	n6_i2c_write(dev, 0x90, 0x01);
	n6_i2c_write(dev, 0xa9, 0x00);
	n6_i2c_write(dev, 0xb5, 0x40);
	n6_i2c_write(dev, 0xb8, 0x39);
	n6_i2c_write(dev, 0xb9, 0x72);
	n6_i2c_write(dev, 0xd1, 0x00);
	n6_i2c_write(dev, 0xd5, 0x80);

	n6_i2c_write(dev, 0xff, 0x09);
	n6_i2c_write(dev, 0x96+ch*0x20, 0x00);
	n6_i2c_write(dev, 0x98+ch*0x20, 0x00);
	n6_i2c_write(dev, ch*0x20+0x9e, 0x00);
	
	n6_i2c_write(dev, 0xff, _MAR_BANK_);
	val_20x01 = n6_i2c_read(dev, 0x01);
	val_20x01 &=(~(0x03<<(ch*2)));
	n6_i2c_write(dev, 0x01, val_20x01);
	n6_i2c_write(dev, 0x12+ch*2, 0xc0);
	n6_i2c_write(dev, 0x13+ch*2, 0x03);
	
}

unsigned char n6_read_vfc(unsigned char dev, unsigned char ch)
{
	unsigned char ch_vfc = 0xff;
	n6_i2c_write(dev, 0xff, 0x05+ch);
	ch_vfc = n6_i2c_read(dev, 0xf0);
	return ch_vfc;
}

int n6_poweron(void *priv)
{
	pattern_ctx *ctx = (pattern_ctx *)priv;
	struct sensor_info *sinf = (struct sensor_info *)ctx->ptn->priv;
	unsigned char ch_vfc[16]={0xff,0xff,0xff,0xff,0xff};
	unsigned char val_13x70, val_13x71;
	struct snr_i2c_adapter *adpt = NULL;
	struct sensor_mode *mode = &n6_mode[N6_MODE_720P30];
	char cmdbuf[8];
	uint32_t value, tmp;
	int check_cnt=0;
	unsigned char ch;
	unsigned int test_mode;

	disable_irq(CSIMAC0_INTR_NUM);
	bm_get_num("manual(0), auto(1)", adet);

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
	// i2c config here
	snr_bm_i2c_get_adap((void *)&adpt);
	sinf->adpt = adpt;

	adpt->slaveaddr = sinf->dev;
	adpt->speed = 400000;
	snr_bm_i2c_init(adpt);

	// start rx init.
	if(adet == 0)   //manual mode 
	{
		n6_common_setting(sinf->dev);
		uartlog("[%s::%d]] fmt[%d]\n", __func__, __LINE__, fmt);
		//n6_set_chn_720p(sinf->dev, 0, VFMT_NTSC);//test
		//n6_set_chn_720p(sinf->dev, 1, VFMT_NTSC);
		n6_set_chn_720p(sinf->dev, 0, VFMT_PAL);
		//n6_set_chn_720p(sinf->dev, 1, VFMT_PAL);
		n6_set_chn_720p(sinf->dev, 2, VFMT_NTSC);
	} else  //auto detection mode
	{
		n6_common_setting(sinf->dev);
		uartlog("[%s::%d] auto detection check routine\n", __func__, __LINE__);
		
		n6_i2c_write(sinf->dev, 0xFF, 0x13);
		n6_i2c_write(sinf->dev, 0x30, 0x7f);
		n6_i2c_write(sinf->dev, 0x70, 0xf0);

		n6_i2c_write(sinf->dev, 0xFF, 0x00);
		n6_i2c_write(sinf->dev, 0x00, 0x18);
		n6_i2c_write(sinf->dev, 0x01, 0x18);
		n6_i2c_write(sinf->dev, 0x02, 0x18);
		n6_i2c_write(sinf->dev, 0x03, 0x18);

		n6_i2c_write(sinf->dev, 0x00, 0x10);
		n6_i2c_write(sinf->dev, 0x01, 0x10);
		n6_i2c_write(sinf->dev, 0x02, 0x10);
		n6_i2c_write(sinf->dev, 0x03, 0x10);
		
		while((check_cnt++) < 50)
		{
			for (ch = 0; ch < 4; ch++)
			{
				ch_vfc[ch] = n6_read_vfc(sinf->dev, ch);
				if(ch_vfc[ch] != 0xff)
					uartlog("ch[%d] video vfc read value0 : %2x\n", ch, ch_vfc[ch]);
			}
			if(ch_vfc[2] != 0xFF)  //for test, just check ch0
			{
				uartlog("ch[%d] check_cnt[%d] video vfc read value0 : %2x\n", ch, check_cnt, ch_vfc[2]);
				break;
			}
			else
				udelay(40000);
		}
		//ch_vfc[0] = 0x21;
		//ch_vfc[0] = 0x21;
		
		{
			for (ch = 0; ch < 4; ch++)
			{
				n6_i2c_write(sinf->dev, 0xFF, 0x13);
				val_13x70 = n6_i2c_read(sinf->dev, 0x70);
				val_13x70 |= (0x01<<ch);
				n6_i2c_write(sinf->dev, 0x70, val_13x70);
				val_13x71 = n6_i2c_read(sinf->dev, 0x71);
				val_13x71 |= (0x01<<ch);
				n6_i2c_write(sinf->dev, 0x71, val_13x71);
				
				switch(ch_vfc[ch]) // only check ch 0
				{
					case 0x00: n6_set_chn_960h(sinf->dev, ch, VFMT_NTSC);	break;
					case 0x10: n6_set_chn_960h(sinf->dev, ch, VFMT_PAL);	break;	
					case 0x20: n6_set_chn_720p(sinf->dev, ch, VFMT_NTSC);	break;
					case 0x21: n6_set_chn_720p(sinf->dev, ch, VFMT_PAL);	break;	
					case 0x30: n6_set_chn_1080p(sinf->dev, ch, VFMT_NTSC);	break;
					case 0x31: n6_set_chn_1080p(sinf->dev, ch, VFMT_PAL);	break;	
					case 0xa0: n6_set_chn_960p(sinf->dev, ch, VFMT_NTSC);	break;
					case 0xa1: n6_set_chn_960p(sinf->dev, ch, VFMT_PAL);	break;	
					default:	
						uartlog("ch_vfc not valid,set to default format\n");	
						n6_i2c_write(sinf->dev, 0xFF, 0x13);
						val_13x70 = n6_i2c_read(sinf->dev, 0x70);
						val_13x70 &= (~(0x01<<ch));
						n6_i2c_write(sinf->dev, 0x70, val_13x70);
						n6_set_chn_1080p(sinf->dev, ch, VFMT_NTSC);
					break;	
				}
			}
		}
	}

	bm_get_num("normal(0), test_mode(1)", test_mode);
	if (test_mode) {
		n6_i2c_write(sinf->dev, 0xFF, 0x5);
		n6_i2c_write(sinf->dev, 0x2c, 0x8);
		n6_i2c_write(sinf->dev, 0xFF, 0x6);
		n6_i2c_write(sinf->dev, 0x2c, 0x8);
		n6_i2c_write(sinf->dev, 0xFF, 0x7);
		n6_i2c_write(sinf->dev, 0x2c, 0x8);
		n6_i2c_write(sinf->dev, 0xFF, 0x8);
		n6_i2c_write(sinf->dev, 0x2c, 0x8);
		n6_i2c_write(sinf->dev, 0xFF, 0x0);
		n6_i2c_write(sinf->dev, 0x78, 0xaa);
		n6_i2c_write(sinf->dev, 0x79, 0xaa);
	}
#if 1
	n6_i2c_write(sinf->dev, 0xff, 0x23);  //continuous clock
	n6_i2c_write(sinf->dev, 0xe1, 0x02);
	n6_i2c_write(sinf->dev, 0xff, 0x33);  //continuous clock
	n6_i2c_write(sinf->dev, 0xe1, 0x02);
#endif
	// linear
	writel(0x0a0c2040, 0x0);
	sinf->ishdr = false;

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
	writel(0x0A0D0054, 0x05010F01);
	// Config the PHYD lane enable
	value = readl(0x0A0D0024);
	value &= ~0xFF0FFFFF;
	writel(0x0A0D0024, value);
	// PHYD lane enable
	value |= (use_4L ? 0x0F000000 :0x03000000);
	// CLK INV
	value |= (use_4L ? (0xF<<20) : (0x3<<20));
	value |= 0x14320;
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
		//[TODO]
		config_csibdg(0, mode->total_w, mode->total_h, ctx,
			      4, 0);
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

int peri_up_n6(void *priv)
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
	uint32_t value;

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
	//[TODO] Enable HDR-VC
	writel(0x0a0c2404, 0xD0000);
	//[TODO] Config VC mapping
	writel(0x0a0c2418, 0x3210);
	// config the vs gen mode.
	writel(0x0a0c2470, 0xF00);

#if !defined(FLOW_AFTER_ECO)
	uartlog("clear reset!\n");
	fpga_pwr_reg = (unsigned int *)HARD_REG_FPGA_PWRPIN;
	// clear reset and enable phy rx
	writel(fpga_pwr_reg, 0x32100000);
#endif
	uartlog("### device poweron\n");

	return 0;
}

int peri_down_n6(void *priv)
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

