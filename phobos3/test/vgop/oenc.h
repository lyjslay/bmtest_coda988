#ifndef _CVI_OENC_H_
#define _CVI_OENC_H_

#define MAX_OSD_ENC_INST 1

enum oenc_gop_format {
	OENC_GOP_FMT_ARGB8888,
	OENC_GOP_FMT_ARGB4444,
	OENC_GOP_FMT_ARGB1555,
	OENC_GOP_FMT_256LUT,
	OENC_GOP_FMT_16LUT,
	OENC_GOP_FMT_FONT,
	OENC_GOP_FMT_MAX
};

struct oenc_size {
	u16 w;
	u16 h;
};

struct oenc_cfg {
	enum oenc_gop_format fmt: 4;
	union {
		struct {
			u32 fmt         : 4;
			u32 resv4       : 4;
			u32 alpha_zero  : 1;
			u32 resv3       : 3;
			u32 rgb_trunc   : 2;
			u32 alpha_trunc : 2;
			u32 limit_bsz_en: 1;
			u32 limit_bsz_bypass: 1;
			u32 wprot_en    : 1;
			u32 resv11      : 11;
			u32 wdog_en     : 1;
			u32 intr_en     : 1;
		} b;
		u32 raw;
	} cfg;
	struct oenc_size src_picture_size;
	struct oenc_size src_mem_size;
	u16 src_pitch;
	u64 src_adr;
	u32 wprot_laddr;
	u32 wprot_uaddr;
	u64 bso_adr;
	u64 bso_adr1;
	u64 bso_adr2;
	u64 bso_adr3;
	u64 bso_adr4;
	u64 bso_adr5;
	u64 bso_adr6;
	u64 bso_adr7;
	u32 limit_bsz;
	u32 bso_sz; //OSD encoder original data of bso_sz
	struct oenc_size bso_mem_size; //for setting VGOP bitstream size
};

struct oenc_int {
	union {
		struct {
			u32 go      : 1;
			u32 resv7   : 7;
			u32 done    : 1;
			u32 resv6   : 6;
			u32 intr_clr: 1;
			u32 intr_vec: 16;
		} b;
		u32 raw;
	} go_intr;
};

extern void oenc_intr_clr(u8 inst, u32 flag);
extern int oenc_intr_status(u8 inst);

void oenc_set_cfg(struct oenc_cfg *oenc_cfg, u8 oenc_inst);
struct oenc_cfg * oenc_get_cfg(u8 oenc_inst);
// void oenc_intr_en(u8 oenc_inst);
#endif  //_CVI_OENC_H_