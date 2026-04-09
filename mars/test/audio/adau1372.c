/*
 * Copyright (C) 2018 bitmain
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
#include <common.h>
#include "system_common.h"
#include "bm_dw_i2c.h"
#include "bm_i2s_tdm.h"
#include "timer.h"
#include "adau1372.h"


struct i2c_adapter *adap=NULL;
struct adau1372 g_adau1372;


static int reg_update_bits(unsigned int reg, unsigned int mask, unsigned int val)
{
	unsigned int byte = 0;
	unsigned int old_data = 0;

    adap->read(adap, ADAU1372_ADDR, reg, 2, (uint8_t *)&old_data, 1);

	byte = (old_data &(~mask))|(mask & val);

	debug("%s, reg=0x%02X, old_data=0x%02X, mask=0x%02X, val=0x%02X, byte=0x%02X\n", __func__, reg, old_data, mask, val, byte);

    adap->write(adap, ADAU1372_ADDR, reg, 2, (uint8_t *)&byte, 1);

	return 0;
}


int adau1372_set_dai_fmt(unsigned int fmt)
{
	unsigned int sai0 = 0, sai1 = 0;
	bool invert_lrclk = false;

	switch (fmt & SND_SOC_DAIFMT_MASTER_MASK) {
	case SND_SOC_DAIFMT_CBM_CFM:
		sai1 |= ADAU1372_SAI1_MS; /* Set ADAU1372 to Master mode */
		break;
	case SND_SOC_DAIFMT_CBS_CFS: /* Set ADAU1372 to Slave mode */
		break;
	default:
		return -EINVAL;
	}


	/* 0: Data changes on falling edge. 1: Data changes on rising edge.*/
	switch (fmt & SND_SOC_DAIFMT_INV_MASK) {
	case SND_SOC_DAIFMT_NB_NF:
		invert_lrclk = false;
		sai1 |= ADAU1372_SAI1_BCLKEDGE; /*Issue may be here, need double check*/
		break;
	case SND_SOC_DAIFMT_NB_IF:
		invert_lrclk = true;
		sai1 |= ADAU1372_SAI1_BCLKEDGE;
		break;
	case SND_SOC_DAIFMT_IB_NF:
		invert_lrclk = false;
		break;
	case SND_SOC_DAIFMT_IB_IF:
		invert_lrclk = true;
		break;
	}

	switch (fmt & SND_SOC_DAIFMT_FORMAT_MASK) {
	case SND_SOC_DAIFMT_I2S:
		sai0 |= ADAU1372_SAI0_DELAY1;
		break;
	case SND_SOC_DAIFMT_LEFT_J:
		sai0 |= ADAU1372_SAI0_DELAY0;
		//invert_lrclk = !invert_lrclk; /*Issue may be here, need double check*/
		break;
	case SND_SOC_DAIFMT_RIGHT_J:
		sai0 |= ADAU1372_SAI0_RIGHT_J_24;
		//invert_lrclk = !invert_lrclk; /*Issue may be here, need double check*/
		break;
	case SND_SOC_DAIFMT_DSP_A:
		sai0 |= ADAU1372_SAI0_DELAY1;
		sai1 |= ADAU1372_SAI1_LR_MODE;
		break;
	case SND_SOC_DAIFMT_DSP_B:
		sai0 |= ADAU1372_SAI0_DELAY0;
		sai1 |= ADAU1372_SAI1_LR_MODE;
		break;
	case SND_SOC_DAIFMT_PDM:
		sai0 |= ADAU1372_SAI0_DELAY1;
		sai1 |= ADAU1372_SAI1_LR_MODE;
		break;
	}

	if (invert_lrclk)
		sai1 |= ADAU1372_SAI1_LR_POL;

	reg_update_bits(ADAU1372_REG_SAI0, ADAU1372_SAI0_DELAY_MASK, sai0);
	reg_update_bits(ADAU1372_REG_SAI1,ADAU1372_SAI1_MS | ADAU1372_SAI1_BCLKEDGE |
		ADAU1372_SAI1_LR_MODE | ADAU1372_SAI1_LR_POL, sai1);

	return 0;
}


void adau1372_set_tdm_format(unsigned int slot_no)
{
	unsigned int sai0 = 0, sout_ctl = 0;

	switch (slot_no) {
	case 2:
		sai0 |= ADAU1372_SAI0_SAI_TDM2;
		sout_ctl = 0xFC;
		break;
	case 4:
		sai0 |= ADAU1372_SAI0_SAI_TDM4;
		sout_ctl = 0xF0;

		break;
	case 8:
		sai0 |= ADAU1372_SAI0_SAI_TDM8;
		sout_ctl = 0x00;

		break;
	default:
		printf("No support of this TDM slot number\n");
	}

	reg_update_bits(ADAU1372_REG_SAI0, ADAU1372_SAI0_SAI_MASK, sai0);
	reg_update_bits(ADAU1372_REG_SOUT_CTRL, ADAU1372_SOUT_CTL_MASK, sout_ctl);

}

int adau1372_hw_params(unsigned int rate, unsigned int slot_width)
{
    /* rate should be the samplingrate set in bm_sound_i2s.c*/
	unsigned int sai0, sai1;
	unsigned int i;
	if (rate != 0) {
		for (i = 0; i < ARRAY_SIZE(adau1372_rates); i++) {
			if (rate == adau1372_rates[i])
				break;
		}

		if (i == ARRAY_SIZE(adau1372_rates))
			return -EINVAL;

		sai0 = i;
		g_adau1372.rate = rate;
		reg_update_bits(ADAU1372_REG_SAI0,ADAU1372_SAI0_FS_MASK, sai0);
	}

	g_adau1372.slot_width = slot_width;

    if (slot_width !=0){
		switch (slot_width) {
			case WSS_16_CLKCYCLE:
				sai1 = ADAU1372_SAI1_BCLKRATE;
				break;
			case WSS_32_CLKCYCLE:
				sai1 = 0;
				break;
			default:
		    	return -EINVAL;
		}
		reg_update_bits(ADAU1372_REG_SAI1,ADAU1372_SAI1_BCLKRATE, sai1);
	}

	return 0;
}

int adau1372_update_ws_params(unsigned int aud_mode, unsigned int slot_width)
{
    /* rate should be the samplingrate set in bm_sound_i2s.c*/
	unsigned int sai1 = 0;
	unsigned int sai0 = 0;

	switch (slot_width) {
		case WSS_16_CLKCYCLE:
			//if (aud_mode == TDM_MODE)
			sai1 |= ADAU1372_SAI1_TDMC_16;
			sai1 |= ADAU1372_SAI1_BCLKRATE_16;
			if (aud_mode == RJ_MODE)
				sai0 |= ADAU1372_SAI0_RIGHT_J_16;
			break;
		case WSS_32_CLKCYCLE:
			//if (aud_mode == TDM_MODE)
			sai1 |= ADAU1372_SAI1_TDMC_24;
			sai1 |= ADAU1372_SAI1_BCLKRATE_32;
			if (aud_mode == RJ_MODE)
				sai0 |= ADAU1372_SAI0_RIGHT_J_24;
			break;
		default:
		    return -EINVAL;
	}

	if (aud_mode == RJ_MODE)
		reg_update_bits(ADAU1372_REG_SAI0,ADAU1372_SAI0_DELAY_MASK, sai0);

	reg_update_bits(ADAU1372_REG_SAI1,ADAU1372_SAI1_BCLKRATE | ADAU1372_SAI1_BCLK_TDMC, sai1);

	return 0;
}

void set_loopback()
{
	uchar byte = 0;
	dw_i2c_get_adap((uchar **)&adap);

	if(adap) {

		printf("Set adau1372 to loopback mode\n");
		byte=0x88;
		adap->write(adap, ADAU1372_ADDR, ADAU1372_REG_SOUT_SOURCE_0_1, 2, &byte, 1);
	}
}

/* Set Audio codec ADAU1372 to loopback mode without TDM output disable*/
int adau1372_set_loopback_without_tdm_enable(void)
{
	int ret = 0;

	uchar byte = 0;

	uchar dev = ADAU1372_ADDR;

	debug("%s\n", __func__);

	dw_i2c_get_adap((uchar **)&adap);

	if ((adap)&& (g_adau1372.loopback == false)) {
		debug("start to set adau1372 wtih loopback mode\n");
		g_adau1372.loopback = true;

		byte=0x03;
		adap->write(adap, dev, ADAU1372_REG_CLK_CTRL, 2, &byte, 1);

		udelay(5000); //delay 5ms

#if defined(CONFIG_USE_AUDIO_PLL)
		byte=0x07; /* Set Clk output off */
#else
		byte=0x01;
#endif
		adap->write(adap, dev, ADAU1372_REG_CLKOUT_SEL, 2, &byte, 1); /* Set Clk output on*/

		byte=0xDC;
		adap->write(adap, dev, ADAU1372_REG_DAC_SOURCE, 2, &byte, 1);

		byte=0x76;
		adap->write(adap, dev, ADAU1372_REG_SOUT_SOURCE_0_1, 2, &byte, 1);

		byte=0x44;
		adap->write(adap, dev, ADAU1372_REG_SOUT_SOURCE_2_3, 2, &byte, 1);

		byte=0x44;
		adap->write(adap, dev, ADAU1372_REG_SOUT_SOURCE_4_5, 2, &byte, 1);

		byte=0x44;
		adap->write(adap, dev, ADAU1372_REG_SOUT_SOURCE_6_7, 2, &byte, 1);

		byte=0x04;
		adap->write(adap, dev, ADAU1372_REG_ADC_SDATA_CH, 2, &byte, 1);

		byte=0x54;
		adap->write(adap, dev, ADAU1372_REG_ASRCO_SOURCE_0_1, 2, &byte, 1);

		byte=0x76;
		adap->write(adap, dev, ADAU1372_REG_ASRCO_SOURCE_2_3, 2, &byte, 1);

		byte=0x03;
		adap->write(adap, dev, ADAU1372_REG_ASRC_MODE, 2, &byte, 1);

		byte=0x01;
		adap->write(adap, dev, ADAU1372_REG_ADC_CTRL0, 2, &byte, 1);

		byte=0x01;
		adap->write(adap, dev, ADAU1372_REG_ADC_CTRL1, 2, &byte, 1);

		byte=0x03;
		adap->write(adap, dev, ADAU1372_REG_ADC_CTRL2, 2, &byte, 1);

		byte=0x03;
		adap->write(adap, dev, ADAU1372_REG_ADC_CTRL3, 2, &byte, 1);

		byte=0x00;
		adap->write(adap, dev, ADAU1372_REG_ADC_VOL(0), 2, &byte, 1);

		byte=0x00;
		adap->write(adap, dev, ADAU1372_REG_ADC_VOL(1), 2, &byte, 1);

		byte=0x00;
		adap->write(adap, dev, ADAU1372_REG_ADC_VOL(2), 2, &byte, 1);

		byte=0x00;
		adap->write(adap, dev, ADAU1372_REG_ADC_VOL(3), 2, &byte, 1);

		byte=0x03;
		adap->write(adap, dev, ADAU1372_REG_DAC_CTRL, 2, &byte, 1);

		byte=0x07;
		adap->write(adap, dev, ADAU1372_REG_SAI0, 2, &byte, 1);

		byte=0x80;
		adap->write(adap, dev, ADAU1372_REG_SAI1, 2, &byte, 1);

		byte=0xFF;
		adap->write(adap, dev, ADAU1372_REG_SOUT_CTRL, 2, &byte, 1);

		byte=0x00;
		adap->write(adap, dev, ADAU1372_REG_MODE_MP(0), 2, &byte, 1);

		byte=0x00;
		adap->write(adap, dev, ADAU1372_REG_MODE_MP(1), 2, &byte, 1);

		byte=0x11;
		adap->write(adap, dev, ADAU1372_REG_MODE_MP(4), 2, &byte, 1);

		byte=0x10;
		adap->write(adap, dev, ADAU1372_REG_MODE_MP(5), 2, &byte, 1);

#if defined(CONFIG_USE_AUDIO_PLL)
		byte=0x11;
#else
		byte=0x12;
#endif
		adap->write(adap, dev, ADAU1372_REG_MODE_MP(6), 2, &byte, 1);

		byte=0x10;
		adap->write(adap, dev, ADAU1372_REG_PB_VOL_SET, 2, &byte, 1);

		byte=0x82;
		adap->write(adap, dev, ADAU1372_REG_PB_VOL_CONV, 2, &byte, 1);

		byte=0x03;
		adap->write(adap, dev, ADAU1372_REG_DEBOUNCE_MODE, 2, &byte, 1);

		byte=0x30;
		adap->write(adap, dev, ADAU1372_REG_OP_STAGE_CTRL, 2, &byte, 1);

		byte=0xFF;
		adap->write(adap, dev, ADAU1372_REG_DECIM_PWR, 2, &byte, 1);

		byte=0x0F;
		adap->write(adap, dev, ADAU1372_REG_INTERP_PWR, 2, &byte, 1);

		byte=0x10;
		adap->write(adap, dev, ADAU1372_REG_DAC_VOL(0), 2, &byte, 1);

		byte=0x10;
		adap->write(adap, dev, ADAU1372_REG_DAC_VOL(1), 2, &byte, 1);
	}

	g_adau1372.enabled = true;
	/* Get the codec Values */
	return ret;
}

/* Set Audio codec ADAU1372 to loopback mode */
int adau1372_set_loopback(void)
{
	int ret = 0;

	uchar byte = 0;

	uchar dev = ADAU1372_ADDR;

	debug("%s\n", __func__);

	dw_i2c_get_adap((uchar **)&adap);

	if ((adap)&& (g_adau1372.loopback == false)){
		debug("start to set adau1372 wtih loopback mode\n");
		g_adau1372.loopback = true;

		byte=0x03;
		adap->write(adap, dev, ADAU1372_REG_CLK_CTRL, 2, &byte, 1);

		udelay(5000); //delay 5ms

#if defined(CONFIG_USE_AUDIO_PLL)
		byte=0x07; /* Set Clk output off */
#else
		byte=0x01; /* Set Clk output on*/
#endif
		adap->write(adap, dev, ADAU1372_REG_CLKOUT_SEL, 2, &byte, 1);

		byte=0xDC;
		adap->write(adap, dev, ADAU1372_REG_DAC_SOURCE, 2, &byte, 1);

		byte=0x76;
		adap->write(adap, dev, ADAU1372_REG_SOUT_SOURCE_0_1, 2, &byte, 1);

		byte=0x44;
		adap->write(adap, dev, ADAU1372_REG_SOUT_SOURCE_2_3, 2, &byte, 1);

		byte=0x44;
		adap->write(adap, dev, ADAU1372_REG_SOUT_SOURCE_4_5, 2, &byte, 1);

		byte=0x44;
		adap->write(adap, dev, ADAU1372_REG_SOUT_SOURCE_6_7, 2, &byte, 1);

		byte=0x04;
		adap->write(adap, dev, ADAU1372_REG_ADC_SDATA_CH, 2, &byte, 1);

		byte=0x54;
		adap->write(adap, dev, ADAU1372_REG_ASRCO_SOURCE_0_1, 2, &byte, 1);

		byte=0x76;
		adap->write(adap, dev, ADAU1372_REG_ASRCO_SOURCE_2_3, 2, &byte, 1);

		byte=0x03;
		adap->write(adap, dev, ADAU1372_REG_ASRC_MODE, 2, &byte, 1);

		byte=0x01;
		adap->write(adap, dev, ADAU1372_REG_ADC_CTRL0, 2, &byte, 1);

		byte=0x01;
		adap->write(adap, dev, ADAU1372_REG_ADC_CTRL1, 2, &byte, 1);

		byte=0x03;
		adap->write(adap, dev, ADAU1372_REG_ADC_CTRL2, 2, &byte, 1);

		byte=0x03;
		adap->write(adap, dev, ADAU1372_REG_ADC_CTRL3, 2, &byte, 1);

		byte=0x00;
		adap->write(adap, dev, ADAU1372_REG_ADC_VOL(0), 2, &byte, 1);

		byte=0x00;
		adap->write(adap, dev, ADAU1372_REG_ADC_VOL(1), 2, &byte, 1);

		byte=0x00;
		adap->write(adap, dev, ADAU1372_REG_ADC_VOL(2), 2, &byte, 1);

		byte=0x00;
		adap->write(adap, dev, ADAU1372_REG_ADC_VOL(3), 2, &byte, 1);

		byte=0x03;
		adap->write(adap, dev, ADAU1372_REG_DAC_CTRL, 2, &byte, 1);

		//byte=0x07;
		//adap->write(adap, dev, ADAU1372_REG_SAI0, 2, &byte, 1);

		adap->read(adap, ADAU1372_ADDR, ADAU1372_REG_SAI1, 2, (uint8_t *)&byte, 1);
		byte=0x00 | byte ;
		adap->write(adap, dev, ADAU1372_REG_SAI1, 2, &byte, 1);

		byte=0xFC;
		adap->write(adap, dev, ADAU1372_REG_SOUT_CTRL, 2, &byte, 1);

		byte=0x00;
		adap->write(adap, dev, ADAU1372_REG_MODE_MP(0), 2, &byte, 1);

		byte=0x00;
		adap->write(adap, dev, ADAU1372_REG_MODE_MP(1), 2, &byte, 1);

		byte=0x11;
		adap->write(adap, dev, ADAU1372_REG_MODE_MP(4), 2, &byte, 1);

		byte=0x10;
		adap->write(adap, dev, ADAU1372_REG_MODE_MP(5), 2, &byte, 1);

#if defined(CONFIG_USE_AUDIO_PLL)
		byte=0x11;
#else
		byte=0x12;
#endif
		adap->write(adap, dev, ADAU1372_REG_MODE_MP(6), 2, &byte, 1);

		byte=0x10;
		adap->write(adap, dev, ADAU1372_REG_PB_VOL_SET, 2, &byte, 1);

		byte=0x82;
		adap->write(adap, dev, ADAU1372_REG_PB_VOL_CONV, 2, &byte, 1);

		byte=0x03;
		adap->write(adap, dev, ADAU1372_REG_DEBOUNCE_MODE, 2, &byte, 1);

		byte=0x30;
		adap->write(adap, dev, ADAU1372_REG_OP_STAGE_CTRL, 2, &byte, 1);

		byte=0xFF;
		adap->write(adap, dev, ADAU1372_REG_DECIM_PWR, 2, &byte, 1);

		byte=0x0F;
		adap->write(adap, dev, ADAU1372_REG_INTERP_PWR, 2, &byte, 1);

		byte=0x10;
		adap->write(adap, dev, ADAU1372_REG_DAC_VOL(0), 2, &byte, 1);

		byte=0x10;
		adap->write(adap, dev, ADAU1372_REG_DAC_VOL(1), 2, &byte, 1);
	}

	g_adau1372.enabled = true;
	/* Get the codec Values */
	return ret;
}

/* ADAU1372 Device Initialisation */
int adau1372_init(	int sampling_rate, int mclk_freq,
			int bits_per_sample, unsigned int channels)
{
	int ret = 0;

	uchar byte = 0;

	uchar dev = ADAU1372_ADDR;

	debug("%s\n", __func__);

	dw_i2c_get_adap((uchar **)&adap);

    debug("%s adap addr=%p\n", __func__, adap);
	g_adau1372.slot_width = bits_per_sample;
	g_adau1372.mclk = mclk_freq;
	g_adau1372.use_pll = false;
	g_adau1372.clk = sampling_rate;
	g_adau1372.loopback = false;

	if (adap) {
		printf("start to init adau1372\n");

		byte=0x03;
		adap->write(adap, dev, ADAU1372_REG_CLK_CTRL, 2, &byte, 1);
		udelay(5000); //delay 5ms

		byte=0x01;
		adap->write(adap, dev, ADAU1372_REG_ADC_CTRL0, 2, &byte, 1);

		byte=0x03;
		adap->write(adap, dev, ADAU1372_REG_ADC_CTRL2, 2, &byte, 1);

		byte=0x03;
		adap->write(adap, dev, ADAU1372_REG_DAC_CTRL, 2, &byte, 1);

		byte=0x00;
		adap->write(adap, dev, ADAU1372_REG_MODE_MP(1), 2, &byte, 1);

		byte=0x30;
		adap->write(adap, dev, ADAU1372_REG_OP_STAGE_CTRL, 2, &byte, 1);

		byte=0x0F;
		adap->write(adap, dev, ADAU1372_REG_DECIM_PWR, 2, &byte, 1);

		byte=0x0C;
		adap->write(adap, dev, ADAU1372_REG_INTERP_PWR, 2, &byte, 1);

		byte=0x00;
		adap->write(adap, dev, ADAU1372_REG_OP_STAGE_MUTE, 2, &byte, 1);

#if defined(CONFIG_USE_AUDIO_PLL)
		byte=0x07;
#else
		byte=0x01;
#endif
		adap->write(adap, dev, ADAU1372_REG_CLKOUT_SEL, 2, &byte, 1); /* Set Clk output on*/

#if defined(CONFIG_USE_AUDIO_PLL)
		byte=0x11;
#else
		byte=0x12;
#endif
		adap->write(adap, dev, ADAU1372_REG_MODE_MP(6), 2, &byte, 1); /* Set Clk output*/

		/* Start to set loopback mode */
		byte=0x10;
		/* Set OUTPUT ASRC0/OUTPUT ASRC1 SOURCE output to serial 0*/
		adap->write(adap, dev, ADAU1372_REG_ASRCO_SOURCE_0_1, 2, &byte, 1);

		byte=0x32;
		/* Set OUTPUT ASRC2/OUTPUT ASRC3 SOURCE output to disable*/
		adap->write(adap, dev, ADAU1372_REG_ASRCO_SOURCE_2_3, 2, &byte, 1);

		byte=0x01;
		/* Set ADAU1372_REG_ASRC_MODE to ASRC input enable*/
		adap->write(adap, dev, ADAU1372_REG_ASRC_MODE, 2, &byte, 1);

		byte=0x98;
		/* Set ADAU1372_REG_SOUT_SOURCE_0_1 to serial 0 and serial 1*/
		adap->write(adap, dev, ADAU1372_REG_SOUT_SOURCE_0_1, 2, &byte, 1);

		byte=0x98;
		/* Set ADAU1372_REG_SOUT_SOURCE_2_3 to serial 0 and serial 1*/
		adap->write(adap, dev, ADAU1372_REG_SOUT_SOURCE_2_3, 2, &byte, 1);

		byte=0x98;
		/* Set ADAU1372_REG_SOUT_SOURCE_4_5 to serial 0 and serial 1*/
		adap->write(adap, dev, ADAU1372_REG_SOUT_SOURCE_4_5, 2, &byte, 1);

		byte=0x98;
		/* Set ADAU1372_REG_SOUT_SOURCE_6_7 to serial 0 and serial 1*/
		adap->write(adap, dev, ADAU1372_REG_SOUT_SOURCE_6_7, 2, &byte, 1);

		byte=0x00;
		/* Set ADAU1372_REG_ADC_SDATA_CH to channel 0*/
		adap->write(adap, dev, ADAU1372_REG_ADC_SDATA_CH, 2, &byte, 1);

		byte=0xCC;
		/* Set ADAU1372_REG_ADC_SDATA_CH to channel 0*/
		adap->write(adap, dev, ADAU1372_REG_DAC_SOURCE, 2, &byte, 1);
		/* end of setting loopback mode */
	}

	g_adau1372.enabled = true;
	return ret;
}
