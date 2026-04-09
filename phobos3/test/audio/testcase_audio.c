/*
 * Copyright (C) 2018 bitmain
 */

#include <stdint.h>
#include <common.h>
#include <stdlib.h>
#include <string.h>
#include "system_common.h"
#include "module_testcase.h"
#include "mmio.h"
#include "bm_dw_i2c.h"
#include "bm_i2s_tdm.h"
#include "bm_sound.h"
#include "adau1372.h"
#include "dma.h"
#include "cli.h"
#include "timer.h"
#include "command.h"

#define INV_MASK 	0xF000
#define NB_NF 		0X3000
#define NB_IF 		0x2000
#define IB_IF 		0X1000
#define IB_NF 		0x0000
#define MODE_MASK	0xF00
#define I2S 		0X000
#define Left_J 		0x100
#define Right_J 	0x200
#define PCM_A 		0X300
#define PCM_B 		0X400
#define TDM 		0x500
#define MS_MASK		0xF0
#define MASTER 		0X00
#define SLAVE 		0x20
#define SLOT_MASK	0xF
#define SLOT_2 		0x2
#define SLOT_4  	0x4

int bm_strcmp (const char *d, const char *s)
{
	while ('\0' != *d) {
		if (*d != *s)
			return *d - *s;
		d++;
		s++;
	}

	return *d - *s;
}

/* To initial channel assignment for I2C/I2S */
static void dma_channel_init(void)
{
	mmio_clrsetbits_32(TOP_DMA_CH_REMAP0, 0x3f<<DMA_REMAP_CH0_OFFSET, DMA_RX_REQ_I2S0 << DMA_REMAP_CH0_OFFSET);
	mmio_clrsetbits_32(TOP_DMA_CH_REMAP0, 0x3f<<DMA_REMAP_CH1_OFFSET, DMA_TX_REQ_I2S0 << DMA_REMAP_CH1_OFFSET);
	mmio_clrsetbits_32(TOP_DMA_CH_REMAP0, 0x3f<<DMA_REMAP_CH2_OFFSET, DMA_RX_REQ_I2S1 << DMA_REMAP_CH2_OFFSET);
	mmio_clrsetbits_32(TOP_DMA_CH_REMAP0, 0x3f<<DMA_REMAP_CH3_OFFSET, DMA_TX_REQ_I2S1 << DMA_REMAP_CH3_OFFSET);
	mmio_clrsetbits_32(TOP_DMA_CH_REMAP0, 0x1<<DMA_REMAP_UPDATE_OFFSET, 1 << DMA_REMAP_UPDATE_OFFSET);
#ifdef CONFIG_USE_I2C_DMA
	mmio_clrsetbits_32(TOP_DMA_CH_REMAP1, 0x3f<<DMA_REMAP_CH4_OFFSET, DMA_RX_REQ_I2C0 << DMA_REMAP_CH4_OFFSET);
	mmio_clrsetbits_32(TOP_DMA_CH_REMAP1, 0x3f<<DMA_REMAP_CH5_OFFSET, DMA_TX_REQ_I2C0 << DMA_REMAP_CH5_OFFSET);
#else
	mmio_clrsetbits_32(TOP_DMA_CH_REMAP1, 0x3f<<DMA_REMAP_CH4_OFFSET, DMA_RX_REQ_I2S2 << DMA_REMAP_CH4_OFFSET);
	mmio_clrsetbits_32(TOP_DMA_CH_REMAP1, 0x3f<<DMA_REMAP_CH5_OFFSET, DMA_TX_REQ_I2S2 << DMA_REMAP_CH5_OFFSET);
#endif
	mmio_clrsetbits_32(TOP_DMA_CH_REMAP1, 0x3f<<DMA_REMAP_CH6_OFFSET, DMA_REQ_AUDSRC << DMA_REMAP_CH6_OFFSET);
//	mmio_clrsetbits_32(TOP_DMA_CH_REMAP1, 0x3f<<DMA_REMAP_CH7_OFFSET, DMA_TX_REQ_I2S3 << DMA_REMAP_CH7_OFFSET);
	mmio_clrsetbits_32(TOP_DMA_CH_REMAP1, 0x1<<DMA_REMAP_UPDATE_OFFSET, 1 << DMA_REMAP_UPDATE_OFFSET);
}

static int show_usage(int argc, char **argv)
{
	printf("Usage: \n");
	printf("i2crw [reg] [val] - Write adau1372 register\n");
	printf("i2crr [reg] - Read adau1372 register\n");
	printf("i2cdump - Dump adau1372 registers\n");
	printf("i2crr2 - Read specific I2C (0x3D) device register\n");
	printf("i2css [speed] - Set i2c speed\n");
	printf("i2srw [reg] [val] - Write I2S register\n");
	printf("i2srr [reg] - Read I2S regiter\n");
	printf("i2sss [samplerate] - Set I2S sample rate\n");
	printf("i2ssf [fmt] - Set audio fmt (fmt=inv|mode|role|slot_no)\n");
	printf("	inv:\n");
	printf("	NB_NF: 0X3000    NB_IF:  0x2000\n");
	printf("	IB_IF: 0X1000    IB_NF(default):  0x0000\n");
	printf("	mode:\n");
	printf("	I2S (default): 0X000  Left_J: 0x100  Right_J: 0x200\n");
	printf("	PCM_A: (data MSB after FRM LRC): 0X300\n");
	printf("	PCM_B: (data MSB during FRM LRC): 0X400\n");
	printf("	TDM: 0x500\n");
	printf("	role:\n");
	printf("	Master (default): 0X00   Slave:  0x20\n");
	printf("	slot_no:\n");
	printf("	2 slots (default): 0X2   4 slots:  0x4\n");
	printf("i2ssw [ws] - Set ws cycle, 16/24/32 bits\n");
	printf("i2ssr [resolution] - Set resolution, 12/16/24/32 bits\n");
	printf("tt - test talkthrough mode\n");
	printf("pl [msec] - Play sound with n msec\n");
	printf("rx - receive sound\n");
	printf("rt [resolution]- start to test RX, resolution is one of 16/24/32 bits\n");
	printf("loop [s]- continuous loopback test with [s] sec\n");
	printf("crx - concurrent rx test\n");
	printf("pdm_loop [s]- continuous loopback test with digital mic for [s] sec\n");
	printf("src - sample rate converter test\n");
	return 0;
}

static int do_i2c_write(int argc, char **argv)
{
	struct i2c_adapter *adap=NULL;
	unsigned int reg;
	u8 val;
	reg = strtoul(argv[1], NULL, 16);
	val = strtoul(argv[2], NULL, 16);

	printf("%s, reg=%X, val=%X\n", __func__, reg, val);
	dw_i2c_get_adap((uchar **)&adap);
	adap->write(adap, 0x3C, reg, 2, &val, 1);

	return 0;
}
static int do_i2c_read(int argc, char **argv)
{
	struct i2c_adapter *adap=NULL;
	uint8_t val = 1;
	unsigned int reg;

	reg = strtoul(argv[1], NULL, 16);
	dw_i2c_get_adap((uchar **)&adap);
	adap->read(adap, 0x3C, reg, 2, &val, 1);

	printf("Read reg %02X get value %02X\n", reg, val);

	return 0;
}

static int do_i2c_dump(int argc, char **argv)
{

	struct i2c_adapter *adap=NULL;
	uint8_t val = 1;
	unsigned int reg;

	dw_i2c_get_adap((uchar **)&adap);

	for (reg=0x01; reg <= 0x4E; reg++){
		adap->read(adap, 0x3C, reg - 1, 2, &val, 1);
		printf("0x%02X=0x%02X  ", reg-1, val);
		if ((reg % 0x5)== 0)
			printf("\n");
	}

	printf("\n");
	return 0;
}

static int do_i2c_set_speed(int argc, char **argv)
{
	struct i2c_adapter *adap=NULL;
	unsigned int speed;

	speed = atoi(argv[1]);

	dw_i2c_get_adap((uchar **)&adap);
	adap->set_bus_speed(adap, speed);

	printf("Set I2C speed with %d\n", speed);
	return 0;
}

static int do_i2s_write_reg(int argc, char **argv)
{
	struct i2stx_info *i2s = NULL;
	uchar *i2s_reg = NULL;
	unsigned int reg;
	unsigned int val;

	reg = strtoul(argv[1], NULL, 16);
	val= strtoul(argv[2], NULL, 16);

	debug("%s\n", __func__);

	get_sound_i2s_values((uchar **)&i2s);
 	i2s_reg = (uchar *)i2s->base_address;

	printf("Write reg 0x%02X with value 0x%04X\n", reg, val);
	writel(i2s_reg+reg, val);
	mdelay(1);
	printf("Confirm reg 0x%02X value is 0x%04X\n", reg, readl(i2s_reg+reg));

	return 0;
}

static int do_i2s_read_reg(int argc, char **argv)
{
	struct i2stx_info *i2s = NULL;
	uchar *i2s_reg = NULL;
	u32 val;
	unsigned int reg;

	debug("%s\n", __func__);
	reg= strtoul(argv[1], NULL, 16);

	get_sound_i2s_values((uchar **)&i2s);
	i2s_reg = (uchar *)(i2s->base_address);
	val=readl(i2s_reg+reg);
	printf("Read reg 0x%02X get value 0x%04X\n", reg, val);
	return 0;
}

static int do_i2s_set_ws(int argc, char **argv)
{
	struct i2stx_info *i2s = NULL;
	uchar *i2s_reg = NULL;
	unsigned int ws;

	debug("Set WS clock cycle to %d bits\n", ws);
	ws = atoi(argv[1]);
	get_sound_i2s_values((uchar **)&i2s);

	i2s_reg = (uchar *)i2s->base_address;

	switch(ws) {
		case(16):
			i2s->sync_div = WSS_16_CLKCYCLE;
			break;
		case(24):
			i2s->sync_div = WSS_24_CLKCYCLE;
			break;
		case(32):
			i2s->sync_div = WSS_32_CLKCYCLE;
			break;
		case(256):
			i2s->sync_div = WSS_256_CLKCYCLE;
			break;
	}

	i2s_set_ws_clock_cycle((struct i2s_tdm_regs *)i2s_reg, i2s->sync_div, i2s->aud_mode);

	return 0;
}

static int do_i2s_set_resolution(int argc, char **argv)
{
	struct i2stx_info *i2s = NULL;
	uchar *i2s_reg = NULL;
	unsigned int resolution, slot_size;

	debug("%s\n", __func__);

	resolution = atoi(argv[1]);
	slot_size = atoi(argv[2]);
	get_sound_i2s_values((uchar **)&i2s);

	i2s_reg = (uchar *)i2s->base_address;

	i2s->bitspersample = resolution;

   	i2s_set_resolution((struct i2s_tdm_regs *)i2s_reg, resolution, slot_size);

   return 0;
}

static int do_i2s_set_rate(int argc, char **argv)
{
	struct i2stx_info *i2s = NULL;
	uchar *i2s_reg = NULL;
	unsigned int sample_rate;

	debug("%s\n", __func__);

	sample_rate = atoi(argv[1]);
	get_sound_i2s_values((uchar **)&i2s);

	i2s_reg = (uchar *)i2s->base_address;

	i2s->samplingrate = sample_rate;

   i2s_set_sample_rate((struct i2s_tdm_regs *)i2s_reg, sample_rate);

   return 0;
}

static int do_i2s_set_fmt(int argc, char **argv)
{
	struct i2stx_info *i2s = NULL;
	uchar *i2s_reg = NULL;
	unsigned int fmt;

	debug("%s\n", __func__);
	fmt = strtoul(argv[1], NULL, 16);
	get_sound_i2s_values((uchar **)&i2s);

	i2s_reg = (uchar *)i2s->base_address;

	switch(fmt & INV_MASK){
		case (NB_NF):
			i2s->inv = FMT_NB_NF;
			break;
		case (NB_IF):
			i2s->inv = FMT_NB_IF;
			break;
		case (IB_IF):
			i2s->inv = FMT_IB_IF;
			break;
		case (IB_NF):
			i2s->inv = FMT_IB_NF;
			break;
		default:
			printf("Warning!! no such inv format. Set to default IB_NF\n");
			i2s->inv = FMT_IB_NF;
			break;
	}

	switch(fmt & MODE_MASK){
		case (I2S):
			i2s->aud_mode = I2S_MODE;
			break;
		case (Left_J):
			i2s->aud_mode = LJ_MODE;
			break;
		case (Right_J):
			i2s->aud_mode = RJ_MODE;
			break;
		case (PCM_A):
			i2s->aud_mode = PCM_A_MODE;
			break;
		case (PCM_B):
			i2s->aud_mode = PCM_B_MODE;
			break;
		case (TDM):
			i2s->aud_mode = TDM_MODE;
			break;
		default:
			printf("Warning!! no such audio mode. Set to default I2S mode\n");
			i2s->aud_mode = I2S_MODE;
			break;
	}

	switch(fmt & MS_MASK){
		case (MASTER):
			i2s->role = MASTER_MODE;
			break;
		case (SLAVE):
			i2s->role = SLAVE_MODE;
			break;
		default:
			printf("Warning!! no such role mode. Set to default master mode\n");
			i2s->role = I2S_MODE;
			break;
	}

	switch(fmt & SLOT_MASK){
		case (SLOT_2):
			i2s->slot_no = SLOT_2;
			break;
		case (SLOT_4):
			i2s->slot_no = SLOT_4;
			break;
		default:
			printf("Warning!! no such slot number. Set to default number 2\n");
			i2s->slot_no = SLOT_2;
			break;
	}

	i2s_set_fmt((struct i2s_tdm_regs *)i2s_reg, i2s->role, i2s->aud_mode, i2s->inv, i2s->slot_no);

	return 0;
}

static int do_au_talkthrough_mode(int argc, char **argv)
{
	struct i2c_adapter *adap=NULL;
	uchar val = 0;

	debug("%s\n", __func__);
	dw_i2c_get_adap((uchar **)&adap);
	val = 0x01;
	adap->write(adap, 0x3C, ADAU1372_REG_TALKTHR, 2, &val, 1);
	val = 0x0F;
	adap->write(adap, 0x3C, ADAU1372_REG_MODE_MP(0), 2, &val, 1);

	return 0;
}

static struct cmd_entry test_cmd_list[] __attribute__ ((unused)) = {
	{"show", show_usage, 0, "show audio test usage"},
	{"i2crr", do_i2c_read, 1, "[reg]"},
	{"i2crw", do_i2c_write, 2, "[reg] [val]"},
	{"i2cdump", do_i2c_dump, 0, ""},
	{"i2crr2", read_special_I2C_device, 0, ""},
	{"i2css", do_i2c_set_speed, 1, "[frequency]"},
	{"i2srw", do_i2s_write_reg, 2, "[reg] [val]"},
	{"i2srr", do_i2s_read_reg, 1, "[reg]"},
	{"i2sss", do_i2s_set_rate, 1, "[sample rate] (ex. 192000/96000/48000/16000/12000/8000)"},
	{"i2ssf", do_i2s_set_fmt, 1, "[fmt] 	, fmt=(inv|mode|role|slot_no)\n \
								  inv:\n \
								  NB_NF: 0X3000    NB_IF:  0x2000\n \
								  IB_IF: 0X1000    IB_NF(default):  0x0000\n \
								  mode:\n \
								  I2S (default): 0X000  Left_J: 0x100  Right_J: 0x200\n \
								  PCM_A: (data MSB after FRM LRC): 0X300 \n \
								  PCM_B: (data MSB during FRM LRC): 0X400  \n \
								  TDM: 0x500\n \
								  role:\n\
								  Master (default): 0X00   Slave:  0x20\n \
								  slot_no:\n \
								  2 slots (default): 0X2   4 slots:  0x4\n"},
	{"i2ssw", do_i2s_set_ws, 1, "[ws]"},
	{"i2ssr", do_i2s_set_resolution, 2, "[resolution] [slot_size]"},
	{"tt", do_au_talkthrough_mode, 0, ""},
	{"pl", bm_sound_play, 1, "[ms]"},
	{"rx", bm_receive_sound, 0, ""},
	{"rt", bm_rx_test, 1, "[ms]"},
	{"loop", bm_loop_test, 1, "[s]"},
	{"crx", bm_concurret_rx_test, 0, ""},
	{"pdm_loop", bm_pdm_loop_test, 1, "[s]"},
	{"src", bm_sample_rate_convert, 0, ""},
	{NULL, NULL, 0, NULL}
};
#if 1
static int irq_handler(int irqn, void *priv)
{

	disable_irq(irqn);

    return 0;
}
#endif

int testcase_audio(void)
{
//	unsigned int ret;
	unsigned int i;

	printf("Start %s\n", __func__);
#if 1
	request_irq(SDMA_INTR_CPU0, irq_handler, 0, "sysDMA int", NULL);
	request_irq(I2C0_INTR, irq_handler, 0, "IC0_INTR int", NULL);
	request_irq(I2C1_INTR, irq_handler, 0, "IC2_INTR int", NULL);
	request_irq(I2C2_INTR, irq_handler, 0, "IC2_INTR int", NULL);
	request_irq(I2C3_INTR, irq_handler, 0, "IC2_INTR int", NULL);
	request_irq(I2C4_INTR, irq_handler, 0, "IC2_INTR int", NULL);
	request_irq(I2S0_INT, irq_handler, 0, "I2S0_INTR int", NULL);
	request_irq(I2S1_INT, irq_handler, 0, "I2S1_INTR int", NULL);
	request_irq(I2S2_INT, irq_handler, 0, "I2S2_INTR int", NULL);
	request_irq(I2S3_INT, irq_handler, 0, "I2S3_INTR int", NULL);
#endif
	/* enable pinmux*/
	pinmux_config(PINMUX_I2C0);
	pinmux_config(PINMUX_I2S1);
	pinmux_config(PINMUX_I2S2);


	dma_channel_init();
	bm_i2c_init();
	set_default_sound_i2s_values(I2S1);
	bm_sound_init();

	for(i = 0;i < ARRAY_SIZE(test_cmd_list) - 1;i++) {
		command_add(&test_cmd_list[i]);
	}
	cli_simple_loop();
//	do {
//		ret = cli_simple_loop();
//	} while (ret == 0);

	return 0;
}

#ifndef BUILD_TEST_CASE_all
int testcase_main(void)
{
  int ret;

  ret = testcase_audio();
  uartlog("testcase audio %s\n", ret?"failed":"passed");
  return ret;
}
#endif

module_testcase("1", testcase_audio);