/*
 * Copyright (C) 2018 bitmain
 */

#ifndef __SNR_BM_SPI_H_
#define __SNR_BM_SPI_H_

#define SPI_BASE			0x04180000

/* Register offsets */
#define DW_SPI_CTRL0			0x00
#define DW_SPI_CTRL1			0x04
#define DW_SPI_SSIENR			0x08
#define DW_SPI_MWCR			0x0c
#define DW_SPI_SER			0x10
#define DW_SPI_BAUDR			0x14
#define DW_SPI_TXFLTR			0x18
#define DW_SPI_RXFLTR			0x1c
#define DW_SPI_TXFLR			0x20
#define DW_SPI_RXFLR			0x24
#define DW_SPI_SR			0x28
#define DW_SPI_IMR			0x2c
#define DW_SPI_ISR			0x30
#define DW_SPI_RISR			0x34
#define DW_SPI_TXOICR			0x38
#define DW_SPI_RXOICR			0x3c
#define DW_SPI_RXUICR			0x40
#define DW_SPI_MSTICR			0x44
#define DW_SPI_ICR			0x48
#define DW_SPI_DMACR			0x4c
#define DW_SPI_DMATDLR			0x50
#define DW_SPI_DMARDLR			0x54
#define DW_SPI_IDR			0x58
#define DW_SPI_VERSION			0x5c
#define DW_SPI_DR			0x60

/* CTRL0 filed*/
#define SPI_TMOD_MASK		(0x3 << 8)
#define	SPI_TMOD_TR			0x0000		/* xmit & recv */
#define SPI_TMOD_TO			0x0100		/* xmit only */
#define SPI_TMOD_RO			0x0200		/* recv only */
#define SPI_EERPOMREAD      0x0300      /* EEPROM READ */
#define SPI_FRF_MASK        0x0030
#define SPI_SCPOL_MASK      0x0080
#define SPI_SCPOL_LOW       0x0000
#define SPI_SCPOL_HGIH      0x0080
#define SPI_SCPH_MASK       0x0040
#define SPI_SCPH_MIDDLE     0x0000
#define SPI_SCPH_START      0x0040
#define SPI_FRF_SPI			0x0
#define SPI_DFS_MASK        0x000F
#define SPI_DFS_8BIT        0x0007
#define SPI_DFS_16BIT       0x000F


/* RISR filed */
#define SPI_RISR_RXFIR      0x10
#define SPI_RISR_RXOIR      0x08
#define SPI_RISR_RXUIR      0x04
#define SPI_RISR_TXOIR      0x02
#define SPI_RISR_TXEIR      0x01

#define SPI_BAUDR_DIV       0x32

#define SPI_WRITE(offset, value) \
	writel(SPI_BASE + offset , (u32)value)

#define SPI_READ(offset) \
	readl(SPI_BASE + offset)



/* fpga bitfile v1 on 1880v2 */
#define HARD_REG_SPI0_CS        (0x0300109C)
#define HARD_REG_GPIO0_DAT      (0x03020000) //GPIO31
#define HARD_REG_GPIO0_OE       (0x03020004)
/* end fpga bitfile v1 on 1880v2 */

#define SPI_CS_BY_GPIO          (1)



struct snr_spi_adapter {
	void	(*init)(struct snr_spi_adapter *adpt, int slaveaddr, int alen, uint8_t n_bytes);
	int	(*read)(struct snr_spi_adapter *adpt, uint32_t addr, uint8_t *buffer, int len);
	int	(*write)(struct snr_spi_adapter *adpt, uint32_t addr, uint8_t *buffer, int len);
	int	alen;
	int	slaveaddr;
	char 	*name;
};


struct snr_spi_adapter *snr_bm_spi_get_adpt(void *arg);


#endif /* __SNR_BM_I2C_H */
