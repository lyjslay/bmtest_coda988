/*
 * Copyright (C) 2018 bitmain
 */

#include <common.h>
#include <malloc.h>
#include <string.h>
#include "bm_i2s_tdm.h"
#include "system_common.h"
#include "timer.h"
#include "bm_dw_i2c.h"
#include "bm_sound.h"
#include "mmio.h"
#include "adau1372.h"
#include "dma.h"
#include "src_sample.h"

#define TIMEOUT_I2S_TX		2	/* i2s transfer timeout */
//#define I2S_PRINT_RAW_DATA
#define PRINT_COUNT 16 *2

u8 txdma_allocated = 0;
u8 rxdma_allocated = 0;
bool skip_codec_setting = false;

bool concurrent_rx_enable = false;

struct i2s_tdm_regs *i2s_get_base(unsigned int i2s_no)
{
    switch(i2s_no){
        case I2S0:
            return (struct i2s_tdm_regs *)CONFIG_SYS_I2S0_BASE;
        case I2S1:
            return (struct i2s_tdm_regs *)CONFIG_SYS_I2S1_BASE;
        case I2S2:
            return (struct i2s_tdm_regs *)CONFIG_SYS_I2S2_BASE;
        case I2S3:
            return (struct i2s_tdm_regs *)CONFIG_SYS_I2S3_BASE;
		default:
			printf("no such I2S device\n");
			break;
    }
	return NULL;
}

struct i2s_sys_regs *i2s_get_sys_base(void)
{
	return (struct i2s_sys_regs *)CONFIG_SYS_I2S_SYS_BASE;
}

static void i2s_config_dma(struct i2s_tdm_regs *i2s_reg, bool on)
{
    u32 blk_mode_setting = 0;
	u32 blk_cfg = 0;

	blk_mode_setting = readl(&i2s_reg->blk_mode_setting) & ~(DMA_MODE_MASK);
	if (on == true) {
		writel(&i2s_reg->blk_mode_setting, blk_mode_setting | HW_DMA_MODE); /*not to use FIFO */
		writel(&i2s_reg->fifo_threshold, RX_FIFO_THRESHOLD(15)|TX_FIFO_THRESHOLD(15)|TX_FIFO_HIGH_THRESHOLD(31));
	} else {
		writel(&i2s_reg->blk_mode_setting, blk_mode_setting | SW_MODE); /*not to use FIFO */
		writel(&i2s_reg->fifo_threshold, RX_FIFO_THRESHOLD(1)|TX_FIFO_THRESHOLD(1)|TX_FIFO_HIGH_THRESHOLD(1));
	}

	blk_cfg = readl(&i2s_reg->blk_cfg) | AUTO_DISABLE_W_CH_EN;
	writel(&i2s_reg->blk_cfg, blk_cfg);
}

void i2s_set_clk_source(struct i2s_tdm_regs *i2s_reg, unsigned int src)
{

	u32 tmp = 0;

	debug("%s\n", __func__);

	tmp = readl(&i2s_reg->i2s_clk_ctrl0) & ~(AUD_CLK_SOURCE_MASK);

	switch(src) {

	case AUD_CLK_FROM_MCLK_IN:
		tmp |= AUD_CLK_FROM_MCLK_IN | AUD_ENABLE;
		break;
	case AUD_CLK_FROM_PLL:
		tmp |= AUD_CLK_FROM_PLL | AUD_ENABLE;
		break;
	}

	writel(&i2s_reg->i2s_clk_ctrl0, tmp);
	debug("Set clk source from %d\n", src);
}

/*
 * Sets the frame size for I2S sample rate
 *
 * @param i2s_reg	i2s regiter address
 */
void i2s_set_sample_rate(struct i2s_tdm_regs *i2s_reg, unsigned int sample_rate)
{
    u32 frame_setting = 0;
    u32 slot_setting = 0;
    u32 data_format = 0;
    u32 clk_ctrl = 0;
	u32 div_multiplier = 1; /* if use audio PLL (25 or 24.576Mhz), div_multiplier should be 2 */

	printf("Set sample rate to %d\n", sample_rate);

    frame_setting = readl(&i2s_reg->frame_setting);
	slot_setting = readl(&i2s_reg->slot_setting1);
	data_format = readl(&i2s_reg->data_format);
	clk_ctrl = readl(&i2s_reg->i2s_clk_ctrl1);

    frame_setting &= ~(FRAME_LENGTH_MASK | FS_ACT_LENGTH_MASK);
	slot_setting &= ~(SLOT_SIZE_MASK | DATA_SIZE_MASK);
	data_format &= ~(WORD_LENGTH_MASK);
#if defined(CONFIG_USE_AUDIO_PLL)
	clk_ctrl = MCLK_DIV(2); /* audio PLL is 25 or 24.576 Mhz, need to div with 2*/
	div_multiplier = 1;
#else
    clk_ctrl = MCLK_DIV(1); /* mclk_in is 12.288 Mhz, no need to div*/
#endif

	switch(sample_rate) {
		case 8000:
            frame_setting |= FRAME_LENGTH(32) | FS_ACT_LENGTH(16);
			slot_setting |= SLOT_SIZE(16) | DATA_SIZE(16);
			data_format = WORD_LEN_16;
			clk_ctrl |= BCLK_DIV(48 * div_multiplier);
			adau1372_hw_params(sample_rate, WSS_16_CLKCYCLE);
			break;

		case 12000:
            frame_setting |= FRAME_LENGTH(64) | FS_ACT_LENGTH(32);
			slot_setting |= SLOT_SIZE(32) | DATA_SIZE(32);
			data_format = WORD_LEN_32;
			clk_ctrl |= BCLK_DIV(16 * div_multiplier);
			adau1372_hw_params(sample_rate, WSS_32_CLKCYCLE);
			break;

		case 16000:
            frame_setting |= FRAME_LENGTH(64) | FS_ACT_LENGTH(32);
			slot_setting |= SLOT_SIZE(32) | DATA_SIZE(32);
			data_format = WORD_LEN_32;
			clk_ctrl |= BCLK_DIV(12 * div_multiplier);
			adau1372_hw_params(sample_rate, WSS_32_CLKCYCLE);
			break;

		case 24000:
            frame_setting |= FRAME_LENGTH(64) | FS_ACT_LENGTH(32);
			slot_setting |= SLOT_SIZE(32) | DATA_SIZE(32);
			data_format = WORD_LEN_32;
			clk_ctrl |= BCLK_DIV(8 * div_multiplier);
			adau1372_hw_params(sample_rate, WSS_32_CLKCYCLE);
			break;
		case 32000:
            frame_setting |= FRAME_LENGTH(64) | FS_ACT_LENGTH(32);
			slot_setting |= SLOT_SIZE(32) | DATA_SIZE(32);
			data_format = WORD_LEN_32;
			clk_ctrl |= BCLK_DIV(6 * div_multiplier);
			adau1372_hw_params(sample_rate, WSS_32_CLKCYCLE);
			break;

		case 48000:
            frame_setting |= FRAME_LENGTH(64) | FS_ACT_LENGTH(32);
			slot_setting |= SLOT_SIZE(32) | DATA_SIZE(32);
			data_format = WORD_LEN_32;
			clk_ctrl |= BCLK_DIV(4 * div_multiplier);
			adau1372_hw_params(sample_rate, WSS_32_CLKCYCLE);
			break;

		case 96000:
            frame_setting |= FRAME_LENGTH(64) | FS_ACT_LENGTH(32);
			slot_setting |= SLOT_SIZE(32) | DATA_SIZE(32);
			data_format = WORD_LEN_32;
			clk_ctrl |= BCLK_DIV(2 * div_multiplier);
			adau1372_hw_params(sample_rate, WSS_32_CLKCYCLE);
			break;

		case 192000:
            frame_setting |= FRAME_LENGTH(64) | FS_ACT_LENGTH(32);
			slot_setting |= SLOT_SIZE(32) | DATA_SIZE(32);
			data_format = WORD_LEN_32;
			clk_ctrl |= BCLK_DIV(1 * div_multiplier);
			adau1372_hw_params(sample_rate, WSS_32_CLKCYCLE);
			break;
		}

     writel(&i2s_reg->frame_setting, frame_setting);
	 writel(&i2s_reg->slot_setting1, slot_setting);
	 writel(&i2s_reg->data_format, data_format);
	 writel(&i2s_reg->i2s_clk_ctrl1, clk_ctrl);
}

void i2s_set_ws_clock_cycle(struct i2s_tdm_regs *i2s_reg, unsigned int ws_clk, u8 aud_mode)
{

	u32 frame_setting = 0;
	u32 slot_setting1 = 0;

	debug("Set ws clkcycle to 0x%d\n", ws_clk);
	frame_setting = readl(&i2s_reg->frame_setting) & ~(FRAME_LENGTH_MASK | FS_ACT_LENGTH_MASK);
	slot_setting1 = readl(&i2s_reg->slot_setting1) & ~(SLOT_SIZE_MASK | DATA_SIZE_MASK);

	if ((aud_mode == I2S_MODE) || (aud_mode == LJ_MODE) || (aud_mode == RJ_MODE)) {
		switch(ws_clk) {
			case WSS_16_CLKCYCLE:
				frame_setting |= FRAME_LENGTH(32) | FS_ACT_LENGTH(16);
				slot_setting1 |= SLOT_SIZE(16) | DATA_SIZE(16);
				break;
			case WSS_24_CLKCYCLE:
				frame_setting |= FRAME_LENGTH(48) | FS_ACT_LENGTH(24);
				slot_setting1 |= SLOT_SIZE(24) | DATA_SIZE(24);
				break;
			case WSS_32_CLKCYCLE:
				frame_setting |= FRAME_LENGTH(64) | FS_ACT_LENGTH(32);
				slot_setting1 |= SLOT_SIZE(32) | DATA_SIZE(32);
				break;
			case WSS_256_CLKCYCLE:
				frame_setting |= FRAME_LENGTH(512) | FS_ACT_LENGTH(256);
				slot_setting1 |= SLOT_SIZE(64) | DATA_SIZE(32);
				break;
		}
	} else if ((aud_mode == PCM_A_MODE) || (aud_mode == PCM_B_MODE)){
		switch(ws_clk) {
			case WSS_16_CLKCYCLE:
				frame_setting |= FRAME_LENGTH(32) | FS_ACT_LENGTH(1);
				slot_setting1 |= SLOT_SIZE(16) | DATA_SIZE(16);
				break;
			case WSS_24_CLKCYCLE:
				frame_setting |= FRAME_LENGTH(48) | FS_ACT_LENGTH(1);
				slot_setting1 |= SLOT_SIZE(24) | DATA_SIZE(24);
				break;
			case WSS_32_CLKCYCLE:
				frame_setting |= FRAME_LENGTH(64) | FS_ACT_LENGTH(1);
				slot_setting1 |= SLOT_SIZE(32) | DATA_SIZE(32);
				break;
			case WSS_256_CLKCYCLE:
				frame_setting |= FRAME_LENGTH(512) | FS_ACT_LENGTH(1);
				slot_setting1 |= SLOT_SIZE(64) | DATA_SIZE(32);
				break;
		}
	} else if (aud_mode == TDM_MODE){
		unsigned int slot_no = (readl(&i2s_reg->slot_setting1) & SLOT_NUM_MASK)+1;

		debug("set TDM mode with slot number=%d\n", slot_no);
		switch(ws_clk) {
			case WSS_16_CLKCYCLE:
				frame_setting |= FRAME_LENGTH(slot_no * 16) | FS_ACT_LENGTH(1);
				slot_setting1 |= SLOT_SIZE(16) | DATA_SIZE(16);
				break;
			case WSS_24_CLKCYCLE:
				frame_setting |= FRAME_LENGTH(slot_no * 24) | FS_ACT_LENGTH(1);
				slot_setting1 |= SLOT_SIZE(24) | DATA_SIZE(24);
				break;
			case WSS_32_CLKCYCLE:
				frame_setting |= FRAME_LENGTH(slot_no * 32) | FS_ACT_LENGTH(1);
				slot_setting1 |= SLOT_SIZE(32) | DATA_SIZE(32);
				break;
			case WSS_256_CLKCYCLE:
				frame_setting |= FRAME_LENGTH(512) | FS_ACT_LENGTH(1);
				slot_setting1 |= SLOT_SIZE(64) | DATA_SIZE(32);
				break;
		}
	}

	debug("Set ws clk cycle to %d\n", ws_clk);
	writel(&i2s_reg->frame_setting, frame_setting);
	writel(&i2s_reg->slot_setting1, slot_setting1);
	if((ws_clk == WSS_16_CLKCYCLE) || (ws_clk == WSS_32_CLKCYCLE))
		adau1372_update_ws_params(aud_mode, ws_clk);
	else
		printf("Warning!!! ADAU1372 doesn't support this WS/LRCK length\n");


}


void i2s_set_resolution(struct i2s_tdm_regs *i2s_reg, unsigned int data_size, unsigned int slot_size)
{

	u32 slot_setting1 = 0;

	debug("Set resolution to data_size=%d\n", data_size);
	slot_setting1 = readl(&i2s_reg->slot_setting1) & ~(DATA_SIZE_MASK | SLOT_SIZE_MASK);
	slot_setting1 |= DATA_SIZE(data_size) | SLOT_SIZE(slot_size);

	writel(&i2s_reg->slot_setting1, slot_setting1);
}


static void i2s_subsys_bclk_oen_switch(int on, unsigned int i2s_no)
{
	struct i2s_sys_regs *i2s_sys_reg = i2s_get_sys_base();

	u32 bclk_oen_sel = readl(&i2s_sys_reg->i2s_bclk_oen_sel);

	if (on)
		bclk_oen_sel |= 1 << (i2s_no + 8);
	else
		bclk_oen_sel &= ~(1 << (i2s_no + 8));

	writel(&i2s_sys_reg->i2s_bclk_oen_sel, bclk_oen_sel);
}


/*
 * Set the i2s enable
 * @param i2s_req 	i2s register address
 * @param enable 	1 to enable i2s, 0 to disable i2s
*/

static void i2s_switch(int on, struct i2s_tdm_regs *i2s_reg)
{
//	struct i2s_tdm_regs *i2s_reg = i2s_get_base(i2s_no);
	u32 i2s_enable = readl(&i2s_reg->i2s_enable);
	u32 aud_enable = readl(&i2s_reg->i2s_clk_ctrl0);
	u32 role = (readl(&i2s_reg->blk_mode_setting) & ROLE_MASK);

	if (on) {
//		dma_turn_on();
		if (i2s_enable == I2S_OFF)
			writel(&i2s_reg->i2s_enable, I2S_ON);
//		if (((aud_enable & AUD_ENABLE) != AUD_ENABLE) && role == MASTER_MODE)
//			writel(&i2s_reg->i2s_clk_ctrl0, aud_enable | AUD_ENABLE);

	} else {

		if (i2s_enable == I2S_ON)
			writel(&i2s_reg->i2s_enable, I2S_OFF);

		if (((aud_enable & AUD_ENABLE) == AUD_ENABLE) && role == MASTER_MODE)
			writel(&i2s_reg->i2s_clk_ctrl0, aud_enable & ~(AUD_ENABLE));

//		dma_turn_off();
	}
}

/*
 * Sets the i2s transfer control
 *
 * @param i2s_reg	i2s regiter address
 * @param on		1 to enable tx , 0 to disable tx transfer
 */
static void i2s_txctrl(struct i2s_tdm_regs *i2s_reg, int on)
{
    u32 blk_mode_setting = 0;
    u32 clk_ctrl = 0;

    blk_mode_setting = (readl(&i2s_reg->blk_mode_setting) & ~(TXRX_MODE_MASK));
	clk_ctrl = (readl(&i2s_reg->i2s_clk_ctrl0) & ~(AUD_SWITCH));

    blk_mode_setting |= TX_MODE;
    writel(&i2s_reg->blk_mode_setting, blk_mode_setting);

    if ((blk_mode_setting & ROLE_MASK) == MASTER_MODE)
    {
	    if (on)
	    {
		    debug("Enable aud_en\n");
		    writel(&i2s_reg->i2s_clk_ctrl0, clk_ctrl| AUD_ENABLE);
	    } else {
		    debug("Disalbe aud_en\n");
		    writel(&i2s_reg->i2s_clk_ctrl0, clk_ctrl & ~(AUD_ENABLE));
	    }
    } else {
        writel(&i2s_reg->i2s_clk_ctrl0, clk_ctrl & ~(AUD_ENABLE));
    }

}

/*
 * Sets the i2s receiver control
 *
 * @param i2s_reg	i2s regiter address
 * @param on		1 to enable rx , 0 to disable rx transfer
 */
static void i2s_rxctrl(struct i2s_tdm_regs *i2s_reg, int on)
{
    u32 blk_mode_setting = 0;
    u32 clk_ctrl = 0;

    blk_mode_setting = (readl(&i2s_reg->blk_mode_setting) & ~(TXRX_MODE_MASK));
	clk_ctrl = (readl(&i2s_reg->i2s_clk_ctrl0) & ~(AUD_SWITCH));

    blk_mode_setting |= RX_MODE;
    writel(&i2s_reg->blk_mode_setting, blk_mode_setting);

    if ((blk_mode_setting & ROLE_MASK) == MASTER_MODE)
    {
	    if (on)
	    {
		    debug("Enable aud_en\n");
		    writel(&i2s_reg->i2s_clk_ctrl0, clk_ctrl | AUD_ENABLE);
	    } else {
		    debug("Disalbe aud_en\n");
		    writel(&i2s_reg->i2s_clk_ctrl0, clk_ctrl | AUD_DISABLE);
	    }
    } else {
        writel(&i2s_reg->i2s_clk_ctrl0, clk_ctrl | AUD_DISABLE);
    }


}

/*
 * flushes the i2stx fifo
 *
 * @param i2s_reg	i2s regiter address
 */
static void i2s_sw_reset(struct i2s_tdm_regs *i2s_reg)
{

    if ((readl(&i2s_reg->blk_mode_setting) & TXRX_MODE_MASK) == TX_MODE)
    {
        //debug("Reset i2s TX\n");
        writel(&i2s_reg->fifo_reset, TX_FIFO_RESET_PULL_UP);
        udelay(10);
        writel(&i2s_reg->fifo_reset, TX_FIFO_RESET_PULL_DOWN);

        writel(&i2s_reg->i2s_reset, I2S_RESET_TX_PULL_UP);
        timer_meter_start();
		udelay(10);
        while (1) {
            if ((readl(&i2s_reg->tx_status) & RESET_TX_SCLK) >> 23) {
                //debug("TX Reset complete\n");
                break;
            } else if (timer_meter_get_ms() > I2S_TIMEOUT){
                printf("TX Reset Timeout\n");
                break;
            }
        }
         writel(&i2s_reg->i2s_reset, I2S_RESET_TX_PULL_DOWN);

    } else { /* reset RX*/
        //debug("Reset i2s RX\n");
        writel(&i2s_reg->fifo_reset, RX_FIFO_RESET_PULL_UP);
        udelay(10);
        writel(&i2s_reg->fifo_reset, RX_FIFO_RESET_PULL_DOWN);

        writel(&i2s_reg->i2s_reset, I2S_RESET_RX_PULL_UP);
        timer_meter_start();
		udelay(10);
        while (1) {
			u32 tmp=readl(&i2s_reg->rx_status);
			//u32 tmp2=readl(&i2s_reg->i2s_clk_ctrl0);
			//printf("rx_status=0x%x, clk_ctrl0=0x%x\n", tmp, tmp2);
            if ((tmp & RESET_RX_SCLK) >> 23) {
                //debug("RX Reset complete\n");
                break;
            } else if (timer_meter_get_ms() > I2S_TIMEOUT){
                printf("RX Reset Timeout\n");
                break;
            }
        }
         writel(&i2s_reg->i2s_reset, I2S_RESET_RX_PULL_DOWN);
    }
}

void i2s_set_interrupt(struct i2s_tdm_regs *i2s_reg)
{
	writel(&i2s_reg->i2s_int_en, I2S_INT_EN_ALL); /* enable all interrupt mask for TX and RX */
}

void i2s_set_audio_gpio(int role)
{

	switch (role) {
	case MASTER_MODE:
		mmio_clrsetbits_32(REG_AUDIO_GPIO_BASE, 0xf << 0, 0x1);
		break;
	case SLAVE_MODE:
		mmio_clrsetbits_32(REG_AUDIO_GPIO_BASE, 0xf << 0, 0xB);
		break;
	default:
		break;
	}
}

/*
 * Sets I2S Clcok format
 *
 * @param fmt		i2s clock properties
 * @param i2s_reg	i2s regiter address
 *
 * @return		int value 0 for success, -1 in case of error
 */

int i2s_set_fmt(struct i2s_tdm_regs *i2s_reg,
                unsigned char role,
                unsigned char aud_mode,
                unsigned int fmt,
                unsigned char slot_no)
{
	unsigned int tmp = 0;
    unsigned int tmp2 = 0;
	unsigned int codec_fmt = 0;

    tmp = readl(&i2s_reg->frame_setting) & ~(FS_OFFSET_MASK | FS_IDEF_MASK | FS_ACT_LENGTH_MASK);
    tmp2 = readl(&i2s_reg->slot_setting1) & ~(SLOT_NUM_MASK);

  	switch (aud_mode) {
	case I2S_MODE:
		tmp |= FS_OFFSET_1_BIT | FS_IDEF_FRAME_SYNC | FS_ACT_LENGTH(((tmp & FRAME_LENGTH_MASK)+1)/2);
        writel(&i2s_reg->frame_setting, tmp);
        tmp2 |= SLOT_NUM(slot_no);
        writel(&i2s_reg->slot_setting1, tmp2);
		codec_fmt |= SND_SOC_DAIFMT_I2S;
		break;
	case LJ_MODE:
		tmp |= NO_FS_OFFSET | FS_IDEF_FRAME_SYNC | FS_ACT_LENGTH(((tmp & FRAME_LENGTH_MASK)+1)/2);
        writel(&i2s_reg->frame_setting, tmp);
        tmp2 |= SLOT_NUM(slot_no);
        writel(&i2s_reg->slot_setting1, tmp2);
		codec_fmt |= SND_SOC_DAIFMT_LEFT_J;
		break;
	case RJ_MODE:
		tmp |= NO_FS_OFFSET | FS_IDEF_FRAME_SYNC | FS_ACT_LENGTH(((tmp & FRAME_LENGTH_MASK)+1)/2);
        writel(&i2s_reg->frame_setting, tmp);
        tmp2 &= ~(FB_OFFSET_MASK);
        tmp2 |= SLOT_NUM(slot_no)|FB_OFFSET((((tmp & FS_ACT_LENGTH_MASK) >> 16)-((tmp2 & DATA_SIZE_MASK) >> 16)));
        writel(&i2s_reg->slot_setting1, tmp2);
		codec_fmt |= SND_SOC_DAIFMT_RIGHT_J;
		break;
    case PCM_A_MODE:
		tmp |= FS_OFFSET_1_BIT | FS_IDEF_FRAME_SYNC | FS_ACT_LENGTH(1);
        writel(&i2s_reg->frame_setting, tmp);
        tmp2 |= SLOT_NUM(slot_no);
        writel(&i2s_reg->slot_setting1, tmp2);
		codec_fmt |= SND_SOC_DAIFMT_DSP_A;
		break;
    case PCM_B_MODE:
		tmp |= NO_FS_OFFSET | FS_IDEF_FRAME_SYNC | FS_ACT_LENGTH(1);
        writel(&i2s_reg->frame_setting, tmp);
        tmp2 |= SLOT_NUM(slot_no);
        writel(&i2s_reg->slot_setting1, tmp2);
		codec_fmt |= SND_SOC_DAIFMT_DSP_B;
		break;
    case TDM_MODE:
		tmp |= NO_FS_OFFSET | FS_IDEF_FRAME_SYNC | FS_ACT_LENGTH(1);
        writel(&i2s_reg->frame_setting, tmp);
        tmp2 |= SLOT_NUM(slot_no);
        writel(&i2s_reg->slot_setting1, tmp2);
		writel(&i2s_reg->slot_setting2, 0x0f); /* enable slot 0-3 for TDM */
		codec_fmt |= SND_SOC_DAIFMT_PDM;
		break;
	default:
		debug("%s: Invalid format\n", __func__);
		return -1;
	}
	if(skip_codec_setting == false)
		adau1372_set_tdm_format(slot_no);
/*
* Default use fs_sample at falling edge
*/
    tmp=readl(&i2s_reg->blk_mode_setting) & ~(SAMPLE_EDGE_MASK | FS_SAMPLE_RX_DELAY_MASK); /* clear bit 2~4 to set frame format */
    tmp2=readl(&i2s_reg->frame_setting) & ~(FS_POLARITY_MASK); /* clear bit 12 to set fs polarity */
	if((aud_mode == I2S_MODE) || (aud_mode == LJ_MODE) || (aud_mode == RJ_MODE)) {
	switch (fmt) {
		case FMT_IB_NF:
			printf("Set format to IBNF\n");
#ifdef CONFIG_SHIFT_HALF_T
			if (concurrent_rx_enable == true)
				tmp |= RX_SAMPLE_EDGE_N | TX_SAMPLE_EDGE_P; /* for crx */
			else
				tmp |= RX_SAMPLE_EDGE_P | TX_SAMPLE_EDGE_N;
#else
			tmp |= RX_SAMPLE_EDGE_N | TX_SAMPLE_EDGE_N;
#endif
        	writel(&i2s_reg->blk_mode_setting, tmp);
        	tmp2 |= FS_ACT_LOW;
        	writel(&i2s_reg->frame_setting, tmp2);
			codec_fmt |= SND_SOC_DAIFMT_IB_NF;
			break;
		case FMT_IB_IF:
			printf("Set format to IBIF\n");
#ifdef CONFIG_SHIFT_HALF_T
			if (concurrent_rx_enable == true)
				tmp |= RX_SAMPLE_EDGE_N | TX_SAMPLE_EDGE_P; /* for crx */
			else
				tmp |= RX_SAMPLE_EDGE_P | TX_SAMPLE_EDGE_N;
#else
			tmp |= RX_SAMPLE_EDGE_N | TX_SAMPLE_EDGE_N;
#endif
        	writel(&i2s_reg->blk_mode_setting, tmp);
        	tmp2 |= FS_ACT_HIGH;
        	writel(&i2s_reg->frame_setting, tmp2);
			codec_fmt |= SND_SOC_DAIFMT_IB_IF;
			break;
		case FMT_NB_NF:
			printf("Set format to NBNF\n");
#ifdef CONFIG_SHIFT_HALF_T
			if (concurrent_rx_enable == true)
				tmp |= RX_SAMPLE_EDGE_P | TX_SAMPLE_EDGE_N; /* for crx  */
			else
				tmp |= RX_SAMPLE_EDGE_N | TX_SAMPLE_EDGE_P;
#else
			tmp |= RX_SAMPLE_EDGE_P | TX_SAMPLE_EDGE_P;
#endif
        writel(&i2s_reg->blk_mode_setting, tmp);
        tmp2 |= FS_ACT_LOW;
        writel(&i2s_reg->frame_setting, tmp2);
		codec_fmt |= SND_SOC_DAIFMT_NB_NF;
		break;
		case FMT_NB_IF:
			printf("Set format to NBIF\n");
#ifdef CONFIG_SHIFT_HALF_T
			if (concurrent_rx_enable == true)
				tmp |= RX_SAMPLE_EDGE_P | TX_SAMPLE_EDGE_N; /* for crx */
			else
				tmp |= RX_SAMPLE_EDGE_N | TX_SAMPLE_EDGE_P;
#else
			tmp |= RX_SAMPLE_EDGE_P | TX_SAMPLE_EDGE_P;
#endif
	        writel(&i2s_reg->blk_mode_setting, tmp);
			tmp2 |= FS_ACT_HIGH;
        	writel(&i2s_reg->frame_setting, tmp2);
			codec_fmt |= SND_SOC_DAIFMT_NB_IF;
			break;
		default:
			debug("%s: Invalid clock ploarity input\n", __func__);
			return -1;
		}
	} else {
#ifdef CONFIG_SHIFT_HALF_T
			if (role == MASTER_MODE)
				tmp |= RX_SAMPLE_EDGE_P | TX_SAMPLE_EDGE_N;
			else
				tmp |= RX_SAMPLE_EDGE_N | TX_SAMPLE_EDGE_N;
#else
			tmp |= RX_SAMPLE_EDGE_P | TX_SAMPLE_EDGE_P;
#endif
	        writel(&i2s_reg->blk_mode_setting, tmp);
			tmp2 |= FS_ACT_HIGH;
        	writel(&i2s_reg->frame_setting, tmp2);
			codec_fmt |= SND_SOC_DAIFMT_IB_IF;
	}

    tmp=readl(&i2s_reg->blk_mode_setting) & ~(ROLE_MASK); /* clear bit 2~4 to set frame format */
	switch (role) {
	case MASTER_MODE:
		tmp |= MASTER_MODE;
		writel(&i2s_reg->blk_mode_setting, tmp);
		i2s_set_audio_gpio(MASTER_MODE);
		codec_fmt |= SND_SOC_DAIFMT_CBS_CFS; /* Set codec to slave */
		break;
	case SLAVE_MODE:
		tmp |= SLAVE_MODE;
		writel(&i2s_reg->blk_mode_setting, tmp);
		i2s_set_audio_gpio(SLAVE_MODE);
		codec_fmt |= SND_SOC_DAIFMT_CBM_CFM; /* Set codec to master*/
		break;
	default:
		debug("%s: Invalid master selection\n", __func__);
		return -1;
	}

	debug("Set codes fmt\n");
	if (skip_codec_setting == false)
		adau1372_set_dai_fmt(codec_fmt);

	return 0;
}

int i2s_test_rx(struct i2stx_info *pi2s_tx, unsigned int resolution)
{
		struct i2s_tdm_regs *i2s_reg = pi2s_tx->base_address;
		u32	slot_setting = readl(&i2s_reg->slot_setting1);
		u32 i2s_int = 0;
	 	unsigned int i;
#ifdef	CONFIG_I2S_USE_DMA
		unsigned int *data;
		unsigned int data_size = 384;
#else
		unsigned int data_size = 0;
		unsigned int *data_L;
		unsigned int *data_R;
#endif

#ifdef	CONFIG_I2S_USE_DMA
		data=malloc(data_size);
		debug("%s, allocat data address=%p\n", __func__, data);
#else
		data_L = malloc(4); /*allocte 4* fifi_depth (3) data size*/
		data_R = malloc(4);
#endif

		printf("Start to test RX\n");

		slot_setting &= ~(SLOT_SIZE_MASK | DATA_SIZE_MASK);

		adau1372_set_loopback();

		i2s_sw_reset(i2s_reg);

		switch(resolution) {
			case (16):
				slot_setting |= SLOT_SIZE(16) | DATA_SIZE(16);
				writel(&i2s_reg->slot_setting1, slot_setting);
				break;
			case (24):
				slot_setting |= SLOT_SIZE(32) | DATA_SIZE(24);
				writel(&i2s_reg->slot_setting1, slot_setting);
				break;
			case (32):
				slot_setting |= SLOT_SIZE(32) | DATA_SIZE(32);
				writel(&i2s_reg->slot_setting1, slot_setting);
				break;
		}

		i2s_switch(I2S_ON, i2s_reg);

		i2s_rxctrl(i2s_reg, I2S_RX_ON);
#ifdef	CONFIG_I2S_USE_DMA
		dma_dev2mem_setting(data, data_size, &i2s_reg->rx_rd_port_ch0, DMA_I2S1);
#endif
		while(1) {
#ifdef	CONFIG_I2S_USE_DMA
			i2s_int = readl(&i2s_reg->i2s_int);
			if ((i2s_int & I2S_INT_RXDA_RAW) == I2S_INT_RXDA_RAW){
				dma_start_receive(DMA_I2S1);
				for (i=0; i< data_size; i+=2)
					debug("received L data=0x%X, R data=0x%X\n", *(data+i), *(data+i+1));
				break;
			}
#else
			if (data_size <= RX_BUFFER_SIZE) {
				i2s_int = readl(&i2s_reg->i2s_int);
				if ((i2s_int & I2S_INT_RXDA_RAW) == I2S_INT_RXDA_RAW) {
					*data_L=readl(&i2s_reg->rx_rd_port_ch0);
					*data_R=readl(&i2s_reg->rx_rd_port_ch0);
					data_size+=2;
					debug("Revice I2S data - L channel = 0x%X, R channel=0x%X\n", *data_L, *data_R);
					i++;
					if (i > 1000)
						break;
				}
			} else {
				debug("RX test finished\n");
				break;
			}
#endif
		}

		i2s_rxctrl(i2s_reg, I2S_RX_OFF); /* enable DW i2s IRER */
		i2s_switch(I2S_OFF, i2s_reg);

#ifdef	CONFIG_I2S_USE_DMA
		return sizeof(data_size);
#else
		free(data_L);
		free(data_R);
		debug("data_size=%d\n", data_size);
		return data_size;
#endif
}

int i2s_receive_rx_data(struct i2stx_info *pi2s_tx, unsigned int *data)
{
	struct i2s_tdm_regs *i2s_reg = pi2s_tx->base_address;
	u32 i2s_int = 0;

#ifdef I2S_PRINT_RAW_DATA
	u32 i;
	u8 out[4];
#endif

#ifndef	CONFIG_I2S_USE_DMA
//	unsigned int time_count = 0;
	unsigned int data_size = 0;
	unsigned int *data_L;
	unsigned int *data_R;
	unsigned int i = 0;

	data_L = malloc(4); /*allocte 4* fifi_depth (3) data size*/
	data_R = malloc(4);
#endif

	adau1372_set_loopback();

	printf("Start to receive data\n");

	i2s_rxctrl(i2s_reg, I2S_RX_ON);
	i2s_sw_reset(i2s_reg);
	i2s_switch(I2S_ON, i2s_reg);

#ifdef CONFIG_I2S_USE_DMA
	printf("Setup DMA configuration for RX\n");
	if (rxdma_allocated == 0) {
		dma_dev2mem_setting(data, RX_BUFFER_SIZE, &i2s_reg->rx_rd_port_ch0, DMA_I2S1);
		rxdma_allocated = 1;
	}
#endif
	while(1) {
#ifdef	CONFIG_I2S_USE_DMA
		i2s_int = readl(&i2s_reg->i2s_int);
		if ((i2s_int & I2S_INT_RXDA_RAW) == I2S_INT_RXDA_RAW) {
			dma_start_receive(DMA_I2S1);
			printf("data[0]=0x%08x, data[1]=0x%08x, data[2]=0x%08x, data[3]=0x%08x\n", *data, *(data+1), *(data+2), *(data+3));
			printf("Record finished\n");
			break;
		}
#else
		if (data_size <= RX_BUFFER_SIZE) {
			i2s_int = readl(&i2s_reg->i2s_int);
			if (i2s_int & I2S_INT_RXDA_RAW)
			{
				*data_L=readl(&i2s_reg->rx_rd_port_ch0);
				*data=*data_L;
				data++;
				*data_R=readl(&i2s_reg->rx_rd_port_ch0);
				*data=*data_R;
				data++;
				data_size+=2;
				if (i<3){
					i++;
					debug("Revice I2S data - L channel = 0x%X, R channel=0x%X\n", *data_L, *data_R);
				}
			}
		} else {
			printf("Record finished\n");
			break;
		}
#endif
	}

	//i2s_rxctrl(i2s_reg, I2S_RX_OFF); /* enable DW i2s IRER */
	i2s_switch(I2S_OFF, i2s_reg);

#ifdef I2S_PRINT_RAW_DATA
	for (i=0; i < PRINT_COUNT; i++){
		if ((i % 16 == 0) && (i != 0))
			printf("\n");
		memcpy(out,  &data[i], sizeof(u32));
		printf("%02x%02x%02x%02x ", out[0], out[1], out[2], out[3]);

	}
#endif

#ifdef	CONFIG_I2S_USE_DMA
	return RX_BUFFER_SIZE;
#else
	free(data_L);
	free(data_R);
	debug("data_size=%d\n", data_size);
	return data_size;
#endif
}

int i2s_transfer_tx_data(struct i2stx_info *pi2s_tx, unsigned int *data,
				unsigned long data_size)
{
#ifndef	CONFIG_I2S_USE_DMA
	int i;
#endif
	struct i2s_tdm_regs *i2s_reg = pi2s_tx->base_address;
	int fifo_depth = (readl(&i2s_reg->fifo_threshold) & TX_FIFO_THRESHOLD_MASK) >> 16;

	debug("%s, i2s_reg base=%p, fifo_depth=%d,\n", __func__, i2s_reg, fifo_depth);

	if (data_size < fifo_depth) {
		debug("%s : Invalid data size\n", __func__);
		return -1; /* invalid pcm data size */
	}

	/* fill the tx buffer before stating the tx transmit */

	debug("enable TX\n");
	i2s_txctrl(i2s_reg, I2S_TX_ON);

	i2s_sw_reset(i2s_reg);
	i2s_switch(I2S_ON, i2s_reg);


#ifdef	CONFIG_I2S_USE_DMA
	debug("Setup DMA configuration\n");
	//i2s_txctrl(i2s_reg, I2S_TX_ON); //enable iter before tranmit data to DMA
	if (txdma_allocated == 0) {
		dma_mem2dev_setting(data, data_size, &i2s_reg->tx_wr_port_ch0, DMA_I2S1);
		txdma_allocated = 1;
	}
	dma_start_transfer(DMA_I2S1);

#else
	debug("Use TX FIFO to transmit\n");
	debug("fifo_depth=%d\n", fifo_depth);
	for (i = 0; i < fifo_depth; i++) {
		debug("write L_data = 0x%X, R data=0x%X\n", *data, *(data+1));
		writel(&i2s_reg->tx_wr_port_ch0, *data);
		data++;
		writel(&i2s_reg->tx_wr_port_ch0, *data);
		data++;
		data_size -= 2;
	}


#if 1  /* remark for only send short data to verify*/
	while (data_size > 0) {
		u32 overrun;
		timer_meter_start();
		overrun = readl(&i2s_reg->i2s_int);
		if (!((overrun & I2S_INT_TXFO_RAW) == I2S_INT_TXFO_RAW)) {
			writel(&i2s_reg->tx_wr_port_ch0, *data);
			data++;
			writel(&i2s_reg->tx_wr_port_ch0, *data);
			data++;
			data_size -=2;
		} else {
			if (timer_meter_get_ms() > I2S_TIMEOUT) {
				i2s_txctrl(i2s_reg, I2S_TX_OFF);
				printf("%s: I2S Transfer Timeout\n", __func__);
				return -1;
			}
		}
	}
#endif
#endif /* #ifdef use DMA*/
	//i2s_txctrl(i2s_reg, I2S_TX_OFF);
	i2s_switch(I2S_OFF, i2s_reg);

	return 0;
}


static void i2s_subsys_int(unsigned int i2s_no)
{
	struct i2s_sys_regs *i2s_sys_reg = (struct i2s_sys_regs *)CONFIG_SYS_I2S_SYS_BASE;
	u32 bclk_oen_sel = readl(&i2s_sys_reg->i2s_bclk_oen_sel);

	bclk_oen_sel |= 1 << i2s_no;

	writel(&i2s_sys_reg->i2s_bclk_oen_sel, bclk_oen_sel);
}


void i2s_loop_test(struct i2stx_info *pi2s_tx, unsigned int sec, unsigned int *data)
{
	struct i2s_tdm_regs *i2s0_reg = (struct i2s_tdm_regs *)CONFIG_SYS_I2S0_BASE;
	struct i2s_tdm_regs *i2s1_reg = (struct i2s_tdm_regs *)CONFIG_SYS_I2S1_BASE;
	struct i2s_sys_regs *i2s_sys_reg = (struct i2s_sys_regs *)CONFIG_SYS_I2S_SYS_BASE;
	u32 i2s1_int = 0;
	u32 c_time = 0;
	u8 rx_allocated = 0;
//	u8 round=0;
#if 1
	u8 tx_allocated = 0;
#endif
#ifdef I2S_PRINT_RAW_DATA
	u32 i;
	u8 out[4];
#endif


	printf("Start loop test for %d secs\n", sec);
/* setup I2S0 and I2S1 parameters */
	/* I2S1 for RX */
	i2s_set_fmt(i2s1_reg, MASTER_MODE, I2S_MODE, FMT_IB_NF, 2);
	//i2s_set_ws_clock_cycle(i2s1_reg, WSS_32_CLKCYCLE, I2S_MODE);
	i2s_set_ws_clock_cycle(i2s1_reg, WSS_16_CLKCYCLE, I2S_MODE);

	skip_codec_setting = true; /* skip codec setting due to already set it with I2S0*/
#if 1
	/* I2S0 for TX */
	i2s_set_interrupt(i2s0_reg);
	i2s_set_sample_rate(i2s0_reg, pi2s_tx->samplingrate);
	i2s_set_fmt(i2s0_reg, SLAVE_MODE, I2S_MODE, FMT_IB_NF, 2);
	i2s_config_dma(i2s0_reg, true);
	//i2s_set_ws_clock_cycle(i2s0_reg, WSS_32_CLKCYCLE, I2S_MODE);
	i2s_set_ws_clock_cycle(i2s0_reg, WSS_16_CLKCYCLE, I2S_MODE);
#endif
/* setup I2S subsystem parameters */

	i2s_sys_reg->i2s_tdm_sclk_in_sel = 0x11; /* BCLK from I2S1 as master, I2S0 BCLK form I2S0 */
	i2s_sys_reg->i2s_tdm_fs_in_sel = 0x11; /* LRCK from I2S1 as master, I2S0 LRCK from I2S0 */
	i2s_sys_reg->i2s_tdm_sdi_in_sel = 0x50; /* I2S1 SDI from codec, I2S0 SDI from N/A */
	i2s_sys_reg->i2s_tdm_sdo_out_sel = 0x40; /* select i2s_tdm0 sdo to IO SDO_i2s1 */
	i2s_sys_reg->i2s_tdm_multi_sync = 0x33; /* I2S0 sync with I2S1 */

	adau1372_set_loopback();
	dma_turn_on();
	dma_mem2dev_setting(data, RX_BUFFER_SIZE, &i2s0_reg->tx_wr_port_ch0, DMA_I2S0);

	timer_meter_start();

	while (c_time < (sec * 500)) {

	/* RX from I2S1 */
		if (rx_allocated == 0) {
			printf("Start to receive audio 2\n");
			i2s_rxctrl(i2s1_reg, I2S_RX_ON);
			i2s_sw_reset(i2s1_reg);
			i2s_switch(I2S_ON, i2s1_reg);
			udelay(10);
			rx_allocated = 1;
		}

	/* TX to I2S0 */
#if 1
		if (tx_allocated == 0) {
			printf("Start to play\n");
			i2s_txctrl(i2s0_reg, I2S_TX_ON);
			i2s_sw_reset(i2s0_reg);
			i2s_switch(I2S_ON, i2s0_reg);
			//udelay(10);
			tx_allocated = 1;
		}
#endif
		i2s1_int = readl(&i2s1_reg->i2s_int);
		if ((i2s1_int & I2S_INT_RXDA_RAW) == I2S_INT_RXDA_RAW){
#if 0
			dma_start_receive(DMA_I2S1);
#else
			dma_start_txrx();
#endif
		}

		c_time = timer_meter_get_ms();

	}
	i2s_switch(I2S_OFF, i2s1_reg);
	i2s_switch(I2S_OFF, i2s0_reg);


/* reset subsystem registers in order to do other basic test */
	i2s_sys_reg->i2s_tdm_sclk_in_sel = 0x7654; /* BCLK from I2S0 as master, I2S1 BCLK form I2S0 */
	i2s_sys_reg->i2s_tdm_fs_in_sel = 0x7654; /* LRCK from I2S0 as master, I2S1 LRCK from I2S0 */
	i2s_sys_reg->i2s_tdm_sdi_in_sel = 0x7654; /* I2S0 SDI from codec, I2S1 SDI from N/A */
	i2s_sys_reg->i2s_tdm_sdo_out_sel = 0x7654; /* select i2s_tdm1 sdo to IO SDO_i2s0 */
	i2s_sys_reg->i2s_tdm_multi_sync = 0x00; /* I2S0 sync with I2S1 */

	txdma_allocated = 0; /* reconfigure tx llp for I2S0 */

#ifdef I2S_PRINT_RAW_DATA
	for (i=0; i < PRINT_COUNT; i++){
		if ((i % 16 == 0) && (i != 0))
			printf("\n");
		memcpy(out,  &data[i], sizeof(u32));
		printf("%02x%02x%02x%02x ", out[0], out[1], out[2], out[3]);

	}
#endif

	printf("loop test finish\n");

}


void i2s_pdm_loop_test(struct i2stx_info *pi2s_tx, unsigned int sec, unsigned int *data)
{
	struct i2s_tdm_regs *i2s1_reg = (struct i2s_tdm_regs *)CONFIG_SYS_I2S1_BASE;
	struct i2s_tdm_regs *i2s2_reg = (struct i2s_tdm_regs *)CONFIG_SYS_I2S2_BASE;
	struct i2s_sys_regs *i2s_sys_reg = (struct i2s_sys_regs *)CONFIG_SYS_I2S_SYS_BASE;
	struct reg_audio_misc_regs *reg_audio_misc = (struct reg_audio_misc_regs *)REG_AUDIO_MISC_BASE;
	u32 i2s1_int = 0;
	u32 c_time = 0;
	u8 rx_allocated = 0;
	u8 tx_allocated = 0;
#ifdef I2S_PRINT_RAW_DATA
	u32 i;
	u8 out[4];
#endif

	printf("Start loop test for %d secs\n", sec);
/* setup I2S0 and I2S1 parameters */
	/* I2S1 for RX */
	i2s_set_fmt(i2s1_reg, SLAVE_MODE, I2S_MODE, FMT_IB_NF, 2);
	i2s_set_ws_clock_cycle(i2s1_reg, WSS_16_CLKCYCLE, I2S_MODE);
	writel(&i2s1_reg->i2s_clk_ctrl0, (readl(&i2s1_reg->i2s_clk_ctrl0) | ADU_BCLK_OUT_EN | AUD_MCLK_OUT_EN));
	writel(&i2s1_reg->data_format, WORD_LEN_16);
	skip_codec_setting = true; /* skip codec setting due to already set it with I2S1*/

	/* I2S2 for TX */
	i2s_set_interrupt(i2s2_reg);
	i2s_set_sample_rate(i2s2_reg, pi2s_tx->samplingrate);
	i2s_set_fmt(i2s2_reg, MASTER_MODE, I2S_MODE, FMT_IB_NF, 2);
	i2s_config_dma(i2s2_reg, true);
	writel(&i2s2_reg->data_format, WORD_LEN_16);

/* setup I2S subsystem parameters */

	i2s_sys_reg->i2s_tdm_sclk_in_sel = 0x210; /* BCLK from internal audio pdm, I2S2 as master to generate BCLK */
	i2s_sys_reg->i2s_tdm_fs_in_sel = 0x210; /* LRCK from audio pdm, I2S2 as master to generate LRCK */
	i2s_sys_reg->i2s_tdm_sdi_in_sel = 0x10; /* I2S1 SDI from pdm */
	i2s_sys_reg->i2s_tdm_sdo_out_sel = 0x600; /* select i2s_tdm2 sdo to IO SDO_i2s2 */
	i2s_sys_reg->i2s_bclk_oen_sel = 0x404; /* IO oen of bclk_out_i2s1 should be controlled by i2s1_bclk_oen_ext */
	//i2s_sys_reg->i2s_tdm_multi_sync = 0x33; /* I2S0 sync with I2S1 */
	i2s_sys_reg->audio_pdm_ctrl = 0x02; /* select I2S1 IO & i2s_tdm_1 IP */

	reg_audio_misc->reg_pdm_clk = 0x00020003; // set pdm clock to 48K
	reg_audio_misc->reg_pdm_i2s = 0x00000107; // set I2S bclk to 48K

	adau1372_set_loopback();
	dma_turn_on();
	dma_mem2dev_setting(data, RX_BUFFER_SIZE, &i2s2_reg->tx_wr_port_ch0, DMA_I2S2);

	timer_meter_start();

	while (c_time < (sec * 500)) {
			reg_audio_misc->reg_pdm_en = 0x1; //turn pdm
	/* RX from I2S1 */
		if (rx_allocated == 0) {
			printf("Start to receive audio from PDM\n");
			reg_audio_misc->reg_pdm_en = 0x1; //turn pdm
			i2s_rxctrl(i2s1_reg, I2S_RX_ON);
			i2s_sw_reset(i2s1_reg);
			i2s_switch(I2S_ON, i2s1_reg);
			udelay(10);
			rx_allocated = 1;
		}

	/* TX to I2S0 */
#if 1
		if (tx_allocated == 0) {
			printf("Start to play\n");
			i2s_txctrl(i2s2_reg, I2S_TX_ON);
			i2s_sw_reset(i2s2_reg);
			i2s_switch(I2S_ON, i2s2_reg);
			//udelay(10);
			tx_allocated = 1;
		}
#endif
		i2s1_int = readl(&i2s1_reg->i2s_int);
		if ((i2s1_int & I2S_INT_RXDA_RAW) == I2S_INT_RXDA_RAW){
		//	printf("round =%d\n", round);
			//dma_start_receive(DMA_I2S1);
			dma_start_txrx_from_pdm();
		//	round++;
			//if(round==2)
			//	break;
		}

		c_time = timer_meter_get_ms();
	}
	reg_audio_misc->reg_pdm_en = 0x0; //turn off pdm
	i2s_switch(I2S_OFF, i2s1_reg);
	i2s_switch(I2S_OFF, i2s2_reg);

#ifdef I2S_PRINT_RAW_DATA
	for (i=0; i < PRINT_COUNT; i++){
		if ((i % 16 == 0) && (i != 0))
			printf("\n");
		memcpy(out,  &data[i], sizeof(u32));
		printf("%02x%02x%02x%02x ", out[0], out[1], out[2], out[3]);

	}
#endif

/* reset subsystem registers in order to do other basic test */
	i2s_sys_reg->i2s_tdm_sclk_in_sel = 0x7654; /* BCLK from I2S0 as master, I2S1 BCLK form I2S0 */
	i2s_sys_reg->i2s_tdm_fs_in_sel = 0x7654; /* LRCK from I2S0 as master, I2S1 LRCK from I2S0 */
	i2s_sys_reg->i2s_tdm_sdi_in_sel = 0x7654; /* I2S0 SDI from codec, I2S1 SDI from N/A */
	i2s_sys_reg->i2s_tdm_sdo_out_sel = 0x7654; /* select i2s_tdm1 sdo to IO SDO_i2s0 */
	i2s_sys_reg->i2s_tdm_multi_sync = 0x00; /* I2S0 sync with I2S1 */

	txdma_allocated = 0; /* reconfigure tx llp for I2S0 */

	printf("PDM loop test finish\n");

}


void i2s_src_test(struct i2stx_info *pi2s_tx)
{
	struct i2s_tdm_regs *i2s1_reg = (struct i2s_tdm_regs *)CONFIG_SYS_I2S1_BASE;
	struct reg_audio_src_regs *aud_src_reg = (struct reg_audio_src_regs *)REG_AUDIO_SRC_BASE;
	struct i2s_sys_regs *i2s_sys_reg = (struct i2s_sys_regs *)CONFIG_SYS_I2S_SYS_BASE;
#ifdef I2S_PRINT_RAW_DATA
	int i;
#endif

	printf("Start sample rate convert, src_rx_data addr=%p\n", src_rx_data);
/* setup I2S0 and I2S1 parameters */
	/* AUDSRC for RX */
	aud_src_reg->reg_src_ratio = AUD_SRC_FSI(441) | AUD_SRC_FSO(320); /* 11.025K to 16K */
	aud_src_reg->reg_src_sel = 0x50; /* select I2S1 tdm as input */
	dma_dev2mem_setting((unsigned int* )src_rx_data, SRC_RX_BUFF_SIZE, &aud_src_reg->reg_src_data_out, DMA_AUDSRC);

	/* I2S1 for TX */
	i2s_set_interrupt(i2s1_reg);
	i2s_set_sample_rate(i2s1_reg, 16000);
	i2s_set_fmt(i2s1_reg, MASTER_MODE, I2S_MODE, FMT_IB_NF, 2);
	i2s_set_ws_clock_cycle(i2s1_reg, WSS_16_CLKCYCLE, I2S_MODE);

	writel(&i2s1_reg->data_format, WORD_LEN_16);
	i2s_config_dma(i2s1_reg, true);

	dma_turn_on();
	dma_mem2dev_setting((unsigned int* )src_tx_data, SRC_BUFF_SIZE, &i2s1_reg->tx_wr_port_ch0, DMA_I2S1);

	i2s_sys_reg->i2s_bclk_oen_sel = 0x002; /* disable i2s1 bclk output */


	aud_src_reg->reg_src_en &= 0x100; /* claer src FSM & fifo */
	aud_src_reg->reg_src_en = AUD_SRC_EN;
	/* TX to I2S1 */
	printf("Start to do resample\n");
	i2s_txctrl(i2s1_reg, I2S_TX_ON);
	i2s_sw_reset(i2s1_reg);
	i2s_switch(I2S_ON, i2s1_reg);
		//udelay(10);


	dma_start_src();

	i2s_switch(I2S_OFF, i2s1_reg);
	aud_src_reg->reg_src_en = AUD_SRC_OFF;; /* claer src FSM & fifo */


/* reset subsystem registers in order to do other basic test */

	i2s_sys_reg->i2s_bclk_oen_sel = 0x0;
	txdma_allocated = 0; /* reconfigure tx llp*/
	rxdma_allocated = 0;

#ifdef I2S_PRINT_RAW_DATA
	printf("src test finish, got data below. original size=%ld \n\n", sizeof(src_tx_data));
	for (i=0; i < PRINT_COUNT; i++){
		if ((i % 16 == 0) && (i != 0))
			printf("\n");
		printf("%02x ", src_rx_data[i]);

	}
	printf("\ndump %d bytes finish!!\n", i);
#else
	printf("src test finish\n");
#endif

}


#define TEMP_BUF_SIZE 128
void concurrent_rx(void){
	struct i2s_tdm_regs *i2s1_reg = (struct i2s_tdm_regs *)CONFIG_SYS_I2S1_BASE;
	struct i2s_tdm_regs *i2s2_reg = (struct i2s_tdm_regs *)CONFIG_SYS_I2S2_BASE;
	struct i2s_sys_regs *i2s_sys_reg = (struct i2s_sys_regs *)CONFIG_SYS_I2S_SYS_BASE;
	unsigned int rx_buf1[TEMP_BUF_SIZE]={0}, rx_buf2[TEMP_BUF_SIZE]={0};
//	u32 i2s1_int = 0;
	u16 idx =0;
	u16 err =0;
	printf("Concurrent rx test\n");
	concurrent_rx_enable = true;

	dma_reset();
	concurrent_RX_dma_dev2mem_setting(rx_buf1, rx_buf2, TEMP_BUF_SIZE,
									  &i2s1_reg->rx_rd_port_ch0, &i2s2_reg->rx_rd_port_ch0);
/* setup I2S1 and I2S2 parameters */
	/* I2S1 for RX */
	i2s_subsys_int(I2S1);
	writel(&i2s1_reg->blk_mode_setting, readl(&i2s1_reg->blk_mode_setting) | MULTI_I2S_SYNC);
	i2s_set_fmt(i2s1_reg, MASTER_MODE, I2S_MODE, FMT_IB_NF, 2);
	i2s_set_ws_clock_cycle(i2s1_reg, WSS_32_CLKCYCLE, I2S_MODE);

	skip_codec_setting = true; /* skip codec setting due to already set it with I2S0*/

	/* I2S2 for RX */
	i2s_subsys_int(I2S2);
	writel(&i2s2_reg->blk_mode_setting, readl(&i2s2_reg->blk_mode_setting) | MULTI_I2S_SYNC);
	i2s_set_interrupt(i2s2_reg);
	i2s_set_sample_rate(i2s2_reg, 48000);
	i2s_set_fmt(i2s2_reg, SLAVE_MODE, I2S_MODE, FMT_IB_NF, 2);
	//writel(&i2s2_reg->blk_mode_setting, readl(&i2s2_reg->blk_mode_setting) & ~ROLE_MASK);
	i2s_config_dma(i2s2_reg, true);
	//i2s_set_ws_clock_cycle(i2s1_reg, WSS_32_CLKCYCLE, I2S_MODE);

	i2s_sys_reg->i2s_tdm_sclk_in_sel = 0x110; /* BCLK from I2S1 as master, I2S2 BCLK form I2S1 */
	i2s_sys_reg->i2s_tdm_fs_in_sel = 0x110; /* LRCK from I2S1 as master, I2S2 LRCK from I2S1 */
	i2s_sys_reg->i2s_tdm_sdi_in_sel = 0x560; /* I2S1 SDI from codec, I2S2 SDI from codec */
	i2s_sys_reg->i2s_tdm_sdo_out_sel = 0x000; /* select i2s_tdm0 sdo to IO SDO_i2s1 */
	i2s_sys_reg->i2s_tdm_multi_sync = 0x660; /* I2S2 sync with I2S1 */

	adau1372_set_loopback();

	i2s_subsys_bclk_oen_switch(I2S_ON, I2S1);

	i2s_subsys_bclk_oen_switch(I2S_ON, I2S2);

	dma_turn_on();

	/* RX from I2S1 */
	printf("Start to receive audio 1\n");
	i2s_rxctrl(i2s1_reg, I2S_RX_ON);
	i2s_sw_reset(i2s1_reg);
	i2s_rxctrl(i2s2_reg, I2S_RX_ON);
	i2s_sw_reset(i2s2_reg);

	i2s_switch(I2S_ON, i2s1_reg);
	//mdelay(100); //use this delay to make sure I2S1 and I2S2 are really synchonized. just for stress test
	i2s_switch(I2S_ON, i2s2_reg);


//	i2s1_int = readl(&i2s1_reg->i2s_int);
//	printf("i2s1 init=0x%08x\n",i2s1_int);
	//if ((i2s1_int & I2S_INT_RXDA_RAW) == I2S_INT_RXDA_RAW) {
	dma_start_conurrent_rx();
	//}
	dma_turn_off();

	i2s_subsys_bclk_oen_switch(I2S_OFF, I2S1);
	i2s_subsys_bclk_oen_switch(I2S_OFF, I2S2);
	writel(&i2s1_reg->blk_mode_setting, readl(&i2s1_reg->blk_mode_setting) & ~MULTI_I2S_MODE_MASK);
	writel(&i2s1_reg->blk_mode_setting, readl(&i2s1_reg->blk_mode_setting) & ~MULTI_I2S_MODE_MASK);
	i2s_switch(I2S_OFF, i2s1_reg);
	i2s_switch(I2S_OFF, i2s2_reg);

	printf("rx_buf1 addr=0x%p, rx_buff2 add=0x%p\n", rx_buf1, rx_buf2);
	for (idx=0; idx < TEMP_BUF_SIZE; idx++)
		printf("rx_buf1[%d]=0x%08x, rx_buf2[%d]=0x%08x\n", idx, *(rx_buf1+idx), idx, *(rx_buf2+idx));

	for (idx=0; idx < TEMP_BUF_SIZE; idx++){
		if (*(rx_buf1+idx) !=  *(rx_buf2+idx)) {
			printf("\n\nrxbuf1[%d]=0x%08x rxbuf2[%d]=0x%08x is not the same\n", idx, *(rx_buf1+idx), idx, *(rx_buf2+idx));
			err++;
		}
	}
	printf("compare result of rx_buf1 and rx_buf2 is %s, failed rate=%d/%d\n", err?"failed":"passed", err, TEMP_BUF_SIZE);
/* reset subsystem registers in order to do other basic test */
	i2s_sys_reg->i2s_tdm_sclk_in_sel = 0x7654; /* BCLK from I2S0 as master, I2S1 BCLK form I2S0 */
	i2s_sys_reg->i2s_tdm_fs_in_sel = 0x7654; /* LRCK from I2S0 as master, I2S1 LRCK from I2S0 */
	i2s_sys_reg->i2s_tdm_sdi_in_sel = 0x7654; /* I2S0 SDI from codec, I2S1 SDI from N/A */
	i2s_sys_reg->i2s_tdm_sdo_out_sel = 0x7654; /* select i2s_tdm1 sdo to IO SDO_i2s0 */
	i2s_sys_reg->i2s_tdm_multi_sync = 0x00; /* I2S0 sync with I2S1 */
	i2s_sys_reg->i2s_bclk_oen_sel = 0x00;

	concurrent_rx_enable = false;

	printf("concurrent test finish\n");

}
int i2s_init(struct i2stx_info *pi2s_tx, unsigned int *data)
{
	int ret;
	struct i2s_tdm_regs *i2s_reg =
				(struct i2s_tdm_regs *)pi2s_tx->base_address;

	printf("%s, i2s_reg=%p, base addr=%p, record_data addr=%p\n", __func__, i2s_reg, pi2s_tx->base_address, data);

	i2s_set_clk_source(i2s_reg, pi2s_tx->clk_src);

	i2s_set_interrupt(i2s_reg);
	/* disable i2s transfer flag and flush the fifo */
	i2s_set_sample_rate(i2s_reg, pi2s_tx->samplingrate); /* sample rate must first prior to fmt */

	ret = i2s_set_fmt(i2s_reg, pi2s_tx->role, pi2s_tx->aud_mode, pi2s_tx->inv, pi2s_tx->slot_no);

	if (ret != 0) {
		debug("%s:set format failed\n", __func__);
		return -1;
	}

	//i2s_txctrl(i2s_reg, I2S_TX_ON);

#ifdef	CONFIG_I2S_USE_DMA
	i2s_config_dma(i2s_reg, true);
	printf("RX_BUFFER_SIZE=%d\n", RX_BUFFER_SIZE);
	dma_reset();

	dma_dev2mem_setting(data, RX_BUFFER_SIZE, &i2s_reg->rx_rd_port_ch0, DMA_I2S1);

	rxdma_allocated = 1;
#else
	i2s_config_dma(i2s_reg, false);
#endif
	printf("%s end\n", __func__);
	return ret;
}