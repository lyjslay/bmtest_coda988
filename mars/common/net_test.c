#include <string.h>
#include "boot_test.h"
#include "net_test.h"
#include "engine_internal.h"
#include "engine_io.h"
#include "debug.h"
#include "timer.h"

static void set_bdc_task_descriptor_dma(u64 desc_offset)
{
  uartlog("set_bdc_task_descriptor_dma desc_offset:0x%lx\n", desc_offset);
  //fw_write32(BD_ENGINE_DESC_ADDR, (desc_offset >> 8));

  fw_write32(BD_ENGINE_DESC_ADDR, desc_offset);
  *(volatile u8 *)(BD_ENGINE_DESC_ADDR + 4) = (u8)(desc_offset >> 32);

  fw_write32(BD_ENGINE_MAIN_CTRL, fw_read32(BD_ENGINE_MAIN_CTRL) | (1 << 30) | (1 << 31));
}

static void set_gdma_task_descriptor_dma(u64 desc_offset)
{
  uartlog("set_gdma_task_descriptor_dma desc_offset:0x%lx\n", desc_offset);
  fw_write32(GDMA_ENGINE_BASE_ADDR + 0x4, (desc_offset >> 6));
  fw_write32(GDMA_ENGINE_MAIN_CTRL, ((1 << 8) | 0x3));
}

static void cmdbuf_exec(DESC_OFFSET *desc_off, DESC_NUM *desc_num)
{
  CMD_ID_NODE id_node;

  int ms = 0;
  u64 tick_start, tick_done;
  u64 bdc_desc_offset = desc_off->bdc;
  u64 gdma_desc_offset = desc_off->gdma;

  uartlog("bdc_desc_offset %lu gdma_desc_offset %lu\n",
          bdc_desc_offset, gdma_desc_offset);

  int bdc_desc_num = desc_num->bdc;
  int gdma_desc_num = desc_num->gdma;

  uartlog("bdc_desc_num %d gdma_desc_num %d\n",
          bdc_desc_num, gdma_desc_num);

  resync_cmd_id(&id_node);

  id_node.bd_cmd_id = bdc_desc_num;
  id_node.gdma_cmd_id = gdma_desc_num;

  uartlog("%s %d: polling cmd id <bd:%d gdma:%d\n",
            __func__, __LINE__,
            id_node.bd_cmd_id,
            id_node.gdma_cmd_id);

  timer_meter_start();
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
      ms = timer_meter_get_ms();
      break;
    }

    idx ++;
    if ((idx % 2000000) == 0)
      uartlog("current npu cmd id <bd:%u, gdma:%u>\n", bd_id, gdma_id);
  }

  poll_all_engine_done(&id_node);

  uartlog("Poll all engine done.\n");
  uartlog("Tick %lu(start %lu, done %lu), Test duration:%d ms\n",
    tick_done - tick_start, tick_start, tick_done, ms);
}

#define ARM_ENGINE_DESCRIPTOR_NUM     32
#define ARM_CMD_ACCPI3                3
#define ARM_CMD_ACCPI4                4
#define ARM_CMD_ACCPI5                5
#define ARM_CMD_ACCPI6                6

typedef struct {
  u32 regs[7];
  union {
    u32 regs[ARM_ENGINE_DESCRIPTOR_NUM - 7];
    char str[100];
  } u;
} ARM_DESC;

static int validate_result(u64 out_addr, u64 ref_addr, int count)
{
  char *p_exp = (char *) ref_addr;
  char *p_got = (char *) out_addr;
  for (int idx = 0; idx < count; idx++) {
    if (p_exp[idx] != p_got[idx]){
      uartlog("Compare failed at index %d exp %d got %d\n",
             idx, p_exp[idx], p_got[idx]);
      return -1;
    }
  }

  uartlog("Compare pass\n");
  return 0;
}

static void dump_net_test_param(NET_INFO_T *net)
{
  uartlog("net test param:\n");
  uartlog("cmdbuf_addr      0x%lx\n", net->cmdbuf_addr);
  uartlog("weights_addr     0x%lx\n", net->weights_addr);
  uartlog("input_addr       0x%lx\n", net->input_addr);
  uartlog("output_ref_addr  0x%lx\n", net->output_ref_addr);
  uartlog("output_addr      0x%lx\n", net->output_addr);
  uartlog("output_size      0x%x\n", net->output_size);
  uartlog("bdc_desc_addr    0x%lx\n", net->desc_off.bdc);
  uartlog("gdma_desc_addr   0x%lx\n", net->desc_off.gdma);
  uartlog("bdc_desc_num     0x%x\n", net->desc_num.bdc);
  uartlog("gdma_desc_num    0x%x\n", net->desc_num.gdma);
}

int do_net_test(NET_INFO_T *net)
{
  u64 output_addr = net->output_addr;
  u32 output_size = net->output_size;
  u64 output_ref_addr = net->output_ref_addr;

  dump_net_test_param(net);
  memset((void *)(net->desc_off.bdc + net->desc_num.bdc * 28 * 4), 0, 128);
  cmdbuf_exec(&net->desc_off, &net->desc_num);

  int err = validate_result(output_addr, output_ref_addr, output_size / sizeof(char));
  if (err) {
    uartlog("Net test failed\n");
  }

  return err;
}
