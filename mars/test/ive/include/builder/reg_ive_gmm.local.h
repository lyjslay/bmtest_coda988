// $Module: reg_ive_gmm $
// $RegisterBank Version: V 1.0.00 $
// $Author: andy.tsao $
// $Date: Sun, 26 Sep 2021 04:20:44 PM $
//

#define  REG_GMM_0_DATA  0b10110011001100111111111111111111
										// reg_gmm_learn_rate:[15:0]=0b1111111111111111   
										// reg_gmm_bg_ratio:[31:16]=0b1011001100110011   
#define  REG_GMM_1_DATA  0b00000000000000000000011001000000
										// reg_gmm_var_thr:[15:0]=0b0000011001000000   
#define  REG_GMM_2_DATA  0b00000000000010101000110000000000
										// reg_gmm_noise_var:[31:0]=0b00000000000010101000110000000000   
#define  REG_GMM_3_DATA  0b00000000010111011100000000000000
										// reg_gmm_max_var:[31:0]=0b00000000010111011100000000000000   
#define  REG_GMM_4_DATA  0b00000000000010010110000000000000
										// reg_gmm_min_var:[31:0]=0b00000000000010010110000000000000   
#define  REG_GMM_5_DATA  0b00000000000000000000110011001101
										// reg_gmm_init_weight:[31:0]=0b00000000000000000000110011001101   
#define  REG_GMM_6_DATA  0b00000000000010000000000000000001
										// reg_gmm_detect_shadow:[7:0]=0b00000001   
										// reg_gmm_shadow_thr:[15:8]=0b00000000   
										// reg_gmm_sns_factor:[23:16]=0b00001000   
#define  REG_GMM_7_DATA  0b00000000000000011111111111111111
										// reg_gmm2_life_update_factor:[15:0]=0b1111111111111111   
										// reg_gmm2_var_rate:[31:16]=0b0000000000000001   
#define  REG_GMM_8_DATA  0b10000000000000001111111100000000
										// reg_gmm2_freq_redu_factor:[15:0]=0b1111111100000000   
										// reg_gmm2_max_var:[31:16]=0b1000000000000000   
#define  REG_GMM_9_DATA  0b00000000111011110010000000000000
										// reg_gmm2_min_var:[15:0]=0b0010000000000000   
										// reg_gmm2_freq_add_factor:[31:16]=0b0000000011101111   
#define  REG_GMM_10_DATA  0b00101110111000000100111000100000
										// reg_gmm2_freq_init:[15:0]=0b0100111000100000   
										// reg_gmm2_freq_thr:[31:16]=0b0010111011100000   
#define  REG_GMM_11_DATA  0b00000000000010000001001110001000
										// reg_gmm2_life_thr:[15:0]=0b0001001110001000   
										// reg_gmm2_sns_factor:[23:16]=0b00001000   
#define  REG_GMM_12_DATA  0b00000000000100011111111111111111
										// reg_gmm2_factor:[15:0]=0b1111111111111111   
										// reg_gmm2_life_update_factor_mode:[16:16]=0b1   
										// reg_gmm2_sns_factor_mode:[20:20]=0b1   
#define  REG_GMM_13_DATA  0b00000000000000110000000100010000
										// reg_gmm_gmm2_enable:[1:0]=0b00   
										// reg_gmm_gmm2_yonly:[4:4]=0b1   
										// reg_gmm_gmm2_shdw_sel:[8:8]=0b1   
										// reg_force_clk_enable:[12:12]=0b0   
										// reg_gmm_gmm2_model_num:[18:16]=0b011   
