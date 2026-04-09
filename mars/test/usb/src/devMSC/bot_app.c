/**********************************************************************
 * Copyright (C) 2014-2017 Cadence Design Systems, Inc.
 * All rights reserved worldwide.
 ***********************************************************************
 * bot_app.c
 * Bulk Only Transport is a part of mass storage application responsible
 * for SCSI command, data and status transport
 ***********************************************************************/
#include <stdlib.h>                        // used for malloc
#include "cdn_stdio.h"                     // standard library
#include "cdn_string.h"                    // standard library
#include "log.h"

#include "cusbd_if.h"                      // USB controller interface
#include "byteorder.h"                     // endian macros
#include "bot.h"                           // BOT protocol
#include "storage.h"                       // storage layer
#include "scsi.h"                          // SCSI layer
#include "config.h"                        // application's configuration
#include "cdn_errno.h"                     // errors definitions

#include "map_system_memory.h"

#define STORAGE_BASE 0x1000000               // physical address of storage device

/*USB application debugging*/
#define DBG_USB_APP          0x000000010
#define DBG_USB_APP_VERBOSE  0x000000020
#define DBG_USB_BOT_APP      0x000000040
#define DBG_USB_SCSI_APP     0x000000008
#define DBG_USB_CTRL_MSG     0x900000002

#define SHOW_BUFF_SIZE 500
static char showBuffTemp[SHOW_BUFF_SIZE];

void showBuff(uint8_t * buff, uint32_t length) {
    uint32_t i=0, ret=0;
    uint32_t buff_len = SHOW_BUFF_SIZE;
    for(i=0; i < length; i++) {
        ret = snprintf (&showBuffTemp[SHOW_BUFF_SIZE - buff_len] , buff_len,  "0x0%x, ", buff[i]);
        buff_len -= ret;
    }
    showBuffTemp[SHOW_BUFF_SIZE - buff_len] = '\0';
    vDbgMsg(DBG_USB_APP, DBG_HIVERB, "BUFF: %s\n", &showBuffTemp[1]);
}

// external dependencies
extern void disable_int(); // used in critical sections
extern void enable_int(); // used in critical sections

// functions used for transferring SCSI data
static uint32_t send_data(void *buff, uint32_t size);
static uint32_t rec_data(void *buff, uint32_t size);

// debug functions
static void displayRequestinfo(struct CUSBD_Req * req);
static void displayEndpointInfo(struct CUSBD_Ep * ep);
static void displayDeviceInfo(CUSBD_Dev * dev);


//-------------- definitions used during configuration -------------------------
#define BCD_USB_HS_ONLY         0x0201  // 2.01  /*Only HS with BOS descriptor*/
#define BULK_EP_IN 0x81
#define BULK_EP_OUT 0x01
//------------------------------------------------------------------------------

// Super Speed USB object configuration

extern uint8_t isDataToTransferOnChip(void *pD, struct CUSBD_Ep * ep) ;

// ---------- Super Speed USB driver configuration -----------------------------
static CUSBD_Config config = {
    .regBase = USB_REGS_BASE, // address where USB core is mapped
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
static void * pD; // driver's private data pointer
static uint8_t appInitializedFlag =0;

// interrupt handler
void mscIsr() {
    drv->isr(pD);
}

// ------------- driver's resources --------------------------------------------
static struct CUSBD_Ep * epIn, *epOut; // endpoint objects

// static allocation of request for default, IN and OUT endpoints
static struct CUSBD_Req *ep0Req, *bulkInReq, *bulkOutReq;

//static memory allocation for default endpoint data buffer
static uint8_t *ep0Buff;
static uint8_t ep0BuffAlloc[100 + 7];

// --------flags used for synchronization---------------------------------------
static volatile uint8_t configValue = 0; // keeps actual configuration value
static uint8_t current_speed = CH9_USB_SPEED_UNKNOWN; // keeps actual speed value
static volatile uint8_t packet_received = 0; // flag active when received packet
static volatile uint8_t packet_sent = 0; // flag active when sent packet
static volatile uint8_t packet_aborted = 0; // flag active when packet aborted

// BOT protocol buffers
static uint8_t* command_buff; // buffer for command packets
static uint8_t* response_buff; // buffer for csw packets

static uint8_t command_buffAlloc[31 + 7];
static uint8_t response_buffAlloc[13 + 7];

// mass storage auxiliary variables - used during MSC error recovery
static uint32_t host_num_of_bytes;
static uint8_t data_dir;
static uint8_t dir_error;
volatile uint8_t msc_failed_flag = 0;




// called on critical MSC error

static void msc_failed() {
    disable_int();
    epIn->ops->epSetWedge(pD, epIn);
    epOut->ops->epSetWedge(pD, epOut);
    msc_failed_flag = 1;
    enable_int();
}

// EP0 completion routine

static void reqComplete(struct CUSBD_Ep *ep, struct CUSBD_Req * req)  {
    vDbgMsg(DBG_USB_APP, DBG_HIVERB, "Request on endpoint %s completed\n", ep->name);
    //displayRequestinfo(req);
}

// bulk IN completion routine

static void bulkInCmpl(struct CUSBD_Ep *ep, struct CUSBD_Req * req)  {
    vDbgMsg(DBG_USB_APP, DBG_HIVERB, "Transfer complete on ep:%02X %08X req\n", ep->address, (uintptr_t) req);
    displayRequestinfo(req);
    if (req->status != 0) {
        return;
    }
    packet_sent = 1;
}

// bulk OUT completion routine

static void bulkOutCmpl(struct CUSBD_Ep *ep, struct CUSBD_Req * req)  {
    vDbgMsg(DBG_USB_APP, DBG_HIVERB, "Transfer complete on ep:%02X %08X req\n", ep->address, (uintptr_t) req);
    displayRequestinfo(req);
    if (req->status != 0) {
        packet_aborted = 1;
        return;
    }
    packet_received = 1;
}

// function returns unicode string for english version

static void get_unicode_string(char * target, const char * src) {

    size_t src_len = strlen(src) * 2;
    int i;

    *target++ = src_len + 2;
    *target++ = CH9_USB_DT_STRING;

    if (src_len > 100)
        src_len = 100;
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
        uint8_t * ret_address = &(((uint8_t*) notAllignedBuff)[alignValue
                - offset]);
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
//------------- Start of Super Speed configuration descriptors -----------------
// configuration descriptors for SuperSpeed mode
static CH9_UsbConfigurationDescriptor confSsDesc = {CH9_USB_DS_CONFIGURATION,
    CH9_USB_DT_CONFIGURATION, cpuToLe16(
    CH9_USB_DS_CONFIGURATION
    + CH9_USB_DS_INTERFACE
    + 2 * CH9_USB_DS_ENDPOINT
    + 2 * CH9_USB_DS_SS_USB_ENDPOINT_COMPANION), 1, 1, 0,
    CH9_USB_CONFIG_RESERVED | CH9_USB_CONFIG_SELF_POWERED, 1};

//0x09, 0x04, 0x00, 0x00, 0x01, 0x03, 0x00, 0x00, 0x00,
static CH9_UsbInterfaceDescriptor interfaceDesc = {CH9_USB_DS_INTERFACE, CH9_USB_DT_INTERFACE, 0, 0, 2,
    CH9_USB_CLASS_MASS_STORAGE, 0x06, 0x50, 0};

static CH9_UsbEndpointDescriptor endpointEpInDesc = {CH9_USB_DS_ENDPOINT,
    CH9_USB_DT_ENDPOINT, BULK_EP_IN, CH9_USB_EP_BULK, cpuToLe16(512), 0};

static CH9_UsbEndpointDescriptor endpointEpOutDesc = {CH9_USB_DS_ENDPOINT,
    CH9_USB_DT_ENDPOINT, BULK_EP_OUT, CH9_USB_EP_BULK, cpuToLe16(512), 0};

//0x06, 0x30, 0x00, 0x00, 0x00, 0x00
static CH9_UsbSSEndpointCompanionDescriptor compDesc = {
    CH9_USB_DS_SS_USB_ENDPOINT_COMPANION,
    CH9_USB_DT_SS_USB_ENDPOINT_COMPANION, 0, 0, cpuToLe16(0)
};
//------------------- End of Super Speed configuration -------------------------

//------------- Start of High Speed configuration descriptors -----------------

// configuration descriptors for HighSpeed mode
static CH9_UsbConfigurationDescriptor confHsDesc = {CH9_USB_DS_CONFIGURATION,
    CH9_USB_DT_CONFIGURATION, cpuToLe16(
    CH9_USB_DS_CONFIGURATION
    + CH9_USB_DS_INTERFACE
    + 2 * CH9_USB_DS_ENDPOINT), 1, 1, 0, CH9_USB_CONFIG_RESERVED
    | CH9_USB_CONFIG_SELF_POWERED, 0};

// language descriptor for english
static uint8_t languageDesc[] = {4, CH9_USB_DT_STRING, 0x09, 0x04};

// string will be filled then in initializing section
static char vendorDesc[sizeof (USB_MANUFACTURER_STRING) * 2 + 2];
static char productDesc[sizeof (USB_PRODUCT_STRING) * 2 + 2];
static char serialDesc[sizeof (USB_SERIAL_NUMBER_STRING) * 2 + 2];

//-------------- BOS descriptor set start --------------------------------------

static CH9_UsbBosDescriptor bosDesc = {
    CH9_USB_DS_BOS,
    CH9_USB_DT_BOS,
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
    CH9_USB_DS_DEVICE_CAPABILITY_20,
    CH9_USB_DT_DEVICE_CAPABILITY,
    CH9_USB_DCT_USB20_EXTENSION,
    cpuToLe32(CH9_USB_USB20_EXT_LPM_SUPPORT | 0x04)

};

//-------------- BOS descriptor set end ----------------------------------------

static CH9_UsbDeviceQualifierDescriptor qualifierDesc = {
    CH9_USB_DS_DEVICE_QUALIFIER, CH9_USB_DT_DEVICE_QUALIFIER,
    cpuToLe16(0x0200), 0x00, 0x00, 0x00, 64, 0x01, 0x00
};

// -------------- driver callback functions ------------------------------------

static void connect(void *pD)  {
    CUSBD_Dev *dev;


    drv->getDevInstance(pD, &dev);

    vDbgMsg(DBG_USB_APP, DBG_HIVERB, "Application: connect at %d speed\n", dev->speed);
}

static void disconnect(void *pD)  {
    CUSBD_Dev *dev;
    if(!drv)
        return;
    drv->getDevInstance(pD, &dev);

    vDbgMsg(DBG_USB_APP, DBG_HIVERB,  "Application: disconnect %c\n", ' ');

    displayDeviceInfo(dev);
    configValue = 0;
    packet_received = 0;
    packet_sent = 0;
    msc_failed_flag =0;
    appInitializedFlag = 0;
}

static void resume(void *pD)  {
    vDbgMsg(DBG_USB_APP, DBG_HIVERB,  "Application: resume %c\n", ' ');
}

static uint32_t setup(void *pD, CH9_UsbSetup *ctrl)  {

    // get device reference
    CUSBD_Dev * dev;
    int length;

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
            break;
    }

    vDbgMsg(DBG_USB_APP, DBG_HIVERB, "Speed %d: \n", dev->speed);
    vDbgMsg(DBG_USB_APP, DBG_HIVERB, "bRequest: %02X\n", ctrl->bRequest);
    vDbgMsg(DBG_USB_APP, DBG_HIVERB, "bRequestType: %02X\n", ctrl->bmRequestType);
    vDbgMsg(DBG_USB_APP, DBG_HIVERB, "wIndex: %04X\n", ctrl->wIndex);
    vDbgMsg(DBG_USB_APP, DBG_HIVERB, "wValue: %04X\n", ctrl->wValue);
    vDbgMsg(DBG_USB_APP, DBG_HIVERB, "wLength: %04X\n", ctrl->wLength);

    ep0Req->buf = ep0Buff;
    ep0Req->dma = (uintptr_t) ep0Buff;
    ep0Req->complete = reqComplete;

    switch (ctrl->bmRequestType & CH9_USB_REQ_TYPE_MASK) {

        case CH9_USB_REQ_TYPE_STANDARD:

            switch (ctrl->bRequest) {

                case CH9_USB_REQ_GET_DESCRIPTOR:
                    vDbgMsg(DBG_USB_APP, DBG_HIVERB, "GET DESCRIPTOR %c\n", ' ');
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
                                vDbgMsg(DBG_USB_APP, DBG_HIVERB, "DevDesc[0] = %d\n", devDesc->bLength);
                                showBuff(ep0Buff, length);
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
                                vDbgMsg(DBG_USB_APP, DBG_HIVERB, "ConfDesc[0] = %02X\n", configDesc->bLength);
                                showBuff(ep0Buff, length);
                            }
                                break;

                            case CH9_USB_DT_STRING:
                            {
                                uint8_t descIndex = (uint8_t) (ctrl->wValue & 0xFF);
                                char *strDesc;
                                vDbgMsg(DBG_USB_APP, DBG_HIVERB, "StringDesc %c\n", ' ');
                                switch (descIndex) {
                                    case 0:
                                        strDesc = (char*) &languageDesc;
                                        length = strDesc[0];
                                        vDbgMsg(DBG_USB_APP, DBG_HIVERB, "language %c\n", ' ');
                                        break;

                                    case 1:
                                        strDesc = (char*) &vendorDesc;
                                        length = strDesc[0];
                                        vDbgMsg(DBG_USB_APP, DBG_HIVERB, "vendor %c\n", ' ');
                                        break;

                                    case 2:
                                        strDesc = (char*) &productDesc;
                                        length = strDesc[0];
                                        vDbgMsg(DBG_USB_APP, DBG_HIVERB, "product %c\n", ' ');
                                        break;

                                    case 3:
                                        strDesc = (char*) &serialDesc;
                                        length = strDesc[0];
                                        vDbgMsg(DBG_USB_APP, DBG_HIVERB, "serial %c\n", ' ');
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

                                vDbgMsg(DBG_USB_APP, DBG_HIVERB, "bosDesc[0] = %02X\n", bosDesc.bLength);
                            {
                                int i;

                                for (i = 0; i < length; i++) {
                                    vDbgMsg(DBG_USB_APP, DBG_HIVERB, "%02X ", ep0Buff[i]);
                                }
                                vDbgMsg(DBG_USB_APP, DBG_HIVERB, " %c\n", ' ');
                            }
                                vDbgMsg(DBG_USB_APP, DBG_HIVERB, "BosDesc %c\n", ' ');
                                break;

                            case CH9_USB_DT_DEVICE_QUALIFIER:
                                length = CH9_USB_DS_DEVICE_QUALIFIER;
                                memmove(ep0Buff, &qualifierDesc, length);
                                vDbgMsg(DBG_USB_APP, DBG_HIVERB, "QualifierDesc %c\n", ' ');
                                break;

                            case CH9_USB_DT_OTHER_SPEED_CONFIGURATION:
                            {
                                int offset = 0;

                                length = le16ToCpu(configDesc->wTotalLength);

                                if (dev->speed == CH9_USB_SPEED_SUPER) return -1;

                                switch (dev->speed) {

                                    case CH9_USB_SPEED_FULL:
                                        endpointEpInDesc.wMaxPacketSize = cpuToLe16(512);
                                        endpointEpOutDesc.wMaxPacketSize = cpuToLe16(512);
                                        break;

                                    case CH9_USB_SPEED_HIGH:
                                        endpointEpInDesc.wMaxPacketSize = cpuToLe16(64);
                                        endpointEpOutDesc.wMaxPacketSize = cpuToLe16(64);
                                        break;

                                    default:
                                        return -1;
                                }

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

                                memmove(&ep0Buff[offset], &endpointEpOutDesc, CH9_USB_DS_ENDPOINT);
                                vDbgMsg(DBG_USB_APP, DBG_HIVERB, "OtherSpeedDesc[0] = %02X\n", configDesc->bLength);
                                showBuff(ep0Buff, length);
                            }
                                break;

                            default:
                                vDbgMsg(DBG_USB_APP, DBG_CRIT, "Error_1 %c\n", ' ');
                                return -1;

                        }//switch
                    }//if
                    break;

                case CH9_USB_REQ_SET_CONFIGURATION:
                {
                    struct CUSBD_Ep * ep;
                    CUSBD_ListHead *list;

                    vDbgMsg(DBG_USB_APP, DBG_HIVERB, "SET CONFIGURATION(%d)\n", le16ToCpu(ctrl->wValue));
                    if (ctrl->wValue > 1) return -1; // no such configuration

                    // unconfigure device
                    if (ctrl->wValue == 0) {
                        configValue = 0;
                        for (list = dev->epList.next; list != &dev->epList; list = list->next) {
                            ep = (struct CUSBD_Ep *) list;
                            if (ep->address == BULK_EP_IN) {
                                ep->ops->epDisable(pD, ep);
                                //displayEndpointInfo(ep);
                                break;
                            }
                        }
                        for (list = dev->epList.next; list != &dev->epList; list = list->next) {
                            ep = (struct CUSBD_Ep *) list;
                            if (ep->address == BULK_EP_OUT) {
                                ep->ops->epDisable(pD, ep);
                                //displayEndpointInfo(ep);
                                break;
                            }
                        }
                        dev->state = CH9_USB_STATE_ADDRESS;
                        return 0;
                    }

                    // device already configured
                    if (configValue == 1 && ctrl->wValue == 1) {
                        return 0;
                    }

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
                    dev->state = CH9_USB_STATE_CONFIGURED;
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
                    if ((ctrl->wValue != 0) || (ctrl->wIndex != 0) || (ctrl->wLength != 0))return -1;
                    length = 0;
                    break;

                case CH9_USB_REQ_GET_INTERFACE:
                    if ((ctrl->wValue != 0) || (ctrl->wIndex != 0) || (ctrl->wLength != 1))return -1;
                    length = 1;
                    ep0Buff[0] = 0;
                    break;

                case CH9_USB_REQ_GET_STATUS:
                    if ((ctrl->wValue != 0) || (ctrl->wIndex != 0) || (ctrl->wLength != 2))return -1;
                    length = 2;
                    ep0Buff[0] = 0;
                    ep0Buff[1] = 0;
                    break;

                default:
                    vDbgMsg(DBG_USB_APP, DBG_CRIT, "Error_2 %c\n", ' ');
                    return -1; //return error
            }
            break;

        case CH9_USB_REQ_TYPE_CLASS:

            if (ctrl->bmRequestType & CH9_USB_EP_DIR_MASK) {
                if ((ctrl->bRequest == 0xFE) && (ctrl->wValue == 0) && (ctrl->wLength
                        == 1) && (ctrl->wIndex == 0)) {
                    devGetMaxLun(ep0Buff);
                    length = 1;
                } else {
                    return -1;
                }
            } else {
                if (ctrl->bRequest != 0xFF || ctrl->wValue != 0 || ctrl->wLength != 0
                        || ctrl->wIndex != 0) {
                    return -1;
                } else {
                    msc_failed_flag = 0;
                    epIn->ops->epSetHalt(pD, epIn, 0);
                    epOut->ops->epSetHalt(pD, epOut, 0);
                    vDbgMsg(DBG_USB_APP, DBG_HIVERB, "MSC RESET%s\n", "");
                    length = 0;
                }
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
    vDbgMsg(DBG_USB_APP, DBG_HIVERB, "Application: suspend %c\n", ' ');
}


// --------------------- debug functions ---------------------------------------

static void displayRequestinfo(struct CUSBD_Req * req) {
    vDbgMsg(DBG_USB_APP_VERBOSE, DBG_HIVERB, "-------Request INFO-------------- %c\n", ' ');
    vDbgMsg(DBG_USB_APP_VERBOSE, DBG_HIVERB, "buf: %08X\n", (uintptr_t) req->buf);
    vDbgMsg(DBG_USB_APP_VERBOSE, DBG_HIVERB, "length: %ld\n", req->length);
    vDbgMsg(DBG_USB_APP_VERBOSE, DBG_HIVERB, "dma: %08X\n", req->dma);
    vDbgMsg(DBG_USB_APP_VERBOSE, DBG_HIVERB, "sg:  %c\n", ' ');
    vDbgMsg(DBG_USB_APP_VERBOSE, DBG_HIVERB, "numOfSgs %ld\n", req->numOfSgs);
    vDbgMsg(DBG_USB_APP_VERBOSE, DBG_HIVERB, "numMappedSgs: %ld\n", req->numMappedSgs);
    vDbgMsg(DBG_USB_APP_VERBOSE, DBG_HIVERB, "streamId: %04X\n", req->streamId);
    vDbgMsg(DBG_USB_APP_VERBOSE, DBG_HIVERB, "noInterrupt: %d\n", req->noInterrupt);
    vDbgMsg(DBG_USB_APP_VERBOSE, DBG_HIVERB, "zero: %d\n", req->zero);
    vDbgMsg(DBG_USB_APP_VERBOSE, DBG_HIVERB, "shortNotOk: %d\n", req->shortNotOk);
    vDbgMsg(DBG_USB_APP_VERBOSE, DBG_HIVERB, "context: %08X\n", (uintptr_t) req->context);
    vDbgMsg(DBG_USB_APP_VERBOSE, DBG_HIVERB, "status: %08lX\n", req->status);
    vDbgMsg(DBG_USB_APP_VERBOSE, DBG_HIVERB, "actual: %ld\n", req->actual);
}

static void displayEndpointInfo(struct CUSBD_Ep * ep) {
    vDbgMsg(DBG_USB_APP_VERBOSE, DBG_HIVERB, "-------Endpoint INFO------------- %c\n", ' ');
    vDbgMsg(DBG_USB_APP_VERBOSE, DBG_HIVERB, "address: %02X\n", ep->address);
    vDbgMsg(DBG_USB_APP_VERBOSE, DBG_HIVERB, "epList: %08X\n", (uintptr_t) & ep->epList);
    vDbgMsg(DBG_USB_APP_VERBOSE, DBG_HIVERB, "driverData: %08X\n", (uintptr_t) ep->driverData);
    vDbgMsg(DBG_USB_APP_VERBOSE, DBG_HIVERB, "name: %s\n", ep->name);
    vDbgMsg(DBG_USB_APP_VERBOSE, DBG_HIVERB, "ops: %c\n", ' ');
    vDbgMsg(DBG_USB_APP_VERBOSE, DBG_HIVERB, "maxPacket: %d\n", ep->maxPacket);
    vDbgMsg(DBG_USB_APP_VERBOSE, DBG_HIVERB, "maxStreams: %d\n", ep->maxStreams);
    vDbgMsg(DBG_USB_APP_VERBOSE, DBG_HIVERB, "mult: %d\n", ep->mult);
    vDbgMsg(DBG_USB_APP_VERBOSE, DBG_HIVERB, "maxburst: %d\n", ep->maxburst);
    vDbgMsg(DBG_USB_APP_VERBOSE, DBG_HIVERB, "desc: %08X\n", (uintptr_t) ep->desc);
    vDbgMsg(DBG_USB_APP_VERBOSE, DBG_HIVERB, "compDesc: %08X\n", (uintptr_t) ep->compDesc);
}

static void displayDeviceInfo(CUSBD_Dev * dev) {
    vDbgMsg(DBG_USB_APP_VERBOSE, DBG_HIVERB, "-------Device INFO--------------- %c\n", ' ');
    vDbgMsg(DBG_USB_APP_VERBOSE, DBG_HIVERB, "epList: prev = %08X, next = %08X\n", (uintptr_t) dev->epList.prev, (uintptr_t) dev->epList.next);
    vDbgMsg(DBG_USB_APP_VERBOSE, DBG_HIVERB, "ep0: %08X\n", (uintptr_t) dev->ep0);
    vDbgMsg(DBG_USB_APP_VERBOSE, DBG_HIVERB, "speed: %d\n", dev->speed);
    vDbgMsg(DBG_USB_APP_VERBOSE, DBG_HIVERB, "maxSpeed: %d\n", dev->maxSpeed);
    vDbgMsg(DBG_USB_APP_VERBOSE, DBG_HIVERB, "state: %d\n", dev->state);
    vDbgMsg(DBG_USB_APP_VERBOSE, DBG_HIVERB, "sgSupported: %d\n", dev->sgSupported);
    vDbgMsg(DBG_USB_APP_VERBOSE, DBG_HIVERB, "isOtg: %d\n", dev->isOtg);
    vDbgMsg(DBG_USB_APP_VERBOSE, DBG_HIVERB, "isAPeripheral: %d\n", dev->isAPeripheral);
    vDbgMsg(DBG_USB_APP_VERBOSE, DBG_HIVERB, "bHnpEnable: %d\n", dev->bHnpEnable);
    vDbgMsg(DBG_USB_APP_VERBOSE, DBG_HIVERB, "aHnpSupport: %d\n", dev->aHnpSupport);
    vDbgMsg(DBG_USB_APP_VERBOSE, DBG_HIVERB, "name: %s\n", dev->name);
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

// reads data from storage and sends it to EP-IN endpoint

static uint32_t storage_to_usb(uint32_t start_sec, uint32_t num_of_sec)  {

    uint32_t size = num_of_sec * SECTOR_SIZE;

    if (!data_dir && dev_num_of_bytes > 0) {
        dir_error = 1;
        return 0;
    }

    if (dev_num_of_bytes == 0 || host_num_of_bytes == 0) {
        return 0;
    }

    bulkInReq->dma = STORAGE_BASE + start_sec * SECTOR_SIZE;
    bulkInReq->complete = bulkInCmpl;
    bulkInReq->length = host_num_of_bytes < size ? host_num_of_bytes : size;
    bulkInReq->streamId = 0;

    disable_int();
    epIn->ops->reqQueue(pD, epIn, bulkInReq);
    enable_int();

    while (!packet_sent) {
        if (!configValue) {
            return 0;
        }
    }
    packet_sent = 0;
    return bulkInReq->actual;

}

// sends data from EP-OUT to storage memory

static uint32_t usb_to_storage(uint32_t start_sec, uint32_t num_of_sec)  {

    uint32_t size = num_of_sec * SECTOR_SIZE;

    if (data_dir && dev_num_of_bytes > 0) {
        dir_error = 1;
        return 0;
    }

    if (dev_num_of_bytes == 0 || host_num_of_bytes == 0) {
        return 0;
    }

    bulkOutReq->dma = STORAGE_BASE + start_sec * SECTOR_SIZE;
    bulkOutReq->complete = bulkOutCmpl;
    bulkOutReq->length = host_num_of_bytes < size ? host_num_of_bytes : size;
    bulkOutReq->streamId = 0;

    disable_int();
    epOut->ops->reqQueue(pD, epOut, bulkOutReq);
    enable_int();

    while (!packet_received) {
        if (!configValue) {
            return 0;
        }
    }
    packet_received = 0; // reset packet_received flag
    return bulkOutReq->actual;

}

// receive CBW

static uint8_t rec_cbw(void *buff, uint32_t size)  {

    uint32_t status;

    bulkOutReq->buf = buff;
    bulkOutReq->dma = (uintptr_t) buff;
    bulkOutReq->complete = bulkOutCmpl;
    bulkOutReq->length = size;
    bulkOutReq->streamId = 0;

    vDbgMsg(DBG_USB_APP, DBG_HIVERB, "rec_cbw%s\n","");
    disable_int();
    status = epOut->ops->reqQueue(pD, epOut, bulkOutReq);
    if (status != 0) {
        enable_int();
        return 0;
    }

    enable_int();

    while (!packet_received) {
        if (packet_aborted) {
            packet_aborted = 0;
            return 0;
        }
    }
    packet_received = 0; // reset packet_received flag
    return (uint8_t) bulkOutReq->actual;
}

// sends CBW response on IN-EP

static uint32_t send_data(void *buff, uint32_t size)  {

    vDbgMsg(DBG_USB_BOT_APP, DBG_HIVERB, "BOT DATA IN<- size: %ld\n", size );

    if (!data_dir && dev_num_of_bytes > 0) {
        dir_error = 1;
        return 0;
    }

    if (dev_num_of_bytes == 0 || host_num_of_bytes == 0) {
        return 0;
    }

    bulkInReq->buf = buff;
    bulkInReq->dma = (uintptr_t) buff;
    bulkInReq->complete = bulkInCmpl;
    bulkInReq->length = host_num_of_bytes < size ? host_num_of_bytes : size;
    bulkInReq->streamId = 0;

    disable_int();
    epIn->ops->reqQueue(pD, epIn, bulkInReq);
    enable_int();

    while (!packet_sent) {
        if (!configValue) {
            return 0;
        }
    }
    packet_sent = 0;
    return bulkInReq->actual;
}

// send ZERO length data packet

static void send_data_0()  {


    vDbgMsg(DBG_USB_BOT_APP, DBG_HIVERB, "BOT DATA ZERO<-%s\n", "");
    bulkInReq->complete = bulkInCmpl;
    bulkInReq->length = 0;
    bulkInReq->streamId = 0;

    disable_int();
    epIn->ops->reqQueue(pD, epIn, bulkInReq);
    enable_int();

    while (!packet_sent) {
        if (!configValue) {
            return;
        }
    }
    packet_sent = 0;
}

// receive data on SCSI command
static uint32_t rec_data(void *buff, uint32_t size)  {

    vDbgMsg(DBG_USB_BOT_APP, DBG_HIVERB, "BOT DATA OUT-> size: %ld\n", size);
    if (data_dir && dev_num_of_bytes > 0) {
        dir_error = 1;
        return 0;
    }

    if (dev_num_of_bytes == 0 || host_num_of_bytes == 0) {
        return 0;
    }

    bulkOutReq->buf = buff;
    bulkOutReq->dma = (uintptr_t) buff;
    bulkOutReq->complete = bulkOutCmpl;
    bulkOutReq->length = host_num_of_bytes < size ? host_num_of_bytes : size;
    bulkOutReq->streamId = 0;

    disable_int();
    epOut->ops->reqQueue(pD, epOut, bulkOutReq);
    enable_int();

    while (!packet_received) {
        if (!configValue) {
            return 0;
        }
    }
    packet_received = 0; // reset packet_received flag
    return bulkOutReq->actual;
}

// sends CSW
static uint32_t send_csw(void *buff, uint32_t size)  {
    uint32_t status;
    bulkInReq->buf = buff;
    bulkInReq->dma = (uintptr_t) buff;
    bulkInReq->complete = bulkInCmpl;
    bulkInReq->length = size;
    bulkInReq->streamId = 0;

    do {
        disable_int();
        status = epIn->ops->reqQueue(pD, epIn, bulkInReq);
        enable_int();
        if (status != 0) {
            status = status;
        }
    } while (status != EOK);

    while (!packet_sent) {
        if (!configValue) {
            return 0;
        }
    }
    packet_sent = 0;
    return bulkInReq->actual;
}

int mscProtocol() {
    uint8_t data_count; // keeps number of received bytes on CBW
    cbw_t * cbw; // Command Block Wrapper auxiliary structure
    csw_t * csw; // Command Block Status auxiliary structure
    uint32_t data_transfered; // Number of bytes transferred on data phase
    uint16_t error_code; // error code returned from SCSI command parser

    // check if critical error occured
    while (msc_failed_flag) {
    }
    disable_int();
    if (msc_failed_flag)
        msc_failed_flag = 0;
    enable_int();

    if (configValue) {

        // receive CBW command
        data_count = rec_cbw(command_buff, 31);

        if (data_count == 0 || !configValue)
            return 1;//continue;

        if (data_count != 31) {
            msc_failed();
            vDbgMsg(DBG_USB_APP, DBG_CRIT, "Bad CBW length %d\n", data_count);
            return 1; //continue;
        }

        cbw = (cbw_t *) command_buff;
        csw = (csw_t *) response_buff;

        if (le32ToCpu(cbw->dCBWSignature) != 0x43425355) {
            msc_failed();
            vDbgMsg(DBG_USB_APP, DBG_CRIT, "Bad signature!!!%s\n","");
            return 1; //continue;
        }

        vDbgMsg(DBG_USB_BOT_APP, DBG_WARN, "BOT CBW-> %s\n","");
        vDbgMsg(DBG_USB_BOT_APP, DBG_WARN, "bmCBWFlags: %02X,\n ", cbw->bmCBWFlags);
        vDbgMsg(DBG_USB_BOT_APP, DBG_WARN, "bmCBWLUN: %d, \n", cbw->bmCBWLun);
        vDbgMsg(DBG_USB_BOT_APP, DBG_WARN, "bmCBWLength: %d,\n", cbw->bmCBWLength);
        vDbgMsg(DBG_USB_BOT_APP, DBG_WARN, "dCBWDataTransferLength: %ld,\n", le32ToCpu(cbw->dCBWDataTransferLength));
        vDbgMsg(DBG_USB_BOT_APP, DBG_WARN, "dCBWSignature: %08lX,\n", le32ToCpu(cbw->dCBWSignature));
        vDbgMsg(DBG_USB_BOT_APP, DBG_WARN, "dCBWTag: %08lX\n", le32ToCpu(cbw->dCBWTag));

        // Execute command SCSI
        dev_num_of_bytes = 0;
        dir_error = 0;
        host_num_of_bytes = le32ToCpu(cbw->dCBWDataTransferLength);
        data_dir = cbw->bmCBWFlags;
        csw->dCSWTag = cbw->dCBWTag;

        // handle CBW command
        error_code = scsiExecCmd((uint8_t*) cbw->cbwcb, &data_transfered);
        if (!configValue) {
            return 1; //continue;
        }
        // Send a Command Status Wrapper
        csw->dCSWSignature = cpuToLe32(0x53425355);
        csw->dCSWDataResidue =
                cpuToLe32(host_num_of_bytes - data_transfered);
        csw->dCSWStatus = error_code;

        if (error_code == ERR_SCSI_UNKNOWN_COMMAND) {
            epIn->ops->epSetHalt(pD, epIn, 1);
            csw->dCSWStatus = 1;
        }

        // thirteen cases handling, see mass stor usb.org spec.

        // ------- case 1, 2, 3 - host expects no data transfers
        if (host_num_of_bytes == 0) {

            // no need to handle case 1 - no errors

            // case 2 and 3
            if (dev_num_of_bytes > 0) {
                vDbgMsg(DBG_USB_BOT_APP, DBG_WARN, "case 2,3%s\n", "");
                csw->dCSWStatus = 2;
            }
        } else

            // ---- case 4, 5, 6, 7, 8 - host expect to recieve data
            if (host_num_of_bytes > 0 && data_dir) {

            // case 8
            if (dir_error) {
                epIn->ops->epSetHalt(pD, epIn, 1);
                csw->dCSWStatus = 2;
                vDbgMsg(DBG_USB_BOT_APP, DBG_WARN, "case 8%s\n", "");
            } else
                // case 4, 5
                if (host_num_of_bytes > dev_num_of_bytes) {
                    vDbgMsg(DBG_USB_BOT_APP, DBG_WARN, "case 4,5%s\n", "");

              epIn->ops->epSetHalt(pD, epIn, 1);
            } else

                // no need to handle case 6 - no errors

                // case 7
                if (host_num_of_bytes < dev_num_of_bytes) {
                vDbgMsg(DBG_USB_BOT_APP, DBG_WARN, "case 7%s\n", "");
                if ((host_num_of_bytes % 512) == 0) {
                    vDbgMsg(DBG_USB_BOT_APP, DBG_WARN, "STATUS=2%s\n", "");
                    csw->dCSWStatus = 2;
                }
            }

        } else
            // ---- case 9, 10, 11, 12, 13 - host expect to send data
            if (host_num_of_bytes > 0 && !data_dir) {

            // case 10
            if (dir_error) {
                vDbgMsg(DBG_USB_BOT_APP, DBG_WARN, "case 10%s\n", "");
                epOut->ops->epSetHalt(pD, epOut, 1);
                vDbgMsg(DBG_USB_BOT_APP, DBG_WARN, "STATUS=2%s\n", "");
                csw->dCSWStatus = 2;
            } else
                // case 9, 11
                if (host_num_of_bytes > dev_num_of_bytes) {
                vDbgMsg(DBG_USB_BOT_APP, DBG_WARN, "case 9,11%s\n", "");
                vDbgMsg(DBG_USB_BOT_APP, DBG_WARN, "STALL EP-OUT%s\n", "");
                epOut->ops->epSetHalt(pD, epOut, 1);
            } else
                // case 13
                if (host_num_of_bytes < dev_num_of_bytes) {
                vDbgMsg(DBG_USB_BOT_APP, DBG_WARN, "case 13%s\n", "");
                vDbgMsg(DBG_USB_BOT_APP, DBG_WARN, "STATUS=2%s\n", "");
                csw->dCSWStatus = 2;
            }
        }
        vDbgMsg(DBG_USB_BOT_APP, DBG_WARN, "BOT CSW<-%s\n", "");
        vDbgMsg(DBG_USB_BOT_APP, DBG_WARN, "dCSWDataResidue: %ld,\n ", le32ToCpu(csw->dCSWDataResidue));
        vDbgMsg(DBG_USB_BOT_APP, DBG_WARN, "dCSWSignature: %08lX,\n ", le32ToCpu(csw->dCSWSignature));
        vDbgMsg(DBG_USB_BOT_APP, DBG_WARN, "dCSWStatus: %02X,\n ", csw->dCSWStatus);
        vDbgMsg(DBG_USB_BOT_APP, DBG_WARN, "dCSWTag: %08lX\n", le32ToCpu(csw->dCSWTag));

        // send CSW
        send_csw(csw, 13);

    } else {

    }
    return 0;
}

int mscAppInit(){
    CUSBD_Dev * dev; // USB device pointer
    CUSBD_ListHead *list; // used in for_each loop

    // set unicode strings
    get_unicode_string(vendorDesc, USB_MANUFACTURER_STRING);
    get_unicode_string(productDesc, USB_PRODUCT_STRING);
    get_unicode_string(serialDesc, USB_SERIAL_NUMBER_STRING);

    // align buffers to modulo8 address
    ep0Buff = alignBuff(ep0BuffAlloc, 8);
    command_buff = alignBuff(command_buffAlloc, 8);
    response_buff = alignBuff(response_buffAlloc, 8);

    // checking device and endpoints parameters correctness
    // get CUSBD device instance exposed as Linux gadget device
    drv->getDevInstance(pD, &dev);
    displayDeviceInfo(dev);

    // allocate request for ep0
    drv->reqAlloc(pD, dev->ep0, &ep0Req);

    /*Change descriptor for maxSpeed == HS only Device*/
    /*For USB2.0 we have to modified wTotalLength of BOS descriptor*/
    if (dev->maxSpeed < CH9_USB_SPEED_SUPER) {
        bosDesc.wTotalLength =
                cpuToLe16(CH9_USB_DS_BOS + CH9_USB_DS_DEVICE_CAPABILITY_20);
        bosDesc.bNumDeviceCaps = 1;
        devHsDesc.bcdUSB = cpuToLe16(BCD_USB_HS_ONLY);
    }

    scsiInit(); // init SCSI driver
    scsi_send_data = send_data;
    storage_send_data = storage_to_usb;
    storage_rec_data = usb_to_storage;

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
    return 0;
}

/*
 * Mass storage application
 */
int mscApp()  {

    uint32_t res; // keeps result of last operation on driver
    CUSBD_SysReq sysReq; // returns driver requirements
    CUSBD_Dev * dev; // USB device pointer
    uint8_t data_count; // keeps number of received bytes on CBW
    CUSBD_ListHead *list; // used in for_each loop
    uint16_t error_code; // error code returned from SCSI command parser
    uint32_t data_transfered; // Number of bytes transferred on data phase
    cbw_t * cbw; // Command Block Wrapper auxiliary structure
    csw_t * csw; // Command Block Status auxiliary structure
    uint8_t isDataOnChip;

#ifdef DEBUG    
    DbgMsgSetLvl(DBG_HIVERB);
    DbgMsgEnableModule(DBG_USB_APP | DBG_USB_APP_VERBOSE | DBG_USB_BOT_APP | DBG_USB_SCSI_APP);
#endif
    
    drv = CUSBD_GetInstance(); // get driver handle

    res = drv->probe(&config, &sysReq);
    if (res != 0)
        goto error;
    vDbgMsg(DBG_USB_APP, 1, "Device memory requirement: %ld bytes\n", sysReq.privDataSize);

    // allocate memory for device object
    pD = (uint8_t*) malloc((size_t) (sysReq.privDataSize));
    if(!pD) goto error;
    
    config.trbAddr = malloc(sysReq.trbMemSize);
    if(!config.trbAddr)
        goto error;
    config.trbDmaAddr = (uintptr_t)config.trbAddr;    
    
    res = drv->init(pD, &config, &callback);

    if (res != 0)
        goto error;
    vDbgMsg(DBG_USB_APP, 1, "Initializing OK! %s\n", " ");

    mscAppInit();

    drv->start(pD); // start driver

    while (!configValue) {
    vDbgMsg(DBG_USB_APP, 1, "Wait for device has been configured!%s\n", " ");
    }

    while (1) {
    mscProtocol();
    }

error:
    vDbgMsg(DBG_USB_APP, 1, "Error %08lX\n", res);
    // free allocated memory
    if (pD)free(pD);      
    if (config.trbAddr)
        free (config.trbAddr);        
    return (res);
}
