#ifndef _BMKERNEL_H_
#define _BMKERNEL_H_

typedef struct {
    u64 gmem_size;
    u64 mem_ptr;
} bmctx_t;

typedef struct dim_s {
    int n, c, h, w;
} dim_t;

int testcase_main(void);
void test_bmkernel_submit(void);
void test_bmkernel_init(void);
void test_bmkernel_exit(void);

void bmkernel_stress_init(void);

#define BMKERNEL_NODE_ID_HOST  (0x0F)

#endif
