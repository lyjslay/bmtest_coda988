#!/bin/bash

# define color
RED='\033[0;31m'
GREEN='\033[0;32m'
NC='\033[0m' # No Color

TOP_DIR=`pwd`
CODE_DIR=$TOP_DIR/../code_dir
INI2_CFG_FILE_NAME=register.cfg
OUT_H_FILE=test_rtl_reg.h

DE_CASE=$TOP_DIR/../register_from_de
REG_CFG_TOOL=$TOP_DIR/ini2code/run_para_parsing.pl
REG_CFG_LIST=$TOP_DIR/ini2code/register.list
REG_FM=$TOP_DIR/ini2code/register.fm
PY_TOOL=$TOP_DIR/ini2code/test_cft2header_v2.py

REG_NAME=reg
FISRT_INI=isp.ini
DEBUG=1

function printf_debug() {
	if [ $DEBUG -eq 1 ]
	then
		printf "$GREEN[DEBUG]$NC${1}"
	fi
}

function PUSHD() {
	if [ $DEBUG -eq 1 ]
	then
		printf_debug "\t\t\tENTER $1 \t\t\t\n"
		pushd ${1}
	else
		pushd ${1}
	fi
}

function POPD() {
	if [ $DEBUG -eq 1 ]
	then
		printf_debug "\t\t\tEXIT $1 \t\t\t\n"
		popd
	else
		popd
	fi
}

# 函数：对list文件进行排序
# 顺序：无后缀 -> _tile_0 -> postraw_done -> _tile_1
function sort_list_files() {
	local list_files=("$@")
	local no_suffix=()
	local tile_0=()
	local postraw_done=()
	local tile_1=()
	local others=()

	for file in "${list_files[@]}"; do
		local basename=$(basename "$file" .list)
		if [[ "$basename" == *"_tile_0" ]]; then
			tile_0+=("$file")
		elif [[ "$basename" == *"postraw_done"* ]] || [[ "$basename" == *"post_raw_done"* ]]; then
			postraw_done+=("$file")
		elif [[ "$basename" == *"_tile_1" ]]; then
			tile_1+=("$file")
		elif [[ "$basename" != *"_tile_"* ]] && [[ "$basename" != *"postraw_done"* ]] && [[ "$basename" != *"post_raw_done"* ]]; then
			no_suffix+=("$file")
		else
			others+=("$file")
		fi
	done

	# 按顺序合并
	sorted_files=("${no_suffix[@]}" "${tile_0[@]}" "${postraw_done[@]}" "${tile_1[@]}" "${others[@]}")
	printf '%s\n' "${sorted_files[@]}"
}

function ini2cfg_func() {
	local target_dir="$CODE_DIR/${1}/reg_cfg"
	PUSHD $REG_NAME

	#check source isp.ini file
	if test -e $FISRT_INI
	then
		printf "$GREEN [found $FISRT_INI] $NC\n"
	else
		printf "$RED [ERROR $FISRT_INI not found not need isp ini] $NC\n"
	fi

	#check dst file
	if test -e $target_dir/reg_common.h
	then
		rm $target_dir/reg_common.h
	fi
	touch $target_dir/reg_common.h

	#iter source ini_frame
	for ini_frame in `ls ini_frame_* -d`
	do
		#combine ini
		printf "$GREEN [found $ini_frame] $NC\n"

		if test -e $target_dir/$ini_frame
		then
			rm $target_dir/$ini_frame -rf
		fi
		mkdir -p $target_dir/$ini_frame/

		# 检查是否存在list文件
		local list_files=()
		if ls $ini_frame/*.list 1> /dev/null 2>&1; then
			# 找到所有list文件并排序
			for list_file in `ls $ini_frame/*.list`; do
				list_files+=("$list_file")
			done

			# 按顺序排序list文件
			local sorted_lists=$(sort_list_files "${list_files[@]}")

			printf "$GREEN [Processing list files in order] $NC\n"
			for list_file in $sorted_lists; do
				printf "$GREEN   -> $list_file $NC\n"
			done

			# 为每个list文件生成独立的.h文件
			for list_file in $sorted_lists; do
				local list_basename=$(basename "$list_file" .list)
				printf "$GREEN [Processing list: $list_file -> $list_basename.h] $NC\n"

				# 创建临时目录用于处理这个list文件
				local temp_dir=$target_dir/$ini_frame/$list_basename
				mkdir -p $temp_dir

				# 复制list文件到临时目录，并修改其中的路径
				# 因为ini文件在源目录，需要创建相对路径或绝对路径
				local temp_list=$temp_dir/register.list
				rm -f $temp_list
				touch $temp_list

				# 读取源list文件，将ini文件路径转换为相对于临时目录的路径
				# 或者使用绝对路径指向源目录
				while IFS= read -r line || [ -n "$line" ]; do
					line=$(echo "$line" | sed 's/^[[:space:]]*//;s/[[:space:]]*$//')
					if [ -n "$line" ] && [[ ! "$line" =~ ^# ]]; then
						# 检查ini文件是否存在（在源目录）
						local ini_path="$ini_frame/$line"
						if [ -f "$ini_path" ]; then
							# 使用绝对路径或相对路径指向源目录
							# 方案1：使用绝对路径（需要知道当前工作目录）
							# 方案2：复制ini文件到临时目录
							# 方案3：在临时目录中创建符号链接
							# 这里使用方案2：复制ini文件
							local ini_basename=$(basename "$line")
							cp "$ini_path" $temp_dir/$ini_basename
							echo "$ini_basename" >> $temp_list
						else
							printf "$RED [WARNING: ini file not found: $ini_path] $NC\n"
						fi
					fi
				done < "$list_file"

				# 复制工具文件
				cp $REG_CFG_TOOL $temp_dir/
				cp $REG_FM $temp_dir/

				# 进入临时目录处理
				PUSHD $temp_dir

				# 生成register.cfg
				if [ $DEBUG -eq 1 ]
				then
					printf_debug "$REG_CFG_TOOL register.list $REG_FM \n"
					$REG_CFG_TOOL register.list $REG_FM
				else
					$REG_CFG_TOOL register.list $REG_FM > /dev/null 2>&1
				fi

				# 生成.h文件
				if [ -f "$INI2_CFG_FILE_NAME" ]; then
					if [ $DEBUG -eq 1 ]
					then
						printf_debug "python $PY_TOOL $temp_dir/$INI2_CFG_FILE_NAME $list_basename.h\n"
						python $PY_TOOL $temp_dir/$INI2_CFG_FILE_NAME $list_basename.h
					else
						python $PY_TOOL $temp_dir/$INI2_CFG_FILE_NAME $list_basename.h > /dev/null 2>&1
					fi

					# 将生成的.h文件移动到ini_frame目录
					if [ -f "$list_basename.h" ]; then
						mv $list_basename.h $target_dir/$ini_frame/
						printf "$GREEN [Generated: $target_dir/$ini_frame/$list_basename.h] $NC\n"
					fi
				else
					printf "$RED [ERROR: Failed to generate register.cfg for $list_file] $NC\n"
				fi

				POPD $temp_dir

				# 清理临时目录
				rm -rf $temp_dir

				# 在reg_common.h中添加include
				echo "#include <$ini_frame/$list_basename.h>" >> $target_dir/reg_common.h
			done

			# 处理.lut文件（如果有，可以单独处理或合并到某个阶段）
			if ls $ini_frame/*.lut 1> /dev/null 2>&1; then
				printf "$GREEN [Found .lut files, will be processed separately] $NC\n"
				# LUT文件会在run_para_parsing.pl中自动处理
			fi
		else
			# 如果没有list文件，使用原来的方式（合并所有ini文件）
			touch $target_dir/$ini_frame/register.ini

			if [ -f "$FISRT_INI" ]; then
				cat $FISRT_INI >> $target_dir/$ini_frame/register.ini
			fi

			for ini_ip in `ls $ini_frame/*.ini 2>/dev/null`
			do
				printf_debug "$ini_ip >> $target_dir/$ini_frame/register.ini\n"
				cat $ini_ip >> $target_dir/$ini_frame/register.ini
			done

			# 创建register.list指向register.ini
			echo "register.ini" > $target_dir/$ini_frame/register.list

			#enter dst dir, gen register.cfg, reg_tool/reg_list/reg_fm must under the same dir
			local target_reg_tool=$target_dir/$ini_frame/run_para_parsing.pl
			local target_reg_list=$target_dir/$ini_frame/register.list
			local target_reg_fm=$target_dir/$ini_frame/register.fm

			cp $REG_CFG_TOOL $target_dir/$ini_frame/
			cp $REG_FM $target_dir/$ini_frame/

			PUSHD $target_dir/$ini_frame

			if [ $DEBUG -eq 1 ]
			then
				printf_debug "$target_reg_tool $target_reg_list $REG_FM \n"
				$target_reg_tool $target_reg_list $REG_FM

				printf_debug "python $PY_TOOL $target_dir/$ini_frame/$INI2_CFG_FILE_NAME $OUT_H_FILE\n"
				python $PY_TOOL $target_dir/$ini_frame/$INI2_CFG_FILE_NAME $ini_frame.h
			else
				$target_reg_tool $target_reg_list $REG_FM > /dev/null 2>&1

				python $PY_TOOL $target_dir/$ini_frame/$INI2_CFG_FILE_NAME $ini_frame.h > /dev/null 2>&1
			fi

			POPD $target_dir/$ini_frame

			echo "#include <$ini_frame/$ini_frame.h>" >> $target_dir/reg_common.h

			rm -f $target_reg_tool
			rm -f $target_reg_fm
		fi
	done

	if [ $? -ne 0 ]
	then
		printf "$RED [ini2code fail] $NC\n"
	fi

	POPD  $REG_NAME
}

function iter_all_case() {
	printf_debug "$ini_ip iter_all_case\n"

	PUSHD $DE_CASE

	for case_dir in `ls`
	do
		if test -d $case_dir
		then
			PUSHD $case_dir

			ini2cfg_func $case_dir

			POPD $case_dir
		else
			printf "$RED [$case_dir not a file, skip it] $NC\n"
		fi
	done

	POPD $DE_CASE
}

if [ $# -eq 0 ]
then
	iter_all_case
else
	if test -d $TOP_DIR/$1
	then
		PUSHD $TOP_DIR/$1
		printf "$GREEN [enter $TOP_DIR/$1] $NC\n"

		if test -d $TOP_DIR/$1
		then
			ini2cfg_func $(basename $1)
		else
			printf "$RED [$1 not a file, skip it] $NC\n"
		fi

		POPD $TOP_DIR/$1
	else
		printf "$RED [$1 not a file, skip it] $NC\n"
	fi
fi