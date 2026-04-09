/**********************************************************************
 * Copyright (C) 2014-2017 Cadence Design Systems, Inc.
 * All rights reserved worldwide.
 ***********************************************************************
 * scsi.c
 * SCSI commands parser
 ***********************************************************************/

#include "dwc2_stdio.h"                     // standard library
#include "dwc2_string.h"                    // standard library
#include "config.h"
#include "storage.h"
#include "scsi.h"
#include "bot.h"
#include "byteorder.h"
#include "log.h"


#define DBG_USB_SCSI_APP     0x000000008

#define CPU_DCASH_ENABLED 0

#if CPU_DCASH_ENABLED
extern void flush_dcash(unsigned int, unsigned);
extern void invalidate_dcash(unsigned int, unsigned);
#endif


//------------------------------------------------------------------------------
uint32_t dev_num_of_bytes; // keeps number of bytes to transfer by device
uint8_t dev_data_dir; // keeps SCSI data direction
uint32_t(*scsi_send_data)(void *, uint32_t);
uint32_t(*scsi_rec_data)(void *, uint32_t);
//------------------------------------------------------------------------------
// All devices shall support the commands in this section in order to boot
//------------------------------------------------------------------------------
#define SCSI_CMD_TEST_UNIT_READY        0x00
#define SCSI_CMD_REQEST_SENSE           0x03
#define SCSI_CMD_FORMAT_UNIT            0x04
#define SCSI_CMD_INQUIRY                0x12
#define SCSI_CMD_MEDIUM_REMOVAL         0x1E
#define SCSI_CMD_MODE_SENSE_10          0x1A   //ellisys tester needs to have 0x5A
#define SCSI_CMD_READ_FMT_CAPACITIES    0x23
#define SCSI_CMD_READ_CAPACITY          0x25
#define SCSI_CMD_READ_10                0x28
#define SCSI_CMD_READ_12                0xA8
#define SCSI_CMD_WRITE_10               0x2A
#define SCSI_CMD_WRITE_12               0xAA
#define SCSI_CMD_VERIFY                 0x2F
#define SCSI_CMD_BLANK                  0xA1
#define SCSI_CMD_REPORT_LUNS        0xA0
//------------------------------------------------------------------------------


// functions aligns buffer to alignValue modulo address

static uint8_t * alignBuff(void * notAllignedBuff, uint8_t alignValue)  {

    uint8_t offset = ((uintptr_t) notAllignedBuff) % alignValue;
    if (offset == 0) {
        return (uint8_t*) notAllignedBuff;
    } else {
        uint8_t * ret_address = &(((uint8_t*) notAllignedBuff)[alignValue - offset]);
        return ret_address;
    }
}

static uint8_t scsi_data_alloc[36 + 7];
static uint8_t * scsi_data;

//------------------------------------------------------------------------------
// TEST UNIT READY Command
//------------------------------------------------------------------------------

static uint16_t scsiTestUnitReady(uint8_t *cmd, uint32_t *retBytes)  {

    uint8_t lun;

    lun = cmd[1] >> 5;

    // Get a unit ready state
    return devTestUnitReady(lun);
}

//------------------------------------------------------------------------------
// Error support for Request sense command
//------------------------------------------------------------------------------
static uint16_t lastSenseCode[8];
//------------------------------------------------------------------------------
// Sense data response codes
#define SENSE_DATA_FIXED_CURRENT        0x70
#define SENSE_DATA_FIXED_DEFERRED       0x71

// Sense key
#define SENSE_KEY_NO_SENSE              0x0000
#define SENSE_KEY_NOT_READY             0x0002
#define SENSE_KEY_MEDIUM_ERROR          0x0003
#define SENSE_KEY_HARDWARE_ERROR        0x0004
#define SENSE_KEY_ILLEGAL_REQUEST       0x0005
#define SENSE_KEY_DATA_PROTECT          0x0007
#define SENSE_KEY_VOLUME_OVERFLOW       0x000D
#define SENSE_KEY_ABORTED_COMMAND       0x000B
#define SENSE_KEY_MISCOMPARE            0x000E

// Additional sense code
#define ASC_LOGICAL_UNIT_NOT_READY      0x0400
#define ASC_LBA_OUT_OF_RANGE            0x2100
#define ASC_WRITE_PROTECTED             0x2700
#define ASC_FORMAT_CORRUPTED            0x3100
#define ASC_INVALID_COMMAND             0x2000
#define ASC_TOO_MUCH_WRITE_DATA         0x2600
#define ASC_MEDIUM_NOT_PRESENT          0x3A00
//------------------------------------------------------------------------------

static uint16_t scsiSetLastError(uint8_t lun, uint16_t errorCode)  {
    // Check a LUN
    if (lun >= sizeof (lastSenseCode) / sizeof (lastSenseCode[0]))
        return errorCode;

    // Translate error code to SCSI Sense Key
    switch (errorCode) {
        case ERR_SCSI_OK:
            lastSenseCode[lun] = SENSE_KEY_NO_SENSE;
            break;

        case ERR_SCSI_UNKNOWN_COMMAND:
        case ERR_SCSI_DAMAGED_COMMAND:
        case ERR_INVALID_LUN:
            lastSenseCode[lun] = SENSE_KEY_ILLEGAL_REQUEST;
            break;

        case ERR_INVALID_SECTOR:
            lastSenseCode[lun] = SENSE_KEY_VOLUME_OVERFLOW | ASC_LBA_OUT_OF_RANGE;
            break;

        case ERR_NOT_READY:
            lastSenseCode[lun] = SENSE_KEY_NOT_READY | ASC_LOGICAL_UNIT_NOT_READY;
            break;

        case ERR_VERIFY_ERROR:
            lastSenseCode[lun] = SENSE_KEY_MISCOMPARE;
            break;

        case ERR_SCSI_INVALID_DATA_SIZE:
            lastSenseCode[lun] = SENSE_KEY_HARDWARE_ERROR;
            break;

        default:
            lastSenseCode[lun] = SENSE_KEY_ABORTED_COMMAND;
    }

    return errorCode;
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
// REQEST SENSE Command
//------------------------------------------------------------------------------

static uint16_t scsiRequestSense(uint8_t *cmd, uint32_t *retBytes)  {

    uint8_t lun; // DataBuff[18];

    lun = cmd[1] >> 5;

    // Clear the data buffer
    memset(scsi_data, 0, 18);

    // Store REQEST SENSE command specific data
    scsi_data[0] = SENSE_DATA_FIXED_CURRENT;
    scsi_data[2] = lastSenseCode[lun] & 0x0F;
    scsi_data[7] = 10;
    scsi_data[12] = lastSenseCode[lun] >> 8;
#if CPU_DCASH_ENABLED
    flush_dcash((unsigned int) scsi_data, 18);
#endif
    dev_num_of_bytes = (cmd[4] < 18 ? cmd[4] : 18);
    dev_data_dir = CBW_FLAG_DIR_IN;
    *retBytes = scsi_send_data(scsi_data, dev_num_of_bytes);
    return ERR_SCSI_OK;
}
//------------------------------------------------------------------------------



//------------------------------------------------------------------------------
// FORMAT UNIT Command
//------------------------------------------------------------------------------

static uint16_t scsiFormatUnit(uint8_t *cmd, uint32_t *retBytes)  {

    uint8_t lun;
    uint16_t interleave;

    // Read command parameters
    lun = cmd[1] >> 5;
    interleave = be16ToCpu(*(uint16_t *) & cmd[3]);

    // Check command structure
    if ((cmd[1] & 0x1F) != 0x17)
        return ERR_SCSI_DAMAGED_COMMAND;

    return devFormatUnit(lun, interleave);
}
//------------------------------------------------------------------------------



//------------------------------------------------------------------------------
// INQUIRY Command
//------------------------------------------------------------------------------
#define SCSI_SBC_DIRECT_ACCESS_DEVICE  0x00
#define SCSI_CD_ROM_DEVICE             0x05
#define SCSI_OPTICAL_MEMORY_DEVICE     0x07
#define SCSI_RBC_DIRECT_ACCESS_DEVICE  0x0E
//------------------------------------------------------------------------------

static void scsiInquiryCopyString(char *dest, char *src, int length)  {
    int i, hasEnd = 0;

    // Function copies Length characters from the source to destination. The empty space
    // behind the source string and and characters with values from 0x00-0x1F and
    // 0x7F to 0xFF are stored in the destination as character ' ' (0x20)
    for (i = 0; i < length; i++) {
        hasEnd |= src[i] == '\0';
        if (!hasEnd && (src[i] > 0x1F) && (src[i] < 0x7F))
            dest[i] = src[i];
        else
            dest[i] = ' ';
    }
}
//------------------------------------------------------------------------------

static uint16_t scsiInquiry(uint8_t *cmd, uint32_t *retBytes)  {

    memset(scsi_data, 0, 36);

    // Store INQUIRY command specific data
    scsi_data[0] = 0x00; // 0x1F & SCSI_RBC_DIRECT_ACCESS_DEVICE;
    scsi_data[1] = 0x80; //0x80; //0; //REMOVEABLE_MEDIA_DEVICE ? 0x80 : 0x00;
    scsi_data[2] = 0x02; //2;
    scsi_data[3] = 0x00; //1; ACA
    scsi_data[4] = 0x1F; // No additional data
    scsi_data[7] = 0x02; // CmdQue

    scsiInquiryCopyString((char *) & scsi_data[8], SCSI_VENDOR_ID_STRING, 7);
    scsiInquiryCopyString((char *) & scsi_data[16], SCSI_PRODUCT_ID_STRING, 12);
    scsiInquiryCopyString((char *) & scsi_data[32], SCSI_PRODUCT_REV_LEFEL_STRING, 4);
#if CPU_DCASH_ENABLED
    flush_dcash((unsigned int) scsi_data, 36);
#endif
    dev_num_of_bytes = (cmd[4] < 36 ? cmd[4] : 36);
    dev_data_dir = CBW_FLAG_DIR_IN;
    *retBytes = scsi_send_data(scsi_data, dev_num_of_bytes);

    return ERR_SCSI_OK;
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
// MEDIUM REMOVAL Command
//------------------------------------------------------------------------------

static uint16_t scsiMediumRemoval(uint8_t *cmd, uint32_t *retBytes)  {

    return ERR_SCSI_OK;
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
// READ FORMAT CAPACITIES Command
//------------------------------------------------------------------------------

static uint16_t scsiReadFormatCapacities(uint8_t *cmd, uint32_t *retBytes)  {

    uint8_t lun, tempBuff[12] = {0, 0, 0, 8, 0, 0, 0, 0, 2, 0, 0, 0};
    uint32_t sectorCount; // ... formated media??
    uint16_t uint8_tsPerSector, err;

    memcpy(scsi_data, tempBuff, sizeof (tempBuff));

    lun = cmd[1] >> 5;

    // Read a device capacity
    err = devGetCapacities(lun, &sectorCount, &uint8_tsPerSector);
    if (err)
        return err;

    // Store command data
    *(uint32_t *) &scsi_data[4] = cpuToBe32(sectorCount);
    *(uint16_t *) &scsi_data[10] = cpuToBe16(uint8_tsPerSector);

#if CPU_DCASH_ENABLED
    flush_dcash((unsigned int) scsi_data, 12);
#endif

    dev_num_of_bytes = 12;
    dev_data_dir = CBW_FLAG_DIR_IN;
    *retBytes = scsi_send_data(scsi_data, 12);

    return ERR_SCSI_OK;
}
//------------------------------------------------------------------------------



//------------------------------------------------------------------------------
// READ CAPACITY Command
//------------------------------------------------------------------------------

static uint16_t scsiReadCapacity(uint8_t *cmd, uint32_t *retBytes)  {

    uint8_t lun;
    uint32_t sectorCount;
    uint16_t uint8_tsPerSector, err;

    // Read command parameters
    lun = cmd[1] >> 5;

    // Read a device capacity
    err = devGetCapacities(lun, &sectorCount, &uint8_tsPerSector);
    if (err)
        return err;

    // Store command data
    *((uint32_t*) & scsi_data[0]) = cpuToBe32(sectorCount - 1);
    *((uint32_t*) & scsi_data[4]) = cpuToBe32((uint32_t) uint8_tsPerSector);
#if CPU_DCASH_ENABLED
    flush_dcash((unsigned int) scsi_data, 8);
#endif
    dev_num_of_bytes = 8;
    dev_data_dir = CBW_FLAG_DIR_IN;
    *retBytes = scsi_send_data(scsi_data, 8);

    return ERR_SCSI_OK;
}
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// READ(10) Command
//------------------------------------------------------------------------------

static uint16_t scsiRead_10(uint8_t *cmd, uint32_t *retBytes)  {

    uint8_t lun;
    uint32_t startSec;
    uint16_t numOfSec;
    uint8_t temp[4];

    lun = cmd[1] >> 5;

    temp[0] = cmd[2];
    temp[1] = cmd[3];
    temp[2] = cmd[4];
    temp[3] = cmd[5];
    startSec = be32ToCpu(*(uint32_t *) temp);

    temp[0] = cmd[7];
    temp[1] = cmd[8];
    numOfSec = be16ToCpu(*(uint16_t *) temp);

    dev_data_dir = CBW_FLAG_DIR_IN;
    dev_num_of_bytes = (uint32_t) numOfSec * SECTOR_SIZE;
    devReadSector(lun, startSec, numOfSec, retBytes);

    return ERR_SCSI_OK;
}
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// WRITE(10) Command
//------------------------------------------------------------------------------

static uint16_t scsiWrite_10(uint8_t *cmd, uint32_t *retBytes, uint8_t write_12_command)  {

    uint8_t lun;
    uint32_t startSec;
    uint32_t numOfSec;
    uint8_t temp[4];

    lun = cmd[1] >> 5;

    temp[0] = cmd[2];
    temp[1] = cmd[3];
    temp[2] = cmd[4];
    temp[3] = cmd[5];
    startSec = be32ToCpu(*(uint32_t *) temp);

    if (!write_12_command) {
        //numOfSec = (uint32_t) be16ToCpu(*(uint16_t *) & cmd[7]);
        temp[0] = cmd[7];
        temp[1] = cmd[8];
        numOfSec = be16ToCpu(*(uint16_t *) temp);
    } else {
        //numOfSec = be32ToCpu(*(uint32_t *) & cmd[6]);
        temp[0] = cmd[6];
        temp[1] = cmd[7];
        temp[2] = cmd[8];
        temp[3] = cmd[9];
        numOfSec = be32ToCpu(*(uint32_t *) temp);
    }

    dev_data_dir = CBW_FLAG_DIR_OUT;
    dev_num_of_bytes = (uint32_t) numOfSec * SECTOR_SIZE;
    devWriteSector(lun, startSec, numOfSec, retBytes);

    return ERR_SCSI_OK;
}
//------------------------------------------------------------------------------



//------------------------------------------------------------------------------
// VERIFY Command
//------------------------------------------------------------------------------

static uint16_t scsiVerify(uint8_t *cmd, uint32_t *retBytes)  {

    uint8_t lun;
    uint32_t startSector;
    uint16_t sectorCount;

    // Read command parameters
    lun = cmd[1] >> 5;
    startSector = be32ToCpu(*(uint32_t *) & cmd[2]);
    sectorCount = be16ToCpu(*(uint16_t *) & cmd[7]);

    // Data transfer??
    // ...

    // Verify a data written on the device

    return devVerifySector(lun, startSector, sectorCount);
}
//------------------------------------------------------------------------------



//------------------------------------------------------------------------------
// MODE SENSE(10) Command
//------------------------------------------------------------------------------

static uint16_t scsiModeSense10(uint8_t *cmd, uint32_t *retBytes)  {

    uint8_t DataBuff_[] = {4, 0, 0, 0};
    memcpy(scsi_data, DataBuff_, sizeof (DataBuff_));

    // Send a Mode Sense command data
#if CPU_DCASH_ENABLED
    flush_dcash((unsigned int) scsi_data, 4);
#endif

    dev_num_of_bytes = 4;
    dev_data_dir = CBW_FLAG_DIR_IN;
    *retBytes = scsi_send_data(scsi_data, 4);

    return ERR_SCSI_OK;
}

//------------------------------------------------------------------------------

void scsiInit(void) {
    int i;

    devInit();
    scsi_data = alignBuff(scsi_data_alloc, 8);

    // Set a sense key to No sense
    for (i = 0; i < sizeof (lastSenseCode) / sizeof (lastSenseCode[0]); i++)
        lastSenseCode[i] = SENSE_KEY_NO_SENSE;
}
//------------------------------------------------------------------------------



//------------------------------------------------------------------------------
// Execute SCSI command for a storage driver
//------------------------------------------------------------------------------

uint16_t scsiExecCmd(uint8_t *cmd, uint32_t *retBytes)  {

    uint16_t err = ERR_SCSI_UNKNOWN_COMMAND;
    uint8_t lun;
    *retBytes = 0;

    switch (*cmd) {
        case SCSI_CMD_TEST_UNIT_READY:
            vDbgMsg(DBG_USB_SCSI_APP, DBG_WARN, "SCSI_CMD_TEST_UNIT_READY%s\n", "");
            err = scsiTestUnitReady(cmd, retBytes);
            break;

        case SCSI_CMD_REQEST_SENSE:
            vDbgMsg(DBG_USB_SCSI_APP, DBG_WARN, "SCSI_CMD_REQEST_SENSE%s\n", "");
            err = scsiRequestSense(cmd, retBytes);
            break;

        case SCSI_CMD_FORMAT_UNIT:
            vDbgMsg(DBG_USB_SCSI_APP, DBG_WARN, "SCSI_CMD_FORMAT_UNIT%s\n", "");
            err = scsiFormatUnit(cmd, retBytes);
            break;

        case SCSI_CMD_INQUIRY:
            vDbgMsg(DBG_USB_SCSI_APP, DBG_WARN, "SCSI_CMD_INQUIRY%s\n", "");
            err = scsiInquiry(cmd, retBytes);
            break;

        case SCSI_CMD_MEDIUM_REMOVAL:
            vDbgMsg(DBG_USB_SCSI_APP, DBG_WARN, "SCSI_CMD_MEDIUM_REMOVAL%s\n", "");
            err = scsiMediumRemoval(cmd, retBytes);
            break;

        case SCSI_CMD_READ_FMT_CAPACITIES:
            vDbgMsg(DBG_USB_SCSI_APP, DBG_WARN, "SCSI_CMD_READ_FMT_CAPACITIES%s\n", "");
            err = scsiReadFormatCapacities(cmd, retBytes);
            break;

        case SCSI_CMD_READ_CAPACITY:
            vDbgMsg(DBG_USB_SCSI_APP, DBG_WARN, "SCSI_CMD_READ_CAPACITY%s\n", "");
            err = scsiReadCapacity(cmd, retBytes);
            break;

        case SCSI_CMD_READ_12:
        case SCSI_CMD_READ_10:
            vDbgMsg(DBG_USB_SCSI_APP, DBG_WARN, "SCSI_CMD_READ_10%s\n", "");
            err = scsiRead_10(cmd, retBytes);
            break;

        case SCSI_CMD_WRITE_12:
            vDbgMsg(DBG_USB_SCSI_APP, DBG_WARN, "SCSI_CMD_WRITE_12%s\n", "");
            err = scsiWrite_10(cmd, retBytes, 1);
            break;
        case SCSI_CMD_WRITE_10:
            vDbgMsg(DBG_USB_SCSI_APP, DBG_WARN, "SCSI_CMD_WRITE_10%s\n", "");
            err = scsiWrite_10(cmd, retBytes, 0);
            break;

        case SCSI_CMD_VERIFY:
            vDbgMsg(DBG_USB_SCSI_APP, DBG_WARN, "SCSI_CMD_VERIFY%s\n", "");
            err = scsiVerify(cmd, retBytes);
            break;

        case SCSI_CMD_MODE_SENSE_10:
            vDbgMsg(DBG_USB_SCSI_APP, DBG_WARN, "SCSI_CMD_MODE_SENSE_10%s\n", "");
            err = scsiModeSense10(cmd, retBytes);
            break;

        case SCSI_CMD_BLANK:
            err = 1;
            break;

        case SCSI_CMD_REPORT_LUNS:
        {
            memset(scsi_data, 0, 36);
            *(uint32_t*) scsi_data = cpuToBe32(2);
            *((uint32_t*) & scsi_data[8]) = cpuToBe32(1);
#if CPU_DCASH_ENABLED
            flush_dcash((unsigned int) scsi_data, 12);
#endif
            dev_num_of_bytes = 12;
            dev_data_dir = CBW_FLAG_DIR_IN;
            *retBytes = scsi_send_data(scsi_data, 12);
            err = 0;
        }
            break;

        default:
            vDbgMsg(DBG_USB_SCSI_APP, DBG_WARN, "Unrecognized SCSI command %02X\n", *cmd);
            return err;
            break;

    }

    // Sets last error code
    lun = cmd[1] >> 5;
    return scsiSetLastError(lun, err);
}
//------------------------------------------------------------------------------


