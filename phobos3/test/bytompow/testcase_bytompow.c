#include "boot_test.h"
#include "1880_test_util.h"
#include "utils.h"
#include "testcase_bytompow.h"
#include "sha3.h"
#include "srypt.h"
#include "timer.h"

/*
git push origin HEAD:refs/for/master
scp /home/johnny/Workspace/bm1880/u-boot/examples/bmtest/bm1880/out/bm1880_bytompow.elf bitmain@10.11.23.99:/home/bitmain/Downloads/li.jiang01
scp /home/johnny/Workspace/bm1880/u-boot/examples/bmtest/bm1880/out/bm1880_bytompow.elf bm@10.11.23.124:/home/bm/Downloads/li.jiang01
make TEST_CASE=bytompow BOARD=PALLADIUM RUN_ENV=DDR
*/

void
dumpArrays32(uint32_t *arrary, uint32_t lda, uint32_t ldb)
{
    for (int i = 0; i < lda; i++) {
        for (int j = 0; j < ldb; j++) {
            printf("%08x ", arrary[ldb * i + j]);
        }
        printf("\n");
    }
}

void
memdump(uint8_t *addr, uint32_t len)
{
    uint8_t ldb = 64;

    if (len > ldb) {
        for (int i = 0; i < len / ldb; i++) {
            for (int j = 0; j < ldb; j++) {
                printf("%02x ", addr[ldb * i + j]);
            }
        }
    }
    else {
        for (int j = 0; j < len; j++) {
            printf("%02x ", addr[j]);
        }
    }
    printf("\n");
}

void
dumpArrays8(uint8_t *arrary, uint32_t lda, uint32_t ldb)
{
    for (int i = 0; i < lda; i++) {
        for (int j = 0; j < ldb; j++) {
            printf("%02x ", arrary[256 * i + j]);
        }
        printf("\n");
    }
}

int8_t *maxtix_list;
int8_t *identify_matrix;

uint8_t compare_result = 0;
uint8_t process = 0;


uint8_t *g_left_data;
uint8_t *g_right_data;
uint8_t *g_res_data;

#if 0
static void tl_matrix_mac_bytom(bmctx_t *ctx, uint8_t *left_data, uint8_t *right_data, uint8_t *C)
{
  /*
   * Hardware supports 255x256 matrice only.
   */
    uint8_t *res_data;

    g_right_data = right_data;
    g_left_data = left_data;
    g_res_data = C;

    shape_t left_shape = shape_t4(1, 256, 1, 256);
    shape_t right_shape = shape_t4(256, 16, 1, 16);
    shape_t res_shape = shape_t4(1, 256, 1, 256);

    printf("start mul\n");

    tensor_lmem *tl_left = tl_alloc(left_shape, DATA_FMT_I8, CTRL_AL);
    if (tl_left == 0) {
        printf("can not allocate left\n");
    }

    tensor_lmem *tl_right = tl_alloc(right_shape, DATA_FMT_I8, CTRL_AL);
    if (tl_right == 0) {
        printf("can not allocate right\n");
    }

    tensor_lmem *tl_res = tl_alloc(res_shape, DATA_FMT_I8, CTRL_AL);
    if (tl_res == 0) {
        printf("can not allocate res\n");
    }

    put_tensor(ctx, tl_left, left_data);
    printf("put left\n");

    put_tensor(ctx, tl_right, right_data);
    printf("put right\n");

    tl_matrix_mac_2_fixed(tl_res, tl_left, tl_right);
    printf("tl_matrix_mac_2_fixed\n");

    res_data = get_tensor(ctx, tl_res);
    printf("get tensor\n");

    memcpy(C, res_data, 256*256);

    tl_free(tl_res);
    tl_free(tl_right);
    tl_free(tl_left);
}
#endif

#if 1
tensor_lmem *tl_left;
tensor_lmem *tl_right;
tensor_lmem *tl_res;

void
tl_res_allocte(void)
{
    shape_t left_shape = shape_t4(1, 256, 1, 256);
    shape_t right_shape = shape_t4(256, 16, 1, 16);
    shape_t res_shape = shape_t4(1, 256, 1, 256);

    printf("tl_res_allocte\n");

    tl_left = tl_alloc(left_shape, DATA_FMT_I8, CTRL_AL);
    if (tl_left == 0) {
        printf("can not allocate left\n");
    }

    printf("tl_left %p\n", tl_left);

    tl_right = tl_alloc(right_shape, DATA_FMT_I8, CTRL_AL);
    if (tl_right == 0) {
        printf("can not allocate right\n");
    }
    printf("tl_right %p\n", tl_right);

    tl_res = tl_alloc(res_shape, DATA_FMT_I8, CTRL_AL);
    if (tl_res == 0) {
        printf("can not allocate res\n");
    }
    printf("tl_res %p\n", tl_res);
}

static void tl_matrix_mac_bytom(bmctx_t *ctx, uint8_t *left_data, uint8_t *right_data, uint8_t *C)
{
  /*
   * Hardware supports 255x256 matrice only.
   */
    uint8_t *res_data;

    g_right_data = right_data;
    g_left_data = left_data;
    g_res_data = C;

    printf("start mul tick %lu\n", timer_get_tick());

    put_tensor(ctx, tl_left, left_data);
    printf("put left\n");

    put_tensor(ctx, tl_right, right_data);
    printf("put right\n");

    tl_matrix_mac_2_fixed(tl_res, tl_left, tl_right);
    printf("tl_matrix_mac_2_fixed\n");

    res_data = get_tensor(ctx, tl_res);
    printf("get tensor\n");

    memcpy(C, res_data, 256 * 256);
}
#endif

uint32_t Words32GetWord(Words_32 *w32, uint32_t i)
{
    if (i < 16) {
        return w32->lo.w[i];
    }
    else if (i < 32) {
        return w32->hi.w[i - 16];
    }
    else {
        assert(false);
        return 0;
    }
}

void Words32_xor_other(Words_32 *w32, Words_32 *other)
{
    for(int i = 0; i < 16; i++) {
        w32->lo.w[i]^=other->lo.w[i];
    }

    for(int i=0; i < 16; i++) {
        w32->hi.w[i]^=other->hi.w[i];
    }
}


Words_32 *LTCMemoryGet(LTC_Memory *ltc, uint32_t i)
{
    return &ltc->w32[i];
}

void
LTCMemoryScrypt(LTC_Memory *ltc, Words_32 *w32)
{
    for (int i = 0; i < 1024; i++) {
        ltc->w32[i] = *w32;
        xor_salsa8(w32->lo.w, w32->hi.w);
        xor_salsa8(w32->hi.w, w32->lo.w);
    }

    for (int i = 0; i < 1024; i++) {
      int j = w32->hi.w[0] & 1023;
      Words32_xor_other(w32, &ltc->w32[j]);
      xor_salsa8(w32->lo.w, w32->hi.w);
      xor_salsa8(w32->hi.w, w32->lo.w);
    }
}

void copyFrom_helper(int8_t *mat, LTC_Memory *ltcMem, int offset)
{
    for (int i = 0; i < 256; i++) {
        Words_32 *lo = LTCMemoryGet(ltcMem, i * 4 + offset);
        Words_32 *hi = LTCMemoryGet(ltcMem, i * 4 + 2 + offset);

        for (int j = 0; j < 64; j++) {
            uint32_t i32 = (j>= 32 ? Words32GetWord(hi, j - 32) : Words32GetWord(lo, j));
            mat[(j * 4 + 0) * 256 + i] = (i32 >> 0) & 0xFF;
            mat[(j * 4 + 1) * 256 + i] = (i32 >> 8) & 0xFF;
            mat[(j * 4 + 2) * 256 + i] = (i32 >> 16) & 0xFF;
            mat[(j * 4 + 3) * 256 + i] = (i32 >> 24) & 0xFF;
        } 
    }
}

void copyFromEven(int8_t *mat, LTC_Memory *ltcMem) {
  copyFrom_helper(mat, ltcMem, 0);
}

void copyFromOdd(int8_t *mat, LTC_Memory *ltcMem) {
  copyFrom_helper(mat, ltcMem, 1);
}

void btm_seed_init(Words_32 *seed, uint32_t _seed[32])
{
    for (int i = 0; i < 16; i++) {
        seed->lo.w[i] = _seed[i];
    }

    for (int i = 0; i < 16; i++) {
        seed->hi.w[i] = _seed[16 + i];
    }
}

#if 0
static void extend(uint32_t *exted, uint8_t *g_seed) {
    uint8_t u8SeedHash[4][32];
    u32to8_t eData;
    sha3_ctx *ctx = (sha3_ctx*)malloc(sizeof(sha3_ctx));
  
    memcpy(u8SeedHash[0], g_seed, 32);
    for (int i = 0; i < 3; ++i) {
	    rhash_sha3_256_init(ctx);
	    rhash_sha3_update(ctx, u8SeedHash[i], 32);
	    rhash_sha3_final(ctx, u8SeedHash[i+1]);
    }

    for (int i = 0; i < 32; ++i) {
        eData.u8Val[0] = u8SeedHash[i / 8][(i * 4 + 0) % 32];
        eData.u8Val[1] = u8SeedHash[i / 8][(i * 4 + 1) % 32];
        eData.u8Val[2] = u8SeedHash[i / 8][(i * 4 + 2) % 32];
        eData.u8Val[3] = u8SeedHash[i / 8][(i * 4 + 3) % 32];
        exted[i] = eData.u32Val;
    }
    free(ctx);
}
#endif

void
MatrixCheckIdentity(int8_t *A, int8_t value)
{
    uint32_t i;

    for (i = 0; i < 256; i++) {
        //printf("A[%d] set to 1\n", i * 256 + i);
        if (A[i * 256 + i] != value) {
            printf("check matrix error at i %d expect %d real %d\n", i, value, A[i * 256 + i]);
        }
    }
}

void
MatrixToIdentifyMul2(int8_t *A)
{
    uint32_t i;

    //printf("MatrixToIdentify memset %p\n", A);

    memset(A, 0, sizeof(256 * 256));

    //printf("MatrixToIdentify memset %p complete\n", A);

    for (i = 0; i < 256; i++) {
        //printf("A[%d] set to 1\n", i * 256 + i);
        A[i * 256 + i] = 2;
    }

    //printf("init idt end\n");
}

void
MatrixToIdentifyMul3(int8_t *A)
{
    uint32_t i;

    //printf("MatrixToIdentify memset %p\n", A);

    memset(A, 0, sizeof(256 * 256));

    //printf("MatrixToIdentify memset %p complete\n", A);

    for (i = 0; i < 256; i++) {
        //printf("A[%d] set to 1\n", i * 256 + i);
        A[i * 256 + i] = 3;
    }

    //printf("init idt end\n");
}

void
MatrixToIdentify(int8_t *A)
{
    uint32_t i;

    //printf("MatrixToIdentify memset %p\n", A);

    memset(A, 0, sizeof(256 * 256));

    //printf("MatrixToIdentify memset %p complete\n", A);

    for (i = 0; i < 256; i++) {
        //printf("A[%d] set to 1\n", i * 256 + i);
        A[i * 256 + i] = 1;
    }

    //printf("init idt end\n");
}

void
MatrixListToIdentify(int8_t *A, uint32_t listlen)
{
    int i, offset;

    for (i = 0; i < listlen; i++) {
        offset = i * 256 * 256;
        MatrixToIdentify(&A[offset]);
    }
}

void
allocte_squre_matrix_list(int8_t **list, uint32_t len)
{
    *list = (int8_t *)0x00000001c1000000;

    printf("256 256 * 256 matrixs is allocated at %p sz %d\n", *list, len * 256 * 256);
}

void
allocte_squre_identify_matrix(int8_t **mat)
{
    *mat = (int8_t *)xmalloc(256 * 256);

    printf("allocte_squre_identify_matrix at %p\n", *mat);

    MatrixToIdentify(*mat);

    printf("1 256 * 256 idt matrixs is allocated at %p sz %d\n", *mat, 256 * 256);
}

int8_t* GetMatrixAddr(int8_t *matrixlist, uint32_t idx)
{
    assert(idx < 256);
    return &matrixlist[idx * 256 * 256];
}

LTC_Memory gLtcMem;
uint32_t init_idx;

void
init_matrix_list_with_seed(int8_t *matrixlist, Words_32 *seed)
{
    int8_t *p;

    //printf("dump srypted matrix list: \n");

    for(init_idx = 0; init_idx < 128; init_idx++) {
        //printf("%d mat\n", i);
        LTCMemoryScrypt(&gLtcMem, seed);
        p = GetMatrixAddr(matrixlist, 2 * init_idx);
        copyFromEven(p, &gLtcMem);
        //dumpArrays8(p, 256, 256);

        p = GetMatrixAddr(matrixlist, 2 * init_idx + 1);
        copyFromOdd(p, &gLtcMem);
        //dumpArrays8(p, 256, 256);

        //while(1);
    }
}

void
matrix_resize(int8_t *mat, uint32_t *data)
{
    uint32_t (*resize_matrix)[256][64] = (uint32_t(*)[256][64])xmalloc(256 * 64 * sizeof(uint32_t));

    //printf("start resize\n");

#if 0
    for(int j=0; j<256; j++) {
        for(int i=0; i<64; i++) {
            (*resize_matrix)[j][i] = ((uint32_t(uint8_t(mat[j * 256 + i  + 192]))) << 24) |
            ((uint32_t(uint8_t(mat[j * 256 + i  + 128]))) << 16) |
            ((uint32_t(uint8_t(mat[j * 256 + i  + 64]))) << 8) |
            ((uint32_t(uint8_t(mat[j * 256 + i  + 0]))) << 0);
        }
    }
#endif

    for(int j=0; j<256; j++) {
        for(int i=0; i<64; i++) {
            (*resize_matrix)[j][i] = (((uint32_t)((uint8_t)(mat[j * 256 + i  + 192]))) << 24) |
            (((uint32_t)((uint8_t)(mat[j * 256 + i  + 128]))) << 16) |
            (((uint32_t)((uint8_t)(mat[j * 256 + i  + 64]))) << 8) |
            (((uint32_t)((uint8_t)(mat[j * 256 + i  + 0]))) << 0);
        }
    }

    //dumpArrays8((uint8_t *)resize_matrix, 16, 16);

    //printf("start resize2\n");

    for(int k=256; k>1; k=k/2) {
        for(int j=0; j<k/2; j++) {
            for(int i=0; i<64; i++) {
                (*resize_matrix)[j][i]=FNV((*resize_matrix)[j][i], (*resize_matrix)[j+k/2][i]);
            }
        }
    }

    //dumpArrays8((uint8_t *)resize_matrix, 16, 16);

    //printf("start reduce\n");

    for(int i=0; i < 64; i++) {
        data[i]=(*resize_matrix)[0][i];
    }

    free(resize_matrix);

    //dumpArrays8((uint8_t *)resize_matrix, 16, 16);

    //printf("reduce complete\n");
}

void
matrix_add_square256(int8_t *A, int8_t *B, int8_t *C)
{
    for (int i = 0; i < 256; i++) {
        for (int j = 0; j < 256; j++) {
            int tmp = (int)A[i * 256 + j] + (int)B[i * 256 + j];
            C[i * 256 + j] = tmp & 0xFF;
        }
    }
}

void
matrix_mul_square256(int8_t *A, int8_t *B, int8_t *C)
{
    for (int i = 0; i < 256; i++) {
        for (int j = 0; j < 256; j++) {
            int tmp = 0;
            for (int k = 0; k < 256; k++) {
                tmp += (int)A[i * 256 + k] * (int)B[k * 256 + j];
            }
            C[i * 256 + j] = ((tmp&0xFF)+ ((tmp>>8)&0xFF))&0xFF;

            //printf("i %d j %d k %d C %02x tmp %04x\n", i, j, k, C[i * 256 + j], tmp);
        }
    }
}

uint32_t i, j , k;
uint32_t gSeqNO;

void
core_mineBytom(bmctx_t *bmctx, uint8_t fourSeq[4][32], int8_t *matrix_list, int8_t *idt_matrix, uint32_t *data)
{
    int8_t *A1, *B1, *A2, *B2;

    int8_t *mat, *tmp; //one 256 * 256
    int8_t *res; // four 256 * 256

    mat = (int8_t *)malloc(256 * 256);
    tmp = (int8_t *)malloc(256 * 256);
    res = (int8_t *)malloc(4 * 256 * 256);

    printf("core_mineBytom2 mat matrix_list, %p, mat %p, tmp %p, res %p\n", matrix_list, mat, tmp, res);

    //dump = (uint8_t *)GetMatrixAddr(matrix_list, 80);

    //printf("dump addr %p, 0 %02x %02x %02x %02x\n", dump, dump[0], dump[1], dump[2], dump[3]);

    //dumpArrays8((uint8_t *)(matrix_list, 80), 256, 256);

    process = 7;

    for (k = 0; k < 4; k++) {
        for (j = 0; j < 2; j++) {
            for (i = 0; i < 32; i += 2) {
                if ((i == 0) && (j == 0)) {
                    A1 = idt_matrix;
                    B1 = GetMatrixAddr(matrix_list, fourSeq[k][i]);
                    gSeqNO = fourSeq[k][i];
                    //printf("B1 addr 0 %p, 1 %p 80 %p\n", GetMatrixAddr(matrix_list, 0), GetMatrixAddr(matrix_list, 1), GetMatrixAddr(matrix_list, fourSeq[k][i]));
                }
                else {
                    A1 = tmp;
                    B1 = GetMatrixAddr(matrix_list, fourSeq[k][i]);
                    gSeqNO = fourSeq[k][i];
                }
                //1st multiple  mat = A1 * B1;

                printf("idt/tmp %p * matlist %p = mat %p i %d j %d k %d seq %d\n", A1, B1, mat, i, j, k, gSeqNO);

                printf("A1[00] %02x A1[01] %02x A1[02] %02x A1[10] %02x A1[11] %02x A1[12] %02x \n", A1[0], A1[1], A1[2], A1[256], A1[257], A1[258]);
                printf("B1[00] %02x B1[01] %02x A1[02] %02x B1[10] %02x B1[11] %02x B1[12] %02x \n", B1[0], B1[1], B1[2], B1[256], B1[257], B1[258]);

                tl_matrix_mac_bytom(bmctx, (uint8_t *)A1, (uint8_t *)B1, (uint8_t *)mat);

                printf("mat[00] %02x mat[01] %02x mat[02] %02x mat[10] %02x mat[11] %02x mat[12] %02x \n", mat[0], mat[1], mat[2], mat[256], mat[257], mat[258]);

                //matrix_mul_square256(A1, B1, mat);

                //printf("1st round i %d, j %d, k %d seq %d\n", i, j, k, fourSeq[k][i]);

                //dumpArrays8((uint8_t *)mat, 256, 256);

                A2 = mat;
                B2 = GetMatrixAddr(matrix_list, fourSeq[k][i + 1]);
                gSeqNO = fourSeq[k][i + 1];
                //2nd multiple  tmp = A2 * B2;

                printf("mat %p * matlist %p = tmp %p i %d j %d k %d seq %d\n", A2, B2, tmp, i, j, k, gSeqNO);

                printf("A2[00] %02x A2[01] %02x A2[02] %02x A2[10] %02x A2[11] %02x A2[12] %02x \n", A2[0], A2[1], A2[2], A2[256], A2[257], A2[258]);
                printf("B2[00] %02x B2[01] %02x B2[02] %02x B2[10] %02x B2[11] %02x B2[12] %02x \n", B2[0], B2[1], B2[2], B2[256], B2[257], B2[258]);

                tl_matrix_mac_bytom(bmctx, (uint8_t *)A2, (uint8_t *)B2, (uint8_t *)tmp);

                printf("tmp[00] %02x tmp[01] %02x tmp[02] %02x tmp[10] %02x tmp[11] %02x tmp[12] %02x \n", tmp[0], tmp[1], tmp[2], tmp[256], tmp[257], tmp[258]);

                //matrix_mul_square256(A2, B2, tmp);

                //printf("2nd round i %d, j %d, k %d seq %d\n", i, j, k, fourSeq[k][i]);

                //dumpArrays8((uint8_t *)tmp, 256, 256);
            }
        }

        memcpy(&res[k * 256 * 256], tmp, 256 * 256);
    }

    //printf("res matirx dump:\n");

   //dumpArrays8((uint8_t *)(&res[0]), 256, 256);
   //dumpArrays8((uint8_t *)(&res[1 * 256 * 256]), 256, 256);
   //dumpArrays8((uint8_t *)(&res[2 * 256 * 256]), 256, 256);
   //dumpArrays8((uint8_t *)(&res[3 * 256 * 256]), 256, 256);

    matrix_add_square256(&res[0], &res[1 * 256 * 256], mat);
    matrix_add_square256(mat, &res[2 * 256 * 256], tmp);
    matrix_add_square256(tmp, &res[3 * 256 * 256], mat);

    //printf("after add dump:\n");
    //dumpArrays8((uint8_t *)mat, 256, 256);

    matrix_resize(mat, data);

    //printf("data dump:\n");
    //for(int i = 0; i < 64; i++) {
    //    printf("%08x ", data[i]);
    //}
    //printf("\n");

    free(tmp);
    free(mat);
    free(res);
}

void iter_mineBytom(
    bmctx_t *bmctx,
    const uint8_t *fixedMessage,
    int8_t *matrixlist,
    int8_t *idet_matrix,
    uint32_t len,
    uint8_t result[32])
{
  uint8_t sequence[32];
  sha3_ctx ctx;

    uint8_t fourSeq[4][32];
    for(int k=0; k<4; k++) { // The k-loop
        rhash_sha3_256_init(&ctx);
        rhash_sha3_update(&ctx, fixedMessage+(len*k/4),len/4);//鍒嗗洓杞秷鑰楁帀fixedMessage
        rhash_sha3_final(&ctx, sequence);
	    for (int i=0; i<32; i++) {
            memcpy(&fourSeq[k][0], sequence, 32);
        }
    }
    uint32_t data[64];

    process = 6;

    core_mineBytom(bmctx, fourSeq, matrixlist, idet_matrix, data);

    rhash_sha3_256_init(&ctx);
    rhash_sha3_update(&ctx, (uint8_t*)data, 256);
    rhash_sha3_final(&ctx, result);
}

uint8_t *testmalloc = 0;

void
test_bytomPoW(bmctx_t *bmctx)
{
    //Words_32 seed;
    //uint32_t exted[32];
    uint8_t result[32];

    int32_t ms = 0;
    uint64_t tick_start, tick_done;

    printf("start test bytom\n");

    //testmalloc = (uint8_t *)xmalloc(16);
    //printf("testmalloc %p\n", testmalloc);

    //process = 16;

    //extend(exted, golden_seed[0]);

    //process = 1;

    //btm_seed_init(&seed, exted);

    //process = 2;

    //printf("dump seed of size %ld\n", sizeof(seed));
    //memdump((uint8_t *)&seed, sizeof(seed));

    allocte_squre_matrix_list(&maxtix_list, 256);

    process = 3;

    printf("matrix list is at %p\n", maxtix_list);

    allocte_squre_identify_matrix(&identify_matrix);

    process = 4;
    //dumpArrays8(identify_matrix, 256, 256);
    //init_matrix_list_with_seed(maxtix_list, &seed);

//    tl_res_allocte();

    process = 5;

    timer_meter_start();
    tick_start = timer_get_tick();

    iter_mineBytom(bmctx, golden_msg[0], maxtix_list, identify_matrix, 32, result);

    tick_done = timer_get_tick();
    ms = timer_meter_get_ms();
	  
	printf("Tick %lu(start %lu, done %lu), Test duration:%d ms\n",
                tick_done - tick_start, tick_start, tick_done, ms);

        //compare results
    for (int i = 0; i < 32; i++) {
        if (golden_result[0][i] != result[i]) {
            printf("compare failed at offset %d\n", i);

            printf("g_golden_results[%d] = %d\n", i, golden_result[0][i]);
            printf("results[%d] = %d\n", i, result[i]);

            compare_result = 0xff;
            return;
        }
    }

    compare_result = 0xfe;
    printf("compare pass\n");

}

int testcase_bytompow()
{
  bmctx_t ctx;
  test_init(&ctx);
  test_bytomPoW_batch(&ctx);
  test_exit(&ctx);
  return 0;
}
module_testcase("1", testcase_bytompow);
