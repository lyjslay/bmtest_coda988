#include <stdio.h>
#include "test.h"

#if TEST_I2C

#define BOARD_FPGA 1

typedef uint32_t u32;
typedef uint16_t u16;
typedef uint8_t u8;
typedef uint32_t uint;
typedef char bool;
typedef unsigned char uchar;

#define false 0
#define true 1

#define writel(a, v) dbg_write_robot((uint32_t)(a), (uint32_t)(v))
#define readl(a) dbg_read_robot((uint32_t)(a))
#define debug printf

static void dbg_write_robot(uint32_t addr, uint32_t value)
{
	write_robot(addr, value);
}

static uint32_t dbg_read_robot(uint32_t addr)
{
	return read_robot(addr);
}

static void timer_meter_start(void)
{
}

static u32 timer_meter_get_ms(void)
{
        return 0;
}

//#define CONFIG_USE_I2C_DMA

#define REG_TOP_IIC1_CLK_DIV 0x80
#define REG_TOP_IIC2_CLK_DIV 0x84
// #define CONFIG_SYS_I2C_BASE 0x4000000
uint32_t xdata CONFIG_SYS_I2C_BASE = RTC_I2C_BASE;
#define CONFIG_SYS_I2C_SPEED 100000

#define CONFIG_SYS_I2C_SLAVE 0x3C

#define BIT_I2C_CMD_DATA_READ_BIT (0x01 << 8)
#define BIT_I2C_CMD_DATA_STOP_BIT (0x01 << 9)

#define BIT_I2C_INT_RX_UNDER (0x01 << 0)
#define BIT_I2C_INT_RX_OVER (0x01 << 1)
#define BIT_I2C_INT_RX_FULL (0x01 << 2)
#define BIT_I2C_INT_TX_OVER (0x01 << 3)
#define BIT_I2C_INT_TX_EMPTY (0x01 << 4)
#define BIT_I2C_INT_RD_REQ (0x01 << 5)
#define BIT_I2C_INT_TX_ABRT (0x01 << 6)
#define BIT_I2C_INT_RX_DONE (0x01 << 7)
#define BIT_I2C_INT_ACTIVITY (0x01 << 8)
#define BIT_I2C_INT_STOP_DET (0x01 << 9)
#define BIT_I2C_INT_START_DET (0x01 << 10)
#define BIT_I2C_INT_GEN_ALL (0x01 << 11)

#define BIT_I2C_INT_RX_UNDER_MASK (0x01 << 0)
#define BIT_I2C_INT_RX_OVER_MASK (0x01 << 1)
#define BIT_I2C_INT_RX_FULL_MASK (0x01 << 2)
#define BIT_I2C_INT_TX_OVER_MASK (0x01 << 3)
#define BIT_I2C_INT_TX_EMPTY_MASK (0x01 << 4)
#define BIT_I2C_INT_RD_REQ_MASK (0x01 << 5)
#define BIT_I2C_INT_TX_ABRT_MASK (0x01 << 6)
#define BIT_I2C_INT_RX_DONE_MASK (0x01 << 7)
#define BIT_I2C_INT_ACTIVITY_MASK (0x01 << 8)
#define BIT_I2C_INT_STOP_DET_MASK (0x01 << 9)
#define BIT_I2C_INT_START_DET_MASK (0x01 << 10)
#define BIT_I2C_INT_GEN_ALL_MASK (0x01 << 11)

#define BIT_I2C_INT_RX_UNDER_RAW (0x01 << 0)
#define BIT_I2C_INT_RX_OVER_RAW (0x01 << 1)
#define BIT_I2C_INT_RX_FULL_RAW (0x01 << 2)
#define BIT_I2C_INT_TX_OVER_RAW (0x01 << 3)
#define BIT_I2C_INT_TX_EMPTY_RAW (0x01 << 4)
#define BIT_I2C_INT_RD_REQ_RAW (0x01 << 5)
#define BIT_I2C_INT_TX_ABRT_RAW (0x01 << 6)
#define BIT_I2C_INT_RX_DONE_RAW (0x01 << 7)
#define BIT_I2C_INT_ACTIVITY_RAW (0x01 << 8)
#define BIT_I2C_INT_STOP_DET_RAW (0x01 << 9)
#define BIT_I2C_INT_START_DET_RAW (0x01 << 10)
#define BIT_I2C_INT_GEN_ALL_RAW (0x01 << 11)

#define BIT_I2C_DMA_CR_TDMAE (0x01 << 1)
#define BIT_I2C_DMA_CR_RDMAE (0x01 << 0)

#define	ic_con 0x00
#define	ic_tar 0x04
#define	ic_sar 0x08
#define	ic_hs_maddr 0x0c
#define	ic_cmd_data 0x10
#define	ic_ss_scl_hcnt 0x14
#define	ic_ss_scl_lcnt 0x18
#define	ic_fs_scl_hcnt 0x1c
#define	ic_fs_scl_lcnt 0x20
#define	ic_hs_scl_hcnt 0x24
#define	ic_hs_scl_lcnt 0x28
#define	ic_intr_stat 0x2c
#define	ic_intr_mask 0x30
#define	ic_raw_intr_stat 0x34
#define	ic_rx_tl 0x38
#define	ic_tx_tl 0x3c
#define	ic_clr_intr 0x40
#define	ic_clr_rx_under 0x44
#define	ic_clr_rx_over 0x48
#define	ic_clr_tx_over 0x4c
#define	ic_clr_rd_req 0x50
#define	ic_clr_tx_abrt 0x54
#define	ic_clr_rx_done 0x58
#define	ic_clr_activity 0x5c
#define	ic_clr_stop_det 0x60
#define	ic_clr_start_det 0x64
#define	ic_clr_gen_call 0x68
#define	ic_enable 0x6c
#define	ic_status 0x70
#define	ic_txflr 0x74
#define	ic_rxflr 0x78
#define	ic_sda_hold 0x7c
#define	ic_tx_abrt_source 0x80
#define	ic_slv_dat_nack_only 0x84
#define	ic_dma_cr 0x88
#define	ic_dma_tdlr 0x8c
#define	ic_dma_rdlr 0x90
#define	ic_sda_setup 0x94
#define	ic_ack_general_call 0x98
#define	ic_enable_status 0x9c
#define	ic_fs_spklen 0xa0
#define	ic_hs_spklen 0xa4

#if defined(BOARD_FPGA)
#define IC_CLK 25L
#else
#define IC_CLK 100L
#endif

#define NANO_TO_MICRO 1000

/* High and low times in different speed modes (in ns) */
#define MIN_SS_SCL_HIGHTIME 4000L
#define MIN_SS_SCL_LOWTIME 4700L
#define MIN_FS_SCL_HIGHTIME 600L
#define MIN_FS_SCL_LOWTIME 1300L
#define MIN_HS100pF_SCL_HIGHTIME 60L
#define MIN_HS100pF_SCL_LOWTIME 120L
#define MIN_HS400pF_SCL_HIGHTIME 160L
#define MIN_HS400pF_SCL_LOWTIME 320L

#define CONFIG_SYS_HZ 1000L /* decrementer freq: 1ms ticks */
/* Worst case timeout for 1 byte is kept as 2ms */
#if defined(BOARD_FPGA)
#define I2C_BYTE_TO (CONFIG_SYS_HZ / 500) + 10
#else
#define I2C_BYTE_TO (CONFIG_SYS_HZ / 500)
#endif
#define I2C_STOPDET_TO (CONFIG_SYS_HZ / 500)
#define I2C_BYTE_TO_BB (I2C_BYTE_TO * 16)

/* i2c control register definitions */
#define IC_CON_SD 0x0040
#define IC_CON_RE 0x0020
#define IC_CON_10BITADDRMASTER 0x0010
#define IC_CON_10BITADDR_SLAVE 0x0008
#define IC_CON_SPD_MSK 0x0006
#define IC_CON_SPD_SS 0x0002
#define IC_CON_SPD_FS 0x0004
#define IC_CON_SPD_HS 0x0006
#define IC_CON_MM 0x0001

/* i2c target address register definitions */
#define TAR_ADDR 0x0050

/* i2c slave address register definitions */
#define IC_SLAVE_ADDR 0x0002

/* i2c data buffer and command register definitions */
#define IC_CMD 0x0100
#define IC_STOP 0x0200

/* i2c interrupt status register definitions */
#define IC_GEN_CALL 0x0800
#define IC_START_DET 0x0400
#define IC_STOP_DET 0x0200
#define IC_ACTIVITY 0x0100
#define IC_RX_DONE 0x0080
#define IC_TX_ABRT 0x0040
#define IC_RD_REQ 0x0020
#define IC_TX_EMPTY 0x0010
#define IC_TX_OVER 0x0008
#define IC_RX_FULL 0x0004
#define IC_RX_OVER 0x0002
#define IC_RX_UNDER 0x0001

/* fifo threshold register definitions */
#define IC_TL0 0x00
#define IC_TL1 0x01
#define IC_TL2 0x02
#define IC_TL3 0x03
#define IC_TL4 0x04
#define IC_TL5 0x05
#define IC_TL6 0x06
#define IC_TL7 0x07
#define IC_RX_TL IC_TL0
#define IC_TX_TL IC_TL0

/* i2c enable register definitions */
#define IC_ENABLE_0B 0x0001

/* i2c status register  definitions */
#define IC_STATUS_SA 0x0040
#define IC_STATUS_MA 0x0020
#define IC_STATUS_RFF 0x0010
#define IC_STATUS_RFNE 0x0008
#define IC_STATUS_TFE 0x0004
#define IC_STATUS_TFNF 0x0002
#define IC_STATUS_ACT 0x0001

/* Speed Selection */
#define IC_SPEED_MODE_STANDARD 1
#define IC_SPEED_MODE_FAST 2
#define IC_SPEED_MODE_MAX 3

#define I2C_MAX_SPEED 3400000
#define I2C_FAST_SPEED 400000
#define I2C_STANDARD_SPEED 100000


struct dw_scl_sda_cfg {
	u32 ss_hcnt;
	u32 fs_hcnt;
	u32 ss_lcnt;
	u32 fs_lcnt;
	u32 sda_hold;
};

#ifdef CONFIG_USE_I2C_DMA
struct tx_data_buffer {
	u16 idx;
	u32 data[10];
};

struct tx_data_buffer tx_data;
u32 rx_data[10];
#endif

void dw_i2c_write_cmd_data(u16 value)
{
	printf("w: 0x%x\n", value);
#ifdef CONFIG_USE_I2C_DMA
	tx_data.data[tx_data.idx] = value;
	tx_data.idx++;
	if (((value & BIT_I2C_CMD_DATA_STOP_BIT) >> 9) ==
	    0x01) { /* no more data need to be transferred */
		dma_mem2dev_setting((unsigned int *)tx_data.data, tx_data.idx * sizeof(u32),
				    CONFIG_SYS_I2C_BASE + ic_cmd_data, DMA_I2C0);
		dma_start_transfer(DMA_I2C0);
	}
#else
	writel(CONFIG_SYS_I2C_BASE + ic_cmd_data, value);
#endif
}

static void dw_i2c_enable(bool enable)
{
	xdata u32 ena = enable ? IC_ENABLE_0B : 0;
	xdata int timeout = 100;

	do {
		writel(CONFIG_SYS_I2C_BASE + ic_enable, ena);
		if ((readl(CONFIG_SYS_I2C_BASE + ic_enable_status) & IC_ENABLE_0B) == ena)
			return;

		/*
		 * Wait 10 times the signaling period of the highest I2C
		 * transfer supported by the driver (for 400KHz this is
		 * 25us) as described in the DesignWare I2C databook.
		 */
		udelay(25);
	} while (timeout--);

	printf("timeout in %sabling I2C adapter\n", enable ? "en" : "dis");
}

/*
 * i2c_set_bus_speed - Set the i2c speed
 * @speed:	required i2c speed
 *
 * Set the i2c speed.
 */
static unsigned int __dw_i2c_set_bus_speed(struct dw_scl_sda_cfg *scl_sda_cfg, uint32_t speed)
{
	xdata uint32_t cntl;
	xdata uint32_t hcnt, lcnt;
	xdata uint32_t i2c_spd;

	if (speed > I2C_FAST_SPEED)
		i2c_spd = IC_SPEED_MODE_MAX;
	else if ((speed <= I2C_FAST_SPEED) && (speed > I2C_STANDARD_SPEED))
		i2c_spd = IC_SPEED_MODE_FAST;
	else
		i2c_spd = IC_SPEED_MODE_STANDARD;

	/* to set speed cltr must be disabled */
	dw_i2c_enable(false);

	cntl = (readl(CONFIG_SYS_I2C_BASE + ic_con) & (~IC_CON_SPD_MSK));
	printf("cntl=0x%lx\n", cntl);

	switch (i2c_spd) {
#ifndef CONFIG_X86 /* No High-speed for BayTrail yet */
	case IC_SPEED_MODE_MAX:
		cntl |= IC_CON_SPD_HS;
		if (scl_sda_cfg) {
			hcnt = scl_sda_cfg->fs_hcnt;
			lcnt = scl_sda_cfg->fs_lcnt;
		} else {
			//hcnt = (u16)(((IC_CLK * MIN_HS100pF_SCL_HIGHTIME) / 1000) - 8);
			/* 7 = 6+1 == MIN LEN +IC_FS_SPKLEN */
			//lcnt = (u16)(((IC_CLK * MIN_HS100pF_SCL_LOWTIME) / 1000) - 1);
			hcnt = 6;
			lcnt = 8;
		}
		debug("__dw_i2c_set_bus_speed, MAX, i2s_spd=%d, hcnt=%d, lcnt=%d\n", i2c_spd, hcnt, lcnt);
		writel(CONFIG_SYS_I2C_BASE + ic_hs_scl_hcnt, hcnt);
		writel(CONFIG_SYS_I2C_BASE + ic_hs_scl_lcnt, lcnt);
		break;
#endif

	case IC_SPEED_MODE_STANDARD:
		cntl |= IC_CON_SPD_SS;
		if (scl_sda_cfg) {
			hcnt = scl_sda_cfg->ss_hcnt;
			lcnt = scl_sda_cfg->ss_lcnt;
		} else {
			hcnt = (((IC_CLK * MIN_SS_SCL_HIGHTIME) / 1000) - 7);
			lcnt = (((IC_CLK * MIN_SS_SCL_LOWTIME) / 1000) - 1);
			printf("hcnt=%lu, lcnt=%lu\n", hcnt, lcnt);
		}
		debug("__dw_i2c_set_bus_speed, SS, ic_clk=%ld, i2s_spd=%lu\n", IC_CLK, i2c_spd);
		writel(CONFIG_SYS_I2C_BASE + ic_ss_scl_hcnt, hcnt);
		writel(CONFIG_SYS_I2C_BASE + ic_ss_scl_lcnt, lcnt);
		break;

	case IC_SPEED_MODE_FAST:
	default:
		cntl |= IC_CON_SPD_FS;
		if (scl_sda_cfg) {
			hcnt = scl_sda_cfg->fs_hcnt;
			lcnt = scl_sda_cfg->fs_lcnt;
		} else {
			hcnt = (u16)(((IC_CLK * MIN_FS_SCL_HIGHTIME) / 1000) - 7);
			lcnt = (u16)(((IC_CLK * MIN_FS_SCL_LOWTIME) / 1000) - 1);
		}
		debug("__dw_i2c_set_bus_speed, FS, ic_clk=%d, i2s_spd=%d, hcnt=%d, lcnt=%d\n", IC_CLK,
		      i2c_spd, hcnt, lcnt);
		writel(CONFIG_SYS_I2C_BASE + ic_fs_scl_hcnt, hcnt);
		writel(CONFIG_SYS_I2C_BASE + ic_fs_scl_lcnt, lcnt);
		break;
	}

	writel(CONFIG_SYS_I2C_BASE + ic_con, cntl);

	/* Configure SDA Hold Time if required */
	if (scl_sda_cfg)
		writel(CONFIG_SYS_I2C_BASE + ic_sda_hold, scl_sda_cfg->sda_hold);

	/* Enable back i2c now speed set */
	dw_i2c_enable(true);

	return 0;
}

/*
 * i2c_setaddress - Sets the target slave address
 * @i2c_addr:	target i2c address
 *
 * Sets the target slave address.
 */
static void i2c_setaddress(unsigned int i2c_addr)
{
	/* Disable i2c */
	dw_i2c_enable(false);
	writel(CONFIG_SYS_I2C_BASE + ic_tar, i2c_addr);
	/* Enable i2c */
	dw_i2c_enable(true);
}

/*
 * i2c_flush_rxfifo - Flushes the i2c RX FIFO
 *
 * Flushes the i2c RX FIFO
 */
static void i2c_flush_rxfifo()
{
	while (readl(CONFIG_SYS_I2C_BASE + ic_status) & IC_STATUS_RFNE)
		readl(CONFIG_SYS_I2C_BASE + ic_cmd_data);
}

/*
 * i2c_wait_for_bb - Waits for bus busy
 *
 * Waits for bus busy
 */
static int i2c_wait_for_bb()
{
	timer_meter_start();

	while ((readl(CONFIG_SYS_I2C_BASE + ic_status) & IC_STATUS_MA) ||
	       !(readl(CONFIG_SYS_I2C_BASE + ic_status) & IC_STATUS_TFE)) {
		/* Evaluate timeout */
		if (timer_meter_get_ms() > (unsigned long)(I2C_BYTE_TO_BB))
			return 1;
	}

	return 0;
}

static int i2c_xfer_init(uchar chip, uint addr, int alen)
{
	if (i2c_wait_for_bb())
		return 1;

	i2c_setaddress(chip);

	while (alen) {
		alen--;
		/* high byte address going out first */
		dw_i2c_write_cmd_data((addr >> (alen * 8)) & 0xff);
		//writel(CONFIG_SYS_I2C_BASE + ic_cmd_data, (addr >> (alen * 8)) & 0xff);
	}
	return 0;
}

static int i2c_xfer_finish()
{
	timer_meter_start();

#ifdef CONFIG_USE_I2C_DMA
	memset(tx_data.data, 0, sizeof(tx_data.data));
	tx_data.idx = 0;
	memset(rx_data, 0, sizeof(rx_data));
#endif

	while (1) {
		if ((readl(CONFIG_SYS_I2C_BASE + ic_raw_intr_stat) & IC_STOP_DET)) {
			readl(CONFIG_SYS_I2C_BASE + ic_clr_stop_det);
			break;
		} else if (timer_meter_get_ms() > I2C_STOPDET_TO) {
			break;
		}
	}

	if (i2c_wait_for_bb()) {
		printf("Timed out waiting for bus\n");
		return 1;
	}

	i2c_flush_rxfifo();

	return 0;
}

/*
 * i2c_read - Read from i2c memory
 * @chip:	target i2c address
 * @addr:	address to read from
 * @alen:
 * @buffer:	buffer for read data
 * @len:	no of bytes to be read
 *
 * Read from i2c memory.
 */
static int __dw_i2c_read(uint dev, uint addr, int alen, u8 *buffer,
			 int len)
{
	xdata unsigned int active = 0;
	xdata unsigned int time_count = 0;

	if (i2c_xfer_init(dev, addr, alen))
		return 1;

	timer_meter_start();
	printf("R: dev=0x%lx addr=0x%lx alen=%d len=%d\n", dev, addr, alen, len);
	while (len) {
		if (!active) {
			/*
			 * Avoid writing to ic_cmd_data multiple times
			 * in case this loop spins too quickly and the
			 * ic_status RFNE bit isn't set after the first
			 * write. Subsequent writes to ic_cmd_data can
			 * trigger spurious i2c transfer.
			 */
			dw_i2c_write_cmd_data((dev << 1) | BIT_I2C_CMD_DATA_READ_BIT |
								BIT_I2C_CMD_DATA_STOP_BIT);
			//writel(CONFIG_SYS_I2C_BASE + ic_cmd_data, (dev <<1) | BIT_I2C_CMD_DATA_READ_BIT | BIT_I2C_CMD_DATA_STOP_BIT);
			active = 1;
		}

		if (readl(CONFIG_SYS_I2C_BASE + ic_raw_intr_stat) & BIT_I2C_INT_RX_FULL) {
#ifdef CONFIG_USE_I2C_DMA
			unsigned int idx;
			//			printf("Use DMA\n");
			dma_dev2mem_setting((unsigned int *)rx_data, len * sizeof(u32),
					    CONFIG_SYS_I2C_BASE + ic_cmd_data, DMA_I2C0);
			dma_start_receive(DMA_I2C0);
			for (idx = 0; idx < len; idx++)
				*buffer++ = rx_data[idx];
			break;
#else
			*buffer++ = (uchar)readl(CONFIG_SYS_I2C_BASE + ic_cmd_data);
			len--;
#endif
			timer_meter_start();
			active = 0;
		} else if ((time_count = timer_meter_get_ms()) > I2C_BYTE_TO) {
			printf("__dw_i2c_read, i2c timeout time=%d ms\n", time_count);
			return 1;
		}
	}

	puts("R10");
	return i2c_xfer_finish();
}

/*
 * i2c_write - Write to i2c memory
 * @chip:	target i2c address
 * @addr:	address to read from
 * @alen:
 * @buffer:	buffer for read data
 * @len:	no of bytes to be read
 *
 * Write to i2c memory.
 */
static int __dw_i2c_write(u8 dev, uint addr, int alen, u8 *buffer,
			  int len)
{
	xdata int nb = len;

	if (i2c_xfer_init(dev, addr, alen))
		return 1;

	timer_meter_start();
	while (len) {
		if (readl(CONFIG_SYS_I2C_BASE + ic_status) & IC_STATUS_TFNF) {
			if (--len == 0) {
				dw_i2c_write_cmd_data(*buffer | IC_STOP);
				//writel(CONFIG_SYS_I2C_BASE + ic_cmd_data, *buffer | IC_STOP);
			} else {
				dw_i2c_write_cmd_data(*buffer);
				//writel(CONFIG_SYS_I2C_BASE + ic_cmd_data, *buffer);
			}
			buffer++;
			timer_meter_start();
		} else if (timer_meter_get_ms() > (nb * I2C_BYTE_TO)) {
			printf("Timed out. i2c write Failed\n");
			return 1;
		}
	}
	return i2c_xfer_finish();
}

/*
 * __dw_i2c_init - Init function
 * @speed:	required i2c speed
 * @slaveaddr:	slave address for the device
 *
 * Initialization function.
 */
static void __dw_i2c_init(int32_t speed, int slaveaddr)
{
	debug("__dw_i2c_init\n");
	/* Disable i2c */
	dw_i2c_enable(false);
	writel(CONFIG_SYS_I2C_BASE + ic_con, (IC_CON_SD | IC_CON_SPD_FS | IC_CON_MM | IC_CON_RE));
	writel(CONFIG_SYS_I2C_BASE + ic_rx_tl, IC_RX_TL);
	writel(CONFIG_SYS_I2C_BASE + ic_tx_tl, IC_TX_TL);
	writel(CONFIG_SYS_I2C_BASE + ic_intr_mask, IC_STOP_DET);
	__dw_i2c_set_bus_speed(NULL, speed);
	writel(CONFIG_SYS_I2C_BASE + ic_sar, slaveaddr);

	/* Enable i2c */
#ifdef CONFIG_USE_I2C_DMA
	writel(CONFIG_SYS_I2C_BASE + ic_dma_cr, BIT_I2C_DMA_CR_RDMAE | BIT_I2C_DMA_CR_TDMAE);
	writel(CONFIG_SYS_I2C_BASE + ic_dma_tdlr, 0x7);
	writel(CONFIG_SYS_I2C_BASE + ic_dma_rdlr, 0x0);
#endif
	dw_i2c_enable(true);
}

void bm_i2c_init(void)
{
	xdata u8 tmp[4];
	xdata int ret;
        xdata u8 dev = 0x3c;
	xdata int32_t speed = I2C_STANDARD_SPEED;

	printf("A2 base=0x%08lx speed=%lu addr=0x%02x\n", CONFIG_SYS_I2C_BASE, speed, (int)dev);
	__dw_i2c_init(speed, dev);
	printf("__dw_i2c_init done\n");
	ret = __dw_i2c_read(dev, 0x16, 2, tmp, 1);
	printf("dw_i2c_read: ret=%d, val=0x%02X\n", ret, (unsigned int)tmp[0]);
}

int test_i2c()
{
	printf("test_i2c()\n");

	printf("RTC_I2C_BASE=0x%08lx\n", read_robot(RTC_I2C_BASE));
	printf("RTC_I2C_BASE+0x04=0x%08lx\n", read_robot(RTC_I2C_BASE + 0x04));

	bm_i2c_init();

	return 0;
}

#endif /* TEST_I2C */