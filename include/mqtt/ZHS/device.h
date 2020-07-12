/**
 * @file carstat.h
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

#ifndef CARSTAT_H
#define CARSTAT_H

#include <string>
#include "MQTTAsync.h"

typedef enum 
{
    idle = 0,
    hearting,           // 心跳上报
    uploading,          // 数据上报
    alarming,           // 报警上报
    reset_alarming,     // 报警回复上报
    warn_alarming,      // 预警上报
    reset_warn,         // 报警回复上报
    par_set,            // 参数设置
    par_setacking,      // 参数设置回复
    faulting,           // 故障上报
    reset_faulting,     // 故障回复上报
    control_acking,     // 控制回复上报
    upgrade_acking,     // 升级回复上报
    bind_acking,        // 绑定回复上报
    native_uploading,   // 本地存储数据上报
    sys_paraming,       // 系统版本参数上报
    sys_selfchecking,   // 系统自检数据上报
} Subset_Status;

/*
 * 系统的状态
 */
typedef enum
{
    normal,       // 正常
    low_vol,      // 低电压
    high_temp,    // 温度过高
    mem_overflow, // 内存不足
    recover,
} SYS_STATE;


/*
 * 设备状态，用于设备自检 sys_selfchecking
 */
typedef struct 
{
    SYS_STATE stat;
    int temp;
    int voltage;
    int freemem;
    int usedmem;
} SYS_DEVICE_MSG, *PT_SYS_DEVICE_MSG;



#define CAR_UPLOAD_ACK               1
#define CAR_UPLOAD                   0
#define CAR_EVENT_ALARM_ACK          2
#define CAR_EVENT_ALARM              3
#define CAR_CONTROL_ACK              4
#define CAR_CONTROL                  5
#define CAR_UPGRADE                  6
#define CAR_UPGRADE_ACK              7

/*
 * 获取 cpuid
 */
std::string getDeviceId(void);
int get_machine_stat(PT_SYS_DEVICE_MSG msg);
#endif