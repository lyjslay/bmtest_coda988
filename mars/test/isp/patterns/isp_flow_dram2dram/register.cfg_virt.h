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

    _tmp = (PTN_REG_READ(reg_bases[0x0]+0x0)&0xfffffffc)|0x2;
    PTN_REG_WRITE(reg_bases[0x0]+0x0, _tmp); // pre_raw.reg_csi_bdg_out_mode

    _tmp = (PTN_REG_READ(reg_bases[0x69]+0x0)&0xfffffffd)|0x2;
    PTN_REG_WRITE(reg_bases[0x69]+0x0, _tmp); // isp_crop4.reg_dma_enable

    _tmp = (PTN_REG_READ(reg_bases[0x6a]+0x0)&0xfffffffd)|0x2;
    PTN_REG_WRITE(reg_bases[0x6a]+0x0, _tmp); // isp_crop5.reg_dma_enable

    _tmp = (PTN_REG_READ(reg_bases[0x6b]+0x0)&0xfffffffd)|0x2;
    PTN_REG_WRITE(reg_bases[0x6b]+0x0, _tmp); // isp_crop6.reg_dma_enable

    _mem_addr = PTN_MALLOC(0x80*0x40, priv);
    PTN_REG_WRITE(reg_bases[0x69]+0x200, (_mem_addr&0xffffffff)); // isp_dma3.reg_dma_addr_31_0
    PTN_REG_WRITE(reg_bases[0x69]+0x208, ((_mem_addr>>32)&0xffffffff)); // isp_dma3.reg_dma_addr_63_32

    _tmp = (PTN_REG_READ(reg_bases[0x69]+0x210)&0xffff0000)|0x80;
    PTN_REG_WRITE(reg_bases[0x69]+0x210, _tmp); // isp_dma3.reg_dma_cnt

    _tmp = (PTN_REG_READ(reg_bases[0x69]+0x248)&0xffffe000)|0x7f;
    PTN_REG_WRITE(reg_bases[0x69]+0x248, _tmp); // isp_dma3.reg_img_width

    _tmp = (PTN_REG_READ(reg_bases[0x69]+0x248)&0xe000ffff)|0x3f0000;
    PTN_REG_WRITE(reg_bases[0x69]+0x248, _tmp); // isp_dma3.reg_img_height

    _tmp = (PTN_REG_READ(reg_bases[0x69]+0x250)&0xffff0000)|0x80;
    PTN_REG_WRITE(reg_bases[0x69]+0x250, _tmp); // isp_dma3.reg_line_offset

    _tmp = (PTN_REG_READ(reg_bases[0x69]+0x250)&0xfff8ffff)|0x0;
    PTN_REG_WRITE(reg_bases[0x69]+0x250, _tmp); // isp_dma3.reg_mod_ctl

    _tmp = (PTN_REG_READ(reg_bases[0x69]+0x268)&0xfffffff0)|0xf;
    PTN_REG_WRITE(reg_bases[0x69]+0x268, _tmp); // isp_dma3.reg_int_sts_en

    _tmp = (PTN_REG_READ(reg_bases[0x69]+0x240)&0xfffffffe)|0x0;
    PTN_REG_WRITE(reg_bases[0x69]+0x240, _tmp); // isp_dma3.reg_job_start

    _mem_addr = PTN_MALLOC(0x40*0x20, priv);
    PTN_REG_WRITE(reg_bases[0x6a]+0x200, (_mem_addr&0xffffffff)); // isp_dma4.reg_dma_addr_31_0
    PTN_REG_WRITE(reg_bases[0x6a]+0x208, ((_mem_addr>>32)&0xffffffff)); // isp_dma4.reg_dma_addr_63_32

    _tmp = (PTN_REG_READ(reg_bases[0x6a]+0x210)&0xffff0000)|0x40;
    PTN_REG_WRITE(reg_bases[0x6a]+0x210, _tmp); // isp_dma4.reg_dma_cnt

    _tmp = (PTN_REG_READ(reg_bases[0x6a]+0x248)&0xffffe000)|0x3f;
    PTN_REG_WRITE(reg_bases[0x6a]+0x248, _tmp); // isp_dma4.reg_img_width

    _tmp = (PTN_REG_READ(reg_bases[0x6a]+0x248)&0xe000ffff)|0x1f0000;
    PTN_REG_WRITE(reg_bases[0x6a]+0x248, _tmp); // isp_dma4.reg_img_height

    _tmp = (PTN_REG_READ(reg_bases[0x6a]+0x250)&0xffff0000)|0x40;
    PTN_REG_WRITE(reg_bases[0x6a]+0x250, _tmp); // isp_dma4.reg_line_offset

    _tmp = (PTN_REG_READ(reg_bases[0x6a]+0x250)&0xfff8ffff)|0x0;
    PTN_REG_WRITE(reg_bases[0x6a]+0x250, _tmp); // isp_dma4.reg_mod_ctl

    _tmp = (PTN_REG_READ(reg_bases[0x6a]+0x268)&0xfffffff0)|0xf;
    PTN_REG_WRITE(reg_bases[0x6a]+0x268, _tmp); // isp_dma4.reg_int_sts_en

    _tmp = (PTN_REG_READ(reg_bases[0x6a]+0x240)&0xfffffffe)|0x0;
    PTN_REG_WRITE(reg_bases[0x6a]+0x240, _tmp); // isp_dma4.reg_job_start

    _mem_addr = PTN_MALLOC(0x40*0x20, priv);
    PTN_REG_WRITE(reg_bases[0x6b]+0x200, (_mem_addr&0xffffffff)); // isp_dma5.reg_dma_addr_31_0
    PTN_REG_WRITE(reg_bases[0x6b]+0x208, ((_mem_addr>>32)&0xffffffff)); // isp_dma5.reg_dma_addr_63_32

    _tmp = (PTN_REG_READ(reg_bases[0x6b]+0x210)&0xffff0000)|0x40;
    PTN_REG_WRITE(reg_bases[0x6b]+0x210, _tmp); // isp_dma5.reg_dma_cnt

    _tmp = (PTN_REG_READ(reg_bases[0x6b]+0x248)&0xffffe000)|0x3f;
    PTN_REG_WRITE(reg_bases[0x6b]+0x248, _tmp); // isp_dma5.reg_img_width

    _tmp = (PTN_REG_READ(reg_bases[0x6b]+0x248)&0xe000ffff)|0x1f0000;
    PTN_REG_WRITE(reg_bases[0x6b]+0x248, _tmp); // isp_dma5.reg_img_height

    _tmp = (PTN_REG_READ(reg_bases[0x6b]+0x250)&0xffff0000)|0x40;
    PTN_REG_WRITE(reg_bases[0x6b]+0x250, _tmp); // isp_dma5.reg_line_offset

    _tmp = (PTN_REG_READ(reg_bases[0x6b]+0x250)&0xfff8ffff)|0x0;
    PTN_REG_WRITE(reg_bases[0x6b]+0x250, _tmp); // isp_dma5.reg_mod_ctl

    _tmp = (PTN_REG_READ(reg_bases[0x6b]+0x268)&0xfffffff0)|0xf;
    PTN_REG_WRITE(reg_bases[0x6b]+0x268, _tmp); // isp_dma5.reg_int_sts_en

    _tmp = (PTN_REG_READ(reg_bases[0x6b]+0x240)&0xfffffffe)|0x0;
    PTN_REG_WRITE(reg_bases[0x6b]+0x240, _tmp); // isp_dma5.reg_job_start

    _mem_addr = PTN_MALLOC(0xc0*0x40, priv);
    PTN_REG_WRITE(reg_bases[0x1]+0x800, (_mem_addr&0xffffffff)); // isp_dma8.reg_dma_addr_31_0
    PTN_REG_WRITE(reg_bases[0x1]+0x808, ((_mem_addr>>32)&0xffffffff)); // isp_dma8.reg_dma_addr_63_32

    _tmp = (PTN_REG_READ(reg_bases[0x1]+0x810)&0xffff0000)|0xc0;
    PTN_REG_WRITE(reg_bases[0x1]+0x810, _tmp); // isp_dma8.reg_dma_cnt

    _tmp = (PTN_REG_READ(reg_bases[0x1]+0x848)&0xffffe000)|0x7f;
    PTN_REG_WRITE(reg_bases[0x1]+0x848, _tmp); // isp_dma8.reg_img_width

    _tmp = (PTN_REG_READ(reg_bases[0x1]+0x848)&0xe000ffff)|0x3f0000;
    PTN_REG_WRITE(reg_bases[0x1]+0x848, _tmp); // isp_dma8.reg_img_height

    _tmp = (PTN_REG_READ(reg_bases[0x1]+0x850)&0xffff0000)|0xc0;
    PTN_REG_WRITE(reg_bases[0x1]+0x850, _tmp); // isp_dma8.reg_line_offset

    _tmp = (PTN_REG_READ(reg_bases[0x1]+0x850)&0xfff8ffff)|0x30000;
    PTN_REG_WRITE(reg_bases[0x1]+0x850, _tmp); // isp_dma8.reg_mod_ctl

    _tmp = (PTN_REG_READ(reg_bases[0x1]+0x868)&0xfffffff0)|0xf;
    PTN_REG_WRITE(reg_bases[0x1]+0x868, _tmp); // isp_dma8.reg_int_sts_en

    _tmp = (PTN_REG_READ(reg_bases[0x1]+0x840)&0xfffffffe)|0x0;
    PTN_REG_WRITE(reg_bases[0x1]+0x840, _tmp); // isp_dma8.reg_job_start

    _tmp = (PTN_REG_READ(reg_bases[0x50]+0x0)&0xfffffffc)|0x2;
    PTN_REG_WRITE(reg_bases[0x50]+0x0, _tmp); // isp_rgb.reg_color_mode

    _tmp = (PTN_REG_READ(reg_bases[0x51]+0x0)&0xfffffffd)|0x2;
    PTN_REG_WRITE(reg_bases[0x51]+0x0, _tmp); // isp_cfa.reg_cfa_enable

    _tmp = (PTN_REG_READ(reg_bases[0x51]+0x10)&0xfffff000)|0x7f;
    PTN_REG_WRITE(reg_bases[0x51]+0x10, _tmp); // isp_cfa.reg_img_wd

    _tmp = (PTN_REG_READ(reg_bases[0x51]+0x10)&0xf000ffff)|0x3f0000;
    PTN_REG_WRITE(reg_bases[0x51]+0x10, _tmp); // isp_cfa.reg_img_ht

    _tmp = (PTN_REG_READ(reg_bases[0x58]+0x130)&0xfffff000)|0x7f;
    PTN_REG_WRITE(reg_bases[0x58]+0x130, _tmp); // isp_rgbee.reg_img_wd

    _tmp = (PTN_REG_READ(reg_bases[0x58]+0x130)&0xf000ffff)|0x3f0000;
    PTN_REG_WRITE(reg_bases[0x58]+0x130, _tmp); // isp_rgbee.reg_img_ht

    _tmp = (PTN_REG_READ(reg_bases[0x55]+0x60)&0xfffff000)|0x7f;
    PTN_REG_WRITE(reg_bases[0x55]+0x60, _tmp); // isp_dhz.reg_img_wd

    _tmp = (PTN_REG_READ(reg_bases[0x55]+0x60)&0xf000ffff)|0x3f0000;
    PTN_REG_WRITE(reg_bases[0x55]+0x60, _tmp); // isp_dhz.reg_img_ht

    _tmp = (PTN_REG_READ(reg_bases[0x70]+0x18)&0xfffffffe)|0x0;
    PTN_REG_WRITE(reg_bases[0x70]+0x18, _tmp); // isp_top.reg_trig_str_sel_pre

    _tmp = (PTN_REG_READ(reg_bases[0x70]+0x18)&0xfffffffd)|0x0;
    PTN_REG_WRITE(reg_bases[0x70]+0x18, _tmp); // isp_top.reg_trig_str_sel_pre1

    _tmp = (PTN_REG_READ(reg_bases[0x70]+0x18)&0xfffffffb)|0x0;
    PTN_REG_WRITE(reg_bases[0x70]+0x18, _tmp); // isp_top.reg_trig_str_sel_post

    _tmp = (PTN_REG_READ(reg_bases[0x70]+0x18)&0xfffffff7)|0x0;
    PTN_REG_WRITE(reg_bases[0x70]+0x18, _tmp); // isp_top.reg_shaw_up_sel_pre

    _tmp = (PTN_REG_READ(reg_bases[0x70]+0x18)&0xffffffef)|0x0;
    PTN_REG_WRITE(reg_bases[0x70]+0x18, _tmp); // isp_top.reg_shaw_up_sel_pre1

    _tmp = (PTN_REG_READ(reg_bases[0x70]+0x18)&0xffffffdf)|0x0;
    PTN_REG_WRITE(reg_bases[0x70]+0x18, _tmp); // isp_top.reg_shaw_up_sel_post

    _tmp = (PTN_REG_READ(reg_bases[0x70]+0x8)&0xfffffffe)|0x1;
    PTN_REG_WRITE(reg_bases[0x70]+0x8, _tmp); // isp_top.reg_frame_done_pre_en

    _tmp = (PTN_REG_READ(reg_bases[0x70]+0x8)&0xfffffffd)|0x2;
    PTN_REG_WRITE(reg_bases[0x70]+0x8, _tmp); // isp_top.reg_frame_done_pre1_en

    _tmp = (PTN_REG_READ(reg_bases[0x70]+0x8)&0xfffffffb)|0x4;
    PTN_REG_WRITE(reg_bases[0x70]+0x8, _tmp); // isp_top.reg_frame_done_post_en

    _tmp = (PTN_REG_READ(reg_bases[0x70]+0x8)&0xfffffff7)|0x8;
    PTN_REG_WRITE(reg_bases[0x70]+0x8, _tmp); // isp_top.reg_shaw_done_pre_en

    _tmp = (PTN_REG_READ(reg_bases[0x70]+0x8)&0xffffffef)|0x10;
    PTN_REG_WRITE(reg_bases[0x70]+0x8, _tmp); // isp_top.reg_shaw_done_pre1_en

    _tmp = (PTN_REG_READ(reg_bases[0x70]+0x8)&0xffffffdf)|0x20;
    PTN_REG_WRITE(reg_bases[0x70]+0x8, _tmp); // isp_top.reg_shaw_done_post_en

    _tmp = (PTN_REG_READ(reg_bases[0x70]+0x8)&0xffffffbf)|0x40;
    PTN_REG_WRITE(reg_bases[0x70]+0x8, _tmp); // isp_top.reg_frame_err_pre_en

    _tmp = (PTN_REG_READ(reg_bases[0x70]+0x8)&0xffffff7f)|0x80;
    PTN_REG_WRITE(reg_bases[0x70]+0x8, _tmp); // isp_top.reg_frame_err_pre1_en

    _tmp = (PTN_REG_READ(reg_bases[0x70]+0x8)&0xfffffeff)|0x100;
    PTN_REG_WRITE(reg_bases[0x70]+0x8, _tmp); // isp_top.reg_frame_err_post_en

    _tmp = (PTN_REG_READ(reg_bases[0x1]+0x0)&0xfffffffb)|0x0;
    PTN_REG_WRITE(reg_bases[0x1]+0x0, _tmp); // isp_csi_bdg0.reg_csi_in_format

    _tmp = (PTN_REG_READ(reg_bases[0x1]+0x0)&0xfffffff7)|0x0;
    PTN_REG_WRITE(reg_bases[0x1]+0x0, _tmp); // isp_csi_bdg0.reg_csi_in_yuv_format

    _tmp = (PTN_REG_READ(reg_bases[0x1]+0x0)&0xffffffcf)|0x0;
    PTN_REG_WRITE(reg_bases[0x1]+0x0, _tmp); // isp_csi_bdg0.reg_ch_num

    _tmp = (PTN_REG_READ(reg_bases[0x1]+0x0)&0xffffffbf)|0x0;
    PTN_REG_WRITE(reg_bases[0x1]+0x0, _tmp); // isp_csi_bdg0.reg_dma_wr_enable

    _tmp = (PTN_REG_READ(reg_bases[0x1]+0x0)&0xfffffeff)|0x0;
    PTN_REG_WRITE(reg_bases[0x1]+0x0, _tmp); // isp_csi_bdg0.reg_ld_ch_id

    _tmp = (PTN_REG_READ(reg_bases[0x1]+0x0)&0xfffffdff)|0x0;
    PTN_REG_WRITE(reg_bases[0x1]+0x0, _tmp); // isp_csi_bdg0.reg_y_only

    _tmp = (PTN_REG_READ(reg_bases[0x1]+0x8)&0xffffe000)|0x7f;
    PTN_REG_WRITE(reg_bases[0x1]+0x8, _tmp); // isp_csi_bdg0.reg_frame_widthm1

    _tmp = (PTN_REG_READ(reg_bases[0x1]+0x8)&0xe000ffff)|0x3f0000;
    PTN_REG_WRITE(reg_bases[0x1]+0x8, _tmp); // isp_csi_bdg0.reg_frame_heightm1

    _tmp = (PTN_REG_READ(reg_bases[0x1]+0x0)&0xfffffffc)|0x2;
    PTN_REG_WRITE(reg_bases[0x1]+0x0, _tmp); // isp_csi_bdg0.reg_csi_mode

} while (0);
