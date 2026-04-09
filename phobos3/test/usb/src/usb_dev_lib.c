/**********************************************************************
 * Copyright (C) 2015 Cadence Design Systems, Inc.
 * All rights reserved worldwide.
 ***********************************************************************
 * usb_dev_lib.c
 * Library with useful functions for USB Device Driver
 *
 * This library is used by usb_dev_isoc Example
 ***********************************************************************/
#include <stdlib.h>
#include <string.h>

#include "usb_dev_api.h"
#include "cusbd_if.h"
#include "byteorder.h"
#include "cdn_errno.h"                     // errors definitions
#include "log.h"

#define SHOW_BUFF_SIZE 500
char showBuffTemp[SHOW_BUFF_SIZE];

/**
 * clears request
 */
void devClearReq(struct CUSBD_Req * req) {
    memset(req, 0, sizeof (struct CUSBD_Req));
}

void devShowBuff(uint8_t * buff, uint32_t length) {
    uint32_t i = 0, ret = 0;
    uint32_t buff_len = SHOW_BUFF_SIZE - 1;
    for(i = 0; i < length; i++) {
        ret = snprintf (&showBuffTemp[SHOW_BUFF_SIZE - buff_len], buff_len,  "0x0%x, ", buff[i]);
        buff_len -= ret;
    }
    showBuffTemp[SHOW_BUFF_SIZE - buff_len - 1] = '\0';
    vDbgMsg(DBG_USB_APP, DBG_HIVERB, "BUFF: %s\n", &showBuffTemp[1]);
}

/** Search buffer for required descriptor
 *  @param[in] buffer buffer containing data to search
 *  @param[in] size length of the buffer in bytes
 *  @param[in] type type of searched descriptor
 *  @param[out] ptr  pointer to found descriptor
 *  @return 0 for success else EINVAL
 */
uint32_t devFindDescriptor(uint8_t *buffer, uint16_t size, uint8_t type,
                           uint8_t **ptr) {
    CH9_UsbHeaderDescriptor * header;

    while (size >= sizeof(CH9_UsbHeaderDescriptor)) {
        header = (CH9_UsbHeaderDescriptor *) buffer;
        if (header->bLength < 2) {
            return EINVAL;
        }

        if (header->bDescriptorType == type) {
            *ptr = (uint8_t*) header;
            return 0;
        }

        buffer += header->bLength;
        size -= header->bLength;
    }
    *ptr = (uint8_t*) buffer;
    return EINVAL;
}

uint32_t devFindInterfaceInConfiguration(CH9_UsbConfigurationDescriptor * config, uint8_t ifNr, uint8_t altIfNr, CH9_UsbInterfaceDescriptor ** interface) {
    uint32_t length, offset = 0;
    uint32_t retval;
    CH9_UsbInterfaceDescriptor * search;
    uint8_t *buff = (uint8_t*)config;

    if(config->bDescriptorType != CH9_USB_DT_CONFIGURATION) {
        return EINVAL;
    }

    length = le16ToCpu(config->wTotalLength);

    while(length >=  CH9_USB_DS_INTERFACE) {
        retval = devFindDescriptor((uint8_t*)buff, length,  CH9_USB_DT_INTERFACE, (uint8_t**)&search);
        if(retval) {
            *interface = NULL;
            return retval;
        }

        if(search->bInterfaceNumber == ifNr && search->bAlternateSetting == altIfNr) {
            *interface = search;
            return 0;
        }

        offset = ((uintptr_t)search - (uintptr_t)buff) + CH9_USB_DS_INTERFACE;
        length -= offset;
        buff = buff + offset;
    }
    *interface = NULL;
    return EINVAL;
}

/** Search first Endpoint Descriptor in buffer before Interface Descriptor *
 *  @param[in] buffer buffer containing data to search. This buffer have to
 *      point at the beginning of first USB descriptor in buffer.
 *  @param[in] size length of the buffer in bytes
 *  @return CH9_UsbEndpointDescriptor object for success else NULL
 */
CH9_UsbEndpointDescriptor * devFindEndpointInInterface(uint8_t * buffer, uint32_t length) {
    uint32_t retval;
    uint32_t realLength;
    uint8_t * temp = NULL;

    retval = devFindDescriptor((uint8_t*)buffer, length,  CH9_USB_DT_INTERFACE, &temp);
    if(retval) {
        realLength = (uintptr_t)temp - (uintptr_t)buffer;
    }
    else{
        realLength = length;
    }

    retval = devFindDescriptor((uint8_t*)buffer, realLength,  CH9_USB_DT_ENDPOINT, &temp);
    if(retval) {
        return NULL;
    }

    return (CH9_UsbEndpointDescriptor*)temp;
}

/**
 * Function add new configuration to configuration array
 * @param[in] buffer buffer for configuration and all other descriptors it should
 *      be sufficient to hold all descriptors for this configuration.
 * @param[in] cfg pointer to configuration descriptor.
 *
 * Descriptor Configuration should be added in accordance with bConfigurationValue number
 * @return 0 on success or else error code
 */
uint32_t devAddConfiguration(struct usbDevice * udev, void *buffer, CH9_UsbConfigurationDescriptor * cfg) {
    if(!buffer || !cfg) {
        return EINVAL;
    }

    if(cfg->bConfigurationValue > MAX_CONFIGURATION_NUMBER) {
        return EINVAL;
    }

    udev->configurations[cfg->bConfigurationValue - 1] = buffer;
    cfg->wTotalLength = cpuToLe16(CH9_USB_DS_CONFIGURATION);
    memmove(buffer, cfg, CH9_USB_DS_CONFIGURATION);

    return 0;
}

/**
 * Function  gets required configuration.
 * @param[in] configNum number of required configuration

 * @return if configuration exist then function return pointer to buffer
 *      with Configuration  Descriptor else NULL
 */
CH9_UsbConfigurationDescriptor * devGetConfiguration(struct usbDevice * udev, uint8_t configNum) {

    if(configNum > MAX_CONFIGURATION_NUMBER) {
        return NULL;
    }

    return (CH9_UsbConfigurationDescriptor *)udev->configurations[configNum - 1];
}

/**
 * Function add new descriptor to configuration
 * @param[in] configIdx index configuration complies with bConfigurationValue field from Configuration Descriptor
 * @param[in] desc pointer to added descriptor
 *
 * Descriptor Configuration should be added in accordance with bConfigurationValue number.
 * @return 0 on success or else error code
 */
uint32_t devAddDescriptor(struct usbDevice * udev, uint8_t configIdx, const uint8_t * desc) {
    CH9_UsbConfigurationDescriptor * cfg = NULL;
    uint16_t length;

    if(configIdx > MAX_CONFIGURATION_NUMBER || !desc || desc[0]==0) {
        return EINVAL;
    }

    if(udev->configurations[configIdx - 1] == NULL) {
        return EINVAL;
    }

    cfg = (CH9_UsbConfigurationDescriptor*)udev->configurations[configIdx - 1];
    length = le16ToCpu(cfg->wTotalLength);

    memmove(&udev->configurations[configIdx - 1][length], desc, desc[0]);
    /*Updating configuration length*/
    cfg->wTotalLength = cpuToLe16(length + desc[0]);
    return 0;
}

/**
 * Function return unicode string for english version
 * @param[in] target pointer to buffer for unicode string
 * @param[in] src pointer to non unicode string
 */
static void getUnicodeString(char * target, const char * src) {
    size_t src_len = strlen(src) * 2;
    uint32_t i;

    *target++ = src_len + 2;
    *target++ = CH9_USB_DT_STRING;

    if (src_len > 100) {src_len = 100;}
    for (i = 0; i < src_len; i += 2) {
        *target++ = *src++;
        *target++ = 0;
    }
}

/**
 * Function add string to string  array and change it to unicode.
 * @param[in] stringId string index
 * @param[in] buffer pointer to non unicode string
 *
 * @return 0 on success or else error code
 */
uint32_t devAddString(struct usbDevice * udev, uint8_t stringId, const char  * buffer) {
    uint32_t length = 0;
    if(stringId < MAX_STRING_DESCRIPTOR && !buffer) {
        return EINVAL;
    }

    if(udev->stringDescriptors[stringId - 1 ]) {
        return EINVAL;
    }

    length = strlen(buffer) * 2 + 2;
    udev->stringDescriptors[stringId - 1] = malloc(length);
    if(!udev->stringDescriptors[stringId - 1]) {
        return ENOMEM;
    }

    getUnicodeString((char*)udev->stringDescriptors[stringId - 1], buffer);
    return 0;
}

/**
 * Function delete string from  array
 * @param[in] stringId string index
 */
uint32_t devDelString(struct usbDevice * udev, uint8_t stringId) {
    uint8_t i;
    //Delete all strings
    if(stringId == 0) {
        for(i = 0; i< MAX_STRING_DESCRIPTOR; i++) {
            if(udev->stringDescriptors[i]) {
                free(udev->stringDescriptors[i]);
                udev->stringDescriptors[i] = NULL;
            }
        }
    } else {
        if(stringId <= MAX_STRING_DESCRIPTOR) {
            if(udev->stringDescriptors[stringId - 1]) {
                free(udev->stringDescriptors[stringId - 1]);
                udev->stringDescriptors[stringId - 1] = NULL;
            }
        } else {
            return EINVAL;
        }
    }
    return 0;
}

/**
 * Function return string descriptor for index stringId
 * @param[in] stringId string index
 */
uint8_t * devGetString(struct usbDevice * udev, uint8_t stringId) {
    if(stringId == 0 ||  stringId > MAX_STRING_DESCRIPTOR ) {
        return NULL;
    } else if(udev->stringDescriptors[stringId - 1]) {
        return udev->stringDescriptors[stringId - 1];
    }
    else {
        return NULL;
    }
}
