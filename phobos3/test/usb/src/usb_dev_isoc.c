/**********************************************************************
 * Copyright (C) 2014-2017 Cadence Design Systems, Inc.
 * All rights reserved worldwide.
 ***********************************************************************
 * usb_dev_isoc.c
 * Application for testing isochronous transfers on OUT and IN endpoints.
 * This application work correctly only with enabled dmultEnabled option
 * in configuration.
 *
 ***********************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "usb_dev_api.h"
#include "cusbd_if.h"
#include "byteorder.h"
#include "cdn_errno.h"                     // errors definitions
#include "log.h"

#include "map_system_memory.h"


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



//-------------- Driver Configuration Section -------------------------

#ifdef ID_PRODUCT
#undef ID_PRODUCT
#endif
#define ID_PRODUCT 0x1005

#ifdef USB_PRODUCT_STRING
#undef USB_PRODUCT_STRING
#endif
#define USB_PRODUCT_STRING "Isochronous transfers tester"

/* For Iscochronous application we need at least double buffering */
#define BUFFERING 2
#define PACKET_PER_REQUEST 200
#define BUFFER_IN_SIZE (PACKET_PER_REQUEST * 3 * 1024)
#define BUFFER_OUT_SIZE (PACKET_PER_REQUEST * 3 * 1024)

#if BUFFERING < 2
#error "Number of buffers for Isochronous transfer hast to be at least equal to 2"
#endif

#define CONF1_NUMBER_ALT_INTEFACE 4
#define CONF1_NUMBER_INTEFACE 1
#define USB_CONFIGURATION1_STRING "Isochronous Configuration"
#define USB_CONFIGURATION1_STRING_INDEX 4

/*USB endpoint used for Endpoint Descriptors*/
#define EP_IN_NR 0x83
#define EP_OUT_NR 0x03

/*Max Packet Size used by alternate settings*/
static uint16_t packetSizeIntEpIn[CONF1_NUMBER_ALT_INTEFACE] = {512, 1024, 2561, 5120};
static uint16_t packetSizeIntEpOut[CONF1_NUMBER_ALT_INTEFACE] = {512, 1024, 2561, 5120};

/*structure extending standard  usbDevice object*/
struct usbIsocDevice {
    struct usbDevice usbDevice;
    uint8_t epInNr;
    uint8_t epOutNr;
    uint8_t *inBuff[BUFFERING];
    uint8_t *outBuff[BUFFERING];
    CUSBD_Req *inReq[BUFFERING];  // USB Device Request for IN (RX) transfer;
    CUSBD_Req *outReq[BUFFERING]; // USB Device Request for OUT (TX) transfer;
    CUSBD_Ep *epIn;    //pointer to instance of Device IN endpoint
    CUSBD_Ep *epOut;   //pointer to instance of Device OUT endpoint
};

struct usbIsocDevice dev;

static void devPrepareRequests(uint8_t configValue);

/**
 * Function prepare Configuration for this device
 */
uint32_t devPrepareConfiguration() {
    uint8_t * configBuffer = NULL;
    uint8_t i = 0;
    struct usbDevice * udev = &dev.usbDevice;
    CH9_UsbConfigurationDescriptor * config = NULL;
    CH9_UsbInterfaceDescriptor tplInterfaceDesc;
    CH9_UsbEndpointDescriptor tplEndpointInDesc;
    CH9_UsbEndpointDescriptor tplEndpointOutDesc;
    uint32_t configurationSize = 0;

    configurationSize = CH9_USB_DS_CONFIGURATION
                        + CH9_USB_DS_INTERFACE * CONF1_NUMBER_INTEFACE
                        + CH9_USB_DS_INTERFACE * CONF1_NUMBER_ALT_INTEFACE // number of Alternate Settings
                        + CH9_USB_DS_ENDPOINT * CONF1_NUMBER_ALT_INTEFACE * 2; //each Alternate Setting has 2 endpoints

    // registered string strings
    devAddString(udev, USB_MANUFACTURER_STRING_INDEX, USB_MANUFACTURER_STRING);
    devAddString(udev, USB_PRODUCT_STRING_INDEX, USB_PRODUCT_STRING);
    devAddString(udev, USB_SERIAL_NUMBER_STRING_INDEX, USB_SERIAL_NUMBER_STRING);

    if (dev.usbDevice.usbDev->speed == CH9_USB_SPEED_SUPER) {
        configurationSize += CH9_USB_DS_SS_USB_ENDPOINT_COMPANION;
        config = &confSsDesc;
    } else{
        config = &confHsDesc;
    }

    configBuffer = malloc(configurationSize);
    if(!configBuffer) {
        return ENOMEM;
    }

    config->bNumInterfaces = CONF1_NUMBER_INTEFACE;

    config->bmAttributes = CH9_USB_CONFIG_RESERVED;
    if(DEVICE_IS_SELF_POWERED) {
        config->bmAttributes |=  CH9_USB_CONFIG_SELF_POWERED;
    }
    if(DEVICE_IS_REMOTE_WAKEUP) {
        config->bmAttributes |= CH9_USB_CONFIG_REMOTE_WAKEUP;
    }

    config->bConfigurationValue = 1;

    config->iConfiguration = USB_CONFIGURATION1_STRING_INDEX;
    devAddString(udev, USB_CONFIGURATION1_STRING_INDEX, USB_CONFIGURATION1_STRING);
    devAddConfiguration(udev, configBuffer, config);

    devFillTplInterface(tplInterfaceDesc, 0, 0, 0, CH9_USB_CLASS_VENDOR, 0, 0, 0);
    devAddDescriptor(udev, config->bConfigurationValue, (const uint8_t*)&tplInterfaceDesc);

    for(i = 1; i <= CONF1_NUMBER_ALT_INTEFACE; i++) {
        uint16_t maxPacketSizeEpOut, maxPacketSizeEpIn;
        uint8_t multIn, multOut;

        devFillTplInterface(tplInterfaceDesc, 0, i, 2, CH9_USB_CLASS_VENDOR, 0, 0, 0);

        maxPacketSizeEpOut = packetSizeIntEpIn[i - 1];
        maxPacketSizeEpIn = packetSizeIntEpOut[i - 1];
        multIn =  packetSizeIntEpIn[i - 1] >> 11;
        multOut = packetSizeIntEpOut[i - 1] >> 11;

        if (udev->usbDev->speed == CH9_USB_SPEED_FULL) {
            if(maxPacketSizeEpOut > 1023 ||  multOut) {
                maxPacketSizeEpOut = 1023;
                multOut = 0;
            }
            if(maxPacketSizeEpIn > 1023 ||  multIn) {
                maxPacketSizeEpIn = 1023;
                multIn = 0;
            }
        }

        if (udev->usbDev->speed == CH9_USB_SPEED_SUPER) {
            /*For Supper Speed we cant set the number of additional transaction in Endpoint Descriptor*/
            maxPacketSizeEpOut = maxPacketSizeEpOut & 0x7FFF;
            maxPacketSizeEpIn = maxPacketSizeEpIn & 0x7FFF;
        }

        devFillTplEndpoint(tplEndpointInDesc, dev.epInNr, CH9_USB_EP_ISOCHRONOUS, maxPacketSizeEpIn, 1);
        devFillTplEndpoint(tplEndpointOutDesc, dev.epOutNr, CH9_USB_EP_ISOCHRONOUS, maxPacketSizeEpOut, 1);

        devAddDescriptor(udev, config->bConfigurationValue, (const uint8_t*)&tplInterfaceDesc);
        devAddDescriptor(udev, config->bConfigurationValue, (const uint8_t*)&tplEndpointInDesc);

        if (udev->usbDev->speed == CH9_USB_SPEED_SUPER) {
            compDesc.bmAttributes |= multIn,
            compDesc.wBytesPerInterval = cpuToLe16(maxPacketSizeEpIn * multIn);
            devAddDescriptor(udev, config->bConfigurationValue, (const uint8_t*)&compDesc);
        }

        devAddDescriptor(udev, config->bConfigurationValue, (const uint8_t*)&tplEndpointOutDesc);

        if (udev->usbDev->speed == CH9_USB_SPEED_SUPER) {
            compDesc.bmAttributes |= multOut,
            compDesc.wBytesPerInterval = cpuToLe16(maxPacketSizeEpOut * multOut);
            devAddDescriptor(udev, config->bConfigurationValue, (const uint8_t*)&compDesc);
        }

    }

    /*Only USB3 should support CH9_USB_DS_DEVICE_CAPABILITY_30 descriptor*/
    if(udev->usbDev->maxSpeed == CH9_USB_SPEED_SUPER || udev->usbDev->maxSpeed == CH9_USB_SPEED_SUPER_PLUS) {
        udev->bosDescriptor = malloc(CH9_USB_DS_BOS + CH9_USB_DS_DEVICE_CAPABILITY_30 + CH9_USB_DS_DEVICE_CAPABILITY_20);
    } else{
        udev->bosDescriptor = malloc(CH9_USB_DS_BOS + CH9_USB_DS_DEVICE_CAPABILITY_20);
    }

    if(!udev->bosDescriptor) {
        return ENOMEM;
    }

    /**
     * Change descriptor for maxSpeed == HS only Device
     * For USB2.0 we have to modified wTotalLength of BOS descriptor
     */
    if (udev->usbDev->maxSpeed == CH9_USB_SPEED_HIGH) {
        bosDesc.wTotalLength = cpuToLe16(CH9_USB_DS_BOS + CH9_USB_DS_DEVICE_CAPABILITY_20);
        bosDesc.bNumDeviceCaps = 1;
        devHsDesc.bcdUSB = cpuToLe16(BCD_USB_HS_ONLY);
    }

    memmove(udev->bosDescriptor, &bosDesc, CH9_USB_DS_BOS);
    i = CH9_USB_DS_BOS;
    if(udev->usbDev->maxSpeed == CH9_USB_SPEED_SUPER || udev->usbDev->maxSpeed == CH9_USB_SPEED_SUPER_PLUS) {
        memmove(&udev->bosDescriptor[i], &capabilitySsDesc, CH9_USB_DS_DEVICE_CAPABILITY_30);
        i += CH9_USB_DS_DEVICE_CAPABILITY_30;
    }

    memmove(&udev->bosDescriptor[i], &capabilityExtDesc, CH9_USB_DS_DEVICE_CAPABILITY_20);

    switch (udev->usbDev->speed) {
    case CH9_USB_SPEED_FULL:
        udev->devDesc = &devHsDesc;
        break;
    case CH9_USB_SPEED_HIGH:
        udev->devDesc = &devHsDesc;
        break;
    case CH9_USB_SPEED_SUPER:
        udev->devDesc  = &devSsDesc;
        break;
    default:
        DbgMsg(DBG_USB_APP_VERBOSE, 1, "ERROR: uknown usb Device speed %d\n", udev->usbDev->speed);
        return EINVAL;
    }

    return 0;
}

static void devReqComplete(struct CUSBD_Ep *ep, struct CUSBD_Req * req) {
    vDbgMsg(DBG_USB_APP_VERBOSE, 1, "Request on endpoint %s completed\n", ep->name);
}

//--------------- Core Driver callback definitions ----------------------------
static uint32_t devRemoveRequests();

static void devConnect(void *pD) {
    uint32_t retval = 0;
    uint8_t i;
    struct usbDevice * udev = &dev.usbDevice;

    //TODO: As alternative firmware in this place can select endpoint from configuration
    dev.epInNr =  EP_IN_NR;
    dev.epOutNr = EP_OUT_NR;
    udev->drv =  CUSBD_GetInstance();
    udev->pD = pD;

    // get CUSBD device instance
    udev->drv->getDevInstance(pD, &udev->usbDev);
    for(i = 0; i < BUFFERING; i++) {
        dev.inBuff[i] = malloc(BUFFER_IN_SIZE);
        if(!dev.inBuff[i]) {
            udev->status = USB_DEV_APP_CRITICAL_ERRROR;
            udev->lastError = ENOMEM;
            return;
        }

        dev.outBuff[i] = malloc(BUFFER_OUT_SIZE);
        if(!dev.outBuff[i]) {
            udev->status = USB_DEV_APP_CRITICAL_ERRROR;
            udev->lastError = ENOMEM;
            goto errorMalloc1;
        }
    }

    retval = udev->drv->reqAlloc(pD, udev->usbDev->ep0, &udev->ep0Req);
    if(retval) {
        udev->status = USB_DEV_APP_CRITICAL_ERRROR;
        udev->lastError = retval;
        goto errorMalloc2;
    }

    udev->ep0Req->complete = devReqComplete;

    if((retval = devPrepareConfiguration())) {
        udev->status = USB_DEV_APP_CRITICAL_ERRROR;
        udev->lastError = retval;
        goto errorMalloc2;
    }

    vDbgMsg(DBG_USB_APP_VERBOSE, 1, "Application: connect at %d speed\n", udev->usbDev->speed);
    udev->status |= USB_DEV_APP_INITIALISED;
    udev->status &= ~USB_DEV_APP_DISCONNECTED;
    return;

errorMalloc2:
    for(i = 0; i < BUFFERING; i++) {
        free(dev.outBuff[i]);
    }
errorMalloc1:
    for(i = 0; i < BUFFERING; i++) {
        free(dev.inBuff[i]);
    }

}

static void devDisconnect(void *pD) {
    uint32_t err = 0;
    uint8_t i = 0;
    struct usbDevice * udev = &dev.usbDevice;

    if(udev->status & USB_DEV_APP_INITIALISED) {
        /**free memory for buffers*/
        for(i = 0; i < BUFFERING; i++) {
            free(dev.inBuff[i]);
            free(dev.outBuff[i]);
            dev.inBuff[i] = NULL;
            dev.outBuff[i] = NULL;
        }

        /** free request for ep0 */
        if(udev->ep0Req->status == EINPROGRESS) {
            err = udev->drv->reqDequeue(pD, udev->usbDev->ep0, udev->ep0Req);
            if(err) {
                vDbgMsg(DBG_USB_APP_VERBOSE, 1, "Error returned from reqDequeue: %d\n", err);
            }
            udev->drv->reqFree(pD, udev->usbDev->ep0, udev->ep0Req);
        }

        /** free requests for other endpoints IN/OUT */
        err = devRemoveRequests();
        if(err) {
        }

        /** Free all created configuration*/
        for(i = 0; i < MAX_CONFIGURATION_NUMBER; i++) {
            if(udev->configurations[i]) {
                free(udev->configurations[i]);
                udev->configurations[i] = NULL;
            }
        }

        /** free memory for BOS Descriptor */
        if(udev->bosDescriptor) {
            free(udev->bosDescriptor);
            udev->bosDescriptor = NULL;
        }

    }

    udev->status |= USB_DEV_APP_DISCONNECTED;
    udev->status &= ~USB_DEV_APP_INITIALISED;
    udev->status &= ~USB_DEV_APP_CONFIGURED;
    udev->status &= ~USB_DEV_APP_SUSPENDED;

    /** free all registered string descriptors */
    devDelString(udev, 0);

    udev->currentAltSetting = 0;
    udev->currentConfig = 0;

    vDbgMsg(DBG_USB_APP_VERBOSE, 1, "Application: disconnect %c\n", ' ');
}

static void devResume(void *pD) {
    dev.usbDevice.status &= ~USB_DEV_APP_SUSPENDED;
    vDbgMsg(DBG_USB_APP_VERBOSE, 1, "Application: resume %c\n", ' ');
}

uint32_t devSetInterface(uint8_t ifNr, uint8_t altIfNr) {
    int i = 0;
    uint32_t length;
    CH9_UsbInterfaceDescriptor * interface = NULL;
    CH9_UsbConfigurationDescriptor * config;
    CH9_UsbEndpointDescriptor * endpoint;
    struct CUSBD_Ep * ep;
    CUSBD_ListHead *list;
    uint8_t *buff = NULL;
    struct usbDevice * udev = &dev.usbDevice;

    config = devGetConfiguration(udev, udev->currentConfig);

    if(!config) {
        return EINVAL;
    }

    /*Get current  used interface*/
    devFindInterfaceInConfiguration(config, ifNr, udev->currentAltSetting, &interface);
    if(!interface) {
        return EINVAL;
    }

    length = le16ToCpu(config->wTotalLength) - ((uintptr_t)interface - (uintptr_t)config);

    /*Disable current alternate setting*/
    buff = (uint8_t*)interface;
    for(i = 0; i < interface->bNumEndpoints; i++) {
        endpoint = devFindEndpointInInterface((uint8_t*)buff, length);
        if(!endpoint) {
            continue;
        }

        length -= ((uintptr_t)endpoint - (uintptr_t)buff) + endpoint->bLength;
        buff = (uint8_t*)endpoint + endpoint->bLength;

        /*Disable all endpoints*/
        for (list = udev->usbDev->epList.next; list != &udev->usbDev->epList; list = list->next) {
            ep = (struct CUSBD_Ep *) list;
            if (ep->address == endpoint->bEndpointAddress) {
                ep->ops->epDisable(udev->pD, ep);
                break;
            }
        }
    }
    /** free all used requests*/
    devRemoveRequests(udev->currentConfig);
    dev.epIn = NULL;
    dev.epOut = NULL;

    udev->currentAltSetting = 0;
    /*Get new selected interface */
    devFindInterfaceInConfiguration(config, ifNr, altIfNr, &interface);
    if(!interface) {
        return EINVAL;
    }

    length = le16ToCpu(config->wTotalLength) - ((uintptr_t)interface - (uintptr_t)config);

    /*Enable current alternate setting*/
    buff = (uint8_t*)interface;
    for(i = 0; i < interface->bNumEndpoints; i++) {
        endpoint = devFindEndpointInInterface((uint8_t*)buff, length);
        if(!endpoint) {
            continue;
        }

        length -= ((uintptr_t)endpoint - (uintptr_t)buff) + endpoint->bLength;
        buff = (uint8_t*)endpoint + endpoint->bLength;

        /*Enable all for given interface endpoints*/
        for (list = udev->usbDev->epList.next; list != &udev->usbDev->epList; list = list->next) {
            ep = (struct CUSBD_Ep *) list;
            if (ep->address == endpoint->bEndpointAddress) {
                if(endpoint->bEndpointAddress & CH9_USB_EP_DIR_IN) {
                    dev.epIn = ep;
                    dev.epInNr =  endpoint->bEndpointAddress & CH9_USB_EP_NUMBER_MASK;
                } else {
                    dev.epOut = ep;
                    dev.epOutNr = endpoint->bEndpointAddress & CH9_USB_EP_NUMBER_MASK;
                }
                ep->ops->epEnable(udev->pD, ep, endpoint);
                break;
            }
        }
    }
    udev->currentAltSetting = altIfNr;
    return 0;
}

uint32_t devSetConfiguration(uint8_t configNum) {
    CH9_UsbConfigurationDescriptor * newConfig;
    CH9_UsbConfigurationDescriptor * currentConfig;
    struct CUSBD_Ep * ep;
    CUSBD_ListHead *list;
    vDbgMsg(DBG_USB_APP_VERBOSE, 1, "SET CONFIGURATION(%d)\n", configNum);
    struct usbDevice * udev = &dev.usbDevice;

    newConfig = devGetConfiguration(udev, configNum);
    if(!newConfig) {
        return ESTALL;
    }

    if(configNum == 0) {
        /*Disable all endpoints*/
        for (list = udev->usbDev->epList.next; list != &udev->usbDev->epList; list = list->next) {
            ep = (struct CUSBD_Ep *) list;
            if (ep->address == (dev.epInNr | CH9_USB_EP_DIR_IN)) {
                ep->ops->epDisable(udev->pD, ep);
                break;
            }
        }
        for (list = udev->usbDev->epList.next; list != &udev->usbDev->epList; list = list->next) {
            ep = (struct CUSBD_Ep *) list;
            if (ep->address == dev.epOutNr) {
                ep->ops->epDisable(udev->pD, ep);
                break;
            }
        }

        /** free all used requests*/
        devRemoveRequests(udev->currentConfig);
        dev.epIn = NULL;
        dev.epOut = NULL;
        udev->status &= ~USB_DEV_APP_CONFIGURED;
        currentConfig = 0;
        udev->currentAltSetting = 0;
        return 0;
    }

    if(udev->currentConfig) {
        currentConfig = devGetConfiguration(udev, udev->currentConfig);

        /** device already configured */
        if((currentConfig!=NULL) && (currentConfig->bConfigurationValue == newConfig->bConfigurationValue)) {
            return 0;
        }
    }
    udev->currentConfig = newConfig->bConfigurationValue;

    devSetInterface(0, 0);

    devPrepareRequests(newConfig->bConfigurationValue);
    /*Code control  Self powered feature of USB*/
    if (newConfig->bmAttributes & CH9_USB_CONFIG_SELF_POWERED) {
        if (udev->drv->dSetSelfpowered) {
            udev->drv->dSetSelfpowered(udev->pD);
        }
    } else {
        if (udev->drv->dClearSelfpowered) {
            udev->drv->dSetSelfpowered(udev->pD);
        }
    }
    return 0;
}

static uint32_t devSetup(void *pD, CH9_UsbSetup *ctrl) {
    // get device reference
    uint16_t length = 0;
    uint8_t * buffer = NULL;
    uint32_t retval;
    uint8_t i = 0;
    CH9_UsbConfigurationDescriptor *configDesc;
    struct usbDevice * udev = &dev.usbDevice;

    ctrl->wIndex = le16ToCpu(ctrl->wIndex);
    ctrl->wLength = le16ToCpu(ctrl->wLength);
    ctrl->wValue = le16ToCpu(ctrl->wValue);

    vDbgMsg(DBG_USB_APP_VERBOSE, 1, "Speed %d: SETUP: [bRequest, bRequestType, wIndex, wValue, wLength \n", udev->usbDev->speed,
            ctrl->bRequest, ctrl->bmRequestType, ctrl->wIndex, ctrl->wValue, ctrl->wLength );

    udev->ep0Req->buf = udev->ep0Buff;
    udev->ep0Req->dma = (uintptr_t) udev->ep0Buff;

    switch (ctrl->bmRequestType & CH9_USB_REQ_TYPE_MASK) {

    case CH9_USB_REQ_TYPE_STANDARD:
        switch (ctrl->bRequest) {
        case CH9_USB_REQ_GET_DESCRIPTOR:
            vDbgMsg(DBG_USB_APP_VERBOSE, 1, "GET DESCRIPTOR %c\n", ' ');
            if ((ctrl->bmRequestType & CH9_REQ_RECIPIENT_MASK) == CH9_USB_REQ_RECIPIENT_INTERFACE) {
                switch (ctrl->wValue >> 8) {
                default:
                    return ESTALL;
                }
            } else {
                if ((ctrl->bmRequestType & CH9_REQ_RECIPIENT_MASK) == CH9_USB_REQ_RECIPIENT_DEVICE) {
                    switch (ctrl->wValue >> 8) {
                    case CH9_USB_DT_DEVICE:
                    {
                        buffer =  (uint8_t*)udev->devDesc;
                        length = CH9_USB_DS_DEVICE;
                        vDbgMsg(DBG_USB_APP_VERBOSE, 1, "DevDesc[0] = %d\n", udev->devDesc->bLength);
                        break;
                    }
                    case CH9_USB_DT_CONFIGURATION:
                    case CH9_USB_DT_OTHER_SPEED_CONFIGURATION:
                    {
                        uint8_t descIndex = (uint8_t) (ctrl->wValue & 0xFF);
                        buffer =  (uint8_t*)udev->configurations[descIndex];
                        configDesc = (CH9_UsbConfigurationDescriptor*)buffer;

                        configDesc->bDescriptorType = CH9_USB_DT_CONFIGURATION;

                        length = le16ToCpu(configDesc->wTotalLength);
                        vDbgMsg(DBG_USB_APP_VERBOSE, 1, "ConfDesc[0] = %02X\n", configDesc->bLength);
                        break;
                    }
                    case CH9_USB_DT_STRING:
                    {
                        CH9_UsbStringDescriptor * strDesc;
                        uint8_t descIndex = (uint8_t) (ctrl->wValue & 0xFF);
                        vDbgMsg(DBG_USB_APP_VERBOSE, 1, "StringDesc %c\n", ' ');
                        if(descIndex == 0) {
                            buffer = (uint8_t*)&languageDesc;
                        } else {
                            buffer = devGetString(udev, descIndex);
                        }

                        if(!buffer) {
                            return ESTALL;
                        }

                        strDesc = (CH9_UsbStringDescriptor *)buffer;
                        length = strDesc->bLength;
                        break;
                    }
                    case CH9_USB_DT_BOS:
                    {
                        buffer = udev->bosDescriptor;
                        length = le16ToCpu(((CH9_UsbBosDescriptor*)udev->bosDescriptor)->wTotalLength);

                        vDbgMsg(DBG_USB_APP_VERBOSE, 1, "bosDesc[0] = %02X\n", length);
                        break;
                    }
                    case CH9_USB_DT_DEVICE_QUALIFIER:
                    {
                        length = CH9_USB_DS_DEVICE_QUALIFIER;
                        buffer  = (uint8_t*)&qualifierDesc;
                        vDbgMsg(DBG_USB_APP_VERBOSE, 1, "QualifierDesc %c\n", ' ');
                        break;
                    }
                    default:
                        vDbgMsg(DBG_USB_APP_VERBOSE, 1, "Error_1 %c\n", ' ');
                        return ESTALL;

                    }    //switch
                }
            }        //if
            break;

        case CH9_USB_REQ_SET_CONFIGURATION:
        {
            retval = devSetConfiguration(ctrl->wValue);
            if(retval) {
                return retval;
            }
            return 0;
        }
        case CH9_USB_REQ_GET_CONFIGURATION:
            length = 1;
            buffer = &udev->currentConfig;
            break;
        case CH9_USB_REQ_SET_INTERFACE:
            length = 0;
            devSetInterface(ctrl->wIndex, ctrl->wValue);
            devPrepareRequests(udev->currentConfig);
            for(i = 0; i < BUFFERING; i++) {
                if(dev.epIn && dev.inReq[i]) {
                    dev.epIn->ops->reqQueue(pD, dev.epIn, dev.inReq[i]);
                }
                if(dev.epOut && dev.outReq[i]) {
                    dev.epOut->ops->reqQueue(pD, dev.epOut, dev.outReq[i]);
                }
            }
            break;
        default:
            vDbgMsg(DBG_USB_APP_VERBOSE, 1, "Error_2 %c\n", ' ');
            return ESTALL;         //return error
        }
        break;

    case CH9_USB_REQ_TYPE_CLASS:
        return ESTALL;
        break;
    }

    if (length > 0) {
        if(buffer) {
            memmove(udev->ep0Buff, buffer, length);
        }
        udev->ep0Req->length = ctrl->wLength < length ? ctrl->wLength : length;
        udev->usbDev->ep0->ops->reqQueue(udev->pD, udev->usbDev->ep0, udev->ep0Req);
    }
    return 0;
}

static void devSuspend(void *pD) {
    struct usbDevice * udev = &dev.usbDevice;
    udev->status |= USB_DEV_APP_SUSPENDED;
    vDbgMsg(DBG_USB_APP_VERBOSE, 1, "Application: suspend %c\n", ' ');
}

static void * devRequestMemAlloc(void * pD, uint32_t requireSize) {
    return malloc(requireSize);
}

static void devRequestMemFree(void * pD, void* usbRequest) {
    if (usbRequest) {
        free(usbRequest);
    }
}

CUSBD_Callbacks callback = {
    .connect = devConnect,
    .disconnect = devDisconnect,
    .resume = devResume,
    .setup = devSetup,
    .suspend = devSuspend,
    .usbRequestMemFree = devRequestMemFree,
    .usbRequestMemAlloc = devRequestMemAlloc,
};

static void outCmpl(struct CUSBD_Ep *ep, struct CUSBD_Req * req){

    if (req->status != 0) {
        return;
    }
    if(req->actual > 0) {
        ++(*(uint8_t*)req->buf);
    }
    vDbgMsg(DBG_USB_APP_VERBOSE, 1, "Transfer complete on ep:%02X %08X req\n", ep->address, (uintptr_t) req);
    ++(*(uint8_t*)req->buf);
    ep->ops->reqQueue(dev.usbDevice.pD, ep, req);
}

static void inCmpl(struct CUSBD_Ep *ep, struct CUSBD_Req * req){

    if (req->status != 0) { return;}
    vDbgMsg(DBG_USB_APP_VERBOSE, 1, "Transfer complete on ep:%02X %08X req\n", ep->address, (uintptr_t) req);

    if(req->actual == req->length) {
        ++(*(uint8_t*)req->buf);
    }

    ep->ops->reqQueue(dev.usbDevice.pD, ep, req);
}

// interrupt handler
void devIsoIsr() {
    dev.usbDevice.drv->isr(dev.usbDevice.pD);
}

static void devPrepareRequests(uint8_t configValue) {
    uint32_t flags;
    uint8_t i = 0;
    uint16_t maxpacket;
    uint8_t packetPerInterval;
    struct usbDevice * udev = &dev.usbDevice;

    flags = startCriticalSection();
    if(dev.epOut) {
        for(i = 0; i < BUFFERING; i++) {
            udev->drv->reqAlloc(udev->pD, dev.epOut, &dev.outReq[i]);
            devClearReq(dev.outReq[i]);
            dev.outReq[i]->buf = dev.outBuff[i];
            dev.outReq[i]->dma = (uintptr_t) dev.outBuff[i];
            dev.outReq[i]->complete = outCmpl;
            dev.outReq[i]->noInterrupt = 0;
            maxpacket = dev.epOut->desc->wMaxPacketSize & CH9_MAX_PACKET_SIZE_MASK;
            packetPerInterval = (dev.epOut->desc->wMaxPacketSize >> CH9_PACKET_PER_FRAME_SHIFT) + 1;
            dev.outReq[i]->length = maxpacket * packetPerInterval * PACKET_PER_REQUEST;
        }
    }
    if(dev.epIn) {
        for(i = 0; i < BUFFERING; i++) {
            udev->drv->reqAlloc(udev->pD, dev.epIn, &dev.inReq[i] );
            devClearReq(dev.inReq[i]);
            dev.inReq[i]->buf = dev.inBuff[i];
            dev.inReq[i]->dma = (uintptr_t) dev.inBuff[i];
            dev.inReq[i]->complete = inCmpl;
            dev.inReq[i]->noInterrupt = 0;
            maxpacket = dev.epIn->desc->wMaxPacketSize & CH9_MAX_PACKET_SIZE_MASK;
            packetPerInterval = (dev.epIn->desc->wMaxPacketSize >> CH9_PACKET_PER_FRAME_SHIFT) + 1;
            dev.inReq[i]->length = maxpacket * packetPerInterval * PACKET_PER_REQUEST;
        }
    }
    endCriticalSection(flags);
}

static uint32_t devRemoveRequests() {
    uint32_t flags = 0;
    uint32_t retval;
    uint8_t i = 0;
    struct usbDevice * udev = &dev.usbDevice;

    flags = startCriticalSection();
    for(i = 0; i < BUFFERING; i++) {
        if(dev.epIn) {
            if(dev.inReq[i] && dev.inReq[i]->status == EINPROGRESS) {
                retval = udev->drv->reqDequeue(udev->pD, dev.epIn, dev.inReq[i]);
                if(retval) {
                    vDbgMsg(DBG_USB_APP_VERBOSE, 1, "Error returned from reqDequeue: %lx\n", retval);
                }
            }
            udev->drv->reqFree(udev->pD, dev.epIn, dev.inReq[i]);
        }

        if(dev.epOut) {
            if(dev.outReq[i] && dev.outReq[i]->status == EINPROGRESS) {
                retval = udev->drv->reqDequeue(udev->pD, dev.epOut, dev.outReq[i]);
                if(retval) {
                    vDbgMsg(DBG_USB_APP_VERBOSE, 1, "Error returned from reqDequeue: %lx\n", retval);
                }
            }
            udev->drv->reqFree(udev->pD, dev.epOut, dev.outReq[i]);
        }
    }
    endCriticalSection(flags);
    return retval;
}

int devIsoApp() {
    uint32_t res; // keeps result of operation on driver
    CUSBD_SysReq sysReq;
    CUSBD_Dev * udev; // auxiliary pointer used for testing purposes
    static CUSBD_OBJ * udrv; // driver pointer
    static void * pD; // private data pointer

    udrv = CUSBD_GetInstance(); // get driver handle

    res = udrv->probe(&configDriver, &sysReq);
    if (res != 0) { goto error;}
    vDbgMsg(DBG_USB_CTRL_MSG, 1, "Device memory requirement: %ld bytes\n", sysReq.privDataSize);

    // allocate memory for device object
    pD = (uint8_t*) malloc((size_t) (sysReq.privDataSize));

    configDriver.trbAddr = malloc(sysReq.trbMemSize);
    if (!configDriver.trbAddr) {
        goto error;
    }
    configDriver.trbDmaAddr = (uintptr_t) configDriver.trbAddr;

    res = udrv->init(pD, &configDriver, &callback);
    if (res != 0) { goto error;}
    vDbgMsg(DBG_USB_CTRL_MSG, 1, "Initializing OK! %s\n", " ");

    while(1) {
        if((!dev.usbDevice.status & USB_DEV_APP_DRIVER_STARTED)) {
            //clear dev object (only once)
            memset(&dev, 0, sizeof(struct usbIsocDevice));
            dev.usbDevice.status |= USB_DEV_APP_DRIVER_STARTED;
            udrv->start(pD);
        }
        if(dev.usbDevice.status & USB_DEV_APP_DISCONNECTED) {
        }

        /*All initialization is done in connect event*/
        if(!(dev.usbDevice.status & USB_DEV_APP_INITIALISED)) {
            continue;
        }

        if(dev.usbDevice.status & USB_DEV_APP_SUSPENDED) {
        }
    }

    udrv->stop(pD);
error:
    udrv->destroy(pD);

    // free allocated memory
    if (pD) {
        free(pD);
    }

    vDbgMsg(DBG_USB_CTRL_MSG, 1, "Error %08lX\n", res);
    if (configDriver.trbAddr) {
        free (configDriver.trbAddr);
    }
    return (res);
}
