#ifdef ENV_CVITEST
#include <common.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "system_common.h"
#include "timer.h"
#elif defined(ENV_HOSTPC)
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "emu/command.h"
#else
#include <linux/types.h>
#include <linux/delay.h>
#endif

#include "vip_common.h"
#include "reg.h"
#include "inc/isp_reg.h"
#include "isp_drv.h"

#define REG_ARRAY_UPDATE2_SIZE(addr, array, size)		\
	do {							\
		uint16_t i;					\
		for (i = 0; i < size; i += 2) {			\
			val = array[i];				\
			if ((i + 1) < size)			\
				val |= (array[i+1] << 16);	\
			_reg_write(addr + (i << 1), val);	\
		}						\
	} while (0)

#define REG_ARRAY_UPDATE2(addr, array)				\
	REG_ARRAY_UPDATE2_SIZE(addr, array, ARRAY_SIZE(array))

#define REG_ARRAY_UPDATE4(addr, array)				\
	do {							\
		uint16_t i;					\
		for (i = 0; i < ARRAY_SIZE(array); i += 4) {	\
			val = array[i];				\
			if ((i + 1) < ARRAY_SIZE(array))	\
				val |= (array[i+1] << 8);	\
			if ((i + 2) < ARRAY_SIZE(array))	\
				val |= (array[i+2] << 16);	\
			if ((i + 3) < ARRAY_SIZE(array))	\
				val |= (array[i+3] << 24);	\
			_reg_write(addr + i, val);		\
		}						\
	} while (0)

static void _manr_init(struct isp_ctx *ctx);
static void _ltm_init(struct isp_ctx *ctx);
static void _cfa_init(struct isp_ctx *ctx);
static void _rgbee_init(struct isp_ctx *ctx);
static void _dhz_init(struct isp_ctx *ctx);
static void _bnr_init(struct isp_ctx *ctx);
static void _ynr_init(struct isp_ctx *ctx);
static void _cnr_init(struct isp_ctx *ctx);
static void _ee_init(struct isp_ctx *ctx);

/****************************************************************************
 * Global parameters
 ****************************************************************************/
static uintptr_t reg_base;

/****************************************************************************
 * Interfaces
 ****************************************************************************/
void isp_set_base_addr(void *base)
{
	uint64_t *addr = isp_get_phys_reg_bases();
	int i = 0;

	for (i = 0; i < ISP_BLK_ID_MAX; ++i) {
		addr[i] -= reg_base;
		addr[i] += (uintptr_t)base;
	}
	reg_base = (uintptr_t)base;
}

void isp_init(struct isp_ctx *ctx)
{
	_manr_init(ctx);
	_ltm_init(ctx);
	_cfa_init(ctx);
	_rgbee_init(ctx);
	_dhz_init(ctx);
	_bnr_init(ctx);
	_ynr_init(ctx);
	_cnr_init(ctx);
	_ee_init(ctx);
}

void isp_uninit(struct isp_ctx *ctx)
{
}

void isp_reset(struct isp_ctx *ctx)
{
	uint64_t isptopb = ctx->phys_regs[ISP_BLK_ID_ISPTOP];
	union isp_intr intr_mask;

	// disable interrupt
	intr_mask.raw = 0;
	isp_intr_set_mask(ctx, intr_mask);

	// switch back to hw trig.
	ISP_WR_BITS(isptopb, REG_ISP_TOP_T, REG_3, TRIG_STR_SEL_PRE, 1);
	ISP_WR_BITS(isptopb, REG_ISP_TOP_T, REG_3, TRIG_STR_SEL_PRE1, 1);
	ISP_WR_BITS(isptopb, REG_ISP_TOP_T, REG_3, TRIG_STR_SEL_POST, 1);

	// reset
	ISP_WR_BITS(isptopb, REG_ISP_TOP_T, REG_6, CSI_RST, 1);
	ISP_WR_BITS(isptopb, REG_ISP_TOP_T, REG_6, CSI2_RST, 1);
	ISP_WR_BITS(isptopb, REG_ISP_TOP_T, REG_6, AXI_RST, 1);
	ISP_WR_BITS(isptopb, REG_ISP_TOP_T, REG_6, IP_RST, 1);

	ISP_WR_REG(isptopb, REG_ISP_TOP_T, REG_6, 0);

	// clear intr
	intr_mask.raw = 0xffffffff;
	isp_intr_clr(ctx, intr_mask);
}

void isp_config(struct isp_ctx *ctx, struct isp_param *param)
{
}

void isp_pchk_config(struct isp_ctx *ctx, uint8_t en_mask)
{
	uint64_t addr[] = {
		reg_base + ISP_BLK_BA_PRE0_PCHK,
		reg_base + ISP_BLK_BA_PRE1_PCHK,
		reg_base + ISP_BLK_BA_RAW_PCHK0,
		reg_base + ISP_BLK_BA_RAW_PCHK1,
		reg_base + ISP_BLK_BA_RGB_PCHK0,
		reg_base + ISP_BLK_BA_RGB_PCHK1,
		reg_base + ISP_BLK_BA_YUV_PCHK0,
		reg_base + ISP_BLK_BA_YUV_PCHK1,
	};
	uint8_t in_sel[] = {
		0, 0, 0, 0, 4, 0, 6, 3,
	};
	uint8_t i;

	for (i = 0; i < ARRAY_SIZE(addr); ++i) {
		if (!(en_mask & BIT(i)))
			continue;

		_reg_write(addr[i]+0x04, in_sel[i]);
		_reg_write(addr[i]+0x08, 0x1ff);
		_reg_write(addr[i]+0x10, ctx->img_width);
		_reg_write(addr[i]+0x14, ctx->img_height);
		_reg_write(addr[i]+0x18, ctx->img_width * ctx->img_height +
			   0x2000);
		_reg_write(addr[i]+0x1c, ctx->img_width);
		_reg_write(addr[i]+0x20, 120);
	}
}

void isp_pchk_chk_status(struct isp_ctx *ctx, uint8_t en_mask,
			 uint32_t intr_status)
{
	uint64_t addr[] = {
		reg_base + ISP_BLK_BA_PRE0_PCHK,
		reg_base + ISP_BLK_BA_PRE1_PCHK,
		reg_base + ISP_BLK_BA_RAW_PCHK0,
		reg_base + ISP_BLK_BA_RAW_PCHK1,
		reg_base + ISP_BLK_BA_RGB_PCHK0,
		reg_base + ISP_BLK_BA_RGB_PCHK1,
		reg_base + ISP_BLK_BA_YUV_PCHK0,
		reg_base + ISP_BLK_BA_YUV_PCHK1,
	};
	uint8_t bit[] = {
		16, 17, 18, 18+5,
		19, 19+5, 20, 20+5,
	};
	static const char * const name[] = {
		"preraw0 pchk", "preraw1 pchk",
		"rawtop pchk0", "rawtop pchk1",
		"rgbtop pchk0", "rgbtop pchk1",
		"yuvtop pchk0", "yuvtop pchk1",
	};
	uint8_t i;

	for (i = 0; i < ARRAY_SIZE(addr); ++i) {
		if (!(en_mask & BIT(i)))
			continue;

		if (intr_status & BIT(bit[i])) {
			uartlog("[bm-vip][isp]%s err: %#x cnt:%#x\n", name[i],
				_reg_read(addr[i]+0x100),
				_reg_read(addr[i]+0x104));
			_reg_write(addr[i]+0x200, 0x1ff);
		}
	}
}

void isp_pre_trig(struct isp_ctx *ctx)
{
	uint64_t preraw0 = ctx->phys_regs[ISP_BLK_ID_PRERAW0];

	if (ctx->is_offline_preraw) {
		uint64_t isptopb = ctx->phys_regs[ISP_BLK_ID_ISPTOP];

		if (!ctx->is_offline_postraw) {
			ISP_WR_REG(isptopb, REG_ISP_TOP_T, REG_2, 0x28);
			ISP_WR_REG(isptopb, REG_ISP_TOP_T, REG_2, 0x5);
		} else {
			ISP_WO_BITS(isptopb, REG_ISP_TOP_T, REG_2, SHAW_UP_PRE, 1);
			ISP_WO_BITS(isptopb, REG_ISP_TOP_T, REG_2, TRIG_STR_PRE, 1);
		}
	} else {
		ISP_WO_BITS(preraw0, REG_PRE_RAW_T, FRAME_VLD, PRE_RAW_FRAME_VLD, 1);
	}
}

void _rgbmap_swap(struct isp_ctx *ctx)
{
	uint64_t dma19 = ctx->phys_regs[ISP_BLK_ID_DMA19];
	uint64_t dma21 = ctx->phys_regs[ISP_BLK_ID_DMA21];
	uint64_t addr0, addr1;

	if (!ctx->is_3dnr_on || !ctx->is_offline_postraw)
		return;

	addr0 = ISP_RD_REG(dma19, REG_ISP_DMA_T, DMA_1);
	addr0 = (addr0 << 32) |
		ISP_RD_REG(dma19, REG_ISP_DMA_T, DMA_0);
	addr1 = ISP_RD_REG(dma21, REG_ISP_DMA_T, DMA_1);
	addr1 = (addr1 << 32) |
		ISP_RD_REG(dma21, REG_ISP_DMA_T, DMA_0);
	ispblk_dma_setaddr(ctx, ISP_BLK_ID_DMA15, addr0);
	ispblk_dma_setaddr(ctx, ISP_BLK_ID_DMA19, addr1);
	ispblk_dma_setaddr(ctx, ISP_BLK_ID_DMA21, addr0);
	if (ctx->is_hdr_on) {
		uint64_t dma20 = ctx->phys_regs[ISP_BLK_ID_DMA20];
		uint64_t dma22 = ctx->phys_regs[ISP_BLK_ID_DMA22];

		addr0 = ISP_RD_REG(dma20, REG_ISP_DMA_T, DMA_1);
		addr0 = (addr0 << 32) |
			ISP_RD_REG(dma20, REG_ISP_DMA_T, DMA_0);
		addr1 = ISP_RD_REG(dma22, REG_ISP_DMA_T, DMA_1);
		addr1 = (addr1 << 32) |
			ISP_RD_REG(dma22, REG_ISP_DMA_T, DMA_0);
		ispblk_dma_setaddr(ctx, ISP_BLK_ID_DMA16, addr0);
		ispblk_dma_setaddr(ctx, ISP_BLK_ID_DMA20, addr1);
		ispblk_dma_setaddr(ctx, ISP_BLK_ID_DMA22, addr0);
	}
}

void isp_post_trig(struct isp_ctx *ctx)
{
	uint64_t isptopb = ctx->phys_regs[ISP_BLK_ID_ISPTOP];

	if (!ctx->is_offline_postraw)
		return;

	_rgbmap_swap(ctx);
	ISP_WO_BITS(isptopb, REG_ISP_TOP_T, REG_2, SHAW_UP_POST, 1);
	ISP_WO_BITS(isptopb, REG_ISP_TOP_T, REG_2, TRIG_STR_POST, 1);
}

void isp_streaming(struct isp_ctx *ctx, uint32_t on)
{
	uint64_t csibdg = (ctx->cam_id == 0)
			  ? ctx->phys_regs[ISP_BLK_ID_CSIBDG0]
			  : ctx->phys_regs[ISP_BLK_ID_CSIBDG1_R1];
	uint64_t isptopb = ctx->phys_regs[ISP_BLK_ID_ISPTOP];

	if (on) {
		ISP_WO_BITS(csibdg, REG_ISP_CSI_BDG_T, CSI_BDG_UP, CSI_UP_REG, 1);

		if (ctx->csibdg_patgen_en) {
			ISP_WR_BITS(csibdg, REG_ISP_CSI_BDG_T, CSI_TGEN_ENABLE,
				    INTERNAL_TIME_GEN_ENABLE, 1);
		}

		ISP_WO_BITS(isptopb, REG_ISP_TOP_T, REG_2, SHAW_UP_PRE, 1);
		ISP_WO_BITS(isptopb, REG_ISP_TOP_T, REG_2, SHAW_UP_POST, 1);

		ISP_WR_BITS(csibdg, REG_ISP_CSI_BDG_T, CSI_CTRL, CSI_ENABLE, 1);
	} else {
		ISP_WR_BITS(csibdg, REG_ISP_CSI_BDG_T, CSI_CTRL, CSI_ENABLE, 0);

		if (ctx->csibdg_patgen_en) {
			ISP_WR_BITS(csibdg, REG_ISP_CSI_BDG_T, CSI_TGEN_ENABLE,
				    INTERNAL_TIME_GEN_ENABLE, 0);
		}
	}
}


uint64_t *isp_get_phys_reg_bases(void)
{
	static uint64_t m_isp_phys_base_list[ISP_BLK_ID_MAX] = {
		[ISP_BLK_ID_PRERAW0] = (ISP_BLK_BA_PRERAW0),
		[ISP_BLK_ID_CSIBDG0] = (ISP_BLK_BA_CSIBDG0),
		[ISP_BLK_ID_CROP0] = (ISP_BLK_BA_CROP0),
		[ISP_BLK_ID_CROP1] = (ISP_BLK_BA_CROP1),
		[ISP_BLK_ID_BLC0] = (ISP_BLK_BA_BLC0),
		[ISP_BLK_ID_BLC1] = (ISP_BLK_BA_BLC1),
		[ISP_BLK_ID_LSCR0] = (ISP_BLK_BA_LSCR0),
		[ISP_BLK_ID_LSCR1] = (ISP_BLK_BA_LSCR1),
		[ISP_BLK_ID_AEHIST0] = (ISP_BLK_BA_AEHIST0),
		[ISP_BLK_ID_AEHIST1] = (ISP_BLK_BA_AEHIST1),
		[ISP_BLK_ID_AWB0] = (ISP_BLK_BA_AWB0),
		[ISP_BLK_ID_AWB1] = (ISP_BLK_BA_AWB1),
		[ISP_BLK_ID_AF] = (ISP_BLK_BA_AF),
		[ISP_BLK_ID_GMS] = (ISP_BLK_BA_GMS),
		[ISP_BLK_ID_WBG0] = (ISP_BLK_BA_WBG0),
		[ISP_BLK_ID_WBG1] = (ISP_BLK_BA_WBG1),
		[ISP_BLK_ID_LMP0] = (ISP_BLK_BA_LMP0),
		[ISP_BLK_ID_RGBMAP0] = (ISP_BLK_BA_RGBMAP0),
		[ISP_BLK_ID_PRERAW1_R1] = (ISP_BLK_BA_PRERAW1_R1),
		[ISP_BLK_ID_CSIBDG1_R1] = (ISP_BLK_BA_CSIBDG1_R1),
		[ISP_BLK_ID_CROP0_R1] = (ISP_BLK_BA_CROP0_R1),
		[ISP_BLK_ID_CROP1_R1] = (ISP_BLK_BA_CROP1_R1),
		[ISP_BLK_ID_BLC0_R1] = (ISP_BLK_BA_BLC0_R1),
		[ISP_BLK_ID_BLC1_R1] = (ISP_BLK_BA_BLC1_R1),
		[ISP_BLK_ID_LSCR2_R1] = (ISP_BLK_BA_LSCR2_R1),
		[ISP_BLK_ID_LSCR3_R1] = (ISP_BLK_BA_LSCR3_R1),
		[ISP_BLK_ID_AEHIST0_R1] = (ISP_BLK_BA_AEHIST0_R1),
		[ISP_BLK_ID_AEHIST1_R1] = (ISP_BLK_BA_AEHIST1_R1),
		[ISP_BLK_ID_AWB0_R1] = (ISP_BLK_BA_AWB0_R1),
		[ISP_BLK_ID_AWB1_R1] = (ISP_BLK_BA_AWB1_R1),
		[ISP_BLK_ID_AF_R1] = (ISP_BLK_BA_AF_R1),
		[ISP_BLK_ID_GMS_R1] = (ISP_BLK_BA_GMS_R1),
		[ISP_BLK_ID_WBG0_R1] = (ISP_BLK_BA_WBG0_R1),
		[ISP_BLK_ID_WBG1_R1] = (ISP_BLK_BA_WBG1_R1),
		[ISP_BLK_ID_LMP2_R1] = (ISP_BLK_BA_LMP2_R1),
		[ISP_BLK_ID_RGBMAP2_R1] = (ISP_BLK_BA_RGBMAP2_R1),
		[ISP_BLK_ID_RAWTOP] = (ISP_BLK_BA_RAWTOP),
		[ISP_BLK_ID_BLC2] = (ISP_BLK_BA_BLC2),
		[ISP_BLK_ID_BLC3] = (ISP_BLK_BA_BLC3),
		[ISP_BLK_ID_DPC0] = (ISP_BLK_BA_DPC0),
		[ISP_BLK_ID_DPC1] = (ISP_BLK_BA_DPC1),
		[ISP_BLK_ID_WBG2] = (ISP_BLK_BA_WBG2),
		[ISP_BLK_ID_WBG3] = (ISP_BLK_BA_WBG3),
		[ISP_BLK_ID_LSCM0] = (ISP_BLK_BA_LSCM0),
		[ISP_BLK_ID_LSCM1] = (ISP_BLK_BA_LSCM1),
		[ISP_BLK_ID_AWB4] = (ISP_BLK_BA_AWB4),
		[ISP_BLK_ID_HDRFUSION] = (ISP_BLK_BA_HDRFUSION),
		[ISP_BLK_ID_HDRLTM] = (ISP_BLK_BA_HDRLTM),
		[ISP_BLK_ID_BNR] = (ISP_BLK_BA_BNR),
		[ISP_BLK_ID_CROP2] = (ISP_BLK_BA_CROP2),
		[ISP_BLK_ID_CROP3] = (ISP_BLK_BA_CROP3),
		[ISP_BLK_ID_MANR] = (ISP_BLK_BA_MANR),
		[ISP_BLK_ID_FPN0] = (ISP_BLK_BA_FPN0),
		[ISP_BLK_ID_FPN1] = (ISP_BLK_BA_FPN1),
		[ISP_BLK_ID_WBG4] = (ISP_BLK_BA_WBG4),
		[ISP_BLK_ID_RGBTOP] = (ISP_BLK_BA_RGBTOP),
		[ISP_BLK_ID_CFA] = (ISP_BLK_BA_CFA),
		[ISP_BLK_ID_CCM] = (ISP_BLK_BA_CCM),
		[ISP_BLK_ID_GAMMA] = (ISP_BLK_BA_GAMMA),
		[ISP_BLK_ID_HSV] = (ISP_BLK_BA_HSV),
		[ISP_BLK_ID_DHZ] = (ISP_BLK_BA_DHZ),
		[ISP_BLK_ID_R2Y4] = (ISP_BLK_BA_R2Y4),
		[ISP_BLK_ID_RGBDITHER] = (ISP_BLK_BA_RGBDITHER),
		[ISP_BLK_ID_RGBEE] = (ISP_BLK_BA_RGBEE),
		[ISP_BLK_ID_YUVTOP] = (ISP_BLK_BA_YUVTOP),
		[ISP_BLK_ID_444422] = (ISP_BLK_BA_444422),
		[ISP_BLK_ID_UVDITHER] = (ISP_BLK_BA_UVDITHER),
		//[ISP_BLK_ID_3DNR] = (ISP_BLK_BA_3DNR),
		[ISP_BLK_ID_YNR] = (ISP_BLK_BA_YNR),
		[ISP_BLK_ID_CNR] = (ISP_BLK_BA_CNR),
		[ISP_BLK_ID_EE] = (ISP_BLK_BA_EE),
		[ISP_BLK_ID_YCURVE] = (ISP_BLK_BA_YCURVE),
		[ISP_BLK_ID_CROP4] = (ISP_BLK_BA_CROP4),
		[ISP_BLK_ID_CROP5] = (ISP_BLK_BA_CROP5),
		[ISP_BLK_ID_CROP6] = (ISP_BLK_BA_CROP6),
		[ISP_BLK_ID_DCI] = (ISP_BLK_BA_DCI),
		[ISP_BLK_ID_ISPTOP] = (ISP_BLK_BA_ISPTOP),
		[ISP_BLK_ID_LMP1] = (ISP_BLK_BA_LMP1),
		[ISP_BLK_ID_RGBMAP1] = (ISP_BLK_BA_RGBMAP1),
		[ISP_BLK_ID_LMP3_R1] = (ISP_BLK_BA_LMP3_R1),
		[ISP_BLK_ID_RGBMAP3_R1] = (ISP_BLK_BA_RGBMAP3_R1),
		[ISP_BLK_ID_DMA0] = (ISP_BLK_BA_DMA0),
		[ISP_BLK_ID_DMA1] = (ISP_BLK_BA_DMA1),
		[ISP_BLK_ID_DMA2] = (ISP_BLK_BA_DMA2),
		[ISP_BLK_ID_DMA3] = (ISP_BLK_BA_DMA3),
		[ISP_BLK_ID_DMA4] = (ISP_BLK_BA_DMA4),
		[ISP_BLK_ID_DMA5] = (ISP_BLK_BA_DMA5),
		[ISP_BLK_ID_DMA6] = (ISP_BLK_BA_DMA6),
		[ISP_BLK_ID_DMA7] = (ISP_BLK_BA_DMA7),
		[ISP_BLK_ID_DMA8] = (ISP_BLK_BA_DMA8),
		[ISP_BLK_ID_DMA9] = (ISP_BLK_BA_DMA9),
		[ISP_BLK_ID_DMA10] = (ISP_BLK_BA_DMA10),
		[ISP_BLK_ID_DMA11] = (ISP_BLK_BA_DMA11),
		[ISP_BLK_ID_DMA12] = (ISP_BLK_BA_DMA12),
		[ISP_BLK_ID_DMA13] = (ISP_BLK_BA_DMA13),
		[ISP_BLK_ID_DMA14] = (ISP_BLK_BA_DMA14),
		[ISP_BLK_ID_DMA15] = (ISP_BLK_BA_DMA15),
		[ISP_BLK_ID_DMA16] = (ISP_BLK_BA_DMA16),
		[ISP_BLK_ID_DMA17] = (ISP_BLK_BA_DMA17),
		[ISP_BLK_ID_DMA18] = (ISP_BLK_BA_DMA18),
		[ISP_BLK_ID_DMA19] = (ISP_BLK_BA_DMA19),
		[ISP_BLK_ID_DMA20] = (ISP_BLK_BA_DMA20),
		[ISP_BLK_ID_DMA21] = (ISP_BLK_BA_DMA21),
		[ISP_BLK_ID_DMA22] = (ISP_BLK_BA_DMA22),
		[ISP_BLK_ID_DMA23] = (ISP_BLK_BA_DMA23),
		[ISP_BLK_ID_DMA24] = (ISP_BLK_BA_DMA24),
		[ISP_BLK_ID_DMA25] = (ISP_BLK_BA_DMA25),
		[ISP_BLK_ID_DMA26] = (ISP_BLK_BA_DMA26),
		[ISP_BLK_ID_DMA27] = (ISP_BLK_BA_DMA27),
		[ISP_BLK_ID_DMA28] = (ISP_BLK_BA_DMA28),
		[ISP_BLK_ID_DMA29] = (ISP_BLK_BA_DMA29),
		[ISP_BLK_ID_DMA30] = (ISP_BLK_BA_DMA30),
		[ISP_BLK_ID_DMA31] = (ISP_BLK_BA_DMA31),
		[ISP_BLK_ID_DMA32] = (ISP_BLK_BA_DMA32),
		[ISP_BLK_ID_DMA33] = (ISP_BLK_BA_DMA33),
		[ISP_BLK_ID_DMA34] = (ISP_BLK_BA_DMA34),
		[ISP_BLK_ID_DMA35] = (ISP_BLK_BA_DMA35),
		[ISP_BLK_ID_DMA36] = (ISP_BLK_BA_DMA36),
		[ISP_BLK_ID_DMA37] = (ISP_BLK_BA_DMA37),
		[ISP_BLK_ID_DMA38] = (ISP_BLK_BA_DMA38),
		[ISP_BLK_ID_DMA39] = (ISP_BLK_BA_DMA39),
		[ISP_BLK_ID_DMA40] = (ISP_BLK_BA_DMA40),
		[ISP_BLK_ID_DMA41] = (ISP_BLK_BA_DMA41),
		[ISP_BLK_ID_DMA42] = (ISP_BLK_BA_DMA42),
		[ISP_BLK_ID_DMA43] = (ISP_BLK_BA_DMA43),
		[ISP_BLK_ID_DMA44] = (ISP_BLK_BA_DMA44),
		[ISP_BLK_ID_DMA45] = (ISP_BLK_BA_DMA45),
		[ISP_BLK_ID_DMA46] = (ISP_BLK_BA_DMA46),
		[ISP_BLK_ID_DMA47] = (ISP_BLK_BA_DMA47),
		[ISP_BLK_ID_DMA48] = (ISP_BLK_BA_DMA48),
		[ISP_BLK_ID_DMA49] = (ISP_BLK_BA_DMA49),
		[ISP_BLK_ID_DMA50] = (ISP_BLK_BA_DMA50),
		[ISP_BLK_ID_DMA51] = (ISP_BLK_BA_DMA51),
		[ISP_BLK_ID_DMA52] = (ISP_BLK_BA_DMA52),
		[ISP_BLK_ID_DMA53] = (ISP_BLK_BA_DMA53),
		[ISP_BLK_ID_DMA54] = (ISP_BLK_BA_DMA54),
		[ISP_BLK_ID_DMA55] = (ISP_BLK_BA_DMA55),
		[ISP_BLK_ID_DMA56] = (ISP_BLK_BA_DMA56),
		[ISP_BLK_ID_DMA57] = (ISP_BLK_BA_DMA57),
		[ISP_BLK_ID_DMA58] = (ISP_BLK_BA_DMA58),
		[ISP_BLK_ID_DMA59] = (ISP_BLK_BA_DMA59),
		[ISP_BLK_ID_DMA60] = (ISP_BLK_BA_DMA60),
		[ISP_BLK_ID_DMA61] = (ISP_BLK_BA_DMA61),
		[ISP_BLK_ID_CMDQ1] = (ISP_BLK_BA_CMDQ1),
		[ISP_BLK_ID_CMDQ2] = (ISP_BLK_BA_CMDQ2),
		[ISP_BLK_ID_CMDQ3] = (ISP_BLK_BA_CMDQ3),
		[ISP_BLK_ID_AF_GAMMA] = (ISP_BLK_BA_AF_GAMMA),
		[ISP_BLK_ID_AF_GAMMA_R1] = (ISP_BLK_BA_AF_GAMMA_R1),
	};
	return m_isp_phys_base_list;
}

int isp_get_vblock_info(struct isp_vblock_info **pinfo, uint32_t *nblocks,
			enum ISPCQ_ID_T cq_group)
{
#define VBLOCK_INFO(_name, _struct) \
	{ISP_BLK_ID_##_name, sizeof(struct _struct), ISP_BLK_BA_##_name}

	static struct isp_vblock_info m_block_preraw[] = {
		VBLOCK_INFO(PRERAW0, VREG_PRE_RAW_T),
		VBLOCK_INFO(CSIBDG0, VREG_ISP_CSI_BDG_T),
		VBLOCK_INFO(CROP0, VREG_ISP_CROP_T),
		VBLOCK_INFO(CROP1, VREG_ISP_CROP_T),
		VBLOCK_INFO(BLC0, VREG_ISP_BLC_T),
		VBLOCK_INFO(BLC1, VREG_ISP_BLC_T),
		VBLOCK_INFO(LSCR0, VREG_ISP_LSCR_T),
		VBLOCK_INFO(LSCR1, VREG_ISP_LSCR_T),
		VBLOCK_INFO(AEHIST0, VREG_ISP_AE_HIST_T),
		VBLOCK_INFO(AEHIST1, VREG_ISP_AE_HIST_T),
		VBLOCK_INFO(AWB0, VREG_ISP_AWB_T),
		VBLOCK_INFO(AWB1, VREG_ISP_AWB_T),
		VBLOCK_INFO(AF, VREG_ISP_AF_T),
		VBLOCK_INFO(GMS, VREG_ISP_GMS_T),
		VBLOCK_INFO(WBG0, VREG_ISP_WBG_T),
		VBLOCK_INFO(WBG1, VREG_ISP_WBG_T),
		VBLOCK_INFO(LMP0, VREG_ISP_LMAP_T),
		VBLOCK_INFO(LMP1, VREG_ISP_LMAP_T),
		VBLOCK_INFO(RGBMAP0, VREG_ISP_RGBMAP_T),
		VBLOCK_INFO(RGBMAP1, VREG_ISP_RGBMAP_T),
	};
	static struct isp_vblock_info m_block_preraw1[] = {
		VBLOCK_INFO(PRERAW1_R1, VREG_PRE_RAW_T),
		VBLOCK_INFO(CSIBDG1_R1, VREG_ISP_CSI_BDG_T),
		VBLOCK_INFO(CROP0_R1, VREG_ISP_CROP_T),
		VBLOCK_INFO(CROP1_R1, VREG_ISP_CROP_T),
		VBLOCK_INFO(BLC0_R1, VREG_ISP_BLC_T),
		VBLOCK_INFO(BLC1_R1, VREG_ISP_BLC_T),
		VBLOCK_INFO(LSCR2_R1, VREG_ISP_LSCR_T),
		VBLOCK_INFO(LSCR3_R1, VREG_ISP_LSCR_T),
		VBLOCK_INFO(AEHIST0_R1, VREG_ISP_AE_HIST_T),
		VBLOCK_INFO(AEHIST1_R1, VREG_ISP_AE_HIST_T),
		VBLOCK_INFO(AWB0_R1, VREG_ISP_AWB_T),
		VBLOCK_INFO(AWB1_R1, VREG_ISP_AWB_T),
		VBLOCK_INFO(AF_R1, VREG_ISP_AF_T),
		VBLOCK_INFO(GMS_R1, VREG_ISP_GMS_T),
		VBLOCK_INFO(WBG0_R1, VREG_ISP_WBG_T),
		VBLOCK_INFO(WBG1_R1, VREG_ISP_WBG_T),
		VBLOCK_INFO(LMP2_R1, VREG_ISP_LMAP_T),
		VBLOCK_INFO(LMP3_R1, VREG_ISP_LMAP_T),
		VBLOCK_INFO(RGBMAP2_R1, VREG_ISP_RGBMAP_T),
		VBLOCK_INFO(RGBMAP3_R1, VREG_ISP_RGBMAP_T),
	};

	// PRE_RAW_1 ... to be added

	static struct isp_vblock_info m_block_postraw[] = {
		///< {ISP_BLK_ID_RAWTOP      , sizeof(32), ISP_BLK_BA_RAWTOP},
		VBLOCK_INFO(BLC2, VREG_ISP_BLC_T),
		VBLOCK_INFO(BLC3, VREG_ISP_BLC_T),
		VBLOCK_INFO(DPC0, VREG_ISP_DPC_T),
		VBLOCK_INFO(DPC1, VREG_ISP_DPC_T),
		VBLOCK_INFO(WBG2, VREG_ISP_WBG_T),
		VBLOCK_INFO(WBG3, VREG_ISP_WBG_T),
		VBLOCK_INFO(LSCM0, VREG_ISP_LSC_T),
		VBLOCK_INFO(LSCM1, VREG_ISP_LSC_T),
		VBLOCK_INFO(AWB4, VREG_ISP_AWB_T),
		VBLOCK_INFO(HDRFUSION, VREG_ISP_FUSION_T),
		VBLOCK_INFO(HDRLTM, VREG_ISP_LTM_T),
		VBLOCK_INFO(BNR, VREG_ISP_BNR_T),
		VBLOCK_INFO(CROP2, VREG_ISP_CROP_T),
		VBLOCK_INFO(CROP3, VREG_ISP_CROP_T),
		VBLOCK_INFO(MANR, VREG_ISP_MM_T),
		VBLOCK_INFO(FPN0, VREG_ISP_FPN_T),
		VBLOCK_INFO(FPN1, VREG_ISP_FPN_T),
		VBLOCK_INFO(RGBTOP, VREG_ISP_RGB_T),
		VBLOCK_INFO(CFA, VREG_ISP_CFA_T),
		VBLOCK_INFO(CCM, VREG_ISP_CCM_T),
		VBLOCK_INFO(GAMMA, VREG_ISP_GAMMA_T),
		VBLOCK_INFO(HSV, VREG_ISP_HSV_T),
		VBLOCK_INFO(DHZ, VREG_ISP_DHZ_T),
		///< {ISP_BLK_ID_R2Y4, sizeof(VREG_ISP_R2Y4), ISP_BLK_BA_R2Y4},
		VBLOCK_INFO(RGBDITHER, VREG_ISP_RGB_DITHER_T),
		VBLOCK_INFO(RGBEE, VREG_ISP_RGBEE_T),
		VBLOCK_INFO(YUVTOP, VREG_YUV_TOP_T),
		VBLOCK_INFO(444422, VREG_ISP_444_422_T),
		VBLOCK_INFO(UVDITHER, VREG_ISP_YUV_DITHER_T),
		///< {ISP_BLK_ID_3DNR        , sizeof(32), ISP_BLK_BA_3DNR},
		VBLOCK_INFO(YNR, VREG_ISP_YNR_T),
		VBLOCK_INFO(CNR, VREG_ISP_CNR_T),
		VBLOCK_INFO(EE, VREG_ISP_EE_T),
		VBLOCK_INFO(YCURVE, VREG_ISP_YCUR_T),
		VBLOCK_INFO(CROP4, VREG_ISP_CROP_T),
		VBLOCK_INFO(CROP5, VREG_ISP_CROP_T),
		VBLOCK_INFO(CROP6, VREG_ISP_CROP_T),
		VBLOCK_INFO(DCI, VREG_ISP_DCI_T),
		VBLOCK_INFO(ISPTOP, VREG_ISP_TOP_T),
		VBLOCK_INFO(DMA0, VREG_ISP_DMA_T),
		VBLOCK_INFO(DMA1, VREG_ISP_DMA_T),
		VBLOCK_INFO(DMA2, VREG_ISP_DMA_T),
		VBLOCK_INFO(DMA3, VREG_ISP_DMA_T),
		VBLOCK_INFO(DMA4, VREG_ISP_DMA_T),
		VBLOCK_INFO(DMA5, VREG_ISP_DMA_T),
		VBLOCK_INFO(DMA6, VREG_ISP_DMA_T),
		VBLOCK_INFO(DMA7, VREG_ISP_DMA_T),
		VBLOCK_INFO(DMA8, VREG_ISP_DMA_T),
		VBLOCK_INFO(DMA9, VREG_ISP_DMA_T),
		VBLOCK_INFO(DMA10, VREG_ISP_DMA_T),
		VBLOCK_INFO(DMA11, VREG_ISP_DMA_T),
		VBLOCK_INFO(DMA12, VREG_ISP_DMA_T),
		VBLOCK_INFO(DMA13, VREG_ISP_DMA_T),
		VBLOCK_INFO(DMA14, VREG_ISP_DMA_T),
		VBLOCK_INFO(DMA15, VREG_ISP_DMA_T),
		VBLOCK_INFO(DMA16, VREG_ISP_DMA_T),
		VBLOCK_INFO(DMA17, VREG_ISP_DMA_T),
		VBLOCK_INFO(DMA18, VREG_ISP_DMA_T),
		VBLOCK_INFO(DMA19, VREG_ISP_DMA_T),
		VBLOCK_INFO(DMA20, VREG_ISP_DMA_T),
		VBLOCK_INFO(DMA21, VREG_ISP_DMA_T),
		VBLOCK_INFO(DMA22, VREG_ISP_DMA_T),
		VBLOCK_INFO(DMA23, VREG_ISP_DMA_T),
		VBLOCK_INFO(DMA24, VREG_ISP_DMA_T),
		VBLOCK_INFO(DMA25, VREG_ISP_DMA_T),
		VBLOCK_INFO(DMA26, VREG_ISP_DMA_T),
		VBLOCK_INFO(DMA27, VREG_ISP_DMA_T),
		VBLOCK_INFO(DMA28, VREG_ISP_DMA_T),
		VBLOCK_INFO(DMA29, VREG_ISP_DMA_T),
		VBLOCK_INFO(DMA30, VREG_ISP_DMA_T),
		VBLOCK_INFO(DMA31, VREG_ISP_DMA_T),
		VBLOCK_INFO(DMA32, VREG_ISP_DMA_T),
		VBLOCK_INFO(DMA33, VREG_ISP_DMA_T),
		VBLOCK_INFO(DMA34, VREG_ISP_DMA_T),
		VBLOCK_INFO(DMA35, VREG_ISP_DMA_T),
		VBLOCK_INFO(DMA36, VREG_ISP_DMA_T),
		VBLOCK_INFO(DMA37, VREG_ISP_DMA_T),
		VBLOCK_INFO(DMA38, VREG_ISP_DMA_T),
		VBLOCK_INFO(DMA39, VREG_ISP_DMA_T),
		VBLOCK_INFO(DMA40, VREG_ISP_DMA_T),
		VBLOCK_INFO(DMA41, VREG_ISP_DMA_T),
		VBLOCK_INFO(DMA42, VREG_ISP_DMA_T),
		VBLOCK_INFO(DMA43, VREG_ISP_DMA_T),
		VBLOCK_INFO(DMA44, VREG_ISP_DMA_T),
		VBLOCK_INFO(DMA45, VREG_ISP_DMA_T),
		VBLOCK_INFO(DMA46, VREG_ISP_DMA_T),
		VBLOCK_INFO(DMA47, VREG_ISP_DMA_T),
		VBLOCK_INFO(DMA48, VREG_ISP_DMA_T),
		VBLOCK_INFO(DMA49, VREG_ISP_DMA_T),
		VBLOCK_INFO(DMA50, VREG_ISP_DMA_T),
		VBLOCK_INFO(DMA51, VREG_ISP_DMA_T),
		VBLOCK_INFO(DMA52, VREG_ISP_DMA_T),
		VBLOCK_INFO(DMA53, VREG_ISP_DMA_T),
		VBLOCK_INFO(DMA54, VREG_ISP_DMA_T),
		VBLOCK_INFO(DMA55, VREG_ISP_DMA_T),
		VBLOCK_INFO(DMA56, VREG_ISP_DMA_T),
		VBLOCK_INFO(DMA57, VREG_ISP_DMA_T),
		VBLOCK_INFO(DMA58, VREG_ISP_DMA_T),
	};

	switch (cq_group) {
	case ISPCQ_ID_PRERAW0:
		*pinfo = m_block_preraw;
		*nblocks = ARRAY_SIZE(m_block_preraw);
		break;
	case ISPCQ_ID_PRERAW1:
		*pinfo = m_block_preraw1;
		*nblocks = ARRAY_SIZE(m_block_preraw1);
		break;
	case ISPCQ_ID_POSTRAW:
		*pinfo = m_block_postraw;
		*nblocks = ARRAY_SIZE(m_block_postraw);
		break;
	default:
		*pinfo = 0;
		*nblocks = 0;
		break;
	}

	return 0;
}

int ispcq_init_cmdset(char *cmdset_ba, int size, uint64_t reg_base)
{
	union CMDSET_FIELD *ctrl_field;

	// set flags for the last cmdset
	ctrl_field = (union CMDSET_FIELD *)(cmdset_ba + size - VREG_SIZE + 4);
	ctrl_field->bits.FLAG_END = 0;
	ctrl_field->bits.FLAG_LAST = 1;

	return 0;
}

int ispcq_set_end_cmdset(char *cmdset_ba, int size)
{
	union CMDSET_FIELD *ctrl_field;

	ctrl_field = (union CMDSET_FIELD *)(cmdset_ba + size - VREG_SIZE + 4);
	ctrl_field->bits.FLAG_END = 1;

	return 0;
}

int ispcq_init_adma_table(char *adma_tb, int num_cmdset)
{
	// 64 addr
	// 32 cmdset size
	// [96]: END, [99]: LINK
	struct ISPCQ_ADMA_DESC_T *pdesc;

	memset(adma_tb, 0, num_cmdset * ADMA_DESC_SIZE);

	pdesc = (struct ISPCQ_ADMA_DESC_T *)(adma_tb +
				      (num_cmdset - 1) * ADMA_DESC_SIZE);

	pdesc->flag.END = 1;

	return 0;
}

int ispcq_add_descriptor(char *adma_tb, int index, uint64_t cmdset_addr,
			 uint32_t cmdset_size)
{
	struct ISPCQ_ADMA_DESC_T *pdesc;

	pdesc = (struct ISPCQ_ADMA_DESC_T *)(adma_tb + index * ADMA_DESC_SIZE);

	pdesc->cmdset_addr = cmdset_addr;
	pdesc->cmdset_size = cmdset_size;

	pdesc->flag.LINK = 0;

	return 0;
}

uint64_t ispcq_get_desc_addr(char *adma_tb, int index)
{
	return (uint64_t)(adma_tb + index * ADMA_DESC_SIZE);
}

int ispcq_set_link_desc(char *adma_tb, int index,
			uint64_t target_desc_addr,
			int is_link)
{
	struct ISPCQ_ADMA_DESC_T *pdesc;

	pdesc = (struct ISPCQ_ADMA_DESC_T *)(adma_tb + index * ADMA_DESC_SIZE);

	if (is_link) {
		pdesc->link_addr = target_desc_addr;
		pdesc->flag.END = 0;
		pdesc->flag.LINK = 1;
	} else
		pdesc->flag.LINK = 0;

	return 0;
}

int ispcq_set_end_desc(char *adma_tb, int index, int is_end)
{
	struct ISPCQ_ADMA_DESC_T *pdesc;

	pdesc = (struct ISPCQ_ADMA_DESC_T *)(adma_tb + index * ADMA_DESC_SIZE);

	if (is_end) {
		pdesc->flag.LINK = 0;
		pdesc->flag.END = 1;
	} else
		pdesc->flag.END = 0;

	return 0;
}

int ispcq_engine_config(uint64_t *phys_regs, struct ispcq_config *cfg)
{
	uint64_t cmdqbase;

	switch (cfg->cq_id) {
	case ISPCQ_ID_PRERAW0:
		cmdqbase = phys_regs[ISP_BLK_ID_CMDQ1];
		break;
	case ISPCQ_ID_PRERAW1:
		cmdqbase = phys_regs[ISP_BLK_ID_CMDQ2];
		break;
	case ISPCQ_ID_POSTRAW:
		cmdqbase = phys_regs[ISP_BLK_ID_CMDQ3];
		break;
	default:
		return -1;
	}

	if (cfg->cq_id == ISPCQ_ID_POSTRAW)
		ISP_WR_BITS(cmdqbase, REG_CMDQ_T, CMDQ_APB_PARA, BASE_ADDR,
			(0x0A000000 >> 22));
	ISP_WR_REG(cmdqbase, REG_CMDQ_T, CMDQ_INT_EVENT, 0xFFFFFFFF);
	ISP_WR_REG(cmdqbase, REG_CMDQ_T, CMDQ_INT_EN, cfg->intr_en);

	if (cfg->op_mode == ISPCQ_OP_ADMA) {
		ISP_WR_REG(cmdqbase, REG_CMDQ_T, CMDQ_DMA_ADDR_L,
			   (cfg->adma_table_pa) & 0xFFFFFFFF);
		ISP_WR_REG(cmdqbase, REG_CMDQ_T, CMDQ_DMA_ADDR_H,
			   ((cfg->adma_table_pa) >> 32) & 0xFFFFFFFF);
		ISP_WR_BITS(cmdqbase, REG_CMDQ_T, CMDQ_DMA_CONFIG, ADMA_EN, 1);
	} else {
		// cmdset mode tbd
		ISP_WR_REG(cmdqbase, REG_CMDQ_T, CMDQ_DMA_ADDR_L,
			   (cfg->cmdset_pa) & 0xFFFFFFFF);
		ISP_WR_REG(cmdqbase, REG_CMDQ_T, CMDQ_DMA_ADDR_H,
			   ((cfg->cmdset_pa) >> 32) & 0xFFFFFFFF);
		ISP_WR_BITS(cmdqbase, REG_CMDQ_T, CMDQ_DMA_CNT, DMA_CNT,
			    cfg->cmdset_size);
		ISP_WR_BITS(cmdqbase, REG_CMDQ_T, CMDQ_DMA_CONFIG, ADMA_EN, 0);
	}

	return 0;
}

int ispcq_engine_start(uint64_t *phys_regs, enum ISPCQ_ID_T id)
{
	uint64_t cmdqbase;

	switch (id) {
	case ISPCQ_ID_PRERAW0:
		cmdqbase = phys_regs[ISP_BLK_ID_CMDQ1];
		break;
	case ISPCQ_ID_PRERAW1:
		cmdqbase = phys_regs[ISP_BLK_ID_CMDQ2];
		break;
	case ISPCQ_ID_POSTRAW:
		cmdqbase = phys_regs[ISP_BLK_ID_CMDQ3];
		break;
	default:
		return -1;
	}

	ISP_WR_BITS(cmdqbase, REG_CMDQ_T, CMDQ_JOB_CTL, JOB_START, 1);

	return 0;
}

int ispblk_crop_enable(struct isp_ctx *ctx, int crop_id, bool en)
{
	uint64_t cropb = ctx->phys_regs[crop_id];

	ISP_WR_BITS(cropb, REG_ISP_CROP_T, CROP_0, CROP_ENABLE, en);
	return 0;
}

int ispblk_crop_config(struct isp_ctx *ctx, int crop_id, struct vip_rect crop)
{
	uint64_t cropb = ctx->phys_regs[crop_id];
	union REG_ISP_CROP_1 reg1;
	union REG_ISP_CROP_2 reg2;

	reg1.bits.CROP_START_Y = crop.y;
	reg1.bits.CROP_END_Y = crop.y + crop.h - 1;
	reg2.bits.CROP_START_X = crop.x;
	reg2.bits.CROP_END_X = crop.x + crop.w - 1;
	ISP_WR_REG(cropb, REG_ISP_CROP_T, CROP_1, reg1.raw);
	ISP_WR_REG(cropb, REG_ISP_CROP_T, CROP_2, reg2.raw);

	if (ctx->is_dpcm_on && ctx->is_tile) {
		ISP_WR_BITS(cropb, REG_ISP_CROP_T, CROP_0, DPCM_MODE, 7);
		ISP_WR_BITS(cropb, REG_ISP_CROP_T, CROP_0, DPCM_XSTR, ctx->tile_cfg.r_in.start);
	} else if (ctx->is_dpcm_on) {
		ISP_WR_BITS(cropb, REG_ISP_CROP_T, CROP_0, DPCM_MODE, 7);
		ISP_WR_BITS(cropb, REG_ISP_CROP_T, CROP_0, DPCM_XSTR, 8191);
	}

	ISP_WR_BITS(cropb, REG_ISP_CROP_T, CROP_0, CROP_ENABLE, true);

	return 0;
}

int ispblk_lscr_config(struct isp_ctx *ctx, int lscr_id, bool en)
{
	uint64_t lscr = ctx->phys_regs[lscr_id];
	uint64_t nom, denom;
	uint32_t lscr_nom;

	nom = 1984 * 1984;
	denom = ((ctx->img_width / 2) * (ctx->img_width / 2)) +
		((ctx->img_height / 2) * (ctx->img_height / 2));
	lscr_nom = (uint32_t)((nom * 1024) / denom);

	ISP_WR_REG(lscr, REG_ISP_LSCR_T, EN, en);
	ISP_WR_REG(lscr, REG_ISP_LSCR_T, HSIZE, ctx->img_width);
	ISP_WR_REG(lscr, REG_ISP_LSCR_T, VSIZE, ctx->img_height);
	ISP_WR_REG(lscr, REG_ISP_LSCR_T, X_CENTER, ctx->img_width >> 1);
	ISP_WR_REG(lscr, REG_ISP_LSCR_T, Y_CENTER, ctx->img_height >> 1);
	ISP_WR_REG(lscr, REG_ISP_LSCR_T, NORM_FACTOR, lscr_nom);
	ISP_WR_REG(lscr, REG_ISP_LSCR_T, LSC_STRENTH, 4095);
	return 0;
}

int ispblk_lscr_set_lut(struct isp_ctx *ctx, int lscr_id, uint16_t *gain_lut,
			uint8_t lut_count)
{
	uint64_t lscr = ctx->phys_regs[lscr_id];
	uint8_t i = 0;

	ISP_WR_REG(lscr, REG_ISP_LSCR_T, INDEX_CLR, 1);
	for (i = 0; i < lut_count; ++i)
		ISP_WR_REG(lscr, REG_ISP_LSCR_T, LSC_LUT, gain_lut[i]);
	return 0;
}

#define F_D (15)
int ispblk_lsc_config(struct isp_ctx *ctx, int lsc_id, bool en)
{
	uint64_t lsc = ctx->phys_regs[lsc_id];
	int width = ctx->src_width;
	int height = ctx->src_height;
	int mesh_num = 37;
	int InnerBlkX = mesh_num - 1 - 4;
	int InnerBlkY = mesh_num - 1 - 4;
	int BlockW = (width + ((InnerBlkX + 2) >> 1)) /
		      MAX(1, InnerBlkX + 2) + 1;
	int BlockH = (height + ((InnerBlkY + 2) >> 1)) /
		      MAX(1, InnerBlkY + 2) + 1;
	int OffsetX = (width - (BlockW*InnerBlkX)) >> 1;
	int OffsetY = (height - (BlockH*InnerBlkY)) >> 1;
	u32 step_x = (1 << (F_D)) / BlockW;
	u32 step_y = (1 << (F_D)) / BlockH;
	u32 offset_y = MAX(2 * BlockH - OffsetY, 0) * step_y;
	u32 offset_x = MAX(2 * BlockW - OffsetX, 0) * step_x;
	//uartlog(" block(%d %d) Offset(%d %d)\n",
	//	    BlockW, BlockH, OffsetX, OffsetY);
	//uartlog(" step(%d %d) offset(%d %d)\n",
	//	    step_x, step_y, offset_x, offset_y);

	if (ctx->is_tile && ctx->is_work_on_r_tile) {
		float OriginalOneTableImgX0 = offset_x + ctx->tile_cfg.r_in.start * step_x;
		float GainNoTile0 = (OriginalOneTableImgX0 / 32768.0);
		float temp = (u32)GainNoTile0;
		float StartingTile1 = (GainNoTile0 - temp) + 1;
		offset_x = (u32)(StartingTile1 * 32768.0);
		//u32 NumGainShift = ((u32)GainNoTile0) - 1;
		//u32 NumGainShiftBase = 16;
		//u32 OffsetNumGainShiftOnImgX0 = (NumGainShift - NumGainShiftBase) * 32768;
		//offset_x = (u32)(StartingTile1 * 32768.0) + OffsetNumGainShiftOnImgX0;
	}

	ISP_WR_REG(lsc, REG_ISP_LSC_T, LSC_ENABLE, en);
	ISP_WR_REG(lsc, REG_ISP_LSC_T, IMG_BAYERID, ctx->rgb_color_mode);
	ISP_WR_REG(lsc, REG_ISP_LSC_T, LSC_STRENGTH, 4095);
	ISP_WR_REG(lsc, REG_ISP_LSC_T, LSC_BLK_NUM_SELECT, 0);
	ISP_WR_REG(lsc, REG_ISP_LSC_T, LSC_GAIN_BASE, 0);
	ISP_WR_REG(lsc, REG_ISP_LSC_T, LSC_XSTEP, step_x);
	ISP_WR_REG(lsc, REG_ISP_LSC_T, LSC_YSTEP, step_y);
	ISP_WR_REG(lsc, REG_ISP_LSC_T, LSC_IMGX0, offset_x);
	ISP_WR_REG(lsc, REG_ISP_LSC_T, LSC_IMGY0, offset_y);
	return 0;
}

#define W_BIT 3
#define H_BIT 3

int ispblk_rgbmap_config(struct isp_ctx *ctx, int map_id)
{
	uint64_t map = ctx->phys_regs[map_id];
	union REG_ISP_RGBMAP_0 reg0;
	union REG_ISP_RGBMAP_1 reg1;

	reg0.raw = 0;
	reg0.bits.RGBMAP_ENABLE = 1;
	//5566
	reg0.bits.RGBMAP_W_BIT = W_BIT;//(ctx->is_offline_postraw) ? 4 : 3;
	reg0.bits.RGBMAP_H_BIT = H_BIT;//(ctx->is_offline_postraw) ? 4 : 3;

	reg0.bits.RGBMAP_W_GRID_NUM =
		UPPER(ctx->img_width, reg0.bits.RGBMAP_W_BIT) - 1;
	reg0.bits.RGBMAP_H_GRID_NUM =
		UPPER(ctx->img_height, reg0.bits.RGBMAP_H_BIT) - 1;
	reg0.bits.IMG_BAYERID = ctx->rgb_color_mode;

	reg1.raw = 0;
	reg1.bits.IMG_WIDTHM1 = ctx->img_width - 1;
	reg1.bits.IMG_HEIGHTM1 = ctx->img_height - 1;
	reg1.bits.RGBMAP_SHDW_SEL = 1;

	ISP_WR_REG(map, REG_ISP_RGBMAP_T, RGBMAP_0, reg0.raw);
	ISP_WR_REG(map, REG_ISP_RGBMAP_T, RGBMAP_1, reg1.raw);

	return 0;
}

int ispblk_lmap_config(struct isp_ctx *ctx, int map_id, bool en)
{
	uint64_t map = ctx->phys_regs[map_id];
	union REG_ISP_LMAP_LMP_0 reg0;
	union REG_ISP_LMAP_LMP_1 reg1;
	union REG_ISP_LMAP_LMP_2 reg2;

	if (!en) {
		ISP_WR_BITS(map, REG_ISP_LMAP_T, LMP_0, LMAP_ENABLE, 0);
		return 0;
	}

	reg0.raw = 0;
	reg0.bits.LMAP_ENABLE = 1;
	reg0.bits.LMAP_W_BIT = 4;
	reg0.bits.LMAP_Y_MODE = 0;
	reg0.bits.LMAP_THD_L = 0;
	reg0.bits.LMAP_THD_H = 4095;

	reg1.raw = 0;
	reg1.bits.LMAP_CROP_WIDTHM1 = ctx->img_width - 1;
	reg1.bits.LMAP_CROP_HEIGHTM1 = ctx->img_height - 1;
	reg1.bits.LMAP_H_BIT = 4;
	reg1.bits.LMAP_BAYER_ID = ctx->rgb_color_mode;
	reg1.bits.LMAP_SHDW_SEL = 1;

	reg2.bits.LMAP_W_GRID_NUM =
		UPPER(ctx->img_width, reg0.bits.LMAP_W_BIT) - 1;
	reg2.bits.LMAP_H_GRID_NUM =
		UPPER(ctx->img_height, reg1.bits.LMAP_H_BIT) - 1;

	ISP_WR_REG(map, REG_ISP_LMAP_T, LMP_0, reg0.raw);
	ISP_WR_REG(map, REG_ISP_LMAP_T, LMP_1, reg1.raw);
	ISP_WR_REG(map, REG_ISP_LMAP_T, LMP_2, reg2.raw);

	return 0;
}

int ispblk_aehist_config(struct isp_ctx *ctx, int blk_id, bool enable)
{
	uint64_t sts = ctx->phys_regs[blk_id];

	ISP_WR_REG(sts, REG_ISP_AE_HIST_T, AE_HIST_BYPASS, !enable);
	if (!enable)
		return 0;
	ISP_WR_BITS(sts, REG_ISP_AE_HIST_T, AE_HIST_ENABLE, AE0_ENABLE, 1);
	ISP_WR_BITS(sts, REG_ISP_AE_HIST_T, AE_HIST_ENABLE, AE0_GAIN_ENABLE, 0);
	ISP_WR_BITS(sts, REG_ISP_AE_HIST_T, AE_HIST_ENABLE, HIST0_ENABLE, 1);
	ISP_WR_BITS(sts, REG_ISP_AE_HIST_T, AE_HIST_ENABLE, AE1_ENABLE, 1);
	ISP_WR_BITS(sts, REG_ISP_AE_HIST_T, AE_HIST_ENABLE, AE1_GAIN_ENABLE, 0);
	ISP_WR_BITS(sts, REG_ISP_AE_HIST_T, AE_HIST_ENABLE, HIST1_ENABLE, 1);

	ISP_WR_REG(sts, REG_ISP_AE_HIST_T, STS_AE_OFFSETX, 3);
	ISP_WR_REG(sts, REG_ISP_AE_HIST_T, STS_AE_OFFSETY, 3);
	ISP_WR_REG(sts, REG_ISP_AE_HIST_T, STS_AE_NUMXM1, 4);
	ISP_WR_REG(sts, REG_ISP_AE_HIST_T, STS_AE_NUMYM1, 4);
	ISP_WR_REG(sts, REG_ISP_AE_HIST_T, STS_AE1_OFFSETX, 0xc);
	ISP_WR_REG(sts, REG_ISP_AE_HIST_T, STS_AE1_OFFSETY, 0xc);
	ISP_WR_REG(sts, REG_ISP_AE_HIST_T, STS_AE1_NUMXM1, 7);
	ISP_WR_REG(sts, REG_ISP_AE_HIST_T, STS_AE1_NUMYM1, 7);
	ISP_WR_REG(sts, REG_ISP_AE_HIST_T, AE_HIST_BAYER_STARTING,
		   ctx->rgb_color_mode);
	return 0;
}

int ispblk_awb_config(struct isp_ctx *ctx, int blk_id, bool enable)
{
	uint64_t sts = ctx->phys_regs[blk_id];

	ISP_WR_BITS(sts, REG_ISP_AWB_T, ENABLE, AWB_ENABLE, enable);
	ISP_WR_BITS(sts, REG_ISP_AWB_T, ENABLE, AWB_GAIN_ENABLE, 0);
	ISP_WR_REG(sts, REG_ISP_AWB_T, BAYER_STARTING, ctx->rgb_color_mode);
	return 0;
}

int ispblk_af_config(struct isp_ctx *ctx, int blk_id, bool enable)
{
	uint64_t sts = ctx->phys_regs[blk_id];

	ISP_WR_BITS(sts, REG_ISP_AF_T, KICKOFF, AF_ENABLE, enable);
	ISP_WR_REG(sts, REG_ISP_AF_T, BYPASS, !enable);
	ISP_WR_REG(sts, REG_ISP_AF_T, CROP_BAYERID, ctx->rgb_color_mode);
	ISP_WR_REG(sts, REG_ISP_AF_T, IMAGE_WIDTH, ctx->img_width - 1);
	ISP_WR_BITS(sts, REG_ISP_AF_T, MXN_IMAGE_WIDTH_M1,
		    AF_MXN_IMAGE_WIDTH, ctx->img_width - 1);
	ISP_WR_BITS(sts, REG_ISP_AF_T, MXN_IMAGE_WIDTH_M1,
		    AF_MXN_IMAGE_HEIGHT, ctx->img_height - 1);
	ISP_WR_REG(sts, REG_ISP_AF_T, BLOCK_WIDTH, 0x14);
	ISP_WR_REG(sts, REG_ISP_AF_T, BLOCK_HEIGHT, 0xf);
	ISP_WR_REG(sts, REG_ISP_AF_T, BLOCK_NUM_X, 0x5);
	ISP_WR_REG(sts, REG_ISP_AF_T, BLOCK_NUM_Y, 0x4);
	return 0;
}

int ispblk_af_gamma_config(struct isp_ctx *ctx, int blk_id,
			   uint8_t sel, uint16_t *data)
{
	uint64_t gamma = ctx->phys_regs[blk_id];
	uint16_t i;
	union REG_ISP_AF_GAMMA_PROG_DATA reg_data;

	ISP_WR_BITS(gamma, REG_ISP_AF_GAMMA_T, GAMMA_PROG_CTRL,
		    GAMMA_WSEL, sel);
	ISP_WR_BITS(gamma, REG_ISP_AF_GAMMA_T, GAMMA_PROG_ST_ADDR,
		    GAMMA_ST_ADDR, 0);
	ISP_WR_BITS(gamma, REG_ISP_AF_GAMMA_T, GAMMA_PROG_ST_ADDR,
		    GAMMA_ST_W, 1);
	ISP_WR_REG(gamma, REG_ISP_AF_GAMMA_T, GAMMA_PROG_MAX, data[256]);
	for (i = 0; i < 256; i += 2) {
		reg_data.raw = 0;
		reg_data.bits.GAMMA_DATA_E = data[i];
		reg_data.bits.GAMMA_DATA_O = data[i + 1];
		reg_data.bits.GAMMA_W = 1;
		ISP_WR_REG(gamma, REG_ISP_AF_GAMMA_T, GAMMA_PROG_DATA,
			   reg_data.raw);
	}
	return 0;
}

int ispblk_af_gamma_enable(struct isp_ctx *ctx, int blk_id,
			   bool enable, uint8_t sel)
{
	uint64_t gamma = ctx->phys_regs[blk_id];

	ISP_WR_BITS(gamma, REG_ISP_AF_GAMMA_T, GAMMA_CTRL, GAMMA_ENABLE,
		    enable);
	ISP_WR_BITS(gamma, REG_ISP_AF_GAMMA_T, GAMMA_PROG_CTRL,
		    GAMMA_RSEL, sel);
	return 0;
}


int ispblk_gms_config(struct isp_ctx *ctx, int blk_id, bool enable)
{
	uint64_t sts = ctx->phys_regs[blk_id];

	ISP_WR_REG(sts, REG_ISP_GMS_T, GMS_ENABLE, enable);
	ISP_WR_REG(sts, REG_ISP_GMS_T, IMG_BAYERID, ctx->rgb_color_mode);
	ISP_WR_REG(sts, REG_ISP_GMS_T, GMS_START_X, 4);
	ISP_WR_REG(sts, REG_ISP_GMS_T, GMS_START_Y, 4);
	ISP_WR_REG(sts, REG_ISP_GMS_T, GMS_X_SECTION_SIZE, 0xf);
	ISP_WR_REG(sts, REG_ISP_GMS_T, GMS_Y_SECTION_SIZE, 0xf);
	ISP_WR_REG(sts, REG_ISP_GMS_T, GMS_X_GAP, 0x2);
	ISP_WR_REG(sts, REG_ISP_GMS_T, GMS_Y_GAP, 0x2);
	return 0;
}
/****************************************************************************
 *	RAW TOP
 ****************************************************************************/
static int _blc_find_hwid(int id)
{
	int blc_id = -1;

	switch (id) {
	case ISP_BLC_ID_PRE0_LE:
		blc_id = ISP_BLK_ID_BLC0;
		break;
	case ISP_BLC_ID_PRE0_SE:
		blc_id = ISP_BLK_ID_BLC1;
		break;
	case ISP_BLC_ID_POST_LE:
		blc_id = ISP_BLK_ID_BLC2;
		break;
	case ISP_BLC_ID_POST_SE:
		blc_id = ISP_BLK_ID_BLC3;
		break;
	case ISP_BLC_ID_PRE1_LE:
		blc_id = ISP_BLK_ID_BLC0_R1;
		break;
	case ISP_BLC_ID_PRE1_SE:
		blc_id = ISP_BLK_ID_BLC1_R1;
		break;
	}
	return blc_id;
}

int ispblk_blc_set_offset(struct isp_ctx *ctx, int blc_id, uint16_t roffset,
			  uint16_t groffset, uint16_t gboffset,
			  uint16_t boffset)
{
	int id = _blc_find_hwid(blc_id);
	uint64_t blc;

	if (id < 0)
		return -EINVAL;
	blc = ctx->phys_regs[id];

	ISP_WR_BITS(blc, REG_ISP_BLC_T, BLC_3, BLC_OFFSET_R, roffset);
	ISP_WR_BITS(blc, REG_ISP_BLC_T, BLC_3, BLC_OFFSET_GR, groffset);
	ISP_WR_BITS(blc, REG_ISP_BLC_T, BLC_4, BLC_OFFSET_GB, gboffset);
	ISP_WR_BITS(blc, REG_ISP_BLC_T, BLC_4, BLC_OFFSET_B, boffset);

	return 0;
}

int ispblk_blc_set_gain(struct isp_ctx *ctx, int blc_id, uint16_t rgain,
			uint16_t grgain, uint16_t gbgain, uint16_t bgain)
{
	int id = _blc_find_hwid(blc_id);
	uint64_t blc;

	if (id < 0)
		return -EINVAL;
	blc = ctx->phys_regs[id];

	ISP_WR_BITS(blc, REG_ISP_BLC_T, BLC_5, BLC_GAIN_R, rgain);
	ISP_WR_BITS(blc, REG_ISP_BLC_T, BLC_5, BLC_GAIN_GR, grgain);
	ISP_WR_BITS(blc, REG_ISP_BLC_T, BLC_6, BLC_GAIN_GB, gbgain);
	ISP_WR_BITS(blc, REG_ISP_BLC_T, BLC_6, BLC_GAIN_B, bgain);

	return 0;
}

int ispblk_blc_enable(struct isp_ctx *ctx, int blc_id, bool en, bool bypass)
{
	int id = _blc_find_hwid(blc_id);
	uint64_t blc;

	if (id < 0)
		return -EINVAL;
	blc = ctx->phys_regs[id];

	ISP_WR_BITS(blc, REG_ISP_BLC_T, BLC_0, BLC_BYPASS, bypass);
	ISP_WR_BITS(blc, REG_ISP_BLC_T, BLC_2, BLC_ENABLE, en);
	ISP_WO_BITS(blc, REG_ISP_BLC_T, IMG_BAYERID, IMG_BAYERID,
		    ctx->rgb_color_mode);

	return 0;
}

static int _wbg_find_hwid(int id)
{
	int wbg_id = -1;

	switch (id) {
	case ISP_WBG_ID_PRE0_LE:
		wbg_id = ISP_BLK_ID_WBG0;
		break;
	case ISP_WBG_ID_PRE0_SE:
		wbg_id = ISP_BLK_ID_WBG1;
		break;
	case ISP_WBG_ID_POST_LE:
		wbg_id = ISP_BLK_ID_WBG2;
		break;
	case ISP_WBG_ID_POST_SE:
		wbg_id = ISP_BLK_ID_WBG3;
		break;
	case ISP_WBG_ID_POST_FS:
		wbg_id = ISP_BLK_ID_WBG4;
		break;
	case ISP_WBG_ID_PRE1_LE:
		wbg_id = ISP_BLK_ID_WBG0_R1;
		break;
	case ISP_WBG_ID_PRE1_SE:
		wbg_id = ISP_BLK_ID_WBG1_R1;
		break;
	}
	return wbg_id;
}

int ispblk_wbg_config(struct isp_ctx *ctx, int wbg_id, uint16_t rgain,
		      uint16_t ggain, uint16_t bgain)
{
	int id = _wbg_find_hwid(wbg_id);
	uint64_t wbg;

	if (id < 0)
		return -EINVAL;

	wbg = ctx->phys_regs[id];
	ISP_WR_BITS(wbg, REG_ISP_WBG_T, WBG_4, WBG_RGAIN, rgain);
	ISP_WR_BITS(wbg, REG_ISP_WBG_T, WBG_4, WBG_GGAIN, ggain);
	ISP_WR_BITS(wbg, REG_ISP_WBG_T, WBG_5, WBG_BGAIN, bgain);
	ISP_WR_BITS(wbg, REG_ISP_WBG_T, IMG_BAYERID, IMG_BAYERID,
		    ctx->rgb_color_mode);

	return 0;
}

int ispblk_wbg_enable(struct isp_ctx *ctx, int wbg_id, bool enable, bool bypass)
{
	int id = _wbg_find_hwid(wbg_id);
	uint64_t wbg;

	if (id < 0)
		return -EINVAL;

	wbg = ctx->phys_regs[id];
	ISP_WR_BITS(wbg, REG_ISP_WBG_T, WBG_0, WBG_BYPASS, bypass);
	ISP_WR_BITS(wbg, REG_ISP_WBG_T, WBG_2, WBG_ENABLE, enable);

	return 0;
}

void isp_fpn_config(struct isp_ctx *ctx, int fpn_id, bool enable)
{
	uint64_t fpn = ctx->phys_regs[fpn_id];
	uint8_t i, j;
	uint16_t val;

	ISP_WR_REG(fpn, REG_ISP_FPN_T, ENABLE, enable);

	if (enable == false)
		return;

	ISP_WR_BITS(fpn, REG_ISP_FPN_T, PROG_CTRL, FPN_WSEL, 0);
	for (j = 0; j < 4; j++) {
	    ISP_WR_BITS(fpn, REG_ISP_FPN_T, MEM_SELECT, FPN_MEM_SELECT, j);
#if 1
	    for (i = 0, val = 0; i < 255 ; ++i, val += 4) {
		ISP_WR_BITS(fpn, REG_ISP_FPN_T, PROG_DATA, FPN_DATA_OFFSEST, val);
		ISP_WR_BITS(fpn, REG_ISP_FPN_T, PROG_DATA, FPN_DATA_GAIN, 1024);
#else
	    for (i = 0, val = 512; i < 255 ; ++i, val += 4) {
		ISP_WR_BITS(fpn, REG_ISP_FPN_T, PROG_DATA, FPN_DATA_OFFSEST, 0);
		ISP_WR_BITS(fpn, REG_ISP_FPN_T, PROG_DATA, FPN_DATA_GAIN, val);
#endif
		ISP_WR_BITS(fpn, REG_ISP_FPN_T, PROG_DATA, FPN_W, 1);
	    }
	}
	ISP_WR_BITS(fpn, REG_ISP_FPN_T, PROG_CTRL, FPN_RSEL, 0);
	ISP_WR_REG(fpn, REG_ISP_FPN_T, IMG_BAYERID, ctx->rgb_color_mode);
	ISP_WR_BITS(fpn, REG_ISP_FPN_T, SECTION_INFO_0, FPN_STARTPIXEL0, 10);
	ISP_WR_BITS(fpn, REG_ISP_FPN_T, SECTION_INFO_0, FPN_WIDTH0, 40);
	ISP_WR_BITS(fpn, REG_ISP_FPN_T, SECTION_INFO_1, FPN_STARTPIXEL1, 60);
	ISP_WR_BITS(fpn, REG_ISP_FPN_T, SECTION_INFO_1, FPN_WIDTH1, 40);
	ISP_WR_BITS(fpn, REG_ISP_FPN_T, SECTION_INFO_2, FPN_STARTPIXEL2, 110);
	ISP_WR_BITS(fpn, REG_ISP_FPN_T, SECTION_INFO_2, FPN_WIDTH2, 40);
	ISP_WR_BITS(fpn, REG_ISP_FPN_T, SECTION_INFO_3, FPN_STARTPIXEL3, 160);
	ISP_WR_BITS(fpn, REG_ISP_FPN_T, SECTION_INFO_3, FPN_WIDTH3, 40);
	ISP_WR_BITS(fpn, REG_ISP_FPN_T, SECTION_INFO_4, FPN_STARTPIXEL4, 210);
	ISP_WR_BITS(fpn, REG_ISP_FPN_T, SECTION_INFO_4, FPN_WIDTH4, 40);
}

int ispblk_fusion_config(struct isp_ctx *ctx, bool enable, bool bypass,
			 bool mc_enable, enum ISP_FS_OUT out_sel)
{
	uint64_t fusion = ctx->phys_regs[ISP_BLK_ID_HDRFUSION];
	union REG_ISP_FUSION_FS_CTRL reg_ctrl;

	ISP_WR_BITS(fusion, REG_ISP_FUSION_T, FS_FRAME_SIZE,
		    FS_WIDTHM1, ctx->img_width-1);
	ISP_WR_BITS(fusion, REG_ISP_FUSION_T, FS_FRAME_SIZE,
		    FS_HEIGHTM1, ctx->img_height-1);

	reg_ctrl.raw = 0;
	reg_ctrl.bits.FS_ENABLE = enable;
	reg_ctrl.bits.FS_BPSS = bypass;
	reg_ctrl.bits.FS_MC_ENABLE = mc_enable;
	reg_ctrl.bits.FS_OUT_SEL = out_sel;
	reg_ctrl.bits.FS_S_MAX = 65535;
	ISP_WR_REG(fusion, REG_ISP_FUSION_T, FS_CTRL, reg_ctrl.raw);
	ISP_WR_REG(fusion, REG_ISP_FUSION_T, FS_SE_GAIN, 0xe0);
	ISP_WR_BITS(fusion, REG_ISP_FUSION_T, FS_LUMA_THD,
		    FS_LUMA_THD_L, 4095);
	ISP_WR_BITS(fusion, REG_ISP_FUSION_T, FS_LUMA_THD,
		    FS_LUMA_THD_H, 4095);
	ISP_WR_BITS(fusion, REG_ISP_FUSION_T, FS_WGT,
		    FS_WGT_MAX, 0xff);
	ISP_WR_BITS(fusion, REG_ISP_FUSION_T, FS_WGT,
		    FS_WGT_MIN, 0);
	ISP_WR_REG(fusion, REG_ISP_FUSION_T, FS_WGT_SLOPE, 0x53b);

	return 0;
}

static void _ltm_init(struct isp_ctx *ctx)
{
	uint64_t ltm = ctx->phys_regs[ISP_BLK_ID_HDRLTM];
	uint8_t lp_dist_wgt[] = {
		31, 20, 12, 8, 6, 4, 2, 1, 1, 1, 1,
	};
	uint8_t lp_diff_wgt[] = {
		16, 16, 15, 14, 10,  6, 5, 4, 3, 2, 1, 1, 1, 1, 1, 1,
		1,  1,  1,  1,  1,  1, 1, 1, 1, 1, 1, 1, 1, 1,
	};
	uint8_t be_dist_wgt[] = {
		31, 20, 12,  8,  6,  4,  2,  1,  1,  1,  1,
	};
	uint8_t de_dist_wgt[] = {
		32, 20, 12,  8,  6,  4,  2,  1,  1,  1,  1,
	};
	uint8_t de_luma_wgt[] = {
		4,  4,  4,  4,  3,  3,  3,  3,  3,  2,  2,  2,  2,  1,  1,  1,
		1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,
	};
	uint64_t addr;

#define LTM_REG_ARRAY_UPDATE11(addr, array)                                   \
	do {                                                                  \
		uint32_t val;                                                 \
		val = array[0] | (array[1] << 5) | (array[2] << 10) |         \
		      (array[3] << 15) | (array[4] << 20) | (array[5] << 25); \
		_reg_write(addr, val);                                        \
		val = array[6] | (array[7] << 5) | (array[8] << 10) |         \
		      (array[9] << 15) | (array[10] << 20);                   \
		_reg_write(addr + 4, val);                                    \
	} while (0)

#define LTM_REG_ARRAY_UPDATE30(addr, array)                                   \
	do {                                                                  \
		uint8_t i;                                                    \
		uint32_t val;                                                 \
		for (i = 0; i < ARRAY_SIZE(array); i += 6) {                  \
			val = array[i] | (array[i+1] << 5) |                  \
			      (array[i+2] << 10) | (array[i+3] << 15) |       \
			      (array[i+4] << 20) | (array[i+5] << 25);        \
			_reg_write(addr, val);                                \
		}                                                             \
	} while (0)

	// lmap0/1_lp_dist_wgt
	addr = ltm + _OFST(REG_ISP_LTM_T, LTM_LMAP0_LP_DIST_WGT_0);
	LTM_REG_ARRAY_UPDATE11(addr, lp_dist_wgt);
	addr = ltm + _OFST(REG_ISP_LTM_T, LTM_LMAP1_LP_DIST_WGT_0);
	LTM_REG_ARRAY_UPDATE11(addr, lp_dist_wgt);

	// lmap0/1_lp_diff_wgt
	addr = ltm + _OFST(REG_ISP_LTM_T, LTM_LMAP0_LP_DIFF_WGT_0);
	LTM_REG_ARRAY_UPDATE30(addr, lp_diff_wgt);
	addr = ltm + _OFST(REG_ISP_LTM_T, LTM_LMAP1_LP_DIFF_WGT_0);
	LTM_REG_ARRAY_UPDATE30(addr, lp_diff_wgt);

	// lp_be_dist_wgt
	addr = ltm + _OFST(REG_ISP_LTM_T, LTM_BE_DIST_WGT_0);
	LTM_REG_ARRAY_UPDATE11(addr, be_dist_wgt);

	// lp_de_dist_wgt
	addr = ltm + _OFST(REG_ISP_LTM_T, LTM_DE_DIST_WGT_0);
	LTM_REG_ARRAY_UPDATE11(addr, de_dist_wgt);

	// lp_de_luma_wgt
	addr = ltm + _OFST(REG_ISP_LTM_T, LTM_DE_LUMA_WGT_0);
	LTM_REG_ARRAY_UPDATE30(addr, de_luma_wgt);

	{
		union REG_ISP_LTM_BE_STRTH_CTRL reg;

		reg.raw = 0;
		reg.bits.BE_LMAP_THR = 35;
		reg.bits.BE_STRTH_DSHFT = 4;
		reg.bits.BE_STRTH_GAIN = 1024;
		ISP_WR_REG(ltm, REG_ISP_LTM_T, LTM_BE_STRTH_CTRL, reg.raw);
	}
	{
		union REG_ISP_LTM_DE_STRTH_CTRL reg;

		reg.raw = 0;
		reg.bits.DE_LMAP_THR = 25;
		reg.bits.DE_STRTH_DSHFT = 4;
		reg.bits.DE_STRTH_GAIN = 393;
		ISP_WR_REG(ltm, REG_ISP_LTM_T, LTM_DE_STRTH_CTRL, reg.raw);
		ISP_WR_BITS(ltm, REG_ISP_LTM_T, LTM_TOP_CTRL, DE_MAX_THR, 830);
	}
	{
		union REG_ISP_LTM_FILTER_WIN_SIZE_CTRL reg;

		reg.raw = 0;
		reg.bits.LMAP0_LP_RNG = 7;
		reg.bits.LMAP1_LP_RNG = 5;
		reg.bits.BE_RNG = 10;
		reg.bits.DE_RNG = 10;
		ISP_WR_REG(ltm, REG_ISP_LTM_T, LTM_FILTER_WIN_SIZE_CTRL,
			   reg.raw);
	}
	{
		union REG_ISP_LTM_BE_STRTH_CTRL reg;

		reg.raw = 0;
		reg.bits.BE_LMAP_THR = 35;
		reg.bits.BE_STRTH_DSHFT = 4;
		reg.bits.BE_STRTH_GAIN = 1024;
		ISP_WR_REG(ltm, REG_ISP_LTM_T, LTM_BE_STRTH_CTRL, reg.raw);
	}
	{
		union REG_ISP_LTM_BGAIN_CTRL_0 reg0;
		union REG_ISP_LTM_BGAIN_CTRL_1 reg1;

		reg0.raw = 0;
		reg0.bits.BRI_IN_THD_L = 0;
		reg0.bits.BRI_IN_THD_H = 640;
		reg0.bits.BRI_OUT_THD_L = 0;
		reg1.bits.BRI_OUT_THD_H = 255;
		reg1.bits.BRI_IN_GAIN_SLOP = 1632;
		ISP_WR_REG(ltm, REG_ISP_LTM_T, LTM_BGAIN_CTRL_0, reg0.raw);
		ISP_WR_REG(ltm, REG_ISP_LTM_T, LTM_BGAIN_CTRL_1, reg1.raw);
	}
	{
		union REG_ISP_LTM_DGAIN_CTRL_0 reg0;
		union REG_ISP_LTM_DGAIN_CTRL_1 reg1;

		reg0.raw = reg1.raw = 0;
		reg0.bits.DAR_IN_THD_L = 0;
		reg0.bits.DAR_IN_THD_H = 640;
		reg0.bits.DAR_OUT_THD_L = 0;
		reg1.bits.DAR_OUT_THD_H = 255;
		reg1.bits.DAR_IN_GAIN_SLOP = 1632;
		ISP_WR_REG(ltm, REG_ISP_LTM_T, LTM_DGAIN_CTRL_0, reg0.raw);
		ISP_WR_REG(ltm, REG_ISP_LTM_T, LTM_DGAIN_CTRL_1, reg1.raw);
	}
	{
		union REG_ISP_LTM_BRI_LCE_CTRL_0 reg0;
		union REG_ISP_LTM_BRI_LCE_CTRL_1 reg1;
		union REG_ISP_LTM_BRI_LCE_CTRL_2 reg2;
		union REG_ISP_LTM_BRI_LCE_CTRL_3 reg3;

		reg0.bits.BRI_LCE_UP_GAIN_0 = 256;
		reg0.bits.BRI_LCE_UP_THD_0 = 3500;
		reg1.bits.BRI_LCE_UP_GAIN_1 = 640;
		reg1.bits.BRI_LCE_UP_THD_1 = 8192;
		reg2.bits.BRI_LCE_UP_GAIN_2 = 640;
		reg2.bits.BRI_LCE_UP_THD_2 = 32768;
		reg3.bits.BRI_LCE_UP_GAIN_3 = 640;
		reg3.bits.BRI_LCE_UP_THD_3 = 65535;
		ISP_WR_REG(ltm, REG_ISP_LTM_T, LTM_BRI_LCE_CTRL_0, reg0.raw);
		ISP_WR_REG(ltm, REG_ISP_LTM_T, LTM_BRI_LCE_CTRL_1, reg1.raw);
		ISP_WR_REG(ltm, REG_ISP_LTM_T, LTM_BRI_LCE_CTRL_2, reg2.raw);
		ISP_WR_REG(ltm, REG_ISP_LTM_T, LTM_BRI_LCE_CTRL_3, reg3.raw);
	}
	ISP_WR_REG(ltm, REG_ISP_LTM_T, LTM_BRI_LCE_CTRL_4, 1);
	ISP_WR_REG(ltm, REG_ISP_LTM_T, LTM_BRI_LCE_CTRL_5, 0);
	ISP_WR_REG(ltm, REG_ISP_LTM_T, LTM_BRI_LCE_CTRL_6, 0);
	{
		union REG_ISP_LTM_BRI_LCE_CTRL_7 reg0;
		union REG_ISP_LTM_BRI_LCE_CTRL_8 reg1;
		union REG_ISP_LTM_BRI_LCE_CTRL_9 reg2;
		union REG_ISP_LTM_BRI_LCE_CTRL_10 reg3;

		reg0.bits.BRI_LCE_DOWN_GAIN_0 = 256;
		reg0.bits.BRI_LCE_DOWN_THD_0 = 3500;
		reg1.bits.BRI_LCE_DOWN_GAIN_1 = 640;
		reg1.bits.BRI_LCE_DOWN_THD_1 = 8192;
		reg2.bits.BRI_LCE_DOWN_GAIN_2 = 768;
		reg2.bits.BRI_LCE_DOWN_THD_2 = 32768;
		reg3.bits.BRI_LCE_DOWN_GAIN_3 = 2048;
		reg3.bits.BRI_LCE_DOWN_THD_3 = 65535;
		ISP_WR_REG(ltm, REG_ISP_LTM_T, LTM_BRI_LCE_CTRL_7, reg0.raw);
		ISP_WR_REG(ltm, REG_ISP_LTM_T, LTM_BRI_LCE_CTRL_8, reg1.raw);
		ISP_WR_REG(ltm, REG_ISP_LTM_T, LTM_BRI_LCE_CTRL_9, reg2.raw);
		ISP_WR_REG(ltm, REG_ISP_LTM_T, LTM_BRI_LCE_CTRL_10, reg3.raw);
	}
	ISP_WR_REG(ltm, REG_ISP_LTM_T, LTM_BRI_LCE_CTRL_11, 335);
	ISP_WR_REG(ltm, REG_ISP_LTM_T, LTM_BRI_LCE_CTRL_12, 21);
	ISP_WR_REG(ltm, REG_ISP_LTM_T, LTM_BRI_LCE_CTRL_13, 160);
	{
		union REG_ISP_LTM_DAR_LCE_CTRL_0 reg0;
		union REG_ISP_LTM_DAR_LCE_CTRL_1 reg1;
		union REG_ISP_LTM_DAR_LCE_CTRL_2 reg2;
		union REG_ISP_LTM_DAR_LCE_CTRL_3 reg3;

		reg0.bits.DAR_LCE_GAIN_0 = 640;
		reg0.bits.DAR_LCE_DIFF_THD_0 = 0;
		reg1.bits.DAR_LCE_GAIN_1 = 384;
		reg1.bits.DAR_LCE_DIFF_THD_1 = 100;
		reg2.bits.DAR_LCE_GAIN_2 = 256;
		reg2.bits.DAR_LCE_DIFF_THD_2 = 200;
		reg3.bits.DAR_LCE_GAIN_3 = 256;
		reg3.bits.DAR_LCE_DIFF_THD_3 = 300;
		ISP_WR_REG(ltm, REG_ISP_LTM_T, LTM_DAR_LCE_CTRL_0, reg0.raw);
		ISP_WR_REG(ltm, REG_ISP_LTM_T, LTM_DAR_LCE_CTRL_1, reg1.raw);
		ISP_WR_REG(ltm, REG_ISP_LTM_T, LTM_DAR_LCE_CTRL_2, reg2.raw);
		ISP_WR_REG(ltm, REG_ISP_LTM_T, LTM_DAR_LCE_CTRL_3, reg3.raw);
	}
	ISP_WR_REG(ltm, REG_ISP_LTM_T, LTM_DAR_LCE_CTRL_4, -40);
	ISP_WR_REG(ltm, REG_ISP_LTM_T, LTM_DAR_LCE_CTRL_5, -19);
	ISP_WR_REG(ltm, REG_ISP_LTM_T, LTM_DAR_LCE_CTRL_6, 0);

	// cfa
	ISP_WR_BITS(ltm, REG_ISP_LTM_T, CFA_CTRL_0, NORM_DEN, 20);
	ISP_WR_BITS(ltm, REG_ISP_LTM_T, CFA_CTRL_0, CFA_HFLP_STRTH, 255);
	ISP_WR_BITS(ltm, REG_ISP_LTM_T, CFA_CTRL_1, CFA_NP_YMIN, 0);
	ISP_WR_BITS(ltm, REG_ISP_LTM_T, CFA_CTRL_1, CFA_NP_YSLOPE, 0);
	ISP_WR_BITS(ltm, REG_ISP_LTM_T, CFA_CTRL_2, CFA_NP_LOW, 0);
	ISP_WR_BITS(ltm, REG_ISP_LTM_T, CFA_CTRL_2, CFA_NP_HIGH, 3);
	ISP_WR_BITS(ltm, REG_ISP_LTM_T, CFA_CTRL_3, CFA_DIFFTHD_MIN, 520);
	ISP_WR_BITS(ltm, REG_ISP_LTM_T, CFA_CTRL_3, CFA_DIFFTHD_SLOPE, 81);
	ISP_WR_BITS(ltm, REG_ISP_LTM_T, CFA_CTRL_4, CFA_DIFFW_LOW, 0);
	ISP_WR_BITS(ltm, REG_ISP_LTM_T, CFA_CTRL_4, CFA_DIFFW_HIGH, 255);
	ISP_WR_BITS(ltm, REG_ISP_LTM_T, CFA_CTRL_5, CFA_SADTHD_MIN, 200);
	ISP_WR_BITS(ltm, REG_ISP_LTM_T, CFA_CTRL_5, CFA_SADTHD_SLOPE, 36);
	ISP_WR_BITS(ltm, REG_ISP_LTM_T, CFA_CTRL_6, CFA_SADW_LOW, 0);
	ISP_WR_BITS(ltm, REG_ISP_LTM_T, CFA_CTRL_6, CFA_SADW_HIGH, 255);
	ISP_WR_BITS(ltm, REG_ISP_LTM_T, CFA_CTRL_7, CFA_LUMATHD_MIN, 300);
	ISP_WR_BITS(ltm, REG_ISP_LTM_T, CFA_CTRL_7, CFA_LUMATHD_SLOPE, 102);
	ISP_WR_BITS(ltm, REG_ISP_LTM_T, CFA_CTRL_8, CFA_LUMAW_LOW, 0);
	ISP_WR_BITS(ltm, REG_ISP_LTM_T, CFA_CTRL_8, CFA_LUMAW_HIGH, 255);
}

void ispblk_ltm_d_lut(struct isp_ctx *ctx, uint8_t sel, uint16_t *data)
{
	uint64_t ltm = ctx->phys_regs[ISP_BLK_ID_HDRLTM];
	uint16_t i;
	union REG_ISP_LTM_DTONE_CURVE_PROG_DATA reg_data;

	ISP_WR_BITS(ltm, REG_ISP_LTM_T, DTONE_CURVE_PROG_CTRL,
		    DTONE_CURVE_WSEL, sel);
	ISP_WR_BITS(ltm, REG_ISP_LTM_T, DTONE_CURVE_CTRL,
		    DTONE_CURVE_ADDR_RST, 1);
	for (i = 0; i < 0x100; i += 2) {
		reg_data.bits.DTONE_CURVE_DATA_E = data[i];
		reg_data.bits.DTONE_CURVE_DATA_O = data[i + 1];
		reg_data.bits.DTONE_CURVE_W = 1;
		ISP_WR_REG(ltm, REG_ISP_LTM_T, DTONE_CURVE_PROG_DATA,
			   reg_data.raw);
	}
	ISP_WR_REG(ltm, REG_ISP_LTM_T, DTONE_CURVE_PROG_MAX,
		   data[0x100]);
	ISP_WR_BITS(ltm, REG_ISP_LTM_T, DTONE_CURVE_PROG_CTRL,
		    DTONE_CURVE_RSEL, sel);
}

void ispblk_ltm_b_lut(struct isp_ctx *ctx, uint8_t sel, uint16_t *data)
{
	uint64_t ltm = ctx->phys_regs[ISP_BLK_ID_HDRLTM];
	uint16_t i;
	union REG_ISP_LTM_BTONE_CURVE_PROG_DATA reg_data;

	ISP_WR_BITS(ltm, REG_ISP_LTM_T, BTONE_CURVE_PROG_CTRL,
		    BTONE_CURVE_WSEL, sel);
	ISP_WR_BITS(ltm, REG_ISP_LTM_T, BTONE_CURVE_CTRL,
		    BTONE_CURVE_ADDR_RST, 1);
	for (i = 0; i < 0x200; i += 2) {
		reg_data.bits.BTONE_CURVE_DATA_E = data[i];
		reg_data.bits.BTONE_CURVE_DATA_O = data[i + 1];
		reg_data.bits.BTONE_CURVE_W = 1;
		ISP_WR_REG(ltm, REG_ISP_LTM_T, BTONE_CURVE_PROG_DATA,
			   reg_data.raw);
	}
	ISP_WR_REG(ltm, REG_ISP_LTM_T, BTONE_CURVE_PROG_MAX,
		   data[0x200]);
	ISP_WR_BITS(ltm, REG_ISP_LTM_T, BTONE_CURVE_PROG_CTRL,
		    BTONE_CURVE_RSEL, sel);
}

void ispblk_ltm_g_lut(struct isp_ctx *ctx, uint8_t sel, uint16_t *data)
{
	uint64_t ltm = ctx->phys_regs[ISP_BLK_ID_HDRLTM];
	uint16_t i;
	union REG_ISP_LTM_GLOBAL_CURVE_PROG_DATA reg_data;

	ISP_WR_BITS(ltm, REG_ISP_LTM_T, GLOBAL_CURVE_PROG_CTRL,
		    GLOBAL_CURVE_WSEL, sel);
	ISP_WR_BITS(ltm, REG_ISP_LTM_T, GLOBAL_CURVE_CTRL,
		    GLOBAL_CURVE_ADDR_RST, 1);
	for (i = 0; i < 0x300; i += 2) {
		reg_data.bits.GLOBAL_CURVE_DATA_E = data[i];
		reg_data.bits.GLOBAL_CURVE_DATA_O = data[i + 1];
		reg_data.bits.GLOBAL_CURVE_W = 1;
		ISP_WR_REG(ltm, REG_ISP_LTM_T, GLOBAL_CURVE_PROG_DATA,
			   reg_data.raw);
	}
	ISP_WR_REG(ltm, REG_ISP_LTM_T, GLOBAL_CURVE_PROG_MAX,
		   data[0x300]);
	ISP_WR_BITS(ltm, REG_ISP_LTM_T, GLOBAL_CURVE_PROG_CTRL,
		    GLOBAL_CURVE_RSEL, sel);
}

void ispblk_ltm_enable(struct isp_ctx *ctx, bool en, bool bypass)
{
	uint64_t ltm = ctx->phys_regs[ISP_BLK_ID_HDRLTM];

	ISP_WR_BITS(ltm, REG_ISP_LTM_T, LTM_TOP_CTRL, LTM_ENABLE, en);
	if (bypass) {
		ISP_WR_BITS(ltm, REG_ISP_LTM_T, LTM_TOP_CTRL, DBG_ENABLE, 1);
		ISP_WR_BITS(ltm, REG_ISP_LTM_T, LTM_TOP_CTRL, DBG_MODE, 4);
	} else {
		ISP_WR_BITS(ltm, REG_ISP_LTM_T, LTM_TOP_CTRL, DBG_ENABLE, 0);
	}
}

/**
 * ispblk_ltm_config - Local-Tone-Mapping configure
 *
 * @param ctx: context of settings
 * @param dehn_en: dtone enhance enable
 * @param dlce_en: dtone lce enable
 * @param behn_en: btone suppressed enable
 * @param blce_en: btone lce enable
 * @param hblk_size: equals to lmap's wbits/4
 * @param vblk_size: equals to lmap's hbits/4
 */
void ispblk_ltm_config(struct isp_ctx *ctx, bool dehn_en,
		       bool dlce_en, bool behn_en, bool blce_en,
		       uint8_t hblk_size, uint8_t vblk_size)
{
	uint64_t ltm = ctx->phys_regs[ISP_BLK_ID_HDRLTM];
	union REG_ISP_LTM_TOP_CTRL reg_top;
	union REG_ISP_LTM_BLK_SIZE reg_blk;
	uint32_t img_width = (ctx->is_tile)
			   ? ctx->tile_cfg.r_out.end + 1
			   : ctx->img_width;

	reg_top.raw = ISP_RD_REG(ltm, REG_ISP_LTM_T, LTM_TOP_CTRL);
	reg_top.bits.DTONE_EHN_EN = dehn_en;
	reg_top.bits.BTONE_EHN_EN = behn_en;
	reg_top.bits.DARK_LCE_EN = dlce_en;
	reg_top.bits.BRIT_LCE_EN = blce_en;
	reg_top.bits.BAYER_ID = ctx->rgb_color_mode;
	ISP_WR_REG(ltm, REG_ISP_LTM_T, LTM_TOP_CTRL, reg_top.raw);

	reg_blk.bits.HORZ_BLK_SIZE = hblk_size;
	reg_blk.bits.BLK_WIDTHM1 = UPPER(img_width, hblk_size + 3) - 1;
	reg_blk.bits.BLK_HEIGHTM1 = UPPER(ctx->img_height, vblk_size + 3) - 1;
	reg_blk.bits.VERT_BLK_SIZE = vblk_size;
	ISP_WR_REG(ltm, REG_ISP_LTM_T, LTM_BLK_SIZE, reg_blk.raw);

	ISP_WR_BITS(ltm, REG_ISP_LTM_T, LTM_FRAME_SIZE, FRAME_WIDTHM1,
		    ctx->img_width - 1);
	ISP_WR_BITS(ltm, REG_ISP_LTM_T, LTM_FRAME_SIZE, FRAME_HEIGHTM1,
		    ctx->img_height - 1);

	if (ctx->is_tile && ctx->is_hdr_on) {
		uint16_t rs_start, rs_end;

		u16 reg_dma_blk_crop_str = 0, reg_blk_win_crop_str = 0;
		u16 reg_dma_blk_crop_end = 0, reg_blk_win_crop_end = 0;
		u16 reg_dma_cnt = 0, reg_img_width = 0;
		u64 dma = 0;
		u16 lmp0_dma_width = 0;

		dma = ctx->phys_regs[ISP_BLK_ID_DMA9];
		lmp0_dma_width = ISP_RD_BITS(dma, REG_ISP_DMA_T, DMA_9, IMG_WIDTH);

		u16 ltm_tile0_end_over_half_blk =
		    (ctx->tile_cfg.l_in.end & ((1 << (hblk_size + 3)) - 1)) < (1 << (hblk_size + 2))
		    ? 0 : 1;
		u16 ltm_tile0_end_blk = ctx->tile_cfg.l_in.end >> (hblk_size + 3);
		u16 ltm_tile0_end_blk_add_window = ltm_tile0_end_blk + 1 + ltm_tile0_end_over_half_blk + 10;
		u16 ltm_tile0_end_blk_tot_blk_clip = (ltm_tile0_end_blk_add_window > lmp0_dma_width) ?
													lmp0_dma_width : ltm_tile0_end_blk_add_window;

		u16 ltm_tile0_end_blk_tot_blk_clip_odd = ((ltm_tile0_end_blk_tot_blk_clip % 2) == 1) ?
														ltm_tile0_end_blk_tot_blk_clip : ltm_tile0_end_blk_tot_blk_clip + 1;

		u16 ltm_tile1_start_over_half_blk = 0;

		u16 ltm_tile1_start_blk = (ctx->tile_cfg.r_in.start >> 4);
		u16 ltm_tile1_start_blk_add_window = ((ltm_tile1_start_blk - 2 + ltm_tile1_start_over_half_blk - 10) < 0) ?
												0 : (ltm_tile1_start_blk - 2 + ltm_tile1_start_over_half_blk - 10);
		u16 ltm_tile1_start_blk_add_window_64pad = (ltm_tile1_start_blk_add_window >> 6) << 6;
		u16 ltm_tile1_end_blk_odd_number = ((lmp0_dma_width % 2) == 1) ?
											lmp0_dma_width : (lmp0_dma_width +1);

		ISP_WR_BITS(ltm, REG_ISP_LTM_T, TILE_MODE_CTRL_0,
			DMA_BLK_CROP_EN, 1);
		ISP_WR_BITS(ltm, REG_ISP_LTM_T, TILE_MODE_CTRL_1,
			BLK_WIN_CROP_EN, 1);
		ISP_WR_BITS(ltm, REG_ISP_LTM_T, TILE_MODE_CTRL_2,
			RS_OUT_CROP_EN, 1);
		if (ctx->is_work_on_r_tile) {
			rs_start = ctx->tile_cfg.r_out.start
				 - ctx->tile_cfg.r_in.start;
			rs_end = ctx->tile_cfg.r_out.end
			       - ctx->tile_cfg.r_in.start;

			reg_dma_blk_crop_str = ltm_tile1_start_blk_add_window - ltm_tile1_start_blk_add_window_64pad;
			reg_dma_blk_crop_end = (ctx->tile_cfg.r_in.end >> 4) - ltm_tile1_start_blk_add_window_64pad;

			reg_blk_win_crop_str = (ctx->tile_cfg.r_in.start >> 4) - 2
									+ ltm_tile1_start_over_half_blk - ltm_tile1_start_blk_add_window;

			uint64_t map = ctx->phys_regs[ISP_BLK_ID_LMP0];
			u32 lmap_w_grid_numm1 = ISP_RD_BITS(map, REG_ISP_LMAP_T, LMP_2, LMAP_W_GRID_NUM);
			reg_blk_win_crop_end = lmap_w_grid_numm1 - ltm_tile1_start_blk_add_window;

			reg_dma_cnt = ((ltm_tile1_end_blk_odd_number - ltm_tile1_start_blk_add_window_64pad + 1) * 12) / 8;
			reg_img_width = ltm_tile1_end_blk_odd_number - ltm_tile1_start_blk_add_window_64pad;

		} else {
			rs_start = 0;
			rs_end = ctx->tile_cfg.l_in.end;

			reg_dma_blk_crop_end = ltm_tile0_end_blk_tot_blk_clip;
			reg_blk_win_crop_end = ltm_tile0_end_blk + 1 + ltm_tile0_end_over_half_blk;

			reg_dma_cnt = (ltm_tile0_end_blk_tot_blk_clip_odd + 1) * 12 / 8;
			reg_img_width = ltm_tile0_end_blk_tot_blk_clip_odd;

		}

		ISP_WR_BITS(ltm, REG_ISP_LTM_T, TILE_MODE_CTRL_0, DMA_BLK_CROP_STR, reg_dma_blk_crop_str);
		ISP_WR_BITS(ltm, REG_ISP_LTM_T, TILE_MODE_CTRL_0, DMA_BLK_CROP_END, reg_dma_blk_crop_end);

		ISP_WR_BITS(ltm, REG_ISP_LTM_T, TILE_MODE_CTRL_1, BLK_WIN_CROP_STR, reg_blk_win_crop_str);
		ISP_WR_BITS(ltm, REG_ISP_LTM_T, TILE_MODE_CTRL_1, BLK_WIN_CROP_END, reg_blk_win_crop_end);

		dma = ctx->phys_regs[ISP_BLK_ID_DMA12];
		ISP_WR_BITS(dma, REG_ISP_DMA_T, DMA_9, IMG_WIDTH, reg_img_width);
		ISP_WR_BITS(dma, REG_ISP_DMA_T, DMA_2, DMA_CNT, reg_dma_cnt);

		dma = ctx->phys_regs[ISP_BLK_ID_DMA13];
		ISP_WR_BITS(dma, REG_ISP_DMA_T, DMA_9, IMG_WIDTH, reg_img_width);
		ISP_WR_BITS(dma, REG_ISP_DMA_T, DMA_2, DMA_CNT, reg_dma_cnt);

		ISP_WR_BITS(ltm, REG_ISP_LTM_T, TILE_MODE_CTRL_2,
			    RS_OUT_CROP_STR, rs_start);
		ISP_WR_BITS(ltm, REG_ISP_LTM_T, TILE_MODE_CTRL_2,
			    RS_OUT_CROP_END, rs_end);
	} else {
		ISP_WR_BITS(ltm, REG_ISP_LTM_T, TILE_MODE_CTRL_0,
			DMA_BLK_CROP_EN, 0);
		ISP_WR_BITS(ltm, REG_ISP_LTM_T, TILE_MODE_CTRL_1,
			BLK_WIN_CROP_EN, 0);
		ISP_WR_BITS(ltm, REG_ISP_LTM_T, TILE_MODE_CTRL_2,
			RS_OUT_CROP_EN, 0);
	}
}

static void _manr_init(struct isp_ctx *ctx)
{
	uint64_t blk = ctx->phys_regs[ISP_BLK_ID_MANR];
	union REG_ISP_MM_04 reg_04;
	union REG_ISP_MM_08 reg_08;
	union REG_ISP_MM_34 reg_34;
	union REG_ISP_MM_38 reg_38;
	uint64_t data[] = {
		0x1d8420368108, 0x1e0428368106, 0x1e8430366104, 0x1f0438366102,
		0x1f84403640ff, 0x1f84483640fd, 0x20044c3620fb, 0x2084543620f9,
		0x21045c3600f6, 0x2184643600f4, 0x22046c35e0f2, 0x22847435e0f0,
		0x23047835c0ed, 0x23848035a0eb, 0x24048835a0e9, 0x2484903580e7,
		0x2504983560e5, 0x2584a03560e3, 0x2604a43540e0, 0x2684ac3520de,
		0x2784b43500dc, 0x2804bc3500da, 0x2884c434e0d8, 0x2904c834c0d6,
		0x2984d034a0d4, 0x2a04d83480d2, 0x2a84e03460d0, 0x2b04e43460ce,
		0x2c04ec3440cc, 0x2c84f43420ca, 0x2d04fc3400c7, 0x2d850433e0c5,
		0x2e850833c0c3, 0x2f051033a0c2, 0x2f85183380c0, 0x3005203360be,
		0x3085243340bc, 0x31852c3320ba, 0x3205343300b8, 0x32853832e0b6,
		0x33854032a0b4, 0x3405483280b2, 0x3485503260b0, 0x3585543240ae,
		0x36055c3220ac, 0x3685643200ab, 0x37856831c0a9, 0x38057031a0a7,
		0x3885743180a5, 0x39857c3160a3, 0x3a05843120a1, 0x3b05883100a0,
		0x3b859030e09e, 0x3c859830c09c, 0x3d059c30809a, 0x3e05a4306099,
		0x3e85a8304097, 0x3f85b0300095, 0x4005b42fe094, 0x4105bc2fa092,
		0x4185c02f8090, 0x4285c82f608f, 0x4305cc2f208d, 0x4405d42f008b,
		0x4505d82ec08a,
	};
	uint8_t i = 0;

	reg_04.bits.MMAP_0_LPF_00 = 3;
	reg_04.bits.MMAP_0_LPF_01 = 4;
	reg_04.bits.MMAP_0_LPF_02 = 3;
	reg_04.bits.MMAP_0_LPF_10 = 4;
	reg_04.bits.MMAP_0_LPF_11 = 4;
	reg_04.bits.MMAP_0_LPF_12 = 4;
	reg_04.bits.MMAP_0_LPF_20 = 3;
	reg_04.bits.MMAP_0_LPF_21 = 4;
	reg_04.bits.MMAP_0_LPF_22 = 3;
	ISP_WR_REG(blk, REG_ISP_MM_T, REG_04, reg_04.raw);

	reg_08.bits.MMAP_0_MAP_CORING = 0;
	reg_08.bits.MMAP_0_MAP_GAIN   = 64;
	reg_08.bits.MMAP_0_MAP_THD_L  = 0;
	reg_08.bits.MMAP_0_MAP_THD_H  = 255;
	ISP_WR_REG(blk, REG_ISP_MM_T, REG_08, reg_08.raw);
	ISP_WR_BITS(blk, REG_ISP_MM_T, REG_0C,
		    MMAP_0_LUMA_ADAPT_LUT_IN_0, 0);
	ISP_WR_BITS(blk, REG_ISP_MM_T, REG_0C,
		    MMAP_0_LUMA_ADAPT_LUT_IN_1, 600);
	ISP_WR_BITS(blk, REG_ISP_MM_T, REG_10,
		    MMAP_0_LUMA_ADAPT_LUT_IN_2, 1500);
	ISP_WR_BITS(blk, REG_ISP_MM_T, REG_10,
		    MMAP_0_LUMA_ADAPT_LUT_IN_3, 2500);
	ISP_WR_BITS(blk, REG_ISP_MM_T, REG_14,
		    MMAP_0_LUMA_ADAPT_LUT_OUT_0, 63);
	ISP_WR_BITS(blk, REG_ISP_MM_T, REG_14,
		    MMAP_0_LUMA_ADAPT_LUT_OUT_1, 48);
	ISP_WR_BITS(blk, REG_ISP_MM_T, REG_14,
		    MMAP_0_LUMA_ADAPT_LUT_OUT_2, 8);
	ISP_WR_BITS(blk, REG_ISP_MM_T, REG_14,
		    MMAP_0_LUMA_ADAPT_LUT_OUT_3, 2);
	ISP_WR_BITS(blk, REG_ISP_MM_T, REG_18,
		    MMAP_0_LUMA_ADAPT_LUT_SLOPE_0, -27);
	ISP_WR_BITS(blk, REG_ISP_MM_T, REG_18,
		    MMAP_0_LUMA_ADAPT_LUT_SLOPE_1, 0);
	ISP_WR_BITS(blk, REG_ISP_MM_T, REG_1C,
		    MMAP_0_LUMA_ADAPT_LUT_SLOPE_2, 0);
	ISP_WR_BITS(blk, REG_ISP_MM_T, REG_1C, MMAP_0_MAP_DSHIFT_BIT, 5);
	ISP_WR_BITS(blk, REG_ISP_MM_T, REG_20, MMAP_0_IIR_PRTCT_LUT_IN_0, 0);
	ISP_WR_BITS(blk, REG_ISP_MM_T, REG_20, MMAP_0_IIR_PRTCT_LUT_IN_1, 45);
	ISP_WR_BITS(blk, REG_ISP_MM_T, REG_20, MMAP_0_IIR_PRTCT_LUT_IN_2, 90);
	ISP_WR_BITS(blk, REG_ISP_MM_T, REG_20, MMAP_0_IIR_PRTCT_LUT_IN_3, 255);
	ISP_WR_BITS(blk, REG_ISP_MM_T, REG_24, MMAP_0_IIR_PRTCT_LUT_OUT_0, 6);
	ISP_WR_BITS(blk, REG_ISP_MM_T, REG_24, MMAP_0_IIR_PRTCT_LUT_OUT_1, 10);
	ISP_WR_BITS(blk, REG_ISP_MM_T, REG_24, MMAP_0_IIR_PRTCT_LUT_OUT_2, 9);
	ISP_WR_BITS(blk, REG_ISP_MM_T, REG_24, MMAP_0_IIR_PRTCT_LUT_OUT_3, 2);
	ISP_WR_BITS(blk, REG_ISP_MM_T, REG_28,
		    MMAP_0_IIR_PRTCT_LUT_SLOPE_0, 12);
	ISP_WR_BITS(blk, REG_ISP_MM_T, REG_28,
		    MMAP_0_IIR_PRTCT_LUT_SLOPE_1, -4);
	ISP_WR_BITS(blk, REG_ISP_MM_T, REG_2C,
		    MMAP_0_IIR_PRTCT_LUT_SLOPE_2, -4);

	ISP_WR_BITS(blk, REG_ISP_MM_T, REG_70, MMAP_0_GAIN_RATIO_R, 4096);
	ISP_WR_BITS(blk, REG_ISP_MM_T, REG_70, MMAP_0_GAIN_RATIO_G, 4096);
	ISP_WR_BITS(blk, REG_ISP_MM_T, REG_74, MMAP_0_GAIN_RATIO_B, 4096);
	ISP_WR_BITS(blk, REG_ISP_MM_T, REG_78, MMAP_0_NS_SLOPE_R, 5);
	ISP_WR_BITS(blk, REG_ISP_MM_T, REG_78, MMAP_0_NS_SLOPE_G, 4);
	ISP_WR_BITS(blk, REG_ISP_MM_T, REG_7C, MMAP_0_NS_SLOPE_B, 6);
	ISP_WR_BITS(blk, REG_ISP_MM_T, REG_80, MMAP_0_NS_LUMA_TH0_R, 16);
	ISP_WR_BITS(blk, REG_ISP_MM_T, REG_80, MMAP_0_NS_LUMA_TH0_G, 16);
	ISP_WR_BITS(blk, REG_ISP_MM_T, REG_84, MMAP_0_NS_LUMA_TH0_B, 16);
	ISP_WR_BITS(blk, REG_ISP_MM_T, REG_84, MMAP_0_NS_LOW_OFFSET_R, 0);
	ISP_WR_BITS(blk, REG_ISP_MM_T, REG_88, MMAP_0_NS_LOW_OFFSET_G, 2);
	ISP_WR_BITS(blk, REG_ISP_MM_T, REG_88, MMAP_0_NS_LOW_OFFSET_B, 0);
	ISP_WR_BITS(blk, REG_ISP_MM_T, REG_8C, MMAP_0_NS_HIGH_OFFSET_R, 724);
	ISP_WR_BITS(blk, REG_ISP_MM_T, REG_8C, MMAP_0_NS_HIGH_OFFSET_G, 724);
	ISP_WR_BITS(blk, REG_ISP_MM_T, REG_90, MMAP_0_NS_HIGH_OFFSET_B, 724);

	reg_34.bits.MMAP_1_LPF_00 = 1;
	reg_34.bits.MMAP_1_LPF_01 = 1;
	reg_34.bits.MMAP_1_LPF_02 = 1;
	reg_34.bits.MMAP_1_LPF_10 = 1;
	reg_34.bits.MMAP_1_LPF_11 = 1;
	reg_34.bits.MMAP_1_LPF_12 = 1;
	reg_34.bits.MMAP_1_LPF_20 = 1;
	reg_34.bits.MMAP_1_LPF_21 = 1;
	reg_34.bits.MMAP_1_LPF_22 = 1;
	ISP_WR_REG(blk, REG_ISP_MM_T, REG_34, reg_34.raw);

	reg_38.bits.MMAP_1_MAP_CORING = 0;
	reg_38.bits.MMAP_1_MAP_GAIN   = 13;
	reg_38.bits.MMAP_1_MAP_THD_L  = 0;
	reg_38.bits.MMAP_1_MAP_THD_H  = 255;
	ISP_WR_REG(blk, REG_ISP_MM_T, REG_38, reg_38.raw);
	ISP_WR_BITS(blk, REG_ISP_MM_T, REG_3C,
		    MMAP_1_LUMA_ADAPT_LUT_IN_0, 0);
	ISP_WR_BITS(blk, REG_ISP_MM_T, REG_3C,
		    MMAP_1_LUMA_ADAPT_LUT_IN_1, 600);
	ISP_WR_BITS(blk, REG_ISP_MM_T, REG_40,
		    MMAP_1_LUMA_ADAPT_LUT_IN_2, 1500);
	ISP_WR_BITS(blk, REG_ISP_MM_T, REG_40,
		    MMAP_1_LUMA_ADAPT_LUT_IN_3, 2500);
	ISP_WR_BITS(blk, REG_ISP_MM_T, REG_44,
		    MMAP_1_LUMA_ADAPT_LUT_OUT_0, 32);
	ISP_WR_BITS(blk, REG_ISP_MM_T, REG_44,
		    MMAP_1_LUMA_ADAPT_LUT_OUT_1, 16);
	ISP_WR_BITS(blk, REG_ISP_MM_T, REG_44,
		    MMAP_1_LUMA_ADAPT_LUT_OUT_2, 16);
	ISP_WR_BITS(blk, REG_ISP_MM_T, REG_44,
		    MMAP_1_LUMA_ADAPT_LUT_OUT_3, 16);
	ISP_WR_BITS(blk, REG_ISP_MM_T, REG_48,
		    MMAP_1_LUMA_ADAPT_LUT_SLOPE_0, 0);
	ISP_WR_BITS(blk, REG_ISP_MM_T, REG_48,
		    MMAP_1_LUMA_ADAPT_LUT_SLOPE_1, 0);
	ISP_WR_BITS(blk, REG_ISP_MM_T, REG_4C,
		    MMAP_1_LUMA_ADAPT_LUT_SLOPE_2, 0);
	ISP_WR_BITS(blk, REG_ISP_MM_T, REG_4C, MMAP_1_MAP_DSHIFT_BIT, 5);
	ISP_WR_BITS(blk, REG_ISP_MM_T, REG_50, MMAP_1_IIR_PRTCT_LUT_IN_0, 0);
	ISP_WR_BITS(blk, REG_ISP_MM_T, REG_50, MMAP_1_IIR_PRTCT_LUT_IN_1, 45);
	ISP_WR_BITS(blk, REG_ISP_MM_T, REG_50, MMAP_1_IIR_PRTCT_LUT_IN_2, 90);
	ISP_WR_BITS(blk, REG_ISP_MM_T, REG_50, MMAP_1_IIR_PRTCT_LUT_IN_3, 255);
	ISP_WR_BITS(blk, REG_ISP_MM_T, REG_54, MMAP_1_IIR_PRTCT_LUT_OUT_0, 12);
	ISP_WR_BITS(blk, REG_ISP_MM_T, REG_54, MMAP_1_IIR_PRTCT_LUT_OUT_1, 15);
	ISP_WR_BITS(blk, REG_ISP_MM_T, REG_54, MMAP_1_IIR_PRTCT_LUT_OUT_2, 12);
	ISP_WR_BITS(blk, REG_ISP_MM_T, REG_54, MMAP_1_IIR_PRTCT_LUT_OUT_3, 1);
	ISP_WR_BITS(blk, REG_ISP_MM_T, REG_58,
		    MMAP_1_IIR_PRTCT_LUT_SLOPE_0, 0);
	ISP_WR_BITS(blk, REG_ISP_MM_T, REG_58,
		    MMAP_1_IIR_PRTCT_LUT_SLOPE_1, 0);
	ISP_WR_BITS(blk, REG_ISP_MM_T, REG_5C,
		    MMAP_1_IIR_PRTCT_LUT_SLOPE_2, 0);

	ISP_WR_BITS(blk, REG_ISP_MM_T, REG_A0, MMAP_1_GAIN_RATIO_R, 4096);
	ISP_WR_BITS(blk, REG_ISP_MM_T, REG_A0, MMAP_1_GAIN_RATIO_G, 4096);
	ISP_WR_BITS(blk, REG_ISP_MM_T, REG_A4, MMAP_1_GAIN_RATIO_B, 4096);
	ISP_WR_BITS(blk, REG_ISP_MM_T, REG_A8, MMAP_1_NS_SLOPE_R, 5*4);
	ISP_WR_BITS(blk, REG_ISP_MM_T, REG_A8, MMAP_1_NS_SLOPE_G, 4*4);
	ISP_WR_BITS(blk, REG_ISP_MM_T, REG_AC, MMAP_1_NS_SLOPE_B, 6*4);
	ISP_WR_BITS(blk, REG_ISP_MM_T, REG_B0, MMAP_1_NS_LUMA_TH0_R, 16);
	ISP_WR_BITS(blk, REG_ISP_MM_T, REG_B0, MMAP_1_NS_LUMA_TH0_G, 16);
	ISP_WR_BITS(blk, REG_ISP_MM_T, REG_B4, MMAP_1_NS_LUMA_TH0_B, 16);
	ISP_WR_BITS(blk, REG_ISP_MM_T, REG_B4, MMAP_1_NS_LOW_OFFSET_R, 0);
	ISP_WR_BITS(blk, REG_ISP_MM_T, REG_B8, MMAP_1_NS_LOW_OFFSET_G, 2);
	ISP_WR_BITS(blk, REG_ISP_MM_T, REG_B8, MMAP_1_NS_LOW_OFFSET_B, 0);
	ISP_WR_BITS(blk, REG_ISP_MM_T, REG_BC, MMAP_1_NS_HIGH_OFFSET_R, 724);
	ISP_WR_BITS(blk, REG_ISP_MM_T, REG_BC, MMAP_1_NS_HIGH_OFFSET_G, 724);
	ISP_WR_BITS(blk, REG_ISP_MM_T, REG_C0, MMAP_1_NS_HIGH_OFFSET_B, 724);

	ISP_WR_BITS(blk, REG_ISP_MM_T, REG_6C, SRAM_WEN, 1);
	for (i = 0; i < ARRAY_SIZE(data); ++i) {
		ISP_WR_BITS(blk, REG_ISP_MM_T, REG_6C, SRAM_WADD, i);
		ISP_WR_REG(blk, REG_ISP_MM_T, REG_64, data[i] & 0xffffffff);
		ISP_WR_REG(blk, REG_ISP_MM_T, REG_68, data[i] >> 32);
	}
	ISP_WR_BITS(blk, REG_ISP_MM_T, REG_6C, SRAM_WEN, 0);
}

void ispblk_manr_config(struct isp_ctx *ctx, bool en)
{
	uint64_t blk = ctx->phys_regs[ISP_BLK_ID_MANR];
	union REG_ISP_MM_00 reg_00;
	union REG_ISP_MM_C4 reg_c4;
	//5566
	uint8_t w_bit = W_BIT;//(ctx->is_offline_postraw) ? 4 : 3;
	uint8_t h_bit = H_BIT;//(ctx->is_offline_postraw) ? 4 : 3;
	uint32_t img_width = (ctx->is_tile)
			   ? ctx->tile_cfg.r_out.end + 1
			   : ctx->img_width;
	uint8_t dma_enable = (ctx->is_offline_postraw)
			     ? (ctx->is_hdr_on) ? 0xff : 0xf5
			     : 0xf1;

	reg_00.raw = ISP_RD_REG(blk, REG_ISP_MM_T, REG_00);
	reg_00.bits.IMG_BAYERID = ctx->rgb_color_mode;
	reg_00.bits.ON_THE_FLY = !ctx->is_offline_postraw;
	reg_00.bits.BYPASS = (en) ? 0 : 1;
	reg_00.bits.DMA_BYPASS = (en) ? 0 : 1;
	reg_00.bits.MMAP_1_ENABLE = (ctx->is_hdr_on) ? 1 : 0;
	ISP_WR_REG(blk, REG_ISP_MM_T, REG_00, reg_00.raw);

	if (!en)
		return;

	ISP_WR_BITS(blk, REG_ISP_MM_T, REG_30, IMG_WIDTHM1, img_width-1);
	ISP_WR_BITS(blk, REG_ISP_MM_T, REG_30, IMG_HEIGHTM1, ctx->img_height-1);
	ISP_WR_BITS(blk, REG_ISP_MM_T, REG_60, RGBMAP_W_BIT, w_bit);
	ISP_WR_BITS(blk, REG_ISP_MM_T, REG_60, RGBMAP_H_BIT, h_bit);

	reg_c4.bits.IMG_WIDTH_CROP = ((UPPER(img_width, w_bit) + 6)/7)*7-1;
	reg_c4.bits.IMG_HEIGHT_CROP = UPPER(ctx->img_height, h_bit) - 1;
	reg_c4.bits.CROP_ENABLE = 1;
	ISP_WR_REG(blk, REG_ISP_MM_T, REG_C4, reg_c4.raw);

	ISP_WR_BITS(blk, REG_ISP_MM_T, REG_C8, CROP_W_END,
		UPPER(img_width, w_bit) - 1);
	ISP_WR_BITS(blk, REG_ISP_MM_T, REG_CC, CROP_H_END,
		UPPER(ctx->img_height, h_bit) - 1);

	ISP_WR_BITS(blk, REG_ISP_MM_T, REG_D0,
		    IMG_WIDTH_CROP_SCALAR, img_width - 1);
	ISP_WR_BITS(blk, REG_ISP_MM_T, REG_D0,
		    IMG_HEIGHT_CROP_SCALAR, ctx->img_height - 1);
	ISP_WR_BITS(blk, REG_ISP_MM_T, REG_D0,
		    CROP_ENABLE_SCALAR, ctx->is_tile);
	if (ctx->is_tile) {
		if (ctx->is_work_on_r_tile) {
			ISP_WR_BITS(blk, REG_ISP_MM_T, REG_D4,
				    CROP_W_STR_SCALAR,
				    ctx->tile_cfg.r_in.start);
			ISP_WR_BITS(blk, REG_ISP_MM_T, REG_D4,
				    CROP_W_END_SCALAR,
				    ctx->tile_cfg.r_in.end);
		} else {
			ISP_WR_BITS(blk, REG_ISP_MM_T, REG_D4,
				    CROP_W_STR_SCALAR,
				    ctx->tile_cfg.l_in.start);
			ISP_WR_BITS(blk, REG_ISP_MM_T, REG_D4,
				    CROP_W_END_SCALAR,
				    ctx->tile_cfg.l_in.end);
		}
		ISP_WR_BITS(blk, REG_ISP_MM_T, REG_D8, CROP_H_STR_SCALAR, 0);
		ISP_WR_BITS(blk, REG_ISP_MM_T, REG_D8, CROP_H_END_SCALAR,
			ctx->img_height - 1);

		ISP_WR_BITS(blk, REG_ISP_MM_T, REG_6C, MANR_DEBUG, 0x80);
	}

	ISP_WR_BITS(blk, REG_ISP_MM_T, REG_6C, DMA_ENABLE, dma_enable);
}

static void _bnr_init(struct isp_ctx *ctx)
{
	uint64_t bnr = ctx->phys_regs[ISP_BLK_ID_BNR];
	uint8_t intensity_sel[8] = {0, 0, 0, 0, 0, 0, 0, 0};
	uint8_t weight_lut[256] = {
		31, 16, 8,  4,  2,  1,  1,  0,  0,  0,  0,  0,  0,  0,  0,  0,
		0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
		0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
		0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
		0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
		0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
		0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
		0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
		0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
		0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
		0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
		0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
		0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
		0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
		0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
		0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	};
	uint8_t lsc_lut[32] = {
		32, 32, 32, 32, 32, 32, 32, 33, 33, 34, 34, 35, 36, 37, 38, 39,
		40, 41, 43, 44, 45, 47, 49, 51, 53, 55, 57, 59, 61, 64, 66, 69,
	};
	uint16_t i = 0;

	ISP_WO_BITS(bnr, REG_ISP_BNR_T, INDEX_CLR, BNR_INDEX_CLR, 1);
	for (i = 0; i < ARRAY_SIZE(intensity_sel); ++i)
		ISP_WR_REG(bnr, REG_ISP_BNR_T, INTENSITY_SEL, intensity_sel[i]);
	for (i = 0; i < ARRAY_SIZE(weight_lut); ++i)
		ISP_WR_REG(bnr, REG_ISP_BNR_T, WEIGHT_LUT, weight_lut[i]);
	for (i = 0; i < ARRAY_SIZE(lsc_lut); ++i)
		ISP_WR_REG(bnr, REG_ISP_BNR_T, LSC_LUT, lsc_lut[i]);

	ISP_WO_BITS(bnr, REG_ISP_BNR_T, SHADOW_RD_SEL,
		    SHADOW_RD_SEL, 1);
	ISP_WO_BITS(bnr, REG_ISP_BNR_T, OUT_SEL,
		    BNR_OUT_SEL, ISP_BNR_OUT_BYPASS);

	ISP_WO_BITS(bnr, REG_ISP_BNR_T, STRENGTH_MODE,
		    BNR_STRENGTH_MODE, 0);
	ISP_WO_BITS(bnr, REG_ISP_BNR_T, WEIGHT_INTRA_0,
		    BNR_WEIGHT_INTRA_0, 6);
	ISP_WO_BITS(bnr, REG_ISP_BNR_T, WEIGHT_INTRA_1,
		    BNR_WEIGHT_INTRA_1, 6);
	ISP_WO_BITS(bnr, REG_ISP_BNR_T, WEIGHT_INTRA_2,
		    BNR_WEIGHT_INTRA_2, 6);
	ISP_WO_BITS(bnr, REG_ISP_BNR_T, WEIGHT_NORM_1,
		    BNR_WEIGHT_NORM_1, 7);
	ISP_WO_BITS(bnr, REG_ISP_BNR_T, WEIGHT_NORM_2,
		    BNR_WEIGHT_NORM_2, 5);
	ISP_WO_BITS(bnr, REG_ISP_BNR_T, NEIGHBOR_MAX,
		    BNR_NEIGHBOR_MAX, 1);

	ISP_WO_BITS(bnr, REG_ISP_BNR_T, RES_K_SMOOTH,
		    BNR_RES_K_SMOOTH, 0);
	ISP_WO_BITS(bnr, REG_ISP_BNR_T, RES_K_TEXTURE,
		    BNR_RES_K_TEXTURE, 0);
	ISP_WO_BITS(bnr, REG_ISP_BNR_T, VAR_TH, BNR_VAR_TH, 128);
	ISP_WO_BITS(bnr, REG_ISP_BNR_T, WEIGHT_SM, BNR_WEIGHT_SM, 0);
	ISP_WO_BITS(bnr, REG_ISP_BNR_T, WEIGHT_V, BNR_WEIGHT_V, 0);
	ISP_WO_BITS(bnr, REG_ISP_BNR_T, WEIGHT_H, BNR_WEIGHT_H, 0);
	ISP_WO_BITS(bnr, REG_ISP_BNR_T, WEIGHT_D45, BNR_WEIGHT_D45, 0);
	ISP_WO_BITS(bnr, REG_ISP_BNR_T, WEIGHT_D135, BNR_WEIGHT_D135, 0);
	ISP_WO_BITS(bnr, REG_ISP_BNR_T, LSC_RATIO, BNR_LSC_RATIO, 15);

	ISP_WO_BITS(bnr, REG_ISP_BNR_T, NS_SLOPE_B,
		    BNR_NS_SLOPE_B, 135);
	ISP_WO_BITS(bnr, REG_ISP_BNR_T, NS_SLOPE_GB,
		    BNR_NS_SLOPE_GB, 106);
	ISP_WO_BITS(bnr, REG_ISP_BNR_T, NS_SLOPE_GR,
		    BNR_NS_SLOPE_GR, 106);
	ISP_WO_BITS(bnr, REG_ISP_BNR_T, NS_SLOPE_R,
		    BNR_NS_SLOPE_R, 127);
	ISP_WO_BITS(bnr, REG_ISP_BNR_T, NS_OFFSET0_B,
		    BNR_NS_OFFSET0_B, 177);
	ISP_WO_BITS(bnr, REG_ISP_BNR_T, NS_OFFSET0_GB,
		    BNR_NS_OFFSET0_GB, 169);
	ISP_WO_BITS(bnr, REG_ISP_BNR_T, NS_OFFSET0_GR,
		    BNR_NS_OFFSET0_GR, 169);
	ISP_WO_BITS(bnr, REG_ISP_BNR_T, NS_OFFSET0_R,
		    BNR_NS_OFFSET0_R, 182);
	ISP_WO_BITS(bnr, REG_ISP_BNR_T, NS_OFFSET1_B,
		    BNR_NS_OFFSET1_B, 1023);
	ISP_WO_BITS(bnr, REG_ISP_BNR_T, NS_OFFSET1_GB,
		    BNR_NS_OFFSET1_GB, 1023);
	ISP_WO_BITS(bnr, REG_ISP_BNR_T, NS_OFFSET1_GR,
		    BNR_NS_OFFSET1_GR, 1023);
	ISP_WO_BITS(bnr, REG_ISP_BNR_T, NS_OFFSET1_R,
		    BNR_NS_OFFSET1_R, 1023);
	ISP_WO_BITS(bnr, REG_ISP_BNR_T, NS_LUMA_TH_B,
		    BNR_NS_LUMA_TH_B, 160);
	ISP_WO_BITS(bnr, REG_ISP_BNR_T, NS_LUMA_TH_GB,
		    BNR_NS_LUMA_TH_GB, 160);
	ISP_WO_BITS(bnr, REG_ISP_BNR_T, NS_LUMA_TH_GR,
		    BNR_NS_LUMA_TH_GR, 160);
	ISP_WO_BITS(bnr, REG_ISP_BNR_T, NS_LUMA_TH_R,
		    BNR_NS_LUMA_TH_R, 160);
	ISP_WO_BITS(bnr, REG_ISP_BNR_T, NS_GAIN, BNR_NS_GAIN, 0);

	ISP_WO_BITS(bnr, REG_ISP_BNR_T, LSC_EN, BNR_LSC_EN, 0);
	ISP_WO_BITS(bnr, REG_ISP_BNR_T, NORM_FACTOR, BNR_NORM_FACTOR, 3322);
	ISP_WO_BITS(bnr, REG_ISP_BNR_T, LSC_STRENTH, BNR_LSC_STRENTH, 128);
}

int ispblk_bnr_config(struct isp_ctx *ctx, enum ISP_BNR_OUT out_sel,
		      bool lsc_en, uint8_t ns_gain, uint8_t str)
{
	uint64_t bnr = ctx->phys_regs[ISP_BLK_ID_BNR];

	ISP_WO_BITS(bnr, REG_ISP_BNR_T, OUT_SEL, BNR_OUT_SEL, out_sel);
	ISP_WO_BITS(bnr, REG_ISP_BNR_T, NS_GAIN, BNR_NS_GAIN, ns_gain);
	ISP_WO_BITS(bnr, REG_ISP_BNR_T, STRENGTH_MODE,
		    BNR_STRENGTH_MODE, str);
	ISP_WO_BITS(bnr, REG_ISP_BNR_T, LSC_EN, BNR_LSC_EN, lsc_en);

	ISP_WO_BITS(bnr, REG_ISP_BNR_T, BAYER_TYPE, BNR_BAYER_TYPE,
		    ctx->rgb_color_mode);
	ISP_WO_BITS(bnr, REG_ISP_BNR_T, HSIZE, BNR_HSIZE, ctx->img_width);
	ISP_WO_BITS(bnr, REG_ISP_BNR_T, VSIZE, BNR_VSIZE, ctx->img_height);
	if (ctx->is_tile) {
		uint32_t center_x = ctx->tile_cfg.r_in.end + 1;

		ISP_WO_BITS(bnr, REG_ISP_BNR_T, X_CENTER,
			    BNR_X_CENTER, center_x >> 1);
	} else {
		ISP_WO_BITS(bnr, REG_ISP_BNR_T, X_CENTER,
			    BNR_X_CENTER, ctx->img_width >> 1);
	}
	ISP_WO_BITS(bnr, REG_ISP_BNR_T, Y_CENTER,
		    BNR_Y_CENTER, ctx->img_height >> 1);

	return 0;
}

/****************************************************************************
 *	RGB TOP
 ****************************************************************************/
int ispblk_rgb_config(struct isp_ctx *ctx)
{
	uint64_t rgbtop = ctx->phys_regs[ISP_BLK_ID_RGBTOP];

	ISP_WR_BITS(rgbtop, REG_ISP_RGB_T, REG_0, COLOR_MODE,
		    ctx->rgb_color_mode);

	return 0;
}

void ispblk_rgb_prob_set_pos(struct isp_ctx *ctx, enum ISP_RGB_PROB_OUT out,
			     uint16_t x, uint16_t y)
{
	uint64_t rgbtop = ctx->phys_regs[ISP_BLK_ID_RGBTOP];

	ISP_WR_BITS(rgbtop, REG_ISP_RGB_T, REG_4, PROB_OUT_SEL, out);
	ISP_WR_REG(rgbtop, REG_ISP_RGB_T, REG_5, (x << 16) | y);
}

void ispblk_rgb_prob_get_values(struct isp_ctx *ctx, uint16_t *r, uint16_t *g,
				uint16_t *b)
{
	uint64_t rgbtop = ctx->phys_regs[ISP_BLK_ID_RGBTOP];
	union REG_ISP_RGB_6 reg6;
	union REG_ISP_RGB_7 reg7;

	reg6.raw = ISP_RD_REG(rgbtop, REG_ISP_RGB_T, REG_6);
	reg7.raw = ISP_RD_REG(rgbtop, REG_ISP_RGB_T, REG_7);
	*r = reg6.bits.PROB_R;
	*g = reg6.bits.PROB_G;
	*b = reg7.bits.PROB_B;
}

static void _cfa_init(struct isp_ctx *ctx)
{
	uint64_t cfa = ctx->phys_regs[ISP_BLK_ID_CFA];

	// field update or reg update?
	ISP_WR_REG(cfa, REG_ISP_CFA_T, REG_3, 0x019001e0);
	ISP_WR_REG(cfa, REG_ISP_CFA_T, REG_4, 0x00280028);
	ISP_WR_BITS(cfa, REG_ISP_CFA_T, REG_4_1, CFA_EDGE_TOL, 0x80);
	ISP_WR_BITS(cfa, REG_ISP_CFA_T, REG_5, CFA_GHP_THD, 0xe00);

	//fcr
	ISP_WR_REG(cfa, REG_ISP_CFA_T, REG_6, 0x06400190);
	ISP_WR_REG(cfa, REG_ISP_CFA_T, REG_7, 0x04000300);
	ISP_WR_REG(cfa, REG_ISP_CFA_T, REG_8, 0x0bb803e8);
	ISP_WR_REG(cfa, REG_ISP_CFA_T, REG_9, 0x03ff0010);

	//ghp lut
	ISP_WR_REG(cfa, REG_ISP_CFA_T, GHP_LUT_0, 0x08080808);
	ISP_WR_REG(cfa, REG_ISP_CFA_T, GHP_LUT_1, 0x0a0a0a0a);
	ISP_WR_REG(cfa, REG_ISP_CFA_T, GHP_LUT_2, 0x0c0c0c0c);
	ISP_WR_REG(cfa, REG_ISP_CFA_T, GHP_LUT_3, 0x0e0e0e0e);
	ISP_WR_REG(cfa, REG_ISP_CFA_T, GHP_LUT_4, 0x10101010);
	ISP_WR_REG(cfa, REG_ISP_CFA_T, GHP_LUT_5, 0x14141414);
	ISP_WR_REG(cfa, REG_ISP_CFA_T, GHP_LUT_6, 0x18181818);
	ISP_WR_REG(cfa, REG_ISP_CFA_T, GHP_LUT_7, 0x1f1f1c1c);

	//moire
	ISP_WR_REG(cfa, REG_ISP_CFA_T, REG_10, 0x40ff);
	ISP_WR_REG(cfa, REG_ISP_CFA_T, REG_11, 0);
	ISP_WR_REG(cfa, REG_ISP_CFA_T, REG_12, 0x3a021c);
	ISP_WR_REG(cfa, REG_ISP_CFA_T, REG_13, 0x60ff00);
	ISP_WR_REG(cfa, REG_ISP_CFA_T, REG_14, 0xff000022);
	ISP_WR_REG(cfa, REG_ISP_CFA_T, REG_15, 0xff00);
	ISP_WR_REG(cfa, REG_ISP_CFA_T, REG_16, 0x300150);
}

int ispblk_cfa_config(struct isp_ctx *ctx)
{
	uint64_t cfa = ctx->phys_regs[ISP_BLK_ID_CFA];

	ISP_WR_BITS(cfa, REG_ISP_CFA_T, REG_0, CFA_SHDW_SEL, 1);
	ISP_WR_BITS(cfa, REG_ISP_CFA_T, REG_0, CFA_ENABLE, 1);
	ISP_WR_BITS(cfa, REG_ISP_CFA_T, REG_0, CFA_FCR_ENABLE, 1);
	ISP_WR_BITS(cfa, REG_ISP_CFA_T, REG_0, CFA_MOIRE_ENABLE, 1);

	ISP_WR_REG(cfa, REG_ISP_CFA_T, REG_2,
		   ((ctx->img_height-1)<<16) | (ctx->img_width-1));

	return 0;
}

static void _rgbee_init(struct isp_ctx *ctx)
{
	uint8_t ac_lut[33] = {
		128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128,
		128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128,
		128, 128, 128, 128, 128, 128, 128, 128, 128,
	};
	uint8_t edge_lut[33] = {
		16, 48, 48, 64, 64, 64, 64, 56, 48, 32, 32, 32,
		32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32,
		32, 32, 32, 32, 32, 32, 32, 32, 32,
	};
	uint16_t np_lut[33] = {
		16, 16, 24, 32, 32, 32, 32, 32, 32, 32, 32, 32,
		32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32,
		32, 32, 32, 32, 32, 32, 32, 32, 32,
	};
	uint64_t rgbee = ctx->phys_regs[ISP_BLK_ID_RGBEE];
	uint64_t addr = 0;
	uint32_t val = 0;

	addr = rgbee + _OFST(REG_ISP_RGBEE_T, AC_LUT_0);
	REG_ARRAY_UPDATE4(addr, ac_lut);

	addr = rgbee + _OFST(REG_ISP_RGBEE_T, EDGE_LUT_0);
	REG_ARRAY_UPDATE4(addr, edge_lut);

	addr = rgbee + _OFST(REG_ISP_RGBEE_T, NP_LUT_0);
	REG_ARRAY_UPDATE2(addr, np_lut);
}

int ispblk_rgbee_config(struct isp_ctx *ctx, bool en, uint16_t osgain,
			uint16_t usgain)
{
	uint64_t rgbee = ctx->phys_regs[ISP_BLK_ID_RGBEE];

	ISP_WR_BITS(rgbee, REG_ISP_RGBEE_T, REG_0, RGBEE_ENABLE, en);
	ISP_WR_BITS(rgbee, REG_ISP_RGBEE_T, REG_2, RGBEE_OSGAIN,
		    osgain);
	ISP_WR_BITS(rgbee, REG_ISP_RGBEE_T, REG_2, RGBEE_USGAIN,
		    usgain);
	ISP_WR_BITS(rgbee, REG_ISP_RGBEE_T, REG_4, IMG_WD,
		    ctx->img_width-1);
	ISP_WR_BITS(rgbee, REG_ISP_RGBEE_T, REG_4, IMG_HT,
		    ctx->img_height-1);

	return 0;
}

int ispblk_gamma_config(struct isp_ctx *ctx, uint8_t sel, uint16_t *data)
{
	uint64_t gamma = ctx->phys_regs[ISP_BLK_ID_GAMMA];
	uint16_t i;
	union REG_ISP_GAMMA_PROG_DATA reg_data;

	ISP_WR_BITS(gamma, REG_ISP_GAMMA_T, GAMMA_PROG_ST_ADDR,
		    GAMMA_ST_ADDR, 0);
	ISP_WR_BITS(gamma, REG_ISP_GAMMA_T, GAMMA_PROG_ST_ADDR,
		    GAMMA_ST_W, 1);

	ISP_WR_BITS(gamma, REG_ISP_GAMMA_T, GAMMA_PROG_CTRL,
		    GAMMA_WSEL, sel);
	ISP_WR_REG(gamma, REG_ISP_GAMMA_T, GAMMA_PROG_MAX, data[256]);
	for (i = 0; i < 256; i += 2) {
		reg_data.raw = 0;
		reg_data.bits.GAMMA_DATA_E = data[i];
		reg_data.bits.GAMMA_DATA_O = data[i + 1];
		reg_data.bits.GAMMA_W = 1;
		ISP_WR_REG(gamma, REG_ISP_GAMMA_T, GAMMA_PROG_DATA,
			   reg_data.raw);
	}

	ISP_WR_BITS(gamma, REG_ISP_GAMMA_T, GAMMA_PROG_CTRL,
		    GAMMA_RSEL, sel);

	return 0;
}

int ispblk_gamma_enable(struct isp_ctx *ctx, bool enable, uint8_t sel)
{
	uint64_t gamma = ctx->phys_regs[ISP_BLK_ID_GAMMA];

	ISP_WR_BITS(gamma, REG_ISP_GAMMA_T, GAMMA_CTRL, GAMMA_ENABLE,
		    enable);
	ISP_WR_BITS(gamma, REG_ISP_GAMMA_T, GAMMA_PROG_CTRL,
		    GAMMA_RSEL, sel);
	return 0;
}

/**
 * ispblk_rgbdither_config - setup rgb dither.
 *
 * @param ctx: global settings
 * @param en: rgb dither enable
 * @param mod_en: 0: mod 32, 1: mod 29
 * @param histidx_en: refer to previous dither number enable
 * @param fmnum_en: refer to frame index enable
 */
int ispblk_rgbdither_config(struct isp_ctx *ctx, bool en, bool mod_en,
			    bool histidx_en, bool fmnum_en)
{
	uint64_t rgbdither = ctx->phys_regs[ISP_BLK_ID_RGBDITHER];
	union REG_ISP_RGB_DITHER_RGB_DITHER reg;

	reg.raw = 0;
	reg.bits.RGB_DITHER_ENABLE = en;
	reg.bits.RGB_DITHER_MOD_EN = mod_en;
	reg.bits.RGB_DITHER_HISTIDX_EN = histidx_en;
	reg.bits.RGB_DITHER_FMNUM_EN = fmnum_en;
	reg.bits.RGB_DITHER_SHDW_SEL = 1;
	reg.bits.CROP_WIDTHM1 = ctx->img_width - 1;
	reg.bits.CROP_HEIGHTM1 = ctx->img_height - 1;

	ISP_WR_REG(rgbdither, REG_ISP_RGB_DITHER_T, RGB_DITHER, reg.raw);

	return 0;
}

int ispblk_ccm_config(struct isp_ctx *ctx, bool en, struct isp_ccm_cfg *cfg)
{
	uint64_t ccm = ctx->phys_regs[ISP_BLK_ID_CCM];

	ISP_WR_BITS(ccm, REG_ISP_CCM_T, CCM_CTRL, CCM_SHDW_SEL, 1);
	ISP_WR_BITS(ccm, REG_ISP_CCM_T, CCM_CTRL, CCM_ENABLE, en);
	ISP_WR_REG(ccm, REG_ISP_CCM_T, CCM_00, cfg->coef[0][0]);
	ISP_WR_REG(ccm, REG_ISP_CCM_T, CCM_01, cfg->coef[0][1]);
	ISP_WR_REG(ccm, REG_ISP_CCM_T, CCM_02, cfg->coef[0][2]);
	ISP_WR_REG(ccm, REG_ISP_CCM_T, CCM_10, cfg->coef[1][0]);
	ISP_WR_REG(ccm, REG_ISP_CCM_T, CCM_11, cfg->coef[1][1]);
	ISP_WR_REG(ccm, REG_ISP_CCM_T, CCM_12, cfg->coef[1][2]);
	ISP_WR_REG(ccm, REG_ISP_CCM_T, CCM_20, cfg->coef[2][0]);
	ISP_WR_REG(ccm, REG_ISP_CCM_T, CCM_21, cfg->coef[2][1]);
	ISP_WR_REG(ccm, REG_ISP_CCM_T, CCM_22, cfg->coef[2][2]);
	return 0;
}

static void _dhz_init(struct isp_ctx *ctx)
{
	uint64_t dhz = ctx->phys_regs[ISP_BLK_ID_DHZ];
	union REG_ISP_DHZ_DEHAZE_PARA reg_para;
	union REG_ISP_DHZ_1 reg_1;
	union REG_ISP_DHZ_2 reg_2;
	union REG_ISP_DHZ_3 reg_3;

	// dehaze strength
	reg_para.bits.DEHAZE_W = 30;
	// threshold for smooth/edge
	reg_para.bits.DEHAZE_TH_SMOOTH = 200;
	ISP_WR_REG(dhz, REG_ISP_DHZ_T, DEHAZE_PARA, reg_para.raw);

	reg_1.bits.DEHAZE_CUM_TH = 1024;
	reg_1.bits.DEHAZE_HIST_TH = 512;
	ISP_WR_REG(dhz, REG_ISP_DHZ_T, REG_1, reg_1.raw);

	reg_2.bits.DEHAZE_SW_DC_TH = 1000;
	reg_2.bits.DEHAZE_SW_AGLOBAL = 3840;
	reg_2.bits.DEHAZE_SW_DC_AGLOBAL_TRIG = 0;
	ISP_WR_REG(dhz, REG_ISP_DHZ_T, REG_2, reg_2.raw);
	ISP_WR_BITS(dhz, REG_ISP_DHZ_T, REG_2,
		    DEHAZE_SW_DC_AGLOBAL_TRIG, 1);

	reg_3.bits.DEHAZE_TMAP_MIN = 819;
	reg_3.bits.DEHAZE_TMAP_MAX = 8191;
	ISP_WR_REG(dhz, REG_ISP_DHZ_T, REG_3, reg_3.raw);
}

/**
 * ispblk_dhz_config - setup dehaze.
 *
 * @param ctx: global settings
 * @param en: dehaze enable
 * @param cfg: dhz cfg
 */
int ispblk_dhz_config(struct isp_ctx *ctx, bool en, struct isp_dhz_cfg *cfg)
{
	uint64_t dhz = ctx->phys_regs[ISP_BLK_ID_DHZ];

	ISP_WR_BITS(dhz, REG_ISP_DHZ_T, DHZ_BYPASS, DEHAZE_ENABLE, en);
	ISP_WR_BITS(dhz, REG_ISP_DHZ_T, DEHAZE_PARA, DEHAZE_W, cfg->strength);
	ISP_WR_BITS(dhz, REG_ISP_DHZ_T, DEHAZE_PARA, DEHAZE_TH_SMOOTH,
		    cfg->th_smooth);
	ISP_WR_BITS(dhz, REG_ISP_DHZ_T, REG_4, IMG_WD,
		    ctx->img_width-1);
	ISP_WR_BITS(dhz, REG_ISP_DHZ_T, REG_4, IMG_HT,
		    ctx->img_height-1);
	// reg_dehaze_cum_th <= image size x 0.1%
	ISP_WR_BITS(dhz, REG_ISP_DHZ_T, REG_1, DEHAZE_CUM_TH, cfg->cum_th);
	ISP_WR_BITS(dhz, REG_ISP_DHZ_T, REG_1, DEHAZE_HIST_TH, cfg->hist_th);
	ISP_WR_BITS(dhz, REG_ISP_DHZ_T, REG_3, DEHAZE_TMAP_MIN, cfg->tmap_min);
	ISP_WR_BITS(dhz, REG_ISP_DHZ_T, REG_3, DEHAZE_TMAP_MAX, cfg->tmap_max);

	ISP_WR_BITS(dhz, REG_ISP_DHZ_T, REG_2, DEHAZE_SW_DC_TH, cfg->sw_dc_th);
	ISP_WR_BITS(dhz, REG_ISP_DHZ_T, REG_2,
		    DEHAZE_SW_AGLOBAL, cfg->sw_aglobal);
	ISP_WR_BITS(dhz, REG_ISP_DHZ_T, REG_2,
		    DEHAZE_SW_DC_AGLOBAL_TRIG, cfg->sw_dc_trig);

	if (ctx->is_tile) {
		ISP_WR_BITS(dhz, REG_ISP_DHZ_T, REG_5, TILE_NM, 1);
		if (ctx->is_work_on_r_tile) {
			ISP_WR_BITS(dhz, REG_ISP_DHZ_T, REG_5, FMT_ST,
				    ctx->tile_cfg.r_out.start -
				    ctx->tile_cfg.r_in.start);
			ISP_WR_BITS(dhz, REG_ISP_DHZ_T, REG_5, FMT_END,
				    ctx->tile_cfg.r_out.end -
				    ctx->tile_cfg.r_in.start);
		} else {
			ISP_WR_BITS(dhz, REG_ISP_DHZ_T, REG_5, FMT_ST,
				    ctx->tile_cfg.l_out.start);
			ISP_WR_BITS(dhz, REG_ISP_DHZ_T, REG_5, FMT_END,
				    ctx->tile_cfg.l_out.end);
		}
	} else {
		ISP_WR_BITS(dhz, REG_ISP_DHZ_T, REG_5, TILE_NM, 0);
	}

	return 0;
}

int ispblk_hsv_config(struct isp_ctx *ctx, uint8_t sel, uint8_t type,
		      uint16_t *lut)
{
	uint64_t hsv = ctx->phys_regs[ISP_BLK_ID_HSV];
	uint16_t i, count;
	uint64_t addr = 0;
	uint32_t val = 0;

	count = (type == 3) ? 0x200 : 0x300;
	if (type == 0) {
		ISP_WR_BITS(hsv, REG_ISP_HSV_T, HSV_0,
			    HSV_SGAIN_LUT_LAST_VAL, lut[count+1]);
		addr = hsv + _OFST(REG_ISP_HSV_T, HSV_3);
	} else if (type == 1) {
		ISP_WR_BITS(hsv, REG_ISP_HSV_T, HSV_4,
			    HSV_VGAIN_LUT_LAST_VAL, lut[count+1]);
		addr = hsv + _OFST(REG_ISP_HSV_T, HSV_6);
	} else if (type == 2) {
		ISP_WR_BITS(hsv, REG_ISP_HSV_T, HSV_0,
			    HSV_H_LUT_LAST_VAL, lut[count+1]);
		addr = hsv + _OFST(REG_ISP_HSV_T, HSV_2);
	} else {
		ISP_WR_BITS(hsv, REG_ISP_HSV_T, HSV_4,
			    HSV_S_LUT_LAST_VAL, lut[count+1]);
		addr = hsv + _OFST(REG_ISP_HSV_T, HSV_1);
	}

	ISP_WR_BITS(hsv, REG_ISP_HSV_T, HSV_5, HSV_LUT_W_SEL, sel);
	for (i = 0; i < count; i += 2) {
		val = BIT(31);
		val |= lut[i];
		val |= (lut[i+1] << 16);
		_reg_write(addr, val);
	}

	return 0;
}

int ispblk_hsv_enable(struct isp_ctx *ctx, bool en, uint8_t sel, bool hsgain_en,
		      bool hvgain_en, bool htune_en, bool stune_en)
{
	uint64_t hsv = ctx->phys_regs[ISP_BLK_ID_HSV];
	union REG_ISP_HSV_0 reg_0;

	ISP_WR_BITS(hsv, REG_ISP_HSV_T, HSV_5, HSV_LUT_R_SEL, sel);

	reg_0.raw = ISP_RD_REG(hsv, REG_ISP_HSV_T, HSV_0);
	reg_0.bits.HSV_ENABLE = en;
	reg_0.bits.HSV_HSGAIN_ENABLE = hsgain_en;
	reg_0.bits.HSV_HVGAIN_ENABLE = hvgain_en;
	reg_0.bits.HSV_HTUNE_ENABLE = htune_en;
	reg_0.bits.HSV_STUNE_ENABLE = stune_en;
	ISP_WR_REG(hsv, REG_ISP_HSV_T, HSV_0, reg_0.raw);

	return 0;
}

int ispblk_dpc_config(struct isp_ctx *ctx, enum ISP_RAW_PATH path,
		      union REG_ISP_DPC_2 reg2)
{
	uint64_t dpc = (path == ISP_RAW_PATH_LE)
		       ? ctx->phys_regs[ISP_BLK_ID_DPC0]
		       : ctx->phys_regs[ISP_BLK_ID_DPC1];

	ISP_WR_BITS(dpc, REG_ISP_DPC_T, DPC_WINDOW, IMG_WD,
		    ctx->img_width-1);

	ISP_WR_BITS(dpc, REG_ISP_DPC_T, DPC_WINDOW, IMG_HT,
		    ctx->img_height-1);

	ISP_WR_BITS(dpc, REG_ISP_DPC_T, DPC_0, COLOR_MODE,
		    ctx->rgb_color_mode);

	ISP_WR_REG(dpc, REG_ISP_DPC_T, DPC_2, reg2.raw);

	return 0;
}

/**
 * ispblk_dpc_set_static - set defect pixels for static dpc.
 *
 * @param ctx: global settings
 * @param offset: mem-offset for 4k tile
 * @param bps: array of defect pixels. [23:12]-row, [11:0]-col.
 * @param count: number of defect pixels.
 */
int ispblk_dpc_set_static(struct isp_ctx *ctx, enum ISP_RAW_PATH path,
			     uint16_t offset, uint32_t *bps, uint8_t count)
{
	uint64_t dpc = (path == ISP_RAW_PATH_LE)
		       ? ctx->phys_regs[ISP_BLK_ID_DPC0]
		       : ctx->phys_regs[ISP_BLK_ID_DPC1];
	uint8_t i = 0;

	ISP_WR_REG(dpc, REG_ISP_DPC_T, DPC_MEM_ST_ADDR, 0x80000000 | offset);

	for (i = 0; i < count; ++i)
		ISP_WR_REG(dpc, REG_ISP_DPC_T, DPC_MEM_W0,
			   0x80000000 | *(bps+i));

	// write 4 fff-fff to end
	ISP_WR_REG(dpc, REG_ISP_DPC_T, DPC_MEM_W0, 0x80ffffff);
	ISP_WR_REG(dpc, REG_ISP_DPC_T, DPC_MEM_W0, 0x80ffffff);
	ISP_WR_REG(dpc, REG_ISP_DPC_T, DPC_MEM_W0, 0x80ffffff);
	ISP_WR_REG(dpc, REG_ISP_DPC_T, DPC_MEM_W0, 0x80ffffff);

	return 0;
}

int ispblk_csc_config(struct isp_ctx *ctx)
{
	uint64_t csc = ctx->phys_regs[ISP_BLK_ID_R2Y4];
	uint8_t enable = !(ctx->is_yuv_sensor);

	ISP_WR_BITS(csc, REG_ISP_CSC_T, REG_0, CSC_ENABLE, enable);

	return 0;
}

/****************************************************************************
 *	YUV TOP
 ****************************************************************************/
/**
 * ispblk_yuvdither_config - setup yuv dither.
 *
 * @param ctx: global settings
 * @param sel: y(0)/uv(1)
 * @param en: dither enable
 * @param mod_en: 0: mod 32, 1: mod 29
 * @param histidx_en: refer to previous dither number enable
 * @param fmnum_en: refer to frame index enable
 */
int ispblk_yuvdither_config(struct isp_ctx *ctx, uint8_t sel, bool en,
			    bool mod_en, bool histidx_en, bool fmnum_en)
{
	uint64_t dither = ctx->phys_regs[ISP_BLK_ID_UVDITHER];

	if (sel == 0) {
		union REG_ISP_YUV_DITHER_Y_DITHER reg;

		reg.raw = 0;
		reg.bits.Y_DITHER_EN = en;
		reg.bits.Y_DITHER_MOD_EN = mod_en;
		reg.bits.Y_DITHER_HISTIDX_EN = histidx_en;
		reg.bits.Y_DITHER_FMNUM_EN = fmnum_en;
		reg.bits.Y_DITHER_SHDW_SEL = 1;
		reg.bits.Y_DITHER_WIDTHM1 = ctx->img_width - 1;
		reg.bits.Y_DITHER_HEIGHTM1 = ctx->img_height - 1;

		ISP_WR_REG(dither, REG_ISP_YUV_DITHER_T, Y_DITHER, reg.raw);
	} else if (sel == 1) {
		union REG_ISP_YUV_DITHER_UV_DITHER reg;

		reg.raw = 0;
		reg.bits.UV_DITHER_EN = en;
		reg.bits.UV_DITHER_MOD_EN = mod_en;
		reg.bits.UV_DITHER_HISTIDX_EN = histidx_en;
		reg.bits.UV_DITHER_FMNUM_EN = fmnum_en;
		reg.bits.UV_DITHER_WIDTHM1 = (ctx->img_width >> 1) - 1;
		reg.bits.UV_DITHER_HEIGHTM1 = (ctx->img_height >> 1) - 1;

		ISP_WR_REG(dither, REG_ISP_YUV_DITHER_T, UV_DITHER, reg.raw);
	}

	return 0;
}

int ispblk_444_422_config(struct isp_ctx *ctx)
{
	uint64_t y42 = ctx->phys_regs[ISP_BLK_ID_444422];

	ISP_WR_BITS(y42, REG_ISP_444_422_T, REG_4, REG_422_444,
		    ctx->is_yuv_sensor);
	ISP_WR_REG(y42, REG_ISP_444_422_T, REG_6, ctx->img_width - 1);
	ISP_WR_REG(y42, REG_ISP_444_422_T, REG_7, ctx->img_height - 1);
	ISP_WR_BITS(y42, REG_ISP_444_422_T, REG_8, DMA_ENABLE,
		    (ctx->is_3dnr_on) ? 0x3f : 0);
	ISP_WR_BITS(y42, REG_ISP_444_422_T, REG_5, DMA_BYPASS,
		    (ctx->is_3dnr_on) ? 0 : 1);
	ISP_WR_BITS(y42, REG_ISP_444_422_T, REG_9, SENSOR_422_IN,
		    ctx->is_yuv_sensor);

	ISP_WR_BITS(y42, REG_ISP_444_422_T, REG_8, TDNR_DEBUG_SEL, (ctx->is_tile) ? 0x4 : 0x0);

	return 0;
}

static void _ynr_init(struct isp_ctx *ctx)
{
	uint64_t ynr = ctx->phys_regs[ISP_BLK_ID_YNR];

	// depth = 8
	uint8_t intensity_sel[] = {
		5, 7, 20, 23, 14, 9, 31, 31
	};
	// depth =64
	uint8_t weight_lut[] = {
		31, 16, 8,  4,  2,  1,  1,  0,  0,  0,  0,  0,  0,  0,  0,  0,
		0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
		0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
		0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	};
	// depth = 6
	uint8_t ns0_luma_th[] = {
		22, 33, 55, 99, 139, 181,
	};
	// depth = 5
	uint16_t ns0_slope[] = {
		279, 209, 81, -12, -121,
	};
	// depth = 6
	uint8_t ns0_offset[] = {
		25, 31, 40, 47, 46, 36
	};
	// depth = 6
	uint8_t ns1_luma_th[] = {
		22, 33, 55, 99, 139, 181,
	};
	// depth = 5
	uint16_t ns1_slope[] = {
		93, 46, 23, 0, -36,
	};
	// depth = 6
	uint8_t ns1_offset[] = {
		7, 9, 11, 13, 13, 10
	};
	uint16_t i = 0;

	ISP_WO_BITS(ynr, REG_ISP_YNR_T, INDEX_CLR, YNR_INDEX_CLR, 1);
	for (i = 0; i < ARRAY_SIZE(intensity_sel); ++i) {
		ISP_WR_REG(ynr, REG_ISP_YNR_T, INTENSITY_SEL,
			   intensity_sel[i]);
	}
	for (i = 0; i < ARRAY_SIZE(weight_lut); ++i) {
		ISP_WR_REG(ynr, REG_ISP_YNR_T, WEIGHT_LUT,
			   weight_lut[i]);
	}
	for (i = 0; i < ARRAY_SIZE(ns0_luma_th); ++i) {
		ISP_WR_REG(ynr, REG_ISP_YNR_T, NS0_LUMA_TH,
			   ns0_luma_th[i]);
	}
	for (i = 0; i < ARRAY_SIZE(ns0_slope); ++i) {
		ISP_WR_REG(ynr, REG_ISP_YNR_T, NS0_SLOPE,
			   ns0_slope[i]);
	}
	for (i = 0; i < ARRAY_SIZE(ns0_offset); ++i) {
		ISP_WR_REG(ynr, REG_ISP_YNR_T, NS0_OFFSET,
			   ns0_offset[i]);
	}
	for (i = 0; i < ARRAY_SIZE(ns1_luma_th); ++i) {
		ISP_WR_REG(ynr, REG_ISP_YNR_T, NS1_LUMA_TH,
			   ns1_luma_th[i]);
	}
	for (i = 0; i < ARRAY_SIZE(ns1_slope); ++i) {
		ISP_WR_REG(ynr, REG_ISP_YNR_T, NS1_SLOPE,
			   ns1_slope[i]);
	}
	for (i = 0; i < ARRAY_SIZE(ns1_offset); ++i) {
		ISP_WR_REG(ynr, REG_ISP_YNR_T, NS1_OFFSET,
			   ns1_offset[i]);
	}

	ISP_WR_REG(ynr, REG_ISP_YNR_T, OUT_SEL, ISP_YNR_OUT_BYPASS);
	ISP_WR_REG(ynr, REG_ISP_YNR_T, STRENGTH_MODE, 0);
	ISP_WR_REG(ynr, REG_ISP_YNR_T, WEIGHT_INTRA_0, 6);
	ISP_WR_REG(ynr, REG_ISP_YNR_T, WEIGHT_INTRA_1, 6);
	ISP_WR_REG(ynr, REG_ISP_YNR_T, WEIGHT_INTRA_2, 6);
	ISP_WR_REG(ynr, REG_ISP_YNR_T, NEIGHBOR_MAX, 1);
	ISP_WR_REG(ynr, REG_ISP_YNR_T, RES_K_SMOOTH, 67);
	ISP_WR_REG(ynr, REG_ISP_YNR_T, RES_K_TEXTURE, 88);
	ISP_WR_REG(ynr, REG_ISP_YNR_T, VAR_TH, 32);
	ISP_WR_REG(ynr, REG_ISP_YNR_T, WEIGHT_SM, 29);
	ISP_WR_REG(ynr, REG_ISP_YNR_T, WEIGHT_V, 23);
	ISP_WR_REG(ynr, REG_ISP_YNR_T, WEIGHT_H, 20);
	ISP_WR_REG(ynr, REG_ISP_YNR_T, WEIGHT_D45, 15);
	ISP_WR_REG(ynr, REG_ISP_YNR_T, WEIGHT_D135, 7);
	ISP_WR_REG(ynr, REG_ISP_YNR_T, WEIGHT_NORM_1, 7);
	ISP_WR_REG(ynr, REG_ISP_YNR_T, WEIGHT_NORM_2, 5);

	ISP_WR_REG(ynr, REG_ISP_YNR_T, NS_GAIN, 16);
	ISP_WR_REG(ynr, REG_ISP_YNR_T, MOTION_NS_TH, 7);
	ISP_WR_REG(ynr, REG_ISP_YNR_T, MOTION_POS_GAIN, 4);
	ISP_WR_REG(ynr, REG_ISP_YNR_T, MOTION_NEG_GAIN, 2);
	ISP_WR_REG(ynr, REG_ISP_YNR_T, ALPHA_GAIN, 256);
}

int ispblk_ynr_config(struct isp_ctx *ctx, enum ISP_YNR_OUT out_sel,
		      uint8_t ns_gain, uint8_t str)
{
	uint64_t ynr = ctx->phys_regs[ISP_BLK_ID_YNR];

	ISP_WR_REG(ynr, REG_ISP_YNR_T, OUT_SEL, out_sel);
	ISP_WR_REG(ynr, REG_ISP_YNR_T, NS_GAIN, ns_gain);
	ISP_WR_REG(ynr, REG_ISP_YNR_T, STRENGTH_MODE, str);

	return 0;
}

static void _cnr_init(struct isp_ctx *ctx)
{
	uint64_t cnr = ctx->phys_regs[ISP_BLK_ID_CNR];
	union REG_ISP_CNR_00 reg_00;
	union REG_ISP_CNR_01 reg_01;
	union REG_ISP_CNR_02 reg_02;
	union REG_ISP_CNR_03 reg_03;
	union REG_ISP_CNR_04 reg_04;
	union REG_ISP_CNR_05 reg_05;
	union REG_ISP_CNR_06 reg_06;
	union REG_ISP_CNR_07 reg_07;
	union REG_ISP_CNR_08 reg_08;
	union REG_ISP_CNR_09 reg_09;
	union REG_ISP_CNR_10 reg_10;
	union REG_ISP_CNR_11 reg_11;
	union REG_ISP_CNR_12 reg_12;

	reg_00.raw = 0;
	reg_00.bits.CNR_ENABLE = 0;
	reg_00.bits.PFC_ENABLE = 0;
	reg_00.bits.CNR_SWIN_ROWS = 7;
	reg_00.bits.CNR_SWIN_COLS = 7;
	reg_00.bits.CNR_DIFF_SHIFT_VAL = 0;
	reg_00.bits.CNR_RATIO = 220;
	ISP_WR_REG(cnr, REG_ISP_CNR_T, CNR_00, reg_00.raw);

	reg_01.raw = 0;
	reg_01.bits.CNR_STRENGTH_MODE = 32;
	reg_01.bits.CNR_FUSION_INTENSITY_WEIGHT = 4;
	reg_01.bits.CNR_WEIGHT_INTER_SEL = 0;
	reg_01.bits.CNR_VAR_TH = 32;
	reg_01.bits.CNR_FLAG_NEIGHBOR_MAX_WEIGHT = 1;
	reg_01.bits.CNR_SHDW_SEL = 1;
	ISP_WR_REG(cnr, REG_ISP_CNR_T, CNR_01, reg_01.raw);

	reg_02.raw = 0;
	reg_02.bits.CNR_PURPLE_TH = 90;
	reg_02.bits.CNR_CORRECT_STRENGTH = 16;
	reg_02.bits.CNR_DIFF_GAIN = 4;
	ISP_WR_REG(cnr, REG_ISP_CNR_T, CNR_02, reg_02.raw);

	reg_03.raw = 0;
	reg_03.bits.CNR_PURPLE_CR = 176;
	reg_03.bits.CNR_GREEN_CB = 43;
	ISP_WR_REG(cnr, REG_ISP_CNR_T, CNR_03, reg_03.raw);

	reg_04.raw = 0;
	reg_04.bits.CNR_PURPLE_CB = 232;
	reg_04.bits.CNR_GREEN_CR = 21;
	ISP_WR_REG(cnr, REG_ISP_CNR_T, CNR_04, reg_04.raw);

	reg_05.raw = 0;
	reg_05.bits.WEIGHT_LUT_INTER_CNR_00 = 16;
	reg_05.bits.WEIGHT_LUT_INTER_CNR_01 = 16;
	reg_05.bits.WEIGHT_LUT_INTER_CNR_02 = 15;
	reg_05.bits.WEIGHT_LUT_INTER_CNR_03 = 13;
	ISP_WR_REG(cnr, REG_ISP_CNR_T, CNR_05, reg_05.raw);

	reg_06.raw = 0;
	reg_06.bits.WEIGHT_LUT_INTER_CNR_04 = 12;
	reg_06.bits.WEIGHT_LUT_INTER_CNR_05 = 10;
	reg_06.bits.WEIGHT_LUT_INTER_CNR_06 = 8;
	reg_06.bits.WEIGHT_LUT_INTER_CNR_07 = 6;
	ISP_WR_REG(cnr, REG_ISP_CNR_T, CNR_06, reg_06.raw);

	reg_07.raw = 0;
	reg_07.bits.WEIGHT_LUT_INTER_CNR_08 = 4;
	reg_07.bits.WEIGHT_LUT_INTER_CNR_09 = 3;
	reg_07.bits.WEIGHT_LUT_INTER_CNR_10 = 2;
	reg_07.bits.WEIGHT_LUT_INTER_CNR_11 = 1;
	ISP_WR_REG(cnr, REG_ISP_CNR_T, CNR_07, reg_07.raw);

	reg_08.raw = 0;
	reg_08.bits.WEIGHT_LUT_INTER_CNR_12 = 1;
	reg_08.bits.WEIGHT_LUT_INTER_CNR_13 = 1;
	reg_08.bits.WEIGHT_LUT_INTER_CNR_14 = 0;
	reg_08.bits.WEIGHT_LUT_INTER_CNR_15 = 0;
	ISP_WR_REG(cnr, REG_ISP_CNR_T, CNR_08, reg_08.raw);

	reg_09.raw = 0;
	reg_09.bits.CNR_INTENSITY_SEL_0 = 10;
	reg_09.bits.CNR_INTENSITY_SEL_1 = 5;
	ISP_WR_REG(cnr, REG_ISP_CNR_T, CNR_09, reg_09.raw);

	reg_10.raw = 0;
	reg_10.bits.CNR_INTENSITY_SEL_2 = 16;
	reg_10.bits.CNR_INTENSITY_SEL_3 = 16;
	ISP_WR_REG(cnr, REG_ISP_CNR_T, CNR_10, reg_10.raw);

	reg_11.raw = 0;
	reg_11.bits.CNR_INTENSITY_SEL_4 = 12;
	reg_11.bits.CNR_INTENSITY_SEL_5 = 13;
	ISP_WR_REG(cnr, REG_ISP_CNR_T, CNR_11, reg_11.raw);

	reg_12.raw = 0;
	reg_12.bits.CNR_INTENSITY_SEL_6 = 6;
	reg_12.bits.CNR_INTENSITY_SEL_7 = 16;
	ISP_WR_REG(cnr, REG_ISP_CNR_T, CNR_12, reg_12.raw);
}

int ispblk_cnr_config(struct isp_ctx *ctx, bool en, bool pfc_en,
		      uint8_t str_mode)
{
	uint64_t cnr = ctx->phys_regs[ISP_BLK_ID_CNR];

	ISP_WR_BITS(cnr, REG_ISP_CNR_T, CNR_00, CNR_ENABLE, en);
	ISP_WR_BITS(cnr, REG_ISP_CNR_T, CNR_00, PFC_ENABLE, pfc_en);
	ISP_WR_BITS(cnr, REG_ISP_CNR_T, CNR_01, CNR_STRENGTH_MODE, str_mode);

	ISP_WR_BITS(cnr, REG_ISP_CNR_T, CNR_13, CNR_IMG_WIDTHM1,
		    (ctx->img_width >> 1) - 1);
	ISP_WR_BITS(cnr, REG_ISP_CNR_T, CNR_13, CNR_IMG_HEIGHTM1,
		    (ctx->img_height >> 1) - 1);

	return 0;
}

static void _ee_init(struct isp_ctx *ctx)
{
	uint64_t ee = ctx->phys_regs[ISP_BLK_ID_EE];
	// filters are 2's complement
	uint16_t dgr4_filter000[] = {
		0, -1, -2, 0, -2, -4, 0, 6, 12,
	};
	uint16_t dgr4_filter045[] = {
		-2, -4, -3, 0, 0, -4, 4, 6, 12,
	};
	uint16_t dgr4_filter090[] = {
		0, 0, 0, -1, -2, 6, -2, -4, 12,
	};
	uint16_t dgr4_filter135[] = {
		0, 0, -3, -4, -2, 6, 4, -4, 12,
	};
	uint16_t dgr4_filternod[] = {
		-2, -4, -4, -2, -2, 40,
	};
	uint16_t luma_coring_lut[] = {
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0,
	};
	uint8_t luma_shtctrl_lut[] = {
		32, 32, 32, 32, 32, 32, 32, 32,
		32, 32, 32, 32, 32, 32, 32, 32,
		32, 32, 32, 32, 32, 32, 32, 32,
		32, 32, 32, 32, 32, 32, 32, 32, 32,
	};
	uint8_t delta_shtctrl_lut[] = {
		0,  0,  0,  0,  4,  8,  12, 16,
		20, 24, 28, 32, 32, 32, 32, 32,
		32, 32, 32, 32, 32, 32, 32, 32,
		32, 32, 32, 32, 32, 32, 32, 32, 32,
	};
	uint8_t luma_adptctrl_lut[] = {
		32, 32, 32, 32, 32, 32, 32, 32,
		32, 32, 32, 32, 32, 32, 32, 32,
		32, 32, 32, 32, 32, 32, 32, 32,
		32, 32, 32, 32, 32, 32, 32, 32, 32,
	};
	uint8_t delta_adptctrl_lut[] = {
		32, 32, 32, 32, 32, 32, 32, 32,
		32, 32, 32, 32, 32, 32, 32, 32,
		32, 32, 32, 32, 32, 32, 32, 32,
		32, 32, 32, 32, 32, 32, 32, 32, 32,
	};
	uint64_t addr = 0;
	uint32_t val = 0;

	addr = ee + _OFST(REG_ISP_EE_T, REG_30);
	REG_ARRAY_UPDATE2(addr, dgr4_filter000);
	addr = ee + _OFST(REG_ISP_EE_T, REG_58);
	REG_ARRAY_UPDATE2(addr, dgr4_filter045);
	addr = ee + _OFST(REG_ISP_EE_T, REG_6C);
	REG_ARRAY_UPDATE2(addr, dgr4_filter090);
	addr = ee + _OFST(REG_ISP_EE_T, REG_80);
	REG_ARRAY_UPDATE2(addr, dgr4_filter135);
	addr = ee + _OFST(REG_ISP_EE_T, REG_94);
	REG_ARRAY_UPDATE2(addr, dgr4_filternod);

	addr = ee + _OFST(REG_ISP_EE_T, REG_A4);
	REG_ARRAY_UPDATE2(addr, luma_coring_lut);
	addr = ee + _OFST(REG_ISP_EE_T, REG_E8);
	REG_ARRAY_UPDATE4(addr, luma_shtctrl_lut);
	addr = ee + _OFST(REG_ISP_EE_T, REG_10C);
	REG_ARRAY_UPDATE4(addr, delta_shtctrl_lut);
	addr = ee + _OFST(REG_ISP_EE_T, REG_130);
	REG_ARRAY_UPDATE4(addr, luma_adptctrl_lut);
	addr = ee + _OFST(REG_ISP_EE_T, REG_154);
	REG_ARRAY_UPDATE4(addr, delta_adptctrl_lut);
}

int ispblk_dci_config(struct isp_ctx *ctx, bool en, uint16_t *lut,
		      uint16_t lut_size)
{
	uint64_t dci = ctx->phys_regs[ISP_BLK_ID_DCI];
	uint64_t addr = 0;
	uint32_t val = 0;

	ISP_WR_BITS(dci, REG_ISP_DCI_T, DCI_ENABLE, DCI_ENABLE, en);
	ISP_WR_BITS(dci, REG_ISP_DCI_T, DCI_ENABLE, DCI_HIST_ENABLE, en);
	ISP_WR_BITS(dci, REG_ISP_DCI_T, DCI_MAP_ENABLE, DCI_MAP_ENABLE, false);
	ISP_WR_BITS(dci, REG_ISP_DCI_T, DCI_MAP_ENABLE, DCI_DITHER_ENABLE, 0);
	ISP_WR_REG(dci, REG_ISP_DCI_T, IMG_WIDTHM1, ctx->img_width - 1);
	ISP_WR_BITS(dci, REG_ISP_DCI_T, DCI_ROI_START, DCI_ROI_START_X, 0);
	ISP_WR_BITS(dci, REG_ISP_DCI_T, DCI_ROI_START, DCI_ROI_START_Y, 0);
	ISP_WR_BITS(dci, REG_ISP_DCI_T, DCI_ROI_GEO,
		    DCI_ROI_WIDTHM1, ctx->img_width - 1);
	ISP_WR_BITS(dci, REG_ISP_DCI_T, DCI_ROI_GEO,
		    DCI_ROI_HEIGHTM1, ctx->img_height - 1);

	addr = dci + _OFST(REG_ISP_DCI_T, DCI_MAPPED_LUT);
	REG_ARRAY_UPDATE2_SIZE(addr, lut, lut_size);

	if (ctx->is_tile) {
		if (ctx->is_work_on_r_tile) {
			ISP_WR_BITS(dci, REG_ISP_DCI_T, DCI_ROI_START, DCI_ROI_START_X, ctx->tile_cfg.r_out.start - ctx->tile_cfg.r_in.start);
			ISP_WR_BITS(dci, REG_ISP_DCI_T, DCI_ROI_GEO, DCI_ROI_WIDTHM1, ctx->tile_cfg.r_out.end - ctx->tile_cfg.r_out.start);
			ISP_WR_BITS(dci, REG_ISP_DCI_T, DCI_MAP_ENABLE, DCI_ZEROING_ENABLE, false);
		} else {
			ISP_WR_BITS(dci, REG_ISP_DCI_T, DCI_ROI_GEO, DCI_ROI_WIDTHM1, ctx->tile_cfg.l_out.end - ctx->tile_cfg.l_out.start);
			ISP_WR_BITS(dci, REG_ISP_DCI_T, DCI_MAP_ENABLE, DCI_ZEROING_ENABLE, true);
		}

		ISP_WR_BITS(dci, REG_ISP_DCI_T, DCI_ROI_GEO, DCI_ROI_HEIGHTM1, ctx->img_height - 1);
		ISP_WR_BITS(dci, REG_ISP_DCI_T, DCI_MAP_ENABLE, DCI_ROI_ENABLE, true);
	}

	return 0;
}

/* This function performs clipping of the histogram and redistribution of bins.
 * The histogram is clipped and the number of excess pixels is counted.
 * Afterwards * the excess pixels are equally redistributed across the whole
 * histogram (providing * the bin count is smaller than the cliplimit).
 */
void ClipHist(uint16_t *pHist, uint16_t nbins, const uint16_t ClipLimit)
{
	uint16_t *ptr = NULL, *ptr_end = NULL, *ptr_Hist = NULL;
	uint32_t nExcess = 0, nExcessPerBin = 0;
	uint16_t Upper = 0;
	int i = 0;

	/* calculate total number of excess pixels */
	for (i = 0; i < nbins; i++)
		if (pHist[i] > ClipLimit)
			nExcess += pHist[i] - ClipLimit;

	/* clip histogram and redistribute excess pixels in each bin */
	nExcessPerBin = nExcess / nbins;
	Upper = ClipLimit - nExcessPerBin;

	for (i = 0; i < nbins; i++) {
		if (pHist[i] > ClipLimit)
			pHist[i] = ClipLimit;
		else {
			if (pHist[i] > Upper) {
				nExcess -= (pHist[i] - Upper);
				pHist[i] = ClipLimit;
			} else {
				nExcess -= nExcessPerBin;
				pHist[i] += nExcessPerBin;
			}
		}
	}

	while (nExcess > 0) {
		bool flag = false;

		ptr_Hist = pHist;
		ptr_end = &pHist[nbins];

		for (ptr = ptr_Hist; ptr < ptr_end && (nExcess > 0); ptr++) {
			if (*ptr < ClipLimit) {
				(*ptr)++;
				nExcess--;
				flag = true;
			}
		}

		if (flag)
			for (ptr = ptr_Hist
			    ; ptr < ptr_end && (nExcess > 0)
			    ; ptr++) {
				(*ptr)++;
				nExcess--;
			}
	}
}

/* @brief Black & white stretching
 */
void BWStretch(uint16_t *pHist, uint16_t nbins, struct dci_param *param)
{
	uint16_t *ptr = NULL;
	uint16_t n = 0;

	for (n = 0, ptr = pHist; n < nbins; ptr++, n++) {
		if (n < param->strecth_th_0)
			*ptr *= ((512 - param->strecth_strength_0) >> 8);
		else if (n > param->strecth_th_1)
			*ptr *= ((512 - param->strecth_strength_1) >> 8);
	}
}

/* This function calculates the equalized lookup table (mapping) by
 * cumulating the input histogram. Note: lookup table is rescaled in
 * range [Min..Max].
 */
void mapHist(uint16_t *hist, uint8_t min, uint8_t max,
	     uint16_t nrGreylevels, unsigned long ulNrOfPixels)
{
	uint16_t i;
	unsigned long sum = 0;
	//const float fScale = ((float)(Max - Min)) / ulNrOfPixels;
	const uint8_t range = (max - min);
	const uint16_t ulMin = min;

	for (i = 0; i < nrGreylevels; i++) {
		sum += hist[i];
		//hist[i] = (ulMin + sum*fScale);
		hist[i] = ulMin +
			  (sum * range + (ulNrOfPixels>>1)) / ulNrOfPixels;
		if (hist[i] > max)
			hist[i] = max;
	}
}

void ispblk_dci_hist_gen(uint8_t *raw, uint16_t *hist)
{
	uint16_t i;

	for (i = 0; i < 0x200; i += 2)
		hist[i>>1] = *(raw + i) | (*(raw + i + 1) << 8);

	for (i = 0; i < 0x100; ++i) {
	    uartlog("%#x ", hist[i]);
	    if ((i & 0xf) == 0xf)
		uartlog("\n");
	}
}

void ispblk_dci_lut_gen(struct isp_ctx *ctx, uint16_t *hist,
			struct dci_param *param)
{
	// Minimum greyvalue of input image
	// (also becomes minimum of output image)
	const int min = 0;
	// Maximum greyvalue of input image
	// (also becomes maximum of output image)
	const int max = ((1 << 8) - 1);
	// Number of greybins for histogram ("dynamic range")
	const int nbins = 256;
	uint32_t nPixels = 0;
	uint32_t ClipLimit = 0;
	int n;

	if (param->cliplimit_sel == 0)
		return;
	if (param->cliplimit_sel > 1023)
		param->cliplimit_sel = 1023;
	ClipLimit = ((uint32_t)(256 + param->cliplimit_sel)
		     * ctx->img_width * ctx->img_height) >> 8;

	ClipHist(hist, nbins, ClipLimit);

	// Black & white stretching
	BWStretch(hist, nbins, param);

	nPixels = 0;
	for (n = 0; n < nbins; n++)
		nPixels += hist[n];

	// Map histogram
	mapHist(hist, min, max, nbins, nPixels);
	for (n = 0; n < 0x100; ++n) {
	    uartlog("%#x ", hist[n]);
	    if ((n & 0xf) == 0xf)
		uartlog("\n");
	}
}

int ispblk_ee_config(struct isp_ctx *ctx, bool enable, bool bypass)
{
	uint64_t ee = ctx->phys_regs[ISP_BLK_ID_EE];

	ISP_WR_BITS(ee, REG_ISP_EE_T, REG_00, EE_BYPASS, 0);
	ISP_WR_BITS(ee, REG_ISP_EE_T, REG_00, EE_TOTAL_GAIN, 32);
	ISP_WR_BITS(ee, REG_ISP_EE_T, REG_00, EE_TOTAL_CORING, 64);
	ISP_WR_BITS(ee, REG_ISP_EE_T, REG_1BC, IMG_WIDTH,
		    ctx->img_width - 1);
	ISP_WR_BITS(ee, REG_ISP_EE_T, REG_1BC, IMG_HEIGHT,
		    ctx->img_height - 1);

	return 0;
}

int ispblk_ycur_config(struct isp_ctx *ctx, uint8_t sel, uint16_t *data)
{
	uint64_t ycur = ctx->phys_regs[ISP_BLK_ID_YCURVE];
	uint16_t i;
	union REG_ISP_YCUR_PROG_DATA reg_data;

	ISP_WR_BITS(ycur, REG_ISP_YCUR_T, YCUR_PROG_CTRL, YCUR_WSEL, sel);
	ISP_WR_BITS(ycur, REG_ISP_YCUR_T, YCUR_PROG_ST_ADDR, YCUR_ST_ADDR, 0);
	ISP_WR_BITS(ycur, REG_ISP_YCUR_T, YCUR_PROG_ST_ADDR, YCUR_ST_W, 1);
	ISP_WR_REG(ycur, REG_ISP_YCUR_T, YCUR_PROG_MAX, data[0x40]);
	for (i = 0; i < 0x40; i += 2) {
		reg_data.raw = 0;
		reg_data.bits.YCUR_DATA_E = data[i];
		reg_data.bits.YCUR_DATA_O = data[i + 1];
		reg_data.bits.YCUR_W = 1;
		ISP_WR_REG(ycur, REG_ISP_YCUR_T, YCUR_PROG_DATA, reg_data.raw);
	}

	return 0;
}

int ispblk_ycur_enable(struct isp_ctx *ctx, bool enable, uint8_t sel)
{
	uint64_t ycur = ctx->phys_regs[ISP_BLK_ID_YCURVE];

	ISP_WR_BITS(ycur, REG_ISP_YCUR_T, YCUR_CTRL, YCUR_ENABLE, enable);
	ISP_WR_BITS(ycur, REG_ISP_YCUR_T, YCUR_PROG_CTRL, YCUR_RSEL, sel);

	return 0;
}

int ispblk_3dlut_config(struct isp_ctx *ctx)
{
	uint64_t threedlut = ctx->phys_regs[ISP_BLK_ID_YUVTOP];
	u32 v_idx = 0, s_idx = 0, h_idx = 0;

	ISP_WR_BITS(threedlut, REG_YUV_TOP_T, HSV_LUT_CONTROL, HSV3DLUT_ENABLE, 0x0);

	for (; v_idx < 9; v_idx++) {
		for (s_idx = 0; s_idx < 13; s_idx++) {
			for (h_idx = 0; h_idx < 28; h_idx++) {
				ISP_WR_REG(threedlut, REG_YUV_TOP_T, HSV_LUT_PROG_SRAM0, 0x0);
				//ISP_WR_REG(threedlut, REG_YUV_TOP_T, HSV_LUT_PROG_SRAM0, 0xFFEFFEFF);
				//ISP_WR_REG(threedlut, REG_YUV_TOP_T, HSV_LUT_PROG_SRAM0, 0x00300301);
				//ISP_WR_REG(threedlut, REG_YUV_TOP_T, HSV_LUT_PROG_SRAM1, ((v_idx << 11) | (s_idx << 7) | (h_idx << 2) | 0x2));
				ISP_WR_REG(threedlut, REG_YUV_TOP_T, HSV_LUT_PROG_SRAM1, ((v_idx << 11) | (s_idx << 7) | (h_idx << 2) | 0x1));
				ISP_WR_REG(threedlut, REG_YUV_TOP_T, HSV_LUT_PROG_SRAM1, ((0x8000) | (v_idx << 11) | (s_idx << 7) | (h_idx << 2) | 0x1));
				//ISP_WR_BITS(threedlut, REG_YUV_TOP_T, HSV_LUT_PROG_SRAM1, SRAM_RD, 0x1);
				//uartlog("[5566] test reg:%x\n", ISP_RD_REG(threedlut, REG_YUV_TOP_T, HSV_LUT_READ_SRAM0));
			}
		}
	}

	ISP_WR_BITS(threedlut, REG_YUV_TOP_T, HSV_ENABLE, HSV_ENABLE, 0x1);
	ISP_WR_BITS(threedlut, REG_YUV_TOP_T, HSV_ENABLE, AVG_MODE, 0x1);
	ISP_WR_BITS(threedlut, REG_YUV_TOP_T, HSV_LUT_CONTROL, HSV3DLUT_ENABLE, 0x1);
	// output yuv444 if sc-online
	ISP_WR_BITS(threedlut, REG_YUV_TOP_T, HSV_ENABLE,
		    SC_DMA_SWITCH, !ctx->is_offline_scaler);

	return 0;
}

int ispblk_preraw_config(struct isp_ctx *ctx)
{
	uint64_t preraw;
	uint8_t bdg_out_mode = 0;

	preraw = (ctx->cam_id == 0)
		 ? ctx->phys_regs[ISP_BLK_ID_PRERAW0]
		 : ctx->phys_regs[ISP_BLK_ID_PRERAW1_R1];

	bdg_out_mode = (ctx->is_offline_postraw) ? 1 : 2;
	ISP_WR_BITS(preraw, REG_PRE_RAW_T, TOP_CTRL, CSI_BDG_OUT_MODE,
		    bdg_out_mode);

	return 0;
}

int ispblk_rawtop_config(struct isp_ctx *ctx)
{
	uint64_t rawtop = ctx->phys_regs[ISP_BLK_ID_RAWTOP];

	ISP_WR_BITS(rawtop, REG_RAW_TOP_T, RAW_2, IMG_WIDTHM1,
		    ctx->img_width - 1);
	ISP_WR_BITS(rawtop, REG_RAW_TOP_T, RAW_2, IMG_HEIGHTM1,
		    ctx->img_height - 1);

	if (ctx->is_yuv_sensor) {
		ISP_WO_BITS(rawtop, REG_RAW_TOP_T, CTRL, LS_CROP_DST_SEL, 1);
		ISP_WO_BITS(rawtop, REG_RAW_TOP_T, RAW_4, YUV_IN_MODE, 1);
	} else {
		ISP_WO_BITS(rawtop, REG_RAW_TOP_T, CTRL, LS_CROP_DST_SEL, 0);
		ISP_WO_BITS(rawtop, REG_RAW_TOP_T, RAW_4, YUV_IN_MODE, 0);
	}

	return 0;
}

int ispblk_yuvtop_config(struct isp_ctx *ctx)
{
	uint64_t yuvtop = ctx->phys_regs[ISP_BLK_ID_YUVTOP];

	ISP_WR_BITS(yuvtop, REG_YUV_TOP_T, YUV_3, Y42_SEL, ctx->is_yuv_sensor);

	return 0;
}

int ispblk_isptop_config(struct isp_ctx *ctx)
{
	union REG_ISP_TOP_1 reg1;
	union REG_ISP_TOP_3 reg3;
	uint64_t isptopb = ctx->phys_regs[ISP_BLK_ID_ISPTOP];
	uint8_t pre_trig_by_hw = !(ctx->is_offline_preraw);
	uint8_t post_trig_by_hw = !(ctx->is_offline_postraw ||
				    ctx->is_offline_preraw);

	// lock(glbLockReg)

	// TODO: Y-only format?
	ISP_WR_BITS(isptopb, REG_ISP_TOP_T, REG_4, DATA_FORMAT,
		    ctx->is_yuv_sensor);

	ISP_WR_BITS(isptopb, REG_ISP_TOP_T, REG_5, IMG_W,
		    ctx->img_width - 1);
	ISP_WR_BITS(isptopb, REG_ISP_TOP_T, REG_5, IMG_H,
		    ctx->img_height - 1);

	reg1.raw = 0;
	reg3.raw = 0;
	if (ctx->cam_id == 0) {
		reg1.bits.FRAME_START_PRE_EN = 1;
		reg1.bits.FRAME_DONE_PRE_EN = 1;
		reg1.bits.SHAW_DONE_PRE_EN = 1;
		reg1.bits.FRAME_ERR_PRE_EN = 1;

		reg3.bits.TRIG_STR_SEL_PRE = pre_trig_by_hw;
		reg3.bits.SHAW_UP_SEL_PRE = pre_trig_by_hw;
	} else {
		reg1.bits.FRAME_START_PRE1_EN = 1;
		reg1.bits.FRAME_DONE_PRE1_EN = 1;
		reg1.bits.SHAW_DONE_PRE1_EN = 1;
		reg1.bits.FRAME_ERR_PRE1_EN = 1;

		reg3.bits.TRIG_STR_SEL_PRE1 = pre_trig_by_hw;
		reg3.bits.SHAW_UP_SEL_PRE1 = pre_trig_by_hw;
	}

	// postraw
	reg1.bits.FRAME_DONE_POST_EN = 1;
	reg1.bits.SHAW_DONE_POST_EN = 1;
	reg1.bits.FRAME_ERR_POST_EN = 1;
	reg3.bits.TRIG_STR_SEL_POST = post_trig_by_hw;
	reg3.bits.SHAW_UP_SEL_POST = post_trig_by_hw;


	ISP_WR_REG(isptopb, REG_ISP_TOP_T, REG_0, 0xffffffff);
	ISP_WR_REG(isptopb, REG_ISP_TOP_T, REG_1, reg1.raw);
	ISP_WR_REG(isptopb, REG_ISP_TOP_T, REG_3, reg3.raw);

	// unlock(glbLockReg)

	return 0;
}

static void _patgen_config_timing(struct isp_ctx *ctx)
{
	uint64_t csibdg;
	uint16_t pat_height = (ctx->is_hdr_on) ? (ctx->src_height*2-1) :
			      (ctx->src_height-1);
	csibdg = (ctx->cam_id == 0)
		 ? ctx->phys_regs[ISP_BLK_ID_CSIBDG0]
		 : ctx->phys_regs[ISP_BLK_ID_CSIBDG1_R1];

	ISP_WR_BITS(csibdg, REG_ISP_CSI_BDG_T, CSI_MDE_V_SIZE, VMDE_STR, 0x00);
	ISP_WR_BITS(csibdg, REG_ISP_CSI_BDG_T,
		    CSI_MDE_V_SIZE, VMDE_STP, pat_height);
	ISP_WR_BITS(csibdg, REG_ISP_CSI_BDG_T, CSI_MDE_H_SIZE, HMDE_STR, 0x00);
	ISP_WR_BITS(csibdg, REG_ISP_CSI_BDG_T, CSI_MDE_H_SIZE,
		    HMDE_STP, ctx->src_width - 1);
	ISP_WR_BITS(csibdg, REG_ISP_CSI_BDG_T, CSI_FDE_V_SIZE, VFDE_STR, 0x06);
	ISP_WR_BITS(csibdg, REG_ISP_CSI_BDG_T, CSI_FDE_V_SIZE,
		    VFDE_STP, 0x06 + pat_height);
	ISP_WR_BITS(csibdg, REG_ISP_CSI_BDG_T, CSI_FDE_H_SIZE, HFDE_STR, 0x0C);
	ISP_WR_BITS(csibdg, REG_ISP_CSI_BDG_T, CSI_FDE_H_SIZE,
		    HFDE_STP, 0x0C + ctx->src_width - 1);
	ISP_WR_BITS(csibdg, REG_ISP_CSI_BDG_T, CSI_HSYNC_CTRL, HS_STR, 0);
	ISP_WR_BITS(csibdg, REG_ISP_CSI_BDG_T, CSI_HSYNC_CTRL, HS_STP, 3);
	ISP_WR_BITS(csibdg, REG_ISP_CSI_BDG_T, CSI_VSYNC_CTRL, VS_STR, 0);
	ISP_WR_BITS(csibdg, REG_ISP_CSI_BDG_T, CSI_VSYNC_CTRL, VS_STP, 3);
#if defined(VIP_PERFORMANCE_TEST)
	u32 htt = (ctx->src_width * 114) / 100;
	// 200M for 2M, 400M for 5M, 600M for 8M
	u32 tick_per_ms;

	switch (ctx->src_width) {
	default:
	case 1920:
		tick_per_ms = 200000000 / 1000;
		break;
	case 2688:
		tick_per_ms = 400000000 / 1000;
		break;
	case 3840:
		tick_per_ms = 600000000 / 1000;
		break;
	}

	ISP_WR_BITS(csibdg, REG_ISP_CSI_BDG_T, CSI_TGEN_TT_SIZE, HTT, htt);
	ISP_WR_BITS(csibdg, REG_ISP_CSI_BDG_T, CSI_TGEN_TT_SIZE, VTT,
		    pat_height + (tick_per_ms * 3 / htt));
#else
	ISP_WR_BITS(csibdg, REG_ISP_CSI_BDG_T, CSI_TGEN_TT_SIZE, VTT, 0x1FFF);
	ISP_WR_BITS(csibdg, REG_ISP_CSI_BDG_T, CSI_TGEN_TT_SIZE, HTT, 0x1FFF);
#endif
}

static void _patgen_config_pat(struct isp_ctx *ctx)
{
	uint64_t csibdg;

	csibdg = (ctx->cam_id == 0)
		 ? ctx->phys_regs[ISP_BLK_ID_CSIBDG0]
		 : ctx->phys_regs[ISP_BLK_ID_CSIBDG1_R1];

	ISP_WR_BITS(csibdg, REG_ISP_CSI_BDG_T, CSI_PAT_GEN_CTRL, GRA_INV, 0);
	ISP_WR_BITS(csibdg, REG_ISP_CSI_BDG_T, CSI_PAT_GEN_CTRL, AUTO_EN, 0);
	ISP_WR_BITS(csibdg, REG_ISP_CSI_BDG_T, CSI_PAT_GEN_CTRL, DITH_EN, 0);
	ISP_WR_BITS(csibdg, REG_ISP_CSI_BDG_T, CSI_PAT_GEN_CTRL, SNOW_EN, 0);
	ISP_WR_BITS(csibdg, REG_ISP_CSI_BDG_T, CSI_PAT_GEN_CTRL, FIX_MC, 0);
	ISP_WR_BITS(csibdg, REG_ISP_CSI_BDG_T, CSI_PAT_GEN_CTRL, DITH_MD, 0);
	ISP_WR_BITS(csibdg, REG_ISP_CSI_BDG_T, CSI_PAT_GEN_CTRL,
		    BAYER_ID, ctx->rgb_color_mode);
	ISP_WR_BITS(csibdg, REG_ISP_CSI_BDG_T, CSI_PAT_IDX_CTRL, PAT_PRD, 0);
	ISP_WR_BITS(csibdg, REG_ISP_CSI_BDG_T, CSI_PAT_IDX_CTRL, PAT_IDX, 7);
	ISP_WR_BITS(csibdg, REG_ISP_CSI_BDG_T, CSI_PAT_COLOR_0, PAT_R, 0xFFF);
	ISP_WR_BITS(csibdg, REG_ISP_CSI_BDG_T, CSI_PAT_COLOR_0, PAT_G, 0xFFF);
	ISP_WR_BITS(csibdg, REG_ISP_CSI_BDG_T, CSI_PAT_COLOR_1, PAT_B, 0xFFF);
	ISP_WR_BITS(csibdg, REG_ISP_CSI_BDG_T,
		    CSI_BACKGROUND_COLOR_0, FDE_R, 0);
	ISP_WR_BITS(csibdg, REG_ISP_CSI_BDG_T,
		    CSI_BACKGROUND_COLOR_0, FDE_G, 1);
	ISP_WR_BITS(csibdg, REG_ISP_CSI_BDG_T,
		    CSI_BACKGROUND_COLOR_1, FDE_B, 2);
	ISP_WR_BITS(csibdg, REG_ISP_CSI_BDG_T, CSI_FIX_COLOR_0, MDE_R, 0x457);
	ISP_WR_BITS(csibdg, REG_ISP_CSI_BDG_T, CSI_FIX_COLOR_0, MDE_G, 0x8AE);
	ISP_WR_BITS(csibdg, REG_ISP_CSI_BDG_T, CSI_FIX_COLOR_1, MDE_B, 0xD05);
}

int ispblk_csibdg_config(struct isp_ctx *ctx)
{
	uint64_t csibdg;
	uint8_t csi_mode = 0;

	csibdg = (ctx->cam_id == 0)
		 ? ctx->phys_regs[ISP_BLK_ID_CSIBDG0]
		 : ctx->phys_regs[ISP_BLK_ID_CSIBDG1_R1];

	ISP_WR_BITS(csibdg, REG_ISP_CSI_BDG_T, CSI_CTRL,
		    RESET_MODE, 1);

	if (ctx->is_yuv_sensor) {
		ISP_WR_BITS(csibdg, REG_ISP_CSI_BDG_T, CSI_CTRL,
			    CSI_IN_FORMAT, 1);
		ISP_WR_BITS(csibdg, REG_ISP_CSI_BDG_T, CSI_CTRL,
			    CSI_IN_YUV_FORMAT, 0);
		ISP_WR_BITS(csibdg, REG_ISP_CSI_BDG_T, CSI_CTRL, Y_ONLY, 0);
		ISP_WR_BITS(csibdg, REG_ISP_CSI_BDG_T, CSI_CTRL,
			    YUV2BAY_ENABLE, 1);
		// patgen's bayer format for yuv2bay
		ISP_WR_BITS(csibdg, REG_ISP_CSI_BDG_T, CSI_PAT_GEN_CTRL,
			    BAYER_ID, ctx->rgb_color_mode);
	} else {
		ISP_WR_BITS(csibdg, REG_ISP_CSI_BDG_T, CSI_CTRL,
			    CSI_IN_FORMAT, 0);
	}
	ISP_WR_BITS(csibdg, REG_ISP_CSI_BDG_T, CSI_CTRL, CH_NUM,
		    ctx->is_hdr_on);

	ISP_WR_BITS(csibdg, REG_ISP_CSI_BDG_T, CSI_CTRL,
		    DMA_WR_ENABLE, ctx->is_offline_postraw);

	if (ctx->csibdg_patgen_en) {
		csi_mode = 3;
		ISP_WR_BITS(csibdg, REG_ISP_CSI_BDG_T, CSI_CTRL,
			    PXL_DATA_SEL, 1);
		ISP_WR_BITS(csibdg, REG_ISP_CSI_BDG_T,
			    CSI_PAT_GEN_CTRL, PAT_EN, 1);

		_patgen_config_timing(ctx);
		_patgen_config_pat(ctx);
	} else {
		ISP_WR_BITS(csibdg, REG_ISP_CSI_BDG_T, CSI_CTRL,
			    PXL_DATA_SEL, 0);

		csi_mode = (ctx->is_offline_preraw) ? 2 : 1;
	}

	ISP_WR_BITS(csibdg, REG_ISP_CSI_BDG_T, CSI_SIZE,
		    FRAME_WIDTHM1, ctx->src_width - 1);
	ISP_WR_BITS(csibdg, REG_ISP_CSI_BDG_T, CSI_SIZE,
		    FRAME_HEIGHTM1, ctx->src_height - 1);

	ISP_WR_BITS(csibdg, REG_ISP_CSI_BDG_T, CSI_CTRL, CSI_MODE, csi_mode);

	if (ctx->is_dpcm_on) {
		union REG_ISP_CSI_BDG_DMA_DPCM_MODE dpcm;
		dpcm.bits.DMA_ST_DPCM_MODE = 0x7;
		dpcm.bits.DPCM_XSTR = ctx->tile_cfg.r_in.start;

		ISP_WR_REG(csibdg, REG_ISP_CSI_BDG_T, CSI_DMA_DPCM_MODE, dpcm.raw);
	}

	return 0;
}

void ispblk_dma_ring_buffer_test(struct isp_ctx *ctx, int dmaid, bool en)
{
	uint64_t dmab = ctx->phys_regs[dmaid];
	uint64_t addr_l;
	uint32_t stride = 0;

	switch(dmaid) {
	case ISP_BLK_ID_DMA6:
		//ring buffer enable
		ISP_WR_REG(dmab, REG_ISP_DMA_T, DMA_7, en);

		if (en) {
			//get dma org base addr_l
			addr_l = ISP_RD_REG(dmab, REG_ISP_DMA_T, DMA_0);
			ISP_WR_REG(dmab, REG_ISP_DMA_T, DMA_3, addr_l); //min addr_l
			ISP_WR_REG(dmab, REG_ISP_DMA_T, DMA_4, 1); //min addr_h

			//Use half buffer to ring.
			stride = ISP_RD_BITS(dmab, REG_ISP_DMA_T, DMA_10, LINE_OFFSET);
			addr_l += stride * (ctx->img_height / 2);

			ISP_WR_REG(dmab, REG_ISP_DMA_T, DMA_5, addr_l); //max addr_l
			ISP_WR_REG(dmab, REG_ISP_DMA_T, DMA_6, 1); //max addr_h
		}
		break;
	}
}

int ispblk_dma_config(struct isp_ctx *ctx, int dmaid, uint64_t buf_addr)
{
	uint64_t dmab = ctx->phys_regs[dmaid];
	uint32_t stride = 0, dma_cnt = 0, w = 0, h = 0, mod = 0;

	switch (dmaid) {
	case ISP_BLK_ID_DMA0:	// preraw0
	case ISP_BLK_ID_DMA53:	// preraw1
		/* csibdg */
		w = ctx->src_width;
		h = ctx->src_height;
		if (ctx->is_yuv_sensor) {
			if (ctx->sensor_bitdepth == 10)
				dma_cnt = 5 * UPPER(w, 2);
			else
				dma_cnt = w;
		} else {	// bayer 12bit only
			dma_cnt = 3 * UPPER(w, 1);
			mod = 3;
		}

		if (ctx->is_dpcm_on) {
			w = ((ctx->src_width + 8) >> 1);
			dma_cnt = (w >> 1) * 3;
		}
		break;
	case ISP_BLK_ID_DMA1:	// preraw0
	case ISP_BLK_ID_DMA54:	// preraw1
		if (ctx->is_yuv_sensor) {
			w = ctx->src_width/2;
			h = ctx->src_height;
			if (ctx->sensor_bitdepth == 10)
				dma_cnt = 5 * UPPER(w, 2);
			else
				dma_cnt = w;
		} else if (ctx->is_hdr_on) {
			w = ctx->src_width;
			h = ctx->src_height;
			dma_cnt = 3 * UPPER(w, 1);
			mod = 3;

			if (ctx->is_dpcm_on) {
				w = ((ctx->src_width + 8) >> 1);
				dma_cnt = (w >> 1) * 3;
			}
		}
		break;
	case ISP_BLK_ID_DMA2:	// preraw0
	case ISP_BLK_ID_DMA55:	// preraw1
		if (ctx->is_yuv_sensor) {
			w = ctx->src_width/2;
			h = ctx->src_height;
			if (ctx->sensor_bitdepth == 10)
				dma_cnt = 5 * UPPER(w, 2);
			else
				dma_cnt = w;
		}
		break;
	case ISP_BLK_ID_DMA3:	// preraw0
		/* yuvtop crop456 */
		if (ctx->is_tile) {
			w = (ctx->is_work_on_r_tile)
			    ? ctx->tile_cfg.r_out.end -
			      ctx->tile_cfg.r_out.start + 1
			    : ctx->tile_cfg.l_out.end + 1;
		} else {
			w = ctx->img_width;
		}
		h = ctx->img_height;
		dma_cnt = w;
		break;
	case ISP_BLK_ID_DMA4:	// preraw0
	case ISP_BLK_ID_DMA5:
		/* yuvtop crop456 */
		if (ctx->is_tile) {
			w = (ctx->is_work_on_r_tile)
			    ? ctx->tile_cfg.r_out.end -
			      ctx->tile_cfg.r_out.start + 1
			    : ctx->tile_cfg.l_out.end + 1;
		} else {
			w = ctx->img_width;
		}
		w >>= 1;
		h = ctx->img_height >> 1;
		dma_cnt = w;
		break;
	// preraw0 lmap
	case ISP_BLK_ID_DMA9:
	case ISP_BLK_ID_DMA14:
	// preraw1 lmap
	case ISP_BLK_ID_DMA29:
	case ISP_BLK_ID_DMA30:
	{
		uint64_t map = 0;
		union REG_ISP_LMAP_LMP_2 reg2;

		switch (dmaid) {
		case ISP_BLK_ID_DMA9:
			map = ctx->phys_regs[ISP_BLK_ID_LMP0];
			break;
		case ISP_BLK_ID_DMA14:
			map = ctx->phys_regs[ISP_BLK_ID_LMP1];
			break;
		case ISP_BLK_ID_DMA29:
			map = ctx->phys_regs[ISP_BLK_ID_LMP2_R1];
			break;
		case ISP_BLK_ID_DMA30:
			map = ctx->phys_regs[ISP_BLK_ID_LMP3_R1];
			break;
		}
		reg2.raw = ISP_RD_REG(map, REG_ISP_LMAP_T, LMP_2);

		// group image into blocks
#if 0	// 2k
		w = (reg2.bits.LMAP_W_GRID_NUM + 1) *
		    (reg2.bits.LMAP_H_GRID_NUM + 1);
		h = 1;
#else	// new 4k
		w = reg2.bits.LMAP_W_GRID_NUM + 1 +
		    ((~reg2.bits.LMAP_W_GRID_NUM) & 0x1);
		h = reg2.bits.LMAP_H_GRID_NUM + 1;
#endif
		dma_cnt = (w * 3) >> 1;
		mod = 3;
		break;
	}
	// preraw0 rgbmap
	case ISP_BLK_ID_DMA15:
	case ISP_BLK_ID_DMA16:
	// preraw1 rgbmap
	case ISP_BLK_ID_DMA31:
	case ISP_BLK_ID_DMA32:
	{
		uint64_t map = 0;
		union REG_ISP_RGBMAP_0 reg0;

		switch (dmaid) {
		case ISP_BLK_ID_DMA15:
			map = ctx->phys_regs[ISP_BLK_ID_RGBMAP0];
			break;
		case ISP_BLK_ID_DMA16:
			map = ctx->phys_regs[ISP_BLK_ID_RGBMAP1];
			break;
		case ISP_BLK_ID_DMA31:
			map = ctx->phys_regs[ISP_BLK_ID_RGBMAP2_R1];
			break;
		case ISP_BLK_ID_DMA32:
			map = ctx->phys_regs[ISP_BLK_ID_RGBMAP3_R1];
			break;
		}

		reg0.raw = ISP_RD_REG(map, REG_ISP_RGBMAP_T, RGBMAP_0);

		// group image into blocks
#if 0	// 2k
		w = reg0.bits.RGBMAP_W_GRID_NUM;
		h = ((reg0.bits.RGBMAP_H_GRID_NUM + 1) + 6) / 7;
#else	// 4k
		w = ((reg0.bits.RGBMAP_W_GRID_NUM + 1) + 6) / 7;
		h = reg0.bits.RGBMAP_H_GRID_NUM + 1;
#endif
		dma_cnt = w << 5;
		mod = 4;
		break;
	}
	case ISP_BLK_ID_DMA17:
	case ISP_BLK_ID_DMA18:
	{
		/* af */
		uint64_t af;
		uint16_t block_num_x, block_num_y;

		switch (dmaid) {
		case ISP_BLK_ID_DMA17:
			af = ctx->phys_regs[ISP_BLK_ID_AF];
			break;
		case ISP_BLK_ID_DMA18:
			af = ctx->phys_regs[ISP_BLK_ID_AF_R1];
			break;
		}
		block_num_x = ISP_RD_REG(af, REG_ISP_AF_T, BLOCK_NUM_X);
		block_num_y = ISP_RD_REG(af, REG_ISP_AF_T, BLOCK_NUM_Y);
		w = block_num_x * block_num_y * 20;
		dma_cnt = (block_num_x * block_num_y) << 5;
		h = 1;
		break;
	}
	case ISP_BLK_ID_DMA27:	// 3dnr y
		w = (ctx->img_width >> 1);
		h = UPPER(ctx->img_height, 4);
		dma_cnt = ctx->img_width << 4;
		mod = 4;
		break;
	case ISP_BLK_ID_DMA28:	// 3dnr uv
		w = (ctx->img_width >> 1);
		h = UPPER(ctx->img_height >> 1, 4);
		dma_cnt = ctx->img_width << 4;
		mod = 4;
		break;
	case ISP_BLK_ID_DMA33: // preraw0 ae_0 (le)
	case ISP_BLK_ID_DMA34: // preraw0 ae_1 (le)
	case ISP_BLK_ID_DMA37: // preraw0 ae_0 (se)
	case ISP_BLK_ID_DMA42: // preraw1 ae_0 (le)
	case ISP_BLK_ID_DMA43: // preraw1 ae_1 (le)
	case ISP_BLK_ID_DMA46: // preraw1 ae_0 (se)
	{
		/* ae */
		uint64_t map = 0;
		uint16_t numym1 = 0;

		switch (dmaid) {
		case ISP_BLK_ID_DMA33:
		case ISP_BLK_ID_DMA34:
			map = ctx->phys_regs[ISP_BLK_ID_AEHIST0];
			break;
		case ISP_BLK_ID_DMA37:
			map = ctx->phys_regs[ISP_BLK_ID_AEHIST1];
			break;
		case ISP_BLK_ID_DMA42:
		case ISP_BLK_ID_DMA43:
			map = ctx->phys_regs[ISP_BLK_ID_AEHIST0_R1];
			break;
		case ISP_BLK_ID_DMA46:
			map = ctx->phys_regs[ISP_BLK_ID_AEHIST1_R1];
			break;
		}
		if ((dmaid == ISP_BLK_ID_DMA34) || (dmaid == ISP_BLK_ID_DMA43))
			numym1 = ISP_RD_REG(map, REG_ISP_AE_HIST_T,
					    STS_AE1_NUMYM1);
		else
			numym1 = ISP_RD_REG(map, REG_ISP_AE_HIST_T,
					    STS_AE_NUMYM1);

		w = 11 * (numym1 + 1);
		h = 1;
		dma_cnt = w << 5;
		mod = 1;
		break;
	}
	case ISP_BLK_ID_DMA35: // preraw0 hist (le)
	case ISP_BLK_ID_DMA38: // preraw0 hist (se)
	case ISP_BLK_ID_DMA44: // preraw1 hist (le)
	case ISP_BLK_ID_DMA47: // preraw1 hist (se)
		/* hist */
		w = 0x100;
		h = 1;
		dma_cnt = w << 5;
		mod = 1;
		break;
	case ISP_BLK_ID_DMA39: // preraw0 awb (le)
	case ISP_BLK_ID_DMA40: // preraw0 awb (se)
	case ISP_BLK_ID_DMA45: // postraw awb
	case ISP_BLK_ID_DMA48: // preraw1 awb (le)
	case ISP_BLK_ID_DMA49: // preraw1 awb (se)
	{
		/* awb */
		uint64_t map = 0;
		uint16_t numym1 = 0;

		switch (dmaid) {
		case ISP_BLK_ID_DMA39:
			map = ctx->phys_regs[ISP_BLK_ID_AWB0];
			break;
		case ISP_BLK_ID_DMA40:
			map = ctx->phys_regs[ISP_BLK_ID_AWB1];
			break;
		case ISP_BLK_ID_DMA45:
			map = ctx->phys_regs[ISP_BLK_ID_AWB4];
			break;
		case ISP_BLK_ID_DMA48:
			map = ctx->phys_regs[ISP_BLK_ID_AWB0_R1];
			break;
		case ISP_BLK_ID_DMA49:
			map = ctx->phys_regs[ISP_BLK_ID_AWB1_R1];
			break;
		}
		numym1 = ISP_RD_REG(map, REG_ISP_AWB_T, STS_NUMYM1);
		w = (numym1 + 1) << 5;
		h = 1;
		dma_cnt = w << 5;
		mod = 1;
		break;
	}
	case ISP_BLK_ID_DMA41: // preraw0 gms
	case ISP_BLK_ID_DMA50: // preraw1 gms
		/* gms */
		w = 0x300;
		h = 1;
		dma_cnt = w;
		mod = 1;
		break;

	case ISP_BLK_ID_DMA6:
	case ISP_BLK_ID_DMA7:
		mod = 3;
		/* rawtop crop2/3 rdma */
		w = ctx->src_width;
		h = ctx->src_height;
		dma_cnt = 3 * UPPER(w, 1);

		if (ctx->is_dpcm_on) {
			w = (ctx->src_width + 8) >> 1;
			dma_cnt = ((ctx->src_width + 8) >> 2) * 3;
		}
		break;
	case ISP_BLK_ID_DMA8:	// preraw0
	case ISP_BLK_ID_DMA56:	// preraw1
		mod = 3;
		/* csibdg rdma */
		w = ctx->src_width;
		h = ctx->src_height;
		dma_cnt = 3 * UPPER(w, 1);
		break;
	case ISP_BLK_ID_DMA10:
	case ISP_BLK_ID_DMA11:
	{
		/* lsc rdma */
		// fixed value for 37x37
		u8 blk_size = 37;

		w = blk_size;
		h = (blk_size + 1) * 3;
		dma_cnt = ((blk_size * 12) + 7) / 8;
		mod = 1;
		break;
	}
	case ISP_BLK_ID_DMA12:
	case ISP_BLK_ID_DMA13: {
		/* ltm0/1 rdma */
		uint64_t map = (dmaid == ISP_BLK_ID_DMA12)
			       ? ctx->phys_regs[ISP_BLK_ID_LMP0]
			       : ctx->phys_regs[ISP_BLK_ID_LMP1];
		union REG_ISP_LMAP_LMP_2 reg2;

		reg2.raw = ISP_RD_REG(map, REG_ISP_LMAP_T, LMP_2);

		// group image into blocks
#if 0	// 2k
		w = (reg2.bits.LMAP_W_GRID_NUM + 1) *
		    (reg2.bits.LMAP_H_GRID_NUM + 1);
		h = 1;
#else	// new 4k
		w = reg2.bits.LMAP_W_GRID_NUM + 1 +
		    ((~reg2.bits.LMAP_W_GRID_NUM) & 0x1);
		h = reg2.bits.LMAP_H_GRID_NUM + 1;
#endif
		dma_cnt = (w * 3) >> 1;
		mod = 3;
		break;
	}
	case ISP_BLK_ID_DMA19:	// le prv frame
	case ISP_BLK_ID_DMA20:	// se prv frame
	case ISP_BLK_ID_DMA21:	// le cur frame
	case ISP_BLK_ID_DMA22:	// se cur frame
	{
		uint64_t map = 0;
		union REG_ISP_RGBMAP_0 reg0;

		switch (dmaid) {
		case ISP_BLK_ID_DMA19:
		case ISP_BLK_ID_DMA21:
			map = (ctx->cam_id == 0)
			    ? ctx->phys_regs[ISP_BLK_ID_RGBMAP0]
			    : ctx->phys_regs[ISP_BLK_ID_RGBMAP2_R1];
			break;
		case ISP_BLK_ID_DMA20:
		case ISP_BLK_ID_DMA22:
			map = (ctx->cam_id == 0)
			    ? ctx->phys_regs[ISP_BLK_ID_RGBMAP1]
			    : ctx->phys_regs[ISP_BLK_ID_RGBMAP3_R1];
			break;
		}

		reg0.raw = ISP_RD_REG(map, REG_ISP_RGBMAP_T, RGBMAP_0);

		// group image into blocks
#if 0	// 2k
		w = reg0.bits.RGBMAP_W_GRID_NUM;
		h = ((reg0.bits.RGBMAP_H_GRID_NUM + 1) + 6) / 7;
#else	// 4k
		w = ((reg0.bits.RGBMAP_W_GRID_NUM + 1) + 6) / 7;
		h = reg0.bits.RGBMAP_H_GRID_NUM + 1;
#endif
		dma_cnt = w << 5;
		mod = 4;
		break;
	}
	case ISP_BLK_ID_DMA23:
	case ISP_BLK_ID_DMA24:
	{
		/* manr rdma */
		uint64_t blk = ctx->phys_regs[ISP_BLK_ID_MANR];
		union REG_ISP_MM_60 reg_60;
		uint8_t align = (ctx->is_offline_postraw) ? 4 : 5;
		uint32_t img_width = (ctx->is_tile)
				   ? ctx->tile_cfg.r_out.end + 1
				   : ctx->img_width;

		reg_60.raw = ISP_RD_REG(blk, REG_ISP_MM_T, REG_60);
		w = UPPER(UPPER(img_width, reg_60.bits.RGBMAP_W_BIT),
			align);
		h = UPPER(ctx->img_height, reg_60.bits.RGBMAP_H_BIT);

		dma_cnt = w << 5;
		mod = 4;
		break;
	}
	case ISP_BLK_ID_DMA25:	// 3dnr y
		w = (ctx->img_width >> 1);
		h = UPPER(ctx->img_height, 4);
		dma_cnt = ctx->img_width << 4;
		mod = 4;
		break;
	case ISP_BLK_ID_DMA26:	// 3dnr uv
		w = (ctx->img_width >> 1);
		h = UPPER(ctx->img_height >> 1, 4);
		dma_cnt = ctx->img_width << 4;
		mod = 4;
		break;
	case ISP_BLK_ID_DMA36:
		w = 0x10;
		h = 0x1;
		dma_cnt = 0x200;
		break;

	case ISP_BLK_ID_DMA59:
		/* yuvtop */
		w = ctx->src_width;
		h = ctx->src_height;
		if (ctx->is_yuv_sensor) {
			if (ctx->sensor_bitdepth == 10)
				dma_cnt = 5 * UPPER(w, 2);
			else
				dma_cnt = w;
		}

		// TODO: DPCM??
		if (ctx->is_dpcm_on) {
			w = ((ctx->src_width + 8) >> 1);
			dma_cnt = (w >> 1) * 3;
		}
		break;

	case ISP_BLK_ID_DMA60:
	case ISP_BLK_ID_DMA61:
		if (ctx->is_yuv_sensor) {
			w = ctx->src_width/2;
			h = ctx->src_height;
			if (ctx->sensor_bitdepth == 10)
				dma_cnt = 5 * UPPER(w, 2);
			else
				dma_cnt = w;
		}

		// TODO: DPCM??
		break;

	default:
		break;
	}

	stride = VIP_ALIGN(dma_cnt);

	if (ctx->is_tile) {
		u32 img_width = 0;

		switch (dmaid) {
		case ISP_BLK_ID_DMA3:
			img_width = ctx->tile_cfg.r_out.end + 1;
			break;
		case ISP_BLK_ID_DMA4:
		case ISP_BLK_ID_DMA5:
			img_width = (ctx->tile_cfg.r_out.end + 1) >> 1;
			break;
		default:
			break;
		}

		if (img_width != 0)
			stride = VIP_ALIGN(img_width);
	}

	ISP_WR_BITS(dmab, REG_ISP_DMA_T, DMA_9, IMG_WIDTH, w-1);
	ISP_WR_BITS(dmab, REG_ISP_DMA_T, DMA_9, IMG_HEIGHT, h-1);

	ISP_WR_BITS(dmab, REG_ISP_DMA_T, DMA_2, DMA_CNT, dma_cnt);
	ISP_WR_BITS(dmab, REG_ISP_DMA_T, DMA_10, LINE_OFFSET, stride);
	ISP_WR_BITS(dmab, REG_ISP_DMA_T, DMA_10, MOD_CTL, mod);

	if (buf_addr)
		ispblk_dma_setaddr(ctx, dmaid, buf_addr);

	return stride * h;
}

void ispblk_dma_setaddr(struct isp_ctx *ctx, uint32_t dmaid, uint64_t buf_addr)
{
	uint64_t dmab = ctx->phys_regs[dmaid];

	ISP_WR_REG(dmab, REG_ISP_DMA_T, DMA_0, (buf_addr & 0xFFFFFFFF));
	ISP_WR_REG(dmab, REG_ISP_DMA_T, DMA_1, ((buf_addr >> 32) & 0xFFFFFFFF));
}

int ispblk_dma_enable(struct isp_ctx *ctx, uint32_t dmaid, uint32_t on)
{
	uint64_t srcb = 0;

	switch (dmaid) {
	case ISP_BLK_ID_DMA0:
	case ISP_BLK_ID_DMA1:
	case ISP_BLK_ID_DMA2:
		/* csibdg */
		break;
		break;
	case ISP_BLK_ID_DMA3:
		/* yuvtop crop4 */
		srcb = ctx->phys_regs[ISP_BLK_ID_CROP4];
		break;
	case ISP_BLK_ID_DMA4:
		/* yuvtop crop5 */
		srcb = ctx->phys_regs[ISP_BLK_ID_CROP5];
		break;
	case ISP_BLK_ID_DMA5:
		/* yuvtop crop6 */
		srcb = ctx->phys_regs[ISP_BLK_ID_CROP6];
		break;
	case ISP_BLK_ID_DMA9:
		/* lmp0 */
		break;
	case ISP_BLK_ID_DMA14:
		/* lmp1 */
		break;
	case ISP_BLK_ID_DMA15:
	case ISP_BLK_ID_DMA16:
		/* rgbmap */
		break;
	case ISP_BLK_ID_DMA17:
	case ISP_BLK_ID_DMA18:
		/* af */
		break;
	case ISP_BLK_ID_DMA27:
	case ISP_BLK_ID_DMA28:
		/* 3dnr */
		break;



	case ISP_BLK_ID_DMA6:
		/* rawtop crop2/3 rdma */
		srcb = ctx->phys_regs[ISP_BLK_ID_CROP2];
		break;
	case ISP_BLK_ID_DMA7:
		/* rawtop crop2/3 rdma */
		srcb = ctx->phys_regs[ISP_BLK_ID_CROP3];
		break;
	case ISP_BLK_ID_DMA8:
		/* csibdg rdma */
		break;
	case ISP_BLK_ID_DMA10:
	case ISP_BLK_ID_DMA11:
		/* lsc rdma */
		break;
	case ISP_BLK_ID_DMA12:
	case ISP_BLK_ID_DMA13:
		/* ltm0/1 rdma */
		break;
	case ISP_BLK_ID_DMA19:
	case ISP_BLK_ID_DMA20:
	case ISP_BLK_ID_DMA21:
	case ISP_BLK_ID_DMA22:
	case ISP_BLK_ID_DMA23:
	case ISP_BLK_ID_DMA24:
		/* manr rdma */
		break;
	case ISP_BLK_ID_DMA25:
	case ISP_BLK_ID_DMA26:
		/* 3dnr rdma */
		break;

	default:
		break;
	}

	if (srcb)
		ISP_WR_BITS(srcb, REG_ISP_CROP_T, CROP_0, DMA_ENABLE, !!on);
	return 0;
}

int ispblk_dma_dbg_st(struct isp_ctx *ctx, uint32_t dmaid, uint32_t bus_sel)
{
	uint64_t dmab = ctx->phys_regs[dmaid];

	ISP_WR_BITS(dmab, REG_ISP_DMA_T, DMA_14, DBUS_SEL, bus_sel);
	return ISP_RD_BITS(dmab, REG_ISP_DMA_T, DMA_16, DBUS);
}

enum ISP_BAYER_TYPE isp_bayer_remap(enum ISP_BAYER_TYPE bayer_id,
	uint16_t x_offset, uint16_t y_offset)
{
	if (x_offset & 0x01)
		bayer_id = ((bayer_id ^ 0x01) & 0x01) | (bayer_id & 0x10);

	if (y_offset & 0x01)
		bayer_id = (bayer_id + 2) & 0x03;

	return bayer_id;
}

void isp_intr_set_mask(struct isp_ctx *ctx, union isp_intr intr_mask)
{
	uint64_t isp_top = ctx->phys_regs[ISP_BLK_ID_ISPTOP];

	ISP_WR_REG(isp_top, REG_ISP_TOP_T, REG_1, intr_mask.raw);
}

union isp_intr isp_intr_get_mask(struct isp_ctx *ctx)
{
	uint64_t isp_top = ctx->phys_regs[ISP_BLK_ID_ISPTOP];
	union isp_intr mask;

	mask.raw = ISP_RD_REG(isp_top, REG_ISP_TOP_T, REG_1);
	return mask;
}

union isp_intr isp_intr_status(struct isp_ctx *ctx)
{
	uint64_t isp_top = ctx->phys_regs[ISP_BLK_ID_ISPTOP];
	union isp_intr status;

	status.raw = ISP_RD_REG(isp_top, REG_ISP_TOP_T, REG_0);
	return status;
}

void isp_intr_clr(struct isp_ctx *ctx, union isp_intr intr_status)
{
	uint64_t isp_top = ctx->phys_regs[ISP_BLK_ID_ISPTOP];

	ISP_WR_REG(isp_top, REG_ISP_TOP_T, REG_0, intr_status.raw);
}

union isp_csi_intr isp_csi_intr_status(struct isp_ctx *ctx)
{
	uint64_t csi_bdg0 = ctx->phys_regs[ISP_BLK_ID_CSIBDG0];
	union isp_csi_intr status;

	status.raw = ISP_RD_REG(csi_bdg0, REG_ISP_CSI_BDG_T,
				CSI_INTERRUPT_STATUS);
	return status;
}

void isp_csi_intr_clr(struct isp_ctx *ctx, union isp_csi_intr intr_status)
{
	uint64_t csi_bdg0 = ctx->phys_regs[ISP_BLK_ID_CSIBDG0];

	ISP_WR_REG(csi_bdg0, REG_ISP_CSI_BDG_T,
		   CSI_INTERRUPT_STATUS, intr_status.raw);
}

void ispblk_ge_tun_cfg(
	struct isp_ctx *ctx,
	struct cvi_vip_isp_ge_config *cfg)
{
	uint64_t ba = ctx->phys_regs[ISP_BLK_ID_DPC0];

	ISP_WR_BITS(ba, REG_ISP_DPC_T, DPC_2, GE_ENABLE, cfg->enable);

	ISP_WR_REGS_BURST(ba, REG_ISP_DPC_T, DPC_10,
				cfg->ge_cfg, cfg->ge_cfg.DPC_10);
}

void ispblk_gamma_tun_cfg(
	struct isp_ctx *ctx,
	struct cvi_vip_isp_gamma_config *cfg)
{
	uint64_t ba = ctx->phys_regs[ISP_BLK_ID_GAMMA];
	union REG_ISP_GAMMA_PROG_DATA reg_data;
	u16 i;

	ISP_WR_BITS(ba, REG_ISP_GAMMA_T, GAMMA_CTRL, GAMMA_ENABLE, cfg->enable);

	ISP_WR_REG(ba, REG_ISP_GAMMA_T, GAMMA_PROG_MAX, cfg->lut[256]);

	for (i = 0; i < 256; i += 2) {
		reg_data.raw = 0;
		reg_data.bits.GAMMA_DATA_E = cfg->lut[i];
		reg_data.bits.GAMMA_DATA_O = cfg->lut[i + 1];
		reg_data.bits.GAMMA_W = 1;
		ISP_WR_REG(ba, REG_ISP_GAMMA_T, GAMMA_PROG_DATA,
			   reg_data.raw);
	}
}

void ispblk_ee_tun_cfg(
	struct isp_ctx *ctx,
	struct cvi_vip_isp_ee_config *cfg)
{
	uint64_t ba = ctx->phys_regs[ISP_BLK_ID_EE];
	union REG_ISP_EE_00  reg_0;
	union REG_ISP_EE_1C4 reg_1c4;
	u8 i;
	u32 raw = 0;

	reg_0.raw = ISP_RD_REG(ba, REG_ISP_EE_T, REG_00);
	reg_0.bits.EE_ENABLE = cfg->enable;
	reg_0.bits.EE_DEBUG_MODE = cfg->dbg_mode;
	reg_0.bits.EE_TOTAL_CORING = cfg->total_coring;
	reg_0.bits.EE_TOTAL_GAIN = cfg->total_gain;
	ISP_WR_REG(ba, REG_ISP_EE_T, REG_00, reg_0.raw);

	reg_1c4.raw = ISP_RD_REG(ba, REG_ISP_EE_T, REG_1C4);
	reg_1c4.bits.EE_SHTCTRL_OSHTGAIN = cfg->shtctrl_oshtgain;
	reg_1c4.bits.EE_SHTCTRL_USHTGAIN = cfg->shtctrl_ushtgain;
	ISP_WR_REG(ba, REG_ISP_EE_T, REG_1C4, reg_1c4.raw);

	for (i = 0; i < 16; i++) {
		raw = cfg->luma_coring_lut[i * 2];
		raw = (raw | (cfg->luma_coring_lut[(i * 2) + 1] << 16));
		ISP_WR_REG_OFT(ba, REG_ISP_EE_T, REG_A4, (i * 0x4), raw);
	}

	ISP_WR_REG(ba, REG_ISP_EE_T, REG_E4, cfg->luma_coring_lut[32]);

	for (i = 0; i < 8; i++) {
		raw = cfg->luma_shtctrl_lut[i * 4];
		raw = (raw | (cfg->luma_shtctrl_lut[(i * 4) + 1] << 8));
		raw = (raw | (cfg->luma_shtctrl_lut[(i * 4) + 2] << 16));
		raw = (raw | (cfg->luma_shtctrl_lut[(i * 4) + 3] << 24));
		ISP_WR_REG_OFT(ba, REG_ISP_EE_T, REG_E8, (i * 0x4), raw);

		raw = cfg->delta_shtctrl_lut[i * 4];
		raw = (raw | (cfg->delta_shtctrl_lut[(i * 4) + 1] << 8));
		raw = (raw | (cfg->delta_shtctrl_lut[(i * 4) + 2] << 16));
		raw = (raw | (cfg->delta_shtctrl_lut[(i * 4) + 3] << 24));
		ISP_WR_REG_OFT(ba, REG_ISP_EE_T, REG_10C, (i * 0x4), raw);

		raw = cfg->luma_adptctrl_lut[i * 4];
		raw = (raw | (cfg->luma_adptctrl_lut[(i * 4) + 1] << 8));
		raw = (raw | (cfg->luma_adptctrl_lut[(i * 4) + 2] << 16));
		raw = (raw | (cfg->luma_adptctrl_lut[(i * 4) + 3] << 24));
		ISP_WR_REG_OFT(ba, REG_ISP_EE_T, REG_130, (i * 0x4), raw);

		raw = cfg->delta_adptctrl_lut[i * 4];
		raw = (raw | (cfg->delta_adptctrl_lut[(i * 4) + 1] << 8));
		raw = (raw | (cfg->delta_adptctrl_lut[(i * 4) + 2] << 16));
		raw = (raw | (cfg->delta_adptctrl_lut[(i * 4) + 3] << 24));
		ISP_WR_REG_OFT(ba, REG_ISP_EE_T, REG_154, (i * 0x4), raw);
	}

	ISP_WR_REG(ba, REG_ISP_EE_T, REG_108, cfg->luma_shtctrl_lut[32]);
	ISP_WR_REG(ba, REG_ISP_EE_T, REG_12C, cfg->delta_shtctrl_lut[32]);
	ISP_WR_REG(ba, REG_ISP_EE_T, REG_150, cfg->luma_adptctrl_lut[32]);
	ISP_WR_REG(ba, REG_ISP_EE_T, REG_174, cfg->delta_adptctrl_lut[32]);

	ISP_WR_REGS_BURST(ba, REG_ISP_EE_T, REG_04,
				cfg->ee_cfg, cfg->ee_cfg.REG_04);
}

