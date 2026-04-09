#include <stdio.h>
#include <reg51.h>
#include "chip_cv1822.h"
#include "driver/dw_ictl.h"
#include "driver/dw_uart.h"

#ifdef CONFIG_ENABLE_ISR
void dw_timer1_isr(void);
void mailbox_isr(void);
void wdt_isr(void);
#endif

static uint32_t xdata dw_ictl_status;
static uint32_t xdata dw_ictl_count;

uint32_t dw_ictl_get_status()
{
	return dw_ictl_status;
}

uint32_t dw_ictl_get_count()
{
	return dw_ictl_count;
}

void dw_ictl_intr_handler(void) interrupt 0 using 1
{
	dw_ictl_status = read_robot_irq(REG_DW_ICTL_FINALSTATUS_L);
	dw_ictl_count++;

	switch (dw_ictl_status) {
#if TEST_DW_TIMER
		case 1 << TIMER1_INTR:
			dw_timer1_isr();
			break;
#endif

#if TEST_MAILBOX
		case 1 << MBOX_INTR:
			mailbox_isr();
			break;
#endif
#if TEST_WDT
		case 1UL << WDT_INTR:
			wdt_isr();
			break;
#endif
		default:
		break;
	}
}

void dw_ictl_init(void)
{
	// Mask all interrupts
	write_robot(REG_DW_ICTL_MASK_L, 0xFFFFFFFF);
	write_robot(REG_DW_ICTL_MASK_H, 0xFFFFFFFF);

	// Enable all interrupts
	write_robot(REG_DW_ICTL_ENABLE_L, 0xFFFFFFFF);
	write_robot(REG_DW_ICTL_ENABLE_H, 0xFFFFFFFF);
}

void dw_ictl_unmask(char d)
{
	xdata uint32_t v;
	xdata uint32_t mask = 1 << d;

	v = read_robot(REG_DW_ICTL_MASK_L);
	v &= ~mask;
	write_robot(REG_DW_ICTL_MASK_L, v);
}

void dw_ictl_mask(char d)
{
	xdata uint32_t v;
	xdata uint32_t mask = 1 << d;

	v = read_robot(REG_DW_ICTL_MASK_L);
	v |= mask;
	write_robot(REG_DW_ICTL_MASK_L, v);
}