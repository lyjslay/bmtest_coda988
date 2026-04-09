// $Module: reg_isp_wdma $
// $RegisterBank Version: V 1.0.00 $
// $Author: brian $
// $Date: Sun, 26 Sep 2021 04:20:44 PM $
//

#define  SHADOW_RD_SEL_DATA  0b00000000000000000000000000000001
										// reg_shadow_rd_sel:[0:0]=0b1   
#define  IP_DISABLE_DATA  0b00000000000000000000000000000000
										// reg_ip_disable:[31:0]=0b00000000000000000000000000000000   
#define  NORM_STATUS0_DATA  0b00000111000000110001111100000000
										// reg_abort_done:[0:0]=0b0   
										// reg_error_axi:[4:4]=0b0   
										// reg_error_dmi:[5:5]=0b0   
										// reg_slot_full:[6:6]=0b0   
										// reg_error_id:[12:8]=0b11111   
										// reg_dma_version:[31:16]=0b0000011100000011   
#define  NORM_STATUS1_DATA  0b00000000000000000000000000000000
										// reg_id_done:[31:0]=0b00000000000000000000000000000000   
#define  NORM_PERF_DATA  0b00000000000000000000000000000000
										// reg_bwlwin:[9:0]=0b0000000000   
										// reg_bwltxn:[15:10]=0b000000   
										// reg_qoso_th:[19:16]=0b0000   
										// reg_qoso_en:[20:20]=0b0   
