#ifndef __CE_TYPES_H__
#define __CE_TYPES_H__

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <cache.h>

typedef uint8_t     ce_u8_t;
typedef int8_t      ce_i8_t;
typedef uint32_t    ce_u32_t;
typedef int32_t     ce_i32_t;
typedef uint64_t    ce_u64_t;
typedef int64_t     ce_i64_t;

#define ce_cont(fmt, args...)	printf(fmt, ##args)
#define ce_err		ce_cont
#define ce_warn		ce_cont
#define ce_info		ce_cont
#define ce_dbg		ce_cont
#define enter()	ce_info("enter %s\n", __func__)
#define line()	ce_info("%s %d\n", __FILE__, __LINE__)

#define ce_assert(cond)	\
	do {								\
		if (!(cond)) {						\
			ce_err("assert condition [%s] failed\n", #cond);\
			while (1)					\
				;					\
		}							\
	} while (0)

static inline ce_u32_t __ce_readl(const void *addr)
{
	return *(volatile ce_u32_t *)addr;
}

static inline void __ce_writel(const void *addr, ce_u32_t value)
{
	*(volatile ce_u32_t *)addr = value;
}
static inline void __ce_setbit(const void *addr, int bit)
{
	ce_u32_t tmp = __ce_readl(addr);
	tmp |= (1 << bit);
	__ce_writel(addr, tmp);
}
static inline void __ce_clrbit(const void *addr, int bit)
{
	ce_u32_t tmp = __ce_readl(addr);
	tmp &= ~(1 << bit);
	__ce_writel(addr, tmp);
}
static inline int __ce_getbit(const void *addr, int bit)
{
	return (__ce_readl(addr) >> bit) & 1;
}

#define ce_readl(reg)		__ce_readl(&(reg))
#define ce_writel(reg, value)	__ce_writel(&(reg), value)
#define ce_setbit(reg, bit)	__ce_setbit(&(reg), bit)
#define ce_clrbit(reg, bit)	__ce_clrbit(&(reg), bit)
#define ce_getbit(reg, bit)	__ce_getbit(&(reg), bit)

static inline void dumpp(const void *p, const unsigned long l)
{
	unsigned long i;

	if (p == NULL)
		ce_cont("null");
	else
		for (i = 0; i < l; ++i)
			ce_cont("%02x ", ((unsigned char *)p)[i]);
	ce_cont("\n");
}

static inline void dump(const void *p, const unsigned long l)
{
	unsigned long i;

	if (p == NULL)
		ce_cont("null");
	else
		for (i = 0; i < l; ++i) {
			if (i % 16 == 0)
				ce_cont("\n %04lx: ", i);
			ce_cont("%02x ", ((unsigned char *)p)[i]);
		}
	ce_cont("\n");
}

static inline void regdump(const void *p, const unsigned long l)
{
	unsigned long i;

	if (p == NULL)
		ce_cont("null");
	else
		for (i = 0; i < l; ++i) {
			if (i % 4 == 0)
				ce_cont("\n %08lx: ", i);
			ce_cont("%08x ", __ce_readl(((ce_u32_t *)p) + i));
		}
	ce_cont("\n");
}

#endif
