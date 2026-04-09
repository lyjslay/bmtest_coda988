C_SRC += \
   $(ROOT)/test/spi_nor/testcase_spi_nor.c

DEFS +=

ifneq ($(RUN_ENV), )
CFLAGS += -mstrict-align
endif

CXXFLAGS +=

LDFLAGS +=
