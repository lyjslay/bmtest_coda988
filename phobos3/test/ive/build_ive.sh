#! /usr/bin/env bash
set -euo pipefail
this_dir=$(dirname $(readlink -f ${BASH_SOURCE[0]}))

cd $this_dir/../

TEST_CASE="ive"

IVE_CASES="Add Canny GMM MagAndAng Shitomasi Xor And DMA GMM2 Map Sobel _16BitTo8Bit BgModel Dilate Hist NCC Sub CCL Erode Integ Or Thresh Filter LBP OrdStaFilter Thresh_S16 CSC FilterAndCSC LKOpticalFlow Resize Thresh_U16"
IVE_CASES="Add Canny GMM MagAndAng Shitomasi Xor And DMA GMM2 Map Sobel _16BitTo8Bit BgModel Dilate Hist NCC Sub CCL Erode Integ Or Thresh Filter LBP OrdStaFilter Thresh_S16 CSC FilterAndCSC Resize Thresh_U16"
#IVE_CASES="Shitomasi"
#IVE_CASES="Add"

# example build script
#COMMONFLAGS= CROSS_COMPILE= ./build_scripts/build_ive.sh
#
for i in $IVE_CASES; do
  SUB_TEST_CASE=$(basename $i)
  # hack for testcase that replace main function name
  #sed -z 's/int main/extern "C"{ void * __dso_handle = 0 ;}\n#include "module_testcase.h"\nint __main( int argc, const char** argv );\nint _main(void){return __main(0, NULL);}\nmodule_testcase("1", _main);\nint __main/g' -i test/${TEST_CASE}/${SUB_TEST_CASE}/main.cpp

  make clean;
  make TEST_CASE=${TEST_CASE} SUB_TEST_CASE=${SUB_TEST_CASE} RUN_ENV=DDR BOARD=FPGA
  #make TEST_CASE=${TEST_CASE} SUB_TEST_CASE=${SUB_TEST_CASE} BOARD=ASIC \
  #    LINK=/usr/lib/ldscripts/elf_x86_64.x \
  #    #RUN_ENV=DDR \

done
