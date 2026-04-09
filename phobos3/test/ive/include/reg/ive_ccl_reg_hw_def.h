// $Module: ive_ccl $
// $Script Version: 2.1.0 $
// $RegisterBank Version: V 1.0.00 $
// $Author: andy.tsao $
// $Date: Mon, 13 Sep 2021 05:29:45 PM $
//
//for section: name
  MODULE_SECTION_DEF(ive_ccl)
//for reg: (name, type, depth, init, min, max, comment...)
  MODULE_REG_DEF (reg_ccl_mode, unsigned int, 1, 0, 0, 0, "")
  MODULE_REG_DEF (reg_ccl_area_thr, unsigned int, 16, 1, 0, 0, "")
  MODULE_REG_DEF (reg_ccl_area_step, unsigned int, 16, 1, 0, 0, "")
//for lookup table: (name, type, size, comment...)
