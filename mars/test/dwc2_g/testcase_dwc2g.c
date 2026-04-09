/**********************************************************************
 * Copyright (C) 2014-2015 Cadence Design Systems, Inc.
 * All rights reserved worldwide.
 ***********************************************************************
 * main.c
 *
 * Cadence USB Core Driver
 * main component function
 ***********************************************************************/

#include <stdint.h>
#include "boot_test.h"
#include "system_common.h"
#include "module_testcase.h"
#include "map_system_memory.h"
#include "cvi_usb.h"
#include "timer.h"
#include "mmio.h"
#include <byteorder.h>
#include <dwc2_ch9.h>
#include "dwc2_drv_if.h"

extern const char fw_version_info[];
// reference to application
extern int (*getApp())();
extern void (*getIsr())();

static void (*usbIsr)();
static int (*usbApp)();

// critical section functions
void enable_int()
{
	cpu_enable_irqs();
}
void disable_int()
{
	cpu_disable_irqs();
}

// interrupt handler
#if USB_IRQ_MODE
static int irq_handler(int irqn, void *priv)
{
	usbIsr();
	return 0;
}
#endif

static void cvi_usb_hw_init()
{
	unsigned int value;

	//printf("clk_lp div 8\n");
	//mmio_write_32(0x03002A10, 0x003F0406);
	printf("disable clk_lp\n");
	mmio_write_32(0x03002A10, 0x003F0500);
	//mmio_write_32(0x03002A10, 0x0);

	printf("overwrite usb_id\n");
	value = mmio_read_32(TOP_BASE + 0x0048) | 0xC0;
	mmio_write_32(TOP_BASE + 0x0048, value);

	printf("[ECO] rxf\n");
	value = mmio_read_32(TOP_BASE + 0x00B4) | 0x80;
	mmio_write_32(TOP_BASE + 0x00B4, value);
#if MEM_REMAP==1
	printf("enable usb mem remap\n");
	mmio_write_32(TOP_BASE + REG_TOP_ADDR_REMAP, ADDR_REMAP_USB(1));
#else
	printf("disable usb mem remap\n");
#endif

	NOTICE ("bm_usb_hw_init done\n");
}

// program starts here
int testcase_dwc2g()
{
	for (;;) {
	usbApp = getApp();
	usbIsr = getIsr();

	printf("fw_version_info: %s\n", fw_version_info);
	printf("built time: %s %s\n", __DATE__, __TIME__);
	cvi_usb_hw_init();
#if USB_IRQ_MODE
	request_irq(USB_IRQS_0,  irq_handler, 0, NULL, NULL);
	NOTICE ("request_irq, done\n");
	printf ("request_irq, done\n");
#else
	NOTICE ("polling mode\n");
#endif
	usbApp();
	NOTICE ("pattern load test done!!\n");
	}
	return 0;
}

#ifndef BUILD_TEST_CASE_all
int testcase_main()
{
	return testcase_dwc2g();
}
#endif

module_testcase("1", testcase_dwc2g);
