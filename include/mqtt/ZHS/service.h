/**
 * @file service.h
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

#ifndef SERVICE_H
#define SERVICE_H

#include "message.h"

enum ctr_stat
{
    HEATING = 0x01,
    BINDING_QUERY,
    SLEFCHECKING,
    PARAMING_QUERY,
    CONTROL_REPLY,
    UPGRADE_REPLY  
};

#ifdef __cplusplus
extern "C" {
#endif
#include "MQTTAsync.h"
void ServiceAck(char *data, char cmd, int tims);
void* MqttAsyncServiceMain(void *buf);
void MqttServic_stop(void);
void DevToServiceDataCreat(char *data, CTL_DEVICE dev);
int msgarrvd(void *context, char *topicName, int topicLen, MQTTAsync_message *message);
/*
 * 将创建 MqttAsyncServiceMain() MQTT 主线程
 * 而后通过 pthread_join()阻塞等待该线程结束，并尝试重新创建它
 */
int MqttServiceMain();
#ifdef __cplusplus
}
#endif

#endif //SERVICE_H