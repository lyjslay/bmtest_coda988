#ifndef _TEST_INI_FRAME_1_REG_SET_H_
#define _TEST_INI_FRAME_1_REG_SET_H_

#define REG_INI_FRAME_1_WRITE_TIME 82

#include "command.h"
u64 ini_frame_1_reg_addr[82] = {0x0a0700a0, \
                         0x0a0700a0, \
                         0x0a050014, \
                         0x0a050014, \
                         0x0a052000, \
                         0x0a052000, \
                         0x0a052004, \
                         0x0a052004, \
                         0x0a052044, \
                         0x0a052000, \
                         0x0a052008, \
                         0x0a052008, \
                         0x0a052008, \
                         0x0a052008, \
                         0x0a052010, \
                         0x0a052010, \
                         0x0a052010, \
                         0x0a052010, \
                         0x0a052014, \
                         0x0a052014, \
                         0x0a052014, \
                         0x0a052014, \
                         0x0a052018, \
                         0x0a052018, \
                         0x0a052018, \
                         0x0a052018, \
                         0x0a05201c, \
                         0x0a052024, \
                         0x0a052024, \
                         0x0a052024, \
                         0x0a052024, \
                         0x0a052028, \
                         0x0a052028, \
                         0x0a052028, \
                         0x0a052028, \
                         0x0a052030, \
                         0x0a052030, \
                         0x0a052030, \
                         0x0a052030, \
                         0x0a052048, \
                         0x0a052048, \
                         0x0a052048, \
                         0x0a052048, \
                         0x0a05204c, \
                         0x0a052050, \
                         0x0a052050, \
                         0x0a052050, \
                         0x0a052050, \
                         0x0a052000, \
                         0x0a052000, \
                         0x0a052020, \
                         0x0a052020, \
                         0x0a052038, \
                         0x0a052040, \
                         0x0a052034, \
                         0x0a052034, \
                         0x0a052000, \
                         0x0a052000, \
                         0x0a052020, \
                         0x0a052020, \
                         0x0a052038, \
                         0x0a052040, \
                         0x0a052034, \
                         0x0a052034, \
                         0x0a053024, \
                         0x0a054000, \
                         0x0a055014, \
                         0x0a056000, \
                         0x0a057000, \
                         0x0a058000, \
                         0x0a058010, \
                         0x0a058010, \
                         0x0a058014, \
                         0x0a058014, \
                         0x0a058018, \
                         0x0a058018, \
                         0x0a05801c, \
                         0x0a05801c, \
                         0x0a058020, \
                         0x0a058020, \
                         0x0a058024, \
                         0x0a058024};

u64 ini_frame_1_reg_mask[82] = {0xffffff0f, \
                         0xfffffff0, \
                         0xffffc000, \
                         0xc000ffff, \
                         0xfffffffe, \
                         0xfffffffb, \
                         0xffffff00, \
                         0xffff00ff, \
                         0xffffff00, \
                         0xfffffffd, \
                         0xffffff00, \
                         0xffff00ff, \
                         0xff00ffff, \
                         0x00ffffff, \
                         0xffffff00, \
                         0xffff00ff, \
                         0xff00ffff, \
                         0x00ffffff, \
                         0xffffff00, \
                         0xffff00ff, \
                         0xff00ffff, \
                         0x00ffffff, \
                         0xffffff00, \
                         0xffff00ff, \
                         0xff00ffff, \
                         0x00ffffff, \
                         0xffffff00, \
                         0xffffff00, \
                         0xffff00ff, \
                         0xff00ffff, \
                         0x00ffffff, \
                         0xffffff00, \
                         0xffff00ff, \
                         0xff00ffff, \
                         0x00ffffff, \
                         0xffffff00, \
                         0xffff00ff, \
                         0xff00ffff, \
                         0x00ffffff, \
                         0xffffff00, \
                         0xffff00ff, \
                         0xff00ffff, \
                         0x00ffffff, \
                         0xffffff00, \
                         0xffffff00, \
                         0xffff00ff, \
                         0xff00ffff, \
                         0x00ffffff, \
                         0xfffffff7, \
                         0xffffffdf, \
                         0xffffff00, \
                         0xff00ffff, \
                         0xfffffe00, \
                         0xffffff00, \
                         0xffffff00, \
                         0xff00ffff, \
                         0xffffffef, \
                         0xffffffbf, \
                         0xffff00ff, \
                         0x00ffffff, \
                         0xfffc01ff, \
                         0xffff00ff, \
                         0xffff00ff, \
                         0x00ffffff, \
                         0xfffffffb, \
                         0xfffffffe, \
                         0xfffffffe, \
                         0xfffffffe, \
                         0xfffffffe, \
                         0xfffffffe, \
                         0xffffc000, \
                         0xc000ffff, \
                         0xffffc000, \
                         0xc000ffff, \
                         0xffffc000, \
                         0xc000ffff, \
                         0xffffc000, \
                         0xc000ffff, \
                         0xffffc000, \
                         0xf800ffff, \
                         0xfffff800, \
                         0xf800ffff};

u64 ini_frame_1_reg_val[82] = {0x00000000, \
                         0x00000000, \
                         0x00000040, \
                         0x00800000, \
                         0x00000000, \
                         0x00000004, \
                         0x000000f0, \
                         0x00000000, \
                         0x00000080, \
                         0x00000002, \
                         0x00000000, \
                         0x00008000, \
                         0x00900000, \
                         0xa0000000, \
                         0x000000b0, \
                         0x0000c000, \
                         0x00d00000, \
                         0xe0000000, \
                         0x000000f0, \
                         0x0000ff00, \
                         0x00ff0000, \
                         0xff000000, \
                         0x000000ff, \
                         0x0000ff00, \
                         0x00ff0000, \
                         0xff000000, \
                         0x000000ff, \
                         0x00000010, \
                         0x00001000, \
                         0x00ff0000, \
                         0x84000000, \
                         0x0000000c, \
                         0x00000000, \
                         0x00000000, \
                         0xff000000, \
                         0x00000084, \
                         0x00000c00, \
                         0x00000000, \
                         0x00000000, \
                         0x00000010, \
                         0x00002000, \
                         0x00300000, \
                         0x40000000, \
                         0x00000050, \
                         0x00000008, \
                         0x00001000, \
                         0x00200000, \
                         0x40000000, \
                         0x00000008, \
                         0x00000020, \
                         0x000000af, \
                         0x00900000, \
                         0x00000104, \
                         0x00000040, \
                         0x000000ff, \
                         0x00ff0000, \
                         0x00000010, \
                         0x00000040, \
                         0x00009000, \
                         0xaf000000, \
                         0x00020800, \
                         0x00004000, \
                         0x0000e800, \
                         0xe8000000, \
                         0x00000000, \
                         0x00000000, \
                         0x00000000, \
                         0x00000000, \
                         0x00000000, \
                         0x00000001, \
                         0x00000132, \
                         0x02590000, \
                         0x00000075, \
                         0x3f530000, \
                         0x00003ead, \
                         0x02000000, \
                         0x00000200, \
                         0x3e530000, \
                         0x00003fad, \
                         0x00000000, \
                         0x00000200, \
                         0x02000000};

static int rtl_ini_frame_1_setreg(int argc, char **argv) {
    int i = 0;
    while(i < REG_INI_FRAME_1_WRITE_TIME) {
        int j = i;
        uintptr_t addr = ini_frame_1_reg_addr[j];
        u32 val = readl(addr);
        while (1) {
            val = ((ini_frame_1_reg_mask[j] & val) | ( ~ini_frame_1_reg_mask[j] & ini_frame_1_reg_val[j]));
            if (ini_frame_1_reg_val[j+1] != addr) break;
            j++;
        }
        i = j + 1;
        writel(addr, val);
    };
    printf("set reg num: %d", i);
    return 0;
}
static struct cmd_entry ini_frame_1_cmd __attribute__ ((unused)) = 
    {"ini_frame_1", rtl_ini_frame_1_setreg, 0, "test"};
#endif
