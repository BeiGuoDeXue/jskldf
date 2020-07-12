/**
 * @file connect.h
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
#ifndef CONNECT_H
#define CONNECT_H
#include <semaphore.h>
#include <MQTTAsync.h>

#ifdef __cplusplus
extern "C" {
#endif
void onSubscribeFailure(void* context, MQTTAsync_failureData* response);
void onSubscribe(void* context, MQTTAsync_successData* response);
/*
 * MQTT 连接成功的回调函数
 * 将订阅 MQTT topic
 */
void onConnect(void* context, MQTTAsync_successData* response);
/*
 * MQTT 连接失败的回调函数
 */
void onConnectFailure(void* context, MQTTAsync_failureData* response);
/*
 * MQTT 连接断开的回调函数
 * 将对各个 topic 取消订阅
 */
void connlost(void *context, char *cause);
/*
 * MQTT 断开连接的回调函数
 */
void onDisconnect(void* context, MQTTAsync_successData* response);
void on_send(void* context, MQTTAsync_successData* response);
void on_send_delrec(void* context, MQTTAsync_successData* response);

#ifdef __cplusplus
}
#endif
#endif //CONNECT_H
