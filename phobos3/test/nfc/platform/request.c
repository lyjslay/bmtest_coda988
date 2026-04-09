/******************************************************************************
 * Copyright (C) 2015-2017 Cadence Design Systems, Inc.
 * All rights reserved worldwide.
 *
 * The material contained herein is the proprietary and confidential
 * information of Cadence or its licensors, and is supplied subject to, and may
 * be used only by Cadence's customer in accordance with a previously executed
 * license and maintenance agreement between Cadence and that customer.
 *
 ******************************************************************************
 * request.c
 *
 * HPNFC request assemble and execute
 *****************************************************************************/

#include <string.h>
#include "system_common.h"
#include <hpnfc_if.h>
#include <hpnfc_obj_if.h>
#include "request.h"
#include <cps.h>
#include <timer.h>

static uint32_t cdmaDesc[8][HPNFC_CDMA_DESC_WITH_DUMMY_SIZE / 4] __attribute__((aligned(8)));

uint8_t intEn = 1;

int ReqWaitForFinish(void *privateData, HPNFC_CmdReq *cmdReq)
{
	HPNFC_OBJ *drv = HPNFC_GetInstance();
	while(cmdReq->status == HPNFC_STATUS_PENDING){
		if (!intEn)
			drv->isr(privateData);
	}
	return 0;
}

void CdmaPrepareCmdReq(void *privateData, HPNFC_CdmaCmd *cdmaCmd,
						  HPNFC_CmdReq *cmdReq, uint8_t thread)
{
	HPNFC_OBJ *drv = HPNFC_GetInstance();
	//HPNFC_OperationResult operationResult;

	//printf("prepare CDMA command %x, desc ptr %p\n", cdmaCmd->cmdType, &cdmaDesc[thread][0]);
	/* assemble CDMA descriptor base on CDMA command */
	drv->assembleCdmaDesc(privateData, &cdmaDesc[thread][0], cdmaCmd);
	CPS_CacheFlush((uintptr_t)&cdmaDesc[thread][0], HPNFC_CDMA_DESC_WITH_DUMMY_SIZE, 0);

	/* prepare command request */
	memset(cmdReq, 0, sizeof(*cmdReq));
	cmdReq->req.cdma.descBuf = &cdmaDesc[thread][0];
	cmdReq->req.cdma.descDmaAddr = (uintptr_t)&cdmaDesc[thread][0];
	cmdReq->thread = thread;
	cmdReq->reqType = HPNFC_REQ_TYPE_CDMA;
}

/* function prepares and executes CDMA request
 * when operation result status is not 0 then
 * function returns it as a result
 * so result of this function can be either standard error codes from
 * cdn_errno.h file or enum values from the HPNFC_OperationStatus type*/
int CdmaExecCmdReq(void *privateData, HPNFC_CdmaCmd *cdmaCmd,
					  HPNFC_CmdReq *cmdReq, uint8_t thread)
{
	HPNFC_OBJ *drv = HPNFC_GetInstance();
	HPNFC_OperationResult operationResult;
	int status;

	CdmaPrepareCmdReq(privateData, cdmaCmd, cmdReq, thread);

	//printf("execute CDMA command %x\n", cdmaCmd->cmdType);

	/* execute command request*/
	status = drv->execCmdReq(privateData, cmdReq);
	if (status)
		return status;

	status = ReqWaitForFinish(privateData, cmdReq);
	if (status)
		return status;

	/* if command request finished with error then check error details */
	if (cmdReq->status){
		printf("Command execution failed: status %d\n", cmdReq->status);
		drv->cdmaDescGetOperationResult(privateData, cmdReq->req.cdma.descBuf, &operationResult);
		if ((cmdReq->status != 0) && ((operationResult.status == 0) || (operationResult.status == HPNFC_STATUS_CMD_UNFINISHED))){
			drv->reqGetOperationResult(privateData, cmdReq, &operationResult);
		}
		switch(operationResult.status){
		case HPNFC_STATUS_DEV_ERR:
			printf("-- status: device error\n");
			break;
		case HPNFC_STATUS_TIMEOUT_ERR:
			printf("-- status: timeout error\n");
			break;
		case HPNFC_STATUS_ECC_CORR_ERR:
			printf("-- status: ECC correctable error\n-- bits fixed: %d\n", operationResult.maxCorrErr);
			break;
		case HPNFC_STATUS_ECC_UNCORR_ERR:
			printf("-- status: ECC uncorrectable error\n-- page erased: %d\n", operationResult.erased);
			int i;
			for (i = 0; i < 8; i++){
				if (operationResult.planesEccError & (1 << i))
					printf("-- error occurred on plane: %d\n", i);
			}
			break;
		case HPNFC_STATUS_CMD_ERR:
			printf("-- status: descriptor error\n");
			break;
		default:
			printf("-- status: other %d\n", status);
			break;
		}
		printf("-- operation number: %d\n", operationResult.errIdx);

		printf("operationResult.status %d, cmdReq->status %d\n", operationResult.status , cmdReq->status);
		if (operationResult.status)
			return operationResult.status;
		else
			return cmdReq->status;
	}


	return 0;
}

int CdmaExecReadPage(void *privateData, uint8_t bank, uint32_t rowAddr, uint32_t pageCnt,  void *buffer, uint8_t thread)
{
	int status;
	HPNFC_CdmaCmd cdmaCmd;
	HPNFC_CmdReq cmdReq;
	printf("[NFC_TEST] %s rowAddr 0x%x\n", __func__, rowAddr);

	memset(&cdmaCmd, 0, sizeof(HPNFC_CdmaCmd));

	cdmaCmd.bank = bank;
	cdmaCmd.rowAddr = rowAddr;
	cdmaCmd.cmdCount = pageCnt;
	cdmaCmd.cmdType = HPNFC_CDMA_CMD_TYPE_READ;
	cdmaCmd.dmaMode = HPNFC_DMA_MODE_MASTER;
	cdmaCmd.memPtr = buffer;
	status = CdmaExecCmdReq(privateData, &cdmaCmd, &cmdReq, 0);
	if (status){
		printf("%s read operation failed\n", __func__);
		return status;
	}

	return 0;
}

int CdmaExecProgramPage(void *privateData, uint8_t bank, uint32_t rowAddr, uint32_t pageCnt, void *buffer, uint8_t thread)
{
	int status;
	HPNFC_CdmaCmd cdmaCmd;
	HPNFC_CmdReq cmdReq;
	printf("[NFC_TEST] %s rowAddr 0x%x\n", __func__, rowAddr);

	memset(&cdmaCmd, 0, sizeof(HPNFC_CdmaCmd));

	cdmaCmd.bank = bank;
	cdmaCmd.rowAddr = rowAddr;
	cdmaCmd.cmdCount = pageCnt;
	cdmaCmd.cmdType = HPNFC_CDMA_CMD_TYPE_PROGRAM;
	cdmaCmd.dmaMode = HPNFC_DMA_MODE_MASTER;
	cdmaCmd.memPtr = buffer;
	status = CdmaExecCmdReq(privateData, &cdmaCmd, &cmdReq, 0);
	if (status){
		printf("%s program operation failed\n", __func__);
		return status;
	}

	return 0;
}


int CdmaExecEraseBlock(void *privateData, uint8_t bank, uint32_t rowAddr, uint32_t blkCnt)
{
	int status;
	HPNFC_CdmaCmd cdmaCmd;
	HPNFC_CmdReq cmdReq;
	printf("[NFC_TEST] %s rowAddr 0x%x\n", __func__, rowAddr);

	memset(&cdmaCmd, 0, sizeof(HPNFC_CdmaCmd));

	cdmaCmd.bank = bank;
	cdmaCmd.rowAddr = rowAddr;
	cdmaCmd.cmdCount = blkCnt;
	cdmaCmd.cmdType = HPNFC_CDMA_CMD_TYPE_ERASE;

	status = CdmaExecCmdReq(privateData, &cdmaCmd, &cmdReq, 0);
	if (status){
		printf("%s erase operation failed\n", __func__);
		return status;
	}
	return 0;
}

int PioExecSetFeatures(void *privateData, uint8_t featAddr,
					   uint32_t featVal, uint8_t bank)
{
	HPNFC_OBJ *drv = HPNFC_GetInstance();
	HPNFC_CmdReq cmdReq;
	int status;

	HPNFC_PioReq *req = &cmdReq.req.pio;

	cmdReq.thread = bank;
	cmdReq.reqType = HPNFC_REQ_TYPE_PIO;
	cmdReq.status = HPNFC_STATUS_PENDING;

	req->featureAddr = featAddr;
	req->bank = bank;
	req->featureValue = featVal;
	req->volume = 0;
	req->cmdType = HPNFC_PIO_CMD_TYPE_SET_FEATURES;

	/* execute command request*/
	status = drv->execCmdReq(privateData, &cmdReq);
	if (status)
		return status;

	status = ReqWaitForFinish(privateData, &cmdReq);
	if (status)
		return status;

	return cmdReq.status;
}

int PioExecEraseBlock(void *privateData, uint8_t bank,
					  uint32_t rowAddr, uint32_t count)
{
	HPNFC_OBJ *drv = HPNFC_GetInstance();
	HPNFC_CmdReq cmdReq;
	HPNFC_PioReq *req = &cmdReq.req.pio;
	int status;

	cmdReq.thread = 0;
	cmdReq.reqType = HPNFC_REQ_TYPE_PIO;
	cmdReq.status = HPNFC_STATUS_PENDING;

	req->rowAddr = rowAddr;
	req->bank = bank;
	req->volume = 0;
	req->cmdType = HPNFC_PIO_CMD_TYPE_ERASE;
	req->cmdCount = count;

	/* execute command request*/
	status = drv->execCmdReq(privateData, &cmdReq);
	if (status)
		return status;

	status = ReqWaitForFinish(privateData, &cmdReq);
	if (status)
		return status;

	return cmdReq.status;
}

int PioExecProgramPage(void* privateData, uint8_t bank,
					   uint32_t rowAddr, uint32_t count,
					   void *buffer,
					   HPNFC_DmaMode dmaMode)
{
	HPNFC_OBJ *drv = HPNFC_GetInstance();
	HPNFC_CmdReq cmdReq;
	HPNFC_PioReq *req = &cmdReq.req.pio;
	int status;

	cmdReq.thread = 0;
	cmdReq.reqType = HPNFC_REQ_TYPE_PIO;
	cmdReq.status = HPNFC_STATUS_PENDING;

	req->rowAddr = rowAddr;
	req->bank = bank;
	req->volume = 0;
	req->cmdType = HPNFC_PIO_CMD_TYPE_PROGRAM;
	req->memPtr = buffer;
	req->dmaMode =dmaMode;
	req->cmdCount = count;

	/* execute command request*/
	status = drv->execCmdReq(privateData, &cmdReq);
	if (status)
		return status;

	status = ReqWaitForFinish(privateData, &cmdReq);
	if (status)
		return status;

	return cmdReq.status;
}

int PioExecReadPage(void* privateData, uint8_t bank,
					uint32_t rowAddr, uint32_t count,
					void *buffer,
					HPNFC_DmaMode dmaMode)
{
	HPNFC_OBJ *drv = HPNFC_GetInstance();
	HPNFC_CmdReq cmdReq;
	HPNFC_PioReq *req = &cmdReq.req.pio;
	int status;

	cmdReq.thread = 0;
	cmdReq.reqType = HPNFC_REQ_TYPE_PIO;
	cmdReq.status = HPNFC_STATUS_PENDING;

	req->rowAddr = rowAddr;
	req->bank = bank;
	req->volume = 0;
	req->cmdType = HPNFC_PIO_CMD_TYPE_READ;
	req->memPtr = buffer;
	req->dmaMode = dmaMode;
	req->cmdCount = count;

	/* execute command request*/
	status = drv->execCmdReq(privateData, &cmdReq);
	if (status)
		return status;

	status = ReqWaitForFinish(privateData, &cmdReq);
	if (status)
		return status;

	return cmdReq.status;
}

void GenAssembleCmdSeq(void *privateData, HPNFC_CmdReq *cmdReq,
					   uint8_t chipSelect, uint8_t cmd,
					   uint8_t thread)
{
	HPNFC_GenReq *genCmd = &cmdReq->req.generic;

	memset(cmdReq, 0, sizeof(*cmdReq));

	cmdReq->thread = thread;
	cmdReq->reqType = HPNFC_REQ_TYPE_GEN;

	genCmd->cmdType = HPNFC_GEN_CMD_TYPE_CMD_SEQ;
	genCmd->twbActive = 0;
	genCmd->input.cmdSeq = cmd;
	genCmd->chipSelect = chipSelect;
}


void GenAssembleAddrSeq(void *privateData, HPNFC_CmdReq *cmdReq,
						uint8_t chipSelect, uint8_t thread,
						uint64_t address, uint8_t addressSize)
{
	HPNFC_GenReq *genCmd = &cmdReq->req.generic;

	memset(cmdReq, 0, sizeof(*cmdReq));

	cmdReq->thread = thread;
	cmdReq->reqType = HPNFC_REQ_TYPE_GEN;

	genCmd->input.addrSeq.address = address;
	genCmd->input.addrSeq.addrSize = addressSize;

	genCmd->cmdType = HPNFC_GEN_CMD_TYPE_ADDR_SEQ;
	genCmd->twbActive = 0;
	genCmd->chipSelect = chipSelect;
}



/* assemble generic cmd with fixed address */
void GenAssembleCmdWithAddr(void *privateData, HPNFC_CmdReq *cmdReq,
							uint8_t chipSelect, uint8_t cmd,
							uint64_t address,  uint8_t thread,
							uint8_t twb)
{
	HPNFC_GenReq *genCmd = &cmdReq->req.generic;

	memset(cmdReq, 0, sizeof(*cmdReq));

	cmdReq->thread = thread;
	cmdReq->reqType = HPNFC_REQ_TYPE_GEN;

	genCmd->cmdType = cmd;
	genCmd->twbActive = twb;
	genCmd->input.addr = address;
	genCmd->chipSelect = chipSelect;
}

/* assemble data sequence */
void GenAssembleDataSeq(void *privateData, HPNFC_CmdReq *cmdReq,
						uint8_t chipSelect, HPNFC_GenReqInputData *data,
						uint8_t thread)
{
	HPNFC_GenReq *genCmd = &cmdReq->req.generic;

	memset(cmdReq, 0, sizeof(*cmdReq));

	genCmd->input.dataSeq = *data;

	cmdReq->thread = thread;
	cmdReq->reqType = HPNFC_REQ_TYPE_GEN;

	genCmd->cmdType = HPNFC_GEN_CMD_TYPE_DATA_SEQ;
	genCmd->twbActive = 0;
	genCmd->chipSelect = chipSelect;

}

int GenExecDataSeq(void *privateData,  HPNFC_GenReqInputData *data,
				   uint8_t chipSelect, uint8_t thread)
{
	int status;
	HPNFC_OBJ *drv = HPNFC_GetInstance();
	HPNFC_CmdReq cmdReq;

	GenAssembleDataSeq(privateData, &cmdReq, chipSelect, data, thread);

	/* execute command request*/
	status = drv->execCmdReq(privateData, &cmdReq);
	if (status)
		return status;
	status = ReqWaitForFinish(privateData, &cmdReq);
	if (status)
		return status;

	return cmdReq.status;

}

//int GenExecChangeReadCmd(void *privateData, uint8_t chipSelect,
//                         uint16_t offset, uint8_t thread)
//{
//    int status;
//    HPNFC_OBJ *drv = HPNFC_GetInstance();
//    HPNFC_CmdReq cmdReq;
//
//    GenAssembleCmdWithAddr(privateData, &cmdReq, chipSelect,
//                           HPNFC_GEN_CMD_TYPE_READ_COLUMN,
//                           offset, thread, 0);
//
//    /* execute command request*/
//    status = drv->execCmdReq(privateData, &cmdReq);
//    if (status)
//        return status;
//    status = ReqWaitForFinish(privateData, &cmdReq);
//    if (status)
//        return status;
//
//    return cmdReq.status;
//}

//int GenExecChangeWriteCmd(void *privateData, uint8_t chipSelect,
//                          uint16_t offset, uint8_t thread)
//{
//    int status;
//    HPNFC_OBJ *drv = HPNFC_GetInstance();
//    HPNFC_CmdReq cmdReq;
//
//    GenAssembleCmdWithAddr(privateData, &cmdReq, chipSelect,
//                           HPNFC_GEN_CMD_TYPE_WRITE_COLUMN,
//                           offset, thread, 0);
//
//    /* execute command request*/
//    status = drv->execCmdReq(privateData, &cmdReq);
//    if (status)
//        return status;
//    status = ReqWaitForFinish(privateData, &cmdReq);
//    if (status)
//        return status;
//
//    return cmdReq.status;
//}

//int GenExecReadID_(void *privateData, uint8_t chipSelect, uint8_t thread)
//{
//    int status;
//    HPNFC_OBJ *drv = HPNFC_GetInstance();
//    HPNFC_CmdReq cmdReq;
//
//    GenAssembleCmdWithAddr(privateData, &cmdReq, chipSelect,
//                           HPNFC_GEN_CMD_TYPE_READ_ID,
//                           0, thread, 1);
//
//    /* execute command request*/
//    status = drv->execCmdReq(privateData, &cmdReq);
//    if (status)
//        return status;
//
//    status = ReqWaitForFinish(privateData, &cmdReq);
//    if (status)
//        return status;
//
//    return cmdReq.status;
//}


/* read page using generic mode */
//int GenExecReadID(void *privateData, uint8_t bank,
//                  void *buffer, uint32_t buffSize)
//{
//    int status;
//    //uint8_t sdmaTrdNum;
//    //uint32_t sdmaSize;
//    //uint32_t tmp[2];
//    const uint8_t subSize = 4;
//    uint32_t offset = 0;
//    uint32_t size = buffSize;
//    HPNFC_GenReqInputData dataSeq;
//    HPNFC_OBJ *drv = HPNFC_GetInstance();
//    uint8_t rnb;
//
//    status = GenExecReadID_(privateData, 0, 0);
//    if (status)
//        return status;
//
//    memset(&dataSeq, 0, sizeof(dataSeq));
//
//    dataSeq.lastSectorSize = buffSize;
//    dataSeq.transferDirection = HPNFC_DATA_TRANSFER_READ;
//    dataSeq.sectorCount = 1;
//    dataSeq.dataPtr = buffer;
//
//    do {
//        drv->getRnb(privateData, 0, &rnb);
//    }
//    while(!rnb);
//
//
//    /* prepare controller to read data in generic mode */
//    status = GenExecDataSeq(privateData, &dataSeq, bank, 0);
//    if (status)
//        return status;
//
//    buffer += subSize;
//    dataSeq.dataPtr = buffer;
//
//    size -= subSize;
//    offset += subSize;
//    return 0;
//}


int GenExecReadParamPage(void *privateData, uint8_t chipSelect, uint8_t thread)
{
	int status;
	HPNFC_OBJ *drv = HPNFC_GetInstance();
	HPNFC_CmdReq cmdReq;

	GenAssembleCmdWithAddr(privateData, &cmdReq, chipSelect,
						   HPNFC_GEN_CMD_TYPE_READ_PARAM_PAGE,
						   0, thread, 1);

	/* execute command request*/
	status = drv->execCmdReq(privateData, &cmdReq);
	if (status)
		return status;

	status = ReqWaitForFinish(privateData, &cmdReq);
	if (status)
		return status;

	return cmdReq.status;
}

int GenExecEraseBlock(void *privateData, uint8_t chipSelect, uint32_t rowAddr)
{
	int status;
	uint8_t rnb;
	HPNFC_OBJ *drv = HPNFC_GetInstance();
	HPNFC_CmdReq cmdReq;
	printf("%s cs %d, row addr 0x%x\n", __func__, chipSelect, rowAddr);

	GenAssembleCmdWithAddr(privateData, &cmdReq, chipSelect, HPNFC_GEN_CMD_TYPE_ERASE,
						   rowAddr, 0, 1);

	/* execute command request*/
	status = drv->execCmdReq(privateData, &cmdReq);
	if (status)
	{
		printf("%s err 1\n", __func__);
		return status;
	}

	status = ReqWaitForFinish(privateData, &cmdReq);
	if (status)
	{
		printf("%s err 2\n", __func__);
		return status;
	}

	do {
		drv->getRnb(privateData, 0, &rnb);
	}
	while(!rnb);

	return cmdReq.status;
}

#define NAND_CMD_UNCLOCK1 0x23
#define NAND_CMD_UNCLOCK2 0x24


int GenExecReadPage_(void *privateData, uint8_t chipSelect, uint8_t rowAddr, uint8_t thread)
{
	int status;
	HPNFC_OBJ *drv = HPNFC_GetInstance();
	HPNFC_CmdReq cmdReq;
	printf("%s \n", __func__);

	GenAssembleCmdWithAddr(privateData, &cmdReq, chipSelect, HPNFC_GEN_CMD_TYPE_READ,
						   rowAddr, thread, 1);

	/* execute command request*/
	status = drv->execCmdReq(privateData, &cmdReq);
	if (status)
	{
		return status;
	}

	status = ReqWaitForFinish(privateData, &cmdReq);
	if (status)
	{
		return status;
	}

	return cmdReq.status;
}

int GenExecWritePage(void *privateData, uint8_t chipSelect, uint8_t rowAddr, uint8_t thread)
{
	int status;
	HPNFC_OBJ *drv = HPNFC_GetInstance();
	HPNFC_CmdReq cmdReq;

	GenAssembleCmdWithAddr(privateData, &cmdReq, chipSelect, HPNFC_GEN_CMD_TYPE_WRITE,
						   rowAddr, thread, 1);

	/* execute command request*/
	status = drv->execCmdReq(privateData, &cmdReq);
	if (status)
		return status;
	status = ReqWaitForFinish(privateData, &cmdReq);
	if (status)
		return status;

	return cmdReq.status;
}

int GenExecProgPage(void *privateData, uint8_t chipSelect, uint8_t thread)
{
	int status;
	uint8_t rnb;
	HPNFC_OBJ *drv = HPNFC_GetInstance();
	HPNFC_CmdReq cmdReq;
	printf("%s \n", __func__);

	GenAssembleCmdSeq(privateData, &cmdReq, chipSelect, 0x10, thread);

	/* execute command request*/
	status = drv->execCmdReq(privateData, &cmdReq);
	if (status)
	{
		printf("%s err 1\n", __func__);
		return status;
	}

	status = ReqWaitForFinish(privateData, &cmdReq);
	if (status)
	{
		printf("%s err 2\n", __func__);
		return status;
	}

	do {
		drv->getRnb(privateData, 0, &rnb);
	}
	while(!rnb);

	return cmdReq.status;

}


/* read parameter page using generic mode */


int GenExecProgramPage(void *privateData, uint8_t bank, uint32_t rowAddr,
					   void *buffer, uint32_t buffSize)
{
	int status;
	HPNFC_GenReqInputData dataSeq;
	printf("%s row addr 0x%x\n", __func__, rowAddr);

	status = GenExecWritePage(privateData, bank, rowAddr, 0);
	if (status)
	{
		return status;
	}

	memset(&dataSeq, 0, sizeof(dataSeq));

#if 1
	dataSeq.corrCap = 8;
	dataSeq.sectorSize = 512;
	dataSeq.lastSectorSize = 512;
	dataSeq.sectorCount = 4;
	dataSeq.erasePageDetectEn = 0;
	dataSeq.eccEn = 1;
	dataSeq.scramblerEnabled = 0;
	dataSeq.transferDirection = HPNFC_DATA_TRANSFER_WRITE;
	dataSeq.dataPtr = buffer;
	dataSeq.transferredData = 0;
#else
	dataSeq.lastSectorSize = buffSize;
	dataSeq.sectorCount = 1;
	dataSeq.transferDirection = HPNFC_DATA_TRANSFER_WRITE;
	dataSeq.dataPtr = buffer;
#endif

	printf("&dataSeq.transferredData 0x%p\n", &dataSeq.transferredData);

	/* prepare controller to read data in generic mode */
	status = GenExecDataSeq(privateData, &dataSeq, bank, 0);
	if (status)
	{
		return status;
	}

	printf("transferred data size %d\n", dataSeq.transferredData);

	status = GenExecProgPage(privateData, bank, 0);
	if (status)
	{
		printf("%s err 3\n", __func__);
		return status;
	}


	return 0;
}

/* read page using generic mode */
int GenExecReadPage(void *privateData, uint8_t bank, uint32_t rowAddr,
					void *buffer, uint32_t buffSize)
{
	int status;
	HPNFC_GenReqInputData dataSeq;
	HPNFC_OBJ *drv = HPNFC_GetInstance();
	uint8_t rnb;
	printf("%s row addr 0x%x\n", __func__, rowAddr);

	status = GenExecReadPage_(privateData, bank, rowAddr, 0);
	if (status)
	{
		return status;
	}

	memset(&dataSeq, 0, sizeof(dataSeq));

	//dataSeq.lastSectorSize = buffSize;
	//dataSeq.transferDirection = HPNFC_DATA_TRANSFER_READ;
	//dataSeq.sectorCount = 1;
	//dataSeq.dataPtr = buffer;

#if 1
	dataSeq.corrCap = 8;
	dataSeq.sectorSize = 512;
	dataSeq.lastSectorSize = 512;
	dataSeq.sectorCount = 4;
	dataSeq.erasePageDetectEn = 0;
	dataSeq.eccEn = 1;
	dataSeq.scramblerEnabled = 0;
	dataSeq.transferDirection = HPNFC_DATA_TRANSFER_READ;
	dataSeq.dataPtr = buffer;
	dataSeq.transferredData = 0;
#else
	/* Original code use only 1 sector and its size is 2048, might encounter ECC */
	dataSeq.lastSectorSize = buffSize;
	dataSeq.sectorCount = 1;
	dataSeq.transferDirection = HPNFC_DATA_TRANSFER_READ;
	dataSeq.dataPtr = buffer;
#endif

	do {
		drv->getRnb(privateData, 0, &rnb);
	}
	while(!rnb);

	/* prepare controller to read data in generic mode */
	status = GenExecDataSeq(privateData, &dataSeq, bank, 0);
	if (status)
	{
		printf("%s err 2\n", __func__);
		return status;
	}

	printf("transferred data size %d\n", dataSeq.transferredData);

	printf("pass %s\n", __func__);
	return 0;
}