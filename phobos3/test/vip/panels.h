#ifndef _VIP_PANELS_H_
#define _VIP_PANELS_H_

enum dsc_op {
	DSC_COMMAND,
	DSC_DELAY,
};

enum ili9881c_op {
	ILI9881C_SWITCH_PAGE,
	ILI9881C_COMMAND,
	ILI9881C_DELAY,
};

enum st7789v2_op{
	ST7789V2_COMM,
	ST7789V2_DATA,
};

struct dsc_instr {
	u8	delay;
	u8	size;
	u8	data[0x40];
};

struct ili9881c_instr {
	enum ili9881c_op	op;

	union {
		struct {
			u8	cmd;
			u8	data;
		} cmd;
		u16	page;
	} arg;
};


/* Define I80's lane (0~3)
 *
 * CS: Chip Select
 * RS(DCX): Data/Command
 * WR: MCU Write to bus
 * RD: MCU Read from bus
 */
typedef struct _i80_lane_s {
	u8 CS;
	u8 RS;
	u8 WR;
	u8 RD;
} i80_lane_s;

/* Define I80's config
 *
 * lane_s: lane mapping
 * fmt: format of data
 * cycle_time: cycle time of WR/RD, unit ns, max 250
 */

typedef enum _VO_I80_FORMAT {
	VO_I80_FORMAT_RGB444 = 0,
	VO_I80_FORMAT_RGB565,
	VO_I80_FORMAT_RGB666,
	VO_I80_FORMAT_MAX
} VO_I80_FORMAT;

typedef struct _i80_cfg_s {
	i80_lane_s lane_s;
	VO_I80_FORMAT fmt;
	u16 cycle_time;
} i80_cfg_s;

typedef struct _VO_I80_INSTR_S {
	u8	delay;
	u8	data_type;
	u8	data;
} VO_I80_INSTR_S;

enum i80_ctrl_type {
	I80_CTRL_CMD = 0,
	I80_CTRL_DATA,
	I80_CTRL_EOL = I80_CTRL_DATA,
	I80_CTRL_EOF,
	I80_CTRL_END = I80_CTRL_EOF,
	I80_CTRL_MAX
};

enum i80_op_type {
	I80_OP_GO = 0,
	I80_OP_TIMER,
	I80_OP_DONE,
	I80_OP_MAX,
};

#define ILI9881C_SWITCH_PAGE_INSTR(_page)	\
	{					\
		.op = ILI9881C_SWITCH_PAGE,	\
		.arg = {			\
			.page = (_page),	\
		},				\
	}

#define ILI9881C_COMMAND_INSTR(_cmd, _data)	\
	{					\
		.op = ILI9881C_COMMAND,		\
		.arg = {			\
			.cmd = {		\
				.cmd = (_cmd),	\
				.data = (_data),\
			},			\
		},				\
	}

#define ILI9881C_DELAY_INSTR(_delay)		\
	{					\
		.op = ILI9881C_DELAY,		\
		.arg = {			\
			.page = (_delay),	\
		},				\
	}

static const struct ili9881c_instr ili9881c_init[] = {
    ILI9881C_SWITCH_PAGE_INSTR(3),
    ILI9881C_COMMAND_INSTR(0x1, 0x0),
    ILI9881C_COMMAND_INSTR(0x2, 0x0),
    ILI9881C_COMMAND_INSTR(0x3, 0x73),
    ILI9881C_COMMAND_INSTR(0x4, 0x0),
    ILI9881C_COMMAND_INSTR(0x5, 0x0),
    ILI9881C_COMMAND_INSTR(0x6, 0xa),
    ILI9881C_COMMAND_INSTR(0x7, 0x0),
    ILI9881C_COMMAND_INSTR(0x8, 0x0),
    ILI9881C_COMMAND_INSTR(0x9, 0x1),
    ILI9881C_COMMAND_INSTR(0xa, 0x0),
    ILI9881C_COMMAND_INSTR(0xb, 0x0),
    ILI9881C_COMMAND_INSTR(0xc, 0x1),
    ILI9881C_COMMAND_INSTR(0xd, 0x0),
    ILI9881C_COMMAND_INSTR(0xe, 0x0),
    ILI9881C_COMMAND_INSTR(0xf, 0x1d),
    ILI9881C_COMMAND_INSTR(0x10, 0x1d),
    ILI9881C_COMMAND_INSTR(0x11, 0x0),
    ILI9881C_COMMAND_INSTR(0x12, 0x0),
    ILI9881C_COMMAND_INSTR(0x13, 0x0),
    ILI9881C_COMMAND_INSTR(0x14, 0x0),
    ILI9881C_COMMAND_INSTR(0x15, 0x0),
    ILI9881C_COMMAND_INSTR(0x16, 0x0),
    ILI9881C_COMMAND_INSTR(0x17, 0x0),
    ILI9881C_COMMAND_INSTR(0x18, 0x0),
    ILI9881C_COMMAND_INSTR(0x19, 0x0),
    ILI9881C_COMMAND_INSTR(0x1a, 0x0),
    ILI9881C_COMMAND_INSTR(0x1b, 0x0),
    ILI9881C_COMMAND_INSTR(0x1c, 0x0),
    ILI9881C_COMMAND_INSTR(0x1d, 0x0),
    ILI9881C_COMMAND_INSTR(0x1e, 0x40),
    ILI9881C_COMMAND_INSTR(0x1f, 0x80),
    ILI9881C_COMMAND_INSTR(0x20, 0x6),
    ILI9881C_COMMAND_INSTR(0x21, 0x2),
    ILI9881C_COMMAND_INSTR(0x22, 0x0),
    ILI9881C_COMMAND_INSTR(0x23, 0x0),
    ILI9881C_COMMAND_INSTR(0x24, 0x0),
    ILI9881C_COMMAND_INSTR(0x25, 0x0),
    ILI9881C_COMMAND_INSTR(0x26, 0x0),
    ILI9881C_COMMAND_INSTR(0x27, 0x0),
    ILI9881C_COMMAND_INSTR(0x28, 0x33),
    ILI9881C_COMMAND_INSTR(0x29, 0x3),
    ILI9881C_COMMAND_INSTR(0x2a, 0x0),
    ILI9881C_COMMAND_INSTR(0x2b, 0x0),
    ILI9881C_COMMAND_INSTR(0x2c, 0x0),
    ILI9881C_COMMAND_INSTR(0x2d, 0x0),
    ILI9881C_COMMAND_INSTR(0x2e, 0x0),
    ILI9881C_COMMAND_INSTR(0x2f, 0x0),
    ILI9881C_COMMAND_INSTR(0x30, 0x0),
    ILI9881C_COMMAND_INSTR(0x31, 0x0),
    ILI9881C_COMMAND_INSTR(0x32, 0x0),
    ILI9881C_COMMAND_INSTR(0x33, 0x0),
    ILI9881C_COMMAND_INSTR(0x34, 0x4),
    ILI9881C_COMMAND_INSTR(0x35, 0x0),
    ILI9881C_COMMAND_INSTR(0x36, 0x0),
    ILI9881C_COMMAND_INSTR(0x37, 0x0),
    ILI9881C_COMMAND_INSTR(0x38, 0x3c),
    ILI9881C_COMMAND_INSTR(0x39, 0x0),
    ILI9881C_COMMAND_INSTR(0x3a, 0x40),
    ILI9881C_COMMAND_INSTR(0x3b, 0x40),
    ILI9881C_COMMAND_INSTR(0x3c, 0x0),
    ILI9881C_COMMAND_INSTR(0x3d, 0x0),
    ILI9881C_COMMAND_INSTR(0x3e, 0x0),
    ILI9881C_COMMAND_INSTR(0x3f, 0x0),
    ILI9881C_COMMAND_INSTR(0x40, 0x0),
    ILI9881C_COMMAND_INSTR(0x41, 0x0),
    ILI9881C_COMMAND_INSTR(0x42, 0x0),
    ILI9881C_COMMAND_INSTR(0x43, 0x0),
    ILI9881C_COMMAND_INSTR(0x44, 0x0),
    ILI9881C_COMMAND_INSTR(0x50, 0x1),
    ILI9881C_COMMAND_INSTR(0x51, 0x23),
    ILI9881C_COMMAND_INSTR(0x52, 0x45),
    ILI9881C_COMMAND_INSTR(0x53, 0x67),
    ILI9881C_COMMAND_INSTR(0x54, 0x89),
    ILI9881C_COMMAND_INSTR(0x55, 0xab),
    ILI9881C_COMMAND_INSTR(0x56, 0x1),
    ILI9881C_COMMAND_INSTR(0x57, 0x23),
    ILI9881C_COMMAND_INSTR(0x58, 0x45),
    ILI9881C_COMMAND_INSTR(0x59, 0x67),
    ILI9881C_COMMAND_INSTR(0x5a, 0x89),
    ILI9881C_COMMAND_INSTR(0x5b, 0xab),
    ILI9881C_COMMAND_INSTR(0x5c, 0xcd),
    ILI9881C_COMMAND_INSTR(0x5d, 0xef),
    ILI9881C_COMMAND_INSTR(0x5e, 0x11),
    ILI9881C_COMMAND_INSTR(0x5f, 0x1),
    ILI9881C_COMMAND_INSTR(0x60, 0x0),
    ILI9881C_COMMAND_INSTR(0x61, 0x15),
    ILI9881C_COMMAND_INSTR(0x62, 0x14),
    ILI9881C_COMMAND_INSTR(0x63, 0xe),
    ILI9881C_COMMAND_INSTR(0x64, 0xf),
    ILI9881C_COMMAND_INSTR(0x65, 0xc),
    ILI9881C_COMMAND_INSTR(0x66, 0xd),
    ILI9881C_COMMAND_INSTR(0x67, 0x6),
    ILI9881C_COMMAND_INSTR(0x68, 0x2),
    ILI9881C_COMMAND_INSTR(0x69, 0x7),
    ILI9881C_COMMAND_INSTR(0x6a, 0x2),
    ILI9881C_COMMAND_INSTR(0x6b, 0x2),
    ILI9881C_COMMAND_INSTR(0x6c, 0x2),
    ILI9881C_COMMAND_INSTR(0x6d, 0x2),
    ILI9881C_COMMAND_INSTR(0x6e, 0x2),
    ILI9881C_COMMAND_INSTR(0x6f, 0x2),
    ILI9881C_COMMAND_INSTR(0x70, 0x2),
    ILI9881C_COMMAND_INSTR(0x71, 0x2),
    ILI9881C_COMMAND_INSTR(0x72, 0x2),
    ILI9881C_COMMAND_INSTR(0x73, 0x2),
    ILI9881C_COMMAND_INSTR(0x74, 0x2),
    ILI9881C_COMMAND_INSTR(0x75, 0x1),
    ILI9881C_COMMAND_INSTR(0x76, 0x0),
    ILI9881C_COMMAND_INSTR(0x77, 0x14),
    ILI9881C_COMMAND_INSTR(0x78, 0x15),
    ILI9881C_COMMAND_INSTR(0x79, 0xe),
    ILI9881C_COMMAND_INSTR(0x7a, 0xf),
    ILI9881C_COMMAND_INSTR(0x7b, 0xc),
    ILI9881C_COMMAND_INSTR(0x7c, 0xd),
    ILI9881C_COMMAND_INSTR(0x7d, 0x6),
    ILI9881C_COMMAND_INSTR(0x7e, 0x2),
    ILI9881C_COMMAND_INSTR(0x7f, 0x7),
    ILI9881C_COMMAND_INSTR(0x80, 0x2),
    ILI9881C_COMMAND_INSTR(0x81, 0x2),
    ILI9881C_COMMAND_INSTR(0x82, 0x2),
    ILI9881C_COMMAND_INSTR(0x83, 0x2),
    ILI9881C_COMMAND_INSTR(0x84, 0x2),
    ILI9881C_COMMAND_INSTR(0x85, 0x2),
    ILI9881C_COMMAND_INSTR(0x86, 0x2),
    ILI9881C_COMMAND_INSTR(0x87, 0x2),
    ILI9881C_COMMAND_INSTR(0x88, 0x2),
    ILI9881C_COMMAND_INSTR(0x89, 0x2),
    ILI9881C_COMMAND_INSTR(0x8a, 0x2),
    ILI9881C_SWITCH_PAGE_INSTR(4),
    ILI9881C_COMMAND_INSTR(0x6c, 0x15),
    ILI9881C_COMMAND_INSTR(0x6e, 0x2b),
    ILI9881C_COMMAND_INSTR(0x6f, 0x33),
    ILI9881C_COMMAND_INSTR(0x8d, 0x18),
    ILI9881C_COMMAND_INSTR(0x87, 0xba),
    ILI9881C_COMMAND_INSTR(0x26, 0x76),
    ILI9881C_COMMAND_INSTR(0xb2, 0xd1),
    ILI9881C_COMMAND_INSTR(0xb5, 0x6),
    ILI9881C_COMMAND_INSTR(0x3a, 0x24),
    ILI9881C_COMMAND_INSTR(0x35, 0x1f),
    ILI9881C_SWITCH_PAGE_INSTR(1),
    ILI9881C_COMMAND_INSTR(0x22, 0x9),
    ILI9881C_COMMAND_INSTR(0x31, 0x0),
    ILI9881C_COMMAND_INSTR(0x40, 0x33),
    ILI9881C_COMMAND_INSTR(0x53, 0xa2),
    ILI9881C_COMMAND_INSTR(0x55, 0x92),
    ILI9881C_COMMAND_INSTR(0x50, 0x96),
    ILI9881C_COMMAND_INSTR(0x51, 0x96),
    ILI9881C_COMMAND_INSTR(0x60, 0x22),
    ILI9881C_COMMAND_INSTR(0x61, 0x0),
    ILI9881C_COMMAND_INSTR(0x62, 0x19),
    ILI9881C_COMMAND_INSTR(0x63, 0x0),
    ILI9881C_COMMAND_INSTR(0xa0, 0x8),
    ILI9881C_COMMAND_INSTR(0xa1, 0x11),
    ILI9881C_COMMAND_INSTR(0xa2, 0x19),
    ILI9881C_COMMAND_INSTR(0xa3, 0xd),
    ILI9881C_COMMAND_INSTR(0xa4, 0xd),
    ILI9881C_COMMAND_INSTR(0xa5, 0x1e),
    ILI9881C_COMMAND_INSTR(0xa6, 0x14),
    ILI9881C_COMMAND_INSTR(0xa7, 0x17),
    ILI9881C_COMMAND_INSTR(0xa8, 0x4f),
    ILI9881C_COMMAND_INSTR(0xa9, 0x1a),
    ILI9881C_COMMAND_INSTR(0xaa, 0x27),
    ILI9881C_COMMAND_INSTR(0xab, 0x49),
    ILI9881C_COMMAND_INSTR(0xac, 0x1a),
    ILI9881C_COMMAND_INSTR(0xad, 0x18),
    ILI9881C_COMMAND_INSTR(0xae, 0x4c),
    ILI9881C_COMMAND_INSTR(0xaf, 0x22),
    ILI9881C_COMMAND_INSTR(0xb0, 0x27),
    ILI9881C_COMMAND_INSTR(0xb1, 0x4b),
    ILI9881C_COMMAND_INSTR(0xb2, 0x60),
    ILI9881C_COMMAND_INSTR(0xb3, 0x39),
    ILI9881C_COMMAND_INSTR(0xc0, 0x8),
    ILI9881C_COMMAND_INSTR(0xc1, 0x11),
    ILI9881C_COMMAND_INSTR(0xc2, 0x19),
    ILI9881C_COMMAND_INSTR(0xc3, 0xd),
    ILI9881C_COMMAND_INSTR(0xc4, 0xd),
    ILI9881C_COMMAND_INSTR(0xc5, 0x1e),
    ILI9881C_COMMAND_INSTR(0xc6, 0x14),
    ILI9881C_COMMAND_INSTR(0xc7, 0x17),
    ILI9881C_COMMAND_INSTR(0xc8, 0x4f),
    ILI9881C_COMMAND_INSTR(0xc9, 0x1a),
    ILI9881C_COMMAND_INSTR(0xca, 0x27),
    ILI9881C_COMMAND_INSTR(0xcb, 0x49),
    ILI9881C_COMMAND_INSTR(0xcc, 0x1a),
    ILI9881C_COMMAND_INSTR(0xcd, 0x18),
    ILI9881C_COMMAND_INSTR(0xce, 0x4c),
    ILI9881C_COMMAND_INSTR(0xcf, 0x33),
    ILI9881C_COMMAND_INSTR(0xd0, 0x27),
    ILI9881C_COMMAND_INSTR(0xd1, 0x4b),
    ILI9881C_COMMAND_INSTR(0xd2, 0x60),
    ILI9881C_COMMAND_INSTR(0xd3, 0x39),
    ILI9881C_SWITCH_PAGE_INSTR(0),
    ILI9881C_COMMAND_INSTR(0x36, 0x0),
    ILI9881C_COMMAND_INSTR(0x35, 0x0),  // tear on
    ILI9881C_COMMAND_INSTR(0x11, 0x0),  // sleep out
    ILI9881C_DELAY_INSTR(120),
    ILI9881C_COMMAND_INSTR(0x29, 0x0),  // display on
};

static const struct dsc_instr hx8394_init[] = {
	#if 1
	{ .delay = 0, .size = 4, .data = {0xB9, 0xFF, 0x83, 0x94} },
	{ .delay = 0, .size = 15, .data = {0xB1, 0x50, 0x15, 0x75, 0x09, 0x32, 0x44, 0x71,
					   0x31, 0x4D, 0x2F, 0x56, 0x73, 0x02, 0x02} },
	{ .delay = 0, .size = 7, .data = {0xBA, 0x63, 0x03, 0x68, 0x6B, 0xB2, 0xC0} },
	{ .delay = 0, .size = 2, .data = {0xD2, 0x88} },
	{ .delay = 0, .size = 6, .data = {0xB2, 0x00, 0x80, 0x64, 0x10, 0x07} },
	{ .delay = 0, .size = 22, .data = {0xB4, 0x01, 0x75, 0x01, 0x75, 0x01, 0x75, 0x01, 0x0C, 0x86
					, 0x75, 0x00, 0x3F, 0x01, 0x75, 0x01, 0x75, 0x01, 0x75, 0x01
					, 0x0C, 0x86} },
	{ .delay = 0, .size = 34, .data = {0xD3, 0x00, 0x00, 0x06, 0x06, 0x40, 0x1A, 0x08, 0x00, 0x32
					, 0x10, 0x08, 0x00, 0x08, 0x54, 0x15, 0x10, 0x05, 0x04, 0x02
					, 0x12, 0x10, 0x05, 0x07, 0x23, 0x23, 0x0C, 0x0C, 0x27, 0x10
					, 0x07, 0x07, 0x10, 0x40, } },
	{ .delay = 0, .size = 45, .data = {0xD5, 0x04, 0x05, 0x06, 0x07, 0x00, 0x01, 0x02, 0x03, 0x20
					, 0x21, 0x22, 0x23, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18
					, 0x18, 0x19, 0x19, 0x18, 0x18, 0x18, 0x18, 0x1B, 0x1B, 0x1A
					, 0x1A, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18
					, 0x18, 0x18, 0x18, 0x18, 0x18,  } },
	{ .delay = 0, .size = 45, .data = {0xD6, 0x03, 0x02, 0x01, 0x00, 0x07, 0x06, 0x05, 0x04, 0x23
					, 0x22, 0x21, 0x20, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x58
					, 0x58, 0x18, 0x18, 0x19, 0x19, 0x18, 0x18, 0x1B, 0x1B, 0x1A
					, 0x1A, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18
					, 0x18, 0x18, 0x18, 0x18, 0x18,   } },
	{ .delay = 0, .size = 59, .data = {0xE0, 0x00, 0x1A, 0x24, 0x2C, 0x2E, 0x32, 0x34, 0x32, 0x66
					, 0x73, 0x82, 0x7F, 0x85, 0x95, 0x97, 0x99, 0xA4, 0xA5, 0xA0
					, 0xAB, 0xBA, 0x5A, 0x59, 0x5D, 0x61, 0x63, 0x6C, 0x72, 0x7F
					, 0x00, 0x19, 0x24, 0x2C, 0x2E, 0x32, 0x34, 0x32, 0x66, 0x73
					, 0x82, 0x7F, 0x85, 0x95, 0x97, 0x99, 0xA4, 0xA5, 0xA0, 0xAB
					, 0xBA, 0x5A, 0x59, 0x5D, 0x61, 0x63, 0x6C, 0x72, 0x7F,    } },
	{ .delay = 0, .size = 2, .data = {0xCC, 0x03} },
	{ .delay = 0, .size = 3, .data = {0xC0, 0x1F, 0x73} },
	{ .delay = 0, .size = 3, .data = {0xB6, 0x42, 0x42} },
	{ .delay = 0, .size = 2, .data = {0xD4, 0x02} },
	{ .delay = 0, .size = 2, .data = {0xBD, 0x01} },
	{ .delay = 0, .size = 2, .data = {0xB1, 0x00} },
	{ .delay = 0, .size = 2, .data = {0xBD, 0x00} },
	{ .delay = 0, .size = 8, .data = {0xBF, 0x40, 0x81, 0x50, 0x00, 0x1A, 0xFC, 0x01} },
	{ .delay = 0, .size = 2, .data = {0xC6, 0xEF} },
	#endif
	{ .delay = 120, .size = 1, .data = {0x11} },
	{ .delay = 20, .size = 1, .data = {0x29} },
};
const VO_I80_INSTR_S i80_st7789v2_init_mcu_cmds[] = {
	{.delay = 120,  .data_type = ST7789V2_COMM,	 .data = 0x11},
	{.delay = 0,   .data_type = ST7789V2_COMM,	 .data = 0x36},
	{.delay = 0,   .data_type = ST7789V2_DATA,	 .data = 0x00},
	{.delay = 0,   .data_type = ST7789V2_COMM,	 .data = 0x3A},
	{.delay = 0,   .data_type = ST7789V2_DATA,	 .data = 0x05}, //0x101 = 16 bit/pixel, 0x110 = 18bit/pixel
	{.delay = 0,   .data_type = ST7789V2_COMM,	 .data = 0xB1},
	{.delay = 0,   .data_type = ST7789V2_DATA,	 .data = 0x00},
	{.delay = 0,   .data_type = ST7789V2_COMM,	 .data = 0xB2},
	{.delay = 0,   .data_type = ST7789V2_DATA,	 .data = 0x0C},
	{.delay = 0,   .data_type = ST7789V2_DATA,	 .data = 0x0C},
	{.delay = 0,   .data_type = ST7789V2_DATA,	 .data = 0x00},
	{.delay = 0,   .data_type = ST7789V2_DATA,	 .data = 0x33},
	{.delay = 0,   .data_type = ST7789V2_DATA,	 .data = 0x33},
	{.delay = 0,   .data_type = ST7789V2_COMM,	 .data = 0xB7},
	{.delay = 0,   .data_type = ST7789V2_DATA,	 .data = 0x75},
	{.delay = 0,   .data_type = ST7789V2_COMM,	 .data = 0xBB},
	{.delay = 0,   .data_type = ST7789V2_DATA,	 .data = 0x1f},//Vcom 0x1F: 0.875, 0x31: 1.35V
	{.delay = 0,   .data_type = ST7789V2_COMM,	 .data = 0xC0},
	{.delay = 0,   .data_type = ST7789V2_DATA,	 .data = 0x2C},
	{.delay = 0,   .data_type = ST7789V2_COMM,	 .data = 0xC2},
	{.delay = 0,   .data_type = ST7789V2_DATA,	 .data = 0x01},
	{.delay = 0,   .data_type = ST7789V2_COMM,	 .data = 0xC3},
	{.delay = 0,   .data_type = ST7789V2_DATA,	 .data = 0x13},
	{.delay = 0,   .data_type = ST7789V2_COMM,	 .data = 0xC4},
	{.delay = 0,   .data_type = ST7789V2_DATA,	 .data = 0x20},//VDV, 0x20:0v
	{.delay = 0,   .data_type = ST7789V2_COMM,	 .data = 0xC6},
	{.delay = 0,   .data_type = ST7789V2_DATA,	 .data = 0x0F},//Frame Rate control 0F: 60hz
	{.delay = 0,   .data_type = ST7789V2_COMM,	 .data = 0xD0},
	{.delay = 0,   .data_type = ST7789V2_DATA,	 .data = 0xA4},
	{.delay = 0,   .data_type = ST7789V2_DATA,	 .data = 0xA1},
	{.delay = 0,   .data_type = ST7789V2_COMM,	 .data = 0xD6},
	{.delay = 0,   .data_type = ST7789V2_DATA,	 .data = 0xA1},
	{.delay = 0,   .data_type = ST7789V2_COMM,	 .data = 0xE0},
	{.delay = 0,   .data_type = ST7789V2_DATA,	 .data = 0xD0},
	{.delay = 0,   .data_type = ST7789V2_DATA,	 .data = 0x0D},
	{.delay = 0,   .data_type = ST7789V2_DATA,	 .data = 0x16},
	{.delay = 0,   .data_type = ST7789V2_DATA,	 .data = 0x14},
	{.delay = 0,   .data_type = ST7789V2_DATA,	 .data = 0x14},
	{.delay = 0,   .data_type = ST7789V2_DATA,	 .data = 0x2F},
	{.delay = 0,   .data_type = ST7789V2_DATA,	 .data = 0x38},
	{.delay = 0,   .data_type = ST7789V2_DATA,	 .data = 0x54},
	{.delay = 0,   .data_type = ST7789V2_DATA,	 .data = 0x48},
	{.delay = 0,   .data_type = ST7789V2_DATA,	 .data = 0x0A},
	{.delay = 0,   .data_type = ST7789V2_DATA,	 .data = 0x1C},
	{.delay = 0,   .data_type = ST7789V2_DATA,	 .data = 0x19},
	{.delay = 0,   .data_type = ST7789V2_DATA,	 .data = 0x1A},
	{.delay = 0,   .data_type = ST7789V2_DATA,	 .data = 0x1D},
	{.delay = 0,   .data_type = ST7789V2_COMM,	 .data = 0xE1},
	{.delay = 0,   .data_type = ST7789V2_DATA,	 .data = 0xD0},
	{.delay = 0,   .data_type = ST7789V2_DATA,	 .data = 0x04},
	{.delay = 0,   .data_type = ST7789V2_DATA,	 .data = 0x0C},
	{.delay = 0,   .data_type = ST7789V2_DATA,	 .data = 0x08},
	{.delay = 0,   .data_type = ST7789V2_DATA,	 .data = 0x09},
	{.delay = 0,   .data_type = ST7789V2_DATA,	 .data = 0x23},
	{.delay = 0,   .data_type = ST7789V2_DATA,	 .data = 0x39},
	{.delay = 0,   .data_type = ST7789V2_DATA,	 .data = 0x54},
	{.delay = 0,   .data_type = ST7789V2_DATA,	 .data = 0x48},
	{.delay = 0,   .data_type = ST7789V2_DATA,	 .data = 0x30},
	{.delay = 0,   .data_type = ST7789V2_DATA,	 .data = 0x14},
	{.delay = 0,   .data_type = ST7789V2_DATA,	 .data = 0x14},
	{.delay = 0,   .data_type = ST7789V2_DATA,	 .data = 0x19},
	{.delay = 0,   .data_type = ST7789V2_DATA,	 .data = 0x1C},
	{.delay = 0,   .data_type = ST7789V2_COMM,	 .data = 0x2A},
	{.delay = 0,   .data_type = ST7789V2_DATA,	 .data = 0x00},
	{.delay = 0,   .data_type = ST7789V2_DATA,	 .data = 0x00},
	{.delay = 0,   .data_type = ST7789V2_DATA,	 .data = 0x00},
	{.delay = 0,   .data_type = ST7789V2_DATA,	 .data = 0xEF},  //240

	{.delay = 0,   .data_type = ST7789V2_COMM,	 .data = 0x2B},
	{.delay = 0,   .data_type = ST7789V2_DATA,	 .data = 0x00},
	{.delay = 0,   .data_type = ST7789V2_DATA,	 .data = 0x00},
	{.delay = 0,   .data_type = ST7789V2_DATA,	 .data = 0x01},
	{.delay = 0,   .data_type = ST7789V2_DATA,	 .data = 0x39},  //320

	{.delay = 0,   .data_type = ST7789V2_COMM,	 .data = 0x29},
	{.delay = 0,   .data_type = ST7789V2_COMM,	 .data = 0x2C},
};
#endif // _VIP_PANELS_H_
