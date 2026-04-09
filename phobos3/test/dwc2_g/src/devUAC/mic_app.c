/**********************************************************************
 * Copyright (C) 2014-2017 Cadence Design Systems, Inc.
 * All rights reserved worldwide.
 ***********************************************************************
 * mic_app.c
 * USB audio class microphone example
 ***********************************************************************/
#include <stdlib.h>                        // used for malloc
#include "dwc2_stdio.h"                     // standard library
#include "dwc2_string.h"                    // standard library

#include "dwc2_drv_if.h"                      // USB controller interface
#include "byteorder.h"                     // endian macros
#include "dwc2_errno.h"                     // errors definitions
#include "log.h"

// select audio application version, only one should be active at a time
//#define UAC_APP_VER10
//#define UAC_APP_VER20
#define UAC_APP_VER30

#include "map_system_memory.h"
#define DBG_USB_CTRL_MSG 0x00000002

#ifdef UAC_APP_VER10
extern uint8_t *getEpDesc1(CH9_UsbSpeed speed);
extern int getDesc1(CH9_UsbSpeed speed, uint8_t * audioDesc); // audio 1.0 application
extern int acRequest1(CH9_UsbSetup *ctrl);
uint8_t *(*getEpDesc)(CH9_UsbSpeed speed) = getEpDesc1;
int (*audioGetDesc)(CH9_UsbSpeed speed, uint8_t * audioDesc) = getDesc1;
int (*audioAcRequest)(CH9_UsbSetup *ctrl) = acRequest1;
#endif

#ifdef UAC_APP_VER20
extern int getDesc2(CH9_UsbSpeed speed, uint8_t * audioDesc); // audio 2.0 application
extern int acRequest2(CH9_UsbSetup *ctrl);
extern uint8_t *getEpDesc2(CH9_UsbSpeed speed);
uint8_t *(*getEpDesc)(CH9_UsbSpeed speed) = getEpDesc2;
int (*audioGetDesc)(CH9_UsbSpeed speed, uint8_t * audioDesc) = getDesc2;
int (*audioAcRequest)(CH9_UsbSetup *ctrl) = acRequest2;
#endif

#ifdef UAC_APP_VER30
extern int getDesc3(CH9_UsbSpeed speed, uint8_t * audioDesc); // audio 3.0 application
extern int acRequest3(CH9_UsbSetup *ctrl);
extern uint8_t *getEpDesc3(CH9_UsbSpeed speed);
uint8_t *(*getEpDesc)(CH9_UsbSpeed speed) = getEpDesc3;
int (*audioGetDesc)(CH9_UsbSpeed speed, uint8_t * audioDesc) = getDesc3;
int (*audioAcRequest)(CH9_UsbSetup *ctrl) = acRequest3;
#endif


// Super Speed USB object configuration

static CUSBD_Config config = {
    .regBase = USB_REGS_BASE, // address where USB core is mapped
    .epIN =
    {
        {.bufferingValue = 4}, // ep1in
        {.bufferingValue = 4}, // ep2in
        {.bufferingValue = 4}, // ep3in
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


#define BCD_USB_HS_ONLY                 0x0201  // 2.01  /*Only HS with BOS descriptor*/
#define ENDPOINT_IN                     0x83
#define ENDPOINT_OUT                    0x01

#define BCD_USB_SS                      0x0300  // 3.00 version USB
#define ID_VENDOR                       0x0559  // CADENCE

#ifdef HS_FS_ONLY
#define ID_PRODUCT                      0x1003  // Product
#else
#define ID_PRODUCT                      0x0002  // Product
#endif

#define BCD_DEVICE_SS                   0x0010  // 0.1

#ifdef HS_FS_ONLY
#define BCD_USB_HS                      0x0200  // 2.00
#else
#define BCD_USB_HS                      0x0210  // 2.10
#endif

#define BCD_DEVICE_HS                   0x0200  // 2.00

#define USB_MANUFACTURER_STRING         "CADENCE"
#define USB_PRODUCT_STRING              "Audio microphone"
#define USB_SERIAL_NUMBER_STRING        "100000000000" // should 12 chars long

static CUSBD_OBJ * drv; // driver pointer
static void * pD; // private data pointer

static volatile uint8_t configValue = 0; // keeps actual configuration value
uint8_t *ep0Buff;
static uint8_t ep0BuffAlloc[200 + 7];
static struct CUSBD_Req *ep0Req;

// static allocation of endpoints objects
static struct CUSBD_Ep * epIn;
static uint8_t *epInBuff;
static uint8_t  epInBuffAlloc[600 + 7];
static struct CUSBD_Req *epInReq;

// audio controls
static uint8_t altSetting = 0;

// debug functions
static void displayRequestinfo(struct CUSBD_Req * req);
static void displayEndpointInfo(struct CUSBD_Ep * ep);
static void displayDeviceInfo(CUSBD_Dev * dev);


static void get_unicode_string(char * target, const char * src) {

    size_t src_len = strlen(src)*2;
    int i;

    *target++ = src_len + 2;
    *target++ = CH9_USB_DT_STRING;

    if (src_len > 100) src_len = 100;
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

static CH9_UsbConfigurationDescriptor confDesc = {
    .bLength = CH9_USB_DS_CONFIGURATION,
    .bDescriptorType = CH9_USB_DT_CONFIGURATION,
    .wTotalLength = 0x0000, // will be filed in get_descriptor request
    .bNumInterfaces = 2,
    .bConfigurationValue = 1,
    .iConfiguration = 0,
    .bmAttributes = CH9_USB_CONFIG_RESERVED | CH9_USB_CONFIG_SELF_POWERED,
    .bMaxPower = 1
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

    if (!drv)
        return;

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
    int length;

    CH9_UsbDeviceDescriptor * devDesc;

    ctrl->wIndex = le16ToCpu(ctrl->wIndex);
    ctrl->wLength = le16ToCpu(ctrl->wLength);
    ctrl->wValue = le16ToCpu(ctrl->wValue);

    drv->getDevInstance(pD, &dev);

    // select descriptors according to actual speed
    switch (dev->speed) {

        case CH9_USB_SPEED_FULL:
            devDesc = &devHsDesc;
            break;

        case CH9_USB_SPEED_HIGH:
            devDesc = &devHsDesc;
            break;

        case CH9_USB_SPEED_SUPER:
            devDesc = &devSsDesc;
            break;

        default:
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
                                uint16_t audioDescLen = (uint16_t)audioGetDesc(dev->speed, &ep0Buff[CH9_USB_DS_CONFIGURATION]);

                                length = le16ToCpu(audioDescLen + CH9_USB_DS_CONFIGURATION);
                                confDesc.wTotalLength = cpuToLe16(length);
                                memmove(ep0Buff, &confDesc, CH9_USB_DS_CONFIGURATION);

                                vDbgMsg(DBG_USB_CTRL_MSG, 1, "ConfDesc[0] = %02X\n", confDesc.bLength);
                                {
                                    int i;

                                    for (i = 0; i < length; i++) {
                                        vDbgMsg(DBG_USB_CTRL_MSG, 1, "%02X ", ep0Buff[i]);
                                    }
                                    vDbgMsg(DBG_USB_CTRL_MSG, 1, " %c\n", ' ');
                                }
                            }
                                break;

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
                            }
                                break;

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
                                uint16_t audioDescLen;

                                if(dev->speed == CH9_USB_SPEED_FULL)
                                    audioDescLen = (uint16_t)audioGetDesc(CH9_USB_SPEED_HIGH, &ep0Buff[CH9_USB_DS_CONFIGURATION]);
                                else
                                    audioDescLen = (uint16_t)audioGetDesc(CH9_USB_SPEED_FULL, &ep0Buff[CH9_USB_DS_CONFIGURATION]);

                                length = le16ToCpu(audioDescLen + CH9_USB_DS_CONFIGURATION);
                                confDesc.wTotalLength = cpuToLe16(length);
                                memmove(ep0Buff, &confDesc, CH9_USB_DS_CONFIGURATION);
                                ep0Buff[1] = CH9_USB_DT_OTHER_SPEED_CONFIGURATION;

                                vDbgMsg(DBG_USB_CTRL_MSG, 1, "OtherSpeedDesc[0] = %02X\n", confDesc.bLength);
                                {
                                    int i;

                                    for (i = 0; i < length; i++) {
                                        vDbgMsg(DBG_USB_CTRL_MSG, 1, "%02X ", ep0Buff[i]);
                                    }
                                    vDbgMsg(DBG_USB_CTRL_MSG, 1, " %c\n", ' ');
                                }
                            }
                                break;

                            default:
                                vDbgMsg(DBG_USB_CTRL_MSG, 1, "Error_1 %c\n", ' ');
                                return -1;

                        }//switch
                    }//if
                    break;

                case CH9_USB_REQ_SET_CONFIGURATION:
                {
                    struct CUSBD_Ep * ep;
                    CUSBD_ListHead *list;

                    vDbgMsg(DBG_USB_CTRL_MSG, 1, "SET CONFIGURATION(%d)\n", le16ToCpu(ctrl->wValue));
                    if (ctrl->wValue > 1) return -1; // no such configuration

                    // unconfigure device
                    if (ctrl->wValue == 0) {
                        configValue = 0;
                        for (list = dev->epList.next; list != &dev->epList; list = list->next) {
                            ep = (struct CUSBD_Ep *) list;
                            if (ep->address == ENDPOINT_IN) {
                                ep->ops->epDisable(pD, ep);
                                //displayEndpointInfo(ep);
                                break;
                            }
                        }
                        return 0;
                    }

                    // device already configured
                    if (configValue == 1 && ctrl->wValue == 1) return 0;

                    // configure device
                    configValue = (uint8_t) ctrl->wValue;
                    for (list = dev->epList.next; list != &dev->epList; list = list->next) {
                        ep = (struct CUSBD_Ep *) list;
                        if (ep->address == ENDPOINT_IN) {
                            if(dev->speed == CH9_USB_SPEED_FULL){
                                epInReq->length = 32;
                            }
                            if(dev->speed > CH9_USB_SPEED_FULL){
                                epInReq->length = 4;
                            }
                            ep->ops->epEnable(pD, ep, getEpDesc(dev->speed));
                            displayEndpointInfo(ep);
                            break;
                        }
                    }

                    /*Code control  Self powered feature of USB*/
                    if (confDesc.bmAttributes & CH9_USB_CONFIG_SELF_POWERED) {
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
                    if (ctrl->wIndex != 1)return -1;
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
                    return -1; //return error
            }
            break;

        case CH9_USB_REQ_TYPE_CLASS:
            length = audioAcRequest(ctrl);
            if(length < 0) return -1; // return error
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
    vDbgMsg(DBG_USB_CTRL_MSG, 1, "epList: %08X\n", (uintptr_t) & ep->epList);
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
    if (usbRequest)
        free(usbRequest);
}

static CUSBD_Callbacks callback = {
    disconnect, connect, setup, suspend, resume, requestMemAlloc, requestMemFree
};

static void epInCmpl(struct CUSBD_Ep *ep, struct CUSBD_Req * req)  {

    displayRequestinfo(req);
    if(req->status == 0)
        ep->ops->reqQueue(pD, ep, req);

}
extern void disable_int();
extern void enable_int();

// interrupt handler
void micIsr() {
    drv->isr(pD);
}
int micApp() {

    uint32_t res; // keeps result of operation on driver
    CUSBD_SysReq sysReq;
    CUSBD_Dev * dev; // auxiliary pointer used for testing purposes
    CUSBD_ListHead *list; // used in for_each loop

    drv = CUSBD_GetInstance(); // get driver handle

    res = drv->probe(&config, &sysReq);
    if (res != 0) goto error;
    vDbgMsg(DBG_USB_CTRL_MSG, 1, "Device memory requirement: %ld bytes\n", sysReq.privDataSize);

    // allocate memory for device object
    pD = (uint8_t*) malloc((size_t) (sysReq.privDataSize));
    if(!pD) goto error;
    config.trbAddr = malloc(sysReq.trbMemSize);
    if(!config.trbAddr)
        goto error;
    config.trbDmaAddr = (uintptr_t)config.trbAddr;
    res = drv->init(pD, &config, &callback);
    if (res != 0) goto error;
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
    while (!configValue);
    disable_int();
    // for each of endpoint print info
    for (list = dev->epList.next; list != &dev->epList; list = list->next) {
        epIn = (struct CUSBD_Ep *) list;
        //displayEndpointInfo(epIn);
        if (epIn->address == ENDPOINT_IN) {
            drv->reqAlloc(pD, epIn, &epInReq);
            break;
        }
    }

    epInBuff[0] = epInBuff[1] = 0x00;
    epInBuff[2] = epInBuff[3] = 0xFF;

    epInReq->buf = epInBuff;
    epInReq->dma = (uintptr_t) epInBuff;
    epInReq->complete = epInCmpl;
    epInReq->length = 4;
    enable_int();
    while (!altSetting);
    epIn->ops->reqQueue(pD, epIn, epInReq);
    // wait until device in default state
    while (configValue);
    goto back;

    drv->stop(pD);
    drv->destroy(pD);

    // free allocated memory
    if (pD)
        free(pD);

    return 0;

error:
    vDbgMsg(DBG_USB_CTRL_MSG, 1, "Error %08lX\n", res);
    // free allocated memory
    if (pD)free(pD);
    if (config.trbAddr)
        free (config.trbAddr);
    return (res);
}

