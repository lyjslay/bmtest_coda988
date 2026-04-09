// $Module: ive_gmm $
// $Script Version: 2.1.0 $
// $RegisterBank Version: V 1.0.00 $
// $Author: andy.tsao $
// $Date: Mon, 13 Sep 2021 05:29:48 PM $
//
//for section: name
  MODULE_SECTION_DEF(ive_gmm)
//for reg: (name, type, depth, init, min, max, comment...)
  MODULE_REG_DEF (reg_gmm_learn_rate, unsigned int, 16, 65535, 0, 0, "")
  MODULE_REG_DEF (reg_gmm_bg_ratio, unsigned int, 16, 45875, 0, 0, "")
  MODULE_REG_DEF (reg_gmm_var_thr, unsigned int, 16, 1600, 0, 0, "")
  MODULE_REG_DEF (reg_gmm_noise_var, unsigned int, 32, 691200, 0, 0, "")
  MODULE_REG_DEF (reg_gmm_max_var, unsigned int, 32, 6144000, 0, 0, "")
  MODULE_REG_DEF (reg_gmm_min_var, unsigned int, 32, 614400, 0, 0, "")
  MODULE_REG_DEF (reg_gmm_init_weight, unsigned int, 32, 3277, 0, 0, "")
  MODULE_REG_DEF (reg_gmm_detect_shadow, unsigned int, 8, 1, 0, 0, "")
  MODULE_REG_DEF (reg_gmm_shadow_thr, unsigned int, 8, 0, 0, 0, "")
  MODULE_REG_DEF (reg_gmm_sns_factor, unsigned int, 8, 8, 0, 0, "")
  MODULE_REG_DEF (reg_gmm2_life_update_factor, unsigned int, 16, 65535, 0, 0, "")
  MODULE_REG_DEF (reg_gmm2_var_rate, unsigned int, 16, 1, 0, 0, "")
  MODULE_REG_DEF (reg_gmm2_freq_redu_factor, unsigned int, 16, 65280, 0, 0, "")
  MODULE_REG_DEF (reg_gmm2_max_var, unsigned int, 16, 32768, 0, 0, "")
  MODULE_REG_DEF (reg_gmm2_min_var, unsigned int, 16, 8192, 0, 0, "")
  MODULE_REG_DEF (reg_gmm2_freq_add_factor, unsigned int, 16, 239, 0, 0, "")
  MODULE_REG_DEF (reg_gmm2_freq_init, unsigned int, 16, 20000, 0, 0, "")
  MODULE_REG_DEF (reg_gmm2_freq_thr, unsigned int, 16, 12000, 0, 0, "")
  MODULE_REG_DEF (reg_gmm2_life_thr, unsigned int, 16, 5000, 0, 0, "")
  MODULE_REG_DEF (reg_gmm2_sns_factor, unsigned int, 8, 8, 0, 0, "")
  MODULE_REG_DEF (reg_gmm2_factor, unsigned int, 16, 65535, 0, 0, "")
  MODULE_REG_DEF (reg_gmm2_life_update_factor_mode, unsigned int, 1, 1, 0, 0, "")
  MODULE_REG_DEF (reg_gmm2_sns_factor_mode, unsigned int, 1, 1, 0, 0, "")
  MODULE_REG_DEF (reg_gmm_gmm2_enable, unsigned int, 2, 0, 0, 0, "")
  MODULE_REG_DEF (reg_gmm_gmm2_yonly, unsigned int, 1, 1, 0, 0, "")
  MODULE_REG_DEF (reg_gmm_gmm2_model_num, unsigned int, 3, 3, 0, 0, "")
//for lookup table: (name, type, size, comment...)
