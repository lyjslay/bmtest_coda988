// $Module: reg_sc_odma $
// $RegisterBank Version: V 1.0.00 $
// $Author:  $
// $Date: Sun, 26 Sep 2021 04:20:45 PM $
//

#define  ODMA_REG_00_DATA  0b00000000000000000000000000000000
										// reg_dma_blen:[0:0]=0b0   
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
#define  SB_REG_CTRL_DATA  0b00000000000000100000001100000000
										// reg_sb_mode:[1:0]=0b00   
										// reg_sb_size:[2:2]=0b0   
										// reg_sb_nb:[12:8]=0b00011   
										// reg_sb_full_nb:[20:16]=0b00010   
										// reg_sb_sw_wptr:[28:24]=0b00000   
										// reg_sb_set_str:[30:30]=0b0   
										// reg_sb_sw_clr:[31:31]=0b0   
#define  SB_REG_C_STAT_DATA  0b00000000000000000000000000000000
										// reg_u_sb_wptr_ro:[4:0]=0b00000   
										// reg_u_sb_full:[6:6]=0b0   
										// reg_u_sb_empty:[7:7]=0b0   
										// reg_u_sb_dptr_ro:[13:8]=0b000000   
										// reg_v_sb_wptr_ro:[20:16]=0b00000   
										// reg_v_sb_full:[22:22]=0b0   
										// reg_v_sb_empty:[23:23]=0b0   
										// reg_v_sb_dptr_ro:[29:24]=0b000000   
#define  SB_REG_Y_STAT_DATA  0b00000000000000000000000000000000
										// reg_y_sb_wptr_ro:[4:0]=0b00000   
										// reg_y_sb_full:[6:6]=0b0   
										// reg_y_sb_empty:[7:7]=0b0   
										// reg_y_sb_dptr_ro:[13:8]=0b000000   
										// reg_sb_full:[15:15]=0b0   
#define  SC_CSC_REG_00_DATA  0b00000000000000000000000000000000
										// reg_sc_csc_en:[0:0]=0b0   
										// reg_sc_csc_q_mode:[1:1]=0b0   
										// reg_sc_csc_q_drop:[3:2]=0b00   
										// reg_sc_hsv_en:[4:4]=0b0   
										// reg_sc_hsv_floor_en:[5:5]=0b0   
										// reg_sc_csc_bd_dis:[8:8]=0b0   
										// reg_sc_hsv_bd_dis:[9:9]=0b0   
										// reg_bf16_bd_type:[10:10]=0b0   
										// reg_sc_csc_q_gain_mode:[11:11]=0b0   
#define  SC_CSC_REG_01_DATA  0b00000010010110010000000100110010
										// reg_sc_csc_r2y_c00:[13:0]=0b00000100110010   
										// reg_sc_csc_r2y_c01:[29:16]=0b00001001011001   
#define  SC_CSC_REG_02_DATA  0b00011111010100110000000001110101
										// reg_sc_csc_r2y_c02:[13:0]=0b00000001110101   
										// reg_sc_csc_r2y_c10:[29:16]=0b01111101010011   
#define  SC_CSC_REG_03_DATA  0b00000010000000000001111010101101
										// reg_sc_csc_r2y_c11:[13:0]=0b01111010101101   
										// reg_sc_csc_r2y_c12:[29:16]=0b00001000000000   
#define  SC_CSC_REG_04_DATA  0b00011110010100110000001000000000
										// reg_sc_csc_r2y_c20:[13:0]=0b00001000000000   
										// reg_sc_csc_r2y_c21:[29:16]=0b01111001010011   
#define  SC_CSC_REG_05_DATA  0b00000000000000000001111110101101
										// reg_sc_csc_r2y_c22:[13:0]=0b01111110101101   
#define  SC_CSC_REG_06_DATA  0b00000000100000001000000000000000
										// reg_sc_csc_r2y_add_0:[7:0]=0b00000000   
										// reg_sc_csc_r2y_add_1:[15:8]=0b10000000   
										// reg_sc_csc_r2y_add_2:[23:16]=0b10000000   
#define  SC_CSC_REG_07_DATA  0b00000010000000000000001000000000
										// reg_sc_csc_r2y_frac_0:[9:0]=0b1000000000   
										// reg_sc_csc_r2y_frac_1:[25:16]=0b1000000000   
#define  SC_CSC_REG_08_DATA  0b00000000000000000000001000000000
										// reg_sc_csc_r2y_frac_2:[9:0]=0b1000000000   
