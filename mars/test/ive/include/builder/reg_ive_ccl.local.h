// $Module: reg_ive_ccl $
// $RegisterBank Version: V 1.0.00 $
// $Author: andy.tsao $
// $Date: Sun, 26 Sep 2021 04:20:44 PM $
//

#define  REG_CCL_00_DATA  0b00000000000000000000000000010000
										// reg_ccl_mode:[0:0]=0b0   
										// reg_ccl_shdw_sel:[4:4]=0b1   
#define  REG_CCL_01_DATA  0b00000000000000010000000000000001
										// reg_ccl_area_thr:[15:0]=0b0000000000000001   
										// reg_ccl_area_step:[31:16]=0b0000000000000001   
#define  REG_CCL_02_DATA  0b00000000000000000000000000000000
										// reg_force_clk_enable:[0:0]=0b0   
