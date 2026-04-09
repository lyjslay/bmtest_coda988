#!/bin/bash

# BOARD=FPGA, ASIC
# SLT_TEST
#	- slt, slt test
#	- else, normal test
DEC_TYPE=h265
BOARD=FPGA

show_help()
{
    echo "usage"
    echo "// ----------------------------- //"
    echo "-h : show help"
    echo "-b : BOARD = FPGA, PALLADIUM, ASIC, default = FPGA"
    echo "     [default = FPGA]"
    echo "-e : DEC_TYPE = h264, h265, default = h264"
    echo "     [default = h264]"
}

while getopts "h?b:e:" opt; do
    case "$opt" in
    h|\?)
        show_help
        exit 0
        ;;
    b)  BOARD=$OPTARG
        ;;
    e)  DEC_TYPE=$OPTARG
        ;;
    esac
done

shift $((OPTIND-1))

[ "${1:-}" = "--" ] && shift

ROOT=$(pwd)
VDEC_DIR=${ROOT}/test/vdec
VDEC_LIB="$VDEC_DIR/libcvi_"$DEC_TYPE"_vdec.a"
VDEC_LIB_PROJ_DIR=$VDEC_DIR/$DEC_TYPE

make clean
#rm "$VDEC_LIB"

for curr_lib in libcvi_h264_vdec.a libcvi_h265_vdec.a
do
    echo "curr_lib = $curr_lib"

    if [ ! -f $VDEC_DIR/$curr_lib ]; then

        if [[ "$curr_lib" == 'libcvi_h265_vdec.a' ]]; then
            pushd $VDEC_DIR/h265
            make -f Wave420Dec.mak clean
            make -f Wave420Dec.mak $curr_lib
            cp -f $VDEC_DIR/h265/$curr_lib $VDEC_DIR
            popd
        fi

        if [[ "$curr_lib" == 'libcvi_h264_vdec.a' ]]; then
            pushd $VDEC_DIR/h264
            make -f Coda980Dec.mak clean
            make -f Coda980Dec.mak $curr_lib
            cp -f $VDEC_DIR/h264/$curr_lib $VDEC_DIR
            cp -f $VDEC_DIR/h264/vdi/cvi_h264_cfg.h $VDEC_DIR
            popd
        fi

    fi
done
#
if [ -f $VDEC_LIB ]; then
    make TEST_CASE=vdec BOARD=$BOARD RUN_ENV=DDR DEBUG=1 SLT_TEST= DEC_TYPE=$DEC_TYPE
else
    echo "$VDEC_LIB is necessary"
fi

#ROOT=$(pwd)
#VDEC_DIR=${ROOT}/test/vdec
#VDEC_LIB_PROJ_DIR=$VDEC_DIR/vdec-cnm
#LIB_NAME=libbmvdec.a
#LIB_INC=main_dec_bmtest.h
#VDEC_LIB=$VDEC_DIR/$LIB_NAME
#
#make clean
#
#if [ ! -f $VDEC_LIB ]; then
#    pushd $VDEC_LIB_PROJ_DIR
#    make -f Wave5xxDecV2.mak clean
#    make -f Wave5xxDecV2.mak libbmvdec
#    cp -f $VDEC_LIB_PROJ_DIR/$LIB_NAME $VDEC_DIR
#    cp -f $VDEC_LIB_PROJ_DIR/$LIB_INC $VDEC_DIR
#    popd
#fi
#
#if [ -f $VDEC_LIB ]; then
#    make TEST_CASE=vdec BOARD=FPGA RUN_ENV=DDR DEBUG=1 SLT_TEST=
#else
#    echo "$VDEC_LIB is necessary"
#fi
