/*
 * Copyright CviTek Inc.
 *
 * Created Time: Feb, 2020
 */
#ifndef __CVI_H264_CFG_H__
#define __CVI_H264_CFG_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define CVI_DBG_MSG_ENABLE

#define CVI_MASK_ERR     0x1
#define CVI_MASK_INFO    0x2
#define CVI_MASK_FLOW    0x4
#define CVI_MASK_INTR    0x8
#define CVI_MASK_DBG     0x10
#define CVI_MASK_TRACE   0x20
#define CVI_MASK_PERF    0x40
#define CVI_MASK_CURR    (0x47)

#ifdef CVI_DBG_MSG_ENABLE
#define CVI_DBG_ERR(msg, ...)   if (CVI_MASK_CURR & CVI_MASK_ERR)    { printf("[ERR] %s = %d, "msg, __FUNCTION__, __LINE__, ## __VA_ARGS__); }
#define CVI_DBG_INFO(msg, ...)  if (CVI_MASK_CURR & CVI_MASK_INFO)   { printf("[INFO] %s = %d, "msg, __FUNCTION__, __LINE__, ## __VA_ARGS__); }
#define CVI_DBG_FLOW(msg, ...)  if (CVI_MASK_CURR & CVI_MASK_FLOW)   { printf("[FLOW] %s = %d, "msg, __FUNCTION__, __LINE__, ## __VA_ARGS__); }
#define CVI_DBG_INTR(msg, ...)  if (CVI_MASK_CURR & CVI_MASK_INTR)   { printf("[INTR] %s = %d, "msg, __FUNCTION__, __LINE__, ## __VA_ARGS__); }
#define CVI_DBG_MEM(msg, ...)   if (CVI_MASK_CURR & CVI_MASK_MEM)    { printf("[MEM] %s = %d, "msg, __FUNCTION__, __LINE__, ## __VA_ARGS__); }
#define CVI_DBG_DBG(msg, ...)   if (CVI_MASK_CURR & CVI_MASK_DBG)    { printf("[DBG] %s = %d, "msg, __FUNCTION__, __LINE__, ## __VA_ARGS__); }
#define CVI_DBG_TRACE(msg, ...)	if (CVI_MASK_CURR & CVI_MASK_TRACE)  { printf("[TRACE] %s = %d, "msg, __FUNCTION__, __LINE__, ## __VA_ARGS__); }
#define CVI_DBG_PERF(msg, ...)	if (CVI_MASK_CURR & CVI_MASK_PERF)   { printf("[PERF] %s = %d, "msg, __FUNCTION__, __LINE__, ## __VA_ARGS__); }
#define CVI_DBG_MCU(msg, ...)	if (CVI_MASK_CURR & CVI_MASK_MCU)    { printf("[MCU] %s = %d, "msg, __FUNCTION__, __LINE__, ## __VA_ARGS__); }
#else
#define CVI_DBG_ERR(msg, ...)
#define CVI_DBG_INFO(msg, ...)
#define CVI_DBG_FLOW(msg, ...)
#define CVI_DBG_MEM(msg, ...)
#define CVI_DBG_DBG(msg, ...)
#define CVI_DBG_TRACE(msg, ...)
#define CVI_DBG_PERF(msg, ...)
#define CVI_DBG_MCU(msg, ...)
#endif

#ifdef PLATFORM_NON_OS
#define FILE_BUFFER_BASE 0x12B000000
#define FILE_BUFFER_SIZE 0xA00000

#define DRAM_START              (FILE_BUFFER_BASE + FILE_BUFFER_SIZE)
//0x10B000000
#define WAVE420L_CODE_ADDR      (DRAM_START + 0x0)
#define CODA980_CODE_ADDR       (DRAM_START + 0x0)

#define VPU_DRAM_PHYSICAL_BASE	0x113100000
#define VPU_DRAM_SIZE			(128*1024*1024)
#define PROFILE_PERFORMANCE
#endif

#define SUPPORT_INTERRUPT
#define H264_VDEC_INTR_NUM  76

#ifdef TEST_H264_DEC
int cvitest_h264_vdec_main(int argc, char **argv);
#ifdef SUPPORT_INTERRUPT
int irq_handler_h264_vdec(int irqn, void *priv);
#endif
#endif

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
