/**********************************************************************
 * Copyright (C) 2014-2017 Cadence Design Systems, Inc.
 * All rights reserved worldwide.
 ***********************************************************************
 * hid_app.c
 * Example HID application for Device mode.
 *
 ***********************************************************************/
#include "dwc2_stdio.h"
#include <stdlib.h>
#include "dwc2_string.h"
#include "dwc2_stdio.h"
#include "dwc2_drv_if.h"
#include "byteorder.h"
#include "log.h"
#include "dwc2_errno.h"
#include "map_system_memory.h"

#define DBG_USB_CTRL_MSG 0x00000002


// Super Speed USB object configuration
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


static CUSBD_OBJ * drv; // driver pointer
static void * pD; // private data pointer


// interrupt handler

void hidIsr() {
    drv->isr(pD);
}
//--------------- HID class types-----------------------------------------------

enum HIDRequest {
    GET_REPORT = 1,
    GET_IDLE,
    GET_PROTOCOL,

    SET_REPORT = 9,
    SET_IDLE,
    SET_PROTOCOL
};

struct usb_hid_interface_descriptor {
    uint8_t bLength; /**< Descriptor size */
    uint8_t bDescriptorType; /**< INTERFACE: 8 bit DESCR_TYPE value */
    uint16_t bcdHID; /**< HID Class specification release */
    uint8_t bCountryCode; /**< Country code of the localized hardware */
    uint8_t bNumDescriptors; /**< Number of class descriptors count */
    uint8_t bDescriptorType2; /**< Type of class descriptor */
    uint16_t wDescriptorLength; /**< Size of the Report descriptor */
} __attribute__((packed));

//-------------- definitions used during configuration -------------------------

#define BCD_USB_SS                      0x0300  // 3.00 version USB
#define ID_VENDOR                       0x0559  // Cadence  VID

#ifdef HS_FS_ONLY
#define ID_PRODUCT                      0x1003  // example hid product
#else
#define ID_PRODUCT                      0x4534  // example hid product
#endif

#define BCD_DEVICE_SS                   0x0001  // 0.1

#define BCD_USB_HS_ONLY                 0x0201  // 2.01  /*Only HS with BOS descriptor*/
#define BCD_USB_HS                      0x0210  // 2.10

#define BCD_DEVICE_HS                   0x0200  // 2.00

#define USB_MANUFACTURER_STRING         "CADENCE"
#define USB_PRODUCT_STRING              "Hid example device"
#define USB_SERIAL_NUMBER_STRING        "100000000000" // should 12 chars long
#define MOUSE_EP_ADDRES_IN 0x81
//------------------------------------------------------------------------------

static uint8_t* hidBuf, *ep0Buff; // keeps mouse report
static uint8_t hidBufAlloc[3 + 7] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
static uint8_t ep0BuffAlloc[100 + 7];
static struct CUSBD_Req *hidReq, *ep0Req; // request used by driver
static uint8_t t1 = 0; // variable used in mouse cursor movement
static uint8_t t2 = 0; // variable used in mouse cursor movement
static uint8_t volatile configValue = 0;

// debug functions
static void displayRequestinfo(struct CUSBD_Req * req);
static void displayEndpointInfo(struct CUSBD_Ep * ep);
static void displayDeviceInfo(CUSBD_Dev * dev);

static void hidReportCmpl(struct CUSBD_Ep *ep, struct CUSBD_Req * req) {
    vDbgMsg(DBG_USB_CTRL_MSG, 1, "HID report complete on %08X req\n", (uintptr_t) req);
    displayRequestinfo(req);
    if (req->status != 0) {return;}

    switch (t1) {
    case 0:
        hidBuf[0] = 0;     // compose IN report,
        hidBuf[1] = 1;     // move cursor to right
        hidBuf[2] = 0;
        break;

    case 1:
        hidBuf[0] = 0;     // compose IN report
        hidBuf[1] = 0;
        hidBuf[2] = 1;     // move cursor down
        break;

    case 2:
        hidBuf[0] = 0;     // compose IN report
        hidBuf[1] = -1;     // move cursor to left
        hidBuf[2] = 0;
        break;

    case 3:
        hidBuf[0] = 0;     // compose IN report
        hidBuf[1] = 0;
        hidBuf[2] = -1;     // move cursor up
        break;
    }
    ++t2;
    if (t2 > 5) {
        t2 = 0;
        t1++;
        if (t1 > 3) {t1 = 0;}
    }

    ep->ops->reqQueue(pD, ep, req);
}

// clears hid report buffer

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
    .bNumConfigurations = 1
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
    .bNumConfigurations = 1
};
//------------- Start of Super Speed configuration descriptors -----------------
// configuration descriptors for SuperSpeed mode
static CH9_UsbConfigurationDescriptor confSsDesc = {
    .bLength = CH9_USB_DS_CONFIGURATION,
    .bDescriptorType = CH9_USB_DT_CONFIGURATION,
    .wTotalLength = cpuToLe16(
        CH9_USB_DS_CONFIGURATION
        + CH9_USB_DS_INTERFACE
        + CH9_USB_DS_ENDPOINT
        + CH9_USB_DS_SS_USB_ENDPOINT_COMPANION
        + sizeof (struct usb_hid_interface_descriptor)),
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
    .bNumEndpoints = 1,
    .bInterfaceClass = CH9_USB_CLASS_HID,
    .bInterfaceSubClass = 0,
    .bInterfaceProtocol = 0,
    .iInterface = 0
};

//0x09, 0x21, 0x01, 0x01, 0x00, 0x01, 0x22, 50, 0x00,
static struct usb_hid_interface_descriptor hidInterfaceDesc = {
    .bLength = 0x09,
    .bDescriptorType = 0x21,
    .bcdHID = cpuToLe16(0x0101),
    .bCountryCode = 0x00,
    .bNumDescriptors = 0x01,
    .bDescriptorType2 = 0x22,
    .wDescriptorLength = cpuToLe16(50)
};

//0x07, 0x05, MOUSE_EP_ADDRES_IN, 0x03, 0x03, 0x00, 0x07,
static CH9_UsbEndpointDescriptor endpointDesc = {
    .bLength = CH9_USB_DS_ENDPOINT,
    .bDescriptorType = CH9_USB_DT_ENDPOINT,
    .bEndpointAddress = MOUSE_EP_ADDRES_IN,
    .bmAttributes = CH9_USB_EP_INTERRUPT,
    .wMaxPacketSize = cpuToLe16(3),
    .bInterval = 1
};

//0x06, 0x30, 0x00, 0x00, 0x00, 0x00
static CH9_UsbSSEndpointCompanionDescriptor compDesc = {
    .bLength = CH9_USB_DS_SS_USB_ENDPOINT_COMPANION,
    .bDescriptorType = CH9_USB_DT_SS_USB_ENDPOINT_COMPANION,
    .bMaxBurst = 0,
    .bmAttributes = 0,
    .wBytesPerInterval = cpuToLe16(0)
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
        + CH9_USB_DS_ENDPOINT
        + sizeof (struct usb_hid_interface_descriptor)),
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
    .bmAttributes = cpuToLe32(CH9_USB_USB20_EXT_LPM_SUPPORT | CH9_USB_USB20_EXT_BESL_AND_ALTERNATE_HIRD)
};
//-------------- BOS descriptor set end ----------------------------------------

static uint8_t hidReportDesc[] = {
    0x05, 0x01, // USAGE_PAGE (Generic Desktop)
    0x09, 0x02, // USAGE (Mouse)
    0xa1, 0x01, // COLLECTION (Application)
    0x09, 0x01, //   USAGE (Pointer)
    0xa1, 0x00, //   COLLECTION (Physical)
    0x05, 0x09, //     USAGE_PAGE (Button)
    0x19, 0x01, //     USAGE_MINIMUM (Button 1)
    0x29, 0x03, //     USAGE_MAXIMUM (Button 3)
    0x15, 0x00, //     LOGICAL_MINIMUM (0)
    0x25, 0x01, //     LOGICAL_MAXIMUM (1)
    0x95, 0x03, //     REPORT_COUNT (3)
    0x75, 0x01, //     REPORT_SIZE (1)
    0x81, 0x02, //     INPUT (Data,Var,Abs)
    0x95, 0x01, //     REPORT_COUNT (1)
    0x75, 0x05, //     REPORT_SIZE (5)
    0x81, 0x03, //     INPUT (Cnst,Var,Abs)
    0x05, 0x01, //     USAGE_PAGE (Generic Desktop)
    0x09, 0x30, //     USAGE (X)
    0x09, 0x31, //     USAGE (Y)
    0x15, 0x81, //     LOGICAL_MINIMUM (-127)
    0x25, 0x7f, //     LOGICAL_MAXIMUM (127)
    0x75, 0x08, //     REPORT_SIZE (8)
    0x95, 0x02, //     REPORT_COUNT (2)
    0x81, 0x06, //     INPUT (Data,Var,Rel)
    0xc0, //   END_COLLECTION
    0xc0 // END_COLLECTION
};

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

static void connect(void *pD) {
    CUSBD_Dev *dev;


    drv->getDevInstance(pD, &dev);
    vDbgMsg(DBG_USB_CTRL_MSG, 1, "Application: connect at %d speed\n", dev->speed);
}

static void disconnect(void *pD) {
    CUSBD_Dev *dev;

    if(!drv) {
        return;
    }

    drv->getDevInstance(pD, &dev);
    vDbgMsg(DBG_USB_CTRL_MSG, 1, "Application: disconnect %s\n", " ");
    displayDeviceInfo(dev);
    configValue = 0;
}

static void resume(void *pD) {
    vDbgMsg(DBG_USB_CTRL_MSG, 1, "Application: resume %c\n", ' ');
}

static void reqComplete(struct CUSBD_Ep *ep, struct CUSBD_Req * req) {
    vDbgMsg(DBG_USB_CTRL_MSG, 1, "Request on endpoint %s completed\n", ep->name);
    //displayRequestinfo(req);

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
                case 0x22:
                    length = sizeof (hidReportDesc);
                    memmove(ep0Buff, &hidReportDesc, length);
                    break;

                case 0x21:
                    length = sizeof (struct usb_hid_interface_descriptor);
                    memmove(ep0Buff, &hidInterfaceDesc, length);
                    break;

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
                            vDbgMsg(DBG_USB_CTRL_MSG, 1, "%02X\n ", ep0Buff[i]);
                        }
                        vDbgMsg(DBG_USB_CTRL_MSG, 1, " %c\n", ' ');
                    }
                    break;

                case CH9_USB_DT_CONFIGURATION:
                {
                    int offset = 0;

                    length = le16ToCpu(configDesc->wTotalLength);

                    memmove(&ep0Buff[offset], configDesc, CH9_USB_DS_CONFIGURATION);
                    offset += CH9_USB_DS_CONFIGURATION;

                    memmove(&ep0Buff[offset], &interfaceDesc, CH9_USB_DS_INTERFACE);
                    offset += CH9_USB_DS_INTERFACE;

                    memmove(&ep0Buff[offset], &hidInterfaceDesc, sizeof (struct usb_hid_interface_descriptor));
                    offset += sizeof (struct usb_hid_interface_descriptor);

                    memmove(&ep0Buff[offset], &endpointDesc, CH9_USB_DS_ENDPOINT);
                    offset += CH9_USB_DS_ENDPOINT;

                    if(dev->speed == CH9_USB_SPEED_SUPER) {
                        memmove(&ep0Buff[offset], &compDesc, CH9_USB_DS_SS_USB_ENDPOINT_COMPANION);
                        offset += CH9_USB_DS_SS_USB_ENDPOINT_COMPANION;
                    }
                    vDbgMsg(DBG_USB_CTRL_MSG, 1, "ConfDesc[0] = %02X\n", configDesc->bLength);
                    {
                        int i;

                        for (i = 0; i < length; i++) {
                            vDbgMsg(DBG_USB_CTRL_MSG, 1, "%02X\n", ep0Buff[i]);
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
                    if(le32ToCpu(capabilityExtDesc.bmAttributes) & CH9_USB_USB20_EXT_LPM_SUPPORT) {
                        return -1;
                    }

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
                            vDbgMsg(DBG_USB_CTRL_MSG, 1, "%02X\n", ep0Buff[i]);
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

                    memmove(&ep0Buff[offset], &hidInterfaceDesc, sizeof (struct usb_hid_interface_descriptor));
                    offset += sizeof (struct usb_hid_interface_descriptor);

                    memmove(&ep0Buff[offset], &endpointDesc, CH9_USB_DS_ENDPOINT);
                    offset += CH9_USB_DS_ENDPOINT;
                    memmove(&ep0Buff[offset], &compDesc, CH9_USB_DS_SS_USB_ENDPOINT_COMPANION);
                    vDbgMsg(DBG_USB_CTRL_MSG, 1, "OtherSpeedDesc[0] = %02X\n", configDesc->bLength);
                    {
                        int i;

                        for (i = 0; i < length; i++) {
                            vDbgMsg(DBG_USB_CTRL_MSG, 1, "%02X\n", ep0Buff[i]);
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
            struct CUSBD_ListHead *list;

            vDbgMsg(DBG_USB_CTRL_MSG, 1, "SET CONFIGURATION(%d)\n", le16ToCpu(ctrl->wValue));
            if (ctrl->wValue > 1) { return -1;       // no such configuration
            }
            // unconfigure device
            if (ctrl->wValue == 0) {
                configValue = 0;
                for (list = dev->epList.next; list != &dev->epList; list = list->next) {
                    ep = (struct CUSBD_Ep *) list;
                    if (ep->address == MOUSE_EP_ADDRES_IN) {
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
                if (ep->address == MOUSE_EP_ADDRES_IN) {
                    ep->ops->epEnable(pD, ep, &endpointDesc);
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

        switch (ctrl->bRequest) {

        case SET_IDLE:
            return -1;
            break;

        case GET_REPORT:         // not supported in winXP
            memset(ep0Buff, 0, 3);
            length = 3;         // size of report

        case GET_IDLE:
            return -1;
            break;

        default:
            return -1;
        }

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

static void * requestMemAlloc(void * pD, uint32_t requireSize) {
    return malloc(requireSize);
}

static void requestMemFree(void * pD, void* usbRequest) {
    if(usbRequest) {
        free(usbRequest);
    }
}

static CUSBD_Callbacks callback = {
    .connect = connect,
    .disconnect = disconnect,
    .resume = resume,
    .setup = setup,
    .suspend = suspend,
    .usbRequestMemFree = requestMemFree,
    .usbRequestMemAlloc = requestMemAlloc
};

/*
 *
 */
int hidApp() {

    uint32_t res; // keeps result of operation on driver
    CUSBD_SysReq sysReq;
    CUSBD_Dev * dev; // auxiliary pointer used for testing purposes
    struct CUSBD_Ep * ep;

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
    hidBuf = alignBuff(hidBufAlloc, 8);
    ep0Buff = alignBuff(ep0BuffAlloc, 8);

    // checking device and endpoints parameters correctness
    // get CUSBD device instance exposed as Linux gadget device
    drv->getDevInstance(pD, &dev);
    displayDeviceInfo(dev);

    // allocate request for ep0
    drv->reqAlloc(pD, dev->ep0, &ep0Req);

    // for each of endpoint print info
    {
        CUSBD_ListHead *list;
        for (list = dev->epList.next; list != &dev->epList; list = list->next) {
            ep = (struct CUSBD_Ep *) list;
            displayEndpointInfo(ep);
            if (ep->address == MOUSE_EP_ADDRES_IN) {
                drv->reqAlloc(pD, ep, &hidReq);
                break;
            }
        }
    }

    /*Change descriptor for maxSpeed == HS only Device*/
    /*For USB2.0 we have to modified wTotalLength of BOS descriptor*/
    if(dev->maxSpeed == CH9_USB_SPEED_HIGH || dev->maxSpeed == CH9_USB_SPEED_FULL) {
        bosDesc.wTotalLength = cpuToLe16(CH9_USB_DS_BOS + CH9_USB_DS_DEVICE_CAPABILITY_20);
        bosDesc.bNumDeviceCaps = 1;
        devHsDesc.bcdUSB = cpuToLe16(BCD_USB_HS_ONLY);
    }

    drv->start(pD);

back:
    // wait for device configured state
    while (!configValue) {;}

    clearReq(hidReq);
    hidBuf[0] = 3;
    hidBuf[1] = 4;
    hidBuf[2] = 5;
    hidReq->buf = hidBuf;
    hidReq->dma = (uintptr_t) hidBuf;
    hidReq->complete = hidReportCmpl;
    hidReq->length = 3;
    disable_int();
    ep->ops->reqQueue(pD, ep, hidReq);
    enable_int();

    // wait until device in default state
    while (configValue) {;}
    goto back;

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
    if (config.trbAddr) {
        free (config.trbAddr);
    }
    return (res);
}
