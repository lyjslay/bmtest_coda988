#C_SRC += $(ROOT)/test/$(TEST_CASE)/uartchain_dw.c
#C_SRC += $(ROOT)/test/$(TEST_CASE)/uart_dma.c

DEFS +=

ifneq ($(RUN_ENV), )
CFLAGS += -mstrict-align
endif

CXXFLAGS +=

LDFLAGS +=
