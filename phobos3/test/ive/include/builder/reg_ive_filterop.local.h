// $Module: reg_ive_filterop $
// $RegisterBank Version: V 1.0.00 $
// $Author:  $
// $Date: Sun, 26 Sep 2021 04:20:44 PM $
//

#define  REG_1_DATA  0b00000000000000000000000000000000
										// reg_softrst:[0:0]=0b0   
										// reg_softrst_wdma_y:[1:1]=0b0   
										// reg_softrst_wdma_c:[2:2]=0b0   
										// reg_softrst_rdma_gradfg:[3:3]=0b0   
										// reg_softrst_op1:[4:4]=0b0   
										// reg_softrst_filter3ch:[5:5]=0b0   
										// reg_softrst_st:[6:6]=0b0   
										// reg_softrst_odma:[7:7]=0b0   
#define  REG_H04_DATA  0b00000000000000000000000000000000
										// reg_gradfg_bggrad_rdma_en:[0:0]=0b0   
#define  REG_2_DATA  0b00000000000000000000000000000001
										// reg_shdw_sel:[0:0]=0b1   
#define  REG_3_DATA  0b00000000000000000000000000000000
										// reg_ctrl_dmy1:[31:0]=0b00000000000000000000000000000000   
#define  REG_h10_DATA  0b00000000000000000000000000000010
										// reg_filterop_mode:[3:0]=0b0010   
#define  REG_h14_DATA  0b00000000000000000000001100000011
										// reg_filterop_op1_cmd:[3:0]=0b0011   
										// reg_filterop_sw_ovw_op:[4:4]=0b0   
										// reg_filterop_3ch_en:[8:8]=0b1   
										// reg_op_y_wdma_en:[9:9]=0b1   
										// reg_op_c_wdma_en:[10:10]=0b0   
										// reg_op_y_wdma_w1b_en:[11:11]=0b0   
										// reg_op_c_wdma_w1b_en:[12:12]=0b0   
#define  REG_4_DATA  0b00000000000000000000000000000000
										// reg_filterop_h_coef00:[7:0]=0b00000000   
										// reg_filterop_h_coef01:[15:8]=0b00000000   
										// reg_filterop_h_coef02:[23:16]=0b00000000   
										// reg_filterop_h_coef03:[31:24]=0b00000000   
#define  REG_5_DATA  0b00000000000000000000000000000000
										// reg_filterop_h_coef04:[7:0]=0b00000000   
										// reg_filterop_h_coef10:[15:8]=0b00000000   
										// reg_filterop_h_coef11:[23:16]=0b00000000   
										// reg_filterop_h_coef12:[31:24]=0b00000000   
#define  REG_6_DATA  0b00000000000000000000000000000000
										// reg_filterop_h_coef13:[7:0]=0b00000000   
										// reg_filterop_h_coef14:[15:8]=0b00000000   
										// reg_filterop_h_coef20:[23:16]=0b00000000   
										// reg_filterop_h_coef21:[31:24]=0b00000000   
#define  REG_7_DATA  0b00000000000000000000000000000000
										// reg_filterop_h_coef22:[7:0]=0b00000000   
										// reg_filterop_h_coef23:[15:8]=0b00000000   
										// reg_filterop_h_coef24:[23:16]=0b00000000   
										// reg_filterop_h_coef30:[31:24]=0b00000000   
#define  REG_8_DATA  0b00000000000000000000000000000000
										// reg_filterop_h_coef31:[7:0]=0b00000000   
										// reg_filterop_h_coef32:[15:8]=0b00000000   
										// reg_filterop_h_coef33:[23:16]=0b00000000   
										// reg_filterop_h_coef34:[31:24]=0b00000000   
#define  REG_9_DATA  0b00000000000000000000000000000000
										// reg_filterop_h_coef40:[7:0]=0b00000000   
										// reg_filterop_h_coef41:[15:8]=0b00000000   
										// reg_filterop_h_coef42:[23:16]=0b00000000   
										// reg_filterop_h_coef43:[31:24]=0b00000000   
#define  REG_10_DATA  0b00000000000000000000000000000000
										// reg_filterop_h_coef44:[7:0]=0b00000000   
										// reg_filterop_h_norm:[11:8]=0b0000   
#define  REG_11_DATA  0b00000000000000000000000000000000
										// reg_filterop_v_coef00:[7:0]=0b00000000   
										// reg_filterop_v_coef01:[15:8]=0b00000000   
										// reg_filterop_v_coef02:[23:16]=0b00000000   
										// reg_filterop_v_coef03:[31:24]=0b00000000   
#define  REG_12_DATA  0b00000000000000000000000000000000
										// reg_filterop_v_coef04:[7:0]=0b00000000   
										// reg_filterop_v_coef10:[15:8]=0b00000000   
										// reg_filterop_v_coef11:[23:16]=0b00000000   
										// reg_filterop_v_coef12:[31:24]=0b00000000   
#define  REG_13_DATA  0b00000000000000000000000000000000
										// reg_filterop_v_coef13:[7:0]=0b00000000   
										// reg_filterop_v_coef14:[15:8]=0b00000000   
										// reg_filterop_v_coef20:[23:16]=0b00000000   
										// reg_filterop_v_coef21:[31:24]=0b00000000   
#define  REG_14_DATA  0b00000000000000000000000000000000
										// reg_filterop_v_coef22:[7:0]=0b00000000   
										// reg_filterop_v_coef23:[15:8]=0b00000000   
										// reg_filterop_v_coef24:[23:16]=0b00000000   
										// reg_filterop_v_coef30:[31:24]=0b00000000   
#define  REG_15_DATA  0b00000000000000000000000000000000
										// reg_filterop_v_coef31:[7:0]=0b00000000   
										// reg_filterop_v_coef32:[15:8]=0b00000000   
										// reg_filterop_v_coef33:[23:16]=0b00000000   
										// reg_filterop_v_coef34:[31:24]=0b00000000   
#define  REG_16_DATA  0b00000000000000000000000000000000
										// reg_filterop_v_coef40:[7:0]=0b00000000   
										// reg_filterop_v_coef41:[15:8]=0b00000000   
										// reg_filterop_v_coef42:[23:16]=0b00000000   
										// reg_filterop_v_coef43:[31:24]=0b00000000   
#define  REG_17_DATA  0b00000000000000000000000000000000
										// reg_filterop_v_coef44:[7:0]=0b00000000   
										// reg_filterop_v_norm:[11:8]=0b0000   
#define  REG_18_DATA  0b00000000000000000000000000010001
										// reg_filterop_mode_trans:[0:0]=0b1   
										// reg_filterop_order_enmode:[6:4]=0b001   
										// reg_filterop_mag_thr:[31:16]=0b0000000000000000   
#define  REG_19_DATA  0b00000000000000000000000000000001
										// reg_filterop_bernsen_win5x5_en:[0:0]=0b1   
										// reg_filterop_bernsen_mode:[1:1]=0b0   
										// reg_filterop_bernsen_thr:[15:8]=0b00000000   
#define  REG_20_DATA  0b00000000000000000000000000000000
										// reg_filterop_lbp_u8bit_thr:[7:0]=0b00000000   
										// reg_filterop_lbp_s8bit_thr:[15:8]=0b00000000   
										// reg_filterop_lbp_enmode:[16:16]=0b0   
#define  REG_21_DATA  0b00000000000000000000000000000000
										// reg_filterop_op2_erodila_coef00:[7:0]=0b00000000   
										// reg_filterop_op2_erodila_coef01:[15:8]=0b00000000   
										// reg_filterop_op2_erodila_coef02:[23:16]=0b00000000   
										// reg_filterop_op2_erodila_coef03:[31:24]=0b00000000   
#define  REG_22_DATA  0b00000000000000000000000000000000
										// reg_filterop_op2_erodila_coef04:[7:0]=0b00000000   
										// reg_filterop_op2_erodila_coef10:[15:8]=0b00000000   
										// reg_filterop_op2_erodila_coef11:[23:16]=0b00000000   
										// reg_filterop_op2_erodila_coef12:[31:24]=0b00000000   
#define  REG_23_DATA  0b00000000000000000000000000000000
										// reg_filterop_op2_erodila_coef13:[7:0]=0b00000000   
										// reg_filterop_op2_erodila_coef14:[15:8]=0b00000000   
										// reg_filterop_op2_erodila_coef20:[23:16]=0b00000000   
										// reg_filterop_op2_erodila_coef21:[31:24]=0b00000000   
#define  REG_24_DATA  0b00000000000000000000000000000000
										// reg_filterop_op2_erodila_coef22:[7:0]=0b00000000   
										// reg_filterop_op2_erodila_coef23:[15:8]=0b00000000   
										// reg_filterop_op2_erodila_coef24:[23:16]=0b00000000   
										// reg_filterop_op2_erodila_coef30:[31:24]=0b00000000   
#define  REG_25_DATA  0b00000000000000000000000000000000
										// reg_filterop_op2_erodila_coef31:[7:0]=0b00000000   
										// reg_filterop_op2_erodila_coef32:[15:8]=0b00000000   
										// reg_filterop_op2_erodila_coef33:[23:16]=0b00000000   
										// reg_filterop_op2_erodila_coef34:[31:24]=0b00000000   
#define  REG_26_DATA  0b00000000000000000000000000000000
										// reg_filterop_op2_erodila_coef40:[7:0]=0b00000000   
										// reg_filterop_op2_erodila_coef41:[15:8]=0b00000000   
										// reg_filterop_op2_erodila_coef42:[23:16]=0b00000000   
										// reg_filterop_op2_erodila_coef43:[31:24]=0b00000000   
#define  REG_27_DATA  0b00000000000000000000000000000000
										// reg_filterop_op2_erodila_coef44:[7:0]=0b00000000   
#define  REG_28_DATA  0b00000000000000000000000000000000
										// reg_filterop_op2_erodila_en:[0:0]=0b0   
#define  REG_33_DATA  0b00000011111010000000000000001100
										// reg_filterop_op2_gradfg_en:[0:0]=0b0   
										// reg_filterop_op2_gradfg_softrst:[1:1]=0b0   
										// reg_filterop_op2_gradfg_enmode:[2:2]=0b1   
										// reg_filterop_op2_gradfg_edwdark:[3:3]=0b1   
										// reg_filterop_op2_gradfg_edwfactor:[31:16]=0b0000001111101000   
#define  REG_34_DATA  0b00000001000000100000010001010000
										// reg_filterop_op2_gradfg_crlcoefthr:[7:0]=0b01010000   
										// reg_filterop_op2_gradfg_magcrlthr:[15:8]=0b00000100   
										// reg_filterop_op2_gradfg_minmagdiff:[23:16]=0b00000010   
										// reg_filterop_op2_gradfg_noiseval:[31:24]=0b00000001   
#define  REG_110_DATA  0b00000000000000000000000000000000
										// reg_filterop_map_enmode:[1:0]=0b00   
										// reg_filterop_norm_out_ctrl:[5:4]=0b00   
										// reg_filterop_magang_out_ctrl:[8:8]=0b0   
#define  ODMA_REG_00_DATA  0b00000000000000000000000000000000
										// reg_dma_blen:[0:0]=0b0   
										// reg_dma_en:[1:1]=0b0   
										// reg_fmt_sel:[11:8]=0b0000   
										// reg_sc_odma_hflip:[16:16]=0b0   
										// reg_sc_odma_vflip:[17:17]=0b0   
										// reg_sc_422_avg:[20:20]=0b0   
										// reg_sc_420_avg:[21:21]=0b0   
										// reg_c_round_mode:[22:22]=0b0   
										// reg_bf16_en:[23:23]=0b0   
#define  ODMA_REG_01_DATA  0b00000000000000000000000000000000
										// reg_dma_y_base_low_part:[31:0]=0b00000000000000000000000000000000   
#define  ODMA_REG_02_DATA  0b00000000000000000000000000000000
										// reg_dma_y_base_high_part:[7:0]=0b00000000   
#define  ODMA_REG_03_DATA  0b00000000000000000000000000000000
										// reg_dma_u_base_low_part:[31:0]=0b00000000000000000000000000000000   
#define  ODMA_REG_04_DATA  0b00000000000000000000000000000000
										// reg_dma_u_base_high_part:[7:0]=0b00000000   
#define  ODMA_REG_05_DATA  0b00000000000000000000000000000000
										// reg_dma_v_base_low_part:[31:0]=0b00000000000000000000000000000000   
#define  ODMA_REG_06_DATA  0b00000000000000000000000000000000
										// reg_dma_v_base_high_part:[7:0]=0b00000000   
#define  ODMA_REG_07_DATA  0b00000000000000000000000000000000
										// reg_dma_y_pitch:[23:0]=0b000000000000000000000000   
#define  ODMA_REG_08_DATA  0b00000000000000000000000000000000
										// reg_dma_c_pitch:[23:0]=0b000000000000000000000000   
#define  ODMA_REG_09_DATA  0b00000000000000000000000000000000
										// reg_dma_x_str:[11:0]=0b000000000000   
#define  ODMA_REG_10_DATA  0b00000000000000000000000000000000
										// reg_dma_y_str:[11:0]=0b000000000000   
#define  ODMA_REG_11_DATA  0b00000000000000000000000000000000
										// reg_dma_wd:[11:0]=0b000000000000   
#define  ODMA_REG_12_DATA  0b00000000000000000000000000000000
										// reg_dma_ht:[11:0]=0b000000000000   
#define  ODMA_REG_13_DATA  0b00000000000000000000000000000000
										// reg_dma_debug:[31:0]=0b00000000000000000000000000000000   
#define  ODMA_REG_14_DATA  0b00000000000000000000000000000000
										// reg_dma_int_line_target:[11:0]=0b000000000000   
										// reg_dma_int_line_target_sel:[17:16]=0b00   
#define  ODMA_REG_15_DATA  0b00000000000000000000000000000000
										// reg_dma_int_cycle_line_target:[10:0]=0b00000000000   
										// reg_dma_int_cycle_line_target_sel:[17:16]=0b00   
#define  ODMA_REG_16_DATA  0b00000000000000000000000000000000
										// reg_dma_latch_line_cnt:[0:0]=0b0   
										// reg_dma_latched_line_cnt:[19:8]=0b000000000000   
#define  ODMA_REG_17_DATA  0b00000000000000000000000000000000
										// axi_active:[0:0]=0b0   
										// axi_y_active:[1:1]=0b0   
										// axi_u_active:[2:2]=0b0   
										// axi_v_active:[3:3]=0b0   
										// y_buf_full:[4:4]=0b0   
										// y_buf_empty:[5:5]=0b0   
										// u_buf_full:[6:6]=0b0   
										// u_buf_empty:[7:7]=0b0   
										// v_buf_full:[8:8]=0b0   
										// v_buf_empty:[9:9]=0b0   
										// line_target_hit:[10:10]=0b0   
										// cycle_line_target_hit:[11:11]=0b0   
										// axi_cmd_cs:[15:12]=0b0000   
										// y_line_cnt:[27:16]=0b000000000000   
#define  REG_CANNY_0_DATA  0b00000000000000000000000000000000
										// reg_canny_lowthr:[15:0]=0b0000000000000000   
										// reg_canny_hithr:[31:16]=0b0000000000000000   
#define  REG_CANNY_1_DATA  0b00000000000000000000000000000000
										// reg_canny_en:[0:0]=0b0   
										// reg_canny_strong_point_cnt:[25:16]=0b0000000000   
										// reg_canny_strong_point_cnt_en:[26:26]=0b0   
										// reg_canny_non_or_weak:[27:27]=0b0   
#define  REG_CANNY_2_DATA  0b00000000000000000000000000000000
										// reg_canny_eof:[31:0]=0b00000000000000000000000000000000   
#define  REG_ST_CANDI_0_DATA  0b00000000000000000000000000000000
										// reg_st_candi_corner_bypass:[0:0]=0b0   
										// reg_st_candi_corner_switch_src:[1:1]=0b0   
#define  REG_ST_EIGVAL_0_DATA  0b00000000000000000000000000000000
										// reg_st_eigval_max_eigval:[15:0]=0b0000000000000000   
#define  REG_h190_DATA  0b00000000000000000000000000000000
										// reg_filterop_op2_csc_tab_sw_0:[11:0]=0b000000000000   
										// reg_filterop_op2_csc_tab_sw_1:[30:16]=0b000000000000000   
#define  REG_h194_DATA  0b00000000000000000000000000000000
										// reg_filterop_op2_csc_tab_sw_update:[0:0]=0b0   
										// reg_filterop_op2_csc_coeff_sw_update:[16:16]=0b0   
#define  REG_CSC_COEFF_0_DATA  0b00000000000000000000000000000000
										// reg_filterop_op2_csc_coeff_sw_00:[18:0]=0b0000000000000000000   
#define  REG_CSC_COEFF_1_DATA  0b00000000000000000000000000000000
										// reg_filterop_op2_csc_coeff_sw_01:[18:0]=0b0000000000000000000   
#define  REG_CSC_COEFF_2_DATA  0b00000000000000000000000000000000
										// reg_filterop_op2_csc_coeff_sw_02:[18:0]=0b0000000000000000000   
#define  REG_CSC_COEFF_3_DATA  0b00000000000000000000000000000000
										// reg_filterop_op2_csc_coeff_sw_03:[18:0]=0b0000000000000000000   
#define  REG_CSC_COEFF_4_DATA  0b00000000000000000000000000000000
										// reg_filterop_op2_csc_coeff_sw_04:[18:0]=0b0000000000000000000   
#define  REG_CSC_COEFF_5_DATA  0b00000000000000000000000000000000
										// reg_filterop_op2_csc_coeff_sw_05:[18:0]=0b0000000000000000000   
#define  REG_CSC_COEFF_6_DATA  0b00000000000000000000000000000000
										// reg_filterop_op2_csc_coeff_sw_06:[18:0]=0b0000000000000000000   
#define  REG_CSC_COEFF_7_DATA  0b00000000000000000000000000000000
										// reg_filterop_op2_csc_coeff_sw_07:[18:0]=0b0000000000000000000   
#define  REG_CSC_COEFF_8_DATA  0b00000000000000000000000000000000
										// reg_filterop_op2_csc_coeff_sw_08:[18:0]=0b0000000000000000000   
#define  REG_CSC_COEFF_9_DATA  0b00000000000000000000000000000000
										// reg_filterop_op2_csc_coeff_sw_09:[18:0]=0b0000000000000000000   
#define  REG_CSC_COEFF_A_DATA  0b00000000000000000000000000000000
										// reg_filterop_op2_csc_coeff_sw_10:[18:0]=0b0000000000000000000   
#define  REG_CSC_COEFF_B_DATA  0b00000000000000000000000000000000
										// reg_filterop_op2_csc_coeff_sw_11:[18:0]=0b0000000000000000000   
#define  REG_h1c8_DATA  0b00000000000000000000000000000000
										// reg_filterop_op2_csc_enmode:[3:0]=0b0000   
