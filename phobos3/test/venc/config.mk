# INC += -I/media/cvitek/yijun.liu01/bmtest/cvi_bmtest/phobos3/test/venc/ -I/media/cvitek/yijun.liu01/bmtest/cvi_bmtest/phobos3/test/venc/cnm_ref_sw/

# LIB_OBJ += /media/cvitek/yijun.liu01/bmtest/cvi_bmtest/phobos3/test/venc/libcoda.a

THIS_DIR := $(dir $(lastword $(MAKEFILE_LIST)))

INC += -I$(THIS_DIR) \
       -I$(THIS_DIR)cnm_ref_sw/

LIB_OBJ += $(THIS_DIR)libcoda.a

ifeq ($(RUN_ENV), )
CFLAGS += -mstrict-align
endif

CXXFLAGS +=

LDFLAGS +=

