#include <vip/vi_drv.h>

/****************************************************************************
 * Global parameters
 ****************************************************************************/

/*******************************************************************************
 *	RAW IPs config
 ******************************************************************************/
#define F_D (15)
void ispblk_lsc_config(struct isp_ctx *ctx, enum cvi_isp_raw raw_num, bool is_le, bool en)
{
	int id = lsc_find_hwid(raw_num, is_le);;
	uintptr_t lsc;
	int width;
	int height;
	int mesh_num = 37;
	int InnerBlkX = mesh_num - 1 - 2;
	int InnerBlkY = mesh_num - 1 - 2;
	int mesh_x_coord_unit;
	int mesh_y_coord_unit;
	u32 reg_lsc_xstep;
	u32 reg_lsc_ystep;
	int image_w_in_mesh_unit;
	int image_h_in_mesh_unit;
	int OuterBlkX;
	int OuterBlkY;
	u32 reg_lsc_imgx0;
	u32 reg_lsc_imgy0;

	union REG_ISP_LSC_SC_WRAP_0 lsc_ctrl;
	union REG_ISP_LSC_SC_WRAP_3 inter_p;
	union REG_ISP_LSC_SC_WRAP_11 lsc_dma;

	width = ctx->isp_pipe_cfg[raw_num].csibdg_width;
	height = ctx->isp_pipe_cfg[raw_num].csibdg_height;

	lsc = ctx->phys_regs[id];

	// vi_pr(VI_INFO, "en:%d\n", en);

	lsc_ctrl.raw = ISP_RD_REG(lsc, REG_ISP_LSC_T, SC_WRAP_0);
	lsc_ctrl.bits.LSC_ENABLE = en;
	ISP_WR_REG(lsc, REG_ISP_LSC_T, SC_WRAP_0, lsc_ctrl.raw);
	//disable dma
	lsc_dma.raw = ISP_RD_REG(lsc, REG_ISP_LSC_T, SC_WRAP_11);
	lsc_dma.bits.DMA_ENABLE_LSC = en;
	ISP_WR_REG(lsc, REG_ISP_LSC_T, SC_WRAP_11, lsc_dma.raw);

	if (!en)
		return;

	mesh_x_coord_unit = (InnerBlkX * (1 << F_D)) / width;
	mesh_y_coord_unit = (InnerBlkY * (1 << F_D)) / height;
	reg_lsc_xstep = mesh_x_coord_unit + 1;
	reg_lsc_ystep = mesh_y_coord_unit + 1;
	image_w_in_mesh_unit = width * reg_lsc_xstep;
	image_h_in_mesh_unit = height * reg_lsc_ystep;
	OuterBlkX = InnerBlkX + 2;
	OuterBlkY = InnerBlkY + 2;
	reg_lsc_imgx0 = (OuterBlkX * (1 << F_D) - image_w_in_mesh_unit) / 2;
	reg_lsc_imgy0 = (OuterBlkY * (1 << F_D) - image_h_in_mesh_unit) / 2;

	ISP_WR_BITS(lsc, REG_ISP_LSC_T, SC_WRAP_13, BLC_OFFSET_R, 0x100);
	ISP_WR_BITS(lsc, REG_ISP_LSC_T, SC_WRAP_13, BLC_OFFSET_GR, 0x100);
	ISP_WR_BITS(lsc, REG_ISP_LSC_T, SC_WRAP_14, BLC_OFFSET_GB, 0x100);
	ISP_WR_BITS(lsc, REG_ISP_LSC_T, SC_WRAP_14, BLC_OFFSET_B, 0x100);

	//Tuning
	ISP_WR_BITS(lsc, REG_ISP_LSC_T, SC_WRAP_4, LSC_STRENGTH, 0xfff);
	ISP_WR_BITS(lsc, REG_ISP_LSC_T, SC_WRAP_4, LSC_GAIN_3P9_0_4P8_1, 0);
	ISP_WR_BITS(lsc, REG_ISP_LSC_T, SC_WRAP_4, LSC_GAIN_BICUBIC_0_BILINEAR_1, 1);
	ISP_WR_BITS(lsc, REG_ISP_LSC_T, SC_WRAP_4, LSC_BOUNDARY_INTERPOLATION_MODE, 1);
	ISP_WR_REG(lsc, REG_ISP_LSC_T, SC_WRAP_5, reg_lsc_imgx0);
	ISP_WR_REG(lsc, REG_ISP_LSC_T, SC_WRAP_6, reg_lsc_imgy0);
	ISP_WR_BITS(lsc, REG_ISP_LSC_T, SC_WRAP_7, LSC_XSTEP, reg_lsc_xstep);
	ISP_WR_BITS(lsc, REG_ISP_LSC_T, SC_WRAP_7, LSC_YSTEP, reg_lsc_ystep);
	ISP_WR_REG(lsc, REG_ISP_LSC_T, SC_WRAP_8, reg_lsc_imgx0);
	ISP_WR_REG(lsc, REG_ISP_LSC_T, SC_WRAP_9, reg_lsc_imgy0);
	ISP_WR_REG(lsc, REG_ISP_LSC_T, SC_WRAP_10, 0x0);

	inter_p.raw = ISP_RD_REG(lsc, REG_ISP_LSC_T, SC_WRAP_3);
	inter_p.bits.LSC_BOUNDARY_INTERPOLATION_LF_RANGE = 0x3;
	inter_p.bits.LSC_BOUNDARY_INTERPOLATION_UP_RANGE = 0x4;
	inter_p.bits.LSC_BOUNDARY_INTERPOLATION_RT_RANGE = 0x1f;
	inter_p.bits.LSC_BOUNDARY_INTERPOLATION_DN_RANGE = 0x1c;
	ISP_WR_REG(lsc, REG_ISP_LSC_T, SC_WRAP_3, inter_p.raw);

	lsc_ctrl.raw = ISP_RD_REG(lsc, REG_ISP_LSC_T, SC_WRAP_0);
	lsc_ctrl.bits.LSC_ENABLE = en;
	ISP_WR_REG(lsc, REG_ISP_LSC_T, SC_WRAP_0, lsc_ctrl.raw);
}

void ispblk_aehist_config(struct isp_ctx *ctx, enum cvi_isp_raw raw_num, bool en)
{
	int id = ae_find_hwid(raw_num);;
	uintptr_t ba = ctx->phys_regs[id];
	uint8_t num_x = 34, num_y = 30;
	uint8_t sub_window_w = 0, sub_window_h = 0;
	union REG_ISP_AE_HIST_STS_AE0_HIST_ENABLE ae_enable;

	ae_enable.raw = ISP_RD_REG(ba, REG_ISP_AE_HIST_T, STS_AE0_HIST_ENABLE);
	ae_enable.bits.STS_AE0_HIST_ENABLE	= en;
	ae_enable.bits.AE0_GAIN_ENABLE	= en;
	ae_enable.bits.HIST0_ENABLE	= en;
	ae_enable.bits.HIST0_GAIN_ENABLE = en;
	ISP_WR_REG(ba, REG_ISP_AE_HIST_T, STS_AE0_HIST_ENABLE, ae_enable.raw);
	ISP_WR_BITS(ba, REG_ISP_AE_HIST_T, DMI_ENABLE, DMI_ENABLE, en);
	if (!en)
		return;

	sub_window_w = ctx->img_width / num_x;
	sub_window_h = ctx->img_height / num_y;

	ISP_WR_REG(ba, REG_ISP_AE_HIST_T, STS_AE_NUMXM1, num_x - 1);
	ISP_WR_REG(ba, REG_ISP_AE_HIST_T, STS_AE_NUMYM1, num_y - 1);
	ISP_WR_REG(ba, REG_ISP_AE_HIST_T, STS_AE_WIDTH, sub_window_w);
	ISP_WR_REG(ba, REG_ISP_AE_HIST_T, STS_AE_HEIGHT, sub_window_h);
}

void ispblk_aehist_reset(struct isp_ctx *ctx, int blk_id)
{
	uintptr_t ba = ctx->phys_regs[blk_id];

	ISP_WR_REG(ba, REG_ISP_AE_HIST_T, AE_HIST_GRACE_RESET, 1);
	ISP_WR_REG(ba, REG_ISP_AE_HIST_T, AE_HIST_GRACE_RESET, 0);
}

void ispblk_bnr_lite_config(struct isp_ctx *ctx, bool en)
{
	uintptr_t bnr_lite = ctx->phys_regs[ISP_BLK_ID_BNR_LITE];

	ISP_WR_REG(bnr_lite, REG_ISP_SE_BNR_T, U1_SE_BNR_ENABLE, en);
}

void ispblk_blc_set_offset(struct isp_ctx *ctx,
			   uint16_t roffset, uint16_t groffset,
			   uint16_t gboffset, uint16_t boffset)
{
	uintptr_t blc;
	int id = ctx->is_hdr_on ? ISP_BLK_ID_BLC_DG_WB0 : ISP_BLK_ID_BLC_DG_WB1;

	blc = ctx->phys_regs[id];

	ISP_WR_BITS(blc, REG_ISP_BLC_DG_WB_T, BLC_LE_OFFSET0, BLC_LE_OFFSET_R, roffset);
	ISP_WR_BITS(blc, REG_ISP_BLC_DG_WB_T, BLC_LE_OFFSET0, BLC_LE_OFFSET_GR, groffset);
	ISP_WR_BITS(blc, REG_ISP_BLC_DG_WB_T, BLC_LE_OFFSET1, BLC_LE_OFFSET_GB, gboffset);
	ISP_WR_BITS(blc, REG_ISP_BLC_DG_WB_T, BLC_LE_OFFSET1, BLC_LE_OFFSET_B, boffset);
}

void ispblk_blc_set_2ndoffset(struct isp_ctx *ctx,
				uint16_t roffset, uint16_t groffset,
				uint16_t gboffset, uint16_t boffset)
{
	uintptr_t blc;
	int id = ctx->is_hdr_on ? ISP_BLK_ID_BLC_DG_WB0 : ISP_BLK_ID_BLC_DG_WB1;

	blc = ctx->phys_regs[id];
	ISP_WR_BITS(blc, REG_ISP_BLC_DG_WB_T, BLC_LE_OFFSET_2ND0, BLC_LE_2NDOFFSET_R, roffset);
	ISP_WR_BITS(blc, REG_ISP_BLC_DG_WB_T, BLC_LE_OFFSET_2ND0, BLC_LE_2NDOFFSET_GR, groffset);
	ISP_WR_BITS(blc, REG_ISP_BLC_DG_WB_T, BLC_LE_OFFSET_2ND1, BLC_LE_2NDOFFSET_GB, gboffset);
	ISP_WR_BITS(blc, REG_ISP_BLC_DG_WB_T, BLC_LE_OFFSET_2ND1, BLC_LE_2NDOFFSET_B, boffset);
}

void ispblk_blc_set_gain(struct isp_ctx *ctx,
				uint16_t rgain, uint16_t grgain,
				uint16_t gbgain, uint16_t bgain)
{
	uintptr_t blc;
	int id = ctx->is_hdr_on ? ISP_BLK_ID_BLC_DG_WB0 : ISP_BLK_ID_BLC_DG_WB1;

	blc = ctx->phys_regs[id];

	ISP_WR_BITS(blc, REG_ISP_BLC_DG_WB_T, BLC_LE_GAIN0, BLC_LE_GAIN_R, rgain);
	ISP_WR_BITS(blc, REG_ISP_BLC_DG_WB_T, BLC_LE_GAIN0, BLC_LE_GAIN_GR, grgain);
	ISP_WR_BITS(blc, REG_ISP_BLC_DG_WB_T, BLC_LE_GAIN1, BLC_LE_GAIN_GB, gbgain);
	ISP_WR_BITS(blc, REG_ISP_BLC_DG_WB_T, BLC_LE_GAIN1, BLC_LE_GAIN_B, bgain);

	if (ctx->is_hdr_on) {
		ISP_WR_BITS(blc, REG_ISP_BLC_DG_WB_T, BLC_SE_GAIN0, BLC_SE_GAIN_R, rgain);
		ISP_WR_BITS(blc, REG_ISP_BLC_DG_WB_T, BLC_SE_GAIN0, BLC_SE_GAIN_GR, grgain);
		ISP_WR_BITS(blc, REG_ISP_BLC_DG_WB_T, BLC_SE_GAIN1, BLC_SE_GAIN_GB, gbgain);
		ISP_WR_BITS(blc, REG_ISP_BLC_DG_WB_T, BLC_SE_GAIN1, BLC_SE_GAIN_B, bgain);
	}
}

void ispblk_blc_enable(struct isp_ctx *ctx, bool en)
{
	uintptr_t blc;
	int id = ctx->is_hdr_on ? ISP_BLK_ID_BLC_DG_WB0 : ISP_BLK_ID_BLC_DG_WB1;

	blc = ctx->phys_regs[id];

	ISP_WR_BITS(blc, REG_ISP_BLC_DG_WB_T, BASE_CONFIG, BLC_LE_ENABLE, en);
}

int ispblk_blc_config(struct isp_ctx *ctx, bool en)
{
	int id = ctx->is_hdr_on ? ISP_BLK_ID_BLC_DG_WB0 : ISP_BLK_ID_BLC_DG_WB1;
	uintptr_t blc = ctx->phys_regs[id];

	ISP_WR_BITS(blc, REG_ISP_BLC_DG_WB_T, BASE_CONFIG, BLC_LE_ENABLE, en);

	ISP_WR_BITS(blc, REG_ISP_BLC_DG_WB_T, BLC_LE_OFFSET0, BLC_LE_OFFSET_R, 511);
	ISP_WR_BITS(blc, REG_ISP_BLC_DG_WB_T, BLC_LE_OFFSET0, BLC_LE_OFFSET_GR, 511);
	ISP_WR_BITS(blc, REG_ISP_BLC_DG_WB_T, BLC_LE_OFFSET1, BLC_LE_OFFSET_GB, 511);
	ISP_WR_BITS(blc, REG_ISP_BLC_DG_WB_T, BLC_LE_OFFSET1, BLC_LE_OFFSET_B, 511);

	ISP_WR_BITS(blc, REG_ISP_BLC_DG_WB_T, BLC_LE_GAIN0, BLC_LE_GAIN_R, 0x40f);
	ISP_WR_BITS(blc, REG_ISP_BLC_DG_WB_T, BLC_LE_GAIN0, BLC_LE_GAIN_GR, 0x419);
	ISP_WR_BITS(blc, REG_ISP_BLC_DG_WB_T, BLC_LE_GAIN1, BLC_LE_GAIN_GB, 0x419);
	ISP_WR_BITS(blc, REG_ISP_BLC_DG_WB_T, BLC_LE_GAIN1, BLC_LE_GAIN_B, 0x405);

	ISP_WR_BITS(blc, REG_ISP_BLC_DG_WB_T, BLC_LE_OFFSET_2ND0, BLC_LE_2NDOFFSET_R, 0);
	ISP_WR_BITS(blc, REG_ISP_BLC_DG_WB_T, BLC_LE_OFFSET_2ND0, BLC_LE_2NDOFFSET_GR, 0);
	ISP_WR_BITS(blc, REG_ISP_BLC_DG_WB_T, BLC_LE_OFFSET_2ND1, BLC_LE_2NDOFFSET_GB, 0);
	ISP_WR_BITS(blc, REG_ISP_BLC_DG_WB_T, BLC_LE_OFFSET_2ND1, BLC_LE_2NDOFFSET_B, 0);

	return 0;
}

int ispblk_wbg_config(struct isp_ctx *ctx, uint16_t rgain, uint16_t ggain, uint16_t bgain)
{
	uintptr_t wbg;
	int id = ctx->is_hdr_on ? ISP_BLK_ID_BLC_DG_WB0 : ISP_BLK_ID_BLC_DG_WB1;
	wbg = ctx->phys_regs[id];

	ISP_WR_BITS(wbg, REG_ISP_BLC_DG_WB_T, WBG_LE_GAIN0, WBG_LE_RGAIN, rgain);
	ISP_WR_BITS(wbg, REG_ISP_BLC_DG_WB_T, WBG_LE_GAIN0, WBG_LE_GGAIN, ggain);
	ISP_WR_BITS(wbg, REG_ISP_BLC_DG_WB_T, WBG_LE_GAIN1, WBG_LE_BGAIN, bgain);

	if (ctx->is_hdr_on) {
		ISP_WR_BITS(wbg, REG_ISP_BLC_DG_WB_T, WBG_SE_GAIN0, WBG_SE_RGAIN, rgain);
		ISP_WR_BITS(wbg, REG_ISP_BLC_DG_WB_T, WBG_SE_GAIN0, WBG_SE_GGAIN, ggain);
		ISP_WR_BITS(wbg, REG_ISP_BLC_DG_WB_T, WBG_SE_GAIN1, WBG_SE_BGAIN, bgain);
	}

	return 0;
}

int ispblk_wbg_enable(struct isp_ctx *ctx, bool en)
{
	uintptr_t wbg;
	int id = ctx->is_hdr_on ? ISP_BLK_ID_BLC_DG_WB0 : ISP_BLK_ID_BLC_DG_WB1;

	wbg = ctx->phys_regs[id];

	ISP_WR_BITS(wbg, REG_ISP_BLC_DG_WB_T, BASE_CONFIG, WBG_LE_ENABLE, en);
	if (ctx->is_hdr_on)
		ISP_WR_BITS(wbg, REG_ISP_BLC_DG_WB_T, BASE_CONFIG, WBG_SE_ENABLE, en);

	return 0;
}

void ispblk_fusion_hdr_cfg(struct isp_ctx *ctx, enum cvi_isp_raw raw_num)
{
	uintptr_t fusion = ctx->phys_regs[ISP_BLK_ID_FUSION];

	if (ctx->isp_pipe_cfg[raw_num].is_hdr_on)
		ISP_WR_BITS(fusion, REG_FUSION_T, CTRL_REG, U1_FUSION_EN, true);
	else
		ISP_WR_BITS(fusion, REG_FUSION_T, CTRL_REG, U1_FUSION_EN, false);
}

void ispblk_fusion_config(struct isp_ctx *ctx, bool enable, int out_sel)
{
	uintptr_t fusion = ctx->phys_regs[ISP_BLK_ID_FUSION];
	union REG_FUSION_CTRL_REG reg_ctrl;

	reg_ctrl.raw = ISP_RD_REG(fusion, REG_FUSION_T, CTRL_REG);
	reg_ctrl.bits.U1_FUSION_EN = enable;
	reg_ctrl.bits.U2_FUSION_FNUM = 1;
	reg_ctrl.bits.U2_FUSION_MODE = out_sel;
	ISP_WR_REG(fusion, REG_FUSION_T, CTRL_REG, reg_ctrl.raw);
}

void ispblk_mapcurve_config(struct isp_ctx *ctx, bool enable)
{
	uintptr_t fusion = ctx->phys_regs[ISP_BLK_ID_MAPCURVE];
	union REG_ISP_MAP_CURVE_01 reg_ctrl;

	reg_ctrl.raw = ISP_RD_REG(fusion, REG_ISP_MAP_CURVE_T, REG_01);
	reg_ctrl.bits.U1_FCURVE16_EN = enable;
	ISP_WR_REG(fusion, REG_ISP_MAP_CURVE_T, REG_01, reg_ctrl.raw);
}

void ispblk_dpc_config(struct isp_ctx *ctx, bool enable, uint8_t test_case)
{
	uintptr_t dpc = ctx->phys_regs[ISP_BLK_ID_DPC];
	union REG_ISP_DPC_BASE_CONFIG cfg;

	cfg.raw = ISP_RD_REG(dpc, REG_ISP_DPC_T, BASE_CONFIG);
	cfg.bits.DPC_ENABLE = enable;
	cfg.bits.GE_ENABLE = enable;
	ISP_WR_REG(dpc, REG_ISP_DPC_T, BASE_CONFIG, cfg.raw);

	if (test_case == 1) { // test static dpc
		cfg.raw = ISP_RD_REG(dpc, REG_ISP_DPC_T, BASE_CONFIG);
		cfg.bits.SPC_ENABLE = 1;
		ISP_WR_REG(dpc, REG_ISP_DPC_T, BASE_CONFIG, cfg.raw);
	}
}

/**
 * ispblk_dpc_set_static - set defect pixels for static dpc.
 *
 * @param ctx: global settings
 * @param offset: mem-offset for 4k tile
 * @param bps: array of defect pixels. [23:12]-row, [11:0]-col.
 * @param count: number of defect pixels.
 */

void ispblk_dpc_set_static(struct isp_ctx *ctx, uint16_t offset, uint32_t *bps, uint16_t count)
{
	uintptr_t dpc = ctx->phys_regs[ISP_BLK_ID_DPC];
	uint16_t i = 0;

	vi_pr(VI_INFO, "set count:%d\n", count);

	ISP_WR_BITS(dpc, REG_ISP_DPC_T, SPC_CONFIG, DPC_MEM_SW_MODE, 1);

	// ISP_WR_BITS(dpc, REG_ISP_DPC_T, WRITE_MEM_ST_ADDR, DPC_BP_MEM_ST_ADDR_W, 1);

	for (i = 0; (i < count) && (i < 4096); ++i)
		ISP_WR_REG(dpc, REG_ISP_DPC_T, WRITE_MEM, 0x80000000 | *(bps + i));

	ISP_WR_BITS(dpc, REG_ISP_DPC_T, SPC_CONFIG, DPC_MEM_SW_MODE, 0);
}

void ispblk_af_config(struct isp_ctx *ctx, bool enable)
{
	uintptr_t sts = ctx->phys_regs[ISP_BLK_ID_AF];
	int numx = 17, numy = 15;

	ISP_WR_BITS(sts, REG_ISP_AF_T, KICKOFF, AF_ENABLE, enable);
	ISP_WR_BITS(sts, REG_ISP_AF_T, DMI_ENABLE, DMI_ENABLE, enable);

	if (!enable)
		return;

	ISP_WR_BITS(sts, REG_ISP_AF_T, ENABLES, AF_HORIZON_0_ENABLE, enable);
	ISP_WR_BITS(sts, REG_ISP_AF_T, ENABLES, AF_HORIZON_1_ENABLE, enable);
	ISP_WR_BITS(sts, REG_ISP_AF_T, ENABLES, AF_VERTICAL_0_ENABLE, enable);
	ISP_WR_BITS(sts, REG_ISP_AF_T, ENABLES, AF_HLC_ENABLE, 1);
	ISP_WR_REG(sts, REG_ISP_AF_T, BYPASS, !enable);

	// 8 <= offset_x <= img_width - 8
	ISP_WR_BITS(sts, REG_ISP_AF_T, OFFSET_X, AF_OFFSET_X, 0x8);
	// 2 <= offset_y <= img_height - 2
	ISP_WR_BITS(sts, REG_ISP_AF_T, OFFSET_X, AF_OFFSET_Y, 0x2);
	// block_width >= 15
	ISP_WR_REG(sts, REG_ISP_AF_T, BLOCK_WIDTH, (ctx->img_width - 16) / numx);
	// block_height >= 15
	ISP_WR_REG(sts, REG_ISP_AF_T, BLOCK_HEIGHT, (ctx->img_height - 4) / numy);
	ISP_WR_REG(sts, REG_ISP_AF_T, BLOCK_NUM_X, 17);
	ISP_WR_REG(sts, REG_ISP_AF_T, BLOCK_NUM_Y, 15);

	ISP_WR_REG(sts, REG_ISP_AF_T, HIGH_Y_THRE, 0x258);

	ISP_WR_REG(sts, REG_ISP_AF_T, IMAGE_WIDTH, ctx->img_width - 1);
	ISP_WR_BITS(sts, REG_ISP_AF_T, MXN_IMAGE_WIDTH_M1, AF_MXN_IMAGE_WIDTH, ctx->img_width - 1);
	ISP_WR_BITS(sts, REG_ISP_AF_T, MXN_IMAGE_WIDTH_M1, AF_MXN_IMAGE_HEIGHT, ctx->img_height - 1);
}

void ispblk_bnr_config(struct isp_ctx *ctx, bool en)
{
	uintptr_t bnr = ctx->phys_regs[ISP_BLK_ID_BNR];
	union REG_ISP_BNR_00 reg_ctrl;

	// Init once tuning
	// _bnr_init(ctx);

	reg_ctrl.raw = ISP_RD_REG(bnr, REG_ISP_BNR_T, REG_00);
	reg_ctrl.bits.U1_BNR_ENABLE = en;
	ISP_WR_REG(bnr, REG_ISP_BNR_T, REG_00, reg_ctrl.raw);
}

void ispblk_lscr_config(struct isp_ctx *ctx, bool en)
{
	uintptr_t lscr = ctx->phys_regs[ISP_BLK_ID_LSCR];
	union REG_ISP_LSCR_SC_WRAP_1 reg_ctrl;

	ISP_WR_BITS(lscr, REG_ISP_LSCR_T, SC_WRAP_25, LSCR_DMA_ENABLE, en);

	reg_ctrl.raw = ISP_RD_REG(lscr, REG_ISP_LSCR_T, SC_WRAP_1);
	reg_ctrl.bits.LSCR_ENABLE = en;
	ISP_WR_REG(lscr, REG_ISP_LSCR_T, SC_WRAP_1, reg_ctrl.raw);
}

void ispblk_lscr_config_dma(struct isp_ctx *ctx, uint64_t dma_addr, bool en)
{
	uintptr_t lscr = ctx->phys_regs[ISP_BLK_ID_LSCR];

	ISP_WR_BITS(lscr, REG_ISP_LSCR_T, SC_WRAP_25, LSCR_DMA_ENABLE, en);

	ISP_WR_BITS(lscr, REG_ISP_LSCR_T, SC_WRAP_25, LSCR_DMA_ADDR_HIGH8, dma_addr & 0xf00000000);

	vi_pr(VI_INFO, "dma addr:0x%lx\n", dma_addr);

	ISP_WR_BITS(lscr, REG_ISP_LSCR_T, SC_WRAP_26, LSCR_DMA_ADDR_LOW32, dma_addr & 0xffffffff);
}

void ispblk_drc_lut(struct isp_ctx *ctx, uint8_t sel, uint16_t *data, bool check_lut)
{
	// uintptr_t ltm = ctx->phys_regs[ISP_BLK_ID_DRC];
	// union REG_LTM_H34 reg_34;
	// union REG_LTM_H3C reg_3c;
	// u16 i = 0;
	// u32 val = 0;

	// vi_pr(VI_DBG, "LTM_G LUT using mem[%d]\n", sel);
	// ISP_WR_REG(ltm, REG_LTM_T, REG_H48, data[LTM_GLOBAL_LUT_SIZE]);

	// reg_34.raw = ISP_RD_REG(ltm, REG_LTM_T, REG_H34);
	// reg_34.bits.LUT_DBG_RSEL = sel;
	// reg_34.bits.LUT_PROG_EN_GLOBAL = 1;
	// ISP_WR_REG(ltm, REG_LTM_T, REG_H34, reg_34.raw);

	// reg_3c.raw = ISP_RD_REG(ltm, REG_LTM_T, REG_H3C);
	// reg_3c.bits.LUT_WSEL = sel;
	// reg_3c.bits.LUT_WSTADDR = 0;
	// ISP_WR_REG(ltm, REG_LTM_T, REG_H3C, reg_3c.raw);

	// ISP_WR_BITS(ltm, REG_LTM_T, REG_H3C, LUT_WSTADDR_TRIG_1T, 1);
	// ISP_WR_BITS(ltm, REG_LTM_T, REG_H3C, LUT_WDATA_TRIG_1T, 1);

	// ISP_WR_BITS(ltm, REG_LTM_T, REG_H3C, LUT_WSTADDR, 0);
	// ISP_WR_BITS(ltm, REG_LTM_T, REG_H3C, LUT_WSTADDR_TRIG_1T, 1);

	// for (i = 0; i < LTM_GLOBAL_LUT_SIZE; i += 2) {
	// 	val = (data[i] | (data[i + 1] << 16));
	// 	ISP_WR_REG(ltm, REG_LTM_T, REG_H38, val);
	// 	ISP_WR_BITS(ltm, REG_LTM_T, REG_H3C, LUT_WDATA_TRIG_1T, 1);
	// }

	// if (check_lut) {
	// 	bool is_lut_pass = true;
	// 	union REG_LTM_H4C reg_4c;
		
	// 	reg_34.raw = ISP_RD_REG(ltm, REG_LTM_T, REG_H34);
	// 	reg_34.bits.LUT_DBG_RSEL = sel;
	// 	reg_34.bits.LUT_PROG_EN_GLOBAL = 1;
	// 	for (i = 0; i < LTM_GLOBAL_LUT_SIZE; i++) {
	// 		reg_34.bits.LUT_DBG_RADDR = i;
	// 		reg_34.bits.LUT_DBG_RSEL  = sel;
	// 		reg_34.bits.LUT_DBG_READ_EN_1T = 1;
	// 		ISP_WR_REG(ltm, REG_LTM_T, REG_H34, reg_34.raw);

	// 		reg_4c.raw = ISP_RD_REG(ltm, REG_LTM_T, REG_H4C);
	// 		val = data[i];
	// 		if ((reg_4c.raw&0xFFFFFFFF) != val){
	// 			is_lut_pass = false;
	// 			break;
	// 		}
	// 	}
	// 	if (is_lut_pass)
	// 		vi_pr(VI_DBG, "LTM_G LUT Check success\n");
	// 	else
	// 		vi_pr(VI_DBG, "LTM_G LUT Check fail, lut[%d] = %d, should be %d\n",
        //             i, (reg_4c.raw&0xFFFFFFFF), val);
	// }

	// ISP_WR_BITS(ltm, REG_LTM_T, REG_H34, LUT_PROG_EN_GLOBAL, 0);
}

void ispblk_drc_config(struct isp_ctx *ctx, bool enable)
{
	uintptr_t drc = ctx->phys_regs[ISP_BLK_ID_DRC];
	union REG_ISP_DRC_ENABLE reg_ctrl;

	reg_ctrl.raw = ISP_RD_REG(drc, REG_ISP_DRC_T, ENABLE);
	reg_ctrl.bits.DRC_ENABLE = enable;
	ISP_WR_REG(drc, REG_ISP_DRC_T, ENABLE, reg_ctrl.raw);

	ISP_WR_BITS(drc, REG_ISP_DRC_T, HIST, DRC_HIST_ENABLE, enable);
}


void ispblk_cfa_config(struct isp_ctx *ctx)
{
	uintptr_t cfa = ctx->phys_regs[ISP_BLK_ID_CFA];
	union REG_ISP_CFA_00 reg_0;

	reg_0.raw = ISP_RD_REG(cfa, REG_ISP_CFA_T, REG_00);
	reg_0.bits.CFA_SHDW_SEL = 1;
	reg_0.bits.CFA_ENABLE	= 1;
	//reg_0.bits.CFA_FCR_ENABLE = 1;
	//reg_0.bits.CFA_MOIRE_ENABLE = 1;
	ISP_WR_REG(cfa, REG_ISP_CFA_T, REG_00, reg_0.raw);
}