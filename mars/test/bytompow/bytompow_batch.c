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
scp /home/johnny/Workspace/bm1880/u-boot/examples/bmtest/bm1880/out/bm1880_bytompow.elf bm1880@192.168.140.20:/home/bm1880/work/li.jiang01
make TEST_CASE=bytompow BOARD=PALLADIUM RUN_ENV=DDR
*/

/*
Attaions:
1. Makefile should change lib and compiler
2. bmkernel cmd_buf = (void *)0x1c2000000, and remove free in emit
*/

//#define BTM_DEBUG_LOG

#define FW_INIT_SEED        0
#define STREAM_MODE         1

#define MAX_JOB_NO          1
#define MAX_WORKER_NO       1 //cant no be larger than 2
#define BANKS_PER_WORKER    4
#define LINES_PER_WORKER    4

#define TMP_BANK_OFFSET 0
#define MAT_BANK_OFFSET 1
#define MATRIXLIST_BANK_OFFSET 2

int banks = 8;
int bank_size = 8192;
int matrix_size = 2048;
int matrice_per_bank = 4;

typedef struct {
    uint8_t msg[32];
    //uint8_t nonce[MAX_WORKER_NO][32];
    uint8_t result[32];
}bytom_job_t;

//each worker ocuupied 3 banks, tmp, mat and matrixlist
typedef struct _bytom_worker_t {
    //each worker has its own tmp, mat, matrixlist space in lmem;
    uint32_t start_bank;

    tensor_lmem *tmp[LINES_PER_WORKER];
    tensor_lmem *mat[LINES_PER_WORKER];
    tensor_lmem *matrixlist[LINES_PER_WORKER];

    gaddr_t *res_gaddr[LINES_PER_WORKER];

    uint8_t fourSeq[LINES_PER_WORKER][32];

    uint32_t data[64];

    bytom_job_t *cur_job;
}bytom_worker_t;

typedef struct _bytom_geometry_t {
    int8_t *identify_matrix;

    //below is used to store seed.
    gaddr_t *gaddr;

    uint8_t workerNo;

    bytom_worker_t worker[MAX_WORKER_NO];
}bytom_geometry_t;

bytom_geometry_t gBytomGeo;

bytom_job_t jobs[MAX_JOB_NO];

void initialize_jobs(bytom_job_t *joblist, uint32_t nrJob)
{
    bytom_job_t *job;

    printf("initialise_jobs %p sz %d\n", joblist, nrJob);

    for (int i = 0; i < nrJob; i++) {
        job = &joblist[i];

        memset(job->result, 0, sizeof(job->result));

        //for (int j = 0; j < BATCH_COUNT_EACH_THREAD; j++) {
        //    printf("%d task %p batch %d addr %p\n", i, task, j, task->calc_results[j]);
        //}

        memcpy(job->msg, &golden_msg[0][0], 32);
        //job->nonces = &g_nonces[0];
        //job->seed = &g_seed[0];
    }
}

int resultAnayze(bytom_job_t *job, int8_t jobId)
{
    int i, ret = 0;

    for (i = 0; i < 32; i++) {
        if (golden_result[0][i] != job->result[i]) {
            printf("compare error at jobId %d inner offset %d [%x %x]\n", jobId, i, golden_result[0][i], job->result[i]);
            ret = -1;
            break;
        }
    }

    if (ret != 0) {
        printf("dump golden_results :");
        for (int i = 0; i < 32; i++) {
            printf("0x%02x, ", golden_result[0][i]);
        }
        printf("\n");

        printf("dump rotten results :");
        for (int i = 0; i < 32; i++) {
            printf("0x%02x, ", job->result[i]);
        }
        printf("\n");
    }

    return ret;
}

gaddr_t* GetMatrixlistGaddr(gaddr_t *gaddr, uint32_t idx)
{
    int8_t *matrix = (int8_t *)gaddr;
    assert(idx < 256);
    return (gaddr_t *)(&matrix[idx * 256 * 256]);
}

static uint32_t addr_of_bank_loc(int bank_i, int in_bank_loc)
{
  assert(in_bank_loc < matrice_per_bank);
  return bank_i * bank_size + in_bank_loc * matrix_size;
}

static tensor_lmem * alloc_left_tensor(int bank_i, int in_bank_loc)
{
  shape_t left_shape = shape_t4(1, 256, 1, 256);
  uint32_t addr = addr_of_bank_loc(bank_i, in_bank_loc);

  return tl_prealloc_align(addr, left_shape, DATA_FMT_I8);
}

static tensor_lmem * alloc_right_tensor(int bank_i, int in_bank_loc)
{
  assert(in_bank_loc + 1 < matrice_per_bank);
  shape_t right_shape = shape_t4(256, 16, 1, 16);
  uint32_t addr = addr_of_bank_loc(bank_i, in_bank_loc);

  return tl_prealloc_align(addr, right_shape, DATA_FMT_I8);
}

void
initWorkers(void)
{
    bytom_worker_t *worker;
    bytom_geometry_t *pBytomGen = &gBytomGeo;
    uint32_t startbank = 0;

    for (int i = 0; i < MAX_WORKER_NO; i++) {
        worker = &pBytomGen->worker[i];
        worker->start_bank = startbank;
        startbank += BANKS_PER_WORKER;

        printf("init worker %d %p, with start bank at %d\n", i, worker, worker->start_bank);

        for (int j = 0; j < LINES_PER_WORKER; j++) {
            uint32_t right_bank_no, right_bank_offset;

            //each left matrix require 2k space in bank
            worker->tmp[j] = alloc_left_tensor(worker->start_bank + TMP_BANK_OFFSET, j);
            worker->mat[j] = alloc_left_tensor(worker->start_bank + MAT_BANK_OFFSET, j);

            //each left matrix require 4k space in bank
            right_bank_no = worker->start_bank + MATRIXLIST_BANK_OFFSET + j / 2;
            right_bank_offset = (j % 2) * 2;
            
            worker->matrixlist[j] = alloc_right_tensor(right_bank_no, right_bank_offset);

            worker->res_gaddr[j] = (gaddr_t *)xmalloc(256 * 256);

            printf("worker %d line %d tmp %p at bank %d offset %d\n", i, j, worker->tmp[j], worker->start_bank + TMP_BANK_OFFSET, j);
            printf("worker %d line %d mat %p at bank %d offset %d\n", i, j, worker->mat[j], worker->start_bank + MAT_BANK_OFFSET, j);
            printf("worker %d line %d lst %p at bank %d offset %d\n", i, j, worker->matrixlist[j], right_bank_no, right_bank_offset);
        }
    }
}

void
initGeometry(void)
{
    printf("start init geometry\n");
    gBytomGeo.gaddr = (gaddr_t *)0x1c1000000;
    gBytomGeo.workerNo = MAX_WORKER_NO;

    initWorkers();
}

uint64_t kernel_tick_start, kernel_tick_end;

void
core_mineBytomBatch(bmctx_t *bmctx)
{
    uint32_t i, j , k;
    gaddr_t *right_gaddr, *left_gaddr;
    bytom_worker_t *worker;
    bytom_geometry_t *geo = &gBytomGeo;
#ifdef BTM_DEBUG_LOG
    uint64_t tick_start, tick_done;
#endif

    int8_t *tmp = (int8_t *)xmalloc(256 * 256);

#ifdef BTM_DEBUG_LOG
    printf("%lu:core_mineBytomBatch\n", timer_get_tick());
#endif
    //dump = (uint8_t *)GetMatrixAddr(matrix_list, 80);

    //printf("dump addr %p, 0 %02x %02x %02x %02x\n", dump, dump[0], dump[1], dump[2], dump[3]);

    //dumpArrays8((uint8_t *)(matrix_list, 80), 256, 256);

#if (STREAM_MODE == 1)
    kernel_create_streams(LINES_PER_WORKER);
#endif

    //for (int w = 0; w < geo->workerNo; w++) {
    //    worker = &geo->worker[w];
    for (j = 0; j < 2; j++) {
        for (i = 0; i < 32; i += 2) {
            for (k = 0; k < LINES_PER_WORKER; k++) {
#if (STREAM_MODE == 1)
                kernel_set_stream(k);
#endif
                for (int w = 0; w < geo->workerNo; w++) {
                    worker = &geo->worker[w];

                    if ((i == 0) && (j == 0)) {
                        left_gaddr = (gaddr_t *)geo->identify_matrix;

#ifdef BTM_DEBUG_LOG
                        printf("%lu:load I from %p into lmem \n", timer_get_tick(), left_gaddr);
                        dumpArrays8((uint8_t *)left_gaddr, 4, 16);
#endif
                        tl_load_fixed(worker->tmp[k], (gaddr_t)left_gaddr, CTRL_NULL);
                    }

                    right_gaddr = GetMatrixlistGaddr(gBytomGeo.gaddr, worker->fourSeq[k][i]);
#ifdef BTM_DEBUG_LOG
                    printf("%lu:mat = tmp * list, right array addr %p seq %d, i %d j %d k %d\n", timer_get_tick(), right_gaddr, worker->fourSeq[k][i], i, j, k);
#endif
                    tl_load_fixed(worker->matrixlist[k], (gaddr_t)right_gaddr, CTRL_NULL);
                    tl_matrix_mac_2_fixed(worker->mat[k], worker->tmp[k], worker->matrixlist[k]);

                    right_gaddr = GetMatrixlistGaddr(gBytomGeo.gaddr, worker->fourSeq[k][i + 1]);
#ifdef BTM_DEBUG_LOG
                    printf("%lu:tmp = mat * list, right array addr %p seq %d, i %d j %d k %d\n", timer_get_tick(), right_gaddr, worker->fourSeq[k][i + 1], i, j, k);
#endif
                    tl_load_fixed(worker->matrixlist[k], (gaddr_t)right_gaddr, CTRL_NULL);
                    tl_matrix_mac_2_fixed(worker->tmp[k], worker->mat[k], worker->matrixlist[k]);
                }
            }
        }
    }
    //}

    for (int w = 0; w < geo->workerNo; w++) {
        worker = &geo->worker[w];

        for (k = 0; k < LINES_PER_WORKER; k++) {
            tl_store_fixed(worker->tmp[k], (gaddr_t)worker->res_gaddr[k], CTRL_NULL);
        }
    }

#ifdef BTM_DEBUG_LOG
    printf("%lu:kernel_submit:\n", timer_get_tick());
    tick_start = timer_get_tick();
#endif

    kernel_tick_start = timer_get_tick();

    kernel_submit();

    kernel_tick_end = timer_get_tick();

#ifdef BTM_DEBUG_LOG
    tick_done = timer_get_tick();

	printf("kernel_submit cost Tick %lu(start %lu, done %lu)\n", tick_done - tick_start, tick_start, tick_done);
#endif

   //dumpArrays8((uint8_t *)(&res[0]), 256, 256);
   //dumpArrays8((uint8_t *)(&res[1 * 256 * 256]), 256, 256);
   //dumpArrays8((uint8_t *)(&res[2 * 256 * 256]), 256, 256);
   //dumpArrays8((uint8_t *)(&res[3 * 256 * 256]), 256, 256);
    //printf("after add dump:\n");
    //dumpArrays8((uint8_t *)mat, 256, 256);

    for (int w = 0; w < geo->workerNo; w++) {
        worker = &geo->worker[w];

#if 1
        printf("get worker %d %p result\n", w, worker);

        for (int k = 0; k < LINES_PER_WORKER; k++) {
            printf("line %d result dump\n", k);
            dumpArrays8((uint8_t *)(worker->res_gaddr[k]), 4, 16);
        }
#endif

#ifdef BTM_DEBUG_LOG
        printf("res[0] + res[1] -> tmp\n");
#endif
        matrix_add_square256((int8_t *)worker->res_gaddr[0], (int8_t *)worker->res_gaddr[1], tmp);

#ifdef BTM_DEBUG_LOG
        printf("tmp + res[2] -> res[0]\n");
#endif
        matrix_add_square256(tmp, (int8_t *)worker->res_gaddr[2], (int8_t *)worker->res_gaddr[0]);

#ifdef BTM_DEBUG_LOG
        printf("res[0] + res[3] -> tmp\n");
#endif
        matrix_add_square256((int8_t *)worker->res_gaddr[0], (int8_t *)worker->res_gaddr[3], tmp);

#ifdef BTM_DEBUG_LOG
        printf("matrix_resize\n");
#endif
        matrix_resize(tmp, &worker->data[0]);
    }

    free(tmp);
}

void
core_mineBytomBatchtest(bmctx_t *bmctx)
{
    uint32_t k = 0;
    gaddr_t  *left_gaddr;
    bytom_worker_t *worker;
    bytom_geometry_t *geo = &gBytomGeo;
#ifdef BTM_DEBUG_LOG
    uint64_t tick_start, tick_done;
#endif

    printf("batch test\n");

    int8_t *right_matrix2 = (int8_t *)xmalloc(256 * 256);
    int8_t *right_matrix3 = (int8_t *)xmalloc(256 * 256);

    MatrixToIdentifyMul2(right_matrix2);
    MatrixToIdentifyMul3(right_matrix3);

#ifdef BTM_DEBUG_LOG
    printf("%lu:core_mineBytomBatch\n", timer_get_tick());
#endif
    //dump = (uint8_t *)GetMatrixAddr(matrix_list, 80);

    //printf("dump addr %p, 0 %02x %02x %02x %02x\n", dump, dump[0], dump[1], dump[2], dump[3]);

    //dumpArrays8((uint8_t *)(matrix_list, 80), 256, 256);

#if (STREAM_MODE == 1)
    kernel_create_streams(LINES_PER_WORKER);
#endif

    worker = &geo->worker[0];

    left_gaddr = (gaddr_t *)geo->identify_matrix;
    //kernel_parallel_disable();
    tl_load_fixed(worker->mat[k], (gaddr_t)right_matrix3, CTRL_NULL);
    tl_load_fixed(worker->tmp[k], (gaddr_t)left_gaddr, CTRL_NULL);
    tl_load_fixed(worker->matrixlist[k], (gaddr_t)right_matrix2, CTRL_NULL);
    kernel_submit();
    tl_matrix_mac_2_fixed(worker->mat[k], worker->tmp[k], worker->matrixlist[k]);
    //tl_mul_fixed(NULL, worker->mat[k], worker->tmp[k], worker->matrixlist[k], 0);

    //tl_load_fixed(worker->matrixlist[k], (gaddr_t)right_matrix3, CTRL_NULL);
    //tl_matrix_mac_2_fixed(worker->tmp[k], worker->mat[k], worker->matrixlist[k]);
    
    tl_store_fixed(worker->mat[k], (gaddr_t)worker->res_gaddr[k], CTRL_NULL);
    
    kernel_submit();

    dumpArrays8((uint8_t *)(worker->res_gaddr[k]), 4, 16);

    MatrixCheckIdentity((int8_t *)worker->res_gaddr[k], 2);

    printf("reload again\n");
    tl_store_fixed(worker->mat[k], (gaddr_t)worker->res_gaddr[k], CTRL_NULL);
    kernel_submit();
    dumpArrays8((uint8_t *)(worker->res_gaddr[k]), 4, 16);
    MatrixCheckIdentity((int8_t *)worker->res_gaddr[k], 2);

    printf("test dead loop\n");
    while(1);
}

void iter_mineBytomBatch(bmctx_t *bmctx)
{
    uint8_t sequence[32];
    sha3_ctx ctx;

    bytom_worker_t *worker;
    bytom_geometry_t *geo = &gBytomGeo;

    for (int w = 0; w < geo->workerNo; w++) {
        worker = &geo->worker[w];
        for(int k=0; k < LINES_PER_WORKER; k++) {
            rhash_sha3_256_init(&ctx);
            rhash_sha3_update(&ctx, worker->cur_job->msg + (8*k), 8);
            rhash_sha3_final(&ctx, sequence);

            memcpy(&worker->fourSeq[k][0], sequence, 32);
#ifdef BTM_DEBUG_LOG
            printf("worker %d line %d fourseq dump\n", w, k);
            memdump((uint8_t *)&worker->fourSeq[k][0], 32);
#endif
        }
    }

    core_mineBytomBatch(bmctx);

    for (int w = 0; w < geo->workerNo; w++) {
        worker = &geo->worker[w];
        rhash_sha3_256_init(&ctx);
        rhash_sha3_update(&ctx, (uint8_t*)worker->data, 256);
        rhash_sha3_final(&ctx, worker->cur_job->result);
    }
}

void
workerFeedOnJob(bytom_job_t *job, uint32_t workerId)
{
    bytom_worker_t *worker;
    bytom_geometry_t *geo = &gBytomGeo;

    worker = &geo->worker[workerId];

    worker->cur_job = job;

#ifdef BTM_DEBUG_LOG
    printf("worker %d %p bind with job %p\n", workerId, worker, job);
#endif
}

//how to test,
//1. load seed inited matrix(256 * 256 * 256) into ddr starting at 0x1c1000000
//2. load firmware and run.

extern uint64_t total_tick, count_number;

void
test_bytomPoW_batch(bmctx_t *bmctx)
{
    //Words_32 seed;
    //uint32_t exted[32];
    int32_t fail = 0;
    uint64_t tick_start, tick_done;

    printf("start test bytom\n");

    initGeometry();

    initialize_jobs(jobs, MAX_JOB_NO);

#if (FW_INIT_SEED == 1)
    printf("init seed\n");
    extend(exted, golden_seed[0]);
    btm_seed_init(&seed, exted);
    printf("dump seed of size %ld\n", sizeof(seed));
    memdump((uint8_t *)&seed, sizeof(seed));
    init_matrix_list_with_seed((int8_t *)gBytomGeo.gaddr, &seed);
#endif

    printf("matrix list is at %p, dump some items\n", gBytomGeo.gaddr);
    dumpArrays8((uint8_t *)gBytomGeo.gaddr, 4, 16);

    allocte_squre_identify_matrix(&gBytomGeo.identify_matrix);
    dumpArrays8((uint8_t *)gBytomGeo.identify_matrix, 4, 16);

    tick_start = timer_get_tick();

    for (int i = 0; i < MAX_JOB_NO; i += MAX_WORKER_NO) {
        for (int w = 0; w < MAX_WORKER_NO; w++) {
            workerFeedOnJob(&jobs[i + w], w);
        }
        iter_mineBytomBatch(bmctx);
        //core_mineBytomBatchtest(bmctx);
    }

    tick_done = timer_get_tick();
	  
    printf("Total Kernel cost time %lums, total tick %lu(start %lu, done %lu)\n",
                ((kernel_tick_end - kernel_tick_start) * 20) / 1000, (kernel_tick_end - kernel_tick_start), kernel_tick_start, kernel_tick_end);

    printf("Total cost time %lums, total tick %lu(start %lu, done %lu)\n",
            ((tick_done - tick_start) * 20) / 1000, (tick_done - tick_start), tick_start, tick_done);

    for (int i = 0; i < MAX_JOB_NO; i++) {
        if (resultAnayze(&jobs[i], i) != 0) {
            fail = 1;
            break;
        }
    }

    if (fail == 0) {
        printf("compare pass\n");
    }

    printf("dead loop\n");
    while(1);
}
