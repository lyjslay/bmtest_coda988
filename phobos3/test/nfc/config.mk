#SRC_DIR := $(ROOT)/test/$(TEST_CASE)/src
CORE_DIR := $(ROOT)/test/$(TEST_CASE)/core_driver
PLATFORM_DIR := $(ROOT)/test/$(TEST_CASE)/platform

DRV_SRC := $(CORE_DIR)/src/hpnfc_cdma.c $(CORE_DIR)/src/hpnfc_core.c $(CORE_DIR)/src/hpnfc_main.c $(CORE_DIR)/src/hpnfc_obj_if.c $(CORE_DIR)/src/hpnfc_phy.c 
#DRV_SRC :=  $(CORE_DIR)/src/hpnfc_core.c $(CORE_DIR)/src/hpnfc_main.c $(CORE_DIR)/src/hpnfc_obj_if.c $(CORE_DIR)/src/hpnfc_phy.c
PLATFORM_SRC := $(PLATFORM_DIR)/cps_bm.c $(PLATFORM_DIR)/pattern.c $(PLATFORM_DIR)/request.c

INC += \
   -I$(CORE_DIR)/src \
   -I$(CORE_DIR)/include \
   -I$(CORE_DIR)/src \
   -I$(PLATFORM_DIR)

DEFS += -D__BARE_METAL__

NFC_APP := $(PLATFORM_DIR)/tests.c

C_SRC += \
   $(DRV_SRC) \
   $(PLATFORM_SRC) \
   $(NFC_APP)

CFLAGS := $(filter-out -Werror, $(CFLAGS))

ifneq ($(RUN_ENV), )
CFLAGS += -mstrict-align
endif

CXXFLAGS +=

LDFLAGS +=
