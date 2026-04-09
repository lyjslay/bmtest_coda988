SRC_DIR := $(ROOT)/test/dwc2_h/src
INC_DIR := $(ROOT)/test/dwc2_h

DRVSRC := \
	$(SRC_DIR)/dwc2.c \
	$(SRC_DIR)/ctype.c \
	$(SRC_DIR)/usb.c \
	$(SRC_DIR)/usb_storage.c \
	$(SRC_DIR)/usb_hub.c

C_SRC += \
	 $(DRVSRC)
INC += \
	-I$(INC_DIR)

CFLAGS := $(filter-out -Werror, $(CFLAGS))
ifneq ($(RUN_ENV), )
CFLAGS += -mstrict-align
endif

ifneq ($(MEM_REMAP), )
DEFS += -DMEM_REMAP=$(MEM_REMAP)
else
MEM_REMAP := 0
DEFS += -DMEM_REMAP=0
endif

CXXFLAGS +=

LDFLAGS +=
