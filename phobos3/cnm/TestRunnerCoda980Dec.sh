#!/bin/bash

source common.sh

# Global varaibles for decoder option
build_yuv=0
fail_stop=0
yuv_dir="./yuv"
stream_dir="./stream"
bin_dir="."
test_exec="./coda980_dec_test"
wiki_log_file="./log/dec_confluence.log"
enable_random=0;
md5_path=""
codec="avc"
jenkins=""
txt_param_switch=0
g_product_name="coda980"                      # This variable is used in common.sh
g_maptype_num=${#CODA980_MAPTYPE_NAME[@]}
g_enable_wtl=0
force_wtl=0
function build_golden_yuvs {
    local src=$1
    local ppu_opts=(0 0 90 0 180 0 270 0 0 1 90 1 180 1 270 1)

    g_enable_deblock=0
    g_enable_dering=0
    for i in 0 2 4 6 8 10 12 14; do
        g_rotAngle=${ppu_opts[$i+0]}
        g_mirDir=${ppu_opts[$i+1]}
        generate_golden_data_path $codec $streamset_path $src 0
        golden_yuv_path="$g_func_ret_str"
        echo "Generating yuv for rotation($g_rotAngle), g_mirDir($g_mirDir)..."
        generate_yuv $stream $golden_yuv_path
    done
    if [ $g_codec_index -eq 2 ] || [ $g_codec_index -eq 3 ]; then
        g_enable_deblock=1
        g_enable_dering=0
        for i in 0 2 4 6 8 10 12 14; do
            g_rotAngle=${ppu_opts[$i+0]}
            g_mirDir=${ppu_opts[$i+1]}
            generate_golden_data_path $codec $streamset_path $src 0
            golden_yuv_path="$g_func_ret_str"
            echo "Generating yuv for deblock($g_enable_deblock) dering($g_enable_dering) rotation($g_rotAngle), g_mirDir($g_mirDir)..."
            generate_yuv $stream $golden_yuv_path
        done
        g_enable_deblock=0
        g_enable_dering=1
        for i in 0 2 4 6 8 10 12 14; do
            g_rotAngle=${ppu_opts[$i+0]}
            g_mirDir=${ppu_opts[$i+1]}
            generate_golden_data_path $codec $streamset_path $src 0
            golden_yuv_path="$g_func_ret_str"
            echo "Generating yuv for deblock($g_enable_deblock) dering($g_enable_dering) rotation($g_rotAngle), g_mirDir($g_mirDir)..."
            generate_yuv $stream $golden_yuv_path
        done
        g_enable_deblock=1
        g_enable_dering=1
        for i in 0 2 4 6 8 10 12 14; do
            g_rotAngle=${ppu_opts[$i+0]}
            g_mirDir=${ppu_opts[$i+1]}
            generate_golden_data_path $codec $streamset_path $src 0
            golden_yuv_path="$g_func_ret_str"
            echo "Generating yuv for deblock($g_enable_deblock) dering($g_enable_dering) rotation($g_rotAngle), g_mirDir($g_mirDir)..."
            generate_yuv $stream $golden_yuv_path
        done
    fi
}

function help {
    echo ""
    echo "-------------------------------------------------------------------------------"
    echo "Chips&Media conformance Tool v2.0"
    echo "All copyright reserved by Chips&Media"
    echo "-------------------------------------------------------------------------------"
    echo "$0 OPTION streamlist_file"
    echo "-h              help"
    echo "-c              codec. (avc, avs, dv3, mp2, mp4, rvx, tho, vc1, vp8)"
    echo "-n num          number of frames."
    echo "--enable-mvc    0 - disable, 1 - enable. For testing H.264 MVC stream, this option should be 1."
    echo "--match-mode    0 - no match, 1 - prebuilt YUV match"
    echo "--mp4class      In case of MPEG-4 test. "
    echo "                0 - mpeg-4,  1 - divx, 2 - xvid, 5 - divx4,  256 - sorenson spark"
    echo "--bsmode        0 - ringbuffer, 1 - picend(use ffmpeg), default 0"
    echo "--bin-dir       ref-c directory"
    echo "--enable-random generate random option."
    echo "--stream-dir    stream directory"
    echo "--yuv-dir       Golden yuv directory"
    echo "--fail-stop     Stop testing if current test is failure"
    echo "--build-yuv     Just generate golden YUV with Ref-C."
}

################################################################################
# Parse arguments                                                              #
################################################################################
OPTSTRING="-o hc:n: -l bsmode:,enable-random,stream-dir:,bin-dir:,match-mode:" 
OPTSTRING="${OPTSTRING},mp4class:,enable-mvc,build-yuv,yuv-dir:,fail-stop"

OPTS=`getopt $OPTSTRING -- "$@"`

if [ $? -ne 0 ]; then
    exit 1
fi

eval set -- "$OPTS"

while true; do
    case "$1" in
        -h) 
            help
            exit 1
            shift;;
        -n) 
            g_frame_num="-n $2"
            shift 2;;
        -c)
            codec=$(echo $2 | tr '[:upper:]' '[:lower:]')
            shift 2;;
        --bsmode)
            g_bsmode_temp=$2
            g_bsmode_force=$2
            shift 2;;
        --bin-dir)
            bin_dir=$2
            shift 2;;
        --enable-random)
            enable_random=1
            shift;;
        --match-mode)
            g_match_mode=$2
            shift 2;;
        --stream-dir)
            stream_dir=$2
            shift 2;;
        --mp4class)
            g_mp4class=$2
            shift 2;;
        --enable-mvc)
            g_enable_mvc=1
            shift;;
        --build-yuv)
            build_yuv=1
            shift;;
        --yuv-dir)
            yuv_dir=$2
            shift 2;;
        --fail-stop)
            fail_stop=1
            shift;;
        --) 
            shift
            break;;
    esac
done

shift $(($OPTIND - 1))

case "${codec}" in
    "avc")  g_codec_index=0;;
    "vc1")  g_codec_index=1;;
    "mp2")  g_codec_index=2;;
    "mp4")  g_codec_index=3;;
    "h263") g_codec_index=4;;
    "dv3")  g_codec_index=5;;
    "rvx")  g_codec_index=6;;
    "avs")  g_codec_index=7;;
    "tho")  g_codec_index=9;;
    "vp8")  g_codec_index=11;;
    *)
        echo "unsupported codec: ${codec}"
        help
        exit 1
        ;;
esac

################################################################################
# Get param from target text file                                              #
################################################################################
name_value=()
input_param_name="TestRunnerParamCoda980Dec.txt"
if [ -f $input_param_name ] && [ $enable_random -eq 0 ]; then
    while read line; do
        # remove right comment
        line="${line%%\#*}"
        if [ "$line" = "" ]; then continue; fi

        OIFS=$IFS
        IFS='='
        count=0
        for word in $line; do
            word=${word// /}
            name_value[$count]="$word"
            count=$(($count+1))
        done
        IFS=$OIFS
        attr="${name_value[0]}"
        value="${name_value[1]}"

        case "$attr" in
            default)              default_opt="$value";;
            n)                    g_frame_num="-n $value";;
            secondary-axi)        secondary_axi_temp="$value";;
            stream-endian)        stream_endian_temp="$value";;
            frame-endian)         frame_endian_temp="$value";;
            bsmode)               bsmode_temp="$value";;
            match-mode)           match_mode_temp="$value";;
            cbcr_interleave_mode) yuv_mode_temp="$value";;
            enable_wtl)           enable_wtl_temp="$value";;
            map_type)             maptype_temp="$value";;
            enable_tiled2linear)  tiled2linear_temp="$value";;
            rotation)             rotation_temp="$value";;
            mirror)               mirror_temp="$value";;
            dering)               dering_temp="$value";;
            deblock)              deblock_temp="$value";;
            txt-param)            txt_param_switch=1;;
            *) ;;
        esac
    done < $input_param_name
else
    if [ ! -f $input_param_name ]; then
        echo "$input_param_name file doesn't exist"; 
    fi
fi

streamset_path=$1
streamset_file=`basename ${streamset_path}`

if [ ! -e "${streamset_path}" ]; then
    echo "No such file: ${streamset_path}"
    exit 1
fi

################################################################################
# count stream number                                                          #
################################################################################
stream_file_array=()
parse_streamset_file $streamset_path stream_file_array

num_of_streams=${#stream_file_array[@]}

#echo ${stream_file_array[@]}

count=1
success_count=0
failure_count=0
remain_count=${num_of_streams}
basedir=${basedir//\/}  # remove last delimeter

mkdir -p temp
mkdir -p output
mkdir -p log/decoder_conformance
conf_log_path="log/decoder_conformance/${streamset_file}_r${g_revision}.log"
conf_err_log_path="log/decoder_conformance/${streamset_file}_error_r${g_revision}.log"
temp_log_path="./temp/temp.log"
# truncate contents of log file
echo "" > $conf_log_path
echo "" > $conf_err_log_path
echo "" > $temp_log_path
beginTime=$(date +%s%N)

echo ""
echo ""
echo "##### PRESS 'q' for stop #####"
echo ""
echo ""

log_conf "STREAM BASE DIR: ${stream_dir}" ${conf_log_path}

################################################################################
# read cmd file.                                                               #
################################################################################
for (( c=0; c< $num_of_streams ; c++ )) do
    line=${stream_file_array[$c]}
    # press 'q' for stop
    read -t 1 -n 1 key
    if [[ $key == q ]]; then
        break;
    fi

    test_param=""
    stream="${stream_dir}/${line}"
    log_conf "--------------------------------------------------------------------------------" ${temp_log_path}
    log_conf "[${count}/${num_of_streams}] ${stream}" ${temp_log_path}
    log_conf "--------------------------------------------------------------------------------" ${temp_log_path}

    if [ ! -f $stream ]; then
        log_conf "Not found $stream" $temp_log_path
        log_conf "[RESULT] FAILURE" $temp_log_path
        echo "Not found $stream" >> $conf_err_log_path
        echo "[RESULT] FAILURE" >> $conf_err_log_path
        cat $temp_log_path >> $conf_log_path
        failure_count=$(($failure_count + 1))
        remain_count=$(($remain_count - 1))
        count=$(($count + 1))
        continue
    fi

    if [ $build_yuv -eq 1 ]; then
        build_golden_yuvs $stream
        remain_count=$(($remain_count - 1))
        count=$(($count + 1))
        continue
    fi

    result=0
    target_stream=$stream

################################################################################
# make argc & argv parameter                                                   #
################################################################################
    if [ $result -eq 0 ]; then
        if [ $enable_random -eq 1 ] && [ $txt_param_switch -eq 0 ]; then
            get_random_param 1 coda9
        else
            get_default_param 1 coda9
        fi
        # doesn't support NV21.
        g_enable_nv21=0
        # NPF
        if [ $g_enable_npftest -eq 1 ]; then
            g_rotAngle=0
            g_mirDir=0
        fi

        if [ $g_enable_wtl -eq 1 ]; then
            g_enable_deblock=0
            g_enable_dering=0
            g_rotAngle=0
            g_mirDir=0
        fi

        if [ $force_wtl -eq 1 ]; then
            echo "forced enable-wtl"
            g_enable_wtl=1
            g_enable_deblock=0
            g_enable_dering=0
            g_rotAngle=0
            g_mirDir=0
            g_enable_tiled2linear=0
        fi
        build_test_param 1 
        test_param="$test_param $g_func_ret_str"

################################################################################
# make prebuilt stream                                                         #
################################################################################

        output_filename="$(basename $line).yuv"

        case $g_match_mode in
            1)  generate_golden_data_path $codec $streamset_path $stream 
                golden_yuv_path="$g_func_ret_str"
                test_param="$test_param --ref-yuv=$golden_yuv_path";;
        esac

################################################################################
# print information                                                            #
################################################################################

        if [ $result -eq 0 ]; then
            log_conf "RANDOM TEST    : ${ON_OFF[$enable_random]}" $temp_log_path
            log_conf "BITSTREAM MODE : ${DEC_BSMODE_NAME[$g_bsmode]}" $temp_log_path
            log_conf "ENDIAN         : STREAM($g_stream_endian) FRAME($g_frame_endian)" $temp_log_path
            log_conf "STANDARD       : ${CODEC_NAME[$g_codec_index]}" $temp_log_path
            log_conf "REORDER        : ON" $temp_log_path
            log_conf "MAP TYPE       : ${CODA980_MAPTYPE_NAME[$g_maptype_index]}" $temp_log_path
            log_conf "TILED2LINEAR   : ${ON_OFF[$g_enable_tiled2linear]}" $temp_log_path
            log_conf "ROTATE         : $g_rotAngle" $temp_log_path
            log_conf "MIRROR         : ${MIRROR_NAME[$g_mirDir]}" $temp_log_path
            log_conf "DERING(MP2/4)  : ${ON_OFF[$g_enable_dering]}" $temp_log_path
            log_conf "DEBLOCK(MP2/4) : ${ON_OFF[$g_enable_deblock]}" $temp_log_path
            log_conf "WTL MODE       : ${ON_OFF[$g_enable_wtl]}" $temp_log_path
            log_conf "CBCR interleave: ${ON_OFF[$g_cbcr_interleave]}" $temp_log_path
            log_conf "NV21           : ${ON_OFF[$g_enable_nv21]}" $temp_log_path
            log_conf "Secondary AXI  : ${g_secondary_axi}" $temp_log_path

            test_param_print=$test_param
            log_conf "Unix   : $test_exec $test_param_print --input=$stream" $temp_log_path
            #winexec=$(echo "$test_exec $test_param_print --input=$stream" | sed -e 's/\//\\/g')
            #log_conf "Windows: $winexec" $temp_log_path
            cat $temp_log_path >> $conf_log_path

            chmod 777 $test_exec
            test_exec_param="$test_exec $test_param --input=$target_stream"
            $test_exec_param || result=$?

            #"gdb $test_exec"
        fi
    else
        cat $temp_log_path >> $conf_log_path
    fi

    if [ $result -eq 0 ]; then
        log_conf "[RESULT] SUCCESS" $conf_log_path
        success_count=$(($success_count + 1))
    else
        cat ./ErrorLog.txt >> $conf_log_path
        log_conf "[RESULT] FAILURE" $conf_log_path
        failure_count=$(($failure_count + 1))
        cat $temp_log_path >> $conf_err_log_path
        cat ./ErrorLog.txt >> $conf_err_log_path
        echo "[RESULT] FAILURE" >> $conf_err_log_path
        if [ $result -eq 10 ]; then
            echo "Abnormal exit!!!"
            break
        fi
    fi
    remain_count=$(($remain_count - 1))
    count=$(($count + 1))
    # clear temp log
    echo "" > $temp_log_path
   # delete temporal input file
    rm -f $g_copy_stream   
    if [ $fail_stop -eq 1 ] && [ $result -ne 0 ]; then
        break;
    fi
done 

if [ $build_yuv -eq 1 ]; then
    exit 0
fi

endTime=$(date +%s%N)
elapsed=$((($endTime - $beginTime) / 1000000000))
elapsedH=$(($elapsed / 3600))
elapsedS=$(($elapsed % 60))
elapsedM=$(((($elapsed - $elapsedS) / 60) % 60))
if [ "$((elapsedS / 10))" == "0" ]; then elapsedS="0${elapsedS}" ;fi
if [ "$((elapsedM / 10))" == "0" ]; then elapsedM="0${elapsedM}" ;fi
if [ "$((elapsedH / 10))" == "0" ]; then elapsedH="0${elapsedH}" ;fi

if [ $elapsed -le 60 ]; then
    time_hms="{color:red}*${elapsedH}:${elapsedM}:${elapsedS}*{color}"
else
    time_hms="${elapsedH}:${elapsedM}:${elapsedS}"
fi

log_filename=$(basename $conf_log_path)
log_err_filename=$(basename $conf_err_log_path)
if [ $failure_count == 0 ] && [ $num_of_streams != 0 ]; then
    pass=${PASS}
    rm $conf_err_log_path
    log_err_filename=""
else
    pass=${FAIL}
fi

wiki_log="| $streamset_file | $num_of_streams | $success_count | $failure_count | $remain_count | $log_filename | ${log_err_filename} | $pass | $time_hms | | | |"

echo $wiki_log
echo $wiki_log >> $wiki_log_file

if [ "$num_of_streams" == "0" ]; then
    echo "num_of_streams: $num_of_streams = exit 1"
    exit 1
fi

exit $failure_count
 
