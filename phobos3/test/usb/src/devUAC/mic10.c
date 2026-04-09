/**********************************************************************
 * Copyright (C) 2014-2017 Cadence Design Systems, Inc.
 * All rights reserved worldwide.
 ***********************************************************************
 * mic10.c
 * USB microphone application as specified in audio10 usb.org specification
 *
 ***********************************************************************/

#include <stdio.h>      // standard libraries
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include "auc10.h"      // audio 1.0 spec
#include "cusb_ch9.h"   // chapter 9 of USB spec
#include "byteorder.h"  // macros for endianes


/*
example of simple microphone with one feature unit for controlling sound volume

    -----------------      -------------------     -------------------
    | UVC_AUDIO_OT_ID |<-- | UVC_AUDIO_FC_ID |<----| UVC_AUDIO_IT_ID |
    ------------------     -------------------     -------------------
     usb streaming           volume control            microphone
 */

/*
external buffer used for sending control block to host
 */
extern uint8_t *ep0Buff;

/*
audio endpoint address
 */
#define EP_ADDRESS_IN 0x83

/* Audio unit IDs */
#define UVC_AUDIO_IT_ID        1
#define UVC_AUDIO_FC_ID        2
#define UVC_AUDIO_OT_ID        3

/* Interfaces */
#define UVC_AUDIO_INTF_IDLE    0
#define UVC_AUDIO_INTF_STREAM  1

/*
 keeps actual volume control value
 */
static uint16_t volumeValue = 0;

/* Audio Control Interface */
static struct CH9_UsbInterfaceDescriptor acInterfaceDesc = {
    .bLength = CH9_USB_DS_INTERFACE,
    .bDescriptorType = CH9_USB_DT_INTERFACE,
    .bInterfaceNumber = UVC_AUDIO_INTF_IDLE,
    .bAlternateSetting = 0,
    .bNumEndpoints = 0,
    .bInterfaceClass = CH9_USB_CLASS_AUDIO,
    .bInterfaceSubClass = INTERFACE_SUBCLASS_AUDIOCONTROL,
    .bInterfaceProtocol = 0,
    .iInterface = 0
};

/* Input Terminal for I/O-Microphone */
static struct InputTerminalDescriptor inputTerminalDesc = {
    .bLength = sizeof (struct InputTerminalDescriptor),
    .bDescriptorType = DT_CS_INTERFACE,
    .bDescriptorSubtype = AC_DESCRIPTOR_INPUT_TERMINAL,
    .bTerminalID = UVC_AUDIO_IT_ID,
    .wTerminalType = cpuToLe16(INPUT_MICROPHONE),
    .bAssocTerminal = 0,
    .bNrChannels = 1,
    .wChannelConfig = cpuToLe16(0x0000),
    .iChannelNames = 0,
    .iTerminal = 0
};

/* Feature Unit descriptor */
static struct FeatureUnitDescriptor featureUnitDesc = {
    .bLength = sizeof (struct FeatureUnitDescriptor),
    .bDescriptorType = DT_CS_INTERFACE,
    .bDescriptorSubtype = AC_DESCRIPTOR_FEATURE_UNIT,
    .bUnitID = UVC_AUDIO_FC_ID,
    .bSourceID = UVC_AUDIO_IT_ID,
    .bControlSize = 1,
    .bmaControls = 0x02,
    .iFeature = 0
};

/* Output Terminal for USB_IN */
static struct OutputTerminalDescriptor outputTerminalDesc = {
    .bLength = sizeof (struct OutputTerminalDescriptor),
    .bDescriptorType = DT_CS_INTERFACE,
    .bDescriptorSubtype = AC_DESCRIPTOR_OUTPUT_TERMINAL,
    .bTerminalID = UVC_AUDIO_OT_ID,
    .wTerminalType = cpuToLe16(TERMINAL_TYPES_STREAMING),
    .bAssocTerminal = 0,
    .bSourceID = UVC_AUDIO_FC_ID,
    .iTerminal = 0
};

/* Audio streaming class specific descriptor*/
static struct ClassSpecificACInterfaceHeaderDescriptor acInterfaceHeaderDesc = {
    .bLength = sizeof (struct ClassSpecificACInterfaceHeaderDescriptor),
    .bDescriptorType = DT_CS_INTERFACE,
    .bDescriptorSubtype = AC_DESCRIPTOR_HEADER,
    .bcdADC = cpuToLe16(0x100),
    .wTotalLength = sizeof (inputTerminalDesc) + sizeof (outputTerminalDesc) + sizeof (featureUnitDesc) + sizeof (struct ClassSpecificACInterfaceHeaderDescriptor),
    .bInCollection = 1,
    .baInterfaceNr = UVC_AUDIO_INTF_STREAM
};

/* Audio Streaming IN Interface - Alt0 */
static struct CH9_UsbInterfaceDescriptor streamInterface0Desc = {
    .bLength = sizeof (struct CH9_UsbInterfaceDescriptor),
    .bDescriptorType = CH9_USB_DT_INTERFACE,
    .bInterfaceNumber = UVC_AUDIO_INTF_STREAM,
    .bAlternateSetting = 0,
    .bNumEndpoints = 0,
    .bInterfaceClass = CH9_USB_CLASS_AUDIO,
    .bInterfaceSubClass = INTERFACE_SUBCLASS_AUDIOSTREAMING,
    .bInterfaceProtocol = 0,
    .iInterface = 0
};

/* Audio Streaming IN Interface - Alt1 */
static struct CH9_UsbInterfaceDescriptor streamInterface1Desc = {
    .bLength = sizeof (struct CH9_UsbInterfaceDescriptor),
    .bDescriptorType = CH9_USB_DT_INTERFACE,
    .bInterfaceNumber = UVC_AUDIO_INTF_STREAM,
    .bAlternateSetting = 1,
    .bNumEndpoints = 1,
    .bInterfaceClass = CH9_USB_CLASS_AUDIO,
    .bInterfaceSubClass = INTERFACE_SUBCLASS_AUDIOSTREAMING,
    .bInterfaceProtocol = 0,
    .iInterface = 0
};

/* Audio Stream IN Interface Descriptor */
static struct ClassSpecificASInterfaceDescriptor classSpecStreamInterfaceDesc = {
    .bLength = sizeof (struct ClassSpecificASInterfaceDescriptor),
    .bDescriptorType = DT_CS_INTERFACE,
    .bDescriptorSubtype = AS_DESCRIPTOR_GENERAL,
    .bTerminalLink = UVC_AUDIO_OT_ID,
    .bDelay = 0,
    .wFormatTag = cpuToLe16(TYPE_I_PCM8)
};

/* Audio USB_IN Format */
static struct TypeIFormatTypeDescriptor formatDesc = {
    .bLength = sizeof (struct TypeIFormatTypeDescriptor),
    .bDescriptorType = DT_CS_INTERFACE,
    .bDescriptorSubtype = AS_DESCRIPTOR_FORMAT_TYPE,
    .bFormatType = FORMAT_TYPE_I,
    .bNrChannels = 1,
    .bSubFrameSize = 1,
    .bBitResolution = 8,
    .bSamFreqType = 1,
    .tSamFreq[0] = 0x00, // sampling frequency 32kHz
    .tSamFreq[1] = 0x7D, //
    .tSamFreq[2] = 0x00 //
};

/* STD AS ISO IN Endpoint Full Speed */
static struct CH9_UsbEndpointDescriptor epInFsDesc = {
    .bLength = CH9_USB_DS_ENDPOINT,
    .bDescriptorType = CH9_USB_DT_ENDPOINT,
    .bEndpointAddress = EP_ADDRESS_IN,
    .bmAttributes = CH9_USB_EP_ISOCHRONOUS | CH9_USB_EP_SYNC_ASYNCHRONOUS,
    .wMaxPacketSize = cpuToLe16(32), // 32 bytes in every frame-1 ms gives 32kB/s, one channel 8 -bit resolution
    .bInterval = 1,
};

/* STD AS ISO IN Endpoint High Speed */
static struct CH9_UsbEndpointDescriptor epInHsDesc = {
    .bLength = CH9_USB_DS_ENDPOINT,
    .bDescriptorType = CH9_USB_DT_ENDPOINT,
    .bEndpointAddress = EP_ADDRESS_IN,
    .bmAttributes = CH9_USB_EP_ISOCHRONOUS | CH9_USB_EP_SYNC_ASYNCHRONOUS,
    .wMaxPacketSize = cpuToLe16(4), // 4 bytes/uframe = 32 bytes/frame = 32kB/s, one channel 8 bytes
    .bInterval = 4,
};

/* CS AS ISO IN Endpoint */
static struct ClassSpecificASIsochronousAudioDataEndpointDescriptor audioEpDesc = {
    .bLength = sizeof (struct ClassSpecificASIsochronousAudioDataEndpointDescriptor),
    .bDescriptorType = DT_CS_ENDPOINT,
    .bDescriptorSubtype = AC_EP_DESCRIPTOR_GENERAL,
    .bmAttributes = 0,
    .bLockDelayUnits = 0,
    .wLockDelay = 0,
};

/*
descriptor set for High speed configuration
 */
static void * descriptorsHs[] = {

    &acInterfaceDesc,
    &acInterfaceHeaderDesc,
    &inputTerminalDesc,
    &featureUnitDesc,
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

    &acInterfaceDesc,
    &acInterfaceHeaderDesc,
    &inputTerminalDesc,
    &featureUnitDesc,
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
handles audio class specific requests
 */
int acRequest1(CH9_UsbSetup *ctrl) {

    uint8_t length = 0;

    switch (ctrl->bRequest) {
        case SET_CUR:
            volumeValue = *(uint16_t*) ep0Buff;
            length = 2;
            break;
        case GET_CUR:
            *(uint16_t*) ep0Buff = volumeValue;
            length = 2;
            break;
        case GET_MIN:
            *(uint16_t*) ep0Buff = (uint16_t) cpuToLe16(0x8000);
            length = 2;
            break;
        case GET_MAX:
            *(uint16_t*) ep0Buff = (uint16_t) cpuToLe16(0);
            length = 2;
            break;
        case GET_RES:
            *(uint16_t*) ep0Buff = (uint16_t) cpuToLe16(1);
            length = 2;
            break;
        case GET_LEN: break;
        case GET_INFO: break;
        case GET_DEF: break;
        default: return -1;
    }
    return length;
}

/*
returns pointer to endpoint descriptor
 */
uint8_t *getEpDesc1(CH9_UsbSpeed speed) {
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
int getDesc1(CH9_UsbSpeed speed, uint8_t * audioDesc) {

    int i = 0;
    void * desc;
    int sum = 0;
    void*(*tab)[];

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

    //printf("\n");
    while (desc) {
        int length = *(uint8_t*) desc;
        memcpy(&audioDesc[sum], desc, length);
        sum += length;
        //for (j = 0; j < length; j++)
        //    printf("0x%02X, ", ((uint8_t*) desc)[j]);
        //printf("\n");
        desc++;
        desc = (*tab)[++i];
    }

    return sum;
}
