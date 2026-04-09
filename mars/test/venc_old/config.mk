INC += -I/media/cvitek/yijun.liu01/cvi_bmtest/mars/test/venc/ -I/media/cvitek/yijun.liu01/cvi_bmtest/mars/test/venc/cnm_ref_sw/

LIB_OBJ += /media/cvitek/yijun.liu01/cvi_bmtest/mars/test/venc/libcoda.a

ifeq ($(RUN_ENV), )
CFLAGS += -mstrict-align
endif

CXXFLAGS +=

LDFLAGS +=

