#include "1880_test_util.h"
#include "engine_internal.h"
#include "engine_io.h"
#include "debug.h"
#include "system_common.h"
#include "bm1880_reg.h"
#include "timer.h"
#include "reg_bdcast.h"

#if 0
#define BD_CMD_REGI1                   0
#define BD_CMD_REGI2                   1
#define BD_CMD_REGI3                   2
#define BD_CMD_REGI4                   3
#define BD_CMD_REGI5                   4
#define BD_CMD_REGI6                   5
#define BD_CMD_REGI7                   6
#define BD_CMD_REGI8                   7
#define BD_CMD_REGI9                   8
#define BD_CMD_REGI10                  9
#define BD_CMD_REGI11                  10
#define BD_CMD_REGI12                  11
#define BD_CMD_REGI13                  12
#define BD_CMD_REGI14                  13
#define BD_CMD_REGI15                  14
#define BD_CMD_REGI16                  15
#define BD_CMD_REGI17                  16
#define BD_CMD_REGI18                  17
#define BD_CMD_REGI19                  18
#define BD_CMD_REGI20                  19
#define BD_CMD_REGI21                  20
#define BD_CMD_REGI22                  21
#define BD_CMD_REGI23                  22
#define BD_CMD_REGI24                  23

#define DCR_TYPE_CONV                  0
#define DCR_TYPE_CONV_COEFF            1
#define DCR_TYPE_MD_SUM                2
#define DCR_TYPE_MD_LINEAR             3
#define DCR_TYPE_MD_SCALAR             4
#define DCR_TYPE_MD_SFU                5
#define DCR_TYPE_MD_CMP                6
#define DCR_TYPE_POOLING_FWD           7
#define DCR_TYPE_POOLING_BWD           8
#define DCR_TYPE_MATRIX_MULTIPLY       9
#define DCR_TYPE_IMAGE_SUM             10
#define DCR_TYPE_CONV_COEFF_MAC        11
#define DCR_TYPE_LMEM_ARRANGE          12
#define DCR_TYPE_TENSOR_ARITHMETIC     13

#define GDMA_CMD_ACCPI0                0
#define GDMA_CMD_ACCPI1                1
#define GDMA_CMD_ACCPI2                2
#define GDMA_CMD_ACCPI3                3
#define GDMA_CMD_ACCPI4                4
#define GDMA_CMD_ACCPI5                5
#define GDMA_CMD_ACCPI6                6
#define GDMA_CMD_ACCPI7                7
#define GDMA_CMD_ACCPI8                8
#define GDMA_CMD_ACCPI9                9
#define GDMA_CMD_ACCPI10               10
#define GDMA_CMD_ACCPI11               11
#define GDMA_CMD_ACCPI12               12
#define GDMA_CMD_ACCPI13               13
#define GDMA_CMD_ACCPI14               14

static inline int desc_get_bd_dcr_type(void *desc)
{
  u32 *r = (u32 *)desc;
  return (r[BD_CMD_REGI1] >> 8) & 0xff;
}

static inline void desc_get_cmd_id(
    void *desc, int engine_id,
    uint32_t *bd_cmd_id, uint32_t *gdma_cmd_id)
{
  u32 *r = (u32 *)desc;
  if(ENGINE_BD == engine_id) {
    *bd_cmd_id = (r[BD_CMD_REGI1] >> 16) & 0x0ffff;
    *gdma_cmd_id = (r[BD_CMD_REGI23] >> 16) & 0x0ffff;
  } else if(ENGINE_GDMA == engine_id) {
    *bd_cmd_id = (r[GDMA_CMD_ACCPI1] >> 16) & 0x0ffff;
    *gdma_cmd_id = (r[GDMA_CMD_ACCPI0] >> 16) & 0x0ffff;
  }
}

static char *bd_type(void *desc)
{
  int dcr_type = desc_get_bd_dcr_type(desc);

  switch (dcr_type) {
    case DCR_TYPE_CONV:
      return "CONV";
    case DCR_TYPE_MD_SUM:
      return "MD_SUM";
    case DCR_TYPE_MD_LINEAR:
      return "MD_LINEAR";
    case DCR_TYPE_MD_SCALAR:
      return "MD_SCALAR ";
    case DCR_TYPE_MD_SFU:
      return "MD_SFU";
    case DCR_TYPE_MD_CMP:
      return "MD_CMP";
    case DCR_TYPE_POOLING_FWD:
      return "MD_POOLING_FWD";
    case DCR_TYPE_POOLING_BWD:
      return "MD_POOLING_BWD";
    case DCR_TYPE_MATRIX_MULTIPLY:
      return "MATMUL";
    case DCR_TYPE_IMAGE_SUM:
      return "IMG_SUM";
    case DCR_TYPE_CONV_COEFF:
      return "CONV_COEFF";
    case DCR_TYPE_CONV_COEFF_MAC:
      return "CONV_COEFF_MAC";
    case DCR_TYPE_LMEM_ARRANGE:
      return "LMEM_ARRANGE";
    case DCR_TYPE_TENSOR_ARITHMETIC:
      return "LMEM_ARITHMETIC";

    default:
      printf("Unknown BD dcr_type %d\n", dcr_type);
  }

  return "";
}

static void parse_gdma_operand_addr(void *desc, u64 *src, u64 *dst) {
  u32 *r = (u32 *)desc;
  *src = 0;
  *dst = 0;
  int direction = (int)((r[GDMA_CMD_ACCPI0] >> GDMA_ACCPI0_DIRECTION_BIT) & 0x3);
  switch (direction){
    case GDMA_DIR_S2L:
      *src = r[GDMA_CMD_ACCPI12] + (((u64)(r[GDMA_CMD_ACCPI13] & 0xff000000)) << 8);
      *dst = r[GDMA_CMD_ACCPI11];
      break;
    case GDMA_DIR_L2S:
      *src = r[GDMA_CMD_ACCPI12];
      *dst = r[GDMA_CMD_ACCPI11] + (((u64)(r[GDMA_CMD_ACCPI13] & 0x00ff0000)) << 16);
      break;
    case GDMA_DIR_S2S:
      *src = r[GDMA_CMD_ACCPI12] + (((u64)(r[GDMA_CMD_ACCPI13] & 0xff000000)) << 8);
      *dst = r[GDMA_CMD_ACCPI11] + (((u64)(r[GDMA_CMD_ACCPI13] & 0x00ff0000)) << 16);
      break;
    case GDMA_DIR_L2L:
      *src = r[GDMA_CMD_ACCPI12];
      *dst = r[GDMA_CMD_ACCPI11];
      break;
    default:
      break;
  }
}

static void parse_cmdbuf(uint8_t *cmdbuf, size_t cmdbuf_size)
{
  uint32_t *buf = (uint32_t *)cmdbuf;
  int i = 0;

  while(i < cmdbuf_size / 4) {
    cmd_hdr_t *hdr;
    void *cmd;
    hdr = (cmd_hdr_t *)(&buf[i]);
    //ASSERT(hdr->magic == 0xa5);
    i += sizeof(cmd_hdr_t) / 4;
    cmd = (void *)&buf[i];
    uint32_t cmd_len = hdr->len / 4;
    u64 src, dst;
    uint32_t bd_cmd_id = 0, gdma_cmd_id = 0;
    desc_get_cmd_id(cmd, hdr->engine_id, &bd_cmd_id, &gdma_cmd_id);

    switch (hdr->engine_id) {
    case ENGINE_BD:
      printf("CMD [BD], < %d, %d >, len %d %s\n", bd_cmd_id, gdma_cmd_id, cmd_len, bd_type(cmd));
      break;
    case ENGINE_GDMA:
      parse_gdma_operand_addr(cmd, &src, &dst);
      printf("CMD [GDMA], < %d, %d >, 0x%lx => 0x%lx, len %d\n", bd_cmd_id, gdma_cmd_id, src, dst, cmd_len);

      break;
    case ENGINE_ARM:
      printf("CMD [ARM ], len %d\n", cmd_len);
      break;
    default:
      printf("UNKNOWN CMD, len %d, engine_id, %d\n", cmd_len, (int)hdr->engine_id);
    }
    i += cmd_len;
  }
}
#endif

static void set_bdc_task_descriptor_dma(u64 desc_offset)
{
  //uartlog("set_bdc_task_descriptor_dma desc_offset:0x%lx\n", desc_offset);
  fw_write32(BD_ENGINE_DESC_ADDR, desc_offset);
  *(volatile u8 *)(BD_ENGINE_DESC_ADDR + 4) = (u8)(desc_offset >> 32);

  fw_write32(BD_ENGINE_MAIN_CTRL, fw_read32(BD_ENGINE_MAIN_CTRL) | (1 << 30) | (1 << 31));
}

static void set_gdma_task_descriptor_dma(u64 desc_offset)
{
  //uartlog("set_gdma_task_descriptor_dma desc_offset:0x%lx\n", desc_offset);
  fw_write32(GDMA_ENGINE_BASE_ADDR + 0x4, (desc_offset >> 6));
  fw_write32(GDMA_ENGINE_MAIN_CTRL, ((1 << 8) | 0x3));
}

static void reorder_tpu_cmdbuf_reg(uint8_t *cmdbuf)
{
  int total_bits = BD_REG_BYTES * 8;

  for (int i = 0; i < total_bits; i += 128)
    cmdbuf[(i + 128 - 8) / 8] |= (i / 128) << 4;

  uint8_t tmp[128 / 8];
  uint8_t *last = &cmdbuf[(total_bits - 128) / 8];
  memcpy(tmp, last, sizeof(tmp));
  memcpy(last, cmdbuf, sizeof(tmp));
  memcpy(cmdbuf, tmp, sizeof(tmp));
}

static void reorder_tpu_cmdbuf(uint8_t *cmdbuf, size_t sz)
{
  cmd_hdr_t *hdr = NULL;
  for (uint32_t i = 0; i < sz; i += sizeof(*hdr) + hdr->len) {
    hdr = (typeof(hdr))(&cmdbuf[i]);
    if (hdr->engine_id == ENGINE_BD)
      reorder_tpu_cmdbuf_reg(hdr->cmd);
  }
}

static void enable_gdma_cmdbuf_barrier(uint8_t *cmdbuf, size_t sz)
{
  cmd_hdr_t *hdr = NULL;
  for (uint32_t i = 0; i < sz; i += sizeof(*hdr) + hdr->len) {
    hdr = (typeof(hdr))(&cmdbuf[i]);
    if (hdr->engine_id == ENGINE_GDMA) {
      uint32_t *buf = (uint32_t *)hdr->cmd;
      buf[0] |= (1 << 4);
    }
  }
}

static void enable_interrupt(uint8_t *cmdbuf, size_t sz)
{
  cmd_hdr_t *hdr = NULL, *last_tpu = NULL, *last_gdma = NULL;
  for (uint32_t i = 0; i < sz; i += sizeof(*hdr) + hdr->len) {
    hdr = (typeof(hdr))(&cmdbuf[i]);
    if (hdr->engine_id == ENGINE_GDMA)
      last_gdma = hdr;
    else if (hdr->engine_id == ENGINE_BD)
      last_tpu = hdr;
  }

  if (last_tpu) {
    reg_t reg;
    parse_reg(&reg, (u32 *)last_tpu->cmd);
    reg.des_cmd_intr_en = 1;
    emit_reg(&reg, (u32 *)last_tpu->cmd);
  }

  if (last_gdma) {
    u32 *p = (u32 *)last_gdma->cmd;
    p[0] |= (1 << 3);
  }
}

static void set_eod(uint8_t *cmdbuf, size_t sz)
{
  cmd_hdr_t *hdr = NULL, *last_tpu = NULL, *last_gdma = NULL;
  for (uint32_t i = 0; i < sz; i += sizeof(*hdr) + hdr->len) {
    hdr = (typeof(hdr))(&cmdbuf[i]);
    if (hdr->engine_id == ENGINE_GDMA)
      last_gdma = hdr;
    else if (hdr->engine_id == ENGINE_BD)
      last_tpu = hdr;
  }

  if (last_tpu) {
    reg_t reg;
    parse_reg(&reg, (u32 *)last_tpu->cmd);
    reg.des_cmd_end = 1;
    emit_reg(&reg, (u32 *)last_tpu->cmd);
  }

  if (last_gdma) {
    u32 *p = (u32 *)last_gdma->cmd;
    p[0] |= (1 << 2);
  }
}

static void adjust_cmdbuf(uint8_t *cmdbuf, size_t sz)
{
  set_eod(cmdbuf, sz);
  enable_interrupt(cmdbuf, sz);
  enable_gdma_cmdbuf_barrier(cmdbuf, sz);

  /*
   * Must after all tpu cmdbuf adjustion
   */
  reorder_tpu_cmdbuf(cmdbuf, sz);
}

static int extract_cmdbuf(
    int engine_id,
    uint8_t *cmdbuf,
    uint8_t *found_cmdbuf,
    size_t sz,
    uint32_t *desc_count)
{
  int found_sz = 0;
  *desc_count = 0;
  cmd_hdr_t *hdr = NULL;
  for (uint32_t i = 0; i < sz; i += sizeof(*hdr) + hdr->len) {
    hdr = (typeof(hdr))(&cmdbuf[i]);
    if (hdr->engine_id == engine_id) {
      memcpy(&found_cmdbuf[found_sz], hdr->cmd, hdr->len);
      found_sz += hdr->len;
      (*desc_count)++;
    }
  }
  return found_sz;
}

static void cmdbuf_exec_dma(uint64_t bdc_desc_offset, uint32_t bdc_desc_num, uint64_t gdma_desc_offset, uint32_t gdma_desc_num)
{
  CMD_ID_NODE id_node;
  u64 tick_start, tick_done;

  uartlog("bdc_desc_offset %lx gdma_desc_offset %lx\n",
          bdc_desc_offset, gdma_desc_offset);

  uartlog("bdc_desc_num %d gdma_desc_num %d\n",
          bdc_desc_num, gdma_desc_num);

  resync_cmd_id(&id_node);

  id_node.bd_cmd_id = bdc_desc_num;
  id_node.gdma_cmd_id = gdma_desc_num;

  uartlog("%s %d: polling cmd id <bd:%d gdma:%d>\n",
            __func__, __LINE__,
            id_node.bd_cmd_id,
            id_node.gdma_cmd_id);

  tick_start = timer_get_tick();

  if (gdma_desc_num > 0) {
    set_gdma_task_descriptor_dma(gdma_desc_offset);
  }

  if(bdc_desc_num > 0) {
    set_bdc_task_descriptor_dma(bdc_desc_offset);
  }

  u64 idx = 0;

  while(1) {
    u32 bd_id = current_id((BD_ENGINE_MAIN_CTRL), 16, 6);
    u32 gdma_id = current_id((GDMA_ENGINE_BASE_ADDR + 0xC), 16, 16);
    if ( bd_id >= id_node.bd_cmd_id && gdma_id >= id_node.gdma_cmd_id) {
      tick_done = timer_get_tick();
      break;
    }

    idx ++;
    if ((idx % 10000) == 0)
      uartlog("current npu cmd id <bd:%u, gdma:%u>\n", bd_id, gdma_id);
  }

  poll_all_engine_done(&id_node);

  uartlog("Poll all engine done.\n");
  uartlog("Tick %lu(start %lu, done %lu)\n",
    tick_done - tick_start, tick_start, tick_done);
}

int bm_wait_cmdbuf_done(uint16_t seq_no) {
  return 0;
}

void handle_dma_cmd_buf(uint8_t *cmdbuf, size_t sz)
{
  uint32_t tpu_desc_cnt, gdma_desc_cnt;

  printf("handle_dma_cmd_buf %p\n", cmdbuf);

  adjust_cmdbuf(cmdbuf, sz);

  //uint8_t *tpu_cmdbuf = new uint8_t[sz];
  uint8_t *tpu_cmdbuf = (uint8_t *)0x1c3000000; //(uint8_t *)malloc(sz)
  int tpu_sz = extract_cmdbuf(ENGINE_BD, cmdbuf, tpu_cmdbuf, sz, &tpu_desc_cnt);
  printf("tpu sz %d\n", tpu_sz);

  //parse_cmdbuf(cmdbuf, sz);

  //uint8_t *gdma_cmdbuf = new uint8_t[sz];
  uint8_t *gdma_cmdbuf = (uint8_t *)0x1c4000000; //(uint8_t *)malloc(sz);
  int gdma_sz = extract_cmdbuf(ENGINE_GDMA, cmdbuf, gdma_cmdbuf, sz, &gdma_desc_cnt);
  printf("gdma sz %d\n", gdma_sz);

  cmdbuf_exec_dma((uint64_t)tpu_cmdbuf, tpu_desc_cnt, (uint64_t)gdma_cmdbuf, gdma_desc_cnt);

  //free(tpu_cmdbuf);
  //free(gdma_cmdbuf);
  return;
}