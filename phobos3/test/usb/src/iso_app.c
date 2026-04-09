/**********************************************************************
 * Copyright (C) 2014-2017 Cadence Design Systems, Inc.
 * All rights reserved worldwide.
 ***********************************************************************
 * iso_app.c
 * Application for testing isochronous transfers on OUT and IN endpoints.
 *
 * This testing application doesn't fully compliant with USB specification
 * and wont be working correctly with standard PC USB stack.
 * This application does not support alternative interface and changing
 * endpoint parameters has to be achieved by SET CONFIGURATION request.
 * This application support three configuration.
 *
 * This application was created for testing purpose for Cadence Simulation
 * Platform, and has been attached to delivery package because has been
 * used to generating testing report for Isochronous transfers.
 *
 ***********************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cusbd_if.h"
#include "byteorder.h"
#include "cdn_errno.h"                     // errors definitions
#include "log.h"
#include "map_system_memory.h"

#define DBG_USB_CTRL_MSG 0x00000002


// Super Speed USB object configuration
static CUSBD_Config configDriver = {
    .regBase = USB_REGS_BASE, // address where USB core is mapped
    .epIN =
    {
        {.bufferingValue = 3}, // ep1in
        {.bufferingValue = 3}, // ep2in
        {.bufferingValue = 3}, // ep3in
        {.bufferingValue = 3}, // ep4in
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
        {.bufferingValue = 3}, //ep1out
        {.bufferingValue = 3}, //ep2out
        {.bufferingValue = 3}, //ep3out
        {.bufferingValue = 3}, //ep4out
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


//-------------- definitions used during configuration -------------------------

#define BCD_USB_SS                      0x0300  // 3.00 version USB
#define ID_VENDOR                       0x0559  // CADENCE ID

#ifdef HS_FS_ONLY
#define ID_PRODUCT                      0x1004
#else
#define ID_PRODUCT                      0x4013
#endif

#define BCD_DEVICE_SS                   0x0001  // 0.1

#define BCD_USB_HS_ONLY                 0x0201  // 2.01  /*Only HS with BOS descriptor*/
#define BCD_USB_HS                      0x0210  // 2.10

#define BCD_DEVICE_HS                   0x0200  // 2.00

#define USB_MANUFACTURER_STRING         "CADENCE"
#define USB_PRODUCT_STRING              "Isochronous transfers tester"
#define USB_SERIAL_NUMBER_STRING        "100000000000" // should 12 chars long

#define EP_IN 0x83
#define EP_OUT 0x03
//------------------------------------------------------------------------------

// buffers for all endpoints
static uint8_t *inBuf, *outBuf, *ep0Buff;

// statically allocated data buffers for all endpoints
static uint8_t inBufAlloc[1024 * 16 + 7];
static uint8_t outBufAlloc[1024 * 16 + 7];
static uint8_t ep0BuffAlloc[100 + 7];

// variables used by CUSBD driver
static struct CUSBD_Req *inReq, *outReq, *ep0Req;
static struct CUSBD_Ep *epIn, *epOut;

// variables used by application
static uint8_t configValue = 0;
static uint8_t current_speed = CH9_USB_SPEED_UNKNOWN;

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
    .bLength = CH9_USB_DS_DEVICE,
    .bDescriptorType = CH9_USB_DT_DEVICE,
    .bcdUSB = cpuToLe16(BCD_USB_SS),
    .bDeviceClass = 0,
    .bDeviceSubClass = 0,
    .bDeviceProtocol = 0,
    .bMaxPacketSize0 = 9,
    .idVendor = cpuToLe16(ID_VENDOR),
    .idProduct = cpuToLe16(ID_PRODUCT),
    .bcdDevice = cpuToLe16(BCD_DEVICE_SS),
    .iManufacturer = 1,
    .iProduct = 2,
    .iSerialNumber = 3,
    .bNumConfigurations = 3
};

// device descriptor for HighSpeed mode
static CH9_UsbDeviceDescriptor devHsDesc = {
    .bLength = CH9_USB_DS_DEVICE,
    .bDescriptorType = CH9_USB_DT_DEVICE,
    .bcdUSB = cpuToLe16(BCD_USB_HS),
    .bDeviceClass = 0,
    .bDeviceSubClass = 0,
    .bDeviceProtocol = 0,
    .bMaxPacketSize0 = 64,
    .idVendor = cpuToLe16(ID_VENDOR),
    .idProduct = cpuToLe16(ID_PRODUCT),
    .bcdDevice = cpuToLe16(BCD_DEVICE_HS),
    .iManufacturer = 1,
    .iProduct = 2,
    .iSerialNumber = 3,
    .bNumConfigurations = 3
};
//------------- Start of Super Speed configuration descriptors -----------------
// configuration descriptors for SuperSpeed mode
static CH9_UsbConfigurationDescriptor confSsDesc = {
    .bLength = CH9_USB_DS_CONFIGURATION,
    .bDescriptorType = CH9_USB_DT_CONFIGURATION,
    .wTotalLength = cpuToLe16(
        CH9_USB_DS_CONFIGURATION
        + CH9_USB_DS_INTERFACE
        + 2 * CH9_USB_DS_ENDPOINT
        + 2 * CH9_USB_DS_SS_USB_ENDPOINT_COMPANION),
    .bNumInterfaces = 1,
    .bConfigurationValue = 1,
    .iConfiguration = 0,
    .bmAttributes = CH9_USB_CONFIG_RESERVED | CH9_USB_CONFIG_SELF_POWERED,
    .bMaxPower = 1
};

//0x09, 0x04, 0x00, 0x00, 0x01, 0x03, 0x00, 0x00, 0x00,
static CH9_UsbInterfaceDescriptor interfaceDesc = {
    .bLength = CH9_USB_DS_INTERFACE,
    .bDescriptorType = CH9_USB_DT_INTERFACE,
    .bInterfaceNumber = 0,
    .bAlternateSetting = 0,
    .bNumEndpoints = 2,
    .bInterfaceClass = CH9_USB_CLASS_VENDOR,
    .bInterfaceSubClass = 0,
    .bInterfaceProtocol = 0,
    .iInterface = 0
};

static CH9_UsbEndpointDescriptor endpointEpInDesc = {
    .bLength = CH9_USB_DS_ENDPOINT,
    .bDescriptorType = CH9_USB_DT_ENDPOINT,
    .bEndpointAddress = EP_IN,
    .bmAttributes = CH9_USB_EP_ISOCHRONOUS,
    .wMaxPacketSize = cpuToLe16(1024),
    .bInterval = 1
};

static CH9_UsbEndpointDescriptor endpointEpOutDesc = {
    .bLength = CH9_USB_DS_ENDPOINT,
    .bDescriptorType = CH9_USB_DT_ENDPOINT,
    .bEndpointAddress = EP_OUT,
    .bmAttributes = CH9_USB_EP_ISOCHRONOUS,
    .wMaxPacketSize = cpuToLe16(1024),
    .bInterval = 1
};

//0x06, 0x30, 0x00, 0x00, 0x00, 0x00
static CH9_UsbSSEndpointCompanionDescriptor compDesc = {
    .bLength = CH9_USB_DS_SS_USB_ENDPOINT_COMPANION,
    .bDescriptorType = CH9_USB_DT_SS_USB_ENDPOINT_COMPANION,
    .bMaxBurst = 0,
    .bmAttributes = 0,
    .wBytesPerInterval = cpuToLe16(1024)
};
//------------------- End of Super Speed configuration -------------------------

//------------- Start of High Speed configuration descriptors -----------------
// configuration descriptors for HighSpeed mode
static CH9_UsbConfigurationDescriptor confHsDesc = {
    .bLength = CH9_USB_DS_CONFIGURATION,
    .bDescriptorType = CH9_USB_DT_CONFIGURATION,
    .wTotalLength = cpuToLe16(
        CH9_USB_DS_CONFIGURATION
        + CH9_USB_DS_INTERFACE
        + 2 * CH9_USB_DS_ENDPOINT),
    .bNumInterfaces = 1,
    .bConfigurationValue = 1,
    .iConfiguration = 0,
    .bmAttributes = CH9_USB_CONFIG_RESERVED | CH9_USB_CONFIG_SELF_POWERED,
    .bMaxPower = 0
};

// language descriptor for english
static uint8_t languageDesc[] = {
    4, CH9_USB_DT_STRING, 0x09, 0x04
};

// string will be filled then in initializing section
static char vendorDesc[sizeof (USB_MANUFACTURER_STRING) * 2 + 2];
static char productDesc[sizeof (USB_PRODUCT_STRING) * 2 + 2];
static char serialDesc[sizeof (USB_SERIAL_NUMBER_STRING) * 2 + 2];

//-------------- BOS descriptor set start --------------------------------------

static CH9_UsbBosDescriptor bosDesc = {
    .bLength = CH9_USB_DS_BOS,
    .bDescriptorType = CH9_USB_DT_BOS,
    .wTotalLength = cpuToLe16(CH9_USB_DS_BOS + CH9_USB_DS_DEVICE_CAPABILITY_30 + CH9_USB_DS_DEVICE_CAPABILITY_20),
    .bNumDeviceCaps = 2
};

static CH9_UsbSSDeviceCapabilityDescriptor capabilitySsDesc = {
    .bLength = CH9_USB_DS_DEVICE_CAPABILITY_30,
    .bDescriptorType = CH9_USB_DT_DEVICE_CAPABILITY,
    .bDevCapabilityType = CH9_USB_DCT_SS_USB,
    .bmAttributes = 0, // LTM not supported
    .wSpeedSupported = cpuToLe16(CH9_USB_SS_CAP_SUPPORT_SS | CH9_USB_SS_CAP_SUPPORT_HS | CH9_USB_SS_CAP_SUPPORT_FS),
    .vFunctionalitySupport = 1, //<----what's that????????????????????, look in spec
    .bU1DevExitLat = 4, // 4 us
    .bU2DevExitLat = cpuToLe16(512) // 512 us
};

static CH9_UsbCapabilityDescriptor capabilityExtDesc = {
    .bLength = CH9_USB_DS_DEVICE_CAPABILITY_20,
    .bDescriptorType = CH9_USB_DT_DEVICE_CAPABILITY,
    .bDevCapabilityType = CH9_USB_DCT_USB20_EXTENSION,
    .bmAttributes = cpuToLe32(CH9_USB_USB20_EXT_LPM_SUPPORT)
};
//-------------- BOS descriptor set end ----------------------------------------

static CH9_UsbDeviceQualifierDescriptor qualifierDesc = {
    .bLength = CH9_USB_DS_DEVICE_QUALIFIER,
    .bDescriptorType = CH9_USB_DT_DEVICE_QUALIFIER,
    .bcdUSB = cpuToLe16(0x0200),
    .bDeviceClass = 0x00,
    .bDeviceSubClass = 0x00,
    .bDeviceProtocol = 0x00,
    .bMaxPacketSize0 = 64,
    .bNumConfigurations = 0x01,
    .bReserved = 0x00
};

//--------------- Core Driver callback definitions ----------------------------

static void prepareRequests(uint8_t configValue);
static void removeRequests();

static void connect(void *pD) {
    CUSBD_Dev *dev;


    drv->getDevInstance(pD, &dev);
    vDbgMsg(DBG_USB_CTRL_MSG, 1, "Application: connect at %d speed\n", dev->speed);
}

static void disconnect(void *pD) {
    CUSBD_Dev *dev;

    if (!drv) {
        return;
    }

    drv->getDevInstance(pD, &dev);
    vDbgMsg(DBG_USB_CTRL_MSG, 1, "Application: disconnect %c\n", ' ');
    configValue = 0;
}

static void resume(void *pD) {
    vDbgMsg(DBG_USB_CTRL_MSG, 1, "Application: resume %c\n", ' ');
}

static void reqComplete(struct CUSBD_Ep *ep, struct CUSBD_Req * req) {
    vDbgMsg(DBG_USB_CTRL_MSG, 1, "Request on endpoint %s completed\n", ep->name);
}

static uint32_t setup(void *pD, CH9_UsbSetup *ctrl) {
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
                    break;

                case CH9_USB_DT_CONFIGURATION:
                {
                    int offset = 0;
                    uint8_t descIndex = (uint8_t) (ctrl->wValue & 0xFF);
                    length = le16ToCpu(configDesc->wTotalLength);
                    current_speed = dev->speed;

                    // select descriptors according to actual speed
                    switch (dev->speed) {

                    case CH9_USB_SPEED_FULL:
                        endpointEpInDesc.wMaxPacketSize = cpuToLe16(1023);
                        endpointEpOutDesc.wMaxPacketSize = cpuToLe16(1023);
                        break;

                    case CH9_USB_SPEED_HIGH:
                        switch(descIndex) {

                        case 0:
                            endpointEpInDesc.wMaxPacketSize = cpuToLe16(0x400);
                            endpointEpOutDesc.wMaxPacketSize = cpuToLe16(0x400);
                            confHsDesc.bConfigurationValue = 1;
                            break;

                        case 1:
                            endpointEpInDesc.wMaxPacketSize = cpuToLe16(0xC00);
                            endpointEpOutDesc.wMaxPacketSize = cpuToLe16(0xC00);
                            confHsDesc.bConfigurationValue = 2;
                            break;

                        case 2:
                            endpointEpInDesc.wMaxPacketSize = cpuToLe16(0x1400);
                            endpointEpOutDesc.wMaxPacketSize = cpuToLe16(0x1400);
                            confHsDesc.bConfigurationValue = 3;
                            break;

                        default:
                            endpointEpInDesc.wMaxPacketSize = cpuToLe16(1024);
                            endpointEpOutDesc.wMaxPacketSize = cpuToLe16(1024);
                            confHsDesc.bConfigurationValue = 1;
                        }
                        break;

                    case CH9_USB_SPEED_SUPER:
                        endpointEpInDesc.wMaxPacketSize = cpuToLe16(1024);
                        endpointEpOutDesc.wMaxPacketSize = cpuToLe16(1024);
                        switch(descIndex) {

                        case 0:
                            compDesc.bmAttributes = 0,
                            compDesc.wBytesPerInterval = cpuToLe16(1024 * 1);
                            confSsDesc.bConfigurationValue = 1;
                            break;

                        case 1:
                            compDesc.bmAttributes = 1,
                            compDesc.wBytesPerInterval = cpuToLe16(1024 * 2);
                            confSsDesc.bConfigurationValue = 2;
                            break;

                        case 2:
                            compDesc.bmAttributes = 2,
                            compDesc.wBytesPerInterval = cpuToLe16(1024 * 3);
                            confSsDesc.bConfigurationValue = 3;
                            break;

                        default:
                            compDesc.bmAttributes = 0,
                            compDesc.wBytesPerInterval = cpuToLe16(1024 * 1);
                        }
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

                    memmove(&ep0Buff[offset], &compDesc, CH9_USB_DS_SS_USB_ENDPOINT_COMPANION);
                    offset += CH9_USB_DS_SS_USB_ENDPOINT_COMPANION;

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

            uint8_t epInBuffering = configDriver.epIN[(EP_IN & 0x0F) - 1].bufferingValue;
            uint8_t epOutBuffering = configDriver.epOUT[(EP_OUT & 0x0F) - 1].bufferingValue;

            vDbgMsg(DBG_USB_CTRL_MSG, 1, "SET CONFIGURATION(%d)\n", le16ToCpu(ctrl->wValue));
            if (ctrl->wValue > 3) { return -1;       // no such configuration
            }
            // unconfigure device
            if (ctrl->wValue == 0) {
                configValue = 0;

                for (list = dev->epList.next; list != &dev->epList; list = list->next) {
                    ep = (struct CUSBD_Ep *) list;
                    if (ep->address == EP_IN) {
                        ep->ops->epDisable(pD, ep);
                        break;
                    }
                }
                for (list = dev->epList.next; list != &dev->epList; list = list->next) {
                    ep = (struct CUSBD_Ep *) list;
                    if (ep->address == EP_OUT) {
                        ep->ops->epDisable(pD, ep);
                        break;
                    }
                }

                return 0;
            }

            // device already configured
            if (configValue == 1 && ctrl->wValue == 1) { return 0;}
            if (configValue == 2 && ctrl->wValue == 2) { return 0;}
            if (configValue == 3 && ctrl->wValue == 3) { return 0;}

            if(epInBuffering < ctrl->wValue  || epOutBuffering < ctrl->wValue) {
                /*We cant run this test*/
                return -1;
            }

            // configure device
            configValue = (uint8_t) ctrl->wValue;
            for (list = dev->epList.next; list != &dev->epList; list = list->next) {
                ep = (struct CUSBD_Ep *) list;
                if (ep->address == EP_IN) {
                    ep->ops->epEnable(pD, ep, &endpointEpInDesc);
                    epIn = ep;
                    break;
                }
            }
            for (list = dev->epList.next; list != &dev->epList; list = list->next) {
                ep = (struct CUSBD_Ep *) list;
                if (ep->address == EP_OUT) {
                    ep->ops->epEnable(pD, ep, &endpointEpOutDesc);
                    epOut = ep;
                    break;
                }
            }

            if(inReq) {
                inReq->status = 0;
            }
            if(outReq) {
                outReq->status = 0;
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

static void suspend(void *pD) {
    vDbgMsg(DBG_USB_CTRL_MSG, 1, "Application: suspend %c\n", ' ');
}

static void * requestMemAlloc(void * pD, uint32_t requireSize) {
    return malloc(requireSize);
}

static void requestMemFree(void * pD, void* usbRequest) {
    if (usbRequest) {
        free(usbRequest);
    }
}

void busInterval(){

    if(configValue) {
        if(outReq->status == 0) {
            printf("---ITP---\n");
        }
        epOut->ops->reqQueue(pD, epOut, outReq);



    }
}

static CUSBD_Callbacks callback =
{
    .connect = connect,
    .disconnect = disconnect,
    .resume = resume,
    .setup = setup,
    .suspend = suspend,
    .usbRequestMemFree = requestMemFree,
    .usbRequestMemAlloc = requestMemAlloc,
    .busInterval = busInterval
};

static void outCmpl(struct CUSBD_Ep *ep, struct CUSBD_Req * req){
    if (req->status != 0) { return;}
    vDbgMsg(DBG_USB_CTRL_MSG, 1, "Transfer complete on ep:%02X %08X req\n", ep->address, (uintptr_t) req);
}

static void inCmpl(struct CUSBD_Ep *ep, struct CUSBD_Req * req){
    if (req->status != 0) { return;}
    vDbgMsg(DBG_USB_CTRL_MSG, 1, "Transfer complete on ep:%02X %08X req\n", ep->address, (uintptr_t) req);
    ++(*(uint8_t*)req->buf);
    ep->ops->reqQueue(pD, ep, req);
}

// interrupt handler
void isoIsr() {
    drv->isr(pD);
}

static void prepareRequests(uint8_t configValue) {
    disable_int();
    drv->reqAlloc(pD, epIn, &inReq);
    drv->reqAlloc(pD, epOut, &outReq);
    enable_int();
    clearReq(outReq);
    outReq->buf = outBuf;
    outReq->dma = (uintptr_t) outBuf;
    outReq->complete = outCmpl;
    outReq->noInterrupt = 1;

    clearReq(inReq);
    inReq->buf = inBuf;
    inReq->dma = (uintptr_t) inBuf;
    inReq->complete = inCmpl;
    inReq->noInterrupt = 1;
    disable_int();

    switch(configValue) {
    case 1:
        inReq->length = 1024 * 1 - 1;
        //inReq->length = 99;
        outReq->length = 1024 * 1;
        break;

    case 2:
        inReq->length = 1024 * 2 - 1;
        //inReq->length = 99;
        outReq->length = 1024 * 2;
        break;

    case 3:
        inReq->length = 1024 * 3 - 1;
        //inReq->length = 99;
        outReq->length = 1024 * 3;
        break;

    default:
        inReq->length = 1024 * 1 - 1;
        //inReq->length = 99;
        outReq->length = 1024 * 1;
    }

    epIn->ops->reqQueue(pD, epIn, inReq);

    enable_int();
}

static void removeRequests() {
    disable_int();
    drv->reqFree(pD, epIn, inReq);
    drv->reqFree(pD, epOut, outReq);
    enable_int();
}

// Application for testing isochronous transfers
int isoApp() {

    uint32_t res; // keeps result of operation on driver
    CUSBD_SysReq sysReq;
    CUSBD_Dev * dev; // auxiliary pointer used for testing purposes

    drv = CUSBD_GetInstance(); // get driver handle

    res = drv->probe(&configDriver, &sysReq);
    if (res != 0) { goto error;}
    vDbgMsg(DBG_USB_CTRL_MSG, 1, "Device memory requirement: %ld bytes\n", sysReq.privDataSize);

    // allocate memory for device object
    pD = (uint8_t*) malloc((size_t) (sysReq.privDataSize));

    configDriver.trbAddr = malloc(sysReq.trbMemSize);
    if (!configDriver.trbAddr) {
        goto error;
    }
    configDriver.trbDmaAddr = (uintptr_t) configDriver.trbAddr;

    res = drv->init(pD, &configDriver, &callback);
    if (res != 0) { goto error;}
    vDbgMsg(DBG_USB_CTRL_MSG, 1, "Initializing OK! %s\n", " ");

    // set unicode strings
    get_unicode_string(vendorDesc, USB_MANUFACTURER_STRING);
    get_unicode_string(productDesc, USB_PRODUCT_STRING);
    get_unicode_string(serialDesc, USB_SERIAL_NUMBER_STRING);

    // align buffers
    ep0Buff = alignBuff(ep0BuffAlloc, 8);
    inBuf = alignBuff(inBufAlloc, 8);
    outBuf = alignBuff(outBufAlloc, 8);
    memset(ep0BuffAlloc, 0, sizeof(ep0BuffAlloc));
    memset(inBufAlloc, 0, sizeof(inBufAlloc));
    memset(outBufAlloc, 0, sizeof(outBufAlloc));

    // checking device and endpoints parameters correctness
    // get CUSBD device instance exposed as Linux gadget device
    drv->getDevInstance(pD, &dev);

    // allocate request for ep0
    drv->reqAlloc(pD, dev->ep0, &ep0Req);

    /*Change descriptor for maxSpeed == HS only Device*/
    /*For USB2.0 we have to modified wTotalLength of BOS descriptor*/
    if (dev->maxSpeed == CH9_USB_SPEED_HIGH) {
        bosDesc.wTotalLength = cpuToLe16(CH9_USB_DS_BOS + CH9_USB_DS_DEVICE_CAPABILITY_20);
        bosDesc.bNumDeviceCaps = 1;
        devHsDesc.bcdUSB = cpuToLe16(BCD_USB_HS_ONLY);
    }
    // start driver
    drv->start(pD);

unconfigured:
    while (!configValue) {
    }

    prepareRequests(configValue);

    while (1) {
        if(!configValue) { goto unconfigured;}
    }

    drv->stop(pD);
    drv->destroy(pD);

    // free allocated memory
    if (pD) {
        free(pD);
    }

    return (EXIT_SUCCESS);

error:
    vDbgMsg(DBG_USB_CTRL_MSG, 1, "Error %08lX\n", res);
    // free allocated memory
    if (pD) { free(pD);}
    if (configDriver.trbAddr) {
        free (configDriver.trbAddr);
    }
    return (res);
}
