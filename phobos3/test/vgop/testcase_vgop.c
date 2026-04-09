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
#include "scaler_reg.h"
#include "reg_sc_odma.h"
#include "reg_sc_top.h"
#include "reg.h"
#include "scaler.h"
#include "cmdq.h"
#include "oenc.h"
#include <cache.h>

#define EPSILON_SCALE	 1000000 // 1/0.000001
#define SINGLE_PACKED
#define OENC0_INTR 17
#define OENC1_INTR 18
#define IRQ_HANDLER 0

#define SC_INTR_1 21 //temp vpss 1 intr num

// static int count_intr_fifofull = 0;
// static int count_intr_frame_start = 0;
// static int count_intr_frame_done = 0;
// static u8 oenc_disp = 0;

struct cvi_img_cfg {
	enum sclr_input in;   // 0(ISP), 1(dwa), 2(share), 3(DRAM)
	enum sclr_format fmt_in;
	enum sclr_csc csc;
	struct sclr_mem mem;
};

struct cvi_sc_cfg {
	struct sclr_rect crop;
	struct sclr_size dst;
	enum sclr_format fmt_out;
	enum sclr_out_mode out_mode;
	enum sclr_odma_datatype datatype;
	enum sclr_csc csc;
	enum sclr_flip_mode flip;
	struct sclr_mem mem;
	enum sclr_algorithm algorithm;
	struct sclr_convertto_formula convert_to_cfg;
	struct sclr_quant_formula quant_form;
	union {
		enum sclr_hsv_rounding hsv_round;
		enum sclr_quant_rounding quant_round;
	};
};

struct cvi_vpss_cfg {
	u8 inst;
	bool tile_enable;
	bool v_tile_enable;
	struct cvi_img_cfg img_cfg;
	struct cvi_sc_cfg sc_cfg;
};

enum CFG_OP {
	CFG_OP_VPSS_SC_SET_COMPR= 0,
	CFG_OP_VPSS_GOP_LUT_CFG,
	CFG_OP_VPSS_GOP_OW_CFG,
	CFG_OP_VPSS_GOP_SCL,
	CFG_OP_VPSS_GOP_CFG,
	CFG_OP_VPSS_SC_SET_COVER,
	CFG_OP_VPSS_GOP_8_DEC_CFG,
	CFG_OP_VPSS_GOP_DECOMPR,
	CFG_OP_VPSS_STATUS_GOP,
	CFG_OP_VPSS_STATUS_ALL = 60,

};

/* struct cvi_vip_fmt
 * @fourcc: v4l2 fourcc code.
 * @fmt: sclr driver's relative format.
 * @buffers: number of buffers.
 * @bit_depth: number of bits per pixel per plane.
 * @plane_sub_h: plane horizontal subsample.
 * @plane_sub_v: plane vertical subsample.
 */
struct cvi_vip_fmt {
	u32 fourcc; 		 /* v4l2 format id */
	enum sclr_format fmt;
	u8	buffers;
	u32 bit_depth[3];
	u8 plane_sub_h;
	u8 plane_sub_v;
};

const struct cvi_vip_fmt cvi_vip_formats[] = {
	{
	.fmt		 = SCL_FMT_YUV420,
	.bit_depth	 = { 8, 2, 2 },
	.buffers	 = 3,
	.plane_sub_h = 2,
	.plane_sub_v = 2,
	},
	{
	.fmt		 = SCL_FMT_YUV422,
	.bit_depth	 = { 8, 4, 4 },
	.buffers	 = 3,
	.plane_sub_h = 2,
	.plane_sub_v = 1,
	},
	{
	.fmt		 = SCL_FMT_YUV444,
	.bit_depth	 = { 8, 8, 8 },
	.buffers	 = 3,
	.plane_sub_h = 1,
	.plane_sub_v = 1,
	},
	{
	.fmt		 = SCL_FMT_NV12,
	.bit_depth	 = { 8, 4, 0 },
	.buffers	 = 2,
	.plane_sub_h = 1,
	.plane_sub_v = 2,
	},
	{
	.fmt		 = SCL_FMT_NV21,
	.bit_depth	 = { 8, 4, 0 },
	.buffers	 = 2,
	.plane_sub_h = 1,
	.plane_sub_v = 2,
	},
	{
	.fmt		 = SCL_FMT_YUV422SP2,
	.bit_depth	 = { 8, 8, 0 },
	.buffers	 = 2,
	.plane_sub_h = 1,
	.plane_sub_v = 1,
	},
	{
	.fmt		 = SCL_FMT_YUV422SP1,
	.bit_depth	 = { 8, 8, 0 },
	.buffers	 = 2,
	.plane_sub_h = 1,
	.plane_sub_v = 1,
	},
	{
	.fmt		 = SCL_FMT_YUYV,
	.bit_depth	 = { 16 },
	.buffers	 = 1,
	.plane_sub_h = 1,
	.plane_sub_v = 1,
	},
	{
	.fmt		 = SCL_FMT_YVYU,
	.bit_depth	 = { 16 },
	.buffers	 = 1,
	.plane_sub_h = 1,
	.plane_sub_v = 1,
	},
	{
	.fmt		 = SCL_FMT_UYVY,
	.bit_depth	 = { 16 },
	.buffers	 = 1,
	.plane_sub_h = 1,
	.plane_sub_v = 1,
	},
	{
	.fmt		 = SCL_FMT_VYUY,
	.bit_depth	 = { 16 },
	.buffers	 = 1,
	.plane_sub_h = 1,
	.plane_sub_v = 1,
	},
	{
	.fmt		 = SCL_FMT_RGB_PLANAR,
	.bit_depth	 = { 8, 8, 8 },
	.buffers	 = 3,
	.plane_sub_h = 1,
	.plane_sub_v = 1,
	},
	{
	.fmt		 = SCL_FMT_BF16,
	.bit_depth	 = { 8, 8, 8 },
	.buffers	 = 3,
	.plane_sub_h = 1,
	.plane_sub_v = 1,
	},
	{
	.fmt		 = SCL_FMT_RGB_PACKED,
	.bit_depth	 = { 24 },
	.buffers	 = 1,
	.plane_sub_h = 1,
	.plane_sub_v = 1,
	},
	{
	.fmt		 = SCL_FMT_BGR_PACKED,
	.bit_depth	 = { 24 },
	.buffers	 = 1,
	.plane_sub_h = 1,
	.plane_sub_v = 1,
	},
	{
	.fmt		 = SCL_FMT_Y_ONLY,
	.bit_depth	 = { 8 },
	.buffers	 = 1,
	.plane_sub_h = 1,
	.plane_sub_v = 1,
	},
	{
	.fmt		 = SCL_FMT_BGR_PACKED,
	.bit_depth	 = { 24 },
	.buffers	 = 1,
	.plane_sub_h = 1,
	.plane_sub_v = 1,
	},
	{
	.fmt		 = SCL_FMT_RGB_PLANAR,
	.bit_depth	 = { 8, 8, 8 },
	.buffers	 = 3,
	.plane_sub_h = 1,
	.plane_sub_v = 1,
	},
	{
	.fmt		 = SCL_FMT_YUV444_PACKED,
	.bit_depth	 = { 24 },
	.buffers	 = 1,
	.plane_sub_h = 1,
	.plane_sub_v = 1,
	},
};

static char* str_sclr_gop_fmt[] = {"ARGB8888", "ARGB4444", "ARGB1555", "256LUT", "16LUT", "FONT"};
static bool is_init = false;

#ifdef BOARD_PALLADIUM
#define DEFAULT_W 800  // 800
#define DEFAULT_H 600  // 600
// #define WAIT_TIME 5
#else
#define DEFAULT_W 800   // 800
#define DEFAULT_H 600   // 600
// #define WAIT_TIME 20000
#endif

static int WAIT_TIME = 2000;
// #define WAIT_TIME 2000   // for 8192x8192

u64 src_addr_scl[4][3] = {
	{BASE_OFFSET + 0x81000000, BASE_OFFSET + 0x00c00000, BASE_OFFSET + 0x01000000},
	{BASE_OFFSET + 0x81000000, BASE_OFFSET + 0x01800000, BASE_OFFSET + 0x01c00000},
	{BASE_OFFSET + 0x81000000, BASE_OFFSET + 0x02400000, BASE_OFFSET + 0x02800000},  //gop(max5338x1080)
	{BASE_OFFSET + 0x81000000, BASE_OFFSET + 0x03000000, BASE_OFFSET + 0x03400000},
};

u64 dst_addr_scl[4][3] = {
	{BASE_OFFSET + 0x88000000, BASE_OFFSET + 0x03c00000, BASE_OFFSET + 0x04000000},
	{BASE_OFFSET + 0x85000000, BASE_OFFSET + 0x04800000, BASE_OFFSET + 0x04c00000},
	{BASE_OFFSET + 0x86000000, BASE_OFFSET + 0x05400000, BASE_OFFSET + 0x05800000}, //primask
	{BASE_OFFSET + 0x87000000, BASE_OFFSET + 0x06000000, BASE_OFFSET + 0x06400000},
};

//gop_addr 0x90000000
static u64 gop_addr = BASE_OFFSET + 0x90000000;
static u64 gop_LUT_addr = BASE_OFFSET + 0x84000000;
// static u32 gop_space[SCL_MAX_INST + 1] = {GOP_FRAME_OFFSET, GOP_FRAME_OFFSET, GOP_FRAME_OFFSET, GOP_FRAME_OFFSET}; // the remaining dram for gop, count down like stack
static u32 gop_space[2] = {
	0, GOP_FRAME_OFFSET * 8};

static bool enable_cmdq = false;
u64 sclr_timer_tick = 0;
u64 oenc_timer_tick = 0;
// u64 vpss_vgop_timer_tick[VPSS_MAX + 1] = {0};
u64 irq_arrival_time[4] = {0,0,0,0};
volatile int sclr_mark;
static u32 enable_intr_msg = 1;
static u32 vpss_enable_mask;
static bool vpss_status[SCL_MAX_INST] = {true, true};
// static bool vpss_status[SCL_MAX_INST] = {false, false, false, false, true};

void sclr_trig_start_ext(int inst_src, int inst_dst)
{
	inst_src = inst_dst;

	//TODO:

}

void sclr_enable_intr_log(u32 enable)
{
	enable_intr_msg = enable;
}

void frame_done_trigger(void)
{
	// _reg_write(0x28100080, 0x2);
}

void fail_trigger(void)
{
	// _reg_write(0x28100080, 0x3);
}

void _get_bytesperline(enum sclr_format sc_fmt, u16 width, u16 *bytesperline)
{
	const struct cvi_vip_fmt *fmt;

	memset(bytesperline, 0, sizeof(bytesperline[0] * 3));
	for (int k = 0; k < ARRAY_SIZE(cvi_vip_formats); k++) {
		fmt = &cvi_vip_formats[k];
		if (fmt->fmt == sc_fmt)
			break;
	}

	for (int p = 0; p < fmt->buffers; p++) {
		/* Calculate the minimum supported bytesperline value */
		if(p == 0)
			bytesperline[p] = (width * fmt->bit_depth[p]) >> 3;
		else {
			bytesperline[p] = bytesperline[0] / fmt->plane_sub_h;
			if(bytesperline[p] * fmt->plane_sub_h < bytesperline[0])
				bytesperline[p] += 1;
		}
		if(bytesperline[p] < 16){
			bytesperline[p] = 16;
			if(p != 0)
				bytesperline[0] = bytesperline[p] * fmt->plane_sub_h;
		}
	}
	// uartlog("fmt(%d) num_planes(%d) pitch(%d %d)\n", sc_fmt, fmt->buffers, bytesperline[0], bytesperline[1]);
}

void sclr_trig_start(u8 inst)
{
	sclr_mark = 0;
	sclr_timer_tick = timer_get_tick();
	// timer_meter_start();
	sclr_img_start(inst);
}

int irq_handler_scl(int irqn, void *priv)
{
	u8 i;
	int inst = -1;
	irq_arrival_time[irqn-9] = timer_get_tick();
	u32 irq_num[SCL_MAX_INST] = {SC_INTR_0, SC_INTR_1};

	for (i = 0; i < SCL_MAX_INST; i++)
		if (irq_num[i] == irqn)
			inst = i;
	if (inst < 0) {
		uartlog("vpss irqn(%d) err.\n", irqn);
		return -1;
	}

	if (enable_intr_msg) {
		// double frame_time_ms = (double)(time - sclr_timer_tick) / TIMER_CNT_MS;
		// double fps = 1000.0 / frame_time_ms;
		// uartlog(" ****** vpss(%d) irq, time-consumption: tick(%d) time(%lf) ms, Frame rate: %lf FPS ****** \n",
		// 		inst, (time - sclr_timer_tick), frame_time_ms, fps);
	}

	if (enable_cmdq) {
		u8 status = cmdQ_intr_status(REG_SCL_CMDQ_BASE(inst));
		if (status) {
			cmdQ_intr_clr(REG_SCL_CMDQ_BASE(inst), status);
			uartlog("vpss(%d) cmdq-status(0x%x)\n", inst, status);

			if ((status & 0x04)&& cmdQ_is_sw_restart(REG_SCL_CMDQ_BASE(inst))) {
				uartlog("vpss cmdq-sw restart\n");
				cmdQ_sw_restart(REG_SCL_CMDQ_BASE(inst));
			}

			if (status & 0x02) {
				sclr_mark++;  // cmdq end
				enable_cmdq = false;
			}
		} else {
			uartlog("vpss(%d) cmdq-status not found\n", inst);
		}
	} else {
		union sclr_intr intr_status = sclr_intr_status(inst);
		sclr_intr_clr(inst, intr_status);
		if (enable_intr_msg)
			uartlog("vpss(%d) status(%#x)\n", inst, intr_status.raw);

		if (!intr_status.b.scl_frame_end)
			return 0;
		// uartlog("vpss(%d) frame done.\n", inst);
		if (!(vpss_enable_mask & BIT(inst))) {
			uartlog("vpss%d is not enable.\n", inst);
			//return -1;
		}
		vpss_enable_mask &= ~BIT(inst);

		if (!vpss_enable_mask) {
			sclr_mark++;
		}
		 //uartlog("sclr_mark%d vpss_enable_mask%d.\n", sclr_mark, vpss_enable_mask);

	}

	uartlog("vpss(%d) done!\n", inst);
	return 0;
}

int sclr_check_irq_handler(int timeout)
{
	int i = 0;

	for (i = 0; i < timeout; ++i) {
		if (sclr_mark > 0) {
			sclr_mark = 0;
			// uartlog("%s: check tick:%d\n", __func__, i);
			return 0;
		}
		mdelay(1);
	}

	return -1;
}

void _show_sclr_mem(struct sclr_mem *mem)
{
	uartlog("start_x(%3d)\t\tstart_y(%3d)\t\twidth(%4d)\t\theight(%4d)\n",
		mem->start_x, mem->start_y, mem->width, mem->height);
	uartlog("pitch_y(%3d)\t\tpitch_c(%3d)\n", mem->pitch_y, mem->pitch_c);
	uartlog("addr0(0x%09lx)\taddr1(0x%09lx)\taddr2(0x%09lx)\n", mem->addr0, mem->addr1, mem->addr2);
}

void _intr_setup(u8 inst)
{
	union sclr_intr mask;

	mask.raw = 0;
	//mask.b.img_in_frame_end = true;
	mask.b.scl_frame_end = true;
	mask.b.cmdq = true;
	//mask.b.scl_line_target_hit = true;
	sclr_set_intr_mask(inst, mask);
}

int _handle_cfg_op(u8 op, ...)
{
	int ret = 0;
	u32 tmp = 0;
	va_list ap;
	va_start(ap, op);
	u8 inst = va_arg(ap, int);

	switch(op) {

	case CFG_OP_VPSS_SC_SET_COMPR:
	{
		struct oenc_cfg oenc_cfg[MAX_OSD_ENC_INST] = {0};
		struct oenc_size img_size;
		u16 mem_width;
		u8 oenc_inst = 0;
		// _reg_write(0x67000028, 0x10100F0F);
		uartlog("Please make sure compress data has been loaded into following address:0x%lx\n", gop_addr);

		oenc_cfg[oenc_inst].cfg.raw = 0x80000000;
		bm_get_num("format(0:ARGB8888, 1:ARGB4444, 2:ARGB1555, 3:256LUT, 4:16LUT): ", oenc_cfg[oenc_inst].fmt);
		bm_get_num("img width: ", img_size.w);
		bm_get_num("img height: ", img_size.h);

		oenc_cfg[oenc_inst].src_picture_size = img_size;
		oenc_cfg[oenc_inst].src_mem_size.h = img_size.h;

		switch (oenc_cfg[oenc_inst].fmt) {
		case OENC_GOP_FMT_ARGB8888:
			mem_width = img_size.w * 4;
			break;
		case OENC_GOP_FMT_ARGB4444:
		case OENC_GOP_FMT_ARGB1555:
			mem_width = img_size.w * 2;
			break;
		case OENC_GOP_FMT_256LUT:
			mem_width = img_size.w;
			break;
		case OENC_GOP_FMT_16LUT:
			mem_width = img_size.w >> 1;
			break;
		default:
			uartlog("invalid fmt(%d)\n", oenc_cfg[oenc_inst].fmt);
			return -1;
		}
		oenc_cfg[oenc_inst].src_pitch = ALIGN(mem_width, 16);
		oenc_cfg[oenc_inst].src_mem_size.w = ALIGN(mem_width, 16);
		uartlog("mem size: (%d*%d), pitch(%d)\n",
				oenc_cfg[oenc_inst].src_mem_size.w, oenc_cfg[oenc_inst].src_mem_size.h, oenc_cfg[oenc_inst].src_pitch);

		// count down from space's end
		oenc_cfg[oenc_inst].src_adr = gop_addr;
		oenc_cfg[oenc_inst].bso_adr = gop_addr + (oenc_cfg[oenc_inst].src_pitch * oenc_cfg[oenc_inst].src_mem_size.h);
		uartlog("osd src mem addr: (0x%09lx)\n", oenc_cfg[oenc_inst].src_adr);
		uartlog("OSD Compression Bit Stream Output addr: (0x%09lx)\n", oenc_cfg[oenc_inst].bso_adr);

		bm_get_num("RGB truncate mode enable, y(1~3)/n(0): ", oenc_cfg[oenc_inst].cfg.b.rgb_trunc);
		bm_get_num("Alpha truncate mode enable, y(1~3)/n(0): ", oenc_cfg[oenc_inst].cfg.b.alpha_trunc);
		bm_get_num("Alpha_zero mode enable (hw init:y), y(1)/n(0): ", oenc_cfg[oenc_inst].cfg.b.alpha_zero);
		bm_get_num("Osd ENC bitstream size limit bypass enable, y(1)/n(0): ", oenc_cfg[oenc_inst].cfg.b.limit_bsz_bypass);
		tmp=0;
		bm_get_num("Osd ENC bitstream size limit enable, y(1)/n(0): ", tmp);
		if (tmp) {
			oenc_cfg[oenc_inst].cfg.b.limit_bsz_en = tmp;
			bm_get_num("Osd ENC Bitstream Output size in byte, max 32MB: ", oenc_cfg[oenc_inst].limit_bsz);
		}
		tmp = 0;
		bm_get_num("Osd ENC AXI write protect enable, y(1)/n(0): ", tmp);
		if (tmp) {
			oenc_cfg[oenc_inst].cfg.b.wprot_en = tmp;
			bm_get_num("Osd ENC AXI write protect low address: ", oenc_cfg[oenc_inst].wprot_laddr);
			bm_get_num("Osd ENC AXI write protect upper address: ", oenc_cfg[oenc_inst].wprot_uaddr);
		}

		bm_get_num("Osd ENC watchdog enable (hw init:y), y(1)/n(0): ", oenc_cfg[oenc_inst].cfg.b.wdog_en);
		uartlog("oenc_inst is %d!!!!!\n", oenc_inst);
		flush_dcache_range(0x90000000, 17280);
		oenc_set_cfg(oenc_cfg, oenc_inst);
		oenc_timer_tick = timer_get_tick();
		break;
	}

	case CFG_OP_VPSS_SC_SET_COVER:
	{
		// u8 inst = va_arg(ap, int);
		struct sclr_cover_cfg cover_cfg;
		u8 cover_w_inst;

		bm_get_num("OSD Cover window inst (0~3): ", tmp);
		if (tmp < SCL_MAX_COVER_INST)
			cover_w_inst = tmp;
		else {
			uartlog("No such OSD cover window(%d): ", tmp);
			return -1;
		}
		bm_get_num("OSD Cover enable, y(1)/n(0): ", tmp);
		cover_cfg.start.b.enable = tmp;
		if (tmp) {
			bm_get_num("Cover frame start pos x: ", cover_cfg.start.b.x);
			bm_get_num("Cover frame start pos y: ", cover_cfg.start.b.y);
			bm_get_num("Cover frame width: ", cover_cfg.img_size.w);
			bm_get_num("Cover frame height: ", cover_cfg.img_size.h);
			bm_get_num("R(0~255): ", cover_cfg.color.b.cover_color_r);
			bm_get_num("G(0~255): ", cover_cfg.color.b.cover_color_g);
			bm_get_num("B(0~255): ", cover_cfg.color.b.cover_color_b);
		}

		sclr_cover_set_cfg(inst, cover_w_inst, &cover_cfg);
		break;
	}

	case CFG_OP_VPSS_GOP_CFG:
	{
		// u8 inst = va_arg(ap, int);
		u8 layer;
		if (inst < SCL_MAX_INST) {
			bm_get_num("Control which gop, gop0(0)/gop1(1): ", layer);
			if(layer >= SCL_MAX_GOP_INST) {
				uartlog("Do not have such layer%d", layer);
				return -1;
			}
		} else {
			uartlog("Do not have such GOP%d!!!\n", inst);
			return -1;
		}

		// Get global g_gop_cfg[inst][layer] or  &g_disp_cfg.gop_cfg
		struct sclr_gop_cfg *gop_cfg = sclr_gop_get_cfg(inst, layer);

		if (inst < SCL_MAX_INST){
			bm_get_num("Control gop on/off?, y(1)/n(0): ", tmp);
			if (tmp) {
				bm_get_num("Control gop ow on/off?, y(1)/n(0): ", tmp);
				if (tmp) {
					bm_get_num("gop ow0 enable, y(1)/n(0): ", gop_cfg->gop_ctrl.b.ow0_en);
					bm_get_num("gop ow1 enable, y(1)/n(0): ", gop_cfg->gop_ctrl.b.ow1_en);
					bm_get_num("gop ow2 enable, y(1)/n(0): ", gop_cfg->gop_ctrl.b.ow2_en);
					bm_get_num("gop ow3 enable, y(1)/n(0): ", gop_cfg->gop_ctrl.b.ow3_en);
					bm_get_num("gop ow4 enable, y(1)/n(0): ", gop_cfg->gop_ctrl.b.ow4_en);
					bm_get_num("gop ow5 enable, y(1)/n(0): ", gop_cfg->gop_ctrl.b.ow5_en);
					bm_get_num("gop ow6 enable, y(1)/n(0): ", gop_cfg->gop_ctrl.b.ow6_en);
					bm_get_num("gop ow7 enable, y(1)/n(0): ", gop_cfg->gop_ctrl.b.ow7_en);
				}

				bm_get_num("gop h-scale(x2) enable, y(1)/n(0): ", gop_cfg->gop_ctrl.b.hscl_en);
				bm_get_num("gop v-scale(x2) enable, y(1)/n(0): ", gop_cfg->gop_ctrl.b.vscl_en);
				bm_get_num("gop color-key enable, y(1)/n(0): ",   gop_cfg->gop_ctrl.b.colorkey_en);
				// bm_get_num("gop burst len(0~15): ", gop_cfg->gop_ctrl.b.burst);
				gop_cfg->gop_ctrl.b.burst = 15;

				bm_get_num("Control gop0 font color?, y(1)/n(0): ", tmp);
				if (tmp) {
					u8 color;
					gop_cfg->font_fg_color = 0;
					gop_cfg->font_bg_color = 0;
					uartlog("gop font's fg color\n");
					bm_get_num("A(0~15): ", color);
					gop_cfg->font_fg_color |= (color << 12);
					bm_get_num("R(0~15): ", color);
					gop_cfg->font_fg_color |= (color << 8);
					bm_get_num("G(0~15): ", color);
					gop_cfg->font_fg_color |= (color << 4);
					bm_get_num("B(0~15): ", color);
					gop_cfg->font_fg_color |= color;
					uartlog("font's bg color\n");
					bm_get_num("A(0~15): ", color);
					gop_cfg->font_bg_color |= (color << 12);
					bm_get_num("R(0~15): ", color);
					gop_cfg->font_bg_color |= (color << 8);
					bm_get_num("G(0~15): ", color);
					gop_cfg->font_bg_color |= (color << 4);
					bm_get_num("B(0~15): ", color);
					gop_cfg->font_bg_color |= color;

					gop_cfg->font_fg_inv_color = 0;
					gop_cfg->font_bg_inv_color = 0;
					uartlog("gop font's fg inv color\n");
					bm_get_num("A(0~15): ", color);
					gop_cfg->font_fg_inv_color |= (color << 12);
					bm_get_num("R(0~15): ", color);
					gop_cfg->font_fg_inv_color |= (color << 8);
					bm_get_num("G(0~15): ", color);
					gop_cfg->font_fg_inv_color |= (color << 4);
					bm_get_num("B(0~15): ", color);
					gop_cfg->font_fg_inv_color |= color;
					uartlog("font's bg inv color\n");
					bm_get_num("A(0~15): ", color);
					gop_cfg->font_bg_inv_color |= (color << 12);
					bm_get_num("R(0~15): ", color);
					gop_cfg->font_bg_inv_color |= (color << 8);
					bm_get_num("G(0~15): ", color);
					gop_cfg->font_bg_inv_color |= (color << 4);
					bm_get_num("B(0~15): ", color);
					gop_cfg->font_bg_inv_color |= color;
				}

				if (gop_cfg->gop_ctrl.b.colorkey_en) {
					u8 color;
					uartlog("Please give gop color-key value\n");
					gop_cfg->colorkey = 0;
					bm_get_num("R(0~255): ", color);
					gop_cfg->colorkey |= (color << 16);
					bm_get_num("G(0~255): ", color);
					gop_cfg->colorkey |= (color << 8);
					bm_get_num("B(0~255): ", color);
					gop_cfg->colorkey |= color;
				}
			}

			sclr_gop_set_cfg(inst, layer, gop_cfg);
		} else {
			uartlog("Do not have such GOP%d!!!\n", inst);
		}
		break;
	}

	case CFG_OP_VPSS_GOP_OW_CFG:
	{
		// u8 inst = va_arg(ap, int);
		u8 layer;
		u8 ow_inst;
		u16 mem_width;
		struct sclr_gop_ow_cfg ow_cfg;
		struct sclr_size osd_size;

		if (inst < SCL_MAX_INST) {
			bm_get_num("Control which gop, gop0(0)/gop1(1): ", layer);
			if(layer >= SCL_MAX_GOP_INST || (inst > 1 && layer > 0)) {
				uartlog("Do not have such layer%d", layer);
				return -1;
			}
		} else {
			uartlog("Do not have such GOP%d!!!\n", inst);
			return -1;
		}

		// Get global g_gop_cfg[inst][layer] or  &g_disp_cfg.gop_cfg
		struct sclr_gop_cfg *gop_cfg = sclr_gop_get_cfg(inst, layer);

		bm_get_num("ow inst (0~7): ", ow_inst);
		bm_get_num("format(0:ARGB8888, 1:ARGB4444, 2:ARGB1555, 3:256LUT, 4:16LUT, 5:Font): ", ow_cfg.fmt);
		bm_get_num("osd width: ", osd_size.w);
		bm_get_num("osd height: ", osd_size.h);

		ow_cfg.img_size = osd_size;
		ow_cfg.mem_size.h = osd_size.h;

		//OW crop pixels (per line)
		//Format ARGB8888,			crop pixels valid value: 0~3
		//Format ARGB4444/ARGB1555, crop pixels valid value: 0~7
		//Format 256LUT,			crop pixels valid value: 0~15
		//Format 16-LUT,			crop pixels valid value: 0~31
		//Format Font-base, 		crop pixels valid value: 0~127"
		bm_get_num("crop width: ", ow_cfg.crop_pixels);

		switch (ow_cfg.fmt) {
			case SCL_GOP_FMT_ARGB8888:
				if (ow_cfg.crop_pixels > 3) {
					uartlog("fmt(ARGB8888) crop_pixels need to be smaller than 3.\n");
					uartlog("invalid crop_pixels(%d)\n", ow_cfg.crop_pixels);
					return -1;
				}
				osd_size.w -= ow_cfg.crop_pixels;
				mem_width = osd_size.w * 4;
				break;
			case SCL_GOP_FMT_ARGB4444:
			case SCL_GOP_FMT_ARGB1555:
				if (ow_cfg.crop_pixels > 7) {
					uartlog("fmt(ARGB4444/1555) crop_pixels need to be smaller than 7.\n");
					uartlog("invalid crop_pixels(%d)\n", ow_cfg.crop_pixels);
					return -1;
				}
				osd_size.w -= ow_cfg.crop_pixels;
				mem_width = osd_size.w * 2;
				break;
			case SCL_GOP_FMT_256LUT:
				if (ow_cfg.crop_pixels > 15) {
					uartlog("fmt(256LUT) crop_pixels need to be smaller than 15.\n");
					uartlog("invalid crop_pixels(%d)\n", ow_cfg.crop_pixels);
					return -1;
				}
				osd_size.w -= ow_cfg.crop_pixels;
				mem_width = osd_size.w;
				break;
			case SCL_GOP_FMT_16LUT:
				if (ow_cfg.crop_pixels > 31) {
					uartlog("fmt(16LUT) crop_pixels need to be smaller than 31.\n");
					uartlog("invalid crop_pixels(%d)\n", ow_cfg.crop_pixels);
					return -1;
				}
				osd_size.w -= ow_cfg.crop_pixels;
				mem_width = (osd_size.w + 1) >> 1;
				break;
			case SCL_GOP_FMT_FONT:
				if (ow_cfg.crop_pixels > 127) {
					uartlog("fmt(font) crop_pixels need to be smaller than 127.\n");
					uartlog("invalid crop_pixels(%d)\n", ow_cfg.crop_pixels);
					return -1;
				}
				osd_size.w -= ow_cfg.crop_pixels;
				mem_width = (osd_size.w + 7) >> 3;
				break;
			default:
				uartlog("invalid fmt(%d)\n", ow_cfg.fmt);
				return -1;
		}
		ow_cfg.pitch = ALIGN(mem_width, 16); //32还是16对齐
		ow_cfg.mem_size.w = ALIGN(mem_width, 16); //32还是16对齐
		uartlog("osd_size: (%d*%d), mem size: (%d*%d), pitch(%d)\n", osd_size.w, osd_size.h, ow_cfg.mem_size.w, ow_cfg.mem_size.h, ow_cfg.pitch);

		bm_get_num("OSD frame start pos x: ", ow_cfg.start.x);
		bm_get_num("OSD frame start pos y: ", ow_cfg.start.y);

		if (layer == 0 || layer == 1) {
			ow_cfg.end.x = ow_cfg.start.x +
					(osd_size.w << gop_cfg->gop_ctrl.b.hscl_en) - gop_cfg->gop_ctrl.b.hscl_en;
			ow_cfg.end.y = ow_cfg.start.y +
					(osd_size.h << gop_cfg->gop_ctrl.b.vscl_en) - gop_cfg->gop_ctrl.b.vscl_en;
		} else {
			uartlog("Do not have such layer%d", layer);
			return -1;
		}

		uartlog("OSD frame start position: (%d*%d)\n", ow_cfg.start.x, ow_cfg.start.y);
		uartlog("OSD frame end position: (%d*%d)\n", ow_cfg.end.x, ow_cfg.end.y);

		// count down from space's end
		// gop_space[inst] -= (ow_cfg.pitch * ow_cfg.mem_size.h);

		// inst and ow_inst begin from 0
		// ow_cfg.addr = gop_addr + GOP_FRAME_OFFSET * (inst + 1) * (ow_inst + 1) + gop_space[inst];
		ow_cfg.addr = gop_addr + GOP_FRAME_OFFSET * (0) * (0) + gop_space[0];
		uartlog("vpss inst(%d), layer (%d), ow_inst(%d) osd mem addr: (0x%09lx)\n", inst, layer, ow_inst, ow_cfg.addr);

		sclr_gop_ow_set_cfg(inst, layer, ow_inst, &ow_cfg);

		if (ow_cfg.fmt == SCL_GOP_FMT_FONT) {
			struct sclr_gop_fb_cfg fb_cfg;
			u16 pix_num;
			int fb_inst;
			fb_cfg.fb_ctrl.b.enable = 1;
			// bm_get_num("enable fontbox?, y(1)/n(0): ", fb_cfg.fb_ctrl.b.enable);
			if (fb_cfg.fb_ctrl.b.enable) {
				bm_get_num("which fontbox?, (0 ~ 1): ", fb_inst);
				fb_cfg.fb_ctrl.b.attach_ow = ow_inst;
				bm_get_num("hi threshold, (0 ~ 255): ", tmp);
				if (layer == 0 || layer == 1) {
					gop_cfg->fb_ctrl.b.hi_thr = tmp >> 2;
					gop_cfg->fb_ctrl.b.lo_thr = gop_cfg->fb_ctrl.b.hi_thr * 0.8;
					bm_get_num("invert condition?, low(1)/high(0): ", tmp);
					gop_cfg->fb_ctrl.b.lo_thr_inv = tmp;
				}
				sclr_gop_set_cfg(inst, layer, gop_cfg);

				bm_get_num("fontbox num?, (1 ~ 32): ", tmp);
				fb_cfg.fb_ctrl.b.fb_num = tmp - 1;
				if (osd_size.w % tmp) {
					uartlog("fontbox-num must divide the ow_width(%d)\n", osd_size.w);
					break;
				}
				fb_cfg.fb_ctrl.b.width = osd_size.w / tmp - 1;
				// pix_thr max(32), unit(8 pixel).
				// reduce sample_rate if too many pixels in fb
				pix_num = (fb_cfg.fb_ctrl.b.width + 1) * osd_size.h;
				fb_cfg.fb_ctrl.b.sample_rate = (pix_num >> 3) >> 5;
				fb_cfg.fb_ctrl.b.pix_thr = ((pix_num >> 3) >> fb_cfg.fb_ctrl.b.sample_rate) * 0.65;
				uartlog("fb_cfg.fb_ctrl.b.pix_thr(%d)\n", fb_cfg.fb_ctrl.b.pix_thr);
			}
			sclr_gop_fb_set_cfg(inst, layer, fb_inst, &fb_cfg);
		}
		break;
	}

	case CFG_OP_VPSS_GOP_LUT_CFG:
	{
		// u8 inst = va_arg(ap, int);
		struct sclr_gop_ow_cfg ow_cfg;
		u8 layer;
		if (inst < SCL_MAX_INST) {
			bm_get_num("Control which gop, gop0(0)/gop1(1): ", layer);
			if(layer >= SCL_MAX_GOP_INST) {
				uartlog("Do not have such layer%d", layer);
				return -1;
			}
		} else {
			uartlog("Do not have such GOP%d!!!\n", inst);
			return -1;
		}

		bm_get_num("format(3:256LUT, 4:16LUT): ", ow_cfg.fmt);
		uartlog("Please make sure LUT256 has been loaded at 0x%lx\n", gop_LUT_addr);
		if (ow_cfg.fmt == SCL_GOP_FMT_256LUT) {
			sclr_gop_setup_256LUT(inst, layer, 255, (u16 *) gop_LUT_addr);
		}else if (ow_cfg.fmt == SCL_GOP_FMT_16LUT){
			sclr_gop_setup_16LUT(inst, layer, 15, (u16 *) gop_LUT_addr);
		}else{
			uartlog("No such format in LUT fmt.\n");
		}

		break;
	}

	case CFG_OP_VPSS_GOP_SCL:
	{
		// u8 inst = va_arg(ap, int);
		uartlog("Please make sure all ow is ARGB8888!!\n");
		u8 layer;
		if (inst < SCL_MAX_INST) {
			bm_get_num("Control which gop, gop0(0)/gop1(1): ", layer);
			if(layer >= SCL_MAX_GOP_INST) {
				uartlog("Do not have such layer%d", layer);
				return -1;
			}
		} else {
			uartlog("Do not have such GOP%d!!!\n", inst);
			return -1;
		}

		// Get global g_gop_cfg[inst][layer] or  &g_disp_cfg.gop_cfg
		struct sclr_gop_cfg *gop_cfg = sclr_gop_get_cfg(inst, layer);

		if (layer == 0 || layer == 1) {
			bm_get_num("h-scal(x2) enable, y(1)/n(0): ", gop_cfg->gop_ctrl.b.hscl_en);
			bm_get_num("v-scal(x2) enable, y(1)/n(0): ", gop_cfg->gop_ctrl.b.vscl_en);
		}
		sclr_gop_set_cfg(inst, layer, gop_cfg);

		for (int i = 0; i < 8; ++i) {
			struct sclr_gop_ow_cfg *ow_cfg;
			if (layer == 0 || layer == 1) {
				ow_cfg = &gop_cfg->ow_cfg[i];
				if (gop_cfg->gop_ctrl.raw & BIT(i)) {
					ow_cfg->end.x = ow_cfg->start.x +
							(ow_cfg->img_size.w << gop_cfg->gop_ctrl.b.hscl_en) - gop_cfg->gop_ctrl.b.hscl_en;
					ow_cfg->end.y = ow_cfg->start.y +
							(ow_cfg->img_size.h << gop_cfg->gop_ctrl.b.vscl_en) - gop_cfg->gop_ctrl.b.vscl_en;
				}
			}
			sclr_gop_ow_set_cfg(inst, layer, i, ow_cfg);
		}
		break;
	}

	case CFG_OP_VPSS_GOP_DECOMPR:
	{
		// u8 inst = va_arg(ap, int);
		u8 oenc_inst = 0;
		u8 oenc_num;
		struct sclr_gop_odec_cfg odec_cfg;
		memset(&odec_cfg, 0, sizeof(odec_cfg));
		u8 layer;
		if (inst < SCL_MAX_INST) {
			uartlog("oenc_inst is %d", oenc_inst);
			bm_get_num("Control which gop, gop0(0)/gop1(1): ", layer);
			if(layer >= SCL_MAX_GOP_INST) {
				uartlog("Do not have such layer%d", layer);
				return -1;
			}
		} else {
			uartlog("Do not have such GOP%d!!!\n", inst);
			return -1;
		}
#if 1
		// Get global g_gop_cfg[inst][layer] or  &g_disp_cfg.gop_cfg
		struct sclr_gop_cfg *gop_cfg = sclr_gop_get_cfg(inst, layer);

		bm_get_num("OSD de-compression enable, y(1)/n(0): ", gop_cfg->odec_cfg.odec_ctrl.b.odec_en);
		odec_cfg.odec_ctrl.b.odec_en = gop_cfg->odec_cfg.odec_ctrl.b.odec_en;

		bm_get_num("OSD watch dog enable, y(1)/n(0): ", gop_cfg->odec_cfg.odec_ctrl.b.wdt_en);
		odec_cfg.odec_ctrl.b.wdt_en = gop_cfg->odec_cfg.odec_ctrl.b.wdt_en;
		if (odec_cfg.odec_ctrl.b.odec_en) {
			// bm_get_num("Only one OW can be set to de-compression. Which OW?(0~7): ",
			// 		gop_cfg->odec_cfg.odec_ctrl.b.odec_attached_idx);
			// odec_cfg.odec_ctrl.b.odec_attached_idx = gop_cfg->odec_cfg.odec_ctrl.b.odec_attached_idx;
			odec_cfg.odec_ctrl.b.odec_int_en = 1;
			uartlog("oenc_inst is %d", oenc_inst);
			bm_get_num("ows_oenc_num: ", oenc_num);
			sclr_gop_odec_set_cfg_from_oenc(inst, oenc_inst, layer, &odec_cfg, oenc_num);
		}
#endif
		break;
	}

	case CFG_OP_VPSS_GOP_8_DEC_CFG:
	{
		// u8 inst = va_arg(ap, int);
		u8 layer;
		u8 ow_inst;
		u8 dec_num;
		u16 mem_width;
		u16 width;
		u16 height;
		struct sclr_gop_ow_cfg ow_cfg;
		struct sclr_size osd_size;

		if (inst < SCL_MAX_INST) {
			bm_get_num("Control which gop, gop0(0)/gop1(1): ", layer);
			if(layer >= SCL_MAX_GOP_INST) {
				uartlog("Do not have such layer%d", layer);
				return -1;
			}
		} else {
			uartlog("Do not have such GOP%d!!!\n", inst);
			return -1;
		}

		bm_get_num("dec_num: ", dec_num);

		bm_get_num("format(0:ARGB8888, 1:ARGB4444, 2:ARGB1555, 3:256LUT, 4:16LUT, 5:Font): ", ow_cfg.fmt);

		bm_get_num("osd width: ", width);
		bm_get_num("osd height: ", height);

		for (int dec_count = 0; dec_count < dec_num; dec_count++)
		{

			// Get global g_gop_cfg[inst][layer] or  &g_disp_cfg.gop_cfg
			struct sclr_gop_cfg *gop_cfg = sclr_gop_get_cfg(inst, layer);

			// bm_get_num("ow inst (0~7): ", ow_inst);
			// bm_get_num("format(0:ARGB8888, 1:ARGB4444, 2:ARGB1555, 3:256LUT, 4:16LUT, 5:Font): ", ow_cfg.fmt);

			// bm_get_num("osd width: ", width);
			// bm_get_num("osd height: ", height);
			ow_inst=dec_count;
			// ow_cfg.fmt=ow_cfg.fmt;
			osd_size.w=width;
			osd_size.h=height;
			ow_cfg.img_size = osd_size;
			ow_cfg.mem_size.h = osd_size.h;

			//OW crop pixels (per line)
			//Format ARGB8888,			crop pixels valid value: 0~3
			//Format ARGB4444/ARGB1555, crop pixels valid value: 0~7
			//Format 256LUT,			crop pixels valid value: 0~15
			//Format 16-LUT,			crop pixels valid value: 0~31
			//Format Font-base, 		crop pixels valid value: 0~127"
			// bm_get_num("crop width: ", ow_cfg.crop_pixels);
			ow_cfg.crop_pixels =0;
			switch (ow_cfg.fmt) {
				case SCL_GOP_FMT_ARGB8888:
					if (ow_cfg.crop_pixels > 3) {
						uartlog("fmt(ARGB8888) crop_pixels need to be smaller than 3.\n");
						uartlog("invalid crop_pixels(%d)\n", ow_cfg.crop_pixels);
						return -1;
					}
					osd_size.w -= ow_cfg.crop_pixels;
					mem_width = osd_size.w * 4;
					break;
				case SCL_GOP_FMT_ARGB4444:
				case SCL_GOP_FMT_ARGB1555:
					if (ow_cfg.crop_pixels > 7) {
						uartlog("fmt(ARGB4444/1555) crop_pixels need to be smaller than 7.\n");
						uartlog("invalid crop_pixels(%d)\n", ow_cfg.crop_pixels);
						return -1;
					}
					osd_size.w -= ow_cfg.crop_pixels;
					mem_width = osd_size.w * 2;
					break;
				case SCL_GOP_FMT_256LUT:
					if (ow_cfg.crop_pixels > 15) {
						uartlog("fmt(256LUT) crop_pixels need to be smaller than 15.\n");
						uartlog("invalid crop_pixels(%d)\n", ow_cfg.crop_pixels);
						return -1;
					}
					osd_size.w -= ow_cfg.crop_pixels;
					mem_width = osd_size.w;
					break;
				case SCL_GOP_FMT_16LUT:
					if (ow_cfg.crop_pixels > 31) {
						uartlog("fmt(16LUT) crop_pixels need to be smaller than 31.\n");
						uartlog("invalid crop_pixels(%d)\n", ow_cfg.crop_pixels);
						return -1;
					}
					osd_size.w -= ow_cfg.crop_pixels;
					mem_width = (osd_size.w + 1) >> 1;
					break;
				case SCL_GOP_FMT_FONT:
					if (ow_cfg.crop_pixels > 127) {
						uartlog("fmt(font) crop_pixels need to be smaller than 127.\n");
						uartlog("invalid crop_pixels(%d)\n", ow_cfg.crop_pixels);
						return -1;
					}
					osd_size.w -= ow_cfg.crop_pixels;
					mem_width = (osd_size.w + 7) >> 3;
					break;
				default:
					uartlog("invalid fmt(%d)\n", ow_cfg.fmt);
					return -1;
			}
			ow_cfg.pitch = ALIGN(mem_width, 16);
			ow_cfg.mem_size.w = ALIGN(mem_width, 16);
			uartlog("mem size: (%d*%d), pitch(%d)\n", ow_cfg.mem_size.w, ow_cfg.mem_size.h, ow_cfg.pitch);

			// bm_get_num("OSD frame start pos x: ", ow_cfg.start.x);
			// bm_get_num("OSD frame start pos y: ", ow_cfg.start.y);
			ow_cfg.start.x=100 * dec_count;
			ow_cfg.start.y=dec_count*60;

			if (layer == 0 || layer == 1) {
				ow_cfg.end.x = ow_cfg.start.x +
						(osd_size.w << gop_cfg->gop_ctrl.b.hscl_en) - gop_cfg->gop_ctrl.b.hscl_en;
				ow_cfg.end.y = ow_cfg.start.y +
						(osd_size.h << gop_cfg->gop_ctrl.b.vscl_en) - gop_cfg->gop_ctrl.b.vscl_en;
			} else {
				uartlog("Do not have such layer%d", layer);
				return -1;
			}

			uartlog("OSD frame end position: (%d*%d)\n", ow_cfg.end.x, ow_cfg.end.y);
			uartlog("OSD frame start position: (%d*%d)\n", ow_cfg.start.x, ow_cfg.start.y);

			// count down from space's end
			// gop_space[inst] -= (ow_cfg.pitch * ow_cfg.mem_size.h);

			// inst and ow_inst begin from 0
			// ow_cfg.addr = gop_addr + GOP_FRAME_OFFSET * (inst + 1) * (ow_inst + 1) + gop_space[inst];
			ow_cfg.addr = gop_addr + GOP_FRAME_OFFSET * (0) * (0) + gop_space[0];

			uartlog("osd mem addr: (0x%09lx)\n", ow_cfg.addr);

			sclr_gop_ow_set_cfg(inst, layer, ow_inst, &ow_cfg);

			if (ow_cfg.fmt == SCL_GOP_FMT_FONT) {
				struct sclr_gop_fb_cfg fb_cfg;
				u16 pix_num;
				int fb_inst;

				bm_get_num("enable fontbox?, y(1)/n(0): ", fb_cfg.fb_ctrl.b.enable);
				if (fb_cfg.fb_ctrl.b.enable) {
					bm_get_num("which fontbox?, (0 ~ 1): ", fb_inst);
					fb_cfg.fb_ctrl.b.attach_ow = ow_inst;
					bm_get_num("hi threshold, (0 ~ 255): ", tmp);
					if (layer == 0 || layer == 1) {
						gop_cfg->fb_ctrl.b.hi_thr = tmp >> 2;
						gop_cfg->fb_ctrl.b.lo_thr = gop_cfg->fb_ctrl.b.hi_thr * 0.8;
						bm_get_num("invert condition?, low(1)/high(0): ", tmp);
						gop_cfg->fb_ctrl.b.lo_thr_inv = tmp;
					}
					sclr_gop_set_cfg(inst, layer, gop_cfg);

					bm_get_num("fontbox num?, (1 ~ 32): ", tmp);
					fb_cfg.fb_ctrl.b.fb_num = tmp - 1;
					if (osd_size.w % tmp) {
						uartlog("fontbox-num must divide the ow_width(%d)\n", osd_size.w);
						break;
					}
					fb_cfg.fb_ctrl.b.width = osd_size.w / tmp - 1;
					// pix_thr max(32), unit(8 pixel).
					// reduce sample_rate if too many pixels in fb
					pix_num = (fb_cfg.fb_ctrl.b.width + 1) * osd_size.h;
					fb_cfg.fb_ctrl.b.sample_rate = (pix_num >> 3) >> 5;
					fb_cfg.fb_ctrl.b.pix_thr = ((pix_num >> 3) >> fb_cfg.fb_ctrl.b.sample_rate) * 0.65;
					uartlog("fb_cfg.fb_ctrl.b.pix_thr(%d)\n", fb_cfg.fb_ctrl.b.pix_thr);
				}
				sclr_gop_fb_set_cfg(inst, layer, fb_inst, &fb_cfg);
			}
		}
		break;
	}

	case CFG_OP_VPSS_STATUS_GOP:
	{
		for(int i = 0; i <= SCL_MAX_INST; ++i) {
			for(int layer = 0; i <= SCL_MAX_GOP_INST; ++i) {
				struct sclr_gop_cfg *cfg = sclr_gop_get_cfg(i, layer);
				uartlog("-------------------GOP%d layer%d summary-------------------------\n", i, layer);
				uartlog("ow_enable(%d-%d-%d-%d-%d-%d-%d-%d) 	scale_up(%dx%d) 	colorkey_en(%d) 	burst(%d)\n",
						cfg->gop_ctrl.b.ow0_en, cfg->gop_ctrl.b.ow1_en, cfg->gop_ctrl.b.ow2_en,
						cfg->gop_ctrl.b.ow3_en, cfg->gop_ctrl.b.ow4_en, cfg->gop_ctrl.b.ow5_en,
						cfg->gop_ctrl.b.ow6_en, cfg->gop_ctrl.b.ow7_en, cfg->gop_ctrl.b.hscl_en,
						cfg->gop_ctrl.b.vscl_en, cfg->gop_ctrl.b.colorkey_en, cfg->gop_ctrl.b.burst);
				uartlog("colorkey(%d-%d-%d)\t\tfont_fg_color(%d-%d-%d-%d)\t\tfont_bg_color(%d-%d-%d-%d)\n",
						(cfg->colorkey >> 16) & 0xff, (cfg->colorkey >> 8) & 0xff, cfg->colorkey & 0xff,
						(cfg->font_fg_color >> 12) && 0xff, (cfg->font_fg_color >> 8) && 0xff,
						(cfg->font_fg_color >> 4) && 0xff, cfg->font_fg_color && 0xff,
						(cfg->font_bg_color >> 12) && 0xff, (cfg->font_bg_color >> 8) && 0xff,
						(cfg->font_bg_color >> 4) && 0xff, cfg->font_bg_color && 0xff);

				for (int j = 0; j < 8; ++j) {
					uartlog("-------------------layer%d osd windows-------------------------\n", layer);
					uartlog("[ow%d]  fmt(%8s)\toverlay-start(%4d*%4d)\tend(%d*%d)\n", j,
							str_sclr_gop_fmt[cfg->ow_cfg[j].fmt],
							cfg->ow_cfg[j].start.x, cfg->ow_cfg[j].start.y,
							cfg->ow_cfg[j].end.x, cfg->ow_cfg[j].end.y);
					uartlog("mem addr(0x%09lx)\tsize(%4d*%4d)\t\t\tpitch(%4d)\n",
							cfg->ow_cfg[j].addr,
							cfg->ow_cfg[j].mem_size.w, cfg->ow_cfg[j].mem_size.h,
							cfg->ow_cfg[j].pitch);
				}
			}
		}
		break;
	}

	case 255:
		ret = 1;
		break;
	default:
		uartlog("undefined cfg-op(%d)\n", op);
		break;
	}

	va_end(ap);

	return ret;
}

/********************************************************************
 *		 cli commands
 ********************************************************************/
int do_cfg(int argc, char **argv)
{
	u32 tmp;
	u8 op = 0xff;
	do {
		uartlog("\n--------------------------------------------------------------------\n");
		uartlog("00) vpss sc_set_oenc 01) vpss gop_LUT_cfg  02) vpss gop_ow_cfg  	03) vpss gop_scale_cfg\n");
		uartlog("04) vpss gop_cfg     05) vpss sc_set_cover 06) vpss 8 ow gop_dec       07) vpss gop_dec\n");
		uartlog("08) vpss gop status\n");
		uartlog("60) All status  255) exit\n");

		bm_get_num(": ", op);

		u8 num_args = 0;
		if (op < CFG_OP_VPSS_STATUS_ALL) {
			int inst;
			bm_get_num("vpss inst, 0~3: ", inst);
			if (inst >= SCL_MAX_INST) {
				uartlog("img_inst inst(%d) does not exist.\n", inst);
				return -1;
			}
			if (_handle_cfg_op(op, inst, num_args) != 0) break;
		} else {
			if (_handle_cfg_op(op, num_args) != 0) break;
		}
		op = 0xff;
	} while(1);

	return 0;
}

void default_src_mem(u16 w, u16 h, enum sclr_format fmt, u8 src_idx,
		struct sclr_mem *mem)
{
	u16 pitch[3];

	_get_bytesperline(fmt, w, pitch);
	mem->width = w;
	mem->height = h;
	mem->start_x = 0;
	mem->start_y = 0;
	mem->pitch_y = pitch[0];
	mem->pitch_c = pitch[1];

#ifdef SINGLE_PACKED
	const struct cvi_vip_fmt *vip_fmt;

	for (int k = 0; k < ARRAY_SIZE(cvi_vip_formats); k++) {
		vip_fmt = &cvi_vip_formats[k];
		if (vip_fmt->fmt == fmt)
			break;
	}
	mem->addr0 = src_addr_scl[src_idx][R_IDX];
	mem->addr1 = mem->addr0 + pitch[0] * h;
	mem->addr2 = mem->addr1 + (pitch[1] * h / vip_fmt->plane_sub_v);
#else
	mem->addr0 = src_addr_scl[src_idx][R_IDX];
	mem->addr1 = src_addr_scl[src_idx][G_IDX];
	mem->addr2 = src_addr_scl[src_idx][B_IDX];
#endif
}

void default_dst_mem(u16 w, u16 h, enum sclr_format fmt, u8 dst_idx,
		struct sclr_mem *mem, u16 data_size)
{
	u16 pitch[3];

	_get_bytesperline(fmt, w, pitch);
	mem->width = w;
	mem->height = h;
	mem->start_x = 0;
	mem->start_y = 0;
	mem->pitch_y = pitch[0] * data_size;
	mem->pitch_c = pitch[1] * data_size;

#ifdef SINGLE_PACKED
	const struct cvi_vip_fmt *vip_fmt;

	for (int k = 0; k < ARRAY_SIZE(cvi_vip_formats); k++) {
		vip_fmt = &cvi_vip_formats[k];
		if (vip_fmt->fmt == fmt)
			break;
	}
	mem->addr0 = dst_addr_scl[dst_idx][R_IDX];
	mem->addr1 = mem->addr0 + pitch[0] * data_size * h;
	mem->addr2 = mem->addr1 + (pitch[1] * data_size * h / vip_fmt->plane_sub_v);
#else
	mem->addr0 = dst_addr_scl[dst_idx][R_IDX];
	mem->addr1 = dst_addr_scl[dst_idx][G_IDX];
	mem->addr2 = dst_addr_scl[dst_idx][B_IDX];
#endif
}


void show_dump_mem(enum sclr_format fmt, struct sclr_mem *mem)
{
	fmt = fmt;
	mem = mem;
#if 0
	const struct cvi_vip_fmt *vip_fmt;
	u16 stride0 = mem->pitch_y;
	u16 stride1 = mem->pitch_c;
	u16 h = mem->height;
	u32 len, plane_len[3] = {0, 0, 0};
	u64 start_addr, end_addr;
	u64 addr[3] = {mem->addr0, mem->addr1, mem->addr2};


	for (int k = 0; k < ARRAY_SIZE(cvi_vip_formats); k++) {
		vip_fmt = &cvi_vip_formats[k];
		if (vip_fmt->fmt == fmt)
			break;
	}

	for (int p = 0; p < vip_fmt->buffers; p++) {
		len = (p == 0) ? (stride0 * h) : (stride1 * h);
		plane_len[p] = len / vip_fmt->plane_sub_v;
	}
	// uartlog("output mem addr(0x%lx, 0x%lx, 0x%lx).\n",
	// 	mem->addr0, mem->addr1, mem->addr2);
	// uartlog("output mem len(%d, %d, %d).\n",
	// 	plane_len[0], plane_len[1], plane_len[2]);

#ifdef SINGLE_PACKED
	start_addr = (addr[0] - 0x80000000)/2;
	end_addr = addr[0];

	for (int p = 0; p < vip_fmt->buffers; p++) {
		len = (p == 0) ? (stride0 * h) : (stride1 * h);
		len = (p == 0) ? len : (len / vip_fmt->plane_sub_v);
		end_addr += len;
	}
	end_addr = (end_addr - 0x80000000)/2 - 1;
	// uartlog("pld dump addr, single packed : start(0x%lx) end (0x%lx).\n", start_addr, end_addr);
	// uartlog("fpga dump addr, single packed : start(0x%lx) len:0x%x (%d).\n", addr[0],
	// 	plane_len[0] + plane_len[1] + plane_len[2], plane_len[0] + plane_len[1] + plane_len[2]);
#else
	for (int p = 0; p < vip_fmt->buffers; p++) {
		len = (p == 0) ? (stride0 * h) : (stride1 * h);
		len = len / vip_fmt->plane_sub_v;
		start_addr = (addr[p] - 0x80000000)/2;
		end_addr = ((addr[p] + len) - 0x80000000)/2;
		uartlog("pld dump addr,planar%d: start(0x%lx) end (0x%lx).\n", p, start_addr, end_addr);
		uartlog("fpga dump addr,planar%d: start(0x%lx) len:0x%x (%d).\n", p, addr[p], len, len);
	}
#endif
#endif
}

void is_odd_yuv(struct cvi_vpss_cfg *cfg){
	if(cfg->img_cfg.fmt_in == SCL_FMT_YUV420 || cfg->img_cfg.fmt_in == SCL_FMT_YUV422)
		if(cfg->img_cfg.mem.width % 2 == 1){
			cfg->img_cfg.mem.width -= 1;
			cfg->sc_cfg.crop.w -= 1;
		}
	if(cfg->sc_cfg.fmt_out == SCL_FMT_YUV420 || cfg->sc_cfg.fmt_out == SCL_FMT_YUV422)
		if(cfg->sc_cfg.dst.w % 2 == 1){
			cfg->sc_cfg.dst.w -= 1;
			cfg->sc_cfg.mem.width -= 1;
		}

	if(cfg->img_cfg.fmt_in == SCL_FMT_YUV420 || cfg->img_cfg.fmt_in == SCL_FMT_NV12 || cfg->img_cfg.fmt_in == SCL_FMT_NV21)
		if(cfg->img_cfg.mem.height % 2 == 1){
			cfg->img_cfg.mem.height -= 1;
			cfg->sc_cfg.crop.h -= 1;
		}
	if(cfg->sc_cfg.fmt_out == SCL_FMT_YUV420 || cfg->sc_cfg.fmt_out == SCL_FMT_NV12 || cfg->sc_cfg.fmt_out == SCL_FMT_NV21)
		if(cfg->sc_cfg.dst.h % 2 == 1){
			cfg->sc_cfg.dst.h -= 1;
			cfg->sc_cfg.mem.height -= 1;
		}
}

//single trig
int vpss_start(enum vpss_dev vpss_inst, bool tile_enable, bool v_tile_enable)
{
	int ret = 0;
	struct sclr_img_checksum_status ch_status;
	struct sclr_core_checksum_status sc_ch_status;
	sclr_top_set_cfg(vpss_inst, true);
	sclr_top_set_src_share(vpss_inst, false);
	if (tile_enable)
		sclr_left_tile(vpss_inst);
	if (v_tile_enable)
		sclr_top_tile(vpss_inst);
	vpss_enable_mask = BIT(vpss_inst);
	uartlog("vpss(%d) start\n", vpss_inst);
	uartlog("vpss_enable_mask(%d)\n", vpss_enable_mask);
	sclr_trig_start(vpss_inst);
	// uartlog("vpss_enable_mask(%d)\n", vpss_enable_mask);
	if (sclr_check_irq_handler(WAIT_TIME) != 0) {
		uartlog("sclr end intr doesn't happens after %d tick\n", WAIT_TIME);
		// sclr_img_reset(vpss_inst);
		vpss_enable_mask = 0;
		fail_trigger();
		ret = -1;
	} else {
		// uartlog("sclr done\n");
		// uartlog("sc(%d) odma status(%#x)\n", vpss_inst, sclr_odma_get_dbg_status(vpss_inst).raw);
		sclr_img_get_checksum_status(vpss_inst, &ch_status);
		// uartlog("checksum en(%d)\n", ch_status.checksum_base.b.enable);
		// uartlog("checksum axi_read_data(0x%x)\n", ch_status.axi_read_data);
		// uartlog("checksum output_data(0x%x)\n", ch_status.checksum_base.b.output_data);
		sclr_core_get_checksum_status(vpss_inst, &sc_ch_status);
		// uartlog("checksum en(%d)\n", sc_ch_status.checksum_base.b.enable);
		// uartlog("checksum axi_write_data(0x%x)\n", sc_ch_status.axi_write_data);
		// uartlog("checksum data_in(0x%x)\n", sc_ch_status.checksum_base.b.data_in_from_img_in);
		// uartlog("checksum data_out(0x%x)\n", sc_ch_status.checksum_base.b.data_out);
		if (v_tile_enable){
			sclr_down_tile(vpss_inst);
			vpss_enable_mask = BIT(vpss_inst);
			sclr_trig_start(vpss_inst);
			if (sclr_check_irq_handler(WAIT_TIME) != 0) {
				uartlog("tile mode on right failed\n");
				sclr_img_reset(vpss_inst);
				vpss_enable_mask = 0;
				fail_trigger();
				ret = -1;
			} else {
				uartlog("tile mode ok\n");
				frame_done_trigger();
			}
		}
		if (tile_enable) {
			sclr_right_tile(vpss_inst);
			if (v_tile_enable){
				sclr_top_tile(vpss_inst);
				vpss_enable_mask = BIT(vpss_inst);
				sclr_trig_start(vpss_inst);
				if (sclr_check_irq_handler(WAIT_TIME) != 0) {
					uartlog("tile mode on right failed\n");
					sclr_img_reset(vpss_inst);
					vpss_enable_mask = 0;
					fail_trigger();
					ret = -1;
				} else {
					uartlog("tile mode ok\n");
					frame_done_trigger();
				}
				sclr_down_tile(vpss_inst);
			}
			vpss_enable_mask = BIT(vpss_inst);
			sclr_trig_start(vpss_inst);
			if (sclr_check_irq_handler(WAIT_TIME) != 0) {
				uartlog("tile mode on right failed\n");
				sclr_img_reset(vpss_inst);
				vpss_enable_mask = 0;
				fail_trigger();
				ret = -1;
			} else {
				uartlog("tile mode ok\n");
				frame_done_trigger();
			}
		} else {
			frame_done_trigger();
		}
	}
	return ret;
}

int basic(struct cvi_vpss_cfg *cfg)
{
	int ret = 0;
	struct sclr_img_cfg img = {0};
	struct sclr_core_cfg core_cfg = {0};
	struct sclr_odma_cfg odma_cfg = {0};
	u8 inst = cfg->inst;
	is_odd_yuv(cfg);
	//setup img

	img.burst = 15;
	img.outstanding = 15;
	img.fifo_y = 130;
	img.fifo_c = (cfg->img_cfg.fmt_in == SCL_FMT_YUV420 || cfg->img_cfg.fmt_in == SCL_FMT_YUV422) ? 65 : 130;
	img.src = cfg->img_cfg.in;
	img.csc = cfg->img_cfg.csc;
	img.fmt = cfg->img_cfg.fmt_in;
	img.mem = cfg->img_cfg.mem;

	if (!IS_YUV_FMT(img.fmt)) {
		img.csc = SCL_CSC_NONE;
	}
	sclr_img_checksum_en(inst, 1);
	sclr_img_set_cfg(inst, &img);
	// uartlog("\n----vpss(%d) img_in----\n", inst);
	// uartlog("in_fmt(%d), rect(%d %d %d %d)\n", img.fmt, img.mem.start_x, img.mem.start_y, img.mem.width, img.mem.height);
	// uartlog("vpss(%d) input mem addr(%lx %lx %lx).\n", inst, img.mem.addr0, img.mem.addr1, img.mem.addr2);

	//setup sc core
	core_cfg.sc.src.w = cfg->img_cfg.mem.width;
	core_cfg.sc.src.h = cfg->img_cfg.mem.height;
	core_cfg.sc.crop = cfg->sc_cfg.crop;
	core_cfg.sc.dst = cfg->sc_cfg.dst;
	core_cfg.sc.algorithm = cfg->sc_cfg.algorithm;
	core_cfg.sc.tile_enable = cfg->tile_enable;
	core_cfg.sc.v_tile_enable = cfg->v_tile_enable;
	core_cfg.sc.mir_enable = 1;
	sclr_core_set_cfg(inst, &core_cfg);
	sclr_core_checksum_en(inst, 1);
	sclr_set_cfg(inst, false, false, true, false);
	// uartlog("\n--sc core--\n");
	// uartlog("crop(%d %d %d %d)\n", core_cfg.sc.crop.x, core_cfg.sc.crop.y, core_cfg.sc.crop.w, core_cfg.sc.crop.h);
	// uartlog("size(%d %d)\n", core_cfg.sc.dst.w, core_cfg.sc.dst.h);

	// setup odma
	// uartlog("\n--odma--\n");
	if ((cfg->sc_cfg.out_mode == SCL_OUT_CSC && IS_YUV_FMT(cfg->sc_cfg.fmt_out)) ||
			(cfg->sc_cfg.out_mode != SCL_OUT_CSC && !IS_YUV_FMT(cfg->sc_cfg.fmt_out))
		) {
			odma_cfg.csc_cfg.datatype = cfg->sc_cfg.datatype;
			odma_cfg.csc_cfg.mode = cfg->sc_cfg.out_mode;
			odma_cfg.csc_cfg.csc_type = cfg->sc_cfg.csc;
		// use yuv format if output-mode is csc, o/w should go rgb format.
	} else {
		uartlog("Invalid parameter, fmt(%d) should be rgb if mode(%d) is not csc.\n",
			cfg->sc_cfg.fmt_out, cfg->sc_cfg.out_mode);
		ret = -1;
	}

	if(cfg->sc_cfg.out_mode == SCL_OUT_QUANT){
		odma_cfg.csc_cfg.quant_form = cfg->sc_cfg.quant_form;
		odma_cfg.csc_cfg.quant_round = cfg->sc_cfg.quant_round;
	}
	else if(cfg->sc_cfg.out_mode == SCL_OUT_CONVERT_TO)
		odma_cfg.csc_cfg.convert_to_cfg = cfg->sc_cfg.convert_to_cfg;

	odma_cfg.csc_cfg.work_on_border = true;
	odma_cfg.flip = cfg->sc_cfg.flip;
	odma_cfg.burst = 7;
	odma_cfg.fmt = cfg->sc_cfg.fmt_out;
	odma_cfg.mem = cfg->sc_cfg.mem;
	sclr_odma_set_cfg(inst, &odma_cfg);

	//uartlog("odma_size(%d %d)\n", odma_cfg.mem.width, odma_cfg.mem.height);
	show_dump_mem(odma_cfg.fmt, &odma_cfg.mem);

	return ret;
}

int test_vpss_basic(int argc, char **argv)
{
	int ret = 0;
	int capab = 0;
	u16 w;
	u16 h;
	u32 tmp;
	struct cvi_vpss_cfg cfg;
	enum vpss_dev inst = (enum vpss_dev)strtol(argv[1], NULL, 10);
	bm_get_num("vpss capab: 0:800x600, 1:1920x1080, 2:3840x2160 ", capab);
	if (capab == 0) {
		w = DEFAULT_W;
		h = DEFAULT_H;
	} else if (capab == 1) {
		w = 1920;
		h = 1080;
	} else if (capab == 2) {
		w = 3840;
		h = 2160;
	}
	memset(&cfg, 0, sizeof(struct cvi_vpss_cfg));
	cfg.inst = inst;
	cfg.tile_enable = false;
	cfg.img_cfg.in = SCL_INPUT_MEM;
	// cfg.img_cfg.fmt_in = SCL_FMT_NV21;
	cfg.img_cfg.fmt_in = SCL_FMT_RGB_PACKED;
	cfg.img_cfg.csc = SCL_CSC_NONE;
	default_src_mem(w, h, cfg.img_cfg.fmt_in, 0, &cfg.img_cfg.mem);

	cfg.sc_cfg.crop.x = 0;
	cfg.sc_cfg.crop.y = 0;
	cfg.sc_cfg.crop.w = w;
	cfg.sc_cfg.crop.h = h;
	cfg.sc_cfg.dst.w = w;
	cfg.sc_cfg.dst.h = h;
	// cfg.sc_cfg.fmt_out = SCL_FMT_NV21;
	cfg.sc_cfg.fmt_out = SCL_FMT_RGB_PACKED;
	cfg.sc_cfg.out_mode = SCL_OUT_DISABLE;
	cfg.sc_cfg.csc = SCL_CSC_NONE;
	cfg.sc_cfg.flip = SCL_FLIP_NO;
	cfg.sc_cfg.algorithm = SCL_BICUBIC;
	default_dst_mem(w, h, cfg.sc_cfg.fmt_out, 0, &cfg.sc_cfg.mem, 1);

	ret = basic(&cfg);
	ret |= vpss_start(inst, cfg.tile_enable, false);
	return ret;
}

int test_gop(int argc, char **argv)
{
	int ret = 0;
	u16 mem_width;
	u16 w, h, fmt_in;
	enum vpss_dev inst = (enum vpss_dev)strtol(argv[1], NULL, 10);
	struct cvi_vpss_cfg cfg;
	struct sclr_gop_ow_cfg ow_cfg;
	struct sclr_size osd_size;
	struct sclr_gop_cfg *gop_cfg = sclr_gop_get_cfg(inst, 0);
	fmt_in = strtol(argv[2], NULL, 10);
	w = strtol(argv[3], NULL, 10);
	h = strtol(argv[4], NULL, 10);

	memset(&cfg, 0, sizeof(struct cvi_vpss_cfg));
	memset(&ow_cfg, 0, sizeof(struct sclr_gop_ow_cfg));
	memset(&osd_size, 0, sizeof(struct sclr_size));
	cfg.inst = inst;
	cfg.tile_enable = false;
	cfg.img_cfg.in = SCL_INPUT_MEM;
	cfg.img_cfg.fmt_in = fmt_in;
	cfg.img_cfg.csc = IS_YUV_FMT(fmt_in) ? SCL_CSC_601_LIMIT_YUV2RGB : SCL_CSC_NONE;
	default_src_mem(w, h, cfg.img_cfg.fmt_in, 0, &cfg.img_cfg.mem);

	cfg.sc_cfg.crop.x = 0;
	cfg.sc_cfg.crop.y = 0;
	cfg.sc_cfg.crop.w = w;
	cfg.sc_cfg.crop.h = h;
	cfg.sc_cfg.dst.w = w;
	cfg.sc_cfg.dst.h = h;
	cfg.sc_cfg.fmt_out = SCL_FMT_YUV420;
	cfg.sc_cfg.out_mode = SCL_OUT_CSC;
	cfg.sc_cfg.csc = SCL_CSC_601_LIMIT_RGB2YUV;
	cfg.sc_cfg.flip = SCL_FLIP_NO;
	cfg.sc_cfg.algorithm = SCL_BICUBIC;
	default_dst_mem(w, h, cfg.sc_cfg.fmt_out, 0, &cfg.sc_cfg.mem, 1);

	ret = basic(&cfg);

	sclr_set_cfg(inst, false, false, true, false);
	gop_cfg->gop_ctrl.b.ow0_en = 1;
	gop_cfg->gop_ctrl.b.burst = 15;
	sclr_gop_set_cfg(inst, 0, gop_cfg);
	sclr_gop_set_cfg(inst, 1, gop_cfg);

	ow_cfg.fmt = 0;
	osd_size.w = strtol(argv[7], NULL, 10);
	osd_size.h = strtol(argv[8], NULL, 10);
	ow_cfg.img_size = osd_size;

	ow_cfg.crop_pixels = 0;
	osd_size.w -= ow_cfg.crop_pixels;
	mem_width = osd_size.w * 4;
	ow_cfg.pitch = ALIGN(mem_width, 16);
	ow_cfg.mem_size.w = ALIGN(mem_width, 16);
	ow_cfg.mem_size.h = ow_cfg.img_size.h;
	ow_cfg.start.x = strtol(argv[5], NULL, 10);
	ow_cfg.start.y = strtol(argv[6], NULL, 10);
	ow_cfg.end.x = ow_cfg.start.x + osd_size.w;
	ow_cfg.end.y = ow_cfg.start.y + osd_size.h;
	// gop_space[inst] -= (ow_cfg.pitch * ow_cfg.mem_size.h);
	// ow_cfg.addr = gop_addr + GOP_FRAME_OFFSET * (inst + 1) * (ow_inst + 1) + gop_space[inst];
	ow_cfg.addr = gop_addr;  //0x102000000
	sclr_gop_ow_set_cfg(inst, 0, 0, &ow_cfg);
	ow_cfg.start.x = strtol(argv[5], NULL, 10) + 200;
	ow_cfg.end.x = ow_cfg.start.x + osd_size.w;
	sclr_gop_ow_set_cfg(inst, 1, 0, &ow_cfg);
	ret |= vpss_start(inst, cfg.tile_enable, false);
	return ret;
}

int test_max_size(int argc, char **argv)
{
	int ret = 0;
	struct cvi_vpss_cfg cfg;
	u16 max_w = 8192;
	u16 max_h = 270;
	u8 inst = strtol(argv[1], NULL, 10);

	memset(&cfg, 0, sizeof(struct cvi_vpss_cfg));
	cfg.inst = inst;
	cfg.tile_enable = true;
	cfg.v_tile_enable = (max_h > 8189);
	cfg.img_cfg.in = SCL_INPUT_MEM;
	cfg.img_cfg.fmt_in = strtol(argv[2], NULL, 10);
	cfg.img_cfg.csc = SCL_CSC_NONE;
	default_src_mem(max_w, max_h, cfg.img_cfg.fmt_in, 0, &cfg.img_cfg.mem);

	cfg.sc_cfg.crop.x = 0;
	cfg.sc_cfg.crop.y = 0;
	cfg.sc_cfg.crop.w = max_w;
	cfg.sc_cfg.crop.h = max_h;
	cfg.sc_cfg.dst.w = max_w;
	cfg.sc_cfg.dst.h = max_h;
	cfg.sc_cfg.fmt_out = strtol(argv[3], NULL, 10);
	cfg.sc_cfg.out_mode = IS_YUV_FMT(cfg.sc_cfg.fmt_out) ? SCL_OUT_CSC : SCL_OUT_DISABLE;
	cfg.sc_cfg.csc = IS_YUV_FMT(cfg.sc_cfg.fmt_out) ? SCL_CSC_601_LIMIT_RGB2YUV : SCL_CSC_NONE;
	cfg.sc_cfg.flip = SCL_FLIP_NO;
	cfg.sc_cfg.algorithm = SCL_BICUBIC;
	default_dst_mem(cfg.sc_cfg.dst.w, cfg.sc_cfg.dst.h, cfg.sc_cfg.fmt_out, 0, &cfg.sc_cfg.mem, 1);

	ret = basic(&cfg);
	ret |= vpss_start(inst, cfg.tile_enable, cfg.v_tile_enable);
	return ret;
}

int test_open_all(int argc, char **argv)
{
	int ret = 0;
	struct cvi_vpss_cfg cfg;
	enum vpss_dev inst = (enum vpss_dev)strtol(argv[1], NULL, 10);
	struct sclr_border_vpp_cfg border_cfg[BORDER_VPP_MAX];
	struct sclr_cover_cfg cover_cfg[COVER_MAX];
	struct sclr_border_cfg border_cfg1;
	struct sclr_privacy_cfg prm_cfg;
	struct sclr_cir_cfg cir_cfg;
	struct sclr_gop_ow_cfg ow_cfg;
	struct sclr_size osd_size;
	struct sclr_gop_cfg *gop_cfg = sclr_gop_get_cfg(inst, 0);
	u64 map_addr = dst_addr_scl[2][R_IDX];
	u8 *ptr = (u8 *)map_addr;
	int map_w, map_h;
	int in_w, in_h;

	memset(&cfg, 0, sizeof(struct cvi_vpss_cfg));
	cfg.inst = inst;
	cfg.tile_enable = false;
	cfg.img_cfg.in = SCL_INPUT_MEM;
	cfg.img_cfg.fmt_in = SCL_FMT_YUV420;
	cfg.img_cfg.csc = SCL_CSC_601_LIMIT_YUV2RGB;

	in_w = 4608;
	in_h = 270;
	default_src_mem(in_w, in_h, cfg.img_cfg.fmt_in, 0, &cfg.img_cfg.mem);

	cfg.sc_cfg.crop.x = 0;
	cfg.sc_cfg.crop.y = 0;
	cfg.sc_cfg.crop.w = in_w;
	cfg.sc_cfg.crop.h = in_h;
	cfg.sc_cfg.dst.w = 4408;
	cfg.sc_cfg.dst.h = 270;
	cfg.sc_cfg.fmt_out = SCL_FMT_NV21;
	cfg.sc_cfg.out_mode = SCL_OUT_CSC;
	cfg.sc_cfg.csc = SCL_CSC_601_LIMIT_RGB2YUV;
	cfg.sc_cfg.flip = SCL_FLIP_HVFLIP;
	cfg.sc_cfg.algorithm = SCL_BICUBIC;
	default_dst_mem(4608, 270, cfg.sc_cfg.fmt_out, 0, &cfg.sc_cfg.mem, 1);
	ret = basic(&cfg);

	for(int i = 0; i < BORDER_VPP_MAX; i++){
		border_cfg[i].cfg.b.enable = 1;
		border_cfg[i].cfg.b.bd_color_r = 255;
		border_cfg[i].cfg.b.bd_color_g = 255;
		border_cfg[i].cfg.b.bd_color_b = 0;
		border_cfg[i].inside_start.x = 100 + i * 150;
		border_cfg[i].inside_start.y = 100;
		border_cfg[i].inside_end.x = 200 + i * 150;
		border_cfg[i].inside_end.y = 200;
		border_cfg[i].outside_start.x = 90 + i * 150;
		border_cfg[i].outside_start.y = 90;
		border_cfg[i].outside_end.x = 210 + i * 150;
		border_cfg[i].outside_end.y = 210;
		sclr_border_vpp_set_cfg(inst, i, &border_cfg[i]);
	}

	for(int i = 0; i < COVER_MAX; i++){
		cover_cfg[i].start.b.enable = 1;
		cover_cfg[i].start.b.x = 500 + i * 150;
		cover_cfg[i].start.b.y = 100;
		cover_cfg[i].img_size.w = 100;
		cover_cfg[i].img_size.h = 100;
		cover_cfg[i].color.b.cover_color_r = 255;
		cover_cfg[i].color.b.cover_color_g = 0;
		cover_cfg[i].color.b.cover_color_b = 0;
		sclr_cover_set_cfg(inst, i, &cover_cfg[i]);
	}

	border_cfg1.cfg.b.enable = 1;
	border_cfg1.cfg.b.bd_color_r = 255;
	border_cfg1.cfg.b.bd_color_g = 255;
	border_cfg1.cfg.b.bd_color_b = 0;
	border_cfg1.start.x = 100;
	border_cfg1.start.y = 0;
	sclr_border_set_cfg(inst, &border_cfg1);

	prm_cfg.cfg.raw = 0;
	prm_cfg.cfg.b.enable = 1;
	prm_cfg.cfg.b.mode = 0;	// grid mode
	prm_cfg.cfg.b.grid_size = 0;	// 8x8
	prm_cfg.cfg.b.fit_picture = 0;	// no full image
	prm_cfg.cfg.b.force_alpha = 0;
	prm_cfg.map_cfg.alpha_factor = 255;
	prm_cfg.cfg.b.mask_rgb332 = 1;
	prm_cfg.map_cfg.no_mask_idx = 0;
	prm_cfg.map_cfg.base = map_addr;
	prm_cfg.map_cfg.axi_burst = 7;
	prm_cfg.start.x = 1000;
	prm_cfg.end.x = prm_cfg.start.x + 127 - 1;	// x + 128
	prm_cfg.start.y = 0;
	prm_cfg.end.y = prm_cfg.start.y + 128 - 1;	// y + 128
	map_w = 16;
	map_h = 16;
	for (int i = 0; i < map_h; i++)
		for (int j = 0; j < map_w; j++)
			ptr[i * map_w + j] = (u8)rand();
	sclr_pri_set_cfg(inst, &prm_cfg);

	memset(&cir_cfg, 0, sizeof(struct sclr_cir_cfg));
	cir_cfg.mode = SCL_CIR_LINE;
	cir_cfg.line_width = 10;
	cir_cfg.center.x = 1500;
	cir_cfg.center.y = 100;
	cir_cfg.radius = 100;
	cir_cfg.rect.x = 0;
	cir_cfg.rect.y = 0;
	cir_cfg.rect.w = 4408;
	cir_cfg.rect.h = 270;
	cir_cfg.color_r = 255;
	cir_cfg.color_g = 0;
	cir_cfg.color_b = 0;
	sclr_cir_set_cfg(inst, &cir_cfg);

	sclr_set_cfg(inst, false, false, false, false);
	gop_cfg->gop_ctrl.b.ow0_en = 1;
	sclr_gop_set_cfg(inst, 0, gop_cfg);
	sclr_gop_set_cfg(inst, 1, gop_cfg);

	ow_cfg.fmt = 0;
	osd_size.w = 300;
	osd_size.h = 300;
	ow_cfg.img_size = osd_size;

	ow_cfg.crop_pixels = 0;
	osd_size.w -= ow_cfg.crop_pixels;
	int mem_width = osd_size.w * 4;
	ow_cfg.pitch = ALIGN(mem_width, 16);
	ow_cfg.mem_size.w = ALIGN(mem_width, 16);
	ow_cfg.mem_size.h = ow_cfg.img_size.h;
	ow_cfg.start.x = 2000;
	ow_cfg.start.y = 0;
	ow_cfg.end.x = ow_cfg.start.x + osd_size.w;
	ow_cfg.end.y = ow_cfg.start.y + osd_size.h;
	ow_cfg.addr = src_addr_scl[1][G_IDX];
	sclr_gop_ow_set_cfg(inst, 0, 0, &ow_cfg);
	ow_cfg.start.x = 2300;
	ow_cfg.end.x = ow_cfg.start.x + osd_size.w;
	sclr_gop_ow_set_cfg(inst, 1, 0, &ow_cfg);

	ret |= vpss_start(inst, cfg.tile_enable, false);

	for(int i = 0; i < BORDER_VPP_MAX; i++){
		border_cfg[i].cfg.b.enable = 0;
		sclr_border_vpp_set_cfg(inst, i, border_cfg + i);
	}

	for(int i = 0; i < COVER_MAX; i++){
		cover_cfg[i].start.b.enable = 0;
		sclr_cover_set_cfg(inst, i, cover_cfg + i);
	}

	border_cfg1.cfg.b.enable = 0;
	sclr_border_set_cfg(inst, &border_cfg1);

	prm_cfg.cfg.b.enable = 0;
	sclr_pri_set_cfg(inst, &prm_cfg);
	sclr_set_cfg(inst, false, true, true, false);

	return ret;
}

int test_cover(int argc, char **argv)
{
	int ret = 0;
	struct cvi_vpss_cfg cfg;
	struct sclr_cover_cfg cover_cfg[COVER_MAX];
	enum vpss_dev inst = (enum vpss_dev)strtol(argv[1], NULL, 10);

	memset(&cfg, 0, sizeof(struct cvi_vpss_cfg));
	cfg.inst = inst;
	cfg.tile_enable = false;
	cfg.img_cfg.in = SCL_INPUT_MEM;
	cfg.img_cfg.fmt_in = SCL_FMT_RGB_PACKED;
	cfg.img_cfg.csc = SCL_CSC_NONE;
	default_src_mem(DEFAULT_W, DEFAULT_H, cfg.img_cfg.fmt_in, 0, &cfg.img_cfg.mem);

	cfg.sc_cfg.crop.x = 0;
	cfg.sc_cfg.crop.y = 0;
	cfg.sc_cfg.crop.w = DEFAULT_W;
	cfg.sc_cfg.crop.h = DEFAULT_H;
	cfg.sc_cfg.dst.w = DEFAULT_W ;
	cfg.sc_cfg.dst.h = DEFAULT_H;
	cfg.sc_cfg.fmt_out = SCL_FMT_RGB_PACKED;
	cfg.sc_cfg.out_mode = SCL_OUT_DISABLE;
	cfg.sc_cfg.csc = SCL_CSC_NONE;
	cfg.sc_cfg.flip = SCL_FLIP_NO;
	cfg.sc_cfg.algorithm = SCL_BICUBIC;
	default_dst_mem(DEFAULT_W, DEFAULT_H, cfg.sc_cfg.fmt_out, 0, &cfg.sc_cfg.mem, 1);
	ret = basic(&cfg);

	for(int i = 0; i < COVER_MAX; i++){
		cover_cfg[i].start.b.enable = 1;
		cover_cfg[i].start.b.x = 200 + i * 50;
		cover_cfg[i].start.b.y = 100 + i * 50;
		cover_cfg[i].img_size.w = 100;
		cover_cfg[i].img_size.h = 200;
		cover_cfg[i].color.b.cover_color_r = 255;
		cover_cfg[i].color.b.cover_color_g = 0;
		cover_cfg[i].color.b.cover_color_b = 0;
		sclr_cover_set_cfg(inst, i, &cover_cfg[i]);
	}

	ret |= vpss_start(inst, cfg.tile_enable, false);

	for(int i = 0; i < COVER_MAX; i++){
		cover_cfg[i].start.b.enable = 0;
		sclr_cover_set_cfg(inst, i, cover_cfg + i);
	}

	return ret;
}

int do_show_addr(int argc, char **argv)
{
	int i = 0;

	for(i = 0 ; i < 4 ; ++i) {
		uartlog("sclr src buf[%d] -- 0x%lx, 0x%lx, 0x%lx\n", i, src_addr_scl[i][0], src_addr_scl[i][1], src_addr_scl[i][2]);
		uartlog("sclr dst buf[%d] -- 0x%lx, 0x%lx, 0x%lx\n", i, dst_addr_scl[i][0], dst_addr_scl[i][1], dst_addr_scl[i][2]);
	}
	return 0;
}

//one in, multiple out
int vpss_start_all(u8 dev_num, enum vpss_dev vpss_start_inst, bool tile_enable)
{
	int i, ret = 0;
	u32 vpss_mask = 0;
	u8 vpss_num = dev_num;
	u8 start_inst = 0, dev_max = 2;
	if(vpss_start_inst < 4)
		dev_max = 4;
	else if(vpss_start_inst < 6)
		start_inst = 4;
	else if(vpss_start_inst < 8)
		start_inst = 6;
	else
		start_inst = 8;
	u8 vpss_inst = ((vpss_start_inst + dev_num - 1) % dev_max) + start_inst;
	do {
		if (vpss_inst >= VPSS_MAX) {
			uartlog("vpss inst err.\n");
			return -1;
		}
		if (tile_enable)
			sclr_left_tile(vpss_inst);
		sclr_top_set_cfg(vpss_inst, true);
		vpss_mask |= BIT(vpss_inst);

		//sclr_trig_start(vpss_inst);
		vpss_inst--;
		vpss_inst = vpss_inst % dev_max + start_inst;
	} while(--vpss_num);

	vpss_inst = ((vpss_start_inst + dev_num - 1) % dev_max) + start_inst;
	vpss_num = dev_num;
	do {
		if(vpss_inst == vpss_start_inst){
			uartlog("vpss(%d) start\n", vpss_inst);
			sclr_trig_start(vpss_inst);
		} else {
			sclr_multi_start(vpss_inst);
			//uartlog("vpss(%d) multi_start\n", vpss_inst);
		}
		vpss_inst--;
		vpss_inst = vpss_inst % dev_max + start_inst;
	} while(--vpss_num);

	vpss_enable_mask = vpss_mask;
	//uartlog("vpss_enable_mask=%d\n", vpss_enable_mask);

	if (sclr_check_irq_handler(WAIT_TIME) != 0) {
		uartlog("sclr end intr doesn't happens after %d tick\n", WAIT_TIME);
		sclr_img_reset(vpss_start_inst);
		vpss_enable_mask = 0;
		fail_trigger();
		ret = -1;
	} else {
			// uartlog("sclr done\n");
			// uartlog("sc(%d) odma status(%#x)\n", vpss_start_inst,
			// 	sclr_odma_get_dbg_status(vpss_start_inst).raw);
			for (int i = 0; i < dev_num; ++i) {
				if (irq_arrival_time[i] == 0) continue;
				double frame_time_ms = (double)((double)irq_arrival_time[i] - (double)sclr_timer_tick) / ((double)25000000 / ((double)1000 * (double)1000));
				double fps = 1000.0 * 1000.0 / frame_time_ms;
				uartlog(" ****** vpss(%d) irq, time-consumption: tick(%d) time(%lf) us, Frame rate: %lf FPS ****** \n",
					i, (irq_arrival_time[i] - sclr_timer_tick), frame_time_ms, fps);
			}
			if (tile_enable){
				vpss_inst = vpss_start_inst;
				for(i = 0; i < dev_num; i++){
					sclr_right_tile(vpss_inst);
					vpss_inst++;
					vpss_inst = vpss_inst % dev_max + start_inst;
				}
				vpss_enable_mask = vpss_mask;
				sclr_trig_start(vpss_start_inst);
				mdelay(20);
				frame_done_trigger();
			} else {
				frame_done_trigger();
			}
	}
	for(i = 1; i < dev_num; i++){
		vpss_inst = (vpss_start_inst + i) % dev_max + start_inst;
		sclr_multi_init(vpss_inst);
	}
	return ret;
}

int vpss_vgop_perform(int argc, char **argv)
{
	int ret = 0;
	int dev_num;
	int in_w, in_h, out_w, out_h;
	enum vpss_dev vpss_start_inst;
	enum vpss_dev vpss_inst;
	enum sclr_input in;
	enum sclr_format src_fmt = 0;
	enum sclr_format dst_fmt = 0;

	struct sclr_gop_cfg *gop_cfg;
	struct sclr_gop_ow_cfg ow_cfg;
	struct sclr_size osd_size;
	memset(&ow_cfg, 0, sizeof(struct sclr_gop_ow_cfg));
	memset(&osd_size, 0, sizeof(struct sclr_size));
	u8 layers = 1;
	u8 ow_inst =0;
	u16 mem_width;

	int input_size = 3840;
	int output_size = 3840;

	dev_num = SCL_MAX_INST;
	vpss_inst = vpss_start_inst = 0;
	if (input_size == 3840) {
		in_w = 3840;
		in_h = 2160;
	} else {
		in_w = 1920;
		in_h = 1080;
	}

	struct cvi_vpss_cfg cfg[dev_num];
	memset(cfg, 0, sizeof(struct cvi_vpss_cfg) * dev_num);
	bool isShare = true;
	uartlog("in_w: %d, in_h: %d\n", in_w, in_h);
	uartlog("out_w out_h:\n");
	for(int i = 0; i < dev_num; i++){
		if(vpss_inst == vpss_start_inst)
			in = SCL_INPUT_MEM;
		else {
			in = SCL_INPUT_SHARE;
			src_fmt = SCL_FMT_RGB_PLANAR;
		}
		if (vpss_inst == 0) {
			if (output_size == 3840) {
				out_w = 3840;
				out_h = 2160;
			} else {
				out_w = 1920;
				out_h = 1080;
			}
		} else {
			out_w = 1920;
			out_h = 1080;
		}

		uartlog("%d %d\n",out_w, out_h);
		cfg[i].inst = vpss_inst;
		cfg[i].tile_enable = false;
		cfg[i].img_cfg.in = in;
		cfg[i].img_cfg.fmt_in = src_fmt;
		cfg[i].img_cfg.csc = IS_YUV_FMT(src_fmt) ? SCL_CSC_601_LIMIT_YUV2RGB : SCL_CSC_NONE;
		default_src_mem(in_w, in_h, cfg[i].img_cfg.fmt_in, i, &cfg[i].img_cfg.mem);

		cfg[i].sc_cfg.crop.x = 0;
		cfg[i].sc_cfg.crop.y = 0;
		cfg[i].sc_cfg.crop.w = in_w;
		cfg[i].sc_cfg.crop.h = in_h;
		cfg[i].sc_cfg.dst.w = out_w;
		cfg[i].sc_cfg.dst.h = out_h;
		cfg[i].sc_cfg.fmt_out = dst_fmt;
		cfg[i].sc_cfg.out_mode = IS_YUV_FMT(dst_fmt) ? SCL_OUT_CSC: SCL_OUT_DISABLE;
		cfg[i].sc_cfg.csc = IS_YUV_FMT(dst_fmt) ? SCL_CSC_601_LIMIT_RGB2YUV : SCL_CSC_NONE;
		cfg[i].sc_cfg.flip = SCL_FLIP_NO;
		cfg[i].sc_cfg.algorithm = SCL_BICUBIC;
		default_dst_mem(cfg[i].sc_cfg.dst.w, cfg[i].sc_cfg.dst.h, cfg[i].sc_cfg.fmt_out, i, &cfg[i].sc_cfg.mem, 1);

		// // vgop
		// if (i > 2)
		// 	layers = 2;
		// else
		layers = 1;

		for (int layer = 0; layer < layers; layer++) {
			gop_cfg = sclr_gop_get_cfg(cfg[i].inst, layer);
			gop_cfg->gop_ctrl.b.burst = 15;
			gop_cfg->gop_ctrl.b.ow0_en = 1;
			sclr_gop_set_cfg(cfg[i].inst, layer, gop_cfg);

			ow_cfg.fmt = 0;
			osd_size.w = out_w/2;
			osd_size.h = out_h;
			ow_cfg.img_size = osd_size;
			ow_cfg.mem_size.h = osd_size.h;

			ow_cfg.crop_pixels = 0;
			osd_size.w -= ow_cfg.crop_pixels;
			mem_width = osd_size.w * 4;
			ow_cfg.pitch = ALIGN(mem_width, 16);
			ow_cfg.mem_size.w = ALIGN(mem_width, 16);
			ow_cfg.start.x = 0;
			ow_cfg.start.y = 0;
			ow_cfg.end.x = ow_cfg.start.x + osd_size.w;
			ow_cfg.end.y = ow_cfg.start.y + osd_size.h;
			if(i == 0) {
				ow_cfg.addr = gop_addr + GOP_FRAME_OFFSET * (0) * (0) + gop_space[0];
			} else {
				ow_cfg.addr = gop_addr + GOP_FRAME_OFFSET * 6 + gop_space[0]; //0x90c0 0000
			}

			sclr_gop_ow_set_cfg(cfg[i].inst, layer, ow_inst, &ow_cfg);
		}
		//
		if (cfg[i].sc_cfg.out_mode == SCL_OUT_CSC) {
			enum sclr_csc csc = SCL_CSC_NONE;
			if (IS_YUV_FMT(cfg[i].sc_cfg.fmt_out))
				csc = SCL_CSC_601_LIMIT_RGB2YUV;
			cfg[i].sc_cfg.csc = csc;
		}
		ret = basic(&cfg[i]);

		if (ret) {
			uartlog("vpss(%d) basic failed\n", vpss_inst);
			return ret;
		}

		//top
		if (i == (dev_num - 1))
			isShare = false;
		sclr_top_set_src_share(vpss_inst, isShare);
		vpss_inst++;
		vpss_inst = (vpss_inst == 4) ? 0 : vpss_inst;
	}

	ret = vpss_start_all(dev_num, vpss_start_inst, false);
	return ret;
}

int vpss_1_to2(int argc, char **argv)
{
	int ret = 0;
	int dev_num;
	enum vpss_dev vpss_start_inst;
	enum vpss_dev vpss_inst;
	enum sclr_input in;
	enum sclr_format src_fmt = SCL_FMT_RGB_PACKED;
	enum sclr_format dst_fmt = SCL_FMT_RGB_PACKED;
	dev_num = strtol(argv[1], NULL, 10);
	struct cvi_vpss_cfg cfg[dev_num];
	memset(cfg, 0, sizeof(struct cvi_vpss_cfg) * dev_num);
	bool isShare = true;

	vpss_inst = vpss_start_inst = strtol(argv[2], NULL, 10);

	for(int i = 0; i < dev_num; i++){
		if(vpss_inst == vpss_start_inst)
			in = SCL_INPUT_MEM;
		else {
			in = SCL_INPUT_SHARE;
			src_fmt = SCL_FMT_RGB_PLANAR;
		}

		cfg[i].inst = vpss_inst;
		cfg[i].tile_enable = false;
		cfg[i].img_cfg.in = in;
		cfg[i].img_cfg.fmt_in = src_fmt;
		cfg[i].img_cfg.csc = SCL_CSC_NONE;
		default_src_mem(1920, 1080, cfg[i].img_cfg.fmt_in, i, &cfg[i].img_cfg.mem);

		cfg[i].sc_cfg.crop.x = 0;
		cfg[i].sc_cfg.crop.y = 0;
		cfg[i].sc_cfg.crop.w = ((i == 0) ? (1920 / 2) : 1920);
		cfg[i].sc_cfg.crop.h = ((i == 0) ? (1080 / 2) : 1080);
		cfg[i].sc_cfg.dst.w = cfg[i].sc_cfg.crop.w;
		cfg[i].sc_cfg.dst.h = cfg[i].sc_cfg.crop.h;
		cfg[i].sc_cfg.fmt_out = dst_fmt;
		cfg[i].sc_cfg.out_mode = SCL_OUT_DISABLE;
		cfg[i].sc_cfg.csc = SCL_CSC_NONE;
		cfg[i].sc_cfg.flip = SCL_FLIP_NO;
		cfg[i].sc_cfg.algorithm = SCL_BICUBIC;
		default_dst_mem(cfg[i].sc_cfg.dst.w, cfg[i].sc_cfg.dst.h, cfg[i].sc_cfg.fmt_out, i, &cfg[i].sc_cfg.mem, 1);

		if (cfg[i].sc_cfg.out_mode == SCL_OUT_CSC) {
			enum sclr_csc csc = SCL_CSC_NONE;
			if (IS_YUV_FMT(cfg[i].sc_cfg.fmt_out))
				csc = SCL_CSC_601_LIMIT_RGB2YUV;
			cfg[i].sc_cfg.csc = csc;
		}
		ret = basic(&cfg[i]);
		if (ret) {
			uartlog("vpss(%d) basic failed\n", vpss_inst);
			return ret;
		}

		//top
		if (i == (dev_num - 1))
			isShare = false;
		sclr_top_set_src_share(vpss_inst, isShare);
		vpss_inst++;
		vpss_inst = (vpss_inst == 4) ? 0 : vpss_inst;
	}

	ret = vpss_start_all(dev_num, vpss_start_inst, false);
	return ret;
}

static struct cmd_entry test_cmd_list[] __attribute__ ((unused)) = {
	{"\n[---SCLR------------------------------------------------]", do_show_addr, 0, ""},
	{"show_addr_s", 		do_show_addr,		  0, "[Show sclr's buffer address]"},
	{"cfg", 			do_cfg, 	          0, "[Modify settings]"},
	{"vpss_basic", 			test_vpss_basic,          1, "[param:inst, basic test of vpss]"},
	{"max_size", 			test_max_size,		  3, "[param:inst, in_fmt, out_fmt]"},
	{"cover",			test_cover,		  1, "[param:inst]"},
	{"gop",				test_gop,	  	  8, "[param:inst, fmt_in, w, h, stx, sty, gop_w, gop_h]"},
	{"all",				test_open_all,	  	  1, "[param:inst]"},
	{"vpss_perform", 		vpss_vgop_perform,	  0, "[param:dev_num, vpss start inst]"},
	{"vpss_1_to2", 			vpss_1_to2,	  	  2, "[param:dev_num, vpss start inst]"},
	{NULL, NULL, 0, NULL}
};

int testcase_vgop(void)
{
	int i = 0, ret;

	union sclr_intr mask;
	u32 irq_num[SCL_MAX_INST] = {SC_INTR_0, SC_INTR_1};
	sclr_mark = 0;
	uartlog("testcase_sclr, enter\n");
	mask.raw = 0xffffffff;

	for (i = 0; i < SCL_MAX_INST; i++) {
		if (vpss_status[i]) {
			uartlog("vpss(%d) init\n", i);
			sclr_ctrl_init(i);
			sclr_intr_ctrl(i, mask);
			request_irq(irq_num[i], irq_handler_scl, 0, "scl int", NULL);
			uartlog("%s:vpss%d request_irq(%d)\n", __func__, i, irq_num[i]);
		}
	}

	is_init = true;
	uartlog("testcase_sclr: add command...\n");

	for (i = 0; i < ARRAY_SIZE(test_cmd_list) - 1; i++) {
		command_add(&test_cmd_list[i]);
	}

	uartlog("testcase_sclr: add command done\n");
    do {
        ret = cli_simple_loop();
    } while (ret == 0);

	return 0;
}

