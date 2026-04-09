// $Module: reg_ive_update_bg $
// $RegisterBank Version: V 1.0.00 $
// $Author:  $
// $Date: Sun, 26 Sep 2021 04:20:45 PM $
//

#define  REG_1_DATA  0b00000000000000000000000000000000
										// reg_softrst:[0:0]=0b0   
#define  REG_H04_DATA  0b00000000000000000000000000000010
										// reg_enable:[0:0]=0b0   
										// reg_ck_en:[1:1]=0b1   
#define  REG_2_DATA  0b00000000000000000000000000000001
										// reg_shdw_sel:[0:0]=0b1   
#define  REG_3_DATA  0b00000000000000000000000000000000
										// reg_ctrl_dmy1:[31:0]=0b00000000000000000000000000000000   
#define  REG_ctrl0_DATA  0b00000000000000000000000000000000
										// reg_u32CurFrmNum:[31:0]=0b00000000000000000000000000000000   
#define  REG_ctrl1_DATA  0b00000000000000000000000000000000
										// reg_u32PreChkTime:[31:0]=0b00000000000000000000000000000000   
#define  REG_ctrl2_DATA  0b00000000011001000000000000110010
										// reg_u32FrmChkPeriod:[11:0]=0b000000110010   
										// reg_u32InitMinTime:[28:16]=0b0000001100100   
#define  REG_ctrl3_DATA  0b00000000110010000000000011001000
										// reg_u32StyBgMinBlendTime:[15:0]=0b0000000011001000   
										// reg_u32StyBgMaxBlendTime:[31:16]=0b0000000011001000   
#define  REG_ctrl4_DATA  0b00000000010100000000000000000000
										// reg_u32DynBgMinBlendTime:[12:0]=0b0000000000000   
										// reg_u32StaticDetMinTime:[28:16]=0b0000001010000   
#define  REG_ctrl5_DATA  0b00011110010100000011110000001111
										// reg_u16FgMaxFadeTime:[7:0]=0b00001111   
										// reg_u16BgMaxFadeTime:[15:8]=0b00111100   
										// reg_u8StyBgAccTimeRateThr:[22:16]=0b1010000   
										// reg_u8ChgBgAccTimeRateThr:[29:23]=0b0111100   
#define  REG_ctrl6_DATA  0b00000000000000110101101000000000
										// reg_u8DynBgAccTimeThr:[5:0]=0b000000   
										// reg_u8BgEffStaRateThr:[14:8]=0b1011010   
										// reg_u8DynBgDepth:[17:16]=0b11   
										// reg_u8AcceBgLearn:[24:24]=0b0   
										// reg_u8DetChgRegion:[25:25]=0b0   
#define  REG_ctrl7_DATA  0b00000000000000000000000000000000
										// reg_stat_pixnum:[31:0]=0b00000000000000000000000000000000   
#define  REG_ctrl8_DATA  0b00000000000000000000000000000000
										// reg_stat_sumlum:[31:0]=0b00000000000000000000000000000000   
