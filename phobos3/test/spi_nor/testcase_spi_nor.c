#include "module_testcase.h"
#include "system_common.h"
#include "mmio.h"
#include "timer.h"
#include "testcase_spi_nor.h"
#include "bm_spi_nor.h"
#include "cli.h"
#include <string.h>
#include "command.h"

#define TEST_ADDR	(0x0)

#define _3B_TEST_ADDR	(0x0)
#define _4B_TEST_ADDR	(0x1000000U)

static uint8_t w_buf[SPI_SECTOR_SIZE];
static uint8_t r_buf[SPI_SECTOR_SIZE];

uint32_t g_sector_size; // size in KB

#ifdef SPI_NOR1
static uint32_t dev_sz = 4 * 1024; // FPGA spinor1 is limited to 4MB
static uint32_t test_sz = 4 * 1024; // size in KB
#else
static uint32_t dev_sz = 16 * 1024; // size in KB
static uint32_t test_sz = 16 * 1024; // size in KB
#endif

// test ragne must < dev size 32MB
static uint32_t test_st_off = 0 * 1024; // size in KB


int spi_test_for_slt();
int spi_erase_test();
int spi_prog_test();
int spi_prog_wholechip_test();
int spi_read_test();
int spi_phase_test();
int spi_dmmr_read_test();
int spi_dmmr_read_random_test();
int spi_clk_param_test();
int spi_prog_fip();
int spi_interrupt_test();

#define ROUND_UP_DIV(a, b) (((a) + (b)-1) / (b))
#define PTR_INC(base, offset) (void *)((uint8_t *)(base) + (offset))

extern void memset_dword_inc(void *buff, u32 start, u32 size);

void spi_set_pinmux()
{
#ifdef SPI_NOR1
	printf("%s for SPINOR 1\n", __func__);
	pinmux_config(PINMUX_SPI_NOR1);
#else
	printf("%s for SPINOR 0\n", __func__);
	pinmux_config(PINMUX_SPI_NOR);
#endif

	mdelay(1);	/* delay 1ms for IO power steady */
}

void spi_init()
{
	printf("%s base addr 0x%x\n", __func__, BASE_SPINOR);

	if (test_st_off + test_sz > dev_sz) {
		printf("test range error!\n");
		return;
	}

	spi_set_pinmux();

	/* clk_div=1, cpha=0, cpol=0, edge=0, int_en=0 */
	bm_spi_init(BASE_SPINOR, 1, 0, 0, 1, 0);

	bm_spi_reset_nor(BASE_SPINOR);
}

int spi_nor_read_id(int argc, char **argv)
{
	uint32_t id;

	id = bm_spi_read_id(BASE_SPINOR);
	printf("\tFlash ID = 0x%x\n", id);

	if ((id & 0xFF) == 0x18)
		dev_sz = 16 * 1024;	// 16MB in KB
	else if ((id & 0xFF) == 0x19)
		dev_sz = 32 * 1024;	// 32MB in KB
	else if ((id & 0xFF) == 0x20)
		dev_sz = 64 * 1024;	// 64MB in KB

	switch (id) {
		case SPINOR_ID_M25P128:
			g_sector_size = 256;
			printf("Micron M25P128! (sector size = %d KB)\n", g_sector_size);
			break;

		default:
			g_sector_size = 64;
			break;
	}
	return 0;
}

#ifdef BUILD_TEST_CASE_SLT_spi_nor
int spi_test_for_slt()
{
	uint32_t i = 0;

	printf("Erase sector\n");

	bm_spi_flash_erase_sector(BASE_SPINOR, TEST_ADDR, ADDR_3B, SIZE_64KB);

	bm_spi_flash_read_sector(BASE_SPINOR, TEST_ADDR, ADDR_3B, r_buf);

	for (i=0; i<256; i+=4) {
		if ( r_buf[i] != 0xFF ) {
			printf("Error: buf[%d] = 0x%x\n", i, r_buf[i]);
			return 0;
		}
	}

	printf("Write pattern to address 0x%X\n", TEST_ADDR);

	memset(w_buf, 0x5A, SPI_SECTOR_SIZE);

	bm_spi_flash_program_sector(BASE_SPINOR, TEST_ADDR, SPI_SECTOR_SIZE, ADDR_3B, w_buf);

	memset(r_buf, 0, SPI_SECTOR_SIZE);

	bm_spi_flash_read_sector(BASE_SPINOR, TEST_ADDR, ADDR_3B, r_buf);

	for (i=0; i<256; i+=4) {
		if ( r_buf[i] != 0x5A ) {
			printf("Error: buf[%d] = 0x%x\n", i, r_buf[i]);
			return 0;
		}
	}

	return 1;
}
#endif

static void dump_hex(char *desc, void *addr, int len)
{
	int i;
	unsigned char buff[17];
	unsigned char *pc = (unsigned char *)addr;

	/* Output description if given. */
	if (desc != NULL)
		printf("%s:\n", desc);

	/* Process every byte in the data. */
	for (i = 0; i < len; i++) {
		/* Multiple of 16 means new line (with line offset). */
		if ((i % 16) == 0) {
			/* Just don't print ASCII for the zeroth line. */
			if (i != 0)
				printf("  %s\n", buff);

			/* Output the offset. */
			printf("  %x ", i);
		}

		/* Now the hex code for the specific character. */
		printf(" %x", pc[i]);

		/* And store a printable ASCII character for later. */
		if ((pc[i] < 0x20) || (pc[i] > 0x7e))
			buff[i % 16] = '.';
		else
			buff[i % 16] = pc[i];
		buff[(i % 16) + 1] = '\0';
	}

	/* Pad out last line if not exactly 16 characters. */
	while ((i % 16) != 0) {
		printf("   ");
		i++;
	}

	/* And print the final ASCII bit. */
	printf("  %s\n", buff);
}

int spi_erase_test(int argc, char **argv)
{
	uint32_t test_addr, i = 0;
	uint8_t buf[SPI_SECTOR_SIZE];

	/* 3-byte mode: erase(D8h) + read(03h) */
	test_addr = _3B_TEST_ADDR;
	printf("\tTest 3-byte mode: erase(D8h) + read(03h)\n");
	memset(buf, 0, SPI_SECTOR_SIZE);
	bm_spi_flash_erase_sector(BASE_SPINOR, test_addr, ADDR_3B, SIZE_64KB);
	bm_spi_flash_read_sector(BASE_SPINOR, test_addr, ADDR_3B, buf);

	for (i = 0; i < SPI_SECTOR_SIZE; i += 4) {
		if ( buf[i] != 0xFF ) {
			printf("Error: buf[%d] = 0x%x\n", i, buf[i]);
			return 1;
		}
	}

	/* 4-byte mode: erase(DCh) + read(13h) */
	if (dev_sz > 16 * 1024) {
		/* start from 16MB */
		test_addr = _4B_TEST_ADDR;
		printf("\tTest 4-byte mode: erase(DCh) + read(13h)\n");
		memset(buf, 0, SPI_SECTOR_SIZE);
		bm_spi_flash_erase_sector(BASE_SPINOR, test_addr, ADDR_4B, SIZE_64KB);
		bm_spi_flash_read_sector(BASE_SPINOR, test_addr, ADDR_4B, buf);

		for (i = 0; i < SPI_SECTOR_SIZE; i += 4) {
			if ( buf[i] != 0xFF ) {
				printf("Error: buf[%d] = 0x%x\n", i, buf[i]);
				return 1;
			}
		}
	}
	printf("\tErase test passed!\n");
	return 0;
}

int spi_prog_fip()
{
	uint32_t size_of_fip = 265 * 1024;
	uint32_t left_prog_bytes = 0;
	uint32_t num_of_blk, num_of_sec, i;
	uint8_t *fip_buf = (uint8_t *)0xA0000000;

	printf("[spinor] fip.bin is loaded to DDR addr: 0x%x\n", 0xA0000000);
	printf("[spinor] fip.bin length: %d\n", size_of_fip);
	// cli_scanf("%d\n", &size_of_fip);

	// calc number of blocks to erase
	if (size_of_fip % SIZE_64KB)
		num_of_blk = size_of_fip / SIZE_64KB + 1;
	else
		num_of_blk = size_of_fip / SIZE_64KB;

	for (i=0; i<num_of_blk; i++) {
		bm_spi_flash_erase_sector(BASE_SPINOR, i*SIZE_64KB, ADDR_3B, SIZE_64KB);
	}

	// calc number of sectors to program
	if (size_of_fip % SPI_SECTOR_SIZE)
		num_of_sec = size_of_fip / SPI_SECTOR_SIZE + 1;
	else
		num_of_sec = size_of_fip / SPI_SECTOR_SIZE;

	left_prog_bytes = size_of_fip;
	for (i=0; i<num_of_sec; i++) {
		if (left_prog_bytes <256) {
			bm_spi_flash_program_sector(BASE_SPINOR, i*SPI_SECTOR_SIZE, left_prog_bytes, ADDR_3B, fip_buf);
			left_prog_bytes -= left_prog_bytes;
			break;
		}
		else {
			bm_spi_flash_program_sector(BASE_SPINOR, i*SPI_SECTOR_SIZE, SPI_SECTOR_SIZE, ADDR_3B, fip_buf);
			left_prog_bytes -= SPI_SECTOR_SIZE;
			fip_buf += SPI_SECTOR_SIZE;
		}
	}
	printf("[spinor] fip.bin prog done (nor offset 0x%X ~ 0x%X)\n", 0, size_of_fip);
	return 1;
}

int gereral_write_test(uint32_t test_addr, uint32_t addr_mode)
{
	/* "Test 3-byte mode: erase(D8h) + program(02h) + read(03h) */
	bm_spi_flash_erase_sector(BASE_SPINOR, test_addr, addr_mode, SIZE_64KB);

	memset_dword_inc((void*)w_buf, 0, SPI_SECTOR_SIZE);
	/* make sure data */
	//dump_hex("w_buf", w_buf, SPI_SECTOR_SIZE);

	bm_spi_flash_program_sector(BASE_SPINOR, test_addr, SPI_SECTOR_SIZE, addr_mode, w_buf);

	memset(r_buf, 0, SPI_SECTOR_SIZE);
	bm_spi_flash_read_sector(BASE_SPINOR, test_addr, addr_mode, r_buf);

	if (memcmp(w_buf, r_buf, SPI_SECTOR_SIZE) != 0) {
		dump_hex("w_buf", w_buf, SPI_SECTOR_SIZE);
		dump_hex("r_buf", r_buf, SPI_SECTOR_SIZE);
		return -1;
	}

	return 0;
}

/* write 256 byte and read back
 *
 */
int spi_prog_test(int argc, char **argv)
{
	uint32_t test_addr, addr_mode;

	test_addr = _3B_TEST_ADDR;
	addr_mode = ADDR_3B;
	/* Test 3-byte mode: erase(D8h) + program(02h) + read(03h) */
	if(gereral_write_test(test_addr, ADDR_3B))
		printf("\t%d addr mode ===> programming test failed\n", addr_mode);
	else
		printf("\t%d addr mode ===> programming test passed\n", addr_mode);

	if (dev_sz > 16 * 1024) {
		/* start from offset 16MB */
		test_addr = _4B_TEST_ADDR;
		addr_mode = ADDR_4B;
		/* Test 4-byte mode: erase(DCh) + program(12h) + read(13h)\n") */
			if(gereral_write_test(test_addr, ADDR_4B))
				printf("\t%d addr mode ===> programming test failed\n", addr_mode);
			else
				printf("\t%d addr mode ===> programming test passed\n", addr_mode);
	}
	return 0;
}

int spi_prog_wholechip_test()
{
	uint32_t test_addr;

	//
	// 3-byte mode: erase(D8h) + read(03h)
	//
	printf("Test 3-byte mode: erase(D8h) + program(02h) + read(03h)\n");
	for (test_addr = 0; test_addr < 16 * SIZE_1MB; test_addr += SIZE_64KB) {
		printf("3B test_addr = 0x%X\n", test_addr);
		bm_spi_flash_erase_sector(BASE_SPINOR, test_addr, ADDR_3B, SIZE_64KB);

		memset_dword_inc((void*)w_buf, 0, SPI_SECTOR_SIZE);
		// dump_hex("w_buf", w_buf, SPI_SECTOR_SIZE);
		bm_spi_flash_program_sector(BASE_SPINOR, test_addr, SPI_SECTOR_SIZE, ADDR_3B, w_buf);

		memset(r_buf, 0, SPI_SECTOR_SIZE);
		bm_spi_flash_read_sector(BASE_SPINOR, test_addr, ADDR_3B, r_buf);

		if (memcmp(w_buf, r_buf, SPI_SECTOR_SIZE) == 0) {
			printf("memcmp pass\n");
		}
		else {
			dump_hex("w_buf", w_buf, SPI_SECTOR_SIZE);
			dump_hex("r_buf", r_buf, SPI_SECTOR_SIZE);
			printf("failed\n");
			return 0;
		}
	}

	if (dev_sz > 16 * 1024) {
		//
		// 4-byte mode: erase(DCh) + read(13h)
		//
		printf("Test 4-byte mode: erase(DCh) + program(12h) + read(13h)\n");
		for (test_addr = 16 * SIZE_1MB; test_addr < 32 * SIZE_1MB; test_addr += SIZE_64KB) {
			printf("4B test_addr = 0x%X\n", test_addr);
			bm_spi_flash_erase_sector(BASE_SPINOR, test_addr, ADDR_4B, SIZE_64KB);

			memset_dword_inc((void*)w_buf, 0, SPI_SECTOR_SIZE);
			// dump_hex("w_buf", w_buf, SPI_SECTOR_SIZE);
			bm_spi_flash_program_sector(BASE_SPINOR, test_addr, SPI_SECTOR_SIZE, ADDR_4B, w_buf);

			memset(r_buf, 0, SPI_SECTOR_SIZE);
			bm_spi_flash_read_sector(BASE_SPINOR, test_addr, ADDR_4B, r_buf);

			if (memcmp(w_buf, r_buf, SPI_SECTOR_SIZE) == 0) {
				printf("memcmp pass\n");
			}
			else {
				dump_hex("w_buf", w_buf, SPI_SECTOR_SIZE);
				dump_hex("r_buf", r_buf, SPI_SECTOR_SIZE);
				printf("failed\n");
				return 0;
			}
		}
	}
	return 1;
}

int spi_read_test()
{
	memset_dword_inc((void*)w_buf, TEST_ADDR, SPI_SECTOR_SIZE);

	memset(r_buf, 0, SPI_SECTOR_SIZE);
	bm_spi_flash_read_sector(BASE_SPINOR, TEST_ADDR, ADDR_3B, r_buf);

	if (memcmp(w_buf, r_buf, SPI_SECTOR_SIZE) == 0) {
//		dump_hex("w_buf", w_buf, SPI_SECTOR_SIZE);
//		dump_hex("r_buf", r_buf, SPI_SECTOR_SIZE);
	}
	else {
		dump_hex("w_buf", w_buf, SPI_SECTOR_SIZE);
		dump_hex("r_buf", r_buf, SPI_SECTOR_SIZE);
		printf("\tRead test failed\n");
	}
	printf("\tRead test passed!\n");
	return 0;
}

int spi_interrupt_test(int argc, char *argv[])
{
	uint32_t test_addr = 0x0;

	/* init and hook interrupt handler */
	/* clk_div=1, cpha=0, cpol=0, edge=1, int_en=1 */
	bm_spi_init(BASE_SPINOR, 1, 0, 0, 1, 1);

	bm_write_int_flag(0);

	printf("\tErase one 64KB sector, address 0x%x\n", test_addr);
	bm_spi_flash_erase_sector(BASE_SPINOR, test_addr, ADDR_3B, SIZE_64KB);

	return 0;
}

struct read_cmd_group {
	uint8_t read_opcode;
	char *desc;
};

int general_dmmr_read(const struct read_cmd_group *read_group, uint32_t addr_mode)
{
	uint32_t i;
	uint32_t off = 0; // unit is byte
	uint8_t read_cmd;
	const struct read_cmd_group *read_cmds = read_group;

	for (; read_cmds->read_opcode; read_cmds++) {

		read_cmd = read_cmds->read_opcode;
#ifdef SPI_NOR1
		for (i = 0; i < 4 * SIZE_1MB; i += SPI_BLOCK_SIZE) {
#else
		for (i = 0; i < 16 * SIZE_1MB; i += SPI_BLOCK_SIZE) {
#endif
			off = i;
			memset(w_buf, 0, SPI_SECTOR_SIZE);
			/* 03h */
			bm_spi_flash_read_sector(BASE_SPINOR, off, addr_mode, w_buf);
			/* enable DMMR (direct memory mapping read) */
			bm_spi_byte4en(BASE_SPINOR, read_cmd);
			memset(r_buf, 0, SPI_SECTOR_SIZE);
			memcpy(r_buf, PTR_INC(BASE_SPINOR, off), SPI_SECTOR_SIZE);

			if (memcmp(w_buf, r_buf, SPI_SECTOR_SIZE) != 0) {
				printf("\tDmmr %s test failed, cmd:%#x \n", read_cmds->desc, read_cmds->read_opcode);
				dump_hex("w_buf", w_buf, SPI_SECTOR_SIZE);
				dump_hex("r_buf", r_buf, SPI_SECTOR_SIZE);
				return -1;
			}
			off += SPI_SECTOR_SIZE;
		}
		printf("\tDmmr %s test passed, cmd:%#x \n", read_cmds->desc, read_cmds->read_opcode);
	}
	return 0;
}

int spi_dmmr_read_test(int argc, char **argv)
{
	const struct read_cmd_group _3b_read_cmd[]= {
		{0x03, "Read Data"},
		{0x0b, "Fast Read"},
		{0x3b, "Dual Output Fast Read"},
		{0xbb, "Dual I/O Fast Read"},
		{0x6b, "Quad Output Fast Read"},
		{0xeb, "Quad I/O Fast Read"},
		{0}
	};

	const struct read_cmd_group _4b_read_cmd[]= {
		{0x13, "Read Data with 4-Byte Address"},
		{0x0c, "Fast Read with 4-Byte Address"},
		{0x3b, "Dual Output Fast Read"},
		{0xbb, "Dual I/O Fast Read"},
		{0x6b, "Quad Output Fast Read"},
		{0xeb, "Quad I/O Fast Read"},
		{0}
	};

	if (general_dmmr_read(_3b_read_cmd, ADDR_3B))
		return -1;

#ifndef SPI_NOR1
	if (dev_sz > 16 * 1024) {
		if (general_dmmr_read(_4b_read_cmd, ADDR_4B))
			return -1;
	}
#endif
	printf("\tDMMR all test passed!\n");
	return 0;
}

extern void *memcpy_dword(void *dst, const void *src, size_t len);
int spi_dmmr_read_random_test(int argc, char **argv)
{
	uint8_t read_cmd = 0xEB;
	uint32_t addr_mode = ADDR_3B;
	uint32_t nr_dev_subsector = (dev_sz * 1024) / SPI_SECTOR_SIZE;
	uint32_t seed = 123513123;
	uint32_t loop = 100;
	uint32_t i = 0;
	/* unit is byte */
	uint32_t off = 0;

	if (dev_sz > 16 * 1024) {
		addr_mode = ADDR_4B;
		read_cmd = 0xEC;
	}
	/* enable DMMR (direct memory mapping read) */
	bm_spi_byte4en(BASE_SPINOR, read_cmd);

	for (i = 0; i < loop; i++) {
		off = (i * seed * loop) % nr_dev_subsector;
		off = off * SPI_SECTOR_SIZE;

		memset(w_buf, 0, SPI_SECTOR_SIZE);
		bm_spi_flash_read_sector(BASE_SPINOR, off, addr_mode, w_buf);

		memset(r_buf, 0, SPI_SECTOR_SIZE);
		memcpy_dword(r_buf, PTR_INC(BASE_SPINOR, off), SPI_SECTOR_SIZE);

		if (memcmp(w_buf, r_buf, SPI_SECTOR_SIZE) != 0) {
			dump_hex("w_buf", w_buf, SPI_SECTOR_SIZE);
			dump_hex("r_buf", r_buf, SPI_SECTOR_SIZE);
			printf("\tramdon dmmr read failed\n");
			return -1;
		}
	}
	printf("\tramdon dmmr read passed!\n");
	return 0;
}

int spi_phase_test(int argc, char **argv)
{
	uint32_t i = 0;
	uint8_t buf[256];

	printf("\t%s: CPHA:0, CPOL:0\n", __func__);
	/* clk_div=1, cpha=0, cpol=0, edge=1, int_en=0 */
	bm_spi_init(BASE_SPINOR, 1, 0, 0, 1, 0);

	memset(buf, 0, SPI_SECTOR_SIZE);
	bm_spi_flash_read_sector(BASE_SPINOR, TEST_ADDR, ADDR_3B, buf);

	/* check first 256 bytes */
	for (i = 0; i < 256; i += 4) {
		if ( buf[i] != i / 4 ) {
			printf("\tError: buf[%d] = 0x%x, expected 0x%x\n", i, buf[i], i/4);
			return 0;
		}
	}

	printf("\t%s: CPHA:1, CPOL:1\n", __func__);
	/* clk_div=1, cpha=1, cpol=1, edge=1, int_en=0 */
	bm_spi_init(BASE_SPINOR, 1, 1, 1, 1, 0);
	memset(buf, 0, SPI_SECTOR_SIZE);
	bm_spi_flash_read_sector(BASE_SPINOR, TEST_ADDR, ADDR_3B, buf);

	/* check first 256 bytes */
	for (i = 0; i < 256; i += 4) {
		if ( buf[i] != i / 4 ) {
			printf("\tError: buf[%d] = 0x%x, expected 0x%x\n", i, buf[i], i/4);
			return 0;
		}
	}
	return 1;
}

int general_clk_test(uint32_t test_addr, uint8_t dmmr, uint32_t addr_mode)
{
	uint32_t i, j, k;
	uint8_t buf[SPI_SECTOR_SIZE], read_cmd;

	/* Note: Must erase+prog test_addr with dword-inc pattern first */
	test_addr = 0;
	/* 0: positive edge, 1: negative edge */
	for (k = 0; k < 2; k++) {
		/* sck_div: 1(75MHz) - 9(15MHz) */
		for (j = 1; j < 9; j++) {
			memset(buf, 0, SPI_SECTOR_SIZE);

			if (j > 4) {
				/* clk_div=j, cpha=0, cpol=0, edge=0, int_en=0 */
				bm_spi_init(BASE_SPINOR, j, 0, 0, 0, 0);
			} else {
				/* clk_div=j, cpha=0, cpol=0, edge=1, int_en=0 */
				bm_spi_init(BASE_SPINOR, j, 0, 0, 1, 0);
			}

			if (dmmr) {
				/* enable DMMR (direct memory mapping read) */
				read_cmd = 0xEB;
				bm_spi_byte4en(BASE_SPINOR, read_cmd);

				memcpy(buf, PTR_INC(BASE_SPINOR, test_addr), SPI_SECTOR_SIZE);
			} else {
				/* 03h */
				bm_spi_flash_read_sector(BASE_SPINOR, test_addr, addr_mode, buf);
			}

			/* check first 256 bytes */
			for (i = 0; i < 256; i += 4) {
				if ( buf[i] != i / 4 ) {
					printf("\tError: buf[%d] = 0x%x, expected 0x%x\n", i, buf[i], i / 4);
					return -1;
				}
			}
			printf("\tsck_div %d sample edge %d test pass!\n", j, k);
		}
	}
	return 0;
}

/* test clock div and edge sampling type */
int spi_clk_param_test(int args, char **argv)
{
	uint32_t test_addr;
	/* 0: non-dmmr  1: dmmr */
	uint8_t dmmr = 1;

	printf("\tNote: Must erase+prog test_addr with dword-inc pattern first\n");
	test_addr = 0;
	if (general_clk_test(test_addr, dmmr, ADDR_3B))
		return -1;

#ifndef SPI_NOR1
	if (dev_sz > 16 * 1024) {
		if (general_clk_test(test_addr, dmmr, ADDR_4B))
			return -1;
	}
#endif
	printf("\tAll clk param test passed!\n");
	return 0;
}

static struct cmd_entry test_cmd_list[] __attribute__ ((unused)) = {
	{"readid", spi_nor_read_id, 0, "read nor flash id\n"},
	{"erase_test", spi_erase_test, 0, "erase test\n"},
	{"write_test", spi_prog_test, 0, "write data and read back check\n"},
	{"read_test", spi_read_test, 0, "read test\n"},
	{"dmmr_read_test", spi_dmmr_read_test, 0, "dmmr read test"},
	{"dmmr_random_read_test", spi_dmmr_read_random_test, 0, "dmmr random read test"},
	{"clk_param_test", spi_clk_param_test, 0, "test clock div and edge sampling type"},
	{"spi_phase_test", spi_phase_test, 0, "CPHA, CPOL test"},
	{"irq_test", spi_interrupt_test, 0, "irq test"},
};

void print_help()
{
	printf("\tcmd: readid  desc: read nor flash id\n");
	printf("\tcmd: erase_test  desc: spi nor erase test\n");
	printf("\tcmd: write_test  desc: write data and read back check\n");
	printf("\tcmd: read_test  desc: write data and read back check\n");
	printf("\tcmd: dmmr_read_test  desc: dmmr read test\n");
	printf("\tcmd: dmmr_random_read_test  desc: dmmr random read test\n");
	printf("\tcmd: clk_param_test  desc: test clock div and edge sampling type\n");
	printf("\tcmd: spi_phase_test  desc: CPHA, CPOL mode test\n");
	printf("\tcmd: irq_test  desc: irq test\n");
}

int testcase_spi_nor(void)
{
	int i;

	printf("\n------------ SPI NOR test start ------------\n\n");

	spi_init();
	print_help();

	for(i = 0; i < sizeof(test_cmd_list) / sizeof(struct cmd_entry); i++)
		command_add(&test_cmd_list[i]);

	while(1)
		cli_simple_loop();
}

module_testcase("1", testcase_spi_nor);

#ifndef BUILD_TEST_CASE_all
int testcase_main(void)
{
	return testcase_spi_nor();
}
#endif
