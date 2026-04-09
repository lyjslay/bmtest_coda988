#ifndef __VI_ISP_BUF_CTRL_H__
#define __VI_ISP_BUF_CTRL_H__

#include <vi_common.h>
#include <vi_defines.h>
#include "inc/vi_list.h"

struct isp_buffer {
	enum cvi_isp_raw  raw_num;
	enum cvi_isp_chn_num chn_num;
	uint64_t          addr;
	struct vi_rect    crop_le;
	struct vi_rect    crop_se;
	struct list_head  list;
	uint32_t          byr_size;
	uint32_t          frm_num;
	uint32_t          is_yuv_frm : 1;
};

struct isp_queue {
	struct list_head rdy_queue;
	uint32_t num_rdy;
	enum cvi_isp_raw raw_num;
};

struct isp_buffer *isp_next_buf(struct isp_queue *q);
void isp_buf_queue(struct isp_queue *q, struct isp_buffer *b);
struct isp_buffer *isp_buf_remove(struct isp_queue *q);

#endif //__VI_ISP_BUF_CTRL_H__

