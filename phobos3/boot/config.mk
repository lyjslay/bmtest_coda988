ifeq ($(RUN_ENV), SRAM)
	LINK := $(ROOT)/boot/phobos3rv_sram.lds
	A_SRC += $(ROOT)/boot/riscv_start.S
else
ifeq ($(ARCH), RISCV)
ifeq ($(RISCV_TYPE), C906B)
	LINK := $(ROOT)/boot/phobosrv_ddr.lds
else ifeq ($(RISCV_TYPE), C906L)
	LINK := $(ROOT)/boot/phobosrv_c906l_ddr.lds
else
	$(error "Error : no define RISCV_TYPE: C906B/C906L type")
endif
	A_SRC += $(ROOT)/boot/riscv_start.S
else
	LINK := $(ROOT)/boot/phobos_ddr.lds
	A_SRC += $(ROOT)/boot/start_alone.S
endif
endif
