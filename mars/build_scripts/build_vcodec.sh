#!/bin/bash

# ENC_TYPE=h265, h264
# BOARD=FPGA, PALLADIUM, ASIC
# SLT_TEST
#	- slt, slt test
#	- else, normal test
ENC_TYPE=h265
BOARD=FPGA

show_help()
{
    echo "usage"
    echo "// ----------------------------- //"
    echo "-h : show help"
    echo "-b : BOARD = FPGA, PALLADIUM, ASIC"
    echo "     [default = FPGA]"
    echo "-e : ENC_TYPE = h264, h265"
    echo "     [default = h265]"
    echo "ex :"
    echo "     ./build_scripts/build_vcodec.sh -h"
    echo "     ./build_scripts/build_vcodec.sh -e h265 -b FPGA"
    echo "     ./build_scripts/build_vcodec.sh -e h265 -b PALLADIUM"
}

#    # A POSIX variable
OPTIND=1         # Reset in case getopts has been used previously in the shell.

while getopts "h?b:e:" opt; do
    case "$opt" in
    h|\?)
        show_help
        exit 0
        ;;
    b)  BOARD=$OPTARG
        ;;
    e)  ENC_TYPE=$OPTARG
        ;;
    esac
done

shift $((OPTIND-1))

[ "${1:-}" = "--" ] && shift

ROOT=$(pwd)
VENC_DIR=${ROOT}/test/venc
if [[ "$ENC_TYPE" == 'h265' ]]; then
    VENC_LIB="$VENC_DIR/libcvi_vcodec.a"
    VENC_LIB_PROJ_DIR=$VENC_DIR/vcodec
else
    VENC_LIB="$VENC_DIR/libcvi_"$ENC_TYPE"_venc.a"
    VENC_LIB_PROJ_DIR=$VENC_DIR/$ENC_TYPE
fi

make clean
#rm "$VENC_LIB"

for curr_lib in libcvi_h264_venc.a libcvi_vcodec.a
do
    echo "curr_lib = $curr_lib"

    if [ ! -f $VENC_DIR/$curr_lib ]; then

        if [[ "$curr_lib" == 'libcvi_vcodec.a' ]]; then
            pushd $VENC_LIB_PROJ_DIR
            make -f Wave420Enc.mak clean
            make -f Wave420Enc.mak $curr_lib
            cp -f $VENC_DIR/vcodec/$curr_lib $VENC_DIR
            cp -f $VENC_DIR/vcodec/sample/main_enc_cvitest.h $VENC_DIR
            popd
        fi

        if [[ "$curr_lib" == 'libcvi_h264_venc.a' ]]; then
            pushd $VENC_DIR/h264
            make -f Coda980Enc.mak clean
            make -f Coda980Enc.mak $curr_lib
            cp -f $VENC_DIR/h264/$curr_lib $VENC_DIR
            popd
        fi

    fi
done
#
if [ -f $VENC_LIB ]; then
    make TEST_CASE=venc BOARD=$BOARD RUN_ENV=DDR DEBUG=1 SLT_TEST= ENC_TYPE=$ENC_TYPE
else
    echo "$VENC_LIB is necessary"
fi
