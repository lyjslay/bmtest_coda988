// $Module: ive_filterop $
// $Script Version: 2.1.0 $
// $RegisterBank Version: V 1.0.00 $
// $Author:  $
// $Date: Mon, 13 Sep 2021 05:48:48 PM $
//

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "ini.h"
#include "log.h"
#include "ive_filterop_reg_hw.h"

int ive_filterop_reg_hw_load(IVE_FILTEROP_REG_HW* hw, const char* section, const char* name, const char* value)
{
  #define MATCH(s, n) strcmp(section, s) == 0 && strcmp(name, n) == 0

  char *tok;

  #undef  MODULE_REG_DEF
  #undef  MODULE_LUT_DEF
  #undef  MODULE_SECTION_DEF
  #define MODULE_SECTION_DEF(name, ...) LOAD_SECTION(name)
  #define MODULE_REG_DEF(name, type, depth, ...) LOAD_REG(hw, name)
  #define MODULE_LUT_DEF(name, type, size, ...) LOAD_LUT(hw, name)
  #include MODULE_INCLUDE_DEF()
  return 1;
}
int ive_filterop_reg_hw_dump_ini(FILE* fp, IVE_FILTEROP_REG_HW* hw)
{
  int i;
  #undef  MODULE_REG_DEF
  #undef  MODULE_LUT_DEF
  #undef  MODULE_SECTION_DEF
  #define MODULE_SECTION_DEF(...) DUMP_SECTION(hw)
  #define MODULE_REG_DEF(name, type, depth, ...) DUMP_REG(hw, name, depth)
  #define MODULE_LUT_DEF(name, type, size, ...)  DUMP_LUT(hw, name)
  #include MODULE_INCLUDE_DEF()
  return 0;
}
int ive_filterop_reg_hw_print(IVE_FILTEROP_REG_HW* hw)
{
  int i;
  char array[1024];
  #undef  MODULE_REG_DEF
  #undef  MODULE_LUT_DEF
  #undef  MODULE_SECTION_DEF
  #define MODULE_SECTION_DEF(...) PRINT_SECTION(hw)
  #define MODULE_REG_DEF(name, type, depth, ...) PRINT_REG(hw, name, depth)
  #define MODULE_LUT_DEF(name, type, size, ...)  PRINT_LUT(hw, name)
  #include MODULE_INCLUDE_DEF()
  return 0;
}
int ive_filterop_reg_hw_init(IVE_FILTEROP_REG_HW* hw, char* section)
{
  #undef  MODULE_SECTION_DEF
  #undef  MODULE_REG_DEF
  #undef  MODULE_LUT_DEF
  #define MODULE_SECTION_DEF(name, ...)          INIT_SECTION(hw, name)
  #define MODULE_REG_DEF(name, type, depth, init, ...) INIT_REG(hw, name, init)
  #define MODULE_LUT_DEF(name, type, size, ...)  INIT_LUT(hw, name, size)
  #include MODULE_INCLUDE_DEF()
  if(section && strcmp(section, "NULL")!=0)
    sprintf(hw->section ,"%s%s", hw->section, section);
  return 0;
}
