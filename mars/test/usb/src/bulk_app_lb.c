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
#include "cdn_stdio.h"
#include <timer.h>

#include "cusbd_if.h"
#include "byteorder.h"
#include "log.h"

#include "map_system_memory.h"

//#define bulk_debug printf
#define bulk_debug(...)
#define EP0_DATA_BUFFER_SIZE 0x8000
#define DBG_USB_CTRL_MSG 0x00000002
#define USE_LARGE_TRB_SIZE

#define BULK_TEST_STALL_OUT	0xDEAD
#define BULK_TEST_STALL_IN	0xBEEF

// Super Speed USB object configuration
static CUSBD_Config config = {
    .regBase = USB_DEV_BASE, // address where USB core is mapped
    .epIN =
    {
        {.bufferingValue = 4}, // ep1in
        {.bufferingValue = 0}, // ep2in
        {.bufferingValue = 0}, // ep3in
        {.bufferingValue = 0}, // ep4in
        {.bufferingValue = 0}, // ep5in
        {.bufferingValue = 0}, // ep6in
        {.bufferingValue = 0}, // ep7in
        {.bufferingValue = 0}, // ep8in
        {.bufferingValue = 0}, // ep9in
        {.bufferingValue = 0}, // ep10in
        {.bufferingValue = 0}, // ep11in
        {.bufferingValue = 0}, // ep12in
        {.bufferingValue = 0}, // ep13in
        {.bufferingValue = 0}, // ep14in
        {.bufferingValue = 0} // ep15in
    },
    .epOUT =
    {
        {.bufferingValue = 4}, //ep1out
        {.bufferingValue = 0}, //ep2out
        {.bufferingValue = 0}, //ep3out
        {.bufferingValue = 0}, //ep4out
        {.bufferingValue = 0}, //ep5out
        {.bufferingValue = 0}, //ep6out
        {.bufferingValue = 0}, //ep7out
        {.bufferingValue = 0}, //ep8out
        {.bufferingValue = 0}, //ep9out
        {.bufferingValue = 0}, //ep10out
        {.bufferingValue = 0}, //ep11out
        {.bufferingValue = 0}, //ep12out
        {.bufferingValue = 0}, //ep13out
        {.bufferingValue = 0}, //ep14out
        {.bufferingValue = 0} //ep15out
    },
    .dmultEnabled = 0, // set to 1 if scatter/gather DMA available
    .dmaInterfaceWidth = CUSBD_DMA_64_WIDTH,
    .preciseBurstLength = CUSBD_PRECISE_BURST_0
};

static CUSBD_OBJ * drv; // driver pointer
static void * pD; // private data pointer

// interrupt handler
void bulkIsrLB() {
    drv->isr(pD);
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

static uint8_t* bulkBuf, * cmdBuf, *ep0Buff; // keeps mouse report
#if defined(USE_LARGE_TRB_SIZE)
static uint8_t bulkBufAlloc[1024 * 1024 * 10  + 7]; //16 MB
#else
static uint8_t bulkBufAlloc[1024 * 8  + 7]; //16 MB
#endif
static uint8_t cmdBufAlloc[8 + 7];
static uint8_t ep0BuffAlloc[1024 * 8 + 7]; //64 MB
static struct CUSBD_Req *bulkInReq, *bulkOutReq, *ep0Req; // request used by driver
static uint8_t configValue = 0;
static uint8_t current_speed = CH9_USB_SPEED_UNKNOWN;
static uint8_t cmd_flag = 0;

// debug functions
static void displayRequestinfo(struct CUSBD_Req * req);
static void displayEndpointInfo(struct CUSBD_Ep * ep);
static void displayDeviceInfo(CUSBD_Dev * dev);
static struct CUSBD_Ep * epIn, * epOut;

static void bulkInCmpl(struct CUSBD_Ep *ep, struct CUSBD_Req * req)  {
    //vDbgMsg(DBG_USB_CTRL_MSG, 1, "Transfer complete on ep:%02X %08X req\n", ep->address, (uintptr_t) req);
    bulk_debug("Transfer complete on ep:%02X %08X req\n", ep->address, (uintptr_t) req);
    //displayRequestinfo(req);
    if (req->status != 0) {return;}

    cmd_flag = 1;
    bulkOutReq->length = 8;
    bulkOutReq->buf = cmdBuf;
    bulkOutReq->dma = (uintptr_t) cmdBuf;
    epOut->ops->reqQueue(pD, epOut, bulkOutReq);

    //ep->ops->reqQueue(pD, ep, req);
}

static void bulkOutCmpl(struct CUSBD_Ep *ep, struct CUSBD_Req * req)  {

    uint8_t dir = ((uint8_t*) req->buf)[0];
    uint32_t length = le32ToCpu(*(uint32_t*) (req->buf + 4));

    bulk_debug("Transfer complete on ep:%02X %08X req\n", ep->address, (uintptr_t) req);
    //displayRequestinfo(req);
    if (req->status != 0) {return;}

    bulk_debug("DATA DIR: %d\n", dir);
    bulk_debug("DATA LENGTH: %X\n", length);

    if (cmd_flag) {
        cmd_flag = 0;
        if (dir) {
	    if (length == BULK_TEST_STALL_IN) {
                epIn->ops->epSetHalt(pD, epIn, 1);
		bulk_debug("stall epIn!\n");

                cmd_flag = 1;
                bulkOutReq->length = 8;
                bulkOutReq->buf = cmdBuf;
                bulkOutReq->dma = (uintptr_t) cmdBuf;
                epOut->ops->reqQueue(pD, epOut, bulkOutReq);
                return;
            }
            bulkInReq->length = length;
            bulkInReq->buf = bulkBuf;
            bulkInReq->dma = (uintptr_t) bulkBuf;
            epIn->ops->reqQueue(pD, epIn, bulkInReq);
        } else {
	    if (length == BULK_TEST_STALL_OUT) {
                epOut->ops->epSetHalt(pD, epOut, 1);
		bulk_debug("stall epOut!\n");

                cmd_flag = 1;
                bulkOutReq->length = 8;
                bulkOutReq->buf = cmdBuf;
                bulkOutReq->dma = (uintptr_t) cmdBuf;
                epOut->ops->reqQueue(pD, epOut, bulkOutReq);
                return;
            }
            bulkOutReq->length = length;
            bulkOutReq->buf = bulkBuf;
            bulkOutReq->dma = (uintptr_t) bulkBuf;
            epOut->ops->reqQueue(pD, epOut, bulkOutReq);
        }
    } else {
        cmd_flag = 1;
        bulkOutReq->length = 8;
        bulkOutReq->buf = cmdBuf;
        bulkOutReq->dma = (uintptr_t) cmdBuf;
        epOut->ops->reqQueue(pD, epOut, bulkOutReq);
    }
    //epIn->ops->reqQueue(pD, epIn, bulkInReq);
    //ep->ops->reqQueue(pD, ep, req);
}

// clears request

static void clearReq(struct CUSBD_Req * req) {
    memset(req, 0, sizeof (struct CUSBD_Req));
}

// function return unicode string for english version

static void get_unicode_string(char * target, const char * src) {

    size_t src_len = strlen(src) * 2;
    int i;

    *target++ = src_len + 2;
    *target++ = CH9_USB_DT_STRING;

    if (src_len > 100) {src_len = 100;}
    for (i = 0; i < src_len; i += 2) {
        *target++ = *src++;
        *target++ = 0;
    }
}

// functions aligns buffer to alignValue modulo address

static uint8_t * alignBuff(void * notAllignedBuff, uint8_t alignValue) {

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

static void connect(void *pD)  {
    CUSBD_Dev *dev;


    //vDbgMsg(DBG_USB_CTRL_MSG, 1, "Application: connect at %d speed\n", dev->speed);
    bulk_debug("Application: connect at %d speed\n", dev->speed);
}

static void disconnect(void *pD)  {
    CUSBD_Dev *dev;

    if(!drv) {
        return;
    }

    drv->getDevInstance(pD, &dev);
    vDbgMsg(DBG_USB_CTRL_MSG, 1, "Application: disconnect %c\n", ' ');
    displayDeviceInfo(dev);
    configValue = 0;
}

static void resume(void *pD)  {
    //vDbgMsg(DBG_USB_CTRL_MSG, 3, "Application: resume %c\n", ' ');
    bulk_debug("Application: leave suspend\n");
}

static void reqComplete(struct CUSBD_Ep *ep, struct CUSBD_Req * req)  {
    vDbgMsg(DBG_USB_CTRL_MSG, 1, "Request on endpoint %s completed\n", ep->name);
    //displayRequestinfo(req);

}

static uint32_t setup(void *pD, CH9_UsbSetup *ctrl)  {

    // get device reference
    CUSBD_Dev * dev;
    int length = 0;

    CH9_UsbConfigurationDescriptor *configDesc;
    CH9_UsbDeviceDescriptor * devDesc;

    ctrl->wIndex = le16ToCpu(ctrl->wIndex);
    ctrl->wLength = le16ToCpu(ctrl->wLength);
    ctrl->wValue = le16ToCpu(ctrl->wValue);

    drv->getDevInstance(pD, &dev);

    // select descriptors according to actual speed
    switch (dev->speed) {

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
        vDbgMsg(DBG_USB_CTRL_MSG, 1, "Unknown speed %d: \n", dev->speed);
        return 1;
        break;
    }

    vDbgMsg(DBG_USB_CTRL_MSG, 1, "Speed %d: \n", dev->speed);
    vDbgMsg(DBG_USB_CTRL_MSG, 1, "bRequest: %02X\n", ctrl->bRequest);
    vDbgMsg(DBG_USB_CTRL_MSG, 1, "bRequestType: %02X\n", ctrl->bmRequestType);
    vDbgMsg(DBG_USB_CTRL_MSG, 1, "wIndex: %04X\n", ctrl->wIndex);
    vDbgMsg(DBG_USB_CTRL_MSG, 1, "wValue: %04X\n", ctrl->wValue);
    vDbgMsg(DBG_USB_CTRL_MSG, 1, "wLength: %04X\n", ctrl->wLength);

    ep0Req->buf = ep0Buff;
    ep0Req->dma = (uintptr_t) ep0Buff;
    ep0Req->complete = reqComplete;

    switch (ctrl->bmRequestType & CH9_USB_REQ_TYPE_MASK) {

    case CH9_USB_REQ_TYPE_STANDARD:

        switch (ctrl->bRequest) {

        case CH9_USB_REQ_GET_DESCRIPTOR:
            vDbgMsg(DBG_USB_CTRL_MSG, 1, "GET DESCRIPTOR %c\n", ' ');
            if ((ctrl->bmRequestType & CH9_REQ_RECIPIENT_MASK) == CH9_USB_REQ_RECIPIENT_INTERFACE) {
                switch (ctrl->wValue >> 8) {

                default:
                    return -1;
                }
            } else
            if ((ctrl->bmRequestType & CH9_REQ_RECIPIENT_MASK) == CH9_USB_REQ_RECIPIENT_DEVICE) {
                switch (ctrl->wValue >> 8) {

                case CH9_USB_DT_DEVICE:
                    length = CH9_USB_DS_DEVICE;
                    memmove(ep0Buff, devDesc, 18);
                    vDbgMsg(DBG_USB_CTRL_MSG, 1, "DevDesc[0] = %d\n", devDesc->bLength);
                    {
                        int i;

                        for (i = 0; i < length; i++) {
                            bulk_debug("%02X ", ep0Buff[i]);
                        }
                        bulk_debug(" %c\n", ' ');
                    }
                    break;

                case CH9_USB_DT_CONFIGURATION:
                {
                    int offset = 0;

                    length = le16ToCpu(configDesc->wTotalLength);
                    current_speed = dev->speed;
                    // select descriptors according to actual speed
                    switch (dev->speed) {

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
                    if (dev->speed == CH9_USB_SPEED_SUPER) {
                        memmove(&ep0Buff[offset], &compDesc, CH9_USB_DS_SS_USB_ENDPOINT_COMPANION);
                        offset += CH9_USB_DS_SS_USB_ENDPOINT_COMPANION;
                    }
                    memmove(&ep0Buff[offset], &endpointEpOutDesc, CH9_USB_DS_ENDPOINT);
                    offset += CH9_USB_DS_ENDPOINT;

                    if (dev->speed == CH9_USB_SPEED_SUPER) {
                        memmove(&ep0Buff[offset], &compDesc, CH9_USB_DS_SS_USB_ENDPOINT_COMPANION);
                    }

                    vDbgMsg(DBG_USB_CTRL_MSG, 1, "ConfDesc[0] = %02X\n", configDesc->bLength);
                    {
                        int i;

                        for (i = 0; i < length; i++) {
                            bulk_debug("%02X ", ep0Buff[i]);
                        }
                        bulk_debug(" %c\n", ' ');
                    }
                    break;
                }

                case CH9_USB_DT_STRING:
                {
                    uint8_t descIndex = (uint8_t) (ctrl->wValue & 0xFF);
                    char *strDesc;
                    vDbgMsg(DBG_USB_CTRL_MSG, 1, "StringDesc %c\n", ' ');
                    switch (descIndex) {
                    case 0:
                        strDesc = (char*) &languageDesc;
                        length = strDesc[0];
                        vDbgMsg(DBG_USB_CTRL_MSG, 1, "language %c\n", ' ');
                        break;

                    case 1:
                        strDesc = (char*) &vendorDesc;
                        length = strDesc[0];
                        vDbgMsg(DBG_USB_CTRL_MSG, 1, "vendor %c\n", ' ');
                        break;

                    case 2:
                        strDesc = (char*) &productDesc;
                        length = strDesc[0];
                        vDbgMsg(DBG_USB_CTRL_MSG, 1, "product %c\n", ' ');
                        break;

                    case 3:
                        strDesc = (char*) &serialDesc;
                        length = strDesc[0];
                        vDbgMsg(DBG_USB_CTRL_MSG, 1, "serial %c\n", ' ');
                        break;

                    default: return -1;
                    }
                    memmove(ep0Buff, strDesc, length);
                    break;
                }

                case CH9_USB_DT_BOS:
                {
                    int offset = 0;
                    length = le16ToCpu(bosDesc.wTotalLength);

                    memmove(ep0Buff, &bosDesc, CH9_USB_DS_BOS);
                    offset += CH9_USB_DS_BOS;
                    /*Only USB3 should support CH9_USB_DS_DEVICE_CAPABILITY_30 descriptor*/
                    if (dev->maxSpeed == CH9_USB_SPEED_SUPER || dev->maxSpeed == CH9_USB_SPEED_SUPER_PLUS) {
                        memmove(&ep0Buff[offset], &capabilitySsDesc, CH9_USB_DS_DEVICE_CAPABILITY_30);
                        offset += CH9_USB_DS_DEVICE_CAPABILITY_30;
                    }

                    memmove(&ep0Buff[offset], &capabilityExtDesc, CH9_USB_DS_DEVICE_CAPABILITY_20);
                }

                    vDbgMsg(DBG_USB_CTRL_MSG, 1, "bosDesc[0] = %02X\n", bosDesc.bLength);
                    {
                        int i;

                        for (i = 0; i < length; i++) {
                            vDbgMsg(DBG_USB_CTRL_MSG, 1, "%02X ", ep0Buff[i]);
                        }
                        vDbgMsg(DBG_USB_CTRL_MSG, 1, " %c\n", ' ');
                    }
                    vDbgMsg(DBG_USB_CTRL_MSG, 1, "BosDesc %c\n", ' ');
                    break;

                case CH9_USB_DT_DEVICE_QUALIFIER:
                    length = CH9_USB_DS_DEVICE_QUALIFIER;
                    memmove(ep0Buff, &qualifierDesc, length);
                    vDbgMsg(DBG_USB_CTRL_MSG, 1, "QualifierDesc %c\n", ' ');
                    break;

                case CH9_USB_DT_OTHER_SPEED_CONFIGURATION:
                {
                    int offset = 0;

                    length = le16ToCpu(configDesc->wTotalLength);

                    memmove(&ep0Buff[offset], configDesc, CH9_USB_DS_CONFIGURATION);
                    ep0Buff[1] = CH9_USB_DS_OTHER_SPEED_CONFIGURATION;
                    offset += CH9_USB_DS_CONFIGURATION;

                    memmove(&ep0Buff[offset], &interfaceDesc, CH9_USB_DS_INTERFACE);
                    offset += CH9_USB_DS_INTERFACE;

                    memmove(&ep0Buff[offset], &endpointEpInDesc, CH9_USB_DS_ENDPOINT);
                    offset += CH9_USB_DS_ENDPOINT;
                    if (dev->speed == CH9_USB_SPEED_SUPER) {
                        memmove(&ep0Buff[offset], &compDesc, CH9_USB_DS_SS_USB_ENDPOINT_COMPANION);
                        offset += CH9_USB_DS_SS_USB_ENDPOINT_COMPANION;
                    }
                    memmove(&ep0Buff[offset], &endpointEpOutDesc, CH9_USB_DS_ENDPOINT);
                    offset += CH9_USB_DS_ENDPOINT;

                    memmove(&ep0Buff[offset], &compDesc, CH9_USB_DS_SS_USB_ENDPOINT_COMPANION);

                    vDbgMsg(DBG_USB_CTRL_MSG, 1, "OtherSpeedDesc[0] = %02X\n", configDesc->bLength);
                    {
                        int i;

                        for (i = 0; i < length; i++) {
                            vDbgMsg(DBG_USB_CTRL_MSG, 1, "%02X ", ep0Buff[i]);
                        }
                        vDbgMsg(DBG_USB_CTRL_MSG, 1, " %c\n", ' ');
                    }
                    break;
                }

                default:
                    vDbgMsg(DBG_USB_CTRL_MSG, 1, "Error_1 %c\n", ' ');
                    return -1;

                }        //switch
            }        //if
            break;

        case CH9_USB_REQ_SET_CONFIGURATION:
        {
            struct CUSBD_Ep * ep;
            CUSBD_ListHead *list;

            bulk_debug("SET CONFIGURATION(%d)\n", le16ToCpu(ctrl->wValue));
            vDbgMsg(DBG_USB_CTRL_MSG, 1, "SET CONFIGURATION(%d)\n", le16ToCpu(ctrl->wValue));
            if (ctrl->wValue > 1) { return -1;       // no such configuration
            }
            // unconfigure device
            if (ctrl->wValue == 0) {
                configValue = 0;
                for (list = dev->epList.next; list != &dev->epList; list = list->next) {
                    ep = (struct CUSBD_Ep *) list;
                    if (ep->address == BULK_EP_IN || ep->address == BULK_EP_OUT) {
                        ep->ops->epDisable(pD, ep);
                        //displayEndpointInfo(ep);
                    }
                }
                return 0;
            }

            cmd_flag = 1;
            // device already configured
            if (configValue == 1 && ctrl->wValue == 1) { return 0;}

            // configure device
            configValue = (uint8_t) ctrl->wValue;
            for (list = dev->epList.next; list != &dev->epList; list = list->next) {
                ep = (struct CUSBD_Ep *) list;
                if (ep->address == BULK_EP_IN) {
                    ep->ops->epEnable(pD, ep, &endpointEpInDesc);
                    displayEndpointInfo(ep);
                    break;
                }
            }
            for (list = dev->epList.next; list != &dev->epList; list = list->next) {
                ep = (struct CUSBD_Ep *) list;
                if (ep->address == BULK_EP_OUT) {
                    ep->ops->epEnable(pD, ep, &endpointEpOutDesc);
                    displayEndpointInfo(ep);
                    break;
                }
            }

            /*Code control  Self powered feature of USB*/
            if (configDesc->bmAttributes & CH9_USB_CONFIG_SELF_POWERED) {
                if (drv->dSetSelfpowered) {
                    drv->dSetSelfpowered(pD);
                }
            } else {
                if (drv->dClearSelfpowered) {
                    drv->dSetSelfpowered(pD);
                }
            }
        }
            return 0;

        case CH9_USB_REQ_GET_CONFIGURATION:
            length = 1;
            memmove(ep0Buff, &configValue, length);
            break;

        default:
            vDbgMsg(DBG_USB_CTRL_MSG, 1, "Error_2 %c\n", ' ');
            return -1;         //return error
        }
        break;

    case CH9_USB_REQ_TYPE_CLASS:
        return -1;
        break;
    }

    if (length > 0) {
        ep0Req->length = ctrl->wLength < length ? ctrl->wLength : length;
        dev->ep0->ops->reqQueue(pD, dev->ep0, ep0Req);
    }
    return 0;
}

static void suspend(void *pD)  {
    //vDbgMsg(DBG_USB_CTRL_MSG, 3, "Application: suspend %c\n", ' ');
    bulk_debug("Application: enter suspend\n");
#if defined(TEST_RWP)
    if (configValue) {
        bulk_debug("Application: rwp after 5s\n");
        mdelay(5000);
        drv->dWakeUp(pD);
    }
#endif
}

static void displayRequestinfo(struct CUSBD_Req * req) {
    bulk_debug("-------Request INFO-------------- %c\n", ' ');
    bulk_debug("buf: %08X\n", (uintptr_t) req->buf);
    bulk_debug("length: %d\n", req->length);
    bulk_debug("dma: %08X\n", req->dma);
    bulk_debug("sg:  %c\n", ' ');
    bulk_debug("numOfSgs %d\n", req->numOfSgs);
    bulk_debug("numMappedSgs: %d\n", req->numMappedSgs);
    bulk_debug("streamId: %04X\n", req->streamId);
    bulk_debug("noInterrupt: %d\n", req->noInterrupt);
    bulk_debug("zero: %d\n", req->zero);
    bulk_debug("shortNotOk: %d\n", req->shortNotOk);
    bulk_debug("context: %08X\n", (uintptr_t) req->context);
    bulk_debug("status: %08X\n", req->status);
    bulk_debug("actual: %d\n", req->actual);
}

static void displayEndpointInfo(struct CUSBD_Ep * ep) {
    bulk_debug("-------Endpoint INFO------------- %c\n", ' ');
    bulk_debug("address: %02X\n", ep->address);
    bulk_debug("epList: %08X\n", (uintptr_t) &ep->epList);
    bulk_debug("driverData: %08X\n", (uintptr_t) ep->driverData);
    //bulk_debug("name: %s\n", ep->name);
    bulk_debug("ops: %c\n", ' ');
    bulk_debug("maxPacket: %d\n", ep->maxPacket);
    bulk_debug("maxStreams: %d\n", ep->maxStreams);
    bulk_debug("mult: %d\n", ep->mult);
    bulk_debug("maxburst: %d\n", ep->maxburst);
    bulk_debug("desc: %08X\n", (uintptr_t) ep->desc);
    bulk_debug("compDesc: %08X\n", (uintptr_t) ep->compDesc);
}

static void displayDeviceInfo(CUSBD_Dev * dev) {

    bulk_debug("-------Device INFO--------------- %c\n", ' ');
    bulk_debug("epList: prev = %08X, next = %08X\n", (uintptr_t) dev->epList.prev, (uintptr_t) dev->epList.next);
    bulk_debug("ep0: %08X\n", (uintptr_t) dev->ep0);
    bulk_debug("speed: %d\n", dev->speed);
    bulk_debug("maxSpeed: %d\n", dev->maxSpeed);
    bulk_debug("state: %d\n", dev->state);
    bulk_debug("sgSupported: %d\n", dev->sgSupported);
    bulk_debug("isOtg: %d\n", dev->isOtg);
    bulk_debug("isAPeripheral: %d\n", dev->isAPeripheral);
    bulk_debug("bHnpEnable: %d\n", dev->bHnpEnable);
    bulk_debug("aHnpSupport: %d\n", dev->aHnpSupport);
    //bulk_debug("name: %s\n", dev->name);
}

static void * requestMemAlloc(void * pD, uint32_t requireSize)  {
    return malloc(requireSize);
}

static void requestMemFree(void * pD, void* usbRequest)  {
    if (usbRequest) {
        free(usbRequest);
    }
}

static CUSBD_Callbacks callback = {
    disconnect, connect, setup, suspend, resume, requestMemAlloc, requestMemFree
};

/*
 *
 */
int bulkAppLB() {
    uint32_t res; // keeps result of operation on driver
    CUSBD_SysReq sysReq;
    CUSBD_Dev * dev; // auxiliary pointer used for testing purposes
    uint32_t transfer_size = 0;
    CUSBD_ListHead *list; // used in for_each loop
    //struct CUSBD_Ep * epIn, * epOut;

    drv = CUSBD_GetInstance(); // get driver handle

    res = drv->probe(&config, &sysReq);
    if (res != 0) { goto error;}
    bulk_debug("Device memory requirement: %ld bytes\n", sysReq.privDataSize);

    // allocate memory for device object
    pD = (uint8_t*)malloc((size_t) (sysReq.privDataSize));
    if(pD == NULL) {
        bulk_debug("Error line: %d", __LINE__);
        goto error;
    } else {
        bulk_debug("pD addr %p\n", pD);
    }

    config.trbAddr = malloc(sysReq.trbMemSize);
    if(config.trbAddr == NULL) {
        bulk_debug("Error line: %d", __LINE__);
        goto error;
    } else {
        bulk_debug("config.trbAddr addr %p\n", config.trbAddr);
    }
    config.trbDmaAddr = (uintptr_t)config.trbAddr;
    bulk_debug("config.trbDmaAddr addr %p\n", config.trbDmaAddr);

    res = drv->init(pD, &config, &callback);
    if (res != 0) { goto error;}
    vDbgMsg(DBG_USB_CTRL_MSG, 1, "Initializing OK! %s\n", " ");
    // set unicode strings
    get_unicode_string(vendorDesc, USB_MANUFACTURER_STRING);
    get_unicode_string(productDesc, USB_PRODUCT_STRING);
    get_unicode_string(serialDesc, USB_SERIAL_NUMBER_STRING);

    // align buffers for bulk-echo application
    bulkBuf = alignBuff(bulkBufAlloc, 128);
    cmdBuf = alignBuff(cmdBufAlloc, 8);
    ep0Buff = alignBuff(ep0BuffAlloc, 8);
    memset(ep0BuffAlloc, 0x00, sizeof (ep0BuffAlloc));
    //memset(bulkBufAlloc, 0x00, sizeof (bulkBufAlloc));
    memset(cmdBufAlloc, 0x00, sizeof (cmdBufAlloc));

    // checking device and endpoints parameters correctness
    // get CUSBD device instance exposed as Linux gadget device
    drv->getDevInstance(pD, &dev);
    displayDeviceInfo(dev);

    // allocate request for ep0
    drv->reqAlloc(pD, dev->ep0, &ep0Req);

    /*Change descriptor for maxSpeed == HS only Device*/
    /*For USB2.0 we have to modified wTotalLength of BOS descriptor*/
    if(dev->maxSpeed == CH9_USB_SPEED_HIGH || dev->maxSpeed == CH9_USB_SPEED_FULL) {
        bosDesc.wTotalLength = cpuToLe16(CH9_USB_DS_BOS + CH9_USB_DS_DEVICE_CAPABILITY_20);
        bosDesc.bNumDeviceCaps = 1;
        devHsDesc.bcdUSB = cpuToLe16(BCD_USB_HS_ONLY);
    }

    drv->start(pD);

unconfigured:
    while (!configValue) {
        bulk_debug("Wait for device has been configured!\n");
    }

    disable_int();
    // find IN endpoint
    for (list = dev->epList.next; list != &dev->epList; list = list->next) {
        epIn = (struct CUSBD_Ep  *) list;
        displayEndpointInfo(epIn);
        if (epIn->address == BULK_EP_IN) {
            drv->reqAlloc(pD, epIn, &bulkInReq);
            break;
        }
    }

    // find OUT endpoint
    for (list = dev->epList.next; list != &dev->epList; list = list->next) {
        epOut = (struct CUSBD_Ep  *) list;
        displayEndpointInfo(epIn);
        if (epOut->address == BULK_EP_OUT) {
            drv->reqAlloc(pD, epOut, &bulkOutReq);
            break;
        }
    }

    current_speed = dev->speed;
    switch (current_speed) {
    case CH9_USB_SPEED_FULL: transfer_size = 64;
        break;
    case CH9_USB_SPEED_HIGH: transfer_size = 512;
        break;
    case CH9_USB_SPEED_SUPER: transfer_size = 1024;
        break;
    default: bulk_debug("Test error\n");
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
    epOut->ops->reqQueue(pD, epOut, bulkOutReq);
    enable_int();

    while (1) {
        //bulk_debug("Device transfers data\n");
        if (!configValue) { goto unconfigured;}
    }

    drv->stop(pD);
    drv->destroy(pD);

    // free allocated memory
    if (pD) {
        free(pD);
    }

    return (0);

error:
    bulk_debug(DBG_USB_CTRL_MSG, 1, "Error %08lX\n", res);
    // free allocated memory
    if (pD) { free(pD);}
    if (config.trbAddr) {
        free (config.trbAddr);
    }
    return (res);
}
