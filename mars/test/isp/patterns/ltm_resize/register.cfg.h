// arg list below is needed ========================
// @PTN_REG_READ, uint32_t (*)(uint64_t), for register read
// @PTN_REG_WRITE, void (*)(uint64_t, uint32_t data), for register write
// @PTN_MALLOC, uint64_t (*)(size_t, uint64_t), for memory allocation
// @phys_regs, uint64_t , for register base
// @priv, void * , for private info, not touched
// =================================================
do {
    uint32_t _tmp;

    _tmp = (PTN_REG_READ(phys_regs[0x3b]+0x140)&0x7fffffff)|0x80000000;
    PTN_REG_WRITE(phys_regs[0x3b]+0x140, _tmp); // isp_ltm.reg_resize_coeff_buff_wen

    _tmp = (PTN_REG_READ(phys_regs[0x3b]+0x140)&0xffffff80)|0x0;
    PTN_REG_WRITE(phys_regs[0x3b]+0x140, _tmp); // isp_ltm.reg_resize_coeff_buff_addr

    _tmp = (PTN_REG_READ(phys_regs[0x3b]+0x140)&0xffffff80)|0x0;
    PTN_REG_WRITE(phys_regs[0x3b]+0x140, _tmp); // isp_ltm.reg_resize_coeff_buff_addr

    _tmp = (PTN_REG_READ(phys_regs[0x3b]+0x144)&0xfc000000)|0x368108;
    PTN_REG_WRITE(phys_regs[0x3b]+0x144, _tmp); // isp_ltm.reg_resize_coeff_buff_wdata_0

    _tmp = (PTN_REG_READ(phys_regs[0x3b]+0x148)&0xfc000000)|0x76108;
    PTN_REG_WRITE(phys_regs[0x3b]+0x148, _tmp); // isp_ltm.reg_resize_coeff_buff_wdata_1

    _tmp = (PTN_REG_READ(phys_regs[0x3b]+0x140)&0xffffff80)|0x1;
    PTN_REG_WRITE(phys_regs[0x3b]+0x140, _tmp); // isp_ltm.reg_resize_coeff_buff_addr

    _tmp = (PTN_REG_READ(phys_regs[0x3b]+0x144)&0xfc000000)|0x368106;
    PTN_REG_WRITE(phys_regs[0x3b]+0x144, _tmp); // isp_ltm.reg_resize_coeff_buff_wdata_0

    _tmp = (PTN_REG_READ(phys_regs[0x3b]+0x148)&0xfc000000)|0x7810a;
    PTN_REG_WRITE(phys_regs[0x3b]+0x148, _tmp); // isp_ltm.reg_resize_coeff_buff_wdata_1

    _tmp = (PTN_REG_READ(phys_regs[0x3b]+0x140)&0xffffff80)|0x2;
    PTN_REG_WRITE(phys_regs[0x3b]+0x140, _tmp); // isp_ltm.reg_resize_coeff_buff_addr

    _tmp = (PTN_REG_READ(phys_regs[0x3b]+0x144)&0xfc000000)|0x366104;
    PTN_REG_WRITE(phys_regs[0x3b]+0x144, _tmp); // isp_ltm.reg_resize_coeff_buff_wdata_0

    _tmp = (PTN_REG_READ(phys_regs[0x3b]+0x148)&0xfc000000)|0x7a10c;
    PTN_REG_WRITE(phys_regs[0x3b]+0x148, _tmp); // isp_ltm.reg_resize_coeff_buff_wdata_1

    _tmp = (PTN_REG_READ(phys_regs[0x3b]+0x140)&0xffffff80)|0x3;
    PTN_REG_WRITE(phys_regs[0x3b]+0x140, _tmp); // isp_ltm.reg_resize_coeff_buff_addr

    _tmp = (PTN_REG_READ(phys_regs[0x3b]+0x144)&0xfc000000)|0x366102;
    PTN_REG_WRITE(phys_regs[0x3b]+0x144, _tmp); // isp_ltm.reg_resize_coeff_buff_wdata_0

    _tmp = (PTN_REG_READ(phys_regs[0x3b]+0x148)&0xfc000000)|0x7c10e;
    PTN_REG_WRITE(phys_regs[0x3b]+0x148, _tmp); // isp_ltm.reg_resize_coeff_buff_wdata_1

    _tmp = (PTN_REG_READ(phys_regs[0x3b]+0x140)&0xffffff80)|0x4;
    PTN_REG_WRITE(phys_regs[0x3b]+0x140, _tmp); // isp_ltm.reg_resize_coeff_buff_addr

    _tmp = (PTN_REG_READ(phys_regs[0x3b]+0x144)&0xfc000000)|0x3640ff;
    PTN_REG_WRITE(phys_regs[0x3b]+0x144, _tmp); // isp_ltm.reg_resize_coeff_buff_wdata_0

    _tmp = (PTN_REG_READ(phys_regs[0x3b]+0x148)&0xfc000000)|0x7e110;
    PTN_REG_WRITE(phys_regs[0x3b]+0x148, _tmp); // isp_ltm.reg_resize_coeff_buff_wdata_1

    _tmp = (PTN_REG_READ(phys_regs[0x3b]+0x140)&0xffffff80)|0x5;
    PTN_REG_WRITE(phys_regs[0x3b]+0x140, _tmp); // isp_ltm.reg_resize_coeff_buff_addr

    _tmp = (PTN_REG_READ(phys_regs[0x3b]+0x144)&0xfc000000)|0x3640fd;
    PTN_REG_WRITE(phys_regs[0x3b]+0x144, _tmp); // isp_ltm.reg_resize_coeff_buff_wdata_0

    _tmp = (PTN_REG_READ(phys_regs[0x3b]+0x148)&0xfc000000)|0x7e112;
    PTN_REG_WRITE(phys_regs[0x3b]+0x148, _tmp); // isp_ltm.reg_resize_coeff_buff_wdata_1

    _tmp = (PTN_REG_READ(phys_regs[0x3b]+0x140)&0xffffff80)|0x6;
    PTN_REG_WRITE(phys_regs[0x3b]+0x140, _tmp); // isp_ltm.reg_resize_coeff_buff_addr

    _tmp = (PTN_REG_READ(phys_regs[0x3b]+0x144)&0xfc000000)|0x3620fb;
    PTN_REG_WRITE(phys_regs[0x3b]+0x144, _tmp); // isp_ltm.reg_resize_coeff_buff_wdata_0

    _tmp = (PTN_REG_READ(phys_regs[0x3b]+0x148)&0xfc000000)|0x80113;
    PTN_REG_WRITE(phys_regs[0x3b]+0x148, _tmp); // isp_ltm.reg_resize_coeff_buff_wdata_1

    _tmp = (PTN_REG_READ(phys_regs[0x3b]+0x140)&0xffffff80)|0x7;
    PTN_REG_WRITE(phys_regs[0x3b]+0x140, _tmp); // isp_ltm.reg_resize_coeff_buff_addr

    _tmp = (PTN_REG_READ(phys_regs[0x3b]+0x144)&0xfc000000)|0x3620f9;
    PTN_REG_WRITE(phys_regs[0x3b]+0x144, _tmp); // isp_ltm.reg_resize_coeff_buff_wdata_0

    _tmp = (PTN_REG_READ(phys_regs[0x3b]+0x148)&0xfc000000)|0x82115;
    PTN_REG_WRITE(phys_regs[0x3b]+0x148, _tmp); // isp_ltm.reg_resize_coeff_buff_wdata_1

    _tmp = (PTN_REG_READ(phys_regs[0x3b]+0x140)&0xffffff80)|0x8;
    PTN_REG_WRITE(phys_regs[0x3b]+0x140, _tmp); // isp_ltm.reg_resize_coeff_buff_addr

    _tmp = (PTN_REG_READ(phys_regs[0x3b]+0x144)&0xfc000000)|0x3600f6;
    PTN_REG_WRITE(phys_regs[0x3b]+0x144, _tmp); // isp_ltm.reg_resize_coeff_buff_wdata_0

    _tmp = (PTN_REG_READ(phys_regs[0x3b]+0x148)&0xfc000000)|0x84117;
    PTN_REG_WRITE(phys_regs[0x3b]+0x148, _tmp); // isp_ltm.reg_resize_coeff_buff_wdata_1

    _tmp = (PTN_REG_READ(phys_regs[0x3b]+0x140)&0xffffff80)|0x9;
    PTN_REG_WRITE(phys_regs[0x3b]+0x140, _tmp); // isp_ltm.reg_resize_coeff_buff_addr

    _tmp = (PTN_REG_READ(phys_regs[0x3b]+0x144)&0xfc000000)|0x3600f4;
    PTN_REG_WRITE(phys_regs[0x3b]+0x144, _tmp); // isp_ltm.reg_resize_coeff_buff_wdata_0

    _tmp = (PTN_REG_READ(phys_regs[0x3b]+0x148)&0xfc000000)|0x86119;
    PTN_REG_WRITE(phys_regs[0x3b]+0x148, _tmp); // isp_ltm.reg_resize_coeff_buff_wdata_1

    _tmp = (PTN_REG_READ(phys_regs[0x3b]+0x140)&0xffffff80)|0xa;
    PTN_REG_WRITE(phys_regs[0x3b]+0x140, _tmp); // isp_ltm.reg_resize_coeff_buff_addr

    _tmp = (PTN_REG_READ(phys_regs[0x3b]+0x144)&0xfc000000)|0x35e0f2;
    PTN_REG_WRITE(phys_regs[0x3b]+0x144, _tmp); // isp_ltm.reg_resize_coeff_buff_wdata_0

    _tmp = (PTN_REG_READ(phys_regs[0x3b]+0x148)&0xfc000000)|0x8811b;
    PTN_REG_WRITE(phys_regs[0x3b]+0x148, _tmp); // isp_ltm.reg_resize_coeff_buff_wdata_1

    _tmp = (PTN_REG_READ(phys_regs[0x3b]+0x140)&0xffffff80)|0xb;
    PTN_REG_WRITE(phys_regs[0x3b]+0x140, _tmp); // isp_ltm.reg_resize_coeff_buff_addr

    _tmp = (PTN_REG_READ(phys_regs[0x3b]+0x144)&0xfc000000)|0x35e0f0;
    PTN_REG_WRITE(phys_regs[0x3b]+0x144, _tmp); // isp_ltm.reg_resize_coeff_buff_wdata_0

    _tmp = (PTN_REG_READ(phys_regs[0x3b]+0x148)&0xfc000000)|0x8a11d;
    PTN_REG_WRITE(phys_regs[0x3b]+0x148, _tmp); // isp_ltm.reg_resize_coeff_buff_wdata_1

    _tmp = (PTN_REG_READ(phys_regs[0x3b]+0x140)&0xffffff80)|0xc;
    PTN_REG_WRITE(phys_regs[0x3b]+0x140, _tmp); // isp_ltm.reg_resize_coeff_buff_addr

    _tmp = (PTN_REG_READ(phys_regs[0x3b]+0x144)&0xfc000000)|0x35c0ed;
    PTN_REG_WRITE(phys_regs[0x3b]+0x144, _tmp); // isp_ltm.reg_resize_coeff_buff_wdata_0

    _tmp = (PTN_REG_READ(phys_regs[0x3b]+0x148)&0xfc000000)|0x8c11e;
    PTN_REG_WRITE(phys_regs[0x3b]+0x148, _tmp); // isp_ltm.reg_resize_coeff_buff_wdata_1

    _tmp = (PTN_REG_READ(phys_regs[0x3b]+0x140)&0xffffff80)|0xd;
    PTN_REG_WRITE(phys_regs[0x3b]+0x140, _tmp); // isp_ltm.reg_resize_coeff_buff_addr

    _tmp = (PTN_REG_READ(phys_regs[0x3b]+0x144)&0xfc000000)|0x35a0eb;
    PTN_REG_WRITE(phys_regs[0x3b]+0x144, _tmp); // isp_ltm.reg_resize_coeff_buff_wdata_0

    _tmp = (PTN_REG_READ(phys_regs[0x3b]+0x148)&0xfc000000)|0x8e120;
    PTN_REG_WRITE(phys_regs[0x3b]+0x148, _tmp); // isp_ltm.reg_resize_coeff_buff_wdata_1

    _tmp = (PTN_REG_READ(phys_regs[0x3b]+0x140)&0xffffff80)|0xe;
    PTN_REG_WRITE(phys_regs[0x3b]+0x140, _tmp); // isp_ltm.reg_resize_coeff_buff_addr

    _tmp = (PTN_REG_READ(phys_regs[0x3b]+0x144)&0xfc000000)|0x35a0e9;
    PTN_REG_WRITE(phys_regs[0x3b]+0x144, _tmp); // isp_ltm.reg_resize_coeff_buff_wdata_0

    _tmp = (PTN_REG_READ(phys_regs[0x3b]+0x148)&0xfc000000)|0x90122;
    PTN_REG_WRITE(phys_regs[0x3b]+0x148, _tmp); // isp_ltm.reg_resize_coeff_buff_wdata_1

    _tmp = (PTN_REG_READ(phys_regs[0x3b]+0x140)&0xffffff80)|0xf;
    PTN_REG_WRITE(phys_regs[0x3b]+0x140, _tmp); // isp_ltm.reg_resize_coeff_buff_addr

    _tmp = (PTN_REG_READ(phys_regs[0x3b]+0x144)&0xfc000000)|0x3580e7;
    PTN_REG_WRITE(phys_regs[0x3b]+0x144, _tmp); // isp_ltm.reg_resize_coeff_buff_wdata_0

    _tmp = (PTN_REG_READ(phys_regs[0x3b]+0x148)&0xfc000000)|0x92124;
    PTN_REG_WRITE(phys_regs[0x3b]+0x148, _tmp); // isp_ltm.reg_resize_coeff_buff_wdata_1

    _tmp = (PTN_REG_READ(phys_regs[0x3b]+0x140)&0xffffff80)|0x10;
    PTN_REG_WRITE(phys_regs[0x3b]+0x140, _tmp); // isp_ltm.reg_resize_coeff_buff_addr

    _tmp = (PTN_REG_READ(phys_regs[0x3b]+0x144)&0xfc000000)|0x3560e5;
    PTN_REG_WRITE(phys_regs[0x3b]+0x144, _tmp); // isp_ltm.reg_resize_coeff_buff_wdata_0

    _tmp = (PTN_REG_READ(phys_regs[0x3b]+0x148)&0xfc000000)|0x94126;
    PTN_REG_WRITE(phys_regs[0x3b]+0x148, _tmp); // isp_ltm.reg_resize_coeff_buff_wdata_1

    _tmp = (PTN_REG_READ(phys_regs[0x3b]+0x140)&0xffffff80)|0x11;
    PTN_REG_WRITE(phys_regs[0x3b]+0x140, _tmp); // isp_ltm.reg_resize_coeff_buff_addr

    _tmp = (PTN_REG_READ(phys_regs[0x3b]+0x144)&0xfc000000)|0x3560e3;
    PTN_REG_WRITE(phys_regs[0x3b]+0x144, _tmp); // isp_ltm.reg_resize_coeff_buff_wdata_0

    _tmp = (PTN_REG_READ(phys_regs[0x3b]+0x148)&0xfc000000)|0x96128;
    PTN_REG_WRITE(phys_regs[0x3b]+0x148, _tmp); // isp_ltm.reg_resize_coeff_buff_wdata_1

    _tmp = (PTN_REG_READ(phys_regs[0x3b]+0x140)&0xffffff80)|0x12;
    PTN_REG_WRITE(phys_regs[0x3b]+0x140, _tmp); // isp_ltm.reg_resize_coeff_buff_addr

    _tmp = (PTN_REG_READ(phys_regs[0x3b]+0x144)&0xfc000000)|0x3540e0;
    PTN_REG_WRITE(phys_regs[0x3b]+0x144, _tmp); // isp_ltm.reg_resize_coeff_buff_wdata_0

    _tmp = (PTN_REG_READ(phys_regs[0x3b]+0x148)&0xfc000000)|0x98129;
    PTN_REG_WRITE(phys_regs[0x3b]+0x148, _tmp); // isp_ltm.reg_resize_coeff_buff_wdata_1

    _tmp = (PTN_REG_READ(phys_regs[0x3b]+0x140)&0xffffff80)|0x13;
    PTN_REG_WRITE(phys_regs[0x3b]+0x140, _tmp); // isp_ltm.reg_resize_coeff_buff_addr

    _tmp = (PTN_REG_READ(phys_regs[0x3b]+0x144)&0xfc000000)|0x3520de;
    PTN_REG_WRITE(phys_regs[0x3b]+0x144, _tmp); // isp_ltm.reg_resize_coeff_buff_wdata_0

    _tmp = (PTN_REG_READ(phys_regs[0x3b]+0x148)&0xfc000000)|0x9a12b;
    PTN_REG_WRITE(phys_regs[0x3b]+0x148, _tmp); // isp_ltm.reg_resize_coeff_buff_wdata_1

    _tmp = (PTN_REG_READ(phys_regs[0x3b]+0x140)&0xffffff80)|0x14;
    PTN_REG_WRITE(phys_regs[0x3b]+0x140, _tmp); // isp_ltm.reg_resize_coeff_buff_addr

    _tmp = (PTN_REG_READ(phys_regs[0x3b]+0x144)&0xfc000000)|0x3500dc;
    PTN_REG_WRITE(phys_regs[0x3b]+0x144, _tmp); // isp_ltm.reg_resize_coeff_buff_wdata_0

    _tmp = (PTN_REG_READ(phys_regs[0x3b]+0x148)&0xfc000000)|0x9e12d;
    PTN_REG_WRITE(phys_regs[0x3b]+0x148, _tmp); // isp_ltm.reg_resize_coeff_buff_wdata_1

    _tmp = (PTN_REG_READ(phys_regs[0x3b]+0x140)&0xffffff80)|0x15;
    PTN_REG_WRITE(phys_regs[0x3b]+0x140, _tmp); // isp_ltm.reg_resize_coeff_buff_addr

    _tmp = (PTN_REG_READ(phys_regs[0x3b]+0x144)&0xfc000000)|0x3500da;
    PTN_REG_WRITE(phys_regs[0x3b]+0x144, _tmp); // isp_ltm.reg_resize_coeff_buff_wdata_0

    _tmp = (PTN_REG_READ(phys_regs[0x3b]+0x148)&0xfc000000)|0xa012f;
    PTN_REG_WRITE(phys_regs[0x3b]+0x148, _tmp); // isp_ltm.reg_resize_coeff_buff_wdata_1

    _tmp = (PTN_REG_READ(phys_regs[0x3b]+0x140)&0xffffff80)|0x16;
    PTN_REG_WRITE(phys_regs[0x3b]+0x140, _tmp); // isp_ltm.reg_resize_coeff_buff_addr

    _tmp = (PTN_REG_READ(phys_regs[0x3b]+0x144)&0xfc000000)|0x34e0d8;
    PTN_REG_WRITE(phys_regs[0x3b]+0x144, _tmp); // isp_ltm.reg_resize_coeff_buff_wdata_0

    _tmp = (PTN_REG_READ(phys_regs[0x3b]+0x148)&0xfc000000)|0xa2131;
    PTN_REG_WRITE(phys_regs[0x3b]+0x148, _tmp); // isp_ltm.reg_resize_coeff_buff_wdata_1

    _tmp = (PTN_REG_READ(phys_regs[0x3b]+0x140)&0xffffff80)|0x17;
    PTN_REG_WRITE(phys_regs[0x3b]+0x140, _tmp); // isp_ltm.reg_resize_coeff_buff_addr

    _tmp = (PTN_REG_READ(phys_regs[0x3b]+0x144)&0xfc000000)|0x34c0d6;
    PTN_REG_WRITE(phys_regs[0x3b]+0x144, _tmp); // isp_ltm.reg_resize_coeff_buff_wdata_0

    _tmp = (PTN_REG_READ(phys_regs[0x3b]+0x148)&0xfc000000)|0xa4132;
    PTN_REG_WRITE(phys_regs[0x3b]+0x148, _tmp); // isp_ltm.reg_resize_coeff_buff_wdata_1

    _tmp = (PTN_REG_READ(phys_regs[0x3b]+0x140)&0xffffff80)|0x18;
    PTN_REG_WRITE(phys_regs[0x3b]+0x140, _tmp); // isp_ltm.reg_resize_coeff_buff_addr

    _tmp = (PTN_REG_READ(phys_regs[0x3b]+0x144)&0xfc000000)|0x34a0d4;
    PTN_REG_WRITE(phys_regs[0x3b]+0x144, _tmp); // isp_ltm.reg_resize_coeff_buff_wdata_0

    _tmp = (PTN_REG_READ(phys_regs[0x3b]+0x148)&0xfc000000)|0xa6134;
    PTN_REG_WRITE(phys_regs[0x3b]+0x148, _tmp); // isp_ltm.reg_resize_coeff_buff_wdata_1

    _tmp = (PTN_REG_READ(phys_regs[0x3b]+0x140)&0xffffff80)|0x19;
    PTN_REG_WRITE(phys_regs[0x3b]+0x140, _tmp); // isp_ltm.reg_resize_coeff_buff_addr

    _tmp = (PTN_REG_READ(phys_regs[0x3b]+0x144)&0xfc000000)|0x3480d2;
    PTN_REG_WRITE(phys_regs[0x3b]+0x144, _tmp); // isp_ltm.reg_resize_coeff_buff_wdata_0

    _tmp = (PTN_REG_READ(phys_regs[0x3b]+0x148)&0xfc000000)|0xa8136;
    PTN_REG_WRITE(phys_regs[0x3b]+0x148, _tmp); // isp_ltm.reg_resize_coeff_buff_wdata_1

    _tmp = (PTN_REG_READ(phys_regs[0x3b]+0x140)&0xffffff80)|0x1a;
    PTN_REG_WRITE(phys_regs[0x3b]+0x140, _tmp); // isp_ltm.reg_resize_coeff_buff_addr

    _tmp = (PTN_REG_READ(phys_regs[0x3b]+0x144)&0xfc000000)|0x3460d0;
    PTN_REG_WRITE(phys_regs[0x3b]+0x144, _tmp); // isp_ltm.reg_resize_coeff_buff_wdata_0

    _tmp = (PTN_REG_READ(phys_regs[0x3b]+0x148)&0xfc000000)|0xaa138;
    PTN_REG_WRITE(phys_regs[0x3b]+0x148, _tmp); // isp_ltm.reg_resize_coeff_buff_wdata_1

    _tmp = (PTN_REG_READ(phys_regs[0x3b]+0x140)&0xffffff80)|0x1b;
    PTN_REG_WRITE(phys_regs[0x3b]+0x140, _tmp); // isp_ltm.reg_resize_coeff_buff_addr

    _tmp = (PTN_REG_READ(phys_regs[0x3b]+0x144)&0xfc000000)|0x3460ce;
    PTN_REG_WRITE(phys_regs[0x3b]+0x144, _tmp); // isp_ltm.reg_resize_coeff_buff_wdata_0

    _tmp = (PTN_REG_READ(phys_regs[0x3b]+0x148)&0xfc000000)|0xac139;
    PTN_REG_WRITE(phys_regs[0x3b]+0x148, _tmp); // isp_ltm.reg_resize_coeff_buff_wdata_1

    _tmp = (PTN_REG_READ(phys_regs[0x3b]+0x140)&0xffffff80)|0x1c;
    PTN_REG_WRITE(phys_regs[0x3b]+0x140, _tmp); // isp_ltm.reg_resize_coeff_buff_addr

    _tmp = (PTN_REG_READ(phys_regs[0x3b]+0x144)&0xfc000000)|0x3440cc;
    PTN_REG_WRITE(phys_regs[0x3b]+0x144, _tmp); // isp_ltm.reg_resize_coeff_buff_wdata_0

    _tmp = (PTN_REG_READ(phys_regs[0x3b]+0x148)&0xfc000000)|0xb013b;
    PTN_REG_WRITE(phys_regs[0x3b]+0x148, _tmp); // isp_ltm.reg_resize_coeff_buff_wdata_1

    _tmp = (PTN_REG_READ(phys_regs[0x3b]+0x140)&0xffffff80)|0x1d;
    PTN_REG_WRITE(phys_regs[0x3b]+0x140, _tmp); // isp_ltm.reg_resize_coeff_buff_addr

    _tmp = (PTN_REG_READ(phys_regs[0x3b]+0x144)&0xfc000000)|0x3420ca;
    PTN_REG_WRITE(phys_regs[0x3b]+0x144, _tmp); // isp_ltm.reg_resize_coeff_buff_wdata_0

    _tmp = (PTN_REG_READ(phys_regs[0x3b]+0x148)&0xfc000000)|0xb213d;
    PTN_REG_WRITE(phys_regs[0x3b]+0x148, _tmp); // isp_ltm.reg_resize_coeff_buff_wdata_1

    _tmp = (PTN_REG_READ(phys_regs[0x3b]+0x140)&0xffffff80)|0x1e;
    PTN_REG_WRITE(phys_regs[0x3b]+0x140, _tmp); // isp_ltm.reg_resize_coeff_buff_addr

    _tmp = (PTN_REG_READ(phys_regs[0x3b]+0x144)&0xfc000000)|0x3400c7;
    PTN_REG_WRITE(phys_regs[0x3b]+0x144, _tmp); // isp_ltm.reg_resize_coeff_buff_wdata_0

    _tmp = (PTN_REG_READ(phys_regs[0x3b]+0x148)&0xfc000000)|0xb413f;
    PTN_REG_WRITE(phys_regs[0x3b]+0x148, _tmp); // isp_ltm.reg_resize_coeff_buff_wdata_1

    _tmp = (PTN_REG_READ(phys_regs[0x3b]+0x140)&0xffffff80)|0x1f;
    PTN_REG_WRITE(phys_regs[0x3b]+0x140, _tmp); // isp_ltm.reg_resize_coeff_buff_addr

    _tmp = (PTN_REG_READ(phys_regs[0x3b]+0x144)&0xfc000000)|0x33e0c5;
    PTN_REG_WRITE(phys_regs[0x3b]+0x144, _tmp); // isp_ltm.reg_resize_coeff_buff_wdata_0

    _tmp = (PTN_REG_READ(phys_regs[0x3b]+0x148)&0xfc000000)|0xb6141;
    PTN_REG_WRITE(phys_regs[0x3b]+0x148, _tmp); // isp_ltm.reg_resize_coeff_buff_wdata_1

    _tmp = (PTN_REG_READ(phys_regs[0x3b]+0x140)&0xffffff80)|0x20;
    PTN_REG_WRITE(phys_regs[0x3b]+0x140, _tmp); // isp_ltm.reg_resize_coeff_buff_addr

    _tmp = (PTN_REG_READ(phys_regs[0x3b]+0x144)&0xfc000000)|0x33c0c3;
    PTN_REG_WRITE(phys_regs[0x3b]+0x144, _tmp); // isp_ltm.reg_resize_coeff_buff_wdata_0

    _tmp = (PTN_REG_READ(phys_regs[0x3b]+0x148)&0xfc000000)|0xb8142;
    PTN_REG_WRITE(phys_regs[0x3b]+0x148, _tmp); // isp_ltm.reg_resize_coeff_buff_wdata_1

    _tmp = (PTN_REG_READ(phys_regs[0x3b]+0x140)&0xffffff80)|0x21;
    PTN_REG_WRITE(phys_regs[0x3b]+0x140, _tmp); // isp_ltm.reg_resize_coeff_buff_addr

    _tmp = (PTN_REG_READ(phys_regs[0x3b]+0x144)&0xfc000000)|0x33a0c2;
    PTN_REG_WRITE(phys_regs[0x3b]+0x144, _tmp); // isp_ltm.reg_resize_coeff_buff_wdata_0

    _tmp = (PTN_REG_READ(phys_regs[0x3b]+0x148)&0xfc000000)|0xbc144;
    PTN_REG_WRITE(phys_regs[0x3b]+0x148, _tmp); // isp_ltm.reg_resize_coeff_buff_wdata_1

    _tmp = (PTN_REG_READ(phys_regs[0x3b]+0x140)&0xffffff80)|0x22;
    PTN_REG_WRITE(phys_regs[0x3b]+0x140, _tmp); // isp_ltm.reg_resize_coeff_buff_addr

    _tmp = (PTN_REG_READ(phys_regs[0x3b]+0x144)&0xfc000000)|0x3380c0;
    PTN_REG_WRITE(phys_regs[0x3b]+0x144, _tmp); // isp_ltm.reg_resize_coeff_buff_wdata_0

    _tmp = (PTN_REG_READ(phys_regs[0x3b]+0x148)&0xfc000000)|0xbe146;
    PTN_REG_WRITE(phys_regs[0x3b]+0x148, _tmp); // isp_ltm.reg_resize_coeff_buff_wdata_1

    _tmp = (PTN_REG_READ(phys_regs[0x3b]+0x140)&0xffffff80)|0x23;
    PTN_REG_WRITE(phys_regs[0x3b]+0x140, _tmp); // isp_ltm.reg_resize_coeff_buff_addr

    _tmp = (PTN_REG_READ(phys_regs[0x3b]+0x144)&0xfc000000)|0x3360be;
    PTN_REG_WRITE(phys_regs[0x3b]+0x144, _tmp); // isp_ltm.reg_resize_coeff_buff_wdata_0

    _tmp = (PTN_REG_READ(phys_regs[0x3b]+0x148)&0xfc000000)|0xc0148;
    PTN_REG_WRITE(phys_regs[0x3b]+0x148, _tmp); // isp_ltm.reg_resize_coeff_buff_wdata_1

    _tmp = (PTN_REG_READ(phys_regs[0x3b]+0x140)&0xffffff80)|0x24;
    PTN_REG_WRITE(phys_regs[0x3b]+0x140, _tmp); // isp_ltm.reg_resize_coeff_buff_addr

    _tmp = (PTN_REG_READ(phys_regs[0x3b]+0x144)&0xfc000000)|0x3340bc;
    PTN_REG_WRITE(phys_regs[0x3b]+0x144, _tmp); // isp_ltm.reg_resize_coeff_buff_wdata_0

    _tmp = (PTN_REG_READ(phys_regs[0x3b]+0x148)&0xfc000000)|0xc2149;
    PTN_REG_WRITE(phys_regs[0x3b]+0x148, _tmp); // isp_ltm.reg_resize_coeff_buff_wdata_1

    _tmp = (PTN_REG_READ(phys_regs[0x3b]+0x140)&0xffffff80)|0x25;
    PTN_REG_WRITE(phys_regs[0x3b]+0x140, _tmp); // isp_ltm.reg_resize_coeff_buff_addr

    _tmp = (PTN_REG_READ(phys_regs[0x3b]+0x144)&0xfc000000)|0x3320ba;
    PTN_REG_WRITE(phys_regs[0x3b]+0x144, _tmp); // isp_ltm.reg_resize_coeff_buff_wdata_0

    _tmp = (PTN_REG_READ(phys_regs[0x3b]+0x148)&0xfc000000)|0xc614b;
    PTN_REG_WRITE(phys_regs[0x3b]+0x148, _tmp); // isp_ltm.reg_resize_coeff_buff_wdata_1

    _tmp = (PTN_REG_READ(phys_regs[0x3b]+0x140)&0xffffff80)|0x26;
    PTN_REG_WRITE(phys_regs[0x3b]+0x140, _tmp); // isp_ltm.reg_resize_coeff_buff_addr

    _tmp = (PTN_REG_READ(phys_regs[0x3b]+0x144)&0xfc000000)|0x3300b8;
    PTN_REG_WRITE(phys_regs[0x3b]+0x144, _tmp); // isp_ltm.reg_resize_coeff_buff_wdata_0

    _tmp = (PTN_REG_READ(phys_regs[0x3b]+0x148)&0xfc000000)|0xc814d;
    PTN_REG_WRITE(phys_regs[0x3b]+0x148, _tmp); // isp_ltm.reg_resize_coeff_buff_wdata_1

    _tmp = (PTN_REG_READ(phys_regs[0x3b]+0x140)&0xffffff80)|0x27;
    PTN_REG_WRITE(phys_regs[0x3b]+0x140, _tmp); // isp_ltm.reg_resize_coeff_buff_addr

    _tmp = (PTN_REG_READ(phys_regs[0x3b]+0x144)&0xfc000000)|0x32e0b6;
    PTN_REG_WRITE(phys_regs[0x3b]+0x144, _tmp); // isp_ltm.reg_resize_coeff_buff_wdata_0

    _tmp = (PTN_REG_READ(phys_regs[0x3b]+0x148)&0xfc000000)|0xca14e;
    PTN_REG_WRITE(phys_regs[0x3b]+0x148, _tmp); // isp_ltm.reg_resize_coeff_buff_wdata_1

    _tmp = (PTN_REG_READ(phys_regs[0x3b]+0x140)&0xffffff80)|0x28;
    PTN_REG_WRITE(phys_regs[0x3b]+0x140, _tmp); // isp_ltm.reg_resize_coeff_buff_addr

    _tmp = (PTN_REG_READ(phys_regs[0x3b]+0x144)&0xfc000000)|0x32a0b4;
    PTN_REG_WRITE(phys_regs[0x3b]+0x144, _tmp); // isp_ltm.reg_resize_coeff_buff_wdata_0

    _tmp = (PTN_REG_READ(phys_regs[0x3b]+0x148)&0xfc000000)|0xce150;
    PTN_REG_WRITE(phys_regs[0x3b]+0x148, _tmp); // isp_ltm.reg_resize_coeff_buff_wdata_1

    _tmp = (PTN_REG_READ(phys_regs[0x3b]+0x140)&0xffffff80)|0x29;
    PTN_REG_WRITE(phys_regs[0x3b]+0x140, _tmp); // isp_ltm.reg_resize_coeff_buff_addr

    _tmp = (PTN_REG_READ(phys_regs[0x3b]+0x144)&0xfc000000)|0x3280b2;
    PTN_REG_WRITE(phys_regs[0x3b]+0x144, _tmp); // isp_ltm.reg_resize_coeff_buff_wdata_0

    _tmp = (PTN_REG_READ(phys_regs[0x3b]+0x148)&0xfc000000)|0xd0152;
    PTN_REG_WRITE(phys_regs[0x3b]+0x148, _tmp); // isp_ltm.reg_resize_coeff_buff_wdata_1

    _tmp = (PTN_REG_READ(phys_regs[0x3b]+0x140)&0xffffff80)|0x2a;
    PTN_REG_WRITE(phys_regs[0x3b]+0x140, _tmp); // isp_ltm.reg_resize_coeff_buff_addr

    _tmp = (PTN_REG_READ(phys_regs[0x3b]+0x144)&0xfc000000)|0x3260b0;
    PTN_REG_WRITE(phys_regs[0x3b]+0x144, _tmp); // isp_ltm.reg_resize_coeff_buff_wdata_0

    _tmp = (PTN_REG_READ(phys_regs[0x3b]+0x148)&0xfc000000)|0xd2154;
    PTN_REG_WRITE(phys_regs[0x3b]+0x148, _tmp); // isp_ltm.reg_resize_coeff_buff_wdata_1

    _tmp = (PTN_REG_READ(phys_regs[0x3b]+0x140)&0xffffff80)|0x2b;
    PTN_REG_WRITE(phys_regs[0x3b]+0x140, _tmp); // isp_ltm.reg_resize_coeff_buff_addr

    _tmp = (PTN_REG_READ(phys_regs[0x3b]+0x144)&0xfc000000)|0x3240ae;
    PTN_REG_WRITE(phys_regs[0x3b]+0x144, _tmp); // isp_ltm.reg_resize_coeff_buff_wdata_0

    _tmp = (PTN_REG_READ(phys_regs[0x3b]+0x148)&0xfc000000)|0xd6155;
    PTN_REG_WRITE(phys_regs[0x3b]+0x148, _tmp); // isp_ltm.reg_resize_coeff_buff_wdata_1

    _tmp = (PTN_REG_READ(phys_regs[0x3b]+0x140)&0xffffff80)|0x2c;
    PTN_REG_WRITE(phys_regs[0x3b]+0x140, _tmp); // isp_ltm.reg_resize_coeff_buff_addr

    _tmp = (PTN_REG_READ(phys_regs[0x3b]+0x144)&0xfc000000)|0x3220ac;
    PTN_REG_WRITE(phys_regs[0x3b]+0x144, _tmp); // isp_ltm.reg_resize_coeff_buff_wdata_0

    _tmp = (PTN_REG_READ(phys_regs[0x3b]+0x148)&0xfc000000)|0xd8157;
    PTN_REG_WRITE(phys_regs[0x3b]+0x148, _tmp); // isp_ltm.reg_resize_coeff_buff_wdata_1

    _tmp = (PTN_REG_READ(phys_regs[0x3b]+0x140)&0xffffff80)|0x2d;
    PTN_REG_WRITE(phys_regs[0x3b]+0x140, _tmp); // isp_ltm.reg_resize_coeff_buff_addr

    _tmp = (PTN_REG_READ(phys_regs[0x3b]+0x144)&0xfc000000)|0x3200ab;
    PTN_REG_WRITE(phys_regs[0x3b]+0x144, _tmp); // isp_ltm.reg_resize_coeff_buff_wdata_0

    _tmp = (PTN_REG_READ(phys_regs[0x3b]+0x148)&0xfc000000)|0xda159;
    PTN_REG_WRITE(phys_regs[0x3b]+0x148, _tmp); // isp_ltm.reg_resize_coeff_buff_wdata_1

    _tmp = (PTN_REG_READ(phys_regs[0x3b]+0x140)&0xffffff80)|0x2e;
    PTN_REG_WRITE(phys_regs[0x3b]+0x140, _tmp); // isp_ltm.reg_resize_coeff_buff_addr

    _tmp = (PTN_REG_READ(phys_regs[0x3b]+0x144)&0xfc000000)|0x31c0a9;
    PTN_REG_WRITE(phys_regs[0x3b]+0x144, _tmp); // isp_ltm.reg_resize_coeff_buff_wdata_0

    _tmp = (PTN_REG_READ(phys_regs[0x3b]+0x148)&0xfc000000)|0xde15a;
    PTN_REG_WRITE(phys_regs[0x3b]+0x148, _tmp); // isp_ltm.reg_resize_coeff_buff_wdata_1

    _tmp = (PTN_REG_READ(phys_regs[0x3b]+0x140)&0xffffff80)|0x2f;
    PTN_REG_WRITE(phys_regs[0x3b]+0x140, _tmp); // isp_ltm.reg_resize_coeff_buff_addr

    _tmp = (PTN_REG_READ(phys_regs[0x3b]+0x144)&0xfc000000)|0x31a0a7;
    PTN_REG_WRITE(phys_regs[0x3b]+0x144, _tmp); // isp_ltm.reg_resize_coeff_buff_wdata_0

    _tmp = (PTN_REG_READ(phys_regs[0x3b]+0x148)&0xfc000000)|0xe015c;
    PTN_REG_WRITE(phys_regs[0x3b]+0x148, _tmp); // isp_ltm.reg_resize_coeff_buff_wdata_1

    _tmp = (PTN_REG_READ(phys_regs[0x3b]+0x140)&0xffffff80)|0x30;
    PTN_REG_WRITE(phys_regs[0x3b]+0x140, _tmp); // isp_ltm.reg_resize_coeff_buff_addr

    _tmp = (PTN_REG_READ(phys_regs[0x3b]+0x144)&0xfc000000)|0x3180a5;
    PTN_REG_WRITE(phys_regs[0x3b]+0x144, _tmp); // isp_ltm.reg_resize_coeff_buff_wdata_0

    _tmp = (PTN_REG_READ(phys_regs[0x3b]+0x148)&0xfc000000)|0xe215d;
    PTN_REG_WRITE(phys_regs[0x3b]+0x148, _tmp); // isp_ltm.reg_resize_coeff_buff_wdata_1

    _tmp = (PTN_REG_READ(phys_regs[0x3b]+0x140)&0xffffff80)|0x31;
    PTN_REG_WRITE(phys_regs[0x3b]+0x140, _tmp); // isp_ltm.reg_resize_coeff_buff_addr

    _tmp = (PTN_REG_READ(phys_regs[0x3b]+0x144)&0xfc000000)|0x3160a3;
    PTN_REG_WRITE(phys_regs[0x3b]+0x144, _tmp); // isp_ltm.reg_resize_coeff_buff_wdata_0

    _tmp = (PTN_REG_READ(phys_regs[0x3b]+0x148)&0xfc000000)|0xe615f;
    PTN_REG_WRITE(phys_regs[0x3b]+0x148, _tmp); // isp_ltm.reg_resize_coeff_buff_wdata_1

    _tmp = (PTN_REG_READ(phys_regs[0x3b]+0x140)&0xffffff80)|0x32;
    PTN_REG_WRITE(phys_regs[0x3b]+0x140, _tmp); // isp_ltm.reg_resize_coeff_buff_addr

    _tmp = (PTN_REG_READ(phys_regs[0x3b]+0x144)&0xfc000000)|0x3120a1;
    PTN_REG_WRITE(phys_regs[0x3b]+0x144, _tmp); // isp_ltm.reg_resize_coeff_buff_wdata_0

    _tmp = (PTN_REG_READ(phys_regs[0x3b]+0x148)&0xfc000000)|0xe8161;
    PTN_REG_WRITE(phys_regs[0x3b]+0x148, _tmp); // isp_ltm.reg_resize_coeff_buff_wdata_1

    _tmp = (PTN_REG_READ(phys_regs[0x3b]+0x140)&0xffffff80)|0x33;
    PTN_REG_WRITE(phys_regs[0x3b]+0x140, _tmp); // isp_ltm.reg_resize_coeff_buff_addr

    _tmp = (PTN_REG_READ(phys_regs[0x3b]+0x144)&0xfc000000)|0x3100a0;
    PTN_REG_WRITE(phys_regs[0x3b]+0x144, _tmp); // isp_ltm.reg_resize_coeff_buff_wdata_0

    _tmp = (PTN_REG_READ(phys_regs[0x3b]+0x148)&0xfc000000)|0xec162;
    PTN_REG_WRITE(phys_regs[0x3b]+0x148, _tmp); // isp_ltm.reg_resize_coeff_buff_wdata_1

    _tmp = (PTN_REG_READ(phys_regs[0x3b]+0x140)&0xffffff80)|0x34;
    PTN_REG_WRITE(phys_regs[0x3b]+0x140, _tmp); // isp_ltm.reg_resize_coeff_buff_addr

    _tmp = (PTN_REG_READ(phys_regs[0x3b]+0x144)&0xfc000000)|0x30e09e;
    PTN_REG_WRITE(phys_regs[0x3b]+0x144, _tmp); // isp_ltm.reg_resize_coeff_buff_wdata_0

    _tmp = (PTN_REG_READ(phys_regs[0x3b]+0x148)&0xfc000000)|0xee164;
    PTN_REG_WRITE(phys_regs[0x3b]+0x148, _tmp); // isp_ltm.reg_resize_coeff_buff_wdata_1

    _tmp = (PTN_REG_READ(phys_regs[0x3b]+0x140)&0xffffff80)|0x35;
    PTN_REG_WRITE(phys_regs[0x3b]+0x140, _tmp); // isp_ltm.reg_resize_coeff_buff_addr

    _tmp = (PTN_REG_READ(phys_regs[0x3b]+0x144)&0xfc000000)|0x30c09c;
    PTN_REG_WRITE(phys_regs[0x3b]+0x144, _tmp); // isp_ltm.reg_resize_coeff_buff_wdata_0

    _tmp = (PTN_REG_READ(phys_regs[0x3b]+0x148)&0xfc000000)|0xf2166;
    PTN_REG_WRITE(phys_regs[0x3b]+0x148, _tmp); // isp_ltm.reg_resize_coeff_buff_wdata_1

    _tmp = (PTN_REG_READ(phys_regs[0x3b]+0x140)&0xffffff80)|0x36;
    PTN_REG_WRITE(phys_regs[0x3b]+0x140, _tmp); // isp_ltm.reg_resize_coeff_buff_addr

    _tmp = (PTN_REG_READ(phys_regs[0x3b]+0x144)&0xfc000000)|0x30809a;
    PTN_REG_WRITE(phys_regs[0x3b]+0x144, _tmp); // isp_ltm.reg_resize_coeff_buff_wdata_0

    _tmp = (PTN_REG_READ(phys_regs[0x3b]+0x148)&0xfc000000)|0xf4167;
    PTN_REG_WRITE(phys_regs[0x3b]+0x148, _tmp); // isp_ltm.reg_resize_coeff_buff_wdata_1

    _tmp = (PTN_REG_READ(phys_regs[0x3b]+0x140)&0xffffff80)|0x37;
    PTN_REG_WRITE(phys_regs[0x3b]+0x140, _tmp); // isp_ltm.reg_resize_coeff_buff_addr

    _tmp = (PTN_REG_READ(phys_regs[0x3b]+0x144)&0xfc000000)|0x306099;
    PTN_REG_WRITE(phys_regs[0x3b]+0x144, _tmp); // isp_ltm.reg_resize_coeff_buff_wdata_0

    _tmp = (PTN_REG_READ(phys_regs[0x3b]+0x148)&0xfc000000)|0xf8169;
    PTN_REG_WRITE(phys_regs[0x3b]+0x148, _tmp); // isp_ltm.reg_resize_coeff_buff_wdata_1

    _tmp = (PTN_REG_READ(phys_regs[0x3b]+0x140)&0xffffff80)|0x38;
    PTN_REG_WRITE(phys_regs[0x3b]+0x140, _tmp); // isp_ltm.reg_resize_coeff_buff_addr

    _tmp = (PTN_REG_READ(phys_regs[0x3b]+0x144)&0xfc000000)|0x304097;
    PTN_REG_WRITE(phys_regs[0x3b]+0x144, _tmp); // isp_ltm.reg_resize_coeff_buff_wdata_0

    _tmp = (PTN_REG_READ(phys_regs[0x3b]+0x148)&0xfc000000)|0xfa16a;
    PTN_REG_WRITE(phys_regs[0x3b]+0x148, _tmp); // isp_ltm.reg_resize_coeff_buff_wdata_1

    _tmp = (PTN_REG_READ(phys_regs[0x3b]+0x140)&0xffffff80)|0x39;
    PTN_REG_WRITE(phys_regs[0x3b]+0x140, _tmp); // isp_ltm.reg_resize_coeff_buff_addr

    _tmp = (PTN_REG_READ(phys_regs[0x3b]+0x144)&0xfc000000)|0x300095;
    PTN_REG_WRITE(phys_regs[0x3b]+0x144, _tmp); // isp_ltm.reg_resize_coeff_buff_wdata_0

    _tmp = (PTN_REG_READ(phys_regs[0x3b]+0x148)&0xfc000000)|0xfe16c;
    PTN_REG_WRITE(phys_regs[0x3b]+0x148, _tmp); // isp_ltm.reg_resize_coeff_buff_wdata_1

    _tmp = (PTN_REG_READ(phys_regs[0x3b]+0x140)&0xffffff80)|0x3a;
    PTN_REG_WRITE(phys_regs[0x3b]+0x140, _tmp); // isp_ltm.reg_resize_coeff_buff_addr

    _tmp = (PTN_REG_READ(phys_regs[0x3b]+0x144)&0xfc000000)|0x2fe094;
    PTN_REG_WRITE(phys_regs[0x3b]+0x144, _tmp); // isp_ltm.reg_resize_coeff_buff_wdata_0

    _tmp = (PTN_REG_READ(phys_regs[0x3b]+0x148)&0xfc000000)|0x10016d;
    PTN_REG_WRITE(phys_regs[0x3b]+0x148, _tmp); // isp_ltm.reg_resize_coeff_buff_wdata_1

    _tmp = (PTN_REG_READ(phys_regs[0x3b]+0x140)&0xffffff80)|0x3b;
    PTN_REG_WRITE(phys_regs[0x3b]+0x140, _tmp); // isp_ltm.reg_resize_coeff_buff_addr

    _tmp = (PTN_REG_READ(phys_regs[0x3b]+0x144)&0xfc000000)|0x2fa092;
    PTN_REG_WRITE(phys_regs[0x3b]+0x144, _tmp); // isp_ltm.reg_resize_coeff_buff_wdata_0

    _tmp = (PTN_REG_READ(phys_regs[0x3b]+0x148)&0xfc000000)|0x10416f;
    PTN_REG_WRITE(phys_regs[0x3b]+0x148, _tmp); // isp_ltm.reg_resize_coeff_buff_wdata_1

    _tmp = (PTN_REG_READ(phys_regs[0x3b]+0x140)&0xffffff80)|0x3c;
    PTN_REG_WRITE(phys_regs[0x3b]+0x140, _tmp); // isp_ltm.reg_resize_coeff_buff_addr

    _tmp = (PTN_REG_READ(phys_regs[0x3b]+0x144)&0xfc000000)|0x2f8090;
    PTN_REG_WRITE(phys_regs[0x3b]+0x144, _tmp); // isp_ltm.reg_resize_coeff_buff_wdata_0

    _tmp = (PTN_REG_READ(phys_regs[0x3b]+0x148)&0xfc000000)|0x106170;
    PTN_REG_WRITE(phys_regs[0x3b]+0x148, _tmp); // isp_ltm.reg_resize_coeff_buff_wdata_1

    _tmp = (PTN_REG_READ(phys_regs[0x3b]+0x140)&0xffffff80)|0x3d;
    PTN_REG_WRITE(phys_regs[0x3b]+0x140, _tmp); // isp_ltm.reg_resize_coeff_buff_addr

    _tmp = (PTN_REG_READ(phys_regs[0x3b]+0x144)&0xfc000000)|0x2f608f;
    PTN_REG_WRITE(phys_regs[0x3b]+0x144, _tmp); // isp_ltm.reg_resize_coeff_buff_wdata_0

    _tmp = (PTN_REG_READ(phys_regs[0x3b]+0x148)&0xfc000000)|0x10a172;
    PTN_REG_WRITE(phys_regs[0x3b]+0x148, _tmp); // isp_ltm.reg_resize_coeff_buff_wdata_1

    _tmp = (PTN_REG_READ(phys_regs[0x3b]+0x140)&0xffffff80)|0x3e;
    PTN_REG_WRITE(phys_regs[0x3b]+0x140, _tmp); // isp_ltm.reg_resize_coeff_buff_addr

    _tmp = (PTN_REG_READ(phys_regs[0x3b]+0x144)&0xfc000000)|0x2f208d;
    PTN_REG_WRITE(phys_regs[0x3b]+0x144, _tmp); // isp_ltm.reg_resize_coeff_buff_wdata_0

    _tmp = (PTN_REG_READ(phys_regs[0x3b]+0x148)&0xfc000000)|0x10c173;
    PTN_REG_WRITE(phys_regs[0x3b]+0x148, _tmp); // isp_ltm.reg_resize_coeff_buff_wdata_1

    _tmp = (PTN_REG_READ(phys_regs[0x3b]+0x140)&0xffffff80)|0x3f;
    PTN_REG_WRITE(phys_regs[0x3b]+0x140, _tmp); // isp_ltm.reg_resize_coeff_buff_addr

    _tmp = (PTN_REG_READ(phys_regs[0x3b]+0x144)&0xfc000000)|0x2f008b;
    PTN_REG_WRITE(phys_regs[0x3b]+0x144, _tmp); // isp_ltm.reg_resize_coeff_buff_wdata_0

    _tmp = (PTN_REG_READ(phys_regs[0x3b]+0x148)&0xfc000000)|0x110175;
    PTN_REG_WRITE(phys_regs[0x3b]+0x148, _tmp); // isp_ltm.reg_resize_coeff_buff_wdata_1

    _tmp = (PTN_REG_READ(phys_regs[0x3b]+0x140)&0xffffff80)|0x40;
    PTN_REG_WRITE(phys_regs[0x3b]+0x140, _tmp); // isp_ltm.reg_resize_coeff_buff_addr

    _tmp = (PTN_REG_READ(phys_regs[0x3b]+0x144)&0xfc000000)|0x2ec08a;
    PTN_REG_WRITE(phys_regs[0x3b]+0x144, _tmp); // isp_ltm.reg_resize_coeff_buff_wdata_0

    _tmp = (PTN_REG_READ(phys_regs[0x3b]+0x148)&0xfc000000)|0x114176;
    PTN_REG_WRITE(phys_regs[0x3b]+0x148, _tmp); // isp_ltm.reg_resize_coeff_buff_wdata_1

    _tmp = (PTN_REG_READ(phys_regs[0x3b]+0x140)&0x7fffffff)|0x0;
    PTN_REG_WRITE(phys_regs[0x3b]+0x140, _tmp); // isp_ltm.reg_resize_coeff_buff_wen

} while (0);
