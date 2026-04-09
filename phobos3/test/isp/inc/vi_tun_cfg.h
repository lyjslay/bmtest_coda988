/*
 * Copyright (C) Cvitek Co., Ltd. 2019-2020. All rights reserved.
 *
 * File Name: cvi_vip_tun_cfg.h
 * Description:
 */

#ifndef _U_CVI_VIP_TUN_CFG_H_
#define _U_CVI_VIP_TUN_CFG_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "isp_reg.h"

typedef s8 __s8;
typedef u8 __u8;

typedef s16 __s16;
typedef u16 __u16;

typedef s32 __s32;
typedef u32 __u32;


#define TUNING_NODE_NUM  2

enum cvi_isp_raw {
	ISP_PRERAW0,
	ISP_PRERAW1,
	ISP_PRERAW2,
	ISP_PRERAW_MAX,
};

enum cvi_isp_chn_num {
	ISP_CHN0,
	ISP_CHN1,
	ISP_CHN2,
	ISP_CHN3,
	ISP_CHN_MAX,
};

enum cvi_isp_pre_chn_num {
	ISP_FE_CH0,
	ISP_FE_CH1,
	ISP_FE_CH2,
	ISP_FE_CH3,
	ISP_FE_CHN_MAX,
};

struct cvi_vip_isp_blc_config {
	__u8  update;
	__u8  inst;
	__u8  enable;
	__u8  bypass;
	__u16 roffset;
	__u16 groffset;
	__u16 gboffset;
	__u16 boffset;
	__u16 roffset_2nd;
	__u16 groffset_2nd;
	__u16 gboffset_2nd;
	__u16 boffset_2nd;
	__u16 rgain;
	__u16 grgain;
	__u16 gbgain;
	__u16 bgain;
};

struct cvi_vip_isp_wbg_config {
	__u8  update;
	__u8  inst;
	__u8  enable;
	__u8  bypass;
	__u16 rgain;
	__u16 ggain;
	__u16 bgain;
	__u32 rgain_fraction;
	__u32 ggain_fraction;
	__u32 bgain_fraction;
};

/* struct cvi_vip_isp_ccm_config
 * @enable: ccm module enable or not
 * @coef: s3.10, (2's complement)
 */
struct cvi_vip_isp_ccm_config {
	__u8  update;
	__u8  inst;
	__u8  enable;
	__u16 coef[3][3];
};

struct cvi_vip_isp_cacp_config {
	__u8  update;
	__u8  enable;
	__u8  mode;
	__u16 iso_ratio;
	__u16 ca_y_ratio_lut[256];
	__u8  cp_y_lut[256];
	__u8  cp_u_lut[256];
	__u8  cp_v_lut[256];
};

struct cvi_vip_isp_ca2_config {
	__u8  update;
	__u8  enable;
	__u16 lut_in[6];
	__u16 lut_out[6];
	__u16 lut_slp[5];
};

struct cvi_vip_isp_gamma_config {
	__u8  update;
	__u8  enable;
	__u16 max;
	__u16 lut[256];
};

struct cvi_isp_demosiac_tun_cfg {
	union REG_ISP_CFA_0C                     reg_0c;
	union REG_ISP_CFA_10                     reg_10;
	union REG_ISP_CFA_14                     reg_14;
	union REG_ISP_CFA_18                     reg_18;
	union REG_ISP_CFA_1C                     reg_1c;
};

struct cvi_isp_demosiac_tun_1_cfg {
	union REG_ISP_CFA_120                   reg_120;
	union REG_ISP_CFA_124                   reg_124;
	union REG_ISP_CFA_128                   reg_128;
	union REG_ISP_CFA_12C                   reg_12c;
	union REG_ISP_CFA_130                   reg_130;
	union REG_ISP_CFA_134                   reg_134;
	union REG_ISP_CFA_138                   reg_138;
	union REG_ISP_CFA_13C                   reg_13c;
	union REG_ISP_CFA_140                   reg_140;
	union REG_ISP_CFA_144                   reg_144;
	union REG_ISP_CFA_148                   reg_148;
	union REG_ISP_CFA_14C                   reg_14c;
	union REG_ISP_CFA_150                   reg_150;
	union REG_ISP_CFA_154                   reg_154;
	union REG_ISP_CFA_158                   reg_158;
	union REG_ISP_CFA_15C                   reg_15c;
	union REG_ISP_CFA_160                   reg_160;
	union REG_ISP_CFA_164                   reg_164;
	union REG_ISP_CFA_168                   reg_168;
	union REG_ISP_CFA_16C                   reg_16c;
	union REG_ISP_CFA_170                   reg_170;
	union REG_ISP_CFA_174                   reg_174;
	union REG_ISP_CFA_178                   reg_178;
	union REG_ISP_CFA_17C                   reg_17c;
	union REG_ISP_CFA_180                   reg_180;
	union REG_ISP_CFA_184                   reg_184;
	union REG_ISP_CFA_188                   reg_188;
	union REG_ISP_CFA_18C                   reg_18c;
	union REG_ISP_CFA_190                   reg_190;
};

struct cvi_isp_demosiac_tun_2_cfg {
	union REG_ISP_CFA_90                    reg_90;
	union REG_ISP_CFA_94                    reg_94;
	union REG_ISP_CFA_98                    reg_98;
	union REG_ISP_CFA_9C                    reg_9c;
	union REG_ISP_CFA_A0                    reg_a0;
	union REG_ISP_CFA_A4                    reg_a4;
	union REG_ISP_CFA_A8                    reg_a8;
};

struct cvi_vip_isp_demosiac_config {
	__u8  update;
	__u8  cfa_enable;
	__u16 cfa_edgee_thd2;
	__u8  cfa_out_sel;
	__u8  cfa_force_dir_enable;
	__u8  cfa_force_dir_sel;
	__u16 cfa_rbsig_luma_thd;
	__u8  cfa_ghp_lut[32];
	__u8  cfa_ymoire_enable;
	__u8  cfa_ymoire_dc_w;
	__u8  cfa_ymoire_lpf_w;
	struct cvi_isp_demosiac_tun_cfg demosiac_cfg;
	struct cvi_isp_demosiac_tun_1_cfg demosiac_1_cfg;
	struct cvi_isp_demosiac_tun_2_cfg demosiac_2_cfg;
};

struct cvi_vip_isp_lsc_config {
	__u8  update;
	__u8  enable;
	__u16 strength;
	__u8  debug;
	__u8  gain_base;
	__u8  gain_3p9_0_4p8_1;
	__u8  renormalize_enable;
	__u8  gain_bicubic_0_bilinear_1;
	__u8  boundary_interpolation_mode;
	__u8  boundary_interpolation_lf_range;
	__u8  boundary_interpolation_up_range;
	__u8  boundary_interpolation_rt_range;
	__u8  boundary_interpolation_dn_range;
	__u16 blc_offset_r;
	__u16 blc_offset_gr;
	__u16 blc_offset_gb;
	__u16 blc_offset_b;
	__u32 intp_gain_max;
	__u32 intp_gain_min;
};

struct cvi_isp_bnr_tun_1_cfg {

};

struct cvi_isp_bnr_tun_2_cfg {

};

struct cvi_vip_isp_bnr_config {
	__u8  update;
	__u8  enable;
	__u8  out_sel;
	__u8  weight_lut[256];
	__u8  intensity_sel[8];
	__u8  weight_intra_0;
	__u8  weight_intra_1;
	__u8  weight_intra_2;
	__u8  weight_norm_1;
	__u8  weight_norm_2;
	__u16 k_smooth;
	__u16 k_texture;
	struct cvi_isp_bnr_tun_1_cfg bnr_1_cfg;
	struct cvi_isp_bnr_tun_2_cfg bnr_2_cfg;
};

struct cvi_vip_isp_clut_config {
	__u8  update;
	__u8  enable;
	__u8  is_update_partial;
	__u8  tbl_idx;
	__u16 r_lut[4913];
	__u16 g_lut[4913];
	__u16 b_lut[4913];
	__u16 update_length;
	__u32 lut[1024][2]; //0:addr, 1:value
};

struct cvi_isp_drc_tun_1_cfg {
};

struct cvi_isp_drc_tun_2_cfg {

};

struct cvi_isp_drc_tun_3_cfg {

};

struct cvi_vip_isp_drc_config {
	__u8  update;
	__u8  drc_enable;
	__u8  dark_enh_en;
	__u8  brit_enh_en;
	__u8  dbg_mode;
	__u8  dark_tone_wgt_refine_en;
	__u8  brit_tone_wgt_refine_en;
	__u16 global_lut[769];
	__u16 dark_lut[257];
	__u16 brit_lut[513];
	__u8  lmap_enable;
	__u8  lmap_w_bit;
	__u8  lmap_h_bit;
	__u16 lmap_thd_l;
	__u16 lmap_thd_h;
	__u8  lmap_y_mode;
	__u8  de_strth_dshft;
	__u16 de_strth_gain;
	__u8  be_strth_dshft;
	__u16 be_strth_gain;
	struct cvi_isp_drc_tun_1_cfg drc_1_cfg;
	struct cvi_isp_drc_tun_2_cfg drc_2_cfg;
	struct cvi_isp_drc_tun_3_cfg drc_3_cfg;
};

struct cvi_vip_isp_cnr_config {
	__u8  update;
	__u8  enable;
	__u8  strength_mode;
	__u8  diff_shift_val;
	__u8  diff_gain;
	__u8  ratio;
	__u8  fusion_intensity_weight;
	__u8  flag_neighbor_max_weight;
	__u8  weight_lut_inter[16];
	__u8  motion_enable;
	__u8  coring_motion_lut[16];
	__u8  motion_lut[16];
};

struct cvi_isp_tnr_tun_cfg {

};

struct cvi_isp_tnr_tun_5_cfg {

};

struct cvi_isp_tnr_tun_1_cfg {

};

struct cvi_isp_tnr_tun_2_cfg {

};

struct cvi_isp_tnr_tun_3_cfg {
};

struct cvi_isp_tnr_tun_4_cfg {
};

struct cvi_isp_tnr_tun_6_cfg {
};

struct cvi_isp_tnr_tun_7_cfg {
};

struct cvi_vip_isp_tnr_config {
	__u8  update;
	__u8  manr_enable;
	__u8  mh_wgt;
	__u8  lpf[3][3];
	__u8  map_gain;
	__u8  map_thd_l;
	__u8  map_thd_h;
	__u8  uv_rounding_type_sel;
	__u8  history_sel_0;
	__u8  history_sel_1;
	__u8  history_sel_3;
	__u16 tdnr_debug_sel;
	__s16 luma_adapt_lut_slope_2;
	__u8  med_enable;
	__u16 med_wgt;
	__u8  mtluma_mode;
	__u8  avg_mode_write;
	__u8  drop_mode_write;
	__u8  tdnr_pixel_lp;
	__u8  tdnr_comp_gain_enable;
	__u16 tdnr_ee_comp_gain;
	struct cvi_isp_tnr_tun_cfg      tnr_cfg;
	struct cvi_isp_tnr_tun_1_cfg    tnr_1_cfg;
	struct cvi_isp_tnr_tun_2_cfg    tnr_2_cfg;
	struct cvi_isp_tnr_tun_3_cfg    tnr_3_cfg;
	struct cvi_isp_tnr_tun_4_cfg    tnr_4_cfg;
	struct cvi_isp_tnr_tun_5_cfg    tnr_5_cfg;
	struct cvi_isp_tnr_tun_6_cfg    tnr_6_cfg;
	struct cvi_isp_tnr_tun_7_cfg    tnr_7_cfg;
};

struct cvi_isp_ee_tun_1_cfg {

};

struct cvi_isp_ee_tun_2_cfg {

};

struct cvi_isp_ee_tun_3_cfg {

};

struct cvi_isp_ee_tun_4_cfg {

};

struct cvi_vip_isp_ee_config {
	__u8  update;
	__u8  enable;
	__u8  dbg_mode;
	__u8  total_coring;
	__u8  total_motion_coring;
	__u8  total_gain;
	__u8  total_oshtthrd;
	__u8  total_ushtthrd;
	__u8  pre_proc_enable;
	__u8  lumaref_lpf_en;
	__u8  luma_coring_en;
	__u8  luma_adptctrl_en;
	__u8  delta_adptctrl_en;
	__u8  delta_adptctrl_shift;
	__u8  chromaref_lpf_en;
	__u8  chroma_adptctrl_en;
	__u8  mf_core_gain;
	__u8  hf_blend_wgt;
	__u8  mf_blend_wgt;
	__u8  soft_clamp_enable;
	__u8  upper_bound_left_diff;
	__u8  lower_bound_right_diff;
	__u8  luma_adptctrl_lut[33];
	__u8  delta_adptctrl_lut[33];
	__u8  chroma_adptctrl_lut[33];
	struct cvi_isp_ee_tun_1_cfg ee_1_cfg;
	struct cvi_isp_ee_tun_2_cfg ee_2_cfg;
	struct cvi_isp_ee_tun_3_cfg ee_3_cfg;
	struct cvi_isp_ee_tun_4_cfg ee_4_cfg;
};

struct cvi_isp_preyee_tun_1_cfg {

};

struct cvi_isp_preyee_tun_2_cfg {

};

struct cvi_isp_preyee_tun_3_cfg {

};

struct cvi_isp_preyee_tun_4_cfg {

};

struct cvi_isp_preyee_tun_5_cfg {

};

struct cvi_vip_isp_pre_ee_config {
	__u8  update;
	__u8  enable;
	__u8  dbg_mode;
	__u8  total_coring;
	__u8  total_motion_coring;
	__u8  total_gain;
	__u8  total_oshtthrd;
	__u8  total_ushtthrd;
	__u8  pre_proc_enable;
	__u8  lumaref_lpf_en;
	__u8  luma_coring_en;
	__u8  luma_adptctrl_en;
	__u8  delta_adptctrl_en;
	__u8  delta_adptctrl_shift;
	__u8  chromaref_lpf_en;
	__u8  chroma_adptctrl_en;
	__u8  mf_core_gain;
	__u8  hf_blend_wgt;
	__u8  mf_blend_wgt;
	__u8  soft_clamp_enable;
	__u8  upper_bound_left_diff;
	__u8  lower_bound_right_diff;
	__u8  luma_adptctrl_lut[33];
	__u8  delta_adptctrl_lut[33];
	__u8  chroma_adptctrl_lut[33];
	struct cvi_isp_preyee_tun_1_cfg pre_ee_1_cfg;
	struct cvi_isp_preyee_tun_2_cfg pre_ee_2_cfg;
	struct cvi_isp_preyee_tun_3_cfg pre_ee_3_cfg;
	struct cvi_isp_preyee_tun_4_cfg pre_ee_4_cfg;
	struct cvi_isp_preyee_tun_5_cfg pre_ee_5_cfg;
};

struct cvi_isp_fswdr_tun_cfg {

};

struct cvi_isp_fswdr_tun_2_cfg {

};

struct cvi_isp_fswdr_tun_3_cfg {
};

struct cvi_vip_isp_fswdr_config {
	__u8  update;
	__u8  enable;
	__u8  mc_enable;
	__u8  dc_mode;
	__u8  luma_mode;
	__u8  lmap_guide_dc_mode;
	__u8  lmap_guide_luma_mode;
	__u32 s_max;
	__u8  fusion_type;
	__u16 fusion_lwgt;
	__u8  motion_ls_mode;
	__u8  mmap_mrg_mode;
	__u8  mmap_1_enable;
	__u8  motion_ls_sel;
	__u8  mmap_mrg_alph;
	__u8  history_sel_2;
	__u16 mmap_v_thd_l;
	__u16 mmap_v_thd_h;
	__u16 mmap_v_wgt_min;
	__u16 mmap_v_wgt_max;
	__s32 mmap_v_wgt_slp;
	__u8  le_in_sel;
	__u8  se_in_sel;
	struct cvi_isp_fswdr_tun_cfg fswdr_cfg;
	struct cvi_isp_fswdr_tun_2_cfg fswdr_2_cfg;
	struct cvi_isp_fswdr_tun_3_cfg fswdr_3_cfg;
};

struct cvi_vip_isp_fswdr_report {
	uint32_t cal_pix_num;
	int32_t diff_sum_r;
	int32_t diff_sum_g;
	int32_t diff_sum_b;
};

struct cvi_vip_isp_ldci_tun_1_cfg {

};

struct cvi_vip_isp_ldci_tun_2_cfg {

};

struct cvi_vip_isp_ldci_tun_3_cfg {
};

struct cvi_vip_isp_ldci_tun_4_cfg {
};

struct cvi_vip_isp_ldci_tun_5_cfg {

};

struct cvi_vip_isp_ldci_config {
	__u8  update;
	__u8  enable;
	__u8  stats_enable;
	__u8  map_enable;
	__u8  uv_gain_enable;
	__u8  first_frame_enable;
	__u8  image_size_div_by_16x12;
	__u16 strength;
	struct cvi_vip_isp_ldci_tun_1_cfg ldci_1_cfg;
	struct cvi_vip_isp_ldci_tun_2_cfg ldci_2_cfg;
	struct cvi_vip_isp_ldci_tun_3_cfg ldci_3_cfg;
	struct cvi_vip_isp_ldci_tun_4_cfg ldci_4_cfg;
	struct cvi_vip_isp_ldci_tun_5_cfg ldci_5_cfg;
};

struct cvi_vip_isp_ycur_config {
	__u8  update;
	__u8  enable;
	__u8  lut[64];
	__u16 lut_256;
};

struct cvi_vip_isp_dci_config {
	__u8  update;
	__u8  enable;
	__u8  map_enable;
	__u8  demo_mode;
	__u16 map_lut[256];
	__u8  per1sample_enable;
	__u8  hist_enable;
};

struct cvi_vip_isp_dhz_luma_tun_cfg {
	union REG_ISP_DEHAZE_9  LUMA_00;
	union REG_ISP_DEHAZE_10 LUMA_04;
	union REG_ISP_DEHAZE_11 LUMA_08;
	union REG_ISP_DEHAZE_12 LUMA_12;
};

struct cvi_vip_isp_dhz_skin_tun_cfg {
	union REG_ISP_DEHAZE_17 SKIN_00;
	union REG_ISP_DEHAZE_18 SKIN_04;
	union REG_ISP_DEHAZE_19 SKIN_08;
	union REG_ISP_DEHAZE_20 SKIN_12;
};

struct cvi_vip_isp_dhz_tmap_tun_cfg {
	union REG_ISP_DEHAZE_TMAP_00 TMAP_00;
	union REG_ISP_DEHAZE_TMAP_01 TMAP_01;
	union REG_ISP_DEHAZE_TMAP_02 TMAP_02;
	union REG_ISP_DEHAZE_TMAP_03 TMAP_03;
	union REG_ISP_DEHAZE_TMAP_04 TMAP_04;
	union REG_ISP_DEHAZE_TMAP_05 TMAP_05;
	union REG_ISP_DEHAZE_TMAP_06 TMAP_06;
	union REG_ISP_DEHAZE_TMAP_07 TMAP_07;
	union REG_ISP_DEHAZE_TMAP_08 TMAP_08;
	union REG_ISP_DEHAZE_TMAP_09 TMAP_09;
	union REG_ISP_DEHAZE_TMAP_10 TMAP_10;
	union REG_ISP_DEHAZE_TMAP_11 TMAP_11;
	union REG_ISP_DEHAZE_TMAP_12 TMAP_12;
	union REG_ISP_DEHAZE_TMAP_13 TMAP_13;
	union REG_ISP_DEHAZE_TMAP_14 TMAP_14;
	union REG_ISP_DEHAZE_TMAP_15 TMAP_15;
	union REG_ISP_DEHAZE_TMAP_16 TMAP_16;
	union REG_ISP_DEHAZE_TMAP_17 TMAP_17;
	union REG_ISP_DEHAZE_TMAP_18 TMAP_18;
	union REG_ISP_DEHAZE_TMAP_19 TMAP_19;
	union REG_ISP_DEHAZE_TMAP_20 TMAP_20;
	union REG_ISP_DEHAZE_TMAP_21 TMAP_21;
	union REG_ISP_DEHAZE_TMAP_22 TMAP_22;
	union REG_ISP_DEHAZE_TMAP_23 TMAP_23;
	union REG_ISP_DEHAZE_TMAP_24 TMAP_24;
	union REG_ISP_DEHAZE_TMAP_25 TMAP_25;
	union REG_ISP_DEHAZE_TMAP_26 TMAP_26;
	union REG_ISP_DEHAZE_TMAP_27 TMAP_27;
	union REG_ISP_DEHAZE_TMAP_28 TMAP_28;
	union REG_ISP_DEHAZE_TMAP_29 TMAP_29;
	union REG_ISP_DEHAZE_TMAP_30 TMAP_30;
	union REG_ISP_DEHAZE_TMAP_31 TMAP_31;
	union REG_ISP_DEHAZE_TMAP_32 TMAP_32;
};

/* struct cvi_vip_isp_dhz_config
 * @param strength:  (0~127) dehaze strength
 * @param th_smooth: (0~1023) threshold for edge/smooth classification.
 */
struct cvi_vip_isp_dhz_config {
	__u8  update;
	__u8  enable;
	__u8  strength;
	__u16 cum_th;
	__u16 hist_th;
	__u16 tmap_min;
	__u16 tmap_max;
	__u16 th_smooth;
	__u8  luma_lut_enable;
	__u8  skin_lut_enable;
	__u8  a_luma_wgt;
	__u8  blend_wgt;
	__u8  tmap_scale;
	__u8  d_wgt;
	__u16 sw_dc_th;
	__u16 sw_aglobal_r;
	__u16 sw_aglobal_g;
	__u16 sw_aglobal_b;
	__u16 aglobal_max;
	__u16 aglobal_min;
	__u8  skin_cb;
	__u8  skin_cr;
	struct cvi_vip_isp_dhz_luma_tun_cfg luma_cfg;
	struct cvi_vip_isp_dhz_skin_tun_cfg skin_cfg;
	struct cvi_vip_isp_dhz_tmap_tun_cfg tmap_cfg;
};

struct cvi_vip_isp_csc_config {
	__u8  update;
	__u8  enable;
	__s16 coeff[9];
	__s16 offset[3];
};

struct cvi_isp_dpc_tun_cfg {

};

struct cvi_vip_isp_dpc_config {
	__u8  update;
	__u8  inst;
	__u8  enable;
	__u8  staticbpc_enable;
	__u8  dynamicbpc_enable;
	__u8  cluster_size;
	__u32 bp_tbl[2047];
	__u16 bp_cnt;
	struct cvi_isp_dpc_tun_cfg dpc_cfg;
};

struct cvi_isp_ae_tun_cfg {
	union REG_ISP_AE_HIST_AE_FACE_ENABLE_CTRL ae_face_enable;
	union REG_ISP_AE_HIST_AE_FACE0_STS_DIV  ae_face0_sts_div;
	union REG_ISP_AE_HIST_AE_FACE1_STS_DIV  ae_face1_sts_div;
	union REG_ISP_AE_HIST_AE_FACE2_STS_DIV  ae_face2_sts_div;
	union REG_ISP_AE_HIST_AE_FACE3_STS_DIV  ae_face3_sts_div;
	union REG_ISP_AE_HIST_STS_ENABLE        sts_enable;
	union REG_ISP_AE_HIST_AE_ALGO_ENABLE    ae_algo_enable;
	union REG_ISP_AE_HIST_AE_HIST_LOW       ae_hist_low;
	union REG_ISP_AE_HIST_AE_HIST_HIGH      ae_hist_high;
	union REG_ISP_AE_HIST_AE_TOP            ae_top;
	union REG_ISP_AE_HIST_AE_BOT            ae_bot;
	union REG_ISP_AE_HIST_AE_OVEREXP_THR    ae_overexp_thr;
	union REG_ISP_AE_HIST_AE_NUM_GAPLINE    ae_num_gapline;
};

struct cvi_isp_ae_2_tun_cfg {
	union REG_ISP_AE_HIST_AE_WGT_00         ae_wgt_00;
	union REG_ISP_AE_HIST_AE_WGT_01         ae_wgt_01;
	union REG_ISP_AE_HIST_AE_WGT_02         ae_wgt_02;
	union REG_ISP_AE_HIST_AE_WGT_03         ae_wgt_03;
	union REG_ISP_AE_HIST_AE_WGT_04         ae_wgt_04;
	union REG_ISP_AE_HIST_AE_WGT_05         ae_wgt_05;
	union REG_ISP_AE_HIST_AE_WGT_06         ae_wgt_06;
	union REG_ISP_AE_HIST_AE_WGT_07         ae_wgt_07;
	union REG_ISP_AE_HIST_AE_WGT_08         ae_wgt_08;
	union REG_ISP_AE_HIST_AE_WGT_09         ae_wgt_09;
	union REG_ISP_AE_HIST_AE_WGT_10         ae_wgt_10;
	union REG_ISP_AE_HIST_AE_WGT_11         ae_wgt_11;
	union REG_ISP_AE_HIST_AE_WGT_12         ae_wgt_12;
	union REG_ISP_AE_HIST_AE_WGT_13         ae_wgt_13;
	union REG_ISP_AE_HIST_AE_WGT_14         ae_wgt_14;
	union REG_ISP_AE_HIST_AE_WGT_15         ae_wgt_15;
	union REG_ISP_AE_HIST_AE_WGT_16         ae_wgt_16;
	union REG_ISP_AE_HIST_AE_WGT_17         ae_wgt_17;
	union REG_ISP_AE_HIST_AE_WGT_18         ae_wgt_18;
	union REG_ISP_AE_HIST_AE_WGT_19         ae_wgt_19;
	union REG_ISP_AE_HIST_AE_WGT_20         ae_wgt_20;
	union REG_ISP_AE_HIST_AE_WGT_21         ae_wgt_21;
	union REG_ISP_AE_HIST_AE_WGT_22         ae_wgt_22;
	union REG_ISP_AE_HIST_AE_WGT_23         ae_wgt_23;
	union REG_ISP_AE_HIST_AE_WGT_24         ae_wgt_24;
	union REG_ISP_AE_HIST_AE_WGT_25         ae_wgt_25;
	union REG_ISP_AE_HIST_AE_WGT_26         ae_wgt_26;
	union REG_ISP_AE_HIST_AE_WGT_27         ae_wgt_27;
	union REG_ISP_AE_HIST_AE_WGT_28         ae_wgt_28;
	union REG_ISP_AE_HIST_AE_WGT_29         ae_wgt_29;
	union REG_ISP_AE_HIST_AE_WGT_30         ae_wgt_30;
	union REG_ISP_AE_HIST_AE_WGT_31         ae_wgt_31;
};

struct cvi_vip_isp_ae_config {
	__u8  update;
	__u8  inst;
	__u8  ae_enable;
	__u16 ae_offsetx;
	__u16 ae_offsety;
	__u8  ae_numx;
	__u8  ae_numy;
	__u16 ae_width;
	__u16 ae_height;
	__u16 ae_sts_div;
	__u16 ae_face_offset_x[4];
	__u16 ae_face_offset_y[4];
	__u8  ae_face_size_minus1_x[4];
	__u8  ae_face_size_minus1_y[4];
	struct cvi_isp_ae_tun_cfg ae_cfg;
	struct cvi_isp_ae_2_tun_cfg ae_2_cfg;
};

struct cvi_vip_isp_af_config {
	__u8  update;
	__u8  inst;
	__u8  enable;
	__u8  dpc_enable;
	__u8  hlc_enable;
	__u8  square_enable;
	__u8  outshift;
	__u8  num_gapline;
	__u16 offsetx;
	__u16 offsety;
	__u16 block_width;
	__u16 block_height;
	__u8  block_numx;
	__u8  block_numy;
	__u8  h_low_pass_value_shift;
	__u32 h_corning_offset_0;
	__u32 h_corning_offset_1;
	__u16 v_corning_offset;
	__u16 high_luma_threshold;
	__u8  h_low_pass_coef[5];
	__u8  h_high_pass_coef_0[5];
	__u8  h_high_pass_coef_1[5];
	__u8  v_high_pass_coef[3];
	__u8  th_low;
	__u8  th_high;
	__u8  gain_low;
	__u8  gain_high;
	__u8  slop_low;
	__u8  slop_high;
};

struct cvi_vip_isp_edge_ext_config {
	__u8  update;
	__u8  enable;
	__u8  luma_mode;
	__u16 offset_x;
	__u16 offset_y;
};

struct cvi_vip_isp_gms_config {
	__u8  update;
	__u8  inst;
	__u8  enable;
	__u16 offset_x;
	__u16 offset_y;
	__u16 x_section_size;
	__u16 y_section_size;
	__u8  x_gap;
	__u8  y_gap;
};

struct cvi_vip_isp_mono_config {
	__u8  update;
	__u8  force_mono_enable;
};

#if 0
struct cvi_vip_isp_3dlut_config {
	__u8  update;
	__u8  enable;
	__u8  h_clamp_wrap_opt;
	__u16 h_lut[3276];
	__u16 s_lut[3276];
	__u16 v_lut[3276];
};

struct cvi_vip_isp_lscr_config {
	__u8  update;
	__u8  inst;
	__u8  enable;
	__u16 strength;
	__u16 strength_ir;
	__u16 norm;
	__u16 norm_ir;
	__u16 centerx;
	__u16 centery;
	__u16 gain_lut[32];
	__u16 gain_lut1[32];
	__u16 gain_lut2[32];
	__u16 gain_lut_ir[32];
};

struct cvi_vip_isp_awb_config {
	__u8  update;
	__u8  inst;
	__u8  enable;
	__u8  bayer_start;
	__u16 awb_offsetx;
	__u16 awb_offsety;
	__u16 awb_sub_win_w;
	__u16 awb_sub_win_h;
	__u8  awb_numx;
	__u8  awb_numy;
	__u8  corner_avg_en;
	__u8  corner_size;
	__u8  awb_sts_div;
	__u16 r_lower_bound;
	__u16 r_upper_bound;
	__u16 g_lower_bound;
	__u16 g_upper_bound;
	__u16 b_lower_bound;
	__u16 b_upper_bound;
};

struct cvi_vip_isp_hsv_config {
	__u8  update;
	__u8  enable;
	__u8  htune_enable;
	__u8  stune_enable;
	__u8  hsgain_enable;
	__u8  hvgain_enable;
	__u16 h_lut[769];
	__u16 s_lut[513];
	__u16 sgain_lut[769];
	__u16 vgain_lut[769];
};

struct cvi_vip_isp_preproc_config {
	__u8  update;
	__u8  inst;
	__u8  enable;
	__s16 r_ir_ratio[128];
	__s16 g_ir_ratio[128];
	__s16 b_ir_ratio[128];
	__u8  w_lut[128];
};

#endif

struct cvi_vip_isp_fe_tun_cfg {
	struct cvi_vip_isp_blc_config	blc_cfg[2];
	struct cvi_vip_isp_wbg_config	wbg_cfg[2];
};

struct cvi_vip_isp_post_tun_cfg {
	struct cvi_vip_isp_bnr_config		bnr_cfg;
	struct cvi_vip_isp_lsc_config		lsc_cfg;
	struct cvi_vip_isp_blc_config		blc_cfg[2];
	struct cvi_vip_isp_dpc_config		dpc_cfg[2];
	struct cvi_vip_isp_af_config		af_cfg;
	struct cvi_vip_isp_ae_config		ae_cfg[2];
	struct cvi_vip_isp_wbg_config		wbg_cfg[2];
	struct cvi_vip_isp_ccm_config		ccm_cfg[2];
	struct cvi_vip_isp_gamma_config		gamma_cfg;
	struct cvi_vip_isp_dhz_config		dhz_cfg;
	struct cvi_vip_isp_csc_config		csc_cfg;
	struct cvi_vip_isp_ldci_config		ldci_cfg;
	struct cvi_vip_isp_pre_ee_config	pre_ee_cfg;
	struct cvi_vip_isp_tnr_config		tnr_cfg;
	struct cvi_vip_isp_cnr_config		cnr_cfg;
	struct cvi_vip_isp_ee_config		ee_cfg;
	struct cvi_vip_isp_cacp_config		cacp_cfg;
	struct cvi_vip_isp_ca2_config		ca2_cfg;
	struct cvi_vip_isp_ycur_config		ycur_cfg;
	struct cvi_vip_isp_demosiac_config	demosiac_cfg;
	struct cvi_vip_isp_clut_config		clut_cfg;
	struct cvi_vip_isp_drc_config		drc_cfg;
	struct cvi_vip_isp_fswdr_config		fswdr_cfg;
	struct cvi_vip_isp_edge_ext_config	edge_ext_cfg;
	struct cvi_vip_isp_gms_config		gms_cfg;
	struct cvi_vip_isp_mono_config		mono_cfg;
};

struct cvi_vip_isp_fe_cfg {
	uint8_t tun_update[TUNING_NODE_NUM];
	uint8_t tun_idx;
	struct cvi_vip_isp_fe_tun_cfg tun_cfg[TUNING_NODE_NUM];
};

struct cvi_vip_isp_post_cfg {
	uint8_t tun_update[TUNING_NODE_NUM];
	uint8_t tun_idx;
	struct cvi_vip_isp_post_tun_cfg tun_cfg[TUNING_NODE_NUM];
};

struct isp_tuning_cfg {
	uint64_t  fe_addr[ISP_PRERAW_MAX];
	void	  *fe_vir[ISP_PRERAW_MAX];
#ifdef __arm__
	__u32	  fe_padding[ISP_PRERAW_MAX];
#endif
	uint64_t  post_addr[ISP_PRERAW_MAX];
	void      *post_vir[ISP_PRERAW_MAX];
#ifdef __arm__
	__u32	  post_padding[ISP_PRERAW_MAX];
#endif
};

#ifdef __cplusplus
}
#endif

#endif /* _U_CVI_VIP_TUN_CFG_H_ */
