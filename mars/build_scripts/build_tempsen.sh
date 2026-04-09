BOARD=FPGA
TEST_CASE=tempsen

# for A53 verification
make clean; make TEST_CASE=${TEST_CASE} RUN_ENV=DRAM BOARD=${BOARD} DEBUG=1

# for RISCV verification
# make clean; make TEST_CASE=${TEST_CASE} RUN_ENV=DRAM BOARD=${BOARD} DEBUG=1 ARCH=RISCV RISCV_TYPE=C906B
# make clean; make TEST_CASE=${TEST_CASE} RUN_ENV=DRAM BOARD=${BOARD} DEBUG=1 ARCH=RISCV RISCV_TYPE=C906L

size out/mars_${BOARD}_${TEST_CASE}.elf
readelf out/mars_${BOARD}_${TEST_CASE}.elf -h | grep 'Entry point address'
