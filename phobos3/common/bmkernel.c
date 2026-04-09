#include <stdlib.h>
#include <string.h>
#include "bm_kernel.h"
#include "bmkernel.h"
#include "system_common.h"
#include "engine_io.h"
#include "common.h"
#include "debug.h"
#include "timer.h"

//#define DEBUG_BM_FW_CMD

static CMD_ID_NODE cmd_id_node;

static unsigned int get_value(u32 data, int start, int width)
{
  return (data >> start) & ((1 << width) - 1);
}

void handle_cmd_buf(void *cmd_buf, u32 len)
{
  u32 *cmd = (u32*)cmd_buf;
  int i = 0;
  cmd_hdr_t *hdr;

  while(i < len / 4) {
    hdr = (cmd_hdr_t *)(&cmd[i]);
    ASSERT(hdr->magic == 0xa5);
    i += sizeof(cmd_hdr_t) / 4;
    int cmd_len = hdr->len / 4;
    switch (hdr->engine_id) {
    case ENGINE_BD:
#ifdef DEBUG_BM_FW_CMD
      printf("CMD BD, len %d, node %d\n", cmd_len, hdr->node_id);
#endif
      //dump_task_descriptor("bd", ENGINE_BD, (P_TASK_DESCRIPTOR *)&cmd[i]);
      cmd_id_node.bd_cmd_id = get_value(((u32 *)(&cmd[i]))[0], 3, 16);
      call_single_cmd((P_TASK_DESCRIPTOR *)&cmd[i], ENGINE_BD); //CAUTION(wwcai): hdr->node_id is always 0 in firmware bk
      break;
    case ENGINE_GDMA:
#ifdef DEBUG_BM_FW_CMD
      printf("CMD GDMA, len %d, node %d\n", cmd_len, hdr->node_id);
#endif
      //dump_task_descriptor("gdma", ENGINE_GDMA, (P_TASK_DESCRIPTOR *)&cmd[i]);
      cmd_id_node.gdma_cmd_id = get_value(((u32 *)(&cmd[i]))[0], 16, 16);
      call_single_cmd((P_TASK_DESCRIPTOR *)&cmd[i], ENGINE_GDMA);
      break;
    case ENGINE_ARM:
      break;
    /*
     * workaround for bmkernel workaround for mdsfu.
    default:
      ASSERT(0);
    */
    }
    i += cmd_len;
  }
}

#if 0
static void* fw_alloc_cmd_buf(void *user_data, u32 size)
{
  void* cmd_buf = malloc(size);
#ifdef DEBUG_BM_FW_CMD
  printf("test alloc cmd_buf, len %d bytes, addr = %p\n", size, cmd_buf);
#endif
  return cmd_buf;
}

static void fw_emit_cmd_buf(void *user_data, void *cmd_buf, u32 len)
{
#ifdef DEBUG_BM_FW_CMD
  u32 *buf = (u32*)cmd_buf;
  printf("test emit cmd_buf, len %d bytes\n", len);
  for (int i = 0; i < len / sizeof(u32); i++) {
    printf("BUF %02d: 0x%08x\n", i, buf[i]);
  }
#endif

  handle_cmd_buf(cmd_buf, len);

  /* free implicitly */
#ifdef DEBUG_BM_FW_CMD
  printf("free cmd_buf, addr = %p\n", cmd_buf);
#endif
  free(cmd_buf);
}
#endif

static void* bmkernel_handle = NULL;
//static bmkernel_info_t bmkernel_info;
static bmk_info_t bmkernel_info;

void test_bmkernel_init(void)
{
  bmkernel_info.chip_version = 1880;
  bmkernel_info.cmdbuf_size = 10240;
  bmkernel_info.cmdbuf = (u8 *)malloc(bmkernel_info.cmdbuf_size);
  memset(bmkernel_info.cmdbuf, 0, bmkernel_info.cmdbuf_size);

  ASSERT(bmkernel_info.cmdbuf);

  bmkernel_handle = bmk_register(&bmkernel_info);

  resync_cmd_id(&cmd_id_node);
  bmk_enter(bmkernel_handle);
}


void test_bmkernel_submit(void)
{
  u32 size;
  u8 *cmdbuf = bmk_acquire_cmdbuf(&size);
  handle_cmd_buf(cmdbuf, size);

  free(bmkernel_info.cmdbuf);
  bmkernel_info.cmdbuf = (u8 *)malloc(bmkernel_info.cmdbuf_size);
  memset(bmkernel_info.cmdbuf, 0, bmkernel_info.cmdbuf_size);

  bmk_reset();
}

void bmkernel_runtime_exit(bmctx_t ctx, void *bk_ctx)
{
  bmk_cleanup(bk_ctx);
}


void test_bmkernel_exit(void)
{
  if (bmkernel_info.cmdbuf) {
    free(bmkernel_info.cmdbuf );
  }

  bmkernel_info.cmdbuf = NULL;
  bmk_cleanup(bmkernel_handle);
}
