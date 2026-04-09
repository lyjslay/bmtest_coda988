SRC_DIR := $(ROOT)/test/dwc2_g/src
CORE_DIR := $(ROOT)/test/dwc2_g/CoreDriver
PLATFORM_DIR := $(ROOT)/test/dwc2_g/platform/vsp

DRVSRC := $(CORE_DIR)/src/dwc2_udc_otg.c $(CORE_DIR)/src/dwc2_udc_otg_xfer_dma.c
PLATFORM_SRC := $(PLATFORM_DIR)/cps_cvi.c

INC += \
   -I$(CORE_DIR)/inc \
   -I$(ROOT)/test/dwc2_g \
   -I$(SRC_DIR)/ \
   -I$(PLATFORM_DIR)

DEFS += -D__BARE_METAL__

ifneq ($(TEST_INDEX), )
DEFS += -DTEST_INDEX=$(TEST_INDEX)
else
TEST_INDEX := 9
DEFS += -DTEST_INDEX=9
endif

#ifndef TEST_INDEX
#@echo "Type TEST_INDEX value"
#endif

#ifeq ($(TEST_INDEX),0)
#@echo "Type TEST_INDEX value"
#endif

ifeq ($(TEST_INDEX),1)
USB_APP := $(SRC_DIR)/ctrl_app.c
endif

ifeq ($(TEST_INDEX),2)
USB_APP := $(SRC_DIR)/bulk_app_lb.c
endif

ifeq ($(TEST_INDEX),9)
USB_APP := $(SRC_DIR)/usb_tty.c
endif

ifeq ($(TEST_INDEX),10)
DEFS += -DTEST_RWP
USB_APP := $(SRC_DIR)/bulk_app_lb.c
endif

USB_APP += $(SRC_DIR)/switch.c

C_SRC += \
   $(DRVSRC) \
   $(PLATFORM_SRC) \
   $(USB_APP)

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
