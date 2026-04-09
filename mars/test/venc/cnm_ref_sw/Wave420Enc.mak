# ----------------------------------------------------------------------
#
# Project: C&M Video encoder sample
#
# ----------------------------------------------------------------------
.PHONY: create_dir force_dependency

USE_FFMPEG=no
USE_PTHREAD=no

INSTALL_DIR = ./release

DEBUG = 1

CROSS_CC_PREFIX=${CROSS_COMPILE}
BUILD_CONFIGURATION=NonOS

UNAME = $(shell uname -a)
ifneq (,$(findstring i386, $(UNAME)))
    USE_32BIT = yes
endif


ifeq ($(USE_32BIT), yes)
PLATFORM    = nativelinux
else
PLATFORM    = nativelinux_64bit
endif


MM_C            =
PLATFORM_FLAGS  =

ifeq ("$(BUILD_CONFIGURATION)", "NonOS")
    VDI_C           = vdi/nonos/vdi.c
    VDI_OSAL_C      = vdi/nonos/vdi_osal.c
    MM_C            = vdi/mm.c
    USE_FFMPEG      = no
    USE_PTHREAD     = no
    PLATFORM        = none
    # DEFINES         = -DLIB_C_STUB
	SLIBNAME        = libwave.a
	LIBS            =
    PLATFORM_FLAGS  =
endif
ifeq ("$(BUILD_CONFIGURATION)", "EmbeddedLinux")
	VDI_C           = vdi/linux/vdi.c
	VDI_OSAL_C      = vdi/linux/vdi_osal.c
    CROSS_CC_PREFIX = arm-none-linux-gnueabi-
    PLATFORM        = armlinux
endif

LIBVIDEO=$(INSTALL_DIR)/$(SLIBNAME)
BUILDLIST=$(LIBVIDEO)

CC  = $(CROSS_CC_PREFIX)gcc
CXX = $(CROSS_CC_PREFIX)g++
LINKING=$(CC)
AR  = $(CROSS_CC_PREFIX)ar

INCLUDES =  -I./vpuapi  -I./sample/helper -I./vdi
DEFINES += -DWAVE420

ifeq ($(USE_FFMPEG), yes)
INCLUDES += -I./ffmpeg/include
endif


CFLAGS  = -I.  -fPIC -Wno-maybe-uninitialized -Wno-format -Wl,--fatal-warning $(INCLUDES) $(DEFINES) $(PLATFORM_FLAGS)
ARFLAGS = rcs

CFLAGS += -Wno-unused-variable -Wno-unused-but-set-variable -Wno-implicit-function-declaration -Wno-misleading-indentation

ifeq ($(DEBUG), 0)
CFLAGS +=
else ifeq ($(DEBUG), 1)
CFLAGS += -g -O0
endif


# LDFLAGS  += $(PLATFORM_FLAGS) -L./theoraparser/
# LDLIBS   += -ltheoraparser

# LDFLAGS += -L./vpuapi/coda9/980_roi_rc_lib/
# LDLIBS  += -lroirc

ifeq ($(USE_FFMPEG), yes)
LDLIBS  += -lavformat -lavcodec -lavutil
LDFLAGS += -L./ffmpeg/lib/$(PLATFORM)
ifneq ($(USE_32BIT), yes)
LDLIBS  += -lz
endif #USE_32BIT
endif #USE_FFMPEG

ifeq ($(USE_PTHREAD), yes)
LDLIBS  += -lpthread
endif
LDLIBS  += -lm


TARGET=wave420_test

MAKEFILE=Wave420Enc.mak
OBJDIR=obj
ALLOBJS=*.o
ALLDEPS=*.dep
ALLLIBS=*.a
RM=rm -f
MKDIR=mkdir -p


SOURCES_ENCTEST = sample/wave420_test.c

SOURCES_COMMON = \
				sample/helper/main_helper.c                       \
				sample/helper/vpuhelper.c                         \
				sample/helper/bitstream/bitstreamfeeder.c         \
				sample/helper/bitstream/bitstreamreader.c         \
				sample/helper/bitstream/binaryreader.c            \
				sample/helper/bitstream/bsfeeder_fixedsize_impl.c \
				sample/helper/bitstream/bsfeeder_framesize_impl.c \
				sample/helper/bitstream/bsfeeder_size_plus_es_impl.c \
				sample/helper/comparator/bin_comparator_impl.c    \
				sample/helper/comparator/comparator.c             \
				sample/helper/comparator/md5_comparator_impl.c    \
				sample/helper/comparator/yuv_comparator_impl.c    \
				sample/helper/display/fbdev_impl.c                \
				sample/helper/display/hdmi_impl.c                 \
				sample/helper/display/simplerenderer.c            \
				sample/helper/misc/cfgParser.c                    \
				sample/helper/misc/cnm_fpga.c                     \
				sample/helper/misc/cnm_video_helper.c             \
				sample/helper/misc/container.c                    \
				sample/helper/misc/datastructure.c                \
				sample/helper/misc/platform.c                     \
				sample/helper/misc/pbu.c                          \
				sample/helper/misc/debug.c                        \
				sample/helper/yuv/yuvfeeder.c                     \
				sample/helper/yuv/yuvLoaderfeeder.c


SOURCES_COMMON += $(VDI_C)                                         \
					$(VDI_OSAL_C)                                     \
					$(MM_C)                                           \
					vpuapi/product.c                                  \
					vpuapi/vpuapifunc.c                               \
					vpuapi/vpuapi.c                                   \
					vpuapi/coda9/coda9.c                              \
					vpuapi/wave/common/common.c                       \
					vpuapi/wave/coda7q/coda7q.c                       \
					vpuapi/wave/wave4/wave4.c                         \
					vpuapi/wave/wave5/wave5.c


OBJECTNAMES_COMMON=$(patsubst %.S,%.o,$(patsubst %.c,%.o,$(patsubst %.cpp,%.o,$(SOURCES_COMMON))))
OBJECTPATHS_COMMON=$(addprefix $(OBJDIR)/,$(notdir $(OBJECTNAMES_COMMON)))


OBJECTNAMES_ENCTEST=$(patsubst %.c,%.o,$(patsubst %.cpp,%.o,$(SOURCES_ENCTEST)))
OBJECTPATHS_ENCTEST=$(addprefix $(OBJDIR)/,$(notdir $(OBJECTNAMES_ENCTEST))) $(OBJECTPATHS_COMMON)


all: create_dir $(BUILDLIST)

$(LIBVIDEO): $(OBJECTPATHS_ENCTEST)
ifeq ("$(BUILD_CONFIGURATION)", "NonOS")
	$(AR) $(ARFLAGS) $(LIBVIDEO) $(OBJECTPATHS_ENCTEST) $(LIBS)
else
	$(CC) $(LDFLAGS) $(CFLAGS) $(SOFLAGS) $(LIBVIDEO) $(OBJECTPATHS_COMMON) $(LIBS)
endif

# $(TARGET): create_dir $(OBJECTPATHS)
# 	$(LINKING)  $@ $(LDFLAGS) -Wl,-gc-section -Wl,--start-group $(OBJECTPATHS) $(LDLIBS) -Wl,--end-group

-include $(OBJECTPATHS:.o=.dep)

clean:
	$(RM) $(TARGET)
	$(RM) $(OBJDIR)/$(ALLOBJS)
	$(RM) $(OBJDIR)/$(ALLDEPS)
	$(RM) theoraparser/$(ALLOBJS)
	$(RM) theoraparser/$(ALLLIBS)
	$(RM) theoraparser/$(ALLDEPS)

# libtheoraparser.a: force_dependency
# 	cd theoraparser; make

$(OBJDIR)/wave420_test.o : sample/wave420_test.c $(MAKEFILE)
	$(CC) $(CFLAGS) -Wall -Werror -c $< -o $@ -MD -MF $(@:.o=.dep)

$(OBJDIR)/main_helper.o : sample/helper/main_helper.c $(MAKEFILE)
	$(CC) $(CFLAGS) -Wall -Werror -c $< -o $@ -MD -MF $(@:.o=.dep)

$(OBJDIR)/vpuhelper.o : sample/helper/vpuhelper.c $(MAKEFILE)
	$(CC) $(CFLAGS) -Wall -Werror -c $< -o $@ -MD -MF $(@:.o=.dep)


$(OBJDIR)/bitstreamfeeder.o : sample/helper/bitstream/bitstreamfeeder.c $(MAKEFILE)
	$(CC) $(CFLAGS) -Wall -Werror -c $< -o $@ -MD -MF $(@:.o=.dep)

$(OBJDIR)/bitstreamreader.o : sample/helper/bitstream/bitstreamreader.c $(MAKEFILE)
	$(CC) $(CFLAGS) -Wall -Werror -c $< -o $@ -MD -MF $(@:.o=.dep)

$(OBJDIR)/binaryreader.o : sample/helper/bitstream/binaryreader.c $(MAKEFILE)
	$(CC) $(CFLAGS) -Wall -Werror -c $< -o $@ -MD -MF $(@:.o=.dep)

$(OBJDIR)/bsfeeder_fixedsize_impl.o : sample/helper/bitstream/bsfeeder_fixedsize_impl.c $(MAKEFILE)
	$(CC) $(CFLAGS) -Wall -Werror -c $< -o $@ -MD -MF $(@:.o=.dep)

$(OBJDIR)/bsfeeder_framesize_impl.o : sample/helper/bitstream/bsfeeder_framesize_impl.c $(MAKEFILE)
	$(CC) $(CFLAGS) -Wall -Werror -c $< -o $@ -MD -MF $(@:.o=.dep)

$(OBJDIR)/bsfeeder_size_plus_es_impl.o : sample/helper/bitstream/bsfeeder_size_plus_es_impl.c $(MAKEFILE)
	$(CC) $(CFLAGS) -Wall -Werror -c $< -o $@ -MD -MF $(@:.o=.dep)

$(OBJDIR)/bin_comparator_impl.o : sample/helper/comparator/bin_comparator_impl.c $(MAKEFILE)
	$(CC) $(CFLAGS) -Wall -Werror -c $< -o $@ -MD -MF $(@:.o=.dep)

$(OBJDIR)/comparator.o : sample/helper/comparator/comparator.c $(MAKEFILE)
	$(CC) $(CFLAGS) -Wall -Werror -c $< -o $@ -MD -MF $(@:.o=.dep)

$(OBJDIR)/md5_comparator_impl.o : sample/helper/comparator/md5_comparator_impl.c $(MAKEFILE)
	$(CC) $(CFLAGS) -Wall -Werror -c $< -o $@ -MD -MF $(@:.o=.dep)

$(OBJDIR)/yuv_comparator_impl.o : sample/helper/comparator/yuv_comparator_impl.c $(MAKEFILE)
	$(CC) $(CFLAGS) -Wall -Werror -c $< -o $@ -MD -MF $(@:.o=.dep)


$(OBJDIR)/fbdev_impl.o : sample/helper/display/fbdev_impl.c $(MAKEFILE)
	$(CC) $(CFLAGS) -Wall -Werror -c $< -o $@ -MD -MF $(@:.o=.dep)

$(OBJDIR)/hdmi_impl.o : sample/helper/display/hdmi_impl.c $(MAKEFILE)
	$(CC) $(CFLAGS) -Wall -Werror -c $< -o $@ -MD -MF $(@:.o=.dep)

$(OBJDIR)/mixer.o : sample/helper/display/mixer.c $(MAKEFILE)
	$(CC) $(CFLAGS) -Wall -Werror -c $< -o $@ -MD -MF $(@:.o=.dep)

$(OBJDIR)/simplerenderer.o : sample/helper/display/simplerenderer.c $(MAKEFILE)
	$(CC) $(CFLAGS) -Wall -Werror -c $< -o $@ -MD -MF $(@:.o=.dep)


$(OBJDIR)/cfgParser.o : sample/helper/misc/cfgParser.c $(MAKEFILE)
	$(CC) $(CFLAGS) -Wall -Werror -c $< -o $@ -MD -MF $(@:.o=.dep)

$(OBJDIR)/cnm_fpga.o : sample/helper/misc/cnm_fpga.c $(MAKEFILE)
	$(CC) $(CFLAGS) -Wall -Werror -c $< -o $@ -MD -MF $(@:.o=.dep)

$(OBJDIR)/cnm_video_helper.o : sample/helper/misc/cnm_video_helper.c $(MAKEFILE)
	$(CC) $(CFLAGS) -Wall -Werror -c $< -o $@ -MD -MF $(@:.o=.dep)

$(OBJDIR)/container.o : sample/helper/misc/container.c $(MAKEFILE)
	$(CC) $(CFLAGS) -Wall -Werror -c $< -o $@ -MD -MF $(@:.o=.dep)

$(OBJDIR)/datastructure.o : sample/helper/misc/datastructure.c $(MAKEFILE)
	$(CC) $(CFLAGS) -Wall -Werror -c $< -o $@ -MD -MF $(@:.o=.dep)

$(OBJDIR)/platform.o : sample/helper/misc/platform.c $(MAKEFILE)
	$(CC) $(CFLAGS) -Wall -Werror -c $< -o $@ -MD -MF $(@:.o=.dep)

$(OBJDIR)/pbu.o : sample/helper/misc/pbu.c $(MAKEFILE)
	$(CC) $(CFLAGS) -Wall -Werror -c $< -o $@ -MD -MF $(@:.o=.dep)

$(OBJDIR)/yuvfeeder.o : sample/helper/yuv/yuvfeeder.c $(MAKEFILE)
	$(CC) $(CFLAGS) -Wall -Werror -c $< -o $@ -MD -MF $(@:.o=.dep)

$(OBJDIR)/yuvLoaderfeeder.o : sample/helper/yuv/yuvLoaderfeeder.c $(MAKEFILE)
	$(CC) $(CFLAGS) -Wall -Werror -c $< -o $@ -MD -MF $(@:.o=.dep)

$(OBJDIR)/debug.o : sample/helper/misc/debug.c $(MAKEFILE)
	$(CC) $(CFLAGS) -Wall -Werror -c $< -o $@ -MD -MF $(@:.o=.dep)




$(OBJDIR)/vdi.o : $(VDI_C) $(MAKEFILE)
	$(CC) $(CFLAGS) -Wall -Werror -c $< -o $@ -MD -MF $(@:.o=.dep)

$(OBJDIR)/vdi_osal.o : $(VDI_OSAL_C) $(MAKEFILE)
	$(CC) $(CFLAGS) -Wall -Werror -c $< -o $@ -MD -MF $(@:.o=.dep)

ifneq ("$(MM_C)", "")
$(OBJDIR)/mm.o : $(MM_C) $(MAKEFILE)
	$(CC) $(CFLAGS) -Wall -Werror -c $< -o $@ -MD -MF $(@:.o=.dep)
endif

$(OBJDIR)/product.o : vpuapi/product.c $(MAKEFILE)
	$(CC) $(CFLAGS) -Wall -Werror -c $< -o $@ -MD -MF $(@:.o=.dep)

$(OBJDIR)/vpuapifunc.o : vpuapi/vpuapifunc.c $(MAKEFILE)
	$(CC) $(CFLAGS) -Wall -Werror -c $< -o $@ -MD -MF $(@:.o=.dep)

$(OBJDIR)/vpuapi.o : vpuapi/vpuapi.c $(MAKEFILE)
	$(CC) $(CFLAGS) -Wall -Werror -c $< -o $@ -MD -MF $(@:.o=.dep)

$(OBJDIR)/coda9.o : vpuapi/coda9/coda9.c $(MAKEFILE)
	$(CC) $(CFLAGS) -Wall -Werror -c $< -o $@ -MD -MF $(@:.o=.dep)

$(OBJDIR)/common.o : vpuapi/wave/common/common.c $(MAKEFILE)
	$(CC) $(CFLAGS) -Wall -Werror -c $< -o $@ -MD -MF $(@:.o=.dep)

$(OBJDIR)/coda7q.o : vpuapi/wave/coda7q/coda7q.c $(MAKEFILE)
	$(CC) $(CFLAGS) -Wall -Werror -c $< -o $@ -MD -MF $(@:.o=.dep)

$(OBJDIR)/wave4.o : vpuapi/wave/wave4/wave4.c $(MAKEFILE)
	$(CC) $(CFLAGS) -Wall -Werror -c $< -o $@ -MD -MF $(@:.o=.dep)

$(OBJDIR)/wave5.o : vpuapi/wave/wave5/wave5.c $(MAKEFILE)
	$(CC) $(CFLAGS) -Wall -Werror -c $< -o $@ -MD -MF $(@:.o=.dep)

create_dir:
	-mkdir -p $(OBJDIR)
	-mkdir -p $(INSTALL_DIR)


install:
	cp -f $(LIBVIDEO) ../mars3/cvi_bmtest/mars3/test/venc/