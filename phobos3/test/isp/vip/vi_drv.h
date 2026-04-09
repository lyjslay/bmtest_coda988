#ifndef __VI_DRV_H__
#define __VI_DRV_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include <inttypes.h>
#include <vi_common.h>
#include <inc/isp_reg.h>
#include "inc/vi_tun_cfg.h"
#include "reg.h"
#include "reg.h"
#include "system_common.h"
#ifndef _OFST
#define _OFST(_BLK_T, _REG)       ((uintptr_t)&(((struct _BLK_T *)0)->_REG))
#endif

#define ISP_RD_REG_BA(_BA) \
	(_reg_read(_BA))

#define ISP_RD_REG(_BA, _BLK_T, _REG) \
	(_reg_read(_BA+_OFST(_BLK_T, _REG)))

#define ISP_RD_BITS(_BA, _BLK_T, _REG, _FLD) \
	({\
		typeof(((struct _BLK_T *)0)->_REG) _r;\
		_r.raw = _reg_read(_BA+_OFST(_BLK_T, _REG));\
		_r.bits._FLD;\
	})

#define ISP_WR_REG(_BA, _BLK_T, _REG, _V) \
	(_reg_write((_BA+_OFST(_BLK_T, _REG)), _V))

#define ISP_WR_REG_OFT(_BA, _BLK_T, _REG, _OFT, _V) \
	(_reg_write((_BA+_OFST(_BLK_T, _REG) + _OFT), _V))

#define ISP_WR_BITS(_BA, _BLK_T, _REG, _FLD, _V) \
	do {\
		typeof(((struct _BLK_T *)0)->_REG) _r;\
		_r.raw = _reg_read(_BA+_OFST(_BLK_T, _REG));\
		_r.bits._FLD = _V;\
		_reg_write((_BA+_OFST(_BLK_T, _REG)), _r.raw);\
	} while (0)

#define ISP_WO_BITS(_BA, _BLK_T, _REG, _FLD, _V) \
	do {\
		typeof(((struct _BLK_T *)0)->_REG) _r;\
		_r.raw = 0;\
		_r.bits._FLD = _V;\
		_reg_write((_BA+_OFST(_BLK_T, _REG)), _r.raw);\
	} while (0)

#define ISP_WR_REGS_BURST(_BA, _BLK_T, _REG, _SIZE, _STR)\
	do {\
		u32 k = 0;\
		uintptr_t ofst = _OFST(_BLK_T, _REG);\
		for (; k < sizeof(_SIZE) / 0x4; k++) {\
			u32 val = (&_STR + k)->raw;\
			_reg_write((_BA + ofst + (k * 0x4)), val);\
		} \
	} while (0)

#define ISP_WR_REG_LOOP_SHFT(_BA, _BLK_T, _REG, _TOTAL_SIZE, _SFT_SIZE, _LUT, _SFT_BIT) \
	do {\
		uint16_t i = 0, j = 0;\
		uint32_t val = 0;\
		for (; i < _TOTAL_SIZE / _SFT_SIZE; i++) {\
			val = 0;\
			for (j = 0; j < _SFT_SIZE; j++) {\
				val += (_LUT[(i * _SFT_SIZE) + j] << (_SFT_BIT * j));\
			} \
			_reg_write((_BA + _OFST(_BLK_T, _REG) + (i * 0x4)), val);\
		} \
	} while (0)

#define REG_ARRAY_UPDATE2_SIZE(addr, array, size)		\
	do {							\
		uint16_t i;					\
		for (i = 0; i < size; i += 2) {			\
			val = array[i];				\
			if ((i + 1) < size)			\
				val |= (array[i+1] << 16);	\
			_reg_write(addr + (i << 1), val);	\
		}						\
	} while (0)

#define REG_ARRAY_UPDATE2(addr, array)				\
	REG_ARRAY_UPDATE2_SIZE(addr, array, ARRAY_SIZE(array))

#define REG_ARRAY_UPDATE4(addr, array)				\
	do {							\
		uint16_t i;					\
		for (i = 0; i < ARRAY_SIZE(array); i += 4) {	\
			val = array[i];				\
			if ((i + 1) < ARRAY_SIZE(array))	\
				val |= (array[i+1] << 8);	\
			if ((i + 2) < ARRAY_SIZE(array))	\
				val |= (array[i+2] << 16);	\
			if ((i + 3) < ARRAY_SIZE(array))	\
				val |= (array[i+3] << 24);	\
			_reg_write(addr + i, val);		\
		}						\
	} while (0)

#define LTM_REG_ARRAY_UPDATE11(addr, array)                                   \
	do {                                                                  \
		uint32_t val;                                                 \
		val = array[0] | (array[1] << 5) | (array[2] << 10) |         \
		      (array[3] << 15) | (array[4] << 20) | (array[5] << 25); \
		_reg_write(addr, val);                                        \
		val = array[6] | (array[7] << 5) | (array[8] << 10) |         \
		      (array[9] << 15) | (array[10] << 20);                   \
		_reg_write(addr + 4, val);                                    \
	} while (0)

#define LTM_REG_ARRAY_UPDATE30(addr, array)                                   \
	do {                                                                  \
		uint8_t i, j;                                                 \
		uint32_t val;                                                 \
		for (i = 0, j = 0; i < ARRAY_SIZE(array); i += 6, j++) {      \
			val = array[i] | (array[i + 1] << 5) |                \
			      (array[i + 2] << 10) | (array[i + 3] << 15) |   \
			      (array[i + 4] << 20) | (array[i + 5] << 25);    \
			_reg_write(addr + j * 4, val);                        \
		}                                                             \
	} while (0)

enum ISP_RAW_PATH {
	ISP_RAW_PATH_LE = 0,
	ISP_RAW_PATH_SE,
	ISP_RAW_PATH_MAX,
};

/*
 * To indicate the 1st two pixel in the bayer_raw.
 */
enum ISP_BAYER_TYPE {
	ISP_BAYER_TYPE_BG = 0,
	ISP_BAYER_TYPE_GB,
	ISP_BAYER_TYPE_GR,
	ISP_BAYER_TYPE_RG,
	ISP_BAYER_TYPE_MAX,
};

enum ISP_BNR_OUT {
	ISP_BNR_OUT_BYPASS = 0,
	ISP_BNR_OUT_B_DELAY,
	ISP_BNR_OUT_FACTOR,
	ISP_BNR_OUT_B_NL,
	ISP_BNR_OUT_RESV_0,
	ISP_BNR_OUT_RESV_1,
	ISP_BNR_OUT_RESV_2,
	ISP_BNR_OUT_RESV_3,
	ISP_BNR_OUT_B_OUT,
	ISP_BNR_OUT_INTENSITY,
	ISP_BNR_OUT_DELTA,
	ISP_BNR_OUT_NOT_SM,
	ISP_BNR_OUT_FLAG_V,
	ISP_BNR_OUT_FLAG_H,
	ISP_BNR_OUT_FLAG_D45,
	ISP_BNR_OUT_FLAG_D135,
	ISP_BNR_OUT_MAX,
};

enum ISP_IP_MODULE {
	ISP_IP_CSIBDG,
	ISP_IP_CLSC,
	ISP_IP_AE,
	ISP_IP_PRE_VI_SEL,
	ISP_IP_AF,
	ISP_IP_SE_BNR,
	ISP_IP_BLC_WBG,
	ISP_IP_FUSION,
	ISP_IP_MAPCURVE,
	ISP_IP_DPC,
	ISP_IP_BNR,
	ISP_IP_LLSC,
	ISP_IP_GTM,
	ISP_IP_DRC,
	ISP_IP_CFA,
	ISP_IP_PFR,
	ISP_IP_PRE_EE_EXT,
	ISP_IP_CCM,
	ISP_IP_RGBGAM,
	ISP_IP_DEHAZE,
	ISP_IP_RGB_DITHER,
	ISP_IP_CLUT,
	ISP_IP_CSC,
	ISP_IP_YUV_DITHER,
	ISP_IP_LDCI,
	ISP_IP_PRE_EE_FRONT,
	ISP_IP_PRE_EE_BACK,
	ISP_IP_MCTF,
	ISP_IP_POST_EE,
	ISP_IP_CNR,
	ISP_IP_CA,
	ISP_IP_CA_LITE,
	ISP_IP_MAX,
};

enum ISP_FS_OUT {
	ISP_FS_OUT_FS = 0,
	ISP_FS_OUT_LONG,
	ISP_FS_OUT_SHORT,
	ISP_FS_OUT_ZERO,
	ISP_FS_OUT_MAX,
};

enum isp_dump_grp {
	ISP_DUMP_PRERAW = 0x1,
	ISP_DUMP_POSTRAW = 0x2,
	ISP_DUMP_ALL = 0x4,
	ISP_DUMP_DMA = 0x8,
	ISP_DUMP_ALL_DMA = 0x10,
};

enum ISP_PRE_PROC_ID {
	ISP_IR_PRE_PROC_ID_LE,
	ISP_IR_PRE_PROC_ID_SE,
	ISP_IR_PRE_PROC_ID_MAX
};

enum ISP_DMA_ID {
	// wdma0 dma_group=1
	ISP_DMA_ID_CSI0_BDG0 = 0,
	ISP_DMA_ID_CSI0_BDG1 = 4,
	ISP_DMA_ID_CSI0_BDG2 = 8,
	ISP_DMA_ID_CSI0_BDG3,
	ISP_DMA_ID_CSI2_BDG0,
	ISP_DMA_ID_BT0_LITE0,
	ISP_DMA_ID_BT0_LITE1,
	ISP_DMA_ID_BT0_LITE2,
	ISP_DMA_ID_BT0_LITE3,
	// wdma1 dma_group=2
	ISP_DMA_ID_AF_W = 0,
	ISP_DMA_ID_AE_RAW0_HIST_LE,
	ISP_DMA_ID_AE_RAW0_HIST_SE,
	ISP_DMA_ID_AE_RAW1_HIST_LE,
	ISP_DMA_ID_GMS,
	ISP_DMA_ID_DRC_POLY_W,
	ISP_DMA_ID_DRC_HIST,
	ISP_DMA_ID_LSCR_HIST,
	// wdma2 dma_group=3
	ISP_DMA_ID_CNR_Y_W = 0,
	ISP_DMA_ID_CNR_C_W,
	ISP_DMA_ID_LDCI_HIST,
	ISP_DMA_ID_LDCI_W,
	ISP_DMA_ID_YUV_CROP_Y,
	ISP_DMA_ID_YUV_CROP_C,
	ISP_DMA_ID_TNR_ST_Y,
	ISP_DMA_ID_TNR_ST_C,
	ISP_DMA_ID_TNR_ST_MV,
	ISP_DMA_ID_TNR_ST_MO,
	ISP_DMA_ID_TNR_ST_FCB,
	ISP_DMA_ID_TNR_ST_MSP,
	// wdma3 dma_group=4
	ISP_DMA_ID_CSI1_BDG0 = 0,
	ISP_DMA_ID_CSI1_BDG1,
	ISP_DMA_ID_AE_FE0_HIST_LE,
	ISP_DMA_ID_AE_FE0_HIST_SE,
	ISP_DMA_ID_AE_FE1_HIST_LE,
	ISP_DMA_ID_AE_FE1_HIST_SE,
	ISP_DMA_ID_AE_FE2_HIST_LE,

	// rdma1 dma_group=7
	ISP_DMA_ID_PRE_RAW_VI_SEL_LE = 0,
	ISP_DMA_ID_PRE_RAW_VI_SEL_SE = 4,
	ISP_DMA_ID_FE0_LSC_RDMA_CTL,
	ISP_DMA_ID_FE1_LSC_RDMA_CTL,
	ISP_DMA_ID_FE2_LSC_RDMA_CTL,

	// rdma2 dma_group=6
	ISP_DMA_ID_DRC_POLY_R = 0,
	ISP_DMA_ID_RAW_LSC_RDMA_CTL,

	// rdma3 dma_group=5
	ISP_DMA_ID_CNR_Y_R = 0,
	ISP_DMA_ID_CNR_C_R,
	ISP_DMA_ID_LDCI_R,
	ISP_DMA_ID_YUV_RDMA_Y,
	ISP_DMA_ID_YUV_RDMA_C,
	ISP_DMA_ID_TNR_LD_Y,
	ISP_DMA_ID_TNR_LD_C,
	ISP_DMA_ID_TNR_LD_MV,
	ISP_DMA_ID_TNR_LD_MO,
	ISP_DMA_ID_TNR_LD_FCB,

	ISP_DMA_ID_MAX,
};

struct isp_dump_info {
	uint64_t phy_base;
	uint64_t reg_base;
	uint32_t blk_size;
};

struct isp_vblock_info {
	uint32_t block_id;
	uint32_t block_size;
	uint64_t reg_base;
};

struct tile {
	u16 start;
	u16 end;
};

struct isp_ccm_cfg {
	u16 coef[3][3];
};

struct _fe_dbg_i {
	uint32_t		fe_idle_sts;
	uint32_t		fe_done_sts;
};

struct _be_dbg_i {
	uint32_t		be_done_sts;
	uint32_t		be_dma_idle_sts;
};

struct _post_dbg_i {
	uint32_t		top_sts_0;
	uint32_t		top_sts_1;
};

struct _dma_dbg_i {
	uint32_t		wdma_1_err_sts;
	uint32_t		wdma_1_idle;
	uint32_t		wdma_2_err_sts;
	uint32_t		wdma_2_idle;
	uint32_t		wdma_3_err_sts;
	uint32_t		wdma_3_idle;
	uint32_t		wdma_4_err_sts;
	uint32_t		wdma_4_idle;
	uint32_t		rdma_1_err_sts;
	uint32_t		rdma_1_idle;
	uint32_t		rdma_2_err_sts;
	uint32_t		rdma_2_idle;
	uint32_t		rdma_3_err_sts;
	uint32_t		rdma_3_idle;
};

struct _isp_dg_info {
	struct _fe_dbg_i	fe_sts;
	struct _post_dbg_i	post_sts;
	struct _dma_dbg_i	dma_sts;
	uint32_t		isp_top_sts;
	uint32_t		bdg_chn_debug[ISP_FE_CHN_MAX];
	uint32_t		bdg_int_sts_0;
	uint32_t		bdg_int_sts_1;
	uint32_t		bdg_fifo_of_cnt;
	uint8_t			bdg_w_gt_cnt[ISP_FE_CHN_MAX];
	uint8_t			bdg_w_ls_cnt[ISP_FE_CHN_MAX];
	uint8_t			bdg_h_gt_cnt[ISP_FE_CHN_MAX];
	uint8_t			bdg_h_ls_cnt[ISP_FE_CHN_MAX];
};

struct _isp_cfg {
	uint32_t		csibdg_width;
	uint32_t		csibdg_height;
	uint32_t		max_width;
	uint32_t		max_height;
	uint32_t		post_img_w;
	uint32_t		post_img_h;
	uint32_t		drop_ref_frm_num;
	uint32_t		drop_frm_cnt;
	uint32_t		isp_reset_frm;
	uint32_t		first_frm_cnt;
	struct vi_rect		crop;
	struct vi_rect		crop_se;
	struct vi_rect		rawdump_crop;
	struct vi_rect		rawdump_crop_se;
	struct vi_rect		postout_crop;
	struct _isp_dg_info	dg_info;
	enum ISP_BAYER_TYPE	rgb_color_mode;
	// enum _VI_INTF_MODE_E	infMode;
	// enum _VI_WORK_MODE_E	muxMode;
	// enum _VI_YUV_DATA_SEQ_E enDataSeq;

	uint32_t		is_patgen_en		: 1;
	uint32_t		is_yuv_bypass_path	: 1;
	uint32_t		is_422_to_420		: 1;
	uint32_t		is_hdr_on		: 1;
	uint32_t		is_hdr_detail_en	: 1;
	uint32_t		is_fbc_on		: 1;
	uint32_t		is_offline_scaler	: 1;
	uint32_t		is_slice_buf_on		: 1;
	uint32_t		is_drop_next_frame	: 1;
	uint32_t		is_ctrl_inited		: 1;
	uint32_t		is_raw_replay		: 1;
};

/*
 * @src_width: width of original image
 * @src_height: height of original image
 * @img_width: width of image after crop
 * @img_height: height of image after crop
 * @pyhs_regs: index by enum ISP_BLK_ID_T, always phys reg
 * @vreg_bases: index by enum ISP_BLK_ID_T
 * @vreg_bases_pa: index by enum ISP_BLK_ID_T
 *
 * @rgb_color_mode: bayer_raw type after crop could change
 *
 * @cam_id: preraw(0,1)
 * @is_offline_postraw: postraw src offline(from dram)
 */
struct isp_ctx {
	uint32_t		src_width;
	uint32_t		src_height;
	uint32_t		img_width;
	uint32_t		img_height;
	uint32_t		crop_x;
	uint32_t		crop_y;
	uint32_t		crop_se_x;
	uint32_t		crop_se_y;

	uintptr_t		*phys_regs;

	struct _isp_cfg		isp_pipe_cfg[ISP_PRERAW_MAX];
	uint8_t			isp_pipe_enable[ISP_PRERAW_MAX];
	enum ISP_BAYER_TYPE	rgb_color_mode[ISP_PRERAW_MAX];
	uint8_t                 rawb_chnstr_num;
	uint8_t			total_dev_num;
	uint8_t			total_chn_num;
	uint8_t			is_post_done;

	uint8_t			cam_id;
	uint8_t			gamma_tbl_idx;
	uint8_t			cnr_scale_ratio;
	uint32_t		is_multi_sensor     : 1;
	uint32_t		is_yuv_sensor       : 1;
	uint32_t		is_hdr_on           : 1;
	uint32_t		is_3dnr_on          : 1;
	uint32_t		is_dpcm_on          : 1;
	uint32_t		is_offline_postraw  : 1;
	uint32_t		is_sublvds_path     : 1;
	uint32_t		is_fbc_on           : 1;
	uint32_t		is_ctrl_inited      : 1;
	uint32_t		is_slice_buf_on     : 1;
	uint32_t		is_rtl_config       : 1;
	uint32_t        	is_sensor_input     : 1; //sensor input
};

struct vi_fbc_cfg {
	u8	cu_size;
	u8	target_cr; //compression ratio
	u8	is_lossless; // lossless or lossy
	u32	y_bs_size; //Y WDMA seglen
	u32	c_bs_size; //C WDMA seglen
	u32	y_buf_size; //total Y buf size
	u32	c_buf_size; //total C buf size
};

struct slc_cfg_s {
	u32 le_buf_size;
	u32 se_buf_size;
	u32 le_w_thshd;
	u32 se_w_thshd;
	u32 le_r_thshd;
	u32 se_r_thshd;
};

struct slice_buf_s {
	u16 line_delay; //sensor exposure ratio
	u16 buffer; //cover for read/write latency, axi latency..etc
	u8  main_max_grid_size; //main path rgbmap grid size
	u8  min_r_thshd; // minimum read threshold
	struct slc_cfg_s main_path;
};

/**********************************************************
 *	SW scenario path check APIs
 **********************************************************/
u32 _is_fe_post_offline(struct isp_ctx *ctx);

/****************************************************************************
 * Interfaces
 ****************************************************************************/
void vi_set_base_addr(void *base);
uintptr_t *isp_get_phys_reg_bases(void);
void isp_debug_dump(struct isp_ctx *ctx);
/**
 * isp_init - setup isp
 *
 * @param :
 */
void isp_init(struct isp_ctx *ctx);


/**
 * isp_reset - do reset. This can be activated only if dma stop to avoid
 * hang fabric.
 *
 */
void isp_reset(struct isp_ctx *ctx);

/**
 * isp_stream_on - start/stop isp stream.
 *
 * @param on: 1 for stream start, 0 for stream stop
 */
void isp_streaming(struct isp_ctx *ctx, uint32_t on, enum cvi_isp_raw raw_num);

void ispblk_preraw_fe_config(struct isp_ctx *ctx, enum cvi_isp_raw raw_num);
void ispblk_preraw_vi_sel_config(struct isp_ctx *ctx);
void ispblk_rawtop_config(struct isp_ctx *ctx, const enum cvi_isp_raw raw_num);
void ispblk_rgbtop_config(struct isp_ctx *ctx, const enum cvi_isp_raw raw_num);
void ispblk_yuvtop_config(struct isp_ctx *ctx, const enum cvi_isp_raw raw_num);
void ispblk_isptop_config(struct isp_ctx *ctx);

void ispblk_crop_enable(struct isp_ctx *ctx, int crop_id, bool en);
int ispblk_crop_config(struct isp_ctx *ctx, int crop_id, struct vi_rect crop);

int csibdg_find_hwid(enum cvi_isp_raw raw_num);
int fe_find_hwid(enum cvi_isp_raw raw_num);
int lsc_find_hwid(enum cvi_isp_raw raw_num, bool is_le);
int ae_find_hwid(enum cvi_isp_raw raw_num);
int ae_dma_find_hwid(enum cvi_isp_raw raw_num, bool is_le);
int csibdg_dma_find_hwid(enum cvi_isp_raw raw_num, bool is_le);

uint64_t ispblk_dma_getaddr(struct isp_ctx *ctx, uint32_t dmaid);
int ispblk_dma_config(struct isp_ctx *ctx, int dmaid, uint64_t buf_addr);
void ispblk_dma_setaddr(struct isp_ctx *ctx, uint32_t dmaid, uint64_t buf_addr);
void ispblk_dma_enable(struct isp_ctx *ctx, uint32_t dmaid, uint32_t on, uint8_t dma_disable);
int ispblk_dma_buf_get_size(struct isp_ctx *ctx, int dmaid, u8 raw_num);
void ispblk_dma_set_sw_mode(struct isp_ctx *ctx, uint32_t dmaid, bool is_sw_mode);

/****************************************************************************
 *	PRERAW FE SUBSYS
 ****************************************************************************/
void ispblk_csidbg_dma_wr_en(struct isp_ctx *ctx, const enum cvi_isp_raw raw_num, const u8 chn_num, const u8 en);
void ispblk_csibdg_wdma_crop_config(struct isp_ctx *ctx, const enum cvi_isp_raw raw_num, struct vi_rect crop, u8 en);
void ispblk_csibdg_crop_update(struct isp_ctx *ctx, enum cvi_isp_raw raw_num, bool en);
int ispblk_csibdg_config(struct isp_ctx *ctx, enum cvi_isp_raw raw_num);

/****************************************************************************
 *	RAW TOP SUBSYS
 ****************************************************************************/
void ispblk_lsc_config(struct isp_ctx *ctx, enum cvi_isp_raw raw_num, bool is_le, bool en);
void ispblk_bnr_lite_config(struct isp_ctx *ctx, bool en);
void ispblk_aehist_config(struct isp_ctx *ctx, enum cvi_isp_raw raw_num, bool en);
void ispblk_aehist_reset(struct isp_ctx *ctx, int blk_id);
void ispblk_blc_set_offset(struct isp_ctx *ctx,
			   uint16_t roffset, uint16_t groffset,
			   uint16_t gboffset, uint16_t boffset);
void ispblk_blc_set_2ndoffset(struct isp_ctx *ctx,
			      uint16_t roffset, uint16_t groffset,
			      uint16_t gboffset, uint16_t boffset);
void ispblk_blc_set_gain(struct isp_ctx *ctx,
			 uint16_t rgain, uint16_t grgain,
			 uint16_t gbgain, uint16_t bgain);
void ispblk_blc_enable(struct isp_ctx *ctx, bool enable);
int ispblk_wbg_config(struct isp_ctx *ctx, uint16_t rgain, uint16_t ggain, uint16_t bgain);
int ispblk_wbg_enable(struct isp_ctx *ctx, bool enable);
void ispblk_fusion_hdr_cfg(struct isp_ctx *ctx, enum cvi_isp_raw raw_num);
void ispblk_fusion_config(struct isp_ctx *ctx, bool enable, int out_sel);
void ispblk_mapcurve_config(struct isp_ctx *ctx, bool enable);
void ispblk_dpc_config(struct isp_ctx *ctx, bool enable, uint8_t test_case);
void ispblk_bnr_config(struct isp_ctx *ctx, bool enable);
void ispblk_lscr_config(struct isp_ctx *ctx, bool enable);
void ispblk_lscr_config_dma(struct isp_ctx *ctx, uint64_t dma_addr, bool en);
void ispblk_dpc_set_static(struct isp_ctx *ctx, uint16_t offset, uint32_t *bps, uint16_t count);
void ispblk_af_config(struct isp_ctx *ctx, bool enable);
void ispblk_drc_config(struct isp_ctx *ctx, bool enable);
void ispblk_drc_lut(struct isp_ctx *ctx, uint8_t sel, uint16_t *data, bool check_lut);
void ispblk_cfa_config(struct isp_ctx *ctx);

/****************************************************************************
 *	RGB TOP SUBSYS
 ****************************************************************************/
void ispblk_edge_ext_config(struct isp_ctx *ctx, bool en);
void ispblk_pfr_config(struct isp_ctx *ctx, bool en);
void ispblk_ccm_config(struct isp_ctx *ctx, bool en, struct isp_ccm_cfg *cfg);
void ispblk_gamma_config(struct isp_ctx *ctx, bool en, uint8_t sel, uint16_t *data, uint8_t inv, bool check_lut);
void ispblk_gamma_enable(struct isp_ctx *ctx, bool enable);
void ispblk_clut_config(struct isp_ctx *ctx, bool en,
				uint16_t *r_lut, uint16_t *g_lut, uint16_t *b_lut, bool check_lut);
void ispblk_rgbdither_config(struct isp_ctx *ctx, bool en, bool mod_en, bool histidx_en, bool fmnum_en);
void ispblk_csc_config(struct isp_ctx *ctx);

/****************************************************************************
 *	YUV TOP SUBSYS
 ****************************************************************************/
int ispblk_post_ee_config(struct isp_ctx *ctx, bool en);
int ispblk_yuvdither_config(struct isp_ctx *ctx, uint8_t sel, bool en,
			    bool mod_en, bool histidx_en, bool fmnum_en);
void ispblk_pre_ee_front_config(struct isp_ctx *ctx, bool en);
void ispblk_pre_ee_back_config(struct isp_ctx *ctx, bool en);
void ispblk_ldci_config(struct isp_ctx *ctx, bool en, uint8_t test_case);
void ispblk_tnr_config(struct isp_ctx *ctx, bool en);
void ispblk_fbc_clear_fbcd_ring_base(struct isp_ctx *ctx, u8 raw_num);
void ispblk_fbc_chg_to_sw_mode(struct isp_ctx *ctx, u8 raw_num);
void vi_fbc_calculate_size(struct isp_ctx *ctx, u8 raw_num);
void ispblk_fbc_ring_buf_config(struct isp_ctx *ctx, u8 en);
void ispblk_fbcd_config(struct isp_ctx *ctx, bool en);
void ispblk_fbce_config(struct isp_ctx *ctx, bool en);
void ispblk_cnr_config(struct isp_ctx *ctx, bool en);
void ispblk_ca_config(struct isp_ctx *ctx, bool en, uint8_t mode);
void ispblk_ca_lite_config(struct isp_ctx *ctx, bool en);
void ispblk_ycur_config(struct isp_ctx *ctx, bool en, uint8_t sel, uint16_t *data, bool check_lut);
void ispblk_ycur_enable(struct isp_ctx *ctx, bool enable, uint8_t sel);

void isp_pre_trig(struct isp_ctx *ctx, enum cvi_isp_raw raw_num, const u8 chn_num);
void isp_post_trig(struct isp_ctx *ctx, enum cvi_isp_raw raw_num);

void isp_intr_set_mask(struct isp_ctx *ctx);
void isp_intr_status(
	struct isp_ctx *ctx,
	union REG_ISP_TOP_INT_EVENT0 *s0,
	union REG_ISP_TOP_INT_EVENT1 *s1,
	union REG_ISP_TOP_INT_EVENT2 *s2);
void isp_csi_intr_status(
	struct isp_ctx *ctx,
	enum cvi_isp_raw raw_num,
	union REG_ISP_CSI_BDG_INTERRUPT_STATUS_0 *s0,
	union REG_ISP_CSI_BDG_INTERRUPT_STATUS_1 *s1);

/****************************************************************************
 *	Runtime Control Flow Config
 ****************************************************************************/
void isp_first_frm_reset(struct isp_ctx *ctx, uint8_t reset);
void ispblk_post_yuv_cfg_update(struct isp_ctx *ctx, const enum cvi_isp_raw raw_num);
void ispblk_post_cfg_update(struct isp_ctx *ctx, const enum cvi_isp_raw raw_num);
int ispblk_dma_get_size(struct isp_ctx *ctx, int dmaid, uint32_t _w, uint32_t _h);
uint32_t ispblk_csibdg_chn_dbg(struct isp_ctx *ctx, enum cvi_isp_raw raw_num, enum cvi_isp_pre_chn_num chn);
struct _fe_dbg_i ispblk_fe_dbg_info(struct isp_ctx *ctx, enum cvi_isp_raw raw_num);
struct _post_dbg_i ispblk_post_dbg_info(struct isp_ctx *ctx);
struct _dma_dbg_i ispblk_dma_dbg_info(struct isp_ctx *ctx);
int isp_frm_err_handler(struct isp_ctx *ctx, const enum cvi_isp_raw err_raw_num, const u8 step);

/****************************************************************************
 *	YUV Bypass Control Flow Config
 ****************************************************************************/
void ispblk_csibdg_yuv_bypass_config(struct isp_ctx *ctx, const enum cvi_isp_raw raw_num);
u32 ispblk_dma_yuv_bypass_config(struct isp_ctx *ctx, uint32_t dmaid, uint64_t buf_addr,
					const enum cvi_isp_raw raw_num);

/****************************************************************************
 *	Slice buffer Control
 ****************************************************************************/
void vi_calculate_slice_buf_setting(struct isp_ctx *ctx, enum cvi_isp_raw raw_num);
void isp_slice_buf_trig(struct isp_ctx *ctx, enum cvi_isp_raw raw_num);
void ispblk_slice_buf_config(struct isp_ctx *ctx, const enum cvi_isp_raw raw_num, u8 en);

/*******************************************************************************
 *	Tuning interfaces
 ******************************************************************************/
void vi_tuning_gamma_ips_update(
	struct isp_ctx *ctx,
	enum cvi_isp_raw raw_num);
void vi_tuning_ldci_update(
	struct isp_ctx *ctx,
	enum cvi_isp_raw raw_num);
void vi_tuning_drc_update(
	struct isp_ctx *ctx,
	enum cvi_isp_raw raw_num);
void vi_tuning_clut_update(
	struct isp_ctx *ctx,
	enum cvi_isp_raw raw_num);
int vi_tuning_get_clut_tbl_idx(void);
int vi_tuning_sw_init(void);
int vi_tuning_buf_setup(void);
void vi_tuning_buf_release(void);
void *vi_get_tuning_buf_addr(u32 *size);
void vi_tuning_buf_clear(void);

/*******************************************************************************
 *	Tuning modules update
 ******************************************************************************/
void pre_fe_tuning_update(
	struct isp_ctx *ctx,
	enum cvi_isp_raw raw_num);
void postraw_tuning_update(
	struct isp_ctx *ctx,
	enum cvi_isp_raw raw_num);

void ispblk_blc_tun_cfg(
	struct isp_ctx *ctx,
	struct cvi_vip_isp_blc_config *cfg,
	const enum cvi_isp_raw raw_num);
void ispblk_wbg_tun_cfg(
	struct isp_ctx *ctx,
	struct cvi_vip_isp_wbg_config *cfg,
	const enum cvi_isp_raw raw_num);
/****************************************************************************
 *	Postraw Tuning Config
 ****************************************************************************/
void ispblk_ccm_tun_cfg(
	struct isp_ctx *ctx,
	struct cvi_vip_isp_ccm_config *cfg,
	const enum cvi_isp_raw raw_num);
void ispblk_cacp_tun_cfg(
	struct isp_ctx *ctx,
	struct cvi_vip_isp_cacp_config *cfg,
	const enum cvi_isp_raw raw_num);
void ispblk_ca2_tun_cfg(
	struct isp_ctx *ctx,
	struct cvi_vip_isp_ca2_config *cfg,
	const enum cvi_isp_raw raw_num);
void ispblk_gamma_tun_cfg(
	struct isp_ctx *ctx,
	struct cvi_vip_isp_gamma_config *cfg,
	const enum cvi_isp_raw raw_num);
void ispblk_demosiac_tun_cfg(
	struct isp_ctx *ctx,
	struct cvi_vip_isp_demosiac_config *cfg,
	const enum cvi_isp_raw raw_num,
	enum ISP_RAW_PATH path);
void ispblk_lsc_tun_cfg(
	struct isp_ctx *ctx,
	struct cvi_vip_isp_lsc_config *cfg,
	const enum cvi_isp_raw raw_num);
void ispblk_bnr_tun_cfg(
	struct isp_ctx *ctx,
	struct cvi_vip_isp_bnr_config *cfg,
	const enum cvi_isp_raw raw_num,
	enum ISP_RAW_PATH path);
void ispblk_clut_tun_cfg(
	struct isp_ctx *ctx,
	struct cvi_vip_isp_clut_config *cfg,
	const enum cvi_isp_raw raw_num);
void ispblk_drc_tun_cfg(
	struct isp_ctx *ctx,
	struct cvi_vip_isp_drc_config *cfg,
	const enum cvi_isp_raw raw_num);
void ispblk_cnr_tun_cfg(
	struct isp_ctx *ctx,
	struct cvi_vip_isp_cnr_config *cfg,
	const enum cvi_isp_raw raw_num);
void ispblk_tnr_tun_cfg(
	struct isp_ctx *ctx,
	struct cvi_vip_isp_tnr_config *cfg,
	const enum cvi_isp_raw raw_num);
void ispblk_ee_tun_cfg(
	struct isp_ctx *ctx,
	struct cvi_vip_isp_ee_config *cfg,
	const enum cvi_isp_raw raw_num);
void ispblk_pre_ee_tun_cfg(
	struct isp_ctx *ctx,
	struct cvi_vip_isp_pre_ee_config *cfg,
	const enum cvi_isp_raw raw_num);
void ispblk_fswdr_tun_cfg(
	struct isp_ctx *ctx,
	struct cvi_vip_isp_fswdr_config *cfg,
	const enum cvi_isp_raw raw_num);
void ispblk_fswdr_update_rpt(
	struct isp_ctx *ctx,
	struct cvi_vip_isp_fswdr_report *cfg);
void ispblk_ldci_tun_cfg(
	struct isp_ctx *ctx,
	struct cvi_vip_isp_ldci_config *cfg,
	const enum cvi_isp_raw raw_num);
void ispblk_ycur_tun_cfg(
	struct isp_ctx *ctx,
	struct cvi_vip_isp_ycur_config *cfg,
	const enum cvi_isp_raw raw_num);
void ispblk_dci_tun_cfg(
	struct isp_ctx *ctx,
	struct cvi_vip_isp_dci_config *cfg,
	const enum cvi_isp_raw raw_num);
void ispblk_dhz_tun_cfg(
	struct isp_ctx *ctx,
	struct cvi_vip_isp_dhz_config *cfg,
	const enum cvi_isp_raw raw_num);
void ispblk_csc_tun_cfg(
	struct isp_ctx *ctx,
	struct cvi_vip_isp_csc_config *cfg,
	const enum cvi_isp_raw raw_num);
void ispblk_dpc_tun_cfg(
	struct isp_ctx *ctx,
	struct cvi_vip_isp_dpc_config *cfg,
	const enum cvi_isp_raw raw_num);
void ispblk_ae_tun_cfg(
	struct isp_ctx *ctx,
	struct cvi_vip_isp_ae_config *cfg,
	const enum cvi_isp_raw raw_num);
void ispblk_af_tun_cfg(
	struct isp_ctx *ctx,
	struct cvi_vip_isp_af_config *cfg,
	const enum cvi_isp_raw raw_num);
void ispblk_edge_ext_tun_cfg(
	struct isp_ctx *ctx,
	struct cvi_vip_isp_edge_ext_config *cfg,
	const enum cvi_isp_raw raw_num);
void ispblk_mono_tun_cfg(
	struct isp_ctx *ctx,
	struct cvi_vip_isp_mono_config *cfg,
	const enum cvi_isp_raw raw_num);
#if 0
/****************************************************************************
 *	Pre Be Tuning Config
 ****************************************************************************/
void ispblk_lscr_tun_cfg(
	struct isp_ctx *ctx,
	struct cvi_vip_isp_lscr_config *cfg,
	const enum cvi_isp_raw raw_num);
void ispblk_preproc_tun_cfg(
	struct isp_ctx *ctx,
	struct cvi_vip_isp_preproc_config *cfg,
	const enum cvi_isp_raw raw_num);
#endif

#ifdef PORTING_TEST
void dump_pass_register(struct isp_ctx *ctx, enum cvi_isp_raw raw_num);
void vi_ip_test_cases_init(struct isp_ctx *ctx);
void vi_ip_test_cases_uninit(struct isp_ctx *ctx);
void ispblk_patgen_config_pat(struct isp_ctx *ctx, enum cvi_isp_raw raw_num, uint8_t test_case);
void ispblk_isptop_fpga_config(struct isp_ctx *ctx, uint16_t test_case);
void ispblk_ca_read_lut_test(struct isp_ctx *ctx, bool en, uint8_t mode);
void ispblk_ycur_read_lut_test(struct isp_ctx *ctx, uint8_t sel);
void ispblk_rgbgamma_read_lut_test(struct isp_ctx *ctx, uint8_t inv, uint8_t sel);
void ispblk_ldci_read_lut_test(struct isp_ctx *ctx, uint8_t sel);
void ispblk_clut_read_lut_test(struct isp_ctx *ctx);
void config_lut_table(struct isp_ctx *ctx);
#endif

#ifdef __cplusplus
}
#endif

#endif /* __VI_DRV_H__ */
