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
make TEST_CASE=usb TEST_INDEX=10  BOARD=FPGA RUN_ENV=DDR DEBUG=1 MEM_REMAP=1 NEW_DMA=0
# Control Test
#make TEST_CASE=usb TEST_INDEX=1  BOARD=FPGA RUN_ENV=DDR DEBUG=1 MEM_REMAP=1 NEW_DMA=1
# Bulk Test
#make TEST_CASE=usb TEST_INDEX=2  BOARD=FPGA RUN_ENV=DDR DEBUG=1 MEM_REMAP=1 NEW_DMA=0
