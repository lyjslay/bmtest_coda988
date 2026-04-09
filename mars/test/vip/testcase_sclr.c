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
#include "dsi_phy.h"
#include "disp_timing.h"
#include "cmdq.h"
#include "ldc.h"
#include "panels.h"
#include "sclr_test.h"

#define EPSILON_SCALE    1000000 // 1/0.000001

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
	CFG_OP_DISP_CFG = 40,
	CFG_OP_DISP_SRC,
	CFG_OP_DISP_TGEN,
	CFG_OP_LVDSTX,
	CFG_OP_BT,
	CFG_OP_DSI_ILI9881C_INIT,
	CFG_OP_DISP_GAMMA,
	CFG_OP_DSI_HX8394_INIT,
	CFG_OP_DSI_LP_LOOP,
	CFG_OP_DSI_DSC_CMD,
	CFG_OP_DISP_CHECKSUM,
	CFG_OP_STATUS_ALL = 60,
	CFG_OP_STATUS_TOP,
	CFG_OP_STATUS_IMG_IN,
	CFG_OP_STATUS_SC,
	CFG_OP_STATUS_GOP,
	CFG_OP_STATUS_ODMA,
	CFG_OP_STATUS_DISP,
	CFG_OP_STATUS_LVDSTX,
	CFG_OP_REG_DUMP = 70,
	CFG_OP_TOP_REG_DUMP,
	CFG_OP_IMG_IN_REG_DUMP,
	CFG_OP_SC_CORE_DUMP,
	CFG_OP_SC_ODMA_DUMP,
	CFG_OP_SC_DISP_DUMP,
	CFG_OP_PANEL_TYPE,
	CFG_OP_PANEL_TTL_RGB_TEST,
	CFG_OP_PANEL_FRD_RGB_TEST,
	CFG_OP_PANEL_FRD_RGB_3_SERIAL_TEST,
	CFG_OP_PANEL_FRD_MCU_TEST_CFG,//80
	CFG_OP_PANEL_TXDT_RGB_4_SERIAL_TEST,
	CFG_OP_PANEL_FRD_HW_MCU_TEST_CFG_ON_FPGA,//82
	CFG_OP_PANEL_FRD_HW_SET_2C,
	CFG_OP_PANEL_FRD_HW_SET_TRIG,
	CFG_OP_PANEL_FRD_HW_STEP7,//85
	CFG_OP_PANEL_FRD_HW_STEP8,//86
	CFG_OP_PANEL_FRD_HW_STEP9,//87
	CFG_OP_PANEL_FRD_HW_STEP10,//88
	CFG_OP_PANEL_FRD_HW_SET_2C_ONLY,//89
	CFG_OP_PANEL_FRD_HW_MCU_TEST_CFG_ON_EVB,//90
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
	u32 fourcc;          /* v4l2 format id */
	enum sclr_format fmt;
	u8  buffers;
	u32 bit_depth[3];
	u8 plane_sub_h;
	u8 plane_sub_v;
};

const struct cvi_vip_fmt cvi_vip_formats[] = {
	{
	.fmt         = SCL_FMT_YUV420,
	.bit_depth   = { 8, 2, 2 },
	.buffers     = 3,
	.plane_sub_h = 2,
	.plane_sub_v = 2,
	},
	{
	.fmt         = SCL_FMT_YUV422,
	.bit_depth   = { 8, 4, 4 },
	.buffers     = 3,
	.plane_sub_h = 2,
	.plane_sub_v = 1,
	},
	{
	.fmt         = SCL_FMT_NV12,
	.bit_depth   = { 8, 4, 0 },
	.buffers     = 2,
	.plane_sub_h = 2,
	.plane_sub_v = 2,
	},
	{
	.fmt         = SCL_FMT_NV21,
	.bit_depth   = { 8, 4, 0 },
	.buffers     = 2,
	.plane_sub_h = 2,
	.plane_sub_v = 2,
	},
	{
	.fmt         = SCL_FMT_YUV422SP2,
	.bit_depth   = { 8, 8, 0 },
	.buffers     = 2,
	.plane_sub_h = 1,
	.plane_sub_v = 1,
	},
	{
	.fmt         = SCL_FMT_YUV422SP1,
	.bit_depth   = { 8, 8, 0 },
	.buffers     = 2,
	.plane_sub_h = 1,
	.plane_sub_v = 1,
	},
	{
	.fmt         = SCL_FMT_YUYV,
	.bit_depth   = { 16 },
	.buffers     = 1,
	.plane_sub_h = 1,
	.plane_sub_v = 1,
	},
	{
	.fmt         = SCL_FMT_YVYU,
	.bit_depth   = { 16 },
	.buffers     = 1,
	.plane_sub_h = 1,
	.plane_sub_v = 1,
	},
	{
	.fmt         = SCL_FMT_UYVY,
	.bit_depth   = { 16 },
	.buffers     = 1,
	.plane_sub_h = 1,
	.plane_sub_v = 1,
	},
	{
	.fmt         = SCL_FMT_VYUY,
	.bit_depth   = { 16 },
	.buffers     = 1,
	.plane_sub_h = 1,
	.plane_sub_v = 1,
	},
	{
	.fmt         = SCL_FMT_RGB_PLANAR,
	.bit_depth   = { 8, 8, 8 },
	.buffers     = 3,
	.plane_sub_h = 1,
	.plane_sub_v = 1,
	},
	{
	.fmt         = SCL_FMT_RGB_PACKED,
	.bit_depth   = { 24 },
	.buffers     = 1,
	.plane_sub_h = 1,
	.plane_sub_v = 1,
	},
	{
	.fmt         = SCL_FMT_BGR_PACKED,
	.bit_depth   = { 24 },
	.buffers     = 1,
	.plane_sub_h = 1,
	.plane_sub_v = 1,
	},
	{
	.fmt         = SCL_FMT_Y_ONLY,
	.bit_depth   = { 8 },
	.buffers     = 1,
	.plane_sub_h = 1,
	.plane_sub_v = 1,
	},
	{
	.fmt         = SCL_FMT_BGR_PACKED,
	.bit_depth   = { 24 },
	.buffers     = 1,
	.plane_sub_h = 1,
	.plane_sub_v = 1,
	},
	{
	.fmt         = SCL_FMT_RGB_PLANAR,
	.bit_depth   = { 8, 8, 8 },
	.buffers     = 3,
	.plane_sub_h = 1,
	.plane_sub_v = 1,
	},
};

static u8 sc_gamma_data0[] = {
    0,  1,  1,  1,  1,  1,  2,  2,  2,  2,  2,  3,  3,  3,  3,  4,
    4,  5,  5,  5,  6,  7,  7,  8,  9,  9,  10, 11, 12, 13, 14, 16,
    17, 19, 21, 22, 24, 27, 29, 32, 35, 38, 41, 45, 49, 54, 58, 64,
    70, 76, 83, 90, 99,107,117,128,139,152,166,181, 197,215,235,255,
    255
};

static u8 sc_gamma_data1[] = {
    0,  4,  8, 12, 16, 20, 24, 28, 32, 36, 40, 44, 48, 52, 56, 60,
   64, 68, 72, 76, 80, 84, 88, 92, 96,100,104,108,112,116,120,124,
  128,132,136,140,144,148,152,156,160,164,168,172,176,180,184,188,
  192,196,200,204,208,212,216,220,224,228,232,236,240,244,248,252,
  255
};

static char* str_sclr_fmt[] = {"YUV420", "YUV422", "RGB_PLANAR", "RGB_PACKED", "BGR_PACKED", "Y_ONLY"};
static char* str_sclr_csc[] = {"Disable", "2RGB_601_Limit", "2RGB_601_Full", "2RGB_709_Limit", "2RGB_709_Full",
        "2YUV_601_Limit", "2YUV_601_Full", "2YUV_709_Limit", "2YUV_709_Full"};
static char* str_sclr_flip[] = {"No", "HFLIP", "VFLIP", "HVFLIP"};
static char* str_sclr_odma_mode[] = {"CSC", "QUANT", "HSV", "BF16", "DISABLE"};
static char* str_sclr_gop_fmt[] = {"ARGB8888", "ARGB4444", "ARGB1555", "256LUT", "16LUT", "FONT"};
static unsigned char *in=NULL;
static unsigned char *out=NULL;

u64 src_addr_scl[3][3] = {
    {BASE_OFFSET + 0x00800000, BASE_OFFSET + 0x00C00000, BASE_OFFSET + 0x01000000},
    {BASE_OFFSET + 0x01400000, BASE_OFFSET + 0x01800000, BASE_OFFSET + 0x01C00000},
    {BASE_OFFSET + 0x02000000, BASE_OFFSET + 0x02400000, BASE_OFFSET + 0x02800000},
};

u64 dst_addr_scl[3][3] = {
    {BASE_OFFSET + 0x03000000, BASE_OFFSET + 0x03400000, BASE_OFFSET + 0x03800000},
    {BASE_OFFSET + 0x03C00000, BASE_OFFSET + 0x04000000, BASE_OFFSET + 0x04400000},
    {BASE_OFFSET + 0x04800000, BASE_OFFSET + 0x04C00000, BASE_OFFSET + 0x05000000},
};

static u64 gop_addr = BASE_OFFSET + 0x06000000;
static u64 gop_LUT_addr = BASE_OFFSET + 0x05400000;
u64 cmdq_set = BASE_OFFSET + 0x06800000;
static u32 gop_space[SCL_MAX_INST + 1] = {GOP_FRAME_OFFSET, GOP_FRAME_OFFSET, GOP_FRAME_OFFSET, GOP_FRAME_OFFSET}; // the remaining dram for gop, count down like stack

static bool enable_cmdq = false;
u64 sclr_timer_tick = 0;
volatile int sclr_mark;
static volatile union sclr_intr intr_mask;
static bool sc_init = false;
static u32 count=0;
unsigned char i80_ctrl[I80_CTRL_MAX] = { 0x31, 0x75, 0xff };
static u32 enable_intr_msg = 1;

void sclr_enable_intr_log(u32 enable)
{
	enable_intr_msg = enable;
}

void _get_bytesperline(enum sclr_format sc_fmt, u16 width, u16 bytesperline[2])
{
	const struct cvi_vip_fmt *fmt;

	memset(bytesperline, 0, sizeof(bytesperline[0] * 2));
	for (int k = 0; k < ARRAY_SIZE(cvi_vip_formats); k++) {
		fmt = &cvi_vip_formats[k];
		if (fmt->fmt == sc_fmt)
			break;
	}

	for (int p = 0; p < fmt->buffers; p++) {
		u8 plane_sub_h = (p == 0) ? 1 : fmt->plane_sub_h;
		/* Calculate the minimum supported bytesperline value */
		bytesperline[p] = VIP_ALIGN((width * fmt->bit_depth[p] * plane_sub_h) >> 3);
	}
	uartlog("fmt(%d) num_planes(%d) pitch(%d %d)\n", sc_fmt, fmt->buffers, bytesperline[0], bytesperline[1]);
}

void sclr_trig_start(int inst)
{
    struct sclr_top_cfg *cfg = sclr_top_get_cfg();

    sclr_mark = 0;
    sclr_timer_tick = timer_get_tick();
    if (inst == SCL_IMG_D) {
	cfg->sclr_enable[0] = true;
	cfg->disp_enable = true;
    } else {
	cfg->sclr_enable[1] = true;
	cfg->sclr_enable[2] = true;
	cfg->sclr_enable[3] = true;
    }
    sclr_top_set_cfg(cfg);

    sclr_img_start(inst);
}

bool is_sclr_job_done(int inst)
{
    union sclr_intr intr_status = sclr_intr_status();
    bool is_done = false;

    if (inst == SCL_IMG_D) {
	is_done = (intr_status.b.img_in_d_frame_start == 0)
		|| (intr_status.b.img_in_d_frame_end == 1);
    } else {
	is_done = (intr_status.b.img_in_v_frame_start == 0)
		|| (intr_status.b.img_in_v_frame_end == 1);
    }
    return is_done;
}

static void _i80_package_eol(unsigned char buffer[3])
{
	buffer[0] = 0xff;
	buffer[1] = i80_ctrl[I80_CTRL_EOF];
	buffer[2] = I80_OP_GO;
}

static void _i80_package_eof(unsigned char buffer[3])
{
	buffer[0] = 0x00;
	buffer[1] = i80_ctrl[I80_CTRL_EOF];
	buffer[2] = I80_OP_DONE;
}

unsigned int _MAKECOLOR(unsigned char r, unsigned char g, unsigned char b, enum _VO_I80_FORMAT fmt)
{
	unsigned char r1, g1, b1;
	unsigned char r_len, g_len, b_len;

	switch (fmt) {
	case VO_I80_FORMAT_RGB444:
		r_len = 4;
		g_len = 4;
		b_len = 4;
		break;

	default:
	case VO_I80_FORMAT_RGB565:
		r_len = 5;
		g_len = 6;
		b_len = 5;
		break;

	case VO_I80_FORMAT_RGB666:
		r_len = 6;
		g_len = 6;
		b_len = 6;
		break;
	}

	r1 = r >> (8 - r_len);
	g1 = g >> (8 - g_len);
	b1 = b >> (8 - b_len);
	//printf("_MAKECOLOR,R(%d), G(%d), B(%d), return pixel(%x)",r,g,b,b1 | (g1 << b_len) | (r1 << (b_len + g_len)));
	return (b1 | (g1 << b_len) | (r1 << (b_len + g_len)));
}

static void _i80_package_rgb(unsigned char r, unsigned char g, unsigned char b,
			     unsigned char *buffer, unsigned char byte_cnt)
{
	unsigned int pixel;

	pixel = _MAKECOLOR(r, g, b, VO_I80_FORMAT_RGB565);

	for (unsigned char i = 0, offset = 0; i < byte_cnt; ++i) {
		*(buffer + offset++) = pixel >> ((byte_cnt - i - 1) << 3);
		//printf("_i80_package_rgb DATA[%x]\n",i80_ctrl[I80_CTRL_DATA]);
		*(buffer + offset++) = i80_ctrl[I80_CTRL_DATA];
		*(buffer + offset++) = I80_OP_GO;
	}
}

int i80_set_cmd(unsigned int cmd)
{
	int ret = 0;

	sclr_i80_packet(cmd);

	return ret;
}

int i80_set_hw_cmd(unsigned int cmd)
{
	int ret = 0;

	sclr_i80_hw_packet(cmd);

	return ret;
}

int i80_init(const struct _VO_I80_INSTR_S *cmds, int size)
{
	int ret = 0;
	unsigned int sw_cmd;

	for (int i = 0; i < size; i++) {
		sw_cmd = (i80_ctrl[cmds[i].data_type] << 8) | cmds[i].data;
		i80_set_cmd(sw_cmd);

		if (cmds[i].delay)
			mdelay(cmds[i].delay);
	}
	return ret;
}

int i80_hw_init(const struct _VO_I80_INSTR_S *cmds, int size)
{
	int ret = 0;
	unsigned int sw_cmd0,sw_cmd1,sw_cmd2,sw_cmd3;
	int cmd_cnt=0;

	printf("ArraySize[%d]\n",size);
	for (int i = 0; i < size; i=i+4) {
		cmd_cnt = 0;
		if(i<size)
		{
			cmd_cnt++;
			sw_cmd0 = (cmds[i].data_type << 8) | cmds[i].data;
			sclr_i80_set_cmd0(sw_cmd0);
		}
		if((i+1)<size)
		{
			cmd_cnt++;
			sw_cmd1 = (cmds[i+1].data_type << 8) | cmds[i+1].data;
			sclr_i80_set_cmd1(sw_cmd1);
		}

		if((i+2)<size)
		{
			cmd_cnt++;
			sw_cmd2 = (cmds[i+2].data_type << 8) | cmds[i+2].data;
			sclr_i80_set_cmd2(sw_cmd2);
		}
		if((i+3)<size)
		{
			cmd_cnt++;
			sw_cmd3 = (cmds[i+3].data_type << 8) | cmds[i+3].data;
			sclr_i80_set_cmd3(sw_cmd3);
		}
		//printf("set cmd cnt [%d]\n",cmd_cnt);
		sclr_i80_set_cmd_cnt(cmd_cnt);

		sclr_i80_trig();
	}

	return ret;
}

void i80_set_sw_mode(unsigned long mode_flags)
{
	if (mode_flags)
		sclr_i80_sw_mode(SCLR_I80_MODE_SW);
	else
		sclr_i80_sw_mode(SCLR_I80_MODE_IDLE);
}

int i80_set_run(void)
{
	int ret = 0;

	sclr_i80_run();

	return ret;
}

void i80_sclr_intr_clr(void)
{
	sclr_intr_clr(sclr_intr_status());
}

float i80_get_bytepp(const struct _i80_cfg_s *i80_cfg)
{
	enum _VO_I80_FORMAT fmt = i80_cfg->fmt;

	switch (fmt) {
	case VO_I80_FORMAT_RGB444:
		return 1.5;
	case VO_I80_FORMAT_RGB565:
		return 2;
	case VO_I80_FORMAT_RGB666:
		return 3;
	default:
		return 0;
	}
}

void i80_set_combo_dev_cfg(void)
{
	unsigned int clkkHz = 0;
	unsigned short i80_cycle_time = 198;
	union sclr_bt_enc enc;
	union sclr_bt_sync_code sync;
	i80_lane_s i80_lane_cfg;

	sclr_disp_set_intf(SCLR_VO_INTF_I80);
	sclr_top_set_i80_if(true, I80_HW_FORMAT_RGB565);//I80_HW_FORMAT_RGB888

	i80_lane_cfg.WR = 0;
	i80_lane_cfg.RS = 2;//CDX
	i80_lane_cfg.CS = 1;
	i80_lane_cfg.RD = 3;

	enc.raw = 0;
	enc.b.fmt_sel = 2;
	enc.b.clk_inv = 1;
	sync.raw = 0;
	sync.b.sav_vld = 0x80;
	sync.b.sav_blk = 0xab;
	sync.b.eav_vld = 0x9d;
	sync.b.eav_blk = 0xb6;

	sclr_bt_set(enc, sync);

	//0x98 for cmd
	i80_ctrl[I80_CTRL_CMD] = BIT(i80_lane_cfg.RD) | ((BIT(i80_lane_cfg.RD) | BIT(i80_lane_cfg.WR)) << 4);
	//0xdc for data
	i80_ctrl[I80_CTRL_DATA] = (BIT(i80_lane_cfg.RD) | BIT(i80_lane_cfg.RS)) |
				  ((BIT(i80_lane_cfg.RD) | BIT(i80_lane_cfg.WR) | BIT(i80_lane_cfg.RS)) << 4);
	i80_ctrl[I80_CTRL_EOF] = 0xff;

	clkkHz = 1000000 / (i80_cycle_time/ 2);
	if (clkkHz < 8000) {
		debug("EINVAL: i80 disp set clk less than 8000 kHz.\n");
	}

	sclr_disp_tgen_enable(false);
	_reg_write_mask(0x03020004, 0x0000016, 0x0000016); //backlight & reset
	_reg_write_mask(0x03020000, 0x0000016, 0x0000012);

}

void i80_hw_set_combo_dev_cfg(void)
{
	unsigned int clkkHz = 0;
	unsigned short i80_cycle_time = 198;
	union sclr_bt_enc enc;
	union sclr_bt_sync_code sync;

	i80_lane_s i80_lane_cfg;

	i80_lane_cfg.WR = 0;
	i80_lane_cfg.RS = 2;//CDX
	i80_lane_cfg.CS = 1;
	i80_lane_cfg.RD = 3;
	enc.raw = 0;
	enc.b.fmt_sel = 2;
	enc.b.clk_inv = 1;
	sync.raw = 0;
	sync.b.sav_vld = 0x80;
	sync.b.sav_blk = 0xab;
	sync.b.eav_vld = 0x9d;
	sync.b.eav_blk = 0xb6;

	sclr_bt_set(enc, sync);
	//0x98 for cmd
	i80_ctrl[I80_CTRL_CMD] = BIT(i80_lane_cfg.RD) | ((BIT(i80_lane_cfg.RD) | BIT(i80_lane_cfg.WR)) << 4);
	//0xdc for data
	i80_ctrl[I80_CTRL_DATA] = (BIT(i80_lane_cfg.RD) | BIT(i80_lane_cfg.RS)) |
				  ((BIT(i80_lane_cfg.RD) | BIT(i80_lane_cfg.WR) | BIT(i80_lane_cfg.RS)) << 4);
	i80_ctrl[I80_CTRL_EOF] = 0xff;

	printf("CMD[%x], DATA[%x],EOF[%x]\n", i80_ctrl[I80_CTRL_CMD],i80_ctrl[I80_CTRL_DATA],i80_ctrl[I80_CTRL_EOF] );
	clkkHz = 1000000 / (i80_cycle_time/ 2);
	if (clkkHz < 8000) {
		debug("EINVAL: i80 disp set clk less than 8000 kHz.\n");
	}

	sclr_disp_tgen_enable(false);
#if 0 // mu hang board
	_reg_write_mask(0x03022004, BIT(19), BIT(19)); //backlight & reset
	_reg_write_mask(0x03022000, BIT(19), BIT(19)); // high
	_reg_write_mask(0x03022000, BIT(19), 0x0); // low
	_reg_write_mask(0x03022000, BIT(19), BIT(19)); // high

#else // PAD_MIPI_TXM4/XGPIOC_18
	_reg_write_mask(0x03022004, BIT(18), BIT(18)); //backlight & reset
	_reg_write_mask(0x03022000, BIT(18), BIT(18)); // high
	_reg_write_mask(0x03022000, BIT(18), 0x0); // low
	_reg_write_mask(0x03022000, BIT(18), BIT(18)); // high

	//PAD_MIPI_TXP4/XGPIOC_19 for RD hardcode high
	//_reg_write_mask(0x03022004, BIT(19), BIT(19)); //backlight & reset
	//_reg_write_mask(0x03022000, BIT(19), BIT(19)); // high

#endif
}

static void _get_frame_rgb(unsigned char **buf, unsigned int stride, unsigned short x, unsigned short y,
			   unsigned char *r, unsigned char *g, unsigned char *b)
{
	unsigned int offset = 3 * x + stride * y;
	//bgr
	*b = *(buf[0] + offset);
	*g = *(buf[0] + offset + 1);
	*r = *(buf[0] + offset + 2);
}

void i80_package_frame(unsigned char *in, unsigned char *out, unsigned int stride, unsigned char byte_cnt,
		       unsigned short w, unsigned short h)
{
	unsigned int out_offset = 0;
	unsigned short line_data = (1 + w * byte_cnt) * 3;
	unsigned short padding = ALIGN(line_data, 32) - line_data;
	unsigned char r, g, b;

	for (int y = (h - 1); y >= 0; --y) {
		for (int x = 0; x < w; ++x) {
			_get_frame_rgb(&in, stride, x, y, &r, &g, &b);
			_i80_package_rgb(r, g, b, out + out_offset, byte_cnt);
			out_offset += byte_cnt * 3;
		}
		_i80_package_eol(out + out_offset);
		out_offset += 3;
		memset(out + out_offset, 0, padding);
		out_offset += padding;
	}
	// replace last eol with eof
	_i80_package_eof(out + out_offset - 3 - padding);
}

void i80_set_pattern(void)
{
	struct sclr_rect rect;
	struct sclr_disp_cfg *cfg = NULL;
	int  stride;

#if 0//for bmp header
	unsigned short w = (*(in + 25) << 24) | (*(in + 24) << 16) | (*(in + 23) << 8) | *(in + 22);
	unsigned short h = (*(in + 21) << 24) | (*(in + 20) << 16) | (*(in + 19) << 8) | *(in + 18);
	unsigned short bmp_bytepp((*(in + 31) << 24)
		| (*(in + 30) << 16) | (*(in + 29) << 8) | *(in + 28)) / 8;
#endif
	unsigned short bmp_bytepp= 3;
	unsigned short i80_bytepp;

	i80_bytepp = 2;//i80_get_bytepp(panel_desc.i80_cfg);

	//rect.w = timing->vfde_end - timing->vfde_start + 1;
	//rect.h = timing->hfde_end - timing->hfde_start + 1;
	rect.w = 240;
	rect.h = 320;

	stride = ALIGN(rect.w * bmp_bytepp, 4); //3: bmp bpp
	rect.y = 0;
	rect.x = 0;

	sclr_disp_set_rect(rect);
	cfg = sclr_disp_get_cfg();

#if 0//for bmp header
	if (*(in) == 'B' && *(in + 1) == 'M' && w == rect.w && h == rect.h) {
		printf("this is a bmp && %d*%d\n", h, w);
		in = in + 54;	 //Remove the bmp header
	} else {
		printf("Pattern test:  display color!\n");
		in = in + 54;
		memset(in, 0x11, rect.w * rect.h * bmp_bytepp);
	}
#endif
	printf("SW i80 frame packeting\n");
	i80_package_frame(in, out, stride, i80_bytepp, rect.w, rect.h);
	printf("SW i80 framek packet done\n");

	cfg->disp_from_sc = false;

	cfg->fmt = SCL_FMT_BGR_PACKED;
	cfg->in_csc = SCL_CSC_NONE;
	cfg->mem.width = rect.w * i80_bytepp + 1;
	cfg->mem.height = rect.h;
	cfg->mem.pitch_y = ALIGN((cfg->mem.width * bmp_bytepp), 32);
	cfg->mem.pitch_c = 0;

	cfg->mem.start_x = 0;
	cfg->mem.start_y = 0;
	cfg->mem.addr0 = (u64) out;
	cfg->mem.addr1 = ALIGN(cfg->mem.addr0 + cfg->mem.pitch_y * ALIGN(((rect.h + 1) & ~(BIT(0))), 16), 0x1000);
	cfg->mem.addr2 = ALIGN(cfg->mem.addr1 + cfg->mem.pitch_c * ALIGN(((rect.h + 1) >> 1), 16), 0x1000);

	sclr_disp_set_cfg(cfg);
	sclr_disp_reg_force_up();
}

static int ili9881c_switch_page(u8 page)
{
	int ret;
#if 0
	static u8 crc[5][2] = { {0x5c, 0x1c}, {0xd5, 0x0d}, {0x4e, 0x3f}, {0xc7, 0x2e}, {0x78, 0x5a} };
	u8 buf[10] = { 0x39, 0x04, 0x00, 0x2c,	    // header
		       0xff, 0x98, 0x81, page,	    // payload
		       0xff, 0xff};		    // crc

	if (page > 4)
		return -1;

	memcpy(&buf[8], &crc[page], 2);
	ret = sclr_dsi_long_packet_raw(buf, sizeof(buf));
#else
	u8 buf[4] = { 0xff, 0x98, 0x81, page};

	ret = sclr_dsi_dcs_write_buffer(buf, sizeof(buf));
#endif
	if (ret < 0)
		return ret;

	return 0;
}

static int ili9881c_send_cmd_data(u8 cmd, u8 data)
{
	u8 buf[2] = { cmd, data };
	int ret;

	ret = sclr_dsi_dcs_write_buffer(buf, sizeof(buf));
	if (ret < 0)
		return ret;

	return 0;
}

static int hx8394_send_cmd_data(u8 size, const u8* data)
{
	int ret;

	ret = sclr_dsi_dcs_write_buffer(data, size);
	if (ret < 0)
		return ret;

	return 0;
}

static int irq_handler_scl(int irqn, void *priv)
{
    u64 now = timer_get_tick();

    if (enable_intr_msg)
	uartlog("%s: time-consumption(%d)ms\n", __func__, (now - sclr_timer_tick)/TIMER_CNT_MS);

    if (enable_cmdq) {
        u8 status = cmdQ_intr_status(REG_SCL_CMDQ_BASE);
        if (status) {
            cmdQ_intr_clr(REG_SCL_CMDQ_BASE, status);
            uartlog("%s: irqn(%d) cmdq-status(0x%x)\n", __func__, irqn, status);

            if ((status & 0x04)&& cmdQ_is_sw_restart(REG_SCL_CMDQ_BASE)) {
                uartlog("%s: cmdq-sw restart\n", __func__);
                cmdQ_sw_restart(REG_SCL_CMDQ_BASE);
            }

            if (status & 0x02) {
                sclr_mark++;  // cmdq end
                enable_cmdq = false;
            }
        } else {
            uartlog("%s: irqn(%d) cmdq-status not found\n", __func__, irqn);
        }
    } else {
        union sclr_intr intr_status = sclr_intr_status();
        sclr_intr_clr(intr_status);

	if (enable_intr_msg)
		uartlog("%s: status(%#x)\n", __func__, intr_status.raw);

        if (intr_mask.raw & intr_status.raw) {
            ++sclr_mark;
        }
    }

    return 0;
}

int sclr_check_irq_handler(int timeout)
{
    int i;

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

static void _show_sclr_mem(struct sclr_mem *mem)
{
    uartlog("start_x(%3d)\t\tstart_y(%3d)\t\twidth(%4d)\t\theight(%4d)\n", mem->start_x, mem->start_y, mem->width, mem->height);
    uartlog("pitch_y(%3d)\t\tpitch_c(%3d)\n", mem->pitch_y, mem->pitch_c);
    uartlog("addr0(0x%09lx)\taddr1(0x%09lx)\taddr2(0x%09lx)\n", mem->addr0, mem->addr1, mem->addr2);
}

void _intr_setup(u8 inst, enum sclr_img_in img_inst)
{
    union sclr_intr mask;
    memset(&mask, 0, sizeof(mask));
    if (img_inst == SCL_IMG_V)
        mask.b.img_in_v_frame_end = true;
    else
        mask.b.img_in_d_frame_end = true;

    if (inst == 0)
        mask.b.scl0_frame_end = true;
    else if (inst == 1)
        mask.b.scl1_frame_end = true;
    else if (inst == 2)
        mask.b.scl2_frame_end = true;
    else if (inst == 3)
        mask.b.scl3_frame_end = true;

    mask.b.cmdq = true;
    mask.b.prog_too_late = true;
    mask.b.osd_cmp_frame_end = true;
    sclr_set_intr_mask(mask);

    intr_mask.raw = BIT(6 + inst);
    sclr_timer_tick = timer_get_tick();
}


void _test_swap(u8 *a, u8 *b)
{
	int temp = *a;
	*a = *b;
	*b = temp;
}

void _test_prem(u8 *arr, u8 i, u8 n)
{
	int j;
	if(i == n)
	{
		count++;
		//if(count > 300)
		{
			printf("PIN MUX CASE[%d]---",count);

			for(j = 0; j < 21; j++)
			{
				printf("%d ", arr[j]);
			}
			for(j=0;j<21;j++)
			{
				sclr_top_set_vo_data_mux(j, arr[j]);
			}
			printf("\n");
			mdelay(400);//ms?
		}
	}
	else
	{
		for(j = i; j <= n; j++)
		{
			_test_swap(&arr[i], &arr[j]);
			_test_prem(arr, i + 1, n);
			_test_swap(&arr[i], &arr[j]);
		}
	}
}


static int _handle_cfg_op(u8 op, ...)
{
    int ret = 0;
    u32 tmp = 0;
    va_list ap;
    va_start(ap, op);

	switch(op) {
	case CFG_OP_IMG_CFG:
	{
		enum sclr_img_in img_inst = va_arg(ap, enum sclr_img_in);
		union sclr_input in;
		enum sclr_format fmt;
		enum sclr_csc csc = SCL_CSC_NONE;

		if (img_inst == SCL_IMG_D) {
			bm_get_num("Src input(0:ISP, 1:MAP, 2:MEM): ", in.d);
		} else {
			bm_get_num("Src input(0:ISP, 2:MEM): ", in.v);
		}

		uartlog("Src format(0:420, 1:422, 2:rgb, 3:rgb_pack, 4:bgr_pack, 5:y_only\n");
		bm_get_num("8:nv12, 9:nv21, 10:422sp1(UV), 11:422sp2(VU), 12:YVYU, 13:YUYV, 14:VYUY, 15:UYVY): ", fmt);
		if (IS_YUV_FMT(fmt)) {
			bm_get_num("CSC(0:601_Limit, 1:601_Full, 2:709_Limit, 3:709_full): ", csc);
			++csc;
		}

		sclr_ctrl_set_input(img_inst, in, fmt, csc);
		break;
	}

	case CFG_OP_IMG_SET_MEM:
	{
		enum sclr_img_in img_inst = va_arg(ap, enum sclr_img_in);
		struct sclr_img_cfg *cfg = sclr_img_get_cfg(img_inst);
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
		if (cfg->src != SCL_INPUT_D_MEM) {
			mem.width = size.w;
			mem.height = size.h;
			sclr_img_set_mem(img_inst, &mem);
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

		sclr_img_set_mem(img_inst, &mem);
		break;
	}

	case CFG_OP_IMG_START:
	{
		enum sclr_img_in img_inst = va_arg(ap, enum sclr_img_in);
		sclr_trig_start(img_inst);
		break;
	}

	case CFG_OP_TOP_SET_BLEND:
	{
		struct sclr_bld_cfg cfg;

		bm_get_num("Mod, blend(0)/gradient(1)/manual(2): ", tmp);

		cfg.ctrl.raw = 0;
		if (tmp == 0) {
			uartlog("Please make sure YUV data placed in img-v\n");
			cfg.ctrl.b.enable = 1;
			cfg.ctrl.b.fix_alpha = 1;
			cfg.ctrl.b.blend_y = 1;
			cfg.ctrl.b.y2r_enable = 1;
			bm_get_num("alpha factor, (0 ~ 256): ", cfg.ctrl.b.alpha_factor);
			sclr_img_csc_en(SCL_IMG_D, false);
			sclr_img_csc_en(SCL_IMG_V, false);
		} else if (tmp == 1) {
			cfg.ctrl.b.enable = 1;
			cfg.ctrl.b.fix_alpha = 0;
			cfg.ctrl.b.blend_y = 0;
			cfg.ctrl.b.y2r_enable = 0;
		} else if (tmp == 2) {
			bm_get_num("enable, y(1)/n(0): ", cfg.ctrl.b.enable);
			if (cfg.ctrl.b.enable) {
				bm_get_num("fix_alpha, 0(stitch)/1(blending): ", cfg.ctrl.b.fix_alpha);
				bm_get_num("y_only, y(1)/n(0): ", cfg.ctrl.b.blend_y);
				bm_get_num("y2r_enable, y(1)/n(0): ", cfg.ctrl.b.y2r_enable);
				bm_get_num("alpha factor, (0 ~ 256): ", cfg.ctrl.b.alpha_factor);
			}
		}
		sclr_top_bld_set_cfg(&cfg);
		break;
	}

	case CFG_OP_IMG_CHECKSUM:
	{
		enum sclr_img_in img_inst = va_arg(ap, enum sclr_img_in);
		bool checksum_en = false;
		bm_get_num("checksum enable, y(1)/n(0): ", checksum_en);
		sclr_img_checksum_en(img_inst, checksum_en);
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

		sclr_set_crop(inst, crop);
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

	case CFG_OP_SC_SET_COMPR:
	{
		struct sclr_oenc_cfg oenc_cfg;
		struct sclr_size img_size;
		u16 mem_width;

		uartlog("Please make sure compress data has been loaded into following address:0x%lx\n", gop_addr);
		bm_get_num("format(0:ARGB8888, 1:ARGB4444, 2:ARGB1555, 3:256LUT, 4:16LUT): ", oenc_cfg.fmt);
		bm_get_num("img width: ", img_size.w);
		bm_get_num("img height: ", img_size.h);

		oenc_cfg.src_picture_size = img_size;
		oenc_cfg.src_mem_size.h = img_size.h;

		switch (oenc_cfg.fmt) {
			case SCL_GOP_FMT_ARGB8888:
				mem_width = img_size.w * 4;
				break;
			case SCL_GOP_FMT_ARGB4444:
			case SCL_GOP_FMT_ARGB1555:
				mem_width = img_size.w * 2;
				break;
			case SCL_GOP_FMT_256LUT:
				mem_width = img_size.w;
				break;
			case SCL_GOP_FMT_16LUT:
				mem_width = img_size.w >> 1;
				break;
			default:
				uartlog("invalid fmt(%d)\n", oenc_cfg.fmt);
				return -1;
		}
		oenc_cfg.src_pitch = ALIGN(mem_width, 16);
		oenc_cfg.src_mem_size.w = ALIGN(mem_width, 16);
		uartlog("mem size: (%d*%d), pitch(%d)\n", oenc_cfg.src_mem_size.w, oenc_cfg.src_mem_size.h, oenc_cfg.src_pitch);

		// count down from space's end
		oenc_cfg.src_adr = gop_addr;
		oenc_cfg.bso_adr = gop_addr + (oenc_cfg.src_pitch * oenc_cfg.src_mem_size.h);
		uartlog("osd src mem addr: (0x%09lx)\n", oenc_cfg.src_adr);
		uartlog("OSD Compression Bit Stream Output addr: (0x%09lx)\n", oenc_cfg.bso_adr);

		bm_get_num("RGB truncate mode enable, y(1~3)/n(0): ", oenc_cfg.cfg.b.rgb_trunc);
		bm_get_num("Alpha truncate mode enable, y(1~3)/n(0): ", oenc_cfg.cfg.b.alpha_trunc);
		bm_get_num("Alpha_zero mode enable (hw init:y), y(1)/n(0): ", oenc_cfg.cfg.b.alpha_zero);

		bm_get_num("Osd ENC bitstream size limit enable, y(1)/n(0): ", tmp);
		if (tmp) {
			oenc_cfg.cfg.b.limit_bsz_en = tmp;
			bm_get_num("Osd ENC Bitstream Output size in byte, max 32MB: ", oenc_cfg.limit_bsz);
		}

		bm_get_num("Osd ENC AXI write protect enable, y(1)/n(0): ", tmp);
		if (tmp) {
			oenc_cfg.cfg.b.wprot_en = tmp;
			bm_get_num("Osd ENC AXI write protect low address: ", oenc_cfg.wprot_laddr);
			bm_get_num("Osd ENC AXI write protect upper address: ", oenc_cfg.wprot_uaddr);
		}

		bm_get_num("Osd ENC watchdog enable (hw init:y), y(1)/n(0): ", oenc_cfg.cfg.b.wdog_en);

		sclr_oenc_set_cfg(&oenc_cfg);
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
			//uartlog("factor:        %d %d %d\n", q_cfg.sc[0], q_cfg.sc[1], q_cfg.sc[2]);
			//uartlog("mean-sub:      %d %d %d\n", q_cfg.sub[0], q_cfg.sub[1], q_cfg.sub[2]);
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
		struct sclr_top_cfg *top_cfg = sclr_top_get_cfg();
		struct sclr_mem mem;
		struct sclr_size size;
		u8 dst_idx = 0;
		u64 addr0, addr1, addr2;
		u16 pitch[3];

		if (0 == inst && top_cfg->disp_from_sc) {
			uartlog("It's disp_from_sc now, so no config on scl_d's odma.\n");
			break;
		}

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

		if (0 == inst) {
			struct sclr_disp_cfg *disp_cfg = sclr_disp_get_cfg();
			disp_cfg->fmt = cfg->fmt;
			if (cfg->csc_cfg.csc_type != SCL_CSC_NONE) disp_cfg->in_csc = cfg->csc_cfg.csc_type - SCL_CSC_601_LIMIT_RGB2YUV + 1;
			disp_cfg->mem = mem;
			sclr_disp_set_cfg(disp_cfg);
		}
		break;
	}

	case CFG_OP_DISP_CFG:
	{
		struct sclr_disp_cfg *cfg = sclr_disp_get_cfg();
		struct sclr_core_cfg *sc_cfg = sclr_get_cfg(0);
		struct sclr_odma_cfg *odma_cfg = sclr_odma_get_cfg(0);
		enum sclr_csc csc = SCL_CSC_NONE;
		struct sclr_rect rect;
		rect.x = rect.y = 0;
		rect.w = sc_cfg->sc.dst.w;
		rect.h = sc_cfg->sc.dst.h;
		sclr_disp_set_rect(rect);

		bm_get_num("disp src, sc_d(1)/dram(0): ", cfg->disp_from_sc);
		bm_get_num("tgen enable, y(1)/n(0): ", cfg->tgen_en);
		uartlog(" format(0:420, 1:422, 2:rgb, 3:rgb_pack, 4:bgr_pack, 5:y_only\n");
		bm_get_num(" 8:nv12, 9:nv21, 10:422sp1(UV), 11:422sp2(VU), 12:YVYU, 13:YUYV, 14:VYUY, 15:UYVY): ", cfg->fmt);
		if (!cfg->disp_from_sc) {
			if (IS_YUV_FMT(cfg->fmt)) {
				bm_get_num("CSC(0:601_Limit, 1:601_Full, 2:709_Limit, 3:709_full): ", csc);
				cfg->in_csc = csc + 1;
			} else {
				cfg->in_csc = SCL_CSC_NONE;
			}

			cfg->mem = odma_cfg->mem;
		}

		bm_get_num("out format, rgb(0)/yuv(1): ", tmp);
		if (tmp) {
			bm_get_num("out CSC(0:601_Limit, 1:601_Full, 2:709_Limit, 3:709_full): ", csc);
			cfg->out_csc = csc + SCL_CSC_601_LIMIT_RGB2YUV;
		} else {
			cfg->out_csc = SCL_CSC_NONE;
		}
		bm_get_num("burst len(0~15): ", cfg->burst);

		sclr_disp_set_cfg(cfg);
		break;
	}

	case CFG_OP_DISP_SRC:
	{
		bm_get_num("disp_from_sc(1)/from_mem(0): ", tmp);
		sclr_ctrl_set_disp_src(tmp);
		break;
	}

	case CFG_OP_DISP_TGEN:
	{
		bm_get_num("tgen enable(1)/disable(0): ", tmp);
		sclr_disp_tgen_enable(tmp);
		break;
	}

	case CFG_OP_LVDSTX:
	{
		union sclr_lvdstx cfg;
		sclr_lvdstx_get(&cfg);

		printf("Panel: AUO G121SN01 V4\n");
		printf("FPAG connector: J11\n");
		printf("Format: LVDS\n");
		printf("Panel Size: 800x600\n");

		bm_get_num("lvdstx enable, y(1)/n(0): ", cfg.b.en);
		if (cfg.b.en) {
			bm_get_num("lvdstx bits 6(0)/8(1)/10(2): ", cfg.b.out_bit);
			bm_get_num("lvdstx JEIDA(0)/VESA(1): ", cfg.b.vesa_mode);
			bm_get_num("lvdstx link single(0)/dual(1): ", cfg.b.dual_ch);
			bm_get_num("lvdstx vs_out enable, y(1)/n(0): ", cfg.b.vs_out_en);
			bm_get_num("lvdstx hs_out enable, y(1)/n(0): ", cfg.b.hs_out_en);
			bm_get_num("lvdstx v-blanking hs_out enable, y(1)/n(0): ", cfg.b.hs_blk_en);
			bm_get_num("lvdstx hs data msb/lsb swap, y(1)/n(0): ", cfg.b.ml_swap);
			bm_get_num("lvdstx serializer lsb first, y(1)/n(0): ", cfg.b.ctrl_rev);
			bm_get_num("lvdstx even/odd link swap, y(1)/n(0): ", cfg.b.oe_swap);

			_reg_write(REG_DSI_PHY_PD, 0x1f1f);
			dphy_lvds_enable(true);
		}
		sclr_lvdstx_set(cfg);
		break;
	}

	case CFG_OP_BT:
	{
		// TODO:
		break;
	}

	case CFG_OP_DSI_ILI9881C_INIT:
	{
		bool data_en[] = {true, true, true, true};
		struct sclr_disp_cfg *cfg = sclr_disp_get_cfg();

		cfg->out_bit = 8;
		sclr_disp_set_cfg(cfg);

		printf("Panel: WD50DTYA3MNN0 \n");
		printf("FPAG connector: J10\n");
		printf("Format: MIPI\n");
		printf("Panel Size: 720x1280\n");

		_reg_write(REG_DSI_PHY_PD, 0x0);
		dphy_dsi_lane_en(true, data_en, true);
		sclr_dsi_config(4, SCLR_DSI_FMT_RGB888, 720);
		sclr_disp_set_timing(&panels[SCL_PANEL_720x1280]);

		bm_get_num("Please do panel reset and then press Enter to continue...\n", tmp);
		for (int i = 0; i < ARRAY_SIZE(ili9881c_init); i++) {
			const struct ili9881c_instr *instr = &(ili9881c_init[i]);

			if (instr->op == ILI9881C_SWITCH_PAGE)
				ret = ili9881c_switch_page(instr->arg.page);
			else if (instr->op == ILI9881C_COMMAND)
				ret = ili9881c_send_cmd_data(instr->arg.cmd.cmd, instr->arg.cmd.data);
			else if (instr->op == ILI9881C_DELAY)
				mdelay(instr->arg.page);
			if (ret) {
				uartlog("ili9881 init failed at %d instr.\n", i);
				return ret;
			}
		}

		sclr_dsi_set_mode(SCLR_DSI_MODE_HS);
		break;
	}

	case CFG_OP_DISP_GAMMA:
	{
		bool enable, pre_osd;
		const u8 *data;

		bm_get_num("disp gamma enable, y(1)/n(0): ", enable);
		if (enable) {
			bm_get_num("disp gamma pre_ods, y(1)/n(0): ", pre_osd);
			sclr_disp_gamma_ctrl(enable, pre_osd);
			bm_get_num("gamma table, normal(1)/dark(0): ", tmp);
			data = (tmp == 0) ? sc_gamma_data0 : sc_gamma_data1;
			sclr_disp_gamma_lut_update(data, data, data);
		} else {
			sclr_disp_gamma_ctrl(enable, false);
		}
		break;
	}

	case CFG_OP_DSI_HX8394_INIT:
	{
		bool data_en[] = {true, true, true, true};
		enum lane_id lane_select[] = {DSI_LANE_0, DSI_LANE_1, DSI_LANE_CLK, DSI_LANE_2, DSI_LANE_3};
		bool lane_pn_swap[] = {true, true, true, true, true};
		struct sclr_disp_cfg *cfg = sclr_disp_get_cfg();

		cfg->out_bit = 8;
		sclr_disp_set_cfg(cfg);

		// MIPI_TX reset
		_reg_write(0x030010A0, 0x03);

		// MIPI_TX PINMUX
		_reg_write(0x03001110, 0x00);
		_reg_write(0x03001114, 0x00);
		_reg_write(0x03001118, 0x00);
		_reg_write(0x0300111C, 0x00);
		_reg_write(0x03001120, 0x00);
		_reg_write(0x03001124, 0x00);
		_reg_write(0x03001128, 0x00);
		_reg_write(0x0300112C, 0x00);
		_reg_write(0x03001130, 0x00);
		_reg_write(0x03001134, 0x00);
		_reg_write(0x0300109c, 0x00);//PWR_ON

		_reg_write(0x030010a4, 0x00);//PWR_GPIO0 => PWR_GPIO_0
		_reg_write(0x030010a8, 0x00);//PWR_GPIO1 => PWR_GPIO_1
		_reg_write(0x030010ac, 0x00);//PWR_GPIO2 => PWR_GPIO_2

		_reg_write(0x05021004, 0x7);//Set GPIO direction
		_reg_write(0x05021000, 0x7);//Set GPIO Output value

		// MIPI_TX pll ref
		_reg_write(0x03002840, 0x3E);

		// disp_clk, dsi_esc_clk
		_reg_write(0x03002030, 0);

		_reg_write(0x03021004, 0x38);
		_reg_write(0x03021000, 0x18);
		mdelay(200);
		_reg_write(0x03021000, 0x38);

		dphy_dsi_init();
		for (int i = 0; i < DSI_LANE_MAX; ++i)
			dphy_dsi_set_lane(i, lane_select[i], lane_pn_swap[i]);
		dphy_dsi_lane_en(true, data_en, true);
		dphy_dsi_set_pll(74250, 4, 24);
		sclr_dsi_config(4, SCLR_DSI_FMT_RGB888, 720);
		sclr_disp_set_timing(&panels[SCL_PANEL_720x1280]);

		for (int i = 0; i < ARRAY_SIZE(hx8394_init); i++) {
			const struct dsc_instr *instr = &(hx8394_init[i]);

			ret = hx8394_send_cmd_data(instr->size, instr->data);
			if (instr->delay)
				mdelay(instr->delay);

			if (ret) {
				uartlog("hx8394 init failed at %d instr.\n", i);
				return ret;
			}
		}

		sclr_dsi_set_mode(SCLR_DSI_MODE_HS);
		break;
	}

	case CFG_OP_DSI_LP_LOOP:
	{
		bool data_en[] = {true, true, true, true};
		u32 reg[3] = {0x1f1f1f1f, 0x1f1f001f, 0x001f001f};
		u8 count;

		_reg_write(REG_DSI_PHY_PD, 0x0);
		dphy_dsi_lane_en(true, data_en, true);

		do {
			_reg_write(REG_DSI_PHY_DATA_OV, reg[count % 3]);
			if (++count >= 3*80)
				count = 0;
		} while(1);
		break;
	}

	case CFG_OP_DSI_DSC_CMD:
	{
		struct dsc_instr inst;
		int i = 0;
		int data;

		bm_get_num("cmd_size: ", inst.size);
		while (i++ < inst.size) {
			uartlog("data%02d: ", i);
			scanf("0x%x", &data);
			inst.data[i] = data;
		}
		uartlog("size(%d), %#x, %#x", inst.size, inst.data[0], inst.data[1]);
		sclr_dsi_dcs_write_buffer(inst.data, inst.size);
		break;
	}

	case CFG_OP_DISP_CHECKSUM:
	{
		bool checksum_en = false;
		bm_get_num("checksum enable, y(1)/n(0): ", checksum_en);
		sclr_disp_checksum_en(checksum_en);
		break;
	}

	case CFG_OP_GOP_CFG:
	{
		u8 inst = va_arg(ap, int);
		u8 layer;
		if (inst <= SCL_GOP_SCV3) {
			bm_get_num("Control which gop, gop0(0)/gop1(1): ", layer);
			if(layer >= SCL_MAX_GOP_INST) {
				uartlog("Do not have such layer%d", layer);
				return -1;
			}
		} else if (inst == SCL_GOP_DISP) {
			uartlog("Display use gop1 cfg!!!\n");
			layer = 1;
		} else {
			uartlog("Do not have such GOP%d!!!\n", inst);
			return -1;
		}

		// Get global g_gop_cfg[inst][layer] or  &g_disp_cfg.gop_cfg
		struct sclr_gop_cfg *gop_cfg = sclr_gop_get_cfg(inst, layer);

		if (inst < SCL_GOP_MAX){
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
		u8 ow_inst;
		u16 mem_width;
		struct sclr_gop_ow_cfg ow_cfg;
		struct sclr_size osd_size;

		u8 layer;
		if (inst <= SCL_GOP_SCV3) {
			bm_get_num("Control which gop, gop0(0)/gop1(1): ", layer);
			if(layer >= SCL_MAX_GOP_INST) {
				uartlog("Do not have such layer%d", layer);
				return -1;
			}
		} else if (inst == SCL_GOP_DISP) {
			uartlog("Display use gop1 cfg!!!\n");
			layer = 1;
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
		//Format ARGB8888,          crop pixels valid value: 0~3
		//Format ARGB4444/ARGB1555, crop pixels valid value: 0~7
		//Format 256LUT,            crop pixels valid value: 0~15
		//Format 16-LUT,            crop pixels valid value: 0~31
		//Format Font-base,         crop pixels valid value: 0~127"
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

		if (layer == 0) {
			ow_cfg.end.x = ow_cfg.start.x +
					(osd_size.w << gop_cfg->gop_ctrl.b.hscl_en) - gop_cfg->gop_ctrl.b.hscl_en;
			ow_cfg.end.y = ow_cfg.start.y +
					(osd_size.h << gop_cfg->gop_ctrl.b.vscl_en) - gop_cfg->gop_ctrl.b.vscl_en;
		} else if (layer == 1) {
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
				if (layer == 0) {
					gop_cfg->fb_ctrl.b.hi_thr = tmp >> 2;
					gop_cfg->fb_ctrl.b.lo_thr = gop_cfg->fb_ctrl.b.hi_thr * 0.8;
					bm_get_num("invert condition?, low(1)/high(0): ", tmp);
					gop_cfg->fb_ctrl.b.lo_thr_inv = tmp;
				} else if (layer == 1) {
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
		if (inst <= SCL_GOP_SCV3) {
			bm_get_num("Control which gop, gop0(0)/gop1(1): ", layer);
			if(layer >= SCL_MAX_GOP_INST) {
				uartlog("Do not have such layer%d", layer);
				return -1;
			}
		} else if (inst == SCL_GOP_DISP) {
			uartlog("Display use gop1 cfg!!!\n");
			layer = 1;
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
		if (inst <= SCL_GOP_SCV3) {
			bm_get_num("Control which gop, gop0(0)/gop1(1): ", layer);
			if(layer >= SCL_MAX_GOP_INST) {
				uartlog("Do not have such layer%d", layer);
				return -1;
			}
		} else if (inst == SCL_GOP_DISP) {
			uartlog("Display use gop1 cfg!!!\n");
			layer = 1;
		} else {
			uartlog("Do not have such GOP%d!!!\n", inst);
			return -1;
		}

		// Get global g_gop_cfg[inst][layer] or  &g_disp_cfg.gop_cfg
		struct sclr_gop_cfg *gop_cfg = sclr_gop_get_cfg(inst, layer);

		if (layer == 0 ) {
			bm_get_num("h-scal(x2) enable, y(1)/n(0): ", gop_cfg->gop_ctrl.b.hscl_en);
			bm_get_num("v-scal(x2) enable, y(1)/n(0): ", gop_cfg->gop_ctrl.b.vscl_en);
		} else if (layer == 1) {
			bm_get_num("h-scal(x2) enable, y(1)/n(0): ", gop_cfg->gop_ctrl.b.hscl_en);
			bm_get_num("v-scal(x2) enable, y(1)/n(0): ", gop_cfg->gop_ctrl.b.vscl_en);
		}
		sclr_gop_set_cfg(inst, layer, gop_cfg);

		for (int i = 0; i < 8; ++i) {
			struct sclr_gop_ow_cfg *ow_cfg;
			if (layer == 0 ) {
				ow_cfg = &gop_cfg->ow_cfg[i];
				if (gop_cfg->gop_ctrl.raw & BIT(i)) {
					ow_cfg->end.x = ow_cfg->start.x +
							(ow_cfg->img_size.w << gop_cfg->gop_ctrl.b.hscl_en) - gop_cfg->gop_ctrl.b.hscl_en;
					ow_cfg->end.y = ow_cfg->start.y +
							(ow_cfg->img_size.h << gop_cfg->gop_ctrl.b.vscl_en) - gop_cfg->gop_ctrl.b.vscl_en;
				}
			} else if (layer == 1) {
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
		if (inst <= SCL_GOP_SCV3) {
			bm_get_num("Control which gop, gop0(0)/gop1(1): ", layer);
			if(layer >= SCL_MAX_GOP_INST) {
				uartlog("Do not have such layer%d", layer);
				return -1;
			}
		} else if (inst == SCL_GOP_DISP) {
			uartlog("Display use gop1 cfg!!!\n");
			layer = 1;
		} else {
			uartlog("Do not have such GOP%d!!!\n", inst);
			return -1;
		}

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
		break;
	}

	case CFG_OP_STATUS_TOP:
	{
		struct sclr_top_cfg *cfg = sclr_top_get_cfg();
		uartlog("-------------------------TOP---------------------------\n");
		uartlog("sc0_en(%d)\t\tsc1_en(%d)\t\tsc2_en(%d)\t\tsc3_en(%d)\n", cfg->sclr_enable[0], cfg->sclr_enable[1], cfg->sclr_enable[2], cfg->sclr_enable[3]);
		uartlog("disp_en(%d)\t\tdisp_from_sc(%d)\t\tscl_d_src(%s)\n", cfg->disp_enable, cfg->disp_from_sc, cfg->sclr_d_src ? "IMG_D":"IMG_V");
		break;
	}

	case CFG_OP_STATUS_IMG_IN:
	{
		for(int i = 0; i < SCL_IMG_MAX; ++i) {
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
				uartlog("ow_enable(%d-%d-%d-%d-%d-%d-%d-%d)     scale_up(%dx%d)     colorkey_en(%d)     burst(%d)\n",
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

	case CFG_OP_STATUS_DISP:
	{
		struct sclr_disp_cfg *cfg = sclr_disp_get_cfg();
		union sclr_disp_dbg_status status = sclr_disp_get_dbg_status(true);
		uartlog("-------------------------DISP---------------------------\n");
		uartlog("disp_from_sc(%d)\t\tsync_ext(%d)\t\ttgen_en(%d)\t\tfmt(%s)\n", cfg->disp_from_sc, cfg->sync_ext, cfg->tgen_en, str_sclr_fmt[cfg->fmt]);
		uartlog("in_csc(%15s)\tout_csc(%15s)burst(%d)\n", str_sclr_csc[cfg->in_csc], str_sclr_csc[cfg->out_csc], cfg->burst);
		_show_sclr_mem(&cfg->mem);

		uartlog("err_fwr_yuv(%d%d%d)\terr_erd_yuv(%d%d%d)\tlb_full_yuv(%d%d%d)\tlb_empty_yuv(%d%d%d)\n", status.b.err_fwr_y, status.b.err_fwr_u,  status.b.err_fwr_v, status.b.err_erd_y, status.b.err_erd_u, status.b.err_erd_v, status.b.lb_full_y, status.b.lb_full_u, status.b.lb_full_v, status.b.lb_empty_y, status.b.lb_empty_u, status.b.lb_empty_v);
		uartlog("bw fail(%d)\n", status.b.bw_fail);

		struct sclr_disp_checksum_status ch_status;
		sclr_disp_get_checksum_status(&ch_status);
		uartlog("checksum en(%d)\n", ch_status.checksum_base.b.enable);
		uartlog("checksum axi_read_from_dram(0x%x)\n", ch_status.axi_read_from_dram);
		uartlog("checksum axi_read_from_gop(0x%x)\n", ch_status.axi_read_from_gop);
		uartlog("checksum axi_read_gop1_data(0x%x)\n", ch_status.checksum_base.b.data_in_from_sc_d);
		uartlog("checksum data_out(0x%x)\n", ch_status.checksum_base.b.data_out);
		break;
	}

	case CFG_OP_STATUS_LVDSTX:
	{
		char *str_lvds_bit[] = { "6", "8", "10" };
		char *str_lvds_mode[] = { "JEIDA", "VESA" };
		char *str_lvds_link[] = { "single", "dual" };
		union sclr_lvdstx cfg;
		sclr_lvdstx_get(&cfg);
		uartlog("lvdstx enable(%d)\n", cfg.b.en);
		uartlog("output bits(%s)\t%s\t%s link\n",
				str_lvds_bit[cfg.b.out_bit], str_lvds_mode[cfg.b.vesa_mode],
				str_lvds_link[cfg.b.dual_ch]);
		uartlog("vs_out_en(%d)\ths_out_en(%d)\ths_blk_en(%d)\n",
				cfg.b.vs_out_en, cfg.b.hs_out_en, cfg.b.hs_blk_en);
		uartlog("hs data msb/lsb swap(%d)\tserializer %s first\n",
				cfg.b.ml_swap, cfg.b.ctrl_rev ? "lsb":"msb");
		uartlog("even/odd link swap(%d)\n", cfg.b.oe_swap);
		break;
	}

	case CFG_OP_REG_DUMP:
	{
		bm_get_num("reg dump, enable(1)/disable(0): ", dump_reg);
		break;
	}

	case CFG_OP_TOP_REG_DUMP:
		sclr_dump_top_register();
		break;

	case CFG_OP_IMG_IN_REG_DUMP:
	{
		u8 img_inst = 0;
		bm_get_num("img_in inst, d(1)/v(0): ", img_inst);
		sclr_dump_img_in_register(img_inst);
		break;
	}

	case CFG_OP_SC_CORE_DUMP:
	{
		u8 inst = 0;
		uartlog("sc inst, sc_d(0), sc_v1(1), sc_v2(2), sc_v3(3)\n");
		bm_get_num("sc inst: ", inst);
		sclr_dump_core_register(inst);
		break;
	}

	case CFG_OP_SC_ODMA_DUMP:
	{
		u8 inst = 0;
		uartlog("sc inst, sc_d(0), sc_v1(1), sc_v2(2), sc_v3(3)\n");
		bm_get_num("sc inst: ", inst);
		sclr_dump_odma_register(inst);
		break;
	}

	case CFG_OP_SC_DISP_DUMP:
		sclr_dump_disp_register();
		break;
	case CFG_OP_PANEL_TTL_RGB_TEST://77
	{
		u8 idx =0,perm_test=0;
		u8 paneltype = 2;
		u8 pinmux_prem_idx_end=0;
		u8 pinmux_prem_idx_start=0;
		u8 pinmux_setting[21]={24,25,26,//vs, hs, hde
                               20,21,22,23,18,19, //R channel
                               12,13,14,15,10,11,//G channel
                               4,5,6,7,2,3};//B channel
		union vip_sys_clk_ctrl0 cfg;

		sclr_disp_set_timing(&panels[SCL_PANEL_TTL]);


		//Step1: Set TTL Panel
		cfg = vip_get_clk_ctrl0();
		cfg.b.bt_src_sel = paneltype;
		vip_set_clk_ctrl0(cfg);

		//Step2: Set T Gen
		sclr_disp_tgen_enable(1);

		printf("Panel: LQ121S1DG41 \n");
		printf("FPAG connector: J12\n");
		printf("Pixel format: RGB888\n");
		printf("Panel Size: 800x600\n");

		//Step3: set vo sel
		sclr_top_set_vo_type_sel(SCLR_VO_SEL_RGB);//set vo sel = 1: rgb signal

		//Step4: set vo pin mux
		for(idx=0;idx<21;idx++)
		{
			sclr_top_set_vo_data_mux(idx, pinmux_setting[idx]);
		}

		bm_get_num("RGB pin mux perm test (1/0)\n ", perm_test);
		if(!perm_test)
			break;

		uartlog("Pin mux R channel idx: 3~8\n");
		uartlog("Pin mux G channel idx: 9~14\n");
		uartlog("Pin mux B channel idx: 15~20\n");

		bm_get_num("VO pin mux test perm Start idx\n ", pinmux_prem_idx_start);
		bm_get_num("VO pin mux test perm End idx\n ", pinmux_prem_idx_end);

		if(pinmux_prem_idx_start == 99 || pinmux_prem_idx_end == 99)
			break;

		//Step5: Test R/G/B channel order by perm
		count=0;
		_test_prem(pinmux_setting, pinmux_prem_idx_start, pinmux_prem_idx_end);
		break;
	}
	case CFG_OP_PANEL_FRD_RGB_3_SERIAL_TEST: //79
	{
		u8 idx =0;
		u8 paneltype = 2;
		//u8 pinmux_prem_idx_end=0;
		//u8 pinmux_prem_idx_start=0;
		u8 pinmux_setting[21]={24,25,26,//vs, hs, hde
                               0,1,2,3,4,5, //R channel
                               6,7,8,9,10,11,//G channel
                               12,13,14,15,16,17};//B channel
		union vip_sys_clk_ctrl0 cfg;

		sclr_disp_set_timing(&panels[SCL_PANEL_FRD]);

		printf("Panel: FRD280C50106-A \n");
		printf("FPAG connector: J10\n");
		printf("Pixel format: RGB 666\n");
		printf("Image prepare: size:240x320 fmt:RGB \n");

		//Step1: Set TTL Panel
		cfg = vip_get_clk_ctrl0();
		cfg.b.bt_src_sel = paneltype;
		vip_set_clk_ctrl0(cfg);

		//Step2: Set T Gen
		sclr_disp_tgen_enable(true);

		//Step3: set vo sel
		sclr_top_set_vo_type_sel(SCLR_VO_SEL_SERIAL_RGB);//set vo sel = serial RGB
		sclr_disp_set_srgb_ttl_en(true);
		sclr_disp_set_srgb_ttl_4x(false);

		//Step4: set vo pin mux
		for(idx=0;idx<sizeof(pinmux_setting);idx++)
		{
			sclr_top_set_vo_data_mux(idx, pinmux_setting[idx]);
		}
		break;
	}
	case CFG_OP_PANEL_FRD_RGB_TEST: //78
	{
		u8 idx =0;
		u8 paneltype = 2;
		//u8 pinmux_prem_idx_end=0;
		//u8 pinmux_prem_idx_start=0;
		u8 pinmux_setting[21]={24,25,26,
                               20,21,22,23,18,19, //R channel
                               12,13,14,15,10,11,//G channel
                               4,5,6,7,2,3};//B channel
		union vip_sys_clk_ctrl0 cfg;
		//i80_lane_s i80_lane_cfg;

		printf("Panel: FRD280C50106-A \n");
		printf("FPAG connector: J10\n");
		printf("Pixel format: RGB 666\n");
		printf("Image prepare: size:240x320 fmt:RGB \n");

		sclr_disp_set_timing(&panels[SCL_PANEL_FRD]);

		//Step1: Set TTL Panel
		cfg = vip_get_clk_ctrl0();
		cfg.b.bt_src_sel = paneltype;
		vip_set_clk_ctrl0(cfg);

		//Step2: Set T Gen
		sclr_disp_tgen_enable(true);

		//Step3: set vo sel
		sclr_top_set_vo_type_sel(SCLR_VO_SEL_RGB);//set vo sel = 1: rgb signal

		//Step4: set vo pin mux
		for(idx=0;idx<sizeof(pinmux_setting);idx++)
		{
			sclr_top_set_vo_data_mux(idx, pinmux_setting[idx]);
		}

		break;
	}
	case CFG_OP_PANEL_TYPE://76
	{

		u8 paneltype = 0;
		union vip_sys_clk_ctrl0 cfg;
		bm_get_num("Disp Panel Type, MIPI(0)/LVDS(0)/TTL(2) ", paneltype);

		cfg = vip_get_clk_ctrl0();
		cfg.b.bt_src_sel = paneltype;
		vip_set_clk_ctrl0(cfg);

		break;
	}
	case CFG_OP_PANEL_FRD_HW_MCU_TEST_CFG_ON_FPGA: //82
	{
		//H.W MCU flow by APN
		//1.Disable tgen
		//2.Set VO interface to HW MCU (RGB565 or RGB888)
		//3.send initial code by reg_sw_tx_n
		//4.enable tgen and normal display setting
		//5.auto send video data by H.W

		int idx = 0;
		u8 pinmux_setting[25]={15,15,15, //vo_data[3:0] no used in mcu mode
                               4,5,6,7,8,9,//vo_data[20:4], 5'd11~5'd4'h:mcu_data[7:0]
                               10,11,12,13,14,15,
                               16,17,18,19,20,21,
                               2,0,1,3}; //vo_data[24:21],5'h3~5'h:mcu_ctrl[3:0]
                               //WRX 0, DCS 1, CSX 2, RDX 3
		u8 paneltype = 2;
		union vip_sys_clk_ctrl0 cfg;

		cfg = vip_get_clk_ctrl0();
		cfg.b.bt_src_sel = paneltype;
		vip_set_clk_ctrl0(cfg);
		printf("felixP; test HW MCU mode\n");
		sclr_disp_tgen_enable(false);

		sclr_disp_set_intf(SCLR_VO_INTF_I80_HW);
		sclr_top_set_i80_if(false, I80_HW_FORMAT_RGB565);//I80_HW_FORMAT_RGB888

		for(idx=0;idx<ARRAY_SIZE(pinmux_setting);idx++)
		{
			sclr_top_set_vo_data_mux(idx, pinmux_setting[idx]);
		}

		i80_hw_set_combo_dev_cfg();

		i80_hw_init(i80_st7789v2_init_mcu_cmds, ARRAY_SIZE(i80_st7789v2_init_mcu_cmds));

		sclr_disp_reg_force_up();
		sclr_disp_enable_window_bgcolor(false);

		sclr_disp_set_timing(&panels[SCL_PANEL_FRD]);
		break;
		//sclr_disp_tgen_enable(true);
	}
	case CFG_OP_PANEL_FRD_HW_MCU_TEST_CFG_ON_EVB://90-> 83->84->set pattern
	{
		//H.W MCU flow by APN
		//1.Disable tgen
		//2.Set VO interface to HW MCU (RGB565 or RGB888)
		//3.send initial code by reg_sw_tx_n
		//4.enable tgen and normal display setting
		//5.auto send video data by H.W

		int idx = 0;
		u8 pinmux_setting[30]=
		{77,77,77,77,77,
		 77,0,1,2,77,
	     77,77,77,10,9,
		 8,7,6,5,4,
		 77,77,77,77,77,
		 3,77,77,0x30,77};

		union vip_sys_clk_ctrl0 cfg;

		pinmux_config(PINMUX_VO);
		cfg = vip_get_clk_ctrl0();

#if 0 //set original source from DISPPLL
		cfg.b.bt_src_sel = 2;//MIPI_TX: 0x00, DISPPLL: 0x10
		cfg.b.disp_sel_bt_div1 = 1; // 0:clk_disp is decided by clk_disp_div_cnt
									//1: clk_disp = clk_bt
		cfg.b.disp_div_cnt = 0;//div4:2, div2:0

#else // set source from MIPI_TX
		cfg.b.bt_src_sel = 0;//MIPI_TX: 0x00, DISPPLL: 0x10
		cfg.b.disp_sel_bt_div1 = 0; // 0:clk_disp is decided by clk_disp_div_cnt
									//1: clk_disp = clk_bt
		cfg.b.disp_div_cnt = 4;//div4:2, div2:0
		cfg.b.clk_disp_div_up_w1t = 1; //trigger by w1t = 1
#endif
		vip_set_clk_ctrl0(cfg);
		_reg_write(0xa0d106c, 0x102136); // set clk to 10MHZ

		sclr_disp_tgen_enable(false);
		sclr_disp_set_intf(SCLR_VO_INTF_I80_HW);
		sclr_top_set_i80_if(false, I80_HW_FORMAT_RGB565);//I80_HW_FORMAT_RGB888

		sclr_top_set_vo_data_mux_tmp(0,6);//MIPI_TXP2=6, VIVO_CLK=3

		for(idx=0;idx<ARRAY_SIZE(pinmux_setting);idx++)
		{
			sclr_top_set_vo_data_mux(idx, pinmux_setting[idx]);
		}
		_reg_write(0xa0d106c, 0x102136);

		i80_hw_set_combo_dev_cfg();
		i80_hw_init(i80_st7789v2_init_mcu_cmds, ARRAY_SIZE(i80_st7789v2_init_mcu_cmds));

		sclr_disp_reg_force_up();
		sclr_disp_enable_window_bgcolor(false);

		sclr_disp_set_timing(&panels[SCL_PANEL_FRD]);
		sclr_disp_tgen_enable(true);


		_reg_write(0xa08804c, 0x770002d0);
		_reg_write(0xa088058, 0x013f00ef);

		break;
	}
	case CFG_OP_PANEL_FRD_MCU_TEST_CFG: //80
	{
		int idx = 0;
		int ans = 0;
		int FRD_width = 240;
		int FRD_height = 320;

		u8 pinmux_setting[25]={15,15,15, //vo_data[3:0] no used in mcu mode
                               4,5,6,7,8,9,//vo_data[20:4], 5'd11~5'd4'h:mcu_data[7:0]
                               10,11,12,13,11,10,
                               9,8,7,6,5,4,
                               0,1,2,3}; //vo_data[24:21],5'h3~5'h:mcu_ctrl[3:0]

		u8 paneltype = 2;
		union vip_sys_clk_ctrl0 cfg;

		cfg = vip_get_clk_ctrl0();
		cfg.b.bt_src_sel = paneltype;
		vip_set_clk_ctrl0(cfg);

		for(idx=0;idx<ARRAY_SIZE(pinmux_setting);idx++)
		{
			sclr_top_set_vo_data_mux(idx, pinmux_setting[idx]);
		}
		if(in == NULL)
			in = malloc(FRD_width*FRD_height*3);
		if(out == NULL)
			out = malloc(FRD_width*FRD_height*3);

		printf("Panel: FRD280C50106-A \n");
		printf("FPAG connector: J10\n");
		printf("Pixel format: SW MCU RGB 565\n");
		printf("Image prepare: size:240x320 fmt:RGB \n");
		printf("Need to load image to addr(%p)\n",in);
		i80_set_combo_dev_cfg();

		bm_get_num("Please load 240x320 RGB to image addr (0:no image to load/1: image loaded)\n ", ans);
		if(!ans)
		{
			printf("Please load 240x320 RGB image first\n");
			break;
		}
		i80_set_pattern();

		i80_set_sw_mode(true);
		i80_init(i80_st7789v2_init_mcu_cmds, ARRAY_SIZE(i80_st7789v2_init_mcu_cmds));

		sclr_disp_reg_force_up();

		i80_set_run();

		sclr_disp_enable_window_bgcolor(false);

		sclr_disp_set_timing(&panels[SCL_PANEL_FRD]);

		break;
	}
	case CFG_OP_PANEL_FRD_HW_STEP7://85
	{
		sclr_i80_set_pre_cmd_en();
		break;
	}

	case CFG_OP_PANEL_FRD_HW_STEP8://86
	{
		sclr_disp_set_timing(&panels[SCL_PANEL_FRD]);
		break;
	}

	case CFG_OP_PANEL_FRD_HW_STEP9://87
	{
		sclr_disp_set_timing(&panels[SCL_PANEL_FRD]);
		break;
	}
	//test HW fix flow:
	//82-> mcu init
	//89 -> se cmd 0x2C & tx_num = 1
	//85 -> set pre cmd en = 1
	//88 -> tgen and hw mcu trig

	//set pattern(loop)
	case CFG_OP_PANEL_FRD_HW_STEP10://88
	{
		sclr_disp_set_timing(&panels[SCL_PANEL_FRD]);
		sclr_i80_set_trig();

		sclr_disp_tgen_enable(true);
		//sclr_i80_set_hfde();

		break;
	}
	case CFG_OP_PANEL_FRD_HW_SET_2C://83, seq 82-> 83 -> set pattern -> 84
	{
		sclr_i80_set_2c();
		break;
	}

	case CFG_OP_PANEL_FRD_HW_SET_2C_ONLY://89
	{
		sclr_i80_set_2c_only();

		break;
	}

	case CFG_OP_PANEL_FRD_HW_SET_TRIG://84
	{
		sclr_i80_set_trig();

		sclr_disp_tgen_enable(true);
		break;
	}

	case CFG_OP_PANEL_TXDT_RGB_4_SERIAL_TEST://81
	{
		u8 paneltype = 2,idx, perm_test = 0;

		union vip_sys_clk_ctrl0 cfg;
		u8 pinmux_setting[11]={24,25,26, //vs, hs, hde
                               16,17,18,19,20,21, 22,23};
		u8 pinmux_prem_idx_end=0;
		u8 pinmux_prem_idx_start=0;

		printf("Panel: TXDT200LER-31V\n");
		printf("FPAG connector: J10\n");
		printf("Pixel format: RGB 4 serial\n");
		printf("NOTE: Need SPI initial commmand before test!!!\n");
		printf("NOTE: Need SPI initial commmand before test!!!\n");
		printf("NOTE: Need SPI initial commmand before test!!!\n");

		sclr_disp_set_timing(&panels[SCL_PANEL_TXDX]);

		//Step2: Set T Gen
		sclr_disp_tgen_enable(1);

		//Step1: Set src sel
		cfg = vip_get_clk_ctrl0();
		cfg.b.bt_src_sel = paneltype;
		vip_set_clk_ctrl0(cfg);

		sclr_top_set_vo_type_sel(SCLR_VO_SEL_SERIAL_RGB);//set vo sel = serial RGB
		sclr_disp_set_srgb_ttl_en(true);
		sclr_disp_set_srgb_ttl_4x(true);

		//Step4: set vo pin mux
		for(idx=0;idx<sizeof(pinmux_setting);idx++)
		{
			sclr_top_set_vo_data_mux(idx, pinmux_setting[idx]);
		}

		bm_get_num("RGB pin mux perm test (1/0)\n ", perm_test);
		if(!perm_test)
			break;

		bm_get_num("VO pin mux test perm Start idx v2\n ", pinmux_prem_idx_start);
		bm_get_num("VO pin mux test perm End idx\n ", pinmux_prem_idx_end);

		//Step5: Test R/G/B channel order by perm
		count=0;
		if(0)
		_test_prem(pinmux_setting, pinmux_prem_idx_start, pinmux_prem_idx_end);

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

static int _cmdq_testcase(union cmdq_set *cmd_start, u8 op, u8 size, u32 pattern, u64 addr1, u64 addr2)
{
    int ret = 0;
    u8 cmd_idx = 0;

    switch (op) {
        case 0: // 0: different number of reg-write entries only
        case 1: // 1: different number of reg-write entries with sw-restart
        case 2: // 2: different number of reg-write entries with timer
            memset(cmd_start, 0, sizeof(u64) * size); // cmdset entry size: 64bit
            cmd_idx = generate_cmdset(cmd_start, addr1, size, pattern, op, true);

            cmdQ_intr_ctrl(REG_SCL_CMDQ_BASE, 0x07);
            enable_cmdq = true;
            sclr_timer_tick = timer_get_tick();
            cmdQ_engine(REG_SCL_CMDQ_BASE, (uintptr_t)cmdq_set , addr1 >> 22, false, false, cmd_idx);

#ifndef ENV_EMU
            if (sclr_check_irq_handler(30) != 0) {
                uartlog("cmdq end intr doesn't happens after 30 tick\n");
                ret = -1;
            } else {
                uartlog("cmdq done and start compare\n");
                u8 count = reg_compare(addr1, size, pattern);
                uartlog("compare result: mismatch(%d)\n", count);
                if (count != 0) ret = -1;
            }
#endif
            break;

        case 3:
        {
            struct cmdq_adma *adma = (void *)(cmdq_set);
            cmdQ_intr_ctrl(REG_SCL_CMDQ_BASE, 0x07);
            memset(adma, 0, (sizeof(u64) * 2) * 2); // adma table entry size: 128bit
            memset(cmd_start+0x20, 0, sizeof(u64) * size);
            cmd_idx = generate_cmdset(cmd_start+0x20, addr1, size, pattern, 0, false);
            cmdQ_adma_package(adma, (uintptr_t)(cmd_start+0x20), 8*size, false, false);
            memset(cmd_start+0x40, 0, sizeof(u64) * size);
            cmd_idx = generate_cmdset(cmd_start+0x40, addr2, size, pattern, 0, true);
            cmdQ_adma_package(adma+1, (uintptr_t)(cmd_start+0x40), 8*size, false, true);

            enable_cmdq = true;
            sclr_timer_tick = timer_get_tick();
            cmdQ_engine(REG_SCL_CMDQ_BASE, (uintptr_t)adma , REG_SCL_TOP_BASE >> 22, false, true, 0);

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
 *       cli commands
 ********************************************************************/
static int do_cfg(int argc, char **argv)
{
    u32 tmp;
    u8 op = 0xff;
    do {
        uartlog("\n--------------------------------------------------------------------\n");
        uartlog("00) img_in_cfg 01) img_in_mem   02) img_in_start    03) set blend\n");
        uartlog("04) img_in_checksum\n");
        uartlog("10) sc_cfg     11) sc_set_src   12) sc_set_crop     13) sc_set_out\n");
        uartlog("14) sc_set_bd  15) set privacy  16) sc_checksum     17) sc_set_osdenc\n");
        uartlog("18) sc_set_cover\n");
        uartlog("20) odma_cfg   21) odma_mem\n");
        uartlog("30) gop_cfg    31) gop_ow_cfg   32) gop_LUT_cfg     33) gop_scale_cfg\n");
        uartlog("34) gop_osddec 35) cover\n");
        uartlog("40) disp_cfg   41) disp_src     42) disp_tgen       43) lvdstx\n");
        uartlog("44) bt         45) il9881 init  46) disp gamma      47) hx8394 init \n");
        uartlog("48) DSI-LP     49) sned dcs cmd 50) disp checksum\n");
        uartlog("60) All status 61) top status   62) img_in status   63) sc status\n");
        uartlog("64) gop status 65) odma status  66) disp status     67) lvdstx status\n");
        uartlog("70) reg dump   71) top reg      72) img_in reg      73) sc core reg\n");
        uartlog("74) odma reg   75) disp reg     77) TTL RGB  \n");
        uartlog("80) FRD sw mcu 81) TXDT 4 RGB \n");
        uartlog("255) exit\n");
        bm_get_num(": ", op);

        u8 num_args = 0;
        if (op < 10) {
            enum sclr_img_in img_inst;
            bm_get_num("img_in inst, d(1)/v(0): ", img_inst);
            if (img_inst > 1) {
                uartlog("img_inst inst(%d) does not exist.\n", img_inst);
                return -1;
            }
            if (_handle_cfg_op(op, img_inst, num_args) != 0) break;
        } else if (op < 30) {
	        int inst;
	        if ( op != 17) { //Only 1 OSD encoder(gop_compr) in gop_top.
                uartlog("inst, sc_d(0), sc_v1(1), sc_v2(2), sc_v3(3), disp(4)\n");
                bm_get_num("inst: ", inst);
                if (inst > SCL_GOP_MAX) {
                    uartlog("inst(%d) does not exist.\n", inst);
                    return -1;
                }
            }
            if (_handle_cfg_op(op, inst, num_args) != 0) break;
        } else if (op < 40) {
            int inst;
            uartlog("sc inst (0~%d)/disp inst(%d)\n", SCL_MAX_INST - 1, SCL_MAX_INST);
            bm_get_num("gop inst on sc/disp: ", inst);
            if (inst > SCL_MAX_INST) {
                uartlog("sc inst(%d) does not exist.\n", inst);
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

static int do_move(int argc, char **argv)
{
    int ret = 0;
    u8 inst;
    u32 tmp;
    struct sclr_scale_cfg cfg;
    struct sclr_top_cfg *top_cfg = sclr_top_get_cfg();
    memset(&cfg, 0, sizeof(cfg));

    inst = strtol(argv[1], NULL, 10);

    if (inst > SCL_MAX_INST) {
        uartlog("invalid instance-id(%d). Only 0~%d available.\n", inst, SCL_MAX_INST);
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
    enum sclr_img_in img_inst;
    u8 num_args = 0;
    if (inst == 0) {
        img_inst = SCL_IMG_D;
    } else {
        img_inst = SCL_IMG_V;
    }
    uartlog("\n  --img_in--\n");
    _handle_cfg_op(CFG_OP_IMG_CFG, img_inst);
    num_args = 2;
    _handle_cfg_op(CFG_OP_IMG_SET_MEM, img_inst, num_args, cfg.src.w, cfg.src.h);

    // setup odma
    uartlog("\n  --odma--\n");
    _handle_cfg_op(CFG_OP_ODMA_CFG, inst);
    num_args = 2;
    _handle_cfg_op(CFG_OP_ODMA_SET_MEM, inst, num_args, cfg.dst.w, cfg.dst.h);

    if (top_cfg->disp_from_sc) {
        struct sclr_rect rect;
        rect.x = rect.y = 0;
        rect.w = cfg.dst.w;
        rect.h = cfg.dst.h;
        sclr_disp_set_rect(rect);
    }

    //struct sclr_odma_cfg *odma_cfg = sclr_odma_get_cfg(inst);
    //if (odma_cfg->mem.pitch_y)
    //    memset((char *)dst_addr_scl[0][R_IDX], 0, odma_cfg->mem.pitch_y * odma_cfg->mem.height);

    if (cfg.tile_enable)
	sclr_left_tile(inst);

    sclr_top_reg_done();
    _intr_setup(inst, img_inst);

	if (!sc_init) {
		sclr_ctrl_init();
		request_irq(SC_INTR_0, irq_handler_scl, 0, "scl int", NULL);
		uartlog("%s: request_irq(%d)\n", __func__, SC_INTR_0);
	}

	sclr_trig_start(img_inst);

#ifndef ENV_EMU
	if (sclr_check_irq_handler(10000) != 0) {
		uartlog("sclr end intr doesn't happens after 10000 tick\n");
		ret = -1;

		//sclr_dump_top_register();
		//sclr_dump_disp_register();
		//sclr_dump_img_in_register(img_inst);
		//sclr_dump_core_register(inst);
	} else {
		uartlog("sclr done\n");
		uartlog("sc(%d) odma status(%#x)\n", inst, sclr_odma_get_dbg_status(inst).raw);
		if (cfg.tile_enable) {
			bm_get_num("Press Enter to continue...\n", tmp);
			sclr_right_tile(inst);
			sclr_trig_start(img_inst);
		}
	}
#endif

    return ret;
}

static int do_cir(int argc, char **argv)
{
    int ret = 0;
    u8 inst = 0;
    u32 tmp;
    struct sclr_cir_cfg cfg;

    inst = strtol(argv[1], NULL, 10);

	if (inst > SCL_MAX_INST) {
		uartlog("invalid instance-id(%d). Only 0~%d available.\n", inst, SCL_MAX_INST);
		return -1;
	}

	bm_get_num("Circle mode(0: empty, 2: shape, 3: line, 4: disable): ", cfg.mode);
    if (cfg.mode < SCL_CIR_DISABLE) {
        bm_get_num("Circle line width: ", cfg.line_width);
        bm_get_num("Circle center point x: ", cfg.center.x);
        bm_get_num("Circle center point y: ", cfg.center.y);
        bm_get_num("Circle radius: ", cfg.radius);
        bm_get_num("Circle frame width: ", cfg.rect.w);
        bm_get_num("Circle frame height: ", cfg.rect.h);
        bm_get_num("Circle offset x: ", cfg.rect.x);
        bm_get_num("Circle offset y: ", cfg.rect.y);
        bm_get_num("Circle color r(0~255): ", cfg.color_r);
        bm_get_num("Circle color g(0~255): ", cfg.color_g);
        bm_get_num("Circle color b(0~255): ", cfg.color_b);
    }

    sclr_cir_set_cfg(inst, &cfg);

    return ret;
}

static int do_pat(int argc, char **argv)
{
    enum sclr_disp_pat_type type;
    enum sclr_disp_pat_color color;
    u16 rgb[3];

    uartlog("1st param: 0(Full) 1(H_Grad) 2(V_Grad) 3(auto) 4(snow) 5(off)\n");
    uartlog("2nd param: 0(White) 1(Red) 2(Green) 3(Blue) 4(Cyan) 5(Magenta) 6(Yellow) 7(Colorbar)\n");

    type = strtol(argv[1], NULL, 10);
    color = strtol(argv[2], NULL, 10);

    sclr_disp_set_pattern(type, color, rgb);

    return 0;
}

static int do_show_addr(int argc, char **argv)
{
    int i = 0;

    for(i = 0 ; i < 3 ; ++i) {
        uartlog("sclr src buf[%d] -- 0x%lx, 0x%lx, 0x%lx\n", i, src_addr_scl[i][0], src_addr_scl[i][1], src_addr_scl[i][2]);
        uartlog("sclr dst buf[%d] -- 0x%lx, 0x%lx, 0x%lx\n", i, dst_addr_scl[i][0], dst_addr_scl[i][1], dst_addr_scl[i][2]);
    }
    return 0;
}

static int do_cmdq(int argc, char **argv)
{
    int ret = 0;
    union cmdq_set *cmd_start = (union cmdq_set *)cmdq_set;
    int size;
    int pattern;
    u8 op = strtol(argv[1], NULL, 10);
    u32 tmp;

    if ((op == 0) || (op == 1) || (op == 2)) {
        u64 addr = 0;
        uartlog("test reg-addr(hex)? 0x");
        scanf("%lx", &addr);
        bm_get_num("How many entries? ", size);
        uartlog("input pattern(hex)? ");
        scanf("%x", &pattern);

        ret = _cmdq_testcase(cmd_start, op, size, pattern, addr, 0);
    } else if (op == 3) {
        u64 addr1 = 0, addr2 = 0;
        uartlog("test reg-addr1(hex)? ");
        scanf("%lu", &addr1);
        uartlog("test reg-addr2(hex)? ");
        scanf("%lu", &addr2);
        bm_get_num("Size of cmdset? ", size);
        for (int i = 0; i < 3; ++i) {
            pattern = (i & 0x01) ? 0x00001234 : 0x00002234;
            ret = _cmdq_testcase(cmd_start, op, size, pattern, addr1, addr2);
        }
    } else if (op == 4) {
        bm_get_num("How many times of loop? ", tmp);
        for (int i = 0; i < tmp; ++i) {
            uartlog("stress test(%d)\n", i);
            pattern = (i & 0x01) ? 0x00001234 : 0x00002234;
            if ((ret = _cmdq_testcase(cmd_start, 0, 9, pattern, REG_SCL_IMG_CSC_COEF0(0), 0)) == -1) break;
            if ((ret = _cmdq_testcase(cmd_start, 0, 9, pattern, REG_SCL_IMG_CSC_COEF0(1), 0)) == -1) break;
            if ((ret = _cmdq_testcase(cmd_start, 0, 6, pattern, REG_SCL_CSC_COEF0(0), 0)) == -1) break;
            if ((ret = _cmdq_testcase(cmd_start, 0, 6, pattern, REG_SCL_CSC_COEF0(1), 0)) == -1) break;
            if ((ret = _cmdq_testcase(cmd_start, 0, 6, pattern, REG_SCL_CSC_COEF0(2), 0)) == -1) break;
            if ((ret = _cmdq_testcase(cmd_start, 0, 6, pattern, REG_SCL_CSC_COEF0(3), 0)) == -1) break;
            if ((ret = _cmdq_testcase(cmd_start, 0, 11, pattern, REG_SCL_DISP_OUT_CSC0 , 0)) == -1) break;
            if ((ret = _cmdq_testcase(cmd_start, 1, 8, pattern, REG_SCL_DISP_OUT_CSC0, 0)) == -1) break;
            if ((ret = _cmdq_testcase(cmd_start, 2, 8, pattern, REG_SCL_DISP_OUT_CSC0, 0)) == -1) break;
            if ((ret = _cmdq_testcase(cmd_start, 3, 7, pattern, REG_SCL_DISP_OUT_CSC0, REG_SCL_DISP_IN_CSC0)) == -1) break;
        }
    } else if (op == 5) {
        // test image: 2k-16.jpg
        //   0x90000000: 2k-16.ppm_Y20
        //   0x91000000: 2k-16.ppm_U20
        //   0x92000000: 2k-16.ppm_V20
        // 1: img_v in, scl_v0 out
        // 2: img_v in, scl_v0 out (offset to compose)
        struct sclr_ctrl_cfg cfgs[2];
        union sclr_intr mask;
        memset(cfgs, 0, sizeof(cfgs));
        memset(&mask, 0, sizeof(mask));

        cfgs[0].img_inst = SCL_IMG_D;
        cfgs[0].input.d = SCL_INPUT_D_MEM;
        cfgs[0].src_fmt = SCL_FMT_YUV420;
        cfgs[0].src_csc = SCL_CSC_709_LIMIT_YUV2RGB;
        cfgs[0].src.w = 1920;
        cfgs[0].src.h = 1080;
        cfgs[0].src_crop.x = 1200;
        cfgs[0].src_crop.y = 96;
        cfgs[0].src_crop.w = 240;
        cfgs[0].src_crop.h = 240;
        cfgs[0].src_addr0 = src_addr_scl[0][R_IDX];
        cfgs[0].src_addr1 = src_addr_scl[0][G_IDX];
        cfgs[0].src_addr2 = src_addr_scl[0][B_IDX];
        cfgs[0].dst[0].inst = 0;
        cfgs[0].dst[0].fmt = SCL_FMT_RGB_PLANAR;
        cfgs[0].dst[0].csc = SCL_CSC_NONE;
        cfgs[0].dst[0].frame.w = 112;
        cfgs[0].dst[0].frame.h = 1120;
        cfgs[0].dst[0].offset.x = 0;
        cfgs[0].dst[0].offset.y = 0;
        cfgs[0].dst[0].window.w = 112;
        cfgs[0].dst[0].window.h = 112;
        cfgs[0].dst[0].addr0 = dst_addr_scl[0][R_IDX];
        cfgs[0].dst[0].addr1 = dst_addr_scl[0][G_IDX];
        cfgs[0].dst[0].addr2 = dst_addr_scl[0][B_IDX];
        cfgs[0].dst[1].inst = SCL_MAX_INST;
        cfgs[0].dst[2].inst = SCL_MAX_INST;
        cfgs[0].dst[3].inst = SCL_MAX_INST;

        cfgs[1].img_inst = SCL_IMG_D;
        cfgs[1].input.d = SCL_INPUT_D_MEM;
        cfgs[1].src_fmt = SCL_FMT_YUV420;
        cfgs[1].src_csc = SCL_CSC_709_LIMIT_YUV2RGB;
        cfgs[1].src.w = 1920;
        cfgs[1].src.h = 1080;
        cfgs[1].src_crop.x = 1646;
        cfgs[1].src_crop.y = 354;
        cfgs[1].src_crop.w = 110;
        cfgs[1].src_crop.h = 130;
        cfgs[1].src_addr0 = src_addr_scl[0][R_IDX];
        cfgs[1].src_addr1 = src_addr_scl[0][G_IDX];
        cfgs[1].src_addr2 = src_addr_scl[0][B_IDX];
        cfgs[1].dst[0].inst = 0;
        cfgs[1].dst[0].fmt = SCL_FMT_RGB_PLANAR;
        cfgs[1].dst[0].csc = SCL_CSC_NONE;
        cfgs[1].dst[0].frame.w = 112;
        cfgs[1].dst[0].frame.h = 1120;
        cfgs[1].dst[0].offset.x = 0;
        cfgs[1].dst[0].offset.y = 112;
        cfgs[1].dst[0].window.w = 112;
        cfgs[1].dst[0].window.h = 112;
        cfgs[1].dst[0].addr0 = dst_addr_scl[0][R_IDX];
        cfgs[1].dst[0].addr1 = dst_addr_scl[0][G_IDX];
        cfgs[1].dst[0].addr2 = dst_addr_scl[0][B_IDX];
        cfgs[1].dst[1].inst = SCL_MAX_INST;
        cfgs[1].dst[2].inst = SCL_MAX_INST;
        cfgs[1].dst[3].inst = SCL_MAX_INST;

        enable_cmdq = true;
        // disable scl's intr to have no scl's irq during process of cmdq
        mask.b.cmdq = true;
        sclr_set_intr_mask(mask);
        sclr_engine_cmdq(cfgs, 2, cmdq_set);
    } else if (op == 6) {
        // test image: 2k-16.jpg
        // 1: img_v in, scl_v0 out
        // 2: img_v in, scl_v0/1/2 out
        struct sclr_ctrl_cfg cfgs[2];
        union sclr_intr mask;
        memset(cfgs, 0, sizeof(cfgs));
        memset(&mask, 0, sizeof(mask));

        cfgs[0].img_inst = SCL_IMG_V;
        cfgs[0].input.d = SCL_INPUT_V_MEM;
        cfgs[0].src_fmt = SCL_FMT_YUV420;
        cfgs[0].src_csc = SCL_CSC_709_LIMIT_YUV2RGB;
        cfgs[0].src.w = 1920;
        cfgs[0].src.h = 1080;
        cfgs[0].src_crop.x = 1200;
        cfgs[0].src_crop.y = 96;
        cfgs[0].src_crop.w = 240;
        cfgs[0].src_crop.h = 240;
        cfgs[0].src_addr0 = src_addr_scl[0][R_IDX];
        cfgs[0].src_addr1 = src_addr_scl[0][G_IDX];
        cfgs[0].src_addr2 = src_addr_scl[0][B_IDX];
        cfgs[0].dst[0].inst = 1;
        cfgs[0].dst[0].fmt = SCL_FMT_RGB_PLANAR;
        cfgs[0].dst[0].csc = SCL_CSC_NONE;
        cfgs[0].dst[0].frame.w = 112;
        cfgs[0].dst[0].frame.h = 1120;
        cfgs[0].dst[0].offset.x = 0;
        cfgs[0].dst[0].offset.y = 0;
        cfgs[0].dst[0].window.w = 112;
        cfgs[0].dst[0].window.h = 112;
        cfgs[0].dst[0].addr0 = 0x106000000;//dst_addr_scl[0][R_IDX];
        cfgs[0].dst[0].addr1 = 0x107000000;//dst_addr_scl[0][G_IDX];
        cfgs[0].dst[0].addr2 = 0x108000000;//dst_addr_scl[0][B_IDX];
        cfgs[0].dst[1].inst = SCL_MAX_INST;
        cfgs[0].dst[2].inst = SCL_MAX_INST;
        cfgs[0].dst[3].inst = SCL_MAX_INST;

        cfgs[1].img_inst = SCL_IMG_V;
        cfgs[1].input.d = SCL_INPUT_V_MEM;
        cfgs[1].src_fmt = SCL_FMT_YUV420;
        cfgs[1].src_csc = SCL_CSC_709_LIMIT_YUV2RGB;
        cfgs[1].src.w = 1920;
        cfgs[1].src.h = 1080;
        cfgs[1].src_crop.x = 0;
        cfgs[1].src_crop.y = 0;
        cfgs[1].src_crop.w = 1920;
        cfgs[1].src_crop.h = 1080;
        cfgs[1].src_addr0 = src_addr_scl[0][R_IDX];
        cfgs[1].src_addr1 = src_addr_scl[0][G_IDX];
        cfgs[1].src_addr2 = src_addr_scl[0][B_IDX];
        cfgs[1].dst[0].inst = 1;
        cfgs[1].dst[0].fmt = SCL_FMT_RGB_PLANAR;
        cfgs[1].dst[0].csc = SCL_CSC_NONE;
        cfgs[1].dst[0].frame.w = 1920;
        cfgs[1].dst[0].frame.h = 1080;
        cfgs[1].dst[0].offset.x = 0;
        cfgs[1].dst[0].offset.y = 0;
        cfgs[1].dst[0].window.w = 1920;
        cfgs[1].dst[0].window.h = 1080;
        cfgs[1].dst[0].addr0 = dst_addr_scl[0][R_IDX];
        cfgs[1].dst[0].addr1 = dst_addr_scl[0][G_IDX];
        cfgs[1].dst[0].addr2 = dst_addr_scl[0][B_IDX];
        cfgs[1].dst[1].inst = 2;
        cfgs[1].dst[1].fmt = SCL_FMT_RGB_PLANAR;
        cfgs[1].dst[1].csc = SCL_CSC_NONE;
        cfgs[1].dst[1].frame.w = 1280;
        cfgs[1].dst[1].frame.h = 720;
        cfgs[1].dst[1].offset.x = 0;
        cfgs[1].dst[1].offset.y = 0;
        cfgs[1].dst[1].window.w = 1280;
        cfgs[1].dst[1].window.h = 720;
        cfgs[1].dst[1].addr0 = dst_addr_scl[1][R_IDX];
        cfgs[1].dst[1].addr1 = dst_addr_scl[1][G_IDX];
        cfgs[1].dst[1].addr2 = dst_addr_scl[1][B_IDX];
        cfgs[1].dst[2].inst = 3;
        cfgs[1].dst[2].fmt = SCL_FMT_RGB_PLANAR;
        cfgs[1].dst[2].csc = SCL_CSC_NONE;
        cfgs[1].dst[2].frame.w = 640;
        cfgs[1].dst[2].frame.h = 480;
        cfgs[1].dst[2].offset.x = 0;
        cfgs[1].dst[2].offset.y = 0;
        cfgs[1].dst[2].window.w = 640;
        cfgs[1].dst[2].window.h = 480;
        cfgs[1].dst[2].addr0 = dst_addr_scl[2][R_IDX];
        cfgs[1].dst[2].addr1 = dst_addr_scl[2][G_IDX];
        cfgs[1].dst[2].addr2 = dst_addr_scl[2][B_IDX];
        cfgs[1].dst[3].inst = SCL_MAX_INST;

        enable_cmdq = true;
        // disable scl's intr to have no scl's irq during process of cmdq
        mask.b.cmdq = true;
        sclr_set_intr_mask(mask);
        sclr_engine_cmdq(cfgs, 2, cmdq_set);
    }

    return ret;
}

static int do_perf(int argc, char **argv)
{
    struct sclr_ctrl_cfg cfg;
    struct sclr_size size[4] = {
        {.w = 320, .h = 240},
        {.w = 640, .h = 480},
        {.w = 1280, .h = 720},
        {.w = 1920, .h = 1080},
    };
    union sclr_intr mask;
    memset(&cfg, 0, sizeof(cfg));
    memset(&mask, 0, sizeof(mask));

    cfg.img_inst = SCL_IMG_D;
    cfg.input.d = SCL_INPUT_D_MEM;
    cfg.src_addr0 = src_addr_scl[0][R_IDX];
    cfg.src_addr1 = src_addr_scl[0][G_IDX];
    cfg.src_addr2 = src_addr_scl[0][B_IDX];
    cfg.dst[0].addr0 = dst_addr_scl[0][R_IDX];
    cfg.dst[0].addr1 = dst_addr_scl[0][G_IDX];
    cfg.dst[0].addr2 = dst_addr_scl[0][B_IDX];
    cfg.dst[1].inst = SCL_MAX_INST;
    cfg.dst[2].inst = SCL_MAX_INST;
    cfg.dst[3].inst = SCL_MAX_INST;

    // disable scl's intr to have no scl's irq during process of cmdq
    mask.b.cmdq = true;
    sclr_set_intr_mask(mask);

    for (int i = 0; i < 4; ++i) {
        cfg.src_fmt = SCL_FMT_YUV420;
        cfg.src_csc = SCL_CSC_709_LIMIT_YUV2RGB;
        cfg.src = size[i];
        cfg.src_crop.x = 0;
        cfg.src_crop.y = 0;
        cfg.src_crop.w = size[i].w;
        cfg.src_crop.h = size[i].h;
        cfg.dst[0].inst = 0;
        cfg.dst[0].fmt = SCL_FMT_RGB_PLANAR;
        cfg.dst[0].csc = SCL_CSC_NONE;
        cfg.dst[0].frame = size[i];
        cfg.dst[0].offset.x = 0;
        cfg.dst[0].offset.y = 0;
        cfg.dst[0].window = size[i];

        enable_cmdq = true;
        uartlog("Perf(YUV420-to-RGB %d*%d):\n", size[i].w, size[i].h);
        sclr_timer_tick = timer_get_tick();
        sclr_engine_cmdq(&cfg, 1, cmdq_set);

#ifndef ENV_EMU
        if (sclr_check_irq_handler(60) != 0) {
            uartlog("sclr end intr doesn't happens after 60 tick\n");
        }
#endif
    }

    for (int i = 0; i < 4; ++i) {
        cfg.src_fmt = SCL_FMT_RGB_PLANAR;
        cfg.src_csc = SCL_CSC_NONE;
        cfg.src = size[i];
        cfg.src_crop.x = 0;
        cfg.src_crop.y = 0;
        cfg.src_crop.w = size[i].w;
        cfg.src_crop.h = size[i].h;
        for (int j = 0; j < 4; ++j) {
            cfg.dst[0].inst = 0;
            cfg.dst[0].fmt = SCL_FMT_RGB_PLANAR;
            cfg.dst[0].csc = SCL_CSC_NONE;
            cfg.dst[0].frame = size[j];
            cfg.dst[0].offset.x = 0;
            cfg.dst[0].offset.y = 0;
            cfg.dst[0].window = size[j];

            enable_cmdq = true;
            uartlog("Perf(Resize %d*%d-to-%d*%d):\n", size[i].w, size[i].h, size[j].w, size[j].h);
            sclr_timer_tick = timer_get_tick();
            sclr_engine_cmdq(&cfg, 1, cmdq_set);

#ifndef ENV_EMU
            if (sclr_check_irq_handler(60) != 0) {
                uartlog("sclr end intr doesn't happens after 60 tick\n");
            }
#endif
        }
    }
    return 0;
}

static float bf16tofloat(u8 *bf16)
{
	float f;
	u8 *tmp = (u8 *)&f;

	*(tmp + 3) = *(bf16 + 1);
	*(tmp + 2) = *(bf16);
	*(tmp + 1) = 0;
	*(tmp + 0) = 0;

	return f;
}

static int do_rounding(int argc, char **argv)
{
    u32 tmp;
    u8 inst;
    bm_get_num("inst(0~3): ", inst);
    sclr_set_cfg(inst, /*sc_bypass*/ true, /*gop_bypass*/ true,
		 /*cir_bypass*/ true, /*odma_bypass*/ false);

    enum sclr_img_in img_inst = (inst == 0) ? SCL_IMG_D : SCL_IMG_V;
    union sclr_input in;
    in.v = SCL_INPUT_D_MEM;
    sclr_ctrl_set_input(img_inst, in, SCL_FMT_RGB_PLANAR, SCL_CSC_NONE);

    struct sclr_mem mem;
    mem.width = 32;
    mem.height = 32;
    mem.start_x = 0;
    mem.start_y = 0;
    mem.addr0 = src_addr_scl[0][R_IDX];
    mem.addr1 = src_addr_scl[0][G_IDX];
    mem.addr2 = src_addr_scl[0][B_IDX];
    mem.pitch_y = VIP_ALIGN(mem.width);
    mem.pitch_c = mem.pitch_y;
    sclr_img_set_mem(img_inst, &mem);

    struct sclr_odma_cfg *cfg = sclr_odma_get_cfg(inst);
    cfg->flip = SCL_FLIP_NO;
    cfg->fmt = SCL_FMT_RGB_PLANAR;
    sclr_odma_set_cfg(inst, cfg);

    mem.addr0 = dst_addr_scl[0][R_IDX];
    mem.addr1 = dst_addr_scl[0][G_IDX];
    mem.addr2 = dst_addr_scl[0][B_IDX];
    sclr_odma_set_mem(inst, &mem);

    sclr_top_reg_done();
    _intr_setup(inst, img_inst);

    #define NUM_OF_INPUT 2
    #define NUM_OF_CASES 4
    u8 test_values[NUM_OF_INPUT][3] = {
        {69, 55, 75},
        {23, 24, 25},
    };
    for (int i = 0 ; i < NUM_OF_INPUT ; ++i) {
        u8 *addr = (u8 *)src_addr_scl[0][R_IDX];
        *(addr + i) = test_values[i][0];
        addr = (u8 *)src_addr_scl[0][G_IDX];
        *(addr + i) = test_values[i][1];
        addr = (u8 *)src_addr_scl[0][B_IDX];
        *(addr + i) = test_values[i][2];
    }

    u8 test_results[NUM_OF_CASES][NUM_OF_INPUT][3] = {
        {
            {12,  3, 12},
            {-12, -12, -12},
        },
        {
            {12,  3, 13},	// 11.5, 3.3, 12.5
            {-12, -12, -13},	// -11.5, -12.2, -12.5
        },
        {
            {-45,  -70, -60},	// -44.9166, -69.664, -59.6592
            {-84, -96, -102},
        },
        {
            {18, 20, 0},	// 18.4, 20.9, -0.25
            {-9, -4, -16},	// -9.2, -4.52, -16.75
        },
    };

    float test_results_f[NUM_OF_CASES][NUM_OF_INPUT][3] = {
        {
            {11.500000, 3.296875, 12.500000},
            {-11.500000, -12.187500, -12.500000},
        },
        {
            {11.500000, 3.296875, 12.500000},		// 11.5, 3.3, 12.5
            {-11.500000, -12.187500, -12.500000},
        },
        {
            {-45.000000, -69.500000, -59.750000},	// -44.9166, -69.664, -59.6592
            {-83.500000, -96.000000, -101.500000},	// -83.6256, -95.7505, -101.7342
        },
        {
            {18.375000, 20.875000, 0.253906},		// 18.4, 20.9, -0.25
            {-9.187500, -4.531250, -16.750000},		// -9.2, -4.52, -16.75
        },
    };
    float param[NUM_OF_CASES][7] = {
        {0.5, 0.5, 0.5, 23.0, 24.2, 25.0, SCL_QUANT_ROUNDING_TO_EVEN},
        {0.5, 0.5, 0.5, 23.0, 24.2, 25.0, SCL_QUANT_ROUNDING_AWAY_FROM_ZERO},
        {0.8415, 0.8415, 0.8415, 102.9801, 115.9465, 122.7717, SCL_QUANT_ROUNDING_TO_EVEN},
        {0.6, 0.82, 0.33, 23.0, 24.2, 25.0,  SCL_QUANT_ROUNDING_TRUNCATE},
    };

    // TODO: Add param
    cfg->csc_cfg.mode = SCL_OUT_QUANT;
    cfg->csc_cfg.work_on_border = false;
    cfg->csc_cfg.quant_gain_mode = SCL_QUANT_GAIN_MODE_1; // reg_q_gain_mode = 0
    for (int i = 0 ; i < NUM_OF_CASES ; ++i) {
        uartlog("------ round %d ------\n", i);

        cfg->csc_cfg.quant_form.sc_frac[0] = param[i][0] * 8192;
        cfg->csc_cfg.quant_form.sc_frac[1] = param[i][1] * 8192;
        cfg->csc_cfg.quant_form.sc_frac[2] = param[i][2] * 8192;
        cfg->csc_cfg.quant_form.sub[0] = (u8)param[i][3];
        cfg->csc_cfg.quant_form.sub_frac[0] = (param[i][3]- cfg->csc_cfg.quant_form.sub[0]) * 1024;
        cfg->csc_cfg.quant_form.sub[1] = (u8)param[i][4];
        cfg->csc_cfg.quant_form.sub_frac[1] = (param[i][4]- cfg->csc_cfg.quant_form.sub[1]) * 1024;
        cfg->csc_cfg.quant_form.sub[2] = (u8)param[i][5];
        cfg->csc_cfg.quant_form.sub_frac[2] = (param[i][5]- cfg->csc_cfg.quant_form.sub[2]) * 1024;
        cfg->csc_cfg.quant_round = param[i][6];
        sclr_ctrl_set_output(inst, &cfg->csc_cfg, cfg->fmt);

	sclr_trig_start(img_inst);

#ifndef ENV_EMU
        if (sclr_check_irq_handler(60) != 0) {
            uartlog("sclr end intr doesn't happens after 60 tick\n");
	    break;
        } else {
            uartlog("sclr done\n");
            uartlog("sc(%d) odma status(%#x)\n", inst, sclr_odma_get_dbg_status(inst).raw);
	    bool mismatch = false;
            for (int j = 0; j < NUM_OF_INPUT; ++j) {
                u8 *addr = (u8 *)dst_addr_scl[0][R_IDX];
                u8 r, g, b;
                r = *(addr + j);
                addr = (u8 *)dst_addr_scl[0][G_IDX];
                g = *(addr + j);
                addr = (u8 *)dst_addr_scl[0][B_IDX];
                b = *(addr + j);
                if (r != test_results[i][j][0] || g != test_results[i][j][1] || b != test_results[i][j][2]) {
                    uartlog("case%d result%d output(%x, %x, %x) expect(%x, %x, %x) mismatch \n",
                        i, j, r, g, b, test_results[i][j][0], test_results[i][j][1], test_results[i][j][2]);
                    mismatch = true;
                }
            }
            if (mismatch) break;
        }
#endif
    }

	mem.pitch_y = VIP_ALIGN(mem.width * 2);
	mem.pitch_c = mem.pitch_y;
	sclr_odma_set_mem(inst, &mem);
	cfg->fmt = SCL_FMT_BF16;
	cfg->csc_cfg.mode = SCL_OUT_QUANT_BF16;
	cfg->csc_cfg.work_on_border = false;
	cfg->csc_cfg.quant_border_type = SCL_QUANT_BORDER_TYPE_255;
	cfg->csc_cfg.quant_gain_mode =
		SCL_QUANT_GAIN_MODE_1; // reg_q_gain_mode = 0
	printf("size of float: %zu\n", sizeof(float));
	for (int i = 0 ; i < NUM_OF_CASES ; ++i) {
	    uartlog("------ round %d ------\n", i);

	    cfg->csc_cfg.quant_form.sc_frac[0] = param[i][0] * 8192;
	    cfg->csc_cfg.quant_form.sc_frac[1] = param[i][1] * 8192;
	    cfg->csc_cfg.quant_form.sc_frac[2] = param[i][2] * 8192;
	    cfg->csc_cfg.quant_form.sub[0] = (u8)param[i][3];
	    cfg->csc_cfg.quant_form.sub_frac[0] = (param[i][3]- cfg->csc_cfg.quant_form.sub[0]) * 1024;
	    cfg->csc_cfg.quant_form.sub[1] = (u8)param[i][4];
	    cfg->csc_cfg.quant_form.sub_frac[1] = (param[i][4]- cfg->csc_cfg.quant_form.sub[1]) * 1024;
	    cfg->csc_cfg.quant_form.sub[2] = (u8)param[i][5];
	    cfg->csc_cfg.quant_form.sub_frac[2] = (param[i][5]- cfg->csc_cfg.quant_form.sub[2]) * 1024;
	    cfg->csc_cfg.quant_round = param[i][6];
	    sclr_ctrl_set_output(inst, &cfg->csc_cfg, cfg->fmt);

	    sclr_trig_start(img_inst);

#ifndef ENV_EMU
	    if (sclr_check_irq_handler(60) != 0) {
		uartlog("sclr end intr doesn't happens after 60 tick\n");
		break;
	    } else {
		uartlog("sclr done\n");
		uartlog("sc(%d) odma status(%#x)\n", inst, sclr_odma_get_dbg_status(inst).raw);
		bool mismatch = false;

		for (int j = 0; j < NUM_OF_INPUT; ++j) {
		    u8 *addr;
		    float r, g, b;
                    s32 dr, dg, db;

		    addr = (u8 *)dst_addr_scl[0][R_IDX];
		    r = bf16tofloat(addr + j * 2);
		    addr = (u8 *)dst_addr_scl[0][G_IDX];
		    g = bf16tofloat(addr + j * 2);
		    addr = (u8 *)dst_addr_scl[0][B_IDX];
		    b = bf16tofloat(addr + j * 2);

                    dr = (s32)((r - test_results_f[i][j][0]) * EPSILON_SCALE);
                    dg = (s32)((g - test_results_f[i][j][1]) * EPSILON_SCALE);
                    db = (s32)((b - test_results_f[i][j][2]) * EPSILON_SCALE);
                    dr = dr > 0 ? dr : (-1 * dr);
                    dg = dg > 0 ? dg : (-1 * dg);
                    db = db > 0 ? db : (-1 * dg);

		    //if (r != test_results_f[i][j][0] || g != test_results_f[i][j][1] ||	b != test_results_f[i][j][2]) {
		    if (dr > 1 || dg > 1 || db > 1) {
                       uartlog("case-%d result-%d output(%f, %f, %f) expect(%f, %f, %f) mismatch \n",
			       i, j, r, g, b, test_results_f[i][j][0], test_results_f[i][j][1], test_results_f[i][j][2]);
			mismatch = true;
		    }
		}
		if (mismatch) break;
	    }
#endif
	}

    do {
        float val = 0.0;
        bm_get_num(" goon(0:n, 1:y): ", tmp);
        if (tmp != 1) break;

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
        //uartlog("factor:        %d %d %d\n", q_cfg.sc[0], q_cfg.sc[1], q_cfg.sc[2]);
        //uartlog("mean-sub:      %d %d %d\n", q_cfg.sub[0], q_cfg.sub[1], q_cfg.sub[2]);
        //uartlog("mean-sub-frac: %d %d %d\n", q_cfg.sub_frac[0], q_cfg.sub_frac[1], q_cfg.sub_frac[2]);

        bm_get_num("rounding(0:none, 1:to_even, 2:away_from_zero): ", tmp);
	cfg->csc_cfg.quant_round = tmp;

        sclr_ctrl_set_output(inst, &cfg->csc_cfg, cfg->fmt);

	sclr_trig_start(img_inst);

#ifndef ENV_EMU
        if (sclr_check_irq_handler(60) != 0) {
            uartlog("sclr end intr doesn't happens after 60 tick\n");
        } else {
            uartlog("sclr done\n");
            uartlog("sc(%d) odma status(%#x)\n", inst, sclr_odma_get_dbg_status(inst).raw);
        }
#endif
    } while(true);

    return 0;
}

int do_vip_fabric(int argc, char **argv)
{
    u8 mode = 0;
    u16 width, height;
    u64 src_addr, dst_addr;
    if (argc != 6) {
        uartlog("-------------------------------\n");
        uartlog("1st param: 0(offline) 1(real-time)\n");
        uartlog("2nd param: bytes per line(32-bytes alignment)\n");
        uartlog("3rd param: number of line\n");
        uartlog("           data-size = (bytes per line) * (number of line)\n");
        uartlog("4th param: src-addr(hex)\n");
        uartlog("           Ex: 106c00000\n");
        uartlog("5th param: dst-addr(hex)\n");
        uartlog("-------------------------------\n");

        return -EINVAL;
    }
    mode = strtol(argv[1], NULL, 10);
    width = strtol(argv[2], NULL, 10);
    height = strtol(argv[3], NULL, 10);
    src_addr = strtol(argv[4], NULL, 16);
    dst_addr = strtol(argv[5], NULL, 16);

    if (!sc_init) {
        sclr_ctrl_init();
        request_irq(SC_INTR_0, irq_handler_scl, 0, "scl int", NULL);
        uartlog("%s: request_irq(%d)\n", __func__, SC_INTR_0);
    }

    _reg_write(REG_SCL_TOP_AXI, mode ? 0x3F : 0x3E );

    u8 inst = 0;
    sclr_set_cfg(inst, /*sc_bypass*/ true, /*gop_bypass*/ true,
		 /*cir_bypass*/ true, /*odma_bypass*/ false);

    enum sclr_img_in img_inst = (inst == 0) ? SCL_IMG_D : SCL_IMG_V;
    union sclr_input in;
    in.v = SCL_INPUT_D_MEM;
    sclr_ctrl_set_input(img_inst, in, SCL_FMT_RGB_PACKED, SCL_CSC_NONE);
    struct sclr_mem mem;

    // img_in setup
    mem.width = width;
    mem.height = height;
    mem.start_x = 0;
    mem.start_y = 0;
    mem.addr0 = src_addr;
    mem.addr1 = 0;
    mem.addr2 = 0;
    mem.pitch_y = VIP_ALIGN(mem.width);
    mem.pitch_c = mem.pitch_y;
    sclr_img_set_mem(img_inst, &mem);

    // odma setup
    struct sclr_odma_cfg *cfg = sclr_odma_get_cfg(inst);
    cfg->flip = SCL_FLIP_NO;
    cfg->csc_cfg.mode = SCL_OUT_QUANT;
    cfg->fmt = SCL_FMT_RGB_PACKED;
    sclr_odma_set_cfg(inst, cfg);

    mem.addr0 = dst_addr;
    sclr_odma_set_mem(inst, &mem);

    sclr_top_reg_done();
    _intr_setup(inst, img_inst);
    sclr_trig_start(img_inst);
#ifndef ENV_EMU
    if (sclr_check_irq_handler(60) != 0) {
        uartlog("sclr end intr doesn't happens after 60 tick\n");
    } else {
        uartlog("sclr done\n");
    }
#endif

    return 0;
}

#ifdef VIP_PERFORMANCE_TEST
void enable_disp2k_for_test(void)
{
    // enable panel-phy
    _reg_write(0x0300c064, 0x1f1f);
    _reg_write(0x0300c0b4, 0x1);
    _reg_write(0x0a080050, 0x974);

    sclr_ctrl_set_disp_src(false);
    sclr_disp_set_timing(&panels[SCL_PANEL_1080P]);
    sclr_disp_tgen_enable(true);
}
#endif

void sc_from_isp(u8 inst, u8 online, struct sclr_size src, struct sclr_size dst, u64 addr0, u64 addr1, u64 addr2, bool osd_on)
{
    if (!sc_init) {
        sclr_ctrl_init();
        request_irq(SC_INTR_0, irq_handler_scl, 0, "scl int", NULL);
        uartlog("%s: request_irq(%d)\n", __func__, SC_INTR_0);
    }

    //if (inst > 1) return;

    struct sclr_scale_cfg sc_cfg;
    sc_cfg.src = src;
    sc_cfg.crop.x = sc_cfg.crop.y = 0;
    sc_cfg.crop.w = src.w;
    sc_cfg.crop.h = src.h;
    sc_cfg.dst = dst;
    sclr_ctrl_set_scale(inst, &sc_cfg);

    enum sclr_img_in img_inst;
    u8 num_args = 0;
    if (inst == 0) {
        img_inst = SCL_IMG_D;
    } else {
        img_inst = SCL_IMG_V;
    }

#ifdef VIP_PERFORMANCE_TEST
    // enable OSD 1/8 of image-size
    if (osd_on) {
	struct sclr_gop_cfg* gop_cfg = sclr_gop_get_cfg(inst);
	gop_cfg->b.burst = 7;
	gop_cfg->b.ow0_en = 1;
	sclr_gop_set_cfg(inst, gop_cfg);

	struct sclr_gop_ow_cfg cfg;
	u16 mem_width;
	struct sclr_size osd_size;

	cfg.fmt = SCL_GOP_FMT_ARGB8888;
	osd_size.w = dst.w >> 1;
	osd_size.h = dst.h >> 2;
	cfg.img_size = osd_size;
	cfg.mem_size.h = osd_size.h;

	switch(cfg.fmt) {
	    case SCL_GOP_FMT_ARGB8888:
		mem_width = osd_size.w * 4;
		break;
	    case SCL_GOP_FMT_ARGB4444:
	    case SCL_GOP_FMT_ARGB1555:
		mem_width = osd_size.w * 2;
		break;
	    case SCL_GOP_FMT_256LUT:
		mem_width = osd_size.w;
		break;
	    case SCL_GOP_FMT_16LUT:
		mem_width = osd_size.w >> 1;
		break;
	    case SCL_GOP_FMT_FONT:
		mem_width = (osd_size.w + 7)>>3;
		break;
	    default:
		uartlog("invalid fmt(%d)\n", cfg.fmt);
	}
	cfg.pitch = VIP_ALIGN(mem_width);
	cfg.mem_size.w = cfg.pitch;
	uartlog("mem size: (%d*%d), pitch(%d)\n", cfg.mem_size.w, cfg.mem_size.h, cfg.pitch);

	cfg.start.x = 0;
	cfg.start.y = 0;

	cfg.end.x = cfg.start.x + (osd_size.w << gop_cfg->b.hscl_en) - gop_cfg->b.hscl_en;
	cfg.end.y = cfg.start.y + (osd_size.h << gop_cfg->b.vscl_en) - gop_cfg->b.vscl_en;
	uartlog("overlay frame end pos: (%d*%d)\n", cfg.end.x, cfg.end.y);

	// count down from space's end
	cfg.addr = addr2 + 0x4000000;
	uartlog("osd mem addr: (0x%09lx)\n", cfg.addr);

	sclr_gop_ow_set_cfg(inst, 0, &cfg);
    }
#endif

    union sclr_input in;
    if (online) {
	in.v = SCL_INPUT_V_ISP;
	sclr_ctrl_set_input(img_inst, in, SCL_FMT_RGB_PLANAR, SCL_CSC_601_LIMIT_YUV2RGB);
    } else {
	in.v = SCL_INPUT_V_MEM;
	sclr_ctrl_set_input(img_inst, in, SCL_FMT_YUV420, SCL_CSC_601_LIMIT_YUV2RGB);
    }

    num_args = (online) ? 2 : 5;
    _handle_cfg_op(CFG_OP_IMG_SET_MEM, img_inst, num_args, sc_cfg.src.w, sc_cfg.src.h, addr0, addr1, addr2);

    // setup odma
    //_handle_cfg_op(CFG_OP_ODMA_CFG, inst);
    {
        struct sclr_odma_cfg *cfg;

        cfg = sclr_odma_get_cfg(inst);

        cfg->flip = SCL_FLIP_NO;
        cfg->csc_cfg.mode = SCL_OUT_DISABLE;
#ifdef VIP_PERFORMANCE_TEST
        cfg->fmt = (inst == 0) ? SCL_FMT_RGB_PLANAR : SCL_FMT_YUV420;
#else
        cfg->fmt = SCL_FMT_RGB_PLANAR;
#endif

        sclr_odma_set_cfg(inst, cfg);
        sclr_ctrl_set_output(inst, &cfg->csc_cfg, cfg->fmt);
    }
#ifdef VIP_PERFORMANCE_TEST
static u64 dst_addr[4][3] = {
    {BASE_OFFSET + 0x50000000, BASE_OFFSET + 0x50800000, BASE_OFFSET + 0x51000000},
    {BASE_OFFSET + 0x51800000, BASE_OFFSET + 0x52000000, BASE_OFFSET + 0x52800000},
    {BASE_OFFSET + 0x53000000, BASE_OFFSET + 0x53800000, BASE_OFFSET + 0x54000000},
    {BASE_OFFSET + 0x54800000, BASE_OFFSET + 0x55000000, BASE_OFFSET + 0x55800000},
};
    num_args = 5;
    _handle_cfg_op(CFG_OP_ODMA_SET_MEM, inst, num_args, sc_cfg.dst.w, sc_cfg.dst.h,
	    dst_addr[inst][0], dst_addr[inst][1], dst_addr[inst][2]);
    uartlog("sc%d dst buf(0x%lx, 0x%lx, 0x%lx\n", inst, dst_addr[inst][0], dst_addr[inst][1], dst_addr[inst][2]);

    sclr_top_reg_done();
    union sclr_intr mask;
    mask.raw = 0;
    mask.b.img_in_v_frame_end = true;
    mask.b.img_in_d_frame_end = true;
    mask.b.scl0_frame_end = true;
    mask.b.scl1_frame_end = true;
    mask.b.scl2_frame_end = true;
    mask.b.scl3_frame_end = true;
    sclr_set_intr_mask(mask);
#else
    num_args = 2;
    _handle_cfg_op(CFG_OP_ODMA_SET_MEM, inst, num_args, sc_cfg.dst.w, sc_cfg.dst.h);

    sclr_top_reg_done();
    _intr_setup(inst, img_inst);
#endif
    if (online)
	sclr_trig_start(img_inst);
}

int do_isp(int argc, char **argv)
{
    int tmp;
    u8 inst = 0;
    struct sclr_size src, dst;
    u64 addr0, addr1, addr2;
    u8 online = 1;

    bm_get_num("using which sc inst(0~3): ", inst);
    bm_get_num("src width(32 ~ 2688): ", src.w);
    bm_get_num("src height(1 ~ 1944): ", src.h);
    bm_get_num("dst width(32 ~ 1920): ", dst.w);
    bm_get_num("dst height(1 ~ 1080): ", dst.h);
    bm_get_num("isp2sc online? y(1)/n(0): ", online);
    if (!online) {
	uartlog("isp buf0 addr: 0x");
	scanf("%lu", &addr0);
	uartlog("isp buf1 addr: 0x");
	scanf("%lu", &addr1);
	uartlog("isp buf2 addr: 0x");
	scanf("%lu", &addr2);
    }

    sc_from_isp(inst, online, src, dst, addr0, addr1, addr2, false);


    return 0;
}

extern u64 src_addr[3][3];
extern u64 chk_addr[3][3];

int do_crops(int argc, char **argv)
{
#define CROP_W 1280
#define CROP_H 720
	int tmp;
	int inst = 0;
	struct sclr_size src = { .w = CROP_W, .h = CROP_H };
	struct sclr_rect crop_rect = { .x = 0, .y = 0, .w = CROP_W, .h = CROP_H };
	struct sclr_size out;

	uartlog("This test works with Panel to check.\n");
	uartlog("img_d should load a 1920x1080 image.\n");
	bm_get_num("Please make sure sc_d and display is ready.\n", tmp);

	// setup sc
	sclr_set_input_size(0, src);
	sclr_set_crop(inst, crop_rect);
	sclr_set_scale(inst);

	struct sclr_img_cfg *img_cfg = sclr_img_get_cfg(SCL_IMG_D);
	struct sclr_odma_cfg *odma_cfg = sclr_odma_get_cfg(0);
	struct sclr_disp_cfg *disp_cfg = sclr_disp_get_cfg();

	img_cfg->mem.width = CROP_W;
	img_cfg->mem.height = CROP_H;
	img_cfg->mem.start_y = 100;
	// test img_in crop read 32-steps
	for (int i = 0; i < 32; ++i) {
		img_cfg->mem.start_x = i + 100;
		sclr_img_set_mem(SCL_IMG_D, &img_cfg->mem);
		sclr_trig_start(SCL_IMG_D);
		uartlog("IMG Crop Step %d,", i);
		bm_get_num("Press ENTER to continue.\n", tmp);
	}

	out.w = odma_cfg->mem.width - 100;
	out.h = odma_cfg->mem.height;
	sclr_set_output_size(inst, out);
	sclr_set_scale(inst);
	odma_cfg->mem.width -= 100;
	disp_cfg->mem.width -= 100;
	sclr_disp_set_mem(&disp_cfg->mem);
	for (int i = 0; i < 32; ++i) {
		odma_cfg->mem.start_x = i;
		sclr_odma_set_mem(0, &odma_cfg->mem);
		sclr_trig_start(SCL_IMG_D);
		uartlog("ODMA Crop Step %d,", i);
		bm_get_num("Press ENTER to continue.\n", tmp);
	}
	return 0;
}

static struct cmd_entry test_cmd_list[] __attribute__ ((unused)) = {
    {"\n[---SCLR------------------------------------------------]", do_show_addr, 0, ""},
    {"show_addr_s",         do_show_addr,        0, "[Show sclr's buffer address]"},
    {"cfg",                 do_cfg,              0, "[Modify settings]"},
    {"move_s",              do_move,             1, "[do sc's move with scaling/crop/csc] \
                                                 \n\t1st param: instance(0~3) "},
    {"cir",                 do_cir,              1, "[control circle of specificed sc] \
                                                 \n\t1st param: instance(0~3) "},
    {"pat",                 do_pat,              2, "[control disp's pattern] "},
    {"cmdQ_s",              do_cmdq,             1, "[cmdq testcases] 0~6"},
    {"perf",                do_perf,             0, "[performance testcases]"},
    {"rounding",            do_rounding,         0, "[rounding testcases]"},
    {"fabric",              do_vip_fabric,       5, "[vip fabric testcases]"},
    {"isp2sc",              do_isp,              0, "[isp2sc testcase]"},
    {"crops",               do_crops,            0, "[crop phase testcase]"},
    {"sclr_size_test",      sclr_size_test,      0, "[check img/scl size]"},
    {"sclr_max_size_test",  sclr_max_size_test,  1, "[check img/scl max_size]"},
    {"sclr_fmt_test_1",     sclr_fmt_test_1,     0, "[800x600 rgb packed -> 800x600 rgb packed]"},
    {"sclr_fmt_test_2",     sclr_fmt_test_2,     0, "[800x600 nv12 -> 800x600 rgb packed]"},
    {"sclr_fmt_test_3",     sclr_fmt_test_3,     0, "[800x600 rgb packed -> 832x640 rgb packed]"},
    {"sclr_fmt_test_4",     sclr_fmt_test_4,     0, "[800x600 rgb packed -> 832x640 nv21 Y, UV]"},
    {"sclr_fmt_test_5",     sclr_fmt_test_5,     0, "[800x600 rgb packed -> 860x600 nv21 packed]"},
    {"sclr_fmt_test_6",     sclr_fmt_test_6,     0, "[800x600 rgb packed -> 860x600 nv12 packed]"},
    {"sclr_fmt_test_7",     sclr_fmt_test_7,     0, "[800x600 rgb packed -> 860x600 yuv420]"},
    {"sclr_fmt_test_8",     sclr_fmt_test_8,     0, "[1920x1080 YUV420 -> 1920x1080 YUV420]"},
    {"sclr_fmt_test_9",     sclr_fmt_test_9,     0, "[1920x1080 YUV420 -> 1920x1080 rgb packed]"},
    {"sclr_fmt_test_10",    sclr_fmt_test_10,    0, "[320x256 nv21 packed -> 320x256 nv21 packed]"},
    {"sclr_disp_test",      sclr_disp_test,      0, "[DISP test 800x600 rgb packed]"},
    {"sclr_dual_img_test",  sclr_dual_img_test,  0, "[dual image in test]"},
    {"sclr_blend_test",     sclr_blend_test,     0, "[800x600 NV12 packed, blend alpha]"},
    {"sclr_blend_test_2",   sclr_blend_test_2,   0, "[800x600 NV12 packed, blend stitch]"},
    {"sclr_blend_test_3",   sclr_blend_test_3,   0, "[800x600 NV12 packed, blend alpha + privacy mask(rgb332)]"},
    {"sclr_blend_test_4",   sclr_blend_test_4,   0, "[800x600 NV12 packed, blend alpha + privacy mask(y)]"},
    {"sclr_resize_test",    sclr_resize_test,    0, "[1920x1080 nv12 packed -> 1920x1088 nv12 packed, 4 tap]"},
    {"sclr_resize_test_2",  sclr_resize_test_2,  0, "[1920x1080 nv12 packed -> 2880x2880 nv12 packed, 4 tap]"},
    {"sclr_resize_test_3",  sclr_resize_test_3,  0, "[1920x1080 nv12 packed -> 1984x1088 nv12 packed, 4 tap]"},
    {"sclr_resize_test_4",  sclr_resize_test_4,  0, "[1920x1080 nv12 packed -> 320x256 nv12 packed, 4 tap]"},
    {"sclr_resize_test_5",  sclr_resize_test_5,  0, "[1920x1080 nv12 packed -> 320x256 rgb packed for cmodel, 4 tap]"},
    {"sclr_resize_test_6",  sclr_resize_test_6,  0, "[1920x1080 nv12 packed -> 320x256 nv21 packed, 4 tap]"},
    {"sclr_resize_test_7",  sclr_resize_test_7,  0, "[64x512 nv12 packed -> 8x512 nv21 packed, 2 tap, fast area]"},
    {"sclr_resize_test_8",  sclr_resize_test_8,  0, "[1920x1080 nv12 packed -> 1920x1080 nv21 packed, 2 tap]"},
    {"sclr_resize_test_9",  sclr_resize_test_9,  0, "[1920x1080 nv12 packed -> 320x256 nv21 packed, 2 tap, area]"},
    {"sclr_resize_test_10", sclr_resize_test_10, 0, "[1920x1080 nv12 packed -> 320x270 nv21 packed, 2 tap, area fast]"},
    {"sclr_resize_test_11", sclr_resize_test_11, 0, "[320x256 nv21 packed -> 1920x1080 nv12 packed, 2 tap, bilinear]"},
    {"sclr_resize_test_12", sclr_resize_test_12, 0, "[800x600 rgb packed -> 320x256 nv21 packed]"},
    {"sclr_2tap_pram_test", sclr_2tap_param_test, 0, "[2tap parameter test]"},
    {"ldc_to_sclr_sb_test", ldc_to_sclr_sb_test, 0, "[Slice buffer: ldc to scaler, 320x256 nv21 -> 320x256 nv21]"},
    {"ldc_to_sclr_sb_test_2", ldc_to_sclr_sb_test_2, 0, "[Slice buffer: ldc rot90 to scaler, 320x256 nv21 -> 256x320 nv21]"},
    {"sclr_to_vc_sb_test" , sclr_to_vc_sb_test, 0,  "[Slice buffer: scaler to vc, 352x288 nv21 -> 352x288 nv21]"},
    {"ldc_to_sclr_test", ldc_to_sclr_test, 0, "[ldc rot90 to scaler, 320x256 nv21 -> 256x320 nv21]"},
    {NULL, NULL, 0, NULL}
};

int testcase_sclr(void)
{
    sclr_mark = 0;
    sclr_ctrl_init();

    union sclr_intr mask;
    mask.raw = 0xffffffff;
    sclr_intr_ctrl(mask);

#if defined(ENV_CVITEST) && !defined(VIP_PERFORMANCE_TEST)
    // TODO: for FPGA TTL
    sclr_disp_set_timing(&panels[SCL_PANEL_TTL]);
#endif

#ifndef VIP_PERFORMANCE_TEST
    request_irq(SC_INTR_0, irq_handler_scl, 0, "scl int", NULL);
    uartlog("%s: request_irq(%d)\n", __func__, SC_INTR_0);
#endif
    sc_init = true;

#ifdef ISP_DUMP_TOOL
    struct sclr_size src,dst;
    src.w = 1308;
    src.h = 736;
    dst.w = 800;
    dst.h = 600;
    sc_from_isp(0, true, src, dst, 0, 0, 0, false);

    struct sclr_rect crop;
    crop.x = 12;
    crop.y = 11;
    crop.w = 800;
    crop.h = 600;

    sclr_set_crop(0, crop);
    sclr_set_scale(0);

    _reg_write(0x0300c064, 0x1f1f);
    _reg_write(0x0300c0b4, 0x1);
    _reg_write(0x0a080050, 0x974);

    sclr_ctrl_set_disp_src(false);
    sclr_disp_tgen_enable(true);
#endif

    uartlog("testcase_sclr: add command...\n");


    for (int i = 0; i < ARRAY_SIZE(test_cmd_list) - 1; i++) {
        command_add(&test_cmd_list[i]);
    }

    uartlog("testcase_sclr: add command done\n");

    return 0;
}

#ifdef ENV_EMU
#else
//module_testcase("1", testcase_sclr);
#endif
