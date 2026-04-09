#ifndef CONFIG_H_I86QB2PE
#define CONFIG_H_I86QB2PE

#define CONFIG_ENABLE_ISR

#define CONFIG_FPGA
// #define CONFIG_ASIC

// #define CONFIG_PUTCHAR_51_SERIAL
#define CONFIG_PUTCHAR_DW_UART

#define CONFIG_DW_UART_TOP
// #define CONFIG_DW_UART_RTC

#define TEST_ROBOT1 0
#define TEST_GPIO 0
#define TEST_RTC_DW_UART 0
#define TEST_MCU51_UART 0
#define TEST_MCU51_TIMER 0
#define TEST_DW_TIMER 0
#define TEST_MAILBOX 0
#define TEST_IDLE 0
#define TEST_IDLE_2 0
#define TEST_I2C 0
#define TEST_SPINOR 0
#define TEST_WDT 0

#define TSET_POWER_WFI 0
#define TSET_POWER_OSC 0

#endif /* end of include guard: CONFIG_H_I86QB2PE */
