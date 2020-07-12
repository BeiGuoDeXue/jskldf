/**
 * @file ulility.cpp
 * @brief 
 * @author xiaozhong (xiaozhong@zhskg.cn)
 * @version 1.0
 * @date 2020-06-11
 * 
 * @copyright Copyright (c) 2020-2024  智慧式集团
 * 
 * @par 修改日志:
 * 2020-06-11 1.0     xiaozhong     
 */
#include <stdint.h>
#include <sys/time.h>
#include "ulitity.h"

uint32_t get_ssecond(void)
{
    time_t t;
    uint32_t stamp;
    
    t = time(NULL);
    stamp = time(&t);
    return stamp;
}

unsigned long get_ms(void)
{
    unsigned long stamp;
    struct  timeval  tm;
    gettimeofday(&tm,NULL); 
    stamp = (1000000 * tm.tv_sec + tm.tv_usec)/1000;
    return stamp;
}

static uint8_t char_to_hex(uint8_t bChar)
{
    if ((bChar >= 0x30) && (bChar <= 0x39))
        bChar -= 0x30;
    else if ((bChar >= 0x41) && (bChar <= 0x46)) //大写字母
        bChar -= 0x37;
    else if ((bChar >= 0x61) && (bChar <= 0x66)) //小写字母
        bChar -= 0x57;
    else
        bChar = 0xff;

    return bChar;
}