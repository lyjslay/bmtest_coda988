#-----------------------------------------------------------------------------
# The confidential and proprietary information contained in this file may
# only be used by a person authorised under and to the extent permitted
# by a subsisting licensing agreement from ARM Limited.
#
#            (C) COPYRIGHT 2013-2014 ARM Limited.
#                ALL RIGHTS RESERVED
#
# This entire notice must be reproduced on all copies of this file
# and copies of this file may only be made by a person if such person is
# permitted to do so under the terms of a subsisting license agreement
# from ARM Limited.
#
#      SVN Information
#
#      Checked In          : $Date: 2014-07-25 09:52:11 +0100 (Fri, 25 Jul 2014) $
#
#      Revision            : $Revision: 285788 $
#
#      Release Information : CORTEXA53-r0p4-00rel1
#
#-----------------------------------------------------------------------------
# Modifid for BM1880 by YC Nieh on May 28, 2018, internal use only
# 1. Remove 32-bit supporting
# 2. Add objdump command
#-----------------------------------------------------------------------------

# Include user configuration file

TARMAC         := no    # Enable tarmac trace
OVL_ASSERTIONS := no    # Enable OVL assertions
SIM_64BIT      := no    # Use 64-bit simulation

# Test options. Set to yes to enable, no to disable.
FPU_TESTS      := no    # Build FPU tests
CRYPTO_TESTS   := no    # Build crypto tests

GCC_TOOLCHAIN_AARCH64 ?= aarch64-linux-gnu

################################################################################
# Toolchains
################################################################################

# AArch64 toolchain executables
AS64      := $(GCC_TOOLCHAIN_AARCH64)-as
GCC64     := $(GCC_TOOLCHAIN_AARCH64)-gcc
LD64      := $(GCC_TOOLCHAIN_AARCH64)-ld
OBJCOPY64 := $(GCC_TOOLCHAIN_AARCH64)-objcopy
OBJDUMP64 := $(GCC_TOOLCHAIN_AARCH64)-objdump


################################################################################
# Processor options
################################################################################

# CPU and architecture
CPU  = cortex-a53
ARCH = armv8-a

# FPU types
FPU_NONE      :=
FPU_V8        := -mfpu=neon-fp-armv8
FPU_V8_CRYPTO := -mfpu=crypto-neon-fp-armv8

# Active FPU, can be overridden by tests
FPU = $(FPU_V8)


################################################################################
# Commands and command options
################################################################################

# Assembler setup
ASM64            = $(AS64)
ASM_OPTS_AARCH64 = -march=$(ARCH)

# C compiler setup
CC64             = $(GCC64)
CC_OPTS_AARCH64  = -march=$(ARCH) -mtune=$(CPU)        -c -DGCC -falign-functions=16 -falign-jumps=8 -falign-loops=8 -fomit-frame-pointer -funroll-loops -g

# Linker setup
LINK64           = $(LD64)
LINK_OPTS_64     = 

LINK_OPTS_NOBOOT_AARCH64 = $(LINK_OPTS_64)               -T link_noboot.ld
LINK_OPTS_ASM_AARCH64    = $(LINK_OPTS_64)               -T link_asm_aarch64.ld

LINK_OPTS_CSRC_AARCH64   = $(LINK_OPTS_64:-%=-Wl,-%) -Wl,-T link_csrc_aarch64.ld

# Image file generation
ELF2BIN64 := $(OBJCOPY64)
BIN2HEX   := ../../../c_env/tools/bin2vhx.pl


################################################################################
# Makefile variables
################################################################################

# Names of defined tests for AArch32 and AArch64
aarch64_tests = max_power


# Directories for shared code
aarch64_shared := aarch64/shared
common_shared  := common/shared

# Shared sources and objects
aarch64_bootcode := $(wildcard $(aarch64_shared)/*.s)
aarch64_bootobj  := $(patsubst %.s,%.o,$(aarch64_bootcode))

aarch64_shared_code := $(wildcard $(aarch64_shared)/*.c)
aarch64_shared_c_obj := $(patsubst %.c,%.o,$(aarch64_shared_code))

# Each test has its own Makefile for compiling and linking
sub_makefiles = $(foreach test, $(aarch32_tests), aarch32/$(test)/Makefile.inc) \
                $(foreach test, $(aarch64_tests), aarch64/$(test)/Makefile.inc) \

# List of test directories and basenames for linked ELFs, binaries and objects.
# The basename for each test is <arch>/<test>/<test>
testdirs64  = $(foreach test, $(aarch64_tests), aarch64/$(test))
basenames64 = $(foreach test, $(aarch64_tests), aarch64/$(test)/$(test))

# Names of elfs, binaries and hex files
elfs64 = $(addsuffix .elf, $(basenames64))
bins64 = $(addsuffix .bin, $(basenames64))
hexes  = $(addsuffix .vhx, $(basenames64))
dumps  = $(addsuffix .dump, $(basenames64))


################################################################################
# Default target: make all
################################################################################

.PHONY: all
#all: $(dumps) $(hexes)
all: $(dumps)


################################################################################
# Include sub-Makefiles
#
#   Each sub-Makefile defines how to compile and link the test
################################################################################

include $(sub_makefiles)


################################################################################
# Rules for shared sources
################################################################################

# AArch64 shared ASM
$(aarch64_bootobj): %.o: %.s
	@echo " [ASM ] $<"
	@$(ASM64) $(ASM_OPTS_AARCH64) -I$(common_shared) $< -o $@

# AArch64 shared C
$(aarch64_shared_c_obj): %.o: %.c
	@echo "[C ] $<"
	@$(CC64) $(CC_OPTS_AARCH64) -I$(common_shared) $< -o $@

################################################################################
# Create binary and hex images
################################################################################

$(bins64): %.bin: %.elf
	@echo " [BIN ] $<"
	@$(ELF2BIN64) -j .text -j testcode -j pagetables -O binary $< $@

$(dumps): %.dump: %.elf
	@echo " [DUMP ] $<"
	@$(OBJDUMP64) -dx $< > $@

$(hexes): %.vhx: %.bin
	@echo " [HEX ] $<"
	@$(BIN2HEX) --width=32 $< $@

################################################################################
# Symbolic test names
################################################################################

.PHONY: $(testdirs64)
$(foreach test, $(aarch64_tests), $(eval aarch64/$(test): aarch64/$(test)/$(test).vhx))


################################################################################
# Clean and tidy
#
#   clean: removes everything that can be rebuilt
#   tidy:  preserves ELF and memory initialisation files, cleans everything else
################################################################################

.PHONY: tidy
tidy:
	@rm -f $(aarch64_bootobj)
	@rm -f $(aarch64_shared_c_obj)
	@rm -f $(foreach test, $(aarch64_tests), aarch64/$(test)/*.o)
	@rm -f $(bins64)

.PHONY: clean
clean: tidy
	@rm -f $(elfs64)
	@rm -f $(hexes)
	@rm -f $(dumps)

