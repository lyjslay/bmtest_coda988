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
#include "bm_usb.h"
#include "mmio.h"
#include "map_system_memory.h"
#include "cps.h"
#ifdef BUILD_ATF
#include "delay_timer.h"
#endif // BUILD_ATF
#include "usb_tty.h"
#include "ss_dev_hw.h"

#define USE_MALLOC 1

#ifdef BUILD_ATF
extern uint16_t bm_usb_vid;
#else
uint16_t bm_usb_vid = 0x30B1;
#endif

// Super Speed USB object configuration
static CUSBD_Config config = {
    .regBase = USB_DEV_BASE, // address where USB core is mapped
    .epIN =
    {
        {.bufferingValue = 4} // ep1in
    },
    .epOUT =
    {
        {.bufferingValue = 4} //ep1out
    },
    .dmultEnabled = 0, // set to 1 if scatter/gather DMA available
    .dmaInterfaceWidth = CUSBD_DMA_64_WIDTH,
    .preciseBurstLength = CUSBD_PRECISE_BURST_0
};

//variable declare
static CUSBD_OBJ * drv; // driver pointer
static void * pD; // private data pointer
static uint8_t* bulkBuf, * cmdBuf, *ep0Buff;
static struct   CUSBD_Ep * epIn, * epOut;
static struct   CUSBD_Req *bulkInReq, *bulkOutReq, *ep0Req; // request used by driver
static uint8_t  configValue = 0;
static uint8_t  configBreak = 0;
static uint8_t  acm_configValue = 0;
static uint8_t  current_speed = CH9_USB_SPEED_UNKNOWN;
static uint8_t  mem_alloc_cnt = 0;
static uint32_t transfer_size = 0;
static uint8_t  flagEnterDL = 0;
struct f_acm *acm;

CUSBD_Dev * dev; // auxiliary pointer used for testing purposes

static uint8_t *pd_buf       = (uint8_t *)PD_ADDR;      // 4096
static uint8_t *bulkBufAlloc = (uint8_t *)BLK_BUF_ADDR; // 1024
static uint8_t *cmdBufAlloc  = (uint8_t *)CMD_BUF_ADDR; // 1024
static uint8_t *trb_buf      = (uint8_t *)TRB_BUF_ADDR; // 256
static uint8_t *cb0_buf      = (uint8_t *)CB0_BUF_ADDR; // 128
static uint8_t *cb1_buf      = (uint8_t *)CB1_BUF_ADDR; // 128
static uint8_t *cb2_buf      = (uint8_t *)CB2_BUF_ADDR; // 128
static uint8_t *ep0BuffAlloc = (uint8_t *)EP0_BUF_ADDR; // 512
static uint8_t *rsp_buf      = (uint8_t *)RSP_BUF_ADDR; // 8
static uint8_t *acm_buf      = (uint8_t *)ACM_BUF_ADDR; // 128

// string will be filled then in initializing section
static char vendorDesc[sizeof (USB_MANUFACTURER_STRING) * 2 + 2];
static char productDesc[sizeof (USB_PRODUCT_STRING) * 2 + 2];
static char serialDesc[sizeof (USB_SERIAL_NUMBER_STRING) * 2 + 2];

typedef void func(void);

/* Driver private data */
typedef struct {
    // this "class must implement" CUSBD class, this class is an extension of CUSBD
    CUSBD_Dev device;

    CUSBD_Config config; // copy of user configuration

    CUSBD_Callbacks callbacks; // copy of user callback
    CH9_UsbSetup *setupPacket;
    uintptr_t setupPacketDma;

    // these variables reflect device state
    uint16_t status_value; // keeps status value
    uint8_t u2_value; // keeps U2 value
    uint8_t u1_value; // keeps U1 value
    uint16_t suspend; // keeps suspend value
    uint16_t isoch_delay;

    ssReg_t  *reg;
} CUSBD_PrivateData;

void print_buf_addr () {
    INFO ("pd_buf: %p\n", pd_buf);
    INFO ("bulkBufAlloc: %p\n", bulkBufAlloc);
    INFO ("cmdBufAlloc: %p\n", cmdBufAlloc);
    INFO ("trb_buf: %p\n", trb_buf);
    INFO ("cb0_buf: %p\n", cb0_buf);
    INFO ("cb1_buf: %p\n", cb1_buf);
    INFO ("cb2_buf: %p\n", cb2_buf);
    INFO ("ep0BuffAlloc: %p\n", ep0BuffAlloc);
    INFO ("rsp_buf: %p\n", rsp_buf);
    INFO ("acm_buf: %p\n", acm_buf);
}

static CUSBD_PrivateData * get_DevPrivateObj(void * pD) {

    return (CUSBD_PrivateData*) pD;
}

// interrupt handler
void AcmIsr() {
    drv->isr(pD);
}

static int getDescAcm(CH9_UsbSpeed speed, uint8_t * acmDesc) {

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

        default: return -1;
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

static void clearReq(struct CUSBD_Req * req) {
    memset(req, 0, sizeof (struct CUSBD_Req));
}

static void connect(void *pD)  {
    //CUSBD_Dev *dev;

    INFO("Application: connect \n");
}

static void disconnect(void *pD)  {
   acm_configValue = 0;
   mem_alloc_cnt = 1;
   configValue = 0;
   NOTICE("Application: disconnect\n");
}

static void resume(void *pD)  {
    VERBOSE("Application: resume\n");
}

static void reqComplete(struct CUSBD_Ep *ep, struct CUSBD_Req * req)  {
    CUSBD_PrivateData* dev;

    dev = get_DevPrivateObj(pD);

    CPS_UncachedWrite32(&dev->reg->USBR_EP_STS, USBRF_EP_DESCMIS);

    VERBOSE("Request on endpoint completed\n");
    //displayRequestinfo(req);

}

static void suspend(void *pD)  {
    VERBOSE("Application: suspend %c\n", ' ');
}

static void * requestMemAlloc(void * pD, uint32_t requireSize)  {
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

static void requestMemFree(void * pD, void* usbRequest)  {
    return;
}

static void bulkInCmpl(struct CUSBD_Ep *ep, struct CUSBD_Req * req)  {
    VERBOSE("bulkInCmpl\n");
    memset(cmdBuf, 0x00, HEADER_SIZE); //next time init
    bulkOutReq->length = transfer_size;
    bulkOutReq->buf = cmdBuf;
    bulkOutReq->dma = (uintptr_t) cmdBuf;
    INFO ("epOut->ops->reqQueue\n");
    epOut->ops->reqQueue(pD, epOut, bulkOutReq);
    return;
}

static void bulkOutCmpl(struct CUSBD_Ep *ep, struct CUSBD_Req * req)  {
    static uint8_t ack_idx = 0;
    uint32_t i;
    void *dest_addr = 0x0;
    func *jump_fun;
    uint16_t crc;
    
    BM_USB_TOKEN token  = ((uint8_t*) req->buf)[0];
    uint32_t length = (((uint8_t*) req->buf)[1] << 8) + \
                      (((uint8_t*) req->buf)[2]);
    
    dest_addr = (void *)(((uint64_t)(((uint8_t*) req->buf)[3]) << 32) + \
                ((uint64_t)(((uint8_t*) req->buf)[4]) << 24) + \
                ((uint64_t)(((uint8_t*) req->buf)[5]) << 16) + \
                ((uint64_t)(((uint8_t*) req->buf)[6]) << 8)  + \
                ((uint64_t)(((uint8_t*) req->buf)[7])));
    jump_fun = (func*)dest_addr;
    // VERBOSE("Transfer complete on ep:%02X %08X req\n", ep->address, (uintptr_t) req);

    if (length == 0 && dest_addr == 0){
        bulkOutReq->length = transfer_size;
        bulkOutReq->buf = cmdBuf;
        bulkOutReq->dma = (uintptr_t) cmdBuf;
        VERBOSE ("buffer zero\n");
        epOut->ops->reqQueue(pD, epOut, bulkOutReq);
        return;
    }

    crc = crc16_ccitt(0, cmdBuf, length);
    NOTICE ("CRC: %x\n", crc);

    switch (token) {
        case BM_USB_VERBOSE:
            for (i=0; i<length; i++) {
                NOTICE ("cmdBuf[%d] = %x\n", i, cmdBuf[i]);
            }
        case BM_USB_INFO:
            NOTICE ("LENGTH: %d\n", length);
            NOTICE ("DEST:   %p\n", dest_addr);
            NOTICE ("CRC:    %x\n", crc);
        case BM_USB_NONE:
            memcpy (dest_addr, cmdBuf + HEADER_SIZE, length - HEADER_SIZE);
            break;

        case BM_USB_JUMP:
            NOTICE ("BM_USB_JUMP to %p\n", dest_addr);
            jump_fun();
            NOTICE ("BM_USB_JUMP back\n");
            break;

        case BM_USB_BREAK:
            NOTICE ("BM_USB_BREAK\n");
            ack_idx = 0;
            configBreak = 1;
            break;

        case BM_USB_KEEP_DL:
            NOTICE ("BM_USB_KEEP_DL\n");
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

        case BM_USB_PRG_CMD:
            NOTICE ("BM_USB_PRG_CMD\n");
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
    INFO ("rsp_buf epIn->ops->reqQueue\n");
    epIn->ops->reqQueue(pD, epIn, bulkInReq);

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

/* ACM control ... data handling is delegated to tty library code.
 * The main task of this function is to activate and deactivate
 * that code based on device state; track parameters like line
 * speed, handshake state, and so on; and issue notifications.
 */

static void acm_complete_set_line_coding(struct CUSBD_Ep *ep, struct CUSBD_Req * req)
{
    CUSBD_PrivateData* dev;

    dev = get_DevPrivateObj(pD);
    struct usb_cdc_line_coding  *value = req->buf;
    //VERBOSE("req buf : %d, %d, %d, %d\n", value->dwDTERate, value->bCharFormat,value->bParityType,value->bDataBits);
    acm->port_line_coding = *value;
    CPS_UncachedWrite32(&dev->reg->USBR_EP_STS, USBRF_EP_DESCMIS);
    VERBOSE("acm data transfer complete\n");
}

static uint32_t setup(void *pD, CH9_UsbSetup *ctrl)  {

    // get device reference
    CUSBD_Dev * dev;
    int length = 0;

    CH9_UsbDeviceDescriptor * devDesc;
    CH9_UsbEndpointDescriptor *endpointEpInDesc, *endpointEpOutDesc;

    ctrl->wIndex = le16ToCpu(ctrl->wIndex);
    ctrl->wLength = le16ToCpu(ctrl->wLength);
    ctrl->wValue = le16ToCpu(ctrl->wValue);

    drv->getDevInstance(pD, &dev);

    VERBOSE("Speed %d: \n", dev->speed);
    VERBOSE("bRequest: %02X\n", ctrl->bRequest);
    VERBOSE("bRequestType: %02X\n", ctrl->bmRequestType);
    VERBOSE("wIndex: %04X\n", ctrl->wIndex);
    VERBOSE("wValue: %04X\n", ctrl->wValue);
    VERBOSE("wLength: %04X\n", ctrl->wLength);

    ep0Req->buf = ep0Buff;
    ep0Req->dma = (uintptr_t) ep0Buff;
    ep0Req->complete = reqComplete;

    switch (dev->speed) {

    case CH9_USB_SPEED_FULL:
        endpointEpInDesc = &acm_fs_in_desc;
        endpointEpOutDesc = &acm_fs_out_desc;
        devDesc = &devHsDesc;
        break;

    case CH9_USB_SPEED_HIGH:
        endpointEpInDesc = &acm_hs_in_desc;
        endpointEpOutDesc = &acm_hs_out_desc;
        devDesc = &devHsDesc;
        break;

    default:
        VERBOSE("Unknown speed %d: \n", dev->speed);
        return 1;
        break;
    }

    switch (ctrl->bmRequestType & CH9_USB_REQ_TYPE_MASK) {

    case CH9_USB_REQ_TYPE_STANDARD:

        switch (ctrl->bRequest) {

        case CH9_USB_REQ_GET_DESCRIPTOR:
            VERBOSE("GET DESCRIPTOR %c\n", ' ');
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

                case CH9_USB_DT_CONFIGURATION:
                {
                    uint16_t acmDescLen = (uint16_t)getDescAcm(dev->speed, &ep0Buff[CH9_USB_DS_CONFIGURATION]);

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

                case CH9_USB_DT_STRING:
                {
                    uint8_t descIndex = (uint8_t) (ctrl->wValue & 0xFF);
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

                case CH9_USB_DT_OTHER_SPEED_CONFIGURATION:
                {
                    uint16_t acmDescLen = (uint16_t)getDescAcm(dev->speed, &ep0Buff[CH9_USB_DS_CONFIGURATION]);

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

        case CH9_USB_REQ_SET_CONFIGURATION:
        {
            struct CUSBD_Ep * ep;
            CUSBD_ListHead *list;

            VERBOSE("SET CONFIGURATION(%d)\n", le16ToCpu(ctrl->wValue));
            if (ctrl->wValue > 1) { return -1;       // no such configuration
            }
            // unconfigure device
            if (ctrl->wValue == 0) {
                configValue = 0;
                for (list = dev->epList.next; list != &dev->epList; list = list->next) {
                    ep = (struct CUSBD_Ep *) list;
                    if (ep->address == BULK_EP_IN || ep->address == BULK_EP_OUT) {
                        ep->ops->epDisable(pD, ep);
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
                if (ep->address == BULK_EP_IN) {
                    ep->ops->epEnable(pD, ep, endpointEpInDesc);
                    VERBOSE("enable EP IN\n");
                    break;
                }
            }
            for (list = dev->epList.next; list != &dev->epList; list = list->next) {
                ep = (struct CUSBD_Ep *) list;
                if (ep->address == BULK_EP_OUT) {
                    ep->ops->epEnable(pD, ep, endpointEpOutDesc);
                    VERBOSE("enable EP OUT\n");
                    break;
                }
            }          
            /*Code control  Self powered feature of USB*/
            if (ConfDesc.bmAttributes & CH9_USB_CONFIG_SELF_POWERED) {
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
            //VERBOSE("CH9_USB_REQ_GET_CONFIGURATION %c\n", ' ');
            break;

        default:
            return -1;         //return error
        }
        break;

    case CH9_USB_REQ_TYPE_CLASS:
    {
        /* SET_LINE_CODING ... just read and save what the host sends */
            switch(ctrl->bRequest){
                case USB_CDC_REQ_SET_LINE_CODING:
                    length = ctrl->wLength;
                    ep0Req->complete = acm_complete_set_line_coding;
                    VERBOSE("USB_CDC_REQ_SET_LINE_CODING %d\n", length);
                    acm_configValue = 1;
                    break;
                case USB_CDC_REQ_SET_CONTROL_LINE_STATE:
                    acm->port_handshake_bits = ctrl->wValue;
                    acm_configValue = 1;
                    VERBOSE("USB_CDC_REQ_SET_CONTROL_LINE_STATE %c\n", ' ');
                    break;
                case USB_CDC_REQ_GET_LINE_CODING:
                    length = ctrl->wLength; 
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
        ep0Req->length = ctrl->wLength < length ? ctrl->wLength : length;
        dev->ep0->ops->reqQueue(pD, dev->ep0, ep0Req);
    }
    return 0;
}

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

static CUSBD_Callbacks callback = {
    disconnect, connect, setup, suspend, resume, requestMemAlloc, requestMemFree
};

int acm_app_init()
{
    //CUSBD_ListHead *list; // used in for_each loop

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

    // checking device and endpoints parameters correctness
    // get CUSBD device instance exposed as Linux gadget device
    drv->getDevInstance(pD, &dev);
    //displayDeviceInfo(dev);

    // allocate request for ep0
    drv->reqAlloc(pD, dev->ep0, &ep0Req);

    /*Change descriptor for maxSpeed == HS only Device*/
    /*For USB2.0 we have to modified wTotalLength of BOS descriptor*/
    if (dev->maxSpeed < CH9_USB_SPEED_SUPER) {
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
    uint32_t res = 0; // keeps result of operation on driver
    CUSBD_SysReq sysReq;
    int fip_src = FIP_SRC_MEMMAP;
    uint32_t ts = 0;
    CUSBD_ListHead *list; // used in for_each loop
#ifdef BUILD_ATF
    uint8_t phy_det_connected = 0;
    phy_det_connected = usb_phy_det_connection ();

    if (phy_det_connected == 0) {
        NOTICE ("USB cable is not connected\n");
        return res;
    }
#endif

    print_buf_addr ();
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

    drv = CUSBD_GetInstance(); // get driver handle

    res = drv->probe(&config, &sysReq);
    if (res != 0) { goto error;}
    VERBOSE("Device memory requirement: %d bytes\n", sysReq.privDataSize);
#if USE_MALLOC
    // allocate memory for device object
    pD = (uint8_t*)malloc((size_t) (sysReq.privDataSize));
    if(pD == NULL) {
        VERBOSE("Error line: %d", __LINE__);
        goto error;
    } else {
        VERBOSE("pD addr %p\n", pD);
    }

    config.trbAddr = malloc(sysReq.trbMemSize);
    //config.trbAddr = 0x04000000;
    if(config.trbAddr == NULL) {
        VERBOSE("Error line: %d", __LINE__);
        goto error;
    } else {
        VERBOSE("config.trbAddr addr %p\n", config.trbAddr);
	trb_buf = config.trbAddr;
    }

    bulkBufAlloc = malloc(BUF_SIZE);
    if (bulkBufAlloc == NULL) {
        VERBOSE("Error line: %d", __LINE__);
        goto error;
    } else {
        VERBOSE("bulkBufAlloc %p\n", bulkBufAlloc);
    }

    cmdBufAlloc = malloc(BUF_SIZE);
    if (cmdBufAlloc == NULL) {
        VERBOSE("Error line: %d", __LINE__);
        goto error;
    } else {
        VERBOSE("bulkBufAlloc %p\n", cmdBufAlloc);
    }

    cb0_buf = malloc(CB_SIZE);
    memset(cb0_buf, 0, CB_SIZE);
    cb1_buf = malloc(CB_SIZE);
    memset(cb1_buf, 0, CB_SIZE);
    cb2_buf = malloc(CB_SIZE);
    memset(cb2_buf, 0, CB_SIZE);

    ep0BuffAlloc = malloc(EP0_SIZE);
    memset(ep0BuffAlloc, 0, EP0_SIZE);
    rsp_buf = malloc(RSP_SIZE);
    memset(rsp_buf, 0, RSP_SIZE);
    acm_buf = malloc(128);
    memset(acm_buf, 0, 128);
#else
    pD = pd_buf;
    VERBOSE("alloc pD addr %p\n", pD);
    
    config.trbAddr = trb_buf;
    VERBOSE("alloc config.trbAddr addr %p\n", config.trbAddr);
#endif
    config.trbDmaAddr = (uintptr_t)config.trbAddr;
    VERBOSE("config.trbDmaAddr addr %lx\n", config.trbDmaAddr);
    res = drv->init(pD, &config, &callback);
    if (res != 0) { goto error;}

    drv->stop(pD);
    NOTICE ("Stop ATF USB port\n");
    acm_app_init();

    VERBOSE("Initializing OK! %d\n", __LINE__);

    drv->start(pD);

    ts = get_timer(0);
    VERBOSE("ts: %u\n", get_timer(ts));

unconfigured:
    while (!acm_configValue) {
        #ifndef USB_IRQ_MODE
        AcmIsr();
        #endif 
        if (get_timer(ts) > 1000 && flagEnterDL == 0) {
            NOTICE("Enumeration failed\n");
            return 0;
        }
    }
    NOTICE("USB enumeration done\n");

    mem_alloc_cnt = 1;
    // find IN endpoint
    for (list = dev->epList.next; list != &dev->epList; list = list->next) {
        epIn = (struct CUSBD_Ep  *) list;
        if (epIn->address == BULK_EP_IN) {
            drv->reqAlloc(pD, epIn, &bulkInReq);
            break;
        }
    }

    // find OUT endpoint

    for (list = dev->epList.next; list != &dev->epList; list = list->next) {
        epOut = (struct CUSBD_Ep  *) list;
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
    default: VERBOSE("Test error\n");
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
    epOut->ops->reqQueue(pD, epOut, bulkOutReq);
    NOTICE("connection speed: %d\n", dev->speed);
    ts = get_timer(0);
    VERBOSE("ts: %u\n", get_timer(ts));

    while(1){
        #ifndef USB_IRQ_MODE
        AcmIsr();
        #endif
        if (!acm_configValue) { goto unconfigured; }
        if (configBreak) { break; }
        if (flagEnterDL == 0) {
            if (get_timer(ts) > 1000) {
                NOTICE("wait data timeout\n");
                break;
            }
        }
    }
    NOTICE("Leave transfer loop\n");

    drv->stop(pD);
    drv->destroy(pD);
    NOTICE("USB stop\n");
    return (0);

error:
    ERROR ("Error %u\n", res);
    return (res);
}
