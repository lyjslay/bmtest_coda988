// $Module: reg_isp_rdma $
// $RegisterBank Version: V 1.0.00 $
// $Author: Siwi $
// $Date: Sun, 26 Sep 2021 04:20:44 PM $
//

#define  SHADOW_RD_SEL_DATA  0b00000000000000000000000000000001
										// reg_shadow_rd_sel:[0:0]=0b1   
#define  IP_DISABLE_DATA  0b00000000000000000000000000000000
										// reg_ip_disable:[31:0]=0b00000000000000000000000000000000   
#define  NORM_STATUS0_DATA  0b00000100001000110001111100000000
										// reg_abort_done:[0:0]=0b0   
										// reg_error_axi:[4:4]=0b0   
										// reg_error_id:[12:8]=0b11111   
										// reg_dma_version:[31:16]=0b0000010000100011   
#define  NORM_STATUS1_DATA  0b00000000000000000000000000000000
										// reg_id_done:[31:0]=0b00000000000000000000000000000000   
#define  NORM_PERF_DATA  0b00000000000000000000000000000000
										// reg_bwlwin:[9:0]=0b0000000000   
										// reg_bwltxn:[15:10]=0b000000   
