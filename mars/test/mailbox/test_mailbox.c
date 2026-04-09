#include <asm/encoding.h>
#include "io.h"
#include "system_common.h"
#include <ptrace.h>
#include "marsrv_common.h"
#include "module_testcase.h"
#include "timer.h"
#include "mmio.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int request_irq(unsigned int irqn, irq_handler_t handler, unsigned long flags,
                 const char *name, void *priv);

#define debud_printf(...) {}

//#define RTC_SYS

#ifdef RTC_SYS
#define MAILBOX_BASE                           0x5024000
#define MAILBOX_MAX_NUM                              0x4
#define MAILBOX_CONTEXT        (MAILBOX_BASE    + 0x0400)
#define MAILBOX_CONTEXT_COMM   (MAILBOX_CONTEXT + 0x0010)
#else
#define MAILBOX_BASE                           0x1900000
#define MAILBOX_MAX_NUM                              0x8
#define MAILBOX_CONTEXT        (MAILBOX_BASE    + 0x0400)
#define MAILBOX_CONTEXT_COMM   (MAILBOX_CONTEXT + 0x0020)
#endif

#define MAILBOX_CPU0_EN   (MAILBOX_BASE + 0x0000)
#define MAILBOX_CPU1_EN   (MAILBOX_BASE + 0x0004)
#define MAILBOX_CPU2_EN   (MAILBOX_BASE + 0x0008)
#define MAILBOX_CPU3_EN   (MAILBOX_BASE + 0x000C)

#define MAILBOX_CPU0_CLR  (MAILBOX_BASE + 0x0010)
#define MAILBOX_CPU0_MSK  (MAILBOX_BASE + 0x0014)
#define MAILBOX_CPU0_INT  (MAILBOX_BASE + 0x0018)
#define MAILBOX_CPU0_RAW  (MAILBOX_BASE + 0x001C)

#define MAILBOX_CPU1_CLR  (MAILBOX_BASE + 0x0020)
#define MAILBOX_CPU1_MSK  (MAILBOX_BASE + 0x0024)
#define MAILBOX_CPU1_INT  (MAILBOX_BASE + 0x0028)
#define MAILBOX_CPU1_RAW  (MAILBOX_BASE + 0x002C)

#define MAILBOX_CPU2_CLR  (MAILBOX_BASE + 0x0030)
#define MAILBOX_CPU2_MSK  (MAILBOX_BASE + 0x0034)
#define MAILBOX_CPU2_INT  (MAILBOX_BASE + 0x0038)
#define MAILBOX_CPU2_RAW  (MAILBOX_BASE + 0x003C)

#define MAILBOX_SET       (MAILBOX_BASE + 0x0060)

#define MAILBOX_STATUS0   (MAILBOX_BASE + 0x0070)
#define MAILBOX_STATUS1   (MAILBOX_BASE + 0x0074)
#define MAILBOX_STATUS2   (MAILBOX_BASE + 0x0078)
#define MAILBOX_STATUS3   (MAILBOX_BASE + 0x007C)

#define INTRID_MAILBOX_CPU0    MBOX_INT_CA53
#define INTRID_MAILBOX_CPU1    MBOX_INT_C906
#define INTRID_MAILBOX_CPU2    MBOX_INT_C906_2ND

#define TEST_CPU0        0x000000
#define TEST_CPU1        0x000001
#define TEST_CPU2        0x000002
#define TEST_CPU_FAIL    0xFFFFFF

#define TEST_SEND        0x000100
#define TEST_RECV        0x000200
#define TEST_DONE        0x000300
#define TEST_MBOX        0x010000

#ifdef RTC_SYS
#define MAILBOX_SELF_TEST_INT          MBOX_INT1
#else
#if defined(__arm__) || defined(__aarch64__) || defined(_M_ARM) || defined(_M_ARM64)
#define MAILBOX_SELF_TEST_INT          INTRID_MAILBOX_CPU0
#elif defined CONFIG_C906B
#define MAILBOX_SELF_TEST_INT          INTRID_MAILBOX_CPU1
#elif defined CONFIG_C906L
#define MAILBOX_SELF_TEST_INT          INTRID_MAILBOX_CPU2
#else
#error "not define MAILBOX_SELF_TEST_CPU"
#endif
#endif

union cpu_mailbox_info_offset{
	char mbox_info;
	int reserved;
};

union cpu_mailbox_int_clr_offset{
	char mbox_int_clr;
	int reserved;
};
union cpu_mailbox_int_mask_offset{
	char mbox_int_mask;
	int reserved;
};
union cpu_mailbox_int_offset{
	char mbox_int;
	int reserved;
};
union cpu_mailbox_int_raw_offset{
	char mbox_int_raw;
	int reserved;
};

union mailbox_set{
	char mbox_set;
	int reserved;
};
union mailbox_status{
	char mbox_status;
	int reserved;
};

union cpu_mailbox_status{
	char mbox_status;
	int reserved;
};

/* register mapping refers to mailbox user guide*/
struct cpu_mbox_int{
	union cpu_mailbox_int_clr_offset  cpu_mbox_int_clr;
	union cpu_mailbox_int_mask_offset cpu_mbox_int_mask;
	union cpu_mailbox_int_offset      cpu_mbox_int_int;
	union cpu_mailbox_int_raw_offset  cpu_mbox_int_raw;
};

struct mailbox_set_register{
	union  cpu_mailbox_info_offset cpu_mbox_en[4];
	struct cpu_mbox_int cpu_mbox_set[4];
	int    reserved[4];
	union  mailbox_set mbox_set;
	union  mailbox_status mbox_status;
	union  cpu_mailbox_status cpu_mbox_status[4];
};

struct mailbox_done_register{
	union  cpu_mailbox_info_offset cpu_mbox_done_en[4];
	struct cpu_mbox_int cpu_mbox_done[4];
};

struct mailbox_set_register *mbox_reg;
struct mailbox_done_register *mbox_done_reg;
unsigned int *mailbox_context; // 32 Bytes
unsigned int *mailbox_comm;  // 32 Bytes

int cvi_mailbox_irq_handler(int irq, void* priv)
{
	debud_printf("\ncvi_mailbox_irq_handler\n");
	int i;
	char set_val, done_val;
	unsigned long *ptr = (unsigned long *)priv;
	int master_cpu = *ptr;
	debud_printf("cpu = %d\n", master_cpu);
	debud_printf("MAILBOX_STATUS1 =%x\n", mmio_read_32(MAILBOX_STATUS1));
	debud_printf("MAILBOX_CPU1_INT=%x\n", mmio_read_32(MAILBOX_CPU1_INT));
	debud_printf("MAILBOX_CPU1_RAW=%x\n", mmio_read_32(MAILBOX_CPU1_RAW));
	set_val = mbox_reg->cpu_mbox_set[master_cpu].cpu_mbox_int_int.mbox_int;
	done_val = mbox_done_reg->cpu_mbox_done[master_cpu].cpu_mbox_int_int.mbox_int;
	debud_printf("set_val=%x\n", set_val);
	debud_printf("done_val=%x\n", done_val);

	if (set_val) {
		for(i = 0; i < MAILBOX_MAX_NUM; i++) {
			if ((set_val >> i ) & 1 ) {
				debud_printf("receive mailbox context[%d] = %x\n", i, mailbox_context[i]);
				mailbox_context[i] = master_cpu | TEST_RECV | (TEST_MBOX * i); 
			}
		}
		mbox_reg->cpu_mbox_set[master_cpu].cpu_mbox_int_clr.mbox_int_clr = set_val;
		debud_printf("int_clr = %lx\n", (unsigned long) &mbox_reg->cpu_mbox_set[master_cpu].cpu_mbox_int_clr.mbox_int_clr);
	}
	if (done_val) {
		for(i = 0; i < MAILBOX_MAX_NUM; i++) {
			if ((done_val >> i ) & 1 ) {
				debud_printf("receive mailbox context[%d] = %x\n", i, mailbox_context[i]);
				mailbox_context[i] = master_cpu | TEST_DONE | (TEST_MBOX * i);
			}
		}
		mbox_done_reg->cpu_mbox_done[master_cpu].cpu_mbox_int_clr.mbox_int_clr = done_val;
		debud_printf("done_int_clr=%lx\n", (unsigned long) &mbox_done_reg->cpu_mbox_done[master_cpu].cpu_mbox_int_clr.mbox_int_clr);
	}
	return 0;
}

static int cpu;
int cvi_mailbox_self_test(int master_cpu)
{
	int i, j, k, mask;
	cpu = master_cpu;

	printf("cvi_mailbox_self_test CPU%d intr=%d\n", master_cpu, MAILBOX_SELF_TEST_INT);
	request_irq(MAILBOX_SELF_TEST_INT, cvi_mailbox_irq_handler , 0, "MAILBOX", &cpu);

	// sequentially send mailbox to itself
	for (i = 0; i < MAILBOX_MAX_NUM; i++) {

		// init fill context
		for (k = 0; k < MAILBOX_MAX_NUM; k++)
			mailbox_context[k] = -1;

		mailbox_context[i] = master_cpu | TEST_SEND | (TEST_MBOX * i);

		// Clear & Enable CPU Mailbox
		// send mailbox to specific MAILBOX_SELF_TEST_CPU
		mbox_reg->cpu_mbox_set[master_cpu].cpu_mbox_int_clr.mbox_int_clr = 1 << i;
		mbox_reg->cpu_mbox_en[master_cpu].mbox_info = 1 << i;
		// Clear & Enable inform back
		// inform back to specific MAILBOX_SELF_TEST_CPU
		mbox_done_reg->cpu_mbox_done[master_cpu].cpu_mbox_int_clr.mbox_int_clr = 1 << i;
		mbox_done_reg->cpu_mbox_done_en[master_cpu].mbox_info = 1 << i;

		// Enable APSYS mailbox
		mbox_reg->mbox_set.mbox_set = 1 << i;
		do {
		} while(mailbox_context[i] != (master_cpu | TEST_DONE | (TEST_MBOX * i)));

		printf("MAILBOX_SET CPU%d single mailbox [%x] done\n", master_cpu, i);
		// Disable CPU Mailbox
		mbox_reg->cpu_mbox_en[master_cpu].mbox_info = 0;
		mbox_done_reg->cpu_mbox_done_en[master_cpu].mbox_info = 0;
	}

	// randomly send mailbox to itself
	for (j = 0; j < 0x100; j++) {
		// init fill context
		for (k = 0; k < MAILBOX_MAX_NUM; k++)
			mailbox_context[k] = -1;

		i = rand() % 0x100;
		for (k = 0; k < MAILBOX_MAX_NUM; k++)
			if ((i >> k) & 1)
				mailbox_context[k] = master_cpu | TEST_SEND | (TEST_MBOX * k);

		// Clear & Enable CPU1 Mailbox
		mbox_reg->cpu_mbox_set[master_cpu].cpu_mbox_int_clr.mbox_int_clr = i;
		mbox_reg->cpu_mbox_en[master_cpu].mbox_info = i;

		// Clear & Enable inform back
		mbox_done_reg->cpu_mbox_done[master_cpu].cpu_mbox_int_clr.mbox_int_clr = i;
		mbox_done_reg->cpu_mbox_done_en[master_cpu].mbox_info = i;

		// Enable APSYS mailbox
		mbox_reg->mbox_set.mbox_set = i;

		for (k = 0; k < MAILBOX_MAX_NUM; k++)
			if ((i >> k) & 1)
				while(mailbox_context[k] != (master_cpu | TEST_DONE | (TEST_MBOX * k)));
		printf("MAILBOX_SET CPU%d multi send[%d] mailbox[%x] done\n", master_cpu, j,i);
		// Disable CPU Mailbox
		mbox_reg->cpu_mbox_en[master_cpu].mbox_info = 0;
		mbox_done_reg->cpu_mbox_done_en[master_cpu].mbox_info = 0;
	}

	// randomly send & mask mailbox to itself
	for (j = 0; j < 0x100; j++) {
		// init fill context
		for (k = 0; k < MAILBOX_MAX_NUM; k++)
			mailbox_context[k] = -1;

		i = rand() % 0x100;
		mask = rand() % 0x100;
		for (k = 0; k < MAILBOX_MAX_NUM; k++)
			if ((i >> k) & 1)
				mailbox_context[k] = master_cpu | TEST_SEND | (TEST_MBOX * k);

		// Clear & Enable CPU Mailbox
		mbox_reg->cpu_mbox_set[master_cpu].cpu_mbox_int_clr.mbox_int_clr = i;
		mbox_reg->cpu_mbox_en[master_cpu].mbox_info = i;
		// mask CPU Mailbox
		mbox_reg->cpu_mbox_set[master_cpu].cpu_mbox_int_mask.mbox_int_mask = mask;

		// Clear & Enable inform back
		mbox_done_reg->cpu_mbox_done[master_cpu].cpu_mbox_int_clr.mbox_int_clr = i;
		mbox_done_reg->cpu_mbox_done_en[master_cpu].mbox_info = i;

		// Enable APSYS mailbox
		mbox_reg->mbox_set.mbox_set = i;

		for (k = 0; k < MAILBOX_MAX_NUM; k++)
			if ((i >> k) & 1) {
				if (((mask & i ) >> k) & 1)
					// if mask , the context should be not modify
					while(mailbox_context[k] != (master_cpu | TEST_SEND | (TEST_MBOX * k)));
				else
					// if trigger mailbox, the context should be modify to TEST_DONE
					while(mailbox_context[k] != (master_cpu | TEST_DONE | (TEST_MBOX * k)));
			}
		printf("MAILBOX_SET CPU%d multi send[%d] & mask mailbox[%x] done\n", master_cpu, j, i);

		// Disable CPU Mailbox
		mbox_reg->cpu_mbox_en[master_cpu].mbox_info = 0;	
		mbox_done_reg->cpu_mbox_done_en[master_cpu].mbox_info = 0;
		// when unmask mailbox,the remain set_int & dont_int will trigger interrupt
		mbox_reg->cpu_mbox_set[master_cpu].cpu_mbox_int_clr.mbox_int_clr = 0xFF;
		mbox_done_reg->cpu_mbox_done[master_cpu].cpu_mbox_int_clr.mbox_int_clr = 0xFF;
		mbox_reg->cpu_mbox_set[master_cpu].cpu_mbox_int_mask.mbox_int_mask = 0;
	}

	return 0;
}

int cvi_send_mailbox(int master_cpu, int slave_cpu)
{
	int i, j, k, mask;
	printf("cvi_send_mailbox_test CPU%d -> CPU%d\n", master_cpu, slave_cpu);
	cpu = master_cpu;
	switch(master_cpu) {
		case TEST_CPU0:
			request_irq(INTRID_MAILBOX_CPU0, cvi_mailbox_irq_handler , 0, "MAILBOX CPU0", &cpu);
			break;
		case TEST_CPU1:
			request_irq(INTRID_MAILBOX_CPU1, cvi_mailbox_irq_handler , 0, "MAILBOX CPU1", &cpu);
			break;
		case TEST_CPU2:
			request_irq(INTRID_MAILBOX_CPU2, cvi_mailbox_irq_handler , 0, "MAILBOX CPU2", &cpu);
			break;
		default:
			printf("wrong master CPU %d\n", master_cpu);
			break;
	};

	// sequentially send mailbox to slave
	for (i = 0; i < MAILBOX_MAX_NUM; i++) {
		// init fill context
		for (j = 0; j < MAILBOX_MAX_NUM; j++)
			mailbox_context[j] = -1;

		mailbox_context[i] = master_cpu | TEST_SEND | (TEST_MBOX * i);

		// Clear & Enable CPU Mailbox
		// send mailbox to slave_cpu
		mbox_reg->cpu_mbox_set[slave_cpu].cpu_mbox_int_clr.mbox_int_clr = 1 << i;
		mbox_reg->cpu_mbox_en[slave_cpu].mbox_info = 1 << i;
		// Clear & Enable inform back
		// inform back to specific MAILBOX_SELF_TEST_CPU
		mbox_done_reg->cpu_mbox_done[master_cpu].cpu_mbox_int_clr.mbox_int_clr = 1 << i;
		mbox_done_reg->cpu_mbox_done_en[master_cpu].mbox_info = 1 << i;

		// Enable APSYS mailbox
		mbox_reg->mbox_set.mbox_set = 1 << i;

		do {
		} while(mailbox_context[i] != (master_cpu | TEST_DONE | (TEST_MBOX * i)));

		printf("MAILBOX_SET master CPU%d slave CPU%d single mailbox [%d] done\n", master_cpu, slave_cpu, i);
		// Disable CPU Mailbox
		mbox_reg->cpu_mbox_en[slave_cpu].mbox_info = 0;	
		mbox_done_reg->cpu_mbox_done_en[master_cpu].mbox_info = 0;
	}

	// randomly send mailbox to slave
	for (j = 0; j < 0x100; j++) {
		// init fill context
		for (k = 0; k < MAILBOX_MAX_NUM; k++)
			mailbox_context[k] = -1;

		i = rand() % 0x100;
		for (k = 0; k < MAILBOX_MAX_NUM; k++)
			if ((i >> k) & 1)
				mailbox_context[k] = master_cpu | TEST_SEND | (TEST_MBOX * k);

		// Clear & Enable CPU Mailbox
		// send mailbox to slave_cpu
		mbox_reg->cpu_mbox_set[slave_cpu].cpu_mbox_int_clr.mbox_int_clr = i;
		mbox_reg->cpu_mbox_en[slave_cpu].mbox_info = i;
		// Clear & Enable inform back
		// inform back to specific MAILBOX_SELF_TEST_CPU
		mbox_done_reg->cpu_mbox_done[master_cpu].cpu_mbox_int_clr.mbox_int_clr = i;
		mbox_done_reg->cpu_mbox_done_en[master_cpu].mbox_info = i;

		// Enable APSYS mailbox
		mbox_reg->mbox_set.mbox_set = i;

		for (k = 0; k < MAILBOX_MAX_NUM; k++)
			if ((i >> k) & 1)
				while(mailbox_context[k] != (master_cpu | TEST_DONE | (TEST_MBOX * k)));

		printf("MAILBOX_SET master CPU%d slave CPU%d multisend[%d] mailbox [%d] done\n", master_cpu, slave_cpu, j, i);
		// Disable CPU Mailbox
		mbox_reg->cpu_mbox_en[slave_cpu].mbox_info = 0;	
		mbox_done_reg->cpu_mbox_done_en[master_cpu].mbox_info = 0;
	}

	// randomly send & mask mailbox to slave
	for (j = 0; j < 0x100; j++) {
		// init fill context
		for (k = 0; k < MAILBOX_MAX_NUM; k++)
			mailbox_context[k] = -1;

		i = rand() % 0x100;
		mask = rand() % 0x100;
		for (k = 0; k < MAILBOX_MAX_NUM; k++)
			if ((i >> k) & 1)
				mailbox_context[k] = master_cpu | TEST_SEND | (TEST_MBOX * k);

		// Clear & Enable CPU Mailbox
		// send mailbox to slave_cpu
		mbox_reg->cpu_mbox_set[slave_cpu].cpu_mbox_int_clr.mbox_int_clr = i;
		mbox_reg->cpu_mbox_en[slave_cpu].mbox_info = i;
		// mask CPU Mailbox
		mbox_reg->cpu_mbox_set[slave_cpu].cpu_mbox_int_mask.mbox_int_mask = mask;

		// Clear & Enable inform back
		// inform back to specific MAILBOX_SELF_TEST_CPU
		mbox_done_reg->cpu_mbox_done[master_cpu].cpu_mbox_int_clr.mbox_int_clr = i;
		mbox_done_reg->cpu_mbox_done_en[master_cpu].mbox_info = i;

		// Enable APSYS mailbox
		mbox_reg->mbox_set.mbox_set = i;

		for (k = 0; k < MAILBOX_MAX_NUM; k++)
		if ((i >> k) & 1) {
			if (((mask & i ) >> k) & 1)
				// if mask , the context should be not modify
				while(mailbox_context[k] != (master_cpu | TEST_SEND | (TEST_MBOX * k)));
			else
				// if trigger mailbox, the context should be modify to TEST_DONE
				while(mailbox_context[k] != (master_cpu | TEST_DONE | (TEST_MBOX * k)));
			}

		printf("MAILBOX_SET master CPU%d slave CPU%d multi send[%d] & mask mailbox [%d] done\n", master_cpu, slave_cpu, j, i);
		// Disable CPU Mailbox
		mbox_reg->cpu_mbox_en[slave_cpu].mbox_info = 0;	
		mbox_done_reg->cpu_mbox_done_en[master_cpu].mbox_info = 0;

		// when unmask mailbox,the remain set_int & dont_int will trigger interrupt
		mbox_reg->cpu_mbox_set[slave_cpu].cpu_mbox_int_clr.mbox_int_clr = 0xFF;
		mbox_done_reg->cpu_mbox_done[master_cpu].cpu_mbox_int_clr.mbox_int_clr = 0xFF;
		mbox_reg->cpu_mbox_set[slave_cpu].cpu_mbox_int_mask.mbox_int_mask = 0;
	}

	return 0;
}

int cvi_receive_mailbox(int slave_cpu)
{
	printf("cvi_receive_mailbox %d\n", slave_cpu);
	cpu = slave_cpu;
	switch(slave_cpu) {
		case TEST_CPU0:
			request_irq(INTRID_MAILBOX_CPU0, cvi_mailbox_irq_handler , 0, "MAILBOX CPU0", &cpu);
			break;
		case TEST_CPU1:
			request_irq(INTRID_MAILBOX_CPU1, cvi_mailbox_irq_handler , 0, "MAILBOX CPU1", &cpu);
			break;
		case TEST_CPU2:
			request_irq(INTRID_MAILBOX_CPU2, cvi_mailbox_irq_handler , 0, "MAILBOX CPU2", &cpu);
			break;
		default:
			printf("wrong slave CPU %d\n", slave_cpu);
			break;
	}

	return 0;
}

#define MAILBOX_COMM_CPUX_IDLE    0
#define MAILBOX_COMM_CPU0_RUN     0x000001
#define MAILBOX_COMM_CPU1_RUN     0x000002
#define MAILBOX_COMM_CPU2_RUN     0x000004

#define MAILBOX_COMM_CPU0_SEND    0x000100
#define MAILBOX_COMM_CPU1_SEND    0x000200
#define MAILBOX_COMM_CPU2_SEND    0x000400
#define MAILBOX_COMM_CPUX_SEND    0x000F00

#define MAILBOX_COMM_CPU0_RECV    0x010000
#define MAILBOX_COMM_CPU1_RECV    0x020000
#define MAILBOX_COMM_CPU2_RECV    0x040000
#define MAILBOX_COMM_CPUX_RECV    0x0F0000

void cvi_mailbox_comm(int cpu)
{
	int master_cpu;
	int slave_cpu;

	switch (cpu & MAILBOX_COMM_CPUX_SEND) {
		case  MAILBOX_COMM_CPU0_SEND:
			master_cpu = TEST_CPU0;
			break;
		case  MAILBOX_COMM_CPU1_SEND:
			master_cpu = TEST_CPU1;
			break;
		case  MAILBOX_COMM_CPU2_SEND:
			master_cpu = TEST_CPU2;
			break;
		default:
			master_cpu = TEST_CPU_FAIL;
			break;
	};
	switch (cpu & MAILBOX_COMM_CPUX_RECV) {
		case  MAILBOX_COMM_CPU0_RECV:
			slave_cpu = TEST_CPU0;
			break;
		case  MAILBOX_COMM_CPU1_RECV:
			slave_cpu = TEST_CPU1;
			break;
		case  MAILBOX_COMM_CPU2_RECV:
			slave_cpu = TEST_CPU2;
			break;
		default:
			slave_cpu = TEST_CPU_FAIL;
			break;
	};

	// receive
	if (slave_cpu != TEST_CPU_FAIL && master_cpu == TEST_CPU_FAIL)
		cvi_receive_mailbox(slave_cpu);
	// send
	else if (master_cpu != TEST_CPU_FAIL && slave_cpu != TEST_CPU_FAIL)
		cvi_send_mailbox(master_cpu, slave_cpu);
}

int cvi_mailbox_test()
{
	mbox_reg = (struct mailbox_set_register *) MAILBOX_BASE;
	mbox_done_reg = (struct mailbox_done_register *) (MAILBOX_BASE + 2);
	mailbox_context = (unsigned int *) MAILBOX_CONTEXT;
	mailbox_comm = (unsigned int *) (MAILBOX_CONTEXT_COMM);
	mailbox_comm[0] = MAILBOX_COMM_CPU0_RUN;

#ifdef RTC_SYS
	// self test ca53->c906b->c906l
#ifdef __riscv
#ifdef CONFIG_C906B
// C906B
	while( mailbox_comm[0] != MAILBOX_COMM_CPU1_RUN);
	cvi_mailbox_self_test(TEST_CPU1);
	disable_irq(MAILBOX_SELF_TEST_INT);
	mailbox_comm[0] = MAILBOX_COMM_CPU2_RUN;

#elif defined (CONFIG_C906L)
// C906L
	while( mailbox_comm[0] != MAILBOX_COMM_CPU2_RUN);
	cvi_mailbox_self_test(TEST_CPU1);
	disable_irq(MAILBOX_SELF_TEST_INT);
	mailbox_comm[0] = MAILBOX_COMM_CPU0_RUN;
#else
#error "not defined RISCV type"
#endif
#else
// ARM
	while( mailbox_comm[0] != MAILBOX_COMM_CPU0_RUN);
	cvi_mailbox_self_test(TEST_CPU1);
	disable_irq(MAILBOX_SELF_TEST_INT);
	mailbox_comm[0] = MAILBOX_COMM_CPU1_RUN;
#endif

#else
	// self test ca53->c906b->c906l
#ifdef __riscv
#ifdef CONFIG_C906B
// C906B
	while( mailbox_comm[0] != MAILBOX_COMM_CPU1_RUN);
	cvi_mailbox_self_test(TEST_CPU1);
	mailbox_comm[0] = MAILBOX_COMM_CPU2_RUN;

	while( mailbox_comm[0] != MAILBOX_COMM_CPU1_RUN);
	// CPU0 -> CPU1
	cvi_mailbox_comm(MAILBOX_COMM_CPU1_RECV);
	mailbox_comm[0] = MAILBOX_COMM_CPU0_RUN;

	while( mailbox_comm[0] != MAILBOX_COMM_CPU1_RUN);
	// CPU1 -> CPU0
	mailbox_comm[0] = MAILBOX_COMM_CPU0_RUN;
	cvi_mailbox_comm(MAILBOX_COMM_CPU1_SEND | MAILBOX_COMM_CPU0_RECV);

	// CPU1 -> CPU2
	mailbox_comm[0] = MAILBOX_COMM_CPU2_RUN;
	cvi_mailbox_comm(MAILBOX_COMM_CPU1_SEND | MAILBOX_COMM_CPU2_RECV);

	// CPU2 -> CPU1
	mailbox_comm[0] = MAILBOX_COMM_CPU2_RUN;
	while( mailbox_comm[0] != MAILBOX_COMM_CPU1_RUN);
	cvi_mailbox_comm(MAILBOX_COMM_CPU1_RECV);

#elif defined (CONFIG_C906L)
// C906L

	while( mailbox_comm[0] != MAILBOX_COMM_CPU2_RUN);
	cvi_mailbox_self_test(TEST_CPU2);
	mailbox_comm[0] = MAILBOX_COMM_CPU0_RUN;

	while( mailbox_comm[0] != MAILBOX_COMM_CPU2_RUN);
	// CPU0 -> CPU2
	cvi_mailbox_comm(MAILBOX_COMM_CPU2_RECV);
	mailbox_comm[0] = MAILBOX_COMM_CPU0_RUN;

	while( mailbox_comm[0] != MAILBOX_COMM_CPU2_RUN);
	// CPU1 -> CPU2
	cvi_mailbox_comm(MAILBOX_COMM_CPU2_RECV);
	mailbox_comm[0] = MAILBOX_COMM_CPU1_RUN;

	while( mailbox_comm[0] != MAILBOX_COMM_CPU2_RUN);
	// CPU2 -> CPU0
	mailbox_comm[0] = MAILBOX_COMM_CPU1_RUN;
	cvi_mailbox_comm(MAILBOX_COMM_CPU2_SEND | MAILBOX_COMM_CPU0_RECV);

	// CPU2 -> CPU1
	mailbox_comm[0] = MAILBOX_COMM_CPU1_RUN;
	cvi_mailbox_comm(MAILBOX_COMM_CPU2_SEND | MAILBOX_COMM_CPU1_RECV);

#else
#error "not defined RISCV type"
#endif
#else
// ARM
	while( mailbox_comm[0] != MAILBOX_COMM_CPU0_RUN);
	cvi_mailbox_self_test(TEST_CPU0);
	mailbox_comm[0] = MAILBOX_COMM_CPU1_RUN;

	while( mailbox_comm[0] != MAILBOX_COMM_CPU0_RUN);
	// CPU0 -> CPU1
	mailbox_comm[0] = MAILBOX_COMM_CPU1_RUN;
	cvi_mailbox_comm(MAILBOX_COMM_CPU0_SEND | MAILBOX_COMM_CPU1_RECV);

	// CPU0 -> CPU2
	mailbox_comm[0] = MAILBOX_COMM_CPU2_RUN;
	cvi_mailbox_comm(MAILBOX_COMM_CPU0_SEND | MAILBOX_COMM_CPU2_RECV);

	// CPU1 -> CPU0
	mailbox_comm[0] = MAILBOX_COMM_CPU1_RUN;
	while( mailbox_comm[0] != MAILBOX_COMM_CPU0_RUN);
	cvi_mailbox_comm(MAILBOX_COMM_CPU0_RECV);
	mailbox_comm[0] = MAILBOX_COMM_CPU1_RUN;

	// CPU2 -> CPU0
	while( mailbox_comm[0] != MAILBOX_COMM_CPU0_RUN);
	cvi_mailbox_comm(MAILBOX_COMM_CPU0_RECV);
	mailbox_comm[0] = MAILBOX_COMM_CPU2_RUN;

#endif
#endif
	while(1);
	return 0;
}

#ifndef BUILD_TEST_CASE_all
int testcase_main(void)
{
    return cvi_mailbox_test();
}
#endif

