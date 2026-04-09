C_SRC +=

DEFS +=

ifneq ($(RUN_ENV), )
CFLAGS += -mstrict-align
endif

CFLAGS := $(filter-out -Werror, $(CFLAGS))

CXXFLAGS +=

LDFLAGS +=
