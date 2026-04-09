_PROJECT_ROOT ?= $(ROOT)/test/${TEST_CASE}
INC += \
     -I${_PROJECT_ROOT}/comm/include \
     -I${_PROJECT_ROOT}/include \
     -I${_PROJECT_ROOT}/include/dpu_common \
     -I${_PROJECT_ROOT}/include/dpu_register \
     -I${_PROJECT_ROOT}/${SUB_TEST_CASE}

include ${_PROJECT_ROOT}/${SUB_TEST_CASE}/config.mk

# for soc
_INC := $(filter-out %.h, $(addprefix -I, $(wildcard $(TEST_CASE:%=$(ROOT)/test/%/include/*))))

INC += \
	 ${_INC} \
     -I${_PROJECT_ROOT}/soc/include/

# NOTICE: cpp used
CXX_COMM ?= $(wildcard $(TEST_CASE:%=$(ROOT)/test/%/comm/src/*.cpp))
CXX_COMM += $(wildcard $(TEST_CASE:%=$(ROOT)/test/%/lib/executor/*.cpp))
CXX_COMM += $(wildcard $(TEST_CASE:%=$(ROOT)/test/%/soc/lib/core_runtime/*.cpp))

# register
# deprecate that use regBuilder
#C_SRC += $(wildcard $(TEST_CASE:%=$(ROOT)/test/%/lib/executor/*/*.c))

# source code
CXX_SRC += $(wildcard $(TEST_CASE:%=$(ROOT)/test/%/${SUB_TEST_CASE}/*.cpp))
CXX_SRC += ${CXX_COMM}

DEFS +=

ifneq ($(RUN_ENV), )
CFLAGS += -mstrict-align
endif

CXXFLAGS += -DRUNTIME=SOC -Wno-int-to-pointer-cast -Wno-narrowing -D__unix -D__linux
# for register
CFLAGS += -Wno-unused-variable -Wno-format -Wno-misleading-indentation -Wno-unused-but-set-variable -Wno-enum-compare -D__unix -D__linux

LDFLAGS += 

_LDFLAGS += -lstdc++
