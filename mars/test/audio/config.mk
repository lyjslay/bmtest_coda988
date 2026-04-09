C_SRC += \
   $(ROOT)/test/audio/bm_dw_i2c.c \
   $(ROOT)/test/audio/bm_i2s_tdm.c \
   $(ROOT)/test/audio/bm_sound_i2s.c \
   $(ROOT)/test/audio/adau1372.c

DEFS += -DBUILD_TEST_CASE_audio

ifneq ($(RUN_ENV), )
CFLAGS += -mstrict-align
endif

CXXFLAGS +=

LDFLAGS +=
