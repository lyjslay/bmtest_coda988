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

    _tmp = (PTN_REG_READ(phys_regs[0x0]+0x0)&0xfffffffc)|0x3;
    PTN_REG_WRITE(phys_regs[0x0]+0x0, _tmp); // pre_raw.reg_csi_bdg_out_mode

    _mem_addr = PTN_MALLOC(0xc0*0x40, priv);
    PTN_REG_WRITE(phys_regs[0x1]+0x100, (_mem_addr&0xffffffff)); // isp_dma0.reg_dma_addr_31_0
    PTN_REG_WRITE(phys_regs[0x1]+0x104, ((_mem_addr>>32)&0xffffffff)); // isp_dma0.reg_dma_addr_63_32

    _tmp = (PTN_REG_READ(phys_regs[0x1]+0x108)&0xffff0000)|0xc0;
    PTN_REG_WRITE(phys_regs[0x1]+0x108, _tmp); // isp_dma0.reg_dma_cnt

    _tmp = (PTN_REG_READ(phys_regs[0x1]+0x124)&0xffffe000)|0x7f;
    PTN_REG_WRITE(phys_regs[0x1]+0x124, _tmp); // isp_dma0.reg_img_width

    _tmp = (PTN_REG_READ(phys_regs[0x1]+0x124)&0xe000ffff)|0x3f0000;
    PTN_REG_WRITE(phys_regs[0x1]+0x124, _tmp); // isp_dma0.reg_img_height

    _tmp = (PTN_REG_READ(phys_regs[0x1]+0x128)&0xffff0000)|0xc0;
    PTN_REG_WRITE(phys_regs[0x1]+0x128, _tmp); // isp_dma0.reg_line_offset

    _tmp = (PTN_REG_READ(phys_regs[0x1]+0x128)&0xfff8ffff)|0x30000;
    PTN_REG_WRITE(phys_regs[0x1]+0x128, _tmp); // isp_dma0.reg_mod_ctl

    _tmp = (PTN_REG_READ(phys_regs[0x1]+0x134)&0xfffffff0)|0xf;
    PTN_REG_WRITE(phys_regs[0x1]+0x134, _tmp); // isp_dma0.reg_int_sts_en

    _tmp = (PTN_REG_READ(phys_regs[0x1]+0x120)&0xfffffffe)|0x0;
    PTN_REG_WRITE(phys_regs[0x1]+0x120, _tmp); // isp_dma0.reg_job_start

    _mem_addr = PTN_MALLOC(0xc0*0x40, priv);
    PTN_REG_WRITE(phys_regs[0x1]+0x200, (_mem_addr&0xffffffff)); // isp_dma1.reg_dma_addr_31_0
    PTN_REG_WRITE(phys_regs[0x1]+0x204, ((_mem_addr>>32)&0xffffffff)); // isp_dma1.reg_dma_addr_63_32

    _tmp = (PTN_REG_READ(phys_regs[0x1]+0x208)&0xffff0000)|0xc0;
    PTN_REG_WRITE(phys_regs[0x1]+0x208, _tmp); // isp_dma1.reg_dma_cnt

    _tmp = (PTN_REG_READ(phys_regs[0x1]+0x224)&0xffffe000)|0x7f;
    PTN_REG_WRITE(phys_regs[0x1]+0x224, _tmp); // isp_dma1.reg_img_width

    _tmp = (PTN_REG_READ(phys_regs[0x1]+0x224)&0xe000ffff)|0x3f0000;
    PTN_REG_WRITE(phys_regs[0x1]+0x224, _tmp); // isp_dma1.reg_img_height

    _tmp = (PTN_REG_READ(phys_regs[0x1]+0x228)&0xffff0000)|0xc0;
    PTN_REG_WRITE(phys_regs[0x1]+0x228, _tmp); // isp_dma1.reg_line_offset

    _tmp = (PTN_REG_READ(phys_regs[0x1]+0x228)&0xfff8ffff)|0x30000;
    PTN_REG_WRITE(phys_regs[0x1]+0x228, _tmp); // isp_dma1.reg_mod_ctl

    _tmp = (PTN_REG_READ(phys_regs[0x1]+0x234)&0xfffffff0)|0xf;
    PTN_REG_WRITE(phys_regs[0x1]+0x234, _tmp); // isp_dma1.reg_int_sts_en

    _tmp = (PTN_REG_READ(phys_regs[0x1]+0x220)&0xfffffffe)|0x0;
    PTN_REG_WRITE(phys_regs[0x1]+0x220, _tmp); // isp_dma1.reg_job_start

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

    _mem_addr = PTN_MALLOC(0xc0*0x40, priv);
    PTN_REG_WRITE(phys_regs[0x3d]+0x100, (_mem_addr&0xffffffff)); // isp_dma6.reg_dma_addr_31_0
    PTN_REG_WRITE(phys_regs[0x3d]+0x104, ((_mem_addr>>32)&0xffffffff)); // isp_dma6.reg_dma_addr_63_32

    _tmp = (PTN_REG_READ(phys_regs[0x3d]+0x108)&0xffff0000)|0xc0;
    PTN_REG_WRITE(phys_regs[0x3d]+0x108, _tmp); // isp_dma6.reg_dma_cnt

    _tmp = (PTN_REG_READ(phys_regs[0x3d]+0x124)&0xffffe000)|0x7f;
    PTN_REG_WRITE(phys_regs[0x3d]+0x124, _tmp); // isp_dma6.reg_img_width

    _tmp = (PTN_REG_READ(phys_regs[0x3d]+0x124)&0xe000ffff)|0x3f0000;
    PTN_REG_WRITE(phys_regs[0x3d]+0x124, _tmp); // isp_dma6.reg_img_height

    _tmp = (PTN_REG_READ(phys_regs[0x3d]+0x128)&0xffff0000)|0xc0;
    PTN_REG_WRITE(phys_regs[0x3d]+0x128, _tmp); // isp_dma6.reg_line_offset

    _tmp = (PTN_REG_READ(phys_regs[0x3d]+0x128)&0xfff8ffff)|0x30000;
    PTN_REG_WRITE(phys_regs[0x3d]+0x128, _tmp); // isp_dma6.reg_mod_ctl

    _tmp = (PTN_REG_READ(phys_regs[0x3d]+0x134)&0xfffffff0)|0xf;
    PTN_REG_WRITE(phys_regs[0x3d]+0x134, _tmp); // isp_dma6.reg_int_sts_en

    _tmp = (PTN_REG_READ(phys_regs[0x3d]+0x120)&0xfffffffe)|0x0;
    PTN_REG_WRITE(phys_regs[0x3d]+0x120, _tmp); // isp_dma6.reg_job_start

    _mem_addr = PTN_MALLOC(0xc0*0x40, priv);
    PTN_REG_WRITE(phys_regs[0x3e]+0x100, (_mem_addr&0xffffffff)); // isp_dma7.reg_dma_addr_31_0
    PTN_REG_WRITE(phys_regs[0x3e]+0x104, ((_mem_addr>>32)&0xffffffff)); // isp_dma7.reg_dma_addr_63_32

    _tmp = (PTN_REG_READ(phys_regs[0x3e]+0x108)&0xffff0000)|0xc0;
    PTN_REG_WRITE(phys_regs[0x3e]+0x108, _tmp); // isp_dma7.reg_dma_cnt

    _tmp = (PTN_REG_READ(phys_regs[0x3e]+0x124)&0xffffe000)|0x7f;
    PTN_REG_WRITE(phys_regs[0x3e]+0x124, _tmp); // isp_dma7.reg_img_width

    _tmp = (PTN_REG_READ(phys_regs[0x3e]+0x124)&0xe000ffff)|0x3f0000;
    PTN_REG_WRITE(phys_regs[0x3e]+0x124, _tmp); // isp_dma7.reg_img_height

    _tmp = (PTN_REG_READ(phys_regs[0x3e]+0x128)&0xffff0000)|0xc0;
    PTN_REG_WRITE(phys_regs[0x3e]+0x128, _tmp); // isp_dma7.reg_line_offset

    _tmp = (PTN_REG_READ(phys_regs[0x3e]+0x128)&0xfff8ffff)|0x30000;
    PTN_REG_WRITE(phys_regs[0x3e]+0x128, _tmp); // isp_dma7.reg_mod_ctl

    _tmp = (PTN_REG_READ(phys_regs[0x3e]+0x134)&0xfffffff0)|0xf;
    PTN_REG_WRITE(phys_regs[0x3e]+0x134, _tmp); // isp_dma7.reg_int_sts_en

    _tmp = (PTN_REG_READ(phys_regs[0x3e]+0x120)&0xfffffffe)|0x0;
    PTN_REG_WRITE(phys_regs[0x3e]+0x120, _tmp); // isp_dma7.reg_job_start

    _tmp = (PTN_REG_READ(phys_regs[0x3d]+0x0)&0xfffffffd)|0x2;
    PTN_REG_WRITE(phys_regs[0x3d]+0x0, _tmp); // isp_crop2.reg_dma_enable

    _tmp = (PTN_REG_READ(phys_regs[0x3e]+0x0)&0xfffffffd)|0x2;
    PTN_REG_WRITE(phys_regs[0x3e]+0x0, _tmp); // isp_crop3.reg_dma_enable

    _tmp = (PTN_REG_READ(phys_regs[0x69]+0x0)&0xfffffffd)|0x2;
    PTN_REG_WRITE(phys_regs[0x69]+0x0, _tmp); // isp_crop4.reg_dma_enable

    _tmp = (PTN_REG_READ(phys_regs[0x6a]+0x0)&0xfffffffd)|0x2;
    PTN_REG_WRITE(phys_regs[0x6a]+0x0, _tmp); // isp_crop5.reg_dma_enable

    _tmp = (PTN_REG_READ(phys_regs[0x6b]+0x0)&0xfffffffd)|0x2;
    PTN_REG_WRITE(phys_regs[0x6b]+0x0, _tmp); // isp_crop6.reg_dma_enable

    _tmp = (PTN_REG_READ(phys_regs[0x3a]+0x0)&0xfffffffe)|0x1;
    PTN_REG_WRITE(phys_regs[0x3a]+0x0, _tmp); // isp_fusion.reg_fs_enable

    _tmp = (PTN_REG_READ(phys_regs[0x3a]+0x0)&0xfffffffd)|0x0;
    PTN_REG_WRITE(phys_regs[0x3a]+0x0, _tmp); // isp_fusion.reg_fs_bpss

    _tmp = (PTN_REG_READ(phys_regs[0x3a]+0x0)&0xffffffef)|0x0;
    PTN_REG_WRITE(phys_regs[0x3a]+0x0, _tmp); // isp_fusion.reg_fs_mc_enable

    _tmp = (PTN_REG_READ(phys_regs[0x3a]+0x0)&0xfffffcff)|0x100;
    PTN_REG_WRITE(phys_regs[0x3a]+0x0, _tmp); // isp_fusion.reg_fs_out_sel

    _tmp = (PTN_REG_READ(phys_regs[0x3a]+0x0)&0xfff)|0xffff000;
    PTN_REG_WRITE(phys_regs[0x3a]+0x0, _tmp); // isp_fusion.reg_fs_s_max

    _tmp = (PTN_REG_READ(phys_regs[0x3a]+0x4)&0xffffe000)|0x7f;
    PTN_REG_WRITE(phys_regs[0x3a]+0x4, _tmp); // isp_fusion.reg_fs_widthm1

    _tmp = (PTN_REG_READ(phys_regs[0x3a]+0x4)&0xe000ffff)|0x3f0000;
    PTN_REG_WRITE(phys_regs[0x3a]+0x4, _tmp); // isp_fusion.reg_fs_heightm1

    _tmp = (PTN_REG_READ(phys_regs[0x3a]+0x8)&0xffffc000)|0x400;
    PTN_REG_WRITE(phys_regs[0x3a]+0x8, _tmp); // isp_fusion.reg_fs_ls_gain

    _tmp = (PTN_REG_READ(phys_regs[0x3a]+0xc)&0xfffff000)|0x0;
    PTN_REG_WRITE(phys_regs[0x3a]+0xc, _tmp); // isp_fusion.reg_fs_luma_thd_l

    _tmp = (PTN_REG_READ(phys_regs[0x3a]+0xc)&0xf000ffff)|0xfff0000;
    PTN_REG_WRITE(phys_regs[0x3a]+0xc, _tmp); // isp_fusion.reg_fs_luma_thd_h

    _tmp = (PTN_REG_READ(phys_regs[0x3a]+0x10)&0xffffff00)|0xff;
    PTN_REG_WRITE(phys_regs[0x3a]+0x10, _tmp); // isp_fusion.reg_fs_wgt_max

    _tmp = (PTN_REG_READ(phys_regs[0x3a]+0x10)&0xffff00ff)|0x0;
    PTN_REG_WRITE(phys_regs[0x3a]+0x10, _tmp); // isp_fusion.reg_fs_wgt_min

    _tmp = (PTN_REG_READ(phys_regs[0x3a]+0x14)&0xfffc0000)|0x40;
    PTN_REG_WRITE(phys_regs[0x3a]+0x14, _tmp); // isp_fusion.reg_fs_wgt_slp

    _tmp = (PTN_REG_READ(phys_regs[0x50]+0x0)&0xfffffffc)|0x2;
    PTN_REG_WRITE(phys_regs[0x50]+0x0, _tmp); // isp_rgb.reg_color_mode

    _tmp = (PTN_REG_READ(phys_regs[0x51]+0x0)&0xfffffffd)|0x2;
    PTN_REG_WRITE(phys_regs[0x51]+0x0, _tmp); // isp_cfa.reg_cfa_enable

    _tmp = (PTN_REG_READ(phys_regs[0x51]+0x8)&0xfffff000)|0x7f;
    PTN_REG_WRITE(phys_regs[0x51]+0x8, _tmp); // isp_cfa.reg_img_wd

    _tmp = (PTN_REG_READ(phys_regs[0x51]+0x8)&0xf000ffff)|0x3f0000;
    PTN_REG_WRITE(phys_regs[0x51]+0x8, _tmp); // isp_cfa.reg_img_ht

    _tmp = (PTN_REG_READ(phys_regs[0x58]+0x98)&0xfffff000)|0x7f;
    PTN_REG_WRITE(phys_regs[0x58]+0x98, _tmp); // isp_rgbee.reg_img_wd

    _tmp = (PTN_REG_READ(phys_regs[0x58]+0x98)&0xf000ffff)|0x3f0000;
    PTN_REG_WRITE(phys_regs[0x58]+0x98, _tmp); // isp_rgbee.reg_img_ht

    _tmp = (PTN_REG_READ(phys_regs[0x55]+0x30)&0xfffff000)|0x7f;
    PTN_REG_WRITE(phys_regs[0x55]+0x30, _tmp); // isp_dhz.reg_img_wd

    _tmp = (PTN_REG_READ(phys_regs[0x55]+0x30)&0xf000ffff)|0x3f0000;
    PTN_REG_WRITE(phys_regs[0x55]+0x30, _tmp); // isp_dhz.reg_img_ht

    _tmp = (PTN_REG_READ(phys_regs[0x70]+0xc)&0xfffffffe)|0x1;
    PTN_REG_WRITE(phys_regs[0x70]+0xc, _tmp); // isp_top.reg_trig_str_sel_pre

    _tmp = (PTN_REG_READ(phys_regs[0x70]+0xc)&0xfffffffb)|0x0;
    PTN_REG_WRITE(phys_regs[0x70]+0xc, _tmp); // isp_top.reg_trig_str_sel_post

    _tmp = (PTN_REG_READ(phys_regs[0x70]+0xc)&0xfffffff7)|0x8;
    PTN_REG_WRITE(phys_regs[0x70]+0xc, _tmp); // isp_top.reg_shaw_up_sel_pre

    _tmp = (PTN_REG_READ(phys_regs[0x70]+0xc)&0xffffffdf)|0x0;
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

    _tmp = (PTN_REG_READ(phys_regs[0x1]+0x0)&0xffffffcf)|0x10;
    PTN_REG_WRITE(phys_regs[0x1]+0x0, _tmp); // isp_csi_bdg0.reg_ch_num

    _tmp = (PTN_REG_READ(phys_regs[0x1]+0x0)&0xffffffbf)|0x40;
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

    _tmp = (PTN_REG_READ(phys_regs[0x1]+0x2c)&0xe000ffff)|0x7f0000;
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
