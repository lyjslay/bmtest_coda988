/*
 * Copyright (C) 2018 Bitmain Technologies Ltd.
 *
 * SPDX-License-Identifier: GPL-2.0+
 */

#include <asm/encoding.h>
#include "io.h"
#include "system_common.h"
#include <ptrace.h>
#include "phobosrv_common.h"
#include "module_testcase.h"
#include "timer.h"
#include "mmio.h"
extern uint64_t __bss_end;
void irq_init(void);
int request_irq(unsigned int irqn, irq_handler_t handler, unsigned long flags,
                 const char *name, void *priv);
void riscv_start_timer(uint64_t interval);
void riscv_stop_timer(void);
uint64_t riscv_get_timer_count(void);

typedef struct {
    uint64_t    start;
    uint64_t    size;
    uint64_t    cacheable;
} mem_region_t;

#define CACHE_CYC           3
#define NON_CACHE_CYC      20

#define CACHEABLE           1
#define NON_CACHEABLE       0
#if 0
mem_region_t test_mem_region[] = {
	{0x03000000, 0x00400000, NON_CACHEABLE},
	{0x04400000, 0x00100000, CACHEABLE },
	{0x04500000, 0x000D0000, NON_CACHEABLE},
	{0x05200000, 0x00200000, CACHEABLE},
	{0x05400000, 0x06c00000, NON_CACHEABLE},
	{0x0C000000, 0x00040000, CACHEABLE},
	{0x0c040000, 0x73FC0000, NON_CACHEABLE},
	{0x80000000, 0x01000000, CACHEABLE},
};
#else
mem_region_t test_mem_region[] = {
	{0x03020000, 0x00004000, NON_CACHEABLE},
	{0x04400000, 0x00004000, CACHEABLE },
//	{0x04500000, 0x00004000, NON_CACHEABLE},
	{0x05200000, 0x00004000, CACHEABLE},
	{0x23020000, 0x00004000, NON_CACHEABLE},
	{0x3C000000, 0x00004000, CACHEABLE},
	{0x3BC00000, 0x00004000, CACHEABLE},
#ifdef CONFIG_C906B
	{0x70000000, 0x00000200, NON_CACHEABLE},
#elif defined (CONFIG_C906L)
	{0x70000000, 0x00000100, NON_CACHEABLE},
#endif
	{0x80000000, 0x00004000, CACHEABLE},
};

#endif

#define NOF_TEST_MEM_REGION (sizeof(test_mem_region)/sizeof(test_mem_region[0]))

typedef struct {
  volatile uint32_t* ipi;
  volatile int mipi_pending;

  volatile uint64_t* timecmp;
  volatile uint64_t* mtime;

  volatile uint32_t* plic_m_thresh;
  volatile uintptr_t* plic_m_ie;
  volatile uint32_t* plic_s_thresh;
  volatile uintptr_t* plic_s_ie;
} hls_t;

typedef struct _test_data_t {
    uint64_t    start_cyc;
    uint64_t    timer_interval;
    uint64_t    timer_intr_cnt;
    uint64_t    intr_cnt;
    uint32_t    intrid;
    uint32_t    last_intrid;

    uint64_t    mem_start;
    uint64_t    mem_end;
    uint64_t    cpu_start;
    uint64_t    cpu_end;
    uint64_t    cpu_src;
    uint64_t    cpu_dst;
    uint64_t    dma_start;
    uint64_t    dma_end;
    uint64_t    dma_src;
    uint64_t    dma_dst;
    uint64_t    total_size;
    uint32_t    rw_size;
    uint32_t    rw_offset;
    uint32_t    gdma_forever;
    uint32_t    gdma_restart_cnt;
    void (*intr_handler)(int intrid, void *priv);
} test_data_t;

static volatile test_data_t g_test_data;

// Supervisor mode will trap into illegal instruction in machine mode
static inline uint64_t get_cycles64(void)
{
    uint64_t cyc;
    asm volatile("rdcycle %0" : "=r" (cyc));
    return cyc;
}
static int bm_riscv_cycle_test(void)
{
    int ret = 0;
    uint64_t t1, t2;

    printf("\n    cycle test\n");

    t1 = get_cycles64();
    t2 = get_cycles64();

    if (!t1 || !t2 || t2 <= t1) {
        printf("      cycle not increase t1=0x%lx, t2=0x%lx\n", t1, t2);
        ret = 1;
    }

    printf("        => %s\n", ret?"fail":"pass");

    return ret;
}

void riscv_timer_interrupt(void)
{
//    printf("riscv_timer_interrupt\n");
    g_test_data.timer_intr_cnt++;
}

#if 0
static void bm_riscv_start_timer(uint64_t interval)
{
    hls_t hls;

    /* Sifive E51-RVCoreIP, CLINT */
    hls.timecmp = (volatile u64 *)CLINT_TIMECMP;
    hls.mtime = (volatile u64 *)CLINT_MTIME;

    /* Enable timer interrupt */
    set_csr(mstatus, MSTATUS_MIE);
    set_csr(mie, MIP_MTIP);
}

static void bm_riscv_stop_timer(void)
{
    /* Stop timer and disable timer interrupt */
    clear_csr(mie, MIP_MTIP);
    clear_csr(mip, MIP_MTIP);
}
#endif

#if 0
static void gdma_memcpy(uint64_t dst, uint64_t src, uint32_t size)
{
    uint64_t nc_src, nc_dst;

    nc_src = NC_SDRAM + (src - PHYS_SDRAM_1);
    nc_dst = NC_SDRAM + (dst - PHYS_SDRAM_1);
    //printf("      gdma_memcpy(0x%llx, 0x%llx, 0x%x)\n", nc_dst, nc_src, size);

    mmio_wr32(GDMA_CMD_ACCP1,  0x00004088);

    /* 0x5_0000_000 <-> 0x1_000_0000(mirror, RISCV) */
    mmio_wr32(GDMA_CMD_ACCP13,
              (((nc_src>>32)&0xffff)<<24) |   // SRC[39:32]
              (((nc_dst>>32)&0xffff)<<16) |   // DST[39:32]
              1);                             // Blob count 1
    mmio_wr32(GDMA_CMD_ACCP12, src);          // SRC[31:0]
    mmio_wr32(GDMA_CMD_ACCP11, dst);          // DST[31:0]

    mmio_wr32(GDMA_CMD_ACCP8,  0x00080000);   // source blob C = 8
    mmio_wr32(GDMA_CMD_ACCP10, 0x0014000f);   // source blob W = 15
                                              // source blob H = 20
    mmio_wr32(GDMA_CMD_ACCP4,  0x012c000f);   // dst blob H stride = 15
                                              // dst blob C stride = 300
    mmio_wr32(GDMA_CMD_ACCP5,  0x00000960);   // source blob H stride = 96
    mmio_wr32(GDMA_CMD_ACCP6,  0x012c000f);   // source blob H stride = 15
                                              // source blob C stride 300
    mmio_wr32(GDMA_CMD_ACCP14, 0x00000000);   // src C stride[31:16]
                                              // dst C stride[31:16]
    mmio_wr32(GDMA_CMD_ACCP7,  size);         // transfer length
    mmio_wr32(GDMA_CMD_ACCP2,  0x00000000);
    mmio_wr32(GDMA_CMD_ACCP0,  0x7ffd04bd);   // descriptor valid
                                              // no next descriptor addr ptr
                                              // EOD
                                              // interrupt enable
                                              // enable stride for all blob
                                              // system memory to system memory
                                              // common dma
    mmio_wr32(GDMA_DMA_CTRL,   0x00000101);   // kick
}
#endif

extern timer_hls_t thls;
int bm_riscv_timer_test(void)
{
    int ret = 0;
    uint64_t mstatus, mcause, interval, timer_count, end_cyc;
    uint16_t mip, mie;

    printf("\n    timer test\n");
    // callback function for timer interrupt
    thls.intr_handler = riscv_timer_interrupt;

    interval = CPU_TIMER_CLOCK/TIMER_HZ;     // 1 ms
    g_test_data.timer_intr_cnt = 0;

    riscv_start_timer(interval);
    g_test_data.start_cyc = get_cycles64();

    while (g_test_data.timer_intr_cnt!=1000) ; // 1 sec
    end_cyc = get_cycles64();
    timer_count = riscv_get_timer_count();
    printf("timer count=%ld\n", timer_count);
    printf("cpu tick = %ld\n", end_cyc - g_test_data.start_cyc);
    /* Stop timer and disable timer interrupt */
    riscv_stop_timer();

    mstatus = read_csr(mstatus);
    mcause = read_csr(mcause);
    mie = read_csr(mie);
    mip = read_csr(mip);
    printf("      mstatus=0x%lx, mcause=0x%lx, mie=0x%x, mip=0x%x\n",
           mstatus, mcause, mie, mip);

    ret = g_test_data.timer_intr_cnt ? 0 : 1;

    printf("        => %s\n", ret?"fail":"pass");

    return ret;
}

static int bm_riscv_wdt_intr_handler(int intrid, void *priv)
{
//    printf("      wdt_intr_handler: wdt_stat=0x%x\n",
//           mmio_read_32(WDT_BASE+0x14));
    g_test_data.intr_cnt++;
    g_test_data.last_intrid = intrid;
#ifdef CONFIG_C906B
    if (intrid != WDT1_INTR)
        printf("      * error ! intrd=%d != WDT1_INTR(%d)\n", intrid, WDT1_INTR);
#elif defined(CONFIG_C906L)
    if (intrid != WDT2_INTR)
        printf("      * error ! intrd=%d != WDT2_INTR(%d)\n", intrid, WDT1_INTR);
#endif
    mmio_write_32(WDT_BASE+0xc, 0x76);  // restart to clear intr
    mmio_write_32(WDT_BASE, 0x0);       // disable wdt, counter keep incr
    printf("      wdt_intr_handler: wdt_stat=0x%x\n",
           mmio_read_32(WDT_BASE+0x14));

    return 0;
}

static int bm_riscv_ext_intr_wdt_test(void)
{
    int ret=0;
    printf("\n    wdt intr test\n");

    g_test_data.last_intrid = 0;
    g_test_data.intrid = 0;
    g_test_data.intr_cnt = 0;

#ifdef CONFIG_C906B
    request_irq(WDT1_INTR, bm_riscv_wdt_intr_handler,0,"WDT INT",0);
#elif defined(CONFIG_C906L)
    request_irq(WDT2_INTR, bm_riscv_wdt_intr_handler,0,"WDT INT",0);
#endif
    // trigger wdt interrupt
    mmio_write_32(WDT_BASE+4, 0x20);
    mmio_write_32(WDT_BASE+0xc, 0x76);
    mmio_write_32(WDT_BASE, 0x13);

    while (!g_test_data.intr_cnt) ;

    printf("      mstatus=0x%lx, mcause=0x%lx, mie=0x%lx, mip=0x%lx\n",
           read_csr(mstatus), read_csr(mcause), read_csr(mie), read_csr(mip));

    ret = g_test_data.timer_intr_cnt ? 0 : 1;
    printf("        => %s\n", ret?"fail":"pass");

    return ret;
}

#if 0
static int bm_riscv_ext_intr_uart_test(void)
{
    uint64_t i;
    int intrid = UART0_INTR;
    int polling = 1;

    printf("\n    ext intr UART test\n");

    // Clean the priority of all IRQ
    for (i = 0; i < 256 * 4; i = i + 4) {
        writel(0, ((void *)PLIC_PRIORITY0 + i));
    }
    // Enable UART0 interrupt (INT41)
    writel(1, ((void *)PLIC_PRIORITY0 + intrid * 4));
    writel(0, (void *)PLIC_THRESHOLD);
    writel((0x1 << (intrid%32)), ((void *)PLIC_ENABLE1+(intrid/32)*4));

    for (intrid=0; intrid<128; intrid++) {
        writel(1, ((void *)PLIC_PRIORITY0 + intrid * 4));
        writel((0x1 << (intrid%32)), ((void *)PLIC_ENABLE1+(intrid/32)*4));
    }

    // UAR0 RX interrupt
    mmio_wr32(UART0_BASE+4, 5);

    if (!polling) {
        // enable externel interrupt
        set_csr(sie, MIP_SEIP);
        set_csr(sstatus, SSTATUS_SIE);
    }

    if (polling) {
        printf("      sie=0x%lx, sip=0x%lx, sstatus=0x%lx, scause=0x%lx\n",
               read_csr(sie), read_csr(sip),
               read_csr(sstatus), read_csr(scause));
        printf("      [0x%08x]=0x%08x\n",
               PLIC_PENDING1+4*(intrid/32),
               readl((void *)PLIC_PENDING1+4*(intrid/32)));

        //for (i=UART0_BASE; i<UART0_BASE+64; i+=4)
        //    printf("      [0x%08x]=0x%08x\n", i, readl(i));
    }

    // Wait uart0 rx
    printf("      wait user input\n");

    // Check SEIP
    while ((read_csr(sip) & (1<<9)) == 0) ;

    if (polling) {
        printf("      sie=0x%lx, sip=0x%lx, sstatus=0x%lx, scause=0x%lx\n",
               read_csr(sie), read_csr(sip),
               read_csr(sstatus), read_csr(scause));
        printf("      [0x%08x]=0x%08x\n",
               PLIC_PENDING1+4*(intrid/32),
               readl((void *)PLIC_PENDING1+4*(intrid/32)));
        //for (i=UART0_BASE; i<UART0_BASE+64; i+=4)
        //    printf("      [0x%08x]=0x%08x\n", i, readl(i));
    }

    return g_test_data.intr_cnt ? 0 : 1;
}

static int bm_riscv_bus_slave_test(void)
{
    uint64_t i, j, k, start, end;
    volatile int val;

    printf("\n    bus slave test\n");

    // Scan hw register
    /* 0x70000000 mirror of 0x50000000 */
    for (i = 0; i<2; i++) {
        for (j = 0; j < NOF_TEST_MEM_REGION; j++) {
            start = test_mem_region[j].start + i*0x20000000;
            end = test_mem_region[j].end + 1 + i*0x20000000;
            printf("      scan 0x%08llx\n", start);

            /* Read 4B per 64B to reduce test time */
            for (k = start; k < end; k+=64) {
                val = readl((void *)k);

                if ((k%0x1000000) == 0)
                    printf("      0x%08llx, 0x%x\n", k, val);

            }
        }
    }

    return 0;
}
static int bm_riscv_mem_rw_test_proc(uint64_t rw_size, uint64_t mem_start, uint64_t total_size)
{
    int cpu_fail=0, dma_fail=0;
    int intrid = INTRID_GDMA;
    uint8_t gdma_int_stat;
    uint32_t exp_val, val;
    uint64_t i, j, rw_offset;
    uint64_t mem_end, start_cyc, elapsed_cyc, timer_interval;

    start_cyc = get_cycles64();
    g_test_data.start_cyc = start_cyc;
    timer_interval = CPU_TIMER_CLOCK/200;   // 5 ms
    g_test_data.timer_interval = timer_interval;
    g_test_data.timer_intr_cnt = 0;
    g_test_data.intr_cnt = 0;
    g_test_data.intrid = intrid;
    g_test_data.last_intrid = 0;

    rw_offset = 2*rw_size;
    mem_end = mem_start + total_size;
    g_test_data.mem_start = mem_start;
    g_test_data.mem_end = mem_end;
    g_test_data.cpu_start = mem_start;
    g_test_data.cpu_end = mem_end - rw_size;
    g_test_data.dma_start = mem_start + rw_size;
    g_test_data.dma_end = mem_end;

    /* Memory layout */
    /* CPU src 0  4   8 12  ... */
    /* DMA src 1  5   9 13  ... */
    /* CPU dst 2  6  10 14  ... */
    /* DMA dst 3  7  11 15  ... */
    g_test_data.cpu_src = g_test_data.cpu_start;
    g_test_data.cpu_dst = g_test_data.cpu_src + rw_offset;
    g_test_data.dma_src = g_test_data.dma_start;
    g_test_data.dma_dst = g_test_data.dma_src + rw_offset;
    g_test_data.rw_size = rw_size;
    g_test_data.rw_offset = rw_offset;
    g_test_data.total_size = total_size;
    g_test_data.gdma_forever = 0;
    g_test_data.gdma_restart_cnt = 1;
    g_test_data.intr_handler = bm_riscv_gdma_intr_handler;

    printf("      mem_start=0x%llx, mem_end=0x%llx, mem_size=0x%llx, rw_size=0x%x\n",
           g_test_data.mem_start, g_test_data.mem_end, g_test_data.total_size, g_test_data.rw_size);
    printf("      cpu_start=0x%llx, cpu_end=0x%llx\n", g_test_data.cpu_start, g_test_data.cpu_end);
    printf("      dma_start=0x%llx, dma_end=0x%llx\n", g_test_data.dma_start, g_test_data.dma_end);

    /* Clear all memory content */
    printf("      Reset all memory content\n");
    memset((void *)g_test_data.mem_start, 0xff, g_test_data.total_size);

    /* Fill CPU source content */
    printf("      Fill cpu source\n");
    for (i=g_test_data.cpu_start; i<g_test_data.cpu_end; i+=2*g_test_data.rw_offset) {
        //printf("        [0x%llx]\n", i);
        for (j=0; j<g_test_data.rw_size; j+=8)
            writeq((rw_size<<48)|(i+j), (void *)i+j);
    }

    // Fill GDMA source content
    printf("      Fill dma source\n");
    for (i=g_test_data.dma_start; i<g_test_data.dma_end; i+=2*g_test_data.rw_offset) {
        //printf("        [0x%llx]\n", i);
        for (j=0; j<g_test_data.rw_size; j+=8)
            writeq((rw_size<<48)|(i+j), (void *)i+j);
    }

    // Clean the priority of all IRQ
    i = PLIC_PRIORITY0;
    for (i = 0; i < 256 * 4; i = i + 4) {
        mmio_wr32((PLIC_PRIORITY0 + i), 0);
    }
    // Enable GDMA interrupt (INT112)
    writel(1, ((void *)PLIC_PRIORITY0 + intrid * 4));
    writel(0, (void *)PLIC_THRESHOLD);
    writel((0x1 << (intrid%32)), ((void *)PLIC_ENABLE1+(intrid/32)*4));

    //printf("      before sie, sstatus=0x%lx, scause=0x%lx, sie=0x%lx, sip=0x%lx\n",
    //       read_csr(sstatus), read_csr(scause), read_csr(sie), read_csr(sip));

    //val = readl(GDMA_INT_MASK_STS);
    //printf("      before kick gdma 0x%x intr_mask=0x%02x, intr_stat=0x%02x, gen_sta=0x%04x\n",
    //       val, val&0xff, (val>>8)&0xff, (val>>16)&0xffff);

    /* Start timer */
    bm_riscv_start_timer(g_test_data.timer_interval);

    // Enable riscv externel interrupt
    set_csr(mie, MIP_MEIP);
    set_csr(mstatus, MSTATUS_MIE);

    gdma_memcpy(g_test_data.dma_dst, g_test_data.dma_src, g_test_data.rw_size);

    /* CPU memory copy */
    printf("      cpu memcpy ...\n");
    while (g_test_data.cpu_dst < g_test_data.cpu_end) {
        //printf("      memcpy: cpu_src=0x%llx, cpu_dst=0x%llx\n",
        //       g_test_data.cpu_src, g_test_data.cpu_dst);

        memcpy((void *)g_test_data.cpu_dst, (void *)g_test_data.cpu_src, g_test_data.rw_size);

        g_test_data.cpu_src = g_test_data.cpu_dst + g_test_data.rw_offset;
        g_test_data.cpu_dst = g_test_data.cpu_src + g_test_data.rw_offset;

        /* random timer */
        g_test_data.timer_interval = timer_interval + (get_cycles64()%timer_interval);
    }

    /* Check CPU memcpy */
    printf("      Check cpu mem content:\n");
    g_test_data.cpu_src = g_test_data.cpu_start;
    g_test_data.cpu_dst = g_test_data.cpu_src + rw_offset;
    while (g_test_data.cpu_dst < g_test_data.cpu_end) {
        //printf("        cpu_src=0x%llx, cpu_dst=0x%llx\n", g_test_data.cpu_src, g_test_data.cpu_dst);
        for (i=0; i<g_test_data.rw_size; i+=4) {
            exp_val = readl((void *)g_test_data.cpu_src+i);
            val = readl((void *)g_test_data.cpu_dst+i);
            if (exp_val != val) {
                printf("        cpu [0x%llx]=0x%x != [0x%llx]=0x%x\n",
                       g_test_data.cpu_src+i, exp_val, g_test_data.cpu_dst+i, val);
                cpu_fail = 1;
                break;
            }
        }

        g_test_data.cpu_src = g_test_data.cpu_dst + g_test_data.rw_offset;
        g_test_data.cpu_dst = g_test_data.cpu_src + g_test_data.rw_offset;
    }
    printf("        => %s\n", cpu_fail?"fail":"pass");

    //printf("      sstatus=0x%lx, scause=0x%lx, sie=0x%lx, sip=0x%lx\n",
    //       read_csr(sstatus), read_csr(scause), read_csr(sie), read_csr(sip));

    /* Stop timer */
    bm_riscv_stop_timer();

    /* Stop gdma retry */
    g_test_data.gdma_forever = 0;

    /* Wait DMA finished */
    printf("      dma_src=0x%llx, dma_dst=0x%llx\n", g_test_data.dma_src, g_test_data.dma_dst);
    for (i = 0; i < 1000; i++) {
        val = readl((void *)GDMA_INT_MASK_STS);
        gdma_int_stat = (val >> 8)&0xff;
        if (!gdma_int_stat)
            break;
    }
    printf("      gdma_int_stat=%d, retry_count=%lld\n", gdma_int_stat, i);

    /* Disable interrupt enable */
    write_csr(sie, 0);

    /* Check DMA copy */
    printf("      Check dma mem content(restart_cnt=%d):\n", g_test_data.gdma_restart_cnt);
    g_test_data.dma_src = g_test_data.dma_start;
    g_test_data.dma_dst = g_test_data.dma_src + rw_offset;
    while (g_test_data.dma_dst < g_test_data.dma_end) {
        //printf("        dma_src=0x%llx, dma_dst=0x%llx\n", g_test_data.dma_src, g_test_data.dma_dst);
        for (i=0; i<g_test_data.rw_size; i+=4) {
            exp_val = readl((void *)g_test_data.dma_src+i);
            val = readl((void *)g_test_data.dma_dst+i);
            if (exp_val != val) {
                printf("        dma [0x%llx]=0x%x != [0x%llx]=0x%x\n",
                       g_test_data.dma_src+i, exp_val, g_test_data.dma_dst+i, val);
                dma_fail = 1;
                break;
            }
        }

        if (dma_fail)
            break;

        g_test_data.dma_src = g_test_data.dma_dst + g_test_data.rw_offset;
        g_test_data.dma_dst = g_test_data.dma_src + g_test_data.rw_offset;
    }
    printf("        => %s\n", dma_fail?"fail":"pass");

    elapsed_cyc = get_cycles64() - start_cyc;

    printf("\n        => duration=0x%llx cycle, %lld sec\n",
           elapsed_cyc, elapsed_cyc/CPU_CLOCK);
    printf("        => timer_intr_cnt=%lld, gdma_intr_cnt=%lld\n\n",
           g_test_data.timer_intr_cnt, g_test_data.intr_cnt);

    return (cpu_fail||dma_fail) ? 1 : 0;
}
static int bm_riscv_mem_rw_test(void)
{
    int ret = 0;
    uint64_t rw_size, mem_start, total_size, start_cyc, elapsed_cyc;

    printf("\n    memory read/write test\n");

    start_cyc = get_cycles64();

    mem_start = PHYS_SDRAM_2;
    total_size = /*512*1024*1024*/PHYS_SDRAM_2_SIZE;
    for (rw_size=64; rw_size<total_size/4; rw_size*=2) {
        ret = bm_riscv_mem_rw_test_proc(rw_size, mem_start, total_size);
        if (ret)
            break;
    }

    elapsed_cyc = get_cycles64() - start_cyc;

    printf("\n    => duration=0x%llx cycle, %lld sec\n\n",
           elapsed_cyc, elapsed_cyc/CPU_CLOCK);

    return ret;
}
#endif

#pragma pack(push)
#pragma GCC optimize("-O2")

#define RUN_LOOPS 100
int bm_riscv_sysmap_test()
{
    long start;//, size;
    long start_cyc, end_cyc;
    int step;
    int time;
    int value = 0;
    int ret = 0;

    printf("bm_riscv_sysmap_test\n");
    inv_clean_dcache_all();
    dcache_enable();
    for (int i = 0; i < NOF_TEST_MEM_REGION; i++) {

        inv_clean_dcache_all();
        start = test_mem_region[i].start;
        printf("\nstart[%d] addr=%lx\n", i, start);
        start_cyc = get_cycles64();
        for (int loops = 0; loops < RUN_LOOPS; loops++) {
            for (int j = 0; j < test_mem_region[i].size; j += 16 * sizeof(int)) {
                value = *((volatile int *) (start + j + sizeof(int) * 0));
                value = *((volatile int *) (start + j + sizeof(int) * 1));
                value = *((volatile int *) (start + j + sizeof(int) * 2));
                value = *((volatile int *) (start + j + sizeof(int) * 3));
                value = *((volatile int *) (start + j + sizeof(int) * 4));
                value = *((volatile int *) (start + j + sizeof(int) * 5));
                value = *((volatile int *) (start + j + sizeof(int) * 6));
                value = *((volatile int *) (start + j + sizeof(int) * 7));
                value = *((volatile int *) (start + j + sizeof(int) * 8));
                value = *((volatile int *) (start + j + sizeof(int) * 9));
                value = *((volatile int *) (start + j + sizeof(int) * 10));
                value = *((volatile int *) (start + j + sizeof(int) * 11));
                value = *((volatile int *) (start + j + sizeof(int) * 12));
                value = *((volatile int *) (start + j + sizeof(int) * 13));
                value = *((volatile int *) (start + j + sizeof(int) * 14));
                value = *((volatile int *) (start + j + sizeof(int) * 15));
            }
        }
        end_cyc = get_cycles64();
        printf("  value=%d\n", value);
        printf("  end=%ld start=%ld \n", end_cyc, start_cyc);
        printf("  size=%lx\n", test_mem_region[i].size * RUN_LOOPS);
        step = (test_mem_region[i].size * RUN_LOOPS) / sizeof(int);
        printf("  step=%x\n", step);
        time = (int)((end_cyc - start_cyc) / step);
        printf("   region[%d] start:%lx size:%lx time:%d\n", i, test_mem_region[i].start, test_mem_region[i].size, time);
        if (test_mem_region[i].cacheable) {
            if (time > CACHE_CYC) {
                printf("-----------------------------------------------------------\n");
                printf("    test[%d] cache NG, run time: %d > set time %d \n", i, time, CACHE_CYC);
                printf("-----------------------------------------------------------\n");
                ret++;
			} else
                printf("    test[%d] cache OK, run time: %d < set time %d \n", i, time, CACHE_CYC);
        } else {
            if (time < NON_CACHE_CYC) {
                printf("-----------------------------------------------------------\n");
                printf("    test[%d] non-cache NG, run time: %d < set time %d \n", i, time, NON_CACHE_CYC);
                printf("-----------------------------------------------------------\n");
                ret++;
			} else
                printf("    test[%d] non-cache OK, run time: %d > set time %d \n", i, time, NON_CACHE_CYC);
        }
    }
    dcache_disable();
    printf("test done\n");
    return ret;
}
#pragma pack(pop)

int bm_riscv_test(void)
{
    int ret = 0;

    printf("cvitek riscv test\n");
    ret += bm_riscv_cycle_test();
//    ret += bm_riscv_mem_rw_test();
    ret += bm_riscv_timer_test();
    ret += bm_riscv_sysmap_test();
    ret += bm_riscv_ext_intr_wdt_test();  // need to kick afer test
    printf("test done %s\n", ret? "FAIL": "OK");

    return 0;
}

#ifndef BUILD_TEST_CASE_all
int testcase_main(void)
{
	return bm_riscv_test();
}
#endif

module_testcase("0", bm_riscv_test);
