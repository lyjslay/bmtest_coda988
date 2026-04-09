OPTFLAGS := \
	-O0 \
	-Wall \
	-Werror

ifeq ($(ARCH), RISCV)
COMMONFLAGS := \
	-march=rv64imafdc -mcmodel=medany -mabi=lp64d
else
COMMONFLAGS ?= \
	-march=armv8-a
endif

ifeq ($(BOARD), ZEBU)
DEFS += -DBOARD_ZEBU
DEFS += -DBOARD_PALLADIUM
endif

ifeq ($(BOARD), ASIC)
DEFS += -DBOARD_ASIC
DEFS += -DSUBTYPE_ASIC
endif

ifeq ($(BOARD), FPGA)
DEFS += -DBOARD_FPGA
DEFS += -DSUBTYPE_FPGA
endif

ifeq ($(BOARD), PALLADIUM)
DEFS += -DBOARD_PALLADIUM
DEFS += -DSUBTYPE_PALLADIUM
endif

ifeq ($(BOARD), QEMU)
DEFS += -DBOARD_QEMU
endif

ifeq ($(DEBUG), 1)
DEFS += -g -DENABLE_DEBUG
endif

ifeq ($(SPI_NOR1), 1)
DEFS += -DSPI_NOR1
endif

ifeq ($(SDIO), SDIO0)
DEFS += -DTEST_SD0
endif

ifeq ($(SDIO), SDIO1)
DEFS += -DTEST_SD1
endif

ifeq ($(SDIO1_PAD), FIRST_PAD)
DEFS += -DSDIO1_1ST_PAD
endif

ifeq ($(SDIO1_PAD), SECOND_PAD)
DEFS += -DSDIO1_2ND_PAD
endif

DEFS += -DUSE_BMTAP
DEFS += -DCONFIG_CHIP_BM1882
#DEFS += -DBUILD_TEST_CASE_$(TEST_CASE)

ASFLAGS := \
	$(OPTFLAGS) \
	$(COMMONFLAGS) \
	-std=gnu99

CFLAGS := \
	$(COMMONFLAGS) \
	$(OPTFLAGS) \
	-ffunction-sections \
	-fdata-sections \
	-nostdlib \
	-std=gnu99

CXXFLAGS := \
	$(COMMONFLAGS) \
	$(OPTFLAGS) \
	-ffunction-sections \
	-fdata-sections \
	-nostdlib \
	-fno-threadsafe-statics \
	-fno-rtti \
	-fno-exceptions \
	-std=c++14

LDFLAGS := \
	$(COMMONFLAGS) \
	-Wl,--build-id=none \
	-Wl,--cref \
	-Wl,--check-sections \
	-Wl,--gc-sections \
	-Wl,--unresolved-symbols=report-all \
	-nostartfiles

INC += \
	-I$(ROOT)/include \
	-I$(ROOT)/drivers/include \
	-I$(ROOT)/test

C_SRC += \
	$(ROOT)/common/main.c                   \
	$(ROOT)/common/version.c                \
	$(ROOT)/common/axi_mon.c                \
	$(ROOT)/common/cli.c                    \
	$(ROOT)/common/crc16.c                  \
	$(ROOT)/common/ddr.c                    \
	$(ROOT)/common/phy_pll_init.c           \
	$(ROOT)/common/ddr_sys.c                \
	$(ROOT)/common/command.c                \
	$(ROOT)/common/system.c                 \
	$(ROOT)/common/syscalls.c               \
	$(ROOT)/common/pll.c                    \
	$(ROOT)/common/timer.c                  \
	$(ROOT)/common/md5.c                    \
	$(ROOT)/common/dma.c \
	$(ROOT)/common/common.c

DDR_CONFIG ?= lpddr4_8g_2666
# DDR_CONFIG ?= ddr3_4g_1866
# DDR_CONFIG ?= ddr3_4g_1866_odt
# DDR_CONFIG ?= ddr3_4g_1066

# DDR configurations
ifeq ($(DDR_CONFIG), ddr3_4g_1066)
C_SRC  += $(ROOT)/ddr_config/regconfig_ddr3_4g_1066.c
DEFS += -DDDR3
DEFS += -D_mem_freq_2133
else ifeq ($(DDR_CONFIG), ddr3_4g_1866)
C_SRC  += $(ROOT)/ddr_config/regconfig_ddr3_4g_1866.c
DEFS += -DDDR3
DEFS += -D_mem_freq_1866
else ifeq ($(DDR_CONFIG), ddr3_4g_1866_odt)
C_SRC  += $(ROOT)/ddr_config/regconfig_ddr3_4g_1866_odt.c
DEFS += -DDDR3
DEFS += -DODT
DEFS += -D_mem_freq_2133
else ifeq ($(DDR_CONFIG), lpddr4_8g_2666)
C_SRC  += $(ROOT)/ddr_config/regconfig_lpddr4_8g_2666.c
DEFS += -DLP4
DEFS += -DODT
# DEFS += -DDBI_OFF
DEFS += -D_mem_freq_2666
else ifeq ($(DDR_CONFIG), ddr4_2666)
C_SRC  += $(ROOT)/ddr_config/regconfig_ddr4_2666.c
DEFS += -DDDR4
DEFS += -D_mem_freq_2666
endif

ifeq ($(ARCH), RISCV)
C_SRC  += $(ROOT)/common/interrupts.c
ifeq ($(RISCV_TYPE), C906B)
CFLAGS += -DCONFIG_C906B
else ifeq ($(RISCV_TYPE), C906L)
CFLAGS += -DCONFIG_C906L
else
$(error "Error : no define RISCV_TYPE: C906B/C906L type")
endif
CFLAGS += -DCONFIG_64BIT
endif

include $(ROOT)/drivers/config.mk

ifeq ($(INIT_DDR), YES)
DEFS += -DINIT_DDR
endif

ifeq ($(RUN_ENV), SRAM)
DEFS += -DRUN_IN_SRAM
else
DEFS += -DRUN_IN_DDR
endif

ifeq ($(BOARD), QEMU)
C_SRC += $(ROOT)/common/uart_pl01x.c
else
DEFS += -DUART_INTR_TEST
C_SRC += $(ROOT)/common/uart_dw.c
endif

A_SRC +=

ifneq ($(BMTAP_SRC_PATH), $(wildcard $(BMTAP_SRC_PATH)))
$(error invalid BMTAP_SRC_PATH "$(BMTAP_SRC_PATH)")
endif

INC += \
	-I$(BMTAP_INC_PATH) \
	-I$(BMTAP_INC_PATH)/../../libbmutils/include \
	-I$(BMTAP_INC_PATH)/../../bmkernel/include/bm188x

include $(ROOT)/boot/config.mk
include $(ROOT)/common/stdlib/stdlib.mk

