/**
 * @file zhsmqtt.h
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
#ifndef ZHSMQTT_H
#define ZHSMQTT_H
/*
 * 对 MQTT 相关信息进行初始化
 *
 * 包括: 
 * x. 初始化 MQTT 的 TOPIC
 * x. 初始化 clientid
 * x. 初始化 net_addr
 */
void MqttInit();
#endif //ZHSMQTT_H