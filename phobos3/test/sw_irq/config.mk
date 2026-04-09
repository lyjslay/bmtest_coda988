C_SRC += \
   $(ROOT)/test/sw_irq/testcase_sw_irq.c

DEFS +=

ifneq ($(RUN_ENV), )
CFLAGS += -mstrict-align
endif

CXXFLAGS +=

LDFLAGS +=
