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
 * hpnfc_phy.h
 * PHY configuration module
 *****************************************************************************/


#ifndef HPNFC_PHY_H
#define HPNFC_PHY_H
#include <hpnfc_if.h>

void HPNFC_PhyPrintPhyConfig(HPNFC_PhyConfig *phyConfig);
void HPNFC_PhyGetPhyConfig(HPNFC_State *hpnfc, HPNFC_PhyConfig *phyConfig);
void HPNFC_PhyWriteDllReset(HPNFC_State *hpnfc, uint8_t dllReset);
void HPNFC_PhySetPhyConfig(HPNFC_State *hpnfc, HPNFC_PhyConfig *phyConfig);

#endif
