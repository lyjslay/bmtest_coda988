#include "boot_test.h"
#include "uart.h"
#include "cli.h"
#include "module_testcase.h"
#include "command.h"
#include "system_common.h"
#include "mmio.h"
#include "reg_tempsen_top.h"

#define TEMPSEN_MASK(REG_NAME) tempsen_top_##REG_NAME##_MASK
#define TEMPSEN_OFFSET(REG_NAME) tempsen_top_##REG_NAME##_OFFSET
#define TEMPSEN_SET(REG_NAME, VAL) \
          mmio_clrsetbits_32(TEMPSEN_BASE + tempsen_top_##REG_NAME, \
            TEMPSEN_MASK(REG_NAME), (VAL) << TEMPSEN_OFFSET(REG_NAME))
#define TEMPSEN_GET(REG_NAME) \
          ((mmio_read_32(TEMPSEN_BASE + tempsen_top_##REG_NAME) & \
            TEMPSEN_MASK(REG_NAME)) >> TEMPSEN_OFFSET(REG_NAME))

#define ARRAY_SIZE(x)	(sizeof(x) / sizeof((x)[0]))

#define BIT(n) (UINT32_C(1) << (n))
#define CH0 BIT(0)
#define CH1 BIT(1)
#define CH2 BIT(2)
#define CH3 BIT(3)

static void wait_for_finish(uint32_t mask)
{
  while((TEMPSEN_GET(sta_tempsen_intr_raw)&mask) != mask)
    __asm__ volatile ("nop");
}

static int read_temp_and_check(uint32_t expect_val)
{
  int ret = 0;
  uint32_t temp_val;

  /* read ch0 temperature */
  temp_val = TEMPSEN_GET(sta_tempsen_ch0_result);
  uartlog("ch0 result = 0x%X\n", temp_val);
  if (temp_val != expect_val) {
    uartlog("Failed: expected ch0 result = 0x%X\n", expect_val);
    ret = -1;
  }

  return ret;
}

static int read_max_and_check(uint32_t expect_val)
{
  int ret = 0;
  uint32_t temp_val;

  /* read ch0 max temperature */
  temp_val = TEMPSEN_GET(sta_tempsen_ch0_max_result);
  uartlog("ch0 max result = 0x%X\n", temp_val);
  if (temp_val != expect_val) {
    uartlog("Failed: expected ch0 max result = 0x%X\n", expect_val);
    ret = -1;
  }

  return ret;
}

static int read_ddr_high_temp_raw_and_check(uint32_t expect_val, uint32_t ch)
{
  int ret = 0;
  uint32_t ddr_high_temp_raw;

  /* read ch0 temperature */
  ddr_high_temp_raw = TEMPSEN_GET(sta_tempsen_ddr_high_temp_raw);
  uartlog("ddr_high_temp_raw = 0x%X\n", ddr_high_temp_raw);
  if ((ddr_high_temp_raw & ch) != expect_val) {
    uartlog("Failed: expected ddr_high_temp_raw of ch(0x%x) = 0x%X\n", ch, expect_val);
    ret = -1;
  }

  return ret;
}

static int read_ddr_high_temp_o_and_check(uint32_t expect_val, uint32_t ch)
{
  int ret = 0;
  uint32_t ddr_high_temp_o;

  /* read ch0 temperature */
  ddr_high_temp_o = TEMPSEN_GET(sta_tempsen_ddr_high_temp_o);
  uartlog("ddr_high_temp_o = 0x%X\n", ddr_high_temp_o);
  if ((ddr_high_temp_o & ch) != expect_val) {
    uartlog("Failed: expected ddr_high_temp_o of ch(0x%x) = 0x%X\n", ch, expect_val);
    ret = -1;
  }

  return ret;
}

static int do_tempsen_1(int argc, char **argv)
{
  int ret = 0;
  uint32_t force_val = 0x30;

  uartlog("%s: START\n", __func__);
  /* set/enable force value*/
  TEMPSEN_SET(reg_tempsen_force_result, force_val);
  TEMPSEN_SET(reg_tempsen_force_en, 1);

  /* clear all interrrupt status */
  TEMPSEN_SET(sta_tempsen_intr_clr, TEMPSEN_GET(sta_tempsen_intr_raw));

  /* clear max result */
  TEMPSEN_SET(clr_tempsen_ch0_max_result, 1);

  /* enable ch0 and ch1 */
  TEMPSEN_SET(reg_tempsen_sel, CH0);
  TEMPSEN_SET(reg_tempsen_en, 1);

  wait_for_finish(CH0);

  ret = read_temp_and_check(force_val);

  /* disable ch0 and ch1 */
  TEMPSEN_SET(reg_tempsen_sel, 0);
  TEMPSEN_SET(reg_tempsen_en, 0);

  /* clear all interrrupt status */
  TEMPSEN_SET(sta_tempsen_intr_clr, TEMPSEN_GET(sta_tempsen_intr_raw));

  uartlog("%s: %s\n", __func__, ret?"failed":"passed");

  return ret;
}

static int do_tempsen_2(int argc, char **argv)
{
  int ret = 0;
  uint32_t force_val = 0x30;
  uint32_t max_force_val = 0x40;

  uartlog("%s: START\n", __func__);
  /* set/enable force value*/
  TEMPSEN_SET(reg_tempsen_force_result, force_val);
  TEMPSEN_SET(reg_tempsen_force_en, 1);

  /* clear all interrrupt status */
  TEMPSEN_SET(sta_tempsen_intr_clr, TEMPSEN_GET(sta_tempsen_intr_raw));

  /* clear max result */
  TEMPSEN_SET(clr_tempsen_ch0_max_result, 1);

  /* enable ch0 and ch1 */
  TEMPSEN_SET(reg_tempsen_sel, CH0);
  TEMPSEN_SET(reg_tempsen_en, 1);

  wait_for_finish(CH0);

  TEMPSEN_SET(reg_tempsen_en, 0);
  TEMPSEN_SET(sta_tempsen_intr_clr, TEMPSEN_GET(sta_tempsen_intr_raw));
  TEMPSEN_SET(reg_tempsen_force_result, max_force_val);
  TEMPSEN_SET(reg_tempsen_en, 1);

  wait_for_finish(CH0);

  TEMPSEN_SET(reg_tempsen_en, 0);
  TEMPSEN_SET(sta_tempsen_intr_clr, TEMPSEN_GET(sta_tempsen_intr_raw));
  TEMPSEN_SET(reg_tempsen_force_result, force_val);
  TEMPSEN_SET(reg_tempsen_en, 1);

  wait_for_finish(CH0);

  do {
    ret = read_temp_and_check(force_val);
    if (ret) {
      uartlog("failed at line %d\n", __LINE__);
      break;
    }

    ret = read_max_and_check(max_force_val);
    if (ret) {
      uartlog("failed at line %d\n", __LINE__);
      break;
    }
  } while (0);

  /* disable ch0 and ch1 */
  TEMPSEN_SET(reg_tempsen_sel, 0);
  TEMPSEN_SET(reg_tempsen_en, 0);

  /* clear all interrrupt status */
  TEMPSEN_SET(sta_tempsen_intr_clr, TEMPSEN_GET(sta_tempsen_intr_raw));

  uartlog("%s: %s\n", __func__, ret?"failed":"passed");

  return ret;
}

static int do_tempsen_3_4_5(int argc, char **argv)
{
  int ret = 0;
  uint32_t force_val = 0x30;
  uint32_t force_val_over_hi = 0x50;
  uint32_t force_val_under_lo = 0x10;
  uint32_t force_val_overheat = 0x70;
  uint32_t hi_th = 0x40;
  uint32_t lo_th = 0x20;
  uint32_t overheat_th = 0x60;
  uint32_t temp_val;

  uartlog("%s: START\n", __func__);

  /* set hi/lo threshold value */
  TEMPSEN_SET(reg_tempsen_ch0_hi_th, hi_th);
  TEMPSEN_SET(reg_tempsen_ch0_lo_th, lo_th);

  /* set overheat threshold value */
  TEMPSEN_SET(reg_tempsen_overheat_th, overheat_th);

  /* clear threshold count */
  TEMPSEN_SET(reg_ch0_temp_th_cnt_clr, 1);
  TEMPSEN_SET(reg_ch1_temp_th_cnt_clr, 1);

  /* clear all interrrupt status */
  TEMPSEN_SET(sta_tempsen_intr_clr, TEMPSEN_GET(sta_tempsen_intr_raw));

  /* clear max result */
  TEMPSEN_SET(clr_tempsen_ch0_max_result, 1);

  /* clear threshold count */
  TEMPSEN_SET(reg_ch0_temp_th_cnt_clr, 1);

  /* set/enable force value*/
  TEMPSEN_SET(reg_tempsen_force_result, force_val);
  TEMPSEN_SET(reg_tempsen_force_en, 1);

  /* enable ch0 and ch1 */
  TEMPSEN_SET(reg_tempsen_sel, CH0);
  TEMPSEN_SET(reg_tempsen_en, 1);

  wait_for_finish(CH0);

  do {

    ret = read_temp_and_check(force_val);

    TEMPSEN_SET(reg_tempsen_en, 0);
    TEMPSEN_SET(sta_tempsen_intr_clr, TEMPSEN_GET(sta_tempsen_intr_raw));
    TEMPSEN_SET(reg_tempsen_force_result, force_val_over_hi);
    TEMPSEN_SET(reg_tempsen_en, 1);

    wait_for_finish(CH0);

    ret = read_temp_and_check(force_val_over_hi);
    if (ret) {
      uartlog("failed at line %d\n", __LINE__);
      break;
    }

    ret = read_max_and_check(force_val_over_hi);
    if (ret) {
      uartlog("failed at line %d\n", __LINE__);
      break;
    }

    /* check interrupt for ch3~ch0 temp higher than high temperature threshold */
    temp_val = TEMPSEN_GET(sta_tempsen_intr_raw);
    uartlog("sta_tempsen_intr_raw = 0x%X\n", temp_val);
    if ((temp_val & BIT(4)) != BIT(4)) {
      uartlog("Fail: bit4 have not been raised.\n");
      ret = -1;
    }
    if (ret) {
      uartlog("failed at line %d\n", __LINE__);
      break;
    }

    /* check over hi threshold counter */
    temp_val = TEMPSEN_GET(sta_ch0_over_hi_temp_th_cnt);
    uartlog("sta_ch0_over_hi_temp_th_cnt = 0x%X\n", temp_val);
    if (temp_val != 1) {
      uartlog("Fail: expect sta_ch0_over_hi_temp_th_cnt = 1.\n");
      ret = -1;
    }
    if (ret) {
      uartlog("failed at line %d\n", __LINE__);
      break;
    }

    /***************************************************************/

    TEMPSEN_SET(reg_tempsen_en, 0);
    TEMPSEN_SET(sta_tempsen_intr_clr, TEMPSEN_GET(sta_tempsen_intr_raw));
    TEMPSEN_SET(reg_tempsen_force_result, force_val_under_lo);
    TEMPSEN_SET(reg_tempsen_en, 1);

    wait_for_finish(CH0);

    ret = read_temp_and_check(force_val_under_lo);
    if (ret) {
      uartlog("failed at line %d\n", __LINE__);
      break;
    }

    ret = read_max_and_check(force_val_over_hi);
    if (ret) {
      uartlog("failed at line %d\n", __LINE__);
      break;
    }

    /* check interrupt for ch3~ch0 temp lower than low temperature threshol */
    temp_val = TEMPSEN_GET(sta_tempsen_intr_raw);
    uartlog("sta_tempsen_intr_raw = 0x%X\n", temp_val);
    if ((temp_val & BIT(8)) != BIT(8)) {
      uartlog("Fail: bit8 have not been raised.\n");
      ret = -1;
    }
    if (ret) {
      uartlog("failed at line %d\n", __LINE__);
      break;
    }

    /* check under lo threshold counter */
    temp_val = TEMPSEN_GET(sta_ch0_under_lo_temp_th_cnt);
    uartlog("sta_ch0_under_lo_temp_th_cnt = 0x%X\n", temp_val);
    if (temp_val != 1) {
      uartlog("Fail: expect sta_ch0_under_lo_temp_th_cnt = 1.\n");
      ret = -1;
    }
    if (ret) {
      uartlog("failed at line %d\n", __LINE__);
      break;
    }

    /***************************************************************/

    TEMPSEN_SET(reg_tempsen_en, 0);
    TEMPSEN_SET(sta_tempsen_intr_clr, TEMPSEN_GET(sta_tempsen_intr_raw));
    TEMPSEN_SET(reg_tempsen_force_result, force_val_overheat);
    TEMPSEN_SET(reg_tempsen_en, 1);

    wait_for_finish(CH0);

    ret = read_temp_and_check(force_val_overheat);
    if (ret) {
      uartlog("failed at line %d\n", __LINE__);
      break;
    }

    ret = read_max_and_check(force_val_overheat);
    if (ret) {
      uartlog("failed at line %d\n", __LINE__);
      break;
    }

    /* check interrupt for ch3~ch0 temp higher than high temperature threshold */
    temp_val = TEMPSEN_GET(sta_tempsen_intr_raw);
    uartlog("sta_tempsen_intr_raw = 0x%X\n", temp_val);
    if ((temp_val & BIT(4)) != BIT(4)) {
      uartlog("Fail: bit4 have not been raised.\n");
      ret = -1;
    }
    if (ret) {
      uartlog("failed at line %d\n", __LINE__);
      break;
    }

    /* check over hi threshold counter */
    temp_val = TEMPSEN_GET(sta_ch0_over_hi_temp_th_cnt);
    uartlog("sta_ch0_over_hi_temp_th_cnt = 0x%X\n", temp_val);
    if (temp_val != 1) {
      uartlog("Fail: expect sta_ch0_over_hi_temp_th_cnt = 1.\n");
      ret = -1;
    }
    if (ret) {
      uartlog("failed at line %d\n", __LINE__);
      break;
    }

    /* check overheat interrupt */
    temp_val = TEMPSEN_GET(sta_tempsen_intr_raw);
    uartlog("sta_tempsen_intr_raw = 0x%X\n", temp_val);
    if ((temp_val & BIT(20)) != BIT(20)) {
      uartlog("Fail: bit20 have not been raised.\n");
      ret = -1;
    }
    if (ret) {
      uartlog("failed at line %d\n", __LINE__);
      break;
    }

  } while (0);

  /* disable ch0 and ch1 */
  TEMPSEN_SET(reg_tempsen_sel, 0);
  TEMPSEN_SET(reg_tempsen_en, 0);

  /* clear all interrrupt status */
  TEMPSEN_SET(sta_tempsen_intr_clr, TEMPSEN_GET(sta_tempsen_intr_raw));

  uartlog("%s: %s\n", __func__, ret?"failed":"passed");

  return ret;
}

static int do_tempsen_6(int argc, char **argv)
{
  int ret = 0;
  uint32_t force_val = 0x30;
  uint32_t force_val_over_hi = 0x50;
  uint32_t force_val_under_lo = 0x10;
  uint32_t force_val_overheat = 0x70;
  uint32_t hi_th = 0x40;
  uint32_t lo_th = 0x20;
  uint32_t overheat_th = 0x60;
  uint32_t temp_val;
  uint32_t overheat_reset_done = 0;

  uartlog("%s: START\n", __func__);

  /* set hi/lo threshold value */
  TEMPSEN_SET(reg_tempsen_ch0_hi_th, hi_th);
  TEMPSEN_SET(reg_tempsen_ch0_lo_th, lo_th);

  /* set overheat threshold value */
  TEMPSEN_SET(reg_tempsen_overheat_th, overheat_th);

  /* clear threshold count */
  TEMPSEN_SET(reg_ch0_temp_th_cnt_clr, 1);

  /* clear all interrrupt status */
  TEMPSEN_SET(sta_tempsen_intr_clr, TEMPSEN_GET(sta_tempsen_intr_raw));

  /* clear max result */
  TEMPSEN_SET(clr_tempsen_ch0_max_result, 1);

  /* clear threshold count */
  TEMPSEN_SET(reg_ch0_temp_th_cnt_clr, 1);

  /* set/enable force value*/
  TEMPSEN_SET(reg_tempsen_force_result, force_val);
  TEMPSEN_SET(reg_tempsen_force_en, 1);

  /* enable ch0 and ch1 */
  TEMPSEN_SET(reg_tempsen_sel, CH0);
  TEMPSEN_SET(reg_tempsen_en, 1);

  wait_for_finish(CH0);

  do {

    ret = read_temp_and_check(force_val);
    if (ret) {
      uartlog("failed at line %d\n", __LINE__);
      break;
    }

    TEMPSEN_SET(reg_tempsen_en, 0);
    TEMPSEN_SET(sta_tempsen_intr_clr, TEMPSEN_GET(sta_tempsen_intr_raw));
    TEMPSEN_SET(reg_tempsen_force_result, force_val_over_hi);
    TEMPSEN_SET(reg_tempsen_en, 1);

    wait_for_finish(CH0);

    ret = read_temp_and_check(force_val_over_hi);
    if (ret) {
      uartlog("failed at line %d\n", __LINE__);
      break;
    }

    ret = read_max_and_check(force_val_over_hi);
    if (ret) {
      uartlog("failed at line %d\n", __LINE__);
      break;
    }

    /* check interrupt for ch3~ch0 temp higher than high temperature threshold */
    temp_val = TEMPSEN_GET(sta_tempsen_intr_raw);
    uartlog("sta_tempsen_intr_raw = 0x%X\n", temp_val);
    if ((temp_val & BIT(4)) != BIT(4)) {
      uartlog("Fail: bit4 have not been raised.\n");
      ret = -1;
    }
    if (ret) {
      uartlog("failed at line %d\n", __LINE__);
      break;
    }

    /* check over hi threshold counter */
    temp_val = TEMPSEN_GET(sta_ch0_over_hi_temp_th_cnt);
    uartlog("sta_ch0_over_hi_temp_th_cnt = 0x%X\n", temp_val);
    if (temp_val != 1) {
      uartlog("Fail: expect sta_ch0_over_hi_temp_th_cnt = 1.\n");
      ret = -1;
    }
    if (ret) {
      uartlog("failed at line %d\n", __LINE__);
      break;
    }

    /***************************************************************/

    TEMPSEN_SET(reg_tempsen_en, 0);
    TEMPSEN_SET(sta_tempsen_intr_clr, TEMPSEN_GET(sta_tempsen_intr_raw));
    TEMPSEN_SET(reg_tempsen_force_result, force_val_under_lo);
    TEMPSEN_SET(reg_tempsen_en, 1);

    wait_for_finish(CH0);

    ret = read_temp_and_check(force_val_under_lo);
    if (ret) {
      uartlog("failed at line %d\n", __LINE__);
      break;
    }

    ret = read_max_and_check(force_val_over_hi);
    if (ret) {
      uartlog("failed at line %d\n", __LINE__);
      break;
    }

    /* check interrupt for ch3~ch0 temp lower than low temperature threshol */
    temp_val = TEMPSEN_GET(sta_tempsen_intr_raw);
    uartlog("sta_tempsen_intr_raw = 0x%X\n", temp_val);
    if ((temp_val & BIT(8)) != BIT(8)) {
      uartlog("Fail: bit8 have not been raised.\n");
      ret = -1;
    }
    if (ret) {
      uartlog("failed at line %d\n", __LINE__);
      break;
    }

    /* check under lo threshold counter */
    temp_val = TEMPSEN_GET(sta_ch0_under_lo_temp_th_cnt);
    uartlog("sta_ch0_under_lo_temp_th_cnt = 0x%X\n", temp_val);
    if (temp_val != 1) {
      uartlog("Fail: expect sta_ch0_under_lo_temp_th_cnt = 1.\n");
      ret = -1;
    }
    if (ret) {
      uartlog("failed at line %d\n", __LINE__);
      break;
    }

    /***************************************************************/
    /* enable overheat reset */
    TEMPSEN_SET(reg_overheat_reset_en, 1);

    TEMPSEN_SET(reg_tempsen_en, 0);
    TEMPSEN_SET(sta_tempsen_intr_clr, TEMPSEN_GET(sta_tempsen_intr_raw));
    TEMPSEN_SET(reg_tempsen_force_result, force_val_overheat);
    TEMPSEN_SET(reg_tempsen_en, 1);

    wait_for_finish(CH0);

    ret = read_temp_and_check(force_val_overheat);
    if (ret) {
      uartlog("failed at line %d\n", __LINE__);
      break;
    }

    ret = read_max_and_check(force_val_overheat);
    if (ret) {
      uartlog("failed at line %d\n", __LINE__);
      break;
    }

    /* check interrupt for ch3~ch0 temp higher than high temperature threshold */
    temp_val = TEMPSEN_GET(sta_tempsen_intr_raw);
    uartlog("sta_tempsen_intr_raw = 0x%X\n", temp_val);
    if ((temp_val & BIT(4)) != BIT(4)) {
      uartlog("Fail: bit4 have not been raised.\n");
      ret = -1;
    }
    if (ret) {
      uartlog("failed at line %d\n", __LINE__);
      break;
    }

    /* check over hi threshold counter */
    temp_val = TEMPSEN_GET(sta_ch0_over_hi_temp_th_cnt);
    uartlog("sta_ch0_over_hi_temp_th_cnt = 0x%X\n", temp_val);
    if (temp_val != 1) {
      uartlog("Fail: expect sta_ch0_over_hi_temp_th_cnt = 1.\n");
      ret = -1;
    }
    if (ret) {
      uartlog("failed at line %d\n", __LINE__);
      break;
    }

    /* check overheat interrupt */
    temp_val = TEMPSEN_GET(sta_tempsen_intr_raw);
    uartlog("sta_tempsen_intr_raw = 0x%X\n", temp_val);
    if ((temp_val & BIT(20)) != BIT(20)) {
      uartlog("Fail: bit20 have not been raised.\n");
      ret = -1;
    }
    if (ret) {
      uartlog("failed at line %d\n", __LINE__);
      break;
    }

    /* check overheat countdown */
    do {
      temp_val = TEMPSEN_GET(sta_tempsen_overheat_countdown);
      uartlog("sta_tempsen_overheat_countdown = 0x%X\n", temp_val);
      uartlog("sta_overheat_reset = 0x%X\n", TEMPSEN_GET(sta_overheat_reset));
      if ((overheat_reset_done == 0) && (temp_val < 0x50000)) {
        TEMPSEN_SET(reg_overheat_reset_clr, 1);
        uartlog("overheat_reset_done\n");
        overheat_reset_done = temp_val;
        TEMPSEN_SET(reg_tempsen_en, 0);
        TEMPSEN_SET(sta_tempsen_intr_clr, TEMPSEN_GET(sta_tempsen_intr_raw));
        TEMPSEN_SET(reg_tempsen_en, 1);
        wait_for_finish(CH0);
      }
    } while (temp_val > 1);

    uartlog("overheat_reset_done = 0x%X\n", overheat_reset_done);

    temp_val = TEMPSEN_GET(sta_overheat_reset);
    uartlog("sta_overheat_reset = 0x%X\n", temp_val);
    if (temp_val != 1) {
      uartlog("Fail: Expect sta_overheat_reset = 1\n");
      ret = -1;
    }
    if (ret) {
      uartlog("failed at line %d\n", __LINE__);
      break;
    }

  } while (0);

  /* disable ch0 and ch1 */
  TEMPSEN_SET(reg_tempsen_sel, 0);
  TEMPSEN_SET(reg_tempsen_en, 0);

  /* clear all interrrupt status */
  TEMPSEN_SET(sta_tempsen_intr_clr, TEMPSEN_GET(sta_tempsen_intr_raw));

  uartlog("%s: %s\n", __func__, ret?"failed":"passed");

  return ret;

}

static int do_tempsen_7_8(int argc, char **argv)
{
  int ret = 0;
  uint32_t force_val = 0x20;
  uint32_t force_val_over_hi = 0x70;
  uint32_t force_val_between_hi_lo = 0x50;
  uint32_t force_val_under_lo = 0x30;
  uint32_t ddr_hi_th = 0x60;
  uint32_t ddr_lo_th = 0x40;

  uartlog("%s: START\n", __func__);
  /* set/enable force value*/
  TEMPSEN_SET(reg_tempsen_force_result, force_val);
  TEMPSEN_SET(reg_tempsen_force_en, 1);

  /* clear all interrrupt status */
  TEMPSEN_SET(sta_tempsen_intr_clr, TEMPSEN_GET(sta_tempsen_intr_raw));

  /* clear max result */
  TEMPSEN_SET(clr_tempsen_ch0_max_result, 1);

  /* set ddr hi/lo threshold */
  TEMPSEN_SET(reg_tempsen_ddr_hi_th, ddr_hi_th);
  TEMPSEN_SET(reg_tempsen_ddr_lo_th, ddr_lo_th);

  /* enable ddr auto refresh rate ctrl signal output */
  TEMPSEN_SET(reg_tempsen_ddr_out_en, 1);

  /* enable ch0 */
  TEMPSEN_SET(reg_tempsen_sel, CH0);
  TEMPSEN_SET(reg_tempsen_en, 1);

  wait_for_finish(CH0);

  do {

    ret = read_temp_and_check(force_val);
    if (ret) {
      uartlog("failed at line %d\n", __LINE__);
      break;
    }

    /* check sta_ddr_high_temp_raw (should be 0) */
    ret = read_ddr_high_temp_raw_and_check(0, CH0);
    if (ret) {
      uartlog("failed at line %d\n", __LINE__);
      break;
    }

    /*** verify overwrite function ***/
    /* set overwrite value of ddr auto refresh rate ctrl signal to 1 */
    TEMPSEN_SET(reg_tempsen_ddr_ow_val, 1);

    /* enable overwrite of ddr auto refresh rate ctrl signal */
    TEMPSEN_SET(reg_tempsen_ddr_ow_en, 1);

    /* check sta_ddr_high_temp_raw (should be 0) */
    ret = read_ddr_high_temp_raw_and_check(0, CH0);
    if (ret) {
      uartlog("failed at line %d\n", __LINE__);
      break;
    }

    /* check sta_ddr_high_temp_o (should be 1) */
    ret = read_ddr_high_temp_o_and_check(1, CH0);
    if (ret) {
      uartlog("failed at line %d\n", __LINE__);
      break;
    }

    /* set overwrite value of ddr auto refresh rate ctrl signal to 0 */
    TEMPSEN_SET(reg_tempsen_ddr_ow_val, 0);

    /* check sta_ddr_high_temp_raw (should be 0) */
    ret = read_ddr_high_temp_raw_and_check(0, CH0);
    if (ret) {
      uartlog("failed at line %d\n", __LINE__);
      break;
    }

    /* check sta_ddr_high_temp_o (should be 0) */
    ret = read_ddr_high_temp_o_and_check(0, CH0);
    if (ret) {
      uartlog("failed at line %d\n", __LINE__);
      break;
    }

    /* disable overwrite of ddr auto refresh rate ctrl signal */
    TEMPSEN_SET(reg_tempsen_ddr_ow_en, 0);

    /*** check ddr hi/lo threshold ***/
    /* Set over hi temp */
    TEMPSEN_SET(reg_tempsen_en, 0);
    TEMPSEN_SET(sta_tempsen_intr_clr, TEMPSEN_GET(sta_tempsen_intr_raw));
    TEMPSEN_SET(reg_tempsen_force_result, force_val_over_hi);
    TEMPSEN_SET(reg_tempsen_en, 1);

    wait_for_finish(CH0);

    /* check sta_ddr_high_temp_raw (should be 1) */
    ret = read_ddr_high_temp_raw_and_check(1, CH0);
    if (ret) {
      uartlog("failed at line %d\n", __LINE__);
      break;
    }

    /* Set temp between hi and lo */
    TEMPSEN_SET(reg_tempsen_en, 0);
    TEMPSEN_SET(sta_tempsen_intr_clr, TEMPSEN_GET(sta_tempsen_intr_raw));
    TEMPSEN_SET(reg_tempsen_force_result, force_val_between_hi_lo);
    TEMPSEN_SET(reg_tempsen_en, 1);

    wait_for_finish(CH0);

    /* check sta_ddr_high_temp_raw (should be 1) */
    ret = read_ddr_high_temp_raw_and_check(1, CH0);
    if (ret) {
      uartlog("failed at line %d\n", __LINE__);
      break;
    }

    /* Set temp between hi and lo */
    TEMPSEN_SET(reg_tempsen_en, 0);
    TEMPSEN_SET(sta_tempsen_intr_clr, TEMPSEN_GET(sta_tempsen_intr_raw));
    TEMPSEN_SET(reg_tempsen_force_result, force_val_under_lo);
    TEMPSEN_SET(reg_tempsen_en, 1);

    wait_for_finish(CH0);

    /* check sta_ddr_high_temp_raw (should be 0) */
    ret = read_ddr_high_temp_raw_and_check(0, CH0);
    if (ret) {
      uartlog("failed at line %d\n", __LINE__);
      break;
    }
  } while (0);

  /* disable ddr auto refresh rate ctrl signal output */
  TEMPSEN_SET(reg_tempsen_ddr_out_en, 0);

  /* disable ch0 */
  TEMPSEN_SET(reg_tempsen_sel, 0);
  TEMPSEN_SET(reg_tempsen_en, 0);

  /* clear all interrrupt status */
  TEMPSEN_SET(sta_tempsen_intr_clr, TEMPSEN_GET(sta_tempsen_intr_raw));

  uartlog("%s: %s\n", __func__, ret?"failed":"passed");

  return ret;
}

static struct cmd_entry tempsen_test_cmd_list[] __attribute__ ((unused)) = {
  {"tempsen_1", do_tempsen_1, 0, "[read temperature]"},
  {"tempsen_2", do_tempsen_2, 0, "[max temperature]"},
  {"tempsen_3_4_5", do_tempsen_3_4_5, 0, "[hi/lo/overheat threshold interrupt/counter]"},
  {"tempsen_6", do_tempsen_6, 0, "[overheat reset]"},
  {"tempsen_7_8", do_tempsen_7_8, 0, "[ddr auto refresh rate ctrl and overwrite]"},
  {NULL, NULL, 0, NULL}
};

static int tempsen_irq_handler(int irqn, void *priv)
{
  uint32_t intr_sta;
  uint32_t intr_sta_raw;
  uartlog("INT: %d\n", irqn);

  disable_irq(irqn);
  intr_sta = TEMPSEN_GET(sta_tempsen_intr_sta);
  uartlog("tempsen_irq_handler intr_sta=0x%X\n", intr_sta);
  intr_sta_raw = TEMPSEN_GET(sta_tempsen_intr_raw);
  uartlog("tempsen_irq_handler intr_sta_raw=0x%X\n", intr_sta_raw);
  // TEMPSEN_SET(sta_tempsen_intr_clr, intr_sta_raw);
  return 0;
}

int tempsen_intr_init(void)
{
  TEMPSEN_SET(sta_tempsen_intr_en, 0xFFFFFFFF);
  request_irq(TEMPSEN_IRQ_O,  tempsen_irq_handler, 0, "TEMPSEN_Int", NULL);
  enable_irq(TEMPSEN_IRQ_O);
  return 0;
}

int testcase_tempsen(void)
{
  int i;
  int ret;

  uartlog("%s\n", __func__);

  tempsen_intr_init();

  for(i = 0;i < ARRAY_SIZE(tempsen_test_cmd_list) - 1;i++) {
    command_add(&tempsen_test_cmd_list[i]);
  }

  do {
    ret = cli_simple_loop();
  } while (ret == 0);
  return 0;
}

#ifndef BUILD_TEST_CASE_all
int testcase_main()
{
  int ret = testcase_tempsen();
  uartlog("testcase tempsen %s\n", ret?"failed":"passed");
  return ret;
}
#endif
