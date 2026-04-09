#include "module_testcase.h"
#include "system_common.h"
#include "mmio.h"
#include "efuse.h"

uint32_t phyadrTransCowAdr(uint32_t addr)
{
	return (addr >> 1) & 0x3f;
}
uint32_t indexTooffset(uint32_t index)
{
	// return (addr >> 1) & 0x3f;
	return (index) << 2;
}

csi_error_t cvi_efuse_wait_idle()
{
	uint32_t status;
	uint32_t time_count = 0;
	csi_error_t ret = CSI_OK;
	do {
		status = mmio_read_32(EFUSE_BASE + EFUSE_STATUS);
		time_count++;

		if (status & 0x8) {
			printf("PGM err\n");
			return CSI_ERROR;
		}

		if (status & 0x6) {
			printf("efuse bit has been locked err\n");
			return CSI_ERROR;
		}

		if (time_count > 0x1000) {
			printf("wait idle timeout\n");
			return CSI_ERROR;
		}
		// wait efuse_busy==0
	} while ((status & BUSY) != 0);
	// printf("wait idle exit\n");
	return ret;
}

csi_error_t cvi_efuse_power_on()
{
	csi_error_t res = CSI_OK;
	cvi_efuse_wait_idle();
	// EFUSE_A_FIRE +EFUSE_OP_MODE power_on
	mmio_write_32(EFUSE_BASE + EFUSE_MODE, 0x10);
	if (cvi_efuse_wait_idle() != CSI_OK) {
		printf("cvi_efuse_power_on failed\n");
		return CSI_ERROR;
	}
	return res;
}

csi_error_t cvi_efuse_refresh_shadow()
{
	// csi_error_t ret = CSI_OK;
	// if (cvi_efuse_wait_idle() != CSI_OK) {
	//     printf("cvi_efuse_refresh_shadow\n");
	//     return CSI_ERROR;
	// }
	mmio_write_32(EFUSE_BASE + EFUSE_MODE, 0x30);
	// if (cvi_efuse_wait_idle() != CSI_OK) {
	//     printf("cvi_efuse_refresh_shadow\n");
	//     return CSI_ERROR;
	// }
	return CSI_OK;
}

csi_error_t cvi_efuse_array_read(uint32_t index, uint32_t *data)
{
	// addr = phyadrTransCowAdr(addr);
	cvi_efuse_power_on();
	if (cvi_efuse_wait_idle() != CSI_OK) {
		printf("cvi_efuse_wait_idle failed\n");
		return CSI_ERROR;
	}
	// double bit
	mmio_write_32(EFUSE_BASE + EFUSE_ADR, index);
	// array read
	mmio_write_32(EFUSE_BASE + EFUSE_MODE, 0x11);
	if (cvi_efuse_wait_idle() != CSI_OK) {
		printf("cvi_efuse_wait_idle failed\n");
		return CSI_ERROR;
	}

	*data = mmio_read_32(EFUSE_BASE + EFUSE_RD_DATA);
	return CSI_OK;
}

// read one line (4 bytes) from shadow
csi_error_t cvi_efuse_read_word_from_shadow(uint32_t index, uint32_t *data)
{
	uint32_t addr = indexTooffset(index);
	if (cvi_efuse_wait_idle() != CSI_OK) {
		printf("cvi_efuse_refresh_shadow\n");
		return CSI_ERROR;
	}
	*data = mmio_read_32(EFUSE_BASE + 0x100 + addr);
	// printf("shadow read addr 0x%x = 0x%x\n", addr, *data);
	return CSI_OK;
}
csi_error_t cvi_efuse_read_word_from_sec_shadow(uint32_t index, uint32_t *data)
{
	uint32_t addr = indexTooffset(index);
	if (cvi_efuse_wait_idle() != CSI_OK) {
		printf("cvi_efuse_refresh_sec_shadow\n");
		return CSI_ERROR;
	}
	*data = mmio_read_32(SE_SYS_CTL_FOR_EFUSE_BASE + 0x100 + addr);
	// printf("shadow read addr 0x%x = 0x%x\n", addr, *data);
	return CSI_OK;
}

// program 4 bytes,addr is cow addr in fact(so need to trans)
csi_error_t cvi_efuse_program_word(uint32_t index, const uint32_t data)
{
	int i;
	uint32_t exist_data = 0;
	cvi_efuse_array_read(index, &exist_data);
	exist_data = (~exist_data) & data;
	if (exist_data == 0) {
		printf("no data need write\n");
		return CSI_OK;
	}

	for (i = 0; i < 32; i++) {
		int bit = exist_data & (1 << i);

		if (bit) {
			if (cvi_efuse_wait_idle() != CSI_OK) {
				printf("cvi_efuse_wait_idle failed\n");
				return CSI_ERROR;
			}
			uint16_t w_addr = (i << 7) | ((index & 0x3F) << 1);
			mmio_write_32(EFUSE_BASE + EFUSE_ADR, (w_addr & 0xFFF));
			mmio_write_32(EFUSE_BASE + EFUSE_MODE, 0x14);

			if (cvi_efuse_wait_idle() != CSI_OK) {
				printf("cvi_efuse_wait_idle failed\n");
				return CSI_ERROR;
			}
			w_addr |= 0x1;
			mmio_write_32(EFUSE_BASE + EFUSE_ADR, (w_addr & 0xFFF));
			// array program + issue command
			mmio_write_32(EFUSE_BASE + EFUSE_MODE, 0x14);
			if (cvi_efuse_wait_idle() != CSI_OK) {
				printf("cvi_efuse_wait_idle failed\n");
				return CSI_ERROR;
			}
		}
	}
	cvi_efuse_refresh_shadow();

	return CSI_OK;
}

csi_error_t cvi_efuse_program_word_magic(uint32_t index, const uint32_t data)
{
	int i;
	uint32_t exist_data = 0;
	cvi_efuse_array_read(index, &exist_data);
	exist_data = (~exist_data) & data;
	if (exist_data == 0) {
		printf("no data need write\n");
		return CSI_OK;
	}

	for (i = 0; i < 32; i++) {
		int bit = exist_data & (1 << i);

		if (bit) {
			if (cvi_efuse_wait_idle() != CSI_OK) {
				printf("cvi_efuse_wait_idle failed\n");
				return CSI_ERROR;
			}
			uint16_t w_addr = (i << 7) | ((index & 0x3F) << 1);
			mmio_write_32(EFUSE_BASE + EFUSE_ADR, (w_addr & 0xFFF));
			mmio_write_32(EFUSE_BASE + EFUSE_MODE, 0xDEFA0014);

			if (cvi_efuse_wait_idle() != CSI_OK) {
				printf("cvi_efuse_wait_idle failed\n");
				return CSI_ERROR;
			}
			w_addr |= 0x1;
			mmio_write_32(EFUSE_BASE + EFUSE_ADR, (w_addr & 0xFFF));
			// array program + issue command
			mmio_write_32(EFUSE_BASE + EFUSE_MODE, 0xDEFA0014);
			if (cvi_efuse_wait_idle() != CSI_OK) {
				printf("cvi_efuse_wait_idle failed\n");
				return CSI_ERROR;
			}
		}
	}
	cvi_efuse_refresh_shadow();

	return CSI_OK;
}

int testcase_loadefuse(void)
{
	int i = 0;
	uint32_t read_data;
	printf("Dump data from efuse...\n");
	for (i = 0; i < (1 << EFUSE_NUM_ADDRESS_BITS); i++) {
		if ((i % 8) == 0)
			printf("0x%02X: ", i);
		cvi_efuse_array_read(i, &read_data);
		printf("%08X ", read_data);

		if ((i % 8) == 7)
			printf("\n");
	}
	return 0;
}

csi_error_t test_getshadow()
{
	int i;
	uint32_t read_data;
	cvi_efuse_refresh_shadow();
	for (i = 0; i < (1 << (EFUSE_NUM_ADDRESS_BITS - 1)); i++) {
		if ((i % 8) == 0)
			printf("0x%02X: ", i);
		cvi_efuse_read_word_from_shadow(i, &read_data);
		printf("%08X ", read_data);
		if ((i % 8) == 7)
			printf("\n");
	}
	return CSI_OK;
}
csi_error_t test_get_sec_shadow()
{
	int i;
	uint32_t read_data;
	cvi_efuse_refresh_shadow();
	printf("get_sec_shadow\n");
	for (i = 0; i < (1 << (EFUSE_NUM_ADDRESS_BITS - 1)); i++) {
		if ((i % 8) == 0)
			printf("0x%02X: ", i);
		cvi_efuse_read_word_from_sec_shadow(i, &read_data);
		printf("%08X ", read_data);
		if ((i % 8) == 7)
			printf("\n");
	}
	return CSI_OK;
}

csi_error_t test_w_lock()
{
	cvi_efuse_power_on();
	cvi_efuse_program_word(eFuse_w_lock0, 0xffffffff);
	cvi_efuse_power_on();
	test_getshadow();
	printf("\n");
	testcase_loadefuse();
	cvi_efuse_program_word(eFuse_sw_info, 0xffffffff);
	test_getshadow();
	return CSI_OK;
}

csi_error_t test_writeefuse()
{
	cvi_efuse_power_on();
	cvi_efuse_program_word(11, 0x13);
	cvi_efuse_power_on();
	test_getshadow();
	printf("\n");
	testcase_loadefuse();
	return CSI_OK;
}

csi_error_t test_swinfo()
{
	cvi_efuse_power_on();
	cvi_efuse_program_word(eFuse_sw_info, 0x13);
	cvi_efuse_power_on();
	test_getshadow();
	printf("\n");
	testcase_loadefuse();
	writel(0x03003020, 0x0); // reset cpus (auto clr)
	return CSI_OK;
}

csi_error_t test_rompatch()
{
	cvi_efuse_power_on();
	cvi_efuse_program_word(eFuse_sw_info, 0x13);
	cvi_efuse_power_on();
	//addr
	cvi_efuse_program_word(eFuse_rompatch_addr0, 0xe000002);
	//data
	cvi_efuse_program_word(eFuse_rompatch_data0, 0x213411);
	cvi_efuse_power_on();
	test_getshadow();
	printf("\n");
	testcase_loadefuse();
	writel(0x03003020, 0x0); // reset cpus (auto clr)
	return CSI_OK;
}

csi_error_t test_secureRegionCTRL()
{
	// cvi_efuse_power_on();
	// cvi_efuse_program_word(secureRegionCTRL, 0xffffffff); // expect err or 0
	// test_getshadow();

	mmio_write_32(EFUSE_BASE + EFUSE_OP_PSSWD_ENTRY, 0xe9546b8d);
	cvi_efuse_power_on();
	cvi_efuse_program_word(secureRegionCTRL, 0x900000);
	test_getshadow();
	printf("\n");
	testcase_loadefuse();
	test_getshadow();
	return CSI_OK;
}

csi_error_t test_secureRegionCTRL_lock()
{
	mmio_write_32(EFUSE_BASE + EFUSE_OP_PSSWD_ENTRY, 0xe9546b8d);
	cvi_efuse_power_on();
	cvi_efuse_program_word(secureRegionCTRL, 0xffffffff);
	test_getshadow();
	cvi_efuse_program_word(eFuse_rompatch_addr0, 0xffffffff);
	cvi_efuse_program_word(eFuse_rompatch_data0, 0xffffffff);
	printf("\n");
	test_getshadow();
	// testcase_loadefuse();
	return CSI_OK;
}

csi_error_t test_pgm_key_port_en()
{
	mmio_write_32(EFUSE_BASE + EFUSE_ONE_WAY, 0x4);
	// cvi_efuse_power_on();
	// cvi_efuse_program_word(eFuse_sw_info, 0x13); //expect PGM err

	cvi_efuse_power_on();
	cvi_efuse_program_word_magic(eFuse_sw_info, 0x13);
	cvi_efuse_power_on();
	test_getshadow();
	printf("\n");
	testcase_loadefuse();
	return CSI_OK;
}

csi_error_t test_sec_efuse()
{
	printf("sec efuse op port = 0x%x\n",
	       mmio_read_32(SE_SYS_CTL_FOR_EFUSE_BASE + OP_PORT));
	printf("sec efuse op passwd = 0x%x\n",
	       mmio_read_32(SE_SYS_CTL_FOR_EFUSE_BASE + OP_PASSWD));
	// mmio_write_32(SE_SYS_CTL_FOR_EFUSE_BASE + OP_PORT, 0xfff);
	mmio_write_32(SE_SYS_CTL_FOR_EFUSE_BASE + OP_PASSWD, 0xfff);
	printf("sec efuse op port = 0x%x\n",
	       mmio_read_32(SE_SYS_CTL_FOR_EFUSE_BASE + OP_PORT));
	printf("sec efuse op passwd = 0x%x\n",
	       mmio_read_32(SE_SYS_CTL_FOR_EFUSE_BASE + OP_PASSWD));
	return CSI_OK;
}

csi_error_t test_sec_op_lock()
{
	mmio_write_32(SE_SYS_CTL_FOR_EFUSE_BASE + OP_PORT, 0x8);
	cvi_efuse_power_on();
	cvi_efuse_program_word(eFuse_sw_info, 0x13);
	cvi_efuse_power_on();
	test_getshadow();
	return CSI_OK;
}
