/*
 * Copyright (C) 2012 bitmain
 *
 */

#ifndef __ADAU1372_H__
#define __ADAU1372_H__

#include <stdlib.h>
#include <stdbool.h>

#define ADAU1372_REG_CLK_CTRL		0x00
#define ADAU1372_REG_PLL(x)		(0x01 + (x))
#define ADAU1372_REG_CLKOUT_SEL     0x07
#define ADAU1372_REG_REGULATOR      0x08
#define ADAU1372_REG_DAC_SOURCE		0x11
#define ADAU1372_REG_SOUT_SOURCE_0_1	0x13
#define ADAU1372_REG_SOUT_SOURCE_2_3	0x14
#define ADAU1372_REG_SOUT_SOURCE_4_5	0x15
#define ADAU1372_REG_SOUT_SOURCE_6_7	0x16
#define ADAU1372_REG_ADC_SDATA_CH	0x17
#define ADAU1372_REG_ASRCO_SOURCE_0_1	0x18
#define ADAU1372_REG_ASRCO_SOURCE_2_3	0x19
#define ADAU1372_REG_ASRC_MODE		0x1a
#define ADAU1372_REG_ADC_CTRL0		0x1b
#define ADAU1372_REG_ADC_CTRL1		0x1c
#define ADAU1372_REG_ADC_CTRL2		0x1d
#define ADAU1372_REG_ADC_CTRL3		0x1e
#define ADAU1372_REG_ADC_VOL(x)		(0x1f + (x))
#define ADAU1372_REG_PGA_CTRL(x)	(0x23 + (x))
#define ADAU1372_REF_PGA_STEP_CTRL	0x27
#define ADAU1372_REG_PGA_BOOST		0x28
#define ADAU1372_REG_POP_SUPPRESS	0x29
#define ADAU1372_REG_TALKTHR		0x2a
#define ADAU1372_REG_MICBIAS		0x2d
#define ADAU1372_REG_DAC_CTRL		0x2e
#define ADAU1372_REG_DAC_VOL(x)		(0x2f + (x))
#define ADAU1372_REG_OP_STAGE_MUTE	0x31
#define ADAU1372_REG_SAI0		0x32
#define ADAU1372_REG_SAI1		0x33
#define ADAU1372_REG_SOUT_CTRL		0x34
#define ADAU1372_REG_MODE_MP(x)		(0x38 + (x))
#define ADAU1372_REG_PB_VOL_SET		0x3F
#define ADAU1372_REG_PB_VOL_CONV	0x40
#define ADAU1372_REG_DEBOUNCE_MODE	0x41
#define ADAU1372_REG_OP_STAGE_CTRL	0x43
#define ADAU1372_REG_DECIM_PWR		0x44
#define ADAU1372_REG_INTERP_PWR		0x45
#define ADAU1372_REG_BIAS_CTRL0		0x46
#define ADAU1372_REG_BIAS_CTRL1		0x47
#define ADAU1372_REG_PAD_CTRL0		0x48
#define ADAU1372_REG_PAD_CTRL1		0x49
#define ADAU1372_REG_PAD_CTRL2		0x4A
#define ADAU1372_REG_PAD_CTRL3		0x4B
#define ADAU1372_REG_PAD_CTRL4		0x4C
#define ADAU1372_REG_PAD_CTRL5		0x4D

#define ADAU1372_CLK_CTRL_PLL_EN	BIT(7)
#define ADAU1372_CLK_CTRL_XTAL_DIS	BIT(4)
#define ADAU1372_CLK_CTRL_CLKSRC	BIT(3)
#define ADAU1372_CLK_CTRL_CC_MDIV	BIT(1)
#define ADAU1372_CLK_CTRL_MCLK_EN	BIT(0)

#define ADAU1372_SAI0_DELAY1		(0x0 << 6)
#define ADAU1372_SAI0_DELAY0		(0x1 << 6)
#define ADAU1372_SAI0_RIGHT_J_24	(0x2 << 6)
#define ADAU1372_SAI0_RIGHT_J_16	(0x3 << 6)
#define ADAU1372_SAI0_DELAY_MASK	(0x3 << 6)
#define ADAU1372_SAI0_SAI_I2S		(0x0 << 4)
#define ADAU1372_SAI0_SAI_TDM2		(0x1 << 4)
#define ADAU1372_SAI0_SAI_TDM4		(0x2 << 4)
#define ADAU1372_SAI0_SAI_TDM8		(0x3 << 4)
#define ADAU1372_SAI0_SAI_MASK		(0x3 << 4)
#define ADAU1372_SAI0_FS_48		0x0
#define ADAU1372_SAI0_FS_8		0x1
#define ADAU1372_SAI0_FS_12		0x2
#define ADAU1372_SAI0_FS_16		0x3
#define ADAU1372_SAI0_FS_24		0x4
#define ADAU1372_SAI0_FS_32		0x5
#define ADAU1372_SAI0_FS_96		0x6
#define ADAU1372_SAI0_FS_192		0x7
#define ADAU1372_SAI0_FS_MASK		0xf

#define ADAU1372_SAI1_TDM_TS		BIT(7)
#define ADAU1372_SAI1_BCLK_TDMC		BIT(6)
#define ADAU1372_SAI1_LR_MODE		BIT(5)
#define ADAU1372_SAI1_LR_POL		BIT(4)
#define ADAU1372_SAI1_BCLKRATE		BIT(2)
#define ADAU1372_SAI1_BCLKEDGE		BIT(1)
#define ADAU1372_SAI1_MS		BIT(0)
#define ADAU1372_SAI1_BCLKRATE_16	1 << 2
#define ADAU1372_SAI1_BCLKRATE_32	0 << 2
#define ADAU1372_SAI1_TDMC_16		1 << 6
#define ADAU1372_SAI1_TDMC_24		0 << 6

#define ADAU1372_SOUT_CTL_MASK		0xff
#define ADAU1372_SOUT_CTL_TDM0_DIS	BIT(0)
#define ADAU1372_SOUT_CTL_TDM1_DIS	BIT(1)
#define ADAU1372_SOUT_CTL_TDM2_DIS	BIT(2)
#define ADAU1372_SOUT_CTL_TDM3_DIS	BIT(3)
#define ADAU1372_SOUT_CTL_TDM4_DIS	BIT(4)
#define ADAU1372_SOUT_CTL_TDM5_DIS	BIT(5)
#define ADAU1372_SOUT_CTL_TDM6_DIS	BIT(6)
#define ADAU1372_SOUT_CTL_TDM7_DIS	BIT(7)
/*
 * DAI hardware audio formats.
 *
 * Describes the physical PCM data formating and clocking. Add new formats
 * to the end.
 */
#define SND_SOC_DAIFMT_I2S		1 /* I2S mode */
#define SND_SOC_DAIFMT_RIGHT_J		2 /* Right Justified mode */
#define SND_SOC_DAIFMT_LEFT_J		3 /* Left Justified mode */
#define SND_SOC_DAIFMT_DSP_A		4 /* L data MSB after FRM LRC */
#define SND_SOC_DAIFMT_DSP_B		5 /* L data MSB during FRM LRC */
#define SND_SOC_DAIFMT_AC97		6 /* AC97 */
#define SND_SOC_DAIFMT_PDM		7 /* Pulse density modulation */

/* left and right justified also known as MSB and LSB respectively */
#define SND_SOC_DAIFMT_MSB		SND_SOC_DAIFMT_LEFT_J
#define SND_SOC_DAIFMT_LSB		SND_SOC_DAIFMT_RIGHT_J

#ifndef EINVAL
#define EINVAL          22      /* Invalid argument */
#endif //EINVAL

#define SNDRV_PCM_RATE_CONTINUOUS	(1<<30)		/* continuous range */
#define SNDRV_PCM_RATE_KNOT		(1<<31)		/* supports more non-continuos rates */

#define ADAU1372_ADDR 0x3C

/* #define ADAU1372_FORMATS (SNDRV_PCM_FMTBIT_S16_LE |	SNDRV_PCM_FMTBIT_S32_LE) */

/*
 * DAI hardware signal polarity.
 *
 * Specifies whether the DAI can also support inverted clocks for the specified
 * format.
 *
 * BCLK:
 * - "normal" polarity means signal is available at rising edge of BCLK
 * - "inverted" polarity means signal is available at falling edge of BCLK
 *
 * FSYNC "normal" polarity depends on the frame format:
 * - I2S: frame consists of left then right channel data. Left channel starts
 *      with falling FSYNC edge, right channel starts with rising FSYNC edge.
 * - Left/Right Justified: frame consists of left then right channel data.
 *      Left channel starts with rising FSYNC edge, right channel starts with
 *      falling FSYNC edge.
 * - DSP A/B: Frame starts with rising FSYNC edge.
 * - AC97: Frame starts with rising FSYNC edge.
 *
 * "Negative" FSYNC polarity is the one opposite of "normal" polarity.
 */
#define SND_SOC_DAIFMT_NB_NF		(0 << 8) /* normal bit clock + frame */
#define SND_SOC_DAIFMT_NB_IF		(2 << 8) /* normal BCLK + inv FRM */
#define SND_SOC_DAIFMT_IB_NF		(3 << 8) /* invert BCLK + nor FRM */
#define SND_SOC_DAIFMT_IB_IF		(4 << 8) /* invert BCLK + FRM */

#define SND_SOC_DAIFMT_CBM_CFM		(1 << 12) /* codec clk & FRM master */
#define SND_SOC_DAIFMT_CBS_CFM		(2 << 12) /* codec clk slave & FRM master */
#define SND_SOC_DAIFMT_CBM_CFS		(3 << 12) /* codec clk master & frame slave */
#define SND_SOC_DAIFMT_CBS_CFS		(4 << 12) /* codec clk & FRM slave */

#define SND_SOC_DAIFMT_FORMAT_MASK	0x000f
#define SND_SOC_DAIFMT_CLOCK_MASK	0x00f0
#define SND_SOC_DAIFMT_INV_MASK		0x0f00
#define SND_SOC_DAIFMT_MASTER_MASK	0xf000


static const unsigned int adau1372_rates[] = {
	[ADAU1372_SAI0_FS_8] = 8000,
	[ADAU1372_SAI0_FS_12] = 12000,
	[ADAU1372_SAI0_FS_16] = 16000,
	[ADAU1372_SAI0_FS_24] = 24000,
	[ADAU1372_SAI0_FS_32] = 32000,
	[ADAU1372_SAI0_FS_48] = 48000,
	[ADAU1372_SAI0_FS_96] = 96000,
	[ADAU1372_SAI0_FS_192] = 192000,
};

#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

struct adau1372 {
	unsigned int clk;
	bool use_pll;
	bool enabled;
	unsigned int slot_width;
	unsigned int rate;
	unsigned int mclk;
	bool loopback;
};

int adau1372_set_dai_fmt(unsigned int fmt);

void adau1372_set_tdm_format(unsigned int slot_no);

int adau1372_hw_params(unsigned int rate, unsigned int slot_width);

int adau1372_update_ws_params(unsigned int aud_mode, unsigned int slot_width);

int adau1372_set_loopback_without_tdm_enable(void);

int adau1372_set_loopback(void);

int adau1372_init(	int sampling_rate, int mclk_freq, int bits_per_sample, unsigned int channels);

#endif /*__ADAU1372_H__ */
