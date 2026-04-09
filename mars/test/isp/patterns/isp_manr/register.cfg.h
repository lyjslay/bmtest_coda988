// arg list below is needed ========================
// @PTN_REG_READ, uint32_t (*)(uint64_t), for register read
// @PTN_REG_WRITE, void (*)(uint64_t, uint32_t data), for register write
// @PTN_MALLOC, uint64_t (*)(size_t, uint64_t), for memory allocation
// @phys_regs, uint64_t , for register base
// @priv, void * , for private info, not touched
// =================================================
do {
    uint32_t _tmp;
    //uint64_t _mem_addr;

    // dma25: 0x141000000
    // dma26: 0x142000000
    // dma27: 0x141000000
    // dma28: 0x142000000
    // dma19: 0x14b000000
    // dma20: 0x143000000
    // dma21: 0x144000000
    // dma22: 0x145000000
    // dma23: 0x147000000
    // dma24: 0x147000000
    _tmp = (PTN_REG_READ(phys_regs[0x3f]+0x6c)&0xfffeffff)|0x0;
    PTN_REG_WRITE(phys_regs[0x3f]+0x6c, _tmp); // manr.motion debug

    _tmp = (PTN_REG_READ(phys_regs[0x3f]+0x0)&0xfeffffff)|0x10;
    PTN_REG_WRITE(phys_regs[0x3f]+0x0, _tmp); // manr.reg_bypass

    _tmp = (PTN_REG_READ(phys_regs[0x3f]+0x6c)&0xffff00ff)|0x8000ff00;
    PTN_REG_WRITE(phys_regs[0x3f]+0x6c, _tmp); // manr.reg_dma_enable

    _tmp = (PTN_REG_READ(phys_regs[0x3f]+0x30)&0xc000c000)|0x2df051b;
    PTN_REG_WRITE(phys_regs[0x3f]+0x30, _tmp); // .

    _tmp = (PTN_REG_READ(phys_regs[0x3f]+0x60)&0xffffff00)|0x33;
    PTN_REG_WRITE(phys_regs[0x3f]+0x60, _tmp); // .

    _tmp = (PTN_REG_READ(phys_regs[0x3f]+0x0)&0xffffffef)|0x10;
    PTN_REG_WRITE(phys_regs[0x3f]+0x0, _tmp); // isp_mmap.reg_on_the_fly

    _tmp = (PTN_REG_READ(phys_regs[0x3f]+0xc4)&0x7fffffff)|0x80000000;
    PTN_REG_WRITE(phys_regs[0x3f]+0xc4, _tmp); // isp_mmap.reg_crop_enable

    _tmp = (PTN_REG_READ(phys_regs[0x3f]+0xc4)&0xffffc000)|0xa7;
    PTN_REG_WRITE(phys_regs[0x3f]+0xc4, _tmp); // isp_mmap.reg_img_width_crop

    _tmp = (PTN_REG_READ(phys_regs[0x3f]+0xc4)&0xc000ffff)|0x5b0000;
    PTN_REG_WRITE(phys_regs[0x3f]+0xc4, _tmp); // isp_mmap.reg_img_height_crop

    _tmp = (PTN_REG_READ(phys_regs[0x3f]+0xc8)&0xffffc000)|0x0;
    PTN_REG_WRITE(phys_regs[0x3f]+0xc8, _tmp); // isp_mmap.reg_crop_w_str

    _tmp = (PTN_REG_READ(phys_regs[0x3f]+0xc8)&0xc000ffff)|0xa30000;
    PTN_REG_WRITE(phys_regs[0x3f]+0xc8, _tmp); // isp_mmap.reg_crop_w_end

    _tmp = (PTN_REG_READ(phys_regs[0x3f]+0xcc)&0xffffc000)|0x0;
    PTN_REG_WRITE(phys_regs[0x3f]+0xcc, _tmp); // isp_mmap.reg_crop_h_str

    _tmp = (PTN_REG_READ(phys_regs[0x3f]+0xcc)&0xc000ffff)|0x5b0000;
    PTN_REG_WRITE(phys_regs[0x3f]+0xcc, _tmp); // isp_mmap.reg_crop_h_end

    _tmp = (PTN_REG_READ(phys_regs[0x3f]+0x6c)&0xffffff7f)|0x80;
    PTN_REG_WRITE(phys_regs[0x3f]+0x6c, _tmp); // .

    _tmp = (PTN_REG_READ(phys_regs[0x3f]+0x6c)&0xffffff80)|0x0;
    PTN_REG_WRITE(phys_regs[0x3f]+0x6c, _tmp); // .

    _tmp = (PTN_REG_READ(phys_regs[0x3f]+0x64)&0x0)|0x20368108;
    PTN_REG_WRITE(phys_regs[0x3f]+0x64, _tmp); // .

    _tmp = (PTN_REG_READ(phys_regs[0x3f]+0x68)&0x0)|0x1d84;
    PTN_REG_WRITE(phys_regs[0x3f]+0x68, _tmp); // .

    _tmp = (PTN_REG_READ(phys_regs[0x3f]+0x6c)&0xffffff80)|0x1;
    PTN_REG_WRITE(phys_regs[0x3f]+0x6c, _tmp); // .

    _tmp = (PTN_REG_READ(phys_regs[0x3f]+0x64)&0x0)|0x28368106;
    PTN_REG_WRITE(phys_regs[0x3f]+0x64, _tmp); // .

    _tmp = (PTN_REG_READ(phys_regs[0x3f]+0x68)&0x0)|0x1e04;
    PTN_REG_WRITE(phys_regs[0x3f]+0x68, _tmp); // .

    _tmp = (PTN_REG_READ(phys_regs[0x3f]+0x6c)&0xffffff80)|0x2;
    PTN_REG_WRITE(phys_regs[0x3f]+0x6c, _tmp); // .

    _tmp = (PTN_REG_READ(phys_regs[0x3f]+0x64)&0x0)|0x30366104;
    PTN_REG_WRITE(phys_regs[0x3f]+0x64, _tmp); // .

    _tmp = (PTN_REG_READ(phys_regs[0x3f]+0x68)&0x0)|0x1e84;
    PTN_REG_WRITE(phys_regs[0x3f]+0x68, _tmp); // .

    _tmp = (PTN_REG_READ(phys_regs[0x3f]+0x6c)&0xffffff80)|0x3;
    PTN_REG_WRITE(phys_regs[0x3f]+0x6c, _tmp); // .

    _tmp = (PTN_REG_READ(phys_regs[0x3f]+0x64)&0x0)|0x38366102;
    PTN_REG_WRITE(phys_regs[0x3f]+0x64, _tmp); // .

    _tmp = (PTN_REG_READ(phys_regs[0x3f]+0x68)&0x0)|0x1f04;
    PTN_REG_WRITE(phys_regs[0x3f]+0x68, _tmp); // .

    _tmp = (PTN_REG_READ(phys_regs[0x3f]+0x6c)&0xffffff80)|0x4;
    PTN_REG_WRITE(phys_regs[0x3f]+0x6c, _tmp); // .

    _tmp = (PTN_REG_READ(phys_regs[0x3f]+0x64)&0x0)|0x403640ff;
    PTN_REG_WRITE(phys_regs[0x3f]+0x64, _tmp); // .

    _tmp = (PTN_REG_READ(phys_regs[0x3f]+0x68)&0x0)|0x1f84;
    PTN_REG_WRITE(phys_regs[0x3f]+0x68, _tmp); // .

    _tmp = (PTN_REG_READ(phys_regs[0x3f]+0x6c)&0xffffff80)|0x5;
    PTN_REG_WRITE(phys_regs[0x3f]+0x6c, _tmp); // .

    _tmp = (PTN_REG_READ(phys_regs[0x3f]+0x64)&0x0)|0x483640fd;
    PTN_REG_WRITE(phys_regs[0x3f]+0x64, _tmp); // .

    _tmp = (PTN_REG_READ(phys_regs[0x3f]+0x68)&0x0)|0x1f84;
    PTN_REG_WRITE(phys_regs[0x3f]+0x68, _tmp); // .

    _tmp = (PTN_REG_READ(phys_regs[0x3f]+0x6c)&0xffffff80)|0x6;
    PTN_REG_WRITE(phys_regs[0x3f]+0x6c, _tmp); // .

    _tmp = (PTN_REG_READ(phys_regs[0x3f]+0x64)&0x0)|0x4c3620fb;
    PTN_REG_WRITE(phys_regs[0x3f]+0x64, _tmp); // .

    _tmp = (PTN_REG_READ(phys_regs[0x3f]+0x68)&0x0)|0x2004;
    PTN_REG_WRITE(phys_regs[0x3f]+0x68, _tmp); // .

    _tmp = (PTN_REG_READ(phys_regs[0x3f]+0x6c)&0xffffff80)|0x7;
    PTN_REG_WRITE(phys_regs[0x3f]+0x6c, _tmp); // .

    _tmp = (PTN_REG_READ(phys_regs[0x3f]+0x64)&0x0)|0x543620f9;
    PTN_REG_WRITE(phys_regs[0x3f]+0x64, _tmp); // .

    _tmp = (PTN_REG_READ(phys_regs[0x3f]+0x68)&0x0)|0x2084;
    PTN_REG_WRITE(phys_regs[0x3f]+0x68, _tmp); // .

    _tmp = (PTN_REG_READ(phys_regs[0x3f]+0x6c)&0xffffff80)|0x8;
    PTN_REG_WRITE(phys_regs[0x3f]+0x6c, _tmp); // .

    _tmp = (PTN_REG_READ(phys_regs[0x3f]+0x64)&0x0)|0x5c3600f6;
    PTN_REG_WRITE(phys_regs[0x3f]+0x64, _tmp); // .

    _tmp = (PTN_REG_READ(phys_regs[0x3f]+0x68)&0x0)|0x2104;
    PTN_REG_WRITE(phys_regs[0x3f]+0x68, _tmp); // .

    _tmp = (PTN_REG_READ(phys_regs[0x3f]+0x6c)&0xffffff80)|0x9;
    PTN_REG_WRITE(phys_regs[0x3f]+0x6c, _tmp); // .

    _tmp = (PTN_REG_READ(phys_regs[0x3f]+0x64)&0x0)|0x643600f4;
    PTN_REG_WRITE(phys_regs[0x3f]+0x64, _tmp); // .

    _tmp = (PTN_REG_READ(phys_regs[0x3f]+0x68)&0x0)|0x2184;
    PTN_REG_WRITE(phys_regs[0x3f]+0x68, _tmp); // .

    _tmp = (PTN_REG_READ(phys_regs[0x3f]+0x6c)&0xffffff80)|0xa;
    PTN_REG_WRITE(phys_regs[0x3f]+0x6c, _tmp); // .

    _tmp = (PTN_REG_READ(phys_regs[0x3f]+0x64)&0x0)|0x6c35e0f2;
    PTN_REG_WRITE(phys_regs[0x3f]+0x64, _tmp); // .

    _tmp = (PTN_REG_READ(phys_regs[0x3f]+0x68)&0x0)|0x2204;
    PTN_REG_WRITE(phys_regs[0x3f]+0x68, _tmp); // .

    _tmp = (PTN_REG_READ(phys_regs[0x3f]+0x6c)&0xffffff80)|0xb;
    PTN_REG_WRITE(phys_regs[0x3f]+0x6c, _tmp); // .

    _tmp = (PTN_REG_READ(phys_regs[0x3f]+0x64)&0x0)|0x7435e0f0;
    PTN_REG_WRITE(phys_regs[0x3f]+0x64, _tmp); // .

    _tmp = (PTN_REG_READ(phys_regs[0x3f]+0x68)&0x0)|0x2284;
    PTN_REG_WRITE(phys_regs[0x3f]+0x68, _tmp); // .

    _tmp = (PTN_REG_READ(phys_regs[0x3f]+0x6c)&0xffffff80)|0xc;
    PTN_REG_WRITE(phys_regs[0x3f]+0x6c, _tmp); // .

    _tmp = (PTN_REG_READ(phys_regs[0x3f]+0x64)&0x0)|0x7835c0ed;
    PTN_REG_WRITE(phys_regs[0x3f]+0x64, _tmp); // .

    _tmp = (PTN_REG_READ(phys_regs[0x3f]+0x68)&0x0)|0x2304;
    PTN_REG_WRITE(phys_regs[0x3f]+0x68, _tmp); // .

    _tmp = (PTN_REG_READ(phys_regs[0x3f]+0x6c)&0xffffff80)|0xd;
    PTN_REG_WRITE(phys_regs[0x3f]+0x6c, _tmp); // .

    _tmp = (PTN_REG_READ(phys_regs[0x3f]+0x64)&0x0)|0x8035a0eb;
    PTN_REG_WRITE(phys_regs[0x3f]+0x64, _tmp); // .

    _tmp = (PTN_REG_READ(phys_regs[0x3f]+0x68)&0x0)|0x2384;
    PTN_REG_WRITE(phys_regs[0x3f]+0x68, _tmp); // .

    _tmp = (PTN_REG_READ(phys_regs[0x3f]+0x6c)&0xffffff80)|0xe;
    PTN_REG_WRITE(phys_regs[0x3f]+0x6c, _tmp); // .

    _tmp = (PTN_REG_READ(phys_regs[0x3f]+0x64)&0x0)|0x8835a0e9;
    PTN_REG_WRITE(phys_regs[0x3f]+0x64, _tmp); // .

    _tmp = (PTN_REG_READ(phys_regs[0x3f]+0x68)&0x0)|0x2404;
    PTN_REG_WRITE(phys_regs[0x3f]+0x68, _tmp); // .

    _tmp = (PTN_REG_READ(phys_regs[0x3f]+0x6c)&0xffffff80)|0xf;
    PTN_REG_WRITE(phys_regs[0x3f]+0x6c, _tmp); // .

    _tmp = (PTN_REG_READ(phys_regs[0x3f]+0x64)&0x0)|0x903580e7;
    PTN_REG_WRITE(phys_regs[0x3f]+0x64, _tmp); // .

    _tmp = (PTN_REG_READ(phys_regs[0x3f]+0x68)&0x0)|0x2484;
    PTN_REG_WRITE(phys_regs[0x3f]+0x68, _tmp); // .

    _tmp = (PTN_REG_READ(phys_regs[0x3f]+0x6c)&0xffffff80)|0x10;
    PTN_REG_WRITE(phys_regs[0x3f]+0x6c, _tmp); // .

    _tmp = (PTN_REG_READ(phys_regs[0x3f]+0x64)&0x0)|0x983560e5;
    PTN_REG_WRITE(phys_regs[0x3f]+0x64, _tmp); // .

    _tmp = (PTN_REG_READ(phys_regs[0x3f]+0x68)&0x0)|0x2504;
    PTN_REG_WRITE(phys_regs[0x3f]+0x68, _tmp); // .

    _tmp = (PTN_REG_READ(phys_regs[0x3f]+0x6c)&0xffffff80)|0x11;
    PTN_REG_WRITE(phys_regs[0x3f]+0x6c, _tmp); // .

    _tmp = (PTN_REG_READ(phys_regs[0x3f]+0x64)&0x0)|0xa03560e3;
    PTN_REG_WRITE(phys_regs[0x3f]+0x64, _tmp); // .

    _tmp = (PTN_REG_READ(phys_regs[0x3f]+0x68)&0x0)|0x2584;
    PTN_REG_WRITE(phys_regs[0x3f]+0x68, _tmp); // .

    _tmp = (PTN_REG_READ(phys_regs[0x3f]+0x6c)&0xffffff80)|0x12;
    PTN_REG_WRITE(phys_regs[0x3f]+0x6c, _tmp); // .

    _tmp = (PTN_REG_READ(phys_regs[0x3f]+0x64)&0x0)|0xa43540e0;
    PTN_REG_WRITE(phys_regs[0x3f]+0x64, _tmp); // .

    _tmp = (PTN_REG_READ(phys_regs[0x3f]+0x68)&0x0)|0x2604;
    PTN_REG_WRITE(phys_regs[0x3f]+0x68, _tmp); // .

    _tmp = (PTN_REG_READ(phys_regs[0x3f]+0x6c)&0xffffff80)|0x13;
    PTN_REG_WRITE(phys_regs[0x3f]+0x6c, _tmp); // .

    _tmp = (PTN_REG_READ(phys_regs[0x3f]+0x64)&0x0)|0xac3520de;
    PTN_REG_WRITE(phys_regs[0x3f]+0x64, _tmp); // .

    _tmp = (PTN_REG_READ(phys_regs[0x3f]+0x68)&0x0)|0x2684;
    PTN_REG_WRITE(phys_regs[0x3f]+0x68, _tmp); // .

    _tmp = (PTN_REG_READ(phys_regs[0x3f]+0x6c)&0xffffff80)|0x14;
    PTN_REG_WRITE(phys_regs[0x3f]+0x6c, _tmp); // .

    _tmp = (PTN_REG_READ(phys_regs[0x3f]+0x64)&0x0)|0xb43500dc;
    PTN_REG_WRITE(phys_regs[0x3f]+0x64, _tmp); // .

    _tmp = (PTN_REG_READ(phys_regs[0x3f]+0x68)&0x0)|0x2784;
    PTN_REG_WRITE(phys_regs[0x3f]+0x68, _tmp); // .

    _tmp = (PTN_REG_READ(phys_regs[0x3f]+0x6c)&0xffffff80)|0x15;
    PTN_REG_WRITE(phys_regs[0x3f]+0x6c, _tmp); // .

    _tmp = (PTN_REG_READ(phys_regs[0x3f]+0x64)&0x0)|0xbc3500da;
    PTN_REG_WRITE(phys_regs[0x3f]+0x64, _tmp); // .

    _tmp = (PTN_REG_READ(phys_regs[0x3f]+0x68)&0x0)|0x2804;
    PTN_REG_WRITE(phys_regs[0x3f]+0x68, _tmp); // .

    _tmp = (PTN_REG_READ(phys_regs[0x3f]+0x6c)&0xffffff80)|0x16;
    PTN_REG_WRITE(phys_regs[0x3f]+0x6c, _tmp); // .

    _tmp = (PTN_REG_READ(phys_regs[0x3f]+0x64)&0x0)|0xc434e0d8;
    PTN_REG_WRITE(phys_regs[0x3f]+0x64, _tmp); // .

    _tmp = (PTN_REG_READ(phys_regs[0x3f]+0x68)&0x0)|0x2884;
    PTN_REG_WRITE(phys_regs[0x3f]+0x68, _tmp); // .

    _tmp = (PTN_REG_READ(phys_regs[0x3f]+0x6c)&0xffffff80)|0x17;
    PTN_REG_WRITE(phys_regs[0x3f]+0x6c, _tmp); // .

    _tmp = (PTN_REG_READ(phys_regs[0x3f]+0x64)&0x0)|0xc834c0d6;
    PTN_REG_WRITE(phys_regs[0x3f]+0x64, _tmp); // .

    _tmp = (PTN_REG_READ(phys_regs[0x3f]+0x68)&0x0)|0x2904;
    PTN_REG_WRITE(phys_regs[0x3f]+0x68, _tmp); // .

    _tmp = (PTN_REG_READ(phys_regs[0x3f]+0x6c)&0xffffff80)|0x18;
    PTN_REG_WRITE(phys_regs[0x3f]+0x6c, _tmp); // .

    _tmp = (PTN_REG_READ(phys_regs[0x3f]+0x64)&0x0)|0xd034a0d4;
    PTN_REG_WRITE(phys_regs[0x3f]+0x64, _tmp); // .

    _tmp = (PTN_REG_READ(phys_regs[0x3f]+0x68)&0x0)|0x2984;
    PTN_REG_WRITE(phys_regs[0x3f]+0x68, _tmp); // .

    _tmp = (PTN_REG_READ(phys_regs[0x3f]+0x6c)&0xffffff80)|0x19;
    PTN_REG_WRITE(phys_regs[0x3f]+0x6c, _tmp); // .

    _tmp = (PTN_REG_READ(phys_regs[0x3f]+0x64)&0x0)|0xd83480d2;
    PTN_REG_WRITE(phys_regs[0x3f]+0x64, _tmp); // .

    _tmp = (PTN_REG_READ(phys_regs[0x3f]+0x68)&0x0)|0x2a04;
    PTN_REG_WRITE(phys_regs[0x3f]+0x68, _tmp); // .

    _tmp = (PTN_REG_READ(phys_regs[0x3f]+0x6c)&0xffffff80)|0x1a;
    PTN_REG_WRITE(phys_regs[0x3f]+0x6c, _tmp); // .

    _tmp = (PTN_REG_READ(phys_regs[0x3f]+0x64)&0x0)|0xe03460d0;
    PTN_REG_WRITE(phys_regs[0x3f]+0x64, _tmp); // .

    _tmp = (PTN_REG_READ(phys_regs[0x3f]+0x68)&0x0)|0x2a84;
    PTN_REG_WRITE(phys_regs[0x3f]+0x68, _tmp); // .

    _tmp = (PTN_REG_READ(phys_regs[0x3f]+0x6c)&0xffffff80)|0x1b;
    PTN_REG_WRITE(phys_regs[0x3f]+0x6c, _tmp); // .

    _tmp = (PTN_REG_READ(phys_regs[0x3f]+0x64)&0x0)|0xe43460ce;
    PTN_REG_WRITE(phys_regs[0x3f]+0x64, _tmp); // .

    _tmp = (PTN_REG_READ(phys_regs[0x3f]+0x68)&0x0)|0x2b04;
    PTN_REG_WRITE(phys_regs[0x3f]+0x68, _tmp); // .

    _tmp = (PTN_REG_READ(phys_regs[0x3f]+0x6c)&0xffffff80)|0x1c;
    PTN_REG_WRITE(phys_regs[0x3f]+0x6c, _tmp); // .

    _tmp = (PTN_REG_READ(phys_regs[0x3f]+0x64)&0x0)|0xec3440cc;
    PTN_REG_WRITE(phys_regs[0x3f]+0x64, _tmp); // .

    _tmp = (PTN_REG_READ(phys_regs[0x3f]+0x68)&0x0)|0x2c04;
    PTN_REG_WRITE(phys_regs[0x3f]+0x68, _tmp); // .

    _tmp = (PTN_REG_READ(phys_regs[0x3f]+0x6c)&0xffffff80)|0x1d;
    PTN_REG_WRITE(phys_regs[0x3f]+0x6c, _tmp); // .

    _tmp = (PTN_REG_READ(phys_regs[0x3f]+0x64)&0x0)|0xf43420ca;
    PTN_REG_WRITE(phys_regs[0x3f]+0x64, _tmp); // .

    _tmp = (PTN_REG_READ(phys_regs[0x3f]+0x68)&0x0)|0x2c84;
    PTN_REG_WRITE(phys_regs[0x3f]+0x68, _tmp); // .

    _tmp = (PTN_REG_READ(phys_regs[0x3f]+0x6c)&0xffffff80)|0x1e;
    PTN_REG_WRITE(phys_regs[0x3f]+0x6c, _tmp); // .

    _tmp = (PTN_REG_READ(phys_regs[0x3f]+0x64)&0x0)|0xfc3400c7;
    PTN_REG_WRITE(phys_regs[0x3f]+0x64, _tmp); // .

    _tmp = (PTN_REG_READ(phys_regs[0x3f]+0x68)&0x0)|0x2d04;
    PTN_REG_WRITE(phys_regs[0x3f]+0x68, _tmp); // .

    _tmp = (PTN_REG_READ(phys_regs[0x3f]+0x6c)&0xffffff80)|0x1f;
    PTN_REG_WRITE(phys_regs[0x3f]+0x6c, _tmp); // .

    _tmp = (PTN_REG_READ(phys_regs[0x3f]+0x64)&0x0)|0x433e0c5;
    PTN_REG_WRITE(phys_regs[0x3f]+0x64, _tmp); // .

    _tmp = (PTN_REG_READ(phys_regs[0x3f]+0x68)&0x0)|0x2d85;
    PTN_REG_WRITE(phys_regs[0x3f]+0x68, _tmp); // .

    _tmp = (PTN_REG_READ(phys_regs[0x3f]+0x6c)&0xffffff80)|0x20;
    PTN_REG_WRITE(phys_regs[0x3f]+0x6c, _tmp); // .

    _tmp = (PTN_REG_READ(phys_regs[0x3f]+0x64)&0x0)|0x833c0c3;
    PTN_REG_WRITE(phys_regs[0x3f]+0x64, _tmp); // .

    _tmp = (PTN_REG_READ(phys_regs[0x3f]+0x68)&0x0)|0x2e05;
    PTN_REG_WRITE(phys_regs[0x3f]+0x68, _tmp); // .

    _tmp = (PTN_REG_READ(phys_regs[0x3f]+0x6c)&0xffffff80)|0x21;
    PTN_REG_WRITE(phys_regs[0x3f]+0x6c, _tmp); // .

    _tmp = (PTN_REG_READ(phys_regs[0x3f]+0x64)&0x0)|0x1033a0c2;
    PTN_REG_WRITE(phys_regs[0x3f]+0x64, _tmp); // .

    _tmp = (PTN_REG_READ(phys_regs[0x3f]+0x68)&0x0)|0x2f05;
    PTN_REG_WRITE(phys_regs[0x3f]+0x68, _tmp); // .

    _tmp = (PTN_REG_READ(phys_regs[0x3f]+0x6c)&0xffffff80)|0x22;
    PTN_REG_WRITE(phys_regs[0x3f]+0x6c, _tmp); // .

    _tmp = (PTN_REG_READ(phys_regs[0x3f]+0x64)&0x0)|0x183380c0;
    PTN_REG_WRITE(phys_regs[0x3f]+0x64, _tmp); // .

    _tmp = (PTN_REG_READ(phys_regs[0x3f]+0x68)&0x0)|0x2f85;
    PTN_REG_WRITE(phys_regs[0x3f]+0x68, _tmp); // .

    _tmp = (PTN_REG_READ(phys_regs[0x3f]+0x6c)&0xffffff80)|0x23;
    PTN_REG_WRITE(phys_regs[0x3f]+0x6c, _tmp); // .

    _tmp = (PTN_REG_READ(phys_regs[0x3f]+0x64)&0x0)|0x203360be;
    PTN_REG_WRITE(phys_regs[0x3f]+0x64, _tmp); // .

    _tmp = (PTN_REG_READ(phys_regs[0x3f]+0x68)&0x0)|0x3005;
    PTN_REG_WRITE(phys_regs[0x3f]+0x68, _tmp); // .

    _tmp = (PTN_REG_READ(phys_regs[0x3f]+0x6c)&0xffffff80)|0x24;
    PTN_REG_WRITE(phys_regs[0x3f]+0x6c, _tmp); // .

    _tmp = (PTN_REG_READ(phys_regs[0x3f]+0x64)&0x0)|0x243340bc;
    PTN_REG_WRITE(phys_regs[0x3f]+0x64, _tmp); // .

    _tmp = (PTN_REG_READ(phys_regs[0x3f]+0x68)&0x0)|0x3085;
    PTN_REG_WRITE(phys_regs[0x3f]+0x68, _tmp); // .

    _tmp = (PTN_REG_READ(phys_regs[0x3f]+0x6c)&0xffffff80)|0x25;
    PTN_REG_WRITE(phys_regs[0x3f]+0x6c, _tmp); // .

    _tmp = (PTN_REG_READ(phys_regs[0x3f]+0x64)&0x0)|0x2c3320ba;
    PTN_REG_WRITE(phys_regs[0x3f]+0x64, _tmp); // .

    _tmp = (PTN_REG_READ(phys_regs[0x3f]+0x68)&0x0)|0x3185;
    PTN_REG_WRITE(phys_regs[0x3f]+0x68, _tmp); // .

    _tmp = (PTN_REG_READ(phys_regs[0x3f]+0x6c)&0xffffff80)|0x26;
    PTN_REG_WRITE(phys_regs[0x3f]+0x6c, _tmp); // .

    _tmp = (PTN_REG_READ(phys_regs[0x3f]+0x64)&0x0)|0x343300b8;
    PTN_REG_WRITE(phys_regs[0x3f]+0x64, _tmp); // .

    _tmp = (PTN_REG_READ(phys_regs[0x3f]+0x68)&0x0)|0x3205;
    PTN_REG_WRITE(phys_regs[0x3f]+0x68, _tmp); // .

    _tmp = (PTN_REG_READ(phys_regs[0x3f]+0x6c)&0xffffff80)|0x27;
    PTN_REG_WRITE(phys_regs[0x3f]+0x6c, _tmp); // .

    _tmp = (PTN_REG_READ(phys_regs[0x3f]+0x64)&0x0)|0x3832e0b6;
    PTN_REG_WRITE(phys_regs[0x3f]+0x64, _tmp); // .

    _tmp = (PTN_REG_READ(phys_regs[0x3f]+0x68)&0x0)|0x3285;
    PTN_REG_WRITE(phys_regs[0x3f]+0x68, _tmp); // .

    _tmp = (PTN_REG_READ(phys_regs[0x3f]+0x6c)&0xffffff80)|0x28;
    PTN_REG_WRITE(phys_regs[0x3f]+0x6c, _tmp); // .

    _tmp = (PTN_REG_READ(phys_regs[0x3f]+0x64)&0x0)|0x4032a0b4;
    PTN_REG_WRITE(phys_regs[0x3f]+0x64, _tmp); // .

    _tmp = (PTN_REG_READ(phys_regs[0x3f]+0x68)&0x0)|0x3385;
    PTN_REG_WRITE(phys_regs[0x3f]+0x68, _tmp); // .

    _tmp = (PTN_REG_READ(phys_regs[0x3f]+0x6c)&0xffffff80)|0x29;
    PTN_REG_WRITE(phys_regs[0x3f]+0x6c, _tmp); // .

    _tmp = (PTN_REG_READ(phys_regs[0x3f]+0x64)&0x0)|0x483280b2;
    PTN_REG_WRITE(phys_regs[0x3f]+0x64, _tmp); // .

    _tmp = (PTN_REG_READ(phys_regs[0x3f]+0x68)&0x0)|0x3405;
    PTN_REG_WRITE(phys_regs[0x3f]+0x68, _tmp); // .

    _tmp = (PTN_REG_READ(phys_regs[0x3f]+0x6c)&0xffffff80)|0x2a;
    PTN_REG_WRITE(phys_regs[0x3f]+0x6c, _tmp); // .

    _tmp = (PTN_REG_READ(phys_regs[0x3f]+0x64)&0x0)|0x503260b0;
    PTN_REG_WRITE(phys_regs[0x3f]+0x64, _tmp); // .

    _tmp = (PTN_REG_READ(phys_regs[0x3f]+0x68)&0x0)|0x3485;
    PTN_REG_WRITE(phys_regs[0x3f]+0x68, _tmp); // .

    _tmp = (PTN_REG_READ(phys_regs[0x3f]+0x6c)&0xffffff80)|0x2b;
    PTN_REG_WRITE(phys_regs[0x3f]+0x6c, _tmp); // .

    _tmp = (PTN_REG_READ(phys_regs[0x3f]+0x64)&0x0)|0x543240ae;
    PTN_REG_WRITE(phys_regs[0x3f]+0x64, _tmp); // .

    _tmp = (PTN_REG_READ(phys_regs[0x3f]+0x68)&0x0)|0x3585;
    PTN_REG_WRITE(phys_regs[0x3f]+0x68, _tmp); // .

    _tmp = (PTN_REG_READ(phys_regs[0x3f]+0x6c)&0xffffff80)|0x2c;
    PTN_REG_WRITE(phys_regs[0x3f]+0x6c, _tmp); // .

    _tmp = (PTN_REG_READ(phys_regs[0x3f]+0x64)&0x0)|0x5c3220ac;
    PTN_REG_WRITE(phys_regs[0x3f]+0x64, _tmp); // .

    _tmp = (PTN_REG_READ(phys_regs[0x3f]+0x68)&0x0)|0x3605;
    PTN_REG_WRITE(phys_regs[0x3f]+0x68, _tmp); // .

    _tmp = (PTN_REG_READ(phys_regs[0x3f]+0x6c)&0xffffff80)|0x2d;
    PTN_REG_WRITE(phys_regs[0x3f]+0x6c, _tmp); // .

    _tmp = (PTN_REG_READ(phys_regs[0x3f]+0x64)&0x0)|0x643200ab;
    PTN_REG_WRITE(phys_regs[0x3f]+0x64, _tmp); // .

    _tmp = (PTN_REG_READ(phys_regs[0x3f]+0x68)&0x0)|0x3685;
    PTN_REG_WRITE(phys_regs[0x3f]+0x68, _tmp); // .

    _tmp = (PTN_REG_READ(phys_regs[0x3f]+0x6c)&0xffffff80)|0x2e;
    PTN_REG_WRITE(phys_regs[0x3f]+0x6c, _tmp); // .

    _tmp = (PTN_REG_READ(phys_regs[0x3f]+0x64)&0x0)|0x6831c0a9;
    PTN_REG_WRITE(phys_regs[0x3f]+0x64, _tmp); // .

    _tmp = (PTN_REG_READ(phys_regs[0x3f]+0x68)&0x0)|0x3785;
    PTN_REG_WRITE(phys_regs[0x3f]+0x68, _tmp); // .

    _tmp = (PTN_REG_READ(phys_regs[0x3f]+0x6c)&0xffffff80)|0x2f;
    PTN_REG_WRITE(phys_regs[0x3f]+0x6c, _tmp); // .

    _tmp = (PTN_REG_READ(phys_regs[0x3f]+0x64)&0x0)|0x7031a0a7;
    PTN_REG_WRITE(phys_regs[0x3f]+0x64, _tmp); // .

    _tmp = (PTN_REG_READ(phys_regs[0x3f]+0x68)&0x0)|0x3805;
    PTN_REG_WRITE(phys_regs[0x3f]+0x68, _tmp); // .

    _tmp = (PTN_REG_READ(phys_regs[0x3f]+0x6c)&0xffffff80)|0x30;
    PTN_REG_WRITE(phys_regs[0x3f]+0x6c, _tmp); // .

    _tmp = (PTN_REG_READ(phys_regs[0x3f]+0x64)&0x0)|0x743180a5;
    PTN_REG_WRITE(phys_regs[0x3f]+0x64, _tmp); // .

    _tmp = (PTN_REG_READ(phys_regs[0x3f]+0x68)&0x0)|0x3885;
    PTN_REG_WRITE(phys_regs[0x3f]+0x68, _tmp); // .

    _tmp = (PTN_REG_READ(phys_regs[0x3f]+0x6c)&0xffffff80)|0x31;
    PTN_REG_WRITE(phys_regs[0x3f]+0x6c, _tmp); // .

    _tmp = (PTN_REG_READ(phys_regs[0x3f]+0x64)&0x0)|0x7c3160a3;
    PTN_REG_WRITE(phys_regs[0x3f]+0x64, _tmp); // .

    _tmp = (PTN_REG_READ(phys_regs[0x3f]+0x68)&0x0)|0x3985;
    PTN_REG_WRITE(phys_regs[0x3f]+0x68, _tmp); // .

    _tmp = (PTN_REG_READ(phys_regs[0x3f]+0x6c)&0xffffff80)|0x32;
    PTN_REG_WRITE(phys_regs[0x3f]+0x6c, _tmp); // .

    _tmp = (PTN_REG_READ(phys_regs[0x3f]+0x64)&0x0)|0x843120a1;
    PTN_REG_WRITE(phys_regs[0x3f]+0x64, _tmp); // .

    _tmp = (PTN_REG_READ(phys_regs[0x3f]+0x68)&0x0)|0x3a05;
    PTN_REG_WRITE(phys_regs[0x3f]+0x68, _tmp); // .

    _tmp = (PTN_REG_READ(phys_regs[0x3f]+0x6c)&0xffffff80)|0x33;
    PTN_REG_WRITE(phys_regs[0x3f]+0x6c, _tmp); // .

    _tmp = (PTN_REG_READ(phys_regs[0x3f]+0x64)&0x0)|0x883100a0;
    PTN_REG_WRITE(phys_regs[0x3f]+0x64, _tmp); // .

    _tmp = (PTN_REG_READ(phys_regs[0x3f]+0x68)&0x0)|0x3b05;
    PTN_REG_WRITE(phys_regs[0x3f]+0x68, _tmp); // .

    _tmp = (PTN_REG_READ(phys_regs[0x3f]+0x6c)&0xffffff80)|0x34;
    PTN_REG_WRITE(phys_regs[0x3f]+0x6c, _tmp); // .

    _tmp = (PTN_REG_READ(phys_regs[0x3f]+0x64)&0x0)|0x9030e09e;
    PTN_REG_WRITE(phys_regs[0x3f]+0x64, _tmp); // .

    _tmp = (PTN_REG_READ(phys_regs[0x3f]+0x68)&0x0)|0x3b85;
    PTN_REG_WRITE(phys_regs[0x3f]+0x68, _tmp); // .

    _tmp = (PTN_REG_READ(phys_regs[0x3f]+0x6c)&0xffffff80)|0x35;
    PTN_REG_WRITE(phys_regs[0x3f]+0x6c, _tmp); // .

    _tmp = (PTN_REG_READ(phys_regs[0x3f]+0x64)&0x0)|0x9830c09c;
    PTN_REG_WRITE(phys_regs[0x3f]+0x64, _tmp); // .

    _tmp = (PTN_REG_READ(phys_regs[0x3f]+0x68)&0x0)|0x3c85;
    PTN_REG_WRITE(phys_regs[0x3f]+0x68, _tmp); // .

    _tmp = (PTN_REG_READ(phys_regs[0x3f]+0x6c)&0xffffff80)|0x36;
    PTN_REG_WRITE(phys_regs[0x3f]+0x6c, _tmp); // .

    _tmp = (PTN_REG_READ(phys_regs[0x3f]+0x64)&0x0)|0x9c30809a;
    PTN_REG_WRITE(phys_regs[0x3f]+0x64, _tmp); // .

    _tmp = (PTN_REG_READ(phys_regs[0x3f]+0x68)&0x0)|0x3d05;
    PTN_REG_WRITE(phys_regs[0x3f]+0x68, _tmp); // .

    _tmp = (PTN_REG_READ(phys_regs[0x3f]+0x6c)&0xffffff80)|0x37;
    PTN_REG_WRITE(phys_regs[0x3f]+0x6c, _tmp); // .

    _tmp = (PTN_REG_READ(phys_regs[0x3f]+0x64)&0x0)|0xa4306099;
    PTN_REG_WRITE(phys_regs[0x3f]+0x64, _tmp); // .

    _tmp = (PTN_REG_READ(phys_regs[0x3f]+0x68)&0x0)|0x3e05;
    PTN_REG_WRITE(phys_regs[0x3f]+0x68, _tmp); // .

    _tmp = (PTN_REG_READ(phys_regs[0x3f]+0x6c)&0xffffff80)|0x38;
    PTN_REG_WRITE(phys_regs[0x3f]+0x6c, _tmp); // .

    _tmp = (PTN_REG_READ(phys_regs[0x3f]+0x64)&0x0)|0xa8304097;
    PTN_REG_WRITE(phys_regs[0x3f]+0x64, _tmp); // .

    _tmp = (PTN_REG_READ(phys_regs[0x3f]+0x68)&0x0)|0x3e85;
    PTN_REG_WRITE(phys_regs[0x3f]+0x68, _tmp); // .

    _tmp = (PTN_REG_READ(phys_regs[0x3f]+0x6c)&0xffffff80)|0x39;
    PTN_REG_WRITE(phys_regs[0x3f]+0x6c, _tmp); // .

    _tmp = (PTN_REG_READ(phys_regs[0x3f]+0x64)&0x0)|0xb0300095;
    PTN_REG_WRITE(phys_regs[0x3f]+0x64, _tmp); // .

    _tmp = (PTN_REG_READ(phys_regs[0x3f]+0x68)&0x0)|0x3f85;
    PTN_REG_WRITE(phys_regs[0x3f]+0x68, _tmp); // .

    _tmp = (PTN_REG_READ(phys_regs[0x3f]+0x6c)&0xffffff80)|0x3a;
    PTN_REG_WRITE(phys_regs[0x3f]+0x6c, _tmp); // .

    _tmp = (PTN_REG_READ(phys_regs[0x3f]+0x64)&0x0)|0xb42fe094;
    PTN_REG_WRITE(phys_regs[0x3f]+0x64, _tmp); // .

    _tmp = (PTN_REG_READ(phys_regs[0x3f]+0x68)&0x0)|0x4005;
    PTN_REG_WRITE(phys_regs[0x3f]+0x68, _tmp); // .

    _tmp = (PTN_REG_READ(phys_regs[0x3f]+0x6c)&0xffffff80)|0x3b;
    PTN_REG_WRITE(phys_regs[0x3f]+0x6c, _tmp); // .

    _tmp = (PTN_REG_READ(phys_regs[0x3f]+0x64)&0x0)|0xbc2fa092;
    PTN_REG_WRITE(phys_regs[0x3f]+0x64, _tmp); // .

    _tmp = (PTN_REG_READ(phys_regs[0x3f]+0x68)&0x0)|0x4105;
    PTN_REG_WRITE(phys_regs[0x3f]+0x68, _tmp); // .

    _tmp = (PTN_REG_READ(phys_regs[0x3f]+0x6c)&0xffffff80)|0x3c;
    PTN_REG_WRITE(phys_regs[0x3f]+0x6c, _tmp); // .

    _tmp = (PTN_REG_READ(phys_regs[0x3f]+0x64)&0x0)|0xc02f8090;
    PTN_REG_WRITE(phys_regs[0x3f]+0x64, _tmp); // .

    _tmp = (PTN_REG_READ(phys_regs[0x3f]+0x68)&0x0)|0x4185;
    PTN_REG_WRITE(phys_regs[0x3f]+0x68, _tmp); // .

    _tmp = (PTN_REG_READ(phys_regs[0x3f]+0x6c)&0xffffff80)|0x3d;
    PTN_REG_WRITE(phys_regs[0x3f]+0x6c, _tmp); // .

    _tmp = (PTN_REG_READ(phys_regs[0x3f]+0x64)&0x0)|0xc82f608f;
    PTN_REG_WRITE(phys_regs[0x3f]+0x64, _tmp); // .

    _tmp = (PTN_REG_READ(phys_regs[0x3f]+0x68)&0x0)|0x4285;
    PTN_REG_WRITE(phys_regs[0x3f]+0x68, _tmp); // .

    _tmp = (PTN_REG_READ(phys_regs[0x3f]+0x6c)&0xffffff80)|0x3e;
    PTN_REG_WRITE(phys_regs[0x3f]+0x6c, _tmp); // .

    _tmp = (PTN_REG_READ(phys_regs[0x3f]+0x64)&0x0)|0xcc2f208d;
    PTN_REG_WRITE(phys_regs[0x3f]+0x64, _tmp); // .

    _tmp = (PTN_REG_READ(phys_regs[0x3f]+0x68)&0x0)|0x4305;
    PTN_REG_WRITE(phys_regs[0x3f]+0x68, _tmp); // .

    _tmp = (PTN_REG_READ(phys_regs[0x3f]+0x6c)&0xffffff80)|0x3f;
    PTN_REG_WRITE(phys_regs[0x3f]+0x6c, _tmp); // .

    _tmp = (PTN_REG_READ(phys_regs[0x3f]+0x64)&0x0)|0xd42f008b;
    PTN_REG_WRITE(phys_regs[0x3f]+0x64, _tmp); // .

    _tmp = (PTN_REG_READ(phys_regs[0x3f]+0x68)&0x0)|0x4405;
    PTN_REG_WRITE(phys_regs[0x3f]+0x68, _tmp); // .

    _tmp = (PTN_REG_READ(phys_regs[0x3f]+0x6c)&0xffffff80)|0x40;
    PTN_REG_WRITE(phys_regs[0x3f]+0x6c, _tmp); // .

    _tmp = (PTN_REG_READ(phys_regs[0x3f]+0x64)&0x0)|0xd82ec08a;
    PTN_REG_WRITE(phys_regs[0x3f]+0x64, _tmp); // .

    _tmp = (PTN_REG_READ(phys_regs[0x3f]+0x68)&0x0)|0x4505;
    PTN_REG_WRITE(phys_regs[0x3f]+0x68, _tmp); // .

    _tmp = (PTN_REG_READ(phys_regs[0x3f]+0x6c)&0xffffff7f)|0x0;
    PTN_REG_WRITE(phys_regs[0x3f]+0x6c, _tmp); // .

    _tmp = (PTN_REG_READ(phys_regs[0x3f]+0x0)&0x7fffffff)|0x0;
    PTN_REG_WRITE(phys_regs[0x3f]+0x0, _tmp); // isp_mmap.reg_dma_bypass

    _tmp = (PTN_REG_READ(phys_regs[0x3f]+0x6c)&0xffff00ff)|0xff00;
    PTN_REG_WRITE(phys_regs[0x3f]+0x6c, _tmp); // isp_mmap.reg_dma_enable

    _tmp = (PTN_REG_READ(phys_regs[0x61]+0x800)&0x0)|0x41000000;
    PTN_REG_WRITE(phys_regs[0x61]+0x800, _tmp); // isp_dma25.reg_dma_addr_l

    _tmp = (PTN_REG_READ(phys_regs[0x61]+0x804)&0xffffff00)|0x1;
    PTN_REG_WRITE(phys_regs[0x61]+0x804, _tmp); // isp_dma25.reg_dma_addr_h

    _tmp = (PTN_REG_READ(phys_regs[0x61]+0x808)&0xffff0000)|0x51c0;
    PTN_REG_WRITE(phys_regs[0x61]+0x808, _tmp); // isp_dma25.reg_dma_cnt

    _tmp = (PTN_REG_READ(phys_regs[0x61]+0x824)&0xffffe000)|0x28d;
    PTN_REG_WRITE(phys_regs[0x61]+0x824, _tmp); // isp_dma25.reg_img_width

    _tmp = (PTN_REG_READ(phys_regs[0x61]+0x824)&0xe000ffff)|0x2d0000;
    PTN_REG_WRITE(phys_regs[0x61]+0x824, _tmp); // isp_dma25.reg_img_height

    _tmp = (PTN_REG_READ(phys_regs[0x61]+0x828)&0xfff8ffff)|0x40000;
    PTN_REG_WRITE(phys_regs[0x61]+0x828, _tmp); // isp_dma25.reg_mod_ctl

    _tmp = (PTN_REG_READ(phys_regs[0x61]+0x828)&0xffff0000)|0x51c0;
    PTN_REG_WRITE(phys_regs[0x61]+0x828, _tmp); // isp_dma25.reg_line_offset

    _tmp = (PTN_REG_READ(phys_regs[0x61]+0x900)&0x0)|0x42000000;
    PTN_REG_WRITE(phys_regs[0x61]+0x900, _tmp); // isp_dma26.reg_dma_addr_l

    _tmp = (PTN_REG_READ(phys_regs[0x61]+0x904)&0xffffff00)|0x1;
    PTN_REG_WRITE(phys_regs[0x61]+0x904, _tmp); // isp_dma26.reg_dma_addr_h

    _tmp = (PTN_REG_READ(phys_regs[0x61]+0x908)&0xffff0000)|0x51c0;
    PTN_REG_WRITE(phys_regs[0x61]+0x908, _tmp); // isp_dma26.reg_dma_cnt

    _tmp = (PTN_REG_READ(phys_regs[0x61]+0x924)&0xffffe000)|0x28d;
    PTN_REG_WRITE(phys_regs[0x61]+0x924, _tmp); // isp_dma26.reg_img_width

    _tmp = (PTN_REG_READ(phys_regs[0x61]+0x924)&0xe000ffff)|0x160000;
    PTN_REG_WRITE(phys_regs[0x61]+0x924, _tmp); // isp_dma26.reg_img_height

    _tmp = (PTN_REG_READ(phys_regs[0x61]+0x928)&0xfff8ffff)|0x40000;
    PTN_REG_WRITE(phys_regs[0x61]+0x928, _tmp); // isp_dma26.reg_mod_ctl

    _tmp = (PTN_REG_READ(phys_regs[0x61]+0x928)&0xffff0000)|0x51c0;
    PTN_REG_WRITE(phys_regs[0x61]+0x928, _tmp); // isp_dma26.reg_line_offset

    _tmp = (PTN_REG_READ(phys_regs[0x61]+0xb00)&0x0)|0x41000000;
    PTN_REG_WRITE(phys_regs[0x61]+0xb00, _tmp); // isp_dma27.reg_dma_addr_l

    _tmp = (PTN_REG_READ(phys_regs[0x61]+0xb04)&0xffffff00)|0x1;
    PTN_REG_WRITE(phys_regs[0x61]+0xb04, _tmp); // isp_dma27.reg_dma_addr_h

    _tmp = (PTN_REG_READ(phys_regs[0x61]+0xb08)&0xffff0000)|0x51c0;
    PTN_REG_WRITE(phys_regs[0x61]+0xb08, _tmp); // isp_dma27.reg_dma_cnt

    _tmp = (PTN_REG_READ(phys_regs[0x61]+0xb24)&0xffffe000)|0x28d;
    PTN_REG_WRITE(phys_regs[0x61]+0xb24, _tmp); // isp_dma27.reg_img_width

    _tmp = (PTN_REG_READ(phys_regs[0x61]+0xb24)&0xe000ffff)|0x2d0000;
    PTN_REG_WRITE(phys_regs[0x61]+0xb24, _tmp); // isp_dma27.reg_img_height

    _tmp = (PTN_REG_READ(phys_regs[0x61]+0xb28)&0xfff8ffff)|0x40000;
    PTN_REG_WRITE(phys_regs[0x61]+0xb28, _tmp); // isp_dma27.reg_mod_ctl

    _tmp = (PTN_REG_READ(phys_regs[0x61]+0xb28)&0xffff0000)|0x51c0;
    PTN_REG_WRITE(phys_regs[0x61]+0xb28, _tmp); // isp_dma27.reg_line_offset

    _tmp = (PTN_REG_READ(phys_regs[0x61]+0xc00)&0x0)|0x42000000;
    PTN_REG_WRITE(phys_regs[0x61]+0xc00, _tmp); // isp_dma28.reg_dma_addr_l

    _tmp = (PTN_REG_READ(phys_regs[0x61]+0xc04)&0xffffff00)|0x1;
    PTN_REG_WRITE(phys_regs[0x61]+0xc04, _tmp); // isp_dma28.reg_dma_addr_h

    _tmp = (PTN_REG_READ(phys_regs[0x61]+0xc08)&0xffff0000)|0x51c0;
    PTN_REG_WRITE(phys_regs[0x61]+0xc08, _tmp); // isp_dma28.reg_dma_cnt

    _tmp = (PTN_REG_READ(phys_regs[0x61]+0xc24)&0xffffe000)|0x28d;
    PTN_REG_WRITE(phys_regs[0x61]+0xc24, _tmp); // isp_dma28.reg_img_width

    _tmp = (PTN_REG_READ(phys_regs[0x61]+0xc24)&0xe000ffff)|0x160000;
    PTN_REG_WRITE(phys_regs[0x61]+0xc24, _tmp); // isp_dma28.reg_img_height

    _tmp = (PTN_REG_READ(phys_regs[0x61]+0xc28)&0xfff8ffff)|0x40000;
    PTN_REG_WRITE(phys_regs[0x61]+0xc28, _tmp); // isp_dma28.reg_mod_ctl

    _tmp = (PTN_REG_READ(phys_regs[0x61]+0xc28)&0xffff0000)|0x51c0;
    PTN_REG_WRITE(phys_regs[0x61]+0xc28, _tmp); // isp_dma28.reg_line_offset

    _tmp = (PTN_REG_READ(phys_regs[0x3f]+0x800)&0x0)| bufaddr;
    PTN_REG_WRITE(phys_regs[0x3f]+0x800, _tmp); // isp_dma19.reg_dma_addr_l

    _tmp = (PTN_REG_READ(phys_regs[0x3f]+0x804)&0xffffff00)| (bufaddr >> 32);
    PTN_REG_WRITE(phys_regs[0x3f]+0x804, _tmp); // isp_dma19.reg_dma_addr_h

    _tmp = (PTN_REG_READ(phys_regs[0x3f]+0x808)&0xffff0000)|0x300;
    PTN_REG_WRITE(phys_regs[0x3f]+0x808, _tmp); // isp_dma19.reg_dma_cnt

    _tmp = (PTN_REG_READ(phys_regs[0x3f]+0x824)&0xffffe000)|0x17;
    PTN_REG_WRITE(phys_regs[0x3f]+0x824, _tmp); // isp_dma19.reg_img_width

    _tmp = (PTN_REG_READ(phys_regs[0x3f]+0x824)&0xe000ffff)|0x5b0000;
    PTN_REG_WRITE(phys_regs[0x3f]+0x824, _tmp); // isp_dma19.reg_img_height

    _tmp = (PTN_REG_READ(phys_regs[0x3f]+0x828)&0xfff8ffff)|0x40000;
    PTN_REG_WRITE(phys_regs[0x3f]+0x828, _tmp); // isp_dma19.reg_mod_ctl

    _tmp = (PTN_REG_READ(phys_regs[0x3f]+0x828)&0xffff0000)|0x300;
    PTN_REG_WRITE(phys_regs[0x3f]+0x828, _tmp); // isp_dma19.reg_line_offset

    _tmp = (PTN_REG_READ(phys_regs[0x3f]+0x900)&0x0)|0x43000000;
    PTN_REG_WRITE(phys_regs[0x3f]+0x900, _tmp); // isp_dma20.reg_dma_addr_l

    _tmp = (PTN_REG_READ(phys_regs[0x3f]+0x904)&0xffffff00)|0x1;
    PTN_REG_WRITE(phys_regs[0x3f]+0x904, _tmp); // isp_dma20.reg_dma_addr_h

    _tmp = (PTN_REG_READ(phys_regs[0x3f]+0x908)&0xffff0000)|0x60;
    PTN_REG_WRITE(phys_regs[0x3f]+0x908, _tmp); // isp_dma20.reg_dma_cnt

    _tmp = (PTN_REG_READ(phys_regs[0x3f]+0x924)&0xffffe000)|0x2;
    PTN_REG_WRITE(phys_regs[0x3f]+0x924, _tmp); // isp_dma20.reg_img_width

    _tmp = (PTN_REG_READ(phys_regs[0x3f]+0x924)&0xe000ffff)|0x70000;
    PTN_REG_WRITE(phys_regs[0x3f]+0x924, _tmp); // isp_dma20.reg_img_height

    _tmp = (PTN_REG_READ(phys_regs[0x3f]+0x928)&0xfff8ffff)|0x40000;
    PTN_REG_WRITE(phys_regs[0x3f]+0x928, _tmp); // isp_dma20.reg_mod_ctl

    _tmp = (PTN_REG_READ(phys_regs[0x3f]+0x928)&0xffff0000)|0x60;
    PTN_REG_WRITE(phys_regs[0x3f]+0x928, _tmp); // isp_dma20.reg_line_offset

    _tmp = (PTN_REG_READ(phys_regs[0x3f]+0xa00)&0x0)|0x44000000;
    PTN_REG_WRITE(phys_regs[0x3f]+0xa00, _tmp); // isp_dma21.reg_dma_addr_l

    _tmp = (PTN_REG_READ(phys_regs[0x3f]+0xa04)&0xffffff00)|0x1;
    PTN_REG_WRITE(phys_regs[0x3f]+0xa04, _tmp); // isp_dma21.reg_dma_addr_h

    _tmp = (PTN_REG_READ(phys_regs[0x3f]+0xa08)&0xffff0000)|0x60;
    PTN_REG_WRITE(phys_regs[0x3f]+0xa08, _tmp); // isp_dma21.reg_dma_cnt

    _tmp = (PTN_REG_READ(phys_regs[0x3f]+0xa24)&0xffffe000)|0x2;
    PTN_REG_WRITE(phys_regs[0x3f]+0xa24, _tmp); // isp_dma21.reg_img_width

    _tmp = (PTN_REG_READ(phys_regs[0x3f]+0xa24)&0xe000ffff)|0x70000;
    PTN_REG_WRITE(phys_regs[0x3f]+0xa24, _tmp); // isp_dma21.reg_img_height

    _tmp = (PTN_REG_READ(phys_regs[0x3f]+0xa28)&0xfff8ffff)|0x40000;
    PTN_REG_WRITE(phys_regs[0x3f]+0xa28, _tmp); // isp_dma21.reg_mod_ctl

    _tmp = (PTN_REG_READ(phys_regs[0x3f]+0xa28)&0xffff0000)|0x60;
    PTN_REG_WRITE(phys_regs[0x3f]+0xa28, _tmp); // isp_dma21.reg_line_offset

    _tmp = (PTN_REG_READ(phys_regs[0x3f]+0xb00)&0x0)|0x45000000;
    PTN_REG_WRITE(phys_regs[0x3f]+0xb00, _tmp); // isp_dma22.reg_dma_addr_l

    _tmp = (PTN_REG_READ(phys_regs[0x3f]+0xb04)&0xffffff00)|0x3;
    PTN_REG_WRITE(phys_regs[0x3f]+0xb04, _tmp); // isp_dma22.reg_dma_addr_h

    _tmp = (PTN_REG_READ(phys_regs[0x3f]+0xb08)&0xffff0000)|0x60;
    PTN_REG_WRITE(phys_regs[0x3f]+0xb08, _tmp); // isp_dma22.reg_dma_cnt

    _tmp = (PTN_REG_READ(phys_regs[0x3f]+0xb24)&0xffffe000)|0x2;
    PTN_REG_WRITE(phys_regs[0x3f]+0xb24, _tmp); // isp_dma22.reg_img_width

    _tmp = (PTN_REG_READ(phys_regs[0x3f]+0xb24)&0xe000ffff)|0x70000;
    PTN_REG_WRITE(phys_regs[0x3f]+0xb24, _tmp); // isp_dma22.reg_img_height

    _tmp = (PTN_REG_READ(phys_regs[0x3f]+0xb28)&0xfff8ffff)|0x40000;
    PTN_REG_WRITE(phys_regs[0x3f]+0xb28, _tmp); // isp_dma22.reg_mod_ctl

    _tmp = (PTN_REG_READ(phys_regs[0x3f]+0xb28)&0xffff0000)|0x60;
    PTN_REG_WRITE(phys_regs[0x3f]+0xb28, _tmp); // isp_dma22.reg_line_offset

    _tmp = (PTN_REG_READ(phys_regs[0x3f]+0xc00)&0x0)|0x47000000;
    PTN_REG_WRITE(phys_regs[0x3f]+0xc00, _tmp); // isp_dma23.reg_dma_addr_l

    _tmp = (PTN_REG_READ(phys_regs[0x3f]+0xc04)&0xffffff00)|0x1;
    PTN_REG_WRITE(phys_regs[0x3f]+0xc04, _tmp); // isp_dma23.reg_dma_addr_h

    _tmp = (PTN_REG_READ(phys_regs[0x3f]+0xc08)&0xffff0000)|0xc0;
    PTN_REG_WRITE(phys_regs[0x3f]+0xc08, _tmp); // isp_dma23.reg_dma_cnt

    _tmp = (PTN_REG_READ(phys_regs[0x3f]+0xc24)&0xffffe000)|0x5;
    PTN_REG_WRITE(phys_regs[0x3f]+0xc24, _tmp); // isp_dma23.reg_img_width

    _tmp = (PTN_REG_READ(phys_regs[0x3f]+0xc24)&0xe000ffff)|0x5b0000;
    PTN_REG_WRITE(phys_regs[0x3f]+0xc24, _tmp); // isp_dma23.reg_img_height

    _tmp = (PTN_REG_READ(phys_regs[0x3f]+0xc28)&0xfff8ffff)|0x40000;
    PTN_REG_WRITE(phys_regs[0x3f]+0xc28, _tmp); // isp_dma23.reg_mod_ctl

    _tmp = (PTN_REG_READ(phys_regs[0x3f]+0xc28)&0xffff0000)|0xc0;
    PTN_REG_WRITE(phys_regs[0x3f]+0xc28, _tmp); // isp_dma23.reg_line_offset

    _tmp = (PTN_REG_READ(phys_regs[0x3f]+0xe00)&0x0)|0x47000000;
    PTN_REG_WRITE(phys_regs[0x3f]+0xe00, _tmp); // isp_dma24.reg_dma_addr_l

    _tmp = (PTN_REG_READ(phys_regs[0x3f]+0xe04)&0xffffff00)|0x1;
    PTN_REG_WRITE(phys_regs[0x3f]+0xe04, _tmp); // isp_dma24.reg_dma_addr_h

    _tmp = (PTN_REG_READ(phys_regs[0x3f]+0xe08)&0xffff0000)|0xc0;
    PTN_REG_WRITE(phys_regs[0x3f]+0xe08, _tmp); // isp_dma24.reg_dma_cnt

    _tmp = (PTN_REG_READ(phys_regs[0x3f]+0xe24)&0xffffe000)|0x5;
    PTN_REG_WRITE(phys_regs[0x3f]+0xe24, _tmp); // isp_dma24.reg_img_width

    _tmp = (PTN_REG_READ(phys_regs[0x3f]+0xe24)&0xe000ffff)|0x5b0000;
    PTN_REG_WRITE(phys_regs[0x3f]+0xe24, _tmp); // isp_dma24.reg_img_height

    _tmp = (PTN_REG_READ(phys_regs[0x3f]+0xe28)&0xfff8ffff)|0x40000;
    PTN_REG_WRITE(phys_regs[0x3f]+0xe28, _tmp); // isp_dma24.reg_mod_ctl

    _tmp = (PTN_REG_READ(phys_regs[0x3f]+0xe28)&0xffff0000)|0xc0;
    PTN_REG_WRITE(phys_regs[0x3f]+0xe28, _tmp); // isp_dma24.reg_line_offset

#if 0
    _tmp = (PTN_REG_READ(phys_regs[0x61]+0x14)&0xfffffffb)|0x0;
    PTN_REG_WRITE(phys_regs[0x61]+0x14, _tmp); // isp_444_422.reg_dma_bypass

    _tmp = (PTN_REG_READ(phys_regs[0x61]+0x20)&0xffffc0ff)|0x3f00;
    PTN_REG_WRITE(phys_regs[0x61]+0x20, _tmp); // isp_444_422.reg_dma_enable

    _tmp = (PTN_REG_READ(phys_regs[0x61]+0x18)&0xffffc000)|0x51b;
    PTN_REG_WRITE(phys_regs[0x61]+0x18, _tmp); // isp_444_422.reg_img_width_trans

    _tmp = (PTN_REG_READ(phys_regs[0x61]+0x1c)&0xffffc000)|0x2df;
    PTN_REG_WRITE(phys_regs[0x61]+0x1c, _tmp); // isp_444_422.reg_img_height_trans

    _tmp = (PTN_REG_READ(phys_regs[0x3c]+0x4)&0xfffffff0)|0x1;
    PTN_REG_WRITE(phys_regs[0x3c]+0x4, _tmp); // .

    _tmp = (PTN_REG_READ(phys_regs[0x3a]+0x0)&0xfffffffe)|0x1;
    PTN_REG_WRITE(phys_regs[0x3a]+0x0, _tmp); // isp_fusion.reg_fs_enable

    _tmp = (PTN_REG_READ(phys_regs[0x3a]+0x0)&0xfffffffd)|0x2;
    PTN_REG_WRITE(phys_regs[0x3a]+0x0, _tmp); // isp_fusion.reg_fs_bpss

    _tmp = (PTN_REG_READ(phys_regs[0x3a]+0x0)&0xffffffef)|0x10;
    PTN_REG_WRITE(phys_regs[0x3a]+0x0, _tmp); // isp_fusion.reg_fs_mc_enable

    _tmp = (PTN_REG_READ(phys_regs[0x3a]+0x0)&0xfffffcff)|0x100;
    PTN_REG_WRITE(phys_regs[0x3a]+0x0, _tmp); // isp_fusion.reg_fs_out_sel

    _tmp = (PTN_REG_READ(phys_regs[0x3a]+0x0)&0xfff)|0xffff000;
    PTN_REG_WRITE(phys_regs[0x3a]+0x0, _tmp); // isp_fusion.reg_fs_s_max

    _tmp = (PTN_REG_READ(phys_regs[0x3a]+0x4)&0xffffe000)|0x51b;
    PTN_REG_WRITE(phys_regs[0x3a]+0x4, _tmp); // isp_fusion.reg_fs_widthm1

    _tmp = (PTN_REG_READ(phys_regs[0x3a]+0x4)&0xe000ffff)|0x2df0000;
    PTN_REG_WRITE(phys_regs[0x3a]+0x4, _tmp); // isp_fusion.reg_fs_heightm1

    _tmp = (PTN_REG_READ(phys_regs[0x3a]+0x8)&0xffffc000)|0x3ff;
    PTN_REG_WRITE(phys_regs[0x3a]+0x8, _tmp); // isp_fusion.reg_fs_ls_gain

    _tmp = (PTN_REG_READ(phys_regs[0x3a]+0xc)&0xfffff000)|0xf3c;
    PTN_REG_WRITE(phys_regs[0x3a]+0xc, _tmp); // isp_fusion.reg_fs_luma_thd_l

    _tmp = (PTN_REG_READ(phys_regs[0x3a]+0xc)&0xf000ffff)|0xfff0000;
    PTN_REG_WRITE(phys_regs[0x3a]+0xc, _tmp); // isp_fusion.reg_fs_luma_thd_h

    _tmp = (PTN_REG_READ(phys_regs[0x3a]+0x10)&0xffffff00)|0xff;
    PTN_REG_WRITE(phys_regs[0x3a]+0x10, _tmp); // isp_fusion.reg_fs_wgt_max

    _tmp = (PTN_REG_READ(phys_regs[0x3a]+0x10)&0xffff00ff)|0x0;
    PTN_REG_WRITE(phys_regs[0x3a]+0x10, _tmp); // isp_fusion.reg_fs_wgt_min

    _tmp = (PTN_REG_READ(phys_regs[0x3a]+0x14)&0xfffc0000)|0x53b;
    PTN_REG_WRITE(phys_regs[0x3a]+0x14, _tmp); // isp_fusion.reg_fs_wgt_slp

    _tmp = (PTN_REG_READ(phys_regs[0x3b]+0x0)&0xfffffffe)|0x1;
    PTN_REG_WRITE(phys_regs[0x3b]+0x0, _tmp); // isp_ltm.reg_ltm_enable

    _tmp = (PTN_REG_READ(phys_regs[0x3b]+0x0)&0xfffffffd)|0x0;
    PTN_REG_WRITE(phys_regs[0x3b]+0x0, _tmp); // isp_ltm.reg_dtone_ehn_en

    _tmp = (PTN_REG_READ(phys_regs[0x3b]+0x0)&0xfffffffb)|0x0;
    PTN_REG_WRITE(phys_regs[0x3b]+0x0, _tmp); // isp_ltm.reg_btone_ehn_en

    _tmp = (PTN_REG_READ(phys_regs[0x3b]+0x0)&0xfffffff7)|0x0;
    PTN_REG_WRITE(phys_regs[0x3b]+0x0, _tmp); // isp_ltm.reg_dark_lce_en

    _tmp = (PTN_REG_READ(phys_regs[0x3b]+0x0)&0xffffffef)|0x0;
    PTN_REG_WRITE(phys_regs[0x3b]+0x0, _tmp); // isp_ltm.reg_brit_lce_en

    _tmp = (PTN_REG_READ(phys_regs[0x3b]+0x8)&0xffffe000)|0x51b;
    PTN_REG_WRITE(phys_regs[0x3b]+0x8, _tmp); // isp_ltm.reg_frame_widthm1

    _tmp = (PTN_REG_READ(phys_regs[0x3b]+0x8)&0xe000ffff)|0x2df0000;
    PTN_REG_WRITE(phys_regs[0x3b]+0x8, _tmp); // isp_ltm.reg_frame_heightm1

    _tmp = (PTN_REG_READ(phys_regs[0x3b]+0x0)&0xffffefff)|0x1000;
    PTN_REG_WRITE(phys_regs[0x3b]+0x0, _tmp); // isp_ltm.reg_dbg_enable

    _tmp = (PTN_REG_READ(phys_regs[0x3b]+0x0)&0xffff1fff)|0x8000;
    PTN_REG_WRITE(phys_regs[0x3b]+0x0, _tmp); // isp_ltm.reg_dbg_mode
#endif

} while (0);
