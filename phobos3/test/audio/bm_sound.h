/*
 * Copyright (C) 2018 bitmain
 */
#ifndef __BM_SOUND_H__
#define __BM_SOUND_H__

/* sound codec enum */
enum en_sound_codec {
    CODEC_ADAU1372,
};

/* Codec information structure to store the info from device tree */
struct sound_codec_info {
	int i2c_bus;
	int i2c_dev_addr;
	enum en_sound_codec codec_type;
};

#if defined(BOARD_PALLADIUM)
#define RX_BUFFER_SIZE 50000
#elif (defined(BOARD_FPGA) || defined(BOARD_ASIC)) &&  defined(RUN_IN_DDR)
#define RX_BUFFER_SIZE 1500000
#else
#define RX_BUFFER_SIZE 50000
#endif
/*
 * Initialises i2s default parameters
*/
int set_default_sound_i2s_values(unsigned int i2s_no);

/*
 * Get i2s parameters
*/
int get_sound_i2s_values(uchar **i2s);


/*
 * Set i2s default parameters
*/

int set_sound_i2s_values(unsigned int samplerate, unsigned int bitspersample, unsigned int ws, unsigned int sclk);


/*
 * Initialises audio sub system
 * @param blob	Pointer of device tree node or NULL if none.
 * @return	int value 0 for success, -1 for error
 */
int bm_sound_init(void);


/*
 * Receive and then playback automatically for certain period
 *
 */
int bm_loop_test(int argc, char **argv);

/*
 * Receive from PDM module and then playback through external codec for certain period
 *
 */
int bm_pdm_loop_test(int argc, char **argv);

/*
 * Sample rate convert test. From 48K to 44.1K
 *
 */

int bm_sample_rate_convert(int argc, char **argv);

/*
* Receive audio data from 2 I2S concurrently
*/
int bm_concurret_rx_test(int argc, char **argv);

/*
 * Receive the pcm data to check if the received data is correct
 * or not
 *
 */

int bm_rx_test(int argc, char **argv);

/*
 * Receive the pcm data buffer through i2s0 to make the
 * sine wave sound
 *
 */

int bm_receive_sound(int argc, char **argv);

/*
 * plays the pcm data buffer in pcm_data.h through i2s0 to make the
 * sine wave sound
 *
 * @return	int 0 for success, -1 for error
 */
int bm_sound_play(int argc, char **argv);

#endif  /* __BM_SOUND__H__ */