# slt test    -- SLT_TEST=slt
# normal test -- SLT_TEST=
make clean; make TEST_CASE=vpp RUN_ENV=DDR BOARD=ASIC SLT_TEST=slt
