// $Module: ive_map $
// $Script Version: 2.1.0 $
// $RegisterBank Version: V 1.0.00 $
// $Author:  $
// $Date: Mon, 13 Sep 2021 05:29:49 PM $
//
#ifndef _IVE_MAP_REG_SW_H
#define _IVE_MAP_REG_SW_H

#include "reg.h"

#undef  MODULE
#define MODULE() ive_map


typedef struct _IVE_MAP_reg_SW_ {
	char section[32];
#undef  MODULE_REG_DEF
#undef  MODULE_LUT_DEF
#undef  MODULE_SECTION_DEF
#define MODULE_SECTION_DEF(...)
#define MODULE_REG_DEF(name, type, depth, ...) REG_ST_DEF(name, type, depth)
#define MODULE_LUT_DEF(name, type, size, ...)  LUT_ST_DEF(name, type, size)
#undef  REG_DEF_H
#define REG_DEF_H() _reg_sw_def.h
	#include MODULE_INCLUDE_DEF()
} IVE_MAP_REG_SW;

int ive_map_reg_sw_load(IVE_MAP_REG_SW* sw, const char* section, const char* name, const char* value);

int ive_map_reg_sw_dump_ini(FILE* fp, IVE_MAP_REG_SW* sw);

int ive_map_reg_sw_print(IVE_MAP_REG_SW* sw);

int ive_map_reg_sw_init(IVE_MAP_REG_SW* sw, char* section);
#endif //_IVE_MAP_REG_SW_H
