#ifndef __VI_DEFINES_H__
#define __VI_DEFINES_H__

#include <vip/vi_drv.h>
#include "bmtest_type.h"

#define DRAM_PHY_BASE   0x83000000

//vi_isp.h
struct cvi_isp_mbus_framefmt {
	u32	width;
	u32	height;
	u32	code;
};

struct cvi_isp_rect {
	s32	left;
	s32	top;
	u32	width;
	u32	height;
};

#ifdef TEST_PERF
struct cvi_isp_timer {
	u32	pre_fe_sof_time[ISP_PRERAW_MAX][ISP_FE_CHN_MAX];
	u32	pre_fe_done_time[ISP_PRERAW_MAX][ISP_FE_CHN_MAX];
	u32	pre_trigpost_time[ISP_PRERAW_MAX];
	u32	post_done_time[ISP_PRERAW_MAX];
#ifdef BUILD_VIP
	u32	vpss_done_time[4];
#endif

	u32	pre_fe_sof_time_sum[ISP_PRERAW_MAX][ISP_FE_CHN_MAX];
	u32	pre_fe_done_time_sum[ISP_PRERAW_MAX][ISP_FE_CHN_MAX];
	u32	post_done_time_sum[ISP_PRERAW_MAX];

	u32 pre_sof2fe_done_frm_time[ISP_PRERAW_MAX][ISP_FE_CHN_MAX];
	u32 pre_fe_done2post_done_frm_time[ISP_PRERAW_MAX];
	u32 trigpost2post_done_frm_time[ISP_PRERAW_MAX];
// #ifdef BUILD_VIP
// 	u32	pre_post_done2vpss_done_frm_time[4];
// #endif
};
#endif
/**
 * struct cvi_vi - VI IP abstraction
 */
struct cvi_vi_dev {
	void*				reg_base;
        uint64_t                        mem_base;
	uint32_t        		alloc_size;     ///<
	uint32_t		        alloc_cnt;      ///<
	uint64_t        		alloc_vec[64];  ///<
        int                             total_dev_num;
	int				irq_num;
	struct isp_ctx			ctx;
	u32				isp_int_flag[ISP_PRERAW_MAX];
	enum cvi_isp_raw		offline_raw_num;
        struct cvi_isp_mbus_framefmt	usr_fmt;
	struct cvi_isp_rect		usr_crop;
	u32				pre_fe_sof_cnt[ISP_PRERAW_MAX][ISP_FE_CHN_MAX];
	u32				pre_fe_frm_num[ISP_PRERAW_MAX][ISP_FE_CHN_MAX];
	u32				pre_fe_trig_cnt[ISP_PRERAW_MAX][ISP_FE_CHN_MAX];
        bool				preraw_first_frm[ISP_PRERAW_MAX];
	u32				postraw_frame_number[ISP_PRERAW_MAX];
	u32				drop_frame_number[ISP_PRERAW_MAX];
	u32				dump_frame_number[ISP_PRERAW_MAX];
	u8				postraw_proc_num;
	uint32_t			isp_streamoff   : 1;
	uint32_t			isp_streamon   : 1;
#ifdef TEST_PERF
	struct cvi_isp_timer isp_timer;
#endif
};

void _vi_debug_info(struct isp_ctx *ctx);
uint64_t PTN_MALLOC(size_t size);
int vi_create_instance(struct cvi_vi_dev *vdev);
int vi_destroy_instance(struct cvi_vi_dev *vdev);
void vi_irq_handler(struct cvi_vi_dev *vdev);
void _pre_hw_enque(struct cvi_vi_dev *vdev, const enum cvi_isp_raw raw_num, const u8 chn_num);
void _post_hw_enque(struct cvi_vi_dev *vdev);

#endif /* __VI_DEFINES_H__ */
