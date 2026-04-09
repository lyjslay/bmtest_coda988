/**********************************************************************
 * Copyright (C) 2016 Cadence Design Systems, Inc.
 * All rights reserved worldwide.
 ***********************************************************************
 * mic30.c
 * USB microphone application as specified in audio30 usb.org specification
 *
 ***********************************************************************/


#include <stdio.h>       // standard libraries
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include "uac30.h"       // audio 3.0 spec
#include "dwc2_ch9.h"    // chapter 9 of USB spec
#include "byteorder.h"   // macros for endianess manipulation

/*
example of simple microphone with one feature unit for controlling sound volume

    -----------------      -------------------     -------------------
    | UVC_AUDIO_OT_ID |<-- | USB_IN_FU_ID    |<----| UVC_AUDIO_IT_ID |
    ------------------     -------------------     -------------------
     usb streaming           volume control            microphone
 */

#define EP_ADDRESS_IN 0x83

#define MIC_IN_IT_ID    4      // INPUT TERMINAL ID
#define USB_IN_FU_ID    5      // FEATURE UNIT TERMINAL ID
#define USB_IN_OT_ID    6      // OUTPUT TERMINAL ID
#define USB_IN_CLK_ID   9      // CLOCK UNIT ID
#define USB_IN_PD_ID    0x0B   // POWER DOMAIN ID

/* keeps actual value of volume control */
static volatile uint32_t current_volume = 0;

/* external buffer used for sending/receiving control block to/from host */
extern uint8_t *ep0Buff;

/* keeps supported audio sampling frequency */
static uint32_t p_srate = 48000;

/* Interface association descriptor */
static struct CH9_UsbInterfaceAssociationDescriptor iadDesc = {
	.bLength = CH9_USB_DS_INTERFACE_ASSOCIATION, // SPEC BUG!! value=9 instead 8
	.bDescriptorType = CH9_USB_DT_INTERFACE_ASSOCIATION,
	.bFirstInterface = 0,
	.bInterfaceCount = 2,
	.bFunctionClass = CH9_USB_CLASS_AUDIO,
	.bFunctionSubClass = FUNCTION_SUBCLASS_UNDEFINED,
	.bFunctionProtocol = FUNCTION_PROTOCOL_AF_VERSION_03_00,
};

/* Audio Control Interface */
static struct CH9_UsbInterfaceDescriptor acInterfaceDesc = {
	.bLength = CH9_USB_DS_INTERFACE,
	.bDescriptorType = CH9_USB_DT_INTERFACE,
	.bInterfaceNumber = 0,
	.bAlternateSetting = 0,
	.bNumEndpoints = 0,
	.bInterfaceClass = CH9_USB_CLASS_AUDIO,
	.bInterfaceSubClass = INTERFACE_SUBCLASS_AUDIOCONTROL,
	.bInterfaceProtocol = INTERFACE_PROTOCOL_IP_VERSION_03_00,
};

/* Input Terminal for I/O-Microphone */
static struct UAC3_InputTerminalDescriptor inputTerminalDesc = {
	.bLength = sizeof(struct UAC3_InputTerminalDescriptor),
	.bDescriptorType = DT_CS_INTERFACE,
	.bDescriptorSubtype = AC_DESCRIPTOR_INPUT_TERMINAL,
	.bTerminalID = MIC_IN_IT_ID, //Input Terminal ID4.
	.wTerminalType = cpuToLe16(INPUT_MICROPHONE),
	.bAssocTerminal = 0,
	.bCSourceID = USB_IN_CLK_ID, //Clock Source ID9.
	.bmControls =
	{0, 0, 0, 0},
	.wClusterDescrID = cpuToLe16(0001),
	.wExTerminalDescrID = 0,
	.wConnectorsDescrID = 0,
	.wTerminalDescrStr = 0
};

/* Output Terminal for USB_IN */
static struct UAC3_OutputTerminalDescriptor outputTerminalDesc = {
	.bLength = sizeof(struct UAC3_OutputTerminalDescriptor),
	.bDescriptorType = DT_CS_INTERFACE,
	.bDescriptorSubtype = AC_DESCRIPTOR_OUTPUT_TERMINAL,
	.bTerminalID = USB_IN_OT_ID, //Output Terminal ID6.
	.wTerminalType = cpuToLe16(TERMINAL_TYPES_STREAMING),
	.bAssocTerminal = 0,
	.bSourceID = USB_IN_FU_ID, //Connected to Feature Unit ID5.
	.bCSourceID = USB_IN_CLK_ID, //Clock Source ID9.
	.bmControls =
	{0, 0, 0, 0},
	.wExTerminalDescrID = 0,
	.wConnectorsDescrID = 0,
	.wTerminalDescrStr = 0
};

/* Feature Unit */
static struct UAC3_FeatureUnitDescriptor_mono featureDesc = {
	.bLength = sizeof(struct UAC3_FeatureUnitDescriptor_mono),
	.bDescriptorType = DT_CS_INTERFACE,
	.bDescriptorSubtype = AC_DESCRIPTOR_FEATURE_UNIT, // SPEC BUG!! ver 20 or 30 definitions should be taken? 0x6 or 0x7 for clock source
	.bUnitID = USB_IN_FU_ID, //Feature Unit ID5.
	.bSourceID = MIC_IN_IT_ID, //connected to IT ID4.
	.bmaControls_0 =
	{3, 0, 0, 0}, //Mute Control.
	.bmaControls_1 =
	{0x0C, 0, 0, 0}, //Volume Control for logical channel 1.
	.wFeatureDescrStr = 0
};

/* Clock Source */
static struct UAC3_ClockSourceDescriptor clockSourceDesc = {
	.bLength = sizeof(struct UAC3_ClockSourceDescriptor),
	.bDescriptorType = DT_CS_INTERFACE,
	.bDescriptorSubtype = AC_DESCRIPTOR_CLOCK_SOURCE, // SPEC BUG!! ver 20 or 30 definitions should be taken? 0xA or 0xB for clock source
	.bClockID = USB_IN_CLK_ID, // Clock Source ID9.
	.bmAttributes = 0, //D0 = 0b1. Internal Clock. D1 is derived from the endpoint Synchronization Type. D1 = 0b0 for Asynchronous. D1 = 0b1 for Synchronous.
	.bmControls =
	{1, 0, 0, 0},
	.bReferenceTerminal = 0,
	.wClockSourceStr = 0
};

/* Power Domain */
static struct UAC3_PowerDomainDescriptor powerDomainDesc = {
	.bLength = sizeof(struct UAC3_PowerDomainDescriptor),
	.bDescriptorType = DT_CS_INTERFACE,
	.bDescriptorSubtype = AC3_DESCRIPTOR_POWER_DOMAIN,
	.bPowerDomainID = USB_IN_PD_ID, // Power Domain ID11.
	.waRecoveryTime_1 = 0x0258, // Time to recover from D1 to D0. 30 ms, expressed in 50 s increments.
	.waRecoveryTime_2 = 0x1770, // Time to recover from D2 to D0. 300 ms, expressed in 50 s increments.
	.bNrEntities = 2,
	.baEntityID_1 = MIC_IN_IT_ID, // Input Terminal ID4.
	.baEntityID_2 = USB_IN_OT_ID, // Output Terminal ID6.
	.wPDomainDescrStr = 0
};

/* Audio Streaming IN Interface - Alt0 */
static struct CH9_UsbInterfaceDescriptor streamInterface0Desc = {
	.bLength = sizeof(struct CH9_UsbInterfaceDescriptor),
	.bDescriptorType = CH9_USB_DT_INTERFACE,
	.bAlternateSetting = 0,
	.bNumEndpoints = 0,
	.bInterfaceClass = CH9_USB_CLASS_AUDIO,
	.bInterfaceSubClass = INTERFACE_SUBCLASS_AUDIOSTREAMING,
	.bInterfaceProtocol = INTERFACE_PROTOCOL_IP_VERSION_03_00,
	.bInterfaceNumber = 1
};

/* Audio Streaming IN Interface - Alt1 */
static struct CH9_UsbInterfaceDescriptor streamInterface1Desc = {
	.bLength = sizeof(struct CH9_UsbInterfaceDescriptor),
	.bDescriptorType = CH9_USB_DT_INTERFACE,
	.bAlternateSetting = 1,
	.bNumEndpoints = 1,
	.bInterfaceClass = CH9_USB_CLASS_AUDIO,
	.bInterfaceSubClass = INTERFACE_SUBCLASS_AUDIOSTREAMING,
	.bInterfaceProtocol = INTERFACE_PROTOCOL_IP_VERSION_03_00,
	.bInterfaceNumber = 1
};

/* Class specific audio streaming interface descriptor */
static struct UAC3_ClassSpecificASInterfaceDescriptor audioStreamDesc = {
	.bLength = sizeof(struct UAC3_ClassSpecificASInterfaceDescriptor),
	.bDescriptorType = DT_CS_INTERFACE,
	.bDescriptorSubtype = AS_DESCRIPTOR_GENERAL,
	.bTerminalLink = 6, // 0x01 if this descriptor is part of the AudioStreaming OUT interface. 0x06 if this descriptor is part of the AudioStreaming IN interface.
	.bmControls =
	{0, 0, 0, 0},
	.wClusterDescrID = cpuToLe16(0x0001), // 0x0001 if the Alternate Setting of the interface supports a mono audio stream. 0x0002 if the Alternate Setting of the interface supports a stereo audio stream.
	.bmFormats =
	{1, 0, 0, 0, 0, 0, 0, 0}, // PCM
	.bSubslotSize = 0x02, // 0x02 (2 bytes) if the Alternate Setting of the interface supports 16 bit. 0x03 (3 bytes) if the Alternate Setting of the interface supports 24 bit.
	.bBitResolution = 0x10, // 0x10 (16) if the Alternate Setting of the interface supports 16 bit. 0x18 (24) if the Alternate Setting of the interface supports 24 bit.
	.bmAuxProtocols =
	{0, 0}, // No Auxiliary protocols.
	.bControlSize = 0 // No Control Channel.
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
static struct UAC3_ClassSpecificASIsochronousAudioDataEndpointDescriptor audioEpDesc = {
	.bLength = sizeof(struct UAC3_ClassSpecificASIsochronousAudioDataEndpointDescriptor),
	.bDescriptorType = DT_CS_ENDPOINT,
	.bDescriptorSubtype = AC_EP_DESCRIPTOR_GENERAL,
	.bmControls =
	{0, 0, 0, 0},
	.bLockDelayUnits = 0,
	.wLockDelay = 0,
};

/* Audio streaming class specific descriptor*/
static struct UAC3_ClassSpecificACInterfaceHeaderDescriptor acInterfaceHeaderDesc = {
	.bLength = sizeof(struct UAC3_ClassSpecificACInterfaceHeaderDescriptor),
	.bDescriptorType = DT_CS_INTERFACE,
	.bDescriptorSubtype = AC_DESCRIPTOR_HEADER,
	.bCategory = FUNCTION_SUBCLASS_MICROPHONE,
	.wTotalLength = sizeof(clockSourceDesc) +
	sizeof(inputTerminalDesc) +
	sizeof(outputTerminalDesc) +
	sizeof(featureDesc) +
	sizeof(powerDomainDesc) +
	sizeof(struct UAC3_ClassSpecificACInterfaceHeaderDescriptor),
	.bmControls =
	{1, 0, 0, 0},
};

/*
 * descriptor set for High speed configuration
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
	&audioStreamDesc,
	&epInHsDesc,
	&audioEpDesc,
	NULL
};

/*
 * descriptor set for full speed configuration
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
	&audioStreamDesc,
	&epInFsDesc,
	&audioEpDesc,
	NULL
};

/*
 * control block for SET/GET control block
 */
struct CntrlRangeLay3 {
	uint16_t wNumSubRanges;
	uint32_t dMIN;
	uint32_t dMAX;
	uint32_t dRES;
} __attribute__((packed));

/*
 * returns current frequency, volume value
 */
static int curReq(CH9_UsbSetup *ctrl)
{
	uint8_t entity_id = (ctrl->wIndex >> 8) & 0xff;
	uint8_t control_selector = ctrl->wValue >> 8;
	int length = -1;
	uint32_t r;


	switch (entity_id) {
	case USB_IN_CLK_ID:
		switch (control_selector) {
		case CS_SAM_FREQ_CONTROL:
			r = cpuToLe32((uint32_t) p_srate);
			length = ctrl->wLength < sizeof(r) ? ctrl->wLength :
				sizeof(r);
			memcpy(ep0Buff, &r, length);
			break;

		case CS_CLOCK_VALID_CONTROL:
			ep0Buff[0] = 1;
			length = ctrl->wLength < 1 ? ctrl->wLength : 1;
			break;

		default: break;
		}
		break;

	case USB_IN_FU_ID:
		// handle feature unit
		switch (control_selector) {

		case FU_MUTE_CONTROL:
			break;

		case FU_VOLUME_CONTROL:
			// check if GET control
			if (ctrl->bmRequestType & CH9_USB_DIR_DEVICE_TO_HOST) {
				length = sizeof(current_volume);
				memcpy(ep0Buff, &current_volume, length);
			} else {
				// SET volume value
				length = sizeof(current_volume);
				memcpy(&current_volume, ep0Buff, length);
			}
			break;
		default: break;
		}
		break;

	default: break;
	}

	return length;
}

/*
 * returns range frequency value
 */
static int rangeReq(CH9_UsbSetup *ctrl)
{
	uint8_t entity_id = (ctrl->wIndex >> 8) & 0xff;
	uint8_t control_selector = ctrl->wValue >> 8;
	struct CntrlRangeLay3 r;
	int length = -1;

	switch (entity_id) {
	case USB_IN_CLK_ID:
		if (control_selector == CS_SAM_FREQ_CONTROL) {
			r.dMIN = cpuToLe32((uint32_t) p_srate);
			r.dMAX = r.dMIN;
			r.dRES = 0;
			r.wNumSubRanges = cpuToLe16(1);
			length = ctrl->wLength < sizeof(r) ? ctrl->wLength :
				sizeof(r);
			memcpy(ep0Buff, &r, length);
		}
		break;

	case USB_IN_FU_ID:
		// handle feature unit
		switch (control_selector) {

		case FU_MUTE_CONTROL:
			break;

		case FU_VOLUME_CONTROL:
			r.dMIN = cpuToLe32(0x8000);
			r.dMAX = 0;
			r.dRES = 1;
			r.wNumSubRanges = cpuToLe16(1);
			length = ctrl->wLength < sizeof(r) ? ctrl->wLength :
				sizeof(r);
			memcpy(ep0Buff, &r, length);
			break;
		default: break;
		}
		break;

	default: break;
	}

	return length;
}

/*
 * handles audio class specific requests
 */
int acRequest3(CH9_UsbSetup *ctrl)
{
	if (ctrl->bRequest == REQUEST_CUR)
		return curReq(ctrl);
	else if (ctrl->bRequest == REQUEST_RANGE)
		return rangeReq(ctrl);
	else
		return -1;
}

/*
 * returns pointer to endpoint descriptor
 */
uint8_t *getEpDesc3(CH9_UsbSpeed speed)
{
	switch (speed) {

	case CH9_USB_SPEED_FULL:
		return((uint8_t*) & epInFsDesc);
		break;

	case CH9_USB_SPEED_SUPER:
	case CH9_USB_SPEED_HIGH:
		return((uint8_t*) & epInHsDesc);
		break;

	default: return NULL;
	}
}

/*
 * returns pointer to endpoint descriptor in audioDesc address, function returns
 * number of bytes of configuration for selected speed mode, or -1 if error
 */
int getDesc3(CH9_UsbSpeed speed, uint8_t * audioDesc)
{

	int i = 0, j;
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

	while (desc) {
		int length = *(uint8_t*) desc;
		memcpy(&audioDesc[sum], desc, length);
		sum += length;
		for (j = 0; j < length; j++)
			printf("0x%02X, ", ((uint8_t*) desc)[j]);
		printf("\n");
		desc = (*tab)[++i];
	}

	return sum;
}
