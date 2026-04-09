// $Module: ive_update_bg $
// $Script Version: 2.1.0 $
// $RegisterBank Version: V 1.0.00 $
// $Author:  $
// $Date: Mon, 13 Sep 2021 05:29:53 PM $
//
//for section: name
  MODULE_SECTION_DEF(ive_update_bg)
//for reg: (name, type, depth, init, min, max, comment...)
  MODULE_REG_DEF (reg_softrst, unsigned int, 1, 0, 0, 0, "")
  MODULE_REG_DEF (reg_enable, unsigned int, 1, 0, 0, 0, "")
  MODULE_REG_DEF (reg_ck_en, unsigned int, 1, 1, 0, 0, "")
  MODULE_REG_DEF (reg_shdw_sel, unsigned int, 1, 1, 0, 0, "")
  MODULE_REG_DEF (reg_ctrl_dmy1, unsigned int, 32, 0, 0, 0, "")
  MODULE_REG_DEF (reg_u32CurFrmNum, unsigned int, 32, 0, 0, 0, "")
  MODULE_REG_DEF (reg_u32PreChkTime, unsigned int, 32, 0, 0, 0, "")
  MODULE_REG_DEF (reg_u32FrmChkPeriod, unsigned int, 12, 50, 0, 0, "")
  MODULE_REG_DEF (reg_u32InitMinTime, unsigned int, 13, 100, 0, 0, "")
  MODULE_REG_DEF (reg_u32StyBgMinBlendTime, unsigned int, 16, 200, 0, 0, "")
  MODULE_REG_DEF (reg_u32StyBgMaxBlendTime, unsigned int, 16, 200, 0, 0, "")
  MODULE_REG_DEF (reg_u32DynBgMinBlendTime, unsigned int, 13, 0, 0, 0, "")
  MODULE_REG_DEF (reg_u32StaticDetMinTime, unsigned int, 13, 80, 0, 0, "")
  MODULE_REG_DEF (reg_u16FgMaxFadeTime, unsigned int, 8, 15, 0, 0, "")
  MODULE_REG_DEF (reg_u16BgMaxFadeTime, unsigned int, 8, 60, 0, 0, "")
  MODULE_REG_DEF (reg_u8StyBgAccTimeRateThr, unsigned int, 7, 80, 0, 0, "")
  MODULE_REG_DEF (reg_u8ChgBgAccTimeRateThr, unsigned int, 7, 60, 0, 0, "")
  MODULE_REG_DEF (reg_u8DynBgAccTimeThr, unsigned int, 6, 0, 0, 0, "")
  MODULE_REG_DEF (reg_u8BgEffStaRateThr, unsigned int, 7, 90, 0, 0, "")
  MODULE_REG_DEF (reg_u8DynBgDepth, unsigned int, 2, 3, 0, 0, "")
  MODULE_REG_DEF (reg_u8AcceBgLearn, unsigned int, 1, 0, 0, 0, "")
  MODULE_REG_DEF (reg_u8DetChgRegion, unsigned int, 1, 0, 0, 0, "")
  MODULE_REG_DEF (reg_stat_pixnum, unsigned int, 32, 0, 0, 0, "")
  MODULE_REG_DEF (reg_stat_sumlum, unsigned int, 32, 0, 0, 0, "")
//for lookup table: (name, type, size, comment...)
