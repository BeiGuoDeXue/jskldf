/**
 * @file ulitity.h
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
#ifndef ULITITY_H
#define ULITITY_H
#include <time.h>
/*
 * 获取系统时间
 */
static uint8_t char_to_hex(uint8_t bChar);
uint32_t get_ssecond(void);
unsigned long get_ms(void);
#endif //ULITITY_H