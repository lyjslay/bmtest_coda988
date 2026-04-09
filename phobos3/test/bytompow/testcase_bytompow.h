/* ustd.h common macros and includes */
#ifndef TESTCASE_BYTOMPOW_H
#define TESTCASE_BYTOMPOW_H

#include "utils.h"

#define TEST_NUM                1
#define TRY_NONCE_MATRIX_LIST_LENGTH  256

static const int FNV_PRIME = 0x01000193;
#define FNV(v1,v2) ((int32_t) ((v1)*FNV_PRIME) ^ (v2))

static uint8_t golden_msg[TEST_NUM][32] = {
    {
      0xd0, 0xda, 0xd7, 0x3f, 0xb2, 0xda, 0xbf, 0x33,
      0x53, 0xfd, 0xa1, 0x55, 0x71, 0xb4, 0xe5, 0xf6,
      0xac, 0x62, 0xff, 0x18, 0x7b, 0x35, 0x4f, 0xad,
      0xd4, 0x84, 0x0d, 0x9f, 0xf2, 0xf1, 0xaf, 0xdf,
    },
};

#if 0
static uint8_t golden_seed[TEST_NUM][32] = {
    {
      0x07, 0x37, 0x52, 0x07, 0x81, 0x34, 0x5b, 0x11,
      0xb7, 0xbd, 0x0f, 0x84, 0x3c, 0x1b, 0xdd, 0x9a,
      0xea, 0x81, 0xb6, 0xda, 0x94, 0xfd, 0x14, 0x1c,
      0xc9, 0xf2, 0xdf, 0x53, 0xac, 0x67, 0x44, 0xd2,
    },
};
#endif

static uint8_t golden_result[TEST_NUM][32] = {
    {
      0xe3, 0x5d, 0xa5, 0x47, 0x95, 0xd8, 0x2f, 0x85,
      0x49, 0xc0, 0xe5, 0x80, 0xcb, 0xf2, 0xe3, 0x75,
      0x7a, 0xb5, 0xef, 0x8f, 0xed, 0x1b, 0xdb, 0xe4,
      0x39, 0x41, 0x6c, 0x7e, 0x6f, 0x8d, 0xf2, 0x27,
    },
};

typedef union {
    uint32_t       u32Val;
    uint8_t        u8Val[4];
} u32to8_t;

typedef struct _Words16 {
    uint32_t w[16];
}Words_16;

typedef struct _Words32 {
  Words_16 lo, hi;
}Words_32;

typedef struct _LTCMemory {
    Words_32 w32[1024];
}LTC_Memory;

extern void
dumpArrays32(uint32_t *arrary, uint32_t lda, uint32_t ldb);

extern void
memdump(uint8_t *addr, uint32_t len);

extern void
dumpArrays8(uint8_t *arrary, uint32_t lda, uint32_t ldb);

extern uint32_t Words32GetWord(Words_32 *w32, uint32_t i);

extern void Words32_xor_other(Words_32 *w32, Words_32 *other);

extern Words_32 *LTCMemoryGet(LTC_Memory *ltc, uint32_t i);

extern void
LTCMemoryScrypt(LTC_Memory *ltc, Words_32 *w32);

extern void copyFrom_helper(int8_t *mat, LTC_Memory *ltcMem, int offset);

extern void copyFromEven(int8_t *mat, LTC_Memory *ltcMem);

extern void copyFromOdd(int8_t *mat, LTC_Memory *ltcMem);

extern void btm_seed_init(Words_32 *seed, uint32_t _seed[32]);

extern void
MatrixToIdentify(int8_t *A);

extern void
MatrixListToIdentify(int8_t *A, uint32_t listlen);

extern void
allocte_squre_matrix_list(int8_t **list, uint32_t len);

extern void
allocte_squre_identify_matrix(int8_t **mat);

extern int8_t* GetMatrixAddr(int8_t *matrixlist, uint32_t idx);

extern void
init_matrix_list_with_seed(int8_t *matrixlist, Words_32 *seed);

extern void
matrix_resize(int8_t *mat, uint32_t *data);

extern void
matrix_add_square256(int8_t *A, int8_t *B, int8_t *C);

extern void
matrix_mul_square256(int8_t *A, int8_t *B, int8_t *C);

extern void
memdump(uint8_t *addr, uint32_t len);

extern void
MatrixCheckIdentity(int8_t *A, int8_t value);

extern void
MatrixToIdentifyMul2(int8_t *A);

extern void
MatrixToIdentifyMul3(int8_t *A);

extern void
test_bytomPoW_batch(bmctx_t *bmctx);


#endif /* LIBRHASH_USTD_H */