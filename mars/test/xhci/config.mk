SRC_DIR := $(ROOT)/test/xhci

C_SRC +=	\
	$(SRC_DIR)/xhci.c	\
	$(SRC_DIR)/xhci-cdc.c	\
	$(SRC_DIR)/xhci-ring.c	\
	$(SRC_DIR)/xhci-mem.c	\
	$(SRC_DIR)/ctype.c	\
	$(SRC_DIR)/usb.c	\
	$(SRC_DIR)/usb_storage.c	\
	$(SRC_DIR)/usb_hub.c

INC += \
	-I$(SRC_DIR)

DEFS +=

ifneq ($(RUN_ENV), )
CFLAGS += -mstrict-align
endif

CXXFLAGS +=

LDFLAGS +=
