#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cvi_ive_hw.h"
#include "cvi_mpi_ive.h"

#include <fstream>
#include <iostream>
#include <vector>
#include <string.h>

using namespace std;

CVI_S32 CVI_MPI_IVE_DMA(IVEReg* iveReg)
{
    // emit op
    iveReg->emitOp = [](IVEReg* iveReg) -> int {
        //iveReg->setSrcDMAAddr(/*srcIdx=*/0, IVE_BLK_ID_IMG_IN);
        iveReg->ive_top_c->REG_h10.reg_dmaf_top_enable = 1;
        iveReg->ive_top_c->REG_94.reg_intr_en_dmaf = !iveReg->bInstant;
        // sw trigger
        iveReg->ive_top_c->REG_1.reg_fmt_vld_dmaf = 1;

        return CVI_SUCCESS;
    };

    iveReg->emitDMA = [](IVEReg* iveReg) -> int {
        iveReg->emitISPWdmaYOnly();

        // wait dma done
        if (iveReg->bInstant) {
            // busy wait
            while(!iveReg->ive_top_c->REG_90.reg_frame_done_dmaf);
        }
        else {
            // schedule
            while (!iveReg->ive_top_c->REG_98.reg_intr_status_dmaf);
        }

        return CVI_SUCCESS;
    };

    return iveReg->emit();
}

CVI_S32 CVI_MPI_IVE_Add(IVEReg* iveReg)
{
    // in: U8C1,U8C1 out: U8C1
    // FIXME: check filter 1 channel setting

    // set RDMA
    iveReg->emitInputOp= [](IVEReg* iveReg) -> int {
      // first input
      iveReg->setImgIn();

      // second input
      iveReg->setRdma();

      return CVI_SUCCESS;
    };

    // emit op
    iveReg->emitOp = [](IVEReg* iveReg) -> int {
        // iveReg->bInstant == true means busy wait
        iveReg->ive_top_c->REG_94.reg_intr_en_filterop_odma = !iveReg->bInstant;
        iveReg->ive_top_c->REG_94.reg_intr_en_filterop_wdma_y = !iveReg->bInstant;
        iveReg->ive_top_c->REG_94.reg_intr_en_filterop_wdma_c = !iveReg->bInstant;

        iveReg->ive_top_c->REG_h10.reg_filterop_top_enable = 1;

        return CVI_SUCCESS;
    };

    // filter op done, copy result to DDR, control flow:
    // wait filterop done -> wait dma(1/3 channels)
    // dma default is hw setting

    iveReg->emitDMA = [](IVEReg* iveReg) -> int {
      iveReg->emitOdma();

      // trigger filterop
      iveReg->ive_top_c->REG_1.reg_fmt_vld_fg = 1;
      
      if (iveReg->bInstant) {
        while(!iveReg->ive_top_c->REG_90.reg_frame_done_filterop_wdma_y);

      }
      else {
        // schedule
        while (!iveReg->ive_top_c->REG_98.reg_intr_status_filterop_odma
            && !iveReg->ive_top_c->REG_98.reg_intr_status_filterop_wdma_y
            && !iveReg->ive_top_c->REG_98.reg_intr_status_filterop_wdma_c);
      }

      return CVI_SUCCESS;
    };

    return iveReg->emit();

}
//
CVI_S32 CVI_MPI_IVE_And(IVEReg* iveReg)
{
  // in:U8C1,U8C1, out: U8C1
  // FIXME: check xxx_top_enable
  // set RDMA
  iveReg->emitInputOp= [](IVEReg* iveReg) -> int {
    iveReg->setImgIn();
    iveReg->setRdma();
    return CVI_SUCCESS;
  };

  // emit op
  iveReg->emitOp = [](IVEReg* iveReg) -> int {
    // iveReg->bInstant == true means busy wait
    iveReg->ive_top_c->REG_94.reg_intr_en_filterop_odma = !iveReg->bInstant;
    iveReg->ive_top_c->REG_94.reg_intr_en_filterop_wdma_y = !iveReg->bInstant;
    iveReg->ive_top_c->REG_94.reg_intr_en_filterop_wdma_c = !iveReg->bInstant;

    iveReg->ive_top_c->REG_h10.reg_filterop_top_enable = 1;
    return CVI_SUCCESS;
  };

  // filter op done, copy result to DDR, control flow:
  // wait filterop done -> wait dma(1/3 channels)
  // dma default is hw setting

  iveReg->emitDMA = [](IVEReg* iveReg) -> int {
    iveReg->emitOdma();

    // trigger
    iveReg->ive_top_c->REG_1.reg_fmt_vld_fg = 1;

    if (iveReg->bInstant) {
      // busy wait, for ONLY one channel output
      while(!iveReg->ive_top_c->REG_90.reg_frame_done_filterop_wdma_y);
    }
    else {
      // schedule
      while (!iveReg->ive_top_c->REG_98.reg_intr_status_filterop_odma
          && !iveReg->ive_top_c->REG_98.reg_intr_status_filterop_wdma_y
          && !iveReg->ive_top_c->REG_98.reg_intr_status_filterop_wdma_c);
    }
    return CVI_SUCCESS;
  };

  return iveReg->emit();
}
//
CVI_S32 CVI_MPI_IVE_Or(IVEReg* iveReg)
{
    // in:U8C1,U8C1, out: U8C1
    // FIXME: check is could reuse CVI_MPI_IVE_And
    return CVI_MPI_IVE_And(iveReg);
}
//
CVI_S32 CVI_MPI_IVE_Sub(IVEReg* iveReg)
{
    // in: U8C1,U8C1 out: U8C1
    // FIXME: check filter 1 channel setting

    // set RDMA
    iveReg->emitInputOp= [](IVEReg* iveReg) -> int {
      // first input
      iveReg->setImgIn();

      // second input
      iveReg->setRdma();

      return CVI_SUCCESS;
    };

    // emit op
    iveReg->emitOp = [](IVEReg* iveReg) -> int {
        // iveReg->bInstant == true means busy wait
        iveReg->ive_top_c->REG_94.reg_intr_en_filterop_odma = !iveReg->bInstant;
        iveReg->ive_top_c->REG_94.reg_intr_en_filterop_wdma_y = !iveReg->bInstant;
        iveReg->ive_top_c->REG_94.reg_intr_en_filterop_wdma_c = !iveReg->bInstant;

        iveReg->ive_top_c->REG_h10.reg_filterop_top_enable = 1;

        return CVI_SUCCESS;
    };

    // filter op done, copy result to DDR, control flow:
    // wait filterop done -> wait dma(1/3 channels)
    // dma default is hw setting

    iveReg->emitDMA = [](IVEReg* iveReg) -> int {
      iveReg->emitOdma();

      // trigger filterop
      iveReg->ive_top_c->REG_1.reg_fmt_vld_fg = 1;
      
      if (iveReg->bInstant) {
        while(!iveReg->ive_top_c->REG_90.reg_frame_done_filterop_wdma_y);

      }
      else {
        // schedule
        while (!iveReg->ive_top_c->REG_98.reg_intr_status_filterop_odma
            && !iveReg->ive_top_c->REG_98.reg_intr_status_filterop_wdma_y
            && !iveReg->ive_top_c->REG_98.reg_intr_status_filterop_wdma_c);
      }

      return CVI_SUCCESS;
    };

    return iveReg->emit();

}


CVI_S32 CVI_MPI_IVE_Xor(IVEReg* iveReg)
{
    // in: U8C1,U8C1 out: U8C1
    // FIXME: check filter 1 channel setting

    // set RDMA
    iveReg->emitInputOp= [](IVEReg* iveReg) -> int {
      // first input
      iveReg->setImgIn();

      // second input
      iveReg->setRdma();

      return CVI_SUCCESS;
    };

    // emit op
    iveReg->emitOp = [](IVEReg* iveReg) -> int {
        // iveReg->bInstant == true means busy wait
        iveReg->ive_top_c->REG_94.reg_intr_en_filterop_odma = !iveReg->bInstant;
        iveReg->ive_top_c->REG_94.reg_intr_en_filterop_wdma_y = !iveReg->bInstant;
        iveReg->ive_top_c->REG_94.reg_intr_en_filterop_wdma_c = !iveReg->bInstant;

        iveReg->ive_top_c->REG_h10.reg_filterop_top_enable = 1;

        return CVI_SUCCESS;
    };

    // filter op done, copy result to DDR, control flow:
    // wait filterop done -> wait dma(1/3 channels)
    // dma default is hw setting

    iveReg->emitDMA = [](IVEReg* iveReg) -> int {
      iveReg->emitOdma();

      // trigger filterop
      iveReg->ive_top_c->REG_1.reg_fmt_vld_fg = 1;
      
      if (iveReg->bInstant) {
        while(!iveReg->ive_top_c->REG_90.reg_frame_done_filterop_wdma_y);

      }
      else {
        // schedule
        while (!iveReg->ive_top_c->REG_98.reg_intr_status_filterop_odma
            && !iveReg->ive_top_c->REG_98.reg_intr_status_filterop_wdma_y
            && !iveReg->ive_top_c->REG_98.reg_intr_status_filterop_wdma_c);
      }

      return CVI_SUCCESS;
    };

    return iveReg->emit();

}

CVI_S32 CVI_MPI_IVE_Thresh(IVEReg* iveReg)
{
    // in:S16C1, out: U8C1/S8C1
    // set RDMA
    iveReg->emitInputOp= [](IVEReg* iveReg) -> int {
        iveReg->setImgIn();
        return CVI_SUCCESS;
    };

    // emit op
    iveReg->emitOp = [](IVEReg* iveReg) -> int {
        // iveReg->bInstant == true means busy wait
        iveReg->ive_top_c->REG_94.reg_intr_en_filterop_odma = !iveReg->bInstant;
        iveReg->ive_top_c->REG_94.reg_intr_en_filterop_wdma_y = !iveReg->bInstant;
        iveReg->ive_top_c->REG_94.reg_intr_en_filterop_wdma_c = !iveReg->bInstant;

        iveReg->ive_top_c->REG_h10.reg_thresh_top_enable = 1;
        iveReg->ive_top_c->REG_h130.reg_thresh_thresh_en = 1;
        iveReg->ive_top_c->REG_h138.reg_thresh_st_16to8_en = 1;
        iveReg->ive_top_c->REG_h130.reg_thresh_top_mod = 1;
        
        return CVI_SUCCESS;
    };

    // filter op done, copy result to DDR, control flow:
    // wait filterop done -> wait dma(1/3 channels)
    // dma default is hw setting

    iveReg->emitDMA = [](IVEReg* iveReg) -> int {
        iveReg->emitOdma();

        // trigger
        iveReg->ive_top_c->REG_1.reg_fmt_vld_fg = 1;

        if (iveReg->bInstant) {
            // busy wait, for ONLY one channel output
            while(!iveReg->ive_top_c->REG_90.reg_frame_done_thresh);
        }
        else {
            // schedule
            // FIXME: check wait reg_intr_status_filterop_odma
            while (!iveReg->ive_top_c->REG_98.reg_intr_status_filterop_odma
                    && !iveReg->ive_top_c->REG_98.reg_intr_status_filterop_wdma_y
                    && !iveReg->ive_top_c->REG_98.reg_intr_status_filterop_wdma_c);
        }
        return CVI_SUCCESS;
    };
    return iveReg->emit();
}

CVI_S32 CVI_MPI_IVE_Thresh_S16(IVEReg* iveReg)
{
    // in:S16C1, out: U8C1/S8C1
    // FIXME: check img_in settings
    // copy from CVI_MPI_IVE_Thresh
    // set RDMA
    iveReg->emitInputOp= [](IVEReg* iveReg) -> int {
        //iveReg->setSrcDMAAddr(/*srcIdx=*/0, IVE_BLK_ID_IMG_IN);

        iveReg->setImgIn();
        
        // TODO: need to wait?

        return CVI_SUCCESS;
    };

    // emit op
    iveReg->emitOp = [](IVEReg* iveReg) -> int {
        // iveReg->bInstant == true means busy wait
        iveReg->ive_top_c->REG_94.reg_intr_en_filterop_odma = !iveReg->bInstant;
        iveReg->ive_top_c->REG_94.reg_intr_en_filterop_wdma_y = !iveReg->bInstant;
        iveReg->ive_top_c->REG_94.reg_intr_en_filterop_wdma_c = !iveReg->bInstant;

        // trigger
        iveReg->ive_top_c->REG_h10.reg_thresh_top_enable = 1;
        iveReg->ive_top_c->REG_h130.reg_thresh_thresh_en = 1;
        iveReg->ive_top_c->REG_h138.reg_thresh_st_16to8_en = 1;
        iveReg->ive_top_c->REG_h130.reg_thresh_top_mod = 1;
        
        return CVI_SUCCESS;
    };

    // filter op done, copy result to DDR, control flow:
    // wait filterop done -> wait dma(1/3 channels)
    // dma default is hw setting

    iveReg->emitDMA = [](IVEReg* iveReg) -> int {
        iveReg->emitOdma();
        iveReg->ive_top_c->REG_1.reg_fmt_vld_fg = 1;

        if (iveReg->bInstant) {
            // busy wait, for ONLY one channel output
            while(!iveReg->ive_top_c->REG_90.reg_frame_done_thresh);
        }
        else {
            // schedule
            // FIXME: check wait reg_intr_status_filterop_odma
            while (!iveReg->ive_top_c->REG_98.reg_intr_status_filterop_odma
                    && !iveReg->ive_top_c->REG_98.reg_intr_status_filterop_wdma_y
                    && !iveReg->ive_top_c->REG_98.reg_intr_status_filterop_wdma_c);
        }
        return CVI_SUCCESS;
    };
    return iveReg->emit();
}

CVI_S32 CVI_MPI_IVE_Thresh_U16(IVEReg* iveReg)
{
  // in: U16C1/S16C1, out:U8C1/S8C1
  // set RDMA
  iveReg->emitInputOp= [](IVEReg* iveReg) -> int {
    // TODO: check set input as u16/s16?
    iveReg->setImgIn();

    return CVI_SUCCESS;
  };

  // emit op
  iveReg->emitOp = [](IVEReg* iveReg) -> int {
    // iveReg->bInstant == true means busy wait
    iveReg->ive_top_c->REG_94.reg_intr_en_filterop_odma = !iveReg->bInstant;
    iveReg->ive_top_c->REG_94.reg_intr_en_filterop_wdma_y = !iveReg->bInstant;
    iveReg->ive_top_c->REG_94.reg_intr_en_filterop_wdma_c = !iveReg->bInstant;

    // trigger filterop
    //"2 frame operation_mode 3'd0: And 3'd1: Or 3'd2: Xor 3'd3: Add 3'd4: Sub 3'd5: Bypass mode output =frame source 0 3'd6: Bypass mode output =frame source 1 default: And"
    iveReg->ive_top_c->REG_20.reg_frame2op_op_mode = 5;
    // "2 frame operation_mode 3'd0: And 3'd1: Or 3'd2: Xor 3'd3: Add 3'd4: Sub 3'd5: Bypass mode output =frame source 0 3'd6: Bypass mode output =frame source 1 default: And"
    iveReg->ive_top_c->REG_h80.reg_frame2op_fg_op_mode = 6;

    iveReg->ive_top_c->REG_h10.reg_thresh_top_enable = 1;
    iveReg->ive_top_c->REG_h130.reg_thresh_thresh_en = 1;
    iveReg->ive_top_c->REG_h138.reg_thresh_st_16to8_en = 1;
    iveReg->ive_top_c->REG_h10.reg_filterop_top_enable = 1;

    return CVI_SUCCESS;
  };

  iveReg->emitDMA = [](IVEReg* iveReg) -> int {
    iveReg->emitOdma();

    iveReg->ive_top_c->REG_1.reg_fmt_vld_fg = 1;

    if (iveReg->bInstant) {
      while(!iveReg->ive_top_c->REG_90.reg_frame_done_filterop_wdma_y);
    }
    else {
      // schedule
      while (!iveReg->ive_top_c->REG_98.reg_intr_status_filterop_odma
          && !iveReg->ive_top_c->REG_98.reg_intr_status_filterop_wdma_y
          && !iveReg->ive_top_c->REG_98.reg_intr_status_filterop_wdma_c);
    }

    return CVI_SUCCESS;
  };

  return iveReg->emit();
}
//
CVI_S32 CVI_MPI_IVE_16BitTo8Bit(IVEReg* iveReg)
{
  // in: U16C1/S16C1, out:U8C1/S8C1
  // set RDMA
  iveReg->emitInputOp= [](IVEReg* iveReg) -> int {
    // TODO: check set input as u16/s16?
    iveReg->setImgIn();

    return CVI_SUCCESS;
  };

  // emit op
  iveReg->emitOp = [](IVEReg* iveReg) -> int {
    // iveReg->bInstant == true means busy wait
    iveReg->ive_top_c->REG_94.reg_intr_en_filterop_odma = !iveReg->bInstant;
    iveReg->ive_top_c->REG_94.reg_intr_en_filterop_wdma_y = !iveReg->bInstant;
    iveReg->ive_top_c->REG_94.reg_intr_en_filterop_wdma_c = !iveReg->bInstant;

    // trigger filterop
    //"2 frame operation_mode 3'd0: And 3'd1: Or 3'd2: Xor 3'd3: Add 3'd4: Sub 3'd5: Bypass mode output =frame source 0 3'd6: Bypass mode output =frame source 1 default: And"
    iveReg->ive_top_c->REG_20.reg_frame2op_op_mode = 5;
    // "2 frame operation_mode 3'd0: And 3'd1: Or 3'd2: Xor 3'd3: Add 3'd4: Sub 3'd5: Bypass mode output =frame source 0 3'd6: Bypass mode output =frame source 1 default: And"
    iveReg->ive_top_c->REG_h80.reg_frame2op_fg_op_mode = 6;
    iveReg->ive_top_c->REG_h10.reg_filterop_top_enable = 1;

    return CVI_SUCCESS;
  };

  iveReg->emitDMA = [](IVEReg* iveReg) -> int {
    iveReg->emitOdma();

    iveReg->ive_top_c->REG_1.reg_fmt_vld_fg = 1;

    if (iveReg->bInstant) {
      while(!iveReg->ive_top_c->REG_90.reg_frame_done_filterop_wdma_y);
    }
    else {
      // schedule
      while (!iveReg->ive_top_c->REG_98.reg_intr_status_filterop_odma
          && !iveReg->ive_top_c->REG_98.reg_intr_status_filterop_wdma_y
          && !iveReg->ive_top_c->REG_98.reg_intr_status_filterop_wdma_c);
    }

    return CVI_SUCCESS;
  };

  return iveReg->emit();
}

CVI_S32 CVI_MPI_IVE_CSC(IVEReg* iveReg)
{
  iveReg->emitInputOp= [](IVEReg* iveReg) -> int {
    iveReg->setImgIn();
    return CVI_SUCCESS;
  };

  // emit op
  iveReg->emitOp = [](IVEReg* iveReg) -> int {

    // set interrupt
    iveReg->ive_top_c->REG_94.reg_intr_en_filterop_odma = !iveReg->bInstant;
    iveReg->ive_top_c->REG_94.reg_intr_en_filterop_wdma_y = !iveReg->bInstant;
    iveReg->ive_top_c->REG_94.reg_intr_en_filterop_wdma_c = !iveReg->bInstant;

    // sw trigger
    iveReg->ive_top_c->REG_h10.reg_csc_top_enable = 1;
    //iveReg->ive_top_c->REG_14.reg_csc_enable = 1;
    // FIXME: second as CSC
    //iveReg->REG_h1c8->REG_h14.reg_filterop_op2_csc_enmode;
    // TODO: need to wait reg_frame_done_csc?
    return CVI_SUCCESS;
  };

  iveReg->emitDMA = [](IVEReg* iveReg) -> int {
    iveReg->emitOdma();
    iveReg->ive_top_c->REG_1.reg_fmt_vld_fg = 1;

    if (iveReg->bInstant) {
      // busy wait, for ONLY one channel output
      while(!iveReg->ive_top_c->REG_90.reg_frame_done_csc);
    }
    else {
      // schedule
      while (!iveReg->ive_top_c->REG_98.reg_intr_status_filterop_odma
          && !iveReg->ive_top_c->REG_98.reg_intr_status_filterop_wdma_y
          && !iveReg->ive_top_c->REG_98.reg_intr_status_filterop_wdma_c);
    }

    return CVI_SUCCESS;
  };

  return iveReg->emit();
}

CVI_S32 CVI_MPI_IVE_GradFg(IVEReg* iveReg)
{
    return iveReg->emit();
}
CVI_S32 CVI_MPI_IVE_NormGrad(IVEReg* iveReg)
{
    return iveReg->emit();
}
CVI_S32 CVI_MPI_IVE_Filter(IVEReg* iveReg)
{
    //NOTE:
    /*
     * input:
     *     img_in: first input
     *     rdma_img1: second input
     * output:
     *     odma: for filter 3ch and csc(handle img format wdma), wait(odma, wdma_y,wmda_c)
     *     wdma: ip->wdma, just wait wdma done
    */
    // set RDMA
    iveReg->emitInputOp= [](IVEReg* iveReg) -> int {
        iveReg->setImgIn();
        return CVI_SUCCESS;
    };

    // emit op
    iveReg->emitOp = [](IVEReg* iveReg) -> int {
        // iveReg->bInstant == true means busy wait
        iveReg->ive_top_c->REG_94.reg_intr_en_filterop_odma = !iveReg->bInstant;
        iveReg->ive_top_c->REG_94.reg_intr_en_filterop_wdma_y = !iveReg->bInstant;
        iveReg->ive_top_c->REG_94.reg_intr_en_filterop_wdma_c = !iveReg->bInstant;

        // trigger filterop
        //"2 frame operation_mode 3'd0: And 3'd1: Or 3'd2: Xor 3'd3: Add 3'd4: Sub 3'd5: Bypass mode output =frame source 0 3'd6: Bypass mode output =frame source 1 default: And"
        iveReg->ive_top_c->REG_20.reg_frame2op_op_mode = 5;
        // "2 frame operation_mode 3'd0: And 3'd1: Or 3'd2: Xor 3'd3: Add 3'd4: Sub 3'd5: Bypass mode output =frame source 0 3'd6: Bypass mode output =frame source 1 default: And"
        iveReg->ive_top_c->REG_h80.reg_frame2op_fg_op_mode = 6;
        iveReg->ive_top_c->REG_h10.reg_filterop_top_enable = 1;
        if (iveReg->srcEnType == IVE_IMAGE_TYPE_U8C1) {
          iveReg->ive_filterop_c->REG_h14.reg_op_y_wdma_en = 1;
        }
        else {
            // 3 channel
            iveReg->ive_filterop_c->REG_h14.reg_filterop_3ch_en = 1;
        }

        return CVI_SUCCESS;
    };

    // filter op done, copy result to DDR, control flow:
    // wait filterop done -> wait dma(1/3 channels)
    // dma default is hw setting

    iveReg->emitDMA = [](IVEReg* iveReg) -> int {
      if (iveReg->dstEnType == IVE_IMAGE_TYPE_U8C1) {
        iveReg->ive_filterop_c->ODMA_REG_00.reg_fmt_sel = iveReg->getImgFmtSel(iveReg->srcEnType);
        iveReg->emitOdma();

        // trigger
        iveReg->ive_top_c->REG_1.reg_fmt_vld_fg = 1;

        if (iveReg->bInstant) {
          // busy wait, for ONLY one channel output
          while(!iveReg->ive_top_c->REG_90.reg_frame_done_filterop_wdma_y);
        }
        else {
          // schedule
          while (!iveReg->ive_top_c->REG_98.reg_intr_status_filterop_odma
              && !iveReg->ive_top_c->REG_98.reg_intr_status_filterop_wdma_y
              && !iveReg->ive_top_c->REG_98.reg_intr_status_filterop_wdma_c);
        }
      }
      else {
        iveReg->emitOdma();
      }
      return CVI_SUCCESS;
    };

    return iveReg->emit();
}

CVI_S32 CVI_MPI_IVE_FilterAndCSC(IVEReg* iveReg)
{
    // leverage CSC
    // FIXME: check set properly order
    iveReg->ive_top_c->REG_h10.reg_filterop_top_enable = 1;
    return CVI_MPI_IVE_CSC(iveReg);
}
//
CVI_S32 CVI_MPI_IVE_Hist(IVEReg* iveReg)
{
  // in:U8C1, out:
  // copy from CVI_MPI_IVE_Integ
  // FIXME: check output format
  // set RDMA
  iveReg->emitInputOp= [](IVEReg* iveReg) -> int {
    iveReg->setImgIn();
    // FIXME: set reg_ive_hist_mem_addr?

    return CVI_SUCCESS;
  };

  // emit op
  iveReg->emitOp = [](IVEReg* iveReg) -> int {
    // iveReg->bInstant == true means busy wait
    iveReg->ive_top_c->REG_94.reg_intr_en_hist = !iveReg->bInstant;

    iveReg->ive_hist_c->REG_0.reg_ive_hist_enable = 1;
    iveReg->ive_top_c->REG_h10.reg_hist_top_enable = 1;

    return CVI_SUCCESS;
  };

  // filter op done, copy result to DDR, control flow:
  // wait filterop done -> wait dma(1/3 channels)
  // dma default is hw setting

  iveReg->emitDMA = [](IVEReg* iveReg) -> int {
    iveReg->emitOdma();

    iveReg->ive_top_c->REG_1.reg_fmt_vld_fg = 1;

    // FIXME: need to set U32C1/U64C1?
    if (iveReg->bInstant) {
      // busy wait, for ONLY one channel output
      while(!iveReg->ive_top_c->REG_90.reg_frame_done_hist);
    }
    else {
      // schedule
      while (!iveReg->ive_top_c->REG_98.reg_intr_status_hist);
    }
    return CVI_SUCCESS;
  };
  return iveReg->emit();
}
//
CVI_S32 CVI_MPI_IVE_EqualizeHist(IVEReg* iveReg)
{
    return iveReg->emit();
}
//
CVI_S32 CVI_MPI_IVE_Map(IVEReg* iveReg)
{
  //in: u8c1, out:u8c1/u16c1/s16c1
  // FIXME: check output settings
  // set RDMA
  iveReg->emitInputOp= [](IVEReg* iveReg) -> int {
    iveReg->setImgIn();
    return CVI_SUCCESS;
  };

  // emit op
  iveReg->emitOp = [](IVEReg* iveReg) -> int {
    // iveReg->bInstant == true means busy wait
    iveReg->ive_top_c->REG_94.reg_intr_en_filterop_odma = !iveReg->bInstant;
    iveReg->ive_top_c->REG_94.reg_intr_en_filterop_wdma_y = !iveReg->bInstant;
    iveReg->ive_top_c->REG_94.reg_intr_en_filterop_wdma_c = !iveReg->bInstant;

    iveReg->ive_top_c->REG_h10.reg_map_top_enable = 1;
    iveReg->ive_map_c->REG_0.reg_ip_enable = 1;
    // FIXME: check how to set ive_map

    return CVI_SUCCESS;
  };

  // filter op done, copy result to DDR, control flow:
  // wait filterop done -> wait dma(1/3 channels)
  // dma default is hw setting

  iveReg->emitDMA = [](IVEReg* iveReg) -> int {
    iveReg->emitOdma();

    // trigger
    iveReg->ive_top_c->REG_1.reg_fmt_vld_fg = 1;

    if (iveReg->bInstant) {
      // busy wait, for ONLY one channel output
      while(!iveReg->ive_top_c->REG_90.reg_frame_done_map);
    }
    else {
      // schedule
      // FIXME: check wait reg_intr_status_filterop_odma
      while (!iveReg->ive_top_c->REG_98.reg_intr_status_filterop_odma
          && !iveReg->ive_top_c->REG_98.reg_intr_status_filterop_wdma_y
          && !iveReg->ive_top_c->REG_98.reg_intr_status_filterop_wdma_c);
    }
    return CVI_SUCCESS;
  };
  return iveReg->emit();
}
//
CVI_S32 CVI_MPI_IVE_NCC(IVEReg* iveReg)
{
  // input: U8C1,U8C1, output:
  iveReg->emitInputOp= [](IVEReg* iveReg) -> int {
    iveReg->setImgIn();
    return CVI_SUCCESS;
  };

  // emit op
  iveReg->emitOp = [](IVEReg* iveReg) -> int {
    // iveReg->bInstant == true means busy wait
    iveReg->ive_top_c->REG_94.reg_intr_en_filterop_odma = !iveReg->bInstant;
    iveReg->ive_top_c->REG_94.reg_intr_en_filterop_wdma_y = !iveReg->bInstant;
    iveReg->ive_top_c->REG_94.reg_intr_en_filterop_wdma_c = !iveReg->bInstant;

    IVE_NCC_DST_MEM_S *ncc = (IVE_NCC_DST_MEM_S *)iveReg->getDst0Au64VirAddr(0);

    ncc->u64Numerator = ((CVI_U64)iveReg->ive_top_c->REG_h160.reg_ncc_nemerator_l << 32)
      | iveReg->ive_top_c->REG_h164.reg_ncc_nemerator_m;
    ncc->u64QuadSum1 = ((CVI_U64)iveReg->ive_top_c->REG_h168.reg_ncc_quadsum0_l << 32)
      | iveReg->ive_top_c->REG_h16C.reg_ncc_quadsum0_m;
    ncc->u64QuadSum2 = ((CVI_U64)iveReg->ive_top_c->REG_h170.reg_ncc_quadsum1_l << 32)
      | iveReg->ive_top_c->REG_h174.reg_ncc_quadsum1_m;
    iveReg->ive_top_c->REG_h10.reg_ncc_top_enable = 1;

    return CVI_SUCCESS;
  };

  // filter op done, copy result to DDR, control flow:
  // wait filterop done -> wait dma(1/3 channels)
  // dma default is hw setting

  iveReg->emitDMA = [](IVEReg* iveReg) -> int {
    // FIXME: is odma ?
    iveReg->emitOdma();
    iveReg->ive_top_c->REG_1.reg_fmt_vld_fg = 1;

    if (iveReg->bInstant) {
      // busy wait, for ONLY one channel output
      while(!iveReg->ive_top_c->REG_90.reg_frame_done_ncc);
    }
    else {
      // schedule
      // FIXME: check wait reg_intr_status_filterop_odma
      while (!iveReg->ive_top_c->REG_98.reg_intr_status_filterop_odma
          && !iveReg->ive_top_c->REG_98.reg_intr_status_filterop_wdma_y
          && !iveReg->ive_top_c->REG_98.reg_intr_status_filterop_wdma_c);
    }
    return CVI_SUCCESS;
  };

  return iveReg->emit();
}
//
CVI_S32 CVI_MPI_IVE_OrdStatFilter(IVEReg* iveReg)
{
  // in:U8C1, out: U8C1
  // set RDMA
  iveReg->emitInputOp= [](IVEReg* iveReg) -> int {
    iveReg->setImgIn();

    return CVI_SUCCESS;
  };

  // emit op
  iveReg->emitOp = [](IVEReg* iveReg) -> int {
    // iveReg->bInstant == true means busy wait
    iveReg->ive_top_c->REG_94.reg_intr_en_filterop_odma = !iveReg->bInstant;
    iveReg->ive_top_c->REG_94.reg_intr_en_filterop_wdma_y = !iveReg->bInstant;
    iveReg->ive_top_c->REG_94.reg_intr_en_filterop_wdma_c = !iveReg->bInstant;

    iveReg->ive_top_c->REG_h10.reg_filterop_top_enable = 1;

    return CVI_SUCCESS;
  };

  // filter op done, copy result to DDR, control flow:
  // wait filterop done -> wait dma(1/3 channels)
  // dma default is hw setting

  iveReg->emitDMA = [](IVEReg* iveReg) -> int {
    iveReg->emitOdma();
    iveReg->ive_top_c->REG_1.reg_fmt_vld_fg = 1;

    if (iveReg->bInstant) {
      // busy wait, for ONLY one channel output
      while(!iveReg->ive_top_c->REG_90.reg_frame_done_thresh);
    }
    else {
      // schedule
      // FIXME: check wait reg_intr_status_filterop_odma
      while (!iveReg->ive_top_c->REG_98.reg_intr_status_filterop_odma
          && !iveReg->ive_top_c->REG_98.reg_intr_status_filterop_wdma_y
          && !iveReg->ive_top_c->REG_98.reg_intr_status_filterop_wdma_c);
    }
    return CVI_SUCCESS;
  };
  return iveReg->emit();
}
//
CVI_S32 CVI_MPI_IVE_Resize(IVEReg* iveReg)
{
    return iveReg->emit();
}
//
CVI_S32 CVI_MPI_IVE_CannyHysEdge(IVEReg* iveReg)
{
    return iveReg->emit();
}
CVI_S32 CVI_MPI_IVE_CannyEdge(IVEReg* iveReg)
{
    return iveReg->emit();
}
//{
//
//	CVI_U32 *u32StackSize = (CVI_U32 *)(pstStack->u64VirAddr + pstStack->u32Size - 16);
//	//
//	// TODO: Here shall be real MPI IVE driver code, which fill in register command, then pass to hw/cmodel
//	// FIXME: call CVI_HW_XXX() directly for now
//	//
//	CVI_HW_CannyEdge(
//			(CVI_U8 *)pstEdge->au64VirAddr[0], (CVI_U16)pstEdge->au32Stride[0],
//			(CVI_U16 *)pstStack->u64VirAddr, u32StackSize,
//			(CVI_U16)pstEdge->u32Width, (CVI_U16)pstEdge->u32Height);
//
//	//return CVI_FAILURE;
//	return CVI_SUCCESS;
//}
//
CVI_S32 CVI_MPI_IVE_Integ(IVEReg* iveReg)
{
    // in:U8C1, out:U32C1/U64C1
    // FIXME: check U32C1/U64C1 settings
    // set RDMA
    iveReg->emitInputOp= [](IVEReg* iveReg) -> int {
        //iveReg->setSrcDMAAddr(/*srcIdx=*/0, IVE_BLK_ID_IMG_IN);
        iveReg->setImgIn();
        // FIXME: set reg_ive_hist_mem_addr?

        // TODO: need to wait?

        return CVI_SUCCESS;
    };

    // emit op
    iveReg->emitOp = [](IVEReg* iveReg) -> int {
        // iveReg->bInstant == true means busy wait
        iveReg->ive_top_c->REG_94.reg_intr_en_intg = !iveReg->bInstant;

        // trigger
        iveReg->ive_intg_c->REG_0.reg_ive_intg_enable = 1;
        iveReg->ive_top_c->REG_h10.reg_intg_top_enable = 1;

        //TODO: need to set up?
        //iveReg->ive_filterop_c->REG_h14.reg_op_y_wdma_en = 1;
        return CVI_SUCCESS;
    };

    // filter op done, copy result to DDR, control flow:
    // wait filterop done -> wait dma(1/3 channels)
    // dma default is hw setting

    iveReg->emitDMA = [](IVEReg* iveReg) -> int {
        iveReg->emitISPWdmaYOnly();

        // trigger filterop
        // TODO: need to setup->axi transaction mode
        iveReg->ive_filterop_c->ODMA_REG_00.reg_dma_blen = 1;
        iveReg->ive_filterop_c->ODMA_REG_00.reg_dma_en = 1;

        // FIXME: need to set U32C1/U64C1?
        if (iveReg->bInstant) {
            // busy wait, for ONLY one channel output
            while(!iveReg->ive_top_c->REG_90.reg_frame_done_intg);
        }
        else {
            // schedule
            while (!iveReg->ive_top_c->REG_98.reg_intr_status_intg);
        }
        return CVI_SUCCESS;
    };


    return iveReg->emit();
}

CVI_S32 CVI_MPI_IVE_LBP(IVEReg* iveReg)
{
  // in: U8C1, out: U8C1
  // set RDMA
  iveReg->emitInputOp= [](IVEReg* iveReg) -> int {
    iveReg->setImgIn();
    return CVI_SUCCESS;
  };

  // emit op
  iveReg->emitOp = [](IVEReg* iveReg) -> int {
    // iveReg->bInstant == true means busy wait
    iveReg->ive_top_c->REG_94.reg_intr_en_filterop_odma = !iveReg->bInstant;
    iveReg->ive_top_c->REG_94.reg_intr_en_filterop_wdma_y = !iveReg->bInstant;
    iveReg->ive_top_c->REG_94.reg_intr_en_filterop_wdma_c = !iveReg->bInstant;

    iveReg->ive_top_c->REG_h10.reg_filterop_top_enable = 1;

    return CVI_SUCCESS;
  };

  // filter op done, copy result to DDR, control flow:
  // wait filterop done -> wait dma(1/3 channels)
  // dma default is hw setting

  iveReg->emitDMA = [](IVEReg* iveReg) -> int {
    iveReg->emitOdma();

    // trigger
    iveReg->ive_top_c->REG_1.reg_fmt_vld_fg = 1;

    if (iveReg->bInstant) {
      // busy wait, for ONLY one channel output
      while(!iveReg->ive_top_c->REG_90.reg_frame_done_thresh);
    }
    else {
      // schedule
      // FIXME: check wait reg_intr_status_filterop_odma
      while (!iveReg->ive_top_c->REG_98.reg_intr_status_filterop_odma
          && !iveReg->ive_top_c->REG_98.reg_intr_status_filterop_wdma_y
          && !iveReg->ive_top_c->REG_98.reg_intr_status_filterop_wdma_c);
    }
    return CVI_SUCCESS;
  };
  return iveReg->emit();
}
//
CVI_S32 CVI_MPI_IVE_MagAndAng(IVEReg* iveReg)
{
  //in: U8C1, out: pstDstMag->U16C1, pstDstAng->U8C1
  // copy from Sobel
  // set RDMA
  iveReg->emitInputOp= [](IVEReg* iveReg) -> int {
    iveReg->setImgIn();
    return CVI_SUCCESS;
  };

  // emit op
  iveReg->emitOp = [](IVEReg* iveReg) -> int {
    // iveReg->bInstant == true means busy wait
    iveReg->ive_top_c->REG_94.reg_intr_en_filterop_odma = !iveReg->bInstant;
    iveReg->ive_top_c->REG_94.reg_intr_en_filterop_wdma_y = !iveReg->bInstant;
    iveReg->ive_top_c->REG_94.reg_intr_en_filterop_wdma_c = !iveReg->bInstant;

    // "0 : U8 1 : S16 2 : U16"
    iveReg->ive_filterop_c->REG_110.reg_filterop_map_enmode = 1;
    iveReg->ive_top_c->REG_h10.reg_filterop_top_enable = 1;

    return CVI_SUCCESS;
  };

  // filter op done, copy result to DDR, control flow:
  // wait filterop done -> wait dma(1/3 channels)
  // dma default is hw setting

  iveReg->emitDMA = [](IVEReg* iveReg) -> int {
    iveReg->emitOdma();

    // "0: Mag only ->  wdma_c 1 : Mag + Ang -> wdma_y and wdma_c"
    iveReg->ive_filterop_c->REG_h14.reg_op_c_wdma_en = 1;
    if (iveReg->ive_filterop_c->REG_110.reg_filterop_magang_out_ctrl == 1) {
      iveReg->emitDmaCtlSecondOutput();
    }

    // trigger
    iveReg->ive_top_c->REG_1.reg_fmt_vld_fg = 1;

    if (iveReg->bInstant) {
      // busy wait
      while(!iveReg->ive_top_c->REG_90.reg_frame_done_filterop_wdma_y);
    }
    else {
      // schedule
      while (!iveReg->ive_top_c->REG_98.reg_intr_status_filterop_odma
          && !iveReg->ive_top_c->REG_98.reg_intr_status_filterop_wdma_y
          && !iveReg->ive_top_c->REG_98.reg_intr_status_filterop_wdma_c);
    }
    return CVI_SUCCESS;
  };

  return iveReg->emit();
}
//
CVI_S32 CVI_MPI_IVE_STCandiCorner(IVEReg* iveReg)
{
    return iveReg->emit();
}
//
//int _CVI_Comp(const void *a1, const void*a2)
//{
//	return **(CVI_U8 **)a2 - **(CVI_U8 **)a1;
//}
//
//typedef struct _IVE_ST_CORNER_CELL_S
//{
//    CVI_U16 u16PointNum;
//	CVI_U16 Reserved;
//    IVE_POINT_U16_S *pstPoint;
//}IVE_ST_CORNER_CELL_S;
//
CVI_S32 CVI_MPI_IVE_STCorner(IVEReg* iveReg)
{
    return iveReg->emit();
}
//    IVE_ST_CORNER_CTRL_S *pstStCornerCtrl)
//{
//	//printf("CVI_MPI_IVE_STCorner\n");
//
//	// CVI_STCornerCollect()
//	CVI_S32 s32Ret = CVI_SUCCESS;
//	CVI_U32 u32Width = pstCandiCorner->u32Width;
//	CVI_U32 u32Height = pstCandiCorner->u32Height;
//	CVI_U32 u32Stride = pstCandiCorner->au32Stride[0];
//	CVI_U8 *pCandiCorner = (CVI_U8 *)pstCandiCorner->au64VirAddr[0];
//	CVI_U8 **Base = NULL;
//	CVI_U8 *Block = NULL;
//	CVI_U32 x, y, k, l, BlkX, BlkY, m, n, ii;
//	CVI_U32 NumOfElements = 0;
//	CVI_U32 u32BlkWidth;
//	CVI_U32 u32BlkHeight;
//	CVI_U16 u16MinDist = pstStCornerCtrl->u16MinDist;
//	CVI_U16 u16MinDista = u16MinDist * u16MinDist;
//	IVE_ST_CORNER_CELL_S *pstCells;
//	IVE_ST_CORNER_CELL_S *cell;
//	CVI_U32 v13, v14, v15, v16, v17, v18;
//	CVI_U32 new_point;
//	IVE_ST_CORNER_INFO_S *pstCornerInfo = (IVE_ST_CORNER_INFO_S *)pstCorner->u64VirAddr;
//	pstCornerInfo->u16CornerNum = 0;
//
//	if (!pstStCornerCtrl->u16MinDist)
//	{
//		printf("u16MinDist is zero\n");
//		return CVI_FAILURE;
//	}
//
//	Base = (CVI_U8 **)malloc(sizeof(CVI_U8 *) * u32Width * u32Height);
//	if (!Base) {
//		printf("malloc Base failed\n");
//		return CVI_FAILURE;
//	}
//	//memset(Base, 0, sizeof(CVI_U8 *) * u32Width * u32Height);
//
//	u32BlkWidth = (u32Width + u16MinDist - 1) / u16MinDist;
//	u32BlkHeight = (u32Height + u16MinDist - 1) / u16MinDist;
//	Block = (CVI_U8 *)malloc(sizeof(IVE_ST_CORNER_CELL_S) * u32BlkWidth * u32BlkHeight);
//	if (!Block) {
//		printf("malloc Block failed\n");
//		free(Base);
//		return CVI_FAILURE;
//	}
//	//memset(Block, 0, sizeof(IVE_ST_CORNER_CELL_S) * u32BlkWidth * u32BlkHeight);
//
//	for (y = 1; y < u32Height - 1; ++y)
//	{
//		for (x = 1; x < u32Width - 1; ++x)
//		{
//			if (pCandiCorner[y * u32Stride + x])
//			{
//				Base[NumOfElements++] = &pCandiCorner[y * u32Stride + x];
//			}
//		}
//	}
//	//printf("NumOfElements = %d\n", NumOfElements);
//
//	qsort(Base, NumOfElements, sizeof(CVI_U8 *), _CVI_Comp);
//
//	//for (l = 0; l < 300; ++l)
//	//{
//	//	y = (CVI_U32)((Base[l] - pCandiCorner) / u32Stride);
//	//	x = (CVI_U32)((Base[l] - pCandiCorner) - u32Stride * y);
//	//	printf("[%d] x = %04x, y = %04x, v = %03d\n", l, x, y, *Base[l]);
//	//}
//
//	memset(Block, 0, sizeof(IVE_ST_CORNER_CELL_S) * u32BlkWidth * u32BlkHeight);
//	pstCells = (IVE_ST_CORNER_CELL_S *)Block;
//	for (k = 0; k < u32BlkWidth * u32BlkHeight; ++k)
//	{
//		pstCells[k].pstPoint = (IVE_POINT_U16_S *)malloc(sizeof(IVE_POINT_U16_S) * u16MinDist * u16MinDist);
//		if (!Block) {
//			printf("malloc pstCells[%d].pstPoint failed\n", k);
//			s32Ret = CVI_FAILURE;
//			goto out;
//		}
//		memset(pstCells[k].pstPoint, 0, sizeof(IVE_POINT_U16_S) * u16MinDist * u16MinDist);
//	}
//
//	for (l = 0; l < NumOfElements; ++l)
//	{
//		y = (CVI_U32)((Base[l] - pCandiCorner) / u32Stride);
//		x = (CVI_U32)((Base[l] - pCandiCorner) - u32Stride * y);
//
//		new_point = 1;
//
//		BlkY = y / u16MinDist;
//		BlkX = x / u16MinDist;
//		v13 = ((BlkX - 1) <= 0) ? 0 : (BlkX - 1);
//		v14 = ((BlkY - 1) <= 0) ? 0 : (BlkY - 1);
//		v15 = ((BlkX + 1) > (u32BlkWidth - 1)) ? (u32BlkWidth - 1) : (BlkX + 1);
//		v16 = ((BlkY + 1) > (u32BlkHeight - 1)) ? (u32BlkHeight - 1) : (BlkY + 1);
//		for (m = v14; m <= v16; ++m)
//		{
//			for (n = v13; n <= v15; ++n)
//			{
//				cell = &pstCells[u32BlkWidth * m + n];
//				if (cell->u16PointNum)
//				{
//					for (ii = 0; ii < cell->u16PointNum; ++ii)
//					{
//						v18 = x - cell->pstPoint[ii].u16X;
//						v17 = y - cell->pstPoint[ii].u16Y;
//						if (v17 * v17 + v18 * v18 < u16MinDista)
//						{
//							new_point = 0;
//
//							// break out the loops
//							n = v15 + 1;
//							m = v16 + 1;
//							break;
//						}
//					}
//				}
//			}
//		}
//
//		if (new_point)
//		{
//			cell = &pstCells[u32BlkWidth * BlkY + BlkX];
//			cell->pstPoint[cell->u16PointNum].u16X = x;
//			cell->pstPoint[cell->u16PointNum].u16Y = y;
//			cell->u16PointNum ++;
//			pstCornerInfo->astCorner[pstCornerInfo->u16CornerNum].u16X = x;
//			pstCornerInfo->astCorner[pstCornerInfo->u16CornerNum].u16Y = y;
//			pstCornerInfo->u16CornerNum ++;
//
//			if (pstCornerInfo->u16CornerNum >= pstStCornerCtrl->u16MaxCornerNum)
//			{
//				break;
//			}
//		}
//	}
//
//out:
//	for (k = 0; k < u32BlkWidth * u32BlkHeight; ++k)
//	{
//		if (pstCells[k].pstPoint)
//		{
//			free(pstCells[k].pstPoint);
//		}
//	}
//	free(Block);
//	free(Base);
//
//	return s32Ret;
//}
//
CVI_S32 CVI_MPI_IVE_Sobel(IVEReg* iveReg)
{
    // in: U8C1, out: two S16C1
    // set RDMA
    iveReg->emitInputOp= [](IVEReg* iveReg) -> int {
        iveReg->setImgIn();
        
        return CVI_SUCCESS;
    };

    // emit op
    iveReg->emitOp = [](IVEReg* iveReg) -> int {
        // iveReg->bInstant == true means busy wait
        iveReg->ive_top_c->REG_94.reg_intr_en_filterop_odma = !iveReg->bInstant;
        iveReg->ive_top_c->REG_94.reg_intr_en_filterop_wdma_y = !iveReg->bInstant;
        iveReg->ive_top_c->REG_94.reg_intr_en_filterop_wdma_c = !iveReg->bInstant;

        // "0 : U8 1 : S16 2 : U16"
        // FIXME: check enable in Sobel
        iveReg->ive_filterop_c->REG_110.reg_filterop_map_enmode = 1;
        //iveReg->ive_filterop_c->REG_110.reg_filterop_norm_out_ctrl = iveReg->ctrl;
        iveReg->ive_top_c->REG_h10.reg_filterop_top_enable = 1;

        return CVI_SUCCESS;
    };

    // filter op done, copy result to DDR, control flow:
    // wait filterop done -> wait dma(1/3 channels)
    // dma default is hw setting

    iveReg->emitDMA = [](IVEReg* iveReg) -> int {
      iveReg->emitOdma();

      // second output for HV
      //"0 : h , v  -> wdma_y wdma_c will be active 1 :h only -> wdma_y 2: v only -> wdma_c 3. h , v pack => {v ,h} -> wdma_y "
      if (iveReg->ive_filterop_c->REG_110.reg_filterop_norm_out_ctrl == 0) {
        iveReg->emitDmaCtlSecondOutput();
      }

      // trigger 
      iveReg->ive_top_c->REG_1.reg_fmt_vld_fg = 1;

      if (iveReg->bInstant) {
        // busy wait
        while(!iveReg->ive_top_c->REG_90.reg_frame_done_filterop_wdma_y);
      }
      else {
        // schedule
        while (!iveReg->ive_top_c->REG_98.reg_intr_status_filterop_odma
            && !iveReg->ive_top_c->REG_98.reg_intr_status_filterop_wdma_y
            && !iveReg->ive_top_c->REG_98.reg_intr_status_filterop_wdma_c);
      }
      return CVI_SUCCESS;
    };

    return iveReg->emit();
}
//
CVI_S32 CVI_MPI_IVE_CCL(IVEReg* iveReg)
{
    return iveReg->emit();
}
//
CVI_S32 CVI_MPI_IVE_Dilate(IVEReg* iveReg)
{
  // in: U8C1, out: U8C1
  // FIXME: check filter 1 channel setting
  return CVI_MPI_IVE_Erode(iveReg);
  //return CVI_MPI_IVE_Filter(iveReg);
}
//
CVI_S32 CVI_MPI_IVE_Erode(IVEReg* iveReg)
{
    // in: U8C1, out: U8C1
    // FIXME: check filter 1 channel setting

    // set RDMA
    iveReg->emitInputOp= [](IVEReg* iveReg) -> int {
        iveReg->setImgIn();
        
        return CVI_SUCCESS;
    };

    // emit op
    iveReg->emitOp = [](IVEReg* iveReg) -> int {
        // iveReg->bInstant == true means busy wait
        iveReg->ive_top_c->REG_94.reg_intr_en_filterop_odma = !iveReg->bInstant;
        iveReg->ive_top_c->REG_94.reg_intr_en_filterop_wdma_y = !iveReg->bInstant;
        iveReg->ive_top_c->REG_94.reg_intr_en_filterop_wdma_c = !iveReg->bInstant;

        // trigger filterop
        //"2 frame operation_mode 3'd0: And 3'd1: Or 3'd2: Xor 3'd3: Add 3'd4: Sub 3'd5: Bypass mode output =frame source 0 3'd6: Bypass mode output =frame source 1 default: And"
        iveReg->ive_top_c->REG_20.reg_frame2op_op_mode = 5;
        // "2 frame operation_mode 3'd0: And 3'd1: Or 3'd2: Xor 3'd3: Add 3'd4: Sub 3'd5: Bypass mode output =frame source 0 3'd6: Bypass mode output =frame source 1 default: And"
        iveReg->ive_top_c->REG_h80.reg_frame2op_fg_op_mode = 6;
        iveReg->ive_top_c->REG_h10.reg_filterop_top_enable = 1;
        // FIXME: check default is 0
        iveReg->ive_top_c->REG_R2Y4_14.reg_csc_r2y4_enmode = 0;

        return CVI_SUCCESS;
    };

    iveReg->emitDMA = [](IVEReg* iveReg) -> int {
      iveReg->emitOdma();

      iveReg->ive_top_c->REG_1.reg_fmt_vld_fg = 1;
      
      //int loop = 20000;
      //printf("cant wait for reg_frame_done_filterop_odma, loop %d\n", loop);
      //while(loop--);
      iveReg->iveDebug("after vld\n");

      if (iveReg->bInstant) {
        while(!iveReg->ive_top_c->REG_90.reg_frame_done_filterop_wdma_y);
      }
      else {
        // schedule
        while (!iveReg->ive_top_c->REG_98.reg_intr_status_filterop_odma
            && !iveReg->ive_top_c->REG_98.reg_intr_status_filterop_wdma_y
            && !iveReg->ive_top_c->REG_98.reg_intr_status_filterop_wdma_c);
      }

      return CVI_SUCCESS;
    };

    return iveReg->emit();

    //return CVI_MPI_IVE_Filter(iveReg);
}
//
CVI_S32 CVI_MPI_IVE_MatchBgModel(IVEReg* iveReg)
{
    return iveReg->emit();
}
//
CVI_S32 CVI_MPI_IVE_UpdateBgModel(IVEReg* iveReg)
{
    return iveReg->emit();
}
//
CVI_S32 CVI_MPI_IVE_GMM(IVEReg* iveReg)
{
    return iveReg->emit();
}
//
CVI_S32 CVI_MPI_IVE_GMM2(IVEReg* iveReg)
{
    return iveReg->emit();
}
//
CVI_S32 CVI_MPI_IVE_LKOpticalFlowPyr(IVEReg* iveReg)
{
    return iveReg->emit();
}
//
CVI_S32 CVI_MPI_IVE_SAD(IVEReg* iveReg)
{
    return iveReg->emit();
}
//
CVI_S32 CVI_MPI_IVE_Bernsen(IVEReg* iveReg)
{
    return iveReg->emit();
}

CVI_S32 CVI_MPI_IVE_imgInToOdma(IVEReg* iveReg)
{
  //NOTE:
  /*
   * input:
   *     img_in: first input
   *     rdma_img1: second input
   * output:
   *     odma: for filter 3ch and csc(handle img format wdma), wait(odma, wdma_y,wmda_c)
   *     wdma: ip->wdma, just wait wdma done
   */
  // set RDMA
  iveReg->emitInputOp= [](IVEReg* iveReg) -> int {
    iveReg->setImgIn();
    return CVI_SUCCESS;
  };

  // emit op
  iveReg->emitOp = [](IVEReg* iveReg) -> int {
    // iveReg->bInstant == true means busy wait
    iveReg->ive_top_c->REG_94.reg_intr_en_filterop_odma = !iveReg->bInstant;
    iveReg->ive_top_c->REG_94.reg_intr_en_filterop_wdma_y = !iveReg->bInstant;
    iveReg->ive_top_c->REG_94.reg_intr_en_filterop_wdma_c = !iveReg->bInstant;

    // trigger filterop
    //"2 frame operation_mode 3'd0: And 3'd1: Or 3'd2: Xor 3'd3: Add 3'd4: Sub 3'd5: Bypass mode output =frame source 0 3'd6: Bypass mode output =frame source 1 default: And"
    iveReg->ive_top_c->REG_20.reg_frame2op_op_mode = 5;
    // "2 frame operation_mode 3'd0: And 3'd1: Or 3'd2: Xor 3'd3: Add 3'd4: Sub 3'd5: Bypass mode output =frame source 0 3'd6: Bypass mode output =frame source 1 default: And"
    iveReg->ive_top_c->REG_h80.reg_frame2op_fg_op_mode = 6;
    iveReg->ive_top_c->REG_h10.reg_filterop_top_enable = 1;
    iveReg->ive_filterop_c->REG_h14.reg_op_y_wdma_en = 0;

    return CVI_SUCCESS;
  };

  // filter op done, copy result to DDR, control flow:
  // wait filterop done -> wait dma(1/3 channels)
  // dma default is hw setting

  iveReg->emitDMA = [](IVEReg* iveReg) -> int {
    iveReg->emitOdma();

    // NOTICE: test img_in = odma
    iveReg->ive_filterop_c->REG_h14.reg_op_y_wdma_en = 0;

    // NOTICE: last one to trigger it
    iveReg->ive_top_c->REG_1.reg_fmt_vld_fg = 1;

    if (iveReg->bInstant) {
#if 0
      while(!iveReg->ive_top_c->REG_90.reg_frame_done_filterop_odma);
#else
      iveReg->iveDebug("after vld\n");

      int loop = 20000;
      printf("cant wait for reg_frame_done_filterop_odma, loop %d\n", loop);
      while(loop--);
#endif
    }
    else {
      // schedule
      while (!iveReg->ive_top_c->REG_98.reg_intr_status_filterop_odma
          && !iveReg->ive_top_c->REG_98.reg_intr_status_filterop_wdma_y
          && !iveReg->ive_top_c->REG_98.reg_intr_status_filterop_wdma_c);
    }
    return CVI_SUCCESS;
  };

  return iveReg->emit();
}

CVI_S32 CVI_MPI_IVE_rgbPToYuvToErodeToDilate(IVEReg* iveReg)
{
  //NOTE:
  /*
   * input:
   *     img_in: first input
   *     rdma_img1: second input
   * output:
   *     odma: for filter 3ch and csc(handle img format wdma), wait(odma, wdma_y,wmda_c)
   *     wdma: ip->wdma, just wait wdma done
   */

  // set RDMA
  uint32_t *p;
  printf("1 direcy set by hex\n");
  p = (uint32_t*)0x0a0a0000; *p = 0x01ff01ff;
  p = (uint32_t*)0x0a0a0000; *p = 0x01ff01ff;
  p = (uint32_t*)0x0a0a0008; *p = 0x01ff01ff;
  p = (uint32_t*)0x0a0a0010; *p = 0x00079fff;
  p = (uint32_t*)0x0a0a001c; *p = 0x00000132;
  p = (uint32_t*)0x0a0a0020; *p = 0x00000259;
  p = (uint32_t*)0x0a0a0024; *p = 0x00000075;
  p = (uint32_t*)0x0a0a0028; *p = 0x00000200;
  p = (uint32_t*)0x0a0a002c; *p = 0x000000b0;
  p = (uint32_t*)0x0a0a0030; *p = 0x0000015b;
  p = (uint32_t*)0x0a0a0034; *p = 0x0000020b;
  p = (uint32_t*)0x0a0a0038; *p = 0x00020200;
  p = (uint32_t*)0x0a0a003c; *p = 0x0000020b;
  p = (uint32_t*)0x0a0a0040; *p = 0x000001b6;
  p = (uint32_t*)0x0a0a0044; *p = 0x00000055;
  p = (uint32_t*)0x0a0a0048; *p = 0x00020200;
  p = (uint32_t*)0x0a0a004c; *p = 0x00000018;
  p = (uint32_t*)0x0a0a0100; *p = 0x01ff01ff;
  p = (uint32_t*)0x0a0a0104; *p = 0x01ff01ff;
  p = (uint32_t*)0x0a0a0114; *p = 0x00000000;
  p = (uint32_t*)0x0a0a0118; *p = 0x00000000;
  p = (uint32_t*)0x0a0a0400; *p = 0x00002822;
  p = (uint32_t*)0x0a0a0408; *p = 0x01ff01ff;
  p = (uint32_t*)0x0a0a040c; *p = 0x00000200;
  p = (uint32_t*)0x0a0a0410; *p = 0x00000200;
  p = (uint32_t*)0x0a0a0420; *p = 0x00000000;
  p = (uint32_t*)0x0a0a0424; *p = 0x20000fc0;
  p = (uint32_t*)0x0a0a0428; *p = 0x00000001;
  p = (uint32_t*)0x0a0a042c; *p = 0x22000fc0;
  p = (uint32_t*)0x0a0a0430; *p = 0x00000001;
  p = (uint32_t*)0x0a0a0434; *p = 0x24000fc0;
  p = (uint32_t*)0x0a0a0438; *p = 0x00000001;
  p = (uint32_t*)0x0a0a0440; *p = 0x00000000;
  p = (uint32_t*)0x0a0a0444; *p = 0x00000000;
  p = (uint32_t*)0x0a0a0448; *p = 0x00000000;
  p = (uint32_t*)0x0a0a044c; *p = 0x00000000;
  p = (uint32_t*)0x0a0a0450; *p = 0x00000000;
  p = (uint32_t*)0x0a0a0458; *p = 0x00000000;
  p = (uint32_t*)0x0a0a0460; *p = 0x00000000;
  p = (uint32_t*)0x0a0a0464; *p = 0x00000000;
  p = (uint32_t*)0x0a0a0480; *p = 0x00000000;
  p = (uint32_t*)0x0a0a2040; *p = 0x00210000;
  p = (uint32_t*)0x0a0a2044; *p = 0x86000fc0;
  p = (uint32_t*)0x0a0a2224; *p = 0xffff0000;
  p = (uint32_t*)0x0a0a2228; *p = 0xff0000ff;
  p = (uint32_t*)0x0a0a222c; *p = 0x0000ffff;
  p = (uint32_t*)0x0a0a2230; *p = 0x00ffffff;
  p = (uint32_t*)0x0a0a2274; *p = 0xffff0000;
  p = (uint32_t*)0x0a0a2278; *p = 0xff0000ff;
  p = (uint32_t*)0x0a0a227c; *p = 0x0000ffff;
  p = (uint32_t*)0x0a0a2280; *p = 0x00ffffff;
  p = (uint32_t*)0x0a0a228c; *p = 0x00000001;
  p = (uint32_t*)0x0a0a2300; *p = 0x00000000;
  p = (uint32_t*)0x0a0a2304; *p = 0x00000000;
  p = (uint32_t*)0x0a0a2320; *p = 0x00000203;
  p = (uint32_t*)0x0a0a2324; *p = 0x80000fc0;
  p = (uint32_t*)0x0a0a232c; *p = 0x82000fc0;
  p = (uint32_t*)0x0a0a2334; *p = 0x84000fc0;
  p = (uint32_t*)0x0a0a233c; *p = 0x00000200;
  p = (uint32_t*)0x0a0a2340; *p = 0x00000200;
  p = (uint32_t*)0x0a0a234c; *p = 0x000001ff;
  p = (uint32_t*)0x0a0a2350; *p = 0x000001ff;
  p = (uint32_t*)0x0a0a0078; *p = 0x00000005;
  p = (uint32_t*)0x0a0a0080; *p = 0x00000006;
  p = (uint32_t*)0x0a0a0004; *p = 0x00000012;
  
    

  iveReg->emitInputOp= [](IVEReg* iveReg) -> int {
    iveReg->img_in_c->REG_Y_BASE_0.reg_src_y_base_b0 =
      iveReg->getSrcLHSAu64PhyAddr(0) & 0xffffffff;
    iveReg->img_in_c->REG_U_BASE_0.reg_src_u_base_b0 =
      iveReg->getSrcLHSAu64PhyAddr(1) & 0xffffffff;
    iveReg->img_in_c->REG_V_BASE_0.reg_src_v_base_b0 =
      iveReg->getSrcLHSAu64PhyAddr(2) & 0xffffffff;
    iveReg->img_in_c->REG_Y_BASE_1.reg_src_y_base_b1 = 0;
    iveReg->img_in_c->REG_U_BASE_1.reg_src_u_base_b1 = 0;
    iveReg->img_in_c->REG_V_BASE_1.reg_src_v_base_b1 = 0;
    return CVI_SUCCESS;
  };

  // emit op
  iveReg->emitOp = [](IVEReg* iveReg) -> int {
    // iveReg->bInstant == true means busy wait
    iveReg->ive_top_c->REG_94.reg_intr_en_filterop_odma = !iveReg->bInstant;
    iveReg->ive_top_c->REG_94.reg_intr_en_filterop_wdma_y = !iveReg->bInstant;
    iveReg->ive_top_c->REG_94.reg_intr_en_filterop_wdma_c = !iveReg->bInstant;

    return CVI_SUCCESS;
  };

  // filter op done, copy result to DDR, control flow:
  // wait filterop done -> wait dma(1/3 channels)
  // dma default is hw setting

  iveReg->emitDMA = [](IVEReg* iveReg) -> int {
    iveReg->ive_filterop_c->ODMA_REG_01.reg_dma_y_base_low_part=  iveReg->getDst1Au64PhyAddr(0) & 0xffffffff          ;// 0x0
    iveReg->ive_filterop_c->ODMA_REG_02.reg_dma_y_base_high_part= 0           ;// 0x0
    iveReg->ive_filterop_c->ODMA_REG_03.reg_dma_u_base_low_part= iveReg->getDst1Au64PhyAddr(1) & 0xffffffff           ;// 0x0
    iveReg->ive_filterop_c->ODMA_REG_04.reg_dma_u_base_high_part= 0           ;// 0x0
    iveReg->ive_filterop_c->ODMA_REG_05.reg_dma_v_base_low_part= iveReg->getDst1Au64PhyAddr(2) & 0xffffffff           ;// 0x0
    iveReg->ive_filterop_c->ODMA_REG_06.reg_dma_v_base_high_part= 0           ;// 0x0
    iveReg->isp_dma_ctl_c->BASE_ADDR.reg_basel            = 
      iveReg->getDst0Au64PhyAddr(0);

    // NOTICE: last one to trigger it
    iveReg->iveDebug("before vld\n");
    iveReg->ive_top_c->REG_1.reg_fmt_vld_fg = 1;
    iveReg->iveDebug("after vld\n");

    if (iveReg->bInstant) {
#if 1
      while(!iveReg->ive_top_c->REG_90.reg_frame_done_filterop_wdma_y);
#else

      int loop = 20000;
      printf("cant wait for reg_frame_done_filterop_odma, loop %d\n", loop);
      while(loop--);
#endif
    }
    else {
      // schedule
      while (!iveReg->ive_top_c->REG_98.reg_intr_status_filterop_odma
          && !iveReg->ive_top_c->REG_98.reg_intr_status_filterop_wdma_y
          && !iveReg->ive_top_c->REG_98.reg_intr_status_filterop_wdma_c);
    }
    return CVI_SUCCESS;
  };

  return iveReg->emit();
}

// fake for link libreg.a
int CVI_HW_Add(
    CVI_U8 *pstSrc1, CVI_U16 Src1_u16Stride,
    CVI_U8 *pstSrc2, CVI_U16 Src2_u16Stride,
    CVI_U8 *pstDst, CVI_U16 Dst_u16Stride,
    CVI_U16 u16Width, CVI_U16 u16Height,
    CVI_U0Q16 u0q16X, CVI_U0Q16 u0q16Y) {return CVI_SUCCESS;}

int CVI_HW_And(
    CVI_U8 *pstSrc1, CVI_U16 Src1_u16Stride,
    CVI_U8 *pstSrc2, CVI_U16 Src2_u16Stride,
    CVI_U8 *pstDst, CVI_U16 Dst_u16Stride,
    CVI_U16 u16Width, CVI_U16 u16Height) {return CVI_SUCCESS;}

int CVI_HW_Or(
    CVI_U8 *pstSrc1, CVI_U16 Src1_u16Stride,
    CVI_U8 *pstSrc2, CVI_U16 Src2_u16Stride,
    CVI_U8 *pstDst, CVI_U16 Dst_u16Stride,
    CVI_U16 u16Width, CVI_U16 u16Height) {return CVI_SUCCESS;}

int CVI_HW_Sub(
    CVI_U8 *pstSrc1, CVI_U16 Src1_u16Stride,
    CVI_U8 *pstSrc2, CVI_U16 Src2_u16Stride,
    CVI_U8 *pstDst, CVI_U16 Dst_u16Stride,
    CVI_U16 u16Width, CVI_U16 u16Height,
    CVI_U8 enMode) {return CVI_SUCCESS;}

int CVI_HW_Xor(
    CVI_U8 *pstSrc1, CVI_U16 Src1_u16Stride,
    CVI_U8 *pstSrc2, CVI_U16 Src2_u16Stride,
    CVI_U8 *pstDst, CVI_U16 Dst_u16Stride,
    CVI_U16 u16Width, CVI_U16 u16Height) {return CVI_SUCCESS;}

int CVI_HW_Thresh(
    CVI_U8 *pstSrc, CVI_U16 Src_u16Stride,
    CVI_U8 *pstDst, CVI_U16 Dst_u16Stride,
    CVI_U16 u16Width, CVI_U16 u16Height,
    CVI_U8 lowThr, CVI_U8 highThr,
    CVI_U8 minVal, CVI_U8 midVal, CVI_U8 maxVal,
    IVE_THRESH_MODE_E enMode) {return CVI_SUCCESS;}
////#warning "remove reference"
//int CVI_HW_Thresh_S16(
//    CVI_S16 *pstSrc, CVI_U16 Src_u16Stride,
//    IVE_8BIT_U *pstDst, CVI_U16 Dst_u16Stride,
//    CVI_U16 u16Width, CVI_U16 u16Height,
//    CVI_S16 lowThr, CVI_S16 highThr,
//    IVE_8BIT_U minVal, IVE_8BIT_U midVal, IVE_8BIT_U maxVal,
//    IVE_THRESH_S16_MODE_E enMode) {return CVI_SUCCESS;}
//
//int CVI_HW_Thresh_U16(
//    CVI_U16 *pstSrc, CVI_U16 Src_u16Stride,
//    CVI_U8 *pstDst, CVI_U16 Dst_u16Stride,
//    CVI_U16 u16Width, CVI_U16 u16Height,
//    CVI_U16 lowThr, CVI_U16 highThr,
//    CVI_U8 minVal, CVI_U8 midVal, CVI_U8 maxVal,
//    IVE_THRESH_U16_MODE_E enMode) {return CVI_SUCCESS;}

int CVI_HW_DMA(
    CVI_U8 *pstSrc, CVI_U16 Src_u16Stride,
    CVI_U8 *pstDst, CVI_U16 Dst_u16Stride,
    CVI_U8 u8HorSegSize, CVI_U8 u8ElemSize, CVI_U8 u8VerSegRows,
    CVI_U32 u64Val_L32b, CVI_U32 u64Val_H32b,
    CVI_U16 u16Width, CVI_U16 u16Height, CVI_U8 enMode) {return CVI_SUCCESS;}

int CVI_HW_16BitTo8Bit(
    CVI_U16 *pstSrc, CVI_U16 Src_u16Stride,
    CVI_U8 *pstDst, CVI_U16 Dst_u16Stride,
    CVI_U16 u16Width, CVI_U16 u16Height,
    CVI_U8 enMode, CVI_U16 u8Num_div_u16Den, CVI_S8 s8Bias) {return CVI_SUCCESS;}

int CVI_HW_CSC(
    CVI_U8 **pstSrc, CVI_U16 *Src_u16Stride,
    CVI_U8 **pstDst, CVI_U16 *Dst_u16Stride,
    CVI_U16 u16Width, CVI_U16 u16Height,
    CVI_U8 enSrcType, CVI_U8 enMode, CVI_U8 enDstType) {return CVI_SUCCESS;}

int CVI_HW_UnpackUV(
    CVI_U8 *pstSrc, CVI_U16 Src_u16Stride,
    CVI_U8 *pstDst,
    CVI_U16 u16Width, CVI_U16 u16Height) {return CVI_SUCCESS;}

int CVI_HW_RepackUV(
    CVI_U8 *pstSrc,
    CVI_U8 *pstDst, CVI_U16 Dst_u16Stride,
    CVI_U16 u16Width, CVI_U16 u16Height) {return CVI_SUCCESS;}

int CVI_HW_Filter(
    CVI_U8 *pstSrc, CVI_U16 Src_u16Stride,
    CVI_U8 *pstDst, CVI_U16 Dst_u16Stride,
    CVI_U16 u16Width, CVI_U16 u16Height,
    CVI_S8 *au8Mask, CVI_U8 u8Norm) {return CVI_SUCCESS;}

int CVI_HW_Hist(
    CVI_U8 *pstSrc, CVI_U16 Src_u16Stride,
    CVI_U32 *pstDst, CVI_U16 u16Width, CVI_U16 u16Height) {return CVI_SUCCESS;}

int CVI_HW_EqualizeHist(
    CVI_U8 *pstSrc, CVI_U16 Src_u16Stride,
    CVI_U8 *pstDst, CVI_U16 u16Width, CVI_U16 u16Height,
    CVI_U32 *u64stMemVirAddr) {return CVI_SUCCESS;}

int CVI_HW_Map(
    CVI_U8 *pstSrc, CVI_U16 Src_u16Stride,
    CVI_U8 *pstDst, CVI_U16 Dst_u16Stride,
    CVI_U8 *pstMap, IVE_MAP_MODE_E enMode,
    CVI_U16 u16Width, CVI_U16 u16Height) {return CVI_SUCCESS;}

int CVI_HW_NCC(
    CVI_U8 *pstSrc1, CVI_U16 Src1_u16Stride,
    CVI_U8 *pstSrc2, CVI_U16 Src2_u16Stride,
    CVI_U64 *u64Numerator, CVI_U64 *u64QuadSum1, CVI_U64 *u64QuadSum2,
    CVI_U16 SrcWidth, CVI_U16 SrcHeight) {return CVI_SUCCESS;}

int CVI_HW_OrdStatFilter(
    CVI_U8 *pstSrc, CVI_U16 Src_u16Stride,
    CVI_U8 *pstDst, CVI_U16 Dst_u16Stride,
    CVI_U16 u16Width, CVI_U16 u16Height,
    IVE_ORD_STAT_FILTER_MODE_E enMode) {return CVI_SUCCESS;}

void CVI_HW_Resize(
    CVI_U8 **pstSrc, CVI_U16 *Src_u16Stride,
    CVI_U8 **pstDst, CVI_U16 *Dst_u16Stride,
    CVI_U16 Src_u16Width, CVI_U16 Src_u16Height,
    CVI_U16 Dst_u16Width, CVI_U16 Dst_u16Height,
    CVI_U16 scale_x, CVI_U16 scale_y,
    CVI_U8 nChannels, IVE_RESIZE_MODE_E enMode) {return ;}

void CVI_HW_ResizeUnit(
    CVI_U8 *pstSrc, CVI_U16 Src_u16Stride,
    CVI_U8 *pstDst, CVI_U16 Dst_u16Stride,
    CVI_U16 Src_u16Width, CVI_U16 Src_u16Height,
    CVI_U16 Dst_u16Width, CVI_U16 Dst_u16Height,
    CVI_U16 scale_x, CVI_U16 scale_y, CVI_U8 enMode) {return ;}

CVI_U16 CVI_HW_Resize_ComputeResizeAreaTab(
    CVI_U16 ssize, CVI_U16 dsize, CVI_U16 scale, CVI_U16 *tab) {return CVI_SUCCESS;}

int CVI_HW_Resize_Bilinear(
    CVI_U8 *pstSrc, CVI_U16 Src_u16Stride,
    CVI_U16 Src_u16Width, CVI_U16 Src_u16Height,
    CVI_U8 *pstDst, CVI_U16 Dst_u16Stride,
    CVI_U16 Dst_u16Width, CVI_U16 Dst_u16Height,
    CVI_U16 xmax, CVI_U16 *alpha, CVI_U16 *beta,
    CVI_U16 *xofs, CVI_U16 *yofs, int ksize, CVI_U32 *buffer_data) {return CVI_SUCCESS;}

int CVI_HW_Resize_Bilinear_Hor(
    CVI_U8 *pstSrc[], CVI_U32 *pstDst[],
    int count, CVI_U16 *xofs, CVI_U16 *alpha, int Src_u16Width,
    CVI_U16 Dst_u16Width, CVI_U16 xmax) {return CVI_SUCCESS;}

int CVI_HW_Resize_Bilinear_Ver(
    CVI_U32 *pstSrc[], CVI_U8 *pstDst, CVI_U16 *beta, CVI_U16 u16Width) {return CVI_SUCCESS;}

int CVI_HW_Resize_Area(
    CVI_U8 *pstSrc, CVI_U16 Src_u16Stride,
    CVI_U16 Src_u16Width, CVI_U16 Src_u16Height,
    CVI_U8 *pstDst, CVI_U16 Dst_u16Stride,
    CVI_U16 Dst_u16Width, CVI_U16 Dst_u16Height,
    CVI_U16 *xtab, unsigned int xtab_size,
    CVI_U16 *ytab, int ytab_size,
    CVI_U16 j_start, CVI_U16 j_end, CVI_U16 *buf) {return CVI_SUCCESS;}

int CVI_HW_Resize_AreaFast(
    CVI_U8 *pstSrc, CVI_U16 Src_u16Stride,
    CVI_U16 Src_u16Width, CVI_U16 Src_u16Height,
    CVI_U8 *pstDst, CVI_U16 Dst_u16Stride,
    CVI_U16 Dst_u16Width, CVI_U16 Dst_u16Height,
    CVI_U16 *ofs, CVI_U16 *xofs, CVI_U16 scale_x, CVI_U32 scale_y, CVI_U16 area) {return CVI_SUCCESS;}

int CVI_HW_CannyHysEdge(
    CVI_U8 *pstSrcAng, CVI_U16 SrcAng_u16Stride,
    CVI_U16 *pstSrcMag, CVI_U16 SrcMag_u16Stride,
    CVI_U8 *pstEdge, CVI_U16 EdgeStride,
    CVI_U16 *pstStack, CVI_U32 *u32StackSize,
    CVI_U16 u16LowThr, CVI_U16 u16HighThr,
    CVI_U16 Src_u16Width, CVI_U16 Src_u16Height) {return CVI_SUCCESS;}

int CVI_HW_CannyEdge(
    CVI_U8 *pstEdge, CVI_U16 EdgeStride,
    CVI_U16 *pstStack, CVI_U32 *u32StackSize,
    CVI_U16 Src_u16Width, CVI_U16 Src_u16Height) {return CVI_SUCCESS;}

int CVI_HW_Integ(
    CVI_U8 *pstSrc, CVI_U16 Src_u16Stride,
    CVI_U32 *pstDst, CVI_U16 Dst_u16Stride,
    CVI_U16 u16Width, CVI_U16 u16Height, IVE_INTEG_OUT_CTRL_E enOutCtrl) {return CVI_SUCCESS;}

int CVI_HW_LBP(
    CVI_U8 *pstSrc, CVI_U16 Src_u16Stride,
    CVI_U8 *pstDst, CVI_U16 Dst_u16Stride,
    CVI_U16 u16Width, CVI_U16 u16Height,
    IVE_8BIT_U un8BitThr, CVI_U8 enMode) {return CVI_SUCCESS;}

int CVI_HW_NormGrad(
    CVI_U8 *pstSrc, CVI_U16 Src_u16Stride,
    CVI_S8 *pstDstH, CVI_U16 DstH_u16Stride,
    CVI_S8 *pstDstV, CVI_U16 DstV_u16Stride,
    CVI_S8 *pstDstHV, CVI_U16 DstHV_u16Stride,
    CVI_S8 *as8Mask, CVI_U16 Src_u16Width, CVI_U16 Src_u16Height,
    CVI_U8 u8Norm, IVE_NORM_GRAD_OUT_CTRL_E enOutCtrl) {return CVI_SUCCESS;}

int CVI_HW_STBoxFltAndEigCalc(
	CVI_S16 *pstGrad_S8C2, CVI_U16 Grad_u16Stride,
	CVI_U16 *pstEigen, CVI_U16 Eigen_u16Stride,
	CVI_U16 Src_u16Width, CVI_U16 Src_u16Height,
	CVI_U16 *u16MaxEig) {return CVI_SUCCESS;}

int CVI_HW_STCandiCorner(
    CVI_U8 *pstPrev, CVI_U16 Prev_u16Stride,
    CVI_U8 *pstCurr, CVI_U16 Curr_u16Stride,
    CVI_U8 *pstDst, CVI_U16 Dst_u16Stride,
    CVI_U16 Src_u16Width, CVI_U16 Src_u16Height) {return CVI_SUCCESS;}

int CVI_HW_Sobel(
    CVI_U8 *pstSrc, CVI_U16 Src_u16Stride,
    CVI_U16 *pstDstMag, CVI_U16 DstMag_u16Stride,
    CVI_U8 *pstDstAng, CVI_U16 DstAng_u16Stride,
    CVI_U16 *pstDstH, CVI_U16 DstH_u16Stride,
    CVI_U16 *pstDstV, CVI_U16 DstV_u16Stride,
    CVI_U16 u16Width, CVI_U16 u16Height,
    CVI_S8 *as8Mask, CVI_U16 u16Thr) {return CVI_SUCCESS;}

int CVI_HW_MagAndAng_Top(
    CVI_U8 *pstSrc, CVI_U16 Src_u16Stride,
    CVI_U16 *pstDstMag, CVI_U16 DstMag_u16Stride,
    CVI_U8 *pstDstAng, CVI_U16 DstAng_u16Stride,
    CVI_U16 u16Width, CVI_U16 u16Height,
    CVI_S8 *as8Mask, CVI_U16 u16Thr, IVE_MAG_AND_ANG_OUT_CTRL_E enOutCtrl) {return CVI_SUCCESS;}

int CVI_HW_Bernsen(
    CVI_U8 *pstSrc, CVI_U16 Src_u16Stride,
    CVI_U8 *pstDst, CVI_U16 Dst_u16Stride,
    CVI_U16 u16Width, CVI_U16 u16Height,
    CVI_U8 u8WinSize, CVI_U8 u8Thr, CVI_U8 u8ContrastThreshold,
    IVE_BERNSEN_MODE_E enMode) {return CVI_SUCCESS;}

CVI_U16 *CVI_HW_CCL(
    CVI_U8 *pstSrcDst, CVI_U16 SrcDst_u16Stride,
    CVI_U16 SrcDst_u16Width, CVI_U16 SrcDst_u16Height,
    IVE_REGION_S *pstRegion, CVI_U16 *u16CurAreaThr,
    CVI_U16 u16InitAreaThr, CVI_U16 u16Step,
    CVI_S8 *s8LabelStatus, CVI_U8 *u8RegionNum, CVI_U8 enMode) {return CVI_SUCCESS;}

int CVI_HW_Dilate(
    CVI_U8 *pstSrc, CVI_U16 Src_u16Stride,
    CVI_U8 *pstDst, CVI_U16 Dst_u16Stride,
    CVI_U16 u16Width, CVI_U16 u16Height,
    CVI_U8 *au8Mask) {return CVI_SUCCESS;}

int CVI_HW_Erode(
    CVI_U8 *pstSrc, CVI_U16 Src_u16Stride,
    CVI_U8 *pstDst, CVI_U16 Dst_u16Stride,
    CVI_U16 u16Width, CVI_U16 u16Height,
    CVI_U8 *au8Mask) {return CVI_SUCCESS;}

int *CVI_HW_MatchBgModel(
    CVI_U16 pstFgFlag_u16Stride,
    CVI_U8 *pstCurImg,
    CVI_U8 *pstBgModel,
    int a4,
    CVI_U16 pstBgDiffFg_u16Stride,
    CVI_U8 *pstFgFlag,
    CVI_S8 *pstBgDiffFg,
    CVI_S8 *pstFrmDiffFg,
    CVI_U16 BgModel_u16Stride,
    CVI_U16 u16Width,
    CVI_U16 u16Height,
    CVI_U32 *pstStatData,
    CVI_U32 u32CurFrmNum,
    CVI_U32 u32PreFrmNum,
    CVI_U16 u16TimeThr,
    CVI_U8 u8DiffThrCrlCoef,
    CVI_U8 u8DiffMaxThr,
    CVI_U8 u8DiffMinThr,
    CVI_U8 u8DiffThrInc,
    CVI_U8 u8FastLearnRate,
    CVI_U8 u8DetChgRegion) {return CVI_SUCCESS;}

int *CVI_HW_UpdateBgModel(
    CVI_U8 *pstBgModel,
    CVI_U16 BgModel_u16Stride,
    CVI_U8 *pstFgFlag,
    CVI_U8 *pstBgImg,
    CVI_U8 *pstChgStaImg,
    CVI_S8 *pstChgStaFg,
    CVI_U16 FgFlag_u16Stride,
    CVI_U16 *pstChgStaLife,
    CVI_U16 ChgStaLife_u16Stride,
    CVI_U16 u16Width,
    CVI_U16 u16Height,
    CVI_U32 *pstStatData,
    CVI_U32 u32CurFrmNum,
    CVI_U32 u32PreChkTime,
    CVI_U32 u32FrmChkPeriod,
    CVI_U32 u32InitMinTime,
    CVI_U32 u32StyBgMinBlendTime,
    CVI_U32 u32StyBgMaxBlendTime,
    CVI_U32 u32DynBgMinBlendTime,
    CVI_U16 u16FgMaxFadeTime,
    CVI_U16 u16BgMaxFadeTime,
    CVI_U32 u32StaticDetMinTime,
    CVI_U8 u8StyBgAccTimeRateThr,
    CVI_U8 u8ChgBgAccTimeRateThr,
    CVI_U8 u8DynBgAccTimeThr,
    CVI_U8 u8DynBgDepth,
    CVI_U8 u8BgEffStaRateThr,
    CVI_U8 u8AcceBgLearn,
    CVI_U8 u8DetChgRegion) {return CVI_SUCCESS;}

int CVI_HW_GMM(
    CVI_U8 *pstSrc, CVI_U16 Src_u16Stride, CVI_U16 Src_u16Width, CVI_U16 Src_u16Height,
    CVI_U8 *pstModel, CVI_U8 u8ModelNum,
    CVI_U8 *pstFg, CVI_U16 Fg_u16Stride, CVI_U8 *pstBg, CVI_U16 Bg_u16Stride,
    CVI_U16 u0q16LearnRate, CVI_U16 u0q16BgRatio, CVI_U16 u8q8VarThr,
    CVI_U32 u22q10NoiseVar, CVI_U32 u22q10MaxVar, CVI_U32 u22q10MinVar, CVI_U16 u0q16InitWeight,
    CVI_U8 enDetectShadow, CVI_U8 u0q8ShadowThr, CVI_U8 u8SnsFactor, CVI_U8 IsU8C3) {return CVI_SUCCESS;}

int CVI_HW_GMM2(
    CVI_U8 *pstSrc, CVI_U16 Src_u16Stride,
    CVI_U16 *pstFactor, CVI_U16 Factor_u16Stride,
    CVI_U8 *pstFg, CVI_U16 Fg_u16Stride,
    CVI_U8 *pstBg, CVI_U16 FgBg_u16Stride,
    CVI_U8 *pstMatchModelInfo, CVI_U16 MatchModelInfo_u16Stride, CVI_U8 *pstModel,
    CVI_U8 u8ModelNum, CVI_U16 u16LifeThr,
    CVI_U16 u16FreqInitVal, CVI_U16 u16FreqReduFactor, CVI_U16 u16FreqAddFactor, CVI_U16 u16FreqThr,
    CVI_U16 u16VarRate, CVI_U16 u9q7MaxVar, CVI_U16 u9q7MinVar,
    CVI_U8 u8GlbSnsFactor, CVI_U8 enSnsFactorMode, CVI_U16 u16GlbLifeUpdateFactor, CVI_U8 enLifeUpdateFactorMode,
    CVI_U16 ncols, CVI_U16 nrows, CVI_U8 IsU8C3) {return CVI_SUCCESS;}

CVI_U8 *CVI_HW_Gmm2_Pixel_Mog(
    CVI_U8 *pstSrc0, CVI_U8 *pstSrc1, CVI_U8 *pstSrc2,
    CVI_U8 *pstFg, CVI_U8 *pstBg0, CVI_U8 *pstBg1, CVI_U8 *pstBg2, CVI_U8 *pstMatchModelInfo,
    CVI_U16 u16LifeUpdateFactor, CVI_U8 u8SnsFactor, CVI_U8 *pstModel, CVI_U8 u8ModelNum,
    CVI_U16 u16VarRate, CVI_U16 varMax, CVI_U16 varMin,
    CVI_U16 u16FreqReduFactor, CVI_U16 u16FreqThr, CVI_U16 u16FreqAddFactor, CVI_U16 u16FreqInitVal,
    CVI_U16 u16LifeThr, CVI_U8 Model_u16Stride, CVI_U8 IsU8C3) {return CVI_SUCCESS;}

//int CVI_HW_GradFg(
//    CVI_S8 *pstBgDiffFg, CVI_U16 BgDiffFg_u16Stride,
//    CVI_S8 *pstCurGrad, CVI_U16 CurGrad_u16Stride,
//    CVI_S8 *pstBgGrad, CVI_U16 BgGrad_u16Stride,
//    CVI_U8 *pstGradFg, CVI_U16 GradFg_u16Stride,
//    CVI_U16 u16Width, CVI_U16 u16Height,
//    IVE_GRAD_FG_MODE_E enMode, CVI_U16 u16EdwFactor,
//    CVI_U8 u8CrlCoefThr, CVI_U8 u8MagCrlThr,
//    CVI_U8 u8MinMagDiff, CVI_U8 u8NoiseVal, CVI_U8 u8EdwDark) {return CVI_SUCCESS;}

int CVI_HW_SAD(
    CVI_U8 *pstSrc1, CVI_U16 Src1_u16Stride,
    CVI_U8 *pstSrc2, CVI_U16 Src2_u16Stride,
    CVI_U16 *pstSad, CVI_U16 Sad_u16Stride,
    CVI_U8 *pstThr, CVI_U16 Thr_u16Stride,
    CVI_U16 u16Width, CVI_U16 u16Height,
    CVI_U16 u16Thr, CVI_U8 u8MinVal, CVI_U8 u8MaxVal,
    IVE_SAD_OUT_CTRL_E enOutCtrl, IVE_SAD_MODE_E enMode) {return CVI_SUCCESS;}

CVI_U8 *CVI_HW_Sort3ElemAscend(CVI_U8 *Min, CVI_U8 *Mid, CVI_U8 *Max) {return CVI_SUCCESS;}

int CVI_HW_DetectGMMShadow(
    CVI_U8 *pix_data, CVI_U8 *pstModel, CVI_U8 u8ModelNum, CVI_U8 IsU8C3,
    CVI_U16 u0q16BgRatio, CVI_U8 u0q8ShadowThr, CVI_U8 u8SnsFactor) {return CVI_SUCCESS;}

int CVI_HW_RGBGMM(
    CVI_U8 *pstSrc, CVI_U16 Src_u16Stride, CVI_U16 Src_u16Width, CVI_U16 Src_u16Height,
    CVI_U8 *pstModel, CVI_U8 u8ModelNum,
    CVI_U8 *pstFg, CVI_U16 Fg_u16Stride, CVI_U8 *pstBg, CVI_U16 Bg_u16Stride,
    CVI_U16 u0q16LearnRate, CVI_U16 u0q16BgRatio, CVI_U16 u8q8VarThr,
    CVI_U32 u22q10NoiseVar, CVI_U32 u22q10MaxVar, CVI_U32 u22q10MinVar, CVI_U16 u0q16InitWeight,
    CVI_U8 enDetectShadow, CVI_U8 u0q8ShadowThr, CVI_U8 u8SnsFactor) {return CVI_SUCCESS;}

int CVI_HW_SingleGMM(
    CVI_U8 *pstSrc, CVI_U16 Src_u16Stride, CVI_U16 Src_u16Width, CVI_U16 Src_u16Height,
    CVI_U8 *pstModel, CVI_U8 u8ModelNum,
    CVI_U8 *pstFg, CVI_U16 Fg_u16Stride, CVI_U8 *pstBg, CVI_U16 Bg_u16Stride,
    CVI_U16 u0q16LearnRate, CVI_U16 u0q16BgRatio, CVI_U16 u8q8VarThr,
    CVI_U32 u22q10NoiseVar, CVI_U32 u22q10MaxVar, CVI_U32 u22q10MinVar, CVI_U16 u0q16InitWeight,
    CVI_U8 enDetectShadow, CVI_U8 u0q8ShadowThr, CVI_U8 u8SnsFactor) {return CVI_SUCCESS;}

int CVI_HW_LKOpticalFlowPyr(
    CVI_U8 **astSrcPrevPyr, CVI_U16 *SrcPrevPyr_u16Stride,
    CVI_U8 **astSrcNextPyr, CVI_U16 *SrcNextPyr_u16Stride,
    CVI_U16 Src_u16Width, CVI_U16 Src_u16Height, CVI_U8 u8MaxLevel,
    IVE_POINT_S25Q7_S *pstPrevPts, IVE_POINT_S25Q7_S *pstNextPts,
    CVI_U8 *pstStatus, CVI_U16 *pstErr,
    CVI_U16 u16PtsNum, CVI_U8 u8IterCnt, CVI_U8 u0q8Eps, CVI_U8 u0q8MinEigThr,
    CVI_U8 bUseInitFlow, CVI_U8 enOutMode) {return CVI_SUCCESS;}

int CVI_HW_GradFg(
    CVI_S8 *pstBgDiffFg, CVI_U16 BgDiffFg_u16Stride,
    CVI_S8 *pstCurGrad, CVI_U16 CurGrad_u16Stride,
    CVI_S8 *pstBgGrad, CVI_U16 BgGrad_u16Stride,
    CVI_U8 *pstGradFg, CVI_U16 GradFg_u16Stride,
    CVI_U16 u16Width, CVI_U16 u16Height,
    IVE_GRAD_FG_MODE_E enMode, CVI_U16 u16EdwFactor,
    CVI_U8 u8CrlCoefThr, CVI_U8 u8MagCrlThr,
    CVI_U8 u8MinMagDiff, CVI_U8 u8NoiseVal, CVI_U8 u8EdwDark) {return CVI_SUCCESS;}
