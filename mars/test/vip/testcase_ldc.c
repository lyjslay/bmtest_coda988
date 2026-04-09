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
#include "reg.h"
#include "cmdq.h"
#include "ldc.h"
#include "ldc_reg.h"

static u64 timer_tick = 0;
static volatile int mark;
static bool enable_cmdq = false;

extern u64 cmdq_set;
extern u64 src_addr_scl[3][3];
extern u64 dst_addr_scl[3][3];

static int irq_handler_ldc(int irqn, void *priv)
{
	u64 now = timer_get_tick();
	u8 intr_status;

	uartlog("%s: time-consumption(%d)ms\n", __func__, (now - timer_tick)/TIMER_CNT_MS);

	if (enable_cmdq) {
		u8 status = cmdQ_intr_status(REG_LDC_CMDQ_BASE);
		if (status) {
			cmdQ_intr_clr(REG_LDC_CMDQ_BASE, status);
			uartlog("%s: irqn(%d) cmdq-status(0x%x)\n", __func__, irqn, status);

			if ((status & 0x04)&& cmdQ_is_sw_restart(REG_LDC_CMDQ_BASE)) {
				uartlog("%s: cmdq-sw restart\n", __func__);
				cmdQ_sw_restart(REG_LDC_CMDQ_BASE);
			}

			if (status & 0x02) {
				mark++;  // cmdq end
				enable_cmdq = false;
			}
		} else {
			uartlog("%s: irqn(%d) cmdq-status not found\n", __func__, irqn);
		}
	} else {
		intr_status = ldc_intr_status();
		ldc_intr_clr(intr_status);
		++mark;
		uartlog("%s: status(%#x)\n", __func__, intr_status);
	}

	return 0;
}

static int check_irq_handler(int timeout)
{
	int i;

	for (i = 0; i < timeout; ++i) {
	if (mark > 0) {
		mark = 0;
		uartlog("%s: check tick:%d\n", __func__, i);
		return 0;
	}
	mdelay(1);
	}

	return -1;
}

static int do_job(int argc, char **argv)
{
	int ret = 0;
	int use_cmdq = 0;
	struct ldc_cfg cfg;
	u32 tmp;
	const u32 ldc_src_buf = src_addr_scl[0][0];
	const u32 ldc_dst_buf = dst_addr_scl[0][0];
	const u32 ldc_map_base = src_addr_scl[1][0];

	bm_get_num("pixel format, 1(NV21)/0(Y): ", cfg.pixel_fmt);
	bm_get_num("bypass, 1(Yes)/0(No): ", cfg.map_bypass);
	cfg.map_base = ldc_map_base;

	bm_get_num("src width : ", cfg.src_width);
	bm_get_num("src height : ", cfg.src_height);
	cfg.ras_width = cfg.src_width;
	cfg.ras_height = cfg.src_height;

	//if ((cfg.src_width & 0x3f) || (cfg.src_height & 0x3f)) {
	//	uartlog("%s: size must be 64x\n", __func__);
	//	return -1;
	//}
	if (cfg.src_width & 0x3f) {
		uartlog("%s: width must be 64x\n", __func__);
		return -1;
	}

	bm_get_num("src x start : ", cfg.src_xstart);
	bm_get_num("src x end : ", cfg.src_xend);
	cfg.bgcolor = (128 << 8) | 128; // yuv black

	cfg.src_y_base = ldc_src_buf;
	cfg.src_c_base = cfg.src_y_base + cfg.src_width * cfg.src_height;
	cfg.dst_y_base = ldc_dst_buf;
	cfg.dst_c_base = cfg.dst_y_base + cfg.src_width * cfg.src_height;

	uartlog("%s: src buf address Y(%#x) C(%#x)\n", __func__, cfg.src_y_base, cfg.src_c_base);
	uartlog("%s: dst buf address Y(%#x) C(%#x)\n", __func__, cfg.dst_y_base, cfg.dst_c_base);

	bm_get_num("dst mode, 0(flat)/1(xy_flip)/2(rot 90)/3(rot 270): ", cfg.dst_mode);

	bm_get_num("reg/cmdq mode, 0(reg)/1(cmdq): ", cfg.use_cmdq);
	use_cmdq = cfg.use_cmdq;

	timer_tick = timer_get_tick();
	if (!use_cmdq) {
		ldc_intr_ctrl(1);
		ldc_engine(&cfg);
	} else {
		enable_cmdq = true;
		ldc_engine_cmdq(&cfg, cmdq_set);
	}

#ifndef ENV_EMU
	if (check_irq_handler(60) != 0) {
		uartlog("ldc end intr doesn't happens after 60 ms\n");
		ret = -1;
	} else {
		uartlog("ldc done\n");
	}
#endif
	return ret;
}

static int do_dump(int argc, char **argv)
{
	ldc_dump_register();

	return 0;
}

// 64x64, memcpy
static int ldc_test1(int argc, char **argv)
{
	const u32 ldc_src_buf = src_addr_scl[0][0];
	const u32 ldc_dst_buf = dst_addr_scl[0][0];
	char *src = (char *)(uintptr_t)ldc_src_buf;
	char *dst = (char *)(uintptr_t)ldc_dst_buf;
	struct ldc_cfg cfg;
	int ret = 0;
	u8 status;
	int i;

	memset(&cfg, 0, sizeof(cfg));
	cfg.pixel_fmt = 0; // Y only
	cfg.dst_mode = 0; // flat
	cfg.map_bypass = 1;
	cfg.src_width = 64; // 64X
	cfg.src_height = 64; // 64X
	cfg.ras_width = cfg.src_width;
	cfg.ras_height = cfg.src_height;
	cfg.src_xstart = 0;
	cfg.src_xend = cfg.ras_width - 1;
	cfg.src_y_base = ldc_src_buf;
	cfg.dst_y_base = ldc_dst_buf;
	cfg.bgcolor = 0xff;

	uartlog("%s: src buf address Y(%#x)\n", __func__, cfg.src_y_base);
	uartlog("%s: dst buf address Y(%#x)\n", __func__, cfg.dst_y_base);

	for (i = 0; i < 32 * 32; i++) {
		src[i] = i;
		dst[i] = 0;
	}

	// Write 1 clear
	ldc_intr_clr(1);

	status = ldc_intr_status();
	if (status) {
		uartlog("ldc intr status(%d) doesn't cleared\n");
		ret = -1;
	}

	ldc_engine(&cfg);

#ifndef ENV_EMU
	if (check_irq_handler(60) != 0) {
		uartlog("ldc end intr doesn't happens after 60 ms\n");
		ret = -1;
	} else {
		uartlog("ldc done\n");
	}
#endif

	status = ldc_intr_status();
	uartlog("ldc intr status(%d) after kicked\n", status);

	for (i = 0; i < 64 * 64; i++) {
		if (src[i] != dst[i]) {
			uartlog("  [%d] src=%d, dst=%d, error !\n", i, src[i], dst[i]);
			ret = -1;
			break;
		}
	}

	return ret;
}

// 128x128, memcpy
static int ldc_test2(int argc, char **argv)
{
	const u32 ldc_src_buf = src_addr_scl[0][0];
	const u32 ldc_dst_buf = dst_addr_scl[0][0];
	char *src = (char *)(uintptr_t)ldc_src_buf;
	char *dst = (char *)(uintptr_t)ldc_dst_buf;
	struct ldc_cfg cfg;
	int ret = 0;
	u8 status;
	int i;

	memset(&cfg, 0, sizeof(cfg));
	cfg.pixel_fmt = 0; // Y only
	cfg.dst_mode = 0; // flat
	cfg.map_bypass = 1;
	cfg.src_width = 128; // 64X
	cfg.src_height = 128; // 64X
	cfg.ras_width = cfg.src_width;
	cfg.ras_height = cfg.src_height;
	cfg.src_xstart = 0;
	cfg.src_xend = cfg.ras_width - 1;
	cfg.src_y_base = ldc_src_buf;
	cfg.dst_y_base = ldc_dst_buf;
	cfg.bgcolor = 0xff;

	uartlog("%s: src buf address Y(%#x)\n", __func__, cfg.src_y_base);
	uartlog("%s: dst buf address Y(%#x)\n", __func__, cfg.dst_y_base);

	for (i = 0; i < 128 * 128; i++) {
		src[i] = i;
		dst[i] = 0;
	}

	// Write 1 clear
	ldc_intr_clr(1);

	status = ldc_intr_status();
	if (status) {
		uartlog("ldc intr status(%d) doesn't cleared\n");
		ret = -1;
	}

	ldc_engine(&cfg);

#ifndef ENV_EMU
	if (check_irq_handler(60) != 0) {
		uartlog("ldc end intr doesn't happens after 60 ms\n");
		ret = -1;
	} else {
		uartlog("ldc done\n");
	}
#endif

	status = ldc_intr_status();
	uartlog("ldc intr status(%d) after kicked\n", status);

	for (i = 0; i < 128 * 128; i++) {
		if (src[i] != dst[i]) {
			uartlog("  [%d] src=%d, dst=%d, error !\n", i, src[i], dst[i]);
			ret = -1;
			break;
		}
	}

	return ret;
}

// 1920x1088, memcpy
static int ldc_test3(int argc, char **argv)
{
	const u32 ldc_src_buf = src_addr_scl[0][0];
	const u32 ldc_dst_buf = dst_addr_scl[0][0];
	u64 *src = (u64 *)(uintptr_t)ldc_src_buf;
	u64 *dst = (u64 *)(uintptr_t)ldc_dst_buf;
	struct ldc_cfg cfg;
	int ret = 0;
	u8 status;
	u64 i;

	memset(&cfg, 0, sizeof(cfg));
	cfg.pixel_fmt = 1; // NV21
	cfg.dst_mode = LDC_DST_FLAT; // flat
	cfg.map_bypass = 1;
	cfg.src_width = 1920; // 64X
	cfg.src_height = 1088; // 64X
	cfg.ras_width = cfg.src_width;
	cfg.ras_height = cfg.src_height;
	cfg.src_xstart = 0;
	cfg.src_xend = cfg.ras_width - 1;
	cfg.src_y_base = ldc_src_buf;
	cfg.src_c_base = cfg.src_y_base + cfg.src_width * cfg.src_height;
	cfg.dst_y_base = ldc_dst_buf;
	cfg.dst_c_base = cfg.dst_y_base + cfg.src_width * cfg.src_height;
	cfg.bgcolor = 0xff;

	uartlog("%s: src buf address Y(%#x) C(%#x)\n", __func__, cfg.src_y_base, cfg.src_c_base);
	uartlog("%s: dst buf address Y(%#x) C(%#x)\n", __func__, cfg.dst_y_base, cfg.dst_c_base);

	for (i = 0; i < (cfg.src_width * cfg.src_height / 64); i++) {
		src[i] = i;
		dst[i] = 0;
	}

	// Write 1 clear
	ldc_intr_clr(1);

	status = ldc_intr_status();
	if (status) {
		uartlog("ldc intr status(%d) doesn't cleared\n");
		ret = -1;
	}

	ldc_engine(&cfg);

#ifndef ENV_EMU
	if (check_irq_handler(500) != 0) {
		uartlog("ldc end intr doesn't happens after 60 ms\n");
		ret = -1;
	} else {
		uartlog("ldc done\n");
	}
#endif

	status = ldc_intr_status();
	uartlog("ldc intr status(%d) after kicked\n", status);

	for (i = 0; i < (cfg.src_width * cfg.src_height / 64); i++) {
		if (src[i] != dst[i]) {
			uartlog("  [%d] src=%d, dst=%d, error !\n", i, src[i], dst[i]);
			ret = -1;
			break;
		}
	}

	return ret;
}

// 1920x1088, 2 pass ldc
static int ldc_test4(int argc, char **argv)
{
	int ret = 0;
	int use_cmdq = 0;
	struct ldc_cfg cfg;
	const u32 ldc_src_buf = src_addr_scl[0][0];
	const u32 ldc_map_base_1st = src_addr_scl[1][0];
	const u32 ldc_map_base_2nd = src_addr_scl[2][0];
	const u32 ldc_dst_buf_1st = dst_addr_scl[0][0];
	const u32 ldc_dst_buf_2nd = dst_addr_scl[1][0];

	// From 182x 20210930_ldc_output
	//uartlog("0x90000000: ldc_1920_1088_nv12.bin, in\n");
	uartlog("0x%08x: sample.yuv, 1920x1080 nv12, in\n", ldc_src_buf);
	uartlog("0x%08x: ldc_1920_1080_srcx_1st_mesh.bin, 1st mesh, in\n", ldc_map_base_1st);
	uartlog("0x%08x: ldc_1920_1080_srcx_2nd_mesh.bin, 2nd mesh, in\n", ldc_map_base_2nd);
	uartlog("0x%08x: 1088x1920 NV12 packed, 1st out\n", ldc_dst_buf_1st);
	uartlog("0x%08x: 1920x1088 NV12 packed, 2nd out\n", ldc_dst_buf_2nd);

	memset(&cfg, 0, sizeof(cfg));
	cfg.pixel_fmt = 0;	// Y
	cfg.map_bypass = 0;
	cfg.map_base = ldc_map_base_1st;

	cfg.src_width = 1920;	// 64X
	cfg.src_height = 1088;	// 64X
	cfg.ras_width = cfg.src_width;
	cfg.ras_height = cfg.src_height;

	cfg.src_xstart = 0;
	cfg.src_xend = cfg.src_width - 1;
	cfg.bgcolor = (128 << 8) | 128; // yuv black

	cfg.src_y_base = ldc_src_buf;
	cfg.src_c_base = cfg.src_y_base + cfg.src_width * cfg.src_height;
	cfg.dst_y_base = ldc_dst_buf_1st;
	cfg.dst_c_base = cfg.dst_y_base + cfg.src_width * cfg.src_height;
	cfg.dst_mode = LDC_DST_ROT_90;

	uartlog("%s: 1st src buf address Y(%#x) C(%#x)\n", __func__, cfg.src_y_base, cfg.src_c_base);
	uartlog("%s: 1st dst buf address Y(%#x) C(%#x)\n", __func__, cfg.dst_y_base, cfg.dst_c_base);

	timer_tick = timer_get_tick();
	if (!use_cmdq) {
		ldc_intr_ctrl(1);
		ldc_engine(&cfg);
	} else {
		enable_cmdq = true;
		ldc_engine_cmdq(&cfg, cmdq_set);
	}

#ifndef ENV_EMU
	if (check_irq_handler(600) != 0) {
		uartlog("1st ldc end intr doesn't happens after 60 ms\n");
		ret = -1;
		return ret;
	} else {
		uartlog("1st ldc done\n");
	}
#endif

	// debug
	if (!ret)
		return ret;

	// 2nd pass
	cfg.map_base = ldc_map_base_2nd;

	cfg.src_width = 1088;	// 64X
	cfg.src_height = 1920;	// 64X
	cfg.ras_width = cfg.src_width;
	cfg.ras_height = cfg.src_height;

	cfg.src_xstart = 0;
	cfg.src_xend = cfg.src_width - 1;
	cfg.bgcolor = (128 << 8) | 128; // yuv black

	cfg.src_y_base = ldc_dst_buf_1st;
	cfg.src_c_base = cfg.src_y_base + cfg.src_width * cfg.src_height;
	cfg.dst_y_base = ldc_dst_buf_2nd;
	cfg.dst_c_base = cfg.dst_y_base + cfg.src_width * cfg.src_height;
	cfg.dst_mode = LDC_DST_ROT_270;
	//cfg.dst_mode = LDC_DST_FLAT;

	uartlog("%s: 2nd src buf address Y(%#x) C(%#x)\n", __func__, cfg.src_y_base, cfg.src_c_base);
	uartlog("%s: 2nd dst buf address Y(%#x) C(%#x)\n", __func__, cfg.dst_y_base, cfg.dst_c_base);

	timer_tick = timer_get_tick();
	if (!use_cmdq) {
		ldc_intr_ctrl(1);
		ldc_engine(&cfg);
	} else {
		enable_cmdq = true;
		ldc_engine_cmdq(&cfg, cmdq_set);
	}

#ifndef ENV_EMU
	if (check_irq_handler(60) != 0) {
		uartlog("2nd ldc end intr doesn't happens after 60 ms\n");
		ret = -1;
	} else {
		uartlog("2nd ldc done\n");
	}
#endif

	return ret;
}

// 984x1088, 2 pass ldc
static int ldc_test5(int argc, char **argv)
{
	int ret = 0;
	int use_cmdq = 0;
	struct ldc_cfg cfg;
	const u32 ldc_src_buf = src_addr_scl[0][0];
	const u32 ldc_map_base_1st = src_addr_scl[1][0];
	const u32 ldc_map_base_2nd = src_addr_scl[2][0];
	const u32 ldc_dst_buf_1st = dst_addr_scl[0][0];
	const u32 ldc_dst_buf_2nd = src_addr_scl[1][0];

	// From 182x 20210930_ldc_output
	uartlog("0x%08x: 1920_1080_2_1984_1088_nv12.bin, in\n", ldc_src_buf);
	uartlog("0x%08x: 1984_1088_srcx_1st_mesh.bin, 1st mesh, in\n", ldc_map_base_1st);
	uartlog("0x%08x: 1984_1088_srcx_2nd_mesh.bin, 2nd mesh, in\n", ldc_map_base_2nd);
	uartlog("0x%08x: 1088x1984 NV12 packed, 1st out\n", ldc_dst_buf_1st);
	uartlog("0x%08x: 1984x1088 NV12 packed, 2nd out\n", ldc_dst_buf_2nd);

	// 1st pass
	memset(&cfg, 0, sizeof(cfg));
	cfg.pixel_fmt = 1;	// nv12
	cfg.map_bypass = 0;
	cfg.map_base = ldc_map_base_1st;

	cfg.src_width = 1984;	// 64X
	cfg.src_height = 1088;	// 64X
	cfg.ras_width = cfg.src_width;
	cfg.ras_height = cfg.src_height;

	cfg.src_xstart = 0;
	cfg.src_xend = cfg.src_width - 1;
	cfg.bgcolor = (128 << 8) | 128; // yuv black

	cfg.src_y_base = ldc_src_buf;
	cfg.src_c_base = cfg.src_y_base + cfg.src_width * cfg.src_height;
	cfg.dst_y_base = ldc_dst_buf_1st;
	cfg.dst_c_base = cfg.dst_y_base + cfg.src_width * cfg.src_height;
	//cfg.dst_mode = LDC_DST_ROT_90;
	cfg.dst_mode = LDC_DST_FLAT;

	uartlog("%s: 1st src buf address Y(%#x) C(%#x)\n", __func__, cfg.src_y_base, cfg.src_c_base);
	uartlog("%s: 1st dst buf address Y(%#x) C(%#x)\n", __func__, cfg.dst_y_base, cfg.dst_c_base);

	timer_tick = timer_get_tick();
	if (!use_cmdq) {
		ldc_intr_ctrl(1);
		ldc_engine(&cfg);
	} else {
		enable_cmdq = true;
		ldc_engine_cmdq(&cfg, cmdq_set);
	}

#ifndef ENV_EMU
	if (check_irq_handler(600) != 0) {
		uartlog("1st ldc end intr doesn't happens after 60 ms\n");
		ret = -1;
		return ret;
	} else {
		uartlog("1st ldc done\n");
	}
#endif

	// debug
	if (!ret)
		return ret;

	// 2nd pass
	cfg.map_base = ldc_map_base_2nd;

	cfg.src_width = 1984;	// 64X
	cfg.src_height = 1088;	// 64X
	cfg.ras_width = cfg.src_width;
	cfg.ras_height = cfg.src_height;

	cfg.src_xstart = 0;
	cfg.src_xend = cfg.src_width - 1;
	cfg.bgcolor = (128 << 8) | 128; // yuv black

	cfg.src_y_base = ldc_dst_buf_1st;
	cfg.src_c_base = cfg.src_y_base + cfg.src_width * cfg.src_height;
	cfg.dst_y_base = ldc_dst_buf_2nd;
	cfg.dst_c_base = cfg.dst_y_base + cfg.src_width * cfg.src_height;
	cfg.dst_mode = LDC_DST_ROT_270;
	//cfg.dst_mode = LDC_DST_FLAT;

	uartlog("%s: 2nd src buf address Y(%#x) C(%#x)\n", __func__, cfg.src_y_base, cfg.src_c_base);
	uartlog("%s: 2nd dst buf address Y(%#x) C(%#x)\n", __func__, cfg.dst_y_base, cfg.dst_c_base);

	timer_tick = timer_get_tick();
	if (!use_cmdq) {
		ldc_intr_ctrl(1);
		ldc_engine(&cfg);
	} else {
		enable_cmdq = true;
		ldc_engine_cmdq(&cfg, cmdq_set);
	}

#ifndef ENV_EMU
	if (check_irq_handler(60) != 0) {
		uartlog("2nd ldc end intr doesn't happens after 60 ms\n");
		ret = -1;
	} else {
		uartlog("2nd ldc done\n");
	}
#endif

	return ret;
}

// 320x256, 2 pass ldc
static int ldc_test6(int argc, char **argv)
{
	int ret = 0;
	int use_cmdq = 0;
	struct ldc_cfg cfg;
	const u32 ldc_src_buf = src_addr_scl[0][0];
	const u32 ldc_map_base_1st = src_addr_scl[1][0];
	const u32 ldc_map_base_2nd = src_addr_scl[2][0];
	const u32 ldc_dst_buf_1st = dst_addr_scl[0][0];
	const u32 ldc_dst_buf_2nd = dst_addr_scl[1][0];

	// From 182x 20210930_ldc_output
	uartlog("0x%08x: ldc_320_256.ppm_NV21, in\n", ldc_src_buf);
	uartlog("0x%08x: ldc_320_256_srcx_1st_mesh, 1st mesh, in\n", ldc_map_base_1st);
	uartlog("0x%08x: ldc_320_256_srcx_2nd_mesh, 2nd mesh, in\n", ldc_map_base_2nd);
	uartlog("0x%08x: 256x320 NV21 packed, 1st out, ref ldc_src_320_256.ppm_NV21\n", ldc_dst_buf_1st);
	uartlog("0x%08x: 320x256 NV21 packed, 2nd out, ref ldc_dst_320_256_2nd_pass.ppm_NV21\n", ldc_dst_buf_2nd);

	// 1st pass
	memset(&cfg, 0, sizeof(cfg));
	cfg.pixel_fmt = 1;	// nv12/nv21
	cfg.map_bypass = 0;
	cfg.map_base = ldc_map_base_1st;

	cfg.src_width = 320;	// 64X
	cfg.src_height = 256;	// 64X
	cfg.ras_width = cfg.src_width;
	cfg.ras_height = cfg.src_height;

	cfg.src_xstart = 0;
	cfg.src_xend = cfg.src_width - 1;
	cfg.bgcolor = (128 << 8) | 128; // yuv black

	cfg.src_y_base = ldc_src_buf;
	cfg.src_c_base = cfg.src_y_base + cfg.src_width * cfg.src_height;
	cfg.dst_y_base = ldc_dst_buf_1st;
	cfg.dst_c_base = cfg.dst_y_base + cfg.src_width * cfg.src_height;
	cfg.dst_mode = LDC_DST_ROT_90;

	uartlog("%s: 1st src buf address Y(%#x) C(%#x)\n", __func__, cfg.src_y_base, cfg.src_c_base);
	uartlog("%s: 1st dst buf address Y(%#x) C(%#x)\n", __func__, cfg.dst_y_base, cfg.dst_c_base);

	timer_tick = timer_get_tick();
	if (!use_cmdq) {
		ldc_intr_ctrl(1);
		ldc_engine(&cfg);
	} else {
		enable_cmdq = true;
		ldc_engine_cmdq(&cfg, cmdq_set);
	}

#ifndef ENV_EMU
	if (check_irq_handler(600) != 0) {
		uartlog("1st ldc end intr doesn't happens after 60 ms\n");
		ret = -1;
		return ret;
	} else {
		uartlog("1st ldc done\n");
	}
#endif

	// 2nd pass
	cfg.map_base = ldc_map_base_2nd;

	cfg.src_width = 256;	// 64X
	cfg.src_height = 320;	// 64X
	cfg.ras_width = cfg.src_width;
	cfg.ras_height = cfg.src_height;

	cfg.src_xstart = 0;
	cfg.src_xend = cfg.src_width - 1;
	cfg.bgcolor = (128 << 8) | 128; // yuv black

	cfg.src_y_base = ldc_dst_buf_1st;
	cfg.src_c_base = cfg.src_y_base + cfg.src_width * cfg.src_height;
	cfg.dst_y_base = ldc_dst_buf_2nd;
	cfg.dst_c_base = cfg.dst_y_base + cfg.src_width * cfg.src_height;
	cfg.dst_mode = LDC_DST_ROT_270;

	uartlog("%s: 2nd src buf address Y(%#x) C(%#x)\n", __func__, cfg.src_y_base, cfg.src_c_base);
	uartlog("%s: 2nd dst buf address Y(%#x) C(%#x)\n", __func__, cfg.dst_y_base, cfg.dst_c_base);

	timer_tick = timer_get_tick();
	if (!use_cmdq) {
		ldc_intr_ctrl(1);
		ldc_engine(&cfg);
	} else {
		enable_cmdq = true;
		ldc_engine_cmdq(&cfg, cmdq_set);
	}

#ifndef ENV_EMU
	if (check_irq_handler(60) != 0) {
		uartlog("2nd ldc end intr doesn't happens after 60 ms\n");
		ret = -1;
	} else {
		uartlog("2nd ldc done\n");
	}
#endif

	return ret;
}

// 320x256, cmdq, bypass, flat
static int ldc_test7(int argc, char **argv)
{
	int ret = 0;
	int use_cmdq = 1;
	struct ldc_cfg cfg;
	const u32 ldc_src_buf = src_addr_scl[0][0];
	const u32 ldc_dst_buf = dst_addr_scl[0][0];

	// From 182x 20210930_ldc_output
	uartlog("0x%08x: ldc_src_320_256.ppm_NV21, in\n", ldc_src_buf);
	uartlog("0x%08x: 320x256 NV21 packed, out\n", ldc_dst_buf);

	// 1st pass
	memset(&cfg, 0, sizeof(cfg));
	cfg.pixel_fmt = 1;	// nv12/nv21
	cfg.map_bypass = 1;

	cfg.src_width = 320;	// 64X
	cfg.src_height = 256;	// 64X
	cfg.ras_width = cfg.src_width;
	cfg.ras_height = cfg.src_height;

	cfg.src_xstart = 0;
	cfg.src_xend = cfg.src_width - 1;
	cfg.bgcolor = (128 << 8) | 128; // yuv black

	cfg.src_y_base = ldc_src_buf;
	cfg.src_c_base = cfg.src_y_base + cfg.src_width * cfg.src_height;
	cfg.dst_y_base = ldc_dst_buf;
	cfg.dst_c_base = cfg.dst_y_base + cfg.src_width * cfg.src_height;
	cfg.dst_mode = LDC_DST_FLAT;

	cfg.use_cmdq = use_cmdq;

	uartlog("%s: 1st src buf address Y(%#x) C(%#x)\n", __func__, cfg.src_y_base, cfg.src_c_base);
	uartlog("%s: 1st dst buf address Y(%#x) C(%#x)\n", __func__, cfg.dst_y_base, cfg.dst_c_base);

	timer_tick = timer_get_tick();
	if (!use_cmdq) {
		ldc_intr_ctrl(1);
		ldc_engine(&cfg);
	} else {
		enable_cmdq = true;
		ldc_engine_cmdq(&cfg, cmdq_set);
	}

#ifndef ENV_EMU
	if (check_irq_handler(600) != 0) {
		uartlog("1st ldc end intr doesn't happens after 60 ms\n");
		ret = -1;
		return ret;
	} else {
		uartlog("1st ldc done\n");
	}
#endif

	return ret;
}

// 320x256 -> 256x320, bypass, rot270
static int ldc_test8(int argc, char **argv)
{
	int ret = 0;
	int use_cmdq = 0;
	struct ldc_cfg cfg;
	const u32 ldc_src_buf = 0x90000000;
	const u32 ldc_dst_buf = 0x96000000;

	// From 182x 20210930_ldc_output
	uartlog("0x90000000: ldc_src_320_256.ppm_NV21, in\n");
	uartlog("0x96000000: 256x320 NV21 packed, out\n");

	cfg.pixel_fmt = 1;	// nv12/nv21
	cfg.map_bypass = 1;

	cfg.src_width = 320;	// 64X
	cfg.src_height = 256;	// 64X
	cfg.ras_width = cfg.src_width;
	cfg.ras_height = cfg.src_height;

	cfg.src_xstart = 0;
	cfg.src_xend = cfg.src_width - 1;
	cfg.bgcolor = (128 << 8) | 128; // yuv black

	cfg.src_y_base = ldc_src_buf;
	cfg.src_c_base = cfg.src_y_base + cfg.src_width * cfg.src_height;
	cfg.dst_y_base = ldc_dst_buf;
	cfg.dst_c_base = cfg.dst_y_base + cfg.src_width * cfg.src_height;
	cfg.dst_mode = LDC_DST_ROT_270;

	cfg.use_cmdq = use_cmdq;

	uartlog("%s: src buf address Y(%#x) C(%#x)\n", __func__, cfg.src_y_base, cfg.src_c_base);
	uartlog("%s: dst buf address Y(%#x) C(%#x)\n", __func__, cfg.dst_y_base, cfg.dst_c_base);

	use_cmdq = 0;
	timer_tick = timer_get_tick();
	if (!use_cmdq) {
		ldc_intr_ctrl(1);
		ldc_engine(&cfg);
	} else {
		enable_cmdq = true;
		ldc_engine_cmdq(&cfg, cmdq_set);
	}

#ifndef ENV_EMU
	if (check_irq_handler(600) != 0) {
		uartlog("1st ldc end intr doesn't happens after 60 ms\n");
		ret = -1;
		return ret;
	} else {
		uartlog("1st ldc done\n");
	}
#endif

	return ret;
}

int ldc_to_sclr_sb_test_start_ldc(uint8_t ldc_fmt, enum ldc_dst_mode dst_mode, uint8_t sb_mode)
{
	int ret = 0;
	int use_cmdq = 0;
	struct ldc_cfg cfg;
	const u32 ldc_src_buf = 0x90000000;
	const u32 ldc_dst_buf = 0x96000000;
	u32 src_width = 320;
	u32 src_height = 256;
	uint8_t ldc_dir = (sb_mode && dst_mode!=LDC_DST_FLAT) ? 1 : 0;

	// From 182x 20210930_ldc_output
	uartlog("0x90000000: ldc_src_320_256.ppm_NV21, in\n");

	if (dst_mode == LDC_DST_ROT_90 || dst_mode == LDC_DST_ROT_270)
		uartlog("0x96000000: 256x320 fmt(%d) packed, out\n", ldc_fmt);
	else
		uartlog("0x96000000: 320x256 fmt(%d) packed, out\n", ldc_fmt);

	cfg.pixel_fmt = ldc_fmt;
	cfg.map_bypass = 1;

	cfg.src_width = ALIGNMENT(src_width, 64);	// 64X
	cfg.src_height = ALIGNMENT(src_height, 64);	// 64X
	cfg.ras_width = cfg.src_width;
	cfg.ras_height = cfg.src_height;

	// if((reg_sb_mode>0 && reg_dst_mode!=Flat) ldc_dir = 1, else ldc_dir = 0
	// reg_src_str = 0
	// reg_src_end = (reg_ldc_dir) ? (reg_src_ysize -1) : (reg_src_xsize - 1)
	cfg.src_xstart = 0;
	cfg.src_xend = ldc_dir ? (cfg.src_height - 1) : (cfg.src_width - 1);

	cfg.bgcolor = (128 << 8) | 128; // yuv black

	cfg.src_y_base = ldc_src_buf;
	cfg.src_c_base = cfg.src_y_base + cfg.src_width * cfg.src_height;
	cfg.dst_y_base = ldc_dst_buf;
	cfg.dst_c_base = cfg.dst_y_base + cfg.src_width * cfg.src_height;
	cfg.dst_mode = dst_mode;

	uartlog("%s: src buf address Y(%#x) C(%#x)\n", __func__, cfg.src_y_base, cfg.src_c_base);
	uartlog("%s: dst buf address Y(%#x) C(%#x)\n", __func__, cfg.dst_y_base, cfg.dst_c_base);

	timer_tick = timer_get_tick();
	if (!use_cmdq) {
		ldc_intr_ctrl(1);
		ldc_engine(&cfg);
	} else {
		enable_cmdq = true;
		ldc_engine_cmdq(&cfg, cmdq_set);
	}

#ifndef ENV_EMU
	if (check_irq_handler(600) != 0) {
		uartlog("ldc end intr doesn't happens after 60 ms\n");
		ret = -1;
		return ret;
	} else {
		uartlog("ldc done\n");
	}
#endif

	return 0;
}

static struct cmd_entry test_cmd_ldc_list[] __attribute__ ((unused)) = {
    {"\n[---LDC------------------------------------------------]", do_job, 0, ""},
    {"ldc_job",         do_job,         0, "[Modify settings]"},
    {"ldc_dump",        do_dump,        0, "[Dump register]"},
    {"ldc_test_1",      ldc_test1,      0, "[64x64, memcpy]"},
    {"ldc_test_2",      ldc_test2,      0, "[128x128, memcpy]"},
    {"ldc_test_3",      ldc_test3,      0, "[1920x1088, memcpy]"},
    {"ldc_test_4",      ldc_test4,      0, "[1920x1088, 2 pass ldc]"},
    {"ldc_test_5",      ldc_test5,      0, "[1984x1088, 2 pass ldc]"},
    {"ldc_test_6",      ldc_test6,      0, "[320x256, 2 pass ldc]"},
    {"ldc_test_7",      ldc_test7,      0, "[320x256, cmdq, bypass, flat]"},
    {"ldc_test_8",      ldc_test8,      0, "[320x256 -> 256x320, bypass, rot270]"},
    {NULL, NULL, 0, NULL}
};

int testcase_ldc(void)
{
	ldc_init();

	ldc_intr_ctrl(1);

#ifndef VIP_PERFORMANCE_TEST
	request_irq(LDC_INT, irq_handler_ldc, 0, "ldc int", NULL);
#endif

	for(int i = 0;i < ARRAY_SIZE(test_cmd_ldc_list) - 1;i++) {
	command_add(&test_cmd_ldc_list[i]);
	}

	return 0;
}

