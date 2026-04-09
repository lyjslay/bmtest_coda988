/**********************************************************************
 * Copyright (C) 2014-2017 Cadence Design Systems, Inc.
 * All rights reserved worldwide.
 ***********************************************************************
 * bulk_app_lb.c
 * Bulk echo application.
 *
 ***********************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "dwc2_stdio.h"
#include <timer.h>

#include "debug.h"
#include <system_common.h>
#include <byteorder.h>
#include <dwc2_ch9.h>
#include "dwc2_drv_if.h"
#include "dwc2_udc_otg_regs.h"
#include "dwc2_udc_otg_priv.h"
#include <dwc2_udc.h>
#include <malloc.h>

#include "map_system_memory.h"

//#define bulk_debug printf
#define bulk_debug(...)
#define EP0_DATA_BUFFER_SIZE 0x8000
#define DBG_USB_CTRL_MSG 0x00000002
#define USE_LARGE_TRB_SIZE

#define BULK_TEST_STALL_OUT	0xDEAD
#define BULK_TEST_STALL_IN	0xBEEF
extern void enable_int(void);
extern void disable_int(void);

static struct dwc2_drv_obj drv_obj = {
	.plat = {
		.regs_otg = USB_BASE, // address where USB core is mapped
		//.usb_gusbcfg    = 0x40081407, //8BIT mode
		.usb_gusbcfg    = 0x40081408, // 16BIT mode
		.rx_fifo_sz     = 512,
		.np_tx_fifo_sz  = 512,
		.tx_fifo_sz     = 512,
	},
};

// interrupt handler
void bulkIsrLB()
{
	usb_gadget_handle_interrupts(0);
}

//-------------- definitions used during configuration -------------------------

#define BCD_USB_SS                      0x0300  // 3.00 version USB
#define ID_VENDOR                       0x0559  // Cadence

#ifdef HS_FS_ONLY
#define ID_PRODUCT                      0x1004  // example bulk product
#else
#define ID_PRODUCT                      0x4012  // example bulk product
#endif

#define BCD_DEVICE_SS                   0x0001  // 0.1

#define BCD_USB_HS_ONLY                 0x0201  // 2.01  /*Only HS with BOS descriptor*/
#define BCD_USB_HS                      0x0210  // 2.10

#define BCD_DEVICE_HS                   0x0200  // 2.00

#define USB_MANUFACTURER_STRING         "CADENCE"
#define USB_PRODUCT_STRING              "Bulk example device"
#define USB_SERIAL_NUMBER_STRING        "100000000000" // should 12 chars long
#define BULK_EP_IN 0x81
#define BULK_EP_OUT 0x01
//------------------------------------------------------------------------------
#define BUF_SIZE_BULK			0x20000
#define BUF_SIZE_SETUP			0x20
#define BUF_SIZE_CMD			0x20
#define BUF_SIZE_EP0			0x400
#define HANDLER_SIZE			1024

#define BUF_BASE			0x0C000000
#define BULK_BUF			(BUF_BASE)
#define SETUP_BUF			(BUF_BASE + BUF_SIZE_BULK)
#define CMD_BUF				(SETUP_BUF + BUF_SIZE_SETUP)
#define EP0_BUF				(CMD_BUF + BUF_SIZE_CMD)
#define HANDLER_ADDR			(EP0_BUF + BUF_SIZE_EP0)  // 1024
static uint8_t* bulkBuf, * cmdBuf, *ep0Buff; // keeps mouse report
#if 1
#if defined(USE_LARGE_TRB_SIZE)
static uint8_t bulkBufAlloc[1024 * 1024 * 10  + 31]; //16 MB
#else
static uint8_t bulkBufAlloc[1024 * 8  + 31]; //16 MB
#endif
static uint8_t setupBufAlloc[8 + 31];
static uint8_t cmdBufAlloc[8 + 31];
static uint8_t ep0BuffAlloc[1024 * 8 + 31]; //64 MB
static uint8_t handler[1024];
#else
static uint8_t *bulkBufAlloc = (uint8_t *) BULK_BUF;
static uint8_t *setupBufAlloc = (uint8_t *) SETUP_BUF;
static uint8_t *cmdBufAlloc = (uint8_t *) CMD_BUF;
static uint8_t *ep0BuffAlloc = (uint8_t *) EP0_BUF;
static uint8_t *handler      = (uint8_t *)HANDLER_ADDR; // 1024
#endif
static struct usb_request *bulkInReq, *bulkOutReq, *ep0Req; // request used by driver
static uint8_t configValue = 0;
static uint8_t current_speed = CH9_USB_SPEED_UNKNOWN;
static uint8_t cmd_flag = 0;

// debug functions
static void displayRequestinfo(struct usb_request *req);
static void displayEndpointInfo(struct usb_ep *ep);
static void displayDeviceInfo(struct usb_gadget *gadget);
static struct usb_ep * epIn, * epOut;

static void bulkInCmpl(struct usb_ep *ep, struct usb_request * req)
{
	VERBOSE("Transfer complete on ep:%02X %08X req\n", ep->desc->bEndpointAddress, (uintptr_t) req);
	displayRequestinfo(req);
	if (req->status != 0) {
		return;
	}

	cmd_flag = 1;
	bulkOutReq->length = 8;
	bulkOutReq->buf = cmdBuf;
	bulkOutReq->dma = (uintptr_t) cmdBuf;
	epOut->ops->queue(epOut, bulkOutReq);
}

static void bulkOutCmpl(struct usb_ep *ep, struct usb_request * req)
{

	uint8_t dir = ((uint8_t*) req->buf)[0];
	uint32_t length = le32ToCpu(*(uint32_t*) (req->buf + 4));

	VERBOSE("Transfer complete on ep:%02X %08X req\n", ep->desc->bEndpointAddress, (uintptr_t) req);
	//displayRequestinfo(req);
	if (req->status != 0) {
		return;
	}

	VERBOSE("DATA DIR: %d\n", dir);
	VERBOSE("DATA LENGTH: %X\n", length);

	if (cmd_flag) {
		cmd_flag = 0;
		if (dir) {
			if (length == BULK_TEST_STALL_IN) {
				epIn->ops->set_halt(epIn, 1);
				VERBOSE("stall epIn!\n");

				cmd_flag = 1;
				bulkOutReq->length = 8;
				bulkOutReq->buf = cmdBuf;
				bulkOutReq->dma = (uintptr_t) cmdBuf;
				epOut->ops->queue(epOut, bulkOutReq);
				return;
			}
			bulkInReq->length = length;
			bulkInReq->buf = bulkBuf;
			bulkInReq->dma = (uintptr_t) bulkBuf;
			epIn->ops->queue(epIn, bulkInReq);
		} else {
			if (length == BULK_TEST_STALL_OUT) {
				epOut->ops->set_halt(epOut, 1);
				VERBOSE("stall epOut!\n");

				cmd_flag = 1;
				bulkOutReq->length = 8;
				bulkOutReq->buf = cmdBuf;
				bulkOutReq->dma = (uintptr_t) cmdBuf;
				epOut->ops->queue(epOut, bulkOutReq);
				return;
			}
			bulkOutReq->length = length;
			bulkOutReq->buf = bulkBuf;
			bulkOutReq->dma = (uintptr_t) bulkBuf;
			epOut->ops->queue(epOut, bulkOutReq);
		}
	} else {
		cmd_flag = 1;
		bulkOutReq->length = 8;
		bulkOutReq->buf = cmdBuf;
		bulkOutReq->dma = (uintptr_t) cmdBuf;
		epOut->ops->queue(epOut, bulkOutReq);
	}
}

// clears request

static void clearReq(struct usb_request * req)
{
	memset(req, 0, sizeof (struct usb_request));
}

// function return unicode string for english version

static void get_unicode_string(char * target, const char * src)
{

	size_t src_len = strlen(src) * 2;
	int i;

	*target++ = src_len + 2;
	*target++ = CH9_USB_DT_STRING;

	if (src_len > 100) {
		src_len = 100;
	}
	for (i = 0; i < src_len; i += 2) {
		*target++ = *src++;
		*target++ = 0;
	}
}

// functions aligns buffer to alignValue modulo address

static uint8_t * alignBuff(void * notAllignedBuff, uint8_t alignValue)
{

	uint8_t offset = ((uintptr_t) notAllignedBuff) % alignValue;
	if (offset == 0) {
		return (uint8_t*) notAllignedBuff;
	} else {
		uint8_t * ret_address = &(((uint8_t*) notAllignedBuff)[alignValue - offset]);
		return ret_address;
	}
}

//------------------- descriptor set--------------------------------------------
// device descriptor for SuperSpeed mode
static CH9_UsbDeviceDescriptor devSsDesc = {
	CH9_USB_DS_DEVICE,
	CH9_USB_DT_DEVICE,
	cpuToLe16(BCD_USB_SS),
	0,
	0,
	0,
	9,
	cpuToLe16(ID_VENDOR),
	cpuToLe16(ID_PRODUCT),
	cpuToLe16(BCD_DEVICE_SS),
	1,
	2,
	3,
	1
};

// device descriptor for HighSpeed mode
static CH9_UsbDeviceDescriptor devHsDesc = {
	CH9_USB_DS_DEVICE,
	CH9_USB_DT_DEVICE,
	cpuToLe16(BCD_USB_HS),
	0,
	0,
	0,
	64,
	cpuToLe16(ID_VENDOR),
	cpuToLe16(ID_PRODUCT),
	cpuToLe16(BCD_DEVICE_HS),
	1,
	2,
	3,
	1
};
//------------- Start of Super Speed configuration descriptors -----------------
// configuration descriptors for SuperSpeed mode
static CH9_UsbConfigurationDescriptor confSsDesc = {
	CH9_USB_DS_CONFIGURATION,
	CH9_USB_DT_CONFIGURATION,
	cpuToLe16(
		CH9_USB_DS_CONFIGURATION
		+ CH9_USB_DS_INTERFACE
		+ 2 * CH9_USB_DS_ENDPOINT
		+ 2 * CH9_USB_DS_SS_USB_ENDPOINT_COMPANION),
	1,
	1,
	0,
#if defined(TEST_RWP)
	CH9_USB_CONFIG_RESERVED | CH9_USB_CONFIG_SELF_POWERED | CH9_USB_CONFIG_REMOTE_WAKEUP,
#else
	CH9_USB_CONFIG_RESERVED | CH9_USB_CONFIG_SELF_POWERED,
#endif
	1
};

//0x09, 0x04, 0x00, 0x00, 0x01, 0x03, 0x00, 0x00, 0x00,
static CH9_UsbInterfaceDescriptor interfaceDesc = {
	CH9_USB_DS_INTERFACE,
	CH9_USB_DT_INTERFACE,
	0,
	0,
	2,
	CH9_USB_CLASS_VENDOR,
	0,
	0,
	0
};

static CH9_UsbEndpointDescriptor endpointEpInDesc = {
	CH9_USB_DS_ENDPOINT,
	CH9_USB_DT_ENDPOINT,
	BULK_EP_IN,
	CH9_USB_EP_BULK,
	cpuToLe16(512),
	0
};

static CH9_UsbEndpointDescriptor endpointEpOutDesc = {
	CH9_USB_DS_ENDPOINT,
	CH9_USB_DT_ENDPOINT,
	BULK_EP_OUT,
	CH9_USB_EP_BULK,
	cpuToLe16(512),
	0
};

//0x06, 0x30, 0x00, 0x00, 0x00, 0x00
static CH9_UsbSSEndpointCompanionDescriptor compDesc = {
	CH9_USB_DS_SS_USB_ENDPOINT_COMPANION,
	CH9_USB_DT_SS_USB_ENDPOINT_COMPANION,
	0,
	0,
	cpuToLe16(0)
};
//------------------- End of Super Speed configuration -------------------------

//------------- Start of High Speed configuration descriptors -----------------
// configuration descriptors for HighSpeed mode
static CH9_UsbConfigurationDescriptor confHsDesc = {
	CH9_USB_DS_CONFIGURATION,
	CH9_USB_DT_CONFIGURATION,
	cpuToLe16(
		CH9_USB_DS_CONFIGURATION
		+ CH9_USB_DS_INTERFACE
		+ 2 * CH9_USB_DS_ENDPOINT),
	1,
	1,
	0,
#if defined(TEST_RWP)
	CH9_USB_CONFIG_RESERVED | CH9_USB_CONFIG_SELF_POWERED | CH9_USB_CONFIG_REMOTE_WAKEUP,
#else
	CH9_USB_CONFIG_RESERVED | CH9_USB_CONFIG_SELF_POWERED,
#endif
	0
};

// language descriptor for english
static uint8_t languageDesc[] = {
	4, CH9_USB_DT_STRING, 0x09, 0x04
};

/*
   static uint8_t ls[2] = {0x09, 0x04};
   static CH9_UsbStringDescriptor languageDesc = {
    .bLength = 4,
    .bDescriptorType = CH9_USB_DT_STRING,
    .bString = ls
   };
 */

// string will be filled then in initializing section
static char vendorDesc[sizeof (USB_MANUFACTURER_STRING) * 2 + 2];
static char productDesc[sizeof (USB_PRODUCT_STRING) * 2 + 2];
static char serialDesc[sizeof (USB_SERIAL_NUMBER_STRING) * 2 + 2];

//-------------- BOS descriptor set start --------------------------------------

static CH9_UsbBosDescriptor bosDesc = {
	CH9_USB_DS_BOS,
	CH9_USB_DT_BOS,
	cpuToLe16(CH9_USB_DS_BOS + CH9_USB_DS_DEVICE_CAPABILITY_30 + CH9_USB_DS_DEVICE_CAPABILITY_20),
	2
};

static CH9_UsbSSDeviceCapabilityDescriptor capabilitySsDesc = {
	CH9_USB_DS_DEVICE_CAPABILITY_30,
	CH9_USB_DT_DEVICE_CAPABILITY,
	CH9_USB_DCT_SS_USB,
	0, // LTM not supported
	cpuToLe16(CH9_USB_SS_CAP_SUPPORT_SS | CH9_USB_SS_CAP_SUPPORT_HS | CH9_USB_SS_CAP_SUPPORT_FS),
	1, //<----what's that????????????????????, look in spec
	4, // 4 us
	cpuToLe16(512) // 512 us
};

static CH9_UsbCapabilityDescriptor capabilityExtDesc = {
	CH9_USB_DS_DEVICE_CAPABILITY_20,
	CH9_USB_DT_DEVICE_CAPABILITY,
	CH9_USB_DCT_USB20_EXTENSION,
	cpuToLe32(CH9_USB_USB20_EXT_LPM_SUPPORT)
};
//-------------- BOS descriptor set end ----------------------------------------

static CH9_UsbDeviceQualifierDescriptor qualifierDesc = {
	CH9_USB_DS_DEVICE_QUALIFIER,
	CH9_USB_DT_DEVICE_QUALIFIER,
	cpuToLe16(0x0200),
	0x00,
	0x00,
	0x00,
	64,
	0x01,
	0x00
};

static void reset(struct usb_gadget *gadget)
{
	NOTICE("Application: reset\n");
}

static void disconnect(struct usb_gadget *gadget)
{
	NOTICE("Application: disconnect %c\n", ' ');
	displayDeviceInfo(gadget);
	configValue = 0;
}

static void resume(struct usb_gadget *gadget)
{
	NOTICE("Application: resume %c\n", ' ');
}

static void reqComplete(struct usb_ep *ep, struct usb_request *req)
{
	VERBOSE("Request on endpoint %s completed\n", ep->name);
	//displayRequestinfo(req);

}

static int setup(struct usb_gadget *gadget,
		 const CH9_UsbSetup *ctrl)
{
	int length = 0;
	CH9_UsbSetup TmpCtrl;
	CH9_UsbConfigurationDescriptor *configDesc;
	CH9_UsbDeviceDescriptor * devDesc;

	TmpCtrl.bRequest = ctrl->bRequest;
	TmpCtrl.bmRequestType = ctrl->bmRequestType;
	TmpCtrl.wIndex = le16ToCpu(ctrl->wIndex);
	TmpCtrl.wLength = le16ToCpu(ctrl->wLength);
	TmpCtrl.wValue = le16ToCpu(ctrl->wValue);

	// select descriptors according to actual speed
	switch (gadget->speed) {

	case CH9_USB_SPEED_FULL:
		configDesc = &confHsDesc;
		devDesc = &devHsDesc;
		break;

	case CH9_USB_SPEED_HIGH:
		configDesc = &confHsDesc;
		devDesc = &devHsDesc;
		break;

	case CH9_USB_SPEED_SUPER:
		configDesc = &confSsDesc;
		devDesc = &devSsDesc;
		break;

	default:
		ERROR("Unknown speed %d: \n", gadget->speed);
		return 1;
		break;
	}

	VERBOSE("Speed %d: \n", gadget->speed);
	VERBOSE("bRequest: %02X\n", TmpCtrl.bRequest);
	VERBOSE("bRequestType: %02X\n", TmpCtrl.bmRequestType);
	VERBOSE("wIndex: %04X\n", TmpCtrl.wIndex);
	VERBOSE("wValue: %04X\n", TmpCtrl.wValue);
	VERBOSE("wLength: %04X\n", TmpCtrl.wLength);

	ep0Req->buf = ep0Buff;
	ep0Req->dma = (uintptr_t) ep0Buff;
	ep0Req->complete = reqComplete;

	switch (TmpCtrl.bmRequestType & CH9_USB_REQ_TYPE_MASK) {

	case CH9_USB_REQ_TYPE_STANDARD:

		switch (TmpCtrl.bRequest) {

		case CH9_USB_REQ_GET_DESCRIPTOR:
			VERBOSE("GET DESCRIPTOR %c\n", ' ');
			if ((TmpCtrl.bmRequestType & CH9_REQ_RECIPIENT_MASK) == CH9_USB_REQ_RECIPIENT_INTERFACE) {
				switch (TmpCtrl.wValue >> 8) {

				default:
					return -1;
				}
			} else if ((TmpCtrl.bmRequestType & CH9_REQ_RECIPIENT_MASK) == CH9_USB_REQ_RECIPIENT_DEVICE) {
				switch (TmpCtrl.wValue >> 8) {

				case CH9_USB_DT_DEVICE:
					length = CH9_USB_DS_DEVICE;
					memmove(ep0Buff, devDesc, 18);
					VERBOSE("DevDesc[0] = %d\n", devDesc->bLength);
					{
						int i;

						for (i = 0; i < length; i++) {
							bulk_debug("%02X ", ep0Buff[i]);
						}
						bulk_debug(" %c\n", ' ');
					}
					break;

				case CH9_USB_DT_CONFIGURATION: {
					int offset = 0;

					length = le16ToCpu(configDesc->wTotalLength);
					current_speed = gadget->speed;
					// select descriptors according to actual speed
					switch (gadget->speed) {

					case CH9_USB_SPEED_FULL:
						endpointEpInDesc.wMaxPacketSize = cpuToLe16(64);
						endpointEpOutDesc.wMaxPacketSize = cpuToLe16(64);
						break;

					case CH9_USB_SPEED_HIGH:
						endpointEpInDesc.wMaxPacketSize = cpuToLe16(512);
						endpointEpOutDesc.wMaxPacketSize = cpuToLe16(512);
						break;

					case CH9_USB_SPEED_SUPER:
						endpointEpInDesc.wMaxPacketSize = cpuToLe16(1024);
						endpointEpOutDesc.wMaxPacketSize = cpuToLe16(1024);
						break;
					default:
						break;
					}

					memmove(&ep0Buff[offset], configDesc, CH9_USB_DS_CONFIGURATION);
					offset += CH9_USB_DS_CONFIGURATION;

					memmove(&ep0Buff[offset], &interfaceDesc, CH9_USB_DS_INTERFACE);
					offset += CH9_USB_DS_INTERFACE;

					memmove(&ep0Buff[offset], &endpointEpInDesc, CH9_USB_DS_ENDPOINT);
					offset += CH9_USB_DS_ENDPOINT;
					if (gadget->speed == CH9_USB_SPEED_SUPER) {
						memmove(&ep0Buff[offset], &compDesc, CH9_USB_DS_SS_USB_ENDPOINT_COMPANION);
						offset += CH9_USB_DS_SS_USB_ENDPOINT_COMPANION;
					}
					memmove(&ep0Buff[offset], &endpointEpOutDesc, CH9_USB_DS_ENDPOINT);
					offset += CH9_USB_DS_ENDPOINT;

					if (gadget->speed == CH9_USB_SPEED_SUPER) {
						memmove(&ep0Buff[offset], &compDesc, CH9_USB_DS_SS_USB_ENDPOINT_COMPANION);
					}

					VERBOSE("ConfDesc[0] = %02X\n", configDesc->bLength);
					{
						int i;

						for (i = 0; i < length; i++) {
							bulk_debug("%02X ", ep0Buff[i]);
						}
						bulk_debug(" %c\n", ' ');
					}
					break;
				}

				case CH9_USB_DT_STRING: {
					uint8_t descIndex = (uint8_t) (TmpCtrl.wValue & 0xFF);
					char *strDesc;
					VERBOSE("StringDesc %c\n", ' ');
					switch (descIndex) {
					case 0:
						strDesc = (char*) &languageDesc;
						length = strDesc[0];
						VERBOSE("language %c\n", ' ');
						break;

					case 1:
						strDesc = (char*) &vendorDesc;
						length = strDesc[0];
						VERBOSE("vendor %c\n", ' ');
						break;

					case 2:
						strDesc = (char*) &productDesc;
						length = strDesc[0];
						VERBOSE("product %c\n", ' ');
						break;

					case 3:
						strDesc = (char*) &serialDesc;
						length = strDesc[0];
						VERBOSE("serial %c\n", ' ');
						break;

					default:
						return -1;
					}
					memmove(ep0Buff, strDesc, length);
					break;
				}

				case CH9_USB_DT_BOS: {
					int offset = 0;
					length = le16ToCpu(bosDesc.wTotalLength);

					memmove(ep0Buff, &bosDesc, CH9_USB_DS_BOS);
					offset += CH9_USB_DS_BOS;
					/*Only USB3 should support CH9_USB_DS_DEVICE_CAPABILITY_30 descriptor*/
					if (gadget->max_speed == CH9_USB_SPEED_SUPER || gadget->max_speed == CH9_USB_SPEED_SUPER_PLUS) {
						memmove(&ep0Buff[offset], &capabilitySsDesc, CH9_USB_DS_DEVICE_CAPABILITY_30);
						offset += CH9_USB_DS_DEVICE_CAPABILITY_30;
					}

					memmove(&ep0Buff[offset], &capabilityExtDesc, CH9_USB_DS_DEVICE_CAPABILITY_20);
				}

				VERBOSE("bosDesc[0] = %02X\n", bosDesc.bLength);
				{
					int i;

					for (i = 0; i < length; i++) {
						VERBOSE("%02X ", ep0Buff[i]);
					}
					VERBOSE(" %c\n", ' ');
				}
				VERBOSE("BosDesc %c\n", ' ');
				break;

				case CH9_USB_DT_DEVICE_QUALIFIER:
					length = CH9_USB_DS_DEVICE_QUALIFIER;
					memmove(ep0Buff, &qualifierDesc, length);
					VERBOSE("QualifierDesc %c\n", ' ');
					break;

				case CH9_USB_DT_OTHER_SPEED_CONFIGURATION: {
					int offset = 0;

					length = le16ToCpu(configDesc->wTotalLength);

					memmove(&ep0Buff[offset], configDesc, CH9_USB_DS_CONFIGURATION);
					ep0Buff[1] = CH9_USB_DS_OTHER_SPEED_CONFIGURATION;
					offset += CH9_USB_DS_CONFIGURATION;

					memmove(&ep0Buff[offset], &interfaceDesc, CH9_USB_DS_INTERFACE);
					offset += CH9_USB_DS_INTERFACE;

					memmove(&ep0Buff[offset], &endpointEpInDesc, CH9_USB_DS_ENDPOINT);
					offset += CH9_USB_DS_ENDPOINT;
					if (gadget->speed == CH9_USB_SPEED_SUPER) {
						memmove(&ep0Buff[offset], &compDesc, CH9_USB_DS_SS_USB_ENDPOINT_COMPANION);
						offset += CH9_USB_DS_SS_USB_ENDPOINT_COMPANION;
					}
					memmove(&ep0Buff[offset], &endpointEpOutDesc, CH9_USB_DS_ENDPOINT);
					offset += CH9_USB_DS_ENDPOINT;

					memmove(&ep0Buff[offset], &compDesc, CH9_USB_DS_SS_USB_ENDPOINT_COMPANION);

					VERBOSE("OtherSpeedDesc[0] = %02X\n", configDesc->bLength);
					{
						int i;

						for (i = 0; i < length; i++) {
							VERBOSE("%02X ", ep0Buff[i]);
						}
						VERBOSE(" %c\n", ' ');
					}
					break;
				}

				default:
					ERROR("Error_1 %c\n", ' ');
					return -1;

				}        //switch
			}        //if
			break;

		case CH9_USB_REQ_SET_CONFIGURATION:
		{
			struct usb_ep *ep;
			struct list_head *list;

			VERBOSE("SET CONFIGURATION(%d)\n", le16ToCpu(TmpCtrl.wValue));
			if (TmpCtrl.wValue > 1) {
				return -1;       // no such configuration
			}
			// unconfigure device
			if (TmpCtrl.wValue == 0) {
				configValue = 0;
				for (list = gadget->ep_list.next; list != &gadget->ep_list; list = list->next) {
					ep = (struct usb_ep *) list;
					if (ep->name) {
						ep->ops->disable(ep);
						//displayEndpointInfo(ep);
					}
				}
				return 0;
			}

			cmd_flag = 1;
			// device already configured
			if (configValue == 1 && TmpCtrl.wValue == 1) {
				return 0;
			}

			// configure device
			configValue = (uint8_t) TmpCtrl.wValue;
			for (list = gadget->ep_list.next; list != &gadget->ep_list; list = list->next) {
				ep = (struct usb_ep *) list;
				if (ep->name && (!strcmp(ep->name, "ep1in-bulk"))) {
					ep->ops->enable(ep, &endpointEpInDesc);
					displayEndpointInfo(ep);
					break;
				}
			}
			for (list = gadget->ep_list.next; list != &gadget->ep_list; list = list->next) {
				ep = (struct usb_ep *) list;
				if ((ep->name && !strcmp(ep->name, "ep2out-bulk"))) {
					ep->ops->enable(ep, &endpointEpOutDesc);
					displayEndpointInfo(ep);
					break;
				}
			}

			/*Code control  Self powered feature of USB*/
			if (configDesc->bmAttributes & CH9_USB_CONFIG_SELF_POWERED) {
				if (gadget->ops->set_selfpowered) {
					gadget->ops->set_selfpowered(gadget, 1);
				}
			} else {
				if (gadget->ops->set_selfpowered) {
					gadget->ops->set_selfpowered(gadget, 0);
				}
			}
		}
			return 0;

		case CH9_USB_REQ_GET_CONFIGURATION:
			length = 1;
			memmove(ep0Buff, &configValue, length);
			break;

		default:
			ERROR("Error_2 %c\n", ' ');
			return -1;         //return error
		}
		break;

	case CH9_USB_REQ_TYPE_CLASS:
		return -1;
		break;
	}

	if (length > 0) {
		ep0Req->length = TmpCtrl.wLength < length ? TmpCtrl.wLength : length;
		gadget->ep0->ops->queue(gadget->ep0, ep0Req);
	}
	return 0;
}

static void suspend(struct usb_gadget *gadget)
{
	NOTICE("Application: enter suspend\n");
#if defined(TEST_RWP)
	if (configValue) {
		NOTICE("Application: rwp after 5s\n");
		mdelay(5000);
		if (gadget->ops->wakeup)
			gadget->ops->wakeup(gadget);
	}
#endif
}

static int bind(struct usb_gadget *gadget)
{
	NOTICE("%s()\n", __func__);
	if (drv_obj.gadget) {
		NOTICE("gadget already bind\n");
		return 0;
	}
	drv_obj.gadget = gadget;

	return 0;
}

static void unbind(struct usb_gadget *gadget)
{
	NOTICE("%s()\n", __func__);
	drv_obj.gadget = NULL;
}

static void displayRequestinfo(struct usb_request *req)
{
	VERBOSE("-------Request INFO-------------- %c\n", ' ');
	VERBOSE("buf: %08lx\n", (uintptr_t) req->buf);
	VERBOSE("length: %d\n", req->length);
	VERBOSE("dma: %08lx\n", req->dma);
	VERBOSE("streamId: %04X\n", req->stream_id);
	VERBOSE("noInterrupt: %d\n", req->no_interrupt);
	VERBOSE("zero: %d\n", req->zero);
	VERBOSE("shortNotOk: %d\n", req->short_not_ok);
	VERBOSE("context: %08lx\n", (uintptr_t) req->context);
	VERBOSE("status: %08X\n", req->status);
	VERBOSE("actual: %d\n", req->actual);
}

static void displayEndpointInfo(struct usb_ep *ep)
{
	VERBOSE("-------Endpoint INFO------------- %c\n", ' ');
	VERBOSE("address: %02X\n", ep->desc->bEndpointAddress);
	VERBOSE("epList: %08lx\n", (uintptr_t)&ep->ep_list);
	VERBOSE("driverData: %08lx\n", (uintptr_t)ep->driver_data);
	//VERBOSE("name: %s\n", ep->name);
	VERBOSE("ops: %c\n", ' ');
	VERBOSE("maxPacket: %d\n", ep->maxpacket);
	VERBOSE("maxStreams: %d\n", ep->max_streams);
	VERBOSE("maxburst: %d\n", ep->maxburst);
	VERBOSE("desc: %08lx\n", (uintptr_t) ep->desc);
}

static void displayDeviceInfo(struct usb_gadget *gadget)
{

	VERBOSE("-------Device INFO--------------- %c\n", ' ');
	VERBOSE("epList: prev = %08lx, next = %08lx\n",
		(uintptr_t) gadget->ep_list.prev,
		(uintptr_t) gadget->ep_list.next);
	VERBOSE("ep0: %08lx\n", (uintptr_t) gadget->ep0);
	VERBOSE("speed: %d\n", gadget->speed);
	VERBOSE("maxSpeed: %d\n", gadget->max_speed);
	VERBOSE("state: %d\n", gadget->state);
	VERBOSE("isOtg: %d\n", gadget->is_otg);
	VERBOSE("isAPeripheral: %d\n", gadget->is_a_peripheral);
	VERBOSE("bHnpEnable: %d\n", gadget->b_hnp_enable);
	VERBOSE("aHnpSupport: %d\n", gadget->a_alt_hnp_support);
}

static void * requestMemAlloc(struct usb_gadget *gadget, uint32_t requireSize)
{
	return malloc(requireSize);
}

static void requestMemFree(struct usb_gadget *gadget, void *usbRequest)
{
	if (usbRequest) {
		free(usbRequest);
	}
}

static struct usb_gadget_driver g_driver = {
	.function	= "BULK",
	.speed		= CH9_USB_SPEED_HIGH,
	.bind		= bind,
	.unbind		= unbind,
	.setup		= setup,
	.reset		= reset,
	.disconnect	= disconnect,
	.suspend	= suspend,
	.resume		= resume,
	.req_mem_alloc	= requestMemAlloc,
	.req_mem_free	= requestMemFree,
};

/*
 *
 */
int bulkAppLB()
{
	struct usb_gadget *gadget;
	uint32_t res; // keeps result of operation on driver
	uint32_t transfer_size = 0;
	struct list_head *list; // used in for_each loop
	struct usb_ep *ep;
	struct usb_ep *ep0;

	drv_obj.plat.handler = (void *)handler;
	drv_obj.plat.size = HANDLER_SIZE;
	drv_obj.plat.ctrl_req = (void *)alignBuff(setupBufAlloc, 32);
	res = dwc2_udc_probe(&drv_obj.plat);
	if (res != 0) {
		goto error;
	}

	if (usb_gadget_register_driver(&g_driver) < 0) {
		NOTICE("Gadget Register Fail\n");
		goto error;
	}
	gadget = drv_obj.gadget;
	ep0 = gadget->ep0;
	if (!gadget) {
		NOTICE("Gadget object not existed!\n");
		goto error;
	}

	NOTICE("Initializing OK! %s\n", " ");
	// set unicode strings
	get_unicode_string(vendorDesc, USB_MANUFACTURER_STRING);
	get_unicode_string(productDesc, USB_PRODUCT_STRING);
	get_unicode_string(serialDesc, USB_SERIAL_NUMBER_STRING);

	// align buffers for bulk-echo application
	bulkBuf = alignBuff(bulkBufAlloc, 32);
	cmdBuf = alignBuff(cmdBufAlloc, 32);
	ep0Buff = alignBuff(ep0BuffAlloc, 32);
	memset(ep0BuffAlloc, 0x00, BUF_SIZE_EP0);
	//memset(bulkBufAlloc, 0x00, BUF_SIZE_BULK);
	memset(cmdBufAlloc, 0x00, BUF_SIZE_CMD);

	// checking device and endpoints parameters correctness
	// get CUSBD device instance exposed as Linux gadget device
	displayDeviceInfo(gadget);

	// allocate request for ep0
	ep0Req = ep0->ops->alloc_request(gadget->ep0);

	/*Change descriptor for max_speed == HS only Device*/
	/*For USB2.0 we have to modified wTotalLength of BOS descriptor*/
	if(gadget->max_speed == CH9_USB_SPEED_HIGH || gadget->max_speed == CH9_USB_SPEED_FULL) {
		bosDesc.wTotalLength = cpuToLe16(CH9_USB_DS_BOS + CH9_USB_DS_DEVICE_CAPABILITY_20);
		bosDesc.bNumDeviceCaps = 1;
		devHsDesc.bcdUSB = cpuToLe16(BCD_USB_HS_ONLY);
	}

	gadget->ops->pullup(gadget, 1);
unconfigured:
	while (!configValue) {
#if !defined(USB_IRQ_MODE)
		bulkIsrLB();
#endif
		bulk_debug("Wait for device has been configured!\n");
	}

#if defined(USB_IRQ_MODE)
	disable_int();
#endif
	// find bulk endpoint
	for (list = gadget->ep_list.next; list != &gadget->ep_list; list = list->next) {
		ep = (struct usb_ep  *)list;
		if (ep->desc && (ep->desc->bEndpointAddress == BULK_EP_IN)) {
			bulkInReq = ep->ops->alloc_request(ep);
			epIn = ep;
		} else if (ep->desc && (ep->desc->bEndpointAddress == BULK_EP_OUT)) {
			bulkOutReq = ep->ops->alloc_request(ep);
			epOut = ep;
		}
	}

	current_speed = gadget->speed;
	switch (current_speed) {
	case CH9_USB_SPEED_FULL:
		transfer_size = 64;
		break;
	case CH9_USB_SPEED_HIGH:
		transfer_size = 512;
		break;
	case CH9_USB_SPEED_SUPER:
		transfer_size = 1024;
		break;
	default:
		bulk_debug("Test error\n");
		return -1;
	}

	bulk_debug("OUT DATA TRANSFER\n");
	clearReq(bulkOutReq);
	bulkOutReq->buf = cmdBuf;
	bulkOutReq->dma = (uintptr_t) cmdBuf;
	bulkOutReq->complete = bulkOutCmpl;
	bulkOutReq->length = transfer_size;

	bulk_debug("IN DATA TRANSFER\n");
	clearReq(bulkInReq);
	bulkInReq->buf = bulkBuf;
	bulkInReq->dma = (uintptr_t) bulkBuf;
	bulkInReq->complete = bulkInCmpl;
	bulkInReq->length = transfer_size;
	cmd_flag = 1;
	epOut->ops->queue(epOut, bulkOutReq);
#if defined(USB_IRQ_MODE)
	enable_int();
#endif

	while (1) {
		//bulk_debug("Device transfers data\n");
#if !defined(USB_IRQ_MODE)
		bulkIsrLB();
#endif
		if (!configValue) {
			goto unconfigured;
		}
	}

	usb_gadget_unregister_driver(&g_driver);
	NOTICE("USB stop\n");
	return (0);

error:
	ERROR("Error %08X\n", res);
	return (res);
}
