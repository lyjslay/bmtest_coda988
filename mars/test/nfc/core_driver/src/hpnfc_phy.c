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
 * hpnfc_phy.c
 * PHY configuration module
 *****************************************************************************/



#include <cdn_inttypes.h>
#include <hpnfc_if.h>
#include <log.h>
#include "hpnfc_core.h"


void HPNFC_PhyPrintPhyConfig(HPNFC_PhyConfig *phyConfig)
{
	DbgMsg(DBG_GEN_MSG, DBG_FYI, "-- phyCtrl - %08"PRIX32"\n", phyConfig->phyCtrl);
	DbgMsg(DBG_GEN_MSG, DBG_FYI, "-- phyTsel - %08"PRIX32"\n", phyConfig->phyTsel);
	DbgMsg(DBG_GEN_MSG, DBG_FYI, "-- phyDqTiming - %08"PRIX32"\n", phyConfig->phyDqTiming);
	DbgMsg(DBG_GEN_MSG, DBG_FYI, "-- phyDqsTiming - %08"PRIX32"\n", phyConfig->phyDqsTiming);
	DbgMsg(DBG_GEN_MSG, DBG_FYI, "-- phyGateLpbkCtrl - %08"PRIX32"\n", phyConfig->phyGateLpbkCtrl);
	DbgMsg(DBG_GEN_MSG, DBG_FYI, "-- phyDllMasterCtrl - %08"PRIX32"\n", phyConfig->phyDllMasterCtrl);
	DbgMsg(DBG_GEN_MSG, DBG_FYI, "-- phyDllSlaveCtrl - %08"PRIX32"\n", phyConfig->phyDllSlaveCtrl);
	DbgMsg(DBG_GEN_MSG, DBG_FYI, "-- toggleTimings0 - %08"PRIX32"\n", phyConfig->toggleTimings0);
	DbgMsg(DBG_GEN_MSG, DBG_FYI, "-- toggleTimings1 - %08"PRIX32"\n", phyConfig->toggleTimings1);
	DbgMsg(DBG_GEN_MSG, DBG_FYI, "-- asyncToggleTimings - %08"PRIX32"\n", phyConfig->asyncToggleTimings);
	DbgMsg(DBG_GEN_MSG, DBG_FYI, "-- timings0 - %08"PRIX32"\n", phyConfig->timings0);
	DbgMsg(DBG_GEN_MSG, DBG_FYI, "-- timings1 - %08"PRIX32"\n", phyConfig->timings1);
	DbgMsg(DBG_GEN_MSG, DBG_FYI, "-- timings2 - %08"PRIX32"\n", phyConfig->timings2);
	DbgMsg(DBG_GEN_MSG, DBG_FYI, "-- dllPhyCtrl - %08"PRIX32"\n", phyConfig->dllPhyCtrl);
}

void HPNFC_PhyGetPhyConfig(HPNFC_State *hpnfc, HPNFC_PhyConfig *phyConfig)
{
	if (hpnfc->ctrlInfo.ddrSupport || hpnfc->ctrlInfo.ddr2Support){
		phyConfig->phyCtrl = HPNFC_GetReg((uintptr_t)&(hpnfc->dll_phy_regs->phy_ctb.phy_ctrl_reg));
		phyConfig->phyTsel = HPNFC_GetReg((uintptr_t)&(hpnfc->dll_phy_regs->phy_ctb.phy_tsel_reg));
		phyConfig->phyDqTiming = HPNFC_GetReg((uintptr_t)&(hpnfc->dll_phy_regs->phy_dataslice.phy_dq_timing_reg));
		phyConfig->phyDqsTiming = HPNFC_GetReg((uintptr_t)&(hpnfc->dll_phy_regs->phy_dataslice.phy_dqs_timing_reg));
		phyConfig->phyGateLpbkCtrl = HPNFC_GetReg((uintptr_t)&(hpnfc->dll_phy_regs->phy_dataslice.phy_gate_lpbk_ctrl_reg));
		phyConfig->phyDllMasterCtrl = HPNFC_GetReg((uintptr_t)&(hpnfc->dll_phy_regs->phy_dataslice.phy_dll_master_ctrl_reg));
		phyConfig->phyDllSlaveCtrl = HPNFC_GetReg((uintptr_t)&(hpnfc->dll_phy_regs->phy_dataslice.phy_dll_slave_ctrl_reg));
	}

	phyConfig->toggleTimings0 = HPNFC_GetReg((uintptr_t)&(hpnfc->minictrl_regs->minictrl_settings.toggle_timings_0));
	phyConfig->toggleTimings1 = HPNFC_GetReg((uintptr_t)&(hpnfc->minictrl_regs->minictrl_settings.toggle_timings_1));
	phyConfig->asyncToggleTimings = HPNFC_GetReg((uintptr_t)&(hpnfc->minictrl_regs->minictrl_settings.async_toggle_timings));
	phyConfig->timings0 = HPNFC_GetReg((uintptr_t)&(hpnfc->minictrl_regs->minictrl_settings.timings0));
	phyConfig->timings1 = HPNFC_GetReg((uintptr_t)&(hpnfc->minictrl_regs->minictrl_settings.timings1));
	phyConfig->timings2 = HPNFC_GetReg((uintptr_t)&(hpnfc->minictrl_regs->minictrl_settings.timings2));
	if (hpnfc->ctrlInfo.ddrSupport || hpnfc->ctrlInfo.ddr2Support){
		phyConfig->dllPhyCtrl = HPNFC_GetReg((uintptr_t)&(hpnfc->minictrl_regs->minictrl_settings.dll_phy_ctrl));
	}

	//HPNFC_PhyPrintPhyConfig(phyConfig);
}

void HPNFC_PhySetPhyConfig(HPNFC_State *hpnfc, HPNFC_PhyConfig *phyConfig)
{
	DbgMsg(DBG_GEN_MSG, DBG_FYI, "%s", "Set new PHY settings\n");
	HPNFC_PhyPrintPhyConfig(phyConfig);

	if (hpnfc->ctrlInfo.ddrSupport || hpnfc->ctrlInfo.ddr2Support){
		HPNFC_SetReg((uintptr_t)&(hpnfc->dll_phy_regs->phy_ctb.phy_ctrl_reg), phyConfig->phyCtrl);
		HPNFC_SetReg((uintptr_t)&(hpnfc->dll_phy_regs->phy_ctb.phy_tsel_reg), phyConfig->phyTsel);
		HPNFC_SetReg((uintptr_t)&(hpnfc->dll_phy_regs->phy_dataslice.phy_dq_timing_reg), phyConfig->phyDqTiming);
		HPNFC_SetReg((uintptr_t)&(hpnfc->dll_phy_regs->phy_dataslice.phy_dqs_timing_reg), phyConfig->phyDqsTiming);
		HPNFC_SetReg((uintptr_t)&(hpnfc->dll_phy_regs->phy_dataslice.phy_gate_lpbk_ctrl_reg), phyConfig->phyGateLpbkCtrl);
		HPNFC_SetReg((uintptr_t)&(hpnfc->dll_phy_regs->phy_dataslice.phy_dll_master_ctrl_reg), phyConfig->phyDllMasterCtrl);
		HPNFC_SetReg((uintptr_t)&(hpnfc->dll_phy_regs->phy_dataslice.phy_dll_slave_ctrl_reg), phyConfig->phyDllSlaveCtrl);
	}

	HPNFC_SetReg((uintptr_t)&(hpnfc->minictrl_regs->minictrl_settings.toggle_timings_0), phyConfig->toggleTimings0);
	HPNFC_SetReg((uintptr_t)&(hpnfc->minictrl_regs->minictrl_settings.toggle_timings_1), phyConfig->toggleTimings1);

	HPNFC_SetReg((uintptr_t)&(hpnfc->minictrl_regs->minictrl_settings.async_toggle_timings), phyConfig->asyncToggleTimings);
	HPNFC_SetReg((uintptr_t)&(hpnfc->minictrl_regs->minictrl_settings.timings0), phyConfig->timings0);
	HPNFC_SetReg((uintptr_t)&(hpnfc->minictrl_regs->minictrl_settings.timings1), phyConfig->timings1);
	HPNFC_SetReg((uintptr_t)&(hpnfc->minictrl_regs->minictrl_settings.timings2), phyConfig->timings2);
	if (hpnfc->ctrlInfo.ddrSupport || hpnfc->ctrlInfo.ddr2Support){
		HPNFC_SetReg((uintptr_t)&(hpnfc->minictrl_regs->minictrl_settings.dll_phy_ctrl), phyConfig->dllPhyCtrl);
	}
}

void HPNFC_PhyWriteDllReset(HPNFC_State *hpnfc, uint8_t dllReset)
{
	if (hpnfc->ctrlInfo.ddrSupport || hpnfc->ctrlInfo.ddr2Support){
		uint32_t reg;
		reg = HPNFC_GetReg((uintptr_t)&(hpnfc->minictrl_regs->minictrl_settings.dll_phy_ctrl));

		if (dllReset){
			reg |= RF_MINICTRL_REGS__RT_DLL_PHY_CTRL__DLL_RST_N__MASK;
		}
		else {
			reg &= ~RF_MINICTRL_REGS__RT_DLL_PHY_CTRL__DLL_RST_N__MASK;
		}
		HPNFC_SetReg((uintptr_t)&(hpnfc->minictrl_regs->minictrl_settings.dll_phy_ctrl), reg);
	}
}

