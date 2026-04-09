#! /usr/bin/env bash
set -euo pipefail
this_dir=$(dirname $(readlink -f ${BASH_SOURCE[0]}))

cd $this_dir

NPU_CASES="get_matrix_stride get_matrix_tp get_tensor_stride get_tensor_stride_unalign get_tensor_tp lut matrix_transfer put_matrix_stride put_matrix_tp put_tensor_stride put_tensor_stride_unalign put_tensor_tp put_tensor_tp_unalign put_tensor_unalign relu tensor_add tensor_and tensor_arith_shift tensor_copy tensor_mac tensor_mac_const tensor_max tensor_mdsum tensor_min tensor_mul tensor_mul_const tensor_or tensor_sub tensor_transfer tensor_xor tg_copy_gdma tl_copy_gdma_fixed conv fc pooling matrix_mac_2 winograd"

make clean
for i in $NPU_CASES; do 
  make TEST_CASE=$(basename $i) BOARD=ASIC RUN_ENV=DDR
done
