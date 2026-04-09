INC += -I$(ROOT)/test/vip -I$(ROOT)/test/vpss/

#C_SRC += $(wildcard $(TEST_CASE:%=$(ROOT)/test/isp/i2c/*.c))
#C_SRC += $(wildcard $(TEST_CASE:%=$(ROOT)/test/isp/spi/*.c))
#C_SRC += $(wildcard $(TEST_CASE:%=$(ROOT)/test/isp/patterns.c))
#C_SRC += $(wildcard $(TEST_CASE:%=$(ROOT)/test/isp/isp_drv.c))
#C_SRC += $(wildcard $(TEST_CASE:%=$(ROOT)/test/isp/sensorlist.c))
#C_SRC += $(wildcard $(TEST_CASE:%=$(ROOT)/test/isp/testcase_isp.c))
#C_SRC += $(wildcard $(TEST_CASE:%=$(ROOT)/test/isp/sensor/*.c))
# C_SRC += $(wildcard $(TEST_CASE:%=$(ROOT)/test/vpss/testcase_vpss.c))
# C_SRC += $(wildcard $(TEST_CASE:%=$(ROOT)/test/vpss/scaler.c))
C_SRC += $(wildcard $(TEST_CASE:%=$(ROOT)/test/vip/testcase_ldc.c))
#C_SRC += $(wildcard $(TEST_CASE:%=$(ROOT)/test/vpss/cmdq.c))
DEFS += -DENV_CVITEST -DENV_FPGA -DBUILD_VIP

#CFLAGS := $(filter-out -Werror, $(CFLAGS))
ifneq ($(RUN_ENV), )
CFLAGS += -mstrict-align
endif

ifeq ($(DEBUG), 1)
CFLAGS += -O0
endif

CXXFLAGS +=

LDFLAGS +=
