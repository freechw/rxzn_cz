#include "define.h"
#include "timer_b.h"
#include "task.h"
ulong  _local_time_tick;

#define INTERVAL_TIME 10 // 10ms
void sys_clock_tick(void){
	if(_local_time_tick > 0xFFFFFFF0)
		_local_time_tick = 0;
	_local_time_tick += INTERVAL_TIME;
}

ulong local_ticktime(void){
	return _local_time_tick	;
}

bool timeout(ulong last_time, ulong ms){
	if(_local_time_tick > last_time)
		return (bool)((_local_time_tick - last_time) > ms);
	else
		return (bool)((last_time - _local_time_tick) > ms);
}

#pragma vector = TIMERB0_VECTOR
__interrupt void Timer_B(void){	 
	sys_clock_tick();
}
