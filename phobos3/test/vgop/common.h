#ifndef _COMMON_VIP_H_
#define _COMMON_VIP_H_

#define BASE_OFFSET 0x100000000
#define GOP_FRAME_OFFSET 0x200000

#define MIN(a, b) (((a) < (b))?(a):(b))
#define MAX(a, b) (((a) > (b))?(a):(b))
#define CLIP(x, min, max) MAX(MIN(x, max), min)
#define DISP_ALIGN(x) (((x) + 0x3F) & ~0x3F)   // for 64byte alignment
#define UPPER(x,y) (((x) + ((1 << (y)) - 1)) >> (y))   // for alignment
#define ALIGN(x,a) (((x) + ((a) - 1)) & ~((a) - 1))

#if defined(ENV_CVITEST) || defined(ENV_EMU)
#define ARRAY_SIZE(x)	(sizeof(x) / sizeof((x)[0]))
#define BIT(nr)         (UINT32_C(1) << (nr))
#endif

#define R_IDX 0
#define G_IDX 1
#define B_IDX 2

/********************************************************************
 *   APIs to replace bmtest's standard APIs
 ********************************************************************/
#if defined(ENV_CVITEST) || defined(ENV_EMU)
extern int cli_readline(char *prompt, char *cmdbuf);
#define uart_getc bm_getc
#undef uartlog
#define uartlog bm_uartlog
#define bm_get_num(str, var) \
	{ \
		uartlog(str); \
		scanf("%d", &tmp); \
		var = tmp; \
	}

void bm_scanf(const char *fmt, ...);
int bm_getc(void);
void bm_uartlog(const char *fmt, ...);
#endif

#endif
