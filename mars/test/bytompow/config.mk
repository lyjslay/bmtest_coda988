#
# Copyright (c) 2016-2017, ARM Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

BYTOMPOW_PATH := $(ROOT)/test/bytompow

BYTOMPOW_SRC :=		\
		$(BYTOMPOW_PATH)/sha3.c	\
		$(BYTOMPOW_PATH)/bytompow_batch.c\
		$(BYTOMPOW_PATH)/byte_order.c

C_SRC += $(BYTOMPOW_SRC)

DEFINES +=

CFLAGS += -mstrict-align
