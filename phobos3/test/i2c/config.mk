C_SRC += \
   $(ROOT)/test/i2c/testcase_i2c.c

DEFS +=

ifeq ($(THERMAL_I2C_CONFIG), 0)
DEFS += -DTHERMAL_USE_I2C_0
endif

ifneq ($(RUN_ENV), )
CFLAGS += -mstrict-align
endif

CXXFLAGS +=

LDFLAGS +=
