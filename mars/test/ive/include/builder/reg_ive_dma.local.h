// $Module: reg_ive_dma $
// $RegisterBank Version: V 1.0.00 $
// $Author:  $
// $Date: Sun, 26 Sep 2021 04:20:44 PM $
//

#define  REG_0_DATA  0b00000000000000000000000000000010
										// reg_ive_dma_enable:[0:0]=0b0   
										// reg_shdw_sel:[1:1]=0b1   
										// reg_softrst:[2:2]=0b0   
										// reg_ive_dma_mode:[5:4]=0b00   
#define  REG_1_DATA  0b00000000000000000000000000000000
										// reg_ive_dma_src_stride:[15:0]=0b0000000000000000   
										// reg_ive_dma_dst_stride:[31:16]=0b0000000000000000   
#define  REG_2_DATA  0b00000000000000000000000000000001
										// reg_ive_dma_src_mem_addr:[31:0]=0b00000000000000000000000000000001   
#define  REG_3_DATA  0b00000000000000000000000000000001
										// reg_ive_dma_dst_mem_addr:[31:0]=0b00000000000000000000000000000001   
#define  REG_4_DATA  0b00000000000000000000000000000000
										// reg_ive_dma_horsegsize:[7:0]=0b00000000   
										// reg_ive_dma_elemsize:[15:8]=0b00000000   
										// reg_ive_dma_versegrow:[23:16]=0b00000000   
#define  REG_5_DATA  0b00000000000000000000000000000000
										// reg_ive_dma_u64_val:[63:0]=0b0000000000000000000000000000000000000000000000000000000000000000   
