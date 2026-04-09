#include <common.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "timer.h"
#include "system_common.h"
#include "module_testcase.h"
#include "snr_bm_spi.h"


#undef uartlog
#define uartlog(fmt, args...) \
{ \
    printf(fmt, ##args); \
    fflush(stdout); \
}



uint8_t default_tx[] = {
	0x01, 0x02, 0x03, 0x04, 0x05, 0x06,
	0x07, 0x08, 0x09, 0x10, 0x11, 0x12,
	0x13, 0x14, 0x15, 0x16, 0x17, 0x18,
	0x19, 0x20, 0x21, 0x22, 0x23, 0x24,
	0x25, 0x26, 0x26, 0x28, 0x29, 0x30,
	0xF0, 0x0D,
};

uint8_t default_rx[sizeof(default_tx)] = {0, };

struct dw_spi {
	/* Current message transfer state info */
	u16		len;
	void		*tx;
	void		*tx_end;
	void		*rx;
	void		*rx_end;
	u8		n_bytes;
};


inline static uint8_t reverse8(uint8_t d8)
{
	d8 = ((d8 >> 4) & 0x0F) | ((d8 << 4) & 0xF0);
	d8 = ((d8 >> 2) & 0x33) | ((d8 << 2) & 0xCC);
	d8 = ((d8 >> 1) & 0x55) | ((d8 << 1) & 0xAA);
	return d8;
}


static void dw_spi_enable(bool enable)
{
	if (enable == true)
		SPI_WRITE(DW_SPI_SSIENR, 0x1);
	else
		SPI_WRITE(DW_SPI_SSIENR, 0x0);

	udelay(25);

	uartlog("%sabling SPI\n", enable ? "En" : "Dis");
}


static int spi_rx(void *rx_buf, u16 len, u8 n_bytes)
{
	u32 max = len;
	u16 rxw;

	while (max--) {
		rxw = SPI_READ(DW_SPI_DR);
		/* Care rx only if the transfer's original "rx" is not null */
		if (n_bytes == 1)
			*(u8 *)(rx_buf) = rxw;
		else if (n_bytes == 2)
			*(u16 *)(rx_buf) = rxw;
		else {
			uartlog("No support of this format\n");
			return 1;
		}
		//rx_buf += n_bytes;
		rx_buf = (void *)((uint8_t *)rx_buf + n_bytes);
	}
	return 0;
}


static int spi_tx(void *tx_buf, u16 len, u8 n_bytes)
{
	u32 max = len;
	u16 txw = 0;

	timer_meter_start();
	while (max > 0) {
		/* Set the tx word if the transfer's original "tx" is not null */
		if ((SPI_READ(DW_SPI_RISR) & SPI_RISR_TXOIR) != SPI_RISR_TXOIR) {
			timer_meter_start();

			if (n_bytes == 1)
				txw = *(u8 *)(tx_buf);
			else
				txw = *(u16 *)(tx_buf);

			SPI_WRITE(DW_SPI_DR, txw);
			//tx_buf += n_bytes;
			tx_buf = (void *)((uint8_t *)tx_buf + n_bytes);
			max--;
		} else if (timer_meter_get_ms() < 100) {
			udelay(10); /* wait FIFO exit overflow status */
		} else {
			uartlog("SPI TX timeout\n");
			return 1;
		}
	}
	return 0;
}

#if 0
static int spi_loopback(struct dw_spi *dws)
{
	u32 max = dws->len;
	u16 txw = 0;
	u16 rxw = 0;

	timer_meter_start();
	while (max > 0) {
		/* Set the tx word if the transfer's original "tx" is not null */
		if ((SPI_READ(DW_SPI_RISR) & SPI_RISR_TXOIR) != SPI_RISR_TXOIR) {
			timer_meter_start();

			if (dws->n_bytes == 1)
				txw = *(u8 *)(dws->tx);
			else
				txw = *(u16 *)(dws->tx);
			SPI_WRITE(DW_SPI_DR, txw);
			//dws->tx += dws->n_bytes;
			dws->tx = (void *)((uint8_t *)dws->tx + dws->n_bytes);
			udelay(1);

			rxw = SPI_READ(DW_SPI_DR);

			if (dws->n_bytes == 1)
				*(u8 *)(dws->rx) = rxw;
			else
				*(u16 *)(dws->rx) = rxw;

			//dws->rx += dws->n_bytes;
			dws->rx = (void *)((uint8_t *)dws->rx + dws->n_bytes);
			max--;
		} else if (timer_meter_get_ms() < 100) {
			udelay(10); /* wait FIFO exit overflow status */
		} else {
			uartlog("SPI TX timeout\n");
			return 1;
		}
	}
	return 0;
}
#endif

/*
 * The legacy I2C functions. These need to get removed once
 * all users of this driver are converted to DM.
 */
static void spi_init(uint8_t n_bytes)
{
	u32 ctrl0 = 0;

	uartlog("%s\n", __func__);

	/* Disable i2c */
	dw_spi_enable(false);
	ctrl0 = (SPI_READ(DW_SPI_CTRL0) & ~(SPI_TMOD_MASK
					| SPI_SCPOL_MASK
					| SPI_SCPH_MASK
					| SPI_FRF_MASK
					| SPI_DFS_MASK));
	if (n_bytes == 1) /* 8 bits data*/
		SPI_WRITE(DW_SPI_CTRL0, ctrl0| SPI_SCPOL_LOW | SPI_SCPH_MIDDLE | SPI_FRF_SPI | SPI_DFS_8BIT); /* Set to SPI frame format */
	else if (n_bytes == 2)
		SPI_WRITE(DW_SPI_CTRL0, ctrl0| SPI_SCPOL_LOW | SPI_SCPH_MIDDLE | SPI_FRF_SPI | SPI_DFS_16BIT);
	SPI_WRITE(DW_SPI_BAUDR, SPI_BAUDR_DIV);
	SPI_WRITE(DW_SPI_TXFLTR, 4);
	SPI_WRITE(DW_SPI_RXFLTR, 4);
	SPI_WRITE(DW_SPI_SER, 0x1); /* enable slave 1 device*/

	/* Enable i2c */
	dw_spi_enable(true);

    #if (SPI_CS_BY_GPIO == 1)
    uartlog("SPI_CS using gpio ...\n");
    writel(HARD_REG_SPI0_CS, 0x1030);
    writel(HARD_REG_GPIO0_DAT, 0);
    writel(HARD_REG_GPIO0_OE, (1 << 31));
    #endif
}

static void snr_spi_init(struct snr_spi_adapter *adpt, int slaveaddr, int alen, uint8_t n_bytes)
{
	uartlog("spi verify: %d %d\n", slaveaddr, alen);
	adpt->alen = alen;
	adpt->slaveaddr = slaveaddr;

	spi_init(n_bytes);
}

static int snr_spi_rx(struct snr_spi_adapter *adpt, uint32_t addr, uint8_t *buffer, int len)
{
	int abyte = 0, dbyte = 0, i;
	uint8_t *dat8 = (uint8_t *)default_rx;

	// TBD TBD TBD TBD TBD TBD TBD TBD
	uartlog("spi rx is not verified, tbd\n");

	dat8[dbyte++] = adpt->slaveaddr;

	for (; abyte < adpt->alen; ++abyte, ++dbyte)
		dat8[dbyte] = ((addr >> ((adpt->alen - 1 - abyte) << 3)) & 0xFF);

	spi_rx(dat8, 1 + adpt->alen + len, 1/*n_bytes*/);

	for (i = 0; i < len; ++i, ++dbyte)
		buffer[i] = dat8[dbyte];

	return 0;
}

static int snr_spi_tx(struct snr_spi_adapter *adpt, uint32_t addr, uint8_t *buffer, int len)
{
    int dbyte = 0, i;
    uint8_t *dat8 = (uint8_t *)default_tx;

    dat8[dbyte++] = adpt->slaveaddr;

    for (int abyte = 0; abyte < adpt->alen; ++abyte, ++dbyte)
        dat8[dbyte] = ((addr >> ((adpt->alen - 1 - abyte) << 3)) & 0xFF);

    for (i = 0; i < len; ++i, ++dbyte)
        dat8[dbyte] = buffer[i];

    for (i = 0; i < dbyte; ++i) {
        dat8[i] = reverse8(dat8[i]);
    }

    //dw_spi_enable(true);
    //SPI_WRITE(DW_SPI_SER, 0x1);
    #if (SPI_CS_BY_GPIO == 1)
    writel(HARD_REG_GPIO0_DAT, 0);
    #endif

    spi_tx(dat8, dbyte, 1/*n_bytes*/);

    //dw_spi_enable(false);
    //SPI_WRITE(DW_SPI_SER, 0x1);
    #if (SPI_CS_BY_GPIO == 1)
    udelay(200);
    writel(HARD_REG_GPIO0_DAT, 1 << 31);
    #endif

	return 0;
}

struct snr_spi_adapter m_snr_spi_adpt = {
	.init = snr_spi_init,
	.read = snr_spi_rx,
	.write = snr_spi_tx,
	.name = "internal_snr_spi_adpt",
};

struct snr_spi_adapter *snr_bm_spi_get_adpt(void *arg)
{
	return &m_snr_spi_adpt;
}

