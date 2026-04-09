#include <stdio.h>
#include "test.h"

#if TEST_GPIO
int test_gpio()
{
        uint32_t xdata v1 = read_robot(RTC_GPIO_BASE + 0x00);
        uint32_t xdata v2 = read_robot(RTC_GPIO_BASE + 0x04);

        printf("test_gpio()\n");
	printf("0x%08lx 0x%08lx\n", v1, v2);

        write_robot(RTC_GPIO_BASE + 0x00, 0xAABB);
        write_robot(RTC_GPIO_BASE + 0x04, 0xCCDD);

        v1 = read_robot(RTC_GPIO_BASE + 0x00);
        v2 = read_robot(RTC_GPIO_BASE + 0x04);
	printf("0x%08lx 0x%08lx\n", v1, v2);

	return 0;
}
#endif /* TEST_GPIO */