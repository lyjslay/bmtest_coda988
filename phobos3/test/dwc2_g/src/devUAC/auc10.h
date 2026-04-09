/**********************************************************************
 * Copyright (C) 2014 Cadence Design Systems, Inc.
 * All rights reserved worldwide.
 ***********************************************************************
 * uac10.h
 * USB AUDIO CLASS version 1.0 header class
 *
 ***********************************************************************/


#ifndef AUC10_H
#define AUC10_H

//Audio Interface Subclass Code Value
#define INTERFACE_SUBCLASS_UNDEFINED        0x00
#define INTERFACE_SUBCLASS_AUDIOCONTROL     0x01
#define INTERFACE_SUBCLASS_AUDIOSTREAMING   0x02

//Audio Class-specific Descriptor Types
#define DT_CS_UNDEFINED                     0x20
#define DT_CS_DEVICE                        0x21
#define DT_CS_CONFIGURATION                 0x22
#define DT_CS_STRING                        0x23
#define DT_CS_INTERFACE                     0x24
#define DT_CS_ENDPOINT                      0x25

// Audio Function Basic Audio Device Codes
#define AF_BASIC_CODE_UNDEFINED         0x00
#define AF_M_HP_HT1                         0x01
#define AF_M_HP_HT2                         0x02
#define AF_M_HP_HT3                         0x03
#define AF_S_HP_HT1                         0x04
#define AF_S_HP_HT2_MAUX1                   0x05
#define AF_S_HP_HT2_SAUX1                   0x06
#define AF_S_HP_HT3_MAUX1_MAUX2         0x07
#define AF_S_HP_HT3_SAUX1_MAUX2         0x08
#define AF_S_HP_HT3_MAUX1_SAUX2         0x09
#define AF_S_HP_HT3_SAUX1_SAUX2         0x0A
#define AF_M_MIC                            0x0B
#define AF_S_MIC                            0x0C
#define AF_M_HS_HS1                         0x0D
#define AF_M_HS_HS2                         0x0E
#define AF_M_HS_HS3                         0x0F
#define AF_S_HS_HS1                         0x10
#define AF_S_HS_HS2_MAUX1                   0x11
#define AF_S_HS_HS2_SAUX1                   0x12
#define AF_S_HS_HS3_MAUX1_MAUX2         0x13
#define AF_S_HS_HS3_SAUX1_MAUX2         0x14
#define AF_S_HS_HS3_MAUX1_SAUX2         0x15
#define AF_S_HS_HS3_SAUX1_SAUX2         0x16

// descriptor subtype
#define AC_DESCRIPTOR_UNDEFINED             0x00
#define AC_DESCRIPTOR_HEADER                0x01
#define AC_DESCRIPTOR_INPUT_TERMINAL        0x02
#define AC_DESCRIPTOR_OUTPUT_TERMINAL       0x03
#define AC_DESCRIPTOR_MIXER_UNIT            0x04
#define AC_DESCRIPTOR_SELECTOR_UNIT         0x05
#define AC_DESCRIPTOR_FEATURE_UNIT          0x06
#define AC_DESCRIPTOR_PROCESSING_UNIT       0x07
#define AC_DESCRIPTOR_EXTENSION_UNIT        0x08

#define AS_DESCRIPTOR_UNDEFINED             0x00
#define AS_DESCRIPTOR_GENERAL               0x01
#define AS_DESCRIPTOR_FORMAT_TYPE           0x02

#define TERMINAL_TYPES_UNDEFINED            0x0100      //I/O USB Terminal, undefined Type.
#define TERMINAL_TYPES_STREAMING            0x0101      //I/O A Terminal dealing with a signal carried over an endpoint in an AudioStreaming interface. The AudioStreaming interface descriptor points to the associated Terminal through the bTerminalLink field.
#define TERMINAL_TYPES_VENDOR_SPECIFIC      0x01FF      //I/O A Terminal dealing with a signal carried over a vendor-specific interface. The vendor-specific interface descriptor must contain a field that references the Terminal.

#define INPUT_UNDEFINED                     0x0200      //I   Input Terminal, undefined Type.
#define INPUT_MICROPHONE                    0x0201      //I   A generic microphone that does not fit under any of the other classifications.
#define INPUT_DESKTOP_MICROPHONE            0x0202      //I   A microphone normally placed on the desktop or integrated into the monitor.
#define INPUT_PERSONAL_MICROPHONE           0x0203      //I   A head-mounted or clip-on microphone.
#define INPUT_OMNI_DIRECTIONAL_MICROPHONE   0x0204      //I   A microphone designed to pick up voice from more than one speaker at relatively long ranges.
#define INPUT_MICROPHONE_ARRAY              0x0205      //I   An array of microphones designed for directional processing using host-based signal processing algorithms.
#define INPUT_PROCESSING_MICROPHONE_ARRAY   0x0206      //I   An array of microphones with an embedded signal processor.

// format type tag
#define TYPE_I_UNDEFINED                    0x0000
#define TYPE_I_PCM                          0x0001
#define TYPE_I_PCM8                         0x0002
#define TYPE_I_IEEE_FLOAT                   0x0003
#define TYPE_I_ALAW                         0x0004
#define TYPE_I_MULAW                        0x0005

// Format Type Codes
#define FORMAT_TYPE_UNDEFINED               0x00
#define FORMAT_TYPE_I                       0x01
#define FORMAT_TYPE_II                      0x02
#define FORMAT_TYPE_III                     0x03

#define AC_EP_DESCRIPTOR_UNDEFINED          0x00
#define AC_EP_DESCRIPTOR_GENERAL            0x01

// Requesst codes
#define REQUEST_CODE_UNDEFINED              0x00
#define SET_CUR                             0x01
#define GET_CUR                             0x81
#define SET_MIN                             0x02
#define GET_MIN                             0x82
#define SET_MAX                             0x03
#define GET_MAX                             0x83
#define SET_RES                             0x04
#define GET_RES                             0x84
#define SET_MEM                             0x05
#define GET_LEN                             0x85
#define GET_INFO                            0x86
#define GET_DEF                             0x87
#define GET_STAT                            0xFF

struct ClassSpecificACInterfaceHeaderDescriptor {
    uint8_t bLength; //Size of this descriptor, in bytes.
    uint8_t bDescriptorType; // CS_INTERFACE descriptor type.
    uint8_t bDescriptorSubtype; // HEADER descriptor subtype.
    uint16_t bcdADC; // Audio 1.0.
    uint16_t wTotalLength; // Total number of bytes returned for the class-specific AudioControl interface descriptor.
    uint8_t bInCollection; // One OUT AudioStreaming interface in the Collection.
    uint8_t baInterfaceNr; // Interface number of the OUT AudioStreaming interface in the Collection. Shall be one greater than the interface number of the associated AudioControl interface.
} __attribute__((packed));

struct InputTerminalDescriptor {
    uint8_t bLength; // Size of this descriptor, in bytes.
    uint8_t bDescriptorType; // CS_INTERFACE descriptor type.
    uint8_t bDescriptorSubtype; // INPUT_TERMINAL descriptor subtype.
    uint8_t bTerminalID; // ID of this Input Terminal.
    uint16_t wTerminalType; // Terminal Type.
    uint8_t bAssocTerminal; // No association.
    uint8_t bNrChannels; // Stereo signal path.
    uint16_t wChannelConfig; // Left Front and Right Front channels.
    uint8_t iChannelNames; // Unused. Shall be set to 0x00.
    uint8_t iTerminal; // Index of a string descriptor, describing the Input Terminal If no string is provided, shall be set to 0x00.
} __attribute__((packed));

struct OutputTerminalDescriptor {
    uint8_t bLength; // Size of this descriptor, in bytes.
    uint8_t bDescriptorType; // CS_INTERFACE descriptor type.
    uint8_t bDescriptorSubtype; // OUTPUT_TERMINAL descriptor subtype.
    uint8_t bTerminalID; // ID of this Input Terminal. Shall be set to 0x03.
    uint16_t wTerminalType; // Headphones Terminal Type.
    uint8_t bAssocTerminal; // No association.
    uint8_t bSourceID; //From Feature Unit ID2.
    uint8_t iTerminal; // Index of a string descriptor, describing the Output Terminal. If no string is provided, shall be set to 0x00.
} __attribute__((packed));

struct ClassSpecificASInterfaceDescriptor {
    uint8_t bLength; // Size of this descriptor, in bytes.
    uint8_t bDescriptorType; // CS_INTERFACE descriptor.
    uint8_t bDescriptorSubtype; // AS_GENERAL subtype.
    uint8_t bTerminalLink; // Terminal ID of the Input Terminal, associated with this interface.
    uint8_t bDelay; // Total interface delay, expressed in frames. Not used and shall be set to 0x00.
    uint16_t wFormatTag; // PCM Format.
} __attribute__((packed));

struct TypeIFormatTypeDescriptor {
    uint8_t bLength; // Size of this descriptor, in bytes.
    uint8_t bDescriptorType; // CS_INTERFACE descriptor.
    uint8_t bDescriptorSubtype; // FORMAT_TYPE subtype.
    uint8_t bFormatType; // FORMAT_TYPE_I.
    uint8_t bNrChannels; // Number of audio channels.
    uint8_t bSubFrameSize; // Two bytes per audio subframe.
    uint8_t bBitResolution; // Bits per sample.
    uint8_t bSamFreqType; // Number of frequencies supported.
    uint8_t tSamFreq[3]; // Table of frequencies values.
} __attribute__((packed));

struct ClassSpecificASIsochronousAudioDataEndpointDescriptor {
    uint8_t bLength; // Size of this descriptor, in bytes.
    uint8_t bDescriptorType; // CS_ENDPOINT descriptor
    uint8_t bDescriptorSubtype; // EP_GENERAL subtype.
    uint8_t bmAttributes; // Sampling frequency control, pitch control, packet padding.
    uint8_t bLockDelayUnits; // Unused. Shall be set to 0x00.
    uint16_t wLockDelay; // Unused. Shall be set to 0x0000.
} __attribute__((packed));

struct FeatureUnitDescriptor {
    uint8_t bLength; //Size of this descriptor, in bytes.
    uint8_t bDescriptorType; //CS_INTERFACE descriptor type.
    uint8_t bDescriptorSubtype; //FEATURE_UNIT descriptor subtype.
    uint8_t bUnitID; //ID of this Feature Unit
    uint8_t bSourceID; //ID of the Entity to which this Feature Unit is connected.
    uint8_t bControlSize; // 1 byte for each element of the bmaControls array.
    uint8_t bmaControls[1]; // Mute Control on Master Channel.
    uint8_t iFeature; // Index of a string descriptor, describing the Feature Unit. If no string is provided, shall be set to 0x00.
} __attribute__((packed));

#endif  /* AUC10_H */


