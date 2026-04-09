C_SRC += \
   $(ROOT)/test/$(TEST_CASE)/bm_sdhci.c \
   $(ROOT)/test/$(TEST_CASE)/mmc.c

DEFS +=

ifneq ($(RUN_ENV), )
CFLAGS += -mstrict-align
endif

CXXFLAGS +=

LDFLAGS +=
