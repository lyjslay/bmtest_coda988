// $Module: reg_ive_map $
// $RegisterBank Version: V 1.0.00 $
// $Author:  $
// $Date: Sun, 26 Sep 2021 04:20:45 PM $
//

#define  REG_0_DATA  0b00000000000000000000000000010100
										// reg_softrst:[0:0]=0b0   
										// reg_ip_enable:[1:1]=0b0   
										// reg_ck_enable:[2:2]=0b1   
										// reg_shdw_sel:[4:4]=0b1   
										// reg_prog_hdk_dis:[8:8]=0b0   
#define  REG_1_DATA  0b00000000000000000000000000000000
										// reg_lut_prog_en:[0:0]=0b0   
										// reg_lut_wsel:[1:1]=0b0   
										// reg_lut_rsel:[2:2]=0b0   
										// reg_sw_lut_rsel:[3:3]=0b0   
#define  REG_2_DATA  0b00000000000000000000000000000000
										// reg_lut_st_addr:[7:0]=0b00000000   
										// reg_lut_st_w1t:[31:31]=0b0   
#define  REG_3_DATA  0b00000000000000000000000000000000
										// reg_lut_wdata:[15:0]=0b0000000000000000   
										// reg_lut_w1t:[31:31]=0b0   
#define  REG_4_DATA  0b00000000000000000000000000000000
										// reg_sw_lut_raddr:[7:0]=0b00000000   
										// reg_sw_lut_r_w1t:[31:31]=0b0   
#define  REG_5_DATA  0b00000000000000000000000000000000
										// reg_sw_lut_rdata:[15:0]=0b0000000000000000   
