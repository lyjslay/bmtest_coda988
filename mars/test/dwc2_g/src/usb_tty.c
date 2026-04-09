/**********************************************************************
 *
 * usb_tty.c
 * ACM class application.
 *
 ***********************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include <crc16.h>
#include "bm_private.h"
#include "debug.h"
#include "cvi_usb.h"
#include "mmio.h"
#include "map_system_memory.h"
#ifdef BUILD_ATF
#include "delay_timer.h"
#endif // BUILD_ATF
#include "usb_tty.h"
#include <system_common.h>
#include <dwc2_ch9.h>
#include <dwc2_drv_if.h>
#include "dwc2_udc_otg_regs.h"
#include "dwc2_udc_otg_priv.h"
#include <dwc2_udc.h>
#include <dwc2_errno.h>

#define USE_MALLOC 0

#if USE_MALLOC
#include <malloc.h>
#endif

#ifdef BUILD_ATF
extern uint16_t bm_usb_vid;
#else
uint16_t bm_usb_vid = 0x30B1;
#endif

// dwc2 USB driver object
static struct dwc2_drv_obj drv_obj = {
	.plat = {
		.regs_otg = USB_BASE, // address where USB core is mapped
		.usb_gusbcfg    = 0x40081408,
		.rx_fifo_sz     = 512,
		.np_tx_fifo_sz  = 512,
		.tx_fifo_sz     = 512,
	},
};

//variable declare
static uint8_t* bulkBuf, * cmdBuf, *ep0Buff;
static struct   usb_ep *epIn, *epOut, *epInNotify;
// request used by driver
static struct   usb_request *bulkInReq, *bulkOutReq, *ep0Req, *IntInReq;
static uint8_t  configValue = 0;
static uint8_t  configBreak = 0;
static uint8_t  acm_configValue = 0;
static uint8_t  current_speed = CH9_USB_SPEED_UNKNOWN;
static uint8_t  mem_alloc_cnt = 0;
static uint32_t transfer_size = 0;
static uint8_t  flagEnterDL = 0;
static uint8_t  flagReboot = 0;
struct f_acm *acm;

static uint8_t *bulkBufAlloc = (uint8_t *)BLK_BUF_ADDR; // 512
static uint8_t *cmdBufAlloc  = (uint8_t *)CMD_BUF_ADDR; // 512
static uint8_t *cb0_buf      = (uint8_t *)CB0_BUF_ADDR; // 128
static uint8_t *cb1_buf      = (uint8_t *)CB1_BUF_ADDR; // 128
static uint8_t *cb2_buf      = (uint8_t *)CB2_BUF_ADDR; // 64
static uint8_t *ep0BuffAlloc = (uint8_t *)EP0_BUF_ADDR; // 32
static uint8_t *rsp_buf      = (uint8_t *)RSP_BUF_ADDR; // 128
static uint8_t *acm_buf      = (uint8_t *)ACM_BUF_ADDR; // 128
static uint8_t *setup_buf    = (uint8_t *)STP_BUF_ADDR; // 32
static uint8_t *handler      = (uint8_t *)HANDLER_ADDR; // 1024

// string will be filled then in initializing section
static char vendorDesc[sizeof (USB_MANUFACTURER_STRING) * 2 + 2];
static char productDesc[sizeof (USB_PRODUCT_STRING) * 2 + 2];
static char serialDesc[sizeof (USB_SERIAL_NUMBER_STRING) * 2 + 2];

typedef void func(void);

static void init_param(void)
{
	bulkBuf = NULL;
	cmdBuf = NULL;
	ep0Buff = NULL;
	epIn = NULL;
	epOut = NULL;
	epInNotify = NULL;
	bulkInReq = NULL;
	bulkOutReq = NULL;
	ep0Req = NULL;
	IntInReq = NULL;
	configValue = 0;
	configBreak = 0;
	acm_configValue = 0;
	current_speed = CH9_USB_SPEED_UNKNOWN;
	mem_alloc_cnt = 0;
	transfer_size = 0;
	flagEnterDL = 0;
	acm = NULL;
	flagReboot = 0;
}

void print_buf_addr(void)
{
	INFO ("bulkBufAlloc: %p\n", bulkBufAlloc);
	INFO ("cmdBufAlloc: %p\n", cmdBufAlloc);
	INFO ("cb0_buf: %p\n", cb0_buf);
	INFO ("cb1_buf: %p\n", cb1_buf);
	INFO ("cb2_buf: %p\n", cb2_buf);
	INFO ("ep0BuffAlloc: %p\n", ep0BuffAlloc);
	INFO ("rsp_buf: %p\n", rsp_buf);
	INFO ("acm_buf: %p\n", acm_buf);
	INFO ("setup_buf: %p\n", setup_buf);
	INFO ("handler: %p\n", handler);
}

// interrupt handler
void AcmIsr()
{
	usb_gadget_handle_interrupts(0);
}

static int getDescAcm(CH9_UsbSpeed speed, uint8_t *acmDesc)
{
	int i = 0;
	void * desc;
	int sum = 0;
	void*(*tab)[];

	switch (speed) {

	case CH9_USB_SPEED_FULL:
		tab = &descriptorsFs;
		break;
	case CH9_USB_SPEED_HIGH:
		tab = &descriptorsHs;
		break;

	default:
		return -1;
	}

	desc = (*tab)[i];

	while (desc) {
		int length = *(uint8_t*) desc;
		VERBOSE("acm get length %d\n", length);
		memcpy(&acmDesc[sum], desc, length);
		sum += length;
		desc = (*tab)[++i];
	}
	////VERBOSE("acm get sum:%d\n", sum);
	return sum;
}

static void clearReq(struct usb_request *req)
{
	memset(req, 0, sizeof(*req));
}

static void reset(struct usb_gadget *gadget)
{
	INFO("Application: reset\n");
}

static void disconnect(struct usb_gadget *gadget)
{
	acm_configValue = 0;
	mem_alloc_cnt = 1;
	configValue = 0;
	INFO("Application: disconnect\n");
}

static void resume(struct usb_gadget *gadget)
{
	VERBOSE("Application: resume\n");
}

static void reqComplete(struct usb_ep *ep, struct usb_request *req)
{
	VERBOSE("Request on endpoint completed\n");
	if (req->status == -EIO) {
		NOTICE("IO Abort !!!!!\n");
		flagReboot = 1;
	}
}

static void suspend(struct usb_gadget *gadget)
{
	VERBOSE("Application: suspend %c\n", ' ');
}

static void * requestMemAlloc(struct usb_gadget *gadget, uint32_t requireSize)
{
	void *ptr;
	// VERBOSE("requestMemAlloc: size %d\n", requireSize);
	if (mem_alloc_cnt == 0) {
		ptr = cb0_buf;
	} else if (mem_alloc_cnt == 1) {
		ptr = cb1_buf;
	} else {
		ptr = cb2_buf;
	}
	VERBOSE("requestMemAlloc: ptr %p, size %d, mem_alloc_cnt %d\n", ptr, requireSize, mem_alloc_cnt);
	mem_alloc_cnt++;
	return ptr;
}

static void requestMemFree(struct usb_gadget *gadget, void *usbRequest)
{
	return;
}

static void bulkInCmpl(struct usb_ep *ep, struct usb_request *req)
{
	if (req->status == -ESHUTDOWN)
		return;

	memset(cmdBuf, 0x00, HEADER_SIZE); //next time init
	bulkOutReq->length = transfer_size;
	bulkOutReq->buf = cmdBuf;
	bulkOutReq->dma = (uintptr_t) cmdBuf;
	INFO ("epOut->ops->queue\n");
	epOut->ops->queue(epOut, bulkOutReq);
}

static void bulkOutCmpl(struct usb_ep *ep, struct usb_request *req)
{
	static uint8_t ack_idx = 0;
	uint32_t i;
	void *dest_addr = 0x0;
	func *jump_fun;
	uint16_t crc;

	if (req->status == -ESHUTDOWN)
		return;

	CVI_USB_TOKEN token  = ((uint8_t*) req->buf)[0];
	uint32_t length = (((uint8_t*) req->buf)[1] << 8) + \
			  (((uint8_t*) req->buf)[2]);

	dest_addr = (void *)(((uint64_t)(((uint8_t*) req->buf)[3]) << 32) + \
			     ((uint64_t)(((uint8_t*) req->buf)[4]) << 24) + \
			     ((uint64_t)(((uint8_t*) req->buf)[5]) << 16) + \
			     ((uint64_t)(((uint8_t*) req->buf)[6]) << 8)  + \
			     ((uint64_t)(((uint8_t*) req->buf)[7])));
	jump_fun = (func*)dest_addr;
	// VERBOSE("Transfer complete on ep:%02X %08X req\n", ep->address, (uintptr_t) req);

	if (length == 0 && dest_addr == 0) {
		bulkOutReq->length = transfer_size;
		bulkOutReq->buf = cmdBuf;
		bulkOutReq->dma = (uintptr_t) cmdBuf;
		VERBOSE ("buffer zero\n");
		epOut->ops->queue(epOut, bulkOutReq);
		return;
	}

	crc = crc16_ccitt(0, cmdBuf, length);
	INFO ("CRC: %x\n", crc);

	switch (token) {
	case CVI_USB_VERBOSE:
		for (i=0; i<length; i++) {
			NOTICE ("cmdBuf[%d] = %x\n", i, cmdBuf[i]);
		}
	case CVI_USB_INFO:
		NOTICE ("LENGTH: %d\n", length);
		NOTICE ("DEST:   %p\n", dest_addr);
		NOTICE ("CRC:    %x\n", crc);
	case CVI_USB_NONE:
		memcpy (dest_addr, cmdBuf + HEADER_SIZE, length - HEADER_SIZE);
		break;

	case CVI_USB_JUMP:
		NOTICE ("CVI_USB_JUMP to %p\n", dest_addr);
		jump_fun();
		NOTICE ("CVI_USB_JUMP back\n");
		break;

	case CVI_USB_BREAK:
		NOTICE ("CVI_USB_BREAK\n");
		ack_idx = 0;
		configBreak = 1;
		break;

	case CVI_USB_KEEP_DL:
		NOTICE ("CVI_USB_KEEP_DL\n");
		if (crc == 0xB353) {
			ack_idx = 0;
			flagEnterDL = 1;
			NOTICE ("flagEnterDL %d\n", flagEnterDL);
		} else {
			flagEnterDL = 0;
			NOTICE ("MAGIC NUM NOT MATCH\n");
			NOTICE ("flagEnterDL %d\n", flagEnterDL);
		}
		break;

	case CVI_USB_PRG_CMD:
		NOTICE ("CVI_USB_PRG_CMD\n");
		for (i=0; i<length; i++) {
			NOTICE ("cmdBuf[%d] = %x\n", i, cmdBuf[i]);
		}
		break;

	default:
		break;
	}

	rsp_buf[0] = 0;
	rsp_buf[1] = 0;
	rsp_buf[2] = (crc >> 8) & 0xFF;
	rsp_buf[3] = crc & 0xFF;
	rsp_buf[4] = 0;
	rsp_buf[5] = 0;
	rsp_buf[6] = token;
	rsp_buf[7] = ack_idx;
	ack_idx++;

	clearReq(bulkInReq);
	bulkInReq->length = RSP_SIZE;
	bulkInReq->buf = rsp_buf;
	bulkInReq->dma = (uintptr_t) rsp_buf;
	bulkInReq->complete = bulkInCmpl;
	INFO ("rsp_buf epIn->ops->queue\n");
	epIn->ops->queue(epIn, bulkInReq);

}

// functions aligns buffer to alignValue modulo address

static uint8_t *alignBuff(void *notAllignedBuff, uint8_t alignValue)
{

	uint8_t offset = ((uintptr_t) notAllignedBuff) % alignValue;
	if (offset == 0) {
		return (uint8_t*) notAllignedBuff;
	} else {
		uint8_t * ret_address = &(((uint8_t*) notAllignedBuff)[alignValue - offset]);
		return ret_address;
	}
}

/* ACM control ... data handling is delegated to tty library code.
 * The main task of this function is to activate and deactivate
 * that code based on device state; track parameters like line
 * speed, handshake state, and so on; and issue notifications.
 */

static void acm_complete_set_line_coding(struct usb_ep *ep, struct usb_request *req)
{
	struct usb_cdc_line_coding  *value = req->buf;

	//VERBOSE("req buf : %d, %d, %d, %d\n", value->dwDTERate, value->bCharFormat,value->bParityType,value->bDataBits);
	acm->port_line_coding = *value;
	VERBOSE("acm data transfer complete\n");
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

static int setup(struct usb_gadget *gadget,
		 const CH9_UsbSetup *ctrl)
{
	int length = 0;
	CH9_UsbDeviceDescriptor * devDesc;
	CH9_UsbEndpointDescriptor *endpointEpInDesc, *endpointEpOutDesc, *endpointEpInDesc2;
	CH9_UsbSetup TmpCtrl;

	TmpCtrl.bRequest = ctrl->bRequest;
	TmpCtrl.bmRequestType = ctrl->bmRequestType;
	TmpCtrl.wIndex = le16ToCpu(ctrl->wIndex);
	TmpCtrl.wLength = le16ToCpu(ctrl->wLength);
	TmpCtrl.wValue = le16ToCpu(ctrl->wValue);

	VERBOSE("Speed %d: \n", gadget->speed);
	VERBOSE("bRequest: %02X\n", TmpCtrl.bRequest);
	VERBOSE("bRequestType: %02X\n", TmpCtrl.bmRequestType);
	VERBOSE("wIndex: %04X\n", TmpCtrl.wIndex);
	VERBOSE("wValue: %04X\n", TmpCtrl.wValue);
	VERBOSE("wLength: %04X\n", TmpCtrl.wLength);

	ep0Req->buf = ep0Buff;
	ep0Req->dma = (uintptr_t) ep0Buff;
	ep0Req->complete = reqComplete;

	switch (gadget->speed) {

	case CH9_USB_SPEED_FULL:
		endpointEpInDesc = &acm_fs_in_desc;
		endpointEpOutDesc = &acm_fs_out_desc;
		endpointEpInDesc2 = &acm_fs_notify_desc;
		devDesc = &devHsDesc;
		break;

	case CH9_USB_SPEED_HIGH:
		endpointEpInDesc = &acm_hs_in_desc;
		endpointEpOutDesc = &acm_hs_out_desc;
		endpointEpInDesc2 = &acm_fs_notify_desc;
		devDesc = &devHsDesc;
		break;

	default:
		VERBOSE("Unknown speed %d: \n", gadget->speed);
		return 1;
		break;
	}

	switch (TmpCtrl.bmRequestType & CH9_USB_REQ_TYPE_MASK) {

	case CH9_USB_REQ_TYPE_STANDARD:

		switch (TmpCtrl.bRequest) {

		case CH9_USB_REQ_GET_DESCRIPTOR:
			VERBOSE("GET DESCRIPTOR %c\n", ' ');
			if ((TmpCtrl.bmRequestType & CH9_REQ_RECIPIENT_MASK) == CH9_USB_REQ_RECIPIENT_INTERFACE) {
				NOTICE("recipient target cannot be interface!\n");
				return -1;
			}
			if ((TmpCtrl.bmRequestType & CH9_REQ_RECIPIENT_MASK) == CH9_USB_REQ_RECIPIENT_DEVICE) {
				switch (TmpCtrl.wValue >> 8) {

				case CH9_USB_DT_DEVICE:
					length = CH9_USB_DS_DEVICE;
					if (bm_usb_vid != 0) {
						NOTICE ("Patch VID %x\n", bm_usb_vid);
						devDesc->idVendor = cpuToLe16(bm_usb_vid);
					}
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
					uint16_t acmDescLen = (uint16_t)getDescAcm(gadget->speed, &ep0Buff[CH9_USB_DS_CONFIGURATION]);

					length = le16ToCpu(acmDescLen + CH9_USB_DS_CONFIGURATION);
					ConfDesc.wTotalLength = cpuToLe16(length);
					memmove(ep0Buff, &ConfDesc, CH9_USB_DS_CONFIGURATION);
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
					memmove(&ep0Buff[offset], &capabilityExtDesc, CH9_USB_DS_DEVICE_CAPABILITY_20);
				}
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
					break;

				case CH9_USB_DT_OTHER_SPEED_CONFIGURATION: {
					uint16_t acmDescLen = (uint16_t)getDescAcm(gadget->speed, &ep0Buff[CH9_USB_DS_CONFIGURATION]);

					length = le16ToCpu(acmDescLen + CH9_USB_DS_CONFIGURATION);
					ConfDesc.wTotalLength = cpuToLe16(length);
					memmove(ep0Buff, &ConfDesc, CH9_USB_DS_CONFIGURATION);
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
					return -1;

				}        //switch
			}        //if
			break;

		case CH9_USB_REQ_SET_CONFIGURATION: {
			struct usb_ep * ep;
			struct list_head *list;

			VERBOSE("SET CONFIGURATION(%d)\n", le16ToCpu(Ctrl->wValue));
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
					}
				}
				return 0;
			}

			// device already configured
			if (configValue == 1 && TmpCtrl.wValue == 1) {
				return 0;
			}

			// configure device
			configValue = (uint8_t) TmpCtrl.wValue;
			for (list = gadget->ep_list.next; list != &gadget->ep_list; list = list->next) {
				ep = (struct usb_ep *) list;
				if (ep->name && (!strcmp(ep->name, "ep1in-bulk"))) {
					ep->ops->enable(ep, endpointEpInDesc);
					break;
				}
			}
			for (list = gadget->ep_list.next; list != &gadget->ep_list; list = list->next) {
				ep = (struct usb_ep *) list;
				if ((ep->name && !strcmp(ep->name, "ep2out-bulk"))) {
					ep->ops->enable(ep, endpointEpOutDesc);
					break;
				}
			}
			for (list = gadget->ep_list.next; list != &gadget->ep_list; list = list->next) {
				ep = (struct usb_ep *) list;
				if ((ep->name && !strcmp(ep->name, "ep3in-int"))) {
					ep->ops->enable(ep, endpointEpInDesc2);
					break;
				}
			}

			/*Code control  Self powered feature of USB*/
			if (ConfDesc.bmAttributes & CH9_USB_CONFIG_SELF_POWERED) {
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
			//VERBOSE("CH9_USB_REQ_GET_CONFIGURATION %c\n", ' ');
			break;

		default:
			return -1;         //return error
		}
		break;

	case CH9_USB_REQ_TYPE_CLASS: {
		/* SET_LINE_CODING ... just read and save what the host sends */
		switch(TmpCtrl.bRequest) {
		case USB_CDC_REQ_SET_LINE_CODING:
			length = TmpCtrl.wLength;
			ep0Req->complete = acm_complete_set_line_coding;
			VERBOSE("USB_CDC_REQ_SET_LINE_CODING %d\n", length);
			acm_configValue = 1;
			break;
		case USB_CDC_REQ_SET_CONTROL_LINE_STATE:
			acm->port_handshake_bits = TmpCtrl.wValue;
			acm_configValue = 1;
			VERBOSE("USB_CDC_REQ_SET_CONTROL_LINE_STATE %c\n", ' ');
			break;
		case USB_CDC_REQ_GET_LINE_CODING:
			length = TmpCtrl.wLength;
			memmove(ep0Buff, &acm->port_line_coding, length);
			//ep0Req->complete = acm_complete_get_line_coding;
			VERBOSE("USB_CDC_REQ_GET_LINE_CODING %d\n", length);
			acm_configValue = 1;
			break;
		}
		break;
	}
	}

	if (length > 0) {
		ep0Req->length = TmpCtrl.wLength < length ? TmpCtrl.wLength : length;
		gadget->ep0->ops->queue(gadget->ep0, ep0Req);
	}
	return 0;
}

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

static struct usb_gadget_driver g_driver = {
	.function	= "TTY",
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

int acm_app_init()
{
	struct usb_ep *ep0 = drv_obj.gadget->ep0;

	// set unicode strings
	get_unicode_string(vendorDesc, USB_MANUFACTURER_STRING);
	get_unicode_string(productDesc, USB_PRODUCT_STRING);
	get_unicode_string(serialDesc, USB_SERIAL_NUMBER_STRING);

	// align buffers to modulo8 address
	ep0Buff = alignBuff(ep0BuffAlloc, 32);
	bulkBuf = alignBuff(bulkBufAlloc, 32);
	cmdBuf = alignBuff(cmdBufAlloc, 32);
	VERBOSE ("bulkBuf %p bulkBufAlloc %p\n", bulkBuf, bulkBufAlloc);
	VERBOSE ("cmdBuf %p cmdBufAlloc %p\n", cmdBuf, cmdBufAlloc);
	VERBOSE ("ep0Buff %p ep0BuffAlloc %p\n", ep0Buff, ep0BuffAlloc);

	memset(ep0BuffAlloc, 0x00, EP0_SIZE);
	memset(bulkBufAlloc, 0x00, BUF_SIZE);
	memset(cmdBufAlloc,  0x00, BUF_SIZE);

	// allocate request for ep0
	ep0Req = ep0->ops->alloc_request(ep0);

	/*Change descriptor for maxSpeed == HS only Device*/
	/*For USB2.0 we have to modified wTotalLength of BOS descriptor*/
	if (drv_obj.gadget->max_speed < CH9_USB_SPEED_SUPER) {
		bosDesc.wTotalLength = cpuToLe16(CH9_USB_DS_BOS + CH9_USB_DS_DEVICE_CAPABILITY_20);
		bosDesc.bNumDeviceCaps = 1;
		devHsDesc.bcdUSB = cpuToLe16(BCD_USB_HS_ONLY);
	}

	//acm init
	acm = (struct f_acm*)acm_buf;
	acm->port_line_coding.dwDTERate = 921600;
	acm->port_line_coding.bCharFormat = USB_CDC_1_STOP_BITS;
	acm->port_line_coding.bParityType = USB_CDC_NO_PARITY;
	acm->port_line_coding.bDataBits = 8;
	acm->port_handshake_bits = 0;
	// VERBOSE("acm size %X\n", sizeof(struct f_acm));
	return 0;

}

#ifdef BUILD_ATF
uint8_t usb_phy_det_connection ()
{
	uint8_t phy_det_connected = 0;
	uint32_t bm_usb_phy_config = 0;

	bm_usb_phy_config  = plat_bm_gpio_read(BIT_MASK_GPIO_USB_PHY_DET_OFF);
	if (bm_usb_phy_config == 0) {
		phy_det_connected = 1;
		NOTICE ("by pass USB phy detection\n");
	} else {
		mmio_clrbits_32(GPIO_BASE + 0x4, 1<<4);
		phy_det_connected = (mmio_read_32(GPIO_BASE + 0x50) & (1<<4)) >> 4;
		INFO ("phy_det_connected %d\n", phy_det_connected);
	}

	return phy_det_connected;
}
#endif

#ifndef BUILD_ATF
uint32_t get_timer(uint32_t ts)
{
	return 0;
}
uint32_t plat_bm_gpio_read(uint32_t mask)
{
	return FIP_SRC_USB;
}
#endif

//ACM entry
int AcmApp()
{
	struct usb_gadget *gadget;
	uint32_t res = 0; // keeps result of operation on driver
	int fip_src = FIP_SRC_MEMMAP;
	uint32_t ts = 0;
	struct list_head *list; // used in for_each loop
	struct usb_ep *ep;
#ifdef BUILD_ATF
	uint8_t phy_det_connected = 0;

	phy_det_connected = usb_phy_det_connection ();

	if (phy_det_connected == 0) {
		NOTICE ("USB cable is not connected\n");
		return res;
	}
#endif
_reboot:
	init_param();

	print_buf_addr();
#ifdef BUILD_ATF
	fip_src = plat_bm_gpio_read(BIT_MASK_GPIO_BOOT_SEL);
#else
	fip_src = FIP_SRC_USB;
#endif // BUILD_ATF
	NOTICE ("fip_src %d\n", fip_src);
	if (fip_src == FIP_SRC_USB) {
		flagEnterDL = 1;
	} else {
		flagEnterDL = 0;
	}

#if USE_MALLOC
	bulkBufAlloc = memalign(32, BUF_SIZE);
	if (bulkBufAlloc == NULL) {
		VERBOSE("Error line: %d", __LINE__);
		goto error;
	} else {
		VERBOSE("bulkBufAlloc %p\n", bulkBufAlloc);
	}

	cmdBufAlloc = memalign(32, BUF_SIZE);
	if (cmdBufAlloc == NULL) {
		VERBOSE("Error line: %d", __LINE__);
		goto error;
	} else {
		VERBOSE("bulkBufAlloc %p\n", cmdBufAlloc);
	}

	cb0_buf = memalign(32, CB_SIZE);
	memset(cb0_buf, 0, CB_SIZE);
	cb1_buf = memalign(32, CB_SIZE);
	memset(cb1_buf, 0, CB_SIZE);
	cb2_buf = memalign(32, CB_SIZE);
	memset(cb2_buf, 0, CB_SIZE);

	ep0BuffAlloc = memalign(32, EP0_SIZE);
	memset(ep0BuffAlloc, 0, EP0_SIZE);
	rsp_buf = memalign(32, RSP_SIZE);
	memset(rsp_buf, 0, RSP_SIZE);
	acm_buf = memalign(32, 128);
	memset(acm_buf, 0, 128);
	setup_buf = memalign(32, 8);
	memset(setup_buf, 0, 8);
	handler = memalign(32, HANDLER_SIZE)
#endif
	drv_obj.plat.handler = (void *)handler;
	drv_obj.plat.size = HANDLER_SIZE;
	drv_obj.plat.ctrl_req = (void *)setup_buf;
	res = dwc2_udc_probe(&drv_obj.plat);
	if (res != 0) {
		goto error;
	}
	// bind the gadget object here.
	if (usb_gadget_register_driver(&g_driver) < 0) {
		NOTICE("Gadget Register Fail\n");
		goto error;
	}
	gadget = drv_obj.gadget;
	if (!gadget) {
		NOTICE("Gadget object not existed!\n");
		goto error;
	}

	acm_app_init();

	VERBOSE("Initializing OK! %d\n", __LINE__);

	ts = get_timer(0);
	VERBOSE("ts: %u\n", get_timer(ts));

	gadget->ops->pullup(gadget, 1);
unconfigured:
	while (!acm_configValue) {
#if !defined(USB_IRQ_MODE)
		AcmIsr();
#endif
		if (get_timer(ts) > 1000 && flagEnterDL == 0) {
			NOTICE("Enumeration failed\n");
			return 0;
		}
		if (flagReboot)
			goto _reboot;
	}
	NOTICE("USB enumeration done\n");

#if defined(USB_IRQ_MODE)
	disable_int();
#endif
	mem_alloc_cnt = 1;
	// find bulk endpoint
	for (list = gadget->ep_list.next; list != &gadget->ep_list; list = list->next) {

		ep = (struct usb_ep  *)list;
		if (ep->desc && (ep->desc->bEndpointAddress == BULK_EP_IN)) {
			bulkInReq = ep->ops->alloc_request(ep);
			epIn = ep;
		} else if (ep->desc && (ep->desc->bEndpointAddress == BULK_EP_OUT)) {
			bulkOutReq = ep->ops->alloc_request(ep);
			epOut = ep;
		} else if (ep->desc && (ep->desc->bEndpointAddress == BULK_EP_NOTIFY)) {
			IntInReq = ep->ops->alloc_request(ep);
			epInNotify = ep;
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
		VERBOSE("Test error\n");
		return -1;
	}

	VERBOSE("OUT DATA TRANSFER size :%d\n",transfer_size);
	clearReq(bulkOutReq);
	bulkOutReq->buf = cmdBuf;
	bulkOutReq->dma = (uintptr_t) cmdBuf;
	bulkOutReq->complete = bulkOutCmpl;
	bulkOutReq->length = transfer_size;

	VERBOSE("IN DATA TRANSFER\n");
	clearReq(bulkInReq);
	bulkInReq->buf = bulkBuf;
	bulkInReq->dma = (uintptr_t) bulkBuf;
	bulkInReq->complete = bulkInCmpl;
	bulkInReq->length = transfer_size;
	epOut->ops->queue(epOut, bulkOutReq);
	NOTICE("connection speed: %d\n", gadget->speed);
	ts = get_timer(0);
	VERBOSE("ts: %u\n", get_timer(ts));

#if defined(USB_IRQ_MODE)
	enable_int();
#endif
	while(1) {
#if !defined(USB_IRQ_MODE)
		AcmIsr();
#endif
		if (!acm_configValue) {
			goto unconfigured;
		}
		if (configBreak) {
			break;
		}
		if (flagEnterDL == 0) {
			if (get_timer(ts) > 1000) {
				NOTICE("wait data timeout\n");
				break;
			}
		}
	}
	NOTICE("Leave transfer loop\n");

	usb_gadget_unregister_driver(&g_driver);
	NOTICE("USB stop\n");
	return (0);

error:
	ERROR ("Error %u\n", res);
	return (res);
}
