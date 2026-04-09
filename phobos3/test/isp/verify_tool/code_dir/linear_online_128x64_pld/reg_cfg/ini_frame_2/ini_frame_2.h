#ifndef _TEST_INI_FRAME_2_REG_SET_H_
#define _TEST_INI_FRAME_2_REG_SET_H_

#define REG_INI_FRAME_2_WRITE_TIME 68

#include "command.h"
u64 ini_frame_2_reg_addr[68] = {0x0a0700a0, \
                         0x0a0700a0, \
                         0x0a040400, \
                         0x0a040400, \
                         0x0a040400, \
                         0x0a040400, \
                         0x0a040404, \
                         0x0a040408, \
                         0x0a04040c, \
                         0x0a040410, \
                         0x0a040500, \
                         0x0a040500, \
                         0x0a040500, \
                         0x0a040500, \
                         0x0a040504, \
                         0x0a040508, \
                         0x0a04050c, \
                         0x0a040510, \
                         0x0a06b080, \
                         0x0a06b080, \
                         0x0a06b084, \
                         0x0a06b100, \
                         0x0a06b100, \
                         0x0a06b100, \
                         0x0a06b100, \
                         0x0a06b100, \
                         0x0a06b104, \
                         0x0a06b120, \
                         0x0a06b120, \
                         0x0a06b120, \
                         0x0a06b120, \
                         0x0a06b120, \
                         0x0a06b124, \
                         0x0a06b128, \
                         0x0a06b12c, \
                         0x0a06b130, \
                         0x0a065210, \
                         0x0a06520c, \
                         0x0a065208, \
                         0x0a065204, \
                         0x0a065200, \
                         0x0a065200, \
                         0x0a065200, \
                         0x0a065200, \
                         0x0a065310, \
                         0x0a06530c, \
                         0x0a065308, \
                         0x0a065304, \
                         0x0a065300, \
                         0x0a065300, \
                         0x0a065300, \
                         0x0a065300, \
                         0x0a065410, \
                         0x0a06540c, \
                         0x0a065408, \
                         0x0a065404, \
                         0x0a065400, \
                         0x0a065400, \
                         0x0a065400, \
                         0x0a065400, \
                         0x0a065510, \
                         0x0a06550c, \
                         0x0a065508, \
                         0x0a065504, \
                         0x0a065500, \
                         0x0a065500, \
                         0x0a065500, \
                         0x0a065500};

u64 ini_frame_2_reg_mask[68] = {0xffffff0f, \
                         0xfffffff0, \
                         0xffefffff, \
                         0xffdfffff, \
                         0xffbfffff, \
                         0xff7fffff, \
                         0x00000000, \
                         0xf0000000, \
                         0xf0000000, \
                         0xffffc000, \
                         0xffefffff, \
                         0xffdfffff, \
                         0xffbfffff, \
                         0xff7fffff, \
                         0x00000000, \
                         0xf0000000, \
                         0xf0000000, \
                         0xffffc000, \
                         0xffefffff, \
                         0xfff0000f, \
                         0x00000000, \
                         0xffefffff, \
                         0xffdfffff, \
                         0xffbfffff, \
                         0xff7fffff, \
                         0xfff0000f, \
                         0x00000000, \
                         0xffefffff, \
                         0xffdfffff, \
                         0xffbfffff, \
                         0xff7fffff, \
                         0xfff0000f, \
                         0x00000000, \
                         0xf0000000, \
                         0xf0000000, \
                         0xffffc000, \
                         0xffffc000, \
                         0xf0000000, \
                         0xf0000000, \
                         0x00000000, \
                         0xffbfffff, \
                         0xffdfffff, \
                         0xff7fffff, \
                         0xffefffff, \
                         0xffffc000, \
                         0xf0000000, \
                         0xf0000000, \
                         0x00000000, \
                         0xffbfffff, \
                         0xffdfffff, \
                         0xff7fffff, \
                         0xffefffff, \
                         0xffffc000, \
                         0xf0000000, \
                         0xf0000000, \
                         0x00000000, \
                         0xffbfffff, \
                         0xffdfffff, \
                         0xff7fffff, \
                         0xffefffff, \
                         0xffffc000, \
                         0xf0000000, \
                         0xf0000000, \
                         0x00000000, \
                         0xffbfffff, \
                         0xffdfffff, \
                         0xff7fffff, \
                         0xffefffff};

u64 ini_frame_2_reg_val[68] = {0x00000000, \
                         0x00000000, \
                         0x00100000, \
                         0x00000000, \
                         0x00000000, \
                         0x00000000, \
                         0x01000000, \
                         0x00000100, \
                         0x00000100, \
                         0x00000020, \
                         0x00100000, \
                         0x00000000, \
                         0x00000000, \
                         0x00000000, \
                         0x02000000, \
                         0x00000100, \
                         0x00000100, \
                         0x00000020, \
                         0x00100000, \
                         0x00000030, \
                         0x31000000, \
                         0x00100000, \
                         0x00000000, \
                         0x00000000, \
                         0x00000000, \
                         0x00000030, \
                         0x32000000, \
                         0x00100000, \
                         0x00200000, \
                         0x00400000, \
                         0x00800000, \
                         0x00000070, \
                         0xf2000000, \
                         0x00000400, \
                         0x00000400, \
                         0x00000001, \
                         0x00000001, \
                         0x00004000, \
                         0x00004000, \
                         0xa0000000, \
                         0x00000000, \
                         0x00000000, \
                         0x00000000, \
                         0x00100000, \
                         0x00000001, \
                         0x00008000, \
                         0x00008000, \
                         0xb0000000, \
                         0x00000000, \
                         0x00000000, \
                         0x00000000, \
                         0x00100000, \
                         0x00000001, \
                         0x00004000, \
                         0x00004000, \
                         0xc0000000, \
                         0x00000000, \
                         0x00000000, \
                         0x00000000, \
                         0x00100000, \
                         0x00000001, \
                         0x00008000, \
                         0x00008000, \
                         0xd0000000, \
                         0x00000000, \
                         0x00000000, \
                         0x00000000, \
                         0x00100000};

static int rtl_ini_frame_2_setreg(int argc, char **argv) {
    int i = 0;
    while(i < REG_INI_FRAME_2_WRITE_TIME) {
        int j = i;
        uintptr_t addr = ini_frame_2_reg_addr[j];
        u32 val = readl(addr);
        while (1) {
            val = ((ini_frame_2_reg_mask[j] & val) | ( ~ini_frame_2_reg_mask[j] & ini_frame_2_reg_val[j]));
            if (ini_frame_2_reg_val[j+1] != addr) break;
            j++;
        }
        i = j + 1;
        writel(addr, val);
    };
    printf("set reg num: %d", i);
    return 0;
}
static struct cmd_entry ini_frame_2_cmd __attribute__ ((unused)) = 
    {"ini_frame_2", rtl_ini_frame_2_setreg, 0, "test"};
#endif
