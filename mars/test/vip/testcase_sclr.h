#ifndef _TESTCASE_SCLR_H_
#define _TESTCASE_SCLR_H_

#include "scaler.h"

int do_vip_fabric(int argc, char **argv);
void sc_from_isp(u8 inst, u8 online, struct sclr_size src, struct sclr_size dst, u64 addr0, u64 addr1, u64 addr2, bool osd_on);
void sclr_trig_start(int inst);
bool is_sclr_job_done(int inst);
void sclr_enable_intr_log(u32 enable);

int testcase_sclr(void);

#endif  // _TESTCASE_SCL
