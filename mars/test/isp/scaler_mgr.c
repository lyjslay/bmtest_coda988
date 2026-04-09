#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stdbool.h>

#ifdef ENV_HOSTPC
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include "cli/command.h"
#include "cli/cli.h"
#else
#include "command.h"
#include "cli.h"
#include "system_common.h"
#include "module_testcase.h"
#include "uart.h"
#include "timer.h"
#endif

#define VIP_ALIGN(x) (((x) + 0x1F) & ~0x1F)   // for 32byte alignment


#include "../vip/scaler.h"


static int sc_init;



static int sclmgr_handle_cfg_op(u8 op, ...)
{
    int ret = 0;
    va_list ap;
    va_start(ap, op);

    switch(op) {
    case 1:
    {
        u8 inst = va_arg(ap, int);
        struct sclr_odma_cfg *cfg = sclr_odma_get_cfg(inst);
        struct sclr_top_cfg *top_cfg = sclr_top_get_cfg();
        struct sclr_mem mem;
        struct sclr_size size;
        u64 addr0, addr1, addr2;

        if (0 == inst && top_cfg->disp_from_sc) {
            uartlog("It's disp_from_sc now, so no config on scl_d's odma.\n");
            break;
        }

        u8 num_args = va_arg(ap, int);
        if (num_args == 0) {
            uartlog("NG : %s %d\n", __func__, __LINE__);
            //bm_get_num("width: ", size.w);
            //bm_get_num("height: ", size.h);
        } else {
            size.w = va_arg(ap, int);
            size.h = va_arg(ap, int);
        }

        if (num_args <= 2) {
            uartlog("NG : %s %d\n", __func__, __LINE__);
            //bm_get_num("dst-mem-idx(0~2): ", dst_idx);
            //uartlog("data will be loaded into following 3 address:0x%lx, 0x%lx, 0x%lx\n", dst_addr_scl[dst_idx][R_IDX], dst_addr_scl[dst_idx][G_IDX], dst_addr_scl[dst_idx][B_IDX]);
        } else if (num_args == 3) {
            uartlog("NG : %s %d\n", __func__, __LINE__);
            //dst_idx = va_arg(ap, int);
        } else if (num_args == 5) {
            addr0 = va_arg(ap, u64);
            addr1 = va_arg(ap, u64);
            addr2 = va_arg(ap, u64);
        }

        mem.width = size.w;
        mem.height = size.h;
        mem.start_x = 0;
        mem.start_y = 0;
        if (num_args == 3) {
            uartlog("NG : %s %d\n", __func__, __LINE__);
        } else {
            mem.addr0 = addr0;
            mem.addr1 = addr1;
            mem.addr2 = addr2;
        }
        if (IS_YUV_FMT(cfg->fmt)) {
            mem.pitch_y = VIP_ALIGN(size.w);
            mem.pitch_c = VIP_ALIGN(size.w/2);
        } else if (IS_PACKED_FMT(cfg->fmt)) {
            mem.pitch_y = VIP_ALIGN(size.w*3);
            mem.pitch_c = 0;
        } else {
            mem.pitch_y = VIP_ALIGN(size.w);
            mem.pitch_c = mem.pitch_y;
        }
        sclr_odma_set_mem(inst, &mem);

        if (0 == inst) {
            struct sclr_disp_cfg *disp_cfg = sclr_disp_get_cfg();
            disp_cfg->fmt = cfg->fmt;
            if (cfg->csc != SCL_CSC_NONE) disp_cfg->in_csc = cfg->csc - SCL_CSC_601_LIMIT_RGB2YUV + 1;
            disp_cfg->mem = mem;
            sclr_disp_set_cfg(disp_cfg);
        }
        break;
    }
    default:
        uartlog("undefined cfg-op(%d)\n", op);
        break;
    }

    va_end(ap);

    return ret;
}


void sclmgr_intr_setup(u8 inst, enum sclr_img_in img_inst)
{
    union sclr_intr mask;
    memset(&mask, 0, sizeof(mask));
    if (img_inst == SCL_IMG_V) mask.b.img_in_v_frame_end = true;
    else mask.b.img_in_d_frame_end = true;
    if (inst == 0) mask.b.scl0_frame_end = true;
    else if (inst == 1) mask.b.scl1_frame_end = true;
    else if (inst == 2) mask.b.scl2_frame_end = true;
    else if (inst == 3) mask.b.scl3_frame_end = true;
    sclr_set_intr_mask(mask);
    mask.raw = 0xffff;
    sclr_intr_ctrl(mask);

    //intr_mask.all = BIT(6 + inst);
    //timer_tick = timer_get_tick();
}

static int irq_handler_scl(int irqn, void *priv)
{
    //u64 now = timer_get_tick();

    union sclr_intr intr_status = sclr_intr_status();
    sclr_intr_clr(intr_status);
    uartlog("%s: status(%#x)\n", __func__, intr_status.raw);

    return 0;
}

void sclmgr_from_isp(u8 inst, uint32_t in_w, uint32_t in_h, uint32_t out_w, uint32_t out_h, u64 addr0, u64 addr1, u64 addr2)
{
    struct sclr_size src, dst;

    if (!sc_init) {
        sc_init = 1;
        sclr_ctrl_init();
        request_irq(SCL_INTR_SCL_NUM, irq_handler_scl, 0, "scl int", NULL);
    }

    if (inst > 1) return;

    src.w = in_w; src.h = in_h;
    dst.w = out_w; dst.h = out_h;

    struct sclr_scale_cfg sc_cfg;
    sc_cfg.src = src;
    sc_cfg.crop.x = sc_cfg.crop.y = 0;
    sc_cfg.crop.w = src.w;
    sc_cfg.crop.h = src.h;
    sc_cfg.dst = dst;
    sclr_ctrl_set_scale(inst, &sc_cfg);

    enum sclr_img_in img_inst;
    u8 num_args = 0;
    if (inst == 0) {
        img_inst = SCL_IMG_D;
    } else {
        img_inst = SCL_IMG_V;
    }

    union sclr_input in;
    in.v = SCL_INPUT_V_ISP;
    sclr_ctrl_set_input(img_inst, in, SCL_FMT_YUV420, SCL_CSC_601_LIMIT_YUV2RGB);

    //num_args = 2;
    //_handle_cfg_op(CFG_OP_IMG_SET_MEM, img_inst, num_args, sc_cfg.src.w, sc_cfg.src.h);
    {
        //struct sclr_img_cfg *cfg = sclr_img_get_cfg(img_inst);
        struct sclr_mem mem;

        memset(&mem, 0, sizeof(mem));
        mem.width = src.w;
        mem.height = src.h;
        sclr_img_set_mem(img_inst, &mem);
    }

    // setup odma
    //_handle_cfg_op(CFG_OP_ODMA_CFG, inst);
    {
        struct sclr_odma_cfg *cfg;

        cfg = sclr_odma_get_cfg(inst);

        cfg->flip = SCL_FLIP_NO;
        cfg->mode = SCL_OUT_DISABLE;
        cfg->fmt = SCL_FMT_RGB_PLANAR;

        sclr_odma_set_cfg(inst, cfg);
        //sclr_ctrl_set_output(inst, cfg->mode, cfg->fmt, NULL);
    }
    num_args = 5;
    sclmgr_handle_cfg_op(1, inst, num_args, sc_cfg.dst.w, sc_cfg.dst.h, addr0, addr1, addr2);

    sclr_top_reg_done();
    sclmgr_intr_setup(inst, img_inst);
    sclr_img_start(img_inst);
}

void sclr_trig_start(int inst)
{
    sclr_img_start(inst);
}
