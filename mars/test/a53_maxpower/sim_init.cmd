#!/bin/bash


pushd ..

echo "Start building A53 max power test."
LD_LIBRARY_PATH=/opt/glibc-2.14/lib:$LD_LIBRARY_PATH make clean
LD_LIBRARY_PATH=/opt/glibc-2.14/lib:$LD_LIBRARY_PATH make

if [ ! -d "a53_build" ]; then
    mkdir a53_build
fi

cp aarch64/max_power/max_power.vhx a53_build/rom.hex
cp aarch64/max_power/max_power.dump a53_build/rom.dump
cp aarch64/max_power/max_power.elf a53_build/rom.elf
cp aarch64/max_power/max_power.bin a53_build/rom.bin

echo "Finish building A53 max power test."

popd


