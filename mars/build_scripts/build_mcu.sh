#!/bin/bash

set -euo pipefail

/bin/rm -f case1.bin
/bin/rm -f bank.bin
touch case1.bin
touch bank.bin

if [ -e ./test/mcu/mcu51/Objects/case1.hex ]; then
        echo "Generate case1"
        srec_cat -Disable_Sequence_Warnings \
                ./test/mcu/mcu51/Objects/case1.hex -Intel -o ./case1.bin -Binary || exit 1
fi

make clean
# make TEST_CASE=mcu RUN_ENV=DDR BOARD= NPU_EN=0 DEBUG=1
make TEST_CASE=mcu RUN_ENV=DDR BOARD=FPGA DEBUG=1 ARCH=RISCV RISCV_TYPE=C906B
ls -l case1.bin bank.bin
