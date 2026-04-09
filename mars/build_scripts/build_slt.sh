RED='\033[0;31m'
NC='\033[0m'
GREEN='\033[0;32m'

SLT_OUT_DIR="slt_out"
SLT_BASIC_DIR="${SLT_OUT_DIR}/out_basic_vpp_jpu_vpu"
SLT_BASIC_I2C0_DIR="${SLT_OUT_DIR}/out_basic_i2c0_vpp_jpu_vpu"
SLT_BASIC_COMPUTE_STICK_CHECK_DIR="${SLT_OUT_DIR}/out_basic_compute_stick_hw_check"
SLT_TPU_DIR="${SLT_OUT_DIR}/out_tpu"
USER_ACCOUNT=fisher.cheng

#DDR config (refer to bm1880/Makefile)
#DDR_CONFIG = 1 EVB SAMSUNG DUAL-DDR3 256MB 1850
#DDR_CONFIG = 2 EVB MICRON DUAL-DDR3 2048MB 1850
#DDR_CONFIG = 3 CONTROL MICRON SINGLE-DDR3L 256MB
#DDR_CONFIG = 4 EVB MICRON/SAMSUNG DUAL-DDR4 1024MB 2400
#DDR_CONFIG = 5 EVB MICRON/SAMSUNG SINGLE-DDR4 512MB 2400
#DDR_CONFIG = 6 EVB/USB SINGLE-LPDDR4 1GB 3200
#DDR_CONFIG = 7 SIP ESMT DUAL-DDR3 256MB 1850
#DDR_CONFIG = 8 EVB/USB SINGLE-LPDDR4 1GB 2400
DDR_CONFIG=8

# Build basic_vpp_jpu_vpu testcase
build_slt_basic_vpp_jpu_vpu() {
    make clean
    make BOARD=ASIC RUN_ENV=DDR DDR_CONFIG=${DDR_CONFIG} SLT_TEST_CASE="slt efuse i2c dhrystone ddr spi emmc sdcard usb vpp jpeg boda"
    if [ $? != 0 ]; then
        echo -e "${RED}Build basic_vpp_jpu_vpu testcase failed${NC}"
        exit 1
    else
        echo -e "${GREEN}Build basic_vpp_jpu_vpu testcase successfully${NC}"
    fi

    if [ -e out/bm1880.bin ]; then
        mkdir -p ${SLT_BASIC_DIR}
        cp out/bm1880.bin out/bm1880.elf ${SLT_BASIC_DIR}
    fi
}

# Build basic_i2c0_vpp_jpu_vpu testcase
build_slt_basic_i2c0_vpp_jpu_vpu() {
    make clean
    make BOARD=ASIC RUN_ENV=DDR DDR_CONFIG=${DDR_CONFIG} THERMAL_I2C_CONFIG=0 SLT_TEST_CASE="slt efuse i2c dhrystone ddr spi emmc sdcard usb vpp jpeg boda"
    if [ $? != 0 ]; then
        echo -e "${RED}Build basic_i2c0_vpp_jpu_vpu testcase failed${NC}"
        exit 1
    else
        echo -e "${GREEN}Build basic_i2c0_vpp_jpu_vpu testcase successfully${NC}"
    fi

    if [ -e out/bm1880.bin ]; then
        mkdir -p ${SLT_BASIC_I2C0_DIR}
        cp out/bm1880.bin out/bm1880.elf ${SLT_BASIC_I2C0_DIR}
    fi
}

# Build basic_compute_stick_hw_check testcase
build_slt_basic_compute_stick_hw_check() {
    make clean
    make BOARD=ASIC RUN_ENV=TPU_SRAM DDR_CONFIG=${DDR_CONFIG} SLT_TEST_CASE="slt efuse i2c ddr emmc" 
    if [ $? != 0 ]; then
        echo -e "${RED}Build basic_compute_stick_hw_check testcase failed${NC}"
        exit 1
    else
        echo -e "${GREEN}Build basic_compute_stick_hw_check testcase successfully${NC}"
    fi

    if [ -e out/bm1880.bin ]; then
        mkdir -p ${SLT_BASIC_COMPUTE_STICK_CHECK_DIR}
        cp out/bm1880.bin out/bm1880.elf ${SLT_BASIC_COMPUTE_STICK_CHECK_DIR}
    fi
}

# Build tpu testcase
build_slt_tpu() {
    make clean
    make BOARD=ASIC RUN_ENV=DDR DDR_CONFIG=${DDR_CONFIG} SLT_TEST_CASE="slt get_matrix_stride get_matrix_tp get_tensor_stride get_tensor_stride_unalign get_tensor_tp lut matrix_transfer put_matrix_stride put_matrix_tp put_tensor_stride put_tensor_stride_unalign put_tensor_tp put_tensor_tp_unalign put_tensor_unalign relu tensor_add tensor_and tensor_arith_shift tensor_copy tensor_mac tensor_mac_const tensor_max tensor_mdsum tensor_min tensor_mul tensor_mul_const tensor_or tensor_sub tensor_transfer tensor_xor tg_copy_gdma tl_copy_gdma_fixed conv fc pooling matrix_mac_2 winograd net_test"
    if [ $? != 0 ]; then
        echo -e "${RED}Build tpu testcase failed${NC}"
        exit 1
    else
        echo -e "${GREEN}Build tpu testcase successfully${NC}"
    fi

    if [ -e out/bm1880.bin ]; then
        mkdir -p ${SLT_TPU_DIR}
        cp out/bm1880.bin out/bm1880.elf ${SLT_TPU_DIR}
    fi
}

# Show size and entry point
show_slt_size_entry_point() {
    if [ -e ${SLT_BASIC_DIR}/bm1880.elf ]; then
        size ${SLT_BASIC_DIR}/bm1880.elf
        readelf ${SLT_BASIC_DIR}/bm1880.elf -h | grep 'Entry point address'
    fi

    if [ -e ${SLT_BASIC_I2C0_DIR}/bm1880.elf ]; then
        size ${SLT_BASIC_I2C0_DIR}/bm1880.elf
        readelf ${SLT_BASIC_I2C0_DIR}/bm1880.elf -h | grep 'Entry point address'
    fi

    if [ -e ${SLT_BASIC_COMPUTE_STICK_CHECK_DIR}/bm1880.elf ]; then
        size ${SLT_BASIC_COMPUTE_STICK_CHECK_DIR}/bm1880.elf
        readelf ${SLT_BASIC_COMPUTE_STICK_CHECK_DIR}/bm1880.elf -h | grep 'Entry point address'
    fi

    if [ -e ${SLT_TPU_DIR}/bm1880.elf ]; then
        size ${SLT_TPU_DIR}/bm1880.elf
        readelf ${SLT_TPU_DIR}/bm1880.elf -h | grep 'Entry point address'
    fi
}

build_slt_all_testcase() {
    rm ${SLT_OUT_DIR} -rf
    build_slt_basic_vpp_jpu_vpu
    build_slt_basic_i2c0_vpp_jpu_vpu
    build_slt_basic_compute_stick_hw_check
    build_slt_tpu
    show_slt_size_entry_point
}

put_slt_test_pattern() {
    if [ ! -e slt_test_pattern.tar.gz ]; then
        echo "pull slt_test_pattern.tar.gz from cuda2, enter your cuda2 password" 
        scp  $USER_ACCOUNT@10.11.23.152:/home/Fisher.Cheng/slt_test_pattern/slt_test_pattern.tar.gz .
    fi
    tar zxvf slt_test_pattern.tar.gz -C ${SLT_OUT_DIR}
}

build_slt_all_testcase
# put_slt_test_pattern
