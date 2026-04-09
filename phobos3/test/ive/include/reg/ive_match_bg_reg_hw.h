// $Module: ive_match_bg $
// $Script Version: 2.1.0 $
// $RegisterBank Version: V 1.0.00 $
// $Author:  $
// $Date: Mon, 13 Sep 2021 05:29:50 PM $
//
#ifndef _IVE_MATCH_BG_REG_HW_H
#define _IVE_MATCH_BG_REG_HW_H

#include "reg.h"

#undef  MODULE
#define MODULE() ive_match_bg


typedef struct _IVE_MATCH_BG_reg_HW_ {
	char section[32];
#undef  MODULE_REG_DEF
#undef  MODULE_LUT_DEF
#undef  MODULE_SECTION_DEF
#define MODULE_SECTION_DEF(...)
#define MODULE_REG_DEF(name, type, depth, ...) REG_ST_DEF(name, type, depth)
#define MODULE_LUT_DEF(name, type, size, ...)  LUT_ST_DEF(name, type, size)
#undef  REG_DEF_H
#define REG_DEF_H() _reg_hw_def.h
	#include MODULE_INCLUDE_DEF()
} IVE_MATCH_BG_REG_HW;

int ive_match_bg_reg_hw_load(IVE_MATCH_BG_REG_HW* hw, const char* section, const char* name, const char* value);

int ive_match_bg_reg_hw_dump_ini(FILE* fp, IVE_MATCH_BG_REG_HW* hw);

int ive_match_bg_reg_hw_print(IVE_MATCH_BG_REG_HW* hw);

int ive_match_bg_reg_hw_init(IVE_MATCH_BG_REG_HW* hw, char* section);
#endif //_IVE_MATCH_BG_REG_HW_H
