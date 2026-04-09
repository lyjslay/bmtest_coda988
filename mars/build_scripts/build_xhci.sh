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
make TEST_CASE=xhci TEST_INDEX=10 BOARD=PALLADIUM RUN_ENV=DDR DEBUG=1
#make TEST_CASE=xhci TEST_INDEX=10 BOARD=FPGA RUN_ENV=DDR DEBUG=1
