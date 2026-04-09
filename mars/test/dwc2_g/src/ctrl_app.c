/**********************************************************************
 * Copyright (C) 2014-2017 Cadence Design Systems, Inc.
 * All rights reserved worldwide.
 ***********************************************************************
 * ctrl_app.c
 * Control echo application.
 *
 ***********************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "debug.h"
#include <system_common.h>
#include <byteorder.h>
#include <dwc2_ch9.h>
#include "dwc2_drv_if.h"
#include "dwc2_udc_otg_regs.h"
#include "dwc2_udc_otg_priv.h"
#include <dwc2_udc.h>

#include "map_system_memory.h"
//#define USE_SRAM
//#define bulk_debug printf
#define bulk_debug(...)
#define EP0_DATA_BUFFER_SIZE 0x200
#define DBG_USB_CTRL_MSG 0x00000002

#define CTRL_TEST_64P0			255	// send 64+0 bytes
#define CTRL_TEST_ZERO			239	// send zero-length data
#define CTRL_TEST_STALL_STATUS		138	// stall status phase
#define CTRL_TEST_STALL_IN		137	// stall data in
#define CTRL_TEST_STALL_OUT		136	// stall data out

extern void enable_int(void);
extern void disable_int(void);

// dwc2 USB driver object
static struct dwc2_drv_obj drv_obj = {
	.plat = {
		.regs_otg = USB_BASE, // address where USB core is mapped
		.usb_gusbcfg    = 0x40081408,
		.rx_fifo_sz     = 1050,
		.np_tx_fifo_sz  = 32,
		.tx_fifo_sz     = 512,
	},
};

// interrupt handler

void ctrlIsr()
{
	usb_gadget_handle_interrupts(0);
}

//-------------- definitions used during configuration -------------------------

#define BCD_USB_SS                      0x0300  // 3.00 version USB
#define ID_VENDOR                       0x0559  // Cadence

#ifdef HS_FS_ONLY
#define ID_PRODUCT                      0x1004  // example bulk product
#else
#define ID_PRODUCT                      0x4000  // example bulk product
#endif

#define BCD_DEVICE_SS                   0x0001  // 0.1

#define BCD_USB_HS_ONLY                 0x0201  // 2.01  /*Only HS with BOS descriptor*/
#define BCD_USB_HS                      0x0210  // 2.10

#define BCD_DEVICE_HS                   0x0200  // 2.00

#define USB_MANUFACTURER_STRING         "CADENCE"
#define USB_PRODUCT_STRING              "Vendor spec example device"
#define USB_SERIAL_NUMBER_STRING        "100000000000" // should 12 chars long

#define HANDLER_SIZE			1024
static uint8_t ep0BuffAlloc[EP0_DATA_BUFFER_SIZE + 31];
static uint8_t setupBufAlloc[8 + 31];
static uint8_t handler[HANDLER_SIZE];
static uint8_t *ep0Buff;               // keeps mouse report
static struct usb_request *ep0Req;       // request used by driver
static uint8_t configValue = 0;

// debug functions
static void displayRequestinfo(struct usb_request *req);
static void displayEndpointInfo(struct usb_ep *ep);
static void displayDeviceInfo(struct usb_gadget *gadget);

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
		+ 2 * CH9_USB_DS_INTERFACE
		+ 2 * CH9_USB_DS_ENDPOINT
		+ 2 * CH9_USB_DS_SS_USB_ENDPOINT_COMPANION),
	2,
	1,
	0,
	CH9_USB_CONFIG_RESERVED | CH9_USB_CONFIG_SELF_POWERED,
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

static CH9_UsbInterfaceDescriptor interfaceEPLessDesc = {
	CH9_USB_DS_INTERFACE,
	CH9_USB_DT_INTERFACE,
	1,
	0,
	0,
	CH9_USB_CLASS_VENDOR,
	0,
	0,
	0
};

static CH9_UsbEndpointDescriptor endpointEpInDesc = {
	CH9_USB_DS_ENDPOINT,
	CH9_USB_DT_ENDPOINT,
	0x81,
	CH9_USB_EP_BULK,
	cpuToLe16(512),
	0
};

static CH9_UsbEndpointDescriptor endpointEpOutDesc = {
	CH9_USB_DS_ENDPOINT,
	CH9_USB_DT_ENDPOINT,
	0x01,
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
		+ 2 * CH9_USB_DS_INTERFACE
		+ 2 * CH9_USB_DS_ENDPOINT),
	2,
	1,
	0,
	CH9_USB_CONFIG_RESERVED | CH9_USB_CONFIG_SELF_POWERED,
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

static int sendZeroLen = 0;

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
#if 0
	if (sendZeroLen) {
		ep0Req->length = 0;
		sendZeroLen = 0;
		ep->ops->queue(ep, ep0Req);
	}
#endif
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
	ep0Req->zero = 0;

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
							VERBOSE("%02X ", ep0Buff[i]);
						}
						VERBOSE(" %c\n", ' ');
					}
					break;

				case CH9_USB_DT_CONFIGURATION: {
					int offset = 0;

					length = le16ToCpu(configDesc->wTotalLength);
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
						offset += CH9_USB_DS_SS_USB_ENDPOINT_COMPANION;
					}

					memmove(&ep0Buff[offset], &interfaceEPLessDesc, CH9_USB_DS_INTERFACE);

					VERBOSE("ConfDesc[0] = %02X\n", configDesc->bLength);
					{
						int i;

						for (i = 0; i < length; i++) {
							VERBOSE("%02X ", ep0Buff[i]);
						}
						VERBOSE(" %c\n", ' ');
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
					VERBOSE("Error_1 %c\n", ' ');
					return -1;

				}        //switch
			}        //if
			break;

		case CH9_USB_REQ_SET_CONFIGURATION: {
			VERBOSE("SET CONFIGURATION(%d)\n", le16ToCpu(TmpCtrl.wValue));
			VERBOSE("SET CONFIGURATION(%d)\n", le16ToCpu(TmpCtrl.wValue));
			if (TmpCtrl.wValue > 1) {
				return -1;       // no such configuration
			}
			// device already configured
			if (configValue == 1 && TmpCtrl.wValue == 1) {
				return 0;
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

	case CH9_USB_REQ_TYPE_VENDOR:

		switch (TmpCtrl.bmRequestType & CH9_REQ_RECIPIENT_MASK) {

		case CH9_USB_REQ_RECIPIENT_INTERFACE:

			switch (TmpCtrl.wIndex) {

			case 1:
				switch (TmpCtrl.bmRequestType & CH9_USB_EP_DIR_MASK) {

				case CH9_USB_DIR_HOST_TO_DEVICE:
					VERBOSE("RECEIVED %d bytes on EP0\n", length);
					length = TmpCtrl.wLength;
					// stall Out
					if (length == CTRL_TEST_STALL_OUT) {
						VERBOSE("stall ep0 out!!!!\n");
						return -1;
					}
					break;

				case CH9_USB_DIR_DEVICE_TO_HOST:
					VERBOSE("SEND %d bytes on EP0\n", length);
					length = TmpCtrl.wLength;
					// return 0-length data
					if (length == CTRL_TEST_ZERO) {
						length = 0;
					} else if (length == CTRL_TEST_64P0) {
						// return 64B+0B data
						sendZeroLen = 1;
						length = 64;
						ep0Req->zero = 1;
					} else if (length == CTRL_TEST_STALL_IN) {
						// stall In
						return -1;
					}

					break;

				default:
					return -1;
					break;
				}

				break;

			default:
				// stall status will enter this!
				return -1;
				break;
			}

			break;

		default:
			return -1;
			break;
		}

		break;

	case CH9_USB_REQ_TYPE_CLASS:
		return -1;
		break;
	}

	if (TmpCtrl.wLength && length >= 0) {
		ep0Req->length = TmpCtrl.wLength < length ? TmpCtrl.wLength : length;
		gadget->ep0->ops->queue(gadget->ep0, ep0Req);
	}
	return 0;
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
	VERBOSE("address: %02X\n", ep->desc->address);
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
	//return 0x0C000000;
}

static void requestMemFree(struct usb_gadget *gadget, void *usbRequest)
{
	if (usbRequest) {
		free(usbRequest);
	}
}

static int bind(struct usb_gadget *gadget)
{
	NOTICE("%s()\n", __func__);
	if (drv_obj.gadget) {
		ERROR("gadget already bind\n");
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

static void suspend(struct usb_gadget *gadget)
{
	NOTICE("Application: suspend %c\n", ' ');
}

static struct usb_gadget_driver g_driver = {
	.function	= "CTRL",
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
int ctrlApp()
{
	struct usb_gadget *gadget;
	struct usb_ep *ep0;
	uint32_t res; // keeps result of operation on driver

	NOTICE("ctrl app start %s\n", " ");
#if defined(USB_IRQ_MODE)
	disable_int();
#endif
	drv_obj.plat.handler = (void *)handler;
	drv_obj.plat.size = HANDLER_SIZE;
#if defined(USE_SRAM)
	drv_obj.plat.ctrl_req = (void *)0x0C000000;
#else
	drv_obj.plat.ctrl_req = (void *)alignBuff(setupBufAlloc, 32);
#endif
	NOTICE("ctrl request %p\n", drv_obj.plat.ctrl_req);
	res = dwc2_udc_probe(&drv_obj.plat);
	if (res != 0) {
		goto error;
	}
	NOTICE("gadget regsiter %s\n", " ");

	// bind the gadget object here.
	if (usb_gadget_register_driver(&g_driver) < 0) {
		ERROR("Gadget Register Fail\n");
		goto error;
	}
	gadget = drv_obj.gadget;
	ep0 = gadget->ep0;
	if (!gadget) {
		ERROR("Gadget object not existed!\n");
		goto error;
	}

	NOTICE("Initializing OK! %s\n", " ");
	// set unicode strings
	get_unicode_string(vendorDesc, USB_MANUFACTURER_STRING);
	get_unicode_string(productDesc, USB_PRODUCT_STRING);
	get_unicode_string(serialDesc, USB_SERIAL_NUMBER_STRING);

#if defined(USE_SRAM)
	ep0Buff = (void*)0x0C000100;
#else
	ep0Buff = alignBuff(ep0BuffAlloc, 32);
#endif
	memset(ep0BuffAlloc, 0, sizeof (ep0BuffAlloc));

	// checking device and endpoints parameters correctness
	// get CUSBD device instance exposed as Linux gadget device
	displayDeviceInfo(gadget);

	// allocate request for ep0
	ep0Req = ep0->ops->alloc_request(ep0);
	if (ep0Req == NULL) {
		return -1;
	}

	/*Change descriptor for maxSpeed == HS only Device*/
	/*For USB2.0 we have to modified wTotalLength of BOS descriptor*/
	if(gadget->max_speed == CH9_USB_SPEED_HIGH || gadget->max_speed == CH9_USB_SPEED_FULL) {
		bosDesc.wTotalLength = cpuToLe16(CH9_USB_DS_BOS + CH9_USB_DS_DEVICE_CAPABILITY_20);
		bosDesc.bNumDeviceCaps = 1;
		devHsDesc.bcdUSB = cpuToLe16(BCD_USB_HS_ONLY);
	}
#if defined(USB_IRQ_MODE)
	enable_int();
#endif

	gadget->ops->pullup(gadget, 1);
	while (1) {
#if !defined(USB_IRQ_MODE)
		ctrlIsr();
#endif
		; // stop main program here, all activity on interrupt level
	}
	usb_gadget_unregister_driver(&g_driver);
	NOTICE("USB stop\n");
	return (0);
error:
	ERROR("Error %08X\n", res);
	// free allocated memory
	return (res);
}
