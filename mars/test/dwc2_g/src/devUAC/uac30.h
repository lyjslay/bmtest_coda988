/**********************************************************************
 * Copyright (C) 2016 Cadence Design Systems, Inc.
 * All rights reserved worldwide.
 ***********************************************************************
 * uac30.h
 * USB AUDIO CLASS version 3.0 header class
 *
 ***********************************************************************/

#ifndef UAC30_H
#define UAC30_H

#ifdef __cplusplus
extern "C" {
#endif

#include "auc20.h"

#define FUNCTION_PROTOCOL_AF_VERSION_03_00  INTERFACE_PROTOCOL_IP_VERSION_03_00
#define INTERFACE_PROTOCOL_IP_VERSION_03_00 0x30

#define AC3_DESCRIPTOR_AC_DESCRIPTOR_UNDEFINED     0x00
#define AC3_DESCRIPTOR_HEADER                      0x01
#define AC3_DESCRIPTOR_INPUT_TERMINAL              0x02
#define AC3_DESCRIPTOR_OUTPUT_TERMINAL             0x03
#define AC3_DESCRIPTOR_EXTENDED_TERMINAL           0x04
#define AC3_DESCRIPTOR_MIXER_UNIT                  0x05
#define AC3_DESCRIPTOR_SELECTOR_UNIT               0x06
#define AC3_DESCRIPTOR_FEATURE_UNIT                0x07
#define AC3_DESCRIPTOR_EFFECT_UNIT                 0x08
#define AC3_DESCRIPTOR_PROCESSING_UNIT             0x09
#define AC3_DESCRIPTOR_EXTENSION_UNIT              0x0A
#define AC3_DESCRIPTOR_CLOCK_SOURCE                0x0B
#define AC3_DESCRIPTOR_CLOCK_SELECTOR              0x0C
#define AC3_DESCRIPTOR_CLOCK_MULTIPLIER            0x0D
#define AC3_DESCRIPTOR_SAMPLE_RATE_CONVERTER       0x0E
#define AC3_DESCRIPTOR_CONNECTORS                  0x0F
#define AC3_DESCRIPTOR_POWER_DOMAIN                0x10

    struct UAC3_ClassSpecificACInterfaceHeaderDescriptor {
        uint8_t bLength;
        uint8_t bDescriptorType;
        uint8_t bDescriptorSubtype;
        uint8_t bCategory;
        uint16_t wTotalLength;
        uint8_t bmControls[4];
    } __attribute__((packed));

    struct UAC3_InputTerminalDescriptor {
        uint8_t bLength;
        uint8_t bDescriptorType;
        uint8_t bDescriptorSubtype;
        uint8_t bTerminalID;
        uint16_t wTerminalType;
        uint8_t bAssocTerminal;
        uint8_t bCSourceID;
        uint8_t bmControls[4];
        uint16_t wClusterDescrID;
        uint16_t wExTerminalDescrID;
        uint16_t wConnectorsDescrID;
        uint16_t wTerminalDescrStr;
    } __attribute__((packed));

    struct UAC3_OutputTerminalDescriptor {
        uint8_t bLength;
        uint8_t bDescriptorType;
        uint8_t bDescriptorSubtype;
        uint8_t bTerminalID;
        uint16_t wTerminalType;
        uint8_t bAssocTerminal;
        uint8_t bSourceID;
        uint8_t bCSourceID;
        uint8_t bmControls[4];
        uint16_t wExTerminalDescrID;
        uint16_t wConnectorsDescrID;
        uint16_t wTerminalDescrStr;
    } __attribute__((packed));

    struct UAC3_FeatureUnitDescriptor_mono {
        uint8_t bLength;
        uint8_t bDescriptorType;
        uint8_t bDescriptorSubtype;
        uint8_t bUnitID;
        uint8_t bSourceID;
        uint8_t bmaControls_0[4];
        uint8_t bmaControls_1[4];
        uint16_t wFeatureDescrStr;
    } __attribute__((packed));

    struct UAC3_FeatureUnitDescriptor_stereo {
        uint8_t bLength;
        uint8_t bDescriptorType;
        uint8_t bDescriptorSubtype;
        uint8_t bUnitID;
        uint8_t bSourceID;
        uint8_t bmaControls_0[4];
        uint8_t bmaControls_1[4];
        uint8_t bmaControls_2[4];
        uint16_t wFeatureDescrStr;
    } __attribute__((packed));

    struct UAC3_ClockSourceDescriptor {
        uint8_t bLength;
        uint8_t bDescriptorType;
        uint8_t bDescriptorSubtype;
        uint8_t bClockID;
        uint8_t bmAttributes;
        uint8_t bmControls[4];
        uint8_t bReferenceTerminal;
        uint16_t wClockSourceStr;
    } __attribute__((packed));

    struct UAC3_PowerDomainDescriptor {
        uint8_t bLength;
        uint8_t bDescriptorType;
        uint8_t bDescriptorSubtype;
        uint8_t bPowerDomainID;
        uint16_t waRecoveryTime_1;
        uint16_t waRecoveryTime_2;
        uint8_t bNrEntities;
        uint8_t baEntityID_1;
        uint8_t baEntityID_2;
        uint16_t wPDomainDescrStr;
    } __attribute__((packed));

    struct UAC3_ClassSpecificASInterfaceDescriptor {
        uint8_t bLength;
        uint8_t bDescriptorType;
        uint8_t bDescriptorSubtype;
        uint8_t bTerminalLink;
        uint8_t bmControls[4];
        uint16_t wClusterDescrID;
        uint8_t bmFormats[8];
        uint8_t bSubslotSize;
        uint8_t bBitResolution;
        uint8_t bmAuxProtocols[2];
        uint8_t bControlSize;
    } __attribute__((packed));

    struct UAC3_ClassSpecificASIsochronousAudioDataEndpointDescriptor {
        uint8_t bLength;
        uint8_t bDescriptorType;
        uint8_t bDescriptorSubtype;
        uint8_t bmControls[4];
        uint8_t bLockDelayUnits;
        uint16_t wLockDelay;
    } __attribute__((packed));

      
#ifdef __cplusplus
}
#endif

#endif /* UAC30_H */

