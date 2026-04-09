// $Module: ive_map $
// $Script Version: 2.1.0 $
// $RegisterBank Version: V 1.0.00 $
// $Author:  $
// $Date: Mon, 13 Sep 2021 05:29:49 PM $
//
//for section: name
  MODULE_SECTION_DEF(ive_map)
//for reg: (name, type, depth, init, min, max, comment...)
  MODULE_REG_DEF (reg_softrst, unsigned int, 1, 0, 0, 0, "")
  MODULE_REG_DEF (reg_ip_enable, unsigned int, 1, 0, 0, 0, "")
  MODULE_REG_DEF (reg_ck_enable, unsigned int, 1, 1, 0, 0, "")
  MODULE_REG_DEF (reg_shdw_sel, unsigned int, 1, 1, 0, 0, "")
  MODULE_REG_DEF (reg_prog_hdk_dis, unsigned int, 1, 0, 0, 0, "")
  MODULE_REG_DEF (reg_lut_prog_en, unsigned int, 1, 0, 0, 0, "")
  MODULE_REG_DEF (reg_lut_wsel, unsigned int, 1, 0, 0, 0, "")
  MODULE_REG_DEF (reg_lut_rsel, unsigned int, 1, 0, 0, 0, "")
  MODULE_REG_DEF (reg_sw_lut_rsel, unsigned int, 1, 0, 0, 0, "")
  MODULE_REG_DEF (reg_lut_st_addr, unsigned int, 8, 0, 0, 0, "")
  MODULE_REG_DEF (reg_lut_st_w1t, unsigned int, 1, 0, 0, 0, "")
  MODULE_REG_DEF (reg_lut_wdata, unsigned int, 16, 0, 0, 0, "")
  MODULE_REG_DEF (reg_lut_w1t, unsigned int, 1, 0, 0, 0, "")
  MODULE_REG_DEF (reg_sw_lut_raddr, unsigned int, 8, 0, 0, 0, "")
  MODULE_REG_DEF (reg_sw_lut_r_w1t, unsigned int, 1, 0, 0, 0, "")
  MODULE_REG_DEF (reg_sw_lut_rdata, unsigned int, 16, 0, 0, 0, "")
//for lookup table: (name, type, size, comment...)
