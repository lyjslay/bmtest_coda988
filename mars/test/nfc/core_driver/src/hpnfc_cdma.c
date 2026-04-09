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
 * hpnfc_cmda.c
 * Command DMA module
 *****************************************************************************/

#include <cdn_inttypes.h>
#include <cdn_errno.h>
#include <log.h>
#include <cps.h>
#include <hpnfc_if.h>
#include "hpnfc_cdma.h"
#include "hpnfc_core.h"


void DumpDescriptor(HPNFC_CdmaDesc* cdmaDesc)
{
	uint32_t *desc32 = (uint32_t*)cdmaDesc;
	int32_t i;

	//DbgMsg(DBG_GEN_MSG, DBG_HIVERB, "desc addr %p\n", cdmaDesc);
	printf("desc addr %p\n", cdmaDesc);
	/* do not print32_t dummy part of descriptor if exists */
	for (i = 0; i < HPNFC_CDMA_DESC_SIZE/ 4; i += 2) {
		//DbgMsg(DBG_GEN_MSG, DBG_HIVERB, "%08"PRIx32": %08"PRIX32"_%08"PRIX32"\n", i * 4, desc32[i + 1], desc32[i]);
		printf("%08"PRIx32": %08"PRIX32"_%08"PRIX32"\n", i * 4, desc32[i + 1], desc32[i]);
	}
	//printf(DBG_GEN_MSG, DBG_HIVERB, "%s", "\n");
	//DbgMsg(DBG_GEN_MSG, DBG_HIVERB, "%s", "\n");
}

static void HPNFC_Cdma_SyncParse(HPNFC_CdmaDesc* cdmaDesc,
								 HPNFC_CdmaSyncConfig* syncConfig)
{
	uint32_t syncFlags = cdmaDesc->syncArguments;

	if (!(syncFlags & HPNFC_CDMA_SYNC_VALID_MASK))
		return;

	syncConfig->syncEnable = 1;

	if (syncFlags & HPNFC_CDMA_SYNC_GREAT_MASK){
		syncConfig->startCondition = HPNFC_CDMA_SYNC_CFG_SC_GREATER;
	}
	else {
		syncConfig->startCondition = HPNFC_CDMA_SYNC_CFG_SC_EQUAL;
	}

	if (syncFlags & HPNFC_CDMA_SYNC_INC_MASK){
		syncConfig->endOperation = HPNFC_CDMA_SYNC_CFG_EO_INCREMENT;
	}
	else {
		syncConfig->endOperation = HPNFC_CDMA_SYNC_CFG_EO_STORE;
		syncConfig->endValue = HPNFC_CDMA_SYNC_EV_READ(syncFlags);
	}


	syncConfig->startValue = HPNFC_CDMA_SYNC_SV_READ(syncFlags);

	syncConfig->syncBuffAddr = cdmaDesc->syncFlagPointer;
}



void HPNFC_Cdma_SyncPrepare(HPNFC_CdmaDesc* cdmaDesc,
							HPNFC_CdmaSyncConfig* syncConfig)
{
	uint32_t syncFlags;
	if (!syncConfig->syncEnable)
		return;

	syncFlags = HPNFC_CDMA_SYNC_VALID_MASK;
	if (syncConfig->startCondition == HPNFC_CDMA_SYNC_CFG_SC_GREATER)
		syncFlags |= HPNFC_CDMA_SYNC_GREAT_MASK;


	if (syncConfig->endOperation == HPNFC_CDMA_SYNC_CFG_EO_STORE){
		syncFlags |= HPNFC_CDMA_SYNC_EV_WRITE(syncConfig->endValue);
	}
	else if (syncConfig->endOperation == HPNFC_CDMA_SYNC_CFG_EO_INCREMENT){
		syncFlags |= HPNFC_CDMA_SYNC_INC_MASK;
	}

	syncFlags |= HPNFC_CDMA_SYNC_SV_WRITE(syncConfig->startValue);

	cdmaDesc->syncArguments = syncFlags;
	cdmaDesc->syncFlagPointer = syncConfig->syncBuffAddr;
}

static void HPNFC_Cdma_SetCmdCnt(HPNFC_State *hpnfc, HPNFC_CdmaDesc* cdmaDesc,
								 HPNFC_CdmaCmd* cdmaCmd)
{
	if (hpnfc->ctrlInfo.extCmdCnt == 1){
		cdmaDesc->commandCounter = (cdmaCmd->cmdCount - 1) & HPNFC_MAX_EXT_CMD_COUNT_MASK;
	}
	else {
		cdmaDesc->commandType |= (cdmaCmd->cmdCount - 1) & HPNFC_MAX_CMD_COUNT_MASK;
	}
}


int32_t HPNFC_Cdma_DescPrepare(HPNFC_State *hpnfc, HPNFC_CdmaDesc* cdmaDesc,
						   HPNFC_CdmaCmd* cdmaCmd)
{
	uint32_t maxCmdCnt = HPNFC_MAX_CMD_COUNT;
	if (hpnfc->ctrlInfo.extCmdCnt == 1)
		maxCmdCnt = HPNFC_MAX_EXT_CMD_COUNT;


	if (cdmaCmd->cmdCount > maxCmdCnt){
		vDbgMsg(DBG_GEN_MSG, DBG_CRIT, "Error: Exceeded maximum command count %"PRId32"\n", cdmaCmd->cmdCount);
		LogAndExit(EINVAL);
	}

	memSet(cdmaDesc, 0, hpnfc->cdmaDescSize);

	cdmaDesc->flashPointer = ((uint32_t)cdmaCmd->bank << HPNFC_CDMA_CFPTR_MEM_SHIFT)
		+ cdmaCmd->rowAddr;

	switch (cdmaCmd->dmaMode){
	case HPNFC_DMA_MODE_MASTER:
		cdmaDesc->commandFlags |= HPNFC_CDMA_CF_DMA_MASTER;
		break;
	case HPNFC_DMA_MODE_SLAVE:
		break;
	default:
		LogAndExit(EINVAL);
	}

	cdmaDesc->commandFlags |= HPNFC_CDMA_CF_INT;

	cdmaDesc->commandFlags |= ((cdmaCmd->volume << HPNFC_CDMA_CF_VOL_SHIFT)
								& HPNFC_CDMA_CF_VOL_MASK);

	if (cdmaCmd->nextPtr){
		cdmaDesc->nextPointer = cdmaCmd->nextPtr;
		cdmaDesc->commandFlags |= HPNFC_CDMA_CF_CONT;
	}

	if (cdmaCmd->flashPtrCont)
		cdmaDesc->commandFlags |= HPNFC_CDMA_CF_FL_PTR_CON;

	if (cdmaCmd->memPtrCont)
		cdmaDesc->commandFlags |= HPNFC_CDMA_CF_MEM_PTR_CON;


	HPNFC_Cdma_SyncPrepare(cdmaDesc, &cdmaCmd->syncConfig);


	switch (cdmaCmd->cmdType){
	case HPNFC_CDMA_CMD_TYPE_ERASE:
		cdmaDesc->commandType = HPNFC_CDMA_CT_ERASE;
		HPNFC_Cdma_SetCmdCnt(hpnfc, cdmaDesc, cdmaCmd);
		break;
	case HPNFC_CDMA_CMD_TYPE_COPY_BACK:
		cdmaDesc->memoryPointer =
			((uint32_t)cdmaCmd->dstBank << HPNFC_CDMA_CFPTR_MEM_SHIFT)
			+ cdmaCmd->dstRowAddr;
		cdmaDesc->commandType = HPNFC_CDMA_CT_CPYB;
		HPNFC_Cdma_SetCmdCnt(hpnfc, cdmaDesc, cdmaCmd);
		break;
	case HPNFC_CDMA_CMD_TYPE_PROGRAM:
		cdmaDesc->memoryPointer = (uintptr_t)(cdmaCmd->memPtr);
		cdmaDesc->commandType = HPNFC_CDMA_CT_WR;
		HPNFC_Cdma_SetCmdCnt(hpnfc, cdmaDesc, cdmaCmd);
		break;
	case HPNFC_CDMA_CMD_TYPE_READ:
		cdmaDesc->memoryPointer = (uintptr_t)(cdmaCmd->memPtr);
		cdmaDesc->commandType = HPNFC_CDMA_CT_RD;
		HPNFC_Cdma_SetCmdCnt(hpnfc, cdmaDesc, cdmaCmd);
		break;
	case HPNFC_CDMA_CMD_TYPE_RESET:
		cdmaDesc->commandType = HPNFC_CDMA_CT_RST;
		break;
	case HPNFC_CDMA_CMD_TYPE_NOP:
		cdmaDesc->commandType = HPNFC_CDMA_CT_NOP;
		break;
	default:
		LogAndExit(EINVAL);
	}

	CPS_CacheFlush((uintptr_t)cdmaDesc, hpnfc->cdmaDescSize, 0);

	return (EOK);
}

static void HPNFC_Cdma_GetCmdCnt(HPNFC_State *hpnfc, HPNFC_CdmaDesc* cdmaDesc,
								 HPNFC_CdmaCmd* cdmaCmd)
{
	if (hpnfc->ctrlInfo.extCmdCnt == 1){
		cdmaCmd->cmdCount =
			(cdmaDesc->commandCounter & HPNFC_MAX_EXT_CMD_COUNT_MASK) + 1;
	}
	else {
		cdmaCmd->cmdCount =
			(cdmaDesc->commandType & HPNFC_MAX_CMD_COUNT_MASK) + 1;
	}
}

void HPNFC_Cdma_DescParse(HPNFC_State *hpnfc, HPNFC_CdmaDesc* cdmaDesc, HPNFC_CdmaCmd* cdmaCmd)
{
	if (cdmaDesc != NULL) {
		memSet(cdmaCmd, 0, sizeof(HPNFC_CdmaCmd));

		cdmaCmd->bank = (cdmaDesc->flashPointer & HPNFC_CDMA_CFPTR_MEM_MASK)
			>> HPNFC_CDMA_CFPTR_MEM_SHIFT;

		if (cdmaDesc->commandFlags & HPNFC_CDMA_CF_DMA_MASTER)
			cdmaCmd->dmaMode = HPNFC_DMA_MODE_MASTER;
		else
			cdmaCmd->dmaMode = HPNFC_DMA_MODE_SLAVE;

		cdmaCmd->volume = (cdmaDesc->commandFlags & HPNFC_CDMA_CF_VOL_MASK)
			>> HPNFC_CDMA_CF_VOL_SHIFT;

		if (cdmaDesc->commandFlags & HPNFC_CDMA_CF_CONT){
			cdmaCmd->nextPtr = cdmaDesc->nextPointer;
		}

		if (cdmaDesc->commandFlags & HPNFC_CDMA_CF_FL_PTR_CON)
			cdmaCmd->flashPtrCont = 1;

		if (cdmaDesc->commandFlags & HPNFC_CDMA_CF_MEM_PTR_CON)
			cdmaCmd->memPtrCont = 1;


		switch (cdmaDesc->commandType & ~0xFF){
		case HPNFC_CDMA_CT_ERASE:
			cdmaCmd->cmdType = HPNFC_CDMA_CMD_TYPE_ERASE;
			HPNFC_Cdma_GetCmdCnt(hpnfc, cdmaDesc, cdmaCmd);
			break;
		case HPNFC_CDMA_CT_CPYB:
			cdmaCmd->dstBank = (cdmaDesc->memoryPointer & HPNFC_CDMA_CFPTR_MEM_MASK)
				>> HPNFC_CDMA_CFPTR_MEM_SHIFT;
			cdmaCmd->dstRowAddr = cdmaDesc->memoryPointer & HPNFC_CDMA_CFPTR_ROW_MASK;
			HPNFC_Cdma_GetCmdCnt(hpnfc, cdmaDesc, cdmaCmd);
			cdmaCmd->cmdType = HPNFC_CDMA_CMD_TYPE_COPY_BACK;
			break;
		case HPNFC_CDMA_CT_WR:
			HPNFC_Cdma_GetCmdCnt(hpnfc, cdmaDesc, cdmaCmd);
			cdmaCmd->cmdType = HPNFC_CDMA_CMD_TYPE_PROGRAM;
			cdmaCmd->memPtr = (void*)(uintptr_t)cdmaDesc->memoryPointer;
			//printf("memoryPointer %p\n ", cdmaDesc->memoryPointer);
			//printf("memPtr %p\n ", cdmaCmd->memPtr);
			break;
		case HPNFC_CDMA_CT_RD:
			HPNFC_Cdma_GetCmdCnt(hpnfc, cdmaDesc, cdmaCmd);
			cdmaCmd->cmdType = HPNFC_CDMA_CMD_TYPE_READ;
			cdmaCmd->memPtr = (void*)(uintptr_t)cdmaDesc->memoryPointer;
			break;
		case HPNFC_CDMA_CT_RST:
			cdmaCmd->cmdType = HPNFC_CDMA_CMD_TYPE_RESET;
			break;
		}

		if (cdmaDesc->commandType == HPNFC_CDMA_CT_NOP)
			cdmaCmd->cmdType = HPNFC_CDMA_CMD_TYPE_NOP;

		HPNFC_Cdma_SyncParse(cdmaDesc, &cdmaCmd->syncConfig);
	}
}


/* fast check all descriptors in chain to check if all finished
 * and if any error appear */

int32_t HPNFC_Cdma_CheckChainStatus(HPNFC_CdmaDesc* cdmaDesc)
{
	int32_t status;
	uint8_t isLast = 0;

	DbgMsg(DBG_GEN_MSG, DBG_HIVERB, "Check chain descriptor status desc:%p\n", cdmaDesc);
	do {
		/* do not care dummy part of descriptor if exists, controller does not write it */
		CPS_CacheInvalidate((uintptr_t)cdmaDesc, HPNFC_CDMA_DESC_SIZE, 0);
		//DumpDescriptor(cdmaDesc);

		if (cdmaDesc->status[0] & (HPNFC_OPER_STATUS_FAIL_MASK | HPNFC_OPER_STATUS_MAXERR_MASK))
			status = EIO;
		else if (cdmaDesc->status[0] & HPNFC_OPER_STATUS_COMP_MASK)
			status = EOK;
		else
			status = HPNFC_STATUS_PENDING;

		if (cdmaDesc->commandFlags & HPNFC_CDMA_CF_CONT) {
			/* not last descriptor */
			cdmaDesc =
				(HPNFC_CdmaDesc*)(uintptr_t)cdmaDesc->nextPointer;
		}
		else {
			isLast = 1;
		}
		/*stop on first error or after all descriptors are checked*/
	} while ((isLast == 0) && (status == 0));

	return (status);
}


void HPNFC_Cdma_GetFailed(HPNFC_CdmaDesc* cdmaDesc, HPNFC_CdmaDesc** failed)
{
	int32_t status;
	*failed = NULL;

	do {
		/* do not care dummy part of descriptor if exists, controller does not write it */
		CPS_CacheInvalidate((uintptr_t)cdmaDesc, HPNFC_CDMA_DESC_SIZE, 0);

		if (cdmaDesc->status[0] & HPNFC_OPER_STATUS_COMP_MASK)
			status = EOK;
		else if (cdmaDesc->status[0] & HPNFC_OPER_STATUS_FAIL_MASK)
			status = EIO;
		else
			status = HPNFC_STATUS_CMD_UNFINISHED;

		if (status != EOK){
			*failed = cdmaDesc;
		}

		/* not last descriptor */
		if (cdmaDesc->commandFlags & HPNFC_CDMA_CF_CONT)
			cdmaDesc = (HPNFC_CdmaDesc*)(uintptr_t)cdmaDesc->nextPointer;
	} while ((cdmaDesc->commandFlags & HPNFC_CDMA_CF_CONT) && (status == EOK));
}

void HPNFC_Cdma_GetNextFailed(HPNFC_CdmaDesc* cdmaDesc, HPNFC_CdmaDesc** failed)
{
	*failed = NULL;
	/* do not care dummy part of descriptor if exists, controller does not write it */
	CPS_CacheInvalidate((uintptr_t)cdmaDesc, HPNFC_CDMA_DESC_SIZE, 0);
	if (cdmaDesc->commandFlags & HPNFC_CDMA_CF_CONT) {
		/* not last descriptor */
		cdmaDesc = (HPNFC_CdmaDesc*)(uintptr_t)cdmaDesc->nextPointer;

		return (HPNFC_Cdma_GetFailed(cdmaDesc, failed));
	}
}

HPNFC_CdmaDesc* HPNFC_Cdma_GetUnfinished(HPNFC_CdmaDesc* cdmaDesc)
{
	HPNFC_CdmaDesc *unfinished = NULL;

	do {
		/* do not care dummy part of descriptor if exists, controller does not write it */
		CPS_CacheInvalidate((uintptr_t)cdmaDesc, HPNFC_CDMA_DESC_SIZE, 0);
		if ((cdmaDesc->status[0]
			& (HPNFC_OPER_STATUS_COMP_MASK | HPNFC_OPER_STATUS_FAIL_MASK)) == 0){
			unfinished = cdmaDesc;
			break;
		}

		/* not last descriptor */
		if (cdmaDesc->commandFlags & HPNFC_CDMA_CF_CONT)
			cdmaDesc = (HPNFC_CdmaDesc*)(uintptr_t)cdmaDesc->nextPointer;
		else
			cdmaDesc = NULL;
	} while (cdmaDesc);
	return (unfinished);
}
