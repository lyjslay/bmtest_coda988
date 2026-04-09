#include <common.h>
#include "module_testcase.h"
#include "system_common.h"
#include "mmio.h"
#include "timer.h"
#include <stdlib.h>
#include "testcase_spi_nand.h"
#include "bm_spi_nand.h"
#include <stdint.h>
#include <assert.h>
#include <string.h>
#include "command.h"
#include "cli.h"

#define ARRAY_SIZE(arr)           (sizeof(arr) / sizeof((arr)[0]))

#define PROGRAM_LOAD_X4

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


int get_param_str(int argc, char **argv, char *index, char *content)
{
	int i=0;
	for(i=0; i<argc-1; i++){
		if(strcmp(argv[i], index) == 0){
			content = argv[i+1];
			return 0;
		}
	}
	return -1;
}

/*
* argv[0]:cmd
* argv[1]:--xxx
* argv[2]:val
*/
int get_param_uint32(int argc, char **argv, char *index, uint32_t *content)
{
	int i=0;
	if(argc<3 || argv==NULL || index==NULL) return -1;
	for(i=1; i<argc-1; i++){
		if(strcmp(argv[i], index) == 0){
			*content = atoi(argv[i+1]);
			return 0;
		}
	}
	return -1;
}

void spi_nand_set_pinmux()
{
	printf("%s\n", __func__);
	pinmux_config(PINMUX_SPI_NAND);
}

/*
* 1. 设置测试哪个nand控制器
* 2. 设置管脚复用
* 3. 设置分频
* 4. 设置时钟极性和相位
* 5. nandinit --clkdiv 3 --cpha 0 --cpol 0
*/
static int spi_nand_init(int argc, char **argv)
{
	u32 spi_nand_ctrl_base = BASE_SPI_NAND;
	u32 clkdiv=3, cpha=0, cpol=0;

	// get_param_uint32(argc, argv, "--nandnum", &spi_nand_ctrl_base);
	get_param_uint32(argc, argv, "--clkdiv", &clkdiv);
	get_param_uint32(argc, argv, "--cpha", &cpha);
	get_param_uint32(argc, argv, "--cpol", &cpol);

	printf("%s base=0x%x, clkdiv=%d, cpha=%d, cpol=%d\n", __func__, spi_nand_ctrl_base, clkdiv, cpha, cpol);

	spi_nand_set_pinmux();

	bm_spi_nand_init(spi_nand_ctrl_base, clkdiv, cpha, cpol);

	return 0;
}

#ifdef BUILD_TEST_CASE_SLT_spi

#else
extern void memset_dword(void *buff, u32 val, u32 size);
extern void memset_dword_inc(void *buff, u32 start, u32 size);

static int compare_buf(void *r_buf, void *w_buf, u32 len)
{
	if (memcmp(r_buf, w_buf, len) != 0) {
		printf("Data compare error!!!!\n");
		dump_hex("r_buf2:", r_buf, 16);
		dump_hex("w_buf2:", w_buf, 16);
		return -1;
	}
	// dump_hex("r_buf2:", r_buf, 16);
	// dump_hex("w_buf2:", w_buf, 16);

	return 0;
}

int spi_nand_read_id_test(int argc, char **argv)
{
	printf("\n------------ spi_nand_read_id_test ------------\n\n");

	u16 id = bm_spi_nand_read_id();
	printf("Flash ID = 0x%x\n", id);

	return 0;
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

/*
* 1. Test Data transfer with different length
*/
int spi_nand_block_data_len_test(int argc, char **argv)
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

	return compare_buf(g_rbuf, g_wbuf, PAGE_SIZE);
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

/*
* 1、 Test Data transfer with different bit mode
*/
int spi_nand_whole_nand_epr_test(int argc, char **argv)
{
	printf("\n------------ spi_nand_whole_nand_epr_test ------------\n\n");

	uint32_t blk_i = 0;
	uint32_t pg_i = 0;
	//uint32_t writemode = SPI_NAND_PROGRAM_LOAD_X1;
	uint32_t writemode = SPI_NAND_PROGRAM_LOAD_X4;
	//uint32_t readmode = SPI_NAND_READ_FROM_CACHE_MODE_X1;
	//uint32_t readmode = SPI_NAND_READ_FROM_CACHE_MODE_X2;
	uint32_t readmode = SPI_NAND_READ_FROM_CACHE_MODE_X4;
	uint32_t blk_inc = 1;
	int ret=0;

	get_param_uint32(argc, argv, "--writemode", &writemode);
	get_param_uint32(argc, argv, "--readmode", &readmode);

	printf("writemode=%d, readmode=%d\n", writemode, readmode);

#if 1
	for (blk_i = START_BLK_ID; blk_i < END_BLK_ID ; blk_i = blk_i + blk_inc) {

		bm_spi_nand_erase_block(blk_i);
		printf("write blk %d \n", blk_i);

		for (pg_i = 0; pg_i < NR_PG_PER_NATIVE_BLK_FOR_TESTING ; pg_i++) {
//			memset((void*)g_wbuf, 0xff, PAGE_SIZE);
			memset_dword_inc((void*)g_wbuf, ((blk_i << 8) | pg_i), PAGE_SIZE);
//			dump_hex("g_wbuf:", g_wbuf, 16);
			bm_spi_nand_write_page(blk_i, pg_i, (void *)g_wbuf, PAGE_SIZE, writemode);
		}
	}

	for (blk_i = START_BLK_ID; blk_i < END_BLK_ID ; blk_i = blk_i + blk_inc) {

		printf("read and compare blk %d\n", blk_i);

		for (pg_i = 0; pg_i < NR_PG_PER_NATIVE_BLK_FOR_TESTING ; pg_i++) {

//			printf("read  pg_i %d \n", pg_i);
//			memset((void*)g_wbuf, 0xff, PAGE_SIZE);
			memset_dword_inc((void*)g_wbuf, ((blk_i << 8) | pg_i), PAGE_SIZE);
			bm_spi_nand_read_page(blk_i, pg_i, (void *)g_rbuf, PAGE_SIZE, readmode);
//			dump_hex("g_rbuf:", g_rbuf, 16);
			if(compare_buf(g_rbuf, g_wbuf, PAGE_SIZE))
				ret = -1;
		}
	}

#else
//		for (blk_i = START_BLK_ID; blk_i <= END_BLK_ID ; blk_i++) 
		blk_i = 1;

		for (i = 0; i < 100; i++) {
			printf("\n iter %d------------------------\n", i);
//			bm_spi_nand_erase_block(blk_i);
	
	//		for (pg_i = 0; pg_i < NR_PG_PER_NATIVE_BLK ; pg_i++) {
			for (pg_i = 0; pg_i < NR_PG_PER_NATIVE_BLK_FOR_TESTING ; pg_i++) {
	
	//			memset((void*)g_wbuf, pg_i* 0x15 + blk_i, PAGE_SIZE);
//				memset((void*)g_wbuf, pg_i + blk_i + i, PAGE_SIZE);
				memset_dword((void*)g_wbuf, ((blk_i << 8) | pg_i), PAGE_SIZE);
				dump_hex("g_wbuf1:", g_wbuf, 16);
				bm_spi_nand_write_page(blk_i, pg_i, (void *)g_wbuf, PAGE_SIZE, 0);
			}
		
	//		for (pg_i = 0; pg_i < NR_PG_PER_NATIVE_BLK ; pg_i++) {
//			for (pg_i = 0; pg_i < NR_PG_PER_NATIVE_BLK_FOR_TESTING ; pg_i++) {
//				memset((void*)g_rbuf, 0, PAGE_SIZE);
////				memset((void*)g_wbuf, pg_i + blk_i + i, PAGE_SIZE);
//				memset_dword((void*)g_wbuf, ((blk_i << 8) | pg_i), PAGE_SIZE);
//				bm_spi_nand_read_page(blk_i, pg_i, (void *)g_rbuf, PAGE_SIZE, 0);
//				dump_hex("g_rbuf1:", g_rbuf, 16);
//				compare_buf(g_rbuf, g_wbuf, PAGE_SIZE);
//			}
		}

#endif

	return ret;
}
#endif

static int irq_handler(int irqn, void *priv)
{

	printf("%s, get irq=%d\n", __func__, irqn);
	disable_irq(irqn);

    return 0;
}

static int tc_999(int argc, char **argv)
{
	int i;
	char param[10][20], *argv_internal[10];

	for(i=0;i<10;i++){
		argv_internal[i] = param[i];
	}

	for(int clkdiv=0; clkdiv<4; clkdiv++){
		for(int cpha=0; cpha<2; cpha++){
			for(int cpol=0; cpol<2; cpol++){
				strcpy(param[1], "--clkdiv");
				strcpy(param[3], "--cpha");
				strcpy(param[5], "--cpol");
				param[2][0] = '1'+clkdiv;
				param[4][0] = '0'+cpha;
				param[6][0] = '0'+cpol;
				argc = 7;
				printf("%d, %d, %d\n", clkdiv, cpha, cpol);
				spi_nand_init(argc, argv_internal);

				spi_nand_read_id_test(0, 0);

				if(spi_nand_block_data_len_test(0, 0))
					return -1;

				for(int writemode=0; writemode<WRITE_MODE_NUM; writemode++){
					for(int readmode=0; readmode<READ_MODE_NUM; readmode++){
						strcpy(param[1], "--writemode");
						strcpy(param[3], "--readmode");
						param[2][0] = '0'+writemode;
						param[4][0] = '0'+readmode;
						argc = 5;
						if(spi_nand_whole_nand_epr_test(argc, argv_internal))
							return -1;
					}
				}
			}
		}
	}
	return 0;
}

static struct cmd_entry test_cmd_list[] __attribute__ ((unused)) = {
#ifdef BUILD_TEST_CASE_SLT_spi
//    { tc_000, spi_nand_test_for_slt, 0, "SPI NAND test for SLT" },
#else
	{.name = "nandinit", .func = spi_nand_init, .arg_count = 6, .usage = "SPI NAND Init" },
	{.name = "tc_000", .func = spi_nand_read_id_test, .arg_count = 0, .usage = "SPI NAND read ID test" },
//	{ tc_001, spi_nand_block_data_ecc_test, 0, "SPI NAND data ecc test" },
	{"tc_002", spi_nand_block_data_len_test, 0, "SPI NAND data len test" },
//	{ tc_003, spi_nand_nand_cache_test, 0, "SPI NAND NAND cache test" },
//	{ tc_004, spi_nand_block_xfer_mode_test, 0, "SPI NAND transfer mode test" },
//	{ tc_005, spi_nand_defect_scan_test, 0, "SPI NAND defect scan test" },
	{"tc_006", spi_nand_whole_nand_epr_test, 4, "SPI NAND whole nand test" },
	// 典型测试用例
	{"tc_999", tc_999, 0, "typical test case" },
#endif
};

int testcase_spi_nand(void)
{
	int i;

	printf("\n------------ SPI NAND test start ------------\n\n");

#ifdef PROGRAM_LOAD_X4
	printf("PROGRAM_LOAD_X4\n");
#endif
	request_irq(SPI_NAND_INTR, irq_handler, 0, "SPI_NAND int", NULL);
	spi_nand_init(0, 0);
	printf("Commands:\nname\targ_count\tusage:\n");
	for(i = 0;i < ARRAY_SIZE(test_cmd_list);i++) {
		command_add(&test_cmd_list[i]);
		printf("%s\t%d\t%s\n",test_cmd_list[i].name, test_cmd_list[i].arg_count, test_cmd_list[i].usage);
	}

	char param[4];
	while(1){
		printf("run all test? input y or n\n");
		cli_scanf("%s", param);
		if(0 == strcmp("y", param)){
			for(i = 0;i < ARRAY_SIZE(test_cmd_list);i++) {
				printf("%s\t%s\n",test_cmd_list[i].name, test_cmd_list[i].usage);
				test_cmd_list[i].func(0, 0);
			}
		}
		cli_simple_loop();
	}
	return 0;
}


#ifndef BUILD_TEST_CASE_all
int testcase_main()
{
	return testcase_spi_nand();
}
#endif

