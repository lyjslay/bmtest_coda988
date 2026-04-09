INC += -I$(ROOT)/test/vpss

#C_SRC += $(wildcard $(TEST_CASE:%=$(ROOT)/test/isp/i2c/*.c))
#C_SRC += $(wildcard $(TEST_CASE:%=$(ROOT)/test/isp/spi/*.c))
#C_SRC += $(wildcard $(TEST_CASE:%=$(ROOT)/test/isp/patterns.c))
#C_SRC += $(wildcard $(TEST_CASE:%=$(ROOT)/test/isp/isp_drv.c))
#C_SRC += $(wildcard $(TEST_CASE:%=$(ROOT)/test/isp/sensorlist.c))
#C_SRC += $(wildcard $(TEST_CASE:%=$(ROOT)/test/isp/testcase_isp.c))
#C_SRC += $(wildcard $(TEST_CASE:%=$(ROOT)/test/isp/sensor/*.c))

DEFS += -DENV_CVITEST -DENV_FPGA

#CFLAGS := $(filter-out -Werror, $(CFLAGS))
ifneq ($(RUN_ENV), )
CFLAGS += -mstrict-align
endif

ifeq ($(DEBUG), 1)
CFLAGS += -O0
endif

CXXFLAGS +=

LDFLAGS +=
