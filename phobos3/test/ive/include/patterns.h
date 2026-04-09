#ifndef _PATTERNS_H_
#define _PATTERNS_H_

#include <stdint.h>

//#include "ive_drv.h"
#define _reg_read(addr) readl((uintptr_t)addr)
#define _reg_write(addr, data) \
    { \
	writel((uintptr_t)addr, data); \
	if (0) \
	    uartlog("MWriteS32 %#x %#x\n", (uint32_t)(addr), (uint32_t)(data)); \
    }


typedef struct {
    int (*config_op)(void *);
    int (*start_op)(void *);
    int (*stop_op)(void *);
    int (*verify_op)(void *);
    int (*intr_op)(void *);

    char        *name;
    void        *priv;
    // point to each op's main function
    int (*main)(int argc, const char** argv ); 
} pattern_entry;

#define IVE_BUF_QUEUE_DEPTH     (4)
#define IVE_PRERAW_INST_NUM     (2)
enum IVECQ_ID_T {
	IVECQ_ID_PRERAW0 = 0,
	IVECQ_ID_PRERAW1,
	IVECQ_ID_POSTRAW,
	IVECQ_ID_MAX
};
typedef struct {
    uint32_t        sof_cnt[IVE_PRERAW_INST_NUM];
    uint32_t        preraw_done_cnt[IVE_PRERAW_INST_NUM];
    uint32_t        postraw_proc_cnt;
    uint64_t        bayer_buf[IVE_BUF_QUEUE_DEPTH];
} ive_runtime;


typedef struct {
    uint64_t        *phys_regs;     ///< [input] index by enum IVE_BLK_ID_T, always phys reg
    uintptr_t       *vreg_bases;    ///< [input] index by enum IVE_BLK_ID_T
    uintptr_t       *vreg_bases_pa; ///< [input] index by enum IVE_BLK_ID_T
    uintptr_t       adma_table[IVECQ_ID_MAX];       ///< [input]
    uintptr_t       adma_table_pa[IVECQ_ID_MAX];    ///< [input]
    uint64_t        mem_base;       ///<
    uint32_t        alloc_size;     ///<
    uint32_t        alloc_cnt;      ///<
    uint64_t        alloc_vec[64];  ///<

    uint32_t        is_offline_preraw	: 1;
    uint32_t        is_offline_postraw	: 1;
    uint32_t        is_offline_scaler	: 1;
    uint32_t        sc_inst		: 1;
    uint32_t        is_stop		: 1;
    uint32_t        is_pre0_frame_done	: 1;
    uint32_t        is_pre1_frame_done	: 1;
    uint32_t        is_post_frame_done	: 1;
    ive_runtime     rc;

    uint64_t        sw_trig_time;

    pattern_entry   *pptn;       ///< [output]
    pattern_entry   ptn;         ///< [output]
} pattern_ctx;


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

static inline int _natoh(char *str, int n)
{
    int i, val = 0;
    char chbase;

    if (str == NULL)
        return 0;

    for (i = 0; (i < n) && (*str); ++i, ++str) {
        if (((*str) >= '0') && ((*str) <= '9'))
            chbase = '0';
        else if (((*str) >= 'a') && ((*str) <= 'f'))
            chbase = 'a' - 10;
        else if (((*str) >= 'A') && ((*str) <= 'F'))
            chbase = 'A' - 10;
        val = val * 16 + (*str) - chbase;
    }

    return val;
}


static inline uint64_t PTN_MALLOC(size_t size, void *priv)
{
#if 1 // bmtest
    pattern_ctx *ctx = (pattern_ctx *)priv;
    uint64_t addr;

    addr = ctx->mem_base + ctx->alloc_size;

    ctx->alloc_size = (ctx->alloc_size + size);

    ctx->alloc_size = ((ctx->alloc_size + 31) & ~31);

    ctx->alloc_vec[ctx->alloc_cnt] = addr;
    ctx->alloc_cnt += 1;

    uartlog("%s: addr:0x%lx size:0x%x total_used:0x%x\n",
        __func__, addr, (unsigned int)size, ctx->alloc_size);
#endif

    return addr;
}

static inline void PTN_FREE(uint64_t addr)
{
}

static inline void PTN_FREE_ALL(void *priv)
{
#if 1 // bmtest
    int i;
    pattern_ctx *ctx = (pattern_ctx *)priv;

    if (ctx->alloc_cnt) {
        for (i = 0; i < (int)ctx->alloc_cnt; i++) {
            PTN_FREE(ctx->alloc_vec[i]);
            ctx->alloc_vec[i] = 0;
        }

        ctx->alloc_size = 0;
        ctx->alloc_cnt = 0;
    }
#endif
}

static inline void PTN_REG_WRITE(uint64_t reg_addr, uint32_t data)
{
    _reg_write(reg_addr, data);
}

static inline uint32_t PTN_REG_READ(uint64_t reg_addr)
{
    return _reg_read(reg_addr);
}




int             list_pattern(void);
void            destroy_pattern_ctx(pattern_ctx *ctx);

void ive_post_m2m(uint64_t bayer_addr, uint64_t y_addr, uint64_t u_addr, uint64_t v_addr);
#ifdef VIP_PERFORMANCE_TEST
enum VIP_STRESS_CB_TYPE {
	VIP_STRESS_CB_TYPE_EOF = 0,
	VIP_STRESS_CB_TYPE_SOF,
	VIP_STRESS_CB_TYPE_CSIBDG_FIFO_OVERFLOW,
	VIP_STRESS_CB_TYPE_CSIBDG_FRAME_DROP,
	VIP_STRESS_CB_TYPE_DIVE_CATCH_FAIL,
	VIP_STRESS_CB_TYPE_MAX,
};
enum VIP_STRESS_MOD_TYPE {
	VIP_STRESS_MOD_SC = 0,
	VIP_STRESS_MOD_DWA,
	VIP_STRESS_MOD_OSD,
	VIP_STRESS_MOD_DIVE,
	VIP_STRESS_MOD_MAX,
};
typedef int (*cb_func)(enum VIP_STRESS_CB_TYPE type);
void vip_stress_config(u8 test_case, u8 _cb_mask, u8 _mod_mask);
int vip_stress_start(void);
int vip_stress_stop(void);
void vip_stress_register_callback(cb_func fp);
#endif

#endif // _PATTERNS_H_
