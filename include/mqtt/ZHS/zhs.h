/**
 * @file zhs.h
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

#ifndef ZHS_H
#define ZHS_H

#include <pthread.h>

/*
 * 是否需要从本地数据库上传设备数据到 MQTT
 * 当与 MQTT 处于断开状态时，则先把设备上传的数据保存在本地 sqlite3
 *
 * refr: 
 * onConnectFailure(), 设置为 1
 * start_native_upload()->native_data_system(), 为 1 时，从本地数据库上传设备数据到 MQTT
 * UartData_Analysis(), 为 1 时，将把设备数据保存在本地 sqlite3
 */

extern char upgrade_status;
extern pthread_mutex_t sqlite_mutex;
extern pthread_cond_t sqlite_convar;

/* 默认的设备 ID, 对应着 /proc/cpuinfo 里的 serial 信息 */
#define DEVICE_ID "432944698949007777"

/* MQTT 上报数据时间间隔 */
#define UPLOADINTERVAL                  60 * 1
//#define UPLOADINTERVAL                  60 * 5
//#define PAYLOAD                         "Hello World!"
#define QOS                             2
#define MAX_RETRY_TIME                  10 * 60 * 60
#define KEEPALIVEINTERVAL               60
/*
 * 使用客户端库的应用程序通常使用类似的结构：
 * 1.创建一个客户端对象；
 * 2.设置连接MQTT服务器的选项；
 * 3.设置回调函数；
 * 4.将客户端和服务器连接；
 * 5.订阅客户端需要接收的所有话题；
 * 6.重复以下操作直到结束：
 *   a.发布客户端需要的任意信息；
 *   b.处理所有接收到的信息；
 * 7.断开客户端连接；
 * 8.释放客户端使用的所有内存
 */
#define ADDRESS "tcp://182.150.115.28:2883"
#define MQTTACCOUNT "smartCar_controller"
#define MQTTACCOUNT_name "xiaozhong"
#define MQTTACCOUNT_pass "xiaozhong"

#if __WORDSIZE == 64
#define __SIZEOF_SEM_T 32
#else
#define __SIZEOF_SEM_T 16
#endif

#endif