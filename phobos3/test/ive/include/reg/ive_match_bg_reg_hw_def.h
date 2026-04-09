// $Module: ive_match_bg $
// $Script Version: 2.1.0 $
// $RegisterBank Version: V 1.0.00 $
// $Author:  $
// $Date: Mon, 13 Sep 2021 05:29:50 PM $
//
//for section: name
  MODULE_SECTION_DEF(ive_match_bg)
//for reg: (name, type, depth, init, min, max, comment...)
  MODULE_REG_DEF (reg_matchbg_en, unsigned int, 1, 0, 0, 0, "")
  MODULE_REG_DEF (reg_matchbg_softrst, unsigned int, 1, 0, 0, 0, "")
  MODULE_REG_DEF (reg_matchbg_curfrmnum, unsigned int, 32, 0, 0, 0, "")
  MODULE_REG_DEF (reg_matchbg_prefrmnum, unsigned int, 32, 0, 0, 0, "")
  MODULE_REG_DEF (reg_matchbg_timethr, unsigned int, 7, 20, 0, 0, "")
  MODULE_REG_DEF (reg_matchbg_diffthrcrlcoef, unsigned int, 3, 0, 0, 0, "")
  MODULE_REG_DEF (reg_matchbg_diffmaxthr, unsigned int, 4, 6, 0, 0, "")
  MODULE_REG_DEF (reg_matchbg_diffminthr, unsigned int, 4, 4, 0, 0, "")
  MODULE_REG_DEF (reg_matchbg_diffthrinc, unsigned int, 3, 0, 0, 0, "")
  MODULE_REG_DEF (reg_matchbg_fastlearnrate, unsigned int, 3, 2, 0, 0, "")
  MODULE_REG_DEF (reg_matchbg_detchgregion, unsigned int, 1, 0, 0, 0, "")
  MODULE_REG_DEF (reg_matchbg_stat_pixnum, unsigned int, 32, 0, 0, 0, "")
  MODULE_REG_DEF (reg_matchbg_stat_sumlum, unsigned int, 32, 0, 0, 0, "")
//for lookup table: (name, type, size, comment...)
