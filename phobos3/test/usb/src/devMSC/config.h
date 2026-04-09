/**********************************************************************
 * Copyright (C) 2014 Cadence Design Systems, Inc.
 * All rights reserved worldwide.
 ***********************************************************************
 * config.h
 * BOT protocol configuration file
 ***********************************************************************/
#ifndef CONFIG_H
#define CONFIG_H

//#define HS_FS_ONLY
//#define GENERATE_SERIAL_STRING

#define MSC_ENDPOINT_IN                 0x81
#define MSC_ENDPOINT_OUT                0x01

#define BCD_USB_SS                      0x0300  // 3.00 version USB
#define ID_VENDOR                       0x0559  // CADENCE

#ifdef HS_FS_ONLY
#define ID_PRODUCT                      0x1002  // Mass storage product
#else
#define ID_PRODUCT                      0x4001  // Mass storage product
#endif

#define BCD_DEVICE_SS                   0x0010  // 0.1

#ifdef HS_FS_ONLY
#define BCD_USB_HS                      0x0200  // 2.00
#else
#define BCD_USB_HS                      0x0210  // 2.10
#endif

#define BCD_DEVICE_HS                   0x0200  // 2.00

#define USB_MANUFACTURER_STRING         "CADENCE"
#define USB_PRODUCT_STRING              "Mass storage device"
#define USB_SERIAL_NUMBER_STRING        "100000000000" // should 12 chars long

#define SCSI_VENDOR_ID_STRING           "Generic"
#define SCSI_PRODUCT_ID_STRING          "Mass-Storage"
#define SCSI_PRODUCT_REV_LEFEL_STRING   "0100"

#endif // Config_H


