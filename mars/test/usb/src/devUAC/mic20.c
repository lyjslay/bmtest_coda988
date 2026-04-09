/**********************************************************************
 * Copyright (C) 2014-2017 Cadence Design Systems, Inc.
 * All rights reserved worldwide.
 ***********************************************************************
 * mic20.c
 * USB microphone application as specified in audio20 usb.org specification
 *
 ***********************************************************************/


#include <stdio.h>       // standard libraries
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include "auc20.h"       // audio 2.0 spec
#include "cusb_ch9.h"    // chapter 9 of USB spec
#include "byteorder.h"   // macros for endianes


/*
example of simple USB microphone terminals

    ----------------    ---------------
    | USB_IN_OT_ID |<--| MIC_IN_IT_ID |
    ----------------    ---------------
     usb streaming        microphone
 */

#define EP_ADDRESS_IN 0x83

#define MIC_IN_IT_ID    1
#define USB_IN_OT_ID    2
#define USB_IN_CLK_ID   3

/*
external buffer used for sending control block to host
 */
extern uint8_t *ep0Buff;

static int p_chmask = 0x1; // one audio channel
static int p_ssize = 1; // resolution 1 byte
static uint32_t p_srate = 32000; // sampling 32kHz

/* returns number of channels on given channel mask*/
static int num_channels(int chanmask) {
    int num = 0;

    while (chanmask) {
        num += (chanmask & 1);
        chanmask >>= 1;
    }

    return num;
}
/* Interface association descriptor */
static struct CH9_UsbInterfaceAssociationDescriptor iadDesc = {
    .bLength = CH9_USB_DS_INTERFACE_ASSOCIATION,
    .bDescriptorType = CH9_USB_DT_INTERFACE_ASSOCIATION,
    .bFirstInterface = 0,
    .bInterfaceCount = 2,
    .bFunctionClass = CH9_USB_CLASS_AUDIO,
    .bFunctionSubClass = FUNCTION_SUBCLASS_UNDEFINED,
    .bFunctionProtocol = FUNCTION_PROTOCOL_AF_VERSION_02_00,
};

/* Audio Control Interface */
static struct CH9_UsbInterfaceDescriptor acInterfaceDesc = {
    .bLength = CH9_USB_DS_INTERFACE,
    .bDescriptorType = CH9_USB_DT_INTERFACE,
    .bAlternateSetting = 0,
    .bNumEndpoints = 0,
    .bInterfaceClass = CH9_USB_CLASS_AUDIO,
    .bInterfaceSubClass = INTERFACE_SUBCLASS_AUDIOCONTROL,
    .bInterfaceProtocol = INTERFACE_PROTOCOL_IP_VERSION_02_00,
};

/* Clock source for IN traffic */
static struct UAC2_ClockSourceDescriptor clockSourceDesc = {
    .bLength = sizeof (struct UAC2_ClockSourceDescriptor),
    .bDescriptorType = DT_CS_INTERFACE,
    .bDescriptorSubtype = AC_DESCRIPTOR_CLOCK_SOURCE,
    .bClockID = USB_IN_CLK_ID,
    .bmAttributes = 0x01, // Internal Fixed Clock
    .bmControls = 0x00,
    .bAssocTerminal = 0,
};

/* Input Terminal for I/O-Microphone */
static struct UAC2_InputTerminalDescriptor inputTerminalDesc = {
    .bLength = sizeof (struct UAC2_InputTerminalDescriptor),
    .bDescriptorType = DT_CS_INTERFACE,
    .bDescriptorSubtype = AC_DESCRIPTOR_INPUT_TERMINAL,
    .bTerminalID = MIC_IN_IT_ID,
    .wTerminalType = cpuToLe16(INPUT_MICROPHONE),
    .bAssocTerminal = 0,
    .bCSourceID = USB_IN_CLK_ID,
    .iChannelNames = 0,
    .bmControls = 0,
};

/* Output Terminal for USB_IN */
static struct UAC2_OutputTerminalDescriptor outputTerminalDesc = {
    .bLength = sizeof (struct UAC2_OutputTerminalDescriptor),
    .bDescriptorType = DT_CS_INTERFACE,
    .bDescriptorSubtype = AC_DESCRIPTOR_OUTPUT_TERMINAL,
    .bTerminalID = USB_IN_OT_ID,
    .wTerminalType = cpuToLe16(TERMINAL_TYPES_STREAMING),
    .bAssocTerminal = 0,
    .bSourceID = MIC_IN_IT_ID,
    .bCSourceID = USB_IN_CLK_ID,
    .bmControls = 0,
};

/* Audio streaming class specific descriptor*/
static struct UAC2_ClassSpecificACInterfaceHeaderDescriptor acInterfaceHeaderDesc = {
    .bLength = sizeof (struct UAC2_ClassSpecificACInterfaceHeaderDescriptor),
    .bDescriptorType = DT_CS_INTERFACE,
    .bDescriptorSubtype = AC_DESCRIPTOR_HEADER,
    .bcdADC = cpuToLe16(0x200),
    .bCategory = FUNCTION_SUBCLASS_MICROPHONE,
    .wTotalLength = sizeof (clockSourceDesc) + sizeof (inputTerminalDesc) + sizeof (outputTerminalDesc),
    .bmControls = 0,
};

/* Audio Streaming IN Interface - Alt0 */
static struct CH9_UsbInterfaceDescriptor streamInterface0Desc = {
    .bLength = sizeof (struct CH9_UsbInterfaceDescriptor),
    .bDescriptorType = CH9_USB_DT_INTERFACE,
    .bAlternateSetting = 0,
    .bNumEndpoints = 0,
    .bInterfaceClass = CH9_USB_CLASS_AUDIO,
    .bInterfaceSubClass = INTERFACE_SUBCLASS_AUDIOSTREAMING,
    .bInterfaceProtocol = INTERFACE_PROTOCOL_IP_VERSION_02_00,
    .bInterfaceNumber = 1
};

/* Audio Streaming IN Interface - Alt1 */
static struct CH9_UsbInterfaceDescriptor streamInterface1Desc = {
    .bLength = sizeof (struct CH9_UsbInterfaceDescriptor),
    .bDescriptorType = CH9_USB_DT_INTERFACE,
    .bAlternateSetting = 1,
    .bNumEndpoints = 1,
    .bInterfaceClass = CH9_USB_CLASS_AUDIO,
    .bInterfaceSubClass = INTERFACE_SUBCLASS_AUDIOSTREAMING,
    .bInterfaceProtocol = INTERFACE_PROTOCOL_IP_VERSION_02_00,
    .bInterfaceNumber = 1
};

/* Audio Stream IN Interface Descriptor */
static struct UAC2_ClassSpecificASInterfaceDescriptor classSpecStreamInterfaceDesc = {
    .bLength = sizeof (struct UAC2_ClassSpecificASInterfaceDescriptor),
    .bDescriptorType = DT_CS_INTERFACE,
    .bDescriptorSubtype = AS_DESCRIPTOR_GENERAL,
    .bTerminalLink = USB_IN_OT_ID,
    .bmControls = 0,
    .bFormatType = FORMAT_TYPE_I,
    .bmFormats = cpuToLe32(FORMAT_TYPE_I_PCM8),
    .iChannelNames = 0,
};

/* Audio USB_IN Format */
static struct UAC2_TypeIFormatTypeDescriptor formatDesc = {
    .bLength = sizeof (struct UAC2_TypeIFormatTypeDescriptor),
    .bDescriptorType = DT_CS_INTERFACE,
    .bDescriptorSubtype = AS_DESCRIPTOR_FORMAT_TYPE,
    .bFormatType = FORMAT_TYPE_I,
};

/* STD AS ISO IN Endpoint */
static struct CH9_UsbEndpointDescriptor epInFsDesc = {
    .bLength = CH9_USB_DS_ENDPOINT,
    .bDescriptorType = CH9_USB_DT_ENDPOINT,
    .bEndpointAddress = EP_ADDRESS_IN,
    .bmAttributes = CH9_USB_EP_ISOCHRONOUS | CH9_USB_EP_SYNC_ASYNCHRONOUS,
    .wMaxPacketSize = cpuToLe16(512),
    .bInterval = 1,
};

static struct CH9_UsbEndpointDescriptor epInHsDesc = {
    .bLength = CH9_USB_DS_ENDPOINT,
    .bDescriptorType = CH9_USB_DT_ENDPOINT,
    .bEndpointAddress = EP_ADDRESS_IN,
    .bmAttributes = CH9_USB_EP_ISOCHRONOUS | CH9_USB_EP_SYNC_ASYNCHRONOUS,
    .wMaxPacketSize = cpuToLe16(1024),
    .bInterval = 4,
};

/* CS AS ISO IN Endpoint */
static struct UAC2_ClassSpecificASIsochronousAudioDataEndpointDescriptor audioEpDesc = {
    .bLength = sizeof (struct UAC2_ClassSpecificASIsochronousAudioDataEndpointDescriptor),
    .bDescriptorType = DT_CS_ENDPOINT,
    .bDescriptorSubtype = AC_EP_DESCRIPTOR_GENERAL,
    .bmAttributes = 0,
    .bmControls = 0,
    .bLockDelayUnits = 0,
    .wLockDelay = 0,
};

/*
descriptor set for High speed configuration
 */
static void * descriptorsHs[] = {
    &iadDesc,
    &acInterfaceDesc,
    &acInterfaceHeaderDesc,
    &clockSourceDesc,
    &inputTerminalDesc,
    &outputTerminalDesc,
    &streamInterface0Desc,
    &streamInterface1Desc,
    &classSpecStreamInterfaceDesc,
    &formatDesc,
    &epInHsDesc,
    &audioEpDesc,
    NULL
};

/*
descriptor set for full speed configuration
 */
static void * descriptorsFs[] = {
    &iadDesc,
    &acInterfaceDesc,
    &acInterfaceHeaderDesc,
    &clockSourceDesc,
    &inputTerminalDesc,
    &outputTerminalDesc,
    &streamInterface0Desc,
    &streamInterface1Desc,
    &classSpecStreamInterfaceDesc,
    &formatDesc,
    &epInFsDesc,
    &audioEpDesc,
    NULL
};

/*
control block for sampling frequency control
 */
struct CntrlRangeLay3 {
    uint16_t wNumSubRanges;
    uint32_t dMIN;
    uint32_t dMAX;
    uint32_t dRES;
} __attribute__((packed));

/*
returns current frequency value
 */
static int curReq(CH9_UsbSetup *ctrl) {
    uint8_t entity_id = (ctrl->wIndex >> 8) & 0xff;
    uint8_t control_selector = ctrl->wValue >> 8;
    int value = -1;
    uint32_t r;

    if (control_selector == CS_SAM_FREQ_CONTROL) {
        if (entity_id == USB_IN_CLK_ID)
            r = cpuToLe32((uint32_t) p_srate);
        else
            return -1;

        value = ctrl->wLength < sizeof (r) ? ctrl->wLength : sizeof (r);
        memcpy(ep0Buff, &r, value);
    } else if (control_selector == CS_CLOCK_VALID_CONTROL) {
        ep0Buff[0] = 1;
        value = ctrl->wLength < 1 ? ctrl->wLength : 1;
    } else {
        return -1;
    }
    return value;
}

/*
returns range frequency value
 */
static int rangeReq(CH9_UsbSetup *ctrl) {
    uint8_t entity_id = (ctrl->wIndex >> 8) & 0xff;
    uint8_t control_selector = ctrl->wValue >> 8;
    struct CntrlRangeLay3 r;
    int value = -1;

    if (control_selector == CS_SAM_FREQ_CONTROL) {
        if (entity_id == USB_IN_CLK_ID)
            r.dMIN = cpuToLe32((uint32_t) p_srate);
        else
            return -1;

        r.dMAX = r.dMIN;
        r.dRES = 0;
        r.wNumSubRanges = cpuToLe16(1);

        value = ctrl->wLength < sizeof (r) ? ctrl->wLength : sizeof (r);
        memcpy(ep0Buff, &r, value);
    } else {
        return -1;
    }
    return value;
}

/*
handles audio class specific requests
 */
int acRequest2(CH9_UsbSetup *ctrl) {
    if (ctrl->bRequest == REQUEST_CUR)
        return curReq(ctrl);
    else if (ctrl->bRequest == REQUEST_RANGE)
        return rangeReq(ctrl);
    else
        return -1;
}

/*
returns pointer to endpoint descriptor
 */
uint8_t *getEpDesc2(CH9_UsbSpeed speed) {
    switch (speed) {

        case CH9_USB_SPEED_FULL:
            return ((uint8_t*) & epInFsDesc);
            break;

        case CH9_USB_SPEED_SUPER:
        case CH9_USB_SPEED_HIGH:
            return ((uint8_t*) & epInHsDesc);
            break;

        default: return NULL;
    }
}

/*
returns pointer to endpoint descriptor in audioDesc address, function returns
 * number of bytes of configuration for selected speed mode, or -1 if error
 */
int getDesc2(CH9_UsbSpeed speed, uint8_t * audioDesc) {

    int i = 0;
    void * desc;
    int sum = 0;
    void*(*tab)[];

    inputTerminalDesc.bNrChannels = (uint8_t) num_channels(p_chmask);
    inputTerminalDesc.bmChannelConfig = cpuToLe32((uint32_t) p_chmask);

    classSpecStreamInterfaceDesc.bNrChannels = (uint8_t) num_channels(p_chmask);
    classSpecStreamInterfaceDesc.bmChannelConfig = cpuToLe32((uint32_t) p_chmask);

    formatDesc.bSubslotSize = (uint8_t) p_ssize;
    formatDesc.bBitResolution = (uint8_t) p_ssize * 8;

    switch (speed) {

        case CH9_USB_SPEED_FULL:
            tab = &descriptorsFs;
            break;

        case CH9_USB_SPEED_SUPER:
        case CH9_USB_SPEED_HIGH:
            tab = &descriptorsHs;
            break;

        default: return -1;
    }

    desc = (*tab)[i];

    while (desc) {
        int length = *(uint8_t*) desc;
        memcpy(&audioDesc[sum], desc, length);
        sum += length;
        //for (j = 0; j < length; j++)
        //    printf("0x%02X, ", ((uint8_t*) desc)[j]);
        //printf("\n");
        //desc++;
        desc = (*tab)[++i];
    }

    return sum;
}

