/*****************************************
*https://www.cnblogs.com/swey/p/4114897.html
*****************************************/
//ms级别定时器
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <sys/time.h>
#include <errno.h>
#include "bsp_timer.h"
#include "config.h"
#include "bsp_lte.h"
//使用select定时器，可以做到ms级别
int timer_init(void (*add_function)(int),unsigned int us_timer,unsigned int s_timer )
{
    if(signal(SIGALRM, add_function) == SIG_ERR){
    perror("signal\n");
    return -1;
    }
    struct itimerval tv;
    tv.it_value.tv_usec = us_timer;
    tv.it_value.tv_sec = s_timer;
    tv.it_interval.tv_usec = us_timer;
    tv.it_interval.tv_sec = s_timer;
    if(setitimer(ITIMER_REAL, &tv, NULL) != 0){
        perror("setitimer\n");
        return -1;
    }
    return 0;
}
//select延时,ms级别，非阻塞式
void milliseconds_sleep(unsigned long mSec)
{
    struct timeval tv;
    tv.tv_sec = mSec/1000;
    tv.tv_usec = (mSec%1000)*1000;
    int err;
    do{
       err=select(0,NULL,NULL,NULL,&tv);
    }while(err<0 && errno==EINTR);
}

//定时器中断处理函数
void timer_interupt(int signo)
{                           
    unsigned long timer = get_ms();   //获取时间
    read_lte_info(EC20_2);
}