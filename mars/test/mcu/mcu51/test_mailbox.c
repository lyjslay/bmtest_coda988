#include <stdio.h>
#include "test.h"

#if TEST_MAILBOX


#define MAILBOX_BASE 0x1900000

#define MAILBOX_CPU0_EN (MAILBOX_BASE + 0x0000)
#define MAILBOX_CPU1_EN (MAILBOX_BASE + 0x0004)
#define MAILBOX_CPU2_EN (MAILBOX_BASE + 0x0008)
#define MAILBOX_CPU3_EN (MAILBOX_BASE + 0x000C)

#define MAILBOX_CPU0_CLR (MAILBOX_BASE + 0x0010)
#define MAILBOX_CPU0_MSK (MAILBOX_BASE + 0x0014)
#define MAILBOX_CPU0_INT (MAILBOX_BASE + 0x0018)
#define MAILBOX_CPU0_RAW (MAILBOX_BASE + 0x001C)

#define MAILBOX_CPU1_CLR (MAILBOX_BASE + 0x0020)
#define MAILBOX_CPU1_MSK (MAILBOX_BASE + 0x0024)
#define MAILBOX_CPU1_INT (MAILBOX_BASE + 0x0028)
#define MAILBOX_CPU1_RAW (MAILBOX_BASE + 0x002C)

#define MAILBOX_CPU2_CLR (MAILBOX_BASE + 0x0030)
#define MAILBOX_CPU2_MSK (MAILBOX_BASE + 0x0034)
#define MAILBOX_CPU2_INT (MAILBOX_BASE + 0x0038)
#define MAILBOX_CPU2_RAW (MAILBOX_BASE + 0x003C)

#define MAILBOX_SET (MAILBOX_BASE + 0x0060)

#define MAILBOX_STATUS0 (MAILBOX_BASE + 0x0070)
#define MAILBOX_STATUS1 (MAILBOX_BASE + 0x0074)
#define MAILBOX_STATUS2 (MAILBOX_BASE + 0x0078)
#define MAILBOX_STATUS3 (MAILBOX_BASE + 0x007C)

#define MAILBOX_CONTEXT (MAILBOX_BASE + 0x0400)

#define INTRID_MAILBOX_CPU0 85
#define INTRID_MAILBOX_CPU1 101
#define INTRID_MAILBOX_CPU2 61

#define TEST_CPU0 0x000000
#define TEST_CPU1 0x000001
#define TEST_CPU2 0x000002
#define TEST_CPU3 0x000003
#define TEST_CPU_FAIL 0xFFFFFF

#define TEST_SEND 0x000100
#define TEST_RECV 0x000200
#define TEST_DONE 0x000300
#define TEST_MBOX 0x010000

#define MAILBOX_MAX_NUM 0x8

union cpu_mailbox_info_offset {
	char mbox_info;
	int reserved;
};

union cpu_mailbox_int_clr_offset {
	char mbox_int_clr;
	int reserved;
};
union cpu_mailbox_int_mask_offset {
	char mbox_int_mask;
	int reserved;
};
union cpu_mailbox_int_offset {
	char mbox_int;
	int reserved;
};
union cpu_mailbox_int_raw_offset {
	char mbox_int_raw;
	int reserved;
};

union mailbox_set {
	char mbox_set;
	int reserved;
};
union mailbox_status {
	char mbox_status;
	int reserved;
};

union cpu_mailbox_status {
	char mbox_status;
	int reserved;
};

/* register mapping refers to mailbox user guide*/
struct cpu_mbox_int {
	union cpu_mailbox_int_clr_offset cpu_mbox_int_clr;
	union cpu_mailbox_int_mask_offset cpu_mbox_int_mask;
	union cpu_mailbox_int_offset cpu_mbox_int_int;
	union cpu_mailbox_int_raw_offset cpu_mbox_int_raw;
};

struct mailbox_set_register {
	union cpu_mailbox_info_offset cpu_mbox_en[4];
	struct cpu_mbox_int cpu_mbox_set[4];
	int reserved[4];
	union mailbox_set mbox_set;
	union mailbox_status mbox_status;
	union cpu_mailbox_status cpu_mbox_status[4];
};

struct mailbox_done_register {
	union cpu_mailbox_info_offset cpu_mbox_done_en[4];
	struct cpu_mbox_int cpu_mbox_done[4];
};

struct mailbox_set_register *mbox_reg;
struct mailbox_done_register *mbox_done_reg;
unsigned int *mailbox_context; // 32 Bytes
unsigned int *mailbox_comm; // 32 Bytes

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

void mailbox2(void)
{
	// int master_cpu = TEST_CPU3;
	int i, k;

	printf("mailbox2\n");
	return;

	// mbox_reg = (struct mailbox_set_register *) MAILBOX_BASE;
	// mbox_done_reg = (struct mailbox_done_register *) (MAILBOX_BASE + 2);
	// mailbox_context = (unsigned int *) MAILBOX_CONTEXT;
	// mailbox_comm = (unsigned int *) (MAILBOX_CONTEXT + 0x20);
	// mailbox_comm[0] = MAILBOX_COMM_CPU0_RUN;

	for (i = 0; i < 1; i++) {
		// init fill context
		for (k = 0; k < MAILBOX_MAX_NUM; k++) {
			// mailbox_context[k] = -1;
			write_robot(MAILBOX_CONTEXT + k * 4, 0xFFFFFFFF);
		}

		// mailbox_context[i] = master_cpu | TEST_SEND | (TEST_MBOX * i);
		write_robot(MAILBOX_CONTEXT + i * 4, TEST_CPU3 | TEST_SEND | (TEST_MBOX * i));

		// // Clear & Enable CPU Mailbox
		// // send mailbox to specific MAILBOX_SELF_TEST_CPU
		// mbox_reg->cpu_mbox_set[master_cpu].cpu_mbox_int_clr.mbox_int_clr = 1 << i;
		write_robot(MAILBOX_BASE + 0x10 + TEST_CPU3 * 4, (1UL << i) | (1UL << 16 << i));
		// mbox_reg->cpu_mbox_en[master_cpu].mbox_info = 1 << i;
		write_robot(MAILBOX_BASE + 0x00 + TEST_CPU3 * 4, (1UL << i) | (1UL << 16 << i));

		// // Clear & Enable inform back
		// // inform back to specific MAILBOX_SELF_TEST_CPU
		// mbox_done_reg->cpu_mbox_done[master_cpu].cpu_mbox_int_clr.mbox_int_clr = 1 << i;
		// mbox_done_reg->cpu_mbox_done_en[master_cpu].mbox_info = 1 << i;

		// // Enable APSYS mailbox
		// mbox_reg->mbox_set.mbox_set = 1 << i;
		write_robot(MAILBOX_BASE + 0x60, (1 << i));
		// do {
		// } while (mailbox_context[i] != (master_cpu | TEST_DONE | (TEST_MBOX * i)));

		// printf("MAILBOX_SET CPU%d single mailbox [%x] done\n", master_cpu, i);
		// // Disable CPU Mailbox
		// mbox_reg->cpu_mbox_en[master_cpu].mbox_info = 0;
		// mbox_done_reg->cpu_mbox_done_en[master_cpu].mbox_info = 0;
	}
}

static int xdata mailbox_trigger;

void mailbox_isr(void)
{
	mailbox_trigger++;
}

int test_mailbox()
{
	EA = 0;
	xs1 = 0xC5;

	mailbox_trigger = 100;

	printf("test_mailbox():\n");

	// write_robot(REG_CPU0_INT_EN, 0xf);
	// write_robot(REG_CPU1_INT_EN, 0xf);
	// write_robot(REG_CPU0_MBOX_EN, 0x000F000F);

	// Enable mailbox interrupt
	dw_ictl_unmask(MBOX_INTR);
	EX0 = 1;
	EA = 1;

	mailbox2();
	printf("mailbox set\n");

	xs1 = 0xC6;
	while(1) {
		printf("mailbox_trigger=%d (0x%08x 0x%08x)\n", mailbox_trigger, dw_ictl_get_status(), dw_ictl_get_count());
	}

        return 0;
}

#endif /* TEST_MAILBOX */