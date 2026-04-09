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
 * tests.h
 *
 * HPNFC driver tests
 *****************************************************************************/



#ifndef TESTS_H
#define TESTS_H

#include <stdio.h>
#include <stdint.h>
#include <hpnfc_if.h>

#define NUM_SUPPORT_TOGGLE_NAND 6

typedef struct _nand_support_para_t {
    uint8_t id[8];

    uint32_t dd_reg_70_value;
    uint32_t dd_reg_74_value;

    uint16_t mainSize;
    uint16_t spareSize;
    uint16_t lunNumber;
    uint16_t pagesPerBlock;
    uint16_t blocksPerLun;
    uint16_t numberOfPlanes;
    uint16_t blockShift;
} nand_support_para_t;

extern nand_support_para_t nand_support_list[];

void slaveDmaHandler(void* privateData, void* data,
                     HPNFC_TransferDirection transferDirection,
                     uint32_t sdmaSize,
                     uint32_t *transferredData);

void* InitDriver(uintptr_t regsBase, uintptr_t sdmaBase);
int CdmaTest(void *privateData);
int CdmaTest2(void *privateData);
int WorkModeTest(void *privateData);
int EccCdmaTest2(void *privateData);
int EccCdmaTest(void *privateData);
int CdmaSyncTest(void *privateData);
int EccScramblerCdmaTest(void *privateData);
int EccTest(void *privateData);
int CdmaBootTest(void *privateData);
int GenericTest(void *privateData);
int InterleavedTest(void *privateData);
int PioMasterDmaTest(void *privateData);
int CdmaCmdParseTest(void *privateData);
int CdmaTestChain(void *privateData);
/* read data by Debug Interface in three supported modes
 * tests passes when functions of driver executes without error
 * there is no way to verify read data */
int DebugInterfaceTest(void *privateData);
/* Try to program page with enabled data integrity
 * and enabled CRC error injection
 * error HPNFC_STATUS_CRC_DAT_ERR is expected as operation result */
int DataIntegrityCrcTest(void *privateData);
/* run tests with set dma trigger level lower than data size
 * to split one transfer to few DMA transfers
 * then written data are compared with read data
 * test is run in PIO, CDMA and Generic mode */
int DmaTriggLvlTest(void *privateData);
/* Try to erase block in CDMA mode with enabled data integrity
 * and enabled contex memory error injection
 * error HPNFC_STATUS_PARITY_CTX_ERR is expected as operation result */
int DataIntegrityCtxMemTest(void *privateData);
/* testing remap function to check if they works propepry
 * writing new remap records, read them and verify if they are the same
 * next read number of records and check it is the same
 * as the number of written records
 * enable remaping and check if it is enabled
 * set remap mask, get remap mask and verify it
 * */
int RemapTest(void *privateData);
/* test verifing whether remapping mechanism works
 * program one page o block 0 and block 1
 * read it and verify if data is ok
 * next enable remap mechanism and add remap block 1 to block 0
 * next read once againg one page from block 0 and block 1
 * data read from both blocks should be the same */
int RemapTest2(void *privateData);
/* data integrity tests for remap mechnism
 * enable parity, set error injection for write
 * remap memory, write few records and expect error
 * */
int DataIntegrityRemapTest(void *privateData);
/* configure timeout feature by setting the smallest
 * timeout value next execute erase command on few blocks
 * timeout error is expected */
// int TimeoutTest(void *privateData, uint32_t resetType);

/* configure NF memory to SLC mode
 * set new pages per block in controller settings
 * (in SLC mode flash memory has less pages per block than in TLC)
 * configure controller to let it know
 * what is first bit of block address in ROW address
 * execute to separate program operation
 * by programming last page in block 0 and first page in block 1
 * Read 2 pages in one operation starting from last page of block 0
 * compare read and written data if the are the same it means that
 * controller was able to get second page */
int PSlcOnfiTest(void *privateData);

/*
    For Test plan
*/

int test_plan1_verify_cdma_mode(void *privateData);
int test_plan2_verify_pio_mode(void *privateData);
int test_plan3_verify_generic_work_mode(void *privateData, uint32_t blk_id);
int test_plan3_1_gen_e_cdma_w_cdma_r(void *privateData, uint32_t blk_id);
int test_plan3_2_cdma_write_gen_read(void *privateData, uint32_t blk_id);
int test_plan4_verify_uecc_error_report(void *privateData);
int test_plan5_verify_desc_error(void *privateData);
int test_plan6_verify_reset_type0(void *privateData);
int test_plan7_verify_reset_type1(void *privateData);
int test_plan8_verify_timeout(void *privateData);
int test_plan9_verify_ecc_engine(void *privateData);
int test_plan10_verify_nand_boot(void *privateData); // unable to verify here
int test_plan11_verify_status_polling(void *privateData);
int test_plan12_verify_interrupts_conf(void *privateData);
int test_plan13_verify_timing_regs(void *privateData);
int test_plan14_verify_sdma(void *privateData);
int test_plan15_verify_nands(void *privateData);

int test_plan16_prg_vec_and_fip(void *privateData);
int test_plan17_prg_all_nand(void *privateData);

HPNFC_DeviceLayout *get_dev_layout(void);
void print_device_layout(void);

#endif
