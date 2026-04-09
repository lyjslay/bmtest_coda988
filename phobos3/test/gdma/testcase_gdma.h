#ifndef _TESTCASE_GDMA_H_
#define _TESTCASE_GDMA_H_

void gdma_g2l(u32 la, u64 ga, int N, int C, int H, int W);
void gdma_l2g(u64 ga, u32 la, int N, int C, int H, int W);
void gdma_g2l2g(u64 ga_out, u32 la, u64 ga_in, int N, int C, int H, int W);
void gdma_g2g(u64 ga_out, u64 ga_in, int N, int C, int H, int W);
void gdma_gfill(u64 ga_out, u32 val, int N, int C, int H, int W);
void gdma_l2l(u32 la_out, u32 la, int N, int C, int H, int W);
void gdma_lfill(u32 la, float val, int N, int C, int H, int W);
void gdma_l2l_mdscalar(u64 la_out, u64 la, int N, int C, int H, int W);
void gdma_lfill_mdscalar(u32 la, float val, int N, int C, int H, int W);

int testcase_gdma(void);

#endif
