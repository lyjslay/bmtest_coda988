#include <vip/vi_drv.h>

/****************************************************************************
 * Global parameters
 ****************************************************************************/

/*******************************************************************************
 *	RGB IPs config
 ******************************************************************************/
void ispblk_edge_ext_config(struct isp_ctx *ctx, bool en)
{
	uintptr_t edge_ext = ctx->phys_regs[ISP_BLK_ID_PRE_EE_EXT];

	ISP_WR_BITS(edge_ext, REG_EE_EXT_T, REG0, EE_ENABLE, en);
}

void ispblk_pfr_config(struct isp_ctx *ctx, bool en)
{
	uintptr_t pfr = ctx->phys_regs[ISP_BLK_ID_PFR];
	union REG_ISP_PFR_REG0 reg_ctrl;

	reg_ctrl.raw = ISP_RD_REG(pfr, REG_ISP_PFR_T, REG0);
	reg_ctrl.bits.PFR_EN = en;
	ISP_WR_REG(pfr, REG_ISP_PFR_T, REG0, reg_ctrl.raw);
}

void ispblk_ccm_config(struct isp_ctx *ctx, bool en, struct isp_ccm_cfg *cfg)
{
	uintptr_t ccm = ctx->phys_regs[ISP_BLK_ID_CCM];

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
}

void ispblk_gamma_config(struct isp_ctx *ctx, bool en, uint8_t sel, uint16_t *data, uint8_t inv, bool check_lut)
{
	uintptr_t gamma = ctx->phys_regs[ISP_BLK_ID_RGBGAMMA];
	int16_t i;
	union REG_ISP_GAMMA_PROG_DATA reg_data;
	union REG_ISP_GAMMA_PROG_CTRL prog_ctrl;

	vi_pr(VI_DBG, "Gamma LUT using mem[%d]\n", sel);
	prog_ctrl.raw = ISP_RD_REG(gamma, REG_ISP_GAMMA_T, GAMMA_PROG_CTRL);
	prog_ctrl.bits.GAMMA_WSEL    = sel;
	prog_ctrl.bits.GAMMA_PROG_EN = 1;
	prog_ctrl.bits.GAMMA_PROG_1TO3_EN = 1;
	ISP_WR_REG(gamma, REG_ISP_GAMMA_T, GAMMA_PROG_CTRL, prog_ctrl.raw);

	ISP_WR_BITS(gamma, REG_ISP_GAMMA_T, GAMMA_PROG_ST_ADDR, GAMMA_ST_ADDR, 0);
	ISP_WR_BITS(gamma, REG_ISP_GAMMA_T, GAMMA_PROG_ST_ADDR, GAMMA_ST_W, 1);

	if (inv) {
		for (i = 255; i >= 0; i -= 2) {
			reg_data.raw = 0;
			reg_data.bits.GAMMA_DATA_E = data[i];
			reg_data.bits.GAMMA_DATA_O = data[i + 1];
			reg_data.bits.GAMMA_W = 1;
			ISP_WR_REG(gamma, REG_ISP_GAMMA_T, GAMMA_PROG_DATA, reg_data.raw);
		}

		// set max to 0
		ISP_WR_REG(gamma, REG_ISP_GAMMA_T, GAMMA_PROG_MAX, 0);
	} else {
		for (i = 0; i < 256; i += 2) {
			reg_data.raw = 0;
			reg_data.bits.GAMMA_DATA_E = data[i];
			reg_data.bits.GAMMA_DATA_O = data[i + 1];
			reg_data.bits.GAMMA_W = 1;
			ISP_WR_REG(gamma, REG_ISP_GAMMA_T, GAMMA_PROG_DATA, reg_data.raw);
		}
	}

	if (check_lut) {
		bool is_lut_pass = true;
		union REG_ISP_GAMMA_MEM_SW_RADDR sw_raddr;
		union REG_ISP_GAMMA_MEM_SW_RDATA sw_rdata;

		for (i = 0; i < 256; i++) {
			sw_raddr.bits.GAMMA_SW_R_MEM_SEL = sel;
			sw_raddr.bits.GAMMA_SW_RADDR = i;
			ISP_WR_REG(gamma, REG_ISP_GAMMA_T, GAMMA_SW_RADDR, sw_raddr.raw);
			ISP_WR_BITS(gamma, REG_ISP_GAMMA_T, GAMMA_SW_RDATA, GAMMA_SW_R, 1);

			sw_rdata.raw = ISP_RD_REG(gamma, REG_ISP_GAMMA_T, GAMMA_SW_RDATA);
			if ((sw_rdata.raw & 0xFFF) != data[i]) 
			{
				is_lut_pass = false;
				break;
			}
		}
		if (is_lut_pass)
			vi_pr(VI_DBG, "RGBgamma LUT Check success\n");
		else
			vi_pr(VI_DBG, "RGBgamma LUT Check fail, lut[%d] = %d, should be %d\n",
					i, (sw_rdata.raw & 0xFFF), data[i]);

	}
	ISP_WR_BITS(gamma, REG_ISP_GAMMA_T, GAMMA_PROG_CTRL, GAMMA_RSEL, sel);
	ISP_WR_BITS(gamma, REG_ISP_GAMMA_T, GAMMA_PROG_CTRL, GAMMA_PROG_EN, 0);
}

void ispblk_gamma_enable(struct isp_ctx *ctx, bool enable)
{
	uintptr_t gamma = ctx->phys_regs[ISP_BLK_ID_RGBGAMMA];

	ISP_WR_BITS(gamma, REG_ISP_GAMMA_T, GAMMA_CTRL, GAMMA_ENABLE, enable);
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
void ispblk_rgbdither_config(struct isp_ctx *ctx, bool en, bool mod_en,
			    bool histidx_en, bool fmnum_en)
{
	uintptr_t rgbdither = ctx->phys_regs[ISP_BLK_ID_RGB_DITHER];
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
}

void ispblk_clut_config(struct isp_ctx *ctx, bool en,
				uint16_t *r_lut, uint16_t *g_lut, uint16_t *b_lut, bool check_lut)
{
	uintptr_t clut = ctx->phys_regs[ISP_BLK_ID_CLUT];
	uint16_t r_idx, g_idx, b_idx;
	union REG_ISP_CLUT_CTRL      ctrl;
	union REG_ISP_CLUT_PROG_ADDR prog_addr;
	union REG_ISP_CLUT_PROG_DATA prog_data;
	u32 idx = 0;

	ctrl.raw = ISP_RD_REG(clut, REG_ISP_CLUT_T, CLUT_CTRL);
	ctrl.bits.PROG_EN = 1;
	ISP_WR_REG(clut, REG_ISP_CLUT_T, CLUT_CTRL, ctrl.raw);

	for (b_idx = 0; b_idx < 17; b_idx++) {
		for (g_idx = 0; g_idx < 17; g_idx++) {
			for (r_idx = 0; r_idx < 17; r_idx++) {
				idx = b_idx * 289 + g_idx * 17 + r_idx;

				prog_addr.raw = 0;
				prog_addr.bits.SRAM_R_IDX = r_idx;
				prog_addr.bits.SRAM_G_IDX = g_idx;
				prog_addr.bits.SRAM_B_IDX = b_idx;
				ISP_WR_REG(clut, REG_ISP_CLUT_T, CLUT_PROG_ADDR, prog_addr.raw);

				prog_data.raw		  = 0;
				prog_data.bits.SRAM_WDATA = b_lut[idx] + (g_lut[idx] << 10) + (r_lut[idx] << 20);
				prog_data.bits.SRAM_WR	  = 1;
				ISP_WR_REG(clut, REG_ISP_CLUT_T, CLUT_PROG_DATA, prog_data.raw);
			}
		}
	}

	if (check_lut) {
		union REG_ISP_CLUT_PROG_RDATA prog_prog_rdata;
		bool is_lut_pass = true;
		u32 val = 0;
		for (b_idx = 0; b_idx < 17; b_idx++) {
			for (g_idx = 0; g_idx < 17; g_idx++) {
				for (r_idx = 0; r_idx < 17; r_idx++) {
					idx = b_idx * 289 + g_idx * 17 + r_idx;

					prog_addr.raw = 0;
					prog_addr.bits.SRAM_R_IDX = r_idx;
					prog_addr.bits.SRAM_G_IDX = g_idx;
					prog_addr.bits.SRAM_B_IDX = b_idx;
					ISP_WR_REG(clut, REG_ISP_CLUT_T, CLUT_PROG_ADDR, prog_addr.raw);
					
					prog_prog_rdata.raw = 0;
					ISP_WR_BITS(clut, REG_ISP_CLUT_T, CLUT_PROG_RDATA, SRAM_RD, 1);
					prog_prog_rdata.bits.SRAM_RDATA = ISP_RD_BITS(clut, REG_ISP_CLUT_T, CLUT_PROG_RDATA, SRAM_RDATA);
					val = b_lut[idx] + (g_lut[idx] << 10) + (r_lut[idx] << 20);
					if ((prog_prog_rdata.bits.SRAM_RDATA & 0x3FFFFFFF)!= val){
						is_lut_pass = false;
						break;
					}
				}
			}
		}
	    if (is_lut_pass)
            vi_pr(VI_DBG, "C_LUT Check success\n");
        else
            vi_pr(VI_DBG, "C_LUT Check fail, lut[%d] = %d, should be %d\n",
                    idx, (prog_prog_rdata.bits.SRAM_RDATA & 0x3FFFFFFF), val);

	}
	ctrl.bits.CLUT_ENABLE = en;
	ctrl.bits.PROG_EN = 0;
	ISP_WR_REG(clut, REG_ISP_CLUT_T, CLUT_CTRL, ctrl.raw);
}

void ispblk_csc_config(struct isp_ctx *ctx)
{
	uintptr_t csc = ctx->phys_regs[ISP_BLK_ID_CSC];

	ISP_WR_BITS(csc, REG_ISP_CSC_T, REG_0, CSC_ENABLE, 1);
}