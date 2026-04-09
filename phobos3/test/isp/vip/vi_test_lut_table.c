#include <vip/vi_drv.h>
#include <vi_common.h>

extern uint16_t ygamma_data[];
extern uint16_t gamma_data[];
extern uint16_t ycur_data[];
extern u16 dci_map_lut_50[];
extern uint16_t drc_lut[];
extern uint16_t ltm_b_lut[];
extern uint16_t ltm_g_lut[];

extern uint16_t c_lut_r_lut[];
extern uint16_t c_lut_g_lut[];
extern uint16_t c_lut_b_lut[];
extern u8 ca_y_lut[];
extern u8 cp_y_lut[];
extern u8 cp_u_lut[];
extern u8 cp_v_lut[];

void ispblk_clut_read_lut_test(struct isp_ctx *ctx)
{
	vi_pr(VI_DBG, "c_lut start check ...\n");
	ispblk_clut_config(ctx, false, c_lut_r_lut, c_lut_g_lut, c_lut_b_lut, true);
}

void ispblk_drc_read_lut_test(struct isp_ctx *ctx, uint8_t sel)
{
	vi_pr(VI_DBG, "drc start check mem[%d]\n", sel);
	ispblk_drc_lut(ctx, sel, drc_lut, true);
}

void ispblk_ldci_read_lut_test(struct isp_ctx *ctx, uint8_t sel)
{
	vi_pr(VI_DBG, "ldci_map_lut_50 start check mem[%d]\n", sel);
	ispblk_ldci_config(ctx, true, 1);
}

void ispblk_ycur_read_lut_test(struct isp_ctx *ctx, uint8_t sel)
{
	vi_pr(VI_DBG, "ycur lut start check mem[%d]\n", sel);
	ispblk_ycur_config(ctx, false, sel, ycur_data, true);
}

void ispblk_rgbgamma_read_lut_test(struct isp_ctx *ctx, uint8_t inv, uint8_t sel)
{
	vi_pr(VI_DBG, "rgb gamma lut start check mem[%d] inv[%d]\n", sel, inv);
	ispblk_gamma_config(ctx, false, sel, gamma_data, inv, true);
}

void ispblk_ca_read_lut_test(struct isp_ctx *ctx, bool en, uint8_t mode)
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
		vi_pr(VI_DBG, "ca mem check\n");
		for (i = 0; i < 256; i++) {
			wdata.raw = 0;
			wdata.bits.CACP_MEM_D = ca_y_lut[i];
			wdata.bits.CACP_MEM_W = 1;
			ISP_WR_REG(cacp, REG_ISP_CA_T, REG_04, wdata.raw);
		}
	} else { //cp mode
		vi_pr(VI_DBG, "cp mem check\n");
		for (i = 0; i < 256; i++) {
			wdata.raw = 0;
			wdata.bits.CACP_MEM_D = ((cp_v_lut[i]) | (cp_u_lut[i] << 8) | (cp_y_lut[i] << 16));
			wdata.bits.CACP_MEM_W = 1;
			ISP_WR_REG(cacp, REG_ISP_CA_T, REG_04, wdata.raw);
		}
	}

	ISP_WR_BITS(cacp, REG_ISP_CA_T, REG_00, CACP_MEM_SW_MODE, 0);

	vi_pr(VI_DBG, "start read check\n");
	//sw read mem
	ISP_WR_BITS(cacp, REG_ISP_CA_T, REG_00, CACP_MEM_SW_MODE, 1);

	bool is_lut_pass = true;

	for (i = 0; i < 256; i++) {
		u32 control, data;

		control = ISP_RD_REG(cacp, REG_ISP_CA_T, REG_0C);
		control = (control & (~0xFF)) | i;
		ISP_WR_REG(cacp, REG_ISP_CA_T, REG_0C, control);

		control = ISP_RD_REG(cacp, REG_ISP_CA_T, REG_0C);
		control = control | (0x1 << 31);
		ISP_WR_REG(cacp, REG_ISP_CA_T, REG_0C, control);

		data = ISP_RD_REG(cacp, REG_ISP_CA_T, REG_10);

		if (mode == 0){
			if (ca_y_lut[i] != (data & 0x7FF)){
				vi_pr(VI_ERR, "expect %x, but data[%x]\n", ca_y_lut[i], data);
				is_lut_pass = false;
			}
		} else {
			if ((cp_y_lut[i] != ((data >> 16) & 0xFF)) || 
				(cp_u_lut[i] != ((data >> 8) & 0xFF)) ||
				(cp_v_lut[i] != (data & 0xFF))) {
				vi_pr(VI_ERR, "cp_y_lut expect %x, but data[%x]\n", cp_y_lut[i], ((data >> 16) & 0xFF));
				vi_pr(VI_ERR, "cp_u_lut expect %x, but data[%x]\n", cp_u_lut[i], ((data >> 8) & 0xFF));
				vi_pr(VI_ERR, "cp_v_lut expect %x, but data[%x]\n", cp_v_lut[i], (data & 0xFF));
				is_lut_pass = false;
			}
		}
	}

	if (is_lut_pass)
		vi_pr(VI_ERR, "cacp mem pass\n");

	ISP_WR_BITS(cacp, REG_ISP_CA_T, REG_00, CACP_MEM_SW_MODE, 0);
}