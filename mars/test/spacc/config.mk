INC +=  -I$(ROOT)/test/spacc -I$(ROOT)/test/spacc/efuse
C_SRC +=$(ROOT)/test/spacc/efuse/efuse.c 



ASFLAGS += -D__ASSEMBLY__

CFLAGS += -mstrict-align -Wno-unused

CXXFLAGS +=

LDFLAGS +=
