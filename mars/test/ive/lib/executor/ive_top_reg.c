// $Module: ive_top $
// $Script Version: 2.1.0 $
// $RegisterBank Version: V 1.0.00 $
// $Author:  $
// $Date: Mon, 13 Sep 2021 05:51:30 PM $
//
#define  _GNU_SOURCE //add for REGEX
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "register.h"
#include "common.h"
#include "ini.h"
#include "log.h"
#include "ive_top_reg.h"

#include <regex.h>
#include <assert.h>
#define inst ive_top_reg

IVE_TOP_REG inst;

int ive_top_reg_loader(void* user, const char* section, const char* name, const char* value)
{
  int hw_hit;
  int sw_hit;
  int hit;
  IVE_TOP_REG* pconfig = (IVE_TOP_REG*)user;
  //-----------------------------------------------------------------------------------
  //Hardware register
  //-----------------------------------------------------------------------------------
  hw_hit = ive_top_reg_hw_load(&pconfig->hw, section, name, value);
  //-----------------------------------------------------------------------------------
  //Software register
  //-----------------------------------------------------------------------------------
  sw_hit = ive_top_reg_sw_load(&pconfig->sw, section, name, value);
  hit = hw_hit | sw_hit;

  return hit;
}

  //-----------------------------------------------------------------------------------
  //LOAD INI Function
  //-----------------------------------------------------------------------------------
int LOAD_IVE_TOP_INI(char *s)
{
  CVI_LOG("[LOAD] Load ive_top INI configurations...");
  if (ini_parse(s, ive_top_reg_loader, &inst) <0) {
    CVI_LOG("Can't handle ive_top_reg.");
    return 1;
  }
  CVI_LOG("[LOAD] Load ive_top Done...");
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
int DUMP_IVE_TOP_INI(char *f)
{
  CVI_LOG("[DUMP] Dump ive_top INI configurations in %s...", f);
  DUMP_IVE_TOP_HW_INI(f);
  DUMP_IVE_TOP_SW_INI(f);
  CVI_LOG("[DUMP] Dump ive_top INI Done....");
  return 0;
}

GEN_DUMP_INI_BASE_FUNC(DUMP_IVE_TOP_HW_INI, hw, ive_top_reg_hw_dump_ini)
GEN_DUMP_INI_BASE_FUNC(DUMP_IVE_TOP_SW_INI, sw, ive_top_reg_sw_dump_ini)


  //-----------------------------------------------------------------------------------
  //Print Function
  //-----------------------------------------------------------------------------------
int PRINT_IVE_TOP_REG()
{
  CVI_LOG("[PRINT] Print ive_top Reg configurations ...");
  ive_top_reg_hw_print(&inst.hw);
  ive_top_reg_sw_print(&inst.sw);
  CVI_LOG("[PRINT] Print ive_top Reg Done....");
  return 0;
}

  //-----------------------------------------------------------------------------------
  //Init Function
  //-----------------------------------------------------------------------------------
int INIT_IVE_TOP_REG(char* section)
{
  CVI_LOG("[INIT] Init ive_top Reg configurations...");
  ive_top_reg_hw_init(&inst.hw, section);
  ive_top_reg_sw_init(&inst.sw, section);
  CVI_LOG("[INIT] Init ive_top Reg Done....");
  return 0;
}
