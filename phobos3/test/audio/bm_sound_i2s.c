/*
 * Copyright (C) 2018 bitmain
 */

#include <malloc.h>
#include <common.h>
#include "system_common.h"
#include "bm_dw_i2c.h"
#include "bm_i2s_tdm.h"
#include "bm_sound.h"
#include "adau1372.h"
#include "dma.h"

/* defines */
#define SOUND_400_HZ 400
#define SOUND_BITS_IN_BYTE 8


/* I2C values */
#define AUDIO_I2C_BUS		0
#define AUDIO_I2C_REG		0x3C

/* Audio Codec */
#define AUDIO_CODEC		"audu1372"

#define ENABLE_DEBUG

unsigned int record_data[RX_BUFFER_SIZE];
unsigned int record_size = 0;
bool loopback = false;

static struct i2stx_info g_i2stx_pri;


static void sound_create_square_wave(unsigned short *data, int size, uint32_t freq)
{
	const int sample = 48000;
	const unsigned short amplitude = 16000; /* between 1 and 32767 */
	const int period = freq ? sample / freq : 0;
	const int half = period / 2;

	/* Make sure we don't overflow our buffer */
	if (size % 2)
		size--;

	while (size) {
		int i;
		for (i = 0; size && i < half; i++) {
			size -= 2;
			*data++ = amplitude;
			*data++ = amplitude;
		}
		for (i = 0; size && i < period - half; i++) {
			size -= 2;
			*data++ = -amplitude;
			*data++ = -amplitude;
		}
	}
}

/*
 * set_sound_i2s_values gets values for i2s parameters
 *
 * @param i2stx_info	i2s transmitter transfer param structure
 */
int set_sound_i2s_values(unsigned int samplerate, unsigned int bitspersample, unsigned int ws, unsigned int sclk)
{
    debug("%s\n", __func__);

	g_i2stx_pri.samplingrate = samplerate;
	g_i2stx_pri.bitspersample = bitspersample;
	g_i2stx_pri.rfs = ws; /* ws clock*/
	return 0;
}

/*
 * set_default_sound_i2s_values for i2s parameters
 *
 */
int set_default_sound_i2s_values(unsigned int i2s_no)
{
    debug("%s\n", __func__);

	g_i2stx_pri.mclk_out_en = 0;
#if defined(CONFIG_USE_AUDIO_PLL)
	g_i2stx_pri.clk_src = AUD_CLK_FROM_PLL;
#else
	g_i2stx_pri.clk_src = AUD_CLK_FROM_MCLK_IN;
#endif
	g_i2stx_pri.channels = 1;
	g_i2stx_pri.id = 1;
	g_i2stx_pri.base_address = i2s_get_base(i2s_no);
	g_i2stx_pri.sys_base_address = i2s_get_sys_base();
	g_i2stx_pri.role = MASTER_MODE;
	g_i2stx_pri.inv = FMT_IB_NF;
	g_i2stx_pri.aud_mode = I2S_MODE;
	g_i2stx_pri.mclk_div = 1; /* 1 means bypass*/
	g_i2stx_pri.slot_no = 2;

#if defined(BOARD_FPGA)
	g_i2stx_pri.samplingrate = 48000;
	g_i2stx_pri.bitspersample = 32; /* bit resolution */
	g_i2stx_pri.sync_div = WSS_32_CLKCYCLE;
	g_i2stx_pri.bclk_div = 0x04; /* 1 means bypass*/
#else
#if defined(BOARD_PALLADIUM)
	g_i2stx_pri.samplingrate = 96000;
	g_i2stx_pri.bitspersample = 32; /* bit resolution */
	g_i2stx_pri.sync_div = WSS_32_CLKCYCLE;
	g_i2stx_pri.bclk_div = 0x02; /* 1 means bypass*/
#else
	g_i2stx_pri.samplingrate = 192000;
	g_i2stx_pri.bitspersample = 32; /* bit resolution */
	g_i2stx_pri.sync_div = WSS_32_CLKCYCLE; /* WS clock cycle (L+R). 0x40: 64 bit, 0x30: 48 bit, 0x20: 32 bit*/
	g_i2stx_pri.bclk_div = 0x01; /* 1 means bypass*/
#endif
#endif
	return 0;
}

/*
 * get_sound_i2s_values gets values for i2s parameters
 *
 * @param i2stx_info	i2s transmitter transfer param structure
 */
int get_sound_i2s_values(uchar **i2s)
{
    debug("%s\n", __func__);

	*i2s=(uchar *)&g_i2stx_pri;

	return 0;
}

/*
 * Init codec
 *
 * @param pi2s_tx	i2s parameters required by codec
 * @return              int value, 0 for success
 */
static int codec_init(struct i2stx_info *pi2s_tx)
{
	int ret;
	const char *codectype;

	printf("%s, samplerate=%d, bit=%d\n", __func__, pi2s_tx->samplingrate, pi2s_tx->bitspersample);


	codectype = "adau1372";
	printf("device = %s\n", codectype);
	ret = adau1372_init(pi2s_tx->samplingrate,
				(pi2s_tx->samplingrate * (pi2s_tx->rfs)),
				pi2s_tx->bitspersample, pi2s_tx->channels);
	if (ret) {
		printf("%s: Codec init failed\n", __func__);
		return -1;
	}
	return 0;
}

int bm_sound_init()
{
	int ret = 0;
	struct i2stx_info *pi2s_tx = &g_i2stx_pri;

	printf("%s, samplerarte=%d\n", __func__, pi2s_tx->samplingrate);
	/* Get the I2S Values */

	if (codec_init(pi2s_tx) < 0) {
		printf(" Codec init failed\n");
		return -1;
	}

	ret = i2s_init(pi2s_tx, record_data);
	if (ret) {
		printf("%s: Failed to init i2s transmit: ret=%d\n", __func__, ret);
		return ret;
	}

	return ret;
}

int bm_rx_test(int argc, char **argv)
{
	unsigned int resolution;

	resolution = atoi(argv[1]);

	record_size = i2s_test_rx(&g_i2stx_pri, resolution);
	loopback = true;

	return 0;
}

int bm_loop_test(int argc, char **argv)
{
	unsigned int time;

	time = atoi(argv[1]);

	i2s_loop_test(&g_i2stx_pri, time, record_data);

	return 0;
}

int bm_pdm_loop_test(int argc, char **argv)
{
	unsigned int time;

	time = atoi(argv[1]);

	i2s_pdm_loop_test(&g_i2stx_pri, time, record_data);

	return 0;
}

int bm_concurret_rx_test(int argc, char **argv)
{
	concurrent_rx();
	return 0;
}

int bm_receive_sound(int argc, char **argv)
{

	debug("record_data=%p\n", record_data);
	record_size = i2s_receive_rx_data(&g_i2stx_pri,record_data);
	debug("Set loopback to true");
	loopback = true;

	return 0;
}

int bm_sound_play(int argc, char **argv)
{
	unsigned int data[6400];
	unsigned long data_size;
	unsigned int ret = 0;
	unsigned int msec;

	debug("%s, record_data=%p\n", __func__, record_data);
	msec = atoi(argv[1]);
	/*Buffer length computation */

	if (loopback == true) {
		ret = i2s_transfer_tx_data(&g_i2stx_pri, record_data, record_size);
		//loopback = false;
	} else {
		data_size= 6400;

		if (data == NULL) {
			printf("%s: malloc failed\n", __func__);
			return -1;
		}

		sound_create_square_wave((unsigned short *)data,
					data_size / sizeof(unsigned short),
					48000);
		while (msec >= 1000) {
			ret = i2s_transfer_tx_data(&g_i2stx_pri, data, (data_size / sizeof(int)));
			msec -= 1000;
		}
		if (msec) {
			unsigned long size = (data_size * msec) / (sizeof(int) * 1000);
			ret = i2s_transfer_tx_data(&g_i2stx_pri, data, size);
		}
	}
	return ret;
}

int bm_sample_rate_convert(int argc, char **argv)
{
	i2s_src_test(&g_i2stx_pri);
	return 0;
}