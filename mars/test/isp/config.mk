INC += \
     -I$(ROOT)/test/isp

C_SRC += $(wildcard $(TEST_CASE:%=$(ROOT)/test/%/i2c/*.c))
C_SRC += $(wildcard $(TEST_CASE:%=$(ROOT)/test/%/spi/*.c))

C_SRC += $(wildcard $(TEST_CASE:%=$(ROOT)/test/vip/scaler.c))
C_SRC += $(wildcard $(TEST_CASE:%=$(ROOT)/test/vip/reg.c))
C_SRC += $(wildcard $(TEST_CASE:%=$(ROOT)/test/vip/vip_sys.c))
C_SRC += $(wildcard $(TEST_CASE:%=$(ROOT)/test/vip/vip_common.c))
DEFS += -DENV_BMTEST

DEFS += -DENV_FPGA

ifneq ($(RUN_ENV), )
CFLAGS += -mstrict-align
endif

CXXFLAGS +=

LDFLAGS +=
