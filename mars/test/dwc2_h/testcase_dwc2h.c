#include <stdint.h>
#include <string.h>
#include <usb.h>
#include "boot_test.h"
#include "mmio.h"
#include "cli.h"

extern int cli_readline(char *prompt, char *cmdbuf);
int cur_dev = -1;

int dwc2_intr_handler(int irqn, void *priv)
{
    return 0;
}

int usb_blk_write(void *addr, lbaint_t blk, lbaint_t cnt)
{
	unsigned long n;

	n = usb_stor_write(cur_dev, blk, cnt, addr);
	printf("%ld blocks write: %s\n", n, (n == cnt) ? "OK" : "ERROR");

	if (n == cnt)
		return 0;

	return -1;
}

int usb_blk_read(void *addr, lbaint_t blk, lbaint_t cnt)
{
	unsigned long n;

	n = usb_stor_read(cur_dev, blk, cnt, addr);
	printf("%ld blocks read: %s\n", n, (n == cnt) ? "OK" : "ERROR");

	if (n == cnt)
		return 0;

	return -1;
}

int usb_suspend(void)
{
	return usb_stor_suspend(cur_dev);
}

int usb_resume(void)
{
	return usb_stor_resume(cur_dev);
}

int usb_rw_test(uint8_t *buf, lbaint_t blk, lbaint_t cnt);

static int dump_regs(uint32_t base, uint32_t count)
{
	uint32_t addr = base;
	int i;

	for (i = 0; i < count; i++) {
		if (i % 4 == 0) {
			if (i != 0)
				printf("\n");
			printf("0x%08x\t", addr);
			addr += 0x10;
		}
		printf("%08x ", mmio_read_32(base + i * 4));
	}
	printf("\n");

	return 0;
}

void test_seq(void)
{
    unsigned int value;

    printf("overwrite usb_id\n");
    value = mmio_read_32(TOP_BASE + 0x0048);
    value &= ~(0xC0);
    value |= 0x40;
    mmio_write_32(TOP_BASE + 0x0048, value);

#if MEM_REMAP==1
    printf("enable usb mem remap\n");
    mmio_write_32(TOP_BASE + 0x0064, ADDR_REMAP_USB(1));
#else
    printf("disable usb mem remap\n");
#endif

}

#define TEST_START_LBA	0x8800//3000
#define TEST_BLOCK_COUNT	5
#define STOR_BLOCK_SIZE	512

int testcase_dwc2h(void)
{
	//uint8_t buf[STOR_BLOCK_SIZE * TEST_BLOCK_COUNT] __attribute__((aligned (STOR_BLOCK_SIZE)));
	uint8_t *buf = (uint8_t *)0x82000000;

	test_seq();

//	while(1);

	/*  init isr */
        request_irq(USB_IRQS_0, dwc2_intr_handler, 0, "dwc2_intr", &cur_dev);
	printf("request irq %d\n", USB_IRQS_0);

	if (usb_init() < 0) {
		printf("usb init failed\n");
		return -1;
	}

	if ((cur_dev = usb_stor_scan(1))) {
		printf("usb storage init failed\n");
		return -1;
	}

	usb_stor_info();

	usb_rw_test(buf, TEST_START_LBA, TEST_BLOCK_COUNT);

	return 0;
}

module_testcase("1", testcase_dwc2h);

#ifndef BUILD_TEST_CASE_all
int testcase_main()
{
    return testcase_dwc2h();
}
#endif

int usb_rw_test(uint8_t *buf, lbaint_t start_lba, lbaint_t cnt)
{
	//char cmdbuf[8];
	int i;
	uint8_t pattern = 0x5a;
	int sus_cnt = 20;

#if 1
	printf("\r\n================write/read lba=0x%lx count=%ld buf:%p================== \r\n", start_lba, cnt, buf);
	//usb_blk_read(buf, start_lba, cnt);
	memset(buf, pattern, cnt * STOR_BLOCK_SIZE);
	usb_blk_write(buf, start_lba, cnt);
	memset(buf, 0x0, cnt * STOR_BLOCK_SIZE);
	usb_blk_read(buf, start_lba, cnt);

	for (i = 0; i < cnt * STOR_BLOCK_SIZE; ++i) {
		if (buf[i] != pattern) {
			printf("write failed at i=%d 0x%x != 0x%x\n", i, buf[i], pattern);
			ASSERT(0);
		}
	}
	printf("\r\n======================== compare ok ========================= \r\n");
#endif
#if 1
	while (sus_cnt--) {
		printf("\r\n======================== suspend ========================= \r\n");
		usb_suspend();
		//cli_readline("suspend! type any key to resume", cmdbuf);
		mdelay(100);
		printf("\r\n======================== resume ========================= \r\n");
		usb_resume();
		//cli_readline("resume! type any key to bulk write", cmdbuf);
		mdelay(100);
		pattern = 0x73;
		memset(buf, pattern, cnt * STOR_BLOCK_SIZE);
		usb_blk_write(buf, start_lba, cnt);
		//cli_readline("suspend! type any key to bulk read", cmdbuf);
		memset(buf, 0x0, cnt * STOR_BLOCK_SIZE);
		usb_blk_read(buf, start_lba, cnt);

		for (i = 0; i < cnt * STOR_BLOCK_SIZE; ++i) {
			if (buf[i] != pattern) {
				printf("write failed at i=%d 0x%x != 0x%x\n", i, buf[i], pattern);
				ASSERT(0);
			}
		}
		printf("\r\n========================[%d] compare ok ========================= \r\n", sus_cnt);

	}
#endif
	printf("\r\n======================== test done ========================= \r\n");
	return 0;
}

