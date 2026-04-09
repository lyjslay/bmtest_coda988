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
 * hpnfc_core.c
 * low level access routines
 *****************************************************************************/


#include <cdn_errno.h>
#include <cdn_inttypes.h>
#include <cps.h>
#include <log.h>
#include <timer.h>

#include <hpnfc_if.h>
#include "hpnfc_core.h"
#include "hpnfc_pio.h"
#include "hpnfc_phy.h"
#include "hpnfc_cdma.h"
#include "hpnfc_gen.h"

#include "hpnfc_regs.h"
#include "hpnfc_regs_macro.h"
#include "nfc9_minictrl_regs.h"
#include "nfc9_minictrl_regs_macro.h"

#include "dll_phy_addr_map.h"
#include "dll_phy_addr_map_macro.h"


#define DEV_ONFI_REV_1_0    (1 << 1)
#define DEV_ONFI_REV_2_0    (1 << 2)
#define DEV_ONFI_REV_2_1    (1 << 3)
#define DEV_ONFI_REV_2_2    (1 << 4)
#define DEV_ONFI_REV_2_3    (1 << 5)
#define DEV_ONFI_REV_3_0    (1 << 6)
#define DEV_ONFI_REV_3_1    (1 << 7)
#define DEV_ONFI_REV_3_2    (1 << 8)
#define DEV_ONFI_REV_4_0    (1 << 9)

#define GEN_PREP_INPUT_CMD_SEQ      255
#define GEN_PREP_INPUT_ADDR_SEQ     254
#define GEN_PREP_INPUT_DATA_SEQ     253
#define GEN_PREP_INPUT_READ_STAT    252
#define GEN_PREP_INPUT_SEED_INIT    251

#define HPNFC_INT_STATUS_ERR \
	(\
	 RF_CONTROLLER_REGS__RT_INTR_EN__CDMA_TERR_EN__MASK \
		| RF_CONTROLLER_REGS__RT_INTR_EN__DDMA_TERR_EN__MASK \
		| RF_CONTROLLER_REGS__RT_INTR_EN__CMD_IGNORED_EN__MASK \
		| RF_CONTROLLER_REGS__RT_INTR_EN__SDMA_ERR_EN__MASK\
		| RF_CONTROLLER_REGS__RT_INTR_EN__CMD_REG_PAR_ERR_EN__MASK\
		| RF_CONTROLLER_REGS__RT_INTR_EN__ZQCL_ERR_EN__MASK \
	)

#define MAX_SEED_VALUE      0xFFFFFF
#define MAX_ROW_ADDRESS     0xFFFFFF

static void
SlaveDmaInitState(HPNFC_State *hpnfc, uint8_t threadId, uint32_t cmdCount,
				  void* memPtr, void* descPtr)
{
	printf("%s memPtr 0x%p, descPtr 0x%p\n", __func__, memPtr, descPtr);

	hpnfc->slvDmaState[threadId].currMemPtr = memPtr;
	hpnfc->slvDmaState[threadId].currCdmaDesc = descPtr;
}

static void
SlaveDmaTransfer(HPNFC_State *hpnfc, uint8_t threadId,
				 HPNFC_TransferDirection direction,
				 uint32_t sdmaSize, uint32_t *transferredData)

{
	printf("%s\n", __func__);
	hpnfc->slaveDmaHandler(hpnfc, hpnfc->slvDmaState[threadId].currMemPtr,
						   direction, sdmaSize,
						   transferredData);

	hpnfc->slvDmaState[threadId].currMemPtr += *transferredData;
}

uint8_t SlaveDmaIsCurrentCdmaDesc(HPNFC_State *hpnfc, uint8_t threadId, void *desc)
{
	if ((desc != NULL) && (hpnfc->slvDmaState[threadId].currCdmaDesc == desc))
		return 1;
	else
		return 0;
}

void memSet(void *buff, uint32_t val, uint32_t size)
{
	uint8_t *buff8 = (uint8_t*)buff;
	while (size--){
		*buff8++ = val;
	}
}

uint32_t HPNFC_GetReg(uintptr_t reg)
{
	uint32_t val = CPS_UncachedRead32((uint32_t*)reg);


	return (val);
}

void HPNFC_SetReg(uintptr_t reg, uint32_t val)
{
	CPS_UncachedWrite32((uint32_t*)reg, val);
}


static uint8_t GenIsAllTransferred(HPNFC_GenReqInputData *dataSeq)
{
	uint32_t toTransfer;
	toTransfer = (dataSeq->sectorCount - 1) * dataSeq->sectorSize
		+ dataSeq->lastSectorSize;

	return (toTransfer == dataSeq->transferredData);
}

static int32_t GetEccCorrCapIdx(struct HPNFC_State *hpnfc, uint8_t corrCap,
							uint8_t *index);

//(TimeoutWaitReg               ((trd_status), 1 << thread, 1 << thread,        0,              ms));
static uint32_t TimeoutWaitReg(uintptr_t reg, uint32_t mask, uint32_t value, uint32_t match, uint32_t timeout)
{
	volatile uint32_t counter = timeout;
	uint32_t readOut = 0;

	vDbgMsg(DBG_GEN_MSG, DBG_HIVERB,
			"Wait addr %x mask %"PRIX32" value %"PRIX32" match %"PRId32"\n",
			 reg, mask, value, match);
	if (timeout == 0) {
	} else {
		do {
			//read register
			readOut = HPNFC_GetReg(reg);
			vDbgMsg(DBG_GEN_MSG, DBG_INFLOOP, "read value = %"PRIX32"\n",
					readOut);
			//decrement counter
			counter--;
			mdelay(1);
			//printf("delay\n");
			//compare with expected value and check counter
		} while (((((readOut & mask) != value) && match)
				|| (((readOut & mask) == value) && !match)) && counter);

		if ((((readOut & mask) == value) && match)
			|| (((readOut & mask) != value) && !match)) {
			//if value matches
			vDbgMsg(DBG_GEN_MSG, DBG_HIVERB, "read value = %"PRIX32"\n",
					readOut);
			//printf("read value = %"PRIX32"\n", readOut);
		} else {
			//if value doesn't match
			vDbgMsg(DBG_GEN_MSG, DBG_CRIT, "read value = %"PRIX32", "
					"reg %p, mask %"PRIX32"\n",
					readOut, (void*)reg, mask);
			//printf("read value = %"PRIX32", " "reg %p, mask %"PRIX32"\n", readOut, (void*)reg, mask);
			LogAndExit(EBUSY);
		}
	}
	return (EOK);
}

static int32_t WaitForThread(HPNFC_State *hpnfc, uint8_t thread, uint32_t ms)
{
	return (TimeoutWaitReg((uintptr_t)&(hpnfc->regs->cmd_status.trd_status), 1 << thread, 1 << thread, 0, ms));
}

int32_t WaitForIdle(HPNFC_State *hpnfc)
{
	return (TimeoutWaitReg((uintptr_t)&(hpnfc->regs->cmd_status.ctrl_status),
						  RF_CONTROLLER_REGS__RT_CTRL_STATUS__CTRL_BUSY__MASK,
						  RF_CONTROLLER_REGS__RT_CTRL_STATUS__CTRL_BUSY__MASK, 0, 10000));
}

// NFC has completed its reset and intialization process
static int32_t WaitForInitComplete(HPNFC_State *hpnfc)
{
	return (TimeoutWaitReg((uintptr_t)&(hpnfc->regs->cmd_status.ctrl_status),
						  RF_CONTROLLER_REGS__RT_CTRL_STATUS__INIT_COMP__MASK,
						  RF_CONTROLLER_REGS__RT_CTRL_STATUS__INIT_COMP__MASK, 1, 10000));
}

int32_t HPNFC_Core_GetRnb(HPNFC_State *hpnfc, uint8_t chip)
{
	uint32_t reg;
	reg = HPNFC_GetReg((uintptr_t)&(hpnfc->minictrl_regs->minictrl_settings.rbn_settings));
	return (reg & (1 << chip));
}

int32_t HPNFC_Core_GetDevInfo(HPNFC_State *hpnfc, HPNFC_DevInfo *devInfo)
{
	uint32_t reg;

	reg = HPNFC_GetReg((uintptr_t)&(hpnfc->regs->controller_params.device_params_0));
	devInfo->devType = RF_CONTROLLER_PARAMS__RT_ONFI_DEVICE_LUNS__DEVICE_TYPE__READ(reg);

	if (devInfo->devType == 0)
		return ENOTSUP;

	devInfo->bitsPerCell =
		RF_CONTROLLER_PARAMS__RT_ONFI_DEVICE_LUNS__BITS_PER_CELL__READ(reg);

	HPNFC_Core_GetDeviceLayout(hpnfc, &devInfo->devLayout);

	reg = HPNFC_GetReg((uintptr_t)&(hpnfc->regs->controller_params.manufacturer_id));
	devInfo->manufacturerId =
		RF_CONTROLLER_PARAMS__RT_MANUFACTURER_ID__MID__READ(reg);
	devInfo->deviceId =
			RF_CONTROLLER_PARAMS__RT_MANUFACTURER_ID__DID__READ(reg);


	return 0;
}

/* obtain information about connected  devices */
int32_t HPNFC_PrintDevInfo(HPNFC_State *hpnfc)
{
	uint32_t reg;
	int32_t i;
	HPNFC_DevInfo *devInfo = &hpnfc->devInfo;

	HPNFC_Core_GetDevInfo(hpnfc, devInfo);

	switch (devInfo->devType) {
	case HPNFC_DEV_TYPE_ONFI:
		printf( "%s", "Detected ONFI device:\n");
		break;
	case HPNFC_DEV_TYPE_JEDEC:
		printf( "%s", "Detected JEDEC device:\n");
		break;
	default:
		DbgMsg(DBG_GEN_MSG, DBG_WARN, "%s", "Device was not detected correctly.\n");
		return (-1);
	}

	printf( "-- Manufacturer ID: 0x%x\n",
			 devInfo->manufacturerId);
	printf( "-- Device ID: 0x%x\n",
			devInfo->deviceId);

	reg = HPNFC_GetReg((uintptr_t)&(hpnfc->regs->controller_params.device_revision));
	reg = RF_CONTROLLER_PARAMS__RT_ONFI_REVISION__REVISIONS__READ(reg);
	printf( "-- Device revisions codeword: 0x%"PRIX32"\n", reg);

	printf( "-- Page main area size: %d\n",
		   devInfo->devLayout.mainSize);
	printf( "-- Page spare area size: %d\n",
		   devInfo->devLayout.spareSize);

	printf( "-- Pages per block: %d\n",
		   devInfo->devLayout.pagesPerBlock);
	printf( "-- Number blocks per LUN %"PRId32"\n",
		   devInfo->devLayout.blocksPerLun);

	printf( "-- Number of LUNs %u\n",
		   devInfo->devLayout.lunNumber);
	printf( "-- Number of Planes %u\n",
		   devInfo->devLayout.numberOfPlanes);


	if (devInfo->devType == HPNFC_DEV_TYPE_ONFI){
		reg = HPNFC_GetReg((uintptr_t)&(hpnfc->regs->controller_params.device_revision));
		if (reg & DEV_ONFI_REV_4_0){
			hpnfc->onfiVersion = DEV_ONFI_REV_4_0;
			printf( "%s", "-- ONFI version 4.0\n");
		}
		else if (reg & DEV_ONFI_REV_3_2){
			hpnfc->onfiVersion = DEV_ONFI_REV_3_2;
			printf( "-- ONFI version 3.2\n");
		}
		else if (reg & DEV_ONFI_REV_3_1){
			hpnfc->onfiVersion = DEV_ONFI_REV_3_1;
			printf( "-- ONFI version 3.1\n");
		}
		else if (reg & DEV_ONFI_REV_3_0){
			hpnfc->onfiVersion = DEV_ONFI_REV_3_0;
			printf( "-- ONFI version 3.0\n");
		}
		else if (reg & DEV_ONFI_REV_2_3){
			hpnfc->onfiVersion = DEV_ONFI_REV_2_3;
			printf( "-- ONFI version 2.3\n");
		}
		else if (reg & DEV_ONFI_REV_2_2){
			hpnfc->onfiVersion = DEV_ONFI_REV_2_2;
			printf( "-- ONFI version 2.2\n");
		}
		else if (reg & DEV_ONFI_REV_2_1){
			hpnfc->onfiVersion = DEV_ONFI_REV_2_1;
			printf( "-- ONFI version 2.1\n");
		}
		else if (reg & DEV_ONFI_REV_2_0){
			hpnfc->onfiVersion = DEV_ONFI_REV_2_0;
			printf( "-- ONFI version 2.0\n");
		}

		printf( "-- Supported NandFlash memory modes:\n");
		reg = HPNFC_GetReg((uintptr_t)&(hpnfc->regs->controller_params.onfi_timing_modes_0));
		reg = RF_CONTROLLER_PARAMS__RT_ONFI_TIMING_MODE_0__SDR_MODES__READ(reg);
		for (i = 0; i < 16; i++) {
			if (reg & (1 << i))
				printf( "---- SDR mode %"PRId32"\n", i);
		}

		reg = HPNFC_GetReg((uintptr_t)&(hpnfc->regs->controller_params.onfi_timing_modes_0));
		reg = RF_CONTROLLER_PARAMS__RT_ONFI_TIMING_MODE_0__NV_DDR_MODES__READ(reg);
		for (i = 0; i < 16; i++) {
			if (reg & (1 << i))
				printf( "---- NV-DDR mode %"PRId32"\n", i);
		}

		reg = HPNFC_GetReg((uintptr_t)&(hpnfc->regs->controller_params.onfi_timing_modes_1));
		reg = RF_CONTROLLER_PARAMS__RT_ONFI_TIMING_MODE_1__NV_DDR2_MODES__READ(reg);
		for (i = 0; i < 16; i++) {
			if (reg & (1 << i))
				printf( "---- NV-DDR2 mode %"PRId32"\n", i);
		}

		reg = HPNFC_GetReg((uintptr_t)&(hpnfc->regs->controller_params.onfi_timing_modes_1));
		reg = RF_CONTROLLER_PARAMS__RT_ONFI_TIMING_MODE_1__NV_DDR3_MODES__READ(reg);
		if (reg & 1){
			printf( "---- NV-DDR3 mode %d\n", 0);
			printf( "---- NV-DDR3 mode %d\n", 1);
			printf( "---- NV-DDR3 mode %d\n", 2);
			printf( "---- NV-DDR3 mode %d\n", 3);
		}
		for (i = 1; i < 16; i++) {
			if (reg & (1 << i))
				printf( "---- NV-DDR3 mode %"PRId32"\n", i + 3);
		}
	}

	return (EOK);
}

int32_t HPNFC_CdmaSend(HPNFC_State *hpnfc, uint8_t thread, HPNFC_CdmaReq *req)
{
	uint32_t reg = 0;
	int32_t status;
	uint64_t descAddr = req->descDmaAddr;
	HPNFC_CdmaCmd cdmaCmd = {0};

	printf("descAddr 0x%lx\n ", descAddr);

	printf( "%s", "Send CDMA request\n");

	HPNFC_Cdma_DescParse(hpnfc, req->descBuf, &cdmaCmd);

	if (((cdmaCmd.cmdType == HPNFC_CDMA_CMD_TYPE_PROGRAM)
		|| (cdmaCmd.cmdType == HPNFC_CDMA_CMD_TYPE_READ))
		&& (cdmaCmd.dmaMode == HPNFC_DMA_MODE_SLAVE)){
		SlaveDmaInitState(hpnfc, thread, cdmaCmd.cmdCount, cdmaCmd.memPtr, req->descBuf);
	}

	/* wait for thread ready*/
	status = WaitForThread(hpnfc, thread, 1000);
	if (status)
		LogAndExit(status);

	DumpDescriptor(req->descBuf);

	HPNFC_SetReg((uintptr_t)&(hpnfc->regs->cmd_status.cmd_reg2), descAddr & 0xFFFFFFFF);
	HPNFC_SetReg((uintptr_t)&(hpnfc->regs->cmd_status.cmd_reg3), descAddr >> 32);

	reg = HPNFC_CMD_REG0_CT_CDMA | HPNFC_CMD_REG0_THREAD_WRITE(thread);

	HPNFC_SetReg((uintptr_t)&(hpnfc->regs->cmd_status.cmd_reg0), reg);

	return (EOK);
}

static int32_t PioCmdCheck(HPNFC_State *hpnfc, HPNFC_PioReq *req)
{
	uint32_t maxCmdCnt = HPNFC_MAX_CMD_COUNT;

	switch (req->cmdType){
	case HPNFC_PIO_CMD_TYPE_COPY_BACK:
	case HPNFC_PIO_CMD_TYPE_RESET:
	case HPNFC_PIO_CMD_TYPE_SET_FEATURES:
		return (EOK);
	default:
		break;
	}

	if (hpnfc->ctrlInfo.extCmdCnt == 1)
		maxCmdCnt = HPNFC_MAX_EXT_CMD_COUNT;

	if (req->cmdCount > maxCmdCnt){
		vDbgMsg(DBG_GEN_MSG, DBG_CRIT, "Error: Exceeded maximum command count %"PRId32"\n", req->cmdCount);
		LogAndExit(EINVAL);
	}

	/* erase command has not memory pointer parameter */
	if (req->cmdType == HPNFC_PIO_CMD_TYPE_ERASE)
		return (EOK);

	if (req->memPtr == NULL){
		vDbgMsg(DBG_GEN_MSG, DBG_CRIT, "Error: memPtr parameter is NULL %s", "\n");
		LogAndExit(EINVAL);
	}

	return (EOK);
}


static void PioCmdSetCmdCount(HPNFC_State *hpnfc, uint32_t *reg0, uint16_t cmdCount)
{
	if (hpnfc->ctrlInfo.extCmdCnt == 1){
		HPNFC_SetReg((uintptr_t)&(hpnfc->regs->cmd_status.cmd_reg4),
			(cmdCount - 1) & HPNFC_MAX_EXT_CMD_COUNT_MASK);
	}
	else {
		*reg0 |= (cmdCount - 1) & HPNFC_MAX_CMD_COUNT_MASK;
	}
}

static void PioCmdSendSetFeatures(HPNFC_State *hpnfc, uint8_t thread, HPNFC_PioReq *req)
{
	uint32_t reg;

	reg = HPNFC_PIO_CMD_REG1_FADDR_WRITE(req->featureAddr);
	reg |= HPNFC_PIO_CMD_REG1_BANK_WRITE(req->bank);
	HPNFC_SetReg((uintptr_t)&(hpnfc->regs->cmd_status.cmd_reg1), reg);
	HPNFC_SetReg((uintptr_t)&(hpnfc->regs->cmd_status.cmd_reg2), req->featureValue);

	reg = HPNFC_CMD_REG0_CT_PIO | HPNFC_PIO_CMD_REG0_THREAD_WRITE(thread);
	/* enable interrupt */
	reg |= HPNFC_PIO_CMD_REG0_INT_MASK;
	reg |= HPNFC_PIO_CMD_REG0_VOL_ID_WRITE(req->volume);
	reg |= HPNFC_PIO_CMD_REG0_CC_SF;
	/* execute command */
	HPNFC_SetReg((uintptr_t)&(hpnfc->regs->cmd_status.cmd_reg0), reg);
}

static void PioCmdSendProgram(HPNFC_State *hpnfc,  uint8_t thread, HPNFC_PioReq *req)
{
	uint32_t reg;
	uint64_t buffAddr = (uint64_t)(uintptr_t)req->memPtr;

	reg = HPNFC_PIO_CMD_REG1_BANK_WRITE(req->bank) | req->rowAddr;
	HPNFC_SetReg((uintptr_t)&(hpnfc->regs->cmd_status.cmd_reg1), reg);

	//printf("piosend prg buf l 0x%x\n", buffAddr & 0xFFFFFFFF);
	//printf("piosend prg buf h 0x%x\n", buffAddr >> 32);
	HPNFC_SetReg((uintptr_t)&(hpnfc->regs->cmd_status.cmd_reg2), buffAddr & 0xFFFFFFFF);
	HPNFC_SetReg((uintptr_t)&(hpnfc->regs->cmd_status.cmd_reg3), buffAddr >> 32);

	reg = HPNFC_CMD_REG0_CT_PIO | HPNFC_PIO_CMD_REG0_THREAD_WRITE(thread);
	/* enable interrupt */
	reg |= HPNFC_PIO_CMD_REG0_INT_MASK;
	reg |= HPNFC_PIO_CMD_REG0_VOL_ID_WRITE(req->volume);
	reg |= HPNFC_PIO_CMD_REG0_CC_WR;
	if (req->dmaMode == HPNFC_DMA_MODE_MASTER)
		reg |= HPNFC_PIO_CMD_REG0_DMA_MASTER_MASK;
	else
		SlaveDmaInitState(hpnfc, thread, req->cmdCount, req->memPtr, NULL);

	PioCmdSetCmdCount(hpnfc, &reg, req->cmdCount);

	/* execute command */
	HPNFC_SetReg((uintptr_t)&(hpnfc->regs->cmd_status.cmd_reg0), reg);
}


static void PioCmdSendRead(HPNFC_State *hpnfc,  uint8_t thread, HPNFC_PioReq *req)
{
	uint32_t reg;
	uint64_t buffAddr = (uint64_t)(uintptr_t)req->memPtr;

	reg = HPNFC_PIO_CMD_REG1_BANK_WRITE(req->bank) | req->rowAddr;
	HPNFC_SetReg((uintptr_t)&(hpnfc->regs->cmd_status.cmd_reg1), reg);

	HPNFC_SetReg((uintptr_t)&(hpnfc->regs->cmd_status.cmd_reg2), buffAddr & 0xFFFFFFFF);
	HPNFC_SetReg((uintptr_t)&(hpnfc->regs->cmd_status.cmd_reg3), buffAddr >> 32);

	reg = HPNFC_CMD_REG0_CT_PIO | HPNFC_PIO_CMD_REG0_THREAD_WRITE(thread);
	/* enable interrupt */
	reg |= HPNFC_PIO_CMD_REG0_INT_MASK;
	reg |= HPNFC_PIO_CMD_REG0_VOL_ID_WRITE(req->volume);
	reg |= HPNFC_PIO_CMD_REG0_CC_RD;
	if (req->dmaMode == HPNFC_DMA_MODE_MASTER)
		reg |= HPNFC_PIO_CMD_REG0_DMA_MASTER_MASK;
	else
		SlaveDmaInitState(hpnfc, thread, req->cmdCount, req->memPtr, NULL);

	PioCmdSetCmdCount(hpnfc, &reg, req->cmdCount);

	/* execute command */
	HPNFC_SetReg((uintptr_t)&(hpnfc->regs->cmd_status.cmd_reg0), reg);
}


static void PioCmdSendReset(HPNFC_State *hpnfc,  uint8_t thread, HPNFC_PioReq *req)
{
	uint32_t reg;

	reg = HPNFC_PIO_CMD_REG1_BANK_WRITE(req->bank);
	HPNFC_SetReg((uintptr_t)&(hpnfc->regs->cmd_status.cmd_reg1), reg);

	reg = HPNFC_CMD_REG0_CT_PIO | HPNFC_PIO_CMD_REG0_THREAD_WRITE(thread);
	/* enable interrupt */
	reg |= HPNFC_PIO_CMD_REG0_INT_MASK;
	reg |= HPNFC_PIO_CMD_REG0_VOL_ID_WRITE(req->volume);
	reg |= HPNFC_PIO_CMD_REG0_CC_RST;

	/* execute command */
	HPNFC_SetReg((uintptr_t)&(hpnfc->regs->cmd_status.cmd_reg0), reg);
}

static void PioCmdSendErase(HPNFC_State *hpnfc, uint8_t thread, HPNFC_PioReq *req)
{
	uint32_t reg;

	reg = HPNFC_PIO_CMD_REG1_BANK_WRITE(req->bank) | req->rowAddr;
	printf("erase reg 0x%x\n", reg);
	HPNFC_SetReg((uintptr_t)&(hpnfc->regs->cmd_status.cmd_reg1), reg);

	reg = HPNFC_CMD_REG0_CT_PIO | HPNFC_PIO_CMD_REG0_THREAD_WRITE(thread);
	/* enable interrupt */
	reg |= HPNFC_PIO_CMD_REG0_INT_MASK;
	reg |= HPNFC_PIO_CMD_REG0_VOL_ID_WRITE(req->volume);
	reg |= HPNFC_PIO_CMD_REG0_CC_ER;

	PioCmdSetCmdCount(hpnfc, &reg, req->cmdCount);

	/* execute command */
	HPNFC_SetReg((uintptr_t)&(hpnfc->regs->cmd_status.cmd_reg0), reg);
}

static void PioCmdSendCopyBck(HPNFC_State *hpnfc, uint8_t thread, HPNFC_PioReq *req)
{
	uint32_t reg;

	reg = HPNFC_PIO_CMD_REG1_BANK_WRITE(req->bank) | req->rowAddr;
	HPNFC_SetReg((uintptr_t)&(hpnfc->regs->cmd_status.cmd_reg1), reg);
	reg = HPNFC_PIO_CMD_REG1_BANK_WRITE(req->dstBank) | req->dstRowAddr;
	HPNFC_SetReg((uintptr_t)&(hpnfc->regs->cmd_status.cmd_reg2), reg);

	reg = HPNFC_CMD_REG0_CT_PIO | HPNFC_PIO_CMD_REG0_THREAD_WRITE(thread);
	/* enable interrupt */
	reg |= HPNFC_PIO_CMD_REG0_INT_MASK;
	reg |= HPNFC_PIO_CMD_REG0_VOL_ID_WRITE(req->volume);
	reg |= HPNFC_PIO_CMD_REG0_CC_ER;

	/* execute command */
	HPNFC_SetReg((uintptr_t)&(hpnfc->regs->cmd_status.cmd_reg0), reg);
}

void GenPrepInputCmdSeq(uint64_t *input, uint16_t cmd)
{
	*input |= GEN_SEQ_CMD_WRITE(cmd);
}

uint8_t GenPrepInputDataSeq(HPNFC_State *hpnfc, uint64_t *input, HPNFC_GenReqInputData *data)
{
	uint8_t corrCapIdx;
	int32_t status;
	data->transferredData = 0;

	if (data->lastSectorSize & 7){
		vDbgMsg(DBG_GEN_MSG, DBG_CRIT, "%s", "Because of SDMA limitation last sector size should be divisible by 8\n");
		LogAndExit(EINVAL);
	}

	*input |= GEN_SEQ_LAST_SECT_SIZE_WRITE(data->lastSectorSize);
	*input |= GEN_SEQ_SECT_SIZE_WRITE(data->sectorSize);
	*input |= GEN_SEQ_SECT_COUNT_WRITE(data->sectorCount);

	if (data->erasePageDetectEn)
		*input |= GEN_SEQ_ERASE_DETECT_ENABLE_MASK;

	if (data->eccEn){
		*input |= GEN_SEQ_ECC_ENABLE_MASK;

		status = GetEccCorrCapIdx(hpnfc, data->corrCap, &corrCapIdx);
		if (status){
			vDbgMsg(DBG_GEN_MSG, DBG_CRIT, "%s", "Wrong ECC corection capability settings.\n");
			LogAndExit(status);
		}
		else {
			*input |= GEN_SEQ_CORR_CAP_WRITE(corrCapIdx);
		}

		if (data->scramblerEnabled){
			*input |= GEN_SEQ_ECC_SCR_ENABLE_MASK;
		}
	}

	if (data->transferDirection == HPNFC_DATA_TRANSFER_WRITE)
		*input |= GEN_SEQ_DIR_WRITE_MASK;
	else
		*input &= ~GEN_SEQ_DIR_WRITE_MASK;

	return (EOK);
}

void GenPrepInputAddrSeq(uint64_t *input, HPNFC_GenReqInputAddress *addr)
{
	*input |= GEN_SEQ_ADDR_WRITE(addr->address, addr->addrSize);
	*input |= GEN_SEQ_BYTES_COUNT_WRITE(addr->addrSize);
}

void GenPrepInputAddr(uint64_t *input, uint8_t addr, uint8_t addrSize)
{
	*input |= GEN_SEQ_ADDR_WRITE(addr, addrSize);
}

void GenPrepInputReadStat(uint64_t *input, uint8_t readStatusF2En)
{
	*input |= GEN_SEQ_STATUS_F2_ENABLE_WRITE(readStatusF2En);
}

uint8_t GenPrepInputSeedValue(uint64_t *input, uint32_t seedValue)
{
	if (seedValue > MAX_SEED_VALUE){
		printf( "%s", "To big seed value\n");
		LogAndExit(EINVAL);
	}

	*input |= GEN_SEQ_SEED_VALUE_WRITE(seedValue);

	return 0;
}


uint8_t HPNFC_GenericPrepare(HPNFC_State *hpnfc, HPNFC_GenReq *req, uint64_t *miniCtrlCmd)
{
	static uint8_t instrTypes[] = {
		HPNFC_GEN_INSTR_CMD_SEQ,
		HPNFC_GEN_INSTR_ADDR_SEQ,
		HPNFC_GEN_INSTR_DATA_SEQ,
		HPNFC_GEN_INSTR_READ,
		HPNFC_GEN_INSTR_WRITE,
		HPNFC_GEN_INSTR_RESET,
		HPNFC_GEN_INSTR_ERASE,
		HPNFC_GEN_INSTR_STATUS,
		HPNFC_GEN_INSTR_ENHANCED_STATUS,
		HPNFC_GEN_INSTR_READ_CACHE,
		HPNFC_GEN_INSTR_COPYBACK_READ,
		HPNFC_GEN_INSTR_COPYBACK_WRITE,
		HPNFC_GEN_INSTR_READ_COLUMN,
		HPNFC_GEN_INSTR_READ_COLUMN_ENHANCED,
		HPNFC_GEN_INSTR_READ_COLUMN_JEDEC,
		HPNFC_GEN_INSTR_MULTIPLANE_READ,
		HPNFC_GEN_INSTR_MULTIPLANE_ERASE,
		HPNFC_GEN_INSTR_MULTIPLANE_ERASE2,
		HPNFC_GEN_INSTR_WRITE_COLUMN,
		HPNFC_GEN_INSTR_SMALL_DATA_MOVE,
		HPNFC_GEN_INSTR_SYNCHRONOUS_RESET,
		HPNFC_GEN_INSTR_VOLUME_SELECT,
		HPNFC_GEN_INSTR_ODT_CONFIGURE,
		HPNFC_GEN_INSTR_SET_FEATURES,
		HPNFC_GEN_INSTR_GET_FEATURES,
		HPNFC_GEN_INSTR_LUN_SET_FEATURES,
		HPNFC_GEN_INSTR_LUN_GET_FEATURES,
		HPNFC_GEN_INSTR_READ_ID,
		HPNFC_GEN_INSTR_READ_PARAM_PAGE,
		HPNFC_GEN_INSTR_ZQ_CALIB_SHORT,
		HPNFC_GEN_INSTR_ZQ_CALIB_LONG,
		HPNFC_GEN_INSTR_LUN_REST,
		HPNFC_GEN_INSTR_SEED_INITIAL,
	};

	static uint8_t instrActions[] = {
		GEN_PREP_INPUT_CMD_SEQ,
		GEN_PREP_INPUT_ADDR_SEQ,
		GEN_PREP_INPUT_DATA_SEQ,
		5,
		5,
		0,
		3,
		GEN_PREP_INPUT_READ_STAT,
		3,
		5,
		5,
		5,
		2,
		5,
		5,
		5,
		3,
		6,
		2,
		5,
		0,
		1,
		1,
		1,
		1,
		1,
		1,
		1,
		1,
		1,
		1,
		3,
		GEN_PREP_INPUT_SEED_INIT
	};
	uint8_t instrType = 0;
	uint8_t status;
	uint8_t action;
	*miniCtrlCmd = 0;

	if (req->cmdType == HPNFC_GEN_INSTR_WRITE || req->cmdType == HPNFC_GEN_INSTR_READ)
	{
		printf("================\n");
		*miniCtrlCmd |= 0x4 << 11; // number of byte - 1
	}

	instrType = instrTypes[req->cmdType];
	action = instrActions[req->cmdType];
	if (action == GEN_PREP_INPUT_CMD_SEQ) {
		GenPrepInputCmdSeq(miniCtrlCmd, req->input.cmdSeq);
	} else if (action == GEN_PREP_INPUT_ADDR_SEQ) {
		GenPrepInputAddrSeq(miniCtrlCmd, &req->input.addrSeq);
	} else if (action == GEN_PREP_INPUT_DATA_SEQ) {
		status = GenPrepInputDataSeq(hpnfc, miniCtrlCmd, &req->input.dataSeq);
		if (status)
			LogAndExit(status);
	} else if (action == GEN_PREP_INPUT_READ_STAT) {
		GenPrepInputReadStat(miniCtrlCmd, req->input.readStatusF2En);
	} else if (action == GEN_PREP_INPUT_SEED_INIT) {
		GenPrepInputSeedValue(miniCtrlCmd, req->input.seedValue);
	} else if (action > 0) {
		GenPrepInputAddr(miniCtrlCmd, req->input.addr, action);
	}

	*miniCtrlCmd |= GEN_SEQ_CHIP_SELECT_WRITE(req->chipSelect);
	if (req->jedecSupp)
		*miniCtrlCmd |= GEN_SEQ_JEDEC_SUPP_MASK;

	if (req->twbActive)
		*miniCtrlCmd |= GEN_SEQ_TWB_ACTIVE_MASK;

	*miniCtrlCmd |= GEN_SEQ_INSTR_TYPE_WRITE(instrType);

	return (EOK);
}

int32_t HPNFC_ResetSend(HPNFC_State *hpnfc, uint8_t thread, HPNFC_RstReq *req)
{
	uint32_t reg = 0;

	reg = HPNFC_CMD_REG0_CT_RST | HPNFC_CMD_REG0_THREAD_WRITE(thread);
	reg |= HPNFC_CMD_REG0_INT_MASK;
	switch (req->resetType){
	case 0:
		reg |= HPNFC_RST_INSTR_RESET_TYPE_0;
		break;
	case 1:
		reg |= HPNFC_RST_INSTR_RESET_TYPE_1;
		break;
	default:
		LogAndExit(EINVAL);
	}

	HPNFC_SetReg((uintptr_t)&(hpnfc->regs->cmd_status.cmd_reg0), reg);

	return (EOK);
}

int32_t HPNFC_GenericSend(HPNFC_State *hpnfc, uint8_t thread, HPNFC_GenReq *req)
{
	uint32_t reg = 0;
	uint8_t status;
	uint64_t miniCtrlCmd;
	uint32_t miniCtrlCmdL;
	uint32_t miniCtrlCmdH;

	//printf( "%s", "Send Generic request\n");
	printf("[NFC_TEST] %s() \n", __func__);

	status = HPNFC_GenericPrepare(hpnfc, req, &miniCtrlCmd);
	if (status){
		LogAndExit(status);
	}

	/* generic commands supports only Slave DMA transmission */
	if (req->cmdType == HPNFC_GEN_INSTR_DATA_SEQ)
		SlaveDmaInitState(hpnfc, thread, 1, req->input.dataSeq.dataPtr, NULL);

	miniCtrlCmdL = miniCtrlCmd & 0xFFFFFFFF;
	//miniCtrlCmdL = 0x78; // for testing command error
	miniCtrlCmdH = miniCtrlCmd >> 32;

	printf("send generic cmd 0-31:%08x 32-63:%08x\n",
			miniCtrlCmdL, miniCtrlCmdH);

	status = WaitForThread(hpnfc, thread, 1000);
	if (status)
	{
		printf("%s err 1\n", __func__);
		return status;
	}

	HPNFC_SetReg((uintptr_t)&(hpnfc->regs->cmd_status.cmd_reg2), miniCtrlCmdL);
	HPNFC_SetReg((uintptr_t)&(hpnfc->regs->cmd_status.cmd_reg3), miniCtrlCmdH);

	reg = HPNFC_CMD_REG0_CT_GEN | HPNFC_GEN_CMD_REG0_THREAD_WRITE(thread);
	/* enable interrupt */
	reg |= HPNFC_GEN_CMD_REG0_INT_MASK;

	/* execute command */
	HPNFC_SetReg((uintptr_t)&(hpnfc->regs->cmd_status.cmd_reg0), reg);

	return (EOK);
}


int32_t HPNFC_PioSend(HPNFC_State *hpnfc, uint8_t thread, HPNFC_PioReq *req)
{
	int32_t status;

	//printf( "%s", "Send PIO request\n");

	status = PioCmdCheck(hpnfc, req);
	//printf("%s status1 %d\n", __func__, status);
	if (status)
		LogAndExit(status);

	/* wait for thread ready*/
	status = WaitForThread(hpnfc, thread, 1000);
	//printf("%s status2 %d\n", __func__, status);
	if (status)
		LogAndExit(status);


	switch (req->cmdType){
	case HPNFC_PIO_CMD_TYPE_ERASE:
		PioCmdSendErase(hpnfc, thread, req);
		break;
	case HPNFC_PIO_CMD_TYPE_PROGRAM:
		PioCmdSendProgram(hpnfc, thread, req);
		break;
	case HPNFC_PIO_CMD_TYPE_READ:
		PioCmdSendRead(hpnfc, thread, req);
		break;
	case HPNFC_PIO_CMD_TYPE_COPY_BACK:
		PioCmdSendCopyBck(hpnfc, thread, req);
		break;
	case HPNFC_PIO_CMD_TYPE_RESET:
		PioCmdSendReset(hpnfc, thread, req);
		break;
	case HPNFC_PIO_CMD_TYPE_SET_FEATURES:
		PioCmdSendSetFeatures(hpnfc, thread, req);
		break;
	}

	return (EOK);
}

static void
GetExtOperationResult(HPNFC_State *hpnfc, uint32_t status[2],
					  HPNFC_ReqType reqType, HPNFC_OperationResult *result)
{
	result->errIdx = (status[1] & HPNFC_OPER_STATUS_EXT_ERR_IDX_MASK)
		>> HPNFC_OPER_STATUS_EXT_ERR_IDX_SHIFT;

	if (status[0] & HPNFC_OPER_STATUS_DEV_ERR_MASK) {
		result->planesDeviceError = (status[1] & HPNFC_OPER_STATUS_EXT_PL_DEV_ERR_MASK)
			>> HPNFC_OPER_STATUS_EXT_PL_DEV_ERR_SHIFT;
	}

	if (status[0] & HPNFC_OPER_STATUS_UNCE_MASK){
		result->planesEccError = (status[1] & HPNFC_OPER_STATUS_EXT_PL_ECC_ERR_MASK)
			>> HPNFC_OPER_STATUS_EXT_PL_ECC_ERR_SHIFT;
	}
}

HPNFC_OperationResult
HPNFC_GetOperationResult(HPNFC_State *hpnfc, uint32_t status[2], HPNFC_ReqType reqType)
{
	HPNFC_OperationResult operationResult;

	memSet(&operationResult, 0, sizeof(operationResult));

	operationResult.status = HPNFC_STATUS_CMD_UNFINISHED;

	/* descriptor finished with no errors */
	if (status[0] & HPNFC_OPER_STATUS_COMP_MASK) {
		operationResult.status = HPNFC_STATUS_NO_ERROR;
	}

	//printf("[NFC_TEST] %s(): status[0] %d\n", __func__, status[0]);

	if (reqType != HPNFC_REQ_TYPE_GEN){
		if (status[0] & HPNFC_OPER_STATUS_ERP_MASK){
			operationResult.erased = 1;
		}

		if (status[0] & HPNFC_OPER_STATUS_FAIL_MASK){
			if (hpnfc->ctrlInfo.extStatus == 1){
				GetExtOperationResult(hpnfc, status, reqType, &operationResult);
			}
			else {
				operationResult.errIdx = (status[0] & HPNFC_OPER_STATUS_ERR_IDX_MASK)
					>> HPNFC_OPER_STATUS_ERR_IDX_SHIFT;
			}
			printf("Error was detected on operation %d\n",
					operationResult.errIdx);
		}
	}

	if (status[0] & HPNFC_OPER_STATUS_MAXERR_MASK){
		operationResult.status = HPNFC_STATUS_ECC_CORR_ERR;
		operationResult.maxCorrErr = (status[0] & HPNFC_OPER_STATUS_MAXERR_MASK)
			>> HPNFC_OPER_STATUS_MAXERR_SHIFT;
		printf( "%s", "ECC correctable error appears\n");
	}

	if (status[0] & HPNFC_OPER_STATUS_FAIL_MASK){

		if (status[0] & HPNFC_OPER_STATUS_UNCE_MASK){
			operationResult.status = HPNFC_STATUS_ECC_UNCORR_ERR;
			printf( "%s", "ECC fatal error appears\n");
		}

		if (status[0] & HPNFC_OPER_STATUS_ERR_MASK) {
			operationResult.status = HPNFC_STATUS_CMD_ERR;
			printf( "%s", "Command error appears\n");
		}

		if (status[0] & HPNFC_OPER_STATUS_DEV_ERR_MASK) {
			operationResult.status = HPNFC_STATUS_DEV_ERR;
			printf( "%s", "Device error appears\n");
		}

		if (status[0] & HPNFC_OPER_STATUS_DQS_ERR_MASK){
			operationResult.status = HPNFC_STATUS_DQS_ERR;
			printf( "%s", "DQS error appears\n");
		}

		if (status[0] & HPNFC_OPER_STATUS_BUS_ERR_MASK){
			operationResult.status = HPNFC_STATUS_BUS_ERR;
			printf( "%s", "Bus access error appears\n");
		}

		if (status[0] & HPNFC_OPER_STATUS_DI_DAT_ERR_MASK){
			operationResult.status = HPNFC_STATUS_CRC_DAT_ERR;
			printf( "%s", "CRC error was detected in the data path\n");
		}

		if (status[0] & HPNFC_OPER_STATUS_DI_DSC_ERR_MASK){
			operationResult.status = HPNFC_STATUS_PARITY_DSC_ERR;
			printf("%s", "Parity error detected on accessing descriptor\n");
		}

		if (status[0] & HPNFC_OPER_STATUS_DI_CTX_ERR_MASK){
			operationResult.status = HPNFC_STATUS_PARITY_CTX_ERR;
			printf( "%s", "Parity error detected on accessing contex memory\n");
		}
	}

	return (operationResult);
}

static int32_t HPNFC_GenParseStatus(HPNFC_State *hpnfc, HPNFC_GenReq *genReq,
								uint32_t status[2])
{
	printf("[NFC_TEST] %s()\n", __func__);

	genReq->operationResult =
		HPNFC_GetOperationResult(hpnfc, status, HPNFC_REQ_TYPE_GEN);

	if (genReq->operationResult.status)
	{
		printf("[NFC_TEST] %s() : genReq->operationResult.status %d\n", __func__, genReq->operationResult.status);
		LogAndExit(EIO);
	}

	return (EOK);
}

static int32_t HPNFC_PioParseStatus(HPNFC_State *hpnfc, HPNFC_PioReq *pioReq,
								uint32_t status[2])
{
	pioReq->operationResult =
		HPNFC_GetOperationResult(hpnfc, status, HPNFC_REQ_TYPE_PIO);

	if (pioReq->operationResult.status)
		LogAndExit(EIO);

	return (EOK);
}

static void PrintControllerInfo(HPNFC_State *hpnfc)
{
	uint32_t reg;
	reg = HPNFC_GetReg((uintptr_t)&(hpnfc->regs->controller_params.ctrl_features_reg));

	printf( "Controller Revision: %d, fixed: %d\n",
		   hpnfc->ctrlInfo.ctrlRev, hpnfc->ctrlInfo.ctrlFix);
	DbgMsg(DBG_GEN_MSG, DBG_WARN, "%s", "Controller Features:\n");
	printf( "-- features reg: %"PRIX32" \n", reg);

	printf( "-- Number of threads: %d \n",
		   hpnfc->ctrlInfo.threadsNumber);

	if (hpnfc->ctrlInfo.dummyDescRead)
		printf( "%s", "-- Dummy descriptor read feature present\n");

	if (hpnfc->ctrlInfo.extStatus)
		printf( "%s", "-- Extended Status feature present\n");

	if (hpnfc->ctrlInfo.remap)
		printf( "%s", "-- Remap mechanism present\n");

	if (hpnfc->ctrlInfo.extCmdCnt)
		printf( "%s", "-- Extended Command feature present\n");

	if (hpnfc->ctrlInfo.dataIntergity)
		printf( "%s", "-- Data Integrity mechanism present\n");

	if (hpnfc->ctrlInfo.preFetch)
		printf( "%s", "-- Pre-fetching mechanism present\n");

	if (hpnfc->ctrlInfo.support16b)
		printf( "%s", "-- Support for 16b NF interface\n");

	if (hpnfc->ctrlInfo.bootAvailable)
		printf( "%s", "-- Boot feature present\n");

	if (hpnfc->ctrlInfo.eccAvailable)
		printf( "%s", "-- Data ECC protection engine present\n");

	printf( "-- Slave and Master DMA address width: %d\n",
		   hpnfc->ctrlInfo.dmaAddr64bit ? 64 : 32);

	printf( "-- Slave and Master DMA data width: %d\n",
		   hpnfc->ctrlInfo.dmaData64bit ? 64 : 32);

	printf( "-- Number of banks: %x \n",
		   hpnfc->ctrlInfo.banksNumber);

	if (hpnfc->ctrlInfo.asyncSupport)
		printf( "%s", "-- Asynchronous work mode supported\n");

	if (hpnfc->ctrlInfo.ddrSupport)
		printf( "%s", "-- NV-DDR work mode supported\n");

	if (hpnfc->ctrlInfo.ddr2Support)
		printf( "%s", "-- NV-DDR2 work mode supported\n");

}

void GetControllerInfo(HPNFC_State *hpnfc)
{
	uint32_t reg;

	reg = HPNFC_GetReg((uintptr_t)&(hpnfc->regs->controller_params.ctrl_version));

	hpnfc->ctrlInfo.ctrlRev = RF_CONTROLLER_PARAMS__RT_VERSION_REG__CTRL_REV__READ(reg);
	hpnfc->ctrlInfo.ctrlFix = RF_CONTROLLER_PARAMS__RT_VERSION_REG__CTRL_FIX__READ(reg);

	reg = HPNFC_GetReg((uintptr_t)&(hpnfc->regs->controller_params.ctrl_features_reg));

	hpnfc->ctrlInfo.threadsNumber = 1uL << RF_CONTROLLER_PARAMS__RT_FEATURES_REG__N_THREADS__READ(reg);
	hpnfc->ctrlInfo.dummyDescRead = RF_CONTROLLER_PARAMS__RT_FEATURES_REG__DUMMY_DESC_READ__READ(reg);
	hpnfc->ctrlInfo.extStatus = RF_CONTROLLER_PARAMS__RT_FEATURES_REG__EXT_STATUS__READ(reg);
	hpnfc->ctrlInfo.remap = RF_CONTROLLER_PARAMS__RT_FEATURES_REG__RMP_AVAILABLE__READ(reg);
	hpnfc->ctrlInfo.extCmdCnt = RF_CONTROLLER_PARAMS__RT_FEATURES_REG__EXT_CMD_CNT__READ(reg);
	hpnfc->ctrlInfo.dataIntergity = RF_CONTROLLER_PARAMS__RT_FEATURES_REG__DI_AVAILABLE__READ(reg);
	hpnfc->ctrlInfo.preFetch = RF_CONTROLLER_PARAMS__RT_FEATURES_REG__PRE_FETCH_AVAILABLE__READ(reg);
	hpnfc->ctrlInfo.support16b = RF_CONTROLLER_PARAMS__RT_FEATURES_REG__NF_16B_SUPP__READ(reg);
	hpnfc->ctrlInfo.bootAvailable = RF_CONTROLLER_PARAMS__RT_FEATURES_REG__BOOT_AVAILABLE__READ(reg);
	hpnfc->ctrlInfo.eccAvailable = RF_CONTROLLER_PARAMS__RT_FEATURES_REG__ECC_AVAILABLE__READ(reg);
	hpnfc->ctrlInfo.dmaAddr64bit = RF_CONTROLLER_PARAMS__RT_FEATURES_REG__DMA_ADDR_WIDTH__READ(reg);
	hpnfc->ctrlInfo.dmaData64bit = RF_CONTROLLER_PARAMS__RT_FEATURES_REG__DMA_DATA_WIDTH__READ(reg);
	hpnfc->ctrlInfo.banksNumber = 1 << (RF_CONTROLLER_PARAMS__RT_FEATURES_REG__N_BANKS__READ(reg));
	hpnfc->ctrlInfo.asyncSupport = RF_CONTROLLER_PARAMS__RT_FEATURES_REG__ASYNC_SUPP__READ(reg);
	hpnfc->ctrlInfo.ddrSupport = RF_CONTROLLER_PARAMS__RT_FEATURES_REG__NVDDR__READ(reg);
	hpnfc->ctrlInfo.ddr2Support = RF_CONTROLLER_PARAMS__RT_FEATURES_REG__NVDDR2_3__READ(reg);
}

static void GetBchCfgInfo(HPNFC_State *hpnfc, HPNFC_Config *config)
{
	uint32_t reg;
	int32_t i;

	hpnfc->bchCfgInfo.numberOfCorrCaps = 0;
	for (i = 0; i < 8; i++){
		hpnfc->bchCfgInfo.corrCaps[i] = 0;
	}
	hpnfc->bchCfgInfo.sectorSizes[0] = 0;
	hpnfc->bchCfgInfo.sectorSizes[1] = 0;

	reg = HPNFC_GetReg((uintptr_t)&(hpnfc->regs->controller_params.bch_cfg_0));

	hpnfc->bchCfgInfo.corrCaps[0] = RF_CONTROLLER_PARAMS__RT_BCH_CFG_0__BCH_CORR_0__READ(reg);
	hpnfc->bchCfgInfo.corrCaps[1] = RF_CONTROLLER_PARAMS__RT_BCH_CFG_0__BCH_CORR_1__READ(reg);
	hpnfc->bchCfgInfo.corrCaps[2] = RF_CONTROLLER_PARAMS__RT_BCH_CFG_0__BCH_CORR_2__READ(reg);
	hpnfc->bchCfgInfo.corrCaps[3] = RF_CONTROLLER_PARAMS__RT_BCH_CFG_0__BCH_CORR_3__READ(reg);

	reg = HPNFC_GetReg((uintptr_t)&(hpnfc->regs->controller_params.bch_cfg_1));

	hpnfc->bchCfgInfo.corrCaps[4] = RF_CONTROLLER_PARAMS__RT_BCH_CFG_1__BCH_CORR_4__READ(reg);
	hpnfc->bchCfgInfo.corrCaps[5] = RF_CONTROLLER_PARAMS__RT_BCH_CFG_1__BCH_CORR_5__READ(reg);
	hpnfc->bchCfgInfo.corrCaps[6] = RF_CONTROLLER_PARAMS__RT_BCH_CFG_1__BCH_CORR_6__READ(reg);
	hpnfc->bchCfgInfo.corrCaps[7] = RF_CONTROLLER_PARAMS__RT_BCH_CFG_1__BCH_CORR_7__READ(reg);

	for (i = 0; i < 8; i++){
		if (hpnfc->bchCfgInfo.corrCaps[i] == 0)
			break;
		hpnfc->bchCfgInfo.numberOfCorrCaps++;
	}

	reg = HPNFC_GetReg((uintptr_t)&(hpnfc->regs->controller_params.bch_cfg_2));
	hpnfc->bchCfgInfo.sectorSizes[0] = RF_CONTROLLER_PARAMS__RT_BCH_CFG_2__BCH_SECT_0__READ(reg);
	hpnfc->bchCfgInfo.sectorSizes[1] = RF_CONTROLLER_PARAMS__RT_BCH_CFG_2__BCH_SECT_1__READ(reg);

	if (!hpnfc->bchCfgInfo.numberOfCorrCaps || !hpnfc->bchCfgInfo.sectorSizes[0]){
		if (config->bchCfgInfo == NULL){
			vDbgMsg(DBG_GEN_MSG, DBG_WARN, "%s", "Bch configuration cannot be read from hardware and from config->bchCfgInfo.\n");
			return;
		}

		for (i = 0; i < 8; i++){
			hpnfc->bchCfgInfo.corrCaps[i] = config->bchCfgInfo->corrCaps[i];
		}
		hpnfc->bchCfgInfo.numberOfCorrCaps = config->bchCfgInfo->numberOfCorrCaps;
		hpnfc->bchCfgInfo.sectorSizes[0] = config->bchCfgInfo->sectorSizes[0];
		hpnfc->bchCfgInfo.sectorSizes[1] = config->bchCfgInfo->sectorSizes[1];
	}
}

int32_t HPNFC_Core_Init(HPNFC_State *hpnfc, HPNFC_Config *config)
{
	int32_t status;
	uint32_t intConfig;
	HPNFC_PhyConfig *phyConfig = &hpnfc->defPhyConfig;

	status = WaitForInitComplete(hpnfc);
	if (status)
		LogAndExit(status);

	/* disable cache and multiplane */
	HPNFC_SetReg((uintptr_t)&(hpnfc->regs->ctrl_config.multiplane_config), 0);
	HPNFC_SetReg((uintptr_t)&(hpnfc->regs->ctrl_config.cache_config), 0);

	/* enable all needed interrupts */
	intConfig = RF_CONTROLLER_REGS__RT_INTR_EN__SDMA_TRIGG_EN__MASK
		| RF_CONTROLLER_REGS__RT_INTR_EN__SDMA_ERR_EN__MASK
		| HPNFC_INT_STATUS_ERR;
	HPNFC_SetReg((uintptr_t)&(hpnfc->regs->cmd_status.intr_enable), intConfig);

	/* enable signaling thread error and timeout interrupts for all threads */
	HPNFC_SetReg((uintptr_t)&(hpnfc->regs->cmd_status.trd_error_intr_en),
				 RF_CONTROLLER_REGS__RT_TRD_ERROR_INTR_EN__TRD_ERROR_INTR_EN__MASK);
	HPNFC_SetReg((uintptr_t)&(hpnfc->regs->cmd_status.trd_timeout_intr_en),
				 RF_CONTROLLER_REGS__RT_TRD_TIMEOUT_INTR_EN__TRD_TIMEOUT_INTR_EN__MASK);


	phyConfig->phyCtrl = 0x4170;
	phyConfig->phyTsel = 0;
	phyConfig->phyDqTiming = 2;
	phyConfig->phyDqsTiming = 0x100004;
	phyConfig->phyGateLpbkCtrl = 0xd80000;
	phyConfig->phyDllMasterCtrl = 0x800000;
	phyConfig->phyDllSlaveCtrl = 0;

	phyConfig->toggleTimings0 = 0x3F3F3F3F;
	phyConfig->toggleTimings1 = 0x3F3F;
	phyConfig->asyncToggleTimings = 0x18181818;
	phyConfig->timings0 = 0xFFFFFFFF;
	phyConfig->timings1 = 0xFFFFFFFF;
	phyConfig->timings2 = 0x3F3F;
	phyConfig->dllPhyCtrl = 0x1030300;

	HPNFC_PhySetPhyConfig(hpnfc, phyConfig);

	GetControllerInfo(hpnfc);
	PrintControllerInfo(hpnfc);

	GetBchCfgInfo(hpnfc, config);

	HPNFC_ConfigInt(hpnfc, 0);

	if (hpnfc->ctrlInfo.dummyDescRead == 1)
		hpnfc->cdmaDescSize = HPNFC_CDMA_DESC_WITH_DUMMY_SIZE;
	else
		hpnfc->cdmaDescSize = HPNFC_CDMA_DESC_SIZE;

	return (EOK);
}


void HPNFC_ConfigInt(HPNFC_State *hpnfc, uint8_t enable)
{
	uint32_t intConfig;

	intConfig = HPNFC_GetReg((uintptr_t)&(hpnfc->regs->cmd_status.intr_enable));

	if (enable){
		intConfig |= RF_CONTROLLER_REGS__RT_INTR_EN__INTR_EN__MASK;
	}
	else {
		intConfig &= ~RF_CONTROLLER_REGS__RT_INTR_EN__INTR_EN__MASK;
	}

	hpnfc->intEnabled = enable;

	HPNFC_SetReg((uintptr_t)&(hpnfc->regs->cmd_status.intr_enable), intConfig);
}

int32_t HPNFC_GetLastStatus(HPNFC_State *hpnfc, uint8_t thread, uint32_t status[2])
{
	//printf("[NFC_TEST] %s()\n", __func__);
	if (thread > hpnfc->ctrlInfo.threadsNumber){
		LogAndExit(EINVAL);
	}

	HPNFC_SetReg((uintptr_t)&(hpnfc->regs->cmd_status.cmd_status_ptr), thread);
	status[0] = HPNFC_GetReg((uintptr_t)&(hpnfc->regs->cmd_status.cmd_status));
	//printf("[NFC_TEST] %s() status 0: %d\n", __func__, status[0]);
	if (hpnfc->ctrlInfo.extStatus == 1){
		status[1] = HPNFC_GetReg((uintptr_t)&(hpnfc->regs->cmd_status.cmd_status_ext));
		//printf("[NFC_TEST] %s() status 1: %d\n", __func__, status[1]);
	}
	else
		status[1] = 0;

	return (EOK);
}


int32_t HPNFC_ReqAttach(HPNFC_State *hpnfc, HPNFC_CmdReq* cmdReq)
{
	if (cmdReq->thread > hpnfc->ctrlInfo.threadsNumber){
		vDbgMsg(DBG_GEN_MSG, DBG_CRIT,
				"given thread number %d is bigger than supported number of threads %d\n",
				cmdReq->thread, hpnfc->ctrlInfo.threadsNumber);
		LogAndExit(EINVAL);
	}
	// printf( "Attach request on thread %d\n", cmdReq->thread);

	if (cmdReq->reqType == HPNFC_REQ_TYPE_RST){
		cmdReq->req.reset.abortingRequest = hpnfc->cmdReq[cmdReq->thread];
	}
	else {
		/* check if thread is not used*/
		if (hpnfc->cmdReq[cmdReq->thread]){
			vDbgMsg(DBG_GEN_MSG, DBG_CRIT, "thread %d is busy\n", cmdReq->thread);
			LogAndExit(EBUSY);
		}
	}

	hpnfc->cmdReq[cmdReq->thread] = cmdReq;

	return (EOK);
}

HPNFC_CmdReq* HPNFC_ReqGet(HPNFC_State *hpnfc, uint8_t thread)
{
	return (hpnfc->cmdReq[thread]);
}

int32_t HPNFC_ReqGetStatus(HPNFC_State *hpnfc, uint8_t thread)
{
	uint32_t status = EINVAL;
	uint32_t lastStatus[2];
	HPNFC_CmdReq* cmdReq = hpnfc->cmdReq[thread];

	/* check if thread is not null*/
	if (!cmdReq)
		return (EOK);

	switch (cmdReq->reqType){
	case HPNFC_REQ_TYPE_CDMA:
		/* check if all descriptors in chain was processed and executed without errors
		   return error otherwise */
		return (HPNFC_Cdma_CheckChainStatus(cmdReq->req.cdma.descBuf));
	case HPNFC_REQ_TYPE_PIO:
		status = HPNFC_GetLastStatus(hpnfc, thread, lastStatus);
		if (status)
			LogAndExit(status);
		return (HPNFC_PioParseStatus(hpnfc, &cmdReq->req.pio, lastStatus));
	case HPNFC_REQ_TYPE_GEN:
		status = HPNFC_GetLastStatus(hpnfc, thread, lastStatus);
		printf("[NFC_TEST] %s() : 1 status %d\n", __func__, status);
		//if (status)
		//    LogAndExit(status);
		status = HPNFC_GenParseStatus(hpnfc, &cmdReq->req.generic, lastStatus);
		printf("[NFC_TEST] %s() : 2 status %d\n", __func__, status);

		if ((status == 0) && (cmdReq->req.generic.cmdType == HPNFC_GEN_CMD_TYPE_DATA_SEQ)){
			if (GenIsAllTransferred(&cmdReq->req.generic.input.dataSeq))
				return (status);
			else
				return (HPNFC_STATUS_PENDING);
		}
	case HPNFC_REQ_TYPE_RST:
		status = 0;
		break;
	default:
		break;
	}

	return (status);
}

int32_t HPNFC_ReqRelease(HPNFC_State *hpnfc, uint8_t thread, int32_t requestStatus)
{
	/* check if thread is not null*/
	if (!hpnfc->cmdReq[thread])
		return (EOK);

	// printf( "Release request on thread %"PRId8" with status %"PRId32"\n", thread, requestStatus);
	//printf("Release request on thread %"PRId8" with status %"PRId32"\n",
	//        thread, requestStatus);

	/* set new status of current request */
	hpnfc->cmdReq[thread]->status = requestStatus;
	if (hpnfc->cmdReq[thread]->reqType == HPNFC_REQ_TYPE_RST){
		/* set new status of aborting request */
		if (hpnfc->cmdReq[thread]->req.reset.abortingRequest)
			hpnfc->cmdReq[thread]->req.reset.abortingRequest->status = EIO;
	}

	/* set request pointer to NULL in order to release it */
	hpnfc->cmdReq[thread] = NULL;

	return (EOK);
}

void HPNFC_Sdma_Isr(HPNFC_State *hpnfc)
{
	uint32_t sdmaSize;
	uint32_t intStat;
	uint8_t sdmaThread;
	HPNFC_CmdReq *cmdReq;
	uint32_t transferredData = 0;

	printf("%s\n", __func__);

	if (!hpnfc->slaveDmaHandler)
		return;

	sdmaSize = HPNFC_GetReg((uintptr_t)&(hpnfc->regs->ctrl_config.sdma_size));
	sdmaThread = HPNFC_GetReg((uintptr_t)&(hpnfc->regs->ctrl_config.sdma_trd_num));

	printf("%s size %d, trd %d\n", __func__, sdmaSize, sdmaThread);

	cmdReq = HPNFC_ReqGet(hpnfc, sdmaThread);
	if (cmdReq == NULL){
		vDbgMsg(DBG_GEN_MSG, DBG_WARN,
				"slave DMA trigger was detected on thread %d, but there is no command request on this thread\n",
				sdmaThread);
		return;
	}

	printf("%s cmdReq->reqType %d\n", __func__, cmdReq->reqType);
	switch (cmdReq->reqType){
	case HPNFC_REQ_TYPE_GEN:
		{
			HPNFC_GenReqInputData *dataSeq = &cmdReq->req.generic.input.dataSeq;
			SlaveDmaTransfer(hpnfc, sdmaThread, dataSeq->transferDirection, sdmaSize,
							 &transferredData);

			dataSeq->transferredData += transferredData;
			/* for read operation if all data transferred it means that operation is finished*/
			if (GenIsAllTransferred(dataSeq)){
				HPNFC_ReqRelease(hpnfc, sdmaThread, 0);
			}
		}
		break;
	case HPNFC_REQ_TYPE_PIO:
		if (cmdReq->req.pio.cmdType == HPNFC_PIO_CMD_TYPE_PROGRAM){
			SlaveDmaTransfer(hpnfc, sdmaThread, HPNFC_DATA_TRANSFER_WRITE, sdmaSize,
							 &transferredData);
		}
		if (cmdReq->req.pio.cmdType == HPNFC_PIO_CMD_TYPE_READ){
			SlaveDmaTransfer(hpnfc, sdmaThread, HPNFC_DATA_TRANSFER_READ, sdmaSize,
							 &transferredData);
		}
		if (transferredData != sdmaSize){
			vDbgMsg(DBG_GEN_MSG, DBG_CRIT, "%s",
					"not all data was transfferred by the SDMA\n");
			HPNFC_ReqRelease(hpnfc, sdmaThread, EIO);
		}

		break;
	case HPNFC_REQ_TYPE_CDMA:
		{
			HPNFC_CdmaCmd cdmaCmd;
			void* cdmaDesc = HPNFC_Cdma_GetUnfinished(cmdReq->req.cdma.descBuf);
			if (cdmaDesc == NULL){
				vDbgMsg(DBG_GEN_MSG, DBG_CRIT, "%s", "SDMA triggered but threre is no waiting request\n");
				HPNFC_ReqRelease(hpnfc, sdmaThread, EIO);
			}

			HPNFC_Cdma_DescParse(hpnfc, cdmaDesc, &cdmaCmd);
			/* if controller jumps to a next descriptor then update sw pointers */
			if (SlaveDmaIsCurrentCdmaDesc(hpnfc, sdmaThread, cdmaDesc) == 0)
				SlaveDmaInitState(hpnfc, sdmaThread, cdmaCmd.cmdCount, cdmaCmd.memPtr, cdmaDesc);


			if (cdmaCmd.cmdType == HPNFC_CDMA_CMD_TYPE_PROGRAM){
				SlaveDmaTransfer(hpnfc, sdmaThread, HPNFC_DATA_TRANSFER_WRITE, sdmaSize,
								 &transferredData);
			}
			if (cdmaCmd.cmdType == HPNFC_CDMA_CMD_TYPE_READ){
				SlaveDmaTransfer(hpnfc, sdmaThread, HPNFC_DATA_TRANSFER_READ, sdmaSize,
								 &transferredData);
			}
			if (transferredData != sdmaSize){
				vDbgMsg(DBG_GEN_MSG, DBG_CRIT, "%s",
						"not all data was transfferred by the SDMA\n");
				HPNFC_ReqRelease(hpnfc, sdmaThread, EIO);
			}
		}
		break;
	default:
		vDbgMsg(DBG_GEN_MSG, DBG_CRIT, "unknown request type %d\n",
				cmdReq->reqType);
	}

	/* after slave DMA transfer check if no error occurs */
	intStat = HPNFC_GetReg((uintptr_t)&(hpnfc->regs->cmd_status.intr_status));
	if (intStat & RF_CONTROLLER_REGS__RT_INTR_EN__SDMA_ERR_EN__MASK){
		HPNFC_SetReg((uintptr_t)&(hpnfc->regs->cmd_status.intr_status),
					 RF_CONTROLLER_REGS__RT_INTR_STATUS__SDMA_TRIGG__MASK);
		HPNFC_ReqRelease(hpnfc, sdmaThread, EIO);
		vDbgMsg(DBG_GEN_MSG, DBG_CRIT, "%s",
				"not allowed access to the Slave DMA interface was detected\n");
	}
}

static uint8_t IsHwThreadBusy(uint32_t trdStatus, uint8_t threadId)
{
	if ((trdStatus & (1uL << threadId)) == 0)
		return 0;
	else
		return 1;
}

static uint8_t IsHwThreadComplete(uint32_t trdCmpl, uint8_t threadId)
{
	if ((trdCmpl & (1uL << threadId)) == 1)
		return 1;
	else
		return 0;
}


/* fatal error
 * if an error not connected with specific thread occurs then
 * check all requests which SW status is pending and HW status is not busy and not finished
 * it means that something goes wrong and request/command was not executed at all and
 * should be marked as failed */
static void HandleFatalError(HPNFC_State *hpnfc)
{
	uint32_t trdStat;
	uint32_t trdCmpl;
	uint32_t intStat;
	int32_t trdId;

	trdStat = HPNFC_GetReg((uintptr_t)&(hpnfc->regs->cmd_status.trd_status));
	trdCmpl = HPNFC_GetReg((uintptr_t)&(hpnfc->regs->cmd_status.trd_comp_intr_status));
	intStat = HPNFC_GetReg((uintptr_t)&(hpnfc->regs->cmd_status.intr_status));

	HPNFC_SetReg((uintptr_t)&(hpnfc->regs->cmd_status.intr_status), HPNFC_INT_STATUS_ERR);

	printf("Fatal error: intr_status: %"PRIX32"\n", intStat);
	for (trdId = 0; trdId < hpnfc->ctrlInfo.threadsNumber; trdId++){
		if ((hpnfc->cmdReq[trdId] != NULL) && (hpnfc->cmdReq[trdId]->status == HPNFC_STATUS_PENDING)){
			if (!IsHwThreadBusy(trdStat, trdId) && !IsHwThreadComplete(trdCmpl, trdId))
				HPNFC_ReqRelease(hpnfc, trdId, EIO);
		}
	}
}

void HPNFC_Core_Isr(HPNFC_State *hpnfc)
{
	uint32_t trdErr;
	uint32_t trdTimeoutErr;
	uint32_t trdCmpl;
	uint32_t intStat;
	int32_t i;
	int32_t status;

	trdErr = HPNFC_GetReg((uintptr_t)&(hpnfc->regs->cmd_status.trd_error_intr_status));
	trdTimeoutErr = HPNFC_GetReg((uintptr_t)&(hpnfc->regs->cmd_status.trd_timeout_intr_status));
	trdCmpl = HPNFC_GetReg((uintptr_t)&(hpnfc->regs->cmd_status.trd_comp_intr_status));
	intStat = HPNFC_GetReg((uintptr_t)&(hpnfc->regs->cmd_status.intr_status));

	vDbgMsg(DBG_GEN_MSG, DBG_INFLOOP, "Interrupt status: thread complete status %"PRIX32","
			" thread error status %"PRIX32", thread error timeout status %"PRIX32
			" status %"PRIX32"\n", trdCmpl, trdErr, trdTimeoutErr, intStat);
	//printf("Interrupt status: thread complete status %"PRIX32","
	//        " thread error status %"PRIX32", thread error timeout status %"PRIX32
	//        " status %"PRIX32"\n", trdCmpl, trdErr, trdTimeoutErr, intStat);

	if (intStat & HPNFC_INT_STATUS_ERR){
		HandleFatalError(hpnfc);
	}

	for (i = 0; i < hpnfc->ctrlInfo.threadsNumber; i++){
		if ((trdCmpl & (1uL << i)) || trdErr & (1uL << i)){
			/* clear thread error interrupt */
			HPNFC_SetReg((uintptr_t)&(hpnfc->regs->cmd_status.trd_error_intr_status), 1 << i);
			HPNFC_SetReg((uintptr_t)&(hpnfc->regs->cmd_status.trd_comp_intr_status), 1 << i);

			status = HPNFC_ReqGetStatus(hpnfc, i);
			if (status != HPNFC_STATUS_PENDING)
				HPNFC_ReqRelease(hpnfc, i, status);
			else {
				if (trdErr & (1uL << i)){
					//vDbgMsg(DBG_GEN_MSG, DBG_CRIT, "%s", "Unknown error appear\n");
					printf("Unknown error appear\n");
					HPNFC_ReqRelease(hpnfc, i, EIO);
				}
			}
		}
		if (trdTimeoutErr & (1uL << i)){
			//vDbgMsg(DBG_GEN_MSG, DBG_CRIT, "Timeout error appear on thread %d\n", (int)i);
			printf("Timeout error appear on thread %d\n", (int)i);
			HPNFC_SetReg((uintptr_t)&(hpnfc->regs->cmd_status.trd_timeout_intr_status), 1 << i);
			HPNFC_ReqRelease(hpnfc, i, EIO);
		}
	}

	if (intStat & RF_CONTROLLER_REGS__RT_INTR_STATUS__SDMA_TRIGG__MASK){
		/* clear slave DMA interrupt */
		HPNFC_SetReg((uintptr_t)&(hpnfc->regs->cmd_status.intr_status),
					 RF_CONTROLLER_REGS__RT_INTR_STATUS__SDMA_TRIGG__MASK);

		HPNFC_Sdma_Isr(hpnfc);
	}
}

static void PioAssembleSetFeatures(HPNFC_CmdReq *cmdReq, uint8_t featAddr,
								   uint8_t featVal, uint8_t mem, uint8_t thread,
								   uint8_t volume)
{
	HPNFC_PioReq *req = &cmdReq->req.pio;

	cmdReq->thread = thread;
	cmdReq->reqType = HPNFC_REQ_TYPE_PIO;
	cmdReq->status = HPNFC_STATUS_PENDING;

	req->featureAddr = featAddr;
	req->bank = mem;
	req->featureValue = featVal;
	req->volume = volume;
	req->cmdType = HPNFC_PIO_CMD_TYPE_SET_FEATURES;
}

static void PioAssembleReset(HPNFC_CmdReq *cmdReq, uint8_t mem, uint8_t thread,
							 uint8_t volume)
{
	HPNFC_PioReq *req = &cmdReq->req.pio;

	cmdReq->thread = thread;
	cmdReq->reqType = HPNFC_REQ_TYPE_PIO;
	cmdReq->status = HPNFC_STATUS_PENDING;

	req->bank = mem;
	req->volume = volume;
	req->cmdType = HPNFC_PIO_CMD_TYPE_RESET;
}

static int32_t HPNFC_ReqWaitForFinish(HPNFC_State *hpnfc, HPNFC_CmdReq *cmdReq)
{
	volatile uint32_t Timeout = 0xFFFFF;
	while ((cmdReq->status == HPNFC_STATUS_PENDING) && --Timeout){
		if (!hpnfc->intEnabled)
			HPNFC_Core_Isr(hpnfc);
	}
	if (!Timeout)
		LogAndExit(EBUSY);

	if (cmdReq->status)
		LogAndExit(cmdReq->status);

	return (EOK);
}


/* set NAND flash memory device work mode */
static int32_t MemSetWorkMode(HPNFC_State *hpnfc, uint8_t workMode,
						  uint8_t timingMode)
{
	uint8_t flashWorkMode = timingMode;
	int32_t i;
	int32_t status;
	int32_t sumStatus = 0;
	HPNFC_CmdReq cmdReq[8];

	switch (workMode) {
	case HPNFC_WORK_MODE_NV_DDR:
		flashWorkMode |= (1 << 4);
		break;
	case HPNFC_WORK_MODE_NV_DDR2:
	case HPNFC_WORK_MODE_TOGG:
		flashWorkMode |= (2 << 4);
		break;
	case HPNFC_WORK_MODE_NV_DDR3:
		flashWorkMode |= (3 << 4);
		break;
	case HPNFC_WORK_MODE_SDR:
	default:
		break;
	}

	/* set new work mode in all connected flash devices */
	for (i = 0; i < hpnfc->devNum; i++){
		/* assemble set feature request */
		PioAssembleSetFeatures(&cmdReq[i], 1, flashWorkMode, i, i, 0);
		/* attach request to thread */
		status = HPNFC_ReqAttach(hpnfc, &cmdReq[i]);
		if (status)
			LogAndExit(status);
		/* send SET FEATURES command */
		status = HPNFC_PioSend(hpnfc, cmdReq[i].thread, &cmdReq[i].req.pio);
		if (status)
			LogAndExit(status);
	}

	/* wait until all requests are finished */
	for (i = 0; i < hpnfc->devNum; i++) {
		status = HPNFC_ReqWaitForFinish(hpnfc, &cmdReq[i]);
		if (status){
			vDbgMsg(DBG_GEN_MSG, DBG_CRIT, "set features command failed on bank %"PRId32"\n", i);
			sumStatus = status;
		}
	}
	if (sumStatus)
		LogAndExit(sumStatus);

	/* wait for controller IDLE */
	status = WaitForIdle(hpnfc);
	if (status)
		LogAndExit(status);

	return (EOK);
}

static void ControllerSetWorkMode(HPNFC_State *hpnfc, uint8_t workMode)
{
	uint32_t reg;

	/* set HPNFC controller work mode */
	reg = HPNFC_GetReg((uintptr_t)&(hpnfc->minictrl_regs->minictrl_settings.common_settings));
	reg &= ~RF_MINICTRL_REGS__RT_COMMON_SETTINGS__OPR_MODE__MASK;
	switch (workMode) {
	case HPNFC_WORK_MODE_NV_DDR:
		reg |= HPNFC_COMMON_SETT_OPR_MODE_NV_DDR;
		break;
	case HPNFC_WORK_MODE_TOGG:
	case HPNFC_WORK_MODE_NV_DDR2:
	case HPNFC_WORK_MODE_NV_DDR3:
		reg |= HPNFC_COMMON_SETT_OPR_MODE_TOGGLE;
		break;
	case HPNFC_WORK_MODE_SDR:
	default:
		reg |= HPNFC_COMMON_SETT_OPR_MODE_SDR;
	}

	HPNFC_SetReg((uintptr_t)&(hpnfc->minictrl_regs->minictrl_settings.common_settings), reg);
}

static int32_t VerifyControllerSetWorkMode(HPNFC_State *hpnfc, uint8_t workMode)
{
	switch (workMode) {
	case HPNFC_WORK_MODE_NV_DDR:
		if (!hpnfc->ctrlInfo.ddrSupport)
			LogAndExit(ENOTSUP);
		break;
	case HPNFC_WORK_MODE_TOGG:
	case HPNFC_WORK_MODE_NV_DDR2:
	case HPNFC_WORK_MODE_NV_DDR3:
		if (!hpnfc->ctrlInfo.ddr2Support)
			LogAndExit(ENOTSUP);
		break;
	case HPNFC_WORK_MODE_SDR:
		if (!hpnfc->ctrlInfo.asyncSupport)
			LogAndExit(ENOTSUP);
		break;
	default:
		LogAndExit(EINVAL);
	}
	return (EOK);
}

int32_t HPNFC_Core_SetWorkMode(HPNFC_State *hpnfc, HPNFC_WorkMode workMode, uint8_t timingMode,
						   HPNFC_PhyConfig *phyConfig)
{
	int32_t i;
	int32_t status;
	int32_t sumStatus = 0;
	HPNFC_CmdReq cmdReq[8];

	status = WaitForIdle(hpnfc);
	if (status)
		LogAndExit(status);

	status = VerifyControllerSetWorkMode(hpnfc, workMode);
	if (status)
		LogAndExit(status);

	/* set NF memory in known work mode by sending the reset command */
	/* select SDR mode in the controller */
	ControllerSetWorkMode(hpnfc, HPNFC_WORK_MODE_SDR);

	/* select default timings */
	HPNFC_PhySetPhyConfig(hpnfc, &hpnfc->defPhyConfig);

	/* set new work mode in all connected flash devices */
	for (i = 0; i < hpnfc->devNum; i++){
		/* assemble set feature request */
		PioAssembleReset(&cmdReq[i], i, i, 0);
		/* attach request to thread */
		status = HPNFC_ReqAttach(hpnfc, &cmdReq[i]);
		if (status)
			LogAndExit(status);
		/* execute pio command command */
		status = HPNFC_PioSend(hpnfc, cmdReq[i].thread, &cmdReq[i].req.pio);
		if (status)
			LogAndExit(status);
	}

	/* wait until all requests are finished */
	for (i = 0; i < hpnfc->devNum; i++) {
		status = HPNFC_ReqWaitForFinish(hpnfc, &cmdReq[i]);
		if (status){
			vDbgMsg(DBG_GEN_MSG, DBG_CRIT, "reset command failed on bank %"PRId32"\n", i);
			sumStatus = status;
		}
	}

	if (sumStatus)
		LogAndExit(sumStatus);

	status = MemSetWorkMode(hpnfc, workMode, timingMode);
	if (status)
		LogAndExit(status);

	/* clear phy dll reset */
	HPNFC_PhyWriteDllReset(hpnfc, 0);

	/* apply PHY configuration */
	HPNFC_PhySetPhyConfig(hpnfc, phyConfig);

	/* change controller work mode */
	ControllerSetWorkMode(hpnfc, workMode);

	/* set phy dll reset */
	HPNFC_PhyWriteDllReset(hpnfc, 1);

	/* wait for controller IDLE */
	status = WaitForIdle(hpnfc);
	if (status)
		LogAndExit(status);

	return (EOK);
}


static int32_t GetEccCorrCapIdx(struct HPNFC_State *hpnfc, uint8_t corrCap, uint8_t *index)
{
	int32_t status = ENOTSUP;
	int32_t i;

	for (i = 0; i < hpnfc->bchCfgInfo.numberOfCorrCaps; i++){
		if (corrCap == hpnfc->bchCfgInfo.corrCaps[i]){
			*index = (uint8_t)i;
			status = EOK;
			break;
		}
	}
	return (status);
}

int32_t HPNFC_Core_GetEccConfig(struct HPNFC_State *hpnfc,
							 HPNFC_EccConfiguration *eccConfig)
{
	uint32_t reg;
	uint8_t corrCapIdx;

	if (!hpnfc->ctrlInfo.eccAvailable){
		LogAndExit(ENOTSUP);
	}

	reg = HPNFC_GetReg((uintptr_t)&(hpnfc->regs->ctrl_config.ecc_config_0));

	eccConfig->eccEnabled = RF_CTRL_CONFIG__RT_ECC_CONFIG_0__ECC_ENABLE__READ(reg);
	eccConfig->scramblerEnabled = RF_CTRL_CONFIG__RT_ECC_CONFIG_0__SCRAMBLER_EN__READ(reg);

	corrCapIdx = RF_CTRL_CONFIG__RT_ECC_CONFIG_0__CORR_STR__READ(reg);
	eccConfig->corrCap = hpnfc->bchCfgInfo.corrCaps[corrCapIdx];

	reg = HPNFC_GetReg((uintptr_t)&(hpnfc->minictrl_regs->minictrl_settings.skip_bytes_offset));
	eccConfig->skipBytesOffset = RF_MINICTRL_REGS__RT_SKIP_BYTES_OFFSET__SKIP_BYTES_OFFSET__READ(reg);

	reg = HPNFC_GetReg((uintptr_t)&(hpnfc->minictrl_regs->minictrl_settings.skip_bytes_conf));
	eccConfig->marker = RF_MINICTRL_REGS__RT_SKIP_BYTES_CONF__MARKER__READ(reg);
	eccConfig->skipBytes = RF_MINICTRL_REGS__RT_SKIP_BYTES_CONF__SKIP_BYTES__READ(reg);

	return (EOK);
}

static int32_t VerifyBchCfg(struct HPNFC_State *hpnfc,
						HPNFC_EccConfiguration *eccConfig,
						HPNFC_TransferConfig *transferCfg)
{
	int32_t status;
	uint8_t corrCapIdx;

	if (eccConfig->eccEnabled){
		int32_t i;
		status = EINVAL;
		for (i = 0; i < 2; i++){
			if (hpnfc->bchCfgInfo.sectorSizes[i] == 0)
				break;
			if (hpnfc->bchCfgInfo.sectorSizes[i] == transferCfg->sectorSize)
				status = EOK;
		}
		if (status != EOK){
			vDbgMsg(DBG_GEN_MSG, DBG_CRIT, "Sector size %d is not supported when ECC is enabled. \n", transferCfg->sectorSize);
			LogAndExit(status);
		}

		status = GetEccCorrCapIdx(hpnfc, eccConfig->corrCap, &corrCapIdx);
		if (status){
			vDbgMsg(DBG_GEN_MSG, DBG_CRIT, "%s", "Wrong ECC corection capability settings.\n");
			LogAndExit(status);
		}
	}

	return (EOK);
}

int32_t HPNFC_Core_SetEccConfig(struct HPNFC_State *hpnfc,
							HPNFC_EccConfiguration *eccConfig)
{
	uint32_t reg;
	int32_t status;
	uint8_t corrCapIdx;
	HPNFC_TransferConfig transferCfg;

	if (!hpnfc->ctrlInfo.eccAvailable){
		LogAndExit(ENOTSUP);
	}

	HPNFC_Core_GetTransferConfig(hpnfc, &transferCfg);
	status = VerifyBchCfg(hpnfc, eccConfig, &transferCfg);
	if (status)
	{
		printf("VerifyBchCfg fail\n");
		LogAndExit(status);
	}

	status = WaitForIdle(hpnfc);
	if (status)
	{
		printf("WaitForIdle fail\n");
		LogAndExit(status);
	}

	HPNFC_Core_GetTransferConfig(hpnfc, &transferCfg);

	reg = HPNFC_GetReg((uintptr_t)&(hpnfc->regs->ctrl_config.ecc_config_0));

	RF_CTRL_CONFIG__RT_ECC_CONFIG_0__ECC_ENABLE__MODIFY(reg, eccConfig->eccEnabled);
	RF_CTRL_CONFIG__RT_ECC_CONFIG_0__SCRAMBLER_EN__MODIFY(reg, eccConfig->scramblerEnabled);

	status = GetEccCorrCapIdx(hpnfc, eccConfig->corrCap, &corrCapIdx);
	if (status){
		//vDbgMsg(DBG_GEN_MSG, DBG_CRIT, "%s", "Wrong ECC corection capability settings.\n");
		printf("Wrong ECC corection capability settings.\n");
		LogAndExit(status);
	}

	RF_CTRL_CONFIG__RT_ECC_CONFIG_0__CORR_STR__MODIFY(reg, corrCapIdx);

	HPNFC_SetReg((uintptr_t)&(hpnfc->regs->ctrl_config.ecc_config_0), reg);

	reg = RF_MINICTRL_REGS__RT_SKIP_BYTES_OFFSET__SKIP_BYTES_OFFSET__WRITE(eccConfig->skipBytesOffset);
	HPNFC_SetReg((uintptr_t)&(hpnfc->minictrl_regs->minictrl_settings.skip_bytes_offset), reg);

	reg = RF_MINICTRL_REGS__RT_SKIP_BYTES_CONF__MARKER__WRITE(eccConfig->marker)
		| RF_MINICTRL_REGS__RT_SKIP_BYTES_CONF__SKIP_BYTES__WRITE(eccConfig->skipBytes);
	HPNFC_SetReg((uintptr_t)&(hpnfc->minictrl_regs->minictrl_settings.skip_bytes_conf), reg);

	return (EOK);
}

int32_t HPNFC_Core_SetTransferConfig(struct HPNFC_State *hpnfc,
								  HPNFC_TransferConfig *transferCfg)
{
	uint32_t reg = 0;
	int32_t status;

	status = VerifyBchCfg(hpnfc, &hpnfc->eccConfig, transferCfg);
	if (status)
		LogAndExit(status);

	status = WaitForIdle(hpnfc);
	if (status)
		LogAndExit(status);

	RF_CTRL_CONFIG__RT_TRANSFER_CFG_0__SECTOR_CNT__MODIFY(reg, transferCfg->sectorCount);
	RF_CTRL_CONFIG__RT_TRANSFER_CFG_0__OFFSET__MODIFY(reg, transferCfg->offset);
	HPNFC_SetReg((uintptr_t)&(hpnfc->regs->ctrl_config.transfer_cfg_0), reg);

	reg = 0;
	RF_CTRL_CONFIG__RT_TRANSFER_CFG_1__SECTOR_SIZE__MODIFY(reg, transferCfg->sectorSize);
	RF_CTRL_CONFIG__RT_TRANSFER_CFG_1__LAST_SECTOR_SIZE__MODIFY(reg, transferCfg->lastSectorSize);
	HPNFC_SetReg((uintptr_t)&(hpnfc->regs->ctrl_config.transfer_cfg_1), reg);

	return (EOK);
}

void HPNFC_Core_GetTransferConfig(struct HPNFC_State *hpnfc,
								  HPNFC_TransferConfig *transferCfg)
{
	uint32_t reg;

	reg = HPNFC_GetReg((uintptr_t)&(hpnfc->regs->ctrl_config.transfer_cfg_0));
	transferCfg->sectorCount = RF_CTRL_CONFIG__RT_TRANSFER_CFG_0__SECTOR_CNT__READ(reg);
	transferCfg->offset = RF_CTRL_CONFIG__RT_TRANSFER_CFG_0__OFFSET__READ(reg);

	reg = HPNFC_GetReg((uintptr_t)&(hpnfc->regs->ctrl_config.transfer_cfg_1));
	transferCfg->sectorSize = RF_CTRL_CONFIG__RT_TRANSFER_CFG_1__SECTOR_SIZE__READ(reg);
	transferCfg->lastSectorSize = RF_CTRL_CONFIG__RT_TRANSFER_CFG_1__LAST_SECTOR_SIZE__READ(reg);
}

static uint32_t GetBlockShift(uint32_t pagesPerBlock)
{
	int i = 0;
	int msbSetBit = 0;
	for (i = 0; i < 32; i++){
		if ((pagesPerBlock & (1uL << i)) != 0)
			msbSetBit = i;
	}
	return msbSetBit;
}

void HPNFC_Core_GetDeviceLayout(struct HPNFC_State *hpnfc,
								HPNFC_DeviceLayout *devLayout)
{
	uint32_t reg;

	reg = HPNFC_GetReg((uintptr_t)&(hpnfc->regs->controller_params.nf_device_areas));  // 0x80C
	devLayout->mainSize = RF_CONTROLLER_PARAMS__RT_NF_DEVICE_AREAS__MAIN_AREA_SIZE__READ(reg);
	devLayout->spareSize = RF_CONTROLLER_PARAMS__RT_NF_DEVICE_AREAS__SPARE_AREA_SIZE__READ(reg);

	reg = HPNFC_GetReg((uintptr_t)&(hpnfc->regs->ctrl_config.nf_dev_layout));  // 0x424
	devLayout->pagesPerBlock = RF_CTRL_CONFIG__RT_NF_DEV_LAYOUT__PPB__READ(reg);

	reg = HPNFC_GetReg((uintptr_t)&(hpnfc->regs->controller_params.device_params_0));   // 0x810
	devLayout->lunNumber = RF_CONTROLLER_PARAMS__RT_ONFI_DEVICE_LUNS__NO_OF_LUNS__READ(reg);
	devLayout->numberOfPlanes =
		(unsigned int)(1 << RF_CONTROLLER_PARAMS__RT_ONFI_DEVICE_LUNS__PLANE_ADDR_BITS__READ(reg)); // 0x81c

	devLayout->blocksPerLun =
		HPNFC_GetReg((uintptr_t)&(hpnfc->regs->controller_params.device_blocks_per_lun));

	devLayout->blockShift = GetBlockShift(devLayout->pagesPerBlock);
}

int32_t HPNFC_Core_SetDeviceSubLayout(struct HPNFC_State *hpnfc,
								  HPNFC_DeviceSubLayout *devSubLayout)
{
	uint32_t reg = 0;
	int32_t status;

	status = WaitForIdle(hpnfc);
	if (status)
		LogAndExit(status);

	reg = HPNFC_GetReg((uintptr_t)&(hpnfc->regs->ctrl_config.nf_dev_layout));
	RF_CTRL_CONFIG__RT_NF_DEV_LAYOUT__PPB__MODIFY(reg, devSubLayout->pagesPerBlock);
	RF_CTRL_CONFIG__RT_NF_DEV_LAYOUT__LN__MODIFY(reg, devSubLayout->lunNumber);
	HPNFC_SetReg((uintptr_t)&(hpnfc->regs->ctrl_config.nf_dev_layout), reg);

	return (EOK);
}

int32_t HPNFC_Core_SetLunInterlConf(struct HPNFC_State *hpnfc,
								 HPNFC_LunInterleavedConfig *lunInterleavedConfig)
{
	uint32_t reg;
	int32_t status;

	status = WaitForIdle(hpnfc);
	if (status)
		LogAndExit(status);

	switch (lunInterleavedConfig->lunStatSelect){
	case HPNFC_LUN_STAT_SELECT_78:
		reg = RF_CTRL_CONFIG__RT_LUN_STATUS_CMD__LUN_STAT_SEL__WRITE(0);
		break;
	case HPNFC_LUN_STAT_SELECT_F1_F2:
		reg = RF_CTRL_CONFIG__RT_LUN_STATUS_CMD__LUN_STAT_SEL__WRITE(1);
		break;
	default:
		LogAndExit(EINVAL);
	}
	HPNFC_SetReg((uintptr_t)&(hpnfc->regs->ctrl_config.lun_status_cmd), reg);

	if (lunInterleavedConfig->lunAddrOff > 0x1F)
		LogAndExit(EINVAL);


	reg = RF_CTRL_CONFIG__RT_LUN_ADDR_OFFSET__LUN_ADDR_OFFSET__WRITE(lunInterleavedConfig->lunAddrOff);
	HPNFC_SetReg((uintptr_t)&(hpnfc->regs->ctrl_config.lun_addr_offset), reg);

	reg = RF_CTRL_CONFIG__RT_LUN_INTERL_CMD__PROGRAM_AFTER_READ__WRITE(lunInterleavedConfig->programAferRead);
	HPNFC_SetReg((uintptr_t)&(hpnfc->regs->ctrl_config.lun_interleaved_cmd), reg);

	reg = HPNFC_GetReg((uintptr_t)&(hpnfc->regs->ctrl_config.nf_dev_layout));
	RF_CTRL_CONFIG__RT_NF_DEV_LAYOUT__LUN_EN__MODIFY(reg, lunInterleavedConfig->multiLunEn);
	HPNFC_SetReg((uintptr_t)&(hpnfc->regs->ctrl_config.nf_dev_layout), reg);

	return (EOK);
}

void HPNFC_Core_GetLunInterlConf(struct HPNFC_State *hpnfc,
								 HPNFC_LunInterleavedConfig *lunInterleavedConfig)
{
	uint32_t reg;

	reg = HPNFC_GetReg((uintptr_t)&(hpnfc->regs->ctrl_config.lun_status_cmd));
	reg = RF_CTRL_CONFIG__RT_LUN_STATUS_CMD__LUN_STAT_SEL__READ(reg);
	switch (reg){
	case 0:
		lunInterleavedConfig->lunStatSelect = HPNFC_LUN_STAT_SELECT_78;
		break;
	case 1:
		lunInterleavedConfig->lunStatSelect = HPNFC_LUN_STAT_SELECT_F1_F2;
		break;
	}

	reg = HPNFC_GetReg((uintptr_t)&(hpnfc->regs->ctrl_config.lun_addr_offset));
	lunInterleavedConfig->lunAddrOff = RF_CTRL_CONFIG__RT_LUN_ADDR_OFFSET__LUN_ADDR_OFFSET__READ(reg);

	reg = HPNFC_GetReg((uintptr_t)&(hpnfc->regs->ctrl_config.lun_interleaved_cmd));
	lunInterleavedConfig->programAferRead = RF_CTRL_CONFIG__RT_LUN_INTERL_CMD__PROGRAM_AFTER_READ__READ(reg);

	reg = HPNFC_GetReg((uintptr_t)&(hpnfc->regs->ctrl_config.nf_dev_layout));
	lunInterleavedConfig->multiLunEn = RF_CTRL_CONFIG__RT_NF_DEV_LAYOUT__LUN_EN__READ(reg);
}


int32_t HPNFC_Core_GetBootStatus(HPNFC_State *hpnfc, HPNFC_BootStatus *bootStatus,
							 uint8_t *cpyId)
{
	uint32_t reg;

	if (hpnfc->ctrlInfo.bootAvailable == 0){
		LogAndExit(ENOTSUP);
	}

	reg = HPNFC_GetReg((uintptr_t)&(hpnfc->regs->cmd_status.boot_status));

	*cpyId = RF_CONTROLLER_REGS__RT_BOOT_STAT__CPY_ID__READ(reg);

	if (RF_CONTROLLER_REGS__RT_BOOT_STAT__BCH_ERR_TYPE__READ(reg) == 1)
		*bootStatus = HPNFC_BOOT_STATUS_ECC_CORR_ERR;

	if (RF_CONTROLLER_REGS__RT_BOOT_STAT__BCH_ERR_TYPE__READ(reg) == 2)
		*bootStatus = HPNFC_BOOT_STATUS_ECC_UNCORR_ERR;

	if (RF_CONTROLLER_REGS__RT_BOOT_STAT__PHY_ERR__READ(reg))
		*bootStatus = HPNFC_BOOT_STATUS_PHY_ERR;

	if (RF_CONTROLLER_REGS__RT_BOOT_STAT__INIT_ERR__READ(reg))
		*bootStatus = HPNFC_BOOT_STATUS_INIT_ERR;

	return (EOK);
}


int32_t HPNFC_Core_SetStatusPollingConfig(HPNFC_State *hpnfc,
									   HPNFC_StatusPollingConfig *statusPollingCfg)
{
	uint32_t reg;
	int32_t status;

	status = WaitForIdle(hpnfc);
	if (status)
		LogAndExit(status);

	reg = RF_CTRL_CONFIG__RT_STATUS_CTRL_0__READY_MASK__WRITE(statusPollingCfg->readyMask);
	reg |= RF_CTRL_CONFIG__RT_STATUS_CTRL_0__READY_VALUE__WRITE(statusPollingCfg->readyValue);

	if (statusPollingCfg->disableStatusPolling)
		reg |= RF_CTRL_CONFIG__RT_STATUS_CTRL_0__RB_ENABLE__MASK;

	HPNFC_SetReg((uintptr_t)&(hpnfc->regs->ctrl_config.rdst_ctrl_0), reg);
	HPNFC_SetReg((uintptr_t)&(hpnfc->regs->ctrl_config.long_polling), statusPollingCfg->longPolling);
	HPNFC_SetReg((uintptr_t)&(hpnfc->regs->ctrl_config.short_polling), statusPollingCfg->shortPolling);

	return (EOK);
}

void HPNFC_Core_GetStatusPollingConfig(HPNFC_State *hpnfc,
									   HPNFC_StatusPollingConfig *statusPollingCfg)
{
	uint32_t reg;
	reg = HPNFC_GetReg((uintptr_t)&(hpnfc->regs->ctrl_config.rdst_ctrl_0));
	statusPollingCfg->readyMask = RF_CTRL_CONFIG__RT_STATUS_CTRL_0__READY_MASK__READ(reg);
	statusPollingCfg->readyValue = RF_CTRL_CONFIG__RT_STATUS_CTRL_0__READY_VALUE__READ(reg);

	if (reg & RF_CTRL_CONFIG__RT_STATUS_CTRL_0__RB_ENABLE__MASK)
		statusPollingCfg->disableStatusPolling = 1;
	else
		statusPollingCfg->disableStatusPolling = 0;

	statusPollingCfg->longPolling = HPNFC_GetReg((uintptr_t)&(hpnfc->regs->ctrl_config.long_polling));
	statusPollingCfg->shortPolling = HPNFC_GetReg((uintptr_t)&(hpnfc->regs->ctrl_config.short_polling));
}

int32_t HPNFC_Core_SetAutoStatusConfig(HPNFC_State *hpnfc,
									HPNFC_AutoStatusConfig *autoStatusConfig)
{
	uint32_t reg;
	int32_t status;

	status = WaitForIdle(hpnfc);
	if (status)
		LogAndExit(status);

	reg = RF_CTRL_CONFIG__RT_STATUS_CTRL_1__ERROR_MASK__WRITE(autoStatusConfig->errorMask);
	reg |= RF_CTRL_CONFIG__RT_STATUS_CTRL_1__ERROR_VALUE__WRITE(autoStatusConfig->errorValue);

	HPNFC_SetReg((uintptr_t)&(hpnfc->regs->ctrl_config.rdst_ctrl_1), reg);

	return (EOK);
}

void HPNFC_Core_GetAutoStatusConfig(HPNFC_State *hpnfc,
									HPNFC_AutoStatusConfig *autoStatusConfig)
{
	uint32_t reg;
	reg = HPNFC_GetReg((uintptr_t)&(hpnfc->regs->ctrl_config.rdst_ctrl_1));
	autoStatusConfig->errorMask = RF_CTRL_CONFIG__RT_STATUS_CTRL_1__ERROR_MASK__READ(reg);
	autoStatusConfig->errorValue = RF_CTRL_CONFIG__RT_STATUS_CTRL_1__ERROR_VALUE__READ(reg);
}

void HPNFC_Core_GetMultiplaneConfig(HPNFC_State *hpnfc,
									HPNFC_MultiplaneConfig* multiplaneConfig)
{
	uint32_t reg;

	reg = HPNFC_GetReg((uintptr_t)&(hpnfc->regs->ctrl_config.multiplane_config));
	multiplaneConfig->mplRdEn = RF_CTRL_CONFIG__RT_MULTIPLANE_CONFIG__MPL_RD_EN__READ(reg);
	multiplaneConfig->mplWrEn = RF_CTRL_CONFIG__RT_MULTIPLANE_CONFIG__MPL_WR_EN__READ(reg);
	multiplaneConfig->planesNumber = (1uL << RF_CTRL_CONFIG__RT_MULTIPLANE_CONFIG__MPL_PL_NUM__READ(reg));
	multiplaneConfig->mplPrgSeq = RF_CTRL_CONFIG__RT_MULTIPLANE_CONFIG__MPL_PRG_SEQ__READ(reg);
	multiplaneConfig->mplRdSeq = RF_CTRL_CONFIG__RT_MULTIPLANE_CONFIG__MPL_RD_SEQ__READ(reg);
	multiplaneConfig->mplEraseSeq = RF_CTRL_CONFIG__RT_MULTIPLANE_CONFIG__MPL_ERASE_SEQ__READ(reg);
	multiplaneConfig->lastWrCmd = RF_CTRL_CONFIG__RT_MULTIPLANE_CONFIG__LAST_WR_CMD__READ(reg);
	multiplaneConfig->extStatusEn = RF_CTRL_CONFIG__RT_MULTIPLANE_CONFIG__PL_STATUS_EN__READ(reg);
	multiplaneConfig->copyBckCmd = RF_CTRL_CONFIG__RT_MULTIPLANE_CONFIG__MPL_CPBK_RD_SEQ__READ(reg);
}

int32_t HPNFC_Core_SetMultiplaneConfig(HPNFC_State *hpnfc,
								   HPNFC_MultiplaneConfig* multiplaneConfig)
{
	uint32_t reg = 0;
	int32_t status;

	status = WaitForIdle(hpnfc);
	if (status)
		LogAndExit(status);

	if ((multiplaneConfig->mplRdEn > 1) || (multiplaneConfig->mplWrEn > 1))
		LogAndExit(EINVAL);

	if (multiplaneConfig->extStatusEn > 1)
		LogAndExit(EINVAL);

	if (multiplaneConfig->copyBckCmd > HPNFC_COPY_BCK_CMD_SEQ_1)
		LogAndExit(EINVAL);

	if (multiplaneConfig->mplEraseSeq > HPNFC_MPL_ERASE_SEQ_1)
		LogAndExit(EINVAL);

	if (multiplaneConfig->lastWrCmd > HPNFC_LAST_WR_CMD_SEQ_1)
		LogAndExit(EINVAL);

	reg = RF_CTRL_CONFIG__RT_MULTIPLANE_CONFIG__MPL_RD_EN__WRITE(multiplaneConfig->mplRdEn);
	reg |= RF_CTRL_CONFIG__RT_MULTIPLANE_CONFIG__MPL_WR_EN__WRITE(multiplaneConfig->mplWrEn);

	switch (multiplaneConfig->planesNumber){
	case HPNFC_PLANES_NUMBER_1:
		reg |= RF_CTRL_CONFIG__RT_MULTIPLANE_CONFIG__MPL_PL_NUM__WRITE(0);
		break;
	case HPNFC_PLANES_NUMBER_2:
		reg |= RF_CTRL_CONFIG__RT_MULTIPLANE_CONFIG__MPL_PL_NUM__WRITE(1);
		break;
	case HPNFC_PLANES_NUMBER_4:
		reg |= RF_CTRL_CONFIG__RT_MULTIPLANE_CONFIG__MPL_PL_NUM__WRITE(2);
		break;
	case HPNFC_PLANES_NUMBER_8:
		reg |= RF_CTRL_CONFIG__RT_MULTIPLANE_CONFIG__MPL_PL_NUM__WRITE(3);
		break;
	default:
		LogAndExit(EINVAL);
	}

	if (multiplaneConfig->mplPrgSeq > HPNFC_MPL_PRG_SEQ_2)
		LogAndExit(EINVAL);

	switch (multiplaneConfig->mplRdSeq){
	case HPNFC_MPL_RD_SEQ_0:
	case HPNFC_MPL_RD_SEQ_1:
	case HPNFC_MPL_RD_SEQ_2:
	case HPNFC_MPL_RD_SEQ_4:
	case HPNFC_MPL_RD_SEQ_6:
		break;
	default:
		LogAndExit(EINVAL);
	}

	if ((multiplaneConfig->extStatusEn == 1) && (hpnfc->ctrlInfo.extStatus == 0)){
		vDbgMsg(DBG_GEN_MSG, DBG_WARN, "%s", "Warning! HPNFC controller does not support extended status feature\n");
	}

	reg |= RF_CTRL_CONFIG__RT_MULTIPLANE_CONFIG__MPL_PRG_SEQ__WRITE(multiplaneConfig->mplPrgSeq);
	reg |= RF_CTRL_CONFIG__RT_MULTIPLANE_CONFIG__MPL_RD_SEQ__WRITE(multiplaneConfig->mplRdSeq);
	reg |= RF_CTRL_CONFIG__RT_MULTIPLANE_CONFIG__MPL_ERASE_SEQ__WRITE(multiplaneConfig->mplEraseSeq);
	reg |= RF_CTRL_CONFIG__RT_MULTIPLANE_CONFIG__LAST_WR_CMD__WRITE(multiplaneConfig->lastWrCmd);
	reg |= RF_CTRL_CONFIG__RT_MULTIPLANE_CONFIG__MPL_CPBK_RD_SEQ__WRITE(multiplaneConfig->copyBckCmd);
	reg |= RF_CTRL_CONFIG__RT_MULTIPLANE_CONFIG__PL_STATUS_EN__WRITE(multiplaneConfig->extStatusEn);

	HPNFC_SetReg((uintptr_t)&(hpnfc->regs->ctrl_config.multiplane_config), reg);

	return (EOK);
}

void HPNFC_Core_GetCacheConfig(HPNFC_State *hpnfc, HPNFC_CacheConfig* cacheConfig)
{
	uint32_t reg;

	reg = HPNFC_GetReg((uintptr_t)&(hpnfc->regs->ctrl_config.cache_config));

	cacheConfig->cacheRdEn = RF_CTRL_CONFIG__RT_CACHE_CONFIG__CACHE_RD_EN__READ(reg);
	cacheConfig->cacheWrEn = RF_CTRL_CONFIG__RT_CACHE_CONFIG__CACHE_WR_EN__READ(reg);
}

int32_t HPNFC_Core_SetCacheConfig(HPNFC_State *hpnfc, HPNFC_CacheConfig* cacheConfig)
{
	uint32_t reg;
	int32_t status;

	status = WaitForIdle(hpnfc);
	if (status)
		LogAndExit(status);

	reg = RF_CTRL_CONFIG__RT_CACHE_CONFIG__CACHE_RD_EN__WRITE(cacheConfig->cacheRdEn)
		| RF_CTRL_CONFIG__RT_CACHE_CONFIG__CACHE_WR_EN__WRITE(cacheConfig->cacheWrEn);

	HPNFC_SetReg((uintptr_t)&(hpnfc->regs->ctrl_config.cache_config), reg);

	return (EOK);
}

int32_t HPNFC_Core_GetDebugData(HPNFC_State *hpnfc, HPNFC_DebugReq *dbgReq)
{
	uint32_t reg;
	int32_t status;

	if (hpnfc->ctrlInfo.ctrlRev < 6)
		LogAndExit(ENOTSUP);

	if ((dbgReq->synchMode < HPNFC_DBG_SYNCH_MODE_NON_SYNCH)
		|| (dbgReq->synchMode > HPNFC_DBG_SYNCH_MODE_SYNCH_BY_BCH_CLK)){
		LogAndExit(EINVAL);
	}

	status = WaitForIdle(hpnfc);
	if (status)
		LogAndExit(status);

	reg = HPNFC_GetReg((uintptr_t)&(hpnfc->regs->dbg_regs.dbg_ctrl));

	/* check if register interface is selected */
	if (RF_DBG_REGS__RT_DBG_CTRL__INT_SEL__READ(reg) == 0){
		DbgMsg(DBG_GEN_MSG, DBG_CRIT, "%s", "Error: software interface is disabled."
			   "It need to be enabled before accessing debug registers.");
		LogAndExit(EIO);
	}

	reg = HPNFC_GetReg((uintptr_t)&(hpnfc->regs->dbg_regs.dbg_ctrl));
	if (dbgReq->synchMode == HPNFC_DBG_SYNCH_MODE_NON_SYNCH){
		RF_DBG_REGS__RT_DBG_CTRL__N_LATCH_REQ__MODIFY(reg, 0);
		RF_DBG_REGS__RT_DBG_CTRL__B_LATCH_REQ__MODIFY(reg, 0);
		HPNFC_SetReg((uintptr_t)&(hpnfc->regs->dbg_regs.dbg_ctrl), reg);
	}
	else if (dbgReq->synchMode == HPNFC_DBG_SYNCH_MODE_SYNCH_BY_NF_CLK){
		RF_DBG_REGS__RT_DBG_CTRL__N_LATCH_REQ__MODIFY(reg, 1);
		RF_DBG_REGS__RT_DBG_CTRL__B_LATCH_REQ__MODIFY(reg, 0);
		HPNFC_SetReg((uintptr_t)&(hpnfc->regs->dbg_regs.dbg_ctrl), reg);
		if (TimeoutWaitReg((uintptr_t)&(hpnfc->regs->dbg_regs.dbg_stat),
						   RF_DBG_REGS__RT_DBG_STAT__N_LATCH_ACK__MASK,
						   RF_DBG_REGS__RT_DBG_STAT__N_LATCH_ACK__MASK, 1, 10000))
			LogAndExit(EIO);
	} else if (dbgReq->synchMode == HPNFC_DBG_SYNCH_MODE_SYNCH_BY_BCH_CLK){
		RF_DBG_REGS__RT_DBG_CTRL__N_LATCH_REQ__MODIFY(reg, 0);
		RF_DBG_REGS__RT_DBG_CTRL__B_LATCH_REQ__MODIFY(reg, 1);
		HPNFC_SetReg((uintptr_t)&(hpnfc->regs->dbg_regs.dbg_ctrl), reg);
		if (TimeoutWaitReg((uintptr_t)&(hpnfc->regs->dbg_regs.dbg_stat),
						   RF_DBG_REGS__RT_DBG_STAT__B_LATCH_ACK__MASK,
						   RF_DBG_REGS__RT_DBG_STAT__B_LATCH_ACK__MASK, 1, 10000))
			LogAndExit(EIO);
	}

	reg = HPNFC_GetReg((uintptr_t)&(hpnfc->regs->dbg_regs.dbg_ctrl));
	RF_DBG_REGS__RT_DBG_CTRL__WORD_SEL__MODIFY(reg, dbgReq->wordSel);
	HPNFC_SetReg((uintptr_t)&(hpnfc->regs->dbg_regs.dbg_ctrl), reg);

	dbgReq->data = HPNFC_GetReg((uintptr_t)&(hpnfc->regs->dbg_regs.dbg_data));

	/* clear stats */
	reg = 0;
	RF_DBG_REGS__RT_DBG_STAT__N_LATCH_ACK__MODIFY(reg, 1);
	RF_DBG_REGS__RT_DBG_STAT__B_LATCH_ACK__MODIFY(reg, 1);
	HPNFC_SetReg((uintptr_t)&(hpnfc->regs->dbg_regs.dbg_stat), reg);

	return (EOK);
}

int32_t HPNFC_Core_SetDebugIfType(HPNFC_State *hpnfc, HPNFC_DebugIfType ifType)
{
	uint32_t reg;
	int32_t status;

	if (hpnfc->ctrlInfo.ctrlRev < 6)
		LogAndExit(ENOTSUP);

	status = WaitForIdle(hpnfc);
	if (status)
		LogAndExit(status);

	reg = HPNFC_GetReg((uintptr_t)&(hpnfc->regs->dbg_regs.dbg_ctrl));
	if (ifType == HPNFC_DBG_IF_TYPE_REGISTER)
		RF_DBG_REGS__RT_DBG_CTRL__INT_SEL__MODIFY(reg, 1);
	else
		RF_DBG_REGS__RT_DBG_CTRL__INT_SEL__MODIFY(reg, 0);

	HPNFC_SetReg((uintptr_t)&(hpnfc->regs->dbg_regs.dbg_ctrl), reg);

	return (EOK);
}

int32_t HPNFC_Core_GetDebugIfType(HPNFC_State *hpnfc, HPNFC_DebugIfType* ifType)
{
	uint32_t reg;

	if (hpnfc->ctrlInfo.ctrlRev < 6)
		LogAndExit(ENOTSUP);

	reg = HPNFC_GetReg((uintptr_t)&(hpnfc->regs->dbg_regs.dbg_ctrl));
	if (RF_DBG_REGS__RT_DBG_CTRL__INT_SEL__READ(reg) == 1)
		*ifType = HPNFC_DBG_IF_TYPE_REGISTER;
	else
		*ifType = HPNFC_DBG_IF_TYPE_HARDWARE;

	return (EOK);
}

int32_t HPNFC_Core_SetDiConfig(HPNFC_State *hpnfc, HPNFC_DiConfig* diConfig)
{
	uint32_t reg;
	int32_t status;

	if (hpnfc->ctrlInfo.dataIntergity == 0)
		LogAndExit(ENOTSUP);

	status = WaitForIdle(hpnfc);
	if (status)
		LogAndExit(status);

	reg = HPNFC_GetReg((uintptr_t)&(hpnfc->regs->di_config.di_control));

	if (diConfig->parityType == HPNFC_DI_PARITY_TYPE_ODD){
		RF_DI_REGS__RT_DI_CONTROL__DI_TYPE__MODIFY(reg, 1);
	}
	else if (diConfig->parityType == HPNFC_DI_PARITY_TYPE_EVEN){
		RF_DI_REGS__RT_DI_CONTROL__DI_TYPE__MODIFY(reg, 0);
	}
	else {
		LogAndExit(EINVAL);
	}

	RF_DI_REGS__RT_DI_CONTROL__DI_PARITY_EN__MODIFY(reg, diConfig->parityEn);
	RF_DI_REGS__RT_DI_CONTROL__DI_CRC_EN__MODIFY(reg, diConfig->crcEn);
	RF_DI_REGS__RT_DI_CONTROL__DI_PAR_RSP_EN__MODIFY(reg, diConfig->errorRespEn);

	HPNFC_SetReg((uintptr_t)&(hpnfc->regs->di_config.di_control), reg);

	return EOK;
}

int32_t HPNFC_Core_GetDiConfig(HPNFC_State *hpnfc, HPNFC_DiConfig* diConfig)
{
	uint32_t reg;

	if (hpnfc->ctrlInfo.dataIntergity == 0)
		LogAndExit(ENOTSUP);

	reg = HPNFC_GetReg((uintptr_t)&(hpnfc->regs->di_config.di_control));

	if (RF_DI_REGS__RT_DI_CONTROL__DI_TYPE__READ(reg) == 0){
		diConfig->parityType = HPNFC_DI_PARITY_TYPE_EVEN;
	}
	else {
		diConfig->parityType = HPNFC_DI_PARITY_TYPE_ODD;
	}

	diConfig->parityEn = RF_DI_REGS__RT_DI_CONTROL__DI_PARITY_EN__READ(reg);
	diConfig->crcEn = RF_DI_REGS__RT_DI_CONTROL__DI_CRC_EN__READ(reg);
	diConfig->errorRespEn = RF_DI_REGS__RT_DI_CONTROL__DI_PAR_RSP_EN__READ(reg);

	return EOK;
}

int32_t HPNFC_Core_SetDiErrInjConfig(HPNFC_State *hpnfc, HPNFC_DiErrInjConfig* diErrInjConfig)
{
	uint32_t reg;
	int32_t status;

	if (hpnfc->ctrlInfo.dataIntergity == 0)
		LogAndExit(ENOTSUP);

	status = WaitForIdle(hpnfc);
	if (status)
		LogAndExit(status);

	reg = HPNFC_GetReg((uintptr_t)&(hpnfc->regs->di_config.di_control));

	if (diErrInjConfig->errInjType == HPNFC_DI_ERR_INJ_TYPE_CONT) {
		RF_DI_REGS__RT_DI_CONTROL__DI_INJ_TYPE__MODIFY(reg, 1);
	}
	else if (diErrInjConfig->errInjType == HPNFC_DI_ERR_INJ_TYPE_ONE_TIME) {
		RF_DI_REGS__RT_DI_CONTROL__DI_INJ_TYPE__MODIFY(reg, 0);
	}
	else {
		vDbgMsg(DBG_GEN_MSG, DBG_CRIT, "Error: Wrong error injection type %s", "\n");
		LogAndExit(EINVAL);
	}
	HPNFC_SetReg((uintptr_t)&(hpnfc->regs->di_config.di_control), reg);

	reg = HPNFC_GetReg((uintptr_t)&(hpnfc->regs->di_config.di_inject0));
	RF_DI_REGS__RT_DI_INJECT0__DI_INJ_DESC_VALUE__MODIFY(reg, diErrInjConfig->injDescVal);
	RF_DI_REGS__RT_DI_INJECT0__DI_INJ_DESC_WR_ADDR__MODIFY(reg, diErrInjConfig->injDescWrAddr);
	RF_DI_REGS__RT_DI_INJECT0__DI_INJ_DESC_RD_ADDR__MODIFY(reg, diErrInjConfig->injDescRdAddr);
	RF_DI_REGS__RT_DI_INJECT0__DI_INJ_CTX_ADDR__MODIFY(reg, diErrInjConfig->injCtxAddr);
	RF_DI_REGS__RT_DI_INJECT0__DI_INJ_CTX_VALUE__MODIFY(reg, diErrInjConfig->injCtxValue);
	HPNFC_SetReg((uintptr_t)&(hpnfc->regs->di_config.di_inject0), reg);

	reg = HPNFC_GetReg((uintptr_t)&(hpnfc->regs->di_config.di_inject1));
	RF_DI_REGS__RT_DI_INJECT1__DI_INJ_DATA_RD_PAR__MODIFY(reg, diErrInjConfig->injDataParityRd);
	RF_DI_REGS__RT_DI_INJECT1__DI_INJ_DATA_WR_CRC__MODIFY(reg, diErrInjConfig->injDataWrCrc);
	RF_DI_REGS__RT_DI_INJECT1__DI_INJ_DATA_WR_ADDR__MODIFY(reg, diErrInjConfig->injDataWrAddr);
	RF_DI_REGS__RT_DI_INJECT1__DI_INJ_DATA_RD_ADDR__MODIFY(reg, diErrInjConfig->injDataRdAddr);
	RF_DI_REGS__RT_DI_INJECT1__DI_INJ_REG_VALUE__MODIFY(reg, diErrInjConfig-> injRegVal);
	HPNFC_SetReg((uintptr_t)&(hpnfc->regs->di_config.di_inject1), reg);

	reg = HPNFC_GetReg((uintptr_t)&(hpnfc->regs->di_config.di_inject2));
	RF_DI_REGS__RT_DI_INJECT2__DI_INJ_RMP_VALUE__MODIFY(reg, diErrInjConfig->injRmpVal);
	RF_DI_REGS__RT_DI_INJECT2__DI_INJ_RMP_ADDR__MODIFY(reg, diErrInjConfig->injRmpAddr);
	HPNFC_SetReg((uintptr_t)&(hpnfc->regs->di_config.di_inject2), reg);

	return EOK;
}

int32_t HPNFC_Core_GetDiErrInjConfig(HPNFC_State *hpnfc, HPNFC_DiErrInjConfig* diErrInjConfig)
{
	int32_t status;
	uint32_t reg;

	if (hpnfc->ctrlInfo.dataIntergity == 0)
		LogAndExit(ENOTSUP);

	status = WaitForIdle(hpnfc);
	if (status)
		LogAndExit(status);

	reg = HPNFC_GetReg((uintptr_t)&(hpnfc->regs->di_config.di_control));

	if (RF_DI_REGS__RT_DI_CONTROL__DI_INJ_TYPE__READ(reg) == 1) {
		diErrInjConfig->errInjType = HPNFC_DI_ERR_INJ_TYPE_CONT;
	}
	else {
		diErrInjConfig->errInjType = HPNFC_DI_ERR_INJ_TYPE_ONE_TIME;
	}

	reg = HPNFC_GetReg((uintptr_t)&(hpnfc->regs->di_config.di_inject0));
	diErrInjConfig->injDescVal = RF_DI_REGS__RT_DI_INJECT0__DI_INJ_DESC_VALUE__READ(reg);
	diErrInjConfig->injDescWrAddr = RF_DI_REGS__RT_DI_INJECT0__DI_INJ_DESC_WR_ADDR__READ(reg);
	diErrInjConfig->injDescRdAddr = RF_DI_REGS__RT_DI_INJECT0__DI_INJ_DESC_RD_ADDR__READ(reg);
	diErrInjConfig->injCtxAddr = RF_DI_REGS__RT_DI_INJECT0__DI_INJ_CTX_ADDR__READ(reg);
	diErrInjConfig->injCtxValue = RF_DI_REGS__RT_DI_INJECT0__DI_INJ_CTX_VALUE__READ(reg);

	reg = HPNFC_GetReg((uintptr_t)&(hpnfc->regs->di_config.di_inject1));
	diErrInjConfig->injDataParityRd = RF_DI_REGS__RT_DI_INJECT1__DI_INJ_DATA_RD_PAR__READ(reg);
	diErrInjConfig->injDataWrCrc = RF_DI_REGS__RT_DI_INJECT1__DI_INJ_DATA_WR_CRC__READ(reg);
	diErrInjConfig->injDataWrAddr = RF_DI_REGS__RT_DI_INJECT1__DI_INJ_DATA_WR_ADDR__READ(reg);
	diErrInjConfig->injDataRdAddr = RF_DI_REGS__RT_DI_INJECT1__DI_INJ_DATA_RD_ADDR__READ(reg);
	diErrInjConfig->injRegVal = RF_DI_REGS__RT_DI_INJECT1__DI_INJ_REG_VALUE__READ(reg);

	reg = HPNFC_GetReg((uintptr_t)&(hpnfc->regs->di_config.di_inject2));
	diErrInjConfig->injRmpVal = RF_DI_REGS__RT_DI_INJECT2__DI_INJ_RMP_VALUE__READ(reg);
	diErrInjConfig->injRmpAddr = RF_DI_REGS__RT_DI_INJECT2__DI_INJ_RMP_ADDR__READ(reg);

	return EOK;
}

int32_t HPNFC_Core_SetDmaConfig(HPNFC_State *hpnfc, HPNFC_DmaConfig* dmaConfig)
{
	uint32_t reg;

	if (hpnfc->ctrlInfo.ctrlRev < 6)
		LogAndExit(ENOTSUP);

	reg = HPNFC_GetReg((uintptr_t)&(hpnfc->regs->ctrl_config.fifo_trigg_level));
	RF_CTRL_CONFIG__RT_FIFO_TRIGG_LEVEL__FIFO_TRIGG_LVL__MODIFY(reg, dmaConfig->trigLvlInWords);
	RF_CTRL_CONFIG__RT_FIFO_TRIGG_LEVEL__DMA_PACKAGE_SIZE__MODIFY(reg, dmaConfig->dmaPackSizeInWords);
	HPNFC_SetReg((uintptr_t)&(hpnfc->regs->ctrl_config.fifo_trigg_level), reg);

	return EOK;
}


int32_t HPNFC_Core_GetDmaConfig(HPNFC_State *hpnfc, HPNFC_DmaConfig* dmaConfig)
{
	uint32_t reg;

	if (hpnfc->ctrlInfo.ctrlRev < 6)
		LogAndExit(ENOTSUP);

	reg = HPNFC_GetReg((uintptr_t)&(hpnfc->regs->ctrl_config.fifo_trigg_level));
	dmaConfig->trigLvlInWords = RF_CTRL_CONFIG__RT_FIFO_TRIGG_LEVEL__FIFO_TRIGG_LVL__READ(reg);
	dmaConfig->dmaPackSizeInWords = RF_CTRL_CONFIG__RT_FIFO_TRIGG_LEVEL__DMA_PACKAGE_SIZE__READ(reg);

	return EOK;
}

static int32_t WaitForRemapReady(HPNFC_State *hpnfc)
{
	return (TimeoutWaitReg((uintptr_t)&(hpnfc->regs->ctrl_config.remap_access),
						  RF_CTRL_CONFIG__RT_REMAP_ACCESS__REC_ACCESS__MASK,
						  RF_CTRL_CONFIG__RT_REMAP_ACCESS__REC_ACCESS__MASK, 0, 10000));
}

static int32_t RemapInitialCheck(HPNFC_State *hpnfc)
{
	int32_t status;

	if (hpnfc->ctrlInfo.remap == 0)
		return ENOTSUP;

	status = WaitForIdle(hpnfc);
	if (status)
		LogAndExit(status);

	status = WaitForRemapReady(hpnfc);
	if (status)
		LogAndExit(status);

	return EOK;
}

static int32_t RemapAccess(HPNFC_State *hpnfc, uint32_t accessType, uint8_t memTrg)
{
	int32_t status;
	uint32_t reg;

	reg = HPNFC_GetReg((uintptr_t)&(hpnfc->regs->ctrl_config.remap_access));
	RF_CTRL_CONFIG__RT_REMAP_ACCESS__REC_TRG__MODIFY(reg, memTrg);
	RF_CTRL_CONFIG__RT_REMAP_ACCESS__REC_ACTYPE__MODIFY(reg, accessType);
	RF_CTRL_CONFIG__RT_REMAP_ACCESS__REC_ACCESS__MODIFY(reg, 1);
	HPNFC_SetReg((uintptr_t)&(hpnfc->regs->ctrl_config.remap_access), reg);

	status = WaitForRemapReady(hpnfc);
	if (status)
		LogAndExit(status);

	reg = HPNFC_GetReg((uintptr_t)&(hpnfc->regs->ctrl_config.remap_access));
	if (RF_CTRL_CONFIG__RT_REMAP_ACCESS__REC_DI_ERR__READ(reg)){
		vDbgMsg(DBG_GEN_MSG, DBG_CRIT, "%s", "Remap data integrity appears\n");
		LogAndExit(EIO);
	}

	return EOK;
}

int32_t HPNFC_Core_WriteRemapRecord(HPNFC_State *hpnfc,
									HPNFC_RemapRecord* remapRecord, uint8_t memTrg)
{
	uint32_t reg;
	int32_t status = RemapInitialCheck(hpnfc);
	if (status)
		LogAndExit(status);

	if (remapRecord->logicAddr > MAX_ROW_ADDRESS)
		LogAndExit(EINVAL);

	if (remapRecord->physAddr > MAX_ROW_ADDRESS)
		LogAndExit(EINVAL);

	reg = HPNFC_GetReg((uintptr_t)&(hpnfc->regs->ctrl_config.remap_log_addr));
	RF_CTRL_CONFIG__RT_REMAP_LOG_ADDR__REMAP_LOG_ADDR__MODIFY(reg, remapRecord->logicAddr);
	HPNFC_SetReg((uintptr_t)&(hpnfc->regs->ctrl_config.remap_log_addr), reg);

	reg = HPNFC_GetReg((uintptr_t)&(hpnfc->regs->ctrl_config.remap_phys_addr));
	RF_CTRL_CONFIG__RT_REMAP_PHYS_ADDR__REMAP_PHYS_ADDR__MODIFY(reg, remapRecord->physAddr);
	HPNFC_SetReg((uintptr_t)&(hpnfc->regs->ctrl_config.remap_phys_addr), reg);

	/* execute adding one record */
	return RemapAccess(hpnfc, HPNFC_REMAP_ACCESS_TYPE_WRITE, memTrg);
}

int32_t HPNFC_Core_ReadRemapRecord(HPNFC_State *hpnfc, uint16_t recordIdx,
								   HPNFC_RemapRecord* remapRecord, uint8_t memTrg)
{
	uint32_t reg;
	int32_t status = RemapInitialCheck(hpnfc);
	if (status)
		LogAndExit(status);

	reg = HPNFC_GetReg((uintptr_t)&(hpnfc->regs->ctrl_config.remap_access));
	RF_CTRL_CONFIG__RT_REMAP_ACCESS__REC_RD_IDX__MODIFY(reg, recordIdx);
	HPNFC_SetReg((uintptr_t)&(hpnfc->regs->ctrl_config.remap_access), reg);

	status = RemapAccess(hpnfc, HPNFC_REMAP_ACCESS_TYPE_READ, memTrg);
	if (status)
		LogAndExit(status);

	reg = HPNFC_GetReg((uintptr_t)&(hpnfc->regs->ctrl_config.remap_log_addr));
	remapRecord->logicAddr = RF_CTRL_CONFIG__RT_REMAP_LOG_ADDR__REMAP_LOG_ADDR__READ(reg);

	reg = HPNFC_GetReg((uintptr_t)&(hpnfc->regs->ctrl_config.remap_phys_addr));
	remapRecord->physAddr = RF_CTRL_CONFIG__RT_REMAP_PHYS_ADDR__REMAP_PHYS_ADDR__READ(reg);

	return EOK;
}

int32_t HPNFC_Core_ClearRemapTable(HPNFC_State *hpnfc)
{
	int32_t status = RemapInitialCheck(hpnfc);
	if (status)
		LogAndExit(status);

	return RemapAccess(hpnfc, HPNFC_REMAP_ACCESS_TYPE_CLEAR_ALL, 0);
}

int32_t HPNFC_Core_GetRemapRecordCount(HPNFC_State *hpnfc, uint16_t* recordCount)
{
	uint32_t reg;

	int32_t status = RemapInitialCheck(hpnfc);
	if (status)
		LogAndExit(status);

	reg = HPNFC_GetReg((uintptr_t)&(hpnfc->regs->ctrl_config.remap_ctrl));

	*recordCount = RF_CTRL_CONFIG__RT_REMAP_CTRL__REC_CNT__READ(reg);
	return EOK;
}

int32_t HPNFC_Core_SetRemapMask(HPNFC_State *hpnfc, uint32_t remapMask)
{
	uint32_t reg;
	int32_t status = RemapInitialCheck(hpnfc);
	if (status)
		LogAndExit(status);

	if (remapMask > MAX_ROW_ADDRESS)
		LogAndExit(EINVAL);

	reg = HPNFC_GetReg((uintptr_t)&(hpnfc->regs->ctrl_config.remap_mask));
	RF_CTRL_CONFIG__RT_REMAP_MASK__REMAP_MASK__MODIFY(reg, remapMask);
	HPNFC_SetReg((uintptr_t)&(hpnfc->regs->ctrl_config.remap_mask), reg);

	return EOK;
}

int32_t HPNFC_Core_GetRemapMask(HPNFC_State *hpnfc, uint32_t* remapMask)
{
	uint32_t reg;
	int32_t status = RemapInitialCheck(hpnfc);
	if (status)
		LogAndExit(status);

	reg = HPNFC_GetReg((uintptr_t)&(hpnfc->regs->ctrl_config.remap_mask));
	*remapMask = RF_CTRL_CONFIG__RT_REMAP_MASK__REMAP_MASK__READ(reg);

	return EOK;
}

int32_t HPNFC_Core_SetRemapEnable(HPNFC_State *hpnfc, uint8_t remapEnable)
{
	uint32_t reg;
	int32_t status = RemapInitialCheck(hpnfc);
	if (status)
		LogAndExit(status);

	if (remapEnable > 1)
		LogAndExit(EINVAL);

	reg = HPNFC_GetReg((uintptr_t)&(hpnfc->regs->ctrl_config.remap_ctrl));
	RF_CTRL_CONFIG__RT_REMAP_CTRL__RMP_EN__MODIFY(reg, remapEnable);
	HPNFC_SetReg((uintptr_t)&(hpnfc->regs->ctrl_config.remap_ctrl), reg);

	return EOK;
}

int32_t HPNFC_Core_GetRemapEnable(HPNFC_State *hpnfc, uint8_t* remapEnable)
{
	uint32_t reg;
	int32_t status = RemapInitialCheck(hpnfc);
	if (status)
		LogAndExit(status);

	reg = HPNFC_GetReg((uintptr_t)&(hpnfc->regs->ctrl_config.remap_ctrl));
	*remapEnable = RF_CTRL_CONFIG__RT_REMAP_CTRL__RMP_EN__READ(reg);

	return EOK;
}

int32_t HPNFC_Core_SetPSlcConfig(HPNFC_State *hpnfc, HPNFC_PSlcConfig* pSlcConfig)
{
	uint32_t reg;

	if (hpnfc->ctrlInfo.ctrlRev < 8)
		LogAndExit(ENOTSUP);

	if ((hpnfc->ctrlInfo.ctrlRev < 9) && (pSlcConfig->pSlcPrefixEn == 1))
		LogAndExit(ENOTSUP);

	if (pSlcConfig->pSlcPrefixEn > 1)
		LogAndExit(EINVAL);

	if (pSlcConfig->blkAddrIdx > 0xF)
		LogAndExit(EINVAL);

	reg = HPNFC_GetReg((uintptr_t)&(hpnfc->regs->ctrl_config.device_ctrl));
	RF_CTRL_CONFIG__RT_DEVICE_CTRL__PSLC_PREFIX_EN__MODIFY(reg, pSlcConfig->pSlcPrefixEn);
	HPNFC_SetReg((uintptr_t)&(hpnfc->regs->ctrl_config.device_ctrl), reg);

	reg = HPNFC_GetReg((uintptr_t)&(hpnfc->regs->ctrl_config.nf_dev_layout));
	RF_CTRL_CONFIG__RT_NF_DEV_LAYOUT__BLK_ADDR_IDX__MODIFY(reg, pSlcConfig->blkAddrIdx);
	HPNFC_SetReg((uintptr_t)&(hpnfc->regs->ctrl_config.nf_dev_layout), reg);

	return EOK;

}

int32_t HPNFC_Core_GetPSlcConfig(HPNFC_State *hpnfc, HPNFC_PSlcConfig* pSlcConfig)
{
	uint32_t reg;

	if (hpnfc->ctrlInfo.ctrlRev < 8)
		LogAndExit(ENOTSUP);

	reg = HPNFC_GetReg((uintptr_t)&(hpnfc->regs->ctrl_config.device_ctrl));
	pSlcConfig->pSlcPrefixEn = RF_CTRL_CONFIG__RT_DEVICE_CTRL__PSLC_PREFIX_EN__READ(reg);

	reg = HPNFC_GetReg((uintptr_t)&(hpnfc->regs->ctrl_config.nf_dev_layout));
	pSlcConfig->blkAddrIdx = RF_CTRL_CONFIG__RT_NF_DEV_LAYOUT__BLK_ADDR_IDX__READ(reg);

	return EOK;
}

int32_t HPNFC_Core_SetZqCalConfig(HPNFC_State *hpnfc, HPNFC_ZqCalConfig* zqCalConfig)
{
	uint32_t reg;

	reg = HPNFC_GetReg((uintptr_t)&(hpnfc->regs->ctrl_config.zq_cal_config));

	switch (zqCalConfig->targetPerPackage){
	case HPNFC_TARGET_PER_PACKAGE_ONE:
		RF_CTRL_CONFIG__RT_ZQ_CAL_CONFIG__TRG_PER_PKG__MODIFY(reg, 0);
		break;
	case HPNFC_TARGET_PER_PACKAGE_TWO:
		RF_CTRL_CONFIG__RT_ZQ_CAL_CONFIG__TRG_PER_PKG__MODIFY(reg, 1);
		break;
	case HPNFC_TARGET_PER_PACKAGE_FOUR:
		RF_CTRL_CONFIG__RT_ZQ_CAL_CONFIG__TRG_PER_PKG__MODIFY(reg, 2);
		break;
	case HPNFC_TARGET_PER_PACKAGE_EIGHT:
		RF_CTRL_CONFIG__RT_ZQ_CAL_CONFIG__TRG_PER_PKG__MODIFY(reg, 3);
		break;
	default:
		LogAndExit(EINVAL);
	}

	if (zqCalConfig->longCalEnable > 1)
		LogAndExit(EINVAL);
	if (zqCalConfig->shortCalEnable > 1)
		LogAndExit(EINVAL);
	if (zqCalConfig->toggImplLcal > 1)
		LogAndExit(EINVAL);

	if (zqCalConfig->tZqcs > 0x1FF)
		LogAndExit(EINVAL);

	if (zqCalConfig->tZqcl > 0x1FF)
		LogAndExit(EINVAL);


	RF_CTRL_CONFIG__RT_ZQ_CAL_CONFIG__TZQCL__MODIFY(reg, zqCalConfig->tZqcl);
	RF_CTRL_CONFIG__RT_ZQ_CAL_CONFIG__TZQCS__MODIFY(reg, zqCalConfig->tZqcs);
	RF_CTRL_CONFIG__RT_ZQ_CAL_CONFIG__LONG_CAL_EN__MODIFY(reg, zqCalConfig->longCalEnable);
	RF_CTRL_CONFIG__RT_ZQ_CAL_CONFIG__SHORT_CAL_EN__MODIFY(reg, zqCalConfig->shortCalEnable);
	RF_CTRL_CONFIG__RT_ZQ_CAL_CONFIG__TOGG_IMPL_LCAL__MODIFY(reg, zqCalConfig->toggImplLcal);
	HPNFC_SetReg((uintptr_t)&(hpnfc->regs->ctrl_config.zq_cal_config), reg);

	HPNFC_SetReg((uintptr_t)&(hpnfc->regs->ctrl_config.long_cal_period), zqCalConfig->longCalPeriod);
	HPNFC_SetReg((uintptr_t)&(hpnfc->regs->ctrl_config.short_cal_period), zqCalConfig->shortCalPeriod);

	return EOK;
}

int32_t HPNFC_Core_GetZqCalConfig(HPNFC_State *hpnfc, HPNFC_ZqCalConfig* zqCalConfig)
{
	uint32_t reg;

	reg = HPNFC_GetReg((uintptr_t)&(hpnfc->regs->ctrl_config.zq_cal_config));

	switch (RF_CTRL_CONFIG__RT_ZQ_CAL_CONFIG__TRG_PER_PKG__READ(reg)){
	case 0:
		zqCalConfig->targetPerPackage = HPNFC_TARGET_PER_PACKAGE_ONE;
		break;
	case 1:
		zqCalConfig->targetPerPackage = HPNFC_TARGET_PER_PACKAGE_TWO;
		break;
	case 2:
		zqCalConfig->targetPerPackage = HPNFC_TARGET_PER_PACKAGE_FOUR;
		break;
	case 3:
		zqCalConfig->targetPerPackage = HPNFC_TARGET_PER_PACKAGE_EIGHT;
		break;
	default:
		LogAndExit(EINVAL);
	}

	zqCalConfig->tZqcl = RF_CTRL_CONFIG__RT_ZQ_CAL_CONFIG__TZQCL__READ(reg);
	zqCalConfig->tZqcs = RF_CTRL_CONFIG__RT_ZQ_CAL_CONFIG__TZQCS__READ(reg);
	zqCalConfig->longCalEnable = RF_CTRL_CONFIG__RT_ZQ_CAL_CONFIG__LONG_CAL_EN__READ(reg);
	zqCalConfig->shortCalEnable = RF_CTRL_CONFIG__RT_ZQ_CAL_CONFIG__SHORT_CAL_EN__READ(reg);
	zqCalConfig->toggImplLcal = RF_CTRL_CONFIG__RT_ZQ_CAL_CONFIG__TOGG_IMPL_LCAL__READ(reg);

	zqCalConfig->longCalPeriod = HPNFC_GetReg((uintptr_t)&(hpnfc->regs->ctrl_config.long_cal_period));
	zqCalConfig->shortCalPeriod = HPNFC_GetReg((uintptr_t)&(hpnfc->regs->ctrl_config.short_cal_period));

	return EOK;
}

int32_t HPNFC_Core_SetZqCalEnable(HPNFC_State *hpnfc, uint8_t target, uint8_t enable)
{
	uint32_t reg;

	reg = HPNFC_GetReg((uintptr_t)&(hpnfc->regs->ctrl_config.zq_cal_en));

	if (target > (RF_CTRL_CONFIG__RT_ZQ_CAL_EN__CAL_EN__WIDTH - 1))
		LogAndExit(EINVAL);

	if (enable > 1)
		LogAndExit(EINVAL);

	if (enable == 0)
		reg &= ~(1uL << target);
	else
		reg |= (1uL << target);

	HPNFC_SetReg((uintptr_t)&(hpnfc->regs->ctrl_config.zq_cal_en), reg);

	return EOK;
}

int32_t HPNFC_Core_GetZqCalEnable(HPNFC_State *hpnfc, uint8_t target, uint8_t* enable)
{
	uint32_t reg;

	reg = HPNFC_GetReg((uintptr_t)&(hpnfc->regs->ctrl_config.zq_cal_en));

	if (target > (RF_CTRL_CONFIG__RT_ZQ_CAL_EN__CAL_EN__WIDTH - 1))
		LogAndExit(EINVAL);

	if (reg & (1UL << target))
		*enable = 1;
	else
		*enable = 0;

	return EOK;
}


int32_t HPNFC_Core_GetZqStat(HPNFC_State *hpnfc, uint16_t* calStat)
{
	uint32_t reg;

	reg = HPNFC_GetReg((uintptr_t)&(hpnfc->regs->cmd_status.zq_cal_stat));

	*calStat = RF_CONTROLLER_REGS__RT_ZQ_CAL_STAT__CAL_STAT__READ(reg);

	return EOK;
}

int32_t HPNFC_Core_ClearZqStat(HPNFC_State *hpnfc, uint16_t calStatToClear)
{
	uint32_t reg;

	reg = HPNFC_GetReg((uintptr_t)&(hpnfc->regs->cmd_status.zq_cal_stat));

	RF_CONTROLLER_REGS__RT_ZQ_CAL_STAT__CAL_STAT__MODIFY(reg, calStatToClear);

	HPNFC_SetReg((uintptr_t)&(hpnfc->regs->cmd_status.zq_cal_stat), reg);

	return EOK;
}


int32_t HPNFC_Core_SetTimeoutConfig(HPNFC_State *hpnfc, HPNFC_TimeoutConfig* timeoutConfig)
{
	uint32_t reg;

	HPNFC_SetReg((uintptr_t)&(hpnfc->regs->ctrl_config.time_out), timeoutConfig->timeout);

	reg = HPNFC_GetReg((uintptr_t)&(hpnfc->regs->ctrl_config.device_ctrl));
	RF_CTRL_CONFIG__RT_DEVICE_CTRL__TIME_OUT_EN__MODIFY(reg, timeoutConfig->timeoutEn);
	HPNFC_SetReg((uintptr_t)&(hpnfc->regs->ctrl_config.device_ctrl), reg);

	return EOK;
}

int32_t HPNFC_Core_GetTimeoutConfig(HPNFC_State *hpnfc, HPNFC_TimeoutConfig* timeoutConfig)
{
	uint32_t reg;

	timeoutConfig->timeout = HPNFC_GetReg((uintptr_t)&(hpnfc->regs->ctrl_config.time_out));

	reg = HPNFC_GetReg((uintptr_t)&(hpnfc->regs->ctrl_config.device_ctrl));
	timeoutConfig->timeoutEn = RF_CTRL_CONFIG__RT_DEVICE_CTRL__TIME_OUT_EN__READ(reg);

	return EOK;
}

int32_t HPNFC_Core_SetCePinReduction(HPNFC_State *hpnfc, uint8_t enable)
{
	uint32_t reg;

	if (enable > 1)
		LogAndExit(EINVAL);

	reg = HPNFC_GetReg((uintptr_t)&(hpnfc->regs->ctrl_config.device_ctrl));

	RF_CTRL_CONFIG__RT_DEVICE_CTRL__CE_PIN_REDUCT__MODIFY(reg, enable);

	HPNFC_SetReg((uintptr_t)&(hpnfc->regs->ctrl_config.device_ctrl), reg);

	return EOK;
}

int32_t HPNFC_Core_GetCePinReduction(HPNFC_State *hpnfc, uint8_t *enable)
{
	uint32_t reg;

	reg = HPNFC_GetReg((uintptr_t)&(hpnfc->regs->ctrl_config.device_ctrl));

	*enable = RF_CTRL_CONFIG__RT_DEVICE_CTRL__CE_PIN_REDUCT__READ(reg);

	return EOK;
}
