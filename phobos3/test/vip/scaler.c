#ifdef ENV_CVITEST
#include <common.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>
#include "system_common.h"
#include "timer.h"
#elif defined(ENV_EMU)
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include "emu/command.h"
#else
#include <linux/types.h>
#include <linux/delay.h>

#endif  // ENV_CVITEST

#include "vip_common.h"
#include "scaler.h"
#include "vip_sys_reg.h"
#include "scaler_reg.h"
#include "reg.h"
#include "cmdq.h"
#include "dsi_phy.h"
#include "reg_sc_core.h"
#include "reg_sc_top.h"
#include "reg_disp.h"
#include "reg_img_in.h"
#include "reg_sc_odma.h"

/****************************************************************************
 * Global parameters
 ****************************************************************************/
static struct sclr_top_cfg g_top_cfg;
static struct sclr_oenc_cfg g_oenc_cfg;
static struct sclr_core_cfg g_sc_cfg[SCL_MAX_INST];
static struct sclr_gop_cfg g_gop_cfg[SCL_MAX_INST][SCL_MAX_GOP_INST];
static struct sclr_img_cfg g_img_cfg[2];
static struct sclr_cir_cfg g_cir_cfg[SCL_MAX_INST];
static struct sclr_border_cfg g_bd_cfg[SCL_MAX_INST];
static struct sclr_odma_cfg g_odma_cfg[SCL_MAX_INST];
static struct sclr_disp_cfg g_disp_cfg;
static struct sclr_disp_timing disp_timing;
static uintptr_t reg_base;
/****************************************************************************
 * Initial info
 ****************************************************************************/
#define DEFINE_CSC_COEF0(a, b, c) \
		.coef[0][0] = a, .coef[0][1] = b, .coef[0][2] = c,
#define DEFINE_CSC_COEF1(a, b, c) \
		.coef[1][0] = a, .coef[1][1] = b, .coef[1][2] = c,
#define DEFINE_CSC_COEF2(a, b, c) \
		.coef[2][0] = a, .coef[2][1] = b, .coef[2][2] = c,
static struct sclr_csc_matrix csc_mtrx[SCL_CSC_MAX] = {
	// none
	{
		DEFINE_CSC_COEF0(BIT(10),	0,		0)
		DEFINE_CSC_COEF1(0,		BIT(10),	0)
		DEFINE_CSC_COEF2(0,		0,		BIT(10))
		.sub[0] = 0,   .sub[1] = 0,   .sub[2] = 0,
		.add[0] = 0,   .add[1] = 0,   .add[2] = 0
	},
	// yuv2rgb
	// 601 Limited
	//  R = Y + 1.402* Pr                           //
	//  G = Y - 0.344 * Pb  - 0.792* Pr             //
	//  B = Y + 1.772 * Pb                          //
	{
		DEFINE_CSC_COEF0(BIT(10),	0,		1436)
		DEFINE_CSC_COEF1(BIT(10),	BIT(13) | 352,	BIT(13) | 731)
		DEFINE_CSC_COEF2(BIT(10),	1815,		0)
		.sub[0] = 0,   .sub[1] = 128, .sub[2] = 128,
		.add[0] = 0,   .add[1] = 0,   .add[2] = 0
	},
	// 601 Full
	//  R = 1.164 *(Y - 16) + 1.596 *(Cr - 128)                     //
	//  G = 1.164 *(Y - 16) - 0.392 *(Cb - 128) - 0.812 *(Cr - 128) //
	//  B = 1.164 *(Y - 16) + 2.016 *(Cb - 128)                     //
	{
		DEFINE_CSC_COEF0(1192,	0,		1634)
		DEFINE_CSC_COEF1(1192,	BIT(13) | 401,	BIT(13) | 833)
		DEFINE_CSC_COEF2(1192,	2065,		0)
		.sub[0] = 16,  .sub[1] = 128, .sub[2] = 128,
		.add[0] = 0,   .add[1] = 0,   .add[2] = 0
	},
	// 709 Limited
	// R = Y + 1.540(Cr – 128)
	// G = Y - 0.183(Cb – 128) – 0.459(Cr – 128)
	// B = Y + 1.816(Cb – 128)
	{
		DEFINE_CSC_COEF0(BIT(10),	0,		1577)
		DEFINE_CSC_COEF1(BIT(10),	BIT(13) | 187,	BIT(13) | 470)
		DEFINE_CSC_COEF2(BIT(10),	1860,		0)
		.sub[0] = 0,   .sub[1] = 128, .sub[2] = 128,
		.add[0] = 0,   .add[1] = 0,   .add[2] = 0
	},
	// 709 Full
	//  R = 1.164 *(Y - 16) + 1.792 *(Cr - 128)                     //
	//  G = 1.164 *(Y - 16) - 0.213 *(Cb - 128) - 0.534 *(Cr - 128) //
	//  B = 1.164 *(Y - 16) + 2.114 *(Cb - 128)                     //
	{
		DEFINE_CSC_COEF0(1192,	0,		1836)
		DEFINE_CSC_COEF1(1192,	BIT(13) | 218,	BIT(13) | 547)
		DEFINE_CSC_COEF2(1192,	2166,		0)
		.sub[0] = 16,  .sub[1] = 128, .sub[2] = 128,
		.add[0] = 0,   .add[1] = 0,   .add[2] = 0
	},
	// rgb2yuv
	// 601 Limited
	//  Y = 0.299 * R + 0.587 * G + 0.114 * B       //
	// Pb =-0.169 * R - 0.331 * G + 0.500 * B       //
	// Pr = 0.500 * R - 0.419 * G - 0.081 * B       //
	{
		DEFINE_CSC_COEF0(306,		601,		117)
		DEFINE_CSC_COEF1(BIT(13)|173,	BIT(13)|339,	512)
		DEFINE_CSC_COEF2(512,		BIT(13)|429,	BIT(13)|83)
		.sub[0] = 0,   .sub[1] = 0,   .sub[2] = 0,
		.add[0] = 0,   .add[1] = 128, .add[2] = 128
	},
	// 601 Full
	//  Y = 16  + 0.257 * R + 0.504 * g + 0.098 * b //
	// Cb = 128 - 0.148 * R - 0.291 * g + 0.439 * b //
	// Cr = 128 + 0.439 * R - 0.368 * g - 0.071 * b //
	{
		DEFINE_CSC_COEF0(263,		516,		100)
		DEFINE_CSC_COEF1(BIT(13)|152,	BIT(13)|298,	450)
		DEFINE_CSC_COEF2(450,		BIT(13)|377,	BIT(13)|73)
		.sub[0] = 0,   .sub[1] = 0,   .sub[2] = 0,
		.add[0] = 16,  .add[1] = 128, .add[2] = 128
	},
	// 709 Limited
	//   Y =       0.2126   0.7152   0.0722
	//  Cb = 128 - 0.1146  -0.3854   0.5000
	//  Cr = 128 + 0.5000  -0.4542  -0.0468
	{
		DEFINE_CSC_COEF0(218,		732,		74)
		DEFINE_CSC_COEF1(BIT(13)|117,	BIT(13)|395,	512)
		DEFINE_CSC_COEF2(512,		BIT(13)|465,	BIT(13)|48)
		.sub[0] = 0,   .sub[1] = 0,   .sub[2] = 0,
		.add[0] = 0,   .add[1] = 128, .add[2] = 128
	},
	// 709 Full
	//  Y = 16  + 0.183 * R + 0.614 * g + 0.062 * b //
	// Cb = 128 - 0.101 * R - 0.339 * g + 0.439 * b //
	// Cr = 128 + 0.439 * R - 0.399 * g - 0.040 * b //
	{
		DEFINE_CSC_COEF0(187,		629,		63)
		DEFINE_CSC_COEF1(BIT(13)|103,	BIT(13)|347,	450)
		DEFINE_CSC_COEF2(450,		BIT(13)|408,	BIT(13)|41)
		.sub[0] = 0,   .sub[1] = 0,   .sub[2] = 0,
		.add[0] = 16,  .add[1] = 128, .add[2] = 128
	},
};

static int scl_coef[128][4] = {
	{-4,    1024,   4,  0},
	{-8,    1024,   8,  0},
	{-12,   1024,   12, 0},
	{-16,   1024,   16, 0},
	{-19,   1023,   20, 0},
	{-23,   1023,   25, -1},
	{-26,   1022,   29, -1},
	{-30,   1022,   33, -1},
	{-34,   1022,   37, -1},
	{-37,   1021,   42, -2},
	{-40,   1020,   46, -2},
	{-44,   1020,   50, -2},
	{-47,   1019,   55, -3},
	{-50,   1018,   59, -3},
	{-53,   1017,   63, -3},
	{-56,   1016,   68, -4},
	{-59,   1015,   72, -4},
	{-62,   1014,   77, -5},
	{-65,   1013,   81, -5},
	{-68,   1012,   86, -6},
	{-71,   1011,   90, -6},
	{-74,   1010,   95, -7},
	{-76,   1008,   100,    -8},
	{-79,   1007,   104,    -8},
	{-81,   1005,   109,    -9},
	{-84,   1004,   113,    -9},
	{-86,   1002,   118,    -10},
	{-89,   1001,   123,    -11},
	{-91,   999,    128,    -12},
	{-94,   998,    132,    -12},
	{-96,   996,    137,    -13},
	{-98,   994,    142,    -14},
	{-100,  992,    147,    -15},
	{-102,  990,    152,    -16},
	{-104,  988,    157,    -17},
	{-106,  986,    161,    -17},
	{-108,  984,    166,    -18},
	{-110,  982,    171,    -19},
	{-112,  980,    176,    -20},
	{-114,  978,    181,    -21},
	{-116,  976,    186,    -22},
	{-117,  973,    191,    -23},
	{-119,  971,    196,    -24},
	{-121,  969,    201,    -25},
	{-122,  966,    206,    -26},
	{-124,  964,    211,    -27},
	{-125,  961,    216,    -28},
	{-127,  959,    221,    -29},
	{-128,  956,    226,    -30},
	{-130,  954,    231,    -31},
	{-131,  951,    237,    -33},
	{-132,  948,    242,    -34},
	{-133,  945,    247,    -35},
	{-134,  942,    252,    -36},
	{-136,  940,    257,    -37},
	{-137,  937,    262,    -38},
	{-138,  934,    267,    -39},
	{-139,  931,    273,    -41},
	{-140,  928,    278,    -42},
	{-141,  925,    283,    -43},
	{-142,  922,    288,    -44},
	{-142,  918,    294,    -46},
	{-143,  915,    299,    -47},
	{-144,  912,    304,    -48},
	{-145,  909,    309,    -49},
	{-145,  905,    315,    -51},
	{-146,  902,    320,    -52},
	{-147,  899,    325,    -53},
	{-147,  895,    330,    -54},
	{-148,  892,    336,    -56},
	{-148,  888,    341,    -57},
	{-149,  885,    346,    -58},
	{-149,  881,    352,    -60},
	{-150,  878,    357,    -61},
	{-150,  874,    362,    -62},
	{-150,  870,    367,    -63},
	{-151,  867,    373,    -65},
	{-151,  863,    378,    -66},
	{-151,  859,    383,    -67},
	{-151,  855,    389,    -69},
	{-151,  851,    394,    -70},
	{-152,  848,    399,    -71},
	{-152,  844,    405,    -73},
	{-152,  840,    410,    -74},
	{-152,  836,    415,    -75},
	{-152,  832,    421,    -77},
	{-152,  828,    426,    -78},
	{-152,  824,    431,    -79},
	{-151,  819,    437,    -81},
	{-151,  815,    442,    -82},
	{-151,  811,    447,    -83},
	{-151,  807,    453,    -85},
	{-151,  803,    458,    -86},
	{-151,  799,    463,    -87},
	{-150,  794,    469,    -89},
	{-150,  790,    474,    -90},
	{-150,  786,    479,    -91},
	{-149,  781,    485,    -93},
	{-149,  777,    490,    -94},
	{-149,  773,    495,    -95},
	{-148,  768,    501,    -97},
	{-148,  764,    506,    -98},
	{-147,  759,    511,    -99},
	{-147,  755,    516,    -100},
	{-146,  750,    522,    -102},
	{-146,  746,    527,    -103},
	{-145,  741,    532,    -104},
	{-144,  736,    537,    -105},
	{-144,  732,    543,    -107},
	{-143,  727,    548,    -108},
	{-142,  722,    553,    -109},
	{-142,  718,    558,    -110},
	{-141,  713,    563,    -111},
	{-140,  708,    569,    -113},
	{-140,  704,    574,    -114},
	{-139,  699,    579,    -115},
	{-138,  694,    584,    -116},
	{-137,  689,    589,    -117},
	{-136,  684,    594,    -118},
	{-135,  679,    600,    -120},
	{-135,  675,    605,    -121},
	{-134,  670,    610,    -122},
	{-133,  665,    615,    -123},
	{-132,  660,    620,    -124},
	{-131,  655,    625,    -125},
	{-130,  650,    630,    -126},
	{-129,  645,    635,    -127},
	{-128,  640,    640,    -128},
};

/****************************************************************************
 * Interfaces
 ****************************************************************************/
void sclr_set_base_addr(void *base)
{
	reg_base = (uintptr_t)base;
}

/**
 * sclr_top_set_cfg - set scl-top's configurations.
 *
 * @param cfg: scl-top's settings.
 */
void sclr_top_set_cfg(struct sclr_top_cfg *cfg)
{
	union sclr_top_cfg_01 cfg_01;

	_reg_write(reg_base + REG_SCL_TOP_CFG0, cfg->ip_trig_src << 3);

	cfg_01.raw = 0;
	cfg_01.b.sc_d_enable = cfg->sclr_enable[0];
	cfg_01.b.sc_v1_enable = cfg->sclr_enable[1];
	cfg_01.b.sc_v2_enable = cfg->sclr_enable[2];
	cfg_01.b.sc_v3_enable = cfg->sclr_enable[3];
	cfg_01.b.disp_enable = cfg->disp_enable;
	cfg_01.b.disp_from_sc = cfg->disp_from_sc;
	cfg_01.b.sc_d_src = cfg->sclr_d_src;
	cfg_01.b.sc_debug_en = 1;
	cfg_01.b.qos_en = 0xff;
	_reg_write(reg_base + REG_SCL_TOP_CFG1, cfg_01.raw);

	sclr_img_set_trig(SCL_IMG_D, cfg->img_in_d_trig_src);
	sclr_img_set_trig(SCL_IMG_V, cfg->img_in_v_trig_src);

	sclr_top_reg_done();
	sclr_top_reg_force_up();
}

/**
 * sclr_rt_set_cfg - configure sc's channels real-time.
 *
 * @param cfg: sc's channels real-time settings.
 */
void sclr_rt_set_cfg(union sclr_rt_cfg cfg)
{
	_reg_write(reg_base + REG_SCL_TOP_AXI, cfg.raw);
}

union sclr_rt_cfg sclr_rt_get_cfg(void)
{
	union sclr_rt_cfg cfg;

	cfg.raw = _reg_read(reg_base + REG_SCL_TOP_AXI);
	return cfg;
}

/**
 * sclr_top_get_cfg - get scl_top's cfg
 *
 * @return: scl_top's cfg
 */
struct sclr_top_cfg *sclr_top_get_cfg(void)
{
	return &g_top_cfg;
}

/**
 * sclr_top_reg_done - to mark all sc-reg valid for update.
 *
 */
void sclr_top_reg_done(void)
{
	_reg_write_mask(reg_base + REG_SCL_TOP_CFG0, BIT(0), BIT(0));
}

/**
 * sclr_top_reg_force_up - trigger reg update by sw.
 *
 */
void sclr_top_reg_force_up(void)
{
	_reg_write_mask(reg_base + REG_SCL_TOP_SHD, 0x00ff, 0xff);
}

void sclr_top_set_vo_type_sel(enum sclr_vo_sel vo_sel)
{
	union  sclr_sc_top_vo_mux_sel vo_mux;
	vo_mux.raw = _reg_read(reg_base + REG_SCL_TOP_VO_MUX);
	vo_mux.b.vo_sel_type = vo_sel;

	_reg_write(reg_base + REG_SCL_TOP_VO_MUX, vo_mux.raw);
}

void sclr_top_set_vo_data_mux(u8 vodata_selID, u8 value)
{
     u32 vo_mux_addr[10]={REG_SCL_TOP_VO_MUX0, REG_SCL_TOP_VO_MUX1, REG_SCL_TOP_VO_MUX2,
                          REG_SCL_TOP_VO_MUX3, REG_SCL_TOP_VO_MUX4, REG_SCL_TOP_VO_MUX5,
                          REG_SCL_TOP_VO_MUX6};

	union  sclr_sc_top_vo_mux vo_mux;
	u8 vodata_muxidx = (vodata_selID + 2) / 4;
	u8 vodata_selidx = (vodata_selID + 2) % 4;

	vo_mux.raw = _reg_read(reg_base + vo_mux_addr[vodata_muxidx]);

	switch(vodata_selidx)
	{
		case 0:
			vo_mux.b.vod_sel0 = value;
			break;
		case 1:
			vo_mux.b.vod_sel1 = value;
			break;
		case 2:
			vo_mux.b.vod_sel2 = value;
			break;
		case 3:
			vo_mux.b.vod_sel3 = value;
			break;
	}
	_reg_write(reg_base + vo_mux_addr[vodata_muxidx], vo_mux.raw);

}


u8 sclr_top_pg_late_get_bus(void)
{
	return (_reg_read(reg_base + REG_SCL_TOP_PG) >> 8) & 0xff;
}

void sclr_top_pg_late_clr(void)
{
	_reg_write_mask(reg_base + REG_SCL_TOP_PG, 0x0f0000, 0x80000);
}

/**
 * sclr_top_bld_set_cfg - configure blending
 *   if y-only enable, y-only data should be placed in img-d. uv data will pick from img-v.
 * @param cfg: blending's settings
 */
void sclr_top_bld_set_cfg(struct sclr_bld_cfg *cfg)
{
	u16 w = g_img_cfg[SCL_IMG_V].mem.width;

	_reg_write(reg_base + REG_SCL_TOP_BLD_CTRL, cfg->ctrl.raw);
	_reg_write(reg_base + REG_SCL_TOP_BLD_SIZE, ((w - 1) << 16) | (0x10000 / w));
}

void sclr_top_get_sb_default(struct sclr_top_sb_cfg *cfg)
{
	memset(cfg, 0, sizeof(*cfg));
}

void sclr_top_set_sb(struct sclr_top_sb_cfg *cfg)
{
	u32 val = ((cfg->sb_wr_ctrl0 << SC_TOP_REG_SB_WR_CTRL0_OFFSET) & SC_TOP_REG_SB_WR_CTRL0_MASK) |
		  ((cfg->sb_wr_ctrl1 << SC_TOP_REG_SB_WR_CTRL1_OFFSET) & SC_TOP_REG_SB_WR_CTRL1_MASK) |
		  ((cfg->sb_rd_ctrl << SC_TOP_REG_SB_RD_CTRL_OFFSET) & SC_TOP_REG_SB_RD_CTRL_MASK);

	_reg_write(reg_base + REG_SC_TOP_SB_CBAR, val);
}

/**
 * sclr_init - setup sclr's scaling coef
 *
 * @param inst: (0~3), the instance of scaler which want to be configured.
 */
void sclr_init(u8 inst)
{
	u8 i = 0;

	if (inst >= SCL_MAX_INST) {
		uartlog("[bm-vip][sc] %s: no enough sclr-instance ", __func__);
		uartlog("for the requirement(%d)\n", inst);
		return;
	}

	_reg_write(reg_base + REG_SCL_TOP_SHD,0x200);

	for (i = 0; i < 128; ++i) {
		_reg_write(reg_base + REG_SCL_COEF1(inst),
			   (scl_coef[i][1] << 16) |
			   (scl_coef[i][0] & 0x0fff));
		_reg_write(reg_base + REG_SCL_COEF2(inst),
			   (scl_coef[i][3] << 16) |
			   (scl_coef[i][2] & 0x0fff));
		_reg_write(reg_base + REG_SCL_COEF0(inst), (0x5 << 8) | i);
	}
}

/**
 * sclr_set_cfg - set scl's configurations.
 *
 * @param inst: (0~3), the instance of scaler which want to be configured.
 * @param sc_bypass: if bypass scaling-engine.
 * @param gop_bypass: if bypass gop-engine.
 * @param cir_bypass: if bypass circle-engine.
 * @param odma_bypass: if bypass odma-engine.
 */
void sclr_set_cfg(u8 inst, bool sc_bypass, bool gop_bypass,
		  bool cir_bypass, bool odma_bypass)
{
	g_sc_cfg[inst].sc_bypass = sc_bypass;
	g_sc_cfg[inst].gop_bypass = gop_bypass;
	g_sc_cfg[inst].cir_bypass = cir_bypass;
	g_sc_cfg[inst].odma_bypass = odma_bypass;

	_reg_write(reg_base + REG_SCL_CFG(inst), g_sc_cfg[inst].raw);
}

/**
 * sclr_get_cfg - get scl_core's cfg
 *
 * @param inst: (0~3), the instance of scaler which want to be configured.
 * @return: scl_core's cfg
 */
struct sclr_core_cfg *sclr_get_cfg(u8 inst)
{
	if (inst >= SCL_MAX_INST)
		return NULL;
	return &g_sc_cfg[inst];
}

/**
 * sclr_reg_shadow_sel - control the read reg-bank.
 *
 * @param inst: (0~3), the instance of scaler which want to be configured.
 * @param read_shadow: true(shadow); false(working)
 */
void sclr_reg_shadow_sel(u8 inst, bool read_shadow)
{
	_reg_write(reg_base + REG_SCL_SHD(inst), (read_shadow ? 0x0 : 0x2));
}

/**
 * sclr_reg_force_up - trigger reg update by sw.
 *
 * @param inst: (0~3), the instance of scaler which want to be configured.
 */
void sclr_reg_force_up(u8 inst)
{
	_reg_write_mask(reg_base + REG_SCL_TOP_SHD, BIT(3+inst), BIT(3+inst));
}

struct sclr_status sclr_get_status(u8 inst)
{
	u32 tmp = _reg_read(reg_base + REG_SCL_STATUS(inst));

	return *(struct sclr_status *)&tmp;
}

/**
 * sclr_set_input_size - update scl's input-size.
 *
 * @param inst: (0~3), the instance of scaler which want to be configured.
 * @param src_rect: size of input-src
 */
void sclr_set_input_size(u8 inst, struct sclr_size src_rect)
{
	_reg_write(reg_base + REG_SCL_SRC_SIZE(inst),
		   ((src_rect.h - 1) << 12) | (src_rect.w - 1));

	g_sc_cfg[inst].sc.src = src_rect;
}

/**
 * sclr_set_crop - update scl's crop-size/position.
 *
 * @param inst: (0~3), the instance of scaler which want to be configured.
 * @param crop_rect: to specify pos/size of crop
 */
void sclr_set_crop(u8 inst, struct sclr_rect crop_rect)
{
	_reg_write(reg_base + REG_SCL_CROP_OFFSET(inst),
		   (crop_rect.y << 12) | crop_rect.x);
	_reg_write(reg_base + REG_SCL_CROP_SIZE(inst),
		   ((crop_rect.h - 1) << 16) | (crop_rect.w - 1));

	g_sc_cfg[inst].sc.crop = crop_rect;
}

/**
 * sclr_set_output_size - update scl's output-size.
 *
 * @param inst: (0~3), the instance of scaler which want to be configured.
 * @param src_rect: size of output-src
 */
void sclr_set_output_size(u8 inst, struct sclr_size size)
{
	_reg_write(reg_base + REG_SCL_OUT_SIZE(inst),
		   ((size.h - 1) << 16) | (size.w - 1));

	g_sc_cfg[inst].sc.dst = size;

	if (inst == 0) {
		struct sclr_rect rect;

		rect.x = 0;
		rect.y = 0;
		rect.w = size.w;
		rect.h = size.h;
		sclr_disp_set_rect(rect);
	}
}

void sclr_set_scale_mir(u8 inst, bool enable)
{
	u32 tmp = 0;

	g_sc_cfg[inst].sc.mir_enable = enable;
	tmp = (enable) ? 0x03 : 0x02;
	// enable cb_mode of scaling, since it only works on fac > 4
	_reg_write(reg_base + REG_SCL_SC_CFG(inst), 0x10 | tmp);
}

/**
 * sclr_set_scale_phase - Used in tile mode to decide the phase of first data in scaling.
 *
 * @param inst: (0~3), the instance of scaler which want to be configured.
 * @param h_ph: initial horizontal phase
 * @param v_ph: initial vertical phase
 */
void sclr_set_scale_phase(u8 inst, u16 h_ph, u16 v_ph)
{
	_reg_write(reg_base + REG_SCL_SC_INI_PH(inst), (v_ph << 16) | h_ph);
}

/**
 * sclr_set_scale_mode - control sclr's scaling coef.
 *
 * @param inst: 0~2
 * @param mir_enable: mirror enable or not
 * @param cb_enable: cb mode or not. cb only work if scaling smaller than 1/4.
 *                   In cb mode, use it's own coeff rather than ones update by sclr_update_coef().
 * @param update: update parameter or not
 */
void sclr_set_scale_mode(u8 inst, bool mir_enable, bool cb_enable, bool update)
{
        u32 tmp = 0;

        if (update) {
                g_sc_cfg[inst].sc.mir_enable = mir_enable;
                //g_sc_cfg[inst].sc.cb_enable = cb_enable;
        }

        // mir/cb isn't suggested to enable together
        if (g_sc_cfg[inst].sc.tile_enable) {
                tmp = mir_enable ? 0x03 : 0x02;
        } else {
                tmp = cb_enable ? 0x13 : 0x03;
        }

        // enable cb_mode of scaling, since it only works on fac > 4
        _reg_write(reg_base + REG_SCL_SC_CFG(inst), tmp);
}

void sclr_get_2tap_scale(struct sclr_scale_2tap_cfg *cfg)
{
	u32 src_wd, src_ht, dst_wd, dst_ht, scale_x, scale_y;
	bool fast_area_x = 0, fast_area_y = 0;
	u32 h_nor = 0, v_nor = 0, h_ph = 0, v_ph = 0;
	bool area_fast = false;
	bool scale_down = false;

	if (!cfg)
		return;
	if (!cfg->src_wd || !cfg->src_ht || !cfg->dst_wd || !cfg->dst_ht)
		return;

	src_wd = cfg->src_wd;
	src_ht = cfg->src_ht;
	dst_wd = cfg->dst_wd;
	dst_ht = cfg->dst_ht;

	// Scale up: bilinear mode
	// Scale down: area mode
	if (src_wd >= dst_wd && src_ht >= dst_ht) {
		scale_down = true;
		fast_area_x = (src_wd % dst_wd) ? false : true;
		fast_area_y = (src_ht % dst_ht) ? false : true;
		area_fast = (fast_area_x && fast_area_y) ? true : false;
	}

	// scale_x = round((src_wd * 2^13)/dst_wd),0) -> 5.13
	scale_x = (src_wd * 8192) / dst_wd;

	// scale_y = round((src_ht * 2^13)/dst_ht),0) -> 5.13
	scale_y = (src_ht * 8192) / dst_ht;

	if (!area_fast) {
		h_nor =  (65536 * 8192) / scale_x;
		v_nor = (65536 * 8192) / scale_y;
	} else {
		// h_nor: don't care
		h_nor =  (65536 * 8192) / scale_x;
		v_nor = 65536 / ((scale_x >> 13) * (scale_y >> 13));
	}

	// Phase is used when scale up and reg_resize_blnr_mode = 0
	// Note: scale down with nonzero phase caused scaler blocked
	if (!scale_down) {
		h_ph = scale_x / 2;
		v_ph = scale_y / 2;
	}

	cfg->area_fast = area_fast;
	cfg->scale_x = scale_x;
	cfg->scale_y = scale_y;
	cfg->h_nor = h_nor;
	cfg->v_nor = v_nor;
	cfg->h_ph = h_ph;
	cfg->v_ph = v_ph;
}

void sclr_set_scale_2tap(u8 inst)
{
	u32 tmp;
	bool isFacOver4 = false;
	struct sclr_scale_2tap_cfg cfg;
	u32 blnr_mode = 0; // 0: 0 : neg-phase, sync with IVE C-code
	u32 dbg_sel = 4; // bit[2]

	if (g_sc_cfg[inst].sc.src.w == 0 || g_sc_cfg[inst].sc.src.h == 0)
		return;
	if (g_sc_cfg[inst].sc.crop.w == 0 || g_sc_cfg[inst].sc.crop.h == 0)
		return;
	if (g_sc_cfg[inst].sc.dst.w == 0 || g_sc_cfg[inst].sc.dst.h == 0)
		return;

	cfg.src_wd = g_sc_cfg[inst].sc.crop.w;
	cfg.src_ht = g_sc_cfg[inst].sc.crop.h;
	cfg.dst_wd = g_sc_cfg[inst].sc.dst.w;
	cfg.dst_ht = g_sc_cfg[inst].sc.dst.h;
	sclr_get_2tap_scale(&cfg);

	// scale_x, 5.13
	tmp = (cfg.scale_x << SC_CORE_REG_H_SC_FAC_OFFSET) & SC_CORE_REG_H_SC_FAC_MASK;
	_reg_write(reg_base + REG_SCL_SC_H_CFG(inst), tmp);

	// scale_y, 5.13
	tmp = (cfg.scale_y << SC_CORE_REG_V_SC_FAC_OFFSET) & SC_CORE_REG_V_SC_FAC_MASK;
	_reg_write(reg_base + REG_SCL_SC_V_CFG(inst), tmp);

	sclr_set_scale_mode(inst, g_sc_cfg[inst].sc.mir_enable, isFacOver4, true);

	tmp = (blnr_mode << SC_CORE_REG_RESIZE_BLNR_MODE_OFFSET) & SC_CORE_REG_RESIZE_BLNR_MODE_MASK;
	tmp |= (cfg.area_fast << SC_CORE_REG_RESIZE_AREA_FAST_OFFSET) & SC_CORE_REG_RESIZE_AREA_FAST_MASK;
	tmp |= (dbg_sel << SC_CORE_REG_RESIZE_DBG_SEL_OFFSET) & SC_CORE_REG_RESIZE_DBG_SEL_MASK;
	_reg_write(reg_base + REG_SCL_2TAP_CFG(inst), tmp);

	tmp = ((cfg.h_nor << SC_CORE_REG_RESIZE_H_NOR_OFFSET) & SC_CORE_REG_RESIZE_H_NOR_MASK);
	tmp |= ((cfg.v_nor << SC_CORE_REG_RESIZE_V_NOR_OFFSET) & SC_CORE_REG_RESIZE_V_NOR_MASK);
	_reg_write(reg_base + REG_SCL_2TAP_NOR(inst), tmp);

	sclr_set_scale_phase(inst, cfg.h_ph, cfg.v_ph);
}

/**
 * sclr_set_scale - update scl's scaling settings by current crop/dst values.
 *
 * @param inst: (0~3), the instance of scaler which want to be configured.
 */
void sclr_set_scale(u8 inst)
{
	u32 tmp = 0, scale_x, scale_y;
	bool isFacOver4 = false;
	bool use_2tap = (inst != 1) ? true : false;	// Only sc_v1 is 4 tap

	if (g_sc_cfg[inst].sc.src.w == 0 || g_sc_cfg[inst].sc.src.h == 0)
		return;
	if (g_sc_cfg[inst].sc.crop.w == 0 || g_sc_cfg[inst].sc.crop.h == 0)
		return;
	if (g_sc_cfg[inst].sc.dst.w == 0 || g_sc_cfg[inst].sc.dst.h == 0)
		return;

	if (use_2tap) {
		sclr_set_scale_2tap(inst);
		return;
	}

	// scale_x, 5.13
	scale_x = (((g_sc_cfg[inst].sc.crop.w - 1) << 13)
		+ (g_sc_cfg[inst].sc.dst.w >> 1))
	      / (g_sc_cfg[inst].sc.dst.w - 1);
	_reg_write_mask(reg_base + REG_SCL_SC_H_CFG(inst), 0x03ffff00,
			scale_x << 8);

	tmp = (g_sc_cfg[inst].sc.crop.w - 1) / (g_sc_cfg[inst].sc.dst.w - 1);
	isFacOver4 |= (tmp >= 4);

	// scale_y, 5.13
	scale_y = (((g_sc_cfg[inst].sc.crop.h - 1) << 13)
		+ (g_sc_cfg[inst].sc.dst.h >> 1))
	      / (g_sc_cfg[inst].sc.dst.h - 1);
	_reg_write_mask(reg_base + REG_SCL_SC_V_CFG(inst), 0x03ffff00,
			scale_y << 8);

	tmp = (g_sc_cfg[inst].sc.crop.h - 1) / (g_sc_cfg[inst].sc.dst.h - 1);
	isFacOver4 |= (tmp >= 4);

	sclr_set_scale_mode(inst, g_sc_cfg[inst].sc.mir_enable, isFacOver4, true);

	sclr_set_scale_phase(inst, 0, 0);
}

void sclr_read_2tap_nor(u8 inst, u16 *resize_hnor, u16 *resize_vnor)
{
	u32 val = _reg_read(reg_base + REG_SCL_2TAP_NOR(inst));

	*resize_hnor = (val & SC_CORE_REG_RESIZE_H_NOR_MASK) >> SC_CORE_REG_RESIZE_H_NOR_OFFSET;
	*resize_vnor = (val & SC_CORE_REG_RESIZE_V_NOR_MASK) >> SC_CORE_REG_RESIZE_V_NOR_OFFSET;
}

union sclr_intr sclr_get_intr_mask(void)
{
	union sclr_intr intr_mask;

	intr_mask.raw = _reg_read(reg_base + REG_SCL_TOP_INTR_MASK);
	return intr_mask;
}

/**
 * sclr_set_intr_mask - sclr's interrupt mask. Only enable ones will be
 *			integrated to vip_subsys.  check 'union sclr_intr'
 *			for each bit mask.
 *
 * @param intr_mask: On/Off ctrl of the interrupt.
 */
void sclr_set_intr_mask(union sclr_intr intr_mask)
{
	_reg_write(reg_base + REG_SCL_TOP_INTR_MASK, intr_mask.raw);
}

/**
 * sclr_intr_ctrl - sclr's interrupt on(1)/off(0)
 *                  check 'union sclr_intr' for each bit mask
 *
 * @param intr_mask: On/Off ctrl of the interrupt.
 */
void sclr_intr_ctrl(union sclr_intr intr_mask)
{
	_reg_write(reg_base + REG_SCL_TOP_INTR_ENABLE, intr_mask.raw);
}

/**
 * sclr_intr_clr - clear sclr's interrupt
 *                 check 'union sclr_intr' for each bit mask
 *
 * @param intr_mask: On/Off ctrl of the interrupt.
 */
void sclr_intr_clr(union sclr_intr intr_mask)
{
	_reg_write(reg_base + REG_SCL_TOP_INTR_STATUS, intr_mask.raw);
}

/**
 * sclr_intr_status - sclr's interrupt status
 *                    check 'union sclr_intr' for each bit mask
 *
 * @return: The interrupt's status
 */
union sclr_intr sclr_intr_status(void)
{
	union sclr_intr status;

	status.raw = (_reg_read(reg_base + REG_SCL_TOP_INTR_STATUS) & 0xffffffff);
	return status;
}

/****************************************************************************
 * SCALER IMG
 ****************************************************************************/
/**
 * sclr_img_set_cfg - set img's configurations.
 *
 * @param img_inst: (0~1), the instance of img-in which want to be configured.
 * @param cfg: img's settings.
 */
void sclr_img_set_cfg(u8 img_inst, struct sclr_img_cfg *cfg)
{
	if (img_inst == SCL_IMG_D) {
		struct sclr_top_cfg *top_cfg = sclr_top_get_cfg();

		top_cfg->sclr_d_src = (cfg->src == SCL_INPUT_D_IMG_V);
		sclr_top_set_cfg(top_cfg);
	}

	_reg_write(reg_base + REG_SCL_IMG_CFG(img_inst),
		   (cfg->csc_en << 12) | (cfg->burst << 8) |
		   (cfg->fmt << 4) | cfg->src);

	if (cfg->src == SCL_INPUT_D_MEM)
		sclr_img_set_mem(img_inst, &cfg->mem);

	uartlog("DBG img_src(%d) img_inst(%d)\n", cfg->src, img_inst);
	if (cfg->src == SCL_INPUT_D_ISP)
		sclr_img_set_trig(img_inst, SCL_IMG_TRIG_SRC_ISP);
	else {
		enum sclr_img_trig_src trig_src =
		    (img_inst == SCL_IMG_D && g_top_cfg.disp_from_sc)
		    ? SCL_IMG_TRIG_SRC_DISP
		    : SCL_IMG_TRIG_SRC_SW;

		sclr_img_set_trig(img_inst, trig_src);
	}

	if (cfg->csc == SCL_CSC_NONE) {
		sclr_img_csc_en(img_inst, false);
	} else {
		sclr_img_csc_en(img_inst, true);
		sclr_img_set_csc(img_inst, &csc_mtrx[cfg->csc]);
	}
	g_img_cfg[img_inst] = *cfg;
}

/**
 * sclr_img_set_trig - set img's src of job_start.
 *
 * @param inst: (0~1), the instance of img-in which want to be configured.
 * @param trig_src: img's src of job_start.
 */
void sclr_img_set_trig(u8 inst, enum sclr_img_trig_src trig_src)
{
	u32 mask = BIT(12) | BIT(8);
	u32 val = 0;

	switch (trig_src) {
	case SCL_IMG_TRIG_SRC_SW:
		break;
	case SCL_IMG_TRIG_SRC_DISP:
		val |= BIT(8);
		break;
	case SCL_IMG_TRIG_SRC_ISP:
		val |= BIT(12);
		break;
	default:
		break;
	}

	if (inst == SCL_IMG_V) {
		mask <<= 1;
		val <<= 1;
		g_top_cfg.img_in_v_trig_src = trig_src;
	} else {
		g_top_cfg.img_in_d_trig_src = trig_src;
	}
	uartlog("mask(%#x) val(%#x)\n", mask, val);
	_reg_write_mask(reg_base + REG_SCL_TOP_IMG_CTRL, mask, val);
}

/**
 * sclr_img_get_cfg - get scl_img's cfg
 *
 * @param inst: 0~1
 * @return: scl_img's cfg
 */
struct sclr_img_cfg *sclr_img_get_cfg(u8 inst)
{
	if (inst >= SCL_IMG_MAX)
		return NULL;
	return &g_img_cfg[inst];
}

/**
 * sclr_img_reg_shadow_sel - control scl_img's the read reg-bank.
 *
 * @param read_shadow: true(shadow); false(working)
 */
void sclr_img_reg_shadow_sel(u8 inst, bool read_shadow)
{
	_reg_write(reg_base + REG_SCL_IMG_SHD(inst), (read_shadow ? 0x0 : 0x4));
}

/**
 * sclr_img_reg_shadow_mask - reg won't be update by sw/hw until unmask.
 *
 * @param mask: true(mask); false(unmask)
 * @return: mask status before this modification.
 */
bool sclr_img_reg_shadow_mask(u8 inst, bool mask)
{
	bool is_masked = (_reg_read(reg_base + REG_SCL_IMG_SHD(inst)) & BIT(1));

	if (is_masked != mask)
		_reg_write_mask(reg_base + REG_SCL_IMG_SHD(inst), BIT(1),
				(mask ? 0x0 : BIT(1)));

	return is_masked;
}

/**
 * sclr_img_reg_force_up - trigger reg update by sw.
 *
 * @param inst: 0~1
 */
void sclr_img_reg_force_up(u8 inst)
{
	_reg_write_mask(reg_base + REG_SCL_TOP_SHD, BIT(1+inst), BIT(1+inst));
}

void sclr_img_start(u8 inst)
{
	inst = (~inst) & 0x01;
	_reg_write_mask(reg_base + REG_SCL_TOP_IMG_CTRL, 0x00000003, BIT(inst));
}

void sclr_img_start_all(void)
{
	u32 insts = 3; // img_in_v + img_in_d
	_reg_write_mask(reg_base + REG_SCL_TOP_IMG_CTRL, 0x00000003, insts);
}

/**
 * sclr_img_set_fmt - set scl_img's input data format
 *
 * @param inst: 0~1
 * @param fmt: scl_img's input format
 */
void sclr_img_set_fmt(u8 inst, enum sclr_format fmt)
{
	_reg_write_mask(reg_base + REG_SCL_IMG_CFG(inst), 0x000000f0, fmt << 4);

	g_img_cfg[inst].fmt = fmt;
}

/**
 * sclr_img_set_mem - setup img's mem settings. Only work if img from mem.
 *
 * @param mem: mem settings for img
 */
void sclr_img_set_mem(u8 inst, struct sclr_mem *mem)
{
	_reg_write(reg_base + REG_SCL_IMG_OFFSET(inst),
		   (mem->start_y << 16) | mem->start_x);
	_reg_write(reg_base + REG_SCL_IMG_SIZE(inst),
		   ((mem->height - 1) << 16) | (mem->width - 1));
	_reg_write(reg_base + REG_SCL_IMG_PITCH_Y(inst), mem->pitch_y);
	_reg_write(reg_base + REG_SCL_IMG_PITCH_C(inst), mem->pitch_c);

	sclr_img_set_addr(inst, mem->addr0, mem->addr1, mem->addr2);

	g_img_cfg[inst].mem = *mem;
}

/**
 * sclr_img_set_addr - setup img's mem address. Only work if img from mem.
 *
 * @param addr0: address of planar0
 * @param addr1: address of planar1
 * @param addr2: address of planar2
 */
void sclr_img_set_addr(u8 inst, u64 addr0, u64 addr1, u64 addr2)
{
	_reg_write(reg_base + REG_SCL_IMG_ADDR0_L(inst), addr0);
	_reg_write(reg_base + REG_SCL_IMG_ADDR0_H(inst), addr0 >> 32);
	_reg_write(reg_base + REG_SCL_IMG_ADDR1_L(inst), addr1);
	_reg_write(reg_base + REG_SCL_IMG_ADDR1_H(inst), addr1 >> 32);
	_reg_write(reg_base + REG_SCL_IMG_ADDR2_L(inst), addr2);
	_reg_write(reg_base + REG_SCL_IMG_ADDR2_H(inst), addr2 >> 32);

	g_img_cfg[inst].mem.addr0 = addr0;
	g_img_cfg[inst].mem.addr1 = addr1;
	g_img_cfg[inst].mem.addr2 = addr2;
}

void sclr_img_csc_en(u8 inst, bool enable)
{
	_reg_write_mask(reg_base + REG_SCL_IMG_CFG(inst), BIT(12),
			enable ? BIT(12) : 0);
}

/**
 * sclr_img_set_csc - configure scl-img's input CSC's coefficient/offset
 *
 * @param inst: (0~1)
 * @param cfg: The settings for CSC
 */
void sclr_img_set_csc(u8 inst, struct sclr_csc_matrix *cfg)
{
	_reg_write(reg_base + REG_SCL_IMG_CSC_COEF0(inst),
		   (cfg->coef[0][1] << 16) | cfg->coef[0][0]);
	_reg_write(reg_base + REG_SCL_IMG_CSC_COEF1(inst),
		   cfg->coef[0][2]);
	_reg_write(reg_base + REG_SCL_IMG_CSC_COEF2(inst),
		   (cfg->coef[1][1] << 16) | cfg->coef[1][0]);
	_reg_write(reg_base + REG_SCL_IMG_CSC_COEF3(inst), cfg->coef[1][2]);
	_reg_write(reg_base + REG_SCL_IMG_CSC_COEF4(inst),
		   (cfg->coef[2][1] << 16) | cfg->coef[2][0]);
	_reg_write(reg_base + REG_SCL_IMG_CSC_COEF5(inst), cfg->coef[2][2]);
	_reg_write(reg_base + REG_SCL_IMG_CSC_SUB(inst),
		   (cfg->sub[2] << 16) | (cfg->sub[1] << 8) | cfg->sub[0]);
	_reg_write(reg_base + REG_SCL_IMG_CSC_ADD(inst),
		   (cfg->add[2] << 16) | (cfg->add[1] << 8) | cfg->add[0]);
}

union sclr_img_dbg_status sclr_img_get_dbg_status(u8 inst, bool clr)
{
	union sclr_img_dbg_status status;

	status.raw = _reg_read(reg_base + REG_SCL_IMG_DBG(inst));

	if (clr) {
		status.b.err_fwr_clr = 1;
		status.b.err_erd_clr = 1;
		status.b.ip_clr = 1;
		status.b.ip_int_clr = 1;
		_reg_write(reg_base + REG_SCL_IMG_DBG(inst), status.raw);
	}

	return status;
}

void sclr_img_checksum_en(u8 inst, bool enable)
{
	_reg_write_mask(reg_base + REG_SCL_IMG_CHECKSUM0(inst), BIT(31),
	                enable ? BIT(31) : 0);
}

void sclr_img_get_checksum_status(u8 inst, struct sclr_img_checksum_status *status)
{
	status->checksum_base.raw = _reg_read(reg_base + REG_SCL_IMG_CHECKSUM0(inst));
	status->axi_read_data = _reg_read(reg_base + REG_SCL_IMG_CHECKSUM1(inst));

}

int sclr_img_validate_cb_cfg(struct sclr_img_in_sb_cfg *cfg)
{
	/* 0 : disable, 1 : free-run mode, 2 : frame base mode */
	if (cfg->sb_mode > 2)
		return -1;

	/* 0 : 64 line, 1 : 128 line, 2 : 192 line, 3 : 256 line */
	if (cfg->sb_size > 3)
		return -1;

	return 0;
}

void sclr_img_get_sb_default(struct sclr_img_in_sb_cfg *cfg)
{
	memset(cfg, 0, sizeof(*cfg));
	//cfg->mode = 0;	// disable
	//cfg->size = 0;	// 64 line
	cfg->sb_nb = 3;
	cfg->sb_sw_rptr = 0;
	//cfg->set_str = 0;
	//cfg->sw_clr = 0;
}

void sclr_img_set_sb(u8 inst, struct sclr_img_in_sb_cfg *cfg)
{
	if (inst >= 2)
		return;

	if (sclr_img_validate_cb_cfg(cfg))
		return;

	u32 val = ((cfg->sb_mode << IMG_IN_REG_SB_MODE_OFFSET) & IMG_IN_REG_SB_MODE_MASK) |
		  ((cfg->sb_size << IMG_IN_REG_SB_SIZE_OFFSET) & IMG_IN_REG_SB_SIZE_MASK) |
		  ((cfg->sb_nb << IMG_IN_REG_SB_NB_OFFSET) & IMG_IN_REG_SB_NB_MASK) |
		  ((cfg->sb_sw_rptr << IMG_IN_REG_SB_SW_RPTR_OFFSET) & IMG_IN_REG_SB_SW_RPTR_MASK) |
		  ((cfg->sb_set_str << IMG_IN_REG_SB_SET_STR_OFFSET) & IMG_IN_REG_SB_SET_STR_MASK) |
		  ((cfg->sb_sw_clr << IMG_IN_REG_SB_SW_CLR_OFFSET) & IMG_IN_REG_SB_SW_CLR_MASK);
	_reg_write(reg_base + REG_SCL_IMG_SB_REG_CTRL(inst), val);
}

/****************************************************************************
 * SCALER CIRCLE
 ****************************************************************************/
/**
 * sclr_cir_set_cfg - configure scl-circle
 *
 * @param inst: (0~3), the instance of scaler which want to be configured.
 * @param cfg: cir's settings.
 */
void sclr_cir_set_cfg(u8 inst, struct sclr_cir_cfg *cfg)
{
	if (cfg->mode == SCL_CIR_DISABLE) {
		_reg_write_mask(reg_base + REG_SCL_CFG(inst), 0x20, 0x20);
	} else {
		_reg_write(reg_base + REG_SCL_CIR_CFG(inst),
			   (cfg->line_width << 8) | cfg->mode);
		_reg_write(reg_base + REG_SCL_CIR_CENTER_X(inst),
			   cfg->center.x);
		_reg_write(reg_base + REG_SCL_CIR_CENTER_Y(inst),
			   cfg->center.y);
		_reg_write(reg_base + REG_SCL_CIR_RADIUS(inst), cfg->radius);
		_reg_write(reg_base + REG_SCL_CIR_SIZE(inst),
			   ((cfg->rect.h - 1) << 16) | (cfg->rect.w - 1));
		_reg_write(reg_base + REG_SCL_CIR_OFFSET(inst),
			   (cfg->rect.y << 16) | cfg->rect.x);
		_reg_write(reg_base + REG_SCL_CIR_COLOR(inst),
			   (cfg->color_b << 16) | (cfg->color_g << 8) |
			   cfg->color_r);

		_reg_write_mask(reg_base + REG_SCL_CFG(inst), 0x20, 0x00);
	}

	g_cir_cfg[inst] = *cfg;
}

/****************************************************************************
 * SCALER BORDER
 ****************************************************************************/
/**
 * sclr_odma_set_cfg - configure scaler's odma
 *
 * @param inst: (0~3), the instance of scaler which want to be configured.
 * @param cfg: new border config
 */
void sclr_border_set_cfg(u8 inst, struct sclr_border_cfg *cfg)
{
	if ((g_sc_cfg[inst].sc.dst.w + cfg->start.x) >
	    g_odma_cfg[inst].mem.width) {
		uartlog("[bm-vip][sc] %s: sc_width(%d) + offset(%d)", __func__,
		       g_sc_cfg[inst].sc.dst.w, cfg->start.x);
		uartlog(" > odma_width(%d)\n", g_odma_cfg[inst].mem.width);
		cfg->start.x = g_odma_cfg[inst].mem.width -
			       g_sc_cfg[inst].sc.dst.w;
	}

	if ((g_sc_cfg[inst].sc.dst.h + cfg->start.y) >
	    g_odma_cfg[inst].mem.height) {
		uartlog("[bm-vip][sc] %s: sc_height(%d) + offset(%d)", __func__,
		       g_sc_cfg[inst].sc.dst.h, cfg->start.y);
		uartlog(" > odma_height(%d)\n", g_odma_cfg[inst].mem.height);
		cfg->start.y = g_odma_cfg[inst].mem.height -
			       g_sc_cfg[inst].sc.dst.h;
	}

	_reg_write(reg_base + REG_SCL_BORDER_CFG(inst), cfg->cfg.raw);
	_reg_write(reg_base + REG_SCL_BORDER_OFFSET(inst),
		   (cfg->start.y << 16) | cfg->start.x);

	g_bd_cfg[inst] = *cfg;
}

/**
 * sclr_border_get_cfg - get scl_border's cfg
 *
 * @param inst: (0~3), the instance of scaler which want to be configured.
 * @return: scl_border's cfg
 */
struct sclr_border_cfg *sclr_border_get_cfg(u8 inst)
{
	if (inst >= SCL_MAX_INST)
		return NULL;
	return &g_bd_cfg[inst];
}

/****************************************************************************
 * SCALER ODMA
 ****************************************************************************/
/**
 * sclr_odma_set_cfg - configure scaler's odma
 *
 * @param inst: (0~3), the instance of scaler which want to be configured.
 * @param burst: dma's burst length
 * @param fmt: dma's format
 */
void sclr_odma_set_cfg(u8 inst, struct sclr_odma_cfg *cfg)
{
	_reg_write(reg_base + REG_SCL_ODMA_CFG(inst),
		   (cfg->flip << 16) |  (cfg->fmt << 8) | cfg->burst);

	sclr_odma_set_mem(inst, &cfg->mem);

	g_odma_cfg[inst] = *cfg;
}

/**
 * sclr_odma_get_cfg - get scl_odma's cfg
 *
 * @param inst: (0~3), the instance of scaler which want to be configured.
 * @return: scl_odma's cfg
 */
struct sclr_odma_cfg *sclr_odma_get_cfg(u8 inst)
{
	if (inst >= SCL_MAX_INST)
		return NULL;
	return &g_odma_cfg[inst];
}

/**
 * sclr_odma_set_fmt - set scl_odma's output data format
 *
 * @param inst: (0~3), the instance of scaler which want to be configured.
 * @param fmt: scl_odma's output format
 */
void sclr_odma_set_fmt(u8 inst, enum sclr_format fmt)
{
	u32 tmp = fmt << 8;

	if (fmt == SCL_FMT_BF16)
		tmp |= BIT(23);
	_reg_write_mask(reg_base + REG_SCL_ODMA_CFG(inst), 0x0080ff00, tmp);

	g_odma_cfg[inst].fmt = fmt;
}

/**
 * sclr_odma_set_mem - setup odma's mem settings.
 *
 * @param mem: mem settings for odma
 */
void sclr_odma_set_mem(u8 inst, struct sclr_mem *mem)
{
	_reg_write(reg_base + REG_SCL_ODMA_OFFSET_X(inst), mem->start_x);
	_reg_write(reg_base + REG_SCL_ODMA_OFFSET_Y(inst), mem->start_y);
	_reg_write(reg_base + REG_SCL_ODMA_WIDTH(inst), mem->width - 1);
	_reg_write(reg_base + REG_SCL_ODMA_HEIGHT(inst), mem->height - 1);
	_reg_write(reg_base + REG_SCL_ODMA_PITCH_Y(inst), mem->pitch_y);
	_reg_write(reg_base + REG_SCL_ODMA_PITCH_C(inst), mem->pitch_c);

	sclr_odma_set_addr(inst, mem->addr0, mem->addr1, mem->addr2);

	g_odma_cfg[inst].mem = *mem;
}

/**
 * sclr_odma_set_addr - setup odma's mem address.
 *
 * @param addr0: address of planar0
 * @param addr1: address of planar1
 * @param addr2: address of planar2
 */
void sclr_odma_set_addr(u8 inst, u64 addr0, u64 addr1, u64 addr2)
{
	_reg_write(reg_base + REG_SCL_ODMA_ADDR0_L(inst), addr0);
	_reg_write(reg_base + REG_SCL_ODMA_ADDR0_H(inst), addr0 >> 32);
	_reg_write(reg_base + REG_SCL_ODMA_ADDR1_L(inst), addr1);
	_reg_write(reg_base + REG_SCL_ODMA_ADDR1_H(inst), addr1 >> 32);
	_reg_write(reg_base + REG_SCL_ODMA_ADDR2_L(inst), addr2);
	_reg_write(reg_base + REG_SCL_ODMA_ADDR2_H(inst), addr2 >> 32);

	g_odma_cfg[inst].mem.addr0 = addr0;
	g_odma_cfg[inst].mem.addr1 = addr1;
	g_odma_cfg[inst].mem.addr2 = addr2;
}

union sclr_odma_dbg_status sclr_odma_get_dbg_status(u8 inst)
{
	union sclr_odma_dbg_status status;

	status.raw = _reg_read(reg_base + REG_SCL_ODMA_DBG(inst));

	return status;
}

void sclr_odma_get_sb_default(struct sclr_odma_sb_cfg *cfg)
{
	memset(cfg, 0, sizeof(*cfg));
	cfg->sb_nb = 3;
	cfg->sb_full_nb = 2;
}

void sclr_odma_set_sb(u8 inst, struct sclr_odma_sb_cfg *cfg)
{
	u32 val = 0;

	val |= (cfg->sb_mode << SC_ODMA_REG_SB_MODE_OFFSET) & SC_ODMA_REG_SB_MODE_MASK;
	val |= (cfg->sb_size << SC_ODMA_REG_SB_SIZE_OFFSET) & SC_ODMA_REG_SB_SIZE_MASK;
	val |= (cfg->sb_nb << SC_ODMA_REG_SB_NB_OFFSET) & SC_ODMA_REG_SB_NB_MASK;
	val |= (cfg->sb_full_nb << SC_ODMA_REG_SB_FULL_NB_OFFSET) & SC_ODMA_REG_SB_FULL_NB_MASK;
	val |= (cfg->sb_sw_wptr << SC_ODMA_REG_SB_SW_WPTR_OFFSET) & SC_ODMA_REG_SB_SW_WPTR_MASK;
	val |= (cfg->sb_set_str << SC_ODMA_REG_SB_SET_STR_OFFSET) & SC_ODMA_REG_SB_SET_STR_MASK;
	val |= (cfg->sb_sw_clr << SC_ODMA_REG_SB_SW_CLR_OFFSET) & SC_ODMA_REG_SB_SW_CLR_MASK;

	_reg_write(reg_base + REG_SCL_ODMA_SB_CTRL(inst), val);
}

/**
 * sclr_set_out_mode - Control sclr output's mode. CSC/Quantization/HSV
 *
 * @param inst: (0~3), the instance of scaler which want to be configured.
 * @param mode: csc/quant/hsv/none
 */
void sclr_set_out_mode(u8 inst, enum sclr_out_mode mode)
{
	u32 tmp = 0;

	switch (mode) {
	case SCL_OUT_CSC:
		tmp = BIT(0);
		break;
	case SCL_OUT_QUANT_BF16:
		tmp |= BIT(23);
	case SCL_OUT_QUANT:
		tmp |= BIT(1) | BIT(0);
		break;
	case SCL_OUT_HSV:
		tmp = BIT(4);
		break;
	case SCL_OUT_DISABLE:
		break;
	}
	_reg_write_mask(reg_base + REG_SCL_CSC_EN(inst), 0x01000013, tmp);

	g_odma_cfg[inst].csc_cfg.mode = mode;
}

/**
 * sclr_set_csc_ctrl - configure scl's output CSC's, including hsv/quant/coefficient/offset
 *
 * @param inst: (0~3), the instance of scaler which want to be configured.
 * @param cfg: The settings for CSC
 */
void sclr_set_csc_ctrl(u8 inst, struct sclr_csc_cfg *cfg)
{
	u32 tmp = 0;

	sclr_set_out_mode(inst, cfg->mode);

	if (cfg->mode == SCL_OUT_HSV) {
		if (cfg->hsv_round == SCL_HSV_ROUNDING_TOWARD_ZERO)
			tmp |= BIT(5);
		if (!cfg->work_on_border)
			tmp |= BIT(9);

		sclr_set_csc(inst, &csc_mtrx[SCL_CSC_NONE]);
	} else if (cfg->mode == SCL_OUT_QUANT) {
		tmp |= (cfg->quant_round << 2);
		if (!cfg->work_on_border)
			tmp |= BIT(8);
		if (cfg->quant_gain_mode == SCL_QUANT_GAIN_MODE_2)
			tmp |= BIT(11);

		sclr_set_quant(inst, &cfg->quant_form);
	} else if (cfg->mode == SCL_OUT_QUANT_BF16) {
		if (cfg->quant_border_type == SCL_QUANT_BORDER_TYPE_127)
			tmp |= BIT(10);
		if (!cfg->work_on_border)
			tmp |= BIT(8);
		if (cfg->quant_gain_mode == SCL_QUANT_GAIN_MODE_2)
			tmp |= BIT(11);

		sclr_set_quant(inst, &cfg->quant_form);
	} else {
		tmp |= BIT(8);

		sclr_set_csc(inst, &csc_mtrx[cfg->csc_type]);
	}
	_reg_write_mask(reg_base + REG_SCL_CSC_EN(inst), 0x00000fec, tmp);

	g_odma_cfg[inst].csc_cfg = *cfg;
}

struct sclr_csc_cfg *sclr_get_csc_ctrl(u8 inst)
{
	if (inst < SCL_MAX_INST)
		return &g_odma_cfg[inst].csc_cfg;

	return NULL;
}

/**
 * sclr_set_csc - configure scl's output CSC's coefficient/offset
 *
 * @param inst: (0~3), the instance of scaler which want to be configured.
 * @param cfg: The settings for CSC
 */
void sclr_set_csc(u8 inst, struct sclr_csc_matrix *cfg)
{
	_reg_write(reg_base + REG_SCL_CSC_COEF0(inst),
		   (cfg->coef[0][1] << 16) | cfg->coef[0][0]);
	_reg_write(reg_base + REG_SCL_CSC_COEF1(inst),
		   (cfg->coef[1][0] << 16) | cfg->coef[0][2]);
	_reg_write(reg_base + REG_SCL_CSC_COEF2(inst),
		   (cfg->coef[1][2] << 16) | cfg->coef[1][1]);
	_reg_write(reg_base + REG_SCL_CSC_COEF3(inst),
		   (cfg->coef[2][1] << 16) | cfg->coef[2][0]);
	_reg_write(reg_base + REG_SCL_CSC_COEF4(inst), cfg->coef[2][2]);

	_reg_write(reg_base + REG_SCL_CSC_OFFSET(inst),
		   (cfg->add[2] << 16) | (cfg->add[1] << 8) | cfg->add[0]);
	_reg_write(reg_base + REG_SCL_CSC_FRAC0(inst), 0);
	_reg_write(reg_base + REG_SCL_CSC_FRAC1(inst), 0);
}

/**
 * sclr_set_quant - set quantization by csc module
 *
 * @param inst: (0~3), the instance of scaler which want to be configured.
 * @param cfg: The settings of quantization
 */
void sclr_set_quant(u8 inst, struct sclr_quant_formula *cfg)
{
	_reg_write(reg_base + REG_SCL_CSC_COEF0(inst), cfg->sc_frac[0]);
	_reg_write(reg_base + REG_SCL_CSC_COEF1(inst), 0);
	_reg_write(reg_base + REG_SCL_CSC_COEF2(inst), cfg->sc_frac[1]);
	_reg_write(reg_base + REG_SCL_CSC_COEF3(inst), 0);
	_reg_write(reg_base + REG_SCL_CSC_COEF4(inst), cfg->sc_frac[2]);

	_reg_write(reg_base + REG_SCL_CSC_OFFSET(inst), (cfg->sub[2] << 16) | (cfg->sub[1] << 8) | cfg->sub[0]);
	_reg_write(reg_base + REG_SCL_CSC_FRAC0(inst), (cfg->sub_frac[1] << 16) | cfg->sub_frac[0]);
	_reg_write(reg_base + REG_SCL_CSC_FRAC1(inst), cfg->sub_frac[2]);
}

/**
 * sclr_get_csc - get current CSC's coefficient/offset
 *
 * @param inst: (0~3), the instance of scaler which want to be configured.
 * @param cfg: The settings of CSC
 */
void sclr_get_csc(u8 inst, struct sclr_csc_matrix *cfg)
{
	u32 tmp;

	tmp = _reg_read(reg_base + REG_SCL_CSC_COEF0(inst));
	cfg->coef[0][0] = tmp;
	cfg->coef[0][1] = tmp >> 16;
	tmp = _reg_read(reg_base + REG_SCL_CSC_COEF1(inst));
	cfg->coef[0][2] = tmp;
	cfg->coef[1][0] = tmp >> 16;
	tmp = _reg_read(reg_base + REG_SCL_CSC_COEF2(inst));
	cfg->coef[1][1] = tmp;
	cfg->coef[1][2] = tmp >> 16;
	tmp = _reg_read(reg_base + REG_SCL_CSC_COEF3(inst));
	cfg->coef[2][0] = tmp;
	cfg->coef[2][1] = tmp >> 16;
	tmp = _reg_read(reg_base + REG_SCL_CSC_COEF4(inst));
	cfg->coef[2][2] = tmp;

	tmp = _reg_read(reg_base + REG_SCL_CSC_OFFSET(inst));
	cfg->add[0] = tmp;
	cfg->add[1] = tmp >> 8;
	cfg->add[2] = tmp >> 16;
	memset(cfg->sub, 0, sizeof(cfg->sub));
}

void sclr_core_checksum_en(u8 inst, bool enable)
{
	_reg_write_mask(reg_base + REG_SCL_CHECKSUM0(inst), BIT(31),
	                enable ? BIT(31) : 0);
}

void sclr_core_get_checksum_status(u8 inst, struct sclr_core_checksum_status *status)
{
	status->checksum_base.raw = _reg_read(reg_base + REG_SCL_CHECKSUM0(inst));
	status->axi_read_gop0_data = _reg_read(reg_base + REG_SCL_CHECKSUM1(inst));
	status->axi_read_gop1_data = _reg_read(reg_base + REG_SCL_CHECKSUM2(inst));
	status->axi_write_data = _reg_read(reg_base + REG_SCL_CHECKSUM3(inst));
}

/****************************************************************************
 * SCALER Compression(OSD Encoder)
 ****************************************************************************/
/**
 * sclr_oenc_set_cfg - set compression configurations.
 *
 * @param oenc_cfg: compression's settings.
 */
void sclr_oenc_set_cfg(struct sclr_oenc_cfg *oenc_cfg)
{
	//reset
	_reg_write_mask(reg_base + REG_SCL_TOP_OENC_RST, BIT(0), 1);
	_reg_write_mask(reg_base + REG_SCL_TOP_OENC_RST, BIT(0), 0);

	//Compression Format
	//4'b0000: ARGB8888
	//4'b0100: ARGB4444
	//4'b0101: ARGB1555
	//4'b1000: 256LUT-ARGB4444
	//4'b1010: 16-LUT-ARGB4444
	static u8 reg_map_fmt[SCL_GOP_FMT_MAX] = {0, 0x4, 0x5, 0x8, 0xa};

	//reset
	_reg_write_mask(reg_base + REG_SCL_TOP_OENC_RST, BIT(0), 1);
	_reg_write_mask(reg_base + REG_SCL_TOP_OENC_RST, BIT(0), 0);

	oenc_cfg->cfg.b.fmt = reg_map_fmt[oenc_cfg->fmt];
	oenc_cfg->cfg.b.intr_en = 1;
	_reg_write(reg_base + REG_SCL_TOP_OENC_CFG,oenc_cfg->cfg.raw);
	_reg_write(reg_base + REG_SCL_TOP_OENC_RANGE,
	           ((oenc_cfg->src_picture_size.h - 1) << 16) | (oenc_cfg->src_picture_size.w - 1));
	_reg_write(reg_base + REG_SCL_TOP_OENC_PITCH, oenc_cfg->src_pitch);
	_reg_write(reg_base + REG_SCL_TOP_OENC_SRC_ADDR, oenc_cfg->src_adr);
	_reg_write(reg_base + REG_SCL_TOP_OENC_BSO_ADDR, oenc_cfg->bso_adr);

	if (oenc_cfg->cfg.b.wprot_en) {
		_reg_write(reg_base + REG_SCL_TOP_OENC_WPROT_LADDR, oenc_cfg->wprot_laddr);
		_reg_write(reg_base + REG_SCL_TOP_OENC_WPROT_UADDR, oenc_cfg->wprot_uaddr);
	}

	if (oenc_cfg->cfg.b.limit_bsz_en)
		_reg_write(reg_base + REG_SCL_TOP_OENC_LIMIT_BSZ, oenc_cfg->limit_bsz);

	//sclr_oenc_trig
	_reg_write_mask(reg_base + REG_SCL_TOP_OENC_INT_GO, BIT(0), 1);
}

/**
 * sclr_oenc_get_cfg - set compression configurations.
 *
 * @return oenc_cfg: compression's settings.
 *
 */
struct sclr_oenc_cfg *sclr_oenc_get_cfg(void)
{
	struct sclr_oenc_int oenc_trig;

	oenc_trig.go_intr.raw = _reg_read(reg_base + REG_SCL_TOP_OENC_INT_GO);

	if (oenc_trig.go_intr.b.done)
		uartlog("[bm-vip][sc] SCLR OSD Compression done!!\n");

	uartlog("[bm-vip][sc] SCLR OSD Compression INTR vector:");
	if (oenc_trig.go_intr.b.intr_vec & BIT(0))
		uartlog("Successful!!\n");
	if (oenc_trig.go_intr.b.intr_vec & BIT(2))
		uartlog("Fail, Bitstream size great than limiter!!\n");
	if (oenc_trig.go_intr.b.intr_vec & BIT(3))
		uartlog("Fail, Watch Dog time-out!!\n");
	if (oenc_trig.go_intr.b.intr_vec & BIT(4))
		uartlog("Fail, Out of Dram Write protection region!!\n");

	g_oenc_cfg.cfg.raw = _reg_read(reg_base + REG_SCL_TOP_OENC_CFG);
	g_oenc_cfg.bso_adr = _reg_read(reg_base + REG_SCL_TOP_OENC_BSO_ADDR);
	g_oenc_cfg.bso_sz  = _reg_read(reg_base + REG_SCL_TOP_OENC_BSO_SZ) + 1; //count from 0
	g_oenc_cfg.bso_mem_size.w = ALIGN(g_oenc_cfg.bso_sz, 16) & 0x3fff;
	g_oenc_cfg.bso_mem_size.h = ALIGN(g_oenc_cfg.bso_sz, 16) >> 14;

	return &g_oenc_cfg;
}

/****************************************************************************
 * SCALER GOP
 ****************************************************************************/
/**
 * sclr_gop_set_cfg - configure gop
 *
 * @param inst: (0~3), the instance of gop which want to be configured.
 *		0~3 is on scl, 4 is on disp.
 * @param layer: (0~1) 0 is layer 0(gop0). 1 is layer 1(gop1).
 * @param cfg: gop's settings
 */
void sclr_gop_set_cfg(u8 inst, u8 layer, struct sclr_gop_cfg *cfg)
{
	if (inst < SCL_MAX_INST) {
		if (layer == 0) {
			_reg_write(reg_base + REG_SCL_GOP0_CFG(inst), cfg->gop_ctrl.raw);
			_reg_write(reg_base + REG_SCL_GOP0_FONTCOLOR(inst),
			           (cfg->font_fg_color << 16) | cfg->font_bg_color);
			if (cfg->gop_ctrl.b.colorkey_en)
			_reg_write(reg_base + REG_SCL_GOP0_COLORKEY(inst),
			           cfg->colorkey);
			_reg_write(reg_base + REG_SCL_GOP0_FONTBOX_CTRL(inst), cfg->fb_ctrl.raw);

		} else if (layer == 1) {
			_reg_write(reg_base + REG_SCL_GOP1_CFG(inst), cfg->gop_ctrl.raw);
			_reg_write(reg_base + REG_SCL_GOP1_FONTCOLOR(inst),
			           (cfg->font_fg_color << 16) | cfg->font_bg_color);
			if (cfg->gop_ctrl.b.colorkey_en)
			_reg_write(reg_base + REG_SCL_GOP1_COLORKEY(inst), cfg->colorkey);
			_reg_write(reg_base + REG_SCL_GOP1_FONTBOX_CTRL(inst), cfg->fb_ctrl.raw);

		} else
			uartlog("[bm-vip][sc] %s: only 0 or 1 layer, no such layer(%d). ", __func__, layer);

		g_gop_cfg[inst][layer] = *cfg;
	} else {
		_reg_write(reg_base + REG_SCL_DISP_GOP_CFG, cfg->gop_ctrl.raw);
		_reg_write(reg_base + REG_SCL_DISP_GOP_FONTCOLOR,
		           (cfg->font_fg_color << 16) | cfg->font_bg_color);
		if (cfg->gop_ctrl.b.colorkey_en)
			_reg_write(reg_base + REG_SCL_DISP_GOP_COLORKEY,cfg->colorkey);
		_reg_write(reg_base + REG_SCL_DISP_GOP_FONTBOX_CTRL, cfg->fb_ctrl.raw);

		// ECO item for threshold invert
		_reg_write_mask(reg_base + 0x90f8, 0x1, cfg->fb_ctrl.b.lo_thr_inv);

		g_disp_cfg.gop_cfg = *cfg;
	}
}

/**
 * sclr_gop_get_cfg - get gop's configurations.
 *
 * @param inst: (0~3), the instance of gop which want to be configured.
 *		0~3 is on scl, 4 is on disp.
 * @param layer: (0~1) 0 is layer 0(gop0). 1 is layer 1(gop1).
 */
struct sclr_gop_cfg *sclr_gop_get_cfg(u8 inst, u8 layer)
{
	if (inst < SCL_MAX_INST && layer < SCL_MAX_GOP_INST)
		return &g_gop_cfg[inst][layer];
	else if (inst == SCL_GOP_DISP)
		return &g_disp_cfg.gop_cfg;

	return NULL;
}

/**
 * sclr_gop_setup 256LUT - setup gop's Look-up table
 *
 * @param inst: (0~3), the instance of gop which want to be configured.
 *		0~3 is on scl, 4 is on disp.
 * @param layer: (0~1) 0 is layer 0(gop0). 1 is layer 1(gop1).
 * @param length: update 256LUT-table for index 0 ~ length.
 *		There should be smaller than 256 instances.
 * @param data: values of 256LUT-table. There should be 256 instances.
 */
int sclr_gop_setup_256LUT(u8 inst, u8 layer, u8 length, u16 *data)
{
	u16 i = 0;

	if (length > 255)
		return -1;

	if (inst < SCL_MAX_INST) {
		uartlog("[bm-vip][sc] update 256LUT in gop%d of sc_%d. Length is %d.\n", layer, inst, length);
		if (layer == 0) {
			for (i = 0; i < length; ++i) {
				_reg_write(reg_base + REG_SCL_GOP0_256LUT0(inst),
				           (i << 16) | *(data + i));
				_reg_write(reg_base + REG_SCL_GOP0_256LUT1(inst), ~BIT(16));
				_reg_write(reg_base + REG_SCL_GOP0_256LUT1(inst), BIT(16));
			}
		} else if (layer == 1) {
			for (i = 0; i < length; ++i) {
				_reg_write(reg_base + REG_SCL_GOP1_256LUT0(inst),
				           (i << 16) | *(data + i));
				_reg_write(reg_base + REG_SCL_GOP1_256LUT1(inst), ~BIT(16));
				_reg_write(reg_base + REG_SCL_GOP1_256LUT1(inst), BIT(16));
			}
		} else
			uartlog("[bm-vip][sc] %s: only 0 or 1 layer, no such layer(%d). ", __func__, layer);
	} else {
		uartlog("[bm-vip][sc] update 256LUT in gop1 of display. Length is %d.\n", layer, inst, length);
		for (i = 0; i < length; ++i) {
			_reg_write(reg_base + REG_SCL_DISP_GOP_256LUT0,
			           (i << 16) | *(data + i));
			_reg_write(reg_base + REG_SCL_DISP_GOP_256LUT1, ~BIT(16));
			_reg_write(reg_base + REG_SCL_DISP_GOP_256LUT1, BIT(16));
		}
	}
	return 0;
}

/**
 * sclr_gop_update_256LUT - update gop's Look-up table by index.
 *
 * @param inst: (0~3), the instance of gop which want to be configured.
 *		0~3 is on scl, 4 is on disp.
 * @param layer: (0~1) 0 is layer 0(gop0). 1 is layer 1(gop1).
 * @param index: start address of 256LUT-table. There should be 256 instances.
 * @param data: value of 256LUT-table.
 */
int sclr_gop_update_256LUT(u8 inst, u8 layer, u8 index, u16 data)
{
	if (index > 255)
		return -1;

	if (inst < SCL_MAX_INST) {
		uartlog("[bm-vip][sc] update 256LUT in gop%d of sc_%d. Index is %d.\n", layer, inst, index);
		if (layer == 0) {
			_reg_write(reg_base + REG_SCL_GOP0_256LUT0(inst),
			           (index << 16) | data);
			_reg_write(reg_base + REG_SCL_GOP0_256LUT1(inst), ~BIT(16));
			_reg_write(reg_base + REG_SCL_GOP0_256LUT1(inst), BIT(16));
		} else if (layer == 1) {
			_reg_write(reg_base + REG_SCL_GOP1_256LUT0(inst),
			           (index << 16) | data);
			_reg_write(reg_base + REG_SCL_GOP1_256LUT1(inst), ~BIT(16));
			_reg_write(reg_base + REG_SCL_GOP1_256LUT1(inst), BIT(16));
		} else
			uartlog("[bm-vip][sc] %s: only 0 or 1 layer, no such layer(%d). ", __func__, layer);
	} else {
		uartlog("[bm-vip][sc] update 256LUT in gop1 of display. Index is %d.\n", layer, inst, index);
		_reg_write(reg_base + REG_SCL_DISP_GOP_256LUT0,
		           (index << 16) | data);
		_reg_write(reg_base + REG_SCL_DISP_GOP_256LUT1, ~BIT(16));
		_reg_write(reg_base + REG_SCL_DISP_GOP_256LUT1, BIT(16));
	}
	return 0;
}

/**
 * sclr_gop_setup 16LUT - setup gop's Look-up table
 *
 * @param inst: (0~3), the instance of gop which want to be configured.
 *		0~3 is on scl, 4 is on disp.
 * @param layer: (0~1) 0 is layer 0(gop0). 1 is layer 1(gop1).
 * @param length: update 16LUT-table for index 0 ~ length.
 *		There should be smaller than 16 instances.
 * @param data: values of 16LUT-table. There should be 16 instances.
 */
int sclr_gop_setup_16LUT(u8 inst, u8 layer, u8 length, u16 *data)
{
	u16 i = 0;
	if (length > 15)
		return -1;

	if (inst < SCL_MAX_INST) {
		uartlog("[bm-vip][sc] update 16LUT in gop%d of sc_%d. Length is %d.\n", layer, inst, length);
		if (layer == 0) {
			for (i = 0; i < length; i += 2 ) {
				_reg_write(reg_base + REG_SCL_GOP0_16LUT(inst, i),
						   ((*(data + i + 1) << 16) | (*(data + i))));
			}
		} else if (layer == 1) {
			for (i = 0; i <= length; i += 2) {
				_reg_write(reg_base + REG_SCL_GOP1_16LUT(inst, i),
				           ((*(data + i + 1) << 16) | (*(data + i))));
			}
		} else
			uartlog("[bm-vip][sc] %s: only 0 or 1 layer, no such layer(%d). ", __func__, layer);
	} else {
		uartlog("[bm-vip][sc] update 16LUT in gop1 of display. Length is %d.\n", length);
		for (i = 0; i <= length; i += 2) {
			_reg_write(reg_base + REG_SCL_DISP_GOP_16LUT(i),
			           ((*(data + i + 1) << 16) | (*(data + i))));
		}
	}
	return 0;
}

/**
 * sclr_gop_update_16LUT - update gop's Look-up table by index.
 *
 * @param inst: (0~3), the instance of gop which want to be configured.
 *		0~3 is on scl, 4 is on disp.
 * @param layer: (0~1) 0 is layer 0(gop0). 1 is layer 1(gop1).
 * @param index: start address of 16LUT-table. There should be 16 instances.
 * @param data: value of 16LUT-table.
 */
int sclr_gop_update_16LUT(u8 inst, u8 layer, u8 index, u16 data)
{
	u16 tmp;
	if (index > 15)
		return -1;

	if (inst < SCL_MAX_INST) {
		uartlog("[bm-vip][sc] update 16LUT in gop%d of sc_%d. Index is %d.\n", layer, inst, index);
		if (layer == 0) {
			if (index % 2 == 0) {
				tmp = _reg_read(reg_base + REG_SCL_GOP0_16LUT(inst, index + 1));
				_reg_write(reg_base + REG_SCL_GOP0_16LUT(inst, index),
				           ((tmp << 16) | data));
			} else {
				tmp = _reg_read(reg_base + REG_SCL_GOP0_16LUT(inst, index - 1));
				_reg_write(reg_base + REG_SCL_GOP0_16LUT(inst, index - 1),
				           ((data << 16) | tmp));
			}
		} else if (layer == 1) {
			if (index % 2 == 0) {
				tmp = _reg_read(reg_base + REG_SCL_GOP1_16LUT(inst, index + 1));
				_reg_write(reg_base + REG_SCL_GOP1_16LUT(inst, index),
				           ((tmp << 16) | data));
			} else {
				tmp = _reg_read(reg_base + REG_SCL_GOP1_16LUT(inst, index - 1));
				_reg_write(reg_base + REG_SCL_GOP1_16LUT(inst, index - 1),
				           ((data << 16) | tmp));
			}
		} else
			uartlog("[bm-vip][sc] %s: only 0 or 1 layer, no such layer(%d). ", __func__, layer);
	} else {
		uartlog("[bm-vip][sc] update 16LUT in gop1 of display. Index is %d.\n", index);
		if (index % 2 == 0) {
			tmp = _reg_read(reg_base + REG_SCL_DISP_GOP_16LUT(index + 1));
			_reg_write(reg_base + REG_SCL_DISP_GOP_16LUT(index),
			           ((tmp << 16) | data));
		} else {
			tmp = _reg_read(reg_base + REG_SCL_DISP_GOP_16LUT(index - 1));
			_reg_write(reg_base + REG_SCL_DISP_GOP_16LUT(index - 1),
			           ((data << 16) | tmp));
		}
	}
	return 0;
}

/**
 * sclr_gop_ow_set_cfg - set gop's osd-window configurations.
 *
 * @param inst: (0~3), the instance of gop which want to be configured.
 *		0~3 is on scl, 4 is on disp.
 * @param layer: (0~1) 0 is layer 0(gop0). 1 is layer 1(gop1).
 * @param ow_inst: (0~7), the instance of ow which want to be configured.
 * @param cfg: ow's settings.
 */
void sclr_gop_ow_set_cfg(u8 inst, u8 layer, u8 ow_inst, struct sclr_gop_ow_cfg *ow_cfg)
{
	//OW Format
	//4'b0000: ARGB8888
	//4'b0100: ARGB4444
	//4'b0101: ARGB1555
	//4'b1000: 256LUT-ARGB4444
	//4'b1010: 16-LUT-ARGB4444
	//4'b1100: Font-base"
	static const u8 reg_map_fmt[SCL_GOP_FMT_MAX] = {0, 0x4, 0x5, 0x8, 0xa, 0xc};

	if (ow_inst >= SCL_MAX_GOP_OW_INST)
		return;

	if (inst < SCL_MAX_INST) {
		if (layer == 0) {
			_reg_write(reg_base + REG_SCL_GOP0_FMT(inst, ow_inst),
			           reg_map_fmt[ow_cfg->fmt]);
			_reg_write(reg_base + REG_SCL_GOP0_H_RANGE(inst, ow_inst),
			           (ow_cfg->end.x << 16) | ow_cfg->start.x);
			_reg_write(reg_base + REG_SCL_GOP0_V_RANGE(inst, ow_inst),
			           (ow_cfg->end.y << 16) | ow_cfg->start.y);
			_reg_write(reg_base + REG_SCL_GOP0_ADDR_L(inst, ow_inst),
			           ow_cfg->addr);
			_reg_write(reg_base + REG_SCL_GOP0_ADDR_H(inst, ow_inst),
			           ow_cfg->addr >> 32);
			_reg_write(reg_base + REG_SCL_GOP0_CROP_PITCH(inst, ow_inst),
			           (ow_cfg->crop_pixels << 16) | ow_cfg->pitch);
			_reg_write(reg_base + REG_SCL_GOP0_SIZE(inst, ow_inst),
			           (ow_cfg->mem_size.h << 16) | ow_cfg->mem_size.w);
			g_gop_cfg[inst][layer].ow_cfg[ow_inst] = *ow_cfg;
		} else if (layer == 1) {
			_reg_write(reg_base + REG_SCL_GOP1_FMT(inst, ow_inst),
			           reg_map_fmt[ow_cfg->fmt]);
			_reg_write(reg_base + REG_SCL_GOP1_H_RANGE(inst, ow_inst),
			           (ow_cfg->end.x << 16) | ow_cfg->start.x);
			_reg_write(reg_base + REG_SCL_GOP1_V_RANGE(inst, ow_inst),
			           (ow_cfg->end.y << 16) | ow_cfg->start.y);
			_reg_write(reg_base + REG_SCL_GOP1_ADDR_L(inst, ow_inst),
			           ow_cfg->addr);
			_reg_write(reg_base + REG_SCL_GOP1_ADDR_H(inst, ow_inst),
			           ow_cfg->addr >> 32);
			_reg_write(reg_base + REG_SCL_GOP1_CROP_PITCH(inst, ow_inst),
			           (ow_cfg->crop_pixels << 16) | ow_cfg->pitch);
			_reg_write(reg_base + REG_SCL_GOP1_SIZE(inst, ow_inst),
			           (ow_cfg->mem_size.h << 16) | ow_cfg->mem_size.w);
			g_gop_cfg[inst][layer].ow_cfg[ow_inst] = *ow_cfg;
		} else
			uartlog("[bm-vip][sc] %s: only 0 or 1 layer, no such layer(%d). ", __func__, layer);

			g_gop_cfg[inst][layer].ow_cfg[ow_inst] = *ow_cfg;

	} else {
		_reg_write(reg_base + REG_SCL_DISP_GOP_FMT(ow_inst),
		           reg_map_fmt[ow_cfg->fmt]);
		_reg_write(reg_base + REG_SCL_DISP_GOP_H_RANGE(ow_inst),
		           (ow_cfg->end.x << 16) | ow_cfg->start.x);
		_reg_write(reg_base + REG_SCL_DISP_GOP_V_RANGE(ow_inst),
		           (ow_cfg->end.y << 16) | ow_cfg->start.y);
		_reg_write(reg_base + REG_SCL_DISP_GOP_ADDR_L(ow_inst),
		           ow_cfg->addr);
		_reg_write(reg_base + REG_SCL_DISP_GOP_ADDR_H(ow_inst),
		           ow_cfg->addr >> 32);
		_reg_write(reg_base + REG_SCL_DISP_GOP_CROP_PITCH(ow_inst),
		           (ow_cfg->crop_pixels << 16) | ow_cfg->pitch);
		_reg_write(reg_base + REG_SCL_DISP_GOP_SIZE(ow_inst),
		           (ow_cfg->mem_size.h << 16) | ow_cfg->mem_size.w);

		g_disp_cfg.gop_cfg.ow_cfg[ow_inst] = *ow_cfg;
	}
}

/**
 * sclr_gop_fb_set_cfg - setup fontbox
 *
 * @param inst: (0~3), the instance of gop which want to be configured.
 *		0~3 is on scl, 4 is on disp.
 * @param layer: (0~1) 0 is layer 0(gop0). 1 is layer 1(gop1).
 * @param fb_inst: (0~1), the instance of ow which want to be configured.
 * @param cfg: fontbox configuration
 */
void sclr_gop_fb_set_cfg(u8 inst, u8 layer, u8 fb_inst, struct sclr_gop_fb_cfg *fb_cfg)
{
	if (fb_inst >= SCL_MAX_GOP_FB_INST)
		return;

	if (fb_cfg->fb_ctrl.b.sample_rate > 3)
		fb_cfg->fb_ctrl.b.sample_rate = 3;
	if (fb_cfg->fb_ctrl.b.pix_thr > 31)
		fb_cfg->fb_ctrl.b.pix_thr = 31;

	if (inst < SCL_MAX_INST) {
		if (layer == 0) {
			_reg_write(reg_base + REG_SCL_GOP0_FONTBOX_CFG(inst, fb_inst), fb_cfg->fb_ctrl.raw);
			_reg_write(reg_base + REG_SCL_GOP0_FONTBOX_INIT(inst, fb_inst), fb_cfg->init_st);
		} else if (layer == 1) {
			_reg_write(reg_base + REG_SCL_GOP1_FONTBOX_CFG(inst, fb_inst), fb_cfg->fb_ctrl.raw);
			_reg_write(reg_base + REG_SCL_GOP1_FONTBOX_INIT(inst, fb_inst), fb_cfg->init_st);
		} else
			uartlog("[bm-vip][sc] %s: only 0 or 1 layer, no such layer(%d). ", __func__, layer);
	} else {
		_reg_write(reg_base + REG_SCL_DISP_GOP_FONTBOX_CFG(fb_inst), fb_cfg->fb_ctrl.raw);
		_reg_write(reg_base + REG_SCL_DISP_GOP_FONTBOX_INIT(fb_inst), fb_cfg->init_st);
	}
}

/**
 * sclr_gop_fb_get_record - get fontbox's record
 *
 * @param inst: (0~3), the instance of gop which want to be configured.
 *		0~3 is on scl, 4 is on disp.
 * @param layer: (0~1) 0 is layer 0(gop0). 1 is layer 1(gop1).
 * @param fb_inst: (0~1), the instance of ow which want to be configured.
 * @return: fontbox's record
 */
u32 sclr_gop_fb_get_record(u8 inst, u8 layer, u8 fb_inst)
{
	if (inst < SCL_MAX_INST) {
		if (layer == 0)
			return _reg_read(reg_base + REG_SCL_GOP0_FONTBOX_REC(inst, fb_inst));
		else if (layer == 1)
			return _reg_read(reg_base + REG_SCL_GOP1_FONTBOX_REC(inst, fb_inst));
		else {
			uartlog("[bm-vip][sc] %s: only 0 or 1 layer, no such layer(%d). ", __func__, layer);
			return -1;
		}
	} else
		return _reg_read(reg_base + REG_SCL_DISP_GOP_FONTBOX_REC(fb_inst));
}

/**
 * sclr_gop_odec_set_cfg_from_oenc - setup odec
 *
 * @param inst: (0~3), the instance of gop which want to be configured.
 *		0~3 is on scl, 4 is on disp.
 * @param layer: (0~1) 0 is layer 0(gop0). 1 is layer 1(gop1).
 * @param cfg: odec_cfg configuration
 */
void sclr_gop_odec_set_cfg_from_oenc(u8 inst, u8 layer, struct sclr_gop_odec_cfg *odec_cfg)
{
	struct sclr_oenc_cfg *oenc_cfg = sclr_oenc_get_cfg();

	if (inst < SCL_MAX_INST) {
		if (layer == 0) {
			if (oenc_cfg->bso_sz != 0) {
				_reg_write(reg_base + REG_SCL_GOP0_DEC_CTRL(inst), odec_cfg->odec_ctrl.raw);
				_reg_write(reg_base + REG_SCL_GOP0_ADDR_L(inst, odec_cfg->odec_ctrl.b.odec_attached_idx),
				           oenc_cfg->bso_adr);
				_reg_write(reg_base + REG_SCL_GOP0_ADDR_H(inst, odec_cfg->odec_ctrl.b.odec_attached_idx),
				           oenc_cfg->bso_adr >> 32);
				_reg_write(reg_base + REG_SCL_GOP0_SIZE(inst, odec_cfg->odec_ctrl.b.odec_attached_idx),
				           (oenc_cfg->bso_mem_size.h << 16) | oenc_cfg->bso_mem_size.w);
			}
		} else if (layer == 1) {
			if (oenc_cfg->bso_sz != 0) {
				_reg_write(reg_base + REG_SCL_GOP1_DEC_CTRL(inst), odec_cfg->odec_ctrl.raw);
				_reg_write(reg_base + REG_SCL_GOP1_ADDR_L(inst, odec_cfg->odec_ctrl.b.odec_attached_idx),
				           oenc_cfg->bso_adr);
				_reg_write(reg_base + REG_SCL_GOP1_ADDR_H(inst, odec_cfg->odec_ctrl.b.odec_attached_idx),
				           oenc_cfg->bso_adr >> 32);
				_reg_write(reg_base + REG_SCL_GOP1_SIZE(inst, odec_cfg->odec_ctrl.b.odec_attached_idx),
				           (oenc_cfg->bso_mem_size.h << 16) | oenc_cfg->bso_mem_size.w);
			}
		} else {
			uartlog("[bm-vip][sc] %s: only 0 or 1 layer, no such layer(%d). ", __func__, layer);
		}
	} else {
		if (oenc_cfg->bso_sz != 0) {
			_reg_write(reg_base + REG_SCL_DISP_GOP_DEC_CTRL, odec_cfg->odec_ctrl.raw);
			_reg_write(reg_base + REG_SCL_DISP_GOP_ADDR_L(odec_cfg->odec_ctrl.b.odec_attached_idx),
			           oenc_cfg->bso_adr);
			_reg_write(reg_base + REG_SCL_DISP_GOP_ADDR_H(odec_cfg->odec_ctrl.b.odec_attached_idx),
			           oenc_cfg->bso_adr >> 32);
			_reg_write(reg_base + REG_SCL_DISP_GOP_SIZE(odec_cfg->odec_ctrl.b.odec_attached_idx),
			           (oenc_cfg->bso_mem_size.h << 16) | oenc_cfg->bso_mem_size.w);
		}
	}
}

/**
 * sclr_cover_set_cfg - setup cover
 *
 * @param inst: (0~3), the instance of gop which want to be configured.
 *		0~3 is on scl, 4 is on disp.
 * @param cover_cfg: cover_cfg configuration
 */
void sclr_cover_set_cfg(u8 inst, u8 cover_w_inst, struct sclr_cover_cfg *cover_cfg)
{
	if (inst < SCL_MAX_INST) {
		_reg_write(reg_base + REG_SCL_COVER_CFG(inst, cover_w_inst), cover_cfg->start.raw);
		_reg_write(reg_base + REG_SCL_COVER_SIZE(inst, cover_w_inst),
		           (cover_cfg->img_size.h << 16) | cover_cfg->img_size.w);
		_reg_write(reg_base + REG_SCL_COVER_COLOR(inst, cover_w_inst), cover_cfg->color.raw);
	} else {
		_reg_write(reg_base + REG_SCL_DISP_COVER_CFG(cover_w_inst), cover_cfg->start.raw);
		_reg_write(reg_base + REG_SCL_DISP_COVER_SIZE(cover_w_inst),
		           (cover_cfg->img_size.h << 16) | cover_cfg->img_size.w);
		_reg_write(reg_base + REG_SCL_DISP_COVER_COLOR(cover_w_inst), cover_cfg->color.raw);
	}
}

/****************************************************************************
 * SCALER PRICAY MASK
 ****************************************************************************/
/**
 * sclr_pri_set_cfg - configure privacy mask
 *
 * @param inst: (0~3), the instance of gop which want to be configured.
 *		0~3 is on scl, 4 is on disp.
 * @param cfg: privacy mask's settings
 */
void sclr_pri_set_cfg(u8 inst, struct sclr_privacy_cfg *cfg)
{
	u16 grid_line_per_frame, grid_num_per_line;
	u16 w, h;
	u16 pitch;

	if (inst < SCL_MAX_INST) {
		_reg_write(reg_base + REG_SCL_PRI_CFG(inst), cfg->cfg.raw);

		if (cfg->cfg.b.fit_picture) {
			w = g_sc_cfg[inst].sc.dst.w;
			h = g_sc_cfg[inst].sc.dst.h;
		} else {
			_reg_write(reg_base + REG_SCL_PRI_START(inst), (cfg->start.y << 16) | cfg->start.x);
			_reg_write(reg_base + REG_SCL_PRI_END(inst), (cfg->end.y << 16) | cfg->end.x);
			w = cfg->end.x - cfg->start.x + 1;
			h = cfg->end.y - cfg->start.y + 1;
		}

		_reg_write(reg_base + REG_SCL_PRI_ALPHA(inst),
			   (cfg->map_cfg.alpha_factor << 8) | cfg->map_cfg.no_mask_idx);
		_reg_write(reg_base + REG_SCL_PRI_MAP_ADDR_L(inst), cfg->map_cfg.base);
		//_reg_write(reg_base + REG_SCL_PRI_MAP_ADDR_H(inst), cfg->map_cfg.base >> 32);

		if (cfg->cfg.b.mode == 0) {
			u8 grid_shift = cfg->cfg.b.grid_size ? 4 : 3;

			grid_num_per_line = UPPER(w, grid_shift);
			grid_line_per_frame = UPPER(h, grid_shift);
			_reg_write(reg_base + REG_SCL_PRI_GRID_CFG(inst),
				   (grid_num_per_line << 16) | (grid_line_per_frame - 1));
			pitch = ALIGN(grid_num_per_line, 16);
		} else {
			_reg_write(reg_base + REG_SCL_PRI_GRID_CFG(inst),
				   (w << 16) | (h - 1));
			pitch = ALIGN(w, 16);
		}

		_reg_write(reg_base + REG_SCL_PRI_MAP_AXI_CFG(inst), (cfg->map_cfg.axi_burst << 16) | pitch);
	} else {
		_reg_write(reg_base + REG_SCL_TOP_PRI_CFG, cfg->cfg.raw);

		if (cfg->cfg.b.fit_picture) {
			w = g_img_cfg[SCL_IMG_V].mem.width;
			h = g_img_cfg[SCL_IMG_V].mem.height;
		} else {
			_reg_write(reg_base + REG_SCL_TOP_PRI_START, (cfg->start.y << 16) | cfg->start.x);
			_reg_write(reg_base + REG_SCL_TOP_PRI_END, (cfg->end.y << 16) | cfg->end.x);
			w = cfg->end.x - cfg->start.x + 1;
			h = cfg->end.y - cfg->start.y + 1;
		}
		_reg_write(reg_base + REG_SCL_TOP_PRI_SRC_SIZE,
			   ((g_img_cfg[SCL_IMG_V].mem.height - 1) << 16) | (g_img_cfg[SCL_IMG_V].mem.width - 1));

		_reg_write(reg_base + REG_SCL_TOP_PRI_ALPHA,
			   (cfg->map_cfg.alpha_factor << 8) | cfg->map_cfg.no_mask_idx);
		_reg_write(reg_base + REG_SCL_TOP_PRI_MAP_ADDR_L, cfg->map_cfg.base);

		if (cfg->cfg.b.mode == 0) {
			u8 grid_shift = cfg->cfg.b.grid_size ? 4 : 3;

			grid_num_per_line = UPPER(w, grid_shift);
			grid_line_per_frame = UPPER(h, grid_shift);
			_reg_write(reg_base + REG_SCL_TOP_PRI_GRID_CFG,
				   (grid_num_per_line << 16) | (grid_line_per_frame - 1));
			pitch = ALIGN(grid_num_per_line, 16);
		} else {
			_reg_write(reg_base + REG_SCL_PRI_GRID_CFG(inst),
				   (w << 16) | (h - 1));
			pitch = ALIGN(w, 16);
		}

		_reg_write(reg_base + REG_SCL_TOP_PRI_MAP_AXI_CFG, (cfg->map_cfg.axi_burst << 16) | pitch);
	}
}

/****************************************************************************
 * SCALER DISP
 ****************************************************************************/
void sclr_disp_set_srgb_ttl_en(u8 value)
{
	union  sclr_srgb_ctrl vo_srgb_ctrl;
	vo_srgb_ctrl.raw = _reg_read(reg_base + REG_SCL_DISP_SRGB_CTRL);
	vo_srgb_ctrl.b.srgb_ttl_en = value;

	_reg_write(reg_base + REG_SCL_DISP_SRGB_CTRL, vo_srgb_ctrl.raw);
}

void sclr_disp_set_srgb_ttl_4x(u8 value)
{
	union  sclr_srgb_ctrl vo_srgb_ctrl;
	vo_srgb_ctrl.raw = _reg_read(reg_base + REG_SCL_DISP_SRGB_CTRL);
	vo_srgb_ctrl.b.srgb_ttl_4t = value;

	_reg_write(reg_base + REG_SCL_DISP_SRGB_CTRL, vo_srgb_ctrl.raw);
}

/**
 * sclr_disp_reg_shadow_sel - control the read reg-bank.
 *
 * @param read_shadow: true(shadow); false(working)
 */
void sclr_disp_reg_shadow_sel(bool read_shadow)
{
	_reg_write_mask(reg_base + REG_SCL_DISP_CFG, BIT(18),
			(read_shadow ? 0x0 : BIT(18)));
}

/**
 * sclr_disp_reg_shadow_mask - reg won't be update by sw/hw until unmask.
 *
 * @param mask: true(mask); false(unmask)
 * @return: mask status before modification.
 */
bool sclr_disp_reg_shadow_mask(bool mask)
{
	bool is_masked = (_reg_read(reg_base + REG_SCL_DISP_CFG) & BIT(17));

	if (is_masked != mask)
		_reg_write_mask(reg_base + REG_SCL_DISP_CFG, BIT(17),
				(mask ? BIT(17) : 0));

	return is_masked;
}

/**
 * sclr_disp_reg_force_up - trigger reg update by sw.
 *
 */
void sclr_disp_reg_force_up(void)
{
	_reg_write_mask(reg_base + REG_SCL_DISP_CFG, BIT(16), BIT(16));
}

/**
 * sclr_disp_tgen - enable timing-generator on disp.
 *
 * @param enable: AKA.
 * @return: tgen's enable status before change.
 */
bool sclr_disp_tgen_enable(bool enable)
{
	bool is_enable = (_reg_read(reg_base + REG_SCL_DISP_CFG) & 0x80);

	if (is_enable != enable) {
		_reg_write_mask(reg_base + REG_SCL_DISP_CFG, 0x0080,
				enable ? 0x80 : 0x00);
		g_disp_cfg.tgen_en = enable;
	}

	return is_enable;
}

/**
 * sclr_disp_set_cfg - set disp's configurations.
 *
 * @param cfg: disp's settings.
 */
void sclr_disp_set_cfg(struct sclr_disp_cfg *cfg)
{
	u32 tmp = 0;
	bool is_masked = false;

	tmp |= cfg->disp_from_sc;
	tmp |= (cfg->fmt << 12);
	if (cfg->sync_ext)
		tmp |= BIT(4);
	if (cfg->tgen_en)
		tmp |= BIT(7);

	is_masked = sclr_disp_reg_shadow_mask(true);
	if (!cfg->disp_from_sc) {
		sclr_disp_set_mem(&cfg->mem);
		_reg_write_mask(reg_base + REG_SCL_DISP_PITCH_Y, 0xf0000000,
				cfg->burst << 28);

		sclr_disp_set_in_csc(cfg->in_csc);
	} else {
		// csc only needed if disp from dram
		sclr_set_out_mode(0, SCL_OUT_DISABLE);
	}

	sclr_disp_set_out_csc(cfg->out_csc);

	_reg_write_mask(reg_base + REG_SCL_DISP_CFG, 0x0000f09f, tmp);
	_reg_write_mask(reg_base + REG_SCL_DISP_CACHE, BIT(0), cfg->cache_mode);

	switch (cfg->out_bit) {
	case 6:
		tmp = 3 << 16;
		break;
	case 8:
		tmp = 2 << 16;
		break;
	default:
		tmp = 0;
		break;
	}
	tmp |= cfg->drop_mode << 18;
	_reg_write_mask(reg_base + REG_SCL_DISP_PAT_COLOR4, 0x000f0000, tmp);

	if (!is_masked)
		sclr_disp_reg_shadow_mask(false);

	g_disp_cfg = *cfg;
}

/**
 * sclr_disp_get_cfg - get scl_disp's cfg
 *
 * @return: scl_disp's cfg
 */
struct sclr_disp_cfg *sclr_disp_get_cfg(void)
{
	return &g_disp_cfg;
}

/**
 * sclr_disp_cfg_setup_from_reg - get settings from register.
 *
 */
void sclr_disp_cfg_setup_from_reg(void)
{
	u32 tmp = 0;

	tmp = _reg_read(reg_base + REG_SCL_DISP_CFG);
	g_disp_cfg.disp_from_sc = tmp & BIT(0);
	g_disp_cfg.sync_ext = tmp & BIT(4);
	g_disp_cfg.tgen_en = tmp & BIT(7);
	g_disp_cfg.fmt = (tmp >> 12) & 0xf;

	tmp = _reg_read(reg_base + REG_SCL_DISP_CACHE);
	g_disp_cfg.cache_mode = tmp & BIT(0);

	tmp = _reg_read(reg_base + REG_SCL_DISP_PAT_COLOR4);
	g_disp_cfg.out_bit = (tmp >> 16) & 0x3;
	g_disp_cfg.drop_mode = (tmp >> 18) & 0x3;

	tmp = _reg_read(reg_base + REG_SCL_DISP_PITCH_Y);
	g_disp_cfg.burst = (tmp >> 28) & 0xf;
}

/**
 * sclr_disp_set_timing - modify disp's timing-generator.
 *
 * @param timing: new timing of disp.
 */
void sclr_disp_set_timing(struct sclr_disp_timing *timing)
{
	u32 tmp = 0;
	bool is_enable = sclr_disp_tgen_enable(false);

	if (timing->vsync_pol)
		tmp |= 0x20;
	if (timing->hsync_pol)
		tmp |= 0x40;

	_reg_write_mask(reg_base + REG_SCL_DISP_CFG, 0x0060, tmp);
	_reg_write(reg_base + REG_SCL_DISP_TOTAL,
		   (timing->htotal << 16) | timing->vtotal);
	_reg_write(reg_base + REG_SCL_DISP_VSYNC,
		   (timing->vsync_end << 16) | timing->vsync_start);
	_reg_write(reg_base + REG_SCL_DISP_VFDE,
		   (timing->vfde_end << 16) | timing->vfde_start);
	_reg_write(reg_base + REG_SCL_DISP_VMDE,
		   (timing->vmde_end << 16) | timing->vmde_start);
	_reg_write(reg_base + REG_SCL_DISP_HSYNC,
		   (timing->hsync_end << 16) | timing->hsync_start);
	_reg_write(reg_base + REG_SCL_DISP_HFDE,
		   (timing->hfde_end << 16) | timing->hfde_start);
	_reg_write(reg_base + REG_SCL_DISP_HMDE,
		   (timing->hmde_end << 16) | timing->hmde_start);

	if (is_enable)
		sclr_disp_tgen_enable(true);
	disp_timing = *timing;
}

struct sclr_disp_timing *sclr_disp_get_timing(void)
{
	return &disp_timing;
}

void sclr_disp_get_hw_timing(struct sclr_disp_timing *timing)
{
	u32 tmp = 0;

	if (!timing)
		return;

	tmp = _reg_read(reg_base + REG_SCL_DISP_TOTAL);
	timing->htotal = (tmp >> 16) & 0xffff;
	timing->vtotal = tmp & 0xffff;
	tmp = _reg_read(reg_base + REG_SCL_DISP_VSYNC);
	timing->vsync_end = (tmp >> 16) & 0xffff;
	timing->vsync_start = tmp & 0xffff;
	tmp = _reg_read(reg_base + REG_SCL_DISP_VFDE);
	timing->vfde_end = (tmp >> 16) & 0xffff;
	timing->vfde_start = tmp & 0xffff;
	tmp = _reg_read(reg_base + REG_SCL_DISP_VMDE);
	timing->vmde_end = (tmp >> 16) & 0xffff;
	timing->vmde_start = tmp & 0xffff;
	tmp = _reg_read(reg_base + REG_SCL_DISP_HSYNC);
	timing->hsync_end = (tmp >> 16) & 0xffff;
	timing->hsync_start = tmp & 0xffff;
	tmp = _reg_read(reg_base + REG_SCL_DISP_HFDE);
	timing->hfde_end = (tmp >> 16) & 0xffff;
	timing->hfde_start = tmp & 0xffff;
	tmp = _reg_read(reg_base + REG_SCL_DISP_HMDE);
	timing->hmde_end = (tmp >> 16) & 0xffff;
	timing->hmde_start = tmp & 0xffff;
}

/**
 * sclr_disp_set_rect - setup rect(me) of disp
 *
 * @param rect: the pos/size of me, which should fit with disp's input.
 */
int sclr_disp_set_rect(struct sclr_rect rect)
{
	bool is_masked = sclr_disp_reg_shadow_mask(true);

	if ((rect.y > disp_timing.vfde_end) ||
	    (rect.x > disp_timing.hfde_end) ||
	    ((disp_timing.vfde_start + rect.y + rect.h - 1) >
	      disp_timing.vfde_end) ||
	    ((disp_timing.hfde_start + rect.x + rect.w - 1) >
	      disp_timing.hfde_end)) {
		uartlog("[bm-vip][sc] %s: me's pos(%d, %d) size(%d, %d) ",
		       __func__, rect.x, rect.y, rect.w, rect.h);
		uartlog(" out of range(%d, %d).\n",
			disp_timing.hfde_end, disp_timing.vfde_end);
		return -EINVAL;
	}

	disp_timing.vmde_start = rect.y + disp_timing.vfde_start;
	disp_timing.hmde_start = rect.x + disp_timing.hfde_start;
	disp_timing.vmde_end = disp_timing.vmde_start + rect.h - 1;
	disp_timing.hmde_end = disp_timing.hmde_start + rect.w - 1;

	_reg_write(reg_base + REG_SCL_DISP_HMDE,
		   (disp_timing.hmde_end << 16) | disp_timing.hmde_start);
	_reg_write(reg_base + REG_SCL_DISP_VMDE,
		   (disp_timing.vmde_end << 16) | disp_timing.vmde_start);

	if (!is_masked)
		sclr_disp_reg_shadow_mask(false);

	return 0;
}

/**
 * sclr_disp_set_mem - setup disp's mem settings. Only work if disp from mem.
 *
 * @param mem: mem settings for disp
 */
void sclr_disp_set_mem(struct sclr_mem *mem)
{
	bool is_masked = sclr_disp_reg_shadow_mask(true);

	_reg_write(reg_base + REG_SCL_DISP_OFFSET,
		   (mem->start_y << 16) | mem->start_x);
	_reg_write(reg_base + REG_SCL_DISP_SIZE,
		   ((mem->height - 1) << 16) | (mem->width - 1));
	_reg_write_mask(reg_base + REG_SCL_DISP_PITCH_Y, 0x00ffffff,
			mem->pitch_y);
	_reg_write(reg_base + REG_SCL_DISP_PITCH_C, mem->pitch_c);

	sclr_disp_set_addr(mem->addr0, mem->addr1, mem->addr2);
	if (!is_masked)
		sclr_disp_reg_shadow_mask(false);

	g_disp_cfg.mem = *mem;
}

/**
 * sclr_disp_set_addr - setup disp's mem address. Only work if disp from mem.
 *
 * @param addr0: address of planar0
 * @param addr1: address of planar1
 * @param addr2: address of planar2
 */
void sclr_disp_set_addr(u64 addr0, u64 addr1, u64 addr2)
{
	_reg_write(reg_base + REG_SCL_DISP_ADDR0_L, addr0);
	_reg_write(reg_base + REG_SCL_DISP_ADDR0_H, addr0 >> 32);
	_reg_write(reg_base + REG_SCL_DISP_ADDR1_L, addr1);
	_reg_write(reg_base + REG_SCL_DISP_ADDR1_H, addr1 >> 32);
	_reg_write(reg_base + REG_SCL_DISP_ADDR2_L, addr2);
	_reg_write(reg_base + REG_SCL_DISP_ADDR2_H, addr2 >> 32);

	g_disp_cfg.mem.addr0 = addr0;
	g_disp_cfg.mem.addr1 = addr1;
	g_disp_cfg.mem.addr2 = addr2;
}

/**
 * sclr_disp_set_csc - configure disp's input CSC's coefficient/offset
 *
 * @param cfg: The settings for CSC
 */
void sclr_disp_set_csc(struct sclr_csc_matrix *cfg)
{
	_reg_write(reg_base + REG_SCL_DISP_IN_CSC0, BIT(31) |
		   (cfg->coef[0][1] << 16) | (cfg->coef[0][0]));
	_reg_write(reg_base + REG_SCL_DISP_IN_CSC1,
		   (cfg->coef[1][0] << 16) | (cfg->coef[0][2]));
	_reg_write(reg_base + REG_SCL_DISP_IN_CSC2,
		   (cfg->coef[1][2] << 16) | (cfg->coef[1][1]));
	_reg_write(reg_base + REG_SCL_DISP_IN_CSC3,
		   (cfg->coef[2][1] << 16) | (cfg->coef[2][0]));
	_reg_write(reg_base + REG_SCL_DISP_IN_CSC4, (cfg->coef[2][2]));
	_reg_write(reg_base + REG_SCL_DISP_IN_CSC_SUB,
		   (cfg->sub[2] << 16) | (cfg->sub[1] << 8) |
		   cfg->sub[0]);
	_reg_write(reg_base + REG_SCL_DISP_IN_CSC_ADD,
		   (cfg->add[2] << 16) | (cfg->add[1] << 8) |
		   cfg->add[0]);
}

/**
 * sclr_disp_set_in_csc - setup disp's csc on input. Only work if disp from mem.
 *
 * @param csc: csc settings
 */
void sclr_disp_set_in_csc(enum sclr_csc csc)
{
	if (csc == SCL_CSC_NONE) {
		_reg_write(reg_base + REG_SCL_DISP_IN_CSC0, 0);
	} else if (csc < SCL_CSC_MAX) {
		sclr_disp_set_csc(&csc_mtrx[csc]);
	}

	g_disp_cfg.in_csc = csc;
}

/**
 * sclr_disp_set_out_csc - setup disp's csc on output.
 *
 * @param csc: csc settings
 */
void sclr_disp_set_out_csc(enum sclr_csc csc)
{
	if (csc == SCL_CSC_NONE) {
		_reg_write(reg_base + REG_SCL_DISP_OUT_CSC0, 0);
	} else if (csc < SCL_CSC_MAX) {
		struct sclr_csc_matrix *cfg = &csc_mtrx[csc];

		_reg_write(reg_base + REG_SCL_DISP_OUT_CSC0, BIT(31) |
			   (cfg->coef[0][1] << 16) | (cfg->coef[0][0]));
		_reg_write(reg_base + REG_SCL_DISP_OUT_CSC1,
			   (cfg->coef[1][0] << 16) | (cfg->coef[0][2]));
		_reg_write(reg_base + REG_SCL_DISP_OUT_CSC2,
			   (cfg->coef[1][2] << 16) | (cfg->coef[1][1]));
		_reg_write(reg_base + REG_SCL_DISP_OUT_CSC3,
			   (cfg->coef[2][1] << 16) | (cfg->coef[2][0]));
		_reg_write(reg_base + REG_SCL_DISP_OUT_CSC4, (cfg->coef[2][2]));
		_reg_write(reg_base + REG_SCL_DISP_OUT_CSC_SUB,
			   (cfg->sub[2] << 16) | (cfg->sub[1] << 8) |
			   cfg->sub[0]);
		_reg_write(reg_base + REG_SCL_DISP_OUT_CSC_ADD,
			   (cfg->add[2] << 16) | (cfg->add[1] << 8) |
			   cfg->add[0]);
	}

	g_disp_cfg.out_csc = csc;
}

/**
 * sclr_disp_set_pattern - setup disp's pattern generator.
 *
 * @param type: type of pattern
 * @param color: color of pattern. Only for Gradient/FULL type.
 */
void sclr_disp_set_pattern(enum sclr_disp_pat_type type,
			   enum sclr_disp_pat_color color, const u16 *rgb)
{
	switch (type) {
	case SCL_PAT_TYPE_OFF:
		_reg_write_mask(reg_base + REG_SCL_DISP_PAT_CFG, 0x16, 0);
		break;

	case SCL_PAT_TYPE_SNOW:
		_reg_write_mask(reg_base + REG_SCL_DISP_PAT_CFG, 0x16, 0x10);
		break;

	case SCL_PAT_TYPE_AUTO:
		_reg_write(reg_base + REG_SCL_DISP_PAT_COLOR0, 0x03ff03ff);
		_reg_write_mask(reg_base + REG_SCL_DISP_PAT_COLOR1, 0x000003ff, 0x3ff);
		_reg_write_mask(reg_base + REG_SCL_DISP_PAT_CFG, 0xff0016,
				0x780006);
		break;

	case SCL_PAT_TYPE_V_GRAD:
	case SCL_PAT_TYPE_H_GRAD:
	case SCL_PAT_TYPE_FULL: {
		if (color == SCL_PAT_COLOR_USR) {
			_reg_write(reg_base + REG_SCL_DISP_PAT_COLOR0, rgb[1] << 16 | rgb[0]);
			_reg_write_mask(reg_base + REG_SCL_DISP_PAT_COLOR1, 0x000003ff, rgb[2]);
			_reg_write_mask(reg_base + REG_SCL_DISP_PAT_CFG, 0x1f000016,
					(type << 27) | (SCL_PAT_COLOR_WHITE << 24) | 0x0002);
		} else {
			_reg_write(reg_base + REG_SCL_DISP_PAT_COLOR0, 0x03ff03ff);
			_reg_write_mask(reg_base + REG_SCL_DISP_PAT_COLOR1, 0x000003ff, 0x3ff);
			_reg_write_mask(reg_base + REG_SCL_DISP_PAT_CFG, 0x1f000016,
					(type << 27) | (color << 24) | 0x0002);
		}
		break;
	}
	default:
		uartlog("%s - unacceptiable pattern-type(%d)\n", __func__, type);
		break;
	}
}

void sclr_top_set_i80_if(u8 sw_mode, enum sclr_hw_mcu_format format)
{
	union  sclr_sc_disp_mcu_if_ctrl mcu_if_ctrl;
	union  sclr_sc_disp_hw_mcu_auto hw_mcu_auto;

	mcu_if_ctrl.raw = _reg_read(reg_base + REG_SCL_DISP_MCU_IF_CTRL);
	hw_mcu_auto.raw = _reg_read(reg_base + REG_SCL_DISP_HW_MCU_AUTO);

    //for sw mcu mode
	mcu_if_ctrl.b.i80_if_en = false;
	mcu_if_ctrl.b.i80_sw_mode_en = true;
#if 0
	if(sw_mode)
	{
		mcu_if_ctrl.b.i80_if_en = true;
		mcu_if_ctrl.b.i80_hw_if_en = false;
		hw_mcu_auto.b.mcu_565 = false;
	}
	else//hw mcu
	{
		mcu_if_ctrl.b.i80_if_en = false;
		mcu_if_ctrl.b.i80_hw_if_en = true;
		if(I80_HW_FORMAT_RGB565 == format)
		{
			hw_mcu_auto.b.mcu_565 = true;
		}
		else if(I80_HW_FORMAT_RGB888 == format)
		{
			hw_mcu_auto.b.mcu_565 = false;
		}
	}
#endif
	_reg_write(reg_base + REG_SCL_DISP_MCU_IF_CTRL, mcu_if_ctrl.raw);
	_reg_write(reg_base + REG_SCL_DISP_HW_MCU_AUTO, hw_mcu_auto.raw);

}

/**
 * sclr_disp_set_frame_bgcolro - setup disp frame(area outside mde)'s
 *				 background color.
 *
 * @param r: 10bit red value
 * @param g: 10bit green value
 * @param b: 10bit blue value
 */
void sclr_disp_set_frame_bgcolor(u16 r, u16 g, u16 b)
{
	_reg_write_mask(reg_base + REG_SCL_DISP_PAT_COLOR1, 0x0fff0000,
			r << 16);
	_reg_write(reg_base + REG_SCL_DISP_PAT_COLOR2, b << 16 | g);
}

/**
 * sclr_disp_set_window_bgcolro - setup disp window's background color.
 *
 * @param r: 10bit red value
 * @param g: 10bit green value
 * @param b: 10bit blue value
 */
void sclr_disp_set_window_bgcolor(u16 r, u16 g, u16 b)
{
	_reg_write(reg_base + REG_SCL_DISP_PAT_COLOR3, g << 16 | r);
	_reg_write_mask(reg_base + REG_SCL_DISP_PAT_COLOR4, 0x0fff, b);
}

/**
 * sclr_disp_enable_window_bgcolor - Use window bg-color to hide everything
 *				     including test-pattern.
 *
 * @param enable: enable window bgcolor or not.
 */
void sclr_disp_enable_window_bgcolor(bool enable)
{
	_reg_write_mask(reg_base + REG_SCL_DISP_PAT_CFG, 0x20, enable ? 0x20 : 0);
}

union sclr_disp_dbg_status sclr_disp_get_dbg_status(bool clr)
{
	union sclr_disp_dbg_status status;

	status.raw = _reg_read(reg_base + REG_SCL_DISP_DBG);

	if (clr) {
		status.b.err_fwr_clr = 1;
		status.b.err_erd_clr = 1;
		status.b.bw_fail_clr = 1;
		_reg_write(reg_base + REG_SCL_DISP_DBG, status.raw);
	}

	return status;
}

void sclr_disp_gamma_ctrl(bool enable, bool pre_osd)
{
	u32 value = 0;

	if (enable)
		value |= 0x04;
	if (pre_osd)
		value |= 0x08;
	_reg_write_mask(reg_base + REG_SCL_DISP_GAMMA_CTRL, 0x0C, value);
}

void sclr_disp_gamma_lut_update(const u8 *b, const u8 *g, const u8 *r)
{
	u8 i;
	u32 value;

	_reg_write_mask(reg_base + REG_SCL_DISP_GAMMA_CTRL, 0x03, 0x03);

	for (i = 0; i < 65; ++i) {
		value = *(b + i) | (*(g + i) << 8) | (*(r + i) << 16)
			| (i << 24) | 0x80000000;
		_reg_write(reg_base + REG_SCL_DISP_GAMMA_WR_LUT, value);
	}

	_reg_write_mask(reg_base + REG_SCL_DISP_GAMMA_CTRL, 0x03, 0x00);
}

void sclr_lvdstx_set(union sclr_lvdstx cfg)
{
	_reg_write(reg_base + REG_SCL_TOP_LVDSTX, cfg.raw);
}

void sclr_lvdstx_get(union sclr_lvdstx *cfg)
{
	cfg->raw = _reg_read(reg_base + REG_SCL_TOP_LVDSTX);
}

void sclr_bt_set(union sclr_bt_enc enc, union sclr_bt_sync_code sync)
{
	_reg_write(reg_base + REG_SCL_TOP_BT_ENC, enc.raw);
	_reg_write(reg_base + REG_SCL_TOP_BT_SYNC_CODE, sync.raw);
}

void sclr_bt_get(union sclr_bt_enc *enc, union sclr_bt_sync_code *sync)
{
	enc->raw = _reg_read(reg_base + REG_SCL_TOP_BT_ENC);
	sync->raw = _reg_read(reg_base + REG_SCL_TOP_BT_SYNC_CODE);
}


void sclr_disp_set_intf(enum sclr_vo_intf intf)
{
	bool data_en[5] = {true, true, true, true};

	if (intf == SCLR_VO_INTF_DISABLE)
		_reg_write_mask(0x03002840, 0xa, 0);
	else
		_reg_write_mask(0x03002840, 0xa, 0xa);

	if(intf == SCLR_VO_INTF_I80)
		dphy_i80_init();
	else if(intf == SCLR_VO_INTF_LVDS)
		dphy_lvds_init();

	if (intf == SCLR_VO_INTF_DISABLE) {
		sclr_top_set_vo_type_sel(SCLR_VO_SEL_DISABLE);
	} else if ((intf == SCLR_VO_INTF_BT601) || (intf == SCLR_VO_INTF_BT656) || (intf == SCLR_VO_INTF_BT1120)) {
		if (intf == SCLR_VO_INTF_BT601)
			sclr_top_set_vo_type_sel(SCLR_VO_SEL_BT601);
		else if (intf == SCLR_VO_INTF_BT656)
			sclr_top_set_vo_type_sel(SCLR_VO_SEL_BT656);
		else if (intf == SCLR_VO_INTF_BT1120)
			sclr_top_set_vo_type_sel(SCLR_VO_SEL_BT1120);
		dphy_dsi_lane_en(true, data_en, false);
	} else if (intf == SCLR_VO_INTF_I80) {
		sclr_top_set_vo_type_sel(SCLR_VO_SEL_I80);
		dphy_dsi_lane_en(true, data_en, false);
	} else if (intf == SCLR_VO_INTF_I80_HW) {
		   dphy_dsi_lane_en(true, data_en, false);
	} else if (intf == SCLR_VO_INTF_SW) {
		sclr_top_set_vo_type_sel(SCLR_VO_SEL_SW);
	} else if (intf == SCLR_VO_INTF_MIPI) {
		sclr_top_set_vo_type_sel(SCLR_VO_SEL_DISABLE);
	} else if (intf == SCLR_VO_INTF_LVDS) {
		sclr_top_set_vo_type_sel(SCLR_VO_SEL_DISABLE);
	}
}


/**
 * sclr_disp_timing_setup_from_reg - get settings from register.
 *
 */
void sclr_disp_timing_setup_from_reg(void)
{
	u32 tmp = 0;

	tmp = _reg_read(reg_base + REG_SCL_DISP_CFG);
	disp_timing.vsync_pol = (tmp & 0x20);
	disp_timing.hsync_pol = (tmp & 0x40);

	tmp = _reg_read(reg_base + REG_SCL_DISP_TOTAL);
	disp_timing.vtotal = tmp & 0xffff;
	disp_timing.htotal = (tmp >> 16) & 0xffff;

	tmp = _reg_read(reg_base + REG_SCL_DISP_VSYNC);
	disp_timing.vsync_start = tmp & 0xffff;
	disp_timing.vsync_end = (tmp >> 16) & 0xffff;

	tmp = _reg_read(reg_base + REG_SCL_DISP_VFDE);
	disp_timing.vfde_start = tmp & 0xffff;
	disp_timing.vfde_end = (tmp >> 16) & 0xffff;

	tmp = _reg_read(reg_base + REG_SCL_DISP_VMDE);
	disp_timing.vmde_start = tmp & 0xffff;
	disp_timing.vmde_end = (tmp >> 16) & 0xffff;

	tmp = _reg_read(reg_base + REG_SCL_DISP_HSYNC);
	disp_timing.hsync_start = tmp & 0xffff;
	disp_timing.hsync_end = (tmp >> 16) & 0xffff;

	tmp = _reg_read(reg_base + REG_SCL_DISP_HFDE);
	disp_timing.hfde_start = tmp & 0xffff;
	disp_timing.hfde_end = (tmp >> 16) & 0xffff;

	tmp = _reg_read(reg_base + REG_SCL_DISP_HMDE);
	disp_timing.hmde_start = tmp & 0xffff;
	disp_timing.hmde_end = (tmp >> 16) & 0xffff;
}

void sclr_disp_checksum_en(bool enable)
{
	_reg_write_mask(reg_base + REG_SCL_DISP_CHECKSUM0, BIT(31),
	                enable ? BIT(31) : 0);
}

void sclr_disp_get_checksum_status(struct sclr_disp_checksum_status *status)
{
	status->checksum_base.raw = _reg_read(reg_base + REG_SCL_DISP_CHECKSUM0);
	status->axi_read_from_dram = _reg_read(reg_base + REG_SCL_DISP_CHECKSUM1);
	status->axi_read_from_gop = _reg_read(reg_base + REG_SCL_DISP_CHECKSUM2);

}

/**
 * sclr_dsi_get_mode - get current dsi mode
 *
 * @return: current dsi mode
 */
enum sclr_dsi_mode sclr_dsi_get_mode(void)
{
	return (_reg_read(reg_base + REG_SCL_DSI_MAC_EN) & 0x0f);
}

/**
 * sclr_dsi_clr_mode - let dsi back to idle mode
 *
 */
void sclr_dsi_clr_mode(void)
{
	u32 mode = _reg_read(reg_base + REG_SCL_DSI_MAC_EN);

	if (mode != SCLR_DSI_MODE_IDLE)
		_reg_write(reg_base + REG_SCL_DSI_MAC_EN, mode);
}

/**
 * sclr_dsi_set_mode - set dsi mode
 *
 * @param mode: new dsi mode except for idle
 * @return: 0 if success
 */
int sclr_dsi_set_mode(enum sclr_dsi_mode mode)
{
	if (mode >= SCLR_DSI_MODE_MAX)
		return -1;

	if (mode == SCLR_DSI_MODE_IDLE) {
		sclr_dsi_clr_mode();
		return 0;
	}

	if (_reg_read(reg_base + REG_SCL_DSI_MAC_EN))
		return -1;

	_reg_write(reg_base + REG_SCL_DSI_MAC_EN, mode);
	return 0;
}

/**
 * sclr_dsi_chk_mode_done - check if dsi's work done.
 *
 * @param mode: the mode to check.
 * @return: 0 if success
 */
int sclr_dsi_chk_mode_done(enum sclr_dsi_mode mode)
{
	u32 val = 0;

	if ((mode == SCLR_DSI_MODE_ESC) || (mode == SCLR_DSI_MODE_SPKT)) {
		val = _reg_read(reg_base + REG_SCL_DSI_MAC_EN) & 0xf0;
		return (val ^ (mode << 4)) ? -1 : 0;
	}

	if ((mode == SCLR_DSI_MODE_IDLE) || (mode == SCLR_DSI_MODE_HS)) {
		val = _reg_read(reg_base + REG_SCL_DSI_MAC_EN) & 0x0f;
		return (val == (mode)) ? 0 : -1;
	}

	return -1;
}

int _dsi_chk_and_clean_mode(enum sclr_dsi_mode mode)
{
	int i, ret;

	for (i = 0; i < 5; ++i) {
		udelay(20);
		ret = sclr_dsi_chk_mode_done(mode);
		if (ret == 0) {
			sclr_dsi_clr_mode();
			break;
		}
	}
	return ret;
}

#define POLY 0x8408
/*
 *                                      16   12   5
 * this is the CCITT CRC 16 polynomial X  + X  + X  + 1.
 * This works out to be 0x1021, but the way the algorithm works
 * lets us use 0x8408 (the reverse of the bit pattern).  The high
 * bit is always assumed to be set, thus we only use 16 bits to
 * represent the 17 bit value.
 */
static u16 crc16(unsigned char *data_p, unsigned short length)
{
	u8 i, data;
	u16 crc = 0xffff;

	if (length == 0)
		return (~crc);

	do {
		for (i = 0, data = 0xff & *data_p++; i < 8; i++, data >>= 1) {
			if ((crc & 0x0001) ^ (data & 0x0001))
				crc = (crc >> 1) ^ POLY;
			else
				crc >>= 1;
		}
	} while (--length);

	return crc;
}

static unsigned char ecc(unsigned char *data)
{
	char D0, D1, D2, D3, D4, D5, D6, D7, D8, D9, D10, D11, D12;
	char D13, D14, D15, D16, D17, D18, D19, D20, D21, D22, D23;
	char P0, P1, P2, P3, P4, P5, P6, P7;

	D0  = data[0] & 0x01;
	D1  = (data[0] >> 1) & 0x01;
	D2  = (data[0] >> 2) & 0x01;
	D3  = (data[0] >> 3) & 0x01;
	D4  = (data[0] >> 4) & 0x01;
	D5  = (data[0] >> 5) & 0x01;
	D6  = (data[0] >> 6) & 0x01;
	D7  = (data[0] >> 7) & 0x01;

	D8  = data[1] & 0x01;
	D9  = (data[1] >> 1) & 0x01;
	D10 = (data[1] >> 2) & 0x01;
	D11 = (data[1] >> 3) & 0x01;
	D12 = (data[1] >> 4) & 0x01;
	D13 = (data[1] >> 5) & 0x01;
	D14 = (data[1] >> 6) & 0x01;
	D15 = (data[1] >> 7) & 0x01;

	D16 = data[2] & 0x01;
	D17 = (data[2] >> 1) & 0x01;
	D18 = (data[2] >> 2) & 0x01;
	D19 = (data[2] >> 3) & 0x01;
	D20 = (data[2] >> 4) & 0x01;
	D21 = (data[2] >> 5) & 0x01;
	D22 = (data[2] >> 6) & 0x01;
	D23 = (data[2] >> 7) & 0x01;

	P7 = 0;
	P6 = 0;
	P5 = (D10^D11^D12^D13^D14^D15^D16^D17^D18^D19^D21^D22^D23) & 0x01;
	P4 = (D4^D5^D6^D7^D8^D9^D16^D17^D18^D19^D20^D22^D23) & 0x01;
	P3 = (D1^D2^D3^D7^D8^D9^D13^D14^D15^D19^D20^D21^D23) & 0x01;
	P2 = (D0^D2^D3^D5^D6^D9^D11^D12^D15^D18^D20^D21^D22) & 0x01;
	P1 = (D0^D1^D3^D4^D6^D8^D10^D12^D14^D17^D20^D21^D22^D23) & 0x01;
	P0 = (D0^D1^D2^D4^D5^D7^D10^D11^D13^D16^D20^D21^D22^D23) & 0x01;

	return (P7 << 7) | (P6 << 6) | (P5 << 5) | (P4 << 4) |
		(P3 << 3) | (P2 << 2) | (P1 << 1) | P0;
}

/**
 * sclr_dsi_long_packet_raw - send dsi long packet by escapet-lpdt.
 *
 * @param data: long packet data including header and crc.
 * @param count: number of long packet data, 16 at most.
 * @return: 0 if success
 */
int sclr_dsi_long_packet_raw(const u8 *data, u8 count)
{
	u64 addr = reg_base + REG_SCL_DSI_ESC_TX0;
	u32 val = 0;
	u8 i = 0;

	if ((count > SCL_MAX_DSI_LP) || (count == 0))
		return -1;

	val = 0x01 | ((count - 1) << 8);
	_reg_write(reg_base + REG_SCL_DSI_ESC, val);
	for (i = 0; i < count; i += 4) {
		if (count - i < 4) {
			val = 0;
			memcpy(&val, &data[i], count - i);
			_reg_write(addr + i, val);
			break;
		}
		memcpy(&val, &data[i], 4);
		_reg_write(addr + i, val);
	}

	sclr_dsi_set_mode(SCLR_DSI_MODE_ESC);
	return _dsi_chk_and_clean_mode(SCLR_DSI_MODE_ESC);
}

/**
 * sclr_dsi_long_packet - send dsi long packet by escapet-lpdt.
 *
 * @param data: long packet data
 * @param count: number of long packet data, 100 at most.
 * @return: 0 if success
 */
int sclr_dsi_long_packet(const u8 *data, u8 count)
{
	u8 packet[100] = {0x39, count, 0, 0};
	u16 crc;

	packet[3] = ecc(packet);
	memcpy(&packet[4], data, count);
	count += 4;
	crc = crc16(packet, count);
	packet[count++] = crc & 0xff;
	packet[count++] = crc >> 8;

#if 0
	return sclr_dsi_long_packet_raw(packet, count);
#else
	dpyh_mipi_tx_manual_packet(packet, count);
	return 0;
#endif
}

/**
 * sclr_dsi_short_packet - send dsi short packet by escapet-lpdt.
 *   *NOTE*: ecc is hw-generated.
 *
 * @param data: short packet data
 * @param count: number of short packet data, 1 or 2.
 * @return: 0 if success
 */
int sclr_dsi_short_packet(const u8 *data, u8 count)
{
	int ret = 0;
	u32 val = 0;

	if ((count > SCL_MAX_DSI_SP) || (count == 0))
		return -1;

	if (count == 2) {
		val = 0x15;
		val |= (data[0] << 8) | (data[1] << 16);
	} else {
		val = 0x05;
		val |= data[0] << 8;
	}
	_reg_write_mask(reg_base + REG_SCL_DSI_HS_0, 0x00ffffff, val);
	sclr_dsi_set_mode(SCLR_DSI_MODE_SPKT);
	udelay(20);
	ret = sclr_dsi_chk_mode_done(SCLR_DSI_MODE_SPKT);
	if (ret == 0)
		sclr_dsi_clr_mode();
	return ret;
}

/**
 * sclr_dsi_dcs_write_buffer - send dsi packet by escapet-lpdt.
 *
 * @param data: packet data
 * @param count: number of packet data
 * @return: Zero on success or a negative error code on failure.
 */
ssize_t sclr_dsi_dcs_write_buffer(const void *data, size_t len)
{
	int ret = 0;

	switch (len) {
	case 0:
		return -EINVAL;
	case 1:
	case 2:
		ret = sclr_dsi_short_packet(data, len);
		break;
	default:
		ret = sclr_dsi_long_packet(data, len);
		break;
	}

	return ret;
}

int sclr_dsi_config(u8 lane_num, enum sclr_dsi_fmt fmt, u16 width)
{
	u32 val = 0;
	u8 bit_depth[] = {24, 18, 16, 30};

	if ((lane_num != 1) && (lane_num != 2) && (lane_num != 4))
		return -EINVAL;
	if (fmt > SCLR_DSI_FMT_MAX)
		return -EINVAL;

	lane_num >>= 1;
	val = (fmt << 30) | (lane_num << 24);
	_reg_write_mask(reg_base + REG_SCL_DSI_HS_0, 0xc3000000, val);
	val = (width/6) << 16 | UPPER(width * bit_depth[fmt], 3);
	_reg_write(reg_base + REG_SCL_DSI_HS_1, val);

	return 0;
}

void sclr_i80_sw_mode(bool enable)
{
	_reg_write_mask(reg_base + REG_SCL_DISP_MCU_IF_CTRL, BIT(11) | BIT(1), enable ? 0x802 : 0x002);

	//1. set sw_mode (disp_0x200[2:0] = 3'h2) = > BIT(1)
	//2. trig i80 (write 1 to disp_0x200[11] ] = >BIT(11)
	//3. check status (disp_208[2:0] = 3'h1, wait cmd)
	//4. set sw commdnad (disp_0x204 = 0x00000F0F)
	//6. trig sw command (write 1 to 0x204[31])
	//7. chek sw command taken (disp_0x204[24] = 1)

	//T gen not enable during sw MCU mode
	if (0) {
		sclr_disp_tgen_enable(true);
		mdelay(40);
		sclr_disp_tgen_enable(false);
	}
}

void sclr_i80_packet(u32 cmd)
{
	u8 cnt = 0;

	_reg_write(reg_base + REG_SCL_DISP_MCU_SW_CTRL, cmd);
	_reg_write_mask(reg_base + REG_SCL_DISP_MCU_SW_CTRL, BIT(31), BIT(31));

	do {
		udelay(1);
		if (_reg_read(reg_base + REG_SCL_DISP_MCU_SW_CTRL) & BIT(24))
			break;
	} while (++cnt < 10);

	if (cnt == 10)
		uartlog("[cvi_vip] %s: cmd(%#x) not ready.\n", __func__, cmd);
}

void sclr_i80_run(void)
{
	u8 cnt = 0, cnt1 = 0;

	do {
		cnt = 0;


		_reg_write_mask(reg_base + REG_SCL_DISP_MCU_IF_CTRL, BIT(11), BIT(11));
		_reg_write_mask(reg_base + REG_SCL_DISP_MCU_IF_CTRL, BIT(1), 0);
		_reg_write_mask(reg_base + REG_SCL_DISP_MCU_IF_CTRL, BIT(0), BIT(0));


		do {
			udelay(2 * 1000);
			if (_reg_read(reg_base + REG_SCL_DISP_MCU_STATUS) == 0x08)
				break;
		} while (++cnt < 30);

		if (cnt == 30) {
			debug("[cvi_vip] %s: not finish. sw clear it.\n", __func__);
			_reg_write_mask(reg_base + REG_SCL_DISP_MCU_IF_CTRL, BIT(10), BIT(10));
		} else
			break;
	} while (++cnt1 < 5);
}

/****************************************************************************
 * SCALER CTRL
 ****************************************************************************/
/**
 * sclr_ctrl_init - setup all sc instances.
 *
 */
void sclr_ctrl_init(void)
{
	union sclr_intr intr_mask;
	union vip_sys_intr vip_intr_mask;
	union sclr_rt_cfg rt_cfg;
	bool disp_from_sc = false;
	unsigned int i,j;

	memset(&intr_mask, 0, sizeof(intr_mask));

	// init variables
	memset(&g_top_cfg, 0, sizeof(g_top_cfg));
	memset(&g_sc_cfg, 0, sizeof(g_sc_cfg));
	memset(&g_bd_cfg, 0, sizeof(g_bd_cfg));
	memset(&g_cir_cfg, 0, sizeof(g_cir_cfg));
	memset(&g_img_cfg, 0, sizeof(g_img_cfg));
	memset(&g_odma_cfg, 0, sizeof(g_odma_cfg));
	memset(&g_disp_cfg, 0, sizeof(g_disp_cfg));
	memset(&disp_timing, 0, sizeof(disp_timing));
	memset(&g_gop_cfg, 0, sizeof(g_gop_cfg));

	for (i = 0; i < SCL_MAX_INST; ++i)
		for (j = 0; j < SCL_MAX_GOP_INST; ++j) {
			g_odma_cfg[i].flip = SCL_FLIP_NO;
			g_odma_cfg[i].burst = false;
			g_gop_cfg[i][j].fb_ctrl.b.hi_thr = 0x30;
			g_gop_cfg[i][j].fb_ctrl.b.lo_thr = 0x20;
			g_gop_cfg[i][j].fb_ctrl.b.detect_fnum = 1;
			g_gop_cfg[i][j].fb_ctrl.b.hi_thr = 0x30;
			g_gop_cfg[i][j].fb_ctrl.b.lo_thr = 0x20;
			g_gop_cfg[i][j].fb_ctrl.b.detect_fnum = 1;
		}

	g_img_cfg[SCL_IMG_D].burst = 7;
	g_img_cfg[SCL_IMG_D].src = SCL_INPUT_D_MEM;
	g_img_cfg[SCL_IMG_V].burst = 7;
	g_img_cfg[SCL_IMG_V].src = SCL_INPUT_D_MEM;
	g_top_cfg.ip_trig_src = 1; // 0(IMG_V), 1(IMG_D)
	g_top_cfg.sclr_enable[0] = false;
	g_top_cfg.sclr_enable[1] = false;
	g_top_cfg.sclr_enable[2] = false;
	g_top_cfg.sclr_enable[3] = false;
	g_top_cfg.disp_enable = false;
	g_top_cfg.disp_from_sc = disp_from_sc;
	g_top_cfg.img_in_d_trig_src = SCL_IMG_TRIG_SRC_SW;
	g_top_cfg.img_in_v_trig_src = SCL_IMG_TRIG_SRC_SW;
	g_disp_cfg.disp_from_sc = disp_from_sc;
	g_disp_cfg.cache_mode = true;
	g_disp_cfg.sync_ext = false;
	g_disp_cfg.tgen_en = false;
	g_disp_cfg.fmt = SCL_FMT_RGB_PLANAR;
	g_disp_cfg.in_csc = SCL_CSC_NONE;
	g_disp_cfg.out_csc = SCL_CSC_NONE;
	g_disp_cfg.burst = 7;
	g_disp_cfg.out_bit = 8;
	g_disp_cfg.drop_mode = SCL_DISP_DROP_MODE_DITHER;
	g_disp_cfg.mem.width = 80;
	g_disp_cfg.mem.height = 80;

	// get current hw-timings
	sclr_disp_timing_setup_from_reg();

	// init hw
	sclr_top_set_cfg(&g_top_cfg);

	rt_cfg.b.sc_d_rt = 0;
	rt_cfg.b.sc_v_rt = 0;
	rt_cfg.b.sc_rot_rt = 0;
	rt_cfg.b.img_d_rt = 0;
	rt_cfg.b.img_v_rt = 0;
	rt_cfg.b.img_d_sel = 1;
	sclr_rt_set_cfg(rt_cfg);

	intr_mask.b.img_in_d_frame_end = true;
	intr_mask.b.img_in_v_frame_end = true;
	intr_mask.b.scl0_frame_end = true;
	intr_mask.b.scl1_frame_end = true;
	intr_mask.b.scl2_frame_end = true;
	intr_mask.b.scl3_frame_end = true;
	intr_mask.b.prog_too_late = true;
	intr_mask.b.cmdq = true;
	intr_mask.b.disp_frame_end = false;
	intr_mask.b.osd_cmp_frame_end = true;
	sclr_set_intr_mask(intr_mask);

	sclr_img_reg_shadow_sel(0, false);
	sclr_img_reg_shadow_sel(1, false);
	sclr_disp_reg_shadow_sel(false);
	for (i = 0; i < SCL_MAX_INST; ++i) {
		sclr_reg_shadow_sel(i, false);
		sclr_init(i);
		sclr_set_cfg(i, false, false, true, false);
		sclr_reg_force_up(i);
	}

	sclr_disp_tgen_enable(false);
	sclr_disp_set_cfg(&g_disp_cfg);

	sclr_top_reg_done();
	sclr_top_reg_force_up();
	sclr_top_pg_late_clr();

	vip_intr_mask = vip_get_intr_mask();
	vip_intr_mask.b.sc = 1;
	vip_set_intr_mask(vip_intr_mask);
}

static void _tile_cal_size(struct sclr_scale_cfg *cfg)
{
	struct sclr_size in_size = cfg->src;
	struct sclr_size out_size = cfg->dst;
	u32 out_l_width = (out_size.w >> 1);
	u32 h_sc_fac = (((in_size.w - 1) << 13) + (out_size.w >> 1))
		       / (out_size.w - 1);
	u32 L_last_phase, R_first_phase;
	u16 L_last_pixel, R_first_pixel;

	L_last_phase = (out_l_width - 1) * h_sc_fac;
	L_last_pixel = (L_last_phase >> 13) + ((cfg->mir_enable) ? 0 : 1);
	cfg->tile.src_l_width = L_last_pixel + 2
				+ ((L_last_phase & 0x1fff) ? 1 : 0);
	cfg->tile.out_l_width = out_l_width;

	// right tile no mirror
	R_first_phase = L_last_phase + h_sc_fac;
	R_first_pixel = (R_first_phase >> 13) + ((cfg->mir_enable) ? 0 : 1);
	cfg->tile.r_ini_phase = R_first_phase & 0x1fff;
	cfg->tile.src_r_offset = R_first_pixel - 1;
	cfg->tile.src_r_width = in_size.w - cfg->tile.src_r_offset;

	// TODO: crop?
	cfg->tile.src = in_size;
	cfg->tile.out = out_size;

	uartlog("h_sc_fac:%d\n", h_sc_fac);
	uartlog("L last phase(%d) last pixel(%d)\n", L_last_phase, L_last_pixel);
	uartlog("R first phase(%d) first pixel(%d)\n", R_first_phase, R_first_pixel);
	uartlog("tile cfg: Left width src(%d) out(%d)\n", cfg->tile.src_l_width, cfg->tile.out_l_width);
	uartlog("tile cfg: Right src offset(%d) src(%d) phase(%d)\n", cfg->tile.src_r_offset, cfg->tile.src_r_width, cfg->tile.r_ini_phase);
}

/**
 * sclr_ctrl_set_scale - setup scaling
 *
 * @param inst: (0~3), the instance of sc
 * @param cfg: scaling settings, include in/crop/out size.
 */
void sclr_ctrl_set_scale(u8 inst, struct sclr_scale_cfg *cfg)
{
	if (inst >= SCL_MAX_INST) {
		uartlog("[bm-vip][sc] %s: no enough sclr-instance ", __func__);
		uartlog("for the requirement(%d)\n", inst);
		return;
	}
	if (memcmp(cfg, &g_sc_cfg[inst].sc, sizeof(*cfg)) == 0)
		return;

	sclr_set_input_size(inst, cfg->src);

	// if crop invalid, use src-size
	if (cfg->crop.w + cfg->crop.x > cfg->src.w) {
		cfg->crop.x = 0;
		cfg->crop.w = cfg->src.w;
	}
	if (cfg->crop.h + cfg->crop.y > cfg->src.h) {
		cfg->crop.y = 0;
		cfg->crop.h = cfg->src.h;
	}
	sclr_set_crop(inst, cfg->crop);
	sclr_set_output_size(inst, cfg->dst);

	if (cfg->tile_enable)
		_tile_cal_size(cfg);
	g_sc_cfg[inst].sc.mir_enable = cfg->mir_enable;
	g_sc_cfg[inst].sc.tile_enable = cfg->tile_enable;
	g_sc_cfg[inst].sc.tile = cfg->tile;
	sclr_set_scale(inst);
}

/**
 * sclr_ctrl_set_input - setup input
 *
 * @param img_inst: (0~1), the instance of img_in
 * @param input: input mux
 * @param fmt: color format
 * @param csc: csc which used to convert from yuv to rgb.
 * @return: 0 if success
 */
int sclr_ctrl_set_input(enum sclr_img_in img_inst, union sclr_input input,
			enum sclr_format fmt, enum sclr_csc csc)
{
	int ret = 0;

	// always transfer to rgb for sc
	if ((img_inst >= SCL_IMG_MAX) || (csc >= SCL_CSC_601_LIMIT_RGB2YUV))
		return -EINVAL;

	switch (img_inst) {
	case SCL_IMG_D:
		g_img_cfg[img_inst].src = input.d;
		break;

	case SCL_IMG_V:
		g_img_cfg[img_inst].src = input.v;
		break;

	default:
		break;
	};

	g_img_cfg[img_inst].fmt = fmt;
	sclr_img_set_cfg(img_inst, &g_img_cfg[img_inst]);

	g_img_cfg[img_inst].csc = csc;
	if (csc == SCL_CSC_NONE) {
		sclr_img_csc_en(img_inst, false);
	} else {
		sclr_img_csc_en(img_inst, true);
		sclr_img_set_csc(img_inst, &csc_mtrx[csc]);
	}

	return ret;
}

/**
 * sclr_ctrl_set_output - setup output of sc
 *
 * @param inst: (0~3), the instance of sc
 * @param cfg: csc config, including hvs/quant settings if any
 * @param fmt: color format
 * @return: 0 if success
 */
int sclr_ctrl_set_output(u8 inst, struct sclr_csc_cfg *cfg,
			 enum sclr_format fmt)
{
	if (inst >= SCL_MAX_INST)
		return -EINVAL;

	if (cfg->mode == SCL_OUT_CSC) {
		// Use yuv for csc
		// sc's data is always rgb
		if (!IS_YUV_FMT(fmt) ||
		    ((cfg->csc_type < SCL_CSC_601_LIMIT_RGB2YUV) &&
		     (cfg->csc_type != SCL_CSC_NONE)))
			return -EINVAL;
	} else {
		// Use rgb for quant/hsv
		if (IS_YUV_FMT(fmt))
			return -EINVAL;
	}

	sclr_set_csc_ctrl(inst, cfg);
	sclr_odma_set_fmt(inst, fmt);

	return 0;
}

/**
 * sclr_ctrl_set_disp_src - setup input-src of disp.
 *
 * @param disp_from_sc: true(from sc_0); false(from mem)
 * @return: 0 if success
 */
int sclr_ctrl_set_disp_src(bool disp_from_sc)
{
	g_top_cfg.disp_from_sc = disp_from_sc;
	g_disp_cfg.disp_from_sc = disp_from_sc;
	if (disp_from_sc) {
		g_top_cfg.img_in_d_trig_src = SCL_IMG_TRIG_SRC_DISP;
	} else {
		g_top_cfg.img_in_d_trig_src =
		    (g_img_cfg[SCL_IMG_D].src == SCL_INPUT_D_ISP)
		    ? SCL_IMG_TRIG_SRC_ISP
		    : SCL_IMG_TRIG_SRC_SW;
	}

	sclr_top_set_cfg(&g_top_cfg);
	sclr_disp_set_cfg(&g_disp_cfg);
	sclr_set_cfg(0, g_sc_cfg[0].sc_bypass, g_sc_cfg[0].gop_bypass,
		     g_sc_cfg[0].cir_bypass, disp_from_sc);

	return 0;
}

static void _map_ctrl_to_img(struct sclr_ctrl_cfg *cfg,
			     struct sclr_img_cfg *img_cfg)
{
	img_cfg->burst = g_img_cfg[cfg->img_inst].burst;
	img_cfg->fmt = cfg->src_fmt;
	img_cfg->csc = cfg->src_csc;
	img_cfg->src = (cfg->img_inst == SCL_IMG_D) ?
		       cfg->input.d : cfg->input.v;
	img_cfg->mem.addr0 = cfg->src_addr0;
	img_cfg->mem.addr1 = cfg->src_addr1;
	img_cfg->mem.addr2 = cfg->src_addr2;
	img_cfg->mem.start_x = 0;
	img_cfg->mem.start_y = 0;
	img_cfg->mem.width = cfg->src.w;
	img_cfg->mem.height = cfg->src.h;
	img_cfg->mem.pitch_y = IS_PACKED_FMT(cfg->src_fmt)
				? VIP_ALIGN(3 * cfg->src.w)
				: VIP_ALIGN(cfg->src.w);
	img_cfg->mem.pitch_c = IS_YUV_FMT(cfg->src_fmt)
				? VIP_ALIGN(cfg->src.w >> 1)
				: VIP_ALIGN(cfg->src.w);
}

static void _map_ctrl_to_sc(struct sclr_ctrl_cfg *cfg,
			    struct sclr_core_cfg *sc_cfg, u8 inst)
{
	sc_cfg->sc_bypass = false;
	sc_cfg->gop_bypass = false;
	sc_cfg->cir_bypass = true;
	sc_cfg->odma_bypass = false;
	sc_cfg->sc.src = cfg->src;
	sc_cfg->sc.crop = cfg->src_crop;
	sc_cfg->sc.dst = cfg->dst[inst].window;
}

static void _map_ctrl_to_odma(struct sclr_ctrl_cfg *cfg,
			      struct sclr_odma_cfg *odma_cfg, u8 inst)
{
	odma_cfg->flip = SCL_FLIP_NO;
	odma_cfg->burst = g_odma_cfg[inst].burst;
	odma_cfg->fmt = cfg->dst[inst].fmt;
	odma_cfg->csc_cfg.csc_type = cfg->dst[inst].csc;
	odma_cfg->mem.addr0 = cfg->dst[inst].addr0;
	odma_cfg->mem.addr1 = cfg->dst[inst].addr1;
	odma_cfg->mem.addr2 = cfg->dst[inst].addr2;
	odma_cfg->mem.start_x = cfg->dst[inst].offset.x;
	odma_cfg->mem.start_y = cfg->dst[inst].offset.y;
	odma_cfg->mem.width = cfg->dst[inst].window.w;
	odma_cfg->mem.height = cfg->dst[inst].window.h;
	odma_cfg->mem.pitch_y = IS_PACKED_FMT(cfg->dst[inst].fmt)
				? VIP_ALIGN(3 * cfg->dst[inst].frame.w)
				: VIP_ALIGN(cfg->dst[inst].frame.w);
	odma_cfg->mem.pitch_c = IS_YUV_FMT(cfg->dst[inst].fmt)
				? VIP_ALIGN(cfg->dst[inst].frame.w >> 1)
				: VIP_ALIGN(cfg->dst[inst].frame.w);
}

int _map_img_to_cmdq(struct sclr_ctrl_cfg *ctrl_cfg, union cmdq_set *cmd_start,
		     u16 cmd_idx)
{
	struct sclr_img_cfg img_cfg;
	struct sclr_csc_matrix *mtrx;
	u8 img_inst = ctrl_cfg->img_inst;

	if (img_inst >= SCL_IMG_MAX)
		return cmd_idx;

	_map_ctrl_to_img(ctrl_cfg, &img_cfg);
	mtrx = &csc_mtrx[img_cfg.csc];

	cmdQ_set_package(&cmd_start[cmd_idx++].reg, REG_SCL_IMG_CFG(img_inst),
			 (img_cfg.burst << 8) | (img_cfg.fmt << 4) |
			 img_cfg.src);
	cmdQ_set_package(&cmd_start[cmd_idx++].reg,
			 REG_SCL_IMG_OFFSET(img_inst),
			 (img_cfg.mem.start_y << 16) | img_cfg.mem.start_x);
	cmdQ_set_package(&cmd_start[cmd_idx++].reg, REG_SCL_IMG_SIZE(img_inst),
			 ((img_cfg.mem.height - 1) << 16) |
			 (img_cfg.mem.width - 1));
	cmdQ_set_package(&cmd_start[cmd_idx++].reg,
			 REG_SCL_IMG_PITCH_Y(img_inst), img_cfg.mem.pitch_y);
	cmdQ_set_package(&cmd_start[cmd_idx++].reg,
			 REG_SCL_IMG_PITCH_C(img_inst), img_cfg.mem.pitch_c);
	cmdQ_set_package(&cmd_start[cmd_idx++].reg,
			 REG_SCL_IMG_ADDR0_L(img_inst), img_cfg.mem.addr0);
	cmdQ_set_package(&cmd_start[cmd_idx++].reg,
			 REG_SCL_IMG_ADDR0_H(img_inst),
			 img_cfg.mem.addr0 >> 32);
	cmdQ_set_package(&cmd_start[cmd_idx++].reg,
			 REG_SCL_IMG_ADDR1_L(img_inst), img_cfg.mem.addr1);
	cmdQ_set_package(&cmd_start[cmd_idx++].reg,
			 REG_SCL_IMG_ADDR1_H(img_inst),
			 img_cfg.mem.addr1 >> 32);
	cmdQ_set_package(&cmd_start[cmd_idx++].reg,
			 REG_SCL_IMG_ADDR2_L(img_inst), img_cfg.mem.addr2);
	cmdQ_set_package(&cmd_start[cmd_idx++].reg,
			 REG_SCL_IMG_ADDR2_H(img_inst),
			 img_cfg.mem.addr2 >> 32);

	cmdQ_set_package(&cmd_start[cmd_idx++].reg,
			 REG_SCL_IMG_CSC_COEF0(img_inst),
			 (mtrx->coef[0][1] << 16) | mtrx->coef[0][0]);
	cmdQ_set_package(&cmd_start[cmd_idx++].reg,
			 REG_SCL_IMG_CSC_COEF1(img_inst), mtrx->coef[0][2]);
	cmdQ_set_package(&cmd_start[cmd_idx++].reg,
			 REG_SCL_IMG_CSC_COEF2(img_inst),
			 (mtrx->coef[1][1] << 16) | mtrx->coef[1][0]);
	cmdQ_set_package(&cmd_start[cmd_idx++].reg,
			 REG_SCL_IMG_CSC_COEF3(img_inst), mtrx->coef[1][2]);
	cmdQ_set_package(&cmd_start[cmd_idx++].reg,
			 REG_SCL_IMG_CSC_COEF4(img_inst),
			 (mtrx->coef[2][1] << 16) | mtrx->coef[2][0]);
	cmdQ_set_package(&cmd_start[cmd_idx++].reg,
			 REG_SCL_IMG_CSC_COEF5(img_inst), mtrx->coef[2][2]);
	cmdQ_set_package(&cmd_start[cmd_idx++].reg,
			 REG_SCL_IMG_CSC_SUB(img_inst),
			 (mtrx->sub[2] << 16) | (mtrx->sub[1] << 8)
			  | mtrx->sub[0]);
	cmdQ_set_package(&cmd_start[cmd_idx++].reg,
			 REG_SCL_IMG_CSC_ADD(img_inst),
			 (mtrx->add[2] << 16) | (mtrx->add[1] << 8)
			  | mtrx->add[0]);

	return cmd_idx;
}

int _map_sc_to_cmdq(struct sclr_ctrl_cfg *ctrl_cfg, u8 j,
		    union cmdq_set *cmd_start, u16 cmd_idx)
{
	u32 tmp = 0;
	u8 inst = ctrl_cfg->dst[j].inst;
	struct sclr_core_cfg sc_cfg;
	struct sclr_odma_cfg odma_cfg;

	if (inst >= SCL_MAX_INST)
		return cmd_idx;

	_map_ctrl_to_sc(ctrl_cfg, &sc_cfg, j);
	_map_ctrl_to_odma(ctrl_cfg, &odma_cfg, j);

	// sc core
	tmp = 0;
	if (sc_cfg.sc_bypass)
		tmp |= 0x02;
	if (sc_cfg.gop_bypass)
		tmp |= 0x04;
	if (sc_cfg.cir_bypass)
		tmp |= 0x20;
	if (sc_cfg.odma_bypass)
		tmp |= 0x40;

	cmdQ_set_package(&cmd_start[cmd_idx++].reg, REG_SCL_CFG(inst), tmp);
	cmdQ_set_package(&cmd_start[cmd_idx++].reg, REG_SCL_SRC_SIZE(inst),
			 ((sc_cfg.sc.src.h - 1) << 12) | (sc_cfg.sc.src.w - 1));
	cmdQ_set_package(&cmd_start[cmd_idx++].reg, REG_SCL_CROP_OFFSET(inst),
			 (sc_cfg.sc.crop.y << 12) | sc_cfg.sc.crop.x);
	cmdQ_set_package(&cmd_start[cmd_idx++].reg, REG_SCL_CROP_SIZE(inst),
			 ((sc_cfg.sc.crop.h - 1) << 16)
			  | (sc_cfg.sc.crop.w - 1));
	cmdQ_set_package(&cmd_start[cmd_idx++].reg, REG_SCL_OUT_SIZE(inst),
			 ((sc_cfg.sc.dst.h - 1) << 16) | (sc_cfg.sc.dst.w - 1));
	tmp = ((sc_cfg.sc.crop.w - 1) << 13) / (sc_cfg.sc.dst.w - 1);
	cmdQ_set_package(&cmd_start[cmd_idx++].reg, REG_SCL_SC_H_CFG(inst),
			 tmp << 8);
	tmp = ((sc_cfg.sc.crop.h - 1) << 13) / (sc_cfg.sc.dst.h - 1);
	cmdQ_set_package(&cmd_start[cmd_idx++].reg, REG_SCL_SC_V_CFG(inst),
			 tmp << 8);

	// sc odma
	cmdQ_set_package(&cmd_start[cmd_idx++].reg, REG_SCL_ODMA_CFG(inst),
			 (odma_cfg.flip << 16) |  (odma_cfg.fmt << 8) |
			 odma_cfg.burst);
	cmdQ_set_package(&cmd_start[cmd_idx++].reg, REG_SCL_ODMA_OFFSET_X(inst),
			 odma_cfg.mem.start_x);
	cmdQ_set_package(&cmd_start[cmd_idx++].reg, REG_SCL_ODMA_OFFSET_Y(inst),
			 odma_cfg.mem.start_y);
	cmdQ_set_package(&cmd_start[cmd_idx++].reg, REG_SCL_ODMA_WIDTH(inst),
			 odma_cfg.mem.width - 1);
	cmdQ_set_package(&cmd_start[cmd_idx++].reg, REG_SCL_ODMA_HEIGHT(inst),
			 odma_cfg.mem.height - 1);
	cmdQ_set_package(&cmd_start[cmd_idx++].reg, REG_SCL_ODMA_PITCH_Y(inst),
			 odma_cfg.mem.pitch_y);
	cmdQ_set_package(&cmd_start[cmd_idx++].reg, REG_SCL_ODMA_PITCH_C(inst),
			 odma_cfg.mem.pitch_c);
	cmdQ_set_package(&cmd_start[cmd_idx++].reg, REG_SCL_ODMA_ADDR0_L(inst),
			 odma_cfg.mem.addr0);
	cmdQ_set_package(&cmd_start[cmd_idx++].reg, REG_SCL_ODMA_ADDR0_H(inst),
			 odma_cfg.mem.addr0 >> 32);
	cmdQ_set_package(&cmd_start[cmd_idx++].reg, REG_SCL_ODMA_ADDR1_L(inst),
			 odma_cfg.mem.addr1);
	cmdQ_set_package(&cmd_start[cmd_idx++].reg, REG_SCL_ODMA_ADDR1_H(inst),
			 odma_cfg.mem.addr1 >> 32);
	cmdQ_set_package(&cmd_start[cmd_idx++].reg, REG_SCL_ODMA_ADDR2_L(inst),
			 odma_cfg.mem.addr2);
	cmdQ_set_package(&cmd_start[cmd_idx++].reg, REG_SCL_ODMA_ADDR2_H(inst),
			 odma_cfg.mem.addr2 >> 32);

	// sc csc
	if (odma_cfg.csc_cfg.csc_type == SCL_CSC_NONE) {
		cmdQ_set_package(&cmd_start[cmd_idx++].reg,
				 REG_SCL_CSC_EN(inst), 0);
	} else {
		struct sclr_csc_matrix *cfg = &csc_mtrx[odma_cfg.csc_cfg.csc_type];

		cmdQ_set_package(&cmd_start[cmd_idx++].reg,
				 REG_SCL_CSC_EN(inst), 1);

		cmdQ_set_package(&cmd_start[cmd_idx++].reg,
				 REG_SCL_CSC_COEF0(inst),
				 (cfg->coef[0][1] << 16) | cfg->coef[0][0]);
		cmdQ_set_package(&cmd_start[cmd_idx++].reg,
				 REG_SCL_CSC_COEF1(inst),
				 (cfg->coef[1][0] << 16) | cfg->coef[0][2]);
		cmdQ_set_package(&cmd_start[cmd_idx++].reg,
				 REG_SCL_CSC_COEF2(inst),
				 (cfg->coef[1][2] << 16) | cfg->coef[1][1]);
		cmdQ_set_package(&cmd_start[cmd_idx++].reg,
				 REG_SCL_CSC_COEF3(inst),
				 (cfg->coef[2][1] << 16) | cfg->coef[2][0]);
		cmdQ_set_package(&cmd_start[cmd_idx++].reg,
				 REG_SCL_CSC_COEF4(inst), cfg->coef[2][2]);

		cmdQ_set_package(&cmd_start[cmd_idx++].reg,
				 REG_SCL_CSC_OFFSET(inst),
				 (cfg->add[2] << 16) | (cfg->add[1] << 8) |
				 cfg->add[0]);
	}

	return cmd_idx;
}
/**
 * sclr_engine_cmdq - trigger sc  by cmdq
 *
 * @param cfgs: settings for cmdq
 * @param cnt: the number of settings
 * @param cmdq_addr: memory-address to put cmdq
 */
void sclr_engine_cmdq(struct sclr_ctrl_cfg *cfgs, u8 cnt, u64 cmdq_addr)
{
	u8 i = 0, j = 0;
	u16 cmd_idx = 0;
	union cmdq_set *cmd_start = (union cmdq_set *)cmdq_addr;
	u32 tmp = 0, inst_enable = 0;
	u32 flag_num = 1;   // 0: sc_str_flag, sc_stp_flag

	memset((void *)cmdq_addr, 0, sizeof(u32)*0x100);

	_reg_write(reg_base + REG_SCL_TOP_CMDQ_START, 0x18000);
	_reg_write(reg_base + REG_SCL_TOP_CMDQ_STOP, 0x18000);

	for (i = 0; i < cnt; ++i) {
		struct sclr_ctrl_cfg *cfg = (cfgs + i);
		u8 img_inst = cfg->img_inst;

		cmd_idx = _map_img_to_cmdq(cfg, cmd_start, cmd_idx);

		for (j = 0; j < SCL_MAX_INST; ++j)
			cmd_idx = _map_sc_to_cmdq(cfg, j, cmd_start, cmd_idx);

		img_inst = (~img_inst) & 0x01;
		// modify & clear sc-cmdq's stop flag
		tmp = 0x10000 | BIT(img_inst+1);
		inst_enable = 0;
		for (j = 0; j < SCL_MAX_INST; ++j) {
			u8 inst = cfg->dst[j].inst;

			if (inst < SCL_MAX_INST) {
				tmp |= BIT(inst + 3);
				inst_enable |= BIT(inst);
			}
		}
		cmdQ_set_package(&cmd_start[cmd_idx++].reg,
				 REG_SCL_TOP_CMDQ_STOP, tmp);

		tmp = _reg_read(reg_base + REG_SCL_TOP_CFG1);
		tmp &= ~0x1f;
		tmp |= inst_enable;
		cmdQ_set_package(&cmd_start[cmd_idx++].reg, REG_SCL_TOP_CFG1,
				 tmp);

		// start sclr
		cmdQ_set_package(&cmd_start[cmd_idx++].reg,
				 REG_SCL_TOP_IMG_CTRL, BIT(img_inst));

		// wait end-inter(1)
		cmdQ_set_wait(&cmd_start[cmd_idx++], /*is_timer*/false, flag_num, 0);
	}
	// clear intr
	cmdQ_set_package(&cmd_start[cmd_idx++].reg, REG_SCL_TOP_INTR_STATUS,
			 0x1000ffff);

	cmd_start[cmd_idx-1].reg.intr_end = 1;
	cmd_start[cmd_idx-1].reg.intr_last = 1;

	cmdQ_intr_ctrl(REG_SCL_CMDQ_BASE, 0x02);
	cmdQ_engine(REG_SCL_CMDQ_BASE, (uintptr_t)cmdq_addr,
		    REG_SCL_TOP_BASE >> 22, true, false, cmd_idx);
}

void sclr_left_tile(u8 inst)
{
	struct sclr_scale_cfg *sc;
	struct sclr_odma_cfg *odma_cfg;
	struct sclr_rect crop;
	struct sclr_size dst;

	if (inst >= SCL_MAX_INST) {
		uartlog("[bm-vip][sc] %s: no enough sclr-instance ", __func__);
		uartlog("for the requirement(%d)\n", inst);
		return;
	}
	sc = &(sclr_get_cfg(inst)->sc);
	odma_cfg = sclr_odma_get_cfg(inst);

	crop.x = 0;
	crop.y = 0;
	crop.w = sc->tile.src_l_width;
	crop.h = sc->tile.src.h;
	dst.w = sc->tile.out_l_width;
	dst.h = sc->tile.out.h;

	sclr_set_crop(inst, crop);
	sclr_set_output_size(inst, dst);
	sclr_set_scale_mir(inst, sc->mir_enable);
	sclr_set_scale_phase(inst, 0, 0);

	odma_cfg->mem.start_x = 0;
	odma_cfg->mem.width = dst.w;
	sclr_odma_set_mem(inst, &odma_cfg->mem);
}

void sclr_right_tile(u8 inst)
{
	struct sclr_scale_cfg *sc;
	struct sclr_odma_cfg *odma_cfg;
	struct sclr_rect crop;
	struct sclr_size dst;

	if (inst >= SCL_MAX_INST) {
		uartlog("[bm-vip][sc] %s: no enough sclr-instance ", __func__);
		uartlog("for the requirement(%d)\n", inst);
		return;
	}
	sc = &(sclr_get_cfg(inst)->sc);
	odma_cfg = sclr_odma_get_cfg(inst);

	crop.x = sc->tile.src_r_offset;
	crop.y = 0;
	crop.w = sc->tile.src_r_width;
	crop.h = sc->src.h;
	dst.w = sc->tile.out.w - sc->tile.out_l_width;
	dst.h = sc->tile.out.h;

	sclr_set_crop(inst, crop);
	sclr_set_output_size(inst, dst);
	sclr_set_scale_mir(inst, 0);
	sclr_set_scale_phase(inst, sc->tile.r_ini_phase, 0);

	odma_cfg->mem.start_x = sc->tile.out_l_width;
	odma_cfg->mem.width = dst.w;
	sclr_odma_set_mem(inst, &odma_cfg->mem);
}

void sclr_dump_top_register(void)
{
	u32 val;
	u32 top_base = reg_base + REG_SCL_TOP_BASE;

	uartlog("top base address=0x%08x\n", top_base);

	val = _reg_read(top_base + SC_TOP_REG_00);
	uartlog("SC_TOP_REG_00=0x%08x\n", val);
	uartlog("    ip_trig_src=%d, force_clk_enable=%d\n",
		(val & SC_TOP_REG_IP_TRIG_SRC_MASK) >> SC_TOP_REG_IP_TRIG_SRC_OFFSET,
		(val & SC_TOP_REG_FORCE_CLK_ENABLE_MASK) >> SC_TOP_REG_FORCE_CLK_ENABLE_OFFSET);

	val = _reg_read(top_base + SC_TOP_REG_01);
	uartlog("SC_TOP_REG_01=0x%08x\n", val);
	uartlog("    sc_d_en=%d, sc_v1_en=%d, sc_v2_en=%d, sc_v3_en=%d\n",
		(val & SC_TOP_REG_SC_D_EN_MASK) >> SC_TOP_REG_SC_D_EN_OFFSET,
		(val & SC_TOP_REG_SC_V1_EN_MASK) >> SC_TOP_REG_SC_V1_EN_OFFSET,
		(val & SC_TOP_REG_SC_V2_EN_MASK) >> SC_TOP_REG_SC_V2_EN_OFFSET,
		(val & SC_TOP_REG_SC_V3_EN_MASK) >> SC_TOP_REG_SC_V3_EN_OFFSET);
	uartlog("    disp_en=%d, disp_src_sel=%d, sc_d_src_sel=%d\n",
		(val & SC_TOP_REG_DISP_EN_MASK) >> SC_TOP_REG_DISP_EN_OFFSET,
		(val & SC_TOP_REG_DISP_SRC_SEL_MASK) >> SC_TOP_REG_DISP_SRC_SEL_OFFSET,
		(val & SC_TOP_REG_SC_D_SRC_SEL_MASK) >> SC_TOP_REG_SC_D_SRC_SEL_OFFSET);
	uartlog("    sc_rot_sw_rst=%d, sc_debug_en=%d, qos_en=%d\n",
		(val & SC_TOP_REG_SC_ROT_SW_RST_MASK) >> SC_TOP_REG_SC_ROT_SW_RST_OFFSET,
		(val & SC_TOP_REG_SC_DEBUG_EN_MASK) >> SC_TOP_REG_SC_DEBUG_EN_OFFSET,
		(val & SC_TOP_REG_QOS_EN_MASK) >> SC_TOP_REG_QOS_EN_OFFSET);

	uartlog("SC_TOP_REG_02=0x%08x\n", _reg_read(top_base + SC_TOP_REG_02));
	uartlog("SC_TOP_REG_03=0x%08x\n", _reg_read(top_base + SC_TOP_REG_03));

	val = _reg_read(top_base + SC_TOP_REG_04);
	uartlog("SC_TOP_REG_04=0x%08x\n", val);
	uartlog("    sw_force_up_b0=%d, sw_force_up_b1=%d, sw_force_up_b2=%d\n",
		(val & SC_TOP_REG_SW_FORCE_UP_B0_MASK) >> SC_TOP_REG_SW_FORCE_UP_B0_OFFSET,
		(val & SC_TOP_REG_SW_FORCE_UP_B1_MASK) >> SC_TOP_REG_SW_FORCE_UP_B1_OFFSET,
		(val & SC_TOP_REG_SW_FORCE_UP_B2_MASK) >> SC_TOP_REG_SW_FORCE_UP_B2_OFFSET);
	uartlog("    sw_force_up_b3=%d, sw_force_up_b4=%d, sw_force_up_b5=%d\n",
		(val & SC_TOP_REG_SW_FORCE_UP_B3_MASK) >> SC_TOP_REG_SW_FORCE_UP_B3_OFFSET,
		(val & SC_TOP_REG_SW_FORCE_UP_B4_MASK) >> SC_TOP_REG_SW_FORCE_UP_B4_OFFSET,
		(val & SC_TOP_REG_SW_FORCE_UP_B5_MASK) >> SC_TOP_REG_SW_FORCE_UP_B5_OFFSET);
	uartlog("    sw_force_up_b6=%d, sw_force_up_b7=%d, sw_mask_up=%d, shrd_sel=%d\n",
		(val & SC_TOP_REG_SW_FORCE_UP_B6_MASK) >> SC_TOP_REG_SW_FORCE_UP_B6_OFFSET,
		(val & SC_TOP_REG_SW_FORCE_UP_B7_MASK) >> SC_TOP_REG_SW_FORCE_UP_B7_OFFSET,
		(val & SC_TOP_REG_SW_MASK_UP_MASK) >> SC_TOP_REG_SW_MASK_UP_OFFSET,
		(val & SC_TOP_REG_SHRD_SEL_MASK) >> SC_TOP_REG_SHRD_SEL_OFFSET);

	uartlog("SC_TOP_REG_05=0x%08x\n", _reg_read(top_base + SC_TOP_REG_05));
	uartlog("SC_TOP_REG_06=0x%08x\n", _reg_read(top_base + SC_TOP_REG_06));

	val = _reg_read(top_base + SC_TOP_REG_SB_CBAR);
	uartlog("SC_TOP_REG_SB_CBAR=0x%08x\n", val);
	uartlog("    sb_wr_ctrl0=%d, sb_wr_ctrl1=%d, sb_rd_ctrl=%d\n",
		(val & SC_TOP_REG_SB_WR_CTRL0_MASK) >> SC_TOP_REG_SB_WR_CTRL0_OFFSET,
		(val & SC_TOP_REG_SB_WR_CTRL1_MASK) >> SC_TOP_REG_SB_WR_CTRL1_OFFSET,
		(val & SC_TOP_REG_SB_RD_CTRL_MASK) >> SC_TOP_REG_SB_RD_CTRL_OFFSET);

	uartlog("SC_TOP_REG_08=0x%08x\n", _reg_read(top_base + SC_TOP_REG_08));

	val = _reg_read(top_base + SC_TOP_REG_0C);
	uartlog("SC_TOP_REG_0C=0x%08x\n", val);
	uartlog("    int_mask=0x%08x\n", val);

	val = _reg_read(top_base + SC_TOP_REG_0D);
	uartlog("SC_TOP_REG_0D=0x%08x\n", val);
	uartlog("    int_status=0x%08x\n", val);

	val = _reg_read(top_base + SC_TOP_REG_0E);
	uartlog("SC_TOP_REG_0E=0x%08x\n", val);
	uartlog("    int_enable=0x%08x\n", val);

	uartlog("SC_TOP_REG_0F=0x%08x\n", _reg_read(top_base + SC_TOP_REG_0F));
	uartlog("SC_TOP_REG_10=0x%08x\n", _reg_read(top_base + SC_TOP_REG_10));
	uartlog("SC_TOP_REG_11=0x%08x\n", _reg_read(top_base + SC_TOP_REG_11));
	uartlog("SC_TOP_REG_12=0x%08x\n", _reg_read(top_base + SC_TOP_REG_12));
	uartlog("SC_TOP_REG_13=0x%08x\n", _reg_read(top_base + SC_TOP_REG_13));
	uartlog("SC_TOP_REG_LVDSTX=0x%08x\n", _reg_read(top_base + SC_TOP_REG_LVDSTX));
	uartlog("SC_TOP_REG_BT_ENC_0=0x%08x\n", _reg_read(top_base + SC_TOP_REG_BT_ENC_0));
	uartlog("SC_TOP_RGG_BT_SYNC_CODE=0x%08x\n", _reg_read(top_base + SC_TOP_RGG_BT_SYNC_CODE));
	uartlog("SC_TOP_REG_BK_BLK_DATA=0x%08x\n", _reg_read(top_base + SC_TOP_REG_BK_BLK_DATA));
	uartlog("SC_TOP_REG_VO_MUX=0x%08x\n", _reg_read(top_base + SC_TOP_REG_VO_MUX));
	uartlog("SC_TOP_REG_SC_074=0x%08x\n", _reg_read(top_base + SC_TOP_REG_SC_074));
	uartlog("SC_TOP_REG_SC_078=0x%08x\n", _reg_read(top_base + SC_TOP_REG_SC_078));
	uartlog("SC_TOP_REG_VO_MUX_EXT=0x%08x\n", _reg_read(top_base + SC_TOP_REG_VO_MUX_EXT));
	uartlog("SC_TOP_REG_VO_MUX_0=0x%08x\n", _reg_read(top_base + SC_TOP_REG_VO_MUX_0));
	uartlog("SC_TOP_REG_VO_MUX_1=0x%08x\n", _reg_read(top_base + SC_TOP_REG_VO_MUX_1));
	uartlog("SC_TOP_REG_VO_MUX_2=0x%08x\n", _reg_read(top_base + SC_TOP_REG_VO_MUX_2));
	uartlog("SC_TOP_REG_VO_MUX_3=0x%08x\n", _reg_read(top_base + SC_TOP_REG_VO_MUX_3));
	uartlog("SC_TOP_REG_VO_MUX_4=0x%08x\n", _reg_read(top_base + SC_TOP_REG_VO_MUX_4));
	uartlog("SC_TOP_REG_VO_MUX_5=0x%08x\n", _reg_read(top_base + SC_TOP_REG_VO_MUX_5));
	uartlog("SC_TOP_REG_VO_MUX_6=0x%08x\n", _reg_read(top_base + SC_TOP_REG_VO_MUX_6));
	uartlog("SC_TOP_REG_VO_MUX_7=0x%08x\n", _reg_read(top_base + SC_TOP_REG_VO_MUX_7));
	uartlog("SC_TOP_REG_VO_MUX_8=0x%08x\n", _reg_read(top_base + SC_TOP_REG_VO_MUX_8));
	uartlog("SC_TOP_REG_VO_MUX_9=0x%08x\n", _reg_read(top_base + SC_TOP_REG_VO_MUX_9));
	uartlog("SC_TOP_REG_VO_MUX_A=0x%08x\n", _reg_read(top_base + SC_TOP_REG_VO_MUX_A));
	uartlog("SC_TOP_REG_RMX_CTRL=0x%08x\n", _reg_read(top_base + SC_TOP_REG_RMX_CTRL));
	uartlog("SC_TOP_REG_FAB_LP=0x%08x\n", _reg_read(top_base + SC_TOP_REG_FAB_LP));
	uartlog("SC_TOP_REG_CLK_EN=0x%08x\n", _reg_read(top_base + SC_TOP_REG_CLK_EN));
	uartlog("SC_TOP_REG_DBG_VALID=0x%08x\n", _reg_read(top_base + SC_TOP_REG_DBG_VALID));
	uartlog("SC_TOP_REG_DBG_READY=0x%08x\n", _reg_read(top_base + SC_TOP_REG_DBG_READY));

	val = _reg_read(top_base + SC_TOP_REG_BLD_CTRL);
	uartlog("SC_TOP_REG_BLD_CTRL=0x%08x\n", val);
	uartlog("    bld_en=%d, bld_fix_alpha=%d, bld_blend_y_only=%d\n",
		(val & SC_TOP_REG_BLD_EN_MASK) >> SC_TOP_REG_BLD_EN_OFFSET,
		(val & SC_TOP_REG_BLD_FIX_ALPHA_MASK) >> SC_TOP_REG_BLD_FIX_ALPHA_OFFSET,
		(val & SC_TOP_REG_BLD_BLEND_Y_ONLY_MASK) >> SC_TOP_REG_BLD_BLEND_Y_ONLY_OFFSET);
	uartlog("    bld_y2r_enable=%d, bld_alpha_factor=%d\n",
		(val & SC_TOP_REG_BLD_Y2R_ENABLE_MASK) >> SC_TOP_REG_BLD_Y2R_ENABLE_OFFSET,
		(val & SC_TOP_REG_BLD_ALPHA_FACTOR_MASK) >> SC_TOP_REG_BLD_ALPHA_FACTOR_OFFSET);

	val = _reg_read(top_base + SC_TOP_REG_BLD_SIZE);
	uartlog("SC_TOP_REG_BLD_SIZE=0x%08x\n", val);
	uartlog("    bld_alpha_stp=%d, bld_wd=%d\n",
		(val & SC_TOP_REG_BLD_ALPHA_STP_MASK) >> SC_TOP_REG_BLD_ALPHA_STP_OFFSET,
		(val & SC_TOP_REG_BLD_WD_MASK) >> SC_TOP_REG_BLD_WD_OFFSET);

	uartlog("SC_TOP_PRI_M_REG_0=0x%08x\n", _reg_read(top_base + SC_TOP_PRI_M_REG_0));
	uartlog("SC_TOP_PRI_M_REG_1=0x%08x\n", _reg_read(top_base + SC_TOP_PRI_M_REG_1));
	uartlog("SC_TOP_PRI_M_REG_2=0x%08x\n", _reg_read(top_base + SC_TOP_PRI_M_REG_2));
	uartlog("SC_TOP_PRI_M_REG_3=0x%08x\n", _reg_read(top_base + SC_TOP_PRI_M_REG_3));
	uartlog("SC_TOP_PRI_M_REG_4=0x%08x\n", _reg_read(top_base + SC_TOP_PRI_M_REG_4));
	uartlog("SC_TOP_PRI_M_REG_5=0x%08x\n", _reg_read(top_base + SC_TOP_PRI_M_REG_5));
	uartlog("SC_TOP_PRI_M_REG_6=0x%08x\n", _reg_read(top_base + SC_TOP_PRI_M_REG_6));
	uartlog("SC_TOP_PRI_M_REG_8=0x%08x\n", _reg_read(top_base + SC_TOP_PRI_M_REG_8));
}

void sclr_dump_disp_register(void)
{
	u32 disp_base = reg_base + REG_SCL_DISP_BASE;

	uartlog("disp base address=0x%08x\n", disp_base);
	uartlog("DISP_REG_00=0x%08x\n", _reg_read(disp_base + DISP_REG_00));
	uartlog("DISP_REG_01=0x%08x\n", _reg_read(disp_base + DISP_REG_01));
	uartlog("DISP_REG_02=0x%08x\n", _reg_read(disp_base + DISP_REG_02));
	uartlog("DISP_REG_03=0x%08x\n", _reg_read(disp_base + DISP_REG_03));
	uartlog("DISP_REG_04=0x%08x\n", _reg_read(disp_base + DISP_REG_04));
	uartlog("DISP_REG_05=0x%08x\n", _reg_read(disp_base + DISP_REG_05));
	uartlog("DISP_REG_06=0x%08x\n", _reg_read(disp_base + DISP_REG_06));
	uartlog("DISP_REG_07=0x%08x\n", _reg_read(disp_base + DISP_REG_07));
	uartlog("DISP_REG_08=0x%08x\n", _reg_read(disp_base + DISP_REG_08));
	uartlog("DISP_REG_12=0x%08x\n", _reg_read(disp_base + DISP_REG_12));
	uartlog("DISP_REG_13=0x%08x\n", _reg_read(disp_base + DISP_REG_13));
	uartlog("DISP_REG_14=0x%08x\n", _reg_read(disp_base + DISP_REG_14));
	uartlog("DISP_REG_15=0x%08x\n", _reg_read(disp_base + DISP_REG_15));
	uartlog("DISP_REG_16=0x%08x\n", _reg_read(disp_base + DISP_REG_16));
	uartlog("DISP_REG_17=0x%08x\n", _reg_read(disp_base + DISP_REG_17));
	uartlog("DISP_REG_18=0x%08x\n", _reg_read(disp_base + DISP_REG_18));
	uartlog("DISP_REG_19=0x%08x\n", _reg_read(disp_base + DISP_REG_19));
	uartlog("DISP_REG_20=0x%08x\n", _reg_read(disp_base + DISP_REG_20));
	uartlog("DISP_REG_21=0x%08x\n", _reg_read(disp_base + DISP_REG_21));
	uartlog("DISP_REG_22=0x%08x\n", _reg_read(disp_base + DISP_REG_22));
	uartlog("DISP_REG_23=0x%08x\n", _reg_read(disp_base + DISP_REG_23));
	uartlog("DISP_REG_24=0x%08x\n", _reg_read(disp_base + DISP_REG_24));
	uartlog("DISP_REG_25=0x%08x\n", _reg_read(disp_base + DISP_REG_25));
	uartlog("DISP_REG_26=0x%08x\n", _reg_read(disp_base + DISP_REG_26));
	uartlog("DISP_REG_27=0x%08x\n", _reg_read(disp_base + DISP_REG_27));
	uartlog("DISP_REG_28=0x%08x\n", _reg_read(disp_base + DISP_REG_28));
	uartlog("DISP_REG_29=0x%08x\n", _reg_read(disp_base + DISP_REG_29));
	uartlog("DISP_REG_30=0x%08x\n", _reg_read(disp_base + DISP_REG_30));
	uartlog("DISP_REG_31=0x%08x\n", _reg_read(disp_base + DISP_REG_31));
	uartlog("DISP_REG_32=0x%08x\n", _reg_read(disp_base + DISP_REG_32));
	uartlog("DISP_REG_33=0x%08x\n", _reg_read(disp_base + DISP_REG_33));
	uartlog("DISP_REG_34=0x%08x\n", _reg_read(disp_base + DISP_REG_34));
	uartlog("DISP_REG_35=0x%08x\n", _reg_read(disp_base + DISP_REG_35));
	uartlog("DISP_REG_36=0x%08x\n", _reg_read(disp_base + DISP_REG_36));
	uartlog("DISP_REG_37=0x%08x\n", _reg_read(disp_base + DISP_REG_37));
	uartlog("DISP_REG_38=0x%08x\n", _reg_read(disp_base + DISP_REG_38));
	uartlog("DISP_REG_39=0x%08x\n", _reg_read(disp_base + DISP_REG_39));
	uartlog("DISP_REG_40=0x%08x\n", _reg_read(disp_base + DISP_REG_40));
	uartlog("DISP_REG_41=0x%08x\n", _reg_read(disp_base + DISP_REG_41));
	uartlog("DISP_REG_42=0x%08x\n", _reg_read(disp_base + DISP_REG_42));
	uartlog("DISP_REG_43=0x%08x\n", _reg_read(disp_base + DISP_REG_43));
	uartlog("DISP_REG_AXI_ST=0x%08x\n", _reg_read(disp_base + DISP_REG_AXI_ST));
	uartlog("DISP_REG_CATCH=0x%08x\n", _reg_read(disp_base + DISP_REG_CATCH));
	uartlog("DISP_REG_CHK_CTRL=0x%08x\n", _reg_read(disp_base + DISP_REG_CHK_CTRL));
	uartlog("DISP_CHK_RD_OSD=0x%08x\n", _reg_read(disp_base + DISP_CHK_RD_OSD));
	uartlog("DISP_CHK_RD_IMG=0x%08x\n", _reg_read(disp_base + DISP_CHK_RD_IMG));
	uartlog("DISP_REG_3E=0x%08x\n", _reg_read(disp_base + DISP_REG_3E));
	uartlog("DISP_REG_3F=0x%08x\n", _reg_read(disp_base + DISP_REG_3F));
	uartlog("DISP_REG_IMG_BWL=0x%08x\n", _reg_read(disp_base + DISP_REG_IMG_BWL));
	uartlog("DISP_REG_GAMMA_CTRL=0x%08x\n", _reg_read(disp_base + DISP_REG_GAMMA_CTRL));
	uartlog("DISP_REG_GAMMA_WR_LUT=0x%08x\n", _reg_read(disp_base + DISP_REG_GAMMA_WR_LUT));
	uartlog("DISP_REG_GAMMA_RD_LUT=0x%08x\n", _reg_read(disp_base + DISP_REG_GAMMA_RD_LUT));
	uartlog("DISP_REG_MCU_IF_CTRL=0x%08x\n", _reg_read(disp_base + DISP_REG_MCU_IF_CTRL));
	uartlog("DISP_REG_MCU_SW_CTRL=0x%08x\n", _reg_read(disp_base + DISP_REG_MCU_SW_CTRL));
	uartlog("DISP_REG_MCU_STATUS=0x%08x\n", _reg_read(disp_base + DISP_REG_MCU_STATUS));
	uartlog("DISP_REG_HW_MCU_AUTO=0x%08x\n", _reg_read(disp_base + DISP_REG_HW_MCU_AUTO));
	uartlog("DISP_REG_HW_MCU_CMD=0x%08x\n", _reg_read(disp_base + DISP_REG_HW_MCU_CMD));
	uartlog("DISP_REG_HW_MCU_CMD_0=0x%08x\n", _reg_read(disp_base + DISP_REG_HW_MCU_CMD_0));
	uartlog("DISP_REG_HW_MCU_CMD_1=0x%08x\n", _reg_read(disp_base + DISP_REG_HW_MCU_CMD_1));
	uartlog("DISP_REG_HW_MCU_CMD_2=0x%08x\n", _reg_read(disp_base + DISP_REG_HW_MCU_CMD_2));
	uartlog("DISP_REG_HW_MCU_CMD_3=0x%08x\n", _reg_read(disp_base + DISP_REG_HW_MCU_CMD_3));
	uartlog("DISP_REG_HW_MCU_CMD_4=0x%08x\n", _reg_read(disp_base + DISP_REG_HW_MCU_CMD_4));
	uartlog("DISP_REG_HW_MCU_CMD_5=0x%08x\n", _reg_read(disp_base + DISP_REG_HW_MCU_CMD_5));
	uartlog("DISP_REG_HW_MCU_CMD_6=0x%08x\n", _reg_read(disp_base + DISP_REG_HW_MCU_CMD_6));
	uartlog("DISP_REG_HW_MCU_CMD_7=0x%08x\n", _reg_read(disp_base + DISP_REG_HW_MCU_CMD_7));
	uartlog("DISP_REG_HW_MCU_OV=0x%08x\n", _reg_read(disp_base + DISP_REG_HW_MCU_OV));
	uartlog("DISP_REG_SRGB_CTRL=0x%08x\n", _reg_read(disp_base + DISP_REG_SRGB_CTRL));
	uartlog("DISP_REG_TGEN_LITE_SIZE=0x%08x\n", _reg_read(disp_base + DISP_REG_TGEN_LITE_SIZE));
	uartlog("DISP_REG_TGEN_LITE_VS=0x%08x\n", _reg_read(disp_base + DISP_REG_TGEN_LITE_VS));
	uartlog("DISP_REG_TGEN_LITE_HS=0x%08x\n", _reg_read(disp_base + DISP_REG_TGEN_LITE_HS));
}

void sclr_dump_img_in_register(int img_inst)
{
	u32 img_base = reg_base + REG_SCL_IMG_BASE(img_inst);
	u32 val;

	if (img_inst >= 2)
		uartlog("img(%d) out of range\n", img_inst);

	uartlog("img(%d) base addr 0x%08x\n", img_inst, img_base);

	val = _reg_read(img_base + IMG_IN_REG_00);
	uartlog("IMG_IN_REG_00=0x%08x\n", val);
	uartlog("    src_sel=%d, fmt_sel=%d, burst_len=%d, img_csc_en=%d\n",
		(val & IMG_IN_REG_SRC_SEL_MASK) >> IMG_IN_REG_SRC_SEL_OFFSET,
		(val & IMG_IN_REG_FMT_SEL_MASK) >> IMG_IN_REG_FMT_SEL_OFFSET,
		(val & IMG_IN_REG_BURST_LN_MASK) >> IMG_IN_REG_BURST_LN_OFFSET,
		(val & IMG_IN_REG_IMG_CSC_EN_MASK) >> IMG_IN_REG_IMG_CSC_EN_OFFSET);
	uartlog("    auto_csc_en=%d, 64b_align=%d, force_clk_enable=%d\n",
		(val & IMG_IN_REG_AUTO_CSC_EN_MASK) >> IMG_IN_REG_AUTO_CSC_EN_OFFSET,
		(val & IMG_IN_REG_64B_ALIGN_MASK) >> IMG_IN_REG_64B_ALIGN_OFFSET,
		(val & IMG_IN_REG_FORCE_CLK_ENABLE_MASK) >> IMG_IN_REG_FORCE_CLK_ENABLE_OFFSET);

	val = _reg_read(img_base + IMG_IN_REG_01);
	uartlog("IMG_IN_REG_01=0x%08x\n", val);
	uartlog("    src_x_str=%d, src_y_str=%d\n",
		(val & IMG_IN_REG_SRC_X_STR_MASK) >> IMG_IN_REG_SRC_X_STR_OFFSET,
		(val & IMG_IN_REG_SRC_Y_STR_MASK) >> IMG_IN_REG_SRC_Y_STR_OFFSET);

	val = _reg_read(img_base + IMG_IN_REG_02);
	uartlog("IMG_IN_REG_02=0x%08x\n", val);
	uartlog("    src_wd=%d, src_ht=%d\n",
		(val & IMG_IN_REG_SRC_WD_MASK) >> IMG_IN_REG_SRC_WD_OFFSET,
		(val & IMG_IN_REG_SRC_HT_MASK) >> IMG_IN_REG_SRC_HT_OFFSET);

	val = _reg_read(img_base + IMG_IN_REG_03);
	uartlog("IMG_IN_REG_03=0x%08x\n", val);
	uartlog("    src_y_pitch=%d\n",
		(val & IMG_IN_REG_SRC_Y_PITCH_MASK) >> IMG_IN_REG_SRC_Y_PITCH_OFFSET);

	val = _reg_read(img_base + IMG_IN_REG_04);
	uartlog("IMG_IN_REG_04=0x%08x\n", val);
	uartlog("    src_c_pitch=%d\n",
		(val & IMG_IN_REG_SRC_C_PITCH_MASK) >> IMG_IN_REG_SRC_C_PITCH_OFFSET);

	val = _reg_read(img_base + IMG_IN_REG_05);
	uartlog("IMG_IN_REG_05=0x%08x\n", val);
	uartlog("    sw_force_up=%d, sw_mask_up=%d, shrd_sel=%d\n",
		(val & IMG_IN_REG_SW_FORCE_UP_MASK) >> IMG_IN_REG_SW_FORCE_UP_OFFSET,
		(val & IMG_IN_REG_SW_MASK_UP_MASK) >> IMG_IN_REG_SW_MASK_UP_OFFSET,
		(val & IMG_IN_REG_SHRD_SEL_MASK) >> IMG_IN_REG_SHRD_SEL_OFFSET);

	uartlog("IMG_IN_REG_06=0x%08x\n", _reg_read(img_base + IMG_IN_REG_06));
	uartlog("IMG_IN_REG_07=0x%08x\n", _reg_read(img_base + IMG_IN_REG_07));
	uartlog("IMG_IN_REG_08=0x%08x\n", _reg_read(img_base + IMG_IN_REG_08));
	uartlog("IMG_IN_REG_Y_BASE_0=0x%08x\n", _reg_read(img_base + IMG_IN_REG_Y_BASE_0));
	uartlog("IMG_IN_REG_Y_BASE_1=0x%08x\n", _reg_read(img_base + IMG_IN_REG_Y_BASE_1));
	uartlog("IMG_IN_REG_U_BASE_0=0x%08x\n", _reg_read(img_base + IMG_IN_REG_U_BASE_0));
	uartlog("IMG_IN_REG_U_BASE_1=0x%08x\n", _reg_read(img_base + IMG_IN_REG_U_BASE_1));
	uartlog("IMG_IN_REG_V_BASE_0=0x%08x\n", _reg_read(img_base + IMG_IN_REG_V_BASE_0));
	uartlog("IMG_IN_REG_V_BASE_1=0x%08x\n", _reg_read(img_base + IMG_IN_REG_V_BASE_1));
	uartlog("IMG_IN_REG_040=0x%08x\n", _reg_read(img_base + IMG_IN_REG_040));
	uartlog("IMG_IN_REG_044=0x%08x\n", _reg_read(img_base + IMG_IN_REG_044));
	uartlog("IMG_IN_REG_048=0x%08x\n", _reg_read(img_base + IMG_IN_REG_048));
	uartlog("IMG_IN_REG_04C=0x%08x\n", _reg_read(img_base + IMG_IN_REG_04C));
	uartlog("IMG_IN_REG_050=0x%08x\n", _reg_read(img_base + IMG_IN_REG_050));
	uartlog("IMG_IN_REG_054=0x%08x\n", _reg_read(img_base + IMG_IN_REG_054));
	uartlog("IMG_IN_REG_058=0x%08x\n", _reg_read(img_base + IMG_IN_REG_058));
	uartlog("IMG_IN_REG_05C=0x%08x\n", _reg_read(img_base + IMG_IN_REG_05C));
	uartlog("IMG_IN_REG_060=0x%08x\n", _reg_read(img_base + IMG_IN_REG_060));
	uartlog("IMG_IN_REG_064=0x%08x\n", _reg_read(img_base + IMG_IN_REG_064));
	uartlog("IMG_IN_REG_068=0x%08x\n", _reg_read(img_base + IMG_IN_REG_068));
	uartlog("IMG_IN_REG_AXI_ST=0x%08x\n", _reg_read(img_base + IMG_IN_REG_AXI_ST));
	uartlog("IMG_IN_REG_BW_LIMIT=0x%08x\n", _reg_read(img_base + IMG_IN_REG_BW_LIMIT));
	uartlog("IMG_IN_REG_CATCH=0x%08x\n", _reg_read(img_base + IMG_IN_REG_CATCH));
	uartlog("IMG_IN_REG_CHK_CTRL=0x%08x\n", _reg_read(img_base + IMG_IN_REG_CHK_CTRL));
	uartlog("IMG_IN_CHKSUM_AXI_RD=0x%08x\n", _reg_read(img_base + IMG_IN_CHKSUM_AXI_RD));

	val = _reg_read(img_base + IMG_IN_SB_REG_CTRL);
	uartlog("IMG_IN_SB_REG_CTRL=0x%08x\n", val);
	uartlog("    sb_mode=%d, sb_size=%d, sb_nb=%d, sb_sw_rptr=%d\n",
		(val & IMG_IN_REG_SB_MODE_MASK) >> IMG_IN_REG_SB_MODE_OFFSET,
		(val & IMG_IN_REG_SB_SIZE_MASK) >> IMG_IN_REG_SB_SIZE_OFFSET,
		(val & IMG_IN_REG_SB_NB_MASK) >> IMG_IN_REG_SB_NB_OFFSET,
		(val & IMG_IN_REG_SB_SW_RPTR_MASK) >> IMG_IN_REG_SB_SW_RPTR_OFFSET);

	uartlog("IMG_IN_SB_REG_C_STAT=0x%08x\n", _reg_read(img_base + IMG_IN_SB_REG_C_STAT));
	uartlog("IMG_IN_SB_REG_Y_STAT=0x%08x\n", _reg_read(img_base + IMG_IN_SB_REG_Y_STAT));
}

void sclr_dump_core_register(int inst)
{
	u32 val;
	u32 core_base = reg_base + REG_SCL_CORE_BASE(inst);

	if (inst >= SCL_MAX_INST)
		uartlog("sc(%d) out of range\n", inst);

	uartlog("sc(%d) core base address=0x%08x\n", inst, core_base);

	val = _reg_read(core_base + SC_CORE_SC_WRAP_REG_0);
	uartlog("SC_CORE_SC_WRAP_REG_0=0x%08x\n", val);
	uartlog("    sc_core_bypass_en=%d, vgop_bypass_en=%d, dbg_en=%d\n",
		(val & SC_CORE_REG_SC_CORE_BYPASS_EN_MASK) >> SC_CORE_REG_SC_CORE_BYPASS_EN_OFFSET,
		(val & SC_CORE_REG_VGOP_BYPASS_EN_MASK) >> SC_CORE_REG_VGOP_BYPASS_EN_OFFSET,
		(val & SC_CORE_REG_DBG_EN_MASK) >> SC_CORE_REG_DBG_EN_OFFSET);
	uartlog("    cir_bypass_en=%d, dma_bypass_en=%d, force_clk_enable=%d\n",
		(val & SC_CORE_REG_CIR_BYPASS_EN_MASK) >> SC_CORE_REG_CIR_BYPASS_EN_OFFSET,
		(val & SC_CORE_REG_DMA_BYPASS_EN_MASK) >> SC_CORE_REG_DMA_BYPASS_EN_OFFSET,
		(val & SC_CORE_REG_FORCE_CLK_ENABLE_MASK) >> SC_CORE_REG_FORCE_CLK_ENABLE_OFFSET);

	val = _reg_read(core_base + SC_CORE_SC_WRAP_REG_1);
	uartlog("SC_CORE_SC_WRAP_REG_1=0x%08x\n", val);
	uartlog("    shdw_read_sel=%d\n",
		(val & SC_CORE_REG_SHDW_READ_SEL_MASK) >> SC_CORE_REG_SHDW_READ_SEL_OFFSET);

	val = _reg_read(core_base + SC_CORE_SC_WRAP_REG_2);
	uartlog("SC_CORE_SC_WRAP_REG_2=0x%08x\n", val);
	uartlog("    crop_idle=%d, hscale_idle=%d, vscale_idle=%d, vgop_idle=%d, sc_wdma_idle=%d\n",
		(val & SC_CORE_REG_CROP_IDLE_MASK) >> SC_CORE_REG_CROP_IDLE_OFFSET,
		(val & SC_CORE_REG_HSCALE_IDLE_MASK) >> SC_CORE_REG_HSCALE_IDLE_OFFSET,
		(val & SC_CORE_REG_VSCALE_IDLE_MASK) >> SC_CORE_REG_VSCALE_IDLE_OFFSET,
		(val & SC_CORE_REG_VGOP_IDLE_MASK) >> SC_CORE_REG_VGOP_IDLE_OFFSET,
		(val & SC_CORE_REG_SC_WDMA_IDLE_MASK) >> SC_CORE_REG_SC_WDMA_IDLE_OFFSET);

	uartlog("SC_CORE_SC_CROP_REG_0=0x%08x\n", _reg_read(core_base + SC_CORE_SC_CROP_REG_0));
	uartlog("SC_CORE_SC_CROP_REG_1=0x%08x\n", _reg_read(core_base + SC_CORE_SC_CROP_REG_1));
	uartlog("SC_CORE_SC_CROP_REG_2=0x%08x\n", _reg_read(core_base + SC_CORE_SC_CROP_REG_2));
	uartlog("SC_CORE_SC_CROP_REG_3=0x%08x\n", _reg_read(core_base + SC_CORE_SC_CROP_REG_3));

	val = _reg_read(core_base + SC_CORE_SC_CORE_CFG);
	uartlog("SC_CORE_SC_CORE_CFG=0x%08x\n", val);
	uartlog("    tap number=%d, Version=%d, line buffer width=%d\n",
		(val & 0xf0000000) >> 28,
		(val & 0x0f000000) >> 24,
		(val & 0xffff));

	val = _reg_read(core_base + SC_CORE_SC_2TAP_CFG);
	uartlog("SC_CORE_SC_2TAP_CFG=0x%08x\n", val);
	uartlog("    resize_blnr_mode=%d, resize_area_fast=%d, resize_dbg_sel=%d\n",
		(val & SC_CORE_REG_RESIZE_BLNR_MODE_MASK) >> SC_CORE_REG_RESIZE_BLNR_MODE_OFFSET,
		(val & SC_CORE_REG_RESIZE_AREA_FAST_MASK) >> SC_CORE_REG_RESIZE_AREA_FAST_OFFSET,
		(val & SC_CORE_REG_RESIZE_DBG_SEL_MASK) >> SC_CORE_REG_RESIZE_DBG_SEL_OFFSET);

	val = _reg_read(core_base + SC_CORE_SC_2TAP_NOR);
	uartlog("SC_CORE_SC_2TAP_NOR=0x%08x\n", val);
	uartlog("    resize_h_nor=0x%04x, resize_v_nor=0x%04x\n",
		(val & SC_CORE_REG_RESIZE_H_NOR_MASK) >> SC_CORE_REG_RESIZE_H_NOR_OFFSET,
		(val & SC_CORE_REG_RESIZE_V_NOR_MASK) >> SC_CORE_REG_RESIZE_V_NOR_OFFSET);

	uartlog("SC_CORE_SC_2_TAP_DBG=0x%08x\n", _reg_read(core_base + SC_CORE_SC_2_TAP_DBG));

	val = _reg_read(core_base + SC_CORE_BORDER_0);
	uartlog("SC_CORE_BORDER_0=0x%08x\n", val);
	uartlog("    bd_r=%d, bd_g=%d, bd_b=%d, border_en=%d\n",
		(val & SC_CORE_REG_BD_R_MASK) >> SC_CORE_REG_BD_R_OFFSET,
		(val & SC_CORE_REG_BD_G_MASK) >> SC_CORE_REG_BD_G_OFFSET,
		(val & SC_CORE_REG_BD_B_MASK) >> SC_CORE_REG_BD_B_OFFSET,
		(val & SC_CORE_REG_BORDER_EN_MASK) >> SC_CORE_REG_BORDER_EN_OFFSET);

	val = _reg_read(core_base + SC_CORE_BORDER_1);
	uartlog("SC_CORE_BORDER_1=0x%08x\n", val);
	uartlog("    border_x_str=%d, border_y_str=%d\n",
		(val & SC_CORE_REG_BORDER_X_STR_MASK) >> SC_CORE_REG_BORDER_X_STR_OFFSET,
		(val & SC_CORE_REG_BORDER_Y_STR_MASK) >> SC_CORE_REG_BORDER_Y_STR_OFFSET);

	uartlog("SC_CORE_REG_CHK_CTRL=0x%08x\n", _reg_read(core_base + SC_CORE_REG_CHK_CTRL));
	uartlog("SC_CORE_CHK_AXI_VGOP_0=0x%08x\n", _reg_read(core_base + SC_CORE_CHK_AXI_VGOP_0));
	uartlog("SC_CORE_CHK_AXI_VGOP_1=0x%08x\n", _reg_read(core_base + SC_CORE_CHK_AXI_VGOP_1));
	uartlog("SC_CORE_CHK_AXI_WDATA=0x%08x\n", _reg_read(core_base + SC_CORE_CHK_AXI_WDATA));
	uartlog("SC_CORE_SC_CORE_REG_0=0x%08x\n", _reg_read(core_base + SC_CORE_SC_CORE_REG_0));
	uartlog("SC_CORE_SC_CORE_REG_1=0x%08x\n", _reg_read(core_base + SC_CORE_SC_CORE_REG_1));
	uartlog("SC_CORE_SC_CORE_REG_2=0x%08x\n", _reg_read(core_base + SC_CORE_SC_CORE_REG_2));
	uartlog("SC_CORE_SC_CORE_REG_3=0x%08x\n", _reg_read(core_base + SC_CORE_SC_CORE_REG_3));
	uartlog("SC_CORE_SC_CORE_REG_4=0x%08x\n", _reg_read(core_base + SC_CORE_SC_CORE_REG_4));
	uartlog("SC_CORE_SC_CORE_REG_5=0x%08x\n", _reg_read(core_base + SC_CORE_SC_CORE_REG_5));
	uartlog("SC_CORE_SC_CORE_REG_6=0x%08x\n", _reg_read(core_base + SC_CORE_SC_CORE_REG_6));

	uartlog("SC_CORE_SC_CORE_REG_7=0x%08x\n", _reg_read(core_base + SC_CORE_SC_CORE_REG_7));
	uartlog("    sc_h_str_mir=%d, sc_v_str_mir=%d, sc_cb_mode_en=%d\n",
		(val & SC_CORE_REG_SC_H_STR_MIR_MASK) >> SC_CORE_REG_SC_H_STR_MIR_OFFSET,
		(val & SC_CORE_REG_SC_V_STR_MIR_MASK) >> SC_CORE_REG_SC_V_STR_MIR_OFFSET,
		(val & SC_CORE_REG_SC_CB_MODE_EN_MASK) >> SC_CORE_REG_SC_CB_MODE_EN_OFFSET);

	val = _reg_read(core_base + SC_CORE_SC_CORE_REG_8);
	uartlog("SC_CORE_SC_CORE_REG_8=0x%08x\n", val);
	uartlog("    h_sc_fac=0x%x\n",
		(val & SC_CORE_REG_H_SC_FAC_MASK) >> SC_CORE_REG_H_SC_FAC_OFFSET);

	val = _reg_read(core_base + SC_CORE_SC_CORE_REG_9);
	uartlog("SC_CORE_SC_CORE_REG_9=0x%08x\n", val);
	uartlog("    v_sc_fac=0x%x\n",
		(val & SC_CORE_REG_V_SC_FAC_MASK) >> SC_CORE_REG_V_SC_FAC_OFFSET);

	val = _reg_read(core_base + SC_CORE_SC_CORE_REG_10);
	uartlog("SC_CORE_SC_CORE_REG_10=0x%08x\n", val);
	uartlog("    sc_core_wd=%d, sc_core_ht=%d\n",
		(val & SC_CORE_REG_SC_CORE_WD_MASK) >> SC_CORE_REG_SC_CORE_WD_OFFSET,
		(val & SC_CORE_REG_SC_CORE_HT_MASK) >> SC_CORE_REG_SC_CORE_HT_OFFSET);

	uartlog("SC_CORE_SC_CORE_REG_11=0x%08x\n", _reg_read(core_base + SC_CORE_SC_CORE_REG_11));

	val =  _reg_read(core_base + SC_CORE_SC_CORE_REG_12);
	uartlog("SC_CORE_SC_CORE_REG_12=0x%08x\n", val);
	uartlog("    h_ini_ph=0x%04x, v_ini_ph=0x%04x\n",
		(val & SC_CORE_REG_H_INI_PH_MASK) >> SC_CORE_REG_H_INI_PH_OFFSET,
		(val & SC_CORE_REG_V_INI_PH_MASK) >> SC_CORE_REG_V_INI_PH_OFFSET);

	uartlog("SC_CORE_BLT_CIR_REG_0=0x%08x\n", _reg_read(core_base + SC_CORE_BLT_CIR_REG_0));
	uartlog("SC_CORE_BLT_CIR_REG_1=0x%08x\n", _reg_read(core_base + SC_CORE_BLT_CIR_REG_1));
	uartlog("SC_CORE_BLT_CIR_REG_2=0x%08x\n", _reg_read(core_base + SC_CORE_BLT_CIR_REG_2));
	uartlog("SC_CORE_BLT_CIR_REG_3=0x%08x\n", _reg_read(core_base + SC_CORE_BLT_CIR_REG_3));
	uartlog("SC_CORE_BLT_CIR_REG_4=0x%08x\n", _reg_read(core_base + SC_CORE_BLT_CIR_REG_4));
	uartlog("SC_CORE_BLT_CIR_REG_5=0x%08x\n", _reg_read(core_base + SC_CORE_BLT_CIR_REG_5));
	uartlog("SC_CORE_BLT_CIR_REG_6=0x%08x\n", _reg_read(core_base + SC_CORE_BLT_CIR_REG_6));

	val = _reg_read(core_base + SC_CORE_PRI_M_REG_0);
	uartlog("SC_CORE_PRI_M_REG_0=0x%08x\n", val);
	uartlog("    pri_m_en=%d, pri_m_pixel_md=%d, pri_force_alpha=%d\n",
		(val & SC_CORE_REG_PRI_M_EN_MASK) >> SC_CORE_REG_PRI_M_EN_OFFSET,
		(val & SC_CORE_REG_PRI_M_PIXEL_MD_MASK) >> SC_CORE_REG_PRI_M_PIXEL_MD_OFFSET,
		(val & SC_CORE_REG_PRI_FORCE_ALPHA_MASK) >> SC_CORE_REG_PRI_FORCE_ALPHA_OFFSET);
	uartlog("    pri_mask_rgb332=%d, pri_blend_y_only=%d, pri_y2r_enable=%d\n",
		(val & SC_CORE_REG_PRI_MASK_RGB332_MASK) >> SC_CORE_REG_PRI_MASK_RGB332_OFFSET,
		(val & SC_CORE_REG_PRI_BLEND_Y_ONLY_MASK) >> SC_CORE_REG_PRI_BLEND_Y_ONLY_OFFSET,
		(val & SC_CORE_REG_PRI_Y2R_ENABLE_MASK) >> SC_CORE_REG_PRI_Y2R_ENABLE_OFFSET);
	uartlog(" pri_grid_size=%d, pri_m_fit_p=%d\n",
		(val & SC_CORE_REG_PRI_GRID_SIZE_MASK) >> SC_CORE_REG_PRI_GRID_SIZE_OFFSET,
		(val & SC_CORE_REG_PRI_M_FIT_P_MASK) >> SC_CORE_REG_PRI_M_FIT_P_OFFSET);

	uartlog("SC_CORE_PRI_M_REG_1=0x%08x\n", _reg_read(core_base + SC_CORE_PRI_M_REG_1));
	uartlog("SC_CORE_PRI_M_REG_2=0x%08x\n", _reg_read(core_base + SC_CORE_PRI_M_REG_2));
	uartlog("SC_CORE_PRI_M_REG_3=0x%08x\n", _reg_read(core_base + SC_CORE_PRI_M_REG_3));
	uartlog("SC_CORE_PRI_M_REG_4=0x%08x\n", _reg_read(core_base + SC_CORE_PRI_M_REG_4));
	uartlog("SC_CORE_PRI_M_REG_5=0x%08x\n", _reg_read(core_base + SC_CORE_PRI_M_REG_5));
	uartlog("SC_CORE_PRI_M_REG_6=0x%08x\n", _reg_read(core_base + SC_CORE_PRI_M_REG_6));
	uartlog("SC_CORE_PRI_M_REG_7=0x%08x\n", _reg_read(core_base + SC_CORE_PRI_M_REG_7));
}

void sclr_dump_odma_register(u8 inst)
{
	u32 val;
	u32 odma_base = reg_base + REG_SCL_ODMA_BASE(inst);

	if (inst >= SCL_MAX_INST)
		uartlog("sc(%d) out of range\n", inst);

	uartlog("sc(%d) odma base address=0x%08x\n", inst, odma_base);

	val = _reg_read(odma_base + SC_ODMA_ODMA_REG_00);
	uartlog("SC_ODMA_ODMA_REG_00=0x%08x\n", val);
	uartlog("    blen=%d, fmt_sel=%d, hflip=%d, vflip=%d\n",
		(val & SC_ODMA_REG_DMA_BLEN_MASK) >> SC_ODMA_REG_DMA_BLEN_OFFSET,
		(val & SC_ODMA_REG_FMT_SEL_MASK) >> SC_ODMA_REG_FMT_SEL_OFFSET,
		(val & SC_ODMA_REG_SC_ODMA_HFLIP_MASK) >> SC_ODMA_REG_SC_ODMA_HFLIP_OFFSET,
		(val & SC_ODMA_REG_SC_ODMA_VFLIP_MASK) >> SC_ODMA_REG_SC_ODMA_VFLIP_OFFSET);
	uartlog("    422_avg=%d, 420_avg=%d, c_round_mode=%d, bf16_en=%d\n",
		(val & SC_ODMA_REG_SC_422_AVG_MASK) >> SC_ODMA_REG_SC_422_AVG_OFFSET,
		(val & SC_ODMA_REG_SC_420_AVG_MASK) >> SC_ODMA_REG_SC_420_AVG_OFFSET,
		(val & SC_ODMA_REG_C_ROUND_MODE_MASK) >> SC_ODMA_REG_C_ROUND_MODE_OFFSET,
		(val & SC_ODMA_REG_BF16_EN_MASK) >> SC_ODMA_REG_BF16_EN_OFFSET);

	val = _reg_read(odma_base + SC_ODMA_ODMA_REG_01);
	uartlog("SC_ODMA_ODMA_REG_01=0x%08x\n", val);
	uartlog ("    reg_dma_y_base_low_part=0x%08x\n", val);

	uartlog("SC_ODMA_ODMA_REG_02=0x%08x\n", _reg_read(odma_base + SC_ODMA_ODMA_REG_02));

	val = _reg_read(odma_base + SC_ODMA_ODMA_REG_03);
	uartlog("SC_ODMA_ODMA_REG_03=0x%08x\n", val);
	uartlog("    reg_dma_u_base_low_part=0x%08x\n", val);

	uartlog("SC_ODMA_ODMA_REG_04=0x%08x\n", _reg_read(odma_base + SC_ODMA_ODMA_REG_04));

	val = _reg_read(odma_base + SC_ODMA_ODMA_REG_05);
	uartlog("SC_ODMA_ODMA_REG_05=0x%08x\n", val);
	uartlog("    reg_dma_v_base_low_part0x%08x\n", val);

	uartlog("SC_ODMA_ODMA_REG_06=0x%08x\n", _reg_read(odma_base + SC_ODMA_ODMA_REG_06));

	val = _reg_read(odma_base + SC_ODMA_ODMA_REG_07);
	uartlog("SC_ODMA_ODMA_REG_07=0x%08x\n", val);
	uartlog("    reg_dma_y_pitch=%d\n",
		(val & SC_ODMA_REG_DMA_Y_PITCH_MASK) >> SC_ODMA_REG_DMA_Y_PITCH_OFFSET);

	val = _reg_read(odma_base + SC_ODMA_ODMA_REG_08);
	uartlog("SC_ODMA_ODMA_REG_08=0x%08x\n", val);
	uartlog("    reg_dma_c_pitch=%d\n",
		(val & SC_ODMA_REG_DMA_Y_PITCH_MASK) >> SC_ODMA_REG_DMA_Y_PITCH_OFFSET);

	val = _reg_read(odma_base + SC_ODMA_ODMA_REG_09);
	uartlog("SC_ODMA_ODMA_REG_09=0x%08x\n", val);
	uartlog("    reg_dma_x_str=%d\n",
		(val & SC_ODMA_REG_DMA_X_STR_MASK) >> SC_ODMA_REG_DMA_X_STR_OFFSET);

	val = _reg_read(odma_base + SC_ODMA_ODMA_REG_10);
	uartlog("SC_ODMA_ODMA_REG_10=0x%08x\n", val);
	uartlog("    reg_dma_y_str=%d\n",
		(val & SC_ODMA_REG_DMA_Y_STR_MASK) >> SC_ODMA_REG_DMA_Y_STR_OFFSET);

	val = _reg_read(odma_base + SC_ODMA_ODMA_REG_11);
	uartlog("SC_ODMA_ODMA_REG_11=0x%08x\n", val);
	uartlog("    reg_dma_wd=%d\n",
		(val & SC_ODMA_REG_DMA_WD_MASK) >> SC_ODMA_REG_DMA_WD_OFFSET);

	val = _reg_read(odma_base + SC_ODMA_ODMA_REG_12);
	uartlog("SC_ODMA_ODMA_REG_12=0x%08x\n", val);
	uartlog("    reg_dma_ht=%d\n",
		(val & SC_ODMA_REG_DMA_HT_MASK) >> SC_ODMA_REG_DMA_HT_OFFSET);

	uartlog("SC_ODMA_ODMA_REG_13=0x%08x\n", _reg_read(odma_base + SC_ODMA_ODMA_REG_13));
	uartlog("SC_ODMA_ODMA_REG_14=0x%08x\n", _reg_read(odma_base + SC_ODMA_ODMA_REG_14));
	uartlog("SC_ODMA_ODMA_REG_15=0x%08x\n", _reg_read(odma_base + SC_ODMA_ODMA_REG_15));
	uartlog("SC_ODMA_ODMA_REG_16=0x%08x\n", _reg_read(odma_base + SC_ODMA_ODMA_REG_16));

	val = _reg_read(odma_base + SC_ODMA_SB_REG_CTRL);
	uartlog("SC_ODMA_SB_REG_CTRL=0x%08x\n", val);
	uartlog("    sb_mode=%d, sb_size=%d, sb_nb=%d, sb_full_nb=%d, sb_sw_wptr=%d\n",
		(val & SC_ODMA_REG_SB_MODE_MASK) >> SC_ODMA_REG_SB_MODE_OFFSET,
		(val & SC_ODMA_REG_SB_SIZE_MASK) >> SC_ODMA_REG_SB_SIZE_OFFSET,
		(val & SC_ODMA_REG_SB_NB_MASK) >> SC_ODMA_REG_SB_NB_OFFSET,
		(val & SC_ODMA_REG_SB_FULL_NB_MASK) >> SC_ODMA_REG_SB_FULL_NB_OFFSET,
		(val & SC_ODMA_REG_SB_SW_WPTR_MASK) >> SC_ODMA_REG_SB_SW_WPTR_OFFSET);

	val = _reg_read(odma_base + SC_ODMA_SB_REG_C_STAT);
	uartlog("SC_ODMA_SB_REG_C_STAT=0x%08x\n", val);
	uartlog("    u_sb_wptr_ro=%d, u_sb_full=%d, u_sb_empty=%d, u_sb_dptr_ro=%d\n",
		(val & SC_ODMA_REG_U_SB_WPTR_RO_MASK) >> SC_ODMA_REG_U_SB_WPTR_RO_OFFSET,
		(val & SC_ODMA_REG_U_SB_FULL_MASK) >> SC_ODMA_REG_U_SB_FULL_OFFSET,
		(val & SC_ODMA_REG_U_SB_EMPTY_MASK) >> SC_ODMA_REG_U_SB_EMPTY_OFFSET,
		(val & SC_ODMA_REG_U_SB_DPTR_RO_MASK) >> SC_ODMA_REG_U_SB_DPTR_RO_OFFSET);
	uartlog("    v_sb_wptr_ro=%d, v_sb_full=%d, v_sb_empty=%d, v_sb_dptr_ro=%d\n",
		(val & SC_ODMA_REG_V_SB_WPTR_RO_MASK) >> SC_ODMA_REG_V_SB_WPTR_RO_OFFSET,
		(val & SC_ODMA_REG_V_SB_FULL_MASK) >> SC_ODMA_REG_V_SB_FULL_OFFSET,
		(val & SC_ODMA_REG_V_SB_EMPTY_MASK) >> SC_ODMA_REG_V_SB_EMPTY_OFFSET,
		(val & SC_ODMA_REG_V_SB_DPTR_RO_MASK) >> SC_ODMA_REG_V_SB_DPTR_RO_OFFSET);

	uartlog("SC_ODMA_SB_REG_Y_STAT=0x%08x\n", _reg_read(odma_base + SC_ODMA_SB_REG_Y_STAT));
	uartlog("    y_sb_wptr_ro=%d, y_sb_full=%d, y_sb_empty=%d, y_sb_dptr_ro=%d, sb_full=%d\n",
		(val & SC_ODMA_REG_Y_SB_WPTR_RO_MASK) >> SC_ODMA_REG_Y_SB_WPTR_RO_OFFSET,
		(val & SC_ODMA_REG_Y_SB_FULL_MASK) >> SC_ODMA_REG_Y_SB_FULL_OFFSET,
		(val & SC_ODMA_REG_Y_SB_EMPTY_MASK) >> SC_ODMA_REG_Y_SB_EMPTY_OFFSET,
		(val & SC_ODMA_REG_Y_SB_DPTR_RO_MASK) >> SC_ODMA_REG_Y_SB_DPTR_RO_OFFSET,
		(val & SC_ODMA_REG_SB_FULL_MASK) >> SC_ODMA_REG_SB_FULL_OFFSET);

	val = _reg_read(odma_base + SC_ODMA_SC_CSC_REG_00);
	uartlog("SC_ODMA_SC_CSC_REG_00=0x%08x\n", val);
	uartlog("    csc_en=%d, csc_q_mode=%d, csc_q_drop=%d, hsv_en=%d, hsv_floor_en=%d\n",
		(val & SC_ODMA_REG_SC_CSC_EN_MASK) >> SC_ODMA_REG_SC_CSC_EN_OFFSET,
		(val & SC_ODMA_REG_SC_CSC_Q_MODE_MASK) >> SC_ODMA_REG_SC_CSC_Q_MODE_OFFSET,
		(val & SC_ODMA_REG_SC_CSC_Q_DROP_MASK) >> SC_ODMA_REG_SC_CSC_Q_DROP_OFFSET,
		(val & SC_ODMA_REG_SC_HSV_EN_MASK) >> SC_ODMA_REG_SC_HSV_EN_OFFSET,
		(val & SC_ODMA_REG_SC_HSV_FLOOR_EN_MASK) >> SC_ODMA_REG_SC_HSV_FLOOR_EN_OFFSET);
	uartlog("    csc_bd_dis=%d, hsv_bd_dis=%d, bf16_bd_type=%d, csc_q_gain_mode=%d\n",
		(val & SC_ODMA_REG_SC_CSC_BD_DIS_MASK) >> SC_ODMA_REG_SC_CSC_BD_DIS_OFFSET,
		(val & SC_ODMA_REG_SC_HSV_BD_DIS_MASK) >> SC_ODMA_REG_SC_HSV_BD_DIS_OFFSET,
		(val & SC_ODMA_REG_BF16_BD_TYPE_MASK) >> SC_ODMA_REG_BF16_BD_TYPE_OFFSET,
		(val & SC_ODMA_REG_SC_CSC_Q_GAIN_MODE_MASK) >> SC_ODMA_REG_SC_CSC_Q_GAIN_MODE_OFFSET);

	uartlog("SC_ODMA_SC_CSC_REG_01=0x%08x\n", _reg_read(odma_base + SC_ODMA_SC_CSC_REG_01));
	uartlog("SC_ODMA_SC_CSC_REG_02=0x%08x\n", _reg_read(odma_base + SC_ODMA_SC_CSC_REG_02));
	uartlog("SC_ODMA_SC_CSC_REG_03=0x%08x\n", _reg_read(odma_base + SC_ODMA_SC_CSC_REG_03));
	uartlog("SC_ODMA_SC_CSC_REG_04=0x%08x\n", _reg_read(odma_base + SC_ODMA_SC_CSC_REG_04));
	uartlog("SC_ODMA_SC_CSC_REG_05=0x%08x\n", _reg_read(odma_base + SC_ODMA_SC_CSC_REG_05));
	uartlog("SC_ODMA_SC_CSC_REG_06=0x%08x\n", _reg_read(odma_base + SC_ODMA_SC_CSC_REG_06));
	uartlog("SC_ODMA_SC_CSC_REG_07=0x%08x\n", _reg_read(odma_base + SC_ODMA_SC_CSC_REG_07));
	uartlog("SC_ODMA_SC_CSC_REG_08=0x%08x\n", _reg_read(odma_base + SC_ODMA_SC_CSC_REG_08));
}
