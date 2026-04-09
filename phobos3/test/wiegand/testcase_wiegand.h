/*
 * Copyright (C) Cvitek Co., Ltd. 2019-2020. All rights reserved.
 *
 * File Name: cvi_saradc_ioctl.h
 * Description:
 */

#ifndef __TESTCASE_WIEGAND_H__
#define __TESTCASE_WIEGAND_H__

#define TX_CONFIG0			0x00
#define TX_CONFIG1			0x04
#define TX_CONFIG2			0x08
#define TX_BUFFER			0x0C
#define TX_BUFFER1			0x10
#define TX_TRIGGER			0x14
#define TX_BUSY				0x18

#define RX_CONFIG0			0x20
#define RX_CONFIG1			0x24
#define RX_CONFIG2			0x28
#define RX_BUFFER			0x2C
#define RX_BUFFER1			0x30
#define RX_BUFFER_VALID		0x38
#define RX_BUFFER_CLEAR		0x3C

#define IRQ_ENABLE			0x44
#define IRQ_FLAG			0x48
#define IRQ_CLEAR			0x4C


struct wgn_tx_cfg {
	uint32_t tx_lowtime;
	uint32_t tx_hightime;
	uint32_t tx_bitcount;
	uint32_t tx_msb1st;
	uint32_t tx_opendrain;
};

struct wgn_rx_cfg {
	uint32_t rx_debounce;
	uint32_t rx_idle_timeout;
	uint32_t rx_bitcount;
	uint32_t rx_msb1st;
};

enum wgn_dev{
    WIEGAND0=0,
    WIEGAND1,
    WIEGAND2
};


struct cvi_wiegand_device {
	enum wgn_dev dev;
	uintptr_t  wiegand_vaddr;
	int wiegand_irq;
	uint64_t tx_data;
};

#define BASE_WIEGAND0       0x03030000
#define BASE_WIEGAND1       0x03031000
#define BASE_WIEGAND2       0x03032000
#define FMUX_REG_IOCTRL_SD0_D0     0x03001008
#define FMUX_REG_IOCTRL_SD0_D1     0x0300100c
#define FMUX_REG_IOCTRL_SD0_D2     0x03001010
#define FMUX_REG_IOCTRL_SD0_D3     0x03001014
#define FMUX_REG_IOCTRL_PWR_WAKEUP0 0x0300106c
#define FMUX_REG_IOCTRL_PWR_BUTTON1 0x03001070

#endif // __CVI_WIEGAND_IOCTL_H__
