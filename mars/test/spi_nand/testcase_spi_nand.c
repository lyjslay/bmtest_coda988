#include "module_testcase.h"
#include "system_common.h"
#include "mmio.h"
#include "timer.h"

#include "testcase_spi_nand.h"
#include "bm_spi_nand.h"
#include <stdint.h>
#include <assert.h>
#include <string.h>

//#define PROGRAM_LOAD_X4

#define TEST_ADDR   0

#define BUF_LEN			2176
#define PAGE_SPARE_SIZE		2112
#define PAGE_SIZE		2048

#define NR_PG_PER_NATIVE_BLK	64
#define TEST_BLK_ID		113

#define NR_PG_PER_NATIVE_BLK_FOR_TESTING	64

#define START_BLK_ID		0
#define END_BLK_ID		4

#define SEED_NAND 0xfeee11

// when internal ECC disabled the Data Byte is 2176, when internal ECC enabled the Data Byte is 2112.
static uint8_t g_wbuf[BUF_LEN] __attribute__((aligned (512)));
static uint8_t g_rbuf[BUF_LEN] __attribute__((aligned (512)));

typedef int (*handler_t)();
struct testcase {
	handler_t handler;
	char *name;
};

int spi_nand_read_id_test();
int spi_nand_block_data_ecc_test();
int spi_nand_block_data_len_test();
int spi_nand_block_xfer_mode_test();
int spi_nand_nand_cache_test();
int spi_nand_defect_scan_test();
int spi_nand_whole_nand_epr_test();

struct testcase spi_nand_testcases[] = {
#ifdef BUILD_TEST_CASE_SLT_spi
//    { .handler = spi_nand_test_for_slt, .name = "SPI NAND test for SLT" },
#else
	{ .handler = spi_nand_read_id_test, .name = "SPI NAND read ID test" },
//	{ .handler = spi_nand_block_data_ecc_test, .name = "SPI NAND data ecc test" },
//	{ .handler = spi_nand_block_data_len_test, .name = "SPI NAND data len test" },
//	{ .handler = spi_nand_nand_cache_test, .name = "SPI NAND NAND cache test" },
//	{ .handler = spi_nand_block_xfer_mode_test, .name = "SPI NAND transfer mode test" },
//	{ .handler = spi_nand_defect_scan_test, .name = "SPI NAND defect scan test" },
	{ .handler = spi_nand_whole_nand_epr_test, .name = "SPI NAND whole nand test" },
#endif
};

void spi_nand_set_pinmux()
{
	printf("%s\n", __func__);
	pinmux_config(PINMUX_SPI_NAND);
}

void spi_nand_init()
{
	printf("%s BASE_SPI_NAND 0x%x\n", __func__, BASE_SPI_NAND);

	spi_nand_set_pinmux();

	//bm_spi_nand_init(BASE_SPI_NAND, 3, 0, 0);   // clk_div = 3, cpha = 0, cpol = 0
	bm_spi_nand_init(BASE_SPI_NAND, 4, 1, 1);   // clk_div = 4, cpha = 1, cpol = 1
}

#ifdef BUILD_TEST_CASE_SLT_spi

#else
extern void memset_dword(void *buff, u32 val, u32 size);
extern void memset_dword_inc(void *buff, u32 start, u32 size);

static u32 compare_buf(void *r_buf, void *w_buf, u32 len)
{
	if (memcmp(r_buf, w_buf, len) != 0) {
		printf("Data compare error!!!!\n");
		dump_hex("r_buf2:", r_buf, 16);
		dump_hex("w_buf2:", w_buf, 16);
//		return 1;
		while (1);
	}

	return 0;
}

int spi_nand_read_id_test()
{
	printf("\n------------ spi_nand_read_id_test ------------\n\n");

	u16 id = bm_spi_nand_read_id();
	printf("Flash ID = 0x%x\n", id);

	return 1;
}

int spi_nand_defect_scan_test()
{
//	u32 ecc_sts = 0;
	printf("\n------------ spi_nand_read_defect_scan_test ------------\n\n");

	// defect scan
	uint32_t i;
	for (i = 0; i < 1024 ; i ++) {
		bm_spi_nand_erase_block(i);
		bm_spi_nand_read_page_non_ecc(i, 0, g_rbuf, BUF_LEN);
		bm_spi_nand_read_page(i, 0, (void *)g_rbuf, PAGE_SIZE, 0);
	}

	return 1;
}

// Erase/Program a block and
// read back with/without ECC
int spi_nand_block_data_ecc_test()
{
//	u32 ecc_sts = 0;

	printf("\n------------ spi_nand_block_data_ecc_test ------------\n\n");

	uint32_t pg_i = 0;
	uint32_t blk_i = TEST_BLK_ID;
//
	bm_spi_nand_erase_block(blk_i);
	memset((void*)g_wbuf, pg_i* 0x15 + blk_i, BUF_LEN);
//	memset_dword((void*)g_wbuf, ((pg_i << 8) + SEED_NAND) + blk_i, BUF_LEN);
	dump_hex("g_wbuf", g_wbuf, BUF_LEN);
	bm_spi_nand_write_page(blk_i, pg_i, (void *)g_wbuf, PAGE_SPARE_SIZE/2, 0);

	memset(g_rbuf, 0, BUF_LEN);
	bm_spi_nand_read_page_non_ecc(blk_i, pg_i, g_rbuf, BUF_LEN);
	dump_hex("non_ecc g_rbuf", g_rbuf, BUF_LEN);

	memset(g_rbuf, 0, BUF_LEN);
	bm_spi_nand_read_page(blk_i, pg_i, (void *)g_rbuf, PAGE_SPARE_SIZE, 0);
	dump_hex("ecc g_rbuf", g_rbuf, BUF_LEN);

	return 1;
}


int spi_nand_block_data_len_test()
{
	printf("\n------------ spi_nand_block_data_len_test ------------\n\n");

	uint32_t pg_i = 0;
	uint32_t blk_i = TEST_BLK_ID;
//	u32 ecc_sts = 0;

//
	bm_spi_nand_erase_block(blk_i);
	memset((void*)g_wbuf, pg_i* 0x15 + blk_i, BUF_LEN);
	memset_dword((void*)g_wbuf, ((pg_i << 8) + SEED_NAND) + blk_i, BUF_LEN);
//	dump_hex("g_wbuf", g_wbuf, BUF_LEN/8);

#ifdef PROGRAM_LOAD_X4
	bm_spi_nand_write_page(blk_i, pg_i, (void *)g_wbuf, PAGE_SPARE_SIZE, 1);
#else
	bm_spi_nand_write_page(blk_i, pg_i, (void *)g_wbuf, PAGE_SPARE_SIZE, 0);
#endif

	memset(g_rbuf, 0, BUF_LEN);
	bm_spi_nand_read_page(blk_i, pg_i, (void *)g_rbuf, PAGE_SPARE_SIZE, 0);
//	dump_hex("g_rbuf", g_rbuf, BUF_LEN);

	compare_buf(g_rbuf, g_wbuf, PAGE_SIZE);

	return 1;
}


int spi_nand_block_xfer_mode_test()
{
	printf("\n------------ spi_nand_block_xfer_mode_test ------------\n\n");

	 uint32_t pg_i = 0;
	 uint32_t blk_i = TEST_BLK_ID;

	for (blk_i = START_BLK_ID; blk_i <= END_BLK_ID ; blk_i++) {
		bm_spi_nand_erase_block(blk_i);
//
		for (pg_i = 0; pg_i < NR_PG_PER_NATIVE_BLK ; pg_i++) {
			memset_dword_inc((void*)g_wbuf, ((pg_i << 8) + SEED_NAND) + blk_i, PAGE_SIZE);
//			dump_hex("g_wbuf:", g_wbuf, PAGE_SIZE);
			bm_spi_nand_write_page(blk_i, pg_i, (void *)g_wbuf, PAGE_SIZE, pg_i % 2);
		}

		for (pg_i = 0; pg_i < NR_PG_PER_NATIVE_BLK ; pg_i++) {
			memset(g_rbuf, 0, BUF_LEN);
			u32 ecc_sts = bm_spi_nand_read_page(blk_i, pg_i, (void *)g_rbuf, PAGE_SIZE, pg_i % 1);

			if (ecc_sts) {
				printf("ecc_sts 0x%x\n", ecc_sts);
			}

//			dump_hex("g_rbuf:", g_rbuf, 16);
			memset_dword_inc((void*)g_wbuf, ((pg_i << 8) + SEED_NAND) + blk_i, PAGE_SIZE);
//			dump_hex("g_wbuf:", g_wbuf, 16);
			u32 ret = compare_buf(g_rbuf, g_wbuf, PAGE_SIZE);

			if (ret) {
				printf("pg_i %d\n", pg_i);
				while(1);
			}
		}
	}

	return 1;
}

int spi_nand_nand_cache_test()
{
	printf("\n------------ spi_nand_nand_cache_test ------------\n\n");

	 uint32_t pg_i = 0;
	 uint32_t blk_i = TEST_BLK_ID;
	 uint32_t seed = SEED_NAND;

	for (blk_i = START_BLK_ID; blk_i <= END_BLK_ID ; blk_i++) {
		printf("%s : blk id %d\n", __func__, blk_i);

		for (pg_i = 0; pg_i < NR_PG_PER_NATIVE_BLK ; pg_i++) {
//			memset(g_rbuf, 0, BUF_LEN);
			memset_dword_inc((void*)g_rbuf, ((pg_i << 8) + seed* pg_i / 2) + blk_i, PAGE_SIZE);
//			dump_hex("g_rbuf1:", g_rbuf, 16);
			u32 status = bm_spi_nand_prg_read_cache(blk_i, pg_i, (void *)g_rbuf, PAGE_SIZE, pg_i % 1);
//			dump_hex("g_rbuf2:", g_rbuf, 16);

			if (status) {
				printf("sts 0x%x\n", status);
			}

//			dump_hex("g_rbuf:", g_rbuf, 16);
			memset_dword_inc((void*)g_wbuf, ((pg_i << 8) + seed* pg_i / 2) + blk_i, PAGE_SIZE);
			dump_hex("g_wbuf:", g_wbuf, 16);
			u32 ret = compare_buf(g_rbuf, g_wbuf, PAGE_SIZE);

			if (ret) {
				printf("pg_i %d\n", pg_i);
				while(1);
			}
		}
	}

	return 1;
}


int spi_nand_whole_nand_epr_test()
{
	printf("\n------------ spi_nand_whole_nand_epr_test ------------\n\n");

	uint32_t blk_i = 0;
	uint32_t pg_i = 0;
	//uint32_t w_qe = SPI_NAND_PROGRAM_LOAD_X1;
	uint32_t w_qe = SPI_NAND_PROGRAM_LOAD_X4;
	//uint32_t r_qe = SPI_NAND_READ_FROM_CACHE_MODE_X1;
	//uint32_t r_qe = SPI_NAND_READ_FROM_CACHE_MODE_X2;
	uint32_t r_qe = SPI_NAND_READ_FROM_CACHE_MODE_X4;
	uint32_t blk_inc = 1;

	printf("w_qe %d r_qe %d\n", w_qe, r_qe);

	for (blk_i = START_BLK_ID; blk_i < END_BLK_ID ; blk_i = blk_i + blk_inc) {

		bm_spi_nand_erase_block(blk_i);
		printf("write blk %d \n", blk_i);

//		for (pg_i = 0; pg_i < NR_PG_PER_NATIVE_BLK ; pg_i++) {
		for (pg_i = 0; pg_i < NR_PG_PER_NATIVE_BLK_FOR_TESTING ; pg_i++) {
//			memset((void*)g_wbuf, 0xff, PAGE_SIZE);
			memset_dword((void*)g_wbuf, ((blk_i << 8) | pg_i), PAGE_SIZE);
//			dump_hex("g_wbuf:", g_wbuf, 16);
			bm_spi_nand_write_page(blk_i, pg_i, (void *)g_wbuf, PAGE_SIZE, w_qe);

			printf("read and compare blk %d, page %d\n", blk_i, pg_i);
			memset_dword((void*)g_rbuf, 0, PAGE_SIZE);
			bm_spi_nand_read_page(blk_i, pg_i, (void *)g_rbuf, PAGE_SIZE, r_qe);
			compare_buf(g_rbuf, g_wbuf, PAGE_SIZE);

		}
	}

	return 1;
}
#endif

static int irq_handler(int irqn, void *priv)
{

	printf("%s, get irq=%d\n", __func__, irqn);
	disable_irq(irqn);

    return 0;
}

int testcase_spi_nand(void)
{
	int i;

	printf("\n------------ SPI NAND test start ------------\n\n");

#ifdef PROGRAM_LOAD_X4
	printf("PROGRAM_LOAD_X4\n");
#endif
	request_irq(SPI_NAND_INTR, irq_handler, 0, "SPI_NAND int", NULL);
	spi_nand_init();

	for (i = 0; i < sizeof(spi_nand_testcases) / sizeof(spi_nand_testcases[0]); ++i) {
		printf("[%d] %s start\n", i, spi_nand_testcases[i].name);
		if ( spi_nand_testcases[i].handler() == 0 ) {
			printf("[%d] %s failed\n", i, spi_nand_testcases[i].name);
			return -1;
		}
		printf("[%d] %s pass\n\n", i, spi_nand_testcases[i].name);
	}

	printf("\n------------ SPI NAND test done ------------\n");
	return 0;
}

#ifndef BUILD_TEST_CASE_all
int testcase_main()
{
	return testcase_spi_nand();
}
#endif

