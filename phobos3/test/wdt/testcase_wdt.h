#ifndef _TESTCASE_WDT_H_
#define _TESTCASE_WDT_H_

#define BIT(n) (UINT32_C(1) << (n)) 

#ifdef CONFIG_C906B
#define WDT2_INTRUPT      -1
#define RTC_WDT_INTRUPT   74
#else // CONFIG_C906L
#define WDT2_INTRUPT      39
#define RTC_WDT_INTRUPT   51
#endif

#define WDT0_BASE     0x10000
#define WDT1_BASE     0x11000
#define WDT2_BASE     0x12000
#define RTC_WDT_BASE  0x0502D000

#endif
