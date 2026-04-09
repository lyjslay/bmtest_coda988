SRC_DIR := $(ROOT)/test/usb/src
CORE_DIR := $(ROOT)/test/usb/CoreDriver
PLATFORM_DIR := $(ROOT)/test/usb/platform/vsp

DRVSRC := $(CORE_DIR)/src/ss_dev.c $(CORE_DIR)/src/sgdma.c
PLATFORM_SRC := $(PLATFORM_DIR)/cps_bm.c

INC += \
   -I$(CORE_DIR)/src \
   -I$(CORE_DIR)/include \
   -I$(CORE_DIR)/src	 \
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

ifeq ($(TEST_INDEX),3)
USB_APP := $(SRC_DIR)/multibulk_app_lb.c
endif

ifeq ($(TEST_INDEX),4)
USB_APP := $(SRC_DIR)/int_app.c
endif

ifeq ($(TEST_INDEX),5)
USB_APP := $(SRC_DIR)/iso_app.c
endif

ifeq ($(TEST_INDEX),6)
USB_APP := $(SRC_DIR)/devMSC/bot_app.c $(SRC_DIR)/devMSC/scsi.c $(SRC_DIR)/devMSC/storage.c
endif

ifeq ($(TEST_INDEX),7)
USB_APP := $(SRC_DIR)/hid_app.c
endif

ifeq ($(TEST_INDEX),8)
USB_APP := $(SRC_DIR)/devUAC/mic_app.c $(SRC_DIR)/devUAC/mic10.c $(SRC_DIR)/devUAC/mic20.c
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

ifneq ($(NEW_DMA), )
DEFS += -DNEW_DMA=$(NEW_DMA)
else
NEW_DMA := 0
DEFS += -DNEW_DMA=0
endif

CXXFLAGS +=

LDFLAGS +=
