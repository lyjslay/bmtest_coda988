C_SRC +=

DEFS += -DBUILD_TEST_CASE_$(TEST_CASE)

ifneq ($(RUN_ENV), )
CFLAGS += -mstrict-align
endif

CXXFLAGS +=

LDFLAGS +=
