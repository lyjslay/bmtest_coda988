#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>

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

#include "mmio.h"

#include "vip_common.h"
#include "inc/isp_reg.h"
#include "vip/vi_drv.h"
#include "vi_defines.h"

#include <reg_common.h>

#ifdef BUILD_VIP
#include "../vpss/scaler.h"
#endif
#ifdef TEST_CIF
#include "../vip/pld_test_cif.h"
#endif
#include "test_rtl_reg.h"

struct cvi_vi_dev g_dev = {
        .reg_base = (void *)0x0a000000,
        .irq_num = 9,
        .total_dev_num = 1,
};

extern int vi_ip_test_case;
extern int vi_bayer_id;
extern int vi_hdr;
extern int vi_max_size;
extern int vi_dpcm;
extern int vi_3dnr;
extern int vi_fbc;
extern int vi_fly_mode;
extern int vi_slice_on;
extern int vi_scaler;
extern int vi_scaler_scenery;
extern int vi_gamma_mem_sel;
extern int vi_sensor_input;
extern int vi_crop_en;

#define _CBSIZE         (32)

#ifdef ENV_HOSTPC
// EMU won't really use image-fb
#else
// the buf-size is capable of 3840x2160 rgb planar
// the 1st set is for 4096*4096
#endif

#if 0
static bool enable_debug = false;
static bool enable_cmdq = false;
#endif

/********************************************************************
 *   APIs to replace bmtest's standard APIs
 ********************************************************************/
extern int cli_readline(char *prompt, char *cmdbuf);
int rtl_setreg(int argc, char **argv);

#undef uartlog
#define uartlog(fmt, args...) \
{ \
    printf(fmt, ##args); \
    fflush(stdout); \
}

static inline int _natoi(char *str, int n)
{
    int i, val = 0;

    if (str == NULL)
        return 0;

    for (i = 0; (i < n) && (*str); ++i, ++str) {
        if (((*str) < '0') || ((*str) > '9'))
            break;

        val = val * 10 + ((*str) - '0');
    }

    return val;
}

int isp_intr_handler(int irqn, void *priv)
{
    struct cvi_vi_dev *vdev = (struct cvi_vi_dev *)priv;

    //uartlog("%s: irq(%d)\n", __func__, irqn);
    vi_irq_handler(vdev);

    return 0;
}

int do_nothing(int argc, char **argv)
{
    uartlog("%s argc: %d\n", __func__, argc);

    return 0;
}

#ifdef BUILD_VIP
int check_vpss_state(int argc, char **argv)
{
    uartlog("%s argc: %d\n", __func__, argc);
    vpss_online_check_irq_handler(5);
    return 0;
}
#endif

int dump_debug_info(int argc, char **argv)
{
    uartlog("%s argc: %d\n", __func__, argc);

    _vi_debug_info(&g_dev.ctx);
    return 0;
}

int trig_pre(int argc, char **argv)
{
    uartlog("%s argc: %d\n", __func__, argc);

    _pre_hw_enque(&g_dev, ISP_PRERAW0, ISP_FE_CH0);
    if (g_dev.ctx.isp_pipe_cfg[ISP_PRERAW0].is_hdr_on)
         _pre_hw_enque(&g_dev, ISP_PRERAW0, ISP_FE_CH1);
    return 0;
}

int rtl_setreg(int argc, char **argv)
{
    int i = 0;
    vi_pr(VI_INFO, "vi_ip_reg_config according to test_reg_set.h \n");

    while(i < REG_WRITE_TIME){
        int j = i;
        uintptr_t addr = test_reg_addr[j];
        u32 val = readl(addr);
        while (1)
        {
            // vi_pr(VI_INFO, "(mask(%x) & (%x)) | (~mask(%x) & %x)=%x \n", test_reg_mask[j], val,  ~test_reg_mask[j], val , ((test_reg_mask[j] & val) | (~test_reg_mask[j] & test_reg_val[j])));
            val = ((test_reg_mask[j] & val) | ( ~test_reg_mask[j] & test_reg_val[j]));
            if (test_reg_addr[j+1] != addr)
                break;
            j++;
        }
        i = j + 1;
        if (i%1000 == 0)
            vi_pr(VI_INFO, "setting [%d/%d]addr = 0x%x, reg =0x%x \n", i, REG_WRITE_TIME, test_reg_addr[j], val);
        writel(addr, val);
    }
    return 0;
}

int trig_post(int argc, char **argv)
{
    uartlog("%s argc: %d\n", __func__, argc);

    // isp_post_trig(&g_dev.ctx, ISP_PRERAW0);
    _post_hw_enque(&g_dev);

    return 0;
}

int vi_ip_test(int argc, char **argv)
{
    if (argc >= 1) {
        vi_ip_test_case = strtol(argv[1], NULL, 10);
    } else {
        uartlog("%s argc: %d\n", __func__, argc);
    }
    return 0;
}

int config_ip_test(int argc, char **argv)
{
    uartlog("%s argc: %d\n", __func__, argc);
    if (argc >= 1) {
        vi_ip_test_case = strtol(argv[1], NULL, 10);
        vi_ip_test_cases_init(&g_dev.ctx);
    }
    return 0;
}

int config_bayerid(int argc, char **argv)
{
    uartlog("%s argc: %d\n", __func__, argc);
    if (argc > 1) {
        vi_bayer_id = strtol(argv[1], NULL, 10);
        uartlog("config_bayerid %d\n", vi_bayer_id);
    }
    return 0;
}

int config_hdr(int argc, char **argv)
{
    uartlog("%s argc: %d\n", __func__, argc);

    vi_hdr = strtol(argv[1], NULL, 10);
    uartlog("vi_hdr %d\n", vi_hdr);

    return 0;
}

int config_crop(int argc, char **argv)
{
    uartlog("%s argc: %d\n", __func__, argc);

    vi_crop_en = strtol(argv[1], NULL, 10);
    uartlog("vi_hdr %d\n", vi_crop_en);

    return 0;
}

int config_slice(int argc, char **argv)
{
    uartlog("%s argc: %d\n", __func__, argc);
    if (argc >= 1) {
        vi_slice_on = strtol(argv[1], NULL, 10);
        uartlog("vi_slice_on %d\n", vi_slice_on);
    }
    return 0;
}

int config_devs(int argc, char **argv)
{
    uartlog("%s argc: %d\n", __func__, argc);
    if (argc >= 1) {
        g_dev.total_dev_num = strtol(argv[1], NULL, 10);
        uartlog("dev_num %d\n", g_dev.total_dev_num);
    }
    return 0;
}

int config_max(int argc, char **argv)
{
    uartlog("%s argc: %d\n", __func__, argc);
    if (argc >= 1) {
        vi_max_size = strtol(argv[1], NULL, 10);
        uartlog("vi_max_size %d\n", vi_max_size);
    }
    return 0;
}

int config_dpcm(int argc, char **argv)
{
    uartlog("%s argc: %d\n", __func__, argc);
    if (argc >= 1) {
        vi_dpcm = strtol(argv[1], NULL, 10);
        uartlog("vi_dpcm %d\n", vi_dpcm);
    }
    return 0;
}

int config_3dnr(int argc, char **argv)
{
    uartlog("%s argc: %d\n", __func__, argc);
    if (argc >= 1) {
        vi_3dnr = strtol(argv[1], NULL, 10);
        uartlog("vi_3dnr %d\n", vi_3dnr);
    }
    return 0;
}

//该func暂不使用
int config_sensor_input(int argc, char **argv)
{
    uartlog("%s argc: %d\n", __func__, argc);
    if (argc >= 1) {
        vi_sensor_input = strtol(argv[1], NULL, 10);
        uartlog("vi_sensor_input %d\n", vi_sensor_input);
    }
    return 0;
}

int config_fbc(int argc, char **argv)
{
    uartlog("%s argc: %d\n", __func__, argc);
    if (argc >= 1) {
        vi_fbc = strtol(argv[1], NULL, 10);
        uartlog("vi_fbc %d\n", vi_fbc);
    }
    return 0;
}

int config_online(int argc, char **argv)
{
    uartlog("%s argc: %d\n", __func__, argc);
    if (argc >= 1) {
        vi_fly_mode = strtol(argv[1], NULL, 10);
        uartlog("vi_fly_mode %d\n", vi_fly_mode);
    }
    return 0;
}

int config_gamma_mem_sel(int argc, char **argv)
{
    uartlog("%s argc: %d\n", __func__, argc);
    if (argc > 1) {
        vi_gamma_mem_sel = strtol(argv[1], NULL, 10);
        if (vi_gamma_mem_sel>=1)
            vi_gamma_mem_sel = 1;
        else
            vi_gamma_mem_sel = 0;
        uartlog("vi_gamma_mem_sel %d\n", vi_gamma_mem_sel);
    }
    return 0;
}


#ifdef BUILD_VIP
int config_scaler(int argc, char **argv)
{
    uartlog("%s argc: %d\n", __func__, argc);
    if (argc > 1) {
        vi_scaler = strtol(argv[1], NULL, 10);
        uartlog("vi_scaler %d\n", vi_scaler);
    }
    return 0;
}

int config_scaler_scenery(int argc, char **argv)
{
    uartlog("%s argc: %d\n", __func__, argc);
    vi_pr(VI_INFO, "USAGE: set val 1: ");
    if (argc > 1) {
        vi_scaler_scenery = strtol(argv[1], NULL, 10);
        uartlog("vi_scaler_scenery = %d\n", vi_scaler_scenery);
        if (vi_scaler_scenery==1){
            vi_pr(VI_INFO, "     |->vpss0\n");
            vi_pr(VI_INFO, "isp->|->vpss1\n");
            vi_pr(VI_INFO, "     |->vpss2\n");
            vi_pr(VI_INFO, "     |->vpss4\n");
        }
    }
    return 0;
}
#endif

// #define MEM_BASE_OFFSET 0x800000
// int config_csibdg_0_raw12(uint32_t width,uint32_t height,uint32_t start_addr,uint32_t stride)
// {

// 	uint64_t num=height;
// 	uint64_t len=3 * UPPER(width,1);
// 	len=VI_ALIGN(len);
// 	stride=VI_ALIGN(stride);

// 	//pre raw configuration
// 	uartlog("*******************pre raw config*******************\n");
// 	writel(0x0a000000,  readl(0x0a000000)|0x33);  //PRE_RAW_FE_CTRL
// 	uartlog("pre_raw_fe REG_00=value(%x)\n",readl(0x0a000000));
// 	writel(0x0a000004, (height-1) << 16 | (width-1));
// 	uartlog("pre_raw_fe REG_04=value(%x)\n",readl(0x0a000004));

// 	// csi bdg config
// 	uartlog("*******************csi bdg config*******************\n");
// 	writel(0x0a000800,0x11108041);
// 	uartlog("reg_isp_csi_bdg REG_00=value(0x%x)\n",readl(0x0a000800));
// 	writel(0x0a000804,0xF000000D);
// 	uartlog("reg_isp_csi_bdg REG_04=value(0x%x)\n",readl(0x0a000804));
// 	writel(0x0a000808,readl(0x0a000808)|0x0);
// 	uartlog("reg_isp_csi_bdg REG_08=value(0x%x)\n",readl(0x0a000808));
// 	// csibdg size
// 	writel(0x0a000810, (height-1) << 16 | (width-1));
// 	uartlog("reg_isp_csi_bdg REG_10==value(0x%x)\n",readl(0x0a000810));

// 	uartlog("*******************config isp_dma_ctl addr*******************\n");
// 	writel(0x0a000b00,0x000F0100);    //high 32 10000/
// 	uartlog("reg_isp_dma_ctl REG_00=value(0x%x)\n",readl(0x0a000b00));
// 	writel(0x0a000b04,start_addr);  //low 32 0x8  0x1008000
// 	uartlog("reg_isp_dma_ctl REG_04==value(0x%x)\n",readl(0x0a000b04));
// 	writel(0x0a000b08,len);
// 	uartlog("reg_isp_dma_ctl REG_08=value(0x%x)\n",readl(0x0a000b08));
// 	writel(0x0a000b0c,stride);
// 	uartlog("reg_isp_dma_ctl REG_0c=value(0x%x)\n",readl(0x0a000b0c));
// 	writel(0x0a000b10,num);
// 	uartlog("reg_isp_dma_ctl REG_10=value(0x%x)\n",readl(0x0a000b10));

// 	uartlog("config csi_bdg_0 done\n");
// 	return 0;
// }

#ifdef TEST_CIF
// extern int test_spliter();
extern int set_phy_1c2d();
extern int verif_sensor();

int trig_sensor(int argc, char **argv)
{
    if (vi_ai_isp_input_mac==0){
        uartlog("%s argc: %d start MIPI 1C4d MAC%d\n", __func__, argc, vi_ai_isp_input_mac);
        _CIF_SET_MIPI(0, 0x3);
    }
    else if(vi_ai_isp_input_mac==3) {
        uartlog("%s argc: %d start MIPI 1C4d MAC%d\n", __func__, argc, vi_ai_isp_input_mac);
        _CIF_SET_MIPI(3, 0x3);
    }
    else {
        uartlog("%s : can't start MIPI 1C4d at MAC%d\n", __func__, vi_ai_isp_input_mac);
        return 0;
    }

    uartlog("trig_pre now...\n");
    trig_pre(2, NULL);

    uartlog("set spliter valid now...\n");
    uintptr_t line_spliter = g_dev.ctx.phys_regs[ISP_BLK_ID_LINE_SPLITER];
    union REG_LINE_SPLITER_FRAME_VLD_CTRL frame_vld_ctrl;
    if (vi_ai_isp_input_mac==0) {
        frame_vld_ctrl.bits.SPLITER_FRAME_VLD_FE0 = 1;
	    frame_vld_ctrl.bits.SPLITER_FRAME_VLD_FE1 = g_dev.ctx.is_tile ? 1 : 0;
    }
    else if (vi_ai_isp_input_mac==3) {
        frame_vld_ctrl.bits.SPLITER_FRAME_VLD_FE1 = 1;
    }
    ISP_WR_REG(line_spliter, REG_ISP_LINE_SPLITER_T, FRAME_VLD_CTRL, frame_vld_ctrl.raw);
    return 0;
}
#endif

int config_vi_sel(int argc, char **argv)
{
    struct cvi_vi_dev *vdev = &g_dev;
    struct isp_ctx *ctx = &vdev->ctx;
    memset(ctx, 0, sizeof(*ctx));

    u32 width = vdev->usr_fmt.width;
    u32 height = vdev->usr_fmt.height;
    u32 dmaid_le = ISP_BLK_ID_DMA_CTL_PRE_RAW_VI_SEL_LE;
    u32 dmaid_se = ISP_BLK_ID_DMA_CTL_PRE_RAW_VI_SEL_SE;

    if (!width || !height) {
        width = 128;
        height = 64;
    }
    vi_pr(VI_INFO, "config_vi_sel w_%d h_%d\n", width, height);

    ctx->is_hdr_on = vi_hdr;
    ctx->isp_pipe_cfg[ISP_PRERAW0].is_hdr_on = vi_hdr;
    ctx->is_dpcm_on = vi_dpcm;
    ctx->is_3dnr_on = vi_3dnr;
    ctx->is_fbc_on = vi_fbc;
    ctx->is_rtl_config = 1;
    vdev->total_dev_num = 1;
    ctx->isp_pipe_cfg[ISP_PRERAW0].is_raw_replay = true;
    ctx->isp_pipe_cfg[ISP_PRERAW0].is_offline_scaler = true;
    ctx->is_offline_postraw = true;

    ctx->isp_pipe_cfg[ISP_PRERAW0].crop.w = width;
    ctx->isp_pipe_cfg[ISP_PRERAW0].crop.h = height;
    ctx->isp_pipe_cfg[ISP_PRERAW0].crop_se.w = width;
    ctx->isp_pipe_cfg[ISP_PRERAW0].crop_se.h = height;
    ctx->isp_pipe_cfg[ISP_PRERAW0].csibdg_width  = width;
    ctx->isp_pipe_cfg[ISP_PRERAW0].csibdg_height = height;
    ctx->img_width = width;
    ctx->img_height = height;

    vdev->ctx.phys_regs	= isp_get_phys_reg_bases();
    vdev->mem_base = DRAM_PHY_BASE;

    config_lut_table(&vdev->ctx); //若无需lut，为了速度可以临时注释

    // maybe used user_space ion buffer
    u64 size = ctx->isp_pipe_cfg[ISP_PRERAW0].csibdg_width * ctx->isp_pipe_cfg[ISP_PRERAW0].csibdg_height * 3 / 2;
    u64 addr = (u64)PTN_MALLOC(size);
    ispblk_dma_setaddr(&vdev->ctx, dmaid_le, addr);
    vi_pr(VI_DBG, "dma_le addr=0x%lx size:%ld\n", addr, size);
	vi_pr(VI_INFO, "le fake addr:-start 0x%lx -end 0x%lx\n", (addr/2)&0xfffffff, ((addr + size)/2-1)&0xfffffff);
    if (vi_hdr) {
        addr = (u64)PTN_MALLOC(size);
        vi_pr(VI_DBG, "dma_se addr=0x%lx\n", addr);
        ispblk_dma_setaddr(&vdev->ctx, dmaid_se, addr);
        vi_pr(VI_DBG, "se fake addr:-start 0x%lx -end 0x%lx\n", (addr/2)&0xfffffff, ((addr + size)/2-1)&0xfffffff);
    }

    // rtl_setreg(2, NULL);

    // vi_pr(VI_INFO, "==============for tnr setting...==================\n");
    // writel(0x0a030018, 0x3f007f);
    // writel(0x0a030038, 0x3f007f);
    // writel(0x0a069104, 0x03000000);
    // writel(0x0a06a104, 0x03002000);
    // writel(0x0a019808, 0x0);
    // writel(0x0a01a008, 0x0);
    // writel(0x0a040000, 0x1);
    // writel(0x0a041000, 0x1);
    // writel(0x0a05306c, 0xa000);

    // writel(0x0a000010, 0x30083010);
    // writel(0x0a000014, 0x30083010);
    // writel(0x0a002100, 0x1);
    // writel(0x0a002108, 0x0);

    // writel(0x0a002400, 0x1);
    // writel(0x0a002408, 0x0);
    // writel(0x0a001000, 0x1);
    // writel(0x0a001008, 0x0);
    // writel(0x0a001800, 0x1);
    // writel(0x0a001808, 0x0);

    uartlog("  raw_replay:%d    \n", ctx->isp_pipe_cfg[ISP_PRERAW0].is_raw_replay );

    return 0;
}

int start_vi(int argc, char **argv)
{
    uartlog("%s argc: %d\n", __func__, argc);

    vi_create_instance(&g_dev);

    return 0;
}

int dump_register(int argc, char **argv)
{
    uartlog("%s argc: %d\n", __func__, argc);
    int raw_num;
    if (argc > 1) {
        raw_num = strtol(argv[1], NULL, 10);
        uartlog("raw_num %d\n", raw_num);
        if (raw_num >= 0 && raw_num < 6)
            dump_pass_register(&g_dev.ctx, raw_num);
    }

    return 0;
}

int stop_vi(int argc, char **argv)
{
    uartlog("%s argc: %d\n", __func__, argc);

    vi_destroy_instance(&g_dev);

    return 0;
}

int load_pattern(int argc, char **argv)
{

    //int i, len;
    int ret = 0;
    //char cmdbuf[_CBSIZE];
    // uint64_t mem_base = DRAM_PHY_BASE;

    // #if 1
    // uartlog("using malloc heap memory\n");
    // mem_base = (uint64_t)malloc(1024*1024);
    // #endif

    // uartlog("dram base %lx\n", mem_base);

    // len = cli_readline("#", cmdbuf);
    // if (len <= 0) {
    //     uartlog("error [%d]: invalid input %d\n", __LINE__, len);
    //     goto _EXIT_MALLOC;
    // }

    // i = _natoi(cmdbuf, _CBSIZE);
    // pctx = create_pattern_ctx(i);
    // if (pctx == NULL) {
    //     goto _EXIT_MALLOC;
    // }
   // uartlog("testcase: %d. %s\n", i, pctx->ptn->name);

    //pctx->phys_regs = isp_get_phys_reg_bases();
    // hw's start phy-addr need to be 32byte-align
    //pctx->mem_base = (mem_base + 31) & ~31;

    vi_create_instance(&g_dev);
// _EXIT_MALLOC:
//     uartlog("free: %lx\n", mem_base);
//     if (mem_base) {
//         free((void *)mem_base);
//         pctx->mem_base = 0;
//     }

    return ret;
}

static struct cmd_entry test_cmd_list[] __attribute__ ((unused)) = {
	{"\n[ISP ========================== ]", do_nothing, 0, ""},
	{"lp", load_pattern, 0, "\t[Load ISP pattern with input test case #]"},
	{"debug", dump_debug_info, 0, "\t[test cmd] dump debug register"},
    {"trig_pre", trig_pre, 0, "\t[trig preraw only FE0]"},
    {"trig_post", trig_post, 0, "\t[trig postraw]"},
    {"vi_ip_test", vi_ip_test, 1, "\t[vi_ip_test [test_no]]"},
    {"stop_vi", stop_vi, 0, "\t[stop_vi] blocked unuse"},
    {"start_vi", start_vi, 0, "\t[start_vi]"},
    {"dump_register", dump_register, 1, "\t[dump_register] dump csibdg/isptop/berdma register"},
    {"config_ip", config_ip_test, 1, "\t[config_ip_test [test_no]] run after start vi, postraw done"},
    {"config_bayerid", config_bayerid, 1, "\t[config_bayerid [bayerid]] default BG for pattern, rawreply use 2, run before start_vi/lp"},
    {"config_hdr", config_hdr, 1, "\t[config_hdr [is_hdr]] run before start_vi/lp"},
    {"config_crop", config_crop, 1, "\t[config_hdr [is_hdr]] run before start_vi/lp"},
    {"config_devs", config_devs, 1, "\t[config_devs [devs_num]] run before start_vi/lp"},
    {"config_max", config_max, 1, "\t[config_max [is_max]] run before start_vi/lp"},
    {"config_dpcm", config_dpcm, 1, "\t[config_dpcm [is_dpcm]] run before start_vi/lp"},
    {"config_3dnr", config_3dnr, 1, "\t[config_3dnr [is_3dnr] run before start_vi/lp"},
    {"config_sensor_input", config_sensor_input, 1, "\t[config_sensor_input 1 run before start_vi/lp"},
    {"config_fbc", config_fbc, 1, "\t[config_fbc [is_fbc]] run before start_vi/lp"},
    {"config_online", config_online, 1, "\t[config_online [is_fe_be_online]] run before start_vi/lp"},
    {"config_slice", config_slice, 1, "\t[config_slice [is_slice]] run before start_vi/lp"},
    {"config_gamma", config_gamma_mem_sel, 1, "\t[config_gamma [gamma_mem_sel]]"},
#ifdef BUILD_VIP
    {"config_scaler", config_scaler, 1, "\t[config_scaler [is_scaler]] run before start_vi/lp"},
    {"config_scaler_scenery", config_scaler_scenery, 1, "\t[config_scaler_scenery [vpss_scenery]] run before config_scaler"},
    {"check_vpss_state", check_vpss_state, 0, "\t[check_vpss_state use isp online to vpss]"},
#endif
#ifdef TEST_CIF
    {"trig_sensor", trig_sensor, 0, "\t[trig_sensor start...]"},
    // {"set_phy_1c2d", set_phy_1c2d, 0, ""},
    // {"verif_sensor",verif_sensor,0,""},
#endif
    {"config_vi_sel", config_vi_sel, 2, "\t[config_vi_sel [width] [height]]"},
    {"rtl_setreg", rtl_setreg, 0, "\t[config register according to <RTL cfg test_reg_set.h>]"},
	{"\n[CMDQ TBD ===================== ]", do_nothing, 0, ""},
	{NULL, NULL, 0, NULL}
};

int testcase_isp(void)
{
    unsigned ret = 0;
    unsigned i = 0;

    uartlog("starting isp test case.\n");

    vi_set_base_addr(g_dev.reg_base);

    uartlog("request isp irq : %d\n", ISP_INT);

    // writel(0x0a0be010, 0x0);
    // uartlog("axi_sw : 0x%x\n", readl(0x0a0be010));

    //reset reg(vi sys)
    // writel(0x0a0be004, 0x2);
    // writel(0x0a0be004, 0x0);
    // writel(0x0a0be000, 0x2);
    // writel(0x0a0be000, 0x0);

    request_irq(ISP_INT, isp_intr_handler, 0, "isp_intr", &g_dev);

    // writel(0x0a0be000, 0x0);

    // uartlog("test read reg before:0x%x\n", readl(0x0a070028));

    // writel(0x0a070028, 0x0000000f);
    // uartlog("read reg 0x%x\n", readl(0x0a070028));

    uartlog("start to add isp command\n");

// #ifdef TEST_C906
//     extern void bringup_c906b(unsigned long addr);
//     bringup_c906b(0x108040000); //adjust addr before test
//     uartlog("bring_up c906b now, test_irq...");
// #elif TEST_TC906_0_IRQ
//     extern void bringup_tc906b(int coreID, unsigned long addr);
//     bringup_tc906b(0, 0x108040000); //adjust addr please test
//     uartlog("bring_up tc906b [0] now, test_irq...");
// #elif TEST_TC906_1_IRQ
//     extern void bringup_tc906b(int coreID, unsigned long addr);
//     bringup_tc906b(1, 0x108040000); //adjust addr please test
//     uartlog("bring_up tc906b [1] now, test_irq...");
// #endif
    //vip_isp_clk_reset();
    for(i = 0;i < ARRAY_SIZE(test_cmd_list) - 1;i++) {
        command_add(&test_cmd_list[i]);
    }

    // add reg cfg cmd
    command_add(&ini_frame_0_cmd);
    command_add(&ini_frame_1_cmd);
    command_add(&ini_frame_2_cmd);

    uartlog("add isp command done\n");

#ifndef VIP_INC_ISP
    do {
        ret = cli_simple_loop();
    } while (0);
#endif

    return ret;
}

#ifndef VIP_INC_ISP
#ifndef BUILD_TEST_CASE_all
#ifdef ENV_HOSTPC
int main(int argc, char *argv[])
#else
int testcase_main(void)
#endif
{
    int ret;

    ret = testcase_isp();

    uartlog("testcase isp -- %s\n", (ret < 0) ? "failed" : "passed");

    return ret;
}
#endif	// BUILD_TEST_CASE_all
#endif	// VIP_INC_ISP

#ifdef ENV_HOSTPC
#else
module_testcase("1", testcase_isp);
#endif
