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
 * hpnfc_main.c
 * high level hpnfc driver function
 *****************************************************************************/


#include <log.h>
#include <cdn_errno.h>
#include <hpnfc_if.h>
#include "hpnfc_core.h"
#include "hpnfc_sanity.h"
#include "hpnfc_cdma.h"




uint32_t HPNFC_Probe(uintptr_t registerBase, HPNFC_SysReq* req)
{
	uint32_t status;
	status = HPNFC_ProbeSF(registerBase, req);
	if (status)
		LogAndExit(status);

	if (registerBase == 0) {
		LogAndExit(EINVAL);
	}

	req->pDataSize = sizeof(HPNFC_State);

	return EOK;
}

void HPNFC_Start(void* privateData)
{
	if (HPNFC_StartSF(privateData) != EOK)
		return;


	HPNFC_ConfigInt(privateData, 1);
}

void HPNFC_Stop(void* privateData)
{
	if (HPNFC_StopSF(privateData) != EOK)
		return;

	HPNFC_ConfigInt(privateData, 0);
}

void HPNFC_Destroy(void* privateData)
{
	if (HPNFC_DestroySF(privateData) != EOK)
		return;
}

void HPNFC_Isr(void* privateData)
{
	if (HPNFC_IsrSF(privateData) != EOK)
		return;

	HPNFC_Core_Isr(privateData);
}

int32_t HPNFC_AssembleCdmaDesc(void* privateData, void* descPtr, HPNFC_CdmaCmd* cdmaCmd)
{
	if (HPNFC_AssembleCdmaDescSF(privateData, descPtr, cdmaCmd) != EOK)
		LogAndExit(EINVAL);

	return HPNFC_Cdma_DescPrepare(privateData, descPtr, cdmaCmd);
}

int32_t HPNFC_ExecCmdReq(void* privateData, HPNFC_CmdReq* cmdReq)
{
	int32_t status = EOK;
	printf("%s \n", __func__);

	if (HPNFC_ExecCmdReqSF(privateData, cmdReq) != EOK)
		LogAndExit(EINVAL);

	cmdReq->status = HPNFC_STATUS_PENDING;
	status = HPNFC_ReqAttach(privateData, cmdReq);
	if (status)
	{
		return status;
	}

	switch(cmdReq->reqType){
	case HPNFC_REQ_TYPE_CDMA:
		status = HPNFC_CdmaSend(privateData, cmdReq->thread, &cmdReq->req.cdma);
		break;
	case HPNFC_REQ_TYPE_PIO:
		status = HPNFC_PioSend(privateData, cmdReq->thread, &cmdReq->req.pio);
		break;
	case HPNFC_REQ_TYPE_GEN:
		status = HPNFC_GenericSend(privateData, cmdReq->thread, &cmdReq->req.generic);
		break;
	case HPNFC_REQ_TYPE_RST:
		status = HPNFC_ResetSend(privateData, cmdReq->thread, &cmdReq->req.reset);
		break;
	}

	if (status){
		HPNFC_ReqRelease(privateData, cmdReq->thread, status);
	}

	return status;

}

int32_t HPNFC_SetWorkMode(void* privateData, HPNFC_WorkMode workMode,
					  uint8_t timingMode, HPNFC_PhyConfig *phyConfig)
{
	int32_t status = HPNFC_SetWorkModeSF(privateData, workMode, timingMode,
										 phyConfig);
	if (status)
		LogAndExit(status);


	return HPNFC_Core_SetWorkMode(privateData, workMode, timingMode,
								  phyConfig);
}

int32_t HPNFC_Init(void* privateData, HPNFC_Config *config)
{
	HPNFC_State* hpnfc = privateData;
	int32_t status;

	status = HPNFC_InitSF(privateData, config);
	if (status)
		LogAndExit(status);

	memSet(hpnfc, 0, sizeof(HPNFC_State));

	if  (config->devNum == 0){
		vDbgMsg(DBG_GEN_MSG, DBG_CRIT, "%s\n", "number of devices cannot be 0");
		LogAndExit(EINVAL);
	}

	hpnfc->regAddress = config->regBase;
	hpnfc->regs = (struct hpnfc_regs*)hpnfc->regAddress;
	hpnfc->dll_phy_regs = (struct dll_phy_addr_map*)hpnfc->regAddress;
	hpnfc->minictrl_regs = (struct nfc9_minictrl_regs*)hpnfc->regAddress;

	status = HPNFC_Core_Init(hpnfc, config);
	if (status)
		LogAndExit(EIO);

	status = HPNFC_Core_GetEccConfig(hpnfc, &hpnfc->eccConfig);
	if (status){
		memSet(&hpnfc->eccConfig, 0, sizeof(hpnfc->eccConfig));
	}
	HPNFC_Core_GetDevInfo(hpnfc, &hpnfc->devInfo);

	if  (config->devNum > hpnfc->ctrlInfo.banksNumber){
		vDbgMsg(DBG_GEN_MSG, DBG_CRIT,
				"given number of devices %d is biger than number of banks %d\n",
				config->devNum, hpnfc->ctrlInfo.banksNumber);
		LogAndExit(EINVAL);
	}

	hpnfc->devNum = config->devNum;

	HPNFC_PrintDevInfo(hpnfc);

	hpnfc->slaveDmaHandler = config->slaveDmaHandler;

	return EOK;
}


int32_t HPNFC_SetEccConfig(void *privateData,
								  HPNFC_EccConfiguration *eccConfig)
{
	int32_t status;
	HPNFC_State* hpnfc = privateData;

	status = HPNFC_SetEccConfigSF(privateData, eccConfig);
	if (status)
		LogAndExit(status);

	status = HPNFC_Core_SetEccConfig(hpnfc, eccConfig);
	hpnfc->eccConfig = *eccConfig;

	return status;
}

int32_t HPNFC_GetEccConfig(void *privateData,
								  HPNFC_EccConfiguration *eccConfig)
{
	int32_t status;
	HPNFC_State* hpnfc = privateData;

	status = HPNFC_GetEccConfigSF(privateData, eccConfig);
	if (status)
		LogAndExit(status);

	*eccConfig = hpnfc->eccConfig;

	return EOK;
}


int32_t HPNFC_SetTransferConfig(void *privateData,
								   HPNFC_TransferConfig *transferCfg)
{
	int32_t status;

	status = HPNFC_SetTransferConfigSF(privateData, transferCfg);
	if (status)
		LogAndExit(status);

	return HPNFC_Core_SetTransferConfig(privateData, transferCfg);
}

int32_t HPNFC_GetTransferConfig(void *privateData,
								   HPNFC_TransferConfig *transferCfg)
{
	int32_t status;

	status = HPNFC_GetTransferConfigSF(privateData, transferCfg);
	if (status)
		LogAndExit(status);

	HPNFC_Core_GetTransferConfig(privateData, transferCfg);

	return EOK;
}

int32_t HPNFC_CdmaGetFailedDesc(void* cdmaDesc, void** failed)
{
	int32_t status;

	status = HPNFC_CdmaGetFailedDescSF(cdmaDesc, failed);
	if (status)
		LogAndExit(status);

	HPNFC_Cdma_GetFailed(cdmaDesc, (HPNFC_CdmaDesc**)failed);

	return EOK;
}

int32_t HPNFC_CdmaGetNextFailedDesc(void* cdmaDesc, void** failed)
{
	int32_t status;

	status = HPNFC_CdmaGetNextFailedDescSF(cdmaDesc, failed);
	if (status)
		LogAndExit(status);

	HPNFC_Cdma_GetNextFailed(cdmaDesc, (HPNFC_CdmaDesc**)failed);

	return EOK;
}

int32_t HPNFC_CdmaDescGetOperationResult(void* privateData, void* cdmaDesc,
									 HPNFC_OperationResult *operationResult)
{
	int32_t status;

	status = HPNFC_CdmaDescGetOperationReSF(privateData, cdmaDesc, operationResult);
	if (status)
		LogAndExit(status);


	*operationResult = HPNFC_GetOperationResult(privateData,
												((HPNFC_CdmaDesc*)cdmaDesc)->status,
												HPNFC_REQ_TYPE_CDMA);
	return EOK;
}

int32_t HPNFC_ReqGetOperationResult(void* privateData, HPNFC_CmdReq* cmdReq,
									HPNFC_OperationResult *operationResult)
{
	int32_t status;
	uint32_t lastStatus[2];

	status = HPNFC_ReqGetOperationResultSF(privateData, cmdReq, operationResult);
	if (status)
		LogAndExit(status);

	status = HPNFC_GetLastStatus(privateData, cmdReq->thread, lastStatus);
	if (status)
		LogAndExit(status);

	*operationResult = HPNFC_GetOperationResult(privateData, lastStatus,
												cmdReq->reqType);
	return EOK;
}

int32_t HPNFC_CdmaParseDesc(void* privateData, void* cdmaDesc, HPNFC_CdmaCmd* cdmaCmd)
{
	int32_t status;

	status = HPNFC_CdmaParseDescSF(privateData, cdmaDesc, cdmaCmd);
	if (status)
		LogAndExit(status);

	HPNFC_Cdma_DescParse(privateData, cdmaDesc, cdmaCmd);
	return EOK;
}

int32_t
HPNFC_SetStatusPollingConfig(void *privateData,
							 HPNFC_StatusPollingConfig *statusPollingCfg)
{
   int32_t status;

	status = HPNFC_SetStatusPollingConfigSF(privateData, statusPollingCfg);
	if (status)
		LogAndExit(status);

	return HPNFC_Core_SetStatusPollingConfig(privateData, statusPollingCfg);
}

int32_t
HPNFC_GetStatusPollingConfig(void *privateData,
							 HPNFC_StatusPollingConfig *statusPollingCfg)
{
   int32_t status;

	status = HPNFC_GetStatusPollingConfigSF(privateData, statusPollingCfg);
	if (status)
		LogAndExit(status);

	HPNFC_Core_GetStatusPollingConfig(privateData, statusPollingCfg);
	return EOK;
}

int32_t
HPNFC_SetAutoStatusConfig(void *privateData,
						  HPNFC_AutoStatusConfig *autoStatusConfig)
{
   int32_t status;

	status = HPNFC_SetAutoStatusConfigSF(privateData, autoStatusConfig);
	if (status)
		LogAndExit(status);

	return HPNFC_Core_SetAutoStatusConfig(privateData, autoStatusConfig);
}

int32_t
HPNFC_GetAutoStatusConfig(void *privateData,
						  HPNFC_AutoStatusConfig *autoStatusConfig)
{
   int32_t status;

	status = HPNFC_GetAutoStatusConfigSF(privateData, autoStatusConfig);
	if (status)
		LogAndExit(status);

	HPNFC_Core_GetAutoStatusConfig(privateData, autoStatusConfig);
	return EOK;
}


int32_t HPNFC_GetDeviceLayout(void *privateData, HPNFC_DeviceLayout *devLayout)
{
	int32_t status;

	status = HPNFC_GetDeviceLayoutSF(privateData, devLayout);
	if (status)
		LogAndExit(status);

	HPNFC_Core_GetDeviceLayout(privateData, devLayout);

	return EOK;

}

int32_t HPNFC_SetDeviceSubLayout(void *privateData, HPNFC_DeviceSubLayout *devSubLayout)
{
	int32_t status;

	status = HPNFC_SetDeviceSubLayoutSF(privateData, devSubLayout);
	if (status)
		LogAndExit(status);

	return HPNFC_Core_SetDeviceSubLayout(privateData, devSubLayout);
}

int32_t HPNFC_GetLunInterleavedConfig(void *privateData,
										 HPNFC_LunInterleavedConfig *lunInterleavedConfig)
{
	int32_t status;

	status = HPNFC_GetLunInterleavedConfiSF(privateData, lunInterleavedConfig);
	if (status)
		LogAndExit(status);

	HPNFC_Core_GetLunInterlConf(privateData, lunInterleavedConfig);

	return EOK;
}

int32_t HPNFC_SetLunInterleavedConfig(void *privateData,
										 HPNFC_LunInterleavedConfig *lunInterleavedConfig)
{
	int32_t status;

	status = HPNFC_SetLunInterleavedConfiSF(privateData, lunInterleavedConfig);
	if (status)
		LogAndExit(status);

	return HPNFC_Core_SetLunInterlConf(privateData, lunInterleavedConfig);
}

int32_t HPNFC_GetRnb(void* privateData, uint8_t chip, uint8_t *rnb)
{
	int32_t status;

	status = HPNFC_GetRnbSF(privateData, chip, rnb);
	if (status)
		LogAndExit(status);

	*rnb = HPNFC_Core_GetRnb(privateData, chip);

	return EOK;
}

int32_t HPNFC_GetBootStatus(void* privateData, HPNFC_BootStatus *bootStatus, uint8_t *cpyId)
{
	int32_t status;

	status = HPNFC_GetBootStatusSF(privateData, bootStatus, cpyId);
	if (status)
		LogAndExit(status);

	HPNFC_Core_GetBootStatus(privateData, bootStatus, cpyId);

	return EOK;
}

int32_t HPNFC_GetMultiplaneConfig(void* privateData, HPNFC_MultiplaneConfig* multiplaneConfig)
{
	int32_t status;

	status = HPNFC_GetMultiplaneConfigSF(privateData, multiplaneConfig);
	if (status)
		LogAndExit(status);

	HPNFC_Core_GetMultiplaneConfig(privateData, multiplaneConfig);

	return EOK;
}

int32_t HPNFC_SetMultiplaneConfig(void* privateData, HPNFC_MultiplaneConfig* multiplaneConfig)
{
	int32_t status;

	status = HPNFC_SetMultiplaneConfigSF(privateData, multiplaneConfig);
	if (status)
		LogAndExit(status);

	return HPNFC_Core_SetMultiplaneConfig(privateData, multiplaneConfig);
}

int32_t HPNFC_GetCacheConfig(void* privateData, HPNFC_CacheConfig* cacheConfig)
{
	int32_t status;

	status = HPNFC_GetCacheConfigSF(privateData, cacheConfig);
	if (status)
		LogAndExit(status);

	HPNFC_Core_GetCacheConfig(privateData, cacheConfig);

	return EOK;
}

int32_t HPNFC_SetCacheConfig(void* privateData, HPNFC_CacheConfig* cacheConfig)
{
	int32_t status;

	status = HPNFC_SetCacheConfigSF(privateData, cacheConfig);
	if (status)
		LogAndExit(status);

	return HPNFC_Core_SetCacheConfig(privateData, cacheConfig);
}

int32_t HPNFC_GetBchConfigInfo(void* privateData, HPNFC_BchConfigInfo* bchConfigInfo)
{
	int32_t status;
	HPNFC_State* hpnfc = privateData;

	status = HPNFC_GetBchConfigInfoSF(privateData, bchConfigInfo);
	if (status)
		LogAndExit(status);

	*bchConfigInfo = hpnfc->bchCfgInfo;

	return EOK;
}

int32_t HPNFC_GetControllerInfo(void* privateData, HPNFC_ControllerInfo* controllerInfo)
{
	int32_t status;
	HPNFC_State* hpnfc = privateData;

	status = HPNFC_GetControllerInfoSF(privateData, controllerInfo);
	if (status)
		LogAndExit(status);

	*controllerInfo = hpnfc->ctrlInfo;

	return EOK;
}

int32_t HPNFC_GetCdmaDescSize(void* privateData, uint8_t* descSize)
{
	int32_t status;
	HPNFC_State* hpnfc = privateData;

	status = HPNFC_GetCdmaDescSizeSF(privateData, descSize);
	if (status)
		LogAndExit(status);

	*descSize = hpnfc->cdmaDescSize;

	return EOK;
}

int32_t HPNFC_GetDebugData(void* privateData, HPNFC_DebugReq *dbgReq)
{
	int32_t status;

	status = HPNFC_GetDebugDataSF(privateData, dbgReq);
	if (status)
		LogAndExit(status);

	status = HPNFC_Core_GetDebugData(privateData, dbgReq);

	return status;
}

int32_t HPNFC_SetDebugIfType(void* privateData, HPNFC_DebugIfType ifType)
{
	int32_t status;

	status = HPNFC_SetDebugIfTypeSF(privateData, ifType);
	if (status)
		LogAndExit(status);

	return HPNFC_Core_SetDebugIfType(privateData, ifType);
}

int32_t HPNFC_GetDebugIfType(void* privateData, HPNFC_DebugIfType* ifType)
{
	int32_t status;

	status = HPNFC_GetDebugIfTypeSF(privateData, ifType);
	if (status)
		LogAndExit(status);

	return HPNFC_Core_GetDebugIfType(privateData, ifType);
}

int32_t HPNFC_SetDiConfig(void* privateData, HPNFC_DiConfig* diConfig)
{
	int32_t status;

	status = HPNFC_SetDiConfigSF(privateData, diConfig);
	if (status)
		LogAndExit(status);

	return HPNFC_Core_SetDiConfig(privateData, diConfig);
}

int32_t HPNFC_GetDiConfig(void* privateData, HPNFC_DiConfig* diConfig)
{
	int32_t status;

	status = HPNFC_GetDiConfigSF(privateData, diConfig);
	if (status)
		LogAndExit(status);

	return HPNFC_Core_GetDiConfig(privateData, diConfig);
}

int32_t HPNFC_SetDiErrInjConfig(void *privateData, HPNFC_DiErrInjConfig* diErrInjConfig)
{
	int32_t status;

	status = HPNFC_SetDiErrInjConfigSF(privateData, diErrInjConfig);
	if (status)
		LogAndExit(status);

	return HPNFC_Core_SetDiErrInjConfig(privateData, diErrInjConfig);
}

int32_t HPNFC_GetDiErrInjConfig(void* privateData, HPNFC_DiErrInjConfig* diErrInjConfig)
{
	int32_t status;

	status = HPNFC_GetDiErrInjConfigSF(privateData, diErrInjConfig);
	if (status)
		LogAndExit(status);

	return HPNFC_Core_GetDiErrInjConfig(privateData, diErrInjConfig);
}

int32_t HPNFC_SetDmaConfig(void* privateData, HPNFC_DmaConfig* dmaConfig)
{
	int32_t status;

	status = HPNFC_SetDmaConfigSF(privateData, dmaConfig);
	if (status)
		LogAndExit(status);

	return HPNFC_Core_SetDmaConfig(privateData, dmaConfig);
}

int32_t HPNFC_GetDmaConfig(void* privateData, HPNFC_DmaConfig* dmaConfig)
{
	int32_t status;

	status = HPNFC_GetDmaConfigSF(privateData, dmaConfig);
	if (status)
		LogAndExit(status);

	return HPNFC_Core_GetDmaConfig(privateData, dmaConfig);
}

int32_t HPNFC_WriteRemapRecord(void* privateData,
							   HPNFC_RemapRecord* remapRecord, uint8_t memTrg)
{
	int32_t status;

	status = HPNFC_WriteRemapRecordSF(privateData, remapRecord, memTrg);
	if (status)
		LogAndExit(status);

	return HPNFC_Core_WriteRemapRecord(privateData, remapRecord, memTrg);
}

int32_t HPNFC_ReadRemapRecord(void* privateData, uint16_t recordIdx,
							  HPNFC_RemapRecord* remapRecord, uint8_t memTrg)
{
	int32_t status;

	status = HPNFC_ReadRemapRecordSF(privateData, recordIdx, remapRecord, memTrg);
	if (status)
		LogAndExit(status);

	return HPNFC_Core_ReadRemapRecord(privateData, recordIdx, remapRecord, memTrg);
}

int32_t HPNFC_ClearRemapTable(void* privateData)
{
	int32_t status;

	status = HPNFC_ClearRemapTableSF(privateData);
	if (status)
		LogAndExit(status);

	return HPNFC_Core_ClearRemapTable(privateData);

}

int32_t HPNFC_GetRemapRecordCount(void* privateData, uint16_t* recordCount)
{
	int32_t status;

	status = HPNFC_GetRemapRecordCountSF(privateData, recordCount);
	if (status)
		LogAndExit(status);

	return HPNFC_Core_GetRemapRecordCount(privateData, recordCount);
}

int32_t HPNFC_SetRemapMask(void* privateData, uint32_t remapMask)
{
	int32_t status;

	status = HPNFC_SetRemapMaskSF(privateData, remapMask);
	if (status)
		LogAndExit(status);

	return HPNFC_Core_SetRemapMask(privateData, remapMask);
}

int32_t HPNFC_GetRemapMask(void* privateData, uint32_t* remapMask)
{
	int32_t status;

	status = HPNFC_GetRemapMaskSF(privateData, remapMask);
	if (status)
		LogAndExit(status);

	return HPNFC_Core_GetRemapMask(privateData, remapMask);
}

int32_t HPNFC_SetRemapEnable(void* privateData, uint8_t remapEnable)
{
	int32_t status;

	status = HPNFC_SetRemapEnableSF(privateData, remapEnable);
	if (status)
		LogAndExit(status);

	return HPNFC_Core_SetRemapEnable(privateData, remapEnable);
}

int32_t HPNFC_GetRemapEnable(void* privateData, uint8_t* remapEnable)
{
	int32_t status;

	status = HPNFC_GetRemapEnableSF(privateData, remapEnable);
	if (status)
		LogAndExit(status);

	return HPNFC_Core_GetRemapEnable(privateData, remapEnable);
}

int32_t HPNFC_SetPSlcConfig(void* privateData, HPNFC_PSlcConfig* pSlcConfig)
{
	int32_t status;

	status = HPNFC_SetPSlcConfigSF(privateData, pSlcConfig);
	if (status)
		LogAndExit(status);

	return HPNFC_Core_SetPSlcConfig(privateData, pSlcConfig);
}


int32_t HPNFC_GetPSlcConfig(void* privateData, HPNFC_PSlcConfig* pSlcConfig)
{
	int32_t status;

	status = HPNFC_GetPSlcConfigSF(privateData, pSlcConfig);
	if (status)
		LogAndExit(status);

	return HPNFC_Core_GetPSlcConfig(privateData, pSlcConfig);
}

int32_t HPNFC_SetZqCalConfig(void* privateData, HPNFC_ZqCalConfig* zqCalConfig)
{
	int32_t status;

	status = HPNFC_SetZqCalConfigSF(privateData, zqCalConfig);
	if (status)
		LogAndExit(status);

	return HPNFC_Core_SetZqCalConfig(privateData, zqCalConfig);
}

int32_t HPNFC_GetZqCalConfig(void* privateData, HPNFC_ZqCalConfig* zqCalConfig)
{
	int32_t status;

	status = HPNFC_GetZqCalConfigSF(privateData, zqCalConfig);
	if (status)
		LogAndExit(status);

	return HPNFC_Core_GetZqCalConfig(privateData, zqCalConfig);
}

int32_t HPNFC_SetZqCalEnable(void* privateData, uint8_t target, uint8_t enable)
{
	int32_t status;

	status = HPNFC_SetZqCalEnableSF(privateData, target, enable);
	if (status)
		LogAndExit(status);

	return HPNFC_Core_SetZqCalEnable(privateData, target, enable);
}

int32_t HPNFC_GetZqCalEnable(void* privateData, uint8_t target, uint8_t* enable)
{
	int32_t status;

	status = HPNFC_GetZqCalEnableSF(privateData, target, enable);
	if (status)
		LogAndExit(status);

	return HPNFC_Core_GetZqCalEnable(privateData, target, enable);
}

int32_t HPNFC_GetZqStat(void* privateData, uint16_t* calStat)
{
	int32_t status;

	status = HPNFC_GetZqStatSF(privateData, calStat);
	if (status)
		LogAndExit(status);

	return HPNFC_Core_GetZqStat(privateData, calStat);
}

int32_t HPNFC_ClearZqStat(void* privateData, uint16_t calStatToClear)
{
	int32_t status;

	status = HPNFC_ClearZqStatSF(privateData, calStatToClear);
	if (status)
		LogAndExit(status);

	return HPNFC_Core_ClearZqStat(privateData, calStatToClear);
}


int32_t HPNFC_SetTimeoutConfig(void* privateData, HPNFC_TimeoutConfig* timeoutConfig)
{
	int32_t status;

	status = HPNFC_SetTimeoutConfigSF(privateData, timeoutConfig);
	if (status)
		LogAndExit(status);

	return HPNFC_Core_SetTimeoutConfig(privateData, timeoutConfig);
}

int32_t HPNFC_GetTimeoutConfig(void* privateData, HPNFC_TimeoutConfig* timeoutConfig)
{
	int32_t status;

	status = HPNFC_GetTimeoutConfigSF(privateData, timeoutConfig);
	if (status)
		LogAndExit(status);

	return HPNFC_Core_GetTimeoutConfig(privateData, timeoutConfig);
}


int32_t HPNFC_SetCePinReduction(void* privateData, uint8_t enable)
{
	int32_t status;

	status = HPNFC_SetCePinReductionSF(privateData, enable);
	if (status)
		LogAndExit(status);

	return HPNFC_Core_SetCePinReduction(privateData, enable);
}

int32_t HPNFC_GetCePinReduction(void* privateData, uint8_t* enabled)
{
	int32_t status;

	status = HPNFC_GetCePinReductionSF(privateData, enabled);
	if (status)
		LogAndExit(status);

	return HPNFC_Core_GetCePinReduction(privateData, enabled);
}

int32_t HPNFC_GetDeviceInfo(void* privateData, HPNFC_DevInfo* deviceInfo)
{
	int32_t status;

	status = HPNFC_GetDeviceInfoSF(privateData, deviceInfo);
	if (status)
		LogAndExit(status);

	return HPNFC_Core_GetDevInfo(privateData, deviceInfo);
}

