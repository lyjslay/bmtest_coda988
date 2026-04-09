// $Module: ive_sad $
// $Script Version: 2.1.0 $
// $RegisterBank Version: V 1.0.00 $
// $Author: andy.tsao $
// $Date: Mon, 13 Sep 2021 05:29:51 PM $
//
//for section: name
  MODULE_SECTION_DEF(ive_sad)
//for reg: (name, type, depth, init, min, max, comment...)
  MODULE_REG_DEF (reg_sad_enmode, unsigned int, 2, 0, 0, 0, "")
  MODULE_REG_DEF (reg_sad_out_ctrl, unsigned int, 3, 0, 0, 0, "")
  MODULE_REG_DEF (reg_sad_u16bit_thr, unsigned int, 16, 0, 0, 0, "")
  MODULE_REG_DEF (reg_sad_u8bit_max, unsigned int, 8, 0, 0, 0, "")
  MODULE_REG_DEF (reg_sad_u8bit_min, unsigned int, 8, 0, 0, 0, "")
  MODULE_REG_DEF (reg_sad_enable, unsigned int, 1, 0, 0, 0, "")
//for lookup table: (name, type, size, comment...)
