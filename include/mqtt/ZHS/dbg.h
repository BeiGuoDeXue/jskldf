/**
 * @file zhsdbg.h
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
#ifndef ZHSDBG_H
#define ZHSDBG_H

    #ifndef __USEDEBGU
    #define __USEDEBGU 0
    #endif


    #if __USEDEBGU
    #define DBG(format, ...) printf (format, ##__VA_ARGS__)
    #define ERR(format, ...) printf (format, ##__VA_ARGS__)
    #else
    #define DBG(format, ...) {}
    #define ERR(format, ...) {}
    #endif

#endif

