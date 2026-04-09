// arg list below is needed ========================
// @PTN_REG_READ, uint32_t (*)(uint64_t), for register read
// @PTN_REG_WRITE, void (*)(uint64_t, uint32_t data), for register write
// @PTN_MALLOC, uint64_t (*)(size_t, uint64_t), for memory allocation
// @phys_regs, uint64_t , for register base
// @priv, void * , for private info, not touched
// =================================================
do {
    uint32_t _tmp;
    uint64_t _mem_addr;

    _tmp = (PTN_REG_READ(phys_regs[0x0]+0x0)&0xfffffffc)|0x2;
    PTN_REG_WRITE(phys_regs[0x0]+0x0, _tmp); // pre_raw.reg_csi_bdg_out_mode

    _tmp = (PTN_REG_READ(phys_regs[0x69]+0x0)&0xfffffffd)|0x2;
    PTN_REG_WRITE(phys_regs[0x69]+0x0, _tmp); // isp_crop4.reg_dma_enable

    _tmp = (PTN_REG_READ(phys_regs[0x6a]+0x0)&0xfffffffd)|0x2;
    PTN_REG_WRITE(phys_regs[0x6a]+0x0, _tmp); // isp_crop5.reg_dma_enable

    _tmp = (PTN_REG_READ(phys_regs[0x6b]+0x0)&0xfffffffd)|0x2;
    PTN_REG_WRITE(phys_regs[0x6b]+0x0, _tmp); // isp_crop6.reg_dma_enable

    _mem_addr = PTN_MALLOC(0x80*0x40, priv);
    PTN_REG_WRITE(phys_regs[0x69]+0x100, (_mem_addr&0xffffffff)); // isp_dma3.reg_dma_addr_31_0
    PTN_REG_WRITE(phys_regs[0x69]+0x104, ((_mem_addr>>32)&0xffffffff)); // isp_dma3.reg_dma_addr_63_32

    _tmp = (PTN_REG_READ(phys_regs[0x69]+0x108)&0xffff0000)|0x80;
    PTN_REG_WRITE(phys_regs[0x69]+0x108, _tmp); // isp_dma3.reg_dma_cnt

    _tmp = (PTN_REG_READ(phys_regs[0x69]+0x124)&0xffffe000)|0x7f;
    PTN_REG_WRITE(phys_regs[0x69]+0x124, _tmp); // isp_dma3.reg_img_width

    _tmp = (PTN_REG_READ(phys_regs[0x69]+0x124)&0xe000ffff)|0x3f0000;
    PTN_REG_WRITE(phys_regs[0x69]+0x124, _tmp); // isp_dma3.reg_img_height

    _tmp = (PTN_REG_READ(phys_regs[0x69]+0x128)&0xffff0000)|0x80;
    PTN_REG_WRITE(phys_regs[0x69]+0x128, _tmp); // isp_dma3.reg_line_offset

    _tmp = (PTN_REG_READ(phys_regs[0x69]+0x128)&0xfff8ffff)|0x0;
    PTN_REG_WRITE(phys_regs[0x69]+0x128, _tmp); // isp_dma3.reg_mod_ctl

    _tmp = (PTN_REG_READ(phys_regs[0x69]+0x134)&0xfffffff0)|0xf;
    PTN_REG_WRITE(phys_regs[0x69]+0x134, _tmp); // isp_dma3.reg_int_sts_en

    _tmp = (PTN_REG_READ(phys_regs[0x69]+0x120)&0xfffffffe)|0x0;
    PTN_REG_WRITE(phys_regs[0x69]+0x120, _tmp); // isp_dma3.reg_job_start

    _mem_addr = PTN_MALLOC(0x40*0x20, priv);
    PTN_REG_WRITE(phys_regs[0x6a]+0x100, (_mem_addr&0xffffffff)); // isp_dma4.reg_dma_addr_31_0
    PTN_REG_WRITE(phys_regs[0x6a]+0x104, ((_mem_addr>>32)&0xffffffff)); // isp_dma4.reg_dma_addr_63_32

    _tmp = (PTN_REG_READ(phys_regs[0x6a]+0x108)&0xffff0000)|0x40;
    PTN_REG_WRITE(phys_regs[0x6a]+0x108, _tmp); // isp_dma4.reg_dma_cnt

    _tmp = (PTN_REG_READ(phys_regs[0x6a]+0x124)&0xffffe000)|0x3f;
    PTN_REG_WRITE(phys_regs[0x6a]+0x124, _tmp); // isp_dma4.reg_img_width

    _tmp = (PTN_REG_READ(phys_regs[0x6a]+0x124)&0xe000ffff)|0x1f0000;
    PTN_REG_WRITE(phys_regs[0x6a]+0x124, _tmp); // isp_dma4.reg_img_height

    _tmp = (PTN_REG_READ(phys_regs[0x6a]+0x128)&0xffff0000)|0x40;
    PTN_REG_WRITE(phys_regs[0x6a]+0x128, _tmp); // isp_dma4.reg_line_offset

    _tmp = (PTN_REG_READ(phys_regs[0x6a]+0x128)&0xfff8ffff)|0x0;
    PTN_REG_WRITE(phys_regs[0x6a]+0x128, _tmp); // isp_dma4.reg_mod_ctl

    _tmp = (PTN_REG_READ(phys_regs[0x6a]+0x134)&0xfffffff0)|0xf;
    PTN_REG_WRITE(phys_regs[0x6a]+0x134, _tmp); // isp_dma4.reg_int_sts_en

    _tmp = (PTN_REG_READ(phys_regs[0x6a]+0x120)&0xfffffffe)|0x0;
    PTN_REG_WRITE(phys_regs[0x6a]+0x120, _tmp); // isp_dma4.reg_job_start

    _mem_addr = PTN_MALLOC(0x40*0x20, priv);
    PTN_REG_WRITE(phys_regs[0x6b]+0x100, (_mem_addr&0xffffffff)); // isp_dma5.reg_dma_addr_31_0
    PTN_REG_WRITE(phys_regs[0x6b]+0x104, ((_mem_addr>>32)&0xffffffff)); // isp_dma5.reg_dma_addr_63_32

    _tmp = (PTN_REG_READ(phys_regs[0x6b]+0x108)&0xffff0000)|0x40;
    PTN_REG_WRITE(phys_regs[0x6b]+0x108, _tmp); // isp_dma5.reg_dma_cnt

    _tmp = (PTN_REG_READ(phys_regs[0x6b]+0x124)&0xffffe000)|0x3f;
    PTN_REG_WRITE(phys_regs[0x6b]+0x124, _tmp); // isp_dma5.reg_img_width

    _tmp = (PTN_REG_READ(phys_regs[0x6b]+0x124)&0xe000ffff)|0x1f0000;
    PTN_REG_WRITE(phys_regs[0x6b]+0x124, _tmp); // isp_dma5.reg_img_height

    _tmp = (PTN_REG_READ(phys_regs[0x6b]+0x128)&0xffff0000)|0x40;
    PTN_REG_WRITE(phys_regs[0x6b]+0x128, _tmp); // isp_dma5.reg_line_offset

    _tmp = (PTN_REG_READ(phys_regs[0x6b]+0x128)&0xfff8ffff)|0x0;
    PTN_REG_WRITE(phys_regs[0x6b]+0x128, _tmp); // isp_dma5.reg_mod_ctl

    _tmp = (PTN_REG_READ(phys_regs[0x6b]+0x134)&0xfffffff0)|0xf;
    PTN_REG_WRITE(phys_regs[0x6b]+0x134, _tmp); // isp_dma5.reg_int_sts_en

    _tmp = (PTN_REG_READ(phys_regs[0x6b]+0x120)&0xfffffffe)|0x0;
    PTN_REG_WRITE(phys_regs[0x6b]+0x120, _tmp); // isp_dma5.reg_job_start

    _tmp = (PTN_REG_READ(phys_regs[0x50]+0x0)&0xfffffffc)|0x2;
    PTN_REG_WRITE(phys_regs[0x50]+0x0, _tmp); // isp_rgb.reg_color_mode

#if 0
    _tmp = (PTN_REG_READ(phys_regs[0x51]+0x0)&0xfffffffd)|0x2;
    PTN_REG_WRITE(phys_regs[0x51]+0x0, _tmp); // isp_cfa.reg_cfa_enable

    _tmp = (PTN_REG_READ(phys_regs[0x51]+0x8)&0xfffff000)|0x7f;
    PTN_REG_WRITE(phys_regs[0x51]+0x8, _tmp); // isp_cfa.reg_img_wd

    _tmp = (PTN_REG_READ(phys_regs[0x51]+0x8)&0xf000ffff)|0x3f0000;
    PTN_REG_WRITE(phys_regs[0x51]+0x8, _tmp); // isp_cfa.reg_img_ht
#else
    _tmp = (PTN_REG_READ(phys_regs[0x51]+0x0)&0xfffffffe)|0x1;
    PTN_REG_WRITE(phys_regs[0x51]+0x0, _tmp); // isp_cfa.reg_cfa_shdw_sel

    _tmp = (PTN_REG_READ(phys_regs[0x51]+0x8)&0xfffff000)|0x7f;
    PTN_REG_WRITE(phys_regs[0x51]+0x8, _tmp); // isp_cfa.reg_img_wd

    _tmp = (PTN_REG_READ(phys_regs[0x51]+0x8)&0xf000ffff)|0x3f0000;
    PTN_REG_WRITE(phys_regs[0x51]+0x8, _tmp); // isp_cfa.reg_img_ht

    _tmp = (PTN_REG_READ(phys_regs[0x51]+0x0)&0xfffffffd)|0x2;
    PTN_REG_WRITE(phys_regs[0x51]+0x0, _tmp); // isp_cfa.reg_cfa_enable

    _tmp = (PTN_REG_READ(phys_regs[0x51]+0xc)&0xfffff000)|0x1e0;
    PTN_REG_WRITE(phys_regs[0x51]+0xc, _tmp); // isp_cfa.reg_cfa_edgee_thd

    _tmp = (PTN_REG_READ(phys_regs[0x51]+0xc)&0xf000ffff)|0x1900000;
    PTN_REG_WRITE(phys_regs[0x51]+0xc, _tmp); // isp_cfa.reg_cfa_sige_thd

    _tmp = (PTN_REG_READ(phys_regs[0x51]+0x10)&0xfffff000)|0x28;
    PTN_REG_WRITE(phys_regs[0x51]+0x10, _tmp); // isp_cfa.reg_cfa_gsig_tol

    _tmp = (PTN_REG_READ(phys_regs[0x51]+0x10)&0xf000ffff)|0x280000;
    PTN_REG_WRITE(phys_regs[0x51]+0x10, _tmp); // isp_cfa.reg_cfa_rbsig_tol

    _tmp = (PTN_REG_READ(phys_regs[0x51]+0x14)&0xfffff000)|0x80;
    PTN_REG_WRITE(phys_regs[0x51]+0x14, _tmp); // isp_cfa.reg_cfa_edge_tol

    _tmp = (PTN_REG_READ(phys_regs[0x51]+0x18)&0xffff0000)|0xe00;
    PTN_REG_WRITE(phys_regs[0x51]+0x18, _tmp); // isp_cfa.reg_cfa_ghp_thd

    _tmp = (PTN_REG_READ(phys_regs[0x51]+0x0)&0xfffffffb)|0x4;
    PTN_REG_WRITE(phys_regs[0x51]+0x0, _tmp); // isp_cfa.reg_cfa_fcr_enable

    _tmp = (PTN_REG_READ(phys_regs[0x51]+0x20)&0xfffff000)|0x190;
    PTN_REG_WRITE(phys_regs[0x51]+0x20, _tmp); // isp_cfa.reg_cfa_fcr_hv_lowerbnd

    _tmp = (PTN_REG_READ(phys_regs[0x51]+0x20)&0xf000ffff)|0x6400000;
    PTN_REG_WRITE(phys_regs[0x51]+0x20, _tmp); // isp_cfa.reg_cfa_fcr_hv_upperbnd

    _tmp = (PTN_REG_READ(phys_regs[0x51]+0x24)&0xfffff000)|0x300;
    PTN_REG_WRITE(phys_regs[0x51]+0x24, _tmp); // isp_cfa.reg_cfa_fcr_sat_lowerbnd

    _tmp = (PTN_REG_READ(phys_regs[0x51]+0x24)&0xf000ffff)|0x4000000;
    PTN_REG_WRITE(phys_regs[0x51]+0x24, _tmp); // isp_cfa.reg_cfa_fcr_sat_upperbnd

    _tmp = (PTN_REG_READ(phys_regs[0x51]+0x28)&0xfffff000)|0x3e8;
    PTN_REG_WRITE(phys_regs[0x51]+0x28, _tmp); // isp_cfa.reg_cfa_fcr_w3_lowerbnd

    _tmp = (PTN_REG_READ(phys_regs[0x51]+0x28)&0xf000ffff)|0xbb80000;
    PTN_REG_WRITE(phys_regs[0x51]+0x28, _tmp); // isp_cfa.reg_cfa_fcr_w3_upperbnd

    _tmp = (PTN_REG_READ(phys_regs[0x51]+0x2c)&0xffffffe0)|0x10;
    PTN_REG_WRITE(phys_regs[0x51]+0x2c, _tmp); // isp_cfa.reg_cfa_fcr_w3max_weight

    _tmp = (PTN_REG_READ(phys_regs[0x51]+0x2c)&0xfc00ffff)|0x3ff0000;
    PTN_REG_WRITE(phys_regs[0x51]+0x2c, _tmp); // isp_cfa.reg_cfa_fcr_strength

    _tmp = (PTN_REG_READ(phys_regs[0x51]+0x30)&0xffffffc0)|0x8;
    PTN_REG_WRITE(phys_regs[0x51]+0x30, _tmp); // isp_cfa.reg_cfa_ghp_lut00

    _tmp = (PTN_REG_READ(phys_regs[0x51]+0x30)&0xffffc0ff)|0x800;
    PTN_REG_WRITE(phys_regs[0x51]+0x30, _tmp); // isp_cfa.reg_cfa_ghp_lut01

    _tmp = (PTN_REG_READ(phys_regs[0x51]+0x30)&0xffc0ffff)|0x80000;
    PTN_REG_WRITE(phys_regs[0x51]+0x30, _tmp); // isp_cfa.reg_cfa_ghp_lut02

    _tmp = (PTN_REG_READ(phys_regs[0x51]+0x30)&0xc0ffffff)|0x8000000;
    PTN_REG_WRITE(phys_regs[0x51]+0x30, _tmp); // isp_cfa.reg_cfa_ghp_lut03

    _tmp = (PTN_REG_READ(phys_regs[0x51]+0x34)&0xffffffc0)|0xa;
    PTN_REG_WRITE(phys_regs[0x51]+0x34, _tmp); // isp_cfa.reg_cfa_ghp_lut04

    _tmp = (PTN_REG_READ(phys_regs[0x51]+0x34)&0xffffc0ff)|0xa00;
    PTN_REG_WRITE(phys_regs[0x51]+0x34, _tmp); // isp_cfa.reg_cfa_ghp_lut05

    _tmp = (PTN_REG_READ(phys_regs[0x51]+0x34)&0xffc0ffff)|0xa0000;
    PTN_REG_WRITE(phys_regs[0x51]+0x34, _tmp); // isp_cfa.reg_cfa_ghp_lut06

    _tmp = (PTN_REG_READ(phys_regs[0x51]+0x34)&0xc0ffffff)|0xa000000;
    PTN_REG_WRITE(phys_regs[0x51]+0x34, _tmp); // isp_cfa.reg_cfa_ghp_lut07

    _tmp = (PTN_REG_READ(phys_regs[0x51]+0x38)&0xffffffc0)|0xc;
    PTN_REG_WRITE(phys_regs[0x51]+0x38, _tmp); // isp_cfa.reg_cfa_ghp_lut08

    _tmp = (PTN_REG_READ(phys_regs[0x51]+0x38)&0xffffc0ff)|0xc00;
    PTN_REG_WRITE(phys_regs[0x51]+0x38, _tmp); // isp_cfa.reg_cfa_ghp_lut09

    _tmp = (PTN_REG_READ(phys_regs[0x51]+0x38)&0xffc0ffff)|0xc0000;
    PTN_REG_WRITE(phys_regs[0x51]+0x38, _tmp); // isp_cfa.reg_cfa_ghp_lut10

    _tmp = (PTN_REG_READ(phys_regs[0x51]+0x38)&0xc0ffffff)|0xc000000;
    PTN_REG_WRITE(phys_regs[0x51]+0x38, _tmp); // isp_cfa.reg_cfa_ghp_lut11

    _tmp = (PTN_REG_READ(phys_regs[0x51]+0x3c)&0xffffffc0)|0xe;
    PTN_REG_WRITE(phys_regs[0x51]+0x3c, _tmp); // isp_cfa.reg_cfa_ghp_lut12

    _tmp = (PTN_REG_READ(phys_regs[0x51]+0x3c)&0xffffc0ff)|0xe00;
    PTN_REG_WRITE(phys_regs[0x51]+0x3c, _tmp); // isp_cfa.reg_cfa_ghp_lut13

    _tmp = (PTN_REG_READ(phys_regs[0x51]+0x3c)&0xffc0ffff)|0xe0000;
    PTN_REG_WRITE(phys_regs[0x51]+0x3c, _tmp); // isp_cfa.reg_cfa_ghp_lut14

    _tmp = (PTN_REG_READ(phys_regs[0x51]+0x3c)&0xc0ffffff)|0xe000000;
    PTN_REG_WRITE(phys_regs[0x51]+0x3c, _tmp); // isp_cfa.reg_cfa_ghp_lut15

    _tmp = (PTN_REG_READ(phys_regs[0x51]+0x40)&0xffffffc0)|0x10;
    PTN_REG_WRITE(phys_regs[0x51]+0x40, _tmp); // isp_cfa.reg_cfa_ghp_lut16

    _tmp = (PTN_REG_READ(phys_regs[0x51]+0x40)&0xffffc0ff)|0x1000;
    PTN_REG_WRITE(phys_regs[0x51]+0x40, _tmp); // isp_cfa.reg_cfa_ghp_lut17

    _tmp = (PTN_REG_READ(phys_regs[0x51]+0x40)&0xffc0ffff)|0x100000;
    PTN_REG_WRITE(phys_regs[0x51]+0x40, _tmp); // isp_cfa.reg_cfa_ghp_lut18

    _tmp = (PTN_REG_READ(phys_regs[0x51]+0x40)&0xc0ffffff)|0x10000000;
    PTN_REG_WRITE(phys_regs[0x51]+0x40, _tmp); // isp_cfa.reg_cfa_ghp_lut19

    _tmp = (PTN_REG_READ(phys_regs[0x51]+0x44)&0xffffffc0)|0x14;
    PTN_REG_WRITE(phys_regs[0x51]+0x44, _tmp); // isp_cfa.reg_cfa_ghp_lut20

    _tmp = (PTN_REG_READ(phys_regs[0x51]+0x44)&0xffffc0ff)|0x1400;
    PTN_REG_WRITE(phys_regs[0x51]+0x44, _tmp); // isp_cfa.reg_cfa_ghp_lut21

    _tmp = (PTN_REG_READ(phys_regs[0x51]+0x44)&0xffc0ffff)|0x140000;
    PTN_REG_WRITE(phys_regs[0x51]+0x44, _tmp); // isp_cfa.reg_cfa_ghp_lut22

    _tmp = (PTN_REG_READ(phys_regs[0x51]+0x44)&0xc0ffffff)|0x14000000;
    PTN_REG_WRITE(phys_regs[0x51]+0x44, _tmp); // isp_cfa.reg_cfa_ghp_lut23

    _tmp = (PTN_REG_READ(phys_regs[0x51]+0x48)&0xffffffc0)|0x18;
    PTN_REG_WRITE(phys_regs[0x51]+0x48, _tmp); // isp_cfa.reg_cfa_ghp_lut24

    _tmp = (PTN_REG_READ(phys_regs[0x51]+0x48)&0xffffc0ff)|0x1800;
    PTN_REG_WRITE(phys_regs[0x51]+0x48, _tmp); // isp_cfa.reg_cfa_ghp_lut25

    _tmp = (PTN_REG_READ(phys_regs[0x51]+0x48)&0xffc0ffff)|0x180000;
    PTN_REG_WRITE(phys_regs[0x51]+0x48, _tmp); // isp_cfa.reg_cfa_ghp_lut26

    _tmp = (PTN_REG_READ(phys_regs[0x51]+0x48)&0xc0ffffff)|0x18000000;
    PTN_REG_WRITE(phys_regs[0x51]+0x48, _tmp); // isp_cfa.reg_cfa_ghp_lut27

    _tmp = (PTN_REG_READ(phys_regs[0x51]+0x4c)&0xffffffc0)|0x1c;
    PTN_REG_WRITE(phys_regs[0x51]+0x4c, _tmp); // isp_cfa.reg_cfa_ghp_lut28

    _tmp = (PTN_REG_READ(phys_regs[0x51]+0x4c)&0xffffc0ff)|0x1c00;
    PTN_REG_WRITE(phys_regs[0x51]+0x4c, _tmp); // isp_cfa.reg_cfa_ghp_lut29

    _tmp = (PTN_REG_READ(phys_regs[0x51]+0x4c)&0xffc0ffff)|0x1f0000;
    PTN_REG_WRITE(phys_regs[0x51]+0x4c, _tmp); // isp_cfa.reg_cfa_ghp_lut30

    _tmp = (PTN_REG_READ(phys_regs[0x51]+0x4c)&0xc0ffffff)|0x1f000000;
    PTN_REG_WRITE(phys_regs[0x51]+0x4c, _tmp); // isp_cfa.reg_cfa_ghp_lut31

    _tmp = (PTN_REG_READ(phys_regs[0x51]+0x0)&0xfffffff7)|0x8;
    PTN_REG_WRITE(phys_regs[0x51]+0x0, _tmp); // isp_cfa.reg_cfa_moire_enable

    _tmp = (PTN_REG_READ(phys_regs[0x51]+0x54)&0xffffff00)|0xff;
    PTN_REG_WRITE(phys_regs[0x51]+0x54, _tmp); // isp_cfa.reg_cfa_moire_strth

    _tmp = (PTN_REG_READ(phys_regs[0x51]+0x54)&0xffff00ff)|0x4000;
    PTN_REG_WRITE(phys_regs[0x51]+0x54, _tmp); // isp_cfa.reg_cfa_moire_wght_gain

    _tmp = (PTN_REG_READ(phys_regs[0x51]+0x58)&0xfffff000)|0x0;
    PTN_REG_WRITE(phys_regs[0x51]+0x58, _tmp); // isp_cfa.reg_cfa_moire_np_ymin

    _tmp = (PTN_REG_READ(phys_regs[0x51]+0x54)&0xff00ffff)|0x0;
    PTN_REG_WRITE(phys_regs[0x51]+0x54, _tmp); // isp_cfa.reg_cfa_moire_np_yslope

    _tmp = (PTN_REG_READ(phys_regs[0x51]+0x58)&0xff00ffff)|0x0;
    PTN_REG_WRITE(phys_regs[0x51]+0x58, _tmp); // isp_cfa.reg_cfa_moire_np_low

    _tmp = (PTN_REG_READ(phys_regs[0x51]+0x58)&0xffffff)|0x0;
    PTN_REG_WRITE(phys_regs[0x51]+0x58, _tmp); // isp_cfa.reg_cfa_moire_np_high

    _tmp = (PTN_REG_READ(phys_regs[0x51]+0x5c)&0xfffff000)|0x21c;
    PTN_REG_WRITE(phys_regs[0x51]+0x5c, _tmp); // isp_cfa.reg_cfa_moire_diffthd_min

    _tmp = (PTN_REG_READ(phys_regs[0x51]+0x5c)&0xff00ffff)|0x3a0000;
    PTN_REG_WRITE(phys_regs[0x51]+0x5c, _tmp); // isp_cfa.reg_cfa_moire_diffthd_slope

    _tmp = (PTN_REG_READ(phys_regs[0x51]+0x60)&0xffffff00)|0x0;
    PTN_REG_WRITE(phys_regs[0x51]+0x60, _tmp); // isp_cfa.reg_cfa_moire_diffw_low

    _tmp = (PTN_REG_READ(phys_regs[0x51]+0x60)&0xffff00ff)|0xff00;
    PTN_REG_WRITE(phys_regs[0x51]+0x60, _tmp); // isp_cfa.reg_cfa_moire_diffw_high

    _tmp = (PTN_REG_READ(phys_regs[0x51]+0x60)&0xf000ffff)|0x600000;
    PTN_REG_WRITE(phys_regs[0x51]+0x60, _tmp); // isp_cfa.reg_cfa_moire_sadthd_min

    _tmp = (PTN_REG_READ(phys_regs[0x51]+0x64)&0xffffff00)|0x22;
    PTN_REG_WRITE(phys_regs[0x51]+0x64, _tmp); // isp_cfa.reg_cfa_moire_sadthd_slope

    _tmp = (PTN_REG_READ(phys_regs[0x51]+0x64)&0xff00ffff)|0x0;
    PTN_REG_WRITE(phys_regs[0x51]+0x64, _tmp); // isp_cfa.reg_cfa_moire_sadw_low

    _tmp = (PTN_REG_READ(phys_regs[0x51]+0x64)&0xffffff)|0xff000000;
    PTN_REG_WRITE(phys_regs[0x51]+0x64, _tmp); // isp_cfa.reg_cfa_moire_sadw_high

    _tmp = (PTN_REG_READ(phys_regs[0x51]+0x68)&0xffffff00)|0x0;
    PTN_REG_WRITE(phys_regs[0x51]+0x68, _tmp); // isp_cfa.reg_cfa_moire_lumaw_low

    _tmp = (PTN_REG_READ(phys_regs[0x51]+0x68)&0xffff00ff)|0xff00;
    PTN_REG_WRITE(phys_regs[0x51]+0x68, _tmp); // isp_cfa.reg_cfa_moire_lumaw_high

    _tmp = (PTN_REG_READ(phys_regs[0x51]+0x6c)&0xfffff000)|0x150;
    PTN_REG_WRITE(phys_regs[0x51]+0x6c, _tmp); // isp_cfa.reg_cfa_moire_lumathd_min

    _tmp = (PTN_REG_READ(phys_regs[0x51]+0x6c)&0xff00ffff)|0x300000;
    PTN_REG_WRITE(phys_regs[0x51]+0x6c, _tmp); // isp_cfa.reg_cfa_moire_lumathd_slope

    _tmp = (PTN_REG_READ(phys_regs[0x56]+0x0)&0xfffffffe)|0x1;
    PTN_REG_WRITE(phys_regs[0x56]+0x0, _tmp); // isp_csc.reg_csc_enable
#endif

    _tmp = (PTN_REG_READ(phys_regs[0x55]+0x30)&0xfffff000)|0x7f;
    PTN_REG_WRITE(phys_regs[0x55]+0x30, _tmp); // isp_dhz.reg_img_wd

    _tmp = (PTN_REG_READ(phys_regs[0x55]+0x30)&0xf000ffff)|0x3f0000;
    PTN_REG_WRITE(phys_regs[0x55]+0x30, _tmp); // isp_dhz.reg_img_ht

    _tmp = (PTN_REG_READ(phys_regs[0x58]+0x98)&0xfffff000)|0x7f;
    PTN_REG_WRITE(phys_regs[0x58]+0x98, _tmp); // isp_rgbee.reg_img_wd

    _tmp = (PTN_REG_READ(phys_regs[0x58]+0x98)&0xf000ffff)|0x3f0000;
    PTN_REG_WRITE(phys_regs[0x58]+0x98, _tmp); // isp_rgbee.reg_img_ht

    _tmp = (PTN_REG_READ(phys_regs[0x70]+0xc)&0xfffffffe)|0x1;
    PTN_REG_WRITE(phys_regs[0x70]+0xc, _tmp); // isp_top.reg_trig_str_sel_pre

    _tmp = (PTN_REG_READ(phys_regs[0x70]+0xc)&0xfffffffd)|0x2;
    PTN_REG_WRITE(phys_regs[0x70]+0xc, _tmp); // isp_top.reg_trig_str_sel_pre1

    _tmp = (PTN_REG_READ(phys_regs[0x70]+0xc)&0xfffffffb)|0x4;
    PTN_REG_WRITE(phys_regs[0x70]+0xc, _tmp); // isp_top.reg_trig_str_sel_post

    _tmp = (PTN_REG_READ(phys_regs[0x70]+0xc)&0xfffffff7)|0x8;
    PTN_REG_WRITE(phys_regs[0x70]+0xc, _tmp); // isp_top.reg_shaw_up_sel_pre

    _tmp = (PTN_REG_READ(phys_regs[0x70]+0xc)&0xffffffef)|0x10;
    PTN_REG_WRITE(phys_regs[0x70]+0xc, _tmp); // isp_top.reg_shaw_up_sel_pre1

    _tmp = (PTN_REG_READ(phys_regs[0x70]+0xc)&0xffffffdf)|0x20;
    PTN_REG_WRITE(phys_regs[0x70]+0xc, _tmp); // isp_top.reg_shaw_up_sel_post

    _tmp = (PTN_REG_READ(phys_regs[0x70]+0x4)&0xfffffffe)|0x1;
    PTN_REG_WRITE(phys_regs[0x70]+0x4, _tmp); // isp_top.reg_frame_done_pre_en

    _tmp = (PTN_REG_READ(phys_regs[0x70]+0x4)&0xfffffffd)|0x2;
    PTN_REG_WRITE(phys_regs[0x70]+0x4, _tmp); // isp_top.reg_frame_done_pre1_en

    _tmp = (PTN_REG_READ(phys_regs[0x70]+0x4)&0xfffffffb)|0x4;
    PTN_REG_WRITE(phys_regs[0x70]+0x4, _tmp); // isp_top.reg_frame_done_post_en

    _tmp = (PTN_REG_READ(phys_regs[0x70]+0x4)&0xfffffff7)|0x8;
    PTN_REG_WRITE(phys_regs[0x70]+0x4, _tmp); // isp_top.reg_shaw_done_pre_en

    _tmp = (PTN_REG_READ(phys_regs[0x70]+0x4)&0xffffffef)|0x10;
    PTN_REG_WRITE(phys_regs[0x70]+0x4, _tmp); // isp_top.reg_shaw_done_pre1_en

    _tmp = (PTN_REG_READ(phys_regs[0x70]+0x4)&0xffffffdf)|0x20;
    PTN_REG_WRITE(phys_regs[0x70]+0x4, _tmp); // isp_top.reg_shaw_done_post_en

    _tmp = (PTN_REG_READ(phys_regs[0x70]+0x4)&0xffffffbf)|0x40;
    PTN_REG_WRITE(phys_regs[0x70]+0x4, _tmp); // isp_top.reg_frame_err_pre_en

    _tmp = (PTN_REG_READ(phys_regs[0x70]+0x4)&0xffffff7f)|0x80;
    PTN_REG_WRITE(phys_regs[0x70]+0x4, _tmp); // isp_top.reg_frame_err_pre1_en

    _tmp = (PTN_REG_READ(phys_regs[0x70]+0x4)&0xfffffeff)|0x100;
    PTN_REG_WRITE(phys_regs[0x70]+0x4, _tmp); // isp_top.reg_frame_err_post_en

    _tmp = (PTN_REG_READ(phys_regs[0x1]+0x0)&0xfffffffb)|0x0;
    PTN_REG_WRITE(phys_regs[0x1]+0x0, _tmp); // isp_csi_bdg0.reg_csi_in_format

    _tmp = (PTN_REG_READ(phys_regs[0x1]+0x0)&0xfffffff7)|0x0;
    PTN_REG_WRITE(phys_regs[0x1]+0x0, _tmp); // isp_csi_bdg0.reg_csi_in_yuv_format

    _tmp = (PTN_REG_READ(phys_regs[0x1]+0x0)&0xffffffcf)|0x0;
    PTN_REG_WRITE(phys_regs[0x1]+0x0, _tmp); // isp_csi_bdg0.reg_ch_num

    _tmp = (PTN_REG_READ(phys_regs[0x1]+0x0)&0xffffffbf)|0x0;
    PTN_REG_WRITE(phys_regs[0x1]+0x0, _tmp); // isp_csi_bdg0.reg_dma_wr_enable

    _tmp = (PTN_REG_READ(phys_regs[0x1]+0x0)&0xfffffeff)|0x0;
    PTN_REG_WRITE(phys_regs[0x1]+0x0, _tmp); // isp_csi_bdg0.reg_ld_ch_id

    _tmp = (PTN_REG_READ(phys_regs[0x1]+0x0)&0xfffffdff)|0x0;
    PTN_REG_WRITE(phys_regs[0x1]+0x0, _tmp); // isp_csi_bdg0.reg_y_only

    _tmp = (PTN_REG_READ(phys_regs[0x1]+0x0)&0xfffffbff)|0x400;
    PTN_REG_WRITE(phys_regs[0x1]+0x0, _tmp); // isp_csi_bdg0.reg_pxl_data_sel

    _tmp = (PTN_REG_READ(phys_regs[0x1]+0x4)&0xffffe000)|0x7f;
    PTN_REG_WRITE(phys_regs[0x1]+0x4, _tmp); // isp_csi_bdg0.reg_frame_widthm1

    _tmp = (PTN_REG_READ(phys_regs[0x1]+0x4)&0xe000ffff)|0x3f0000;
    PTN_REG_WRITE(phys_regs[0x1]+0x4, _tmp); // isp_csi_bdg0.reg_frame_heightm1

    _tmp = (PTN_REG_READ(phys_regs[0x1]+0xc)&0xfffffffe)|0x1;
    PTN_REG_WRITE(phys_regs[0x1]+0xc, _tmp); // isp_csi_bdg0.reg_pat_en

    _tmp = (PTN_REG_READ(phys_regs[0x1]+0xc)&0xfffffffd)|0x0;
    PTN_REG_WRITE(phys_regs[0x1]+0xc, _tmp); // isp_csi_bdg0.reg_gra_inv

    _tmp = (PTN_REG_READ(phys_regs[0x1]+0xc)&0xfffffffb)|0x4;
    PTN_REG_WRITE(phys_regs[0x1]+0xc, _tmp); // isp_csi_bdg0.reg_auto_en

    _tmp = (PTN_REG_READ(phys_regs[0x1]+0xc)&0xfffffff7)|0x0;
    PTN_REG_WRITE(phys_regs[0x1]+0xc, _tmp); // isp_csi_bdg0.reg_dith_en

    _tmp = (PTN_REG_READ(phys_regs[0x1]+0xc)&0xffffffef)|0x0;
    PTN_REG_WRITE(phys_regs[0x1]+0xc, _tmp); // isp_csi_bdg0.reg_snow_en

    _tmp = (PTN_REG_READ(phys_regs[0x1]+0xc)&0xffffffdf)|0x0;
    PTN_REG_WRITE(phys_regs[0x1]+0xc, _tmp); // isp_csi_bdg0.reg_fix_mc

    _tmp = (PTN_REG_READ(phys_regs[0x1]+0xc)&0xfffff8ff)|0x0;
    PTN_REG_WRITE(phys_regs[0x1]+0xc, _tmp); // isp_csi_bdg0.reg_dith_md

    _tmp = (PTN_REG_READ(phys_regs[0x1]+0xc)&0xffffcfff)|0x2000;
    PTN_REG_WRITE(phys_regs[0x1]+0xc, _tmp); // isp_csi_bdg0.reg_bayer_id

    _tmp = (PTN_REG_READ(phys_regs[0x1]+0x10)&0xffffff00)|0x0;
    PTN_REG_WRITE(phys_regs[0x1]+0x10, _tmp); // isp_csi_bdg0.reg_pat_prd

    _tmp = (PTN_REG_READ(phys_regs[0x1]+0x10)&0xffffe0ff)|0x0;
    PTN_REG_WRITE(phys_regs[0x1]+0x10, _tmp); // isp_csi_bdg0.reg_pat_idx

    _tmp = (PTN_REG_READ(phys_regs[0x1]+0x14)&0xfffff000)|0x400;
    PTN_REG_WRITE(phys_regs[0x1]+0x14, _tmp); // isp_csi_bdg0.reg_pat_r

    _tmp = (PTN_REG_READ(phys_regs[0x1]+0x14)&0xf000ffff)|0x8000000;
    PTN_REG_WRITE(phys_regs[0x1]+0x14, _tmp); // isp_csi_bdg0.reg_pat_g

    _tmp = (PTN_REG_READ(phys_regs[0x1]+0x18)&0xfffff000)|0xffd;
    PTN_REG_WRITE(phys_regs[0x1]+0x18, _tmp); // isp_csi_bdg0.reg_pat_b

    _tmp = (PTN_REG_READ(phys_regs[0x1]+0x1c)&0xfffff000)|0x0;
    PTN_REG_WRITE(phys_regs[0x1]+0x1c, _tmp); // isp_csi_bdg0.reg_fde_r

    _tmp = (PTN_REG_READ(phys_regs[0x1]+0x1c)&0xf000ffff)|0x10000;
    PTN_REG_WRITE(phys_regs[0x1]+0x1c, _tmp); // isp_csi_bdg0.reg_fde_g

    _tmp = (PTN_REG_READ(phys_regs[0x1]+0x20)&0xfffff000)|0x2;
    PTN_REG_WRITE(phys_regs[0x1]+0x20, _tmp); // isp_csi_bdg0.reg_fde_b

    _tmp = (PTN_REG_READ(phys_regs[0x1]+0x24)&0xfffff000)|0x457;
    PTN_REG_WRITE(phys_regs[0x1]+0x24, _tmp); // isp_csi_bdg0.reg_mde_r

    _tmp = (PTN_REG_READ(phys_regs[0x1]+0x24)&0xf000ffff)|0x8ae0000;
    PTN_REG_WRITE(phys_regs[0x1]+0x24, _tmp); // isp_csi_bdg0.reg_mde_g

    _tmp = (PTN_REG_READ(phys_regs[0x1]+0x28)&0xfffff000)|0xd05;
    PTN_REG_WRITE(phys_regs[0x1]+0x28, _tmp); // isp_csi_bdg0.reg_mde_b

    _tmp = (PTN_REG_READ(phys_regs[0x1]+0x2c)&0xffffe000)|0x0;
    PTN_REG_WRITE(phys_regs[0x1]+0x2c, _tmp); // isp_csi_bdg0.reg_vmde_str

    _tmp = (PTN_REG_READ(phys_regs[0x1]+0x2c)&0xe000ffff)|0x3f0000;
    PTN_REG_WRITE(phys_regs[0x1]+0x2c, _tmp); // isp_csi_bdg0.reg_vmde_stp

    _tmp = (PTN_REG_READ(phys_regs[0x1]+0x30)&0xffffe000)|0x0;
    PTN_REG_WRITE(phys_regs[0x1]+0x30, _tmp); // isp_csi_bdg0.reg_hmde_str

    _tmp = (PTN_REG_READ(phys_regs[0x1]+0x30)&0xe000ffff)|0x7f0000;
    PTN_REG_WRITE(phys_regs[0x1]+0x30, _tmp); // isp_csi_bdg0.reg_hmde_stp

    _tmp = (PTN_REG_READ(phys_regs[0x1]+0x34)&0xffffe000)|0x6;
    PTN_REG_WRITE(phys_regs[0x1]+0x34, _tmp); // isp_csi_bdg0.reg_vfde_str

    _tmp = (PTN_REG_READ(phys_regs[0x1]+0x34)&0xe000ffff)|0x450000;
    PTN_REG_WRITE(phys_regs[0x1]+0x34, _tmp); // isp_csi_bdg0.reg_vfde_stp

    _tmp = (PTN_REG_READ(phys_regs[0x1]+0x38)&0xffffe000)|0xc;
    PTN_REG_WRITE(phys_regs[0x1]+0x38, _tmp); // isp_csi_bdg0.reg_hfde_str

    _tmp = (PTN_REG_READ(phys_regs[0x1]+0x38)&0xe000ffff)|0x8b0000;
    PTN_REG_WRITE(phys_regs[0x1]+0x38, _tmp); // isp_csi_bdg0.reg_hfde_stp

    _tmp = (PTN_REG_READ(phys_regs[0x1]+0x3c)&0xffffe000)|0x0;
    PTN_REG_WRITE(phys_regs[0x1]+0x3c, _tmp); // isp_csi_bdg0.reg_hs_str

    _tmp = (PTN_REG_READ(phys_regs[0x1]+0x3c)&0xe000ffff)|0x30000;
    PTN_REG_WRITE(phys_regs[0x1]+0x3c, _tmp); // isp_csi_bdg0.reg_hs_stp

    _tmp = (PTN_REG_READ(phys_regs[0x1]+0x40)&0xffffe000)|0x0;
    PTN_REG_WRITE(phys_regs[0x1]+0x40, _tmp); // isp_csi_bdg0.reg_vs_str

    _tmp = (PTN_REG_READ(phys_regs[0x1]+0x40)&0xe000ffff)|0x30000;
    PTN_REG_WRITE(phys_regs[0x1]+0x40, _tmp); // isp_csi_bdg0.reg_vs_stp

    _tmp = (PTN_REG_READ(phys_regs[0x1]+0x44)&0xffffe000)|0x93;
    PTN_REG_WRITE(phys_regs[0x1]+0x44, _tmp); // isp_csi_bdg0.reg_htt

    _tmp = (PTN_REG_READ(phys_regs[0x1]+0x44)&0xe000ffff)|0xa80000;
    PTN_REG_WRITE(phys_regs[0x1]+0x44, _tmp); // isp_csi_bdg0.reg_vtt

    _tmp = (PTN_REG_READ(phys_regs[0x1]+0x0)&0xfffffffc)|0x3;
    PTN_REG_WRITE(phys_regs[0x1]+0x0, _tmp); // isp_csi_bdg0.reg_csi_mode

} while (0);
