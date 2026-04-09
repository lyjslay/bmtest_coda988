DEFS +=

INC += -I$(ROOT)/test/security

A_SRC += $(ROOT)/test/security/switch_el.S

ASFLAGS += -D__ASSEMBLY__

CFLAGS += -mstrict-align -Wno-unused

CXXFLAGS +=

LDFLAGS +=
