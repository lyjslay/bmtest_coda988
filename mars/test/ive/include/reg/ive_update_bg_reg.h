// $Module: ive_update_bg $
// $Script Version: 2.1.0 $
// $RegisterBank Version: V 1.0.00 $
// $Author:  $
// $Date: Mon, 13 Sep 2021 05:29:53 PM $
//
#ifndef _IVE_UPDATE_BG_REG_H
#define _IVE_UPDATE_BG_REG_H
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
#include "ive_update_bg_reg_hw.h"
#include "ive_update_bg_reg_sw.h"

typedef struct _IVE_UPDATE_BG_reg_ {
  IVE_UPDATE_BG_REG_HW hw;
  IVE_UPDATE_BG_REG_SW sw;
} IVE_UPDATE_BG_REG;
//-----------------------------------------------------------------------------------
//END OF register definition
//-----------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------
//Function declaration
//-----------------------------------------------------------------------------------
int ive_update_bg_reg_loader(void* user, const char* section, const char* name, const char* value);

int LOAD_IVE_UPDATE_BG_INI(char *f);
int DUMP_IVE_UPDATE_BG_INI(char *f);
int DUMP_IVE_UPDATE_BG_HW_INI(char *f);
int DUMP_IVE_UPDATE_BG_SW_INI(char *f);
int PRINT_IVE_UPDATE_BG_REG();
int INIT_IVE_UPDATE_BG_REG(char *section);
#ifdef __cplusplus
}
#endif
#endif //_IVE_UPDATE_BG_REG_H
