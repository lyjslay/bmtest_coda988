// $Module: ive_sad $
// $Script Version: 2.1.0 $
// $RegisterBank Version: V 1.0.00 $
// $Author: andy.tsao $
// $Date: Mon, 13 Sep 2021 05:29:51 PM $
//
#ifndef _IVE_SAD_REG_H
#define _IVE_SAD_REG_H
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

#define MY_DEVICE_BASE_ADDR       0x0A0C65E0
#define MY_DEVICE_REG_CTRL_OFS    0x20
#define MY_DEVICE_REG_STATUS_OFS  0x30
#define MY_DEVICE_REG_INTR_OFS    0x40

#define REG(name) (MY_DEVICE_BASE_ADDR + MY_DEVICE_REG_##name##_OFS)
//-----------------------------------------------------------------------------------
//START OF register definition
//-----------------------------------------------------------------------------------
#include "ive_sad_reg_hw.h"
#include "ive_sad_reg_sw.h"

typedef struct _IVE_SAD_reg_ {
  IVE_SAD_REG_HW hw;
  IVE_SAD_REG_SW sw;
} IVE_SAD_REG;
//-----------------------------------------------------------------------------------
//END OF register definition
//-----------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------
//Function declaration
//-----------------------------------------------------------------------------------
int ive_sad_reg_loader(void* user, const char* section, const char* name, const char* value);

int LOAD_IVE_SAD_INI(char *f);
int DUMP_IVE_SAD_INI(char *f);
int DUMP_IVE_SAD_HW_INI(char *f);
int DUMP_IVE_SAD_SW_INI(char *f);
int PRINT_IVE_SAD_REG();
int INIT_IVE_SAD_REG(char *section);
#ifdef __cplusplus
}
#endif
#endif //_IVE_SAD_REG_H
