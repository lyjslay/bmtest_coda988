#include <stdio.h>
#include <math.h>
#include <stdlib.h>
//#include <sys/mman.h>
#include "timer.h"
#include "module_testcase.h"
#include "fw_config.h"
#include<string.h>

#ifdef CALC_MD5
#include "md5.h"
#endif

#define VPP_SUPPORT_CMD_LIST_MODE      /*bm1880 vpp support cmd list mode and cpu config mode*/
//#define VPP_SCL_FILTER_COE_TEST             /*updata scl filter coef test*/
//#define VPP_DES_16B_ALIGNED_TEST         /*vpp descriptor addr 16B aligned test*/
//#define VPP_SOFT_REST_TEST                     /*vpp soft rest test(only rest aclk domain)*/
//#define VPP_MUTI_CROP_TEST                   /*vpp muti crop test:crop num:3, include data compare with cmodel*/
//#define VPP_MUTI_CROP_64_TEST                   /*vpp muti crop test:crop num:64, not include data compare with cmodel*/
//#define VPP_INT_TEST                                      /*test vpp int*/
//#define VPP_PERFORMANCE_TEST                   /*vpp performance test*/

/*****************************************/
#ifdef SLT_TEST
#define BASE_OFFSET          0x002000000
#else
#define BASE_OFFSET          0x000000000
#endif

#define VPP_SRC_ADDR0        0x100000000 + BASE_OFFSET
#define VPP_SRC_ADDR1        0x101000000 + BASE_OFFSET
#define VPP_SRC_ADDR2        0x102000000 + BASE_OFFSET

#define VPP_DST_ADDR0        0x103000000 + BASE_OFFSET
#define VPP_DST_ADDR1        0x104000000 + BASE_OFFSET
#define VPP_DST_ADDR2        0x105000000 + BASE_OFFSET

#define DES_CADDR0           0x106000000 + BASE_OFFSET

#define CMODEL_DST_ADDR0     0x107000000 + BASE_OFFSET
#define CMODEL_DST_ADDR1     0x108000000 + BASE_OFFSET
#define CMODEL_DST_ADDR2     0x109000000 + BASE_OFFSET

#define VPP_SRC_ADDR3        0x10a000000 + BASE_OFFSET
#define VPP_SRC_ADDR4        0x10a400000 + BASE_OFFSET
#define VPP_SRC_ADDR5        0x10a800000 + BASE_OFFSET

#define VPP_DST_ADDR3        0x10ac00000 + BASE_OFFSET
#define VPP_DST_ADDR4        0x10b000000 + BASE_OFFSET
#define VPP_DST_ADDR5        0x10b400000 + BASE_OFFSET

#define DES_CADDR1           0x10d000000 + BASE_OFFSET

#define CMODEL_DST_ADDR3     0x10c000000 + BASE_OFFSET
#define CMODEL_DST_ADDR4     0x10c400000 + BASE_OFFSET
#define CMODEL_DST_ADDR5     0x10c800000 + BASE_OFFSET

/*****************************************/

#define CMD_LIST_MODE   2
#define CPU_CONFIG_MODE   3

#define VPP_BASE         (0x58007000)
#define CROP_NUM_TEST         (64)
/*vpp global control*/
#define VPP_VERSION      (VPP_BASE + 0x000)
#define VPP_CONTROL0 (VPP_BASE + 0x004)
#define VPP_CONTROL1       (VPP_BASE + 0x008)
#define VPP_STATUS  (VPP_BASE + 0x00C)
#define VPP_INT_EN    (VPP_BASE + 0x010)
#define VPP_INT_CLEAR  (VPP_BASE + 0x014)
#define VPP_INT_STATUS    (VPP_BASE + 0x018)
#define VPP_INT_RAW_STATUS    (VPP_BASE + 0x01c)
#define VPP_CMD_BASE    (VPP_BASE + 0x020)
#define VPP_CMD_BASE_EXT    (VPP_BASE + 0x024)

/*Tile GDI2AXI control*/
#define VPP_GDI_XY2_CAS_0    (VPP_BASE + 0x030)
#define VPP_GDI_XY2_CAS_1    (VPP_BASE + 0x034)
#define VPP_GDI_XY2_CAS_2    (VPP_BASE + 0x038)
#define VPP_GDI_XY2_CAS_3    (VPP_BASE + 0x03c)
#define VPP_GDI_XY2_CAS_4    (VPP_BASE + 0x040)
#define VPP_GDI_XY2_CAS_5    (VPP_BASE + 0x044)
#define VPP_GDI_XY2_CAS_6    (VPP_BASE + 0x048)
#define VPP_GDI_XY2_CAS_7    (VPP_BASE + 0x04c)
#define VPP_GDI_XY2_CAS_8    (VPP_BASE + 0x050)
#define VPP_GDI_XY2_CAS_9    (VPP_BASE + 0x054)
#define VPP_GDI_XY2_CAS_10    (VPP_BASE + 0x058)
#define VPP_GDI_XY2_CAS_11    (VPP_BASE + 0x05c)
#define VPP_GDI_XY2_CAS_12    (VPP_BASE + 0x060)
#define VPP_GDI_XY2_CAS_13    (VPP_BASE + 0x064)
#define VPP_GDI_XY2_CAS_14    (VPP_BASE + 0x068)
#define VPP_GDI_XY2_CAS_15    (VPP_BASE + 0x06c)
#define VPP_GDI_XY2_BA_0    (VPP_BASE + 0x070)
#define VPP_GDI_XY2_BA_1    (VPP_BASE + 0x074)
#define VPP_GDI_XY2_BA_2    (VPP_BASE + 0x078)
#define VPP_GDI_XY2_BA_3    (VPP_BASE + 0x07c)
#define VPP_GDI_XY2_RAS_0    (VPP_BASE + 0x080)
#define VPP_GDI_XY2_RAS_1    (VPP_BASE + 0x084)
#define VPP_GDI_XY2_RAS_2    (VPP_BASE + 0x088)
#define VPP_GDI_XY2_RAS_3    (VPP_BASE + 0x08c)
#define VPP_GDI_XY2_RAS_4    (VPP_BASE + 0x090)
#define VPP_GDI_XY2_RAS_5    (VPP_BASE + 0x094)
#define VPP_GDI_XY2_RAS_6    (VPP_BASE + 0x098)
#define VPP_GDI_XY2_RAS_7    (VPP_BASE + 0x09c)
#define VPP_GDI_XY2_RAS_8    (VPP_BASE + 0x0a0)
#define VPP_GDI_XY2_RAS_9    (VPP_BASE + 0x0a4)
#define VPP_GDI_XY2_RAS_10    (VPP_BASE + 0x0a8)
#define VPP_GDI_XY2_RAS_11    (VPP_BASE + 0x0ac)
#define VPP_GDI_XY2_RAS_12    (VPP_BASE + 0x0b0)
#define VPP_GDI_XY2_RAS_13    (VPP_BASE + 0x0b4)
#define VPP_GDI_XY2_RAS_14    (VPP_BASE + 0x0b8)
#define VPP_GDI_XY2_RAS_15    (VPP_BASE + 0x0bc)
#define VPP_GDI_RBC2_AXI_0    (VPP_BASE + 0x0c0)
#define VPP_GDI_RBC2_AXI_1    (VPP_BASE + 0x0c4)
#define VPP_GDI_RBC2_AXI_2    (VPP_BASE + 0x0c8)
#define VPP_GDI_RBC2_AXI_3    (VPP_BASE + 0x0cc)
#define VPP_GDI_RBC2_AXI_4    (VPP_BASE + 0x0d0)
#define VPP_GDI_RBC2_AXI_5    (VPP_BASE + 0x0d4)
#define VPP_GDI_RBC2_AXI_6    (VPP_BASE + 0x0d8)
#define VPP_GDI_RBC2_AXI_7    (VPP_BASE + 0x0dc)
#define VPP_GDI_RBC2_AXI_8    (VPP_BASE + 0x0e0)
#define VPP_GDI_RBC2_AXI_9    (VPP_BASE + 0x0e4)
#define VPP_GDI_RBC2_AXI_10    (VPP_BASE + 0x0e8)
#define VPP_GDI_RBC2_AXI_11    (VPP_BASE + 0x0ec)
#define VPP_GDI_RBC2_AXI_12    (VPP_BASE + 0x0f0)
#define VPP_GDI_RBC2_AXI_13    (VPP_BASE + 0x0f4)
#define VPP_GDI_RBC2_AXI_14    (VPP_BASE + 0x0f8)
#define VPP_GDI_RBC2_AXI_15    (VPP_BASE + 0x0fc)
#define VPP_GDI_RBC2_AXI_16    (VPP_BASE + 0x0100)
#define VPP_GDI_RBC2_AXI_17    (VPP_BASE + 0x0104)
#define VPP_GDI_RBC2_AXI_18    (VPP_BASE + 0x0108)
#define VPP_GDI_RBC2_AXI_19    (VPP_BASE + 0x010c)
#define VPP_GDI_RBC2_AXI_20    (VPP_BASE + 0x0110)
#define VPP_GDI_RBC2_AXI_21    (VPP_BASE + 0x0114)
#define VPP_GDI_RBC2_AXI_22    (VPP_BASE + 0x0118)
#define VPP_GDI_RBC2_AXI_23    (VPP_BASE + 0x011c)
#define VPP_GDI_RBC2_AXI_24    (VPP_BASE + 0x0120)
#define VPP_GDI_RBC2_AXI_25    (VPP_BASE + 0x0124)
#define VPP_GDI_RBC2_AXI_26    (VPP_BASE + 0x0128)
#define VPP_GDI_RBC2_AXI_27    (VPP_BASE + 0x012c)
#define VPP_GDI_RBC2_AXI_28    (VPP_BASE + 0x0130)
#define VPP_GDI_RBC2_AXI_29    (VPP_BASE + 0x0134)
#define VPP_GDI_RBC2_AXI_30    (VPP_BASE + 0x0138)
#define VPP_GDI_RBC2_AXI_31    (VPP_BASE + 0x013c)
#define VPP_GDI_XY2_RBC_CONFIG    (VPP_BASE + 0x0140)
#define VPP_GDI_TILEDBUF_BASE    (VPP_BASE + 0x0144)

/*vpp descriptor for DMA CMD LIST*/
#define VPP_DES_HEAD    (VPP_BASE + 0x0300)
#define VPP_NEXT_CMD_BASE    (VPP_BASE + 0x0304)
#define VPP_NEXT_CMD_BASE_EXT    (VPP_BASE + 0x0308)
#define VPP_PADDING_CTRL   (VPP_BASE + 0x030c)


#define VPP_CSC_COE_00   (VPP_BASE + 0x0310)
#define VPP_CSC_COE_01   (VPP_BASE + 0x0314)
#define VPP_CSC_COE_02   (VPP_BASE + 0x0318)
#define VPP_CSC_ADD_0   (VPP_BASE + 0x031c)
#define VPP_CSC_COE_10   (VPP_BASE + 0x0320)
#define VPP_CSC_COE_11   (VPP_BASE + 0x0324)
#define VPP_CSC_COE_12   (VPP_BASE + 0x0328)
#define VPP_CSC_ADD_1   (VPP_BASE + 0x032c)
#define VPP_CSC_COE_20   (VPP_BASE + 0x0330)
#define VPP_CSC_COE_21   (VPP_BASE + 0x0334)
#define VPP_CSC_COE_22   (VPP_BASE + 0x0338)
#define VPP_CSC_ADD_2   (VPP_BASE + 0x033c)


#define VPP_SRC_CTRL (VPP_BASE + 0x0340)
#define VPP_SRC_SIZE (VPP_BASE + 0x0344)
#define VPP_SRC_CROP_ST (VPP_BASE + 0x0348)
#define VPP_SRC_CROP_SIZE (VPP_BASE + 0x034c)
#define VPP_SRC_RY_BASE    (VPP_BASE + 0x0350)
#define VPP_SRC_GU_BASE    (VPP_BASE + 0x0354)
#define VPP_SRC_BV_BASE    (VPP_BASE + 0x0358)
#define VPP_SRC_STRIDE    (VPP_BASE + 0x035c)
#define VPP_SRC_RY_BASE_EXT    (VPP_BASE + 0x0360)
#define VPP_SRC_GU_BASE_EXT    (VPP_BASE + 0x0364)
#define VPP_SRC_BV_BASE_EXT    (VPP_BASE + 0x0368)


#define VPP_DST_CTRL (VPP_BASE + 0x0370)
#define VPP_DST_SIZE (VPP_BASE + 0x0374)
#define VPP_DST_CROP_ST (VPP_BASE + 0x0378)
#define VPP_DST_CROP_SIZE (VPP_BASE + 0x037c)
#define VPP_DST_RY_BASE    (VPP_BASE + 0x0380)
#define VPP_DST_GU_BASE    (VPP_BASE + 0x0384)
#define VPP_DST_BV_BASE    (VPP_BASE + 0x0388)
#define VPP_DST_STRIDE    (VPP_BASE + 0x038c)
#define VPP_DST_RY_BASE_EXT    (VPP_BASE + 0x0390)
#define VPP_DST_GU_BASE_EXT    (VPP_BASE + 0x0394)
#define VPP_DST_BV_BASE_EXT    (VPP_BASE + 0x0398)
#define VPP_DST_PADDING_HEIGHT    (VPP_BASE + 0x039c)

#define VPP_SCL_CTRL   (VPP_BASE + 0x03a0)
#define VPP_SCL_INIT   (VPP_BASE + 0x03a4)
#define VPP_SCL_X   (VPP_BASE + 0x03a8)
#define VPP_SCL_Y   (VPP_BASE + 0x03ac)

#define VPP_SCL_Y_COE_P0C0   (VPP_BASE + 0x03b0)
#define VPP_SCL_Y_COE_P0C2   (VPP_BASE + 0x03b4)
#define VPP_SCL_Y_COE_P1C0   (VPP_BASE + 0x03b8)
#define VPP_SCL_Y_COE_P1C2   (VPP_BASE + 0x03bc)
#define VPP_SCL_Y_COE_P2C0   (VPP_BASE + 0x03c0)
#define VPP_SCL_Y_COE_P2C2   (VPP_BASE + 0x03c4)
#define VPP_SCL_Y_COE_P3C0   (VPP_BASE + 0x03c8)
#define VPP_SCL_Y_COE_P3C2   (VPP_BASE + 0x03cc)
#define VPP_SCL_Y_COE_P4C0   (VPP_BASE + 0x03d0)
#define VPP_SCL_Y_COE_P4C2   (VPP_BASE + 0x03d4)
#define VPP_SCL_Y_COE_P5C0   (VPP_BASE + 0x03d8)
#define VPP_SCL_Y_COE_P5C2   (VPP_BASE + 0x03dc)
#define VPP_SCL_Y_COE_P6C0   (VPP_BASE + 0x03e0)
#define VPP_SCL_Y_COE_P6C2   (VPP_BASE + 0x03e4)
#define VPP_SCL_Y_COE_P7C0   (VPP_BASE + 0x03e8)
#define VPP_SCL_Y_COE_P7C2   (VPP_BASE + 0x03ec)
#define VPP_SCL_Y_COE_P8C0   (VPP_BASE + 0x03f0)
#define VPP_SCL_Y_COE_P8C2   (VPP_BASE + 0x03f4)
#define VPP_SCL_Y_COE_P9C0   (VPP_BASE + 0x03f8)
#define VPP_SCL_Y_COE_P9C2   (VPP_BASE + 0x03fc)
#define VPP_SCL_Y_COE_PAC0   (VPP_BASE + 0x0400)
#define VPP_SCL_Y_COE_PAC2   (VPP_BASE + 0x0404)
#define VPP_SCL_Y_COE_PBC0   (VPP_BASE + 0x0408)
#define VPP_SCL_Y_COE_PBC2   (VPP_BASE + 0x040c)
#define VPP_SCL_Y_COE_PCC0   (VPP_BASE + 0x0410)
#define VPP_SCL_Y_COE_PCC2   (VPP_BASE + 0x0414)
#define VPP_SCL_Y_COE_PDC0   (VPP_BASE + 0x0418)
#define VPP_SCL_Y_COE_PDC2   (VPP_BASE + 0x041c)
#define VPP_SCL_Y_COE_PEC0   (VPP_BASE + 0x0420)
#define VPP_SCL_Y_COE_PEC2   (VPP_BASE + 0x0424)
#define VPP_SCL_Y_COE_PFC0   (VPP_BASE + 0x0428)
#define VPP_SCL_Y_COE_PFC2   (VPP_BASE + 0x042c)

#define VPP_SCL_X_COE_P0C0   (VPP_BASE + 0x0430)
#define VPP_SCL_X_COE_P0C2   (VPP_BASE + 0x0434)
#define VPP_SCL_X_COE_P0C4   (VPP_BASE + 0x0438)
#define VPP_SCL_X_COE_P0C6   (VPP_BASE + 0x043c)
#define VPP_SCL_X_COE_P1C0   (VPP_BASE + 0x0440)
#define VPP_SCL_X_COE_P1C2   (VPP_BASE + 0x0444)
#define VPP_SCL_X_COE_P1C4   (VPP_BASE + 0x0448)
#define VPP_SCL_X_COE_P1C6   (VPP_BASE + 0x044c)
#define VPP_SCL_X_COE_P2C0   (VPP_BASE + 0x0450)
#define VPP_SCL_X_COE_P2C2   (VPP_BASE + 0x0454)
#define VPP_SCL_X_COE_P2C4   (VPP_BASE + 0x0458)
#define VPP_SCL_X_COE_P2C6   (VPP_BASE + 0x045c)
#define VPP_SCL_X_COE_P3C0   (VPP_BASE + 0x0460)
#define VPP_SCL_X_COE_P3C2   (VPP_BASE + 0x0464)
#define VPP_SCL_X_COE_P3C4   (VPP_BASE + 0x0468)
#define VPP_SCL_X_COE_P3C6   (VPP_BASE + 0x046c)
#define VPP_SCL_X_COE_P4C0   (VPP_BASE + 0x0470)
#define VPP_SCL_X_COE_P4C2   (VPP_BASE + 0x0474)
#define VPP_SCL_X_COE_P4C4   (VPP_BASE + 0x0478)
#define VPP_SCL_X_COE_P4C6   (VPP_BASE + 0x047c)
#define VPP_SCL_X_COE_P5C0   (VPP_BASE + 0x0480)
#define VPP_SCL_X_COE_P5C2   (VPP_BASE + 0x0484)
#define VPP_SCL_X_COE_P5C4   (VPP_BASE + 0x0488)
#define VPP_SCL_X_COE_P5C6   (VPP_BASE + 0x048c)
#define VPP_SCL_X_COE_P6C0   (VPP_BASE + 0x0490)
#define VPP_SCL_X_COE_P6C2   (VPP_BASE + 0x0494)
#define VPP_SCL_X_COE_P6C4   (VPP_BASE + 0x0498)
#define VPP_SCL_X_COE_P6C6   (VPP_BASE + 0x049c)
#define VPP_SCL_X_COE_P7C0   (VPP_BASE + 0x04a0)
#define VPP_SCL_X_COE_P7C2   (VPP_BASE + 0x04a4)
#define VPP_SCL_X_COE_P7C4   (VPP_BASE + 0x04a8)
#define VPP_SCL_X_COE_P7C6   (VPP_BASE + 0x04ac)
#define VPP_SCL_X_COE_P8C0   (VPP_BASE + 0x04b0)
#define VPP_SCL_X_COE_P8C2   (VPP_BASE + 0x04b4)
#define VPP_SCL_X_COE_P8C4   (VPP_BASE + 0x04b8)
#define VPP_SCL_X_COE_P8C6   (VPP_BASE + 0x04bc)
#define VPP_SCL_X_COE_P9C0   (VPP_BASE + 0x04c0)
#define VPP_SCL_X_COE_P9C2   (VPP_BASE + 0x04c4)
#define VPP_SCL_X_COE_P9C4   (VPP_BASE + 0x04c8)
#define VPP_SCL_X_COE_P9C6   (VPP_BASE + 0x04cc)
#define VPP_SCL_X_COE_PAC0   (VPP_BASE + 0x04d0)
#define VPP_SCL_X_COE_PAC2   (VPP_BASE + 0x04d4)
#define VPP_SCL_X_COE_PAC4   (VPP_BASE + 0x04d8)
#define VPP_SCL_X_COE_PAC6   (VPP_BASE + 0x04dc)
#define VPP_SCL_X_COE_PBC0   (VPP_BASE + 0x04e0)
#define VPP_SCL_X_COE_PBC2   (VPP_BASE + 0x04e4)
#define VPP_SCL_X_COE_PBC4   (VPP_BASE + 0x04e8)
#define VPP_SCL_X_COE_PBC6   (VPP_BASE + 0x04ec)
#define VPP_SCL_X_COE_PCC0   (VPP_BASE + 0x04f0)
#define VPP_SCL_X_COE_PCC2   (VPP_BASE + 0x04f4)
#define VPP_SCL_X_COE_PCC4   (VPP_BASE + 0x04f8)
#define VPP_SCL_X_COE_PCC6   (VPP_BASE + 0x04fc)
#define VPP_SCL_X_COE_PDC0   (VPP_BASE + 0x0500)
#define VPP_SCL_X_COE_PDC2   (VPP_BASE + 0x0504)
#define VPP_SCL_X_COE_PDC4   (VPP_BASE + 0x0508)
#define VPP_SCL_X_COE_PDC6   (VPP_BASE + 0x050c)
#define VPP_SCL_X_COE_PEC0   (VPP_BASE + 0x0510)
#define VPP_SCL_X_COE_PEC2   (VPP_BASE + 0x0514)
#define VPP_SCL_X_COE_PEC4   (VPP_BASE + 0x0518)
#define VPP_SCL_X_COE_PEC6   (VPP_BASE + 0x051c)
#define VPP_SCL_X_COE_PFC0   (VPP_BASE + 0x0520)
#define VPP_SCL_X_COE_PFC2   (VPP_BASE + 0x0524)
#define VPP_SCL_X_COE_PFC4   (VPP_BASE + 0x0528)
#define VPP_SCL_X_COE_PFC6   (VPP_BASE + 0x052c)

#define REG(x)           (*(volatile uint32 *)(x))

// 1684
#define YUV420        0
#define YOnly         1
#define YUV444        3
#define RGB24         4
#define ARGB32        5

#define YUV2RGB          2
#define RGB2YUV444       3

typedef unsigned char uint8;
typedef unsigned int uint32;
typedef unsigned long long uint64;
typedef unsigned long long      u64;

typedef struct rect {
  int format;
  int stride;
  int videosize;
  uint64 addr0;
  uint64 addr1;
  uint64 addr2;
  int axisX;
  int axisY;
  int cropW;
  int cropH;
  int endian;
  int endian_a;
  int plannar;
  int a_val;
  int tile_mode;
  int scl_ctrl;
  int scl_init;
}rect_t;
typedef struct tile_gdi {
  uint32 xy2_cas0;
  uint32 xy2_cas1;
  uint32 xy2_cas2;
  uint32 xy2_cas3;
  uint32 xy2_cas4;
  uint32 xy2_cas5;
  uint32 xy2_cas6;
  uint32 xy2_cas7;
  uint32 xy2_cas8;
  uint32 xy2_cas9;
  uint32 xy2_cas10;
  uint32 xy2_cas11;
  uint32 xy2_cas12;
  uint32 xy2_cas13;
  uint32 xy2_cas14;
  uint32 xy2_cas15;
  uint32 xy2_ba0;
  uint32 xy2_ba1;
  uint32 xy2_ba2;
  uint32 xy2_ba3;
  uint32 xy2_ras0;
  uint32 xy2_ras1;
  uint32 xy2_ras2;
  uint32 xy2_ras3;
  uint32 xy2_ras4;
  uint32 xy2_ras5;
  uint32 xy2_ras6;
  uint32 xy2_ras7;
  uint32 xy2_ras8;
  uint32 xy2_ras9;
  uint32 xy2_ras10;
  uint32 xy2_ras11;
  uint32 xy2_ras12;
  uint32 xy2_ras13;
  uint32 xy2_ras14;
  uint32 xy2_ras15;
  uint32 rbc2_axi0;
  uint32 rbc2_axi1;
  uint32 rbc2_axi2;
  uint32 rbc2_axi3;
  uint32 rbc2_axi4;
  uint32 rbc2_axi5;
  uint32 rbc2_axi6;
  uint32 rbc2_axi7;
  uint32 rbc2_axi8;
  uint32 rbc2_axi9;
  uint32 rbc2_axi10;
  uint32 rbc2_axi11;
  uint32 rbc2_axi12;
  uint32 rbc2_axi13;
  uint32 rbc2_axi14;
  uint32 rbc2_axi15;
  uint32 rbc2_axi16;
  uint32 rbc2_axi17;
  uint32 rbc2_axi18;
  uint32 rbc2_axi19;
  uint32 rbc2_axi20;
  uint32 rbc2_axi21;
  uint32 rbc2_axi22;
  uint32 rbc2_axi23;
  uint32 rbc2_axi24;
  uint32 rbc2_axi25;
  uint32 rbc2_axi26;
  uint32 rbc2_axi27;
  uint32 rbc2_axi28;
  uint32 rbc2_axi29;
  uint32 rbc2_axi30;
  uint32 rbc2_axi31;
  uint32 xy2_rbc_config;
  uint32 tiledbuf_base;
}gdi_t;
typedef struct Des {
  uint32 des_head;
  uint32 next_cmd_base;
  uint32 next_cmd_base_ext;
  uint32 padding_ctrl;
  uint32 csc_coe00;
  uint32 csc_coe01;
  uint32 csc_coe02;
  uint32 csc_add0;
  uint32 csc_coe10;
  uint32 csc_coe11;
  uint32 csc_coe12;
  uint32 csc_add1;
  uint32 csc_coe20;
  uint32 csc_coe21;
  uint32 csc_coe22;
  uint32 csc_add2;
  uint32 src_ctrl;
  uint32 src_size;
  uint32 src_crop_st;
  uint32 src_crop_size;
  uint32 src_ry_base;
  uint32 src_gu_base;
  uint32 src_bv_base;
  uint32 src_stride;
  uint32 src_ry_base_ext;
  uint32 src_gu_base_ext;
  uint32 src_bv_base_ext;
  uint32 rsv;
  uint32 dst_ctrl;
  uint32 dst_size;
  uint32 dst_crop_st;
  uint32 dst_crop_size;
  uint32 dst_ry_base;
  uint32 dst_gu_base;
  uint32 dst_bv_base;
  uint32 dst_stride;
  uint32 dst_ry_base_ext;
  uint32 dst_gu_base_ext;
  uint32 dst_bv_base_ext;
  uint32 dst_padding_height;
  uint32 scl_ctrl;
  uint32 scl_int;
  uint32 scl_x;
  uint32 scl_y;
  uint32 scl_y_coe_p0c0;
  uint32 scl_y_coe_p0c2;
  uint32 scl_y_coe_p1c0;
  uint32 scl_y_coe_p1c2;
  uint32 scl_y_coe_p2c0;
  uint32 scl_y_coe_p2c2;
  uint32 scl_y_coe_p3c0;
  uint32 scl_y_coe_p3c2;
  uint32 scl_y_coe_p4c0;
  uint32 scl_y_coe_p4c2;
  uint32 scl_y_coe_p5c0;
  uint32 scl_y_coe_p5c2;
  uint32 scl_y_coe_p6c0;
  uint32 scl_y_coe_p6c2;
  uint32 scl_y_coe_p7c0;
  uint32 scl_y_coe_p7c2;
  uint32 scl_y_coe_p8c0;
  uint32 scl_y_coe_p8c2;
  uint32 scl_y_coe_p9c0;
  uint32 scl_y_coe_p9c2;
  uint32 scl_y_coe_pac0;
  uint32 scl_y_coe_pac2;
  uint32 scl_y_coe_pbc0;
  uint32 scl_y_coe_pbc2;
  uint32 scl_y_coe_pcc0;
  uint32 scl_y_coe_pcc2;
  uint32 scl_y_coe_pdc0;
  uint32 scl_y_coe_pdc2;
  uint32 scl_y_coe_pec0;
  uint32 scl_y_coe_pec2;
  uint32 scl_y_coe_pfc0;
  uint32 scl_y_coe_pfc2;
  uint32 scl_x_coe_p0c0;
  uint32 scl_x_coe_p0c2;
  uint32 scl_x_coe_p0c4;
  uint32 scl_x_coe_p0c6;
  uint32 scl_x_coe_p1c0;
  uint32 scl_x_coe_p1c2;
  uint32 scl_x_coe_p1c4;
  uint32 scl_x_coe_p1c6;
  uint32 scl_x_coe_p2c0;
  uint32 scl_x_coe_p2c2;
  uint32 scl_x_coe_p2c4;
  uint32 scl_x_coe_p2c6;
  uint32 scl_x_coe_p3c0;
  uint32 scl_x_coe_p3c2;
  uint32 scl_x_coe_p3c4;
  uint32 scl_x_coe_p3c6;
  uint32 scl_x_coe_p4c0;
  uint32 scl_x_coe_p4c2;
  uint32 scl_x_coe_p4c4;
  uint32 scl_x_coe_p4c6;
  uint32 scl_x_coe_p5c0;
  uint32 scl_x_coe_p5c2;
  uint32 scl_x_coe_p5c4;
  uint32 scl_x_coe_p5c6;
  uint32 scl_x_coe_p6c0;
  uint32 scl_x_coe_p6c2;
  uint32 scl_x_coe_p6c4;
  uint32 scl_x_coe_p6c6;
  uint32 scl_x_coe_p7c0;
  uint32 scl_x_coe_p7c2;
  uint32 scl_x_coe_p7c4;
  uint32 scl_x_coe_p7c6;
  uint32 scl_x_coe_p8c0;
  uint32 scl_x_coe_p8c2;
  uint32 scl_x_coe_p8c4;
  uint32 scl_x_coe_p8c6;
  uint32 scl_x_coe_p9c0;
  uint32 scl_x_coe_p9c2;
  uint32 scl_x_coe_p9c4;
  uint32 scl_x_coe_p9c6;
  uint32 scl_x_coe_pac0;
  uint32 scl_x_coe_pac2;
  uint32 scl_x_coe_pac4;
  uint32 scl_x_coe_pac6;
  uint32 scl_x_coe_pbc0;
  uint32 scl_x_coe_pbc2;
  uint32 scl_x_coe_pbc4;
  uint32 scl_x_coe_pbc6;
  uint32 scl_x_coe_pcc0;
  uint32 scl_x_coe_pcc2;
  uint32 scl_x_coe_pcc4;
  uint32 scl_x_coe_pcc6;
  uint32 scl_x_coe_pdc0;
  uint32 scl_x_coe_pdc2;
  uint32 scl_x_coe_pdc4;
  uint32 scl_x_coe_pdc6;
  uint32 scl_x_coe_pec0;
  uint32 scl_x_coe_pec2;
  uint32 scl_x_coe_pec4;
  uint32 scl_x_coe_pec6;
  uint32 scl_x_coe_pfc0;
  uint32 scl_x_coe_pfc2;
  uint32 scl_x_coe_pfc4;
  uint32 scl_x_coe_pfc6;
}Descriptor_t;


static const uint32 CSC_COE[9] =
{ 0x04a8, 0x0000, 0x0662,
  0x04a8, 0x0191, 0x0341,
  0x04a8, 0x0811, 0x0000 };

static const uint32 CSC_ADD[3] =
{ 0x010dee98, 0x000879db, 0x01114ccd };

#ifdef VPP_SCL_FILTER_COE_TEST
static const uint32 SCL_X_COE[64] =
{ 0xffea000c, 0x00d8001e, 0xffdb009c, 0xffff000a,
   0xffec000c, 0x00d80018, 0xffe80025, 0xfffe000d,
   0xffee000b, 0x00da0012, 0xffe5002d, 0xfffb000e,
   0xfff0000a, 0x00d9000c, 0xffe30035, 0xfffb000e,
   0xfff3000a, 0x00d60006, 0xffe1003c, 0xfffb000f,
   0xfff50009, 0x00d40000, 0xffe00044, 0xfffb000f,
   0xfff70008, 0x00d2fffb, 0xffde004c, 0xfffb000f,
   0xfffa0007, 0x00cefff6, 0xffdc0054, 0xfffb000f,
   0xfffc0006, 0x00cbfff1, 0xffdb005d, 0xfffb000f,
   0xfffe0005, 0x00c6ffed, 0xffda0065, 0xfffc000f,
   0x00000004, 0x00c1ffe9, 0xffd9006d, 0xfffc000f,
   0x00020003, 0x00bcffe6, 0xffd90075, 0xfffd000e,
   0x00040003, 0x00b6ffe3, 0xffd9007d, 0xfffd000d,
   0x00050002, 0x00b0ffe1, 0xffd90085, 0xfffd000d,
   0x00070001, 0x00aaffde, 0xffd9008d, 0xfffe000c,
   0x00090000, 0x00a2ffdd, 0xffda0094, 0xffff000b };

static const uint32 SCL_Y_COE[32] =
{ 0x01000000, 0xffe80098, 0x00fffffa, 0x00000007,
   0x00fefff5, 0xffff000e, 0x00fbfff1, 0xfffe0016,
   0x00f8ffed, 0xfffd001e, 0x00f3ffeb, 0xfffc0026,
   0x00eeffe8, 0xfffa0030, 0x00e8ffe6, 0xfff90039,
   0x00e1ffe5, 0xfff70043, 0x00d9ffe4, 0xfff5004e,
   0x00d2ffe3, 0xfff30058, 0x00c9ffe4, 0xfff10062,
   0x00c0ffe4, 0xffef006d, 0x00b6ffe5, 0xffed0078,
   0x00adffe5, 0xffeb0083, 0x00a2ffe7, 0xffea008d };
#endif

typedef struct vpp1684_param
{
     uint32          CROP_NUM;
     uint32          SCA_CTRL;

     uint32          PADDING_CTRL;
     uint32          SRC_CTRL;
     uint32          SRC_SIZE;
     uint32          *SRC_CROP_ST;
     uint32          *SRC_CROP_SIZE;
     uint32          SRC_RY_BASE;
     uint32          SRC_GU_BASE;
     uint32          SRC_BV_BASE;
     uint32          SRC_STRIDE;
     uint32          SRC_RY_BASE_EXT;
     uint32          SRC_GU_BASE_EXT;
     uint32          SRC_BV_BASE_EXT;

     uint32          DST_CTRL;
     uint32          DST_SIZE;
     uint32          *DST_CROP_ST;
     uint32          *DST_CROP_SIZE;
     uint32          *DST_RY_BASE;
     uint32          *DST_GU_BASE;
     uint32          *DST_BV_BASE;
     uint32          *DST_STRIDE;
     uint32          *DST_RY_BASE_EXT;
     uint32          *DST_GU_BASE_EXT;
     uint32          *DST_BV_BASE_EXT;
     uint32          *DST_PADDING_HEIGHT;

     uint32          *SCL_CTRL;
     uint32          *SCL_INIT;
     uint32          *SCL_X;
     uint32          *SCL_Y;
}VPP1684_PARAM;

#define LV_ERROR 0x1
#define LV_DEBUG 0x2

#define PRINT_ERROR(lv, msg, ...) if (1)             { printf("[ERROR] "msg, ## __VA_ARGS__); }
#define PRINT_DEBUG(lv, msg, ...) if (lv & LV_DEBUG) { printf("[DEBUG] "msg, ## __VA_ARGS__); }

#ifdef RANDOM_INPUT
static unsigned long int next = 1;
void generate_random_input(struct rect *src, struct rect *dst);
int my_rand(void);
void my_srand(unsigned int seed);
int debug_level = 0x0;
#else
int debug_level = 0x3;
#endif

#ifdef CALC_MD5
int checkMD5(struct rect *dst, char *pGolden);
void calculateMD5(Uint8 *pAddr, int size, unsigned char *pMD5);
#endif

#if 0
void print_regs(int crop)
{
  const int tile_mode = (REG(VPP_SRC_CTRL) >> 9) & 0x0003;
  const int crop_num = crop;
  printf("VPP_1684\n");
  if(tile_mode==0) {
    printf("INPUT_0\n");
    printf("%s\n", "random.dat");
    printf("INPUT_1\n");
    printf("%s\n", "random.dat");
  }else {
    printf("INPUT_0\n");
    printf("%s\n", "random.dat");
    printf("INPUT_1\n");
    printf("%s\n", "random.dat");
  }
  printf("CROP_NUM\n");
  printf("%08x\n", 1);
  printf("SCA_CTRL\n");
  printf("%s\n", "fpga no this parameter");
  printf("PADDING_CTRL\n");
  printf("%08x\n", REG(VPP_PADDING_CTRL));
  printf("SRC_CTRL\n");
  printf("%08x\n", REG(VPP_SRC_CTRL));
  printf("SRC_SIZE\n");
  printf("%08x\n", REG(VPP_SRC_SIZE));

  for(int i=0; i<crop_num; i++) {

    printf("SRC_CROP_ST\n");
    printf("%08x\n", REG(VPP_SRC_CROP_ST));
    printf("SRC_CROP_SIZE\n");
    printf("%08x\n", REG(VPP_SRC_CROP_SIZE));

    if(i==0) {
      printf("SRC_RY_BASE\n");
      printf("%08x\n", REG(VPP_SRC_RY_BASE));
      printf("SRC_GU_BASE\n");
      printf("%08x\n", REG(VPP_SRC_GU_BASE));
      printf("SRC_BV_BASE\n");
      printf("%08x\n", REG(VPP_SRC_BV_BASE));
      printf("SRC_STRIDE\n");
      printf("%08x\n", REG(VPP_SRC_STRIDE));
      printf("SRC_RY_BASE_EXT\n");
      printf("%08x\n", REG(VPP_SRC_RY_BASE_EXT));
      printf("SRC_GU_BASE_EXT\n");
      printf("%08x\n", REG(VPP_SRC_GU_BASE_EXT));
      printf("SRC_BV_BASE_EXT\n");
      printf("%08x\n", REG(VPP_SRC_BV_BASE_EXT));

      printf("DST_CTRL\n");
      printf("%08x\n", REG(VPP_DST_CTRL));
      printf("DST_SIZE\n");
      printf("%08x\n", REG(VPP_DST_SIZE));
    }

    printf("DST_CROP_ST\n");
    printf("%08x\n", REG(VPP_DST_CROP_ST));
    printf("DST_CROP_SIZE\n");
    printf("%08x\n", REG(VPP_DST_CROP_SIZE));
    printf("DST_RY_BASE\n");
    printf("%08x\n", REG(VPP_DST_RY_BASE));
    printf("DST_GU_BASE\n");
    printf("%08x\n", REG(VPP_DST_GU_BASE));
    printf("DST_BV_BASE\n");
    printf("%08x\n", REG(VPP_DST_BV_BASE));
    printf("DST_STRIDE\n");
    printf("%08x\n", REG(VPP_DST_STRIDE));
    printf("DST_RY_BASE_EXT\n");
    printf("%08x\n", REG(VPP_DST_RY_BASE_EXT));
    printf("DST_GU_BASE_EXT\n");
    printf("%08x\n", REG(VPP_DST_GU_BASE_EXT));
    printf("DST_BV_BASE_EXT\n");
    printf("%08x\n", REG(VPP_DST_BV_BASE_EXT));
    printf("DST_PADDING_HEIGHT\n");
    printf("%08x\n", REG(VPP_DST_PADDING_HEIGHT));

    printf("SCL_CTRL\n");
    printf("%08x\n", REG(VPP_SCL_CTRL));
    printf("SCL_INIT\n");
    printf("%08x\n", REG(VPP_SCL_INIT));
    printf("SCL_X\n");
    printf("%08x\n", REG(VPP_SCL_X));
    printf("SCL_Y\n");
    printf("%08x\n", REG(VPP_SCL_Y));
  }

}
#endif
#if 0
static int vppRst(void)
{
  uint32 reg_read;
  uint32 active_check;
  /*set dma_stop=1*/
  REG(VPP_CONTROL0) = 0x00080008;

  /*check dma_stop_active==1*/
  for (;;)
  {
    printf("wait for vpp dma stop active......\n");
    reg_read = REG(VPP_STATUS);
    if ((reg_read >> 9) & 0x1)
    {
      printf("dma stop active!\n");
      active_check++;
      if (active_check == 5)
      {
        break;
      }
    }
  }
  /*vpp soft reset*/
  REG(VPP_CONTROL0) = 0x00020000;

  for (int idx = 0; idx < 10; idx++)
  {
    asm("nop");
  }

  REG(VPP_CONTROL0) = 0x00020002;

/*deassert dma_stop and check dma_stop_active*/
  REG(VPP_CONTROL0) = 0x00080000;
  for (;;)
  {
    printf("check dma stop active......\n");
    reg_read = REG(VPP_STATUS);
    if ((reg_read >> 9) & 0x1)
    {
      break;
    }
  }

  return 0;
}
#endif
static int waitProc()
{
#ifdef VPP_INT_TEST
  int vpp_int, frm_done_int, crop_done_int, gic_vpp;
  for (;;)
  {
    vpp_int = (REG(VPP_INT_STATUS) >> 0) & 0x1;//vpp_int_status
    frm_done_int = (REG(VPP_INT_STATUS) >> 1) & 0x1;//frame_done_int_status
    crop_done_int = (REG(VPP_INT_STATUS) >> 2) & 0x1;//frame_done_int_status
    gic_vpp = (REG(0x50001d0c) >> 13) & 0x1;//GICD_SPISR[2]    bit[13]:vpp  int
    printf("vpp_int : %d, frm_done_int : %d, crop_done_int : %d, gic_vpp : %d\n",vpp_int, frm_done_int, crop_done_int, gic_vpp);
    //unsigned int raw_status = REG(VPP_INT_RAW_STATUS);
    printf("waite: VPP_INT_RAW_STATUS 0x%08x\n",REG(VPP_INT_RAW_STATUS));
    if (frm_done_int == 0x01) {
      break;
    }
  }
  REG(VPP_INT_CLEAR) = 0xffffffff;//frame_done_int_clear     vpp_int_clear     W1C

  vpp_int = (REG(VPP_INT_STATUS) >> 0) & 0x1;//vpp_int_status
  frm_done_int = (REG(VPP_INT_STATUS) >> 1) & 0x1;//frame_done_int_status
  crop_done_int = (REG(VPP_INT_STATUS) >> 2) & 0x1;//frame_done_int_status
  gic_vpp = (REG(0x50001d0c) >> 13) & 0x1;//GICD_SPISR[2]    bit[13]:vpp  int
  printf("vpp_int222 : %d, frm_done_int : %d, crop_done_int : %d, gic_vpp : %d\n",vpp_int, frm_done_int, crop_done_int, gic_vpp);
  printf("waite222: VPP_INT_RAW_STATUS 0x%08x\n",REG(VPP_INT_RAW_STATUS));

#else
  for (;;)
  {
    int done = (REG(VPP_INT_RAW_STATUS) >> 1) & 0x1;//frame_done_int_raw_status
    //printf("waite: VPP_INT_RAW_STATUS 0x%08x, done %d\n",REG(VPP_INT_RAW_STATUS), done);
    if (done == 0x1) {
      break;
    }
  }
  REG(VPP_INT_CLEAR) = 0xffffffff;//frame_done_int_clear     vpp_int_clear     W1C
#endif

  return 0;
}

static uint32 cnt1 = 0;
unsigned long long vpp_ret_addr = 0x106100000;

int DataCompare_planar(uint8 *pcm_data, uint8 *pfpga_data,uint32 dst_ulx,uint32 dst_uly,uint32 dst_w,uint32 dst_h,uint32 dst_strid,uint32 ch)
{

  uint32 w,h;
  uint32 fpga_val, cm_val;
  printf("ch = %d,dst_ulx = %d,dst_uly = %d,dst_w = %d,dst_h = %d,dst_strid = %d\n",ch,dst_ulx,dst_uly,dst_w,dst_h,dst_strid);

  for (h = 0; h < dst_h; h++){
    for (w = 0; w < dst_w; w++){
      cm_val = (uint8)pcm_data[h * dst_w + w];
      fpga_val = (uint8)pfpga_data[(dst_uly + h) * dst_strid + dst_ulx + w];
      //printf("vpp print! ch = %d,cm_data[%d] = %d;fpga_data[%d] = %d\n",ch,(h * dst_w + w),cm_val,((dst_uly + h) * dst_strid + dst_ulx + w),fpga_val);
      if (cm_val != fpga_val) {
        cnt1++;
        printf("error! ch = %d,cm_data[%d] = %d;fpga_data[%d] = %d\n",ch,(h * dst_w + w),cm_val,((dst_uly + h) * dst_strid + dst_ulx + w),fpga_val);
      }
    }
  }
  return 0;
}
int DataCompare_packet24(uint8 *pcm_data, uint8 *pfpga_data,uint32 dst_ulx,uint32 dst_uly,uint32 dst_w,uint32 dst_h,uint32 dst_strid,uint32 ch)
{

  uint32 w,h;
  uint32 cm_idx,fpga_idx;
  printf("in DataCompare_packet24 .........................\n");
  printf("ch = %d,dst_ulx = %d,dst_uly = %d,dst_w = %d,dst_h = %d,dst_strid = %d\n",ch,dst_ulx,dst_uly,dst_w,dst_h,dst_strid);

  for (h = 0; h < dst_h; h++){
    for (w = 0; w < dst_w; w++){
      cm_idx = (h * dst_w + w) * 3;
      fpga_idx = (dst_uly + h) * dst_strid + (dst_ulx + w) * 3;
      if ((pcm_data[cm_idx] != pfpga_data[fpga_idx]) || (pcm_data[cm_idx + 1] != pfpga_data[fpga_idx + 1]) || (pcm_data[cm_idx + 2] != pfpga_data[fpga_idx + 2])) {
        cnt1++;
        printf("diff: [%dx%d] %d:%d: %d:%d-%d:%d-%d:%d\n", h, w, cm_idx, fpga_idx, pcm_data[cm_idx], pfpga_data[fpga_idx], pcm_data[cm_idx + 1], pfpga_data[fpga_idx + 1], pcm_data[cm_idx + 2], pfpga_data[fpga_idx + 2]);
      }
    }
  }

  return 0;
}
int DataCompare_packet32(uint8 *pcm_data, uint8 *pfpga_data,uint32 dst_ulx,uint32 dst_uly,uint32 dst_w,uint32 dst_h,uint32 dst_strid,uint32 ch)
{
  uint32 w,h;
  uint32 cm_idx,fpga_idx;
  printf("ch = %d,dst_ulx = %d,dst_uly = %d,dst_w = %d,dst_h = %d,dst_strid = %d\n",ch,dst_ulx,dst_uly,dst_w,dst_h,dst_strid);
  for (h = 0; h < dst_h; h++){
    for (w = 0; w < dst_w; w++){
      cm_idx = (h * dst_w + w) * 4;
      fpga_idx = (dst_uly + h) * dst_strid + (dst_ulx + w) * 4;
      //printf("vpp print! ch = %d,cm_data[%d] = %d;fpga_data[%d] = %d\n",ch,(h * dst_w + w),cm_val,((dst_uly + h) * dst_strid + dst_ulx + w),fpga_val);
      if ((pcm_data[cm_idx] != pfpga_data[fpga_idx]) || (pcm_data[cm_idx + 1] != pfpga_data[fpga_idx + 1]) || (pcm_data[cm_idx + 2] != pfpga_data[fpga_idx + 2]) || (pcm_data[cm_idx + 3] != pfpga_data[fpga_idx + 3])) {
        cnt1++;
        printf("diff: [%dx%d] %d:%d: %d:%d-%d:%d-%d:%d-%d:%d\n", h, w, cm_idx, fpga_idx, pcm_data[cm_idx], pfpga_data[fpga_idx], pcm_data[cm_idx + 1], pfpga_data[fpga_idx + 1], pcm_data[cm_idx + 2], pfpga_data[fpga_idx + 2], pcm_data[cm_idx + 3], pfpga_data[fpga_idx + 3]);
      }
    }
  }

  return 0;
}

static int toColorCrop2(struct rect *src, struct rect *dst,struct rect *src2, struct rect *dst2)
{
  int input_format,output_format,srcUV,dstUV, input_plannar, output_plannar;
  int yuv2rgb_enable = 0;

  Descriptor_t *pdes, *pdes2;

#ifdef VPP_PERFORMANCE_TEST
  unsigned long long vpp_time_start;
  unsigned long long vpp_time_consume;
#endif

#ifdef VPP_DES_16B_ALIGNED_TEST
//  srand((unsigned)time(NULL));
  srand((unsigned int)timer_get_tick());
  int idx = (rand()%(255));
  
  unsigned long long des_caddr = DES_CADDR0 + (idx << 4);
  printf("idx %d, des_caddr 0x%llx\n",idx, des_caddr);
    unsigned long long des_caddr2 = 0x116000000 + (idx << 4);
  printf("idx %d, des_caddr 0x%llx\n",idx, des_caddr2);
#else
  unsigned long long des_caddr = DES_CADDR0;
  unsigned long long des_caddr2 = DES_CADDR1;
#endif

  printf("func:toColor\n");
/*first  crop*/
  input_format = src->format;
  input_plannar = src->plannar;
  output_format = dst->format;
  output_plannar = dst->plannar;
  srcUV = ((input_format == YUV420) && (input_plannar == 1)) ? 2 : 1;
  dstUV = ((output_format == YUV420) && (output_plannar == 1)) ? 2 : 1;

  yuv2rgb_enable = ((input_format < YOnly) && (output_format > YOnly)) ? 1 : 0;

  printf("cmd list mode----------\n");
  //pdes = (Descriptor_t *)malloc(fd, VPP_DES_LEN, &des_paddr);
  pdes = (Descriptor_t *)(des_caddr);
  //memset(pdes, 0, VPP_DES_LEN);

/*one cmd*/
  pdes->des_head = 0x200;   //bit 9: 0 :updata scale coeff   1:keep scale coeff, not to read scale coeff in descriptor
  pdes->next_cmd_base = (uint32)(des_caddr2 & 0xffffffff);
  pdes->next_cmd_base_ext = (uint32)((des_caddr2 >> 32) & 0xff);
  
/*not change*/
  pdes->padding_ctrl = 0x00808080;  //bm1880 not support padding
  pdes->csc_coe00 = CSC_COE[0];
  pdes->csc_coe01 = CSC_COE[1];
  pdes->csc_coe02 = CSC_COE[2];
  pdes->csc_add0 = CSC_ADD[0];
  pdes->csc_coe10 = CSC_COE[3];
  pdes->csc_coe11 = CSC_COE[4];
  pdes->csc_coe12 = CSC_COE[5];
  pdes->csc_add1 = CSC_ADD[1];
  pdes->csc_coe20 = CSC_COE[6];
  pdes->csc_coe21 = CSC_COE[7];
  pdes->csc_coe22 = CSC_COE[8];
  pdes->csc_add2 = CSC_ADD[2];

  pdes->src_ctrl = ((yuv2rgb_enable & 0x1) << 11) | ((src->tile_mode & 0x3) << 9) | ((src->plannar & 0x1) << 8) |((src->endian & 0x1) << 5) \
                                          | ((src->endian_a & 0x1) << 4) |(src->format & 0xf);
  pdes->src_crop_st = (src->axisX << 16) | src->axisY;
  pdes->src_crop_size = (src->cropW << 16) | src->cropH;
  pdes->src_ry_base = (uint32)(src->addr0 & 0xffffffff);
  pdes->src_gu_base = (uint32)(src->addr1 & 0xffffffff);
  pdes->src_bv_base = (uint32)(src->addr2 & 0xffffffff);
  pdes->src_stride = (srcUV << 24) | src->stride;
  pdes->src_ry_base_ext = (uint32)((src->addr0 >> 32) & 0xff);
  pdes->src_gu_base_ext = (uint32)((src->addr1 >> 32) & 0xff);
  pdes->src_bv_base_ext = (uint32)((src->addr2 >> 32) & 0xff);

  pdes->dst_ctrl = ((dst->plannar & 0x1) << 8) |((dst->endian & 0x1) << 5) | ((dst->endian_a & 0x1) << 4) |(dst->format & 0xf);
  pdes->dst_crop_st = (dst->axisX << 16) | dst->axisY;
  pdes->dst_crop_size = (dst->cropW << 16) | dst->cropH;
  pdes->dst_ry_base = (uint32)(dst->addr0 & 0xffffffff);
  pdes->dst_gu_base = (uint32)(dst->addr1 & 0xffffffff);
  pdes->dst_bv_base = (uint32)(dst->addr2 & 0xffffffff);
  pdes->dst_stride = (dstUV << 24) | dst->stride;
  pdes->dst_ry_base_ext = (uint32)((dst->addr0 >> 32) & 0xff);
  pdes->dst_gu_base_ext = (uint32)((dst->addr1 >> 32) & 0xff);
  pdes->dst_bv_base_ext = (uint32)((dst->addr2 >> 32) & 0xff);

  pdes->scl_ctrl = src->scl_ctrl;
  pdes->scl_int = src->scl_init;


  pdes->scl_x = floor(src->cropW * 16384 / dst->cropW);
  pdes->scl_y = floor(src->cropH * 16384 / dst->cropH);

/*second------------crop*/
  input_format = src2->format;
  input_plannar = src2->plannar;
  output_format = dst2->format;
  output_plannar = dst2->plannar;
  srcUV = ((input_format == YUV420) && (input_plannar == 1)) ? 2 : 1;
  dstUV = ((output_format == YUV420) && (output_plannar == 1)) ? 2 : 1;

  yuv2rgb_enable = ((input_format < YOnly) && (output_format > YOnly)) ? 1 : 0;
  

  printf("cmd list mode----------\n");
  //pdes = (Descriptor_t *)malloc(fd, VPP_DES_LEN, &des_paddr);
  pdes2 = (Descriptor_t *)(des_caddr2);
  //memset(pdes, 0, VPP_DES_LEN);

/*one cmd*/
  pdes2->des_head = 0x301;   //bit 9: 0 :updata scale coeff   1:keep scale coeff, not to read scale coeff in descriptor
  pdes2->next_cmd_base = 0x0;
  pdes2->next_cmd_base_ext = 0x0;
/*not change*/
  pdes2->padding_ctrl = 0x00808080;  //bm1880 not support padding
  pdes2->csc_coe00 = CSC_COE[0];
  pdes2->csc_coe01 = CSC_COE[1];
  pdes2->csc_coe02 = CSC_COE[2];
  pdes2->csc_add0 = CSC_ADD[0];
  pdes2->csc_coe10 = CSC_COE[3];
  pdes2->csc_coe11 = CSC_COE[4];
  pdes2->csc_coe12 = CSC_COE[5];
  pdes2->csc_add1 = CSC_ADD[1];
  pdes2->csc_coe20 = CSC_COE[6];
  pdes2->csc_coe21 = CSC_COE[7];
  pdes2->csc_coe22 = CSC_COE[8];
  pdes2->csc_add2 = CSC_ADD[2];

  pdes2->src_ctrl = ((yuv2rgb_enable & 0x1) << 11) | ((src2->tile_mode & 0x3) << 9) | ((src2->plannar & 0x1) << 8) |((src2->endian & 0x1) << 5) \
                                          | ((src2->endian_a & 0x1) << 4) |(src2->format & 0xf);
  pdes2->src_crop_st = (src2->axisX << 16) | src2->axisY;
  pdes2->src_crop_size = (src2->cropW << 16) | src2->cropH;
  pdes2->src_ry_base = (uint32)(src2->addr0 & 0xffffffff);
  pdes2->src_gu_base = (uint32)(src2->addr1 & 0xffffffff);
  pdes2->src_bv_base = (uint32)(src2->addr2 & 0xffffffff);
  pdes2->src_stride = (srcUV << 24) | src2->stride;
  pdes2->src_ry_base_ext = (uint32)((src2->addr0 >> 32) & 0xff);
  pdes2->src_gu_base_ext = (uint32)((src2->addr1 >> 32) & 0xff);
  pdes2->src_bv_base_ext = (uint32)((src2->addr2 >> 32) & 0xff);

  pdes2->dst_ctrl = ((dst2->plannar & 0x1) << 8) |((dst2->endian & 0x1) << 5) | ((dst2->endian_a & 0x1) << 4) |(dst2->format & 0xf);
  pdes2->dst_crop_st = (dst2->axisX << 16) | dst2->axisY;
  pdes2->dst_crop_size = (dst2->cropW << 16) | dst2->cropH;
  pdes2->dst_ry_base = (uint32)(dst2->addr0 & 0xffffffff);
  pdes2->dst_gu_base = (uint32)(dst2->addr1 & 0xffffffff);
  pdes2->dst_bv_base = (uint32)(dst2->addr2 & 0xffffffff);
  pdes2->dst_stride = (dstUV << 24) | dst2->stride;
  pdes2->dst_ry_base_ext = (uint32)((dst2->addr0 >> 32) & 0xff);
  pdes2->dst_gu_base_ext = (uint32)((dst2->addr1 >> 32) & 0xff);
  pdes2->dst_bv_base_ext = (uint32)((dst2->addr2 >> 32) & 0xff);

  pdes2->scl_ctrl = src2->scl_ctrl;
  pdes2->scl_int = src2->scl_init;


  pdes2->scl_x = floor(src2->cropW * 16384 / dst2->cropW);
  pdes2->scl_y = floor(src2->cropH * 16384 / dst2->cropH);
/*crop end*/

  while(1) {
    int status;

    status = (REG(VPP_STATUS) >> 8) & 0x1;
    printf("vpp status   %d\n", status);
    if (status == 0x01)
      break;
  }

  while(1) {
    int raw_status;

    raw_status = REG(VPP_INT_RAW_STATUS);
    printf("raw_status 0x%08x\n", raw_status);
    if (raw_status != 0x0) {
      REG(VPP_INT_CLEAR) = 0xffffffff; 
      raw_status = REG(VPP_INT_RAW_STATUS);
      printf("raw_status 222: 0x%08x\n", raw_status);
    } else {
      break;
    }
  }

  REG(VPP_CMD_BASE) = (uint32)(des_caddr & 0xffffffff);
  REG(VPP_CMD_BASE_EXT) = (uint32)((des_caddr >> 32) & 0xff);

#ifdef VPP_PERFORMANCE_TEST
  vpp_time_start = timer_get_tick();
#endif

  REG(VPP_CONTROL0) = 0x00010001;//vpp frame start

  //print_regs(1);
  waitProc();

  #ifdef VPP_PERFORMANCE_TEST
  vpp_time_consume = (timer_get_tick() - vpp_time_start) / TIMER_CNT_US;
  printf("vpp 1920x1080 tile mode to rgb24 time consuming %lld us\n", vpp_time_consume);
  #endif

  return 0;
}

static uint g_uiSwitch = 0;
static int toColor_switch(struct rect *src, struct rect *dst)
{
  int input_format,output_format,srcUV,dstUV, input_plannar, output_plannar;
  int yuv2rgb_enable = 0;

  Descriptor_t *pdes;

#ifdef VPP_PERFORMANCE_TEST
  unsigned long long vpp_time_start;
  unsigned long long vpp_time_consume;
#endif

#ifdef VPP_DES_16B_ALIGNED_TEST
//  srand((unsigned)time(NULL));
  srand((unsigned int)timer_get_tick());
  int idx = (rand()%(255));
  
  unsigned long long des_caddr = DES_CADDR0 + (idx << 4);
  printf("idx %d, des_caddr 0x%llx\n",idx, des_caddr);
#else
  unsigned long long des_caddr = DES_CADDR0;
#endif

  printf("func:toColor\n");

  input_format = src->format;
  input_plannar = src->plannar;
  output_format = dst->format;
  output_plannar = dst->plannar;
  srcUV = ((input_format == YUV420) && (input_plannar == 1)) ? 2 : 1;
  dstUV = ((output_format == YUV420) && (output_plannar == 1)) ? 2 : 1;

  yuv2rgb_enable = ((input_format < YOnly) && (output_format > YOnly)) ? 1 : 0;
  
if(CMD_LIST_MODE == g_uiSwitch)
{

  REG(VPP_CONTROL0) = 0x00040004;  // cmd list mode, DMA auto fetch linked command list;
  printf("cmd list mode----------\n");
  //pdes = (Descriptor_t *)malloc(fd, VPP_DES_LEN, &des_paddr);
  pdes = (Descriptor_t *)(des_caddr);
  //memset(pdes, 0, VPP_DES_LEN);

/*one cmd*/
  pdes->des_head = 0x300;   //bit 9: 0 :updata scale coeff   1:keep scale coeff, not to read scale coeff in descriptor
  pdes->next_cmd_base = 0x0;
  pdes->next_cmd_base_ext = 0x0;
/*not change*/
  pdes->padding_ctrl = 0x00808080;  //bm1880 not support padding
  pdes->csc_coe00 = CSC_COE[0];
  pdes->csc_coe01 = CSC_COE[1];
  pdes->csc_coe02 = CSC_COE[2];
  pdes->csc_add0 = CSC_ADD[0];
  pdes->csc_coe10 = CSC_COE[3];
  pdes->csc_coe11 = CSC_COE[4];
  pdes->csc_coe12 = CSC_COE[5];
  pdes->csc_add1 = CSC_ADD[1];
  pdes->csc_coe20 = CSC_COE[6];
  pdes->csc_coe21 = CSC_COE[7];
  pdes->csc_coe22 = CSC_COE[8];
  pdes->csc_add2 = CSC_ADD[2];

  pdes->src_ctrl = ((yuv2rgb_enable & 0x1) << 11) | ((src->tile_mode & 0x3) << 9) | ((src->plannar & 0x1) << 8) |((src->endian & 0x1) << 5) \
                                          | ((src->endian_a & 0x1) << 4) |(src->format & 0xf);
  pdes->src_crop_st = (src->axisX << 16) | src->axisY;
  pdes->src_crop_size = (src->cropW << 16) | src->cropH;
  pdes->src_ry_base = (uint32)(src->addr0 & 0xffffffff);
  pdes->src_gu_base = (uint32)(src->addr1 & 0xffffffff);
  pdes->src_bv_base = (uint32)(src->addr2 & 0xffffffff);
  pdes->src_stride = (srcUV << 24) | src->stride;
  pdes->src_ry_base_ext = (uint32)((src->addr0 >> 32) & 0xff);
  pdes->src_gu_base_ext = (uint32)((src->addr1 >> 32) & 0xff);
  pdes->src_bv_base_ext = (uint32)((src->addr2 >> 32) & 0xff);

  pdes->dst_ctrl = ((dst->plannar & 0x1) << 8) |((dst->endian & 0x1) << 5) | ((dst->endian_a & 0x1) << 4) |(dst->format & 0xf);
  pdes->dst_crop_st = (dst->axisX << 16) | dst->axisY;
  pdes->dst_crop_size = (dst->cropW << 16) | dst->cropH;
  pdes->dst_ry_base = (uint32)(dst->addr0 & 0xffffffff);
  pdes->dst_gu_base = (uint32)(dst->addr1 & 0xffffffff);
  pdes->dst_bv_base = (uint32)(dst->addr2 & 0xffffffff);
  pdes->dst_stride = (dstUV << 24) | dst->stride;
  pdes->dst_ry_base_ext = (uint32)((dst->addr0 >> 32) & 0xff);
  pdes->dst_gu_base_ext = (uint32)((dst->addr1 >> 32) & 0xff);
  pdes->dst_bv_base_ext = (uint32)((dst->addr2 >> 32) & 0xff);

  pdes->scl_ctrl = src->scl_ctrl;
  pdes->scl_int = src->scl_init;


  pdes->scl_x = floor(src->cropW * 16384 / dst->cropW);
  pdes->scl_y = floor(src->cropH * 16384 / dst->cropH);


#ifdef VPP_SCL_FILTER_COE_TEST
  printf("vpp scl filter coef test....\n");
  pdes->des_head &= ~(1UL << 9);;   //bit 9: 0 :updata scale coeff   1:keep scale coeff, not to read scale coeff in descriptor
  /*y coef*/
  pdes->scl_y_coe_p0c0 = SCL_Y_COE[0];
  pdes->scl_y_coe_p0c2 = SCL_Y_COE[1];
  pdes->scl_y_coe_p1c0 = SCL_Y_COE[2];
  pdes->scl_y_coe_p1c2 = SCL_Y_COE[3];
  pdes->scl_y_coe_p2c0 = SCL_Y_COE[4];
  pdes->scl_y_coe_p2c2 = SCL_Y_COE[5];
  pdes->scl_y_coe_p3c0 = SCL_Y_COE[6];
  pdes->scl_y_coe_p3c2 = SCL_Y_COE[7];
  pdes->scl_y_coe_p4c0 = SCL_Y_COE[8];
  pdes->scl_y_coe_p4c2 = SCL_Y_COE[9];
  pdes->scl_y_coe_p5c0 = SCL_Y_COE[10];
  pdes->scl_y_coe_p5c2 = SCL_Y_COE[11];
  pdes->scl_y_coe_p6c0 = SCL_Y_COE[12];
  pdes->scl_y_coe_p6c2 = SCL_Y_COE[13];
  pdes->scl_y_coe_p7c0 = SCL_Y_COE[14];
  pdes->scl_y_coe_p7c2 = SCL_Y_COE[15];
  pdes->scl_y_coe_p8c0 = SCL_Y_COE[16];
  pdes->scl_y_coe_p8c2 = SCL_Y_COE[17];
  pdes->scl_y_coe_p9c0 = SCL_Y_COE[18];
  pdes->scl_y_coe_p9c2 = SCL_Y_COE[19];
  pdes->scl_y_coe_pac0 = SCL_Y_COE[20];
  pdes->scl_y_coe_pac2 = SCL_Y_COE[21];
  pdes->scl_y_coe_pbc0 = SCL_Y_COE[22];
  pdes->scl_y_coe_pbc2 = SCL_Y_COE[23];
  pdes->scl_y_coe_pcc0 = SCL_Y_COE[24];
  pdes->scl_y_coe_pcc2 = SCL_Y_COE[25];
  pdes->scl_y_coe_pdc0 = SCL_Y_COE[26];
  pdes->scl_y_coe_pdc2 = SCL_Y_COE[27];
  pdes->scl_y_coe_pec0 = SCL_Y_COE[28];
  pdes->scl_y_coe_pec2 = SCL_Y_COE[29];
  pdes->scl_y_coe_pfc0 = SCL_Y_COE[30];
  pdes->scl_y_coe_pfc2 = SCL_Y_COE[31];

  /*x coef*/
  pdes->scl_x_coe_p0c0 = SCL_X_COE[0];
  pdes->scl_x_coe_p0c2 = SCL_X_COE[1];
  pdes->scl_x_coe_p0c4 = SCL_X_COE[2];
  pdes->scl_x_coe_p0c6 = SCL_X_COE[3];
  pdes->scl_x_coe_p1c0 = SCL_X_COE[4];
  pdes->scl_x_coe_p1c2 = SCL_X_COE[5];
  pdes->scl_x_coe_p1c4 = SCL_X_COE[6];
  pdes->scl_x_coe_p1c6 = SCL_X_COE[7];
  pdes->scl_x_coe_p2c0 = SCL_X_COE[8];
  pdes->scl_x_coe_p2c2 = SCL_X_COE[9];
  pdes->scl_x_coe_p2c4 = SCL_X_COE[10];
  pdes->scl_x_coe_p2c6 = SCL_X_COE[11];
  pdes->scl_x_coe_p3c0 = SCL_X_COE[12];
  pdes->scl_x_coe_p3c2 = SCL_X_COE[13];
  pdes->scl_x_coe_p3c4 = SCL_X_COE[14];
  pdes->scl_x_coe_p3c6 = SCL_X_COE[15];
  pdes->scl_x_coe_p4c0 = SCL_X_COE[16];
  pdes->scl_x_coe_p4c2 = SCL_X_COE[17];
  pdes->scl_x_coe_p4c4 = SCL_X_COE[18];
  pdes->scl_x_coe_p4c6 = SCL_X_COE[19];
  pdes->scl_x_coe_p5c0 = SCL_X_COE[20];
  pdes->scl_x_coe_p5c2 = SCL_X_COE[21];
  pdes->scl_x_coe_p5c4 = SCL_X_COE[22];
  pdes->scl_x_coe_p5c6 = SCL_X_COE[23];
  pdes->scl_x_coe_p6c0 = SCL_X_COE[24];
  pdes->scl_x_coe_p6c2 = SCL_X_COE[25];
  pdes->scl_x_coe_p6c4 = SCL_X_COE[26];
  pdes->scl_x_coe_p6c6 = SCL_X_COE[27];
  pdes->scl_x_coe_p7c0 = SCL_X_COE[28];
  pdes->scl_x_coe_p7c2 = SCL_X_COE[29];
  pdes->scl_x_coe_p7c4 = SCL_X_COE[30];
  pdes->scl_x_coe_p7c6 = SCL_X_COE[31];
  pdes->scl_x_coe_p8c0 = SCL_X_COE[32];
  pdes->scl_x_coe_p8c2 = SCL_X_COE[33];
  pdes->scl_x_coe_p8c4 = SCL_X_COE[34];
  pdes->scl_x_coe_p8c6 = SCL_X_COE[35];
  pdes->scl_x_coe_p9c0 = SCL_X_COE[36];
  pdes->scl_x_coe_p9c2 = SCL_X_COE[37];
  pdes->scl_x_coe_p9c4 = SCL_X_COE[38];
  pdes->scl_x_coe_p9c6 = SCL_X_COE[39];
  pdes->scl_x_coe_pac0 = SCL_X_COE[40];
  pdes->scl_x_coe_pac2 = SCL_X_COE[41];
  pdes->scl_x_coe_pac4 = SCL_X_COE[42];
  pdes->scl_x_coe_pac6 = SCL_X_COE[43];
  pdes->scl_x_coe_pbc0 = SCL_X_COE[44];
  pdes->scl_x_coe_pbc2 = SCL_X_COE[45];
  pdes->scl_x_coe_pbc4 = SCL_X_COE[46];
  pdes->scl_x_coe_pbc6 = SCL_X_COE[47];
  pdes->scl_x_coe_pcc0 = SCL_X_COE[48];
  pdes->scl_x_coe_pcc2 = SCL_X_COE[49];
  pdes->scl_x_coe_pcc4 = SCL_X_COE[50];
  pdes->scl_x_coe_pcc6 = SCL_X_COE[51];
  pdes->scl_x_coe_pdc0 = SCL_X_COE[52];
  pdes->scl_x_coe_pdc2 = SCL_X_COE[53];
  pdes->scl_x_coe_pdc4 = SCL_X_COE[54];
  pdes->scl_x_coe_pdc6 = SCL_X_COE[55];
  pdes->scl_x_coe_pec0 = SCL_X_COE[56];
  pdes->scl_x_coe_pec2 = SCL_X_COE[57];
  pdes->scl_x_coe_pec4 = SCL_X_COE[58];
  pdes->scl_x_coe_pec6 = SCL_X_COE[59];
  pdes->scl_x_coe_pfc0 = SCL_X_COE[60];
  pdes->scl_x_coe_pfc2 = SCL_X_COE[61];
  pdes->scl_x_coe_pfc4 = SCL_X_COE[62];
  pdes->scl_x_coe_pfc6 = SCL_X_COE[63];
#endif


  while(1) {
    int status;

    status = (REG(VPP_STATUS) >> 8) & 0x1;
    printf("vpp status   %d\n", status);
    if (status == 0x01)
      break;
  }

  while(1) {
    int raw_status;

    raw_status = REG(VPP_INT_RAW_STATUS);
    printf("raw_status 0x%08x\n", raw_status);
    if (raw_status != 0x0) {
      REG(VPP_INT_CLEAR) = 0xffffffff; 
      raw_status = REG(VPP_INT_RAW_STATUS);
      printf("raw_status 222: 0x%08x\n", raw_status);
    } else {
      break;
    }
  }

  REG(VPP_CMD_BASE) = (uint32)(des_caddr & 0xffffffff);
  REG(VPP_CMD_BASE_EXT) = (uint32)((des_caddr >> 32) & 0xff);

#ifdef VPP_PERFORMANCE_TEST
  vpp_time_start = timer_get_tick();
#endif

  REG(VPP_CONTROL0) = 0x00010001;//vpp frame start

  //print_regs(1);
  waitProc();

  #ifdef VPP_PERFORMANCE_TEST
  vpp_time_consume = (timer_get_tick() - vpp_time_start) / TIMER_CNT_US;
  printf("vpp 1920x1080 tile mode to rgb24 time consuming %lld us\n", vpp_time_consume);
  #endif
}
else if(CPU_CONFIG_MODE == g_uiSwitch)
{
  printf("here cpu config mode----------\n");
  while(1)
  {
   
    int status;
    status = (REG(VPP_STATUS) >> 8) & 0x1;
    printf("vpp status   %d\n",status);
    if (status == 0x01)
      break;
  }

  while(1)
  {
    int raw_status;
    raw_status = REG(VPP_INT_RAW_STATUS);
    printf("raw_status 0x%08x\n",raw_status);
    if (raw_status != 0x0)
    {
      REG(VPP_INT_CLEAR) = 0xffffffff; 
      raw_status = REG(VPP_INT_RAW_STATUS);
      printf("raw_status 222: 0x%08x\n",raw_status);
    } else 
    {
      break;
    }
  }
  REG(VPP_CONTROL0) = 0x00040000;  //single cmd mode, CPU config;
  
  REG(VPP_DES_HEAD) = 0x100;
  REG(VPP_NEXT_CMD_BASE) = 0x0;
  REG(VPP_NEXT_CMD_BASE_EXT) = 0x0;
  REG(VPP_PADDING_CTRL) = 0x00808080;
  REG(VPP_CSC_COE_00) = CSC_COE[0];
  REG(VPP_CSC_COE_01) = CSC_COE[1];
  REG(VPP_CSC_COE_02) = CSC_COE[2];
  REG(VPP_CSC_ADD_0) = CSC_ADD[0];
  REG(VPP_CSC_COE_10) = CSC_COE[3];
  REG(VPP_CSC_COE_11) = CSC_COE[4];
  REG(VPP_CSC_COE_12) = CSC_COE[5];
  REG(VPP_CSC_ADD_1) = CSC_ADD[1];
  REG(VPP_CSC_COE_20) = CSC_COE[6];
  REG(VPP_CSC_COE_21) = CSC_COE[7];
  REG(VPP_CSC_COE_22) = CSC_COE[8];
  REG(VPP_CSC_ADD_2) = CSC_ADD[2];
  

  REG(VPP_SRC_CTRL) = ((yuv2rgb_enable & 0x1) << 11) | ((src->tile_mode & 0x3) << 9) | ((src->plannar & 0x1) << 8) |((src->endian & 0x1) << 5) \
                                          | ((src->endian_a & 0x1) << 4) |(src->format & 0xf);
  REG(VPP_SRC_CROP_ST) = (src->axisX << 16) | src->axisY;
  REG(VPP_SRC_CROP_SIZE) = (src->cropW << 16) | src->cropH;
  REG(VPP_SRC_RY_BASE) = (uint32)(src->addr0 & 0xffffffff);             //low 32 bit addr
  REG(VPP_SRC_GU_BASE) = (uint32)(src->addr1 & 0xffffffff);
  REG(VPP_SRC_BV_BASE) = (uint32)(src->addr2 & 0xffffffff);
  REG(VPP_SRC_STRIDE) = (srcUV << 24) | src->stride;
  REG(VPP_SRC_RY_BASE_EXT) = (uint32)((src->addr0 >> 32) & 0xff);     //high 8 bit addr
  REG(VPP_SRC_GU_BASE_EXT) = (uint32)((src->addr1 >> 32) & 0xff);
  REG(VPP_SRC_BV_BASE_EXT) = (uint32)((src->addr2 >> 32) & 0xff);

  REG(VPP_DST_CTRL) = ((dst->plannar & 0x1) << 8) |((dst->endian & 0x1) << 5) | ((dst->endian_a & 0x1) << 4) |(dst->format & 0xf);
  REG(VPP_DST_CROP_ST) = (dst->axisX << 16) | dst->axisY;
  REG(VPP_DST_CROP_SIZE) = (dst->cropW << 16) | dst->cropH;
  REG(VPP_DST_RY_BASE) = (uint32)(dst->addr0 & 0xffffffff);                   //low 32 bit addr
  REG(VPP_DST_GU_BASE) = (uint32)(dst->addr1 & 0xffffffff);
  REG(VPP_DST_BV_BASE) = (uint32)(dst->addr2 & 0xffffffff);
  REG(VPP_DST_STRIDE) = (dstUV << 24) | dst->stride;
  REG(VPP_DST_RY_BASE_EXT) = (uint32)((dst->addr0 >> 32) & 0xff);            //high 8 bit addr
  REG(VPP_DST_GU_BASE_EXT) = (uint32)((dst->addr1 >> 32) & 0xff);
  REG(VPP_DST_BV_BASE_EXT) = (uint32)((dst->addr2 >> 32) & 0xff);

  /*scl_ctl*/
  REG(VPP_SCL_CTRL) = src->scl_ctrl;
  REG(VPP_SCL_INIT) = src->scl_init;

  REG(VPP_SCL_X) = floor(src->cropW * 16384 / dst->cropW);
  REG(VPP_SCL_Y) = floor(src->cropH * 16384 / dst->cropH);

  REG(VPP_PADDING_CTRL) = 0x00808080;
  REG(VPP_DST_SIZE) = 0x0;

  //REG(VPP_SCL_X) = ((src->cropW << 15) / dst->cropW + 1) >> 1;
  //REG(VPP_SCL_Y) = ((src->cropH << 15) / dst->cropH + 1) >> 1;

  REG(VPP_CONTROL0) = 0x00010001;//vpp frame start
  //print_regs(1);
  waitProc();
}

  return 0;
}

static int toColor(struct rect *src, struct rect *dst)
{
  int input_format,output_format,srcUV,dstUV, input_plannar, output_plannar;
  int yuv2rgb_enable = 0;

  Descriptor_t *pdes;

#ifdef VPP_PERFORMANCE_TEST
  unsigned long long vpp_time_start;
  unsigned long long vpp_time_consume;
#endif

#ifdef VPP_DES_16B_ALIGNED_TEST
//  srand((unsigned)time(NULL));
  srand((unsigned int)timer_get_tick());
  int idx = (rand()%(255));

  unsigned long long des_caddr = DES_CADDR0 + (idx << 4);
  PRINT_DEBUG(debug_level, "idx %d, des_caddr 0x%llx\n",idx, des_caddr);
#else
  unsigned long long des_caddr = DES_CADDR0;
#endif

  PRINT_DEBUG(debug_level, "func:toColor\n");

  input_format = src->format;
  input_plannar = src->plannar;
  output_format = dst->format;
  output_plannar = dst->plannar;
  srcUV = ((input_format == YUV420) && (input_plannar == 1)) ? 2 : 1;
  dstUV = ((output_format == YUV420) && (output_plannar == 1)) ? 2 : 1;

  yuv2rgb_enable = ((input_format < YOnly) && (output_format > YOnly)) ? 1 : 0;

#ifdef VPP_SUPPORT_CMD_LIST_MODE
  PRINT_DEBUG(debug_level, "cmd list mode----------\n");
  //pdes = (Descriptor_t *)malloc(fd, VPP_DES_LEN, &des_paddr);
  pdes = (Descriptor_t *)(des_caddr);
  //memset(pdes, 0, VPP_DES_LEN);

/*one cmd*/
  pdes->des_head = 0x300;   //bit 9: 0 :updata scale coeff   1:keep scale coeff, not to read scale coeff in descriptor
  pdes->next_cmd_base = 0x0;
  pdes->next_cmd_base_ext = 0x0;
/*not change*/
  pdes->padding_ctrl = 0x00808080;  //bm1880 not support padding
  pdes->csc_coe00 = CSC_COE[0];
  pdes->csc_coe01 = CSC_COE[1];
  pdes->csc_coe02 = CSC_COE[2];
  pdes->csc_add0 = CSC_ADD[0];
  pdes->csc_coe10 = CSC_COE[3];
  pdes->csc_coe11 = CSC_COE[4];
  pdes->csc_coe12 = CSC_COE[5];
  pdes->csc_add1 = CSC_ADD[1];
  pdes->csc_coe20 = CSC_COE[6];
  pdes->csc_coe21 = CSC_COE[7];
  pdes->csc_coe22 = CSC_COE[8];
  pdes->csc_add2 = CSC_ADD[2];

  pdes->src_ctrl = ((yuv2rgb_enable & 0x1) << 11) | ((src->tile_mode & 0x3) << 9) | ((src->plannar & 0x1) << 8) |((src->endian & 0x1) << 5) \
                                          | ((src->endian_a & 0x1) << 4) |(src->format & 0xf);
  pdes->src_crop_st = (src->axisX << 16) | src->axisY;
  pdes->src_crop_size = (src->cropW << 16) | src->cropH;
  pdes->src_ry_base = (uint32)(src->addr0 & 0xffffffff);
  pdes->src_gu_base = (uint32)(src->addr1 & 0xffffffff);
  pdes->src_bv_base = (uint32)(src->addr2 & 0xffffffff);
  pdes->src_stride = (srcUV << 24) | src->stride;
  pdes->src_ry_base_ext = (uint32)((src->addr0 >> 32) & 0xff);
  pdes->src_gu_base_ext = (uint32)((src->addr1 >> 32) & 0xff);
  pdes->src_bv_base_ext = (uint32)((src->addr2 >> 32) & 0xff);

  pdes->dst_ctrl = ((dst->plannar & 0x1) << 8) |((dst->endian & 0x1) << 5) | ((dst->endian_a & 0x1) << 4) |(dst->format & 0xf);
  pdes->dst_crop_st = (dst->axisX << 16) | dst->axisY;
  pdes->dst_crop_size = (dst->cropW << 16) | dst->cropH;
  pdes->dst_ry_base = (uint32)(dst->addr0 & 0xffffffff);
  pdes->dst_gu_base = (uint32)(dst->addr1 & 0xffffffff);
  pdes->dst_bv_base = (uint32)(dst->addr2 & 0xffffffff);
  pdes->dst_stride = (dstUV << 24) | dst->stride;
  pdes->dst_ry_base_ext = (uint32)((dst->addr0 >> 32) & 0xff);
  pdes->dst_gu_base_ext = (uint32)((dst->addr1 >> 32) & 0xff);
  pdes->dst_bv_base_ext = (uint32)((dst->addr2 >> 32) & 0xff);

  pdes->scl_ctrl = src->scl_ctrl;
  pdes->scl_int = src->scl_init;


  pdes->scl_x = floor(src->cropW * 16384 / dst->cropW);
  pdes->scl_y = floor(src->cropH * 16384 / dst->cropH);


#ifdef VPP_SCL_FILTER_COE_TEST
  printf("vpp scl filter coef test....\n");
  pdes->des_head &= ~(1UL << 9);;   //bit 9: 0 :updata scale coeff   1:keep scale coeff, not to read scale coeff in descriptor
  /*y coef*/
  pdes->scl_y_coe_p0c0 = SCL_Y_COE[0];
  pdes->scl_y_coe_p0c2 = SCL_Y_COE[1];
  pdes->scl_y_coe_p1c0 = SCL_Y_COE[2];
  pdes->scl_y_coe_p1c2 = SCL_Y_COE[3];
  pdes->scl_y_coe_p2c0 = SCL_Y_COE[4];
  pdes->scl_y_coe_p2c2 = SCL_Y_COE[5];
  pdes->scl_y_coe_p3c0 = SCL_Y_COE[6];
  pdes->scl_y_coe_p3c2 = SCL_Y_COE[7];
  pdes->scl_y_coe_p4c0 = SCL_Y_COE[8];
  pdes->scl_y_coe_p4c2 = SCL_Y_COE[9];
  pdes->scl_y_coe_p5c0 = SCL_Y_COE[10];
  pdes->scl_y_coe_p5c2 = SCL_Y_COE[11];
  pdes->scl_y_coe_p6c0 = SCL_Y_COE[12];
  pdes->scl_y_coe_p6c2 = SCL_Y_COE[13];
  pdes->scl_y_coe_p7c0 = SCL_Y_COE[14];
  pdes->scl_y_coe_p7c2 = SCL_Y_COE[15];
  pdes->scl_y_coe_p8c0 = SCL_Y_COE[16];
  pdes->scl_y_coe_p8c2 = SCL_Y_COE[17];
  pdes->scl_y_coe_p9c0 = SCL_Y_COE[18];
  pdes->scl_y_coe_p9c2 = SCL_Y_COE[19];
  pdes->scl_y_coe_pac0 = SCL_Y_COE[20];
  pdes->scl_y_coe_pac2 = SCL_Y_COE[21];
  pdes->scl_y_coe_pbc0 = SCL_Y_COE[22];
  pdes->scl_y_coe_pbc2 = SCL_Y_COE[23];
  pdes->scl_y_coe_pcc0 = SCL_Y_COE[24];
  pdes->scl_y_coe_pcc2 = SCL_Y_COE[25];
  pdes->scl_y_coe_pdc0 = SCL_Y_COE[26];
  pdes->scl_y_coe_pdc2 = SCL_Y_COE[27];
  pdes->scl_y_coe_pec0 = SCL_Y_COE[28];
  pdes->scl_y_coe_pec2 = SCL_Y_COE[29];
  pdes->scl_y_coe_pfc0 = SCL_Y_COE[30];
  pdes->scl_y_coe_pfc2 = SCL_Y_COE[31];

  /*x coef*/
  pdes->scl_x_coe_p0c0 = SCL_X_COE[0];
  pdes->scl_x_coe_p0c2 = SCL_X_COE[1];
  pdes->scl_x_coe_p0c4 = SCL_X_COE[2];
  pdes->scl_x_coe_p0c6 = SCL_X_COE[3];
  pdes->scl_x_coe_p1c0 = SCL_X_COE[4];
  pdes->scl_x_coe_p1c2 = SCL_X_COE[5];
  pdes->scl_x_coe_p1c4 = SCL_X_COE[6];
  pdes->scl_x_coe_p1c6 = SCL_X_COE[7];
  pdes->scl_x_coe_p2c0 = SCL_X_COE[8];
  pdes->scl_x_coe_p2c2 = SCL_X_COE[9];
  pdes->scl_x_coe_p2c4 = SCL_X_COE[10];
  pdes->scl_x_coe_p2c6 = SCL_X_COE[11];
  pdes->scl_x_coe_p3c0 = SCL_X_COE[12];
  pdes->scl_x_coe_p3c2 = SCL_X_COE[13];
  pdes->scl_x_coe_p3c4 = SCL_X_COE[14];
  pdes->scl_x_coe_p3c6 = SCL_X_COE[15];
  pdes->scl_x_coe_p4c0 = SCL_X_COE[16];
  pdes->scl_x_coe_p4c2 = SCL_X_COE[17];
  pdes->scl_x_coe_p4c4 = SCL_X_COE[18];
  pdes->scl_x_coe_p4c6 = SCL_X_COE[19];
  pdes->scl_x_coe_p5c0 = SCL_X_COE[20];
  pdes->scl_x_coe_p5c2 = SCL_X_COE[21];
  pdes->scl_x_coe_p5c4 = SCL_X_COE[22];
  pdes->scl_x_coe_p5c6 = SCL_X_COE[23];
  pdes->scl_x_coe_p6c0 = SCL_X_COE[24];
  pdes->scl_x_coe_p6c2 = SCL_X_COE[25];
  pdes->scl_x_coe_p6c4 = SCL_X_COE[26];
  pdes->scl_x_coe_p6c6 = SCL_X_COE[27];
  pdes->scl_x_coe_p7c0 = SCL_X_COE[28];
  pdes->scl_x_coe_p7c2 = SCL_X_COE[29];
  pdes->scl_x_coe_p7c4 = SCL_X_COE[30];
  pdes->scl_x_coe_p7c6 = SCL_X_COE[31];
  pdes->scl_x_coe_p8c0 = SCL_X_COE[32];
  pdes->scl_x_coe_p8c2 = SCL_X_COE[33];
  pdes->scl_x_coe_p8c4 = SCL_X_COE[34];
  pdes->scl_x_coe_p8c6 = SCL_X_COE[35];
  pdes->scl_x_coe_p9c0 = SCL_X_COE[36];
  pdes->scl_x_coe_p9c2 = SCL_X_COE[37];
  pdes->scl_x_coe_p9c4 = SCL_X_COE[38];
  pdes->scl_x_coe_p9c6 = SCL_X_COE[39];
  pdes->scl_x_coe_pac0 = SCL_X_COE[40];
  pdes->scl_x_coe_pac2 = SCL_X_COE[41];
  pdes->scl_x_coe_pac4 = SCL_X_COE[42];
  pdes->scl_x_coe_pac6 = SCL_X_COE[43];
  pdes->scl_x_coe_pbc0 = SCL_X_COE[44];
  pdes->scl_x_coe_pbc2 = SCL_X_COE[45];
  pdes->scl_x_coe_pbc4 = SCL_X_COE[46];
  pdes->scl_x_coe_pbc6 = SCL_X_COE[47];
  pdes->scl_x_coe_pcc0 = SCL_X_COE[48];
  pdes->scl_x_coe_pcc2 = SCL_X_COE[49];
  pdes->scl_x_coe_pcc4 = SCL_X_COE[50];
  pdes->scl_x_coe_pcc6 = SCL_X_COE[51];
  pdes->scl_x_coe_pdc0 = SCL_X_COE[52];
  pdes->scl_x_coe_pdc2 = SCL_X_COE[53];
  pdes->scl_x_coe_pdc4 = SCL_X_COE[54];
  pdes->scl_x_coe_pdc6 = SCL_X_COE[55];
  pdes->scl_x_coe_pec0 = SCL_X_COE[56];
  pdes->scl_x_coe_pec2 = SCL_X_COE[57];
  pdes->scl_x_coe_pec4 = SCL_X_COE[58];
  pdes->scl_x_coe_pec6 = SCL_X_COE[59];
  pdes->scl_x_coe_pfc0 = SCL_X_COE[60];
  pdes->scl_x_coe_pfc2 = SCL_X_COE[61];
  pdes->scl_x_coe_pfc4 = SCL_X_COE[62];
  pdes->scl_x_coe_pfc6 = SCL_X_COE[63];
#endif


  while(1) {
    int status;

    status = (REG(VPP_STATUS) >> 8) & 0x1;
    PRINT_DEBUG(debug_level, "vpp status   %d\n", status);
    if (status == 0x01)
      break;
  }

  while(1) {
    int raw_status;

    raw_status = REG(VPP_INT_RAW_STATUS);
    PRINT_DEBUG(debug_level, "raw_status 0x%08x\n", raw_status);
    if (raw_status != 0x0) {
      REG(VPP_INT_CLEAR) = 0xffffffff;
      raw_status = REG(VPP_INT_RAW_STATUS);
      printf("raw_status 222: 0x%08x\n", raw_status);
    } else {
      break;
    }
  }

  REG(VPP_CMD_BASE) = (uint32)(des_caddr & 0xffffffff);
  REG(VPP_CMD_BASE_EXT) = (uint32)((des_caddr >> 32) & 0xff);

#ifdef VPP_PERFORMANCE_TEST
  vpp_time_start = timer_get_tick();
#endif

  REG(VPP_CONTROL0) = 0x00010001;//vpp frame start

  //print_regs(1);
  waitProc();

  #ifdef VPP_PERFORMANCE_TEST
  vpp_time_consume = (timer_get_tick() - vpp_time_start) / TIMER_CNT_US;
  printf("vpp 1920x1080 tile mode to rgb24 time consuming %lld us\n", vpp_time_consume);
  #endif

#else
  printf("here cpu config mode----------\n");
  while(1)
  {
    int status;
    status = (REG(VPP_STATUS) >> 8) & 0x1;
    PRINT_DEBUG(debug_level, "vpp status   %d\n",status);
    if (status == 0x01)
      break;
  }

  while(1)
  {
    int raw_status;
    raw_status = REG(VPP_INT_RAW_STATUS);
    PRINT_DEBUG(debug_level, "raw_status 0x%08x\n",raw_status);
    if (raw_status != 0x0)
    {
      REG(VPP_INT_CLEAR) = 0xffffffff;
      raw_status = REG(VPP_INT_RAW_STATUS);
      printf("raw_status 222: 0x%08x\n",raw_status);
    } else
    {
      break;
    }
  }

  REG(VPP_DES_HEAD) = 0x100;
  REG(VPP_NEXT_CMD_BASE) = 0x0;
  REG(VPP_NEXT_CMD_BASE_EXT) = 0x0;
  REG(VPP_PADDING_CTRL) = 0x00808080;
  REG(VPP_CSC_COE_00) = CSC_COE[0];
  REG(VPP_CSC_COE_01) = CSC_COE[1];
  REG(VPP_CSC_COE_02) = CSC_COE[2];
  REG(VPP_CSC_ADD_0) = CSC_ADD[0];
  REG(VPP_CSC_COE_10) = CSC_COE[3];
  REG(VPP_CSC_COE_11) = CSC_COE[4];
  REG(VPP_CSC_COE_12) = CSC_COE[5];
  REG(VPP_CSC_ADD_1) = CSC_ADD[1];
  REG(VPP_CSC_COE_20) = CSC_COE[6];
  REG(VPP_CSC_COE_21) = CSC_COE[7];
  REG(VPP_CSC_COE_22) = CSC_COE[8];
  REG(VPP_CSC_ADD_2) = CSC_ADD[2];
  

  REG(VPP_SRC_CTRL) = ((yuv2rgb_enable & 0x1) << 11) | ((src->tile_mode & 0x3) << 9) | ((src->plannar & 0x1) << 8) |((src->endian & 0x1) << 5) \
                                          | ((src->endian_a & 0x1) << 4) |(src->format & 0xf);
  REG(VPP_SRC_CROP_ST) = (src->axisX << 16) | src->axisY;
  REG(VPP_SRC_CROP_SIZE) = (src->cropW << 16) | src->cropH;
  REG(VPP_SRC_RY_BASE) = (uint32)(src->addr0 & 0xffffffff);             //low 32 bit addr
  REG(VPP_SRC_GU_BASE) = (uint32)(src->addr1 & 0xffffffff);
  REG(VPP_SRC_BV_BASE) = (uint32)(src->addr2 & 0xffffffff);
  REG(VPP_SRC_STRIDE) = (srcUV << 24) | src->stride;
  REG(VPP_SRC_RY_BASE_EXT) = (uint32)((src->addr0 >> 32) & 0xff);     //high 8 bit addr
  REG(VPP_SRC_GU_BASE_EXT) = (uint32)((src->addr1 >> 32) & 0xff);
  REG(VPP_SRC_BV_BASE_EXT) = (uint32)((src->addr2 >> 32) & 0xff);

  REG(VPP_DST_CTRL) = ((dst->plannar & 0x1) << 8) |((dst->endian & 0x1) << 5) | ((dst->endian_a & 0x1) << 4) |(dst->format & 0xf);
  REG(VPP_DST_CROP_ST) = (dst->axisX << 16) | dst->axisY;
  REG(VPP_DST_CROP_SIZE) = (dst->cropW << 16) | dst->cropH;
  REG(VPP_DST_RY_BASE) = (uint32)(dst->addr0 & 0xffffffff);                   //low 32 bit addr
  REG(VPP_DST_GU_BASE) = (uint32)(dst->addr1 & 0xffffffff);
  REG(VPP_DST_BV_BASE) = (uint32)(dst->addr2 & 0xffffffff);
  REG(VPP_DST_STRIDE) = (dstUV << 24) | dst->stride;
  REG(VPP_DST_RY_BASE_EXT) = (uint32)((dst->addr0 >> 32) & 0xff);            //high 8 bit addr
  REG(VPP_DST_GU_BASE_EXT) = (uint32)((dst->addr1 >> 32) & 0xff);
  REG(VPP_DST_BV_BASE_EXT) = (uint32)((dst->addr2 >> 32) & 0xff);

  /*scl_ctl*/
  REG(VPP_SCL_CTRL) = src->scl_ctrl;
  REG(VPP_SCL_INIT) = src->scl_init;

  REG(VPP_SCL_X) = floor(src->cropW * 16384 / dst->cropW);
  REG(VPP_SCL_Y) = floor(src->cropH * 16384 / dst->cropH);

  REG(VPP_PADDING_CTRL) = 0x00808080;
  REG(VPP_DST_SIZE) = 0x0;

  //REG(VPP_SCL_X) = ((src->cropW << 15) / dst->cropW + 1) >> 1;
  //REG(VPP_SCL_Y) = ((src->cropH << 15) / dst->cropH + 1) >> 1;

  REG(VPP_CONTROL0) = 0x00010001;//vpp frame start
  //print_regs(1);
  waitProc();
#endif

  return 0;
}

__attribute__((__noinline__,__noclone__)) void test_vpp_exit(void)
{
  while(1);
}
void test_nv12_tile_rgb24(void)
{
  struct rect src, dst;
  uint32 vpp_version;
  int scale_enable = 1;   //v3:0   v4:1

  int in_mode = 0;
  int in_axis_x = 0;
  int in_axis_y = 0;
  int in_crop_w = 1920;
  int in_crop_h = 1080;
  int in_stride = 1920;
  int in_endian_a = 0;
  int in_endian = 0;
  int in_plannar = 0;
  int tile_mode = 1;

  int out_mode = 2;
  int out_axis_x = 0;
  int out_axis_y = 0;
  int out_crop_w = 1920;
  int out_crop_h = 1080;
  int out_stride = 5760;
  int out_endian_a = 0;
  int out_endian = 0;
  int out_plannar = 0;

  /*scl ctrl*/
  int hor_filter_sel = 2;
  int ver_filter_sel = 2;
  int scale_x_init = 0;
  int scale_y_init = 0;

  vpp_version = REG(VPP_VERSION);
  printf("vpp version %08x\n", vpp_version);
  src.format = in_mode;
  src.stride = in_stride;
  src.videosize = ((1920 << 16) | 1080);
  
  src.addr0 = VPP_SRC_ADDR0;
  src.addr1 = VPP_SRC_ADDR1;
  src.addr2 = VPP_SRC_ADDR2;

  src.axisX = in_axis_x;
  src.axisY = in_axis_y;
  src.cropW = in_crop_w;
  src.cropH = in_crop_h;
  src.endian_a = in_endian_a;
  src.endian = in_endian;
  src.plannar = in_plannar;
  src.tile_mode = tile_mode;


  dst.format = out_mode;
  dst.stride = out_stride;

  dst.addr0 = VPP_DST_ADDR0;
  dst.addr1 = VPP_DST_ADDR1;
  dst.addr2 = VPP_DST_ADDR2;

  dst.axisX = out_axis_x;
  dst.axisY = out_axis_y;
  dst.cropW = out_crop_w;
  dst.cropH = out_crop_h;
  dst.endian_a = out_endian_a;
  dst.endian = out_endian;
  dst.plannar = out_plannar;

  /*scl ctrl*/
  src.scl_ctrl= ((ver_filter_sel & 0xf) << 12) | ((hor_filter_sel & 0xf) << 8) | (scale_enable & 0x1);
  src.scl_init= ((scale_y_init & 0x3fff) << 16) | (scale_x_init & 0x3fff);

  toColor(&src, &dst);

  DataCompare_packet24((uint8 *)(CMODEL_DST_ADDR0), (uint8 *)(VPP_DST_ADDR0), dst.axisX, dst.axisY, dst.cropW, dst.cropH, dst.stride,0);

  if (cnt1 != 0) {
  REG(vpp_ret_addr) = 0x1;
  } else {
    REG(vpp_ret_addr) = 0x0;
  }
  printf("test nv12 tile to rgb24, down scale pass, cnt1 : %d\n", cnt1);
  cnt1 = 0;
}

void test_nv12_tile_rgb32(void)
{
  struct rect src, dst;
  uint32 vpp_version;
  int scale_enable = 1;   //v3:0   v4:1

  int in_mode = 0;
  int in_axis_x = 0;
  int in_axis_y = 0;
  int in_crop_w = 1920;
  int in_crop_h = 1080;
  int in_stride = 1920;
  int in_endian_a = 0;
  int in_endian = 0;
  int in_plannar = 0;
  int tile_mode = 1;

  int out_mode = 3;
  int out_axis_x = 0;
  int out_axis_y = 0;
  int out_crop_w = 1920;
  int out_crop_h = 1080;
  int out_stride = 7680;
  int out_endian_a = 0;
  int out_endian = 0;
  int out_plannar = 0;

  /*scl ctrl*/
  int hor_filter_sel = 1;
  int ver_filter_sel = 1;
  int scale_x_init = 147;
  int scale_y_init = 147;

  vpp_version = REG(VPP_VERSION);
  printf("vpp version %08x\n", vpp_version);
  src.format = in_mode;
  src.stride = in_stride;
  src.videosize = ((1920 << 16) | 1080);
  
  src.addr0 = VPP_SRC_ADDR0;
  src.addr1 = VPP_SRC_ADDR1;
  src.addr2 = VPP_SRC_ADDR2;

  src.axisX = in_axis_x;
  src.axisY = in_axis_y;
  src.cropW = in_crop_w;
  src.cropH = in_crop_h;
  src.endian_a = in_endian_a;
  src.endian = in_endian;
  src.plannar = in_plannar;
  src.tile_mode = tile_mode;


  dst.format = out_mode;
  dst.stride = out_stride;

  dst.addr0 = VPP_DST_ADDR0;
  dst.addr1 = VPP_DST_ADDR1;
  dst.addr2 = VPP_DST_ADDR2;

  dst.axisX = out_axis_x;
  dst.axisY = out_axis_y;
  dst.cropW = out_crop_w;
  dst.cropH = out_crop_h;
  dst.endian_a = out_endian_a;
  dst.endian = out_endian;
  dst.plannar = out_plannar;

  /*scl ctrl*/
  src.scl_ctrl= ((ver_filter_sel & 0xf) << 12) | ((hor_filter_sel & 0xf) << 8) | (scale_enable & 0x1);
  src.scl_init= ((scale_y_init & 0x3fff) << 16) | (scale_x_init & 0x3fff);

  toColor(&src, &dst);

  DataCompare_packet32((uint8 *)(CMODEL_DST_ADDR0), (uint8 *)(VPP_DST_ADDR0), dst.axisX, dst.axisY, dst.cropW, dst.cropH, dst.stride,0);

  if (cnt1 != 0) {
  REG(vpp_ret_addr) = 0x1;
  } else {
    REG(vpp_ret_addr) = 0x0;
  }
  printf("test nv12 tile to rgb32, down scale pass, cnt1 : %d\n", cnt1);
  cnt1 = 0;
}

void test_yonly_tile_yonly(void)
{
  struct rect src, dst;
  uint32 vpp_version;
  int scale_enable = 1;   //v3:0   v4:1

  vpp_version = REG(VPP_VERSION);
  printf("vpp version %08x\n", vpp_version);

  /*rgb24 to rgbp*/
  int in_mode = 1;
  int in_axis_x = 0;
  int in_axis_y = 0;
  int in_crop_w = 1920;
  int in_crop_h = 1080;
  int in_stride = 1920;
  int in_endian_a = 0;
  int in_endian = 0;
  int in_plannar = 1;
  int tile_mode = 1;

  int out_mode = 1;
  int out_axis_x = 0;
  int out_axis_y = 0;
  int out_crop_w = 1920;
  int out_crop_h = 1080;
  int out_stride = 1920;
  int out_endian_a = 0;
  int out_endian = 0;
  int out_plannar = 1;

  /*scl ctrl*/
  int hor_filter_sel = 3;
  int ver_filter_sel = 3;
  int scale_x_init = 9201;
  int scale_y_init = 9201;

  src.format = in_mode;
  src.stride = in_stride;
  src.videosize = ((256 << 16) | 256);
  
  src.addr0 = VPP_SRC_ADDR0;
  src.addr1 = VPP_SRC_ADDR1;
  src.addr2 = VPP_SRC_ADDR2;

  src.axisX = in_axis_x;
  src.axisY = in_axis_y;
  src.cropW = in_crop_w;
  src.cropH = in_crop_h;
  src.endian_a = in_endian_a;
  src.endian = in_endian;
  src.plannar = in_plannar;
  src.tile_mode = tile_mode;


  dst.format = out_mode;
  dst.stride = out_stride;

  dst.addr0 = VPP_DST_ADDR0;
  dst.addr1 = VPP_DST_ADDR1;
  dst.addr2 = VPP_DST_ADDR2;

  dst.axisX = out_axis_x;
  dst.axisY = out_axis_y;
  dst.cropW = out_crop_w;
  dst.cropH = out_crop_h;
  dst.endian_a = out_endian_a;
  dst.endian = out_endian;
  dst.plannar = out_plannar;

  /*scl ctrl*/
  src.scl_ctrl= ((ver_filter_sel & 0xf) << 12) | ((hor_filter_sel & 0xf) << 8) | (scale_enable & 0x1);
  src.scl_init= ((scale_y_init & 0x3fff) << 16) | (scale_x_init & 0x3fff);

  toColor(&src, &dst);

  DataCompare_planar((uint8 *)(CMODEL_DST_ADDR0), (uint8 *)(VPP_DST_ADDR0), dst.axisX, dst.axisY, dst.cropW, dst.cropH, dst.stride,0);

  if (cnt1 != 0) {
  REG(vpp_ret_addr) = 0x1;
  } else {
    REG(vpp_ret_addr) = 0x0;
  }
  printf("test yonly tile to yonly, up & down scale pass, cnt1 : %d\n", cnt1);
  cnt1 = 0;
}


void test_nv12_tile_rgbp(void)
{
  struct rect src, dst;
  int scale_enable = 1;   //v3:0   v4:1

  int in_mode = 0;
  int in_axis_x = 0;
  int in_axis_y = 0;
  int in_crop_w = 1920;
  int in_crop_h = 1080;
  int in_stride = 1920;
  int in_endian_a = 0;
  int in_endian = 0;
  int in_plannar = 0;
  int tile_mode = 1;

  int out_mode = 2;
  int out_axis_x = 0;
  int out_axis_y = 0;
  int out_crop_w = 1920;
  int out_crop_h = 1080;
  int out_stride = 1920;
  int out_endian_a = 0;
  int out_endian = 0;
  int out_plannar = 1;

  /*scl ctrl*/
  int hor_filter_sel = 1;
  int ver_filter_sel = 1;
  int scale_x_init = 7728;
  int scale_y_init = 7728;

  src.format = in_mode;
  src.stride = in_stride;
  src.videosize = ((1920 << 16) | 1080);
  
  src.addr0 = VPP_SRC_ADDR0;
  src.addr1 = VPP_SRC_ADDR1;
  src.addr2 = VPP_SRC_ADDR2;

  src.axisX = in_axis_x;
  src.axisY = in_axis_y;
  src.cropW = in_crop_w;
  src.cropH = in_crop_h;
  src.endian_a = in_endian_a;
  src.endian = in_endian;
  src.plannar = in_plannar;
  src.tile_mode = tile_mode;


  dst.format = out_mode;
  dst.stride = out_stride;

  dst.addr0 = VPP_DST_ADDR0;
  dst.addr1 = VPP_DST_ADDR1;
  dst.addr2 = VPP_DST_ADDR2;

  dst.axisX = out_axis_x;
  dst.axisY = out_axis_y;
  dst.cropW = out_crop_w;
  dst.cropH = out_crop_h;
  dst.endian_a = out_endian_a;
  dst.endian = out_endian;
  dst.plannar = out_plannar;

  /*scl ctrl*/
  src.scl_ctrl= ((ver_filter_sel & 0xf) << 12) | ((hor_filter_sel & 0xf) << 8) | (scale_enable & 0x1);
  src.scl_init= ((scale_y_init & 0x3fff) << 16) | (scale_x_init & 0x3fff);

  toColor(&src, &dst);

  DataCompare_planar((uint8 *)(CMODEL_DST_ADDR0), (uint8 *)(VPP_DST_ADDR0), dst.axisX, dst.axisY, dst.cropW, dst.cropH, dst.stride,0);
  DataCompare_planar((uint8 *)(CMODEL_DST_ADDR1), (uint8 *)(VPP_DST_ADDR1), dst.axisX, dst.axisY, dst.cropW, dst.cropH, dst.stride,1);
  DataCompare_planar((uint8 *)(CMODEL_DST_ADDR2), (uint8 *)(VPP_DST_ADDR2), dst.axisX, dst.axisY, dst.cropW, dst.cropH, dst.stride,2);

  if (cnt1 != 0) {
  REG(vpp_ret_addr) = 0x1;
  } else {
    REG(vpp_ret_addr) = 0x0;
  }
  printf("test nv12 tile to rgbp, up & down scale pass, cnt1 : %d\n", cnt1);
  cnt1 = 0;
}

void test_nv12_tile_yuv420p(void)
{
  struct rect src, dst;
  int scale_enable = 1;   //v3:0   v4:1

  int in_mode = 0;
  int in_axis_x = 0;
  int in_axis_y = 0;
  int in_crop_w = 1920;
  int in_crop_h = 1080;
  int in_stride = 1920;
  int in_endian_a = 0;
  int in_endian = 0;
  int in_plannar = 0;
  int tile_mode = 1;

  int out_mode = 0;
  int out_axis_x = 0;
  int out_axis_y = 0;
  int out_crop_w = 1920;
  int out_crop_h = 1080;
  int out_stride = 1920;
  int out_endian_a = 0;
  int out_endian = 0;
  int out_plannar = 1;

  /*scl ctrl*/
  int hor_filter_sel = 2;
  int ver_filter_sel = 2;
  int scale_x_init = 9345;
  int scale_y_init = 9345;

  src.format = in_mode;
  src.stride = in_stride;
  src.videosize = ((1920 << 16) | 1080);
  
  src.addr0 = VPP_SRC_ADDR0;
  src.addr1 = VPP_SRC_ADDR1;
  src.addr2 = VPP_SRC_ADDR2;

  src.axisX = in_axis_x;
  src.axisY = in_axis_y;
  src.cropW = in_crop_w;
  src.cropH = in_crop_h;
  src.endian_a = in_endian_a;
  src.endian = in_endian;
  src.plannar = in_plannar;
  src.tile_mode = tile_mode;


  dst.format = out_mode;
  dst.stride = out_stride;

  dst.addr0 = VPP_DST_ADDR0;
  dst.addr1 = VPP_DST_ADDR1;
  dst.addr2 = VPP_DST_ADDR2;

  dst.axisX = out_axis_x;
  dst.axisY = out_axis_y;
  dst.cropW = out_crop_w;
  dst.cropH = out_crop_h;
  dst.endian_a = out_endian_a;
  dst.endian = out_endian;
  dst.plannar = out_plannar;

  /*scl ctrl*/
  src.scl_ctrl= ((ver_filter_sel & 0xf) << 12) | ((hor_filter_sel & 0xf) << 8) | (scale_enable & 0x1);
  src.scl_init= ((scale_y_init & 0x3fff) << 16) | (scale_x_init & 0x3fff);

  toColor(&src, &dst);

  DataCompare_planar((uint8 *)(CMODEL_DST_ADDR0), (uint8 *)(VPP_DST_ADDR0), dst.axisX, dst.axisY, dst.cropW, dst.cropH, dst.stride,0);
  DataCompare_planar((uint8 *)(CMODEL_DST_ADDR1), (uint8 *)(VPP_DST_ADDR1), dst.axisX/2, dst.axisY/2, dst.cropW/2, dst.cropH/2, dst.stride/2,1);
  DataCompare_planar((uint8 *)(CMODEL_DST_ADDR2), (uint8 *)(VPP_DST_ADDR2), dst.axisX/2, dst.axisY/2, dst.cropW/2, dst.cropH/2, dst.stride/2,2);

  if (cnt1 != 0) {
  REG(vpp_ret_addr) = 0x1;
  } else {
    REG(vpp_ret_addr) = 0x0;
  }
  printf("test nv12 tile to yuv420p, up & down scale pass, cnt1 : %d\n", cnt1);
  cnt1 = 0;
}


void test_nv12_tile_rgb24_performance(void)
{
    struct rect src, dst;
  uint32 vpp_version;
  int scale_enable = 1;   //v3:0   v4:1

  int in_mode = 0;
  int in_axis_x = 0;
  int in_axis_y = 0;
  int in_crop_w = 1920;
  int in_crop_h = 1080;
  int in_stride = 1920;
  int in_endian_a = 0;
  int in_endian = 0;
  int in_plannar = 0;
  int tile_mode = 1;

  int out_mode = 2;
  int out_axis_x = 0;
  int out_axis_y = 0;
  int out_crop_w = 1920;
  int out_crop_h = 1080;
  int out_stride = 5760;
  int out_endian_a = 0;
  int out_endian = 0;
  int out_plannar = 0;

  /*scl ctrl*/
  int hor_filter_sel = 2;
  int ver_filter_sel = 2;
  int scale_x_init = 0;
  int scale_y_init = 0;

  vpp_version = REG(VPP_VERSION);
  printf("vpp version %08x\n", vpp_version);
  src.format = in_mode;
  src.stride = in_stride;
  src.videosize = ((1920 << 16) | 1080);
  
  src.addr0 = VPP_SRC_ADDR0;
  src.addr1 = VPP_SRC_ADDR1;
  src.addr2 = VPP_SRC_ADDR2;

  src.axisX = in_axis_x;
  src.axisY = in_axis_y;
  src.cropW = in_crop_w;
  src.cropH = in_crop_h;
  src.endian_a = in_endian_a;
  src.endian = in_endian;
  src.plannar = in_plannar;
  src.tile_mode = tile_mode;


  dst.format = out_mode;
  dst.stride = out_stride;

  dst.addr0 = VPP_DST_ADDR0;
  dst.addr1 = VPP_DST_ADDR1;
  dst.addr2 = VPP_DST_ADDR2;

  dst.axisX = out_axis_x;
  dst.axisY = out_axis_y;
  dst.cropW = out_crop_w;
  dst.cropH = out_crop_h;
  dst.endian_a = out_endian_a;
  dst.endian = out_endian;
  dst.plannar = out_plannar;

  /*scl ctrl*/
  src.scl_ctrl= ((ver_filter_sel & 0xf) << 12) | ((hor_filter_sel & 0xf) << 8) | (scale_enable & 0x1);
  src.scl_init= ((scale_y_init & 0x3fff) << 16) | (scale_x_init & 0x3fff);

  toColor(&src, &dst);

  DataCompare_packet24((uint8 *)(CMODEL_DST_ADDR0), (uint8 *)(VPP_DST_ADDR0), dst.axisX, dst.axisY, dst.cropW, dst.cropH, dst.stride,0);

  if (cnt1 != 0) {
  REG(vpp_ret_addr) = 0x1;
  } else {
    REG(vpp_ret_addr) = 0x0;
  }
  printf("vpp performance test nv12 tile(1920x1080) to rgb24(1920x1080), down scale pass, cnt1 : %d\n", cnt1);
  cnt1 = 0;
}

int test_nv12_linear_rgb24(void)
{
  struct rect src, dst;
  int scale_enable = 1;   //v3:0   v4:1

  int in_mode = 0;
  int in_axis_x = 0;
  int in_axis_y = 0;
  int in_crop_w = 1920;
  int in_crop_h = 1080;
  int in_stride = 1920;
  int in_endian_a = 0;
  int in_endian = 0;
  int in_plannar = 0;
  int tile_mode = 0;

  int out_mode = 2;
  int out_axis_x = 0;
  int out_axis_y = 0;
  int out_crop_w = 1920;
  int out_crop_h = 1080;
  int out_stride = 5760;
  int out_endian_a = 0;
  int out_endian = 0;
  int out_plannar = 0;

  /*scl ctrl*/
  int hor_filter_sel = 3;
  int ver_filter_sel = 3;
  int scale_x_init = 12401;
  int scale_y_init = 12401;

  src.format = in_mode;
  src.stride = in_stride;
  src.videosize = ((1920 << 16) | 1080);

  src.addr0 = VPP_SRC_ADDR0;
  src.addr1 = VPP_SRC_ADDR1;
  src.addr2 = VPP_SRC_ADDR2;

  src.axisX = in_axis_x;
  src.axisY = in_axis_y;
  src.cropW = in_crop_w;
  src.cropH = in_crop_h;
  src.endian_a = in_endian_a;
  src.endian = in_endian;
  src.plannar = in_plannar;
  src.tile_mode = tile_mode;


  dst.format = out_mode;
  dst.stride = out_stride;

  dst.addr0 = VPP_DST_ADDR0;
  dst.addr1 = VPP_DST_ADDR1;
  dst.addr2 = VPP_DST_ADDR2;

  dst.axisX = out_axis_x;
  dst.axisY = out_axis_y;
  dst.cropW = out_crop_w;
  dst.cropH = out_crop_h;
  dst.endian_a = out_endian_a;
  dst.endian = out_endian;
  dst.plannar = out_plannar;

  /*scl ctrl*/
  src.scl_ctrl= ((ver_filter_sel & 0xf) << 12) | ((hor_filter_sel & 0xf) << 8) | (scale_enable & 0x1);
  src.scl_init= ((scale_y_init & 0x3fff) << 16) | (scale_x_init & 0x3fff);

#ifdef RANDOM_INPUT
  generate_random_input(&src, &dst);
#endif

  toColor(&src, &dst);

#ifdef CALC_MD5
  return checkMD5(&dst, "c25494b1674d83ab61fb8463f15919b7");
#else
  DataCompare_packet24((uint8 *)(CMODEL_DST_ADDR0), (uint8 *)(VPP_DST_ADDR0), dst.axisX, dst.axisY, dst.cropW, dst.cropH, dst.stride,0);

  if (cnt1 != 0) {
  REG(vpp_ret_addr) = 0x1;
  } else {
    REG(vpp_ret_addr) = 0x0;
  }
  printf("test nv12 linear to rgb24, up & down scale pass, cnt1 : %d\n", cnt1);
  cnt1 = 0;

  return 0;
#endif
}

#ifdef RANDOM_INPUT
void generate_random_input(struct rect *src, struct rect *dst)
{
    int width, height, size;
    int idx, val;
    unsigned char *start;

    my_srand(0);
    width = src->cropW;
    height = src->cropH;

    size = width * height;
    start = (unsigned char *) src->addr0;

    for (idx = 0; idx < size; idx++)
    {
        val = my_rand();
        *start++ = val  & 0xff;
    }

    size >>= 1;
    start = (unsigned char *) src->addr1;
    for (idx = 0; idx < size; idx++)
    {
        val = my_rand();
        *start++ = val & 0xff;
    }
}

// from ISO/IEC 9899
int my_rand(void) // RAND_MAX assumed to be 32767
{
    next = next * 1103515245 + 12345;
    return (unsigned int)(next/65536) % 32768;
}

void my_srand(unsigned int seed)
{
    next = seed;
}
#endif

#ifdef CALC_MD5
int checkMD5(struct rect *dst, char *pGolden)
{
    int idx, val;
    int res = 0;
    unsigned char md5[MD5_DIGEST_LENGTH];

    calculateMD5((Uint8*)dst->addr0, dst->cropW * dst->cropH * 3, md5);

    PRINT_DEBUG(debug_level, "pGolden = %s\n", pGolden);
    for (idx = 0; idx < MD5_DIGEST_LENGTH; idx++)
    {
        val = (hex2decimal(pGolden[idx * 2]) << 4) | hex2decimal(pGolden[idx * 2 + 1]);
        if (md5[idx] != (unsigned char)val)
        {
            res = -1;
            printf("idx = %d, md5 = %02x, ans = %2x, -- wrong\n",
                    idx, md5[idx], val);
            break;
        }
    }
    printf("\n");

    return res;
}

void calculateMD5(Uint8 *pAddr, int size, unsigned char *pMD5)
{
    int idx;

    MD5(pAddr, size, pMD5);
    if (debug_level & LV_DEBUG)
    {
        printf("md5 =\n");
        for (idx = 0; idx < MD5_DIGEST_LENGTH; idx++)
            printf("%02x", pMD5[idx]);
        printf("\n");
    }
}
#endif

void test_resize_nv12_linear_rgb24(void)
{
  struct rect src, dst;
  int scale_enable = 1;   //v3:0   v4:1

  int in_mode = 0;
  int in_axis_x = 0;
  int in_axis_y = 0;
  int in_crop_w = 8;
  int in_crop_h = 8;
  int in_stride = 1920;
  int in_endian_a = 0;
  int in_endian = 0;
  int in_plannar = 0;
  int tile_mode = 0;

  int out_mode = 2;
  int out_axis_x = 0;
  int out_axis_y = 0;
  int out_crop_w = 16;
  int out_crop_h = 16;
  int out_stride = 5760;
  int out_endian_a = 0;
  int out_endian = 0;
  int out_plannar = 0;

  /*scl ctrl*/
  int hor_filter_sel = 3;
  int ver_filter_sel = 3;
  int scale_x_init = 7325;
  int scale_y_init = 7325;

  src.format = in_mode;
  src.stride = in_stride;
  src.videosize = ((1920 << 16) | 1080);
  
  src.addr0 = VPP_SRC_ADDR0;
  src.addr1 = VPP_SRC_ADDR1;
  src.addr2 = VPP_SRC_ADDR2;

  src.axisX = in_axis_x;
  src.axisY = in_axis_y;
  src.cropW = in_crop_w;
  src.cropH = in_crop_h;
  src.endian_a = in_endian_a;
  src.endian = in_endian;
  src.plannar = in_plannar;
  src.tile_mode = tile_mode;


  dst.format = out_mode;
  dst.stride = out_stride;

  dst.addr0 = VPP_DST_ADDR0;
  dst.addr1 = VPP_DST_ADDR1;
  dst.addr2 = VPP_DST_ADDR2;

  dst.axisX = out_axis_x;
  dst.axisY = out_axis_y;
  dst.cropW = out_crop_w;
  dst.cropH = out_crop_h;
  dst.endian_a = out_endian_a;
  dst.endian = out_endian;
  dst.plannar = out_plannar;

  /*scl ctrl*/
  src.scl_ctrl= ((ver_filter_sel & 0xf) << 12) | ((hor_filter_sel & 0xf) << 8) | (scale_enable & 0x1);
  src.scl_init= ((scale_y_init & 0x3fff) << 16) | (scale_x_init & 0x3fff);

  toColor(&src, &dst);

  DataCompare_packet24((uint8 *)(CMODEL_DST_ADDR0), (uint8 *)(VPP_DST_ADDR0), dst.axisX, dst.axisY, dst.cropW, dst.cropH, dst.stride,0);

  if (cnt1 != 0) {
  REG(vpp_ret_addr) = 0x1;
  } else {
    REG(vpp_ret_addr) = 0x0;
  }
  printf("test_resize_nv12_linear_rgb24, up & down scale pass, cnt1 : %d\n", cnt1);
  cnt1 = 0;
}

void test_scloe_nv12_linear_rgb24(void)
{
  struct rect src, dst;
  int scale_enable = 1;   //v3:0   v4:1

  int in_mode = 0;
  int in_axis_x = 0;
  int in_axis_y = 0;
  int in_crop_w = 1920;
  int in_crop_h = 1080;
  int in_stride = 1920;
  int in_endian_a = 0;
  int in_endian = 0;
  int in_plannar = 0;
  int tile_mode = 0;

  int out_mode = 2;
  int out_axis_x = 0;
  int out_axis_y = 0;
  int out_crop_w = 1920;
  int out_crop_h = 1080;
  int out_stride = 5760;
  int out_endian_a = 0;
  int out_endian = 0;
  int out_plannar = 0;

  /*scl ctrl*/
  int hor_filter_sel = 15;
  int ver_filter_sel = 15;
  int scale_x_init = 10394;
  int scale_y_init = 10394;

  src.format = in_mode;
  src.stride = in_stride;
  src.videosize = ((1920 << 16) | 1080);
  
  src.addr0 = VPP_SRC_ADDR0;
  src.addr1 = VPP_SRC_ADDR1;
  src.addr2 = VPP_SRC_ADDR2;

  src.axisX = in_axis_x;
  src.axisY = in_axis_y;
  src.cropW = in_crop_w;
  src.cropH = in_crop_h;
  src.endian_a = in_endian_a;
  src.endian = in_endian;
  src.plannar = in_plannar;
  src.tile_mode = tile_mode;


  dst.format = out_mode;
  dst.stride = out_stride;

  dst.addr0 = VPP_DST_ADDR0;
  dst.addr1 = VPP_DST_ADDR1;
  dst.addr2 = VPP_DST_ADDR2;

  dst.axisX = out_axis_x;
  dst.axisY = out_axis_y;
  dst.cropW = out_crop_w;
  dst.cropH = out_crop_h;
  dst.endian_a = out_endian_a;
  dst.endian = out_endian;
  dst.plannar = out_plannar;

  /*scl ctrl*/
  src.scl_ctrl= ((ver_filter_sel & 0xf) << 12) | ((hor_filter_sel & 0xf) << 8) | (scale_enable & 0x1);
  src.scl_init= ((scale_y_init & 0x3fff) << 16) | (scale_x_init & 0x3fff);

  toColor(&src, &dst);

  DataCompare_packet24((uint8 *)(CMODEL_DST_ADDR0), (uint8 *)(VPP_DST_ADDR0), dst.axisX, dst.axisY, dst.cropW, dst.cropH, dst.stride,0);

  if (cnt1 != 0) {
  REG(vpp_ret_addr) = 0x1;
  } else {
    REG(vpp_ret_addr) = 0x0;
  }
  printf("test_scloe_nv12_linear_rgb24, up & down scale pass, cnt1 : %d\n", cnt1);
  cnt1 = 0;
}

void test_nv12_linear_rgb32(void)
{
  struct rect src, dst;
  int scale_enable = 1;   //v3:0   v4:1

  int in_mode = 0;
  int in_axis_x = 0;
  int in_axis_y = 0;
  int in_crop_w = 1920;
  int in_crop_h = 1080;
  int in_stride = 1920;
  int in_endian_a = 0;
  int in_endian = 0;
  int in_plannar = 0;
  int tile_mode = 0;

  int out_mode = 3;
  int out_axis_x = 0;
  int out_axis_y = 0;
  int out_crop_w = 1920;
  int out_crop_h = 1080;
  int out_stride = 7680;
  int out_endian_a = 0;
  int out_endian = 0;
  int out_plannar = 0;

  /*scl ctrl*/
  int hor_filter_sel = 2;
  int ver_filter_sel = 2;
  int scale_x_init = 7328;
  int scale_y_init = 7328;

  src.format = in_mode;
  src.stride = in_stride;
  src.videosize = ((1920 << 16) | 1080);
  
  src.addr0 = VPP_SRC_ADDR0;
  src.addr1 = VPP_SRC_ADDR1;
  src.addr2 = VPP_SRC_ADDR2;

  src.axisX = in_axis_x;
  src.axisY = in_axis_y;
  src.cropW = in_crop_w;
  src.cropH = in_crop_h;
  src.endian_a = in_endian_a;
  src.endian = in_endian;
  src.plannar = in_plannar;
  src.tile_mode = tile_mode;


  dst.format = out_mode;
  dst.stride = out_stride;

  dst.addr0 = VPP_DST_ADDR0;
  dst.addr1 = VPP_DST_ADDR1;
  dst.addr2 = VPP_DST_ADDR2;

  dst.axisX = out_axis_x;
  dst.axisY = out_axis_y;
  dst.cropW = out_crop_w;
  dst.cropH = out_crop_h;
  dst.endian_a = out_endian_a;
  dst.endian = out_endian;
  dst.plannar = out_plannar;

  /*scl ctrl*/
  src.scl_ctrl= ((ver_filter_sel & 0xf) << 12) | ((hor_filter_sel & 0xf) << 8) | (scale_enable & 0x1);
  src.scl_init= ((scale_y_init & 0x3fff) << 16) | (scale_x_init & 0x3fff);

  toColor(&src, &dst);

  DataCompare_packet32((uint8 *)(CMODEL_DST_ADDR0), (uint8 *)(VPP_DST_ADDR0), dst.axisX, dst.axisY, dst.cropW, dst.cropH, dst.stride,0);

  if (cnt1 != 0) {
  REG(vpp_ret_addr) = 0x1;
  } else {
    REG(vpp_ret_addr) = 0x0;
  }
  printf("test nv12 linear to rgb32, up & down scale pass, cnt1 : %d\n", cnt1);
  cnt1 = 0;
}

void test_nv12_linear_rgbp(void)
{
  struct rect src, dst;
  int scale_enable = 1;   //v3:0   v4:1

  int in_mode = 0;
  int in_axis_x = 0;
  int in_axis_y = 0;
  int in_crop_w = 1920;
  int in_crop_h = 1080;
  int in_stride = 1920;
  int in_endian_a = 0;
  int in_endian = 0;
  int in_plannar = 0;
  int tile_mode = 0;

  int out_mode = 2;
  int out_axis_x = 0;
  int out_axis_y = 0;
  int out_crop_w = 1920;
  int out_crop_h = 1080;
  int out_stride = 1920;
  int out_endian_a = 0;
  int out_endian = 0;
  int out_plannar = 1;

  /*scl ctrl*/
  int hor_filter_sel = 1;
  int ver_filter_sel = 1;
  int scale_x_init = 10728;
  int scale_y_init = 10728;

  src.format = in_mode;
  src.stride = in_stride;
  src.videosize = ((1920 << 16) | 1080);
  
  src.addr0 = VPP_SRC_ADDR0;
  src.addr1 = VPP_SRC_ADDR1;
  src.addr2 = VPP_SRC_ADDR2;

  src.axisX = in_axis_x;
  src.axisY = in_axis_y;
  src.cropW = in_crop_w;
  src.cropH = in_crop_h;
  src.endian_a = in_endian_a;
  src.endian = in_endian;
  src.plannar = in_plannar;
  src.tile_mode = tile_mode;


  dst.format = out_mode;
  dst.stride = out_stride;

  dst.addr0 = VPP_DST_ADDR0;
  dst.addr1 = VPP_DST_ADDR1;
  dst.addr2 = VPP_DST_ADDR2;

  dst.axisX = out_axis_x;
  dst.axisY = out_axis_y;
  dst.cropW = out_crop_w;
  dst.cropH = out_crop_h;
  dst.endian_a = out_endian_a;
  dst.endian = out_endian;
  dst.plannar = out_plannar;

  /*scl ctrl*/
  src.scl_ctrl= ((ver_filter_sel & 0xf) << 12) | ((hor_filter_sel & 0xf) << 8) | (scale_enable & 0x1);
  src.scl_init= ((scale_y_init & 0x3fff) << 16) | (scale_x_init & 0x3fff);

  toColor(&src, &dst);

  DataCompare_planar((uint8 *)(CMODEL_DST_ADDR0), (uint8 *)(VPP_DST_ADDR0), dst.axisX, dst.axisY, dst.cropW, dst.cropH, dst.stride,0);
  DataCompare_planar((uint8 *)(CMODEL_DST_ADDR1), (uint8 *)(VPP_DST_ADDR1), dst.axisX, dst.axisY, dst.cropW, dst.cropH, dst.stride,1);
  DataCompare_planar((uint8 *)(CMODEL_DST_ADDR2), (uint8 *)(VPP_DST_ADDR2), dst.axisX, dst.axisY, dst.cropW, dst.cropH, dst.stride,2);

  if (cnt1 != 0) {
  REG(vpp_ret_addr) = 0x1;
  } else {
    REG(vpp_ret_addr) = 0x0;
  }
  printf("test nv12 linear to rgbp, up & down scale pass, cnt1 : %d\n", cnt1);
  cnt1 = 0;
}

void test_nv12_linear_yuv420p(void)
{
  struct rect src, dst;
  int scale_enable = 1;   //v3:0   v4:1

  int in_mode = 0;
  int in_axis_x = 0;
  int in_axis_y = 0;
  int in_crop_w = 1920;
  int in_crop_h = 1080;
  int in_stride = 1920;
  int in_endian_a = 0;
  int in_endian = 0;
  int in_plannar = 0;
  int tile_mode = 0;

  int out_mode = 0;
  int out_axis_x = 0;
  int out_axis_y = 0;
  int out_crop_w = 1920;
  int out_crop_h = 1080;
  int out_stride = 1920;
  int out_endian_a = 0;
  int out_endian = 0;
  int out_plannar = 1;

  /*scl ctrl*/
  int hor_filter_sel = 0;
  int ver_filter_sel = 0;
  int scale_x_init = 8515;
  int scale_y_init = 8515;

  src.format = in_mode;
  src.stride = in_stride;
  src.videosize = ((1920 << 16) | 1080);
  
  src.addr0 = VPP_SRC_ADDR0;
  src.addr1 = VPP_SRC_ADDR1;
  src.addr2 = VPP_SRC_ADDR2;

  src.axisX = in_axis_x;
  src.axisY = in_axis_y;
  src.cropW = in_crop_w;
  src.cropH = in_crop_h;
  src.endian_a = in_endian_a;
  src.endian = in_endian;
  src.plannar = in_plannar;
  src.tile_mode = tile_mode;


  dst.format = out_mode;
  dst.stride = out_stride;

  dst.addr0 = VPP_DST_ADDR0;
  dst.addr1 = VPP_DST_ADDR1;
  dst.addr2 = VPP_DST_ADDR2;

  dst.axisX = out_axis_x;
  dst.axisY = out_axis_y;
  dst.cropW = out_crop_w;
  dst.cropH = out_crop_h;
  dst.endian_a = out_endian_a;
  dst.endian = out_endian;
  dst.plannar = out_plannar;

  /*scl ctrl*/
  src.scl_ctrl= ((ver_filter_sel & 0xf) << 12) | ((hor_filter_sel & 0xf) << 8) | (scale_enable & 0x1);
  src.scl_init= ((scale_y_init & 0x3fff) << 16) | (scale_x_init & 0x3fff);

  toColor(&src, &dst);

  DataCompare_planar((uint8 *)(CMODEL_DST_ADDR0), (uint8 *)(VPP_DST_ADDR0), dst.axisX, dst.axisY, dst.cropW, dst.cropH, dst.stride,0);
  DataCompare_planar((uint8 *)(CMODEL_DST_ADDR1), (uint8 *)(VPP_DST_ADDR1), dst.axisX/2, dst.axisY/2, dst.cropW/2, dst.cropH/2, dst.stride/2,1);
  DataCompare_planar((uint8 *)(CMODEL_DST_ADDR2), (uint8 *)(VPP_DST_ADDR2), dst.axisX/2, dst.axisY/2, dst.cropW/2, dst.cropH/2, dst.stride/2,2);

  if (cnt1 != 0) {
  REG(vpp_ret_addr) = 0x1;
  } else {
    REG(vpp_ret_addr) = 0x0;
  }
  printf("test nv12 linear to yuv420p, up & down scale pass, cnt1 : %d\n", cnt1);
  cnt1 = 0;
}

void test_rgb32_rgbp(void)
{
  struct rect src, dst;
  uint32 vpp_version;
  int scale_enable = 1;   //v3:0   v4:1

  vpp_version = REG(VPP_VERSION);
  printf("vpp version %08x\n", vpp_version);

  /*rgb24 to rgbp*/
  int in_mode = 3;
  int in_axis_x = 0;
  int in_axis_y = 0;
  int in_crop_w = 1920;
  int in_crop_h = 1080;
  int in_stride = 7680;
  int in_endian_a = 0;
  int in_endian = 0;
  int in_plannar = 0;
  int tile_mode = 0;

  int out_mode = 2;
  int out_axis_x = 0;
  int out_axis_y = 0;
  int out_crop_w = 1920;
  int out_crop_h = 1080;
  int out_stride = 1920;
  int out_endian_a = 0;
  int out_endian = 0;
  int out_plannar = 1;

  /*scl ctrl*/
  int hor_filter_sel = 4;
  int ver_filter_sel = 4;
  int scale_x_init = 7070;
  int scale_y_init = 7070;

  src.format = in_mode;
  src.stride = in_stride;
  src.videosize = ((64 << 16) | 64);
  
  src.addr0 = VPP_SRC_ADDR0;
  src.addr1 = VPP_SRC_ADDR1;
  src.addr2 = VPP_SRC_ADDR2;

  src.axisX = in_axis_x;
  src.axisY = in_axis_y;
  src.cropW = in_crop_w;
  src.cropH = in_crop_h;
  src.endian_a = in_endian_a;
  src.endian = in_endian;
  src.plannar = in_plannar;
  src.tile_mode = tile_mode;


  dst.format = out_mode;
  dst.stride = out_stride;

  dst.addr0 = VPP_DST_ADDR0;
  dst.addr1 = VPP_DST_ADDR1;
  dst.addr2 = VPP_DST_ADDR2;

  dst.axisX = out_axis_x;
  dst.axisY = out_axis_y;
  dst.cropW = out_crop_w;
  dst.cropH = out_crop_h;
  dst.endian_a = out_endian_a;
  dst.endian = out_endian;
  dst.plannar = out_plannar;

  /*scl ctrl*/
  src.scl_ctrl= ((ver_filter_sel & 0xf) << 12) | ((hor_filter_sel & 0xf) << 8) | (scale_enable & 0x1);
  src.scl_init= ((scale_y_init & 0x3fff) << 16) | (scale_x_init & 0x3fff);

  toColor(&src, &dst);

  DataCompare_planar((uint8 *)(CMODEL_DST_ADDR0), (uint8 *)(VPP_DST_ADDR0), dst.axisX, dst.axisY, dst.cropW, dst.cropH, dst.stride,0);
  DataCompare_planar((uint8 *)(CMODEL_DST_ADDR1), (uint8 *)(VPP_DST_ADDR1), dst.axisX, dst.axisY, dst.cropW, dst.cropH, dst.stride,1);
  DataCompare_planar((uint8 *)(CMODEL_DST_ADDR2), (uint8 *)(VPP_DST_ADDR2), dst.axisX, dst.axisY, dst.cropW, dst.cropH, dst.stride,2);

  if (cnt1 != 0) {
  REG(vpp_ret_addr) = 0x1;
  } else {
    REG(vpp_ret_addr) = 0x0;
  }
  printf("test rgb32 to rgbp, up scale pass, cnt1 : %d\n", cnt1);
  cnt1 = 0;
}


void test_rgb24_rgbp(void)
{
  struct rect src, dst;
  uint32 vpp_version;
  int scale_enable = 1;   //v3:0   v4:1

  vpp_version = REG(VPP_VERSION);
  printf("vpp version %08x\n", vpp_version);

  /*rgb24 to rgbp*/
  int in_mode = 2;
  int in_axis_x = 0;
  int in_axis_y = 0;
  int in_crop_w = 1920;
  int in_crop_h = 1080;
  int in_stride = 5760;
  int in_endian_a = 0;
  int in_endian = 0;
  int in_plannar = 0;
  int tile_mode = 0;

  int out_mode = 2;
  int out_axis_x = 0;
  int out_axis_y = 0;
  int out_crop_w = 1920;
  int out_crop_h = 1080;
  int out_stride = 1920;
  int out_endian_a = 0;
  int out_endian = 0;
  int out_plannar = 1;

  /*scl ctrl*/
  int hor_filter_sel = 0;
  int ver_filter_sel = 0;
  int scale_x_init = 10264;
  int scale_y_init = 10264;

  src.format = in_mode;
  src.stride = in_stride;
  src.videosize = ((64 << 16) | 64);
  
  src.addr0 = VPP_SRC_ADDR0;
  src.addr1 = VPP_SRC_ADDR1;
  src.addr2 = VPP_SRC_ADDR2;

  src.axisX = in_axis_x;
  src.axisY = in_axis_y;
  src.cropW = in_crop_w;
  src.cropH = in_crop_h;
  src.endian_a = in_endian_a;
  src.endian = in_endian;
  src.plannar = in_plannar;
  src.tile_mode = tile_mode;


  dst.format = out_mode;
  dst.stride = out_stride;

  dst.addr0 = VPP_DST_ADDR0;
  dst.addr1 = VPP_DST_ADDR1;
  dst.addr2 = VPP_DST_ADDR2;

  dst.axisX = out_axis_x;
  dst.axisY = out_axis_y;
  dst.cropW = out_crop_w;
  dst.cropH = out_crop_h;
  dst.endian_a = out_endian_a;
  dst.endian = out_endian;
  dst.plannar = out_plannar;

  /*scl ctrl*/
  src.scl_ctrl= ((ver_filter_sel & 0xf) << 12) | ((hor_filter_sel & 0xf) << 8) | (scale_enable & 0x1);
  src.scl_init= ((scale_y_init & 0x3fff) << 16) | (scale_x_init & 0x3fff);

  toColor(&src, &dst);

  DataCompare_planar((uint8 *)(CMODEL_DST_ADDR0), (uint8 *)(VPP_DST_ADDR0), dst.axisX, dst.axisY, dst.cropW, dst.cropH, dst.stride,0);
  DataCompare_planar((uint8 *)(CMODEL_DST_ADDR1), (uint8 *)(VPP_DST_ADDR1), dst.axisX, dst.axisY, dst.cropW, dst.cropH, dst.stride,1);
  DataCompare_planar((uint8 *)(CMODEL_DST_ADDR2), (uint8 *)(VPP_DST_ADDR2), dst.axisX, dst.axisY, dst.cropW, dst.cropH, dst.stride,2);

  if (cnt1 != 0) {
  REG(vpp_ret_addr) = 0x1;
  } else {
    REG(vpp_ret_addr) = 0x0;
  }
  printf("test rgb24 to rgbp, up scale pass, cnt1 : %d\n", cnt1);
  cnt1 = 0;
}

void test_rgbp_rgbp(void)
{
  struct rect src, dst;
  uint32 vpp_version;
  int scale_enable = 1;   //v3:0   v4:1

  vpp_version = REG(VPP_VERSION);
  printf("vpp version %08x\n", vpp_version);

  /*rgb24 to rgbp*/
  int in_mode = 2;
  int in_axis_x = 0;
  int in_axis_y = 0;
  int in_crop_w = 1920;
  int in_crop_h = 1080;
  int in_stride = 1920;
  int in_endian_a = 0;
  int in_endian = 0;
  int in_plannar = 1;
  int tile_mode = 0;

  int out_mode = 2;
  int out_axis_x = 0;
  int out_axis_y = 0;
  int out_crop_w = 1920;
  int out_crop_h = 1080;
  int out_stride = 1920;
  int out_endian_a = 0;
  int out_endian = 0;
  int out_plannar = 1;

  /*scl ctrl*/
  int hor_filter_sel = 1;
  int ver_filter_sel = 1;
  int scale_x_init = 2137;
  int scale_y_init = 2137;

  src.format = in_mode;
  src.stride = in_stride;
  src.videosize = ((64 << 16) | 64);
  
  src.addr0 = VPP_SRC_ADDR0;
  src.addr1 = VPP_SRC_ADDR1;
  src.addr2 = VPP_SRC_ADDR2;

  src.axisX = in_axis_x;
  src.axisY = in_axis_y;
  src.cropW = in_crop_w;
  src.cropH = in_crop_h;
  src.endian_a = in_endian_a;
  src.endian = in_endian;
  src.plannar = in_plannar;
  src.tile_mode = tile_mode;


  dst.format = out_mode;
  dst.stride = out_stride;

  dst.addr0 = VPP_DST_ADDR0;
  dst.addr1 = VPP_DST_ADDR1;
  dst.addr2 = VPP_DST_ADDR2;

  dst.axisX = out_axis_x;
  dst.axisY = out_axis_y;
  dst.cropW = out_crop_w;
  dst.cropH = out_crop_h;
  dst.endian_a = out_endian_a;
  dst.endian = out_endian;
  dst.plannar = out_plannar;

  /*scl ctrl*/
  src.scl_ctrl= ((ver_filter_sel & 0xf) << 12) | ((hor_filter_sel & 0xf) << 8) | (scale_enable & 0x1);
  src.scl_init= ((scale_y_init & 0x3fff) << 16) | (scale_x_init & 0x3fff);

  toColor(&src, &dst);

  DataCompare_planar((uint8 *)(CMODEL_DST_ADDR0), (uint8 *)(VPP_DST_ADDR0), dst.axisX, dst.axisY, dst.cropW, dst.cropH, dst.stride,0);
  DataCompare_planar((uint8 *)(CMODEL_DST_ADDR1), (uint8 *)(VPP_DST_ADDR1), dst.axisX, dst.axisY, dst.cropW, dst.cropH, dst.stride,1);
  DataCompare_planar((uint8 *)(CMODEL_DST_ADDR2), (uint8 *)(VPP_DST_ADDR2), dst.axisX, dst.axisY, dst.cropW, dst.cropH, dst.stride,2);

  if (cnt1 != 0) {
  REG(vpp_ret_addr) = 0x1;
  } else {
    REG(vpp_ret_addr) = 0x0;
  }
  printf("test rgbp to rgbp, up scale pass, cnt1 : %d\n", cnt1);
  cnt1 = 0;
}

void test_yuv420p_rgbp(void)
{
  struct rect src, dst;
  uint32 vpp_version;
  int scale_enable = 1;   //v3:0   v4:1

  vpp_version = REG(VPP_VERSION);
  printf("vpp version %08x\n", vpp_version);

  /*rgb24 to rgbp*/
  int in_mode = 0;
  int in_axis_x = 0;
  int in_axis_y = 0;
  int in_crop_w = 1920;
  int in_crop_h = 1080;
  int in_stride = 1920;
  int in_endian_a = 0;
  int in_endian = 0;
  int in_plannar = 1;
  int tile_mode = 0;

  int out_mode = 2;
  int out_axis_x = 0;
  int out_axis_y = 0;
  int out_crop_w = 1920;
  int out_crop_h = 1080;
  int out_stride = 1920;
  int out_endian_a = 0;
  int out_endian = 0;
  int out_plannar = 1;

  /*scl ctrl*/
  int hor_filter_sel = 0;
  int ver_filter_sel = 0;
  int scale_x_init = 15567;
  int scale_y_init = 15567;

  src.format = in_mode;
  src.stride = in_stride;
  src.videosize = ((64 << 16) | 64);
  
  src.addr0 = VPP_SRC_ADDR0;
  src.addr1 = VPP_SRC_ADDR1;
  src.addr2 = VPP_SRC_ADDR2;

  src.axisX = in_axis_x;
  src.axisY = in_axis_y;
  src.cropW = in_crop_w;
  src.cropH = in_crop_h;
  src.endian_a = in_endian_a;
  src.endian = in_endian;
  src.plannar = in_plannar;
  src.tile_mode = tile_mode;


  dst.format = out_mode;
  dst.stride = out_stride;

  dst.addr0 = VPP_DST_ADDR0;
  dst.addr1 = VPP_DST_ADDR1;
  dst.addr2 = VPP_DST_ADDR2;

  dst.axisX = out_axis_x;
  dst.axisY = out_axis_y;
  dst.cropW = out_crop_w;
  dst.cropH = out_crop_h;
  dst.endian_a = out_endian_a;
  dst.endian = out_endian;
  dst.plannar = out_plannar;

  /*scl ctrl*/
  src.scl_ctrl= ((ver_filter_sel & 0xf) << 12) | ((hor_filter_sel & 0xf) << 8) | (scale_enable & 0x1);
  src.scl_init= ((scale_y_init & 0x3fff) << 16) | (scale_x_init & 0x3fff);

  toColor(&src, &dst);

  DataCompare_planar((uint8 *)(CMODEL_DST_ADDR0), (uint8 *)(VPP_DST_ADDR0), dst.axisX, dst.axisY, dst.cropW, dst.cropH, dst.stride,0);
  DataCompare_planar((uint8 *)(CMODEL_DST_ADDR1), (uint8 *)(VPP_DST_ADDR1), dst.axisX, dst.axisY, dst.cropW, dst.cropH, dst.stride,1);
  DataCompare_planar((uint8 *)(CMODEL_DST_ADDR2), (uint8 *)(VPP_DST_ADDR2), dst.axisX, dst.axisY, dst.cropW, dst.cropH, dst.stride,2);

  if (cnt1 != 0) {
  REG(vpp_ret_addr) = 0x1;
  } else {
    REG(vpp_ret_addr) = 0x0;
  }
  printf("test yuv420p to rgbp, up scale pass, cnt1 : %d\n", cnt1);
  cnt1 = 0;
}

void test_yuv420p_yuv420p(void)
{
  struct rect src, dst;
  uint32 vpp_version;
  int scale_enable = 1;   //v3:0   v4:1

  vpp_version = REG(VPP_VERSION);
  printf("vpp version %08x\n", vpp_version);

  /*rgb24 to rgbp*/
  int in_mode = 0;
  int in_axis_x = 0;
  int in_axis_y = 0;
  int in_crop_w = 1920;
  int in_crop_h = 1080;
  int in_stride = 1920;
  int in_endian_a = 0;
  int in_endian = 0;
  int in_plannar = 1;
  int tile_mode = 0;

  int out_mode = 0;
  int out_axis_x = 0;
  int out_axis_y = 0;
  int out_crop_w = 1920;
  int out_crop_h = 1080;
  int out_stride = 1920;
  int out_endian_a = 0;
  int out_endian = 0;
  int out_plannar = 1;

  /*scl ctrl*/
  int hor_filter_sel = 0;
  int ver_filter_sel = 0;
  int scale_x_init = 3635;
  int scale_y_init = 3635;

  src.format = in_mode;
  src.stride = in_stride;
  src.videosize = ((64 << 16) | 64);
  
  src.addr0 = VPP_SRC_ADDR0;
  src.addr1 = VPP_SRC_ADDR1;
  src.addr2 = VPP_SRC_ADDR2;

  src.axisX = in_axis_x;
  src.axisY = in_axis_y;
  src.cropW = in_crop_w;
  src.cropH = in_crop_h;
  src.endian_a = in_endian_a;
  src.endian = in_endian;
  src.plannar = in_plannar;
  src.tile_mode = tile_mode;


  dst.format = out_mode;
  dst.stride = out_stride;

  dst.addr0 = VPP_DST_ADDR0;
  dst.addr1 = VPP_DST_ADDR1;
  dst.addr2 = VPP_DST_ADDR2;

  dst.axisX = out_axis_x;
  dst.axisY = out_axis_y;
  dst.cropW = out_crop_w;
  dst.cropH = out_crop_h;
  dst.endian_a = out_endian_a;
  dst.endian = out_endian;
  dst.plannar = out_plannar;

  /*scl ctrl*/
  src.scl_ctrl= ((ver_filter_sel & 0xf) << 12) | ((hor_filter_sel & 0xf) << 8) | (scale_enable & 0x1);
  src.scl_init= ((scale_y_init & 0x3fff) << 16) | (scale_x_init & 0x3fff);

  toColor(&src, &dst);

  DataCompare_planar((uint8 *)(CMODEL_DST_ADDR0), (uint8 *)(VPP_DST_ADDR0), dst.axisX, dst.axisY, dst.cropW, dst.cropH, dst.stride,0);
  DataCompare_planar((uint8 *)(CMODEL_DST_ADDR1), (uint8 *)(VPP_DST_ADDR1), dst.axisX/2, dst.axisY/2, dst.cropW/2, dst.cropH/2, dst.stride/2,1);
  DataCompare_planar((uint8 *)(CMODEL_DST_ADDR2), (uint8 *)(VPP_DST_ADDR2), dst.axisX/2, dst.axisY/2, dst.cropW/2, dst.cropH/2, dst.stride/2,2);

  if (cnt1 != 0) {
  REG(vpp_ret_addr) = 0x1;
  } else {
    REG(vpp_ret_addr) = 0x0;
  }
  printf("test yuv420p to yuv420p, up scale pass, cnt1 : %d\n", cnt1);
  cnt1 = 0;
}

void test_nv12_tile_rgbp_switch_yuv420p_linear_yuv420p(void)
{
  struct rect src, dst, src1, dst1;
  uint32 vpp_version;
  int scale_enable = 1;   //v3:0   v4:1

  vpp_version = REG(VPP_VERSION);
  printf("vpp version %08x\n", vpp_version);
  
/*nv12_tile_rgbp*/
  int in_mode = 0;
  int in_axis_x = 0;
  int in_axis_y = 0;
  int in_crop_w = 1920;
  int in_crop_h = 1080;
  int in_stride = 1920;
  int in_endian_a = 0;
  int in_endian = 0;
  int in_plannar = 0;
  int tile_mode = 1;

  int out_mode = 2;
  int out_axis_x = 0;
  int out_axis_y = 0;
  int out_crop_w = 1920;
  int out_crop_h = 1080;
  int out_stride = 1920;
  int out_endian_a = 0;
  int out_endian = 0;
  int out_plannar = 1;

  /*scl ctrl*/
  int hor_filter_sel = 1;
  int ver_filter_sel = 1;
  int scale_x_init = 7728;
  int scale_y_init = 7728;

  src1.format = in_mode;
  src1.stride = in_stride;
  src1.videosize = ((1920 << 16) | 1080);
  
  src1.addr0 = VPP_SRC_ADDR3;
  src1.addr1 = VPP_SRC_ADDR4;
  src1.addr2 = VPP_SRC_ADDR5;

  src1.axisX = in_axis_x;
  src1.axisY = in_axis_y;
  src1.cropW = in_crop_w;
  src1.cropH = in_crop_h;
  src1.endian_a = in_endian_a;
  src1.endian = in_endian;
  src1.plannar = in_plannar;
  src1.tile_mode = tile_mode;


  dst1.format = out_mode;
  dst1.stride = out_stride;

  dst1.addr0 = VPP_DST_ADDR3;
  dst1.addr1 = VPP_DST_ADDR4;
  dst1.addr2 = VPP_DST_ADDR5;

  dst1.axisX = out_axis_x;
  dst1.axisY = out_axis_y;
  dst1.cropW = out_crop_w;
  dst1.cropH = out_crop_h;
  dst1.endian_a = out_endian_a;
  dst1.endian = out_endian;
  dst1.plannar = out_plannar;

  /*scl ctrl*/
  src1.scl_ctrl= ((ver_filter_sel & 0xf) << 12) | ((hor_filter_sel & 0xf) << 8) | (scale_enable & 0x1);
  src1.scl_init= ((scale_y_init & 0x3fff) << 16) | (scale_x_init & 0x3fff);
/*nv12_tile_rgbp  end*/

  /*yuv420p_linear_yuv420p*/
  in_mode = 0;
  in_axis_x = 0;
  in_axis_y = 0;
  in_crop_w = 1920;
  in_crop_h = 1080;
  in_stride = 1920;
  in_endian_a = 0;
  in_endian = 0;
  in_plannar = 1;
  tile_mode = 0;

  out_mode = 0;
  out_axis_x = 0;
  out_axis_y = 0;
  out_crop_w = 1920;
  out_crop_h = 1080;
  out_stride = 1920;
  out_endian_a = 0;
  out_endian = 0;
  out_plannar = 1;

  /*scl ctrl*/
  hor_filter_sel = 0;
  ver_filter_sel = 0;
  scale_x_init = 3635;
  scale_y_init = 3635;

  src.format = in_mode;
  src.stride = in_stride;
  src.videosize = ((64 << 16) | 64);
  
  src.addr0 = VPP_SRC_ADDR0;
  src.addr1 = VPP_SRC_ADDR1;
  src.addr2 = VPP_SRC_ADDR2;

  src.axisX = in_axis_x;
  src.axisY = in_axis_y;
  src.cropW = in_crop_w;
  src.cropH = in_crop_h;
  src.endian_a = in_endian_a;
  src.endian = in_endian;
  src.plannar = in_plannar;
  src.tile_mode = tile_mode;


  dst.format = out_mode;
  dst.stride = out_stride;

  dst.addr0 = VPP_DST_ADDR0;
  dst.addr1 = VPP_DST_ADDR1;
  dst.addr2 = VPP_DST_ADDR2;

  dst.axisX = out_axis_x;
  dst.axisY = out_axis_y;
  dst.cropW = out_crop_w;
  dst.cropH = out_crop_h;
  dst.endian_a = out_endian_a;
  dst.endian = out_endian;
  dst.plannar = out_plannar;

  /*scl ctrl*/
  src.scl_ctrl= ((ver_filter_sel & 0xf) << 12) | ((hor_filter_sel & 0xf) << 8) | (scale_enable & 0x1);
  src.scl_init= ((scale_y_init & 0x3fff) << 16) | (scale_x_init & 0x3fff);
/*yuv420p_linear_yuv420p end*/

  toColorCrop2(&src1, &dst1, &src, &dst);

  DataCompare_planar((uint8 *)(CMODEL_DST_ADDR0), (uint8 *)(VPP_DST_ADDR0), dst.axisX, dst.axisY, dst.cropW, dst.cropH, dst.stride,0);
  DataCompare_planar((uint8 *)(CMODEL_DST_ADDR1), (uint8 *)(VPP_DST_ADDR1), dst.axisX/2, dst.axisY/2, dst.cropW/2, dst.cropH/2, dst.stride/2,1);
  DataCompare_planar((uint8 *)(CMODEL_DST_ADDR2), (uint8 *)(VPP_DST_ADDR2), dst.axisX/2, dst.axisY/2, dst.cropW/2, dst.cropH/2, dst.stride/2,2);

  DataCompare_planar((uint8 *)(CMODEL_DST_ADDR3), (uint8 *)(VPP_DST_ADDR3), dst1.axisX, dst1.axisY, dst1.cropW, dst1.cropH, dst1.stride,0);
  DataCompare_planar((uint8 *)(CMODEL_DST_ADDR4), (uint8 *)(VPP_DST_ADDR4), dst1.axisX, dst1.axisY, dst1.cropW, dst1.cropH, dst1.stride,1);
  DataCompare_planar((uint8 *)(CMODEL_DST_ADDR5), (uint8 *)(VPP_DST_ADDR5), dst1.axisX, dst1.axisY, dst1.cropW, dst1.cropH, dst1.stride,2);
  
  if (cnt1 != 0) {
  REG(vpp_ret_addr) = 0x1;
  } else {
    REG(vpp_ret_addr) = 0x0;
  }
  printf("test_nv12_tile_rgbp_switch_yuv420p_linear_yuv420p, up & down scale pass, cnt1 : %d\n", cnt1);
  cnt1 = 0;
}



void test_yonly_yonly(void)
{
  struct rect src, dst;
  uint32 vpp_version;
  int scale_enable = 1;   //v3:0   v4:1

  vpp_version = REG(VPP_VERSION);
  printf("vpp version %08x\n", vpp_version);

  /*rgb24 to rgbp*/
  int in_mode = 1;
  int in_axis_x = 0;
  int in_axis_y = 0;
  int in_crop_w = 1920;
  int in_crop_h = 1080;
  int in_stride = 1920;
  int in_endian_a = 0;
  int in_endian = 0;
  int in_plannar = 1;
  int tile_mode = 0;

  int out_mode = 1;
  int out_axis_x = 0;
  int out_axis_y = 0;
  int out_crop_w = 1920;
  int out_crop_h = 1080;
  int out_stride = 1920;
  int out_endian_a = 0;
  int out_endian = 0;
  int out_plannar = 1;

  /*scl ctrl*/
  int hor_filter_sel = 4;
  int ver_filter_sel = 4;
  int scale_x_init = 9012;
  int scale_y_init = 9012;

  src.format = in_mode;
  src.stride = in_stride;
  src.videosize = ((256 << 16) | 256);
  
  src.addr0 = VPP_SRC_ADDR0;
  src.addr1 = VPP_SRC_ADDR1;
  src.addr2 = VPP_SRC_ADDR2;

  src.axisX = in_axis_x;
  src.axisY = in_axis_y;
  src.cropW = in_crop_w;
  src.cropH = in_crop_h;
  src.endian_a = in_endian_a;
  src.endian = in_endian;
  src.plannar = in_plannar;
  src.tile_mode = tile_mode;


  dst.format = out_mode;
  dst.stride = out_stride;

  dst.addr0 = VPP_DST_ADDR0;
  dst.addr1 = VPP_DST_ADDR1;
  dst.addr2 = VPP_DST_ADDR2;

  dst.axisX = out_axis_x;
  dst.axisY = out_axis_y;
  dst.cropW = out_crop_w;
  dst.cropH = out_crop_h;
  dst.endian_a = out_endian_a;
  dst.endian = out_endian;
  dst.plannar = out_plannar;

  /*scl ctrl*/
  src.scl_ctrl= ((ver_filter_sel & 0xf) << 12) | ((hor_filter_sel & 0xf) << 8) | (scale_enable & 0x1);
  src.scl_init= ((scale_y_init & 0x3fff) << 16) | (scale_x_init & 0x3fff);

  toColor(&src, &dst);

  DataCompare_planar((uint8 *)(CMODEL_DST_ADDR0), (uint8 *)(VPP_DST_ADDR0), dst.axisX, dst.axisY, dst.cropW, dst.cropH, dst.stride,0);

  if (cnt1 != 0) {
  REG(vpp_ret_addr) = 0x1;
  } else {
    REG(vpp_ret_addr) = 0x0;
  }
  printf("test yonly to yonly, up & down scale pass, cnt1 : %d\n", cnt1);
  cnt1 = 0;
}


void test_rgb24_rgb24(void)
{
  struct rect src, dst;
  uint32 vpp_version;
  int scale_enable = 1;   //v3:0   v4:1

  vpp_version = REG(VPP_VERSION);
  printf("vpp version %08x\n", vpp_version);

  /*rgb24 to rgbp*/
  int in_mode = 2;
  int in_axis_x = 0;
  int in_axis_y = 0;
  int in_crop_w = 1920;
  int in_crop_h = 1080;
  int in_stride = 5760;
  int in_endian_a = 0;
  int in_endian = 0;
  int in_plannar = 0;
  int tile_mode = 0;

  int out_mode = 2;
  int out_axis_x = 0;
  int out_axis_y = 0;
  int out_crop_w = 1920;
  int out_crop_h = 1080;
  int out_stride = 5760;
  int out_endian_a = 0;
  int out_endian = 1;
  int out_plannar = 0;

  /*scl ctrl*/
  int hor_filter_sel = 2;
  int ver_filter_sel = 2;
  int scale_x_init = 8361;
  int scale_y_init = 8361;

  src.format = in_mode;
  src.stride = in_stride;
  src.videosize = ((256 << 16) | 256);
  
  src.addr0 = VPP_SRC_ADDR0;
  src.addr1 = VPP_SRC_ADDR1;
  src.addr2 = VPP_SRC_ADDR2;

  src.axisX = in_axis_x;
  src.axisY = in_axis_y;
  src.cropW = in_crop_w;
  src.cropH = in_crop_h;
  src.endian_a = in_endian_a;
  src.endian = in_endian;
  src.plannar = in_plannar;
  src.tile_mode = tile_mode;


  dst.format = out_mode;
  dst.stride = out_stride;

  dst.addr0 = VPP_DST_ADDR0;
  dst.addr1 = VPP_DST_ADDR1;
  dst.addr2 = VPP_DST_ADDR2;

  dst.axisX = out_axis_x;
  dst.axisY = out_axis_y;
  dst.cropW = out_crop_w;
  dst.cropH = out_crop_h;
  dst.endian_a = out_endian_a;
  dst.endian = out_endian;
  dst.plannar = out_plannar;

  /*scl ctrl*/
  src.scl_ctrl= ((ver_filter_sel & 0xf) << 12) | ((hor_filter_sel & 0xf) << 8) | (scale_enable & 0x1);
  src.scl_init= ((scale_y_init & 0x3fff) << 16) | (scale_x_init & 0x3fff);

  toColor(&src, &dst);

  DataCompare_packet24((uint8 *)(CMODEL_DST_ADDR0), (uint8 *)(VPP_DST_ADDR0), dst.axisX, dst.axisY, dst.cropW, dst.cropH, dst.stride,0);

  if (cnt1 != 0) {
  REG(vpp_ret_addr) = 0x1;
  } else {
    REG(vpp_ret_addr) = 0x0;
  }
  printf("test rgb24 to rgb24, up & down scale pass, cnt1 : %d\n", cnt1);
  cnt1 = 0;
}

void test_rgbp_rgb24(void)
{
  struct rect src, dst;
  uint32 vpp_version;
  int scale_enable = 1;   //v3:0   v4:1

  vpp_version = REG(VPP_VERSION);
  printf("vpp version %08x\n", vpp_version);

  /*rgb24 to rgbp*/
  int in_mode = 2;
  int in_axis_x = 0;
  int in_axis_y = 0;
  int in_crop_w = 1920;
  int in_crop_h = 1080;
  int in_stride = 1920;
  int in_endian_a = 0;
  int in_endian = 0;
  int in_plannar = 1;
  int tile_mode = 0;

  int out_mode = 2;
  int out_axis_x = 0;
  int out_axis_y = 0;
  int out_crop_w = 1920;
  int out_crop_h = 1080;
  int out_stride = 5760;
  int out_endian_a = 0;
  int out_endian = 0;
  int out_plannar = 0;

  /*scl ctrl*/
  int hor_filter_sel = 4;
  int ver_filter_sel = 4;
  int scale_x_init = 5928;
  int scale_y_init = 5928;

  src.format = in_mode;
  src.stride = in_stride;
  src.videosize = ((256 << 16) | 256);
  
  src.addr0 = VPP_SRC_ADDR0;
  src.addr1 = VPP_SRC_ADDR1;
  src.addr2 = VPP_SRC_ADDR2;

  src.axisX = in_axis_x;
  src.axisY = in_axis_y;
  src.cropW = in_crop_w;
  src.cropH = in_crop_h;
  src.endian_a = in_endian_a;
  src.endian = in_endian;
  src.plannar = in_plannar;
  src.tile_mode = tile_mode;


  dst.format = out_mode;
  dst.stride = out_stride;

  dst.addr0 = VPP_DST_ADDR0;
  dst.addr1 = VPP_DST_ADDR1;
  dst.addr2 = VPP_DST_ADDR2;

  dst.axisX = out_axis_x;
  dst.axisY = out_axis_y;
  dst.cropW = out_crop_w;
  dst.cropH = out_crop_h;
  dst.endian_a = out_endian_a;
  dst.endian = out_endian;
  dst.plannar = out_plannar;

  /*scl ctrl*/
  src.scl_ctrl= ((ver_filter_sel & 0xf) << 12) | ((hor_filter_sel & 0xf) << 8) | (scale_enable & 0x1);
  src.scl_init= ((scale_y_init & 0x3fff) << 16) | (scale_x_init & 0x3fff);

  toColor(&src, &dst);

  DataCompare_packet24((uint8 *)(CMODEL_DST_ADDR0), (uint8 *)(VPP_DST_ADDR0), dst.axisX, dst.axisY, dst.cropW, dst.cropH, dst.stride,0);

  if (cnt1 != 0) {
  REG(vpp_ret_addr) = 0x1;
  } else {
    REG(vpp_ret_addr) = 0x0;
  }
  printf("test rgbp to rgb24, up & down scale pass, cnt1 : %d\n", cnt1);
  cnt1 = 0;
}

void test_yuv420p_rgb24(void)
{
  struct rect src, dst;
  uint32 vpp_version;
  int scale_enable = 1;   //v3:0   v4:1

  vpp_version = REG(VPP_VERSION);
  printf("vpp version %08x\n", vpp_version);

  /*rgb24 to rgbp*/
  int in_mode = 0;
  int in_axis_x = 0;
  int in_axis_y = 0;
  int in_crop_w = 1920;
  int in_crop_h = 1080;
  int in_stride = 1920;
  int in_endian_a = 0;
  int in_endian = 0;
  int in_plannar = 1;
  int tile_mode = 0;

  int out_mode = 2;
  int out_axis_x = 0;
  int out_axis_y = 0;
  int out_crop_w = 1920;
  int out_crop_h = 1080;
  int out_stride = 5760;
  int out_endian_a = 0;
  int out_endian = 0;
  int out_plannar = 0;

  /*scl ctrl*/
  int hor_filter_sel = 4;
  int ver_filter_sel = 4;
  int scale_x_init = 973;
  int scale_y_init = 973;

  src.format = in_mode;
  src.stride = in_stride;
  src.videosize = ((256 << 16) | 256);
  
  src.addr0 = VPP_SRC_ADDR0;
  src.addr1 = VPP_SRC_ADDR1;
  src.addr2 = VPP_SRC_ADDR2;

  src.axisX = in_axis_x;
  src.axisY = in_axis_y;
  src.cropW = in_crop_w;
  src.cropH = in_crop_h;
  src.endian_a = in_endian_a;
  src.endian = in_endian;
  src.plannar = in_plannar;
  src.tile_mode = tile_mode;


  dst.format = out_mode;
  dst.stride = out_stride;

  dst.addr0 = VPP_DST_ADDR0;
  dst.addr1 = VPP_DST_ADDR1;
  dst.addr2 = VPP_DST_ADDR2;

  dst.axisX = out_axis_x;
  dst.axisY = out_axis_y;
  dst.cropW = out_crop_w;
  dst.cropH = out_crop_h;
  dst.endian_a = out_endian_a;
  dst.endian = out_endian;
  dst.plannar = out_plannar;

  /*scl ctrl*/
  src.scl_ctrl= ((ver_filter_sel & 0xf) << 12) | ((hor_filter_sel & 0xf) << 8) | (scale_enable & 0x1);
  src.scl_init= ((scale_y_init & 0x3fff) << 16) | (scale_x_init & 0x3fff);

  toColor(&src, &dst);

  DataCompare_packet24((uint8 *)(CMODEL_DST_ADDR0), (uint8 *)(VPP_DST_ADDR0), dst.axisX, dst.axisY, dst.cropW, dst.cropH, dst.stride,0);

  if (cnt1 != 0) {
  REG(vpp_ret_addr) = 0x1;
  } else {
    REG(vpp_ret_addr) = 0x0;
  }
  printf("test yuv420p to rgb24, up & down scale pass, cnt1 : %d\n", cnt1);
  cnt1 = 0;
}

void test_yuv420p_rgb32(void)
{
  struct rect src, dst;
  uint32 vpp_version;
  int scale_enable = 1;   //v3:0   v4:1

  vpp_version = REG(VPP_VERSION);
  printf("vpp version %08x\n", vpp_version);

  /*rgb24 to rgbp*/
  int in_mode = 0;
  int in_axis_x = 0;
  int in_axis_y = 0;
  int in_crop_w = 1920;
  int in_crop_h = 1080;
  int in_stride = 1920;
  int in_endian_a = 0;
  int in_endian = 0;
  int in_plannar = 1;
  int tile_mode = 0;

  int out_mode = 3;
  int out_axis_x = 0;
  int out_axis_y = 0;
  int out_crop_w = 1920;
  int out_crop_h = 1080;
  int out_stride = 7680;
  int out_endian_a = 0;
  int out_endian = 0;
  int out_plannar = 0;

  /*scl ctrl*/
  int hor_filter_sel = 0;
  int ver_filter_sel = 0;
  int scale_x_init = 12584;
  int scale_y_init = 12584;

  src.format = in_mode;
  src.stride = in_stride;
  src.videosize = ((256 << 16) | 256);
  
  src.addr0 = VPP_SRC_ADDR0;
  src.addr1 = VPP_SRC_ADDR1;
  src.addr2 = VPP_SRC_ADDR2;

  src.axisX = in_axis_x;
  src.axisY = in_axis_y;
  src.cropW = in_crop_w;
  src.cropH = in_crop_h;
  src.endian_a = in_endian_a;
  src.endian = in_endian;
  src.plannar = in_plannar;
  src.tile_mode = tile_mode;


  dst.format = out_mode;
  dst.stride = out_stride;

  dst.addr0 = VPP_DST_ADDR0;
  dst.addr1 = VPP_DST_ADDR1;
  dst.addr2 = VPP_DST_ADDR2;

  dst.axisX = out_axis_x;
  dst.axisY = out_axis_y;
  dst.cropW = out_crop_w;
  dst.cropH = out_crop_h;
  dst.endian_a = out_endian_a;
  dst.endian = out_endian;
  dst.plannar = out_plannar;

  /*scl ctrl*/
  src.scl_ctrl= ((ver_filter_sel & 0xf) << 12) | ((hor_filter_sel & 0xf) << 8) | (scale_enable & 0x1);
  src.scl_init= ((scale_y_init & 0x3fff) << 16) | (scale_x_init & 0x3fff);

  toColor(&src, &dst);

  DataCompare_packet32((uint8 *)(CMODEL_DST_ADDR0), (uint8 *)(VPP_DST_ADDR0), dst.axisX, dst.axisY, dst.cropW, dst.cropH, dst.stride,0);

  if (cnt1 != 0) {
  REG(vpp_ret_addr) = 0x1;
  } else {
    REG(vpp_ret_addr) = 0x0;
  }
  printf("test yuv420p to rgb32, up & down scale pass, cnt1 : %d\n", cnt1);
  cnt1 = 0;
}


void test_rgbp_rgb32(void)
{
  struct rect src, dst;
  uint32 vpp_version;
  int scale_enable = 1;   //v3:0   v4:1

  vpp_version = REG(VPP_VERSION);
  printf("vpp version %08x\n", vpp_version);

  /*rgb24 to rgbp*/
  int in_mode = 2;
  int in_axis_x = 0;
  int in_axis_y = 0;
  int in_crop_w = 1920;
  int in_crop_h = 1080;
  int in_stride = 1920;
  int in_endian_a = 0;
  int in_endian = 0;
  int in_plannar = 1;
  int tile_mode = 0;

  int out_mode = 3;
  int out_axis_x = 0;
  int out_axis_y = 0;
  int out_crop_w = 1920;
  int out_crop_h = 1080;
  int out_stride = 7680;
  int out_endian_a = 0;
  int out_endian = 0;
  int out_plannar = 0;

  /*scl ctrl*/
  int hor_filter_sel = 2;
  int ver_filter_sel = 2;
  int scale_x_init = 0;
  int scale_y_init = 0;

  src.format = in_mode;
  src.stride = in_stride;
  src.videosize = ((256 << 16) | 256);
  
  src.addr0 = VPP_SRC_ADDR0;
  src.addr1 = VPP_SRC_ADDR1;
  src.addr2 = VPP_SRC_ADDR2;

  src.axisX = in_axis_x;
  src.axisY = in_axis_y;
  src.cropW = in_crop_w;
  src.cropH = in_crop_h;
  src.endian_a = in_endian_a;
  src.endian = in_endian;
  src.plannar = in_plannar;
  src.tile_mode = tile_mode;


  dst.format = out_mode;
  dst.stride = out_stride;

  dst.addr0 = VPP_DST_ADDR0;
  dst.addr1 = VPP_DST_ADDR1;
  dst.addr2 = VPP_DST_ADDR2;

  dst.axisX = out_axis_x;
  dst.axisY = out_axis_y;
  dst.cropW = out_crop_w;
  dst.cropH = out_crop_h;
  dst.endian_a = out_endian_a;
  dst.endian = out_endian;
  dst.plannar = out_plannar;

  /*scl ctrl*/
  src.scl_ctrl= ((ver_filter_sel & 0xf) << 12) | ((hor_filter_sel & 0xf) << 8) | (scale_enable & 0x1);
  src.scl_init= ((scale_y_init & 0x3fff) << 16) | (scale_x_init & 0x3fff);

  toColor(&src, &dst);

  DataCompare_packet32((uint8 *)(CMODEL_DST_ADDR0), (uint8 *)(VPP_DST_ADDR0), dst.axisX, dst.axisY, dst.cropW, dst.cropH, dst.stride,0);

  if (cnt1 != 0) {
  REG(vpp_ret_addr) = 0x1;
  } else {
    REG(vpp_ret_addr) = 0x0;
  }
  printf("test rgbp to rgb32, up & down scale pass, cnt1 : %d\n", cnt1);
  cnt1 = 0;
}

void test_cmdlist_switch_cpuconfig_rgbp_rgb32(void)
{
  struct rect src, dst;
  uint32 vpp_version;
  int scale_enable = 1;   //v3:0   v4:1

  vpp_version = REG(VPP_VERSION);
  printf("vpp version %08x\n", vpp_version);

  /*rgb24 to rgbp*/
  int in_mode = 2;
  int in_axis_x = 0;
  int in_axis_y = 0;
  int in_crop_w = 1920;
  int in_crop_h = 1080;
  int in_stride = 1920;
  int in_endian_a = 0;
  int in_endian = 0;
  int in_plannar = 1;
  int tile_mode = 0;

  int out_mode = 3;
  int out_axis_x = 0;
  int out_axis_y = 0;
  int out_crop_w = 1920;
  int out_crop_h = 1080;
  int out_stride = 7680;
  int out_endian_a = 0;
  int out_endian = 0;
  int out_plannar = 0;

  /*scl ctrl*/
  int hor_filter_sel = 2;
  int ver_filter_sel = 2;
  int scale_x_init = 0;
  int scale_y_init = 0;

  src.format = in_mode;
  src.stride = in_stride;
  src.videosize = ((256 << 16) | 256);
  
  src.addr0 = VPP_SRC_ADDR0;
  src.addr1 = VPP_SRC_ADDR1;
  src.addr2 = VPP_SRC_ADDR2;

  src.axisX = in_axis_x;
  src.axisY = in_axis_y;
  src.cropW = in_crop_w;
  src.cropH = in_crop_h;
  src.endian_a = in_endian_a;
  src.endian = in_endian;
  src.plannar = in_plannar;
  src.tile_mode = tile_mode;


  dst.format = out_mode;
  dst.stride = out_stride;

  dst.addr0 = VPP_DST_ADDR0;
  dst.addr1 = VPP_DST_ADDR1;
  dst.addr2 = VPP_DST_ADDR2;

  dst.axisX = out_axis_x;
  dst.axisY = out_axis_y;
  dst.cropW = out_crop_w;
  dst.cropH = out_crop_h;
  dst.endian_a = out_endian_a;
  dst.endian = out_endian;
  dst.plannar = out_plannar;

  /*scl ctrl*/
  src.scl_ctrl= ((ver_filter_sel & 0xf) << 12) | ((hor_filter_sel & 0xf) << 8) | (scale_enable & 0x1);
  src.scl_init= ((scale_y_init & 0x3fff) << 16) | (scale_x_init & 0x3fff);

  g_uiSwitch = CPU_CONFIG_MODE;
  
  toColor_switch(&src, &dst);

  DataCompare_packet32((uint8 *)(CMODEL_DST_ADDR0), (uint8 *)(VPP_DST_ADDR0), dst.axisX, dst.axisY, dst.cropW, dst.cropH, dst.stride,0);

  g_uiSwitch = CMD_LIST_MODE; 

  dst.addr0 = VPP_DST_ADDR3;
  dst.addr1 = VPP_DST_ADDR4;
  dst.addr2 = VPP_DST_ADDR5;
  
  toColor_switch(&src, &dst);
  
  DataCompare_packet32((uint8 *)(CMODEL_DST_ADDR0), (uint8 *)(VPP_DST_ADDR3), dst.axisX, dst.axisY, dst.cropW, dst.cropH, dst.stride,0);

  if (cnt1 != 0) {
  REG(vpp_ret_addr) = 0x1;
  } else {
    REG(vpp_ret_addr) = 0x0;
  }
  printf("test_cmdlist_switch_cpuconfig_rgbp_rgb32, up & down scale pass, cnt1 : %d\n", cnt1);
  cnt1 = 0;
}



void test_rgb32_rgb24(void)
{
  struct rect src, dst;
  uint32 vpp_version;
  int scale_enable = 1;   //v3:0   v4:1

  vpp_version = REG(VPP_VERSION);
  printf("vpp version %08x\n", vpp_version);

  /*rgb24 to rgbp*/
  int in_mode = 3;
  int in_axis_x = 0;
  int in_axis_y = 0;
  int in_crop_w = 1920;
  int in_crop_h = 1080;
  int in_stride = 7680;
  int in_endian_a = 0;
  int in_endian = 0;
  int in_plannar = 0;
  int tile_mode = 0;

  int out_mode = 2;
  int out_axis_x = 0;
  int out_axis_y = 0;
  int out_crop_w = 1920;
  int out_crop_h = 1080;
  int out_stride = 5760;
  int out_endian_a = 0;
  int out_endian = 0;
  int out_plannar = 0;

  /*scl ctrl*/
  int hor_filter_sel = 2;
  int ver_filter_sel = 2;
  int scale_x_init = 2708;
  int scale_y_init = 2708;

  src.format = in_mode;
  src.stride = in_stride;
  src.videosize = ((256 << 16) | 256);
  
  src.addr0 = VPP_SRC_ADDR0;
  src.addr1 = VPP_SRC_ADDR1;
  src.addr2 = VPP_SRC_ADDR2;

  src.axisX = in_axis_x;
  src.axisY = in_axis_y;
  src.cropW = in_crop_w;
  src.cropH = in_crop_h;
  src.endian_a = in_endian_a;
  src.endian = in_endian;
  src.plannar = in_plannar;
  src.tile_mode = tile_mode;


  dst.format = out_mode;
  dst.stride = out_stride;

  dst.addr0 = VPP_DST_ADDR0;
  dst.addr1 = VPP_DST_ADDR1;
  dst.addr2 = VPP_DST_ADDR2;

  dst.axisX = out_axis_x;
  dst.axisY = out_axis_y;
  dst.cropW = out_crop_w;
  dst.cropH = out_crop_h;
  dst.endian_a = out_endian_a;
  dst.endian = out_endian;
  dst.plannar = out_plannar;

  /*scl ctrl*/
  src.scl_ctrl= ((ver_filter_sel & 0xf) << 12) | ((hor_filter_sel & 0xf) << 8) | (scale_enable & 0x1);
  src.scl_init= ((scale_y_init & 0x3fff) << 16) | (scale_x_init & 0x3fff);

  toColor(&src, &dst);

  DataCompare_packet24((uint8 *)(CMODEL_DST_ADDR0), (uint8 *)(VPP_DST_ADDR0), dst.axisX, dst.axisY, dst.cropW, dst.cropH, dst.stride,0);

  if (cnt1 != 0) {
  REG(vpp_ret_addr) = 0x1;
  } else {
    REG(vpp_ret_addr) = 0x0;
  }
  printf("test rgb32 to rgb24, up & down scale pass, cnt1 : %d\n", cnt1);
  cnt1 = 0;
}

void test_rgb32_rgb32(void)
{
  struct rect src, dst;
  uint32 vpp_version;
  int scale_enable = 1;   //v3:0   v4:1

  vpp_version = REG(VPP_VERSION);
  printf("vpp version %08x\n", vpp_version);

  /*rgb24 to rgbp*/
  int in_mode = 3;
  int in_axis_x = 0;
  int in_axis_y = 0;
  int in_crop_w = 1920;
  int in_crop_h = 1080;
  int in_stride = 7680;
  int in_endian_a = 0;
  int in_endian = 0;
  int in_plannar = 0;
  int tile_mode = 0;

  int out_mode = 3;
  int out_axis_x = 0;
  int out_axis_y = 0;
  int out_crop_w = 1920;
  int out_crop_h = 1080;
  int out_stride = 7680;
  int out_endian_a = 0;
  int out_endian = 0;
  int out_plannar = 0;

  /*scl ctrl*/
  int hor_filter_sel = 2;
  int ver_filter_sel = 2;
  int scale_x_init = 14027;
  int scale_y_init = 14027;

  src.format = in_mode;
  src.stride = in_stride;
  src.videosize = ((256 << 16) | 256);
  
  src.addr0 = VPP_SRC_ADDR0;
  src.addr1 = VPP_SRC_ADDR1;
  src.addr2 = VPP_SRC_ADDR2;

  src.axisX = in_axis_x;
  src.axisY = in_axis_y;
  src.cropW = in_crop_w;
  src.cropH = in_crop_h;
  src.endian_a = in_endian_a;
  src.endian = in_endian;
  src.plannar = in_plannar;
  src.tile_mode = tile_mode;


  dst.format = out_mode;
  dst.stride = out_stride;

  dst.addr0 = VPP_DST_ADDR0;
  dst.addr1 = VPP_DST_ADDR1;
  dst.addr2 = VPP_DST_ADDR2;

  dst.axisX = out_axis_x;
  dst.axisY = out_axis_y;
  dst.cropW = out_crop_w;
  dst.cropH = out_crop_h;
  dst.endian_a = out_endian_a;
  dst.endian = out_endian;
  dst.plannar = out_plannar;

  /*scl ctrl*/
  src.scl_ctrl= ((ver_filter_sel & 0xf) << 12) | ((hor_filter_sel & 0xf) << 8) | (scale_enable & 0x1);
  src.scl_init= ((scale_y_init & 0x3fff) << 16) | (scale_x_init & 0x3fff);

  toColor(&src, &dst);

  DataCompare_packet32((uint8 *)(CMODEL_DST_ADDR0), (uint8 *)(VPP_DST_ADDR0), dst.axisX, dst.axisY, dst.cropW, dst.cropH, dst.stride,0);

  if (cnt1 != 0) {
  REG(vpp_ret_addr) = 0x1;
  } else {
    REG(vpp_ret_addr) = 0x0;
  }
  printf("test rgb32 to rgb32, up & down scale pass, cnt1 : %d\n", cnt1);
  cnt1 = 0;
}



void test_rgb24_rgb32(void)
{
  struct rect src, dst;
  uint32 vpp_version;
  int scale_enable = 1;   //v3:0   v4:1

  vpp_version = REG(VPP_VERSION);
  printf("vpp version %08x\n", vpp_version);

  /*rgb24 to rgbp*/
  int in_mode = 2;
  int in_axis_x = 0;
  int in_axis_y = 0;
  int in_crop_w = 1920;
  int in_crop_h = 1080;
  int in_stride = 7680;
  int in_endian_a = 0;
  int in_endian = 0;
  int in_plannar = 0;
  int tile_mode = 0;

  int out_mode = 3;
  int out_axis_x = 0;
  int out_axis_y = 0;
  int out_crop_w = 1920;
  int out_crop_h = 1080;
  int out_stride = 7680;
  int out_endian_a = 0;
  int out_endian = 0;
  int out_plannar = 0;

  /*scl ctrl*/
  int hor_filter_sel = 0;
  int ver_filter_sel = 0;
  int scale_x_init = 4514;
  int scale_y_init = 4514;

  src.format = in_mode;
  src.stride = in_stride;
  src.videosize = ((256 << 16) | 256);
  
  src.addr0 = VPP_SRC_ADDR0;
  src.addr1 = VPP_SRC_ADDR1;
  src.addr2 = VPP_SRC_ADDR2;

  src.axisX = in_axis_x;
  src.axisY = in_axis_y;
  src.cropW = in_crop_w;
  src.cropH = in_crop_h;
  src.endian_a = in_endian_a;
  src.endian = in_endian;
  src.plannar = in_plannar;
  src.tile_mode = tile_mode;


  dst.format = out_mode;
  dst.stride = out_stride;

  dst.addr0 = VPP_DST_ADDR0;
  dst.addr1 = VPP_DST_ADDR1;
  dst.addr2 = VPP_DST_ADDR2;

  dst.axisX = out_axis_x;
  dst.axisY = out_axis_y;
  dst.cropW = out_crop_w;
  dst.cropH = out_crop_h;
  dst.endian_a = out_endian_a;
  dst.endian = out_endian;
  dst.plannar = out_plannar;

  /*scl ctrl*/
  src.scl_ctrl= ((ver_filter_sel & 0xf) << 12) | ((hor_filter_sel & 0xf) << 8) | (scale_enable & 0x1);
  src.scl_init= ((scale_y_init & 0x3fff) << 16) | (scale_x_init & 0x3fff);

  toColor(&src, &dst);

  DataCompare_packet32((uint8 *)(CMODEL_DST_ADDR0), (uint8 *)(VPP_DST_ADDR0), dst.axisX, dst.axisY, dst.cropW, dst.cropH, dst.stride,0);

  if (cnt1 != 0) {
  REG(vpp_ret_addr) = 0x1;
  } else {
    REG(vpp_ret_addr) = 0x0;
  }
  printf("test rgb24 to rgb32, up & down scale pass, cnt1 : %d\n", cnt1);
  cnt1 = 0;
}

#ifdef SLT_TEST
int testcase_vpp(void)
{
    int ret = 0;

#ifdef VPP_SUPPORT_CMD_LIST_MODE
  REG(VPP_CONTROL0) = 0x00040004;  // cmd list mode, DMA auto fetch linked command list;
#else
  REG(VPP_CONTROL0) = 0x00040000;  //single cmd mode, CPU config;
#endif

#ifdef VPP_INT_TEST
    REG(VPP_INT_EN) = 0x7;  //enable frame_done_int_en and vpp_int_en
#endif

    //test case 1
    if (test_nv12_linear_rgb24())
    {
        printf("vpp test: fail\n");
    }
    else
    {
        printf("vpp test: success\n");
    }
    return ret;
}
module_testcase("1", testcase_vpp);

#else
static int TestVpp(void)
{
#ifdef VPP_INT_TEST
//test_nv12_linear_yuv420p();  //case 28
#else
//	test_nv12_linear_rgb24();      //test case 1
//	test_nv12_linear_rgb32();      //test case 2
//	test_nv12_linear_rgbp();       //case 3
//	test_nv12_linear_yuv420p();  //case 4
//	test_rgb24_rgb24();              //test case 5
//	test_rgb24_rgb32();              //test case 6
//	test_rgb24_rgbp();              //test case 7	
//	test_rgb32_rgb24();              //test case 8
//	test_rgb32_rgb32();              //test case 9
//	test_rgb32_rgbp();              //test case 10
//	test_rgbp_rgb24();              //test case 11
//	test_rgbp_rgb32();              //test case 12
//	test_rgbp_rgbp();              //test case 13
//	test_yonly_yonly();              //test case 14
//	test_yuv420p_rgb24();              //test case 15
//	test_yuv420p_rgb32();              //test case 16
//	test_yuv420p_rgbp();              //test case 17
//	test_yuv420p_yuv420p();              //test case 18

//	test_nv12_tile_rgb24();              //test case 19
//	test_nv12_tile_rgb32();              //test case 20
//	test_nv12_tile_rgbp();              //test case 21
//	test_nv12_tile_yuv420p();              //test case 22
//	test_yonly_tile_yonly();              //test case 23
//	test_scloe_nv12_linear_rgb24();      //test case 24
//	test_resize_nv12_linear_rgb24();      //test case 25
//	test_cmdlist_switch_cpuconfig_rgbp_rgb32();              //test case 26

  test_nv12_tile_rgbp_switch_yuv420p_linear_yuv420p();              //test case 27
#endif

#ifdef VPP_PERFORMANCE_TEST
  test_nv12_tile_rgb24_performance();     //vpp performance test
#endif

  test_vpp_exit();
  return 0;
}

int testcase_vpp(void)
{
  int ret = 0;

#ifdef VPP_SUPPORT_CMD_LIST_MODE
  REG(VPP_CONTROL0) = 0x00040004;  // cmd list mode, DMA auto fetch linked command list;
#else
  REG(VPP_CONTROL0) = 0x00040000;  //single cmd mode, CPU config;
#endif

#ifdef VPP_INT_TEST
    REG(VPP_INT_EN) = 0x7;  //enable frame_done_int_en and vpp_int_en
#endif

    TestVpp();
  return ret;
}
#endif
