#ifndef TEST_H_LNCFF1U6
#define TEST_H_LNCFF1U6

#include <c8051F000.h>
#include "config.h"
#include "cvi_mcu51.h"
#include "chip_cv1822.h"
#include "driver/dw_uart.h"
#include "driver/dw_timer.h"
#include "driver/dw_ictl.h"
#include "driver/mailbox.h"

void udelay_loop(unsigned long usec);
void mdelay_loop(unsigned long msec);
void mcu51_uart_init(char hi, char lo);

int test_robot1();
int test_mcu51_timer();
int test_dw_timer();
int test_mcu51_uart();
int test_rtc_dw_uart();
int test_mailbox();
int test_robot1();
int test_idle();
int test_mailbox();
int test_spinor();
int test_i2c();
int test_gpio();
int test_wfi();
void test_power_osc();
void test_power_wakeup();

#define udelay(usec) udelay_loop(usec)

#endif /* end of include guard: TEST_H_LNCFF1U6 */
