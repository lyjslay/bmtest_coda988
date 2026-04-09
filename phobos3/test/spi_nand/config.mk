C_SRC += \
   $(ROOT)/test/spi_nand/testcase_spi_nand.c

DEFS +=

ifneq ($(RUN_ENV), )
CFLAGS += -mstrict-align
endif

CXXFLAGS +=

LDFLAGS +=
