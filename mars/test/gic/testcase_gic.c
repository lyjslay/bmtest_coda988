#include <stdio.h>
#include "boot_test.h"
#include "timer.h"
#include "system_common.h"

#define GIC_TEST_DEBUG
#ifdef GIC_TEST_DEBUG
#define gic_test_dbg(x, args...)    printf("[%s] - " x, __func__, ##args)
#else
#define gic_test_dbg(x, args...)
#endif

#define REG_BASE_UART1	0x5002A000
#define UART1_IER	(REG_BASE_UART1 + 0x04)

#define UART_IER_ETBEI	(0x1UL << 1)

static volatile int mark;
static volatile int cur_irq = -1;

static int irq_handler(int irqn, void *priv)
{
	gic_test_dbg("irq_handler:%d\n", irqn);
	mark++;
	writel(UART1_IER, 0);

	return 0;
}

static int check_irq_handler(int timeout)
{
	int i;

	for (i = 0; i < timeout; ++i) {
		if (mark > 0)
			return 0;
		mdelay(1);
	}

	return -1;
}

int testcase_gic(void)
{
	cur_irq = irq_get_nums() - 1;
	gic_test_dbg("gic spi interrupt nums:%d\n", cur_irq);
	for (; cur_irq >= 0; --cur_irq) {
		mark = 0;
		request_irq(cur_irq, irq_handler, 0, "software int", NULL);
		irq_trigger(cur_irq);
		if (check_irq_handler(100) != 0) {
			gic_test_dbg("test err int-num:%d\n", cur_irq);
			return -1;
		}
		irq_clear(cur_irq);
	}

	gic_test_dbg("Peripheral trigger test!\n");
	cur_irq = PERI_UART1_INTR;
	request_irq(cur_irq, irq_handler, 0, "uart1 int", NULL);
	mark = 0;
	writel(UART1_IER, UART_IER_ETBEI);
	mdelay(10);
	if (mark == 0) {
		gic_test_dbg("irq_handler missed\n");
		writel(UART1_IER, 0);
		return -1;
	} else {
		gic_test_dbg("enable/disable irq test!\n");
		disable_irq(cur_irq);
		writel(UART1_IER, UART_IER_ETBEI);
		mark = 0;
		mdelay(10);
		if (mark != 0) {
			gic_test_dbg("disable_irq failed\n");
			mark = 0;
			return -1;
		} else {
			enable_irq(cur_irq);
			mdelay(10);
			if (mark == 0) {
				gic_test_dbg("enable_irq failed\n");
				writel(UART1_IER, 0);
				return -1;
			} else {
				mark = 0;
			}
		}

		gic_test_dbg("cpu enable/disable irq test!\n");
		cpu_disable_irqs();
		writel(UART1_IER, UART_IER_ETBEI);
		mark = 0;
		mdelay(10);
		if (mark != 0) {
			gic_test_dbg("cpu_disable_irqs failed\n");
			mark = 0;
			return -1;
		} else {
			cpu_enable_irqs();
			mdelay(10);
			if (mark == 0) {
				gic_test_dbg("cpu_enable_irqs failed\n");
				writel(UART1_IER, 0);
				return -1;
			} else {
				mark = 0;
			}
		}
	}

	printf("test done!\n");

	return 0;
}

module_testcase("1", testcase_gic);
