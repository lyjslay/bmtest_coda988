#include <errno.h>
#include <inttypes.h>
#include "vi.h"
#include "vi_defines.h"
#include "vi_common.h"
#include "vi_isp_buf_ctrl.h"
#include "vip/vi_drv.h"
#include "inc/isp_reg.h"
#include "vip/reg.h"
#include "system_common.h"
#include "timer.h"
#include "../vpss/scaler.h"

extern struct cvi_vi_dev g_dev;

/*******************************************************
 *  MACRO defines
 ******************************************************/

#define vi_err_wake_up_th(vdev, raw_num)

#define DMA_SETUP_2(id, raw_num)					\
	do {								\
		bufaddr = _mempool_get_addr();				\
		ispblk_dma_setaddr(ictx, id, bufaddr);			\
		bufsize = ispblk_dma_buf_get_size(ictx, id, raw_num);	\
		_mempool_pop(bufsize);					\
	} while (0)

#define DMA_SETUP(id)						\
	do {							\
		bufaddr = _mempool_get_addr();			\
		bufsize = ispblk_dma_config(ictx, id, bufaddr); \
		_mempool_pop(bufsize);				\
	} while (0)

#define VI_SHARE_MEM_SIZE	(0x2000)
#define VI_PROFILE
#define VI_MAX_SNS_CFG_NUM	(0x10)

/* In practical application, it is necessary to drop the frame of AE convergence process.
 * But in vi-vpss online & vpss-vc sbm scenario, there is no way to drop the frame.
 * Use cover with black to avoid this problem.
 */
#ifndef PORTING_TEST
#define COVER_WITH_BLACK
#endif
/*******************************************************
 *  Global variables
 ******************************************************/
u32 vi_log_lv = VI_ERR | VI_WARN | VI_NOTICE | VI_INFO | VI_DBG;

struct cvi_vi_ctx *gViCtx;
struct cvi_overflow_info *gOverflowInfo;

/*******************************************************
 *  Internal APIs
 ******************************************************/

/**
 * _mempool_reset - reset the byteused and assigned buffer for each dma
 *
 */

void vi_mempool_set(uint64_t base, uint32_t size)
{
	isp_mempool.base = base;
	isp_mempool.size = size;

	vi_pr(VI_INFO, "isp_mempool.base:0x%lx, size:0x%x\n", isp_mempool.base, isp_mempool.size);
}

static void _vi_mempool_reset(void)
{
	isp_mempool.byteused = 0;

	memset(isp_bufpool, 0x0, (sizeof(struct _membuf) * ISP_PRERAW1)); //ISP_PRERAW_MAX
}

/**
 * _mempool_get_addr - get mempool's latest address.
 *
 * @return: the latest address of the mempool.
 */
static uint64_t _mempool_get_addr(void)
{
	return isp_mempool.base + isp_mempool.byteused;
}

/**
 * _mempool_pop - acquire a buffer-space from mempool.
 *
 * @param size: the space acquired.
 * @return: negative if no enough space; o/w, the address of the buffer needed.
 */
static int64_t _mempool_pop(uint32_t size)
{
	int64_t addr;

	size = VI_ALIGN(size);

	if ((isp_mempool.byteused + size) > isp_mempool.size) {
		vi_pr(VI_ERR, "reserved_memory(0x%x) is not enough. byteused(0x%x) alloc_size(0x%x)\n",
				isp_mempool.size, isp_mempool.byteused, size);
		return -EINVAL;
	}

	addr = isp_mempool.base + isp_mempool.byteused;
	isp_mempool.byteused += size;

	return addr;
}

void _vi_yuv_dma_setup(struct isp_ctx *ctx, const enum cvi_isp_raw raw_num)
{
	// struct isp_buffer *b;
	// uint32_t bufsize_yuyv = 0;
	// uint8_t  i = 0;

	// struct _membuf *pool = &isp_bufpool[raw_num];

	// u8 total_chn = (raw_num == ISP_PRERAW0) ?
	// 		ctx->rawb_chnstr_num :
	// 		ctx->total_chn_num;
	// u8 chn_str = (raw_num == ISP_PRERAW0) ? 0 : ctx->rawb_chnstr_num;

	// for (; chn_str < total_chn; chn_str++) {
	// 	enum ISP_BLK_ID_T dma;

	// 	if (raw_num == ISP_PRERAW2) {
	// 		dma = (chn_str == ctx->rawb_chnstr_num) ? ISP_BLK_ID_DMA_CTL18 : ISP_BLK_ID_DMA_CTL19;
	// 	} else {
	// 		dma = (chn_str == ctx->rawb_chnstr_num) ? ISP_BLK_ID_DMA_CTL12 : ISP_BLK_ID_DMA_CTL13;
	// 	}

	// 	for (i = 0; i < OFFLINE_YUV_BUF_NUM; i++) {
	// 		b = (struct isp_buffer *)PTN_MALLOC(sizeof(*b));
	// 		if (b == NULL) {
	// 			vi_pr(VI_ERR, "yuv_buf isp_buf_%d PTN_MALLOC size(%zu) fail\n", i, sizeof(*b));
	// 			return;
	// 		}
	// 		memset(b, 0, sizeof(*b));
	// 		b->chn_num = chn_str;
	// 		b->is_yuv_frm = true;
	// 		bufsize_yuyv = ispblk_dma_yuv_bypass_config(ctx, dma, 0, raw_num);
	// 		pool->yuv_yuyv[b->chn_num][i] = b->addr = _mempool_pop(bufsize_yuyv);

	// 		if (i == 0)
	// 			ispblk_dma_setaddr(ctx, dma, b->addr);

	// 		isp_buf_queue(&pre_out_queue[b->chn_num], b);
	// 	}
	// }
}

static void _isp_preraw_fe_dma_dump(struct isp_ctx *ictx, enum cvi_isp_raw  raw_num)
{
	// u8 i = 0;
	// char str[64] = "PRERAW_FE";

	// vi_pr(VI_DBG, "***************%s_%d************************\n", str, raw_num);
	// for (i = 0; i < OFFLINE_RAW_BUF_NUM; i++)
	// 	vi_pr(VI_DBG, "bayer_le(0x%#"PRIx64")\n", isp_bufpool[raw_num].bayer_le[i]);

	// if (ictx->isp_pipe_cfg[raw_num].is_hdr_on) {
	// 	for (i = 0; i < OFFLINE_RAW_BUF_NUM; i++)
	// 		vi_pr(VI_DBG, "bayer_se(0x%#"PRIx64")\n", isp_bufpool[raw_num].bayer_se[i]);
	// }

	// if (ictx->isp_pipe_cfg[raw_num].is_yuv_bypass_path &&
	// 	!ictx->isp_pipe_cfg[raw_num].is_offline_scaler) {
	// 	for (i = 0; i < ISP_CHN_MAX; i++) {
	// 		vi_pr(VI_DBG, "yuyv_yuv(0x%#"PRIx64"), yuyv_yuv(0x%#"PRIx64")\n",
	// 			isp_bufpool[raw_num].yuv_yuyv[i][0], isp_bufpool[raw_num].yuv_yuyv[i][1]);
	// 	}
	// }
	// vi_pr(VI_DBG, "*************************************************\n");
}

static void _isp_rawtop_dma_dump(struct isp_ctx *ictx, enum cvi_isp_raw raw_max)
{
	// char str[64] = "RAW_TOP";
	// enum cvi_isp_raw raw = ISP_PRERAW0;

	// vi_pr(VI_DBG, "***************%s************************\n", str);

	// for (; raw < raw_max; raw++) {
	// 	if (raw == ISP_PRERAW1)
	// 		vi_pr(VI_DBG, "***********************Dual sensor dump**********************\n");

	// 	vi_pr(VI_DBG, "ae_le(%#"PRIx64", %#"PRIx64")\n",
	// 			isp_bufpool[raw].sts_mem[0].ae_le.phy_addr,
	// 			isp_bufpool[raw].sts_mem[1].ae_le.phy_addr);
	// 	vi_pr(VI_DBG, "awb(%#"PRIx64", %#"PRIx64")\n",
	// 			isp_bufpool[raw].sts_mem[0].awb.phy_addr,
	// 			isp_bufpool[raw].sts_mem[1].awb.phy_addr);

	// 	if (ictx->isp_pipe_cfg[raw].is_hdr_on) {
	// 		vi_pr(VI_DBG, "ae_se(%#"PRIx64", %#"PRIx64")\n",
	// 				isp_bufpool[raw].sts_mem[0].ae_se.phy_addr,
	// 				isp_bufpool[raw].sts_mem[1].ae_se.phy_addr);
	// 	}
	// }

	// vi_pr(VI_DBG, "*************************************************\n");
}

static void _isp_yuvtop_dma_dump(struct isp_ctx *ictx, enum cvi_isp_raw raw_max)
{
	// char str[64] = "YUV_TOP";
	// enum cvi_isp_raw raw = ISP_PRERAW0;

	// vi_pr(VI_DBG, "***************%s************************\n", str);
	// for (; raw < raw_max; raw++) {
	// 	if (raw == ISP_PRERAW1)
	// 		vi_pr(VI_DBG, "***********************Dual sensor dump**********************\n");

	// 	vi_pr(VI_DBG, "ldci(0x%#"PRIx64")\n", isp_bufpool[raw].ldci);

	/* 	if (ictx->is_3dnr_on) {
	// 		vi_pr(VI_DBG, "tdnr_mo(0x%#"PRIx64", 0x%#"PRIx64")\n \
	// 				tdnr_mv(0x%#"PRIx64", 0x%#"PRIx64")\n \
	// 				tdnr_fcb(0x%#"PRIx64", 0x%#"PRIx64")\n",
	// 			isp_bufpool[raw].tdnr_mo[0], isp_bufpool[raw].tdnr_mo[1],
	// 			isp_bufpool[raw].tdnr_mv[0], isp_bufpool[raw].tdnr_mv[1],
	// 			isp_bufpool[raw].tdnr_fcb[0], isp_bufpool[raw].tdnr_fcb[1]);
	// 	}
	// 	vi_pr(VI_DBG, "cnv_v2(0x%#"PRIx64", 0x%#"PRIx64")\n",
	// 		isp_bufpool[raw].cnr[0], isp_bufpool[raw].cnr[1]);
	// }
	// vi_pr(VI_DBG, "*************************************************\n");
	// vi_pr(VI_DBG, "VI total reserved memory(0x%x)\n", isp_mempool.byteused);
	// vi_pr(VI_DBG, "*************************************************\n");*/
}

void _isp_preraw_fe_dma_setup(struct isp_ctx *ictx, enum cvi_isp_raw raw_num)
{
	uint64_t bufaddr = 0;
	uint32_t bufsize = 0;
	uint8_t  i = 0;
	struct isp_buffer *b;

	u32 raw_le, raw_se;

	if (ictx->isp_pipe_cfg[raw_num].is_yuv_bypass_path) { //YUV sensor
		if (!ictx->isp_pipe_cfg[raw_num].is_offline_scaler) //Online mode to scaler
			_vi_yuv_dma_setup(ictx, raw_num);

		goto EXIT;
	}

	raw_le = csibdg_dma_find_hwid(raw_num, true);
	raw_se = csibdg_dma_find_hwid(raw_num, false);

	if (_is_fe_post_offline(ictx)) { //fe->dram->post
		for (i = 0; i < OFFLINE_RAW_BUF_NUM; i++) {
			DMA_SETUP_2(raw_le, raw_num);
			b = (struct isp_buffer *)PTN_MALLOC(sizeof(*b));
			if (b == NULL) {
				vi_pr(VI_ERR, "raw_le isp_buf_%d PTN_MALLOC size(%zu) fail\n", i, sizeof(*b));
				return;
			}
			memset(b, 0, sizeof(*b));
			b->addr = bufaddr;
			b->raw_num = raw_num;
			isp_bufpool[raw_num].bayer_le[i] = b->addr;
			isp_buf_queue(&pre_out_queue[b->raw_num], b);

			vi_pr(VI_DBG, "raw_le %d fake addr:s-%lx e-%lx\n", i, bufaddr/2, (bufaddr + bufsize)/2-1);
		}
		ispblk_dma_setaddr(ictx, raw_le, isp_bufpool[raw_num].bayer_le[0]);
		if (ictx->is_slice_buf_on)
			ispblk_dma_setaddr(ictx, ISP_BLK_ID_DMA_CTL_PRE_RAW_VI_SEL_LE, isp_bufpool[raw_num].bayer_le[0]);

		vi_pr(VI_DBG, "get vi_sel addr:0x%lx\n", ispblk_dma_getaddr(ictx, ISP_BLK_ID_DMA_CTL_PRE_RAW_VI_SEL_LE));
		if (ictx->isp_pipe_cfg[raw_num].is_hdr_on) {
			for (i = 0; i < OFFLINE_RAW_BUF_NUM; i++) {
				DMA_SETUP_2(raw_se, raw_num);
				b = (struct isp_buffer *)PTN_MALLOC(sizeof(*b));
				if (b == NULL) {
					vi_pr(VI_ERR, "raw_se isp_buf_%d PTN_MALLOC size(%zu) fail\n", i, sizeof(*b));
					return;
				}
				memset(b, 0, sizeof(*b));
				b->addr = bufaddr;
				b->raw_num = raw_num;
				isp_bufpool[raw_num].bayer_se[i] = b->addr;
				isp_buf_queue(&pre_out_se_queue[b->raw_num], b);
			}

			ispblk_dma_setaddr(ictx, raw_se, isp_bufpool[raw_num].bayer_se[0]);
			if (ictx->is_slice_buf_on)
				ispblk_dma_setaddr(ictx, ISP_BLK_ID_DMA_CTL_PRE_RAW_VI_SEL_SE,
						  isp_bufpool[raw_num].bayer_se[0]);
		}
	}

EXIT:
	_isp_preraw_fe_dma_dump(ictx, raw_num);
}

void _isp_rawtop_dma_setup(struct isp_ctx *ictx, enum cvi_isp_raw raw_max)
{
	uint64_t bufaddr = 0;
	uint32_t bufsize = 0;

	enum cvi_isp_raw raw = ISP_PRERAW0;

	u8 cfg_dma = false;

	//RGB path
	if (!ictx->isp_pipe_cfg[ISP_PRERAW0].is_yuv_bypass_path) {
		cfg_dma = true;
	} else if (ictx->is_multi_sensor && !ictx->isp_pipe_cfg[ISP_PRERAW1].is_yuv_bypass_path) {
		cfg_dma = true;
	}

	if (cfg_dma == false) //YUV sensor only
		goto EXIT;


	for (raw = ISP_PRERAW0; raw < raw_max; raw++) {

		if (ictx->isp_pipe_cfg[raw].is_yuv_bypass_path) //YUV sensor
			continue;

		DMA_SETUP(ISP_BLK_ID_DMA_CTL_AF_W);
		isp_bufpool[raw].sts_mem[0].af.phy_addr = bufaddr;
		isp_bufpool[raw].sts_mem[0].af.size = bufsize;
		isp_bufpool[raw].sts_mem[1].af.phy_addr = _mempool_pop(bufsize);
		isp_bufpool[raw].sts_mem[1].af.size = bufsize;

		// lscr hist
		DMA_SETUP(ISP_BLK_ID_DMA_CTL_LSCR_HIST);
		ispblk_lscr_config_dma(ictx, bufaddr, true);

		// drc
		uint64_t drc_bufaddr0 = 0, drc_bufaddr1 = 0;
		uint64_t drc_tmp_bufaddr0 = 0, drc_tmp_bufaddr1 = 0;

		DMA_SETUP(ISP_BLK_ID_DMA_CTL_DRC_HIST);
		// DMA_SETUP(ISP_BLK_ID_DMA_CTL_DRC_POLY_W);
		// ispblk_dma_config(ictx, ISP_BLK_ID_DMA_CTL_DRC_POLY_R, bufaddr);
		// isp_bufpool[raw].drc_poly = bufaddr;

		drc_tmp_bufaddr0 = _mempool_get_addr();
		drc_bufaddr0 = VI_256_ALIGN(drc_tmp_bufaddr0);
		bufsize = ispblk_dma_config(ictx, ISP_BLK_ID_DMA_CTL_DRC_POLY_W, drc_bufaddr0);
		_mempool_pop(bufsize + (uint32_t)(drc_bufaddr0 - drc_tmp_bufaddr0));
		isp_bufpool[raw].drc_poly[0] = drc_bufaddr0;

		drc_tmp_bufaddr1 = _mempool_get_addr();
		drc_bufaddr1 = VI_256_ALIGN(drc_tmp_bufaddr1);
		bufsize = ispblk_dma_config(ictx, ISP_BLK_ID_DMA_CTL_DRC_POLY_R, drc_bufaddr1);
		_mempool_pop(bufsize + (uint32_t)(drc_bufaddr1 - drc_tmp_bufaddr1));
		isp_bufpool[raw].drc_poly[1] = drc_bufaddr1;

	}

EXIT:
	_isp_rawtop_dma_dump(ictx, raw_max);
}

void _isp_rgbtop_dma_setup(struct isp_ctx *ictx, enum cvi_isp_raw raw_max)
{
	//no dma setup
}

void _isp_yuvtop_dma_setup(struct isp_ctx *ictx, enum cvi_isp_raw raw_max)
{
	uint64_t bufaddr = 0;
	// uint64_t tmp_bufaddr = 0;
	uint32_t bufsize = 0;

	enum cvi_isp_raw raw = ISP_PRERAW0;

	u8 cfg_dma = false;

	//RGB path
	if (!ictx->isp_pipe_cfg[ISP_PRERAW0].is_yuv_bypass_path) {
		cfg_dma = true;
	} else if (ictx->is_multi_sensor && !ictx->isp_pipe_cfg[ISP_PRERAW1].is_yuv_bypass_path) {
		cfg_dma = true;
	}

	if (cfg_dma == false) //YUV sensor only
		goto EXIT;

	for (raw = ISP_PRERAW0; raw < raw_max; raw++) {

		if (ictx->isp_pipe_cfg[raw].is_yuv_bypass_path) //YUV sensor
			continue;

		// ldci
		//addr 256B alignment workaround
		// tmp_bufaddr = _mempool_get_addr();
		// bufaddr = VI_256_ALIGN(tmp_bufaddr);
		// bufsize = ispblk_dma_config(ictx, ISP_BLK_ID_DMA_CTL_LDCI_W, bufaddr);
		// _mempool_pop(bufsize + (uint32_t)(bufaddr - tmp_bufaddr));
		// isp_bufpool[raw].ldci = bufaddr;
		// ispblk_dma_config(ictx, ISP_BLK_ID_DMA_CTL_LDCI_R, isp_bufpool[raw].ldci);
		uint64_t ldci_bufaddr0 = 0, ldci_bufaddr1 = 0;
		uint64_t ldci_tmp_bufaddr0 = 0, ldci_tmp_bufaddr1 = 0;

		ldci_tmp_bufaddr0 = _mempool_get_addr();
		ldci_bufaddr0 = VI_256_ALIGN(ldci_tmp_bufaddr0);
		bufsize = ispblk_dma_config(ictx, ISP_BLK_ID_DMA_CTL_LDCI_W, ldci_bufaddr0);
		_mempool_pop(bufsize + (uint32_t)(ldci_bufaddr0 - ldci_tmp_bufaddr0));
		isp_bufpool[raw].ldci[0] = ldci_bufaddr0;

		ldci_tmp_bufaddr1 = _mempool_get_addr();
		ldci_bufaddr1 = VI_256_ALIGN(ldci_tmp_bufaddr1);
		bufsize = ispblk_dma_config(ictx, ISP_BLK_ID_DMA_CTL_LDCI_R, ldci_bufaddr1);
		_mempool_pop(bufsize + (uint32_t)(ldci_bufaddr1 - ldci_tmp_bufaddr1));
		isp_bufpool[raw].ldci[1] = ldci_bufaddr1;

		DMA_SETUP(ISP_BLK_ID_DMA_CTL_LDCI_HIST);

		// mctf
		uint64_t mctf_bufaddr0 = 0, mctf_bufaddr1 = 0;
		uint64_t mctf_tmp_bufaddr0 = 0, mctf_tmp_bufaddr1 = 0;

		//	MO
		mctf_tmp_bufaddr0 = _mempool_get_addr();
		mctf_bufaddr0 = VI_256_ALIGN(mctf_tmp_bufaddr0);
		bufsize = ispblk_dma_config(ictx, ISP_BLK_ID_DMA_CTL_TNR_ST_MO, mctf_bufaddr0);
		_mempool_pop(bufsize + (uint32_t)(mctf_bufaddr0 - mctf_tmp_bufaddr0));
		isp_bufpool[raw].tdnr_mo[0] = mctf_bufaddr0;

		mctf_tmp_bufaddr1 = _mempool_get_addr();
		mctf_bufaddr1 = VI_256_ALIGN(mctf_tmp_bufaddr1);
		bufsize = ispblk_dma_config(ictx, ISP_BLK_ID_DMA_CTL_TNR_LD_MO, mctf_bufaddr1);
		_mempool_pop(bufsize + (uint32_t)(mctf_bufaddr1 - mctf_tmp_bufaddr1));
		isp_bufpool[raw].tdnr_mo[1] = mctf_bufaddr1;

		//	MV
		mctf_tmp_bufaddr0 = _mempool_get_addr();
		mctf_bufaddr0 = VI_256_ALIGN(mctf_tmp_bufaddr0);
		bufsize = ispblk_dma_config(ictx, ISP_BLK_ID_DMA_CTL_TNR_ST_MV, mctf_bufaddr0);
		_mempool_pop(bufsize + (uint32_t)(mctf_bufaddr0 - mctf_tmp_bufaddr0));
		isp_bufpool[raw].tdnr_mv[0] = mctf_bufaddr0;

		mctf_tmp_bufaddr1 = _mempool_get_addr();
		mctf_bufaddr1 = VI_256_ALIGN(mctf_tmp_bufaddr1);
		bufsize = ispblk_dma_config(ictx, ISP_BLK_ID_DMA_CTL_TNR_LD_MV, mctf_bufaddr1);
		_mempool_pop(bufsize + (uint32_t)(mctf_bufaddr1 - mctf_tmp_bufaddr1));
		isp_bufpool[raw].tdnr_mv[1] = mctf_bufaddr1;

		//	FCB
		mctf_tmp_bufaddr0 = _mempool_get_addr();
		mctf_bufaddr0 = VI_256_ALIGN(mctf_tmp_bufaddr0);
		bufsize = ispblk_dma_config(ictx, ISP_BLK_ID_DMA_CTL_TNR_ST_FCB, mctf_bufaddr0);
		_mempool_pop(bufsize + (uint32_t)(mctf_bufaddr0 - mctf_tmp_bufaddr0));
		isp_bufpool[raw].tdnr_fcb[0] = mctf_bufaddr0;

		mctf_tmp_bufaddr1 = _mempool_get_addr();
		mctf_bufaddr1 = VI_256_ALIGN(mctf_tmp_bufaddr1);
		bufsize = ispblk_dma_config(ictx, ISP_BLK_ID_DMA_CTL_TNR_LD_FCB, mctf_bufaddr1);
		_mempool_pop(bufsize + (uint32_t)(mctf_bufaddr1 - mctf_tmp_bufaddr1));
		isp_bufpool[raw].tdnr_fcb[1] = mctf_bufaddr1;

		//	Y
		mctf_tmp_bufaddr0 = _mempool_get_addr();
		mctf_bufaddr0 = VI_256_ALIGN(mctf_tmp_bufaddr0);
		bufsize = ispblk_dma_config(ictx, ISP_BLK_ID_DMA_CTL_TNR_ST_Y, mctf_bufaddr0);
		_mempool_pop(bufsize + (uint32_t)(mctf_bufaddr0 - mctf_tmp_bufaddr0));
		isp_bufpool[raw].tdnr_y[0] = mctf_bufaddr0;

		mctf_tmp_bufaddr1 = _mempool_get_addr();
		mctf_bufaddr1 = VI_256_ALIGN(mctf_tmp_bufaddr1);
		bufsize = ispblk_dma_config(ictx, ISP_BLK_ID_DMA_CTL_TNR_LD_Y, mctf_bufaddr1);
		_mempool_pop(bufsize + (uint32_t)(mctf_bufaddr1 - mctf_tmp_bufaddr1));
		isp_bufpool[raw].tdnr_y[1] = mctf_bufaddr1;

		//	UV
		mctf_tmp_bufaddr0 = _mempool_get_addr();
		mctf_bufaddr0 = VI_256_ALIGN(mctf_tmp_bufaddr0);
		bufsize = ispblk_dma_config(ictx, ISP_BLK_ID_DMA_CTL_TNR_ST_C, mctf_bufaddr0);
		_mempool_pop(bufsize + (uint32_t)(mctf_bufaddr0 - mctf_tmp_bufaddr0));
		isp_bufpool[raw].tdnr_uv[0] = mctf_bufaddr0;

		mctf_tmp_bufaddr1 = _mempool_get_addr();
		mctf_bufaddr1 = VI_256_ALIGN(mctf_tmp_bufaddr1);
		bufsize = ispblk_dma_config(ictx, ISP_BLK_ID_DMA_CTL_TNR_LD_C, mctf_bufaddr1);
		_mempool_pop(bufsize + (uint32_t)(mctf_bufaddr1 - mctf_tmp_bufaddr1));
		isp_bufpool[raw].tdnr_uv[1] = mctf_bufaddr1;

		DMA_SETUP_2(ISP_BLK_ID_DMA_CTL_TNR_ST_MSP, raw);

		// if (ictx->is_3dnr_on) {
			// DMA_SETUP_2(ISP_BLK_ID_DMA_CTL_TNR_ST_MO, raw);
			// isp_bufpool[raw].tdnr_mo[0] = bufaddr;
			// ispblk_dma_config(ictx, ISP_BLK_ID_DMA_CTL_TNR_LD_MO, bufaddr);

			// DMA_SETUP_2(ISP_BLK_ID_DMA_CTL_TNR_ST_MV, raw);
			// isp_bufpool[raw].tdnr_mv[0] = bufaddr;
			// ispblk_dma_config(ictx, ISP_BLK_ID_DMA_CTL_TNR_LD_MV, bufaddr);

			// DMA_SETUP_2(ISP_BLK_ID_DMA_CTL_TNR_ST_FCB, raw);
			// isp_bufpool[raw].tdnr_fcb[0] = bufaddr;
			// ispblk_dma_config(ictx, ISP_BLK_ID_DMA_CTL_TNR_LD_FCB, bufaddr);

			// DMA_SETUP_2(ISP_BLK_ID_DMA_CTL_TNR_ST_MSP, raw);

			// if (ictx->is_fbc_on) {
			// 	u64 bufaddr_tmp = 0;

			// 	bufaddr_tmp = _mempool_get_addr();
			// 	//ring buffer constraint. reg_base is 256 byte-align
			// 	bufaddr = VI_256_ALIGN(bufaddr_tmp);
			// 	ispblk_dma_config(ictx, ISP_BLK_ID_DMA_CTL_TNR_LD_C, bufaddr);
			// 	bufsize = ispblk_dma_buf_get_size(ictx, ISP_BLK_ID_DMA_CTL_TNR_LD_C, raw);
			// 	_mempool_pop(bufsize + (u32)(bufaddr - bufaddr_tmp));

			// 	ispblk_dma_config(ictx, ISP_BLK_ID_DMA_CTL_TNR_ST_C, bufaddr);
			// 	isp_bufpool[raw].tdnr[0] = bufaddr;

			// 	bufaddr_tmp = _mempool_get_addr();
			// 	//ring buffer constraint. reg_base is 256 byte-align
			// 	bufaddr = VI_256_ALIGN(bufaddr_tmp);
			// 	ispblk_dma_config(ictx, ISP_BLK_ID_DMA_CTL_TNR_LD_Y, bufaddr);
			// 	bufsize = ispblk_dma_buf_get_size(ictx, ISP_BLK_ID_DMA_CTL_TNR_LD_Y, raw);
			// 	_mempool_pop(bufsize + (u32)(bufaddr - bufaddr_tmp));

			// 	ispblk_dma_config(ictx, ISP_BLK_ID_DMA_CTL_TNR_ST_Y, bufaddr);
			// 	isp_bufpool[raw].tdnr[1] = bufaddr;
			// } else {
			// 	DMA_SETUP_2(ISP_BLK_ID_DMA_CTL_TNR_LD_C, raw);
			// 	ispblk_dma_setaddr(ictx, ISP_BLK_ID_DMA_CTL_TNR_ST_C, bufaddr);
			// 	isp_bufpool[raw].tdnr[0] = bufaddr;

			// 	DMA_SETUP_2(ISP_BLK_ID_DMA_CTL_TNR_LD_Y, raw);
			// 	ispblk_dma_setaddr(ictx, ISP_BLK_ID_DMA_CTL_TNR_ST_Y, bufaddr);
			// 	isp_bufpool[raw].tdnr[1] = bufaddr;
			// }
		// }

		//cnr_v2
		uint64_t cnr_bufaddr0 = 0, cnr_bufaddr1 = 0;
		uint64_t cnr_tmp_bufaddr0 = 0, cnr_tmp_bufaddr1 = 0;

		// Y
		cnr_tmp_bufaddr0 = _mempool_get_addr();
		cnr_bufaddr0 = VI_256_ALIGN(cnr_tmp_bufaddr0);
		bufsize = ispblk_dma_config(ictx, ISP_BLK_ID_DMA_CTL_CNR_Y_W, cnr_bufaddr0);
		_mempool_pop(bufsize + (uint32_t)(cnr_bufaddr0 - cnr_tmp_bufaddr0));
		isp_bufpool[raw].cnr_y[0] = cnr_bufaddr0;

		cnr_tmp_bufaddr1 = _mempool_get_addr();
		cnr_bufaddr1 = VI_256_ALIGN(cnr_tmp_bufaddr1);
		bufsize = ispblk_dma_config(ictx, ISP_BLK_ID_DMA_CTL_CNR_Y_R, cnr_bufaddr1);
		_mempool_pop(bufsize + (uint32_t)(cnr_bufaddr1 - cnr_tmp_bufaddr1));
		isp_bufpool[raw].cnr_y[1] = cnr_bufaddr1;

		// UV
		cnr_tmp_bufaddr0 = _mempool_get_addr();
		cnr_bufaddr0 = VI_256_ALIGN(cnr_tmp_bufaddr0);
		bufsize = ispblk_dma_config(ictx, ISP_BLK_ID_DMA_CTL_CNR_C_W, cnr_bufaddr0);
		_mempool_pop(bufsize + (uint32_t)(cnr_bufaddr0 - cnr_tmp_bufaddr0));
		isp_bufpool[raw].cnr_uv[0] = cnr_bufaddr0;

		cnr_tmp_bufaddr1 = _mempool_get_addr();
		cnr_bufaddr1 = VI_256_ALIGN(cnr_tmp_bufaddr1);
		bufsize = ispblk_dma_config(ictx, ISP_BLK_ID_DMA_CTL_CNR_C_R, cnr_bufaddr1);
		_mempool_pop(bufsize + (uint32_t)(cnr_bufaddr1 - cnr_tmp_bufaddr1));
		isp_bufpool[raw].cnr_uv[1] = cnr_bufaddr1;
		// DMA_SETUP_2(ISP_BLK_ID_DMA_CTL_CNR_C_W, raw);
		// ispblk_dma_setaddr(ictx, ISP_BLK_ID_DMA_CTL_CNR_C_R, bufaddr);
		// isp_bufpool[raw].cnr[0] = bufaddr;
		// DMA_SETUP_2(ISP_BLK_ID_DMA_CTL_CNR_Y_W, raw);
		// ispblk_dma_setaddr(ictx, ISP_BLK_ID_DMA_CTL_CNR_Y_R, bufaddr);
		// isp_bufpool[raw].cnr[1] = bufaddr;
	}

	if (cfg_dma) {
		if (ictx->isp_pipe_cfg[ISP_PRERAW0].is_offline_scaler ||
			(ictx->is_multi_sensor && ictx->isp_pipe_cfg[ISP_PRERAW1].is_offline_scaler)) {
			//SW workaround. Need to set y/uv dma_disable = 1 before csibdg enable
			if (_is_fe_post_offline(ictx) && !ictx->is_slice_buf_on) {
				ispblk_dma_enable(ictx, ISP_BLK_ID_DMA_CTL_YUV_CROP_Y, 1, 1);
				ispblk_dma_enable(ictx, ISP_BLK_ID_DMA_CTL_YUV_CROP_C, 1, 1);
			} else {
				ispblk_dma_enable(ictx, ISP_BLK_ID_DMA_CTL_YUV_CROP_Y, 1, 0);
				ispblk_dma_enable(ictx, ISP_BLK_ID_DMA_CTL_YUV_CROP_C, 1, 0);
			}
		} else {
			ispblk_dma_enable(ictx, ISP_BLK_ID_DMA_CTL_YUV_CROP_Y, 0, 0);
			ispblk_dma_enable(ictx, ISP_BLK_ID_DMA_CTL_YUV_CROP_C, 0, 0);
		}
	}

EXIT:
	_isp_yuvtop_dma_dump(ictx, raw_max);
}

void _vi_dma_setup(struct isp_ctx *ictx, enum cvi_isp_raw raw_max)
{
	enum cvi_isp_raw raw_num = ISP_PRERAW0;

	for (raw_num = ISP_PRERAW0; raw_num < raw_max; raw_num++)
		_isp_preraw_fe_dma_setup(ictx, raw_num);

	_isp_rawtop_dma_setup(ictx, raw_max);
	_isp_rgbtop_dma_setup(ictx, raw_max);
	_isp_yuvtop_dma_setup(ictx, raw_max);
}

void _vi_dma_set_sw_mode(struct isp_ctx *ctx)
{
	ispblk_dma_set_sw_mode(ctx, ISP_BLK_ID_DMA_CTL_PRE_RAW_VI_SEL_LE, false);
	ispblk_dma_set_sw_mode(ctx, ISP_BLK_ID_DMA_CTL_PRE_RAW_VI_SEL_SE, false);
	ispblk_dma_set_sw_mode(ctx, ISP_BLK_ID_DMA_CTL_CSI0_BDG0, false);
	ispblk_dma_set_sw_mode(ctx, ISP_BLK_ID_DMA_CTL_CSI0_BDG1, false);
	ispblk_dma_set_sw_mode(ctx, ISP_BLK_ID_DMA_CTL_CSI0_BDG2, false);
	ispblk_dma_set_sw_mode(ctx, ISP_BLK_ID_DMA_CTL_CSI0_BDG3, false);
	ispblk_dma_set_sw_mode(ctx, ISP_BLK_ID_DMA_CTL_CSI1_BDG0, false);
	ispblk_dma_set_sw_mode(ctx, ISP_BLK_ID_DMA_CTL_CSI1_BDG1, false);
	ispblk_dma_set_sw_mode(ctx, ISP_BLK_ID_DMA_CTL_CSI2_BDG0, false);
	ispblk_dma_set_sw_mode(ctx, ISP_BLK_ID_DMA_CTL_AF_W, true);
	ispblk_dma_set_sw_mode(ctx, ISP_BLK_ID_DMA_CTL_FE0_LSC_LE, true);
	ispblk_dma_set_sw_mode(ctx, ISP_BLK_ID_DMA_CTL_FE0_LSC_SE, true);
	ispblk_dma_set_sw_mode(ctx, ISP_BLK_ID_DMA_CTL_FE1_LSC_LE, true);
	ispblk_dma_set_sw_mode(ctx, ISP_BLK_ID_DMA_CTL_FE1_LSC_SE, true);
	ispblk_dma_set_sw_mode(ctx, ISP_BLK_ID_DMA_CTL_FE2_LSC_LE, true);
	ispblk_dma_set_sw_mode(ctx, ISP_BLK_ID_DMA_CTL_FE0_AE_HIST_LE, true);
	ispblk_dma_set_sw_mode(ctx, ISP_BLK_ID_DMA_CTL_FE0_AE_HIST_SE, true);
	ispblk_dma_set_sw_mode(ctx, ISP_BLK_ID_DMA_CTL_FE1_AE_HIST_LE, true);
	ispblk_dma_set_sw_mode(ctx, ISP_BLK_ID_DMA_CTL_FE1_AE_HIST_SE, true);
	ispblk_dma_set_sw_mode(ctx, ISP_BLK_ID_DMA_CTL_FE2_AE_HIST_LE, true);
	ispblk_dma_set_sw_mode(ctx, ISP_BLK_ID_DMA_CTL_LSCR_HIST, true);
	ispblk_dma_set_sw_mode(ctx, ISP_BLK_ID_DMA_CTL_DRC_POLY_W, true);
	ispblk_dma_set_sw_mode(ctx, ISP_BLK_ID_DMA_CTL_DRC_POLY_R, true);
	ispblk_dma_set_sw_mode(ctx, ISP_BLK_ID_DMA_CTL_DRC_HIST, true);
	ispblk_dma_set_sw_mode(ctx, ISP_BLK_ID_DMA_CTL_LDCI_W, false);
	ispblk_dma_set_sw_mode(ctx, ISP_BLK_ID_DMA_CTL_LDCI_R, false);
	ispblk_dma_set_sw_mode(ctx, ISP_BLK_ID_DMA_CTL_LDCI_HIST, true);

	//mctf
	ispblk_dma_set_sw_mode(ctx, ISP_BLK_ID_DMA_CTL_TNR_ST_MO, false);
	ispblk_dma_set_sw_mode(ctx, ISP_BLK_ID_DMA_CTL_TNR_ST_MV, false);
	ispblk_dma_set_sw_mode(ctx, ISP_BLK_ID_DMA_CTL_TNR_LD_MO, false);
	ispblk_dma_set_sw_mode(ctx, ISP_BLK_ID_DMA_CTL_TNR_LD_MV, false);
	ispblk_dma_set_sw_mode(ctx, ISP_BLK_ID_DMA_CTL_TNR_ST_Y, false);
	ispblk_dma_set_sw_mode(ctx, ISP_BLK_ID_DMA_CTL_TNR_ST_C, false);
	if (ctx->is_fbc_on) {
		ispblk_dma_set_sw_mode(ctx, ISP_BLK_ID_DMA_CTL_TNR_LD_Y, true);
		ispblk_dma_set_sw_mode(ctx, ISP_BLK_ID_DMA_CTL_TNR_LD_C, true);
	} else {
		ispblk_dma_set_sw_mode(ctx, ISP_BLK_ID_DMA_CTL_TNR_LD_Y, false);
		ispblk_dma_set_sw_mode(ctx, ISP_BLK_ID_DMA_CTL_TNR_LD_C, false);
	}
	ispblk_dma_set_sw_mode(ctx, ISP_BLK_ID_DMA_CTL_TNR_ST_FCB, false);
	ispblk_dma_set_sw_mode(ctx, ISP_BLK_ID_DMA_CTL_TNR_LD_FCB, false);
	ispblk_dma_set_sw_mode(ctx, ISP_BLK_ID_DMA_CTL_TNR_ST_MSP, false);

	//cnr v2
	ispblk_dma_set_sw_mode(ctx, ISP_BLK_ID_DMA_CTL_CNR_Y_W, true);
	ispblk_dma_set_sw_mode(ctx, ISP_BLK_ID_DMA_CTL_CNR_Y_R, true);
	ispblk_dma_set_sw_mode(ctx, ISP_BLK_ID_DMA_CTL_CNR_C_W, true);
	ispblk_dma_set_sw_mode(ctx, ISP_BLK_ID_DMA_CTL_CNR_C_R, true);

	ispblk_dma_set_sw_mode(ctx, ISP_BLK_ID_DMA_CTL_YUV_CROP_Y, false);
	ispblk_dma_set_sw_mode(ctx, ISP_BLK_ID_DMA_CTL_YUV_CROP_C, false);
}

void _vi_yuv_get_dma_size(struct isp_ctx *ctx, const enum cvi_isp_raw raw_num)
{
	// uint32_t bufsize_yuyv = 0;
	// uint8_t  i = 0;

	// u8 total_chn = (raw_num == ISP_PRERAW0) ?
	// 		ctx->rawb_chnstr_num :
	// 		ctx->total_chn_num;
	// u8 chn_str = (raw_num == ISP_PRERAW0) ? 0 : ctx->rawb_chnstr_num;

	// for (; chn_str < total_chn; chn_str++) {
	// 	enum ISP_BLK_ID_T dma;

	// 	if (raw_num == ISP_PRERAW2) {
	// 		dma = (chn_str == ctx->rawb_chnstr_num) ? ISP_BLK_ID_DMA_CTL18 : ISP_BLK_ID_DMA_CTL19;
	// 	} else {
	// 		dma = (chn_str == ctx->rawb_chnstr_num) ? ISP_BLK_ID_DMA_CTL12 : ISP_BLK_ID_DMA_CTL13;
	// 	}

	// 	for (i = 0; i < OFFLINE_YUV_BUF_NUM; i++) {
	// 		bufsize_yuyv = ispblk_dma_yuv_bypass_config(ctx, dma, 0, raw_num);
	// 		_mempool_pop(bufsize_yuyv);
	// 	}
	// }
}

void _vi_pre_fe_get_dma_size(struct isp_ctx *ictx, enum cvi_isp_raw  raw_num)
{
	uint32_t bufsize = 0;
	uint8_t  i = 0;
	u32 raw_le, raw_se;

	if (ictx->isp_pipe_cfg[raw_num].is_yuv_bypass_path) { //YUV sensor
		if (!ictx->isp_pipe_cfg[raw_num].is_offline_scaler) //Online mode to scaler
			_vi_yuv_get_dma_size(ictx, raw_num);

		goto EXIT;
	}

	raw_le = csibdg_dma_find_hwid(raw_num, true);
	raw_se = csibdg_dma_find_hwid(raw_num, false);

	if (_is_fe_post_offline(ictx)) { //fe->dram->post
		for (i = 0; i < OFFLINE_RAW_BUF_NUM; i++) {
			bufsize = ispblk_dma_buf_get_size(ictx, raw_le, raw_num);
			_mempool_pop(bufsize);
		}

		if (ictx->isp_pipe_cfg[raw_num].is_hdr_on) {
			for (i = 0; i < OFFLINE_RAW_BUF_NUM; i++) {
				bufsize = ispblk_dma_buf_get_size(ictx, raw_se, raw_num);
				_mempool_pop(bufsize);
			}
		}
	}
EXIT:
	return;
}

void _vi_rawtop_get_dma_size(struct isp_ctx *ictx, enum cvi_isp_raw raw_max)
{
	uint32_t bufsize = 0;

	enum cvi_isp_raw raw = ISP_PRERAW0;

	u8 cfg_dma = false;

	//RGB path
	if (!ictx->isp_pipe_cfg[ISP_PRERAW0].is_yuv_bypass_path) {
		cfg_dma = true;
	} else if (ictx->is_multi_sensor && !ictx->isp_pipe_cfg[ISP_PRERAW1].is_yuv_bypass_path) {
		cfg_dma = true;
	}

	if (cfg_dma == false) //YUV sensor only
		goto EXIT;

	for (raw = ISP_PRERAW0; raw < raw_max; raw++) {

		if (ictx->isp_pipe_cfg[raw].is_yuv_bypass_path) //YUV sensor
			continue;

		//af
		bufsize = ispblk_dma_buf_get_size(ictx, ISP_BLK_ID_DMA_CTL_AF_W, raw);
		_mempool_pop(bufsize);

		// lscr
		bufsize = ispblk_dma_buf_get_size(ictx, ISP_BLK_ID_DMA_CTL_LSCR_HIST, raw);
		_mempool_pop(bufsize);

		// drc poly_rw & hist
		bufsize = ispblk_dma_buf_get_size(ictx, ISP_BLK_ID_DMA_CTL_DRC_POLY_W, raw);
		_mempool_pop(bufsize);
		_mempool_pop(bufsize);
		bufsize = ispblk_dma_buf_get_size(ictx, ISP_BLK_ID_DMA_CTL_DRC_HIST, raw);
		_mempool_pop(bufsize);
	}
EXIT:
	return;
}

void _vi_rgbtop_get_dma_size(struct isp_ctx *ictx, enum cvi_isp_raw raw_max)
{
	//no dma buffer
}

void _vi_yuvtop_get_dma_size(struct isp_ctx *ictx, enum cvi_isp_raw raw_max)
{
	uint32_t bufsize = 0;
	uint64_t bufaddr = 0;
	uint64_t tmp_bufaddr = 0;

	enum cvi_isp_raw raw = ISP_PRERAW0;

	u8 cfg_dma = false;

	//RGB path
	if (!ictx->isp_pipe_cfg[ISP_PRERAW0].is_yuv_bypass_path) {
		cfg_dma = true;
	} else if (ictx->is_multi_sensor && !ictx->isp_pipe_cfg[ISP_PRERAW1].is_yuv_bypass_path) {
		cfg_dma = true;
	}

	if (cfg_dma == false) //YUV sensor only
		goto EXIT;

	for (raw = ISP_PRERAW0; raw < raw_max; raw++) {

		if (ictx->isp_pipe_cfg[raw].is_yuv_bypass_path) //YUV sensor
			continue;

		// ldci
		bufaddr = VI_256_ALIGN(tmp_bufaddr);
		bufsize = ispblk_dma_buf_get_size(ictx, ISP_BLK_ID_DMA_CTL_LDCI_W, raw);
		_mempool_pop(bufsize + (uint32_t)(bufaddr - tmp_bufaddr));

		bufsize = ispblk_dma_buf_get_size(ictx, ISP_BLK_ID_DMA_CTL_LDCI_HIST, raw);
		_mempool_pop(bufsize);

		vi_pr(VI_INFO, "ldci bufsize: total(%d), used(%d), wasted_for_alignment(%d)\n",
			bufsize + (uint32_t)(bufaddr - tmp_bufaddr),
			bufsize,
			(uint32_t)(bufaddr - tmp_bufaddr));

		// mctf
		if (ictx->is_3dnr_on) {
			uint64_t bufaddr = 0;
			uint64_t tmp_bufaddr = 0;

			bufsize = ispblk_dma_buf_get_size(ictx, ISP_BLK_ID_DMA_CTL_TNR_LD_MO, raw);
			_mempool_pop(bufsize);
			_mempool_pop(bufsize);

			bufsize = ispblk_dma_buf_get_size(ictx, ISP_BLK_ID_DMA_CTL_TNR_LD_MV, raw);
			_mempool_pop(bufsize);
			_mempool_pop(bufsize);

			bufsize = ispblk_dma_buf_get_size(ictx, ISP_BLK_ID_DMA_CTL_TNR_LD_FCB, raw);
			_mempool_pop(bufsize);
			_mempool_pop(bufsize);

			bufsize = ispblk_dma_buf_get_size(ictx, ISP_BLK_ID_DMA_CTL_TNR_ST_MSP, raw);
			_mempool_pop(bufsize);

			if (ictx->is_fbc_on) {
				// TNR UV
				tmp_bufaddr = _mempool_get_addr();
				//ring buffer constraint. reg_base is 256 byte-align
				bufaddr = VI_4K_ALIGN(tmp_bufaddr);

				bufsize = ispblk_dma_buf_get_size(ictx, ISP_BLK_ID_DMA_CTL_TNR_LD_C, raw);
				_mempool_pop(bufsize + (uint32_t)(bufaddr - tmp_bufaddr));

				// TNR Y
				tmp_bufaddr = _mempool_get_addr();
				//ring buffer constraint. reg_base is 256 byte-align
				bufaddr = VI_4K_ALIGN(tmp_bufaddr);
				bufsize = ispblk_dma_buf_get_size(ictx, ISP_BLK_ID_DMA_CTL_TNR_LD_Y, raw);
				_mempool_pop(bufsize + (uint32_t)(bufaddr - tmp_bufaddr));
			} else {
				// TNR UV
				bufsize = ispblk_dma_buf_get_size(ictx, ISP_BLK_ID_DMA_CTL_TNR_LD_C, raw);
				_mempool_pop(bufsize);

				// TNR Y
				bufsize = ispblk_dma_buf_get_size(ictx, ISP_BLK_ID_DMA_CTL_TNR_LD_Y, raw);
				_mempool_pop(bufsize);
			}

			//cnr v2
			bufsize = ispblk_dma_buf_get_size(ictx, ISP_BLK_ID_DMA_CTL_CNR_Y_W, raw);
			_mempool_pop(bufsize);
			_mempool_pop(bufsize);
			bufsize = ispblk_dma_buf_get_size(ictx, ISP_BLK_ID_DMA_CTL_CNR_C_W, raw);
			_mempool_pop(bufsize);
			_mempool_pop(bufsize);
		}
	}
EXIT:
	return;
}

void _vi_get_dma_buf_size(struct isp_ctx *ictx, enum cvi_isp_raw raw_max)
{
	enum cvi_isp_raw raw_num = ISP_PRERAW0;

	for (raw_num = ISP_PRERAW0; raw_num < raw_max; raw_num++)
		_vi_pre_fe_get_dma_size(ictx, raw_num);

	_vi_rawtop_get_dma_size(ictx, raw_max);
	_vi_rgbtop_get_dma_size(ictx, raw_max);
	_vi_yuvtop_get_dma_size(ictx, raw_max);
}

static void _vi_preraw_fe_init(struct cvi_vi_dev *vdev)
{
	struct isp_ctx *ctx = &vdev->ctx;
	u8 cfg_fe = false;

	//TODO support yuv sensor
	if (!ctx->isp_pipe_cfg[ISP_PRERAW0].is_yuv_bypass_path) {
		cfg_fe = true;
	} else if (ctx->is_multi_sensor && !ctx->isp_pipe_cfg[ISP_PRERAW1].is_yuv_bypass_path) {
		cfg_fe = true;
	}

	if (cfg_fe) { //RGB sensor
		// preraw_vi_sel
		ispblk_preraw_vi_sel_config(ctx);

		ispblk_slice_buf_config(&vdev->ctx, ISP_PRERAW0, vdev->ctx.is_slice_buf_on);
	}
}

static void _isp_rawtop_init(struct cvi_vi_dev *vdev)
{
	struct isp_ctx *ictx = &vdev->ctx;
	// uint32_t bps[40] = {((0 << 12) | 0), ((0 << 12) | 20), ((0 << 12) | 40), ((0 << 12) | 60)};

	// raw_top
	ispblk_rawtop_config(ictx, ISP_PRERAW0);

	ispblk_bnr_lite_config(ictx, true);

	ispblk_bnr_config(ictx, true);

	//raw replay
	// ispblk_lsc_config(ictx, ISP_PRERAW0, true, true);
	// ispblk_aehist_config(ictx, ISP_PRERAW0, true);
	// if (ictx->is_hdr_on) {
	// 	ispblk_lsc_config(ictx, ISP_PRERAW0, false, true);
	// 	ispblk_aehist_config(ictx, ISP_PRERAW0, true);
	// }

	ispblk_af_config(ictx, true);

	//ispblk_blc_set_offset(ctx, ISP_BLK_ID_BLC_DG_WB1, 511, 511, 511, 511);
	ispblk_blc_set_gain(ictx, 0x40f, 0x419, 0x419, 0x405);
	//ispblk_blc_set_2ndoffset(ctx, ISP_BLK_ID_BLC_DG_WB1, 511, 511, 511, 511);
	ispblk_blc_enable(ictx, true);

	ispblk_wbg_config(ictx, 0x400, 0x400, 0x400);
	ispblk_wbg_enable(ictx, true);

	ispblk_mapcurve_config(ictx, true);

	// ispblk_dpc_set_static(ictx, 0, bps, 4);
	// ispblk_dpc_config(ictx, false, 0);

	if (ictx->is_hdr_on) {
		ispblk_fusion_config(ictx, true, ISP_FS_OUT_FS);
	} else {
		ispblk_fusion_config(ictx, false, ISP_FS_OUT_LONG);
	}

	ispblk_drc_config(ictx, true);

	ispblk_cfa_config(ictx);
}

static void _isp_rgbtop_init(struct cvi_vi_dev *vdev)
{
	struct isp_ctx *ictx = &vdev->ctx;

	ispblk_rgbtop_config(ictx, ISP_PRERAW0);

	ispblk_edge_ext_config(ictx, true);

	ispblk_pfr_config(ictx, true);

	ispblk_ccm_config(ictx, false, &ccm_hw_cfg);

	ispblk_gamma_enable(ictx, true);

	//ispblk_clut_config(ictx, false, c_lut_r_lut, c_lut_g_lut, c_lut_b_lut, false);
	ispblk_rgbdither_config(ictx, false, false, false, false);
	ispblk_csc_config(ictx);
}

static void _isp_yuvtop_init(struct cvi_vi_dev *vdev)
{
	struct isp_ctx *ictx = &vdev->ctx;

	ispblk_yuvtop_config(ictx, ISP_PRERAW0);

	ispblk_yuvdither_config(ictx, 0, false, true, true, true);
	ispblk_yuvdither_config(ictx, 1, false, true, true, true);

	ispblk_pre_ee_front_config(ictx, false);
	ispblk_pre_ee_back_config(ictx, false);

	ispblk_ldci_config(ictx, false, 0);

	ispblk_tnr_config(ictx, ictx->is_3dnr_on);
	if (ictx->is_3dnr_on && ictx->is_fbc_on) {
		ispblk_fbce_config(ictx, true);
		ispblk_fbcd_config(ictx, true);
		ispblk_fbc_ring_buf_config(ictx, true);
	} else {
		ispblk_fbce_config(ictx, false);
		ispblk_fbcd_config(ictx, false);
		ispblk_fbc_ring_buf_config(ictx, false);
	}

	ispblk_cnr_config(ictx, false);
	ispblk_post_ee_config(ictx, true);
#ifdef COVER_WITH_BLACK
	memset(ycur_data, 0, sizeof(ycur_data));
	ispblk_ycur_config(ictx, false, ictx->gamma_tbl_idx, ycur_data, false);
	ispblk_ycur_enable(ictx, true, 0);
#else
	ispblk_ycur_config(ictx, false, ictx->gamma_tbl_idx, ycur_data, false);
	ispblk_ycur_enable(ictx, false, 0);
#endif

	ispblk_ca_config(ictx, false, 1);
	ispblk_ca_lite_config(ictx, false);
}

static inline void _vi_clear_mmap_fbc_ring_base(struct cvi_vi_dev *vdev, const enum cvi_isp_raw raw_num)
{
	struct isp_ctx *ctx = &vdev->ctx;

	//Clear mmap previous ring base to start addr after first frame done.
	if (ctx->is_3dnr_on && (ctx->isp_pipe_cfg[raw_num].first_frm_cnt == 1)) {
		if (ctx->is_fbc_on) {
			ispblk_fbc_chg_to_sw_mode(&vdev->ctx, raw_num);
			ispblk_fbc_clear_fbcd_ring_base(&vdev->ctx, raw_num);
		}
	}
}

void _vi_postraw_ctrl_setup(struct cvi_vi_dev *vdev)
{
	struct isp_ctx *ctx = &vdev->ctx;
	u8 cfg_post = false;

	if (!ctx->isp_pipe_cfg[ISP_PRERAW0].is_yuv_bypass_path) {
		cfg_post = true;
	} else if (ctx->is_multi_sensor && !ctx->isp_pipe_cfg[ISP_PRERAW1].is_yuv_bypass_path) {
		cfg_post = true;
	}

	if (cfg_post) { //RGB sensor
		_isp_rawtop_init(vdev);
		_isp_rgbtop_init(vdev);
		_isp_yuvtop_init(vdev);
	}

	ispblk_isptop_config(ctx);
}

void _vi_pre_fe_ctrl_setup(enum cvi_isp_raw raw_num, struct cvi_vi_dev *vdev)
{
	struct isp_ctx *ictx = &vdev->ctx;

	if (ictx->isp_pipe_cfg[raw_num].is_yuv_bypass_path) {//YUV sensor

		// ispblk_csibdg_yuv_bypass_config(ictx, raw_num);

		// if (ictx->isp_pipe_cfg[raw_num].is_offline_scaler) { //vi vpss offline mode
		// 	u8 total_chn = (raw_num == ISP_PRERAW0) ?
		// 			ictx->rawb_chnstr_num :
		// 			ictx->total_chn_num;
		// 	u8 chn_str = (raw_num == ISP_PRERAW0) ? 0 : ictx->rawb_chnstr_num;

		// 	for (; chn_str < total_chn; chn_str++)
		// 		_isp_yuv_bypass_buf_enq(vdev, raw_num, chn_str);
		// }
	} else { //RGB sensor
		ispblk_preraw_fe_config(ictx, raw_num);
		ispblk_csibdg_config(ictx, raw_num);
		// ispblk_csibdg_crop_update(ictx, raw_num, true);

		ispblk_lsc_config(ictx, raw_num, true, true);
		if (ictx->isp_pipe_cfg[raw_num].is_hdr_on) {
			ispblk_lsc_config(ictx, raw_num, false, true);
		}

		// ispblk_aehist_config(ictx, raw_num, true);
	}
}

void _vi_ctrl_init(enum cvi_isp_raw raw_num, struct cvi_vi_dev *vdev)
{
	struct isp_ctx *ictx = &vdev->ctx;

	if (ictx->is_ctrl_inited)
		return;

	if (ictx->isp_pipe_cfg[raw_num].is_patgen_en) {
		ictx->isp_pipe_cfg[raw_num].crop.w = vdev->usr_crop.width;
		ictx->isp_pipe_cfg[raw_num].crop.h = vdev->usr_crop.height;
		ictx->isp_pipe_cfg[raw_num].crop.x = vdev->usr_crop.left;
		ictx->isp_pipe_cfg[raw_num].crop.y = vdev->usr_crop.top;
		ictx->isp_pipe_cfg[raw_num].crop_se.w = vdev->usr_crop.width;
		ictx->isp_pipe_cfg[raw_num].crop_se.h = vdev->usr_crop.height;
		ictx->isp_pipe_cfg[raw_num].crop_se.x = vdev->usr_crop.left;
		ictx->isp_pipe_cfg[raw_num].crop_se.y = vdev->usr_crop.top;

		ictx->isp_pipe_cfg[raw_num].csibdg_width	= vdev->usr_fmt.width;
		ictx->isp_pipe_cfg[raw_num].csibdg_height	= vdev->usr_fmt.height;
		ictx->isp_pipe_cfg[raw_num].max_width		= vdev->usr_fmt.width;
		ictx->isp_pipe_cfg[raw_num].max_height		= vdev->usr_fmt.height;

		ictx->rgb_color_mode[raw_num] = vdev->usr_fmt.code;

		vi_pr(VI_INFO, "patgen csibdg_w_h(%d:%d)\n",
			ictx->isp_pipe_cfg[raw_num].csibdg_width, ictx->isp_pipe_cfg[raw_num].csibdg_height);
	} else {
		ictx->isp_pipe_cfg[raw_num].crop.w = vdev->usr_crop.width;
		ictx->isp_pipe_cfg[raw_num].crop.h = vdev->usr_crop.height;
		ictx->isp_pipe_cfg[raw_num].crop.x = vdev->usr_crop.left;
		ictx->isp_pipe_cfg[raw_num].crop.y = vdev->usr_crop.top;
		ictx->isp_pipe_cfg[raw_num].crop_se.w = vdev->usr_crop.width;
		ictx->isp_pipe_cfg[raw_num].crop_se.h = vdev->usr_crop.height;
		ictx->isp_pipe_cfg[raw_num].crop_se.x = vdev->usr_crop.left;
		ictx->isp_pipe_cfg[raw_num].crop_se.y = vdev->usr_crop.top;

		ictx->isp_pipe_cfg[raw_num].csibdg_width	= vdev->usr_fmt.width;
		ictx->isp_pipe_cfg[raw_num].csibdg_height	= vdev->usr_fmt.height;
		ictx->isp_pipe_cfg[raw_num].max_width		= vdev->usr_fmt.width;
		ictx->isp_pipe_cfg[raw_num].max_height		= vdev->usr_fmt.height;

		ictx->rgb_color_mode[raw_num] = vdev->usr_fmt.code;

		vi_pr(VI_INFO, "csi_bdg=%d:%d, post_crop=%d:%d:%d:%d\n",
				vdev->usr_fmt.width, vdev->usr_fmt.height,
				vdev->usr_crop.width, vdev->usr_crop.height,
				vdev->usr_crop.left, vdev->usr_crop.top);
	}

	ictx->isp_pipe_cfg[raw_num].post_img_w = ictx->isp_pipe_cfg[raw_num].crop.w;
	ictx->isp_pipe_cfg[raw_num].post_img_h = ictx->isp_pipe_cfg[raw_num].crop.h;

	/* use csibdg crop */
	ictx->crop_x = 0;
	ictx->crop_y = 0;
	ictx->crop_se_x = 0;
	ictx->crop_se_y = 0;

	if (!ictx->isp_pipe_cfg[raw_num].is_yuv_bypass_path) {
		//Postraw out size
		ictx->img_width = ictx->isp_pipe_cfg[ISP_PRERAW0].crop.w;
		ictx->img_height = ictx->isp_pipe_cfg[ISP_PRERAW0].crop.h;
	}

	if (raw_num == ISP_PRERAW0) {
		if (ictx->is_slice_buf_on)
			vi_calculate_slice_buf_setting(ictx, raw_num);

		isp_init(ictx);
	}

	if (raw_num == vdev->total_dev_num - 1)
		ictx->is_ctrl_inited = true;

}

#if 0
void _vi_scene_ctrl(struct cvi_vi_dev *vdev, enum cvi_isp_raw *raw_max)
{
	struct isp_ctx *ctx = &vdev->ctx;
	enum cvi_isp_raw raw_num = ISP_PRERAW0;

	if (ctx->is_ctrl_inited) {
		// vi_scene_ctrl had been inited before.
		*raw_max = gViCtx->total_dev_num;
		return;
	}
#ifndef FPGA_PORTING
	if (gViCtx->total_dev_num >= 2) { // multi sensor scenario
		*raw_max = gViCtx->total_dev_num;
		ctx->is_multi_sensor = true;

		if (*raw_max == ISP_PRERAW_MAX) { // three sensor
			ctx->is_offline_postraw = false;
			ctx->is_slice_buf_on = false;
			ctx->is_fbc_on = false;

			// if 422to420, chnAttr must be NV21
			if (ctx->isp_pipe_cfg[ISP_PRERAW0].is_offline_scaler) {
				if (ctx->isp_pipe_cfg[ISP_PRERAW2].is_yuv_bypass_path &&
				ctx->isp_pipe_cfg[ISP_PRERAW2].muxMode == VI_WORK_MODE_1Multiplex &&
				gViCtx->chnAttr[ctx->rawb_chnstr_num].enPixelFormat == PIXEL_FORMAT_NV21)
					ctx->isp_pipe_cfg[ISP_PRERAW2].is_422_to_420 = true;
			}
		} else { // two sensor
			if (!ctx->isp_pipe_cfg[ISP_PRERAW0].is_yuv_bypass_path &&
			    ctx->isp_pipe_cfg[ISP_PRERAW1].is_yuv_bypass_path) { //rgb + yuv
				if (ctx->isp_pipe_cfg[ISP_PRERAW0].is_offline_scaler) {
					// dual offline
					// rgb: fe->be->dram(sbm)->post->dram
					// yuv: fe->dram
					ctx->is_offline_postraw = true;
					ctx->is_slice_buf_on = true;

					// if 422to420, chnAttr must be NV21
					if (ctx->isp_pipe_cfg[ISP_PRERAW1].muxMode == VI_WORK_MODE_1Multiplex &&
					    gViCtx->chnAttr[ctx->rawb_chnstr_num].enPixelFormat == PIXEL_FORMAT_NV21)
						ctx->isp_pipe_cfg[ISP_PRERAW1].is_422_to_420 = true;
				} else {
					// dual online
					// rgb: fe->be->dram->post->sc
					// yuv: fe->dram->post->sc
					ctx->is_offline_postraw = true;
					ctx->is_slice_buf_on = false;

					//TODO
					// // rgb: fe->dram->post->sc
					// // yuv: fe->dram->post->sc
					// ctx->is_offline_postraw = false;
					// ctx->is_slice_buf_on = false;
				}
			} else { // rgb + rgb, fe->dram->post
				ctx->is_offline_postraw = false;
				ctx->is_slice_buf_on = false;
				ctx->is_fbc_on = false;
			}
		}
	} else { // single sensor scenario
		*raw_max = ISP_PRERAW1;
		ctx->is_multi_sensor = false;
		ctx->is_slice_buf_on = true;

		//Currently don't support single yuv sensor online to scaler or on-the-fly
		if (ctx->isp_pipe_cfg[ISP_PRERAW0].is_yuv_bypass_path) {
			ctx->isp_pipe_cfg[ISP_PRERAW0].is_offline_scaler = true;
			ctx->isp_pipe_cfg[ISP_PRERAW0].is_422_to_420 = false;
			ctx->is_slice_buf_on = false;
		}
	}
#endif
	if (!sbm_en)
		ctx->is_slice_buf_on = false;


	if (!ctx->isp_pipe_cfg[ISP_PRERAW0].is_yuv_bypass_path) //RGB sensor
		ctx->rawb_chnstr_num = 1;
	else if (ctx->isp_pipe_cfg[ISP_PRERAW0].is_yuv_bypass_path) //YUV sensor
		ctx->rawb_chnstr_num = ctx->isp_pipe_cfg[ISP_PRERAW0].muxMode + 1;

	if (ctx->is_multi_sensor) {
		for (raw_num = ISP_PRERAW1; raw_num < gViCtx->total_dev_num - 1; raw_num++) {
			if (!ctx->isp_pipe_cfg[raw_num].is_yuv_bypass_path) //RGB sensor
				ctx->rawb_chnstr_num++;
			else if (ctx->isp_pipe_cfg[raw_num].is_yuv_bypass_path) //YUV sensor
				ctx->rawb_chnstr_num += ctx->isp_pipe_cfg[raw_num].muxMode + 1;
		}
	}

	for (raw_num = ISP_PRERAW0; raw_num < *raw_max; raw_num++) {
		if (!ctx->isp_pipe_cfg[raw_num].is_yuv_bypass_path) //RGB sensor
			ctx->total_chn_num++;
		else //YUV sensor
			ctx->total_chn_num += ctx->isp_pipe_cfg[raw_num].muxMode + 1;
	}

	if (ctx->total_chn_num > 4) {
		vi_pr(VI_ERR, "[ERR] Total chn_num(%d) is wrong\n", ctx->total_chn_num);
		vi_pr(VI_ERR, "[ERR] raw_A,infMode(%d),muxMode(%d)\n",
				ctx->isp_pipe_cfg[ISP_PRERAW0].infMode, ctx->isp_pipe_cfg[ISP_PRERAW0].muxMode);
		if (ctx->is_multi_sensor) {
			vi_pr(VI_ERR, "[ERR] raw_B,infMode(%d),muxMode(%d)\n",
				ctx->isp_pipe_cfg[ISP_PRERAW1].infMode, ctx->isp_pipe_cfg[ISP_PRERAW1].muxMode);
		}
	}

	vi_pr(VI_INFO, "Total_chn_num=%d, rawb_chnstr_num=%d\n",
			ctx->total_chn_num, ctx->rawb_chnstr_num);
}
#endif

void _vi_scene_ctrl(struct cvi_vi_dev *vdev, enum cvi_isp_raw *raw_max)
{
	struct isp_ctx *ctx = &vdev->ctx;

	if (vi_fly_mode) {
		//fe->postraw
		ctx->is_offline_postraw = false;
	} else {
		//fe->dram->postraw
	ctx->is_offline_postraw = true;
	}

	if (vi_sensor_input){
		ctx->is_sensor_input = true;
	}
	else {
		for (int i = 0; i < vdev->total_dev_num; i++) {
				csi_patgen_en[i] = 1;
		}
	}

	vi_pr(VI_INFO, "raw_replay:%d patgen_en:%d\n", ctx->isp_pipe_cfg[0].is_raw_replay, csi_patgen_en[0]);

	if (vdev->total_dev_num > 1)
		ctx->is_multi_sensor = true;
	ctx->is_slice_buf_on = vi_slice_on;

	//hdr or not
	ctx->is_hdr_on = vi_hdr;
	for (int i = 0; i<ISP_PRERAW_MAX; i++)
		ctx->isp_pipe_cfg[i].is_hdr_on = vi_hdr;
	ctx->is_dpcm_on = vi_dpcm;
	ctx->is_3dnr_on = vi_3dnr ? 1 : 0;
	ctx->is_fbc_on = vi_fbc;
	ctx->is_sensor_input  = vi_sensor_input ? 1 : 0;

	*raw_max = vdev->total_dev_num;
	ctx->total_dev_num = vdev->total_dev_num;

	ctx->isp_pipe_cfg[ISP_PRERAW0].is_offline_scaler = vi_scaler ? 0 : 1;

	vi_pr(VI_DBG, "3dnr[%d], vi_fbc[%d], vi_bayerid=[%d]\n",
		ctx->is_3dnr_on, ctx->is_fbc_on, vi_bayer_id);
	vi_pr(VI_INFO, "vi_off_scaler [%d] vi_hdr=[%d], vi_dpcm[%d], vi_devs=[%d]\n",
		ctx->isp_pipe_cfg[ISP_PRERAW0].is_offline_scaler,
		ctx->is_hdr_on, ctx->is_dpcm_on, vdev->total_dev_num);
}

#define pat_width 128
#define pat_height 64

int vi_start_streaming(struct cvi_vi_dev *vdev)
{
	//struct cif_attr_s cif_attr;
	enum cvi_isp_raw raw_num = ISP_PRERAW0;
	enum cvi_isp_raw raw_max = ISP_PRERAW_MAX - 1;
	int rc = 0;

	// _vi_mempool_reset();
	// vi_tuning_buf_clear();

	_vi_scene_ctrl(vdev, &raw_max);

	//SW workaround to disable csibdg enable first due to csibdg enable is on as default.
	for (raw_num = ISP_PRERAW0; raw_num < ISP_PRERAW_MAX; raw_num++)
		isp_streaming(&vdev->ctx, false, raw_num);

	for (raw_num = ISP_PRERAW0; raw_num < raw_max; raw_num++) {

		vdev->ctx.isp_pipe_cfg[raw_num].is_patgen_en = csi_patgen_en[0];

		if (vdev->ctx.isp_pipe_cfg[raw_num].is_patgen_en) {

			if (vi_max_size) {
				vdev->usr_fmt.width = 3840;
				vdev->usr_fmt.height = 2160;
				vdev->usr_crop.width = 3840;
				vdev->usr_crop.height = 2160;
			} else {
				vdev->usr_fmt.width = pat_width;
				vdev->usr_fmt.height = pat_height;
				if (vi_crop_en) {
					vdev->usr_crop.width = 1920;
					vdev->usr_crop.height = 1080;
				} else {
					vdev->usr_crop.width = pat_width;
					vdev->usr_crop.height = pat_height;
				}
			}

			vdev->usr_fmt.code = vi_bayer_id; //depend on config_bayerid
			vdev->usr_crop.left = 0;
			vdev->usr_crop.top = 0;

			vi_pr(VI_WARN, "patgen enable, w_h(%d:%d), color mode(%d)\n",
					vdev->usr_fmt.width, vdev->usr_fmt.height, vdev->usr_fmt.code);
		} else if (vdev->ctx.is_sensor_input) {
			vdev->usr_fmt.width = 1920;
			vdev->usr_fmt.height = 1080;
			vdev->usr_crop.width = 1920;
			vdev->usr_crop.height = 1080;
			vdev->usr_fmt.code = vi_bayer_id; //depend on config_bayerid
			vdev->usr_crop.left = 0;
			vdev->usr_crop.top = 0;

			vi_pr(VI_WARN, "sensor input, w_h(%d:%d), color mode(%d)\n",
					vdev->usr_fmt.width, vdev->usr_fmt.height, vdev->usr_fmt.code);
		}

		_vi_ctrl_init(raw_num, vdev);

		_vi_pre_fe_ctrl_setup(raw_num, vdev);

		if (!vdev->ctx.is_multi_sensor) { //only single sensor maybe break
			if (!_is_fe_post_offline(&vdev->ctx) || vdev->ctx.is_slice_buf_on) {
				vi_pr(VI_INFO, "on-the-fly mode or slice_buffer is on\n");
				break;
			}
		}

		if (!vdev->ctx.isp_pipe_cfg[raw_num].is_yuv_bypass_path) { //RGB sensor
			isp_pre_trig(&vdev->ctx, raw_num, ISP_FE_CH0);
			if (vdev->ctx.isp_pipe_cfg[raw_num].is_hdr_on)
				isp_pre_trig(&vdev->ctx, raw_num, ISP_FE_CH1);
		}
	}

	_vi_preraw_fe_init(vdev);
	_vi_postraw_ctrl_setup(vdev);
	_vi_dma_setup(&vdev->ctx, raw_max);
	_vi_dma_set_sw_mode(&vdev->ctx);

	vi_pr(VI_INFO, "ISP scene path, post_off=%d, slice_buff_on=%d raw_replay=%d\n",
		vdev->ctx.is_offline_postraw, vdev->ctx.is_slice_buf_on, vdev->ctx.isp_pipe_cfg[0].is_raw_replay);

	raw_num = ISP_PRERAW0;

	if (vdev->ctx.is_slice_buf_on) {
		if (vdev->ctx.isp_pipe_cfg[raw_num].is_offline_scaler) { //offline mode
			_postraw_outbuf_enq(vdev, raw_num);

			isp_post_trig(&vdev->ctx, raw_num);

			if (!vdev->ctx.isp_pipe_cfg[raw_num].is_raw_replay) {
				if (!vdev->ctx.is_yuv_sensor) { //RGB sensor
					isp_pre_trig(&vdev->ctx, raw_num, ISP_FE_CH0);
					if (vdev->ctx.isp_pipe_cfg[raw_num].is_hdr_on)
						isp_pre_trig(&vdev->ctx, raw_num, ISP_FE_CH1);
				}
			}
		} else { //online mode
			// struct sc_cfg_cb post_para = {0};
			// u8 dev_num = vi_get_dev_num_by_raw(&vdev->ctx, raw_num);

			// /* VI Online VPSS sc cb trigger */
			// post_para.snr_num = dev_num;
			// post_para.is_tile = false;
			// if (_vi_call_cb(E_MODULE_VPSS, VPSS_CB_VI_ONLINE_TRIGGER, &post_para) != 0) {
			// 	vi_pr(VI_INFO, "snr_num_%d, sc_%d is not ready. try later\n", raw_num, dev_num);
			// } else {
			// 	atomic_set(&vdev->ol_sc_frm_done, 0);

			// 	isp_post_trig(&vdev->ctx, raw_num);
			// 	vi_record_post_trigger(vdev, raw_num);

			// 	if (!vdev->ctx.isp_pipe_cfg[raw_num].is_raw_replay_be) {
			// 		isp_pre_trig(&vdev->ctx, raw_num, ISP_FE_CH0);
			// 		if (vdev->ctx.isp_pipe_cfg[raw_num].is_hdr_on)
			// 			isp_pre_trig(&vdev->ctx, raw_num, ISP_FE_CH1);
			// 	}

			// 	atomic_set(&vdev->pre_be_state[ISP_BE_CH0], ISP_STATE_RUNNING);
			// }
		}
	} else {
		if (vdev->ctx.isp_pipe_cfg[raw_num].is_offline_scaler) { //vpss offline mode
			if (!vdev->ctx.isp_pipe_cfg[raw_num].is_raw_replay)
				isp_pre_trig(&vdev->ctx, raw_num, ISP_FE_CH0);

			_postraw_outbuf_enq(vdev, raw_num); //all online must be ch0
		} else { //vpss online mode
			// struct sc_cfg_cb post_para = {0};
			// u8 dev_num = vi_get_dev_num_by_raw(&vdev->ctx, raw_num);

			// /* VI Online VPSS sc cb trigger */
			// post_para.snr_num = dev_num;
			// post_para.is_tile = false;
			// if (_vi_call_cb(E_MODULE_VPSS, VPSS_CB_VI_ONLINE_TRIGGER, &post_para) != 0) {
			// 	vi_pr(VI_INFO, "snr_num_%d, sc_%d is not ready. try later\n", raw_num, dev_num);
			// } else {
			// 	atomic_set(&vdev->ol_sc_frm_done, 0);

			// 	if (!vdev->ctx.isp_pipe_cfg[raw_num].is_raw_replay_be)
			// 		isp_pre_trig(&vdev->ctx, raw_num, ISP_FE_CH0);
			// }
		}
	}

#ifdef PORTING_TEST
	vi_ip_test_cases_init(&vdev->ctx);
#endif

	for (raw_num = ISP_PRERAW0; raw_num < raw_max; raw_num++)
		isp_streaming(&vdev->ctx, true, raw_num);

	return rc;
}

/* abort streaming and wait for last buffer */
int vi_stop_streaming(struct cvi_vi_dev *vdev)
{
	//struct isp_buffer *isp_b;
	enum cvi_isp_raw raw_num = ISP_PRERAW0;
	//u8 i = 0;
	u8 rc = 0;

	vi_pr(VI_INFO, "+\n");
	//TODO use PTN_MALLOC, so remove free
	// for (i = 0; i < ISP_CHN_MAX; i++) {
	// 	while ((isp_b = isp_buf_remove(&pre_out_queue[i])) != NULL) {
	// 		vi_pr(VI_INFO, "%p+\n", isp_b);
	// 		vi_pr(VI_DBG, "addr %lx +\n", isp_b->addr);
	// 		//free(isp_b);
	// 	}

	// 	while ((isp_b = isp_buf_remove(&pre_out_se_queue[i])) != NULL) {
	// 		vi_pr(VI_DBG, "addr %lx +\n", isp_b->addr);
	// 		//free(isp_b);
	// 	}
	// }

	// while ((isp_b = isp_buf_remove(&post_in_queue)) != NULL);
	// 	//free(isp_b);
	// while ((isp_b = isp_buf_remove(&post_in_se_queue)) != NULL);
	// 	//free(isp_b);
	// vi_pr(VI_INFO, "+\n");
	// reset at stop for next run.
	// isp_reset(&vdev->ctx);
	for (raw_num = ISP_PRERAW0; raw_num < vdev->total_dev_num; raw_num++)
		isp_streaming(&vdev->ctx, false, raw_num);
	vi_pr(VI_INFO, "-\n");
	return rc;
}

static int _pre_fe_outbuf_enque(
	struct cvi_vi_dev *vdev,
	const enum cvi_isp_raw raw_num,
	const enum cvi_isp_pre_chn_num fe_chn_num)
{
	struct isp_ctx *ctx = &vdev->ctx;

	if (!ctx->isp_pipe_cfg[raw_num].is_yuv_bypass_path) { //RGB sensor
		enum ISP_BLK_ID_T pre_fe_dma;
		struct isp_queue *fe_out_q = (fe_chn_num == ISP_FE_CH0) ?
						&pre_out_queue[raw_num] : &pre_out_se_queue[raw_num];
		struct isp_buffer *b = NULL;

		pre_fe_dma = csibdg_dma_find_hwid(raw_num, fe_chn_num == ISP_FE_CH0);

		b = isp_next_buf(fe_out_q);
		if (!b) {
			vi_pr(VI_DBG, "pre_fe_%d chn_num_%d outbuf is empty\n", raw_num, fe_chn_num);
			return 0;
		}

		ispblk_dma_setaddr(ctx, pre_fe_dma, b->addr);
		vi_pr(VI_DBG, "pre_fe_%d chn_num_%d update addr=%lx\n", raw_num, fe_chn_num, b->addr);
	} else if (ctx->is_multi_sensor &&
		   ctx->isp_pipe_cfg[raw_num].is_yuv_bypass_path) { //RGB+YUV sensor
		u8 buf_chn = vdev->ctx.rawb_chnstr_num + fe_chn_num;
		enum ISP_BLK_ID_T pre_fe_dma = csibdg_dma_find_hwid(raw_num, ISP_RAW_PATH_LE) + fe_chn_num;
		struct isp_queue *fe_out_q = &pre_out_queue[buf_chn];
		struct isp_buffer *b = NULL;

		b = isp_next_buf(fe_out_q);
		if (!b) {
			vi_pr(VI_DBG, "pre_fe_%d buf_chn_num_%d outbuf is empty\n", raw_num, buf_chn);
			return 0;
		}

		ispblk_dma_setaddr(ctx, pre_fe_dma, b->addr);
	}

	return 1;
}

static int _postraw_inbuf_enq_check(
	struct cvi_vi_dev *vdev,
	enum cvi_isp_raw *raw_num,
	enum cvi_isp_chn_num *chn_num)
{
	struct isp_ctx *ctx = &vdev->ctx;
	struct isp_queue *in_q = NULL, *in_se_q = NULL;
	struct isp_buffer *b = NULL, *b_se = NULL;
	int ret = 1;

	if (!_is_fe_post_offline(ctx)) //fe->dram->post
		return ret;

	in_q = &post_in_queue;
	b = isp_next_buf(in_q);
	if (b == NULL) {
		vi_pr(VI_DBG, "Postraw input buf is empty\n");
		return ret;
	}

	*raw_num = b->raw_num;
	*chn_num = (b->is_yuv_frm) ? b->chn_num : b->raw_num;

	vdev->ctx.isp_pipe_cfg[b->raw_num].crop.x = b->crop_le.x;
	vdev->ctx.isp_pipe_cfg[b->raw_num].crop.y = b->crop_le.y;
	vdev->ctx.isp_pipe_cfg[b->raw_num].crop.w = vdev->ctx.img_width =
							ctx->isp_pipe_cfg[b->raw_num].post_img_w;
	vdev->ctx.isp_pipe_cfg[b->raw_num].crop.h = vdev->ctx.img_height =
							ctx->isp_pipe_cfg[b->raw_num].post_img_h;

	//YUV sensor, offline return error
	if (ctx->isp_pipe_cfg[b->raw_num].is_yuv_bypass_path) {
		if (ctx->isp_pipe_cfg[b->raw_num].is_offline_scaler) {
			return ret;
		}
	}

	if (ctx->isp_pipe_cfg[b->raw_num].is_hdr_on) {
		in_se_q = &post_in_se_queue;
		b_se = isp_next_buf(in_se_q);
		if (b_se == NULL) {
			vi_pr(VI_DBG, "Postraw se input buf is empty\n");
			return ret;
		}
	}

	if (_is_fe_post_offline(ctx)) { //fe->dram->post
		ispblk_dma_config(ctx, ISP_BLK_ID_DMA_CTL_PRE_RAW_VI_SEL_LE, b->addr);
		vi_pr(VI_DBG, "postraw b_le->addr[0x%lx] = %x\n", b->addr, readl(b->addr));
		if (ctx->isp_pipe_cfg[b->raw_num].is_hdr_on) {
			ispblk_dma_config(ctx, ISP_BLK_ID_DMA_CTL_PRE_RAW_VI_SEL_SE, b_se->addr);
			vi_pr(VI_DBG, "postraw b_se->addr[0x%lx] = %x\n", b_se->addr,  readl(b_se->addr));
		}
	}

	return 0;
}

static void _postraw_outbuf_enque(struct cvi_vi_dev *vdev, const enum cvi_isp_raw raw_num)
{
	struct vi_buffer *vb2_buf;
	struct isp_ctx *ctx = &vdev->ctx;
	u64 tmp_addr = 0, i;
	static bool is_init = false;

	if (is_init) {
		vi_pr(VI_DBG, "is_init return\n");
		return;
	}
#if 0
	struct cvi_isp_buf *b = NULL;
	//Get the buffer for postraw output buffer
	b = _cvi_isp_next_buf(vdev, raw_num);
	if (b == NULL)
		return;

	vb2_buf = &b->buf;
#else
	is_init = true;
	vb2_buf = (struct vi_buffer *)PTN_MALLOC(sizeof(struct vi_buffer));
	u64 size = vdev->usr_crop.width * vdev->usr_crop.height;

	vb2_buf->planes[0].addr = (u64)PTN_MALLOC(size);
	vb2_buf->planes[1].addr = (u64)PTN_MALLOC(size);
#endif
	vi_pr(VI_DBG, "size:%ld,  addr: %#"PRIx64"-%#"PRIx64"\n", size,
		vb2_buf->planes[0].addr, vb2_buf->planes[1].addr);
	vi_pr(VI_INFO, "fake ddr start %#"PRIx64" -end %#"PRIx64"\n",
		(vb2_buf->planes[0].addr / 2)&0xfffffff, ((vb2_buf->planes[0].addr + size * 3 / 2) / 2 - 1)&0xfffffff);

	for (i = 0; i < 2; i++) {
		tmp_addr = (u64)vb2_buf->planes[i].addr;
		if (i == 0)
			ispblk_dma_config(ctx, ISP_BLK_ID_DMA_CTL_YUV_CROP_Y, tmp_addr);
		else
			ispblk_dma_config(ctx, ISP_BLK_ID_DMA_CTL_YUV_CROP_C, tmp_addr);
	}
}

static u8 _postraw_outbuf_empty(struct cvi_vi_dev *vdev, const enum cvi_isp_raw raw_num)
{
#if 0
	u8 ret = 0;

	if (cvi_isp_rdy_buf_empty(vdev, raw_num)) {
		vi_pr(VI_DBG, "postraw chn_%d output buffer is empty\n", raw_num);
		ret = 1;
	}

	return ret;
#else
	return 0;
#endif
}

void _postraw_outbuf_enq(struct cvi_vi_dev *vdev, const enum cvi_isp_raw raw_num)
{
	//cvi_isp_rdy_buf_pop(vdev, raw_num);
	_postraw_outbuf_enque(vdev, raw_num);
}

/*
 * for postraw offline only.
 *  trig preraw if there is output buffer in preraw output.
 */
void _pre_hw_enque(
	struct cvi_vi_dev *vdev,
	const enum cvi_isp_raw raw_num,
	const u8 chn_num)
{

	struct isp_ctx *ctx = &vdev->ctx;

	if (_is_fe_post_offline(ctx)) { //fe->dram->post
		// only if fe->dram
		if (_pre_fe_outbuf_enque(vdev, raw_num, chn_num)){
			isp_pre_trig(ctx, raw_num, chn_num);
		}
	}
#if 0
	//ISP frame error handling
	if (atomic_read(&vdev->isp_err_handle_flag) == 1) {
		vi_pr(VI_DBG, "wait err_handling done\n");
		return;
	}

#ifdef PORTING_TEST //test only
	if (stop_stream_en) {
		vi_pr(VI_WARN, "stop_stream_en\n");
		return;
	}
#endif

	if (atomic_read(&vdev->isp_streamoff) == 0) {
		if (_is_drop_next_frame(vdev, raw_num, chn_num)) {
			vi_pr(VI_DBG, "Pre_fe_%d chn_num_%d drop_frame_num %d\n",
					raw_num, chn_num, vdev->drop_frame_number[raw_num]);
			return;
		}

		if (!ctx->is_slice_buf_on) { //fe->dram->post
			if (!ctx->isp_pipe_cfg[raw_num].is_yuv_bypass_path) { //RGB sensor
				if (atomic_cmpxchg(&vdev->pre_be_state[chn_num],
							ISP_PRE_BE_IDLE, ISP_PRE_BE_RUNNING) ==
							ISP_PRE_BE_RUNNING) {
					vi_pr(VI_DBG, "Pre_be chn_num_%d is running\n", chn_num);
					return;
				}
			} else { //YUV sensor
				if (atomic_cmpxchg(&vdev->pre_fe_state[raw_num][chn_num],
							ISP_PRERAW_IDLE, ISP_PRERAW_RUNNING) ==
							ISP_PRERAW_RUNNING) {
					vi_pr(VI_DBG, "Pre_fe_%d chn_num_%d is running\n", raw_num, chn_num);
					return;
				}
			}

			// only if fe->be->dram
			if (_pre_be_outbuf_enque(vdev, raw_num, chn_num)) {
				if (atomic_read(&vdev->isp_raw_dump_en[raw_num]) == 1) //raw_dump flow
					_isp_fe_be_raw_dump_cfg(vdev, raw_num, chn_num);
				isp_pre_trig(ctx, raw_num, chn_num);
			} else {
				if (!ctx->isp_pipe_cfg[raw_num].is_yuv_bypass_path) //RGB sensor
					atomic_set(&vdev->pre_be_state[chn_num], ISP_PRE_BE_IDLE);
				else  //YUV sensor
					atomic_set(&vdev->pre_fe_state[raw_num][chn_num], ISP_PRERAW_IDLE);
			}
		} else if (_is_fe_post_offline(ctx)) { //fe->dram->post
			if (ctx->isp_pipe_cfg[ISP_PRERAW0].is_offline_preraw) {
				#if 0
				if (atomic_read(&vdev->isp_streamon) == 0) {
					vi_pr(VI_DBG, "VI not ready\n");
					return;
				}

				if (atomic_cmpxchg(&vdev->postraw_state, ISP_POSTRAW_IDLE, ISP_POSTRAW_RUNNING)
							!= ISP_POSTRAW_IDLE) {
					vi_pr(VI_DBG, "Postraw is running\n");
					return;
				}

				if (ctx->isp_pipe_cfg[raw_num].is_offline_scaler) { //Scaler onffline mode
					if (_postraw_outbuf_empty(vdev, raw_num)) {
						atomic_set(&vdev->postraw_state, ISP_POSTRAW_IDLE);
						return;
					}

					_postraw_outbuf_enq(vdev, raw_num);
				} else { //Scaler online mode
					struct sc_cfg_cb post_para = {0};

					/* VI Online VPSS sc cb trigger */
					post_para.snr_num = raw_num;
					post_para.bypass_num = gViCtx->bypass_frm[raw_num];
					vi_fill_mlv_info(NULL, raw_num, &post_para.m_lv_i, false);
					if (_vi_call_cb(E_MODULE_VPSS, VPSS_CB_VI_ONLINE_TRIGGER, &post_para) != 0) {
						vi_pr(VI_DBG, "snr_num_%d, SC is running\n", raw_num);
						atomic_set(&vdev->pre_be_state[ISP_BE_CH0], ISP_PRE_BE_IDLE);
						atomic_set(&vdev->postraw_state, ISP_POSTRAW_IDLE);
						return;
					}

					atomic_set(&vdev->ol_sc_frm_done, 0);
				}

				if (atomic_read(&vdev->isp_raw_dump_en[raw_num]) == 1) //raw_dump flow
					_isp_fe_be_raw_dump_cfg(vdev, raw_num, chn_num);

				isp_pre_trig(ctx, raw_num, chn_num);
				#endif
			} else {
				// if (atomic_cmpxchg(&vdev->pre_fe_state[raw_num][chn_num],
				// 			ISP_PRERAW_IDLE, ISP_PRERAW_RUNNING) ==
				// 			ISP_PRERAW_RUNNING) {
				// 	vi_pr(VI_DBG, "Pre_fe_%d chn_num_%d is running\n", raw_num, chn_num);
				// 	return;
				// }

				// only if fe->dram
				if (_pre_fe_outbuf_enque(vdev, raw_num, chn_num))
					isp_pre_trig(ctx, raw_num, chn_num);
				// else
				// 	atomic_set(&vdev->pre_fe_state[raw_num][chn_num], ISP_PRERAW_IDLE);
			}
		} else if (!_is_fe_post_offline(ctx)) {
			if (atomic_cmpxchg(&vdev->postraw_state, ISP_POSTRAW_IDLE, ISP_POSTRAW_RUNNING)
						!= ISP_POSTRAW_IDLE) {
				vi_pr(VI_DBG, "Postraw is running\n");
				return;
			}

			if (ctx->isp_pipe_cfg[raw_num].is_offline_scaler) { //Scaler onffline mode
				if (_postraw_outbuf_empty(vdev, raw_num)) {
					atomic_set(&vdev->postraw_state, ISP_POSTRAW_IDLE);
					return;
				}

				_postraw_outbuf_enq(vdev, raw_num);
			}

			if (atomic_read(&vdev->isp_raw_dump_en[raw_num]) == 1) //raw_dump flow
				_isp_fe_be_raw_dump_cfg(vdev, raw_num, chn_num);

			isp_pre_trig(ctx, raw_num, chn_num);
		} else if (ctx->is_slice_buf_on) { //fe->be->dram->post
			if (!ctx->isp_pipe_cfg[raw_num].is_yuv_bypass_path) { //RGB sensor
				if (atomic_cmpxchg(&vdev->pre_fe_state[raw_num][chn_num],
							ISP_PRERAW_IDLE, ISP_PRERAW_RUNNING) ==
							ISP_PRERAW_RUNNING) {
					vi_pr(VI_DBG, "Pre_fe chn_num_%d is running\n", chn_num);
					return;
				}
			} else { //YUV sensor
				if (atomic_cmpxchg(&vdev->pre_fe_state[raw_num][chn_num],
							ISP_PRERAW_IDLE, ISP_PRERAW_RUNNING) ==
							ISP_PRERAW_RUNNING) {
					vi_pr(VI_DBG, "Pre_fe_%d chn_num_%d is running\n", raw_num, chn_num);
					return;
				}
			}

			if (atomic_read(&vdev->isp_raw_dump_en[raw_num]) == 1) //raw_dump flow
				_isp_fe_be_raw_dump_cfg(vdev, raw_num, chn_num);
			isp_pre_trig(ctx, raw_num, chn_num);
		}

	}
#endif
}

static inline void _swap_post_sts_buf(struct isp_ctx *ctx, const enum cvi_isp_raw raw_num)
{
	struct _membuf *pool;
	unsigned long flags;
	// int idx;

	pool = &isp_bufpool[raw_num];

	spin_lock_irqsave(&pool->post_sts_lock, flags);
	if (pool->post_sts_in_use == 1) {
		spin_unlock_irqrestore(&pool->post_sts_lock, flags);
		return;
	}
	pool->post_sts_busy_idx ^= 1;
	spin_unlock_irqrestore(&pool->post_sts_lock, flags);

	// if (_is_fe_post_offline(ctx))
	// 	idx = pool->post_sts_busy_idx ^ 1;
	// else
	// 	idx = pool->post_sts_busy_idx;
}

static inline void _post_drc_update(struct isp_ctx *ctx, const enum cvi_isp_raw raw_num)
{
	uint64_t drc_tmp = isp_bufpool[raw_num].drc_poly[0];
	isp_bufpool[raw_num].drc_poly[0] = isp_bufpool[raw_num].drc_poly[1];
	isp_bufpool[raw_num].drc_poly[1] = drc_tmp;

	ispblk_dma_setaddr(ctx, ISP_BLK_ID_DMA_CTL_DRC_POLY_W, isp_bufpool[raw_num].drc_poly[0]);
	ispblk_dma_setaddr(ctx, ISP_BLK_ID_DMA_CTL_DRC_POLY_R, isp_bufpool[raw_num].drc_poly[1]);
}

static inline void _post_ldci_update(struct isp_ctx *ctx, const enum cvi_isp_raw raw_num)
{
	uint64_t ldci_tmp = isp_bufpool[raw_num].ldci[0];
	isp_bufpool[raw_num].ldci[0] = isp_bufpool[raw_num].ldci[1];
	isp_bufpool[raw_num].ldci[1] = ldci_tmp;

	ispblk_dma_setaddr(ctx, ISP_BLK_ID_DMA_CTL_LDCI_W, isp_bufpool[raw_num].ldci[0]);
	ispblk_dma_setaddr(ctx, ISP_BLK_ID_DMA_CTL_LDCI_R, isp_bufpool[raw_num].ldci[1]);
}

static inline void _post_mctf_update(struct isp_ctx *ctx, const enum cvi_isp_raw raw_num)
{
	uint64_t mctf_tmp;

	mctf_tmp = isp_bufpool[raw_num].tdnr_mo[0];
	isp_bufpool[raw_num].tdnr_mo[0] = isp_bufpool[raw_num].tdnr_mo[1];
	isp_bufpool[raw_num].tdnr_mo[1] = mctf_tmp;

	ispblk_dma_setaddr(ctx, ISP_BLK_ID_DMA_CTL_TNR_ST_MO, isp_bufpool[raw_num].tdnr_mo[0]);
	ispblk_dma_setaddr(ctx, ISP_BLK_ID_DMA_CTL_TNR_LD_MO, isp_bufpool[raw_num].tdnr_mo[1]);

	mctf_tmp = isp_bufpool[raw_num].tdnr_mv[0];
	isp_bufpool[raw_num].tdnr_mv[0] = isp_bufpool[raw_num].tdnr_mv[1];
	isp_bufpool[raw_num].tdnr_mv[1] = mctf_tmp;

	ispblk_dma_setaddr(ctx, ISP_BLK_ID_DMA_CTL_TNR_ST_MV, isp_bufpool[raw_num].tdnr_mv[0]);
	ispblk_dma_setaddr(ctx, ISP_BLK_ID_DMA_CTL_TNR_LD_MV, isp_bufpool[raw_num].tdnr_mv[1]);

	mctf_tmp = isp_bufpool[raw_num].tdnr_fcb[0];
	isp_bufpool[raw_num].tdnr_fcb[0] = isp_bufpool[raw_num].tdnr_fcb[1];
	isp_bufpool[raw_num].tdnr_fcb[1] = mctf_tmp;

	ispblk_dma_setaddr(ctx, ISP_BLK_ID_DMA_CTL_TNR_ST_FCB, isp_bufpool[raw_num].tdnr_fcb[0]);
	ispblk_dma_setaddr(ctx, ISP_BLK_ID_DMA_CTL_TNR_LD_FCB, isp_bufpool[raw_num].tdnr_fcb[1]);

	mctf_tmp = isp_bufpool[raw_num].tdnr_y[0];
	isp_bufpool[raw_num].tdnr_y[0] = isp_bufpool[raw_num].tdnr_y[1];
	isp_bufpool[raw_num].tdnr_y[1] = mctf_tmp;

	ispblk_dma_setaddr(ctx, ISP_BLK_ID_DMA_CTL_TNR_ST_Y, isp_bufpool[raw_num].tdnr_y[0]);
	ispblk_dma_setaddr(ctx, ISP_BLK_ID_DMA_CTL_TNR_LD_Y, isp_bufpool[raw_num].tdnr_y[1]);

	mctf_tmp = isp_bufpool[raw_num].tdnr_uv[0];
	isp_bufpool[raw_num].tdnr_uv[0] = isp_bufpool[raw_num].tdnr_uv[1];
	isp_bufpool[raw_num].tdnr_uv[1] = mctf_tmp;

	ispblk_dma_setaddr(ctx, ISP_BLK_ID_DMA_CTL_TNR_ST_C, isp_bufpool[raw_num].tdnr_uv[0]);
	ispblk_dma_setaddr(ctx, ISP_BLK_ID_DMA_CTL_TNR_LD_C, isp_bufpool[raw_num].tdnr_uv[1]);
}

static inline void _post_cnr_update(struct isp_ctx *ctx, const enum cvi_isp_raw raw_num)
{
	uint64_t cnr_tmp;

	cnr_tmp = isp_bufpool[raw_num].cnr_y[0];
	isp_bufpool[raw_num].cnr_y[0] = isp_bufpool[raw_num].cnr_y[1];
	isp_bufpool[raw_num].cnr_y[1] = cnr_tmp;

	ispblk_dma_setaddr(ctx, ISP_BLK_ID_DMA_CTL_CNR_Y_W, isp_bufpool[raw_num].cnr_y[0]);
	ispblk_dma_setaddr(ctx, ISP_BLK_ID_DMA_CTL_CNR_Y_R, isp_bufpool[raw_num].cnr_y[1]);

	cnr_tmp = isp_bufpool[raw_num].cnr_uv[0];
	isp_bufpool[raw_num].cnr_uv[0] = isp_bufpool[raw_num].cnr_uv[1];
	isp_bufpool[raw_num].cnr_uv[1] = cnr_tmp;

	ispblk_dma_setaddr(ctx, ISP_BLK_ID_DMA_CTL_CNR_C_W, isp_bufpool[raw_num].cnr_uv[0]);
	ispblk_dma_setaddr(ctx, ISP_BLK_ID_DMA_CTL_CNR_C_R, isp_bufpool[raw_num].cnr_uv[1]);
}

static inline void _post_dma_update(struct cvi_vi_dev *vdev, const enum cvi_isp_raw raw_num)
{
	struct isp_ctx *ctx = &vdev->ctx;
	uint64_t r_uv_addr, r_y_addr;
	uint64_t w_uv_addr, w_y_addr;

	r_uv_addr = w_uv_addr = isp_bufpool[raw_num].tdnr[0];
	r_y_addr  = w_y_addr  = isp_bufpool[raw_num].tdnr[1];

	//update drc dma
	_post_drc_update(ctx, raw_num);

	//update ldci dma
	_post_ldci_update(ctx, raw_num);

	//update mctf dma
	_post_mctf_update(ctx, raw_num);

	//update cnr dma
	_post_cnr_update(ctx, raw_num);

	if (ctx->is_3dnr_on) {
		if (ctx->is_fbc_on) {
			//3dnr y
			ispblk_dma_config(ctx, ISP_BLK_ID_DMA_CTL_TNR_ST_Y, r_y_addr);
			ispblk_dma_config(ctx, ISP_BLK_ID_DMA_CTL_TNR_LD_Y, w_y_addr);

			//3dnr uv
			ispblk_dma_config(ctx, ISP_BLK_ID_DMA_CTL_TNR_ST_C, r_uv_addr);
			ispblk_dma_config(ctx, ISP_BLK_ID_DMA_CTL_TNR_LD_C, w_uv_addr);
		} else {
			//3dnr y
			ispblk_dma_setaddr(ctx, ISP_BLK_ID_DMA_CTL_TNR_ST_Y, r_y_addr);
			ispblk_dma_setaddr(ctx, ISP_BLK_ID_DMA_CTL_TNR_LD_Y, w_y_addr);

			//3dnr uv
			ispblk_dma_setaddr(ctx, ISP_BLK_ID_DMA_CTL_TNR_ST_C, r_uv_addr);
			ispblk_dma_setaddr(ctx, ISP_BLK_ID_DMA_CTL_TNR_LD_C, w_uv_addr);
		}
	}

	if (ctx->isp_pipe_cfg[raw_num].is_offline_scaler) {
		ispblk_dma_enable(ctx, ISP_BLK_ID_DMA_CTL_YUV_CROP_Y, true, false);
		ispblk_dma_enable(ctx, ISP_BLK_ID_DMA_CTL_YUV_CROP_C, true, false);
	} else {
		ispblk_dma_enable(ctx, ISP_BLK_ID_DMA_CTL_YUV_CROP_Y, false, false);
		ispblk_dma_enable(ctx, ISP_BLK_ID_DMA_CTL_YUV_CROP_C, false, false);
	}
}

static u32 _is_fisrt_frm_after_drop(
	struct cvi_vi_dev *vdev,
	const enum cvi_isp_raw raw_num)
{
	struct isp_ctx *ctx = &vdev->ctx;
	uint32_t first_frm_num_after_drop = ctx->isp_pipe_cfg[raw_num].isp_reset_frm;
	u32 frm_num = 0;

	frm_num = vdev->pre_fe_frm_num[raw_num][ISP_FE_CH0];

	if ((first_frm_num_after_drop != 0) && (frm_num == first_frm_num_after_drop)) {
		vi_pr(VI_DBG, "reset isp frm_num[%d]\n", frm_num);
		ctx->isp_pipe_cfg[raw_num].isp_reset_frm = 0;
		return 1;
	} else
		return 0;
}

static inline void _post_ctrl_update(struct cvi_vi_dev *vdev, const enum cvi_isp_raw raw_num)
{
	struct isp_ctx *ctx = &vdev->ctx;

	ispblk_post_cfg_update(ctx, raw_num);

	ispblk_fusion_hdr_cfg(ctx, raw_num);

	_vi_clear_mmap_fbc_ring_base(vdev, raw_num);

	if (ctx->is_multi_sensor) {
		//To set apply the prev frm or not for manr/3dnr
		if (vdev->preraw_first_frm[raw_num]) {
			vdev->preraw_first_frm[raw_num] = false;
			isp_first_frm_reset(ctx, 1);
		} else {
			isp_first_frm_reset(ctx, 0);
		}
	}

	if (_is_fisrt_frm_after_drop(vdev, raw_num)) {
		isp_first_frm_reset(ctx, 1);
	} else {
		isp_first_frm_reset(ctx, 0);
	}
}

static inline int _isp_clk_dynamic_en(struct cvi_vi_dev *vdev, bool en)
{
#if 0//ToDo
	if (clk_dynamic_en && vdev->isp_clk[5]) {
		struct isp_ctx *ctx = &vdev->ctx;

		if (en && !__clk_is_enabled(vdev->isp_clk[5])) {
			if (clk_enable(vdev->isp_clk[5])) {
				vi_pr(VI_ERR, "[ERR] ISP_CLK(%s) enable fail\n", CLK_ISP_NAME[5]);
				if (_is_fe_post_offline(ctx)) {
					atomic_set(&vdev->pre_be_state[ISP_BE_CH0], ISP_PRE_BE_IDLE);
					atomic_set(&vdev->postraw_state, ISP_POSTRAW_IDLE);
				}

				return -1;
			}

			vi_pr(VI_DBG, "enable clk(%s)\n", CLK_ISP_NAME[5]);
		} else if (!en && __clk_is_enabled(vdev->isp_clk[5])) {
			clk_disable(vdev->isp_clk[5]);

			vi_pr(VI_DBG, "disable clk(%s)\n", CLK_ISP_NAME[5]);
		}
	} else { //check isp_top_clk is enabled
		struct isp_ctx *ctx = &vdev->ctx;

		if (!__clk_is_enabled(vdev->isp_clk[5])) {
			if (clk_enable(vdev->isp_clk[5])) {
				vip_pr(CVI_ERR, "[ERR] ISP_CLK(%s) enable fail\n", CLK_ISP_NAME[5]);
				if (_is_fe_post_offline(ctx)) {
					atomic_set(&vdev->pre_be_state[ISP_BE_CH0], ISP_PRE_BE_IDLE);
					atomic_set(&vdev->postraw_state, ISP_POSTRAW_IDLE);
				}
			}
		}
	}
#endif
	return 0;
}

/*
 * - postraw offline -
 *  trig postraw if there is in/out buffer for postraw
 * - postraw online -
 *  trig preraw if there is output buffer for postraw
 */

void _post_hw_enque(struct cvi_vi_dev *vdev)
{
	struct isp_ctx *ctx = &vdev->ctx;
	enum cvi_isp_raw raw_num = ISP_PRERAW0;
	enum cvi_isp_chn_num chn_num = ISP_CHN0;

	// if (atomic_read(&vdev->isp_streamoff) == 1 && !ctx->is_slice_buf_on) {
	// 	vi_pr(VI_DBG, "stop streaming\n");
	// 	return;
	// }

	// if (atomic_read(&vdev->isp_err_handle_flag) == 1) {
	// 	vi_pr(VI_DBG, "wait err_handing done\n");
	// 	return;
	// }
	vi_pr(VI_DBG, "post_off:%d scaler_off:%d raw_replay:%d\n",
		ctx->is_offline_postraw, ctx->isp_pipe_cfg[raw_num].is_offline_scaler,
		ctx->isp_pipe_cfg[raw_num].is_raw_replay);
	if (_is_fe_post_offline(ctx)) { //fe->dram->post
		// if (atomic_cmpxchg(&vdev->pre_be_state[ISP_BE_CH0], ISP_PRE_BE_IDLE, ISP_PRE_BE_RUNNING)
		// 								!= ISP_PRE_BE_IDLE) {
		// 	vi_pr(VI_DBG, "Pre_be ch_num_%d is running\n", ISP_BE_CH0);
		// 	return;
		// }

		// if (atomic_cmpxchg(&vdev->postraw_state, ISP_POSTRAW_IDLE, ISP_POSTRAW_RUNNING) != ISP_POSTRAW_IDLE) {
		// 	atomic_set(&vdev->pre_be_state[ISP_BE_CH0], ISP_PRE_BE_IDLE);
		// 	vi_pr(VI_DBG, "Postraw is running\n");
		// 	return;
		// }
		//TODO multi sensor not trigger
		// if (ctx->total_dev_num > 1) {
		// 	vi_pr(VI_DBG, "multi sensor_%d not trigger postraw\n", raw_num);
		// 	return;
		// }

		if (ctx->is_slice_buf_on) {
			//Things have to be done in post done
			// if (atomic_cmpxchg(&ctx->is_post_done, 1, 0) == 1) { //Change is_post_done flag to 0
			if (ctx->is_post_done == 1) {
				// if (_is_drop_next_frame(vdev, raw_num, ISP_FE_CH0)) {
				// 	vi_pr(VI_DBG, "is drop\n");
				// 	atomic_set(&ctx->is_post_done, 1);
				// 	return;
				// }
				ctx->is_post_done = 0;
				if (ctx->isp_pipe_cfg[raw_num].is_offline_scaler) { //Scaler offline mode
					if (!_postraw_outbuf_empty(vdev, raw_num))
						_postraw_outbuf_enq(vdev, raw_num);
				}

				_vi_clear_mmap_fbc_ring_base(vdev, raw_num);

				vi_tuning_gamma_ips_update(ctx, raw_num);
				vi_tuning_clut_update(ctx, raw_num);
				vi_tuning_ldci_update(ctx, raw_num);
				vi_tuning_drc_update(ctx, raw_num);
			} else { //Things have to be done in be done for fps issue
				// if (atomic_read(&vdev->isp_streamoff) == 0) {
				// 	if (atomic_cmpxchg(&vdev->pre_be_state[ISP_BE_CH0], ISP_PRE_BE_IDLE, ISP_PRE_BE_RUNNING)
				// 		!= ISP_PRE_BE_IDLE) {
				// 		vi_pr(VI_DBG, "BE is running\n");
				// 		return;
				// 	}

				// 	if (!ctx->isp_pipe_cfg[raw_num].is_offline_scaler) { //Scaler online mode
				// 		struct sc_cfg_cb post_para = {0};

				// 		/* VI Online VPSS sc cb trigger */
				// 		post_para.snr_num = raw_num;
				// 		post_para.bypass_num = gViCtx->bypass_frm[raw_num];
				// 		vi_fill_mlv_info(NULL, raw_num, &post_para.m_lv_i, false);
				// 		if (_vi_call_cb(E_MODULE_VPSS, VPSS_CB_VI_ONLINE_TRIGGER, &post_para) != 0) {
				// 			vi_pr(VI_DBG, "snr_num_%d, SC is not ready\n", raw_num);
				// 			atomic_set(&vdev->pre_be_state[ISP_BE_CH0], ISP_PRE_BE_IDLE);
				// 			return;
				// 		}

				// 		atomic_set(&vdev->ol_sc_frm_done, 0);
				// 	}
				// }

				vdev->offline_raw_num = raw_num;
				ctx->cam_id = raw_num;

				//atomic_set(&vdev->postraw_state, ISP_POSTRAW_RUNNING);

				isp_post_trig(ctx, raw_num);
				//vi_record_post_trigger(vdev, raw_num);
	#ifdef TEST_PERF
				vdev->isp_timer.pre_trigpost_time[raw_num] = timer_get_tick();
	#endif


				//_pre_hw_enque(vdev, raw_num, ISP_FE_CH0);
				//if (ctx->isp_pipe_cfg[raw_num].is_hdr_on)
				//	_pre_hw_enque(vdev, raw_num, ISP_FE_CH1);
			}
		} else {
			if (!ctx->isp_pipe_cfg[raw_num].is_raw_replay)
				if (_postraw_inbuf_enq_check(vdev, &raw_num, &chn_num)) {
					// atomic_set(&vdev->pre_be_state[ISP_BE_CH0], ISP_PRE_BE_IDLE);
					// atomic_set(&vdev->postraw_state, ISP_POSTRAW_IDLE);
					return;
				}

			if (!ctx->isp_pipe_cfg[raw_num].is_offline_scaler) { //Scaler online mode
				// struct sc_cfg_cb post_para = {0};

				// /* VI Online VPSS sc cb trigger */
				// post_para.snr_num = raw_num;
				// post_para.bypass_num = gViCtx->bypass_frm[raw_num];
				// vi_fill_mlv_info(NULL, raw_num, &post_para.m_lv_i, false);
				// if (_vi_call_cb(E_MODULE_VPSS, VPSS_CB_VI_ONLINE_TRIGGER, &post_para) != 0) {
				// 	vi_pr(VI_DBG, "snr_num_%d, SC is running\n", raw_num);
				// 	atomic_set(&vdev->pre_be_state[ISP_BE_CH0], ISP_PRE_BE_IDLE);
				// 	atomic_set(&vdev->postraw_state, ISP_POSTRAW_IDLE);
				// 	return;
				// }
	#ifdef BUILD_VIP
				extern int vpss_start_online(void);
				if (vi_scaler_scenery == 0) {
					if (vi_scaler == 5) {
						u64 dst_addr[4] = {0};
						u32 size = ctx->isp_pipe_cfg[raw_num].csibdg_width *  ctx->isp_pipe_cfg[raw_num].csibdg_height * 3;

						dst_addr[0] = PTN_MALLOC(size);
						dst_addr[1] = PTN_MALLOC(size);
						dst_addr[2] = PTN_MALLOC(size);
						dst_addr[3] = PTN_MALLOC(size);

						vpss_online_multi(ctx->isp_pipe_cfg[raw_num].csibdg_width, ctx->isp_pipe_cfg[raw_num].csibdg_height, dst_addr);
						vpss_start_online();
						vi_pr(VI_DBG, "vi_scaler %d, vpss_online_multi\n", vi_scaler);
					} else {
						u64 dst_addr = 0;
						u32 size = ctx->isp_pipe_cfg[raw_num].csibdg_width *  ctx->isp_pipe_cfg[raw_num].csibdg_height * 3;
						dst_addr = PTN_MALLOC(size);
						vpss_online_single(vi_scaler - 1, ctx->isp_pipe_cfg[raw_num].csibdg_width,
									ctx->isp_pipe_cfg[raw_num].csibdg_height, dst_addr);
						vpss_start_online();
						vi_pr(VI_DBG, "dev %d, vpss_online_single\n", vi_scaler - 1);
					}
				} else {
					// scenery 1:
					//       |---> VPSS0
					//       |---> VPSS1
					// ISP-> |
					//       |---> VPSS2
					//       |---> VPSS3
					u64 dst_addr[4] = {0};
					u32 size = ctx->isp_pipe_cfg[raw_num].csibdg_width *  ctx->isp_pipe_cfg[raw_num].csibdg_height * 3;

					dst_addr[0] = PTN_MALLOC(size);
					dst_addr[1] = PTN_MALLOC(size);
					dst_addr[2] = PTN_MALLOC(size);
					dst_addr[3] = PTN_MALLOC(size);

					if (vi_scaler==1 || vi_scaler==5)
						vpss_online_single(VPSS_V0, ctx->isp_pipe_cfg[raw_num].csibdg_width, ctx->isp_pipe_cfg[raw_num].csibdg_height, dst_addr[0]);
					if (vi_scaler==2 || vi_scaler==5)
						vpss_online_single(VPSS_V1, ctx->isp_pipe_cfg[raw_num].csibdg_width, ctx->isp_pipe_cfg[raw_num].csibdg_height, dst_addr[1]);
					if (vi_scaler==3 || vi_scaler==5)
						vpss_online_single(VPSS_V2, ctx->isp_pipe_cfg[raw_num].csibdg_width, ctx->isp_pipe_cfg[raw_num].csibdg_height, dst_addr[2]);
					if (vi_scaler==4 || vi_scaler==5)
						vpss_online_single(VPSS_V3, ctx->isp_pipe_cfg[raw_num].csibdg_width, ctx->isp_pipe_cfg[raw_num].csibdg_height, dst_addr[3]);

					// vpss_start_online();
					vi_pr(VI_DBG, "vpss_online_I1O4 vi_scaler_scenery[%d]\n", vi_scaler_scenery);
				}
	#endif
				// atomic_set(&vdev->ol_sc_frm_done, 0);
			} else { //Scaler offline mode
				if (_postraw_outbuf_empty(vdev, raw_num)) {
					// atomic_set(&vdev->pre_be_state[ISP_BE_CH0], ISP_PRE_BE_IDLE);
					// atomic_set(&vdev->postraw_state, ISP_POSTRAW_IDLE);
					return;
				}

				_postraw_outbuf_enq(vdev, raw_num);
			}

			// if (_isp_clk_dynamic_en(vdev, true) < 0)
			// 	return;

			//ispblk_post_yuv_cfg_update(ctx, raw_num);

			if (ctx->isp_pipe_cfg[raw_num].is_yuv_bypass_path) //YUV sensor online mode
				goto YUV_POSTRAW;

			postraw_tuning_update(&vdev->ctx, raw_num);

			//Update postraw size/ctrl flow
			_post_ctrl_update(vdev, raw_num);
			//Update postraw dma size/addr
			_post_dma_update(vdev, raw_num);
			//Update postraw sts awb/dci/hist_edge_v dma size/addr
			_swap_post_sts_buf(ctx, raw_num);

	YUV_POSTRAW:
			vdev->offline_raw_num = raw_num;

			ctx->cam_id = raw_num;

			isp_post_trig(ctx, raw_num);
	#ifdef TEST_PERF
			vdev->isp_timer.pre_trigpost_time[raw_num] = timer_get_tick();
	#endif
		}

	} else { //on-the-fly

		// if (atomic_read(&vdev->postraw_state) == ISP_POSTRAW_RUNNING) {
		// 	vi_pr(VI_DBG, "Postraw is running\n");
		// 	return;
		// }

		if (!ctx->isp_pipe_cfg[raw_num].is_offline_scaler) { //Scaler online mode
			// struct sc_cfg_cb post_para = {0};

			// /* VI Online VPSS sc cb trigger */
			// post_para.snr_num = raw_num;
			// post_para.bypass_num = gViCtx->bypass_frm[raw_num];
			// vi_fill_mlv_info(NULL, raw_num, &post_para.m_lv_i, false);
			// if (_vi_call_cb(E_MODULE_VPSS, VPSS_CB_VI_ONLINE_TRIGGER, &post_para) != 0) {
			// 	vi_pr(VI_DBG, "snr_num_%d, SC is running\n", raw_num);
			// 	atomic_set(&vdev->postraw_state, ISP_POSTRAW_IDLE);
			// 	return;
			// }

			// atomic_set(&vdev->ol_sc_frm_done, 0);
		}

		_vi_clear_mmap_fbc_ring_base(vdev, raw_num);

		vi_tuning_gamma_ips_update(ctx, raw_num);
		vi_tuning_clut_update(ctx, raw_num);
		vi_tuning_ldci_update(ctx, raw_num);
		vi_tuning_drc_update(ctx, raw_num);

		// _pre_hw_enque(vdev, raw_num, ISP_FE_CH0);
	}
}

static void _vi_sw_init(struct cvi_vi_dev *vdev)
{
	struct isp_ctx *ctx = &vdev->ctx;
	u8 i = 0, j = 0;

	vdev->postraw_proc_num		= 0;
	ctx->is_offline_postraw		= 1;
	ctx->is_3dnr_on			= 1;
	ctx->is_dpcm_on			= 0;
	ctx->is_hdr_on			= 0;
	ctx->is_multi_sensor		= 0;
	ctx->is_yuv_sensor		= 0;
	ctx->is_sublvds_path		= 0;
	ctx->is_fbc_on			= 1;
	ctx->is_ctrl_inited		= 0;
	ctx->is_slice_buf_on		= 0;
	ctx->cam_id			= 0;
	//ctx->rawb_chnstr_num		= 1;
	ctx->total_chn_num		= 0;
	ctx->gamma_tbl_idx		= vi_gamma_mem_sel;
	ctx->cnr_scale_ratio		= 8;

	for (i = 0; i < vdev->total_dev_num; i++) { //ISP_PRERAW_MAX
		vdev->preraw_first_frm[i]			= true;
		vdev->postraw_frame_number[i]			= 0;
		vdev->drop_frame_number[i]			= 0;
		vdev->dump_frame_number[i]			= 0;
		vdev->isp_int_flag[i]				= false;

		// memset(&ctx->isp_pipe_cfg[i], 0, sizeof(struct _isp_cfg));

		ctx->isp_pipe_cfg[i].is_offline_scaler		= true;

		for (j = 0; j < ISP_FE_CHN_MAX; j++) {
			vdev->pre_fe_sof_cnt[i][j]		= 0;
			vdev->pre_fe_frm_num[i][j]		= 0;
			vdev->pre_fe_trig_cnt[i][j]		= 0;

		}
	}

	for (i = 0; i < ISP_PRERAW_MAX; i++) {
		INIT_LIST_HEAD(&pre_out_queue[i].rdy_queue);
		INIT_LIST_HEAD(&pre_out_se_queue[i].rdy_queue);
		pre_out_queue[i].num_rdy       = 0;
		pre_out_queue[i].raw_num       = 0;
		pre_out_se_queue[i].num_rdy    = 0;
		pre_out_se_queue[i].raw_num    = 0;
	}

	INIT_LIST_HEAD(&post_in_queue.rdy_queue);
	INIT_LIST_HEAD(&post_in_se_queue.rdy_queue);
	post_in_queue.num_rdy	 = 0;
	post_in_se_queue.num_rdy = 0;

	vi_tuning_sw_init();
}

static void _vi_init_param(struct cvi_vi_dev *vdev)
{
	struct isp_ctx *ctx = &vdev->ctx;
	uint8_t i = 0;

	memset(ctx, 0, sizeof(*ctx));

	ctx->phys_regs		= isp_get_phys_reg_bases();
	ctx->cam_id		= 0;

	for (i = 0; i < vdev->total_dev_num; i++) { //ISP_PRERAW_MAX
		ctx->rgb_color_mode[i]			= ISP_BAYER_TYPE_GB;
		ctx->isp_pipe_cfg[i].is_patgen_en	= 0;
		ctx->isp_pipe_cfg[i].is_yuv_bypass_path	= 0;
		ctx->isp_pipe_cfg[i].is_drop_next_frame	= 0;
		ctx->isp_pipe_cfg[i].isp_reset_frm	= 0;
		ctx->isp_pipe_cfg[i].is_422_to_420	= 0;
		ctx->isp_pipe_cfg[i].max_height		= 0;
		ctx->isp_pipe_cfg[i].max_width		= 0;
		ctx->isp_pipe_cfg[i].csibdg_width	= 0;
		ctx->isp_pipe_cfg[i].csibdg_height	= 0;
    		ctx->isp_pipe_cfg[i].is_raw_replay 	= 1;
	}

	memset(&vdev->usr_crop, 0, sizeof(vdev->usr_crop));
}

static int _vi_mempool_setup(void)
{
	int ret = 0;

	_vi_mempool_reset();
	ret = vi_tuning_buf_setup();

	return ret;
}

/*******************************************************
 *  Irq handlers
 ******************************************************/

void _vi_debug_info(struct isp_ctx *ctx)
{
	uint8_t i = 0;
	uintptr_t isptop = ctx->phys_regs[ISP_BLK_ID_ISPTOP];
	uintptr_t preraw_fe = ctx->phys_regs[ISP_BLK_ID_PRE_RAW_FE0];
	uintptr_t preraw_fe_1 = ctx->phys_regs[ISP_BLK_ID_PRE_RAW_FE1];
	uintptr_t yuvtop = ctx->phys_regs[ISP_BLK_ID_YUVTOP];
	uintptr_t rgbtop = ctx->phys_regs[ISP_BLK_ID_RGBTOP];
	uintptr_t rawtop1 = ctx->phys_regs[ISP_BLK_ID_RAWTOP1];
	uintptr_t wdma_com_1 = ctx->phys_regs[ISP_BLK_ID_WDMA_CORE1];
	uintptr_t wdma_com_2 = ctx->phys_regs[ISP_BLK_ID_WDMA_CORE2];
	uintptr_t wdma_com_3 = ctx->phys_regs[ISP_BLK_ID_WDMA_CORE3];
	uintptr_t wdma_com_4 = ctx->phys_regs[ISP_BLK_ID_WDMA_CORE4];
	uintptr_t rdma_com_1 = ctx->phys_regs[ISP_BLK_ID_RDMA_CORE1];
	uintptr_t rdma_com_2 = ctx->phys_regs[ISP_BLK_ID_RDMA_CORE2];
	uintptr_t rdma_com_3 = ctx->phys_regs[ISP_BLK_ID_RDMA_CORE3];
	struct cvi_vi_info vi_info;
	struct _dma_dbg_i data;

	//RDMA_0 err status
	data.rdma_1_err_sts = ISP_RD_REG(rdma_com_1, REG_RDMA_CORE_T, NORM_STATUS0); //0x10
	//RDMA_0 status
	data.rdma_1_idle = ISP_RD_REG(rdma_com_1, REG_RDMA_CORE_T, NORM_STATUS1); //0x14

	//RDMA_1 err status
	data.rdma_2_err_sts = ISP_RD_REG(rdma_com_2, REG_RDMA_CORE_T, NORM_STATUS0); //0x10
	//RDMA_1 status
	data.rdma_2_idle = ISP_RD_REG(rdma_com_2, REG_RDMA_CORE_T, NORM_STATUS1); //0x14

	//RDMA_1 err status
	data.rdma_3_err_sts = ISP_RD_REG(rdma_com_3, REG_RDMA_CORE_T, NORM_STATUS0); //0x10
	//RDMA_1 status
	data.rdma_3_idle = ISP_RD_REG(rdma_com_3, REG_RDMA_CORE_T, NORM_STATUS1); //0x14

	//WDMA_1 err status
	data.wdma_1_err_sts = ISP_RD_REG(wdma_com_1, REG_WDMA_CORE_T, NORM_STATUS0); //0x10
	//WDMA_1 status
	data.wdma_1_idle = ISP_RD_REG(wdma_com_1, REG_WDMA_CORE_T, NORM_STATUS1); //0x14

	//WDMA_2 err status
	data.wdma_2_err_sts = ISP_RD_REG(wdma_com_2, REG_WDMA_CORE_T, NORM_STATUS0); //0x10
	//WDMA_2 status
	data.wdma_2_idle = ISP_RD_REG(wdma_com_2, REG_WDMA_CORE_T, NORM_STATUS1); //0x14

	//WDMA_3 err status
	data.wdma_3_err_sts = ISP_RD_REG(wdma_com_3, REG_WDMA_CORE_T, NORM_STATUS0); //0x10
	//WDMA_3 status
	data.wdma_3_idle = ISP_RD_REG(wdma_com_3, REG_WDMA_CORE_T, NORM_STATUS1); //0x14

	//WDMA_4 err status
	data.wdma_4_err_sts = ISP_RD_REG(wdma_com_4, REG_WDMA_CORE_T, NORM_STATUS0); //0x10
	//WDMA_4 status
	data.wdma_4_idle = ISP_RD_REG(wdma_com_4, REG_WDMA_CORE_T, NORM_STATUS1); //0x14

	//isp_top
	vi_info.isp_top.blk_idle = ISP_RD_REG(isptop, REG_ISP_TOP_T, BLK_IDLE);
	for (i = 0; i <= 6; i++) {
		//Debug
		ISP_WR_BITS(isptop, REG_ISP_TOP_T, DUMMY, DBUS_SEL, i);
		vi_info.isp_top.dbus_sel[i].r_0 = ISP_RD_REG(isptop, REG_ISP_TOP_T, DBUS0); //0x0A070040
		vi_info.isp_top.dbus_sel[i].r_4 = ISP_RD_REG(isptop, REG_ISP_TOP_T, DBUS1); //0x0A070044
		vi_info.isp_top.dbus_sel[i].r_8 = ISP_RD_REG(isptop, REG_ISP_TOP_T, DBUS2); //0x0A070048
		vi_info.isp_top.dbus_sel[i].r_c = ISP_RD_REG(isptop, REG_ISP_TOP_T, DBUS3); //0x0A07004C
	}

	//pre_raw_fe
	vi_info.preraw_fe.preraw_info = ISP_RD_REG(preraw_fe, REG_PRE_RAW_FE_T, PRE_RAW_INFO);
	vi_info.preraw_fe.fe_idle_info = ISP_RD_REG(preraw_fe, REG_PRE_RAW_FE_T, FE_IDLE_INFO);

	vi_info.preraw_fe_1.preraw_info = ISP_RD_REG(preraw_fe_1, REG_PRE_RAW_FE_T, PRE_RAW_INFO);
	vi_info.preraw_fe_1.fe_idle_info = ISP_RD_REG(preraw_fe_1, REG_PRE_RAW_FE_T, FE_IDLE_INFO);

	//rawtop
	// vi_info.rawtop.stvalid_status = ISP_RD_REG(rawtop1, REG_RAW_TOP_T, STVALID_STATUS);
	// vi_info.rawtop.stready_status = ISP_RD_REG(rawtop1, REG_RAW_TOP_T, STREADY_STATUS);
	vi_info.rawtop.debug = ISP_RD_REG(rawtop1, REG_RAW_TOP1_T, RAW_TOP_DEBUG);
	vi_info.rawtop.dma_idle = ISP_RD_REG(rawtop1, REG_RAW_TOP1_T, RAW_TOP_DMA_IDLE);

	//rgbtop
	vi_info.rgbtop.xcnt_rpt = ISP_RD_BITS(rgbtop, REG_ISP_RGB_TOP_T, PATGEN_04, XCNT_RPT);
	vi_info.rgbtop.ycnt_rpt = ISP_RD_BITS(rgbtop, REG_ISP_RGB_TOP_T, PATGEN_04, YCNT_RPT);

	//yuvtop
	vi_info.yuvtop.debug_state = ISP_RD_REG(yuvtop, REG_YUV_TOP_T, YUV_DEBUG_STATE);
	vi_info.yuvtop.stvalid_status = ISP_RD_REG(yuvtop, REG_YUV_TOP_T, STVALID_STATUS);
	vi_info.yuvtop.stready_status = ISP_RD_REG(yuvtop, REG_YUV_TOP_T, STREADY_STATUS);
	vi_info.yuvtop.xcnt_rpt = ISP_RD_BITS(yuvtop, REG_YUV_TOP_T, PATGEN4, XCNT_RPT);
	vi_info.yuvtop.ycnt_rpt = ISP_RD_BITS(yuvtop, REG_YUV_TOP_T, PATGEN4, YCNT_RPT);

	printf("VIWdma1ErrStatus\t:0x%x\tVIWdma1IdleStatus\t:0x%x\n", data.wdma_1_err_sts, data.wdma_1_idle);
	printf("VIWdma2ErrStatus\t:0x%x\tVIWdma2IdleStatus\t:0x%x\n", data.wdma_2_err_sts, data.wdma_2_idle);
	printf("VIWdma3ErrStatus\t:0x%x\tVIWdma3IdleStatus\t:0x%x\n", data.wdma_3_err_sts, data.wdma_3_idle);
	printf("VIWdma4ErrStatus\t:0x%x\tVIWdma4IdleStatus\t:0x%x\n", data.wdma_4_err_sts, data.wdma_4_idle);
	printf("VIRdma1ErrStatus\t\t:0x%x\tVIRdma1IdleStatus\t:0x%x\n", data.rdma_1_err_sts, data.rdma_1_idle);
	printf("VIRdma2ErrStatus\t\t:0x%x\tVIRdma2IdleStatus\t:0x%x\n", data.rdma_2_err_sts, data.rdma_2_idle);
	printf("VIRdma3ErrStatus\t\t:0x%x\tVIRdma3IdleStatus\t:0x%x\n", data.rdma_3_err_sts, data.rdma_3_idle);

	printf("ISP_TOP, blk_idle(h38)=0x%x\n", vi_info.isp_top.blk_idle);
		for (i = 0; i <= 6; i++) {
			printf("dbus_sel=%d, r_0=0x%x, r_4=0x%x, r_8=0x%x, r_c=0x%x\n",
				i,
				vi_info.isp_top.dbus_sel[i].r_0,
				vi_info.isp_top.dbus_sel[i].r_4,
				vi_info.isp_top.dbus_sel[i].r_8,
				vi_info.isp_top.dbus_sel[i].r_c);
		}
	printf("PRE_RAW_FE0, preraw_info(h34)=0x%x, fe_idle_info(h50)=0x%x\n",
		vi_info.preraw_fe.preraw_info,
		vi_info.preraw_fe.fe_idle_info);
	printf("PRE_RAW_FE1, preraw_info(h34)0x%x, fe_idle_info(h50)=0x%x\n",
		vi_info.preraw_fe_1.preraw_info,
		vi_info.preraw_fe_1.fe_idle_info);
	printf("RAW_TOP, debug(h14)=0x%x, dma_idle(h24)=0x%x\n",
		vi_info.rawtop.debug, vi_info.rawtop.dma_idle);
	printf("RGB_TOP xcnt_rpt=0x%x, ycnt_rpt=0x%x\n",
		vi_info.rgbtop.xcnt_rpt,
		vi_info.rgbtop.ycnt_rpt);
	printf("YUV_TOP debug_state(h18)=0x%x, stvalid_status(h6c)=0x%x, stready_status(h70)=0x%x\n",
		vi_info.yuvtop.debug_state,
		vi_info.yuvtop.stvalid_status,
		vi_info.yuvtop.stready_status);
	printf("YUV_TOP xcnt_rpt=0x%x, ycnt_rpt=0x%x\n",
		vi_info.yuvtop.xcnt_rpt,
		vi_info.yuvtop.ycnt_rpt);
}

u32 isp_err_chk(
	struct cvi_vi_dev *vdev,
	struct isp_ctx *ctx,
	union REG_ISP_CSI_BDG_INTERRUPT_STATUS_0 *cbdg_0_sts,
	union REG_ISP_CSI_BDG_INTERRUPT_STATUS_1 *cbdg_1_sts)
{
	u32 ret = 0;
	enum cvi_isp_raw raw_num = ISP_PRERAW0;
	enum cvi_isp_pre_chn_num fe_chn = ISP_FE_CH0;

	for (raw_num = ISP_PRERAW0; raw_num < vdev->total_dev_num; raw_num++) {
		if (cbdg_1_sts[raw_num].bits.FIFO_OVERFLOW_INT) {
			vi_pr(VI_ERR, "CSIBDG_%d fifo overflow\n", raw_num);
			_vi_debug_info(ctx);
			ctx->isp_pipe_cfg[raw_num].dg_info.bdg_fifo_of_cnt++;
			vi_err_wake_up_th(vdev, raw_num);
			ret = -1;
		}

		if (cbdg_1_sts[raw_num].bits.FRAME_RESOLUTION_OVER_MAX_INT) {
			vi_pr(VI_ERR, "CSIBDG_%d frm size over max\n", raw_num);
			ret = -1;
		}

		if (cbdg_1_sts[raw_num].bits.DMA_ERROR_INT) {
			u32 wdma_1_err = ctx->isp_pipe_cfg[raw_num].dg_info.dma_sts.wdma_1_err_sts;
			u32 wdma_4_err = ctx->isp_pipe_cfg[raw_num].dg_info.dma_sts.wdma_4_err_sts;
			u32 rdma_err = ctx->isp_pipe_cfg[raw_num].dg_info.dma_sts.rdma_1_err_sts;
			u32 wdma_1_idle = ctx->isp_pipe_cfg[raw_num].dg_info.dma_sts.wdma_1_idle;
			u32 wdma_4_idle = ctx->isp_pipe_cfg[raw_num].dg_info.dma_sts.wdma_4_idle;
			u32 rdma_idle = ctx->isp_pipe_cfg[raw_num].dg_info.dma_sts.rdma_1_idle;

			if ((wdma_1_err & 0x10) || (wdma_4_err & 0x10) || (rdma_err & 0x10)) {
				vi_pr(VI_ERR, "DMA axi error wdma1_status(0x%x) wdma4_status(0x%x) rdma_status(0x%x)\n",
						wdma_1_err, wdma_4_err, rdma_err);
				ret = -1;
			} else if ((wdma_1_err & 0x20) || (wdma_4_err & 0x20)) {
				vi_pr(VI_ERR, "DMA size mismatch\n wdma1_status(0x%x) wdma4_status(0x%x) rdma_status(0x%x)\n",
						wdma_1_err, wdma_4_err, rdma_err);
				vi_pr(VI_ERR, "wdma1_idle(0x%x) wdma4_idle(0x%x) rdma_idle(0x%x)\n",
						wdma_1_idle, wdma_4_idle, rdma_idle);
				ret = -1;
			} else if ((wdma_1_err & 0x40) || (wdma_4_err & 0x40)) {
				vi_pr(VI_WARN, "WDMA buffer full\n");
			}
		}

		if (cbdg_0_sts[raw_num].bits.CH0_FRAME_WIDTH_GT_INT) {
			vi_pr(VI_ERR, "CSIBDG_%d CH%d frm width greater than setting(%d)\n",
					raw_num, fe_chn, ctx->isp_pipe_cfg[raw_num].csibdg_width);
			ctx->isp_pipe_cfg[raw_num].dg_info.bdg_w_gt_cnt[fe_chn]++;
			vi_err_wake_up_th(vdev, raw_num);
			ret = -1;
		}

		if (cbdg_0_sts[raw_num].bits.CH0_FRAME_WIDTH_LS_INT) {
			vi_pr(VI_ERR, "CSIBDG_%d CH%d frm width less than setting(%d)\n",
					raw_num, fe_chn, ctx->isp_pipe_cfg[raw_num].csibdg_width);
			ctx->isp_pipe_cfg[raw_num].dg_info.bdg_w_ls_cnt[fe_chn]++;
			vi_err_wake_up_th(vdev, raw_num);
			ret = -1;
		}

		if (cbdg_0_sts[raw_num].bits.CH0_FRAME_HEIGHT_GT_INT) {
			vi_pr(VI_ERR, "CSIBDG_%d CH%d frm height greater than setting(%d)\n",
					raw_num, fe_chn, ctx->isp_pipe_cfg[raw_num].csibdg_height);
			ctx->isp_pipe_cfg[raw_num].dg_info.bdg_h_gt_cnt[fe_chn]++;
			vi_err_wake_up_th(vdev, raw_num);
			ret = -1;
		}

		if (cbdg_0_sts[raw_num].bits.CH0_FRAME_HEIGHT_LS_INT) {
			vi_pr(VI_ERR, "CSIBDG_%d CH%d frm height less than setting(%d)\n",
					raw_num, fe_chn, ctx->isp_pipe_cfg[raw_num].csibdg_height);
			ctx->isp_pipe_cfg[raw_num].dg_info.bdg_h_ls_cnt[fe_chn]++;
			vi_err_wake_up_th(vdev, raw_num);
			ret = -1;
		}

		if (ctx->isp_pipe_cfg[raw_num].is_hdr_on) {
			fe_chn = ISP_FE_CH1;

			if (cbdg_0_sts[raw_num].bits.CH1_FRAME_WIDTH_GT_INT) {
				vi_pr(VI_ERR, "CSIBDG_%d CH%d frm width greater than setting(%d)\n",
						raw_num, fe_chn, ctx->isp_pipe_cfg[raw_num].csibdg_width);
				ctx->isp_pipe_cfg[raw_num].dg_info.bdg_w_gt_cnt[fe_chn]++;
				vi_err_wake_up_th(vdev, raw_num);
				ret = -1;
			}

			if (cbdg_0_sts[raw_num].bits.CH1_FRAME_WIDTH_LS_INT) {
				vi_pr(VI_ERR, "CSIBDG_%d CH%d frm width less than setting(%d)\n",
						raw_num, fe_chn, ctx->isp_pipe_cfg[raw_num].csibdg_width);
				ctx->isp_pipe_cfg[raw_num].dg_info.bdg_w_ls_cnt[fe_chn]++;
				vi_err_wake_up_th(vdev, raw_num);
				ret = -1;
			}

			if (cbdg_0_sts[raw_num].bits.CH1_FRAME_HEIGHT_GT_INT) {
				vi_pr(VI_ERR, "CSIBDG_%d CH%d frm height greater than setting(%d)\n",
						raw_num, fe_chn, ctx->isp_pipe_cfg[raw_num].csibdg_height);
				ctx->isp_pipe_cfg[raw_num].dg_info.bdg_h_gt_cnt[fe_chn]++;
				vi_err_wake_up_th(vdev, raw_num);
				ret = -1;
			}

			if (cbdg_0_sts[raw_num].bits.CH1_FRAME_HEIGHT_LS_INT) {
				vi_pr(VI_ERR, "CSIBDG_%d CH%d frm height less than setting(%d)\n",
						raw_num, fe_chn, ctx->isp_pipe_cfg[raw_num].csibdg_height);
				ctx->isp_pipe_cfg[raw_num].dg_info.bdg_h_ls_cnt[fe_chn]++;
				vi_err_wake_up_th(vdev, raw_num);
				ret = -1;
			}
		}
	}
	return ret;
}

static void _isp_sof_handler(struct cvi_vi_dev *vdev, const enum cvi_isp_raw raw_num, const enum cvi_isp_pre_chn_num chn_num)
{
	// u64 now = timer_get_tick();

	// vi_pr(VI_DBG, "raw[%d] %ld\n", raw_num, now/TIMER_CNT_MS);

#ifdef TEST_PERF
	vdev->isp_timer.pre_fe_sof_time[raw_num][chn_num] = now;
	vdev->isp_timer.pre_fe_sof_time_sum[raw_num][chn_num] += now;
#endif
}

static inline void _isp_pre_fe_done_handler(
	struct cvi_vi_dev *vdev,
	const enum cvi_isp_raw raw_num,
	const enum cvi_isp_pre_chn_num chn_num)
{
	struct isp_ctx *ctx = &vdev->ctx;
	u32 trigger = false;

	if (ctx->is_rtl_config) {
		vi_pr(VI_DBG, "[%ld] pre_fe_%d frm_done chn_num=%d frm_num=%d\n",
			timer_get_tick()/TIMER_CNT_MS, raw_num, chn_num, vdev->pre_fe_frm_num[raw_num][chn_num]);
		return;
	}

	if (ctx->isp_pipe_cfg[raw_num].is_yuv_bypass_path) {
		if (ctx->isp_pipe_cfg[raw_num].is_offline_scaler) { //offline mode
			vi_pr(VI_DBG, "pre_fe_%d yuv offline done chn_num=%d frm_num=%d\n",
					raw_num, chn_num, vdev->pre_fe_frm_num[raw_num][chn_num]);
			// _isp_yuv_bypass_handler(vdev, raw_num, chn_num);
		} else { //YUV sensor online mode
			vi_pr(VI_DBG, "pre_fe_%d yuv online done chn_num=%d frm_num=%d\n",
					raw_num, chn_num, vdev->pre_fe_frm_num[raw_num][chn_num]);
			// _isp_yuv_online_handler(vdev, raw_num, chn_num);
		}
		return;
	}

	vi_pr(VI_PERF, "[%ld] pre_fe_%d frm_done chn_num=%d frm_num=%d\n",
			timer_get_tick()/TIMER_CNT_MS, raw_num, chn_num, vdev->pre_fe_frm_num[raw_num][chn_num]);

	if (_is_fe_post_offline(ctx)) { //fe->dram->post
		if (!ctx->is_slice_buf_on) {
			struct isp_buffer *b = NULL;
			struct isp_queue *fe_out_q = (chn_num == ISP_FE_CH0) ?
							&pre_out_queue[raw_num] : &pre_out_se_queue[raw_num];
			struct isp_queue *post_in_q = (chn_num == ISP_FE_CH0) ? &post_in_queue : &post_in_se_queue;

			b = isp_buf_remove(fe_out_q);
			if (b == NULL) {
				vi_pr(VI_ERR, "Pre_fe_%d chn_num_%d outbuf is empty\n", raw_num, chn_num);
				return;
			}

			b->is_yuv_frm	= 0;
			b->chn_num	= raw_num;

			isp_buf_queue(post_in_q, b);
			vi_pr(VI_DBG, "chn_num=%d 0x%lx=[%x]\n", chn_num, b->addr, readl(b->addr));
		}

		if (ctx->isp_pipe_cfg[raw_num].is_hdr_on) {
			trigger = (vdev->pre_fe_frm_num[raw_num][ISP_FE_CH0] ==
					vdev->pre_fe_frm_num[raw_num][ISP_FE_CH1]);
		} else
			trigger = true;


		if (trigger) {
			//NOTICE trigger postraw
			// //vi_pr(VI_DBG, "fe->dram->post trigger raw_num=%d\n", raw_num);
			// if (atomic_read(&vdev->isp_raw_dump_en[raw_num]) == 3) { //raw dump flow
			// 	_isp_raw_dump_chk(vdev, raw_num, b->frm_num);
			// } else {
			// 	tasklet_hi_schedule(&vdev->job_work);
			// }
			// _vi_wake_up_vblank_th(vdev, raw_num);
			_post_hw_enque(vdev);
		}
	}

	#ifdef TEST_PERF
	vdev->isp_timer.pre_fe_done_time[raw_num][chn_num] = timer_get_tick();
	vdev->isp_timer.pre_sof2fe_done_frm_time[raw_num][chn_num] += vdev->isp_timer.pre_fe_done_time[raw_num][chn_num] - \
									vdev->isp_timer.pre_fe_sof_time[raw_num][chn_num];
	vdev->isp_timer.pre_fe_done_time_sum[raw_num][chn_num] += vdev->isp_timer.pre_fe_done_time[raw_num][chn_num];
	// uartlog("[debug] fe done chn=%d time=%d sof->fe=%d\n", chn_num, vdev->isp_timer.pre_fe_done_time[raw_num][chn_num]/TIMER_CNT_US, vdev->isp_timer.pre_sof2fe_done_frm_time[raw_num][chn_num]/TIMER_CNT_US);
#endif
}

static void _isp_postraw_shaw_done_handler(struct cvi_vi_dev *vdev)
{
	if (vdev->ctx.is_slice_buf_on) {
		vi_pr(VI_INFO, "postraw shaw done\n");
	}
}

static void _isp_postraw_done_handler(struct cvi_vi_dev *vdev)
{
	struct isp_ctx *ctx = &vdev->ctx;
	enum cvi_isp_raw raw_num = ISP_PRERAW0;

	vi_pr(VI_INFO, "+\n");

#ifdef TEST_PERF
	vdev->isp_timer.post_done_time[raw_num] = timer_get_tick();
	vdev->isp_timer.trigpost2post_done_frm_time[raw_num] += vdev->isp_timer.post_done_time[raw_num] - \
								vdev->isp_timer.pre_trigpost_time[raw_num];
	if (ctx->isp_pipe_cfg[raw_num].is_hdr_on){
		vdev->isp_timer.pre_fe_done2post_done_frm_time[raw_num] += vdev->isp_timer.post_done_time[raw_num] - \
									   vdev->isp_timer.pre_fe_done_time[raw_num][ISP_FE_CH1];
	}
	else{
		vdev->isp_timer.pre_fe_done2post_done_frm_time[raw_num] += vdev->isp_timer.post_done_time[raw_num] - \
									   vdev->isp_timer.pre_fe_done_time[raw_num][ISP_FE_CH0];
	}
	vdev->isp_timer.post_done_time_sum[raw_num] += vdev->isp_timer.post_done_time[raw_num];
#endif
#if 0
	if (vdev->postraw_frame_number[raw_num] % 10 == 0)
		vi_pr(VI_PERF, "Post done frames: %d\n", vdev->postraw_frame_number[raw_num]);

	if (vdev->postraw_frame_number[raw_num]+1 >= 0){
		vi_pr(VI_PERF, "===============================PERF===============================\n");
		vi_pr(VI_PERF, "Avg sof ch0: frm_num=%d  tot_time:%d us  avg_time:%d us\n", \
						vdev->pre_fe_frm_num[raw_num][0], \
						vdev->isp_timer.pre_fe_sof_time_sum[raw_num][0]/TIMER_CNT_US, \
						(vdev->isp_timer.pre_fe_sof_time_sum[raw_num][0]/TIMER_CNT_US)/vdev->pre_fe_frm_num[raw_num][0]);
		vi_pr(VI_PERF, "Avg sof ch1: frm_num=%d  tot_time:%d us  avg_time:%d us\n", \
						vdev->pre_fe_frm_num[raw_num][1], \
						vdev->isp_timer.pre_fe_sof_time_sum[raw_num][1]/TIMER_CNT_US, \
						(vdev->isp_timer.pre_fe_sof_time_sum[raw_num][1]/TIMER_CNT_US)/vdev->pre_fe_frm_num[raw_num][1]);

		vi_pr(VI_PERF, "Avg fe ch0: frm_num=%d  tot_time:%d us  avg_time:%d us\n", \
						vdev->pre_fe_frm_num[raw_num][0], \
						vdev->isp_timer.pre_fe_done_time_sum[raw_num][0]/TIMER_CNT_US, \
						(vdev->isp_timer.pre_fe_done_time_sum[raw_num][0]/TIMER_CNT_US)/vdev->pre_fe_frm_num[raw_num][0]);
		vi_pr(VI_PERF, "Avg fe ch1: frm_num=%d  tot_time:%d us  avg_time:%d us\n", \
						vdev->pre_fe_frm_num[raw_num][1], \
						vdev->isp_timer.pre_fe_done_time_sum[raw_num][1]/TIMER_CNT_US, \
						(vdev->isp_timer.pre_fe_done_time_sum[raw_num][1]/TIMER_CNT_US)/vdev->pre_fe_frm_num[raw_num][1]);

		vi_pr(VI_PERF, "Avg post: frm_num=%d  tot_time:%d us  avg_time:%d us\n", \
						vdev->postraw_frame_number[raw_num]+1, \
						vdev->isp_timer.post_done_time_sum[raw_num]/TIMER_CNT_US, \
						(vdev->isp_timer.post_done_time_sum[raw_num]/TIMER_CNT_US)/(vdev->postraw_frame_number[raw_num]+1));

		vi_pr(VI_PERF, "- - - - - - - - - - - - - - - - - - - - - - - - - - - - - \n");
		vi_pr(VI_PERF, "sof cnt: chn0=%d   chn1=%d\n", vdev->pre_fe_sof_cnt[ISP_PRERAW0][ISP_FE_CH0], \
						vdev->pre_fe_sof_cnt[ISP_PRERAW0][ISP_FE_CH1]);
		vi_pr(VI_PERF, "sof2fe fe0 ch0 : frm_num=%d  tot_time:%d us  avg_time:%d us\n", \
						vdev->pre_fe_frm_num[raw_num][0], \
						vdev->isp_timer.pre_sof2fe_done_frm_time[raw_num][0]/TIMER_CNT_US,\
						(vdev->isp_timer.pre_sof2fe_done_frm_time[raw_num][0]/TIMER_CNT_US)/vdev->pre_fe_frm_num[raw_num][0]);
		vi_pr(VI_PERF, "sof2fe fe0 ch1 : frm_num=%d  tot_time:%d us  avg_time:%d us\n", \
						vdev->pre_fe_frm_num[raw_num][1], \
						vdev->isp_timer.pre_sof2fe_done_frm_time[raw_num][1]/TIMER_CNT_US,\
						(vdev->isp_timer.pre_sof2fe_done_frm_time[raw_num][1]/TIMER_CNT_US)/vdev->pre_fe_frm_num[raw_num][1]);

		vi_pr(VI_PERF, "fe->post : frm_num=%d  tot_time:%d us  avg_time:%d us\n", \
						vdev->postraw_frame_number[raw_num]+1, \
						vdev->isp_timer.pre_fe_done2post_done_frm_time[raw_num]/TIMER_CNT_US,\
						(vdev->isp_timer.pre_fe_done2post_done_frm_time[raw_num]/TIMER_CNT_US)/(vdev->postraw_frame_number[raw_num]+1));

		vi_pr(VI_PERF, "trig_post->post done: frm_num=%d  tot_time:%d us  avg_time:%d us\n", \
						vdev->postraw_frame_number[raw_num]+1, \
						vdev->isp_timer.trigpost2post_done_frm_time[raw_num]/TIMER_CNT_US,\
						(vdev->isp_timer.trigpost2post_done_frm_time[raw_num]/TIMER_CNT_US)/(vdev->postraw_frame_number[raw_num]+1));

		vi_pr(VI_PERF, "=====================================================================\n");
	}
#endif
	if (ctx->is_rtl_config) {
		++vdev->postraw_frame_number[raw_num];
		// vi_pr(VI_DBG, "[%ld] Postraw_%d frm_done frm_num=%d\n", timer_get_tick()/TIMER_CNT_MS,
		// 		raw_num, vdev->postraw_frame_number[raw_num]);
		// uartlog("=========for rtl tnr swith rgb buffer=========\n");
		// if (vdev->postraw_frame_number[raw_num]%2==1){
		// 	uartlog("cur->buf1, pre->buf0\n");
		// 	writel(0x0a002204, 0x4006000);
		// 	writel(0x0a002504, 0x4006300);
		// 	writel(0x0a053404, 0x4006000);
		// 	writel(0x0a053504, 0x4006300);
		// 	writel(0x0a053204, 0x2400000);
		// 	writel(0x0a053304, 0x2500000);

		// 	uartlog("1.Now dump output[00%d] addr: 0x83000000, size: 0x3000\n", vdev->postraw_frame_number[raw_num]-1);
		// 	uartlog("2.Now dump rgb le[00%d] addr: 0x82400000, size: 0x300 \n", vdev->postraw_frame_number[raw_num]-1);
		// 	uartlog("3.Now dump rgb se[00%d] addr: 0x82500000, size: 0x300 \n", vdev->postraw_frame_number[raw_num]-1);
		// 	uartlog("4.Now dump mmap  [00%d] addr: 0x82800000, size: 0x100 \n", vdev->postraw_frame_number[raw_num]-1);
		// 	uartlog("5.Now load pic  [00%d] addr:  0x84000000 & 0x84003000 \n", vdev->postraw_frame_number[raw_num]);
		// 	uartlog("6.Now trig <PLD need:wm 0x0a02893c 0x1>\n");

		// }
		// else{
		// 	uartlog("cur->buf0, pre->buf1\n");
		// 	writel(0x0a002204, 0x2400000);
		// 	writel(0x0a002504, 0x2500000);
		// 	writel(0x0a053404, 0x2400000);
		// 	writel(0x0a053504, 0x2500000);
		// 	writel(0x0a053204, 0x4006000);
		// 	writel(0x0a053304, 0x4006300);

		// 	uartlog("1.Now dump output[00%d] addr: 0x83000000, size: 0x3000\n", vdev->postraw_frame_number[raw_num]-1);
		// 	uartlog("2.Now dump rgb le[00%d] addr: 0x84006000, size: 0x300 \n", vdev->postraw_frame_number[raw_num]-1);
		// 	uartlog("3.Now dump rgb se[00%d] addr: 0x84006300, size: 0x300 \n", vdev->postraw_frame_number[raw_num]-1);
		// 	uartlog("4.Now dump mmap  [00%d] addr: 0x82800000, size: 0x100 \n", vdev->postraw_frame_number[raw_num]-1);
		// 	uartlog("5.Now load pic   [00%d] addr: 0x84000000 & 0x84003000 \n", vdev->postraw_frame_number[raw_num]);
		// 	uartlog("6.Now trig <PLD need:wm 0x0a02893c 0x1>\n");
		// }
		vi_pr(VI_PERF, "[%ld] Postraw_%d frm_done frm_num=%d\n", timer_get_tick()/TIMER_CNT_MS, raw_num, vdev->postraw_frame_number[raw_num]);
		{
			uint64_t y_addr, c_addr;

			y_addr = ispblk_dma_getaddr(ctx, ISP_BLK_ID_DMA_CTL_YUV_CROP_Y);
			c_addr = ispblk_dma_getaddr(ctx, ISP_BLK_ID_DMA_CTL_YUV_CROP_C);
			vi_pr(VI_DBG, "dump: %#"PRIx64"-%#"PRIx64" 0x:%x\n", y_addr, c_addr, readl(y_addr));
		}
		return;
	}

	raw_num = ctx->cam_id;

	if (_is_fe_post_offline(ctx) && !ctx->is_slice_buf_on) {
		struct isp_buffer *b = NULL;
		struct isp_queue *post_in_q = &post_in_queue;
		struct isp_queue *pre_out_q = NULL;
		u8 chn_num = 0;

		b = isp_buf_remove(post_in_q);
		if (b == NULL) {
			vi_pr(VI_ERR, "post_in_queue is empty\n");
			return;
		}
		if (b->chn_num >= ISP_CHN_MAX) {
			vi_pr(VI_ERR, "buf chn_num_%d is wrong\n", b->chn_num);
			return;
		}
		chn_num = b->chn_num;

		pre_out_q = &pre_out_queue[chn_num];
		isp_buf_queue(pre_out_q, b);

		if (ctx->isp_pipe_cfg[raw_num].is_hdr_on) {
			post_in_q = &post_in_se_queue;
			b = isp_buf_remove(post_in_q);
			if (b == NULL) {
				vi_pr(VI_ERR, "post_in_se_queue is empty\n");
				return;
			}
			pre_out_q = &pre_out_se_queue[chn_num];
			isp_buf_queue(pre_out_q, b);
		}
	} else {
		//Update postraw stt gms/ae/hist_edge_v dma size/addr
		_swap_post_sts_buf(ctx, raw_num);

		// //Change post done flag to be true
		// if (ctx->is_slice_buf_on)
		// 	atomic_set(&vdev->ctx.is_post_done, 1);
		vdev->ctx.is_post_done = 1;
	}

	++vdev->postraw_frame_number[raw_num];
	++ctx->isp_pipe_cfg[raw_num].first_frm_cnt;

	// --------------for debug pld wave------------------------
	// writel(0x0a060088, 0x30); //0x10
	// u32 val = readl(0x0a069014);
	// uartlog("read 0x0a069014=[0x%x] write 0x%x\n", val, val|0x2);
	// writel(0x0a069014, val|0x2);
	// val = readl(0x0a06A014);
	// uartlog("read 0x0a06A014=[0x%x] write 0x%x\n", val, val|0x2);
	// writel(0x0a06A014, val|0x2);

	// writel(0x0a06008c, 0x437077f);
	// writel(0x0a060098, 0x43703bf);
	// writel(0x0a061100, 0x10000);
	// writel(0x0a05306c, 0xaa40);
	// writel(0x0a053804, 0x469e510);

	// writel(0x0a053000, 0x49);
	// writel(0x0a053000, 0x10000049); //只打开inter4_en，正常
	// writel(0x0a053000, 0x2000009); //只打开inter1_en，且tap en设置为0
	// writel(0x0a053000, 0x2000049); //只打开inter1_en，异常
	// writel(0x0a053000, 0x18000049); //打开inter3_en+inter4_en  正常
	// writel(0x0a053000, 0x1c000049); //打开inter2_en+inter3_en+inter4_en 正常
	// writel(0x0a053000, 0x1e000049);// //打开inter1_en+inter2_en+inter3_en+inter4_en 异常
	// _pre_hw_enque(vdev, raw_num, ISP_FE_CH0);
	// uartlog("trig_pre ttt...\n");
	// ----------------------------------------------
	vi_pr(VI_PERF, "[%ld] Postraw_%d frm_done frm_num=%d\n", timer_get_tick()/TIMER_CNT_MS, raw_num, vdev->postraw_frame_number[raw_num]);

	{
		uint64_t y_addr, c_addr;

		y_addr = ispblk_dma_getaddr(ctx, ISP_BLK_ID_DMA_CTL_YUV_CROP_Y);
		c_addr = ispblk_dma_getaddr(ctx, ISP_BLK_ID_DMA_CTL_YUV_CROP_C);
		vi_pr(VI_DBG, "dump: %#"PRIx64"-%#"PRIx64" 0x:%x\n", y_addr, c_addr, readl(y_addr));
	}
	if (ctx->isp_pipe_cfg[raw_num].is_offline_scaler) {
		//TODO change postraw_done buffer
		// cvi_isp_rdy_buf_remove(vdev, raw_num);

		// cvi_isp_dqbuf_list(vdev, vdev->postraw_frame_number[raw_num], raw_num);
	}

	//TODO maybe rertigger
	//tasklet_hi_schedule(&vdev->job_work);

	// if (!ctx->is_slice_buf_on &&
	//     !(ctx->isp_pipe_cfg[raw_num].is_hdr_on)) {
	// 	_pre_hw_enque(vdev, raw_num, ISP_FE_CH0);
	// 	if (ctx->isp_pipe_cfg[raw_num].is_hdr_on)
	// 		_pre_hw_enque(vdev, raw_num, ISP_FE_CH1);
	// }

#ifdef TEST_PERF
	// Retrigger
	if (!ctx->is_slice_buf_on) {
		_pre_hw_enque(vdev, raw_num, ISP_FE_CH0);
		if (ctx->isp_pipe_cfg[raw_num].is_hdr_on)
			_pre_hw_enque(vdev, raw_num, ISP_FE_CH1);
	}
#endif

	// if (vdev->postraw_frame_number[raw_num]==2){
	// 	//第三帧开始trig
	// 	writel(0x0a060088, 0x37); //0x10
	// 	u32 val = readl(0x0a069014);
	// 	uartlog("read 0x0a069014=[0x%x] write 0x%x\n", val, val&0x1);
	// 	writel(0x0a069014, val&0x1);
	// 	val = readl(0x0a06A014);
	// 	uartlog("read 0x0a06A014=[0x%x] write 0x%x\n", val, val&0x1);
	// 	writel(0x0a06A014, val|0x1);
	// 	// _pre_hw_enque(vdev, raw_num, ISP_FE_CH0);
	// 	uartlog("trig_pre frame3...\n");
	// }
	// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
}

static void sof_chk(struct cvi_vi_dev *vdev,
		union REG_ISP_TOP_INT_EVENT2 top_sts_2)
{
	/* pre_fe0 ch0 frame start */
	if (top_sts_2.bits.FRAME_START_FE0 & 0x1) {
		//TODO perf
		//vi_record_sof_perf(vdev, ISP_PRERAW0, ISP_FE_CH0);

		++vdev->pre_fe_sof_cnt[ISP_PRERAW0][ISP_FE_CH0];

		vi_pr(VI_PERF, "[%ld] pre_fe_%d sof chn_num=%d frm_num=%d\n",
				timer_get_tick()/TIMER_CNT_MS, ISP_PRERAW0, ISP_FE_CH0, vdev->pre_fe_sof_cnt[ISP_PRERAW0][ISP_FE_CH0]);

		if (!vdev->ctx.isp_pipe_cfg[ISP_PRERAW0].is_yuv_bypass_path) //RGB sensor
			_isp_sof_handler(vdev, ISP_PRERAW0, ISP_FE_CH0);
	}

	/* pre_fe0 ch1 frame start */
	if (top_sts_2.bits.FRAME_START_FE0 & 0x2) {
		++vdev->pre_fe_sof_cnt[ISP_PRERAW0][ISP_FE_CH1];

		_isp_sof_handler(vdev, ISP_PRERAW0, ISP_FE_CH1);
	}

	/* pre_fe1 ch0 frame start */
	if (top_sts_2.bits.FRAME_START_FE1 & 0x1) {
		++vdev->pre_fe_sof_cnt[ISP_PRERAW1][ISP_FE_CH0];

		vi_pr(VI_INFO, "[%ld] pre_fe_%d sof chn_num=%d frm_num=%d\n",
				timer_get_tick()/TIMER_CNT_MS, ISP_PRERAW1, ISP_FE_CH0, vdev->pre_fe_sof_cnt[ISP_PRERAW1][ISP_FE_CH0]);

		if (!vdev->ctx.isp_pipe_cfg[ISP_PRERAW1].is_yuv_bypass_path) { //RGB sensor
			_isp_sof_handler(vdev, ISP_PRERAW1, ISP_FE_CH0);
		}
	}

	/* pre_fe1 ch1 frame start */
	if (top_sts_2.bits.FRAME_START_FE1 & 0x2) {
		++vdev->pre_fe_sof_cnt[ISP_PRERAW1][ISP_FE_CH1];

		//_isp_sof_handler(vdev, ISP_PRERAW1, ISP_FE_CH1);
	}

	/* pre_fe2 ch0 frame start */
	if (top_sts_2.bits.FRAME_START_FE2 & 0x1) {
		++vdev->pre_fe_sof_cnt[ISP_PRERAW2][ISP_FE_CH0];
		 vi_pr(VI_INFO, "pre_fe_%d sof chn_num=%d frm_num=%d\n",
				ISP_PRERAW2, ISP_FE_CH0, vdev->pre_fe_sof_cnt[ISP_PRERAW2][ISP_FE_CH0]);
		// _isp_sof_handler(vdev, ISP_PRERAW2, ISP_FE_CH0);
	}

	/* pre_fe2 ch1 frame start */
	if (top_sts_2.bits.FRAME_START_FE1 & 0x2) {
		++vdev->pre_fe_sof_cnt[ISP_PRERAW2][ISP_FE_CH1];

		//_isp_sof_handler(vdev, ISP_PRERAW2, ISP_FE_CH1);
	}
}

static void fe_done_chk(struct cvi_vi_dev *vdev,
		union REG_ISP_TOP_INT_EVENT0 top_sts_0)
{
	struct isp_ctx *ctx = &vdev->ctx;

	/* pre_fe0 ch0 frm_done */
	if (top_sts_0.bits.FRAME_DONE_FE0 & 0x1) {
		//TODO perf
		//vi_record_fe_perf(vdev, ISP_PRERAW0, ISP_FE_CH0);

		// In synthetic HDR mode, we assume that the first SOF is long exposure frames,
		// and the second SOF is short exposure frames.
		if (ctx->isp_pipe_cfg[ISP_PRERAW0].is_hdr_on) {
			if (vdev->pre_fe_frm_num[ISP_PRERAW0][ISP_FE_CH0] ==
			    vdev->pre_fe_frm_num[ISP_PRERAW0][ISP_FE_CH1]) { // LE
				++vdev->pre_fe_frm_num[ISP_PRERAW0][ISP_FE_CH0];
				_isp_pre_fe_done_handler(vdev, ISP_PRERAW0, ISP_FE_CH0);
			} else { // SE
				++vdev->pre_fe_frm_num[ISP_PRERAW0][ISP_FE_CH1];

				_isp_pre_fe_done_handler(vdev, ISP_PRERAW0, ISP_FE_CH1);
			}
		} else {
			++vdev->pre_fe_frm_num[ISP_PRERAW0][ISP_FE_CH0];
			_isp_pre_fe_done_handler(vdev, ISP_PRERAW0, ISP_FE_CH0);
		}
	}

	/* pre_fe0 ch1 frm_done */
	if (top_sts_0.bits.FRAME_DONE_FE0 & 0x2) {
		++vdev->pre_fe_frm_num[ISP_PRERAW0][ISP_FE_CH1];
		_isp_pre_fe_done_handler(vdev, ISP_PRERAW0, ISP_FE_CH1);
	}

	/* pre_fe0 ch2 frm_done */
	if (top_sts_0.bits.FRAME_DONE_FE0 & 0x4) {
		++vdev->pre_fe_frm_num[ISP_PRERAW0][ISP_FE_CH2];
		_isp_pre_fe_done_handler(vdev, ISP_PRERAW0, ISP_FE_CH2);
	}

	/* pre_fe0 ch3 frm_done */
	if (top_sts_0.bits.FRAME_DONE_FE0 & 0x8) {
		++vdev->pre_fe_frm_num[ISP_PRERAW0][ISP_FE_CH3];
		_isp_pre_fe_done_handler(vdev, ISP_PRERAW0, ISP_FE_CH3);
	}

	/* pre_fe1 ch0 frm_done */
	if (top_sts_0.bits.FRAME_DONE_FE1 & 0x1) {
		++vdev->pre_fe_frm_num[ISP_PRERAW1][ISP_FE_CH0];
		_isp_pre_fe_done_handler(vdev, ISP_PRERAW1, ISP_FE_CH0);
	}

	/* pre_fe1 ch1 frm_done */
	if (top_sts_0.bits.FRAME_DONE_FE1 & 0x2) {
		++vdev->pre_fe_frm_num[ISP_PRERAW1][ISP_FE_CH1];
		_isp_pre_fe_done_handler(vdev, ISP_PRERAW1, ISP_FE_CH1);
	}

	/* pre_fe2 ch0 frm_done */
	if (top_sts_0.bits.FRAME_DONE_FE2 & 0x1) {
		++vdev->pre_fe_frm_num[ISP_PRERAW2][ISP_FE_CH0];
		_isp_pre_fe_done_handler(vdev, ISP_PRERAW2, ISP_FE_CH0);
	}

	/* pre_fe2 ch1 frm_done */
	if (top_sts_0.bits.FRAME_DONE_FE2 & 0x2) {
		++vdev->pre_fe_frm_num[ISP_PRERAW2][ISP_FE_CH1];
		_isp_pre_fe_done_handler(vdev, ISP_PRERAW2, ISP_FE_CH1);
	}
}

void vi_irq_handler(struct cvi_vi_dev *vdev)
{
	struct isp_ctx *ctx = &vdev->ctx;
	union REG_ISP_CSI_BDG_INTERRUPT_STATUS_0 cbdg_0_sts[ISP_PRERAW_MAX] = { 0 };
	union REG_ISP_CSI_BDG_INTERRUPT_STATUS_1 cbdg_1_sts[ISP_PRERAW_MAX] = { 0 };

	union REG_ISP_TOP_INT_EVENT0 top_sts_0;
	union REG_ISP_TOP_INT_EVENT1 top_sts_1;
	union REG_ISP_TOP_INT_EVENT2 top_sts_2;
	u8 i = 0, raw_num = ISP_PRERAW0;

	isp_intr_status(ctx, &top_sts_0, &top_sts_1, &top_sts_2);

	//TODO perf
	//vi_perf_record_dump();

	for (raw_num = ISP_PRERAW0; raw_num < vdev->total_dev_num; raw_num++) {
		isp_csi_intr_status(ctx, raw_num, &cbdg_0_sts[raw_num], &cbdg_1_sts[raw_num]);
	}

	for (raw_num = ISP_PRERAW0; raw_num < vdev->total_dev_num; raw_num++) {

		ctx->isp_pipe_cfg[raw_num].dg_info.bdg_int_sts_0 = cbdg_0_sts[raw_num].raw;
		ctx->isp_pipe_cfg[raw_num].dg_info.bdg_int_sts_1 = cbdg_1_sts[raw_num].raw;

		ctx->isp_pipe_cfg[raw_num].dg_info.fe_sts = ispblk_fe_dbg_info(ctx, raw_num);
		if (raw_num == ISP_PRERAW0) {
			ctx->isp_pipe_cfg[raw_num].dg_info.post_sts = ispblk_post_dbg_info(ctx);
			ctx->isp_pipe_cfg[raw_num].dg_info.dma_sts = ispblk_dma_dbg_info(ctx);
		}

		for (i = 0; i < ISP_FE_CHN_MAX; i++)
			ctx->isp_pipe_cfg[raw_num].dg_info.bdg_chn_debug[i] = ispblk_csibdg_chn_dbg(ctx, raw_num, i);
	}

	if (isp_err_chk(vdev, ctx, cbdg_0_sts, cbdg_1_sts) == -1)
		return;

	sof_chk(vdev, top_sts_2);
	//if (top_sts.bits.INT_DMA_ERR)
	//	vi_pr(VI_ERR, "DMA error\n");


	//HW limit
	//On hdr and stagger vsync mode, need to re-trigger pq vld after ch0 pq done.
	/* pre_fe0 ch0 pq done */
	if (top_sts_1.bits.PQ_DONE_FE0 & 0x1) {
		if (ctx->isp_pipe_cfg[ISP_PRERAW0].is_hdr_on) {
			ISP_WR_BITS(ctx->phys_regs[ISP_BLK_ID_PRE_RAW_FE0],
				REG_PRE_RAW_FE_T, PRE_RAW_FRAME_VLD, FE_PQ_VLD_CH1, 1);
		}
	}

	/* pre_fe1 ch0 pq done */
	if (top_sts_1.bits.PQ_DONE_FE1 & 0x1) {
		if (ctx->isp_pipe_cfg[ISP_PRERAW1].is_hdr_on) {
			ISP_WR_BITS(ctx->phys_regs[ISP_BLK_ID_PRE_RAW_FE1],
				REG_PRE_RAW_FE_T, PRE_RAW_FRAME_VLD, FE_PQ_VLD_CH1, 1);
		}
	}

	if (!ctx->isp_pipe_cfg[ISP_PRERAW0].is_raw_replay)
		fe_done_chk(vdev, top_sts_0);

	/* pre_fe0 ch0 shaw done */
	if (top_sts_0.bits.SHAW_DONE_FE0 & 0x1) {
		vi_pr(VI_DBG, "pre_fe0 ch0 shaw done\n");
	}

	/* post shadow up done */
	if (top_sts_0.bits.SHAW_DONE_POST) {
		if (ctx->is_slice_buf_on) {
			_isp_postraw_shaw_done_handler(vdev);
		}
	}

	/* post frm done */
	if (top_sts_0.bits.FRAME_DONE_POST)
		_isp_postraw_done_handler(vdev);
}

uint64_t PTN_MALLOC(size_t size)
{
	uint64_t addr;
	addr = g_dev.mem_base + g_dev.alloc_size;
	g_dev.alloc_size = g_dev.alloc_size + size;
	g_dev.alloc_size = ((g_dev.alloc_size + 255) & ~255);
	g_dev.alloc_cnt += 1;

	// vi_pr(VI_INFO, "addr:0x%lx size:0x%x total_used:0x%x\n",
	// 	addr, (unsigned int)size, g_dev.alloc_size);
	return addr;
}

void PIN_Clear_All()
{
	g_dev.alloc_size = 0;
	g_dev.alloc_cnt = 0;
}

int vi_destroy_instance(struct cvi_vi_dev *vdev)
{
	vi_stop_streaming(vdev);
	PIN_Clear_All();
	vi_tuning_buf_release();
	return 0;
}

int vi_create_instance(struct cvi_vi_dev *vdev)
{
	int ret = 0;
	uint64_t mempool_base = 0;
	uint32_t mempool_size = 0;

	vdev->mem_base = DRAM_PHY_BASE;

	if (vi_max_size || vdev->total_dev_num > 2) {
		mempool_size = 1024*1024*70;
#ifdef TEST_PERF
		mempool_size = 1024*1024*300;
#endif
	} else {
		mempool_size = 1024*1024*100;
#ifdef TEST_PERF
		mempool_size = 1024*1024*100;
#endif
	}


	// vi_pr(VI_INFO, "membase = 0x%lx\n", vdev->mem_base);

	_vi_init_param(vdev);
	_vi_sw_init(vdev);

	ret = _vi_mempool_setup();
	if (ret) {
		vi_pr(VI_ERR, "Failed to setup isp memory\n");
		goto err;
	}
	mempool_base = PTN_MALLOC(mempool_size);
	mempool_base = ISP_ALIGN(mempool_base, 0x1000);

	vi_mempool_set(mempool_base, mempool_size);


	// vi_pr(VI_INFO, "mempool_base = 0x%lx, size=0x%x\n", mempool_base, mempool_size);

	vi_start_streaming(vdev);
err:
	return ret;
}