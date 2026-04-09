#include <stdio.h>
#include "cvi_mcu51.h"
#include "chip_cv1822.h"
#include "driver/dw_timer.h"

static xdata uint32_t timer_loaded_cnt;

void dw_timer_udelay(uint32_t timer_id, uint32_t n)
{
	timer_loaded_cnt = DW_TIMER_NS_PER_TICK * n;
	write_robot(REG_TIMER1_CONTROL + timer_id * 0x14, 0x0);
	write_robot(REG_TIMER1_LOADCNT + timer_id * 0x14, timer_loaded_cnt);
	write_robot(REG_TIMER1_CONTROL + timer_id * 0x14, 0x1);

	while(!read_robot(REG_TIMER1_INTSTATUS))
		;
}

void dw_timer_start(uint32_t timer_id, uint32_t cnt)
{
	timer_loaded_cnt = cnt;
	// printf("timer_loaded_cnt=%lu\n", timer_loaded_cnt);

	// disable timer
	write_robot(REG_TIMER1_CONTROL + timer_id * 0x14, 0x0);

	// set timer load count
	write_robot(REG_TIMER1_LOADCNT + timer_id * 0x14, timer_loaded_cnt);

	// enable timer
	write_robot(REG_TIMER1_CONTROL + timer_id * 0x14, 0x1);
}

uint32_t dw_timer_stop(uint32_t timer_id)
{
	xdata uint32_t time_end;
	xdata uint32_t timer_interval_ms;

	time_end = read_robot(REG_TIMER1_CURRENT_VALUE + timer_id * 0x14);

	// disable timer
	write_robot(REG_TIMER1_CONTROL + timer_id * 0x14, 0x0);

	timer_interval_ms = (timer_loaded_cnt - time_end) * DW_TIMER_NS_PER_TICK / 1000000;

	return timer_interval_ms;
}
