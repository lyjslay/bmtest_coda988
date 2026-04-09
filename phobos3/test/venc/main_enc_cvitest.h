/*
 * Copyright Bitmain Technologies Inc.
 *
 * Created Time: Dec, 2019
 */
#ifndef __MAIN_ENC_CVITEST_H__
#define __MAIN_ENC_CVITEST_H__

#define H264_VENC_INTR_NUM  76
#define H265_VENC_INTR_NUM  77
#define SUPPORT_INTERRUPT

#ifdef TEST_H264_ENC
int cvitest_h264_venc_main(int argc, char **argv);
#ifdef SUPPORT_INTERRUPT
int irq_handler_h264_venc(int irqn, void *priv);
#endif
#endif

#ifdef TEST_H265_ENC
int cvitest_venc_main(int argc, char **argv);
#ifdef SUPPORT_INTERRUPT
int irq_handler_h265_venc(int irqn, void *priv);
#endif
#endif

#define PXP_STRESS_TEST 1

#if PXP_STRESS_TEST
int cvi_venc_pxp_main(int argc, char **argv);
void *cvi_venc_pxp_init(int argc, char **argv);
int cvi_venc_pxp_enc_one_frame(void *handle);
#endif

enum TestEnc
{
    TE_STA_ENC_BREAK = 20,
    TE_STA_ENC_CONTINUE = 21,
    TE_ERR_ENC_INIT = -10,
    TE_ERR_ENC_OPEN = -11,
};

#endif
