#!/bin/bash

TOP_DIR=`pwd`
PLD_DDR_MEMTOOL=$TOP_DIR/pld2ddr/athena2_pld_ddr_mem_tool

echo output_addr $1 input_le_addr $2 input_se_addr $3

if [[ "$1" == "" ]] || [[ "$2" == "" ]];
then
	echo "[ERROR]##############must output_addr/set le_addr##################"
	echo "Usage ./ppm2addr.sh output_addr input_le_addr input_se_addr"
else
	pushd to_pld
	echo "Enter dir: to_pld"
	for dir in `ls`
	do
		echo "gen load file!"
		pushd $dir
		echo "Enter dir: $dir"
		for src_file in `ls *[ls]*.bin`
		do
			size=`ls -l $src_file | awk '{printf $5}'`
			target=${src_file%%.*}
			rm $target -rf
			mkdir -p $target
			test_le=`echo $src_file | grep le`
			test_se=`echo $src_file | grep se`
			if [[ "$test_le" != "" ]];
			then
				echo $PLD_DDR_MEMTOOL 0 $src_file $size $target $2 ./ 1
				$PLD_DDR_MEMTOOL 0 $src_file $size $target $2 ./ 1
				sed -i "1,8 s/.\//..\/to_pld\/$dir\/$target\//g" reload_ddr.tcl
				mv reload_ddr.tcl reload_load_ddr_$target.tcl
				mv *.h $target
			elif [[ "$test_se" != "" ]]; then
				if [[ "$3" == "" ]];
				then
					echo "[ERROR]##############must set se addr##################"
				else
					echo $PLD_DDR_MEMTOOL 0 $src_file $size $target $3 ./ 1
					$PLD_DDR_MEMTOOL 0 $src_file $size $target $3 ./ 1
					sed -i "1,8 s/.\//..\/to_pld\/$dir\/$target\//g" reload_ddr.tcl
					mv reload_ddr.tcl reload_load_ddr_$target.tcl
					mv *.h $target
				fi
			else
				#for result.bin
				mkdir ./$target/regression -p
				echo $PLD_DDR_MEMTOOL 0 $src_file $size $target $1 ./ 1
				$PLD_DDR_MEMTOOL 0 $src_file $size $target $1 ./ 1
				echo "combine file cmd"
				echo "$PLD_DDR_MEMTOOL 1 $target $size out_$src_file $1 ./ 1" > ./$target/regression/combine.log
				sed -i "1,8 s/load/dump/g" reload_ddr.tcl
				sed -i "1,8 s/.\//..\/to_pld\/$dir\/$target\/regression\//g" reload_ddr.tcl
				mv reload_ddr.tcl reload_dump_ddr_$target.tcl
				mv *.h $target
			fi
		done
		popd
	done
	popd
fi

