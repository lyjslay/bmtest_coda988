/**********************************************************************
 * Copyright (C) 2015-2017 Cadence Design Systems, Inc.
 * All rights reserved worldwide.
 ***********************************************************************
 * usb_dev_api.h
 * Generic header file for USB Device Drivers
 *
 ***********************************************************************/
#ifndef _USB_DEV_API_H
#define _USB_DEV_API_H

#include "dwc2_drv_if.h"
#include "byteorder.h"

//-------------- definitions used for debugging purpose ---------------
#define DBG_USB_APP_VERBOSE 0x00000001
#define DBG_USB_APP_ERROR   0x00000002
#define DBG_USB_APP         0x00000004
#define DBG_USB_CTRL_MSG    0x00000008

//-------------- Generic Configuration Section -------------------------
#define ID_VENDOR                       0x0559  // CADENCE ID
#define ID_PRODUCT                      0x1004  // default ID PRODUCT
#define BCD_DEVICE_SS                   0x0001  // 0.1
#define BCD_USB_SS                      0x0300  // 3.00 version USB
#define BCD_USB_HS_ONLY                 0x0201  // 2.01  /*Only HS with BOS descriptor*/
#define BCD_USB_HS                      0x0210  // 2.10
#define BCD_DEVICE_HS                   0x0200  // 2.00

#define USB_MANUFACTURER_STRING_INDEX   1
#define USB_PRODUCT_STRING_INDEX        2
#define USB_SERIAL_NUMBER_STRING_INDEX  3

#define USB_MANUFACTURER_STRING         "CADENCE"
#define USB_PRODUCT_STRING              "Cadence Example Application"
#define USB_SERIAL_NUMBER_STRING        "100000000000" // should 12 chars long

/*Set to 1 if device is Self Powered else set 0*/
#define DEVICE_IS_SELF_POWERED 1
/*Set to 1 if device support Remote Wakeup else set 0*/
#define DEVICE_IS_REMOTE_WAKEUP 1

/*Error Codes*/
#define ESTALL 1
//------------------------------------------------------------------------------

/*Driver status code*/
#define USB_DEV_APP_INITIALISED 0x00000001
#define USB_DEV_APP_CRITICAL_ERRROR 0x00000002
#define USB_DEV_APP_DISCONNECTED 0x00000004
#define USB_DEV_APP_CONFIGURED 0x00000008
#define USB_DEV_APP_SUSPENDED 0x00000010
#define USB_DEV_APP_DRIVER_STARTED 0x00000020

/*Maximum number of Configurations and Strings Descriptors that can be handled by USB Device Driver */
#define MAX_CONFIGURATION_NUMBER 3
#define MAX_STRING_DESCRIPTOR 10

/*Buffer size for Ep0*/
#define BUFFER_EP0_SIZE 512

struct usbDevice {
    uint8_t ep0Buff[BUFFER_EP0_SIZE];
    CUSBD_Dev *usbDev;
    uint8_t status;  /*appInitializedFlag*/
    uint32_t lastError;
    CUSBD_OBJ *drv; //driver pointer
    void *pD; //private data pointer
    CUSBD_Req *ep0Req; // USB Device Request for Ep0;
    CH9_UsbDeviceDescriptor * devDesc;
    uint8_t * bosDescriptor;
    uint8_t currentConfig; //Actually used configuration
    uint8_t currentAltSetting;
    uint8_t *configurations[MAX_CONFIGURATION_NUMBER];
    uint8_t *stringDescriptors[MAX_STRING_DESCRIPTOR];
};

/**
 * clears request
 */
void devClearReq(struct CUSBD_Req * req);
void devShowBuff(uint8_t * buff, uint32_t length);

/** Search buffer for required descriptor
 *  @param[in] buffer buffer containing data to search
 *  @param[in] size length of the buffer in bytes
 *  @param[in] type type of searched descriptor
 *  @param[out] ptr  pointer to found descriptor
 *  @return 0 for success else EINVAL
 */
uint32_t devFindDescriptor(uint8_t *buffer, uint16_t size, uint8_t type,
        uint8_t **ptr);
uint32_t devFindInterfaceInConfiguration(CH9_UsbConfigurationDescriptor * config,
		uint8_t ifNr, uint8_t altIfNr, CH9_UsbInterfaceDescriptor ** interface);

/** Search first Endpoint Descriptor in buffer before Interface Descriptor *
 *  @param[in] buffer buffer containing data to search. This buffer have to
 *      point at the beginning of first USB descriptor in buffer.
 *  @param[in] size length of the buffer in bytes
 *  @return CH9_UsbEndpointDescriptor object for success else NULL
 */
CH9_UsbEndpointDescriptor * devFindEndpointInInterface(uint8_t * buffer, uint32_t length);

/**
 * Function add new configuration to configuration array
 * @param[in] buffer buffer for configuration and all other descriptors it should
 *      be sufficient to hold all descriptors for this configuration.
 * @param[in] cfg pointer to configuration descriptor.
 *
 * Descriptor Configuration should be added in accordance with bConfigurationValue number
 * @return 0 on success or else error code
 */
uint32_t devAddConfiguration(struct usbDevice * udev, void *buffer, CH9_UsbConfigurationDescriptor * cfg);

/**
 * Function  gets required configuration.
 * @param[in] configNum number of required configuration

 * @return if configuration exist then function return pointer to buffer
 *      with Configuration  Descriptor else NULL
 */
extern CH9_UsbConfigurationDescriptor * devGetConfiguration(struct usbDevice * udev, uint8_t configNum);

/**
 * Function add new descriptor to configuration
 * @param[in] configIdx index configuration complies with bConfigurationValue field from Configuration Descriptor
 * @param[in] desc pointer to added descriptor
 *
 * Descriptor Configuration should be added in accordance with bConfigurationValue number.
 * @return 0 on success or else error code
 */
uint32_t devAddDescriptor(struct usbDevice * udev, uint8_t configIdx, const uint8_t * desc);

/**
 * Function add string to string  array and change it to unicode.
 * @param[in] stringId string index
 * @param[in] buffer pointer to non unicode string
 *
 * @return 0 on success or else error code
 */
uint32_t devAddString(struct usbDevice * udev, uint8_t stringId, const char  * buffer);

/**
 * Function delete string from  array
 * @param[in] stringId string index
 */
uint32_t devDelString(struct usbDevice * udev, uint8_t stringId);

/**
 * Function return string descriptor for index stringId
 * @param[in] stringId string index
 */
uint8_t * devGetString(struct usbDevice * udev, uint8_t stringId);

#define devFillTplInterface(tpl, _bInterfaceNumber, _bAlternateSetting, _bNumEndpoints, \
        _bInterfaceClass, _bInterfaceSubClass, _bInterfaceProtocol, _iInterface) { \
        tpl.bLength = CH9_USB_DS_INTERFACE; \
        tpl.bDescriptorType = CH9_USB_DT_INTERFACE; \
        tpl.bInterfaceNumber = _bInterfaceNumber; \
        tpl.bAlternateSetting = _bAlternateSetting; \
        tpl.bNumEndpoints = _bNumEndpoints; \
        tpl.bInterfaceClass = _bInterfaceClass; \
        tpl.bInterfaceSubClass = _iInterface; }


#define devFillTplEndpoint(tpl, _bEndpointAddress, _bmAttributes, _wMaxPacketSize, _bInterval) {\
        tpl.bLength = CH9_USB_DS_ENDPOINT; \
        tpl.bDescriptorType = CH9_USB_DT_ENDPOINT; \
        tpl.bEndpointAddress = _bEndpointAddress; \
        tpl.bmAttributes = _bmAttributes; \
        tpl.wMaxPacketSize = cpuToLe16(_wMaxPacketSize); \
        tpl.bInterval = _bInterval; }



//------------------- Generic descriptor set--------------------------------------------

/**
 * device descriptor for SuperSpeed mode
 */
static CH9_UsbDeviceDescriptor devSsDesc = {
    .bLength = CH9_USB_DS_DEVICE,
    .bDescriptorType = CH9_USB_DT_DEVICE,
    .bcdUSB = cpuToLe16(BCD_USB_SS),
    .bDeviceClass = 0,
    .bDeviceSubClass = 0,
    .bDeviceProtocol = 0,
    .bMaxPacketSize0 = 9,
    .idVendor = cpuToLe16(ID_VENDOR),
    .idProduct = cpuToLe16(ID_PRODUCT),
    .bcdDevice = cpuToLe16(BCD_DEVICE_SS),
    .iManufacturer = USB_MANUFACTURER_STRING_INDEX,
    .iProduct = USB_PRODUCT_STRING_INDEX,
    .iSerialNumber = USB_SERIAL_NUMBER_STRING_INDEX,
    .bNumConfigurations = 3
};

/**
 * device descriptor for HighSpeed mode
 */
static CH9_UsbDeviceDescriptor devHsDesc = {
    .bLength = CH9_USB_DS_DEVICE,
    .bDescriptorType = CH9_USB_DT_DEVICE,
    .bcdUSB = cpuToLe16(BCD_USB_HS),
    .bDeviceClass = 0,
    .bDeviceSubClass = 0,
    .bDeviceProtocol = 0,
    .bMaxPacketSize0 = 64,
    .idVendor = cpuToLe16(ID_VENDOR),
    .idProduct = cpuToLe16(ID_PRODUCT),
    .bcdDevice = cpuToLe16(BCD_DEVICE_HS),
    .iManufacturer = USB_MANUFACTURER_STRING_INDEX,
    .iProduct = USB_PRODUCT_STRING_INDEX,
    .iSerialNumber = USB_SERIAL_NUMBER_STRING_INDEX,
    .bNumConfigurations = 3
};

/**
 * configuration descriptor for SupperSpeed mode
 */
static CH9_UsbConfigurationDescriptor confSsDesc = {
    .bLength = CH9_USB_DS_CONFIGURATION,
    .bDescriptorType = CH9_USB_DT_CONFIGURATION,
    .wTotalLength = cpuToLe16(0), //Field filed in dynamically
    .bNumInterfaces = 1,
    .bConfigurationValue = 1,
    .iConfiguration = 0,
    .bmAttributes = 0,
    .bMaxPower = 1
};

/**
 * Template for Configuration Descriptors.
 * This descriptor will be changed before adding to configuration
 */
static CH9_UsbConfigurationDescriptor confHsDesc = {
    .bLength = CH9_USB_DS_CONFIGURATION,
    .bDescriptorType = CH9_USB_DT_CONFIGURATION,
    .wTotalLength = cpuToLe16(0), //Field filed in  dynamically
    .bNumInterfaces = 1,
    .bConfigurationValue = 1,
    .iConfiguration = 0,
    .bmAttributes = CH9_USB_CONFIG_RESERVED,
    .bMaxPower = 0
};

/**
 *  USB SuperSpeed Endpoint Companion Descriptor
 *  Descriptor dedicated for USB SuperSpeed device
 */
static CH9_UsbSSEndpointCompanionDescriptor compDesc = {
    .bLength = CH9_USB_DS_SS_USB_ENDPOINT_COMPANION,
    .bDescriptorType = CH9_USB_DT_SS_USB_ENDPOINT_COMPANION,
    .bMaxBurst = 0,
    .bmAttributes = 0,
    .wBytesPerInterval = cpuToLe16(1024)
};


/**
 *  language descriptor for english
 */
static uint8_t languageDesc[] = {
    4, CH9_USB_DT_STRING, 0x09, 0x04
};

/**
 * BOS Descriptor
 */
static CH9_UsbBosDescriptor bosDesc = {
    .bLength = CH9_USB_DS_BOS,
    .bDescriptorType = CH9_USB_DT_BOS,
    .wTotalLength = cpuToLe16(CH9_USB_DS_BOS + CH9_USB_DS_DEVICE_CAPABILITY_30 + CH9_USB_DS_DEVICE_CAPABILITY_20),
    .bNumDeviceCaps = 2
};

/**
 * Device Capability Descriptor for SuperSpeed USB device
 */
static CH9_UsbSSDeviceCapabilityDescriptor capabilitySsDesc = {
    .bLength = CH9_USB_DS_DEVICE_CAPABILITY_30,
    .bDescriptorType = CH9_USB_DT_DEVICE_CAPABILITY,
    .bDevCapabilityType = CH9_USB_DCT_SS_USB,
    .bmAttributes = 0, // LTM not supported
    .wSpeedSupported = cpuToLe16(CH9_USB_SS_CAP_SUPPORT_SS | CH9_USB_SS_CAP_SUPPORT_HS | CH9_USB_SS_CAP_SUPPORT_FS),
    .vFunctionalitySupport = 1,
    .bU1DevExitLat = 4, // 4 us
    .bU2DevExitLat = cpuToLe16(512) // 512 us
};

/**
 * Device Capability Descriptor
 */
static CH9_UsbCapabilityDescriptor capabilityExtDesc = {
    .bLength = CH9_USB_DS_DEVICE_CAPABILITY_20,
    .bDescriptorType = CH9_USB_DT_DEVICE_CAPABILITY,
    .bDevCapabilityType = CH9_USB_DCT_USB20_EXTENSION,
    .bmAttributes = cpuToLe32(CH9_USB_USB20_EXT_LPM_SUPPORT | CH9_USB_USB20_EXT_BESL_AND_ALTERNATE_HIRD)
};

/**
 * Qualifier Descriptor
 */
static CH9_UsbDeviceQualifierDescriptor qualifierDesc = {
    .bLength = CH9_USB_DS_DEVICE_QUALIFIER,
    .bDescriptorType = CH9_USB_DT_DEVICE_QUALIFIER,
    .bcdUSB = cpuToLe16(0x0200),
    .bDeviceClass = 0x00,
    .bDeviceSubClass = 0x00,
    .bDeviceProtocol = 0x00,
    .bMaxPacketSize0 = 64,
    .bNumConfigurations = 0x01,
    .bReserved = 0x00
};

#endif /*_USB_DEV_API_H*/
