#include <vip/vi_drv.h>

/****************************************************************************
 * Global parameters
 ****************************************************************************/


/*******************************************************************************
 *	Subsys config
 ******************************************************************************/
void ispblk_preraw_fe_config(struct isp_ctx *ctx, enum cvi_isp_raw raw_num)
{
	int id = fe_find_hwid(raw_num);
	uintptr_t preraw_fe = ctx->phys_regs[id];
	uint32_t width = ctx->isp_pipe_cfg[raw_num].csibdg_width;
	uint32_t height = ctx->isp_pipe_cfg[raw_num].csibdg_height;
	union REG_PRE_RAW_FE_PRE_RAW_CTRL raw_ctrl;
	union REG_PRE_RAW_FE_PRE_RAW_FRAME_SIZE  frm_size;

	frm_size.raw = 0;

	raw_ctrl.raw = ISP_RD_REG(preraw_fe, REG_PRE_RAW_FE_T, PRE_RAW_CTRL);
	raw_ctrl.bits.BAYER_TYPE_LE = ctx->rgb_color_mode[raw_num];
	raw_ctrl.bits.BAYER_TYPE_SE = ctx->rgb_color_mode[raw_num];
	ISP_WR_REG(preraw_fe, REG_PRE_RAW_FE_T, PRE_RAW_CTRL, raw_ctrl.raw);

	frm_size.bits.FRAME_WIDTHM1 = width - 1;
	frm_size.bits.FRAME_HEIGHTM1 = height - 1;
	ISP_WR_REG(preraw_fe, REG_PRE_RAW_FE_T, PRE_RAW_FRAME_SIZE, frm_size.raw);
}

void ispblk_preraw_vi_sel_config(struct isp_ctx *ctx)
{
	uintptr_t vi_sel = ctx->phys_regs[ISP_BLK_ID_PRE_RAW_VI_SEL];
	union REG_PRE_RAW_VI_SEL_1 vi_sel_1;

	vi_sel_1.raw = 0;
	vi_sel_1.bits.FRAME_WIDTHM1 = ctx->isp_pipe_cfg[0].csibdg_width - 1;
	vi_sel_1.bits.FRAME_HEIGHTM1 = ctx->isp_pipe_cfg[0].csibdg_height - 1;
	ISP_WR_REG(vi_sel, REG_PRE_RAW_VI_SEL_T, REG_1, vi_sel_1.raw);

	if (_is_fe_post_offline(ctx) && ctx->is_dpcm_on) { // dram->be
		ISP_WR_BITS(vi_sel, REG_PRE_RAW_VI_SEL_T, REG_0, DMA_LD_DPCM_MODE, 7);
		ISP_WR_BITS(vi_sel, REG_PRE_RAW_VI_SEL_T, REG_0, DPCM_RX_XSTR, 8191);
	} else {
		ISP_WR_BITS(vi_sel, REG_PRE_RAW_VI_SEL_T, REG_0, DMA_LD_DPCM_MODE, 0);
		ISP_WR_BITS(vi_sel, REG_PRE_RAW_VI_SEL_T, REG_0, DPCM_RX_XSTR, 0);
	}

	if (ctx->is_yuv_sensor) {
		ISP_WR_BITS(vi_sel, REG_PRE_RAW_VI_SEL_T, REG_0, CSI_IN_FORMAT, 1);
	}
	//vi_sel crop need to enable
	// ISP_WR_BITS(vi_sel, REG_PRE_RAW_VI_SEL_T, REG_9, CROP_ENABLE_LE, 1);
	// if (ctx->is_hdr_on)
	// 	ISP_WR_BITS(vi_sel, REG_PRE_RAW_VI_SEL_T, REG_9, CROP_ENABLE_SE, 1);
}

void ispblk_rawtop_config(struct isp_ctx *ctx, const enum cvi_isp_raw raw_num)
{
	uintptr_t rawtop0 = ctx->phys_regs[ISP_BLK_ID_RAWTOP0];
	uintptr_t rawtop1 = ctx->phys_regs[ISP_BLK_ID_RAWTOP1];
	union REG_RAW_TOP0_RAW_2 rawtop0_2;
	union REG_RAW_TOP0_PASS_SEL pass_sel;
	union REG_RAW_TOP1_RAW_2 rawtop1_2;

	rawtop0_2.raw = ISP_RD_REG(rawtop0, REG_RAW_TOP0_T, RAW_2);
	rawtop0_2.bits.IMG_WIDTHM_1_2X = ctx->img_width - 1;
	rawtop0_2.bits.IMG_HEIGHTM_1_2X = ctx->img_height - 1;
	ISP_WR_REG(rawtop0, REG_RAW_TOP0_T, RAW_2, rawtop0_2.raw);

	if (ctx->is_hdr_on) {
		pass_sel.raw = ISP_RD_REG(rawtop0, REG_RAW_TOP0_T, PASS_SEL);
		pass_sel.raw = 0;
		pass_sel.bits.PASS_SEL = 1;
		pass_sel.bits.PASS_CNT_M1 = 1;
		ISP_WR_REG(rawtop0, REG_RAW_TOP0_T, PASS_SEL, pass_sel.raw);
	}

	rawtop1_2.raw = ISP_RD_REG(rawtop0, REG_RAW_TOP0_T, RAW_2);
	rawtop1_2.bits.IMG_WIDTHM_1_1X = ctx->img_width - 1;
	rawtop1_2.bits.IMG_HEIGHTM_1_1X = ctx->img_height - 1;
	ISP_WR_REG(rawtop1, REG_RAW_TOP1_T, RAW_2, rawtop1_2.raw);
}

void ispblk_rgbtop_config(struct isp_ctx *ctx, const enum cvi_isp_raw raw_num)
{
	uintptr_t rgbtop = ctx->phys_regs[ISP_BLK_ID_RGBTOP];
	union REG_RGB_TOP_PATGEN_05 reg_5;

	reg_5.raw = 0;

	reg_5.bits.RGBTOP_IMGW = ctx->img_width;
	reg_5.bits.RGBTOP_IMGH = ctx->img_height;
	ISP_WR_REG(rgbtop, REG_ISP_RGB_TOP_T, PATGEN_05, reg_5.raw);
}

void ispblk_yuvtop_config(struct isp_ctx *ctx, const enum cvi_isp_raw raw_num)
{
	uintptr_t yuvtop = ctx->phys_regs[ISP_BLK_ID_YUVTOP];
	union REG_YUV_TOP_IMGW_M1 imgw_m1;

	imgw_m1.raw = 0;

	ISP_WO_BITS(yuvtop, REG_YUV_TOP_T, YUV_3, YONLY_EN, ctx->is_yuv_sensor);

	imgw_m1.bits.YUV_TOP_IMGW_M1 = ctx->img_width - 1;
	imgw_m1.bits.YUV_TOP_IMGH_M1 = ctx->img_height - 1;
	ISP_WR_REG(yuvtop, REG_YUV_TOP_T, IMGW_M1, imgw_m1.raw);

	ISP_WR_BITS(yuvtop, REG_YUV_TOP_T, YUV_CTRL_PHOBOS3, DROP_MODE, 1);

	//output fmt nv21:0  nv12:1
	ISP_WR_BITS(yuvtop, REG_YUV_TOP_T, YUV_CTRL_PHOBOS3, UV_SWAP_EN, 0);

	// if (!_is_fe_post_offline(ctx) && !ctx->isp_pipe_cfg[ISP_PRERAW0].is_offline_scaler) {
	// 	//bypass_v = 1 -> 422P online to scaler
	// 	ISP_WR_BITS(yuvtop, REG_YUV_TOP_T, YUV_CTRL, BYPASS_V, 1);
	// } else {
	// 	ISP_WR_BITS(yuvtop, REG_YUV_TOP_T, YUV_CTRL, BYPASS_V, !ctx->isp_pipe_cfg[raw_num].is_offline_scaler);
	// }

	// ISP_WR_BITS(yuvtop, REG_YUV_TOP_T, YUV_CTRL_PHOBOS3, BYPASS_V_1, 1);//vi-vpss online
}

void ispblk_isptop_config(struct isp_ctx *ctx)
{
	uintptr_t isptopb = ctx->phys_regs[ISP_BLK_ID_ISPTOP];
	enum cvi_isp_raw raw_num = ISP_PRERAW0;
	uint8_t pre_fe_trig_by_hw[ISP_PRERAW_MAX] = {0};
	uint8_t post_trig_by_hw = 0;

	union REG_ISP_TOP_INT_EVENT0_EN ev0_en;
	union REG_ISP_TOP_INT_EVENT1_EN ev1_en;
	union REG_ISP_TOP_INT_EVENT2_EN ev2_en;
	union REG_ISP_TOP_CTRL_MODE_SEL0 trig_sel0;
	union REG_ISP_TOP_CTRL_MODE_SEL1 trig_sel1;
	union REG_ISP_TOP_SCENARIOS_CTRL scene_ctrl;

	ev0_en.raw = ev1_en.raw = ev2_en.raw = 0;
	trig_sel0.raw = trig_sel1.raw = scene_ctrl.raw = 0;

	for (raw_num = ISP_PRERAW0; raw_num < ctx->total_dev_num; raw_num++) {
		if (!ctx->isp_pipe_cfg[raw_num].is_yuv_bypass_path) { //RGB sensor
			if (ctx->isp_pipe_cfg[raw_num].is_hdr_on)
				pre_fe_trig_by_hw[raw_num] = 3;
			else
				pre_fe_trig_by_hw[raw_num] = 1;
		} else { //TODO for YUV sensor we need check chn num;
			switch (ctx->total_chn_num) {
			case 1:
				pre_fe_trig_by_hw[raw_num] = 1;
				break;
			case 2:
				pre_fe_trig_by_hw[raw_num] = 3;
				break;
			case 3:
				pre_fe_trig_by_hw[raw_num]= 7;
				break;
			case 4:
				pre_fe_trig_by_hw[raw_num] = 15;
				break;
			default:
				break;
			}
		}
	}

	// offline mode post trigger by sw
	if (_is_fe_post_offline(ctx) && !ctx->is_slice_buf_on)
		post_trig_by_hw = 0;
	else {// fly mode or slice trigger by vsync
		if (ctx->isp_pipe_cfg[raw_num].is_hdr_on)
			post_trig_by_hw = 0x3;
		else
			post_trig_by_hw = 0x1;
	}

	vi_pr(VI_INFO, "post_trig_by_hw:%d\n", post_trig_by_hw);

	//pre_fe0
	ev0_en.bits.FRAME_DONE_ENABLE_FE0	= 0xF;
	ev2_en.bits.FRAME_START_ENABLE_FE0	= 0xF;
	trig_sel0.bits.TRIG_STR_SEL_FE0		= pre_fe_trig_by_hw[ISP_PRERAW0];
	trig_sel0.bits.SHAW_UP_SEL_FE0		= pre_fe_trig_by_hw[ISP_PRERAW0];
	trig_sel1.bits.PQ_UP_SEL_FE0		= pre_fe_trig_by_hw[ISP_PRERAW0];

	//pre_fe1
	ev0_en.bits.FRAME_DONE_ENABLE_FE1	= 0xF;
	ev2_en.bits.FRAME_START_ENABLE_FE1	= 0xF;
	trig_sel0.bits.TRIG_STR_SEL_FE1		= pre_fe_trig_by_hw[ISP_PRERAW1];
	trig_sel0.bits.SHAW_UP_SEL_FE1		= pre_fe_trig_by_hw[ISP_PRERAW1];
	trig_sel1.bits.PQ_UP_SEL_FE1		= pre_fe_trig_by_hw[ISP_PRERAW1];

	//pre_fe2
	ev0_en.bits.FRAME_DONE_ENABLE_FE2	= 0x3;
	ev2_en.bits.FRAME_START_ENABLE_FE2	= 0x3;
	trig_sel0.bits.TRIG_STR_SEL_FE2		= pre_fe_trig_by_hw[ISP_PRERAW2];
	trig_sel0.bits.SHAW_UP_SEL_FE2		= pre_fe_trig_by_hw[ISP_PRERAW2];
	trig_sel1.bits.PQ_UP_SEL_FE2		= pre_fe_trig_by_hw[ISP_PRERAW2];

	//pre_be
	ev0_en.bits.FRAME_DONE_ENABLE_BE	= 0x3;
	trig_sel0.bits.TRIG_STR_SEL_BE		= post_trig_by_hw;
	trig_sel0.bits.SHAW_UP_SEL_BE		= post_trig_by_hw;
	trig_sel1.bits.PQ_UP_SEL_BE		= post_trig_by_hw;

	//postraw
	ev0_en.bits.FRAME_DONE_ENABLE_POST	= 0x1;
	ev0_en.bits.SHAW_DONE_ENABLE_POST	= 0x1;
	trig_sel0.bits.TRIG_STR_SEL_RAW		= post_trig_by_hw;
	trig_sel0.bits.SHAW_UP_SEL_RAW		= post_trig_by_hw;
	trig_sel1.bits.PQ_UP_SEL_RAW		= post_trig_by_hw;
	trig_sel0.bits.TRIG_STR_SEL_POST	= post_trig_by_hw;
	trig_sel0.bits.SHAW_UP_SEL_POST		= post_trig_by_hw;
	trig_sel1.bits.PQ_UP_SEL_POST		= post_trig_by_hw;

	//err int
	ev2_en.bits.FRAME_ERR_ENABLE	= 1;
	ev2_en.bits.INT_DMA_ERR_ENABLE	= 1;

	//scenario_ctrl mode
	scene_ctrl.raw = ISP_RD_REG(isptopb, REG_ISP_TOP_T, SCENARIOS_CTRL);
	scene_ctrl.bits.PRE2BE_L_ENABLE		= !ctx->is_offline_postraw;
	scene_ctrl.bits.PRE2BE_S_ENABLE		= ctx->is_hdr_on && !ctx->is_offline_postraw;
	scene_ctrl.bits.PRE2YUV_422_ENABLE	= 0;
	scene_ctrl.bits.BE_RDMA_L_ENABLE	= ctx->is_offline_postraw;
	scene_ctrl.bits.BE_RDMA_S_ENABLE	= ctx->is_hdr_on && ctx->is_offline_postraw;

	scene_ctrl.bits.BE2RAW_L_ENABLE		= ctx->is_offline_postraw;
	scene_ctrl.bits.BE2RAW_S_ENABLE		= ctx->is_hdr_on && ctx->is_offline_postraw;

	scene_ctrl.bits.BE_SRC_SEL		= 0; //0/1/2 from fe0/fe1/fe2
	scene_ctrl.bits.HDR_ENABLE		= ctx->is_hdr_on;
	scene_ctrl.bits.DST2SC			= !ctx->isp_pipe_cfg[0].is_offline_scaler;
	scene_ctrl.bits.DST2DMA			= ctx->isp_pipe_cfg[0].is_offline_scaler;

	scene_ctrl.bits.FE_DMA_SHARE_ENABLE	= 1;

	// to verify IP, turn off HW LUT of rgbgamma, ynr, and cnr.
	scene_ctrl.bits.HW_AUTO_ENABLE		= 0;


	// scene_ctrl.bits.YUV_EDGE_EN = 1;//ee_ext
	ISP_WR_REG(isptopb, REG_ISP_TOP_T, INT_EVENT0, 0xffffffff);
	ISP_WR_REG(isptopb, REG_ISP_TOP_T, INT_EVENT1, 0xffffffff);
	ISP_WR_REG(isptopb, REG_ISP_TOP_T, INT_EVENT2, 0xffffffff);
	ISP_WR_REG(isptopb, REG_ISP_TOP_T, INT_EVENT0_EN, ev0_en.raw);
	ISP_WR_REG(isptopb, REG_ISP_TOP_T, INT_EVENT1_EN, ev1_en.raw);
	ISP_WR_REG(isptopb, REG_ISP_TOP_T, INT_EVENT2_EN, ev2_en.raw);
	ISP_WR_REG(isptopb, REG_ISP_TOP_T, CTRL_MODE_SEL0, trig_sel0.raw);
	ISP_WR_REG(isptopb, REG_ISP_TOP_T, CTRL_MODE_SEL1, trig_sel1.raw);
	ISP_WR_REG(isptopb, REG_ISP_TOP_T, SCENARIOS_CTRL, scene_ctrl.raw);

	ISP_WR_BITS(isptopb, REG_ISP_TOP_T, DUMMY, DBUS_SEL, 4);
	//ISP_WR_REG(isptopb, REG_ISP_TOP_T, REG_1C, 7);

	{//TODO
		vi_pr(VI_INFO, "disable except cfa&csc hdr:%d\n", ctx->is_hdr_on);
		if (ctx->is_hdr_on)
			ISP_WR_REG(isptopb, REG_ISP_TOP_T, IP_ENABLE1, 0x4000698);
		else
			// ISP_WR_REG(isptopb, REG_ISP_TOP_T, IP_ENABLE1, 0x4004110);//dpc+blc1
			// ISP_WR_REG(isptopb, REG_ISP_TOP_T, IP_ENABLE1, 0x4000034);//bnr+llsc
			// ISP_WR_REG(isptopb, REG_ISP_TOP_T, IP_ENABLE1, 0x4700010);//prf+ccm+rgbgamma
			// ISP_WR_REG(isptopb, REG_ISP_TOP_T, IP_ENABLE1, 0x7000010);//rgb dither+clut+csc
			// ISP_WR_REG(isptopb, REG_ISP_TOP_T, IP_ENABLE1, 0x4008010);//drc+cfa
			// ISP_WR_REG(isptopb, REG_ISP_TOP_T, IP_ENABLE1, 0x3e000010);//edge+preEE_f+preEE_B
			ISP_WR_REG(isptopb, REG_ISP_TOP_T, IP_ENABLE1, 0x4000010);
			// ISP_WR_REG(isptopb, REG_ISP_TOP_T, IP_ENABLE1, 0x0);
		// ISP_WR_REG(isptopb, REG_ISP_TOP_T, IP_ENABLE2, 0x63);//edge+preEE_f+preEE_B
		// ISP_WR_REG(isptopb, REG_ISP_TOP_T, IP_ENABLE2, 0x60);
		// ISP_WR_REG(isptopb, REG_ISP_TOP_T, IP_ENABLE2, 0x460);//ldci
		// ISP_WR_REG(isptopb, REG_ISP_TOP_T, IP_ENABLE2, 0x64);//mctf
		// ISP_WR_REG(isptopb, REG_ISP_TOP_T, IP_ENABLE2, 0x378);//cnr+ca+postee
		ISP_WR_REG(isptopb, REG_ISP_TOP_T, IP_ENABLE2, 0xe0);//yuvr
		ISP_WR_REG(isptopb, REG_ISP_TOP_T, IP_ENABLE3, 0x0);
	}
}

#ifdef PORTING_TEST

void ispblk_isptop_fpga_config(struct isp_ctx *ctx, uint16_t test_case)
{
	uintptr_t isptopb = ctx->phys_regs[ISP_BLK_ID_ISPTOP];
	union REG_ISP_TOP_SCENARIOS_CTRL scene_ctrl;

	scene_ctrl.raw = ISP_RD_REG(isptopb, REG_ISP_TOP_T, SCENARIOS_CTRL);
	// to verify IP, turn off HW LUT of rgbgamma, ynr, and cnr.
	if (test_case == 0) {
		scene_ctrl.bits.HW_AUTO_ENABLE		= 0;
		scene_ctrl.bits.HW_AUTO_ISO		= 0;
	} else if (test_case == 1) {
		scene_ctrl.bits.HW_AUTO_ENABLE		= 1;
		scene_ctrl.bits.HW_AUTO_ISO		= 0;
	} else if (test_case == 2) { //auto_iso
		scene_ctrl.bits.HW_AUTO_ENABLE		= 1;
		scene_ctrl.bits.HW_AUTO_ISO		= 2;
	} else if (test_case == 0xFF) { //store default config
		scene_ctrl.bits.HW_AUTO_ENABLE		= 1;
		scene_ctrl.bits.HW_AUTO_ISO		= 0;
	}

	ISP_WR_REG(isptopb, REG_ISP_TOP_T, SCENARIOS_CTRL, scene_ctrl.raw);
}

#endif

void isp_intr_set_mask(struct isp_ctx *ctx)
{
	uintptr_t isp_top = ctx->phys_regs[ISP_BLK_ID_ISPTOP];

	ISP_WR_REG(isp_top, REG_ISP_TOP_T, INT_EVENT0_EN, 0);
	ISP_WR_REG(isp_top, REG_ISP_TOP_T, INT_EVENT1_EN, 0);
	ISP_WR_REG(isp_top, REG_ISP_TOP_T, INT_EVENT2_EN, 0);
}

void isp_reset(struct isp_ctx *ctx)
{
	uintptr_t isptopb = ctx->phys_regs[ISP_BLK_ID_ISPTOP];
	union REG_ISP_TOP_CTRL_MODE_SEL0 mode_sel0;

	// disable interrupt
	isp_intr_set_mask(ctx);

	// switch back to hw trig.
	mode_sel0.raw = ISP_RD_REG(isptopb, REG_ISP_TOP_T, CTRL_MODE_SEL0);
	mode_sel0.bits.TRIG_STR_SEL_FE0 = 0xf;
	mode_sel0.bits.TRIG_STR_SEL_FE1 = 0x3;
	mode_sel0.bits.TRIG_STR_SEL_FE2 = 0x3;
	mode_sel0.bits.TRIG_STR_SEL_BE  = 0x3;
	mode_sel0.bits.TRIG_STR_SEL_RAW  = 0x1;
	mode_sel0.bits.TRIG_STR_SEL_POST  = 0x1;
	ISP_WR_REG(isptopb, REG_ISP_TOP_T, CTRL_MODE_SEL0, mode_sel0.raw);

	// reset
	//AXI_RST first, then reset other
	ISP_WR_REG(isptopb, REG_ISP_TOP_T, SW_RST, 0x40);
	ISP_WR_REG(isptopb, REG_ISP_TOP_T, SW_RST, 0x37F);
	ISP_WR_REG(isptopb, REG_ISP_TOP_T, SW_RST, 0x40);
	ISP_WR_REG(isptopb, REG_ISP_TOP_T, SW_RST, 0);

	// clear intr
	ISP_WR_REG(isptopb, REG_ISP_TOP_T, INT_EVENT0, 0xffffffff);
	ISP_WR_REG(isptopb, REG_ISP_TOP_T, INT_EVENT1, 0xffffffff);
	ISP_WR_REG(isptopb, REG_ISP_TOP_T, INT_EVENT2, 0xffffffff);
}

/*******************************************************************************
 *	Subsys Debug Infomation
 ******************************************************************************/

struct _fe_dbg_i ispblk_fe_dbg_info(struct isp_ctx *ctx, enum cvi_isp_raw raw_num)
{
	uintptr_t preraw_fe;
	struct _fe_dbg_i data;

	if (raw_num == ISP_PRERAW0) {
		preraw_fe = ctx->phys_regs[ISP_BLK_ID_PRE_RAW_FE0];
	} else if (raw_num == ISP_PRERAW1) {
		preraw_fe = ctx->phys_regs[ISP_BLK_ID_PRE_RAW_FE1];
	} else {
		preraw_fe = ctx->phys_regs[ISP_BLK_ID_PRE_RAW_FE2];
	}

	data.fe_idle_sts = ISP_RD_REG(preraw_fe, REG_PRE_RAW_FE_T, PRE_RAW_DEBUG_STATE);
	data.fe_done_sts = ISP_RD_REG(preraw_fe, REG_PRE_RAW_FE_T, FE_IDLE_INFO);

	return data;
}

struct _post_dbg_i ispblk_post_dbg_info(struct isp_ctx *ctx)
{
	uintptr_t isptopb = ctx->phys_regs[ISP_BLK_ID_ISPTOP];
	struct _post_dbg_i data;

	data.top_sts_0 = ISP_RD_REG(isptopb, REG_ISP_TOP_T, BLK_IDLE);

	return data;
}

struct _dma_dbg_i ispblk_dma_dbg_info(struct isp_ctx *ctx)
{
	uintptr_t wdma_com_1 = ctx->phys_regs[ISP_BLK_ID_WDMA_CORE1];
	uintptr_t wdma_com_2 = ctx->phys_regs[ISP_BLK_ID_WDMA_CORE2];
	uintptr_t wdma_com_3 = ctx->phys_regs[ISP_BLK_ID_WDMA_CORE3];
	uintptr_t wdma_com_4 = ctx->phys_regs[ISP_BLK_ID_WDMA_CORE4];
	uintptr_t rdma_com_1 = ctx->phys_regs[ISP_BLK_ID_RDMA_CORE1];
	uintptr_t rdma_com_2 = ctx->phys_regs[ISP_BLK_ID_RDMA_CORE2];
	uintptr_t rdma_com_3 = ctx->phys_regs[ISP_BLK_ID_RDMA_CORE3];

	struct _dma_dbg_i data;

	//RDMA_1 err status
	data.rdma_1_err_sts = ISP_RD_REG(rdma_com_1, REG_RDMA_CORE_T, NORM_STATUS0); //0x10
	//RDMA_1 status
	data.rdma_1_idle = ISP_RD_REG(rdma_com_1, REG_RDMA_CORE_T, NORM_STATUS1); //0x14

	//RDMA_2 err status
	data.rdma_2_err_sts = ISP_RD_REG(rdma_com_2, REG_RDMA_CORE_T, NORM_STATUS0); //0x10
	//RDMA_2 status
	data.rdma_2_idle = ISP_RD_REG(rdma_com_2, REG_RDMA_CORE_T, NORM_STATUS1); //0x14

	//RDMA_3 err status
	data.rdma_3_err_sts = ISP_RD_REG(rdma_com_3, REG_RDMA_CORE_T, NORM_STATUS0); //0x10
	//RDMA_3 status
	data.rdma_3_idle = ISP_RD_REG(rdma_com_3, REG_RDMA_CORE_T, NORM_STATUS1); //0x14

	//WDMA_1 err status
	data.wdma_1_err_sts = ISP_RD_REG(wdma_com_1, REG_WDMA_CORE_T, NORM_STATUS0); //0x10
	//WDMA_1 status
	data.wdma_1_idle = ISP_RD_REG(wdma_com_1, REG_WDMA_CORE_T, NORM_STATUS1); //0x14

	//WDMA_2 err status
	data.wdma_2_err_sts = ISP_RD_REG(wdma_com_2, REG_WDMA_CORE_T, NORM_STATUS0); //0x10
	//WDMA_2 status
	data.wdma_2_idle = ISP_RD_REG(wdma_com_2, REG_WDMA_CORE_T, NORM_STATUS1); //0x14

	//WDMA_3 err status
	data.wdma_3_err_sts = ISP_RD_REG(wdma_com_3, REG_WDMA_CORE_T, NORM_STATUS0); //0x10
	//WDMA_3 status
	data.wdma_3_idle = ISP_RD_REG(wdma_com_3, REG_WDMA_CORE_T, NORM_STATUS1); //0x14

	//WDMA_4 err status
	data.wdma_4_err_sts = ISP_RD_REG(wdma_com_4, REG_WDMA_CORE_T, NORM_STATUS0); //0x10
	//WDMA_4 status
	data.wdma_4_idle = ISP_RD_REG(wdma_com_4, REG_WDMA_CORE_T, NORM_STATUS1); //0x14

	return data;
}
