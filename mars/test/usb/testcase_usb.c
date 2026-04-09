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
#include "bm_usb.h"
#include "timer.h"
#include "mmio.h"

extern const char fw_version_info[];
// reference to application
extern int (*getApp())();
extern void (*getIsr())();

static void (*usbIsr)();
static int (*usbApp)();

// critical section functions
void enable_int() {
    cpu_enable_irqs();
}
void disable_int() {
    cpu_disable_irqs();
}

// interrupt handler
static int irq_handler(int irqn, void *priv) {
    usbIsr();
    return 0;
}

static void bm_usb_hw_init()
{
    unsigned int value;
    mmio_write_32(TOP_BASE + REG_TOP_SOFT_RST,
            mmio_read_32(TOP_BASE + REG_TOP_SOFT_RST) & (~BIT_TOP_SOFT_RST_USB));
    // udelay(50);
    mmio_write_32(TOP_BASE + REG_TOP_SOFT_RST,
            mmio_read_32(TOP_BASE + REG_TOP_SOFT_RST) | BIT_TOP_SOFT_RST_USB);
    // Wait till OTG_NRDY cleared.
    printf("wait OTG register ready\n");
    do {
      value = mmio_read_32(USB_BASE + 0x04);
      mdelay(1);
    } while (value & OTG_NRDY);
    printf("OTG register is ready\n");

    mmio_write_32(USB_BASE, (SS_PERIPH_DISABLED_SET | HOST_BUS_DROP | DEV_BUS_REQ));

    // Wait till IN_RST set.
    printf("wait DEV register ready\n");
    do {
      value = mmio_read_32(USB_DEV_BASE + 0x04);
      mdelay(1);
    } while (!(value & IN_RST));
    printf("DEV register is ready\n");
#if MEM_REMAP==1
    printf("enable usb mem remap\n");
    mmio_write_32(TOP_BASE + REG_TOP_ADDR_REMAP, ADDR_REMAP_USB(1));
#else
    printf("disable usb mem remap\n");
#endif
    NOTICE ("bm_usb_hw_init done\n");
}

// program starts here
int testcase_usb()
{
    usbApp = getApp();
    usbIsr = getIsr();

    printf("fw_version_info: %s\n", fw_version_info);
    printf("built time: %s %s\n", __DATE__, __TIME__);
    bm_usb_hw_init();
#if USB_IRQ_MODE
    request_irq(USB_OTGIRQ,  irq_handler, 0, NULL, NULL);
    request_irq(USB_IRQS_0, irq_handler, 0, NULL, NULL);
    request_irq(USB_IRQS_1, irq_handler, 0, NULL, NULL);
    NOTICE ("request_irq, done\n");
#else
    NOTICE ("polling mode\n");
#endif
    usbApp();

    return 0;
}

#ifndef BUILD_TEST_CASE_all
int testcase_main()
{
    return testcase_usb();
}
#endif

module_testcase("1", testcase_usb);
