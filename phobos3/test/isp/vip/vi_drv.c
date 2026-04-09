#include <vip/vi_drv.h>
#include <vi_common.h>
#include "vi_defines.h"

/****************************************************************************
 * Global parameters
 ****************************************************************************/

/****************************************************************************
 * FBC_CONFIG
 ****************************************************************************/
extern struct vi_fbc_cfg fbc_cfg;

/****************************************************************************
 * SLICE_BUFFER_CONFIG
 ****************************************************************************/
struct slice_buf_s slc_b_cfg = {
	.line_delay		= 512,
	.buffer			= 16,
	.main_max_grid_size	= 1,
	.min_r_thshd		= 1,
};

/**********************************************************
 *	SW scenario path check APIs
 **********************************************************/
u32 _is_fe_post_offline(struct isp_ctx *ctx)
{
	if (ctx->is_offline_postraw) //fe->dram->post
		return 1;
	return 0;
}

/****************************************************************************
 *  Interfaces
 ****************************************************************************/
void vi_set_base_addr(void *base)
{
	uintptr_t *addr = isp_get_phys_reg_bases();
	int i = 0;

	for (i = 0; i < ISP_BLK_ID_MAX; ++i) {
		addr[i] += (uintptr_t)base;
	}
}

uintptr_t *isp_get_phys_reg_bases(void)
{
	static uintptr_t m_isp_phys_base_list[ISP_BLK_ID_MAX] = {
		[ISP_BLK_ID_PRE_RAW_FE0]		= (ISP_BLK_BA_PRE_RAW_FE0),
		[ISP_BLK_ID_CSIBDG0]			= (ISP_BLK_BA_CSIBDG0),
		[ISP_BLK_ID_DMA_CTL_CSI0_BDG0]		= (ISP_BLK_BA_DMA_CTL_CSI0_BDG0),
		[ISP_BLK_ID_DMA_CTL_CSI0_BDG1]		= (ISP_BLK_BA_DMA_CTL_CSI0_BDG1),
		[ISP_BLK_ID_DMA_CTL_CSI0_BDG2]		= (ISP_BLK_BA_DMA_CTL_CSI0_BDG2),
		[ISP_BLK_ID_DMA_CTL_CSI0_BDG3]		= (ISP_BLK_BA_DMA_CTL_CSI0_BDG3),
		[ISP_BLK_ID_PRE_RAW_FE0_LSC0]		= (ISP_BLK_BA_PRE_RAW_FE0_LSC0),
		[ISP_BLK_ID_PRE_RAW_FE0_LSC1]		= (ISP_BLK_BA_PRE_RAW_FE0_LSC1),
		[ISP_BLK_ID_DMA_CTL_FE0_LSC_LE]		= (ISP_BLK_BA_DMA_CTL_FE0_LSC_LE),
		[ISP_BLK_ID_DMA_CTL_FE0_LSC_SE]		= (ISP_BLK_BA_DMA_CTL_FE0_LSC_SE),
		[ISP_BLK_ID_AE_HIST_FE0]		= (ISP_BLK_BA_AE_HIST_FE0),
		[ISP_BLK_ID_DMA_CTL_FE0_AE_HIST_LE]	= (ISP_BLK_BA_DMA_CTL_FE0_AE_HIST_LE),
		[ISP_BLK_ID_DMA_CTL_FE0_AE_HIST_SE]	= (ISP_BLK_BA_DMA_CTL_FE0_AE_HIST_SE),

		[ISP_BLK_ID_PRE_RAW_FE1]		= (ISP_BLK_BA_PRE_RAW_FE1),
		[ISP_BLK_ID_CSIBDG1]			= (ISP_BLK_BA_CSIBDG1),
		[ISP_BLK_ID_DMA_CTL_CSI1_BDG0]		= (ISP_BLK_BA_DMA_CTL_CSI1_BDG0),
		[ISP_BLK_ID_DMA_CTL_CSI1_BDG1]		= (ISP_BLK_BA_DMA_CTL_CSI1_BDG1),
		[ISP_BLK_ID_PRE_RAW_FE1_LSC0]		= (ISP_BLK_BA_PRE_RAW_FE1_LSC0),
		[ISP_BLK_ID_PRE_RAW_FE1_LSC1]		= (ISP_BLK_BA_PRE_RAW_FE1_LSC1),
		[ISP_BLK_ID_DMA_CTL_FE1_LSC_LE]		= (ISP_BLK_BA_DMA_CTL_FE1_LSC_LE),
		[ISP_BLK_ID_DMA_CTL_FE1_LSC_SE]		= (ISP_BLK_BA_DMA_CTL_FE1_LSC_SE),
		[ISP_BLK_ID_AE_HIST_FE1]		= (ISP_BLK_BA_AE_HIST_FE1),
		[ISP_BLK_ID_DMA_CTL_FE1_AE_HIST_LE]	= (ISP_BLK_BA_DMA_CTL_FE1_AE_HIST_LE),
		[ISP_BLK_ID_DMA_CTL_FE1_AE_HIST_SE]	= (ISP_BLK_BA_DMA_CTL_FE1_AE_HIST_SE),

		[ISP_BLK_ID_PRE_RAW_FE2]		= (ISP_BLK_BA_PRE_RAW_FE2),
		[ISP_BLK_ID_CSIBDG2]			= (ISP_BLK_BA_CSIBDG2),
		[ISP_BLK_ID_DMA_CTL_CSI2_BDG0]		= (ISP_BLK_BA_DMA_CTL_CSI2_BDG0),
		[ISP_BLK_ID_PRE_RAW_FE2_LSC0]		= (ISP_BLK_BA_PRE_RAW_FE2_LSC0),
		[ISP_BLK_ID_DMA_CTL_FE2_LSC_LE]		= (ISP_BLK_BA_DMA_CTL_FE2_LSC_LE),
		[ISP_BLK_ID_AE_HIST_FE2]		= (ISP_BLK_BA_AE_HIST_FE2),
		[ISP_BLK_ID_DMA_CTL_FE2_AE_HIST_LE]	= (ISP_BLK_BA_DMA_CTL_FE2_AE_HIST_LE),

		[ISP_BLK_ID_WDMA_CORE1]			= (ISP_BLK_BA_WDMA_CORE1),
		[ISP_BLK_ID_WDMA_CORE2]			= (ISP_BLK_BA_WDMA_CORE2),
		[ISP_BLK_ID_WDMA_CORE3]			= (ISP_BLK_BA_WDMA_CORE3),
		[ISP_BLK_ID_WDMA_CORE4]			= (ISP_BLK_BA_WDMA_CORE4),

		[ISP_BLK_ID_RAWTOP0]			= (ISP_BLK_BA_RAWTOP0),
		[ISP_BLK_ID_RAWTOP1]			= (ISP_BLK_BA_RAWTOP1),
		[ISP_BLK_ID_BLC_DG_WB0]			= (ISP_BLK_BA_BLC_DG_WB0),
		[ISP_BLK_ID_BLC_DG_WB1]			= (ISP_BLK_BA_BLC_DG_WB1),
		[ISP_BLK_ID_BNR_LITE]			= (ISP_BLK_BA_BNR_LITE),
		[ISP_BLK_ID_FUSION]			= (ISP_BLK_BA_FUSION),
		[ISP_BLK_ID_MAPCURVE]			= (ISP_BLK_BA_MAPCURVE),
		[ISP_BLK_ID_DPC]			= (ISP_BLK_BA_DPC),
		[ISP_BLK_ID_AF]				= (ISP_BLK_BA_AF),
		[ISP_BLK_ID_DMA_CTL_AF_W]		= (ISP_BLK_BA_DMA_CTL_AF_W),
		[ISP_BLK_ID_BNR]			= (ISP_BLK_BA_BNR),
		[ISP_BLK_ID_LSCR]			= (ISP_BLK_BA_LSCR),
		[ISP_BLK_ID_DMA_CTL_LSCR_HIST]		= (ISP_BLK_BA_DMA_CTL_LSCR_HIST),
		[ISP_BLK_ID_DRC]			= (ISP_BLK_BA_DRC),
		[ISP_BLK_ID_DMA_CTL_DRC_POLY_R]		= (ISP_BLK_BA_DMA_CTL_DRC_POLY_R),
		[ISP_BLK_ID_DMA_CTL_DRC_POLY_W]		= (ISP_BLK_BA_DMA_CTL_DRC_POLY_W),
		[ISP_BLK_ID_DMA_CTL_DRC_HIST]		= (ISP_BLK_BA_DMA_CTL_DRC_HIST),
		[ISP_BLK_ID_CFA]			= (ISP_BLK_BA_CFA),

		[ISP_BLK_ID_RGBTOP]			= (ISP_BLK_BA_RGBTOP),
		[ISP_BLK_ID_PRE_EE_EXT]			= (ISP_BLK_BA_PRE_EE_EXT),
		[ISP_BLK_ID_PFR]			= (ISP_BLK_BA_PFR),
		[ISP_BLK_ID_CCM]			= (ISP_BLK_BA_CCM),
		[ISP_BLK_ID_RGBGAMMA]			= (ISP_BLK_BA_RGBGAMMA),
		[ISP_BLK_ID_CLUT]			= (ISP_BLK_BA_CLUT),
		[ISP_BLK_ID_DEHAZE]			= (ISP_BLK_BA_DEHAZE),
		[ISP_BLK_ID_CSC]			= (ISP_BLK_BA_CSC),
		[ISP_BLK_ID_RGB_DITHER]			= (ISP_BLK_BA_RGB_DITHER),

		[ISP_BLK_ID_YUVTOP]			= (ISP_BLK_BA_YUVTOP),
		[ISP_BLK_ID_YUV_DITHER]			= (ISP_BLK_BA_YUV_DITHER),
		[ISP_BLK_ID_PRE_EE_FRONT]		= (ISP_BLK_BA_PRE_EE_FRONT),
		[ISP_BLK_ID_PRE_EE_BACK]		= (ISP_BLK_BA_PRE_EE_BACK),
		[ISP_BLK_ID_LDCI]			= (ISP_BLK_BA_LDCI),
		[ISP_BLK_ID_DMA_CTL_LDCI_W]		= (ISP_BLK_BA_DMA_CTL_LDCI_W),
		[ISP_BLK_ID_DMA_CTL_LDCI_R]		= (ISP_BLK_BA_DMA_CTL_LDCI_R),
		[ISP_BLK_ID_DMA_CTL_LDCI_HIST]		= (ISP_BLK_BA_DMA_CTL_LDCI_HIST),
		[ISP_BLK_ID_TNR]			= (ISP_BLK_BA_TNR),
		[ISP_BLK_ID_DMA_CTL_TNR_LD_Y]		= (ISP_BLK_BA_DMA_CTL_TNR_LD_Y),
		[ISP_BLK_ID_DMA_CTL_TNR_LD_C]		= (ISP_BLK_BA_DMA_CTL_TNR_LD_C),
		[ISP_BLK_ID_DMA_CTL_TNR_LD_MV]		= (ISP_BLK_BA_DMA_CTL_TNR_LD_MV),
		[ISP_BLK_ID_DMA_CTL_TNR_LD_MO]		= (ISP_BLK_BA_DMA_CTL_TNR_LD_MO),
		[ISP_BLK_ID_DMA_CTL_TNR_LD_FCB]		= (ISP_BLK_BA_DMA_CTL_TNR_LD_FCB),
		[ISP_BLK_ID_DMA_CTL_TNR_ST_Y]		= (ISP_BLK_BA_DMA_CTL_TNR_ST_Y),
		[ISP_BLK_ID_DMA_CTL_TNR_ST_C]		= (ISP_BLK_BA_DMA_CTL_TNR_ST_C),
		[ISP_BLK_ID_DMA_CTL_TNR_ST_MV]		= (ISP_BLK_BA_DMA_CTL_TNR_ST_MV),
		[ISP_BLK_ID_DMA_CTL_TNR_ST_MO]		= (ISP_BLK_BA_DMA_CTL_TNR_ST_MO),
		[ISP_BLK_ID_DMA_CTL_TNR_ST_FCB]		= (ISP_BLK_BA_DMA_CTL_TNR_ST_FCB),
		[ISP_BLK_ID_DMA_CTL_TNR_ST_MSP]		= (ISP_BLK_BA_DMA_CTL_TNR_ST_MSP),
		[ISP_BLK_ID_FBCD]			= (ISP_BLK_BA_FBCD),
		[ISP_BLK_ID_FBCE]			= (ISP_BLK_BA_FBCE),
		[ISP_BLK_ID_CNR]			= (ISP_BLK_BA_CNR),
		[ISP_BLK_ID_DMA_CTL_CNR_Y_W]		= (ISP_BLK_BA_DMA_CTL_CNR_Y_W),
		[ISP_BLK_ID_DMA_CTL_CNR_Y_R]		= (ISP_BLK_BA_DMA_CTL_CNR_Y_R),
		[ISP_BLK_ID_DMA_CTL_CNR_C_W]		= (ISP_BLK_BA_DMA_CTL_CNR_C_W),
		[ISP_BLK_ID_DMA_CTL_CNR_C_R]		= (ISP_BLK_BA_DMA_CTL_CNR_C_R),
		[ISP_BLK_ID_CA]				= (ISP_BLK_BA_CA),
		[ISP_BLK_ID_CA_LITE]			= (ISP_BLK_BA_CA_LITE),
		[ISP_BLK_ID_POST_EE]			= (ISP_BLK_BA_POST_EE),
		[ISP_BLK_ID_YCURVE]			= (ISP_BLK_BA_YCURVE),
		[ISP_BLK_ID_YUV_CROP_Y]			= (ISP_BLK_BA_YUV_CROP_Y),
		[ISP_BLK_ID_DMA_CTL_YUV_CROP_Y]		= (ISP_BLK_BA_DMA_CTL_YUV_CROP_Y),
		[ISP_BLK_ID_YUV_CROP_C]			= (ISP_BLK_BA_YUV_CROP_C),
		[ISP_BLK_ID_DMA_CTL_YUV_CROP_C]		= (ISP_BLK_BA_DMA_CTL_YUV_CROP_C),
		[ISP_BLK_ID_DMA_CTL_YUV_RDMA_Y]		= (ISP_BLK_BA_DMA_CTL_YUV_RDMA_Y),
		[ISP_BLK_ID_DMA_CTL_YUV_RDMA_C]		= (ISP_BLK_BA_DMA_CTL_YUV_RDMA_C),

		[ISP_BLK_ID_ISPTOP]			= (ISP_BLK_BA_ISPTOP),
		[ISP_BLK_ID_RDMA_CORE1]			= (ISP_BLK_BA_RDMA_CORE1),
		[ISP_BLK_ID_RDMA_CORE2]			= (ISP_BLK_BA_RDMA_CORE2),
		[ISP_BLK_ID_RDMA_CORE3]			= (ISP_BLK_BA_RDMA_CORE3),
		[ISP_BLK_ID_CSIBDG0_LITE]		= (ISP_BLK_BA_CSIBDG0_LITE),
		[ISP_BLK_ID_DMA_CTL_BT0_LITE0]		= (ISP_BLK_BA_DMA_CTL_BT0_LITE0),
		[ISP_BLK_ID_DMA_CTL_BT0_LITE1]		= (ISP_BLK_BA_DMA_CTL_BT0_LITE1),
		[ISP_BLK_ID_DMA_CTL_BT0_LITE2]		= (ISP_BLK_BA_DMA_CTL_BT0_LITE2),
		[ISP_BLK_ID_DMA_CTL_BT0_LITE3]		= (ISP_BLK_BA_DMA_CTL_BT0_LITE3),
		[ISP_BLK_ID_PRE_RAW_VI_SEL]		= (ISP_BLK_BA_PRE_RAW_VI_SEL),
		[ISP_BLK_ID_DMA_CTL_PRE_RAW_VI_SEL_LE]	= (ISP_BLK_BA_DMA_CTL_PRE_RAW_VI_SEL_LE),
		[ISP_BLK_ID_DMA_CTL_PRE_RAW_VI_SEL_SE]	= (ISP_BLK_BA_DMA_CTL_PRE_RAW_VI_SEL_SE),
		[ISP_BLK_ID_PRE_RAW_VI_SEL_CROP_LE]	= (ISP_BLK_BA_PRE_RAW_VI_SEL_CROP_LE),
		[ISP_BLK_ID_PRE_RAW_VI_SEL_CROP_SE]	= (ISP_BLK_BA_PRE_RAW_VI_SEL_CROP_SE),
		[ISP_BLK_ID_PRE_RAW_VI_SEL_CROP_YUV]	= (ISP_BLK_BA_PRE_RAW_VI_SEL_CROP_YUV),
		[ISP_BLK_ID_CMDQ]			= (ISP_BLK_BA_CMDQ),
	};
	return m_isp_phys_base_list;
}

void isp_debug_dump(struct isp_ctx *ctx)
{
}

void isp_intr_status(
	struct isp_ctx *ctx,
	union REG_ISP_TOP_INT_EVENT0 *s0,
	union REG_ISP_TOP_INT_EVENT1 *s1,
	union REG_ISP_TOP_INT_EVENT2 *s2)
{
	uintptr_t isp_top = ctx->phys_regs[ISP_BLK_ID_ISPTOP];

	s0->raw = ISP_RD_REG(isp_top, REG_ISP_TOP_T, INT_EVENT0);
	//clear isp top event0 status
	ISP_WR_REG(isp_top, REG_ISP_TOP_T, INT_EVENT0, s0->raw);

	s1->raw = ISP_RD_REG(isp_top, REG_ISP_TOP_T, INT_EVENT1);
	//clear isp top event1 status
	ISP_WR_REG(isp_top, REG_ISP_TOP_T, INT_EVENT1, s1->raw);

	s2->raw = ISP_RD_REG(isp_top, REG_ISP_TOP_T, INT_EVENT2);
	//clear isp top event2 status
	ISP_WR_REG(isp_top, REG_ISP_TOP_T, INT_EVENT2, s2->raw);
}

void isp_csi_intr_status(
	struct isp_ctx *ctx,
	enum cvi_isp_raw raw_num,
	union REG_ISP_CSI_BDG_INTERRUPT_STATUS_0 *s0,
	union REG_ISP_CSI_BDG_INTERRUPT_STATUS_1 *s1)
{
	int id = csibdg_find_hwid(raw_num);
	uintptr_t csi_bdg = ctx->phys_regs[id];

	s0->raw = ISP_RD_REG(csi_bdg, REG_ISP_CSI_BDG_T, INTERRUPT_STATUS_0);
	//clear status
	ISP_WR_REG(csi_bdg, REG_ISP_CSI_BDG_T, INTERRUPT_STATUS_0, s0->raw);

	s1->raw = ISP_RD_REG(csi_bdg, REG_ISP_CSI_BDG_T, INTERRUPT_STATUS_1);
	//clear status
	ISP_WR_REG(csi_bdg, REG_ISP_CSI_BDG_T, INTERRUPT_STATUS_1, s1->raw);
}

void isp_init(struct isp_ctx *ctx)
{
	u8 i = 0;

	for (i = 0; i < ISP_PRERAW_MAX; i++) {
		//TODO
	}
}

void isp_streaming(struct isp_ctx *ctx, uint32_t on, enum cvi_isp_raw raw_num)
{
	int id = csibdg_find_hwid(raw_num);
	uintptr_t csibdg = ctx->phys_regs[id];
	uintptr_t isptopb = ctx->phys_regs[ISP_BLK_ID_ISPTOP];
	union REG_ISP_TOP_SW_CTRL_0 sw_ctrl_0;
	union REG_ISP_TOP_SW_CTRL_1 sw_ctrl_1;
	union REG_ISP_CSI_BDG_TOP_CTRL csibdg_topctl;

	sw_ctrl_0.raw = sw_ctrl_1.raw = 0;

	if (on) {
		if (raw_num == ISP_PRERAW0) {
			if (ctx->isp_pipe_cfg[raw_num].is_hdr_on) {
				sw_ctrl_0.bits.SHAW_UP_FE0	= 3;
				sw_ctrl_1.bits.PQ_UP_FE0	= 3;
			} else {
				sw_ctrl_0.bits.SHAW_UP_FE0	= 1;
				sw_ctrl_1.bits.PQ_UP_FE0	= 1;
			}
		} else if (raw_num == ISP_PRERAW1) {
			if (ctx->isp_pipe_cfg[raw_num].is_hdr_on) {
				sw_ctrl_0.bits.SHAW_UP_FE1	= 3;
				sw_ctrl_1.bits.PQ_UP_FE1	= 3;
			} else {
				sw_ctrl_0.bits.SHAW_UP_FE1	= 1;
				sw_ctrl_1.bits.PQ_UP_FE1	= 1;
			}
		} else if (raw_num == ISP_PRERAW2) {
			sw_ctrl_0.bits.SHAW_UP_FE2	= 1;
			sw_ctrl_1.bits.PQ_UP_FE2	= 1;
		}

		sw_ctrl_0.bits.SHAW_UP_BE	= 1;
		sw_ctrl_1.bits.PQ_UP_BE		= 1;
		sw_ctrl_0.bits.SHAW_UP_RAW	= 1;
		sw_ctrl_1.bits.PQ_UP_RAW	= 1;
		sw_ctrl_0.bits.SHAW_UP_POST	= 1;
		sw_ctrl_1.bits.PQ_UP_POST	= 1;

		ISP_WR_REG(isptopb, REG_ISP_TOP_T, SW_CTRL_1, sw_ctrl_1.raw);
		ISP_WR_REG(isptopb, REG_ISP_TOP_T, SW_CTRL_0, sw_ctrl_0.raw);

		csibdg_topctl.raw = ISP_RD_REG(csibdg, REG_ISP_CSI_BDG_T, CSI_BDG_TOP_CTRL);
		if (ctx->isp_pipe_cfg[raw_num].is_raw_replay) {
			csibdg_topctl.bits.CSI_UP_REG = 0;
			csibdg_topctl.bits.CSI_ENABLE = 0;
			csibdg_topctl.bits.TGEN_ENABLE = 0;
		} else {
			csibdg_topctl.bits.CSI_UP_REG = 1;
			csibdg_topctl.bits.CSI_ENABLE = 1;

			if (ctx->isp_pipe_cfg[raw_num].is_patgen_en)
				csibdg_topctl.bits.TGEN_ENABLE = 1;
			else
				csibdg_topctl.bits.TGEN_ENABLE = 0;
		}

		ISP_WR_REG(csibdg, REG_ISP_CSI_BDG_T, CSI_BDG_TOP_CTRL, csibdg_topctl.raw);
	} else {
		csibdg_topctl.raw = ISP_RD_REG(csibdg, REG_ISP_CSI_BDG_T, CSI_BDG_TOP_CTRL);
		csibdg_topctl.bits.CSI_ENABLE = 0;

		if (ctx->isp_pipe_cfg[raw_num].is_patgen_en)
			csibdg_topctl.bits.TGEN_ENABLE = 0;
		ISP_WR_REG(csibdg, REG_ISP_CSI_BDG_T, CSI_BDG_TOP_CTRL, csibdg_topctl.raw);
	}
}

void isp_pre_trig(struct isp_ctx *ctx, enum cvi_isp_raw raw_num, const u8 chn_num)
{
	int id = fe_find_hwid(raw_num);
	uintptr_t preraw0 = ctx->phys_regs[id];

	// patgen or sensor->fe
	vi_pr(VI_INFO, "trigger fe_%d chn_num_%d frame_vld\n", raw_num, chn_num);

	switch (chn_num) {
	case 0:
		ISP_WR_BITS(preraw0, REG_PRE_RAW_FE_T, PRE_RAW_FRAME_VLD, FE_PQ_VLD_CH0, 1);
		ISP_WR_BITS(preraw0, REG_PRE_RAW_FE_T, PRE_RAW_FRAME_VLD, FE_FRAME_VLD_CH0, 1);
		break;
	case 1:
		ISP_WR_BITS(preraw0, REG_PRE_RAW_FE_T, PRE_RAW_FRAME_VLD, FE_PQ_VLD_CH1, 1);
		ISP_WR_BITS(preraw0, REG_PRE_RAW_FE_T, PRE_RAW_FRAME_VLD, FE_FRAME_VLD_CH1, 1);
		break;
	case 2:
		ISP_WR_BITS(preraw0, REG_PRE_RAW_FE_T, PRE_RAW_FRAME_VLD, FE_PQ_VLD_CH2, 1);
		ISP_WR_BITS(preraw0, REG_PRE_RAW_FE_T, PRE_RAW_FRAME_VLD, FE_FRAME_VLD_CH2, 1);
		break;
	case 3:
		ISP_WR_BITS(preraw0, REG_PRE_RAW_FE_T, PRE_RAW_FRAME_VLD, FE_PQ_VLD_CH3, 1);
		ISP_WR_BITS(preraw0, REG_PRE_RAW_FE_T, PRE_RAW_FRAME_VLD, FE_FRAME_VLD_CH3, 1);
		break;
	default:
		break;
	}
}

void isp_post_trig(struct isp_ctx *ctx, enum cvi_isp_raw raw_num)
{
	uintptr_t isptopb = ctx->phys_regs[ISP_BLK_ID_ISPTOP];
	union REG_ISP_TOP_SW_CTRL_0 sw_ctrl_0;
	union REG_ISP_TOP_SW_CTRL_1 sw_ctrl_1;

	sw_ctrl_0.raw = sw_ctrl_1.raw = 0;

	if (ctx->is_slice_buf_on) { //slice buffer path
		vi_pr(VI_DBG, "dram->post trig raw_num(%d), is_slice_buf_on(%d)\n",
				raw_num, ctx->is_slice_buf_on);

		isp_slice_buf_trig(ctx, ISP_PRERAW0);
	} else {
		vi_pr(VI_DBG, "dram->post trig raw_num(%d), is_hdr_on(%d)\n",
				raw_num, ctx->isp_pipe_cfg[raw_num].is_hdr_on);
		if (!ctx->isp_pipe_cfg[raw_num].is_yuv_bypass_path) {
			if (ctx->isp_pipe_cfg[raw_num].is_hdr_on) {
				sw_ctrl_0.bits.SHAW_UP_BE	= 3;
				sw_ctrl_0.bits.TRIG_STR_BE	= 3;
				sw_ctrl_1.bits.PQ_UP_BE		= 3;
				sw_ctrl_0.bits.SHAW_UP_RAW	= 1;
				sw_ctrl_0.bits.TRIG_STR_RAW	= 1;
				sw_ctrl_1.bits.PQ_UP_RAW	= 1;
				sw_ctrl_0.bits.SHAW_UP_POST	= 1;
				sw_ctrl_0.bits.TRIG_STR_POST	= 1;
				sw_ctrl_1.bits.PQ_UP_POST	= 1;
			} else {
				sw_ctrl_0.bits.SHAW_UP_BE	= 1;
				sw_ctrl_0.bits.TRIG_STR_BE	= 1;
				sw_ctrl_1.bits.PQ_UP_BE		= 1;
				sw_ctrl_0.bits.SHAW_UP_RAW	= 1;
				sw_ctrl_0.bits.TRIG_STR_RAW	= 1;
				sw_ctrl_1.bits.PQ_UP_RAW	= 1;
				sw_ctrl_0.bits.SHAW_UP_POST	= 1;
				sw_ctrl_0.bits.TRIG_STR_POST	= 1;
				sw_ctrl_1.bits.PQ_UP_POST	= 1;
			}
		} else {
			sw_ctrl_0.bits.SHAW_UP_RAW	= 1;
			sw_ctrl_0.bits.TRIG_STR_RAW	= 1;
			sw_ctrl_1.bits.PQ_UP_RAW	= 1;
			sw_ctrl_0.bits.SHAW_UP_POST	= 1;
			sw_ctrl_0.bits.TRIG_STR_POST	= 1;
			sw_ctrl_1.bits.PQ_UP_POST	= 1;
		}

		ISP_WR_REG(isptopb, REG_ISP_TOP_T, SW_CTRL_1, sw_ctrl_1.raw);
		ISP_WR_REG(isptopb, REG_ISP_TOP_T, SW_CTRL_0, sw_ctrl_0.raw);
	}
}

/*********************************************************************************
 *	Common IPs for subsys
 ********************************************************************************/
uint64_t ispblk_dma_getaddr(struct isp_ctx *ctx, uint32_t dmaid)
{
	uintptr_t dmab = ctx->phys_regs[dmaid];
	uint64_t addr_h = ISP_RD_BITS(dmab, REG_ISP_DMA_CTL_T, SYS_CONTROL, BASEH);

	return ((uint64_t)ISP_RD_REG(dmab, REG_ISP_DMA_CTL_T, BASE_ADDR) | (addr_h << 32));
}

int ispblk_dma_buf_get_size(struct isp_ctx *ctx, int dmaid, u8 raw_num)
{
	uint32_t len = 0, num = 0, w;

	switch (dmaid) {
	case ISP_BLK_ID_DMA_CTL_CSI0_BDG0:
	case ISP_BLK_ID_DMA_CTL_CSI0_BDG1:
	case ISP_BLK_ID_DMA_CTL_CSI0_BDG2:
	case ISP_BLK_ID_DMA_CTL_CSI0_BDG3:
	case ISP_BLK_ID_DMA_CTL_CSI1_BDG0:
	case ISP_BLK_ID_DMA_CTL_CSI1_BDG1:
	case ISP_BLK_ID_DMA_CTL_CSI2_BDG0:
	{
		/* csibdg */
		if (dmaid == ISP_BLK_ID_DMA_CTL_CSI0_BDG0 || dmaid == ISP_BLK_ID_DMA_CTL_CSI0_BDG1 ||
			dmaid == ISP_BLK_ID_DMA_CTL_CSI0_BDG2 || dmaid == ISP_BLK_ID_DMA_CTL_CSI0_BDG3)
			raw_num = ISP_PRERAW0;
		else if (dmaid == ISP_BLK_ID_DMA_CTL_CSI1_BDG0 || dmaid == ISP_BLK_ID_DMA_CTL_CSI1_BDG1)
			raw_num = ISP_PRERAW1;
		else if (dmaid == ISP_BLK_ID_DMA_CTL_CSI2_BDG0)
			raw_num = ISP_PRERAW2;

		{
			w = ctx->isp_pipe_cfg[raw_num].crop.w;
			num = ctx->isp_pipe_cfg[raw_num].crop.h;
			if (ctx->is_dpcm_on)
				w >>= 1;

			len = 3 * UPPER(w, 1);
		}
		break;
	}
	case ISP_BLK_ID_DMA_CTL_PRE_RAW_VI_SEL_LE://pre_raw_vi_sel_le
	case ISP_BLK_ID_DMA_CTL_PRE_RAW_VI_SEL_SE: //pre_raw_vi_sel_se
	{
		if (ctx->is_slice_buf_on) {
			if (dmaid == ISP_BLK_ID_DMA_CTL_PRE_RAW_VI_SEL_LE)
				len = slc_b_cfg.main_path.le_buf_size;
			else
				len = slc_b_cfg.main_path.se_buf_size;
			num = 1;
		} else {
			u32 dpcm_on = (ctx->is_dpcm_on) ? 2 : 1;
			u32 w = 0;

			if (ctx->isp_pipe_cfg[raw_num].is_raw_replay) {
				w = ctx->isp_pipe_cfg[raw_num].csibdg_width;
				num = ctx->isp_pipe_cfg[raw_num].csibdg_height;
			} else {
				w = ctx->img_width;
				num = ctx->img_height;		
			}

			len = 3 * UPPER(w, dpcm_on);
		}
		break;
	}
	case ISP_BLK_ID_DMA_CTL_AF_W:
	{
		/* af */
		uint16_t block_num_x = 17, block_num_y = 15;

		len = (block_num_x * block_num_y) << 5;
		num = 1;

		break;
	}
	case ISP_BLK_ID_DMA_CTL_FE0_LSC_LE://lsc
	case ISP_BLK_ID_DMA_CTL_FE0_LSC_SE:
	case ISP_BLK_ID_DMA_CTL_FE1_LSC_LE:
	case ISP_BLK_ID_DMA_CTL_FE1_LSC_SE:
	case ISP_BLK_ID_DMA_CTL_FE2_LSC_LE:
	{
		num = 1;
		len = 10960;

		break;
	}
	case ISP_BLK_ID_DMA_CTL_FE0_AE_HIST_LE:
	case ISP_BLK_ID_DMA_CTL_FE0_AE_HIST_SE:
	case ISP_BLK_ID_DMA_CTL_FE1_AE_HIST_LE:
	case ISP_BLK_ID_DMA_CTL_FE1_AE_HIST_SE:
	case ISP_BLK_ID_DMA_CTL_FE2_AE_HIST_LE:
	{
		int ae_dma_counts, hist_dma_counts, faceae_dma_counts;

		ae_dma_counts		= 0x21C0;
		hist_dma_counts		= 0x2000;
		faceae_dma_counts	= 0x80;

		len = ae_dma_counts + hist_dma_counts + faceae_dma_counts;
		num = 1;

		break;
	}
	case ISP_BLK_ID_DMA_CTL_LSCR_HIST:
	{
		len = 1024;
		num = 1;

		break;
	}
	case ISP_BLK_ID_DMA_CTL_DRC_POLY_R:
	case ISP_BLK_ID_DMA_CTL_DRC_POLY_W:
	{
		len = 18432;
		num = 1;

		break;
	}
	case ISP_BLK_ID_DMA_CTL_DRC_HIST:
	{
		len = 512;
		num = 1;

		break;
	}
	case ISP_BLK_ID_DMA_CTL_LDCI_W: //ldci_iir_w
	case ISP_BLK_ID_DMA_CTL_LDCI_R: //ldci_iir_r
	{
		len = 3969;
		num = 0x1;

		break;
	}
	case ISP_BLK_ID_DMA_CTL_LDCI_HIST:
	{
		len = 1024;
		num = 0x1;

		break;
	}
	case ISP_BLK_ID_DMA_CTL_TNR_ST_C:
	case ISP_BLK_ID_DMA_CTL_TNR_LD_C:
	{
		//TNR UV
		if (ctx->is_fbc_on) {
			vi_fbc_calculate_size(ctx, raw_num);
			len = fbc_cfg.c_buf_size;
			num = 1;
		} else {
			len = (((((ctx->isp_pipe_cfg[raw_num].crop.w) << 3) + 127) >> 7) << 7) >> 3;
			num = ctx->isp_pipe_cfg[raw_num].crop.h >> 1;
		}

		break;
	}
	case ISP_BLK_ID_DMA_CTL_TNR_ST_Y:
	case ISP_BLK_ID_DMA_CTL_TNR_LD_Y:
	{
		//TNR Y
		if (ctx->is_fbc_on) {
			vi_fbc_calculate_size(ctx, raw_num);
			len = fbc_cfg.y_buf_size;
			num = 1;
		} else {
			len = (((((ctx->isp_pipe_cfg[raw_num].crop.w) << 3) + 127) >> 7) << 7) >> 3;
			num = ctx->isp_pipe_cfg[raw_num].crop.h;
		}

		break;
	}
	case ISP_BLK_ID_DMA_CTL_TNR_ST_MV:
	case ISP_BLK_ID_DMA_CTL_TNR_LD_MV:
	{
		// ((IMG_W*8+127)/128)*128/8  (byte)
		// (W/8)*(H/2)*2
		len = (((((ctx->isp_pipe_cfg[raw_num].crop.w) << 3) + 127) >> 7) << 7) >> 3 >> 3;
		num = ctx->isp_pipe_cfg[raw_num].crop.h ;
		break;
	}
	case ISP_BLK_ID_DMA_CTL_TNR_ST_MO:
	case ISP_BLK_ID_DMA_CTL_TNR_LD_MO:
	{
		// ((IMG_W*8+127)/128)*128/8  (byte)
		// (W/16)*(H/16)/8
		len = (((((ctx->isp_pipe_cfg[raw_num].crop.w) << 3) + 127) >> 7) << 7) >> 3 >> 4;
		num = ctx->isp_pipe_cfg[raw_num].crop.h >> 4 >> 3;
		break;
	}
	case ISP_BLK_ID_DMA_CTL_TNR_ST_FCB:
	case ISP_BLK_ID_DMA_CTL_TNR_LD_FCB:
	{
		// (W/16)*(H/16)
		len = ctx->isp_pipe_cfg[raw_num].crop.w >> 4;
		num = ctx->isp_pipe_cfg[raw_num].crop.h >> 4;

		break;
	}
	case ISP_BLK_ID_DMA_CTL_TNR_ST_MSP:
	{
		// (W/16)*(H/16)/4
		len = ctx->isp_pipe_cfg[raw_num].crop.w >> 4;
		num = ctx->isp_pipe_cfg[raw_num].crop.h >> 4 >> 2;

		break;
	}
	case ISP_BLK_ID_DMA_CTL_CNR_Y_W:
	case ISP_BLK_ID_DMA_CTL_CNR_Y_R:
	{
		u8 sd = ctx->cnr_scale_ratio;
		len = ctx->img_width * ctx->img_height / sd / sd;
		num = 1;

		break;
	}
	case ISP_BLK_ID_DMA_CTL_CNR_C_W:
	case ISP_BLK_ID_DMA_CTL_CNR_C_R:
	{
		u8 sd = ctx->cnr_scale_ratio;
		len = ctx->img_width * ctx->img_height * 2 / sd / sd;
		num = 1;

		break;
	}
	default:
		break;
	}

	len = VI_ALIGN(len);

	vi_pr(VI_INFO, "dmaid=%d, size=%d\n", dmaid, len * num);

	return len * num;
}

void ispblk_dma_setaddr(struct isp_ctx *ctx, uint32_t dmaid, uint64_t buf_addr)
{
	uintptr_t dmab = ctx->phys_regs[dmaid];

	ISP_WR_REG(dmab, REG_ISP_DMA_CTL_T, BASE_ADDR, (buf_addr & 0xffffffff));
	ISP_WR_BITS(dmab, REG_ISP_DMA_CTL_T, SYS_CONTROL, BASEH, ((buf_addr >> 32) & 0xffffffff));
}

void ispblk_dma_set_sw_mode(struct isp_ctx *ctx, uint32_t dmaid, bool is_sw_mode)
{
	uintptr_t dmab = ctx->phys_regs[dmaid];
	union REG_ISP_DMA_CTL_SYS_CONTROL sys_ctrl;

	//SW mode: config by SW
	sys_ctrl.raw = ISP_RD_REG(dmab, REG_ISP_DMA_CTL_T, SYS_CONTROL);
	sys_ctrl.bits.BASE_SEL		= 0x1;
	sys_ctrl.bits.STRIDE_SEL	= is_sw_mode;
	sys_ctrl.bits.SEGLEN_SEL	= is_sw_mode;
	sys_ctrl.bits.SEGNUM_SEL	= is_sw_mode;
	ISP_WR_REG(dmab, REG_ISP_DMA_CTL_T, SYS_CONTROL, sys_ctrl.raw);
}

int ispblk_dma_config(struct isp_ctx *ctx, int dmaid, uint64_t buf_addr)
{
	uintptr_t dmab = ctx->phys_regs[dmaid];
	uint32_t w = 0, len = 0, stride = 0, num = 0;
	enum cvi_isp_raw raw_num = ISP_PRERAW0;

	switch (dmaid) {
	case ISP_BLK_ID_DMA_CTL_PRE_RAW_VI_SEL_LE://pre_raw_vi_sel_le
	case ISP_BLK_ID_DMA_CTL_PRE_RAW_VI_SEL_SE: //pre_raw_vi_sel_se
	{
		// preraw read dma
		u32 dpcm_on = (ctx->is_dpcm_on) ? 2 : 1;

		if (ctx->is_slice_buf_on) {
			if (dmaid == ISP_BLK_ID_DMA_CTL_PRE_RAW_VI_SEL_LE)
				len = slc_b_cfg.main_path.le_buf_size;
			else
				len = slc_b_cfg.main_path.se_buf_size;
			num = 1;
		} else {
			u32 w = 0;

			if (ctx->isp_pipe_cfg[raw_num].is_raw_replay) {
				w = ctx->isp_pipe_cfg[raw_num].csibdg_width;
				num = ctx->isp_pipe_cfg[raw_num].csibdg_height;
			} else {
				w = 2560;
				num = 1440;		
			}

			len = 3 * UPPER(w, dpcm_on);
		}
		stride = len;
		vi_pr(VI_INFO, "dmaid=%d, len=%d, num:%d\n", dmaid, len, num);

		break;
	}
	case ISP_BLK_ID_DMA_CTL_CSI0_BDG0:
	case ISP_BLK_ID_DMA_CTL_CSI0_BDG1:
	case ISP_BLK_ID_DMA_CTL_CSI0_BDG2:
	case ISP_BLK_ID_DMA_CTL_CSI0_BDG3:
	case ISP_BLK_ID_DMA_CTL_CSI1_BDG0:
	case ISP_BLK_ID_DMA_CTL_CSI1_BDG1:
	case ISP_BLK_ID_DMA_CTL_CSI2_BDG0:
	{	/* csibdg */
		if (dmaid == ISP_BLK_ID_DMA_CTL_CSI0_BDG0 || dmaid == ISP_BLK_ID_DMA_CTL_CSI0_BDG1 ||
			dmaid == ISP_BLK_ID_DMA_CTL_CSI0_BDG2 || dmaid == ISP_BLK_ID_DMA_CTL_CSI0_BDG3)
			raw_num = ISP_PRERAW0;
		else if (dmaid == ISP_BLK_ID_DMA_CTL_CSI1_BDG0 || dmaid == ISP_BLK_ID_DMA_CTL_CSI1_BDG1)
			raw_num = ISP_PRERAW1;
		else if (dmaid == ISP_BLK_ID_DMA_CTL_CSI2_BDG0)
			raw_num = ISP_PRERAW2;

		{
			w = ctx->isp_pipe_cfg[raw_num].csibdg_width;
			num = ctx->isp_pipe_cfg[raw_num].csibdg_height;
			if (ctx->is_dpcm_on)
				w >>= 1;

			len = 3 * UPPER(w, 1);
			stride = len;
		}
		vi_pr(VI_INFO, "dmaid=%d, len=%d, num:%d\n", dmaid, len, num);

		break;
	}
	case ISP_BLK_ID_DMA_CTL_AF_W:
	{
		/* af */
		uint16_t block_num_x = 17, block_num_y = 15;

		num = 1;
		len = (block_num_x * block_num_y) << 5;
		stride = len;

		break;
	}
	case ISP_BLK_ID_DMA_CTL_FE0_LSC_LE:
	case ISP_BLK_ID_DMA_CTL_FE0_LSC_SE:
	case ISP_BLK_ID_DMA_CTL_FE1_LSC_LE:
	case ISP_BLK_ID_DMA_CTL_FE1_LSC_SE:
	case ISP_BLK_ID_DMA_CTL_FE2_LSC_LE:
	{
		/* lsc rdma */
		// fixed value for 10960

		num = 1;
		len = 10960;
		stride = len;

		break;
	}
	case ISP_BLK_ID_DMA_CTL_FE0_AE_HIST_LE:
	case ISP_BLK_ID_DMA_CTL_FE0_AE_HIST_SE:
	case ISP_BLK_ID_DMA_CTL_FE1_AE_HIST_LE:
	case ISP_BLK_ID_DMA_CTL_FE1_AE_HIST_SE:
	case ISP_BLK_ID_DMA_CTL_FE2_AE_HIST_LE:
	{
		int ae_dma_counts, hist_dma_counts, faceae_dma_counts;

		ae_dma_counts		= 0x21C0;
		hist_dma_counts		= 0x2000;
		faceae_dma_counts	= 0x80;

		num = 1;
		len = ae_dma_counts + hist_dma_counts + faceae_dma_counts;
		stride = len;

		break;
	}
	case ISP_BLK_ID_DMA_CTL_LSCR_HIST:
	{
		len = 1024;
		num = 1;
		stride = len;

		break;
	}
	case ISP_BLK_ID_DMA_CTL_DRC_POLY_R:
	case ISP_BLK_ID_DMA_CTL_DRC_POLY_W:
	{
		len = 18432;
		num = 1;
		stride = len;

		break;
	}
	case ISP_BLK_ID_DMA_CTL_DRC_HIST:
	{
		len = 512;
		num = 1;
		stride = len;

		break;
	}
	case ISP_BLK_ID_DMA_CTL_LDCI_W: //ldci_iir_w
	case ISP_BLK_ID_DMA_CTL_LDCI_R: //ldci_iir_r
	{
		len = 3969;
		num = 0x1;
		stride = len;

		break;
	}
	case ISP_BLK_ID_DMA_CTL_LDCI_HIST:
	{
		len = 1024;
		num = 0x1;
		stride = len;

		break;
	}
	case ISP_BLK_ID_DMA_CTL_TNR_ST_C:
	case ISP_BLK_ID_DMA_CTL_TNR_LD_C:
	{
		//TNR UV
		if (ctx->is_fbc_on) {
			vi_fbc_calculate_size(ctx, raw_num);
			len = fbc_cfg.c_bs_size;
			num = 1;
		} else {
			len = (((((ctx->isp_pipe_cfg[raw_num].crop.w) << 3) + 127) >> 7) << 7) >> 3;
			num = ctx->isp_pipe_cfg[raw_num].crop.h >> 1;
		}
		stride = len;

		break;
	}
	case ISP_BLK_ID_DMA_CTL_TNR_ST_Y:
	case ISP_BLK_ID_DMA_CTL_TNR_LD_Y:
	{
		//TNR Y
		if (ctx->is_fbc_on) {
			vi_fbc_calculate_size(ctx, raw_num);
			len = fbc_cfg.y_bs_size;
			num = 1;
		} else {
			len = (((((ctx->isp_pipe_cfg[raw_num].crop.w) << 3) + 127) >> 7) << 7) >> 3;
			num = ctx->isp_pipe_cfg[raw_num].crop.h;
		}
		stride = len;

		break;
	}
	case ISP_BLK_ID_DMA_CTL_TNR_ST_MO:
	case ISP_BLK_ID_DMA_CTL_TNR_LD_MO:
	{
		len = (((((ctx->isp_pipe_cfg[raw_num].crop.w) << 3) + 127) >> 7) << 7) >> 3;
		num = ctx->isp_pipe_cfg[raw_num].crop.h;
		stride = len;
		break;
	}
	case ISP_BLK_ID_DMA_CTL_TNR_ST_FCB:
	case ISP_BLK_ID_DMA_CTL_TNR_LD_FCB:
	{
		// (W/16)*(H/16)
		len = ctx->isp_pipe_cfg[raw_num].crop.w >> 4;
		num = ctx->isp_pipe_cfg[raw_num].crop.h >> 4;

		stride = len;
		break;
	}
	case ISP_BLK_ID_DMA_CTL_TNR_ST_MSP:
	{
		// (W/16)*(H/16)/4
		len = ctx->isp_pipe_cfg[raw_num].crop.w >> 4;
		num = ctx->isp_pipe_cfg[raw_num].crop.h >> 4 >> 2;
		stride = len;

		break;
	}
	case ISP_BLK_ID_DMA_CTL_CNR_Y_W:
	case ISP_BLK_ID_DMA_CTL_CNR_Y_R:
	{
		u8 sd = ctx->cnr_scale_ratio;
		len = ctx->img_width * ctx->img_height / sd / sd;
		num = 1;
		stride = len;

		break;
	}
	case ISP_BLK_ID_DMA_CTL_CNR_C_W:
	case ISP_BLK_ID_DMA_CTL_CNR_C_R:
	{
		u8 sd = ctx->cnr_scale_ratio;
		len = ctx->img_width * ctx->img_height * 2 / sd / sd;
		num = 1;
		stride = len;

		break;
	}
	case ISP_BLK_ID_DMA_CTL_YUV_CROP_Y:
		//yuvtop y out
		len = (ctx->isp_pipe_cfg[raw_num].postout_crop.w) ?
			ctx->isp_pipe_cfg[raw_num].postout_crop.w : ctx->img_width;
		num = (ctx->isp_pipe_cfg[raw_num].postout_crop.h) ?
			ctx->isp_pipe_cfg[raw_num].postout_crop.h : ctx->img_height;
		stride = len;
		vi_pr(VI_INFO, "dmaid=%d, len=%d, num:%d\n", dmaid, len, num);

		break;
	case ISP_BLK_ID_DMA_CTL_YUV_CROP_C:
		//yuvtop uv out
		len = (ctx->isp_pipe_cfg[raw_num].postout_crop.w) ?
			ctx->isp_pipe_cfg[raw_num].postout_crop.w : ctx->img_width;
		num = (ctx->isp_pipe_cfg[raw_num].postout_crop.h) ?
			(ctx->isp_pipe_cfg[raw_num].postout_crop.h >> 1) : (ctx->img_height >> 1);
		stride = len;
		vi_pr(VI_INFO, "dmaid=%d, len=%d, num:%d\n", dmaid, len, num);

		break;
	default:
		break;
	}

	len = VI_ALIGN(len);
	stride = VI_ALIGN(stride);

	ISP_WR_REG(dmab, REG_ISP_DMA_CTL_T, DMA_SEGLEN, len);
	ISP_WR_REG(dmab, REG_ISP_DMA_CTL_T, DMA_STRIDE, stride);
	ISP_WR_REG(dmab, REG_ISP_DMA_CTL_T, DMA_SEGNUM, num);

	if (buf_addr)
		ispblk_dma_setaddr(ctx, dmaid, buf_addr);

	return len * num;
}

void ispblk_dma_enable(struct isp_ctx *ctx, uint32_t dmaid, uint32_t on, uint8_t dma_disable)
{
	uintptr_t srcb = 0;

	switch (dmaid) {
	case ISP_BLK_ID_DMA_CTL_YUV_CROP_Y:
		/* yuvtop y crop4 */
		srcb = ctx->phys_regs[ISP_BLK_ID_YUV_CROP_Y];
		break;
	case ISP_BLK_ID_DMA_CTL_YUV_CROP_C:
		/* yuvtop uv crop5 */
		srcb = ctx->phys_regs[ISP_BLK_ID_YUV_CROP_C];
		break;
	default:
		break;
	}

	if (srcb) {
		ISP_WR_BITS(srcb, REG_CROP_T, REG_0, DMA_ENABLE, on);
		ISP_WR_BITS(srcb, REG_CROP_T, DEBUG, FORCE_DMA_DISABLE, dma_disable);
	}
}

void ispblk_crop_enable(struct isp_ctx *ctx, int crop_id, bool en)
{
	uintptr_t cropb = ctx->phys_regs[crop_id];

	ISP_WR_BITS(cropb, REG_CROP_T, REG_0, CROP_ENABLE, en);
}

int ispblk_crop_config(struct isp_ctx *ctx, int crop_id, struct vi_rect crop)
{
	uintptr_t cropb = ctx->phys_regs[crop_id];
	union REG_CROP_1 reg1;
	union REG_CROP_2 reg2;

	// crop out size
	reg1.bits.CROP_START_Y = crop.y;
	reg1.bits.CROP_END_Y = crop.y + crop.h - 1;
	reg2.bits.CROP_START_X = crop.x;
	reg2.bits.CROP_END_X = crop.x + crop.w - 1;
	ISP_WR_REG(cropb, REG_CROP_T, REG_1, reg1.raw);
	ISP_WR_REG(cropb, REG_CROP_T, REG_2, reg2.raw);

	return 0;
}

int csibdg_find_hwid(enum cvi_isp_raw raw_num)
{
	int csibdg_id = -1;

	switch (raw_num) {
	case ISP_PRERAW0:
		csibdg_id = ISP_BLK_ID_CSIBDG0;
		break;
	case ISP_PRERAW1:
		csibdg_id = ISP_BLK_ID_CSIBDG1;
		break;
	case ISP_PRERAW2:
		csibdg_id = ISP_BLK_ID_CSIBDG2;
		break;
	default:
		break;
	}

	return csibdg_id;
}

int csibdg_dma_find_hwid(enum cvi_isp_raw raw_num, bool is_le)
{
	int dma_id = -1;

	switch (raw_num) {
	case ISP_PRERAW0:
		dma_id = is_le ? ISP_BLK_ID_DMA_CTL_CSI0_BDG0 : ISP_BLK_ID_DMA_CTL_CSI0_BDG1;
		break;
	case ISP_PRERAW1:
		dma_id = is_le ? ISP_BLK_ID_DMA_CTL_CSI1_BDG0 : ISP_BLK_ID_DMA_CTL_CSI1_BDG1;
		break;
	case ISP_PRERAW2:
		dma_id = ISP_BLK_ID_DMA_CTL_CSI2_BDG0;
		break;
	default:
		break;
	}

	return dma_id;
}

int fe_find_hwid(enum cvi_isp_raw raw_num)
{
	int fe_id = -1;

	switch (raw_num) {
	case ISP_PRERAW0:
		fe_id = ISP_BLK_ID_PRE_RAW_FE0;
		break;
	case ISP_PRERAW1:
		fe_id = ISP_BLK_ID_PRE_RAW_FE1;
		break;
	case ISP_PRERAW2:
		fe_id = ISP_BLK_ID_PRE_RAW_FE2;
		break;
	default:
		break;
	}

	return fe_id;
}

int lsc_find_hwid(enum cvi_isp_raw raw_num, bool is_le)
{
	int lsc_id = -1;

	switch (raw_num) {
	case ISP_PRERAW0:
		lsc_id = is_le ? ISP_BLK_ID_PRE_RAW_FE0_LSC0 : ISP_BLK_ID_PRE_RAW_FE0_LSC1;
		break;
	case ISP_PRERAW1:
		lsc_id = is_le ? ISP_BLK_ID_PRE_RAW_FE1_LSC0 : ISP_BLK_ID_PRE_RAW_FE1_LSC1;
		break;
	case ISP_PRERAW2:
		lsc_id = ISP_BLK_ID_PRE_RAW_FE2_LSC0;
		break;
	default:
		break;
	}

	return lsc_id;
}

int ae_find_hwid(enum cvi_isp_raw raw_num)
{
	int ae_id = -1;

	switch (raw_num) {
	case ISP_PRERAW0:
		ae_id = ISP_BLK_ID_AE_HIST_FE0;
		break;
	case ISP_PRERAW1:
		ae_id = ISP_BLK_ID_AE_HIST_FE1;
		break;
	case ISP_PRERAW2:
		ae_id = ISP_BLK_ID_AE_HIST_FE2;
		break;
	default:
		break;
	}

	return ae_id;
}

int ae_dma_find_hwid(enum cvi_isp_raw raw_num, bool is_le)
{
	int dma_id = -1;

	switch (raw_num) {
	case ISP_PRERAW0:
		dma_id = is_le ? ISP_BLK_ID_DMA_CTL_FE0_AE_HIST_LE : ISP_BLK_ID_DMA_CTL_FE0_AE_HIST_SE;
		break;
	case ISP_PRERAW1:
		dma_id = is_le ? ISP_BLK_ID_DMA_CTL_FE1_AE_HIST_LE : ISP_BLK_ID_DMA_CTL_FE1_AE_HIST_SE;
		break;
	case ISP_PRERAW2:
		dma_id = ISP_BLK_ID_DMA_CTL_FE2_AE_HIST_LE;
		break;
	default:
		break;
	}

	return dma_id;
}

/****************************************************************************
 *	Runtime Control Flow Config
 ****************************************************************************/
void isp_first_frm_reset(struct isp_ctx *ctx, uint8_t reset)
{
	uintptr_t isptopb = ctx->phys_regs[ISP_BLK_ID_ISPTOP];
	return;
	vi_pr(VI_DBG, "reset[%d]\n", reset);
	//0: reg_first_frame_reset(in IP)
	//1: reg_first_frame_sw
	ISP_WR_BITS(isptopb, REG_ISP_TOP_T, FIRST_FRAME, FIRST_FRAME_TOP, reset ? 0x1f : 0x0);

	//reg_first_frame_sw
	//[0]: LTM
	//[1]: LDCI
	//[2]: TNR
	//[3]: MMAP
	ISP_WR_BITS(isptopb, REG_ISP_TOP_T, FIRST_FRAME, FIRST_FRAME_SW, reset ? 0x1f : 0x0);
}

void _ispblk_isptop_cfg_update(struct isp_ctx *ctx, const enum cvi_isp_raw raw_num)
{
	// uintptr_t isptopb = ctx->phys_regs[ISP_BLK_ID_ISPTOP];
	// union REG_ISP_TOP_SCENARIOS_CTRL scene_ctrl;

	// scene_ctrl.raw = ISP_RD_REG(isptopb, REG_ISP_TOP_T, SCENARIOS_CTRL);

	// if (!ctx->is_slice_buf_on) {
	// 	if (ctx->isp_pipe_cfg[raw_num].is_yuv_bypass_path) { //YUV sensor
	// 		scene_ctrl.bits.RAW2YUV_422_ENABLE = 1;
	// 		scene_ctrl.bits.DCI_RGB0YUV1 = 1;
	// 	} else { //RGB sensor
	// 		scene_ctrl.bits.RAW2YUV_422_ENABLE = 0;
	// 		scene_ctrl.bits.DCI_RGB0YUV1 = 0;
	// 	}
	// } else if (_is_fe_post_offline(ctx)) {
	// 	if (ctx->isp_pipe_cfg[raw_num].is_yuv_bypass_path) { //YUV sensor
	// 		scene_ctrl.bits.RAW2YUV_422_ENABLE = 1;
	// 		scene_ctrl.bits.DCI_RGB0YUV1 = 1;

	// 		scene_ctrl.bits.BE2RAW_L_ENABLE = 0;
	// 		scene_ctrl.bits.BE2RAW_S_ENABLE = 0;
	// 		scene_ctrl.bits.BE_RDMA_L_ENABLE = 0;
	// 		scene_ctrl.bits.BE_RDMA_S_ENABLE = 0;
	// 	} else { //RGB sensor
	// 		scene_ctrl.bits.RAW2YUV_422_ENABLE = 0;
	// 		scene_ctrl.bits.DCI_RGB0YUV1 = 0;

	// 		scene_ctrl.bits.BE2RAW_L_ENABLE = 1;
	// 		scene_ctrl.bits.BE2RAW_S_ENABLE = ctx->isp_pipe_cfg[raw_num].is_hdr_on;
	// 		scene_ctrl.bits.BE_RDMA_L_ENABLE = 1;
	// 		scene_ctrl.bits.BE_RDMA_S_ENABLE = ctx->isp_pipe_cfg[raw_num].is_hdr_on;
	// 	}
	// }

	// ISP_WR_REG(isptopb, REG_ISP_TOP_T, SCENARIOS_CTRL, scene_ctrl.raw);
}

//TODO rgb+yuv sensor
#if 0
void _ispblk_rawtop_cfg_update(struct isp_ctx *ctx, const enum cvi_isp_raw raw_num)
{
	uintptr_t rawtop = ctx->phys_regs[ISP_BLK_ID_RAWTOP];
	uintptr_t raw_rdma = ctx->phys_regs[ISP_BLK_ID_RAW_RDMA];
	uintptr_t lsc = ctx->phys_regs[ISP_BLK_ID_LSC];
	union REG_RAW_TOP_PATGEN1 patgen1;

	if (ctx->isp_pipe_cfg[raw_num].is_yuv_bypass_path) { //YUV sensor
		//Disable lsc
		ISP_WR_BITS(lsc, REG_ISP_LSC_T, LSC_ENABLE, LSC_ENABLE, 0);
		ISP_WR_BITS(lsc, REG_ISP_LSC_T, DMI_ENABLE, DMI_ENABLE, 0);

		ISP_WR_BITS(rawtop, REG_RAW_TOP_T, RDMI_ENABLE, CH_NUM, 0);
		ISP_WR_BITS(rawtop, REG_RAW_TOP_T, RDMI_ENABLE, RDMI_EN, 1);
		ISP_WO_BITS(rawtop, REG_RAW_TOP_T, CTRL, LS_CROP_DST_SEL, 1);
		ISP_WO_BITS(rawtop, REG_RAW_TOP_T, RAW_4, YUV_IN_MODE, 1);

		ISP_WR_BITS(rawtop, REG_RAW_TOP_T, DPCM_MODE, DPCM_MODE, 0);
		ISP_WR_BITS(rawtop, REG_RAW_TOP_T, DPCM_MODE, DPCM_XSTR, 0);

		if (_is_fe_post_offline(ctx)) {
			ISP_WR_BITS(raw_rdma, REG_RAW_RDMA_CTRL_T, CONFIG, LE_RDMA_EN, 1);
			ISP_WR_BITS(raw_rdma, REG_RAW_RDMA_CTRL_T, CONFIG, SE_RDMA_EN, 0);
			ISP_WR_BITS(raw_rdma, REG_RAW_RDMA_CTRL_T, RDMA_SIZE, RDMI_WIDTHM1, ctx->img_width - 1);
			ISP_WR_BITS(raw_rdma, REG_RAW_RDMA_CTRL_T, RDMA_SIZE, RDMI_HEIGHTM1, ctx->img_height - 1);
			ISP_WR_BITS(raw_rdma, REG_RAW_RDMA_CTRL_T, DPCM_MODE, DPCM_MODE, 0);
			ISP_WR_BITS(raw_rdma, REG_RAW_RDMA_CTRL_T, DPCM_MODE, DPCM_XSTR, 0);
		}
	} else { //RGB sensor
		if (_is_fe_post_offline(ctx)) //fe->dram->post
			ISP_WR_BITS(rawtop, REG_RAW_TOP_T, RDMI_ENABLE, RDMI_EN, 0);

		ISP_WO_BITS(rawtop, REG_RAW_TOP_T, CTRL, LS_CROP_DST_SEL, 0);
		ISP_WO_BITS(rawtop, REG_RAW_TOP_T, RAW_4, YUV_IN_MODE, 0);
	}

	ISP_WR_BITS(rawtop, REG_RAW_TOP_T, RDMA_SIZE, RDMI_WIDTHM1, ctx->img_width - 1);
	ISP_WR_BITS(rawtop, REG_RAW_TOP_T, RDMA_SIZE, RDMI_HEIGHTM1, ctx->img_height - 1);
	ISP_WR_BITS(rawtop, REG_RAW_TOP_T, RAW_2, IMG_WIDTHM1, ctx->img_width - 1);
	ISP_WR_BITS(rawtop, REG_RAW_TOP_T, RAW_2, IMG_HEIGHTM1, ctx->img_height - 1);
}

void _ispblk_rgbtop_cfg_update(struct isp_ctx *ctx, const enum cvi_isp_raw raw_num)
{
	uintptr_t rgbtop = ctx->phys_regs[ISP_BLK_ID_RGBTOP];

	ISP_WR_BITS(rgbtop, REG_ISP_RGB_TOP_T, REG_9, RGBTOP_IMGW_M1, ctx->img_width - 1);
	ISP_WR_BITS(rgbtop, REG_ISP_RGB_TOP_T, REG_9, RGBTOP_IMGH_M1, ctx->img_height - 1);
}

void _ispblk_yuvtop_cfg_update(struct isp_ctx *ctx, const enum cvi_isp_raw raw_num)
{
	uintptr_t yuvtop = ctx->phys_regs[ISP_BLK_ID_YUVTOP];
	uintptr_t tnr = ctx->phys_regs[ISP_BLK_ID_TNR];
	uintptr_t dither = ctx->phys_regs[ISP_BLK_ID_YUV_DITHER];
	uintptr_t cnr = ctx->phys_regs[ISP_BLK_ID_CNR];
	uintptr_t pre_ee = ctx->phys_regs[ISP_BLK_ID_PRE_EE_FRONT];
	uintptr_t ee = ctx->phys_regs[ISP_BLK_ID_POST_EE];
	uintptr_t ycur = ctx->phys_regs[ISP_BLK_ID_YCURVE];
	uintptr_t ldci = ctx->phys_regs[ISP_BLK_ID_LDCI];
	uintptr_t cacp = ctx->phys_regs[ISP_BLK_ID_CA];
	uintptr_t ca_lite = ctx->phys_regs[ISP_BLK_ID_CA_LITE];

	if (ctx->isp_pipe_cfg[raw_num].is_yuv_bypass_path) { //YUV sensor
		//Disable 3DNR dma
		ISP_WR_BITS(tnr, REG_ISP_444_422_T, REG_8, FORCE_DMA_DISABLE, 0x3f);
		ISP_WR_BITS(tnr, REG_ISP_444_422_T, REG_4, REG_422_444, 1);
		ISP_WR_BITS(tnr, REG_ISP_444_422_T, REG_5, TDNR_ENABLE, 0);
		ISP_WR_BITS(tnr, REG_ISP_444_422_T, REG_5, FORCE_MONO_ENABLE, 0);
		//Because the yuv sensor don't pass the 444->422 module
		//Therefore the format output from 3dnr is yvu, need to swap to yuv
		//workaround, hw yuvtop's bug, only support yuyv and yvyu
		//for uyvy and vyuy, should set csi_ctrl_top's csi_format_frc[1] and csi_format_set[raw_16]
		// if (ctx->isp_pipe_cfg[raw_num].enDataSeq == VI_DATA_SEQ_UYVY ||
		// 	ctx->isp_pipe_cfg[raw_num].enDataSeq == VI_DATA_SEQ_YUYV)
		// 	ISP_WR_BITS(tnr, REG_ISP_444_422_T, REG_4, SWAP, 3);
		// else
			ISP_WR_BITS(tnr, REG_ISP_444_422_T, REG_4, SWAP, 0);

		//Disable yuv dither
		ISP_WR_BITS(dither, REG_ISP_YUV_DITHER_T, Y_DITHER, Y_DITHER_ENABLE, 0);
		ISP_WR_BITS(dither, REG_ISP_YUV_DITHER_T, UV_DITHER, UV_DITHER_ENABLE, 0);
		//Disable cnr
		ISP_WR_BITS(cnr, REG_ISP_CNR_T, CNR_ENABLE, CNR_ENABLE, 0);
		ISP_WR_BITS(cnr, REG_ISP_CNR_T, CNR_ENABLE, PFC_ENABLE, 0);
		//Disable pre_ee
		ISP_WR_BITS(pre_ee, REG_ISP_EE_T, REG_00, EE_ENABLE, 0);
		//Disable ee
		ISP_WR_BITS(ee, REG_ISP_EE_T, REG_00, EE_ENABLE, 0);
		//Disable ycurv
		ISP_WR_BITS(ycur, REG_ISP_YCURV_T, YCUR_CTRL, YCUR_ENABLE, 0);
		//Disable dci
		ISP_WR_BITS(dci, REG_ISP_DCI_T, DCI_ENABLE, DCI_ENABLE, 0);
		//Disable ldci
		ISP_WR_BITS(ldci, REG_ISP_LDCI_T, LDCI_ENABLE, LDCI_ENABLE, 0);
		ISP_WR_BITS(ldci, REG_ISP_LDCI_T, DMI_ENABLE, DMI_ENABLE, 0);
		//Disable cacp
		ISP_WR_BITS(cacp, REG_ISP_CA_T, REG_00, CACP_ENABLE, 0);
		//Disable ca_lite
		ISP_WR_BITS(ca_lite, REG_ISP_CA_LITE_T, REG_00, CA_LITE_ENABLE, 0);

		if (ctx->isp_pipe_cfg[raw_num].is_offline_scaler) {
			ispblk_dma_enable(ctx, ISP_BLK_ID_DMA_CTL_YUV_CROP_Y, true, false);
			ispblk_dma_enable(ctx, ISP_BLK_ID_DMA_CTL_YUV_CROP_C, true, false);
		} else {
			ispblk_dma_enable(ctx, ISP_BLK_ID_DMA_CTL_YUV_CROP_Y, false, false);
			ispblk_dma_enable(ctx, ISP_BLK_ID_DMA_CTL_YUV_CROP_C, false, false);
		}
	} else { //RGB sensor
		//Enable 3DNR dma
		ISP_WR_BITS(tnr, REG_ISP_444_422_T, REG_8, FORCE_DMA_DISABLE, 0);
		ISP_WR_BITS(tnr, REG_ISP_444_422_T, REG_4, REG_422_444, 0);
		ISP_WR_BITS(tnr, REG_ISP_444_422_T, REG_5, TDNR_ENABLE, 1);
		ISP_WR_BITS(tnr, REG_ISP_444_422_T, REG_4, SWAP, 0);
	}

	ISP_WR_BITS(yuvtop, REG_YUV_TOP_T, IMGW_M1, YUV_TOP_IMGW_M1, ctx->img_width - 1);
	ISP_WR_BITS(yuvtop, REG_YUV_TOP_T, IMGW_M1, YUV_TOP_IMGH_M1, ctx->img_height - 1);

	//bypass_v = 1 -> 422P online to scaler
	ISP_WR_BITS(yuvtop, REG_YUV_TOP_T, YUV_CTRL, BYPASS_V, !ctx->isp_pipe_cfg[raw_num].is_offline_scaler);
}
#endif

void ispblk_post_yuv_cfg_update(struct isp_ctx *ctx, const enum cvi_isp_raw raw_num)
{
	// _ispblk_isptop_cfg_update(ctx, raw_num);
	// _ispblk_rawtop_cfg_update(ctx, raw_num);
	// _ispblk_rgbtop_cfg_update(ctx, raw_num);
	// _ispblk_yuvtop_cfg_update(ctx, raw_num);
}

void ispblk_post_cfg_update(struct isp_ctx *ctx, const enum cvi_isp_raw raw_num)
{
	ispblk_rawtop_config(ctx, raw_num);
	ispblk_rgbtop_config(ctx, raw_num);
	ispblk_yuvtop_config(ctx, raw_num);
}

int ispblk_dma_get_size(struct isp_ctx *ctx, int dmaid, uint32_t _w, uint32_t _h)
{
	uint32_t len = 0, num = 0, w;

	switch (dmaid) {
	case ISP_BLK_ID_DMA_CTL_CSI0_BDG0:
	case ISP_BLK_ID_DMA_CTL_CSI0_BDG1:
	case ISP_BLK_ID_DMA_CTL_CSI0_BDG2:
	case ISP_BLK_ID_DMA_CTL_CSI0_BDG3:
	case ISP_BLK_ID_DMA_CTL_CSI1_BDG0:
	case ISP_BLK_ID_DMA_CTL_CSI1_BDG1:
	case ISP_BLK_ID_DMA_CTL_CSI2_BDG0:
	{	w = _w;
		num = _h;
		if (ctx->is_dpcm_on)
			w >>= 1;

		len = 3 * UPPER(w, 1);

		break;
	}
	default:
		break;
	}

	len = VI_ALIGN(len);

	vi_pr(VI_INFO, "dmaid=%d, size=%d\n", dmaid, len * num);

	return len * num;
}

#if 0
int isp_frm_err_handler(struct isp_ctx *ctx, const enum cvi_isp_raw err_raw_num, const u8 step)
{
	uintptr_t isptopb = ctx->phys_regs[ISP_BLK_ID_ISPTOP];
	union REG_ISP_TOP_SW_RST sw_rst;
	union VIP_SYS_RESET vip_rst;

	if (step == 1) {
		uintptr_t fe;

		if (err_raw_num == ISP_PRERAW0)
			fe = ctx->phys_regs[ISP_BLK_ID_PRE_RAW_FE0];
		else if (err_raw_num == ISP_PRERAW_B)
			fe = ctx->phys_regs[ISP_BLK_ID_PRE_RAW_FE1];
		else
			fe = ctx->phys_regs[ISP_BLK_ID_PRE_RAW_FE2];

		ISP_WR_BITS(fe, REG_PRE_RAW_FE_T, PRE_RAW_FRAME_VLD, FE_FRAME_VLD_CH0, 0);
		ISP_WR_BITS(fe, REG_PRE_RAW_FE_T, PRE_RAW_FRAME_VLD, FE_FRAME_VLD_CH1, 0);
	} else if (step == 3) {
		uintptr_t csibdg;
		uintptr_t wdma_com_1 = ctx->phys_regs[ISP_BLK_ID_WDMA_CORE1];
		uintptr_t wdma_com_3= ctx->phys_regs[ISP_BLK_ID_WDMA_CORE3];
		uintptr_t rdma_com = ctx->phys_regs[ISP_BLK_ID_RDMA_CORE1];
		u8 count = 10;

		if (err_raw_num == ISP_PRERAW0) {
			csibdg = ctx->phys_regs[ISP_BLK_ID_DMA_CTL_CSI0_BDG0];
		} else if (err_raw_num == ISP_PRERAW_B) {
			csibdg = ctx->phys_regs[ISP_BLK_ID_DMA_CTL_CSI0_BDG1];
		} else {
			csibdg = ctx->phys_regs[ISP_BLK_ID_DMA_CTL_CSI0_BDG2]; // REG_ISP_CSI_BDG_DVP_T
		}

		ISP_WR_BITS(csibdg, REG_ISP_CSI_BDG_T, CSI_BDG_TOP_CTRL, ABORT, 1);

		while (--count > 0) {
			if (ISP_RD_BITS(wdma_com_0, REG_WDMA_CORE_T, NORM_STATUS0, ABORT_DONE) &&
				ISP_RD_BITS(wdma_com_3, REG_WDMA_CORE_T, NORM_STATUS0, ABORT_DONE) &&
				ISP_RD_BITS(rdma_com, REG_RDMA_CORE_T, NORM_STATUS0, ABORT_DONE)) {
				vi_pr(VI_INFO, "W/RDMA_CORE abort done, count(%d)\n", count);
				break;
			}
			usleep_range(1, 5);
		}

		if (count == 0) {
			vi_pr(VI_ERR, "WDMA/RDMA_CORE abort fail\n");
			return -1;
		}
	} else if (step == 4) {
		sw_rst.raw = 0;
		sw_rst.bits.AXI_RST = 1;
		ISP_WR_REG(isptopb, REG_ISP_TOP_T, SW_RST, sw_rst.raw);
		sw_rst.raw = 0x37f;
		ISP_WR_REG(isptopb, REG_ISP_TOP_T, SW_RST, sw_rst.raw);

		vip_rst.raw = 0;
		vip_rst.b.isp_top = 1;
		vip_set_reset(vip_rst);

		vi_pr(VI_INFO, "ISP and vip_sys isp rst pull up\n");
	} else if (step == 5) {
		vip_rst = vip_get_reset();
		vip_rst.b.isp_top = 0;
		vip_set_reset(vip_rst);

		sw_rst.raw = ISP_RD_REG(isptopb, REG_ISP_TOP_T, SW_RST);
		sw_rst.bits.AXI_RST = 0;
		ISP_WR_REG(isptopb, REG_ISP_TOP_T, SW_RST, sw_rst.raw);
		ISP_WR_REG(isptopb, REG_ISP_TOP_T, SW_RST, 0);

		vi_pr(VI_INFO, "ISP and vip_sys isp rst pull down\n");
	} else if (step == 6) {
		uintptr_t fe0 = ctx->phys_regs[ISP_BLK_ID_PRE_RAW_FE0];
		uintptr_t fe1 = ctx->phys_regs[ISP_BLK_ID_PRE_RAW_FE1];
		uintptr_t isptopb = ctx->phys_regs[ISP_BLK_ID_ISPTOP];
		u8 cnt = 10;

		vi_pr(VI_INFO, "Wait ISP idle\n");

		while (--cnt > 0) {
			if ((ISP_RD_REG(fe0, REG_PRE_RAW_FE_T, FE_IDLE_INFO) == 0x3F) &&
				(ISP_RD_REG(fe1, REG_PRE_RAW_FE_T, FE_IDLE_INFO) == 0x3F) &&
				(ISP_RD_REG(isptopb, REG_ISP_TOP_T, BLK_IDLE)) == 0x3FF) {
				vi_pr(VI_INFO, "FE/BE/ISP idle done, count(%d)\n", cnt);
				break;
			}
			usleep_range(1, 5);
		}

		if (cnt == 0) {
			vi_pr(VI_ERR, "FE0(0x%x)/FE1(0x%x)/BE(0x%x)/ISP(0x%x) not idle.",
				ISP_RD_REG(fe0, REG_PRE_RAW_FE_T, FE_IDLE_INFO),
				ISP_RD_REG(fe1, REG_PRE_RAW_FE_T, FE_IDLE_INFO),
				ISP_RD_REG(isptopb, REG_ISP_TOP_T, BLK_IDLE));
			return -1;
		}
	}

	return 0;
}
#endif

/****************************************************************************
 *	YUV Bypass Control Flow Config
 ****************************************************************************/
u32 ispblk_dma_yuv_bypass_config(struct isp_ctx *ctx, uint32_t dmaid, uint64_t buf_addr,
					const enum cvi_isp_raw raw_num)
{
	uintptr_t dmab = ctx->phys_regs[dmaid];
	uint32_t len = 0, stride = 0, num = 0;

	switch (dmaid) {
	case ISP_BLK_ID_DMA_CTL_CSI0_BDG0:
	case ISP_BLK_ID_DMA_CTL_CSI0_BDG1:
	case ISP_BLK_ID_DMA_CTL_CSI0_BDG2:
	case ISP_BLK_ID_DMA_CTL_CSI0_BDG3:
	case ISP_BLK_ID_DMA_CTL_CSI1_BDG0:
	case ISP_BLK_ID_DMA_CTL_CSI1_BDG1:
	case ISP_BLK_ID_DMA_CTL_CSI2_BDG0:
	{
		/* csibdg */
		len = ctx->isp_pipe_cfg[raw_num].csibdg_width * 2;
		num = ctx->isp_pipe_cfg[raw_num].csibdg_height;
		stride = len;

		break;
	}
	case ISP_BLK_ID_DMA_CTL_YUV_RDMA_Y: //yuv_top crop
	{
		/* yuv_top */
		len = (((((ctx->isp_pipe_cfg[raw_num].crop.w) << 3) + 127) >> 7) << 7) >> 3;
		num = ctx->isp_pipe_cfg[raw_num].crop.h;
		stride = len;

		break;
	}
	case ISP_BLK_ID_DMA_CTL_YUV_RDMA_C: //yuv_top crop
	{
		len = (((((ctx->isp_pipe_cfg[raw_num].crop.w) << 3) + 127) >> 7) << 7) >> 3;
		num = ctx->isp_pipe_cfg[raw_num].crop.h >> 1;
		stride = len;

		break;
	}
	default:
		break;
	}

	len = VI_ALIGN(len);

	ISP_WR_REG(dmab, REG_ISP_DMA_CTL_T, DMA_SEGLEN, len);
	ISP_WR_REG(dmab, REG_ISP_DMA_CTL_T, DMA_STRIDE, stride);
	ISP_WR_REG(dmab, REG_ISP_DMA_CTL_T, DMA_SEGNUM, num);

	if (buf_addr)
		ispblk_dma_setaddr(ctx, dmaid, buf_addr);

	return len * num;
}

/****************************************************************************
 *	Slice buffer Control
 ****************************************************************************/
void vi_calculate_slice_buf_setting(struct isp_ctx *ctx, enum cvi_isp_raw raw_num)
{
	u32 main_le_num = 0, main_se_num = 0;
	u32 main_le_size = 0, main_se_size = 0;
	u16 main_le_w_th = 0, main_le_r_th = 0;
	u16 main_se_w_th = 0, main_se_r_th = 0;

	// u32 line_delay = slc_b_cfg.line_delay,;
	// u32 buffer = slc_b_cfg.buffer;

	u32 w = ctx->isp_pipe_cfg[raw_num].crop.w;

	// Calculate the ring buffer line number
	main_le_num = 41;//line_delay + buffer;
	main_le_size = VI_256_ALIGN(main_le_num * ((w * 3) / 2));

	// Calculate the r/w threshold
	main_le_r_th = 8;
	main_le_w_th = main_le_num - 1;

	if (ctx->isp_pipe_cfg[raw_num].is_hdr_on) {
		main_se_num = 32;
		main_se_size = VI_256_ALIGN(main_se_num * ((w * 3) / 2));
		main_se_r_th = 8;
		main_se_w_th = main_se_num - 1;
	}

	slc_b_cfg.main_path.le_buf_size = main_le_size;
	slc_b_cfg.main_path.le_w_thshd  = main_le_w_th;
	slc_b_cfg.main_path.le_r_thshd  = main_le_r_th;

	if (ctx->isp_pipe_cfg[raw_num].is_hdr_on) {
		slc_b_cfg.main_path.se_buf_size = main_se_size;
		slc_b_cfg.main_path.se_w_thshd  = main_se_w_th;
		slc_b_cfg.main_path.se_r_thshd  = main_se_r_th;
	}
}

void isp_slice_buf_trig(struct isp_ctx *ctx, enum cvi_isp_raw raw_num)
{
	uintptr_t isptopb = ctx->phys_regs[ISP_BLK_ID_ISPTOP];

	if (ctx->isp_pipe_cfg[raw_num].is_hdr_on)
		ISP_WR_REG(isptopb, REG_ISP_TOP_T, VI_SEL_FRAME_VALID, 0xF);
	else
		ISP_WR_REG(isptopb, REG_ISP_TOP_T, VI_SEL_FRAME_VALID, 0x3);
}

void _ispblk_dma_slice_config(struct isp_ctx *ctx, int dmaid, int en)
{
	uintptr_t dmab = ctx->phys_regs[dmaid];

	ISP_WR_BITS(dmab, REG_ISP_DMA_CTL_T, DMA_SLICESIZE, SLICE_SIZE, 1);
	ISP_WR_BITS(dmab, REG_ISP_DMA_CTL_T, SYS_CONTROL, SLICE_ENABLE, en);
}

void ispblk_slice_buf_config(struct isp_ctx *ctx, const enum cvi_isp_raw raw_num, u8 en)
{
	uintptr_t isptopb = ctx->phys_regs[ISP_BLK_ID_ISPTOP];
	uintptr_t wdma_com_1 = ctx->phys_regs[ISP_BLK_ID_WDMA_CORE1];
	uintptr_t rdma_com_1 = ctx->phys_regs[ISP_BLK_ID_RDMA_CORE1];
	union REG_WDMA_CORE_RING_BUFFER_EN      w_ring_buf_en_1;
	union REG_RDMA_CORE_RING_BUFFER_EN      r_ring_buf_en_1;

	if (raw_num != ISP_PRERAW0)
		return;

	// Multi-sensor don't support slice buffer mode.
	if (en) {
		// isptop config
		union REG_ISP_TOP_SCLIE_ENABLE			slice_en;
		union REG_ISP_TOP_W_SLICE_THRESH_MAIN		w_th_main;
		union REG_ISP_TOP_R_SLICE_THRESH_MAIN		r_th_main;

		slice_en.raw = 0;
		slice_en.bits.SLICE_ENABLE_MAIN_LEXP = en;

		w_th_main.raw = 0;
		w_th_main.bits.W_SLICE_THR_MAIN_LEXP = slc_b_cfg.main_path.le_w_thshd;

		r_th_main.raw = 0;
		r_th_main.bits.R_SLICE_THR_MAIN_LEXP = slc_b_cfg.main_path.le_r_thshd;

		if (ctx->isp_pipe_cfg[raw_num].is_hdr_on) {
			slice_en.bits.SLICE_ENABLE_MAIN_SEXP = en;

			w_th_main.bits.W_SLICE_THR_MAIN_SEXP = slc_b_cfg.main_path.se_w_thshd;
			r_th_main.bits.R_SLICE_THR_MAIN_SEXP = slc_b_cfg.main_path.se_r_thshd;
		}

		ISP_WR_REG(isptopb, REG_ISP_TOP_T, SCLIE_ENABLE, slice_en.raw);
		ISP_WR_REG(isptopb, REG_ISP_TOP_T, W_SLICE_THRESH_MAIN, w_th_main.raw);
		ISP_WR_REG(isptopb, REG_ISP_TOP_T, R_SLICE_THRESH_MAIN, r_th_main.raw);

		// wdma/rdma core config
		w_ring_buf_en_1.raw = ISP_RD_REG(wdma_com_1, REG_WDMA_CORE_T, RING_BUFFER_EN);
		w_ring_buf_en_1.raw |= (1 << ISP_DMA_ID_CSI0_BDG0);

		r_ring_buf_en_1.raw = ISP_RD_REG(rdma_com_1, REG_RDMA_CORE_T, RING_BUFFER_EN);
		r_ring_buf_en_1.raw |= (1 << ISP_DMA_ID_PRE_RAW_VI_SEL_LE);

		if (ctx->isp_pipe_cfg[raw_num].is_hdr_on) {
			w_ring_buf_en_1.raw |= (1 << ISP_DMA_ID_CSI0_BDG1);
			r_ring_buf_en_1.raw |= (1 << ISP_DMA_ID_PRE_RAW_VI_SEL_SE);
		}

		vi_pr(VI_DBG, "le_buf_size:0x%x, w_thshd:%d, r_thshd:%d\n",
		      slc_b_cfg.main_path.le_buf_size, slc_b_cfg.main_path.le_w_thshd,
		      slc_b_cfg.main_path.le_r_thshd);
		vi_pr(VI_DBG, "se_buf_size:0x%x, w_thshd:%d, r_thshd:%d\n",
		      slc_b_cfg.main_path.se_buf_size, slc_b_cfg.main_path.se_w_thshd,
		      slc_b_cfg.main_path.se_r_thshd);

		ISP_WR_REG(wdma_com_1, REG_WDMA_CORE_T, RING_BUFFER_EN, w_ring_buf_en_1.raw);
		ISP_WR_REG(wdma_com_1, REG_WDMA_CORE_T, RING_BUFFER_SIZE0, slc_b_cfg.main_path.le_buf_size);
		ISP_WR_REG(wdma_com_1, REG_WDMA_CORE_T, RING_BUFFER_SIZE4, slc_b_cfg.main_path.se_buf_size);
		ISP_WR_REG(rdma_com_1, REG_RDMA_CORE_T, RING_BUFFER_EN, r_ring_buf_en_1.raw);
		ISP_WR_REG(rdma_com_1, REG_RDMA_CORE_T, RING_BUFFER_SIZE0, slc_b_cfg.main_path.le_buf_size);
		ISP_WR_REG(rdma_com_1, REG_RDMA_CORE_T, RING_BUFFER_SIZE4, slc_b_cfg.main_path.se_buf_size);
	} else if (!en || _is_fe_post_offline(ctx)) {
		w_ring_buf_en_1.raw = ISP_RD_REG(wdma_com_1, REG_WDMA_CORE_T, RING_BUFFER_EN);
		w_ring_buf_en_1.raw &= ~(1 << ISP_DMA_ID_CSI0_BDG0);

		r_ring_buf_en_1.raw = ISP_RD_REG(rdma_com_1, REG_RDMA_CORE_T, RING_BUFFER_EN);
		r_ring_buf_en_1.raw &= ~(1 << ISP_DMA_ID_PRE_RAW_VI_SEL_LE);

		if (ctx->isp_pipe_cfg[raw_num].is_hdr_on) {
			w_ring_buf_en_1.raw &= ~(1 << ISP_DMA_ID_CSI0_BDG1);
			r_ring_buf_en_1.raw &= ~(1 << ISP_DMA_ID_PRE_RAW_VI_SEL_SE);
		}

		ISP_WR_REG(isptopb, REG_ISP_TOP_T, SCLIE_ENABLE, 0);
		ISP_WR_REG(wdma_com_1, REG_WDMA_CORE_T, RING_BUFFER_EN, w_ring_buf_en_1.raw);
		ISP_WR_REG(rdma_com_1, REG_RDMA_CORE_T, RING_BUFFER_EN, r_ring_buf_en_1.raw);
	}

	_ispblk_dma_slice_config(ctx, ISP_BLK_ID_DMA_CTL_CSI0_BDG0, en);
	_ispblk_dma_slice_config(ctx, ISP_BLK_ID_DMA_CTL_PRE_RAW_VI_SEL_LE, en);
	if (ctx->isp_pipe_cfg[raw_num].is_hdr_on) {
		_ispblk_dma_slice_config(ctx, ISP_BLK_ID_DMA_CTL_CSI0_BDG1, en);
		_ispblk_dma_slice_config(ctx, ISP_BLK_ID_DMA_CTL_PRE_RAW_VI_SEL_SE, en);
	}
}