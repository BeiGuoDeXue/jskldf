#ifndef __BSP_TIMER_H__
#define __BSP_TIMER_H__



int timer_init(void (*add_function)(int),unsigned int us_timer,unsigned int s_timer);
void milliseconds_sleep(unsigned long mSec);
void timer_interupt(int signo);



#endif 