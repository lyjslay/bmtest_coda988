
VDEC_DIR = $(ROOT)/test/vdec
VDEC_H264_LIB = $(VDEC_DIR)/libcvi_h264_vdec.a
VDEC_H265_LIB = $(VDEC_DIR)/libcvi_h265_vdec.a

INC += -I$(VDEC_DIR)

C_SRC +=

ifeq ($(DEC_TYPE), h264)
LIB_OBJ += $(VDEC_H264_LIB)
CFLAGS += -DTEST_H264_DEC
endif

ifeq ($(DEC_TYPE), h265)
LIB_OBJ += $(VDEC_H265_LIB)
CFLAGS += -DTEST_H265_DEC
endif

ifeq ($(RUN_ENV), ALONE)
CFLAGS += -mstrict-align
endif

ifeq ($(SLT_TEST), slt)
CFLAGS += -DSLT_TEST
endif

ifeq ($(SLT_TEST_CASE), slt)
CFLAGS += -DSLT_TEST
endif

CXXFLAGS +=

LDFLAGS +=

