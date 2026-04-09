SHELL=/bin/bash

ROOT := $(shell pwd)
TOOL_PATH := $(shell pwd)/../..

DEBUG := 0

USE_BMTAP ?= 1
USE_BMDNN ?= 0
# ASIC, FPGA ,QEMU, ZEBU or PALLADIUM
BOARD     ?= ASIC
CHIP      ?= MARS
ARCH      ?= ARM
DDR_CONFIG = lpddr4_8g_2666
# DDR_CONFIG = ddr3_4g_1866
# DDR_CONFIG = ddr3_4g_1866_odt
# DDR_CONFIG = ddr3_4g_1066

ifeq ($(ARCH), RISCV)
CROSS_COMPILE := $(TOOL_PATH)/host-tools/gcc/riscv64-elf-x86_64/bin/riscv64-unknown-elf-
else
CROSS_COMPILE := $(TOOL_PATH)/host-tools/gcc/gcc-linaro-6.3.1-2017.05-x86_64_aarch64-elf/bin/aarch64-elf-
endif
OUTPUT_NAME := mars
OUTPUT_PATH := out
RELEASE_PATH := release_out

BMTAP_INC_PATH := $(HOME)/bmtap2/install/include
BMTAP_LIB_PATH := $(HOME)/bmtap2/install/lib

include config.mk

ifneq ($(TEST_CASE), )
empty   :=
space   := $(empty) $(empty)
OUTPUT_NAME := $(OUTPUT_NAME)_$(BOARD)_$(strip $(subst $(space),_,$(TEST_CASE)))${SUB_TEST_CASE}
C_SRC += $(wildcard $(TEST_CASE:%=$(ROOT)/test/%/*.c))
C_SRC += $(wildcard $(TEST_CASE:%=$(ROOT)/test/%/$(CHIP)/*.c))
configs := $(foreach testcase,$(TEST_CASE),$(ROOT)/test/$(testcase)/config.mk)
ifneq ($(configs), )
sinclude $(configs)
endif
endif

ifneq ($(SLT_TEST_CASE), )
DEFS += -DBUILD_TEST_CASE_all
DEFS += $(foreach testcase,$(SLT_TEST_CASE),-DBUILD_TEST_CASE_SLT_$(testcase))
C_SRC += $(wildcard $(foreach testcase,$(SLT_TEST_CASE),$(testcase:%=$(ROOT)/test/%/*.c)))
C_SRC += $(wildcard $(foreach testcase,$(SLT_TEST_CASE),$(testcase:%=$(ROOT)/test/%/$(CHIP)/*.c)))
configs := $(foreach testcase,$(SLT_TEST_CASE),$(ROOT)/test/$(testcase)/config.mk)
ifneq ($(configs), )
sinclude $(configs)
endif
endif

AS := $(CROSS_COMPILE)as
CC := $(CROSS_COMPILE)gcc
CXX := $(CROSS_COMPILE)g++
LD := $(CROSS_COMPILE)ld
OBJCOPY := $(CROSS_COMPILE)objcopy
OBJDUMP := $(CROSS_COMPILE)objdump
MKDIR := mkdir -p
RM := rm -rf
CP := cp -r
SSEC := *.slow*

EXT_C_OBJ := $(patsubst %,   %.o, $(EXT_C_SRC))
A_OBJ   := $(addprefix $(OUTPUT_PATH)/, $(patsubst $(ROOT)/%,   %.o, $(A_SRC)))
C_OBJ   := $(addprefix $(OUTPUT_PATH)/, $(patsubst $(ROOT)/%,   %.o, $(C_SRC)))
CXX_OBJ := $(addprefix $(OUTPUT_PATH)/, $(patsubst $(ROOT)/%,   %.o, $(CXX_SRC)))
BIN_OBJ := $(addprefix $(OUTPUT_PATH)/, $(patsubst $(ROOT)/%,   %.o, $(BIN_SRC)))

MAP	:= $(OUTPUT_PATH)/$(OUTPUT_NAME).map
ELF	:= $(OUTPUT_PATH)/$(OUTPUT_NAME).elf
BIN	:= $(OUTPUT_PATH)/$(OUTPUT_NAME).bin
ASM	:= $(OUTPUT_PATH)/$(OUTPUT_NAME).asm

CFLAGS += -g

all: $(OUTPUT_PATH) $(BIN) $(ASM)

.PHONY: $(USER_PHONY) $(OUTPUT_PATH) clean release all_src all_def all_inc all_flag

$(OUTPUT_PATH):
	$(MKDIR) $(OUTPUT_PATH) $(dir $(A_OBJ)) $(dir $(C_OBJ)) $(dir $(CXX_OBJ)) $(dir $(MAP)) $(dir $(ELF)) $(dir $(BIN)) $(dir $(ASM)) $(dir $(BIN_OBJ))

clean: $(USER_CLEAN)
	$(RM) $(OUTPUT_PATH)

release: all
	$(MKDIR) $(RELEASE_PATH)
	$(CP) $(MAP) $(RELEASE_PATH)/$(OUTPUT_NAME)_$(shell date "+%F-%H-%M-%S").map
	$(CP) $(ELF) $(RELEASE_PATH)/$(OUTPUT_NAME)_$(shell date "+%F-%H-%M-%S").elf
	$(CP) $(BIN) $(RELEASE_PATH)/$(OUTPUT_NAME)_$(shell date "+%F-%H-%M-%S").bin
	$(CP) $(ASM) $(RELEASE_PATH)/$(OUTPUT_NAME)_$(shell date "+%F-%H-%M-%S").asm

$(EXT_C_OBJ): %.o: %
	"$(CC)" -c $(DEFS) $(CFLAGS) $< -o $@ $(INC)

$(A_OBJ): $(OUTPUT_PATH)/%.o: $(ROOT)/%
	"$(CC)" -c $(DEFS) $(ASFLAGS) $< -o $@ $(INC)

$(C_OBJ): $(OUTPUT_PATH)/%.o: $(ROOT)/%
	"$(CC)" -c $(DEFS) $(CFLAGS) $< -o $@ $(INC)

$(CXX_OBJ): $(OUTPUT_PATH)/%.o: $(ROOT)/%
	"$(CXX)" -c $(DEFS) $(CXXFLAGS) $< -o $@ $(INC)

$(BIN_OBJ): $(OUTPUT_PATH)/%.o: $(ROOT)/%
	"$(OBJCOPY)" -I binary -O elf64-littleaarch64 --binary-architecture aarch64 $< $@ 
	"$(OBJCOPY)" --redefine-sym _binary_`echo $< | sed -z 's/[/.]/_/g'`_start=_binary_`echo $* | sed -z 's/[/.]/_/g'`_start $@

$(ELF): $(A_OBJ) $(EXT_C_OBJ) $(C_OBJ) $(CXX_OBJ) $(LIB_OBJ) $(BIN_OBJ)
	#touch $(ROOT)/test/$(TEST_CASE)/testcase_$(TEST_CASE).c
	echo "const char fw_version_info[] = \"`git rev-parse HEAD`\";" > common/version.c
	$(CC) -T$(LINK) -o $@ -Wl,-Map,$(MAP) $(LDFLAGS) \
	-Wl,--start-group \
	$^ \
	-lm ${_LDFLAGS} \
	-Wl,--end-group

$(BIN): $(ELF)
	$(OBJCOPY) -O binary -R $(SSEC) $^ $@

$(ASM): $(ELF)
	$(OBJDUMP) -DS $^ > $@

all_obj:
	@echo "ASM OBJ:<"$(A_OBJ)">"
	@echo "EXT C OBJ:<"$(EXT_C_OBJ)">"
	@echo "C OBJ:<"$(C_OBJ)">"
	@echo "CXX OBJ:<"$(CXX_OBJ)">"

all_src:
	@echo "ASM SRC:<"$(A_SRC)">"
	@echo "C SRC:<"$(C_SRC)">"
	@echo "CXX SRC:<"$(CXX_SRC)">"

all_def:
	@echo $(DEFS)

all_inc:
	@echo $(INC)

all_flag:
	@echo "LDFLAGS:<"$(LDFLAGS)">"
	@echo "ASFLAGS:<"$(ASFLAGS)">"
	@echo "CFLAGS:<"$(CFLAGS)">"
