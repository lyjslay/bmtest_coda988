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
#include "cache.h"

#define EPSILON_SCALE	 1000000 // 1/0.000001
#define SINGLE_PACKED

#define VIVO_INT_VPSS0		      9
#define VIVO_INT_VPSS1		      10

struct cvi_img_cfg {
	enum sclr_input in;   // 0(ISP), 1(dwa), 2(share), 3(DRAM)
	enum sclr_format fmt_in;
	enum sclr_csc csc;
	struct sclr_mem mem;
	bool is_skip_en;
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
	CFG_OP_IMG_CFG = 0,
	CFG_OP_IMG_SET_MEM,
	CFG_OP_IMG_START,
	CFG_OP_TOP_SET_BLEND,
	CFG_OP_IMG_CHECKSUM,
	CFG_OP_SC_CFG = 10,
	CFG_OP_SC_SET_INPUT,
	CFG_OP_SC_SET_CROP,
	CFG_OP_SC_SET_OUTPUT,
	CFG_OP_SC_SET_BORDER,
	CFG_OP_SC_SET_PRIVACY,
	CFG_OP_SC_SET_CHECKSUM,
	CFG_OP_SC_SET_COMPR,
	CFG_OP_SC_SET_COVER,
	CFG_OP_ODMA_CFG = 20,
	CFG_OP_ODMA_SET_MEM,
	CFG_OP_GOP_CFG = 30,
	CFG_OP_GOP_OW_CFG,
	CFG_OP_GOP_LUT_CFG,
	CFG_OP_GOP_SCL,
	CFG_OP_GOP_DECOMPR,
	CFG_OP_STATUS_ALL = 60,
	CFG_OP_STATUS_TOP,
	CFG_OP_STATUS_IMG_IN,
	CFG_OP_STATUS_SC,
	CFG_OP_STATUS_GOP,
	CFG_OP_STATUS_ODMA,
	CFG_OP_STATUS_DISP,
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

static char* str_sclr_fmt[] = {"YUV420", "YUV422", "RGB_PLANAR", "RGB_PACKED", "BGR_PACKED", "Y_ONLY"};
static char* str_sclr_csc[] = {"Disable", "2RGB_601_Limit", "2RGB_601_Full", "2RGB_709_Limit", "2RGB_709_Full",
		"2YUV_601_Limit", "2YUV_601_Full", "2YUV_709_Limit", "2YUV_709_Full"};
static char* str_sclr_flip[] = {"No", "HFLIP", "VFLIP", "HVFLIP"};
static char* str_sclr_odma_mode[] = {"CSC", "QUANT", "HSV", "BF16", "DISABLE"};
static char* str_sclr_gop_fmt[] = {"ARGB8888", "ARGB4444", "ARGB1555", "256LUT", "16LUT", "FONT"};
static bool is_init = false;
u64 irq_arrival_time[4] = {0,0,0,0};
bool is_irq_ctx_print_allowed = true;
#ifdef BOARD_PALLADIUM
#define DEFAULT_W 800  // 800
#define DEFAULT_H 600  // 600
// #define WAIT_TIME 5
#else
#define DEFAULT_W 800   // 800
#define DEFAULT_H 600   // 600
// #define WAIT_TIME 20000
#endif

static int WAIT_TIME = 100;
// #define WAIT_TIME 2000   // for 8192x8192

u64 src_addr_scl[5][3] = {
	{BASE_OFFSET + 0x01000000, BASE_OFFSET + 0x02000000, BASE_OFFSET + 0x03000000},
	{BASE_OFFSET + 0x04000000, BASE_OFFSET + 0x05000000, BASE_OFFSET + 0x06000000},
	{BASE_OFFSET + 0x07000000, BASE_OFFSET + 0x08000000, BASE_OFFSET + 0x09000000},  //gop(max5338x1080)
	{BASE_OFFSET + 0x0a000000, BASE_OFFSET + 0x0b000000, BASE_OFFSET + 0x0c000000},
	{BASE_OFFSET + 0x0d000000, BASE_OFFSET + 0x0e000000, BASE_OFFSET + 0x0f000000},
};

u64 dst_addr_scl[4][3] = {
	{BASE_OFFSET + 0x0d000000, BASE_OFFSET + 0x0e000000, BASE_OFFSET + 0x0f000000},
	{BASE_OFFSET + 0x10000000, BASE_OFFSET + 0x11000000, BASE_OFFSET + 0x12000000},
	{BASE_OFFSET + 0x13000000, BASE_OFFSET + 0x14000000, BASE_OFFSET + 0x15000000}, //primask
	{BASE_OFFSET + 0x16000000, BASE_OFFSET + 0x17000000, BASE_OFFSET + 0x18000000},
};

enum sclr_format g_fmt_in[] = {
	SCL_FMT_YUV420,
	SCL_FMT_YUV422,
	SCL_FMT_RGB_PLANAR,
	SCL_FMT_BGR_PACKED, // B lsb
	SCL_FMT_RGB_PACKED, // R lsb
	SCL_FMT_Y_ONLY,
	SCL_FMT_NV12,
	SCL_FMT_NV21,
	SCL_FMT_YUV422SP1,
	SCL_FMT_YUV422SP2,
	SCL_FMT_YVYU,
	SCL_FMT_YUYV,
	SCL_FMT_VYUY,
	SCL_FMT_UYVY,
	SCL_FMT_YUV444,
	SCL_FMT_YUV444_PACKED,
	SCL_FMT_MAX
};

// u64 src_addr_scl[4][3] = {
// 	{BASE_OFFSET + 0x00800001, BASE_OFFSET + 0x00c00000, BASE_OFFSET + 0x01000000},
// 	{BASE_OFFSET + 0x01400000, BASE_OFFSET + 0x01800000, BASE_OFFSET + 0x01c00000},
// 	{BASE_OFFSET + 0x02000000, BASE_OFFSET + 0x02400000, BASE_OFFSET + 0x02800000},  //gop(max5338x1080)
// 	{BASE_OFFSET + 0x02c00000, BASE_OFFSET + 0x03000000, BASE_OFFSET + 0x03400000},
// };

// u64 dst_addr_scl[4][3] = {
// 	{BASE_OFFSET + 0x03800001, BASE_OFFSET + 0x03c00000, BASE_OFFSET + 0x04000000},
// 	{BASE_OFFSET + 0x04400000, BASE_OFFSET + 0x04800000, BASE_OFFSET + 0x04c00000},
// 	{BASE_OFFSET + 0x05000000, BASE_OFFSET + 0x05400000, BASE_OFFSET + 0x05800000}, //primask
// 	{BASE_OFFSET + 0x05c00000, BASE_OFFSET + 0x06000000, BASE_OFFSET + 0x06400000},
// };

// for 8192 * 8192
// u64 dst_addr_scl[4][3] = {
// 	{BASE_OFFSET + 0x103800000, BASE_OFFSET + 0x13c00000, BASE_OFFSET + 0x14000000},
// 	{BASE_OFFSET + 0x04400000, BASE_OFFSET + 0x04800000, BASE_OFFSET + 0x04c00000},
// 	{BASE_OFFSET + 0x05000000, BASE_OFFSET + 0x05400000, BASE_OFFSET + 0x05800000}, //primask
// 	{BASE_OFFSET + 0x05c00000, BASE_OFFSET + 0x06000000, BASE_OFFSET + 0x06400000},
// };

// u64 src_addr_scl[4][3] = {
// 	{0x1000000, 0x1c00000, 0x11000000},
// 	// {BASE_OFFSET + 0x01400000, BASE_OFFSET + 0x01800000, BASE_OFFSET + 0x01c00000},
// 	// {BASE_OFFSET + 0x02000000, BASE_OFFSET + 0x02400000, BASE_OFFSET + 0x02800000},  //gop(max5338x1080)
// 	// {BASE_OFFSET + 0x02c00000, BASE_OFFSET + 0x03000000, BASE_OFFSET + 0x03400000},
// };

// u64 dst_addr_scl[4][3] = {
// 	{0x15000000, 0x15c00000, 0x16000000},
// 	// {BASE_OFFSET + 0x04400000, BASE_OFFSET + 0x04800000, BASE_OFFSET + 0x04c00000},
// 	// {BASE_OFFSET + 0x05000000, BASE_OFFSET + 0x05400000, BASE_OFFSET + 0x05800000}, //primask
// 	// {BASE_OFFSET + 0x05c00000, BASE_OFFSET + 0x06000000, BASE_OFFSET + 0x06400000},
// };

//gop_addr 0x90000000
static u64 gop_addr = BASE_OFFSET + 0x90000000;
static u64 gop_LUT_addr = BASE_OFFSET + 0x06800000;
static u32 gop_space[SCL_MAX_INST + 1] = {GOP_FRAME_OFFSET, GOP_FRAME_OFFSET, GOP_FRAME_OFFSET}; // the remaining dram for gop, count down like stack

static u64 cmdq_set_addr = BASE_OFFSET + 0x07800000;
static bool enable_cmdq = false;
u64 sclr_timer_tick = 0;
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
	uartlog("fmt(%d) num_planes(%d) pitch(%d %d)\n", sc_fmt, fmt->buffers, bytesperline[0], bytesperline[1]);
}

void sclr_trig_start(u8 inst)
{
	sclr_mark = 0;
	sclr_timer_tick = timer_get_tick();
	// timer_meter_start();
	uartlog(" ---***--->>> REG_SCL_IMG_CFG(%d) = %llx\n", inst, _reg_read(REG_SCL_IMG_CFG(inst)));
	sclr_img_start(inst);
}

int irq_handler_scl(int irqn, void *priv)
{
	u8 i;
	int inst = -1;
	irq_arrival_time[irqn-9] = timer_get_tick();
	u32 irq_num[SCL_MAX_INST] = {VIVO_INT_VPSS0, VIVO_INT_VPSS1};

	for (i = 0; i < SCL_MAX_INST; i++)
		if (irq_num[i] == irqn)
			inst = i;
	if (inst < 0) {
		uartlog("vpss irqn(%d) err.\n", irqn);
		return -1;
	}

	if (enable_intr_msg) {
		if (is_irq_ctx_print_allowed) {
			double frame_time_ms = (double)((double)irq_arrival_time[irqn-9] - (double)sclr_timer_tick) / (double)TIMER_CNT_MS;
			double fps = 1000.0 / frame_time_ms;
			uartlog(" ****** vpss(%d) irq, time-consumption: tick(%d) time(%lf) ms, Frame rate: %lf FPS ****** \n",
					inst, (irq_arrival_time[irqn-9] - sclr_timer_tick), frame_time_ms, fps);
		}
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
		if (enable_intr_msg && is_irq_ctx_print_allowed)
			uartlog("vpss(%d) status(%#x)\n", inst, intr_status.raw);

		if (!intr_status.b.scl_frame_end)
			return 0;
		if (is_irq_ctx_print_allowed)
			uartlog("vpss(%d) frame done.\n", inst);
		if (!(vpss_enable_mask & BIT(inst))) {
			uartlog("vpss%d is not enable.\n", inst);
			//return -1;
		}
		vpss_enable_mask &= ~BIT(inst);

		if (!vpss_enable_mask) {
			sclr_mark++;
		}
		if (is_irq_ctx_print_allowed)
			uartlog("sclr_mark%d vpss_enable_mask%d.\n", sclr_mark, vpss_enable_mask);

	}

	return 0;
}

int sclr_check_irq_handler(int timeout)
{
	int i = 0;

	for (i = 0; i < timeout; ++i) {
		if (sclr_mark > 0) {
			sclr_mark = 0;
			uartlog("%s: check tick:%d\n", __func__, i);
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

	switch(op) {
	case CFG_OP_IMG_CFG:
	{
		u8 inst = va_arg(ap, int);
		enum sclr_input in;
		enum sclr_format fmt;
		enum sclr_csc csc = SCL_CSC_NONE;

		bm_get_num("Src input(0:ISP, 1:DWA, 2:share 3:MEM): ", in);

		uartlog("Src format(0:420, 1:422, 2:rgb, 3:rgb_pack, 4:bgr_pack, 5:y_only\n");
		bm_get_num("8:nv12, 9:nv21, 10:422sp1(UV), 11:422sp2(VU), 12:YVYU, 13:YUYV, 14:VYUY, 15:UYVY): ", fmt);
		if (IS_YUV_FMT(fmt)) {
			bm_get_num("CSC(0:601_Limit, 1:601_Full, 2:709_Limit, 3:709_full): ", csc);
			++csc;
		}

		sclr_ctrl_set_input(inst, in, fmt, csc);
		break;
	}

	case CFG_OP_IMG_SET_MEM:
	{
		u8 inst = va_arg(ap, int);
		struct sclr_img_cfg *cfg = sclr_img_get_cfg(inst);
		struct sclr_mem mem;
		struct sclr_size size;
		u8 src_idx = 0;
		u64 addr0, addr1, addr2;
		u16 pitch[3];

		u8 num_args = va_arg(ap, int);
		if (num_args == 0) {
			bm_get_num("width: ", size.w);
			bm_get_num("height: ", size.h);
		} else {
			size.w = va_arg(ap, int);
			size.h = va_arg(ap, int);
		}

		memset(&mem, 0, sizeof(mem));
		// need to set size if online
		if (cfg->src != SCL_INPUT_MEM) {
			mem.width = size.w;
			mem.height = size.h;
			sclr_img_set_mem(inst, &mem);
			break;
		}

		if (num_args <= 2) {
			bm_get_num("src-mem-idx(0~2): ", src_idx);
			uartlog("Please make sure raw RGB/YUV planar data has been loaded into following 3 address:0x%lx, 0x%lx, 0x%lx\n", src_addr_scl[src_idx][R_IDX], src_addr_scl[src_idx][G_IDX], src_addr_scl[src_idx][B_IDX]);
		} else if (num_args == 3) {
			src_idx = va_arg(ap, int);
		} else if (num_args == 5) {
			addr0 = va_arg(ap, u64);
			addr1 = va_arg(ap, u64);
			addr2 = va_arg(ap, u64);
		}
		if (num_args >= 2)
			uartlog("size(%d %d)\n", size.w, size.h);
		if (num_args == 3)
			uartlog("mem src-idx(%d)\n", src_idx);
		if (num_args == 5)
			uartlog("mem addr(%#x %#x %#x)\n", addr0, addr1, addr2);

		mem.width = size.w;
		mem.height = size.h;
		mem.start_x = 0;
		mem.start_y = 0;
		if (num_args <= 3) {
			mem.addr0 = src_addr_scl[src_idx][R_IDX];
			mem.addr1 = src_addr_scl[src_idx][G_IDX];
			mem.addr2 = src_addr_scl[src_idx][B_IDX];
		} else {
			mem.addr0 = addr0;
			mem.addr1 = addr1;
			mem.addr2 = addr2;
		}

		_get_bytesperline(cfg->fmt, size.w, pitch);
		mem.pitch_y = pitch[0];
		mem.pitch_c = pitch[1];

		sclr_img_set_mem(inst, &mem);
		break;
	}

	case CFG_OP_IMG_START:
	{
		u8 inst = va_arg(ap, int);
		sclr_trig_start(inst);
		break;
	}

	case CFG_OP_IMG_CHECKSUM:
	{
		u8 inst = va_arg(ap, int);
		bool checksum_en = false;
		bm_get_num("checksum enable, y(1)/n(0): ", checksum_en);
		sclr_img_checksum_en(inst, checksum_en);
		break;
	}

	case CFG_OP_SC_CFG:
	{
		u8 inst = va_arg(ap, int);
		bool sc_bypass = false;
		bool gop_bypass = false;
		bool cir_bypass = false;
		bool odma_bypass = false;
		bm_get_num("sc bypass, y(1)/n(0)): ", sc_bypass);
		bm_get_num("gop bypass, y(1)/n(0)): ", gop_bypass);
		bm_get_num("cir bypass, y(1)/n(0)): ", cir_bypass);
		bm_get_num("odma bypass, y(1)/n(0)): ", odma_bypass);
		sclr_set_cfg(inst, sc_bypass, gop_bypass, cir_bypass, odma_bypass);
		break;
	}

	case CFG_OP_SC_SET_INPUT:
	{
		u8 inst = va_arg(ap, int);
		struct sclr_size src;
		bm_get_num("Src width(pixel num): ", src.w);
		bm_get_num("Src height(line num): ", src.h);

		sclr_set_input_size(inst, src);
		break;
	}

	case CFG_OP_SC_SET_CROP:
	{
		u8 inst = va_arg(ap, int);
		struct sclr_rect crop;
		bm_get_num("Crop pos-x: ", crop.x);
		bm_get_num("Crop pos-y: ", crop.y);
		bm_get_num("Crop width(pixel num): ", crop.w);
		bm_get_num("Crop height(line num): ", crop.h);

		sclr_set_crop(inst, crop, true);
		sclr_set_scale(inst);
		break;
	}

	case CFG_OP_SC_SET_OUTPUT:
	{
		u8 inst = va_arg(ap, int);
		struct sclr_size dst;
		bm_get_num("Dst width(pixel num): ", dst.w);
		bm_get_num("Dst height(line num): ", dst.h);

		sclr_set_output_size(inst, dst);
		sclr_set_scale(inst);
		break;
	}

	case CFG_OP_SC_SET_BORDER:
	{
		u8 inst = va_arg(ap, int);
		struct sclr_border_cfg *cfg;

		cfg = sclr_border_get_cfg(inst);
		bm_get_num("border enable, y(1)/n(0): ", cfg->cfg.b.enable);
		if (cfg->cfg.b.enable) {
			bm_get_num("R(0~255): ", cfg->cfg.b.bd_color_r);
			bm_get_num("G(0~255): ", cfg->cfg.b.bd_color_g);
			bm_get_num("B(0~255): ", cfg->cfg.b.bd_color_b);
			bm_get_num("offset_x: ", cfg->start.x);
			bm_get_num("offset_y: ", cfg->start.y);
		}

		sclr_border_set_cfg(inst, cfg);
		break;
	}

	case CFG_OP_SC_SET_PRIVACY:
	{
		u8 inst = va_arg(ap, int);
		struct sclr_privacy_cfg cfg;

		cfg.cfg.raw = 0;
		bm_get_num("privacy enable, y(1)/n(0): ", cfg.cfg.b.enable);
		if (cfg.cfg.b.enable) {
			bm_get_num("mode, 0(grid)/1(pixel): ", cfg.cfg.b.mode);
			if (cfg.cfg.b.mode == 0) {
				bm_get_num("grid_size, 0(8x8)/1(16x16): ", cfg.cfg.b.grid_size);
			}
			bm_get_num("Full image, y(1)/n(0): ", cfg.cfg.b.fit_picture);
			bm_get_num("force alpha, y(1)/n(0): ", cfg.cfg.b.force_alpha);
			bm_get_num("alpha factor, (0 ~ 256): ", cfg.map_cfg.alpha_factor);
			bm_get_num("mask data, rgb332(1)/Y(0): ", cfg.cfg.b.mask_rgb332);
			bm_get_num("no mask idx, (0~255): ", cfg.map_cfg.no_mask_idx);
			cfg.map_cfg.base = dst_addr_scl[2][0];
			cfg.map_cfg.axi_burst = 7;
			uartlog("fill map(%#x) with the color you want before continue\n", cfg.map_cfg.base);
			uartlog("rgb332 red(0xe0) green(0x1c) blue(0x03)\n");

			if (!cfg.cfg.b.mask_rgb332) {
				cfg.cfg.b.blend_y = 1;
				cfg.cfg.b.y2r_enable = 1;
			}

			if (!cfg.cfg.b.fit_picture) {
				bm_get_num("start_x: ", cfg.start.x);
				bm_get_num("end_x: ", cfg.end.x);
				bm_get_num("start_y: ", cfg.start.y);
				bm_get_num("end_y: ", cfg.end.y);
			}
		}

		sclr_pri_set_cfg(inst, &cfg);
		break;
	}

	case CFG_OP_SC_SET_CHECKSUM:
	{
		u8 inst = va_arg(ap, int);
		bool checksum_en = false;
		bm_get_num("checksum enable, y(1)/n(0): ", checksum_en);
		sclr_core_checksum_en(inst, checksum_en);
		break;
	}

	case CFG_OP_SC_SET_COVER:
	{
		u8 inst = va_arg(ap, int);
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

	case CFG_OP_ODMA_CFG:
	{
		u8 inst = va_arg(ap, int);
		struct sclr_odma_cfg *cfg;

		cfg = sclr_odma_get_cfg(inst);

		bm_get_num(" flip(0:no, 1:hflip, 2:vflip, 3:hvflip): ", cfg->flip);
		bm_get_num(" mode(0:csc, 1:quant, 2:hsv, 3:bf16, 4:disable): ", cfg->csc_cfg.mode);
		uartlog(" format(0:420, 1:422, 2:rgb, 3:rgb_pack, 4:bgr_pack, 5:y_only\n");
		bm_get_num(" 8:nv12, 9:nv21, 10:422sp1(UV), 11:422sp2(VU), 12:YVYU, 13:YUYV, 14:VYUY, 15:UYVY): ", cfg->fmt);
		if ((cfg->csc_cfg.mode == SCL_OUT_CSC && IS_YUV_FMT(cfg->fmt)) ||
				(cfg->csc_cfg.mode != SCL_OUT_CSC && !IS_YUV_FMT(cfg->fmt))
		   ) {
			// use yuv format if output-mode is csc, o/w should go rgb format.
		} else {
			uartlog("Invalid parameter, fmt(%d) should be rgb if mode(%d) is not csc.\n", cfg->fmt, cfg->csc_cfg.mode);
			break;
		}

		sclr_odma_set_cfg(inst, cfg);
		if (cfg->csc_cfg.mode == SCL_OUT_CSC) {
			enum sclr_csc csc = SCL_CSC_NONE;
			if (IS_YUV_FMT(cfg->fmt)) {
				bm_get_num("CSC(0:601_Limit, 1:601_Full, 2:709_Limit, 3:709_full): ", csc);
				csc += SCL_CSC_601_LIMIT_RGB2YUV;
			}

			cfg->csc_cfg.csc_type = csc;
		} else if (cfg->csc_cfg.mode == SCL_OUT_QUANT) {
			float val = 0.0;
			uartlog("Please give sc-factor(<0) for R: ");
			scanf("%f", &val);
			cfg->csc_cfg.quant_form.sc_frac[0] = val * 8192;
			uartlog("Please give sc-factor(<0) for G: ");
			scanf("%f", &val);
			cfg->csc_cfg.quant_form.sc_frac[1] = val * 8192;
			uartlog("Please give sc-factor(<0) for B: ");
			scanf("%f", &val);
			cfg->csc_cfg.quant_form.sc_frac[2] = val * 8192;
			uartlog("Please give mean for R: ");
			scanf("%f", &val);
			cfg->csc_cfg.quant_form.sub[0] = (u8)val;
			cfg->csc_cfg.quant_form.sub_frac[0] = (val - cfg->csc_cfg.quant_form.sub[0]) * 1024;
			uartlog("Please give mean for G: ");
			scanf("%f", &val);
			cfg->csc_cfg.quant_form.sub[1] = (u8)val;
			cfg->csc_cfg.quant_form.sub_frac[1] = (val - cfg->csc_cfg.quant_form.sub[1]) * 1024;
			uartlog("Please give mean for B: ");
			scanf("%f", &val);
			cfg->csc_cfg.quant_form.sub[2] = (u8)val;
			cfg->csc_cfg.quant_form.sub_frac[2] = (val - cfg->csc_cfg.quant_form.sub[2]) * 1024;
			//uartlog("factor:		  %d %d %d\n", q_cfg.sc[0], q_cfg.sc[1], q_cfg.sc[2]);
			//uartlog("mean-sub:	  %d %d %d\n", q_cfg.sub[0], q_cfg.sub[1], q_cfg.sub[2]);
			//uartlog("mean-sub-frac: %d %d %d\n", q_cfg.sub_frac[0], q_cfg.sub_frac[1], q_cfg.sub_frac[2]);

			bm_get_num("rounding(0:to_even, 1:away_from_zero, 2:truncate): ", cfg->csc_cfg.quant_round);
		}
		sclr_ctrl_set_output(inst, &cfg->csc_cfg, cfg->fmt);
		break;
	}

	case CFG_OP_ODMA_SET_MEM:
	{
		u8 inst = va_arg(ap, int);
		struct sclr_odma_cfg *cfg = sclr_odma_get_cfg(inst);
		struct sclr_mem mem;
		struct sclr_size size;
		u8 dst_idx = 0;
		u64 addr0, addr1, addr2;
		u16 pitch[3];

		u8 num_args = va_arg(ap, int);
		if (num_args == 0) {
			bm_get_num("width: ", size.w);
			bm_get_num("height: ", size.h);
		} else {
			size.w = va_arg(ap, int);
			size.h = va_arg(ap, int);
		}

		if (num_args <= 2) {
			bm_get_num("dst-mem-idx(0~2): ", dst_idx);
			uartlog("data will be loaded into following 3 address:0x%lx, 0x%lx, 0x%lx\n", dst_addr_scl[dst_idx][R_IDX], dst_addr_scl[dst_idx][G_IDX], dst_addr_scl[dst_idx][B_IDX]);
		} else if (num_args == 3) {
			dst_idx = va_arg(ap, int);
		} else if (num_args == 5) {
			addr0 = va_arg(ap, u64);
			addr1 = va_arg(ap, u64);
			addr2 = va_arg(ap, u64);
		}
		if (num_args >= 2)
			uartlog("size(%d %d)\n", size.w, size.h);
		if (num_args == 3)
			uartlog("mem dst-idx(%d)\n", dst_idx);
		if (num_args == 5)
			uartlog("mem addr(%#x %#x %#x)\n", addr0, addr1, addr2);

		mem.width = size.w;
		mem.height = size.h;
		mem.start_x = 0;
		mem.start_y = 0;
		if (num_args <= 3) {
			mem.addr0 = dst_addr_scl[dst_idx][R_IDX];
			mem.addr1 = dst_addr_scl[dst_idx][G_IDX];
			mem.addr2 = dst_addr_scl[dst_idx][B_IDX];
		} else {
			mem.addr0 = addr0;
			mem.addr1 = addr1;
			mem.addr2 = addr2;
		}

		_get_bytesperline(cfg->fmt, size.w, pitch);
		mem.pitch_y = pitch[0];
		mem.pitch_c = pitch[1];

		sclr_odma_set_mem(inst, &mem);

		break;
	}

	case CFG_OP_GOP_CFG:
	{
		u8 inst = va_arg(ap, int);
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
				bm_get_num("gop burst len(0~15): ", gop_cfg->gop_ctrl.b.burst);

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

	case CFG_OP_GOP_OW_CFG:
	{
		u8 inst = va_arg(ap, int);
		u8 layer;
		u8 ow_inst;
		u16 mem_width;
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
				mem_width = osd_size.w >> 1;
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

		uartlog("OSD frame end position: (%d*%d)\n", ow_cfg.end.x, ow_cfg.end.y);

		// count down from space's end
		gop_space[inst] -= (ow_cfg.pitch * ow_cfg.mem_size.h);

		// inst and ow_inst begin from 0
		ow_cfg.addr = gop_addr + GOP_FRAME_OFFSET * (inst + 1) * (ow_inst + 1) + gop_space[inst];
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
			}
			sclr_gop_fb_set_cfg(inst, layer, fb_inst, &fb_cfg);
		}
		break;
	}

	case CFG_OP_GOP_LUT_CFG:
	{
		u8 inst = va_arg(ap, int);
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

	case CFG_OP_GOP_SCL:
	{
		u8 inst = va_arg(ap, int);
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

	case CFG_OP_GOP_DECOMPR:
	{
		u8 inst = va_arg(ap, int);
		struct sclr_gop_odec_cfg odec_cfg;
		memset(&odec_cfg, 0, sizeof(odec_cfg));
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
#if 0
		// Get global g_gop_cfg[inst][layer] or  &g_disp_cfg.gop_cfg
		struct sclr_gop_cfg *gop_cfg = sclr_gop_get_cfg(inst, layer);

		bm_get_num("OSD de-compression enable, y(1)/n(0): ", gop_cfg->odec_cfg.odec_ctrl.b.odec_en);
		odec_cfg.odec_ctrl.b.odec_en = gop_cfg->odec_cfg.odec_ctrl.b.odec_en;
		if (tmp) {
			bm_get_num("Only one OW can be set to de-compression. Which OW?(0~7): ",
					   gop_cfg->odec_cfg.odec_ctrl.b.odec_attached_idx);
			odec_cfg.odec_ctrl.b.odec_attached_idx = gop_cfg->odec_cfg.odec_ctrl.b.odec_attached_idx;
			odec_cfg.odec_ctrl.b.odec_int_en = 1;
			sclr_gop_odec_set_cfg_from_oenc(inst, layer, &odec_cfg);
		}
#endif
		break;
	}

	case CFG_OP_STATUS_IMG_IN:
	{
		for(int i = 0; i < SCL_MAX_INST; ++i) {
			struct sclr_img_cfg *cfg = sclr_img_get_cfg(i);
			union sclr_img_dbg_status status = sclr_img_get_dbg_status(i, true);
			uartlog("-------------------------IMG%d(%s)---------------------------\n", i, (i == 0) ? "V":"D");
			uartlog("src(%d)\t\t\tcsc(%15s)\tfmt(%s)\t\tburst(%d)\n", cfg->src, str_sclr_csc[cfg->csc], str_sclr_fmt[cfg->fmt], cfg->burst);
			_show_sclr_mem(&cfg->mem);
			uartlog("err_fwr_yuv(%d%d%d)\terr_erd_yuv(%d%d%d)\tlb_full_yuv(%d%d%d)\tlb_empty_yuv(%d%d%d)\n", status.b.err_fwr_y, status.b.err_fwr_u,  status.b.err_fwr_v, status.b.err_erd_y, status.b.err_erd_u, status.b.err_erd_v, status.b.lb_full_y, status.b.lb_full_u, status.b.lb_full_v, status.b.lb_empty_y, status.b.lb_empty_u, status.b.lb_empty_v);
			uartlog("ip idle(%d)\t\tip int(%d)\n", status.b.ip_idle, status.b.ip_int);

			struct sclr_img_checksum_status ch_status;
			sclr_img_get_checksum_status(i, &ch_status);
			uartlog("checksum en(%d)\n", ch_status.checksum_base.b.enable);
			uartlog("checksum axi_read_data(0x%x)\n", ch_status.axi_read_data);
			uartlog("checksum output_data(0x%x)\n", ch_status.checksum_base.b.output_data);
		}
		break;
	}

	case CFG_OP_STATUS_SC:
	{
		for(int i = 0; i < SCL_MAX_INST; ++i) {
			struct sclr_core_cfg *cfg = sclr_get_cfg(i);
			uartlog("-------------------------SC%d---------------------------\n", i);
			uartlog("sc bypass(%d)\t\tgop bypass(%d)\t\tcir bypass(%d)\t\todma bypass(%d)\n", cfg->sc_bypass, cfg->gop_bypass, cfg->cir_bypass, cfg->odma_bypass);
			uartlog("src-size(%4d*%4d)\tcrop offset(%4d*%4d)\tcrop size(%4d*%4d)\toutput size(%4d*%4d)\n", cfg->sc.src.w, cfg->sc.src.h, cfg->sc.crop.x, cfg->sc.crop.y, cfg->sc.crop.w, cfg->sc.crop.h, cfg->sc.dst.w, cfg->sc.dst.h);

			struct sclr_core_checksum_status ch_status;
			sclr_core_get_checksum_status(i, &ch_status);
			uartlog("checksum en(%d)\n", ch_status.checksum_base.b.enable);
			uartlog("checksum axi_read_gop0_data(0x%x)\n", ch_status.axi_read_gop0_data);
			uartlog("checksum axi_read_gop1_data(0x%x)\n", ch_status.axi_read_gop1_data);
			uartlog("checksum axi_write_data(0x%x)\n", ch_status.axi_write_data);
			uartlog("checksum data_in(0x%x)\n", ch_status.checksum_base.b.data_in_from_img_in);
			uartlog("checksum data_out(0x%x)\n", ch_status.checksum_base.b.data_out);
		}
		break;
	}

	case CFG_OP_STATUS_GOP:
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

	case CFG_OP_STATUS_ODMA:
	{
		for(int i = 0; i < SCL_MAX_INST; ++i) {
			struct sclr_odma_cfg *cfg = sclr_odma_get_cfg(i);
			union sclr_odma_dbg_status status = sclr_odma_get_dbg_status(i);
			uartlog("-------------------------ODMA%d---------------------------\n", i);
			uartlog("flip(%s)\t\tfmt(%s)\t\tburst(%d)\n", str_sclr_flip[cfg->flip], str_sclr_fmt[cfg->fmt], cfg->burst);
			uartlog("mode(%s)\t\tcsc(%15s)\t\n", str_sclr_odma_mode[cfg->csc_cfg.mode], str_sclr_csc[cfg->csc_cfg.csc_type]);
			uartlog("full_yuv(%d%d%d)\t\tempty_yuv(%d%d%d)\t\taxi_active_yuv(%d%d%d)\taxi_active(%d)\n", status.b.y_buf_full, status.b.u_buf_full, status.b.v_buf_full, status.b.y_buf_empty, status.b.u_buf_empty, status.b.v_buf_empty, status.b.y_axi_active, status.b.u_axi_active, status.b.v_axi_active, status.b.axi_active);

			_show_sclr_mem(&cfg->mem);
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

int _cmdq_testcase(u8 inst, union cmdq_set *cmd_start, u8 op, u8 size,
		u32 pattern, u64 addr1, u64 addr2)
{
	int ret = 0;
	u8 cmd_idx = 0;

	switch (op) {
		case 0: // 0: different number of reg-write entries only
		case 1: // 1: different number of reg-write entries with sw-restart
		case 2: // 2: different number of reg-write entries with timer
			memset(cmd_start, 0, sizeof(u64) * size); // cmdset entry size: 64bit
			uartlog("cmdq addr = 0x%x, pattern = 0x%x, size = %d\n", addr1, pattern, size);
			cmd_idx = generate_cmdset(cmd_start, addr1, size, pattern, op, true);
			// flush_dcache_range(cmdq_set_addr, 0x10000);
			cmdQ_intr_ctrl(REG_SCL_CMDQ_BASE(inst), 0x07);
			enable_cmdq = true;

			sclr_timer_tick = timer_get_tick();
			cmdQ_engine(REG_SCL_CMDQ_BASE(inst), (uintptr_t)cmdq_set_addr , addr1 >> 22, false, false, cmd_idx);

#ifndef ENV_EMU
			if (sclr_check_irq_handler(5) != 0) {
				uartlog("cmdq end intr doesn't happens after 5 tick\n");
				ret = -1;
			} else {
				uartlog("cmdq done and start compare\n");
				u8 count = reg_compare(addr1, size, pattern);
				if (count != 0) {
					uartlog("test fail, compare result: mismatch(%d)\n", count);
					ret = -1;
				} else {
					uartlog("test pass\n");
				}
			}
#endif
			break;

		case 3:
		{
			struct cmdq_adma *adma = (void *)(cmdq_set_addr);
			cmdQ_intr_ctrl(REG_SCL_CMDQ_BASE(inst), 0x07);
			memset(adma, 0, (sizeof(u64) * 2) * 2); // adma table entry size: 128bit
			memset(cmd_start+0x20, 0, sizeof(u64) * size);
			cmd_idx = generate_cmdset(cmd_start+0x20, addr1, size, pattern, 0, false);
			cmdQ_adma_package(adma, (uintptr_t)(cmd_start+0x20), 8*size, false, false);
			memset(cmd_start+0x40, 0, sizeof(u64) * size);
			cmd_idx = generate_cmdset(cmd_start+0x40, addr2, size, pattern, 0, true);
			cmdQ_adma_package(adma + 1, (uintptr_t)(cmd_start + 0x40), 8*size, false, true);

			enable_cmdq = true;
			sclr_timer_tick = timer_get_tick();
			cmdQ_engine(REG_SCL_CMDQ_BASE(inst), (uintptr_t)adma , REG_SCL_TOP_BASE(inst) >> 22, false, true, 0);

#ifndef ENV_EMU
			if (sclr_check_irq_handler(30) != 0) {
				uartlog("cmdq end intr doesn't happens after 30 tick\n");
				ret = -1;
			} else {
				uartlog("cmdq done and start compare\n");
				u8 count = reg_compare(addr1, size, pattern);
				count += reg_compare(addr2, size, pattern);
				uartlog("compare result: mismatch(%d)\n", count);
				if (count != 0) ret = -1;
			}
#endif
		}
		break;

		default:
			uartlog("cmdq undefined op(%d)\n", op);
			break;
	}

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
		uartlog("00) img_in_cfg  01) img_in_mem   02) img_in_start	 03) set blend\n");
		uartlog("04) img_in_checksum  10) sc_cfg  11) sc_set_src	 12) sc_set_crop\n");
		uartlog("13) sc_set_out  14) sc_set_bd	  15) set privacy	 16) sc_checksum\n");
		uartlog("17) sc_set_osdenc	18) sc_set_cover  20) odma_cfg	 21) odma_mem\n");
		uartlog("22) gop_cfg	23) gop_ow_cfg	 24) gop_LUT_cfg	 30) gop_scale_cfg\n");
		uartlog("31) gop_osddec 32) cover  33) All status  34) top status	40) img_in status\n");
		uartlog("41) sc status	42) gop status 43) odma status	44) disp status\n");
		uartlog("45) reg dump	46) top reg    47) img_in reg	48) sc core reg 49) odma reg \n");
		uartlog("255) exit\n");
		bm_get_num(": ", op);

		u8 num_args = 0;
		if (op < CFG_OP_STATUS_ALL) {
			int inst;
			bm_get_num("vpss inst, 0~9: ", inst);
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
	uartlog("output mem addr(0x%lx, 0x%lx, 0x%lx).\n",
		mem->addr0, mem->addr1, mem->addr2);
	uartlog("output mem len(%d, %d, %d).\n",
		plane_len[0], plane_len[1], plane_len[2]);

#ifdef SINGLE_PACKED
	start_addr = (addr[0] - 0x80000000)/2;
	end_addr = addr[0];

	for (int p = 0; p < vip_fmt->buffers; p++) {
		len = (p == 0) ? (stride0 * h) : (stride1 * h);
		len = (p == 0) ? len : (len / vip_fmt->plane_sub_v);
		end_addr += len;
	}
	end_addr = (end_addr - 0x80000000)/2 - 1;
	uartlog("pld dump addr, single packed : start(0x%lx) end (0x%lx).\n", start_addr, end_addr);
	uartlog("fpga dump addr, single packed : start(0x%lx) len:0x%x (%d).\n", addr[0],
		plane_len[0] + plane_len[1] + plane_len[2], plane_len[0] + plane_len[1] + plane_len[2]);
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
	uartlog("vpss_enable_mask(%d)\n", vpss_enable_mask);
	if (sclr_check_irq_handler(WAIT_TIME) != 0) {
		uartlog("sclr end intr doesn't happens after %d tick\n", WAIT_TIME);
		// sclr_img_reset(vpss_inst);
		vpss_enable_mask = 0;
		fail_trigger();
		ret = -1;
	} else {
		uartlog("sclr done\n");
		uartlog("sc(%d) odma status(%#x)\n", vpss_inst, sclr_odma_get_dbg_status(vpss_inst).raw);
		sclr_img_get_checksum_status(vpss_inst, &ch_status);
		uartlog("checksum en(%d)\n", ch_status.checksum_base.b.enable);
		uartlog("checksum axi_read_data(0x%x)\n", ch_status.axi_read_data);
		uartlog("checksum output_data(0x%x)\n", ch_status.checksum_base.b.output_data);
		sclr_core_get_checksum_status(vpss_inst, &sc_ch_status);
		uartlog("checksum en(%d)\n", sc_ch_status.checksum_base.b.enable);
		uartlog("checksum axi_write_data(0x%x)\n", sc_ch_status.axi_write_data);
		uartlog("checksum data_in(0x%x)\n", sc_ch_status.checksum_base.b.data_in_from_img_in);
		uartlog("checksum data_out(0x%x)\n", sc_ch_status.checksum_base.b.data_out);
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
			sclr_trig_start(vpss_inst);
			if (is_irq_ctx_print_allowed)
				uartlog("vpss(%d) start\n", vpss_inst);
		} else {
			sclr_multi_start(vpss_inst);
			if (is_irq_ctx_print_allowed)
				uartlog("vpss(%d) multi_start\n", vpss_inst);
		}
		vpss_inst--;
		vpss_inst = vpss_inst % dev_max + start_inst;
	} while(--vpss_num);

	vpss_enable_mask = vpss_mask;
	if (is_irq_ctx_print_allowed)
		uartlog("vpss_enable_mask=%d\n", vpss_enable_mask);

	if (sclr_check_irq_handler(WAIT_TIME) != 0) {
		uartlog("sclr end intr doesn't happens after %d tick\n", WAIT_TIME);
		sclr_img_reset(vpss_start_inst);
		vpss_enable_mask = 0;
		fail_trigger();
		ret = -1;
	} else {
			uartlog("sclr done\n");
			uartlog("sc(%d) odma status(%#x)\n", vpss_start_inst,
				sclr_odma_get_dbg_status(vpss_start_inst).raw);
			for (int i = 0; i < 4; ++i) {
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
//one in multiple out, one of which is closed
int vpss_start_all_mask(u8 dev_num, enum vpss_dev vpss_start_inst, bool tile_enable, u8 mask_dev)
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
	int vpss_inst = ((vpss_start_inst + dev_num - 1) % dev_max) + start_inst;
	do {
		if (vpss_inst >= VPSS_MAX) {
			uartlog("vpss inst err.\n");
			return -1;
		}
		if (tile_enable)
			sclr_left_tile(vpss_inst);
		if(vpss_inst == mask_dev)
			sclr_top_set_cfg(vpss_inst, false);
		else{
			sclr_top_set_cfg(vpss_inst, true);
			vpss_mask |= BIT(vpss_inst);
		}

		//sclr_trig_start(vpss_inst);
		vpss_inst--;
		vpss_inst = vpss_inst % dev_max + start_inst;
	} while(--vpss_num);

	vpss_inst = ((vpss_start_inst + dev_num - 1) % dev_max) + start_inst;
	vpss_num = dev_num;
	do {
		if(vpss_inst == vpss_start_inst){
			sclr_trig_start(vpss_inst);
			uartlog("vpss(%d) start\n", vpss_inst);
		} else {
			sclr_multi_start(vpss_inst);
			uartlog("vpss(%d) multi_start\n", vpss_inst);
		}
		//uartlog("vpss(%d) start\n", vpss_inst);
		vpss_inst--;
		vpss_inst = vpss_inst % dev_max + start_inst;
	} while(--vpss_num);


	vpss_enable_mask = vpss_mask;
	uartlog("vpss_enable_mask=%d\n", vpss_enable_mask);

	if (sclr_check_irq_handler(WAIT_TIME) != 0) {
		uartlog("sclr end intr doesn't happens after %d tick\n", WAIT_TIME);
		sclr_img_reset(vpss_start_inst);
		vpss_enable_mask = 0;
		fail_trigger();
		ret = -1;
	} else {
			uartlog("sclr done\n");
			uartlog("sc(%d) odma status(%#x)\n", vpss_start_inst,
				sclr_odma_get_dbg_status(vpss_start_inst).raw);
			if (tile_enable){
				vpss_inst = vpss_start_inst;
				for(i = 0; i < dev_num; i++){
					sclr_right_tile(vpss_inst);
					vpss_inst++;
					vpss_inst = (vpss_inst == 4) ? 0 : vpss_inst;
				}
				vpss_enable_mask = vpss_mask;
				sclr_trig_start(vpss_start_inst);
				mdelay(20);
				frame_done_trigger();
			} else {
				frame_done_trigger();
			}
	}
	for(i = 0; i < dev_num; i++){
		vpss_inst = (vpss_start_inst + i) % dev_max + start_inst;
		sclr_multi_init(vpss_inst);
	}
	return ret;
}
//online trig(one in, multiple out)(no wait)
int vpss_start_online(void)
{
	int ret = 0;
	u32 vpss_mask = 0;
	u8 dev_num = 4;
	u8 vpss_num = dev_num;
	enum vpss_dev vpss_start_inst = 3; // isp vpss online
	int vpss_inst = vpss_start_inst + dev_num - 1;
	vpss_inst = (vpss_inst == 4) ? 0 : vpss_inst;

	do {
		if (vpss_inst >= VPSS_MAX) {
			uartlog("vpss inst err.\n");
			return -1;
		}
		sclr_top_set_cfg(vpss_inst, true);
		vpss_mask |= BIT(vpss_inst);

		//sclr_trig_start(vpss_inst);
		vpss_inst--;
		vpss_inst = (vpss_inst < 0) ? 3 : vpss_inst;
	} while(--vpss_num);

	vpss_inst = vpss_start_inst + dev_num - 1;
	vpss_inst = (vpss_inst == 4) ? 0 : vpss_inst;
	vpss_num = dev_num;
	do {
		sclr_multi_start(vpss_inst);
		uartlog("vpss(%d) multi_start\n", vpss_inst);
		if(vpss_inst == vpss_start_inst){
			sclr_trig_start(vpss_inst);
			uartlog("vpss(%d) start\n", vpss_inst);
		}
		//uartlog("vpss(%d) start\n", vpss_inst);
		vpss_inst--;
		vpss_inst = (vpss_inst < 0) ? 3 : vpss_inst;
	} while(--vpss_num);
	vpss_enable_mask = vpss_mask;
	uartlog("vpss_enable_mask=%d\n", vpss_enable_mask);
	return ret;
}
//trig wait interrupt
int vpss_online_check_irq_handler(int timeout){
	int ret = 0;
	if (sclr_check_irq_handler(timeout) != 0) {
		uartlog("sclr end intr doesn't happens\n");
		sclr_img_reset(0);
		vpss_enable_mask = 0;
		fail_trigger();
		ret = -1;
	} else {
		uartlog("sclr done\n");
		uartlog("sc(%d) odma status(%#x)\n", 0,
			sclr_odma_get_dbg_status(0).raw);
			frame_done_trigger();
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
	img.is_skip_en = cfg->img_cfg.is_skip_en;

	if (!IS_YUV_FMT(img.fmt)) {
		img.csc = SCL_CSC_NONE;
	}
	sclr_img_checksum_en(inst, 1);
	sclr_img_set_cfg(inst, &img);
	uartlog("\n----vpss(%d) img_in----\n", inst);
	uartlog("in_fmt(%d), rect(%d %d %d %d)\n", img.fmt, img.mem.start_x, img.mem.start_y, img.mem.width, img.mem.height);
	uartlog("vpss(%d) input mem addr(%lx %lx %lx).\n", inst, img.mem.addr0, img.mem.addr1, img.mem.addr2);

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
	sclr_set_cfg(inst, false, true, true, false);
	uartlog("\n--sc core--\n");
	uartlog("crop(%d %d %d %d)\n", core_cfg.sc.crop.x, core_cfg.sc.crop.y, core_cfg.sc.crop.w, core_cfg.sc.crop.h);
	uartlog("size(%d %d)\n", core_cfg.sc.dst.w, core_cfg.sc.dst.h);

	// setup odma
	uartlog("\n--odma--\n");
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

	uartlog("odma_size(%d %d)\n", odma_cfg.mem.width, odma_cfg.mem.height);
	show_dump_mem(odma_cfg.fmt, &odma_cfg.mem);

	return ret;
}

int test_vpss_basic(int argc, char **argv)
{
	int ret = 0;
	struct cvi_vpss_cfg cfg;
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
	cfg.sc_cfg.dst.w = DEFAULT_W;
	cfg.sc_cfg.dst.h = DEFAULT_H;
	cfg.sc_cfg.fmt_out = SCL_FMT_RGB_PACKED;
	cfg.sc_cfg.out_mode = SCL_OUT_DISABLE;
	cfg.sc_cfg.csc = SCL_CSC_NONE;
	cfg.sc_cfg.flip = SCL_FLIP_NO;
	cfg.sc_cfg.algorithm = SCL_BICUBIC;
	default_dst_mem(DEFAULT_W, DEFAULT_H, cfg.sc_cfg.fmt_out, 0, &cfg.sc_cfg.mem, 1);

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

int vpss_online_set_interrupt(void){
	int ret = 0, i;
	union sclr_intr mask;
	u32 irq_num[SCL_MAX_INST] = {VIVO_INT_VPSS0, VIVO_INT_VPSS1};
	sclr_mark = 0;
	mask.raw = 0xffffffff;
	uartlog("testcase_sclr, enter\n");

	for (i = 0; i < SCL_MAX_INST; i++) {
	// for (i = 0; i < 1; i++) {
		if (vpss_status[i]) {
			uartlog("vpss(%d) init\n", i);
			sclr_ctrl_init(i);
			sclr_intr_ctrl(i, mask);
			request_irq(irq_num[i], irq_handler_scl, 0, "scl int", NULL);
			uartlog("%s:vpss%d request_irq(%d)\n", __func__, i, irq_num[i]);
		}
	}

	return ret;
}

//for isp online (vpss one in, multiple out) (trig)
int vpss_online_multi(int w, int h, u64 *dst_addr)
{
	int ret = 0;
	enum vpss_dev vpss_inst, vpss_start_inst = 0;
	u8 dev_num = 4;
	enum sclr_input in;
	enum sclr_format src_fmt = SCL_FMT_YUV422;
	enum sclr_format dst_fmt = SCL_FMT_RGB_PACKED;
	struct cvi_vpss_cfg cfg[dev_num];
	memset(cfg, 0, sizeof(struct cvi_vpss_cfg) * dev_num);
	bool isShare = true;

	vpss_inst = vpss_start_inst;
	if(is_init == false){
		ret = vpss_online_set_interrupt();
		is_init = true;
	}
	for(int i = 0; i < dev_num; i++){
		if(vpss_inst == vpss_start_inst)
			in = SCL_INPUT_ISP;
		else {
			in = SCL_INPUT_SHARE;
			src_fmt = SCL_FMT_RGB_PLANAR;
		}

		cfg[i].inst = vpss_inst;
		cfg[i].tile_enable = false;
		cfg[i].img_cfg.in = in;
		cfg[i].img_cfg.fmt_in = src_fmt;
		cfg[i].img_cfg.csc = SCL_CSC_601_LIMIT_YUV2RGB;
		default_src_mem(w, h, cfg[i].img_cfg.fmt_in, i, &cfg[i].img_cfg.mem);

		cfg[i].sc_cfg.crop.x = 0;
		cfg[i].sc_cfg.crop.y = 0;
		cfg[i].sc_cfg.crop.w = w;
		cfg[i].sc_cfg.crop.h = h;
		cfg[i].sc_cfg.dst.w = cfg[i].sc_cfg.crop.w;
		cfg[i].sc_cfg.dst.h = cfg[i].sc_cfg.crop.h;
		cfg[i].sc_cfg.fmt_out = dst_fmt;
		cfg[i].sc_cfg.out_mode = SCL_OUT_DISABLE;
		cfg[i].sc_cfg.csc = SCL_CSC_NONE;
		cfg[i].sc_cfg.flip = SCL_FLIP_NO;
		cfg[i].sc_cfg.algorithm = SCL_BICUBIC;
		default_dst_mem(cfg[i].sc_cfg.dst.w, cfg[i].sc_cfg.dst.h, cfg[i].sc_cfg.fmt_out, i, &cfg[i].sc_cfg.mem, 1);
		cfg[i].sc_cfg.mem.addr0 = dst_addr[i];

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
	vpss_start_online();
	return ret;
}
//for isp online (vpss single) (trig)
int vpss_online_single(enum vpss_dev inst, int w, int h, u64 dst_addr)
{
	int ret = 0;
	struct cvi_vpss_cfg cfg;

	memset(&cfg, 0, sizeof(struct cvi_vpss_cfg));

	if(is_init == false){
		ret = vpss_online_set_interrupt();
		is_init = true;
	}
	cfg.inst = inst;
	cfg.tile_enable = false;
	cfg.img_cfg.in = SCL_INPUT_ISP;
	cfg.img_cfg.fmt_in = SCL_FMT_YUV422;
	cfg.img_cfg.csc = SCL_CSC_601_LIMIT_YUV2RGB;
	default_src_mem(w, h, cfg.img_cfg.fmt_in, 0, &cfg.img_cfg.mem);

	cfg.sc_cfg.crop.x = 0;
	cfg.sc_cfg.crop.y = 0;
	cfg.sc_cfg.crop.w = w;
	cfg.sc_cfg.crop.h = h;
	cfg.sc_cfg.dst.w = w;
	cfg.sc_cfg.dst.h = h;
	cfg.sc_cfg.fmt_out = SCL_FMT_RGB_PACKED;
	cfg.sc_cfg.out_mode = SCL_OUT_DISABLE;
	cfg.sc_cfg.csc = SCL_CSC_NONE;
	cfg.sc_cfg.flip = SCL_FLIP_NO;
	cfg.sc_cfg.algorithm = SCL_BICUBIC;
	default_dst_mem(w, h, cfg.sc_cfg.fmt_out, 0, &cfg.sc_cfg.mem, 1);
	cfg.sc_cfg.mem.addr0 = dst_addr;

	ret = basic(&cfg);
	sclr_multi_start(inst);
	sclr_top_set_cfg(inst, true);
	vpss_enable_mask = BIT(inst);
	sclr_trig_start(inst);
	uartlog("vpss(%d) start\n", inst);
	return ret;
}
//for dwa online (vpss one in, multiple out) (trig)
int vpss_dwa_online_multi(int w, int h, enum sclr_format fmt_in, u64 *dst_addr)
{
	int ret = 0;
	enum vpss_dev vpss_inst, vpss_start_inst = 0;
	u8 dev_num = 4;
	enum sclr_input in;
	enum sclr_format src_fmt = fmt_in;
	enum sclr_format dst_fmt = SCL_FMT_RGB_PACKED;
	struct cvi_vpss_cfg cfg[dev_num];
	memset(cfg, 0, sizeof(struct cvi_vpss_cfg) * dev_num);
	bool isShare = true;

	vpss_inst = vpss_start_inst;
	if(is_init == false){
		ret = vpss_online_set_interrupt();
		is_init = true;
	}
	for(int i = 0; i < dev_num; i++){
		if(vpss_inst == vpss_start_inst)
			in = SCL_INPUT_DWA;
		else {
			in = SCL_INPUT_SHARE;
			src_fmt = SCL_FMT_RGB_PLANAR;
		}

		cfg[i].inst = vpss_inst;
		cfg[i].tile_enable = false;
		cfg[i].img_cfg.in = in;
		cfg[i].img_cfg.fmt_in = src_fmt;
		cfg[i].img_cfg.csc = SCL_CSC_601_LIMIT_YUV2RGB;
		default_src_mem(w, h, cfg[i].img_cfg.fmt_in, i, &cfg[i].img_cfg.mem);

		cfg[i].sc_cfg.crop.x = 0;
		cfg[i].sc_cfg.crop.y = 0;
		cfg[i].sc_cfg.crop.w = w;
		cfg[i].sc_cfg.crop.h = h;
		cfg[i].sc_cfg.dst.w = cfg[i].sc_cfg.crop.w;
		cfg[i].sc_cfg.dst.h = cfg[i].sc_cfg.crop.h;
		cfg[i].sc_cfg.fmt_out = dst_fmt;
		cfg[i].sc_cfg.out_mode = SCL_OUT_DISABLE;
		cfg[i].sc_cfg.csc = SCL_CSC_NONE;
		cfg[i].sc_cfg.flip = SCL_FLIP_NO;
		cfg[i].sc_cfg.algorithm = SCL_BICUBIC;
		default_dst_mem(cfg[i].sc_cfg.dst.w, cfg[i].sc_cfg.dst.h, cfg[i].sc_cfg.fmt_out, i, &cfg[i].sc_cfg.mem, 1);
		cfg[i].sc_cfg.mem.addr0 = dst_addr[i];

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
	vpss_start_all(dev_num, 0, false);
	return ret;
}
//for dwa online (vpss single) (trig)
int vpss_dwa_online_single(enum vpss_dev inst, int w, int h, enum sclr_format fmt_in, u64 dst_addr)
{
	int ret = 0;
	struct cvi_vpss_cfg cfg;

	memset(&cfg, 0, sizeof(struct cvi_vpss_cfg));

	if(is_init == false){
		ret = vpss_online_set_interrupt();
		is_init = true;
	}
	cfg.inst = inst;
	cfg.tile_enable = false;
	cfg.img_cfg.in = SCL_INPUT_DWA;
	cfg.img_cfg.fmt_in = fmt_in;
	cfg.img_cfg.csc = IS_YUV_FMT(fmt_in) ? SCL_CSC_601_LIMIT_YUV2RGB : SCL_CSC_NONE;
	default_src_mem(w, h, cfg.img_cfg.fmt_in, 0, &cfg.img_cfg.mem);

	cfg.sc_cfg.crop.x = 0;
	cfg.sc_cfg.crop.y = 0;
	cfg.sc_cfg.crop.w = w;
	cfg.sc_cfg.crop.h = h;
	cfg.sc_cfg.dst.w = w;
	cfg.sc_cfg.dst.h = h;
	cfg.sc_cfg.fmt_out = SCL_FMT_RGB_PACKED;
	cfg.sc_cfg.out_mode = SCL_OUT_DISABLE;
	cfg.sc_cfg.csc = SCL_CSC_NONE;
	cfg.sc_cfg.flip = SCL_FLIP_NO;
	cfg.sc_cfg.algorithm = SCL_BICUBIC;
	default_dst_mem(w, h, cfg.sc_cfg.fmt_out, 0, &cfg.sc_cfg.mem, 1);
	cfg.sc_cfg.mem.addr0 = dst_addr;

	ret = basic(&cfg);
	sclr_top_set_cfg(inst, true);
	vpss_enable_mask = BIT(inst);
	sclr_trig_start(inst);
	uartlog("vpss(%d) start\n", inst);
	return ret;
}
//for stitch online (vpss single) (no trig)
int vpss_stitch_online_single(enum vpss_dev inst, int w, int h, u64 src_addr)
{
	int ret = 0;
	struct cvi_vpss_cfg cfg;

	memset(&cfg, 0, sizeof(struct cvi_vpss_cfg));

	if(is_init == false){
		ret = vpss_online_set_interrupt();
		is_init = true;
	}
	cfg.inst = inst;
	cfg.tile_enable = false;
	cfg.img_cfg.in = SCL_INPUT_MEM;
	cfg.img_cfg.fmt_in = SCL_FMT_YUV444;
	cfg.img_cfg.csc = SCL_CSC_601_LIMIT_YUV2RGB;
	src_addr_scl[0][R_IDX] = src_addr;
	default_src_mem(w, h, cfg.img_cfg.fmt_in, 0, &cfg.img_cfg.mem);

	cfg.sc_cfg.crop.x = 0;
	cfg.sc_cfg.crop.y = 0;
	cfg.sc_cfg.crop.w = w;
	cfg.sc_cfg.crop.h = h;
	cfg.sc_cfg.dst.w = w;
	cfg.sc_cfg.dst.h = h;
	cfg.sc_cfg.fmt_out = SCL_FMT_YUV444;
	cfg.sc_cfg.out_mode = SCL_OUT_CSC;
	cfg.sc_cfg.csc = SCL_CSC_601_LIMIT_RGB2YUV;
	cfg.sc_cfg.flip = SCL_FLIP_NO;
	cfg.sc_cfg.algorithm = SCL_BICUBIC;
	default_dst_mem(w, h, cfg.sc_cfg.fmt_out, 0, &cfg.sc_cfg.mem, 1);

	ret = basic(&cfg);
	sclr_top_set_cfg(inst, true);
	vpss_enable_mask = BIT(inst);
	sclr_set_cfg(inst, false, true, true, true);
	_reg_write(REG_SCL_TOP_STITCHING_PATH(inst), BIT(0));
	uartlog("vpss(%d) start\n", inst);
	return ret;
}

//for disp online (vpss single) (trig) (fmt_out=fmt_in)
int vpss_disp_online_single(enum vpss_dev inst, int w, int h, enum sclr_format fmt_in, u64 src_addr, bool vgop_enable)
{
	int ret = 0;
	struct cvi_vpss_cfg cfg;
	struct sclr_gop_cfg *gop_cfg;
	struct sclr_gop_ow_cfg ow_cfg;
	struct sclr_size osd_size;
	int mem_width;

	memset(&cfg, 0, sizeof(struct cvi_vpss_cfg));

	if(is_init == false){
		ret = vpss_online_set_interrupt();
		is_init = true;
	}

	cfg.inst = inst;
	cfg.tile_enable = false;
	cfg.img_cfg.in = SCL_INPUT_MEM;
	cfg.img_cfg.fmt_in = fmt_in;
	cfg.img_cfg.csc = IS_YUV_FMT(fmt_in) ? SCL_CSC_601_LIMIT_YUV2RGB : SCL_CSC_NONE;
	src_addr_scl[0][R_IDX] = src_addr;
	default_src_mem(w, h, cfg.img_cfg.fmt_in, 0, &cfg.img_cfg.mem);

	cfg.sc_cfg.crop.x = 0;
	cfg.sc_cfg.crop.y = 0;
	cfg.sc_cfg.crop.w = w;
	cfg.sc_cfg.crop.h = h;
	cfg.sc_cfg.dst.w = w;
	cfg.sc_cfg.dst.h = h;
	cfg.sc_cfg.fmt_out = SCL_FMT_RGB_PLANAR;
	cfg.sc_cfg.out_mode = SCL_OUT_DISABLE;
	cfg.sc_cfg.csc = SCL_CSC_NONE;
	cfg.sc_cfg.flip = SCL_FLIP_NO;
	cfg.sc_cfg.algorithm = SCL_BICUBIC;
	default_dst_mem(w, h, cfg.sc_cfg.fmt_out, 0, &cfg.sc_cfg.mem, 1);

	ret = basic(&cfg);
	if(vgop_enable){
		gop_cfg = sclr_gop_get_cfg(inst, 0);
		gop_cfg->gop_ctrl.b.ow0_en = 1;
		sclr_gop_set_cfg(inst, 0, gop_cfg);
		sclr_gop_set_cfg(inst, 1, gop_cfg);

		ow_cfg.fmt = 0;
		osd_size.w = w / 2;
		osd_size.h = h / 2;
		ow_cfg.img_size = osd_size;

		ow_cfg.crop_pixels = 0;
		osd_size.w -= ow_cfg.crop_pixels;
		mem_width = osd_size.w * 4;
		ow_cfg.pitch = ALIGN(mem_width, 16);
		ow_cfg.mem_size.w = ALIGN(mem_width, 16);
		ow_cfg.mem_size.h = ow_cfg.img_size.h;
		ow_cfg.start.x = 100;
		ow_cfg.start.y = 100;
		ow_cfg.end.x = ow_cfg.start.x + osd_size.w;
		ow_cfg.end.y = ow_cfg.start.y + osd_size.h;
		// gop_space[inst] -= (ow_cfg.pitch * ow_cfg.mem_size.h);
		// ow_cfg.addr = gop_addr + GOP_FRAME_OFFSET * (inst + 1) * (ow_inst + 1) + gop_space[inst];
		ow_cfg.addr = gop_addr;
		sclr_gop_ow_set_cfg(inst, 0, 0, &ow_cfg);
		sclr_gop_ow_set_cfg(inst, 1, 0, &ow_cfg);
	}
	vpss_enable_mask = BIT(inst);
	sclr_set_cfg(inst, false, (!vgop_enable), true, true);
	sclr_top_set_cfg(inst, true);
	// _reg_write(REG_SCL_TOP_SW_VS_EN(inst), BIT(0) | BIT(3));
	_reg_write(REG_SCL_TOP_STITCHING_PATH(inst), BIT(0));
	_reg_write(0x6700000c, 0x7);//vo realtime ddr
	sclr_multi_start(inst);
	sclr_trig_start(inst);
	uartlog("vpss(%d) start\n", inst);
	return ret;
}

//for disp online (vpss single) (trig) (fmt_out=fmt_in)
int vpss_disp_online_hw_single(enum vpss_dev inst, int w, int h, enum sclr_format fmt_in, u64 src_addr, bool vgop_enable)
{
	int ret = 0;
	struct cvi_vpss_cfg cfg;
	struct sclr_gop_cfg *gop_cfg;
	struct sclr_gop_ow_cfg ow_cfg;
	struct sclr_size osd_size;
	int mem_width;

	memset(&cfg, 0, sizeof(struct cvi_vpss_cfg));

	if(is_init == false){
		ret = vpss_online_set_interrupt();
		is_init = true;
	}

	cfg.inst = inst;
	cfg.tile_enable = false;
	cfg.img_cfg.in = SCL_INPUT_MEM;
	cfg.img_cfg.fmt_in = fmt_in;
	cfg.img_cfg.csc = IS_YUV_FMT(fmt_in) ? SCL_CSC_601_LIMIT_YUV2RGB : SCL_CSC_NONE;
	src_addr_scl[0][R_IDX] = src_addr;
	default_src_mem(w, h, cfg.img_cfg.fmt_in, 0, &cfg.img_cfg.mem);

	cfg.sc_cfg.crop.x = 0;
	cfg.sc_cfg.crop.y = 0;
	cfg.sc_cfg.crop.w = w;
	cfg.sc_cfg.crop.h = h;
	cfg.sc_cfg.dst.w = w;
	cfg.sc_cfg.dst.h = h;
	cfg.sc_cfg.fmt_out = SCL_FMT_RGB_PLANAR;
	cfg.sc_cfg.out_mode = SCL_OUT_DISABLE;
	cfg.sc_cfg.csc = SCL_CSC_NONE;
	cfg.sc_cfg.flip = SCL_FLIP_NO;
	cfg.sc_cfg.algorithm = SCL_BICUBIC;
	default_dst_mem(w, h, cfg.sc_cfg.fmt_out, 0, &cfg.sc_cfg.mem, 1);

	ret = basic(&cfg);
	if(vgop_enable){
		gop_cfg = sclr_gop_get_cfg(inst, 0);
		gop_cfg->gop_ctrl.b.ow0_en = 1;
		sclr_gop_set_cfg(inst, 0, gop_cfg);
		sclr_gop_set_cfg(inst, 1, gop_cfg);

		ow_cfg.fmt = 0;
		osd_size.w = w / 2;
		osd_size.h = h / 2;
		ow_cfg.img_size = osd_size;

		ow_cfg.crop_pixels = 0;
		osd_size.w -= ow_cfg.crop_pixels;
		mem_width = osd_size.w * 4;
		ow_cfg.pitch = ALIGN(mem_width, 16);
		ow_cfg.mem_size.w = ALIGN(mem_width, 16);
		ow_cfg.mem_size.h = ow_cfg.img_size.h;
		ow_cfg.start.x = 100;
		ow_cfg.start.y = 100;
		ow_cfg.end.x = ow_cfg.start.x + osd_size.w;
		ow_cfg.end.y = ow_cfg.start.y + osd_size.h;
		// gop_space[inst] -= (ow_cfg.pitch * ow_cfg.mem_size.h);
		// ow_cfg.addr = gop_addr + GOP_FRAME_OFFSET * (inst + 1) * (ow_inst + 1) + gop_space[inst];
		ow_cfg.addr = gop_addr;
		sclr_gop_ow_set_cfg(inst, 0, 0, &ow_cfg);
		sclr_gop_ow_set_cfg(inst, 1, 0, &ow_cfg);
	}
	vpss_enable_mask = BIT(inst);
	sclr_set_cfg(inst, false, (!vgop_enable), true, true);
	sclr_top_set_cfg(inst, true);
	_reg_write(REG_SCL_TOP_SW_VS_EN(inst), BIT(0) | BIT(3));
	_reg_write(REG_SCL_TOP_STITCHING_PATH(inst), BIT(0));
	_reg_write(0x6700000c, 0x7);//vo realtime ddr
	sclr_multi_start(inst); // need to set inside this func
	// sclr_trig_start(inst);
	uartlog("vpss(%d) start\n", inst);
	return ret;
}

// for vo sys performance
int vpss3_config()
{
	int ret = 0;
	enum vpss_dev inst;
	u16 in_w, in_h, x, y, w, h, out_w, out_h;
	enum sclr_algorithm algorithm;
	struct cvi_vpss_cfg cfg;

	inst = 3;
	in_w = 4608;
	in_h = 2400;
	x 	 = 0;
	y	 = 0;
	w	 = 4608;
	h	 = 2400;
	out_w = 4608;
	out_h = 2400;
	algorithm = 1;

	if(is_init == false) {
		ret = vpss_online_set_interrupt();
		is_init = true;
	}

	memset(&cfg, 0, sizeof(struct cvi_vpss_cfg));
	cfg.inst = inst;
	cfg.tile_enable = (w > 4608);
	// cfg.v_tile_enable = (h > 8189);
	cfg.v_tile_enable = (h > 8192);
	cfg.img_cfg.in = SCL_INPUT_MEM;
	cfg.img_cfg.fmt_in = SCL_FMT_RGB_PACKED;
	cfg.img_cfg.csc = SCL_CSC_NONE;

	// cfg.img_cfg.fmt_in = SCL_FMT_YUV420;
	// cfg.img_cfg.csc = SCL_CSC_601_LIMIT_YUV2RGB;

	default_src_mem(in_w, in_h, cfg.img_cfg.fmt_in, 0, &cfg.img_cfg.mem);

	cfg.sc_cfg.crop.x = x;
	cfg.sc_cfg.crop.y = y;
	cfg.sc_cfg.crop.w = w;
	cfg.sc_cfg.crop.h = h;
	cfg.sc_cfg.dst.w = out_w;
	cfg.sc_cfg.dst.h = out_h;
	cfg.sc_cfg.fmt_out = SCL_FMT_RGB_PACKED;
	cfg.sc_cfg.out_mode = SCL_OUT_DISABLE;
	cfg.sc_cfg.csc = SCL_CSC_NONE;

	// cfg.sc_cfg.fmt_out = SCL_FMT_YUV420;
	// cfg.sc_cfg.out_mode = SCL_OUT_CSC;
	// cfg.sc_cfg.csc = SCL_CSC_601_LIMIT_RGB2YUV;

	cfg.sc_cfg.flip = SCL_FLIP_NO;
	cfg.sc_cfg.algorithm = algorithm;
	default_dst_mem(cfg.sc_cfg.dst.w, cfg.sc_cfg.dst.h, cfg.sc_cfg.fmt_out, 0, &cfg.sc_cfg.mem, 1);

	ret = basic(&cfg);

	sclr_top_set_cfg(inst, true);
	sclr_top_set_src_share(inst, false);

	return ret;
}

int vpss3_trigger_single() {
	enum vpss_dev inst = 3;
	vpss_enable_mask = BIT(inst);
	uartlog("vpss(%d) start\n", inst);
	uartlog("vpss_enable_mask(%d)\n", vpss_enable_mask);
	sclr_trig_start(inst);
	return 0;
}

int vpss3_trigger_multi() {
	enum vpss_dev inst = 3;
	vpss_enable_mask = BIT(inst);
	for(int i = 0; i < 20; i++) {
		uartlog("The %d time:\n", i);
		uartlog("vpss(%d) start\n", inst);
		uartlog("vpss_enable_mask(%d)\n", vpss_enable_mask);
		sclr_trig_start(inst);
		mdelay(20);
	}
	return 0;
}


//reset during operation, then run the case to see if it is normal
int soft_reset(int argc, char **argv) {
	int ret = 0;
	struct cvi_vpss_cfg cfg;
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
	cfg.sc_cfg.dst.w = DEFAULT_W;
	cfg.sc_cfg.dst.h = DEFAULT_H;
	cfg.sc_cfg.fmt_out = SCL_FMT_RGB_PACKED;
	cfg.sc_cfg.out_mode = SCL_OUT_DISABLE;
	cfg.sc_cfg.csc = SCL_CSC_NONE;
	cfg.sc_cfg.flip = SCL_FLIP_NO;
	cfg.sc_cfg.algorithm = SCL_BICUBIC;
	default_dst_mem(DEFAULT_W, DEFAULT_H, cfg.sc_cfg.fmt_out, 0, &cfg.sc_cfg.mem, 1);

	ret = basic(&cfg);
	sclr_top_set_cfg(inst, true);
	vpss_enable_mask = BIT(inst);
	sclr_trig_start(inst);
	uartlog("vpss(%d) start\n", inst);
	sclr_img_reset(inst);
	uartlog("vpss(%d) reset\n", inst);
	if (sclr_check_irq_handler(WAIT_TIME) != 0) {
		uartlog("sclr end intr doesn't happens after 5 tick\n");
		sclr_img_reset(inst);
		vpss_enable_mask = 0;
		fail_trigger();
		ret = -1;
	} else {
		uartlog("sclr done\n");
		frame_done_trigger();
	}
	return ret;
}

int test_reset(int argc, char **argv) {
	enum vpss_dev inst = strtol(argv[1], NULL, 10);
	// union sclr_intr mask;
	// mask.raw = 0xffffffff;
	// uartlog("REG_SCL_TOP_INTR_STATUS(%d)_0 = %lx\n", inst, _reg_read(REG_SCL_TOP_INTR_STATUS(inst)));
	// sclr_intr_clr(inst, mask);
	uartlog("REG_SCL_TOP_INTR_STATUS(%d)_1 = %lx\n", inst, _reg_read(REG_SCL_TOP_INTR_STATUS(inst)));
	// uartlog("all intertupts are cleared! \n");

	if(inst <= 3) {
		_reg_write_mask(REG_IP_RESETS, BIT(inst + 3), BIT(inst + 3));
		_reg_write_mask(REG_APB_RESETS, BIT(inst + 3), BIT(inst + 3));
		for(int i = 0; i < 50; i++) {
		}
		_reg_write_mask(REG_APB_RESETS, BIT(inst + 3), 0);
		_reg_write_mask(REG_IP_RESETS, BIT(inst + 3), 0);
		uartlog("vpss(%d) reset is finished!\n", inst);
	} else {
		uartlog("inst is invalid\n");
	}
	uartlog("REG_SCL_TOP_INTR_STATUS(%d)_2 = %lx\n", inst, _reg_read(REG_SCL_TOP_INTR_STATUS(inst)));

	// u32 irq_num[SCL_MAX_INST] = {VIVO_INT_VPSS0, VIVO_INT_VPSS1,
	// 							VPSS_INTR};
	sclr_mark = 0;
	uartlog("testcase_sclr, enter\n");

	if (vpss_status[inst]) {
		uartlog("vpss(%d) init\n", inst);
		sclr_ctrl_init(inst);
		uartlog("REG_SCL_TOP_INTR_STATUS(%d)_3 = %lx\n", inst, _reg_read(REG_SCL_TOP_INTR_STATUS(inst)));
		// sclr_intr_ctrl(inst, mask);
		// uartlog("not request_irq repeatly\n");
		// request_irq(irq_num[inst], irq_handler_scl, 0, "scl int", NULL);
		// uartlog("%s:vpss%d request_irq(%d)\n", __func__, inst, irq_num[inst]);
	} else {
		uartlog("vpss(%d) status is false\n", inst);
		uartlog("exit reset\n", inst);
	}

	return 0;
}

int test_stopper_reset(int argc, char **argv) {
	int ret = 0;
	struct cvi_vpss_cfg cfg;
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
	cfg.sc_cfg.dst.w = DEFAULT_W;
	cfg.sc_cfg.dst.h = DEFAULT_H;
	cfg.sc_cfg.fmt_out = SCL_FMT_RGB_PACKED;
	cfg.sc_cfg.out_mode = SCL_OUT_DISABLE;
	cfg.sc_cfg.csc = SCL_CSC_NONE;
	cfg.sc_cfg.flip = SCL_FLIP_NO;
	cfg.sc_cfg.algorithm = SCL_BICUBIC;
	default_dst_mem(DEFAULT_W, DEFAULT_H, cfg.sc_cfg.fmt_out, 0, &cfg.sc_cfg.mem, 1);

	ret = basic(&cfg);
	sclr_top_set_cfg(inst, true);
	vpss_enable_mask = BIT(inst);
	sclr_trig_start(inst);
	uartlog("vpss(%d) start\n", inst);
	// Stopper Reset
	_reg_write_mask(REG_SCL_STOPPER_ENABLE(inst), BIT(0), BIT(0));
	while((_reg_read(REG_SCL_STOPPER_ENABLE(inst)) & BIT(2)) == 0) {
	}
	// SW_RESET
	if(inst <= 3) {
		_reg_write_mask(REG_IP_RESETS, BIT(inst + 3), BIT(inst + 3));
		_reg_write_mask(REG_APB_RESETS, BIT(inst + 3), BIT(inst + 3));
		for(int i = 0; i < 50; i++) {
		}
		_reg_write_mask(REG_APB_RESETS, BIT(inst + 3), 0);
		_reg_write_mask(REG_IP_RESETS, BIT(inst + 3), 0);
		uartlog("vpss(%d) reset is finished!\n", inst);
	} else {
		uartlog("inst is invalid\n");
	}

	union sclr_intr mask;
	// u32 irq_num[SCL_MAX_INST] = {VIVO_INT_VPSS0, VIVO_INT_VPSS1,
	// 							VPSS_INTR};
	sclr_mark = 0;
	uartlog("testcase_sclr, enter\n");
	mask.raw = 0xffffffff;

	if (vpss_status[inst]) {
		uartlog("vpss(%d) init\n", inst);
		sclr_ctrl_init(inst);
		sclr_intr_ctrl(inst, mask);
		// request_irq(irq_num[inst], irq_handler_scl, 0, "scl int", NULL);
		// uartlog("%s:vpss%d request_irq(%d)\n", __func__, inst, irq_num[inst]);
	} else {
		uartlog("vpss(%d) status is false\n", inst);
		uartlog("exit reset\n", inst);
	}

	_reg_write_mask(REG_SCL_STOPPER_ENABLE(inst), BIT(0), 0);

	uartlog("vpss(%d) reset\n", inst);
	if (sclr_check_irq_handler(WAIT_TIME) != 0) {
		uartlog("sclr end intr doesn't happens after 10000 tick\n");
		sclr_img_reset(inst);
		vpss_enable_mask = 0;
		fail_trigger();
		ret = -1;
	} else {
		uartlog("sclr done\n");
		frame_done_trigger();
	}
	return ret;
}

int test_stopper_reset_1(int argc, char **argv) {
	int ret = 0;
	enum vpss_dev inst = (enum vpss_dev)strtol(argv[1], NULL, 10);

	// Stopper Reset
	_reg_write_mask(REG_SCL_STOPPER_ENABLE(inst), BIT(0), BIT(0));
	while((_reg_read(REG_SCL_STOPPER_ENABLE(inst)) & BIT(2)) == 0) {
	}
	// SW_RESET
	if(inst <= 3) {
		_reg_write_mask(REG_IP_RESETS, BIT(inst + 3), BIT(inst + 3));
		_reg_write_mask(REG_APB_RESETS, BIT(inst + 3), BIT(inst + 3));
		for(int i = 0; i < 50; i++) {
		}
		_reg_write_mask(REG_APB_RESETS, BIT(inst + 3), 0);
		_reg_write_mask(REG_IP_RESETS, BIT(inst + 3), 0);
		uartlog("vpss(%d) reset is finished!\n", inst);
	} else {
		uartlog("inst is invalid\n");
	}

	union sclr_intr mask;
	// u32 irq_num[SCL_MAX_INST] = {VIVO_INT_VPSS0, VIVO_INT_VPSS1,
	// 							VPSS_INTR};
	sclr_mark = 0;
	uartlog("testcase_sclr, enter\n");
	mask.raw = 0xffffffff;

	if (vpss_status[inst]) {
		uartlog("vpss(%d) init\n", inst);
		sclr_ctrl_init(inst);
		sclr_intr_ctrl(inst, mask);
		// request_irq(irq_num[inst], irq_handler_scl, 0, "scl int", NULL);
		// uartlog("%s:vpss%d request_irq(%d)\n", __func__, inst, irq_num[inst]);
	} else {
		uartlog("vpss(%d) status is false\n", inst);
		uartlog("exit reset\n", inst);
	}

	_reg_write_mask(REG_SCL_STOPPER_ENABLE(inst), BIT(0), 0);

	uartlog("vpss(%d) reset\n", inst);
	return ret;
}

int test_vpss_enable(int argc, char **argv) {
	enum vpss_dev inst = strtol(argv[1], NULL, 10);
	int status = strtol(argv[2], NULL, 10);
	if(status == 1) {
		_reg_write(REG_SCL_TOP_INTR_ENABLE(inst), 0xffffffff);
	} else if(status == 0) {
		_reg_write(REG_SCL_TOP_INTR_ENABLE(inst), 0);
	} else {
		uartlog("status should be 0 or 1!\n");
	}
	uartlog("REG_SCL_TOP_INTR_ENABLE(%d) = %lx\n", inst, _reg_read(REG_SCL_TOP_INTR_ENABLE(inst)));
	return 0;
}

int test_vpss_mask(int argc, char **argv) {
	enum vpss_dev inst = strtol(argv[1], NULL, 10);
	int status = strtol(argv[2], NULL, 10);
	if(status == 1) {
		_reg_write(REG_SCL_TOP_INTR_MASK(inst), BIT(7));
	} else if(status == 0) {
		_reg_write(REG_SCL_TOP_INTR_MASK(inst), 0);
	} else {
		uartlog("status should be 0 or 1!\n");
	}
	uartlog("REG_SCL_TOP_INTR_MASK(%d) = %lx\n", inst, _reg_read(REG_SCL_TOP_INTR_MASK(inst)));
	return 0;
}

int test_multi_channel(int argc, char **argv)
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
		default_src_mem(DEFAULT_W, DEFAULT_H, cfg[i].img_cfg.fmt_in, i, &cfg[i].img_cfg.mem);

		cfg[i].sc_cfg.crop.x = 0;
		cfg[i].sc_cfg.crop.y = 0;
		cfg[i].sc_cfg.crop.w = ((i == 0) ? (DEFAULT_W / 2) : DEFAULT_W);
		cfg[i].sc_cfg.crop.h = ((i == 0) ? (DEFAULT_H / 2) : DEFAULT_H);
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

int test_multi_channel_performance(int argc, char **argv)
{
	int ret = 0;
	int dev_num;
	int in_w, in_h, out_w, out_h, vgop_enable, mem_width;
	struct sclr_gop_cfg *gop_cfg;
	struct sclr_gop_ow_cfg ow_cfg;
	struct sclr_size osd_size;
	enum vpss_dev vpss_start_inst;
	enum vpss_dev vpss_inst;
	enum sclr_input in;
	enum sclr_format src_fmt = 0;
	enum sclr_format dst_fmt = 0;
	memset(&ow_cfg, 0, sizeof(struct sclr_gop_ow_cfg));
	memset(&osd_size, 0, sizeof(struct sclr_size));
	int input_size = strtol(argv[1], NULL, 10);
	int output_size = strtol(argv[2], NULL, 10);

	dev_num = 4;
	vpss_inst = vpss_start_inst = 0;
	if (input_size == 3840) {
		in_w = 3840;
		in_h = 2160;
	} else {
		in_w = 1920;
		in_h = 1080;
	}

	vgop_enable = 0;
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

		if (cfg[i].sc_cfg.out_mode == SCL_OUT_CSC) {
			enum sclr_csc csc = SCL_CSC_NONE;
			if (IS_YUV_FMT(cfg[i].sc_cfg.fmt_out))
				csc = SCL_CSC_601_LIMIT_RGB2YUV;
			cfg[i].sc_cfg.csc = csc;
		}
		ret = basic(&cfg[i]);
		if(vgop_enable){
			gop_cfg = sclr_gop_get_cfg(cfg[i].inst, 0);
			sclr_set_cfg(cfg[i].inst, false, false, true, false);
			gop_cfg->gop_ctrl.b.ow0_en = 1;
			sclr_gop_set_cfg(cfg[i].inst, 0, gop_cfg);

			ow_cfg.fmt = 0;
			osd_size.w = out_w/2;
			osd_size.h = out_h/2;
			ow_cfg.img_size = osd_size;

			ow_cfg.crop_pixels = 0;
			osd_size.w -= ow_cfg.crop_pixels;
			mem_width = osd_size.w * 4;
			ow_cfg.pitch = ALIGN(mem_width, 16);
			ow_cfg.mem_size.w = ALIGN(mem_width, 16);
			ow_cfg.mem_size.h = ow_cfg.img_size.h;
			ow_cfg.start.x = 0;
			ow_cfg.start.y = 0;
			ow_cfg.end.x = ow_cfg.start.x + osd_size.w;
			ow_cfg.end.y = ow_cfg.start.y + osd_size.h;
			// gop_space[inst] -= (ow_cfg.pitch * ow_cfg.mem_size.h);
			// ow_cfg.addr = gop_addr + GOP_FRAME_OFFSET * (inst + 1) * (ow_inst + 1) + gop_space[inst];
			ow_cfg.addr = gop_addr;
			sclr_gop_ow_set_cfg(cfg[i].inst, 0, 0, &ow_cfg);
		}

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

//one in multiple out, one of which is closed
int test_multi_mask(int argc, char **argv)
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
	u8 mask_dev = strtol(argv[3], NULL, 10);

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
		default_src_mem(DEFAULT_W, DEFAULT_H, cfg[i].img_cfg.fmt_in, i, &cfg[i].img_cfg.mem);

		cfg[i].sc_cfg.crop.x = 0;
		cfg[i].sc_cfg.crop.y = 0;
		cfg[i].sc_cfg.crop.w = ((i == 0) ? (DEFAULT_W / 2) : DEFAULT_W);
		cfg[i].sc_cfg.crop.h = ((i == 0) ? (DEFAULT_H / 2) : DEFAULT_H);
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

	ret = vpss_start_all_mask(dev_num, vpss_start_inst, false, mask_dev);
	return ret;
}

//imgin crop
int test_img_crop_resize(int argc, char **argv)
{
	int ret = 0;
	struct cvi_vpss_cfg cfg;
	u8 inst;
	u16 in_w, in_h, x, y, w, h, out_w, out_h;
	enum sclr_algorithm algorithm;
	inst = strtol(argv[1], NULL, 10);
	in_w = strtol(argv[2], NULL, 10);
	in_h = strtol(argv[3], NULL, 10);
	x = strtol(argv[4], NULL, 10);
	y = strtol(argv[5], NULL, 10);
	w = strtol(argv[6], NULL, 10);
	h = strtol(argv[7], NULL, 10);
	out_w = strtol(argv[8], NULL, 10);
	out_h = strtol(argv[9], NULL, 10);
	algorithm = strtol(argv[10], NULL, 10);
	memset(&cfg, 0, sizeof(struct cvi_vpss_cfg));

	cfg.inst = inst;
	cfg.tile_enable = false;
	cfg.img_cfg.in = SCL_INPUT_MEM;
	cfg.img_cfg.fmt_in = SCL_FMT_RGB_PACKED;
	cfg.img_cfg.csc = SCL_CSC_NONE;
	default_src_mem(in_w, in_h, cfg.img_cfg.fmt_in, 0, &cfg.img_cfg.mem);
	cfg.img_cfg.mem.start_x = x;
	cfg.img_cfg.mem.start_y = y;
	cfg.img_cfg.mem.width = w;
	cfg.img_cfg.mem.height = h;

	cfg.sc_cfg.crop.x = 0;
	cfg.sc_cfg.crop.y = 0;
	cfg.sc_cfg.crop.w = w;
	cfg.sc_cfg.crop.h = h;
	cfg.sc_cfg.dst.w = out_w;
	cfg.sc_cfg.dst.h = out_h;
	cfg.sc_cfg.fmt_out = SCL_FMT_RGB_PACKED;
	cfg.sc_cfg.out_mode = SCL_OUT_DISABLE;
	cfg.sc_cfg.csc = SCL_CSC_NONE;
	cfg.sc_cfg.flip = SCL_FLIP_NO;
	cfg.sc_cfg.algorithm = algorithm;
	default_dst_mem(cfg.sc_cfg.dst.w, cfg.sc_cfg.dst.h, cfg.sc_cfg.fmt_out, 0, &cfg.sc_cfg.mem, 1);

	ret = basic(&cfg);
	ret |= vpss_start(inst, cfg.tile_enable, false);
	return ret;
}

int test_crop_resize_basic(u8 inst, u16 in_w, u16 in_h, u16 x, u16 y, u16 w, u16 h, u16 out_w, u16 out_h, enum sclr_algorithm algorithm)
{
	int ret = 0;
	struct cvi_vpss_cfg cfg;

	memset(&cfg, 0, sizeof(struct cvi_vpss_cfg));
	cfg.inst = inst;
	cfg.tile_enable = (w > 4608);
	// cfg.v_tile_enable = (h > 8189);
	cfg.v_tile_enable = (h > 8192);
	cfg.img_cfg.in = SCL_INPUT_MEM;
	cfg.img_cfg.fmt_in = SCL_FMT_RGB_PACKED;
	cfg.img_cfg.csc = SCL_CSC_NONE;

	default_src_mem(in_w, in_h, cfg.img_cfg.fmt_in, 0, &cfg.img_cfg.mem);

	cfg.sc_cfg.crop.x = x;
	cfg.sc_cfg.crop.y = y;
	cfg.sc_cfg.crop.w = w;
	cfg.sc_cfg.crop.h = h;
	cfg.sc_cfg.dst.w = out_w;
	cfg.sc_cfg.dst.h = out_h;
	cfg.sc_cfg.fmt_out = SCL_FMT_RGB_PACKED;
	cfg.sc_cfg.out_mode = SCL_OUT_DISABLE;
	cfg.sc_cfg.csc = SCL_CSC_NONE;

	cfg.sc_cfg.flip = SCL_FLIP_NO;
	cfg.sc_cfg.algorithm = algorithm;
	default_dst_mem(cfg.sc_cfg.dst.w, cfg.sc_cfg.dst.h, cfg.sc_cfg.fmt_out, 0, &cfg.sc_cfg.mem, 1);

	ret = basic(&cfg);
	ret |= vpss_start(inst, cfg.tile_enable, cfg.v_tile_enable);
	return ret;
}

int test_crop_resize(int argc, char **argv)
{
	u8 inst;
	u16 in_w, in_h, x, y, w, h, out_w, out_h;
	// u64 cnt;
	enum sclr_algorithm algorithm;

	inst = strtol(argv[1], NULL, 10);
	in_w = strtol(argv[2], NULL, 10);
	in_h = strtol(argv[3], NULL, 10);
	x 	 = strtol(argv[4], NULL, 10);
	y	 = strtol(argv[5], NULL, 10);
	w	 = strtol(argv[6], NULL, 10);
	h	 = strtol(argv[7], NULL, 10);
	out_w = strtol(argv[8], NULL, 10);
	out_h = strtol(argv[9], NULL, 10);
	algorithm = strtol(argv[10], NULL, 10);
	// printf("cnt: %lu \n", timer_get_tick());
	// CLINT_MTIME(cnt);
	// printf("mcnt: %lu\n", cnt);
	// timer_mdelay(1);
	// CLINT_MTIME(cnt);
	// printf("mcnt: %lu \n", cnt);
	// printf("cnt: %lu \n\n", timer_get_tick());
	// u8 start = 0;
	// for (start = 0; start < 30;start++)
		test_crop_resize_basic(inst, in_w, in_h, x, y, w, h, out_w, out_h, algorithm);
	// return test_crop_resize_basic(inst, in_w, in_h, x, y, w, h, out_w, out_h, algorithm);
	return 0;
}

int test_crop_0(int argc, char **argv)
{
	u8 inst = strtol(argv[1], NULL, 10);

	return test_crop_resize_basic(inst, DEFAULT_W, DEFAULT_H, 0, 0, 16, 16, 16, 16, SCL_BICUBIC);
}

int test_crop_1(int argc, char **argv)
{
	u8 inst = strtol(argv[1], NULL, 10);

	return test_crop_resize_basic(inst, DEFAULT_W, DEFAULT_H, DEFAULT_W - 100, DEFAULT_H - 100, 100, 100, 100, 100, SCL_BICUBIC);
}

int test_resize_0(int argc, char **argv)
{
	u8 inst = strtol(argv[1], NULL, 10);

	return test_crop_resize_basic(inst, DEFAULT_W, DEFAULT_H, 0, 0, DEFAULT_W, DEFAULT_H, 16, 16, SCL_BICUBIC);
}

int test_resize_1(int argc, char **argv)
{
	u8 inst = strtol(argv[1], NULL, 10);

	return test_crop_resize_basic(inst, DEFAULT_W, DEFAULT_H, 0, 0, 16, 16, 2048, 2048, SCL_BICUBIC);
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

int test_hsv(int argc, char **argv)
{
	int ret = 0;
	struct cvi_vpss_cfg cfg;
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
	cfg.sc_cfg.dst.w = DEFAULT_W;
	cfg.sc_cfg.dst.h = DEFAULT_H;
	cfg.sc_cfg.fmt_out = strtol(argv[2], NULL, 10) == 1 ? SCL_FMT_RGB_PACKED : SCL_FMT_RGB_PLANAR; //hsv888 packed
	cfg.sc_cfg.out_mode = SCL_OUT_HSV;
	cfg.sc_cfg.hsv_round = SCL_HSV_ROUNDING_AWAY_FROM_ZERO;
	cfg.sc_cfg.csc = SCL_CSC_NONE;
	cfg.sc_cfg.flip = SCL_FLIP_NO;
	cfg.sc_cfg.algorithm = SCL_BICUBIC;
	default_dst_mem(DEFAULT_W, DEFAULT_H, cfg.sc_cfg.fmt_out, 0, &cfg.sc_cfg.mem, 1);

	ret = basic(&cfg);
	ret |= vpss_start(inst, cfg.tile_enable, false);
	return ret;
}

int test_datatype(int argc, char **argv)
{
	int ret = 0, datasize = 0;
	struct cvi_vpss_cfg cfg;
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
	cfg.sc_cfg.dst.w = DEFAULT_W;
	cfg.sc_cfg.dst.h = DEFAULT_H;
	cfg.sc_cfg.fmt_out = SCL_FMT_BF16;
	cfg.sc_cfg.out_mode = SCL_OUT_DISABLE;
	cfg.sc_cfg.datatype = strtol(argv[2], NULL, 10); //	BF16 = 5, FP16 = 3, FP32 = 4, INT8 = 2, U8 = 1;
	cfg.sc_cfg.csc = SCL_CSC_NONE;
	cfg.sc_cfg.flip = SCL_FLIP_NO;
	cfg.sc_cfg.algorithm = SCL_BICUBIC;
	datasize = cfg.sc_cfg.datatype == SCL_OUT_FP32 ? 4 :
				((cfg.sc_cfg.datatype == SCL_OUT_BF16 || cfg.sc_cfg.datatype == SCL_OUT_FP16) ? 2 : 1);
	default_dst_mem(DEFAULT_W, DEFAULT_H, cfg.sc_cfg.fmt_out, 0, &cfg.sc_cfg.mem, datasize);

	ret = basic(&cfg);
	ret |= vpss_start(inst, cfg.tile_enable, false);
	return ret;
}

int test_csc(int argc, char **argv)
{
	int ret = 0;
	struct cvi_vpss_cfg cfg;
	u16 w = 1920;
	u16 h = 1080;
	enum vpss_dev inst = (enum vpss_dev)strtol(argv[1], NULL, 10);
	u8 csc_type = strtol(argv[4], NULL, 10);

	if (csc_type < 0 || csc_type > 4) {
		uartlog("param error\n");
		return -1;
	}

	memset(&cfg, 0, sizeof(struct cvi_vpss_cfg));
	cfg.inst = inst;
	cfg.tile_enable = false;
	cfg.img_cfg.in = SCL_INPUT_MEM;
	cfg.img_cfg.fmt_in = strtol(argv[2], NULL, 10);
	default_src_mem(w, h, cfg.img_cfg.fmt_in, 0, &cfg.img_cfg.mem);
	if (IS_YUV_FMT(cfg.img_cfg.fmt_in))
		cfg.img_cfg.csc = csc_type;
	else
		cfg.img_cfg.csc = SCL_CSC_NONE;

	cfg.sc_cfg.crop.x = 0;
	cfg.sc_cfg.crop.y = 0;
	cfg.sc_cfg.crop.w = w;
	cfg.sc_cfg.crop.h = h;
	cfg.sc_cfg.dst.w = w;
	cfg.sc_cfg.dst.h = h;
	cfg.sc_cfg.fmt_out = strtol(argv[3], NULL, 10);
	cfg.sc_cfg.flip = SCL_FLIP_NO;
	cfg.sc_cfg.algorithm = SCL_BICUBIC;
	default_dst_mem(w, h, cfg.sc_cfg.fmt_out, 0, &cfg.sc_cfg.mem, 1);
	if (IS_YUV_FMT(cfg.sc_cfg.fmt_out)) {
		cfg.sc_cfg.out_mode = SCL_OUT_CSC;
		cfg.sc_cfg.csc = csc_type + 4;
	} else {
		cfg.sc_cfg.out_mode = SCL_OUT_DISABLE;
		cfg.sc_cfg.csc = SCL_CSC_NONE;
	}
	ret = basic(&cfg);
	ret |= vpss_start(inst, cfg.tile_enable, false);
	return ret;
}

int test_csc_ds(int argc, char **argv)
{
	int ret = 0;
	struct cvi_vpss_cfg cfg;
	u16 w = 800;
	u16 h = 600;
	enum vpss_dev inst = (enum vpss_dev)strtol(argv[1], NULL, 10);

	memset(&cfg, 0, sizeof(struct cvi_vpss_cfg));
	cfg.inst = inst;
	cfg.tile_enable = false;
	cfg.img_cfg.in = SCL_INPUT_MEM;
	cfg.img_cfg.fmt_in = strtol(argv[2], NULL, 10);
	default_src_mem(w, h, cfg.img_cfg.fmt_in, 0, &cfg.img_cfg.mem);
	if (IS_YUV_FMT(cfg.img_cfg.fmt_in))
		cfg.img_cfg.csc = SCL_CSC_601_LIMIT_YUV2RGB;
	else
		cfg.img_cfg.csc = SCL_CSC_NONE;

	cfg.sc_cfg.crop.x = 0;
	cfg.sc_cfg.crop.y = 0;
	cfg.sc_cfg.crop.w = w;
	cfg.sc_cfg.crop.h = h;
	cfg.sc_cfg.dst.w = w;
	cfg.sc_cfg.dst.h = h;
	cfg.sc_cfg.fmt_out = strtol(argv[3], NULL, 10);
	cfg.sc_cfg.flip = SCL_FLIP_NO;
	cfg.sc_cfg.algorithm = SCL_BICUBIC;
	default_dst_mem(w, h, cfg.sc_cfg.fmt_out, 0, &cfg.sc_cfg.mem, 1);
	if (IS_YUV_FMT(cfg.sc_cfg.fmt_out)) {
		cfg.sc_cfg.out_mode = SCL_OUT_CSC;
		cfg.sc_cfg.csc = SCL_CSC_601_LIMIT_RGB2YUV;
	} else {
		cfg.sc_cfg.out_mode = SCL_OUT_DISABLE;
		cfg.sc_cfg.csc = SCL_CSC_NONE;
	}
	ret = basic(&cfg);

	if(cfg.sc_cfg.fmt_out == SCL_FMT_YUV420) {
		_reg_write_mask(REG_SCL_ODMA_CFG(inst), BIT(29) | BIT(30), BIT(30));
	} else if (cfg.sc_cfg.fmt_out == SCL_FMT_YUV422) {
		_reg_write_mask(REG_SCL_ODMA_CFG(inst), BIT(29) | BIT(30), BIT(29));
	} else {
		_reg_write_mask(REG_SCL_ODMA_CFG(inst), BIT(29) | BIT(30), 0);
	}
	u32 sc_ds_val = strtol(argv[4], NULL, 16);
	_reg_write(REG_SCL_ODMA_DS(inst), sc_ds_val);

	uartlog("REG_SCL_ODMA_CFG(%d) = %lx\n", inst, _reg_read(REG_SCL_ODMA_CFG(inst)));
	uartlog("REG_SCL_ODMA_DS(%d) = %lx\n", inst, _reg_read(REG_SCL_ODMA_DS(inst)));

	ret |= vpss_start(inst, cfg.tile_enable, false);
	return ret;
}

int test_clk(int argc, char **argv) {
	enum vpss_dev inst = (enum vpss_dev)strtol(argv[1], NULL, 10);
	uartlog("APB clock control = 0x%lx\n", _reg_read(REG_APB_CLOCK));
	_reg_write_mask(REG_APB_CLOCK, BIT(3 + inst), 0);
	_reg_write_mask(REG_APB_CLOCK, BIT(19 + inst), BIT(19 + inst));
	uartlog("APB clock control = 0x%lx\n", _reg_read(REG_APB_CLOCK));

	uartlog("ODMA_RGB_00 = 0x%lx\n", _reg_read(REG_SCL_ODMA_CFG(inst)));

	_reg_write_mask(REG_SCL_ODMA_CFG(inst), BIT(28), BIT(28));
	_reg_write_mask(REG_SCL_ODMA_CFG(inst), BIT(29), BIT(29));
	_reg_write_mask(REG_SCL_ODMA_CFG(inst), BIT(30), BIT(30));
	uartlog("ODMA_RGB_00 = 0x%lx\n", _reg_read(REG_SCL_ODMA_CFG(inst)));

	return 0;
}

int test_csc_resize(int argc, char **argv)
{
	int ret = 0;
	struct cvi_vpss_cfg cfg;
	int in_w = strtol(argv[4], NULL, 10);
	int in_h = strtol(argv[5], NULL, 10);
	int out_w = strtol(argv[6], NULL, 10);
	int out_h = strtol(argv[7], NULL, 10);
	enum vpss_dev inst = (enum vpss_dev)strtol(argv[1], NULL, 10);
	u8 csc_type = SCL_CSC_601_LIMIT_YUV2RGB;

	if (csc_type < 0 || csc_type > 4) {
		uartlog("param error\n");
		return -1;
	}

	memset(&cfg, 0, sizeof(struct cvi_vpss_cfg));
	cfg.inst = inst;
	cfg.tile_enable = (in_w > 4608);
	cfg.img_cfg.in = SCL_INPUT_MEM;
	cfg.img_cfg.fmt_in = strtol(argv[2], NULL, 10);
	default_src_mem(in_w, in_h, cfg.img_cfg.fmt_in, 0, &cfg.img_cfg.mem);
	if (IS_YUV_FMT(cfg.img_cfg.fmt_in))
		cfg.img_cfg.csc = csc_type;
	else
		cfg.img_cfg.csc = SCL_CSC_NONE;

	cfg.sc_cfg.crop.x = 0;
	cfg.sc_cfg.crop.y = 0;
	cfg.sc_cfg.crop.w = in_w;
	cfg.sc_cfg.crop.h = in_h;
	cfg.sc_cfg.dst.w = out_w;
	cfg.sc_cfg.dst.h = out_h;
	cfg.sc_cfg.fmt_out = strtol(argv[3], NULL, 10);
	cfg.sc_cfg.flip = SCL_FLIP_NO;
	cfg.sc_cfg.algorithm = SCL_BICUBIC;
	default_dst_mem(cfg.sc_cfg.dst.w, cfg.sc_cfg.dst.h, cfg.sc_cfg.fmt_out, 0, &cfg.sc_cfg.mem, 1);
	if (IS_YUV_FMT(cfg.sc_cfg.fmt_out)) {
		cfg.sc_cfg.out_mode = SCL_OUT_CSC;
		cfg.sc_cfg.csc = csc_type + 4;
	} else {
		cfg.sc_cfg.out_mode = SCL_OUT_DISABLE;
		cfg.sc_cfg.csc = SCL_CSC_NONE;
	}
	ret = basic(&cfg);
	ret |= vpss_start(inst, cfg.tile_enable, false);
	return ret;
}

int test_upsample(int argc, char **argv)
{
	int ret = 0;
	struct cvi_vpss_cfg cfg;
	int in_w = strtol(argv[4], NULL, 10);
	int in_h = strtol(argv[5], NULL, 10);
	int out_w = in_w;
	int out_h = in_h;
	enum vpss_dev inst = (enum vpss_dev)strtol(argv[1], NULL, 10);
	u8 csc_type = SCL_CSC_601_LIMIT_YUV2RGB;

	if (csc_type < 0 || csc_type > 4) {
		uartlog("param error\n");
		return -1;
	}

	memset(&cfg, 0, sizeof(struct cvi_vpss_cfg));
	cfg.inst = inst;
	cfg.tile_enable = false;
	cfg.img_cfg.in = SCL_INPUT_MEM;
	cfg.img_cfg.fmt_in = strtol(argv[2], NULL, 10);
	default_src_mem(in_w, in_h, cfg.img_cfg.fmt_in, 0, &cfg.img_cfg.mem);
	if (IS_YUV_FMT(cfg.img_cfg.fmt_in))
		cfg.img_cfg.csc = csc_type;
	else
		cfg.img_cfg.csc = SCL_CSC_NONE;

	cfg.sc_cfg.crop.x = 0;
	cfg.sc_cfg.crop.y = 0;
	cfg.sc_cfg.crop.w = in_w;
	cfg.sc_cfg.crop.h = in_h;
	cfg.sc_cfg.dst.w = out_w;
	cfg.sc_cfg.dst.h = out_h;
	cfg.sc_cfg.fmt_out = strtol(argv[3], NULL, 10);
	cfg.sc_cfg.flip = SCL_FLIP_NO;
	cfg.sc_cfg.algorithm = SCL_BICUBIC;
	default_dst_mem(cfg.sc_cfg.dst.w, cfg.sc_cfg.dst.h, cfg.sc_cfg.fmt_out, 0, &cfg.sc_cfg.mem, 1);
	if (IS_YUV_FMT(cfg.sc_cfg.fmt_out)) {
		cfg.sc_cfg.out_mode = SCL_OUT_CSC;
		cfg.sc_cfg.csc = csc_type + 4;
	} else {
		cfg.sc_cfg.out_mode = SCL_OUT_DISABLE;
		cfg.sc_cfg.csc = SCL_CSC_NONE;
	}
	ret = basic(&cfg);
	_reg_write_mask(REG_SCL_IMG_CFG(inst), BIT(12), 0);
	_reg_write_mask(REG_SCL_CSC_EN(inst), BIT(0), 0);
	// sclr_set_cfg(inst, /*sc_bypass*/ true, /*gop_bypass*/ true,
	// 	 /*cir_bypass*/ true, /*odma_bypass*/ false);
	ret |= vpss_start(inst, cfg.tile_enable, false);
	return ret;
}

int test_flip_basic(u8 inst, enum sclr_flip_mode flip)
{
	int ret = 0;
	struct cvi_vpss_cfg cfg;

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
	cfg.sc_cfg.dst.w = DEFAULT_W;
	cfg.sc_cfg.dst.h = DEFAULT_H;
	cfg.sc_cfg.fmt_out = SCL_FMT_RGB_PACKED;
	cfg.sc_cfg.out_mode = SCL_OUT_DISABLE;
	cfg.sc_cfg.csc = SCL_CSC_NONE;
	cfg.sc_cfg.flip = flip;
	cfg.sc_cfg.algorithm = SCL_BICUBIC;
	default_dst_mem(DEFAULT_W, DEFAULT_H, cfg.sc_cfg.fmt_out, 0, &cfg.sc_cfg.mem, 1);

	ret = basic(&cfg);
	ret |= vpss_start(inst, cfg.tile_enable, false);
	return ret;
}

int test_mirror(int argc, char **argv)
{
	u8 inst = strtol(argv[1], NULL, 10);

	return test_flip_basic(inst, SCL_FLIP_HFLIP);
}

int test_flip(int argc, char **argv)
{
	u8 inst = strtol(argv[1], NULL, 10);

	return test_flip_basic(inst, SCL_FLIP_VFLIP);
}

int test_mirror_flip(int argc, char **argv)
{
	u8 inst = strtol(argv[1], NULL, 10);

	return test_flip_basic(inst, SCL_FLIP_HVFLIP);
}

int test_tile_mode(int argc, char **argv)
{
	int ret = 0;
	struct cvi_vpss_cfg cfg;
	u16 width = 1920;//7680;
	u16 height = 1080;//4320;
	enum vpss_dev inst = (enum vpss_dev)strtol(argv[1], NULL, 10);

	uartlog("width = %d\t height = %d\n", width, height);

	memset(&cfg, 0, sizeof(struct cvi_vpss_cfg));
	cfg.inst = inst;
	cfg.tile_enable = true;
	cfg.v_tile_enable = true;
	cfg.img_cfg.in = SCL_INPUT_MEM;
	cfg.img_cfg.fmt_in = SCL_FMT_RGB_PACKED;  // SCL_FMT_YUV420
	cfg.img_cfg.csc = SCL_CSC_NONE;   // SCL_CSC_601_LIMIT_YUV2RGB
	default_src_mem(width, height, cfg.img_cfg.fmt_in, 0, &cfg.img_cfg.mem);

	cfg.sc_cfg.crop.x = 0;
	cfg.sc_cfg.crop.y = 0;
	cfg.sc_cfg.crop.w = width;
	cfg.sc_cfg.crop.h = height;
	cfg.sc_cfg.dst.w = width;
	cfg.sc_cfg.dst.h = height;
	cfg.sc_cfg.fmt_out = SCL_FMT_RGB_PACKED;  // SCL_FMT_YUV420
	cfg.sc_cfg.out_mode = SCL_OUT_DISABLE;   // SCL_OUT_CSC
	cfg.sc_cfg.csc = SCL_CSC_NONE;  // SCL_CSC_601_LIMIT_RGB2YUV
	cfg.sc_cfg.flip = SCL_FLIP_NO;
	cfg.sc_cfg.algorithm = SCL_BICUBIC;
	default_dst_mem(cfg.sc_cfg.dst.w, cfg.sc_cfg.dst.h, cfg.sc_cfg.fmt_out, 0, &cfg.sc_cfg.mem, 1);

	ret = basic(&cfg);
	ret |= vpss_start(inst, cfg.tile_enable, cfg.v_tile_enable);
	return ret;
}

int test_crop_tile(int argc, char **argv)
{
	int ret = 0;
	struct cvi_vpss_cfg cfg;
	u16 width = strtol(argv[2], NULL, 10);
	u16 height = strtol(argv[3], NULL, 10);
	enum vpss_dev inst = (enum vpss_dev)strtol(argv[1], NULL, 10);

	memset(&cfg, 0, sizeof(struct cvi_vpss_cfg));
	cfg.inst = inst;
	cfg.tile_enable = true;
	cfg.v_tile_enable = (strtol(argv[7], NULL, 10) > 8189);
	cfg.img_cfg.in = SCL_INPUT_MEM;
	cfg.img_cfg.fmt_in = SCL_FMT_RGB_PACKED;
	cfg.img_cfg.csc = SCL_CSC_NONE;
	default_src_mem(width, height, cfg.img_cfg.fmt_in, 0, &cfg.img_cfg.mem);

	cfg.sc_cfg.crop.x = strtol(argv[4], NULL, 10);
	cfg.sc_cfg.crop.y = strtol(argv[5], NULL, 10);
	cfg.sc_cfg.crop.w = strtol(argv[6], NULL, 10);
	cfg.sc_cfg.crop.h = strtol(argv[7], NULL, 10);
	cfg.sc_cfg.dst.w = strtol(argv[8], NULL, 10);
	cfg.sc_cfg.dst.h = strtol(argv[9], NULL, 10);
	cfg.sc_cfg.fmt_out = SCL_FMT_RGB_PACKED;
	cfg.sc_cfg.out_mode = SCL_OUT_DISABLE;
	cfg.sc_cfg.csc = SCL_CSC_NONE;
	cfg.sc_cfg.flip = SCL_FLIP_NO;
	cfg.sc_cfg.algorithm = strtol(argv[10], NULL, 10);
	default_dst_mem(cfg.sc_cfg.dst.w, cfg.sc_cfg.dst.h, cfg.sc_cfg.fmt_out, 0, &cfg.sc_cfg.mem, 1);

	ret = basic(&cfg);
	ret |= vpss_start(inst, cfg.tile_enable, cfg.v_tile_enable);
	return ret;
}

int test_privacy_mask(int argc, char **argv)
{
	int ret = 0;
	struct cvi_vpss_cfg cfg;
	struct sclr_privacy_cfg prm_cfg;
	u64 map_addr = src_addr_scl[2][R_IDX];
	u8 *ptr = (u8 *)map_addr;
	int map_w, map_h;
	int privacy_mask_w = 128, privacy_mask_h = 128;
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
	cfg.sc_cfg.dst.w = DEFAULT_W;
	cfg.sc_cfg.dst.h = DEFAULT_H;
	cfg.sc_cfg.fmt_out = SCL_FMT_RGB_PACKED;
	cfg.sc_cfg.out_mode = SCL_OUT_DISABLE;
	cfg.sc_cfg.csc = SCL_CSC_NONE;
	cfg.sc_cfg.flip = SCL_FLIP_NO;
	cfg.sc_cfg.algorithm = SCL_BICUBIC;
	default_dst_mem(DEFAULT_W, DEFAULT_H, cfg.sc_cfg.fmt_out, 0, &cfg.sc_cfg.mem, 1);

	// union sclr_intr intr_status_test = sclr_intr_status(inst);
	// uartlog(" ***> vpss test (%d) status(%#x)\n", inst, intr_status_test.raw);

	ret = basic(&cfg);

	// red(0xe0) green(0x1c) blue(0x03)
	prm_cfg.cfg.raw = 0;
	prm_cfg.cfg.b.enable = 1;
	prm_cfg.cfg.b.mode = strtol(argv[2], NULL, 10);	// grid mode
	prm_cfg.cfg.b.grid_size = strtol(argv[3], NULL, 10);	// 8x8
	prm_cfg.cfg.b.fit_picture = 0;	// no full image
	prm_cfg.cfg.b.force_alpha = 0;
	prm_cfg.map_cfg.alpha_factor = 255;
	prm_cfg.cfg.b.mask_rgb332 = strtol(argv[4], NULL, 10);
	prm_cfg.map_cfg.no_mask_idx = 0;
	prm_cfg.map_cfg.base = map_addr;
	prm_cfg.map_cfg.axi_burst = 7;
	prm_cfg.start.x = strtol(argv[5], NULL, 10);
	prm_cfg.end.x = prm_cfg.start.x + privacy_mask_w - 1;	// x + 128
	prm_cfg.start.y = strtol(argv[6], NULL, 10);
	prm_cfg.end.y = prm_cfg.start.y + privacy_mask_h - 1;	// y + 128

	if (prm_cfg.cfg.b.mode) {
		map_w = privacy_mask_w;
		map_h = privacy_mask_h;
	} else {
		map_w = privacy_mask_w / (prm_cfg.cfg.b.grid_size ? 16 : 8);
		map_h = privacy_mask_h / (prm_cfg.cfg.b.grid_size ? 16 : 8);
	}

	for (int i = 0; i < map_h; i++)
		for (int j = 0; j < map_w; j++)
			ptr[i * map_w + j] = (u8)rand();
	flush_dcache_range((unsigned long)(map_addr), map_w*map_h);
	// inval_dcache_range((unsigned long)(map_addr), 16*16*3);
	sclr_pri_set_cfg(inst, &prm_cfg);

	ret |= vpss_start(inst, cfg.tile_enable, false);

	prm_cfg.cfg.b.enable = 0;
	sclr_pri_set_cfg(inst, &prm_cfg);

	return ret;
}

int privacy_mask_complex(int argc, char **argv)
{
	int ret = 0;
	struct cvi_vpss_cfg cfg;
	struct sclr_privacy_cfg prm_cfg;
	u64 map_addr = src_addr_scl[2][R_IDX];
	u8 *ptr = (u8 *)map_addr;
	int map_w, map_h;
	int is_mem = strtol(argv[9], NULL, 10);
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
	cfg.sc_cfg.dst.w = DEFAULT_W;
	cfg.sc_cfg.dst.h = DEFAULT_H;
	cfg.sc_cfg.fmt_out = SCL_FMT_RGB_PACKED;
	cfg.sc_cfg.out_mode = SCL_OUT_DISABLE;
	cfg.sc_cfg.csc = SCL_CSC_NONE;
	cfg.sc_cfg.flip = SCL_FLIP_NO;
	cfg.sc_cfg.algorithm = SCL_BICUBIC;
	default_dst_mem(DEFAULT_W, DEFAULT_H, cfg.sc_cfg.fmt_out, 0, &cfg.sc_cfg.mem, 1);

	ret = basic(&cfg);

	// red(0xe0) green(0x1c) blue(0x03)
	prm_cfg.cfg.raw = 0;
	prm_cfg.cfg.b.enable = 1;
	prm_cfg.cfg.b.mode = strtol(argv[2], NULL, 10);	// grid mode
	prm_cfg.cfg.b.grid_size = strtol(argv[3], NULL, 10);	// 8x8
	prm_cfg.cfg.b.fit_picture = 0;	// no full image
	prm_cfg.cfg.b.force_alpha = strtol(argv[5], NULL, 10);
	prm_cfg.map_cfg.alpha_factor = strtol(argv[6], NULL, 10);
	prm_cfg.cfg.b.mask_rgb332 = strtol(argv[4], NULL, 10);
	prm_cfg.map_cfg.no_mask_idx = strtol(argv[7], NULL, 10);
	prm_cfg.cfg.b.y2r_enable = strtol(argv[8], NULL, 10);
	prm_cfg.map_cfg.base = map_addr;
	prm_cfg.map_cfg.axi_burst = 7;
	prm_cfg.start.x = 100;
	prm_cfg.end.x = 228 - 1;	// x + 128
	prm_cfg.start.y = 200;
	prm_cfg.end.y = 328 - 1;	// y + 128
	map_w = 16;
	map_h = 16;
	if(!is_mem){
		for (int i = 0; i < map_h; i++)
			for (int j = 0; j < map_w; j++)
				ptr[i * map_w + j] = (u8)(rand());
	}
	// flush_dcache_range((unsigned long)(map_addr), 16*16*3);
	// inval_dcache_range((unsigned long)(map_addr), 16*16*3);
	sclr_pri_set_cfg(inst, &prm_cfg);

	ret |= vpss_start(inst, cfg.tile_enable, false);

	prm_cfg.cfg.b.enable = 0;
	sclr_pri_set_cfg(inst, &prm_cfg);
	return ret;
}

int test_circle(int argc, char **argv)
{
	int ret = 0;
	struct cvi_vpss_cfg cfg;
	struct sclr_cir_cfg cir_cfg;
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
	cfg.sc_cfg.dst.w = DEFAULT_W;
	cfg.sc_cfg.dst.h = DEFAULT_H;
	cfg.sc_cfg.fmt_out = SCL_FMT_RGB_PACKED;
	cfg.sc_cfg.out_mode = SCL_OUT_DISABLE;
	cfg.sc_cfg.csc = SCL_CSC_NONE;
	cfg.sc_cfg.flip = SCL_FLIP_NO;
	cfg.sc_cfg.algorithm = SCL_BICUBIC;
	default_dst_mem(DEFAULT_W, DEFAULT_H, cfg.sc_cfg.fmt_out, 0, &cfg.sc_cfg.mem, 1);
	ret = basic(&cfg);

	memset(&cir_cfg, 0, sizeof(struct sclr_cir_cfg));
	cir_cfg.mode = strtol(argv[2], NULL, 10);
	cir_cfg.line_width = 10;
	cir_cfg.center.x = 399;
	cir_cfg.center.y = 299;
	cir_cfg.radius = 100;
	cir_cfg.rect.x = 0;
	cir_cfg.rect.y = 0;
	cir_cfg.rect.w = DEFAULT_W;
	cir_cfg.rect.h = DEFAULT_H;
	cir_cfg.color_r = 255;
	cir_cfg.color_g = 0;
	cir_cfg.color_b = 0;
	sclr_cir_set_cfg(inst, &cir_cfg);
	sclr_set_cfg(inst, false, true, false, false);

	ret |= vpss_start(inst, cfg.tile_enable, false);
	sclr_set_cfg(inst, false, true, true, false);

	return ret;
}

int test_border(int argc, char **argv)
{
	int ret = 0;
	struct cvi_vpss_cfg cfg;
	struct sclr_border_cfg border_cfg;
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
	cfg.sc_cfg.dst.w = strtol(argv[4], NULL, 10);
	cfg.sc_cfg.dst.h = strtol(argv[5], NULL, 10);
	cfg.sc_cfg.fmt_out = SCL_FMT_RGB_PACKED;
	cfg.sc_cfg.out_mode = SCL_OUT_DISABLE;
	cfg.sc_cfg.csc = SCL_CSC_NONE;
	cfg.sc_cfg.flip = SCL_FLIP_NO;
	cfg.sc_cfg.algorithm = SCL_BICUBIC;
	default_dst_mem(DEFAULT_W, DEFAULT_H, cfg.sc_cfg.fmt_out, 0, &cfg.sc_cfg.mem, 1);
	ret = basic(&cfg);

	border_cfg.cfg.b.enable = 1;
	border_cfg.cfg.b.bd_color_r = 255;
	border_cfg.cfg.b.bd_color_g = 255;
	border_cfg.cfg.b.bd_color_b = 0;
	border_cfg.start.x = strtol(argv[2], NULL, 10);
	border_cfg.start.y = strtol(argv[3], NULL, 10);
	sclr_border_set_cfg(inst, &border_cfg);

	ret |= vpss_start(inst, cfg.tile_enable, false);

	border_cfg.cfg.b.enable = 0;
	sclr_border_set_cfg(inst, &border_cfg);

	return ret;
}

int test_border_vpp(int argc, char **argv)
{
	int ret = 0;
	struct cvi_vpss_cfg cfg;
	struct sclr_border_vpp_cfg border_cfg[BORDER_VPP_MAX];
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

	for(int i = 0; i < BORDER_VPP_MAX; i++){
		border_cfg[i].cfg.b.enable = 1;
		border_cfg[i].cfg.b.bd_color_r = 255;
		border_cfg[i].cfg.b.bd_color_g = 255;
		border_cfg[i].cfg.b.bd_color_b = 0;
		border_cfg[i].inside_start.x = 100 + i * 50;
		border_cfg[i].inside_start.y = 100 + i * 50;
		border_cfg[i].inside_end.x = 200 + i * 50;
		border_cfg[i].inside_end.y = 200 + i * 50;
		border_cfg[i].outside_start.x = 90 + i * 50;
		border_cfg[i].outside_start.y = 90 + i * 50;
		border_cfg[i].outside_end.x = 210 + i * 50;
		border_cfg[i].outside_end.y = 210 + i * 50;
		sclr_border_vpp_set_cfg(inst, i, &border_cfg[i]);
	}

	ret |= vpss_start(inst, cfg.tile_enable, false);

	for(int i = 0; i < BORDER_VPP_MAX; i++){
		border_cfg[i].cfg.b.enable = 0;
		sclr_border_vpp_set_cfg(inst, i, border_cfg + i);
	}

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

int test_draw_rectangle(int argc, char **argv)
{
	int ret = 0;
	struct cvi_vpss_cfg cfg;
	struct sclr_border_vpp_cfg border_cfg;
	enum vpss_dev inst = (enum vpss_dev)strtol(argv[1], NULL, 10);

	memset(&cfg, 0, sizeof(struct cvi_vpss_cfg));
	cfg.inst = inst;
	cfg.tile_enable = false;
	cfg.img_cfg.in = SCL_INPUT_MEM;
	cfg.img_cfg.fmt_in = SCL_FMT_RGB_PACKED;
	cfg.img_cfg.csc = IS_YUV_FMT(cfg.img_cfg.fmt_in) ? SCL_CSC_601_LIMIT_YUV2RGB : SCL_CSC_NONE;
	default_src_mem(DEFAULT_W, DEFAULT_H, cfg.img_cfg.fmt_in, 0, &cfg.img_cfg.mem);

	cfg.sc_cfg.crop.x = 0;
	cfg.sc_cfg.crop.y = 0;
	cfg.sc_cfg.crop.w = DEFAULT_W;
	cfg.sc_cfg.crop.h = DEFAULT_H;
	cfg.sc_cfg.dst.w = DEFAULT_W ;
	cfg.sc_cfg.dst.h = DEFAULT_H;
	cfg.sc_cfg.fmt_out = SCL_FMT_RGB_PACKED;
	cfg.sc_cfg.out_mode = IS_YUV_FMT(cfg.sc_cfg.fmt_out) ? SCL_OUT_CSC : SCL_OUT_DISABLE;
	cfg.sc_cfg.csc = IS_YUV_FMT(cfg.sc_cfg.fmt_out) ? SCL_CSC_601_LIMIT_RGB2YUV : SCL_CSC_NONE;
	cfg.sc_cfg.flip = SCL_FLIP_NO;
	cfg.sc_cfg.algorithm = SCL_BICUBIC;
	default_dst_mem(DEFAULT_W, DEFAULT_H, cfg.sc_cfg.fmt_out, 0, &cfg.sc_cfg.mem, 1);
	ret = basic(&cfg);

	border_cfg.cfg.b.enable = 1;
	border_cfg.cfg.b.bd_color_r = 255;
	border_cfg.cfg.b.bd_color_g = 0;
	border_cfg.cfg.b.bd_color_b = 0;
	border_cfg.inside_start.x = strtol(argv[2], NULL, 10);
	border_cfg.inside_start.y = strtol(argv[3], NULL, 10);
	border_cfg.inside_end.x = strtol(argv[4], NULL, 10);
	border_cfg.inside_end.y = strtol(argv[5], NULL, 10);
	border_cfg.outside_start.x = strtol(argv[6], NULL, 10);
	border_cfg.outside_start.y = strtol(argv[7], NULL, 10);
	border_cfg.outside_end.x = strtol(argv[8], NULL, 10);
	border_cfg.outside_end.y = strtol(argv[9], NULL, 10);
	sclr_border_vpp_set_cfg(inst, 0, &border_cfg);

	ret |= vpss_start(inst, cfg.tile_enable, false);

	border_cfg.cfg.b.enable = 0;
	sclr_border_vpp_set_cfg(inst, 0, &border_cfg);

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

int test_fill_rectangle(int argc, char **argv)
{
	int ret = 0;
	struct cvi_vpss_cfg cfg;
	struct sclr_cover_cfg cover_cfg;
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

	cover_cfg.start.b.enable = 1;
	cover_cfg.start.b.x = strtol(argv[2], NULL, 10);
	cover_cfg.start.b.y = strtol(argv[3], NULL, 10);
	cover_cfg.img_size.w = strtol(argv[4], NULL, 10);
	cover_cfg.img_size.h = strtol(argv[5], NULL, 10);
	cover_cfg.color.b.cover_color_r = 255;
	cover_cfg.color.b.cover_color_g = 0;
	cover_cfg.color.b.cover_color_b = 0;
	sclr_cover_set_cfg(inst, 0, &cover_cfg);

	ret |= vpss_start(inst, cfg.tile_enable, false);

	cover_cfg.start.b.enable = 0;
	sclr_cover_set_cfg(inst, 0, &cover_cfg);

	return ret;
}

int test_quant(int argc, char **argv)
{
	int ret = 0;
	struct cvi_vpss_cfg cfg;
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
	cfg.sc_cfg.dst.w = DEFAULT_W;
	cfg.sc_cfg.dst.h = DEFAULT_H;
	cfg.sc_cfg.fmt_out = SCL_FMT_RGB_PACKED;
	cfg.sc_cfg.csc = SCL_CSC_NONE;
	cfg.sc_cfg.flip = SCL_FLIP_NO;
	cfg.sc_cfg.algorithm = SCL_BICUBIC;
	cfg.sc_cfg.datatype = strtol(argv[9], NULL, 10);//BF16 = 5, FP16 = 3, FP32 = 4, INT8 = 2, U8 = 1;
	int datasize = cfg.sc_cfg.datatype == SCL_OUT_FP32 ? 4 :
				((cfg.sc_cfg.datatype == SCL_OUT_BF16 || cfg.sc_cfg.datatype == SCL_OUT_FP16) ? 2 : 1);
	if(cfg.sc_cfg.datatype != SCL_DISABLE)
		cfg.sc_cfg.fmt_out = SCL_FMT_BF16;
	default_dst_mem(DEFAULT_W, DEFAULT_H, cfg.sc_cfg.fmt_out, 0, &cfg.sc_cfg.mem, datasize);

	cfg.sc_cfg.out_mode = SCL_OUT_QUANT;
	cfg.sc_cfg.quant_form.sc_frac[0] = strtod(argv[2], NULL) * 8192;
	cfg.sc_cfg.quant_form.sc_frac[1] = strtod(argv[3], NULL) * 8192;
	cfg.sc_cfg.quant_form.sc_frac[2] = strtod(argv[4], NULL) * 8192;
	cfg.sc_cfg.quant_form.sub[0] = (u8)strtod(argv[5], NULL);
	cfg.sc_cfg.quant_form.sub[1] = (u8)strtod(argv[6], NULL);
	cfg.sc_cfg.quant_form.sub[2] = (u8)strtod(argv[7], NULL);
	cfg.sc_cfg.quant_form.sub_frac[0] = (strtod(argv[5], NULL) - cfg.sc_cfg.quant_form.sub[0]) * 1024;
	cfg.sc_cfg.quant_form.sub_frac[1] = (strtod(argv[6], NULL) - cfg.sc_cfg.quant_form.sub[1]) * 1024;
	cfg.sc_cfg.quant_form.sub_frac[2] = (strtod(argv[7], NULL) - cfg.sc_cfg.quant_form.sub[2]) * 1024;
	cfg.sc_cfg.quant_round = strtol(argv[8], NULL, 10);

	uartlog("cfg.sc_cfg.quant_form.sub[0] = %u\n", cfg.sc_cfg.quant_form.sub[0]);
	uartlog("cfg.sc_cfg.quant_form.sub[1] = %u\n", cfg.sc_cfg.quant_form.sub[1]);
	uartlog("cfg.sc_cfg.quant_form.sub[2] = %u\n", cfg.sc_cfg.quant_form.sub[2]);
	uartlog("cfg.sc_cfg.quant_form.sub_frac[0] = %u\n", cfg.sc_cfg.quant_form.sub_frac[0]);
	uartlog("cfg.sc_cfg.quant_form.sub_frac[1] = %u\n", cfg.sc_cfg.quant_form.sub_frac[1]);
	uartlog("cfg.sc_cfg.quant_form.sub_frac[2] = %u\n", cfg.sc_cfg.quant_form.sub_frac[2]);

	ret = basic(&cfg);
	ret |= vpss_start(inst, cfg.tile_enable, false);

	return ret;
}

int test_cmdq(int argc, char **argv)
{
	int ret = 0;
	union cmdq_set *cmd_start = (union cmdq_set *)cmdq_set_addr;
	int size;
	int pattern;
	u8 inst = strtol(argv[1], NULL, 10);
	u8 op = strtol(argv[2], NULL, 10);

	if ((op == 0) || (op == 1) || (op == 2)) {
		u64 addr = 0;
		addr = strtol(argv[3], NULL, 10);
		size = strtol(argv[4], NULL, 10);
		pattern = strtol(argv[5], NULL, 10);

		ret = _cmdq_testcase(inst, cmd_start, op, size, pattern, addr, 0);
	} else if (op == 3) {
		u64 addr1 = 0, addr2 = 0;
		addr1 = strtol(argv[3], NULL, 10);
		addr2 = strtol(argv[4], NULL, 10);
		size = strtol(argv[5], NULL, 10);
		for (int i = 0; i < 3; ++i) {
			pattern = (i & 0x01) ? 0x00001234 : 0x00002234;
			ret = _cmdq_testcase(inst, cmd_start, op, size, pattern, addr1, addr2);
		}
	} else if (op == 4) {
		for (int i = 0; i < 10; ++i) {
			uartlog("stress test(%d)\n", i);
			pattern = (i & 0x01) ? 0x00001234 : 0x00002234;
			if ((ret = _cmdq_testcase(inst, cmd_start, 0, 6, pattern, REG_SCL_IMG_CSC_COEF0(0), 0)) == -1) break;
			//if ((ret = _cmdq_testcase(cmd_start, 0, 6, pattern, REG_SCL_CSC_COEF0(0), 0)) == -1) break;
		}
	} else if (op == 5) {
		// test image: 2k-16.jpg
		struct sclr_ctrl_cfg cfgs;
		union sclr_intr mask;
		memset(&cfgs, 0, sizeof(cfgs));
		memset(&mask, 0, sizeof(mask));

		cfgs.img_inst = inst;
		cfgs.input = SCL_INPUT_MEM;
		cfgs.src_fmt = SCL_FMT_RGB_PACKED;
		cfgs.src_csc = SCL_CSC_NONE;
		cfgs.src.w = DEFAULT_W;
		cfgs.src.h = DEFAULT_H;
		cfgs.src_crop.x = 0;
		cfgs.src_crop.y = 0;
		cfgs.src_crop.w = DEFAULT_W;
		cfgs.src_crop.h = DEFAULT_H;
		cfgs.src_addr0 = src_addr_scl[0][R_IDX];
		cfgs.src_addr1 = src_addr_scl[0][G_IDX];
		cfgs.src_addr2 = src_addr_scl[0][B_IDX];
		cfgs.dst.inst = inst;
		cfgs.dst.fmt = SCL_FMT_RGB_PACKED;
		cfgs.dst.csc = SCL_CSC_NONE;
		cfgs.dst.frame.w = DEFAULT_W;
		cfgs.dst.frame.h = DEFAULT_H;
		cfgs.dst.offset.x = 0;
		cfgs.dst.offset.y = 0;
		cfgs.dst.window.w = DEFAULT_W;
		cfgs.dst.window.h = DEFAULT_H;
		cfgs.dst.addr0 = dst_addr_scl[0][R_IDX];
		cfgs.dst.addr1 = dst_addr_scl[0][G_IDX];
		cfgs.dst.addr2 = dst_addr_scl[0][B_IDX];
		cfgs.dst.pitch_y = DEFAULT_W * 3;

		enable_cmdq = true;
		// disable scl's intr to have no scl's irq during process of cmdq
		mask.b.cmdq = true;
		sclr_set_intr_mask(inst, mask);
		uartlog("sclr_set_intr_mask\n");
		sclr_engine_cmdq(inst, &cfgs, 1, cmdq_set_addr);
		uartlog("sclr_engine_cmdq\n");

		if (sclr_check_irq_handler(WAIT_TIME) != 0) {
			uartlog("cmdq end intr doesn't happens after %d tick\n", WAIT_TIME);
			ret = -1;
		} else {
			uartlog("vpss(%d) cmdq done\n", inst);
		}
		enable_cmdq = false;

		mask.b.scl_frame_end = true;
		sclr_set_intr_mask(inst, mask);
	}

	return ret;
}

int vpss_stitch(enum vpss_dev inst, struct vpss_stitch_cfg cfg){
	union sclr_intr mask;
	int ret = 0;
	u8 stitch_num = cfg.num;
	struct sclr_ctrl_cfg ctrl_cfg[stitch_num];
	memset(&ctrl_cfg, 0, sizeof(struct sclr_ctrl_cfg) * stitch_num);
	memset(&mask, 0, sizeof(mask));

	for(int i = 0; i < stitch_num; i++){
		ctrl_cfg[i].img_inst = inst;
		ctrl_cfg[i].input = SCL_INPUT_MEM;
		ctrl_cfg[i].src_fmt = cfg.chn_info[i].fmt_in;
		ctrl_cfg[i].src_csc = SCL_CSC_NONE;
		ctrl_cfg[i].src.w = cfg.chn_info[i].mem_in.width;
		ctrl_cfg[i].src.h = cfg.chn_info[i].mem_in.height;
		ctrl_cfg[i].src_crop.x = 0;
		ctrl_cfg[i].src_crop.y = 0;
		ctrl_cfg[i].src_crop.w = cfg.chn_info[i].mem_in.width;
		ctrl_cfg[i].src_crop.h = cfg.chn_info[i].mem_in.height;
		ctrl_cfg[i].src_addr0 = cfg.chn_info[i].mem_in.addr0;
		ctrl_cfg[i].src_addr1 = cfg.chn_info[i].mem_in.addr1;
		ctrl_cfg[i].src_addr2 = cfg.chn_info[i].mem_in.addr2;
		ctrl_cfg[i].dst.inst = inst;
		ctrl_cfg[i].dst.fmt = cfg.fmt_out;
		ctrl_cfg[i].dst.csc = SCL_CSC_NONE;
		ctrl_cfg[i].dst.addr0 = cfg.mem_out.addr0;
		ctrl_cfg[i].dst.addr1 = cfg.mem_out.addr1;
		ctrl_cfg[i].dst.addr2 = cfg.mem_out.addr2;
		if(i == 0){
			ctrl_cfg[i].border_cfg.cfg.b.enable = 1;
			ctrl_cfg[i].border_cfg.cfg.b.bd_color_r = cfg.color >> 16;
			ctrl_cfg[i].border_cfg.cfg.b.bd_color_g = (cfg.color & 0xffff) >> 8;
			ctrl_cfg[i].border_cfg.cfg.b.bd_color_b = cfg.color & 0xff;
			ctrl_cfg[i].border_cfg.start.x = cfg.chn_info[i].rect_dst.x;
			ctrl_cfg[i].border_cfg.start.y = cfg.chn_info[i].rect_dst.y;
			ctrl_cfg[i].dst.offset.x = 0;
			ctrl_cfg[i].dst.offset.y = 0;
			ctrl_cfg[i].dst.window.w = cfg.mem_out.width;
			ctrl_cfg[i].dst.window.h = cfg.mem_out.height;
			ctrl_cfg[i].dst.frame.w = cfg.chn_info[i].rect_dst.w;
			ctrl_cfg[i].dst.frame.h = cfg.chn_info[i].rect_dst.h;
		}
		else{
			ctrl_cfg[i].dst.offset.x = cfg.chn_info[i].rect_dst.x;
			ctrl_cfg[i].dst.offset.y = cfg.chn_info[i].rect_dst.y;
			ctrl_cfg[i].dst.window.w = cfg.chn_info[i].rect_dst.w;
			ctrl_cfg[i].dst.window.h = cfg.chn_info[i].rect_dst.h;
			ctrl_cfg[i].dst.frame.w = cfg.chn_info[i].rect_dst.w;
			ctrl_cfg[i].dst.frame.h = cfg.chn_info[i].rect_dst.h;
		}
		ctrl_cfg[i].dst.pitch_y = IS_PACKED_FMT(cfg.fmt_out)
										? VIP_ALIGN(3 * cfg.mem_out.width)
										: VIP_ALIGN(cfg.mem_out.width);
		ctrl_cfg[i].dst.pitch_c = IS_YUV_FMT(cfg.fmt_out)
										? VIP_ALIGN(cfg.mem_out.width >> 1)
										: VIP_ALIGN(cfg.mem_out.width);
	}
	enable_cmdq = true;
	// disable scl's intr to have no scl's irq during process of cmdq
	mask.b.cmdq = true;
	sclr_set_intr_mask(inst, mask);
	uartlog("sclr_set_intr_mask\n");
	sclr_engine_cmdq(inst, ctrl_cfg, stitch_num, cmdq_set_addr);
	uartlog("sclr_engine_cmdq%d\n", stitch_num);

	if (sclr_check_irq_handler(WAIT_TIME) != 0) {
		uartlog("cmdq end intr doesn't happens after %d tick\n", WAIT_TIME);
		ret = -1;
	} else {
		uartlog("vpss(%d) cmdq done\n", inst);
	}
	enable_cmdq = false;

	mask.b.scl_frame_end = true;
	sclr_set_intr_mask(inst, mask);
	return ret;
}

int test_stitch_field(int argc, char **argv){
	int ret = 0;
	struct vpss_stitch_cfg cfg;
	memset(&cfg, 0, sizeof(struct vpss_stitch_cfg));
	enum vpss_dev inst = strtol(argv[1], NULL, 10);
	int datasize = 1;
	int idx = 0;
	cfg.num = 9;
	cfg.color = 0;
	cfg.fmt_out = SCL_FMT_RGB_PACKED;
	default_dst_mem(1920, 1080, cfg.fmt_out, 0, &cfg.mem_out, datasize);
	for(int i = 0; i < 3; i++){
		for(int j = 0; j < 3; j++){
			idx = i * 3 + j;
			cfg.chn_info[idx].fmt_in = SCL_FMT_RGB_PACKED;
			default_src_mem(DEFAULT_W, DEFAULT_H, cfg.chn_info[idx].fmt_in, 0, &cfg.chn_info[idx].mem_in);
			cfg.chn_info[idx].rect_dst.x = i * 640;
			cfg.chn_info[idx].rect_dst.y = j * 360;
			cfg.chn_info[idx].rect_dst.w = 640;
			cfg.chn_info[idx].rect_dst.h = 360;
		}
	}
	ret = vpss_stitch(inst, cfg);
	return ret;
}

int test_stitch(int argc, char **argv){
	int ret = 0;
	struct vpss_stitch_cfg cfg;
	memset(&cfg, 0, sizeof(struct vpss_stitch_cfg));
	enum vpss_dev inst = strtol(argv[1], NULL, 10);
	int datasize = 1;
	cfg.num = 2;
	cfg.color = 0;
	cfg.fmt_out = SCL_FMT_RGB_PACKED;
	default_dst_mem(1920, 1080, cfg.fmt_out, 0, &cfg.mem_out, datasize);

	cfg.chn_info[0].fmt_in = SCL_FMT_RGB_PACKED;
	default_src_mem(DEFAULT_W, DEFAULT_H, cfg.chn_info[0].fmt_in, 0, &cfg.chn_info[0].mem_in);
	cfg.chn_info[0].rect_dst.x = strtol(argv[2], NULL, 10);
	cfg.chn_info[0].rect_dst.y = strtol(argv[3], NULL, 10);
	cfg.chn_info[0].rect_dst.w = strtol(argv[4], NULL, 10);
	cfg.chn_info[0].rect_dst.h = strtol(argv[5], NULL, 10);

	cfg.chn_info[1].fmt_in = SCL_FMT_RGB_PACKED;
	default_src_mem(DEFAULT_W, DEFAULT_H, cfg.chn_info[1].fmt_in, 0, &cfg.chn_info[1].mem_in);
	cfg.chn_info[1].rect_dst.x = strtol(argv[6], NULL, 10);
	cfg.chn_info[1].rect_dst.y = strtol(argv[7], NULL, 10);
	cfg.chn_info[1].rect_dst.w = strtol(argv[8], NULL, 10);
	cfg.chn_info[1].rect_dst.h = strtol(argv[9], NULL, 10);
	ret = vpss_stitch(inst, cfg);
	return ret;
}

int test_convertto(int argc, char **argv)
{
	int ret = 0;
	struct cvi_vpss_cfg cfg;
	double a[3];
	double b[3];
	enum vpss_dev inst = (enum vpss_dev)strtol(argv[1], NULL, 10);

	memset(&cfg, 0, sizeof(struct cvi_vpss_cfg));
	cfg.inst = inst;
	cfg.tile_enable = false;
	cfg.img_cfg.in = SCL_INPUT_MEM;
	cfg.img_cfg.fmt_in = SCL_FMT_RGB_PLANAR;  // SCL_FMT_RGB_PLANAR
	cfg.img_cfg.csc = SCL_CSC_NONE;
	default_src_mem(1164, 646, cfg.img_cfg.fmt_in, 0, &cfg.img_cfg.mem);

	cfg.sc_cfg.crop.x = 0;
	cfg.sc_cfg.crop.y = 0;
	cfg.sc_cfg.crop.w = 1164;
	cfg.sc_cfg.crop.h = 646;
	cfg.sc_cfg.dst.w = 1164;
	cfg.sc_cfg.dst.h = 646;
	cfg.sc_cfg.fmt_out = SCL_FMT_BF16;
	cfg.sc_cfg.csc = SCL_CSC_NONE;
	cfg.sc_cfg.flip = SCL_FLIP_NO;
	cfg.sc_cfg.algorithm = SCL_BICUBIC;
	cfg.sc_cfg.out_mode = SCL_OUT_CONVERT_TO;
	cfg.sc_cfg.datatype = strtol(argv[8], NULL, 10);//BF16 = 5, FP16 = 3, FP32 = 4, INT8 = 2, U8 = 1;
	int datasize = cfg.sc_cfg.datatype == SCL_OUT_FP32 ? 4 :
				((cfg.sc_cfg.datatype == SCL_OUT_BF16 || cfg.sc_cfg.datatype == SCL_OUT_FP16) ? 2 : 1);
	default_dst_mem(1164, 646, cfg.sc_cfg.fmt_out, 0, &cfg.sc_cfg.mem, datasize);

	a[0] = strtod(argv[2], NULL);
	a[1] = strtod(argv[3], NULL);
	a[2] = strtod(argv[4], NULL);
	b[0] = strtod(argv[5], NULL);
	b[1] = strtod(argv[6], NULL);
	b[2] = strtod(argv[7], NULL);
	cfg.sc_cfg.convert_to_cfg.a_frac[0] = a[0] < 0 ? ((u32)(-a[0] * 8192) | BIT(25)) : (u32)(a[0] * 8192);
	cfg.sc_cfg.convert_to_cfg.a_frac[1] = a[1] < 0 ? ((u32)(-a[1] * 8192) | BIT(25)) : (u32)(a[1] * 8192);
	cfg.sc_cfg.convert_to_cfg.a_frac[2] = a[2] < 0 ? ((u32)(-a[2] * 8192) | BIT(25)) : (u32)(a[2] * 8192);
	cfg.sc_cfg.convert_to_cfg.b_frac[0] = b[0] < 0 ? ((u32)(-b[0] * 8192) | BIT(25)) : (u32)(b[0] * 8192);
	cfg.sc_cfg.convert_to_cfg.b_frac[1] = b[1] < 0 ? ((u32)(-b[1] * 8192) | BIT(25)) : (u32)(b[1] * 8192);
	cfg.sc_cfg.convert_to_cfg.b_frac[2] = b[2] < 0 ? ((u32)(-b[2] * 8192) | BIT(25)) : (u32)(b[2] * 8192);
	ret = basic(&cfg);
	ret |= vpss_start(inst, cfg.tile_enable, false);

	return ret;
}


int do_move(int argc, char **argv)
{
	int ret = 0;
	u8 inst;
	u32 tmp;
	u8 num_args = 0;
	struct sclr_scale_cfg cfg;
	memset(&cfg, 0, sizeof(cfg));

	inst = strtol(argv[1], NULL, 10);

	if (inst >= SCL_MAX_INST) {
		uartlog("invalid instance-id(%d). Only 0~%d available.\n", inst, SCL_MAX_INST - 1);
		return -1;
	}

	bm_get_num("tile, 1(on)/0(off): ", cfg.tile_enable);

	// setup sc_scale
	bm_get_num("Src width(pixel num): ", cfg.src.w);
	bm_get_num("Src height(line num): ", cfg.src.h);
	bm_get_num("Crop pos-x: ", cfg.crop.x);
	bm_get_num("Crop pos-y: ", cfg.crop.y);
	bm_get_num("Crop width(pixel num): ", cfg.crop.w);
	bm_get_num("Crop height(line num): ", cfg.crop.h);
	bm_get_num("Dst width(pixel num): ", cfg.dst.w);
	bm_get_num("Dst height(line num): ", cfg.dst.h);
	bm_get_num("mir enable(1/0): ", cfg.mir_enable);

	sclr_ctrl_set_scale(inst, &cfg);
	sclr_set_scale_phase(inst, 0, 0);

	// setup img_in

	uartlog("\n  --img_in--\n");
	_handle_cfg_op(CFG_OP_IMG_CFG, inst);
	num_args = 2;
	_handle_cfg_op(CFG_OP_IMG_SET_MEM, inst, num_args, cfg.src.w, cfg.src.h);

	// setup odma
	uartlog("\n  --odma--\n");
	_handle_cfg_op(CFG_OP_ODMA_CFG, inst);
	num_args = 2;
	_handle_cfg_op(CFG_OP_ODMA_SET_MEM, inst, num_args, cfg.dst.w, cfg.dst.h);

	//struct sclr_odma_cfg *odma_cfg = sclr_odma_get_cfg(inst);
	//if (odma_cfg->mem.pitch_y)
	//	  memset((char *)dst_addr_scl[0][R_IDX], 0, odma_cfg->mem.pitch_y * odma_cfg->mem.height);

	if (cfg.tile_enable)
		sclr_left_tile(inst);

	_intr_setup(inst);
	sclr_top_set_cfg(inst, true);

	vpss_enable_mask = BIT(inst);
	sclr_trig_start(inst);

#ifndef ENV_EMU
	if (sclr_check_irq_handler(WAIT_TIME) != 0) {
		uartlog("sclr end intr doesn't happens after %d tick\n", WAIT_TIME);
		ret = -1;
		vpss_enable_mask = 0;
		//sclr_dump_top_register();
		//sclr_dump_img_in_register(img_inst);
		//sclr_dump_core_register(inst);
	} else {
		uartlog("sclr done\n");
		uartlog("sc(%d) odma status(%#x)\n", inst, sclr_odma_get_dbg_status(inst).raw);
		if (cfg.tile_enable) {
			bm_get_num("Press Enter to continue...\n", tmp);
			sclr_right_tile(inst);
			vpss_enable_mask = BIT(inst);
			sclr_trig_start(inst);
		}
	}
#endif

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

int test_realtime_fabric(int argc, char **argv)
{
	u64 vi_addr = 0x680be000;
	u8 inst = strtol(argv[1], NULL, 10);
	_reg_write_mask(vi_addr + 0x10, BIT(inst), 0);
	test_crop_resize_basic(inst, DEFAULT_W, DEFAULT_H, 0, 0, DEFAULT_W, DEFAULT_H, DEFAULT_W, DEFAULT_H, SCL_BILINEAR);
	_reg_write_mask(vi_addr + 0x10, BIT(inst), BIT(inst));
	return 0;
}

int test_pitch(int argc, char **argv)
{
	int ret = 0;
	struct cvi_vpss_cfg cfg;
	enum vpss_dev inst = (enum vpss_dev)strtol(argv[1], NULL, 10);
	int in_width, in_pitch, out_width, out_pitch, in_h;
	in_width = strtol(argv[2], NULL, 10);
	in_pitch = strtol(argv[3], NULL, 10);
	out_width = strtol(argv[4], NULL, 10);
	out_pitch = strtol(argv[5], NULL, 10);
	in_h = strtol(argv[6], NULL, 10);
	u64 in_addr = strtol(argv[7], NULL, 16);
	u64 out_addr = strtol(argv[8], NULL, 16);

	memset(&cfg, 0, sizeof(struct cvi_vpss_cfg));
	cfg.inst = inst;
	cfg.tile_enable = (in_width > 4608);
	cfg.v_tile_enable = (in_h > 8192);
	cfg.img_cfg.in = SCL_INPUT_MEM;
	cfg.img_cfg.fmt_in = SCL_FMT_RGB_PACKED;
	cfg.img_cfg.csc = SCL_CSC_NONE;
	cfg.img_cfg.mem.width = in_width;
	cfg.img_cfg.mem.height = in_h;
	cfg.img_cfg.mem.start_x = 0;
	cfg.img_cfg.mem.start_y = 0;
	cfg.img_cfg.mem.pitch_y = in_pitch;
	cfg.img_cfg.mem.pitch_c = in_pitch;
	cfg.img_cfg.mem.addr0 = in_addr;
	cfg.img_cfg.mem.addr1 = in_addr + in_pitch * in_h;
	cfg.img_cfg.mem.addr2 = in_addr + in_pitch * in_h;

	cfg.sc_cfg.crop.x = 0;
	cfg.sc_cfg.crop.y = 0;
	cfg.sc_cfg.crop.w = in_width;
	cfg.sc_cfg.crop.h = in_h;
	cfg.sc_cfg.dst.w = out_width;
	cfg.sc_cfg.dst.h = in_h;
	cfg.sc_cfg.fmt_out = SCL_FMT_RGB_PACKED;
	cfg.sc_cfg.out_mode = SCL_OUT_DISABLE;
	cfg.sc_cfg.csc = SCL_CSC_NONE;
	cfg.sc_cfg.flip = SCL_FLIP_NO;
	cfg.sc_cfg.algorithm = SCL_BICUBIC;
	cfg.sc_cfg.mem.width = out_width;
	cfg.sc_cfg.mem.height = in_h;
	cfg.sc_cfg.mem.start_x = 0;
	cfg.sc_cfg.mem.start_y = 0;
	cfg.sc_cfg.mem.pitch_y = out_pitch;
	cfg.sc_cfg.mem.pitch_c = out_pitch;
	cfg.sc_cfg.mem.addr0 = out_addr;
	cfg.sc_cfg.mem.addr1 = out_addr + out_pitch * in_h;
	cfg.sc_cfg.mem.addr2 = out_addr + out_pitch * in_h;

	ret = basic(&cfg);
	ret |= vpss_start(inst, cfg.tile_enable, cfg.v_tile_enable);
	return ret;
}

int test_src_pitch(int argc, char **argv)
{
	int ret = 0;
	struct cvi_vpss_cfg cfg;
	enum vpss_dev inst = strtol(argv[1], NULL, 10);
	int in_width, in_pitch, out_width, out_pitch, in_h;
	in_width = strtol(argv[3], NULL, 10);
	in_pitch = strtol(argv[4], NULL, 10);
	out_width = 800;
	out_pitch = 2400;
	in_h = 600;
	u64 in_addr = strtol(argv[6], NULL, 16);
	u64 out_addr = 0x103800000;

	memset(&cfg, 0, sizeof(struct cvi_vpss_cfg));
	cfg.inst = inst;
	cfg.tile_enable = (in_width > 4608);
	cfg.v_tile_enable = (in_h > 8189);
	cfg.img_cfg.in = SCL_INPUT_MEM;
	cfg.img_cfg.fmt_in = strtol(argv[2], NULL, 10);
	cfg.img_cfg.csc = SCL_CSC_601_LIMIT_YUV2RGB;
	cfg.img_cfg.mem.width = in_width;
	cfg.img_cfg.mem.height = in_h;
	cfg.img_cfg.mem.start_x = 0;
	cfg.img_cfg.mem.start_y = 0;
	cfg.img_cfg.mem.pitch_y = in_pitch;
	cfg.img_cfg.mem.pitch_c = strtol(argv[5], NULL, 10);
	cfg.img_cfg.mem.addr0 = in_addr;
	cfg.img_cfg.mem.addr1 = strtol(argv[7], NULL, 16);
	cfg.img_cfg.mem.addr2 = strtol(argv[8], NULL, 16);

	cfg.sc_cfg.crop.x = 0;
	cfg.sc_cfg.crop.y = 0;
	cfg.sc_cfg.crop.w = in_width;
	cfg.sc_cfg.crop.h = in_h;
	cfg.sc_cfg.dst.w = out_width;
	cfg.sc_cfg.dst.h = in_h;
	cfg.sc_cfg.fmt_out = SCL_FMT_RGB_PACKED;
	cfg.sc_cfg.out_mode = SCL_OUT_DISABLE;
	cfg.sc_cfg.csc = SCL_CSC_NONE;
	cfg.sc_cfg.flip = SCL_FLIP_NO;
	cfg.sc_cfg.algorithm = SCL_BICUBIC;
	cfg.sc_cfg.mem.width = out_width;
	cfg.sc_cfg.mem.height = in_h;
	cfg.sc_cfg.mem.start_x = 0;
	cfg.sc_cfg.mem.start_y = 0;
	cfg.sc_cfg.mem.pitch_y = out_pitch;
	cfg.sc_cfg.mem.pitch_c = out_pitch;
	cfg.sc_cfg.mem.addr0 = out_addr;
	cfg.sc_cfg.mem.addr1 = out_addr + out_pitch * in_h;
	cfg.sc_cfg.mem.addr2 = out_addr + out_pitch * in_h + cfg.sc_cfg.mem.pitch_c * in_h;

	ret = basic(&cfg);
	ret |= vpss_start(inst, cfg.tile_enable, cfg.v_tile_enable);
	return ret;
}

int close_vi_clk(int argc, char **argv){
	enum vpss_dev inst = (enum vpss_dev)strtol(argv[1], NULL, 10);
	u64 clk_addr = 0x28102000;
	u64 vi_addr = 0x680be000;
	u64 reg_enable_addr[4] = {0x178, 0x17c, 0x17c, 0x17c};
	u64 reg_enable_mask[4] = {BIT(31), BIT(0), BIT(1), BIT(2)};
	u64 reg_apb_en_addr = 0x14;
	u64 reg_apb_en_mask[4] = {BIT(12), BIT(13), BIT(14), BIT(15)};
	u64 reg_auto_gate_addr = 0x1c;
	u64 reg_auto_gate_mask[4] = {BIT(16), BIT(17), BIT(18), BIT(19)};

	//close clk
	_reg_write_mask(clk_addr + reg_enable_addr[inst], reg_enable_mask[inst], 0);
	_reg_write_mask(vi_addr + reg_apb_en_addr, reg_apb_en_mask[inst], 0);
	_reg_write_mask(vi_addr + reg_auto_gate_addr, reg_auto_gate_mask[inst], reg_auto_gate_mask[inst]);
	uartlog("close clk: %d", inst);

	return 0;
}

int vivo_vpss_clk_gen_ctrl(int argc, char **argv)
{
	u64 sys_ctrl_addr = 0x03000000;
	u64 clk_gen_offset = 0x2000;
	u64 reg_enable_offfset = 0xf0;
	u64 mask = 0b111100000000000;
	int var = strtol(argv[1], NULL, 10);

	//close clk
	_reg_write_mask(sys_ctrl_addr + clk_gen_offset + reg_enable_offfset, mask, var);

	uartlog("close vpss clk_gen");
	return 0;
}

int open_vi_clk(int argc, char **argv){
	enum vpss_dev inst = (enum vpss_dev)strtol(argv[1], NULL, 10);
	u64 clk_addr = 0x28102000;
	u64 vi_addr = 0x680be000;
	u64 reg_enable_addr[4] = {0x178, 0x17c, 0x17c, 0x17c};
	u64 reg_enable_mask[4] = {BIT(31), BIT(0), BIT(1), BIT(2)};
	u64 reg_apb_en_addr = 0x14;
	u64 reg_apb_en_mask[4] = {BIT(12), BIT(13), BIT(14), BIT(15)};
	u64 reg_auto_gate_addr = 0x1c;
	u64 reg_auto_gate_mask[4] = {BIT(16), BIT(17), BIT(18), BIT(19)};

	//open clk
	_reg_write_mask(clk_addr + reg_enable_addr[inst], reg_enable_mask[inst], reg_enable_mask[inst]);
	_reg_write_mask(vi_addr + reg_apb_en_addr, reg_apb_en_mask[inst], reg_apb_en_mask[inst]);
	_reg_write_mask(vi_addr + reg_auto_gate_addr, reg_auto_gate_mask[inst], 0);

	//read
	uartlog("open clk: %d", inst);
	return 0;
}

int set_wait_time(int argc, char **argv)
{
  WAIT_TIME = strtoul(argv[1], NULL, 10);
  printf("WAIT_TIME = %d\n", WAIT_TIME);
  return 0;
}

/*
void ldc_get_sb_default(struct ldc_sb_cfg *cfg)
{
	memset(cfg, 0, sizeof(*cfg));
}

void ldc_set_sb(struct ldc_sb_cfg *cfg)
{
	_reg_write(reg_base + REG_SB_MODE,    cfg->sb_mode);
	_reg_write(reg_base + REG_SB_NB,      cfg->sb_nb);
	_reg_write(reg_base + REG_SB_SIZE,    cfg->sb_size);
	_reg_write(reg_base + REG_SB_SW_CLR,  1);
	_reg_write(reg_base + REG_SB_SW_WPTR, cfg->sb_sw_wptr);
	_reg_write(reg_base + REG_SB_SET_STR, cfg->sb_set_str);

	_reg_write(reg_base + REG_LDC_DIR, cfg->ldc_dir);
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
}*/

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

#if 0
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
	enum sclr_input in;
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
	in = SCL_INPUT_MEM;	// still from dram

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

	sclr_top_reg_done(inst);
	_intr_setup(inst);

	// CFG_OP_IMG_START, still need to kick scaler
	sclr_trig_start_ext(img_inst, inst);
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


// 320x256 + YUV420 + sb freerun = 2 + 64line + 3 frames
int ldc_to_sclr_sb_test(int argc, char **argv)
{
	int ret = 0;
	u8 inst = 1; // sc_v1
	enum vpss_dev img_inst = (enum vpss_dev)strtol(argv[1], NULL, 10);
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
	enum vpss_dev img_inst = (enum vpss_dev)strtol(argv[1], NULL, 10);
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
	enum vpss_dev img_inst = (enum vpss_dev)strtol(argv[1], NULL, 10);
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
#endif


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
	enum sclr_input in;
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
	in = SCL_INPUT_MEM;	// still from dram

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
	sclr_set_crop(inst, sc_cfg.crop, true);
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

	sclr_top_reg_done(inst);
	_intr_setup(inst);

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
	enum vpss_dev img_inst = (enum vpss_dev)strtol(argv[1], NULL, 10);

	sclr_set_sclr_to_vc_sb(inst);
	ret = sclr_to_vc_sb_test_start_sclr(inst, img_inst);

	return ret;
}

int test_frc(int argc, char **argv)
{
	int ret = 0;
	struct cvi_vpss_cfg cfg;
	enum vpss_dev inst = (enum vpss_dev)strtol(argv[1], NULL, 10);

	memset(&cfg, 0, sizeof(struct cvi_vpss_cfg));
	cfg.inst = inst;
	cfg.tile_enable = false;
	cfg.img_cfg.in = SCL_INPUT_MEM;
	cfg.img_cfg.fmt_in = SCL_FMT_RGB_PACKED;
	cfg.img_cfg.csc = SCL_CSC_NONE;
	cfg.img_cfg.is_skip_en = 1;
	default_src_mem(DEFAULT_W, DEFAULT_H, cfg.img_cfg.fmt_in, 0, &cfg.img_cfg.mem);

	cfg.sc_cfg.crop.x = 0;
	cfg.sc_cfg.crop.y = 0;
	cfg.sc_cfg.crop.w = DEFAULT_W;
	cfg.sc_cfg.crop.h = DEFAULT_H;
	cfg.sc_cfg.dst.w = DEFAULT_W;
	cfg.sc_cfg.dst.h = DEFAULT_H;
	cfg.sc_cfg.fmt_out = SCL_FMT_RGB_PACKED;
	cfg.sc_cfg.out_mode = SCL_OUT_DISABLE;
	cfg.sc_cfg.csc = SCL_CSC_NONE;
	cfg.sc_cfg.flip = SCL_FLIP_NO;
	cfg.sc_cfg.algorithm = SCL_BICUBIC;
	default_dst_mem(DEFAULT_W, DEFAULT_H, cfg.sc_cfg.fmt_out, 0, &cfg.sc_cfg.mem, 1);

	ret = basic(&cfg);
	ret |= vpss_start(inst, cfg.tile_enable, false);
	return ret;
}

int nv12_to_other(int in_w, int in_h, int out_w, int out_h, enum sclr_format fmt_in, enum sclr_format fmt_out, u8 src_idx)
{
	int ret = 0;
	struct cvi_vpss_cfg cfg;
	enum vpss_dev inst = 0;
	u8 csc_type = SCL_CSC_601_LIMIT_YUV2RGB;

	if (csc_type < 0 || csc_type > 4) {
		uartlog("param error\n");
		return -1;
	}

	memset(&cfg, 0, sizeof(struct cvi_vpss_cfg));
	cfg.inst = inst;
	cfg.tile_enable = (in_w > 4608);
	cfg.img_cfg.in = SCL_INPUT_MEM;
	cfg.img_cfg.fmt_in = fmt_in;
	default_src_mem(in_w, in_h, cfg.img_cfg.fmt_in, src_idx, &cfg.img_cfg.mem);
	if (IS_YUV_FMT(cfg.img_cfg.fmt_in))
		cfg.img_cfg.csc = csc_type;
	else
		cfg.img_cfg.csc = SCL_CSC_NONE;

	cfg.sc_cfg.crop.x = 0;
	cfg.sc_cfg.crop.y = 0;
	cfg.sc_cfg.crop.w = in_w;
	cfg.sc_cfg.crop.h = in_h;
	cfg.sc_cfg.dst.w = out_w;
	cfg.sc_cfg.dst.h = out_h;
	cfg.sc_cfg.fmt_out = fmt_out;
	cfg.sc_cfg.flip = SCL_FLIP_NO;
	cfg.sc_cfg.algorithm = SCL_BICUBIC;
	default_dst_mem(cfg.sc_cfg.dst.w, cfg.sc_cfg.dst.h, cfg.sc_cfg.fmt_out, 0, &cfg.sc_cfg.mem, 1);
	if (IS_YUV_FMT(cfg.sc_cfg.fmt_out)) {
		cfg.sc_cfg.out_mode = SCL_OUT_CSC;
		cfg.sc_cfg.csc = csc_type + 4;
	} else {
		cfg.sc_cfg.out_mode = SCL_OUT_DISABLE;
		cfg.sc_cfg.csc = SCL_CSC_NONE;
	}
	ret = basic(&cfg);
	ret |= vpss_start(inst, cfg.tile_enable, false);
	return ret;
}

int test_nv12_to_other(int argc, char **argv)
{
	for (int j = 0; j < 100; j++) {
		for (int i = 0; i < 16; ++i) {
			if (0 != nv12_to_other(800, 600, 800, 600, SCL_FMT_NV12, SCL_FMT_RGB_PLANAR, 7)) {
				uartlog("test_nv12_to_other fail!\n");
				return -1;
			}
			if (0 != nv12_to_other(800, 600, 800, 600, g_fmt_in[i], SCL_FMT_RGB_PLANAR, i)) {
				uartlog("test_nv12_to_other fail!\n");
				return -1;
			}
		}
	}

	uartlog("test_nv12_to_other pass!\n");

	return 0;
}

static struct cmd_entry test_cmd_list[] __attribute__ ((unused)) = {
	{"\n[---SCLR------------------------------------------------]", do_show_addr, 0, ""},
	{"show_addr_s", 		do_show_addr,		  0, "[Show sclr's buffer address]"},
	{"cfg", 				do_cfg, 			  0, "[Modify settings]"},
	{"move_s",				do_move,			  1, "[do sc's move with scaling/crop/csc] 1st param: instance(0~9) "},
	{"cmdQ_s",				test_cmdq,			  5, "[param:inst, option, addr, size, pattern]"},
	{"vpss_basic", 			test_vpss_basic,      1, "[param:inst, basic test of vpss]"},
	{"battery", 			test_multi_channel,   2, "[param:dev_num, vpss start inst]"},
	{"battery_performance", test_multi_channel_performance,9, "[param:dev_num, vpss start inst, in_w, in_h, out_w, out_h, in_fmt, out_fmt, vgop_enable]"},
	{"battery_mask", 		test_multi_mask,   	  3, "[param:dev_num, vpss start inst, mask_dev]"},
	{"crop_resize",			test_crop_resize,    10, "[param:inst, in_w, in_h, crop_x, crop_y, crop_w, crop_h, dst_w, dst_h, algorithm]"},
	{"img_crop_resize",		test_img_crop_resize,10, "[param:inst, in_w, in_h, crop_x, crop_y, crop_w, crop_h, dst_w, dst_h, algorithm]"},
	{"crop_0", 				test_crop_0,	      1, "[param:inst]"},
	{"crop_1",				test_crop_1,		  1, "[param:inst]"},
	{"resize_0",			test_resize_0,		  1, "[param:inst]"},
	{"resize_1",			test_resize_1,		  1, "[param:inst]"},
	{"max_size", 			test_max_size,		  3, "[param:inst, in_fmt, out_fmt]"},
	{"hsv",					test_hsv,			  2, "[param:inst, is_packed]"},
	{"datatype",			test_datatype,		  2, "[param:inst, datatype]"},
	{"csc_resize",			test_csc_resize,	  8, "[param:inst, src_fmt, dst_fmt, in_w, in_h, out_w, out_h]"},
	{"csc", 				test_csc,			  4, "[param:inst, src_fmt, dst_fmt, csc_type(1: 601 limit 2: 601 full 3: 701 limit 4: 701 full)]"},
	{"mirror",				test_mirror,		  1, "[param:inst]"},
	{"flip",				test_flip,			  1, "[param:inst]"},
	{"mirror_flip",			test_mirror_flip,	  1, "[param:inst]"},
	{"tile_mode",			test_tile_mode,		  1, "[param:inst]"},
	{"crop_tile",			test_crop_tile,		 10, "[param:inst, in_w, in_h, crop_x, crop_y, crop_w, crop_h, out_w, out_h, algorithm]"},
	{"privacy_mask",		test_privacy_mask,	  6, "[param:inst, grid_mode, grid_size, mask_rgb332, stx, sty]"},
	{"mask_complex",		privacy_mask_complex, 9, "[param:inst, grid_mode, grid_size, mask_rgb332, force_alpha, alpha_factor, no_mask_idx, y2b_enable, is_mem]"},
	{"circle",				test_circle,		  2, "[param:inst, cir_mode]"},
	{"border",				test_border,		  5, "[param:inst, stx, sty, w, h]"},
	{"border_vpp",			test_border_vpp,	  1, "[param:inst]"},
	{"rectangle",			test_draw_rectangle,  9, "[param:inst, in_stx, in_sty, in_endx, in_endy, out_stx, out_sty, out_endx, out_endy]"},
	{"quant",				test_quant, 		  9, "[param:inst, fac_r, fac_g, fac_b, mean_r, mean_g, mean_b, round, datatype]"},
	{"convertto",			test_convertto, 	 10, "[param:inst, a_r, a_b, a_b, b_r, b_g, b_b, datatype, u_addr, v_addr]"},
	{"realtime",			test_realtime_fabric, 1, "[param:inst]"},
	{"cover",				test_cover,		  	  1, "[param:inst]"},
	{"fill_rectangle",		test_fill_rectangle,  5, "[param:inst, stx, sty, w, h]"},
	{"src_pitch",			test_src_pitch,		  8, "[param:inst, in_fmt, in_width, in_pitch_y, in_pitch_c, inaddr0, inaddr1, inaddr2]"},
	{"pitch",				test_pitch,			  8, "[param:inst, in_width, in_pitch, out_width, out_pitch, h, in_addr, out_addr]"},
	{"upsample",			test_upsample,		  6, "[param:inst, src_fmt, dst_fmt, in_w, in_h]"},
	{"field",				test_stitch_field,	  1, "[param:inst]"},
	{"stitch",				test_stitch,	  	  9, "[param:inst, stx0, sty0, w0, h0, stx1, sty1, w1, h1]"},
	{"soft_reset",			soft_reset,	  	  	  1, "[param:inst]"},
	{"gop",					test_gop,	  	  	  8, "[param:inst, fmt_in, w, h, stx, sty, gop_w, gop_h]"},
	{"open_viclk",			open_vi_clk,	  	  1, "[param:inst]"},
	{"close_viclk",			close_vi_clk,	  	  1, "[param:inst]"},
	{"all",					test_open_all,	  	  1, "[param:inst]"},
	{"set_wait",			set_wait_time,	  	  1, "[param:write_time]"},
	{"reset",				test_reset,	  		  1, "[param:inst]"},
	{"stopper_reset",		test_stopper_reset,	  1, "[param:inst]"},
	{"stopper_reset_1",		test_stopper_reset_1, 1, "[param:inst]"},
	{"vpss_enable", 		test_vpss_enable,	  2, "[param:inst, status]"},
	{"vpss_mask", 			test_vpss_mask,		  2, "[param:inst, status]"},
	{"csc_ds", 				test_csc_ds,		  4, "[param:inst]"},
	{"vo_clk", 				test_clk,		  1, "[param:inst]"},
	{"vpss3_config", 		vpss3_config,		  0, "[param:]"},
	{"trigger_single", 		vpss3_trigger_single, 0, "[param:]"},
	{"trigger_multi", 		vpss3_trigger_multi,  0, "[param:]"},
	{"sclr_to_vc_sb_test" ,	sclr_to_vc_sb_test,	  0, "[param:] slice buffer: scaler to vc, 352x288 nv21 -> 352x288 nv21]"},
	{"frame_rate_ctrl", 	test_frc,			  0, "[param:] drop frame no need"},
	{"nv12_to_other", 		test_nv12_to_other,	  0, "[param:]"},
	{"vivo_clk_ctrl",		vivo_vpss_clk_gen_ctrl, 0, "[param:] test vpss clk_gen"},
	{NULL, NULL, 0, NULL}
};

int testcase_vpss(void)
{
	int i = 0, ret;

	union sclr_intr mask;
	u32 irq_num[SCL_MAX_INST] = {VIVO_INT_VPSS0, VIVO_INT_VPSS1};
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

