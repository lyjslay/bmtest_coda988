#ifndef __VI_COMMON_H__
#define __VI_COMMON_H__

#ifdef __cplusplus
	extern "C" {
#endif

#include "bmtest_type.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

extern int vi_dump_reg;

typedef uint64_t spinlock_t;

#define spin_lock_irqsave(lock, flags)  ((void)(flags))
#define spin_unlock_irqrestore(lock, flags) ((void)(flags))

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))
#endif

#define MIN(a, b) (((a) < (b))?(a):(b))
#define MAX(a, b) (((a) > (b))?(a):(b))
#define VI_64_ALIGN(x) (((x) + 0x3F) & ~0x3F)   // for 64byte alignment
#define VI_256_ALIGN(x) (((x) + 0xFF) & ~0xFF)   // for 256byte alignment
#define VI_ALIGN(x) (((x) + 0xF) & ~0xF)   // for 16byte alignment
#define VI_256_ALIGN(x) (((x) + 0xFF) & ~0xFF)   // for 256byte alignment
#define ISP_ALIGN(x, y) (((x) + (y - 1)) & ~(y - 1))   // for any bytes alignment
#define UPPER(x, y) (((x) + ((1 << (y)) - 1)) >> (y))   // for alignment
#define CEIL(x, y) (((x) + ((1 << (y)))) >> (y))   // for alignment
#define VI_4K_ALIGN(x) (((x) + 0xFFF) & ~0xFFF)   // for 4kbyte alignment

extern u32 vi_log_lv;

enum vi_msg_pri {
	VI_ERR		= 0x1,
	VI_WARN		= 0x2,
	VI_NOTICE	= 0x4,
	VI_INFO		= 0x8,
	VI_DBG		= 0x10,
	VI_PERF     = 0xA
};

#ifndef TEST_PERF
#define vi_pr(level, fmt, arg...) \
	do { \
		printf("%s:%d(): " fmt,  __func__, __LINE__, ##arg); \
	} while (0)
#else
#define vi_pr(level, fmt, arg...) \
	do { \
		if (level == VI_PERF || level == VI_ERR) \
			printf("%s:%d(): " fmt,  __func__, __LINE__, ##arg); \
	} while (0)
#endif

struct vi_rect {
	u16 x;
	u16 y;
	u16 w;
	u16 h;
};

#ifdef __cplusplus
}
#endif

#endif /* __VI_COMMON_H__ */
