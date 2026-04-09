#
# Copyright (c) 2016-2017, ARM Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

JPEG_PATH := $(ROOT)/test/jpeg

JPUAPI_PATH = $(JPEG_PATH)/jpuapi
JPUAPI_SRCS	:=	\
		$(JPUAPI_PATH)/jpuapi.c		\
		$(JPUAPI_PATH)/jpuapifunc.c
C_SRC += $(JPUAPI_SRCS)

JDI_PATH = $(JPEG_PATH)/jdi
JDI_SRCS	:=	\
		$(JDI_PATH)/mm.c			\
		$(JDI_PATH)/nonos/jdi.c		\
		$(JDI_PATH)/nonos/jdi_osal.c
C_SRC += $(JDI_SRCS)

JPEG_SRC :=                             \
        $(JPEG_PATH)/src/jpurun.c       \
        $(JPEG_PATH)/src/jpuhelper.c    \
        $(JPEG_PATH)/src/mixer.c

C_SRC += $(JPEG_SRC)

DEFINES += -DJPEG_FPGA

ifneq ($(SLT_TEST_CASE), )
CFLAGS +=
else
CFLAGS += -mstrict-align -fpack-struct=8 -fno-builtin
endif

CFLAGS += -I$(JPEG_PATH)/jpuapi -I$(JPEG_PATH)/include -I$(JDI_PATH) $(DEFINES)

ifeq ($(DEBUG), 1)
CFLAGS += -O0
endif

ifneq ($(SLT_TEST), )
CFLAGS += -DSLT_TEST -DCALC_MD5
endif

ifneq ($(SLT_TEST_CASE), )
CFLAGS += -DSLT_TEST -DCALC_MD5
endif
