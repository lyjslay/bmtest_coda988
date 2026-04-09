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
#make clean; make TEST_CASE=vip RUN_ENV=DDR BOARD=PALLADIUM NPU_EN=0 SLT_TEST=slt
make clean; make TEST_CASE=vip RUN_ENV=DDR BOARD=FPGA NPU_EN=0 DEBUG=1
