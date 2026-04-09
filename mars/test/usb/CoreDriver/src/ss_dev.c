/******************************************************************************
 * Copyright (C) 2014-2015 Cadence Design Systems, Inc.
 * All rights reserved worldwide.
 ******************************************************************************
 * ss_dev.c
 * Device Super Speed controller driver,
 *
 * Main source file
 *
 *
 *****************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "cdn_errno.h"
#include "cps.h"
// #include "mmio.h"

#include "cusbd_if.h"
#include "byteorder.h"

#include "ss_dev_hw.h"
#include "cusb_dma_if.h"
#include "sgdma_regs.h"
#define ListHead CUSBD_ListHead
#include "sduc_list.h"
#include <debug.h>

#define DBG_CUSBD_ISR_MSG 0x00010000
#define DBG_CUSBD_INIT_EP_MSG 0x00020000
#define DBG_CUSBD_CONTROL_MSG 0x00040000
#define DBG_CUSBD_VERBOSE_CONTROL_MSG 0x00080000
#define DBG_CUSBD_BASIC_MSG 0x00100000
#define DBG_CUSBD_VERBOSE_MSG 0x00200000
#define DBG_CUSBD_ERR_MSG 0x00400000

#define EOK 0

#define WAIT_UNTIL_BIT_CLEARED(reg, bit) while(reg & bit)

//--------- set these depending on used platform --------------------
//#define CPU_16BIT
#define MASS_STORAGE_APP 0
//-------------------------------------------------------------------

#ifdef CPU_16BIT
#define UINT16_T uint16_t
#else
#define UINT16_T uint32_t
#endif

#if MASS_STORAGE_APP
extern volatile uint8_t msc_failed_flag;
#else
static uint8_t msc_failed_flag = 0;
#endif

//------------------------------------------------------------------------------
#define SS_DEV_NAME "CSS"
#define EP0_NAME "EP_0"
#define EP0_ADDRESS 0x80

#define CUSBSS_U1_DEV_EXIT_LAT   4
#define CUSBSS_U2_DEV_EXIT_LAT   512
#define USB_SS_MAX_PACKET_SIZE   1024

typedef enum {
    EP0_UNCONNECTED = 0,
    EP0_SETUP_PHASE,
    EP0_DATA_PHASE,
    EP0_STATUS_PHASE,
} ep0StateEnum;

typedef enum {
    EP_DISABLED = 0,
    EP_ENABLED,
    EP_STALLED
} epState;

typedef struct {
    struct CUSBD_ListHead reqList;
    struct CUSBD_Ep ep;
    void * channel;
    epState ep_state;
    uint8_t transferPending;
    struct CUSBD_Req * actualReq;
    uint8_t wedgeFlag;
} CUSBD_EpPrivate;

static CUSBD_EpPrivate * get_EpPrivateObj(struct CUSBD_Ep * ep) {
    return ((CUSBD_EpPrivate  *) ((uintptr_t) ep - sizeof (struct CUSBD_ListHead)));
}

/* Driver private data */
typedef struct {
    // this "class must implement" CUSBD class, this class is an extension of CUSBD
    CUSBD_Dev device;

    CUSBD_Config config; // copy of user configuration
    // USB protocol configuration passed with
    // SET_CONFIGURATION request, it keeps rather
    // hardware configuration

    CUSBD_Callbacks callbacks; // copy of user callback
    CH9_UsbSetup *setupPacket;
    uintptr_t setupPacketDma;

    // DMA section
    CUSBDMA_OBJ * dmaDrv;
    void * dmaController;
    CUSBDMA_Callbacks dmaCallback;
    void * ep0DmaChannelIn;
    void * ep0DmaChannelOut;

    // these variables reflect device state
    uint16_t status_value; // keeps status value
    uint8_t u2_value; // keeps U2 value
    uint8_t u1_value; // keeps U1 value
    uint16_t suspend; // keeps suspend value
    uint16_t isoch_delay;
    uint32_t remoteWakeup;

    ssReg_t  *reg;
    CUSBD_EpPrivate ep0; // bidirectional endpoint 0
    ep0StateEnum ep0NextState;
    uint8_t ep0DataDirFlag;
    struct CUSBD_Req *request;
    struct CUSBD_Req reqAlloc;
    CUSBD_EpPrivate ep_in_container[1];
    CUSBD_EpPrivate ep_out_container[1];
} CUSBD_PrivateData;

static CUSBD_PrivateData * get_DevPrivateObj(void * pD) {

    return (CUSBD_PrivateData*) pD;
}

static struct CUSBD_Req * listToRequest(struct CUSBD_ListHead *list) {
    return (struct CUSBD_Req *) list;
}

static uint32_t epEnable(void *pD, struct CUSBD_Ep *ep, const CH9_UsbEndpointDescriptor * desc);
static uint32_t epDisable(void *pD, struct CUSBD_Ep * ep);
static uint32_t reqQueue(void *pD, struct CUSBD_Ep *ep, struct CUSBD_Req * req);
static uint32_t reqDequeue(void *pD, struct CUSBD_Ep *ep, struct CUSBD_Req * req);
static uint32_t epSetHalt(void *pD, struct CUSBD_Ep *ep, uint8_t value);
static uint32_t epSetWedge(void *pD, struct CUSBD_Ep * ep);
static void epFifoFlush(void *pD, struct CUSBD_Ep * ep);
static uint32_t epFifoStatus(void *pD, struct CUSBD_Ep * ep);
static uint32_t reqAlloc(void *pD, struct CUSBD_Ep *ep, CUSBD_Req **req);
static void reqFree(void *pD, struct CUSBD_Ep *ep, struct CUSBD_Req * req);

static const struct CUSBD_epOps epOps = {
    epEnable,
    epDisable,
    epSetHalt,
    epSetWedge,
    epFifoStatus,
    epFifoFlush,
    reqQueue,
    reqDequeue,
    reqAlloc,
    reqFree
};

// char *strncpy(char *dst, const char *src, size_t n)
// {
//     if (n != 0) {
//         char *d = dst;
//         const char *s = src;
//         do {
//             if ((*d++ = *s++) == 0) {
//                  NUL pad the remaining n-1 bytes 
//                 while (--n != 0)
//                     *d++ = 0;
//                 break;
//             }
//         } while (--n != 0);
//     }
//     return (dst);
// }

static struct CUSBD_Req *getNexReq(CUSBD_EpPrivate * ep) {
    struct CUSBD_ListHead * list = &ep->reqList;

    //Check if list is empty
    if (list->next == list) {
        VERBOSE("Request list for endpoint %02X is empty\n ", ep->ep.address);
        return NULL;
    }

    return listToRequest(list->next);
}

static void startHwTransfer(CUSBD_PrivateData * dev, CUSBD_EpPrivate *epp, struct CUSBD_Req *req) {

    uint32_t bytesToTransfer = req->length - req->actual;
    uint32_t numOfbytes = bytesToTransfer > TD_SING_MAX_TRB_DATA_SIZE ? TD_SING_MAX_TRB_DATA_SIZE : bytesToTransfer;
    dev->dmaDrv->channelProgram(dev->dmaController, epp->channel,
            epp->ep.maxPacket, req->dma, numOfbytes, NULL, 0);
}

#if 0
static void displayData(uint8_t * s, int numOfData) {

    int i;

    VERBOSE("EP0 SEND: %c\n", ' ');
    for (i = 0; i < numOfData; i++)
        VERBOSE("%02X\n ", s[i]);
    VERBOSE("%c\n", ' ');
}

void displayReq(struct CUSBD_Req * req) {

    //struct CUSBD_ListHead list;
    //struct CUSBD_ListHead * list = &req->list;
    VERBOSE("-------------------------------------------%c\n", ' ');
    VERBOSE("this: %08X\n", /* void* */ (uint32_t) req);
    VERBOSE("next: %08X\n", /* void* */ (uint32_t) req->list->next);
    VERBOSE("prev: %08X\n", /* void* */ (uint32_t) req->list->prev);
    VERBOSE("buf: %08X\n", /* void* */ (uint32_t) req->buf);
    VERBOSE("length: %08X\n", /*uint32_t*/ req->length);
    VERBOSE("dma: %08X\n", /*uintptr_t*/ (uint32_t) req->dma);
    VERBOSE("sg: \n", /*struct CUSBD_SgList* */ (uint32_t) req->sg);
    VERBOSE("numOfSgs: %08X\n", /*uint32_t*/ req->numOfSgs);
    VERBOSE("numMappedSgs: %08X\n", /*uint32_t*/ req->numMappedSgs);
    VERBOSE("streamId: %d\n", /*uint16_t*/ req->streamId);
    VERBOSE("noInterrupt: %d\n", /*uint8_t*/ req->noInterrupt);
    VERBOSE("zero: %d\n", /*uint8_t*/ req->zero);
    VERBOSE("shortNotOk: %d\n", /*uint8_t*/ req->shortNotOk);
    VERBOSE("complete: %08X\n", /*CUSBD_reqComplete*/ (uint32_t) req->complete);
    VERBOSE("context: %08X\n", /*void*/ (uint32_t) req->context);
    VERBOSE("status: %d\n", /*uint32_t*/ req->status);
    VERBOSE("actual: %d\n", /*uint32_t*/ req->actual);
    VERBOSE("-------------------------------------------%c\n", ' ');
    /*
    if(list->next == list){
        VERBOSE("No more requests queued %c\n", ' ');
    }
    displayReq(listToRequest(list->next));
     */
    return;
}
#endif

static void ep0transfer(CUSBD_PrivateData * dev, uint8_t dir, uintptr_t buffer, uint32_t buffer_size, uint8_t erdy) {

    dev->dmaDrv->channelProgram(dev->dmaController, dir ? dev->ep0DmaChannelIn : dev->ep0DmaChannelOut,
            dev->ep0.ep.maxPacket, buffer, buffer_size < dev->ep0.ep.maxPacket ? buffer_size : dev->ep0.ep.maxPacket, NULL, 0);
    if (erdy){
        CPS_UncachedWrite32(&dev->reg->USBR_EP_CMD, USBRF_EP_ERDY); // OUT data phase
        // CPS_UncachedWrite32(&dev->reg->USBR_DRBL, 1); // markmarkmark
    }
}

// private function, initializes default endpoint, main/ISR context

static void initSingleEp0Hw(CUSBD_PrivateData * dev, uint8_t endp_addr, uint32_t maxPacketSize) {

    // select endpoint
    CPS_UncachedWrite32(&dev->reg->USBR_EP_SEL, endp_addr);

    // configure endpoint
    CPS_UncachedWrite32(&dev->reg->USBR_EP_CFG, USBRD_EP_ENABLED | USBRD_EP_CONTROL | USBRD_EP_MAXPKSIZE(maxPacketSize));

    // enable required interrupt
    CPS_UncachedWrite32(&dev->reg->USBR_EP_STS_EN, USBRF_EP_SETUPEN | USBRF_EP_DESCMISEN | USBRF_EP_TRBERREN | USBRF_EP_OUTSMMEN);

    // enable interrupts from ep0in and ep0out
    CPS_UncachedWrite32(&dev->reg->USBR_EP_IEN, 0x00010001);
}

static void initEp0(CUSBD_PrivateData * dev) {

    uint32_t maxPacketSize;

    switch (dev->device.speed) {
        case CH9_USB_SPEED_LOW: maxPacketSize = 8;
            break;
        case CH9_USB_SPEED_FULL: maxPacketSize = 64;
            break;
        case CH9_USB_SPEED_HIGH: maxPacketSize = 64;
            break;
        case CH9_USB_SPEED_SUPER: maxPacketSize = 512;
            break;
        default: maxPacketSize = 512; // ????????
    }

    initSingleEp0Hw(dev, 0x80, maxPacketSize); // configure ep0in
    initSingleEp0Hw(dev, 0x00, maxPacketSize); // configure ep0out
    dev->ep0.ep.maxPacket = maxPacketSize;
    dev->ep0NextState = EP0_SETUP_PHASE;
    dev->ep0.ep_state = EP_ENABLED;
    INFO("EP0 maxPacket: %d\n", dev->ep0.ep.maxPacket);
}

static void initDevHw(CUSBD_PrivateData * dev) {

    uint32_t dmult;

    if (dev->config.dmultEnabled) dmult = USBRF_DMULT;
    else dmult = 0;

    // configure device
    CPS_UncachedWrite32(&dev->reg->USBR_CONF, /*USBRF_U1EN | USBRF_U2EN |*/ USBRF_L1EN | dmult);

    // clear all interrupts
    CPS_UncachedWrite32(&dev->reg->USBR_ISTS, 0xFFFFFFFF);
    CPS_UncachedWrite32(&dev->reg->USBR_EP_STS, 0xFFFFFFFF);

    // enable interrupts
    CPS_UncachedWrite32(&dev->reg->USBR_IEN, USBRF_UWRESIEN |
            USBRF_UHRESIEN |
            USBRF_DISIEN |
            USBRF_CONIEN |
            USBRF_CON2IEN |
            USBRF_U1ENTIEN |
            USBRF_U2ENTIEN |
            USBRF_UHSFSRESIEN |
            USBRF_L2ENTIEN |
            USBRF_L2EXTIEN |
            USBRF_DIS2IEN);
}

static uint32_t initSwEp(CUSBD_PrivateData * dev, CUSBD_EpConfig *ep, uint8_t dir) {
    int i;
    CUSBD_EpPrivate * epPriv;
    for (i = 0; i < 1; i++, ep++) {
        if (ep->bufferingValue > 0) {
            // char nameBuff[10];
            if (ep->bufferingValue > 16)return EINVAL;
            // add endpoint to endpoint list
            if (dir) epPriv = &dev->ep_in_container[i];
            else epPriv = &dev->ep_out_container[i];

            if (epPriv == NULL)return EINVAL;
            // clear endpoint object
            memset(epPriv, 0, sizeof (CUSBD_EpPrivate));
            // set endpoint address
            epPriv->ep.address = (dir ? CH9_USB_EP_DIR_IN : 0) | (uint8_t) (i + 1);
            // set MaxPacketSize
            epPriv->ep.maxPacket = 1024;
            // set Max Burst
            epPriv->ep.maxburst = 16;
            //set max stream
            epPriv->ep.maxStreams = 32;
            //set MULT
            epPriv->ep.mult = 2;
            // set endpoint name
            // snprintf(nameBuff, sizeof (nameBuff), "EP_%d_%s", i + 1, dir ? "IN" : "OUT");
            // strncpy(epPriv->ep.name, nameBuff, 10);
            // set endpoint pseudo-class public methods
            epPriv->ep.ops = &epOps;
            //add endpoint to endpoint list
            listAddTail(&epPriv->ep.epList, &dev->device.epList);
            // initialize request list
            listInit(&epPriv->reqList);
        }
    }
    return EOK;
}

static void getStatusCmpl(struct CUSBD_Ep *ep, struct CUSBD_Req * req){
}

static uint32_t getStatus(CUSBD_PrivateData * dev, CH9_UsbSetup *ctrl) {

    uint16_t size = ctrl->wLength;
    uint8_t recip = (ctrl->bmRequestType & CH9_REQ_RECIPIENT_MASK);
    uint32_t res;

    switch (recip) {
        case CH9_USB_REQ_RECIPIENT_DEVICE:
#if defined(TEST_RWP)
            dev->status_value = cpuToLe16(dev->u2_value | dev->u1_value | 3);
#else
            dev->status_value = cpuToLe16(dev->u2_value | dev->u1_value | 1);
#endif
            break;

        case CH9_USB_REQ_RECIPIENT_INTERFACE:
            /*
             * Function Remote Wake Capable	D0
             * Function Remote Wakeup	D1
             */
            //dev->status_value = cpu_to_le16(0);
            ctrl->wIndex = cpuToLe16(ctrl->wIndex);
            ctrl->wLength = cpuToLe16(ctrl->wLength);
            ctrl->wValue = cpuToLe16(ctrl->wValue);
            res = dev->callbacks.setup(&dev->device, ctrl);
            //if (res != 0)
            VERBOSE ("getStatus res: %d\n", res);
            return res;

        case CH9_USB_REQ_RECIPIENT_ENDPOINT:
            CPS_UncachedWrite32(&dev->reg->USBR_EP_SEL, ctrl->wIndex);
            if (CPS_UncachedRead32(&dev->reg->USBR_EP_STS) & USBRF_EP_STALL)
                dev->status_value = cpuToLe16(1);
            else dev->status_value = cpuToLe16(0);
            break;
        default:
            return EINVAL;
    }
    memcpy((void*) dev->setupPacket, &dev->status_value, (size_t) size);
    dev->reqAlloc.length = size;
    dev->reqAlloc.buf = (void*)dev->setupPacketDma;
    dev->reqAlloc.dma = dev->setupPacketDma;
    dev->reqAlloc.complete = getStatusCmpl;
    reqQueue(dev, &dev->ep0.ep, &dev->reqAlloc);
    return EOK;
}

static uint32_t handleFeatureRecipientDevice(CUSBD_PrivateData * dev, CH9_UsbSetup *ctrl, uint8_t set)
{
    CH9_UsbState state;
    uint32_t wIndex;
    uint32_t reg;
    uint32_t wValue;

    wValue = ctrl->wValue;
    wIndex = ctrl->wIndex;
    state = dev->device.state;

    switch (wValue) {
        case CH9_USB_FS_DEVICE_REMOTE_WAKEUP:
            break;
            /*
             * 9.4.1 says only only for SS, in AddressState only for
             * default control pipe
             */
        case CH9_USB_FS_U1_ENABLE:
            if (state != CH9_USB_STATE_CONFIGURED)
                return EINVAL;
            if (dev->device.speed != CH9_USB_SPEED_SUPER)
                return EINVAL;

            reg = CPS_UncachedRead32(&dev->reg->USBR_CONF);
            if (set) {
                dev->u1_value = 0x04;
                reg |= USBRF_U1EN;
            } else {
                dev->u1_value = 0x00;
                reg |= USBRF_U1DS;
            }

            CPS_UncachedWrite32(&dev->reg->USBR_CONF, reg);
            break;

        case CH9_USB_FS_U2_ENABLE:
            if (state != CH9_USB_STATE_CONFIGURED)
                return EINVAL;
            if (dev->device.speed != CH9_USB_SPEED_SUPER)
                return EINVAL;

            reg = CPS_UncachedRead32(&dev->reg->USBR_CONF);
            if (set) {
                dev->u2_value = 0x08;
                reg |= USBRF_U2EN;
            } else {
                dev->u2_value = 0x00;
                reg |= USBRF_U2DS;
            }

            CPS_UncachedWrite32(&dev->reg->USBR_CONF, reg);
            break;

        case CH9_USB_FS_LTM_ENABLE:
            return EINVAL;
            break;

        case CH9_USB_FS_TEST_MODE:
            if ((wIndex & 0xff) != 0)
                return EINVAL;
            if (!set)
                return EINVAL;
        {
            uint8_t test_selector = (uint8_t) ((ctrl->wIndex >> 8)& 0x00FF);
            switch (test_selector) {
                case CH9_TEST_J: CPS_UncachedWrite32(&dev->reg->USBR_CMD, USBRD_SET_TESTMODE(USBRV_TM_TEST_J));
                    break;
                case CH9_TEST_K: CPS_UncachedWrite32(&dev->reg->USBR_CMD, USBRD_SET_TESTMODE(USBRV_TM_TEST_K));
                    break;
                case CH9_TEST_SE0_NAK: CPS_UncachedWrite32(&dev->reg->USBR_CMD, USBRD_SET_TESTMODE(USBRV_TM_SE0_NAK));
                    break;
                case CH9_TEST_PACKET: CPS_UncachedWrite32(&dev->reg->USBR_CMD, USBRD_SET_TESTMODE(USBRV_TM_TEST_PACKET));
                    break;
                case CH9_TEST_FORCE_EN: break;
                default: break;
            }
        }
            break;
        default:
            return EINVAL;
    }
    return EOK;
}

static uint32_t handleFeatureRecipientInterface(CUSBD_PrivateData * dev, CH9_UsbSetup *ctrl, uint8_t set)
{
    // CH9_UsbState state;
    uint32_t wIndex;
    //uint32_t reg;
    uint32_t wValue;

    wValue = ctrl->wValue;
    wIndex = ctrl->wIndex;
    //state = dev->device.state;

    switch (wValue) {
        case CH9_USB_FS_FUNCTION_SUSPEND:
            if (wIndex & CH9_USB_SF_LOW_POWER_SUSPEND_STATE) {
                /* XXX enable Low power suspend */
                dev->suspend = (ctrl->wIndex & 0x0100);
            }

            if (wIndex & CH9_USB_SF_REMOTE_WAKE_ENABLED) {
                /* XXX enable remote wakeup */
                dev->remoteWakeup = set;
            }
            break;
        default:
            return EINVAL;
    }
    return EOK;
}

static uint32_t handleFeatureRecipientEndpoint(CUSBD_PrivateData * dev, CH9_UsbSetup *ctrl, uint8_t set)
{
    //CH9_UsbState state;
    //uint32_t wIndex;
    //uint32_t reg;
    uint32_t wValue;
    struct CUSBD_ListHead *list;
    struct CUSBD_Ep * ep;
    CUSBD_EpPrivate * epp;
    struct CUSBD_Req * req;
    uint32_t tempSel;

    wValue = ctrl->wValue;
    //wIndex = ctrl->wIndex;
    //state = dev->device.state;

    for (list = dev->device.epList.next; list != &dev->device.epList; list = list->next) {
        ep = (struct CUSBD_Ep *) list;
        if (ep->address == (uint8_t) ctrl->wIndex) {
            epp = get_EpPrivateObj(ep);
            goto ep_found;
        }
    }
    return EINVAL;

ep_found:
    tempSel = CPS_UncachedRead32(&dev->reg->USBR_EP_SEL);
    CPS_UncachedWrite32(&dev->reg->USBR_EP_SEL, (uint32_t) ep->address);

    switch (wValue) {
        case CH9_USB_FS_ENDPOINT_HALT:
            if (set) {
                CPS_UncachedWrite32(&dev->reg->USBR_EP_CMD, USBRF_EP_SSTALL);
                epp->ep_state = EP_STALLED;
            } else {
                if (/*!epp->wedgeFlag*/msc_failed_flag == 0) {
                    epp->wedgeFlag = 0;
                    CPS_UncachedWrite32(&dev->reg->USBR_EP_CMD, USBRF_EP_EPRST | USBRF_EP_CSTALL);
                    WAIT_UNTIL_BIT_CLEARED(CPS_UncachedRead32(&dev->reg->USBR_EP_CMD), USBRF_EP_EPRST);
                    epp->ep_state = EP_ENABLED;

                    req = getNexReq(epp);
                    if (req) {
                        epp->actualReq = req;
                        epp->transferPending = 1;
                        startHwTransfer(dev, epp, req);
                    }
                } else {
                    CPS_UncachedWrite32(&dev->reg->USBR_EP_CMD, USBRF_EP_EPRST);
                    WAIT_UNTIL_BIT_CLEARED(CPS_UncachedRead32(&dev->reg->USBR_EP_CMD), USBRF_EP_EPRST);
                    // vDbgMsg(DBG_CUSBD_BASIC_MSG, 1, "CAN'T CLEAR ENDPOINT STALL %c\n", ' ');
                }
            }//else (set)
            break;

        default:
            CPS_UncachedWrite32(&dev->reg->USBR_EP_SEL, tempSel);
            return EINVAL;
    }
    CPS_UncachedWrite32(&dev->reg->USBR_EP_SEL, tempSel);
    return EOK;
}

static uint32_t handleFeature(CUSBD_PrivateData * dev, CH9_UsbSetup *ctrl, uint8_t set) {

    //uint32_t wIndex;
    //uint32_t wValue;
    //uint32_t reg;
    uint32_t stat;

    // CH9_UsbState state;

    //wValue = ctrl->wValue;
    //wIndex = ctrl->wIndex;
    //state = dev->device.state;

    switch (ctrl->bmRequestType & CH9_REQ_RECIPIENT_MASK) {
        case CH9_USB_REQ_RECIPIENT_DEVICE:

            stat = handleFeatureRecipientDevice(dev, ctrl, set);
            if (stat != EOK)
                return stat;
            break;

        case CH9_USB_REQ_RECIPIENT_INTERFACE:
            stat = handleFeatureRecipientInterface(dev, ctrl, set);
            if (stat != EOK)
                return stat;
            break;

        case CH9_USB_REQ_RECIPIENT_ENDPOINT:
            stat = handleFeatureRecipientEndpoint(dev, ctrl, set);
            if (stat != EOK)
                return stat;
            break;
        default:
            return EINVAL;
    };

    return EOK;
}

static uint32_t setAddress(CUSBD_PrivateData * dev, CH9_UsbSetup *ctrl) {

    CH9_UsbState state = dev->device.state;
    uint16_t addr;


    addr = ctrl->wValue;

    if (addr > 127) {
        ERROR("Invalid device address %d\n", addr);
        return EINVAL;
    }

    if (state == CH9_USB_STATE_CONFIGURED) {
        NOTICE("Trying to set address when configured %c\n", ' ');
        return EINVAL;
    }

    CPS_UncachedWrite32(&dev->reg->USBR_CMD, ((ctrl->wValue & 0x007F) << 1) | 1);

    if (addr)
        dev->device.state = CH9_USB_STATE_ADDRESS;
    else
        dev->device.state = CH9_USB_STATE_DEFAULT;

    return EOK;
}

static uint32_t setConfig(CUSBD_PrivateData * dev, CH9_UsbSetup *ctrl) {

    uint8_t confValue = (uint8_t) (ctrl->wValue & 0x00FF);
    uint32_t res;

    switch (dev->device.state) {
        case CH9_USB_STATE_DEFAULT:
            return EINVAL;
            break;

        case CH9_USB_STATE_ADDRESS:
            ctrl->wIndex = cpuToLe16(ctrl->wIndex);
            ctrl->wLength = cpuToLe16(ctrl->wLength);
            ctrl->wValue = cpuToLe16(ctrl->wValue);
            if (confValue)CPS_UncachedWrite32(&dev->reg->USBR_CONF, USBRF_CFGSET);
            res = dev->callbacks.setup(&dev->device, ctrl);
            if (confValue && !res) {
                //CPS_UncachedWrite32(&dev->reg->USBR_CONF, USBRF_CFGSET);
                // check if USB controller has resources for required configuration
                // and return error if no
                if (CPS_UncachedRead32(&dev->reg->USBR_STS) & USBRF_MEM_OV) return EINVAL;
                //dev->actualConfigValue = confValue;
                //dev->device.state = CH9_USB_STATE_CONFIGURED;
            }
            break;

        case CH9_USB_STATE_CONFIGURED:
            ctrl->wIndex = cpuToLe16(ctrl->wIndex);
            ctrl->wLength = cpuToLe16(ctrl->wLength);
            ctrl->wValue = cpuToLe16(ctrl->wValue);
            res = dev->callbacks.setup(&dev->device, ctrl);
            if (res != 0) return res;

            // SET_CONFIGURATION(0)
            if (!confValue) {
                CPS_UncachedWrite32(&dev->reg->USBR_CONF, USBRF_CFGRST);
                dev->device.state = CH9_USB_STATE_ADDRESS;
                //delete all queued requests
                //dev->actualConfigValue = 0;
            } else {
                // SET_CONFIGURATION(2) - switching to other configuration
                //if (confValue != dev->actualConfigValue) {
                CPS_UncachedWrite32(&dev->reg->USBR_CONF, USBRF_CFGSET);
                if (CPS_UncachedRead32(&dev->reg->USBR_STS) & USBRF_MEM_OV) return EINVAL;
                //dev->actualConfigValue = confValue;
            }
            break;

        default:
            res = EINVAL;
    }

    return res;
}

static uint32_t setSel(CUSBD_PrivateData * dev, CH9_UsbSetup *ctrl) {

    if (dev->device.state == CH9_USB_STATE_DEFAULT)
        return EINVAL;

    if (ctrl->wLength != 6) {
        ERROR("Set SEL should be 6 bytes, got %d\n", ctrl->wLength);
        return EINVAL;
    }
    ep0transfer(dev, 0, (uintptr_t) dev->setupPacketDma, ctrl->wLength, 1);
    return EOK;
}

static uint32_t setIsochDelay(CUSBD_PrivateData * dev, CH9_UsbSetup *ctrl) {

    if (ctrl->wIndex || ctrl->wLength)
        return EINVAL;

    dev->isoch_delay = ctrl->wValue;
    return EOK;
}

static uint32_t setup(CUSBD_PrivateData * dev, CH9_UsbSetup *ctrl) {

    uint32_t res = 0;
    uint16_t size = ctrl->wLength;

    if (size > 0) dev->ep0NextState = EP0_DATA_PHASE;
    else dev->ep0NextState = EP0_STATUS_PHASE;

    if (ctrl->bmRequestType & CH9_USB_DIR_DEVICE_TO_HOST) dev->ep0DataDirFlag = 1;
    else dev->ep0DataDirFlag = 0;

    // check if request is a standard request
    if ((ctrl->bmRequestType & CH9_USB_REQ_TYPE_MASK) == CH9_USB_REQ_TYPE_STANDARD) {
        switch (ctrl->bRequest) {
            case CH9_USB_REQ_GET_STATUS:
                res = getStatus(dev, ctrl);
                break;

            case CH9_USB_REQ_CLEAR_FEATURE:
                res = handleFeature(dev, ctrl, 0);
                break;

            case CH9_USB_REQ_SET_FEATURE:
                res = handleFeature(dev, ctrl, 1);
                break;

            case CH9_USB_REQ_SET_ADDRESS:
                res = setAddress(dev, ctrl);
                break;

            case CH9_USB_REQ_SET_CONFIGURATION:
                res = setConfig(dev, ctrl);
                break;

            case CH9_USB_REQ_SET_SEL:
                res = setSel(dev, ctrl);
                break;

            case CH9_USB_REQ_ISOCH_DELAY:
                res = setIsochDelay(dev, ctrl);
                break;

            default:
                ctrl->wIndex = cpuToLe16(ctrl->wIndex);
                ctrl->wLength = cpuToLe16(ctrl->wLength);
                ctrl->wValue = cpuToLe16(ctrl->wValue);
                VERBOSE("res: %08X\n", res);
                res = dev->callbacks.setup(&dev->device, ctrl);
                break;
        }
    } else {
        // delegate setup for class and vendor specific requests
        ctrl->wIndex = cpuToLe16(ctrl->wIndex);
        ctrl->wLength = cpuToLe16(ctrl->wLength);
        ctrl->wValue = cpuToLe16(ctrl->wValue);
        res = dev->callbacks.setup(&dev->device, ctrl);
    }

    return res;
}

static void handleEp0Irq(CUSBD_PrivateData* dev, uint8_t dirFlag) {

    uint32_t ep_sts;
    uint8_t dir = dirFlag ? 0x80 : 0x00;

    CPS_UncachedWrite32(&dev->reg->USBR_EP_SEL, dir);
    ep_sts = CPS_UncachedRead32(&dev->reg->USBR_EP_STS);
    VERBOSE("ep_sts: %02X\n", ep_sts);

    if (ep_sts & USBRF_EP_TRBERR) {
        CPS_UncachedWrite32(&dev->reg->USBR_EP_STS, USBRF_EP_TRBERR);
    }

    if (ep_sts & USBRF_EP_OUTSMM) {
        CPS_UncachedWrite32(&dev->reg->USBR_EP_STS, USBRF_EP_OUTSMM);
    }

    // Setup packet completion
    if (ep_sts & USBRF_EP_SETUP) {
        uint32_t res=0;
        int i;
        CH9_UsbSetup ctrl;
        // uint8_t *s = (uint8_t *) dev->setupPacket;
        dev->request = NULL;

        VERBOSE("SETUP: %c\n", ' ');
        for (i = 0; i < 8; i++)
            VERBOSE("%X", s[i]);
        VERBOSE("%c\n", ' ');

        ctrl.bRequest = dev->setupPacket->bRequest;
        ctrl.bmRequestType = dev->setupPacket->bmRequestType;
        ctrl.wIndex = le16ToCpu(dev->setupPacket->wIndex);
        ctrl.wLength = le16ToCpu(dev->setupPacket->wLength);
        ctrl.wValue = le16ToCpu(dev->setupPacket->wValue);


        dev->dmaDrv->isr(dev->dmaController);
        CPS_UncachedWrite32(&dev->reg->USBR_EP_SEL, 0x00);
        CPS_UncachedWrite32(&dev->reg->USBR_EP_STS, USBRF_EP_SETUP | USBRF_EP_ISP | USBRF_EP_IOC);
        ep_sts &= ~(USBRF_EP_SETUP | USBRF_EP_ISP | USBRF_EP_IOC);
        res = setup(dev, &ctrl);
        //VERBOSE("res: %08X\n", res);
        CPS_UncachedWrite32(&dev->reg->USBR_EP_SEL, 0x00);

        // -------- Patch for Linux SET_CONFIGURATION(1) handling --------------
        if (res == 0x7FFF) {
            INFO("Respond delayed not finished yet%s\n", " ");
            dev->ep0NextState = EP0_STATUS_PHASE;
            return;
        }
        //----------------------------------------------------------------------

        if (res != 0) {
            CPS_UncachedWrite32(&dev->reg->USBR_EP_CMD, USBRF_EP_SSTALL);
            CPS_UncachedWrite32(&dev->reg->USBR_EP_CMD, USBRF_EP_REQ_CMPL | USBRF_EP_ERDY);
            // CPS_UncachedWrite32(&dev->reg->USBR_DRBL, 1); // markmarkmark
            dev->ep0NextState = EP0_SETUP_PHASE;
        } else {
            if (dev->ep0NextState == EP0_STATUS_PHASE) {
                // set status stage OK
                CPS_UncachedWrite32(&dev->reg->USBR_EP_CMD, USBRF_EP_REQ_CMPL | USBRF_EP_ERDY); // status phase
                // CPS_UncachedWrite32(&dev->reg->USBR_DRBL, 1); // markmarkmark
                dev->ep0NextState = EP0_SETUP_PHASE;
                // check if any ep0 request waits for completion, If YES complete it
                if (dev->request) {
                    if ((dev->request->length == 0) && (dev->request->status == EINPROGRESS)) {
                        VERBOSE("Call callback%s\n", " ");
                        //displayReq(dev->request);
                        dev->request->status = 0;
                        if(dev->request->complete)
                            dev->request->complete(&dev->ep0.ep, dev->request);
                    }
                }
            }
        }

        return;
    }

    if (ep_sts & USBRF_EP_DESCMIS) {
        CPS_UncachedWrite32(&dev->reg->USBR_EP_STS, USBRF_EP_DESCMIS);
        VERBOSE("EP_DESCMIS!!!, next state = %d\n", dev->ep0NextState);
        // check if setup packet came in
        // setup always come on OUT direction
        if (dir == 0 && dev->ep0NextState == EP0_SETUP_PHASE) {
            ep0transfer(dev, 0, (uintptr_t) dev->setupPacketDma, sizeof (CH9_UsbSetup), 0);
        }
        return;
    }

    // should be called only when data phase exists
    if ((ep_sts & USBRF_EP_IOC) || (ep_sts & USBRF_EP_ISP)) {

        dev->dmaDrv->isr(dev->dmaController);
        dev->request->actual += dev->dmaDrv->getActualLength(dev->dmaController, dirFlag ? dev->ep0DmaChannelIn : dev->ep0DmaChannelOut);

        VERBOSE("Transfer complete: data length: %d\n", dev->request->actual);

        if (dev->request->actual >= dev->request->length) {
            VERBOSE("No more data to transfer on ep0%c\n", ' ');
            dev->request->status = 0;
            //displayReq(dev->request);
            dev->request->complete(&dev->ep0.ep, dev->request);
            if (dev->request->status == EINPROGRESS) {
                // we have new request, stay in data phase
                return;
            }
            dev->ep0NextState = EP0_SETUP_PHASE;
            CPS_UncachedWrite32(&dev->reg->USBR_EP_SEL, 0x00);
            CPS_UncachedWrite32(&dev->reg->USBR_EP_CMD, USBRF_EP_REQ_CMPL | USBRF_EP_ERDY);
            // CPS_UncachedWrite32(&dev->reg->USBR_DRBL, 1); // markmarkmark
        } else {
            uint16_t bytesToTransfer = dev->request->length - dev->request->actual;
            uint16_t numOfbytes = bytesToTransfer < dev->ep0.ep.maxPacket ? bytesToTransfer : dev->ep0.ep.maxPacket;

            dev->request->dma += dev->ep0.ep.maxPacket;
            // For OUT direction ERDY in this place may by redundant - it is
            // strictly related to OUT endpoint buffering value but it is safer to
            // send ERDY
            //displayData(dev->request->buf, numOfbytes);
            ep0transfer(dev, dirFlag, dev->request->dma, numOfbytes, dirFlag ? 1 : 0);
            return;
        }
        return;
    }
}

static void dmaCallback(void * pD, uint8_t epNum, uint8_t epDir) {

    struct CUSBD_ListHead *list;
    CUSBD_PrivateData* dev;
    struct CUSBD_Ep * ep;
    CUSBD_EpPrivate * epp;
    struct CUSBD_Req * req;
    uint32_t temp_ep_sel;

    if (epNum == 0) return;

    dev = get_DevPrivateObj(pD);
    for (list = dev->device.epList.next; list != &dev->device.epList; list = list->next) {
        ep = (struct CUSBD_Ep *) list;
        if (ep->address == (epNum | epDir)) goto ep_found;
    }
    return;

ep_found:
    epp = get_EpPrivateObj(ep);
    // check if no empty interrupt, exit if YES
    if (!epp->transferPending) {
        INFO("Empty IOC on %02X endpoint\n", ep->address);
        return;
    }

    req = epp->actualReq;
    req->actual += dev->dmaDrv->getActualLength(dev->dmaController, epp->channel);

    // check if all data transfered for actual request
    if (req->actual < req->length) {
        // check if last packet is short packet, if Yes go to the next request
        if ((req->actual % ep->maxPacket) == 0) {
            req->dma += TD_SING_MAX_TRB_DATA_SIZE;
            startHwTransfer(dev, epp, req);
            return;
        }
    }
    req->status = EOK;

    if (!req->noInterrupt) listDelete(&req->list);

    // handle deferred STALL
    if (epp->ep_state == EP_STALLED) {
        CPS_UncachedWrite32(&dev->reg->USBR_EP_CMD, USBRF_EP_SSTALL | USBRF_EP_ERDY | USBRF_EP_DFLUSH);
        // CPS_UncachedWrite32(&dev->reg->USBR_DRBL, 1); // markmarkmark
        WAIT_UNTIL_BIT_CLEARED(CPS_UncachedRead32(&dev->reg->USBR_EP_CMD), USBRF_EP_DFLUSH);
        epp->transferPending = 0;
    }

    temp_ep_sel = CPS_UncachedRead32(&dev->reg->USBR_EP_SEL);
    req->complete(ep, req);
    CPS_UncachedWrite32(&dev->reg->USBR_EP_SEL, temp_ep_sel);
    if (req->noInterrupt) return;
    if (epp->ep_state == EP_STALLED) return;

    req = getNexReq(epp);
    if (req) {
        epp->actualReq = req;
        startHwTransfer(dev, epp, req);
        return;
    }
    epp->transferPending = 0;
}
//------------------------------------------------------------------------------

static uint32_t probe(CUSBD_Config* config, CUSBD_SysReq * sysReq)  {

    CUSBDMA_OBJ * dmaDrv = NULL;
    CUSBDMA_Config dmaConfig;
    CUSBDMA_SysReq dmaSysReq;
    uint32_t res; // keeps result of operation on driver

    if ((config == 0) || (sysReq == 0))
        return EINVAL;

    if (config->dmultEnabled > 2) {
        return EINVAL;
    }

    if(!config->regBase) {
      return EINVAL;
    }

    if (config->dmaInterfaceWidth != CUSBD_DMA_32_WIDTH && config->dmaInterfaceWidth != CUSBD_DMA_64_WIDTH) {
        return EINVAL;
    }

    // check DMA controller memory consumption
    dmaDrv = CUSBDMA_GetInstance();
    res = dmaDrv->probe(&dmaConfig, &dmaSysReq);
    if (res) return res;

    // Driver memory consumption is equal to sum of driver taken memory + DMA
    // memory
    sysReq->privDataSize = sizeof (CUSBD_PrivateData) + dmaSysReq.privDataSize;
    sysReq->trbMemSize = dmaSysReq.trbMemSize + 8; // 8 is a length of setup packet
    // VERBOSE ("sysReq->privDataSize %d\n", sysReq->privDataSize);
    // VERBOSE ("sizeof (CUSBD_PrivateData) %lu\n", sizeof (CUSBD_PrivateData));
    // VERBOSE ("dmaSysReq.privDataSize %d\n", dmaSysReq.privDataSize);
    // VERBOSE ("sysReq->trbMemSize %d\n", sysReq->trbMemSize);


    return EOK;
}

static uint32_t init(void *pD, const CUSBD_Config* config, CUSBD_Callbacks * callbacks)  {

    uint32_t res;
    CUSBD_EpConfig *ep;
    CUSBD_PrivateData * dev;
    CUSBDMA_SysReq dmaSysReq;
    CUSBDMA_Config dmaConfig;
    int i;

    if (pD == NULL || config == NULL || callbacks == NULL)
        return EINVAL;

    if (config->dmaInterfaceWidth == 0) {
        return EINVAL;
    }

    dev = get_DevPrivateObj(pD);
    //
    memset(dev, 0, sizeof (CUSBD_PrivateData));

    memcpy(&dev->callbacks, callbacks, sizeof (CUSBD_Callbacks));
    memcpy(&dev->config, config, sizeof (CUSBD_Config));

    dev->reg = (ssReg_t*) dev->config.regBase;
    VERBOSE("regBase %08X\n", dev->config.regBase);

#ifdef CPU_BIG_ENDIAN
    CPS_UncachedWrite32(&dev->reg->USBR_CONF, cpu_to_le32(USBRF_BENDIAN));
#endif

#if NEW_DMA==1
    VERBOSE("use new DMA\n");
    CPS_UncachedWrite32((volatile uint32_t *)(dev->config.regBase+USBR_NEWDMA), USBRF_NEWDMA_EN);
#endif

    // configure IN endpoints
    for (i = 0, ep = dev->config.epIN; i < 1; i++, ep++) {
        if (ep->bufferingValue > 0) {
            if (ep->bufferingValue > 16) return EINVAL;
            CPS_UncachedWrite32(&dev->reg->USBR_EP_SEL, (uint32_t) (0x80 | (i + 1)));
            CPS_UncachedWrite32(&dev->reg->USBR_EP_CFG,
                    USBRD_EP_ENABLED |
                    USBRD_EP_MAXPKSIZE(USB_SS_MAX_PACKET_SIZE) |
                    USBRD_EP_BUFFERING(ep->bufferingValue)
                    );
        }
    }

    // configure OUT endpoints
    for (i = 0, ep = dev->config.epOUT; i < 1; i++, ep++) {
        if (ep->bufferingValue > 0) {
            if (ep->bufferingValue > 16) return EINVAL;
            CPS_UncachedWrite32(&dev->reg->USBR_EP_SEL, (uint32_t) (0x00 | (i + 1)));
            CPS_UncachedWrite32(&dev->reg->USBR_EP_CFG,
                    USBRD_EP_ENABLED |
                    USBRD_EP_MAXPKSIZE(USB_SS_MAX_PACKET_SIZE) |
                    USBRD_EP_BUFFERING(ep->bufferingValue)
                    );
        }
    }

    // check if hardware has enough resources for required configuration
    CPS_UncachedWrite32(&dev->reg->USBR_CONF, USBRF_CFGSET);
    // and return error if resources problem
    if (CPS_UncachedRead32(&dev->reg->USBR_STS) & USBRF_MEM_OV) return ENOTSUP;

    // if resources available, unconfigure device by now. It will be configured
    // in SET_CONFIG context during enumeration
    CPS_UncachedWrite32(&dev->reg->USBR_CONF, USBRF_CFGRST);

    // set device general device state to initial state
    dev->device.speed = CH9_USB_SPEED_UNKNOWN;
    dev->device.maxSpeed = CH9_USB_SPEED_SUPER;
    dev->device.state = CH9_USB_STATE_DEFAULT; // <----- check if we set status, probably higher layer sets it
    dev->device.aHnpSupport = 0;
    dev->device.bHnpEnable = 0;
    // strncpy(dev->device.name, SS_DEV_NAME, sizeof (dev->device.name));
    dev->device.isOtg = 0;
    dev->device.isAPeripheral = 0;

    if (dev->config.dmultEnabled) {
        dev->device.sgSupported = 1;
    } else {
        dev->device.sgSupported = 0;
    }

    //--------------------------------------------------------------------------
    /*Creating DMA Controller*/
    dev->dmaDrv = CUSBDMA_GetInstance();
    dev->dmaController = (void*) ((uintptr_t) pD + sizeof (CUSBD_PrivateData));

    dmaConfig.regBase = dev->config.regBase;
    dmaConfig.trbAddr = config->trbAddr + 8;
    dmaConfig.trbDmaAddr = (uintptr_t) (config->trbDmaAddr + 8);

    res = dev->dmaDrv->probe(&dmaConfig, &dmaSysReq);
    if (res) return res;

    if (config->dmultEnabled) {
        dmaConfig.dmaModeRx = 0xFFFF;
        dmaConfig.dmaModeTx = 0xFFFF;
    } else {
        dmaConfig.dmaModeRx = 0x0000;
        dmaConfig.dmaModeTx = 0x0000;
    }

    dev->dmaCallback.complete = dmaCallback;

    res = dev->dmaDrv->init(dev->dmaController, &dmaConfig, &dev->dmaCallback);
    if (res) return res;

    dev->dmaDrv->setParentPriv(dev->dmaController, pD);
    //--------------------------------------------------------------------------

    // initialize endpoint 0 - software
    dev->ep0.ep.address = EP0_ADDRESS;
    dev->ep0.ep.ops = &epOps;
    // strncpy(dev->ep0.ep.name, EP0_NAME, sizeof (dev->ep0.ep.name));
    dev->ep0DmaChannelIn = dev->dmaDrv->channelAlloc(dev->dmaController, 1, 0, 0);
    dev->ep0DmaChannelOut = dev->dmaDrv->channelAlloc(dev->dmaController, 0, 0, 0);
    dev->device.ep0 = &dev->ep0.ep;

    // initialize endpoints given in configuration
    listInit(&dev->device.epList);

    ep = dev->config.epIN;
    res = initSwEp(dev, ep, 1);
    // VERBOSE("initSwEp in %d\n", res);
    if (res != 0)return res;

    ep = dev->config.epOUT;
    res = initSwEp(dev, ep, 0);
    // VERBOSE("initSwEp out %d\n", res);
    if (res != 0)return res;

    // useful buffers memory alignment
    dev->setupPacketDma = config->trbDmaAddr;
    dev->setupPacket = (CH9_UsbSetup *) config->trbAddr;

    // reset all internal state variable
    dev->status_value = dev->suspend = dev->u1_value = dev->u2_value = 0;

    //--------------------------
    // hardware initializing
    //--------------------------

    // Configure Device hardware
    initDevHw(dev);

    // Initialize Ep0 hardware
    initEp0(dev);
    //--------------------------

    return EOK;
}

static void destroy(void *pD)  {

    //CUSBD_PrivateData* dev;

    VERBOSE ("destroy\n");
    if (pD == NULL)
        return;

    //dev = get_DevPrivateObj(pD);
    //dev->dmaDrv->destroy(dev->dmaController);
}

static void start(void *pD)  {

    CUSBD_PrivateData* dev;

    if (pD == NULL)
        return;

    dev = get_DevPrivateObj(pD);


    dev->dmaDrv->start(dev->dmaController);
    // connect device to host
    CPS_UncachedWrite32(&dev->reg->USBR_CONF, USBRF_DEVEN);
}

static void stop(void *pD)  {

    CUSBD_PrivateData* dev;

    if (pD == NULL)
        return;

    dev = get_DevPrivateObj(pD);
    dev->dmaDrv->stop(dev->dmaController);

    // disconnect device from host
    CPS_UncachedWrite32(&dev->reg->USBR_CONF, USBRF_DEVDS);
}

static void isr(void *pD)  {

    uint32_t usb_ists, ep_ists;
    struct CUSBD_ListHead *list;
    CUSBD_PrivateData* dev;
    struct CUSBD_Ep * ep;
    CUSBD_EpPrivate * epp;
    struct CUSBD_Req * req;

    if (pD == NULL)
        return;

    dev = get_DevPrivateObj(pD);
    usb_ists = CPS_UncachedRead32(&dev->reg->USBR_ISTS);
    if (usb_ists) {

        VERBOSE("USB_ISTS: %x\n", usb_ists);
        if (
                (usb_ists & USBRF_DISI) ||
                (usb_ists & USBRF_UWRESI) ||
                (usb_ists & USBRF_UHRESI) ||
                (usb_ists & USBRF_DIS2I) ||
                (usb_ists & USBRF_UHSFSRESI)
                ) { // for HS and FS mode

            if (usb_ists & USBRF_DISI) {
                dev->device.speed = CH9_USB_SPEED_UNKNOWN;
            } else
                if (usb_ists & USBRF_UWRESI) {
            } else
                if (usb_ists & USBRF_UHRESI) {
            } else
                if (usb_ists & USBRF_DIS2I) {
                dev->device.speed = CH9_USB_SPEED_UNKNOWN;
            } else
                if (usb_ists & USBRF_UHSFSRESI) {
                dev->device.speed = USBRD_GET_USBSPEED(CPS_UncachedRead32(&dev->reg->USBR_STS));
            }

            // abort all pending requests on all active endpoints
            for (list = dev->device.epList.next; list != &dev->device.epList; list = list->next) {
                ep = (struct CUSBD_Ep *) list;
                epp = get_EpPrivateObj(ep);

                dev->dmaDrv->channelRelease(dev->dmaController, epp->channel);
                epp->transferPending = 0;
                epp->ep_state = EP_DISABLED; // endpoint disabled
                while ((req = getNexReq(epp)) != NULL) {
                    listDelete(&req->list);
                    req->status = ECANCELED;
                    req->complete(ep, req);
                }
            }

            //CPS_UncachedWrite32(USBR_CONF, USBRF_CFGRST);
            CPS_UncachedWrite32(&dev->reg->USBR_ISTS, USBRF_DISI | USBRF_UWRESI | USBRF_UHRESI | USBRF_DIS2I | USBRF_UHSFSRESI);

            dev->device.state = CH9_USB_STATE_DEFAULT;
            dev->u1_value = dev->u2_value = 0;
            dev->callbacks.disconnect(&dev->device);


            // for testing purposes line should be enabled
            //((CUSBD_PrivateData*) pD)->callback.disconnect(pD);

            // clear all active transfers
            // clear all STALLS on endpoints
            return;
        }

        // USB Connect interrupt
        if ((usb_ists & USBRF_CONI) || (usb_ists & USBRF_CON2I)) {
            CPS_UncachedWrite32(&dev->reg->USBR_ISTS, USBRF_CONI | USBRF_CON2I);

            // set actual USB speed
            dev->device.speed = USBRD_GET_USBSPEED(CPS_UncachedRead32(&dev->reg->USBR_STS));

            // discrepancy between speed values for USBSS_DEV core and Linux
            // Linux has 5 value for SS mode, USBSS_DEV has 4 for SS mode
            if (dev->device.speed == USBRV_SUPER_SPEED) dev->device.speed = CH9_USB_SPEED_SUPER;

            initDevHw(dev); // configure device hardware
            initEp0(dev); // initialize default endpoint hardware
            VERBOSE("CONNECT EVENT, actual speed: %d\n", dev->device.speed);
            VERBOSE("CONNECT EVENT, max speed: %d\n", dev->device.maxSpeed);
            dev->callbacks.connect(&dev->device); // call user callback
            return;
        }

        if (usb_ists & USBRF_U1ENTI) {
            CPS_UncachedWrite32(&dev->reg->USBR_ISTS, USBRF_U1ENTI);
            return;
        }

        if (usb_ists & USBRF_U1EXTI) {
            CPS_UncachedWrite32(&dev->reg->USBR_ISTS, USBRF_U1EXTI);
            return;
        }

        if (usb_ists & USBRF_U2ENTI) {
            CPS_UncachedWrite32(&dev->reg->USBR_ISTS, USBRF_U2ENTI);
            return;
        }

        if (usb_ists & USBRF_U2EXTI) {
            CPS_UncachedWrite32(&dev->reg->USBR_ISTS, USBRF_U2EXTI);
            return;
        }

        if (usb_ists & USBRF_U3ENTI) {
            CPS_UncachedWrite32(&dev->reg->USBR_ISTS, USBRF_U3ENTI);
            return;
        }

        if (usb_ists & USBRF_U3EXTI) {
            CPS_UncachedWrite32(&dev->reg->USBR_ISTS, USBRF_U3EXTI);
            return;
        }

        if (usb_ists & USBRF_L2ENTI) {
            CPS_UncachedWrite32(&dev->reg->USBR_ISTS, USBRF_L2ENTI);
            dev->callbacks.suspend(&dev->device);
            return;
        }

        if (usb_ists & USBRF_L2EXTI) {
            CPS_UncachedWrite32(&dev->reg->USBR_ISTS, USBRF_L2EXTI);
            dev->callbacks.resume(&dev->device);
            return;
        }

        if (usb_ists & USBRF_ITPI) {
            CPS_UncachedWrite32(&dev->reg->USBR_ISTS, USBRF_ITPI);
            if (dev->callbacks.busInterval != NULL) dev->callbacks.busInterval(&dev->device);
            return;
        }
    }
    // check if interrupt generated by endpoint and return if no
    ep_ists = CPS_UncachedRead32(&dev->reg->USBR_EP_ISTS);
    VERBOSE("EP_ISTS_0: %x\n", ep_ists);
    if (ep_ists == 0) return;

    // check if interrupt generated by default endpoint on both directions (ss speed)

    // ep0out
    if (ep_ists & 0x00000001) {
        handleEp0Irq(dev, 0);
        ep_ists &= ~0x00000001;
        return;
    }

    // ep0in
    if (ep_ists & 0x00010000) {
        handleEp0Irq(dev, 1);
        ep_ists &= ~0x00010000;
        return;
    }

    VERBOSE("EP_ISTS_1: %x\n", ep_ists);
    dev->dmaDrv->isr(dev->dmaController);
}

static uint32_t epEnable(void *pD, struct CUSBD_Ep *ep, const CH9_UsbEndpointDescriptor * desc) {

    CUSBD_PrivateData* dev;
    CUSBD_EpPrivate * epp;
    uint32_t int_flag;
    uint32_t ep_ien;
    uint32_t ep_config = 0;
    uint32_t epBuffering = 0;

    VERBOSE("ep:%02X\n", ep->address);
    VERBOSE("ep_desc:%02X\n", desc->bEndpointAddress);

    if (!pD || !ep || !desc || desc->bDescriptorType != CH9_USB_DT_ENDPOINT) {
        ERROR("Invalid input parameter %c\n", ' ');
        return EINVAL;
    }

    if (!desc->wMaxPacketSize) {
        ERROR("Missing wMaxPacketSize %c\n", ' ');
        return EINVAL;
    }

    dev = get_DevPrivateObj(pD);
    epp = get_EpPrivateObj(ep);

    // set enabled state
    epp->ep_state = EP_ENABLED;

    // no pending transfer on initializing procedure
    epp->transferPending = 0;

    // No actual request
    epp->actualReq = NULL;

    // clear wedge flag
    epp->wedgeFlag = 0;

    /*
        switch (desc->bmAttributes & CH9_USB_EP_TRANSFER_MASK) {
            case CH9_USB_EP_CONTROL:
                strcat(ep->name, "-control");
                break;
            case CH9_USB_EP_ISOCHRONOUS:
                strcat(ep->name, "-isoc");
                break;
            case CH9_USB_EP_BULK:
                strcat(ep->name, "-bulk");
                break;
            case CH9_USB_EP_INTERRUPT:
                strcat(ep->name, "-int");
                break;
            default:
                cusbd_debug("invalid endpoint transfer type\n");
        }
     */
    ep->desc = desc;

    // get buffering value
    if (ep->address & CH9_USB_EP_DIR_IN) {
        epBuffering = dev->config.epIN[(ep->address & 0x7F) - 1].bufferingValue;
    } else {
        epBuffering = dev->config.epOUT[(ep->address & 0x7F) - 1].bufferingValue;
    }
    if (epBuffering > 0)
        epBuffering--;

    ep_config = USBRD_EP_ENABLED |
            USBRD_EP_TYPE(ep->desc->bmAttributes) |
            USBRD_EP_LITTLE_ENDIAN |
            USBRD_EP_MAXPKSIZE(le16ToCpu(ep->desc->wMaxPacketSize)) |
            USBRD_EP_BUFFERING(epBuffering) |
            USBRD_EP_MULT(2)

            /*
                        USBRD_EP_MULT(ep->compDesc->bmAttributes) | USBRD_EP_MAXBURST((usb_speed_mode == SS_MODE) ? ep_comp->bMaxBurst : 0) |
                        stream_enabled |
                        burst_support |
             */
            ;

    // Configure endpoint
    CPS_UncachedWrite32(&dev->reg->USBR_EP_SEL, desc->bEndpointAddress);
    CPS_UncachedWrite32(&dev->reg->USBR_EP_CFG, ep_config);
    CPS_UncachedWrite32(&dev->reg->USBR_EP_STS_EN, USBRF_EP_TRBERREN | USBRF_EP_ISOERREN);

    // Enable interrupts
    ep_ien = CPS_UncachedRead32(&dev->reg->USBR_EP_IEN);
    int_flag = USBRD_EP_INTERRUPT_EN(USBD_EPNUM_FROM_EPADDR(ep->address),
            USBD_EPDIR_FROM_EPADDR(ep->address));

    // Enable ITP interrupt for iso endpoint
    if ((ep->desc->bmAttributes & CH9_USB_EP_TRANSFER_MASK) == CH9_USB_EP_ISOCHRONOUS) {
        uint32_t regTemp = CPS_UncachedRead32(&dev->reg->USBR_IEN);
        regTemp |= USBRF_ITPIEN;
        CPS_UncachedWrite32(&dev->reg->USBR_IEN, regTemp);
    }

    if (dev->dmaController) {
        epp->channel = dev->dmaDrv->channelAlloc(dev->dmaController,
                ep->address & CH9_USB_EP_DIR_IN, ep->address & 0x7F, 0);
    }

    ep_ien |= int_flag;
    CPS_UncachedWrite32(&dev->reg->USBR_EP_IEN, ep_ien);
    return EOK;
}

static uint32_t epDisable(void *pD, struct CUSBD_Ep * ep)  {

    uint32_t int_flag, ep_ien;
    CUSBD_PrivateData* dev;
    CUSBD_EpPrivate * epp;

    VERBOSE("ep:%02X\n", ep->address);

    if (!ep || !pD) {
        return EINVAL;
    }

    // check if endpoint exist and return if no
    if (ep->address == 0) {
        return EINVAL;
    }

    dev = get_DevPrivateObj(pD);
    epp = get_EpPrivateObj(ep);

    dev->dmaDrv->channelRelease(dev->dmaController, epp->channel);
    // check if request list not empty and call complete function
    if (epp->reqList.next != &epp->reqList) {
        struct CUSBD_Req * req_;
        while ((req_ = getNexReq(epp)) != NULL) {
            listDelete(&req_->list);
            req_->status = ECANCELED;
            req_->complete(ep, req_);
        }
    }

    // set enabled state to disabled
    epp->ep_state = EP_DISABLED;
    epp->transferPending = 0;

    // disable endpoint
    CPS_UncachedWrite32(&dev->reg->USBR_EP_SEL, ep->address);
    CPS_UncachedWrite32(&dev->reg->USBR_EP_CMD, USBRF_EP_EPRST);
    WAIT_UNTIL_BIT_CLEARED(CPS_UncachedRead32(&dev->reg->USBR_EP_CMD), USBRF_EP_EPRST);
    CPS_UncachedWrite32(&dev->reg->USBR_EP_CFG, USBRD_EP_DISABLED);
    CPS_UncachedWrite32(&dev->reg->USBR_EP_STS_EN, 0x00000000);

    // disable interrupts
    ep_ien = CPS_UncachedRead32(&dev->reg->USBR_EP_IEN);
    int_flag = USBRD_EP_INTERRUPT_EN(USBD_EPNUM_FROM_EPADDR(ep->address),
            USBD_EPDIR_FROM_EPADDR(ep->address));
    ep_ien &= ~int_flag;
    CPS_UncachedWrite32(&dev->reg->USBR_EP_IEN, ep_ien);
    return EOK;
}

static uint32_t epSetHalt(void *pD, struct CUSBD_Ep *ep, uint8_t value)  {

    CUSBD_PrivateData* dev;
    CUSBD_EpPrivate *epp;
    //struct CUSBD_Req * req;

    //INFO("ep:%02X\n", ep->address);

    if (!ep || !pD) {
        return EINVAL;
    }

    dev = get_DevPrivateObj(pD);
    epp = get_EpPrivateObj(ep);
    if (epp->ep_state == EP_DISABLED) return EPERM;
    CPS_UncachedWrite32(&dev->reg->USBR_EP_SEL, ep->address);

    if (value) {
        // Important! Race between software and hardware
        CPS_UncachedWrite32(&dev->reg->USBR_EP_CMD, USBRF_EP_SSTALL | USBRF_EP_ERDY | USBRF_EP_DFLUSH);
        // CPS_UncachedWrite32(&dev->reg->USBR_DRBL, 1); // markmarkmark
        WAIT_UNTIL_BIT_CLEARED(CPS_UncachedRead32(&dev->reg->USBR_EP_CMD), USBRF_EP_DFLUSH);
        // Not to set stall for ep0 since it's cleared in next setup.
        if (ep->address & CH9_USB_EP_NUMBER_MASK)
            epp->ep_state = EP_STALLED;
        else
            dev->ep0NextState = EP0_SETUP_PHASE;
    } else {
        epp->wedgeFlag = 0;
        CPS_UncachedWrite32(&dev->reg->USBR_EP_CMD, USBRF_EP_EPRST | USBRF_EP_CSTALL);
        WAIT_UNTIL_BIT_CLEARED(CPS_UncachedRead32(&dev->reg->USBR_EP_CMD), USBRF_EP_EPRST);
        epp->ep_state = EP_ENABLED;
    }

    epp->transferPending = 0;
    return EOK;
}

static uint32_t epSetWedge(void *pD, struct CUSBD_Ep * ep)  {

    uint32_t res;

    VERBOSE("ep:%02X\n", ep->address);

    if (!ep || !pD) {
        return EINVAL;
    }

    get_EpPrivateObj(ep)->wedgeFlag = 1;
    res = epSetHalt(pD, ep, 1);

    return res;
}

static void epFifoFlush(void *pD, struct CUSBD_Ep * ep)  {

    CUSBD_PrivateData* dev;

    VERBOSE("ep:%02X\n", ep->address);

    if (!ep || !pD) {
        return;
    }

    dev = get_DevPrivateObj(pD);
    CPS_UncachedWrite32(&dev->reg->USBR_EP_SEL, (uint32_t) ep->address);
    CPS_UncachedWrite32(&dev->reg->USBR_EP_CMD, USBRF_EP_DFLUSH);
    WAIT_UNTIL_BIT_CLEARED(CPS_UncachedRead32(&dev->reg->USBR_EP_CMD), USBRF_EP_DFLUSH);
}

static uint32_t epFifoStatus(void *pD, struct CUSBD_Ep * ep) {

    VERBOSE("ep:%02X\n", ep->address);

    if (!ep || !pD) {
        return EINVAL;
    }
    return 0;
}

static uint32_t reqQueue(void *pD, struct CUSBD_Ep *ep, struct CUSBD_Req * req)  {

    CUSBD_PrivateData * dev;
    CUSBD_EpPrivate * epp = get_EpPrivateObj(ep);

    if (!ep || !pD || !req) {
        return EINVAL;
    }

    dev = get_DevPrivateObj(pD);

    req->actual = 0;
    req->status = EINPROGRESS;

    VERBOSE("reqQueue for ep: %d\n", ep->address);
    //displayReq(req);
    if (ep->address == EP0_ADDRESS) {
        VERBOSE("dev->ep0NextState: %d\n", dev->ep0NextState);
        // -------- Patch for Linux SET_CONFIGURATION(1) handling --------------
        if (dev->ep0NextState == EP0_STATUS_PHASE) {
            // set status stage OK
            dev->device.state = CH9_USB_STATE_CONFIGURED;
            CPS_UncachedWrite32(&dev->reg->USBR_EP_SEL, 0x00);
            CPS_UncachedWrite32(&dev->reg->USBR_EP_CMD, USBRF_EP_REQ_CMPL | USBRF_EP_ERDY); // status phase
            // CPS_UncachedWrite32(&dev->reg->USBR_DRBL, 1); // markmarkmark
            //CPS_UncachedWrite32(&dev->reg->USBR_CONF, USBRF_CFGSET);
            dev->ep0NextState = EP0_SETUP_PHASE;
            req->status = 0;
            if(req->complete)
                req->complete(&dev->ep0.ep, req);
            return EOK;
        }
        //----------------------------------------------------------------------

        // for default endpoint data must be transfered in context of setup request
        if (dev->ep0NextState != EP0_DATA_PHASE) return EPROTO;
        // it is assumed that ep0 doesn't enqueues requests
        //displayData(req->buf, req->length);
        //ep0transfer(dev, dev->ep0DataDirFlag, req->dma, req->length, dev->ep0DataDirFlag ? 0 : 1);
        ep0transfer(dev, dev->ep0DataDirFlag, req->dma, req->length, 1);
        dev->request = req;
    } else {

        // check if endpoint enabled
        if (epp->ep_state == EP_DISABLED) {
            return EPERM;
        }

        if (req->noInterrupt) {
            epp->transferPending = 1;
            epp->actualReq = req;
            startHwTransfer(dev, epp, req);
        } else {
            // lock(epp->lock);
            listAddTail(&req->list, &epp->reqList);
            if (epp->ep_state == EP_STALLED)return EOK;

            if (epp->transferPending == 0) {
                epp->transferPending = 1;
                req = getNexReq(epp);
                if (req) {
                    epp->actualReq = req;
                    startHwTransfer(dev, epp, req);
                } else {
                    epp->transferPending = 0;
                }
            }
        }
    }
    return EOK;
}

static uint32_t reqDequeue(void *pD, struct CUSBD_Ep *ep, struct CUSBD_Req * req)  {


    if (!ep || !pD || !req) {
        return EINVAL;
    }

    req->status = ECANCELED;
    listDelete(&req->list);
    if (req->complete) {
        req->complete(ep, req);
    }
    return EOK;
}

static uint32_t reqAlloc(void *pD, struct CUSBD_Ep *ep, CUSBD_Req **req)  {

    void * memptr = NULL;

    //INFO("reqAlloc for ep: %d\n", ep->address);

    if (!ep || !pD || !req) {
        return EINVAL;
    }
    CUSBD_PrivateData * dev;
    dev = get_DevPrivateObj(pD);

    if (dev->callbacks.usbRequestMemAlloc)
        memptr = dev->callbacks.usbRequestMemAlloc(pD, sizeof (CUSBD_Req));
    if (memptr == NULL) return ENOMEM;
    *req = (CUSBD_Req *) memptr;

    return EOK;
}

static void reqFree(void *pD, struct CUSBD_Ep *ep, struct CUSBD_Req * req)  {
    VERBOSE("reqFree for ep: %d\n", ep->address);
    //    free(req);
}

static void getDevReference(void *pD, CUSBD_Dev ** dev)  {
    if (!pD || !dev) {
        return;
    }
    *dev = &(get_DevPrivateObj(pD)->device);
}

static uint32_t dGetFrame(void *pD, uint32_t *numOfFrame)  {

    CUSBD_PrivateData* dev;

    if (!pD || !numOfFrame) {
        return EINVAL;
    }

    dev = get_DevPrivateObj(pD);
    *numOfFrame = CPS_UncachedRead32(&dev->reg->USBR_ITPN);
    return EOK;
}


// tries to wake-up host connected to this device

static uint32_t dWakeUp(void *pD)  {
    CUSBD_PrivateData* dev;

    if (!pD) {
        return EINVAL;
    }
    dev = get_DevPrivateObj(pD);
    CPS_UncachedWrite32(&dev->reg->USBR_CONF, USBRF_LGO_L0);
    return ENOTSUP;
}

// sets self powered feature device feature

static uint32_t dSetSelfpowered(void *pD)  {

    if (!pD) {
        return EINVAL;
    }
    return ENOTSUP;
}

// clears self powered feature device feature

static uint32_t dClearSelfpowered(void *pD)  {

    if (!pD) {
        return EINVAL;
    }
    return ENOTSUP;
}

// used in connect or disconnect  VBUS power function

static uint32_t dVbusSession(void *pD, uint8_t isActive)  {

    if (!pD) {
        return EINVAL;
    }
    return ENOTSUP;
}

// constrain controller's VBUS current to mA value

static uint32_t dVbusDraw(void *pD, uint8_t mA)  {

    if (!pD) {
        return EINVAL;
    }
    return ENOTSUP;
}

// connect or disconnect device to/from host

static uint32_t dPullUp(void *pD, uint8_t isOn)  {

    CUSBD_PrivateData* dev;

    if (!pD) return EINVAL;
    dev = get_DevPrivateObj(pD);
    if (isOn) {
        CPS_UncachedWrite32(&dev->reg->USBR_CONF, USBRF_DEVEN);
    } else {
        CPS_UncachedWrite32(&dev->reg->USBR_CONF, USBRF_DEVDS);
    }
    return EOK;
}

// used mainly for debugging purposes

static uint32_t dExecDebug(void *pD, uint32_t code_l, uint32_t param)  {

    if (!pD) {
        return EINVAL;
    }
    return ENOTSUP;
}

// function returns U1 and U2 exit latency - used only in SS mode

static void dGetConfigParams(void *pD, CH9_ConfigParams *configParams)  {
    if (!pD || !configParams) {
        return;
    }
    configParams->bU1devExitLat = CUSBSS_U1_DEV_EXIT_LAT;
    configParams->bU2DevExitLat = CUSBSS_U2_DEV_EXIT_LAT;
}

static void isr_temp(void *pD)  {

    VERBOSE("-------IRQ_Enter--------%c\n", ' ');
    isr(pD);
    VERBOSE("-------IRQ_Exit---------%c\n", ' ');
}

//------------------------------------------------------------------------------

static CUSBD_OBJ CusbdDrv = {

    probe,
    init,
    destroy,
    start,
    stop,
    isr_temp,
    epEnable,
    epDisable,
    epSetHalt,
    epSetWedge,
    epFifoStatus,
    epFifoFlush,
    reqQueue,
    reqDequeue,
    reqAlloc,
    reqFree,
    getDevReference,
    dGetFrame,
    dWakeUp,
    dSetSelfpowered,
    dClearSelfpowered,
    dVbusSession,
    dVbusDraw,
    dPullUp,
    dExecDebug,
    dGetConfigParams
};

CUSBD_OBJ *CUSBD_GetInstance(void) {

    return &CusbdDrv;
}

// back door functions - not documented, used for testing purposes

void setSetup(void *pD, uint8_t * setup) {
    CUSBD_PrivateData* dev = get_DevPrivateObj(pD);
    memcpy((void*) dev->setupPacket, setup, 8);
}

// function required for meeting case 5 requirements on Ellisys tester

uint8_t isDataToTransferOnChip(void *pD, struct CUSBD_Ep * ep)  {

    CUSBD_PrivateData* dev;

    if (!ep || !pD) {
        return EINVAL;
    }

    dev = get_DevPrivateObj(pD);

    CPS_UncachedWrite32(&dev->reg->USBR_EP_SEL, ep->address);

    if (CPS_UncachedRead32(&dev->reg->USBR_EP_STS) & USBRF_EP_BUFFEMPTY)
        return 0;
    else return 1;
}
