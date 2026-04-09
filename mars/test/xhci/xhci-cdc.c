/*
 * OMAP USB HOST xHCI Controller
 *
 * (C) Copyright 2013
 * Texas Instruments, <www.ti.com>
 *
 * Author: Dan Murphy <dmurphy@ti.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <usb.h>
#include <errno.h>

#include <asm/types.h>

#include "mmio.h"
#include "xhci.h"
#include "xhci-cdc.h"

/* Declare global data pointer */

static struct cdc_xhci cadence;

static int cadence_core_init(struct cdc_xhci *cdns)
{
	u32	reg;

	reg = xhci_readl(&cdns->regs->OTGVERSION);

	if ((OTGVERSION_TYPE__OTGVERSION__READ(reg)) != OTG_HW_VERSION) {
		printf("this is not a Cadence USB3 OTG Core\n");
		return -ENODEV;
	}
	cdns->otg_version = OTGVERSION_TYPE__OTGVERSION__READ(reg);

	reg = xhci_readl(&cdns->regs->OTGSTS);
	if (OTGSTS_TYPE__OTG_NRDY__READ(reg) != 0) {
		printf("Cadence USB3 OTG device not ready\n");
		return -ENODEV;
	}

	printf("%s otg_version:0x%x\n", __func__, cdns->otg_version);

	return 0;
}

void cdns_set_mode(struct cdc_xhci *cdns, u32 mode)
{
	u32 reg;

	switch (mode) {
	case CDNS_ROLE_GADGET:
		xhci_writel(&cdns->regs->OTGCMD,
				OTGCMD_TYPE__DEV_BUS_REQ__MASK
				| OTGCMD_TYPE__OTG_DIS__MASK);
		break;
	case CDNS_ROLE_HOST:
		xhci_writel(&cdns->regs->OTGCMD,
				OTGCMD_TYPE__HOST_BUS_REQ__MASK
				| OTGCMD_TYPE__OTG_DIS__MASK);
		break;
	case CDNS_ROLE_OTG:
		reg = xhci_readl(&cdns->regs->OTGCTRL1);
		xhci_writel(&cdns->regs->OTGCTRL1,
				OTGCTRL1_TYPE__IDPULLUP__SET(reg));
		/*
		 * wait until valid ID (ID_VALUE) can be sampled (50ms)
		 */
#ifdef BOARD_PALLADIUM
		udelay(50);
#else
		mdelay(50);
#endif
		/*
		 * OTG mode is initialized later
		 */
		break;
	default:
		printf("Unsupported mode of operation %d\n", mode);
		return;
	}

	/* TODO: Force to usb2.0 mode */
	xhci_writel(&cdns->regs->OTGCMD, xhci_readl(&cdns->regs->OTGCMD) | OTGCMD_TYPE__SS_HOST_DISABLED_SET__MASK);
	cdns->current_mode = mode;
}

static int cadence_core_init_mode(struct cdc_xhci *cdns)
{
	/* Remap usb address of 32~39bit for accessing DDR */
	mmio_write_32(TOP_DDR_ADDR_MODE_REG, (1 << DAMR_REG_USB_REMAP_ADDR_39_32_OFFSET));

    mmio_write_32(TOP_BASE + REG_TOP_SOFT_RST,
            mmio_read_32(TOP_BASE + REG_TOP_SOFT_RST) & (~BIT_TOP_SOFT_RST_USB));
    udelay(50);
	xhci_readl((void *)TOP_USB_CTRSTS_REG);
	mmio_write_32(TOP_USB_CTRSTS_REG, UCR_MODE_STRAP_HOST | (1 << UCR_PORT_OVER_CURRENT_ACTIVE_OFFSET));
	xhci_readl((void *)TOP_USB_CTRSTS_REG);
    mmio_write_32(TOP_BASE + REG_TOP_SOFT_RST,
            mmio_read_32(TOP_BASE + REG_TOP_SOFT_RST) | BIT_TOP_SOFT_RST_USB);
    udelay(50);
	mmio_write_32(TOP_USB_PHY_CTRSTS_REG, mmio_read_32(TOP_USB_PHY_CTRSTS_REG) | (1 << UPCR_EXTERNAL_VBUS_VALID_OFFSET));

	xhci_readl((void *)TOP_USB_PHY_CTRSTS_REG);
	xhci_readl((void *)TOP_USB_CTRSTS_REG);

	cdns_set_mode(cdns, CDNS_ROLE_HOST);

	return 0;
}

static int cadence_xhci_core_init(struct cdc_xhci *cdns)
{
	int ret = 0;

	ret = cadence_core_init(cdns);
	if (ret) {
		debug("%s:failed to initialize core\n", __func__);
		return ret;
	}

	cadence_core_init_mode(cdns);

	/* Reduce reset time value for hub port reset */
	//open in palladium
#ifdef BOARD_PALLADIUM
	xhci_writel((void *)(DRD_XHCI_BASE + 0x8138), (xhci_readl((void *)(DRD_XHCI_BASE + 0x8138)) & ~0x7fff) | 0x100);
	// force fs
	//xhci_writel((void *)(DRD_XHCI_BASE + 0x8130), xhci_readl((void *)(DRD_XHCI_BASE + 0x8130)) | 0x10000);
	// disable serial mode
	xhci_writel((void *)(DRD_XHCI_BASE + 0x8130), xhci_readl((void *)(DRD_XHCI_BASE + 0x8130)) & ~0x100);
#endif
	//scratch_pad_en
	//Command Manager: Enables scratch pad function
	xhci_writel((void *)(DRD_XHCI_BASE + 0x80d4), (xhci_readl((void *)(DRD_XHCI_BASE + 0x80d4)) & ~0x100));

	return ret;
}

static void cadence_xhci_core_exit(struct cdc_xhci *cdsn)
{
	return;
}

int xhci_hcd_init(int index, struct xhci_hccr **hccr, struct xhci_hcor **hcor)
{
	struct cdc_xhci *ctx = &cadence;
	int ret = 0;

	ctx->hcd = (struct xhci_hccr *)DRD_XHCI_BASE;
	ctx->regs = (void *)DRD_REG_BASE;

	ret = cadence_xhci_core_init(ctx);
	if (ret < 0) {
		puts("Failed to initialize xhci\n");
		return ret;
	}

	*hccr = (struct xhci_hccr *)(ctx->hcd);
	*hcor = (struct xhci_hcor *)((uintptr_t) *hccr
				+ HC_LENGTH(xhci_readl(&(*hccr)->cr_capbase)));

	debug("cadence-xhci: init hccr %lx and hcor %lx hc_length %ld\n",
	      (uintptr_t)*hccr, (uintptr_t)*hcor,
	      (uintptr_t)HC_LENGTH(xhci_readl(&(*hccr)->cr_capbase)));

	return ret;
}

void xhci_hcd_stop(int index)
{
	struct cdc_xhci *ctx = &cadence;

	cadence_xhci_core_exit(ctx);
}
