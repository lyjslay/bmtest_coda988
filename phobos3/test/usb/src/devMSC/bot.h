/**********************************************************************
 * Copyright (C) 2014 Cadence Design Systems, Inc.
 * All rights reserved worldwide.
 ***********************************************************************
 * bot.h
 * BOT protocol header file
 ***********************************************************************/
#ifndef BOT_H
#define BOT_H

#include "cdn_stdint.h"
#include "config.h"

//-------------------------------------------------------------------------------------------
// Definitions
//-------------------------------------------------------------------------------------------
#define CBW_FLAG_DIR_MASK 0x80
#define CBW_FLAG_DIR_IN 0x80 // (to host)
#define CBW_FLAG_DIR_OUT 0x00 // (from host)

#define CBW_LUN_MASK 0x0F
#define CBW_LENGTH_MASK 0X1F

//-------------------------------------------------------------------------------------------
// Types definitions
//-------------------------------------------------------------------------------------------

typedef struct {
    uint32_t dCBWSignature;
    uint32_t dCBWTag;
    uint32_t dCBWDataTransferLength;
    uint8_t bmCBWFlags;
    uint8_t bmCBWLun;
    uint8_t bmCBWLength;
    uint8_t cbwcb[16];
} cbw_t;

typedef struct {
    uint32_t dCSWSignature;
    uint32_t dCSWTag;
    uint32_t dCSWDataResidue;
    uint8_t dCSWStatus;
} csw_t;

#endif // BOT_H


