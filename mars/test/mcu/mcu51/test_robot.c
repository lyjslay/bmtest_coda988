#include <stdio.h>
#include "test.h"

static uint32_t xdata xu[6];

#if TEST_ROBOT1
int test_robot1(void)
{
    int i;
    // write_robot(0x05025018, 0x0003fe7f);
    // write_robot(0x05025018, 0x0003FFFF);

    write_robot(REG_GP_REG0, 0x11121314); // TOP
    write_robot(0x03020000,  0xB5B6B7B8); // GPIO
    // write_robot(0x05021000,  0xE1E2E3E4); // gpio_ctrl APB32 4KB
    // write_robot(0x05023000,  0xE1E2E3E4); // intr_ctrl APB32 4KB
    // write_robot(0x0502b000,  0xE1E2E3E4); // i2c APB32 4KB

    xu[0] = read_robot(REG_GP_REG0); // TOP
    xu[1] = read_robot(0x03020000); // GPIO
    xu[2] = read_robot(0x05021000); // gpio_ctrl APB32 4KB
    xu[3] = read_robot(0x05023000); // intr_ctrl APB32 4KB
    xu[4] = read_robot(0x0502b000); // i2c APB32 4KB
    xu[5] = read_robot(0x05400000); // spinor
    for(i = 0; i < sizeof(xu) / sizeof(xu[0]); i++) {

	    printf("xu[%d]=0x%lx\n", i, xu[i]);
    }

    return 0;
}
#endif /* TEST_ROBOT1 */