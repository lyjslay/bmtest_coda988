#ifdef ENV_CVITEST
#include <common.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdlib.h>
#include "system_common.h"
#elif defined(ENV_EMU)
#include <stdarg.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "emu/command.h"
#else
#include <asm/io.h>
#endif  // ENV_CVITEST

#include "vip_common.h"
#include "reg.h"

void bm_scanf(const char * fmt, ...)
{
    char buf[32];
    va_list arg;

    va_start (arg, fmt);
    cli_readline(NULL, buf);
    sscanf(buf, fmt, va_arg(arg, char*));
    va_end(arg);
}

void bm_uartlog(const char * fmt, ...)
{
    va_list arg;

    va_start(arg, fmt);
    vfprintf(stdout, fmt, arg);
    fflush(stdout);
    va_end(arg);
}

int bm_getc(void)
{
    char buf[8];
    cli_readline(NULL, buf);
    return buf[0];
}

void *malloc_aligned(size_t size, u8 alignment, u32** ptr_org)
{
    *ptr_org = malloc(size + alignment-1);
    if (!*ptr_org) return NULL;
    return (void *)(VIP_ALIGN((intptr_t)*ptr_org));
}

// op
// 0: different number of reg-write entries only
// 1: different number of reg-write entries with sw-restart
// 2: different number of reg-write entries with timer
u8 generate_cmdset(union cmdq_set *set, u32 addr, u8 num, u32 pattern, u8 op, bool is_end)
{
    u8 cmd_idx = 0;
    for (u8 i = 0; i < num; ++i) {
        cmdQ_set_package(&set[cmd_idx++].reg, addr + i*4, pattern);

        if ((op == 1) && (cmd_idx == num/2)) {
            u32 flag_num = 0;   // 0: sc_str_flag, sc_stp_flag
            cmdQ_set_wait(&set[cmd_idx++], /*is_timer*/false, flag_num, 0b010);
        }
        else if ((op == 2) && (cmd_idx == num/2))
            cmdQ_set_wait(&set[cmd_idx++], /*is_timer*/true, 4000, 0b000);
    }

    set[cmd_idx-1].reg.intr_int = 1;
    set[cmd_idx-1].reg.intr_end = (is_end) ? 1 : 0;
    set[cmd_idx-1].reg.intr_last = 1;

    return cmd_idx;
}

int reg_compare(u32 addr, u8 count, u32 pattern)
{
    int mismatch = 0;
    u32 tmp = 0;
    for (int cmd_idx = 0; cmd_idx < count; ++cmd_idx) {
        if ((tmp = _reg_read(addr + cmd_idx*4)) != pattern) {
            ++mismatch;
            uartlog("item-%d: reg(0x%x) value(0x%x) mismatch, pattern(0x%x)\n",
            	cmd_idx, addr + cmd_idx*4, tmp, pattern);
        }
    }

    return mismatch;
}

