#!/bin/bash
j=$1
for ((i = 0; i < j; i++))
do
   idx=`printf "%03d" $i`
  ./ppm5_gen ../image_input_le_$idx.bin image_input_le_$idx.ppm 128 64 12 
  ./ppm5_gen ../image_input_se_$idx.bin image_input_se_$idx.ppm 128 64 12 
done
