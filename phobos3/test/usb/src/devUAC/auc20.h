/**********************************************************************
 * Copyright (C) 2014 Cadence Design Systems, Inc.
 * All rights reserved worldwide.
 ***********************************************************************
 * uac20.h
 * USB AUDIO CLASS version 2.0 header class
 *
 ***********************************************************************/

#ifndef AUC20_H
#define AUC20_H

//Table 4-1: Audio Channel Cluster Descriptor

struct UAC2_AudioChannelClusterDescriptor {
    uint8_t bNrChannels; //1   Number  Number of logical output channels in the Terminal’s output audio channel cluster.
    uint32_t bmChannelConfig; //4   Bitmap  Describes the spatial location of the logical channels.
    uint8_t iChannelNames; //1   Index   Index of a string descriptor, describing the name of the first logical channel.
} __attribute__((packed));

//Table 4-1: Dolby Prologic Cluster Descriptor

struct UAC2_DolbyPrologicClusterDescriptor {
    uint8_t bNrChannels; //1   4   There are 4 channels in the cluster.
    uint32_t bmChannelConfig; //4   0x00000107  Front Left, Front Right, Front Center and Back Center are present.
    uint8_t iChannelNames; //1   Index   Because there are no non-predefined channels, this index must be set to 0.
} __attribute__((packed));

//Table 4-2: Left Group Cluster Descriptor

struct UAC2_LeftGroupClusteDescriptor {
    uint8_t bNrChannels; //1   5   There are 5 channels in the cluster
    uint32_t bmChannelConfig; //4   0x00000051  Front Left, Back Left , Front Left of Center and two undefined channels are present. (bNrChannels > [Number_Of_Bits_Set])
    uint8_t iChannelNames; //1   Index   Optional index of the first non-predefined string descriptor
} __attribute__((packed));

//Table 4-5: Class-Specific AC Interface Header Descriptor

struct UAC2_ClassSpecificACInterfaceHeaderDescriptor {
    uint8_t bLength; //1   Number  Size of this descriptor, in bytes: 9
    uint8_t bDescriptorType; //1   Constant    CS_INTERFACE descriptor type.
    uint8_t bDescriptorSubtype; //1   Constant    HEADER descriptor subtype.
    uint16_t bcdADC; //2   BCD Audio Device Class Specification Release Number in Binary-Coded Decimal.
    uint8_t bCategory; //1   Constant    Constant, indicating the primary use of this audio function, as intended by the manufacturer. See Appendix A.7, “Audio Function Category Codes.”
    uint16_t wTotalLength; //2   Number  Total number of bytes returned for the class-specific AudioControl interface descriptor. Includes the combined length of this descriptor header and all Clock Source, Unit and Terminal descriptors.
    uint8_t bmControls; //1   Bitmap  D1..0:    Latency Control D7..2:     Reserved. Must be set to 0.
} __attribute__((packed));

//Table 4-6: Clock Source Descriptor

struct UAC2_ClockSourceDescriptor {
    uint8_t bLength; //1   Number  Size of this descriptor, in bytes: 8
    uint8_t bDescriptorType; //1   Constant    CS_INTERFACE descriptor type.
    uint8_t bDescriptorSubtype; //1   Constant    CLOCK_SOURCE descriptor subtype.
    uint8_t bClockID; //1   Constant    Constant uniquely identifying the Clock Source Entity within the audio function. This value is used in all requests to address this Entity.
    uint8_t bmAttributes; //1   Bitmap  "D1..0:    Clock Type:External Clock 01:  Internal fixed Clock10:  Internal variable Clock11:  Internal programmable Clock D2:        Clock synchronized to SOF D7..3:    Reserved. Must be set to 0."
    uint8_t bmControls; //1   Bitmap  "D1..0:    Clock Frequency Control D3..2:    Clock Validity ControlD7..4:    Reserved. Must be set to 0."
    uint8_t bAssocTerminal; //1   Constant    Terminal ID of the Terminal that is associated with this Clock Source.
    uint8_t iClockSource; //1   Number  Index of a string descriptor, describing the Clock Source Entity.
} __attribute__((packed));

//Table 4-7: Clock Selector Descriptor

struct UAC2_ClockSelectorDescriptor {
    uint8_t bLength; // 1   Number  Size of this descriptor, in bytes: 7+p
    uint8_t bDescriptorType; //     1   Constant    CS_INTERFACE descriptor type.
    uint8_t bDescriptorSubtype; //         1   Constant    CLOCK_SELECTOR descriptor subtype.
    uint8_t bClockID; // 1   Number  Constant uniquely identifying the Clock Selector Entity within the audio function. This value is used in all requests to address this Entity.
    uint8_t bNrInPins; // bLength             1   Number  Size of this descriptor, in bytes: 7+p1   Number  Number of Input Pins of this Unit: p
    uint8_t baCSourceID[0]; //    p   Number  ID of the Clock Entity to which the first Clock Input Pin of this Clock Selector Entity is connected.
    uint8_t bmControls; // 1   Bitmap  D1..0:    Clock Selector Control D7..2:    Reserved. Must be set to 0.
    uint8_t iClockSelector; //     1   Index   Index of a string descriptor, describing the Clock Selector Entity.
} __attribute__((packed));

//Table 4-8: Clock Multiplier Descriptor

struct UAC2_ClockMultiplierDescriptor {
    uint8_t bLength; //1   Number  Size of this descriptor, in bytes: 7
    uint8_t bDescriptorType; //1   Constant    CS_INTERFACE descriptor type.
    uint8_t bDescriptorSubtype; //1   Constant    CLOCK_MULTIPLIER descriptor subtype.
    uint8_t bClockID; //1   Constant    Constant uniquely identifying the Clock Multiplier Entity within the audio function. This value is used in all requests to address this Entity.
    uint8_t bCSourceID; //1   Number  ID of the Clock Entity to which the last Clock Input Pin of this Clock Selector Entity is connected.
    uint8_t bmControls; //1   Bitmap  "D1..0:    Clock Numerator Control D3..2:    Clock Denominator ControlD7..4:    Reserved. Must be set to 0."
    uint8_t iClockMultiplier; //1   Index   Index of a string descriptor, describing the Clock Multiplier Entity.
} __attribute__((packed));

//Table 4-9: Input Terminal Descriptor

struct UAC2_InputTerminalDescriptor {
    uint8_t bLength; // 1   Number  Size of this descriptor, in bytes: 17
    uint8_t bDescriptorType; // 1   Constant    CS_INTERFACE descriptor type.
    uint8_t bDescriptorSubtype; // 1   Constant    INPUT_TERMINAL descriptor subtype.
    uint8_t bTerminalID; // 1   Constant    Constant uniquely identifying the Terminal within the audio function. This value is used in all requests to address this Terminal.
    uint16_t wTerminalType; //  2   Constant    Constant characterizing the type of Terminal. See USB Audio Terminal Types.
    uint8_t bAssocTerminal; // 1   Constant    ID of the Output Terminal to which this Input Terminal is associated.
    uint8_t bCSourceID; // 1   Constant    ID of the Clock Entity to which this Input Terminal is connected.
    uint8_t bNrChannels; // 1   Number  Number of logical output channels in the Terminal’s output audio channel cluster.
    uint32_t bmChannelConfig; //  4   Bitmap  Describes the spatial location of the logical channels.
    uint8_t iChannelNames; // 1   Index   Index of a string descriptor, describing the name of the first logical channel.
    uint16_t bmControls; //  2   Bitmap  "D1..0:    Copy Protect Control D3..2:    Connector Control D5..4:    Overload Control D7..6:    Cluster Control D9..8:    Underflow Control D11..10: Overflow ControlD15..12: Reserved. Must be set to 0."
    uint8_t iTerminal; // 1   Index   Index of a string descriptor, describing the Input Terminal.
} __attribute__((packed));

//Table 4-10: Output Terminal Descriptor

struct UAC2_OutputTerminalDescriptor {
    uint8_t bLength; // 1   Number  Size of this descriptor, in bytes: 12
    uint8_t bDescriptorType; //    1   Constant    CS_INTERFACE descriptor type.
    uint8_t bDescriptorSubtype; //        1   Constant    OUTPUT_TERMINAL descriptor subtype.
    uint8_t bTerminalID; //1   Constant    Constant uniquely identifying the Terminal within the audio function. This value is used in all requests to address this Terminal.
    uint16_t wTerminalType; //     2   Constant    Constant characterizing the type of Terminal. See USB Audio Terminal Types.
    uint8_t bAssocTerminal; //    1   Constant    Constant, identifying the Input Terminal to which this Output Terminal is associated.
    uint8_t bSourceID; //1   Constant    ID of the Unit or Terminal to which this Terminal is connected.
    uint8_t bCSourceID; //1   Constant    ID of the Clock Entity to which this Output Terminal is connected.
    uint16_t bmControls; // 2   Bitmap  "D1..0:    Copy Protect Control D3..2:    Connector Control D5..4:    Overload Control D7..6:    Underflow Control D9..8:    Overflow ControlD15..10: Reserved. Must be set to 0."
    uint8_t iTerminal; //1   Index   Index of a string descriptor, describing the Output Terminal.
} __attribute__((packed));

//Table 4-11: Mixer Unit Descriptor

struct UAC2_MixerUnitDescriptor {
    uint8_t bLength; //  1   Number  Size of this descriptor, in bytes: 13+p+N
    uint8_t bDescriptorType; //      1   Constant    CS_INTERFACE descriptor type.
    uint8_t bDescriptorSubtype; //          1   Constant    MIXER_UNIT descriptor subtype.
    uint8_t bUnitID; //  1   Number  Constant uniquely identifying the Unit within the audio function. This value is used in all requests to address this Unit.
    uint8_t bNrInPins; //  1   Number  Number of Input Pins of this Unit: p
    uint8_t baSourceID[0]; //     p   Number  ID of the Unit or Terminal to which the first Input Pin of this Mixer Unit is connected.
    uint8_t bNrChannels; //   1   Number  Number of logical output channels in the Mixer’s output audio channel cluster.
    uint32_t bmChannelConfig; //       4   Bitmap  Describes the spatial location of the logical channels.
    uint8_t iChannelNames; //     1   Index   Index of a string descriptor, describing the name of the first logical channel.
    uint8_t bmMixerControls[0]; //       N   Number  Bit map indicating which Mixer Controls are programmable.
    uint8_t bmControls; // 1   Bitmap  "D1..0:    Cluster Control D3..2:    Underflow Control D5..4:    Overflow ControlD7..6:    Reserved. Must be set to 0."
    uint8_t iMixer; //1   Index   Index of a string descriptor, describing the Mixer Unit.
} __attribute__((packed));

//Table 4-12: Selector Unit Descriptor

struct UAC2_SelectorUnitDescriptor {
    uint8_t bLength; //  1   Number  Size of this descriptor, in bytes: 7+p
    uint8_t bDescriptorType; //     1   Constant    CS_INTERFACE descriptor type.
    uint8_t bDescriptorSubtype; //         1   Constant    SELECTOR_UNIT descriptor subtype.
    uint8_t bUnitID; //  1   Number  Constant uniquely identifying the Unit within the audio function. This value is used in all requests to address this Unit.
    uint8_t bNrInPins; // 1   Number  Number of Input Pins of this Unit: p
    uint8_t baSourceID[0]; //    p   Number  ID of the Unit or Terminal to which the first Input Pin of this Selector Unit is connected.
    uint8_t bmControls; // 1   Bitmap  D1..0:    Selector Control D7..2:     Reserved. Must be set to 0.
    uint8_t iSelector; // 1   Index   Index of a string descriptor, describing the Selector Unit.
} __attribute__((packed));

//Table 4-13: Feature Unit Descriptor

struct UAC2_FeatureUnitDescriptor {
    uint8_t bLength; //1   Number  Size of this descriptor, in bytes: 6+(ch+1)*4
    uint8_t bDescriptorType; //     1   Constant    CS_INTERFACE descriptor type.
    uint8_t bDescriptorSubtype; //         1   Constant    FEATURE_UNIT descriptor subtype.
    uint8_t bUnitID; // 1   Number  Constant uniquely identifying the Unit within the audio function. This value is used in all requests to address this Unit.
    uint8_t bSourceID; // 1   Constant    ID of the Unit or Terminal to which this Feature Unit is connected.
    uint32_t bmaControls[0]; //     4*ch   Bitmap  "The Controls bitmap for master channel 0: D1..0:    Mute ControlD3..2:    Volume ControlD5..4:    Bass Control D7..6:    Mid Control D9..8:    Treble ControlD11..10: Graphic Equalizer Control D13..12: Automatic Gain Control D15..14: Delay ControlD17..16: Bass Boost Control D19..18: Loudness Control D21..20: Input Gain Control D23..22: Input Gain Pad Control D25..24: Phase Inverter Control D27..26: Underflow Control D29..28: Overfow ControlD31..30: Reserved. Must be set to 0."
    uint8_t iFeature; // 1   Index   Index of a string descriptor, describing this Feature Unit.
} __attribute__((packed));

//Table 4-14: Sampling Rate Converter Unit Descriptor

struct UAC2_SamplingRateConverterUnitDescriptor {
    uint8_t bLength; //1   Number  Size of this descriptor, in bytes: 8
    uint8_t bDescriptorType; //    1   Constant    CS_INTERFACE descriptor type.
    uint8_t bDescriptorSubtype; //        1   Constant    SAMPLE_RATE_CONVERTER descriptor subtype.
    uint8_t bUnitID; // 1   Constant    Constant uniquely identifying the Unit within the audio function. This value is used in all requests to address this Unit.
    uint8_t bSourceID; //1   Constant    ID of the Unit or Terminal to which this SRC Unit is connected.
    uint8_t bCSourceInID; //    1   Constant    ID of the Clock Entity to which this SRC Unit input section is connected.
    uint8_t bCSourceOutID; //    1   Constant    ID of the Clock Entity to which this SRC Unit output section is connected.
    uint8_t iSRC; // 1   Index   Index of a string descriptor, describing the SRC Unit.
} __attribute__((packed));

//Table 4-15: Common Part of the Effect Unit Descriptor

struct UAC2_CommonPartoftheEffectUnitDescriptor {
    uint8_t bLength; //  1   Number  Size of this descriptor, in bytes: 16+(ch*4)
    uint8_t bDescriptorType; //     1   Constant    CS_INTERFACE descriptor type.
    uint8_t bDescriptorSubtype; //         1   Constant    EFFECT_UNIT descriptor subtype.
    uint8_t bUnitID; // 1   Number  Constant uniquely identifying the Unit within the audio function. This value is used in all requests to address this Unit.
    uint16_t wEffectType; //  2   Constant    Constant identifying the type of effect this Unit is performing.
    uint8_t bSourceID; // 1   Constant    ID of the Unit or Terminal to which this Effect Unit is connected.
    uint32_t bmaControls[0]; //  ch*4   Bitmap  The Controls bitmap for master channel 0: D31..0:  Effect-specific allocation.
    uint8_t iEffects; // 1   Index   Index of a string descriptor, describing this Effect Unit.
} __attribute__((packed));

//Table 4-16: Parametric Equalizer Section Effect Unit Descriptor

struct UAC2_ParametricEqualizerSectionEffectUnitDescriptor {
    uint8_t bLength; //1   Number  Size of this descriptor, in bytes: 16+(ch*4)
    uint8_t bDescriptorType; //    1   Constant    CS_INTERFACE descriptor type.
    uint8_t bDescriptorSubtype; //        1   Constant    EFFECT_UNIT descriptor subtype.
    uint8_t bUnitID; //1   Number  Constant uniquely identifying the Unit within the audio function. This value is used in all requests to address this Unit.
    uint16_t wEffectType; // 2   Constant    PARAM_EQ_SECTION_EFFECT effect type.
    uint8_t bSourceID; //1   Constant    ID of the Unit or Terminal to which this Effect Unit is connected.
    uint32_t bmaControls[0]; //    ch*4   Bitmap  "The Controls bitmap for master channel 0: D1..0:    Enable ControlD3..2:    Center Frequency ControlD5..4:    Q Factor Control D7..6:    Gain Control D9..8:    Underflow Control D11..10: Overflow ControlD31..12: Reserved. Must be set to 0"
    uint8_t iEffects; //1   Index   Index of a string descriptor, describing this Effect Unit.
} __attribute__((packed));

//Table 4-17: Reverberation Effect Unit Descriptor

struct UAC2_ReverberationEffectUnitDescriptor {
    uint8_t bLength; //  1   Number  Size of this descriptor, in bytes: 16+(ch*4)
    uint8_t bDescriptorType; //     1   Constant    CS_INTERFACE descriptor type.
    uint8_t bDescriptorSubtype; //         1   Constant    EFFECT_UNIT descriptor subtype.
    uint8_t bUnitID; // 1   Number  Constant uniquely identifying the Unit within the audio function. This value is used in all requests to address this Unit.
    uint16_t wEffectType; //  2   Constant    REVERBERATION_EFFECT effect type.
    uint8_t bSourceID; // 1   Constant    ID of the Unit or Terminal to which this Effect Unit is connected.
    uint32_t bmaControls[0]; //     ch*4   Bitmap  "The Controls bitmap for master channel 0: D1..0:    Enable ControlD3..2:    Type ControlD5..4:    Level Control D7..6:    Time ControlD9..8:    Delay Feedback Control D11..10: Pre-Delay Control D13..12: Density Control D15..14: Hi-Freq Roll-Off Control D17..16: Underflow Control D19..18: Overflow ControlD31..20: Reserved. Must be set to 0"
    uint8_t iEffects; // 1   Index   Index of a string descriptor, describing this Effect Unit.
} __attribute__((packed));

//Table 4-18: Modulation Delay Effect Unit Descriptor

struct UAC2_ModulationDelayEffectUnitDescriptor {
    uint8_t bLength; //1   Number  Size of this descriptor, in bytes: 16+(ch*4)
    uint8_t bDescriptorType; //    1   Constant    CS_INTERFACE descriptor type.
    uint8_t bDescriptorSubtype; //        1   Constant    EFFECT_UNIT descriptor subtype.
    uint8_t bUnitID; //1   Number  Constant uniquely identifying the Unit within the audio function. This value is used in all requests to address this Unit.
    uint16_t wEffectType; // 2   Constant    MOD_DELAY_EFFECT effect type.
    uint8_t bSourceID; //1   Constant    ID of the Unit or Terminal to which this Effect Unit is connected.
    uint32_t bmaControls[0]; //    ch*4   Bitmap  "The Controls bitmap for master channel 0: D1..0:    Enable ControlD3..2:    Balance ControlD5..4:    Rate Control D7..6:    Depth Control D9..8:    Time ControlD11..10: Feedback Level Control D13..12: Underflow Control D15..14: Overflow ControlD31..16: Reserved. Must be set to 0"
    uint8_t iEffects; //1   Index   Index of a string descriptor, describing this Effect Unit.
} __attribute__((packed));

//Table 4-19: Dynamic Range Compressor Effect Unit Descriptor

struct UAC2_DynamicRangeCompressorEffectUnitDescriptor {
    uint8_t bLength; //1   Number  Size of this descriptor, in bytes: 16+(ch*4)
    uint8_t bDescriptorType; //     1   Constant    CS_INTERFACE descriptor type.
    uint8_t bDescriptorSubtype; //         1   Constant    EFFECT_UNIT descriptor subtype.
    uint8_t bUnitID; //1   Number  Constant uniquely identifying the Unit within the audio function. This value is used in all requests to address this Unit.
    uint16_t wEffectType; //  2   Constant    DYN_RANGE_COMP_EFFECT effect type.
    uint8_t bSourceID; // 1   Constant    ID of the Unit or Terminal to which this Effect Unit is connected.
    uint32_t bmaControls[0]; //     ch*4   Bitmap  "The Controls bitmap for master channel 0: D1..0:    Enable ControlD3..2:    Compression Ratio ControlD5..4:    MaxAmpl Control D7..6:    Threshold Control D9..8:    Attack Time Control D11..10: Release Time Control D13..12: Underflow Control D15..14: Overflow ControlD31..16: Reserved. Must be set to 0"
    uint8_t iEffects; // 1   Index   Index of a string descriptor, describing this Effect Unit.
} __attribute__((packed));

//Table 4-20: Common Part of the Processing Unit Descriptor

struct UAC2_CommonPartoftheProcessingUnitDescriptor {
    uint8_t bLength; //    1   Number  Size of this descriptor, in bytes: 17+p+x
    uint8_t bDescriptorType; //     1   Constant    CS_INTERFACE descriptor type.
    uint8_t bDescriptorSubtype; //         1   Constant    PROCESSING_UNIT descriptor subtype.
    uint8_t bUnitID; //   1   Number  Constant uniquely identifying the Unit within the audio function. This value is used in all requests to address this Unit.
    uint16_t wProcessType; //      2   Constant    Constant identifying the type of processing this Unit is performing.
    uint8_t bNrInPins; //  1   Number  Number of Input Pins of this Unit: p
    uint8_t baSourceID[0]; //    p*1   Number  ID of the Unit or Terminal to which the first Input Pin of this Processing Unit is connected.
    uint8_t bNrChannels; // 1   Number  Number of logical output channels in the audio channel cluster of the Processing Unit.
    uint32_t bmChannelConfig; //      4   Bitmap  Describes the spatial location of the logical channels in the audio channel cluster of the Processing Unit.
    uint8_t iChannelNames; //      1   Index   Index of a string descriptor that describes the name of the first logical channel in the audio channel cluster of the Processing Unit.
    uint16_t bmControls; //   2   Bitmap  D1..0:    Enable Control D15..2:   Process-specific allocation.
    uint8_t iProcessing; //   1   Index   Index of a string descriptor, describing this Processing Unit.
    //uint8_t Process - specific; //           x   NA  A process-specific descriptor is appended to the common descriptor. See the following paragraphs.
} __attribute__((packed));

//Table 4-21: Up/Down-mix Processing Unit Descriptor

struct UAC2_UpDownMixProcessingUnitDescriptor {
    uint8_t bLength; //1   Number  Size of this descriptor, in bytes: 18+4*m
    uint8_t bDescriptorType; //    1   Constant    CS_INTERFACE descriptor type.
    uint8_t bDescriptorSubtype; //        1   Constant    PROCESSING_UNIT descriptor subtype.
    uint8_t bUnitID; //1   Number  Constant uniquely identifying the Unit within the audio function. This value is used in all requests to address this Unit.
    uint16_t wProcessType; //     2   Constant    UP/DOWNMIX_PROCESS process type.
    uint8_t bNrInPins; //1   Number  Number of Input Pins of this Unit: 1
    uint8_t bSourceID; //1   Number  ID of the Unit or Terminal to which the Input Pin of this Processing Unit is connected.
    uint8_t bNrChannels; //1   Number  Number of logical output channels in the output channel cluster of the Processing Unit.
    uint32_t bmChannelConfig; //     4   Bitmap  Describes the spatial location of the logical channels in the output channel cluster of the Processing Unit.
    uint8_t iChannelNames; //    1   Index   Index of a string descriptor, describing the name of the first logical channel in the Processing Unit’s output channel cluster.
    uint16_t bmControls; // 2   Bitmap  "D1..0:    Enable Control D3..2:    Mode Select Control D5..4:    Cluster Control D7..6:    Underflow Control D9..8:    Overflow ControlD15..10: Reserved. Must be set to 0"
    uint8_t iProcessing; //1   Index   Index of a string descriptor, describing this Processing Unit.
    uint8_t bNrModes; //1   Number  Number of modes, supported by this Processing Unit: m
    uint32_t daModes[]; //4*m   Bitmap  Describes the active logical channels in mode 1.
} __attribute__((packed));

//Table 4-22: Dolby Prologic Processing Unit Descriptor

struct UAC2_DolbyPrologicProcessingUnitDescriptor {
    uint8_t bLength; // 1   Number  Size of this descriptor, in bytes: 18+4*m
    uint8_t bDescriptorType; //      1   Constant    CS_INTERFACE descriptor type.
    uint8_t bDescriptorSubtype; //          1   Constant    PROCESSING_UNIT descriptor subtype.
    uint8_t bUnitID; //  1   Number  Constant uniquely identifying the Unit within the audio function. This value is used in all requests to address this Unit.
    uint16_t wProcessType; //       2   Constant    DOLBY_PROLOGIC_PROCESS process type.
    uint8_t bNrInPins; //  1   Number  Number of Input Pins of this Unit: 1
    uint8_t bSourceID; //  1   Number  ID of the Unit or Terminal to which the Input Pin of this Processing Unit is connected.
    uint8_t bNrChannels; //  1   Number  Number of logical output channels in the output channel cluster of the Processing Unit.
    uint32_t bmChannelConfig; //       4   Bitmap  Describes the spatial location of the logical channels in the Processing Unit’s output channel cluster. At least Left, Right, Center and/or Surround must be set.
    uint8_t iChannelNames; //      1   Index   Index of a string descriptor, describing the name of the first logical channel in the Processing Unit’s output channel cluster.
    uint16_t bmControls; //   2   Bitmap  "D1..0:    Enable Control D3..2:    Mode Select Control D5..4:    Cluster Control D7..6:    Underflow Control D9..8:    Overflow ControlD15..10: Reserved. Must be set to 0"
    uint8_t iProcessing; //  1   Index   Index of a string descriptor, describing this Processing Unit.
    uint8_t bNrModes; //  1   Number  "Number of modes, supported by this Processing Unit: mA maximum of 3 different modes is possible."
    uint32_t daModes[]; //  4*m   Bitmap  Describes the active logical channels in mode 1.
} __attribute__((packed));

//Table 4-23: Stereo Extender Processing Unit Descriptor

struct UAC2_StereoExtenderProcessingUnitDescriptor {
    uint8_t bLength; //    1   Number  Size of this descriptor, in bytes: 17
    uint8_t bDescriptorType; //    1   Constant    CS_INTERFACE descriptor type.
    uint8_t bDescriptorSubtype; //        1   Constant    PROCESSING_UNIT descriptor subtype.
    uint8_t bUnitID; //    1   Number  Constant uniquely identifying the Unit within the audio function. This value is used in all requests to address this Unit.
    uint16_t wProcessType; //     2   Constant    "STEREO_EXTENDER_PROCESSprocess type."
    uint8_t bNrInPins; //1   Number  Number of Input Pins of this Unit: 1
    uint8_t bSourceID; //1   Number  ID of the Unit or Terminal to which the Input Pin of this Processing Unit is connected.
    uint8_t bNrChannels; //1   Number  Number of logical output channels in the Processing Unit’s output channel cluster.
    uint32_t bmChannelConfig; //     4   Bitmap  Describes the spatial location of the logical channels in the output channel cluster of the Processing Unit. At least Left and Right must be set.
    uint8_t iChannelNames; //    1   Index   Index of a string descriptor, describing the name of the first logical channel in the Processing Unit’s output channel cluster.
    uint16_t bmControls; // 2   Bitmap  "D1..0:    Enable Control D3..2:    Width Control D5..4:    Cluster Control D7..6:    Underflow Control D9..8:    Overflow ControlD15..10: Reserved. Must be set to 0"
    uint8_t iProcessing; //1   Index   Index of a string descriptor, describing this Processing Unit.
} __attribute__((packed));

//Table 4-24: Extension Unit Descriptor

struct UAC2_ExtensionUnitDescriptor {
    uint8_t bLength; // 1   Number  Size of this descriptor, in bytes: 16+p
    uint8_t bDescriptorType; //     1   Constant    CS_INTERFACE descriptor type.
    uint8_t bDescriptorSubtype; //         1   Constant    EXTENSION_UNIT descriptor subtype.
    uint8_t bUnitID; // 1   Number  Constant uniquely identifying the Unit within the audio function. This value is used in all requests to address this Unit.
    uint16_t wExtensionCode; //      2   Constant    Vendor-specific code identifying the Extension Unit.
    uint8_t bNrInPins; // 1   Number  Number of Input Pins of this Unit: p
    uint8_t baSourceID[0]; //    1*p   Number  ID of the Unit or Terminal to which the first Input Pin of this Extension Unit is connected.
    uint8_t bNrChannels; //        1   Number  Number of logical output channels in the audio channel cluster of the Extension Unit.
    uint32_t bmChannelConfig; //         4   Bitmap  Describes the spatial location of the logical channels in the audio channel cluster of the Extension Unit.
    uint8_t iChannelNames; //        1   Index   Index of a string descriptor, describing the name of the first logical channel in the audio channel cluster of the Extension Unit.
    uint8_t bmControls; //     1   Bitmap  D1..0:    Enable Control D3..2:    Cluster Control D5..4:    Underflow Control D7..6:    Overflow Control
    uint8_t iExtension; //     1   Index   Index of a string descriptor, describing this Extension Unit.
} __attribute__((packed));


//Table 4-25: Standard AC Interrupt Endpoint Descriptor

struct UAC2_StandardACInterruptEndpointDescriptor {
    uint8_t bLength; //1   Number  Size of this descriptor, in bytes: 7
    uint8_t bDescriptorType; //    1   Constant    ENDPOINT descriptor type
    uint8_t bEndpointAddress; //        1   Endpoint    "The address of the endpoint on the USB device described by this descriptor. The address is encoded as follows:D7:        Direction. 1 = IN endpoint D6..4:    Reserved, reset to zero D3..0:    The endpoint number,determined by the designer."
    uint8_t bmAttributes; //    1   Bit Map "D1..0:    Transfer Type11 = InterruptAll other bits are reserved."
    uint16_t wMaxPacketSize; //     2   Number  Maximum packet size this endpoint is capable of sending or receiving when this configuration is selected. Used here to pass 6-byte interrupt information.
    uint8_t bInterval; // 1   Number  Interval for polling the Interrupt endpoint.
} __attribute__((packed));

//Table 4-26: Standard AS Interface Descriptor

struct UAC2_StandardASInterfaceDescriptor {
    uint8_t bLength; // 1   Number  Size of this descriptor, in bytes: 9
    uint8_t bDescriptorType; //     1   Constant    INTERFACE descriptor type
    uint8_t bInterfaceNumber; //         1   Number  Number of interface. A zero-based value identifying the index in the array of concurrent interfaces supported by this configuration.
    uint8_t bAlternateSetting; //         1   Number  Value used to select an Alternate Setting for the interface identified in the prior field.
    uint8_t bNumEndpoints; //     1   Number  Number of endpoints used by this interface (excluding endpoint 0). Must be either 0 (no data endpoint), 1 (data endpoint) or 2 (data and explicit feedback endpoint).
    uint8_t bInterfaceClass; //     1   Class   AUDIO Audio Interface Class code (assigned by the USB). See Appendix A.4, “Audio Interface Class Code”
    uint8_t bInterfaceSubClass; //         1   Subclass    AUDIO_STREAMING Audio Interface Subclass code. Assigned by this specification. See Appendix A.5, “Audio Interface Subclass Codes.”
    uint8_t bInterfaceProtocol; //         1   Protocol    IP_VERSION_02_00 Interface Protocol code. Indicates the current version of the specification. See Appendix A.6, “Audio Interface Protocol Codes”
    uint8_t iInterface; //   1   Index   Index of a string descriptor that describes this interface.
} __attribute__((packed));

//Table 4-27: Class-Specific AS Interface Descriptor

struct UAC2_ClassSpecificASInterfaceDescriptor {
    uint8_t bLength; //     1   Number  Size of this descriptor in bytes: 16
    uint8_t bDescriptorType; //    1   Constant    CS_INTERFACE descriptor type.
    uint8_t bDescriptorSubtype; //        1   Constant    AS_GENERAL descriptor subtype.
    uint8_t bTerminalLink; //    1   Constant    The Terminal ID of the Terminal to which this interface is connected.
    uint8_t bmControls; //    1   Bitmap  "D1..0:    Active Alternate Setting Control D3..2:    Valid Alternate Settings ControlD7..4:    Reserved. Must be set to 0."
    uint8_t bFormatType; //    1   Constant    Constant identifying the Format Type the AudioStreaming interface is using.
    uint32_t bmFormats; // 4   Bitmap  The Audio Data Format(s) that can be used to communicate with this interface. See the USB Audio Data Formats document for further details.
    uint8_t bNrChannels; //1   Number  Number of physical channels in the AS Interface audio channel cluster.
    uint32_t bmChannelConfig; //     4   Bitmap  Describes the spatial location of the physical channels.
    uint8_t iChannelNames; //    1   Index   Index of a string descriptor, describing the name of the first physical channel.
} __attribute__((packed));

//Table 4-28: Encoder Descriptor

struct UAC2_EncoderDescriptor {
    uint8_t bLength; //    1   Number  Size of this descriptor, in bytes: 21
    uint8_t bDescriptorType; //    1   Constant    CS_INTERFACE descriptor type.
    uint8_t bDescriptorSubtype; //        1   Constant    ENCODER descriptor subtype.
    uint8_t bEncoderID; //    1   Constant    Constant uniquely identifying the encoder within the interface. This value is used in all requests to address this encoder.
    uint8_t bEncoder; //1   Constant    Constant identifying the encoder. See Appendix A.15, “Encoder Type Codes.”
    uint32_t bmControls; //      4   Bitmap  "D1..0:    Bit Rate Control D3..2:    Quality Control D5..4:    VBR Control D7..6:    Type Control D9..8:    Underflow Control D11..10: Overflow ControlD13..12: Encoder Error Control D15..14: Param1 Control D17..16: Param2 Control D19..18: Param3 Control D21..20: Param4 Control D23..22: Param5 Control D25..24: Param6 Control D27..26: Param7 Control D29..28: Param8 ControlD31..30: Reserved. Must be set to 0."
    uint8_t iParam1; // 1   Index   Index of a string descriptor, describing the purpose of Param1.
    uint8_t iParam2; // 1   Index   Index of a string descriptor, describing the purpose of Param2.
    uint8_t iParam3; // 1   Index   Index of a string descriptor, describing the purpose of Param3.
    uint8_t iParam4; // 1   Index   Index of a string descriptor, describing the purpose of Param4.
    uint8_t iParam5; // 1   Index   Index of a string descriptor, describing the purpose of Param5.
    uint8_t iParam6; // 1   Index   Index of a string descriptor, describing the purpose of Param6.
    uint8_t iParam7; // 1   Index   Index of a string descriptor, describing the purpose of Param7.
    uint8_t iParam8; // 1   Index   Index of a string descriptor, describing the purpose of Param8.
    uint8_t iEncoder; //     1   Index   Index of a string descriptor, describing the encoder.
} __attribute__((packed));

//Table 4-29: MPEG Decoder Descriptor

struct UAC2_MPEGDecoderDescriptor {
    uint8_t bLength; //1   Number  Size of this descriptor, in bytes: 10
    uint8_t bDescriptorType; //    1   Constant    CS_INTERFACE descriptor type.
    uint8_t bDescriptorSubtype; //        1   Constant    DECODER descriptor subtype.
    uint8_t bDecoderID; //1   Constant    Constant uniquely identifying the decoder within the interface. This value is used in all requests to address this decoder.
    uint8_t bDecoder; //1   Constant    MPEG_DECODER. Constant identifying the decoder. See Appendix A.16, “Decoder Type Codes.”
    uint16_t bmMPEGCapabilities; //         2   Bitmap  "Bitmap identifying the MPEG capabilities of the decoder. A bit set indicates that the capability is supported:D2..0:    Layer support:D0 = Layer I D1 = Layer II D2 = Layer IIID3:        MPEG-1 only.D4:        MPEG-1 dual-channel. D5:        MPEG-2 second stereo.D6:        MPEG-2 7.1 channel augmentation.D7:        Adaptive multi-channel prediction.D9..8:    MPEG-2 multilingual support: 00 = Not supported01 = Supported at Fs 10 = Reserved11 = Supported at Fs and ½Fs.D10:      Support for ½Fs.D15..11: Reserved. Must be set to 0."
    uint8_t bmMPEGFeatures; //    1   Bitmap  "Bitmap identifying the features the decoder supports. A bit set indicates that the feature is supported:D3..0:    Reserved.D5..4:    Internal Dynamic Range Control: 00 = not supported.01 = supported but not scalable. 10 = scalable, common boostand cut scaling value.11 = scalable, separate boost and cut scaling value.D7..6:    Reserved. Must be set to 0."
    uint8_t bmControls; //1   Bitmap  "D1..0:    Underflow Control D3..2:    Overflow Control D5..4:    Decoder Error ControlD7..6:    Reserved. Must be set to 0."
    uint8_t iDecoder; //1   Index   Index of a string descriptor, describing the decoder.
} __attribute__((packed));

//Table 4-30: AC-3 Decoder Descriptor

struct UAC2_AC3DecoderDescriptor {
    uint8_t bLength; //  1   Number  Size of this descriptor, in bytes: 12
    uint8_t bDescriptorType; //     1   Constant    CS_INTERFACE descriptor type.
    uint8_t bDescriptorSubtype; //         1   Constant    DECODER descriptor subtype.
    uint8_t bDecoderID; // 1   Constant    Constant uniquely identifying the decoder within the interface. This value is used in all requests to address this decoder.
    uint8_t bDecoder; // 1   Constant    AC-3_DECODER. Constant identifying the decoder. See Appendix A.16, “Decoder Type Codes.”
    uint32_t bmBSID; // 4   Bitmap  A bit set to 1 indicates that the corresponding BSID mode is supported.
    uint8_t bmAC3Features; //     1   Bitmap  "A bit set to 1 indicates that the mentioned feature is supported:D0:        RF modeD1:        Line modeD2:        Custom0 modeD3:        Custom1 modeD5..4:    Internal Dynamic Range Control: 00 = not supported.01 = supported but not scalable. 10 = scalable, common boostand cut scaling value.11 = scalable, separate boost and cut scaling value.D7..6:    Reserved. Must be set to 0."
    uint8_t bmControls; // 1   Bitmap  "D1..0:    Underflow Control D3..2:    Overflow Control D5..4:    Decoder Error ControlD7..6:    Reserved. Must be set to 0."
    uint8_t iDecoder; // 1   Index   Index of a string descriptor, describing the decoder.
} __attribute__((packed));

//Table 4-31: WMA Decoder Descriptor

struct UAC2_WMADecoderDescriptor {
    uint8_t bLength; //1   Number  Size of this descriptor, in bytes: 9
    uint8_t bDescriptorType; //    1   Constant    CS_INTERFACE descriptor type.
    uint8_t bDescriptorSubtype; //        1   Constant    DECODER descriptor subtype.
    uint8_t bDecoderID; //1   Constant    Constant uniquely identifying the decoder within the interface. This value is used in all requests to address this decoder.
    uint8_t bDecoder; //1   Constant    WMA_DECODER. Constant identifying the decoder. See Appendix A.16, “Decoder Type Codes.”
    uint16_t bmWMAProfile; //     2   Bitmap  "A bit set to 1 indicates that the mentioned feature is supported:D0:        WMA Profile 1, L1D1:        WMA Profile 2, L2D2:        WMA Profile 3, L3 D3:         WMA Profile Other, L           D4:        WMA Speech 1, S1 D5:        WMA Speech 2, S2 D6:        WMAPro Profile 1, M1 D7:        WMAPro Profile 2, M2 D8:        WMAPro Profile 3, M3D9:        WMAPro Profile Other, MD10:      WMA lossless decoding is supportedD15..11: Reserved. Must be set to 0."
    uint8_t bmControls; //1   Bitmap  "D1..0:    Underflow Control D3..2:    Overflow Control D5..4:    Decoder Error ControlD7..6:    Reserved. Must be set to 0."
    uint8_t iDecoder; //1   Index   Index of a string descriptor, describing the decoder.
} __attribute__((packed));

//Table 4-32: DTS Decoder Descriptor

struct UAC2_DTSDecoderDescriptor {
    uint8_t bLength; // 1   Number  Size of this descriptor, in bytes: 8
    uint8_t bDescriptorType; //     1   Constant    CS_INTERFACE descriptor type.
    uint8_t bDescriptorSubtype; //         1   Constant    DECODER descriptor subtype.
    uint8_t bDecoderID; // 1   Constant    Constant uniquely identifying the decoder within the interface. This value is used in all requests to address this decoder.
    uint8_t bDecoder; // 1   Constant    DTS_DECODER. Constant identifying the decoder. See Appendix A.16, “Decoder Type Codes.”
    uint8_t bmCapabilities; //     1   Bitmap  "A bit set to 1 indicates that the mentioned feature is supported:D0:        CoreD1:        LosslessD2:        LBRD3:        MultipleStreamMixing D4:        DualDecodeD7..5:    Reserved. Must be set to 0."
    uint8_t bmControls; // 1   Bitmap  "D1..0:    Reserved. Must be set to 0b00. D3..2:    Underflow ControlD5..4:    Overflow Control D7..6:    Decoder Error Control"
    uint8_t iDecoder; // 1   Index   Index of a string descriptor, describing the decoder.
} __attribute__((packed));

//Table 4-33: Standard AS Isochronous Audio Data Endpoint Descriptor

struct UAC2_StandardASIsochronousAudioDataEndpointDescriptor {
    uint8_t bLength; //1   Number  Size of this descriptor, in bytes: 7
    uint8_t bDescriptorType; //    1   Constant    ENDPOINT descriptor type
    uint8_t bEndpointAddress; //        1   Endpoint    "The address of the endpoint on the USB device described by this descriptor. The address is encoded as follows:D3..0:    The endpoint number, determined by the designer.D6..4:    Reserved, reset to zero D7:        Direction.0 = OUT endpoint 1 = IN endpoint"
    uint8_t bmAttributes; //    1   Bit Map "D1..0:    Transfer type01 = IsochronousD3..2:    Synchronization Type 01 = Asynchronous10 = Adaptive11 = SynchronousD5..4:    Usage Type00 = Data endpoint or10 = Implicit feedback Data endpointAll other bits are reserved."
    uint16_t wMaxPacketSize; //     2   Number  "Maximum packet size this endpoint is capable of sending or receiving when this configuration is selected.This is determined by the audio bandwidth constraints of the endpoint."
    uint8_t bInterval; //1   Number  Interval for polling endpoint for data transfers.
} __attribute__((packed));

//Table 4-34: Class-Specific AS Isochronous Audio Data Endpoint Descriptor

struct UAC2_ClassSpecificASIsochronousAudioDataEndpointDescriptor {
    uint8_t bLength; //1   Number  Size of this descriptor, in bytes: 8
    uint8_t bDescriptorType; //    1   Constant    CS_ENDPOINT descriptor type.
    uint8_t bDescriptorSubtype; //        1   Constant    EP_GENERAL descriptor subtype.
    uint8_t bmAttributes; //    1   Bit Map "Bit D7 indicates a requirement forwMaxPacketSize packets. D7:      MaxPacketsOnly"
    uint8_t bmControls; //1   Bitmap  "D1..0:    Pitch Control D3..2:       Data Overrun Control D5..4:        Data Underrun ControlD7..6:    Reserved. Must be set to 0."
    uint8_t bLockDelayUnits; //    1   Number  "Indicates the units used for the wLockDelay field:0:          Undefined1:          Milliseconds2:          Decoded PCM samples 3..255:  Reserved"
    uint16_t wLockDelay; // 2   Number  Indicates the time it takes this endpoint to reliably lock its internal clock recovery circuitry. Units used depend on the value of the bLockDelayUnits field.
} __attribute__((packed));

//Table 4-35: Standard AS Isochronous Feedback Endpoint Descriptor

struct UAC2_StandardASIsochronousFeedbackEndpointDescriptor {
    uint8_t bLength; // 1   Number  Size of this descriptor, in bytes: 7
    uint8_t bDescriptorType; //    1   Constant    ENDPOINT descriptor type.
    uint8_t bEndpointAddress; //        1   Endpoint    "The address of the endpoint on the USB device described by this descriptor. The address is encoded as follows:D3..0:    The endpoint number, determined by the designer.D6..4:    Reserved, reset to zero D7:        Direction.0 = OUT endpoint 1 = IN endpoint"
    uint8_t bmAttributes; //    1   Bit Map "D1..0:    Transfer type01 = IsochronousD3..2:    Synchronization Type 00 = No SynchronizationD5..4:    Usage Type01 = Feedback endpoint All other bits are reserved."
    uint16_t wMaxPacketSize; //     2   Number  Maximum packet size this endpoint is capable of sending or receiving when this configuration is selected.
    uint8_t bInterval; //1   Number  Interval for polling endpoint for data transfers.
} __attribute__((packed));

//Table 5-2: 1-byte Control CUR Parameter Block
//wLength                     1

struct UAC2_ControlCURParameterBlockB {
    uint8_t bCUR; //1   Number      The setting for the CUR attribute of the addressed Control
} __attribute__((packed));

//Table 5-3: 1-byte Control RANGE Parameter Block
//wLength                     2+3*n

/*
0       wNumSubRanges       2   Number      The number of subranges of the addressed Control: n
2       bMIN(1)             1   Number      The setting for the MIN attribute of the first subrange of the addressed Control
3       bMAX(1)             1   Number      The setting for the MAX attribute of the first subrange of the addressed Control
4       bRES(1)             1   Number      The setting for the RES attribute of the first subrange of the addressed Control
…       …               …   …       …
2+3*(n-1)       bMIN(n)             1   Number      The setting for the MIN attribute of the last subrange of the addressed Control
3+3*(n-1)       bMAX(n)             1   Number      The setting for the MAX attribute of the last subrange of the addressed Control
4+3*(n-1)       bRES(n)             1   Number      The setting for the RES attribute of the last subrange of the addressed Control
 */
struct UAC2_ControlRANGEParameterBlockB {
    uint16_t wNumSubRanges;
    uint8_t bMinMaxRes[][3];
} __attribute__((packed));


//Table 5-4: 2-byte Control CUR Parameter Block
//wLength                     2

struct UAC2_ControlCURParameterBlockW {
    uint16_t wCUR; //2   Number      The setting for the CUR attribute of the addressed Control
} __attribute__((packed));

//Table 5-5: 2-byte Control RANGE Parameter Block

/*
wLength                     2+6*n
0       wNumSubRanges               2   Number      The number of subranges of the addressed Control: n
2       wMIN(1)             2   Number      The setting for the MIN attribute of the first subrange of the addressed Control
4       wMAX(1)             2   Number      The setting for the MAX attribute of the first subrange of the addressed Control
6       wRES(1)             2   Number      The setting for the RES attribute of the first subrange of the addressed Control
…       …               …   …       …
2+6*(n-1)       wMIN(n)             2   Number      The setting for the MIN attribute of the last subrange of the addressed Control
4+6*(n-1)       wMAX(n)             2   Number      The setting for the MAX attribute of the last subrange of the addressed Control
6+6*(n-1)       wRES(n)             2   Number      The setting for the RES attribute of the last subrange of the addressed Control
 */
struct UAC2_ControlRANGEParameterBlockW {
    uint16_t wNumSubRanges;
    uint16_t wMinMaxRes[][3];
} __attribute__((packed));


//Table 5-6: 4-byte Control CUR Parameter Block
//wLength                     4

struct UAC2_ControlCURParameterBlockDW {
    uint32_t dCUR; //4   Number      The setting for the CUR attribute of the addressed Control
} __attribute__((packed));

//Table 5-7: 4-byte Control RANGE Parameter Block
//wLength                     2+12*n

/*
struct UAC2_ControlRANGEParameterBlockDW {
0       wNumSubRanges               2   Number      The number of subranges of the addressed Control: n
2       dMIN (1)                4   Number      The setting for the MIN attribute of the first subrange of addressed Control
6       dMAX (1)                4   Number      The setting for the MAX attribute of the first subrange of the addressed Control
10      dRES (1)                4   Number      The setting for the RES attribute of the first subrange of the addressed Control
…       …               …   …       …
2+12*(n-1)      dMIN(n)             4   Number      The setting for the MIN attribute of the last subrange of the addressed Control
6+12*(n-1)      dMAX(n)             4   Number      The setting for the MAX attribute of the last subrange of the addressed Control
10+12*(n-1)     dRES(n)             4   Number      The setting for the RES attribute of the last subrange of addressed Control
} __attribute__ ((packed));
 */
struct UAC2_ControlRANGEParameterBlockDW {
    uint16_t wNumSubRanges;
    uint32_t dMinMaxRes[][3];
} __attribute__((packed));

#define DECLARE_ControlRANGEParameterBlockDW(n) \
struct UAC2_ControlRANGEParameterBlockDW_##n {       \
    uint16_t wNumSubRanges;                     \
    uint32_t dMinMaxRes[n][3];                  \
} __attribute__((packed))

//Table 5-8: Cluster Control CUR Parameter Block
//wLength                     6

struct UAC2_ClusterControlCURParameterBlock {
    uint8_t bNrChannels; //             1   Number      Number of logical output channels in the Entity’s output audio channel cluster.
    uint32_t bmChannelConfig; //             4   Bitmap      Describes the spatial location of the logical channels.
    uint8_t iChannelNames; //               1   Index       Index of a string descriptor, describing the name of the first logical channel.
} __attribute__((packed));

//Table 5-9: Error Codes
//Error Code          Description
//-32768 .. -1            Reserved for Vendor Extensions.
#define ERROR_NO_ERROR                             0
#define ERROR_OUT_OF_MEMORY                        1
#define ERROR_OUT_OF_BANDWIDTH                     2
#define ERROR_OUT_OF_PROCESSING_CYCLES             3
#define ERROR_GENERAL_FORMAT_FRAME_ERROR           4
#define ERROR_FORMAT_FRAME_TOO_SMALL               5
#define ERROR_FORMAT_FRAME_TOO_LARGE               6
#define ERROR_BAD_DATA_FORMAT                      7
#define ERROR_INCORRECT_NUMBER_OF_CHANNELS         8
#define ERROR_INCORRECT_SAMPLING_RATE              9
#define ERROR_UNABLE_TO_MEET_TARGET_BITRATE        10
#define ERROR_INCONSISTENT_SET_OF_PARAMETERS       11
#define ERROR_NOT_READY                            12
#define ERROR_BUSY                                 13
//14 .. 32767         Reserved for future USB Audio Device Class Extensions.

//Table 5-10: Connector Control CUR Parameter Block

struct UAC2_ConnectorControlCURParameterBlock {
    uint8_t bNrChannels; //             1   Number      Total number of physical channels that are currently inserted.
    uint32_t bmChannelConfig; //             4   Bitmap      Describes the predefined spatial locations of the currently inserted channels.
    uint8_t iChannelNames; //               1   Index       Index of a string descriptor, describing the name of the first inserted channel with a non-predefined spatial location.
} __attribute__((packed));

//Table 5-11: Band Numbers and Center Frequencies (ANSI S1.11-1986 Standard)
#define BAND_25Hz            14
#define BAND_31_5Hz          15
#define BAND_40Hz            16
#define BAND_50Hz            17
#define BAND_63Hz            18
#define BAND_80Hz            19
#define BAND_100Hz           20
#define BAND_125Hz           21
#define BAND_160Hz           22
#define BAND_200Hz           23
#define BAND_250Hz           24
#define BAND_315Hz           25
#define BAND_400Hz           26
#define BAND_500Hz           27
#define BAND_630Hz           28
#define BAND_800Hz           29
#define BAND_1000Hz          30
#define BAND_1250Hz          31
#define BAND_1600Hz          32
#define BAND_2000Hz          33
#define BAND_2500Hz          34
#define BAND_3150Hz          35
#define BAND_4000Hz          36
#define BAND_5000Hz          37
#define BAND_6300Hz          38
#define BAND_8000Hz          39
#define BAND_10000Hz         40
#define BAND_12500Hz         41
#define BAND_16000Hz         42
#define BAND_20000Hz         43

//Table 5-12: Graphic Equalizer Control CUR Parameter Block

/*
wLength                     4+(number of bits set in bmBandsPresent : NrBits)
0       bmBandsPresent              4   Bit Map     "A bit set indicates the band is present:D0:Band 14 is present D1:Band 15 is present ... D29:Band 43 is present D30:Reserved D31:Reserved"
4       bCUR(Lowest)                1   Number      "The setting for the CUR attribute of the lowest band present:0x7F: +31.75dB 0x7E:+31.50 dB…0x00:0.00 dB…0x82:-31.50 dB0x81:-31.75 dB0x80:-32.00 dB"
…       …               …   …       …
4+(NrBits-1)        bCUR(Highest)               1   Number      The setting for the CUR attribute of the highest band present.
 */

struct UAC2_GraphicEqualizerControlCURParameterBlock {
    uint32_t bmBandsPresent;
    uint8_t bCUR[];
} __attribute__((packed));

//Table 5-13: Graphic Equalizer Control RANGE Parameter Block

/*
wLength                     2+3*n
0       wNumSubRanges               2   Number      The number of subranges of the Graphic Equalizer Control: n
2       bMIN(1)             1   Number      The setting for the MIN attribute of the first subrange of the Graphic Equalizer Control
3       bMAX(1)             1   Number      The setting for the MAX attribute of the first subrange of the Graphic Equalizer Control
4       bRES(1)             1   Number      The setting for the RES attribute of the first subrange of the Graphic Equalizer Control
…       …               …   …       …
2+3*(n-1)       bMIN(n)             1   Number      The setting for the MIN attribute of the last subrange of the Graphic Equalizer Control
3+3*(n-1)       bMAX(n)             1   Number      The setting for the MAX attribute of the last subrange of the Graphic Equalizer Control
4+3*(n-1)       bRES(n)             1   Number      The setting for the RES attribute of the last subrange of the Graphic Equalizer Control
 */

struct UAC2_GraphicEqualizerControlRANGEParameterBlock {
    uint16_t wNumSubRanges;
    uint8_t bMinMaxRes [][3];
} __attribute__((packed));

//Table 5-14: Valid Alternate Settings Control CUR Parameter Block
//wLength                     1+n

struct UAC2_ValidAlternateSettingsControlCURParameterBlock {
    uint8_t bControlSize; //                1   Number      Indicates the number of bytes in the bmValidAltSettings bit map: n
    uint8_t bmValidAltSettings; //          n   Bit Map     Each set bit represents a currently valid Alternate Setting for the interface.
} __attribute__((packed));

//Table 5-15: High/Low Scaling Control CUR Parameter Block
//wLength                     2

struct UAC2_HighLowScalingControlCURParameterBlock {
    uint8_t bCUR_Lo; //             1   Number      The setting for the CUR attribute of the low level Scaling Control.
    uint8_t bCUR_Hi; //             1   Number      The setting for the CUR attribute of the high level Scaling Control.
} __attribute__((packed));


//Table 5-16: High/Low Scaling Control RANGE Parameter Block
//wLength                     2+3*n

/*
0       wNumSubRanges               2   Number      The number of subranges of the High/Low Scaling Control: n
2       bMIN(1)             1   Number      The setting for the MIN attribute of the first subrange of the High/Low Scaling Control
3       bMAX(1)             1   Number      The setting for the MAX attribute of the first subrange of the High/Low Scaling Control
4       bRES(1)             1   Number      The setting for the RES attribute of the first subrange of the High/Low Scaling Control
…       …               …   …       …
2+3*(n-1)       bMIN(n)             1   Number      The setting for the MIN attribute of the last subrange of the High/Low Scaling Control
3+3*(n-1)       bMAX(n)             1   Number      The setting for the MAX attribute of the last subrange of the High/Low Scaling Control
4+3*(n-1)       bRES(n)             1   Number      The setting for the RES attribute of the last subrange of the High/Low Scaling Control
 */
struct UAC2_HighLowScalingControlRANGEParameterBlock {
    uint16_t wNumSubRanges;
    uint8_t bMinMaxRes [][3];
} __attribute__((packed));

//Table 5-17: High/Low Scaling Control CUR Parameter Block, identical to 5-15, bug in spec?
//wLength                     2
/*
struct UAC2_HighLowScalingControlCURParameterBlock {
    uint8_t bCUR_Lo; //             1   Number      The setting for the CUR attribute of the low level Scaling Control.
    uint8_t bCUR_Hi; //             1   Number      The setting for the CUR attribute of the high level Scaling Control.
} __attribute__ ((packed));
 */

//Table 5-18: High/Low Scaling Control RANGE Parameter Block identical to 5-16 ?? bug in spec????
/*
wLength                     2+3*n
0       wNumSubRanges               2   Number      The number of subranges of the High/Low Scaling Control: n
2       bMIN(1)             1   Number      The setting for the MIN attribute of the first subrange of the High/Low Scaling Control
3       bMAX(1)             1   Number      The setting for the MAX attribute of the first subrange of the High/Low Scaling Control
4       bRES(1)             1   Number      The setting for the RES attribute of the first subrange of the High/Low Scaling Control
…       …               …   …       …
2+3*(n-1)       bMIN(n)             1   Number      The setting for the MIN attribute of the last subrange of the High/Low Scaling Control
3+3*(n-1)       bMAX(n)             1   Number      The setting for the MAX attribute of the last subrange of the High/Low Scaling Control
4+3*(n-1)       bRES(n)             1   Number      The setting for the RES attribute of the last subrange of the High/Low Scaling Control
 */

//Table 5-19: Memory Request Values
/*
bmRequest Type      bRequest   wValue          wIndex          wLength                    Data
"00100001B          MEM       Offset    Entity ID and Interface Length of parameter block
"00100010B
10100010B"                              Endpoint
 */

//Table 6-1: Interrupt Data Message Format

struct UAC2_InterruptDataMessageFormat {
    uint8_t bInfo; //1 Bitmap      "D0:        Vendor-specific. D1:         Interface or EndpointD7..2:    Reserved for future extensions.Must be set to 0."
    uint8_t bAttribute; //1 Constant        The attribute that caused the interrupt
    uint16_t wValue; //2 Number      CS in the high byte and CN or MCN in the low byte.
    uint16_t wIndex; //2 Number      Entity ID or zero in the high byte and Interface or Endpoint in the low byte.
} __attribute__((packed));

//Table A-1: Audio Function Class Code
//Audio Function Class Code Value
#define AUDIO_FUNCTION  AUDIO

//Table A-2: Audio Function Subclass Codes
//Audio Function Subclass Code  Value
#define FUNCTION_SUBCLASS_UNDEFINED 0x00

//Table A-3: Audio Function Protocol Codes
//Audio Function Protocol Code  Value
#define FUNCTION_PROTOCOL_UNDEFINED 0x00
#define FUNCTION_PROTOCOL_AF_VERSION_02_00  INTERFACE_PROTOCOL_IP_VERSION_02_00

//Table A-4: Audio Interface Class Code
//Audio Interface Class Code    Value
#define AUDIO   0x01

//Table A-5: Audio Interface Subclass Codes
//Audio Interface Subclass Code Value
#define INTERFACE_SUBCLASS_UNDEFINED        0x00
#define INTERFACE_SUBCLASS_AUDIOCONTROL     0x01
#define INTERFACE_SUBCLASS_AUDIOSTREAMING   0x02
#define INTERFACE_SUBCLASS_MIDISTREAMING    0x03

//Table A-6: Audio Interface Protocol Codes
//Audio Interface Protocol Code Value
#define INTERFACE_PROTOCOL_UNDEFINED        0x00
#define INTERFACE_PROTOCOL_IP_VERSION_02_00 0x20

//Table A-7: Audio Function Category Codes
//Audio Function Subclass Code  Value
#define FUNCTION_SUBCLASS_UNDEFINED              0x00
#define FUNCTION_SUBCLASS_DESKTOP_SPEAKER        0x01
#define FUNCTION_SUBCLASS_HOME_THEATER           0x02
#define FUNCTION_SUBCLASS_MICROPHONE             0x03
#define FUNCTION_SUBCLASS_HEADSET                0x04
#define FUNCTION_SUBCLASS_TELEPHONE              0x05
#define FUNCTION_SUBCLASS_CONVERTER              0x06
#define FUNCTION_SUBCLASS_VOICE_SOUND_RECORDER   0x07
#define FUNCTION_SUBCLASS_IO_BOX                 0x08
#define FUNCTION_SUBCLASS_MUSICAL_INSTRUMENT     0x09
#define FUNCTION_SUBCLASS_PRO_AUDIO              0x0A
#define FUNCTION_SUBCLASS_AUDIO_VIDEO            0x0B
#define FUNCTION_SUBCLASS_CONTROL_PANEL          0x0C
#define FUNCTION_SUBCLASS_OTHER                  0xFF

//Table A-8: Audio Class-specific Descriptor Types
//Descriptor Type   Value
#define DT_CS_UNDEFINED                    0x20
#define DT_CS_DEVICE                       0x21
#define DT_CS_CONFIGURATION                0x22
#define DT_CS_STRING                       0x23
#define DT_CS_INTERFACE                    0x24
#define DT_CS_ENDPOINT                     0x25

//Table A-9: Audio Class-Specific AC Interface Descriptor Subtypes
//Descriptor Subtype    Value
#define AC_DESCRIPTOR_UNDEFINED         0x00
#define AC_DESCRIPTOR_HEADER                          0x01
#define AC_DESCRIPTOR_INPUT_TERMINAL                  0x02
#define AC_DESCRIPTOR_OUTPUT_TERMINAL                 0x03
#define AC_DESCRIPTOR_MIXER_UNIT                      0x04
#define AC_DESCRIPTOR_SELECTOR_UNIT                   0x05
#define AC_DESCRIPTOR_FEATURE_UNIT                    0x06
#define AC_DESCRIPTOR_EFFECT_UNIT                     0x07
#define AC_DESCRIPTOR_PROCESSING_UNIT                 0x08
#define AC_DESCRIPTOR_EXTENSION_UNIT                  0x09
#define AC_DESCRIPTOR_CLOCK_SOURCE                    0x0A
#define AC_DESCRIPTOR_CLOCK_SELECTOR                  0x0B
#define AC_DESCRIPTOR_CLOCK_MULTIPLIER                0x0C
#define AC_DESCRIPTOR_SAMPLE_RATE_CONVERTER           0x0D

//Table A-10: Audio Class-Specific AS Interface Descriptor Subtypes
//Descriptor Subtype    Value
#define AS_DESCRIPTOR_UNDEFINED         0x00
#define AS_DESCRIPTOR_GENERAL           0x01
#define AS_DESCRIPTOR_FORMAT_TYPE       0x02
#define AS_DESCRIPTOR_ENCODER           0x03
#define AS_DESCRIPTOR_ DECODER          0x04

//Table A-11: Effect Unit Effect Types
//wEffectType   Value
#define EFFECT_UNDEFINED                0x00
#define EFFECT_PARAM_EQ_SECTION_EFFECT  0x01
#define EFFECT_REVERBERATION_EFFECT     0x02
#define EFFECT_MOD_DELAY_EFFECT         0x03
#define EFFECT_DYN_RANGE_COMP_EFFECT    0x04

//Table A-12: Processing Unit Process Types
//wProcessType  Value
#define PROCESS_UNDEFINED               0x00
#define PROCESS_UP_DOWNMIX_PROCESS      0x01
#define PROCESS_DOLBY_PROLOGIC_PROCESS  0x02
#define PROCESS_STEREO_EXTENDER_PROCESS 0x03

//Table A-13: Audio Class-Specific Endpoint Descriptor Subtypes
//Descriptor Subtype    Value
#define AC_EP_DESCRIPTOR_UNDEFINED      0x00
#define AC_EP_DESCRIPTOR_GENERAL        0x01

//Table A-14: Audio Class-Specific Request Codes
//Class-Specific Request Code   Value
#define REQUEST_CODE_UNDEFINED          0x00
#define REQUEST_CUR                     0x01
#define REQUEST_RANGE                   0x02
#define REQUEST_MEM                     0x03

//Table A-15: Encoder Type Codes
#define ENCODER_UNDEFINED               0x00
#define ENCODER_OTHER                   0x01
#define ENCODER_MPEG                    0x02
#define ENCODER_AC3                     0x03
#define ENCODER_WMA                     0x04
#define ENCODER_DTS                     0x05

//Table A-16: Decoder Type Codes
#define DECODER_UNDEFINED               0x00
#define DECODER_OTHER                   0x01
#define DECODER_MPEG                    0x02
#define DECODER_AC3                     0x03
#define DECODER_WMA                     0x04
#define DECODER_DTS                     0x05

//Table A-17: Clock Source Control Selectors
#define CS_CONTROL_UNDEFINED            0x00
#define CS_SAM_FREQ_CONTROL             0x01
#define CS_CLOCK_VALID_CONTROL          0x02

//Table A-18: Clock Selector Control Selectors
#define CX_CONTROL_UNDEFINED            0x00
#define CX_CLOCK_SELECTOR_CONTROL       0x01

//Table A-19: Clock Multiplier Control Selectors
#define CM_CONTROL_UNDEFINED            0x00
#define CM_NUMERATOR_CONTROL            0x01
#define CM_DENOMINATOR_CONTROL          0x02

//Table A-20: Terminal Control Selectors
#define TE_CONTROL_UNDEFINED            0x00
#define TE_COPY_PROTECT_CONTROL         0x01
#define TE_CONNECTOR_CONTROL            0x02
#define TE_OVERLOAD_CONTROL             0x03
#define TE_CLUSTER_CONTROL              0x04
#define TE_UNDERFLOW_CONTROL            0x05
#define TE_OVERFLOW_CONTROL             0x06
#define TE_LATENCY_CONTROL              0x07

//Table A-21: Mixer Control Selectors
#define MU_CONTROL_UNDEFINED            0x00
#define MU_MIXER_CONTROL                0x01
#define MU_CLUSTER_CONTROL              0x02
#define MU_UNDERFLOW_CONTROL            0x03
#define MU_OVERFLOW_CONTROL             0x04
#define MU_LATENCY_CONTROL              0x05

//Table A-22: Selector Control Selectors
#define SU_CONTROL_UNDEFINED            0x00
#define SU_SELECTOR_CONTROL             0x01
#define SU_LATENCY_CONTROL              0x02

//Table A-23: Feature Unit Control Selectors
#define FU_CONTROL_UNDEFINED            0x00
#define FU_MUTE_CONTROL                 0x01
#define FU_VOLUME_CONTROL               0x02
#define FU_BASS_CONTROL                 0x03
#define FU_MID_CONTROL                  0x04
#define FU_TREBLE_CONTROL               0x05
#define FU_GRAPHIC_EQUALIZER_CONTROL    0x06
#define FU_AUTOMATIC_GAIN_CONTROL       0x07
#define FU_DELAY_CONTROL                0x08
#define FU_BASS_BOOST_CONTROL           0x09
#define FU_LOUDNESS_CONTROL             0x0A
#define FU_INPUT_GAIN_CONTROL           0x0B
#define FU_INPUT_GAIN_PAD_CONTROL       0x0C
#define FU_PHASE_INVERTER_CONTROL       0x0D
#define FU_UNDERFLOW_CONTROL            0x0E
#define FU_OVERFLOW_CONTROL             0x0F
#define FU_LATENCY_CONTROL              0x10

//Table A-24: Reverberation Effect Unit Control Selectors
#define PE_CONTROL_UNDEFINED            0x00
#define PE_ENABLE_CONTROL               0x01
#define PE_CENTERFREQ_CONTROL           0x02
#define PE_QFACTOR_CONTROL              0x03
#define PE_GAIN_CONTROL                 0x04
#define PE_UNDERFLOW_CONTROL            0x05
#define PE_OVERFLOW_CONTROL             0x06
#define PE_LATENCY_CONTROL              0x07

//Table A-25: Reverberation Effect Unit Control Selectors
#define RV_CONTROL_UNDEFINED            0x00
#define RV_ENABLE_CONTROL               0x01
#define RV_TYPE_CONTROL                 0x02
#define RV_LEVEL_CONTROL                0x03
#define RV_TIME_CONTROL                 0x04
#define RV_FEEDBACK_CONTROL             0x05
#define RV_PREDELAY_CONTROL             0x06
#define RV_DENSITY_CONTROL              0x07
#define RV_HIFREQ_ROLLOFF_CONTROL       0x08
#define RV_UNDERFLOW_CONTROL            0x09
#define RV_OVERFLOW_CONTROL             0x0A
#define RV_LATENCY_CONTROL              0x0B

//Table A-26: Modulation Delay Effect Unit Control Selectors
#define MD_CONTROL_UNDEFINED            0x00
#define MD_ENABLE_CONTROL               0x01
#define MD_BALANCE_CONTROL              0x02
#define MD_RATE_CONTROL                 0x03
#define MD_DEPTH_CONTROL                0x04
#define MD_TIME_CONTROL                 0x05
#define MD_FEEDBACK_CONTROL             0x06
#define MD_UNDERFLOW_CONTROL            0x07
#define MD_OVERFLOW_CONTROL             0x08
#define MD_LATENCY_CONTROL              0x09

//Table A-27: Dynamic Range Compressor Effect Unit Control Selectors
#define DR_CONTROL_UNDEFINED            0x00
#define DR_ENABLE_CONTROL               0x01
#define DR_COMPRESSION_RATE_CONTROL     0x02
#define DR_MAXAMPL_CONTROL              0x03
#define DR_THRESHOLD_CONTROL            0x04
#define DR_ATTACK_TIME_CONTROL          0x05
#define DR_RELEASE_TIME_CONTROL         0x06
#define DR_UNDERFLOW_CONTROL            0x07
#define DR_OVERFLOW_CONTROL             0x08
#define DR_LATENCY_CONTROL              0x09

//Table A-28: Up/Down-mix Processing Unit Control Selectors
#define UD_CONTROL_UNDEFINED            0x00
#define UD_ENABLE_CONTROL               0x01
#define UD_MODE_SELECT_CONTROL          0x02
#define UD_CLUSTER_CONTROL              0x03
#define UD_UNDERFLOW_CONTROL            0x04
#define UD_OVERFLOW_CONTROL             0x05
#define UD_LATENCY_CONTROL              0x06

//Table A-29: Dolby Prologic Processing Unit Control Selectors
#define DP_CONTROL_UNDEFINED            0x00
#define DP_ENABLE_CONTROL               0x01
#define DP_MODE_SELECT_CONTROL          0x02
#define DP_CLUSTER_CONTROL              0x03
#define DP_UNDERFLOW_CONTROL            0x04
#define DP_OVERFLOW_CONTROL             0x05
#define DP_LATENCY_CONTROL              0x06

//Table A-30: Stereo Extender Processing Unit Control Selectors
#define ST_EXT_CONTROL_UNDEFINED        0x00
#define ST_EXT_ENABLE_CONTROL           0x01
#define ST_EXT_WIDTH_CONTROL            0x02
#define ST_EXT_UNDERFLOW_CONTROL        0x03
#define ST_EXT_OVERFLOW_CONTROL         0x04
#define ST_EXT_LATENCY_CONTROL          0x05

//Table A-31: Extension Unit Control Selectors
#define XU_CONTROL_UNDEFINED            0x00
#define XU_ENABLE_CONTROL               0x01
#define XU_CLUSTER_CONTROL              0x02
#define XU_UNDERFLOW_CONTROL            0x03
#define XU_OVERFLOW_CONTROL             0x04
#define XU_LATENCY_CONTROL              0x05

//Table A-32: AudioStreaming Interface Control Selectors
#define AS_CONTROL_UNDEFINED            0x00
#define AS_ACT_ALT_SETTING_CONTROL      0x01
#define AS_VAL_ALT_SETTINGS_CONTROL     0x02
#define AS_AUDIO_DATA_FORMAT_CONTROL    0x03

//Table A-33: Encoder Control Selectors
#define EN_CONTROL_UNDEFINED            0x00
#define EN_BIT_RATE_CONTROL             0x01
#define EN_QUALITY_CONTROL              0x02
#define EN_VBR_CONTROL                  0x03
#define EN_TYPE_CONTROL                 0x04
#define EN_UNDERFLOW_CONTROL            0x05
#define EN_OVERFLOW_CONTROL             0x06
#define EN_ENCODER_ERROR_CONTROL        0x07
#define EN_PARAM1_CONTROL               0x08
#define EN_PARAM2_CONTROL               0x09
#define EN_PARAM3_CONTROL               0x0A
#define EN_PARAM4_CONTROL               0x0B
#define EN_PARAM5_CONTROL               0x0C
#define EN_PARAM6_CONTROL               0x0D
#define EN_PARAM7_CONTROL               0x0E
#define EN_PARAM8_CONTROL               0x0F

//Table A-34: MPEG Decoder Control Selectors
#define MD_CONTROL_UNDEFINED            0x00
#define MD_DUAL_CHANNEL_CONTROL         0x01
#define MD_SECOND_STEREO_CONTROL        0x02
#define MD_MULTILINGUAL_CONTROL         0x03
#define MD_DYN_RANGE_CONTROL            0x04
#define MD_SCALING_CONTROL              0x05
#define MD_HILO_SCALING_CONTROL         0x06
#define MD_UNDERFLOW_CONTROL            0x07
#define MD_OVERFLOW_CONTROL             0x08
#define MD_DECODER_ERROR_CONTROL        0x09

//Table A-35: AC-3 Decoder Control Selectors
#define AD_CONTROL_UNDEFINED            0x00
#define AD_MODE_CONTROL                 0x01
#define AD_DYN_RANGE_CONTROL            0x02
#define AD_SCALING_CONTROL              0x03
#define AD_HILO_SCALING_CONTROL         0x04
#define AD_UNDERFLOW_CONTROL            0x05
#define AD_OVERFLOW_CONTROL             0x06
#define AD_DECODER_ERROR_CONTROL        0x07

//Table A-36: WMA Decoder Control Selectors
#define WD_CONTROL_UNDEFINED            0x00
#define WD_UNDERFLOW_CONTROL            0x01
#define WD_OVERFLOW_CONTROL             0x02
#define WD_DECODER_ERROR_CONTROL        0x03

//Table A-37: DTS Decoder Control Selectors
#define DD_CONTROL_UNDEFINED            0x00
#define DD_UNDERFLOW_CONTROL            0x01
#define DD_OVERFLOW_CONTROL             0x02
#define DD_DECODER_ERROR_CONTROL        0x03

//Table A-38: Endpoint Control Selectors
#define EP_CONTROL_UNDEFINED            0x00
#define EP_CONTROL_PITCH                0x01
#define EP_CONTROL_DATA_OVERRUN         0x02
#define EP_CONTROL_DATA_UNDERRUN        0x03

//Table 2-1: USB Terminal Types
#define TERMINAL_TYPES_UNDEFINED        0x0100      //I/O USB Terminal, undefined Type.
#define TERMINAL_TYPES_STREAMING        0x0101      //I/O A Terminal dealing with a signal carried over an endpoint in an AudioStreaming interface. The AudioStreaming interface descriptor points to the associated Terminal through the bTerminalLink field.
#define TERMINAL_TYPES_VENDOR_SPECIFIC  0x01FF      //I/O A Terminal dealing with a signal carried over a vendor-specific interface. The vendor-specific interface descriptor must contain a field that references the Terminal.

//Table 2-2: Input Terminal Types//
#define INPUT_UNDEFINED                     0x0200      //I   Input Terminal, undefined Type.
#define INPUT_MICROPHONE                    0x0201      //I   A generic microphone that does not fit under any of the other classifications.
#define INPUT_DESKTOP_MICROPHONE            0x0202      //I   A microphone normally placed on the desktop or integrated into the monitor.
#define INPUT_PERSONAL_MICROPHONE           0x0203      //I   A head-mounted or clip-on microphone.
#define INPUT_OMNI_DIRECTIONAL_MICROPHONE   0x0204      //I   A microphone designed to pick up voice from more than one speaker at relatively long ranges.
#define INPUT_MICROPHONE_ARRAY              0x0205      //I   An array of microphones designed for directional processing using host-based signal processing algorithms.
#define INPUT_PROCESSING_MICROPHONE_ARRAY   0x0206      //I   An array of microphones with an embedded signal processor.

//Table 2-3: Output Terminal Types//
#define OUTPUT_UNDEFINED                       0x0300      //O   Output Terminal, undefined Type.
#define OUTPUT_SPEAKER                         0x0301      //O   A generic speaker or set of speakers that does not fit under any of the other classifications.
#define OUTPUT_HEADPHONES                      0x0302      //O   A head-mounted audio output device.
#define OUTPUT_HEAD_MOUNTED_DISPLAY_AUDIO      0x0303      //O   The audio part of a VR head mounted display. The Associated Interfaces descriptor can be used to reference the HID interface used to report the position and orientation of the HMD.
#define OUTPUT_DESKTOP_SPEAKER                 0x0304      //O   Relatively small speaker or set of speakers normally placed on the desktop or integrated into the monitor. These speakers are close to the user and have limited stereo separation.
#define OUTPUT_ROOM_SPEAKER                    0x0305      //O   Larger speaker or set of speakers that are heard well anywhere in the room.
#define OUTPUT_COMMUNICATION_SPEAKER           0x0306      //O   Speaker or set of speakers designed for voice communication.
#define OUTPUT_LOW_FREQUENCY_EFFECTS_SPEAKER   0x0307      //O   Speaker designed for low frequencies (subwoofer). Not capable of reproducing speech or music.

//Table 2-4: Bi-directional Terminal Types//
#define BI_DIRECTIONAL_UNDEFINED                       0x0400      //I/O Bi-directional Terminal, undefined Type.
#define BI_DIRECTIONAL_HANDSET                         0x0401      //I/O Hand-held bi-directional audio device.
#define BI_DIRECTIONAL_HEADSET_TERMINAL                0x0402      //I/O Head-mounted bi-directional audio device.
#define BI_DIRECTIONAL_SPEAKERPHONE_NO_ECHO_REDUCTION  0x0403      //I/O A hands-free audio device designed for host-based echo cancellation.
#define BI_DIRECTIONAL_ECHO_SUPPRESSING_SPEAKERPHONE   0x0404      //I/O A hands-free audio device with echo suppression capable of half-duplex operation.
#define BI_DIRECTIONAL_ECHO_CANCELING_SPEAKERPHONE     0x0405      //I/O A hands-free audio device with echo cancellation capable of full-duplex operation.

//Table 2-5: Telephony Terminal Types//
#define TELEPHONY_UNDEFINED             0x0500      //I/O Telephony Terminal, undefined Type.
#define TELEPHONY_PHONE_LINE            0x0501      //I/O May be an analog telephone line jack, an ISDN line, a proprietary PBX interface, or a wireless link.
#define TELEPHONY_TERMINAL              0x0502      //I/O Device can be used as a telephone. When not in use as a telephone, handset is used as a bi-directional audio device.
#define TELEPHONY_DOWN_LINE_PHONE       0x0503      //I/O A standard telephone set connected to the device. When not in use as a telephone, it can be used as a bi- directional audio device.

//Table 2-6: External Terminal Types//
#define EXTERNAL_UNDEFINED                  0x0600      //I/O External Terminal, undefined Type.
#define EXTERNAL_ANALOG_CONNECTOR           0x0601      //I/O A generic analog connector.
#define EXTERNAL_DIGITAL_AUDIO_INTERFACE    0x0602      //I/O A generic digital audio interface.
#define EXTERNAL_LINE_CONNECTOR             0x0603      //I/O An analog connector at standard line levels. Usually uses 3.5mm.
#define EXTERNAL_LEGACY_AUDIO_CONNECTOR     0x0604      //I/O An input connector assumed to be connected to the lineout of the legacy audio system of the host computer. Used for backward compatibility.
#define EXTERNAL_S_PDIF_INTERFACE           0x0605      //I/O An S/PDIF digital audio interface. The Associated Interface descriptor can be used to reference an interface used for controlling special functions of this interface.
#define EXTERNAL_1394_DA_STREAM             0x0606      //I/O An interface to audio streams on a 1394 bus.
#define EXTERNAL_1394_DV_STREAM_SOUNDTRACK  0x0607      //I/O An interface to soundtrack of A/V stream on a 1394 bus.
#define EXTERNAL_ADAT_LIGHTPIPE             0x0608      //I/O   An optical connection carrying the Alesis Digital Audio Tape stream. Contact Alesis for technical information.
#define EXTERNAL_TDIF                       0x0609      //I/O   Tascam Digital Interface. Contact Tascam for technical information.
#define EXTERNAL_MADI                       0x060A      //I/O   Multi-channel Audio Digital Interface as defined by AES.

//Table 2-7: Embedded Terminal Types//

#define EMBEDDED_UNDEFINED                       0x0700      //I/O Embedded Terminal, undefined Type.
#define EMBEDDED_LEVEL_CALIBRATION_NOISE_SOURCE  0x0701      //O   Internal Noise source for level calibration (MPEG  decoding,  Dolby Prologic™, AC-3  etc.)
#define EMBEDDED_EQUALIZATION_NOISE              0x0702      //O   Internal Noise source for measurements.
#define EMBEDDED_CD_PLAYER                       0x0703      //I   Audio compact disc player or CD-ROM capable of audio playback.
#define EMBEDDED_DAT                             0x0704      //I/O Digital Audio Tape.
#define EMBEDDED_DCC                             0x0705      //I/O Digital Compact Cassette.
#define EMBEDDED_COMPRESSED_AUDIO_PLAYER         0x0706      //I/O Compressed Audio player.
#define EMBEDDED_ANALOG_TAPE                     0x0707      //I/O Analog Audio Tape.
#define EMBEDDED_PHONOGRAPH                      0x0708      //I   Analog vinyl record player.
#define EMBEDDED_VCR_AUDIO                       0x0709      //I   Audio track of VCR.
#define EMBEDDED_VIDEO_DISC_AUDIO                0x070A      //I   Audio track of VideoDisc player.
#define EMBEDDED_DVD_AUDIO                       0x070B      //I   Audio track of DVD player.
#define EMBEDDED_TV_TUNER_AUDIO                  0x070C      //I   Audio track of TV tuner.
#define EMBEDDED_SATELLITE_RECEIVER_AUDIO        0x070D      //I   Audio track of satellite receiver.
#define EMBEDDED_CABLE_TUNER_AUDIO               0x070E      //I   Audio track of cable tuner.
#define EMBEDDED_DSS_AUDIO                       0x070F      //I   Audio track of DSS receiver.
#define EMBEDDED_RADIO_RECEIVER                  0x0710      //I   AM/FM radio receiver.
#define EMBEDDED_RADIO_TRANSMITTER               0x0711      //O   AM/FM radio transmitter.
#define EMBEDDED_MULTI_TRACK_RECORDER            0x0712      //I/O A multi-track recording system.
#define EMBEDDED_SYNTHESIZER                     0x0713      //I/O Synthesizer.
#define EMBEDDED_PIANO                           0x0714      //I/O Piano
#define EMBEDDED_GUITAR                          0x0715      //I/O Guitar
#define EMBEDDED_DRUMS_RHYTHM                    0x0716      //I/O Percussion Instrument
#define EMBEDDED_OTHER_MUSICAL_INSTRUMENT        0x0717      //I/O Undefined Musical Instrument

// Format Type Codes

#define FORMAT_TYPE_UNDEFINED           0x00
#define FORMAT_TYPE_I                   0x01
#define FORMAT_TYPE_II                  0x02
#define FORMAT_TYPE_III                 0x03
#define FORMAT_TYPE_IV                  0x04
#define EXT_FORMAT_TYPE_I               0x81
#define EXT_FORMAT_TYPE_II              0x82
#define EXT_FORMAT_TYPE_III             0x83

#define PROTOCOL_UNDEFINED              0x00
#define PRES_TIMESTAMP_PROTOCOL         0x01

// Audio Data Format Type I Bit Allocations
#define FORMAT_TYPE_I_PCM                      (1 << 0)
#define FORMAT_TYPE_I_PCM8                     (1 << 1)
#define FORMAT_TYPE_I_IEEE_FLOAT               (1 << 2)
#define FORMAT_TYPE_I_ALAW                     (1 << 3)
#define FORMAT_TYPE_I_MULAW                    (1 << 4)
#define FORMAT_TYPE_I_RAW_DATA                 (1 << 31)

// Audio Data Format Type II Bit Allocations
#define FORMAT_TYPE_II_MPEG                    (1 << 0)
#define FORMAT_TYPE_II_AC_3                    (1 << 1)
#define FORMAT_TYPE_II_WMA                     (1 << 2)
#define FORMAT_TYPE_II_DTS                     (1 << 3)
#define FORMAT_TYPE_II_RAW_DATA                (1 << 31)

// Audio Data Format Type III Bit Allocations
#define FORMAT_TYPE_III_IEC61937_AC_3                  (1 << 0)
#define FORMAT_TYPE_III_IEC61937_MPEG_1_Layer1         (1 << 1)
#define FORMAT_TYPE_III_IEC61937_MPEG_1_Layer2_3       (1 << 2)
#define FORMAT_TYPE_III_IEC61937_MPEG_2_NOEXT          (1 << 2)
#define FORMAT_TYPE_III_IEC61937_MPEG_2_EXT            (1 << 3)
#define FORMAT_TYPE_III_IEC61937_MPEG_2_AAC_ADTS       (1 << 4)
#define FORMAT_TYPE_III_IEC61937_MPEG_2_Layer1_LS      (1 << 5)
#define FORMAT_TYPE_III_IEC61937_MPEG_2_Layer2_3_LS    (1 << 6)
#define FORMAT_TYPE_III_IEC61937_DTS_I                 (1 << 7)
#define FORMAT_TYPE_III_IEC61937_DTS_II                (1 << 8)
#define FORMAT_TYPE_III_IEC61937_DTS_III               (1 << 9)
#define FORMAT_TYPE_III_IEC61937_ATRAC                 (1 << 10)
#define FORMAT_TYPE_III_IEC61937_ATRAC2_3              (1 << 11)
#define FORMAT_TYPE_III_WMA                            (1 << 12)

// Audio Data Format Type IV Bit Allocations
#define FORMAT_TYPE_IV_PCM8                            (1 << 1)
#define FORMAT_TYPE_IV_IEEE_FLOAT                      (1 << 2)
#define FORMAT_TYPE_IV_ALAW                            (1 << 3)
#define FORMAT_TYPE_IV_MULAW                           (1 << 4)
#define FORMAT_TYPE_IV_MPEG                            (1 << 5)
#define FORMAT_TYPE_IV_AC_3                            (1 << 6)
#define FORMAT_TYPE_IV_WMA                             (1 << 7)
#define FORMAT_TYPE_IV_IEC61937_AC_3                   (1 << 8)
#define FORMAT_TYPE_IV_IEC61937_MPEG_1_Layer1          (1 << 9)
#define FORMAT_TYPE_IV_IEC61937_MPEG_1_Layer2_3        (1 << 10)
#define FORMAT_TYPE_IV_IEC61937_MPEG_2_NOEXT           (1 << 10)
#define FORMAT_TYPE_IV_IEC61937_MPEG_2_EXT             (1 << 11)
#define FORMAT_TYPE_IV_IEC61937_MPEG_2_AAC_ADTS        (1 << 12)
#define FORMAT_TYPE_IV_IEC61937_MPEG_2_Layer1_LS       (1 << 13)
#define FORMAT_TYPE_IV_IEC61937_MPEG_2_Layer2_3_LS     (1 << 14)
#define FORMAT_TYPE_IV_IEC61937_DTS_I                  (1 << 15)
#define FORMAT_TYPE_IV_IEC61937_DTS_II                 (1 << 16)
#define FORMAT_TYPE_IV_IEC61937_DTS_III                (1 << 17)
#define FORMAT_TYPE_IV_IEC61937_ATRAC                  (1 << 18)
#define FORMAT_TYPE_IV_IEC61937_ATRAC2_3               (1 << 19)
#define FORMAT_TYPE_IV_TYPE_III_WMA                    (1 << 20)    //??? bug in spec?
#define FORMAT_TYPE_IV_IEC60958_PCM                    (1 << 21)

struct UAC2_TypeIFormatTypeDescriptor {
    uint8_t bLength; // Value: Number    Desc: Size of this descriptor, in bytes: 6
    uint8_t bDescriptorType; // Value: Constant  Desc: CS_INTERFACE descriptor type.
    uint8_t bDescriptorSubtype; // Value: Constant  Desc: FORMAT_TYPE descriptor subtype.
    uint8_t bFormatType; // Value: Constant  Desc: FORMAT_TYPE_I. Constant identifying the Format Type the AudioStreaming interface is using.
    uint8_t bSubslotSize; // Value: Number    Desc: The number of bytes occupied by one audio subslot. Can be 1, 2, 3 or 4.
    uint8_t bBitResolution; // Value: Number    Desc: The number of effectively used bits from the available bits in an audio subslot.
} __attribute__((packed));

struct UAC2_TypeIIFormatTypeDescriptor {
    uint8_t bLength; // Value: Number    Desc: Size of this descriptor, in bytes: 8
    uint8_t bDescriptorType; // Value: Constant  Desc: CS_INTERFACE descriptor type.
    uint8_t bDescriptorSubtype; // Value: Constant  Desc: FORMAT_TYPE descriptor subtype.
    uint8_t bFormatType; // Value: Constant  Desc: FORMAT_TYPE_II. Constant identifying the Format Type the AudioStreaming interface is using.
    uint16_t wMaxBitRate; // Value: Number    Desc: "Indicates the maximum number of bits per second this interface can handle. Expressed in kbits/s."
    uint16_t wSlotsPerFrame; // Value: Number    Desc: Indicates the number of PCM audio slots contained in one encoded audio frame.
} __attribute__((packed));

struct UAC2_TypeIIIFormatTypeDescriptor {
    uint8_t bLength; // Value: Number    Desc: Size of this descriptor, in bytes: 6
    uint8_t bDescriptorType; // Value: Constant  Desc: CS_INTERFACE descriptor type.
    uint8_t bDescriptorSubtype; // Value: Constant  Desc: FORMAT_TYPE descriptor subtype.
    uint8_t bFormatType; // Value: Constant  Desc: FORMAT_TYPE_III. Constant identifying the Format Type the AudioStreaming interface is using.
    uint8_t bSubslotSize; // Value: Number    Desc: The number of bytes occupied by one audio subslot. Must be set to two.
    uint8_t bBitResolution; // Value: Number    Desc: The number of effectively used bits from the available bits in an audio subframe.
} __attribute__((packed));

struct UAC2_TypeIVFormatTypeDescriptor {
    uint8_t bLength; // Value: Number    Desc: Size of this descriptor, in bytes: 4
    uint8_t bDescriptorType; // Value: Constant  Desc: CS_INTERFACE descriptor type.
    uint8_t bDescriptorSubtype; // Value: Constant  Desc: FORMAT_TYPE descriptor subtype.
    uint8_t bFormatType; // Value: Constant  Desc: FORMAT_TYPE_IV. Constant identifying the Format Type the AudioStreaming interface is using.
} __attribute__((packed));

struct UAC2_ExtendedTypeIFormatTypeDescriptor {
    uint8_t bLength; // Value: Number    Desc: Size of this descriptor, in bytes: 9
    uint8_t bDescriptorType; // Value: Constant  Desc: CS_INTERFACE descriptor type.
    uint8_t bDescriptorSubtype; // Value: Constant  Desc: FORMAT_TYPE descriptor subtype.
    uint8_t bFormatType; // Value: Constant  Desc: EXT_FORMAT_TYPE_I. Constant identifying the Format Type the AudioStreaming interface is using.
    uint8_t bSubslotSize; // Value: Number    Desc: The number of bytes occupied by one audio subslot. Can be 1, 2, 3 or 4.
    uint8_t bBitResolution; // Value: Number    Desc: The number of effectively used bits from the available bits in an audio subslot.
    uint8_t bHeaderLength; // Value: Number    Desc: Size of the Packet Header, in bytes.
    uint8_t bControlSize; // Value: Number    Desc: Size of the Control Channel Words, in bytes.
    uint8_t bSideBandProtocol; // Value: Constant  Desc: Constant, identifying the Side Band Protocol used for the Packet Header and Control Channel content.
} __attribute__((packed));

struct UAC2_ExtendedTypeIIFormatTypeDescriptor {
    uint8_t bLength; // Value: Number    Desc: Size of this descriptor, in bytes: 10
    uint8_t bDescriptorType; // Value: Constant  Desc: CS_INTERFACE descriptor type.
    uint8_t bDescriptorSubtype; // Value: Constant  Desc: FORMAT_TYPE descriptor subtype.
    uint8_t bFormatType; // Value: Constant  Desc: Ext_FORMAT_TYPE_II. Constant identifying the Format Type the AudioStreaming interface is using.
    uint16_t wMaxBitRate; // Value: Number    Desc: "Indicates the maximum number of bits per second this interface can handle.Expressed in kbits/s."
    uint16_t wSamplesPerFrame; // Value: Number    Desc: Indicates the number of PCM audio samples contained in one encoded audio frame.
    uint8_t bHeaderLength; // Value: Number    Desc: Size of the Packet Header, in bytes.
    uint8_t bSideBandProtocol; // Value: Constant  Desc: Constant, identifying the Side Band Protocol used for the Packet Header content.
} __attribute__((packed));

struct UAC2_ExtendedTypeIIIFormatTypeDescriptor {
    uint8_t bLength; // Value: Number    Desc: Size of this descriptor, in bytes: 8
    uint8_t bDescriptorType; // Value: Constant  Desc: CS_INTERFACE descriptor type.
    uint8_t bDescriptorSubtype; // Value: Constant  Desc: FORMAT_TYPE descriptor subtype.
    uint8_t bFormatType; // Value: Constant  Desc: EXT_FORMAT_TYPE_III. Constant identifying the Format Type the AudioStreaming interface is using.
    uint8_t bSubslotSize; // Value: Number    Desc: The number of bytes occupied by one audio subslot. Must be set to two.
    uint8_t bBitResolution; // Value: Number    Desc: The number of effectively used bits from the available bits in an audio subslot.
    uint8_t bHeaderLength; // Value: Number    Desc: Size of the Packet Header, in bytes.
    uint8_t bSideBandProtocol; // Value: Constant  Desc: Constant, identifying the Side Band Protocol used for the Packet Header content.
} __attribute__((packed));

struct UAC2_HiResPresentationTimeStampLayout {
    uint32_t bmFlags; // Value: Bitmap    Desc: D30..0: Reserved. Must be set to 0. D31: Valid.
    uint64_t qNanoSeconds; // Value: Number    Desc: Offset in nanoseconds from the beginning of the audio stream.
} __attribute__((packed));

#endif  /* AUC20_H */
