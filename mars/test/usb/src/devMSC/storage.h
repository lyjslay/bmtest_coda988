/**********************************************************************
 * Copyright (C) 2014 Cadence Design Systems, Inc.
 * All rights reserved worldwide.
 ***********************************************************************
 * storage.h
 * RAM Disk device storage header file
 ***********************************************************************/

#ifndef Storage_H
#define Storage_H

#include "cdn_stdint.h"

// only one physical storage can be active at the time

#define DISK_FLASH 0
#define DISK_RAM   1


#if DISK_RAM

#define SECTOR_SIZE 512
#define TRANSPORT_CHUNK_SIZE 0x10000
#define LUN_COUNT 1

#ifdef WINDOWS
#define SECTOR_COUNT 4000
#else
#define SECTOR_COUNT 64
#endif

#endif

#if DISK_FLASH

#define SECTOR_SIZE 512
#define TRANSPORT_CHUNK_SIZE 4096
#define LUN_COUNT 1
#define SECTOR_COUNT 134217728   // 64GB

#endif


// definition typical for all types of storages

// All errors
#define ERR_NO_ERRORS 0

// Device specific errors
#define ERR_INVALID_LUN 1
#define ERR_INVALID_SECTOR 2
#define ERR_NOT_READY 3
#define ERR_VERIFY_ERROR 4

// USB receiver/sender
extern uint32_t(*storage_send_data)(uint32_t start_sec, uint32_t num_of_sec);
extern uint32_t(*storage_rec_data)(uint32_t start_sec, uint32_t num_of_sec);

//extern uint8_t *MemBuffer;
//------------------------------------------------------------------------------
// Driver interface
//------------------------------------------------------------------------------
uint16_t devInit(void);
uint16_t devDeinit(void);
uint16_t devPowerUp(void);
uint16_t devPowerDown(void);
uint16_t devGetMaxLun(uint8_t *MaxLUN);
uint16_t devGetCapacities(uint8_t LUN, uint32_t *SectorCount,
        uint16_t *BytesPerSector);
uint16_t devReadSector(uint8_t LUN, uint32_t StartSector, uint32_t SectorCount,
        uint32_t * BytesRead);
uint16_t devWriteSector(uint8_t LUN, uint32_t StartSector,
        uint32_t SectorCount, uint32_t * BytesWritten);
uint16_t devVerifySector(uint8_t LUN, uint32_t StartSector,
        uint32_t SectorCount);
uint16_t devFormatUnit(uint8_t LUN, uint16_t Interleave);
uint16_t devTestUnitReady(uint8_t LUN);

//------------------------------------------------------------------------------
#endif // Storage_H
//------------------------------------------------------------------------------

