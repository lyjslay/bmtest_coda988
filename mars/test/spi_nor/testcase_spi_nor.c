#include "module_testcase.h"
#include "system_common.h"
#include "mmio.h"
#include "timer.h"
#include "testcase_spi_nor.h"
#include "bm_spi_nor.h"
#include "cli.h"
#include <string.h>

#define TEST_ADDR	(0x0)
#define DISP_GAP	0x100

static uint8_t w_buf[SPI_SECTOR_SIZE];
static uint8_t r_buf[SPI_SECTOR_SIZE];

typedef int (*handler_t)();
struct testcase {
	handler_t handler;
	char *name;
};

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

struct testcase spi_testcases[] = {
#ifdef BUILD_TEST_CASE_SLT_spi_nor
	{ .handler = spi_test_for_slt, .name = "SPI NOR test for SLT" },
#else
	// { .handler = spi_erase_test, .name = "SPI NOR erase test" },
	// { .handler = spi_prog_test, .name = "SPI NOR program test" },
	// { .handler = spi_dmmr_read_test, .name = "SPI NOR DMMR mode read test" },
	// { .handler = spi_dmmr_read_random_test, .name = "SPI NOR DMMR random read test" },
	// { .handler = spi_clk_param_test, .name = "SPI NOR clock parameter test" },
	// { .handler = spi_phase_test, .name = "SPI NOR CPOL/CPHA test" },
	// { .handler = spi_read_test, .name = "SPI NOR read test" },
	// { .handler = spi_interrupt_test, .name = "SPI NOR interrupt test" },

// Optional use:
	// { .handler = spi_prog_wholechip_test, .name = "SPI NOR program whole chip test" },
	// { .handler = spi_prog_fip, .name = "SPI NOR program fip.bin" },
#endif
};

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

	/* Power control */
//	mmio_setbits_32(BASE_PINMUX + 0x0, 0x1); /* Set 1.8V for MS Control Group 0 */
	mdelay(1);	/* delay 1ms for IO power steady */
}

void spi_init()
{
	uint32_t id;

	printf("%s base addr 0x%x\n", __func__, BASE_SPINOR);

	if (test_st_off + test_sz > dev_sz) {
		printf("test range error!\n");
		return;
	}

	spi_set_pinmux();

	// clk_div=1, cpha=0, cpol=0, edge=0, int_en=0
	bm_spi_init(BASE_SPINOR, 1, 0, 0, 1, 0);

	bm_spi_reset_nor(BASE_SPINOR);

	// id = spi_read_status(BASE_SPINOR);
	// printf("status = 0x%x\n", id);

	id = bm_spi_read_id(BASE_SPINOR);
	printf("Flash ID = 0x%x\n", id);

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
		case SPINOR_ID_N25Q128:
			g_sector_size = 64;
			printf("Micron N25Q128! (sector size = %d KB)\n", g_sector_size);
			break;
		case SPINOR_ID_GD25LQ128:
			g_sector_size = 64;
			printf("Micron GD25LQ128! (sector size = %d KB)\n", g_sector_size);
			break;

		case SPINOR_ID_MT25Q256:
			g_sector_size = 64;
			printf("Micron MT25Q256! (sector size = %d KB), dev size %d KB\n",
				g_sector_size, dev_sz);
			break;
		case SPINOR_ID_W25Q512JVEIQ:
			g_sector_size = 64;
			printf("Winbond W25Q512JVEIQ! (sector size = %d KB), dev size %d KB\n",
				g_sector_size, dev_sz);
			break;
		case SPINOR_ID_GD25Q128:
			g_sector_size = 64;
			printf("GigaDevice GD25Q128! (sector size = %d KB), dev size %d KB\n",
				g_sector_size, dev_sz);
			break;
		default:
			printf("Error: unknown spi flash ID!\n");
			break;
	}
	printf("\n");

	// bm_spi_nor_sfdp(BASE_SPINOR);
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
#else
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

int spi_erase_test()
{
	uint32_t test_addr, i = 0;
	uint8_t buf[256];

	//
	// 3-byte mode: erase(D8h) + read(03h)
	//
	test_addr = 0x0;
	printf("Test 3-byte mode: erase(D8h) + read(03h)\n");
	memset(buf, 0, SPI_SECTOR_SIZE);  //256 Bytes
	bm_spi_flash_erase_sector(BASE_SPINOR, test_addr, ADDR_3B, SIZE_64KB);
	bm_spi_flash_read_sector(BASE_SPINOR, test_addr, ADDR_3B, buf);

	for (i=0; i<SPI_SECTOR_SIZE; i+=4) {
		if ( buf[i] != 0xFF ) {
			printf("Error: buf[%d] = 0x%x\n", i, buf[i]);
			return 0;
		}
	}

	if (dev_sz > 16*1024) {
		//
		// 4-byte mode: erase(DCh) + read(13h)
		//
		test_addr = 0x1000000;	// start from 16MB
		printf("Test 4-byte mode: erase(DCh) + read(13h)\n");
		memset(buf, 0, SPI_SECTOR_SIZE);  //256 Bytes
		bm_spi_flash_erase_sector(BASE_SPINOR, test_addr, ADDR_4B, SIZE_64KB);
		bm_spi_flash_read_sector(BASE_SPINOR, test_addr, ADDR_4B, buf);

		for (i=0; i<SPI_SECTOR_SIZE; i+=4) {
			if ( buf[i] != 0xFF ) {
				printf("Error: buf[%d] = 0x%x\n", i, buf[i]);
				return 0;
			}
		}
	}
	return 1;
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

int spi_prog_test()
{
	uint32_t test_addr;

	//
	// 3-byte mode: erase(D8h) + read(03h)
	//
	test_addr = 0x0;	// start from offset 0MB

	printf("Test 3-byte mode: erase(D8h) + program(02h) + read(03h)\n");
	bm_spi_flash_erase_sector(BASE_SPINOR, test_addr, ADDR_3B, SIZE_64KB);

	memset_dword_inc((void*)w_buf, 0, SPI_SECTOR_SIZE);
	// dump_hex("w_buf", w_buf, SPI_SECTOR_SIZE);
	bm_spi_flash_program_sector(BASE_SPINOR, test_addr, SPI_SECTOR_SIZE, ADDR_3B, w_buf);

	memset(r_buf, 0, SPI_SECTOR_SIZE);
	bm_spi_flash_read_sector(BASE_SPINOR, test_addr, ADDR_3B, r_buf);

	if (memcmp(w_buf, r_buf, SPI_SECTOR_SIZE) == 0) {
		printf("pass\n");
	}
	else {
		dump_hex("w_buf", w_buf, SPI_SECTOR_SIZE);
		dump_hex("r_buf", r_buf, SPI_SECTOR_SIZE);
		printf("failed\n");
		return 0;
	}

	if (dev_sz > 16*1024) {
		//
		// 4-byte mode: erase(DCh) + read(13h)
		//
		test_addr = 0x01000000;	// start from offset 16MB

		printf("Test 4-byte mode: erase(DCh) + program(12h) + read(13h)\n");
		bm_spi_flash_erase_sector(BASE_SPINOR, test_addr, ADDR_4B, SIZE_64KB);

		memset_dword_inc((void*)w_buf, 0, SPI_SECTOR_SIZE);
		// dump_hex("w_buf", w_buf, SPI_SECTOR_SIZE);
		bm_spi_flash_program_sector(BASE_SPINOR, test_addr, SPI_SECTOR_SIZE, ADDR_4B, w_buf);

		memset(r_buf, 0, SPI_SECTOR_SIZE);
		bm_spi_flash_read_sector(BASE_SPINOR, test_addr, ADDR_4B, r_buf);

		if (memcmp(w_buf, r_buf, SPI_SECTOR_SIZE) == 0) {
			printf("pass\n");
		}
		else {
			dump_hex("w_buf", w_buf, SPI_SECTOR_SIZE);
			dump_hex("r_buf", r_buf, SPI_SECTOR_SIZE);
			printf("failed\n");
			return 0;
		}
	}

	return 1;
}

int spi_prog_wholechip_test()
{
	uint32_t test_addr;

	//
	// 3-byte mode: erase(D8h) + read(03h)
	//
	printf("Test 3-byte mode: erase(D8h) + program(02h) + read(03h)\n");
	for (test_addr=0; test_addr<16*SIZE_1MB; test_addr+=SIZE_64KB) {
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

	if (dev_sz > 16*1024) {
		//
		// 4-byte mode: erase(DCh) + read(13h)
		//
		printf("Test 4-byte mode: erase(DCh) + program(12h) + read(13h)\n");
		for (test_addr=16*SIZE_1MB; test_addr<32*SIZE_1MB; test_addr+=SIZE_64KB) {
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
		printf("failed\n");
	}

	return 1;
}

int spi_interrupt_test()
{
	uint32_t test_addr = 0x0;

	// init and hook interrupt handler
	// clk_div=1, cpha=0, cpol=0, edge=1, int_en=1
	bm_spi_init(BASE_SPINOR, 1, 0, 0, 1, 1);

	bm_write_int_flag(0);
	
	printf("Erase one 64KB sector, address 0x%x\n", test_addr);
	bm_spi_flash_erase_sector(BASE_SPINOR, test_addr, ADDR_3B, SIZE_64KB);

	// memset_dword_inc((void*)w_buf, 0, SPI_SECTOR_SIZE);
	// bm_spi_flash_program_sector(BASE_SPINOR, test_addr, SPI_SECTOR_SIZE, ADDR_3B, w_buf);

	// always return 1, check by UART log
	return 1;
}

int spi_dmmr_read_test()
{
	// How many 64KB block
	// uint32_t nr_dev_sector = dev_sz / g_sector_size;
	uint32_t i, j;
	uint32_t off = 0; // unit is byte
	uint8_t read_cmd;
	// const uint8_t cmd_sel[6] = {0x03, 0x6B, 0xEB, 0x13, 0x6C, 0xEC};
	const uint8_t cmd_sel[3] = {0x03, 0x6B, 0xEB};

	for (j=0; j<3; j++) {

		read_cmd = cmd_sel[j];
#ifdef SPI_NOR1
		for (i = 0; i < 4*SIZE_1MB; i+=SPI_BLOCK_SIZE) {
#else
		for (i = 0; i < 16*SIZE_1MB; i+=SPI_BLOCK_SIZE) {
#endif
			off = i;

			memset(w_buf, 0, SPI_SECTOR_SIZE);

			// 03h
			bm_spi_flash_read_sector(BASE_SPINOR, off, ADDR_3B, w_buf);

			printf("DMMR read(%02Xh) sector addr 0x%x\n", read_cmd, off);

			// enable DMMR (direct memory mapping read)

			bm_spi_byte4en(BASE_SPINOR, read_cmd);
			memset(r_buf, 0, SPI_SECTOR_SIZE);
			memcpy(r_buf, PTR_INC(BASE_SPINOR, off), SPI_SECTOR_SIZE);

			if (memcmp(w_buf, r_buf, SPI_SECTOR_SIZE) == 0) {
				// dump_hex("w_buf", w_buf, SPI_SECTOR_SIZE);
				// dump_hex("r_buf", r_buf, SPI_SECTOR_SIZE);
			}
			else {
				dump_hex("w_buf", w_buf, SPI_SECTOR_SIZE);
				dump_hex("r_buf", r_buf, SPI_SECTOR_SIZE);
				printf("failed\n");
				return 0;
			}
			off += SPI_SECTOR_SIZE;
		}
	}

#ifndef SPI_NOR1
	if (dev_sz > 16*1024) {
		for (j=3; j<6; j++) {

			read_cmd = cmd_sel[j];

			for (i = 16*SIZE_1MB; i < 32*SIZE_1MB; i+=SPI_BLOCK_SIZE) {
				off = i;

				memset(w_buf, 0, SPI_SECTOR_SIZE);

				// 13h
				bm_spi_flash_read_sector(BASE_SPINOR, off, ADDR_4B, w_buf);

				printf("DMMR read(%02Xh) sector addr 0x%x\n", read_cmd, off);

				// enable DMMR (direct memory mapping read)

				bm_spi_byte4en(BASE_SPINOR, read_cmd);
				memset(r_buf, 0, SPI_SECTOR_SIZE);
				memcpy(r_buf, PTR_INC(BASE_SPINOR, off), SPI_SECTOR_SIZE);

				if (memcmp(w_buf, r_buf, SPI_SECTOR_SIZE) == 0) {
					// dump_hex("w_buf", w_buf, SPI_SECTOR_SIZE);
					// dump_hex("r_buf", r_buf, SPI_SECTOR_SIZE);
				}
				else {
					dump_hex("w_buf", w_buf, SPI_SECTOR_SIZE);
					dump_hex("r_buf", r_buf, SPI_SECTOR_SIZE);
					printf("failed\n");
					return 0;
				}
				off += SPI_SECTOR_SIZE;
			}
		}
	}
#endif
	return 1;
}

extern void *memcpy_dword(void *dst, const void *src, size_t len);
int spi_dmmr_read_random_test()
{
	uint8_t read_cmd;

	// How many 64KB block
	uint32_t nr_dev_sector = dev_sz / g_sector_size;
	uint32_t nr_dev_subsector = (dev_sz * 1024)/ SPI_SECTOR_SIZE;

	uint32_t seed = 123513123;
	uint32_t loop = 100;
	uint32_t i = 0;
	uint32_t off = 0; // unit is byte

	printf("nr_dev_sector %d\n", nr_dev_sector);
	printf("how many 256-byte %d\n", nr_dev_subsector);

	// enable DMMR (direct memory mapping read)
	read_cmd = 0xEB;
	bm_spi_byte4en(BASE_SPINOR, read_cmd);

	for (i = 0; i < loop; i++) {
		off = (i * seed * loop) % nr_dev_subsector;
		off = off * SPI_SECTOR_SIZE;
		printf("test offset 0x%x\n", off);

		if (i % DISP_GAP == 0)
			printf("do ramdon dmmr read 0x%x\n", off);

		// memset_dword_inc((void*)w_buf, off, SPI_SECTOR_SIZE);
		memset(w_buf, 0, SPI_SECTOR_SIZE);
		bm_spi_flash_read_sector(BASE_SPINOR, off, ADDR_3B, w_buf);

		// memcpy(r_buf, PTR_INC(BASE_SPINOR, off), SPI_SECTOR_SIZE);
		memset(r_buf, 0, SPI_SECTOR_SIZE);
		memcpy_dword(r_buf, PTR_INC(BASE_SPINOR, off), SPI_SECTOR_SIZE);

		if (memcmp(w_buf, r_buf, SPI_SECTOR_SIZE) == 0) {
			// pass
		}
		else {
			dump_hex("w_buf", w_buf, SPI_SECTOR_SIZE);
			dump_hex("r_buf", r_buf, SPI_SECTOR_SIZE);
			printf("failed\n");
			return -1;
		}
	}
	return 1;
}

int spi_phase_test()
{
	uint32_t i = 0;
	uint8_t buf[256];

	//
	// cpha = 0, cpol = 0
	//
	printf("%s: CPHA:0, CPOL:0\n", __func__);
	// clk_div=1, cpha=0, cpol=0, edge=1, int_en=0
	bm_spi_init(BASE_SPINOR, 1, 0, 0, 1, 0);

	memset(buf, 0, SPI_SECTOR_SIZE);
	bm_spi_flash_read_sector(BASE_SPINOR, TEST_ADDR, ADDR_3B, buf);

	// check first 256 bytes
	for (i=0; i<256; i+=4) {
		if ( buf[i] != i/4 ) {
			printf("Error: buf[%d] = 0x%x, expected 0x%x\n", i, buf[i], i/4);
			return 0;
		}
	}

	//
	// cpha = 1, cpol = 1
	//
	printf("%s: CPHA:1, CPOL:1\n", __func__);
	// clk_div=1, cpha=1, cpol=1, edge=1, int_en=0
	bm_spi_init(BASE_SPINOR, 1, 1, 1, 1, 0);
	memset(buf, 0, SPI_SECTOR_SIZE);
	bm_spi_flash_read_sector(BASE_SPINOR, TEST_ADDR, ADDR_3B, buf);

	// check first 256 bytes
	for (i=0; i<256; i+=4) {
		if ( buf[i] != i/4 ) {
			printf("Error: buf[%d] = 0x%x, expected 0x%x\n", i, buf[i], i/4);
			return 0;
		}
	}
	return 1;
}

// test clock div and edge sampling type
int spi_clk_param_test()
{
	uint32_t i, j, k, test_addr;
	uint8_t buf[256], read_cmd;
	uint8_t dmmr = 1;	// 0: non-dmmr  1: dmmr

	// Note: Must erase+prog test_addr with dword-inc pattern first
	test_addr = 0;

	for (k=0; k<2; k++) {	// 0: positive edge, 1: negative edge

		for (j=1; j<9; j++) {	// sck_div: 1(75MHz) - 9(15MHz)

			// Clean memory buffer
			memset(buf, 0, SPI_SECTOR_SIZE);

			if (j > 4) {
				// clk_div=j, cpha=0, cpol=0, edge=0, int_en=0
				bm_spi_init(BASE_SPINOR, j, 0, 0, 0, 0);
			} else {
				// clk_div=j, cpha=0, cpol=0, edge=1, int_en=0
				bm_spi_init(BASE_SPINOR, j, 0, 0, 1, 0);
			}

			if (dmmr) {
				// enable DMMR (direct memory mapping read)
				read_cmd = 0xEB;
				bm_spi_byte4en(BASE_SPINOR, read_cmd);

				memcpy(buf, PTR_INC(BASE_SPINOR, test_addr), SPI_SECTOR_SIZE);
			} else {
				// 03h
				bm_spi_flash_read_sector(BASE_SPINOR, test_addr, ADDR_3B, buf);
			}

			// check first 256 bytes
			for (i=0; i<256; i+=4) {
				if ( buf[i] != i/4 ) {
					printf("Error: buf[%d] = 0x%x, expected 0x%x\n", i, buf[i], i/4);
					return 0;
				}
			}
			printf("sck_div %d + sample edge %d test pass!\n", j, k);
		}
	}

#ifndef SPI_NOR1
	if (dev_sz > 16*1024) {

		// Note: Must erase+prog test_addr with dword-inc pattern first
		test_addr = 0x1000000;

		for (k=0; k<2; k++) {	// 0: positive edge, 1: negative edge

			for (j=1; j<9; j++) {	// sck_div: 1(75MHz) - 9(15MHz)

				// Clean memory buffer
				memset(buf, 0, SPI_SECTOR_SIZE);

				// clk_div=j, cpha=0, cpol=0, edge=k, int_en=0
				bm_spi_init(BASE_SPINOR, j, 0, 0, k, 0);

				if (dmmr) {
					// enable DMMR (direct memory mapping read)
					read_cmd = 0xEC;
					bm_spi_byte4en(BASE_SPINOR, read_cmd);

					memcpy(buf, PTR_INC(BASE_SPINOR, test_addr), SPI_SECTOR_SIZE);
				} else {
					// 13h
					bm_spi_flash_read_sector(BASE_SPINOR, test_addr, ADDR_4B, buf);
				}

				// check first 256 bytes
				for (i=0; i<256; i+=4) {
					if ( buf[i] != i/4 ) {
						printf("Error: buf[%d] = 0x%x, expected 0x%x\n", i, buf[i], i/4);
						return 0;
					}
				}
				printf("sck_div %d + sample edge %d test pass!\n", j, k);
			}
		}
	}
#endif
	return 1;
}
#endif

int testcase_spi_nor(void)
{
	int i;

	printf("\n------------ SPI NOR test start ------------\n\n");

	spi_init();	// read id

	for (i = 0; i < sizeof(spi_testcases) / sizeof(spi_testcases[0]); ++i) {
		printf("[%d] %s start\n", i, spi_testcases[i].name);
		if ( spi_testcases[i].handler() == 0 ) {
			printf("[%d] %s failed\n", i, spi_testcases[i].name);
			return -1;
		}
		printf("[%d] %s pass\n\n", i, spi_testcases[i].name);
	}

	printf("\n------------ SPI NOR test done ------------\n");

	return 0;
}

module_testcase("1", testcase_spi_nor);

#ifndef BUILD_TEST_CASE_all
int testcase_main(void)
{
	return testcase_spi_nor();
}
#endif
