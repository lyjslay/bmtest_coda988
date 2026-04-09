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
#include "mmio.h"
#define BIT(nr)                 (1UL << (nr))

static u64 timer_tick = 0;
static volatile int mark;
static bool enable_cmdq = false;

//#define BASE_OFFSET 0x100000000

//extern u64 src_addr_scl[3][3];
//extern u64 dst_addr_scl[3][3];
/*
u64 src_addr_ldc[4][3] = {
	{BASE_OFFSET + 0x00000000, BASE_OFFSET + 0x00400000, BASE_OFFSET + 0x00800000},
	{BASE_OFFSET + 0x00c00000, BASE_OFFSET + 0x01000000, BASE_OFFSET + 0x01400000},
	{BASE_OFFSET + 0x01800000, BASE_OFFSET + 0x01c00000, BASE_OFFSET + 0x02000000},
	{BASE_OFFSET + 0x02400000, BASE_OFFSET + 0x02800000, BASE_OFFSET + 0x02c00000},
};

u64 dst_addr_ldc[4][3] = {
	{BASE_OFFSET + 0x03000000, BASE_OFFSET + 0x03400000, BASE_OFFSET + 0x03800000},
	{BASE_OFFSET + 0x03C00000, BASE_OFFSET + 0x04000000, BASE_OFFSET + 0x04400000},
	{BASE_OFFSET + 0x04800000, BASE_OFFSET + 0x04C00000, BASE_OFFSET + 0x05000000},
	{BASE_OFFSET + 0x05400000, BASE_OFFSET + 0x05800000, BASE_OFFSET + 0x05C00000},
};
*/
#define ddr_offset  (0x81000000)

u64 src_addr_ldc[4][3] = {
    {ddr_offset + 0x000000,    ddr_offset + 0x200000,    ddr_offset + 0x400000},
    {ddr_offset + 0x600000,    ddr_offset + 0x700000,    ddr_offset + 0x800000},
    {ddr_offset + 0x900000,    ddr_offset + 0xa00000,    ddr_offset + 0xb00000},
};

u64 dst_addr_ldc[4][3] = {
    {ddr_offset + 0xc00000,    ddr_offset + 0xe00000,   ddr_offset + 0x1000000},
    {ddr_offset + 0x2000000,   ddr_offset + 0x2400000,   ddr_offset + 0x2600000},
    {ddr_offset + 0x2800000,   ddr_offset + 0x2a00000,   ddr_offset + 0x2c00000},
};
u64 cmdq_set = ddr_offset + 0x3000000;

static int irq_handler_ldc(int irqn, void *priv)
{
	u64 now = timer_get_tick();
	u8 intr_status;

	uartlog("\n%s: time-consumption(%d)ms\n", __func__, (now - timer_tick)/TIMER_CNT_MS);

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
	u8 status;

	for (i = 0; i < timeout; ++i) {
		if (mark > 0) {
			mark = 0;
			uartlog("%s: check tick:%d\n", __func__, i);
			return 0;
		}
		mdelay(1);
		status = ldc_intr_status();
		uartlog("ldc intr status(%d) after kicked\n", status);
	}

	return -1;
}

// 用户自己输入参数配置具体的testcase
static int do_job(int argc, char **argv)
{
	int ret = 0;
	int use_cmdq = 0;
	struct ldc_cfg cfg;
	u32 tmp;
	u64 ldc_src_buf = src_addr_ldc[0][0];
	u64 ldc_dst_buf = dst_addr_ldc[0][0];
	u64 ldc_map_base = src_addr_ldc[1][0];

	bm_get_num("pixel format, 1(NV12)/0(Y): ", cfg.pixel_fmt);
	bm_get_num("bypass, 1(Yes)/0(No): ", cfg.map_bypass);
	cfg.map_base = ldc_map_base;

	bm_get_num("src width : ", cfg.src_width);
	bm_get_num("src height : ", cfg.src_height);
	cfg.ras_width = cfg.src_width;
	cfg.ras_height = cfg.src_height;

	if (cfg.src_width >= 4096) {
		// ldc_src_buf = 0x105000000
		ldc_src_buf = ldc_src_buf - 0x2000000;		// 0x103000000
		ldc_dst_buf = ldc_src_buf + 0x2000000;		// 0x105000000
	}
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

	uartlog("%s: src buf address Y(%#llx) C(%#llx)\n", __func__, cfg.src_y_base, cfg.src_c_base);
	uartlog("%s: dst buf address Y(%#llx) C(%#llx)\n", __func__, cfg.dst_y_base, cfg.dst_c_base);

	bm_get_num("dst mode, 0(flat)/1(xy_flip)/2(rot 90)/3(rot 270): ", cfg.dst_mode);

	bm_get_num("reg/cmdq mode, 0(reg)/1(cmdq): ", cfg.use_cmdq);
	use_cmdq = cfg.use_cmdq;
	// cfg.extend_haddr = 0;
	cfg.extend_haddr = (((cfg.src_y_base >> 33) & 0x7)<<1 |((cfg.dst_y_base >> 33) & 0x7)<<13);

	timer_tick = timer_get_tick();
	if (!use_cmdq) {
		ldc_intr_ctrl(1);
		ldc_engine(&cfg);
		ldc_dump_register();		// dump ldc register
	} else {
		enable_cmdq = true;
		ldc_engine_cmdq(&cfg, cmdq_set);
	}

#ifndef ENV_EMU
	if (check_irq_handler(20) != 0) {
		uartlog("ldc end intr doesn't happens after 600 ms\n");
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

static int do_dump_ddr(int argc, char **argv)
{
	const u64 ldc_src_buf = src_addr_ldc[0][0];
	const u64 ldc_dst_buf = dst_addr_ldc[0][0];
	unsigned char *src = (unsigned char *)(uintptr_t)ldc_src_buf;
	unsigned char *dst = (unsigned char *)(uintptr_t)ldc_dst_buf;
	u8 i;

	for (i = 0; i < 16; i++) {
        	src[i] = 0xff;
		dst[i] = 0xaa;
		uartlog("%d\n", i);

	}

	return 0;
}

static int ldc_test1(int argc, char **argv)
{
	int ret = 0;
	int use_cmdq = 0;
	struct ldc_cfg cfg;
	// u32 tmp;
	u64 ldc_src_buf = src_addr_ldc[0][0];
	u64 ldc_dst_buf = dst_addr_ldc[0][0];
	u64 ldc_map_base = src_addr_ldc[1][0];

	cfg.pixel_fmt = 1;		// nv12
	cfg.map_bypass = 1;
	cfg.map_base = ldc_map_base;
	cfg.src_width = 1920;
	cfg.src_height = 1088;
	cfg.ras_width = cfg.src_width;
	cfg.ras_height = cfg.src_height;
	if (cfg.src_width >= 4096) {
		// ldc_src_buf = 0x105000000
		ldc_src_buf = ldc_src_buf - 0x2000000;		// 0x103000000
		ldc_dst_buf = ldc_src_buf + 0x2000000;		// 0x105000000
	}
	if (cfg.src_width & 0x3f) {
		uartlog("%s: width must be 64x\n", __func__);
		return -1;
	}
	cfg.src_xstart = 0;
	cfg.src_xend = cfg.src_width - 1;
	cfg.bgcolor = (128 << 8) | 128; // yuv black

	cfg.src_y_base = ldc_src_buf;
	cfg.src_c_base = cfg.src_y_base + cfg.src_width * cfg.src_height;
	cfg.dst_y_base = ldc_dst_buf;
	cfg.dst_c_base = cfg.dst_y_base + cfg.src_width * cfg.src_height;
	uartlog("%s: src buf address Y(%#llx) C(%#llx)\n", __func__, cfg.src_y_base, cfg.src_c_base);
	uartlog("%s: dst buf address Y(%#llx) C(%#llx)\n", __func__, cfg.dst_y_base, cfg.dst_c_base);

	cfg.dst_mode = 0;		// flat
	cfg.use_cmdq = 0;
	use_cmdq = cfg.use_cmdq;
	// cfg.extend_haddr = 0;
	cfg.extend_haddr = (((cfg.src_y_base >> 33) & 0x7)<<1 |((cfg.dst_y_base >> 33) & 0x7)<<13);

	timer_tick = timer_get_tick();
	if (!use_cmdq) {
		ldc_intr_ctrl(1);
		ldc_engine(&cfg);
		ldc_dump_register();		// dump ldc register
	} else {
		enable_cmdq = true;
		ldc_engine_cmdq(&cfg, cmdq_set);
	}

#ifndef ENV_EMU
	if (check_irq_handler(20) != 0) {
		uartlog("ldc end intr doesn't happens after 600 ms\n");
		ret = -1;
	} else {
		uartlog("ldc done\n");
	}
#endif
	return ret;
}


// testcase-2
// 1920x1088-nv12-xyflip
static int ldc_test2(int argc, char **argv)
{
	int ret = 0;
	int use_cmdq = 0;
	struct ldc_cfg cfg;
	// u32 tmp;
	u64 ldc_src_buf = src_addr_ldc[0][0];
	u64 ldc_dst_buf = dst_addr_ldc[0][0];
	u64 ldc_map_base = src_addr_ldc[1][0];

	cfg.pixel_fmt = 1;		// nv12
	cfg.map_bypass = 1;
	cfg.map_base = ldc_map_base;
	cfg.src_width = 1920;
	cfg.src_height = 1088;
	cfg.ras_width = cfg.src_width;
	cfg.ras_height = cfg.src_height;
	if (cfg.src_width >= 4096) {
		// ldc_src_buf = 0x105000000
		ldc_src_buf = ldc_src_buf - 0x2000000;		// 0x103000000
		ldc_dst_buf = ldc_src_buf + 0x2000000;		// 0x105000000
	}
	if (cfg.src_width & 0x3f) {
		uartlog("%s: width must be 64x\n", __func__);
		return -1;
	}
	cfg.src_xstart = 0;
	cfg.src_xend = cfg.src_width - 1;
	cfg.bgcolor = (128 << 8) | 128; // yuv black

	cfg.src_y_base = ldc_src_buf;
	cfg.src_c_base = cfg.src_y_base + cfg.src_width * cfg.src_height;
	cfg.dst_y_base = ldc_dst_buf;
	cfg.dst_c_base = cfg.dst_y_base + cfg.src_width * cfg.src_height;
	uartlog("%s: src buf address Y(%#llx) C(%#llx)\n", __func__, cfg.src_y_base, cfg.src_c_base);
	uartlog("%s: dst buf address Y(%#llx) C(%#llx)\n", __func__, cfg.dst_y_base, cfg.dst_c_base);

	cfg.dst_mode = 1;		// xy-flip
	cfg.use_cmdq = 0;
	use_cmdq = cfg.use_cmdq;
	// cfg.extend_haddr = 0;
	cfg.extend_haddr = (((cfg.src_y_base >> 33) & 0x7)<<1 |((cfg.dst_y_base >> 33) & 0x7)<<13);

	timer_tick = timer_get_tick();
	if (!use_cmdq) {
		ldc_intr_ctrl(1);
		ldc_engine(&cfg);
		ldc_dump_register();
	} else {
		enable_cmdq = true;
		ldc_engine_cmdq(&cfg, cmdq_set);
	}

#ifndef ENV_EMU
	if (check_irq_handler(20) != 0) {
		uartlog("ldc end intr doesn't happens after 600 ms\n");
		ret = -1;
	} else {
		uartlog("ldc done\n");
	}
#endif
	return ret;
}

// testcase-3
// 1920x1088-nv12-rot90
static int ldc_test3(int argc, char **argv)
{
	int ret = 0;
	int use_cmdq = 0;
	struct ldc_cfg cfg;
	// u32 tmp;
	u64 ldc_src_buf = src_addr_ldc[0][0];
	u64 ldc_dst_buf = dst_addr_ldc[0][0];
	u64 ldc_map_base = src_addr_ldc[1][0];

	cfg.pixel_fmt = 1;		// nv12
	cfg.map_bypass = 1;
	cfg.map_base = ldc_map_base;
	cfg.src_width = 1920;
	cfg.src_height = 1088;
	cfg.ras_width = cfg.src_width;
	cfg.ras_height = cfg.src_height;
	if (cfg.src_width >= 4096) {
		// ldc_src_buf = 0x105000000
		ldc_src_buf = ldc_src_buf - 0x2000000;		// 0x103000000
		ldc_dst_buf = ldc_src_buf + 0x2000000;		// 0x105000000
	}
	if (cfg.src_width & 0x3f) {
		uartlog("%s: width must be 64x\n", __func__);
		return -1;
	}
	cfg.src_xstart = 0;
	cfg.src_xend = cfg.src_width - 1;
	cfg.bgcolor = (128 << 8) | 128; // yuv black

	cfg.src_y_base = ldc_src_buf;
	cfg.src_c_base = cfg.src_y_base + cfg.src_width * cfg.src_height;
	cfg.dst_y_base = ldc_dst_buf;
	cfg.dst_c_base = cfg.dst_y_base + cfg.src_width * cfg.src_height;
	uartlog("%s: src buf address Y(%#llx) C(%#llx)\n", __func__, cfg.src_y_base, cfg.src_c_base);
	uartlog("%s: dst buf address Y(%#llx) C(%#llx)\n", __func__, cfg.dst_y_base, cfg.dst_c_base);

	cfg.dst_mode = 2;		// rot90
	cfg.use_cmdq = 0;
	use_cmdq = cfg.use_cmdq;
	// cfg.extend_haddr = 0;
	cfg.extend_haddr = (((cfg.src_y_base >> 33) & 0x7)<<1 |((cfg.dst_y_base >> 33) & 0x7)<<13);

	timer_tick = timer_get_tick();
	if (!use_cmdq) {
		ldc_intr_ctrl(1);
		ldc_engine(&cfg);
		ldc_dump_register();
	} else {
		enable_cmdq = true;
		ldc_engine_cmdq(&cfg, cmdq_set);
	}

#ifndef ENV_EMU
	if (check_irq_handler(20) != 0) {
		uartlog("ldc end intr doesn't happens after 600 ms\n");
		ret = -1;
	} else {
		uartlog("ldc done\n");
	}
#endif
	return ret;
}

// testcase-4
// 1920x1088-nv12-rot270
static int ldc_test4(int argc, char **argv)
{
	int ret = 0;
	int use_cmdq = 0;
	struct ldc_cfg cfg;
	// u32 tmp;
	u64 ldc_src_buf = src_addr_ldc[0][0];
	u64 ldc_dst_buf = dst_addr_ldc[0][0];
	u64 ldc_map_base = src_addr_ldc[1][0];

	cfg.pixel_fmt = 1;		// nv12
	cfg.map_bypass = 1;
	cfg.map_base = ldc_map_base;
	cfg.src_width = 1920;
	cfg.src_height = 1088;
	cfg.ras_width = cfg.src_width;
	cfg.ras_height = cfg.src_height;
	if (cfg.src_width >= 4096) {
		// ldc_src_buf = 0x105000000
		ldc_src_buf = ldc_src_buf - 0x2000000;		// 0x103000000
		ldc_dst_buf = ldc_src_buf + 0x2000000;		// 0x105000000
	}
	if (cfg.src_width & 0x3f) {
		uartlog("%s: width must be 64x\n", __func__);
		return -1;
	}
	cfg.src_xstart = 0;
	cfg.src_xend = cfg.src_width - 1;
	cfg.bgcolor = (128 << 8) | 128; // yuv black

	cfg.src_y_base = ldc_src_buf;
	cfg.src_c_base = cfg.src_y_base + cfg.src_width * cfg.src_height;
	cfg.dst_y_base = ldc_dst_buf;
	cfg.dst_c_base = cfg.dst_y_base + cfg.src_width * cfg.src_height;
	uartlog("%s: src buf address Y(%#llx) C(%#llx)\n", __func__, cfg.src_y_base, cfg.src_c_base);
	uartlog("%s: dst buf address Y(%#llx) C(%#llx)\n", __func__, cfg.dst_y_base, cfg.dst_c_base);

	cfg.dst_mode = 3;		// rot270
	cfg.use_cmdq = 0;
	use_cmdq = cfg.use_cmdq;
	// cfg.extend_haddr = 0;
	cfg.extend_haddr = (((cfg.src_y_base >> 33) & 0x7)<<1 |((cfg.dst_y_base >> 33) & 0x7)<<13);

	timer_tick = timer_get_tick();
	if (!use_cmdq) {
		ldc_intr_ctrl(1);
		ldc_engine(&cfg);
		ldc_dump_register();
	} else {
		enable_cmdq = true;
		ldc_engine_cmdq(&cfg, cmdq_set);
	}

#ifndef ENV_EMU
	if (check_irq_handler(20) != 0) {
		uartlog("ldc end intr doesn't happens after 600 ms\n");
		ret = -1;
	} else {
		uartlog("ldc done\n");
	}
#endif
	return ret;
}


// testcase-5
// 1920x1088-Y-only-flat
static int ldc_test5(int argc, char **argv)
{
	int ret = 0;
	int use_cmdq = 0;
	struct ldc_cfg cfg;
	// u32 tmp;
	u64 ldc_src_buf = src_addr_ldc[0][0];
	u64 ldc_dst_buf = dst_addr_ldc[0][0];
	u64 ldc_map_base = src_addr_ldc[1][0];

	cfg.pixel_fmt = 0;		// Y-only
	cfg.map_bypass = 1;
	cfg.map_base = ldc_map_base;
	cfg.src_width = 1920;
	cfg.src_height = 1088;
	cfg.ras_width = cfg.src_width;
	cfg.ras_height = cfg.src_height;
	if (cfg.src_width >= 4096) {
		// ldc_src_buf = 0x105000000
		ldc_src_buf = ldc_src_buf - 0x2000000;		// 0x103000000
		ldc_dst_buf = ldc_src_buf + 0x2000000;		// 0x105000000
	}
	if (cfg.src_width & 0x3f) {
		uartlog("%s: width must be 64x\n", __func__);
		return -1;
	}
	cfg.src_xstart = 0;
	cfg.src_xend = cfg.src_width - 1;
	cfg.bgcolor = (128 << 8) | 128; // yuv black

	cfg.src_y_base = ldc_src_buf;
	cfg.src_c_base = cfg.src_y_base + cfg.src_width * cfg.src_height;
	cfg.dst_y_base = ldc_dst_buf;
	cfg.dst_c_base = cfg.dst_y_base + cfg.src_width * cfg.src_height;
	uartlog("%s: src buf address Y(%#llx) C(%#llx)\n", __func__, cfg.src_y_base, cfg.src_c_base);
	uartlog("%s: dst buf address Y(%#llx) C(%#llx)\n", __func__, cfg.dst_y_base, cfg.dst_c_base);

	cfg.dst_mode = 0;		// flat
	cfg.use_cmdq = 0;
	use_cmdq = cfg.use_cmdq;
	// cfg.extend_haddr = 0;
	cfg.extend_haddr = (((cfg.src_y_base >> 33) & 0x7)<<1 |((cfg.dst_y_base >> 33) & 0x7)<<13);

	timer_tick = timer_get_tick();
	if (!use_cmdq) {
		ldc_intr_ctrl(1);
		ldc_engine(&cfg);
		ldc_dump_register();
	} else {
		enable_cmdq = true;
		ldc_engine_cmdq(&cfg, cmdq_set);
	}

#ifndef ENV_EMU
	if (check_irq_handler(20) != 0) {
		uartlog("ldc end intr doesn't happens after 600 ms\n");
		ret = -1;
	} else {
		uartlog("ldc done\n");
	}
#endif
	return ret;
}


// testcase-6
// 1920x1088-Y-only-flip
static int ldc_test6(int argc, char **argv)
{
	int ret = 0;
	int use_cmdq = 0;
	struct ldc_cfg cfg;
	// u32 tmp;
	u64 ldc_src_buf = src_addr_ldc[0][0];
	u64 ldc_dst_buf = dst_addr_ldc[0][0];
	u64 ldc_map_base = src_addr_ldc[1][0];

	cfg.pixel_fmt = 0;		// Y-only
	cfg.map_bypass = 1;
	cfg.map_base = ldc_map_base;
	cfg.src_width = 1920;
	cfg.src_height = 1088;
	cfg.ras_width = cfg.src_width;
	cfg.ras_height = cfg.src_height;
	if (cfg.src_width >= 4096) {
		// ldc_src_buf = 0x105000000
		ldc_src_buf = ldc_src_buf - 0x2000000;		// 0x103000000
		ldc_dst_buf = ldc_src_buf + 0x2000000;		// 0x105000000
	}
	if (cfg.src_width & 0x3f) {
		uartlog("%s: width must be 64x\n", __func__);
		return -1;
	}
	cfg.src_xstart = 0;
	cfg.src_xend = cfg.src_width - 1;
	cfg.bgcolor = (128 << 8) | 128; // yuv black

	cfg.src_y_base = ldc_src_buf;
	cfg.src_c_base = cfg.src_y_base + cfg.src_width * cfg.src_height;
	cfg.dst_y_base = ldc_dst_buf;
	cfg.dst_c_base = cfg.dst_y_base + cfg.src_width * cfg.src_height;
	uartlog("%s: src buf address Y(%#llx) C(%#llx)\n", __func__, cfg.src_y_base, cfg.src_c_base);
	uartlog("%s: dst buf address Y(%#llx) C(%#llx)\n", __func__, cfg.dst_y_base, cfg.dst_c_base);

	cfg.dst_mode = 1;		// xy-flip
	cfg.use_cmdq = 0;
	use_cmdq = cfg.use_cmdq;
	// cfg.extend_haddr = 0;
	cfg.extend_haddr = (((cfg.src_y_base >> 33) & 0x7)<<1 |((cfg.dst_y_base >> 33) & 0x7)<<13);

	timer_tick = timer_get_tick();
	if (!use_cmdq) {
		ldc_intr_ctrl(1);
		ldc_engine(&cfg);
		ldc_dump_register();
	} else {
		enable_cmdq = true;
		ldc_engine_cmdq(&cfg, cmdq_set);
	}

#ifndef ENV_EMU
	if (check_irq_handler(20) != 0) {
		uartlog("ldc end intr doesn't happens after 600 ms\n");
		ret = -1;
	} else {
		uartlog("ldc done\n");
	}
#endif
	return ret;
}

// testcase-7
// 1920x1088-Y-only-rot90
static int ldc_test7(int argc, char **argv)
{
	int ret = 0;
	int use_cmdq = 0;
	struct ldc_cfg cfg;
	// u32 tmp;
	u64 ldc_src_buf = src_addr_ldc[0][0];
	u64 ldc_dst_buf = dst_addr_ldc[0][0];
	u64 ldc_map_base = src_addr_ldc[1][0];

	cfg.pixel_fmt = 0;		// Y-only
	cfg.map_bypass = 1;
	cfg.map_base = ldc_map_base;
	cfg.src_width = 1920;
	cfg.src_height = 1088;
	cfg.ras_width = cfg.src_width;
	cfg.ras_height = cfg.src_height;
	if (cfg.src_width >= 4096) {
		// ldc_src_buf = 0x105000000
		ldc_src_buf = ldc_src_buf - 0x2000000;		// 0x103000000
		ldc_dst_buf = ldc_src_buf + 0x2000000;		// 0x105000000
	}
	if (cfg.src_width & 0x3f) {
		uartlog("%s: width must be 64x\n", __func__);
		return -1;
	}
	cfg.src_xstart = 0;
	cfg.src_xend = cfg.src_width - 1;
	cfg.bgcolor = (128 << 8) | 128; // yuv black

	cfg.src_y_base = ldc_src_buf;
	cfg.src_c_base = cfg.src_y_base + cfg.src_width * cfg.src_height;
	cfg.dst_y_base = ldc_dst_buf;
	cfg.dst_c_base = cfg.dst_y_base + cfg.src_width * cfg.src_height;
	uartlog("%s: src buf address Y(%#llx) C(%#llx)\n", __func__, cfg.src_y_base, cfg.src_c_base);
	uartlog("%s: dst buf address Y(%#llx) C(%#llx)\n", __func__, cfg.dst_y_base, cfg.dst_c_base);

	cfg.dst_mode = 2;		// rot90
	cfg.use_cmdq = 0;
	use_cmdq = cfg.use_cmdq;
	// cfg.extend_haddr = 0;
	cfg.extend_haddr = (((cfg.src_y_base >> 33) & 0x7)<<1 |((cfg.dst_y_base >> 33) & 0x7)<<13);

	timer_tick = timer_get_tick();
	if (!use_cmdq) {
		ldc_intr_ctrl(1);
		ldc_engine(&cfg);
		ldc_dump_register();
	} else {
		enable_cmdq = true;
		ldc_engine_cmdq(&cfg, cmdq_set);
	}

#ifndef ENV_EMU
	if (check_irq_handler(20) != 0) {
		uartlog("ldc end intr doesn't happens after 600 ms\n");
		ret = -1;
	} else {
		uartlog("ldc done\n");
	}
#endif
	return ret;
}

// testcase-8
// 1920x1088-Y-only-rot270
static int ldc_test8(int argc, char **argv)
{
	int ret = 0;
	int use_cmdq = 0;
	struct ldc_cfg cfg;
	// u32 tmp;
	u64 ldc_src_buf = src_addr_ldc[0][0];
	u64 ldc_dst_buf = dst_addr_ldc[0][0];
	u64 ldc_map_base = src_addr_ldc[1][0];

	cfg.pixel_fmt = 0;		// Y-only
	cfg.map_bypass = 1;
	cfg.map_base = ldc_map_base;
	cfg.src_width = 1920;
	cfg.src_height = 1088;
	cfg.ras_width = cfg.src_width;
	cfg.ras_height = cfg.src_height;
	if (cfg.src_width >= 4096) {
		// ldc_src_buf = 0x105000000
		ldc_src_buf = ldc_src_buf - 0x2000000;		// 0x103000000
		ldc_dst_buf = ldc_src_buf + 0x2000000;		// 0x105000000
	}
	if (cfg.src_width & 0x3f) {
		uartlog("%s: width must be 64x\n", __func__);
		return -1;
	}
	cfg.src_xstart = 0;
	cfg.src_xend = cfg.src_width - 1;
	cfg.bgcolor = (128 << 8) | 128; // yuv black

	cfg.src_y_base = ldc_src_buf;
	cfg.src_c_base = cfg.src_y_base + cfg.src_width * cfg.src_height;
	cfg.dst_y_base = ldc_dst_buf;
	cfg.dst_c_base = cfg.dst_y_base + cfg.src_width * cfg.src_height;
	uartlog("%s: src buf address Y(%#llx) C(%#llx)\n", __func__, cfg.src_y_base, cfg.src_c_base);
	uartlog("%s: dst buf address Y(%#llx) C(%#llx)\n", __func__, cfg.dst_y_base, cfg.dst_c_base);

	cfg.dst_mode = 3;		// rot270
	cfg.use_cmdq = 0;
	use_cmdq = cfg.use_cmdq;
	// cfg.extend_haddr = 0;
	cfg.extend_haddr = (((cfg.src_y_base >> 33) & 0x7)<<1 |((cfg.dst_y_base >> 33) & 0x7)<<13);

	timer_tick = timer_get_tick();
	if (!use_cmdq) {
		ldc_intr_ctrl(1);
		ldc_engine(&cfg);
		ldc_dump_register();
	} else {
		enable_cmdq = true;
		ldc_engine_cmdq(&cfg, cmdq_set);
	}

#ifndef ENV_EMU
	if (check_irq_handler(20) != 0) {
		uartlog("ldc end intr doesn't happens after 600 ms\n");
		ret = -1;
	} else {
		uartlog("ldc done\n");
	}
#endif
	return ret;
}

// testcase-9
// LDC 2 phase
// 320x256, 2 pass ldc
static int ldc_test9(int argc, char **argv)
{
	int ret = 0;
	int use_cmdq = 0;
	struct ldc_cfg cfg;
	const u64 ldc_src_buf      = src_addr_ldc[0][0];
	const u64 ldc_map_base_1st = src_addr_ldc[1][0];
	const u64 ldc_map_base_2nd = src_addr_ldc[2][0];
	const u64 ldc_dst_buf_1st  = dst_addr_ldc[0][0];
	const u64 ldc_dst_buf_2nd  = dst_addr_ldc[1][0];

	// From 182x 20210930_ldc_output
	uartlog("(%#llx) ldc_320_256.ppm_NV12, in\n", ldc_src_buf);
	uartlog("(%#llx) ldc_320_256_srcx_1st_mesh, 1st mesh, in\n", ldc_map_base_1st);
	uartlog("(%#llx) ldc_320_256_srcx_2nd_mesh, 2nd mesh, in\n", ldc_map_base_2nd);
	uartlog("(%#llx) 256x320 NV12 packed, 1st out, ref ldc_src_320_256.ppm_NV12\n", ldc_dst_buf_1st);
	uartlog("(%#llx) 320x256 NV12 packed, 2nd out, ref ldc_dst_320_256_2nd_pass.ppm_NV12\n", ldc_dst_buf_2nd);

	// 1st pass
	memset(&cfg, 0, sizeof(cfg));
	cfg.pixel_fmt  = 1;	// nv12
	cfg.map_bypass = 0;
	cfg.map_base   = ldc_map_base_1st;

	cfg.src_width  = 320;	// 64X
	cfg.src_height = 256;	// 64X
	cfg.ras_width  = cfg.src_width;
	cfg.ras_height = cfg.src_height;

	cfg.src_xstart = 0;
	cfg.src_xend   = cfg.src_width - 1;
	cfg.bgcolor    = (128 << 8) | 128; // yuv black
	cfg.extend_haddr = (((cfg.src_y_base >> 33) & 0x7)<<1 |((cfg.dst_y_base >> 33) & 0x7)<<13);

	cfg.src_y_base = ldc_src_buf;
	cfg.src_c_base = cfg.src_y_base + cfg.src_width * cfg.src_height;
	cfg.dst_y_base = ldc_dst_buf_1st;
	cfg.dst_c_base = cfg.dst_y_base + cfg.src_width * cfg.src_height;
	cfg.dst_mode   = LDC_DST_ROT_90;

	uartlog("%s: 1st src buf address Y(%#llx) C(%#llx)\n", __func__, cfg.src_y_base, cfg.src_c_base);
	uartlog("%s: 1st dst buf address Y(%#llx) C(%#llx)\n", __func__, cfg.dst_y_base, cfg.dst_c_base);

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
		uartlog("1st ldc end intr doesn't happens after 600 ms\n");
		ret = -1;
		return ret;
	} else {
		uartlog("1st ldc done\n");
	}
#endif

	// 2nd pass
	cfg.map_base = ldc_map_base_2nd;

	cfg.src_width  = 256;	// 64X
	cfg.src_height = 320;	// 64X
	cfg.ras_width  = cfg.src_width;
	cfg.ras_height = cfg.src_height;

	cfg.src_xstart = 0;
	cfg.src_xend   = cfg.src_width - 1;
	cfg.bgcolor    = (128 << 8) | 128; // yuv black

	cfg.src_y_base = ldc_dst_buf_1st;
	cfg.src_c_base = cfg.src_y_base + cfg.src_width * cfg.src_height;
	cfg.dst_y_base = ldc_dst_buf_2nd;
	cfg.dst_c_base = cfg.dst_y_base + cfg.src_width * cfg.src_height;
	cfg.dst_mode   = LDC_DST_ROT_270;

	uartlog("%s: 2nd src buf address Y(%#llx) C(%#llx)\n", __func__, cfg.src_y_base, cfg.src_c_base);
	uartlog("%s: 2nd dst buf address Y(%#llx) C(%#llx)\n", __func__, cfg.dst_y_base, cfg.dst_c_base);

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
		uartlog("2nd ldc end intr doesn't happens after 600 ms\n");
		ret = -1;
	} else {
		uartlog("2nd ldc done\n");
	}
#endif

	return ret;
}

// testcase-10
// Command Queue
// 320x256, cmdq, bypass, flat
static int ldc_test10(int argc, char **argv)
{
	int ret = 0;
	int use_cmdq = 1;
	struct ldc_cfg cfg;
	const u64 ldc_src_buf = src_addr_ldc[0][0];
	const u64 ldc_dst_buf = dst_addr_ldc[0][0];

	// From 182x 20210930_ldc_output
	uartlog("0x%08x: ldc_src_320_256.ppm_NV12, in\n", ldc_src_buf);
	uartlog("0x%08x: 320x256 NV12 packed, out\n", ldc_dst_buf);

	// 1st pass
	memset(&cfg, 0, sizeof(cfg));
	cfg.pixel_fmt  = 1;	// nv12
	cfg.map_bypass = 1;

	cfg.src_width  = 320;	// 64X
	cfg.src_height = 256;	// 64X
	cfg.ras_width  = cfg.src_width;
	cfg.ras_height = cfg.src_height;

	cfg.src_xstart = 0;
	cfg.src_xend   = cfg.src_width - 1;
	cfg.bgcolor    = (128 << 8) | 128; // yuv black

	cfg.src_y_base = ldc_src_buf;
	cfg.src_c_base = cfg.src_y_base + cfg.src_width * cfg.src_height;
	cfg.dst_y_base = ldc_dst_buf;
	cfg.dst_c_base = cfg.dst_y_base + cfg.src_width * cfg.src_height;
	cfg.dst_mode   = LDC_DST_FLAT;
	cfg.extend_haddr = (((cfg.src_y_base >> 33) & 0x7)<<1 |((cfg.dst_y_base >> 33) & 0x7)<<13);

	cfg.use_cmdq = use_cmdq;

	uartlog("%s: 1st src buf address Y(%#llx) C(%#llx)\n", __func__, cfg.src_y_base, cfg.src_c_base);
	uartlog("%s: 1st dst buf address Y(%#llx) C(%#llx)\n", __func__, cfg.dst_y_base, cfg.dst_c_base);

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
		uartlog("1st ldc end intr doesn't happens after 600 ms\n");
		ret = -1;
		return ret;
	} else {
		uartlog("ldc cmdq done\n");
	}
#endif

	return ret;
}

// testcase-11
// 2560x1472-nv12-flat
static int ldc_test11(int argc, char **argv)
{
	int ret = 0;
	int use_cmdq = 0;
	struct ldc_cfg cfg;
	// u32 tmp;
	u64 ldc_src_buf = src_addr_ldc[0][0];
	u64 ldc_dst_buf = dst_addr_ldc[0][0];
	u64 ldc_map_base = src_addr_ldc[1][0];

	cfg.pixel_fmt = 1;		// nv12
	cfg.map_bypass = 1;
	cfg.map_base = ldc_map_base;
	cfg.src_width = 2560;
	cfg.src_height = 1472;
	cfg.ras_width = cfg.src_width;
	cfg.ras_height = cfg.src_height;
	if (cfg.src_width >= 4096) {
		// ldc_src_buf = 0x105000000
		ldc_src_buf = ldc_src_buf - 0x2000000;		// 0x103000000
		ldc_dst_buf = ldc_src_buf + 0x2000000;		// 0x105000000
	}
	if (cfg.src_width & 0x3f) {
		uartlog("%s: width must be 64x\n", __func__);
		return -1;
	}
	cfg.src_xstart = 0;
	cfg.src_xend = cfg.src_width - 1;
	cfg.bgcolor = (128 << 8) | 128; // yuv black

	cfg.src_y_base = ldc_src_buf;
	cfg.src_c_base = cfg.src_y_base + cfg.src_width * cfg.src_height;
	cfg.dst_y_base = ldc_dst_buf;
	cfg.dst_c_base = cfg.dst_y_base + cfg.src_width * cfg.src_height;
	uartlog("%s: src buf address Y(%#llx) C(%#llx)\n", __func__, cfg.src_y_base, cfg.src_c_base);
	uartlog("%s: dst buf address Y(%#llx) C(%#llx)\n", __func__, cfg.dst_y_base, cfg.dst_c_base);

	cfg.dst_mode = 0;		// flat
	cfg.use_cmdq = 0;
	use_cmdq = cfg.use_cmdq;
	// cfg.extend_haddr = 0;
	cfg.extend_haddr = (((cfg.src_y_base >> 33) & 0x7)<<1 |((cfg.dst_y_base >> 33) & 0x7)<<13);

	timer_tick = timer_get_tick();
	if (!use_cmdq) {
		ldc_intr_ctrl(1);
		ldc_engine(&cfg);
		ldc_dump_register();
	} else {
		enable_cmdq = true;
		ldc_engine_cmdq(&cfg, cmdq_set);
	}

#ifndef ENV_EMU
	if (check_irq_handler(20) != 0) {
		uartlog("ldc end intr doesn't happens after 600 ms\n");
		ret = -1;
	} else {
		uartlog("ldc done\n");
	}
#endif
	return ret;
}

// testcase-12
// LDC 2 phase
// 2560x1472, 2 pass ldc
static int ldc_test12(int argc, char **argv)
{
	int ret = 0;
	int use_cmdq = 0;
	int i = 0;
	int times = 1;
	u32 tmp;
	struct ldc_cfg cfg;
	const u64 ldc_src_buf      = src_addr_ldc[0][0];	// 0x105000000
	const u64 ldc_map_base_1st = src_addr_ldc[1][0];	// 0x105600000
	const u64 ldc_map_base_2nd = src_addr_ldc[2][0];	// 0x105900000
	const u64 ldc_dst_buf_1st  = dst_addr_ldc[0][0];	// 0x105C00000
	const u64 ldc_dst_buf_2nd  = dst_addr_ldc[1][0];	// 0x107000000

	// From 182x 20210930_ldc_output
	uartlog("(%#llx) 2560x1472_nv12, in\n", ldc_src_buf);
	uartlog("(%#llx) 2560x1472-srcx_1st_mesh.bin, 1st mesh, in\n", ldc_map_base_1st);
	uartlog("(%#llx) 2560x1472-srcx_2nd_mesh.bin, 2nd mesh, in\n", ldc_map_base_2nd);
	uartlog("(%#llx) 2560x1472 NV12 packed, 1st out, ref ldc_src_2560x1472.ppm_NV12\n", ldc_dst_buf_1st);
	uartlog("(%#llx) 2560x1472 NV12 packed, 2nd out, ref ldc_dst_2560x1472_2nd_pass.ppm_NV12\n", ldc_dst_buf_2nd);

	// 1st pass
	memset(&cfg, 0, sizeof(cfg));
	cfg.pixel_fmt  = 1;	// nv12
	cfg.map_bypass = 0;
	cfg.map_base   = ldc_map_base_1st;

	cfg.src_width  = 2560;	// 64X
	cfg.src_height = 1472;	// 64X
	cfg.ras_width  = cfg.src_width;
	cfg.ras_height = cfg.src_height;

	cfg.src_xstart = 0;
	cfg.src_xend   = cfg.src_width - 1;
	cfg.bgcolor    = (128 << 8) | 128; // yuv black
	cfg.extend_haddr = (((cfg.src_y_base >> 33) & 0x7)<<1 |((cfg.dst_y_base >> 33) & 0x7)<<13);

	cfg.src_y_base = ldc_src_buf;
	cfg.src_c_base = cfg.src_y_base + cfg.src_width * cfg.src_height;
	cfg.dst_y_base = ldc_dst_buf_1st;
	cfg.dst_c_base = cfg.dst_y_base + cfg.src_width * cfg.src_height;
	cfg.dst_mode   = LDC_DST_ROT_270;

	uartlog("%s: 1st src buf address Y(%#llx) C(%#llx)\n", __func__, cfg.src_y_base, cfg.src_c_base);
	uartlog("%s: 1st dst buf address Y(%#llx) C(%#llx)\n", __func__, cfg.dst_y_base, cfg.dst_c_base);
	bm_get_num("times: ", times);
	for (i = 0; i < times; i++) {
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
			uartlog("1st ldc end intr doesn't happens after 600 ms\n");
			ret = -1;
			return ret;
		} else {
			uartlog("1st ldc done\n");
		}
#endif

		// 2nd pass
		cfg.map_base = ldc_map_base_2nd;

		cfg.src_width  = 1472;	// 64X
		cfg.src_height = 2560;	// 64X
		cfg.ras_width  = cfg.src_width;
		cfg.ras_height = cfg.src_height;

		cfg.src_xstart = 0;
		cfg.src_xend   = cfg.src_width - 1;
		cfg.bgcolor    = (128 << 8) | 128; // yuv black

		cfg.src_y_base = ldc_dst_buf_1st;
		cfg.src_c_base = cfg.src_y_base + cfg.src_width * cfg.src_height;
		cfg.dst_y_base = ldc_dst_buf_2nd;
		cfg.dst_c_base = cfg.dst_y_base + cfg.src_width * cfg.src_height;
		cfg.dst_mode   = LDC_DST_ROT_90;

		uartlog("%s: 2nd src buf address Y(%#llx) C(%#llx)\n", __func__, cfg.src_y_base, cfg.src_c_base);
		uartlog("%s: 2nd dst buf address Y(%#llx) C(%#llx)\n", __func__, cfg.dst_y_base, cfg.dst_c_base);

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
			uartlog("2nd ldc end intr doesn't happens after 600 ms\n");
			ret = -1;
		} else {
			uartlog("2nd ldc done\n");
		}
#endif
	}

	return ret;
}

// testcase-13
// 64x64-nv12-flat
static int ldc_test13(int argc, char **argv)
{
	int ret = 0;
	int use_cmdq = 0;
	struct ldc_cfg cfg;
	// u32 tmp;
	u64 ldc_src_buf = src_addr_ldc[0][0];
	u64 ldc_dst_buf = dst_addr_ldc[0][0];
	u64 ldc_map_base = src_addr_ldc[1][0];

	cfg.pixel_fmt = 1;		// nv12
	cfg.map_bypass = 1;
	cfg.map_base = ldc_map_base;
	cfg.src_width = 64;
	cfg.src_height = 64;
	cfg.ras_width = cfg.src_width;
	cfg.ras_height = cfg.src_height;
	if (cfg.src_width >= 4096) {
		// ldc_src_buf = 0x105000000
		ldc_src_buf = ldc_src_buf - 0x2000000;		// 0x103000000
		ldc_dst_buf = ldc_src_buf + 0x2000000;		// 0x105000000
	}
	if (cfg.src_width & 0x3f) {
		uartlog("%s: width must be 64x\n", __func__);
		return -1;
	}
	cfg.src_xstart = 0;
	cfg.src_xend = cfg.src_width - 1;
	cfg.bgcolor = (128 << 8) | 128; // yuv black

	cfg.src_y_base = ldc_src_buf;
	cfg.src_c_base = cfg.src_y_base + cfg.src_width * cfg.src_height;
	cfg.dst_y_base = ldc_dst_buf;
	cfg.dst_c_base = cfg.dst_y_base + cfg.src_width * cfg.src_height;
	uartlog("%s: src buf address Y(%#llx) C(%#llx)\n", __func__, cfg.src_y_base, cfg.src_c_base);
	uartlog("%s: dst buf address Y(%#llx) C(%#llx)\n", __func__, cfg.dst_y_base, cfg.dst_c_base);

	cfg.dst_mode = 0;		// flat
	cfg.use_cmdq = 0;
	use_cmdq = cfg.use_cmdq;
	// cfg.extend_haddr = 0;
	cfg.extend_haddr = (((cfg.src_y_base >> 33) & 0x7)<<1 |((cfg.dst_y_base >> 33) & 0x7)<<13);

	timer_tick = timer_get_tick();
	if (!use_cmdq) {
		ldc_intr_ctrl(1);
		ldc_engine(&cfg);
		ldc_dump_register();
	} else {
		enable_cmdq = true;
		ldc_engine_cmdq(&cfg, cmdq_set);
	}

#ifndef ENV_EMU
	if (check_irq_handler(20) != 0) {
		uartlog("ldc end intr doesn't happens after 600 ms\n");
		ret = -1;
	} else {
		uartlog("ldc done\n");
	}
#endif
	return ret;
}

// testcase-14
// 4096x4096-nv12-flat
static int ldc_test14(int argc, char **argv)
{
	int ret = 0;
	int use_cmdq = 0;
	struct ldc_cfg cfg;
	// u32 tmp;
	u64 ldc_src_buf = src_addr_ldc[0][0];
	u64 ldc_dst_buf = dst_addr_ldc[0][0];
	u64 ldc_map_base = src_addr_ldc[1][0];

	cfg.pixel_fmt = 1;		// nv12
	cfg.map_bypass = 1;
	cfg.map_base = ldc_map_base;
	cfg.src_width = 4096;
	cfg.src_height = 4096;
	cfg.ras_width = cfg.src_width;
	cfg.ras_height = cfg.src_height;

	if (cfg.src_width >= 4096) {
		// ldc_src_buf = 0x105000000
		//ldc_src_buf = ldc_src_buf - 0x2000000;		// 0x103000000
		ldc_dst_buf = ldc_src_buf + 0x2000000;		// 0x105000000
	}
	if (cfg.src_width & 0x3f) {
		uartlog("%s: width must be 64x\n", __func__);
		return -1;
	}
	cfg.src_xstart = 0;
	cfg.src_xend = cfg.src_width - 1;
	cfg.bgcolor = (128 << 8) | 128; // yuv black

	cfg.src_y_base = ldc_src_buf;
	cfg.src_c_base = cfg.src_y_base + cfg.src_width * cfg.src_height;
	cfg.dst_y_base = ldc_dst_buf;
	cfg.dst_c_base = cfg.dst_y_base + cfg.src_width * cfg.src_height;
	uartlog("%s: src buf address Y(%#llx) C(%#llx)\n", __func__, cfg.src_y_base, cfg.src_c_base);
	uartlog("%s: dst buf address Y(%#llx) C(%#llx)\n", __func__, cfg.dst_y_base, cfg.dst_c_base);

	cfg.dst_mode = 0;		// flat
	cfg.use_cmdq = 0;
	use_cmdq = cfg.use_cmdq;
	// cfg.extend_haddr = 0;
	cfg.extend_haddr = (((cfg.src_y_base >> 33) & 0x7)<<1 |((cfg.dst_y_base >> 33) & 0x7)<<13);

	timer_tick = timer_get_tick();
	if (!use_cmdq) {
		ldc_intr_ctrl(1);
		ldc_engine(&cfg);
		ldc_dump_register();
	} else {
		enable_cmdq = true;
		ldc_engine_cmdq(&cfg, cmdq_set);
	}

#ifndef ENV_EMU
	if (check_irq_handler(20) != 0) {
		uartlog("ldc end intr doesn't happens after 600 ms\n");
		ret = -1;
	} else {
		uartlog("ldc done\n");
	}
#endif
	return ret;
}

static int close_vi_clk(int argc, char **argv){
	u64 clk_gen_top = 0x03002000;
	u64 reg_clk_en_hf0_addr = 0xf0;
	u64 vi_clk_ldc_mask = BIT(15);

	u64 vi_sys_addr = 0xA092000;
	u64 VI_CLK_LP0 = 0x14;
	u64 VI_CLK_LP2 = 0x1c;

	u64 apb_clk_en_ldc_mask = BIT(16);
	u64 apb_auto_gating_en_ldc = BIT(20);

	//disable clk en
	_reg_write_mask(clk_gen_top + reg_clk_en_hf0_addr
		, vi_clk_ldc_mask
		, 0);

	//(low power control) disable apb force clk, write1:force on, write0:depend on clk gating
	_reg_write_mask(vi_sys_addr + VI_CLK_LP0
		, apb_clk_en_ldc_mask
		, 0);

	//apb clock auto gating en
	_reg_write_mask(vi_sys_addr + VI_CLK_LP2
		, apb_auto_gating_en_ldc
		, apb_auto_gating_en_ldc);

	return 0;
}

static int open_vi_clk(int argc, char **argv){
	u64 clk_gen_top = 0x03002000;
	u64 reg_clk_en_hf0_addr = 0xf0;
	u64 vi_clk_ldc_mask = BIT(15);

	u64 vi_sys_addr = 0xA092000;
	u64 VI_CLK_LP0 = 0x14;
	u64 VI_CLK_LP2 = 0x1c;

	u64 apb_clk_en_ldc_mask = BIT(16);
	u64 apb_auto_gating_en_ldc = BIT(20);

	//enable clk en
	_reg_write_mask(clk_gen_top + reg_clk_en_hf0_addr
		, vi_clk_ldc_mask
		, vi_clk_ldc_mask);

	//(low power control) enable apb force clk, write1:force on, write0:depend on clk gating
	_reg_write_mask(vi_sys_addr + VI_CLK_LP0
		, apb_clk_en_ldc_mask
		, apb_clk_en_ldc_mask);

	//apb clock auto gating disable
	_reg_write_mask(vi_sys_addr + VI_CLK_LP2
		, apb_auto_gating_en_ldc
		, 0);

	return 0;
}


/* test case rst
** u64 vi_sys_addr = 0xA092000;
** VI_RESETS offset = 0x0;
** LDC BIT(20); bit set 1 is reset.

** VI_RESETS_APB offset = 0x4;
** LDC BIT(15); bit set 1 is reset.

** run do_job, set invalid param, irq will not respond.
** do RST,UNRST
** run do_job, work job is ok
*/

static void bist_enable(void)
{
	mmio_wr32(0x08010000, 0x000e000e);
	mmio_wr32(0x08010040, 0x401ffa00);
	mmio_wr32(0x08010044, 0x801ffa00);
	mmio_wr32(0x08010048, 0xc000ffff);
	mmio_wr32(0x0801004c, 0x0);
	mmio_wr32(0x08010050, 0x0);
	mmio_wr32(0x08010054, 0x0);

	mmio_wr32(0x08010010, 0x09f00000);
	mmio_wr32(0x08010014, 0x09ffffff);

	mmio_wr32(0x08010018, 0x4);
	mmio_wr32(0x08010000, 0x00010001);
}

// bist_grid_info_ldc
// LDC 2 phase
// 1920x1088, 2 pass ldc
static int bist_grid_info_ldc(int argc, char **argv)
{
	int ret = 0;
	int use_cmdq = 0;
	int i = 0;
	int times = 1;
	u32 tmp;
	struct ldc_cfg cfg;
	const u64 ldc_src_buf      = src_addr_ldc[0][0];	// 0x105000000
	const u64 ldc_map_base_1st = src_addr_ldc[1][0];	// 0x105600000
	u64 ldc_map_base_2nd = src_addr_ldc[2][0];	// 0x105900000
	const u64 ldc_dst_buf_1st  = dst_addr_ldc[0][0];	// 0x105C00000
	const u64 ldc_dst_buf_2nd  = dst_addr_ldc[1][0];	// 0x107000000

	ldc_map_base_2nd = ldc_map_base_1st + 0x3C19B0;
	bist_enable();

	// From 182x 20210930_ldc_output
	uartlog("(%#llx) 1920x1088_nv12, in\n", ldc_src_buf);
	uartlog("(%#llx) 1920x1088-srcx_1st_mesh.bin, 1st mesh, in\n", ldc_map_base_1st);
	uartlog("(%#llx) 1920x1088-srcx_2nd_mesh.bin, 2nd mesh, in\n", ldc_map_base_2nd);
	uartlog("(%#llx) 1920x1088 NV12 packed, 1st out, ref ldc_src_1920x1088.ppm_NV12\n", ldc_dst_buf_1st);
	uartlog("(%#llx) 1920x1088 NV12 packed, 2nd out, ref ldc_dst_1920x1088_2nd_pass.ppm_NV12\n", ldc_dst_buf_2nd);

	// 1st pass
	memset(&cfg, 0, sizeof(cfg));
	cfg.pixel_fmt  = 1;	// nv12
	cfg.map_bypass = 0;
	cfg.map_base   = ldc_map_base_1st;

	cfg.src_width  = 1920;	// 64X
	cfg.src_height = 1088;	// 64X
	cfg.ras_width  = cfg.src_width;
	cfg.ras_height = cfg.src_height;

	cfg.src_xstart = 0;
	cfg.src_xend   = cfg.src_width - 1;
	cfg.bgcolor    = (128 << 8) | 128; // yuv black
	cfg.extend_haddr = (((cfg.src_y_base >> 33) & 0x7)<<1 |((cfg.dst_y_base >> 33) & 0x7)<<13);

	cfg.src_y_base = ldc_src_buf;
	cfg.src_c_base = cfg.src_y_base + cfg.src_width * cfg.src_height;
	cfg.dst_y_base = ldc_dst_buf_1st;
	cfg.dst_c_base = cfg.dst_y_base + cfg.src_width * cfg.src_height;
	cfg.dst_mode   = LDC_DST_ROT_270;

	uartlog("%s: 1st src buf address Y(%#llx) C(%#llx)\n", __func__, cfg.src_y_base, cfg.src_c_base);
	uartlog("%s: 1st dst buf address Y(%#llx) C(%#llx)\n", __func__, cfg.dst_y_base, cfg.dst_c_base);
	bm_get_num("times: ", times);
	for (i = 0; i < times; i++) {
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
			uartlog("1st ldc end intr doesn't happens after 600 ms\n");
			ret = -1;
			return ret;
		} else {
			uartlog("1st ldc done\n");
		}
#endif

		// 2nd pass
		cfg.map_base = ldc_map_base_2nd;

		cfg.src_width  = 1088;	// 64X
		cfg.src_height = 1920;	// 64X
		cfg.ras_width  = cfg.src_width;
		cfg.ras_height = cfg.src_height;

		cfg.src_xstart = 0;
		cfg.src_xend   = cfg.src_width - 1;
		cfg.bgcolor    = (128 << 8) | 128; // yuv black

		cfg.src_y_base = ldc_dst_buf_1st;
		cfg.src_c_base = cfg.src_y_base + cfg.src_width * cfg.src_height;
		cfg.dst_y_base = ldc_dst_buf_2nd;
		cfg.dst_c_base = cfg.dst_y_base + cfg.src_width * cfg.src_height;
		cfg.dst_mode   = LDC_DST_ROT_90;

		uartlog("%s: 2nd src buf address Y(%#llx) C(%#llx)\n", __func__, cfg.src_y_base, cfg.src_c_base);
		uartlog("%s: 2nd dst buf address Y(%#llx) C(%#llx)\n", __func__, cfg.dst_y_base, cfg.dst_c_base);

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
			uartlog("2nd ldc end intr doesn't happens after 600 ms\n");
			ret = -1;
		} else {
			uartlog("2nd ldc done\n");
		}
#endif
	}

	return ret;
}

static struct cmd_entry test_cmd_ldc_list[] __attribute__ ((unused)) = {
	{"\n[---LDC------------------------------------------------]", do_job, 0, ""},
	{"ldc_job",         do_job,          0, "[Modify settings]"},
	{"ldc_dump",        do_dump,         0, "[Dump register]"},
	{"ldc_dump_ddr",    do_dump_ddr,     0, "[Dump ddr]"},
	{"ldc_test_1",      ldc_test1,       0, " [1920x1088-nv12, memcpy]"},
	{"ldc_test_2",      ldc_test2,       0, " [1920x1088-nv12, xyflip]"},
	{"ldc_test_3",      ldc_test3,       0, " [1920x1088-nv12, ROT90]"},
	{"ldc_test_4",      ldc_test4,       0, " [1920x1088-nv12, ROT270]"},
	{"ldc_test_5",      ldc_test5,       0, " [1920x1088-Y-only, memcpy]"},
	{"ldc_test_6",      ldc_test6,       0, " [1920x1088-Y-only, xyflip]"},
	{"ldc_test_7",      ldc_test7,       0, " [1920x1088-Y-only, ROT90]"},
	{"ldc_test_8",      ldc_test8,       0, " [1920x1088-Y-only, ROT270]"},
	{"ldc_test_9",      ldc_test9,       0, " [320x256, 2 pass ldc]"},
	{"ldc_test_10",     ldc_test10,      0, "[320x256, cmdq, bypass, flat]"},
	{"ldc_test_11",     ldc_test11,      0, "[2560x1472-nv12, memcpy]"},
	{"ldc_test_12",     ldc_test12,      0, "[2560x1472, 2 pass ldc/distortion coefficient min & max/bist]"},
	{"ldc_test_13",     ldc_test13,      0, "[64x64-nv12, memcpy]"},
	{"ldc_test_14",     ldc_test14,      0, "[4096x4096-nv12, memcpy]"},
	{"ldc_test_15",     close_vi_clk,    0, "close vi sys clk"},
	{"ldc_test_16",     open_vi_clk,     0, "open vi sys clk"},
	{"ldc_test_17",     bist_grid_info_ldc,     0, "bist + grid_info"},
	{NULL, NULL, 0, NULL}
};

int testcase_ldc(void)
{
	int ldc_top_id =0;
	uartlog("ldc init\n");
	uartlog("sel ldc top(0 or 1):");		// select第0/1个IP
	scanf("%d", &ldc_top_id);

	ldc_init();

	ldc_intr_ctrl(1);

#ifndef VIP_PERFORMANCE_TEST
	uartlog("%s. request_irq(%d)\n", __func__, LDC_INT);
	request_irq(LDC_INT, irq_handler_ldc, 0, "ldc int", NULL);

	// 2380 only have one LDC ip
	// if (ldc_top_id) {
	// 	ldc_set_base_addr((void*)0x2000);
	// 	request_irq(LDC_INT_2, irq_handler_ldc, 0, "ldc2 int", NULL);
	// }
#endif

	for(int i = 0;i < ARRAY_SIZE(test_cmd_ldc_list) - 1;i++) {
	command_add(&test_cmd_ldc_list[i]);
	}
	uartlog("%s. add ldc cmd\n", __func__);
	return 0;
}

