/**********************************************************************
 * Copyright (C) 2014 Cadence Design Systems, Inc.
 * All rights reserved worldwide.
 ***********************************************************************
 * scsi.h
 * SCSI parser header file
 ***********************************************************************/
#ifndef SCSI_H
#define SCSI_H

#include "cdn_stdint.h"


// SCSI Errors
#define ERR_SCSI_OK              0
#define ERR_SCSI_UNKNOWN_COMMAND 5
#define ERR_SCSI_DAMAGED_COMMAND 6
#define ERR_SCSI_INVALID_DATA_SIZE 7

// USB receiver/sender
extern uint32_t(*scsi_send_data)(void *, uint32_t);
extern uint32_t(*scsi_rec_data)(void *, uint32_t);

// keeps number of bytes required by device to transfer
extern uint32_t dev_num_of_bytes;

// keeps direction of data required to transfer: 1-IN, 0-OUT
extern uint8_t dev_data_dir;
//-------------------------------------------------------------------------------------------

// initializes SCSI device
void scsiInit(void);

// executes SCSI command
uint16_t scsiExecCmd(uint8_t *Cmd, uint32_t *retBytes);
//-------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------
#endif // SCSI_H
//-------------------------------------------------------------------------------------------


