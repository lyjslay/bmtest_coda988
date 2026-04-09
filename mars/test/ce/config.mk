TOM_CRYPT=$(ROOT)/../bm1684/third_party/libtomcrypt

LIB_OBJ+=$(TOM_CRYPT)/libtomcrypt.a
CE_INCLUDE=-I$(TOM_CRYPT)/src/headers -I$(ROOT)/test/ce

DEFS +=
CFLAGS += -mstrict-align -Wno-unused $(CE_INCLUDE)
CXXFLAGS +=
LDFLAGS +=

