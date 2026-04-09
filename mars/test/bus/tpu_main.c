#include "common.h"
#include "system_common.h"

#define TDMA_INTNUM     TDMA_INTERRUPT

// BD control bits base on BD_CTRL_BASE_ADDR
#define BD_TPU_EN					0    // TPU Enable bit
#define BD_LANE_NUM					22   // Lane number bit[29:22]
#define BD_DES_ADDR_VLD				30   // enable descriptor mode
#define BD_INTR_ENABLE				31   // TIU interrupt global enable

#define TDMA_CTRL_ENABLE_BIT        0
#define TDMA_CTRL_MODESEL_BIT       1
#define TDMA_CTRL_RESET_SYNCID_BIT  2
#define TDMA_CTRL_FORCE_1ARRAY      5
#define TDMA_CTRL_FORCE_2ARRAY      6
#define TDMA_CTRL_BURSTLEN_BIT      8
#define TDMA_CTRL_64BYTE_ALIGN_EN   10
#define TDMA_CTRL_INTRA_CMD_OFF     13
#define TDMA_CTRL_DESNUM_BIT        16

#define TDMA_MASK_INIT    0x20	//ignore descriptor nchw/stride=0 error
#define TDMA_INT_EOD      0x1
#define TDMA_INT_EOPMU    0x8000
#define TDMA_ALL_IDLE     0X1F

#define TDMA_CONTROL_BASE 0x0C100000
#define TPU_SRAM 0x0C000000
#define TIU_CONTROL_BASE    (0x0C101000 + 0x100)

#define TDMA_DMA_CONTROL    (TDMA_CONTROL_BASE + 0x000)
#define TDMA_DMA_INTMASK    (TDMA_CONTROL_BASE + 0x008)
#define TDMA_SYNC_STATS     (TDMA_CONTROL_BASE + 0x00C)
#define TDMA_CMD_ACCP0      (TDMA_CONTROL_BASE + 0x10)
#define TDMA_DEBUG_MODE     (TDMA_CONTROL_BASE + 0xA0)
#define TDMA_DCM_DISABLE    (TDMA_CONTROL_BASE + 0xA4)

typedef struct {
	uint32_t vld;
	uint32_t compress_en;
	uint32_t eod;
	uint32_t intp_en;
	uint32_t bar_en;
	uint32_t check_bf16_value;
	uint32_t trans_dir;
	uint32_t rsv00;
	uint32_t trans_fmt;
	uint32_t transpose_md;
	uint32_t rsv01;
	uint32_t intra_cmd_paral;
	uint32_t outstanding_en;
	uint32_t cmd_id;
	uint32_t spec_func;
	uint32_t dst_fmt;
	uint32_t src_fmt;
	uint32_t cmprs_fmt;
	uint32_t sys_dtype;
	uint32_t rsv2_1;
	uint32_t int8_sign;
	uint32_t compress_zero_guard;
	uint32_t int8_rnd_mode;
	uint32_t wait_id_tpu;
	uint32_t wait_id_other_tdma;
	uint32_t wait_id_sdma;
	uint32_t const_val;
	uint32_t src_base_reg_sel;
	uint32_t mv_lut_idx;
	uint32_t dst_base_reg_sel;
	uint32_t mv_lut_base;
	uint32_t rsv4_5;
	uint32_t dst_h_stride;
	uint32_t dst_c_stride_low;
	uint32_t dst_n_stride;
	uint32_t src_h_stride;
	uint32_t src_c_stride_low;
	uint32_t src_n_stride;
	uint32_t dst_c;
	uint32_t src_c;
	uint32_t dst_w;
	uint32_t dst_h;
	uint32_t src_w;
	uint32_t src_h;
	uint32_t dst_base_addr_low;
	uint32_t src_base_addr_low;
	uint32_t src_n;
	uint32_t dst_base_addr_high;
	uint32_t src_base_addr_high;
	uint32_t src_c_stride_high;
	uint32_t dst_c_stride_high;
	uint32_t compress_bias0;
	uint32_t compress_bias1;
	uint32_t layer_ID;
} tdma_reg_t;

static void emit_tdma_reg(const tdma_reg_t *r, uint32_t *_p)
{
	uint32_t *p = _p;

	p[15] = (r->compress_bias0 & ((1u << 8) - 1)) |
		((r->compress_bias1 & ((1u << 8) - 1)) << 8) |
		((r->layer_ID & ((1u << 16) - 1)) << 16);
	p[14] = (r->src_c_stride_high & ((1u << 16) - 1)) |
		((r->dst_c_stride_high & ((1u << 16) - 1)) << 16);
	p[13] = (r->src_n & ((1u << 16) - 1)) |
		((r->dst_base_addr_high & ((1u << 8) - 1)) << 16) |
		((r->src_base_addr_high & ((1u << 8) - 1)) << 24);
	p[12] = (r->src_base_addr_low & (((uint64_t)1 << 32) - 1));
	p[11] = (r->dst_base_addr_low & (((uint64_t)1 << 32) - 1));
	p[10] = (r->src_w & ((1u << 16) - 1)) |
		((r->src_h & ((1u << 16) - 1)) << 16);
	p[9] = (r->dst_w & ((1u << 16) - 1)) |
		((r->dst_h & ((1u << 16) - 1)) << 16);
	p[8] = (r->dst_c & ((1u << 16) - 1)) |
		((r->src_c & ((1u << 16) - 1)) << 16);
	p[7] = (r->src_n_stride & (((uint64_t)1 << 32) - 1));
	p[6] = (r->src_h_stride & ((1u << 16) - 1)) |
		((r->src_c_stride_low & ((1u << 16) - 1)) << 16);
	p[5] = (r->dst_n_stride & (((uint64_t)1 << 32) - 1));
	p[4] = (r->dst_h_stride & ((1u << 16) - 1)) |
		((r->dst_c_stride_low & ((1u << 16) - 1)) << 16);
	p[3] = (r->const_val & ((1u << 16) - 1)) |
		((r->src_base_reg_sel & ((1u << 3) - 1)) << 16) |
		((r->mv_lut_idx & 1) << 19) |
		((r->dst_base_reg_sel & ((1u << 3) - 1)) << 20) |
		((r->mv_lut_base & 1) << 23) |
		((r->rsv4_5 & ((1u << 8) - 1)) << 24);
	p[2] = (r->wait_id_other_tdma & ((1u << 16) - 1)) |
		((r->wait_id_sdma & ((1u << 16) - 1)) << 16);
	p[1] = (r->spec_func & ((1u << 3) - 1)) |
		((r->dst_fmt & ((1u << 2) - 1)) << 3) |
		((r->src_fmt & ((1u << 2) - 1)) << 5) |
		((r->cmprs_fmt & 1) << 7) |
		((r->sys_dtype & 1) << 8) |
		((r->rsv2_1 & ((1u << 4) - 1)) << 9) |
		((r->int8_sign & 1) << 13) |
		((r->compress_zero_guard & 1) << 14) |
		((r->int8_rnd_mode & 1) << 15) |
		((r->wait_id_tpu & ((1u << 16) - 1)) << 16);
	p[0] = (r->vld & 1) |
		((r->compress_en & 1) << 1) |
		((r->eod & 1) << 2) |
		((r->intp_en & 1) << 3) |
		((r->bar_en & 1) << 4) |
		((r->check_bf16_value & 1) << 5) |
		((r->trans_dir & ((1u << 2) - 1)) << 6) |
		((r->rsv00 & ((1u << 2) - 1)) << 8) |
		((r->trans_fmt & 1) << 10) |
		((r->transpose_md & ((1u << 2) - 1)) << 11) |
		((r->rsv01 & 1) << 13) |
		((r->intra_cmd_paral & 1) << 14) |
		((r->outstanding_en & 1) << 15) |
		((r->cmd_id & ((1u << 16) - 1)) << 16);
}

static void reset_tdma_reg(tdma_reg_t *r)
{
	r->vld = 0x0;
	r->compress_en = 0x0;
	r->eod = 0x0;
	r->intp_en = 0x0;
	r->bar_en = 0x0;
	r->check_bf16_value = 0x0;
	r->trans_dir = 0x0;
	r->rsv00 = 0x0;
	r->trans_fmt = 0x0;
	r->transpose_md = 0x0;
	r->rsv01 = 0x0;
	r->intra_cmd_paral = 0x0;
	r->outstanding_en = 0x0;
	r->cmd_id = 0x0;
	r->spec_func = 0x0;
	r->dst_fmt = 0x1;
	r->src_fmt = 0x1;
	r->cmprs_fmt = 0x0;
	r->sys_dtype = 0x0;
	r->rsv2_1 = 0x0;
	r->int8_sign = 0x0;
	r->compress_zero_guard = 0x0;
	r->int8_rnd_mode = 0x0;
	r->wait_id_tpu = 0x0;
	r->wait_id_other_tdma = 0x0;
	r->wait_id_sdma = 0x0;
	r->const_val = 0x0;
	r->src_base_reg_sel = 0x0;
	r->mv_lut_idx = 0x0;
	r->dst_base_reg_sel = 0x0;
	r->mv_lut_base = 0x0;
	r->rsv4_5 = 0x0;
	r->dst_h_stride = 0x1;
	r->dst_c_stride_low = 0x1;
	r->dst_n_stride = 0x1;
	r->src_h_stride = 0x1;
	r->src_c_stride_low = 0x1;
	r->src_n_stride = 0x1;
	r->dst_c = 0x1;
	r->src_c = 0x1;
	r->dst_w = 0x1;
	r->dst_h = 0x1;
	r->src_w = 0x1;
	r->src_h = 0x1;
	r->dst_base_addr_low = 0x0;
	r->src_base_addr_low = 0x0;
	r->src_n = 0x1;
	r->dst_base_addr_high = 0x0;
	r->src_base_addr_high = 0x0;
	r->src_c_stride_high = 0x0;
	r->dst_c_stride_high = 0x0;
	r->compress_bias0 = 0x0;
	r->compress_bias1 = 0x0;
	r->layer_ID = 0x0;
}

static void resync_cmd_id(void)
{
	uint32_t regVal = 0;

	//reset TIU ID
	regVal = RAW_READ32(TIU_CONTROL_BASE + 0xC);
	RAW_WRITE32(TIU_CONTROL_BASE + 0xC, regVal | 0x1);
	RAW_WRITE32(TIU_CONTROL_BASE + 0xC, regVal & ~0x1);

	regVal = RAW_READ32(TIU_CONTROL_BASE);
	RAW_WRITE32(TIU_CONTROL_BASE, regVal & ~((0x1 << BD_TPU_EN)
	| (0x1 << BD_DES_ADDR_VLD)));

	//reset TIU interrupt status
	regVal = RAW_READ32(TIU_CONTROL_BASE);
	RAW_WRITE32(TIU_CONTROL_BASE, regVal | (0x1 << 1));

	//reset DMA ID
	RAW_WRITE32(TDMA_DMA_CONTROL, 0x1 << TDMA_CTRL_RESET_SYNCID_BIT);
	RAW_WRITE32(TDMA_DMA_CONTROL, 0x0);

	//reset DMA interrupt status
	RAW_WRITE32(TDMA_DMA_CONTROL, 0xFFFF0000);
}

static void set_tdma_pio(uint32_t *pio_array)
{
	uint32_t i = 0;

	for (i = 0; i < 16; i++) {
		//pr_debug("addr=0x%px, value=0x%x\n", iomem_tdmaBase + TDMA_CMD_ACCP0 + (i << 2), pio_array[i]);
		RAW_WRITE32((uint32_t)(TDMA_CMD_ACCP0 + (i << 2)), pio_array[i]);
	}

	//make sure that debug mode is disable
	RAW_WRITE32(TDMA_DEBUG_MODE, 0x0);

	//tdma dcm enable
	RAW_WRITE32(TDMA_DCM_DISABLE, 0x0);

	//init interrupt mask
	RAW_WRITE32(TDMA_DMA_INTMASK, TDMA_MASK_INIT);

	RAW_WRITE32(TDMA_DMA_CONTROL,
		(0x1 << TDMA_CTRL_ENABLE_BIT) |
//		(0x1 << TDMA_CTRL_MODESEL_BIT) |
		(0x1 << TDMA_CTRL_DESNUM_BIT) |
		(0x3 << TDMA_CTRL_BURSTLEN_BIT) |
		(0x1 << TDMA_CTRL_FORCE_1ARRAY) |	//set 1 array
		(0x1 << TDMA_CTRL_INTRA_CMD_OFF) |	//force off intra_cmd feature
		(0x1 << TDMA_CTRL_64BYTE_ALIGN_EN));

#if 0
	for (i = 0; i < 2; i++) {
		pr_debug("addr=0x%px, value=0x%x\n",
		iomem_tdmaBase + TDMA_CTRL + (i << 2),
		RAW_READ32(iomem_tdmaBase + TDMA_CTRL + (i << 2)));
	}
#endif
}

int TPU_INTCallBack(int irqn, void *priv)
{
  uint32_t reg_value, int_status;

  //get interrupt status
  reg_value = RAW_READ32(TDMA_DMA_INTMASK);
  int_status = (reg_value >> 16) & ~(TDMA_MASK_INIT);
  printf("TPU_INTCallBack()=0x%x int_status=0x%x\n", reg_value, int_status);

  RAW_WRITE32(TDMA_DMA_INTMASK, 0xFFFF0000);
  printf("TPU_INTCallBack() done\n");
  return 0;
}

int TPU_Main(void)
{
  uint32_t pio_array[16] = {0};
  tdma_reg_t reg;

  uint8_t src_buf[0x100];
  uint8_t dst_buf[0x100];

  printf("TPU_Main()...\n");

  //init interrupt
  enable_irq(TDMA_INTNUM);
  request_irq(TDMA_INTNUM, TPU_INTCallBack, 0, "TDMAINT", NULL);

  reset_tdma_reg(&reg);
  reg.vld = 1;
  reg.trans_dir = 2; // 0:tg2l, 1:l2tg, 2:g2g, 3:l2l
  reg.src_base_addr_low = (uint32_t)(src_buf);
  reg.src_base_addr_high = 0;
  reg.dst_base_addr_low = (uint32_t)(dst_buf);
  reg.dst_base_addr_high = 0;
  reg.eod = 1;
  reg.intp_en = 1;
  reg.trans_fmt = 1; // 0:tensor, 1:common
  reg.src_n_stride = 0x100;

  emit_tdma_reg(&reg, pio_array);
  set_tdma_pio(pio_array);


  return 0;
}


