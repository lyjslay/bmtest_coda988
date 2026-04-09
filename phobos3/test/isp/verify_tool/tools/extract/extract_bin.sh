#!/bin/bash
j=$1
USER=$USER

for ((i = 0; i < j; i++))
do
   idx=`printf "%03d" $i`
   ./extract ./result_y_$idx.ppm  result_y_$i.bin 3
   ./extract ./result_u_$idx.ppm  result_u_$i.bin 1
   ./extract ./result_v_$idx.ppm  result_v_$i.bin 1
   ./combine result_v_$i.bin result_u_$i.bin result_nv21_$i.bin 
   cat result_y_$i.bin >> result_target_nv21_$i.bin
   cat result_nv21_$i.bin >> result_target_nv21_$i.bin
done
