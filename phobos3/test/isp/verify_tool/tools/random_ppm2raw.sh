#!/bin/bash

# define color
RED='\033[0;31m'
GREEN='\033[0;32m'
NC='\033[0m' # No Color

TOP_DIR=`pwd`
PPM2RAW=$TOP_DIR/ppm2raw/ppm2raw
EXTRACT=$TOP_DIR/extract/extract
COMBINE=$TOP_DIR/extract/combine
PLD_DIR=$TOP_DIR/../to_pld
DE_CASE=$TOP_DIR/../register_from_de

PPM_SRC_NAME=ppm_src
PPM_GOLDEN_NAME=ppm_golden_yuv420

function ppm2raw_func() {
	local targer_dir="$PLD_DIR/${1}/raw_src"
	mkdir -p $targer_dir

	#ppm2raw find input ppm
	pushd $PPM_SRC_NAME
	for ppm_src in `ls *[ls]e*.ppm`
	do
		echo $PPM2RAW $ppm_src $targer_dir/$ppm_src.bin 128 64 12
		$PPM2RAW $ppm_src  $targer_dir/$ppm_src.bin 128 64 12
		hexdump -ve '"%08x\n"' $targer_dir/$ppm_src.bin > $targer_dir/$ppm_src.bin.text
	done

	if [ $? -ne 0 ]
	then
		printf "$RED [ppm2raw fail] $NC\n"
	fi

	popd
}

function ppm2nv21_func() {

	pushd $PPM_GOLDEN_NAME

	local targer_dir="$PLD_DIR/${1}/nv21_golden/"
	mkdir -p $targer_dir

	for result in `ls result_*.ppm`
	do
		#find result result_y*.ppm
		test=`echo $result | grep _y_`
		if [[ "$test" != "" ]];
		then
			echo $EXTRACT $result $targer_dir/$result.bin 3
			$EXTRACT $result $targer_dir/$result.bin 3
		else
			echo $EXTRACT $result $targer_dir/$result.bin 1
			$EXTRACT $result $targer_dir/$result.bin 1
		fi
	done

	result_sum=`ls $targer_dir/*_y_*.bin | wc -l`	
	for ((i = 0; i < result_sum; i++))
	do
		idx=`printf "%03d" $i`
		$COMBINE $targer_dir/result_v_$idx.ppm.bin $targer_dir/result_u_$idx.ppm.bin $targer_dir/result_nv21_$idx.bin
		cat $targer_dir/result_y_$idx.ppm.bin >> $targer_dir/result_target_nv21_$idx.bin
		cat $targer_dir/result_nv21_$idx.bin >> $targer_dir/result_target_nv21_$idx.bin
	done

	if [ $? -ne 0 ]
	then
		printf "$RED [ppm2nv21 fail] $NC\n"
	fi
	popd
}

function iter_all_case() {
	pushd $DE_CASE
	printf "$GREEN [enter $DE_CASE] $NC\n"

	for case_dir in `ls`
	do
		if test -d $case_dir
		then
			pushd $case_dir
			printf "$GREEN [enter $case_dir] $NC\n"

			ppm2raw_func $case_dir
			ppm2nv21_func $case_dir

			printf "$GREEN [exit $case_dir] $NC\n"
			popd
		else
			printf "$RED [$case_dir not a file, skip it] $NC\n"
		fi
	done

	printf "$GREEN [exit $DE_CASE] $NC\n"
	popd
}

if [ $# -eq 0 ]
then
	iter_all_case
else
	if test -d $TOP_DIR/$1
	then
		pushd $TOP_DIR/$1

		printf "$GREEN [enter $TOP_DIR/$1] $NC\n"

		if test -d $TOP_DIR/$1
		then
			ppm2raw_func $(basename $1)
			ppm2nv21_func $(basename $1)
		else
			printf "$RED [$1 not a file, skip it] $NC\n"
		fi

		popd
	else
		printf "$RED [$1 not a file, skip it] $NC\n"
	fi
fi