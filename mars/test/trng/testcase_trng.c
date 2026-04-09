#include "boot_test.h"
#include "uart.h"
#include "testcase_trng.h"
#include "cli.h"
#include "mmio.h"
#include "system_common.h"

int uart_loopback_test(uint8_t data)
{
  uint8_t recv;

  uart_putc(data);
  recv = (uint8_t)uart_getc();
  if (data != recv)
    return -1;

  return 0;
}

int testcase_trng()
{
    uint32_t  data;
    uint32_t  tmp_data;
    int ret = 0;

    //check noise ready
    data = mmio_read_32(GetTrngAddr(0xc));
    while((data & 0x80000000) == 0x80000000)
    {
        data = mmio_read_32(GetTrngAddr(0xc));
        uartlog("1. check noise ready\n");
    }

    data = mmio_read_32(GetTrngAddr(0x14));
    while(!(data >> 2))
    {
        data = mmio_read_32(GetTrngAddr(0x14));
        uartlog("2. check istate %#X\n", data);
    }

    uartlog("start seed generation");
    mmio_write_32(GetTrngAddr(0x00), 0x1);

    data = mmio_read_32(GetTrngAddr(0x14));
    while(!(data >> 2))
    {//busy bit
        data = mmio_read_32(GetTrngAddr(0x14));
        uartlog("3. check istate %#X\n", data);
    }
    // ack and clear the done bit flag
    mmio_write_32(GetTrngAddr(0x14), 0x10);

    uartlog("create state\n");
    mmio_write_32(GetTrngAddr(0x00), 0x3);

    data = mmio_read_32(GetTrngAddr(0x14));
    while(!(data >> 2))
    {//busy bit
        data = mmio_read_32(GetTrngAddr(0x14));
        uartlog("4. check istate %#X\n", data);
    }
    // ack and clear the done bit flag
    mmio_write_32(GetTrngAddr(0x14), 0x10);

    while(1) {

        uartlog("generation random number\n");
        mmio_write_32(GetTrngAddr(0x00), 0x6);

        for(int i=0; i<(TRNG_DATA_WIDTH/TRNG_ADDR_WIDTH); i++)
            uartlog("random number[%d] = %#x\n", i, mmio_read_32(GetTrngAddr(0x24)+(4*i)));

        data = 0x0;
        for(int k = 0; k<(TRNG_DATA_WIDTH/TRNG_ADDR_WIDTH); k++)
        {
            uint32_t reg_addr;
            reg_addr = GetTrngAddr(0x24)+(4*k);
            tmp_data = mmio_read_32(reg_addr);
            tmp_data = tmp_data & 0xff;
            data = data | (tmp_data << TRNG_ADDR_WIDTH*k);
        }
        uartlog("data %x\n", data);
    }

    return ret;
}

#ifndef BUILD_TEST_CASE_all
int testcase_main()
{
  int ret = testcase_trng();
  uartlog("testcase trng dec %s\n", ret?"failed":"passed");

  return ret;
}
#endif
