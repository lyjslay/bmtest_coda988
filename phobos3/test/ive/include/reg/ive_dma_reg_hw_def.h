// $Module: ive_dma $
// $Script Version: 2.1.0 $
// $RegisterBank Version: V 1.0.00 $
// $Author:  $
// $Date: Tue, 14 Sep 2021 09:57:55 AM $
//
//for section: name
  MODULE_SECTION_DEF(ive_dma)
//for reg: (name, type, depth, init, min, max, comment...)

  MODULE_REG_DEF (reg_dma_mode, unsigned int, 2, 0, 0, 0, "")
  MODULE_REG_DEF (reg_dma_horsegsize, unsigned int, 8, 0, 0, 0, "")
  MODULE_REG_DEF (reg_dma_versegrow, unsigned int, 8, 0, 0, 0, "")
  MODULE_REG_DEF (reg_dma_elemsize, unsigned int, 8, 0, 0, 0, "")
  MODULE_REG_DEF (reg_dma_src_stride, unsigned int, 16, 0, 0, 0, "")
  MODULE_REG_DEF (reg_dma_dst_stride, unsigned int, 16, 0, 0, 0, "")
  MODULE_REG_DEF (reg_dma_val_l32, unsigned int, 32, 0, 0, 0, "")
  MODULE_REG_DEF (reg_dma_val_h32, unsigned int, 32, 0, 0, 0, "")
//for lookup table: (name, type, size, comment...)
