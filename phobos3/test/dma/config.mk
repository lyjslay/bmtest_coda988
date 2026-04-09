C_SRC += $(ROOT)/drivers/sysdma/sysdma.c

DEFS +=

ifneq ($(RUN_ENV), )
CFLAGS += -mstrict-align
endif

CXXFLAGS +=

LDFLAGS +=
