// $Module: ive_map $
// $Script Version: 2.1.0 $
// $RegisterBank Version: V 1.0.00 $
// $Author:  $
// $Date: Mon, 13 Sep 2021 05:29:49 PM $
//
#ifndef _IVE_MAP_REG_H
#define _IVE_MAP_REG_H
#ifdef __cplusplus
extern "C"
{
#endif
// Please follow the following template to define registers:

#include <stdio.h>

#undef MY_DEVICE_BASE_ADDR       
#undef MY_DEVICE_REG_CTRL_OFS    
#undef MY_DEVICE_REG_STATUS_OFS  
#undef MY_DEVICE_REG_INTR_OFS    
#undef REG

#define MY_DEVICE_BASE_ADDR       0x00000000
#define MY_DEVICE_REG_CTRL_OFS    0x20
#define MY_DEVICE_REG_STATUS_OFS  0x30
#define MY_DEVICE_REG_INTR_OFS    0x40

#define REG(name) (MY_DEVICE_BASE_ADDR + MY_DEVICE_REG_##name##_OFS)
//-----------------------------------------------------------------------------------
//START OF register definition
//-----------------------------------------------------------------------------------
#include "ive_map_reg_hw.h"
#include "ive_map_reg_sw.h"

typedef struct _IVE_MAP_reg_ {
  IVE_MAP_REG_HW hw;
  IVE_MAP_REG_SW sw;
} IVE_MAP_REG;
//-----------------------------------------------------------------------------------
//END OF register definition
//-----------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------
//Function declaration
//-----------------------------------------------------------------------------------
int ive_map_reg_loader(void* user, const char* section, const char* name, const char* value);

int LOAD_IVE_MAP_INI(char *f);
int DUMP_IVE_MAP_INI(char *f);
int DUMP_IVE_MAP_HW_INI(char *f);
int DUMP_IVE_MAP_SW_INI(char *f);
int PRINT_IVE_MAP_REG();
int INIT_IVE_MAP_REG(char *section);
#ifdef __cplusplus
}
#endif
#endif //_IVE_MAP_REG_H
