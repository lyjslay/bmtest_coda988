#include "boot_test.h"
#include "uart.h"
#include "cli.h"
#include "module_testcase.h"
#include "command.h"
#include "system_common.h"

#define ARRAY_SIZE(x)	(sizeof(x) / sizeof((x)[0]))


#ifdef BUILD_TEST_CASE_SLT_efuse
extern int testcase_efuse(void);
static int do_slt_efuse(int argc, char **argv)
{
	int ret;

	uartlog("SLT_EFUSE_TEST: START\n");
  ret = testcase_efuse();
	uartlog("SLT_EFUSE_TEST: %s\n", ret?"failed":"passed");

	return ret;
}
#endif //BUILD_TEST_CASE_SLT_efuse

#ifdef BUILD_TEST_CASE_SLT_i2c
extern int testcase_i2c(void);
static int do_slt_i2c(int argc, char **argv)
{
	int ret;

	uartlog("SLT_I2C_TEST: START\n");
  ret = testcase_i2c();
	uartlog("SLT_I2C_TEST: %s\n", ret?"failed":"passed");

	return ret;
}
#endif //BUILD_TEST_CASE_SLT_i2c

#ifdef BUILD_TEST_CASE_SLT_dhrystone
extern int testcase_dhrystone(void);
static int do_slt_dhrystone(int argc, char **argv)
{
	int ret;

	uartlog("SLT_DHRYSTONE_TEST: START\n");
  ret = testcase_dhrystone();
	uartlog("SLT_DHRYSTONE_TEST: %s\n", ret?"failed":"passed");

	return ret;
}
#endif //BUILD_TEST_CASE_SLT_dhrystone

#ifdef BUILD_TEST_CASE_SLT_audio
extern int do_audio(int argc, char **argv);
static int do_slt_audio(int argc, char **argv)
{
	int ret;

	uartlog("SLT_AUDIO_TEST: START\n");
  ret = do_audio(argc, argv);
	uartlog("SLT_AUDIO_TEST: %s\n", ret?"failed":"passed");

	return ret;
}
#endif //BUILD_TEST_CASE_SLT_audio

#ifdef BUILD_TEST_CASE_SLT_ddr
extern int testcase_ddr(void);
static int do_slt_ddr(int argc, char **argv)
{
	int ret;

	uartlog("SLT_DDR_TEST: START\n");
  ret = testcase_ddr();
	uartlog("SLT_DDR_TEST: %s\n", ret?"failed":"passed");

	return ret;
}
#endif //BUILD_TEST_CASE_SLT_ddr

#ifdef BUILD_TEST_CASE_SLT_spi_nor
extern int testcase_spi_nor(void);
static int do_slt_spi_nor(int argc, char **argv)
{
	int ret;

	uartlog("SLT_SPI_TEST: START\n");
	ret = testcase_spi_nor();
	uartlog("SLT_SPI_TEST: %s\n", ret?"failed":"passed");

	return ret;
}
#endif //BUILD_TEST_CASE_SLT_spi_nor

#ifdef BUILD_TEST_CASE_SLT_emmc
extern int testcase_emmc(void);
static int do_slt_emmc(int argc, char **argv)
{
	int ret;

	uartlog("SLT_EMMC_TEST: START\n");
  ret = testcase_emmc();
	uartlog("SLT_EMMC_TEST: %s\n", ret?"failed":"passed");

	return ret;
}
#endif //BUILD_TEST_CASE_SLT_emmc

#ifdef BUILD_TEST_CASE_SLT_sdcard
extern int testcase_sdcard(void);
static int do_slt_sdcard(int argc, char **argv)
{
	int ret;

	uartlog("SLT_SDCARD_TEST: START\n");
  ret = testcase_sdcard();
	uartlog("SLT_SDCARD_TEST: %s\n", ret?"failed":"passed");

	return ret;
}
#endif //BUILD_TEST_CASE_SLT_sdcard

#ifdef BUILD_TEST_CASE_SLT_usb
extern int testcase_usb(void);
static int do_slt_usb_device(int argc, char **argv)
{
	int ret;

	uartlog("SLT_USB_DEVICE_TEST: START\n");
  ret = testcase_usb();
	uartlog("SLT_USB_DEVICE_TEST: %s\n", ret?"failed":"passed");

	return ret;
}
#endif //BUILD_TEST_CASE_SLT_usb

#ifdef BUILD_TEST_CASE_SLT_vpp
extern int testcase_vpp(void);
static int do_slt_vpp(int argc, char **argv)
{
	int ret;

	uartlog("SLT_VPP_TEST: START\n");
  ret = testcase_vpp();
	uartlog("SLT_VPP_TEST: %s\n", ret?"failed":"passed");

	return ret;
}
#endif //BUILD_TEST_CASE_SLT_vpp

#ifdef BUILD_TEST_CASE_SLT_jpeg
extern int testcase_jpeg(void);
static int do_slt_jpeg(int argc, char **argv)
{
    int ret;

    uartlog("SLT_JPEG_TEST: START\n");
    ret = testcase_jpeg();
    uartlog("SLT_JPEG_TEST: %s\n", ret?"failed":"passed");

    return ret;
}
#endif //BUILD_TEST_CASE_SLT_jpeg

#ifdef BUILD_TEST_CASE_SLT_boda
extern int testcase_boda(void);
static int do_slt_boda(int argc, char **argv)
{
    int ret;

    uartlog("SLT_BODA_TEST: START\n");
    ret = testcase_boda();
    uartlog("SLT_BODA_TEST: %s\n", ret?"failed":"passed");

    return ret;
}
#endif //BUILD_TEST_CASE_SLT_boda

#ifdef BUILD_TEST_CASE_SLT_get_matrix_stride
extern int testcase_get_matrix_stride(void);
static int do_slt_get_matrix_stride(int argc, char **argv)
{
	int ret;

	uartlog("SLT_GET_MATRIX_STRIDE_TEST: START\n");
	ret = testcase_get_matrix_stride();
	uartlog("SLT_GET_MATRIX_STRIDE_TEST: %s\n", ret?"failed":"passed");

	return ret;
}
#endif //BUILD_TEST_CASE_SLT_get_matrix_stride

#ifdef BUILD_TEST_CASE_SLT_get_matrix_tp
extern int testcase_get_matrix_tp(void);
static int do_slt_get_matrix_tp(int argc, char **argv)
{
	int ret;

	uartlog("SLT_GET_MATRIX_TP_TEST: START\n");
	ret = testcase_get_matrix_tp();
	uartlog("SLT_GET_MATRIX_TP_TEST: %s\n", ret?"failed":"passed");

	return ret;
}
#endif //BUILD_TEST_CASE_SLT_get_matrix_tp

#ifdef BUILD_TEST_CASE_SLT_get_tensor_stride
extern int testcase_get_tensor_stride(void);
static int do_slt_get_tensor_stride(int argc, char **argv)
{
	int ret;

	uartlog("SLT_GET_TENSOR_STRIDE_TEST: START\n");
	ret = testcase_get_tensor_stride();
	uartlog("SLT_GET_TENSOR_STRIDE_TEST: %s\n", ret?"failed":"passed");

	return ret;
}
#endif //BUILD_TEST_CASE_SLT_get_tensor_stride

#ifdef BUILD_TEST_CASE_SLT_get_tensor_stride_unalign
extern int testcase_get_tensor_stride_unalign(void);
static int do_slt_get_tensor_stride_unalign(int argc, char **argv)
{
	int ret;

	uartlog("SLT_GET_TENSOR_STRIDE_UNALIGN_TEST: START\n");
	ret = testcase_get_tensor_stride_unalign();
	uartlog("SLT_GET_TENSOR_STRIDE_UNALIGN_TEST: %s\n", ret?"failed":"passed");

	return ret;
}
#endif //BUILD_TEST_CASE_SLT_get_tensor_stride_unalign

#ifdef BUILD_TEST_CASE_SLT_get_tensor_tp
extern int testcase_get_tensor_tp(void);
static int do_slt_get_tensor_tp(int argc, char **argv)
{
	int ret;

	uartlog("SLT_GET_TENSOR_TP_TEST: START\n");
	ret = testcase_get_tensor_tp();
	uartlog("SLT_GET_TENSOR_TP_TEST: %s\n", ret?"failed":"passed");

	return ret;
}
#endif //BUILD_TEST_CASE_SLT_get_tensor_tp

#ifdef BUILD_TEST_CASE_SLT_lut
extern int testcase_lut(void);
static int do_slt_lut(int argc, char **argv)
{
	int ret;

	uartlog("SLT_LUT_TEST: START\n");
	ret = testcase_lut();
	uartlog("SLT_LUT_TEST: %s\n", ret?"failed":"passed");

	return ret;
}
#endif //BUILD_TEST_CASE_SLT_lut

#ifdef BUILD_TEST_CASE_SLT_matrix_transfer
extern int testcase_matrix_transfer(void);
static int do_slt_matrix_transfer(int argc, char **argv)
{
	int ret;

	uartlog("SLT_MATRIX_TRANSFER_TEST: START\n");
	ret = testcase_matrix_transfer();
	uartlog("SLT_MATRIX_TRANSFER_TEST: %s\n", ret?"failed":"passed");

	return ret;
}
#endif //BUILD_TEST_CASE_SLT_matrix_transfer

#ifdef BUILD_TEST_CASE_SLT_put_matrix_stride
extern int testcase_put_matrix_stride(void);
static int do_slt_put_matrix_stride(int argc, char **argv)
{
	int ret;

	uartlog("SLT_PUT_MATRIX_STRIDE_TEST: START\n");
	ret = testcase_put_matrix_stride();
	uartlog("SLT_PUT_MATRIX_STRIDE_TEST: %s\n", ret?"failed":"passed");

	return ret;
}
#endif //BUILD_TEST_CASE_SLT_put_matrix_stride

#ifdef BUILD_TEST_CASE_SLT_put_matrix_tp
extern int testcase_put_matrix_tp(void);
static int do_slt_put_matrix_tp(int argc, char **argv)
{
	int ret;

	uartlog("SLT_PUT_MATRIX_TP_TEST: START\n");
	ret = testcase_put_matrix_tp();
	uartlog("SLT_PUT_MATRIX_TP_TEST: %s\n", ret?"failed":"passed");

	return ret;
}
#endif //BUILD_TEST_CASE_SLT_put_matrix_tp

#ifdef BUILD_TEST_CASE_SLT_put_tensor_stride
extern int testcase_put_tensor_stride(void);
static int do_slt_put_tensor_stride(int argc, char **argv)
{
	int ret;

	uartlog("SLT_PUT_TENSOR_STRIDE_TEST: START\n");
	ret = testcase_put_tensor_stride();
	uartlog("SLT_PUT_TENSOR_STRIDE_TEST: %s\n", ret?"failed":"passed");

	return ret;
}
#endif //BUILD_TEST_CASE_SLT_put_tensor_stride

#ifdef BUILD_TEST_CASE_SLT_put_tensor_stride_unalign
extern int testcase_put_tensor_stride_unalign(void);
static int do_slt_put_tensor_stride_unalign(int argc, char **argv)
{
	int ret;

	uartlog("SLT_PUT_TENSOR_STRIDE_UNALIGN_TEST: START\n");
	ret = testcase_put_tensor_stride_unalign();
	uartlog("SLT_PUT_TENSOR_STRIDE_UNALIGN_TEST: %s\n", ret?"failed":"passed");

	return ret;
}
#endif //BUILD_TEST_CASE_SLT_put_tensor_stride_unalign

#ifdef BUILD_TEST_CASE_SLT_put_tensor_tp
extern int testcase_put_tensor_tp(void);
static int do_slt_put_tensor_tp(int argc, char **argv)
{
	int ret;

	uartlog("SLT_PUT_TENSOR_TP_TEST: START\n");
	ret = testcase_put_tensor_tp();
	uartlog("SLT_PUT_TENSOR_TP_TEST: %s\n", ret?"failed":"passed");

	return ret;
}
#endif //BUILD_TEST_CASE_SLT_put_tensor_tp

#ifdef BUILD_TEST_CASE_SLT_put_tensor_tp_unalign
extern int testcase_put_tensor_tp_unalign(void);
static int do_slt_put_tensor_tp_unalign(int argc, char **argv)
{
	int ret;

	uartlog("SLT_PUT_TENSOR_TP_UNALIGN_TEST: START\n");
	ret = testcase_put_tensor_tp_unalign();
	uartlog("SLT_PUT_TENSOR_TP_UNALIGN_TEST: %s\n", ret?"failed":"passed");

	return ret;
}
#endif //BUILD_TEST_CASE_SLT_put_tensor_tp_unalign

#ifdef BUILD_TEST_CASE_SLT_put_tensor_unalign
extern int testcase_put_tensor_unalign(void);
static int do_slt_put_tensor_unalign(int argc, char **argv)
{
	int ret;

	uartlog("SLT_PUT_TENSOR_UNALIGN_TEST: START\n");
	ret = testcase_put_tensor_unalign();
	uartlog("SLT_PUT_TENSOR_UNALIGN_TEST: %s\n", ret?"failed":"passed");

	return ret;
}
#endif //BUILD_TEST_CASE_SLT_put_tensor_unalign

#ifdef BUILD_TEST_CASE_SLT_relu
extern int testcase_relu(void);
static int do_slt_relu(int argc, char **argv)
{
	int ret;

	uartlog("SLT_RELU_TEST: START\n");
	ret = testcase_relu();
	uartlog("SLT_RELU_TEST: %s\n", ret?"failed":"passed");

	return ret;
}
#endif //BUILD_TEST_CASE_SLT_relu

#ifdef BUILD_TEST_CASE_SLT_tensor_add
extern int testcase_tensor_add(void);
static int do_slt_tensor_add(int argc, char **argv)
{
	int ret;

	uartlog("SLT_TENSOR_ADD_TEST: START\n");
	ret = testcase_tensor_add();
	uartlog("SLT_TENSOR_ADD_TEST: %s\n", ret?"failed":"passed");

	return ret;
}
#endif //BUILD_TEST_CASE_SLT_tensor_add

#ifdef BUILD_TEST_CASE_SLT_tensor_and
extern int testcase_tensor_and(void);
static int do_slt_tensor_and(int argc, char **argv)
{
	int ret;

	uartlog("SLT_TENSOR_AND_TEST: START\n");
	ret = testcase_tensor_and();
	uartlog("SLT_TENSOR_AND_TEST: %s\n", ret?"failed":"passed");

	return ret;
}
#endif //BUILD_TEST_CASE_SLT_tensor_and

#ifdef BUILD_TEST_CASE_SLT_tensor_arith_shift
extern int testcase_tensor_arith_shift(void);
static int do_slt_tensor_arith_shift(int argc, char **argv)
{
	int ret;

	uartlog("SLT_TENSOR_ARITH_SHIFT_TEST: START\n");
	ret = testcase_tensor_arith_shift();
	uartlog("SLT_TENSOR_ARITH_SHIFT_TEST: %s\n", ret?"failed":"passed");

	return ret;
}
#endif //BUILD_TEST_CASE_SLT_tensor_arith_shift

#ifdef BUILD_TEST_CASE_SLT_tensor_copy
extern int testcase_tensor_copy(void);
static int do_slt_tensor_copy(int argc, char **argv)
{
	int ret;

	uartlog("SLT_TENSOR_COPY_TEST: START\n");
	ret = testcase_tensor_copy();
	uartlog("SLT_TENSOR_COPY_TEST: %s\n", ret?"failed":"passed");

	return ret;
}
#endif //BUILD_TEST_CASE_SLT_tensor_copy

#ifdef BUILD_TEST_CASE_SLT_tensor_mac
extern int testcase_tensor_mac(void);
static int do_slt_tensor_mac(int argc, char **argv)
{
	int ret;

	uartlog("SLT_TENSOR_MAC_TEST: START\n");
	ret = testcase_tensor_mac();
	uartlog("SLT_TENSOR_MAC_TEST: %s\n", ret?"failed":"passed");

	return ret;
}
#endif //BUILD_TEST_CASE_SLT_tensor_mac

#ifdef BUILD_TEST_CASE_SLT_tensor_mac_const
extern int testcase_tensor_mac_const(void);
static int do_slt_tensor_mac_const(int argc, char **argv)
{
	int ret;

	uartlog("SLT_TENSOR_MAC_CONST_TEST: START\n");
	ret = testcase_tensor_mac_const();
	uartlog("SLT_TENSOR_MAC_CONST_TEST: %s\n", ret?"failed":"passed");

	return ret;
}
#endif //BUILD_TEST_CASE_SLT_tensor_mac_const

#ifdef BUILD_TEST_CASE_SLT_tensor_max
extern int testcase_tensor_max(void);
static int do_slt_tensor_max(int argc, char **argv)
{
	int ret;

	uartlog("SLT_TENSOR_MAX_TEST: START\n");
	ret = testcase_tensor_max();
	uartlog("SLT_TENSOR_MAX_TEST: %s\n", ret?"failed":"passed");

	return ret;
}
#endif //BUILD_TEST_CASE_SLT_tensor_max

#ifdef BUILD_TEST_CASE_SLT_tensor_mdsum
extern int testcase_tensor_mdsum(void);
static int do_slt_tensor_mdsum(int argc, char **argv)
{
	int ret;

	uartlog("SLT_TENSOR_MDSUM_TEST: START\n");
	ret = testcase_tensor_mdsum();
	uartlog("SLT_TENSOR_MDSUM_TEST: %s\n", ret?"failed":"passed");

	return ret;
}
#endif //BUILD_TEST_CASE_SLT_tensor_mdsum

#ifdef BUILD_TEST_CASE_SLT_tensor_min
extern int testcase_tensor_min(void);
static int do_slt_tensor_min(int argc, char **argv)
{
	int ret;

	uartlog("SLT_TENSOR_MIN_TEST: START\n");
	ret = testcase_tensor_min();
	uartlog("SLT_TENSOR_MIN_TEST: %s\n", ret?"failed":"passed");

	return ret;
}
#endif //BUILD_TEST_CASE_SLT_tensor_min

#ifdef BUILD_TEST_CASE_SLT_tensor_mul
extern int testcase_tensor_mul(void);
static int do_slt_tensor_mul(int argc, char **argv)
{
	int ret;

	uartlog("SLT_TENSOR_MUL_TEST: START\n");
	ret = testcase_tensor_mul();
	uartlog("SLT_TENSOR_MUL_TEST: %s\n", ret?"failed":"passed");

	return ret;
}
#endif //BUILD_TEST_CASE_SLT_tensor_mul

#ifdef BUILD_TEST_CASE_SLT_tensor_mul_const
extern int testcase_tensor_mul_const(void);
static int do_slt_tensor_mul_const(int argc, char **argv)
{
	int ret;

	uartlog("SLT_TENSOR_MUL_CONST_TEST: START\n");
	ret = testcase_tensor_mul_const();
	uartlog("SLT_TENSOR_MUL_CONST_TEST: %s\n", ret?"failed":"passed");

	return ret;
}
#endif //BUILD_TEST_CASE_SLT_tensor_mul_const

#ifdef BUILD_TEST_CASE_SLT_tensor_or
extern int testcase_tensor_or(void);
static int do_slt_tensor_or(int argc, char **argv)
{
	int ret;

	uartlog("SLT_TENSOR_OR_TEST: START\n");
	ret = testcase_tensor_or();
	uartlog("SLT_TENSOR_OR_TEST: %s\n", ret?"failed":"passed");

	return ret;
}
#endif //BUILD_TEST_CASE_SLT_tensor_or

#ifdef BUILD_TEST_CASE_SLT_tensor_sub
extern int testcase_tensor_sub(void);
static int do_slt_tensor_sub(int argc, char **argv)
{
	int ret;

	uartlog("SLT_TENSOR_SUB_TEST: START\n");
	ret = testcase_tensor_sub();
	uartlog("SLT_TENSOR_SUB_TEST: %s\n", ret?"failed":"passed");

	return ret;
}
#endif //BUILD_TEST_CASE_SLT_tensor_sub

#ifdef BUILD_TEST_CASE_SLT_tensor_transfer
extern int testcase_tensor_transfer(void);
static int do_slt_tensor_transfer(int argc, char **argv)
{
	int ret;

	uartlog("SLT_TENSOR_TRANSFER_TEST: START\n");
	ret = testcase_tensor_transfer();
	uartlog("SLT_TENSOR_TRANSFER_TEST: %s\n", ret?"failed":"passed");

	return ret;
}
#endif //BUILD_TEST_CASE_SLT_tensor_transfer

#ifdef BUILD_TEST_CASE_SLT_tensor_xor
extern int testcase_tensor_xor(void);
static int do_slt_tensor_xor(int argc, char **argv)
{
	int ret;

	uartlog("SLT_TENSOR_XOR_TEST: START\n");
	ret = testcase_tensor_xor();
	uartlog("SLT_TENSOR_XOR_TEST: %s\n", ret?"failed":"passed");

	return ret;
}
#endif //BUILD_TEST_CASE_SLT_tensor_xor

#ifdef BUILD_TEST_CASE_SLT_tg_copy_gdma
extern int testcase_tg_copy_gdma(void);
static int do_slt_tg_copy_gdma(int argc, char **argv)
{
	int ret;

	uartlog("SLT_TG_COPY_GDMA_TEST: START\n");
	ret = testcase_tg_copy_gdma();
	uartlog("SLT_TG_COPY_GDMA_TEST: %s\n", ret?"failed":"passed");

	return ret;
}
#endif //BUILD_TEST_CASE_SLT_tg_copy_gdma

#ifdef BUILD_TEST_CASE_SLT_tl_copy_gdma_fixed
extern int testcase_tl_copy_gdma_fixed(void);
static int do_slt_tl_copy_gdma_fixed(int argc, char **argv)
{
	int ret;

	uartlog("SLT_TL_COPY_GDMA_FIXED_TEST: START\n");
	ret = testcase_tl_copy_gdma_fixed();
	uartlog("SLT_TL_COPY_GDMA_FIXED_TEST: %s\n", ret?"failed":"passed");

	return ret;
}
#endif //BUILD_TEST_CASE_SLT_tl_copy_gdma_fixed

#ifdef BUILD_TEST_CASE_SLT_conv
extern int testcase_conv(void);
static int do_slt_conv(int argc, char **argv)
{
	int ret;

	uartlog("SLT_CONV_TEST: START\n");
	ret = testcase_conv();
	uartlog("SLT_CONV_TEST: %s\n", ret?"failed":"passed");

	return ret;
}
#endif //BUILD_TEST_CASE_SLT_conv

#ifdef BUILD_TEST_CASE_SLT_fc
extern int testcase_fc(void);
static int do_slt_fc(int argc, char **argv)
{
	int ret;

	uartlog("SLT_FC_TEST: START\n");
	ret = testcase_fc();
	uartlog("SLT_FC_TEST: %s\n", ret?"failed":"passed");

	return ret;
}
#endif //BUILD_TEST_CASE_SLT_fc

#ifdef BUILD_TEST_CASE_SLT_pooling
extern int testcase_pooling(void);
static int do_slt_pooling(int argc, char **argv)
{
	int ret;

	uartlog("SLT_POOLING_TEST: START\n");
	ret = testcase_pooling();
	uartlog("SLT_POOLING_TEST: %s\n", ret?"failed":"passed");

	return ret;
}
#endif //BUILD_TEST_CASE_SLT_pooling

#ifdef BUILD_TEST_CASE_SLT_matrix_mac_2
extern int testcase_matrix_mac_2(void);
static int do_slt_matrix_mac_2(int argc, char **argv)
{
	int ret;

	uartlog("SLT_MATRIX_MAC_2_TEST: START\n");
	ret = testcase_matrix_mac_2();
	uartlog("SLT_MATRIX_MAC_2_TEST: %s\n", ret?"failed":"passed");

	return ret;
}
#endif //BUILD_TEST_CASE_SLT_matrix_mac_2

#ifdef BUILD_TEST_CASE_SLT_winograd
extern int testcase_winograd(void);
static int do_slt_winograd(int argc, char **argv)
{
	int ret;

	uartlog("SLT_WINOGRAD_TEST: START\n");
	ret = testcase_winograd();
	uartlog("SLT_WINOGRAD_TEST: %s\n", ret?"failed":"passed");

	return ret;
}
#endif //BUILD_TEST_CASE_SLT_winograd

#ifdef BUILD_TEST_CASE_SLT_net_test
extern int testcase_net_test(void);
static int do_slt_net_test(int argc, char **argv)
{
	int ret;

	uartlog("SLT_NET_TEST_TEST: START\n");
	ret = testcase_net_test();
	uartlog("SLT_NET_TEST_TEST: %s\n", ret?"failed":"passed");

	return ret;
}
#endif //BUILD_TEST_CASE_SLT_net_test

static struct cmd_entry slt_test_cmd_list[] __attribute__ ((unused)) = {
#ifdef BUILD_TEST_CASE_SLT_efuse
	{"slt_efuse", do_slt_efuse, 0, "[SLT EFUSE test]"},
#endif
#ifdef BUILD_TEST_CASE_SLT_i2c
	{"slt_i2c", do_slt_i2c, 0, "[SLT I2C test]"},
#endif
#ifdef BUILD_TEST_CASE_SLT_dhrystone
	{"slt_dhrystone", do_slt_dhrystone, 0, "[SLT Dhrystone test]"},
#endif
#ifdef BUILD_TEST_CASE_SLT_audio
	{"slt_audio", do_slt_audio, 2, "[SLT AUDIO test]"},
#endif
#ifdef BUILD_TEST_CASE_SLT_ddr
	{"slt_ddr", do_slt_ddr, 0, "[SLT DDR test]"},
#endif
#ifdef BUILD_TEST_CASE_SLT_spi_nor
	{"slt_spi", do_slt_spi_nor, 0, "[SLT SPI NOR test]"},
#endif
#ifdef BUILD_TEST_CASE_SLT_emmc
	{"slt_emmc", do_slt_emmc, 0, "[SLT eMMC test]"},
#endif
#ifdef BUILD_TEST_CASE_SLT_sdcard
	{"slt_sdcard", do_slt_sdcard, 0, "[SLT SD Card test]"},
#endif
#ifdef BUILD_TEST_CASE_SLT_usb
	{"slt_usb_device", do_slt_usb_device, 0, "[SLT USB device test]"},
#endif
#ifdef BUILD_TEST_CASE_SLT_vpp
	{"slt_vpp", do_slt_vpp, 0, "[SLT VPP test]"},
#endif
#ifdef BUILD_TEST_CASE_SLT_jpeg
    {"slt_jpeg", do_slt_jpeg, 0, "[SLT JPEG test]"},
#endif
#ifdef BUILD_TEST_CASE_SLT_boda
    {"slt_boda", do_slt_boda, 0, "[SLT BODA test]"},
#endif
#ifdef BUILD_TEST_CASE_SLT_get_matrix_stride
	{"slt_get_matrix_stride", do_slt_get_matrix_stride, 0, "[SLT get_matrix_stride test]"},
#endif
#ifdef BUILD_TEST_CASE_SLT_get_matrix_tp
	{"slt_get_matrix_tp", do_slt_get_matrix_tp, 0, "[SLT get_matrix_tp test]"},
#endif
#ifdef BUILD_TEST_CASE_SLT_get_tensor_stride
	{"slt_get_tensor_stride", do_slt_get_tensor_stride, 0, "[SLT get_tensor_stride test]"},
#endif
#ifdef BUILD_TEST_CASE_SLT_get_tensor_stride_unalign
	{"slt_get_tensor_stride_unalign", do_slt_get_tensor_stride_unalign, 0, "[SLT get_tensor_stride_unalign test]"},
#endif
#ifdef BUILD_TEST_CASE_SLT_get_tensor_tp
	{"slt_get_tensor_tp", do_slt_get_tensor_tp, 0, "[SLT get_tensor_tp test]"},
#endif
#ifdef BUILD_TEST_CASE_SLT_lut
	{"slt_lut", do_slt_lut, 0, "[SLT lut test]"},
#endif
#ifdef BUILD_TEST_CASE_SLT_matrix_transfer
	{"slt_matrix_transfer", do_slt_matrix_transfer, 0, "[SLT matrix_transfer test]"},
#endif
#ifdef BUILD_TEST_CASE_SLT_put_matrix_stride
	{"slt_put_matrix_stride", do_slt_put_matrix_stride, 0, "[SLT put_matrix_stride test]"},
#endif
#ifdef BUILD_TEST_CASE_SLT_put_matrix_tp
	{"slt_put_matrix_tp", do_slt_put_matrix_tp, 0, "[SLT put_matrix_tp test]"},
#endif
#ifdef BUILD_TEST_CASE_SLT_put_tensor_stride
	{"slt_put_tensor_stride", do_slt_put_tensor_stride, 0, "[SLT put_tensor_stride test]"},
#endif
#ifdef BUILD_TEST_CASE_SLT_put_tensor_stride_unalign
	{"slt_put_tensor_stride_unalign", do_slt_put_tensor_stride_unalign, 0, "[SLT put_tensor_stride_unalign test]"},
#endif
#ifdef BUILD_TEST_CASE_SLT_put_tensor_tp
	{"slt_put_tensor_tp", do_slt_put_tensor_tp, 0, "[SLT put_tensor_tp test]"},
#endif
#ifdef BUILD_TEST_CASE_SLT_put_tensor_tp_unalign
	{"slt_put_tensor_tp_unalign", do_slt_put_tensor_tp_unalign, 0, "[SLT put_tensor_tp_unalign test]"},
#endif
#ifdef BUILD_TEST_CASE_SLT_put_tensor_unalign
	{"slt_put_tensor_unalign", do_slt_put_tensor_unalign, 0, "[SLT put_tensor_unalign test]"},
#endif
#ifdef BUILD_TEST_CASE_SLT_relu
	{"slt_relu", do_slt_relu, 0, "[SLT relu test]"},
#endif
#ifdef BUILD_TEST_CASE_SLT_tensor_add
	{"slt_tensor_add", do_slt_tensor_add, 0, "[SLT tensor_add test]"},
#endif
#ifdef BUILD_TEST_CASE_SLT_tensor_and
	{"slt_tensor_and", do_slt_tensor_and, 0, "[SLT tensor_and test]"},
#endif
#ifdef BUILD_TEST_CASE_SLT_tensor_arith_shift
	{"slt_tensor_arith_shift", do_slt_tensor_arith_shift, 0, "[SLT tensor_arith_shift test]"},
#endif
#ifdef BUILD_TEST_CASE_SLT_tensor_copy
	{"slt_tensor_copy", do_slt_tensor_copy, 0, "[SLT tensor_copy test]"},
#endif
#ifdef BUILD_TEST_CASE_SLT_tensor_mac
	{"slt_tensor_mac", do_slt_tensor_mac, 0, "[SLT tensor_mac test]"},
#endif
#ifdef BUILD_TEST_CASE_SLT_tensor_mac_const
	{"slt_tensor_mac_const", do_slt_tensor_mac_const, 0, "[SLT tensor_mac_const test]"},
#endif
#ifdef BUILD_TEST_CASE_SLT_tensor_max
	{"slt_tensor_max", do_slt_tensor_max, 0, "[SLT tensor_max test]"},
#endif
#ifdef BUILD_TEST_CASE_SLT_tensor_mdsum
	{"slt_tensor_mdsum", do_slt_tensor_mdsum, 0, "[SLT tensor_mdsum test]"},
#endif
#ifdef BUILD_TEST_CASE_SLT_tensor_min
	{"slt_tensor_min", do_slt_tensor_min, 0, "[SLT tensor_min test]"},
#endif
#ifdef BUILD_TEST_CASE_SLT_tensor_mul
	{"slt_tensor_mul", do_slt_tensor_mul, 0, "[SLT tensor_mul test]"},
#endif
#ifdef BUILD_TEST_CASE_SLT_tensor_mul_const
	{"slt_tensor_mul_const", do_slt_tensor_mul_const, 0, "[SLT tensor_mul_const test]"},
#endif
#ifdef BUILD_TEST_CASE_SLT_tensor_or
	{"slt_tensor_or", do_slt_tensor_or, 0, "[SLT tensor_or test]"},
#endif
#ifdef BUILD_TEST_CASE_SLT_tensor_sub
	{"slt_tensor_sub", do_slt_tensor_sub, 0, "[SLT tensor_sub test]"},
#endif
#ifdef BUILD_TEST_CASE_SLT_tensor_transfer
	{"slt_tensor_transfer", do_slt_tensor_transfer, 0, "[SLT tensor_transfer test]"},
#endif
#ifdef BUILD_TEST_CASE_SLT_tensor_xor
	{"slt_tensor_xor", do_slt_tensor_xor, 0, "[SLT tensor_xor test]"},
#endif
#ifdef BUILD_TEST_CASE_SLT_tg_copy_gdma
	{"slt_tg_copy_gdma", do_slt_tg_copy_gdma, 0, "[SLT tg_copy_gdma test]"},
#endif
#ifdef BUILD_TEST_CASE_SLT_tl_copy_gdma_fixed
	{"slt_tl_copy_gdma_fixed", do_slt_tl_copy_gdma_fixed, 0, "[SLT tl_copy_gdma_fixed test]"},
#endif
#ifdef BUILD_TEST_CASE_SLT_conv
	{"slt_conv", do_slt_conv, 0, "[SLT conv test]"},
#endif
#ifdef BUILD_TEST_CASE_SLT_fc
	{"slt_fc", do_slt_fc, 0, "[SLT fc test]"},
#endif
#ifdef BUILD_TEST_CASE_SLT_pooling
	{"slt_pooling", do_slt_pooling, 0, "[SLT pooling test]"},
#endif
#ifdef BUILD_TEST_CASE_SLT_matrix_mac_2
	{"slt_matrix_mac_2", do_slt_matrix_mac_2, 0, "[SLT matrix_mac_2 test]"},
#endif
#ifdef BUILD_TEST_CASE_SLT_winograd
	{"slt_winograd", do_slt_winograd, 0, "[SLT winograd test]"},
#endif
#ifdef BUILD_TEST_CASE_SLT_net_test
	{"slt_net_test", do_slt_net_test, 0, "[SLT net_test test]"},
#endif
	{NULL, NULL, 0, NULL}
};

int testcase_slt(void)
{
  int i;
  int ret;

  uartlog("%s\n", __func__);
	
  for(i = 0;i < ARRAY_SIZE(slt_test_cmd_list) - 1;i++) {
		command_add(&slt_test_cmd_list[i]);
	}
  
  do {
    ret = cli_simple_loop();
  } while (ret == 0);
  
  return 0;
}

module_testcase("0", testcase_slt);

#ifndef BUILD_TEST_CASE_all
int testcase_main(void)
{
  int ret = testcase_slt();
  return ret;
}
#endif
