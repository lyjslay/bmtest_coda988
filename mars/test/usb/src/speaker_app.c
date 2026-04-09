/**********************************************************************
 * Copyright (C) 2017 Cadence Design Systems, Inc.
 * All rights reserved worldwide.
 ***********************************************************************
 * speaker_app.c
 * USB audio class speaker example
 ***********************************************************************/
#include <stdlib.h>                        // used for malloc
#include "cdn_stdio.h"                     // standard library
#include "cdn_string.h"                    // standard library

#include "cusbd_if.h"                      // USB controller interface
#include "byteorder.h"                     // endian macros
#include "cdn_errno.h"                     // errors definitions
#include "log.h"

#include "map_system_memory.h"
#define DBG_USB_CTRL_MSG 0x00000002


// Super Speed USB object configuration
#define USB_REGS_BASE 0x50080000           // base address of SFR of USB core
#define STORAGE_BASE 0x10000               // physical address of storage device

static CUSBD_Config config = {
    .regBase = USB_REGS_BASE, // address where USB core is mapped
    .epIN =
    {
        {.bufferingValue = 4}, // ep1in
        {.bufferingValue = 4}, // ep2in
        {.bufferingValue = 4}, // ep3in
        {.bufferingValue = 4}, // ep4in
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
        {.bufferingValue = 4}, //ep2out
        {.bufferingValue = 4}, //ep3out
        {.bufferingValue = 4}, //ep4out
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


#define BCD_USB_HS_ONLY                 0x0201  // 2.01  /*Only HS with BOS descriptor*/
#define ENDPOINT_IN                     0x83
#define ENDPOINT_OUT                    0x03

#define BCD_USB_SS                      0x0300  // 3.00 version USB
#define ID_VENDOR                       0x0559  // CADENCE

#ifdef HS_FS_ONLY
#define ID_PRODUCT                      0x1003  // Product
#else
#define ID_PRODUCT                      0x0004  // Product
#endif

#define BCD_DEVICE_SS                   0x0010  // 0.1

#ifdef HS_FS_ONLY
#define BCD_USB_HS                      0x0200  // 2.00
#else
#define BCD_USB_HS                      0x0210  // 2.10
#endif

#define BCD_DEVICE_HS                   0x0200  // 2.00

#define USB_MANUFACTURER_STRING         "CADENCE"
#define USB_PRODUCT_STRING              "USB speakers"
#define USB_SERIAL_NUMBER_STRING        "100000000000" // should 12 chars long

/* Interfaces */
#define UVC_AUDIO_COLLECTION   0
#define UVC_AUDIO_INTF_IDLE    0
#define UVC_AUDIO_INTF_STREAM  1

/* Audio unit IDs */
#define UVC_AUDIO_IT_ID        1
#define UVC_AUDIO_FC_ID        2
#define UVC_AUDIO_OT_ID        3

/* Endpoints */
#define AUDIO_ENDPOINT_ADD     ENDPOINT_OUT

/* Class-specific Request Type */
#define USB_SET_CUR            0x01
#define USB_GET_CUR            0x81
#define USB_GET_MIN            0x82
#define USB_GET_MAX            0x83
#define USB_GET_RES            0x84
#define USB_GET_LEN            0x85
#define USB_GET_INFO           0x86
#define USB_GET_DEF            0x87

static CUSBD_OBJ * drv; // driver pointer
static void * pD; // private data pointer


static volatile uint8_t configValue = 0; // keeps actual configuration value
static uint8_t *ep0Buff;
static uint8_t ep0BuffAlloc[200 + 7];
static struct CUSBD_Req *ep0Req;
static CH9_UsbEndpointDescriptor *endpointDesc;

// static allocation of endpoints objects
static struct CUSBD_Ep * epIn;
static uint8_t *epInBuff;
static uint8_t  epInBuffAlloc[1024 * 3 + 7];
static struct CUSBD_Req *epInReq;

// audio controls
static uint8_t altSetting = 0;
static uint16_t volumeValue = 0;

// debug functions
static void displayRequestinfo(struct CUSBD_Req * req);
static void displayEndpointInfo(struct CUSBD_Ep * ep);
static void displayDeviceInfo(CUSBD_Dev * dev);


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
static CH9_UsbDeviceDescriptor devSsDesc = {CH9_USB_DS_DEVICE,
                                            CH9_USB_DT_DEVICE, cpuToLe16(BCD_USB_SS), 0, 0, 0, 9,
                                            cpuToLe16(ID_VENDOR), cpuToLe16(ID_PRODUCT), cpuToLe16(BCD_DEVICE_SS),
                                            1, 2, 3, 1};

// device descriptor for HighSpeed mode
static CH9_UsbDeviceDescriptor devHsDesc = {CH9_USB_DS_DEVICE,
                                            CH9_USB_DT_DEVICE, cpuToLe16(BCD_USB_HS), 0, 0, 0, 64,
                                            cpuToLe16(ID_VENDOR), cpuToLe16(ID_PRODUCT), cpuToLe16(BCD_DEVICE_HS),
                                            1, 2, 3, 1};

static const uint8_t confDesc[] = {
    /* configuration descriptor */
    0x09,          /* bLength */
    0x02,          /* bDescriptorType */
    0xA5 + 12,0x00,     /* wTotalLength */
    0x02,          /* bNumInterface */
    0x01,          /* bConfigurationValue */
    0x00,          /* iConfiguration - index string not used */
    0xC0,          /* self powered */
    0x00,          /* bMaxPower */

    /*Interface descriptor     */
    0x09,          /* bLength */
    0x04,          /* bDescriptorType INTERFACE */
    0x00,          /* bInterfaceNumber */
    0x00,          /* bAlternateSetting */
    0x00,          /* bNumEndpoints */
    0x01,          /* bInterfaceClass Audio */
    0x01,          /* bInterfaceSubClass Audio Control */
    0x00,          /* None */
    0x00,          /* iInterface */

    /* Audio Descriptor */
    0x09,          /* bLength */
    0x24,          /* bDescriptorType CS_INTERFACE */
    0x01,          /* bDescriptorSubtype HEADER */
    0x00,0x01,     /* bcdADC 1.0 */
    0x28,0x00,     /* wTotalLength 40 u8s */
    0x01,          /* bInCollection 1 */
    0x01,          /* baInterfaceNr(1) 1 */

    /* Audio Descriptor */
    0x0C,          /* bLength */
    0x24,          /* bDescriptorType CS_INTERFACE */
    0x02,          /* bDescriptorSubtype INPUT_TERMINAL */
    0x01,          /* bTerminalID 1 */
    0x01,0x01,     /* wTerminalType USB streaming */
    0x00,          /* bAssocTerminal 0 */
    0x02,          /* bNrChannels 2 */
    0x03,          /* wChannelConfig. Left Front (L) & wChannelConfig. Right Front (R) */
    0x00,          /* wChannelConfig. Reserved 0x0 */
    0x00,          /* iChannelNames 0 */
    0x00,          /* iTerminal 0 */

    /* Audio Descriptor */
    0x09,          /* bLength */
    0x24,          /* bDescriptorType CS_INTERFACE */
    0x03,          /* bDescriptorSubtype OUTPUT_TERMINAL */
    0x02,          /* bTerminalID 2 */
    0x01,0x03,     /* wTerminalType Speaker */
    0x00,          /* bAssocTerminal 0 */
    0x03,          /* bSourceID 3 */
    0x00,          /* iTerminal 0 */

    /* Audio Descriptor */
    0x0A,          /* bLength */
    0x24,          /* bDescriptorType CS_INTERFACE */
    0x06,          /* bDescriptorSubtype FEATURE_UNIT */
    0x03,          /* bUnitID */
    0x01,          /* bSourceID */
    0x01,          /* bControlSize */
    0x03,          /* bmaControls(0). Mute & bmaControls(0). Volume */
    0x02,          /* bmaControls(1). Volume */
    0x02,          /* bmaControls(2). Volume */
    0x00,          /* iFeature */

    /* Interface descriptor */
    0x09,          /* bLength */
    0x04,          /* bDescriptorType INTERFACE */
    0x01,          /* bInterfaceNumber */
    0x00,          /* bAlternateSetting */
    0x00,          /* bNumEndpoints */
    0x01,          /* bInterfaceClass Audio  */
    0x02,          /* bInterfaceSubClass Audio Streaming */
    0x00,          /* bInterfaceProtocol None */
    0x00,          /* iInterface */

    /* Interface descriptor */
    0x09,          /* bLength */
    0x04,          /* bDescriptorType INTERFACE */
    0x01,          /* bInterfaceNumber */
    0x01,          /* bAlternateSetting */
    0x01,          /* bNumEndpoints */
    0x01,          /* bInterfaceClass Audio  */
    0x02,          /* bInterfaceSubClass Audio Streaming */
    0x00,          /* bInterfaceProtocol None */
    0x00,          /* iInterface */

    /* Audio Descriptor */
    0x07,          /* bLength  */
    0x24,          /* bDescriptorType CS_INTERFACE */
    0x01,          /* bDescriptorSubtype AS_GENERAL */
    0x01,          /* bTerminalLink 1 */
    0x00,          /* bDelay 0 frames */
    0x01,0x00,     /* wFormatTag PCM */

    /* Audio Descriptor */
    0x11,          /* bLength */
    0x24,          /* bDescriptorType INTERFACE */
    0x02,          /* bDescriptorSubtype FORMAT_TYPE */
    0x01,          /* bFormatType FORMAT_TYPE_I */
    0x02,          /* bNrChannels 2 */
    0x02,          /* bSubframeSize 2 u8s */
    0x10,          /* bBitResolution 16 bits */
    0x03,          /* bSamFreqType 3 discrete sampling frequencies */
    0x00,0x7D,0x00,/* tSamFreq(1) 32.0 kHz */
    0x44,0xAC,0x00,/* tSamFreq(2) 44.1 kHz */
    0x80,0xBB,0x00,/* tSamFreq(3) 48.0 kHz */

    /* Endpoint descriptor */
    0x09,          /* bLength */
    0x05,          /* bDescriptorType ENDPOINT */
    AUDIO_ENDPOINT_ADD,          /* bEndpointAddress 1 OUT */
    0x09,          /* Isochronous & Adaptive & Data endpoint */
    0xC0,0x00,     /* wMaxPacketSize 192 u8s */
    0x04,          /* bInterval 8 uFrames (1ms) <------------ */
    0x00,          /* bRefresh Not used */
    0x00,          /* bSynchAddress Not used */

    0x06, 0x30, /*0x0F*/ 0, 0x00, 200, 0x00,

    /* Audio Descriptor */
    0x07,          /* bLength */
    0x25,          /* bDescriptorType CS_ENDPOINT */
    0x01,          /* bDescriptorSubtype EP_GENERAL */
    0x00,          /* Not suported */
    0x02,          /* bLockDelayUnits Decoded PCM samples */
    0x00,0x02,     /* wLockDelay 512 */

    /* Interface descriptor */
    0x09,          /* bLength */

    0x04,          /* bDescriptorType INTERFACE */
    0x01,          /* bInterfaceNumber 1 */
    0x02,          /* bAlternateSetting 2 */
    0x01,          /* bNumEndpoints1 */
    0x01,          /* bInterfaceClass Audio  */
    0x02,          /* bInterfaceSubClass Audio Streaming */
    0x00,          /* bInterfaceProtocol None */
    0x00,          /* iInterface */

    /* Audio Descriptor */
    0x07,          /* bLength  */
    0x24,          /* CS_INTERFACE */
    0x01,          /* bDescriptorSubtype AS_GENERAL */
    0x01,          /* bTerminalLink */
    0x00,          /* bDelay 0 frames */
    0x01,0x00,     /* wFormatTag PCM */

    /* Audio Descriptor */
    0x11,          /* bLength */
    0x24,          /* CS_INTERFACE */
    0x02,          /* bDescriptorSubtype FORMAT_TYPE */
    0x01,          /* bFormatType FORMAT_TYPE_I */
    0x01,          /* bNrChannels 1 */
    0x02,          /* bSubframeSize 2 u8s */
    0x10,          /* bBitResolution 16 bits */
    0x03,          /* bSamFreqType 3 discrete sampling frequencies */
    0x00,0x7D,0x00,/* tSamFreq(1) 32.0 kHz */
    0x44,0xAC,0x00,/* tSamFreq(2) 44.1 kHz */
    0x80,0xBB,0x00,/* tSamFreq(3) 48.0 kHz */

    /* Endpoint descriptor */
    0x09,          /* bLength */
    0x05,          /* bDescriptorType ENDPOINT */
    AUDIO_ENDPOINT_ADD,          /* bEndpointAddress 1 OUT */
    0x09,          /* Isochronous & Adaptive & Data endpoint */
    0x60,0x00,     /* wMaxPacketSize 96 u8s */
    0x04,          /* bInterval 8 uFrames (1 ms) <------------------ */
    0x00,          /* bRefresh Not used */
    0x00,          /* bSynchAddress Not used */

    0x06, 0x30, /*0x0F*/ 0, 0x00, 200, 0x00,

    /* Audio Descriptor */
    0x07,          /* bLength */
    0x25,          /* bDescriptorType CS_ENDPOINT */
    0x01,          /* bDescriptorSubtype EP_GENERAL */
    0x00,          /* bmAttributes. Not supported */
    0x02,          /* bLockDelayUnits Decoded PCM samples */
    0x00,0x02      /* wLockDelay 512 */
};

// language descriptor for english
static uint8_t languageDesc[] = {4, CH9_USB_DT_STRING, 0x09, 0x04};

// string will be filled then in initializing section
static char vendorDesc[sizeof (USB_MANUFACTURER_STRING) * 2 + 2];
static char productDesc[sizeof (USB_PRODUCT_STRING) * 2 + 2];
static char serialDesc[sizeof (USB_SERIAL_NUMBER_STRING) * 2 + 2];

//-------------- BOS descriptor set start --------------------------------------

static CH9_UsbBosDescriptor bosDesc = {CH9_USB_DS_BOS, CH9_USB_DT_BOS,
                                       cpuToLe16(CH9_USB_DS_BOS + CH9_USB_DS_DEVICE_CAPABILITY_30 + CH9_USB_DS_DEVICE_CAPABILITY_20),
                                       2};

static CH9_UsbSSDeviceCapabilityDescriptor capabilitySsDesc = {CH9_USB_DS_DEVICE_CAPABILITY_30, CH9_USB_DT_DEVICE_CAPABILITY,
                                                               CH9_USB_DCT_SS_USB,
                                                               0, // LTM not supported
                                                               cpuToLe16(CH9_USB_SS_CAP_SUPPORT_SS | CH9_USB_SS_CAP_SUPPORT_HS | CH9_USB_SS_CAP_SUPPORT_FS),
                                                               1, // 1 us
                                                               4, // 4 us
                                                               cpuToLe16(512) // 512 us
};

static CH9_UsbCapabilityDescriptor capabilityExtDesc = {
    CH9_USB_DS_DEVICE_CAPABILITY_20, CH9_USB_DT_DEVICE_CAPABILITY,
    CH9_USB_DCT_USB20_EXTENSION, cpuToLe32(CH9_USB_USB20_EXT_LPM_SUPPORT)
};
//-------------- BOS descriptor set end ----------------------------------------

static CH9_UsbDeviceQualifierDescriptor qualifierDesc = {
    CH9_USB_DS_DEVICE_QUALIFIER, CH9_USB_DT_DEVICE_QUALIFIER,
    cpuToLe16(0x0200), 0x00, 0x00, 0x00, 64, 0x01, 0x00
};

static void connect(void *pD)  {
    CUSBD_Dev *dev;

    drv->getDevInstance(pD, &dev);
    //vDbgMsg(DBG_USB_CTRL_MSG, 1, "Application: connect at %d speed\n", dev->speed);
}

static void disconnect(void *pD)  {
    CUSBD_Dev *dev;

    if (!drv) {
        return;
    }

    drv->getDevInstance(pD, &dev);
    //vDbgMsg(DBG_USB_CTRL_MSG, 1, "Application: disconnect %c\n", ' ');
    displayDeviceInfo(dev);
    configValue = 0;
}

static void resume(void *pD)  {
    vDbgMsg(DBG_USB_CTRL_MSG, 1, "Application: resume %c\n", ' ');
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
        configDesc = (CH9_UsbConfigurationDescriptor *) confDesc;
        devDesc = &devHsDesc;
        break;

    case CH9_USB_SPEED_HIGH:
        configDesc = (CH9_UsbConfigurationDescriptor *) confDesc;
        devDesc = &devHsDesc;
        break;

    case CH9_USB_SPEED_SUPER:
        configDesc = (CH9_UsbConfigurationDescriptor *) confDesc;
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
                            vDbgMsg(DBG_USB_CTRL_MSG, 1, "%02X ", ep0Buff[i]);
                        }
                        vDbgMsg(DBG_USB_CTRL_MSG, 1, " %c\n", ' ');
                    }
                    break;

                case CH9_USB_DT_CONFIGURATION:
                {
                    int offset = 0;

                    length = le16ToCpu(configDesc->wTotalLength);

                    memmove(&ep0Buff[offset], configDesc, /*CH9_USB_DS_CONFIGURATION*/ length);
                    //offset += CH9_USB_DS_CONFIGURATION;

                    vDbgMsg(DBG_USB_CTRL_MSG, 1, "ConfDesc[0] = %02X\n", configDesc->bLength);
                    {
                        int i;

                        for (i = 0; i < length; i++) {
                            vDbgMsg(DBG_USB_CTRL_MSG, 1, "%02X ", ep0Buff[i]);
                        }
                        vDbgMsg(DBG_USB_CTRL_MSG, 1, " %c\n", ' ');
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

            vDbgMsg(DBG_USB_CTRL_MSG, 1, "SET CONFIGURATION(%d)\n", le16ToCpu(ctrl->wValue));
            if (ctrl->wValue > 1) { return -1;       // no such configuration
            }
            // unconfigure device
            if (ctrl->wValue == 0) {
                configValue = 0;
                for (list = dev->epList.next; list != &dev->epList; list = list->next) {
                    ep = (struct CUSBD_Ep *) list;
                    if (ep->address == ENDPOINT_OUT) {
                        ep->ops->epDisable(pD, ep);
                        //displayEndpointInfo(ep);
                        break;
                    }
                }
                return 0;
            }

            // device already configured
            if (configValue == 1 && ctrl->wValue == 1) { return 0;}

            // configure device
            configValue = (uint8_t) ctrl->wValue;
            for (list = dev->epList.next; list != &dev->epList; list = list->next) {
                ep = (struct CUSBD_Ep *) list;
                if (ep->address == ENDPOINT_OUT) {
                    ep->ops->epEnable(pD, ep, endpointDesc);
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
            ep0Buff[0] = configValue;
            break;

        case CH9_USB_REQ_SET_INTERFACE:
            if (ctrl->wIndex != 1) { return -1;}
            altSetting = ctrl->wValue;
            length = 0;
            if (altSetting) {
            } else {
                if(epInReq->status == EINPROGRESS) {
                    epIn->ops->reqDequeue(pD, epIn, epInReq);
                }
            }
            break;

        default:
            vDbgMsg(DBG_USB_CTRL_MSG, 1, "Error_2 %c\n", ' ');
            return -1;         //return error
        }
        break;

    case CH9_USB_REQ_TYPE_CLASS:

        switch (ctrl->bRequest) {
        case USB_SET_CUR:
            volumeValue = *(uint16_t*) ep0Buff;
            length = 2;
            break;
        case USB_GET_CUR:
            *(uint16_t*) ep0Buff = volumeValue;
            length = 2;
            break;
        case USB_GET_MIN:
            *(uint16_t*) ep0Buff = (uint16_t) cpuToLe16(0x8000);
            length = 2;
            break;
        case USB_GET_MAX:
            *(uint16_t*) ep0Buff = (uint16_t) cpuToLe16(0);
            length = 2;
            break;
        case USB_GET_RES:
            *(uint16_t*) ep0Buff = (uint16_t) cpuToLe16(1);
            length = 2;
            break;
        case USB_GET_LEN: break;
        case USB_GET_INFO: break;
        case USB_GET_DEF: break;
        default: return -1;
        }

        break;
    }

    if (length > 0) {
        ep0Req->length = ctrl->wLength < length ? ctrl->wLength : length;
        dev->ep0->ops->reqQueue(pD, dev->ep0, ep0Req);
    }
    return 0;
}

static void suspend(void *pD)  {
    vDbgMsg(DBG_USB_CTRL_MSG, 1, "Application: suspend %c\n", ' ');
}

static void displayRequestinfo(struct CUSBD_Req * req) {
    vDbgMsg(DBG_USB_CTRL_MSG, 1, "-------Request INFO-------------- %c\n", ' ');
    vDbgMsg(DBG_USB_CTRL_MSG, 1, "buf: %08X\n", (uintptr_t) req->buf);
    vDbgMsg(DBG_USB_CTRL_MSG, 1, "length: %ld\n", req->length);
    vDbgMsg(DBG_USB_CTRL_MSG, 1, "dma: %08X\n", req->dma);
    vDbgMsg(DBG_USB_CTRL_MSG, 1, "sg:  %c\n", ' ');
    vDbgMsg(DBG_USB_CTRL_MSG, 1, "numOfSgs %ld\n", req->numOfSgs);
    vDbgMsg(DBG_USB_CTRL_MSG, 1, "numMappedSgs: %ld\n", req->numMappedSgs);
    vDbgMsg(DBG_USB_CTRL_MSG, 1, "streamId: %04X\n", req->streamId);
    vDbgMsg(DBG_USB_CTRL_MSG, 1, "noInterrupt: %d\n", req->noInterrupt);
    vDbgMsg(DBG_USB_CTRL_MSG, 1, "zero: %d\n", req->zero);
    vDbgMsg(DBG_USB_CTRL_MSG, 1, "shortNotOk: %d\n", req->shortNotOk);
    vDbgMsg(DBG_USB_CTRL_MSG, 1, "context: %08X\n", (uintptr_t) req->context);
    vDbgMsg(DBG_USB_CTRL_MSG, 1, "status: %08lX\n", req->status);
    vDbgMsg(DBG_USB_CTRL_MSG, 1, "actual: %ld\n", req->actual);
}

static void displayEndpointInfo(struct CUSBD_Ep * ep) {
    vDbgMsg(DBG_USB_CTRL_MSG, 1, "-------Endpoint INFO------------- %c\n", ' ');
    vDbgMsg(DBG_USB_CTRL_MSG, 1, "address: %02X\n", ep->address);
    vDbgMsg(DBG_USB_CTRL_MSG, 1, "epList: %08X\n", (uintptr_t) &ep->epList);
    vDbgMsg(DBG_USB_CTRL_MSG, 1, "driverData: %08X\n", (uintptr_t) ep->driverData);
    vDbgMsg(DBG_USB_CTRL_MSG, 1, "name: %s\n", ep->name);
    vDbgMsg(DBG_USB_CTRL_MSG, 1, "ops: %c\n", ' ');
    vDbgMsg(DBG_USB_CTRL_MSG, 1, "maxPacket: %d\n", ep->maxPacket);
    vDbgMsg(DBG_USB_CTRL_MSG, 1, "maxStreams: %d\n", ep->maxStreams);
    vDbgMsg(DBG_USB_CTRL_MSG, 1, "mult: %d\n", ep->mult);
    vDbgMsg(DBG_USB_CTRL_MSG, 1, "maxburst: %d\n", ep->maxburst);
    vDbgMsg(DBG_USB_CTRL_MSG, 1, "desc: %08X\n", (uintptr_t) ep->desc);
    vDbgMsg(DBG_USB_CTRL_MSG, 1, "compDesc: %08X\n", (uintptr_t) ep->compDesc);
}

static void displayDeviceInfo(CUSBD_Dev * dev) {

    vDbgMsg(DBG_USB_CTRL_MSG, 1, "-------Device INFO--------------- %c\n", ' ');
    vDbgMsg(DBG_USB_CTRL_MSG, 1, "epList: prev = %08X, next = %08X\n", (uintptr_t) dev->epList.prev, (uintptr_t) dev->epList.next);
    vDbgMsg(DBG_USB_CTRL_MSG, 1, "ep0: %08X\n", (uintptr_t) dev->ep0);
    vDbgMsg(DBG_USB_CTRL_MSG, 1, "speed: %d\n", dev->speed);
    vDbgMsg(DBG_USB_CTRL_MSG, 1, "maxSpeed: %d\n", dev->maxSpeed);
    vDbgMsg(DBG_USB_CTRL_MSG, 1, "state: %d\n", dev->state);
    vDbgMsg(DBG_USB_CTRL_MSG, 1, "sgSupported: %d\n", dev->sgSupported);
    vDbgMsg(DBG_USB_CTRL_MSG, 1, "isOtg: %d\n", dev->isOtg);
    vDbgMsg(DBG_USB_CTRL_MSG, 1, "isAPeripheral: %d\n", dev->isAPeripheral);
    vDbgMsg(DBG_USB_CTRL_MSG, 1, "bHnpEnable: %d\n", dev->bHnpEnable);
    vDbgMsg(DBG_USB_CTRL_MSG, 1, "aHnpSupport: %d\n", dev->aHnpSupport);
    vDbgMsg(DBG_USB_CTRL_MSG, 1, "name: %s\n", dev->name);
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

static void epInCmpl(struct CUSBD_Ep *ep, struct CUSBD_Req * req)  {

    //bulk_debug("Transfer complete on ep:%02X %08X req\n", ep->address, (uintptr_t) req);
    displayRequestinfo(req);
    if(req->status == 0) {
        ep->ops->reqQueue(pD, ep, req);
    }

}

// interrupt handler
extern void disable_int();
extern void enable_int();

void speakerIsr() {
    drv->isr(pD);
}

int speakerApp() {
    uint32_t res; // keeps result of operation on driver
    CUSBD_SysReq sysReq;
    CUSBD_Dev * dev; // auxiliary pointer used for testing purposes
    struct CUSBD_Ep * ep;
    CUSBD_ListHead *list; // used in for_each loop

    drv = CUSBD_GetInstance(); // get driver handle

    res = drv->probe(&config, &sysReq);
    if (res != 0) { goto error;}
    vDbgMsg(DBG_USB_CTRL_MSG, 1, "Device memory requirement: %ld bytes\n", sysReq.privDataSize);

    // allocate memory for device object
    pD = (uint8_t*) malloc((size_t) (sysReq.privDataSize));
    if(!pD) { goto error;}
    config.trbAddr = malloc(sysReq.trbMemSize);
    if(!config.trbAddr) {
        goto error;
    }
    config.trbDmaAddr = (uintptr_t)config.trbAddr;
    res = drv->init(pD, &config, &callback);
    if (res != 0) { goto error;}
    vDbgMsg(DBG_USB_CTRL_MSG, 1, "Initializing OK! %s\n", " ");

    // set unicode strings
    get_unicode_string(vendorDesc, USB_MANUFACTURER_STRING);
    get_unicode_string(productDesc, USB_PRODUCT_STRING);
    get_unicode_string(serialDesc, USB_SERIAL_NUMBER_STRING);

    // align buffer for hid mouse application
    ep0Buff = alignBuff(ep0BuffAlloc, 8);
    epInBuff = alignBuff(epInBuffAlloc, 8);

    // checking device and endpoints parameters correctness
    // get CUSBD device instance exposed as Linux gadget device
    drv->getDevInstance(pD, &dev);
    displayDeviceInfo(dev);

    // allocate request for ep0
    drv->reqAlloc(pD, dev->ep0, &ep0Req);

    endpointDesc = (CH9_UsbEndpointDescriptor *)&confDesc[100];

    /*Change descriptor for maxSpeed == HS only Device*/
    /*For USB2.0 we have to modified wTotalLength of BOS descriptor*/
    if(dev->maxSpeed == CH9_USB_SPEED_HIGH || dev->maxSpeed == CH9_USB_SPEED_FULL) {
        bosDesc.wTotalLength =
            cpuToLe16(CH9_USB_DS_BOS + CH9_USB_DS_DEVICE_CAPABILITY_20);
        bosDesc.bNumDeviceCaps = 1;
        devHsDesc.bcdUSB = cpuToLe16(BCD_USB_HS_ONLY);
    }

    drv->start(pD);
back:
    // wait for device configured state
    while (!configValue) {;}
    disable_int();

    // for each of endpoint print info
    for (list = dev->epList.next; list != &dev->epList; list = list->next) {
        epIn = (struct CUSBD_Ep *) list;
        //displayEndpointInfo(epIn);
        if (epIn->address == ENDPOINT_OUT) {
            drv->reqAlloc(pD, epIn, &epInReq);
            break;
        }
    }

    epInReq->buf = epInBuff;
    epInReq->dma = (uintptr_t) epInBuff;
    epInReq->complete = epInCmpl;
    epInReq->length = 1024;
    enable_int();
    while (!altSetting) {;}
    epIn->ops->reqQueue(pD, epIn, epInReq);
    // wait until device in default state
    while (configValue) {;}
    goto back;

    drv->stop(pD);
    drv->destroy(pD);

    // free allocated memory
    if (pD) {
        free(pD);
    }

    return 0;

error:
    vDbgMsg(DBG_USB_CTRL_MSG, 1, "Error %08lX\n", res);
    // free allocated memory
    if (pD) { free(pD);}
    if (config.trbAddr) {
        free (config.trbAddr);
    }
    return (res);
}
