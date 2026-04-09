// $Module: reg_ive_top $
// $RegisterBank Version: V 1.0.00 $
// $Author:  $
// $Date: Sun, 26 Sep 2021 04:20:45 PM $
//

#define  REG_0_DATA  0b00000100001101110000011101111111
										// reg_img_in_widthm1:[12:0]=0b0011101111111   
										// reg_img_in_heightm1:[28:16]=0b0010000110111   
#define  REG_1_DATA  0b00000000000000000000000000000010
										// reg_softrst:[0:0]=0b0   
										// reg_shdw_sel:[1:1]=0b1   
										// reg_fmt_vld_fg:[4:4]=0b0   
										// reg_fmt_vld_ccl:[5:5]=0b0   
										// reg_fmt_vld_dmaf:[6:6]=0b0   
										// reg_fmt_vld_lk:[7:7]=0b0   
										// reg_cmdq_tsk_trig:[11:8]=0b0000   
										// reg_cmdq_tsk_sel:[12:12]=0b0   
										// reg_cmdq_tsk_en:[13:13]=0b0   
#define  REG_2_DATA  0b00000100001101110000011101111111
										// reg_img_widthm1:[12:0]=0b0011101111111   
										// reg_img_heightm1:[28:16]=0b0010000110111   
#define  REG_3_DATA  0b00000000000000000000000000001001
										// reg_imgmux_img0_sel:[0:0]=0b1   
										// reg_mapmux_rdma_sel:[1:1]=0b0   
										// reg_ive_rdma_img1_en:[2:2]=0b0   
										// reg_ive_rdma_img1_mod_u8:[3:3]=0b1   
										// reg_ive_rdma_eigval_en:[4:4]=0b0   
#define  REG_h10_DATA  0b00000000000001111111111111111111
										// reg_img_in_top_enable:[0:0]=0b1   
										// reg_resize_top_enable:[1:1]=0b1   
										// reg_gmm_top_enable:[2:2]=0b1   
										// reg_csc_top_enable:[3:3]=0b1   
										// reg_rdma_img1_top_enable:[4:4]=0b1   
										// reg_bgm_top_enable:[5:5]=0b1   
										// reg_bgu_top_enable:[6:6]=0b1   
										// reg_r2y4_top_enable:[7:7]=0b1   
										// reg_map_top_enable:[8:8]=0b1   
										// reg_rdma_eigval_top_enable:[9:9]=0b1   
										// reg_thresh_top_enable:[10:10]=0b1   
										// reg_hist_top_enable:[11:11]=0b1   
										// reg_intg_top_enable:[12:12]=0b1   
										// reg_ncc_top_enable:[13:13]=0b1   
										// reg_sad_top_enable:[14:14]=0b1   
										// reg_filterop_top_enable:[15:15]=0b1   
										// reg_dmaf_top_enable:[16:16]=0b1   
										// reg_ccl_top_enable:[17:17]=0b1   
										// reg_lk_top_enable:[18:18]=0b1   
#define  REG_11_DATA  0b00000000000000000000000000000000
										// reg_csc_tab_sw_0:[11:0]=0b000000000000   
										// reg_csc_tab_sw_1:[30:16]=0b000000000000000   
#define  REG_12_DATA  0b00000000000000000000000000000000
										// reg_csc_tab_sw_update:[0:0]=0b0   
										// reg_csc_coeff_sw_update:[16:16]=0b0   
#define  REG_CSC_COEFF_0_DATA  0b00000000000000000000000000000000
										// reg_csc_coeff_sw_00:[18:0]=0b0000000000000000000   
#define  REG_CSC_COEFF_1_DATA  0b00000000000000000000000000000000
										// reg_csc_coeff_sw_01:[18:0]=0b0000000000000000000   
#define  REG_CSC_COEFF_2_DATA  0b00000000000000000000000000000000
										// reg_csc_coeff_sw_02:[18:0]=0b0000000000000000000   
#define  REG_CSC_COEFF_3_DATA  0b00000000000000000000000000000000
										// reg_csc_coeff_sw_03:[18:0]=0b0000000000000000000   
#define  REG_CSC_COEFF_4_DATA  0b00000000000000000000000000000000
										// reg_csc_coeff_sw_04:[18:0]=0b0000000000000000000   
#define  REG_CSC_COEFF_5_DATA  0b00000000000000000000000000000000
										// reg_csc_coeff_sw_05:[18:0]=0b0000000000000000000   
#define  REG_CSC_COEFF_6_DATA  0b00000000000000000000000000000000
										// reg_csc_coeff_sw_06:[18:0]=0b0000000000000000000   
#define  REG_CSC_COEFF_7_DATA  0b00000000000000000000000000000000
										// reg_csc_coeff_sw_07:[18:0]=0b0000000000000000000   
#define  REG_CSC_COEFF_8_DATA  0b00000000000000000000000000000000
										// reg_csc_coeff_sw_08:[18:0]=0b0000000000000000000   
#define  REG_CSC_COEFF_9_DATA  0b00000000000000000000000000000000
										// reg_csc_coeff_sw_09:[18:0]=0b0000000000000000000   
#define  REG_CSC_COEFF_A_DATA  0b00000000000000000000000000000000
										// reg_csc_coeff_sw_10:[18:0]=0b0000000000000000000   
#define  REG_CSC_COEFF_B_DATA  0b00000000000000000000000000000000
										// reg_csc_coeff_sw_11:[18:0]=0b0000000000000000000   
#define  REG_14_DATA  0b00000000000000000000000000000000
										// reg_csc_enmode:[3:0]=0b0000   
#define  REG_15_DATA  0b00000000000000000000000000000000
										// reg_lbp_u8bit_thr:[7:0]=0b00000000   
										// reg_lbp_s8bit_thr:[15:8]=0b00000000   
										// reg_lbp_enmode:[16:16]=0b0   
#define  REG_20_DATA  0b00000000000000000000000000000000
										// reg_frame2op_op_mode:[2:0]=0b000   
										// reg_frame2op_sub_mode:[3:3]=0b0   
										// reg_frame2op_sub_change_order:[4:4]=0b0   
										// reg_frame2op_add_mode_rounding:[5:5]=0b0   
										// reg_frame2op_add_mode_clipping:[6:6]=0b0   
										// reg_frame2op_sub_switch_src:[7:7]=0b0   
#define  REG_21_DATA  0b00000000000000000000000000000000
										// reg_fram2op_x_u0q16:[15:0]=0b0000000000000000   
										// reg_fram2op_y_u0q16:[31:16]=0b0000000000000000   
#define  REG_h80_DATA  0b00000000000000000000000000000000
										// reg_frame2op_fg_op_mode:[2:0]=0b000   
										// reg_frame2op_fg_sub_mode:[3:3]=0b0   
										// reg_frame2op_fg_sub_change_order:[4:4]=0b0   
										// reg_frame2op_fg_add_mode_rounding:[5:5]=0b0   
										// reg_frame2op_fg_add_mode_clipping:[6:6]=0b0   
										// reg_frame2op_fg_sub_switch_src:[7:7]=0b0   
#define  REG_84_DATA  0b00000000000000000000000000000000
										// reg_fram2op_fg_x_u0q16:[15:0]=0b0000000000000000   
										// reg_fram2op_fg_y_u0q16:[31:16]=0b0000000000000000   
#define  REG_90_DATA  0b00000000000000000000000000000000
										// reg_frame_done_img_in:[0:0]=0b0   
										// reg_frame_done_rdma_img1:[1:1]=0b0   
										// reg_frame_done_rdma_eigval:[2:2]=0b0   
										// reg_frame_done_resize:[3:3]=0b0   
										// reg_frame_done_gmm:[4:4]=0b0   
										// reg_frame_done_csc:[5:5]=0b0   
										// reg_frame_done_hist:[6:6]=0b0   
										// reg_frame_done_intg:[7:7]=0b0   
										// reg_frame_done_sad:[8:8]=0b0   
										// reg_frame_done_ncc:[9:9]=0b0   
										// reg_frame_done_bgm:[10:10]=0b0   
										// reg_frame_done_bgu:[11:11]=0b0   
										// reg_frame_done_r2y4:[12:12]=0b0   
										// reg_frame_done_frame2op_bg:[13:13]=0b0   
										// reg_frame_done_frame2op_fg:[14:14]=0b0   
										// reg_frame_done_map:[15:15]=0b0   
										// reg_frame_done_thresh16ro8:[16:16]=0b0   
										// reg_frame_done_thresh:[17:17]=0b0   
										// reg_frame_done_filterop_odma:[18:18]=0b0   
										// reg_frame_done_filterop_wdma_y:[19:19]=0b0   
										// reg_frame_done_filterop_wdma_c:[20:20]=0b0   
										// reg_frame_done_dmaf:[21:21]=0b0   
										// reg_frame_done_ccl:[22:22]=0b0   
										// reg_frame_done_lk:[23:23]=0b0   
#define  REG_94_DATA  0b00000000000000000000000000010000
										// reg_intr_en_hist:[0:0]=0b0   
										// reg_intr_en_intg:[1:1]=0b0   
										// reg_intr_en_sad:[2:2]=0b0   
										// reg_intr_en_ncc:[3:3]=0b0   
										// reg_intr_en_filterop_odma:[4:4]=0b1   
										// reg_intr_en_filterop_wdma_y:[5:5]=0b0   
										// reg_intr_en_filterop_wdma_c:[6:6]=0b0   
										// reg_intr_en_dmaf:[7:7]=0b0   
										// reg_intr_en_ccl:[8:8]=0b0   
										// reg_intr_en_lk:[9:9]=0b0   
#define  REG_98_DATA  0b00000000000000000000000000000000
										// reg_intr_status_hist:[0:0]=0b0   
										// reg_intr_status_intg:[1:1]=0b0   
										// reg_intr_status_sad:[2:2]=0b0   
										// reg_intr_status_ncc:[3:3]=0b0   
										// reg_intr_status_filterop_odma:[4:4]=0b0   
										// reg_intr_status_filterop_wdma_y:[5:5]=0b0   
										// reg_intr_status_filterop_wdma_c:[6:6]=0b0   
										// reg_intr_status_dmaf:[7:7]=0b0   
										// reg_intr_status_ccl:[8:8]=0b0   
										// reg_intr_status_lk:[9:9]=0b0   
#define  REG_RS_SRC_SIZE_DATA  0b00000100001101110000011101111111
										// reg_resize_src_wd:[15:0]=0b0000011101111111   
										// reg_resize_src_ht:[31:16]=0b0000010000110111   
#define  REG_RS_DST_SIZE_DATA  0b00000100001101110000011101111111
										// reg_resize_dst_wd:[15:0]=0b0000011101111111   
										// reg_resize_dst_ht:[31:16]=0b0000010000110111   
#define  REG_RS_H_SC_DATA  0b00000000000000000010000000000000
										// reg_resize_h_sc_fac:[17:0]=0b000010000000000000   
#define  REG_RS_V_SC_DATA  0b00000000000000000010000000000000
										// reg_resize_v_sc_fac:[17:0]=0b000010000000000000   
#define  REG_RS_PH_INI_DATA  0b00000000000000000000000000000000
										// reg_resize_h_ini_ph:[12:0]=0b0000000000000   
										// reg_resize_v_ini_ph:[28:16]=0b0000000000000   
#define  REG_RS_NOR_DATA  0b10000000000000001000000000000000
										// reg_resize_h_nor:[15:0]=0b1000000000000000   
										// reg_resize_v_nor:[31:16]=0b1000000000000000   
#define  REG_RS_CTRL_DATA  0b00000000000000000000000000000100
										// reg_resize_ip_en:[0:0]=0b0   
										// reg_resize_dbg_en:[1:1]=0b0   
										// reg_resize_area_fast:[2:2]=0b1   
										// reg_resize_blnr_mode:[3:3]=0b0   
#define  REG_RS_dBG_H1_DATA  0b00000000000000000000000000000000
										// reg_resize_sc_dbg_h1:[31:0]=0b00000000000000000000000000000000   
#define  REG_RS_dBG_H2_DATA  0b00000000000000000000000000000000
										// reg_resize_sc_dbg_h2:[31:0]=0b00000000000000000000000000000000   
#define  REG_RS_dBG_V1_DATA  0b00000000000000000000000000000000
										// reg_resize_sc_dbg_v1:[31:0]=0b00000000000000000000000000000000   
#define  REG_RS_dBG_V2_DATA  0b00000000000000000000000000000000
										// reg_resize_sc_dbg_v2:[31:0]=0b00000000000000000000000000000000   
#define  REG_h130_DATA  0b00000000000000000000000000000000
										// reg_thresh_top_mod:[1:0]=0b00   
										// reg_thresh_thresh_en:[4:4]=0b0   
										// reg_thresh_softrst:[8:8]=0b0   
#define  REG_h134_DATA  0b00000000000000000000000000000000
										// reg_thresh_16to8_mod:[2:0]=0b000   
										// reg_thresh_16to8_s8bias:[15:8]=0b00000000   
										// reg_thresh_16to8_u8Num_div_u16Den:[31:16]=0b0000000000000000   
#define  REG_h138_DATA  0b00000000000000000000000000000000
										// reg_thresh_st_16to8_en:[0:0]=0b0   
										// reg_thresh_st_16to8_u8Numerator:[15:8]=0b00000000   
										// reg_thresh_st_16to8_maxeigval:[31:16]=0b0000000000000000   
#define  REG_h13c_DATA  0b00000000000000000000000000000000
										// reg_thresh_s16_enmode:[1:0]=0b00   
										// reg_thresh_s16_u8bit_min:[15:8]=0b00000000   
										// reg_thresh_s16_u8bit_mid:[23:16]=0b00000000   
										// reg_thresh_s16_u8bit_max:[31:24]=0b00000000   
#define  REG_h140_DATA  0b00000000000000000000000000000000
										// reg_thresh_s16_bit_thr_l:[15:0]=0b0000000000000000   
										// reg_thresh_s16_bit_thr_h:[31:16]=0b0000000000000000   
#define  REG_h144_DATA  0b00000000000000000000000000000000
										// reg_thresh_u16_enmode:[0:0]=0b0   
										// reg_thresh_u16_u8bit_min:[15:8]=0b00000000   
										// reg_thresh_u16_u8bit_mid:[23:16]=0b00000000   
										// reg_thresh_u16_u8bit_max:[31:24]=0b00000000   
#define  REG_h148_DATA  0b00000000000000000000000000000000
										// reg_thresh_u16_bit_thr_l:[15:0]=0b0000000000000000   
										// reg_thresh_u16_bit_thr_h:[31:16]=0b0000000000000000   
#define  REG_h14c_DATA  0b00000000000000000000000000000000
										// reg_thresh_u8bit_thr_l:[7:0]=0b00000000   
										// reg_thresh_u8bit_thr_h:[15:8]=0b00000000   
										// reg_thresh_enmode:[18:16]=0b000   
#define  REG_h150_DATA  0b00000000000000000000000000000000
										// reg_thresh_u8bit_min:[7:0]=0b00000000   
										// reg_thresh_u8bit_mid:[15:8]=0b00000000   
										// reg_thresh_u8bit_max:[23:16]=0b00000000   
#define  REG_h160_DATA  0b00000000000000000000000000000000
										// reg_ncc_nemerator_l:[31:0]=0b00000000000000000000000000000000   
#define  REG_h164_DATA  0b00000000000000000000000000000000
										// reg_ncc_nemerator_m:[31:0]=0b00000000000000000000000000000000   
#define  REG_h168_DATA  0b00000000000000000000000000000000
										// reg_ncc_quadsum0_l:[31:0]=0b00000000000000000000000000000000   
#define  REG_h16C_DATA  0b00000000000000000000000000000000
										// reg_ncc_quadsum0_m:[31:0]=0b00000000000000000000000000000000   
#define  REG_h170_DATA  0b00000000000000000000000000000000
										// reg_ncc_quadsum1_l:[31:0]=0b00000000000000000000000000000000   
#define  REG_h174_DATA  0b00000000000000000000000000000000
										// reg_ncc_quadsum1_m:[31:0]=0b00000000000000000000000000000000   
#define  REG_R2Y4_11_DATA  0b00000000000000000000000000000000
										// reg_csc_r2y4_tab_sw_0:[11:0]=0b000000000000   
										// reg_csc_r2y4_tab_sw_1:[30:16]=0b000000000000000   
#define  REG_R2Y4_12_DATA  0b00000000000000000000000000000000
										// reg_csc_r2y4_tab_sw_update:[0:0]=0b0   
										// reg_csc_r2y4_coeff_sw_update:[16:16]=0b0   
#define  REG_R2Y4_COEFF_0_DATA  0b00000000000000000000000000000000
										// reg_csc_r2y4_coeff_sw_00:[18:0]=0b0000000000000000000   
#define  REG_R2Y4_COEFF_1_DATA  0b00000000000000000000000000000000
										// reg_csc_r2y4_coeff_sw_01:[18:0]=0b0000000000000000000   
#define  REG_R2Y4_COEFF_2_DATA  0b00000000000000000000000000000000
										// reg_csc_r2y4_coeff_sw_02:[18:0]=0b0000000000000000000   
#define  REG_R2Y4_COEFF_3_DATA  0b00000000000000000000000000000000
										// reg_csc_r2y4_coeff_sw_03:[18:0]=0b0000000000000000000   
#define  REG_R2Y4_COEFF_4_DATA  0b00000000000000000000000000000000
										// reg_csc_r2y4_coeff_sw_04:[18:0]=0b0000000000000000000   
#define  REG_R2Y4_COEFF_5_DATA  0b00000000000000000000000000000000
										// reg_csc_r2y4_coeff_sw_05:[18:0]=0b0000000000000000000   
#define  REG_R2Y4_COEFF_6_DATA  0b00000000000000000000000000000000
										// reg_csc_r2y4_coeff_sw_06:[18:0]=0b0000000000000000000   
#define  REG_R2Y4_COEFF_7_DATA  0b00000000000000000000000000000000
										// reg_csc_r2y4_coeff_sw_07:[18:0]=0b0000000000000000000   
#define  REG_R2Y4_COEFF_8_DATA  0b00000000000000000000000000000000
										// reg_csc_r2y4_coeff_sw_08:[18:0]=0b0000000000000000000   
#define  REG_R2Y4_COEFF_9_DATA  0b00000000000000000000000000000000
										// reg_csc_r2y4_coeff_sw_09:[18:0]=0b0000000000000000000   
#define  REG_R2Y4_COEFF_A_DATA  0b00000000000000000000000000000000
										// reg_csc_r2y4_coeff_sw_10:[18:0]=0b0000000000000000000   
#define  REG_R2Y4_COEFF_B_DATA  0b00000000000000000000000000000000
										// reg_csc_r2y4_coeff_sw_11:[18:0]=0b0000000000000000000   
#define  REG_R2Y4_14_DATA  0b00000000000000000000000000001000
										// reg_csc_r2y4_enmode:[3:0]=0b1000   
