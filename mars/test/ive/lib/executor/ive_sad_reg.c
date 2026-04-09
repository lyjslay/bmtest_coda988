// $Module: ive_sad $
// $Script Version: 2.1.0 $
// $RegisterBank Version: V 1.0.00 $
// $Author: andy.tsao $
// $Date: Mon, 13 Sep 2021 05:29:51 PM $
//
#define  _GNU_SOURCE //add for REGEX
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "register.h"
#include "common.h"
#include "ini.h"
#include "log.h"
#include "ive_sad_reg.h"

#include <regex.h>
#include <assert.h>
#define inst ive_sad_reg

IVE_SAD_REG inst;

int ive_sad_reg_loader(void* user, const char* section, const char* name, const char* value)
{
  int hw_hit;
  int sw_hit;
  int hit;
  IVE_SAD_REG* pconfig = (IVE_SAD_REG*)user;
  //-----------------------------------------------------------------------------------
  //Hardware register
  //-----------------------------------------------------------------------------------
  hw_hit = ive_sad_reg_hw_load(&pconfig->hw, section, name, value);
  //-----------------------------------------------------------------------------------
  //Software register
  //-----------------------------------------------------------------------------------
  sw_hit = ive_sad_reg_sw_load(&pconfig->sw, section, name, value);
  hit = hw_hit | sw_hit;

  return hit;
}

  //-----------------------------------------------------------------------------------
  //LOAD INI Function
  //-----------------------------------------------------------------------------------
int LOAD_IVE_SAD_INI(char *s)
{
  CVI_LOG("[LOAD] Load ive_sad INI configurations...");
  if (ini_parse(s, ive_sad_reg_loader, &inst) <0) {
    CVI_LOG("Can't handle ive_sad_reg.");
    return 1;
  }
  CVI_LOG("[LOAD] Load ive_sad Done...");
  #undef  MODULE_SECTION_DEF
  #undef  MODULE_REG_DEF
  #undef  MODULE_LUT_DEF
  #define MODULE_SECTION_DEF(...)
  #define MODULE_REG_DEF(name, ...)  LOAD_INI_CHECK_REG_DEF(hw, name)
  #define MODULE_LUT_DEF(name, ...)  LOAD_INI_CHECK_LUT_DEF(hw, name)
  #undef   REG_DEF_H
  #define  REG_DEF_H() _reg_hw_def.h
  #include MODULE_INCLUDE_DEF()
  #undef  MODULE_SECTION_DEF
  #undef  MODULE_REG_DEF
  #undef  MODULE_LUT_DEF
  #define MODULE_SECTION_DEF(...)
  #define MODULE_REG_DEF(name, ...)  LOAD_INI_CHECK_REG_DEF(sw, name)
  #define MODULE_LUT_DEF(name, ...)  LOAD_INI_CHECK_LUT_DEF(sw, name)
  #undef   REG_DEF_H
  #define  REG_DEF_H() _reg_sw_def.h
  #include MODULE_INCLUDE_DEF()
  return 0;
}

  //-----------------------------------------------------------------------------------
  //DUMP INI Function
  //-----------------------------------------------------------------------------------
int DUMP_IVE_SAD_INI(char *f)
{
  CVI_LOG("[DUMP] Dump ive_sad INI configurations in %s...", f);
  DUMP_IVE_SAD_HW_INI(f);
  DUMP_IVE_SAD_SW_INI(f);
  CVI_LOG("[DUMP] Dump ive_sad INI Done....");
  return 0;
}

GEN_DUMP_INI_BASE_FUNC(DUMP_IVE_SAD_HW_INI, hw, ive_sad_reg_hw_dump_ini)
GEN_DUMP_INI_BASE_FUNC(DUMP_IVE_SAD_SW_INI, sw, ive_sad_reg_sw_dump_ini)


  //-----------------------------------------------------------------------------------
  //Print Function
  //-----------------------------------------------------------------------------------
int PRINT_IVE_SAD_REG()
{
  CVI_LOG("[PRINT] Print ive_sad Reg configurations ...");
  ive_sad_reg_hw_print(&inst.hw);
  ive_sad_reg_sw_print(&inst.sw);
  CVI_LOG("[PRINT] Print ive_sad Reg Done....");
  return 0;
}

  //-----------------------------------------------------------------------------------
  //Init Function
  //-----------------------------------------------------------------------------------
int INIT_IVE_SAD_REG(char* section)
{
  CVI_LOG("[INIT] Init ive_sad Reg configurations...");
  ive_sad_reg_hw_init(&inst.hw, section);
  ive_sad_reg_sw_init(&inst.sw, section);
  CVI_LOG("[INIT] Init ive_sad Reg Done....");
  return 0;
}
