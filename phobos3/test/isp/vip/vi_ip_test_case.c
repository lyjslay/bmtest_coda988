#include <vip/vi_drv.h>
#include <vi_common.h>

#ifdef TEST_PERF
bool bypass = false;
#else
bool bypass = true;
#endif


/****************************************************************************
 * Global parameters
 ****************************************************************************/
enum VI_TEST_CASE {
	VI_TEST_RGBGAMMA_ENABLE = 1,
	VI_TEST_RGBGAMMA_ENABLE_TBL_INVERSE,
	VI_TEST_CNR_ENABLE,
	VI_TEST_CNR_SET_LUT,
	VI_TEST_BNR_ENABLE_WEAK_STRENGTH,
	VI_TEST_BNR_ENABLE_STRONG_STRENGTH,
	VI_TEST_MAPCURVE_ENABLE,
	VI_TEST_BLC_GAIN_800,
	VI_TEST_BLC_GAIN_B00,
	VI_TEST_BLC_OFFSET_1FF,
	VI_TEST_BLC_GAIN_800_OFFSET_1FF,
	VI_TEST_BLC_GAIN_800_OFFSET_1FF_2ND_OFFSET_1FF,
	VI_TEST_WBG_GAIN_800,
	VI_TEST_DPC_ENABLE,
	VI_TEST_DPC_ENABLE_STATIC_ONLY,
	VI_TEST_DPC_ENABLE_DYNAMIC_ONLY,
	VI_TEST_PREEE_ENABLE,
	VI_TEST_EE_ENABLE,
	VI_TEST_CCM_ENABLE,
	VI_TEST_YCURVE_ENABLE_TBL_INVERSE,
	VI_TEST_FUSION_LE_OUTPUT,
	VI_TEST_FUSION_SE_OUTPUT,
	VI_TEST_AE_HIST_ENABLE,
	VI_TEST_GMS_ENABLE,
	VI_TEST_AF_ENABLE,
	VI_TEST_LSC_ENABLE,
	VI_TEST_DCI_ENABLE,
	VI_TEST_LDCI_ENABLE,
	VI_TEST_LDCI_ENABLE_TONE_CURVE_LUT_P_1023,
	VI_TEST_RGB_DITHER_OFF,
	VI_TEST_RGB_DITHER_ON,
	VI_TEST_YUV_DITHER_OFF,
	VI_TEST_YUV_DITHER_ON,
	VI_TEST_CLUT_ON,
	VI_TEST_DCI_ENABLE_DEMO_MODE_ALL_FF_WHITE,
	VI_TEST_DCI_ENABLE_DEMO_MODE_ALL_FF_BLACK,
	VI_TEST_CACP_OFF,
	VI_TEST_CA_ON_MODE_0,
	VI_TEST_CP_ON_MODE_1,
	VI_TEST_CP_LITE_OFF,
	VI_TEST_CP_LITE_ON,
	VI_TEST_CNR_OFF,
	VI_TEST_CNR_HW_AUTO_ENABLE_ISO_2,
	VI_TEST_CROP_YUV,
	VI_TEST_CROP_VI_SEL,
	VI_TEST_CROP_RAW,
	VI_TEST_PREEE_EE_DISABLE,
	VI_TEST_LUT_TABLE,
};

extern int vi_ip_test_case;
extern struct isp_ccm_cfg ccm_hw_cfg;
extern uint16_t ygamma_data[];
extern uint16_t gamma_data[];
extern uint16_t ycur_data[];
extern u16 dci_map_lut_50[];
extern uint16_t ltm_d_lut[];
extern uint16_t ltm_b_lut[];
extern uint16_t ltm_g_lut[];

extern uint16_t c_lut_r_lut[];
extern uint16_t c_lut_g_lut[];
extern uint16_t c_lut_b_lut[];

/*******************************************************************************
 *	IPs test case config
 ******************************************************************************/
static void vi_ip_all_bypass(struct isp_ctx *ictx)
{
	//for 3ndr modify ctx->is_3dnr_on = true, otherwise false(vi.c _vi_sw_init);
	vi_pr(VI_INFO, "all ip bypass\n");

	//csibdg
	struct vi_rect crop;

	ispblk_csibdg_wdma_crop_config(ictx, ISP_PRERAW0, crop, 0);

	//fe
	ispblk_lsc_config(ictx, ISP_PRERAW0, true,  false);
	if (ictx->is_hdr_on) {
		ispblk_lsc_config(ictx, ISP_PRERAW0, false, false);
	}
	ispblk_aehist_config(ictx, ISP_PRERAW0, false);

	//postraw
	//rawtop
	ispblk_blc_enable(ictx, false);
	ispblk_wbg_enable(ictx, false);
	ispblk_dpc_config(ictx, false, 0);
	ispblk_af_config(ictx, false);
	ispblk_mapcurve_config(ictx, false);
	ispblk_bnr_lite_config(ictx, false);
	ispblk_bnr_config(ictx, false);
	ispblk_af_config(ictx, false);
	ispblk_lscr_config(ictx, false);

	if (ictx->is_hdr_on) {
		ispblk_fusion_config(ictx, true, ISP_FS_OUT_FS);
	} else {
		ispblk_fusion_config(ictx, false, ISP_FS_OUT_LONG);
	}
	ispblk_drc_config(ictx, false);

	//rgbtop
	ispblk_edge_ext_config(ictx, false);
	ispblk_pfr_config(ictx, false);
	ispblk_ccm_config(ictx, false, &ccm_hw_cfg);
	ispblk_gamma_enable(ictx, false);
	ispblk_rgbdither_config(ictx, false, false, false, false);
	// ispblk_clut_config(ictx, false, c_lut_r_lut, c_lut_g_lut, c_lut_b_lut, false);

	//yuvtop
	ispblk_yuvdither_config(ictx, 0, false, true, true, true);
	ispblk_yuvdither_config(ictx, 1, false, true, true, true);

	ispblk_pre_ee_front_config(ictx, !bypass);
	ispblk_pre_ee_back_config(ictx, !bypass);

	ispblk_tnr_config(ictx, ictx->is_3dnr_on);

	ispblk_cnr_config(ictx, false);

	ispblk_post_ee_config(ictx, false);

	ispblk_ycur_enable(ictx, false, 0);

	ispblk_ldci_config(ictx, false, 0);

	ispblk_ca_config(ictx, false, 1);
	ispblk_ca_lite_config(ictx, false);

	// ictx->isp_pipe_cfg[ISP_PRERAW0].postout_crop.w = 1920;
	// ictx->isp_pipe_cfg[ISP_PRERAW0].postout_crop.h = 1080;

	ispblk_crop_enable(ictx, ISP_BLK_ID_YUV_CROP_Y, false);
	ispblk_crop_enable(ictx, ISP_BLK_ID_YUV_CROP_C, false);

	ispblk_crop_enable(ictx, ISP_BLK_ID_PRE_RAW_VI_SEL_CROP_LE, false);
	ispblk_crop_enable(ictx, ISP_BLK_ID_PRE_RAW_VI_SEL_CROP_LE, false);

	vi_pr(VI_INFO, "bypass config done\n");
}

void vi_ip_test_cases_init(struct isp_ctx *ctx)
{
	vi_pr(VI_INFO, "bypass addr= 0x%p, value[%d]\n", &bypass, bypass);

	vi_ip_all_bypass(ctx);

	switch (vi_ip_test_case) {
	case VI_TEST_RGBGAMMA_ENABLE: //rgbgamma enable
	{
		vi_pr(VI_INFO, "RGBgamma enable\n");

		ispblk_gamma_config(ctx, false, ctx->gamma_tbl_idx, gamma_data, 0, false);
		ispblk_gamma_enable(ctx, true);
		break;
	}
	case VI_TEST_RGBGAMMA_ENABLE_TBL_INVERSE: //2
	{
		vi_pr(VI_INFO, "RGBgamma enable, tbl inverse\n");

		ispblk_gamma_config(ctx, false, ctx->gamma_tbl_idx, gamma_data, 1, false);
		ispblk_gamma_enable(ctx, true);
		break;
	}
	case VI_TEST_CNR_ENABLE: //3
	{
		vi_pr(VI_INFO, "CNR enable\n");

		ispblk_cnr_config(ctx, true);
		break;
	}
	case VI_TEST_MAPCURVE_ENABLE: //10
	{
		vi_pr(VI_INFO, "mapcurve enable\n");

		ispblk_mapcurve_config(ctx, false);
		break;
	}
	case VI_TEST_BLC_GAIN_B00: //12
	{
		vi_pr(VI_INFO, " enable, gain 0xB00\n");

		ispblk_blc_set_gain(ctx, 0xB00, 0xB00, 0xB00, 0xB00);
		ispblk_blc_enable(ctx, true);
		break;
	}
	case VI_TEST_BLC_OFFSET_1FF: //13
	{
		vi_pr(VI_INFO, "BLC be_le enable, offset 0x1ff\n");

		ispblk_blc_set_offset(ctx, 0x1FF, 0x1FF, 0x1FF, 0x1FF);
		ispblk_blc_enable(ctx, true);
		break;
	}
	case VI_TEST_WBG_GAIN_800: //16
	{
		vi_pr(VI_INFO, "WBG rawtop_le enable, gain 0x800\n");

		ispblk_wbg_config(ctx, 0x800, 0x800, 0x800);
		ispblk_wbg_enable(ctx, true);
		break;
	}
	case VI_TEST_DPC_ENABLE: //17
	{
		vi_pr(VI_INFO, "DPC enable\n");
		ispblk_dpc_config(ctx, true, 0);
		break;
	}
	case VI_TEST_DPC_ENABLE_STATIC_ONLY: //18
	{
		vi_pr(VI_INFO, "DPC enable static only\n");
		ispblk_dpc_config(ctx, true, 1);
		if (ctx->is_hdr_on)
			ispblk_dpc_config(ctx, true, 1);
		break;
	}
	case VI_TEST_DPC_ENABLE_DYNAMIC_ONLY: //19
	{
		vi_pr(VI_INFO, "DPC enable dynamic only\n");
		ispblk_dpc_config(ctx, true, 2);
		if (ctx->is_hdr_on)
			ispblk_dpc_config(ctx, true, 2);
		break;
	}
	case VI_TEST_PREEE_ENABLE: //20
	{
		vi_pr(VI_INFO, "PREEE enable\n");
		ispblk_post_ee_config(ctx, false);
		ispblk_pre_ee_front_config(ctx, true);
		ispblk_pre_ee_back_config(ctx, true);
		break;
	}
	case VI_TEST_EE_ENABLE: //21
	{
		vi_pr(VI_INFO, "EE enable\n");
		ispblk_post_ee_config(ctx, false);
		ispblk_pre_ee_front_config(ctx, true);
		ispblk_pre_ee_back_config(ctx, true);
		break;
	}
	case VI_TEST_PREEE_EE_DISABLE: //71
	{
		vi_pr(VI_INFO, "PREEE EE disable\n");
		ispblk_post_ee_config(ctx, false);
		ispblk_pre_ee_front_config(ctx, false);
		ispblk_pre_ee_back_config(ctx, false);
		break;
	}
	case VI_TEST_CCM_ENABLE: //22
	{
		vi_pr(VI_INFO, "CCM enable\n");
		ispblk_ccm_config(ctx, true, &ccm_hw_cfg);
		break;
	}
	case VI_TEST_YCURVE_ENABLE_TBL_INVERSE: //23
	{
		vi_pr(VI_INFO, "YCURVE enable, tbl inverse\n");
		ispblk_ycur_config(ctx, true, ctx->gamma_tbl_idx, ycur_data, false);
		ispblk_ycur_enable(ctx, true, 0);
		break;
	}
	case VI_TEST_FUSION_LE_OUTPUT: //29
	{
		vi_pr(VI_INFO, "ltm disable, le frame output\n");
		ispblk_fusion_config(ctx, true, ISP_FS_OUT_LONG);
		break;
	}
	case VI_TEST_FUSION_SE_OUTPUT: //30
	{
		vi_pr(VI_INFO, "ltm disable, se frame output\n");
		ispblk_fusion_config(ctx, true, ISP_FS_OUT_SHORT);
		break;
	}
	case VI_TEST_AE_HIST_ENABLE: //31
	{
		vi_pr(VI_INFO, "AE_HIST enable\n");

		ispblk_aehist_config(ctx, ISP_PRERAW0, true);

		break;
	}
	case VI_TEST_AF_ENABLE: //42
	{
		vi_pr(VI_INFO, "af enable\n");
		ispblk_af_config(ctx, true);
		break;
	}
	case VI_TEST_LSC_ENABLE: //43
	{
		vi_pr(VI_INFO, "lsc enable\n");
		ispblk_lsc_config(ctx, ISP_PRERAW0, true, true);
		if (ctx->is_hdr_on) {
			ispblk_lsc_config(ctx, ISP_PRERAW0, false, true);
			writel(0x680390a0, 0x1001); //后续修改
		}
		break;
	}
	case VI_TEST_DCI_ENABLE: //35
	{
		vi_pr(VI_INFO, "DCI enable\n");
		ispblk_ldci_config(ctx, true, 0);
		break;
	}
	case VI_TEST_LDCI_ENABLE: //44
	{
		vi_pr(VI_INFO, "ldci enable\n");
		ispblk_ldci_config(ctx, true, 0);
		break;
	}
	case VI_TEST_LDCI_ENABLE_TONE_CURVE_LUT_P_1023: //45
	{
		vi_pr(VI_INFO, "ldci enable, TONE_CURVE_LUT_P_1023\n");
		ispblk_ldci_config(ctx, true, 1);
		break;
	}
	case VI_TEST_RGB_DITHER_OFF: //52
	{
		vi_pr(VI_INFO, "RGB DITHER disable\n");
		ispblk_rgbdither_config(ctx, false, false, false, false);
		break;
	}
	case VI_TEST_RGB_DITHER_ON: //53
	{
		vi_pr(VI_INFO, "RGB DITHER enable\n");
		ispblk_rgbdither_config(ctx, true, true, true, true);
		break;
	}
	case VI_TEST_YUV_DITHER_OFF: //54
	{
		vi_pr(VI_INFO, "YUV DITHER disable\n");
		ispblk_yuvdither_config(ctx, 0, false, false, false, false);
		ispblk_yuvdither_config(ctx, 1, false, false, false, false);
		break;
	}
	case VI_TEST_YUV_DITHER_ON: //55
	{
		vi_pr(VI_INFO, "YUV DITHER enable\n");
		ispblk_yuvdither_config(ctx, 0, true, true, true, true);
		ispblk_yuvdither_config(ctx, 1, true, true, true, true);
		break;
	}
	case VI_TEST_CLUT_ON: //56
	{
		vi_pr(VI_INFO, "CLUT enable\n");
		ispblk_clut_config(ctx, true, c_lut_r_lut, c_lut_g_lut, c_lut_b_lut, false);
		break;
	}
	case VI_TEST_DCI_ENABLE_DEMO_MODE_ALL_FF_WHITE: //57
	{
		vi_pr(VI_INFO, "LDCI enable, demo_mode all white\n");
		ispblk_ldci_config(ctx, true, 1);
		break;
	}
	case VI_TEST_CACP_OFF: //59
	{
		vi_pr(VI_INFO, "CACP OFF\n");
		ispblk_ca_config(ctx, false, 1);
		break;
	}
	case VI_TEST_CA_ON_MODE_0: //60
	{
		vi_pr(VI_INFO, "CA ON, mode 0\n");
		ispblk_ca_config(ctx, true, 0);
		break;
	}
	case VI_TEST_CP_ON_MODE_1: //61
	{
		vi_pr(VI_INFO, "CP ON, mode 1\n");
		ispblk_ca_config(ctx, true, 1);
		break;
	}
	case VI_TEST_CP_LITE_OFF: //62
	{
		vi_pr(VI_INFO, "CP LITE OFF\n");
		ispblk_patgen_config_pat(ctx, ISP_PRERAW0, 3);
		ispblk_ca_lite_config(ctx, false);
		break;
	}
	case VI_TEST_CP_LITE_ON: //63
	{
		vi_pr(VI_INFO, "CP LITE ON\n");
		ispblk_patgen_config_pat(ctx, ISP_PRERAW0, 3);
		ispblk_ca_lite_config(ctx, true);
		break;
	}
	case VI_TEST_CNR_OFF: //64
	{
		vi_pr(VI_INFO, "CNR ALL OFF\n");
		ispblk_cnr_config(ctx, false);
		break;
	}
	case VI_TEST_CROP_YUV:
	{
		vi_pr(VI_INFO, " crop yuv\n");

		struct vi_rect crop;

		crop.x = 0;
		crop.y = 0;
		crop.w = 1920;
		crop.h = 1080;
		ctx->isp_pipe_cfg[ISP_PRERAW0].postout_crop = crop;
		ispblk_crop_config(ctx, ISP_BLK_ID_YUV_CROP_Y, crop);
		crop.w >>= 1;
		crop.h >>= 1;
		ispblk_crop_config(ctx, ISP_BLK_ID_YUV_CROP_C, crop);


		ispblk_crop_enable(ctx, ISP_BLK_ID_YUV_CROP_Y, true);
		ispblk_crop_enable(ctx, ISP_BLK_ID_YUV_CROP_C, true);

		break;
	}
	case VI_TEST_CROP_VI_SEL:
	{
		vi_pr(VI_INFO, " crop vi_sel_raw\n");

		struct vi_rect crop;

		crop.x = 0;
		crop.y = 0;
		crop.w = 1920;
		crop.h = 1080;
		ispblk_crop_config(ctx, ISP_BLK_ID_PRE_RAW_VI_SEL_CROP_LE, crop);
		ispblk_crop_enable(ctx, ISP_BLK_ID_PRE_RAW_VI_SEL_CROP_LE, true);

		break;
	}
	case VI_TEST_CROP_RAW:
	{
		vi_pr(VI_INFO, " crop raw\n");

		struct vi_rect crop;

		crop.x = 0;
		crop.y = 0;
		crop.w = 1920;
		crop.h = 1080;
		ispblk_csibdg_wdma_crop_config(ctx, ISP_PRERAW0, crop, 1);
		break;
	}
	case VI_TEST_LUT_TABLE: //72
	{
		ispblk_ca_read_lut_test(ctx, 0, 0);
		ispblk_ca_read_lut_test(ctx, 0, 1);

		/* rgbgamma */
		ispblk_rgbgamma_read_lut_test(ctx, 0, 0);
		ispblk_rgbgamma_read_lut_test(ctx, 0, 1);

		/* ldci */
		ispblk_ldci_read_lut_test(ctx, 0); //sel 0
		ispblk_ldci_read_lut_test(ctx, 1); //sel 1

		/* ycur */
		ispblk_ycur_read_lut_test(ctx, 0);
		ispblk_ycur_read_lut_test(ctx, 1);

		/* clut */
		ispblk_clut_read_lut_test(ctx);

		break;
	}
	default:
		break;
	}

	vi_pr(VI_INFO, "vi_ip_test_case = %d\n", vi_ip_test_case);
}

void vi_ip_test_cases_uninit(struct isp_ctx *ctx) //need restore default status
{
	// union vip_sys_reset mask;

	// mask.raw = 0;
	// mask.b.isp_top_apb = 1;
	// mask.b.isp_top = 1;

	// vip_toggle_reset(mask);
}