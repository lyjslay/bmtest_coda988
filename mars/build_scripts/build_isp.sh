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
#make clean; make TEST_CASE=isp RUN_ENV=SRAM BOARD=PALLADIUM DEBUG=1

#make clean; make TEST_CASE=isp RUN_ENV=DDR BOARD=PALLADIUM SLT_TEST=slt
make clean; make TEST_CASE=isp RUN_ENV=DDR BOARD=FPGA SLT_TEST=slt
