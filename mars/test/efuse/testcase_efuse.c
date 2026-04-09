#include "module_testcase.h"
#include "system_common.h"
#include "mmio.h"
#include "testcase_efuse.h"

static void efuse_mode_md_write(uint32_t val)
{
	uint32_t mode = mmio_read_32(EFUSE_BASE + REG_EFUSE_MODE);
	uint32_t new = (mode & 0xfffffffc) | (val & 0x3);

	mmio_write_32(EFUSE_BASE + REG_EFUSE_MODE, new);
}

static void efuse_mode_wait_ready(void)
{
	while (mmio_read_32(EFUSE_BASE + REG_EFUSE_MODE) != 0x80)
		;
}

static void efuse_mode_reset(void)
{
	mmio_write_32(EFUSE_BASE + REG_EFUSE_MODE, 0);
	efuse_mode_wait_ready();
}

static uint32_t make_adr_val(uint32_t address, uint32_t bit_i)
{
	const uint32_t address_mask = (1 << EFUSE_NUM_ADDRESS_BITS) - 1;

	return (address & address_mask) |
		((bit_i & 0x1f) << EFUSE_NUM_ADDRESS_BITS);
}

static uint32_t plat_bm_efuse_read(uint32_t address)
{
	uint32_t adr_val = make_adr_val(address, 0);

	efuse_mode_reset();
	mmio_write_32(EFUSE_BASE + REG_EFUSE_ADR, adr_val);
	efuse_mode_md_write(0x2);
	efuse_mode_wait_ready();
	return mmio_read_32(EFUSE_BASE + REG_EFUSE_RD_DATA);
}

int testcase_efuse(void)
{
	int i = 0;

	printf("Dump data from efuse...\n");
	for (i = 0; i < (1 << EFUSE_NUM_ADDRESS_BITS); i++) {
		if ((i % 8) == 0)
			printf("0x%02X: ", i);

		printf("%08X ", plat_bm_efuse_read(i));

		if ((i % 8) == 7)
			printf("\n");
	}

	return 0;
}

module_testcase("1", testcase_efuse);

#ifndef BUILD_TEST_CASE_all
int testcase_main(void)
{
	return testcase_efuse();
}
#endif
