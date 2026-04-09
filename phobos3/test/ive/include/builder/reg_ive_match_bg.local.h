// $Module: reg_ive_match_bg $
// $RegisterBank Version: V 1.0.00 $
// $Author:  $
// $Date: Sun, 26 Sep 2021 04:20:45 PM $
//

#define  REG_00_DATA  0b00000000000000000000000000000000
										// reg_matchbg_en:[0:0]=0b0   
										// reg_matchbg_softrst:[4:4]=0b0   
#define  REG_04_DATA  0b00000000000000000000000000000000
										// reg_matchbg_curfrmnum:[31:0]=0b00000000000000000000000000000000   
#define  REG_08_DATA  0b00000000000000000000000000000000
										// reg_matchbg_prefrmnum:[31:0]=0b00000000000000000000000000000000   
#define  REG_0C_DATA  0b00000010000001000110000000010100
										// reg_matchbg_timethr:[6:0]=0b0010100   
										// reg_matchbg_diffthrcrlcoef:[10:8]=0b000   
										// reg_matchbg_diffmaxthr:[15:12]=0b0110   
										// reg_matchbg_diffminthr:[19:16]=0b0100   
										// reg_matchbg_diffthrinc:[22:20]=0b000   
										// reg_matchbg_fastlearnrate:[26:24]=0b010   
										// reg_matchbg_detchgregion:[28:28]=0b0   
#define  REG_10_DATA  0b00000000000000000000000000000000
										// reg_matchbg_stat_pixnum:[31:0]=0b00000000000000000000000000000000   
#define  REG_14_DATA  0b00000000000000000000000000000000
										// reg_matchbg_stat_sumlum:[31:0]=0b00000000000000000000000000000000   
