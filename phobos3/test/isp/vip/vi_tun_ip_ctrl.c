#include <vip/vi_drv.h>
#include <vi_defines.h>

#define BE_RUNTIME_TUN(_name) \
	{\
		struct cvi_vip_isp_##_name##_config *cfg;\
		cfg = &be_tun->_name##_cfg;\
		ispblk_##_name##_tun_cfg(ctx, cfg, raw_num);\
	}

#define POST_RUNTIME_TUN(_name) \
	{\
		struct cvi_vip_isp_##_name##_config *cfg;\
		cfg = &post_tun->_name##_cfg;\
		ispblk_##_name##_tun_cfg(ctx, cfg, raw_num);\
	}

#define POST_RUNTIME_TUN2(_name, _path) \
	{\
		struct cvi_vip_isp_##_name##_config *cfg;\
		cfg = &post_tun->_name##_cfg;\
		ispblk_##_name##_tun_cfg(ctx, cfg, raw_num, _path);\
	}

/****************************************************************************
 * Global parameters
 ****************************************************************************/
extern int tuning_dis[4];

struct isp_tuning_cfg tuning_buf_addr;
static void *vi_tuning_ptr[ISP_PRERAW_MAX];

struct vi_clut_idx {
	u32		clut_tbl_idx;
	spinlock_t	clut_idx_lock;
};

struct vi_clut_idx gClutIdx;

/*******************************************************************************
 *	Tuning modules update
 ******************************************************************************/
void vi_tuning_gamma_ips_update(
	struct isp_ctx *ctx,
	enum cvi_isp_raw raw_num)
{
	u8 tun_idx = 0;
	static int stop_update_gamma_ip = -1;
	struct cvi_vip_isp_post_cfg     *post_cfg;
	//struct cvi_vip_isp_post_tun_cfg *post_tun;

	struct cvi_vip_isp_gamma_config   *gamma_cfg;
	struct cvi_vip_isp_ycur_config    *ycur_cfg;

	post_cfg = (struct cvi_vip_isp_post_cfg *)tuning_buf_addr.post_vir[raw_num];
	tun_idx  = post_cfg->tun_idx;

	if ((tun_idx >= TUNING_NODE_NUM) || (post_cfg->tun_update[tun_idx] == 0))
		return;

	//post_tun = &post_cfg->tun_cfg[tun_idx];

	if (tuning_dis[3]) {
		if (stop_update_gamma_ip > 0)
			return;
		else if (tuning_dis[0] == 0) {
			stop_update_gamma_ip = 1;
			return;
		} else if ((tuning_dis[0] - 1) == raw_num)
			stop_update_gamma_ip = 1; // stop on next
	} else
		stop_update_gamma_ip = 0;

	gamma_cfg = &post_cfg->tun_cfg[tun_idx].gamma_cfg;
	ispblk_gamma_tun_cfg(ctx, gamma_cfg, raw_num);

	ycur_cfg = &post_cfg->tun_cfg[tun_idx].ycur_cfg;
	ispblk_ycur_tun_cfg(ctx, ycur_cfg, raw_num);
}

void vi_tuning_ldci_update(
	struct isp_ctx *ctx,
	enum cvi_isp_raw raw_num)
{
	u8 tun_idx = 0;
	static int stop_update_ldci = -1;
	struct cvi_vip_isp_post_cfg     *post_cfg;
	//struct cvi_vip_isp_post_tun_cfg *post_tun;
	struct cvi_vip_isp_ldci_config   *ldci_cfg;

	post_cfg = (struct cvi_vip_isp_post_cfg *)tuning_buf_addr.post_vir[raw_num];
	tun_idx  = post_cfg->tun_idx;

	if ((tun_idx >= TUNING_NODE_NUM) || (post_cfg->tun_update[tun_idx] == 0))
		return;

	//post_tun = &post_cfg->tun_cfg[tun_idx];

	if (tuning_dis[3]) {
		if (stop_update_ldci > 0)
			return;
		else if (tuning_dis[0] == 0) {
			stop_update_ldci = 1;
			return;
		} else if ((tuning_dis[0] - 1) == raw_num)
			stop_update_ldci = 1; // stop on next
	} else
		stop_update_ldci = 0;

	ldci_cfg = &post_cfg->tun_cfg[tun_idx].ldci_cfg;
	ispblk_ldci_tun_cfg(ctx, ldci_cfg, raw_num);
}

void vi_tuning_drc_update(
	struct isp_ctx *ctx,
	enum cvi_isp_raw raw_num)
{
	u8 tun_idx = 0;
	static int stop_update_drc = -1;
	struct cvi_vip_isp_post_cfg     *post_cfg;
	//struct cvi_vip_isp_post_tun_cfg *post_tun;
	struct cvi_vip_isp_drc_config   *drc_cfg;

	post_cfg = (struct cvi_vip_isp_post_cfg *)tuning_buf_addr.post_vir[raw_num];
	drc_cfg = &post_cfg->tun_cfg[tun_idx].drc_cfg;
	tun_idx  = post_cfg->tun_idx;

	if ((tun_idx >= TUNING_NODE_NUM) || (post_cfg->tun_update[tun_idx] == 0))
		return;

	//post_tun = &post_cfg->tun_cfg[tun_idx];

	if (tuning_dis[3]) {
		if (stop_update_drc > 0)
			return;
		else if (tuning_dis[0] == 0) {
			stop_update_drc = 1;
			return;
		} else if ((tuning_dis[0] - 1) == raw_num)
			stop_update_drc = 1; // stop on next
	} else
		stop_update_drc = 0;

	ispblk_drc_tun_cfg(ctx, drc_cfg, raw_num);
}

void vi_tuning_clut_update(
	struct isp_ctx *ctx,
	enum cvi_isp_raw raw_num)
{
	u8 tun_idx = 0;
	static int stop_update_clut = -1;
	struct cvi_vip_isp_post_cfg     *post_cfg;
	//struct cvi_vip_isp_post_tun_cfg *post_tun;
	struct cvi_vip_isp_clut_config  *clut_cfg;
	unsigned long flags;

	post_cfg = (struct cvi_vip_isp_post_cfg *)tuning_buf_addr.post_vir[raw_num];
	tun_idx  = post_cfg->tun_idx;

	vi_pr(VI_DBG, "Postraw_%d tuning update(%d):idx(%d)\n",
			raw_num, post_cfg->tun_update[tun_idx], tun_idx);

	if ((tun_idx >= TUNING_NODE_NUM) || (post_cfg->tun_update[tun_idx] == 0))
		return;

	//post_tun = &post_cfg->tun_cfg[tun_idx];

	if (tuning_dis[3]) {
		if (stop_update_clut > 0)
			return;
		else if (tuning_dis[0] == 0) {
			stop_update_clut = 1;
			return;
		} else if ((tuning_dis[0] - 1) == raw_num)
			stop_update_clut = 1; // stop on next
	} else
		stop_update_clut = 0;

	clut_cfg = &post_cfg->tun_cfg[tun_idx].clut_cfg;
	ispblk_clut_tun_cfg(ctx, clut_cfg, raw_num);

	//Record the clut tbl idx written into HW for ISP MW.
	spin_lock_irqsave(&gClutIdx.clut_idx_lock, flags);
	gClutIdx.clut_tbl_idx = clut_cfg->tbl_idx;
	spin_unlock_irqrestore(&gClutIdx.clut_idx_lock, flags);
}

int vi_tuning_get_clut_tbl_idx(void)
{
	unsigned long flags;
	u32 tbl_idx = 0;

	//Return clut tbl idx to ISP MW that currently is written into ISP HW.
	spin_lock_irqsave(&gClutIdx.clut_idx_lock, flags);
	tbl_idx = gClutIdx.clut_tbl_idx;
	spin_unlock_irqrestore(&gClutIdx.clut_idx_lock, flags);

	return tbl_idx;
}

int vi_tuning_sw_init(void)
{
	return 0;
}

int vi_tuning_buf_setup(void)
{
	u8 i = 0;
	static u64 fe_paddr[ISP_PRERAW_MAX] = {0, 0};
	static u64 post_paddr[ISP_PRERAW_MAX] = {0, 0};
	u32 size = 0;

	size = (sizeof(struct cvi_vip_isp_post_cfg) +
		sizeof(struct cvi_vip_isp_fe_cfg));
	size = VI_ALIGN(size);

	for (i = 0; i < ISP_PRERAW1; i++) { //ISP_PRERAW_MAX
		u64 phyAddr = 0;

		if (vi_tuning_ptr[i] == NULL) {
			vi_tuning_ptr[i] = (void *)PTN_MALLOC(size);
			if (vi_tuning_ptr[i] == NULL) {
				vi_pr(VI_ERR, "tuning_buf ptr[%d] kmalloc size(%d) fail\n", i, size);
				return -ENOMEM;
			}

			phyAddr = (u64)(vi_tuning_ptr[i]);
		}

		if (post_paddr[i] == 0) {
			post_paddr[i] = phyAddr;
			tuning_buf_addr.post_addr[i] = post_paddr[i];
			tuning_buf_addr.post_vir[i] = (void *)(post_paddr[i]);
		}

		if (fe_paddr[i] == 0) {
			fe_paddr[i] = phyAddr + VI_ALIGN(sizeof(struct cvi_vip_isp_post_cfg));
			tuning_buf_addr.fe_addr[i] = fe_paddr[i];
			tuning_buf_addr.fe_vir[i] = (void *)(fe_paddr[i]);
		}

		// vi_pr(VI_INFO, "tuning fe_addr[%u]=0x%"PRIx64", post_addr[%u]=0x%"PRIx64"\n",
		// 		i, tuning_buf_addr.fe_addr[i],
		// 		i, tuning_buf_addr.post_addr[i]);
	}

	return 0;
}

void vi_tuning_buf_release(void)
{
	u8 i;

	for (i = 0; i < ISP_PRERAW1; i++) { //ISP_PRERAW_MAX
		//free(vi_tuning_ptr[i]);
		vi_tuning_ptr[i] = NULL;
	}
}

void *vi_get_tuning_buf_addr(u32 *size)
{
	*size = sizeof(struct isp_tuning_cfg);

	return (void *)&tuning_buf_addr;
}

//TODO memset cost too much time
void vi_tuning_buf_clear(void)
{
	struct cvi_vip_isp_post_cfg *post_cfg;
	struct cvi_vip_isp_fe_cfg   *fe_cfg;
	u8 i = 0, tun_idx = 0;

	for (i = 0; i < ISP_PRERAW_MAX; i++) { //ISP_PRERAW_MAX
		post_cfg = (struct cvi_vip_isp_post_cfg *)tuning_buf_addr.post_vir[i];
		fe_cfg   = (struct cvi_vip_isp_fe_cfg *)tuning_buf_addr.fe_vir[i];

		if (tuning_buf_addr.post_vir[i] != NULL) {
			//memset((void *)tuning_buf_addr.post_vir[i], 0x0, sizeof(struct cvi_vip_isp_post_cfg));
			tun_idx = post_cfg->tun_idx;
			vi_pr(VI_INFO, "Clear post tuning tun_update(%d), tun_idx(%d)\n",
					post_cfg->tun_update[tun_idx], tun_idx);
		}

		if (tuning_buf_addr.fe_vir[i] != NULL) {
			//memset((void *)tuning_buf_addr.fe_vir[i], 0x0, sizeof(struct cvi_vip_isp_fe_cfg));
			tun_idx = fe_cfg->tun_idx;
			vi_pr(VI_INFO, "Clear fe tuning tun_update(%d), tun_idx(%d)\n",
					fe_cfg->tun_update[tun_idx], tun_idx);
		}
	}
}

void pre_fe_tuning_update(
	struct isp_ctx *ctx,
	enum cvi_isp_raw raw_num)
{
	u8 idx = 0, tun_idx = 0;
	static int stop_update = -1;
	struct cvi_vip_isp_fe_cfg *fe_cfg;
	struct cvi_vip_isp_fe_tun_cfg *fe_tun;

	fe_cfg = (struct cvi_vip_isp_fe_cfg *)tuning_buf_addr.fe_vir[raw_num];
	tun_idx = fe_cfg->tun_idx;

	vi_pr(VI_DBG, "Pre_fe_%d tuning update(%d):idx(%d)\n",
			raw_num, fe_cfg->tun_update[tun_idx], tun_idx);

	if ((tun_idx >= TUNING_NODE_NUM) || (fe_cfg->tun_update[tun_idx] == 0))
		return;

	fe_tun = &fe_cfg->tun_cfg[tun_idx];

	if (tuning_dis[1]) {
		if (stop_update > 0)
			return;
		else if (tuning_dis[0] == 0) {
			stop_update = 1;
			return;
		} else if ((tuning_dis[0] - 1) == raw_num)
			stop_update = 1; // stop on next
	} else
		stop_update = 0;

	for (idx = 0; idx < 2; idx++) {
		struct cvi_vip_isp_blc_config	*blc_cfg;
		//struct cvi_vip_isp_lscr_config	*lscr_cfg;
		struct cvi_vip_isp_wbg_config	*wbg_cfg;

		blc_cfg  = &fe_tun->blc_cfg[idx];
		ispblk_blc_tun_cfg(ctx, blc_cfg, raw_num);

		//lscr_cfg = &fe_tun->lscr_cfg[idx];
		//ispblk_lscr_tun_cfg(ctx, lscr_cfg, raw_num);

		wbg_cfg = &fe_tun->wbg_cfg[idx];
		ispblk_wbg_tun_cfg(ctx, wbg_cfg, raw_num);
	}
}

void postraw_tuning_update(
	struct isp_ctx *ctx,
	enum cvi_isp_raw raw_num)
{
	u8 idx = 0, tun_idx = 0;
	static int stop_update = -1;
	struct cvi_vip_isp_post_cfg     *post_cfg;
	struct cvi_vip_isp_post_tun_cfg *post_tun;
	struct cvi_vip_isp_clut_config  *clut_cfg;
	unsigned long flags;

	post_cfg = (struct cvi_vip_isp_post_cfg *)tuning_buf_addr.post_vir[raw_num];
	tun_idx  = post_cfg->tun_idx;

	vi_pr(VI_DBG, "Postraw_%d tuning update(%d):idx(%d)\n",
			raw_num, post_cfg->tun_update[tun_idx], tun_idx);

	if ((tun_idx >= TUNING_NODE_NUM) || (post_cfg->tun_update[tun_idx] == 0)) {
		vi_pr(VI_DBG, "raw_%d stop tuning_update\n", raw_num);
		return;
	}

	post_tun = &post_cfg->tun_cfg[tun_idx];

	if (tuning_dis[3]) {
		if (tuning_dis[0] == 0) {
			vi_pr(VI_DBG, "raw_%d stop tuning_update immediately\n", raw_num);
			return;
		} else if ((tuning_dis[0] - 1) == raw_num) {//stop on next
			if (stop_update > 0) {
				vi_pr(VI_DBG, "raw_%d stop tuning_update\n", raw_num);
				return;
			}
			stop_update = 1;
		} else {//must update tuning buf for sensor, it's will be not trrigered
			stop_update = 0;
		}
	} else
		stop_update = 0;

	for (idx = 0; idx < 2; idx++) {
		struct cvi_vip_isp_blc_config	*blc_cfg;
		struct cvi_vip_isp_dpc_config	*dpc_cfg;

		blc_cfg = &post_tun->blc_cfg[idx];
		ispblk_blc_tun_cfg(ctx, blc_cfg, raw_num);

		dpc_cfg = &post_tun->dpc_cfg[idx];
		ispblk_dpc_tun_cfg(ctx, dpc_cfg, raw_num);
	}
	POST_RUNTIME_TUN(af);

	for (idx = 0; idx < (ctx->isp_pipe_cfg[raw_num].is_hdr_on + 1); idx++) {
		struct cvi_vip_isp_wbg_config	*wbg_cfg;
		struct cvi_vip_isp_ccm_config	*ccm_cfg;
		struct cvi_vip_isp_ae_config	*ae_cfg;

		wbg_cfg = &post_tun->wbg_cfg[idx];
		ispblk_wbg_tun_cfg(ctx, wbg_cfg, raw_num);

		ccm_cfg = &post_tun->ccm_cfg[idx];
		ispblk_ccm_tun_cfg(ctx, ccm_cfg, raw_num);

		ae_cfg = &post_tun->ae_cfg[idx];
		ispblk_ae_tun_cfg(ctx, ae_cfg, raw_num);
	}

	POST_RUNTIME_TUN2(bnr, ISP_RAW_PATH_LE);
	if (ctx->is_hdr_on)
		POST_RUNTIME_TUN2(bnr, ISP_RAW_PATH_SE);
	POST_RUNTIME_TUN(lsc);

	POST_RUNTIME_TUN2(demosiac, ISP_RAW_PATH_LE);
	if (ctx->is_hdr_on)
		POST_RUNTIME_TUN2(demosiac, ISP_RAW_PATH_SE);

	POST_RUNTIME_TUN(fswdr);
	// POST_RUNTIME_TUN(drc);
	POST_RUNTIME_TUN(edge_ext);
	//HW limit
	//Need to update gamma ips in postraw done
	//POST_RUNTIME_TUN(gamma);
	POST_RUNTIME_TUN(dhz);

	//Clut can't be writen when streaming
	//HW limit
	//Need to update gamma ips in postraw done
	if (_is_fe_post_offline(ctx) || (!ctx->is_slice_buf_on)) {// _be_post_online or not slice buffer mode
		POST_RUNTIME_TUN(drc);
		POST_RUNTIME_TUN(gamma);
		POST_RUNTIME_TUN(ycur);

		clut_cfg = &post_tun->clut_cfg;
		ispblk_clut_tun_cfg(ctx, clut_cfg, raw_num);

		//Record the clut tbl idx written into HW for ISP MW.
		spin_lock_irqsave(&gClutIdx.clut_idx_lock, flags);
		gClutIdx.clut_tbl_idx = clut_cfg->tbl_idx;
		spin_unlock_irqrestore(&gClutIdx.clut_idx_lock, flags);
	}

	POST_RUNTIME_TUN(csc);
	//HW limit
	//To update dci tuning at postraw done because josh's ping pong sram has bug
	POST_RUNTIME_TUN(ldci);
	POST_RUNTIME_TUN(pre_ee);
	POST_RUNTIME_TUN(tnr);
	POST_RUNTIME_TUN(cnr);
	POST_RUNTIME_TUN(ee);
	POST_RUNTIME_TUN(cacp);
	POST_RUNTIME_TUN(ca2);
	//HW limit
	//Need to update gamma ips in postraw done
	//POST_RUNTIME_TUN(ycur);
	// {
	// 	struct cvi_vip_isp_mono_config *cfg;
	// 	static u8 mono_mode_sts[ISP_PRERAW_MAX];
	// 	uintptr_t tnr = ctx->phys_regs[ISP_BLK_ID_TNR];

	// 	cfg = &post_tun->mono_cfg;
	// 	ispblk_mono_tun_cfg(ctx, cfg, raw_num);
	// 	if (cfg->force_mono_enable != mono_mode_sts[raw_num]) {
	// 		ISP_WR_BITS(tnr, REG_ISP_444_422_T, REG_8, FORCE_DMA_DISABLE, 0x3f);
	// 		isp_first_frm_reset(ctx, 1);
	// 	} else {
	// 		isp_first_frm_reset(ctx, 0);
	// 		ISP_WR_BITS(tnr, REG_ISP_444_422_T, REG_8, FORCE_DMA_DISABLE, 0);
	// 	}
	// 	mono_mode_sts[raw_num] = cfg->force_mono_enable;
	// }
}

/****************************************************************************
 *	Tuning Config
 ****************************************************************************/
void ispblk_blc_tun_cfg(
	struct isp_ctx *ctx,
	struct cvi_vip_isp_blc_config *cfg,
	const enum cvi_isp_raw raw_num)
{
	int id = cfg->inst == 0 ? ISP_BLK_ID_BLC_DG_WB0 : ISP_BLK_ID_BLC_DG_WB1;
	uintptr_t ba = ctx->phys_regs[id];

	if (!cfg->update)
		return;

	if (!cfg->enable)
		return;

	if (cfg->inst == 0) {
		ISP_WR_BITS(ba, REG_ISP_BLC_DG_WB_T, BASE_CONFIG, BLC_LE_ENABLE, cfg->enable);

		ISP_WR_BITS(ba, REG_ISP_BLC_DG_WB_T, BLC_LE_OFFSET0, BLC_LE_OFFSET_R, cfg->roffset);
		ISP_WR_BITS(ba, REG_ISP_BLC_DG_WB_T, BLC_LE_OFFSET0, BLC_LE_OFFSET_GR, cfg->groffset);
		ISP_WR_BITS(ba, REG_ISP_BLC_DG_WB_T, BLC_LE_OFFSET1, BLC_LE_OFFSET_GB, cfg->gboffset);
		ISP_WR_BITS(ba, REG_ISP_BLC_DG_WB_T, BLC_LE_OFFSET1, BLC_LE_OFFSET_B, cfg->boffset);

		ISP_WR_BITS(ba, REG_ISP_BLC_DG_WB_T, BLC_LE_OFFSET_2ND0, BLC_LE_2NDOFFSET_R, cfg->roffset_2nd);
		ISP_WR_BITS(ba, REG_ISP_BLC_DG_WB_T, BLC_LE_OFFSET_2ND0, BLC_LE_2NDOFFSET_GR, cfg->groffset_2nd);
		ISP_WR_BITS(ba, REG_ISP_BLC_DG_WB_T, BLC_LE_OFFSET_2ND1, BLC_LE_2NDOFFSET_GB, cfg->gboffset_2nd);
		ISP_WR_BITS(ba, REG_ISP_BLC_DG_WB_T, BLC_LE_OFFSET_2ND1, BLC_LE_2NDOFFSET_B, cfg->boffset_2nd);

		ISP_WR_BITS(ba, REG_ISP_BLC_DG_WB_T, BLC_LE_GAIN0, BLC_LE_GAIN_R, cfg->rgain);
		ISP_WR_BITS(ba, REG_ISP_BLC_DG_WB_T, BLC_LE_GAIN0, BLC_LE_GAIN_GR, cfg->grgain);
		ISP_WR_BITS(ba, REG_ISP_BLC_DG_WB_T, BLC_LE_GAIN1, BLC_LE_GAIN_GB, cfg->gbgain);
		ISP_WR_BITS(ba, REG_ISP_BLC_DG_WB_T, BLC_LE_GAIN1, BLC_LE_GAIN_B, cfg->bgain);
	} else {
		ISP_WR_BITS(ba, REG_ISP_BLC_DG_WB_T, BASE_CONFIG, BLC_SE_ENABLE, cfg->enable);

		ISP_WR_BITS(ba, REG_ISP_BLC_DG_WB_T, BLC_SE_OFFSET0, BLC_SE_OFFSET_R, cfg->roffset);
		ISP_WR_BITS(ba, REG_ISP_BLC_DG_WB_T, BLC_SE_OFFSET0, BLC_SE_OFFSET_GR, cfg->groffset);
		ISP_WR_BITS(ba, REG_ISP_BLC_DG_WB_T, BLC_SE_OFFSET1, BLC_SE_OFFSET_GB, cfg->gboffset);
		ISP_WR_BITS(ba, REG_ISP_BLC_DG_WB_T, BLC_SE_OFFSET1, BLC_SE_OFFSET_B, cfg->boffset);

		ISP_WR_BITS(ba, REG_ISP_BLC_DG_WB_T, BLC_SE_OFFSET_2ND0, BLC_SE_2NDOFFSET_R, cfg->roffset_2nd);
		ISP_WR_BITS(ba, REG_ISP_BLC_DG_WB_T, BLC_SE_OFFSET_2ND0, BLC_SE_2NDOFFSET_GR, cfg->groffset_2nd);
		ISP_WR_BITS(ba, REG_ISP_BLC_DG_WB_T, BLC_SE_OFFSET_2ND1, BLC_SE_2NDOFFSET_GB, cfg->gboffset_2nd);
		ISP_WR_BITS(ba, REG_ISP_BLC_DG_WB_T, BLC_SE_OFFSET_2ND1, BLC_SE_2NDOFFSET_B, cfg->boffset_2nd);

		ISP_WR_BITS(ba, REG_ISP_BLC_DG_WB_T, BLC_SE_GAIN0, BLC_SE_GAIN_R, cfg->rgain);
		ISP_WR_BITS(ba, REG_ISP_BLC_DG_WB_T, BLC_SE_GAIN0, BLC_SE_GAIN_GR, cfg->grgain);
		ISP_WR_BITS(ba, REG_ISP_BLC_DG_WB_T, BLC_SE_GAIN1, BLC_SE_GAIN_GB, cfg->gbgain);
		ISP_WR_BITS(ba, REG_ISP_BLC_DG_WB_T, BLC_SE_GAIN1, BLC_SE_GAIN_B, cfg->bgain);
	}
}

void ispblk_wbg_tun_cfg(
	struct isp_ctx *ctx,
	struct cvi_vip_isp_wbg_config *cfg,
	const enum cvi_isp_raw raw_num)
{
	int id = cfg->inst == 0 ? ISP_BLK_ID_BLC_DG_WB0 : ISP_BLK_ID_BLC_DG_WB1;
	uintptr_t ba = ctx->phys_regs[id];

	if (!cfg->update)
		return;

	if (!cfg->enable)
		return;

	if (cfg->inst == 0) {
		ISP_WR_BITS(ba, REG_ISP_BLC_DG_WB_T, BASE_CONFIG, WBG_LE_ENABLE, cfg->enable);

		ISP_WR_BITS(ba, REG_ISP_BLC_DG_WB_T, WBG_LE_GAIN0, WBG_LE_RGAIN, cfg->rgain);
		ISP_WR_BITS(ba, REG_ISP_BLC_DG_WB_T, WBG_LE_GAIN0, WBG_LE_GGAIN, cfg->ggain);
		ISP_WR_BITS(ba, REG_ISP_BLC_DG_WB_T, WBG_LE_GAIN1, WBG_LE_BGAIN, cfg->bgain);
	} else {
		ISP_WR_BITS(ba, REG_ISP_BLC_DG_WB_T, BASE_CONFIG, WBG_SE_ENABLE, cfg->enable);

		ISP_WR_BITS(ba, REG_ISP_BLC_DG_WB_T, WBG_SE_GAIN0, WBG_SE_RGAIN, cfg->rgain);
		ISP_WR_BITS(ba, REG_ISP_BLC_DG_WB_T, WBG_SE_GAIN0, WBG_SE_GGAIN, cfg->bgain);
		ISP_WR_BITS(ba, REG_ISP_BLC_DG_WB_T, WBG_SE_GAIN1, WBG_SE_BGAIN, cfg->bgain);
	}
}

/****************************************************************************
 *	Postraw Tuning Config
 ****************************************************************************/
void ispblk_ccm_tun_cfg(
	struct isp_ctx *ctx,
	struct cvi_vip_isp_ccm_config *cfg,
	const enum cvi_isp_raw raw_num)
{
	uintptr_t ccm = ctx->phys_regs[ISP_BLK_ID_CCM];

	if (!cfg->update)
		return;

	ISP_WR_BITS(ccm, REG_ISP_CCM_T, CCM_CTRL, CCM_ENABLE, cfg->enable);

	if (!cfg->enable)
		return;

	ISP_WR_REG(ccm, REG_ISP_CCM_T, CCM_00, cfg->coef[0][0]);
	ISP_WR_REG(ccm, REG_ISP_CCM_T, CCM_01, cfg->coef[0][1]);
	ISP_WR_REG(ccm, REG_ISP_CCM_T, CCM_02, cfg->coef[0][2]);
	ISP_WR_REG(ccm, REG_ISP_CCM_T, CCM_10, cfg->coef[1][0]);
	ISP_WR_REG(ccm, REG_ISP_CCM_T, CCM_11, cfg->coef[1][1]);
	ISP_WR_REG(ccm, REG_ISP_CCM_T, CCM_12, cfg->coef[1][2]);
	ISP_WR_REG(ccm, REG_ISP_CCM_T, CCM_20, cfg->coef[2][0]);
	ISP_WR_REG(ccm, REG_ISP_CCM_T, CCM_21, cfg->coef[2][1]);
	ISP_WR_REG(ccm, REG_ISP_CCM_T, CCM_22, cfg->coef[2][2]);
}

void ispblk_cacp_tun_cfg(
	struct isp_ctx *ctx,
	struct cvi_vip_isp_cacp_config *cfg,
	const enum cvi_isp_raw raw_num)
{
	uintptr_t cacp = ctx->phys_regs[ISP_BLK_ID_CA];
	uint16_t i;
	union REG_ISP_CA_00 ca_00;
	union REG_ISP_CA_04 wdata;

	if (!cfg->update)
		return;

	ca_00.raw = ISP_RD_REG(cacp, REG_ISP_CA_T, REG_00);
	ca_00.bits.CACP_ENABLE		= cfg->enable;
	ca_00.bits.CACP_MODE		= cfg->mode; // 0 CA mode, 1 CP mode
	ca_00.bits.CACP_ISO_RATIO	= cfg->iso_ratio;
	ISP_WR_REG(cacp, REG_ISP_CA_T, REG_00, ca_00.raw);

	if (!cfg->enable)
		return;

	ISP_WR_BITS(cacp, REG_ISP_CA_T, REG_00, CACP_MEM_SW_MODE, 1);
	if (cfg->mode == 0) {
		for (i = 0; i < 256; i++) {
			wdata.raw = 0;
			wdata.bits.CACP_MEM_D = cfg->ca_y_ratio_lut[i];
			wdata.bits.CACP_MEM_W = 1;
			ISP_WR_REG(cacp, REG_ISP_CA_T, REG_04, wdata.raw);
		}
	} else { //cp mode
		for (i = 0; i < 256; i++) {
			wdata.raw = 0;
			wdata.bits.CACP_MEM_D = ((cfg->cp_y_lut[i] << 16) |
					(cfg->cp_u_lut[i] << 8) | (cfg->cp_v_lut[i]));
			wdata.bits.CACP_MEM_W = 1;
			ISP_WR_REG(cacp, REG_ISP_CA_T, REG_04, wdata.raw);
		}
	}

	ISP_WR_BITS(cacp, REG_ISP_CA_T, REG_00, CACP_MEM_SW_MODE, 0);
}

void ispblk_ca2_tun_cfg(
	struct isp_ctx *ctx,
	struct cvi_vip_isp_ca2_config *cfg,
	const enum cvi_isp_raw raw_num)
{
	uintptr_t ca_lite = ctx->phys_regs[ISP_BLK_ID_CA_LITE];

	if (!cfg->update)
		return;

	ISP_WR_BITS(ca_lite, REG_ISP_CA_LITE_T, REG_00, CA_LITE_ENABLE, cfg->enable);

	if (!cfg->enable)
		return;

	ISP_WR_BITS(ca_lite, REG_ISP_CA_LITE_T, REG_04, CA_LITE_LUT_IN_0, cfg->lut_in[0]);
	ISP_WR_BITS(ca_lite, REG_ISP_CA_LITE_T, REG_04, CA_LITE_LUT_IN_1, cfg->lut_in[1]);

	ISP_WR_BITS(ca_lite, REG_ISP_CA_LITE_T, REG_08, CA_LITE_LUT_IN_2, cfg->lut_in[2]);
	ISP_WR_BITS(ca_lite, REG_ISP_CA_LITE_T, REG_08, CA_LITE_LUT_IN_3, cfg->lut_in[3]);

	ISP_WR_BITS(ca_lite, REG_ISP_CA_LITE_T, REG_0C, CA_LITE_LUT_IN_4, cfg->lut_in[4]);
	ISP_WR_BITS(ca_lite, REG_ISP_CA_LITE_T, REG_0C, CA_LITE_LUT_IN_5, cfg->lut_in[5]);

	ISP_WR_BITS(ca_lite, REG_ISP_CA_LITE_T, REG_10, CA_LITE_LUT_OUT_0, cfg->lut_out[0]);
	ISP_WR_BITS(ca_lite, REG_ISP_CA_LITE_T, REG_10, CA_LITE_LUT_OUT_1, cfg->lut_out[1]);

	ISP_WR_BITS(ca_lite, REG_ISP_CA_LITE_T, REG_14, CA_LITE_LUT_OUT_2, cfg->lut_out[2]);
	ISP_WR_BITS(ca_lite, REG_ISP_CA_LITE_T, REG_14, CA_LITE_LUT_OUT_3, cfg->lut_out[3]);

	ISP_WR_BITS(ca_lite, REG_ISP_CA_LITE_T, REG_18, CA_LITE_LUT_OUT_4, cfg->lut_out[4]);
	ISP_WR_BITS(ca_lite, REG_ISP_CA_LITE_T, REG_18, CA_LITE_LUT_OUT_5, cfg->lut_out[5]);

	ISP_WR_BITS(ca_lite, REG_ISP_CA_LITE_T, REG_1C, CA_LITE_LUT_SLP_0, cfg->lut_slp[0]);
	ISP_WR_BITS(ca_lite, REG_ISP_CA_LITE_T, REG_1C, CA_LITE_LUT_SLP_1, cfg->lut_slp[1]);

	ISP_WR_BITS(ca_lite, REG_ISP_CA_LITE_T, REG_20, CA_LITE_LUT_SLP_2, cfg->lut_slp[2]);
	ISP_WR_BITS(ca_lite, REG_ISP_CA_LITE_T, REG_20, CA_LITE_LUT_SLP_3, cfg->lut_slp[3]);

	ISP_WR_BITS(ca_lite, REG_ISP_CA_LITE_T, REG_24, CA_LITE_LUT_SLP_4, cfg->lut_slp[4]);
}

void ispblk_gamma_tun_cfg(
	struct isp_ctx *ctx,
	struct cvi_vip_isp_gamma_config *cfg,
	const enum cvi_isp_raw raw_num)
{
	uintptr_t gamma = ctx->phys_regs[ISP_BLK_ID_RGBGAMMA];
	int16_t i;

	union REG_ISP_GAMMA_PROG_DATA reg_data;
	union REG_ISP_GAMMA_PROG_CTRL prog_ctrl;

	if (!cfg->update)
		return;

	ISP_WR_BITS(gamma, REG_ISP_GAMMA_T, GAMMA_CTRL, GAMMA_ENABLE, cfg->enable);

	if (!cfg->enable)
		return;

	prog_ctrl.raw = ISP_RD_REG(gamma, REG_ISP_GAMMA_T, GAMMA_PROG_CTRL);
	prog_ctrl.bits.GAMMA_WSEL		= 0;
	prog_ctrl.bits.GAMMA_PROG_EN		= 1;
	prog_ctrl.bits.GAMMA_PROG_1TO3_EN	= 1;
	ISP_WR_REG(gamma, REG_ISP_GAMMA_T, GAMMA_PROG_CTRL, prog_ctrl.raw);

	ISP_WR_BITS(gamma, REG_ISP_GAMMA_T, GAMMA_PROG_ST_ADDR, GAMMA_ST_ADDR, 0);
	ISP_WR_BITS(gamma, REG_ISP_GAMMA_T, GAMMA_PROG_ST_ADDR, GAMMA_ST_W, 1);
	ISP_WR_REG(gamma, REG_ISP_GAMMA_T, GAMMA_PROG_MAX, cfg->max);

	for (i = 0; i < 256; i += 2) {
		reg_data.raw = 0;
		reg_data.bits.GAMMA_DATA_E = cfg->lut[i];
		reg_data.bits.GAMMA_DATA_O = cfg->lut[i + 1];
		reg_data.bits.GAMMA_W = 1;
		ISP_WR_REG(gamma, REG_ISP_GAMMA_T, GAMMA_PROG_DATA, reg_data.raw);
	}

	ISP_WR_BITS(gamma, REG_ISP_GAMMA_T, GAMMA_PROG_CTRL, GAMMA_RSEL, prog_ctrl.bits.GAMMA_WSEL);
	ISP_WR_BITS(gamma, REG_ISP_GAMMA_T, GAMMA_PROG_CTRL, GAMMA_PROG_EN, 0);
}

void ispblk_demosiac_tun_cfg(
	struct isp_ctx *ctx,
	struct cvi_vip_isp_demosiac_config *cfg,
	const enum cvi_isp_raw raw_num,
	enum ISP_RAW_PATH path)
{
	uintptr_t cfa = ctx->phys_regs[ISP_BLK_ID_CFA];
	union REG_ISP_CFA_00 reg_00;
	union REG_ISP_CFA_04 reg_04;
	union REG_ISP_CFA_110 reg_110;

	if (!cfg->update)
		return;

	reg_00.raw = ISP_RD_REG(cfa, REG_ISP_CFA_T, REG_00);
	reg_00.bits.CFA_ENABLE			= cfg->cfa_enable;
	reg_00.bits.CFA_FORCE_DIR_ENABLE	= cfg->cfa_force_dir_enable;
	reg_00.bits.CFA_FORCE_DIR_SEL		= cfg->cfa_force_dir_sel;
	reg_00.bits.CFA_YMOIRE_ENABLE		= cfg->cfa_ymoire_enable;
	ISP_WR_REG(cfa, REG_ISP_CFA_T, REG_00, reg_00.raw);

	if (!cfg->cfa_enable)
		return;

	reg_04.raw = ISP_RD_REG(cfa, REG_ISP_CFA_T, REG_04);
	reg_04.bits.CFA_OUT_SEL		= cfg->cfa_out_sel;
	reg_04.bits.CFA_EDGEE_THD2	= cfg->cfa_edgee_thd2;
	ISP_WR_REG(cfa, REG_ISP_CFA_T, REG_04, reg_04.raw);

	ISP_WR_BITS(cfa, REG_ISP_CFA_T, REG_20, CFA_RBSIG_LUMA_THD, cfg->cfa_rbsig_luma_thd);

	reg_110.raw = ISP_RD_REG(cfa, REG_ISP_CFA_T, REG_110);
	reg_110.bits.CFA_YMOIRE_LPF_W	= cfg->cfa_ymoire_lpf_w;
	reg_110.bits.CFA_YMOIRE_DC_W	= cfg->cfa_ymoire_dc_w;
	ISP_WR_REG(cfa, REG_ISP_CFA_T, REG_110, reg_110.raw);

	ISP_WR_REG_LOOP_SHFT(cfa, REG_ISP_CFA_T, REG_30, 32, 4, cfg->cfa_ghp_lut, 8);

	ISP_WR_REGS_BURST(cfa, REG_ISP_CFA_T, REG_0C, cfg->demosiac_cfg, cfg->demosiac_cfg.reg_0c);

	ISP_WR_REGS_BURST(cfa, REG_ISP_CFA_T, REG_120, cfg->demosiac_1_cfg, cfg->demosiac_1_cfg.reg_120);

	ISP_WR_REGS_BURST(cfa, REG_ISP_CFA_T, REG_90, cfg->demosiac_2_cfg, cfg->demosiac_2_cfg.reg_90);
}

void ispblk_lsc_tun_cfg(
	struct isp_ctx *ctx,
	struct cvi_vip_isp_lsc_config *cfg,
	const enum cvi_isp_raw raw_num)
{
	uintptr_t lsc;
	int id;
	union REG_ISP_LSC_SC_WRAP_4 lsc_mode;
	union REG_ISP_LSC_SC_WRAP_3 inter_p;

	id = lsc_find_hwid(raw_num, true);
	lsc = ctx->phys_regs[id];

	if (!cfg->update)
		return;

	ISP_WR_BITS(lsc, REG_ISP_LSC_T, SC_WRAP_0, LSC_ENABLE, cfg->enable);
	ISP_WR_BITS(lsc, REG_ISP_LSC_T, SC_WRAP_11, DMA_ENABLE_LSC, cfg->enable);

	if (!cfg->enable)
		return;

	ISP_WR_BITS(lsc, REG_ISP_LSC_T, SC_WRAP_0, LSC_DEBUG, cfg->debug);
	// ISP_WR_REG(lsc, REG_ISP_LSC_T, SC_WRAP_2, IMG_WIDTH, ctx->img_width);
	// ISP_WR_REG(lsc, REG_ISP_LSC_T, SC_WRAP_2, IMG_HEIGHT, ctx->img_height);

	inter_p.raw = ISP_RD_REG(lsc, REG_ISP_LSC_T, SC_WRAP_3);
	inter_p.bits.LSC_BOUNDARY_INTERPOLATION_LF_RANGE = cfg->boundary_interpolation_lf_range;
	inter_p.bits.LSC_BOUNDARY_INTERPOLATION_UP_RANGE = cfg->boundary_interpolation_up_range;
	inter_p.bits.LSC_BOUNDARY_INTERPOLATION_RT_RANGE = cfg->boundary_interpolation_rt_range;
	inter_p.bits.LSC_BOUNDARY_INTERPOLATION_DN_RANGE = cfg->boundary_interpolation_dn_range;
	ISP_WR_REG(lsc, REG_ISP_LSC_T, SC_WRAP_3, inter_p.raw);

	lsc_mode.raw = ISP_RD_REG(lsc, REG_ISP_LSC_T, SC_WRAP_4);
	lsc_mode.bits.LSC_GAIN_3P9_0_4P8_1 = cfg->gain_3p9_0_4p8_1;
	lsc_mode.bits.LSC_GAIN_BICUBIC_0_BILINEAR_1 = cfg->gain_bicubic_0_bilinear_1;
	lsc_mode.bits.LSC_BOUNDARY_INTERPOLATION_MODE = cfg->boundary_interpolation_mode;
	lsc_mode.bits.LSC_STRENGTH = cfg->strength;
	ISP_WR_REG(lsc, REG_ISP_LSC_T, SC_WRAP_4, lsc_mode.raw);

	ISP_WR_REG(lsc, REG_ISP_LSC_T, SC_WRAP_8, cfg->intp_gain_min);
	ISP_WR_REG(lsc, REG_ISP_LSC_T, SC_WRAP_9, cfg->intp_gain_max);

	ISP_WR_BITS(lsc, REG_ISP_LSC_T, SC_WRAP_13, BLC_OFFSET_R, cfg->blc_offset_r);
	ISP_WR_BITS(lsc, REG_ISP_LSC_T, SC_WRAP_13, BLC_OFFSET_GR, cfg->blc_offset_gr);
	ISP_WR_BITS(lsc, REG_ISP_LSC_T, SC_WRAP_14, BLC_OFFSET_GB, cfg->blc_offset_gb);
	ISP_WR_BITS(lsc, REG_ISP_LSC_T, SC_WRAP_14, BLC_OFFSET_B, cfg->blc_offset_b);
}

void ispblk_bnr_tun_cfg(
	struct isp_ctx *ctx,
	struct cvi_vip_isp_bnr_config *cfg,
	const enum cvi_isp_raw raw_num,
	enum ISP_RAW_PATH path)
{
	uintptr_t ba = ctx->phys_regs[ISP_BLK_ID_BNR];
	union REG_ISP_BNR_00 enable;

	if (!cfg->update)
		return;

	enable.raw = ISP_RD_REG(ba, REG_ISP_BNR_T, REG_00);
	enable.bits.U1_BNR_ENABLE = cfg->enable;
	ISP_WR_REG(ba, REG_ISP_BNR_T, REG_00, enable.raw);

	if (!cfg->enable)
		return;
}

void ispblk_clut_partial_update(
	struct isp_ctx *ctx,
	struct cvi_vip_isp_clut_config *cfg,
	const enum cvi_isp_raw raw_num)
{
	uintptr_t clut = ctx->phys_regs[ISP_BLK_ID_CLUT];
	union REG_ISP_CLUT_CTRL      ctrl;
	union REG_ISP_CLUT_PROG_DATA prog_data;
	u32 i = 0;

	if ((!_is_fe_post_offline(ctx) || ctx->is_slice_buf_on) && cfg->update_length >= 256)
		cfg->update_length = 256;
	else if (cfg->update_length >= 1024)
		cfg->update_length = 1024;

	ctrl.raw = ISP_RD_REG(clut, REG_ISP_CLUT_T, CLUT_CTRL);
	ctrl.bits.PROG_EN = 1;
	ISP_WR_REG(clut, REG_ISP_CLUT_T, CLUT_CTRL, ctrl.raw);

	for (; i < cfg->update_length; i++) {
		ISP_WR_REG(clut, REG_ISP_CLUT_T, CLUT_PROG_ADDR, cfg->lut[i][0]);

		prog_data.raw			= 0;
		prog_data.bits.SRAM_WDATA	= cfg->lut[i][1];
		prog_data.bits.SRAM_WR		= 1;
		ISP_WR_REG(clut, REG_ISP_CLUT_T, CLUT_PROG_DATA, prog_data.raw);
	}

	ctrl.bits.CLUT_ENABLE = cfg->enable;
	ctrl.bits.PROG_EN = 0;
	ISP_WR_REG(clut, REG_ISP_CLUT_T, CLUT_CTRL, ctrl.raw);
}

void ispblk_clut_tun_cfg(
	struct isp_ctx *ctx,
	struct cvi_vip_isp_clut_config *cfg,
	const enum cvi_isp_raw raw_num)
{
	if (!cfg->update)
		return;

	if (cfg->is_update_partial) { //partail update table
		ispblk_clut_partial_update(ctx, cfg, raw_num);
	} else if (!(!_is_fe_post_offline(ctx) || ctx->is_slice_buf_on)) {
		ispblk_clut_config(ctx, cfg->enable, cfg->r_lut, cfg->g_lut, cfg->b_lut, false);
	}
}

void ispblk_csc_tun_cfg(
	struct isp_ctx *ctx,
	struct cvi_vip_isp_csc_config *cfg,
	const enum cvi_isp_raw raw_num)
{
	uintptr_t csc = ctx->phys_regs[ISP_BLK_ID_CSC];
	union REG_ISP_CSC_4 csc_4;
	union REG_ISP_CSC_5 csc_5;
	union REG_ISP_CSC_6 csc_6;
	union REG_ISP_CSC_7 csc_7;
	union REG_ISP_CSC_8 csc_8;
	union REG_ISP_CSC_9 csc_9;

	if (!cfg->update)
		return;

	ISP_WR_BITS(csc, REG_ISP_CSC_T, REG_0, CSC_ENABLE, cfg->enable);
	if (!cfg->enable)
		return;

	csc_4.raw = 0;
	csc_4.bits.COEFF_00 = cfg->coeff[0] & 0x3fff;
	csc_4.bits.COEFF_01 = cfg->coeff[1] & 0x3fff;
	ISP_WR_REG(csc, REG_ISP_CSC_T, REG_4, csc_4.raw);

	csc_5.raw = 0;
	csc_5.bits.COEFF_02 = cfg->coeff[2] & 0x3fff;
	csc_5.bits.COEFF_10 = cfg->coeff[3] & 0x3fff;
	ISP_WR_REG(csc, REG_ISP_CSC_T, REG_5, csc_5.raw);

	csc_6.raw = 0;
	csc_6.bits.COEFF_11 = cfg->coeff[4] & 0x3fff;
	csc_6.bits.COEFF_12 = cfg->coeff[5] & 0x3fff;
	ISP_WR_REG(csc, REG_ISP_CSC_T, REG_6, csc_6.raw);

	csc_7.raw = 0;
	csc_7.bits.COEFF_20 = cfg->coeff[6] & 0x3fff;
	csc_7.bits.COEFF_21 = cfg->coeff[7] & 0x3fff;
	ISP_WR_REG(csc, REG_ISP_CSC_T, REG_7, csc_7.raw);

	csc_8.raw = 0;
	csc_8.bits.COEFF_22 = cfg->coeff[8] & 0x3fff;
	csc_8.bits.OFFSET_0 = cfg->offset[0] & 0x7ff;
	ISP_WR_REG(csc, REG_ISP_CSC_T, REG_8, csc_8.raw);

	csc_9.raw = 0;
	csc_9.bits.OFFSET_1 = cfg->offset[1] & 0x7ff;
	csc_9.bits.OFFSET_2 = cfg->offset[2] & 0x7ff;
	ISP_WR_REG(csc, REG_ISP_CSC_T, REG_9, csc_9.raw);
}

void ispblk_drc_tun_cfg(
	struct isp_ctx *ctx,
	struct cvi_vip_isp_drc_config *cfg,
	const enum cvi_isp_raw raw_num)
{
	uintptr_t ba = ctx->phys_regs[ISP_BLK_ID_DRC];
	union REG_ISP_DRC_ENABLE enable;

	if (!cfg->update)
		return;

	enable.raw = ISP_RD_REG(ba, REG_ISP_DRC_T, ENABLE);
	enable.bits.DRC_ENABLE = cfg->drc_enable;
	ISP_WR_REG(ba, REG_ISP_DRC_T, ENABLE, enable.raw);
}

void ispblk_cnr_tun_cfg(
	struct isp_ctx *ctx,
	struct cvi_vip_isp_cnr_config *cfg,
	const enum cvi_isp_raw raw_num)
{
	uintptr_t ba = ctx->phys_regs[ISP_BLK_ID_CNR];
	union REG_ISP_CNR_CTRL_HW_ONLY enable;

	if (!cfg->update)
		return;

	enable.raw = ISP_RD_REG(ba, REG_ISP_CNR_T, CNR_CTRL_HW_ONLY);
	enable.bits.CNR_ENABLE = cfg->enable;
	ISP_WR_REG(ba, REG_ISP_CNR_T, CNR_CTRL_HW_ONLY, enable.raw);

	if (!cfg->enable)
		return;
}

void ispblk_tnr_tun_cfg(
	struct isp_ctx *ctx,
	struct cvi_vip_isp_tnr_config *cfg,
	const enum cvi_isp_raw raw_num)
{
	// uintptr_t ba = ctx->phys_regs[ISP_BLK_ID_TNR];
	// union REG_ISP_CNR_CTRL_HW_ONLY enable;

	// if (!ctx->is_3dnr_on || !cfg->update)
	// 	return;

	// enable.raw = ISP_RD_REG(ba, REG_ISP_CNR_T, CNR_CTRL_HW_ONLY);
	// enable.bits.CNR_ENABLE = cfg->enable;
	// ISP_WR_REG(ba, REG_ISP_CNR_T, CNR_CTRL_HW_ONLY, enable.raw);
}

void ispblk_ee_tun_cfg(
	struct isp_ctx *ctx,
	struct cvi_vip_isp_ee_config *cfg,
	const enum cvi_isp_raw raw_num)
{
	uintptr_t ba = ctx->phys_regs[ISP_BLK_ID_POST_EE];
	union REG_ISP_EE_00  reg_0;

	if (!cfg->update)
		return;

	reg_0.raw = ISP_RD_REG(ba, REG_ISP_EE_T, REG_00);
	reg_0.bits.EE_ENABLE = cfg->enable;
	reg_0.bits.EE_DEBUG_MODE = cfg->dbg_mode;
	ISP_WR_REG(ba, REG_ISP_EE_T, REG_00, reg_0.raw);

	if (!cfg->enable)
		return;
}

void ispblk_pre_ee_tun_cfg(
	struct isp_ctx *ctx,
	struct cvi_vip_isp_pre_ee_config *cfg,
	const enum cvi_isp_raw raw_num)
{
	uintptr_t ba = ctx->phys_regs[ISP_BLK_ID_PRE_EE_FRONT];
	union REG_ISP_EE_00  reg_0;

	if (!cfg->update)
		return;

	reg_0.raw = ISP_RD_REG(ba, REG_ISP_EE_ADD_T, REG0);
	reg_0.bits.EE_ENABLE = cfg->enable;
	reg_0.bits.EE_DEBUG_MODE = cfg->dbg_mode;
	ISP_WR_REG(ba, REG_ISP_EE_ADD_T, REG0, reg_0.raw);

	if (!cfg->enable)
		return;
}

void ispblk_fswdr_tun_cfg(
	struct isp_ctx *ctx,
	struct cvi_vip_isp_fswdr_config *cfg,
	const enum cvi_isp_raw raw_num)
{

}

void ispblk_fswdr_update_rpt(
	struct isp_ctx *ctx,
	struct cvi_vip_isp_fswdr_report *cfg)
{

}

void ispblk_ldci_tun_cfg(
	struct isp_ctx *ctx,
	struct cvi_vip_isp_ldci_config *cfg,
	const enum cvi_isp_raw raw_num)
{
	uintptr_t ba = ctx->phys_regs[ISP_BLK_ID_LDCI];
	union REG_ISP_LDCI_CTL reg_enable;

	if (!cfg->update)
		return;

	reg_enable.raw = ISP_RD_REG(ba, REG_ISP_LDCI_T, LDCI_CTL);
	reg_enable.bits.LDCI_ENABLE = cfg->enable;
	ISP_WR_REG(ba, REG_ISP_LDCI_T, LDCI_CTL, reg_enable.raw);

	if (!cfg->enable)
		return;
}

void ispblk_ycur_tun_cfg(
	struct isp_ctx *ctx,
	struct cvi_vip_isp_ycur_config *cfg,
	const enum cvi_isp_raw raw_num)
{
	uintptr_t ycur = ctx->phys_regs[ISP_BLK_ID_YCURVE];
	uint16_t i = 0;

	union REG_ISP_YCURV_YCUR_PROG_DATA reg_data;
	union REG_ISP_YCURV_YCUR_PROG_CTRL prog_ctrl;

	if (!cfg->update)
		return;

	ISP_WR_BITS(ycur, REG_ISP_YCURV_T, YCUR_CTRL, YCUR_ENABLE, cfg->enable);

	if (cfg->enable) {
		prog_ctrl.raw = ISP_RD_REG(ycur, REG_ISP_YCURV_T, YCUR_PROG_CTRL);
		prog_ctrl.bits.YCUR_WSEL = prog_ctrl.bits.YCUR_WSEL ^ 1;
		prog_ctrl.bits.YCUR_PROG_EN = 1;
		ISP_WR_REG(ycur, REG_ISP_YCURV_T, YCUR_PROG_CTRL, prog_ctrl.raw);

		ISP_WR_BITS(ycur, REG_ISP_YCURV_T, YCUR_PROG_ST_ADDR, YCUR_ST_ADDR, 0);
		ISP_WR_BITS(ycur, REG_ISP_YCURV_T, YCUR_PROG_ST_ADDR, YCUR_ST_W, 1);
		ISP_WR_REG(ycur, REG_ISP_YCURV_T, YCUR_PROG_MAX, cfg->lut_256);

		for (i = 0; i < 64; i += 2) {
			reg_data.raw = 0;
			reg_data.bits.YCUR_DATA_E = cfg->lut[i];
			reg_data.bits.YCUR_DATA_O = cfg->lut[i + 1];
			reg_data.bits.YCUR_W = 1;
			ISP_WR_REG(ycur, REG_ISP_YCURV_T, YCUR_PROG_DATA, reg_data.raw);
		}

		ISP_WR_BITS(ycur, REG_ISP_YCURV_T, YCUR_PROG_CTRL, YCUR_RSEL, prog_ctrl.bits.YCUR_WSEL);
		ISP_WR_BITS(ycur, REG_ISP_YCURV_T, YCUR_PROG_CTRL, YCUR_PROG_EN, 0);
	}
}

void ispblk_dhz_tun_cfg(
	struct isp_ctx *ctx,
	struct cvi_vip_isp_dhz_config *cfg,
	const enum cvi_isp_raw raw_num)
{
	uintptr_t dhz = ctx->phys_regs[ISP_BLK_ID_DEHAZE];
	union REG_ISP_DEHAZE_DHZ_WGT dhz_wgt;

	if (!cfg->update)
		return;

	ISP_WR_BITS(dhz, REG_ISP_DEHAZE_T, DHZ_BYPASS, DEHAZE_ENABLE, cfg->enable);

	if (!cfg->enable)
		return;

	ISP_WR_BITS(dhz, REG_ISP_DEHAZE_T, DHZ_SMOOTH, DEHAZE_W, cfg->strength);
	ISP_WR_BITS(dhz, REG_ISP_DEHAZE_T, DHZ_SMOOTH, DEHAZE_TH_SMOOTH, cfg->th_smooth);
	ISP_WR_BITS(dhz, REG_ISP_DEHAZE_T, REG_1, DEHAZE_CUM_TH, cfg->cum_th);
	ISP_WR_BITS(dhz, REG_ISP_DEHAZE_T, REG_1, DEHAZE_HIST_TH, cfg->hist_th);
	ISP_WR_BITS(dhz, REG_ISP_DEHAZE_T, REG_3, DEHAZE_TMAP_MIN, cfg->tmap_min);
	ISP_WR_BITS(dhz, REG_ISP_DEHAZE_T, REG_3, DEHAZE_TMAP_MAX, cfg->tmap_max);
	ISP_WR_BITS(dhz, REG_ISP_DEHAZE_T, DHZ_BYPASS, DEHAZE_LUMA_LUT_ENABLE, cfg->luma_lut_enable);
	ISP_WR_BITS(dhz, REG_ISP_DEHAZE_T, DHZ_BYPASS, DEHAZE_SKIN_LUT_ENABLE, cfg->skin_lut_enable);

	dhz_wgt.raw = ISP_RD_REG(dhz, REG_ISP_DEHAZE_T, DHZ_WGT);
	dhz_wgt.bits.DEHAZE_A_LUMA_WGT = cfg->a_luma_wgt;
	dhz_wgt.bits.DEHAZE_BLEND_WGT = cfg->blend_wgt;
	dhz_wgt.bits.DEHAZE_TMAP_SCALE = cfg->tmap_scale;
	dhz_wgt.bits.DEHAZE_D_WGT = cfg->d_wgt;
	ISP_WR_REG(dhz, REG_ISP_DEHAZE_T, DHZ_WGT, dhz_wgt.raw);

	ISP_WR_BITS(dhz, REG_ISP_DEHAZE_T, REG_2, DEHAZE_SW_DC_TH, cfg->sw_dc_th);
	ISP_WR_BITS(dhz, REG_ISP_DEHAZE_T, REG_2, DEHAZE_SW_AGLOBAL_R, cfg->sw_aglobal_r);

	ISP_WR_BITS(dhz, REG_ISP_DEHAZE_T, REG_28, DEHAZE_SW_AGLOBAL_G, cfg->sw_aglobal_g);
	ISP_WR_BITS(dhz, REG_ISP_DEHAZE_T, REG_28, DEHAZE_SW_AGLOBAL_B, cfg->sw_aglobal_b);

	ISP_WR_BITS(dhz, REG_ISP_DEHAZE_T, REG_2C, DEHAZE_AGLOBAL_MAX, cfg->aglobal_max);
	ISP_WR_BITS(dhz, REG_ISP_DEHAZE_T, REG_2C, DEHAZE_AGLOBAL_MIN, cfg->aglobal_min);

	ISP_WR_BITS(dhz, REG_ISP_DEHAZE_T, DHZ_SKIN, DEHAZE_SKIN_CB, cfg->skin_cb);
	ISP_WR_BITS(dhz, REG_ISP_DEHAZE_T, DHZ_SKIN, DEHAZE_SKIN_CR, cfg->skin_cr);

	ISP_WR_REGS_BURST(dhz, REG_ISP_DEHAZE_T, REG_9, cfg->luma_cfg, cfg->luma_cfg.LUMA_00);
	ISP_WR_REGS_BURST(dhz, REG_ISP_DEHAZE_T, REG_17, cfg->skin_cfg, cfg->skin_cfg.SKIN_00);
	ISP_WR_REGS_BURST(dhz, REG_ISP_DEHAZE_T, TMAP_00, cfg->tmap_cfg, cfg->tmap_cfg.TMAP_00);
}

void ispblk_dpc_tun_cfg(
	struct isp_ctx *ctx,
	struct cvi_vip_isp_dpc_config *cfg,
	const enum cvi_isp_raw raw_num)
{
	uintptr_t dpc = ctx->phys_regs[ISP_BLK_ID_DPC];
	union REG_ISP_DPC_BASE_CONFIG base;
	// uint16_t i;

	if (!cfg->update)
		return;

	ISP_WR_BITS(dpc, REG_ISP_DPC_T, BASE_CONFIG, DPC_ENABLE, cfg->enable);
	if (!cfg->enable)
		return;

	base.raw = ISP_RD_REG(dpc, REG_ISP_DPC_T, BASE_CONFIG);
	base.bits.DPC_ENABLE = cfg->enable;
	base.bits.SPC_ENABLE = cfg->enable ? cfg->staticbpc_enable : 0;
	ISP_WR_REG(dpc, REG_ISP_DPC_T, BASE_CONFIG, base.raw);

	// if (cfg->staticbpc_enable && (cfg->bp_cnt > 0) && (cfg->bp_cnt < 2048)) {
	// 	ISP_WR_REG(dpc, REG_ISP_DPC_T, dpc_mem_st_addr, 0x80000000);

	// 	for (i = 0; i < cfg->bp_cnt; i++)
	// 		ISP_WR_BITS(dpc, REG_ISP_DPC_T, SPC_CONFIG, DPC_MEM0_ADDR,
	// 			0x80000000 | cfg->bp_tbl[i]);

	// 	// write 1 fff-fff to end
	// 	ISP_WR_BITS(dpc, REG_ISP_DPC_T, SPC_CONFIG, DPC_MEM0_ADDR, 0x80ffffff);
	// }
}

void ispblk_ae_tun_cfg(
	struct isp_ctx *ctx,
	struct cvi_vip_isp_ae_config *cfg,
	const enum cvi_isp_raw raw_num)
{
	uintptr_t ba = 0;
	uint32_t raw;

	if (!cfg->update)
		return;

	switch (raw_num) {
	case ISP_PRERAW0:
		ba = cfg->inst == 0 ? ctx->phys_regs[ISP_BLK_ID_DMA_CTL_FE0_AE_HIST_LE] :
		     ctx->phys_regs[ISP_BLK_ID_DMA_CTL_FE0_AE_HIST_SE];
		break;
	case ISP_PRERAW1:
		ba = cfg->inst == 0 ? ctx->phys_regs[ISP_BLK_ID_DMA_CTL_FE1_AE_HIST_LE] :
		     ctx->phys_regs[ISP_BLK_ID_DMA_CTL_FE1_AE_HIST_SE];
		break;
	case ISP_PRERAW2:
		ba = ctx->phys_regs[ISP_BLK_ID_DMA_CTL_FE2_AE_HIST_LE];
		break;
	default:
		vi_pr(VI_ERR, "Wrong raw_num\n");
		return;
	}

	ISP_WR_BITS(ba, REG_ISP_AE_HIST_T, STS_AE0_HIST_ENABLE, STS_AE0_HIST_ENABLE, cfg->ae_enable);
	ISP_WR_BITS(ba, REG_ISP_AE_HIST_T, DMI_ENABLE, DMI_ENABLE, cfg->ae_enable);

	if (!cfg->ae_enable)
		return;

	ISP_WR_REG(ba, REG_ISP_AE_HIST_T, STS_AE_OFFSETX, cfg->ae_offsetx);
	ISP_WR_REG(ba, REG_ISP_AE_HIST_T, STS_AE_OFFSETY, cfg->ae_offsety);
	ISP_WR_REG(ba, REG_ISP_AE_HIST_T, STS_AE_NUMXM1, cfg->ae_numx - 1);
	ISP_WR_REG(ba, REG_ISP_AE_HIST_T, STS_AE_NUMYM1, cfg->ae_numy - 1);
	ISP_WR_REG(ba, REG_ISP_AE_HIST_T, STS_AE_WIDTH, cfg->ae_width);
	ISP_WR_REG(ba, REG_ISP_AE_HIST_T, STS_AE_HEIGHT, cfg->ae_height);
	ISP_WR_REG(ba, REG_ISP_AE_HIST_T, STS_AE_STS_DIV, cfg->ae_sts_div);

	raw = cfg->ae_face_offset_x[0] + (cfg->ae_face_offset_y[0] << 16);
	ISP_WR_REG(ba, REG_ISP_AE_HIST_T, AE_FACE0_LOCATION, raw);

	raw = cfg->ae_face_size_minus1_x[0] + (cfg->ae_face_size_minus1_y[0] << 16);
	ISP_WR_REG(ba, REG_ISP_AE_HIST_T, AE_FACE0_SIZE, raw);

	raw = cfg->ae_face_offset_x[1] + (cfg->ae_face_offset_y[1] << 16);
	ISP_WR_REG(ba, REG_ISP_AE_HIST_T, AE_FACE1_LOCATION, raw);

	raw = cfg->ae_face_size_minus1_x[1] + (cfg->ae_face_size_minus1_y[1] << 16);
	ISP_WR_REG(ba, REG_ISP_AE_HIST_T, AE_FACE1_SIZE, raw);

	raw = cfg->ae_face_offset_x[2] + (cfg->ae_face_offset_y[2] << 16);
	ISP_WR_REG(ba, REG_ISP_AE_HIST_T, AE_FACE2_LOCATION, raw);

	raw = cfg->ae_face_size_minus1_x[2] + (cfg->ae_face_size_minus1_y[2] << 16);
	ISP_WR_REG(ba, REG_ISP_AE_HIST_T, AE_FACE2_SIZE, raw);

	raw = cfg->ae_face_offset_x[3] + (cfg->ae_face_offset_y[3] << 16);
	ISP_WR_REG(ba, REG_ISP_AE_HIST_T, AE_FACE3_LOCATION, raw);

	raw = cfg->ae_face_size_minus1_x[3] + (cfg->ae_face_size_minus1_y[3] << 16);
	ISP_WR_REG(ba, REG_ISP_AE_HIST_T, AE_FACE3_SIZE, raw);

	ISP_WR_REGS_BURST(ba, REG_ISP_AE_HIST_T, AE_FACE_ENABLE_CTRL, cfg->ae_cfg, cfg->ae_cfg.ae_face_enable);
	ISP_WR_REGS_BURST(ba, REG_ISP_AE_HIST_T, AE_WGT_00, cfg->ae_2_cfg, cfg->ae_2_cfg.ae_wgt_00);
}

void ispblk_af_tun_cfg(
	struct isp_ctx *ctx,
	struct cvi_vip_isp_af_config *cfg,
	const enum cvi_isp_raw raw_num)
{
	uintptr_t ba = ctx->phys_regs[ISP_BLK_ID_AF];

	union REG_ISP_AF_ENABLES af_enables;
	union REG_ISP_AF_LOW_PASS_HORIZON low_pass_horizon;
	union REG_ISP_AF_HIGH_PASS_HORIZON_0 high_pass_horizon_0;
	union REG_ISP_AF_HIGH_PASS_HORIZON_1 high_pass_horizon_1;
	union REG_ISP_AF_HIGH_PASS_VERTICAL_0 high_pass_vertical_0;

	if (!cfg->update)
		return;

	ISP_WR_BITS(ba, REG_ISP_AF_T, KICKOFF, AF_ENABLE, cfg->enable);
	if (!cfg->enable)
		return;

	ISP_WR_BITS(ba, REG_ISP_AF_T, DMI_ENABLE, DMI_ENABLE, cfg->enable);
	ISP_WR_REG(ba, REG_ISP_AF_T, BYPASS, !cfg->enable);

	af_enables.raw = ISP_RD_REG(ba, REG_ISP_AF_T, ENABLES);
	af_enables.bits.AF_DPC_ENABLE = cfg->dpc_enable;
	af_enables.bits.AF_HLC_ENABLE = cfg->hlc_enable;
	ISP_WR_REG(ba, REG_ISP_AF_T, ENABLES, af_enables.raw);

	ISP_WR_BITS(ba, REG_ISP_AF_T, SQUARE_ENABLE, AF_SQUARE_ENABLE, cfg->square_enable);
	ISP_WR_BITS(ba, REG_ISP_AF_T, OUTSHIFT, AF_OUTSHIFT, cfg->outshift);
	ISP_WR_BITS(ba, REG_ISP_AF_T, NUM_GAPLINE, AF_NUM_GAPLINE, cfg->num_gapline);

	// 8 <= offset_x <= img_width - 8
	ISP_WR_BITS(ba, REG_ISP_AF_T, OFFSET_X, AF_OFFSET_X, cfg->offsetx);
	// 2 <= offset_y <= img_height - 2
	ISP_WR_BITS(ba, REG_ISP_AF_T, OFFSET_X, AF_OFFSET_Y, cfg->offsety);

	ISP_WR_REG(ba, REG_ISP_AF_T, BLOCK_WIDTH, cfg->block_width);	// (ctx->img_width - 16) / numx
	ISP_WR_REG(ba, REG_ISP_AF_T, BLOCK_HEIGHT, cfg->block_height);	// (ctx->img_height - 4) / numy
	ISP_WR_REG(ba, REG_ISP_AF_T, BLOCK_NUM_X, cfg->block_numx);		// should fixed to 17
	ISP_WR_REG(ba, REG_ISP_AF_T, BLOCK_NUM_Y, cfg->block_numy);		// should fixed to 15

	ISP_WR_REG(ba, REG_ISP_AF_T, HOR_LOW_PASS_VALUE_SHIFT, cfg->h_low_pass_value_shift);
	ISP_WR_REG(ba, REG_ISP_AF_T, OFFSET_HORIZONTAL_0, cfg->h_corning_offset_0);
	ISP_WR_REG(ba, REG_ISP_AF_T, OFFSET_HORIZONTAL_1, cfg->h_corning_offset_1);
	ISP_WR_REG(ba, REG_ISP_AF_T, OFFSET_VERTICAL, cfg->v_corning_offset);
	ISP_WR_REG(ba, REG_ISP_AF_T, HIGH_Y_THRE, cfg->high_luma_threshold);

	low_pass_horizon.raw = 0;
	low_pass_horizon.bits.AF_LOW_PASS_HORIZON_0 = cfg->h_low_pass_coef[0];
	low_pass_horizon.bits.AF_LOW_PASS_HORIZON_1 = cfg->h_low_pass_coef[1];
	low_pass_horizon.bits.AF_LOW_PASS_HORIZON_2 = cfg->h_low_pass_coef[2];
	low_pass_horizon.bits.AF_LOW_PASS_HORIZON_3 = cfg->h_low_pass_coef[3];
	low_pass_horizon.bits.AF_LOW_PASS_HORIZON_4 = cfg->h_low_pass_coef[4];
	ISP_WR_REG(ba, REG_ISP_AF_T, LOW_PASS_HORIZON, low_pass_horizon.raw);

	high_pass_horizon_0.raw = 0;
	high_pass_horizon_0.bits.AF_HIGH_PASS_HORIZON_0_0 = cfg->h_high_pass_coef_0[0];
	high_pass_horizon_0.bits.AF_HIGH_PASS_HORIZON_0_1 = cfg->h_high_pass_coef_0[1];
	high_pass_horizon_0.bits.AF_HIGH_PASS_HORIZON_0_2 = cfg->h_high_pass_coef_0[2];
	high_pass_horizon_0.bits.AF_HIGH_PASS_HORIZON_0_3 = cfg->h_high_pass_coef_0[3];
	high_pass_horizon_0.bits.AF_HIGH_PASS_HORIZON_0_4 = cfg->h_high_pass_coef_0[4];
	ISP_WR_REG(ba, REG_ISP_AF_T, HIGH_PASS_HORIZON_0, high_pass_horizon_0.raw);

	high_pass_horizon_1.raw = 0;
	high_pass_horizon_1.bits.AF_HIGH_PASS_HORIZON_1_0 = cfg->h_high_pass_coef_1[0];
	high_pass_horizon_1.bits.AF_HIGH_PASS_HORIZON_1_1 = cfg->h_high_pass_coef_1[1];
	high_pass_horizon_1.bits.AF_HIGH_PASS_HORIZON_1_2 = cfg->h_high_pass_coef_1[2];
	high_pass_horizon_1.bits.AF_HIGH_PASS_HORIZON_1_3 = cfg->h_high_pass_coef_1[3];
	high_pass_horizon_1.bits.AF_HIGH_PASS_HORIZON_1_4 = cfg->h_high_pass_coef_1[4];
	ISP_WR_REG(ba, REG_ISP_AF_T, HIGH_PASS_HORIZON_1, high_pass_horizon_1.raw);

	high_pass_vertical_0.raw = 0;
	high_pass_vertical_0.bits.AF_HIGH_PASS_VERTICAL_0_0 = cfg->v_high_pass_coef[0];
	high_pass_vertical_0.bits.AF_HIGH_PASS_VERTICAL_0_1 = cfg->v_high_pass_coef[1];
	high_pass_vertical_0.bits.AF_HIGH_PASS_VERTICAL_0_2 = cfg->v_high_pass_coef[2];
	ISP_WR_REG(ba, REG_ISP_AF_T, HIGH_PASS_VERTICAL_0, high_pass_vertical_0.raw);

	ISP_WR_BITS(ba, REG_ISP_AF_T, TH_LOW, AF_TH_LOW, cfg->th_low);
	ISP_WR_BITS(ba, REG_ISP_AF_T, TH_LOW, AF_TH_HIGH, cfg->th_high);
	ISP_WR_BITS(ba, REG_ISP_AF_T, GAIN_LOW, AF_GAIN_LOW, cfg->gain_low);
	ISP_WR_BITS(ba, REG_ISP_AF_T, GAIN_LOW, AF_GAIN_HIGH, cfg->gain_high);
	ISP_WR_BITS(ba, REG_ISP_AF_T, SLOP_LOW, AF_SLOP_LOW, cfg->slop_low);
	ISP_WR_BITS(ba, REG_ISP_AF_T, SLOP_LOW, AF_SLOP_HIGH, cfg->slop_high);
}

void ispblk_edge_ext_tun_cfg(
	struct isp_ctx *ctx,
	struct cvi_vip_isp_edge_ext_config *cfg,
	const enum cvi_isp_raw raw_num)
{
	uintptr_t edge_ext = ctx->phys_regs[ISP_BLK_ID_PRE_EE_EXT];

	if (!cfg->update)
		return;

	ISP_WR_BITS(edge_ext, REG_EE_EXT_T, REG0, EE_ENABLE, cfg->enable);

	if (!cfg->enable)
		return;
}

void ispblk_mono_tun_cfg(
	struct isp_ctx *ctx,
	struct cvi_vip_isp_mono_config *cfg,
	const enum cvi_isp_raw raw_num)
{
	uintptr_t tnr = ctx->phys_regs[ISP_BLK_ID_TNR];

	if (!cfg->update)
		return;

	ISP_WR_BITS(tnr, REG_ISP_444_422_T, REG_0, FORCE_MONO_ENABLE, cfg->force_mono_enable);
}

#if 0
void ispblk_lscr_tun_cfg(
	struct isp_ctx *ctx,
	struct cvi_vip_isp_lscr_config *cfg,
	const enum cvi_isp_raw raw_num)
{
}
void ispblk_preproc_tun_cfg(
	struct isp_ctx *ctx,
	struct cvi_vip_isp_preproc_config *cfg,
	const enum cvi_isp_raw raw_num)
{
}
#endif
