/**********************************************************************
 * copyright (C) 2015-2016 Cadence Design Systems
 * All rights reserved.
 ***********************************************************************
 * request.h
 *
 * HPNFC request assemble and execute
 ***********************************************************************/

#ifndef REQUEST_H
#define REQUEST_H


#include <hpnfc_if.h>

int ReqWaitForFinish(void *privateData, HPNFC_CmdReq *cmdReq);

void CdmaPrepareCmdReq(void *privateData, HPNFC_CdmaCmd *cdmaCmd,
                       HPNFC_CmdReq *cmdReq, uint8_t thread);

int CdmaExecCmdReq(void *privateData, HPNFC_CdmaCmd *cdmaCmd,
                   HPNFC_CmdReq *cmdReq, uint8_t thread);

int CdmaExecReadPage(void *privateData, uint8_t bank, uint32_t rowAddr,
                     uint32_t pageCnt,  void *buffer, uint8_t thread);

int CdmaExecProgramPage(void *privateData, uint8_t bank, uint32_t rowAddr,
                        uint32_t pageCnt, void *buffer, uint8_t thread);

int CdmaExecEraseBlock(void *privateData, uint8_t bank, uint32_t rowAddr,
                       uint32_t blkCnt);

int GenExecEraseBlock(void *privateData, uint8_t chipSelect,
                      uint32_t rowAddr);

int GenExecProgramPage(void *privateData, uint8_t bank, uint32_t rowAddr,
                       void *buffer, uint32_t buffSize);

int GenExecReadPage(void *privateData, uint8_t bank, uint32_t rowAddr,
                    void *buffer, uint32_t buffSize);

int GenExecProgramPageSplit(void *privateData, uint8_t bank, uint32_t rowAddr,
                              void *buffer, uint32_t buffSize);

int GenExecReadPageSplit(void *privateData, uint8_t bank, uint32_t rowAddr,
                           void *buffer, uint32_t buffSize);

int PioExecEraseBlock(void *privateData,
                      uint8_t bank, uint32_t rowAddr, uint32_t count);

int PioExecProgramPage(void* privateData, uint8_t bank,
                       uint32_t rowAddr, uint32_t count, void *buffer,
                       HPNFC_DmaMode dmaMode);

int PioExecReadPage(void* privateData, uint8_t bank,
                    uint32_t rowAddr, uint32_t count, void *buffer,
                    HPNFC_DmaMode dmaMode);

int GenUnlockCmd(void *privateData, uint8_t chipSelect, uint8_t rowAddrUp,
                 uint8_t rowAddrLo);

int GenExecReadID(void *privateData, uint8_t bank,
                  void *buffer, uint32_t buffSize);

int PioExecSetFeatures(void *privateData, uint8_t featAddr,
                       uint32_t featVal, uint8_t bank);

void GenAssembleCmdWithAddr(void *privateData, HPNFC_CmdReq *cmdReq,
                        uint8_t chipSelect, uint8_t cmd,
                        uint64_t address, uint8_t thread,
                        uint8_t twb);
#endif
