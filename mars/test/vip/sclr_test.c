#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdarg.h>
#include <errno.h>
#ifdef ENV_EMU
#include <stdint.h>
#include <unistd.h>
#include "emu/command.h"
#include "emu/cli.h"
#else
#include "command.h"
#include "cli.h"
#include "system_common.h"
#include "module_testcase.h"
#include "uart.h"
#include "timer.h"
#endif

#include "vip_common.h"
#include "vip_sys_reg.h"
#include "scaler_reg.h"
#include "reg.h"
#include "scaler.h"
#include "cmdq.h"
#include "ldc.h"
#include "testcase_ldc.h"
#include "sclr_test.h"
#include "testcase_sclr.h"

extern u64 src_addr_scl[3][3];
extern u64 dst_addr_scl[3][3];
extern volatile int sclr_mark;
extern u64 sclr_timer_tick;

extern void _get_bytesperline(enum sclr_format sc_fmt, u16 width, u16 bytesperline[2]);
extern void _intr_setup(u8 inst, enum sclr_img_in img_inst);
extern int sclr_check_irq_handler(int timeout);

void sclr_trig_start_ext(int img_inst, int sc_inst)
{
	struct sclr_top_cfg *cfg = sclr_top_get_cfg();

	sclr_mark = 0;
	sclr_timer_tick = timer_get_tick();
	if (img_inst == SCL_IMG_D) {
		cfg->sclr_enable[0] = true;
		cfg->disp_enable = true;
	} else if (sc_inst > 0 && sc_inst < SCL_MAX_INST) {
		cfg->sclr_enable[sc_inst] = true;
	} else {
		uartlog("%s: img(%d), sc(%d), wrong path\n", __func__, img_inst, sc_inst);
		return;
	}

	sclr_top_set_cfg(cfg);

	sclr_img_start(img_inst);
}

int sclr_size_test(int argc, char **argv)
{
	int ret = 0;
	int i;
	struct sclr_scale_cfg cfg;
	u8 inst;
	int max_size[SCL_MAX_INST] = {1920, 2880, 1920, 1280};
	enum sclr_img_in img_inst;
	union sclr_input in;
	enum sclr_format img_in_fmt = SCL_FMT_NV21;
	enum sclr_csc img_in_csc = SCL_CSC_601_LIMIT_YUV2RGB;
	enum sclr_format odma_fmt = SCL_FMT_NV21;
	enum sclr_csc odma_csc = SCL_CSC_601_LIMIT_RGB2YUV;
	struct sclr_mem mem;
	u16 pitch[3];
	struct sclr_size src;
	struct sclr_size dst;
	struct sclr_rect crop;
	struct sclr_odma_cfg *odma_cfg;

	for (inst = 0; inst < SCL_MAX_INST; inst++) {
		for (i = 64; i <= 2880; i += 32) {
			if (i > max_size[inst])
				continue;

			memset(&cfg, 0, sizeof(cfg));

			cfg.src.w = i;
			cfg.src.h = i;
			cfg.crop.w = i;
			cfg.crop.h = i;
			cfg.dst.w = i;
			cfg.dst.h = i;

			sclr_ctrl_set_scale(inst, &cfg);
			sclr_set_scale_phase(inst, 0, 0);

			// CFG_OP_IMG_CFG
			if (inst == 0) {
				img_inst = SCL_IMG_D;
			} else {
				img_inst = SCL_IMG_V;
			}

			if (img_inst == SCL_IMG_D)
				in.d = SCL_INPUT_D_MEM;
			else
				in.v = SCL_INPUT_V_MEM;

			// one line: 8192 byte
			//   rgb packed: max height/width 2730, 2730 * 3 = 8190 < 8192
			sclr_ctrl_set_input(img_inst, in, img_in_fmt, img_in_csc);

			// CFG_OP_IMG_SET_MEM
			memset(&mem, 0, sizeof(mem));
			mem.width = i;
			mem.height = i;
			mem.start_x = 0;
			mem.start_y = 0;
			mem.addr0 = src_addr_scl[0][R_IDX];
			mem.addr1 = src_addr_scl[0][G_IDX];
			mem.addr2 = src_addr_scl[0][B_IDX];

			_get_bytesperline(img_in_fmt, i, pitch);
			mem.pitch_y = pitch[0];
			mem.pitch_c = pitch[1];

			sclr_img_set_mem(img_inst, &mem);

			// CFG_OP_SC_CFG
			sclr_set_cfg(inst, /*sc_bypass*/false, /*gop_bypass*/true, /*cir_bypass*/true, /*odma_bypass*/false);

			// CFG_OP_SC_SET_INPUT
			src.w = i;
			src.h = i;
			sclr_set_input_size(inst, src);

			// CFG_OP_SC_SET_CROP
			crop.x = 0;
			crop.y = 0;
			crop.w = src.w;
			crop.h = src.h;
			sclr_set_crop(inst, crop);
			sclr_set_scale(inst);

			// CFG_OP_SC_SET_OUTPUT
			dst.w = crop.w;
			dst.h = crop.h;
			sclr_set_output_size(inst, dst);
			sclr_set_scale(inst);

			// CFG_OP_ODMA_CFG
			odma_cfg = sclr_odma_get_cfg(inst);
			memset(odma_cfg, 0, sizeof(*odma_cfg));
			odma_cfg->csc_cfg.mode = SCL_OUT_CSC;
			odma_cfg->fmt = odma_fmt;
			sclr_odma_set_cfg(inst, odma_cfg);

			odma_cfg->csc_cfg.csc_type = odma_csc;

			sclr_ctrl_set_output(inst, &odma_cfg->csc_cfg, odma_cfg->fmt);

			// CFG_OP_ODMA_SET_MEM
			mem.width = dst.w;
			mem.height = dst.h;
			mem.start_x = 0;
			mem.start_y = 0;
			mem.addr0 = dst_addr_scl[0][R_IDX];
			mem.addr1 = dst_addr_scl[0][G_IDX];
			mem.addr2 = dst_addr_scl[0][B_IDX];

			_get_bytesperline(odma_cfg->fmt, dst.w, pitch);
			mem.pitch_y = pitch[0];
			mem.pitch_c = pitch[1];
			sclr_odma_set_mem(inst, &mem);

			sclr_top_reg_done();
			_intr_setup(inst, img_inst);

			// CFG_OP_IMG_START
			sclr_trig_start_ext(img_inst, inst);

			if (sclr_check_irq_handler(10000) != 0) {
				uartlog("img(%d), sc(%d), (w=h=%d), end intr doesn't happens after 10000 tick\n", img_inst, inst, src.w);
				ret = -1;
				break;
			} else {
				uartlog("img(%d), sc(%d), (w=h=%d), sclr done\n", img_inst, inst, src.w);
				uartlog("sc(%d) odma status(%#x)\n", inst, sclr_odma_get_dbg_status(inst).raw);
			}
		}

		if (ret)
			break;
	}

	return ret;
}

int sclr_max_size_test(int argc, char **argv)
{
	int ret = 0;
	int i;
	struct sclr_scale_cfg cfg;
	int max_size[4] = {1920, 2880, 1920, 1280};
	enum sclr_img_in img_inst;
	union sclr_input in;
	enum sclr_format fmt;
	enum sclr_csc csc = SCL_CSC_NONE;
	struct sclr_mem mem;
	u16 pitch[3];
	struct sclr_size src;
	struct sclr_size dst;
	struct sclr_rect crop;
	struct sclr_odma_cfg *odma_cfg;

	u8 inst = strtol(argv[1], NULL, 10);
	i = max_size[inst];

	memset(&cfg, 0, sizeof(cfg));

	cfg.src.w = i;
	cfg.src.h = i;
	cfg.crop.w = i;
	cfg.crop.h = i;
	cfg.dst.w = i;
	cfg.dst.h = i;

	sclr_ctrl_set_scale(inst, &cfg);
	sclr_set_scale_phase(inst, 0, 0);

	// CFG_OP_IMG_CFG
	if (inst == 0) {
		img_inst = SCL_IMG_D;
	} else {
		img_inst = SCL_IMG_V;
	}

	if (img_inst == SCL_IMG_D)
		in.d = 2; // MEM
	else
		in.v = 2; // MEM

	// one line: 8192 byte
	//   rgb packed: max height/width 2730, 2730 * 3 = 8190 < 8192
	fmt = SCL_FMT_BGR_PACKED;
	sclr_ctrl_set_input(img_inst, in, fmt, csc);

	// CFG_OP_IMG_SET_MEM
	memset(&mem, 0, sizeof(mem));
	mem.width = i;
	mem.height = i;
	mem.start_x = 0;
	mem.start_y = 0;
	mem.addr0 = src_addr_scl[0][R_IDX];
	mem.addr1 = src_addr_scl[0][G_IDX];
	mem.addr2 = src_addr_scl[0][B_IDX];

	_get_bytesperline(fmt, i, pitch);
	mem.pitch_y = pitch[0];
	mem.pitch_c = pitch[1];

	sclr_img_set_mem(img_inst, &mem);

	// CFG_OP_SC_CFG
	// RGB w=h=2752, no interrupt
	sclr_set_cfg(inst, /*sc_bypass*/false, /*gop_bypass*/true, /*cir_bypass*/true, /*odma_bypass*/false);

	// CFG_OP_SC_SET_INPUT
	src.w = i;
	src.h = i;
	sclr_set_input_size(inst, src);

	// CFG_OP_SC_SET_CROP
	crop.x = 0;
	crop.y = 0;
	crop.w = src.w;
	crop.h = src.h;
	sclr_set_crop(inst, crop);
	sclr_set_scale(inst);

	// CFG_OP_SC_SET_OUTPUT
	dst.w = crop.w;
	dst.h = crop.h;
	sclr_set_output_size(inst, dst);
	sclr_set_scale(inst);

	// CFG_OP_ODMA_CFG
	odma_cfg = sclr_odma_get_cfg(inst);
	memset(odma_cfg, 0, sizeof(*odma_cfg));
	odma_cfg->csc_cfg.mode = SCL_OUT_DISABLE;
	odma_cfg->fmt = SCL_FMT_BGR_PACKED;
	sclr_odma_set_cfg(inst, odma_cfg);

	odma_cfg->csc_cfg.csc_type = SCL_CSC_NONE;

	sclr_ctrl_set_output(inst, &odma_cfg->csc_cfg, odma_cfg->fmt);

	// CFG_OP_ODMA_SET_MEM
	mem.width = dst.w;
	mem.height = dst.h;
	mem.start_x = 0;
	mem.start_y = 0;
	mem.addr0 = dst_addr_scl[0][R_IDX];
	mem.addr1 = dst_addr_scl[0][G_IDX];
	mem.addr2 = dst_addr_scl[0][B_IDX];

	_get_bytesperline(odma_cfg->fmt, dst.w, pitch);
	mem.pitch_y = pitch[0];
	mem.pitch_c = pitch[1];
	sclr_odma_set_mem(inst, &mem);

	sclr_top_reg_done();
	_intr_setup(inst, img_inst);

	// CFG_OP_IMG_START
	sclr_trig_start_ext(img_inst, inst);

	if (sclr_check_irq_handler(10000) != 0) {
		uartlog("img(%d), sc(%d), (w=h=%d), end intr doesn't happens after 10000 tick\n", img_inst, inst, src.w);
		ret = -1;
	} else {
		uartlog("img(%d), sc(%d), (w=h=%d), sclr done\n", img_inst, inst, src.w);
		uartlog("sc-d-odma status(%#x)\n", sclr_odma_get_dbg_status(inst).raw);
	}

	return ret;
}

int sclr_fmt_test_1(int argc, char **argv)
{
	int ret = 0;
	struct sclr_scale_cfg cfg;
	u8 inst = 1; // sc_v1
	enum sclr_img_in img_inst = SCL_IMG_V;
	union sclr_input in;
	enum sclr_format fmt = SCL_FMT_BGR_PACKED;
	struct sclr_mem mem;
	u16 pitch[3];
	struct sclr_odma_cfg *odma_cfg;

	uartlog("0x90000000: 800-16.ppm_rgbp, 800x600 rgb packed\n");

	memset(&cfg, 0, sizeof(cfg));

	cfg.src.w = 800;
	cfg.src.h = 600;
	cfg.crop.w = 800;
	cfg.crop.h = 600;
	cfg.dst.w = 800;
	cfg.dst.h = 600;

	sclr_ctrl_set_scale(inst, &cfg);
	sclr_set_scale_phase(inst, 0, 0);

	// CFG_OP_IMG_CFG
	if (inst == 0) {
		img_inst = SCL_IMG_D;
	} else {
		img_inst = SCL_IMG_V;
	}

	if (img_inst == SCL_IMG_D)
		in.d = SCL_INPUT_D_MEM;
	else
		in.v = SCL_INPUT_V_MEM;

	// dram rgb packed -> sclr rgb
	sclr_ctrl_set_input(img_inst, in, fmt, SCL_CSC_NONE);

	// CFG_OP_IMG_SET_MEM,
	memset(&mem, 0, sizeof(mem));
	mem.width = cfg.src.w;
	mem.height = cfg.src.h;
	mem.start_x = 0;
	mem.start_y = 0;
	mem.addr0 = src_addr_scl[0][R_IDX];
	mem.addr1 = mem.addr0 + cfg.src.w * cfg.src.h;

	_get_bytesperline(fmt, cfg.src.w, pitch);
	mem.pitch_y = pitch[0];
	mem.pitch_c = pitch[1];
	sclr_img_set_mem(img_inst, &mem);

	// CFG_OP_SC_CFG
	sclr_set_cfg(inst, /*sc_bypass*/false, /*gop_bypass*/true, /*cir_bypass*/true, /*odma_bypass*/false);

	// CFG_OP_SC_SET_INPUT
	sclr_set_input_size(inst, cfg.src);

	// CFG_OP_SC_SET_CROP
	sclr_set_crop(inst, cfg.crop);
	sclr_set_scale(inst);

	// CFG_OP_SC_SET_OUTPUT
	sclr_set_output_size(inst, cfg.dst);
	sclr_set_scale(inst);

	// CFG_OP_ODMA_CFG, sclr rgb -> odma
	odma_cfg = sclr_odma_get_cfg(inst);
	memset(odma_cfg, 0, sizeof(*odma_cfg));
	odma_cfg->csc_cfg.mode = SCL_OUT_DISABLE;
	odma_cfg->fmt = fmt;
	odma_cfg->csc_cfg.csc_type = SCL_CSC_NONE;
	sclr_odma_set_cfg(inst, odma_cfg);
	sclr_ctrl_set_output(inst, &odma_cfg->csc_cfg, odma_cfg->fmt);

	// CFG_OP_ODMA_SET_MEM, rgb packed
	mem.width = cfg.dst.w;
	mem.height = cfg.dst.h;
	mem.start_x = 0;
	mem.start_y = 0;
	mem.addr0 = dst_addr_scl[0][R_IDX];

	_get_bytesperline(odma_cfg->fmt, cfg.dst.w, pitch);
	mem.pitch_y = pitch[0];
	mem.pitch_c = pitch[1];
	sclr_odma_set_mem(inst, &mem);

	sclr_top_reg_done();
	_intr_setup(inst, img_inst);

	// CFG_OP_IMG_START
	sclr_trig_start_ext(img_inst, inst);

	if (sclr_check_irq_handler(10000) != 0) {
		uartlog("img(%d), sc(%d), (w=%d, h=%d), end intr doesn't happens after 10000 tick\n", img_inst, inst, cfg.dst.w, cfg.dst.h);
		ret = -1;
	} else {
		uartlog("img(%d), sc(%d), (w=%d, h=%d), sclr done\n", img_inst, inst, cfg.dst.w, cfg.dst.h);
		uartlog("sc(%d) odma status(%#x)\n", inst, sclr_odma_get_dbg_status(inst).raw);
	}

	return ret;
}

int sclr_fmt_test_2(int argc, char **argv)
{
	int ret = 0;
	struct sclr_scale_cfg cfg;
	u8 inst = 1; // sc_v1
	enum sclr_img_in img_inst = SCL_IMG_V;
	union sclr_input in;
	enum sclr_format img_in_fmt = 9/*SCL_FMT_NV12*/; // SCL_FMT_NV12 == 8
	enum sclr_format odma_fmt = SCL_FMT_RGB_PACKED;
	enum sclr_csc img_in_csc = SCL_CSC_601_LIMIT_YUV2RGB;
	struct sclr_mem mem;
	u16 pitch[3];
	struct sclr_odma_cfg *odma_cfg;

	uartlog("0x90000000: 800-16.ppm_NV12_Y, 800x600 NV12 Y\n");
	uartlog("0x91000000: 800-16.ppm_NV12_C, 800x600 NV12 C\n");

	memset(&cfg, 0, sizeof(cfg));

	cfg.src.w = 800;
	cfg.src.h = 600;
	cfg.crop.w = 800;
	cfg.crop.h = 600;
	cfg.dst.w = 800;
	cfg.dst.h = 600;

	sclr_ctrl_set_scale(inst, &cfg);
	sclr_set_scale_phase(inst, 0, 0);

	// CFG_OP_IMG_CFG
	if (inst == 0) {
		img_inst = SCL_IMG_D;
	} else {
		img_inst = SCL_IMG_V;
	}

	if (img_inst == SCL_IMG_D)
		in.d = SCL_INPUT_D_MEM;
	else
		in.v = SCL_INPUT_V_MEM;

	// dram nv12 -> sclr rgb
	if (IS_YUV_FMT(img_in_fmt))
	++img_in_csc;
	sclr_ctrl_set_input(img_inst, in, img_in_fmt, img_in_csc);

	// CFG_OP_IMG_SET_MEM,
	memset(&mem, 0, sizeof(mem));
	mem.width = cfg.src.w;
	mem.height = cfg.src.h;
	mem.start_x = 0;
	mem.start_y = 0;
	mem.addr0 = src_addr_scl[0][R_IDX];
	mem.addr1 = src_addr_scl[0][G_IDX];

	_get_bytesperline(img_in_fmt, cfg.src.w, pitch);
	mem.pitch_y = pitch[0];
	mem.pitch_c = pitch[1];
	sclr_img_set_mem(img_inst, &mem);

	// CFG_OP_SC_CFG
	sclr_set_cfg(inst, /*sc_bypass*/false, /*gop_bypass*/true, /*cir_bypass*/true, /*odma_bypass*/false);

	// CFG_OP_SC_SET_INPUT
	sclr_set_input_size(inst, cfg.src);

	// CFG_OP_SC_SET_CROP
	sclr_set_crop(inst, cfg.crop);
	sclr_set_scale(inst);

	// CFG_OP_SC_SET_OUTPUT
	sclr_set_output_size(inst, cfg.dst);
	sclr_set_scale(inst);

	// CFG_OP_ODMA_CFG, sclr rgb -> odma
	odma_cfg = sclr_odma_get_cfg(inst);
	memset(odma_cfg, 0, sizeof(*odma_cfg));
	odma_cfg->csc_cfg.mode = SCL_OUT_DISABLE;
	odma_cfg->fmt = odma_fmt;
	odma_cfg->csc_cfg.csc_type = SCL_CSC_NONE;

	if (odma_cfg->csc_cfg.mode == SCL_OUT_CSC) {
		enum sclr_csc csc = SCL_CSC_NONE;
		if (IS_YUV_FMT(odma_cfg->fmt))
			csc += SCL_CSC_601_LIMIT_RGB2YUV;

		odma_cfg->csc_cfg.csc_type = csc;
	}

	sclr_odma_set_cfg(inst, odma_cfg);
	sclr_ctrl_set_output(inst, &odma_cfg->csc_cfg, odma_cfg->fmt);

	// CFG_OP_ODMA_SET_MEM, rgb packed
	mem.width = cfg.dst.w;
	mem.height = cfg.dst.h;
	mem.start_x = 0;
	mem.start_y = 0;
	mem.addr0 = dst_addr_scl[0][R_IDX];

	_get_bytesperline(odma_cfg->fmt, cfg.dst.w, pitch);
	mem.pitch_y = pitch[0];
	mem.pitch_c = pitch[1];
	sclr_odma_set_mem(inst, &mem);

	sclr_top_reg_done();
	_intr_setup(inst, img_inst);

	// CFG_OP_IMG_START
	sclr_trig_start_ext(img_inst, inst);

	if (sclr_check_irq_handler(10000) != 0) {
		uartlog("img(%d), sc(%d), (w=%d, h=%d), end intr doesn't happens after 10000 tick\n", img_inst, inst, cfg.dst.w, cfg.dst.h);
		ret = -1;
	} else {
		uartlog("img(%d), sc(%d), (w=%d, h=%d), sclr done\n", img_inst, inst, cfg.dst.w, cfg.dst.h);
		uartlog("sc(%d) odma status(%#x)\n", inst, sclr_odma_get_dbg_status(inst).raw);
	}

	return ret;
}

int sclr_fmt_test_3(int argc, char **argv)
{
	int ret = 0;
	struct sclr_scale_cfg cfg;
	u8 inst = 1; // sc_v1
	enum sclr_img_in img_inst = SCL_IMG_V;
	union sclr_input in;
	enum sclr_format fmt = SCL_FMT_BGR_PACKED;
	struct sclr_mem mem;
	u16 pitch[3];
	struct sclr_odma_cfg *odma_cfg;

	uartlog("0x90000000: 800-16.ppm_rgbp, 800x600 rgb packed\n");

	memset(&cfg, 0, sizeof(cfg));

	cfg.src.w = 800;
	cfg.src.h = 600;
	cfg.crop.w = 800;
	cfg.crop.h = 600;
	cfg.dst.w = 832;
	cfg.dst.h = 640;

	sclr_ctrl_set_scale(inst, &cfg);
	sclr_set_scale_phase(inst, 0, 0);

	// CFG_OP_IMG_CFG
	if (inst == 0) {
		img_inst = SCL_IMG_D;
	} else {
		img_inst = SCL_IMG_V;
	}

	if (img_inst == SCL_IMG_D)
		in.d = SCL_INPUT_D_MEM;
	else
		in.v = SCL_INPUT_V_MEM;

	// dram rgb packed -> sclr rgb
	sclr_ctrl_set_input(img_inst, in, fmt, SCL_CSC_NONE);

	// CFG_OP_IMG_SET_MEM
	memset(&mem, 0, sizeof(mem));
	mem.width = cfg.src.w;
	mem.height = cfg.src.h;
	mem.start_x = 0;
	mem.start_y = 0;
	mem.addr0 = src_addr_scl[0][R_IDX];
	mem.addr1 = mem.addr0 + cfg.src.w * cfg.src.h;

	_get_bytesperline(fmt, cfg.src.w, pitch);
	mem.pitch_y = pitch[0];
	mem.pitch_c = pitch[1];
	sclr_img_set_mem(img_inst, &mem);

	// CFG_OP_SC_CFG
	sclr_set_cfg(inst, /*sc_bypass*/false, /*gop_bypass*/true, /*cir_bypass*/true, /*odma_bypass*/false);

	// CFG_OP_SC_SET_INPUT
	sclr_set_input_size(inst, cfg.src);

	// CFG_OP_SC_SET_CROP
	sclr_set_crop(inst, cfg.crop);
	sclr_set_scale(inst);

	// CFG_OP_SC_SET_OUTPUT
	sclr_set_output_size(inst, cfg.dst);
	sclr_set_scale(inst);

	// CFG_OP_ODMA_CFG, sclr rgb -> odma
	odma_cfg = sclr_odma_get_cfg(inst);
	memset(odma_cfg, 0, sizeof(*odma_cfg));
	odma_cfg->csc_cfg.mode = SCL_OUT_DISABLE;
	odma_cfg->fmt = fmt;
	odma_cfg->csc_cfg.csc_type = SCL_CSC_NONE;
	sclr_odma_set_cfg(inst, odma_cfg);
	sclr_ctrl_set_output(inst, &odma_cfg->csc_cfg, odma_cfg->fmt);

	// CFG_OP_ODMA_SET_MEM, rgb packed
	mem.width = cfg.dst.w;
	mem.height = cfg.dst.h;
	mem.start_x = 0;
	mem.start_y = 0;
	mem.addr0 = dst_addr_scl[0][R_IDX];

	_get_bytesperline(odma_cfg->fmt, cfg.dst.w, pitch);
	mem.pitch_y = pitch[0];
	mem.pitch_c = pitch[1];
	sclr_odma_set_mem(inst, &mem);

	sclr_top_reg_done();
	_intr_setup(inst, img_inst);

	// CFG_OP_IMG_START
	sclr_trig_start_ext(img_inst, inst);

	if (sclr_check_irq_handler(10000) != 0) {
		uartlog("img(%d), sc(%d), (w=%d, h=%d), end intr doesn't happens after 10000 tick\n", img_inst, inst, cfg.dst.w, cfg.dst.h);
		ret = -1;
	} else {
		uartlog("img(%d), sc(%d), (w=%d, h=%d), sclr done\n", img_inst, inst, cfg.dst.w, cfg.dst.h);
		uartlog("sc(%d) odma status(%#x)\n", inst, sclr_odma_get_dbg_status(inst).raw);
	}

	return ret;
}

int sclr_fmt_test_4(int argc, char **argv)
{
	int ret = 0;
	struct sclr_scale_cfg cfg;
	u8 inst = 1; // sc_v1
	enum sclr_img_in img_inst = SCL_IMG_V;
	union sclr_input in;
	enum sclr_format src_fmt = SCL_FMT_BGR_PACKED;
	enum sclr_format dst_fmt = SCL_FMT_NV21;
	struct sclr_mem mem;
	u16 pitch[3];
	struct sclr_odma_cfg *odma_cfg;

	uartlog("0x90000000: 800-16.ppm_rgbp, 800x600 rgb packed, in\n");
	uartlog("0x96000000: 832x640 NV21 Y, out\n");
	uartlog("0x97000000: 832x640 NV21 U, out\n");

	memset(&cfg, 0, sizeof(cfg));

	cfg.src.w = 800;
	cfg.src.h = 600;
	cfg.crop.w = 800;
	cfg.crop.h = 600;
	cfg.dst.w = 832;
	cfg.dst.h = 640;

	sclr_ctrl_set_scale(inst, &cfg);
	sclr_set_scale_phase(inst, 0, 0);

	// CFG_OP_IMG_CFG
	if (inst == 0) {
		img_inst = SCL_IMG_D;
	} else {
		img_inst = SCL_IMG_V;
	}

	if (img_inst == SCL_IMG_D)
		in.d = SCL_INPUT_D_MEM;
	else
		in.v = SCL_INPUT_V_MEM;

	// dram rgb packed -> sclr rgb
	sclr_ctrl_set_input(img_inst, in, src_fmt, SCL_CSC_NONE);

	// CFG_OP_IMG_SET_MEM,
	memset(&mem, 0, sizeof(mem));
	mem.width = cfg.src.w;
	mem.height = cfg.src.h;
	mem.start_x = 0;
	mem.start_y = 0;
	mem.addr0 = src_addr_scl[0][R_IDX];
	mem.addr1 = mem.addr0 + cfg.src.w * cfg.src.h;

	_get_bytesperline(src_fmt, cfg.src.w, pitch);
	mem.pitch_y = pitch[0];
	mem.pitch_c = pitch[1];
	sclr_img_set_mem(img_inst, &mem);

	// CFG_OP_SC_CFG
	sclr_set_cfg(inst, /*sc_bypass*/false, /*gop_bypass*/true, /*cir_bypass*/true, /*odma_bypass*/false);

	// CFG_OP_SC_SET_INPUT
	sclr_set_input_size(inst, cfg.src);

	// CFG_OP_SC_SET_CROP
	sclr_set_crop(inst, cfg.crop);
	sclr_set_scale(inst);

	// CFG_OP_SC_SET_OUTPUT
	sclr_set_output_size(inst, cfg.dst);
	sclr_set_scale(inst);

	// CFG_OP_ODMA_CFG, sclr rgb -> odma
	odma_cfg = sclr_odma_get_cfg(inst);
	memset(odma_cfg, 0, sizeof(*odma_cfg));
	odma_cfg->csc_cfg.mode = SCL_OUT_CSC;
	odma_cfg->fmt = dst_fmt;
	odma_cfg->csc_cfg.csc_type = SCL_CSC_NONE;

	if (odma_cfg->csc_cfg.mode == SCL_OUT_CSC) {
		enum sclr_csc csc = SCL_CSC_NONE;
		if (IS_YUV_FMT(odma_cfg->fmt))
			csc = SCL_CSC_601_LIMIT_RGB2YUV;

		odma_cfg->csc_cfg.csc_type = csc;
	}

	sclr_odma_set_cfg(inst, odma_cfg);
	sclr_ctrl_set_output(inst, &odma_cfg->csc_cfg, odma_cfg->fmt);

	// CFG_OP_ODMA_SET_MEM, rgb packed
	mem.width = cfg.dst.w;
	mem.height = cfg.dst.h;
	mem.start_x = 0;
	mem.start_y = 0;
	mem.addr0 = dst_addr_scl[0][R_IDX];
	mem.addr1 = dst_addr_scl[0][G_IDX];

	_get_bytesperline(odma_cfg->fmt, cfg.dst.w, pitch);
	mem.pitch_y = pitch[0];
	mem.pitch_c = pitch[1];
	sclr_odma_set_mem(inst, &mem);

	sclr_top_reg_done();
	_intr_setup(inst, img_inst);

	// CFG_OP_IMG_START
	sclr_trig_start_ext(img_inst, inst);

	if (sclr_check_irq_handler(10000) != 0) {
		uartlog("img(%d), sc(%d), (w=%d, h=%d), end intr doesn't happens after 10000 tick\n", img_inst, inst, cfg.dst.w, cfg.dst.h);
		ret = -1;
	} else {
		uartlog("img(%d), sc(%d), (w=%d, h=%d), sclr done\n", img_inst, inst, cfg.dst.w, cfg.dst.h);
		uartlog("sc(%d) odma status(%#x)\n", inst, sclr_odma_get_dbg_status(inst).raw);
	}

	return ret;
}

int sclr_fmt_test_5(int argc, char **argv)
{
	int ret = 0;
	struct sclr_scale_cfg cfg;
	u8 inst = 1; // sc_v1
	enum sclr_img_in img_inst = SCL_IMG_V;
	union sclr_input in;
	enum sclr_format src_fmt = SCL_FMT_BGR_PACKED;
	enum sclr_format dst_fmt = SCL_FMT_NV21;
	struct sclr_mem mem;
	u16 pitch[3];
	struct sclr_odma_cfg *odma_cfg;

	uartlog("0x90000000: 800-16.ppm_rgbp, 800x600 rgb packed, in\n");
	uartlog("0x96000000: 800-16.ppm_nv21p, 800x600 NV21 packed, out\n");

	memset(&cfg, 0, sizeof(cfg));

	cfg.src.w = 800;
	cfg.src.h = 600;
	cfg.crop.w = cfg.src.w;
	cfg.crop.h = cfg.src.h;
	cfg.dst.w = cfg.src.w;
	cfg.dst.h = cfg.src.h;

	sclr_ctrl_set_scale(inst, &cfg);
	sclr_set_scale_phase(inst, 0, 0);

	// CFG_OP_IMG_CFG
	if (inst == 0) {
		img_inst = SCL_IMG_D;
	} else {
		img_inst = SCL_IMG_V;
	}

	if (img_inst == SCL_IMG_D)
		in.d = SCL_INPUT_D_MEM;
	else
		in.v = SCL_INPUT_V_MEM;

	// dram rgb packed -> sclr rgb
	sclr_ctrl_set_input(img_inst, in, src_fmt, SCL_CSC_NONE);

	// CFG_OP_IMG_SET_MEM,
	memset(&mem, 0, sizeof(mem));
	mem.width = cfg.src.w;
	mem.height = cfg.src.h;
	mem.start_x = 0;
	mem.start_y = 0;
	mem.addr0 = src_addr_scl[0][R_IDX];
	mem.addr1 = mem.addr0 + cfg.src.w * cfg.src.h;

	_get_bytesperline(src_fmt, cfg.src.w, pitch);
	mem.pitch_y = pitch[0];
	mem.pitch_c = pitch[1];
	sclr_img_set_mem(img_inst, &mem);

	// CFG_OP_SC_CFG
	sclr_set_cfg(inst, /*sc_bypass*/false, /*gop_bypass*/true, /*cir_bypass*/true, /*odma_bypass*/false);

	// CFG_OP_SC_SET_INPUT
	sclr_set_input_size(inst, cfg.src);

	// CFG_OP_SC_SET_CROP
	sclr_set_crop(inst, cfg.crop);
	sclr_set_scale(inst);

	// CFG_OP_SC_SET_OUTPUT
	sclr_set_output_size(inst, cfg.dst);
	sclr_set_scale(inst);

	// CFG_OP_ODMA_CFG, sclr rgb -> odma
	odma_cfg = sclr_odma_get_cfg(inst);
	memset(odma_cfg, 0, sizeof(*odma_cfg));
	odma_cfg->csc_cfg.mode = SCL_OUT_CSC;
	odma_cfg->fmt = dst_fmt;
	odma_cfg->csc_cfg.csc_type = SCL_CSC_NONE;

	if (odma_cfg->csc_cfg.mode == SCL_OUT_CSC) {
		enum sclr_csc csc = SCL_CSC_NONE;
		if (IS_YUV_FMT(odma_cfg->fmt))
			csc = SCL_CSC_601_LIMIT_RGB2YUV;

		odma_cfg->csc_cfg.csc_type = csc;
	}

	sclr_odma_set_cfg(inst, odma_cfg);
	sclr_ctrl_set_output(inst, &odma_cfg->csc_cfg, odma_cfg->fmt);

	// CFG_OP_ODMA_SET_MEM, rgb packed
	mem.width = cfg.dst.w;
	mem.height = cfg.dst.h;
	mem.start_x = 0;
	mem.start_y = 0;
	mem.addr0 = dst_addr_scl[0][R_IDX];
	mem.addr1 = mem.addr0 + mem.width * mem.height;

	_get_bytesperline(odma_cfg->fmt, cfg.dst.w, pitch);
	mem.pitch_y = pitch[0];
	mem.pitch_c = pitch[1];
	sclr_odma_set_mem(inst, &mem);

	sclr_top_reg_done();
	_intr_setup(inst, img_inst);

	// CFG_OP_IMG_START
	sclr_trig_start_ext(img_inst, inst);

	if (sclr_check_irq_handler(10000) != 0) {
		uartlog("img(%d), sc(%d), (w=%d, h=%d), end intr doesn't happens after 10000 tick\n", img_inst, inst, cfg.dst.w, cfg.dst.h);
		ret = -1;
	} else {
		uartlog("img(%d), sc(%d), (w=%d, h=%d), sclr done\n", img_inst, inst, cfg.dst.w, cfg.dst.h);
		uartlog("sc(%d) odma status(%#x)\n", inst, sclr_odma_get_dbg_status(inst).raw);
	}

	return ret;
}

int sclr_fmt_test_6(int argc, char **argv)
{
	int ret = 0;
	struct sclr_scale_cfg cfg;
	u8 inst = 1; // sc_v1
	enum sclr_img_in img_inst = SCL_IMG_V;
	union sclr_input in;
	enum sclr_format src_fmt = SCL_FMT_BGR_PACKED;
	enum sclr_format dst_fmt = SCL_FMT_NV12;
	struct sclr_mem mem;
	u16 pitch[3];
	struct sclr_odma_cfg *odma_cfg;

	uartlog("0x90000000: 800-16.ppm_rgbp, 800x600 rgb packed, in\n");
	uartlog("0x96000000: 800-16.ppm_nv12p, 800x600 NV12 packed, out\n");

	memset(&cfg, 0, sizeof(cfg));

	cfg.src.w = 800;
	cfg.src.h = 600;
	cfg.crop.w = cfg.src.w;
	cfg.crop.h = cfg.src.h;
	cfg.dst.w = cfg.src.w;
	cfg.dst.h = cfg.src.h;

	sclr_ctrl_set_scale(inst, &cfg);
	sclr_set_scale_phase(inst, 0, 0);

	// CFG_OP_IMG_CFG
	if (inst == 0) {
		img_inst = SCL_IMG_D;
	} else {
		img_inst = SCL_IMG_V;
	}

	if (img_inst == SCL_IMG_D)
		in.d = SCL_INPUT_D_MEM;
	else
		in.v = SCL_INPUT_V_MEM;

	// dram rgb packed -> sclr rgb
	sclr_ctrl_set_input(img_inst, in, src_fmt, SCL_CSC_NONE);

	// CFG_OP_IMG_SET_MEM,
	memset(&mem, 0, sizeof(mem));
	mem.width = cfg.src.w;
	mem.height = cfg.src.h;
	mem.start_x = 0;
	mem.start_y = 0;
	mem.addr0 = src_addr_scl[0][R_IDX];
	mem.addr1 = mem.addr0 + cfg.src.w * cfg.src.h;

	_get_bytesperline(src_fmt, cfg.src.w, pitch);
	mem.pitch_y = pitch[0];
	mem.pitch_c = pitch[1];
	sclr_img_set_mem(img_inst, &mem);

	// CFG_OP_SC_CFG
	sclr_set_cfg(inst, /*sc_bypass*/false, /*gop_bypass*/true, /*cir_bypass*/true, /*odma_bypass*/false);

	// CFG_OP_SC_SET_INPUT
	sclr_set_input_size(inst, cfg.src);

	// CFG_OP_SC_SET_CROP
	sclr_set_crop(inst, cfg.crop);
	sclr_set_scale(inst);

	// CFG_OP_SC_SET_OUTPUT
	sclr_set_output_size(inst, cfg.dst);
	sclr_set_scale(inst);

	// CFG_OP_ODMA_CFG
	odma_cfg = sclr_odma_get_cfg(inst);
	memset(odma_cfg, 0, sizeof(*odma_cfg));
	odma_cfg->csc_cfg.mode = SCL_OUT_CSC;
	odma_cfg->fmt = dst_fmt;
	odma_cfg->csc_cfg.csc_type = SCL_CSC_NONE;

	if (odma_cfg->csc_cfg.mode == SCL_OUT_CSC) {
		enum sclr_csc csc = SCL_CSC_NONE;
		if (IS_YUV_FMT(odma_cfg->fmt))
			csc = SCL_CSC_601_LIMIT_RGB2YUV;

		odma_cfg->csc_cfg.csc_type = csc;
	}

	sclr_odma_set_cfg(inst, odma_cfg);
	sclr_ctrl_set_output(inst, &odma_cfg->csc_cfg, odma_cfg->fmt);

	// CFG_OP_ODMA_SET_MEM
	mem.width = cfg.dst.w;
	mem.height = cfg.dst.h;
	mem.start_x = 0;
	mem.start_y = 0;
	mem.addr0 = dst_addr_scl[0][R_IDX];
	mem.addr1 = mem.addr0 + mem.width * mem.height;

	_get_bytesperline(odma_cfg->fmt, cfg.dst.w, pitch);
	mem.pitch_y = pitch[0];
	mem.pitch_c = pitch[1];
	sclr_odma_set_mem(inst, &mem);

	sclr_top_reg_done();
	_intr_setup(inst, img_inst);

	// CFG_OP_IMG_START
	sclr_trig_start_ext(img_inst, inst);

	if (sclr_check_irq_handler(10000) != 0) {
		uartlog("img(%d), sc(%d), (w=%d, h=%d), end intr doesn't happens after 10000 tick\n", img_inst, inst, cfg.dst.w, cfg.dst.h);
		ret = -1;
	} else {
		uartlog("img(%d), sc(%d), (w=%d, h=%d), sclr done\n", img_inst, inst, cfg.dst.w, cfg.dst.h);
		uartlog("sc(%d) odma status(%#x)\n", inst, sclr_odma_get_dbg_status(inst).raw);
	}

	return ret;
}

int sclr_fmt_test_7(int argc, char **argv)
{
	int ret = 0;
	struct sclr_scale_cfg cfg;
	u8 inst = 1; // sc_v1
	enum sclr_img_in img_inst = SCL_IMG_V;
	union sclr_input in;
	enum sclr_format src_fmt = SCL_FMT_BGR_PACKED;
	enum sclr_format dst_fmt = SCL_FMT_YUV420;
	struct sclr_mem mem;
	u16 pitch[3];
	struct sclr_odma_cfg *odma_cfg;

	uartlog("0x90000000: 800-16.ppm_rgbp, 800x600 rgb packed, in\n");
	uartlog("0x96000000: 800x600 YUV420, Y, out\n");
	uartlog("0x97000000: 800x600 YUV420, U, w=400, pitch=416(aligned 32), out\n");
	uartlog("0x98000000: 800x600 YUV420, V, w=400, pitch=416(aligned 32), out\n");
	uartlog("rawViewer.py: yuv420, w=800, h=600, alignment=32, fpga\n");

	memset(&cfg, 0, sizeof(cfg));

	cfg.src.w = 800;
	cfg.src.h = 600;
	cfg.crop.w = cfg.src.w;
	cfg.crop.h = cfg.src.h;
	cfg.dst.w = cfg.src.w;
	cfg.dst.h = cfg.src.h;

	sclr_ctrl_set_scale(inst, &cfg);
	sclr_set_scale_phase(inst, 0, 0);

	// CFG_OP_IMG_CFG
	if (inst == 0) {
		img_inst = SCL_IMG_D;
	} else {
		img_inst = SCL_IMG_V;
	}

	if (img_inst == SCL_IMG_D)
		in.d = SCL_INPUT_D_MEM;
	else
		in.v = SCL_INPUT_V_MEM;

	// dram rgb packed -> sclr rgb
	sclr_ctrl_set_input(img_inst, in, src_fmt, SCL_CSC_NONE);

	// CFG_OP_IMG_SET_MEM
	memset(&mem, 0, sizeof(mem));
	_get_bytesperline(src_fmt, cfg.src.w, pitch);
	mem.pitch_y = pitch[0];
	mem.pitch_c = pitch[1];

	mem.width = cfg.src.w;
	mem.height = cfg.src.h;
	mem.start_x = 0;
	mem.start_y = 0;
	mem.addr0 = src_addr_scl[0][R_IDX];

	sclr_img_set_mem(img_inst, &mem);

	// CFG_OP_SC_CFG
	sclr_set_cfg(inst, /*sc_bypass*/false, /*gop_bypass*/true, /*cir_bypass*/true, /*odma_bypass*/false);

	// CFG_OP_SC_SET_INPUT
	sclr_set_input_size(inst, cfg.src);

	// CFG_OP_SC_SET_CROP
	sclr_set_crop(inst, cfg.crop);
	sclr_set_scale(inst);

	// CFG_OP_SC_SET_OUTPUT
	sclr_set_output_size(inst, cfg.dst);
	sclr_set_scale(inst);

	// CFG_OP_ODMA_CFG, sclr rgb -> odma
	odma_cfg = sclr_odma_get_cfg(inst);
	memset(odma_cfg, 0, sizeof(*odma_cfg));
	odma_cfg->csc_cfg.mode = SCL_OUT_CSC;
	odma_cfg->fmt = dst_fmt;
	odma_cfg->csc_cfg.csc_type = SCL_CSC_NONE;

	if (odma_cfg->csc_cfg.mode == SCL_OUT_CSC) {
		enum sclr_csc csc = SCL_CSC_NONE;
		if (IS_YUV_FMT(odma_cfg->fmt))
			csc = SCL_CSC_601_LIMIT_RGB2YUV;

		odma_cfg->csc_cfg.csc_type = csc;
	}

	sclr_odma_set_cfg(inst, odma_cfg);
	sclr_ctrl_set_output(inst, &odma_cfg->csc_cfg, odma_cfg->fmt);

	// CFG_OP_ODMA_SET_MEM
	mem.width = cfg.dst.w;
	mem.height = cfg.dst.h;
	mem.start_x = 0;
	mem.start_y = 0;
	mem.addr0 = dst_addr_scl[0][R_IDX];
	mem.addr1 = dst_addr_scl[0][G_IDX];
	mem.addr2 = dst_addr_scl[0][B_IDX];

	_get_bytesperline(odma_cfg->fmt, cfg.dst.w, pitch);
	mem.pitch_y = pitch[0];
	mem.pitch_c = pitch[1];
	sclr_odma_set_mem(inst, &mem);

	sclr_top_reg_done();
	_intr_setup(inst, img_inst);

	// CFG_OP_IMG_START
	sclr_trig_start_ext(img_inst, inst);

	if (sclr_check_irq_handler(10000) != 0) {
		uartlog("img(%d), sc(%d), (w=%d, h=%d), end intr doesn't happens after 10000 tick\n", img_inst, inst, cfg.dst.w, cfg.dst.h);
		ret = -1;
	} else {
		uartlog("img(%d), sc(%d), (w=%d, h=%d), sclr done\n", img_inst, inst, cfg.dst.w, cfg.dst.h);
		uartlog("sc(%d) odma status(%#x)\n", inst, sclr_odma_get_dbg_status(inst).raw);
	}

	return ret;
}

int sclr_fmt_test_9(int argc, char **argv)
{
	int ret = 0;
	struct sclr_scale_cfg cfg;
	u8 inst = 1; // sc_v1
	enum sclr_img_in img_inst = SCL_IMG_V;
	union sclr_input in;
	enum sclr_format src_fmt = SCL_FMT_YUV420;
	enum sclr_format dst_fmt = SCL_FMT_RGB_PACKED;
	struct sclr_mem mem;
	u16 pitch[3];
	struct sclr_odma_cfg *odma_cfg;

	uartlog("0x90000000: 2k-16.ppm_Y20, 1920x1080, YUV420, Y, in\n");
	uartlog("0x91000000: 2k-16.ppm_U20, 1920x1080, YUV420, U, in\n");
	uartlog("0x92000000: 2k-16.ppm_V20, 1920x1080, YUV420, V, in\n");
	uartlog("0x96000000: 1920x1080, rgb packed, out\n");

	memset(&cfg, 0, sizeof(cfg));

	cfg.src.w = 1920;
	cfg.src.h = 1080;
	cfg.crop.w = cfg.src.w;
	cfg.crop.h = cfg.src.h;
	cfg.dst.w = cfg.src.w;
	cfg.dst.h = cfg.src.h;

	sclr_ctrl_set_scale(inst, &cfg);
	sclr_set_scale_phase(inst, 0, 0);

	// CFG_OP_IMG_CFG
	if (inst == 0) {
		img_inst = SCL_IMG_D;
	} else {
		img_inst = SCL_IMG_V;
	}

	if (img_inst == SCL_IMG_D)
		in.d = SCL_INPUT_D_MEM;
	else
		in.v = SCL_INPUT_V_MEM;

	// dram -> sclr rgb
	sclr_ctrl_set_input(img_inst, in, src_fmt, SCL_CSC_601_LIMIT_YUV2RGB);

	// CFG_OP_IMG_SET_MEM
	memset(&mem, 0, sizeof(mem));
	_get_bytesperline(src_fmt, cfg.src.w, pitch);
	mem.pitch_y = pitch[0];
	mem.pitch_c = pitch[1];

	mem.width = cfg.src.w;
	mem.height = cfg.src.h;
	mem.start_x = 0;
	mem.start_y = 0;
	mem.addr0 = src_addr_scl[0][R_IDX];
	mem.addr1 = src_addr_scl[0][G_IDX];
	mem.addr2 = src_addr_scl[0][B_IDX];

	sclr_img_set_mem(img_inst, &mem);

	// CFG_OP_SC_CFG
	sclr_set_cfg(inst, /*sc_bypass*/false, /*gop_bypass*/true, /*cir_bypass*/true, /*odma_bypass*/false);

	// CFG_OP_SC_SET_INPUT
	sclr_set_input_size(inst, cfg.src);

	// CFG_OP_SC_SET_CROP
	sclr_set_crop(inst, cfg.crop);
	sclr_set_scale(inst);

	// CFG_OP_SC_SET_OUTPUT
	sclr_set_output_size(inst, cfg.dst);
	sclr_set_scale(inst);

	// CFG_OP_ODMA_CFG, sclr rgb -> odma
	odma_cfg = sclr_odma_get_cfg(inst);
	memset(odma_cfg, 0, sizeof(*odma_cfg));
	odma_cfg->csc_cfg.mode = SCL_OUT_DISABLE;
	odma_cfg->fmt = dst_fmt;
	odma_cfg->csc_cfg.csc_type = SCL_CSC_NONE;

	if (odma_cfg->csc_cfg.mode == SCL_OUT_CSC) {
		enum sclr_csc csc = SCL_CSC_NONE;
		if (IS_YUV_FMT(odma_cfg->fmt))
			csc = SCL_CSC_601_LIMIT_RGB2YUV;

		odma_cfg->csc_cfg.csc_type = csc;
	}

	sclr_odma_set_cfg(inst, odma_cfg);
	sclr_ctrl_set_output(inst, &odma_cfg->csc_cfg, odma_cfg->fmt);

	// CFG_OP_ODMA_SET_MEM
	mem.width = cfg.dst.w;
	mem.height = cfg.dst.h;
	mem.start_x = 0;
	mem.start_y = 0;
	mem.addr0 = dst_addr_scl[0][R_IDX];
	mem.addr1 = dst_addr_scl[0][G_IDX];
	mem.addr2 = dst_addr_scl[0][B_IDX];

	_get_bytesperline(odma_cfg->fmt, cfg.dst.w, pitch);
	mem.pitch_y = pitch[0];
	mem.pitch_c = pitch[1];
	sclr_odma_set_mem(inst, &mem);

	sclr_top_reg_done();
	_intr_setup(inst, img_inst);

	// CFG_OP_IMG_START
	sclr_trig_start_ext(img_inst, inst);

	if (sclr_check_irq_handler(10000) != 0) {
		uartlog("img(%d), sc(%d), (w=%d, h=%d), end intr doesn't happens after 10000 tick\n", img_inst, inst, cfg.dst.w, cfg.dst.h);
		ret = -1;
	} else {
		uartlog("img(%d), sc(%d), (w=%d, h=%d), sclr done\n", img_inst, inst, cfg.dst.w, cfg.dst.h);
		uartlog("sc(%d) odma status(%#x)\n", inst, sclr_odma_get_dbg_status(inst).raw);
	}

	return ret;
}

int sclr_fmt_test_8(int argc, char **argv)
{
	int ret = 0;
	struct sclr_scale_cfg cfg;
	u8 inst = 1; // sc_v1
	enum sclr_img_in img_inst = SCL_IMG_V;
	union sclr_input in;
	enum sclr_format src_fmt = SCL_FMT_YUV420;
	enum sclr_format dst_fmt = SCL_FMT_YUV420;
	struct sclr_mem mem;
	u16 pitch[3];
	struct sclr_odma_cfg *odma_cfg;

	uartlog("0x90000000: 2k-16.ppm_Y20, 1920x1080, YUV420, Y, in\n");
	uartlog("0x91000000: 2k-16.ppm_U20, 1920x1080, YUV420, U, in\n");
	uartlog("0x92000000: 2k-16.ppm_V20, 1920x1080, YUV420, V, in\n");
	uartlog("0x96000000: 1920x1080 YUV420, Y, out\n");
	uartlog("0x97000000: 1920x1080 YUV420, U, out\n");
	uartlog("0x98000000: 1920x1080 YUV420, V, out\n");

	memset(&cfg, 0, sizeof(cfg));

	cfg.src.w = 1920;
	cfg.src.h = 1080;
	cfg.crop.w = cfg.src.w;
	cfg.crop.h = cfg.src.h;
	cfg.dst.w = cfg.src.w;
	cfg.dst.h = cfg.src.h;

	sclr_ctrl_set_scale(inst, &cfg);
	sclr_set_scale_phase(inst, 0, 0);

	// CFG_OP_IMG_CFG
	if (inst == 0) {
		img_inst = SCL_IMG_D;
	} else {
		img_inst = SCL_IMG_V;
	}

	if (img_inst == SCL_IMG_D)
		in.d = SCL_INPUT_D_MEM;
	else
		in.v = SCL_INPUT_V_MEM;

	// dram -> sclr rgb
	sclr_ctrl_set_input(img_inst, in, src_fmt, SCL_CSC_601_LIMIT_YUV2RGB);

	// CFG_OP_IMG_SET_MEM
	memset(&mem, 0, sizeof(mem));
	_get_bytesperline(src_fmt, cfg.src.w, pitch);
	mem.pitch_y = pitch[0];
	mem.pitch_c = pitch[1];

	mem.width = cfg.src.w;
	mem.height = cfg.src.h;
	mem.start_x = 0;
	mem.start_y = 0;
	mem.addr0 = src_addr_scl[0][R_IDX];
	mem.addr1 = src_addr_scl[0][G_IDX];
	mem.addr2 = src_addr_scl[0][B_IDX];

	sclr_img_set_mem(img_inst, &mem);

	// CFG_OP_SC_CFG
	sclr_set_cfg(inst, /*sc_bypass*/false, /*gop_bypass*/true, /*cir_bypass*/true, /*odma_bypass*/false);

	// CFG_OP_SC_SET_INPUT
	sclr_set_input_size(inst, cfg.src);

	// CFG_OP_SC_SET_CROP
	sclr_set_crop(inst, cfg.crop);
	sclr_set_scale(inst);

	// CFG_OP_SC_SET_OUTPUT
	sclr_set_output_size(inst, cfg.dst);
	sclr_set_scale(inst);

	// CFG_OP_ODMA_CFG, sclr rgb -> odma
	odma_cfg = sclr_odma_get_cfg(inst);
	memset(odma_cfg, 0, sizeof(*odma_cfg));
	odma_cfg->csc_cfg.mode = SCL_OUT_CSC;
	odma_cfg->fmt = dst_fmt;
	odma_cfg->csc_cfg.csc_type = SCL_CSC_NONE;

	if (odma_cfg->csc_cfg.mode == SCL_OUT_CSC) {
		enum sclr_csc csc = SCL_CSC_NONE;
		if (IS_YUV_FMT(odma_cfg->fmt))
			csc = SCL_CSC_601_LIMIT_RGB2YUV;

		odma_cfg->csc_cfg.csc_type = csc;
	}

	sclr_odma_set_cfg(inst, odma_cfg);
	sclr_ctrl_set_output(inst, &odma_cfg->csc_cfg, odma_cfg->fmt);

	// CFG_OP_ODMA_SET_MEM
	mem.width = cfg.dst.w;
	mem.height = cfg.dst.h;
	mem.start_x = 0;
	mem.start_y = 0;
	mem.addr0 = dst_addr_scl[0][R_IDX];
	mem.addr1 = dst_addr_scl[0][G_IDX];
	mem.addr2 = dst_addr_scl[0][B_IDX];

	_get_bytesperline(odma_cfg->fmt, cfg.dst.w, pitch);
	mem.pitch_y = pitch[0];
	mem.pitch_c = pitch[1];
	sclr_odma_set_mem(inst, &mem);

	sclr_top_reg_done();
	_intr_setup(inst, img_inst);

	// CFG_OP_IMG_START
	sclr_trig_start_ext(img_inst, inst);

	if (sclr_check_irq_handler(10000) != 0) {
		uartlog("img(%d), sc(%d), (w=%d, h=%d), end intr doesn't happens after 10000 tick\n", img_inst, inst, cfg.dst.w, cfg.dst.h);
		ret = -1;
	} else {
		uartlog("img(%d), sc(%d), (w=%d, h=%d), sclr done\n", img_inst, inst, cfg.dst.w, cfg.dst.h);
		uartlog("sc(%d) odma status(%#x)\n", inst, sclr_odma_get_dbg_status(inst).raw);
	}

	return ret;
}

// 320x256 nv21 packed -> 320x256 nv21 packed
int sclr_fmt_test_10(int argc, char **argv)
{
	int ret = 0;
	struct sclr_scale_cfg cfg;
	u8 inst = 1; // sc_v1
	enum sclr_img_in img_inst = SCL_IMG_V;
	union sclr_input in;
	enum sclr_format src_fmt = SCL_FMT_NV12;
	enum sclr_format dst_fmt = SCL_FMT_NV12;
	struct sclr_mem mem;
	u16 pitch[3];
	struct sclr_odma_cfg *odma_cfg;

	uartlog("0x90000000: 800-16.ppm_2_320_256_nv21p.bin, 320x256 NV21 packed, in\n");
	uartlog("0x96000000: 320x256 NV21 packed, out\n");

	memset(&cfg, 0, sizeof(cfg));
	cfg.src.w = 320;
	cfg.src.h = 256;
	cfg.crop.w = cfg.src.w;
	cfg.crop.h = cfg.src.h;
	cfg.dst.w = cfg.crop.w;
	cfg.dst.h = cfg.crop.h;

	sclr_ctrl_set_scale(inst, &cfg);
	sclr_set_scale_phase(inst, 0, 0);

	// CFG_OP_IMG_CFG
	if (inst == 0) {
		img_inst = SCL_IMG_D;
	} else {
		img_inst = SCL_IMG_V;
	}

	if (img_inst == SCL_IMG_D)
		in.d = SCL_INPUT_D_MEM;
	else
		in.v = SCL_INPUT_V_MEM;

	// dram nv12 -> sclr rgb
	sclr_ctrl_set_input(img_inst, in, src_fmt, SCL_CSC_601_LIMIT_YUV2RGB);

	// CFG_OP_IMG_SET_MEM,
	memset(&mem, 0, sizeof(mem));
	mem.width = cfg.src.w;
	mem.height = cfg.src.h;
	mem.start_x = 0;
	mem.start_y = 0;
	mem.addr0 = src_addr_scl[0][R_IDX];
	mem.addr1 = mem.addr0 + cfg.src.w * cfg.src.h;

	_get_bytesperline(src_fmt, cfg.src.w, pitch);
	mem.pitch_y = pitch[0];
	mem.pitch_c = pitch[1];
	sclr_img_set_mem(img_inst, &mem);

	// CFG_OP_SC_CFG
	sclr_set_cfg(inst, /*sc_bypass*/false, /*gop_bypass*/true, /*cir_bypass*/true, /*odma_bypass*/false);

	// CFG_OP_SC_SET_INPUT
	sclr_set_input_size(inst, cfg.src);

	// CFG_OP_SC_SET_CROP
	sclr_set_crop(inst, cfg.crop);
	sclr_set_scale(inst);

	// CFG_OP_SC_SET_OUTPUT
	sclr_set_output_size(inst, cfg.dst);
	sclr_set_scale(inst);

	// CFG_OP_ODMA_CFG, sclr rgb -> odma
	odma_cfg = sclr_odma_get_cfg(inst);
	memset(odma_cfg, 0, sizeof(*odma_cfg));
	odma_cfg->csc_cfg.mode = SCL_OUT_CSC;
	odma_cfg->fmt = dst_fmt;
	odma_cfg->csc_cfg.csc_type = SCL_CSC_NONE;

	if (odma_cfg->csc_cfg.mode == SCL_OUT_CSC) {
		enum sclr_csc csc = SCL_CSC_NONE;
		if (IS_YUV_FMT(odma_cfg->fmt))
			csc = SCL_CSC_601_LIMIT_RGB2YUV;

		odma_cfg->csc_cfg.csc_type = csc;
	}

	sclr_odma_set_cfg(inst, odma_cfg);
	sclr_ctrl_set_output(inst, &odma_cfg->csc_cfg, odma_cfg->fmt);

	// CFG_OP_ODMA_SET_MEM, nv12 packed
	mem.width = cfg.dst.w;
	mem.height = cfg.dst.h;
	mem.start_x = 0;
	mem.start_y = 0;
	mem.addr0 = dst_addr_scl[0][R_IDX];
	mem.addr1 = mem.addr0 + mem.width * mem.height;

	_get_bytesperline(odma_cfg->fmt, cfg.dst.w, pitch);
	mem.pitch_y = pitch[0];
	mem.pitch_c = pitch[1];
	sclr_odma_set_mem(inst, &mem);

	sclr_top_reg_done();
	_intr_setup(inst, img_inst);

	// CFG_OP_IMG_START
	sclr_trig_start_ext(img_inst, inst);

	if (sclr_check_irq_handler(10000) != 0) {
		uartlog("img(%d), sc(%d), (w=%d, h=%d), end intr doesn't happens after 10000 tick\n", img_inst, inst, cfg.dst.w, cfg.dst.h);
		ret = -1;
	} else {
		uartlog("img(%d), sc(%d), (w=%d, h=%d), sclr done\n", img_inst, inst, cfg.dst.w, cfg.dst.h);
		uartlog("sc(%d) odma status(%#x)\n", inst, sclr_odma_get_dbg_status(inst).raw);
	}

	return ret;
}

int sclr_disp_test(int argc, char **arg)
{
	int ret = 0;
	u8 inst = 0; // sc_v0
	u32 tmp;
	u16 pitch[3];
	enum sclr_img_in img_inst = SCL_IMG_D;
	struct sclr_scale_cfg cfg;
	union sclr_input in;
	enum sclr_format src_fmt = SCL_FMT_RGB_PACKED;
	enum sclr_format dst_fmt = SCL_FMT_RGB_PACKED;
	u64 img_in_src_addr = src_addr_scl[0][R_IDX];
	struct sclr_mem mem;
	struct sclr_odma_cfg *odma_cfg;
	struct sclr_top_cfg *top_cfg;

	uartlog("Please run 42) disp_tgen, 43) lvdstx first\n");
	uartlog("0x90000000: 800-16.ppm_rgbp, 800x600 rgb packed\n");

#if 0
	// CFG_OP_DISP_TGEN
	sclr_disp_tgen_enable(1);

	// CFG_OP_LVDSTX
	//union sclr_lvdstx lvdstx_cfg;
	sclr_lvdstx_get(&lvdstx_cfg);
	lvdstx_cfg.b.en = 1;
	if (lvdstx_cfg.b.en) {
		lvdstx_cfg.b.out_bit = 0;
		lvdstx_cfg.b.vesa_mode = 1;
		lvdstx_cfg.b.dual_ch = 0;
		lvdstx_cfg.b.vs_out_en = 1;
		lvdstx_cfg.b.hs_out_en = 1;
		lvdstx_cfg.b.hs_blk_en = 1;
		lvdstx_cfg.b.ml_swap = 1;
		lvdstx_cfg.b.ctrl_rev = 0;
		lvdstx_cfg.b.oe_swap = 0;
		_reg_write(REG_DSI_PHY_PD, 0x1f1f);
		dphy_lvds_enable(true);
	}
	sclr_lvdstx_set(lvdstx_cfg);
#endif

	// Scaler
	memset(&cfg, 0, sizeof(cfg));
	cfg.src.w = 800;
	cfg.src.h = 600;
	cfg.crop.w = cfg.src.w;
	cfg.crop.h = cfg.src.h;
	cfg.dst.w = cfg.src.w;
	cfg.dst.h = cfg.src.h;

	sclr_ctrl_set_scale(inst, &cfg);
	sclr_set_scale_phase(inst, 0, 0);

	// CFG_OP_IMG_CFG
	if (img_inst == SCL_IMG_D)
		in.d = SCL_INPUT_D_MEM;
	else
		in.v = SCL_INPUT_V_MEM;

	// dram rgbp -> sclr rgb
	sclr_ctrl_set_input(img_inst, in, src_fmt, SCL_CSC_NONE);

	// CFG_OP_IMG_SET_MEM
	memset(&mem, 0, sizeof(mem));
	mem.width = cfg.src.w;
	mem.height = cfg.src.h;
	mem.start_x = 0;
	mem.start_y = 0;
	mem.addr0 = img_in_src_addr;
	mem.addr1 = mem.addr0 + cfg.src.w * cfg.src.h;

	_get_bytesperline(src_fmt, cfg.src.w, pitch);
	mem.pitch_y = pitch[0];
	mem.pitch_c = pitch[1];
	sclr_img_set_mem(img_inst, &mem);

	// CFG_OP_SC_CFG, odma disabled
	sclr_set_cfg(inst, /*sc_bypass*/false, /*gop_bypass*/true, /*cir_bypass*/true, /*odma_bypass*/true);

	// CFG_OP_SC_SET_INPUT
	sclr_set_input_size(inst, cfg.src);

	// CFG_OP_SC_SET_CROP
	sclr_set_crop(inst, cfg.crop);
	sclr_set_scale(inst);

	// CFG_OP_SC_SET_OUTPUT
	sclr_set_output_size(inst, cfg.dst);
	sclr_set_scale(inst);

	// CFG_OP_ODMA_CFG, odma disabled in when display used
	odma_cfg = sclr_odma_get_cfg(inst);
	memset(odma_cfg, 0, sizeof(*odma_cfg));
	odma_cfg->csc_cfg.mode = SCL_OUT_DISABLE;
	odma_cfg->fmt = dst_fmt;
	odma_cfg->csc_cfg.csc_type = SCL_CSC_NONE;
	sclr_odma_set_cfg(inst, odma_cfg);
	sclr_ctrl_set_output(inst, &odma_cfg->csc_cfg, odma_cfg->fmt);

	// CFG_OP_ODMA_SET_MEM
	mem.width = cfg.dst.w;
	mem.height = cfg.dst.h;
	mem.start_x = 0;
	mem.start_y = 0;
	mem.addr0 = dst_addr_scl[0][R_IDX];

	_get_bytesperline(odma_cfg->fmt, cfg.dst.w, pitch);
	mem.pitch_y = pitch[0];
	mem.pitch_c = pitch[1];
	sclr_odma_set_mem(inst, &mem);

	if (0 == inst) {
		struct sclr_disp_cfg *disp_cfg = sclr_disp_get_cfg();
		disp_cfg->fmt = odma_cfg->fmt;
		if (odma_cfg->csc_cfg.csc_type != SCL_CSC_NONE) disp_cfg->in_csc = odma_cfg->csc_cfg.csc_type - SCL_CSC_601_LIMIT_RGB2YUV + 1;
		disp_cfg->mem = mem;
		sclr_disp_set_cfg(disp_cfg);

		sclr_ctrl_set_disp_src(/*disp_from_sc*/true);
	}

	sclr_top_reg_done();
	_intr_setup(inst, img_inst);

	// No interrupt log
	sclr_enable_intr_log(0);

	// CFG_OP_IMG_START
	sclr_trig_start_ext(img_inst, inst);

	if (sclr_check_irq_handler(10000) != 0) {
		uartlog("img(%d), sc(%d), (w=%d, h=%d), end intr doesn't happens after 10000 tick\n", img_inst, inst, cfg.dst.w, cfg.dst.h);
		ret = -1;

		sclr_dump_top_register();
		sclr_dump_img_in_register(img_inst);
		sclr_dump_core_register(inst);
		sclr_dump_odma_register(inst);
		sclr_dump_disp_register();
	} else {
		uartlog("img(%d), sc(%d), (w=%d, h=%d), sclr done\n", img_inst, inst, cfg.dst.w, cfg.dst.h);
	}

	bm_get_num("Press any key to stop", tmp);

	// Disable scaler
	top_cfg = sclr_top_get_cfg();
	top_cfg->sclr_enable[0] = 0;
	sclr_top_set_cfg(top_cfg);

	sclr_enable_intr_log(1);

	return ret;
}

// trigger img_in_d and img_in_v at the same time.
int sclr_dual_img_test(int argc, char **argv)
{
	int ret = 0;

	u8 sc_inst_d = 0; // sc_d
	u8 sc_inst_v = 1; // sc_v1
	enum sclr_img_in img_inst_d = SCL_IMG_D;
	enum sclr_img_in img_inst_v = SCL_IMG_V;
	union sclr_input in;
	enum sclr_format src_fmt_d = SCL_FMT_BGR_PACKED;
	enum sclr_format src_fmt_v = SCL_FMT_NV12;
	enum sclr_format dst_fmt_d = src_fmt_d;
	enum sclr_format dst_fmt_v = src_fmt_v;
	struct sclr_scale_cfg sc_cfg;
	struct sclr_odma_cfg *odma_cfg;
	struct sclr_top_cfg *top_cfg;
	struct sclr_mem mem;
	u16 pitch[3];
	struct sclr_img_checksum_status img_chksum_status;
	struct sclr_core_checksum_status sc_chsum_status;

	uartlog("0x%08x: 800-16.ppm_2_320_256_nv21p.bin, 320x256 nv21 packed, in\n", src_addr_scl[0][R_IDX]);
	uartlog("0x%08x: 800-16.ppm_rgbp, 800x600 rgb packed, in\n", src_addr_scl[0][G_IDX]);
	uartlog("0x%08x: 320x256 NV21 packed, out\n", dst_addr_scl[0][R_IDX]);
	uartlog("0x%08x: 800x600 rgb packed, out\n", dst_addr_scl[0][G_IDX]);

	// 7yuv YUV420 planar NV12

	//////////////////////////////////////
	// 
	// img_in_d + sc_d
	//
	memset(&sc_cfg, 0, sizeof(sc_cfg));
	sc_cfg.src.w = 800;
	sc_cfg.src.h = 600;
	sc_cfg.crop.w = sc_cfg.src.w;
	sc_cfg.crop.h = sc_cfg.src.h;
	sc_cfg.dst.w = sc_cfg.src.w;
	sc_cfg.dst.h = sc_cfg.src.h;
	sclr_ctrl_set_scale(sc_inst_d, &sc_cfg);
	sclr_set_scale_phase(sc_inst_d, 0, 0);

	// CFG_OP_IMG_CFG, img_in_d
	in.d = SCL_INPUT_D_MEM;
	sclr_ctrl_set_input(img_inst_d, in, src_fmt_d, SCL_CSC_NONE);

	// CFG_OP_IMG_SET_MEM
	memset(&mem, 0, sizeof(mem));
	mem.width = sc_cfg.src.w;
	mem.height = sc_cfg.src.h;
	mem.start_x = 0;
	mem.start_y = 0;
	mem.addr0 = src_addr_scl[0][G_IDX];

	_get_bytesperline(src_fmt_d, sc_cfg.src.w, pitch);
	mem.pitch_y = pitch[0];
	mem.pitch_c = pitch[1];
	sclr_img_set_mem(img_inst_d, &mem);

	// CFG_OP_SC_CFG
	sclr_set_cfg(sc_inst_d, /*sc_bypass*/false, /*gop_bypass*/true, /*cir_bypass*/true, /*odma_bypass*/false);

	// CFG_OP_SC_SET_INPUT
	sclr_set_input_size(sc_inst_d, sc_cfg.src);

	// CFG_OP_SC_SET_CROP
	sclr_set_crop(sc_inst_d, sc_cfg.crop);
	sclr_set_scale(sc_inst_d);

	// CFG_OP_SC_SET_OUTPUT
	sclr_set_output_size(sc_inst_d, sc_cfg.dst);
	sclr_set_scale(sc_inst_d);

	// CFG_OP_ODMA_CFG, sclr rgb -> odma
	odma_cfg = sclr_odma_get_cfg(sc_inst_d);
	memset(odma_cfg, 0, sizeof(*odma_cfg));
	odma_cfg->csc_cfg.mode = SCL_OUT_DISABLE;
	odma_cfg->fmt = dst_fmt_d;
	odma_cfg->csc_cfg.csc_type = SCL_CSC_NONE;

	if (odma_cfg->csc_cfg.mode == SCL_OUT_CSC) {
		enum sclr_csc csc = SCL_CSC_NONE;
		if (IS_YUV_FMT(odma_cfg->fmt))
			csc = SCL_CSC_601_LIMIT_RGB2YUV;

		odma_cfg->csc_cfg.csc_type = csc;
	}

	sclr_odma_set_cfg(sc_inst_d, odma_cfg);
	sclr_ctrl_set_output(sc_inst_d, &odma_cfg->csc_cfg, odma_cfg->fmt);

	// CFG_OP_ODMA_SET_MEM
	mem.width = sc_cfg.dst.w;
	mem.height = sc_cfg.dst.h;
	mem.start_x = 0;
	mem.start_y = 0;
	mem.addr0 = dst_addr_scl[0][G_IDX];
	mem.addr1 = mem.addr0 + mem.width * mem.height;

	_get_bytesperline(odma_cfg->fmt, sc_cfg.dst.w, pitch);
	mem.pitch_y = pitch[0];
	mem.pitch_c = pitch[1];
	sclr_odma_set_mem(sc_inst_d, &mem);

	// always enable checksum
	sclr_img_checksum_en(img_inst_v, 1);
	sclr_core_checksum_en(sc_inst_v, 1);

	//////////////////////////////////////
	// 
	// img_in_v + sc_v1
	//
	memset(&sc_cfg, 0, sizeof(sc_cfg));
	sc_cfg.src.w = 320;
	sc_cfg.src.h = 256;
	sc_cfg.crop.w = sc_cfg.src.w;
	sc_cfg.crop.h = sc_cfg.src.h;
	sc_cfg.dst.w = sc_cfg.src.w;
	sc_cfg.dst.h = sc_cfg.src.h;
	sclr_ctrl_set_scale(sc_inst_v, &sc_cfg);
	sclr_set_scale_phase(sc_inst_v, 0, 0);

	// CFG_OP_IMG_CFG, img_in_v
	in.v = SCL_INPUT_V_MEM;
	sclr_ctrl_set_input(img_inst_v, in, src_fmt_v, SCL_CSC_601_LIMIT_YUV2RGB);

	// CFG_OP_IMG_SET_MEM
	memset(&mem, 0, sizeof(mem));
	mem.width = sc_cfg.src.w;
	mem.height = sc_cfg.src.h;
	mem.start_x = 0;
	mem.start_y = 0;
	mem.addr0 = src_addr_scl[0][R_IDX];
	mem.addr1 = mem.addr0 + sc_cfg.src.w * sc_cfg.src.h;

	_get_bytesperline(src_fmt_v, sc_cfg.src.w, pitch);
	mem.pitch_y = pitch[0];
	mem.pitch_c = pitch[1];
	sclr_img_set_mem(img_inst_v, &mem);

	// CFG_OP_SC_CFG
	sclr_set_cfg(sc_inst_v, /*sc_bypass*/false, /*gop_bypass*/true, /*cir_bypass*/true, /*odma_bypass*/false);

	// CFG_OP_SC_SET_INPUT
	sclr_set_input_size(sc_inst_v, sc_cfg.src);

	// CFG_OP_SC_SET_CROP
	sclr_set_crop(sc_inst_v, sc_cfg.crop);
	sclr_set_scale(sc_inst_v);

	// CFG_OP_SC_SET_OUTPUT
	sclr_set_output_size(sc_inst_v, sc_cfg.dst);
	sclr_set_scale(sc_inst_v);

	// CFG_OP_ODMA_CFG, sclr rgb -> odma
	odma_cfg = sclr_odma_get_cfg(sc_inst_v);
	memset(odma_cfg, 0, sizeof(*odma_cfg));
	odma_cfg->csc_cfg.mode = SCL_OUT_CSC;
	odma_cfg->fmt = dst_fmt_v;
	odma_cfg->csc_cfg.csc_type = SCL_CSC_NONE;

	if (odma_cfg->csc_cfg.mode == SCL_OUT_CSC) {
		enum sclr_csc csc = SCL_CSC_NONE;
		if (IS_YUV_FMT(odma_cfg->fmt))
			csc = SCL_CSC_601_LIMIT_RGB2YUV;

		odma_cfg->csc_cfg.csc_type = csc;
	}

	sclr_odma_set_cfg(sc_inst_v, odma_cfg);
	sclr_ctrl_set_output(sc_inst_v, &odma_cfg->csc_cfg, odma_cfg->fmt);

	// CFG_OP_ODMA_SET_MEM
	mem.width = sc_cfg.dst.w;
	mem.height = sc_cfg.dst.h;
	mem.start_x = 0;
	mem.start_y = 0;
	mem.addr0 = dst_addr_scl[0][R_IDX];
	mem.addr1 = mem.addr0 + mem.width * mem.height;

	_get_bytesperline(odma_cfg->fmt, sc_cfg.dst.w, pitch);
	mem.pitch_y = pitch[0];
	mem.pitch_c = pitch[1];
	sclr_odma_set_mem(sc_inst_v, &mem);

	// always enable checksum
	sclr_img_checksum_en(img_inst_v, 1);
	sclr_core_checksum_en(sc_inst_v, 1);

	////////////////////////////////////////////////
	top_cfg = sclr_top_get_cfg();
	top_cfg->sclr_enable[sc_inst_d] = 1;
	top_cfg->sclr_enable[sc_inst_v] = 1;
	top_cfg->img_in_v_trig_src = SCL_IMG_TRIG_SRC_SW;
	top_cfg->img_in_d_trig_src = SCL_IMG_TRIG_SRC_SW;
	sclr_top_set_cfg(top_cfg);

	sclr_top_reg_done();
	_intr_setup(sc_inst_v, img_inst_v);

	// CFG_OP_IMG_START
	// trigger both img_in_d, img_in_v
	sclr_img_start_all();
	//sclr_img_start(img_inst_d);
	//sclr_img_start(img_inst_v);

	if (sclr_check_irq_handler(10000) != 0) {
		uartlog("img_d(%d), img_v(%d), sc(%d), sc(%d), end intr doesn't happens after 10000 tick\n",
			img_inst_d, img_inst_v, sc_inst_d, sc_inst_v);
		ret = -1;
	} else {
		uartlog("img_d(%d), img_v(%d), sc(%d), sc(%d), sclr done\n",
			img_inst_d, img_inst_v, sc_inst_d, sc_inst_v);
		uartlog("sc_d(%d) odma status(%#x)\n", sc_inst_d, sclr_odma_get_dbg_status(sc_inst_d).raw);
		uartlog("sc_v(%d) odma status(%#x)\n", sc_inst_v, sclr_odma_get_dbg_status(sc_inst_v).raw);
	}

	sclr_img_get_checksum_status(img_inst_d, &img_chksum_status);
	uartlog("img(%d) checksum en(%d)\n", img_inst_d, img_chksum_status.checksum_base.b.enable);
	uartlog("img(%d) checksum axi_read_data(0x%x)\n", img_inst_d, img_chksum_status.axi_read_data);
	uartlog("img(%d) checksum output_data(0x%x)\n", img_inst_d, img_chksum_status.checksum_base.b.output_data);

	sclr_core_get_checksum_status(sc_inst_d, &sc_chsum_status);
	uartlog("sc(%d) checksum en(%d)\n", sc_inst_d, sc_chsum_status.checksum_base.b.enable);
	uartlog("sc(%d) checksum axi_read_gop0_data(0x%x)\n", sc_inst_d, sc_chsum_status.axi_read_gop0_data);
	uartlog("sc(%d) checksum axi_read_gop1_data(0x%x)\n", sc_inst_d, sc_chsum_status.axi_read_gop1_data);
	uartlog("sc(%d) checksum axi_write_data(0x%x)\n", sc_inst_d, sc_chsum_status.axi_write_data);
	uartlog("sc(%d) checksum data_in(0x%x)\n", sc_inst_d, sc_chsum_status.checksum_base.b.data_in_from_img_in);
	uartlog("sc(%d) checksum data_out(0x%x)\n", sc_inst_d, sc_chsum_status.checksum_base.b.data_out);

	sclr_img_get_checksum_status(img_inst_v, &img_chksum_status);
	uartlog("img(%d) checksum en(%d)\n", img_inst_v, img_chksum_status.checksum_base.b.enable);
	uartlog("img(%d) checksum axi_read_data(0x%x)\n", img_inst_v, img_chksum_status.axi_read_data);
	uartlog("img(%d) checksum output_data(0x%x)\n", img_inst_v, img_chksum_status.checksum_base.b.output_data);

	sclr_core_get_checksum_status(sc_inst_v, &sc_chsum_status);
	uartlog("sc(%d) checksum en(%d)\n", sc_inst_v, sc_chsum_status.checksum_base.b.enable);
	uartlog("sc(%d) checksum axi_read_gop0_data(0x%x)\n", sc_inst_v, sc_chsum_status.axi_read_gop0_data);
	uartlog("sc(%d) checksum axi_read_gop1_data(0x%x)\n", sc_inst_v, sc_chsum_status.axi_read_gop1_data);
	uartlog("sc(%d) checksum axi_write_data(0x%x)\n", sc_inst_v, sc_chsum_status.axi_write_data);
	uartlog("sc(%d) checksum data_in(0x%x)\n", sc_inst_v, sc_chsum_status.checksum_base.b.data_in_from_img_in);
	uartlog("sc(%d) checksum data_out(0x%x)\n", sc_inst_v, sc_chsum_status.checksum_base.b.data_out);

	return ret;
}


// blend alpha test
//   IMG-D, Y-Only. Top half, 0xff; Bottom half, 0x00
//   IMG-V, NV12

//   1. enable blending test, with alpha factor, 256
//   2. alpha factor, 128

//   1. cover with top half white, bottom half black
//   2. transparent blending"
//
//   img_in_d + img_in_v + sc_v1 + sc_v1_odma

static int sclr_blend_privacy_mask_test(struct sclr_bld_cfg *bld_cfg,
					struct sclr_privacy_cfg *prm_cfg)
{
	int ret = 0;
	struct sclr_scale_cfg cfg;
	u8 inst_d = 0; // sc_d
	u8 inst_v = 1; // sc_v1
	enum sclr_img_in img_inst_d = SCL_IMG_D;
	enum sclr_img_in img_inst_v = SCL_IMG_V;
	union sclr_input in;
	enum sclr_format src_fmt_d = SCL_FMT_Y_ONLY;
	enum sclr_format src_fmt_v = SCL_FMT_NV12;
	enum sclr_format dst_fmt = SCL_FMT_NV12;
	struct sclr_mem mem;
	u16 pitch[3];
	struct sclr_odma_cfg *odma_cfg;
	u8 *ptr = (u8 *)src_addr_scl[0][R_IDX];
	struct sclr_top_cfg *top_cfg;

	uartlog("0x%08x: 800x600, Y only, top half white, bottom half black, 800_600_top_ff_bot_00_y.bin, in\n", src_addr_scl[0][R_IDX]);
	uartlog("0x%08x: 800x600 NV12 packed, 800-16.ppm_nv12p, in\n", src_addr_scl[1][R_IDX]);
	uartlog("0x%08x: privacy mask data, primacy_mask_rgb.bin, in\n", prm_cfg->map_cfg.base);
	uartlog("0x%08x: 800x600 NV12 packed, out\n", dst_addr_scl[0][R_IDX]);

	memset(&cfg, 0, sizeof(cfg));

	cfg.src.w = 800;
	cfg.src.h = 600;
	cfg.crop.w = cfg.src.w;
	cfg.crop.h = cfg.src.h;
	cfg.dst.w = 800;
	cfg.dst.h = 600;

	// Y only, top half white, bottom half black
	// Top half: 0xff; Bottom half: 0x00
	uartlog("Fill img_d dram content ...\n");
	memset(ptr, 0xff, cfg.src.w*cfg.src.h/2);
	memset(ptr+cfg.src.w*cfg.src.h/2, 0, cfg.src.w*cfg.src.h/2);
	uartlog("Fill img_d dram content done\n");

	//========================================
	sclr_ctrl_set_scale(inst_d, &cfg);
	sclr_set_scale_phase(inst_d, 0, 0);

	// CFG_OP_IMG_CFG
	// dram -> sclr rgb, img_in_d
	in.d = SCL_INPUT_D_MEM;
	sclr_ctrl_set_input(img_inst_d, in, src_fmt_d, SCL_CSC_601_LIMIT_YUV2RGB);

	// CFG_OP_IMG_SET_MEM, Y only
	memset(&mem, 0, sizeof(mem));
	mem.width = cfg.src.w;
	mem.height = cfg.src.h;
	mem.start_x = 0;
	mem.start_y = 0;
	mem.addr0 = src_addr_scl[0][R_IDX];

	_get_bytesperline(src_fmt_d, cfg.src.w, pitch);
	mem.pitch_y = pitch[0];
	mem.pitch_c = pitch[1];
	sclr_img_set_mem(img_inst_d, &mem);

	// CFG_OP_SC_CFG
	sclr_set_cfg(inst_d, /*sc_bypass*/true, /*gop_bypass*/true, /*cir_bypass*/true, /*odma_bypass*/true);

	//========================================
	sclr_ctrl_set_scale(inst_v, &cfg);
	sclr_set_scale_phase(inst_v, 0, 0);

	// CFG_OP_IMG_CFG
	// dram -> sclr rgb, img_in_v
	in.v = SCL_INPUT_V_MEM;
	sclr_ctrl_set_input(img_inst_v, in, src_fmt_v, SCL_CSC_601_LIMIT_YUV2RGB);

	// CFG_OP_IMG_SET_MEM, NV21 packed
	memset(&mem, 0, sizeof(mem));
	mem.width = cfg.src.w;
	mem.height = cfg.src.h;
	mem.start_x = 0;
	mem.start_y = 0;
	mem.addr0 = src_addr_scl[1][R_IDX];
	mem.addr1 = mem.addr0 + cfg.src.w * cfg.src.h;

	_get_bytesperline(src_fmt_v, cfg.src.w, pitch);
	mem.pitch_y = pitch[0];
	mem.pitch_c = pitch[1];
	sclr_img_set_mem(img_inst_v, &mem);

	// CFG_OP_SC_CFG
	sclr_set_cfg(inst_v, /*sc_bypass*/false, /*gop_bypass*/true, /*cir_bypass*/true, /*odma_bypass*/false);

	// CFG_OP_SC_SET_INPUT
	sclr_set_input_size(inst_v, cfg.src);

	// CFG_OP_SC_SET_CROP
	sclr_set_crop(inst_v, cfg.crop);
	sclr_set_scale(inst_v);

	// CFG_OP_SC_SET_OUTPUT
	sclr_set_output_size(inst_v, cfg.dst);
	sclr_set_scale(inst_v);

	// CFG_OP_ODMA_CFG, sclr rgb -> odma
	odma_cfg = sclr_odma_get_cfg(inst_v);
	memset(odma_cfg, 0, sizeof(*odma_cfg));
	odma_cfg->csc_cfg.mode = SCL_OUT_CSC;
	odma_cfg->fmt = dst_fmt;
	odma_cfg->csc_cfg.csc_type = SCL_CSC_NONE;

	if (odma_cfg->csc_cfg.mode == SCL_OUT_CSC) {
		enum sclr_csc csc = SCL_CSC_NONE;
		if (IS_YUV_FMT(odma_cfg->fmt))
			csc = SCL_CSC_601_LIMIT_RGB2YUV;

		odma_cfg->csc_cfg.csc_type = csc;
	}

	sclr_odma_set_cfg(inst_v, odma_cfg);
	sclr_ctrl_set_output(inst_v, &odma_cfg->csc_cfg, odma_cfg->fmt);

	// CFG_OP_ODMA_SET_MEM
	mem.width = cfg.dst.w;
	mem.height = cfg.dst.h;
	mem.start_x = 0;
	mem.start_y = 0;
	mem.addr0 = dst_addr_scl[0][R_IDX];
	mem.addr1 = mem.addr0 + mem.width * mem.height;

	_get_bytesperline(odma_cfg->fmt, cfg.dst.w, pitch);
	mem.pitch_y = pitch[0];
	mem.pitch_c = pitch[1];
	sclr_odma_set_mem(inst_v, &mem);

	// CFG_OP_TOP_SET_BLEND
	sclr_top_bld_set_cfg(bld_cfg);

	// CFG_OP_SC_SET_PRIVACY
	sclr_pri_set_cfg(inst_v, prm_cfg);

	top_cfg = sclr_top_get_cfg();
	top_cfg->sclr_enable[inst_d] = 0;
	top_cfg->sclr_enable[inst_v] = 1;
	top_cfg->img_in_v_trig_src = SCL_IMG_TRIG_SRC_SW;
	top_cfg->img_in_d_trig_src = SCL_IMG_TRIG_SRC_SW;
	sclr_top_set_cfg(top_cfg);

	sclr_top_reg_done();
	_intr_setup(inst_v, img_inst_v);

	// CFG_OP_IMG_START
	sclr_img_start(img_inst_d);
	sclr_img_start(img_inst_v);

	if (sclr_check_irq_handler(10000) != 0) {
		uartlog("img_d(%d), img_v(%d), sc(%d), sc(%d), (w=%d, h=%d), end intr doesn't happens after 10000 tick\n",
			img_inst_d, img_inst_v, inst_d, inst_v, cfg.dst.w, cfg.dst.h);
		ret = -1;
	} else {
		uartlog("img_d(%d), img_v(%d), sc(%d), sc(%d), (w=%d, h=%d), sclr done\n",
			img_inst_d, img_inst_v, inst_d, inst_v, cfg.dst.w, cfg.dst.h);
		uartlog("sc_d(%d) odma status(%#x)\n", inst_d, sclr_odma_get_dbg_status(inst_d).raw);
		uartlog("sc_v(%d) odma status(%#x)\n", inst_v, sclr_odma_get_dbg_status(inst_v).raw);
	}

	return ret;
}

// blend alpha test
//   IMG-D, Y-Only. Top half, 0xff; Bottom half, 0x00
//   IMG-V, NV12

//   1. enable blending test, with alpha factor, 256
//   2. alpha factor, 128

//   1. cover with top half white, bottom half black
//   2. transparent blending"
//
//   img_in_d + img_in_v + sc_v1 + sc_v1_odma
int sclr_blend_test(int argc, char **argv)
{
	struct sclr_bld_cfg bld_cfg;
	struct sclr_privacy_cfg prm_cfg;

	bld_cfg.ctrl.raw = 0;
	bld_cfg.ctrl.b.enable = 1;
	bld_cfg.ctrl.b.fix_alpha = 1;	// fix alpha value
	bld_cfg.ctrl.b.blend_y = 1;	// y channel only
	bld_cfg.ctrl.b.y2r_enable = 1;	// translate blending output from YUV to RGB
	bld_cfg.ctrl.b.alpha_factor = 256;

	prm_cfg.cfg.raw = 0;

	return sclr_blend_privacy_mask_test(&bld_cfg, &prm_cfg);
}

// blend stitch test
//   IMG-D, Y-Only. Top half, 0xff; Bottom half, 0x00
//   IMG-V, NV12

//   1. enable blending test, with alpha factor, 256
//   2. alpha factor, 128

//   1. cover with top half white, bottom half black
//   2. transparent blending"
//
//   img_in_d + img_in_v + sc_v1 + sc_v1_odma
int sclr_blend_test_2(int argc, char **argv)
{
	struct sclr_bld_cfg bld_cfg;
	struct sclr_privacy_cfg prm_cfg;

	bld_cfg.ctrl.raw = 0;
	bld_cfg.ctrl.b.enable = 1;
	bld_cfg.ctrl.b.fix_alpha = 0;	// gradient, stitching
	bld_cfg.ctrl.b.blend_y = 1;	// y channel only
	bld_cfg.ctrl.b.y2r_enable = 1;	// translate blending output from YUV to RGB
	bld_cfg.ctrl.b.alpha_factor = 256;

	prm_cfg.cfg.raw = 0;

	return sclr_blend_privacy_mask_test(&bld_cfg, &prm_cfg);
}

//   1. enable blending test, with alpha factor, 256
//   2. alpha factor, 128

//   1. cover with top half white, bottom half black
//   2. transparent blending"
//
//   img_in_d + img_in_v + sc_v1 + sc_v1_odma
//
//   Privacy mask(rgb332)
int sclr_blend_test_3(int argc, char **arg)
{
	struct sclr_bld_cfg bld_cfg;
	struct sclr_privacy_cfg prm_cfg;
	u64 map_addr = dst_addr_scl[2][R_IDX];
	u8 *ptr = (u8 *)map_addr;
	int i, j;

	bld_cfg.ctrl.raw = 0;
	bld_cfg.ctrl.b.enable = 1;
	bld_cfg.ctrl.b.fix_alpha = 1;	// fix alpha value
	bld_cfg.ctrl.b.blend_y = 1;	// y channel only
	bld_cfg.ctrl.b.y2r_enable = 1;	// translate blending output from YUV to RGB
	bld_cfg.ctrl.b.alpha_factor = 256;

	// red(0xe0) green(0x1c) blue(0x03)
	prm_cfg.cfg.raw = 0;
	prm_cfg.cfg.b.enable = 1;
	prm_cfg.cfg.b.mode = 0;	// grid mode
	prm_cfg.cfg.b.grid_size = 0;	// 8x8
	prm_cfg.cfg.b.fit_picture = 0;	// no full image
	prm_cfg.cfg.b.force_alpha = 0;
	prm_cfg.map_cfg.alpha_factor = 81;
	prm_cfg.cfg.b.mask_rgb332 = 1;
	prm_cfg.map_cfg.no_mask_idx = 0;
	prm_cfg.map_cfg.base = map_addr;
	prm_cfg.map_cfg.axi_burst = 7;
	prm_cfg.start.x = 512;
	prm_cfg.end.x = 640;	// x + 128
	prm_cfg.start.y = 64;
	prm_cfg.end.y = 192;	// y + 128

	// 8x8 grid => 128x128
	for (i = 0; i < 16; i++)
		for (j = 0; j < 16; j++)
			ptr[i * 16 + j] = (u8)rand();

	return sclr_blend_privacy_mask_test(&bld_cfg, &prm_cfg);
}

//   1. enable blending test, with alpha factor, 256
//   2. alpha factor, 128

//   1. cover with top half white, bottom half black
//   2. transparent blending"
//
//   img_in_d + img_in_v + sc_v1 + sc_v1_odma
//
//   Privacy mask(y)
int sclr_blend_test_4(int argc, char **arg)
{
	struct sclr_bld_cfg bld_cfg;
	struct sclr_privacy_cfg prm_cfg;
	u64 map_addr = dst_addr_scl[2][R_IDX];
	u8 *ptr = (u8 *)map_addr;
	int i, j;

	bld_cfg.ctrl.raw = 0;
	bld_cfg.ctrl.b.enable = 1;
	bld_cfg.ctrl.b.fix_alpha = 1;	// fix alpha value
	bld_cfg.ctrl.b.blend_y = 1;	// y channel only
	bld_cfg.ctrl.b.y2r_enable = 1;	// translate blending output from YUV to RGB
	bld_cfg.ctrl.b.alpha_factor = 256;

	// red(0xe0) green(0x1c) blue(0x03)
	prm_cfg.cfg.raw = 0;
	prm_cfg.cfg.b.enable = 1;
	prm_cfg.cfg.b.mode = 0;	// grid mode
	prm_cfg.cfg.b.grid_size = 0;	// 8x8
	prm_cfg.cfg.b.fit_picture = 0;	// no full image
	prm_cfg.cfg.b.force_alpha = 0;
	prm_cfg.map_cfg.alpha_factor = 81;
	prm_cfg.cfg.b.mask_rgb332 = 0;
	prm_cfg.cfg.b.blend_y = 1;
	prm_cfg.cfg.b.y2r_enable = 1;
	prm_cfg.map_cfg.no_mask_idx = 0;
	prm_cfg.map_cfg.base = map_addr;
	prm_cfg.map_cfg.axi_burst = 7;
	prm_cfg.start.x = 512;
	prm_cfg.end.x = 640;	// x + 128
	prm_cfg.start.y = 64;
	prm_cfg.end.y = 192;	// y + 128

	// 8x8 grid => 128x128
	for (i = 0; i < 16; i++)
		for (j = 0; j < 16; j++)
			ptr[i * 16 + j] = (u8)rand();

	return sclr_blend_privacy_mask_test(&bld_cfg, &prm_cfg);
}

// 1920x1080 nv12 packed -> 1920x1088 nv12 packed, 4 tap
int sclr_resize_test(int argc, char **argv)
{
	int ret = 0;
	struct sclr_scale_cfg cfg;
	u8 inst = 1; // sc_v1
	enum sclr_img_in img_inst = SCL_IMG_V;
	union sclr_input in;
	enum sclr_format src_fmt = SCL_FMT_NV12;
	enum sclr_format dst_fmt = SCL_FMT_NV12;
	struct sclr_mem mem;
	u16 pitch[3];
	struct sclr_odma_cfg *odma_cfg;

	uartlog("0x90000000: ldc_1920_1080_nv12, 1920x1080 NV12 packed, in\n");
	uartlog("0x96000000: 1920x1088 NV12 packed, out\n");

	memset(&cfg, 0, sizeof(cfg));

	cfg.src.w = 1920;
	cfg.src.h = 1080;
	cfg.crop.w = 1920;
	cfg.crop.h = 1080;
	cfg.dst.w = 1920;
	cfg.dst.h = 1088;

	sclr_ctrl_set_scale(inst, &cfg);
	sclr_set_scale_phase(inst, 0, 0);

	// CFG_OP_IMG_CFG
	if (inst == 0) {
		img_inst = SCL_IMG_D;
	} else {
		img_inst = SCL_IMG_V;
	}

	if (img_inst == SCL_IMG_D)
		in.d = SCL_INPUT_D_MEM;
	else
		in.v = SCL_INPUT_V_MEM;

	// dram nv12 -> sclr rgb
	sclr_ctrl_set_input(img_inst, in, src_fmt, SCL_CSC_601_LIMIT_YUV2RGB);

	// CFG_OP_IMG_SET_MEM,
	memset(&mem, 0, sizeof(mem));
	mem.width = cfg.src.w;
	mem.height = cfg.src.h;
	mem.start_x = 0;
	mem.start_y = 0;
	mem.addr0 = src_addr_scl[0][R_IDX];
	mem.addr1 = mem.addr0 + cfg.src.w * cfg.src.h;

	_get_bytesperline(src_fmt, cfg.src.w, pitch);
	mem.pitch_y = pitch[0];
	mem.pitch_c = pitch[1];
	sclr_img_set_mem(img_inst, &mem);

	// CFG_OP_SC_CFG
	sclr_set_cfg(inst, /*sc_bypass*/false, /*gop_bypass*/true, /*cir_bypass*/true, /*odma_bypass*/false);

	// CFG_OP_SC_SET_INPUT
	sclr_set_input_size(inst, cfg.src);

	// CFG_OP_SC_SET_CROP
	sclr_set_crop(inst, cfg.crop);
	sclr_set_scale(inst);

	// CFG_OP_SC_SET_OUTPUT
	sclr_set_output_size(inst, cfg.dst);
	sclr_set_scale(inst);

	// CFG_OP_ODMA_CFG, sclr rgb -> odma
	odma_cfg = sclr_odma_get_cfg(inst);
	memset(odma_cfg, 0, sizeof(*odma_cfg));
	odma_cfg->csc_cfg.mode = SCL_OUT_CSC;
	odma_cfg->fmt = dst_fmt;
	odma_cfg->csc_cfg.csc_type = SCL_CSC_NONE;

	if (odma_cfg->csc_cfg.mode == SCL_OUT_CSC) {
		enum sclr_csc csc = SCL_CSC_NONE;
		if (IS_YUV_FMT(odma_cfg->fmt))
			csc = SCL_CSC_601_LIMIT_RGB2YUV;

		odma_cfg->csc_cfg.csc_type = csc;
	}

	sclr_odma_set_cfg(inst, odma_cfg);
	sclr_ctrl_set_output(inst, &odma_cfg->csc_cfg, odma_cfg->fmt);

	// CFG_OP_ODMA_SET_MEM, nv12 packed
	mem.width = cfg.dst.w;
	mem.height = cfg.dst.h;
	mem.start_x = 0;
	mem.start_y = 0;
	mem.addr0 = dst_addr_scl[0][R_IDX];
	mem.addr1 = mem.addr0 + mem.width * mem.height;

	_get_bytesperline(odma_cfg->fmt, cfg.dst.w, pitch);
	mem.pitch_y = pitch[0];
	mem.pitch_c = pitch[1];
	sclr_odma_set_mem(inst, &mem);

	sclr_top_reg_done();
	_intr_setup(inst, img_inst);

	// CFG_OP_IMG_START
	sclr_trig_start_ext(img_inst, inst);

	if (sclr_check_irq_handler(10000) != 0) {
		uartlog("img(%d), sc(%d), (w=%d, h=%d), end intr doesn't happens after 10000 tick\n", img_inst, inst, cfg.dst.w, cfg.dst.h);
		ret = -1;
	} else {
		uartlog("img(%d), sc(%d), (w=%d, h=%d), sclr done\n", img_inst, inst, cfg.dst.w, cfg.dst.h);
		uartlog("sc(%d) odma status(%#x)\n", inst, sclr_odma_get_dbg_status(inst).raw);
	}

	return ret;
}

int sclr_resize_test_2(int argc, char **argv)
{
	int ret = 0;
	struct sclr_scale_cfg cfg;
	u8 inst = 1; // sc_v1
	enum sclr_img_in img_inst = SCL_IMG_V;
	union sclr_input in;
	enum sclr_format src_fmt = SCL_FMT_NV12;
	enum sclr_format dst_fmt = SCL_FMT_NV12;
	struct sclr_mem mem;
	u16 pitch[3];
	struct sclr_odma_cfg *odma_cfg;

	uartlog("0x90000000: ldc_1920_1080_nv12, 1920x1080 NV12 packed, in\n");
	uartlog("0x96000000: 2880x2880 NV12 packed, out\n");

	memset(&cfg, 0, sizeof(cfg));

	cfg.src.w = 1920;
	cfg.src.h = 1080;
	cfg.crop.w = 1920;
	cfg.crop.h = 1080;
	cfg.dst.w = 2880;
	cfg.dst.h = 2880;

	sclr_ctrl_set_scale(inst, &cfg);
	sclr_set_scale_phase(inst, 0, 0);

	// CFG_OP_IMG_CFG
	if (inst == 0) {
		img_inst = SCL_IMG_D;
	} else {
		img_inst = SCL_IMG_V;
	}

	if (img_inst == SCL_IMG_D)
		in.d = SCL_INPUT_D_MEM;
	else
		in.v = SCL_INPUT_V_MEM;

	// dram nv12 -> sclr rgb
	sclr_ctrl_set_input(img_inst, in, src_fmt, SCL_CSC_601_LIMIT_YUV2RGB);

	// CFG_OP_IMG_SET_MEM,
	memset(&mem, 0, sizeof(mem));
	mem.width = cfg.src.w;
	mem.height = cfg.src.h;
	mem.start_x = 0;
	mem.start_y = 0;
	mem.addr0 = src_addr_scl[0][R_IDX];
	mem.addr1 = mem.addr0 + cfg.src.w * cfg.src.h;

	_get_bytesperline(src_fmt, cfg.src.w, pitch);
	mem.pitch_y = pitch[0];
	mem.pitch_c = pitch[1];
	sclr_img_set_mem(img_inst, &mem);

	// CFG_OP_SC_CFG
	sclr_set_cfg(inst, /*sc_bypass*/false, /*gop_bypass*/true, /*cir_bypass*/true, /*odma_bypass*/false);

	// CFG_OP_SC_SET_INPUT
	sclr_set_input_size(inst, cfg.src);

	// CFG_OP_SC_SET_CROP
	sclr_set_crop(inst, cfg.crop);
	sclr_set_scale(inst);

	// CFG_OP_SC_SET_OUTPUT
	sclr_set_output_size(inst, cfg.dst);
	sclr_set_scale(inst);

	// CFG_OP_ODMA_CFG, sclr rgb -> odma
	odma_cfg = sclr_odma_get_cfg(inst);
	memset(odma_cfg, 0, sizeof(*odma_cfg));
	odma_cfg->csc_cfg.mode = SCL_OUT_CSC;
	odma_cfg->fmt = dst_fmt;
	odma_cfg->csc_cfg.csc_type = SCL_CSC_NONE;

	if (odma_cfg->csc_cfg.mode == SCL_OUT_CSC) {
		enum sclr_csc csc = SCL_CSC_NONE;
		if (IS_YUV_FMT(odma_cfg->fmt))
			csc = SCL_CSC_601_LIMIT_RGB2YUV;

		odma_cfg->csc_cfg.csc_type = csc;
	}

	sclr_odma_set_cfg(inst, odma_cfg);
	sclr_ctrl_set_output(inst, &odma_cfg->csc_cfg, odma_cfg->fmt);

	// CFG_OP_ODMA_SET_MEM, nv12 packed
	mem.width = cfg.dst.w;
	mem.height = cfg.dst.h;
	mem.start_x = 0;
	mem.start_y = 0;
	mem.addr0 = dst_addr_scl[0][R_IDX];
	mem.addr1 = mem.addr0 + mem.width * mem.height;

	_get_bytesperline(odma_cfg->fmt, cfg.dst.w, pitch);
	mem.pitch_y = pitch[0];
	mem.pitch_c = pitch[1];
	sclr_odma_set_mem(inst, &mem);

	sclr_top_reg_done();
	_intr_setup(inst, img_inst);

	// CFG_OP_IMG_START
	sclr_trig_start_ext(img_inst, inst);

	if (sclr_check_irq_handler(10000) != 0) {
		uartlog("img(%d), sc(%d), (w=%d, h=%d), end intr doesn't happens after 10000 tick\n", img_inst, inst, cfg.dst.w, cfg.dst.h);
		ret = -1;
	} else {
		uartlog("img(%d), sc(%d), (w=%d, h=%d), sclr done\n", img_inst, inst, cfg.dst.w, cfg.dst.h);
		uartlog("sc(%d) odma status(%#x)\n", inst, sclr_odma_get_dbg_status(inst).raw);
	}

	return ret;
}

int sclr_resize_test_3(int argc, char **argv)
{
	int ret = 0;
	struct sclr_scale_cfg cfg;
	u8 inst = 1; // sc_v1
	enum sclr_img_in img_inst = SCL_IMG_V;
	union sclr_input in;
	enum sclr_format src_fmt = SCL_FMT_NV12;
	enum sclr_format dst_fmt = SCL_FMT_NV12;
	struct sclr_mem mem;
	u16 pitch[3];
	struct sclr_odma_cfg *odma_cfg;

	uartlog("0x90000000: ldc_1920_1080_nv12, 1920x1080 NV12 packed, in\n");
	uartlog("0x96000000: 1984x1088 NV12 packed, out\n");

	memset(&cfg, 0, sizeof(cfg));

	cfg.src.w = 1920;
	cfg.src.h = 1080;
	cfg.crop.w = 1920;
	cfg.crop.h = 1080;
	cfg.dst.w = 1984;
	cfg.dst.h = 1088;

	sclr_ctrl_set_scale(inst, &cfg);
	sclr_set_scale_phase(inst, 0, 0);

	// CFG_OP_IMG_CFG
	if (inst == 0) {
		img_inst = SCL_IMG_D;
	} else {
		img_inst = SCL_IMG_V;
	}

	if (img_inst == SCL_IMG_D)
		in.d = SCL_INPUT_D_MEM;
	else
		in.v = SCL_INPUT_V_MEM;

	// dram nv12 -> sclr rgb
	sclr_ctrl_set_input(img_inst, in, src_fmt, SCL_CSC_601_LIMIT_YUV2RGB);

	// CFG_OP_IMG_SET_MEM,
	memset(&mem, 0, sizeof(mem));
	mem.width = cfg.src.w;
	mem.height = cfg.src.h;
	mem.start_x = 0;
	mem.start_y = 0;
	mem.addr0 = src_addr_scl[0][R_IDX];
	mem.addr1 = mem.addr0 + cfg.src.w * cfg.src.h;

	_get_bytesperline(src_fmt, cfg.src.w, pitch);
	mem.pitch_y = pitch[0];
	mem.pitch_c = pitch[1];
	sclr_img_set_mem(img_inst, &mem);

	// CFG_OP_SC_CFG
	sclr_set_cfg(inst, /*sc_bypass*/false, /*gop_bypass*/true, /*cir_bypass*/true, /*odma_bypass*/false);

	// CFG_OP_SC_SET_INPUT
	sclr_set_input_size(inst, cfg.src);

	// CFG_OP_SC_SET_CROP
	sclr_set_crop(inst, cfg.crop);
	sclr_set_scale(inst);

	// CFG_OP_SC_SET_OUTPUT
	sclr_set_output_size(inst, cfg.dst);
	sclr_set_scale(inst);

	// CFG_OP_ODMA_CFG, sclr rgb -> odma
	odma_cfg = sclr_odma_get_cfg(inst);
	memset(odma_cfg, 0, sizeof(*odma_cfg));
	odma_cfg->csc_cfg.mode = SCL_OUT_CSC;
	odma_cfg->fmt = dst_fmt;
	odma_cfg->csc_cfg.csc_type = SCL_CSC_NONE;

	if (odma_cfg->csc_cfg.mode == SCL_OUT_CSC) {
		enum sclr_csc csc = SCL_CSC_NONE;
		if (IS_YUV_FMT(odma_cfg->fmt))
			csc = SCL_CSC_601_LIMIT_RGB2YUV;

		odma_cfg->csc_cfg.csc_type = csc;
	}

	sclr_odma_set_cfg(inst, odma_cfg);
	sclr_ctrl_set_output(inst, &odma_cfg->csc_cfg, odma_cfg->fmt);

	// CFG_OP_ODMA_SET_MEM, nv12 packed
	mem.width = cfg.dst.w;
	mem.height = cfg.dst.h;
	mem.start_x = 0;
	mem.start_y = 0;
	mem.addr0 = dst_addr_scl[0][R_IDX];
	mem.addr1 = mem.addr0 + mem.width * mem.height;

	_get_bytesperline(odma_cfg->fmt, cfg.dst.w, pitch);
	mem.pitch_y = pitch[0];
	mem.pitch_c = pitch[1];
	sclr_odma_set_mem(inst, &mem);

	sclr_top_reg_done();
	_intr_setup(inst, img_inst);

	// CFG_OP_IMG_START
	sclr_trig_start_ext(img_inst, inst);

	if (sclr_check_irq_handler(10000) != 0) {
		uartlog("img(%d), sc(%d), (w=%d, h=%d), end intr doesn't happens after 10000 tick\n", img_inst, inst, cfg.dst.w, cfg.dst.h);
		ret = -1;
	} else {
		uartlog("img(%d), sc(%d), (w=%d, h=%d), sclr done\n", img_inst, inst, cfg.dst.w, cfg.dst.h);
		uartlog("sc(%d) odma status(%#x)\n", inst, sclr_odma_get_dbg_status(inst).raw);
	}

	return ret;
}

int sclr_resize_test_4(int argc, char **argv)
{
	int ret = 0;
	struct sclr_scale_cfg cfg;
	u8 inst = 1; // sc_v1
	enum sclr_img_in img_inst = SCL_IMG_V;
	union sclr_input in;
	enum sclr_format src_fmt = SCL_FMT_NV12;
	enum sclr_format dst_fmt = SCL_FMT_NV12;
	struct sclr_mem mem;
	u16 pitch[3];
	struct sclr_odma_cfg *odma_cfg;

	uartlog("0x90000000: ldc_1920_1080_nv12, 1920x1080 NV12 packed, in\n");
	uartlog("0x96000000: 320x256 NV12 packed, out\n");

	memset(&cfg, 0, sizeof(cfg));

	cfg.src.w = 1920;
	cfg.src.h = 1080;
	cfg.crop.w = 1920;
	cfg.crop.h = 1080;
	cfg.dst.w = 320;
	cfg.dst.h = 256;

	sclr_ctrl_set_scale(inst, &cfg);
	sclr_set_scale_phase(inst, 0, 0);

	// CFG_OP_IMG_CFG
	if (inst == 0) {
		img_inst = SCL_IMG_D;
	} else {
		img_inst = SCL_IMG_V;
	}

	if (img_inst == SCL_IMG_D)
		in.d = SCL_INPUT_D_MEM;
	else
		in.v = SCL_INPUT_V_MEM;

	// dram nv12 -> sclr rgb
	sclr_ctrl_set_input(img_inst, in, src_fmt, SCL_CSC_601_LIMIT_YUV2RGB);

	// CFG_OP_IMG_SET_MEM,
	memset(&mem, 0, sizeof(mem));
	mem.width = cfg.src.w;
	mem.height = cfg.src.h;
	mem.start_x = 0;
	mem.start_y = 0;
	mem.addr0 = src_addr_scl[0][R_IDX];
	mem.addr1 = mem.addr0 + cfg.src.w * cfg.src.h;

	_get_bytesperline(src_fmt, cfg.src.w, pitch);
	mem.pitch_y = pitch[0];
	mem.pitch_c = pitch[1];
	sclr_img_set_mem(img_inst, &mem);

	// CFG_OP_SC_CFG
	sclr_set_cfg(inst, /*sc_bypass*/false, /*gop_bypass*/true, /*cir_bypass*/true, /*odma_bypass*/false);

	// CFG_OP_SC_SET_INPUT
	sclr_set_input_size(inst, cfg.src);

	// CFG_OP_SC_SET_CROP
	sclr_set_crop(inst, cfg.crop);
	sclr_set_scale(inst);

	// CFG_OP_SC_SET_OUTPUT
	sclr_set_output_size(inst, cfg.dst);
	sclr_set_scale(inst);

	// CFG_OP_ODMA_CFG, sclr rgb -> odma
	odma_cfg = sclr_odma_get_cfg(inst);
	memset(odma_cfg, 0, sizeof(*odma_cfg));
	odma_cfg->csc_cfg.mode = SCL_OUT_CSC;
	odma_cfg->fmt = dst_fmt;
	odma_cfg->csc_cfg.csc_type = SCL_CSC_NONE;

	if (odma_cfg->csc_cfg.mode == SCL_OUT_CSC) {
		enum sclr_csc csc = SCL_CSC_NONE;
		if (IS_YUV_FMT(odma_cfg->fmt))
			csc = SCL_CSC_601_LIMIT_RGB2YUV;

		odma_cfg->csc_cfg.csc_type = csc;
	}

	sclr_odma_set_cfg(inst, odma_cfg);
	sclr_ctrl_set_output(inst, &odma_cfg->csc_cfg, odma_cfg->fmt);

	// CFG_OP_ODMA_SET_MEM, nv12 packed
	mem.width = cfg.dst.w;
	mem.height = cfg.dst.h;
	mem.start_x = 0;
	mem.start_y = 0;
	mem.addr0 = dst_addr_scl[0][R_IDX];
	mem.addr1 = mem.addr0 + mem.width * mem.height;

	_get_bytesperline(odma_cfg->fmt, cfg.dst.w, pitch);
	mem.pitch_y = pitch[0];
	mem.pitch_c = pitch[1];
	sclr_odma_set_mem(inst, &mem);

	sclr_top_reg_done();
	_intr_setup(inst, img_inst);

	// CFG_OP_IMG_START
	sclr_trig_start_ext(img_inst, inst);

	if (sclr_check_irq_handler(10000) != 0) {
		uartlog("img(%d), sc(%d), (w=%d, h=%d), end intr doesn't happens after 10000 tick\n", img_inst, inst, cfg.dst.w, cfg.dst.h);
		ret = -1;
	} else {
		uartlog("img(%d), sc(%d), (w=%d, h=%d), sclr done\n", img_inst, inst, cfg.dst.w, cfg.dst.h);
		uartlog("sc(%d) odma status(%#x)\n", inst, sclr_odma_get_dbg_status(inst).raw);
	}

	return ret;
}

int sclr_resize_test_5(int argc, char **argv)
{
	int ret = 0;
	struct sclr_scale_cfg cfg;
	u8 inst = 1; // sc_v1
	enum sclr_img_in img_inst = SCL_IMG_V;
	union sclr_input in;
	enum sclr_format src_fmt = SCL_FMT_NV12;
	enum sclr_format dst_fmt = SCL_FMT_BGR_PACKED;
	struct sclr_mem mem;
	u16 pitch[3];
	struct sclr_odma_cfg *odma_cfg;

	uartlog("0x90000000: ldc_1920_1080_nv12, 1920x1080 NV12 packed, in\n");
	uartlog("0x96000000: 320x256 rgb packed, out\n");

	memset(&cfg, 0, sizeof(cfg));

	cfg.src.w = 1920;
	cfg.src.h = 1080;
	cfg.crop.w = 1920;
	cfg.crop.h = 1080;
	cfg.dst.w = 320;
	cfg.dst.h = 256;

	sclr_ctrl_set_scale(inst, &cfg);
	sclr_set_scale_phase(inst, 0, 0);

	// CFG_OP_IMG_CFG
	if (inst == 0) {
		img_inst = SCL_IMG_D;
	} else {
		img_inst = SCL_IMG_V;
	}

	if (img_inst == SCL_IMG_D)
		in.d = SCL_INPUT_D_MEM;
	else
		in.v = SCL_INPUT_V_MEM;

	// dram nv12 -> sclr rgb
	sclr_ctrl_set_input(img_inst, in, src_fmt, SCL_CSC_601_LIMIT_YUV2RGB);

	// CFG_OP_IMG_SET_MEM,
	memset(&mem, 0, sizeof(mem));
	mem.width = cfg.src.w;
	mem.height = cfg.src.h;
	mem.start_x = 0;
	mem.start_y = 0;
	mem.addr0 = src_addr_scl[0][R_IDX];
	mem.addr1 = mem.addr0 + cfg.src.w * cfg.src.h;

	_get_bytesperline(src_fmt, cfg.src.w, pitch);
	mem.pitch_y = pitch[0];
	mem.pitch_c = pitch[1];
	sclr_img_set_mem(img_inst, &mem);

	// CFG_OP_SC_CFG
	sclr_set_cfg(inst, /*sc_bypass*/false, /*gop_bypass*/true, /*cir_bypass*/true, /*odma_bypass*/false);

	// CFG_OP_SC_SET_INPUT
	sclr_set_input_size(inst, cfg.src);

	// CFG_OP_SC_SET_CROP
	sclr_set_crop(inst, cfg.crop);
	sclr_set_scale(inst);

	// CFG_OP_SC_SET_OUTPUT
	sclr_set_output_size(inst, cfg.dst);
	sclr_set_scale(inst);

	// CFG_OP_ODMA_CFG, sclr rgb -> odma
	odma_cfg = sclr_odma_get_cfg(inst);
	memset(odma_cfg, 0, sizeof(*odma_cfg));
	odma_cfg->csc_cfg.mode = SCL_OUT_DISABLE;
	odma_cfg->fmt = dst_fmt;
	odma_cfg->csc_cfg.csc_type = SCL_CSC_NONE;

	if (odma_cfg->csc_cfg.mode == SCL_OUT_CSC) {
		enum sclr_csc csc = SCL_CSC_NONE;
		if (IS_YUV_FMT(odma_cfg->fmt))
			csc = SCL_CSC_601_LIMIT_RGB2YUV;

		odma_cfg->csc_cfg.csc_type = csc;
	}

	sclr_odma_set_cfg(inst, odma_cfg);
	sclr_ctrl_set_output(inst, &odma_cfg->csc_cfg, odma_cfg->fmt);

	// CFG_OP_ODMA_SET_MEM, rgb packed
	mem.width = cfg.dst.w;
	mem.height = cfg.dst.h;
	mem.start_x = 0;
	mem.start_y = 0;
	mem.addr0 = dst_addr_scl[0][R_IDX];
	mem.addr1 = mem.addr0 + mem.width * mem.height;

	_get_bytesperline(odma_cfg->fmt, cfg.dst.w, pitch);
	mem.pitch_y = pitch[0];
	mem.pitch_c = pitch[1];
	sclr_odma_set_mem(inst, &mem);

	sclr_top_reg_done();
	_intr_setup(inst, img_inst);

	// CFG_OP_IMG_START
	sclr_trig_start_ext(img_inst, inst);

	if (sclr_check_irq_handler(10000) != 0) {
		uartlog("img(%d), sc(%d), (w=%d, h=%d), end intr doesn't happens after 10000 tick\n", img_inst, inst, cfg.dst.w, cfg.dst.h);
		ret = -1;
	} else {
		uartlog("img(%d), sc(%d), (w=%d, h=%d), sclr done\n", img_inst, inst, cfg.dst.w, cfg.dst.h);
		uartlog("sc(%d) odma status(%#x)\n", inst, sclr_odma_get_dbg_status(inst).raw);
	}

	return ret;
}

// 1920x1080 nv12 packed -> 320x256 nv21 packed, 4 tap
int sclr_resize_test_6(int argc, char **argv)
{
	int ret = 0;
	struct sclr_scale_cfg cfg;
	u8 inst = 1; // sc_v1
	enum sclr_img_in img_inst = SCL_IMG_V;
	union sclr_input in;
	enum sclr_format src_fmt = SCL_FMT_NV12;
	enum sclr_format dst_fmt = SCL_FMT_NV21;
	struct sclr_mem mem;
	u16 pitch[3];
	struct sclr_odma_cfg *odma_cfg;

	uartlog("0x90000000: ldc_1920_1080_nv12, 1920x1080 NV12 packed, in\n");
	uartlog("0x96000000: 320x256 NV21 packed, out\n");

	memset(&cfg, 0, sizeof(cfg));

	cfg.src.w = 1920;
	cfg.src.h = 1080;
	cfg.crop.w = 1920;
	cfg.crop.h = 1080;
	cfg.dst.w = 320;
	cfg.dst.h = 256;

	sclr_ctrl_set_scale(inst, &cfg);
	sclr_set_scale_phase(inst, 0, 0);

	// CFG_OP_IMG_CFG
	if (inst == 0) {
		img_inst = SCL_IMG_D;
	} else {
		img_inst = SCL_IMG_V;
	}

	if (img_inst == SCL_IMG_D)
		in.d = SCL_INPUT_D_MEM;
	else
		in.v = SCL_INPUT_V_MEM;

	// dram nv12 -> sclr rgb
	sclr_ctrl_set_input(img_inst, in, src_fmt, SCL_CSC_601_LIMIT_YUV2RGB);

	// CFG_OP_IMG_SET_MEM,
	memset(&mem, 0, sizeof(mem));
	mem.width = cfg.src.w;
	mem.height = cfg.src.h;
	mem.start_x = 0;
	mem.start_y = 0;
	mem.addr0 = src_addr_scl[0][R_IDX];
	mem.addr1 = mem.addr0 + cfg.src.w * cfg.src.h;

	_get_bytesperline(src_fmt, cfg.src.w, pitch);
	mem.pitch_y = pitch[0];
	mem.pitch_c = pitch[1];
	sclr_img_set_mem(img_inst, &mem);

	// CFG_OP_SC_CFG
	sclr_set_cfg(inst, /*sc_bypass*/false, /*gop_bypass*/true, /*cir_bypass*/true, /*odma_bypass*/false);

	// CFG_OP_SC_SET_INPUT
	sclr_set_input_size(inst, cfg.src);

	// CFG_OP_SC_SET_CROP
	sclr_set_crop(inst, cfg.crop);
	sclr_set_scale(inst);

	// CFG_OP_SC_SET_OUTPUT
	sclr_set_output_size(inst, cfg.dst);
	sclr_set_scale(inst);

	// CFG_OP_ODMA_CFG, sclr rgb -> odma
	odma_cfg = sclr_odma_get_cfg(inst);
	memset(odma_cfg, 0, sizeof(*odma_cfg));
	odma_cfg->csc_cfg.mode = SCL_OUT_CSC;
	odma_cfg->fmt = dst_fmt;
	odma_cfg->csc_cfg.csc_type = SCL_CSC_NONE;

	if (odma_cfg->csc_cfg.mode == SCL_OUT_CSC) {
		enum sclr_csc csc = SCL_CSC_NONE;
		if (IS_YUV_FMT(odma_cfg->fmt))
			csc = SCL_CSC_601_LIMIT_RGB2YUV;

		odma_cfg->csc_cfg.csc_type = csc;
	}

	sclr_odma_set_cfg(inst, odma_cfg);
	sclr_ctrl_set_output(inst, &odma_cfg->csc_cfg, odma_cfg->fmt);

	// CFG_OP_ODMA_SET_MEM, nv12 packed
	mem.width = cfg.dst.w;
	mem.height = cfg.dst.h;
	mem.start_x = 0;
	mem.start_y = 0;
	mem.addr0 = dst_addr_scl[0][R_IDX];
	mem.addr1 = mem.addr0 + mem.width * mem.height;

	_get_bytesperline(odma_cfg->fmt, cfg.dst.w, pitch);
	mem.pitch_y = pitch[0];
	mem.pitch_c = pitch[1];
	sclr_odma_set_mem(inst, &mem);

	sclr_top_reg_done();
	_intr_setup(inst, img_inst);

	// CFG_OP_IMG_START
	sclr_trig_start_ext(img_inst, inst);

	if (sclr_check_irq_handler(10000) != 0) {
		uartlog("img(%d), sc(%d), (w=%d, h=%d), end intr doesn't happens after 10000 tick\n", img_inst, inst, cfg.dst.w, cfg.dst.h);
		ret = -1;
	} else {
		uartlog("img(%d), sc(%d), (w=%d, h=%d), sclr done\n", img_inst, inst, cfg.dst.w, cfg.dst.h);
		uartlog("sc(%d) odma status(%#x)\n", inst, sclr_odma_get_dbg_status(inst).raw);
	}

	return ret;
}

// 64x512 nv12 packed -> 8x512 nv21 packed, 2 tap, fast area
int sclr_resize_test_7(int argc, char **argv)
{
	int ret = 0;
	struct sclr_scale_cfg cfg;
	u8 inst = 2; // sc_v2, 2 tap
	enum sclr_img_in img_inst = SCL_IMG_V;
	union sclr_input in;
	enum sclr_format src_fmt = SCL_FMT_NV12;
	enum sclr_format dst_fmt = SCL_FMT_NV21;
	struct sclr_mem mem;
	u16 pitch[3];
	struct sclr_odma_cfg *odma_cfg;
	u16 h_nor = 0;
	u16 v_nor = 0;
	u16 ref_h_nor = 8192;
	u16 ref_v_nor = 8192;

	uartlog("Compare golden setting\n");

	memset(&cfg, 0, sizeof(cfg));

	cfg.src.w = 64;
	cfg.src.h = 512;
	cfg.crop.w = cfg.src.w;
	cfg.crop.h = cfg.src.h;
	cfg.dst.w = 8;
	cfg.dst.h = 512;

	sclr_ctrl_set_scale(inst, &cfg);
	sclr_set_scale_phase(inst, 0, 0);

	// CFG_OP_IMG_CFG
	if (inst == 0) {
		img_inst = SCL_IMG_D;
	} else {
		img_inst = SCL_IMG_V;
	}

	if (img_inst == SCL_IMG_D)
		in.d = SCL_INPUT_D_MEM;
	else
		in.v = SCL_INPUT_V_MEM;

	// dram nv12 -> sclr rgb
	sclr_ctrl_set_input(img_inst, in, src_fmt, SCL_CSC_601_LIMIT_YUV2RGB);

	// CFG_OP_IMG_SET_MEM,
	memset(&mem, 0, sizeof(mem));
	mem.width = cfg.src.w;
	mem.height = cfg.src.h;
	mem.start_x = 0;
	mem.start_y = 0;
	mem.addr0 = src_addr_scl[0][R_IDX];
	mem.addr1 = mem.addr0 + cfg.src.w * cfg.src.h;

	_get_bytesperline(src_fmt, cfg.src.w, pitch);
	mem.pitch_y = pitch[0];
	mem.pitch_c = pitch[1];
	sclr_img_set_mem(img_inst, &mem);

	// CFG_OP_SC_CFG
	sclr_set_cfg(inst, /*sc_bypass*/false, /*gop_bypass*/true, /*cir_bypass*/true, /*odma_bypass*/false);

	// CFG_OP_SC_SET_INPUT
	sclr_set_input_size(inst, cfg.src);

	// CFG_OP_SC_SET_CROP
	sclr_set_crop(inst, cfg.crop);
	sclr_set_scale(inst);

	// CFG_OP_SC_SET_OUTPUT
	sclr_set_output_size(inst, cfg.dst);
	sclr_set_scale(inst);

	// CFG_OP_ODMA_CFG, sclr rgb -> odma
	odma_cfg = sclr_odma_get_cfg(inst);
	memset(odma_cfg, 0, sizeof(*odma_cfg));
	odma_cfg->csc_cfg.mode = SCL_OUT_CSC;
	odma_cfg->fmt = dst_fmt;
	odma_cfg->csc_cfg.csc_type = SCL_CSC_NONE;

	if (odma_cfg->csc_cfg.mode == SCL_OUT_CSC) {
		enum sclr_csc csc = SCL_CSC_NONE;
		if (IS_YUV_FMT(odma_cfg->fmt))
			csc = SCL_CSC_601_LIMIT_RGB2YUV;

		odma_cfg->csc_cfg.csc_type = csc;
	}

	sclr_odma_set_cfg(inst, odma_cfg);
	sclr_ctrl_set_output(inst, &odma_cfg->csc_cfg, odma_cfg->fmt);

	// CFG_OP_ODMA_SET_MEM, nv12 packed
	mem.width = cfg.dst.w;
	mem.height = cfg.dst.h;
	mem.start_x = 0;
	mem.start_y = 0;
	mem.addr0 = dst_addr_scl[0][R_IDX];
	mem.addr1 = mem.addr0 + mem.width * mem.height;

	_get_bytesperline(odma_cfg->fmt, cfg.dst.w, pitch);
	mem.pitch_y = pitch[0];
	mem.pitch_c = pitch[1];
	sclr_odma_set_mem(inst, &mem);

	sclr_top_reg_done();
	_intr_setup(inst, img_inst);

	// CFG_OP_IMG_START
	sclr_trig_start_ext(img_inst, inst);

	if (sclr_check_irq_handler(10000) != 0) {
		uartlog("img(%d), sc(%d), (w=%d, h=%d), end intr doesn't happens after 10000 tick\n", img_inst, inst, cfg.dst.w, cfg.dst.h);
		ret = -1;
	} else {
		uartlog("img(%d), sc(%d), (w=%d, h=%d), sclr done\n", img_inst, inst, cfg.dst.w, cfg.dst.h);
		uartlog("sc(%d) odma status(%#x)\n", inst, sclr_odma_get_dbg_status(inst).raw);
	}

	sclr_read_2tap_nor(inst, &h_nor, &v_nor);

	if ((ref_h_nor != h_nor) || (ref_v_nor != v_nor)) {
		uartlog("error ! h_nor=%d, ref_h_nor=%d, v_nor=%d, ref_v_nor=%d",
			h_nor, ref_h_nor, v_nor, ref_v_nor);
		ret = -1;
	}

	return ret;
}

// 1920x1080 nv12 packed -> 1920x1080 nv21 packed, 2 tap
int sclr_resize_test_8(int argc, char **argv)
{
	int ret = 0;
	struct sclr_scale_cfg cfg;
	u8 inst = 2; // sc_v2, 2 tap
	enum sclr_img_in img_inst = SCL_IMG_V;
	union sclr_input in;
	enum sclr_format src_fmt = SCL_FMT_NV12;
	enum sclr_format dst_fmt = SCL_FMT_NV21;
	struct sclr_mem mem;
	u16 pitch[3];
	struct sclr_odma_cfg *odma_cfg;

	// sc(2) tap number=2, Version=3, line buffer width=1920
	// => max width 1920 for planar pixel format
	uartlog("0x90000000: ldc_1920_1080_nv12, 1920x1080 NV12 packed, in\n");
	uartlog("0x96000000: 1920x1080 NV21 packed, out\n");

	memset(&cfg, 0, sizeof(cfg));

	cfg.src.w = 1920;
	cfg.src.h = 1080;
	cfg.crop.w = cfg.src.w;
	cfg.crop.h = cfg.src.h;
	cfg.dst.w = 1920;
	cfg.dst.h = 1080;

	sclr_ctrl_set_scale(inst, &cfg);
	sclr_set_scale_phase(inst, 0, 0);

	// CFG_OP_IMG_CFG
	if (inst == 0) {
		img_inst = SCL_IMG_D;
	} else {
		img_inst = SCL_IMG_V;
	}

	if (img_inst == SCL_IMG_D)
		in.d = SCL_INPUT_D_MEM;
	else
		in.v = SCL_INPUT_V_MEM;

	// dram nv12 -> sclr rgb
	sclr_ctrl_set_input(img_inst, in, src_fmt, SCL_CSC_601_LIMIT_YUV2RGB);

	// CFG_OP_IMG_SET_MEM,
	memset(&mem, 0, sizeof(mem));
	mem.width = cfg.src.w;
	mem.height = cfg.src.h;
	mem.start_x = 0;
	mem.start_y = 0;
	mem.addr0 = src_addr_scl[0][R_IDX];
	mem.addr1 = mem.addr0 + cfg.src.w * cfg.src.h;

	_get_bytesperline(src_fmt, cfg.src.w, pitch);
	mem.pitch_y = pitch[0];
	mem.pitch_c = pitch[1];
	sclr_img_set_mem(img_inst, &mem);

	// CFG_OP_SC_CFG
	sclr_set_cfg(inst, /*sc_bypass*/false, /*gop_bypass*/true, /*cir_bypass*/true, /*odma_bypass*/false);

	// CFG_OP_SC_SET_INPUT
	sclr_set_input_size(inst, cfg.src);

	// CFG_OP_SC_SET_CROP
	sclr_set_crop(inst, cfg.crop);
	sclr_set_scale(inst);

	// CFG_OP_SC_SET_OUTPUT
	sclr_set_output_size(inst, cfg.dst);
	sclr_set_scale(inst);

	// CFG_OP_ODMA_CFG, sclr rgb -> odma
	odma_cfg = sclr_odma_get_cfg(inst);
	memset(odma_cfg, 0, sizeof(*odma_cfg));
	odma_cfg->csc_cfg.mode = SCL_OUT_CSC;
	odma_cfg->fmt = dst_fmt;
	odma_cfg->csc_cfg.csc_type = SCL_CSC_NONE;

	if (odma_cfg->csc_cfg.mode == SCL_OUT_CSC) {
		enum sclr_csc csc = SCL_CSC_NONE;
		if (IS_YUV_FMT(odma_cfg->fmt))
			csc = SCL_CSC_601_LIMIT_RGB2YUV;

		odma_cfg->csc_cfg.csc_type = csc;
	}

	sclr_odma_set_cfg(inst, odma_cfg);
	sclr_ctrl_set_output(inst, &odma_cfg->csc_cfg, odma_cfg->fmt);

	// CFG_OP_ODMA_SET_MEM, nv12 packed
	mem.width = cfg.dst.w;
	mem.height = cfg.dst.h;
	mem.start_x = 0;
	mem.start_y = 0;
	mem.addr0 = dst_addr_scl[0][R_IDX];
	mem.addr1 = mem.addr0 + mem.width * mem.height;

	_get_bytesperline(odma_cfg->fmt, cfg.dst.w, pitch);
	mem.pitch_y = pitch[0];
	mem.pitch_c = pitch[1];
	sclr_odma_set_mem(inst, &mem);

	sclr_top_reg_done();
	_intr_setup(inst, img_inst);

	// CFG_OP_IMG_START
	sclr_trig_start_ext(img_inst, inst);

	if (sclr_check_irq_handler(10000) != 0) {
		uartlog("img(%d), sc(%d), (w=%d, h=%d), end intr doesn't happens after 10000 tick\n", img_inst, inst, cfg.dst.w, cfg.dst.h);
		ret = -1;
	} else {
		uartlog("img(%d), sc(%d), (w=%d, h=%d), sclr done\n", img_inst, inst, cfg.dst.w, cfg.dst.h);
		uartlog("sc(%d) odma status(%#x)\n", inst, sclr_odma_get_dbg_status(inst).raw);
	}

	return ret;
}

// 1920x1080 nv12 packed -> 320x256 nv21 packed, 2 tap, area
int sclr_resize_test_9(int argc, char **argv)
{
	int ret = 0;
	struct sclr_scale_cfg cfg;
	u8 inst = 2; // sc_v2, 2 tap
	enum sclr_img_in img_inst = SCL_IMG_V;
	union sclr_input in;
	enum sclr_format src_fmt = SCL_FMT_NV12;
	enum sclr_format dst_fmt = SCL_FMT_NV21;
	struct sclr_mem mem;
	u16 pitch[3];
	struct sclr_odma_cfg *odma_cfg;

	uartlog("0x90000000: ldc_1920_1080_nv12, 1920x1080 NV12 packed, in\n");
	uartlog("0x96000000: 320x256 NV21 packed, out\n");

	memset(&cfg, 0, sizeof(cfg));

	cfg.src.w = 1920;
	cfg.src.h = 1080;
	cfg.crop.w = 1920;
	cfg.crop.h = 1080;
	cfg.dst.w = 320;
	cfg.dst.h = 256;

	sclr_ctrl_set_scale(inst, &cfg);
	sclr_set_scale_phase(inst, 0, 0);

	// CFG_OP_IMG_CFG
	if (inst == 0) {
		img_inst = SCL_IMG_D;
	} else {
		img_inst = SCL_IMG_V;
	}

	if (img_inst == SCL_IMG_D)
		in.d = SCL_INPUT_D_MEM;
	else
		in.v = SCL_INPUT_V_MEM;

	// dram nv12 -> sclr rgb
	sclr_ctrl_set_input(img_inst, in, src_fmt, SCL_CSC_601_LIMIT_YUV2RGB);

	// CFG_OP_IMG_SET_MEM,
	memset(&mem, 0, sizeof(mem));
	mem.width = cfg.src.w;
	mem.height = cfg.src.h;
	mem.start_x = 0;
	mem.start_y = 0;
	mem.addr0 = src_addr_scl[0][R_IDX];
	mem.addr1 = mem.addr0 + cfg.src.w * cfg.src.h;

	_get_bytesperline(src_fmt, cfg.src.w, pitch);
	mem.pitch_y = pitch[0];
	mem.pitch_c = pitch[1];
	sclr_img_set_mem(img_inst, &mem);

	// CFG_OP_SC_CFG
	sclr_set_cfg(inst, /*sc_bypass*/false, /*gop_bypass*/true, /*cir_bypass*/true, /*odma_bypass*/false);

	// CFG_OP_SC_SET_INPUT
	sclr_set_input_size(inst, cfg.src);

	// CFG_OP_SC_SET_CROP
	sclr_set_crop(inst, cfg.crop);
	sclr_set_scale(inst);

	// CFG_OP_SC_SET_OUTPUT
	sclr_set_output_size(inst, cfg.dst);
	sclr_set_scale(inst);

	// CFG_OP_ODMA_CFG, sclr rgb -> odma
	odma_cfg = sclr_odma_get_cfg(inst);
	memset(odma_cfg, 0, sizeof(*odma_cfg));
	odma_cfg->csc_cfg.mode = SCL_OUT_CSC;
	odma_cfg->fmt = dst_fmt;
	odma_cfg->csc_cfg.csc_type = SCL_CSC_NONE;

	if (odma_cfg->csc_cfg.mode == SCL_OUT_CSC) {
		enum sclr_csc csc = SCL_CSC_NONE;
		if (IS_YUV_FMT(odma_cfg->fmt))
			csc = SCL_CSC_601_LIMIT_RGB2YUV;

		odma_cfg->csc_cfg.csc_type = csc;
	}

	sclr_odma_set_cfg(inst, odma_cfg);
	sclr_ctrl_set_output(inst, &odma_cfg->csc_cfg, odma_cfg->fmt);

	// CFG_OP_ODMA_SET_MEM, nv12 packed
	mem.width = cfg.dst.w;
	mem.height = cfg.dst.h;
	mem.start_x = 0;
	mem.start_y = 0;
	mem.addr0 = dst_addr_scl[0][R_IDX];
	mem.addr1 = mem.addr0 + mem.width * mem.height;

	_get_bytesperline(odma_cfg->fmt, cfg.dst.w, pitch);
	mem.pitch_y = pitch[0];
	mem.pitch_c = pitch[1];
	sclr_odma_set_mem(inst, &mem);

	sclr_top_reg_done();
	_intr_setup(inst, img_inst);

	// CFG_OP_IMG_START
	sclr_trig_start_ext(img_inst, inst);

	if (sclr_check_irq_handler(10000) != 0) {
		uartlog("img(%d), sc(%d), (w=%d, h=%d), end intr doesn't happens after 10000 tick\n", img_inst, inst, cfg.dst.w, cfg.dst.h);
		ret = -1;
	} else {
		uartlog("img(%d), sc(%d), (w=%d, h=%d), sclr done\n", img_inst, inst, cfg.dst.w, cfg.dst.h);
		uartlog("sc(%d) odma status(%#x)\n", inst, sclr_odma_get_dbg_status(inst).raw);
	}

	return ret;
}

// 1920x1080 nv12 packed -> 320x270 nv21 packed, 2 tap, area fast
int sclr_resize_test_10(int argc, char **argv)
{
	int ret = 0;
	struct sclr_scale_cfg cfg;
	u8 inst = 2; // sc_v2, 2 tap
	enum sclr_img_in img_inst = SCL_IMG_V;
	union sclr_input in;
	enum sclr_format src_fmt = SCL_FMT_NV12;
	enum sclr_format dst_fmt = SCL_FMT_NV21;
	struct sclr_mem mem;
	u16 pitch[3];
	struct sclr_odma_cfg *odma_cfg;

	uartlog("0x90000000: ldc_1920_1080_nv12, 1920x1080 NV12 packed, in\n");
	uartlog("0x96000000: 320x270 NV21 packed, out\n");

	memset(&cfg, 0, sizeof(cfg));

	cfg.src.w = 1920;
	cfg.src.h = 1080;
	cfg.crop.w = cfg.src.w;
	cfg.crop.h = cfg.src.h;
	cfg.dst.w = 320;
	cfg.dst.h = 270;

	sclr_ctrl_set_scale(inst, &cfg);
	sclr_set_scale_phase(inst, 0, 0);

	// CFG_OP_IMG_CFG
	if (inst == 0) {
		img_inst = SCL_IMG_D;
	} else {
		img_inst = SCL_IMG_V;
	}

	if (img_inst == SCL_IMG_D)
		in.d = SCL_INPUT_D_MEM;
	else
		in.v = SCL_INPUT_V_MEM;

	// dram nv12 -> sclr rgb
	sclr_ctrl_set_input(img_inst, in, src_fmt, SCL_CSC_601_LIMIT_YUV2RGB);

	// CFG_OP_IMG_SET_MEM,
	memset(&mem, 0, sizeof(mem));
	mem.width = cfg.src.w;
	mem.height = cfg.src.h;
	mem.start_x = 0;
	mem.start_y = 0;
	mem.addr0 = src_addr_scl[0][R_IDX];
	mem.addr1 = mem.addr0 + cfg.src.w * cfg.src.h;

	_get_bytesperline(src_fmt, cfg.src.w, pitch);
	mem.pitch_y = pitch[0];
	mem.pitch_c = pitch[1];
	sclr_img_set_mem(img_inst, &mem);

	// CFG_OP_SC_CFG
	sclr_set_cfg(inst, /*sc_bypass*/false, /*gop_bypass*/true, /*cir_bypass*/true, /*odma_bypass*/false);

	// CFG_OP_SC_SET_INPUT
	sclr_set_input_size(inst, cfg.src);

	// CFG_OP_SC_SET_CROP
	sclr_set_crop(inst, cfg.crop);
	sclr_set_scale(inst);

	// CFG_OP_SC_SET_OUTPUT
	sclr_set_output_size(inst, cfg.dst);
	sclr_set_scale(inst);

	// CFG_OP_ODMA_CFG, sclr rgb -> odma
	odma_cfg = sclr_odma_get_cfg(inst);
	memset(odma_cfg, 0, sizeof(*odma_cfg));
	odma_cfg->csc_cfg.mode = SCL_OUT_CSC;
	odma_cfg->fmt = dst_fmt;
	odma_cfg->csc_cfg.csc_type = SCL_CSC_NONE;

	if (odma_cfg->csc_cfg.mode == SCL_OUT_CSC) {
		enum sclr_csc csc = SCL_CSC_NONE;
		if (IS_YUV_FMT(odma_cfg->fmt))
			csc = SCL_CSC_601_LIMIT_RGB2YUV;

		odma_cfg->csc_cfg.csc_type = csc;
	}

	sclr_odma_set_cfg(inst, odma_cfg);
	sclr_ctrl_set_output(inst, &odma_cfg->csc_cfg, odma_cfg->fmt);

	// CFG_OP_ODMA_SET_MEM, nv12 packed
	mem.width = cfg.dst.w;
	mem.height = cfg.dst.h;
	mem.start_x = 0;
	mem.start_y = 0;
	mem.addr0 = dst_addr_scl[0][R_IDX];
	mem.addr1 = mem.addr0 + mem.width * mem.height;

	_get_bytesperline(odma_cfg->fmt, cfg.dst.w, pitch);
	mem.pitch_y = pitch[0];
	mem.pitch_c = pitch[1];
	sclr_odma_set_mem(inst, &mem);

	sclr_top_reg_done();
	_intr_setup(inst, img_inst);

	// CFG_OP_IMG_START
	sclr_trig_start_ext(img_inst, inst);

	if (sclr_check_irq_handler(10000) != 0) {
		uartlog("img(%d), sc(%d), (w=%d, h=%d), end intr doesn't happens after 10000 tick\n", img_inst, inst, cfg.dst.w, cfg.dst.h);
		ret = -1;
	} else {
		uartlog("img(%d), sc(%d), (w=%d, h=%d), sclr done\n", img_inst, inst, cfg.dst.w, cfg.dst.h);
		uartlog("sc(%d) odma status(%#x)\n", inst, sclr_odma_get_dbg_status(inst).raw);
	}

	return ret;
}

// 320x256 nv21 packed -> 1920x1080 nv12 packed, 2 tap, bilinear
int sclr_resize_test_11(int argc, char **argv)
{
	int ret = 0;
	struct sclr_scale_cfg cfg;
	u8 inst = 2; // sc_v2, 2 tap
	enum sclr_img_in img_inst = SCL_IMG_V;
	union sclr_input in;
	enum sclr_format src_fmt = SCL_FMT_NV21;
	enum sclr_format dst_fmt = SCL_FMT_NV12;
	struct sclr_mem mem;
	u16 pitch[3];
	struct sclr_odma_cfg *odma_cfg;

	uartlog("0x90000000: 320x256 NV21 packed, in\n");
	uartlog("0x96000000: 1920x1080 NV12 packed, out\n");

	memset(&cfg, 0, sizeof(cfg));

	cfg.src.w = 320;
	cfg.src.h = 256;
	cfg.crop.w = cfg.src.w;
	cfg.crop.h = cfg.src.h;
	cfg.dst.w = 1920;
	cfg.dst.h = 1080;

	sclr_ctrl_set_scale(inst, &cfg);
	sclr_set_scale_phase(inst, 0, 0);

	// CFG_OP_IMG_CFG
	if (inst == 0) {
		img_inst = SCL_IMG_D;
	} else {
		img_inst = SCL_IMG_V;
	}

	if (img_inst == SCL_IMG_D)
		in.d = SCL_INPUT_D_MEM;
	else
		in.v = SCL_INPUT_V_MEM;

	// dram nv12 -> sclr rgb
	sclr_ctrl_set_input(img_inst, in, src_fmt, SCL_CSC_601_LIMIT_YUV2RGB);

	// CFG_OP_IMG_SET_MEM,
	memset(&mem, 0, sizeof(mem));
	mem.width = cfg.src.w;
	mem.height = cfg.src.h;
	mem.start_x = 0;
	mem.start_y = 0;
	mem.addr0 = src_addr_scl[0][R_IDX];
	mem.addr1 = mem.addr0 + cfg.src.w * cfg.src.h;

	_get_bytesperline(src_fmt, cfg.src.w, pitch);
	mem.pitch_y = pitch[0];
	mem.pitch_c = pitch[1];
	sclr_img_set_mem(img_inst, &mem);

	// CFG_OP_SC_CFG
	sclr_set_cfg(inst, /*sc_bypass*/false, /*gop_bypass*/true, /*cir_bypass*/true, /*odma_bypass*/false);

	// CFG_OP_SC_SET_INPUT
	sclr_set_input_size(inst, cfg.src);

	// CFG_OP_SC_SET_CROP
	sclr_set_crop(inst, cfg.crop);
	sclr_set_scale(inst);

	// CFG_OP_SC_SET_OUTPUT
	sclr_set_output_size(inst, cfg.dst);
	sclr_set_scale(inst);

	// CFG_OP_ODMA_CFG, sclr rgb -> odma
	odma_cfg = sclr_odma_get_cfg(inst);
	memset(odma_cfg, 0, sizeof(*odma_cfg));
	odma_cfg->csc_cfg.mode = SCL_OUT_CSC;
	odma_cfg->fmt = dst_fmt;
	odma_cfg->csc_cfg.csc_type = SCL_CSC_NONE;

	if (odma_cfg->csc_cfg.mode == SCL_OUT_CSC) {
		enum sclr_csc csc = SCL_CSC_NONE;
		if (IS_YUV_FMT(odma_cfg->fmt))
			csc = SCL_CSC_601_LIMIT_RGB2YUV;

		odma_cfg->csc_cfg.csc_type = csc;
	}

	sclr_odma_set_cfg(inst, odma_cfg);
	sclr_ctrl_set_output(inst, &odma_cfg->csc_cfg, odma_cfg->fmt);

	// CFG_OP_ODMA_SET_MEM, nv12 packed
	mem.width = cfg.dst.w;
	mem.height = cfg.dst.h;
	mem.start_x = 0;
	mem.start_y = 0;
	mem.addr0 = dst_addr_scl[0][R_IDX];
	mem.addr1 = mem.addr0 + mem.width * mem.height;

	_get_bytesperline(odma_cfg->fmt, cfg.dst.w, pitch);
	mem.pitch_y = pitch[0];
	mem.pitch_c = pitch[1];
	sclr_odma_set_mem(inst, &mem);

	sclr_top_reg_done();
	_intr_setup(inst, img_inst);

	// CFG_OP_IMG_START
	sclr_trig_start_ext(img_inst, inst);

	if (sclr_check_irq_handler(10000) != 0) {
		uartlog("img(%d), sc(%d), (w=%d, h=%d), end intr doesn't happens after 10000 tick\n", img_inst, inst, cfg.dst.w, cfg.dst.h);
		ret = -1;
	} else {
		uartlog("img(%d), sc(%d), (w=%d, h=%d), sclr done\n", img_inst, inst, cfg.dst.w, cfg.dst.h);
		uartlog("sc(%d) odma status(%#x)\n", inst, sclr_odma_get_dbg_status(inst).raw);
	}

	return ret;
}

// 800x600 rgb packed -> 320x256 nv21 packed
int sclr_resize_test_12(int argc, char **argv)
{
	int ret = 0;
	struct sclr_scale_cfg cfg;
	u8 inst = 1; // sc_v1
	enum sclr_img_in img_inst = SCL_IMG_V;
	union sclr_input in;
	enum sclr_format src_fmt = SCL_FMT_BGR_PACKED;
	enum sclr_format dst_fmt = SCL_FMT_NV21;
	struct sclr_mem mem;
	u16 pitch[3];
	struct sclr_odma_cfg *odma_cfg;

	uartlog("0x90000000: 800-16.ppm_rgbp, 800x600 rgb packed\n");
	uartlog("0x96000000: 320x256 NV21 packed, out\n");

	memset(&cfg, 0, sizeof(cfg));

	cfg.src.w = 800;
	cfg.src.h = 600;
	cfg.crop.w = 800;
	cfg.crop.h = 600;
	cfg.dst.w = 320;
	cfg.dst.h = 256;

	sclr_ctrl_set_scale(inst, &cfg);
	sclr_set_scale_phase(inst, 0, 0);

	// CFG_OP_IMG_CFG
	if (inst == 0) {
		img_inst = SCL_IMG_D;
	} else {
		img_inst = SCL_IMG_V;
	}

	if (img_inst == SCL_IMG_D)
		in.d = SCL_INPUT_D_MEM;
	else
		in.v = SCL_INPUT_V_MEM;

	// dram nv12 -> sclr rgb
	sclr_ctrl_set_input(img_inst, in, src_fmt, SCL_CSC_NONE);

	// CFG_OP_IMG_SET_MEM,
	memset(&mem, 0, sizeof(mem));
	mem.width = cfg.src.w;
	mem.height = cfg.src.h;
	mem.start_x = 0;
	mem.start_y = 0;
	mem.addr0 = src_addr_scl[0][R_IDX];
	mem.addr1 = mem.addr0 + cfg.src.w * cfg.src.h;

	_get_bytesperline(src_fmt, cfg.src.w, pitch);
	mem.pitch_y = pitch[0];
	mem.pitch_c = pitch[1];
	sclr_img_set_mem(img_inst, &mem);

	// CFG_OP_SC_CFG
	sclr_set_cfg(inst, /*sc_bypass*/false, /*gop_bypass*/true, /*cir_bypass*/true, /*odma_bypass*/false);

	// CFG_OP_SC_SET_INPUT
	sclr_set_input_size(inst, cfg.src);

	// CFG_OP_SC_SET_CROP
	sclr_set_crop(inst, cfg.crop);
	sclr_set_scale(inst);

	// CFG_OP_SC_SET_OUTPUT
	sclr_set_output_size(inst, cfg.dst);
	sclr_set_scale(inst);

	// CFG_OP_ODMA_CFG, sclr rgb -> odma
	odma_cfg = sclr_odma_get_cfg(inst);
	memset(odma_cfg, 0, sizeof(*odma_cfg));
	odma_cfg->csc_cfg.mode = SCL_OUT_CSC;
	odma_cfg->fmt = dst_fmt;
	odma_cfg->csc_cfg.csc_type = SCL_CSC_NONE;

	if (odma_cfg->csc_cfg.mode == SCL_OUT_CSC) {
		enum sclr_csc csc = SCL_CSC_NONE;
		if (IS_YUV_FMT(odma_cfg->fmt))
			csc = SCL_CSC_601_LIMIT_RGB2YUV;

		odma_cfg->csc_cfg.csc_type = csc;
	}

	sclr_odma_set_cfg(inst, odma_cfg);
	sclr_ctrl_set_output(inst, &odma_cfg->csc_cfg, odma_cfg->fmt);

	// CFG_OP_ODMA_SET_MEM, nv12 packed
	mem.width = cfg.dst.w;
	mem.height = cfg.dst.h;
	mem.start_x = 0;
	mem.start_y = 0;
	mem.addr0 = dst_addr_scl[0][R_IDX];
	mem.addr1 = mem.addr0 + mem.width * mem.height;

	_get_bytesperline(odma_cfg->fmt, cfg.dst.w, pitch);
	mem.pitch_y = pitch[0];
	mem.pitch_c = pitch[1];
	sclr_odma_set_mem(inst, &mem);

	sclr_top_reg_done();
	_intr_setup(inst, img_inst);

	// CFG_OP_IMG_START
	sclr_trig_start_ext(img_inst, inst);

	if (sclr_check_irq_handler(10000) != 0) {
		uartlog("img(%d), sc(%d), (w=%d, h=%d), end intr doesn't happens after 10000 tick\n", img_inst, inst, cfg.dst.w, cfg.dst.h);
		ret = -1;
	} else {
		uartlog("img(%d), sc(%d), (w=%d, h=%d), sclr done\n", img_inst, inst, cfg.dst.w, cfg.dst.h);
		uartlog("sc(%d) odma status(%#x)\n", inst, sclr_odma_get_dbg_status(inst).raw);
	}

	return ret;
}

int sclr_2tap_fast_area_param_test_1(void)
{
	int ret = 0;
	struct sclr_scale_2tap_cfg cfg;
	bool ref_fast_area_mode = true;
	u32 ref_scale_x = 65536;
	u32 ref_scale_y = 8192;
	u32 ref_h_nor = 8192;
	u32 ref_v_nor = 8192;

	// Scale down, fast area mode
	cfg.src_wd = 64;
	cfg.src_ht = 512;
	cfg.dst_wd = 8;
	cfg.dst_ht = 512;

	sclr_get_2tap_scale(&cfg);

	if (cfg.area_fast != ref_fast_area_mode ||
	    cfg.scale_x != ref_scale_x || cfg.scale_y != ref_scale_y ||
	    cfg.h_nor != ref_h_nor || cfg.v_nor != ref_v_nor) {
		uartlog("fast area test 1: src_wd=%d, src_ht=%d, dst_wd=%d, dst_ht=%d\n",
			cfg.src_wd, cfg.src_ht, cfg.dst_wd, cfg.dst_ht);
		uartlog("  area_fast=%d(ref=%d), scale_x=%d(ref=%d), scale_y=%d(ref=%d)\n",
			cfg.area_fast, ref_fast_area_mode,
			cfg.scale_x, ref_scale_x, cfg.scale_y, ref_scale_y);
		uartlog("  h_nor=%d(ref=%d), v_hor=%d(ref=%d)\n",
			cfg.h_nor, ref_h_nor, cfg.v_nor, ref_v_nor);
		ret = -1;
	}

	return ret;
}

int sclr_2tap_fast_area_param_test_2(void)
{
	int ret = 0;
	struct sclr_scale_2tap_cfg cfg;
	bool ref_fast_area_mode = true;
	u32 ref_scale_x = 65536;
	u32 ref_scale_y = 8192;
	u32 ref_h_nor = 8192;
	u32 ref_v_nor = 8192;

	// Scale down, fast area mode
	cfg.src_wd = 512;
	cfg.src_ht = 512;
	cfg.dst_wd = 64;
	cfg.dst_ht = 512;

	sclr_get_2tap_scale(&cfg);

	if (cfg.area_fast != ref_fast_area_mode ||
	    cfg.scale_x != ref_scale_x || cfg.scale_y != ref_scale_y ||
	    cfg.h_nor != ref_h_nor || cfg.v_nor != ref_v_nor) {
		uartlog("fast area test 2: src_wd=%d, src_ht=%d, dst_wd=%d, dst_ht=%d\n",
			cfg.src_wd, cfg.src_ht, cfg.dst_wd, cfg.dst_ht);
		uartlog("  area_fast=%d(ref=%d), scale_x=%d(ref=%d), scale_y=%d(ref=%d)\n",
			cfg.area_fast, ref_fast_area_mode,
			cfg.scale_x, ref_scale_x, cfg.scale_y, ref_scale_y);
		uartlog("  h_nor=%d(ref=%d), v_hor=%d(ref=%d)\n",
			cfg.h_nor, ref_h_nor, cfg.v_nor, ref_v_nor);
		ret = -1;
	}

	return ret;
}

int sclr_2tap_area_param_test(void)
{
	int ret = 0;
	struct sclr_scale_2tap_cfg cfg;
	bool ref_fast_area_mode = false;
	u32 ref_scale_x = 12300;
	u32 ref_scale_y = 12300;
	u32 ref_h_nor = 43648;
	u32 ref_v_nor = 43648;

	// Scale down, area mode
	cfg.src_wd = 512;
	cfg.src_ht = 512;
	cfg.dst_wd = 341;
	cfg.dst_ht = 341;

	sclr_get_2tap_scale(&cfg);

	if (cfg.area_fast != ref_fast_area_mode ||
	    cfg.scale_x != ref_scale_x || cfg.scale_y != ref_scale_y ||
	    cfg.h_nor != ref_h_nor || cfg.v_nor != ref_v_nor) {
		uartlog("area test: src_wd=%d, src_ht=%d, dst_wd=%d, dst_ht=%d\n",
			cfg.src_wd, cfg.src_ht, cfg.dst_wd, cfg.dst_ht);
		uartlog("  area_fast=%d(ref=%d), scale_x=%d(ref=%d), scale_y=%d(ref=%d)\n",
			cfg.area_fast, ref_fast_area_mode,
			cfg.scale_x, ref_scale_x, cfg.scale_y, ref_scale_y);
		uartlog("  h_nor=%d(ref=%d), v_hor=%d(ref=%d)\n",
			cfg.h_nor, ref_h_nor, cfg.v_nor, ref_v_nor);
		ret = -1;
	}

	return ret;
}

int sclr_2tap_bilinear_param_test(void)
{
	int ret = 0;
	struct sclr_scale_2tap_cfg cfg;
	bool ref_fast_area_mode = false;
	u32 ref_scale_x = 7376;
	u32 ref_scale_y = 7376;

	// Scale up, bilinear
	cfg.src_wd = 199;
	cfg.src_ht = 199;
	cfg.dst_wd = 221;
	cfg.dst_ht = 221;

	sclr_get_2tap_scale(&cfg);

	if (cfg.area_fast != ref_fast_area_mode ||
	    cfg.scale_x != ref_scale_x || cfg.scale_y != ref_scale_y) {
		uartlog("bilinear test: src_wd=%d, src_ht=%d, dst_wd=%d, dst_ht=%d\n",
			cfg.src_wd, cfg.src_ht, cfg.dst_wd, cfg.dst_ht);
		uartlog("  area_fast=%d(ref=%d), scale_x=%d(ref=%d), scale_y=%d(ref=%d)\n",
			cfg.area_fast, ref_fast_area_mode,
			cfg.scale_x, ref_scale_x, cfg.scale_y, ref_scale_y);
		ret = -1;
	}

	return ret;
}

int sclr_2tap_param_test(int argc, char **argv)
{
	int ret = 0;

	ret |= sclr_2tap_fast_area_param_test_1();
	ret |= sclr_2tap_fast_area_param_test_2();
	ret |= sclr_2tap_area_param_test();

	uartlog("2tap param test %s\n", ret ? "fail" : "pass");

	return ret;
}

// sb_mode     sb_nb     sb_size          reg_dst_mode    odma        x_end    sc bypass
// 2(frame)    0         0(64 lines)      3(rot270)       9(nv12)     319      0            => O
// 2(frame)    1         0(64 lines)      3(rot270)       9(nv12)     319      0            => X
// 2(frame)    2         0(64 lines)      3(rot270)       9(nv12)     319      0            => X (3 black lines)
// 2(frame)    3         0(64 lines)      3(rot270)       9(nv12)     319      0            => X
// 2(frame)    4         1(128 lines)     3(rot270)       9(nv12)     319      0            => O
// 1(free)     2         0(64 lines)      3(rot270)       9(nv12)     319      0            => X
// 2(frame)    0         1(128 lines)     3(rot270)       9(nv12)     319      0            => O
// 2(frame)    1         1(128 lines)     3(rot270)       9(nv12)     319      0            => O
// 2(frame)    2         1(128 lines)     3(rot270)       9(nv12)     319      0            => O
//        
// 2(frame)    0         0(64 lines)      3(rot270)       9(nv12)     255      0             => 
// 2(frame)    1         0(64 lines)      3(rot270)       9(nv12)     255      0             => X (3 black lines)
// 2(frame)    2         0(64 lines)      3(rot270)       9(nv12)     255      0             =>
// 2(frame)    3         0(64 lines)      3(rot270)       9(nv12)     255      0             =>
//
// 2(frame)    4         0(64 lines)      3(rot270)       9(nv12)     255      1             =>
//
// 2(frame)    1         0(64 lines)      3(rot270)       3(rgbp)     255      0             => O
// 2(frame)    2         0(64 lines)      3(rot270)       3(rgbp)     255      0             => O
// 2(frame)    3         0(64 lines)      3(rot270)       3(rgbp)     255      0             => 
//
// 2(frame)    0         0(64 lines)      2(rot90)        9(nv12)     319      0             => O
// 2(frame)    1         0(64 lines)      2(rot90)        9(nv12)     319      0             => O
// 2(frame)    2         0(64 lines)      2(rot90)        9(nv12)     319      0             => O
void sclr_set_ldc_to_sclr_sb(u8 img_inst, enum ldc_dst_mode ldc_rot, u8 sb_mode, u8 sb_nb, u8 sb_size)
{
	struct sclr_top_sb_cfg top_sb_cfg;
	struct sclr_img_in_sb_cfg img_sb_cfg;
	struct ldc_sb_cfg ldc_sb_cfg;
	u8 ldc_idr = (sb_mode && ldc_rot != LDC_DST_FLAT) ? 1 : 0;

	// top
	sclr_top_get_sb_default(&top_sb_cfg);
	top_sb_cfg.sb_rd_ctrl = 1; // ldc -> img_in_v
	sclr_top_set_sb(&top_sb_cfg);

	// img_in_v
	sclr_img_get_sb_default(&img_sb_cfg);
	img_sb_cfg.sb_set_str = 1;
	img_sb_cfg.sb_mode = sb_mode;
	img_sb_cfg.sb_size = sb_size;
	img_sb_cfg.sb_nb = sb_nb;
	sclr_img_set_sb(img_inst, &img_sb_cfg);

	// ldc
	ldc_get_sb_default(&ldc_sb_cfg);
	ldc_sb_cfg.sb_mode = sb_mode;
	ldc_sb_cfg.sb_nb = sb_nb;
	ldc_sb_cfg.sb_size = sb_size;
	ldc_sb_cfg.sb_set_str = 1;
	ldc_sb_cfg.ldc_dir = ldc_idr;
	ldc_set_sb(&ldc_sb_cfg);
}

void sclr_top_set_default_sb(void)
{
	struct sclr_top_sb_cfg top_sb_cfg;

	sclr_top_get_sb_default(&top_sb_cfg);
	sclr_top_set_sb(&top_sb_cfg);
}

void sclr_img_set_default_sb(u8 img_inst)
{
	struct sclr_img_in_sb_cfg img_sb_cfg;

	// img_in_v
	sclr_img_get_sb_default(&img_sb_cfg);
	sclr_img_set_sb(img_inst, &img_sb_cfg);
}

void ldc_set_default_sb(void)
{
	struct ldc_sb_cfg ldc_sb_cfg;

	ldc_get_sb_default(&ldc_sb_cfg);
	ldc_set_sb(&ldc_sb_cfg);
}

void ldc_to_sclr_sb_test_prepare_sclr(u8 inst, u8 img_inst,
				      enum sclr_format sc_fmt,
				      enum sclr_format sc_dst_fmt,
				      struct sclr_scale_cfg *cfg)
{
	union sclr_input in;
	enum sclr_format src_fmt = sc_fmt;
	enum sclr_format dst_fmt = sc_dst_fmt;
	u64 img_in_src_addr = dst_addr_scl[0][R_IDX];
	u64 odma_out_addr = dst_addr_scl[0][G_IDX];
	struct sclr_mem mem;
	u16 pitch[3];
	struct sclr_odma_cfg *odma_cfg;

	uartlog("0x%08x: %dx%d fmt(%d) packed\n", img_in_src_addr, cfg->src.w,
		cfg->src.h, sc_fmt);
	uartlog("0x%08x: %dx%d fmt(%d) packed, out\n", odma_out_addr,
		cfg->dst.w, cfg->dst.h, sc_fmt);

	sclr_ctrl_set_scale(inst, cfg);
	sclr_set_scale_phase(inst, 0, 0);

	// CFG_OP_IMG_CFG
	in.v = SCL_INPUT_V_MEM;	// still from dram

	// dram nv21 -> sclr rgb
	sclr_ctrl_set_input(img_inst, in, src_fmt, SCL_CSC_601_LIMIT_YUV2RGB);

	// CFG_OP_IMG_SET_MEM
	memset(&mem, 0, sizeof(mem));
	mem.width = cfg->src.w;
	mem.height = cfg->src.h;
	mem.start_x = 0;
	mem.start_y = 0;
	mem.addr0 = img_in_src_addr;
	mem.addr1 = mem.addr0 + cfg->src.w * cfg->src.h;

	_get_bytesperline(src_fmt, cfg->src.w, pitch);
	mem.pitch_y = pitch[0];
	mem.pitch_c = pitch[1];
	sclr_img_set_mem(img_inst, &mem);

	// CFG_OP_SC_CFG
//	sclr_set_cfg(inst, /*sc_bypass*/false, /*gop_bypass*/true, /*cir_bypass*/true, /*odma_bypass*/false);
	sclr_set_cfg(inst, /*sc_bypass*/true, /*gop_bypass*/true, /*cir_bypass*/true, /*odma_bypass*/false);

	// CFG_OP_SC_SET_INPUT
	sclr_set_input_size(inst, cfg->src);

	// CFG_OP_SC_SET_CROP
	sclr_set_crop(inst, cfg->crop);
	sclr_set_scale(inst);

	// CFG_OP_SC_SET_OUTPUT
	sclr_set_output_size(inst, cfg->dst);
	sclr_set_scale(inst);

	// CFG_OP_ODMA_CFG, sclr rgb -> odma
	odma_cfg = sclr_odma_get_cfg(inst);
	memset(odma_cfg, 0, sizeof(*odma_cfg));
	odma_cfg->csc_cfg.mode = SCL_OUT_CSC;
	odma_cfg->fmt = dst_fmt;
	odma_cfg->csc_cfg.csc_type = SCL_CSC_NONE;

	if (odma_cfg->csc_cfg.mode == SCL_OUT_CSC) {
		enum sclr_csc csc = SCL_CSC_NONE;
		if (IS_YUV_FMT(odma_cfg->fmt))
			csc = SCL_CSC_601_LIMIT_RGB2YUV;

		odma_cfg->csc_cfg.csc_type = csc;
	}

	sclr_odma_set_cfg(inst, odma_cfg);
	sclr_ctrl_set_output(inst, &odma_cfg->csc_cfg, odma_cfg->fmt);

	// CFG_OP_ODMA_SET_MEM, nv12 packed
	mem.width = cfg->dst.w;
	mem.height = cfg->dst.h;
	mem.start_x = 0;
	mem.start_y = 0;
	mem.addr0 = odma_out_addr;

	if (dst_fmt == SCL_FMT_RGB_PLANAR) {
		mem.addr1 = mem.addr0 + mem.width * mem.height;
		mem.addr2 = mem.addr1 + mem.width * mem.height;
	} else {
		// nv12 packed
		mem.addr1 = mem.addr0 + mem.width * mem.height;
	}

	_get_bytesperline(odma_cfg->fmt, cfg->dst.w, pitch);
	mem.pitch_y = pitch[0];
	mem.pitch_c = pitch[1];
	sclr_odma_set_mem(inst, &mem);

	sclr_top_reg_done();
	_intr_setup(inst, img_inst);

	// CFG_OP_IMG_START, still need to kick scaler
	sclr_trig_start_ext(img_inst, inst);
}

// 320x256 + YUV420 + sb freerun = 2 + 64line + 3 frames
int ldc_to_sclr_sb_test(int argc, char **argv)
{
	int ret = 0;
	u8 inst = 1; // sc_v1
	enum sclr_img_in img_inst = SCL_IMG_V;
	struct sclr_scale_cfg cfg;
	enum sclr_format sc_fmt = SCL_FMT_NV21;
	enum sclr_format sc_dst_fmt = SCL_FMT_NV21;
	uint8_t ldc_fmt = (sc_fmt == SCL_FMT_Y_ONLY) ? 0 : 1;
	enum ldc_dst_mode ldc_rot = LDC_DST_FLAT;
	u8 sb_mode = 2; // frame base
	u8 sb_nb   = 4;
	u8 sb_size = 0; // 0:64 lines, 1:128 lines, 2: 192 lines, 3: 256 lines

	memset(&cfg, 0, sizeof(cfg));
	cfg.src.w = 320;
	cfg.src.h = 256;
	cfg.crop.w = 320;
	cfg.crop.h = 256;
	cfg.dst.w = 320;
	cfg.dst.h = 256;

	sclr_set_ldc_to_sclr_sb(img_inst, ldc_rot, sb_mode, sb_nb, sb_size);
	ldc_to_sclr_sb_test_prepare_sclr(inst, img_inst, sc_fmt, sc_dst_fmt, &cfg);

	ret = ldc_to_sclr_sb_test_start_ldc(ldc_fmt, ldc_rot, sb_mode);

	if (sclr_check_irq_handler(10000) != 0) {
		uartlog("img(%d), sc(%d), (w=%d, h=%d), end intr doesn't happens after 10000 tick\n",
			img_inst, inst, cfg.dst.w, cfg.dst.h);
		ret = -1;
	} else {
		uartlog("img(%d), sc(%d), (w=%d, h=%d), sclr done\n", img_inst,
			inst, cfg.dst.w, cfg.dst.h);
		uartlog("sc(%d) odma status(%#x)\n", inst,
			sclr_odma_get_dbg_status(inst).raw);
	}

	return ret;
}

// 320x256 + YUV420 + rotate + sb freerun = 2 + 64line + 3 frames
int ldc_to_sclr_sb_test_2(int argc, char **argv)
{
	int ret = 0;
	u8 inst = 1; // sc_v1
	enum sclr_img_in img_inst = SCL_IMG_V;
	struct sclr_scale_cfg cfg;
	enum ldc_dst_mode ldc_rot = LDC_DST_ROT_270;
	//enum ldc_dst_mode ldc_rot = LDC_DST_ROT_90;
	enum sclr_format sc_fmt = SCL_FMT_NV21;
	enum sclr_format sc_dst_fmt = SCL_FMT_NV21;
	//enum sclr_format sc_dst_fmt = SCL_FMT_BGR_PACKED;
	uint8_t ldc_fmt = (sc_fmt == SCL_FMT_Y_ONLY) ? 0 : 1;
	u8 sb_mode = 2; // frame base
	u8 sb_nb   = 4;
	u8 sb_size = 0; // 0:64 lines, 1:128 lines, 2: 192 lines, 3: 256 lines

	memset(&cfg, 0, sizeof(cfg));
	if (ldc_rot == LDC_DST_ROT_270 || ldc_rot == LDC_DST_ROT_90) {
		cfg.src.w = 256;
		cfg.src.h = 320;
	} else {
		cfg.src.w = 320;
		cfg.src.h = 256;
	}
	cfg.crop.w = cfg.src.w;
	cfg.crop.h = cfg.src.h;
	cfg.dst.w = cfg.crop.w;
	cfg.dst.h = cfg.crop.h;

	// ldc_src_320_256.ppm_NV21
	// 800-16.ppm_2_320_256_nv21p_c906_4.bin

	sclr_set_ldc_to_sclr_sb(img_inst, ldc_rot, sb_mode, sb_nb, sb_size);
	//sclr_top_set_default_sb();
	//sclr_img_set_default_sb(img_inst);
	//ldc_set_default_sb();

	ldc_to_sclr_sb_test_prepare_sclr(inst, img_inst, sc_fmt, sc_dst_fmt, &cfg);
	ret = ldc_to_sclr_sb_test_start_ldc(ldc_fmt, ldc_rot, sb_mode);

	if (sclr_check_irq_handler(10000) != 0) {
		uartlog("img(%d), sc(%d), (w=%d, h=%d), end intr doesn't happens after 10000 tick\n", img_inst, inst, cfg.dst.w, cfg.dst.h);
		ret = -1;
	} else {
		uartlog("img(%d), sc(%d), (w=%d, h=%d), sclr done\n", img_inst, inst, cfg.dst.w, cfg.dst.h);
		uartlog("sc(%d) odma status(%#x)\n", inst, sclr_odma_get_dbg_status(inst).raw);
	}

	return ret;
}

// 320x256 + YUV420 + rotate
int ldc_to_sclr_test(int argc, char **argv)
{
	int ret = 0;
	u8 inst = 1; // sc_v1
	enum sclr_img_in img_inst = SCL_IMG_V;
	struct sclr_scale_cfg cfg;
	enum ldc_dst_mode ldc_rot = LDC_DST_ROT_270;
	//enum ldc_dst_mode ldc_rot = LDC_DST_ROT_90;
	enum sclr_format sc_fmt = SCL_FMT_NV21;
	enum sclr_format sc_dst_fmt = SCL_FMT_NV21;
	//enum sclr_format sc_dst_fmt = SCL_FMT_BGR_PACKED;
	uint8_t ldc_fmt = (sc_fmt == SCL_FMT_Y_ONLY) ? 0 : 1;

	memset(&cfg, 0, sizeof(cfg));
	if (ldc_rot == LDC_DST_ROT_270 || ldc_rot == LDC_DST_ROT_90) {
		cfg.src.w = 256;
		cfg.src.h = 320;
	} else {
		cfg.src.w = 320;
		cfg.src.h = 256;
	}
	cfg.crop.w = cfg.src.w;
	cfg.crop.h = cfg.src.h;
	cfg.dst.w = cfg.crop.w;
	cfg.dst.h = cfg.crop.h;

	// ldc_src_320_256.ppm_NV21
	// 800-16.ppm_2_320_256_nv21p_c906_4.bin

	sclr_top_set_default_sb();
	sclr_img_set_default_sb(img_inst);
	ldc_set_default_sb();

	// LDC -> DRAM
	ret = ldc_to_sclr_sb_test_start_ldc(ldc_fmt, ldc_rot, 0);
	if (ret) {
		uartlog("ldc -> dram failed\n");
		return ret;
	}

	// DRAM -> SCL
	ldc_to_sclr_sb_test_prepare_sclr(inst, img_inst, sc_fmt, sc_dst_fmt, &cfg);

	if (sclr_check_irq_handler(10000) != 0) {
		uartlog("img(%d), sc(%d), (w=%d, h=%d), end intr doesn't happens after 10000 tick\n", img_inst, inst, cfg.dst.w, cfg.dst.h);
		ret = -1;
	} else {
		uartlog("img(%d), sc(%d), (w=%d, h=%d), sclr done\n", img_inst, inst, cfg.dst.w, cfg.dst.h);
		uartlog("sc(%d) odma status(%#x)\n", inst, sclr_odma_get_dbg_status(inst).raw);
	}

	return ret;
}


void sclr_set_sclr_to_vc_sb(u8 inst)
{
	struct sclr_top_sb_cfg top_sb_cfg;
	struct sclr_odma_sb_cfg odma_sb_cfg;

	// top
	sclr_top_get_sb_default(&top_sb_cfg);
	top_sb_cfg.sb_wr_ctrl0 = 1; // sc_v1 -> vc
	sclr_top_set_sb(&top_sb_cfg);

	// odma
	sclr_odma_get_sb_default(&odma_sb_cfg);
	odma_sb_cfg.sb_nb = 1;
	odma_sb_cfg.sb_full_nb = 0;
	odma_sb_cfg.sb_mode = 2; // frame base mode
	sclr_odma_set_sb(inst, &odma_sb_cfg);
}

int sclr_to_vc_sb_test_start_sclr(u8 inst, u8 img_inst)
{
	int ret = 0;
	union sclr_input in;
	enum sclr_format src_fmt = SCL_FMT_NV21;
	enum sclr_format dst_fmt = SCL_FMT_NV21;
	u64 img_in_src_addr = src_addr_scl[0][R_IDX];
	u64 odma_out_addr = dst_addr_scl[0][R_IDX];
	struct sclr_mem mem;
	u16 pitch[3];
	struct sclr_scale_cfg sc_cfg;
	struct sclr_odma_cfg *odma_cfg;

	uartlog("0x91000000: coastguard_352x288_1_nv21.yuv, 352x288 NV21 packed\n");
	uartlog("0x96000000: 352x288 NV21 packed\n");

	memset(&sc_cfg, 0, sizeof(sc_cfg));

	sc_cfg.src.w = 352;
	sc_cfg.src.h = 288;
	sc_cfg.crop.w = sc_cfg.src.w;
	sc_cfg.crop.h = sc_cfg.src.h;
	sc_cfg.dst.w = 352;
	sc_cfg.dst.h = 288;

	sclr_ctrl_set_scale(inst, &sc_cfg);
	sclr_set_scale_phase(inst, 0, 0);

	// CFG_OP_IMG_CFG
	in.v = SCL_INPUT_V_MEM;	// still from dram

	// dram -> sclr rgb
	sclr_ctrl_set_input(img_inst, in, src_fmt, SCL_CSC_601_LIMIT_YUV2RGB);

	// CFG_OP_IMG_SET_MEM
	memset(&mem, 0, sizeof(mem));
	mem.width = sc_cfg.src.w;
	mem.height = sc_cfg.src.h;
	mem.start_x = 0;
	mem.start_y = 0;
	mem.addr0 = img_in_src_addr;
	mem.addr1 = mem.addr0 + sc_cfg.src.w * sc_cfg.src.h;

	_get_bytesperline(src_fmt, sc_cfg.src.w, pitch);
	mem.pitch_y = pitch[0];
	mem.pitch_c = pitch[1];
	sclr_img_set_mem(img_inst, &mem);

	// CFG_OP_SC_CFG
	sclr_set_cfg(inst, /*sc_bypass*/false, /*gop_bypass*/true, /*cir_bypass*/true, /*odma_bypass*/false);

	// CFG_OP_SC_SET_INPUT
	sclr_set_input_size(inst, sc_cfg.src);

	// CFG_OP_SC_SET_CROP
	sclr_set_crop(inst, sc_cfg.crop);
	sclr_set_scale(inst);

	// CFG_OP_SC_SET_OUTPUT
	sclr_set_output_size(inst, sc_cfg.dst);
	sclr_set_scale(inst);

	// CFG_OP_ODMA_CFG, sclr rgb -> odma
	odma_cfg = sclr_odma_get_cfg(inst);
	memset(odma_cfg, 0, sizeof(*odma_cfg));
	odma_cfg->csc_cfg.mode = SCL_OUT_CSC;
	odma_cfg->fmt = dst_fmt;
	odma_cfg->csc_cfg.csc_type = SCL_CSC_NONE;

	if (odma_cfg->csc_cfg.mode == SCL_OUT_CSC) {
		enum sclr_csc csc = SCL_CSC_NONE;
		if (IS_YUV_FMT(odma_cfg->fmt))
			csc = SCL_CSC_601_LIMIT_RGB2YUV;

		odma_cfg->csc_cfg.csc_type = csc;
	}

	sclr_odma_set_cfg(inst, odma_cfg);
	sclr_ctrl_set_output(inst, &odma_cfg->csc_cfg, odma_cfg->fmt);

	// CFG_OP_ODMA_SET_MEM
	mem.width = sc_cfg.dst.w;
	mem.height = sc_cfg.dst.h;
	mem.start_x = 0;
	mem.start_y = 0;
	mem.addr0 = odma_out_addr;
	mem.addr1 = mem.addr0 + mem.width * mem.height;

	_get_bytesperline(odma_cfg->fmt, sc_cfg.dst.w, pitch);
	mem.pitch_y = pitch[0];
	mem.pitch_c = pitch[1];
	sclr_odma_set_mem(inst, &mem);

	sclr_top_reg_done();
	_intr_setup(inst, img_inst);

	// CFG_OP_IMG_START
	sclr_trig_start_ext(img_inst, inst);

	if (sclr_check_irq_handler(10000) != 0) {
		uartlog("img(%d), sc(%d), (w=%d, h=%d), end intr doesn't happens after 10000 tick\n", img_inst, inst, sc_cfg.dst.w, sc_cfg.dst.h);
		ret = -1;
	} else {
		uartlog("img(%d), sc(%d), (w=%d, h=%d), sclr done\n", img_inst, inst, sc_cfg.dst.w, sc_cfg.dst.h);
		uartlog("sc(%d) odma status(%#x)\n", inst, sclr_odma_get_dbg_status(inst).raw);
	}

	return ret;
}

int sclr_to_vc_sb_test(int argc, char **argv)
{
	int ret = 0;
	u8 inst = 1; // sc_v1
	enum sclr_img_in img_inst = SCL_IMG_V;

	sclr_set_sclr_to_vc_sb(inst);
	ret = sclr_to_vc_sb_test_start_sclr(inst, img_inst);

	return ret;
}
