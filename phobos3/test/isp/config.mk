INC += \
     -I$(ROOT)/test/isp -I$(ROOT)/test/isp/vip -I$(ROOT)/test/vpss/ \
     -I$(ROOT)/test/isp/verify_tool/code_dir/linear_online_128x64_pld/reg_cfg

C_SRC += $(wildcard $(TEST_CASE:%=$(ROOT)/test/%/i2c/*.c))
C_SRC += $(wildcard $(TEST_CASE:%=$(ROOT)/test/%/spi/*.c))
C_SRC += $(wildcard $(TEST_CASE:%=$(ROOT)/test/%/vip/*.c))

# TEST_CIF=true
# BUILD_VIP=true
#TEST_PERF=true

# TEST_C906_IRQ=true
# TEST_TC906_0_IRQ=true
# TEST_TC906_1_IRQ=true

ifneq ($(TEST_C906), )
DEFS += -DTEST_C906
endif
ifneq ($(TEST_TC906_0_IRQ), )
DEFS += -DTEST_TC906_0_IRQ
endif
ifneq ($(TEST_TC906_1_IRQ), )
DEFS += -DTEST_TC906_1_IRQ
endif

ifneq ($(TEST_CIF), )
C_SRC += $(wildcard $(TEST_CASE:%=$(ROOT)/test/vip/testcase_cif.c))
C_SRC += $(wildcard $(TEST_CASE:%=$(ROOT)/test/vip/cif.c))
C_SRC += $(wildcard $(TEST_CASE:%=$(ROOT)/test/vip/sensorlist.c))
C_SRC += $(wildcard $(TEST_CASE:%=$(ROOT)/test/vip/sensor_imx327.c))
C_SRC += $(wildcard $(TEST_CASE:%=$(ROOT)/test/vip/sensor_imx585.c))
C_SRC += $(wildcard $(TEST_CASE:%=$(ROOT)/test/vip/sensor_sc200ai.c))
C_SRC += $(wildcard $(TEST_CASE:%=$(ROOT)/test/vip/sensor_imx327_sublvds.c))
C_SRC += $(wildcard $(TEST_CASE:%=$(ROOT)/test/vip/sensor_pr2020.c))
DEFS += -DTEST_CIF
endif

ifneq ($(TEST_PERF), )
DEFS += -DTEST_PERF
endif

ifneq ($(BUILD_VIP), )
C_SRC += $(wildcard $(TEST_CASE:%=$(ROOT)/test/vpss/testcase_vpss.c))
C_SRC += $(wildcard $(TEST_CASE:%=$(ROOT)/test/vpss/scaler.c))
C_SRC += $(wildcard $(TEST_CASE:%=$(ROOT)/test/vpss/cmdq.c))
DEFS += -DBUILD_VIP
endif

C_SRC += $(wildcard $(TEST_CASE:%=$(ROOT)/test/vip/scaler.c))
# C_SRC += $(wildcard $(TEST_CASE:%=$(ROOT)/test/vip/reg.c))
C_SRC += $(wildcard $(TEST_CASE:%=$(ROOT)/test/vip/vip_sys.c))
C_SRC += $(wildcard $(TEST_CASE:%=$(ROOT)/test/vip/vip_common.c))
DEFS += -DENV_BMTEST -DENV_CVITEST -DPORTING_TEST

DEFS += -DENV_FPGA

ifneq ($(RUN_ENV), )
CFLAGS += -mstrict-align
endif

CXXFLAGS +=

LDFLAGS +=
