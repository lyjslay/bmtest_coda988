// $Module: ive_top $
// $Script Version: 2.1.0 $
// $RegisterBank Version: V 1.0.00 $
// $Author:  $
// $Date: Mon, 13 Sep 2021 05:51:30 PM $
//

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "ini.h"
#include "log.h"
#include "ive_top_reg_sw.h"

int ive_top_reg_sw_load(IVE_TOP_REG_SW* sw, const char* section, const char* name, const char* value)
{
  #define MATCH(s, n) strcmp(section, s) == 0 && strcmp(name, n) == 0

  char *tok;

  #undef  MODULE_REG_DEF
  #undef  MODULE_LUT_DEF
  #undef  MODULE_SECTION_DEF
  #define MODULE_SECTION_DEF(name, ...) LOAD_SECTION(name)
  #define MODULE_REG_DEF(name, type, depth, ...) LOAD_REG(sw, name)
  #define MODULE_LUT_DEF(name, type, size, ...) LOAD_LUT(sw, name)
  #include MODULE_INCLUDE_DEF()
  return 1;
}
int ive_top_reg_sw_dump_ini(FILE* fp, IVE_TOP_REG_SW* sw)
{
  int i;
  #undef  MODULE_REG_DEF
  #undef  MODULE_LUT_DEF
  #undef  MODULE_SECTION_DEF
  #define MODULE_SECTION_DEF(...) DUMP_SECTION(sw)
  #define MODULE_REG_DEF(name, type, depth, ...) DUMP_REG(sw, name, depth)
  #define MODULE_LUT_DEF(name, type, size, ...)  DUMP_LUT(sw, name)
  #include MODULE_INCLUDE_DEF()
  return 0;
}
int ive_top_reg_sw_print(IVE_TOP_REG_SW* sw)
{
  int i;
  char array[1024];
  #undef  MODULE_REG_DEF
  #undef  MODULE_LUT_DEF
  #undef  MODULE_SECTION_DEF
  #define MODULE_SECTION_DEF(...) PRINT_SECTION(sw)
  #define MODULE_REG_DEF(name, type, depth, ...) PRINT_REG(sw, name, depth)
  #define MODULE_LUT_DEF(name, type, size, ...)  PRINT_LUT(sw, name)
  #include MODULE_INCLUDE_DEF()
  return 0;
}
int ive_top_reg_sw_init(IVE_TOP_REG_SW* sw, char* section)
{
  #undef  MODULE_SECTION_DEF
  #undef  MODULE_REG_DEF
  #undef  MODULE_LUT_DEF
  #define MODULE_SECTION_DEF(name, ...)          INIT_SECTION(sw, name)
  #define MODULE_REG_DEF(name, type, depth, init, ...) INIT_REG(sw, name, init)
  #define MODULE_LUT_DEF(name, type, size, ...)  INIT_LUT(sw, name, size)
  #include MODULE_INCLUDE_DEF()
  if(section && strcmp(section, "NULL")!=0)
    sprintf(sw->section ,"%s%s", sw->section, section);
  return 0;
}
