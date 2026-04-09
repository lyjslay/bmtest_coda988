#include <stdio.h>
#include "test.h"

#if TEST_SPINOR

int test_spinor(void)
{
        uint32_t spinor_base = 0x05400000;
        uint32_t a, v;
        int i;

	printf("spinor1:\n");
        for(i = 0; i < 0x10; i += 4) {
		a = spinor_base + i;
                v = read_robot(a);
                printf("0x%08lx: 0x%08lx\n", a, v);
        }

        return 0;
}

#endif /* TEST_SPINOR */