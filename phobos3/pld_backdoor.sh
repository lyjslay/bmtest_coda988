#!/bin/bash

BMTEST_BIN=./out/bmtest.bin
BMTEST_ADDR=0x80000000

mkdir -p backdoor/c_build/ddr
mkdir -p backdoor/script

python3 real_ddr_axi2dram.py ${BMTEST_BIN} ${BMTEST_ADDR}
mv *.text ./backdoor/c_build/ddr/
mv *.tcl ./backdoor/script/

tar zcvf backdoor.tar.gz ./backdoor
