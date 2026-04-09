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
 * tests.c
 *
 * HPNFC driver tests and reference code.
 *****************************************************************************/


#include <string.h>
#include <stdlib.h>
#include <hpnfc_if.h>
#include <hpnfc_obj_if.h>
#include <cdn_errno.h>

#include <tests.h>
#include <request.h>
#include <wrapper.h>
#include <pattern.h>
#include <cps.h>
#include <bm_nfc.h>
#include <mmio.h>
#include <timer.h>
#include <assert.h>


uint32_t wBuffer[(2048 + 128) / 4]  __attribute__((aligned(8))); // 2K page
uint32_t rBuffer[(2048 + 128) / 4]  __attribute__((aligned(8))); // 2K page

nand_base_vector_t *vec;

#define ROUND_UP_DIV(a, b) (((a) + (b)-1) / (b))
#define PTR_INC(base, offset) (void *)((uint8_t *)(base) + (offset))

volatile uint32_t *gSdmaBase;
int dbg = 1;

#define DESC_COUNT  8
//static void print_device_layout(void);
static void enable_access_ddr(uint32_t enable);
HPNFC_DeviceLayout deviceLayout;

uint32_t sectorSize = 0;
uint32_t eccCorrCap = 0;

uint8_t bytesPerDmaAccess;
/* handler function called when slave dma trigger occurs */
void slaveDmaHandler(void* privateData, void* data,
                     HPNFC_TransferDirection transferDirection,
                     uint32_t sdmaSize,
                     uint32_t *transferredData)
{
    volatile uint32_t *d32 = data;
    int i;

    printf("%s sdmaSize:%d, transferredData 0x%p\n", __func__, sdmaSize, transferredData);

    /* read data from flash*/
    if (transferDirection == HPNFC_DATA_TRANSFER_READ){
        //printf("%s: read data from flash. RAM buf %p, size %d\n", __func__, data, sdmaSize);
        for (i = 0; i < sdmaSize / bytesPerDmaAccess; i++){
            *d32 = *gSdmaBase;
            //printf("gSdmabase 0x%p, read %d, addr %p\n", gSdmaBase, *d32, data);
            d32++;
            *transferredData += bytesPerDmaAccess;
        }
    }
    else{
        //printf("%s: write data to flash %d\n", __func__, sdmaSize);
        for (i = 0; i < sdmaSize / bytesPerDmaAccess; i++){
            *gSdmaBase = *d32;
            //printf("gSdmabase 0x%p, write %d, addr %p\n", gSdmaBase, *d32, data);
            d32++;
            *transferredData += bytesPerDmaAccess;
        }
    }

}

/* simple CDMA Erase, Program Read testing function */
int Cdma_Err_Desc_Test(void *privateData)
{
    int status;
    uint8_t pageCnt = 1;
    uint32_t dataSize = deviceLayout.mainSize * pageCnt;

    if (!dataSize)
        return 1;

    Fillbuf(wBuffer, dataSize / 4, 0);
    CPS_CacheFlush((uintptr_t)wBuffer, dataSize, 0);
    Clearbuf(rBuffer, dataSize / 4, 0xDEADC0DE);
    CPS_CacheFlush((uintptr_t)rBuffer, dataSize, 0);

    //int CdmaExecEraseBlock(void *privateData, uint8_t bank, uint32_t rowAddr, uint32_t blkCnt)
    status = CdmaExecEraseBlock(privateData, 9, 0, 1); // set the wrong bank number, more than 8
    if (status){
        return status;
    }

    return 0;
}

static int SetUpSecSizeAndCorrCap(uint32_t secSize, uint32_t corrCap)
{
    sectorSize = secSize;
    eccCorrCap = corrCap;

    return 0;
}

static int SetUpEccConfig(void *privateData)
{
    HPNFC_OBJ *drv = HPNFC_GetInstance();
    HPNFC_EccConfiguration eccConfig;
    HPNFC_TransferConfig transCfg;
    uint8_t pageCnt = 1;
    uint32_t dataSize = deviceLayout.mainSize * pageCnt;
    int status;

    printf("[NFC_TEST] %s()\n", __func__);

    if (!dataSize)
        return 1;

    memset(&eccConfig, 0, sizeof(eccConfig));
    memset(&transCfg, 0, sizeof(transCfg));

    transCfg.lastSectorSize = sectorSize;
    transCfg.sectorSize = sectorSize;
    transCfg.sectorCount = deviceLayout.mainSize / sectorSize;
    transCfg.offset = 0;

    printf("==================================\n");
    printf("transCfg.lastSectorSize %d\n",transCfg.lastSectorSize);
    printf("transCfg.sectorSize %d\n",transCfg.sectorSize);
    printf("transCfg.sectorCount %d\n",transCfg.sectorCount);
    printf("==================================\n");
    status = drv->setTransferConfig(privateData, &transCfg);
    if (status){
        printf("transfer configuration failed\n");
        return status;
    }

    if (eccCorrCap == 0){
        printf("ECC is not supported\n");
        return ENOTSUP;
    }

    eccConfig.eccEnabled = 1;
    eccConfig.scramblerEnabled = 0;
    eccConfig.corrCap = eccCorrCap;
    printf("eccConfig.eccEnabled %d\n",eccConfig.eccEnabled);
    printf("eccConfig.corrCap %d\n",eccConfig.corrCap);
    printf("eccConfig.scramblerEnabled %d\n",eccConfig.scramblerEnabled);

    status = drv->setEccConfig(privateData, &eccConfig);
    if (status){
        printf("ECC configuration failed\n");
        return status;
    }

    return status;
}

///* check program and read with ECC enabled
// * also check read after erase to check
// * if ECC fatal error occurs */
int EccCdmaTest(void *privateData)
{
    int status;
    //HPNFC_CdmaCmd cdmaCmd;
    //HPNFC_CmdReq cmdReq;
    uint8_t pageCnt = 1;
    uint32_t dataSize = deviceLayout.mainSize * pageCnt;

    printf("[NFC_TEST] %s()\n", __func__);

    SetUpEccConfig(privateData);

    status = CdmaExecEraseBlock(privateData, 0, 0, 1);
    if (status){
        return status;
    }

    status = CdmaExecProgramPage(privateData, 0, 0, 1, wBuffer, 0);
    if (status){
        return status;
    }

    status = CdmaExecReadPage(privateData, 0, 0, 1, rBuffer, 0);
    if (status){
        return status;
    }

    CPS_CacheInvalidate((uintptr_t)rBuffer, dataSize, 0);
    if (Comparebuf(wBuffer, rBuffer, dataSize / 4)){
        printf("==================================\n");
        PrintCmpBuf(wBuffer, rBuffer, dataSize / 4);
        printf("%s written and read data differs\n", __func__);
        printf("==================================\n");
        assert(0);
        return 1;
    }
    else
    {
        printf("==================================\n");
        printf("%s written and read data the same!!\n", __func__);
        printf("==================================\n");
    }



    return 0;
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

static int scan_factory_defect(void *privateData)
{
    int status;
    HPNFC_OBJ *drv = HPNFC_GetInstance();

    /* set transfer configuration to read whole page with spare area*/
    HPNFC_EccConfiguration eccConfig;
    HPNFC_TransferConfig transCfg;

    memset(&eccConfig, 0, sizeof(eccConfig));
    memset(&transCfg, 0, sizeof(transCfg));

    /* disable ECC to avoid error correction (read data and ECC codes)  */
    eccConfig.eccEnabled = 0;
    eccConfig.scramblerEnabled = 0;
    eccConfig.corrCap = eccCorrCap;
    status = drv->setEccConfig(privateData, &eccConfig);
    if (status){
        printf("ECC configuration failed\n");
    }

    transCfg.lastSectorSize = deviceLayout.mainSize + deviceLayout.spareSize;    // fix me ! , buffer overflow , main size + spare > 2K
    transCfg.sectorCount = 1;
    transCfg.offset = 0;
    status = drv->setTransferConfig(privateData, &transCfg);
    if (status){
        printf("transfer configuration failed\n");
    return 0;
        //return status;
    }

    int blk_shift = GetBlockShift(deviceLayout.pagesPerBlock);
    printf("blk shift %d\n", blk_shift);

    for (uint32_t i = 0; i < deviceLayout.blocksPerLun; i++) {
        uint32_t row_addr = i << blk_shift;
        printf("erase row addr 0x%x ", row_addr);
#if 1
        status = CdmaExecEraseBlock(privateData, 0, row_addr, 1);
        if (status){
            printf("erase error status %d, ", status);
        }
#endif
        printf("scan block %d \n", i);
//      (void *privateData, bank, rowAddr, pageCnt,  *buffer, thread)
        status = CdmaExecReadPage(privateData, 0, i << blk_shift, 1, rBuffer, 0);

        if (status){
            printf("read error status %d, ", status);
        }

        uint8_t *mark = (uint8_t *)PTR_INC(rBuffer, deviceLayout.mainSize);

        if (*mark != 0xff)
        {
            printf("scan block %d, ", i);
            printf("bad ");
            printf(" mark : 0x%x\n", *mark);
            //SET_MASK_BIT(buf, bit);
        }
    }

    return 0;
}

static int kernel_test(void *privateData)
{
    int status;
    HPNFC_OBJ *drv = HPNFC_GetInstance();

    /* set transfer configuration to read whole page with spare area*/
    HPNFC_EccConfiguration eccConfig;
    HPNFC_TransferConfig transCfg;

    memset(&eccConfig, 0, sizeof(eccConfig));
    memset(&transCfg, 0, sizeof(transCfg));

    /* disable ECC to avoid error correction (read data and ECC codes)  */
    eccConfig.eccEnabled = 0;
    eccConfig.scramblerEnabled = 0;
    eccConfig.corrCap = eccCorrCap;
    status = drv->setEccConfig(privateData, &eccConfig);
    if (status){
        printf("ECC configuration failed\n");
    }

    transCfg.lastSectorSize = deviceLayout.mainSize + deviceLayout.spareSize;    // fix me ! , buffer overflow , main size + spare > 2K
    transCfg.sectorCount = 1;
    transCfg.offset = 0;
    status = drv->setTransferConfig(privateData, &transCfg);
    if (status){
        printf("transfer configuration failed\n");
    return 0;
        //return status;
    }

    int blk_shift = GetBlockShift(deviceLayout.pagesPerBlock);
    printf("blk shift %d\n", blk_shift);

    for (uint32_t i = 0; i < deviceLayout.blocksPerLun; i++) {
        uint32_t row_addr = i << blk_shift;
        printf("erase row addr 0x%x ", row_addr);
#if 1
        status = CdmaExecEraseBlock(privateData, 0, row_addr, 1);
        if (status){
            printf("erase error status %d, ", status);
        }
#endif
        printf("scan block %d \n", i);
//      (void *privateData, bank, rowAddr, pageCnt,  *buffer, thread)
        status = CdmaExecReadPage(privateData, 0, i << blk_shift, 1, rBuffer, 0);

        if (status){
            printf("read error status %d, ", status);
        }

        uint8_t *mark = (uint8_t *)PTR_INC(rBuffer, deviceLayout.mainSize);

        if (*mark != 0xff)
        {
            printf("scan block %d, ", i);
            printf("bad ");
            printf(" mark : 0x%x\n", *mark);
            //SET_MASK_BIT(buf, bit);
        }
    }

    return 0;
}

int ecc_cdma_all_nand_test(void *privateData)
{
    int status;
    uint8_t pageCnt = 1;
    uint32_t dataSize = deviceLayout.mainSize * pageCnt;
    uint32_t nr_blk = deviceLayout.blocksPerLun;
    uint32_t nr_page = deviceLayout.pagesPerBlock;
    uint32_t row_addr;

    printf("[NFC_TEST] %s()\n", __func__);

    SetUpEccConfig(privateData);

    for (uint32_t i = 0 ; i < nr_blk; i = i + 1)
    // for (uint32_t i = 0 ; i < nr_blk; i++)
    //for (uint32_t i = 0; i < 10; i++)
    {
        row_addr = i << deviceLayout.blockShift;
        printf("erase blk %d, row_addr 0x%x\n", i, row_addr);
                                //(privateData, bank, rowAddr, blkCnt)
        status = CdmaExecEraseBlock(privateData, 0, row_addr, 1);
        if (status){
            printf("erase error %d \n", status);
            //return status;
        }

        for (uint32_t j = 0; j < nr_page; j++)
        {
            row_addr = i << deviceLayout.blockShift | j;
            //(privateData, bank, rowAddr, pageCnt, buffer, thread)
            //printf("write blk %d, row_addr 0x%x\n", i, row_addr);

            uint32_t start_val = (i + j)*(j*j);
            Fillbuf(wBuffer, dataSize / 4, start_val);
            CPS_CacheFlush((uintptr_t)wBuffer, dataSize, 0);

            status = CdmaExecProgramPage(privateData, 0, row_addr, 1, wBuffer, 0);
            if (status){
                printf("write error %d \n", status);
                //return status;
            }
        }

        for (uint32_t j = 0; j < nr_page; j++)
        {
            row_addr = i << deviceLayout.blockShift | j;
            //(privateData, bank, rowAddr, pageCnt, buffer, thread)
            //printf("read blk %d, row_addr 0x%x\n", i, row_addr);
            Clearbuf(rBuffer, dataSize / 4, 0xDEADC0DE);
            CPS_CacheFlush((uintptr_t)rBuffer, dataSize, 0);
            status = CdmaExecReadPage(privateData, 0, row_addr, 1, rBuffer, 0);
            if (status){
                printf("read error %d \n", status);
            }

            CPS_CacheInvalidate((uintptr_t)rBuffer, dataSize, 0);
            uint32_t start_val = (i + j)*(j*j);
            Fillbuf(wBuffer, dataSize / 4, start_val);
            CPS_CacheFlush((uintptr_t)wBuffer, dataSize, 0);
            //PrintCmpBuf(wBuffer, rBuffer, 4);
            if (Comparebuf(wBuffer, rBuffer, dataSize / 4)){
                printf("==================================\n");
                PrintCmpBuf(wBuffer, rBuffer, dataSize / 4);
                printf("%s written and read data differs\n", __func__);
                printf("==================================\n");
                return 1;
            }
            else {
                //printf("==================================\n");
                //printf("%s written and read data the same!!\n", __func__);
                //printf("==================================\n");
            }
        }
    }

    return 0;
}

int NandBootWriteBl1Bin(void *privateData)
{
    int status;
    HPNFC_CdmaCmd cdmaCmd;
    HPNFC_CmdReq cmdReq;
    uint8_t pageCnt = 1;
    uint32_t dataSize = deviceLayout.mainSize * pageCnt;

    SetUpEccConfig(privateData);

    status = CdmaExecEraseBlock(privateData, 0, 0, 1);
    if (status){
        return status;
    }

    status = CdmaExecProgramPage(privateData, 0, 0, 1, wBuffer, 0);
    if (status){
        return status;
    }

    status = CdmaExecReadPage(privateData, 0, 0, 1, rBuffer, 0);
    if (status){
        return status;
    }

    CPS_CacheInvalidate((uintptr_t)rBuffer, dataSize, 0);
    if (Comparebuf(wBuffer, rBuffer, dataSize / 4)){
        PrintCmpBuf(wBuffer, rBuffer, dataSize / 4);
        printf("%s written and read data differs\n", __func__);
        return 1;
    }
    else
    {
        printf("%s written and read data the same!!\n", __func__);
    }

    memset(&cdmaCmd, 0, sizeof(HPNFC_CdmaCmd));
    cdmaCmd.bank = 0;
    cdmaCmd.rowAddr = 0;
    cdmaCmd.cmdCount = 1;
    cdmaCmd.cmdType = HPNFC_CDMA_CMD_TYPE_ERASE;

    status = CdmaExecCmdReq(privateData, &cdmaCmd, &cmdReq, 0);
    if (status){
        printf("%s erase operation failed\n", __func__);
        return status;
    }

    /* read erased page to cause uncerrectable error */
    status = CdmaExecReadPage(privateData, 0, 0, 1, rBuffer, 0);
    if (status != HPNFC_STATUS_ECC_UNCORR_ERR){
        printf("%s expected status was %d, but status is %d\n", __func__,
               HPNFC_STATUS_ECC_UNCORR_ERR, status);
        return 1;
    }

    return 0;
}
//
///* test correction error injection */
int EccCdmaTest2(void *privateData)
{
    int status;
    uint32_t dataSize = deviceLayout.mainSize;
    HPNFC_OBJ *drv = HPNFC_GetInstance();
    HPNFC_EccConfiguration eccConfig;
    HPNFC_TransferConfig transCfg;

    if (!dataSize)
        return 1;

    memset(&eccConfig, 0, sizeof(eccConfig));
    memset(&transCfg, 0, sizeof(transCfg));

    transCfg.lastSectorSize = sectorSize;
    transCfg.sectorSize = sectorSize;
    transCfg.sectorCount = deviceLayout.mainSize / sectorSize;
    transCfg.offset = 0;
    status = drv->setTransferConfig(privateData, &transCfg);
    if (status){
        printf("tarnsfer configuration failed\n");
        return status;
    }

    if (eccCorrCap == 0){
        printf("ECC is not supported\n");
        return ENOTSUP;
    }

    eccConfig.eccEnabled = 1;
    eccConfig.scramblerEnabled = 0;
    eccConfig.corrCap = eccCorrCap;
    status = drv->setEccConfig(privateData, &eccConfig);
    if (status){
        printf("ECC configuration failed\n");
        return status;
    }

    Fillbuf(wBuffer, dataSize / 4, 5);
    CPS_CacheFlush((uintptr_t)wBuffer, dataSize, 0);
    Clearbuf(rBuffer, dataSize / 4, 0xDEADC0DE);
    CPS_CacheFlush((uintptr_t)rBuffer, dataSize, 0);

    status = CdmaExecEraseBlock(privateData, 0, 0, 1);
    if (status){
        return status;
    }

    status = CdmaExecProgramPage(privateData, 0, 0, 1, wBuffer, 0);
    if (status){
        return status;
    }

    /* set transfer configuration to read whole page with spare area*/
    transCfg.lastSectorSize = deviceLayout.mainSize + deviceLayout.spareSize;    // fix me ! , buffer overflow , main size + spare > 2K
    transCfg.sectorCount = 1;
    transCfg.offset = 0;
    status = drv->setTransferConfig(privateData, &transCfg);
    if (status){
        printf("transfer configuration failed\n");
        return status;
    }

    /* disable ECC to avoid error correction (read data and ECC codes)  */
    eccConfig.eccEnabled = 0;
    status = drv->setEccConfig(privateData, &eccConfig);
    if (status){
        printf("ECC configuration failed\n");
        return status;
    }

    status = CdmaExecReadPage(privateData, 0, 0, 1, rBuffer, 0);
    if (status){
        return status;
    }

    /* change two bits */
    if (rBuffer[0] & 0x40)
        rBuffer[0] &= ~0x40;
    else
        rBuffer[0] |= 0x40;

    if (rBuffer[0] & 0x02)
        rBuffer[0] &= ~0x02;
    else
        rBuffer[0] |= 0x02;

    CPS_CacheFlush((uintptr_t)rBuffer, dataSize, 0);

    status = CdmaExecEraseBlock(privateData, 0, 0, 1);
    if (status){
        return status;
    }

    /* program data with old ECC codes and changed two bits */
    status = CdmaExecProgramPage(privateData, 0, 0, 1, rBuffer, 0);
    if (status){
        return status;
    }

    transCfg.lastSectorSize = sectorSize;
    transCfg.sectorSize = sectorSize;
    transCfg.sectorCount = deviceLayout.mainSize / sectorSize;
    transCfg.offset = 0;
    status = drv->setTransferConfig(privateData, &transCfg);
    if (status){
        printf("transfer configuration failed\n");
        return status;
    }

    eccConfig.eccEnabled = 1;
    eccConfig.scramblerEnabled = 0;
    eccConfig.corrCap = eccCorrCap;
    status = drv->setEccConfig(privateData, &eccConfig);
    if (status){
        printf("ECC configuration failed\n");
        return status;
    }

    Clearbuf(rBuffer, dataSize / 4, 0xDEADC0DE);
    CPS_CacheFlush((uintptr_t)rBuffer, dataSize, 0);

    /* read page with modified two bits to cause correctable error */
    status = CdmaExecReadPage(privateData, 0, 0, 1, rBuffer, 0);
    if (status != HPNFC_STATUS_ECC_CORR_ERR){
        printf("%s expected status was %d, but status is %d\n", __func__,
               HPNFC_STATUS_ECC_CORR_ERR, status);
        return 1;
    }

    CPS_CacheInvalidate((uintptr_t)rBuffer, dataSize, 0);
    if (Comparebuf(wBuffer, rBuffer, dataSize / 4)){
        PrintCmpBuf(wBuffer, rBuffer, 128);
        printf("%s written and read data differs\n", __func__);
        return 1;
    }

    return 0;
}
//
//
//
///* test data transfer in synchronous mode */
int work_mode_test(void *privateData)
{
    HPNFC_PhyConfig sdrPhyCfg;
    //HPNFC_PhyConfig sdrPhyCfg, sdrPhyCfg;
    int status;
    HPNFC_CdmaCmd cdmaCmd;
    //HPNFC_CmdReq cmdReq;
    uint8_t pageCnt = 1;
    uint32_t dataSize = deviceLayout.mainSize * pageCnt;
    HPNFC_OBJ *drv = HPNFC_GetInstance();

    if (!dataSize)
        return 1;

    // org setting
    Fillbuf(wBuffer, dataSize / 4, 0);
    CPS_CacheFlush((uintptr_t)wBuffer, dataSize, 0);
    Clearbuf(rBuffer, dataSize / 4, 0xDEADC0DE);
    CPS_CacheFlush((uintptr_t)rBuffer, dataSize, 0);

    memset(&cdmaCmd, 0, sizeof(HPNFC_CdmaCmd));
    while (dbg) {}

    printf("====setting 1\n");

    status = CdmaExecEraseBlock(privateData, 0, 0, 1);
    if (status){
        return status;
    }

    status = CdmaExecProgramPage(privateData, 0, 0, 1, wBuffer, 0);
    if (status){
        return status;
    }

    status = CdmaExecReadPage(privateData, 0, 0, 1, rBuffer, 0);
    if (status){
        return status;
    }

    CPS_CacheInvalidate((uintptr_t)rBuffer, dataSize, 0);
    if (Comparebuf(wBuffer, rBuffer, dataSize / 4)){
        PrintCmpBuf(wBuffer, rBuffer, 128);
        printf("%s written and read data differs\n", __func__);
        return 1;
    }

    /////////////////////////////////////////////////////////////////////////////////

    printf("====setting 2\n");
    // sdr mode 3 setting
    sdrPhyCfg.phyCtrl = 0;
    sdrPhyCfg.phyTsel = 0;
    sdrPhyCfg.phyDqTiming = 2;
    sdrPhyCfg.phyDqsTiming = 4;
    sdrPhyCfg.phyGateLpbkCtrl = 0x00200000;
    sdrPhyCfg.phyDllMasterCtrl = 0x01140004;
    sdrPhyCfg.phyDllSlaveCtrl = 0x1F1F;
    sdrPhyCfg.toggleTimings0 = 0x3F3F3F3F;
    sdrPhyCfg.toggleTimings1 = 0x3F3F;
    sdrPhyCfg.asyncToggleTimings = 0x3F3F;
    sdrPhyCfg.timings0 = 0xFFFFFFFF;
    sdrPhyCfg.timings1 = 0xFFFFFFFF;
    sdrPhyCfg.timings2 = 0x3F3F;
    sdrPhyCfg.dllPhyCtrl = 0x0030300;

    //status = drv->setWorkMode(privateData, HPNFC_WORK_MODE_SDR, 0, &sdrPhyCfg);
    if (status){
        printf("setting work mode failed\n");
        return status;
    }

    Fillbuf(wBuffer, dataSize / 4, 0);
    CPS_CacheFlush((uintptr_t)wBuffer, dataSize, 0);
    Clearbuf(rBuffer, dataSize / 4, 0xDEADC0DE);
    CPS_CacheFlush((uintptr_t)rBuffer, dataSize, 0);

    memset(&cdmaCmd, 0, sizeof(HPNFC_CdmaCmd));

    status = CdmaExecEraseBlock(privateData, 0, 0, 1);
    if (status){
        return status;
    }

    status = CdmaExecProgramPage(privateData, 0, 0, 1, wBuffer, 0);
    if (status){
        return status;
    }

    status = CdmaExecReadPage(privateData, 0, 0, 1, rBuffer, 0);
    if (status){
        return status;
    }

    CPS_CacheInvalidate((uintptr_t)rBuffer, dataSize, 0);
    if (Comparebuf(wBuffer, rBuffer, dataSize / 4)){
        PrintCmpBuf(wBuffer, rBuffer, 128);
        printf("%s written and read data differs\n", __func__);
        return 1;
    }

    ////////////////////////////////////////////////////////////////////
    printf("====setting 3\n");
    // sdr mode 0 setting
    sdrPhyCfg.phyCtrl = 0;
    sdrPhyCfg.phyTsel = 0;
    sdrPhyCfg.phyDqTiming = 2;
    sdrPhyCfg.phyDqsTiming = 4;
    sdrPhyCfg.phyGateLpbkCtrl = 0x00200000;
    sdrPhyCfg.phyDllMasterCtrl = 0x01140004;
    sdrPhyCfg.phyDllSlaveCtrl = 0x1F1F;
    sdrPhyCfg.toggleTimings0 = 0x1F1F1F1F;// 0x3F3F3F3F;
    sdrPhyCfg.toggleTimings1 = 0x1F1F;
    sdrPhyCfg.asyncToggleTimings = 0x1F1F;
    sdrPhyCfg.timings0 = 0x7F7F7F7F;
    sdrPhyCfg.timings1 = 0x7F7F7F7F;
    sdrPhyCfg.timings2 = 0x1F1F;
    sdrPhyCfg.dllPhyCtrl = 0x0030300;

    status = drv->setWorkMode(privateData, HPNFC_WORK_MODE_SDR, 0, &sdrPhyCfg);
    if (status){
        printf("setting work mode failed\n");
        return status;
    }

    Fillbuf(wBuffer, dataSize / 4, 0);
    CPS_CacheFlush((uintptr_t)wBuffer, dataSize, 0);
    Clearbuf(rBuffer, dataSize / 4, 0xDEADC0DE);
    CPS_CacheFlush((uintptr_t)rBuffer, dataSize, 0);

    memset(&cdmaCmd, 0, sizeof(HPNFC_CdmaCmd));

    status = CdmaExecEraseBlock(privateData, 0, 0, 1);
    if (status){
        return status;
    }

    status = CdmaExecProgramPage(privateData, 0, 0, 1, wBuffer, 0);
    if (status){
        return status;
    }

    status = CdmaExecReadPage(privateData, 0, 0, 1, rBuffer, 0);
    if (status){
        return status;
    }

    CPS_CacheInvalidate((uintptr_t)rBuffer, dataSize, 0);
    if (Comparebuf(wBuffer, rBuffer, dataSize / 4)){
        PrintCmpBuf(wBuffer, rBuffer, 128);
        printf("%s written and read data differs\n", __func__);
        return 1;
    }

    ////////////////////////////////////////////////////////////////////
    printf("====setting 4\n");
    // sdr mode 3 new value setting
    sdrPhyCfg.phyCtrl = 0;
    sdrPhyCfg.phyTsel = 0;
    sdrPhyCfg.phyDqTiming = 2;
    sdrPhyCfg.phyDqsTiming = 4;
    sdrPhyCfg.phyGateLpbkCtrl = 0x00200000;
    sdrPhyCfg.phyDllMasterCtrl = 0x01140004;
    sdrPhyCfg.phyDllSlaveCtrl = 0x1F1F;
    sdrPhyCfg.toggleTimings0 = 0x1F1F1F1F;// 0x3F3F3F3F;
    sdrPhyCfg.toggleTimings1 = 0x1F1F;
    sdrPhyCfg.asyncToggleTimings = 0x1F1F;
    sdrPhyCfg.timings0 = 0x7F7F7F7F;
    sdrPhyCfg.timings1 = 0x7F7F7F7F;
    sdrPhyCfg.timings2 = 0x1F1F;
    sdrPhyCfg.dllPhyCtrl = 0x0030300;

    status = drv->setWorkMode(privateData, HPNFC_WORK_MODE_SDR, 3, &sdrPhyCfg);
    if (status){
        printf("setting work mode failed\n");
        return status;
    }

    Fillbuf(wBuffer, dataSize / 4, 0);
    CPS_CacheFlush((uintptr_t)wBuffer, dataSize, 0);
    Clearbuf(rBuffer, dataSize / 4, 0xDEADC0DE);
    CPS_CacheFlush((uintptr_t)rBuffer, dataSize, 0);

    memset(&cdmaCmd, 0, sizeof(HPNFC_CdmaCmd));

    status = CdmaExecEraseBlock(privateData, 0, 0, 1);
    if (status){
        return status;
    }

    status = CdmaExecProgramPage(privateData, 0, 0, 1, wBuffer, 0);
    if (status){
        return status;
    }

    status = CdmaExecReadPage(privateData, 0, 0, 1, rBuffer, 0);
    if (status){
        return status;
    }

    CPS_CacheInvalidate((uintptr_t)rBuffer, dataSize, 0);
    if (Comparebuf(wBuffer, rBuffer, dataSize / 4)){
        PrintCmpBuf(wBuffer, rBuffer, 128);
        printf("%s written and read data differs\n", __func__);
        return 1;
    }

    return 0;
}
//
///* execute operations in the oposite order
// * read, program, erase instead of erase program read
// * and order it properly by sync functionality*/
//int CdmaSyncTest(void *privateData)
//{
//    int status;
//    HPNFC_CdmaCmd cdmaCmd;
//    HPNFC_CmdReq cmdReq1, cmdReq2, cmdReq3;
//    uint8_t pageCnt = 1;
//    uint32_t dataSize = deviceLayout.mainSize * pageCnt;
//    HPNFC_OBJ *drv = HPNFC_GetInstance();
//    HPNFC_CdmaSyncConfig syncConfig;
//
//    volatile uint64_t syncVal = 1;
//
//    CPS_CacheFlush((uintptr_t)&syncVal, sizeof(syncVal), 0);
//
//    if (!dataSize)
//        return 1;
//
//    Fillbuf(wBuffer, dataSize / 4, 22);
//    CPS_CacheFlush((uintptr_t)wBuffer, dataSize, 0);
//    Clearbuf(rBuffer, dataSize / 4, 0xDEADC0DE);
//    CPS_CacheFlush((uintptr_t)rBuffer, dataSize, 0);
//
//    memset(&cdmaCmd, 0, sizeof(HPNFC_CdmaCmd));
//
//
//    syncConfig.startCondition = HPNFC_CDMA_SYNC_CFG_SC_EQUAL;
//    syncConfig.endOperation = HPNFC_CDMA_SYNC_CFG_EO_INCREMENT;
//    syncConfig.syncBuffAddr = (uintptr_t)&syncVal;
//    syncConfig.syncEnable = 1;
//
//    cdmaCmd.bank = 0;
//    cdmaCmd.rowAddr = 0;
//    cdmaCmd.cmdCount = 1;
//    cdmaCmd.cmdType = HPNFC_CDMA_CMD_TYPE_READ;
//    cdmaCmd.dmaMode = HPNFC_DMA_MODE_MASTER;
//    cdmaCmd.memPtr = rBuffer;
//    // start sync value set to 3
//    syncConfig.startValue = 3;
//    cdmaCmd.syncConfig = syncConfig;
//    CdmaPrepareCmdReq(privateData, &cdmaCmd, &cmdReq1, 0);
//    status = drv->execCmdReq(privateData, &cmdReq1);
//    if (status)
//        return status;
//
//    memset(&cdmaCmd, 0, sizeof(HPNFC_CdmaCmd));
//
//    cdmaCmd.bank = 0;
//    cdmaCmd.rowAddr = 0;
//    cdmaCmd.cmdCount = 1;
//    cdmaCmd.cmdType = HPNFC_CDMA_CMD_TYPE_PROGRAM;
//    cdmaCmd.dmaMode = HPNFC_DMA_MODE_MASTER;
//    cdmaCmd.memPtr = wBuffer;
//    // start sync value set to 2
//    syncConfig.startValue = 2;
//    cdmaCmd.syncConfig = syncConfig;
//    CdmaPrepareCmdReq(privateData, &cdmaCmd, &cmdReq2, 1);
//    status = drv->execCmdReq(privateData, &cmdReq2);
//    if (status)
//        return status;
//
//    memset(&cdmaCmd, 0, sizeof(HPNFC_CdmaCmd));
//
//    cdmaCmd.bank = 0;
//    cdmaCmd.rowAddr = 0;
//    cdmaCmd.cmdCount = 1;
//    cdmaCmd.cmdType = HPNFC_CDMA_CMD_TYPE_ERASE;
//    // start sync value set to 1
//    syncConfig.startValue = 1;
//    cdmaCmd.syncConfig = syncConfig;
//    CdmaPrepareCmdReq(privateData, &cdmaCmd, &cmdReq3, 2);
//    status = drv->execCmdReq(privateData, &cmdReq3);
//    if (status)
//        return status;
//
//    status = ReqWaitForFinish(privateData, &cmdReq3);
//    if (status)
//        return status;
//
//    status = ReqWaitForFinish(privateData, &cmdReq2);
//    if (status)
//        return status;
//
//    status = ReqWaitForFinish(privateData, &cmdReq1);
//    if (status)
//        return status;
//
//    if (cmdReq1.status){
//        printf("%s read operation failed\n", __func__);
//        return 1;
//    }
//
//    if (cmdReq2.status){
//        printf("%s program operation failed\n", __func__);
//        return 1;
//    }
//
//    if (cmdReq3.status){
//        printf("%s erase operation failed\n", __func__);
//        return 1;
//    }
//
//    CPS_CacheInvalidate((uintptr_t)rBuffer, dataSize, 0);
//    if (Comparebuf(wBuffer, rBuffer, dataSize / 4)){
//        printf("%s written and read data differs\n", __func__);
//        PrintCmpBuf(wBuffer, rBuffer, 128);
//        return 1;
//    }
//
//    return 0;
//}

typedef struct BootHeader {
    uint64_t hostAddress;
    uint16_t fileSize;
}BootHeader;

/* test boot functionality */
int CdmaBootTest(void *privateData)
{
    int status;
    HPNFC_CdmaCmd cdmaCmd;
    uint8_t pageCnt = 1;
    uint32_t dataSize = deviceLayout.mainSize * pageCnt;
    BootHeader *bootHeader = (BootHeader*)&wBuffer[0];
    uint32_t bl1_img_sz_kb = 60;

    if (!dataSize)
        return 1;

    memset(&cdmaCmd, 0, sizeof(HPNFC_CdmaCmd));

    status = CdmaExecEraseBlock(privateData, 0, 0, 1);
    if (status){
        return status;
    }

    bootHeader->hostAddress = 0x08000000; /* move code to TPU SRAM*/
    bootHeader->fileSize = bl1_img_sz_kb*1024 / sectorSize;
    CPS_CacheFlush((uintptr_t)wBuffer, dataSize, 0);

    printf("rBuffer 0x%p\n", rBuffer);
    printf("wBuffer 0x%p\n", wBuffer);
    printf("bootHeader->hostAddress 0x%lx\n", bootHeader->hostAddress);
    printf("bootHeader->fileSize 0x%x\n", bootHeader->fileSize);
    SetUpEccConfig(privateData);

    status = CdmaExecProgramPage(privateData, 0, 0, 1, wBuffer, 0);
    //(privateData, bank, rowAddr, count, *buffer, HPNFC_DmaMode dmaMode)
    //status = PioExecProgramPage(privateData, 0, 0, 1, wBuffer, HPNFC_DMA_MODE_MASTER);
    if (status){
        return status;
    }

    static int dbg = 1;
    Fillbuf(wBuffer, dataSize / 4, 88);
    Clearbuf(rBuffer, dataSize / 4, 0xDEADC0DE);
    printf("Please load bl1.bin to RAM\n");
    while (dbg) {}

    uint32_t offset = 0;
    uint32_t blk_id = 0;
    uint32_t row_addr = 0;
    uint32_t prg_pg = bl1_img_sz_kb * 1024 / deviceLayout.mainSize;

    for (int i = 1; i < prg_pg + 2; i++)
    {
        row_addr = blk_id << deviceLayout.blockShift | i;
#if 0
        enable_access_ddr(1);
        status = PioExecProgramPage(privateData, 0, row_addr, 1, (void *)(0x100000000 + offset), HPNFC_DMA_MODE_MASTER);
        enable_access_ddr(0);
#else
        printf("PioExecProgramPage pg %d, rowaddr 0x%x\n", i, row_addr);
        // status = PioExecProgramPage(privateData, 0, row_addr, 1, (void *)(0x08000000+ offset), HPNFC_DMA_MODE_MASTER);
        status = PioExecProgramPage(privateData, 0, row_addr, 1, PTR_INC(0x08000000, offset), HPNFC_DMA_MODE_MASTER);
#endif
        if (status){
            return status;
        }
        offset += deviceLayout.mainSize;
    }

    return status;

}

static int flush_fip_bin(void *privateData, uintptr_t *buffer, uint32_t len, nand_base_vector_t *vec)
{
    void *src_buf_addr = 0;
    uint32_t total_prg_pg_cnt = ROUND_UP_DIV(len, deviceLayout.mainSize);
    uint32_t block_size = deviceLayout.pagesPerBlock * deviceLayout.mainSize;
    int status;

    // erase block first
    for (uint32_t i = 0; i < NAND_BASE_DATA_BACKUP_COPY; i++)
    {
        for (uint32_t j = 0; j < NAND_FIP_DATA_BLOCK_COUNT; j++)
        {
            uint32_t blk_id = vec->fip_bin_blk[i][j];
            uint32_t row_addr = blk_id << deviceLayout.blockShift;

            printf("flush_fip_bin, erase blk_id %d, row_addr 0x%x\n", blk_id, row_addr);
#if 1
            //(void *privateData, uint8_t bank, uint32_t rowAddr, uint32_t blkCnt)
            status = CdmaExecEraseBlock(privateData, 0, row_addr, 1);
#else
            //(privateData, bank, rowAddr, count)
            //status = PioExecEraseBlock(privateData, 0, row_addr, 1);
#endif
            if (status) {
                printf("erase error\n");
                return status;
            }
        }
    }

    // write fip.bin
    for (uint32_t i = 0; i < NAND_BASE_DATA_BACKUP_COPY; i++)
    {
        for (uint32_t pg_idx = 0; pg_idx < total_prg_pg_cnt; pg_idx++)
        {
            uint32_t offset = pg_idx * deviceLayout.mainSize;
            uint32_t blk_idx = offset / block_size;
            uint32_t blk_id = vec->fip_bin_blk[i][blk_idx];

            src_buf_addr = PTR_INC(buffer, pg_idx * deviceLayout.mainSize);

            uint32_t status;
            uint32_t row_addr = blk_id << deviceLayout.blockShift | (pg_idx % deviceLayout.pagesPerBlock);

            //memset(rBuffer, 0xFFFFFFFF, deviceLayout.mainSize);

            if ((pg_idx % 30) == 0) printf("flush_fip_bin, page idx %d, blk_id %d, row_addr 0x%x, src buf addr %p\n", pg_idx, blk_id, row_addr, src_buf_addr);

            #if 0
                                    //(privateData, bank, rowAddr, count, *buffer, dmaMode)
            status = PioExecProgramPage(privateData, 0, row_addr, 1, src_buf_addr, HPNFC_DMA_MODE_MASTER);
            #else
                                    //(privateData, bank, rowAddr, pageCnt, buffer, thread)
            status = CdmaExecProgramPage(privateData, 0, row_addr, 1, src_buf_addr, 0);
            #endif

            if (status)
            {
                printf("pio write fail\n");
                return status;
            }
        }
    }

    return 0;
}

#define DESC_CONV(x)	((((((x[0]<<8)|x[1])<<8)|x[2])<<8)|x[3])
#define NAND_VECTOR_SIGNATURE DESC_CONV("NANV")

static int flush_vec(void *privateData, uint32_t block, nand_base_vector_t *vec)
{
    int status;

    vec->signature = NAND_VECTOR_SIGNATURE;
    vec->version = 0x1;
    vec->erase_count = 0;
    vec->rsvd_block_count = 24;
    vec->nand_vector_blk[0] = 0;
    vec->nand_vector_blk[1] = 1;
    vec->nand_vector_length = deviceLayout.mainSize;

    vec->nand_base_block_usage[0] = 0xff;
    vec->nand_base_block_usage[1] = 0xff;
    vec->nand_base_block_usage[2] = 0xff;

    vec->fip_bin_blk[0][0] = 2;
    vec->fip_bin_blk[0][1] = 3;
    vec->fip_bin_blk[0][2] = 4;
    vec->fip_bin_blk[0][3] = 5;
    vec->fip_bin_blk[1][0] = 6;
    vec->fip_bin_blk[1][1] = 7;
    vec->fip_bin_blk[1][2] = 8;
    vec->fip_bin_blk[1][3] = 9;

//   (void *privateData, uint8_t bank, uint32_t rowAddr, uint32_t blkCnt)
    status = CdmaExecEraseBlock(privateData, 0, 0, 1);
    if (status) {
        return status;
    }

    //(void *privateData, bank, rowAddr, pageCnt, buffer, thread)
    status = CdmaExecProgramPage(privateData, 0, 0, 1, wBuffer, 0);
    if (status){
        return status;
    }

    return 0;
}

static void enable_access_ddr(uint32_t enable)
{
    uint32_t tmp = mmio_read_32(0x50010064);
    printf("org 0x50010064 0x%x\n", tmp);

    tmp &= 0xFFFFFEFF;

    if (enable)
    {
        tmp |= 0x1 << 8;
        mmio_write_32(0x50010064, tmp);
    }
    else
    {
        mmio_write_32(0x50010064, tmp);
    }
    printf("new 0x50010064 0x%x\n", tmp);

}

/* test boot functionality */
int prg_vec_and_fip(void *privateData)
{
    int status;

    //1. scan defect
    //scan_factory_defect(privateData);
    //while (dbg) {}

    SetUpEccConfig(privateData);

    memset(wBuffer, 0, deviceLayout.mainSize);
    vec = (nand_base_vector_t *)&wBuffer[0];
    // 2. program vec in block 0
    enable_access_ddr(1);
    status = flush_vec(privateData, 0, vec);

    // 3. program fip.bin
    //while (dbg) {}
#if 1
    //memset((void *)0x04000000, 0xa8, 128 * 1024);
    status = flush_fip_bin(privateData, (uintptr_t *)0x04000000, 1024 * 128, vec);
    //memset((void *)0x100000000, 0xa8, 256 * 1024);

#else
    status = flush_fip_bin(privateData, (uintptr_t *)0x100000000, 1024 * 512, vec);
    enable_access_ddr(0);
#endif

    return status;

    /* execute reset with boot enable, wait for boot complete */
    //ResetControllerBootEn();

    //drv->getBootStatus(privateData, &bootStatus, &cpyId);
    //CPS_CacheInvalidate((uintptr_t)rBuffer, dataSize, 0);
    //if (Comparebuf(wBuffer, rBuffer, dataSize / 4)){
    //    printf("%s written and read data differs\n", __func__);
    //    PrintCmpBuf(wBuffer, rBuffer, 128);
    //    return 1;
    //}

    /* reset controller to exit from boot */
    // fixme
    //ResetController();

    //return 0;
}
//
////int GenericTest(void *privateData)
////{
////    int status;
////    //uint8_t pageCnt = 1;
////    uint32_t dataSize = 16;
////    HPNFC_OBJ *drv = HPNFC_GetInstance();
////    HPNFC_ControllerInfo controllerInfo;
////
////    if (!dataSize)
////        return 1;
////
////    Fillbuf(wBuffer, dataSize / 4, 8);
////    Clearbuf(rBuffer, dataSize / 4, 0xDEADC0DE);
////
////    status = GenExecReadID(privateData, 0, rBuffer, 8);
////    if (status){
////        return status;
////    }
////
////    printf("ID %x\n", rBuffer[0]);
////
////    status = GenExecEraseBlock(privateData, 0, 0);
////    if (status){
////        return status;
////    }
////
////    drv->getControllerInfo(privateData, &controllerInfo);
////    if (controllerInfo.dmaData64bit){
////        status = GenExecProgramPageSplit(privateData, 0, 0, wBuffer, dataSize);
////        if (status){
////            return status;
////        }
////
////        status = GenExecReadPageSplit(privateData, 0, 0, rBuffer, dataSize);
////        if (status){
////            return status;
////        }
////    }
////    else {
////        status = GenExecProgramPage(privateData, 0, 0, wBuffer, dataSize);
////        if (status){
////            return status;
////        }
////
////        status = GenExecReadPage(privateData, 0, 0, rBuffer, dataSize);
////        if (status){
////            return status;
////        }
////    }
////
////    if (Comparebuf(wBuffer, rBuffer, dataSize / 4)){
////        printf("%s written and read data differs\n", __func__);
////        PrintCmpBuf(wBuffer, rBuffer, dataSize / 4);
////        return 1;
////    }
////
////    return 0;
////}
//
///* simple CDMA Erase, Program Read testing function */
static int PioTest(void *privateData, HPNFC_DmaMode dmaMode, uint32_t block)
{
    int status;
    uint8_t pageCnt = 1;
    uint32_t dataSize = deviceLayout.mainSize * pageCnt;
    printf("[NFC_TEST] %s() : block %d \n", __func__, block);

    if (!dataSize)
        return 1;

    Fillbuf(wBuffer, dataSize / 4, 0);
    CPS_CacheFlush((uintptr_t)wBuffer, dataSize, 0);
    Clearbuf(rBuffer, dataSize / 4, 0xDEADC0DE);
    CPS_CacheFlush((uintptr_t)rBuffer, dataSize, 0);

    status = PioExecEraseBlock(privateData, 0, 0, 1);
    if (status){
        return status;
    }

    status = PioExecProgramPage(privateData, 0, 0, pageCnt, wBuffer,
                                dmaMode);
    if (status){
        return status;
    }

    status = PioExecReadPage(privateData, 0, 0, pageCnt, rBuffer,
                             dmaMode);
    if (status){
        return status;
    }

    CPS_CacheInvalidate((uintptr_t)rBuffer, dataSize, 0);
    if (Comparebuf(wBuffer, rBuffer, dataSize / 4)){
        printf("%s written and read data differs\n", __func__);
        PrintCmpBuf(wBuffer, rBuffer, dataSize / 4);
        return 1;
    }
    else
    {
        printf("==================================\n");
        printf("%s written and read data the same!!\n", __func__);
        printf("==================================\n");
    }

    return 0;
}
//
//int PioMasterDmaTest(void *privateData)
//{
//    return PioTest(privateData, HPNFC_DMA_MODE_MASTER);
//}
//

void print_device_layout(void)
{
    printf("device layout main size 0x%x\n", deviceLayout.mainSize);
    printf("device layout spareSize 0x%x\n", deviceLayout.spareSize);
    printf("device layout lunNumber 0x%x\n", deviceLayout.lunNumber);
    printf("device layout pagesPerBlock 0x%x\n", deviceLayout.pagesPerBlock);
    printf("device layout blocksPerLun 0x%x\n", deviceLayout.blocksPerLun);
    printf("device layout  numberOfPlanes 0x%x\n", deviceLayout.numberOfPlanes);
}

/* driver and controller init function */
void* InitDriver(uintptr_t regsBase, uintptr_t sdmaBase)
{
    uint32_t status;
    HPNFC_SysReq req;
    HPNFC_Config config;
    HPNFC_OBJ *drv = HPNFC_GetInstance();
    void *privateData;
    HPNFC_BchConfigInfo bchCfgInfo;
    HPNFC_ControllerInfo controllerInfo;

    gSdmaBase = (volatile uint32_t*)sdmaBase;

    memset(&req, 0, sizeof(HPNFC_SysReq));

    status = drv->probe(regsBase, &req);
    if (status){
        printf("probe failed\n");
        return NULL;
    }
    printf("Probe succeed\n");

    memset(&config, 0, sizeof(config));
    config.regBase = regsBase;
    config.slaveDmaHandler =  slaveDmaHandler;
    config.devNum = 1;

    bchCfgInfo.corrCaps[0] = 2;
    bchCfgInfo.corrCaps[1] = 4;
    bchCfgInfo.corrCaps[2] = 8;
    bchCfgInfo.corrCaps[3] = 12;
    bchCfgInfo.corrCaps[4] = 16;
    bchCfgInfo.corrCaps[5] = 24;
    bchCfgInfo.corrCaps[6] = 40;
    bchCfgInfo.numberOfCorrCaps = 7;
    bchCfgInfo.sectorSizes[0] = 512;
    bchCfgInfo.sectorSizes[1] = 1024;
    /*
     * this bchConfig info will be used only
     * if driver is not able to read BCH configuration from HW registers
     * it is ignored otherwise
     */
    config.bchCfgInfo = &bchCfgInfo;

    privateData = malloc(req.pDataSize);

    status = drv->init(privateData, &config);
    if (status){
        printf("Driver initialzation failed\n");
        free(privateData);
        return NULL;
    }

    drv->getDeviceLayout(privateData, &deviceLayout);

    // print_device_layout();

    drv->getBchConfigInfo(privateData, &bchCfgInfo);

    if (bchCfgInfo.sectorSizes[1]){
        sectorSize = bchCfgInfo.sectorSizes[1];
    }
    else if (bchCfgInfo.sectorSizes[0]){
        sectorSize = bchCfgInfo.sectorSizes[0];
    }
    else
        sectorSize = 1024;

    /* use one global ecc correction ability for all ECC tests
     * the smallest one */
    if (bchCfgInfo.numberOfCorrCaps > 0){
        eccCorrCap = bchCfgInfo.corrCaps[0];
    }

    drv->getControllerInfo(privateData, &controllerInfo);
    if (controllerInfo.dmaData64bit)
        bytesPerDmaAccess = 8;
    else
        bytesPerDmaAccess = 4;

    drv->start(privateData);

    return privateData;
}
//
//int CdmaCmdParseTest(void *privateData)
//{
//    HPNFC_CdmaCmd cdmaCmd1, cdmaCmd2;
//    HPNFC_CdmaSyncConfig syncConfig;
//    HPNFC_OBJ *drv = HPNFC_GetInstance();
//    volatile uint64_t syncVal = 1;
//
//    memset(&cdmaCmd1, 0, sizeof(HPNFC_CdmaCmd));
//    memset(&syncConfig, 0, sizeof(syncConfig));
//
//    syncConfig.startCondition = HPNFC_CDMA_SYNC_CFG_SC_EQUAL;
//    syncConfig.endOperation = HPNFC_CDMA_SYNC_CFG_EO_INCREMENT;
//    syncConfig.syncBuffAddr = (uintptr_t)&syncVal;
//    syncConfig.syncEnable = 1;
//
//    cdmaCmd1.bank = 0;
//    cdmaCmd1.rowAddr = 0;
//    cdmaCmd1.cmdCount = 1;
//    cdmaCmd1.cmdType = HPNFC_CDMA_CMD_TYPE_READ;
//    cdmaCmd1.dmaMode = HPNFC_DMA_MODE_MASTER;
//    cdmaCmd1.memPtr = rBuffer;
//    // start sync value set to 3
//    syncConfig.startValue = 3;
//    cdmaCmd1.syncConfig = syncConfig;
//
//    drv->assembleCdmaDesc(privateData, &cdmaDesc[0][0], &cdmaCmd1);
//    drv->cdmaParseDesc(privateData, &cdmaDesc[0][0], &cdmaCmd2);
//
//    return memcmp(&cdmaCmd1, &cdmaCmd2, sizeof(cdmaCmd1));
//}
//
//
//int CdmaTestChain(void *privateData)
//{
//    int status;
//    HPNFC_CdmaCmd cdmaCmd;
//    HPNFC_CmdReq cmdReq;
//    uint8_t pageCnt = 1;
//    uint32_t dataSize = deviceLayout.mainSize * pageCnt;
//    HPNFC_OBJ *drv = HPNFC_GetInstance();
//
//    if (!dataSize)
//        return 1;
//
//    Fillbuf(wBuffer, dataSize / 4, 9);
//    CPS_CacheFlush((uintptr_t)wBuffer, dataSize, 0);
//    Clearbuf(rBuffer, dataSize / 4, 0xDEADC0DE);
//    CPS_CacheFlush((uintptr_t)rBuffer, dataSize, 0);
//
//    memset(&cdmaCmd, 0, sizeof(HPNFC_CdmaCmd));
//
//    cdmaCmd.bank = 0;
//    cdmaCmd.rowAddr = 0;
//    cdmaCmd.cmdCount = 1;
//    cdmaCmd.cmdType = HPNFC_CDMA_CMD_TYPE_READ;
//    cdmaCmd.dmaMode = HPNFC_DMA_MODE_MASTER;
//    cdmaCmd.memPtr = rBuffer;
//
//    drv->assembleCdmaDesc(privateData, &cdmaDesc[0][0], &cdmaCmd);
//    CPS_CacheFlush((uintptr_t)&cdmaDesc[0][0], HPNFC_CDMA_DESC_SIZE, 0);
//
//    memset(&cdmaCmd, 0, sizeof(HPNFC_CdmaCmd));
//
//    cdmaCmd.bank = 0;
//    cdmaCmd.rowAddr = 0;
//    cdmaCmd.cmdCount = 1;
//    cdmaCmd.cmdType = HPNFC_CDMA_CMD_TYPE_PROGRAM;
//    cdmaCmd.dmaMode = HPNFC_DMA_MODE_MASTER;
//    cdmaCmd.memPtr = wBuffer;
//    cdmaCmd.nextPtr = (uint64_t)(uintptr_t)&cdmaDesc[0][0];
//    drv->assembleCdmaDesc(privateData, &cdmaDesc[1][0], &cdmaCmd);
//    CPS_CacheFlush((uintptr_t)&cdmaDesc[1][0], HPNFC_CDMA_DESC_SIZE, 0);
//
//    memset(&cdmaCmd, 0, sizeof(HPNFC_CdmaCmd));
//
//    cdmaCmd.bank = 0;
//    cdmaCmd.rowAddr = 0;
//    cdmaCmd.cmdCount = 1;
//    cdmaCmd.cmdType = HPNFC_CDMA_CMD_TYPE_ERASE;
//    cdmaCmd.nextPtr = (uint64_t)(uintptr_t)&cdmaDesc[1][0];
//    status = CdmaExecCmdReq(privateData, &cdmaCmd, &cmdReq, 0);
//    if (status){
//        printf("%s chain CDMA operation failed \n", __func__);
//        return status;
//    }
//
//    status = ReqWaitForFinish(privateData, &cmdReq);
//    if (status)
//        return status;
//
//    if (cmdReq.status){
//        printf("%s chain CDMA operation failed \n", __func__);
//        return 1;
//    }
//
//    CPS_CacheInvalidate((uintptr_t)rBuffer, dataSize, 0);
//    if (Comparebuf(wBuffer, rBuffer, dataSize / 4)){
//        printf("%s written and read data differs\n", __func__);
//        PrintCmpBuf(wBuffer, rBuffer, 128);
//        return 1;
//    }
//
//    return 0;
//}
//
//int CdmaChainSlaveDmaTest(void *privateData)
//{
//    int status;
//    HPNFC_CdmaCmd cdmaCmd;
//    HPNFC_CmdReq cmdReq;
//    uint8_t pageCnt = 2;
//    uint32_t dataSize = deviceLayout.mainSize * pageCnt;
//    HPNFC_OBJ *drv = HPNFC_GetInstance();
//
//    if (!dataSize)
//        return 1;
//
//    Fillbuf(wBuffer, dataSize / 4, 9);
//    CPS_CacheFlush((uintptr_t)wBuffer, dataSize, 0);
//    Clearbuf(rBuffer, dataSize / 4, 0xDEADC0DE);
//    CPS_CacheFlush((uintptr_t)rBuffer, dataSize, 0);
//
//
//    status = CdmaExecEraseBlock(privateData, 0, 0, 1);
//    if (status){
//        return status;
//    }
//
//    status = CdmaExecProgramPage(privateData, 0, 0, pageCnt, wBuffer, 0);
//    if (status){
//        return status;
//    }
//
//    memset(&cdmaCmd, 0, sizeof(HPNFC_CdmaCmd));
//
//    cdmaCmd.bank = 0;
//    cdmaCmd.rowAddr = 0;
//    cdmaCmd.cmdCount = 1;
//    cdmaCmd.cmdType = HPNFC_CDMA_CMD_TYPE_READ;
//    cdmaCmd.dmaMode = HPNFC_DMA_MODE_SLAVE;
//    cdmaCmd.memPtr = rBuffer;
//
//    drv->assembleCdmaDesc(privateData, &cdmaDesc[0][0], &cdmaCmd);
//    CPS_CacheFlush((uintptr_t)&cdmaDesc[0][0], HPNFC_CDMA_DESC_SIZE, 0);
//
//    memset(&cdmaCmd, 0, sizeof(HPNFC_CdmaCmd));
//
//    cdmaCmd.bank = 0;
//    cdmaCmd.rowAddr = 1;
//    cdmaCmd.cmdCount = 1;
//    cdmaCmd.cmdType = HPNFC_CDMA_CMD_TYPE_READ;
//    cdmaCmd.dmaMode = HPNFC_DMA_MODE_SLAVE;
//    cdmaCmd.memPtr = rBuffer + deviceLayout.mainSize / 4;
//    cdmaCmd.nextPtr = (uint64_t)(uintptr_t)&cdmaDesc[0][0];
//
//    status = CdmaExecCmdReq(privateData, &cdmaCmd, &cmdReq, 0);
//    if (status){
//        printf("%s chain CDMA operation failed \n", __func__);
//        return status;
//    }
//
//    CPS_CacheInvalidate((uintptr_t)rBuffer, dataSize, 0);
//    if (Comparebuf(wBuffer, rBuffer, dataSize / 4)){
//        printf("%s written and read data differs\n", __func__);
//        PrintCmpBuf(wBuffer, rBuffer, 128);
//        return 1;
//    }
//
//    return 0;
//}
//
//int DebugInterfaceTest(void *privateData)
//{
//    int status;
//    HPNFC_DebugReq dbgReq;
//    HPNFC_OBJ *drv = HPNFC_GetInstance();
//
//    memset(&dbgReq, 0, sizeof(dbgReq));
//
//    status = drv->setDebugIfType(privateData, HPNFC_DBG_IF_TYPE_REGISTER);
//    if (status){
//        printf("%s cannot set register access to debug interface \n", __func__);
//        return status;
//    }
//
//    dbgReq.wordSel = 1;
//    dbgReq.synchMode = HPNFC_DBG_SYNCH_MODE_NON_SYNCH;
//    status = drv->getDebugData(privateData, &dbgReq);
//    if (status){
//        printf("%s get dabug data failed \n", __func__);
//        return status;
//    }
//    printf("%s: Read debug data: %d\n", __func__, dbgReq.data);
//
//    dbgReq.wordSel = 1;
//    dbgReq.synchMode = HPNFC_DBG_SYNCH_MODE_SYNCH_BY_NF_CLK;
//    status = drv->getDebugData(privateData, &dbgReq);
//    if (status){
//        printf("%s get dabug data failed \n", __func__);
//        return status;
//    }
//    printf("%s: Read debug data: %d\n", __func__, dbgReq.data);
//
//
//    dbgReq.wordSel = 1;
//    dbgReq.synchMode = HPNFC_DBG_SYNCH_MODE_SYNCH_BY_BCH_CLK;
//    status = drv->getDebugData(privateData, &dbgReq);
//    if (status){
//        printf("%s get dabug data failed \n", __func__);
//        return status;
//    }
//    printf("%s: Read debug data: %d\n", __func__, dbgReq.data);
//
//
//    return 0;
//}
//
////int DmaTriggLvlTest(void *privateData)
////{
////    int status;
////    HPNFC_DmaConfig dmaConfig;
////    HPNFC_OBJ *drv = HPNFC_GetInstance();
////    uint8_t pageCnt = 1;
////    uint32_t dataSize = deviceLayout.mainSize * pageCnt;
////
////    if (!dataSize)
////        return 1;
////
////
////    /* split data for 4 packages
////     * size is given in 64b words so divide by 8 */
////    dmaConfig.dmaPackSizeInWords = (dataSize / 4) / 8;
////    dmaConfig.trigLvlInWords = (dataSize / 4) / 8;
////    status = drv->setDmaConfig(privateData, &dmaConfig);
////    if (status){
////        printf("%s set DMA configuration failed \n", __func__);
////        return status;
////    }
////
////    /* run spllited transmission in DMA mode with chain of descriptors */
////    status = CdmaChainSlaveDmaTest(privateData);
////    if (status){
////        printf("%s DMA trigger level test failed in CDMA mode \n", __func__);
////        return status;
////    }
////
////    /* run spllited transmission in PIO mode */
////    status = PioTest(privateData, HPNFC_DMA_MODE_SLAVE);
////    if (status){
////        printf("%s DMA trigger level test failed in PIO mode \n", __func__);
////        return status;
////    }
////
////    dmaConfig.dmaPackSizeInWords = 1;
////    dmaConfig.trigLvlInWords = 1;
////    status = drv->setDmaConfig(privateData, &dmaConfig);
////    if (status){
////        printf("%s set DMA configuration failed \n", __func__);
////        return status;
////    }
////
////    /* run spllited transmission in Generic mode */
////    status = GenericTest(privateData);
////    if (status){
////        printf("%s DMA trigger level test failed in Generic mode \n", __func__);
////        return status;
////    }
////
////    dmaConfig.dmaPackSizeInWords = 0;
////    dmaConfig.trigLvlInWords = 0;
////    status = drv->setDmaConfig(privateData, &dmaConfig);
////    if (status){
////        printf("%s set DMA configuration failed \n", __func__);
////        return status;
////    }
////
////    return 0;
////}
//
//int RemapTest(void *privateData)
//{
//    int status, i;
//    //HPNFC_DmaConfig dmaConfig;
//    HPNFC_OBJ *drv = HPNFC_GetInstance();
//    uint16_t count;
//    uint8_t enabled = 0;
//    uint32_t mask;
//    HPNFC_RemapRecord rmpRec[3] = {
//        {.logicAddr = 100, .physAddr = 500},
//        {.logicAddr = 120 , .physAddr = 400},
//        {.logicAddr = 290, .physAddr = 300}};
//
//    status = drv->setRemapEnable(privateData, 1);
//    if (status)
//        return status;
//
//    status = drv->getRemapEnable(privateData, &enabled);
//    if (status)
//        return status;
//
//    if (enabled != 1){
//        printf("%s Remap is not enabled \n", __func__);
//        return 1;
//    }
//
//    status = drv->setRemapMask(privateData, 0xFF00FF);
//    if (status)
//        return status;
//
//    status = drv->getRemapMask(privateData, &mask);
//    if (status)
//        return status;
//
//    if (mask != 0xFF00FF){
//        printf("%s Get remap mask is different than set remap mask \n", __func__);
//        return 1;
//    }
//
//    status = drv->setRemapMask(privateData, 0xFFFFFF);
//    if (status)
//        return status;
//
//    status = drv->clearRemapTable(privateData);
//    if (status){
//        printf("%s clear Remap Table failed \n", __func__);
//        return status;
//    }
//
//    status = drv->getRemapRecordCount(privateData, &count);
//    if (status){
//        printf("%s get Remap records count failed \n", __func__);
//        return status;
//    }
//
//    if (count != 0){
//        printf("%s There are some records but table is expected to be empty \n", __func__);
//        return 1;
//    }
//
//    for (i = 0; i < 3; i++){
//        status = drv->writeRemapRecord(privateData, &rmpRec[i], 0);
//        if (status){
//            printf("%s adding record to remap table failed \n", __func__);
//            return status;
//        }
//    }
//
//    for (i = 0; i < 3; i++){
//        HPNFC_RemapRecord rmpRecRd;
//        status = drv->readRemapRecord(privateData, i, &rmpRecRd, 0);
//        if (status){
//            printf("%s read record from remap table failed \n", __func__);
//            return status;
//        }
//        if ((rmpRecRd.logicAddr != rmpRec[i].logicAddr) || (rmpRecRd.physAddr != rmpRec[i].physAddr)){
//            printf("%s written record not found\n", __func__);
//            return 1;
//        }
//    }
//
//    status = drv->getRemapRecordCount(privateData, &count);
//    if (status){
//        printf("%s get Remap records count failed \n", __func__);
//        return status;
//    }
//
//    if (count != 3){
//        return 1;
//    }
//
//    /* disable remapping */
//    status = drv->setRemapEnable(privateData, 0);
//    if (status)
//        return status;
//
//    return 0;
//}
//
//int RemapTest2(void *privateData)
//{
//    int status;
//    HPNFC_OBJ *drv = HPNFC_GetInstance();
//    //uint16_t count;
//    uint8_t pageCnt = 1;
//    uint32_t dataSize = deviceLayout.mainSize * pageCnt;
//    HPNFC_RemapRecord rmpRec;
//    uint8_t memNr = 0;
//    uint32_t block1 = 0, block2 = 1;
//
//    uint32_t *wBufferBlock1 = wBuffer;
//    uint32_t *wBufferBlock2 = wBuffer + dataSize / 4;
//    uint32_t *rBufferBlock1 = rBuffer;
//    uint32_t *rBufferBlock2 = rBuffer + dataSize / 4;
//
//    Fillbuf(wBufferBlock1, dataSize / 4, 1);
//    Fillbuf(wBufferBlock2, dataSize / 4, 2);
//    CPS_CacheFlush((uintptr_t)wBuffer, dataSize * 2, 0);
//    Clearbuf(rBuffer, (dataSize / 4) * 2, 0xDEADC0DE);
//    CPS_CacheFlush((uintptr_t)rBuffer, dataSize * 2, 0);
//
//    status = CdmaExecEraseBlock(privateData, memNr,
//                                block1 * deviceLayout.pagesPerBlock, 1);
//    if (status){
//        return status;
//    }
//
//    status = CdmaExecEraseBlock(privateData, memNr,
//                                block2 * deviceLayout.pagesPerBlock, 1);
//    if (status){
//        return status;
//    }
//
//    status = CdmaExecProgramPage(privateData, memNr,
//                                 block1 * deviceLayout.pagesPerBlock,
//                                 pageCnt, wBufferBlock1, HPNFC_DMA_MODE_MASTER);
//    if (status){
//        return status;
//    }
//
//    status = CdmaExecProgramPage(privateData, memNr,
//                                 block2 * deviceLayout.pagesPerBlock,
//                                 pageCnt, wBufferBlock2, HPNFC_DMA_MODE_MASTER);
//    if (status){
//        return status;
//    }
//
//    status = CdmaExecReadPage(privateData, memNr,
//                              block1 * deviceLayout.pagesPerBlock,
//                              pageCnt, rBufferBlock1, HPNFC_DMA_MODE_MASTER);
//    if (status){
//        return status;
//    }
//
//    status = CdmaExecReadPage(privateData, memNr,
//                              block2 * deviceLayout.pagesPerBlock,
//                              pageCnt, rBufferBlock2, HPNFC_DMA_MODE_MASTER);
//    if (status){
//        return status;
//    }
//
//    CPS_CacheInvalidate((uintptr_t)rBuffer, dataSize * 2, 0);
//    if (Comparebuf(wBufferBlock1, rBufferBlock1, dataSize / 4)){
//        printf("%s:%d written and read data differs\n", __func__, __LINE__);
//        return 1;
//    }
//    if (Comparebuf(wBufferBlock2, rBufferBlock2, dataSize / 4)){
//        printf("%s:%d written and read data differs\n", __func__, __LINE__);
//        return 1;
//    }
//
//    /* clear both read buffers */
//    Clearbuf(rBuffer, (dataSize / 4) * 2, 0xDEADC0DE);
//    CPS_CacheFlush((uintptr_t)rBuffer, dataSize * 2, 0);
//
//    status = drv->setRemapEnable(privateData, 1);
//    if (status)
//        return status;
//
//    status = drv->clearRemapTable(privateData);
//    if (status){
//        printf("%s clear Remap Table failed \n", __func__);
//        return status;
//    }
//
//    /* remap block 2 to block 1 */
//    rmpRec.logicAddr = block2 * deviceLayout.pagesPerBlock;
//    rmpRec.physAddr = block1;
//    status = drv->writeRemapRecord(privateData, &rmpRec, memNr);
//    if (status){
//        printf("%s adding record to remap table failed \n", __func__);
//        return status;
//    }
//
//    status = CdmaExecReadPage(privateData, memNr,
//                              block1 * deviceLayout.pagesPerBlock,
//                              pageCnt, rBufferBlock1, HPNFC_DMA_MODE_MASTER);
//    if (status){
//        return status;
//    }
//
//    status = CdmaExecReadPage(privateData, memNr,
//                              block2 * deviceLayout.pagesPerBlock,
//                              pageCnt, rBufferBlock2, HPNFC_DMA_MODE_MASTER);
//    if (status){
//        return status;
//    }
//
//    CPS_CacheInvalidate((uintptr_t)rBuffer, dataSize * 2, 0);
//    if (Comparebuf(wBufferBlock1, rBufferBlock1, dataSize / 4)){
//        printf("%s:%d written and read data differs\n", __func__, __LINE__);
//        return 1;
//    }
//
//    /* aftrer remap data read from secod block should be same as data in first block */
//    if (Comparebuf(wBufferBlock1, rBufferBlock2, dataSize / 4)){
//        printf("%s:%d written and read data differs\n", __func__, __LINE__);
//        return 1;
//    }
//
//    return 0;
//}
//
//
static int TimeoutTest(void *privateData, uintptr_t resetType)
{
    int status;
    HPNFC_OBJ *drv = HPNFC_GetInstance();
    uint8_t memNr = 0;
    HPNFC_TimeoutConfig timeoutConfig;
    HPNFC_CmdReq cmdReq;
    HPNFC_RstReq *req = &cmdReq.req.reset;
    uint32_t blk_id = 20;
    uint32_t row_addr = blk_id << deviceLayout.blockShift;
    printf("[NFC_TEST] %s() and resetType set to %ld\n", __func__, resetType);

    cmdReq.thread = 0;
    cmdReq.reqType = HPNFC_REQ_TYPE_RST;
    cmdReq.status = HPNFC_STATUS_PENDING;
    req->resetType = 0;

    printf("Step 1 : Enable timeout watchdog with the smallest timeout value\n");
    /* enable timeout watchdog with the smallest timeout value */
    timeoutConfig.timeoutEn = 1;
    timeoutConfig.timeout = 0x1;
    status = drv->setTimeoutConfig(privateData, &timeoutConfig);
    if (status)
        return status;

    // goto skip;
    printf("Step 2: Check if timeout is reported in CDMA mode\n");
    /* check if timeout is reported in CDMA mode*/
    status = CdmaExecEraseBlock(privateData, memNr, row_addr, 5);
    if (status == 0)
        return 1;

    // timeout should occur after sending cdma erase command


    // printf("Step 3: Execute reset request\n");
    // /* execute reset request*/
    // status = drv->execCmdReq(privateData, &cmdReq);
    // if (status)
    // 	return status;

    // status = ReqWaitForFinish(privateData, &cmdReq);
    // if (status)
    // 	return status;

    // if (cmdReq.status)
    // 	return 1;

    printf("Step 4: Check if timeout is reported in PIO mode\n");
    /* check if timeout is reported in PIO mode*/
    status = PioExecEraseBlock(privateData, memNr, row_addr, 5);
    printf("2 status %d\n", status);
    if (status == 0)
        return 1;

    // printf("Step 5: Execute reset request\n");
    // cmdReq.thread = 0;
    // cmdReq.reqType = HPNFC_REQ_TYPE_RST;
    // cmdReq.status = HPNFC_STATUS_PENDING;
    // req->resetType = 1;
    // /* execute reset request*/
    // status = drv->execCmdReq(privateData, &cmdReq);
    // if (status)
    //    return status;

    // status = ReqWaitForFinish(privateData, &cmdReq);
    // if (status)
    // 	return status;

// skip:

    printf("Step 6: Disable timeout watchdog\n");
    /* disable timeout watchdog */
    timeoutConfig.timeoutEn = 0;
    timeoutConfig.timeout = 0;
    status = drv->setTimeoutConfig(privateData, &timeoutConfig);
    printf("Disable timeout watchdog status %d\n", status);
    if (status)
        return status;

    printf("Step 7: After disable timeout detect, send a cdma erase command\n");
    // after disable timeout detect
    status = CdmaExecEraseBlock(privateData, memNr, row_addr, 1);
    printf("No timeout cmd erase status %d\n", status);
    if (status == 0)
        return 1;

    return 0;
}

/*

[Initial condition]
Initialize NFC IP and set correct ECC configuration

[Test steps]
1. Pick a block
2. Send CDMA Reset command to NAND
3. Send CDMA Erase command to erase a block
4. Send CDMA write command to all pages in a block
5. Send CDMA Read command to all pages in a block

[Expected Result]
All NAND operations work properly.

Code done
*/
int test_plan1_verify_cdma_mode(void *privateData)
{
    printf("[NFC_TEST] %s()\n", __func__);

    EccCdmaTest(privateData);
    return 0;
}

/*

[Initial condition]
Initialize NFC IP and set correct ECC configuration

[Test steps]
1. Pick a block
2. Send PIO Reset command to NAND
3. Send PIO Set Feature command to NAND
4. Send PIO Erase command to erase a block
5. Send PIO write command to all pages in a block
6. Send PIO Read command to all pages in a block

[Expected Result]
All NAND operations work properly.

Code done;
*/
int test_plan2_verify_pio_mode(void *privateData)
{
    printf("[NFC_TEST] %s()\n", __func__);

    int result = PioTest(privateData, HPNFC_DMA_MODE_SLAVE, 0);
    if (result) {
        assert(0);
        return -1;
    }

    result = PioTest(privateData, HPNFC_DMA_MODE_MASTER, 0);
    if (result) {
        assert(0);
        return -1;
    }

    return 0;
}
//
/*

[Initial condition]
Initialize NFC IP and set correct ECC configuration

[Test steps]
A. Use generic work mode to send E/P/R commands and verify data.
B. Use CMD/Add/Data sequence command and check waveform
C. Send Set/Get features
D. Send Read parameter page command

[Expected Result]
All NAND operations work properly.

*/
/*
 * half done
 */
int test_plan3_verify_generic_work_mode(void *privateData, uint32_t blk_id)
{
    uint8_t pageCnt = 1;
    uint32_t dataSize = deviceLayout.mainSize * pageCnt;
    int status = 0;
    printf("[NFC_TEST] %s(), do generic erase, program, read and verify data \n", __func__);
    // uint32_t blk_id = 20;
    uint32_t row_addr = blk_id << deviceLayout.blockShift;
    printf("blk shift %d\n", deviceLayout.blockShift);

    SetUpEccConfig(privateData);

    status = GenExecEraseBlock(privateData, 0, row_addr);
    printf("gen erase status %d\n", status);

    /* we need some time to erase block */
    mdelay(10);

    Fillbuf(wBuffer, dataSize / 4, 0);
    CPS_CacheFlush((uintptr_t)wBuffer, dataSize, 0);
    Clearbuf(rBuffer, dataSize / 4, 0xDEADC0DE);
    CPS_CacheFlush((uintptr_t)rBuffer, dataSize, 0);
    printf("org buf \n");
    PrintCmpBuf(wBuffer, rBuffer, 32);

    status = GenExecProgramPage(privateData, 0, row_addr, wBuffer, deviceLayout.mainSize);
    printf("generic write status %d\n\n", status);
    status = GenExecReadPage(privateData, 0, row_addr,rBuffer, deviceLayout.mainSize);
    printf("generic read status %d\n\n", status);

    CPS_CacheInvalidate((uintptr_t)rBuffer, dataSize, 0);
    if (Comparebuf(wBuffer, rBuffer, dataSize / 4)){
        printf("%s written and read data differs\n", __func__);
        PrintCmpBuf(wBuffer, rBuffer, dataSize / 4);
        assert(0);
        return 1;
    }
    else
    {
        printf("==%s written and read data the same==\n", __func__);
    }

    return 0;
}

int test_plan3_1_gen_e_cdma_w_cdma_r(void *privateData, uint32_t blk_id)
{
    uint8_t pageCnt = 1;
    uint32_t dataSize = deviceLayout.mainSize * pageCnt;
    int status = 0;
    // uint32_t blk_id = 1;
    uint32_t row_addr = blk_id << deviceLayout.blockShift;
    printf("blk shift %d\n", deviceLayout.blockShift);

    printf("[NFC_TEST] %s(), generic erase, cdma write and read\n", __func__);

    SetUpEccConfig(privateData);

    status = GenExecEraseBlock(privateData, 0, row_addr);
    printf("gen e status %d\n", status);
    if (status){
        return status;
    }

    Fillbuf(wBuffer, dataSize / 4, 0);
    CPS_CacheFlush((uintptr_t)wBuffer, dataSize, 0);
    Clearbuf(rBuffer, dataSize / 4, 0xDEADC0DE);
    CPS_CacheFlush((uintptr_t)rBuffer, dataSize, 0);
    printf("org buf \n");
    PrintCmpBuf(wBuffer, rBuffer, 32);

    status = CdmaExecProgramPage(privateData, 0, blk_id, 1, wBuffer, 0);
    printf("cdma w status %d\n", status);

    status = CdmaExecReadPage(privateData, 0, blk_id, 1, rBuffer, 0);
    printf("cdma r status %d\n", status);
    if (status){
        return status;
    }

    CPS_CacheInvalidate((uintptr_t)rBuffer, dataSize, 0);
    if (Comparebuf(wBuffer, rBuffer, dataSize / 4)){
        printf("%s written and read data differs\n", __func__);
        PrintCmpBuf(wBuffer, rBuffer, dataSize / 4);
        assert(0);
        return 1;
    }
    else
    {
        printf("==%s written and read data the same==\n", __func__);
    }

    return 0;
}
int test_plan3_2_cdma_write_gen_read(void *privateData, uint32_t blk_id)
{
    uint8_t pageCnt = 1;
    uint32_t dataSize = deviceLayout.mainSize * pageCnt;
    int status = 0;
    // uint32_t blk_id = 1;
    uint32_t row_addr = blk_id << deviceLayout.blockShift;
    printf("blk shift %d\n", deviceLayout.blockShift);

    printf("[NFC_TEST] %s(), cdma erase, write and gen read\n", __func__);

    SetUpEccConfig(privateData);

    status = CdmaExecEraseBlock(privateData, 0, row_addr, 1);
    printf("cdma e status %d\n", status);
    if (status){
        return status;
    }

    Fillbuf(wBuffer, dataSize / 4, 0);
    // CPS_CacheFlush((uintptr_t)wBuffer, dataSize, 0);
    Clearbuf(rBuffer, dataSize / 4, 0xDEADC0DE);
    // CPS_CacheFlush((uintptr_t)rBuffer, dataSize, 0);
    // printf("org buf \n");
    // PrintCmpBuf(wBuffer, rBuffer, 32);

    status = CdmaExecProgramPage(privateData, 0, row_addr, 1, wBuffer, 0);
    printf("cdma w status %d\n", status);

    status = GenExecReadPage(privateData, 0, row_addr, rBuffer, deviceLayout.mainSize);
    printf("gen r status %d\n", status);

    CPS_CacheInvalidate((uintptr_t)rBuffer, dataSize, 0);
    if (Comparebuf(wBuffer, rBuffer, dataSize / 4)){
        printf("%s written and read data differs\n", __func__);
        PrintCmpBuf(wBuffer, rBuffer, dataSize / 4);
        assert(0);
        return 1;
    }
    else
    {
        printf("==%s written and read data the same==\n", __func__);
    }

    return 0;
}

// int test_plan3_3_cdma_write_gen_read(void *privateData, uint32_t blk_id)
// {
// 	uint8_t pageCnt = 1;
// 	uint32_t dataSize = deviceLayout.mainSize * pageCnt;
// 	int status = 0;
// 	// uint32_t blk_id = 1;
// 	uint32_t row_addr = blk_id << deviceLayout.blockShift;
// 	printf("blk shift %d\n", deviceLayout.blockShift);

// 	printf("[NFC_TEST] %s()\n\n\n", __func__);

// 	SetUpEccConfig(privateData);

// 	status = CdmaExecEraseBlock(privateData, 0, 0, 1);
// 	if (status){
// 		return status;
// 	}

// 	Fillbuf(wBuffer, dataSize / 4, 0);
// 	CPS_CacheFlush((uintptr_t)wBuffer, dataSize, 0);
// 	Clearbuf(rBuffer, dataSize / 4, 0xDEADC0DE);
// 	CPS_CacheFlush((uintptr_t)rBuffer, dataSize, 0);
// 	printf("org buf \n");
// 	PrintCmpBuf(wBuffer, rBuffer, 32);

// 	status = GenExecProgramPage(privateData, 0, 0, wBuffer, deviceLayout.mainSize);
// 	printf("gen w status %d\n", status);

// 	status = CdmaExecReadPage(privateData, 0, 0, 1, rBuffer, 0);
// 	printf("cdma r status %d\n", status);

// 	CPS_CacheInvalidate((uintptr_t)rBuffer, dataSize, 0);
// 	if (Comparebuf(wBuffer, rBuffer, dataSize / 4)){
// 		printf("%s written and read data differs\n", __func__);
// 		PrintCmpBuf(wBuffer, rBuffer, 32);
// 		return 1;
// 	}
// 	else
// 	{
// 		printf("==%s written and read data the same==\n", __func__);
// 	}

// 	return 0;
// }

/*
* Done
*/
int test_plan4_verify_uecc_error_report(void *privateData)
{
    printf("[NFC_TEST] %s()\n", __func__);
    if (EccCdmaTest2(privateData))
    {
        assert(0);
    }
    return 0;
}

///*
// * Done
// */
int test_plan5_verify_desc_error(void *privateData)
{
#if 0
    //uint8_t pageCnt = 1;
    //uint32_t dataSize = deviceLayout.mainSize * pageCnt;
    //int status = 0;
    //printf("[NFC_TEST] %s()\n", __func__);

    //SetUpEccConfig(privateData);

    //status = GenExecEraseBlock(privateData, 0, 0);
    //printf("gen e status %d\n", status);

    Cdma_Err_Desc_Test(privateData);
    return 0;
#else
    int status;
    uint8_t rnb;
    HPNFC_OBJ *drv = HPNFC_GetInstance();
    HPNFC_CmdReq cmdReq;
    printf("%s\n", __func__);

    // send invalid cmd type to introduce cmd desc error
    GenAssembleCmdWithAddr(privateData, &cmdReq, 0, HPNFC_GEN_CMD_TYPE_ERASE , 0, 0, 1);
    /*
        set minictrlL to 0x78 (wrong value) in HPNFC_GenericSend to trigger command error

        miniCtrlCmdL = miniCtrlCmd & 0xFFFFFFFF;
        miniCtrlCmdL = 0x78; // for testing command error
        miniCtrlCmdH = miniCtrlCmd >> 32;
    */

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
#endif
}

///*
// * pending
// */
int test_plan6_verify_reset_type0(void *privateData)
{
    // pass testing
    printf("[NFC_TEST] %s()\n", __func__);

    TimeoutTest(privateData, 0);
    return 0;
}

/*
 * pending
 */
int test_plan7_verify_reset_type1(void *privateData)
{
    // pass testing
    printf("[NFC_TEST] %s()\n", __func__);

    TimeoutTest(privateData, 1);
    return 0;
}

int test_plan8_verify_timeout(void *privateData)
{
    printf("[NFC_TEST] %s()\n", __func__);

    TimeoutTest(privateData, 0);
    return 0;
}

/*

[Initial condition]
Initialize NFC IP and set correct ECC configuration

 [Test steps]

1. Set different sector size/correction strength
2. Change sector_size & last_sector_size field in the transfer_cfg_1 (0x0404) register
3. Send E/P/R command and verify data integrity, go back to step 1 and use different ECC setting, finish the test when all setting are verified.

 [Expected Result]
All NAND operations work properly.

correction,  sector size in bytes, check-bit size in bytes, corr_str value
8			512-544					14						0
            1024-1056

12			512-544					22						1
            1024-1056

16			512-544					28						2
            1024-1056

20			512-544					36						3
            1024-1056

32			512-544					56						4
            1024-1056

40			512-544					70						5
            1024-1056

60			512-544					106						6
            1024-1056

80			512-544					140						7

*/
int test_plan9_verify_ecc_engine(void *privateData)
{
    printf("[NFC_TEST] %s()\n", __func__);

    printf("test1 should pass\n");
    SetUpSecSizeAndCorrCap(1024, 8);
    if (EccCdmaTest(privateData))
    {
        assert(0);
    }

    printf("test2 should pass\n");
    SetUpSecSizeAndCorrCap(1024, 12);
    if (EccCdmaTest(privateData))
    {
        assert(0);
    }

    printf("test3 should pass\n");
    SetUpSecSizeAndCorrCap(1024, 16);
    if (EccCdmaTest(privateData))
    {
        assert(0);
    }
    // should fail
    printf("test4 should fail\n");
    SetUpSecSizeAndCorrCap(512, 12);
    if (EccCdmaTest(privateData))
    {
//        assert(0);
    }
    else
    {
        assert(0);
    }

    printf("test5 should fail\n");
    SetUpSecSizeAndCorrCap(1024, 20);
    if (EccCdmaTest(privateData))
    {
//        assert(0);
    }
    else
    {
        assert(0);
    }

    printf("test6 should pass\n");
    SetUpSecSizeAndCorrCap(512, 8);
    if (EccCdmaTest(privateData))
    {
        assert(0);
    }

    return 0;
}

/*
 * Program NAND and try boot from it
 */
int test_plan10_verify_nand_boot(void *privateData)
{
    printf("[NFC_TEST] %s()\n", __func__);
    CdmaBootTest(privateData);
    return 0;
}

int test_plan11_verify_status_polling(void *privateData)
{
    HPNFC_OBJ *drv = HPNFC_GetInstance();
    printf("[NFC_TEST] %s()\n", __func__);

    HPNFC_StatusPollingConfig statusPollingCfg;
    drv->getStatusPollingConfig(privateData, &statusPollingCfg);
    printf("org statusPollingCfg.disableStatusPolling %d\n", statusPollingCfg.disableStatusPolling);
    printf("statusPollingCfg.longPolling %d\n", statusPollingCfg.longPolling);
    printf("statusPollingCfg.readyMask %d\n", statusPollingCfg.readyMask);
    printf("statusPollingCfg.readyValue %d\n", statusPollingCfg.readyValue);
    printf("statusPollingCfg.shortPolling %d\n", statusPollingCfg.shortPolling);

    statusPollingCfg.disableStatusPolling = 0;
    drv->setStatusPollingConfig(privateData, &statusPollingCfg);
    drv->getStatusPollingConfig(privateData, &statusPollingCfg);
    printf("new statusPollingCfg.disableStatusPolling %d\n", statusPollingCfg.disableStatusPolling);
    printf("statusPollingCfg.longPolling %d\n", statusPollingCfg.longPolling);
    printf("statusPollingCfg.readyMask %d\n", statusPollingCfg.readyMask);
    printf("statusPollingCfg.readyValue %d\n", statusPollingCfg.readyValue);
    printf("statusPollingCfg.shortPolling %d\n", statusPollingCfg.shortPolling);

    if (EccCdmaTest(privateData))
    {
        assert(0);
    }

    statusPollingCfg.disableStatusPolling = 1;
    drv->setStatusPollingConfig(privateData, &statusPollingCfg);
    drv->getStatusPollingConfig(privateData, &statusPollingCfg);
    printf("restore statusPollingCfg.disableStatusPolling %d\n", statusPollingCfg.disableStatusPolling);
    printf("statusPollingCfg.longPolling %d\n", statusPollingCfg.longPolling);
    printf("statusPollingCfg.readyMask %d\n", statusPollingCfg.readyMask);
    printf("statusPollingCfg.readyValue %d\n", statusPollingCfg.readyValue);
    printf("statusPollingCfg.shortPolling %d\n", statusPollingCfg.shortPolling);

    if (EccCdmaTest(privateData))
    {
        assert(0);
    }

    return 0;
}

/*
//[Initial condition]
//Initialize NFC IP and set correct ECC configuration
//
//[Test steps]
//1. Enable interrupt in intr_enable register
//2. Prepare command
//3. Trigger CE to start processing these command
//
//[Expected Result]
//After command done, interrupt should be triggerred and ISR will be called.
*/
int test_plan12_verify_interrupts_conf(void *privateData)
{
    printf("[NFC_TEST] %s()\n", __func__);

    return 0;
}

/*
//[Initial condition]
//Initialize NFC IP and set correct ECC configuration
//
//[Test steps]
//1. Setup LA to capture the waveform
//2. Change value of async_toggle_timings/timings0/timings1/timings2 to precalculated value.
//3. Prepare commad and trigger CE to process them
//
//[Expected Result]
//Check waveform to see if match the value set in timing registers
//
//Pass testing
*/
int test_plan13_verify_timing_regs(void *privateData)
{
    printf("[NFC_TEST] %s()\n", __func__);
    work_mode_test(privateData);
    return 0;
}

/*
 *	Already verified by other test items.
 */
int test_plan14_verify_sdma(void *privateData)
{
    printf("[NFC_TEST] %s()\n", __func__);

    return 0;
}

/*
 *  Already verified by other test items
 */
int test_plan15_verify_nands(void *privateData)
{
    printf("[NFC_TEST] %s()\n", __func__);

    int result = PioTest(privateData, HPNFC_DMA_MODE_SLAVE, 0);
    if (result)
    {
        return -1;
    }

    return 0;
}

int test_plan16_prg_vec_and_fip(void *privateData)
{
    printf("[NFC_TEST] %s()\n", __func__);
    prg_vec_and_fip(privateData);

    //for (uint32_t i = 0; i < 20; i++)
    //{
    //    int result = PioTest(privateData, HPNFC_DMA_MODE_SLAVE, i);
    //    if (result)
    //    {
    //        return -1;
    //    }
    //}

    return 0;
}

int test_plan17_prg_all_nand(void *privateData)
{
    printf("[NFC_TEST] %s()\n", __func__);

    scan_factory_defect(privateData);
    ecc_cdma_all_nand_test(privateData);

    return 0;
}

int test_plan18_kernel_related_testing(void *privateData)
{
    printf("[NFC_TEST] %s()\n", __func__);

    kernel_test(privateData);

    return 0;
}

HPNFC_DeviceLayout *get_dev_layout(void)
{
    return &deviceLayout;
}
