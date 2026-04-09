#ifndef __hal_timer_H__
#define __hal_timer_H__
#define __weak __attribute__((weak))
typedef int (*timer_callback_t)(void *);

#define NS_PER_TICK		40

#define REG_TIMER_BASE	0x030A0000

//#define DEBUG

#define timer_log uartlog

#ifdef DEBUG
#define timer_dbg uartlog
#else
#define timer_dbg(...)
#endif

#ifdef CONFIG_C906B
#define TIMER_IRQ(n)	(79 + n)
#else // CONFIG_C906L
#define TIMER_IRQ(n)	(51 + n)
#endif

enum TIMER_ID{
	TIMER_ID0 = 0,
	TIMER_ID1,
	TIMER_ID2,
	TIMER_ID3,
	TIMER_ID4,
	TIMER_ID5,
	TIMER_ID6,
	TIMER_ID7,
	MAX_TIMER_ID,
};

struct timerN_regs{
	uint32_t TimerNLoadCount;
	uint32_t TimerNCurrentValue;
	uint32_t TimerNControlReg;
	uint32_t TimerNEOI;
	uint32_t TimerNIntStatus;
};

struct timer_reg{
	struct timerN_regs timern_regs[8];
	uint32_t TimersIntStatus;
	uint32_t TimersEOI;
	uint32_t TimersRawIntStatus;
	uint32_t TIMERS_COMP_VERSION;
	uint32_t TimerNLoadCount2[8];
};

struct hal_timer{
	uint8_t timer_id;
	struct timer_reg *root_regs;
	struct timerN_regs *timern_regs;
	timer_callback_t callback;
	void *arg;
	int irq;
	uint32_t load_value;
};

struct hal_timer * hal_timer_init(uint8_t timer_id);
void hal_timer_uninit(struct hal_timer *timer);
int hal_timer_start(struct hal_timer *hal_timer, uint32_t timeout_us);
void hal_timer_stop(struct hal_timer *hal_timer);
void hal_timer_detach_callback(struct hal_timer *timer);
void hal_timer_attach_callback(struct hal_timer *timer, timer_callback_t callback, void *arg);
uint32_t hal_timer_get_remaining_value(struct hal_timer *timer);
uint32_t hal_timer_get_load_value(struct hal_timer *timer);
uint32_t hal_timer_is_running(struct hal_timer *timer);

#endif /* __hal_timer_H__ */

