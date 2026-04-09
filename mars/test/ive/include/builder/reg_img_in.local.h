// $Module: reg_img_in $
// $RegisterBank Version: V 1.0.00 $
// $Author:  $
// $Date: Sun, 26 Sep 2021 04:20:43 PM $
//

#define  REG_00_DATA  0b00000000000000010010001100000001
										// reg_src_sel:[1:0]=0b01   
										// reg_fmt_sel:[7:4]=0b0000   
										// reg_burst_ln:[11:8]=0b0011   
										// reg_img_csc_en:[12:12]=0b0   
										// reg_auto_csc_en:[13:13]=0b1   
										// reg_64b_align:[16:16]=0b1   
										// reg_force_clk_enable:[31:31]=0b0   
#define  REG_01_DATA  0b00000000000000000000000000000000
										// reg_src_x_str:[15:0]=0b0000000000000000   
										// reg_src_y_str:[31:16]=0b0000000000000000   
#define  REG_02_DATA  0b00000000111011110000000100111111
										// reg_src_wd:[15:0]=0b0000000100111111   
										// reg_src_ht:[31:16]=0b0000000011101111   
#define  REG_03_DATA  0b00000000000000000000000000000000
										// reg_src_y_pitch:[23:0]=0b000000000000000000000000   
#define  REG_04_DATA  0b00000000000000000000000000000000
										// reg_src_c_pitch:[23:0]=0b000000000000000000000000   
#define  REG_05_DATA  0b00000000000000000000000000000000
										// reg_sw_force_up:[0:0]=0b0   
										// reg_sw_mask_up:[1:1]=0b0   
										// reg_shrd_sel:[2:2]=0b0   
#define  REG_06_DATA  0b00000000000000000000000000000000
										// reg_dummy_ro:[31:0]=0b00000000000000000000000000000000   
#define  REG_07_DATA  0b00000000000000000000000000000000
										// reg_dummy_0:[31:0]=0b00000000000000000000000000000000   
#define  REG_08_DATA  0b11111111111111111111111111111111
										// reg_dummy_1:[31:0]=0b11111111111111111111111111111111   
#define  REG_Y_BASE_0_DATA  0b00000000000000000000000000000000
										// reg_src_y_base_b0:[31:0]=0b00000000000000000000000000000000   
#define  REG_Y_BASE_1_DATA  0b00000000000000000000000000000000
										// reg_src_y_base_b1:[7:0]=0b00000000   
#define  REG_U_BASE_0_DATA  0b00000000000000000000000000000000
										// reg_src_u_base_b0:[31:0]=0b00000000000000000000000000000000   
#define  REG_U_BASE_1_DATA  0b00000000000000000000000000000000
										// reg_src_u_base_b1:[7:0]=0b00000000   
#define  REG_V_BASE_0_DATA  0b00000000000000000000000000000000
										// reg_src_v_base_b0:[31:0]=0b00000000000000000000000000000000   
#define  REG_V_BASE_1_DATA  0b00000000000000000000000000000000
										// reg_src_v_base_b1:[7:0]=0b00000000   
#define  REG_040_DATA  0b00000000000000000000010000000000
										// reg_c00:[13:0]=0b00010000000000   
										// reg_c01:[29:16]=0b00000000000000   
#define  REG_044_DATA  0b00000000000000000000010110011100
										// reg_c02:[13:0]=0b00010110011100   
#define  REG_048_DATA  0b00100001011000000000010000000000
										// reg_c10:[13:0]=0b00010000000000   
										// reg_c11:[29:16]=0b10000101100000   
#define  REG_04C_DATA  0b00000000000000000010001011011011
										// reg_c12:[13:0]=0b10001011011011   
#define  REG_050_DATA  0b00000111000101110000010000000000
										// reg_c20:[13:0]=0b00010000000000   
										// reg_c21:[29:16]=0b00011100010111   
#define  REG_054_DATA  0b00000000000000000000000000000000
										// reg_c22:[13:0]=0b00000000000000   
#define  REG_058_DATA  0b00000000100000001000000000000000
										// reg_sub_0:[7:0]=0b00000000   
										// reg_sub_1:[15:8]=0b10000000   
										// reg_sub_2:[23:16]=0b10000000   
#define  REG_05C_DATA  0b00000000000000000000000000000000
										// reg_add_0:[7:0]=0b00000000   
										// reg_add_1:[15:8]=0b00000000   
										// reg_add_2:[23:16]=0b00000000   
#define  REG_060_DATA  0b00010000001000000001000000100000
										// reg_fifo_rd_th_y:[7:0]=0b00100000   
										// reg_fifo_pr_th_y:[15:8]=0b00010000   
										// reg_fifo_rd_th_c:[23:16]=0b00100000   
										// reg_fifo_pr_th_c:[31:24]=0b00010000   
#define  REG_064_DATA  0b00000000000000000000000000000111
										// reg_os_max:[3:0]=0b0111   
#define  REG_068_DATA  0b00000000000000000000000000000000
										// reg_err_fwr_y:[0:0]=0b0   
										// reg_err_fwr_u:[1:1]=0b0   
										// reg_err_fwr_v:[2:2]=0b0   
										// reg_clr_fwr_w1t:[3:3]=0b0   
										// reg_err_erd_y:[4:4]=0b0   
										// reg_err_erd_u:[5:5]=0b0   
										// reg_err_erd_v:[6:6]=0b0   
										// reg_clr_erd_w1t:[7:7]=0b0   
										// reg_lb_full_y:[8:8]=0b0   
										// reg_lb_full_u:[9:9]=0b0   
										// reg_lb_full_v:[10:10]=0b0   
										// reg_lb_empty_y:[12:12]=0b0   
										// reg_lb_empty_u:[13:13]=0b0   
										// reg_lb_empty_v:[14:14]=0b0   
										// reg_ip_idle:[16:16]=0b0   
										// reg_ip_int:[17:17]=0b0   
										// reg_ip_clr_w1t:[18:18]=0b0   
										// reg_clr_int_w1t:[19:19]=0b0   
#define  REG_AXI_ST_DATA  0b00000000000000000000000000000000
										// reg_axi_idle:[0:0]=0b0   
										// reg_axi_status:[15:8]=0b00000000   
#define  REG_BW_LIMIT_DATA  0b00000000000000000000000000000000
										// reg_bwl_win:[9:0]=0b0000000000   
										// reg_bwl_vld:[25:16]=0b0000000000   
										// reg_bwl_en:[31:31]=0b0   
#define  REG_CATCH_DATA  0b00000000000000000000000000000110
										// reg_catch_mode:[0:0]=0b0   
										// reg_dma_urgent_en:[1:1]=0b1   
										// reg_qos_sel_rr:[2:2]=0b1   
										// reg_catch_act_y:[4:4]=0b0   
										// reg_catch_act_u:[5:5]=0b0   
										// reg_catch_act_v:[6:6]=0b0   
										// reg_catch_fail_y:[8:8]=0b0   
										// reg_catch_fail_u:[9:9]=0b0   
										// reg_catch_fail_v:[10:10]=0b0   
#define  REG_CHK_CTRL_DATA  0b00000000000000000000000000000000
										// reg_chksum_dat_out:[7:0]=0b00000000   
										// reg_checksum_en:[31:31]=0b0   
#define  CHKSUM_AXI_RD_DATA  0b00000000000000000000000000000000
										// reg_chksum_axi_rd:[31:0]=0b00000000000000000000000000000000   
#define  SB_REG_CTRL_DATA  0b00000000000000000000001100000000
										// reg_sb_mode:[1:0]=0b00   
										// reg_sb_size:[3:2]=0b00   
										// reg_sb_nb:[11:8]=0b0011   
										// reg_sb_sw_rptr:[27:24]=0b0000   
										// reg_sb_set_str:[30:30]=0b0   
										// reg_sb_sw_clr:[31:31]=0b0   
#define  SB_REG_C_STAT_DATA  0b00000000000000000000000000000000
										// reg_u_sb_rptr_ro:[3:0]=0b0000   
										// reg_u_sb_full:[6:6]=0b0   
										// reg_u_sb_empty:[7:7]=0b0   
										// reg_u_sb_dptr_ro:[12:8]=0b00000   
										// reg_v_sb_rptr_ro:[19:16]=0b0000   
										// reg_v_sb_full:[22:22]=0b0   
										// reg_v_sb_empty:[23:23]=0b0   
										// reg_v_sb_dptr_ro:[28:24]=0b00000   
#define  SB_REG_Y_STAT_DATA  0b00000000000000000000000000000000
										// reg_y_sb_rptr_ro:[3:0]=0b0000   
										// reg_y_sb_full:[5:5]=0b0   
										// reg_y_sb_empty:[6:6]=0b0   
										// reg_y_sb_dptr_ro:[12:8]=0b00000   
										// reg_sb_empty:[15:15]=0b0   
