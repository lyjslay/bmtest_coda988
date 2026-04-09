
VENC_DIR = $(ROOT)/test/venc
VENC_H264_LIB = $(VENC_DIR)/libcvi_h264_venc.a
VCODEC_LIB = $(VENC_DIR)/libcvi_vcodec.a

INC += -I$(VENC_DIR)

C_SRC +=

ifeq ($(ENC_TYPE), h264)
LIB_OBJ += $(VENC_H264_LIB)
CFLAGS += -DTEST_H264_ENC
endif

ifeq ($(ENC_TYPE), h265)
LIB_OBJ += $(VCODEC_LIB)
CFLAGS += -DTEST_H265_ENC
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

