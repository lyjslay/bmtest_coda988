// $Module: reg_ive_sad $
// $RegisterBank Version: V 1.0.00 $
// $Author: andy.tsao $
// $Date: Sun, 26 Sep 2021 04:20:45 PM $
//

#define  REG_SAD_00_DATA  0b00000001000000000000000000000000
										// reg_sad_enmode:[1:0]=0b00   
										// reg_sad_out_ctrl:[6:4]=0b000   
										// reg_sad_u16bit_thr:[23:8]=0b0000000000000000   
										// reg_sad_shdw_sel:[24:24]=0b1   
#define  REG_SAD_01_DATA  0b00000000000000000000000000000000
										// reg_sad_u8bit_max:[7:0]=0b00000000   
										// reg_sad_u8bit_min:[15:8]=0b00000000   
#define  REG_SAD_02_DATA  0b00000000000000000000000000000000
										// reg_sad_enable:[0:0]=0b0   
#define  REG_SAD_03_DATA  0b00000000000000000000000000000000
										// reg_force_clk_enable:[0:0]=0b0   
