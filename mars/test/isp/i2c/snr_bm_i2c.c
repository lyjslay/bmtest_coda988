/*
 * (C) Copyright 2009
 * Vipin Kumar, ST Micoelectronics, vipin.kumar@st.com.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <stdlib.h>
#include <stdbool.h>
#include "timer.h"
#include "system_common.h"
#include "snr_bm_i2c.h"


#undef uartlog
#define uartlog(fmt, args...) \
{ \
    printf(fmt, ##args); \
    fflush(stdout); \
}


static void dw_i2c_init(struct snr_i2c_adapter *adap, int speed, int slaveaddr);
static int dw_i2c_probe(struct snr_i2c_adapter *adap, u8 dev);
static int dw_i2c_read(struct snr_i2c_adapter *adap, u8 dev, uint addr,
		       int alen, u8 *buffer, int len);
static int dw_i2c_write(struct snr_i2c_adapter *adap, u8 dev, uint addr,
			int alen, u8 *buffer, int len);
static unsigned int dw_i2c_set_bus_speed(struct snr_i2c_adapter *adap,
					 unsigned int speed);



struct dw_scl_sda_cfg {
	u32 ss_hcnt;
	u32 fs_hcnt;
	u32 ss_lcnt;
	u32 fs_lcnt;
	u32 sda_hold;
};


struct snr_i2c_adapter m_snr_i2c_adp = {
	.init = dw_i2c_init,
	.probe = dw_i2c_probe,
 	.read = dw_i2c_read,
	.write = dw_i2c_write,
	.set_bus_speed = dw_i2c_set_bus_speed,
	.speed = I2C_STANDARD_SPEED,
	.waitdelay = 0,
	.slaveaddr = 0x1A, //need << 1 for r/w direction
	.init_done = 0,
	.hwadapnr = 0,
	.name = "dw_0",
};


void snr_bm_i2c_get_adap(uchar **adap)
{
    *adap=(uchar *)&m_snr_i2c_adp;
}

static void dw_i2c_enable(struct i2c_regs *i2c_base, bool enable)
{
	u32 ena = enable ? IC_ENABLE_0B : 0;
	int timeout = 10000;//100;

	do {
		writel(&i2c_base->ic_enable, ena);
		if ((readl(&i2c_base->ic_enable_status) & IC_ENABLE_0B) == ena)
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
static unsigned int __dw_i2c_set_bus_speed(struct i2c_regs *i2c_base,
						struct dw_scl_sda_cfg *scl_sda_cfg,
						unsigned int speed)
{
	unsigned int cntl;
	unsigned int hcnt, lcnt;
//	unsigned int high_time;
//	unsigned int low_time;
//	unsigned int temp_x;
	int i2c_spd;

	if (speed > I2C_FAST_SPEED)
		i2c_spd = IC_SPEED_MODE_MAX;
	else if ((speed <= I2C_FAST_SPEED) && (speed > I2C_STANDARD_SPEED))
		i2c_spd = IC_SPEED_MODE_FAST;
	else
		i2c_spd = IC_SPEED_MODE_STANDARD;

	/* to set speed cltr must be disabled */
	dw_i2c_enable(i2c_base, false);

	cntl = (readl(&i2c_base->ic_con) & (~IC_CON_SPD_MSK));

	switch (i2c_spd) {
#ifndef CONFIG_X86 /* No High-speed for BayTrail yet */
	case IC_SPEED_MODE_MAX:
		cntl |= IC_CON_SPD_HS;
		if (scl_sda_cfg) {
			hcnt = scl_sda_cfg->fs_hcnt;
			lcnt = scl_sda_cfg->fs_lcnt;
		} else {
			hcnt = (u16)(((IC_CLK * MIN_HS100pF_SCL_HIGHTIME) / 1000) - 8);
			/* 7 = 6+1 == MIN LEN +IC_FS_SPKLEN */
			lcnt = (u16)(((IC_CLK * MIN_HS100pF_SCL_LOWTIME) / 1000) - 1);
		}
        debug("%s, MAX, i2s_spd=%d, hcnt=%d, lcnt=%d\n", __func__, i2c_spd, hcnt, lcnt);
		writel(&i2c_base->ic_hs_scl_hcnt, hcnt);
		writel(&i2c_base->ic_hs_scl_lcnt, lcnt);
		break;
#endif

	case IC_SPEED_MODE_STANDARD:
		cntl |= IC_CON_SPD_SS;
		if (scl_sda_cfg) {
			hcnt = scl_sda_cfg->ss_hcnt;
			lcnt = scl_sda_cfg->ss_lcnt;
		} else {
			hcnt = (u16)(((IC_CLK * MIN_SS_SCL_HIGHTIME) / 1000) - 7);
			lcnt = (u16)(((IC_CLK * MIN_SS_SCL_LOWTIME) / 1000) - 1);
		}
		debug("%s, SS, ic_clk=%d, i2s_spd=%d, hcnt=%d, lcnt=%d\n", __func__, IC_CLK, i2c_spd, hcnt, lcnt);
		writel(&i2c_base->ic_ss_scl_hcnt, hcnt);
		writel(&i2c_base->ic_ss_scl_lcnt, lcnt);
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
		debug("%s, FS, ic_clk=%d, i2s_spd=%d, hcnt=%d, lcnt=%d\n", __func__, IC_CLK, i2c_spd, hcnt, lcnt);
		writel(&i2c_base->ic_fs_scl_hcnt, hcnt);
		writel(&i2c_base->ic_fs_scl_lcnt, lcnt);
		break;
	}

	writel(&i2c_base->ic_con, cntl);

	/* Configure SDA Hold Time if required */
	if (scl_sda_cfg)
		writel(&i2c_base->ic_sda_hold, scl_sda_cfg->sda_hold);

	/* Enable back i2c now speed set */
	dw_i2c_enable(i2c_base, true);

	return 0;
}

/*
 * i2c_setaddress - Sets the target slave address
 * @i2c_addr:	target i2c address
 *
 * Sets the target slave address.
 */
static void i2c_setaddress(struct i2c_regs *i2c_base, unsigned int i2c_addr)
{
	/* Disable i2c */
	dw_i2c_enable(i2c_base, false);
	writel(&i2c_base->ic_tar, i2c_addr);
	/* Enable i2c */
	dw_i2c_enable(i2c_base, true);
}

/*
 * i2c_flush_rxfifo - Flushes the i2c RX FIFO
 *
 * Flushes the i2c RX FIFO
 */
static void i2c_flush_rxfifo(struct i2c_regs *i2c_base)
{
	while (readl(&i2c_base->ic_status) & IC_STATUS_RFNE)
		readl(&i2c_base->ic_cmd_data);
}

/*
 * i2c_wait_for_bb - Waits for bus busy
 *
 * Waits for bus busy
 */
static int i2c_wait_for_bb(struct i2c_regs *i2c_base)
{
	timer_meter_start();

	while ((readl(&i2c_base->ic_status) & IC_STATUS_MA) ||
	       !(readl(&i2c_base->ic_status) & IC_STATUS_TFE)) {

		/* Evaluate timeout */
		  if (timer_meter_get_ms() > (unsigned long)(I2C_BYTE_TO_BB)) {
			#if 1
			printf("[TBD]%s timeout: %d force okkkkk\n", __func__, timer_meter_get_ms()); fflush(stdout);
			return 0;
			#else
			return 1;
			#endif
		  }
	}
        #if 0//dbg only
        printf("force delay\n"); fflush(stdout);
        while (1) {
                if (timer_meter_get_ms() > (unsigned long)(I2C_BYTE_TO_BB)) {
                        break;
                }
        }
        #endif

	return 0;
}

static int i2c_xfer_init(struct i2c_regs *i2c_base, uchar chip, uint addr,
			int alen)
{
	if (i2c_wait_for_bb(i2c_base))
		return 1;

	i2c_setaddress(i2c_base, chip);

	while (alen) {
		alen--;
		/* high byte address going out first */
		writel(&i2c_base->ic_cmd_data, (addr >> (alen * 8)) & 0xff);
	}
	return 0;
}

static int i2c_xfer_finish(struct i2c_regs *i2c_base)
{
	timer_meter_start();

	while (1) {
		if ((readl(&i2c_base->ic_raw_intr_stat) & IC_STOP_DET)) {
			readl(&i2c_base->ic_clr_stop_det);
			break;
		} else if (timer_meter_get_ms() > I2C_STOPDET_TO) {
			break;
		}
	}

	if (i2c_wait_for_bb(i2c_base)) {
		printf("Timed out waiting for bus\n");
		return 1;
	}

	i2c_flush_rxfifo(i2c_base);

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
static int __dw_i2c_read(struct i2c_regs *i2c_base, u8 dev, uint addr,
			 int alen, u8 *buffer, int len)
{
	unsigned int active = 0;
	unsigned int time_count = 0;

	if (i2c_xfer_init(i2c_base, dev, addr, alen))
		return 1;

	timer_meter_start();
	while (len) {
		if (!active) {
			/*
			 * Avoid writing to ic_cmd_data multiple times
			 * in case this loop spins too quickly and the
			 * ic_status RFNE bit isn't set after the first
			 * write. Subsequent writes to ic_cmd_data can
			 * trigger spurious i2c transfer.
			 */

			writel(&i2c_base->ic_cmd_data, (dev <<1) | BIT_I2C_CMD_DATA_READ_BIT | BIT_I2C_CMD_DATA_STOP_BIT);
			active = 1;
		}

		if (readl(&i2c_base->ic_raw_intr_stat) & BIT_I2C_INT_RX_FULL) {
			*buffer++ = (uchar)readl(&i2c_base->ic_cmd_data);
			len--;
			timer_meter_start();
			active = 0;
		}
		else if ((time_count = timer_meter_get_ms()) > I2C_BYTE_TO) {
			printf("%s, i2c timeout time=%d ms, return 1\n", __func__, time_count);
			return 1;
		}
	}
	return i2c_xfer_finish(i2c_base);
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
static int __dw_i2c_write(struct i2c_regs *i2c_base, u8 dev, uint addr,
			int alen, u8 *buffer, int len)
{
	int nb = len;

	if (i2c_xfer_init(i2c_base, dev, addr, alen))
		return 1;

	timer_meter_start();
	while (len) {
		if (readl(&i2c_base->ic_status) & IC_STATUS_TFNF) {
			if (--len == 0) {
				writel(&i2c_base->ic_cmd_data, *buffer | IC_STOP);
			} else {
				writel(&i2c_base->ic_cmd_data, *buffer);
			}
			buffer++;
			timer_meter_start();
		} else if (timer_meter_get_ms() > (nb * (I2C_BYTE_TO))) {
				printf("Timed out. i2c write Failed\n");
				return 1;
		}
	}
	return i2c_xfer_finish(i2c_base);
}

/*
 * __dw_i2c_init - Init function
 * @speed:	required i2c speed
 * @slaveaddr:	slave address for the device
 *
 * Initialization function.
 */
static void __dw_i2c_init(struct i2c_regs *i2c_base, int speed, int slaveaddr)
{
    debug("%s\n", __func__);
	/* Disable i2c */
	dw_i2c_enable(i2c_base, false);
	writel(&i2c_base->ic_con, (IC_CON_SD | IC_CON_SPD_FS | IC_CON_MM | IC_CON_RE));
	writel(&i2c_base->ic_rx_tl, IC_RX_TL);
	writel(&i2c_base->ic_tx_tl, IC_TX_TL);
	writel(&i2c_base->ic_intr_mask, IC_STOP_DET);
	__dw_i2c_set_bus_speed(i2c_base, NULL, speed);
	writel(&i2c_base->ic_sar, slaveaddr);
	/* Enable i2c */
	dw_i2c_enable(i2c_base, true);
}

/*
 * The legacy I2C functions. These need to get removed once
 * all users of this driver are converted to DM.
 */
static struct i2c_regs *i2c_get_base(struct snr_i2c_adapter *adap)
{
	return (struct i2c_regs *)CONFIG_SYS_I2C_BASE;
}

static unsigned int dw_i2c_set_bus_speed(struct snr_i2c_adapter *adap,
					unsigned int speed)
{
	adap->speed = speed;
	return __dw_i2c_set_bus_speed(i2c_get_base(adap), NULL, speed);
}

static void dw_i2c_init(struct snr_i2c_adapter *adap, int speed, int slaveaddr)
{
	__dw_i2c_init(i2c_get_base(adap), speed, slaveaddr);
}

static int dw_i2c_read(struct snr_i2c_adapter *adap, u8 dev, uint addr,
		       int alen, u8 *buffer, int len)
{
	return __dw_i2c_read(i2c_get_base(adap), dev, addr, alen, buffer, len);
}

static int dw_i2c_write(struct snr_i2c_adapter *adap, u8 dev, uint addr,
			int alen, u8 *buffer, int len)
{
	return __dw_i2c_write(i2c_get_base(adap), dev, addr, alen, buffer, len);
}

/* dw_i2c_probe - Probe the i2c chip */
static int dw_i2c_probe(struct snr_i2c_adapter *adap, u8 dev)
{
	struct i2c_regs *i2c_base = i2c_get_base(adap);

	printf("%s, i2c_regs address=%p, dev=%x\n",__func__, i2c_base, dev);
	/*
	 * Try to read the first location of the chip.
	 */
	dw_i2c_init(adap, adap->speed, adap->slaveaddr);

	return 0;
}

void snr_bm_i2c_init(struct snr_i2c_adapter *adpt)
{
    adpt->name = "dw_0";
    dw_i2c_probe(adpt, adpt->slaveaddr);
}
