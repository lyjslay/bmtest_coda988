#!/bin/bash
SCRIPT=$(readlink -f "$0")
SCRIPT_DIR=$(dirname "$SCRIPT")
export CHIP=Mars
cd "$SCRIPT_DIR/../test/venc/cnm_ref_sw"
echo " ########################"
echo " $SCRIPT_DIR"
echo " ########################"

# path=/media/cvitek/yijun.liu01/host-tools/gcc/Xuantie-900-gcc-elf-newlib-x86_64-V2.8.1/bin/
# CROSS_COMPILE=${path}/riscv64-unknown-elf-
# for cv182x eval
# path=/media/cvitek/yijun.liu01/cv182x/host-tools/gcc/arm-cvitek-linux-uclibcgnueabihf/bin
# CROSS_COMPILE=${path}/arm-cvitek-linux-uclibcgnueabihf-gcc


make -f Coda980Enc.mak clean
make -f Coda980Enc.mak
# make -f Wave420Enc.mak

if [ $? -ne 0 ]; then
    echo "coda980 lib compilation failed."
    exit 1
fi

echo " ########################"
echo " coda980 lib compilation success."
echo " ########################"
# cp release/libcnm_vcodec.a $SCRIPT_DIR/../test/venc/
# cp sample/coda988_test.h $SCRIPT_DIR/../test/venc/

# cd "$SCRIPT_DIR/../test/venc"
# echo "/media/cvitek/yijun.liu01/cv182x/host-tools/gcc/arm-cvitek-linux-uclibcgnueabihf/bin/arm-cvitek-linux-uclibcgnueabihf-gcc test_on_cv182x.c -I. coda988_test.h -L. libcnm_vcodec.a -o test_on_cv182x"
# /media/cvitek/yijun.liu01/cv182x/host-tools/gcc/arm-cvitek-linux-uclibcgnueabihf/bin/arm-cvitek-linux-uclibcgnueabihf-gcc test_on_cv182x.c -I. coda988_test.h -L. libcnm_vcodec.a -o test_on_cv182x
# if [ $? -ne 0 ]; then
#     echo "libcnm_vcodec test code on cv182x compilation failed."
#     exit 1
# fi
# echo " ########################"
# echo " libcnm_vcodec test code on cv182x compilation success."
# echo " ########################"

cp release/libcoda.a $SCRIPT_DIR/../test/venc/
cp sample/coda988_test.h $SCRIPT_DIR/../test/venc/
cd "$SCRIPT_DIR/.."

make clean; make TEST_CASE=venc RUN_ENV=DDR BOARD=ASIC NPU_EN=0 DEBUG=1 ARCH=RISCV RISCV_TYPE=C906B

if [ $? -ne 0 ]; then
    echo "venc fpga tgz file compilation failed."
    exit 1
fi

echo " ########################"
echo " venc fpga tgz file compilation success!!!"
echo " ########################"
