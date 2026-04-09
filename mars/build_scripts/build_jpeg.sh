# BOARD
#	- FPGA -- fpga
#	- ASIC -- asic
#	- PALLADIUM -- PALLADIUM
#PALLADIUM
# SLT_TEST
#	- slt  -- slt test
#	- else -- normal test
make clean; make TEST_CASE=jpeg RUN_ENV=DDR BOARD=ASIC SLT_TEST=slt
