C_SRC += \
   $(ROOT)/test/spi/bm_dw_spi.c

DEFS +=

ifneq ($(RUN_ENV), )
CFLAGS += -mstrict-align
endif

CXXFLAGS +=

LDFLAGS +=
