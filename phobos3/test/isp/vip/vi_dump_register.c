#include <vip/vi_drv.h>

void dump_csi_bdg_register(struct isp_ctx *ctx, enum cvi_isp_raw raw_num)
{
	u32 val;
	int id = csibdg_find_hwid(raw_num);
	uintptr_t csi_bdg = ctx->phys_regs[id];

	printf("######################csi_bdg_%d#########################\n", raw_num);
	val = ISP_RD_REG(csi_bdg, REG_ISP_CSI_BDG_T, CSI_BDG_TOP_CTRL);
	printf("CSI_BDG_TOP_CTRL=[0x%0x]\n", val);
	val = ISP_RD_REG(csi_bdg, REG_ISP_CSI_BDG_T, CSI_BDG_INTERRUPT_CTRL);
	printf("CSI_BDG_INTERRUPT_CTRL=[0x%0x]\n", val);
	val = ISP_RD_REG(csi_bdg, REG_ISP_CSI_BDG_T, CSI_BDG_DMA_DPCM_MODE);
	printf("CSI_BDG_DMA_DPCM_MODE=[0x%0x]\n", val);

	val = ISP_RD_REG(csi_bdg, REG_ISP_CSI_BDG_T, CH0_SIZE);
	printf("CH0_SIZE=[0x%0x]\n", val);
	val = ISP_RD_REG(csi_bdg, REG_ISP_CSI_BDG_T, CH1_SIZE);
	printf("CH1_SIZE=[0x%0x]\n", val);

	val = ISP_RD_REG(csi_bdg, REG_ISP_CSI_BDG_T, CH0_CROP_EN);
	printf("CH0_CROP_EN=[0x%0x]\n", val);

	val = ISP_RD_REG(csi_bdg, REG_ISP_CSI_BDG_T, CH0_HORZ_CROP);
	printf("CH0_HORZ_CROP=[0x%0x]\n", val);
	val = ISP_RD_REG(csi_bdg, REG_ISP_CSI_BDG_T, CH0_VERT_CROP);
	printf("CH0_VERT_CROP=[0x%0x]\n", val);
	val = ISP_RD_REG(csi_bdg, REG_ISP_CSI_BDG_T, CH0_BLC_SUM);
	printf("CH0_BLC_SUM=[0x%0x]\n", val);

	val = ISP_RD_REG(csi_bdg, REG_ISP_CSI_BDG_T, CH1_CROP_EN);
	printf("CH1_CROP_EN=[0x%0x]\n", val);

	val = ISP_RD_REG(csi_bdg, REG_ISP_CSI_BDG_T, CH1_HORZ_CROP);
	printf("CH1_HORZ_CROP=[0x%0x]\n", val);
	val = ISP_RD_REG(csi_bdg, REG_ISP_CSI_BDG_T, CH1_VERT_CROP);
	printf("CH1_VERT_CROP=[0x%0x]\n", val);
	val = ISP_RD_REG(csi_bdg, REG_ISP_CSI_BDG_T, CH1_BLC_SUM);
	printf("CH1_BLC_SUM=[0x%0x]\n", val);

	val = ISP_RD_REG(csi_bdg, REG_ISP_CSI_BDG_T, CSI_PAT_GEN_CTRL);
	printf("CSI_PAT_GEN_CTRL=[0x%0x]\n", val);
	val = ISP_RD_REG(csi_bdg, REG_ISP_CSI_BDG_T, CSI_PAT_IDX_CTRL);
	printf("CSI_PAT_IDX_CTRL=[0x%0x]\n", val);
	val = ISP_RD_REG(csi_bdg, REG_ISP_CSI_BDG_T, CSI_PAT_COLOR_0);
	printf("CSI_PAT_COLOR_0=[0x%0x]\n", val);
	val = ISP_RD_REG(csi_bdg, REG_ISP_CSI_BDG_T, CSI_PAT_COLOR_1);
	printf("CSI_PAT_COLOR_1=[0x%0x]\n", val);
	val = ISP_RD_REG(csi_bdg, REG_ISP_CSI_BDG_T, CSI_BACKGROUND_COLOR_0);
	printf("CSI_BACKGROUND_COLOR_0=[0x%0x]\n", val);
	val = ISP_RD_REG(csi_bdg, REG_ISP_CSI_BDG_T, CSI_BACKGROUND_COLOR_1);
	printf("CSI_BACKGROUND_COLOR_1=[0x%0x]\n", val);

	val = ISP_RD_REG(csi_bdg, REG_ISP_CSI_BDG_T, CSI_FIX_COLOR_0);
	printf("CSI_FIX_COLOR_0=[0x%0x]\n", val);
	val = ISP_RD_REG(csi_bdg, REG_ISP_CSI_BDG_T, CSI_FIX_COLOR_1);
	printf("CSI_FIX_COLOR_1=[0x%0x]\n", val);

	val = ISP_RD_REG(csi_bdg, REG_ISP_CSI_BDG_T, CSI_MDE_V_SIZE);
	printf("CSI_MDE_V_SIZE=[0x%0x]\n", val);
	val = ISP_RD_REG(csi_bdg, REG_ISP_CSI_BDG_T, CSI_MDE_H_SIZE);
	printf("CSI_MDE_H_SIZE=[0x%0x]\n", val);

	val = ISP_RD_REG(csi_bdg, REG_ISP_CSI_BDG_T, CSI_FDE_V_SIZE);
	printf("CSI_FDE_V_SIZE=[0x%0x]\n", val);
	val = ISP_RD_REG(csi_bdg, REG_ISP_CSI_BDG_T, CSI_FDE_H_SIZE);
	printf("CSI_FDE_H_SIZE=[0x%0x]\n", val);

	val = ISP_RD_REG(csi_bdg, REG_ISP_CSI_BDG_T, CSI_HSYNC_CTRL);
	printf("CSI_HSYNC_CTRL=[0x%0x]\n", val);
	val = ISP_RD_REG(csi_bdg, REG_ISP_CSI_BDG_T, CSI_VSYNC_CTRL);
	printf("CSI_VSYNC_CTRL=[0x%0x]\n", val);

	val = ISP_RD_REG(csi_bdg, REG_ISP_CSI_BDG_T, CSI_TGEN_TT_SIZE);
	printf("CSI_TGEN_TT_SIZE=[0x%0x]\n", val);
}

void dump_fe_register(struct isp_ctx *ctx, enum cvi_isp_raw raw_num)
{
	u32 val;
	int id = fe_find_hwid(raw_num);
	uintptr_t preraw_fe = ctx->phys_regs[id];

	printf("######################preraw_fe_%d#########################\n", raw_num);
	val = ISP_RD_REG(preraw_fe, REG_PRE_RAW_FE_T, PRE_RAW_CTRL);
	printf("PRE_RAW_CTRL=[0x%0x]\n", val);
	val = ISP_RD_REG(preraw_fe, REG_PRE_RAW_FE_T, PRE_RAW_FRAME_SIZE);
	printf("PRE_RAW_FRAME_SIZE=[0x%0x]\n", val);

	val = ISP_RD_REG(preraw_fe, REG_PRE_RAW_FE_T, PRE_RAW_FRAME_VLD);
	printf("PRE_RAW_FRAME_VLD=[0x%0x]\n", val);
	val = ISP_RD_REG(preraw_fe, REG_PRE_RAW_FE_T, PRE_RAW_DEBUG_STATE);
	printf("PRE_RAW_DEBUG_STATE=[0x%0x]\n", val);
	val = ISP_RD_REG(preraw_fe, REG_PRE_RAW_FE_T, PRE_RAW_DUMMY);
	printf("PRE_RAW_DUMMY=[0x%0x]\n", val);
	val = ISP_RD_REG(preraw_fe, REG_PRE_RAW_FE_T, PRE_RAW_INFO);
	printf("PRE_RAW_INFO=[0x%0x]\n", val);
	val = ISP_RD_REG(preraw_fe, REG_PRE_RAW_FE_T, FE_IDLE_INFO);
	printf("FE_IDLE_INFO=[0x%0x]\n", val);
}

void dump_isp_top_register(struct isp_ctx *ctx)
{
	u32 val;
	uintptr_t isptopb = ctx->phys_regs[ISP_BLK_ID_ISPTOP];

	printf("######################isp_top#########################\n");
	val = ISP_RD_REG(isptopb, REG_ISP_TOP_T, INT_EVENT0_EN);
	printf("INT_EVENT0_EN=[0x%0x]\n", val);
	val = ISP_RD_REG(isptopb, REG_ISP_TOP_T, INT_EVENT1_EN);
	printf("INT_EVENT1_EN=[0x%0x]\n", val);
	val = ISP_RD_REG(isptopb, REG_ISP_TOP_T, INT_EVENT2_EN);
	printf("INT_EVENT2_EN=[0x%0x]\n", val);

	val = ISP_RD_REG(isptopb, REG_ISP_TOP_T, SW_CTRL_0);
	printf("SW_CTRL_0=[0x%0x]\n", val);
	val = ISP_RD_REG(isptopb, REG_ISP_TOP_T, SW_CTRL_1);
	printf("SW_CTRL_1=[0x%0x]\n", val);

	val = ISP_RD_REG(isptopb, REG_ISP_TOP_T, CTRL_MODE_SEL0);
	printf("CTRL_MODE_SEL0=[0x%0x]\n", val);
	val = ISP_RD_REG(isptopb, REG_ISP_TOP_T, CTRL_MODE_SEL1);
	printf("CTRL_MODE_SEL1=[0x%0x]\n", val);

	val = ISP_RD_REG(isptopb, REG_ISP_TOP_T, SCENARIOS_CTRL);
	printf("SCENARIOS_CTRL=[0x%0x]\n", val);
	val = ISP_RD_REG(isptopb, REG_ISP_TOP_T, BLK_IDLE);
	printf("BLK_IDLE=[0x%0x]\n", val);
	val = ISP_RD_REG(isptopb, REG_ISP_TOP_T, BLK_IDLE_ENABLE);
	printf("BLK_IDLE_ENABLE=[0x%0x]\n", val);

	val = ISP_RD_REG(isptopb, REG_ISP_TOP_T, IP_ENABLE1);
	printf("IP_ENABLE1=[0x%0x]\n", val);
	val = ISP_RD_REG(isptopb, REG_ISP_TOP_T, IP_ENABLE2);
	printf("IP_ENABLE2=[0x%0x]\n", val);

	val = ISP_RD_REG(isptopb, REG_ISP_TOP_T, SCLIE_ENABLE);
	printf("SCLIE_ENABLE=[0x%0x]\n", val);

	val = ISP_RD_REG(isptopb, REG_ISP_TOP_T, BLK_IDLE_1);
	printf("BLK_IDLE_1=[0x%0x]\n", val);	
}

void dump_fe_dma_register(struct isp_ctx *ctx, enum cvi_isp_raw raw_num)
{
	u32 val;
	uintptr_t csi_bdg_dma_le;
	uintptr_t csi_bdg_dma_se;

	if (raw_num == ISP_PRERAW0) {
		csi_bdg_dma_le = ctx->phys_regs[ISP_BLK_ID_DMA_CTL_CSI0_BDG0];
		csi_bdg_dma_se = ctx->phys_regs[ISP_BLK_ID_DMA_CTL_CSI0_BDG1];
	} else {
		csi_bdg_dma_le = ctx->phys_regs[ISP_BLK_ID_DMA_CTL_CSI1_BDG0];
		csi_bdg_dma_se = ctx->phys_regs[ISP_BLK_ID_DMA_CTL_CSI1_BDG1];
	}

	uintptr_t pre_raw_vi_sel_le = ctx->phys_regs[ISP_BLK_ID_DMA_CTL_PRE_RAW_VI_SEL_LE];
	uintptr_t pre_raw_vi_sel_se = ctx->phys_regs[ISP_BLK_ID_DMA_CTL_PRE_RAW_VI_SEL_SE];

	printf("######################csi_bdg_dma_le_%d#######################\n", raw_num);
	val = ISP_RD_REG(csi_bdg_dma_le, REG_ISP_DMA_CTL_T, SYS_CONTROL);
	printf("SYS_CONTROL=[0x%0x]\n", val);
	val = ISP_RD_REG(csi_bdg_dma_le, REG_ISP_DMA_CTL_T, BASE_ADDR);
	printf("BASE_ADDR=[0x%0x]\n", val);
	val = ISP_RD_REG(csi_bdg_dma_le, REG_ISP_DMA_CTL_T, DMA_SEGLEN);
	printf("DMA_SEGLEN=[0x%0x]\n", val);
	val = ISP_RD_REG(csi_bdg_dma_le, REG_ISP_DMA_CTL_T, DMA_STRIDE);
	printf("DMA_STRIDE=[0x%0x]\n", val);
	val = ISP_RD_REG(csi_bdg_dma_le, REG_ISP_DMA_CTL_T, DMA_SEGNUM);
	printf("DMA_SEGNUM=[0x%0x]\n", val);
	printf("######################csi_bdg_dma_se_%d#######################\n", raw_num);
	val = ISP_RD_REG(csi_bdg_dma_se, REG_ISP_DMA_CTL_T, SYS_CONTROL);
	printf("SYS_CONTROL=[0x%0x]\n", val);
	val = ISP_RD_REG(csi_bdg_dma_se, REG_ISP_DMA_CTL_T, BASE_ADDR);
	printf("BASE_ADDR=[0x%0x]\n", val);
	val = ISP_RD_REG(csi_bdg_dma_se, REG_ISP_DMA_CTL_T, DMA_SEGLEN);
	printf("DMA_SEGLEN=[0x%0x]\n", val);
	val = ISP_RD_REG(csi_bdg_dma_se, REG_ISP_DMA_CTL_T, DMA_STRIDE);
	printf("DMA_STRIDE=[0x%0x]\n", val);
	val = ISP_RD_REG(csi_bdg_dma_se, REG_ISP_DMA_CTL_T, DMA_SEGNUM);
	printf("DMA_SEGNUM=[0x%0x]\n", val);

	printf("######################pre_raw_vi_sel_le#######################\n");
	val = ISP_RD_REG(pre_raw_vi_sel_le, REG_ISP_DMA_CTL_T, SYS_CONTROL);
	printf("SYS_CONTROL=[0x%0x]\n", val);
	val = ISP_RD_REG(pre_raw_vi_sel_le, REG_ISP_DMA_CTL_T, BASE_ADDR);
	printf("BASE_ADDR=[0x%0x]\n", val);
	val = ISP_RD_REG(pre_raw_vi_sel_le, REG_ISP_DMA_CTL_T, DMA_SEGLEN);
	printf("DMA_SEGLEN=[0x%0x]\n", val);
	val = ISP_RD_REG(pre_raw_vi_sel_le, REG_ISP_DMA_CTL_T, DMA_STRIDE);
	printf("DMA_STRIDE=[0x%0x]\n", val);
	val = ISP_RD_REG(pre_raw_vi_sel_le, REG_ISP_DMA_CTL_T, DMA_SEGNUM);
	printf("DMA_SEGNUM=[0x%0x]\n", val);

	printf("######################pre_raw_vi_sel_se#######################\n");
	val = ISP_RD_REG(pre_raw_vi_sel_se, REG_ISP_DMA_CTL_T, SYS_CONTROL);
	printf("SYS_CONTROL=[0x%0x]\n", val);
	val = ISP_RD_REG(pre_raw_vi_sel_se, REG_ISP_DMA_CTL_T, BASE_ADDR);
	printf("BASE_ADDR=[0x%0x]\n", val);
	val = ISP_RD_REG(pre_raw_vi_sel_se, REG_ISP_DMA_CTL_T, DMA_SEGLEN);
	printf("DMA_SEGLEN=[0x%0x]\n", val);
	val = ISP_RD_REG(pre_raw_vi_sel_se, REG_ISP_DMA_CTL_T, DMA_STRIDE);
	printf("DMA_STRIDE=[0x%0x]\n", val);
	val = ISP_RD_REG(pre_raw_vi_sel_se, REG_ISP_DMA_CTL_T, DMA_SEGNUM);
	printf("DMA_SEGNUM=[0x%0x]\n", val);
}

void dump_pass_register(struct isp_ctx *ctx, enum cvi_isp_raw raw_num)
{
	dump_csi_bdg_register(ctx, raw_num);
	dump_fe_register(ctx, raw_num);
	dump_isp_top_register(ctx);
	dump_fe_dma_register(ctx, raw_num);
}