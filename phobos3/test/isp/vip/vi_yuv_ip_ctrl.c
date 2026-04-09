#include <vip/vi_drv.h>

/****************************************************************************
 * Global parameters
 ****************************************************************************/

/****************************************************************************
 * FBC_CONFIG
 ****************************************************************************/
#define TNR_Y_W		6
#define TNR_C_W		7
#define TNR_Y_R		10
#define TNR_C_R		11

#define DEFAULT_K	2
#define CPLX_SHIFT	3
#define PEN_POS_SHIFT	4
#define TARGET_CR	43//, 55, 68, 80, 93, 100

struct vi_fbc_cfg fbc_cfg = {
	.cu_size	= 8,
	.target_cr	= TARGET_CR,
	.is_lossless	= 0,
	.y_bs_size	= 0,
	.c_bs_size	= 0,
	.y_buf_size	= 0,
	.c_buf_size	= 0,
};

/***************************************************************************
 * CA global setting
 ***************************************************************************/
u8 ca_y_lut[] = {
64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
};

u8 cp_y_lut[] = {
0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,
16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31,
32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47,
48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63,
64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79,
80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95,
96, 97, 98, 99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111,
112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127,
128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143,
144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 159,
160, 161, 162, 163, 164, 165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175,
176, 177, 178, 179, 180, 181, 182, 183, 184, 185, 186, 187, 188, 189, 190, 191,
192, 193, 194, 195, 196, 197, 198, 199, 200, 201, 202, 203, 204, 205, 206, 207,
208, 209, 210, 211, 212, 213, 214, 215, 216, 217, 218, 219, 220, 221, 222, 223,
224, 225, 226, 227, 228, 229, 230, 231, 232, 233, 234, 235, 236, 237, 238, 239,
240, 241, 242, 243, 244, 245, 246, 247, 248, 249, 250, 251, 252, 253, 254, 255,
};

u8 cp_u_lut[] = {
51, 51, 51, 51, 51, 51, 51, 51, 51, 51, 51, 51, 51, 51, 51, 51,
51, 51, 51, 51, 51, 51, 51, 51, 51, 51, 51, 51, 51, 51, 51, 51,
51, 51, 51, 51, 51, 51, 51, 51, 51, 51, 51, 51, 51, 51, 51, 51,
51, 51, 51, 51, 51, 51, 51, 51, 51, 51, 51, 51, 51, 51, 51, 51,
51, 51, 51, 51, 51, 51, 51, 51, 51, 51, 51, 51, 51, 51, 51, 51,
51, 51, 51, 51, 51, 51, 51, 51, 51, 51, 51, 51, 51, 51, 51, 51,
51, 51, 51, 51, 51, 51, 51, 51, 51, 51, 51, 51, 51, 51, 51, 51,
51, 51, 51, 51, 51, 51, 51, 51, 51, 51, 51, 51, 51, 51, 51, 51,
51, 51, 51, 51, 51, 51, 51, 51, 51, 51, 51, 51, 51, 51, 51, 51,
51, 51, 51, 51, 51, 51, 51, 51, 51, 51, 51, 51, 51, 51, 51, 51,
51, 51, 51, 51, 51, 51, 51, 51, 51, 51, 51, 51, 51, 51, 51, 51,
51, 51, 51, 51, 51, 51, 51, 51, 51, 51, 51, 51, 51, 51, 51, 51,
51, 51, 51, 51, 51, 51, 51, 51, 51, 51, 51, 51, 51, 51, 51, 51,
51, 51, 51, 51, 51, 51, 51, 51, 51, 51, 51, 51, 51, 51, 51, 51,
51, 51, 51, 51, 51, 51, 51, 51, 51, 51, 51, 51, 51, 51, 51, 51,
51, 51, 51, 51, 51, 51, 51, 51, 51, 51, 51, 51, 51, 51, 51, 51,
};

u8 cp_v_lut[] = {
153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153,
153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153,
153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153,
153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153,
153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153,
153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153,
153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153,
153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153,
153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153,
153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153,
153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153,
153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153,
153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153,
153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153,
153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153,
153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153, 153,
};

/*******************************************************************************
 *	YUV IPs config
 ******************************************************************************/

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
	uintptr_t dither = ctx->phys_regs[ISP_BLK_ID_YUV_DITHER];

	if (sel == 0) {
		union REG_ISP_YUV_DITHER_Y_DITHER reg;

		reg.raw = 0;
		reg.bits.Y_DITHER_ENABLE = en;
		reg.bits.Y_DITHER_MOD_ENABLE = mod_en;
		reg.bits.Y_DITHER_HISTIDX_ENABLE = histidx_en;
		reg.bits.Y_DITHER_FMNUM_ENABLE = fmnum_en;
		reg.bits.Y_DITHER_SHDW_SEL = 1;
		reg.bits.Y_DITHER_WIDTHM1 = ctx->img_width - 1;
		reg.bits.Y_DITHER_HEIGHTM1 = ctx->img_height - 1;

		ISP_WR_REG(dither, REG_ISP_YUV_DITHER_T, Y_DITHER, reg.raw);
	} else if (sel == 1) {
		union REG_ISP_YUV_DITHER_UV_DITHER reg;

		reg.raw = 0;
		reg.bits.UV_DITHER_ENABLE = en;
		reg.bits.UV_DITHER_MOD_ENABLE = mod_en;
		reg.bits.UV_DITHER_HISTIDX_ENABLE = histidx_en;
		reg.bits.UV_DITHER_FMNUM_ENABLE = fmnum_en;
		reg.bits.UV_DITHER_WIDTHM1 = (ctx->img_width >> 1) - 1;
		reg.bits.UV_DITHER_HEIGHTM1 = (ctx->img_height >> 1) - 1;

		ISP_WR_REG(dither, REG_ISP_YUV_DITHER_T, UV_DITHER, reg.raw);
	}

	return 0;
}

void ispblk_pre_ee_front_config(struct isp_ctx *ctx, bool en)
{
	uintptr_t pre_ee = ctx->phys_regs[ISP_BLK_ID_PRE_EE_FRONT];
	union REG_ISP_EE_ADD_REG0  reg_0;

	reg_0.raw = ISP_RD_REG(pre_ee, REG_ISP_EE_ADD_T, REG0);
	reg_0.bits.EE_ENABLE = en;
	ISP_WR_REG(pre_ee, REG_ISP_EE_ADD_T, REG0, reg_0.raw);
}

void ispblk_pre_ee_back_config(struct isp_ctx *ctx, bool en)
{
	uintptr_t pre_ee = ctx->phys_regs[ISP_BLK_ID_PRE_EE_BACK];
	union REG_ISP_EE_ADD_BACK_EE_ADD_B_REG0  reg_0;

	reg_0.raw = ISP_RD_REG(pre_ee, REG_ISP_EE_ADD_BACK_T, EE_ADD_B_REG0);
	reg_0.bits.EE_ENABLE = en;
	ISP_WR_REG(pre_ee, REG_ISP_EE_ADD_BACK_T, EE_ADD_B_REG0, reg_0.raw);
}

void ispblk_ldci_config(struct isp_ctx *ctx, bool en, uint8_t test_case)
{
	uintptr_t ldci = ctx->phys_regs[ISP_BLK_ID_LDCI];
	union REG_ISP_LDCI_CTL                  ctl;
	union REG_ISP_LDCI_LUMA_PROT_LUT0       prot_lut0;
	union REG_ISP_LDCI_LUMA_PROT_LUT1       prot_lut1;
	union REG_ISP_LDCI_LUMA_PROT_LUT4       prot_lut4;

	ctl.raw = ISP_RD_REG(ldci, REG_ISP_LDCI_T, LDCI_CTL);
	ctl.bits.LDCI_ENABLE = en;
	ctl.bits.DCI_ENABLE = en;
	ctl.bits.LDCI_LUMA_PROT_EN = en;
	ISP_WR_REG(ldci, REG_ISP_LDCI_T, LDCI_CTL, ctl.raw );

	{//luma_prot
		prot_lut0.raw = ISP_RD_REG(ldci, REG_ISP_LDCI_T, LDCI_LUMA_PROT_LUT0);
		prot_lut0.bits.LDCI_LUMA_PROT_LUT_0 = 63;
		prot_lut0.bits.LDCI_LUMA_PROT_LUT_1 = 58;
		prot_lut0.bits.LDCI_LUMA_PROT_LUT_2 = 43;
		prot_lut0.bits.LDCI_LUMA_PROT_LUT_3 = 27;
		ISP_WR_REG(ldci, REG_ISP_LDCI_T, LDCI_LUMA_PROT_LUT0, prot_lut0.raw);

		prot_lut1.raw = ISP_RD_REG(ldci, REG_ISP_LDCI_T, LDCI_LUMA_PROT_LUT1);
		prot_lut1.bits.LDCI_LUMA_PROT_LUT_4 = 13;
		prot_lut1.bits.LDCI_LUMA_PROT_LUT_5 = 6;
		prot_lut1.bits.LDCI_LUMA_PROT_LUT_6 = 2;
		prot_lut1.bits.LDCI_LUMA_PROT_LUT_7 = 1;
		ISP_WR_REG(ldci, REG_ISP_LDCI_T, LDCI_LUMA_PROT_LUT1, prot_lut1.raw);

		ISP_WR_REG(ldci, REG_ISP_LDCI_T, LDCI_LUMA_PROT_LUT2, 0);
		ISP_WR_REG(ldci, REG_ISP_LDCI_T, LDCI_LUMA_PROT_LUT3, 0);

		prot_lut4.raw = ISP_RD_REG(ldci, REG_ISP_LDCI_T, LDCI_LUMA_PROT_LUT4);
		prot_lut4.bits.LDCI_LUMA_PROT_CW = 1;
		prot_lut4.bits.LDCI_LUMA_PROT_BW = 8;
		ISP_WR_REG(ldci, REG_ISP_LDCI_T, LDCI_LUMA_PROT_LUT4, prot_lut4.raw);
	}
	{//luma_gain
		ISP_WR_REG(ldci, REG_ISP_LDCI_T, LDCI_LUMA_GAIN_LUT0, 0x40404040);
		ISP_WR_REG(ldci, REG_ISP_LDCI_T, LDCI_LUMA_GAIN_LUT1, 0x40404040);
		ISP_WR_REG(ldci, REG_ISP_LDCI_T, LDCI_LUMA_GAIN_LUT2, 0x40);
		ISP_WR_REG(ldci, REG_ISP_LDCI_T, LDCI_THR, 6);

		ISP_WR_REG(ldci, REG_ISP_LDCI_T, LDCI_DIFF_GAIN_LUT_L0, 0x80808080);
		ISP_WR_REG(ldci, REG_ISP_LDCI_T, LDCI_DIFF_GAIN_LUT_L1, 0x80);
		ISP_WR_REG(ldci, REG_ISP_LDCI_T, LDCI_DIFF_GAIN_LUT_R0, 0x80808080);
		ISP_WR_REG(ldci, REG_ISP_LDCI_T, LDCI_DIFF_GAIN_LUT_R1, 0x80);
	}
}

void ispblk_tnr_config(struct isp_ctx *ctx, bool en)
{
	uintptr_t tnr = ctx->phys_regs[ISP_BLK_ID_TNR];
	union REG_ISP_444_422_0 reg_0;

	reg_0.raw = ISP_RD_REG(tnr, REG_ISP_444_422_T, REG_0);
	reg_0.bits.TDNR_ENABLE 	= en;
	reg_0.bits.DMA_ENABLE	= en;
	ISP_WR_REG(tnr, REG_ISP_444_422_T, REG_0, reg_0.raw);
}

void ispblk_fbc_clear_fbcd_ring_base(struct isp_ctx *ctx, u8 raw_num)
{
	uintptr_t rdma_com_3 = ctx->phys_regs[ISP_BLK_ID_RDMA_CORE3];

	ISP_WR_REG(rdma_com_3, REG_RDMA_CORE_T, UP_RING_BASE, ((1 << ISP_DMA_ID_TNR_LD_Y) | (1 << ISP_DMA_ID_TNR_LD_C)));
}

void ispblk_fbc_chg_to_sw_mode(struct isp_ctx *ctx, u8 raw_num)
{
	uintptr_t y_rdma = ctx->phys_regs[ISP_BLK_ID_DMA_CTL_TNR_ST_Y];
	uintptr_t uv_rdma = ctx->phys_regs[ISP_BLK_ID_DMA_CTL_TNR_ST_C];
	union REG_ISP_DMA_CTL_SYS_CONTROL sys_ctrl;

	// Y/UV rdma config seglen to HW mode, other SW mode
	sys_ctrl.raw = ISP_RD_REG(y_rdma, REG_ISP_DMA_CTL_T, SYS_CONTROL);
	sys_ctrl.bits.BASE_SEL		= 0x1;
	sys_ctrl.bits.STRIDE_SEL	= true;
	sys_ctrl.bits.SEGLEN_SEL	= false;
	sys_ctrl.bits.SEGNUM_SEL	= true;
	ISP_WR_REG(y_rdma, REG_ISP_DMA_CTL_T, SYS_CONTROL, sys_ctrl.raw);

	sys_ctrl.raw = ISP_RD_REG(uv_rdma, REG_ISP_DMA_CTL_T, SYS_CONTROL);
	sys_ctrl.bits.BASE_SEL		= 0x1;
	sys_ctrl.bits.STRIDE_SEL	= true;
	sys_ctrl.bits.SEGLEN_SEL	= false;
	sys_ctrl.bits.SEGNUM_SEL	= true;
	ISP_WR_REG(uv_rdma, REG_ISP_DMA_CTL_T, SYS_CONTROL, sys_ctrl.raw);
}

void vi_fbc_calculate_size(struct isp_ctx *ctx, u8 raw_num)
{
	u32 img_w = ctx->isp_pipe_cfg[raw_num].crop.w;
	u32 img_h = ctx->isp_pipe_cfg[raw_num].crop.h;

	u32 max_cu_bit = fbc_cfg.is_lossless ? 65 : 67; // = CU_SIZE * 8 + cu_md_bit
	u32 line_cu_num = (img_w + fbc_cfg.cu_size - 1) / fbc_cfg.cu_size;
	u32 total_line_bit_budget = fbc_cfg.is_lossless ?
				(max_cu_bit * line_cu_num) : ((img_w * 8 * fbc_cfg.target_cr) / 100);
	u32 total_first_line_bit_budget = fbc_cfg.is_lossless ? total_line_bit_budget : (img_w * 8);

	u32 y_bs_size = VI_ALIGN(((total_line_bit_budget * (img_h - 1) + total_first_line_bit_budget) / 512) * 64);
	u32 uv_bs_size = VI_ALIGN((((total_line_bit_budget * (img_h / 2) - 1)
				+ total_first_line_bit_budget) / 512) * 64);
	u32 y_buf_size = VI_4K_ALIGN(y_bs_size);
	u32 uv_buf_size = VI_4K_ALIGN(uv_bs_size);

	fbc_cfg.y_bs_size = y_bs_size;
	fbc_cfg.c_bs_size = uv_bs_size;
	fbc_cfg.y_buf_size = y_buf_size;
	fbc_cfg.c_buf_size = uv_buf_size;
}

void ispblk_fbc_ring_buf_config(struct isp_ctx *ctx, u8 en)
{
	//WDMA_CORE, RDMA_CORE ring buffer ctrl
	uintptr_t wdma_com_3 = ctx->phys_regs[ISP_BLK_ID_WDMA_CORE3];
	uintptr_t rdma_com_3 = ctx->phys_regs[ISP_BLK_ID_RDMA_CORE3];
	union REG_WDMA_CORE_DISABLE_SEGLEN	disable_seglen;
	union REG_WDMA_CORE_RING_BUFFER_EN	ring_buf_en;

	vi_fbc_calculate_size(ctx, ISP_PRERAW0);

	if (en) {
		disable_seglen.raw = ISP_RD_REG(wdma_com_3, REG_WDMA_CORE_T, DISABLE_SEGLEN);
		disable_seglen.bits.SEGLEN_DISABLE |= ((1 << ISP_DMA_ID_TNR_ST_Y) | (1 << ISP_DMA_ID_TNR_ST_C));
		ISP_WR_REG(wdma_com_3, REG_WDMA_CORE_T, DISABLE_SEGLEN, disable_seglen.raw);

		ring_buf_en.raw = ISP_RD_REG(wdma_com_3, REG_WDMA_CORE_T, RING_BUFFER_EN);
		ring_buf_en.bits.RING_ENABLE |= ((1 << ISP_DMA_ID_TNR_ST_Y) | (1 << ISP_DMA_ID_TNR_ST_C));
		ISP_WR_REG(wdma_com_3, REG_WDMA_CORE_T, RING_BUFFER_EN, ring_buf_en.raw);

		ring_buf_en.raw = ISP_RD_REG(rdma_com_3, REG_RDMA_CORE_T, RING_BUFFER_EN);
		ring_buf_en.bits.RING_ENABLE |= ((1 << ISP_DMA_ID_TNR_LD_Y) | (1 << ISP_DMA_ID_TNR_LD_C));
		ISP_WR_REG(rdma_com_3, REG_RDMA_CORE_T, RING_BUFFER_EN, ring_buf_en.raw);

		//WDMA ctrl cfg
		ISP_WR_REG(wdma_com_3, REG_WDMA_CORE_T, RING_BUFFER_SIZE6, fbc_cfg.y_buf_size);
		ISP_WR_REG(wdma_com_3, REG_WDMA_CORE_T, RING_BUFFER_SIZE7, fbc_cfg.c_buf_size);
		ISP_WR_REG(wdma_com_3, REG_WDMA_CORE_T, UP_RING_BASE, ((1 << ISP_DMA_ID_TNR_ST_Y) | (1 << ISP_DMA_ID_TNR_ST_C)));

		//RDMA ctrl cfg
		ISP_WR_REG(rdma_com_3, REG_RDMA_CORE_T, RING_BUFFER_SIZE5, fbc_cfg.y_buf_size);
		ISP_WR_REG(rdma_com_3, REG_RDMA_CORE_T, RING_BUFFER_SIZE6, fbc_cfg.c_buf_size);
		ISP_WR_REG(rdma_com_3, REG_RDMA_CORE_T, UP_RING_BASE, ((1 << ISP_DMA_ID_TNR_LD_Y) | (1 << ISP_DMA_ID_TNR_LD_C)));
	} else {

		disable_seglen.raw = ISP_RD_REG(wdma_com_3, REG_WDMA_CORE_T, DISABLE_SEGLEN);
		disable_seglen.bits.SEGLEN_DISABLE &= ~((1 << ISP_DMA_ID_TNR_ST_Y) | (1 << ISP_DMA_ID_TNR_ST_C));
		ISP_WR_REG(wdma_com_3, REG_WDMA_CORE_T, DISABLE_SEGLEN, disable_seglen.raw);

		ring_buf_en.raw = ISP_RD_REG(wdma_com_3, REG_WDMA_CORE_T, RING_BUFFER_EN);
		ring_buf_en.bits.RING_ENABLE &= ~((1 << ISP_DMA_ID_TNR_ST_Y) | (1 << ISP_DMA_ID_TNR_ST_C));
		ISP_WR_REG(wdma_com_3, REG_WDMA_CORE_T, RING_BUFFER_EN, ring_buf_en.raw);

		ring_buf_en.raw = ISP_RD_REG(rdma_com_3, REG_RDMA_CORE_T, RING_BUFFER_EN);
		ring_buf_en.bits.RING_ENABLE &= ~((1 << ISP_DMA_ID_TNR_LD_Y) | (1 << ISP_DMA_ID_TNR_LD_C));
		ISP_WR_REG(rdma_com_3, REG_RDMA_CORE_T, RING_BUFFER_EN, ring_buf_en.raw);
	}
}

void ispblk_fbcd_config(struct isp_ctx *ctx, bool en)
{
	uintptr_t fbcd = ctx->phys_regs[ISP_BLK_ID_FBCD];
	uintptr_t fbce = ctx->phys_regs[ISP_BLK_ID_FBCE];
	union REG_FBCD_24	d_reg_24;
	union REG_FBCD_28	d_reg_28;
	union REG_FBCE_10	reg_10;
	union REG_FBCE_20	reg_20;

	if (en) {
		reg_10.raw = ISP_RD_REG(fbce, REG_FBCE_T, REG_10);
		reg_20.raw = ISP_RD_REG(fbce, REG_FBCE_T, REG_20);

		d_reg_24.raw = ISP_RD_REG(fbcd, REG_FBCD_T, REG_24);
		d_reg_24.bits.Y_LOSSLESS		= reg_10.bits.Y_LOSSLESS;
		d_reg_24.bits.Y_BASE_QDPCM_Q		= reg_10.bits.Y_BASE_QDPCM_Q;
		d_reg_24.bits.Y_BASE_PCM_BD_MINUS2	= reg_10.bits.Y_BASE_PCM_BD_MINUS2;
		d_reg_24.bits.Y_DEFAULT_GR_K		= DEFAULT_K;
		ISP_WR_REG(fbcd, REG_FBCD_T, REG_24, d_reg_24.raw);

		d_reg_28.raw = ISP_RD_REG(fbcd, REG_FBCD_T, REG_28);
		d_reg_28.bits.C_LOSSLESS		= reg_20.bits.C_LOSSLESS;
		d_reg_28.bits.C_BASE_QDPCM_Q		= reg_20.bits.C_BASE_QDPCM_Q;
		d_reg_28.bits.C_BASE_PCM_BD_MINUS2	= reg_20.bits.C_BASE_PCM_BD_MINUS2;
		d_reg_28.bits.C_DEFAULT_GR_K		= DEFAULT_K;
		ISP_WR_REG(fbcd, REG_FBCD_T, REG_28, d_reg_28.raw);
	}

	ISP_WR_BITS(fbcd, REG_FBCD_T, REG_00, FBCD_EN, en);
}

void ispblk_fbce_config(struct isp_ctx *ctx, bool en)
{
	uintptr_t fbce = ctx->phys_regs[ISP_BLK_ID_FBCE];
	union REG_FBCE_10	reg_10;
	union REG_FBCE_14	reg_14;
	union REG_FBCE_1C	reg_1C;
	union REG_FBCE_20	reg_20;
	union REG_FBCE_24	reg_24;
	union REG_FBCE_2C	reg_2C;
	union REG_FBCE_30	reg_30;
	union REG_FBCE_34	reg_34;

	u32 img_w = ctx->isp_pipe_cfg[ISP_PRERAW0].crop.w;

	u32 cu_md_bit = fbc_cfg.is_lossless ? 1 : 3;
	u32 max_cu_bit = fbc_cfg.is_lossless ? 65 : 67; // = CU_SIZE * 8 + cu_md_bit
	u32 line_cu_num = (img_w + fbc_cfg.cu_size - 1) / fbc_cfg.cu_size;
	u32 total_line_bit_budget = fbc_cfg.is_lossless ?
				(max_cu_bit * line_cu_num) : ((img_w * 8 * fbc_cfg.target_cr) / 100);
	u32 total_first_line_bit_budget = fbc_cfg.is_lossless ? total_line_bit_budget : (img_w * 8);
	u32 cu_target_bit = total_line_bit_budget / line_cu_num;
	u32 base_dpcm_q = ((cu_target_bit <= 27) ? 1 : 0);
	u32 base_pcm_bd = (cu_target_bit - cu_md_bit) / fbc_cfg.cu_size;
	u32 min_cu_bit = fbc_cfg.is_lossless ? max_cu_bit : (base_pcm_bd * fbc_cfg.cu_size + cu_md_bit);

	if (base_pcm_bd < 2)
		base_pcm_bd = 2;
	else if (base_pcm_bd > 8)
		base_pcm_bd = 8;

	if (en) {
		reg_14.raw = ISP_RD_REG(fbce, REG_FBCE_T, REG_14);
		reg_14.bits.Y_MAX_CU_BIT		    = max_cu_bit;
		reg_14.bits.Y_MIN_CU_BIT		    = min_cu_bit;
		ISP_WR_REG(fbce, REG_FBCE_T, REG_14, reg_14.raw);

		reg_1C.raw = ISP_RD_REG(fbce, REG_FBCE_T, REG_1C);
		reg_1C.bits.Y_TOTAL_LINE_BIT_BUDGET	= total_line_bit_budget;
		ISP_WR_REG(fbce, REG_FBCE_T, REG_1C, reg_1C.raw);

		reg_10.raw = ISP_RD_REG(fbce, REG_FBCE_T, REG_10);
		reg_10.bits.Y_BASE_QDPCM_Q		= base_dpcm_q;
		reg_10.bits.Y_BASE_PCM_BD_MINUS2	= base_pcm_bd - 2;
		reg_10.bits.Y_CPLX_SHIFT		= CPLX_SHIFT;
		reg_10.bits.Y_PEN_POS_SHIFT		= PEN_POS_SHIFT;
		reg_10.bits.Y_DEFAULT_GR_K		= DEFAULT_K;
		reg_10.bits.Y_LOSSLESS			= fbc_cfg.is_lossless;
		ISP_WR_REG(fbce, REG_FBCE_T, REG_10, reg_10.raw);

		reg_30.raw = ISP_RD_REG(fbce, REG_FBCE_T, REG_30);
		reg_30.bits.Y_TOTAL_FIRST_LINE_BIT_BUDGET = total_first_line_bit_budget;
		ISP_WR_REG(fbce, REG_FBCE_T, REG_30, reg_30.raw);

		reg_24.raw = ISP_RD_REG(fbce, REG_FBCE_T, REG_24);
		reg_24.bits.C_MAX_CU_BIT		     = max_cu_bit;
		reg_24.bits.C_MIN_CU_BIT		     = min_cu_bit;
		ISP_WR_REG(fbce, REG_FBCE_T, REG_24, reg_24.raw);

		reg_2C.raw = ISP_RD_REG(fbce, REG_FBCE_T, REG_2C);
		reg_2C.bits.C_TOTAL_LINE_BIT_BUDGET = total_line_bit_budget;
		ISP_WR_REG(fbce, REG_FBCE_T, REG_2C, reg_2C.raw);

		reg_20.raw = ISP_RD_REG(fbce, REG_FBCE_T, REG_20);
		reg_20.bits.C_BASE_QDPCM_Q		= 0;
		reg_20.bits.C_BASE_PCM_BD_MINUS2	= base_pcm_bd - 2;
		reg_20.bits.C_CPLX_SHIFT		= CPLX_SHIFT;
		reg_20.bits.C_PEN_POS_SHIFT		= PEN_POS_SHIFT;
		reg_20.bits.C_DEFAULT_GR_K		= DEFAULT_K;
		reg_20.bits.C_LOSSLESS			= fbc_cfg.is_lossless;
		ISP_WR_REG(fbce, REG_FBCE_T, REG_20, reg_20.raw);

		reg_34.raw = ISP_RD_REG(fbce, REG_FBCE_T, REG_34);
		reg_34.bits.C_TOTAL_FIRST_LINE_BIT_BUDGET = total_first_line_bit_budget;
		ISP_WR_REG(fbce, REG_FBCE_T, REG_34, reg_34.raw);
	}

	ISP_WR_BITS(fbce, REG_FBCE_T, REG_00, FBCE_EN, en);
}

void ispblk_cnr_config(struct isp_ctx *ctx, bool en)
{
	uintptr_t ba = ctx->phys_regs[ISP_BLK_ID_CNR];
	union REG_ISP_CNR_CTRL_HW_ONLY  ctrl_hw;
	union REG_ISP_CNR_CTRL_SW_HW    ctrl_sw;
	u32 w = ctx->img_width;
	u32 h = ctx->img_height;
	u8 ratio = ctx->cnr_scale_ratio;

	ctrl_hw.raw = ISP_RD_REG(ba, REG_ISP_CNR_T, CNR_CTRL_HW_ONLY);
	ctrl_hw.bits.CNR_ENABLE = en;
	ISP_WR_REG(ba, REG_ISP_CNR_T, CNR_CTRL_HW_ONLY, ctrl_hw.raw);

	ctrl_sw.raw = ISP_RD_REG(ba, REG_ISP_CNR_T, CNR_CTRL_SW_HW);
	ctrl_sw.bits.CNR_CMF_EN = en;
	ctrl_sw.bits.CNR_LCA_ENABLE = en;
	ctrl_sw.bits.CNR_IFE2_FILTER_EN = en;
	ISP_WR_REG(ba, REG_ISP_CNR_T, CNR_CTRL_SW_HW, ctrl_sw.raw);

	ISP_WR_REG(ba, REG_ISP_CNR_T, CNR_LCA_SRC_IMG_SIZE_H, w);
	ISP_WR_REG(ba, REG_ISP_CNR_T, CNR_LCA_SRC_IMG_SIZE_V, h);
	ISP_WR_REG(ba, REG_ISP_CNR_T, CNR_LCA_SUB_IMG_SIZE_H, w / ratio);
	ISP_WR_REG(ba, REG_ISP_CNR_T, CNR_LCA_SUB_IMG_SIZE_V, h / ratio);
	ISP_WR_REG(ba, REG_ISP_CNR_T, CNR_LCA_H_SFACT, 65536 / ratio);
	ISP_WR_REG(ba, REG_ISP_CNR_T, CNR_LCA_V_SFACT, 65536 / ratio);
}

int ispblk_post_ee_config(struct isp_ctx *ctx, bool en)
{
	uintptr_t ba = ctx->phys_regs[ISP_BLK_ID_POST_EE];
	union REG_ISP_EE_00  reg_0;

	reg_0.raw = ISP_RD_REG(ba, REG_ISP_EE_T, REG_00);
	reg_0.bits.EE_ENABLE = en;
	ISP_WR_REG(ba, REG_ISP_EE_T, REG_00, reg_0.raw);

	return 0;
}

void ispblk_ca_config(struct isp_ctx *ctx, bool en, uint8_t mode)
{
	uintptr_t cacp = ctx->phys_regs[ISP_BLK_ID_CA];
	u16 i = 0;
	union REG_ISP_CA_04 wdata;

	ISP_WR_BITS(cacp, REG_ISP_CA_T, REG_00, CACP_ENABLE, en);
	// 0 CA mode, 1 CP mode
	ISP_WR_BITS(cacp, REG_ISP_CA_T, REG_00, CACP_MODE, mode);

	ISP_WR_BITS(cacp, REG_ISP_CA_T, REG_00, CACP_ISO_RATIO, 64);

	ISP_WR_BITS(cacp, REG_ISP_CA_T, REG_00, CACP_MEM_SW_MODE, 1);

	if (mode == 0) {
		for (i = 0; i < sizeof(ca_y_lut) / sizeof(u8); i++) {
			wdata.raw = 0;
			wdata.bits.CACP_MEM_D = ca_y_lut[i];
			wdata.bits.CACP_MEM_W = 1;
			ISP_WR_REG(cacp, REG_ISP_CA_T, REG_04, wdata.raw);
		}
	} else { //cp mode
		for (i = 0; i < sizeof(cp_y_lut) / sizeof(u8); i++) {
			wdata.raw = 0;
			wdata.bits.CACP_MEM_D = ((cp_v_lut[i]) | (cp_u_lut[i] << 8) | (cp_y_lut[i] << 16));
			wdata.bits.CACP_MEM_W = 1;
			ISP_WR_REG(cacp, REG_ISP_CA_T, REG_04, wdata.raw);
		}
	}

	ISP_WR_BITS(cacp, REG_ISP_CA_T, REG_00, CACP_MEM_SW_MODE, 0);
}

void ispblk_ca_lite_config(struct isp_ctx *ctx, bool en)
{
	uintptr_t ca_lite = ctx->phys_regs[ISP_BLK_ID_CA_LITE];

	ISP_WR_BITS(ca_lite, REG_ISP_CA_LITE_T, REG_00, CA_LITE_ENABLE, en);

	ISP_WR_BITS(ca_lite, REG_ISP_CA_LITE_T, REG_04, CA_LITE_LUT_IN_0, 0x0);
	ISP_WR_BITS(ca_lite, REG_ISP_CA_LITE_T, REG_04, CA_LITE_LUT_IN_1, 0x80);

	ISP_WR_BITS(ca_lite, REG_ISP_CA_LITE_T, REG_08, CA_LITE_LUT_IN_2, 0x100);
	ISP_WR_BITS(ca_lite, REG_ISP_CA_LITE_T, REG_08, CA_LITE_LUT_IN_3, 0x100);

	ISP_WR_BITS(ca_lite, REG_ISP_CA_LITE_T, REG_0C, CA_LITE_LUT_IN_4, 0x100);
	ISP_WR_BITS(ca_lite, REG_ISP_CA_LITE_T, REG_0C, CA_LITE_LUT_IN_5, 0x100);

	ISP_WR_BITS(ca_lite, REG_ISP_CA_LITE_T, REG_10, CA_LITE_LUT_OUT_0, 0x100);
	ISP_WR_BITS(ca_lite, REG_ISP_CA_LITE_T, REG_10, CA_LITE_LUT_OUT_1, 0x80);

	ISP_WR_BITS(ca_lite, REG_ISP_CA_LITE_T, REG_14, CA_LITE_LUT_OUT_2, 0x40);
	ISP_WR_BITS(ca_lite, REG_ISP_CA_LITE_T, REG_14, CA_LITE_LUT_OUT_3, 0x40);

	ISP_WR_BITS(ca_lite, REG_ISP_CA_LITE_T, REG_18, CA_LITE_LUT_OUT_4, 0x40);
	ISP_WR_BITS(ca_lite, REG_ISP_CA_LITE_T, REG_18, CA_LITE_LUT_OUT_5, 0x40);

	ISP_WR_BITS(ca_lite, REG_ISP_CA_LITE_T, REG_1C, CA_LITE_LUT_SLP_0, 0x0);
	ISP_WR_BITS(ca_lite, REG_ISP_CA_LITE_T, REG_1C, CA_LITE_LUT_SLP_1, 0x0);

	ISP_WR_BITS(ca_lite, REG_ISP_CA_LITE_T, REG_20, CA_LITE_LUT_SLP_2, 0x0);
	ISP_WR_BITS(ca_lite, REG_ISP_CA_LITE_T, REG_20, CA_LITE_LUT_SLP_3, 0x0);

	ISP_WR_BITS(ca_lite, REG_ISP_CA_LITE_T, REG_24, CA_LITE_LUT_SLP_4, 0x0);
}

void ispblk_ycur_config(struct isp_ctx *ctx, bool en, uint8_t sel, uint16_t *data, bool check_lut)
{
	uintptr_t ycur = ctx->phys_regs[ISP_BLK_ID_YCURVE];
	uint16_t i;
	union REG_ISP_YCURV_YCUR_PROG_DATA reg_data;

	vi_pr(VI_DBG, "YCur LUT using mem[%d]\n", sel);
	ISP_WR_BITS(ycur, REG_ISP_YCURV_T, YCUR_PROG_CTRL, YCUR_PROG_EN, 1);

	ISP_WR_BITS(ycur, REG_ISP_YCURV_T, YCUR_PROG_CTRL, YCUR_WSEL, sel);
	ISP_WR_BITS(ycur, REG_ISP_YCURV_T, YCUR_PROG_ST_ADDR, YCUR_ST_ADDR, 0);
	ISP_WR_BITS(ycur, REG_ISP_YCURV_T, YCUR_PROG_ST_ADDR, YCUR_ST_W, 1);
	ISP_WR_REG(ycur, REG_ISP_YCURV_T, YCUR_PROG_MAX, data[64]);
	for (i = 0; i < 64; i += 2) {
		reg_data.raw = 0;
		reg_data.bits.YCUR_DATA_E = data[i];
		reg_data.bits.YCUR_DATA_O = data[i + 1];
		reg_data.bits.YCUR_W = 1;
		ISP_WR_REG(ycur, REG_ISP_YCURV_T, YCUR_PROG_DATA, reg_data.raw);
	}

	if (check_lut) {
    	bool is_lut_pass = true;
        union REG_ISP_YCURV_YCUR_MEM_SW_MODE sw_mode; 
        union REG_ISP_YCURV_YCUR_MEM_SW_RDATA sw_rdata;
        for (i = 0; i < 64; i++) {
            sw_mode.bits.YCUR_SW_R_MEM_SEL = sel;
            sw_mode.bits.YCUR_SW_RADDR = i;
            ISP_WR_REG(ycur, REG_ISP_YCURV_T, YCUR_SW_MODE, sw_mode.raw);
            ISP_WR_BITS(ycur, REG_ISP_YCURV_T, YCUR_SW_RDATA, YCUR_SW_R, 1);
            sw_rdata.raw = ISP_RD_REG(ycur, REG_ISP_YCURV_T, YCUR_SW_RDATA);
            if ((sw_rdata.raw & 0xFF) != data[i]) 
            {
                is_lut_pass = false;
                break;
            }
        }
        if (is_lut_pass)
            vi_pr(VI_DBG, "YCur LUT Check success\n");
        else
            vi_pr(VI_DBG, "YCur LUT Check fail, lut[%d] = %d, should be %d\n",
                    i, (sw_rdata.raw & 0xFF), data[i]);
	}
	ISP_WR_BITS(ycur, REG_ISP_YCURV_T, YCUR_PROG_CTRL, YCUR_RSEL, sel);
	ISP_WR_BITS(ycur, REG_ISP_YCURV_T, YCUR_PROG_CTRL, YCUR_PROG_EN, 0);
}

void ispblk_ycur_enable(struct isp_ctx *ctx, bool enable, uint8_t sel)
{
	uintptr_t ycur = ctx->phys_regs[ISP_BLK_ID_YCURVE];

	ISP_WR_BITS(ycur, REG_ISP_YCURV_T, YCUR_CTRL, YCUR_ENABLE, enable);
	ISP_WR_BITS(ycur, REG_ISP_YCURV_T, YCUR_PROG_CTRL, YCUR_RSEL, sel);
}
