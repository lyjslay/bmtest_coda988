#!/bin/bash
j=$1
for ((i = 0; i < j; i++))
do
   idx=`printf "%03d" $i`
  ./ppm2raw image_input_le_$idx.ppm image_input_le_$idx.bin 128 64 12
  ./ppm2raw image_input_se_$idx.ppm image_input_se_$idx.bin 128 64 12
  hexdump -ve '"%08x\n"' image_input_le_$idx.bin > image_input_le_$idx.bin.text
  hexdump -ve '"%08x\n"' image_input_se_$idx.bin > image_input_se_$idx.bin.text
done
