// $Module: reg_isp_dma_ctl $
// $RegisterBank Version: V 1.0.00 $
// $Author: brian $
// $Date: Sun, 26 Sep 2021 04:20:44 PM $
//

#define  SYS_CONTROL_DATA  0b00000000001000000000000000000000
										// reg_qos_sel:[0:0]=0b0   
										// reg_sw_qos:[1:1]=0b0   
										// reg_enable_sel:[2:2]=0b0   
										// reg_sw_disable:[3:3]=0b0   
										// reg_baseh:[15:8]=0b00000000   
										// reg_base_sel:[16:16]=0b0   
										// reg_stride_sel:[17:17]=0b0   
										// reg_seglen_sel:[18:18]=0b0   
										// reg_segnum_sel:[19:19]=0b0   
										// reg_slice_enable:[20:20]=0b0   
										// reg_update_base_addr:[21:21]=0b1   
										// reg_dbg_sel:[30:28]=0b000   
#define  BASE_ADDR_DATA  0b00000000000000000000000000000000
										// reg_basel:[31:0]=0b00000000000000000000000000000000   
#define  DMA_SEGLEN_DATA  0b00000000000000000000000000000000
										// reg_seglen:[23:0]=0b000000000000000000000000   
#define  DMA_STRIDE_DATA  0b00000000000000000000000000000000
										// reg_stride:[23:0]=0b000000000000000000000000   
#define  DMA_SEGNUM_DATA  0b00000000000000000000000000000000
										// reg_segnum:[12:0]=0b0000000000000   
#define  DMA_STATUS_DATA  0b00000000000000000000000000000000
										// reg_status:[31:0]=0b00000000000000000000000000000000   
#define  DMA_SLICESIZE_DATA  0b00000000000000000000000000000000
										// reg_slice_size:[5:0]=0b000000   
#define  DMA_SLICENUM_DATA  0b00000000000000000000000000000000
										// reg_slice_num:[28:16]=0b0000000000000   
#define  DMA_DUMMY_DATA  0b00000000000000000101101001011010
										// reg_dummy:[15:0]=0b0101101001011010   
