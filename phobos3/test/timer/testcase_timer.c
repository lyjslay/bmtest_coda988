#include <stdio.h>
#include "boot_test.h"
#include "system_common.h"
#include "timer.h"
#include "hal_timer.h"
#include "mmio.h"

#define barrier() __asm__ __volatile__("": : :"memory")

uint8_t timer_alloc_status[8];
struct hal_timer *timers[7];
int status;

static inline uint64_t get_cycles64(void)
{
	uint64_t cyc;
	asm volatile("rdtime %0" : "=r" (cyc));
	return cyc;
}

static inline uint32_t hal_timer_get_intstatus(struct hal_timer *timer)
{
	return mmio_read_32((uintptr_t)&timer->timern_regs->TimerNIntStatus);
}

static inline void hal_timer_set_period_tick(struct timerN_regs *timern_regs, uint32_t ticks)
{
	mmio_write_32((uintptr_t)&timern_regs->TimerNLoadCount, ticks);
}

static inline void hal_timer_enable(struct hal_timer *timer)
{
	struct timerN_regs *timern_regs = timer->timern_regs;
	mmio_write_32((uintptr_t)&timern_regs->TimerNControlReg, 1);
}

static inline void hal_timer_disable(struct hal_timer *timer)
{
	struct timerN_regs *timern_regs = timer->timern_regs;
	mmio_write_32((uintptr_t)&timern_regs->TimerNControlReg, 0);
}

uint32_t hal_timer_get_remaining_value(struct hal_timer *timer)
{
	return mmio_read_32((uintptr_t)&timer->timern_regs->TimerNCurrentValue);
}

uint32_t hal_timer_get_load_value(struct hal_timer *timer)
{
	return timer->load_value;
}

uint32_t hal_timer_is_running(struct hal_timer *timer)
{
	return mmio_read_32((uintptr_t)&timer->timern_regs->TimerNControlReg) & 0x1;
}

static int hal_timer_irq_handler(int irq, void *arg)
{
	struct hal_timer *timer = arg;

	if (!(hal_timer_get_intstatus(timer) & 0x1)){
		//no interrupt generate
		return 0;
	}
	//turn off timer, so the interrupe is also turned off
	hal_timer_disable(timer);

	if (timer->callback)
		timer->callback(timer->arg);

	return 0;
}

void hal_timer_detach_callback(struct hal_timer *timer)
{
	if(!timer)
		return;
	timer->callback = NULL;
	timer->arg = NULL;
}

void hal_timer_attach_callback(struct hal_timer *timer, timer_callback_t callback, void *arg)
{
	timer_dbg("enter hal_timer_attach_callback\n");
	timer_dbg("timer=%p, callback=%p, arg=%p\n", timer, callback, arg);

	if(!timer){
		timer_log("%s, timer is NULL\n", __func__);
		return;
	}

	timer->callback = callback;
	timer->arg = arg;
}

static struct hal_timer *alloc_hal_timer(enum TIMER_ID timer_id)
{
	struct hal_timer *timer = malloc(sizeof(struct hal_timer));

	memset(timer, 0, sizeof(struct hal_timer));

	timer_dbg("enter alloc_hal_timer\n");

	if(!timer){
		timer_log("hal_timer alloc failed!\n");
		return timer;
	}

	timer_dbg("timer=%p\n", timer);

	timer->timer_id = timer_id;
	timer->irq = TIMER_IRQ(timer_id);
	timer->root_regs = (struct timer_reg *)REG_TIMER_BASE;
	timer->timern_regs = &timer->root_regs->timern_regs[timer_id];

	return timer;
}

struct hal_timer * hal_timer_init(uint8_t timer_id)
{
	struct hal_timer *timer;

	timer_log("enter hal_timer_init\n");
	if(timer_id >= MAX_TIMER_ID){
		timer_log("timer_id error\n");
		return NULL;
	}

	//taskENTER_CRITICAL();
	if(timer_alloc_status[timer_id]){
		//taskEXIT_CRITICAL();;
		return NULL;
	}

	mmio_write_32(0x03000000 + 0x1a0, 0x00);

	timer_alloc_status[timer_id] = 1;
	//taskEXIT_CRITICAL();

	timer = alloc_hal_timer(timer_id);

	hal_timer_disable(timer);

	//timer_log("request_irq %d\n", TIMER_IRQ(timer_id));
	request_irq(TIMER_IRQ(timer_id), hal_timer_irq_handler, 0, "timer_irq", timer);

	return timer;
}

void __weak free_irq(int irq)
{
	return;
}

void hal_timer_uninit(struct hal_timer *timer)
{
	if(!timer)
		return;
	free_irq(timer->irq);

	hal_timer_disable(timer);
	free(timer);

	//taskENTER_CRITICAL_FROM_ISR();
	timer_alloc_status[timer->timer_id] = 0;
	//taskEXIT_CRITICAL_FROM_ISR(0);
}

int hal_timer_start(struct hal_timer *hal_timer, uint32_t timeout_us)
{
	struct timerN_regs *timern_regs = hal_timer->timern_regs;
	uint64_t timeout_ns = timeout_us * 1000;
	uint32_t ticks = timeout_ns / NS_PER_TICK;

	timer_dbg("enter hal_timer_start\n");
	if(!hal_timer){
		return -1;
	}

	hal_timer->load_value = ticks;

	hal_timer_set_period_tick(timern_regs, hal_timer->load_value);
	hal_timer_enable(hal_timer);
	timer_dbg("hal_timer_start finish\n");
	return 0;
}

void hal_timer_stop(struct hal_timer *hal_timer)
{
	if(!hal_timer)
		return;

	hal_timer_disable(hal_timer);
}

static int timer_callback(void *arg)
{
	struct hal_timer *timer = arg;

	u32 tick_ms = timer_meter_get_ms();

	uartlog("timer delay 10s end, meter value=%d\n", tick_ms);

	timer_log("enter timer_callback, timer_id=%d, status = %d\n", timer->timer_id, status);
	timer_log("timer is running = %x\n", hal_timer_is_running(timer));
	status = 0;

	timer_log("exit timer_callback\n");
	return 0;
}

// test case for C906B
int testcase_timer_C906B(void)
{
	struct hal_timer *timer = NULL;
	int ret;

	timer_log("timer_dbg test\n");
	timer_meter_start();

	for(int i = 0; i < 8; i++){
		while(status) barrier();
		status = 1;
		if(timer)
			hal_timer_uninit(timer);
		timer = hal_timer_init(i);
		if(!timer){
			timer_log("timer_init failed\n");
		}

		timer_dbg("timer=%p\n", timer);

		hal_timer_attach_callback(timer, &timer_callback, timer);

		ret = hal_timer_start(timer, 2 * 1000 * 1000);
		u32 test_start = timer_meter_get_ms();
		uartlog("timer delay 10s end, meter value=%d\n", test_start);
		if(ret)
			continue;
		if(ret)
			continue;
		if(ret)
			continue;
		//u32 test_end = timer_meter_get_ms();

		//timer_log("timer load value=%x\n", hal_timer_get_load_value(timer));
		//timer_log("timer remaining value=%x\n", hal_timer_get_remaining_value(timer));
		//timer_log("timer is running = %x\n", hal_timer_is_running(timer));
	}

	hal_timer_stop(timer);

	hal_timer_uninit(timer);

	uartlog("timer test end, meter value=%d\n", timer_meter_get_ms());
	
	return 0;
}

// test case for C906L irq
int testcase_timer_irq_C906L(void)
{
	struct hal_timer *timer = NULL;
	int ret;

	timer_meter_start();
	
	timer_log("timer_dbg test\n");
	for(int i = 4; i < 8; i++){
		while(status) barrier();
		status = 1;
		if(timer)
			hal_timer_uninit(timer);

		timer = hal_timer_init(i);
		if(!timer){
			timer_log("timer_init failed\n");
		}

		timer_dbg("timer=%p\n", timer);

		hal_timer_attach_callback(timer, &timer_callback, timer);

		ret = hal_timer_start(timer, 2 * 1000 * 1000);
		//u32 test_start = timer_meter_get_ms();
		if(ret)
			continue;
		if(ret)
			continue;
		if(ret)
			continue;
		//u32 test_end = timer_meter_get_ms();

		//timer_log("timer load value=%x\n", hal_timer_get_load_value(timer));
		//timer_log("timer remaining value=%x\n", hal_timer_get_remaining_value(timer));
		//timer_log("timer is running = %x\n", hal_timer_is_running(timer));
	}

	long start_cyc = get_cycles64();
	while(1){
		if (start_cyc % 25000000 == 0) {
			uartlog("cpu is still active\n");
		}
		start_cyc = get_cycles64();
	}

	hal_timer_stop(timer);

	hal_timer_uninit(timer);
	
	return 0;
}

#ifndef BUILD_TEST_CASE_all
int testcase_main()
{
	int ret = testcase_timer_C906B();
	return ret;
}
#endif
