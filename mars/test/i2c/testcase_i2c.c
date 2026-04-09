#include "module_testcase.h"
#include "system_common.h"
#include "mmio.h"
#include "timer.h"
#include "testcase_i2c.h"

#define DBG_I2C_SLAVE_ID	0x56
#define THERMAL_SLAVE_ID	0x4C
#define TEST_ADDR_W	0x04000000
#define TEST_ADDR_R	0x04000004

#ifdef THERMAL_USE_I2C_0
#define THERMAL_I2C_BASE BASE_I2C0
#else
#define THERMAL_I2C_BASE BASE_I2C4
#endif


typedef int (*handler_t)();
struct testcase {
	handler_t handler;
	char *name;
};

int i2c_write_test();
int i2c_read_test();
int i2c_read_thermal_id();
int i2c_read_thermal_local();
int i2c_read_thermal_external();

struct testcase i2c_testcases[] = {
#ifndef BUILD_TEST_CASE_SLT_i2c
	{ .handler = i2c_write_test, .name = "I2C write test (master: I2C0, slave: DBG I2C)" },
	{ .handler = i2c_read_test, .name = "I2C read test (master: I2C0, slave: DBG I2C)" },
#endif
	{ .handler = i2c_read_thermal_id, .name = "I2C read ID from thermal" },
	{ .handler = i2c_read_thermal_local, .name = "I2C read local temperature from thermal" },
	{ .handler = i2c_read_thermal_external, .name = "I2C read external temperature from thermal" }
};

void wait_until_tx_empty(uint32_t i2c_base)
{
	uint16_t timeout = 1000;
	while (1) {
		udelay(90);	// somehow delay for FPGA bus speed
		uint32_t sts = mmio_read_32(i2c_base + REG_RAW_INT_STS);
		if (sts & TX_EMPTY)
			break;
		if (--timeout == 0) {
			printf("wait_until_tx_empty timeout\n");
			break;
		}
	}
}

void wait_until_rx_not_empty(uint32_t i2c_base)
{
	uint16_t timeout = 1000;
	while (1) {
		uint32_t sts = mmio_read_32(i2c_base + REG_STATUS);
		if (sts & RFNE)
			break;
		if (--timeout == 0) {
			printf("wait_until_rx_not_empty timeout\n");
			break;
		}
	}
}

void i2c_set_pinmux(uint32_t i2c_base)
{
	if (i2c_base == BASE_I2C0) {
		mmio_clrsetbits_32(BASE_PINMUX + 0x9C, 0x3<<20, 0x1<<20); /* IIC0_SDA */
		mmio_clrsetbits_32(BASE_PINMUX + 0xA0, 0x3<<4, 0x1<<4); /* IIC0_SCL */
	} else if (i2c_base == BASE_I2C1) {
		mmio_clrsetbits_32(BASE_PINMUX + 0xA0, 0x3<<20, 0x1<<20); /* IIC1_SDA */
		mmio_clrsetbits_32(BASE_PINMUX + 0xA4, 0x3<<4, 0x1<<4); /* IIC1_SCL */
	} else if (i2c_base == BASE_I2C2) {
		mmio_clrsetbits_32(BASE_PINMUX + 0xA4, 0x3<<20, 0x1<<20); /* IIC2_SDA */
		mmio_clrsetbits_32(BASE_PINMUX + 0xA8, 0x3<<4, 0x1<<4); /* IIC2_SCL */
	} else if (i2c_base == BASE_I2C3) {
		mmio_clrsetbits_32(BASE_PINMUX + 0xA8, 0x3<<20, 0x1<<20); /* IIC3_SDA */
		mmio_clrsetbits_32(BASE_PINMUX + 0xAC, 0x3<<4, 0x1<<4); /* IIC3_SCL */
	} else if (i2c_base == BASE_I2C4) {
		mmio_clrsetbits_32(BASE_PINMUX + 0xAC, 0x3<<20, 0x1<<20); /* IIC4_SDA */
		mmio_clrsetbits_32(BASE_PINMUX + 0xB0, 0x3<<4, 0x1<<4); /* IIC4_SCL */
	}
}

void i2c_set_clk_in_khz(uint32_t i2c_base, uint32_t khz)
{
	uint8_t hcnt, lcnt;

	if (khz > 400) {
		mmio_write_32(i2c_base + REG_CON, 0x67);
		hcnt = 0.4 * 14900 / khz;
		lcnt = 0.6 * 14900 / khz;
		mmio_write_32(i2c_base + REG_HS_HCNT, hcnt);
		mmio_write_32(i2c_base + REG_HS_LCNT, lcnt);
	} else {
		mmio_write_32(i2c_base + REG_CON, 0x65);
		hcnt = 0.4 * 20000 / khz;
		lcnt = 0.6 * 20000 / khz;
		mmio_write_32(i2c_base + REG_FS_HCNT, hcnt);
		mmio_write_32(i2c_base + REG_FS_LCNT, lcnt);
	}
	// printf("hcnt = 0x%x\n", hcnt);
	// printf("lcnt = 0x%x\n", lcnt);
}

void i2c_init(uint32_t i2c_base, uint8_t id)
{
	i2c_set_pinmux(i2c_base);

	mmio_write_32(i2c_base + REG_ENABLE, 0);
	mmio_write_32(i2c_base + REG_TAR, id);

	i2c_set_clk_in_khz(i2c_base, 200);

	mmio_write_32(i2c_base + REG_INT_MASK, 0);

	mmio_write_32(i2c_base + REG_RX_TL, 4);
	mmio_write_32(i2c_base + REG_TX_TL, 1);

	mmio_write_32(i2c_base + REG_ENABLE, 1);
}

/*
*  I2C write protocol for DBG I2C slave
*/
void drv_i2c_write(uint32_t i2c_base, uint64_t addr, uint32_t data)
{
	// set addr mode
	if ((addr>>32) != 0)
		mmio_write_32(i2c_base + REG_DATA_CMD, ADDR_MODE_40);
	else
		mmio_write_32(i2c_base + REG_DATA_CMD, ADDR_MODE_32);

	// wait for tx fifo empty
	wait_until_tx_empty(i2c_base);

	// write addr
	mmio_write_32(i2c_base + REG_DATA_CMD, 0x12);
	mmio_write_32(i2c_base + REG_DATA_CMD, addr&0xFF);
	mmio_write_32(i2c_base + REG_DATA_CMD, (addr>>8)&0xFF);
	mmio_write_32(i2c_base + REG_DATA_CMD, (addr>>16)&0xFF);
	mmio_write_32(i2c_base + REG_DATA_CMD, (addr>>24)&0xFF);
	if ((addr>>32) != 0)
		mmio_write_32(i2c_base + REG_DATA_CMD, (addr>>32)&0xFF);

	// write data
	mmio_write_32(i2c_base + REG_DATA_CMD, data&0xFF);
	mmio_write_32(i2c_base + REG_DATA_CMD, (data>>8)&0xFF);
	mmio_write_32(i2c_base + REG_DATA_CMD, (data>>16)&0xFF);
	mmio_write_32(i2c_base + REG_DATA_CMD, (data>>24)&0xFF);

	// wait for tx fifo empty
	wait_until_tx_empty(i2c_base);
}

/*
*  I2C read protocol for DBG I2C slave
*/
uint32_t drv_i2c_read(uint32_t i2c_base, uint64_t addr)
{
	uint32_t data = 0;

	// set addr mode
	if ((addr>>32) != 0)
		mmio_write_32(i2c_base + REG_DATA_CMD, ADDR_MODE_40);
	else
		mmio_write_32(i2c_base + REG_DATA_CMD, ADDR_MODE_32);

	// wait for tx fifo empty
	wait_until_tx_empty(i2c_base);

	// write addr
	mmio_write_32(i2c_base + REG_DATA_CMD, 0x12);
	mmio_write_32(i2c_base + REG_DATA_CMD, addr&0xFF);
	mmio_write_32(i2c_base + REG_DATA_CMD, (addr>>8)&0xFF);
	mmio_write_32(i2c_base + REG_DATA_CMD, (addr>>16)&0xFF);
	mmio_write_32(i2c_base + REG_DATA_CMD, (addr>>24)&0xFF);
	if ((addr>>32) != 0)
		mmio_write_32(i2c_base + REG_DATA_CMD, (addr>>32)&0xFF);

	// wait for tx fifo empty
	wait_until_tx_empty(i2c_base);

	// trigger 32 bytes data read
	mmio_write_16(i2c_base + REG_DATA_CMD, 0x100);
	mmio_write_16(i2c_base + REG_DATA_CMD, 0x100);
	mmio_write_16(i2c_base + REG_DATA_CMD, 0x100);
	mmio_write_16(i2c_base + REG_DATA_CMD, 0x100);

	// wait and read rx fifo
	wait_until_rx_not_empty(i2c_base);
	data = mmio_read_8(i2c_base + REG_DATA_CMD)<<24;

	wait_until_rx_not_empty(i2c_base);
	data |= (mmio_read_8(i2c_base + REG_DATA_CMD)<<16);

	wait_until_rx_not_empty(i2c_base);
	data |= (mmio_read_8(i2c_base + REG_DATA_CMD)<<8);

	wait_until_rx_not_empty(i2c_base);
	data |= mmio_read_8(i2c_base + REG_DATA_CMD);

	return data;
}

/*
*  I2C write protocol for thermal sensor(NCT218) slave
*/
void drv_i2c_write_thermal(uint32_t i2c_base, uint8_t addr, uint8_t data)
{
	// set address pointer register
	mmio_write_32(i2c_base + REG_DATA_CMD, addr&0xFF);

	// wait for tx fifo empty
	// wait_until_tx_empty(i2c_base);

	// write data
	mmio_write_32(i2c_base + REG_DATA_CMD, data&0xFF);

	// wait for tx fifo empty
	wait_until_tx_empty(i2c_base);
}

/*
*  I2C read protocol for thermal sensor(NCT218) slave
*/
uint8_t drv_i2c_read_thermal(uint32_t i2c_base, uint8_t addr)
{
	uint8_t data = 0;

	// set address pointer register
	mmio_write_32(i2c_base + REG_DATA_CMD, addr&0xFF);

	// wait for tx fifo empty
	wait_until_tx_empty(i2c_base);

	// trigger 1 byte data read
	mmio_write_16(i2c_base + REG_DATA_CMD, 0x100);

	// wait and read rx fifo
	wait_until_rx_not_empty(i2c_base);
	data = mmio_read_8(i2c_base + REG_DATA_CMD);

	return data;
}

int i2c_write_test()
{
	uint32_t tmp, result;

	i2c_init(BASE_I2C0, DBG_I2C_SLAVE_ID);

	tmp = mmio_read_32(TEST_ADDR_W);

	drv_i2c_write(BASE_I2C0, TEST_ADDR_W, 0x55AAAA55);
	udelay(500);
	if ( mmio_read_32(TEST_ADDR_W) == 0x55AAAA55 )
		result = 1;
	else
		result = 0;

	mmio_write_32(TEST_ADDR_W, tmp);

	return result;
}

int i2c_read_test()
{
	uint32_t tmp, result;

	i2c_init(BASE_I2C0, DBG_I2C_SLAVE_ID);

	tmp = mmio_read_32(TEST_ADDR_R);

	mmio_write_32(TEST_ADDR_R, 0x55AAAA55);
	if ( drv_i2c_read(BASE_I2C0, TEST_ADDR_R) == 0x55AAAA55 )
		result = 1;
	else
		result = 0;

	mmio_write_32(TEST_ADDR_R, tmp);

	return result;
}

int i2c_read_thermal_id()
{
	uint8_t id;

	#ifdef THERMAL_USE_I2C_0
	printf("Read thermal sensor ID from I2C0\n");
	#else
	printf("Read thermal sensor ID from I2C4\n");
	#endif

	i2c_init(THERMAL_I2C_BASE, THERMAL_SLAVE_ID);

	id = drv_i2c_read_thermal(THERMAL_I2C_BASE, 0xFE);

	printf("Thermal sensor ID: %X\n", id);

	if ((id == 0x55) || (id == 0x1a))
		return 1;
	else
		return 0;
}

int i2c_read_thermal_local()
{
	uint8_t temp;

	#ifdef THERMAL_USE_I2C_0
	printf("Read thermal local temperature from I2C0\n");
	#else
	printf("Read thermal local temperature from I2C4\n");
	#endif

	i2c_init(THERMAL_I2C_BASE, THERMAL_SLAVE_ID);

	temp = drv_i2c_read_thermal(THERMAL_I2C_BASE, 0x00);

	printf("Local Temperature: %d (degree Celsius)\n", temp);

	return 1;
}

int i2c_read_thermal_external()
{
	uint8_t temp_hi, temp_lo;

	#ifdef THERMAL_USE_I2C_0
	printf("Read thermal external temperature from I2C0\n");
	#else
	printf("Read thermal external temperature from I2C4\n");
	#endif

	i2c_init(THERMAL_I2C_BASE, THERMAL_SLAVE_ID);

	temp_hi = drv_i2c_read_thermal(THERMAL_I2C_BASE, 0x01);
	temp_lo = drv_i2c_read_thermal(THERMAL_I2C_BASE, 0x10);

	printf("temp_hi = %02X, temp_lo = %02X\n", temp_hi, temp_lo);
	printf("External Temperature: %2d.%02d (degree Celsius)\n", temp_hi, (temp_lo*100)/256);

	return 1;
}

int testcase_i2c(void)
{
	int i;

	printf("\n------------ I2C test start ------------\n\n");

	for (i = 0; i < sizeof(i2c_testcases) / sizeof(i2c_testcases[0]); ++i) {
		printf("[%d] %s start\n", i, i2c_testcases[i].name);
		if ( i2c_testcases[i].handler() == 0) {
			printf("[%d] %s failed\n", i, i2c_testcases[i].name);
			return -1;
		}
		printf("[%d] %s pass\n\n", i, i2c_testcases[i].name);
	}

	printf("\n------------ I2C test done ------------\n");

	return 0;
}

module_testcase("1", testcase_i2c);