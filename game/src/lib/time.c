#include "common.h"

uint32_t time_tick=0;

void timer_event(void){
	time_tick++;
}
