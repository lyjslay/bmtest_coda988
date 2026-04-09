#include <common.h>
#include "bus.h"
#include "timer.h"

#define ENABLE_PCMONTEST
#define ENABLE_BUSRECTEST
/* Kernel includes. */
/*-----------------------------------------------------------*/

typedef unsigned char datum;    /* Set the data bus width to 8 bits.  */

/**********************************************************************
 *
 * Function:    memTestDataBus()
 *
 * Description: Test the data bus wiring in a memory region by
 *              performing a walking 1's test at a fixed address
 *              within that region.  The address (and hence the
 *              memory region) is selected by the caller.
 *
 * Notes:
 *
 * Returns:     0 if the test succeeds.
 *              A non-zero result is the first pattern that failed.
 *
 **********************************************************************/
datum
memTestDataBus(volatile datum * address)
{
    datum pattern;


    /*
     * Perform a walking 1's test at the given address.
     */
    for (pattern = 1; pattern != 0; pattern <<= 1)
    {
        /*
         * Write the test pattern.
         */
        *address = pattern;

        /*
         * Read it back (immediately is okay for this test).
         */
        if (*address != pattern)
        {
            return (pattern);
        }
    }

    return (0);

}   /* memTestDataBus() */

/**********************************************************************
 *
 * Function:    memTestAddressBus()
 *
 * Description: Test the address bus wiring in a memory region by
 *              performing a walking 1's test on the relevant bits
 *              of the address and checking for aliasing. This test
 *              will find single-bit address failures such as stuck
 *              -high, stuck-low, and shorted pins.  The base address
 *              and size of the region are selected by the caller.
 *
 * Notes:       For best results, the selected base address should
 *              have enough LSB 0's to guarantee single address bit
 *              changes.  For example, to test a 64-Kbyte region,
 *              select a base address on a 64-Kbyte boundary.  Also,
 *              select the region size as a power-of-two--if at all
 *              possible.
 *
 * Returns:     NULL if the test succeeds.
 *              A non-zero result is the first address at which an
 *              aliasing problem was uncovered.  By examining the
 *              contents of memory, it may be possible to gather
 *              additional information about the problem.
 *
 **********************************************************************/
datum *
memTestAddressBus(volatile datum * baseAddress, unsigned long nBytes)
{
    unsigned long addressMask = (nBytes/sizeof(datum) - 1);
    unsigned long offset;
    unsigned long testOffset;

    datum pattern     = (datum) 0xAAAAAAAA;
    datum antipattern = (datum) 0x55555555;


    /*
     * Write the default pattern at each of the power-of-two offsets.
     */
    for (offset = 1; (offset & addressMask) != 0; offset <<= 1)
    {
        baseAddress[offset] = pattern;
    }

    /* 
     * Check for address bits stuck high.
     */
    testOffset = 0;
    baseAddress[testOffset] = antipattern;

    for (offset = 1; (offset & addressMask) != 0; offset <<= 1)
    {
        if (baseAddress[offset] != pattern)
        {
            return ((datum *) &baseAddress[offset]);
        }
    }

    baseAddress[testOffset] = pattern;

    /*
     * Check for address bits stuck low or shorted.
     */
    for (testOffset = 1; (testOffset & addressMask) != 0; testOffset <<= 1)
    {
        baseAddress[testOffset] = antipattern;

        if (baseAddress[0] != pattern)
        {
            return ((datum *) &baseAddress[testOffset]);
        }

        for (offset = 1; (offset & addressMask) != 0; offset <<= 1)
        {
            if ((baseAddress[offset] != pattern) && (offset != testOffset))
            {
                return ((datum *) &baseAddress[testOffset]);
            }
        }

        baseAddress[testOffset] = pattern;
    }

    return (NULL);

}   /* memTestAddressBus() */

/**********************************************************************
 *
 * Function:    memTestDevice()
 *
 * Description: Test the integrity of a physical memory device by
 *              performing an increment/decrement test over the
 *              entire region.  In the process every storage bit
 *              in the device is tested as a zero and a one.  The
 *              base address and the size of the region are
 *              selected by the caller.
 *
 * Notes:
 *
 * Returns:     NULL if the test succeeds.
 *
 *              A non-zero result is the first address at which an
 *              incorrect value was read back.  By examining the
 *              contents of memory, it may be possible to gather
 *              additional information about the problem.
 *
 **********************************************************************/
datum *
memTestDevice(volatile datum * baseAddress, unsigned long nBytes)
{
    unsigned long offset;
    unsigned long nWords = nBytes / sizeof(datum);

    datum pattern;
    datum antipattern;


    /*
     * Fill memory with a known pattern.
     */
    for (pattern = 1, offset = 0; offset < nWords; pattern++, offset++)
    {
        baseAddress[offset] = pattern;
    }

    /*
     * Check each location and invert it for the second pass.
     */
    for (pattern = 1, offset = 0; offset < nWords; pattern++, offset++)
    {
        if (baseAddress[offset] != pattern)
        {
            return ((datum *) &baseAddress[offset]);
        }

        antipattern = ~pattern;
        baseAddress[offset] = antipattern;
    }

    /*
     * Check each location for the inverted pattern and zero it.
     */
    for (pattern = 1, offset = 0; offset < nWords; pattern++, offset++)
    {
        antipattern = ~pattern;
        if (baseAddress[offset] != antipattern)
        {
            return ((datum *) &baseAddress[offset]);
        }
    }

    return (NULL);

}   /* memTestDevice() */

int
memTest(unsigned long address, unsigned long size)
{
    if ((memTestDataBus((volatile datum *) address) != 0) ||
        (memTestAddressBus((volatile datum *) address, size) != NULL) ||
        (memTestDevice((volatile datum *) address, size) != NULL))
    {
        return (-1);
    }
    else
    {
        return (0);
    }
}   /* memTest() */


//#define BASE_ADDRESS  (volatile datum *) 0x00000000
#define NUM_BYTES     (64 * 1024)

//char userMemTestBuf[NUM_BYTES];
unsigned long *userMemTestBuf = (unsigned long *)0x20E000000;
    
void userTest( void )
{
    int count = 0, ret = 0xFF;

    for (count = 0; count < 5; count ++)
    {
        printf("userTest count = %d\r\n", count);
    }
    

    userMemTestBuf += 16;
    
    if (userMemTestBuf >= 0x20E040000)
    {
        userMemTestBuf = 0x20E000000;
    }
    
    ret = memTest((unsigned long)userMemTestBuf, NUM_BYTES);
    
    if (!ret)
    {
        printf("start=0x%lx, len=0x%d, memtest PASS\r\n", userMemTestBuf, NUM_BYTES);
    }
    else
    {
        printf("start=0x%lx, len=0x%d, memtest FAIL\r\n", userMemTestBuf, NUM_BYTES);
    }
}
/*-----------------------------------------------------------*/



#define BUSTEST_DRAMADDR		0x100000000
#define BUSTEST_TPUSRAMADDR		0x0E000000
#define BUSTEST_APBADDR			0x04000000

#define BUSTEST_WDTBASE			0x03010000
#define BUSTEST_WDTCR			(BUSTEST_WDTBASE + 0x000)
#define BUSTEST_WDTORR			(BUSTEST_WDTBASE + 0x004)
#define BUSTEST_SWRESET_EN		(0x03000000 + 0x008)


static int BusTest_AccessTest(unsigned long address)
{
  //just test read length=0x20
  *((volatile unsigned int *)address) ;
  *((volatile unsigned int *)address + 0x4);
  *((volatile unsigned int *)address + 0x8);
  *((volatile unsigned int *)address + 0xC);
  *((volatile unsigned int *)address + 0x10);
  *((volatile unsigned int *)address + 0x14);
  *((volatile unsigned int *)address + 0x18);
  *((volatile unsigned int *)address + 0x1C);
  return 0;
}

int BusTest_MemoryMapToggle(void)
{
  //only toggle with read
  //dram access
  printf("BusTest_MemoryMapToggle start BUSTEST_DRAMADDR\r\n");
  BusTest_AccessTest(BUSTEST_DRAMADDR);

  //TPU sram access
  printf("BusTest_MemoryMapToggle start BUSTEST_TPUSRAMADDR\r\n");
  BusTest_AccessTest(BUSTEST_TPUSRAMADDR);

  //register access
  printf("BusTest_MemoryMapToggle start BUSTEST_APBADDR\r\n");
  BusTest_AccessTest(BUSTEST_APBADDR);

  //using dma engine to read
  //it should use dma
  return 0;
}


int BusTest_Main(void)
{
  int loopCount;

  //memory map toggle
  printf("BusTest_Main BusTest_MemoryMapToggle\r\n");
  BusTest_MemoryMapToggle();

  //drystone?
  printf("BusTest_Main BusTest_DhrystoneMain\r\n");
  BusTest_DhrystoneMain(100);

  for (loopCount = 0; loopCount < 1000; loopCount ++) {
    //memory test
    printf("BusTest_Main userTest loopCount = %d\r\n", loopCount);
    userTest();
  }
  return 0;
}

int BusTest_TriggerWDT(void)
{
  printf("BusTest_TriggerWDT start\n");

  //enable sw root reset
  RAW_WRITE32(BUSTEST_SWRESET_EN, 0x4);

  //enable watchdog
  RAW_WRITE32(BUSTEST_WDTORR, 0x7);
  RAW_WRITE32(BUSTEST_WDTCR, 0x13);

  printf("BusTest_TriggerWDT end\n");
  return 0;
}



#ifdef ENABLE_PCMONTEST
/**********************************************************************
Verilog simulation code from designer YC:

#define TRCBASE         0x01040000
#define TRCPRGCTLR      (TRCBASE + 0x004)
#define TRCOSLAR        (TRCBASE + 0x300)
#define TRCLAR          (TRCBASE + 0xfb0)
#define TRCLASTPCLOR    (TRCBASE + 0x800)
#define TRCLASTPCHIR    (TRCBASE + 0x804)
#define TRCOSLAR_V      0x0
#define TRCLAR_V        0xc5acce55
	
  // Unlock ETM of CPU0
  mmio_wr32(TRCLAR, TRCLAR_V);
  mmio_wr32(TRCOSLAR, TRCOSLAR_V);

  // Enable ETM
  mmio_wr32(TRCPRGCTLR, 0x1);

  delay(10);	  // Add some delay here

  // Disable ETM
  mmio_wr32(TRCPRGCTLR, 0x0);

  // Read the last PC
  temp0_lo = mmio_rd32(TRCLASTPCLOR);
  temp0_hi = mmio_rd32(TRCLASTPCHIR);
**********************************************************************/
#define PCMON_TRCBASE         0x01040000
#define PCMON_TRCPRGCTLR      (PCMON_TRCBASE + 0x004)
#define PCMON_TRCOSLAR        (PCMON_TRCBASE + 0x300)
#define PCMON_TRCLAR          (PCMON_TRCBASE + 0xfb0)
#define PCMON_TRCLASTPCLOR    (PCMON_TRCBASE + 0x800)
#define PCMON_TRCLASTPCHIR    (PCMON_TRCBASE + 0x810)
#define PCMON_TRCOSLAR_V      0x0
#define PCMON_TRCLAR_V        0xc5acce55

int PCMon_Main(void)
{
  int i = 0;
  unsigned long pcmonPC0 = 0, pcmonPC1 = 0;

  /**********************************************************************/
  //Test case 1, test enable and disable
  /**********************************************************************/
  //Unlock ETM of CPU0
  RAW_WRITE32(PCMON_TRCLAR, PCMON_TRCLAR_V);
  RAW_WRITE32(PCMON_TRCOSLAR, PCMON_TRCOSLAR_V);

  //Enable ETM
  RAW_WRITE32(PCMON_TRCPRGCTLR, 0x1);
  udelay (1000);

  pcmonPC0 = RAW_READ32(PCMON_TRCLASTPCHIR);
  pcmonPC0 = (pcmonPC0 << 32) | RAW_READ32(PCMON_TRCLASTPCLOR);

  pcmonPC1 = RAW_READ32(PCMON_TRCLASTPCHIR);
  pcmonPC1 = (pcmonPC1 << 32) | RAW_READ32(PCMON_TRCLASTPCLOR);

  //After pcmon triggering, we should read back differnt value twice
  if (pcmonPC0 != pcmonPC1)
    printf("[PASS] PCMON is triggered, and pcmonPC0 = 0x%lx, pcmonPC1 = 0x%lx\r\n", pcmonPC0, pcmonPC1);
  else
    printf("[FAILED] PCMON is triggered, but read back twice same value, pcmonPC0 = 0x%lx, pcmonPC1 = 0x%lx\r\n", pcmonPC0, pcmonPC1);

  //Disable ETM
  RAW_WRITE32(PCMON_TRCPRGCTLR, 0x0);

  udelay (1000000);
  pcmonPC0 = RAW_READ32(PCMON_TRCLASTPCHIR);
  pcmonPC0 = (pcmonPC0 << 32) | RAW_READ32(PCMON_TRCLASTPCLOR);

  pcmonPC1 = RAW_READ32(PCMON_TRCLASTPCHIR);
  pcmonPC1 = (pcmonPC1 << 32) | RAW_READ32(PCMON_TRCLASTPCLOR);


  //After pcmon stopping, we should read back same value twice
  //Since FPGA ETM clock is always enable, this test item will not PASS
  if (pcmonPC0 == pcmonPC1)
    printf("[PASS] PCMON is stopped, and pcmonPC0 = 0x%lx, pcmonPC1 = 0x%lx\r\n", pcmonPC0, pcmonPC1);
  else
    printf("[FAILED] PCMON is stopped, but read back twice different value, pcmonPC0 = 0x%lx, pcmonPC1 = 0x%lx\r\n", pcmonPC0, pcmonPC1);

  /**********************************************************************/
  //Test case 2, test debug path
  //Using CVD access EAPB or APB on 0x80000000+PCMON_TRCLASTPCHIR
  //the value and PCMON_TRCLASTPCHIR should be identical
  /**********************************************************************/
  //Enable ETM
  RAW_WRITE32(PCMON_TRCPRGCTLR, 0x1);
  udelay (1000);
  pcmonPC0 = RAW_READ32(PCMON_TRCLASTPCHIR);
  pcmonPC0 = (pcmonPC0 << 32) | RAW_READ32(PCMON_TRCLASTPCLOR);
  pcmonPC1 = RAW_READ32(PCMON_TRCLASTPCHIR);
  pcmonPC1 = (pcmonPC1 << 32) | RAW_READ32(PCMON_TRCLASTPCLOR);

  printf("pcmonPC0 = 0x%lx\n", pcmonPC0);
  printf("pcmonPC1 = 0x%lx\n", pcmonPC1);

  //BusTest_TriggerWDT();

  while (1) {
    pcmonPC0 = RAW_READ32(PCMON_TRCLASTPCHIR);
    pcmonPC0 = (pcmonPC0 << 32) | RAW_READ32(PCMON_TRCLASTPCLOR);
    printf("pcmonPC0 = 0x%lx\n", pcmonPC0);
  }
  return 0;
}
#endif

#ifdef ENABLE_BUSRECTEST

enum BUSREC_TYPE {
  BUSREC_TYPE_DISABLE   = 0x0,
  BUSREC_TYPE_MON       = 0x1,
  BUSREC_TYPE_HANG_INT  = 0x2,
  BUSREC_TYPE_HITR_INT  = 0x4,
  BUSREC_TYPE_HITW_INT  = 0x8,
  BUSREC_TYPE_ALL       = 0xF,
};

enum BUSREC_INT {
  BUSREC_INT_ZERO       = 0x0,
  BUSREC_INT_HANGR      = 0x1,
  BUSREC_INT_HANGW      = 0x2,
  BUSREC_INT_HITR       = 0x4,
  BUSREC_INT_HITW       = 0x8,
  BUSREC_INT_ALL        = 0xF,
};


#define BUSREC_BASE           0x01800000

#define BUSREC_ENABLE         (BUSREC_BASE + 0x000)
#define BUSREC_RSTATUS        (BUSREC_BASE + 0x010)
#define BUSREC_WSTATUS        (BUSREC_BASE + 0x014)
#define BUSREC_INTSTATUS      (BUSREC_BASE + 0x018)
#define BUSREC_CLRINTSTATUS   (BUSREC_BASE + 0x01C)

#define BUSREC_HIT_RADDR      (BUSREC_BASE + 0x020)
#define BUSREC_HIT_WADDR      (BUSREC_BASE + 0x028)
#define BUSREC_HIT_RADDR_MASK (BUSREC_BASE + 0x030)
#define BUSREC_HIT_WADDR_MASK (BUSREC_BASE + 0x038)

#define BUSREC_1STUNCOMPLET   (BUSREC_BASE + 0x384)

#define TPUSRAM_BASE          0x0C000000
#define BUSREC_INTNUM         182


u8 u8BusRec_INT = 0;
int BusRec_IRQCallBack(void)
{
  unsigned int u32Reg = 0;

  printf("BusRec_IRQCallBack() enter\n");
  printf("BusRec_IRQCallBack() mask IRQ\n");

  //print some infomation
  u32Reg = RAW_READ32(BUSREC_ENABLE);
  printf("BUSREC_ENABLE = 0x%x\n", u32Reg);

  u32Reg = RAW_READ32(BUSREC_HIT_RADDR);
  printf("BUSREC_HIT_RADDR = 0x%x\n", u32Reg);

  u32Reg = RAW_READ32(BUSREC_HIT_WADDR);
  printf("BUSREC_HIT_WADDR = 0x%x\n", u32Reg);

  u32Reg = RAW_READ32(BUSREC_INTSTATUS);
  printf("BUSREC_INTSTATUS = 0x%x\n", u32Reg);

  //clear interrput
  RAW_WRITE32(BUSREC_CLRINTSTATUS, BUSREC_INT_ALL);
  RAW_WRITE32(BUSREC_CLRINTSTATUS, BUSREC_INT_ZERO);
  printf("==BusRec_IRQCallBack clear done\n");
  u8BusRec_INT = 1;
  return 0;
}

int BusRec_MonitorMode(void)
{
  int i = 0;
  unsigned int unComplete0 = 0;
  /**********************************************************************/
  //Test case 1, enable bus recoder
  /**********************************************************************/
  //enable monitor mode
  RAW_WRITE32(BUSREC_ENABLE, BUSREC_TYPE_MON | BUSREC_TYPE_HANG_INT);

  //read 1st uncomplete address will generate 1st uncomplete transaction, so the result will be itself
  unComplete0 = RAW_READ32(BUSREC_1STUNCOMPLET);
  if (unComplete0 == BUSREC_1STUNCOMPLET)
    printf("[PASS] BusRec read 1st uncomplete address = 0x%x\r\n", unComplete0);
  else
    printf("[FAIL] BusRec read 1st uncomplete address = 0x%x\r\n", unComplete0);

#if 1
  //enable watchdog
  //BusTest_TriggerWDT();
  //if you want bus hang situation, just read TPU SRAM

  u32 regVal = RAW_READ32(0x03002000);
  regVal &= ~(0x1<<26);
  RAW_WRITE32(0x03002000, regVal);

  //enable watchdog
  //BusTest_TriggerWDT();

  RAW_READ32(0x04510000);

  //eth0 slave
  //0x03002000 &=~(0x1<<26)  // disable eth0 clock
  //access 0x04510000
#endif

	return 0;
}

int BusRec_HitMode(void)
{
  unsigned int u32status = 0;

#if 1
  /**********************************************************************/
  //Begin hit read mode
  /**********************************************************************/
  //enable hit read mode
  RAW_WRITE32(BUSREC_ENABLE, BUSREC_TYPE_MON | BUSREC_TYPE_HANG_INT | BUSREC_TYPE_HITR_INT);

  //set hit read address = BUSREC_ENABLE
  RAW_WRITE32(BUSREC_HIT_RADDR, BUSREC_ENABLE);

  //clear interrupt status
  RAW_WRITE32(BUSREC_CLRINTSTATUS, BUSREC_INT_ALL);
  RAW_WRITE32(BUSREC_CLRINTSTATUS, BUSREC_INT_ZERO);
  u32status = RAW_READ32(BUSREC_INTSTATUS);
  if (u32status != BUSREC_INT_ZERO)
    printf("[FAIL] after clear interrupt status = 0x%x\r\n", u32status);

  //read hit trigger and check final interrupt status
  RAW_READ32(BUSREC_ENABLE);
  while (u8BusRec_INT == 0)
    printf("read hit polling interrupt status = 0x%x\r\n", RAW_READ32(BUSREC_INTSTATUS));
#endif

#if 1
  /**********************************************************************/
  //Begin hit write mode
  /**********************************************************************/
  //clear interrupt variable for checking
  u8BusRec_INT = 0;

  //enable hit write mode
  RAW_WRITE32(BUSREC_ENABLE, BUSREC_TYPE_MON | BUSREC_TYPE_HANG_INT | BUSREC_TYPE_HITW_INT);

  //set hit write address = BUSREC_ENABLE
  RAW_WRITE32(BUSREC_HIT_WADDR, BUSREC_ENABLE);

  //clear interrupt status
  RAW_WRITE32(BUSREC_CLRINTSTATUS, BUSREC_INT_ALL);
  RAW_WRITE32(BUSREC_CLRINTSTATUS, BUSREC_INT_ZERO);
  u32status = RAW_READ32(BUSREC_INTSTATUS);
  if (u32status != 0)
    printf("[FAIL] after clear interrupt status = 0x%x\r\n", u32status);

  //write hit trigger check final interrupt status
  RAW_WRITE32(BUSREC_ENABLE, BUSREC_TYPE_MON | BUSREC_TYPE_HANG_INT | BUSREC_TYPE_HITW_INT);

  while (u8BusRec_INT == 0)
    printf("write hit polling interrupt status = 0x%x\r\n", RAW_READ32(BUSREC_INTSTATUS));

#endif
  return 0;
}

int BusRec_Main(void)
{
  //init interrupt
  enable_irq(BUSREC_INTNUM);
  request_irq(BUSREC_INTNUM, BusRec_IRQCallBack, 0, "BusRecInt", NULL);
  u8BusRec_INT = 0;

  //test monitor mode
  BusRec_MonitorMode();

  //test hit mode
  //BusRec_HitMode();

  return 0;
}
#endif

#ifndef BUILD_TEST_CASE_all
int testcase_main()
{
  //PC monitor test case
  //PCMon_Main();

  //BusRec_Main();
  //BusTest_Main();

  AXIMon_Main();
  //test_flatcc_main();
  return 0;
}
#endif

