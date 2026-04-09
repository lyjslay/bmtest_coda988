/**********************************************************************
 * Copyright (C) 2015-2017 Cadence Design Systems, Inc.
 * All rights reserved worldwide.
 **********************************************************************
 * WARNING: This file is auto-generated using api-generator utility.
 *          Do not edit it manually.
 **********************************************************************/

#include "hpnfc_obj_if.h"


HPNFC_OBJ *HPNFC_GetInstance(void)
{
	static HPNFC_OBJ driver =
	{
		.probe = HPNFC_Probe,
		.init = HPNFC_Init,
		.isr = HPNFC_Isr,
		.start = HPNFC_Start,
		.stop = HPNFC_Stop,
		.destroy = HPNFC_Destroy,
		.assembleCdmaDesc = HPNFC_AssembleCdmaDesc,
		.execCmdReq = HPNFC_ExecCmdReq,
		.setWorkMode = HPNFC_SetWorkMode,
		.setEccConfig = HPNFC_SetEccConfig,
		.getEccConfig = HPNFC_GetEccConfig,
		.setTransferConfig = HPNFC_SetTransferConfig,
		.getTransferConfig = HPNFC_GetTransferConfig,
		.cdmaGetFailedDesc = HPNFC_CdmaGetFailedDesc,
		.cdmaGetNextFailedDesc = HPNFC_CdmaGetNextFailedDesc,
		.cdmaDescGetOperationResult = HPNFC_CdmaDescGetOperationResult,
		.reqGetOperationResult = HPNFC_ReqGetOperationResult,
		.cdmaParseDesc = HPNFC_CdmaParseDesc,
		.setStatusPollingConfig = HPNFC_SetStatusPollingConfig,
		.getStatusPollingConfig = HPNFC_GetStatusPollingConfig,
		.setAutoStatusConfig = HPNFC_SetAutoStatusConfig,
		.getAutoStatusConfig = HPNFC_GetAutoStatusConfig,
		.getDeviceLayout = HPNFC_GetDeviceLayout,
		.setDeviceSubLayout = HPNFC_SetDeviceSubLayout,
		.setLunInterleavedConfig = HPNFC_SetLunInterleavedConfig,
		.getLunInterleavedConfig = HPNFC_GetLunInterleavedConfig,
		.getRnb = HPNFC_GetRnb,
		.getBootStatus = HPNFC_GetBootStatus,
		.getMultiplaneConfig = HPNFC_GetMultiplaneConfig,
		.setMultiplaneConfig = HPNFC_SetMultiplaneConfig,
		.getCacheConfig = HPNFC_GetCacheConfig,
		.setCacheConfig = HPNFC_SetCacheConfig,
		.getBchConfigInfo = HPNFC_GetBchConfigInfo,
		.getControllerInfo = HPNFC_GetControllerInfo,
		.getCdmaDescSize = HPNFC_GetCdmaDescSize,
		.getDebugData = HPNFC_GetDebugData,
		.setDebugIfType = HPNFC_SetDebugIfType,
		.getDebugIfType = HPNFC_GetDebugIfType,
		.setDiConfig = HPNFC_SetDiConfig,
		.getDiConfig = HPNFC_GetDiConfig,
		.setDiErrInjConfig = HPNFC_SetDiErrInjConfig,
		.getDiErrInjConfig = HPNFC_GetDiErrInjConfig,
		.setDmaConfig = HPNFC_SetDmaConfig,
		.getDmaConfig = HPNFC_GetDmaConfig,
		.writeRemapRecord = HPNFC_WriteRemapRecord,
		.readRemapRecord = HPNFC_ReadRemapRecord,
		.clearRemapTable = HPNFC_ClearRemapTable,
		.getRemapRecordCount = HPNFC_GetRemapRecordCount,
		.setRemapMask = HPNFC_SetRemapMask,
		.getRemapMask = HPNFC_GetRemapMask,
		.setRemapEnable = HPNFC_SetRemapEnable,
		.getRemapEnable = HPNFC_GetRemapEnable,
		.setPSlcConfig = HPNFC_SetPSlcConfig,
		.getPSlcConfig = HPNFC_GetPSlcConfig,
		.setZqCalConfig = HPNFC_SetZqCalConfig,
		.getZqCalConfig = HPNFC_GetZqCalConfig,
		.setZqCalEnable = HPNFC_SetZqCalEnable,
		.getZqCalEnable = HPNFC_GetZqCalEnable,
		.getZqStat = HPNFC_GetZqStat,
		.clearZqStat = HPNFC_ClearZqStat,
		.setTimeoutConfig = HPNFC_SetTimeoutConfig,
		.getTimeoutConfig = HPNFC_GetTimeoutConfig,
		.setCePinReduction = HPNFC_SetCePinReduction,
		.getCePinReduction = HPNFC_GetCePinReduction,
		.getDeviceInfo = HPNFC_GetDeviceInfo,
	};

	return &driver;
}
