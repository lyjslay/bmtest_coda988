#include <stdint.h>
#include "vip/vi_drv.h"

/****************************************************************************
 * Global parameters
 ****************************************************************************/
extern int vi_max_size;
/*******************************************************************************
 *	FE IPs config
 ******************************************************************************/
static void _patgen_config_timing(struct isp_ctx *ctx, enum cvi_isp_raw raw_num)
{
	uint16_t pat_height = (ctx->isp_pipe_cfg[raw_num].is_hdr_on) ?
				(ctx->isp_pipe_cfg[raw_num].csibdg_height * 2 - 1) :
				(ctx->isp_pipe_cfg[raw_num].csibdg_height - 1);
	int id = csibdg_find_hwid(raw_num);
	uintptr_t csibdg = ctx->phys_regs[id];
	uint16_t htt = 0, vtt = 0;

	ISP_WR_BITS(csibdg, REG_ISP_CSI_BDG_T, CSI_MDE_V_SIZE, VMDE_STR, 0x00);
	ISP_WR_BITS(csibdg, REG_ISP_CSI_BDG_T, CSI_MDE_V_SIZE, VMDE_STP, pat_height);
	ISP_WR_BITS(csibdg, REG_ISP_CSI_BDG_T, CSI_MDE_H_SIZE, HMDE_STR, 0x00);
	ISP_WR_BITS(csibdg, REG_ISP_CSI_BDG_T, CSI_MDE_H_SIZE, HMDE_STP,
							ctx->isp_pipe_cfg[raw_num].csibdg_width - 1);

	ISP_WR_BITS(csibdg, REG_ISP_CSI_BDG_T, CSI_FDE_V_SIZE, VFDE_STR, 0x10);
	ISP_WR_BITS(csibdg, REG_ISP_CSI_BDG_T, CSI_FDE_V_SIZE, VFDE_STP, 0x10 + pat_height);
	ISP_WR_BITS(csibdg, REG_ISP_CSI_BDG_T, CSI_FDE_H_SIZE, HFDE_STR, 0x10);
	ISP_WR_BITS(csibdg, REG_ISP_CSI_BDG_T, CSI_FDE_H_SIZE, HFDE_STP,
							0x10 + ctx->isp_pipe_cfg[raw_num].csibdg_width - 1);

	ISP_WR_BITS(csibdg, REG_ISP_CSI_BDG_T, CSI_HSYNC_CTRL, HS_STR, 4);
	ISP_WR_BITS(csibdg, REG_ISP_CSI_BDG_T, CSI_HSYNC_CTRL, HS_STP, 5);
	ISP_WR_BITS(csibdg, REG_ISP_CSI_BDG_T, CSI_VSYNC_CTRL, VS_STR, 4);
	ISP_WR_BITS(csibdg, REG_ISP_CSI_BDG_T, CSI_VSYNC_CTRL, VS_STP, 5);

	if (ctx->isp_pipe_cfg[raw_num].is_hdr_on || vi_max_size) {
		vtt = pat_height + 100;
		// htt = 600000000 / vtt / 30; //30fps	
	} else {
		vtt = 2000;
	}

	htt = ctx->isp_pipe_cfg[raw_num].csibdg_height * 2 + 0x100;

	vi_pr(VI_ERR, "HTT: %d, VTT:%d, max_fps:%d\n", htt, vtt, 600000000/htt/vtt);
	ISP_WR_BITS(csibdg, REG_ISP_CSI_BDG_T, CSI_TGEN_TT_SIZE, HTT, htt);
	ISP_WR_BITS(csibdg, REG_ISP_CSI_BDG_T, CSI_TGEN_TT_SIZE, VTT, vtt);
}

static void _patgen_config_pat(struct isp_ctx *ctx, enum cvi_isp_raw raw_num)
{
	int id = csibdg_find_hwid(raw_num);
	uintptr_t csibdg = ctx->phys_regs[id];

	ISP_WR_BITS(csibdg, REG_ISP_CSI_BDG_T, CSI_PAT_GEN_CTRL, GRA_INV, 0);
	ISP_WR_BITS(csibdg, REG_ISP_CSI_BDG_T, CSI_PAT_GEN_CTRL, AUTO_EN, 0);
	ISP_WR_BITS(csibdg, REG_ISP_CSI_BDG_T, CSI_PAT_GEN_CTRL, DITH_EN, 0);
	ISP_WR_BITS(csibdg, REG_ISP_CSI_BDG_T, CSI_PAT_GEN_CTRL, SNOW_EN, 0);
	ISP_WR_BITS(csibdg, REG_ISP_CSI_BDG_T, CSI_PAT_GEN_CTRL, FIX_MC, 0);
	ISP_WR_BITS(csibdg, REG_ISP_CSI_BDG_T, CSI_PAT_GEN_CTRL, DITH_MD, 0);
	ISP_WR_BITS(csibdg, REG_ISP_CSI_BDG_T, CSI_PAT_GEN_CTRL, BAYER_ID, 0);
	ISP_WR_BITS(csibdg, REG_ISP_CSI_BDG_T, CSI_PAT_IDX_CTRL, PAT_PRD, 0);
	if (raw_num == ISP_PRERAW0) {
		vi_pr(VI_DBG, "config pagten 0x7\n");
		ISP_WR_BITS(csibdg, REG_ISP_CSI_BDG_T, CSI_PAT_IDX_CTRL, PAT_IDX, 0x7);
	}
	else
		ISP_WR_BITS(csibdg, REG_ISP_CSI_BDG_T, CSI_PAT_IDX_CTRL, PAT_IDX, 1);

	ISP_WR_BITS(csibdg, REG_ISP_CSI_BDG_T, CSI_PAT_COLOR_0, PAT_R, 0xFFF);
	ISP_WR_BITS(csibdg, REG_ISP_CSI_BDG_T, CSI_PAT_COLOR_0, PAT_G, 0xFFF);
	ISP_WR_BITS(csibdg, REG_ISP_CSI_BDG_T, CSI_PAT_COLOR_1, PAT_B, 0xFFF);

	ISP_WR_BITS(csibdg, REG_ISP_CSI_BDG_T, CSI_BACKGROUND_COLOR_0, FDE_R, 0);
	ISP_WR_BITS(csibdg, REG_ISP_CSI_BDG_T, CSI_BACKGROUND_COLOR_0, FDE_G, 1);
	ISP_WR_BITS(csibdg, REG_ISP_CSI_BDG_T, CSI_BACKGROUND_COLOR_1, FDE_B, 2);

	ISP_WR_BITS(csibdg, REG_ISP_CSI_BDG_T, CSI_FIX_COLOR_0, MDE_R, 0x457);
	ISP_WR_BITS(csibdg, REG_ISP_CSI_BDG_T, CSI_FIX_COLOR_0, MDE_G, 0x8AE);
	ISP_WR_BITS(csibdg, REG_ISP_CSI_BDG_T, CSI_FIX_COLOR_1, MDE_B, 0xD05);
}
#ifdef PORTING_TEST
void ispblk_patgen_config_pat(struct isp_ctx *ctx, enum cvi_isp_raw raw_num, uint8_t test_case)
{
	uintptr_t csibdg;

	if (raw_num == ISP_PRERAW0) {
		csibdg = ctx->phys_regs[ISP_BLK_ID_CSIBDG0];
	} else if (raw_num == ISP_PRERAW1) {
		csibdg = ctx->phys_regs[ISP_BLK_ID_CSIBDG1];
	} else {
		csibdg = ctx->phys_regs[ISP_BLK_ID_CSIBDG2];
	}

	ISP_WR_BITS(csibdg, REG_ISP_CSI_BDG_T, CSI_PAT_GEN_CTRL, GRA_INV, 0);
	ISP_WR_BITS(csibdg, REG_ISP_CSI_BDG_T, CSI_PAT_GEN_CTRL, AUTO_EN, 0);
	ISP_WR_BITS(csibdg, REG_ISP_CSI_BDG_T, CSI_PAT_GEN_CTRL, DITH_EN, 0);
	ISP_WR_BITS(csibdg, REG_ISP_CSI_BDG_T, CSI_PAT_GEN_CTRL, SNOW_EN, 0);
	ISP_WR_BITS(csibdg, REG_ISP_CSI_BDG_T, CSI_PAT_GEN_CTRL, FIX_MC, 0);
	ISP_WR_BITS(csibdg, REG_ISP_CSI_BDG_T, CSI_PAT_GEN_CTRL, DITH_MD, 0);
	ISP_WR_BITS(csibdg, REG_ISP_CSI_BDG_T, CSI_PAT_GEN_CTRL, BAYER_ID, 0);
	ISP_WR_BITS(csibdg, REG_ISP_CSI_BDG_T, CSI_PAT_IDX_CTRL, PAT_PRD, 0);

	if (raw_num == ISP_PRERAW0)
		ISP_WR_BITS(csibdg, REG_ISP_CSI_BDG_T, CSI_PAT_IDX_CTRL, PAT_IDX, 0x7);
	else
		ISP_WR_BITS(csibdg, REG_ISP_CSI_BDG_T, CSI_PAT_IDX_CTRL, PAT_IDX, 1);

	if (test_case == 0) { //white
		ISP_WR_BITS(csibdg, REG_ISP_CSI_BDG_T, CSI_PAT_IDX_CTRL, PAT_IDX, 0x0);

		ISP_WR_BITS(csibdg, REG_ISP_CSI_BDG_T, CSI_PAT_COLOR_0, PAT_R, 0xFFF);
		ISP_WR_BITS(csibdg, REG_ISP_CSI_BDG_T, CSI_PAT_COLOR_0, PAT_G, 0xFFF);
		ISP_WR_BITS(csibdg, REG_ISP_CSI_BDG_T, CSI_PAT_COLOR_1, PAT_B, 0xFFF);

		ISP_WR_BITS(csibdg, REG_ISP_CSI_BDG_T, CSI_BACKGROUND_COLOR_0, FDE_R, 0);
		ISP_WR_BITS(csibdg, REG_ISP_CSI_BDG_T, CSI_BACKGROUND_COLOR_0, FDE_G, 1);
		ISP_WR_BITS(csibdg, REG_ISP_CSI_BDG_T, CSI_BACKGROUND_COLOR_1, FDE_B, 2);

		ISP_WR_BITS(csibdg, REG_ISP_CSI_BDG_T, CSI_FIX_COLOR_0, MDE_R, 0x457);
		ISP_WR_BITS(csibdg, REG_ISP_CSI_BDG_T, CSI_FIX_COLOR_0, MDE_G, 0x8AE);
		ISP_WR_BITS(csibdg, REG_ISP_CSI_BDG_T, CSI_FIX_COLOR_1, MDE_B, 0xD05);
	} else if (test_case == 1) { //black
		ISP_WR_BITS(csibdg, REG_ISP_CSI_BDG_T, CSI_PAT_IDX_CTRL, PAT_IDX, 0x0);

		ISP_WR_BITS(csibdg, REG_ISP_CSI_BDG_T, CSI_PAT_COLOR_0, PAT_R, 0x0);
		ISP_WR_BITS(csibdg, REG_ISP_CSI_BDG_T, CSI_PAT_COLOR_0, PAT_G, 0x0);
		ISP_WR_BITS(csibdg, REG_ISP_CSI_BDG_T, CSI_PAT_COLOR_1, PAT_B, 0x0);

		ISP_WR_BITS(csibdg, REG_ISP_CSI_BDG_T, CSI_BACKGROUND_COLOR_0, FDE_R, 0);
		ISP_WR_BITS(csibdg, REG_ISP_CSI_BDG_T, CSI_BACKGROUND_COLOR_0, FDE_G, 0);
		ISP_WR_BITS(csibdg, REG_ISP_CSI_BDG_T, CSI_BACKGROUND_COLOR_1, FDE_B, 0);

		ISP_WR_BITS(csibdg, REG_ISP_CSI_BDG_T, CSI_FIX_COLOR_0, MDE_R, 0x0);
		ISP_WR_BITS(csibdg, REG_ISP_CSI_BDG_T, CSI_FIX_COLOR_0, MDE_G, 0x0);
		ISP_WR_BITS(csibdg, REG_ISP_CSI_BDG_T, CSI_FIX_COLOR_1, MDE_B, 0x0);
	} else if (test_case == 3) { // to test ca lite
		ISP_WR_BITS(csibdg, REG_ISP_CSI_BDG_T, CSI_PAT_IDX_CTRL, PAT_IDX, 0xF);
	}
}
#endif

void ispblk_csidbg_dma_wr_en(struct isp_ctx *ctx, const enum cvi_isp_raw raw_num, const u8 chn_num, const u8 en)
{
	int id = csibdg_find_hwid(raw_num);
	uintptr_t csibdg = ctx->phys_regs[id];

	switch (chn_num) {
	case 0:
		ISP_WR_BITS(csibdg, REG_ISP_CSI_BDG_T, CSI_BDG_TOP_CTRL, CH0_DMA_WR_ENABLE, en);
		break;
	case 1:
		ISP_WR_BITS(csibdg, REG_ISP_CSI_BDG_T, CSI_BDG_TOP_CTRL, CH1_DMA_WR_ENABLE, en);
		break;
	case 2:
		ISP_WR_BITS(csibdg, REG_ISP_CSI_BDG_T, CSI_BDG_TOP_CTRL, CH2_DMA_WR_ENABLE, en);
		break;
	case 3:
		ISP_WR_BITS(csibdg, REG_ISP_CSI_BDG_T, CSI_BDG_TOP_CTRL, CH3_DMA_WR_ENABLE, en);
		break;
	default:
		break;
	}
}

void ispblk_csibdg_wdma_crop_config(struct isp_ctx *ctx, const enum cvi_isp_raw raw_num, struct vi_rect crop, u8 en)
{
	int id = csibdg_find_hwid(raw_num);
	uintptr_t csibdg = ctx->phys_regs[id];

	ISP_WR_BITS(csibdg, REG_ISP_CSI_BDG_T, WDMA_CH0_CROP_EN, ST_CH0_CROP_EN, en);
	ISP_WR_BITS(csibdg, REG_ISP_CSI_BDG_T, WDMA_CH0_HORZ_CROP, ST_CH0_HORZ_CROP_START, crop.x);
	ISP_WR_BITS(csibdg, REG_ISP_CSI_BDG_T, WDMA_CH0_HORZ_CROP, ST_CH0_HORZ_CROP_END, crop.x + crop.w - 1);
	ISP_WR_BITS(csibdg, REG_ISP_CSI_BDG_T, WDMA_CH0_VERT_CROP, ST_CH0_VERT_CROP_START, crop.y);
	ISP_WR_BITS(csibdg, REG_ISP_CSI_BDG_T, WDMA_CH0_VERT_CROP, ST_CH0_VERT_CROP_END, crop.y + crop.h - 1);

	if (ctx->isp_pipe_cfg[raw_num].is_hdr_on) {
		ISP_WR_BITS(csibdg, REG_ISP_CSI_BDG_T, WDMA_CH1_CROP_EN, ST_CH1_CROP_EN, en);
		ISP_WR_BITS(csibdg, REG_ISP_CSI_BDG_T, WDMA_CH1_HORZ_CROP, ST_CH1_HORZ_CROP_START, crop.x);
		ISP_WR_BITS(csibdg, REG_ISP_CSI_BDG_T, WDMA_CH1_HORZ_CROP, ST_CH1_HORZ_CROP_END, crop.x + crop.w - 1);
		ISP_WR_BITS(csibdg, REG_ISP_CSI_BDG_T, WDMA_CH1_VERT_CROP, ST_CH1_VERT_CROP_START, crop.y);
		ISP_WR_BITS(csibdg, REG_ISP_CSI_BDG_T, WDMA_CH1_VERT_CROP, ST_CH1_VERT_CROP_END, crop.y + crop.h - 1);
	}
}

void ispblk_csibdg_crop_update(struct isp_ctx *ctx, enum cvi_isp_raw raw_num, bool en)
{
	int id = csibdg_find_hwid(raw_num);
	uintptr_t csibdg = ctx->phys_regs[id];
	struct vi_rect crop, crop_se;

	crop.x = (ctx->isp_pipe_cfg[raw_num].crop.x == 0) ? 0 : ctx->isp_pipe_cfg[raw_num].crop.x;
	crop.y = (ctx->isp_pipe_cfg[raw_num].crop.y == 0) ? 0 : ctx->isp_pipe_cfg[raw_num].crop.y;
	crop.w = (ctx->isp_pipe_cfg[raw_num].crop.x + ctx->isp_pipe_cfg[raw_num].crop.w) - 1;
	crop.h = (ctx->isp_pipe_cfg[raw_num].crop.y + ctx->isp_pipe_cfg[raw_num].crop.h) - 1;

	ISP_WR_BITS(csibdg, REG_ISP_CSI_BDG_T, CH0_CROP_EN, CH0_CROP_EN, en);
	ISP_WR_BITS(csibdg, REG_ISP_CSI_BDG_T, CH0_HORZ_CROP, CH0_HORZ_CROP_START, crop.x);
	ISP_WR_BITS(csibdg, REG_ISP_CSI_BDG_T, CH0_HORZ_CROP, CH0_HORZ_CROP_END, crop.w);
	ISP_WR_BITS(csibdg, REG_ISP_CSI_BDG_T, CH0_VERT_CROP, CH0_VERT_CROP_START, crop.y);
	ISP_WR_BITS(csibdg, REG_ISP_CSI_BDG_T, CH0_VERT_CROP, CH0_VERT_CROP_END, crop.h);

	if (ctx->isp_pipe_cfg[raw_num].is_hdr_on) {
		crop_se.x = (ctx->isp_pipe_cfg[raw_num].crop_se.x == 0) ? 0 : ctx->isp_pipe_cfg[raw_num].crop_se.x;
		crop_se.y = (ctx->isp_pipe_cfg[raw_num].crop_se.y == 0) ? 0 : ctx->isp_pipe_cfg[raw_num].crop_se.y;
		crop_se.w = (ctx->isp_pipe_cfg[raw_num].crop_se.x + ctx->isp_pipe_cfg[raw_num].crop_se.w) - 1;
		crop_se.h = (ctx->isp_pipe_cfg[raw_num].crop_se.y + ctx->isp_pipe_cfg[raw_num].crop_se.h) - 1;

		ISP_WR_BITS(csibdg, REG_ISP_CSI_BDG_T, CH1_CROP_EN, CH1_CROP_EN, en);
		ISP_WR_BITS(csibdg, REG_ISP_CSI_BDG_T, CH1_HORZ_CROP, CH1_HORZ_CROP_START, crop_se.x);
		ISP_WR_BITS(csibdg, REG_ISP_CSI_BDG_T, CH1_HORZ_CROP, CH1_HORZ_CROP_END, crop_se.w);
		ISP_WR_BITS(csibdg, REG_ISP_CSI_BDG_T, CH1_VERT_CROP, CH1_VERT_CROP_START, crop_se.y);
		ISP_WR_BITS(csibdg, REG_ISP_CSI_BDG_T, CH1_VERT_CROP, CH1_VERT_CROP_END, crop_se.h);
	}
}

int ispblk_csibdg_config(struct isp_ctx *ctx, enum cvi_isp_raw raw_num)
{
	int id = csibdg_find_hwid(raw_num);
	uintptr_t csibdg = ctx->phys_regs[id];
	uint8_t csi_mode = 0;
	union REG_ISP_CSI_BDG_TOP_CTRL top_ctrl;
	union REG_ISP_CSI_BDG_INTERRUPT_CTRL int_ctrl;

	top_ctrl.raw = ISP_RD_REG(csibdg, REG_ISP_CSI_BDG_T, CSI_BDG_TOP_CTRL);
	top_ctrl.bits.RESET_MODE	= 0;
	top_ctrl.bits.ABORT_MODE	= 0;
	top_ctrl.bits.CSI_IN_FORMAT	= 0;
	top_ctrl.bits.CH_NUM		= ctx->isp_pipe_cfg[raw_num].is_hdr_on;

	if (_is_fe_post_offline(ctx)) { //fe->dram->post
		top_ctrl.bits.CH0_DMA_WR_ENABLE = 1;
		top_ctrl.bits.CH1_DMA_WR_ENABLE = ctx->isp_pipe_cfg[raw_num].is_hdr_on;
	} else {
		top_ctrl.bits.CH0_DMA_WR_ENABLE = 0;
		top_ctrl.bits.CH1_DMA_WR_ENABLE = 0;
	}

	if (ctx->isp_pipe_cfg[raw_num].is_patgen_en) {
		csi_mode = 3;
		top_ctrl.bits.PXL_DATA_SEL = 1;
		ISP_WR_BITS(csibdg, REG_ISP_CSI_BDG_T, CSI_PAT_GEN_CTRL, PAT_EN, 1);

		_patgen_config_timing(ctx, raw_num);
		_patgen_config_pat(ctx, raw_num);
	} else {
		top_ctrl.bits.PXL_DATA_SEL = 0;

		csi_mode = 1;
	}

	top_ctrl.bits.CSI_MODE	= csi_mode;
	top_ctrl.bits.VS_MODE	= 0;
	ISP_WR_REG(csibdg, REG_ISP_CSI_BDG_T, CSI_BDG_TOP_CTRL, top_ctrl.raw);

	ISP_WR_BITS(csibdg, REG_ISP_CSI_BDG_T, CH0_SIZE, CH0_FRAME_WIDTHM1,
					ctx->isp_pipe_cfg[raw_num].csibdg_width - 1);
	ISP_WR_BITS(csibdg, REG_ISP_CSI_BDG_T, CH0_SIZE, CH0_FRAME_HEIGHTM1,
					ctx->isp_pipe_cfg[raw_num].csibdg_height - 1);

	ISP_WR_BITS(csibdg, REG_ISP_CSI_BDG_T, CH1_SIZE, CH1_FRAME_WIDTHM1,
					ctx->isp_pipe_cfg[raw_num].csibdg_width - 1);
	ISP_WR_BITS(csibdg, REG_ISP_CSI_BDG_T, CH1_SIZE, CH1_FRAME_HEIGHTM1,
					ctx->isp_pipe_cfg[raw_num].csibdg_height - 1);

	ISP_WR_BITS(csibdg, REG_ISP_CSI_BDG_T, CH2_SIZE, CH2_FRAME_WIDTHM1,
					ctx->isp_pipe_cfg[raw_num].csibdg_width - 1);
	ISP_WR_BITS(csibdg, REG_ISP_CSI_BDG_T, CH2_SIZE, CH2_FRAME_HEIGHTM1,
					ctx->isp_pipe_cfg[raw_num].csibdg_height - 1);

	ISP_WR_BITS(csibdg, REG_ISP_CSI_BDG_T, CH3_SIZE, CH3_FRAME_WIDTHM1,
					ctx->isp_pipe_cfg[raw_num].csibdg_width - 1);
	ISP_WR_BITS(csibdg, REG_ISP_CSI_BDG_T, CH3_SIZE, CH3_FRAME_HEIGHTM1,
					ctx->isp_pipe_cfg[raw_num].csibdg_height - 1);

	if (ctx->is_dpcm_on) {
		union REG_ISP_CSI_BDG_DMA_DPCM_MODE dpcm;

		dpcm.bits.DMA_ST_DPCM_MODE = 0x7; //12->6 mode
		dpcm.bits.DPCM_XSTR = 8191;
		ISP_WR_REG(csibdg, REG_ISP_CSI_BDG_T, CSI_BDG_DMA_DPCM_MODE, dpcm.raw);
	} else {
		ISP_WR_REG(csibdg, REG_ISP_CSI_BDG_T, CSI_BDG_DMA_DPCM_MODE, 0);
	}

	int_ctrl.raw = ISP_RD_REG(csibdg, REG_ISP_CSI_BDG_T, CSI_BDG_INTERRUPT_CTRL);
	int_ctrl.bits.CH0_VS_INT_EN		= 1;
	int_ctrl.bits.CH1_VS_INT_EN		= 1;
	int_ctrl.bits.CH2_VS_INT_EN		= 1;
	int_ctrl.bits.CH3_VS_INT_EN		= 1;
	int_ctrl.bits.CH0_TRIG_INT_EN		= 0;
	int_ctrl.bits.CH1_TRIG_INT_EN		= 0;
	int_ctrl.bits.CH2_TRIG_INT_EN		= 0;
	int_ctrl.bits.CH3_TRIG_INT_EN		= 0;
	ISP_WR_REG(csibdg, REG_ISP_CSI_BDG_T, CSI_BDG_INTERRUPT_CTRL, int_ctrl.raw);

	return 0;
}

uint32_t ispblk_csibdg_chn_dbg(struct isp_ctx *ctx, enum cvi_isp_raw raw_num, enum cvi_isp_pre_chn_num chn)
{
	return 0;
}

#if 0
void ispblk_csibdg_yuv_bypass_config(struct isp_ctx *ctx, const enum cvi_isp_raw raw_num)
{
	int id = csibdg_find_hwid(raw_num);
	uintptr_t csibdg = ctx->phys_regs[id];
	union REG_ISP_CSI_BDG_TOP_CTRL csibdg_ctrl;
	u8 chn_num = ctx->isp_pipe_cfg[raw_num].muxMode;

	/*
	 * MIPI--->MUX use FE0 or FE1
	 *      |->No Mux use FE0 or FE1 -> chn_num = 0
	 * BT----->MUX use csibdg lite
	 *      |->No Mux use FE0 or FE1 -> chn_num = 0
	 */
	vi_pr(VI_DBG, "raw[%d], chn_num[%d], infMode[%d]\n", raw_num, chn_num, ctx->isp_pipe_cfg[raw_num].infMode);

	if (chn_num == 0 || (ctx->isp_pipe_cfg[raw_num].infMode >= VI_MODE_MIPI_YUV420_NORMAL &&
		ctx->isp_pipe_cfg[raw_num].infMode <= VI_MODE_MIPI_YUV422)) {

		csibdg_ctrl.raw = ISP_RD_REG(csibdg, REG_ISP_CSI_BDG_T, CSI_BDG_TOP_CTRL);
		csibdg_ctrl.bits.RESET_MODE		= 0;
		csibdg_ctrl.bits.ABORT_MODE		= 0;
		csibdg_ctrl.bits.CSI_MODE		= 1;
		csibdg_ctrl.bits.CSI_IN_FORMAT		= 1;
		csibdg_ctrl.bits.CSI_IN_YUV_FORMAT	= 0;
		csibdg_ctrl.bits.Y_ONLY			= 0;
		csibdg_ctrl.bits.YUV2BAY_ENABLE		= 0;
		csibdg_ctrl.bits.CH_NUM			= chn_num;
		csibdg_ctrl.bits.MULTI_CH_FRAME_SYNC_EN	= 0;
		ISP_WR_REG(csibdg, REG_ISP_CSI_BDG_T, CSI_BDG_TOP_CTRL, csibdg_ctrl.raw);

	} else if (chn_num > 0 && ctx->isp_pipe_cfg[raw_num].infMode >= VI_MODE_BT656 &&
		ctx->isp_pipe_cfg[raw_num].infMode <= VI_MODE_BT1120_INTERLEAVED) {
		csibdg = ctx->phys_regs[ISP_BLK_ID_CSIBDG_LITE];
	}

	ISP_WR_BITS(csibdg, REG_ISP_CSI_BDG_T, CSI_BDG_TOP_CTRL, VS_MODE, 0);

	if (_is_fe_post_offline(ctx)) // sensor->fe->dram
		ISP_WR_BITS(csibdg, REG_ISP_CSI_BDG_T, CSI_BDG_TOP_CTRL, CH0_DMA_WR_ENABLE, true);
	else // sensor->fe->yuvtop
		ISP_WR_BITS(csibdg, REG_ISP_CSI_BDG_T, CSI_BDG_TOP_CTRL, CH0_DMA_WR_ENABLE, false);

	ISP_WR_BITS(csibdg, REG_ISP_CSI_BDG_T, CSI_BDG_TOP_CTRL, CH1_DMA_WR_ENABLE, (chn_num > 0) ? true : false);
	ISP_WR_BITS(csibdg, REG_ISP_CSI_BDG_T, CSI_BDG_TOP_CTRL, CH2_DMA_WR_ENABLE, (chn_num > 1) ? true : false);
	ISP_WR_BITS(csibdg, REG_ISP_CSI_BDG_T, CSI_BDG_TOP_CTRL, CH3_DMA_WR_ENABLE, (chn_num > 2) ? true : false);

	if (ctx->isp_pipe_cfg[raw_num].is_422_to_420) {
		ISP_WR_BITS(csibdg, REG_ISP_CSI_BDG_T, CSI_BDG_TOP_CTRL, CH0_DMA_420_WR_ENABLE, true);
		ISP_WR_BITS(csibdg, REG_ISP_CSI_BDG_T, CSI_BDG_TOP_CTRL, CH1_DMA_420_WR_ENABLE,
									(chn_num > 0) ? true : false);
		//only chn0 support avg mode
		ISP_WR_BITS(csibdg, REG_ISP_CSI_BDG_T, CSI_BDG_DMA_DPCM_MODE, AVG_MODE, true);
	}

	ISP_WR_BITS(csibdg, REG_ISP_CSI_BDG_T, CH0_SIZE, CH0_FRAME_WIDTHM1,
					ctx->isp_pipe_cfg[raw_num].csibdg_width - 1);
	ISP_WR_BITS(csibdg, REG_ISP_CSI_BDG_T, CH0_SIZE, CH0_FRAME_HEIGHTM1,
					ctx->isp_pipe_cfg[raw_num].csibdg_height - 1);

	ISP_WR_BITS(csibdg, REG_ISP_CSI_BDG_T, CH1_SIZE, CH1_FRAME_WIDTHM1,
					ctx->isp_pipe_cfg[raw_num].csibdg_width - 1);
	ISP_WR_BITS(csibdg, REG_ISP_CSI_BDG_T, CH1_SIZE, CH1_FRAME_HEIGHTM1,
					ctx->isp_pipe_cfg[raw_num].csibdg_height - 1);

	ISP_WR_BITS(csibdg, REG_ISP_CSI_BDG_T, CH2_SIZE, CH2_FRAME_WIDTHM1,
					ctx->isp_pipe_cfg[raw_num].csibdg_width - 1);
	ISP_WR_BITS(csibdg, REG_ISP_CSI_BDG_T, CH2_SIZE, CH2_FRAME_HEIGHTM1,
					ctx->isp_pipe_cfg[raw_num].csibdg_height - 1);

	ISP_WR_BITS(csibdg, REG_ISP_CSI_BDG_T, CH3_SIZE, CH3_FRAME_WIDTHM1,
					ctx->isp_pipe_cfg[raw_num].csibdg_width - 1);
	ISP_WR_BITS(csibdg, REG_ISP_CSI_BDG_T, CH3_SIZE, CH3_FRAME_HEIGHTM1,
					ctx->isp_pipe_cfg[raw_num].csibdg_height - 1);
}
#endif