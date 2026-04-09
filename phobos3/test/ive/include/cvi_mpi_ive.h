#ifndef CVI_MPI_IVE
#define CVI_MPI_IVE

#include "cvi_ive_reg.h"
#include "cvi_type.h"

#include <fstream>
#include <iostream>
#include <vector>
#include <string.h>
#include <unistd.h>

using namespace std;

// ip address
#define IVE_INTR_NUM            (138)
#define CSIMAC0_INTR_NUM        (155)
#define CSIMAC1_INTR_NUM        (156)
#define IVE_TOP_PHY_REG_BASE    (0x0A000000)
#define DRAM_PHY_BASE           (0x80000000)
#define IVE_BLK_REGS_BITW       (12)
#define IVE_BLK_ID_BITW         (8)
#define IVE_PRERAW_INST_NUM     (2)

#define VREG_SIZE               (sizeof(struct VREG_RESV))
#define ADMA_DESC_SIZE          (sizeof(struct IVECQ_ADMA_DESC_T))

/* IVE REG FIELD DEFINE */

/* IVE BLOCK ADDR OFFSET DEFINE */
//#define IVE_BLK_BA_IVE_TOP 0XA0C_6000
#define IVE_BLK_BA_IVE_TOP              (0X00000000)
#define IVE_BLK_BA_IMG_IN               (0X00000400)
#define IVE_BLK_BA_RDMA_IMG1            (0X00000480)
#define IVE_BLK_BA_MAP                  (0X000004C0)
#define IVE_BLK_BA_HIST                 (0X00000500)
#define IVE_BLK_BA_INTG                 (0X00000580)
#define IVE_BLK_BA_SAD                  (0X00000600)
#define IVE_BLK_BA_SAD_WDMA             (0X00000680)
#define IVE_BLK_BA_SAD_WDMA_THR         (0X00000700)
#define IVE_BLK_BA_NCC                  (0X00000780)
#define IVE_BLK_BA_GMM_MODEL_RDMA_0     (0X00000800)
#define IVE_BLK_BA_GMM_MODEL_RDMA_1     (0X00000840)
#define IVE_BLK_BA_GMM_MODEL_RDMA_2     (0X00000880)
#define IVE_BLK_BA_GMM_MODEL_RDMA_3     (0X000008C0)
#define IVE_BLK_BA_GMM_MODEL_RDMA_4     (0X00000900)
#define IVE_BLK_BA_GMM_MODEL_WDMA_0     (0X00000940)
#define IVE_BLK_BA_GMM_MODEL_WDMA_1     (0X00000980)
#define IVE_BLK_BA_GMM_MODEL_WDMA_2     (0X000009C0)
#define IVE_BLK_BA_GMM_MODEL_WDMA_3     (0X00000A00)
#define IVE_BLK_BA_GMM_MODEL_WDMA_4     (0X00000A40)
#define IVE_BLK_BA_GMM                  (0X00000A80)
#define IVE_BLK_BA_GMM2_MODEL_RDMA_0    (0X00000AC0)
#define IVE_BLK_BA_GMM2_MODEL_RDMA_1    (0X00000B00)
#define IVE_BLK_BA_GMM2_MODEL_RDMA_2    (0X00000B40)
#define IVE_BLK_BA_GMM2_MODEL_RDMA_3    (0X00000B80)
#define IVE_BLK_BA_GMM2_MODEL_RDMA_4    (0X00000BC0)
#define IVE_BLK_BA_GMM2_MODEL_WDMA_0    (0X00000C00)
#define IVE_BLK_BA_GMM2_MODEL_WDMA_1    (0X00000C40)
#define IVE_BLK_BA_GMM2_MODEL_WDMA_2    (0X00000C80)
#define IVE_BLK_BA_GMM2_MODEL_WDMA_3    (0X00000CC0)
#define IVE_BLK_BA_GMM2_MODEL_WDMA_4    (0X00000D00)
#define IVE_BLK_BA_GMM2_MATCH_WDMA      (0X00000D40)
#define IVE_BLK_BA_GMM2                 (0X00000D80)
#define IVE_BLK_BA_BG_MATCH             (0X00000E00)
#define IVE_BLK_BA_BG_UPDATE            (0X00001000)
#define IVE_BLK_BA_FILTEROP             (0X00002000)
#define IVE_BLK_BA_CCL                  (0X00002400)
#define IVE_BLK_BA_CCL_SRC_RDMA         (0X00002440)
#define IVE_BLK_BA_CCL_DST_WDMA         (0X00002480)
#define IVE_BLK_BA_CCL_REGION_WDMA      (0X000024C0)
#define IVE_BLK_BA_DMAF                 (0X00002500)
#define IVE_BLK_BA_LK                   (0X00002600)
#define IVE_BLK_BA_RDMA_EIGVAL          (0X00002700)
#define IVE_BLK_BA_WDMA                 (0X00002740) // (W AXI MASTER)
#define IVE_BLK_BA_RDMA                 (0X00002780) // (R AXI MASTER)
//#define IVE_BLK_BA_WDMA_Y          (IVE_BLK_BA_FILTEROP - 0x1C0)

#define MAP_IVE_BLOCK_ID(_ba)   (((_ba) >> IVE_BLK_REGS_BITW) \
				& ((1 << IVE_BLK_ID_BITW) - 1))

enum IVE_BLK_ID_T {
    IVE_BLK_ID_IVE_TOP              ,//= MAP_IVE_BLOCK_ID(IVE_BLK_BA_IVE_TOP),
    IVE_BLK_ID_IMG_IN               ,//= MAP_IVE_BLOCK_ID(IVE_BLK_BA_IMG_IN),
    IVE_BLK_ID_RDMA_IMG1            ,//= MAP_IVE_BLOCK_ID(IVE_BLK_BA_RDMA_IMG1),
    IVE_BLK_ID_MAP                  ,//= MAP_IVE_BLOCK_ID(IVE_BLK_BA_MAP),
    IVE_BLK_ID_HIST                 ,//= MAP_IVE_BLOCK_ID(IVE_BLK_BA_HIST),
    IVE_BLK_ID_INTG                 ,//= MAP_IVE_BLOCK_ID(IVE_BLK_BA_INTG),
    IVE_BLK_ID_SAD                  ,//= MAP_IVE_BLOCK_ID(IVE_BLK_BA_SAD),
    IVE_BLK_ID_SAD_WDMA             ,//= MAP_IVE_BLOCK_ID(IVE_BLK_BA_SAD_WDMA),
    IVE_BLK_ID_SAD_WDMA_THR         ,//= MAP_IVE_BLOCK_ID(IVE_BLK_BA_SAD_WDMA_THR),
    IVE_BLK_ID_NCC                  ,//= MAP_IVE_BLOCK_ID(IVE_BLK_BA_NCC),
    IVE_BLK_ID_GMM_MODEL_RDMA_0     ,//= MAP_IVE_BLOCK_ID(IVE_BLK_BA_GMM_MODEL_RDMA_0),
    IVE_BLK_ID_GMM_MODEL_RDMA_1     ,//= MAP_IVE_BLOCK_ID(IVE_BLK_BA_GMM_MODEL_RDMA_1),
    IVE_BLK_ID_GMM_MODEL_RDMA_2     ,//= MAP_IVE_BLOCK_ID(IVE_BLK_BA_GMM_MODEL_RDMA_2),
    IVE_BLK_ID_GMM_MODEL_RDMA_3     ,//= MAP_IVE_BLOCK_ID(IVE_BLK_BA_GMM_MODEL_RDMA_3),
    IVE_BLK_ID_GMM_MODEL_RDMA_4     ,//= MAP_IVE_BLOCK_ID(IVE_BLK_BA_GMM_MODEL_RDMA_4),
    IVE_BLK_ID_GMM_MODEL_WDMA_0     ,//= MAP_IVE_BLOCK_ID(IVE_BLK_BA_GMM_MODEL_WDMA_0),
    IVE_BLK_ID_GMM_MODEL_WDMA_1     ,//= MAP_IVE_BLOCK_ID(IVE_BLK_BA_GMM_MODEL_WDMA_1),
    IVE_BLK_ID_GMM_MODEL_WDMA_2     ,//= MAP_IVE_BLOCK_ID(IVE_BLK_BA_GMM_MODEL_WDMA_2),
    IVE_BLK_ID_GMM_MODEL_WDMA_3     ,//= MAP_IVE_BLOCK_ID(IVE_BLK_BA_GMM_MODEL_WDMA_3),
    IVE_BLK_ID_GMM_MODEL_WDMA_4     ,//= MAP_IVE_BLOCK_ID(IVE_BLK_BA_GMM_MODEL_WDMA_4),
    IVE_BLK_ID_GMM                  ,//= MAP_IVE_BLOCK_ID(IVE_BLK_BA_GMM),
    IVE_BLK_ID_GMM2_MODEL_RDMA_0    ,//= MAP_IVE_BLOCK_ID(IVE_BLK_BA_GMM2_MODEL_RDMA_0),
    IVE_BLK_ID_GMM2_MODEL_RDMA_1    ,//= MAP_IVE_BLOCK_ID(IVE_BLK_BA_GMM2_MODEL_RDMA_1),
    IVE_BLK_ID_GMM2_MODEL_RDMA_2    ,//= MAP_IVE_BLOCK_ID(IVE_BLK_BA_GMM2_MODEL_RDMA_2),
    IVE_BLK_ID_GMM2_MODEL_RDMA_3    ,//= MAP_IVE_BLOCK_ID(IVE_BLK_BA_GMM2_MODEL_RDMA_3),
    IVE_BLK_ID_GMM2_MODEL_RDMA_4    ,//= MAP_IVE_BLOCK_ID(IVE_BLK_BA_GMM2_MODEL_RDMA_4),
    IVE_BLK_ID_GMM2_MODEL_WDMA_0    ,//= MAP_IVE_BLOCK_ID(IVE_BLK_BA_GMM2_MODEL_WDMA_0),
    IVE_BLK_ID_GMM2_MODEL_WDMA_1    ,//= MAP_IVE_BLOCK_ID(IVE_BLK_BA_GMM2_MODEL_WDMA_1),
    IVE_BLK_ID_GMM2_MODEL_WDMA_2    ,//= MAP_IVE_BLOCK_ID(IVE_BLK_BA_GMM2_MODEL_WDMA_2),
    IVE_BLK_ID_GMM2_MODEL_WDMA_3    ,//= MAP_IVE_BLOCK_ID(IVE_BLK_BA_GMM2_MODEL_WDMA_3),
    IVE_BLK_ID_GMM2_MODEL_WDMA_4    ,//= MAP_IVE_BLOCK_ID(IVE_BLK_BA_GMM2_MODEL_WDMA_4),
    IVE_BLK_ID_GMM2_MATCH_WDMA      ,//= MAP_IVE_BLOCK_ID(IVE_BLK_BA_GMM2_MATCH_WDMA),
    IVE_BLK_ID_GMM2                 ,//= MAP_IVE_BLOCK_ID(IVE_BLK_BA_GMM2),
    IVE_BLK_ID_BG_MATCH             ,//= MAP_IVE_BLOCK_ID(IVE_BLK_BA_BG_MATCH),
    IVE_BLK_ID_BG_UPDATE            ,//= MAP_IVE_BLOCK_ID(IVE_BLK_BA_BG_UPDATE),
    IVE_BLK_ID_FILTEROP             ,//= MAP_IVE_BLOCK_ID(IVE_BLK_BA_FILTEROP),
    IVE_BLK_ID_CCL                  ,//= MAP_IVE_BLOCK_ID(IVE_BLK_BA_CCL),
    IVE_BLK_ID_CCL_SRC_RDMA         ,//= MAP_IVE_BLOCK_ID(IVE_BLK_BA_CCL_SRC_RDMA),
    IVE_BLK_ID_CCL_DST_WDMA         ,//= MAP_IVE_BLOCK_ID(IVE_BLK_BA_CCL_DST_WDMA),
    IVE_BLK_ID_CCL_REGION_WDMA      ,//= MAP_IVE_BLOCK_ID(IVE_BLK_BA_CCL_REGION_WDMA),
    IVE_BLK_ID_DMAF                 ,//= MAP_IVE_BLOCK_ID(IVE_BLK_BA_DMAF),
    IVE_BLK_ID_LK                   ,//= MAP_IVE_BLOCK_ID(IVE_BLK_BA_LK),
    IVE_BLK_ID_RDMA_EIGVAL          ,//= MAP_IVE_BLOCK_ID(IVE_BLK_BA_RDMA_EIGVAL),
    IVE_BLK_ID_WDMA                 ,//= MAP_IVE_BLOCK_ID(IVE_BLK_BA_WDMA),
    IVE_BLK_ID_RDMA                 ,//= MAP_IVE_BLOCK_ID(IVE_BLK_BA_RDMA),
    IVE_BLK_ID_MAX
};

void setRegBase(uintptr_t _reg_base);
uintptr_t getRegBase(void);
uint64_t *ive_get_phys_reg_bases(void);
void setDmaBase(uintptr_t _dma_base);
uint64_t getDmaBase(void);
void ive_set_base_addr(void *base);
enum IVEModule {
  IVEModuleHIST = 0,
  IVEModuleINTG,
  IVEModuleSAD,
  IVEModuleGMM,
  IVEModuleMATCH_BG,
  IVEModuleBGU,
  IVEModuleFILTEROP,
  IVEModuleCCL,
  IVEModuleDMAF,
  IVEModuleBUFF,
};

class IVEReg {
  public:
    IVEReg(IVE_HANDLE *_pIveHandle, CVI_BOOL _bInstant,
        CVI_U16 _input_h, CVI_U16 _input_w,
        enum IVEModule module = IVEModuleBUFF) {

      pIveHandle = _pIveHandle;
      bInstant = _bInstant;
      input_h = _input_h - 1;
      input_w = _input_w - 1;

      if (!getRegBase()) {
        ive_dma_c = &_ive_dma_c;
        ive_top_c = &_ive_top_c;
            ive_filterop_c = &_ive_filterop_c;
            ive_ccl_c = &_ive_ccl_c;
            ive_gmm_c = &_ive_gmm_c;
            ive_update_bg_c = &_ive_update_bg_c;
            ive_sad_c = &_ive_sad_c;
            ive_match_bg_c = &_ive_match_bg_c;
            ive_map_c = &_ive_map_c;
            isp_dma_ctl_c = &_isp_dma_ctl_c;
            isp_rdma_c = &_isp_rdma_c;
            isp_wdma_c = &_isp_wdma_c;
            img_in_c = &_img_in_c;
            sc_odma_c = &_sc_odma_c;
            ive_intg_c = &_ive_intg_c;
            ive_hist_c = &_ive_hist_c;
        }
        else {
            addrBase = ive_get_phys_reg_bases();
            ive_dma_c = (IVE_DMA_C*) addrBase[IVE_BLK_ID_DMAF];
            ive_top_c = (IVE_TOP_C*) (addrBase[IVE_BLK_ID_IVE_TOP]);
            ive_filterop_c = (IVE_FILTEROP_C*) (addrBase[IVE_BLK_ID_FILTEROP] );
            isp_dma_ctl_c = (ISP_DMA_CTL_C*) (addrBase[IVE_BLK_ID_FILTEROP] );
            ive_ccl_c = (IVE_CCL_C*) addrBase[IVE_BLK_ID_CCL];
            ive_gmm_c = (IVE_GMM_C*) addrBase[IVE_BLK_ID_GMM];
            ive_update_bg_c =
                (IVE_UPDATE_BG_C*) addrBase[IVE_BLK_ID_BG_UPDATE];
            ive_sad_c = (IVE_SAD_C*) addrBase[IVE_BLK_ID_SAD];
            ive_match_bg_c = (IVE_MATCH_BG_C*) addrBase[IVE_BLK_ID_BG_MATCH];
            ive_map_c = (IVE_MAP_C*) addrBase[IVE_BLK_ID_MAP];

            img_in_c = (IMG_IN_C* )addrBase[IVE_BLK_ID_IMG_IN];
            // FIXME: set ODMA
            //sc_odma_c = (SC_ODMA_C* )addrBase[IVE_BLK_ID_WDMA];
            // isp
            //sc_odma_c = ()addrBase[];
            ive_intg_c = (IVE_INTG_C* )addrBase[IVE_BLK_ID_INTG];
            ive_hist_c = (IVE_HIST_C* )addrBase[IVE_BLK_ID_HIST];
            if (module != IVEModuleBUFF) {
              decoder[module](this);
            }
        }

        ive_reset();
        dma_reset();
        // FIXME: init it
        //ive_init();
        
        // init
#define IMAGE_DEPTH (3)
        for (int i = 0; i < IMAGE_DEPTH; i++) {
            s.lhs.au64PhyAddr[i] = 0;
            s.lhs.au64VirAddr[i] = 0;
            s.lhs.au32Stride[i]  = 0;
            s.rhs.au64PhyAddr[i] = 0;
            s.rhs.au64VirAddr[i] = 0;
            s.rhs.au32Stride[i]  = 0;
            s.third.au64PhyAddr[i] = 0;
            s.third.au64VirAddr[i] = 0;
            s.third.au32Stride[i]  = 0;
            for (int j = 0; j < IMAGE_DEPTH; j++) {
                d.o[j].au64PhyAddr[i] = 0;
                d.o[j].au64VirAddr[i] = 0;
                d.o[j].au32Stride[i]  = 0;
            }
        }
    }

    // revert origin input cuz ive ip pass width - 1 to it
    CVI_U32 u32Width() {
        return ive_top_c->REG_0.reg_img_in_widthm1 + 1;
    }

    CVI_U32 u32Height() {
        return ive_top_c->REG_0.reg_img_in_heightm1 + 1;
    }
    CVI_U32 u32InputHeight() {
        return input_h + 1;
    }
    CVI_U32 u32InputWidth() {
        return input_w + 1;
    }

    CVI_VOID dumpBin(const char* shuffix = "") {
        UNUSED(shuffix);
        auto pFile = fopen ("ive_dma.ini","w");
        ive_dma_dump_ini(pFile, ive_dma_c);
        fclose (pFile);
        pFile = fopen ("ive_top.ini","w");
        ive_top_dump_ini(pFile, ive_top_c);
        fclose (pFile);

        pFile = fopen ("ive_filterop.ini","w");
        ive_filterop_dump_ini(pFile, ive_filterop_c);
        fclose (pFile);

        pFile = fopen ("ive_ccl.ini","w");
        ive_ccl_dump_ini(pFile, ive_ccl_c);
        fclose (pFile);

        pFile = fopen ("ive_gmm.ini","w");
        ive_gmm_dump_ini(pFile, ive_gmm_c);
        fclose (pFile);

        pFile = fopen ("ive_update_bg.ini","w");
        ive_update_bg_dump_ini(pFile, ive_update_bg_c);
        fclose (pFile);

        pFile = fopen ("ive_sad.ini","w");
        ive_sad_dump_ini(pFile, ive_sad_c);
        fclose (pFile);

        pFile = fopen ("ive_match_bg.ini","w");
        ive_match_bg_dump_ini(pFile, ive_match_bg_c);
        fclose (pFile);

        pFile = fopen ("ive_map.ini","w");
        ive_map_dump_ini(pFile, ive_map_c);
        fclose (pFile);

        pFile = fopen ("isp_dma_ctl.ini","w");
        isp_dma_ctl_dump_ini(pFile, isp_dma_ctl_c);
        fclose (pFile);

        pFile = fopen ("isp_rdma.ini","w");
        isp_rdma_dump_ini(pFile, isp_rdma_c);
        fclose (pFile);

        pFile = fopen ("isp_wdma.ini","w");
        isp_wdma_dump_ini(pFile, isp_wdma_c);
        fclose (pFile);

        pFile = fopen ("img_in.ini","w");
        img_in_dump_ini(pFile, img_in_c);
        fclose (pFile);

        //pFile = fopen ("sc_odma.ini","w");
        //sc_odma_dump_ini(pFile, sc_odma_c);
        //fclose (pFile);

        pFile = fopen ("ive_intg.ini","w");
        ive_intg_dump_ini(pFile, ive_intg_c);
        fclose (pFile);

        pFile = fopen ("ive_hist.ini","w");
        ive_hist_dump_ini(pFile, ive_hist_c);
        fclose (pFile);

    }

    CVI_VOID ive_init() {
      CVI_ASSERT(0 && "default no need ini register");
#if 0
      DEFINE_IVE_FILTEROP_C(__ive_filterop_c);
      memcpy(ive_filterop_c, &__ive_filterop_c, sizeof(IVE_FILTEROP_C));

      DEFINE_IVE_TOP_C(__ive_top_c);
      memcpy(ive_top_c, &__ive_top_c, sizeof(IVE_TOP_C));

      // pass structure rather than pointer
      DEFINE_IVE_MATCH_BG_C(__ive_match_bg_c);
      memcpy(ive_match_bg_c, &__ive_match_bg_c, sizeof(IVE_MATCH_BG_C));


      DEFINE_IVE_DMA_C(__ive_dma_c);
      memcpy(ive_dma_c, &__ive_dma_c, sizeof(IVE_DMA_C));

      DEFINE_IVE_CCL_C(__ive_ccl_c);
      memcpy(ive_ccl_c, &__ive_ccl_c, sizeof(IVE_CCL_C));

      DEFINE_IVE_GMM_C(__ive_gmm_c);
      memcpy(ive_gmm_c, &__ive_gmm_c, sizeof(IVE_GMM_C));

      DEFINE_IVE_UPDATE_BG_C(__ive_update_bg_c);
      memcpy(ive_update_bg_c, &__ive_update_bg_c, sizeof(IVE_UPDATE_BG_C));

      DEFINE_IVE_SAD_C(__ive_sad_c);
      memcpy(ive_sad_c, &__ive_sad_c, sizeof(IVE_SAD_C));

      DEFINE_IVE_MAP_C(__ive_map_c);
      memcpy(ive_map_c, &__ive_map_c, sizeof(IVE_MAP_C));


      // rdma/wdma
      DEFINE_ISP_DMA_CTL_C(__isp_dma_ctl_c);
      memcpy(isp_dma_ctl_c, &__isp_dma_ctl_c, sizeof(ISP_DMA_CTL_C));

      DEFINE_ISP_RDMA_C(__isp_rdma_c);
      memcpy(isp_rdma_c, &__isp_rdma_c, sizeof(ISP_RDMA_C));

      DEFINE_ISP_WDMA_C(__isp_wdma_c);
      memcpy(isp_wdma_c, &__isp_wdma_c, sizeof(ISP_WDMA_C));

      // img_in/sc_odma/intg/hist/dmaf
      DEFINE_IMG_IN_C(__img_in_c);
      memcpy(img_in_c, &__img_in_c, sizeof(IMG_IN_C));

      //DEFINE_SC_ODMA_C(__sc_odma_c);
      //memcpy(sc_odma_c, &__sc_odma_c, sizeof(SC_ODMA_C));

      DEFINE_IVE_INTG_C(__ive_intg_c);
      memcpy(ive_intg_c, &__ive_intg_c, sizeof(IVE_INTG_C));

      DEFINE_IVE_HIST_C(__ive_hist_c);
      memcpy(ive_hist_c, &__ive_hist_c, sizeof(IVE_HIST_C));
#endif
    }

    CVI_VOID ive_set_int(bool isEnable) {
        ive_top_c->REG_94.reg_intr_en_hist = isEnable;
        ive_top_c->REG_94.reg_intr_en_intg = isEnable;
        ive_top_c->REG_94.reg_intr_en_sad = isEnable;
        ive_top_c->REG_94.reg_intr_en_ncc = isEnable;
        ive_top_c->REG_94.reg_intr_en_filterop_odma = isEnable;
        ive_top_c->REG_94.reg_intr_en_filterop_wdma_y = isEnable;
        ive_top_c->REG_94.reg_intr_en_filterop_wdma_c = isEnable;
        ive_top_c->REG_94.reg_intr_en_dmaf = isEnable;
        ive_top_c->REG_94.reg_intr_en_ccl = isEnable;
        ive_top_c->REG_94.reg_intr_en_lk = isEnable;
    }

    CVI_VOID ive_reset() {
        // disable
        ive_set_int(0);

        // default disable it
        ive_top_c->REG_h10.reg_img_in_top_enable = 0;
        ive_top_c->REG_h10.reg_resize_top_enable = 0;
        ive_top_c->REG_h10.reg_gmm_top_enable = 0;
        ive_top_c->REG_h10.reg_csc_top_enable = 0;
        ive_top_c->REG_h10.reg_rdma_img1_top_enable = 0;
        ive_top_c->REG_h10.reg_bgm_top_enable = 0;
        ive_top_c->REG_h10.reg_bgu_top_enable = 0;
        ive_top_c->REG_h10.reg_r2y4_top_enable = 0;
        ive_top_c->REG_h10.reg_map_top_enable = 0;
        ive_top_c->REG_h10.reg_rdma_eigval_top_enable = 0;
        ive_top_c->REG_h10.reg_thresh_top_enable = 0;
        ive_top_c->REG_h10.reg_hist_top_enable = 0;
        ive_top_c->REG_h10.reg_intg_top_enable = 0;
        ive_top_c->REG_h10.reg_ncc_top_enable = 0;
        ive_top_c->REG_h10.reg_sad_top_enable = 0;
        ive_top_c->REG_h10.reg_filterop_top_enable = 0;
        ive_top_c->REG_h10.reg_dmaf_top_enable = 0;
        ive_top_c->REG_h10.reg_ccl_top_enable = 0;
        ive_top_c->REG_h10.reg_lk_top_enable = 0;

        // enable
        //ive_set_int(1);
    }

    CVI_VOID dma_reset() {
    
    }

    CVI_VOID readBin(const char* shuffix="") {
        UNUSED(shuffix);
        //ifstream input_file("students.data", ios::binary);
        //student_t master[3];
        //input_file.read((char*)&master, sizeof(master));
    }

    CVI_VOID iveDebug(const char* shuffix="before dump") {
      // show ive_interrupt_status
      //printf("skip debug\n");
      //return ;
      printf("ive_interrupt_status:"
          ",reg_intr_status_hist:%d\n"
          ",reg_intr_status_intg:%d\n"
          ",reg_intr_status_sad:%d\n"
          ",reg_intr_status_ncc:%d\n"
          "reg_intr_status_filterop_odma:%d\n"
          "reg_intr_status_filterop_wdma_y:%d\n"
          "reg_intr_status_filterop_wdma_c:%d\n"
          ",reg_intr_status_dmaf:%d\n"
          ",reg_intr_status_ccl:%d\n"
          ",reg_intr_status_lk:%d\n",
          ive_top_c->REG_98.reg_intr_status_hist,
          ive_top_c->REG_98.reg_intr_status_intg,
          ive_top_c->REG_98.reg_intr_status_sad,
          ive_top_c->REG_98.reg_intr_status_ncc,
          ive_top_c->REG_98.reg_intr_status_filterop_odma,
          ive_top_c->REG_98.reg_intr_status_filterop_wdma_y,
          ive_top_c->REG_98.reg_intr_status_filterop_wdma_c,
          ive_top_c->REG_98.reg_intr_status_dmaf,
          ive_top_c->REG_98.reg_intr_status_ccl,
          ive_top_c->REG_98.reg_intr_status_lk);

      //ive_dma_print(ive_dma_c);
      printf("ive_top_c at %p, ive_filterop_c at %p, "
          "isp_dma_ctl_c at %p, img_in_c at %p\n",
          ive_top_c, ive_filterop_c, isp_dma_ctl_c, img_in_c);
      ive_top_print(ive_top_c);
      ive_filterop_print(ive_filterop_c);
      //ive_ccl_print(ive_ccl_c);
      //ive_update_bg_print(ive_update_bg_c);
      //ive_sad_print(ive_sad_c);
      //ive_match_bg_print(ive_match_bg_c);
      //ive_map_print(ive_map_c);
      img_in_print(img_in_c);
      ////sc_odma_print(sc_odma_c);
      //ive_intg_print(ive_intg_c);
      //ive_hist_print(ive_hist_c);

      // hang
      //isp_wdma_print(isp_wdma_c);
      //ive_gmm_print(ive_gmm_c);
      //isp_rdma_print(isp_rdma_c);
      isp_dma_ctl_print(isp_dma_ctl_c);

      printf("%s\n", shuffix);
      //dumpBin();
    }

    CVI_S32 _setDmaCtl(bool isSecondInput = false,
        bool isSecondOutput = false) {
      CVI_ASSERT((isSecondInput || isSecondOutput) && "MUST set input/output in dma_ctl");
      CVI_U64 addr = getDst0Au64PhyAddr(0);
      CVI_U16 stride = getDst0Au32Stride(0);

      if (isSecondInput) {
        ive_top_c->REG_h10.reg_rdma_img1_top_enable = 1;
        addr = getSrcRHSAu64PhyAddr(0);
        stride = getSrcRHSAu32Stride(0);
      }
      else if (isSecondOutput) {
        addr = getDst1Au64PhyAddr(0);
        stride = getDst1Au32Stride(0);
      }

      isp_dma_ctl_c->BASE_ADDR.reg_basel = addr & 0xffffffff;
      isp_dma_ctl_c->SYS_CONTROL.reg_baseh = (addr >> 32) & 0xffffffff;

      isp_dma_ctl_c->SYS_CONTROL.reg_stride_sel = swMode;
      isp_dma_ctl_c->SYS_CONTROL.reg_seglen_sel = swMode;
      isp_dma_ctl_c->SYS_CONTROL.reg_segnum_sel = swMode;

      if (swMode) {
        // set height
        isp_dma_ctl_c->DMA_SEGNUM.reg_segnum = u32Height();
        // set width
        isp_dma_ctl_c->DMA_SEGLEN.reg_seglen = u32Width();
        // set stride
        isp_dma_ctl_c->DMA_STRIDE.reg_stride = stride;
        // FIXME: set U8/S8
      }
      else {
        // hw mode, no need to set height/width/stride
      }

      isp_dma_ctl_c->SYS_CONTROL.reg_base_sel = 1; // sw specify address
      //isp_dma_ctl_c->SYS_CONTROL.reg_sw_disable = 0;
      //isp_wdma_c->IP_DISABLE.reg_ip_disable = 0;
      return CVI_SUCCESS;
    }

    CVI_S32 emitISPWdmaYOnly() {
        return _setDmaCtl(/*isSecondInput =*/false);
    }

    CVI_S32 emitDmaCtlSecondOutput() {
        return _setDmaCtl(/*isSecondInput =*/false, /*isSecondOutput=*/true);
    }

    int getImgFmtSel(IVE_IMAGE_TYPE_E enType) {
        int r = CVI_FAILURE;
        switch (enType) {
            case IVE_IMAGE_TYPE_U8C1:
                r = 5;
                break;
            case IVE_IMAGE_TYPE_U8C3_PLANAR:
                r = 2;
                break;
            case IVE_IMAGE_TYPE_U8C3_PACKAGE:
                r = 3;
                break;
            case IVE_IMAGE_TYPE_YUV420SP:
                r = 8;
                break;
            case IVE_IMAGE_TYPE_YUV422SP:
                r = 0xa;
                break;
            case IVE_IMAGE_TYPE_S16C1:
            case IVE_IMAGE_TYPE_U16C1:
                printf("check setting for S16C1/U16C1\n");
                r = 5;
                break;
            default:
                // TODO: check YUV422SP set to 0xa
                CVI_ASSERT(0 && "not support src type");
                return CVI_FAILURE;
                break;
        }
        return r;
    }

    int setImgIn() {
        // NOTICE: need to first set it
        ive_top_c->REG_h10.reg_img_in_top_enable = 1;

        // "source select 0 : ISP 1 : dewap/map_convert other : DRAM"
        //img_in_c->REG_00.reg_src_sel = 2; // 2 for others: DRMA
        //img_in_c->REG_00.reg_fmt_sel = getImgFmtSel(srcEnType);
        // FIXME: set to 8?
        // Burst len unit is byte, 8 means 128bit
        img_in_c->REG_00.reg_burst_ln = 8;

        img_in_c->REG_03.reg_src_y_pitch = getSrcLHSAu32Stride(0);
        // NOTICE: need to set 1 to valify settings immediantly
        //img_in_c->REG_05.reg_shrd_sel = 0;
        img_in_c->REG_05.reg_shrd_sel = 1;
        img_in_c->REG_02.reg_src_wd = ive_top_c->REG_0.reg_img_in_widthm1;
        img_in_c->REG_02.reg_src_ht = ive_top_c->REG_0.reg_img_in_heightm1;

        img_in_c->REG_00.reg_src_sel = 2; // 2 for others: DRMA
        img_in_c->REG_00.reg_fmt_sel = getImgFmtSel(srcEnType);
        img_in_c->REG_Y_BASE_0.reg_src_y_base_b0 = getSrcLHSAu64PhyAddr(0) & 0xffffffff;
        img_in_c->REG_Y_BASE_1.reg_src_y_base_b1 = (getSrcLHSAu64PhyAddr(0) >> 32) & 0xffffffff;
        switch (srcEnType) {
            case IVE_IMAGE_TYPE_S16C1:
                img_in_c->REG_00.reg_burst_ln = 8;
                break;
            case IVE_IMAGE_TYPE_U8C1:
                break;
            case IVE_IMAGE_TYPE_U8C3_PLANAR:
            case IVE_IMAGE_TYPE_U8C3_PACKAGE:
                img_in_c->REG_04.reg_src_c_pitch = getSrcLHSAu32Stride(0);
                img_in_c->REG_U_BASE_0.reg_src_u_base_b0 = getSrcLHSAu64PhyAddr(1) & 0xffffffff;
                img_in_c->REG_U_BASE_1.reg_src_u_base_b1 = (getSrcLHSAu64PhyAddr(1) >> 32) & 0xffffffff;

                img_in_c->REG_V_BASE_0.reg_src_v_base_b0 = (getSrcLHSAu64PhyAddr(2)) & 0xffffffff;
                img_in_c->REG_V_BASE_1.reg_src_v_base_b1 = (getSrcLHSAu64PhyAddr(2) >> 32) & 0xffffffff;
                break;
            case IVE_IMAGE_TYPE_YUV420SP:
            case IVE_IMAGE_TYPE_YUV422SP:
                img_in_c->REG_04.reg_src_c_pitch = getSrcLHSAu32Stride(0);
                img_in_c->REG_U_BASE_0.reg_src_u_base_b0 = getSrcLHSAu64PhyAddr(1) & 0xffffffff;
                img_in_c->REG_U_BASE_1.reg_src_u_base_b1 = (getSrcLHSAu64PhyAddr(1) >> 32) & 0xffffffff;

                img_in_c->REG_V_BASE_0.reg_src_v_base_b0 = (getSrcLHSAu64PhyAddr(1) + 1) & 0xffffffff;
                img_in_c->REG_V_BASE_1.reg_src_v_base_b1 = ((getSrcLHSAu64PhyAddr(1) + 1)>> 32) & 0xffffffff;
                break;
            default:
                CVI_ASSERT(0 && "not support src type");
                return CVI_FAILURE;
                break;
        }

        // TODO: need to wait reg_frame_done_img_in?
        return CVI_SUCCESS;
    }

    int setRdma() {
        _setDmaCtl(/*isSecondInput =*/true);
        
        // FIXME: need to set reg_ive_rdma_img1_mod_u8 in two input?
        switch (srcEnType) {
            case IVE_IMAGE_TYPE_U8C1:
                ive_top_c->REG_3.reg_ive_rdma_img1_mod_u8 = 1;
                break;
            case IVE_IMAGE_TYPE_S16C1:
            case IVE_IMAGE_TYPE_U16C1:
                // FIXME: check it
                ive_top_c->REG_3.reg_ive_rdma_img1_mod_u8 = 0;
                break;
            default:
                CVI_ASSERT(0 && "not support src type");
                return CVI_FAILURE;
                break;
        }

        // TODO: need to set vld?
        ive_top_c->REG_3.reg_ive_rdma_img1_en = 1;
        ive_top_c->REG_h10.reg_rdma_img1_top_enable = 1;

        // TODO: need to wait reg_frame_done_rdma_img1?
        return CVI_SUCCESS;
    }

    static int decoderIVEModuleFILTEROP(IVEReg* iveReg) {
      iveReg->ive_filterop_c = (IVE_FILTEROP_C*) (iveReg->addrBase[IVE_BLK_ID_FILTEROP] + 0x200);
      iveReg->isp_dma_ctl_c = (ISP_DMA_CTL_C*) (iveReg->addrBase[IVE_BLK_ID_FILTEROP] + 0x40);
      return CVI_SUCCESS;
    }

    static int decoderIVEModuleHIST(IVEReg* iveReg) {
      CVI_ASSERT(0 && "not implemented yet");
      UNUSED(iveReg);
      return CVI_SUCCESS;
    }
    static int decoderIVEModuleINTG(IVEReg* iveReg) {
      CVI_ASSERT(0 && "not implemented yet");
      UNUSED(iveReg);
      return CVI_SUCCESS;
    }
    static int decoderIVEModuleSAD(IVEReg* iveReg) {
      CVI_ASSERT(0 && "not implemented yet");
      UNUSED(iveReg);
      return CVI_SUCCESS;
    }
    static int decoderIVEModuleGMM(IVEReg* iveReg) {
      CVI_ASSERT(0 && "not implemented yet");
      UNUSED(iveReg);
      return CVI_SUCCESS;
    }
    static int decoderIVEModuleMATCH_BG(IVEReg* iveReg) {
      CVI_ASSERT(0 && "not implemented yet");
      UNUSED(iveReg);
      return CVI_SUCCESS;
    }
    static int decoderIVEModuleBGU(IVEReg* iveReg) {
      CVI_ASSERT(0 && "not implemented yet");
      UNUSED(iveReg);
      return CVI_SUCCESS;
    }
    static int decoderIVEModuleCCL(IVEReg* iveReg) {
      CVI_ASSERT(0 && "not implemented yet");
      UNUSED(iveReg);
      return CVI_SUCCESS;
    }
    static int decoderIVEModuleDMAF(IVEReg* iveReg) {
      CVI_ASSERT(0 && "not implemented yet");
      UNUSED(iveReg);
      return CVI_SUCCESS;
    }


    int (*decoder[IVEModuleBUFF])(IVEReg*) = {
      decoderIVEModuleHIST,
      decoderIVEModuleINTG,
      decoderIVEModuleSAD,
      decoderIVEModuleGMM,
      decoderIVEModuleMATCH_BG,
      decoderIVEModuleBGU,
      decoderIVEModuleFILTEROP,
      decoderIVEModuleCCL,
      decoderIVEModuleDMAF,
    };

    int (*emitInputOp)(IVEReg* ) = NULL;
    int (*emitOp)(IVEReg* ) = NULL;
    int (*emitDMA)(IVEReg*) = NULL;
    int emitOdma() {
        ive_filterop_c->ODMA_REG_00.reg_fmt_sel = getImgFmtSel(srcEnType);
        switch (dstEnType) {
            case IVE_IMAGE_TYPE_U8C1:
                // two inputs case
                break;
            case IVE_IMAGE_TYPE_YUV420SP:
                // NV21
            case IVE_IMAGE_TYPE_YUV422SP:
                ive_filterop_c->ODMA_REG_03.reg_dma_u_base_low_part =
                    getDst0Au64PhyAddr(1) & 0xffffffff;
                ive_filterop_c->ODMA_REG_04.reg_dma_u_base_high_part = 
                    (getDst0Au64PhyAddr(1) >> 32) & 0xffffffff;
                ive_filterop_c->ODMA_REG_05.reg_dma_v_base_low_part = 
                    (getDst0Au64PhyAddr(1) + 1) & 0xffffffff;
                ive_filterop_c->ODMA_REG_06.reg_dma_v_base_high_part = 
                    ((getDst0Au64PhyAddr(1) + 1) >> 32) & 0xffffffff;
                ive_filterop_c->ODMA_REG_08.reg_dma_c_pitch = getDst0Au32Stride(0);
                break;
            case IVE_IMAGE_TYPE_U8C3_PLANAR:
            case IVE_IMAGE_TYPE_U8C3_PACKAGE:
                ive_filterop_c->ODMA_REG_03.reg_dma_u_base_low_part =
                    getDst0Au64PhyAddr(1) & 0xffffffff;
                ive_filterop_c->ODMA_REG_04.reg_dma_u_base_high_part = 
                    (getDst0Au64PhyAddr(1) >> 32) & 0xffffffff;
                ive_filterop_c->ODMA_REG_05.reg_dma_v_base_low_part = 
                    (getDst0Au64PhyAddr(2)) & 0xffffffff;
                ive_filterop_c->ODMA_REG_06.reg_dma_v_base_high_part = 
                    ((getDst0Au64PhyAddr(2)) >> 32) & 0xffffffff;
                ive_filterop_c->ODMA_REG_08.reg_dma_c_pitch = getDst0Au32Stride(0);
                break;
            default:
                CVI_ASSERT(0 && "not support dstEnType");
                return CVI_FAILURE;
        }

        // odma for filter3ch + csc
        ive_filterop_c->ODMA_REG_01.reg_dma_y_base_low_part =
            getDst0Au64PhyAddr(0) & 0xffffffff;
        ive_filterop_c->ODMA_REG_02.reg_dma_y_base_high_part = 
            (getDst0Au64PhyAddr(0) >> 32) & 0xffffffff;

        ive_filterop_c->ODMA_REG_07.reg_dma_y_pitch = getDst0Au32Stride(0);
        ive_filterop_c->ODMA_REG_11.reg_dma_wd = ive_top_c->REG_0.reg_img_in_widthm1;
        ive_filterop_c->ODMA_REG_12.reg_dma_ht = ive_top_c->REG_0.reg_img_in_heightm1;

        // trigger odma
        ive_filterop_c->ODMA_REG_00.reg_dma_blen = 1;
        ive_filterop_c->ODMA_REG_00.reg_dma_en = 1;

        // enable it
        ive_filterop_c->REG_h14.reg_op_y_wdma_en = 1;

        return CVI_SUCCESS;
    }

    CVI_S32 emit() {
        CVI_S32 ret = CVI_SUCCESS;

        //iveDebug();

        CVI_ASSERT(emitInputOp && "MUST assign emitInputOp");
        CVI_ASSERT(emitOp && "MUST assign emitOp");
        CVI_ASSERT(emitDMA && "MUST assign emitDMA");

        ret = emitInputOp(this);
        printf("emitInputOp\n");

        if (ret != CVI_SUCCESS) {
            CVI_ASSERT(ret == CVI_SUCCESS && "emit input op fail");
            printf("emit input op fail\n");
            return CVI_FAILURE;
        }

        ret = emitOp(this);
        printf("emitOp\n");

        if (ret != CVI_SUCCESS) {
        CVI_ASSERT(ret == CVI_SUCCESS && "emit op fail");
            printf("emit op fail\n");
            return CVI_FAILURE;
        }

        ret = emitDMA(this);
        printf("emitDMA\n");

        return ret;
    }

    IVE_DMA_C _ive_dma_c;
    IVE_TOP_C _ive_top_c;
    IVE_FILTEROP_C _ive_filterop_c;
    IVE_CCL_C _ive_ccl_c;
    IVE_GMM_C _ive_gmm_c;
    IVE_UPDATE_BG_C _ive_update_bg_c;
    IVE_SAD_C _ive_sad_c;
    IVE_MATCH_BG_C _ive_match_bg_c;
    IVE_MAP_C _ive_map_c;

    IVE_DMA_C* ive_dma_c;
    IVE_TOP_C* ive_top_c;
    IVE_FILTEROP_C* ive_filterop_c;
    IVE_CCL_C *ive_ccl_c;
    IVE_GMM_C *ive_gmm_c;
    IVE_UPDATE_BG_C *ive_update_bg_c;
    IVE_SAD_C *ive_sad_c;
    IVE_MATCH_BG_C *ive_match_bg_c;
    IVE_MAP_C *ive_map_c;

    // use rdma/wdma (one channel)
    ISP_DMA_CTL_C _isp_dma_ctl_c;
    ISP_RDMA_C _isp_rdma_c;
    ISP_WDMA_C _isp_wdma_c;

    ISP_DMA_CTL_C* isp_dma_ctl_c;
    ISP_RDMA_C* isp_rdma_c;
    ISP_WDMA_C* isp_wdma_c;

    // img_in/sc_odma/intg/hist/dmaf
    IMG_IN_C _img_in_c;
    SC_ODMA_C _sc_odma_c;
    IVE_INTG_C _ive_intg_c;
    IVE_HIST_C _ive_hist_c;

    IMG_IN_C* img_in_c;
    SC_ODMA_C* sc_odma_c;
    IVE_INTG_C* ive_intg_c;
    IVE_HIST_C* ive_hist_c;

    // FIXME: set register
    struct AddrInfo {
        CVI_U64  au64PhyAddr[IMAGE_DEPTH];
        CVI_U64  au64VirAddr[IMAGE_DEPTH];
        CVI_U32  au32Stride[IMAGE_DEPTH];
    };
    struct {
        struct AddrInfo lhs;
        struct AddrInfo rhs;
        struct AddrInfo third;
    } s; // src
    struct {
        AddrInfo o[IMAGE_DEPTH];
    } d; // dst

    void setDMA(IVE_SRC_IMAGE_S* lhs, IVE_DST_IMAGE_S *out0,
            IVE_SRC_IMAGE_S* rhs = NULL, IVE_DST_IMAGE_S *out1 = NULL,
            IVE_SRC_IMAGE_S* third = NULL, IVE_DST_IMAGE_S *out2 = NULL) {
        

        for (int i = 0; i < IMAGE_DEPTH; i++) {
            // init at constructer
            #pragma GCC diagnostic push
            #pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
            #pragma GCC diagnostic ignored "-Wuninitialized"
            if (lhs) {
                s.lhs.au64PhyAddr[i] = lhs->au64PhyAddr[i];
                s.lhs.au64VirAddr[i] = lhs->au64VirAddr[i];
                s.lhs.au32Stride[i]  = lhs->au32Stride[i];
            }
            if (out0) {
                d.o[0].au64PhyAddr[i] =out0->au64PhyAddr[i];
                d.o[0].au64VirAddr[i] =out0->au64VirAddr[i];
                d.o[0].au32Stride[i]  =out0->au32Stride[i];
            }
            if (rhs) {
                s.rhs.au64PhyAddr[i] = rhs->au64PhyAddr[i];
                s.rhs.au64VirAddr[i] = rhs->au64VirAddr[i];
                s.rhs.au32Stride[i]  = rhs->au32Stride[i];
            }
            if (out1) {
                d.o[1].au64PhyAddr[i] = out1->au64PhyAddr[i];
                d.o[1].au64VirAddr[i] = out1->au64VirAddr[i];
                d.o[1].au32Stride[i]  = out1->au32Stride[i];
            }
            if (third) {
                s.third.au64PhyAddr[i] = third->au64PhyAddr[i];
                s.third.au64VirAddr[i] = third->au64VirAddr[i];
                s.third.au32Stride[i]  = third->au32Stride[i];
            }
            if (out2) {
                d.o[2].au64PhyAddr[i] = out2->au64PhyAddr[i];
                d.o[2].au64VirAddr[i] = out2->au64VirAddr[i];
                d.o[2].au32Stride[i]  = out2->au32Stride[i];
            }
            // hw limitation check align 16B
            if (0) {
                // FIXME: check all ops
                CVI_ASSERT(s.lhs.au32Stride[i] % 16 == 0 && "must align 16B");
                CVI_ASSERT(d.o[0].au32Stride[i] % 16 == 0 && "must align 16B");
                CVI_ASSERT(s.rhs.au32Stride[i] % 16 == 0 && "must align 16B");
                CVI_ASSERT(d.o[1].au32Stride[i] % 16 == 0 && "must align 16B");
                CVI_ASSERT(s.third.au32Stride[i] % 16 == 0 && "must align 16B");
                CVI_ASSERT(d.o[2].au32Stride[i] % 16 == 0 && "must align 16B");
            }
            #pragma GCC diagnostic pop
        }
    }

    CVI_U64 getSrcLHSAu64PhyAddr(int idx) {
        return s.lhs.au64PhyAddr[idx];
    }
    CVI_U64 getSrcLHSAu64VirAddr(int idx) {
        return s.lhs.au64VirAddr[idx];
    }
    CVI_U32 getSrcLHSAu32Stride(int idx) {
        return s.lhs.au32Stride[idx];
    }
    CVI_U64 getSrcRHSAu64PhyAddr(int idx) {
        return s.rhs.au64PhyAddr[idx];
    }
    CVI_U64 getSrcRHSAu64VirAddr(int idx) {
        return s.rhs.au64VirAddr[idx];
    }
    CVI_U32 getSrcRHSAu32Stride(int idx) {
        return s.rhs.au32Stride[idx];
    }
    CVI_U64 getSrcThirdAu64PhyAddr(int idx) {
        return s.third.au64PhyAddr[idx];
    }
    CVI_U64 getSrcThirdAu64VirAddr(int idx) {
        return s.third.au64VirAddr[idx];
    }
    CVI_U32 getSrcThirdAu32Stride(int idx) {
        return s.third.au32Stride[idx];
    }

    CVI_U64 getDst0Au64PhyAddr(int idx) {
        return d.o[0].au64PhyAddr[idx];
    }
    CVI_U64 getDst0Au64VirAddr(int idx) {
        return d.o[0].au64VirAddr[idx];
    }
    CVI_U32 getDst0Au32Stride(int idx) {
        return d.o[0].au32Stride[idx];
    }
    CVI_U64 getDst1Au64PhyAddr(int idx) {
        return d.o[1].au64PhyAddr[idx];
    }
    CVI_U64 getDst1Au64VirAddr(int idx) {
        return d.o[1].au64VirAddr[idx];
    }
    CVI_U32 getDst1Au32Stride(int idx) {
        return d.o[1].au32Stride[idx];
    }
    CVI_U64 getDst2Au64PhyAddr(int idx) {
        return d.o[2].au64PhyAddr[idx];
    }
    CVI_U64 getDst2Au64VirAddr(int idx) {
        return d.o[2].au64VirAddr[idx];
    }
    CVI_U32 getDst2Au32Stride(int idx) {
        return d.o[2].au32Stride[idx];
    }

    IVE_IMAGE_TYPE_E srcEnType = IVE_IMAGE_TYPE_BUTT;
    IVE_IMAGE_TYPE_E dstEnType = IVE_IMAGE_TYPE_BUTT;
    CVI_U16 input_w;
    CVI_U16 input_h;


    CVI_VOID setEnableFilterop(bool status = true) {
        enableFilterop = status;
    }
    CVI_VOID setEnableTop(bool status = true) {
        enableTop = status;
    }
    CVI_VOID setEnableDma(bool status = true) {
        enableDma = status;
    }

    CVI_S32 setSrcDMAAddr(int srcIdx0, enum IVE_BLK_ID_T dstBLKID0) {
        CVI_U64 phyAddr = ive_get_phys_reg_bases()[dstBLKID0];
        CVI_U64 im = -1;
        if (dstBLKID0 == IVE_BLK_ID_IMG_IN) {
            // first input
            im = getSrcLHSAu64PhyAddr(srcIdx0);
        }
        else if (dstBLKID0 == IVE_BLK_ID_RDMA_IMG1){
            // second input, such like add, sub, etc.
            im = getSrcRHSAu64PhyAddr(srcIdx0);
        }
        else {
            CVI_ASSERT(0 && "not support dstBLKID0");
            return CVI_FAILURE;
        }
        *((CVI_U64*)phyAddr) = *((CVI_U64*)im);
        return CVI_SUCCESS;
    }

	IVE_HANDLE *pIveHandle;
    CVI_BOOL bInstant;
    int ctrl;
    uint64_t* addrBase = NULL;
private:
    bool swMode = 0;
    bool enableFilterop;
    bool enableTop;
    bool enableDma;

    // map
    //uint64_t    bayer_r_addr[2];
    //uint64_t    bayer_w_addr[2];
    //uint64_t    yuv_addr[2];
    //uint64_t    tdnr_addr[2];
    //uint64_t    rgbmap_addr[2];
    //uint64_t    lmap_addr[2];
    //uint64_t    manr_addr;
    //uint64_t    lsc_addr[4];
    //uint64_t    sts_addr[2];
    //uint64_t    dci_addr;
    //uint64_t    tdnr_rtile_addr[2];
    //uint64_t    yuv422_addr[3];

};


CVI_S32 CVI_MPI_IVE_DMA(IVEReg* iveReg);
CVI_S32 CVI_MPI_IVE_Filter(IVEReg* iveReg);
CVI_S32 CVI_MPI_IVE_CSC(IVEReg* iveReg);
CVI_S32 CVI_MPI_IVE_FilterAndCSC(IVEReg* iveReg);
CVI_S32 CVI_MPI_IVE_Sobel(IVEReg* iveReg);
CVI_S32 CVI_MPI_IVE_MagAndAng(IVEReg* iveReg);
CVI_S32 CVI_MPI_IVE_Dilate(IVEReg* iveReg);
CVI_S32 CVI_MPI_IVE_Erode(IVEReg* iveReg);
CVI_S32 CVI_MPI_IVE_Thresh(IVEReg* iveReg);
CVI_S32 CVI_MPI_IVE_And(IVEReg* iveReg);
CVI_S32 CVI_MPI_IVE_Sub(IVEReg* iveReg);
CVI_S32 CVI_MPI_IVE_Or(IVEReg* iveReg);
CVI_S32 CVI_MPI_IVE_Integ(IVEReg* iveReg);
CVI_S32 CVI_MPI_IVE_Hist(IVEReg* iveReg);
CVI_S32 CVI_MPI_IVE_EqualizeHist(IVEReg* iveReg);
CVI_S32 CVI_MPI_IVE_Thresh_S16(IVEReg* iveReg);
CVI_S32 CVI_MPI_IVE_Thresh_U16(IVEReg* iveReg);
CVI_S32 CVI_MPI_IVE_16BitTo8Bit(IVEReg* iveReg);
CVI_S32 CVI_MPI_IVE_OrdStatFilter(IVEReg* iveReg);
CVI_S32 CVI_MPI_IVE_Map(IVEReg* iveReg);
CVI_S32 CVI_MPI_IVE_DMA(IVEReg* iveReg);
CVI_S32 CVI_MPI_IVE_Add(IVEReg* iveReg);
CVI_S32 CVI_MPI_IVE_Xor(IVEReg* iveReg);
CVI_S32 CVI_MPI_IVE_NCC(IVEReg* iveReg);
CVI_S32 CVI_MPI_IVE_CCL(IVEReg* iveReg);
CVI_S32 CVI_MPI_IVE_GMM(IVEReg* iveReg);
CVI_S32 CVI_MPI_IVE_GMM2(IVEReg* iveReg);
CVI_S32 CVI_MPI_IVE_CannyHysEdge(IVEReg* iveReg);
CVI_S32 CVI_MPI_IVE_CannyEdge(IVEReg* iveReg);
CVI_S32 CVI_MPI_IVE_LBP(IVEReg* iveReg);
CVI_S32 CVI_MPI_IVE_NormGrad(IVEReg* iveReg);
CVI_S32 CVI_MPI_IVE_NormGrad(IVEReg* iveReg);
CVI_S32 CVI_MPI_IVE_LKOpticalFlowPyr(IVEReg* iveReg);
CVI_S32 CVI_MPI_IVE_STCandiCorner(IVEReg* iveReg);
CVI_S32 CVI_MPI_IVE_STCorner(IVEReg* iveReg);
CVI_S32 CVI_MPI_IVE_GradFg(IVEReg* iveReg);
CVI_S32 CVI_MPI_IVE_MatchBgModel(IVEReg* iveReg);
CVI_S32 CVI_MPI_IVE_UpdateBgModel(IVEReg* iveReg);
CVI_S32 CVI_MPI_IVE_ANN_MLP_LoadModel(IVEReg* iveReg);
CVI_VOID CVI_MPI_IVE_ANN_MLP_UnloadModel(IVEReg* iveReg);
CVI_S32 CVI_MPI_IVE_ANN_MLP_Predict(IVEReg* iveReg);
CVI_S32 CVI_MPI_IVE_SVM_LoadModel(IVEReg* iveReg);
CVI_VOID CVI_MPI_IVE_SVM_UnloadModel(IVEReg* iveReg);
CVI_S32 CVI_MPI_IVE_SVM_Predict(IVEReg* iveReg);
CVI_S32 CVI_MPI_IVE_SAD(IVEReg* iveReg);
CVI_S32 CVI_MPI_IVE_Resize(IVEReg* iveReg);
CVI_S32 CVI_MPI_IVE_CNN_LoadModel(IVEReg* iveReg);
CVI_VOID CVI_MPI_IVE_CNN_UnloadModel(IVEReg* iveReg);
CVI_S32 CVI_MPI_IVE_CNN_Predict(IVEReg* iveReg);
CVI_S32 CVI_MPI_IVE_CNN_GetResult(IVEReg* iveReg);
CVI_S32 CVI_MPI_IVE_Query(IVEReg* iveReg);
CVI_S32 CVI_MPI_IVE_Bernsen(IVEReg* iveReg);
CVI_S32 CVI_MPI_IVE_imgInToOdma(IVEReg* iveReg);
CVI_S32 CVI_MPI_IVE_rgbPToYuvToErodeToDilate(IVEReg* iveReg);
#endif /* ifndef CVI_MPI_IVE */
