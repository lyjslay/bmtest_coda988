#include "boot_test.h"
#include "cli.h"
#include "mmio.h"
#include "testcase_rtc.h"
#include "uart.h"

#define REG_RTC_BASE 0x05026000

#define RTC_INFO0 0x1C
#define RTC_INFO1 0x20
#define RTC_DB_REQ_WARM_RST 0x60
#define RTC_EN_WARM_RST_REQ 0xCC
#define RTC_ST_ON_REASON 0xF8

#define REG_RTCFC_BASE 0x03004000
#define RTC_CTRL0_UNLOCK 0x4
#define RTC_CTRL0 0x8

#define PWR_VBAT_DET 0
#define PWR_ON 1
#define PWR_BTTON0 2
#define PWR_BTTON1 3
#define PWR_BTTON1_7_SEC 4
#define PWR_WAKEUP0 5
#define PWR_WAKEUP1 6
#define ALARM 7
#define REQ_POR_CYC 8
#define REQ_THM_SHDN 9
#define REQ_WARM_RST 10
#define REQ_WDT_RST 11
#define REQ_SHDN 12
#define REQ_SUSPEND 13
#define RESERVED 14
#define ST_ON 15

typedef struct st_on_reason
{
  uint32_t idx;
  char *reason;
} st_on_reason_st;

st_on_reason_st pwr_on_reason[] = {
                                    {0, "PWR_VBAT_DET"},
                                    {1, "PWR_ON"},
                                    {2, "PWR_BTTON0"},
                                    {3, "PWR_BTTON1"},
                                    {4, "PWR_BTTON1_7_SEC"},
                                    {5, "PWR_WAKEUP0"},
                                    {6, "PWR_WAKEUP1"},
                                    {7, "Alarm"},
                                    {8, "REQ_POR_CYC"},
                                    {9, "REQ_THM_SHDN"},
                                    {10, "REQ_WARM_RST"},
                                    {11, "REQ_WDT_RST"},
                                    {12, "REQ_SHDN"},
                                    {13, "REQ_SUSPEND"},
                                    {14, "reserved"},
                                    {15, "ST_ON"},
                                  };

void check_power_on_reason(uint32_t expected_state)
{
  uint32_t read_data = 0;
  int i = 0;
  
  read_data = mmio_read_32(REG_RTC_BASE + RTC_ST_ON_REASON);
  uartlog("RTC_ST_ON_REASON: %X\n", read_data);
  for (i=15; i>=0; i--)
  {
    if(((read_data >> (i+16)) & 0x1) == 1)
    {
      uartlog("%s\n", pwr_on_reason[i].reason);
    }
  }

  if(((read_data >> (16 + expected_state)) & 0x1) == 1)
    uartlog("PASS\n");
  else
    uartlog("FAIL\n");
  return;
}

int testcase_rtc(void)
{
  uartlog("%s\n", __func__);
  uint32_t read_data = mmio_read_32(REG_RTC_BASE + RTC_INFO0);
  uint32_t write_data = 0;

  uartlog("RTC_INFO0: %X\n", read_data);

  if(read_data == 0xABCD1234)
  {
    mmio_write_32(REG_RTC_BASE + RTC_INFO0, 0xABCD5678);
    read_data = mmio_read_32(REG_RTC_BASE + RTC_INFO0);
    uartlog("current status: 1st time power up\n");
    uartlog("SW overwrite RTC_INFO0: %X\n", read_data);
  } 

  if(read_data == 0xABCD5678)
  {
    check_power_on_reason(ST_ON);

    // enable warm reset request
    uartlog("Enable warm reset request\n");
    mmio_write_32(REG_RTC_BASE + RTC_DB_REQ_WARM_RST, 0x4);
    mmio_write_32(REG_RTC_BASE + RTC_EN_WARM_RST_REQ, 0x1);

    read_data = mmio_read_32(REG_RTC_BASE + RTC_INFO1);
    if(read_data != 0x99999999)
    {
      // unlock rtc_ctrl0
      mmio_write_32(REG_RTCFC_BASE + RTC_CTRL0_UNLOCK, 0xAB18);

      // trigger req_warm_rst
      read_data = mmio_read_32(REG_RTCFC_BASE + RTC_CTRL0);
      write_data = read_data | 0xFFFF0000 | (0x1 << 4);
      uartlog("Set req_warm_rst to 1\n");
      mmio_write_32(REG_RTCFC_BASE + RTC_CTRL0, write_data);
    }

    check_power_on_reason(REQ_WARM_RST);
  } 
  else {
      uartlog("failed to overwrite RTC_INFO0\n");
      return 1;
  }

  check_power_on_reason(REQ_WARM_RST);

  return 0;
}

#ifndef BUILD_TEST_CASE_all
int testcase_main()
{
  int ret = testcase_rtc();
  uartlog("testcase rtc %s\n", ret?"failed":"passed");
  return ret;
}
#endif
