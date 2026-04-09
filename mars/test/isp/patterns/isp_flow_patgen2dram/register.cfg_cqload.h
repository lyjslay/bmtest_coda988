// arg list below is needed ========================
// @PTN_REG_READ, uint32_t (*)(uint64_t), for register read
// @PTN_REG_WRITE, void (*)(uint64_t, uint32_t data), for register write
// @PTN_MALLOC, uint64_t (*)(size_t, uint64_t), for memory allocation
// @phys_regs, uint64_t , for register base
// @reg_bases, uint64_t , for dram virtual register base
// @priv, void * , for private info, not touched
// =================================================
do {
    uint32_t _tmp;
    union CMDSET_FIELD *_ctrl;

    _tmp = PTN_REG_READ(phys_regs[0x0]+0x0);
    PTN_REG_WRITE(reg_bases[0x0]+0x0, _tmp); // pre_raw.reg_csi_bdg_out_mode
    _ctrl = (union CMDSET_FIELD *)(reg_bases[0x0]+0x0+0x4);
    _ctrl->bits.BWR_MASK = 0xF;
    _ctrl->bits.REG_ADDR = ((0xa000000) >> 2) & 0xFFFFF;

    _tmp = PTN_REG_READ(phys_regs[0x69]+0x0);
    PTN_REG_WRITE(reg_bases[0x69]+0x0, _tmp); // isp_crop4.reg_dma_enable
    _ctrl = (union CMDSET_FIELD *)(reg_bases[0x69]+0x0+0x4);
    _ctrl->bits.BWR_MASK = 0xF;
    _ctrl->bits.REG_ADDR = ((0xa069000) >> 2) & 0xFFFFF;

    _tmp = PTN_REG_READ(phys_regs[0x6a]+0x0);
    PTN_REG_WRITE(reg_bases[0x6a]+0x0, _tmp); // isp_crop5.reg_dma_enable
    _ctrl = (union CMDSET_FIELD *)(reg_bases[0x6a]+0x0+0x4);
    _ctrl->bits.BWR_MASK = 0xF;
    _ctrl->bits.REG_ADDR = ((0xa06a000) >> 2) & 0xFFFFF;

    _tmp = PTN_REG_READ(phys_regs[0x6b]+0x0);
    PTN_REG_WRITE(reg_bases[0x6b]+0x0, _tmp); // isp_crop6.reg_dma_enable
    _ctrl = (union CMDSET_FIELD *)(reg_bases[0x6b]+0x0+0x4);
    _ctrl->bits.BWR_MASK = 0xF;
    _ctrl->bits.REG_ADDR = ((0xa06b000) >> 2) & 0xFFFFF;

    _ctrl = (union CMDSET_FIELD *)(reg_bases[0x69]+0x200+0x4);
    _ctrl->bits.BWR_MASK = 0xF;
    _ctrl->bits.REG_ADDR = ((0xa069100) >> 2) & 0xFFFFF;

    _ctrl = (union CMDSET_FIELD *)(reg_bases[0x69]+0x208+0x4);
    _ctrl->bits.BWR_MASK = 0xF;
    _ctrl->bits.REG_ADDR = ((0xa069104) >> 2) & 0xFFFFF;

    _tmp = PTN_REG_READ(phys_regs[0x69]+0x108);
    PTN_REG_WRITE(reg_bases[0x69]+0x210, _tmp); // isp_dma3.reg_dma_cnt
    _ctrl = (union CMDSET_FIELD *)(reg_bases[0x69]+0x210+0x4);
    _ctrl->bits.BWR_MASK = 0xF;
    _ctrl->bits.REG_ADDR = ((0xa069108) >> 2) & 0xFFFFF;

    _tmp = PTN_REG_READ(phys_regs[0x69]+0x124);
    PTN_REG_WRITE(reg_bases[0x69]+0x248, _tmp); // isp_dma3.reg_img_width
    _ctrl = (union CMDSET_FIELD *)(reg_bases[0x69]+0x248+0x4);
    _ctrl->bits.BWR_MASK = 0xF;
    _ctrl->bits.REG_ADDR = ((0xa069124) >> 2) & 0xFFFFF;

    _tmp = PTN_REG_READ(phys_regs[0x69]+0x124);
    PTN_REG_WRITE(reg_bases[0x69]+0x248, _tmp); // isp_dma3.reg_img_height
    _ctrl = (union CMDSET_FIELD *)(reg_bases[0x69]+0x248+0x4);
    _ctrl->bits.BWR_MASK = 0xF;
    _ctrl->bits.REG_ADDR = ((0xa069124) >> 2) & 0xFFFFF;

    _tmp = PTN_REG_READ(phys_regs[0x69]+0x128);
    PTN_REG_WRITE(reg_bases[0x69]+0x250, _tmp); // isp_dma3.reg_line_offset
    _ctrl = (union CMDSET_FIELD *)(reg_bases[0x69]+0x250+0x4);
    _ctrl->bits.BWR_MASK = 0xF;
    _ctrl->bits.REG_ADDR = ((0xa069128) >> 2) & 0xFFFFF;

    _tmp = PTN_REG_READ(phys_regs[0x69]+0x128);
    PTN_REG_WRITE(reg_bases[0x69]+0x250, _tmp); // isp_dma3.reg_mod_ctl
    _ctrl = (union CMDSET_FIELD *)(reg_bases[0x69]+0x250+0x4);
    _ctrl->bits.BWR_MASK = 0xF;
    _ctrl->bits.REG_ADDR = ((0xa069128) >> 2) & 0xFFFFF;

    _tmp = PTN_REG_READ(phys_regs[0x69]+0x134);
    PTN_REG_WRITE(reg_bases[0x69]+0x268, _tmp); // isp_dma3.reg_int_sts_en
    _ctrl = (union CMDSET_FIELD *)(reg_bases[0x69]+0x268+0x4);
    _ctrl->bits.BWR_MASK = 0xF;
    _ctrl->bits.REG_ADDR = ((0xa069134) >> 2) & 0xFFFFF;

    _tmp = PTN_REG_READ(phys_regs[0x69]+0x120);
    PTN_REG_WRITE(reg_bases[0x69]+0x240, _tmp); // isp_dma3.reg_job_start
    _ctrl = (union CMDSET_FIELD *)(reg_bases[0x69]+0x240+0x4);
    _ctrl->bits.BWR_MASK = 0xF;
    _ctrl->bits.REG_ADDR = ((0xa069120) >> 2) & 0xFFFFF;

    _ctrl = (union CMDSET_FIELD *)(reg_bases[0x6a]+0x200+0x4);
    _ctrl->bits.BWR_MASK = 0xF;
    _ctrl->bits.REG_ADDR = ((0xa06a100) >> 2) & 0xFFFFF;

    _ctrl = (union CMDSET_FIELD *)(reg_bases[0x6a]+0x208+0x4);
    _ctrl->bits.BWR_MASK = 0xF;
    _ctrl->bits.REG_ADDR = ((0xa06a104) >> 2) & 0xFFFFF;

    _tmp = PTN_REG_READ(phys_regs[0x6a]+0x108);
    PTN_REG_WRITE(reg_bases[0x6a]+0x210, _tmp); // isp_dma4.reg_dma_cnt
    _ctrl = (union CMDSET_FIELD *)(reg_bases[0x6a]+0x210+0x4);
    _ctrl->bits.BWR_MASK = 0xF;
    _ctrl->bits.REG_ADDR = ((0xa06a108) >> 2) & 0xFFFFF;

    _tmp = PTN_REG_READ(phys_regs[0x6a]+0x124);
    PTN_REG_WRITE(reg_bases[0x6a]+0x248, _tmp); // isp_dma4.reg_img_width
    _ctrl = (union CMDSET_FIELD *)(reg_bases[0x6a]+0x248+0x4);
    _ctrl->bits.BWR_MASK = 0xF;
    _ctrl->bits.REG_ADDR = ((0xa06a124) >> 2) & 0xFFFFF;

    _tmp = PTN_REG_READ(phys_regs[0x6a]+0x124);
    PTN_REG_WRITE(reg_bases[0x6a]+0x248, _tmp); // isp_dma4.reg_img_height
    _ctrl = (union CMDSET_FIELD *)(reg_bases[0x6a]+0x248+0x4);
    _ctrl->bits.BWR_MASK = 0xF;
    _ctrl->bits.REG_ADDR = ((0xa06a124) >> 2) & 0xFFFFF;

    _tmp = PTN_REG_READ(phys_regs[0x6a]+0x128);
    PTN_REG_WRITE(reg_bases[0x6a]+0x250, _tmp); // isp_dma4.reg_line_offset
    _ctrl = (union CMDSET_FIELD *)(reg_bases[0x6a]+0x250+0x4);
    _ctrl->bits.BWR_MASK = 0xF;
    _ctrl->bits.REG_ADDR = ((0xa06a128) >> 2) & 0xFFFFF;

    _tmp = PTN_REG_READ(phys_regs[0x6a]+0x128);
    PTN_REG_WRITE(reg_bases[0x6a]+0x250, _tmp); // isp_dma4.reg_mod_ctl
    _ctrl = (union CMDSET_FIELD *)(reg_bases[0x6a]+0x250+0x4);
    _ctrl->bits.BWR_MASK = 0xF;
    _ctrl->bits.REG_ADDR = ((0xa06a128) >> 2) & 0xFFFFF;

    _tmp = PTN_REG_READ(phys_regs[0x6a]+0x134);
    PTN_REG_WRITE(reg_bases[0x6a]+0x268, _tmp); // isp_dma4.reg_int_sts_en
    _ctrl = (union CMDSET_FIELD *)(reg_bases[0x6a]+0x268+0x4);
    _ctrl->bits.BWR_MASK = 0xF;
    _ctrl->bits.REG_ADDR = ((0xa06a134) >> 2) & 0xFFFFF;

    _tmp = PTN_REG_READ(phys_regs[0x6a]+0x120);
    PTN_REG_WRITE(reg_bases[0x6a]+0x240, _tmp); // isp_dma4.reg_job_start
    _ctrl = (union CMDSET_FIELD *)(reg_bases[0x6a]+0x240+0x4);
    _ctrl->bits.BWR_MASK = 0xF;
    _ctrl->bits.REG_ADDR = ((0xa06a120) >> 2) & 0xFFFFF;

    _ctrl = (union CMDSET_FIELD *)(reg_bases[0x6b]+0x200+0x4);
    _ctrl->bits.BWR_MASK = 0xF;
    _ctrl->bits.REG_ADDR = ((0xa06b100) >> 2) & 0xFFFFF;

    _ctrl = (union CMDSET_FIELD *)(reg_bases[0x6b]+0x208+0x4);
    _ctrl->bits.BWR_MASK = 0xF;
    _ctrl->bits.REG_ADDR = ((0xa06b104) >> 2) & 0xFFFFF;

    _tmp = PTN_REG_READ(phys_regs[0x6b]+0x108);
    PTN_REG_WRITE(reg_bases[0x6b]+0x210, _tmp); // isp_dma5.reg_dma_cnt
    _ctrl = (union CMDSET_FIELD *)(reg_bases[0x6b]+0x210+0x4);
    _ctrl->bits.BWR_MASK = 0xF;
    _ctrl->bits.REG_ADDR = ((0xa06b108) >> 2) & 0xFFFFF;

    _tmp = PTN_REG_READ(phys_regs[0x6b]+0x124);
    PTN_REG_WRITE(reg_bases[0x6b]+0x248, _tmp); // isp_dma5.reg_img_width
    _ctrl = (union CMDSET_FIELD *)(reg_bases[0x6b]+0x248+0x4);
    _ctrl->bits.BWR_MASK = 0xF;
    _ctrl->bits.REG_ADDR = ((0xa06b124) >> 2) & 0xFFFFF;

    _tmp = PTN_REG_READ(phys_regs[0x6b]+0x124);
    PTN_REG_WRITE(reg_bases[0x6b]+0x248, _tmp); // isp_dma5.reg_img_height
    _ctrl = (union CMDSET_FIELD *)(reg_bases[0x6b]+0x248+0x4);
    _ctrl->bits.BWR_MASK = 0xF;
    _ctrl->bits.REG_ADDR = ((0xa06b124) >> 2) & 0xFFFFF;

    _tmp = PTN_REG_READ(phys_regs[0x6b]+0x128);
    PTN_REG_WRITE(reg_bases[0x6b]+0x250, _tmp); // isp_dma5.reg_line_offset
    _ctrl = (union CMDSET_FIELD *)(reg_bases[0x6b]+0x250+0x4);
    _ctrl->bits.BWR_MASK = 0xF;
    _ctrl->bits.REG_ADDR = ((0xa06b128) >> 2) & 0xFFFFF;

    _tmp = PTN_REG_READ(phys_regs[0x6b]+0x128);
    PTN_REG_WRITE(reg_bases[0x6b]+0x250, _tmp); // isp_dma5.reg_mod_ctl
    _ctrl = (union CMDSET_FIELD *)(reg_bases[0x6b]+0x250+0x4);
    _ctrl->bits.BWR_MASK = 0xF;
    _ctrl->bits.REG_ADDR = ((0xa06b128) >> 2) & 0xFFFFF;

    _tmp = PTN_REG_READ(phys_regs[0x6b]+0x134);
    PTN_REG_WRITE(reg_bases[0x6b]+0x268, _tmp); // isp_dma5.reg_int_sts_en
    _ctrl = (union CMDSET_FIELD *)(reg_bases[0x6b]+0x268+0x4);
    _ctrl->bits.BWR_MASK = 0xF;
    _ctrl->bits.REG_ADDR = ((0xa06b134) >> 2) & 0xFFFFF;

    _tmp = PTN_REG_READ(phys_regs[0x6b]+0x120);
    PTN_REG_WRITE(reg_bases[0x6b]+0x240, _tmp); // isp_dma5.reg_job_start
    _ctrl = (union CMDSET_FIELD *)(reg_bases[0x6b]+0x240+0x4);
    _ctrl->bits.BWR_MASK = 0xF;
    _ctrl->bits.REG_ADDR = ((0xa06b120) >> 2) & 0xFFFFF;

    _tmp = PTN_REG_READ(phys_regs[0x50]+0x0);
    PTN_REG_WRITE(reg_bases[0x50]+0x0, _tmp); // isp_rgb.reg_color_mode
    _ctrl = (union CMDSET_FIELD *)(reg_bases[0x50]+0x0+0x4);
    _ctrl->bits.BWR_MASK = 0xF;
    _ctrl->bits.REG_ADDR = ((0xa050000) >> 2) & 0xFFFFF;

    _tmp = PTN_REG_READ(phys_regs[0x51]+0x0);
    PTN_REG_WRITE(reg_bases[0x51]+0x0, _tmp); // isp_cfa.reg_cfa_enable
    _ctrl = (union CMDSET_FIELD *)(reg_bases[0x51]+0x0+0x4);
    _ctrl->bits.BWR_MASK = 0xF;
    _ctrl->bits.REG_ADDR = ((0xa051000) >> 2) & 0xFFFFF;

    _tmp = PTN_REG_READ(phys_regs[0x51]+0x8);
    PTN_REG_WRITE(reg_bases[0x51]+0x10, _tmp); // isp_cfa.reg_img_wd
    _ctrl = (union CMDSET_FIELD *)(reg_bases[0x51]+0x10+0x4);
    _ctrl->bits.BWR_MASK = 0xF;
    _ctrl->bits.REG_ADDR = ((0xa051008) >> 2) & 0xFFFFF;

    _tmp = PTN_REG_READ(phys_regs[0x51]+0x8);
    PTN_REG_WRITE(reg_bases[0x51]+0x10, _tmp); // isp_cfa.reg_img_ht
    _ctrl = (union CMDSET_FIELD *)(reg_bases[0x51]+0x10+0x4);
    _ctrl->bits.BWR_MASK = 0xF;
    _ctrl->bits.REG_ADDR = ((0xa051008) >> 2) & 0xFFFFF;

    _tmp = PTN_REG_READ(phys_regs[0x55]+0x30);
    PTN_REG_WRITE(reg_bases[0x55]+0x60, _tmp); // isp_dhz.reg_img_wd
    _ctrl = (union CMDSET_FIELD *)(reg_bases[0x55]+0x60+0x4);
    _ctrl->bits.BWR_MASK = 0xF;
    _ctrl->bits.REG_ADDR = ((0xa055030) >> 2) & 0xFFFFF;

    _tmp = PTN_REG_READ(phys_regs[0x55]+0x30);
    PTN_REG_WRITE(reg_bases[0x55]+0x60, _tmp); // isp_dhz.reg_img_ht
    _ctrl = (union CMDSET_FIELD *)(reg_bases[0x55]+0x60+0x4);
    _ctrl->bits.BWR_MASK = 0xF;
    _ctrl->bits.REG_ADDR = ((0xa055030) >> 2) & 0xFFFFF;

    _tmp = PTN_REG_READ(phys_regs[0x58]+0x98);
    PTN_REG_WRITE(reg_bases[0x58]+0x130, _tmp); // isp_rgbee.reg_img_wd
    _ctrl = (union CMDSET_FIELD *)(reg_bases[0x58]+0x130+0x4);
    _ctrl->bits.BWR_MASK = 0xF;
    _ctrl->bits.REG_ADDR = ((0xa058098) >> 2) & 0xFFFFF;

    _tmp = PTN_REG_READ(phys_regs[0x58]+0x98);
    PTN_REG_WRITE(reg_bases[0x58]+0x130, _tmp); // isp_rgbee.reg_img_ht
    _ctrl = (union CMDSET_FIELD *)(reg_bases[0x58]+0x130+0x4);
    _ctrl->bits.BWR_MASK = 0xF;
    _ctrl->bits.REG_ADDR = ((0xa058098) >> 2) & 0xFFFFF;

    _tmp = PTN_REG_READ(phys_regs[0x70]+0xc);
    PTN_REG_WRITE(reg_bases[0x70]+0x18, _tmp); // isp_top.reg_trig_str_sel_pre
    _ctrl = (union CMDSET_FIELD *)(reg_bases[0x70]+0x18+0x4);
    _ctrl->bits.BWR_MASK = 0xF;
    _ctrl->bits.REG_ADDR = ((0xa07000c) >> 2) & 0xFFFFF;

    _tmp = PTN_REG_READ(phys_regs[0x70]+0xc);
    PTN_REG_WRITE(reg_bases[0x70]+0x18, _tmp); // isp_top.reg_trig_str_sel_pre1
    _ctrl = (union CMDSET_FIELD *)(reg_bases[0x70]+0x18+0x4);
    _ctrl->bits.BWR_MASK = 0xF;
    _ctrl->bits.REG_ADDR = ((0xa07000c) >> 2) & 0xFFFFF;

    _tmp = PTN_REG_READ(phys_regs[0x70]+0xc);
    PTN_REG_WRITE(reg_bases[0x70]+0x18, _tmp); // isp_top.reg_trig_str_sel_post
    _ctrl = (union CMDSET_FIELD *)(reg_bases[0x70]+0x18+0x4);
    _ctrl->bits.BWR_MASK = 0xF;
    _ctrl->bits.REG_ADDR = ((0xa07000c) >> 2) & 0xFFFFF;

    _tmp = PTN_REG_READ(phys_regs[0x70]+0xc);
    PTN_REG_WRITE(reg_bases[0x70]+0x18, _tmp); // isp_top.reg_shaw_up_sel_pre
    _ctrl = (union CMDSET_FIELD *)(reg_bases[0x70]+0x18+0x4);
    _ctrl->bits.BWR_MASK = 0xF;
    _ctrl->bits.REG_ADDR = ((0xa07000c) >> 2) & 0xFFFFF;

    _tmp = PTN_REG_READ(phys_regs[0x70]+0xc);
    PTN_REG_WRITE(reg_bases[0x70]+0x18, _tmp); // isp_top.reg_shaw_up_sel_pre1
    _ctrl = (union CMDSET_FIELD *)(reg_bases[0x70]+0x18+0x4);
    _ctrl->bits.BWR_MASK = 0xF;
    _ctrl->bits.REG_ADDR = ((0xa07000c) >> 2) & 0xFFFFF;

    _tmp = PTN_REG_READ(phys_regs[0x70]+0xc);
    PTN_REG_WRITE(reg_bases[0x70]+0x18, _tmp); // isp_top.reg_shaw_up_sel_post
    _ctrl = (union CMDSET_FIELD *)(reg_bases[0x70]+0x18+0x4);
    _ctrl->bits.BWR_MASK = 0xF;
    _ctrl->bits.REG_ADDR = ((0xa07000c) >> 2) & 0xFFFFF;

    _tmp = PTN_REG_READ(phys_regs[0x70]+0x4);
    PTN_REG_WRITE(reg_bases[0x70]+0x8, _tmp); // isp_top.reg_frame_done_pre_en
    _ctrl = (union CMDSET_FIELD *)(reg_bases[0x70]+0x8+0x4);
    _ctrl->bits.BWR_MASK = 0xF;
    _ctrl->bits.REG_ADDR = ((0xa070004) >> 2) & 0xFFFFF;

    _tmp = PTN_REG_READ(phys_regs[0x70]+0x4);
    PTN_REG_WRITE(reg_bases[0x70]+0x8, _tmp); // isp_top.reg_frame_done_pre1_en
    _ctrl = (union CMDSET_FIELD *)(reg_bases[0x70]+0x8+0x4);
    _ctrl->bits.BWR_MASK = 0xF;
    _ctrl->bits.REG_ADDR = ((0xa070004) >> 2) & 0xFFFFF;

    _tmp = PTN_REG_READ(phys_regs[0x70]+0x4);
    PTN_REG_WRITE(reg_bases[0x70]+0x8, _tmp); // isp_top.reg_frame_done_post_en
    _ctrl = (union CMDSET_FIELD *)(reg_bases[0x70]+0x8+0x4);
    _ctrl->bits.BWR_MASK = 0xF;
    _ctrl->bits.REG_ADDR = ((0xa070004) >> 2) & 0xFFFFF;

    _tmp = PTN_REG_READ(phys_regs[0x70]+0x4);
    PTN_REG_WRITE(reg_bases[0x70]+0x8, _tmp); // isp_top.reg_shaw_done_pre_en
    _ctrl = (union CMDSET_FIELD *)(reg_bases[0x70]+0x8+0x4);
    _ctrl->bits.BWR_MASK = 0xF;
    _ctrl->bits.REG_ADDR = ((0xa070004) >> 2) & 0xFFFFF;

    _tmp = PTN_REG_READ(phys_regs[0x70]+0x4);
    PTN_REG_WRITE(reg_bases[0x70]+0x8, _tmp); // isp_top.reg_shaw_done_pre1_en
    _ctrl = (union CMDSET_FIELD *)(reg_bases[0x70]+0x8+0x4);
    _ctrl->bits.BWR_MASK = 0xF;
    _ctrl->bits.REG_ADDR = ((0xa070004) >> 2) & 0xFFFFF;

    _tmp = PTN_REG_READ(phys_regs[0x70]+0x4);
    PTN_REG_WRITE(reg_bases[0x70]+0x8, _tmp); // isp_top.reg_shaw_done_post_en
    _ctrl = (union CMDSET_FIELD *)(reg_bases[0x70]+0x8+0x4);
    _ctrl->bits.BWR_MASK = 0xF;
    _ctrl->bits.REG_ADDR = ((0xa070004) >> 2) & 0xFFFFF;

    _tmp = PTN_REG_READ(phys_regs[0x70]+0x4);
    PTN_REG_WRITE(reg_bases[0x70]+0x8, _tmp); // isp_top.reg_frame_err_pre_en
    _ctrl = (union CMDSET_FIELD *)(reg_bases[0x70]+0x8+0x4);
    _ctrl->bits.BWR_MASK = 0xF;
    _ctrl->bits.REG_ADDR = ((0xa070004) >> 2) & 0xFFFFF;

    _tmp = PTN_REG_READ(phys_regs[0x70]+0x4);
    PTN_REG_WRITE(reg_bases[0x70]+0x8, _tmp); // isp_top.reg_frame_err_pre1_en
    _ctrl = (union CMDSET_FIELD *)(reg_bases[0x70]+0x8+0x4);
    _ctrl->bits.BWR_MASK = 0xF;
    _ctrl->bits.REG_ADDR = ((0xa070004) >> 2) & 0xFFFFF;

    _tmp = PTN_REG_READ(phys_regs[0x70]+0x4);
    PTN_REG_WRITE(reg_bases[0x70]+0x8, _tmp); // isp_top.reg_frame_err_post_en
    _ctrl = (union CMDSET_FIELD *)(reg_bases[0x70]+0x8+0x4);
    _ctrl->bits.BWR_MASK = 0xF;
    _ctrl->bits.REG_ADDR = ((0xa070004) >> 2) & 0xFFFFF;

    _tmp = PTN_REG_READ(phys_regs[0x1]+0x0);
    PTN_REG_WRITE(reg_bases[0x1]+0x0, _tmp); // isp_csi_bdg0.reg_csi_in_format
    _ctrl = (union CMDSET_FIELD *)(reg_bases[0x1]+0x0+0x4);
    _ctrl->bits.BWR_MASK = 0xF;
    _ctrl->bits.REG_ADDR = ((0xa001000) >> 2) & 0xFFFFF;

    _tmp = PTN_REG_READ(phys_regs[0x1]+0x0);
    PTN_REG_WRITE(reg_bases[0x1]+0x0, _tmp); // isp_csi_bdg0.reg_csi_in_yuv_format
    _ctrl = (union CMDSET_FIELD *)(reg_bases[0x1]+0x0+0x4);
    _ctrl->bits.BWR_MASK = 0xF;
    _ctrl->bits.REG_ADDR = ((0xa001000) >> 2) & 0xFFFFF;

    _tmp = PTN_REG_READ(phys_regs[0x1]+0x0);
    PTN_REG_WRITE(reg_bases[0x1]+0x0, _tmp); // isp_csi_bdg0.reg_ch_num
    _ctrl = (union CMDSET_FIELD *)(reg_bases[0x1]+0x0+0x4);
    _ctrl->bits.BWR_MASK = 0xF;
    _ctrl->bits.REG_ADDR = ((0xa001000) >> 2) & 0xFFFFF;

    _tmp = PTN_REG_READ(phys_regs[0x1]+0x0);
    PTN_REG_WRITE(reg_bases[0x1]+0x0, _tmp); // isp_csi_bdg0.reg_dma_wr_enable
    _ctrl = (union CMDSET_FIELD *)(reg_bases[0x1]+0x0+0x4);
    _ctrl->bits.BWR_MASK = 0xF;
    _ctrl->bits.REG_ADDR = ((0xa001000) >> 2) & 0xFFFFF;

    _tmp = PTN_REG_READ(phys_regs[0x1]+0x0);
    PTN_REG_WRITE(reg_bases[0x1]+0x0, _tmp); // isp_csi_bdg0.reg_ld_ch_id
    _ctrl = (union CMDSET_FIELD *)(reg_bases[0x1]+0x0+0x4);
    _ctrl->bits.BWR_MASK = 0xF;
    _ctrl->bits.REG_ADDR = ((0xa001000) >> 2) & 0xFFFFF;

    _tmp = PTN_REG_READ(phys_regs[0x1]+0x0);
    PTN_REG_WRITE(reg_bases[0x1]+0x0, _tmp); // isp_csi_bdg0.reg_y_only
    _ctrl = (union CMDSET_FIELD *)(reg_bases[0x1]+0x0+0x4);
    _ctrl->bits.BWR_MASK = 0xF;
    _ctrl->bits.REG_ADDR = ((0xa001000) >> 2) & 0xFFFFF;

    _tmp = PTN_REG_READ(phys_regs[0x1]+0x0);
    PTN_REG_WRITE(reg_bases[0x1]+0x0, _tmp); // isp_csi_bdg0.reg_pxl_data_sel
    _ctrl = (union CMDSET_FIELD *)(reg_bases[0x1]+0x0+0x4);
    _ctrl->bits.BWR_MASK = 0xF;
    _ctrl->bits.REG_ADDR = ((0xa001000) >> 2) & 0xFFFFF;

    _tmp = PTN_REG_READ(phys_regs[0x1]+0x4);
    PTN_REG_WRITE(reg_bases[0x1]+0x8, _tmp); // isp_csi_bdg0.reg_frame_widthm1
    _ctrl = (union CMDSET_FIELD *)(reg_bases[0x1]+0x8+0x4);
    _ctrl->bits.BWR_MASK = 0xF;
    _ctrl->bits.REG_ADDR = ((0xa001004) >> 2) & 0xFFFFF;

    _tmp = PTN_REG_READ(phys_regs[0x1]+0x4);
    PTN_REG_WRITE(reg_bases[0x1]+0x8, _tmp); // isp_csi_bdg0.reg_frame_heightm1
    _ctrl = (union CMDSET_FIELD *)(reg_bases[0x1]+0x8+0x4);
    _ctrl->bits.BWR_MASK = 0xF;
    _ctrl->bits.REG_ADDR = ((0xa001004) >> 2) & 0xFFFFF;

    _tmp = PTN_REG_READ(phys_regs[0x1]+0xc);
    PTN_REG_WRITE(reg_bases[0x1]+0x18, _tmp); // isp_csi_bdg0.reg_pat_en
    _ctrl = (union CMDSET_FIELD *)(reg_bases[0x1]+0x18+0x4);
    _ctrl->bits.BWR_MASK = 0xF;
    _ctrl->bits.REG_ADDR = ((0xa00100c) >> 2) & 0xFFFFF;

    _tmp = PTN_REG_READ(phys_regs[0x1]+0xc);
    PTN_REG_WRITE(reg_bases[0x1]+0x18, _tmp); // isp_csi_bdg0.reg_gra_inv
    _ctrl = (union CMDSET_FIELD *)(reg_bases[0x1]+0x18+0x4);
    _ctrl->bits.BWR_MASK = 0xF;
    _ctrl->bits.REG_ADDR = ((0xa00100c) >> 2) & 0xFFFFF;

    _tmp = PTN_REG_READ(phys_regs[0x1]+0xc);
    PTN_REG_WRITE(reg_bases[0x1]+0x18, _tmp); // isp_csi_bdg0.reg_auto_en
    _ctrl = (union CMDSET_FIELD *)(reg_bases[0x1]+0x18+0x4);
    _ctrl->bits.BWR_MASK = 0xF;
    _ctrl->bits.REG_ADDR = ((0xa00100c) >> 2) & 0xFFFFF;

    _tmp = PTN_REG_READ(phys_regs[0x1]+0xc);
    PTN_REG_WRITE(reg_bases[0x1]+0x18, _tmp); // isp_csi_bdg0.reg_dith_en
    _ctrl = (union CMDSET_FIELD *)(reg_bases[0x1]+0x18+0x4);
    _ctrl->bits.BWR_MASK = 0xF;
    _ctrl->bits.REG_ADDR = ((0xa00100c) >> 2) & 0xFFFFF;

    _tmp = PTN_REG_READ(phys_regs[0x1]+0xc);
    PTN_REG_WRITE(reg_bases[0x1]+0x18, _tmp); // isp_csi_bdg0.reg_snow_en
    _ctrl = (union CMDSET_FIELD *)(reg_bases[0x1]+0x18+0x4);
    _ctrl->bits.BWR_MASK = 0xF;
    _ctrl->bits.REG_ADDR = ((0xa00100c) >> 2) & 0xFFFFF;

    _tmp = PTN_REG_READ(phys_regs[0x1]+0xc);
    PTN_REG_WRITE(reg_bases[0x1]+0x18, _tmp); // isp_csi_bdg0.reg_fix_mc
    _ctrl = (union CMDSET_FIELD *)(reg_bases[0x1]+0x18+0x4);
    _ctrl->bits.BWR_MASK = 0xF;
    _ctrl->bits.REG_ADDR = ((0xa00100c) >> 2) & 0xFFFFF;

    _tmp = PTN_REG_READ(phys_regs[0x1]+0xc);
    PTN_REG_WRITE(reg_bases[0x1]+0x18, _tmp); // isp_csi_bdg0.reg_dith_md
    _ctrl = (union CMDSET_FIELD *)(reg_bases[0x1]+0x18+0x4);
    _ctrl->bits.BWR_MASK = 0xF;
    _ctrl->bits.REG_ADDR = ((0xa00100c) >> 2) & 0xFFFFF;

    _tmp = PTN_REG_READ(phys_regs[0x1]+0xc);
    PTN_REG_WRITE(reg_bases[0x1]+0x18, _tmp); // isp_csi_bdg0.reg_bayer_id
    _ctrl = (union CMDSET_FIELD *)(reg_bases[0x1]+0x18+0x4);
    _ctrl->bits.BWR_MASK = 0xF;
    _ctrl->bits.REG_ADDR = ((0xa00100c) >> 2) & 0xFFFFF;

    _tmp = PTN_REG_READ(phys_regs[0x1]+0x10);
    PTN_REG_WRITE(reg_bases[0x1]+0x20, _tmp); // isp_csi_bdg0.reg_pat_prd
    _ctrl = (union CMDSET_FIELD *)(reg_bases[0x1]+0x20+0x4);
    _ctrl->bits.BWR_MASK = 0xF;
    _ctrl->bits.REG_ADDR = ((0xa001010) >> 2) & 0xFFFFF;

    _tmp = PTN_REG_READ(phys_regs[0x1]+0x10);
    PTN_REG_WRITE(reg_bases[0x1]+0x20, _tmp); // isp_csi_bdg0.reg_pat_idx
    _ctrl = (union CMDSET_FIELD *)(reg_bases[0x1]+0x20+0x4);
    _ctrl->bits.BWR_MASK = 0xF;
    _ctrl->bits.REG_ADDR = ((0xa001010) >> 2) & 0xFFFFF;

    _tmp = PTN_REG_READ(phys_regs[0x1]+0x14);
    PTN_REG_WRITE(reg_bases[0x1]+0x28, _tmp); // isp_csi_bdg0.reg_pat_r
    _ctrl = (union CMDSET_FIELD *)(reg_bases[0x1]+0x28+0x4);
    _ctrl->bits.BWR_MASK = 0xF;
    _ctrl->bits.REG_ADDR = ((0xa001014) >> 2) & 0xFFFFF;

    _tmp = PTN_REG_READ(phys_regs[0x1]+0x14);
    PTN_REG_WRITE(reg_bases[0x1]+0x28, _tmp); // isp_csi_bdg0.reg_pat_g
    _ctrl = (union CMDSET_FIELD *)(reg_bases[0x1]+0x28+0x4);
    _ctrl->bits.BWR_MASK = 0xF;
    _ctrl->bits.REG_ADDR = ((0xa001014) >> 2) & 0xFFFFF;

    _tmp = PTN_REG_READ(phys_regs[0x1]+0x18);
    PTN_REG_WRITE(reg_bases[0x1]+0x30, _tmp); // isp_csi_bdg0.reg_pat_b
    _ctrl = (union CMDSET_FIELD *)(reg_bases[0x1]+0x30+0x4);
    _ctrl->bits.BWR_MASK = 0xF;
    _ctrl->bits.REG_ADDR = ((0xa001018) >> 2) & 0xFFFFF;

    _tmp = PTN_REG_READ(phys_regs[0x1]+0x1c);
    PTN_REG_WRITE(reg_bases[0x1]+0x38, _tmp); // isp_csi_bdg0.reg_fde_r
    _ctrl = (union CMDSET_FIELD *)(reg_bases[0x1]+0x38+0x4);
    _ctrl->bits.BWR_MASK = 0xF;
    _ctrl->bits.REG_ADDR = ((0xa00101c) >> 2) & 0xFFFFF;

    _tmp = PTN_REG_READ(phys_regs[0x1]+0x1c);
    PTN_REG_WRITE(reg_bases[0x1]+0x38, _tmp); // isp_csi_bdg0.reg_fde_g
    _ctrl = (union CMDSET_FIELD *)(reg_bases[0x1]+0x38+0x4);
    _ctrl->bits.BWR_MASK = 0xF;
    _ctrl->bits.REG_ADDR = ((0xa00101c) >> 2) & 0xFFFFF;

    _tmp = PTN_REG_READ(phys_regs[0x1]+0x20);
    PTN_REG_WRITE(reg_bases[0x1]+0x40, _tmp); // isp_csi_bdg0.reg_fde_b
    _ctrl = (union CMDSET_FIELD *)(reg_bases[0x1]+0x40+0x4);
    _ctrl->bits.BWR_MASK = 0xF;
    _ctrl->bits.REG_ADDR = ((0xa001020) >> 2) & 0xFFFFF;

    _tmp = PTN_REG_READ(phys_regs[0x1]+0x24);
    PTN_REG_WRITE(reg_bases[0x1]+0x48, _tmp); // isp_csi_bdg0.reg_mde_r
    _ctrl = (union CMDSET_FIELD *)(reg_bases[0x1]+0x48+0x4);
    _ctrl->bits.BWR_MASK = 0xF;
    _ctrl->bits.REG_ADDR = ((0xa001024) >> 2) & 0xFFFFF;

    _tmp = PTN_REG_READ(phys_regs[0x1]+0x24);
    PTN_REG_WRITE(reg_bases[0x1]+0x48, _tmp); // isp_csi_bdg0.reg_mde_g
    _ctrl = (union CMDSET_FIELD *)(reg_bases[0x1]+0x48+0x4);
    _ctrl->bits.BWR_MASK = 0xF;
    _ctrl->bits.REG_ADDR = ((0xa001024) >> 2) & 0xFFFFF;

    _tmp = PTN_REG_READ(phys_regs[0x1]+0x28);
    PTN_REG_WRITE(reg_bases[0x1]+0x50, _tmp); // isp_csi_bdg0.reg_mde_b
    _ctrl = (union CMDSET_FIELD *)(reg_bases[0x1]+0x50+0x4);
    _ctrl->bits.BWR_MASK = 0xF;
    _ctrl->bits.REG_ADDR = ((0xa001028) >> 2) & 0xFFFFF;

    _tmp = PTN_REG_READ(phys_regs[0x1]+0x2c);
    PTN_REG_WRITE(reg_bases[0x1]+0x58, _tmp); // isp_csi_bdg0.reg_vmde_str
    _ctrl = (union CMDSET_FIELD *)(reg_bases[0x1]+0x58+0x4);
    _ctrl->bits.BWR_MASK = 0xF;
    _ctrl->bits.REG_ADDR = ((0xa00102c) >> 2) & 0xFFFFF;

    _tmp = PTN_REG_READ(phys_regs[0x1]+0x2c);
    PTN_REG_WRITE(reg_bases[0x1]+0x58, _tmp); // isp_csi_bdg0.reg_vmde_stp
    _ctrl = (union CMDSET_FIELD *)(reg_bases[0x1]+0x58+0x4);
    _ctrl->bits.BWR_MASK = 0xF;
    _ctrl->bits.REG_ADDR = ((0xa00102c) >> 2) & 0xFFFFF;

    _tmp = PTN_REG_READ(phys_regs[0x1]+0x30);
    PTN_REG_WRITE(reg_bases[0x1]+0x60, _tmp); // isp_csi_bdg0.reg_hmde_str
    _ctrl = (union CMDSET_FIELD *)(reg_bases[0x1]+0x60+0x4);
    _ctrl->bits.BWR_MASK = 0xF;
    _ctrl->bits.REG_ADDR = ((0xa001030) >> 2) & 0xFFFFF;

    _tmp = PTN_REG_READ(phys_regs[0x1]+0x30);
    PTN_REG_WRITE(reg_bases[0x1]+0x60, _tmp); // isp_csi_bdg0.reg_hmde_stp
    _ctrl = (union CMDSET_FIELD *)(reg_bases[0x1]+0x60+0x4);
    _ctrl->bits.BWR_MASK = 0xF;
    _ctrl->bits.REG_ADDR = ((0xa001030) >> 2) & 0xFFFFF;

    _tmp = PTN_REG_READ(phys_regs[0x1]+0x34);
    PTN_REG_WRITE(reg_bases[0x1]+0x68, _tmp); // isp_csi_bdg0.reg_vfde_str
    _ctrl = (union CMDSET_FIELD *)(reg_bases[0x1]+0x68+0x4);
    _ctrl->bits.BWR_MASK = 0xF;
    _ctrl->bits.REG_ADDR = ((0xa001034) >> 2) & 0xFFFFF;

    _tmp = PTN_REG_READ(phys_regs[0x1]+0x34);
    PTN_REG_WRITE(reg_bases[0x1]+0x68, _tmp); // isp_csi_bdg0.reg_vfde_stp
    _ctrl = (union CMDSET_FIELD *)(reg_bases[0x1]+0x68+0x4);
    _ctrl->bits.BWR_MASK = 0xF;
    _ctrl->bits.REG_ADDR = ((0xa001034) >> 2) & 0xFFFFF;

    _tmp = PTN_REG_READ(phys_regs[0x1]+0x38);
    PTN_REG_WRITE(reg_bases[0x1]+0x70, _tmp); // isp_csi_bdg0.reg_hfde_str
    _ctrl = (union CMDSET_FIELD *)(reg_bases[0x1]+0x70+0x4);
    _ctrl->bits.BWR_MASK = 0xF;
    _ctrl->bits.REG_ADDR = ((0xa001038) >> 2) & 0xFFFFF;

    _tmp = PTN_REG_READ(phys_regs[0x1]+0x38);
    PTN_REG_WRITE(reg_bases[0x1]+0x70, _tmp); // isp_csi_bdg0.reg_hfde_stp
    _ctrl = (union CMDSET_FIELD *)(reg_bases[0x1]+0x70+0x4);
    _ctrl->bits.BWR_MASK = 0xF;
    _ctrl->bits.REG_ADDR = ((0xa001038) >> 2) & 0xFFFFF;

    _tmp = PTN_REG_READ(phys_regs[0x1]+0x3c);
    PTN_REG_WRITE(reg_bases[0x1]+0x78, _tmp); // isp_csi_bdg0.reg_hs_str
    _ctrl = (union CMDSET_FIELD *)(reg_bases[0x1]+0x78+0x4);
    _ctrl->bits.BWR_MASK = 0xF;
    _ctrl->bits.REG_ADDR = ((0xa00103c) >> 2) & 0xFFFFF;

    _tmp = PTN_REG_READ(phys_regs[0x1]+0x3c);
    PTN_REG_WRITE(reg_bases[0x1]+0x78, _tmp); // isp_csi_bdg0.reg_hs_stp
    _ctrl = (union CMDSET_FIELD *)(reg_bases[0x1]+0x78+0x4);
    _ctrl->bits.BWR_MASK = 0xF;
    _ctrl->bits.REG_ADDR = ((0xa00103c) >> 2) & 0xFFFFF;

    _tmp = PTN_REG_READ(phys_regs[0x1]+0x40);
    PTN_REG_WRITE(reg_bases[0x1]+0x80, _tmp); // isp_csi_bdg0.reg_vs_str
    _ctrl = (union CMDSET_FIELD *)(reg_bases[0x1]+0x80+0x4);
    _ctrl->bits.BWR_MASK = 0xF;
    _ctrl->bits.REG_ADDR = ((0xa001040) >> 2) & 0xFFFFF;

    _tmp = PTN_REG_READ(phys_regs[0x1]+0x40);
    PTN_REG_WRITE(reg_bases[0x1]+0x80, _tmp); // isp_csi_bdg0.reg_vs_stp
    _ctrl = (union CMDSET_FIELD *)(reg_bases[0x1]+0x80+0x4);
    _ctrl->bits.BWR_MASK = 0xF;
    _ctrl->bits.REG_ADDR = ((0xa001040) >> 2) & 0xFFFFF;

    _tmp = PTN_REG_READ(phys_regs[0x1]+0x44);
    PTN_REG_WRITE(reg_bases[0x1]+0x88, _tmp); // isp_csi_bdg0.reg_htt
    _ctrl = (union CMDSET_FIELD *)(reg_bases[0x1]+0x88+0x4);
    _ctrl->bits.BWR_MASK = 0xF;
    _ctrl->bits.REG_ADDR = ((0xa001044) >> 2) & 0xFFFFF;

    _tmp = PTN_REG_READ(phys_regs[0x1]+0x44);
    PTN_REG_WRITE(reg_bases[0x1]+0x88, _tmp); // isp_csi_bdg0.reg_vtt
    _ctrl = (union CMDSET_FIELD *)(reg_bases[0x1]+0x88+0x4);
    _ctrl->bits.BWR_MASK = 0xF;
    _ctrl->bits.REG_ADDR = ((0xa001044) >> 2) & 0xFFFFF;

    _tmp = PTN_REG_READ(phys_regs[0x1]+0x0);
    PTN_REG_WRITE(reg_bases[0x1]+0x0, _tmp); // isp_csi_bdg0.reg_csi_mode
    _ctrl = (union CMDSET_FIELD *)(reg_bases[0x1]+0x0+0x4);
    _ctrl->bits.BWR_MASK = 0xF;
    _ctrl->bits.REG_ADDR = ((0xa001000) >> 2) & 0xFFFFF;

} while (0);
