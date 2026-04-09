/*
 * Copyright Cvitek Technologies Inc.
 *
 * Created Time: Feb, 2020
 */
#ifndef __TESTCASE_VDEC_H__
#define __TESTCASE_VDEC_H__

#define H265_VDEC_INTR_NUM  77

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifdef TEST_H265_DEC
int cvitest_h265_vdec_main(int argc, char **argv);
int irq_handler_h265_vdec(int irqn, void *priv);
#endif

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
