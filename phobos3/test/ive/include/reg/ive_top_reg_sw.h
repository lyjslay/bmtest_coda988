// $Module: ive_top $
// $Script Version: 2.1.0 $
// $RegisterBank Version: V 1.0.00 $
// $Author:  $
// $Date: Mon, 13 Sep 2021 05:51:30 PM $
//
#ifndef _IVE_TOP_REG_SW_H
#define _IVE_TOP_REG_SW_H

#include "reg.h"

#undef  MODULE
#define MODULE() ive_top


typedef struct _IVE_TOP_reg_SW_ {
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
} IVE_TOP_REG_SW;

int ive_top_reg_sw_load(IVE_TOP_REG_SW* sw, const char* section, const char* name, const char* value);

int ive_top_reg_sw_dump_ini(FILE* fp, IVE_TOP_REG_SW* sw);

int ive_top_reg_sw_print(IVE_TOP_REG_SW* sw);

int ive_top_reg_sw_init(IVE_TOP_REG_SW* sw, char* section);
#endif //_IVE_TOP_REG_SW_H
