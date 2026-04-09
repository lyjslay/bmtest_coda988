# BOARD
#	- FPGA -- fpga
#	- PALLADIUM -- PALLADIUM
#	- ASIC -- asic
# RUN_ENV
#   - DDR
#   - SRAM
# SLT_TEST
#	- slt  -- slt test
#	- else -- normal test
make clean
# suspend/resume/remote wakeup
#make TEST_CASE=dwc2_g TEST_INDEX=10  BOARD=FPGA RUN_ENV=DDR NPU_EN=0 DEBUG=1
# Control Test
#make TEST_CASE=dwc2_g TEST_INDEX=1  BOARD=FPGA RUN_ENV=DDR NPU_EN=0 DEBUG=1
#make TEST_CASE=dwc2_g TEST_INDEX=1  BOARD=FPGA RUN_ENV=DDR NPU_EN=0 DEBUG=1
#make TEST_CASE=dwc2_g TEST_INDEX=1  BOARD=PALLADIUM RUN_ENV=DDR NPU_EN=0 DEBUG=1
# Bulk Test
#make TEST_CASE=dwc2_g TEST_INDEX=2  BOARD=FPGA RUN_ENV=DDR NPU_EN=0 DEBUG=1
# Comport
#make TEST_CASE=dwc2_g TEST_INDEX=9  BOARD=FPGA RUN_ENV=DDR NPU_EN=0 DEBUG=1

#cp out/mars_FPGA_dwc2_g.elf /home/max/share/mars_fpga/
# ========================================================
# RISCV
# suspend/resume/remote wakeup
#make TEST_CASE=dwc2_g TEST_INDEX=10  BOARD=FPGA RUN_ENV=DDR NPU_EN=0 DEBUG=1 ARCH=RISCV RISCV_TYPE=C906B
# Control Test
#make TEST_CASE=dwc2_g TEST_INDEX=1  BOARD=FPGA RUN_ENV=DDR NPU_EN=0 DEBUG=1 ARCH=RISCV RISCV_TYPE=C906B
# Bulk Test
#make TEST_CASE=dwc2_g TEST_INDEX=2  BOARD=FPGA RUN_ENV=DDR NPU_EN=0 DEBUG=1 ARCH=RISCV RISCV_TYPE=C906B
# Comport
make TEST_CASE=dwc2_g TEST_INDEX=9  BOARD=FPGA RUN_ENV=DDR NPU_EN=0 DEBUG=1 ARCH=RISCV RISCV_TYPE=C906B

cp out/mars_FPGA_dwc2_g.bin /home/max/share/mars_fpga/
