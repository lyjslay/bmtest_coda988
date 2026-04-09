# BOARD
#	- FPGA -- fpga
#	- ASIC -- asic
#	- PALLADIUM -- PALLADIUM
# RUN_ENV
#	- DDR
#	- SRAM (0x0e00_0000 - 0x0e00_ffff)
#	- TPU_SRAM (0x0c00_0000 -0x0c0f_ffff)
make clean; make TEST_CASE=ddr RUN_ENV=SRAM BOARD=ASIC

size out/mars_ASIC_ddr.elf
readelf out/mars_ASIC_ddr.elf -h | grep 'Entry point address'
