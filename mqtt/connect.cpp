/**
 * @file connect.cpp
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
/**
    MQTT 的连接状态
**/

#include "connect.h"
#include "dbg.h"
#include "zhs.h"
#include "topic.h"
#include "zhssqlite.h"

extern char TOPIC[TOPICNUM][MAX_TOPIC_LEN];

/*
 * MQTT 的连接是否断开
 *
 * refr: 
 * connlost(), 连接断开
 */
int mqtt_connected = 0;
int disc_finish = 0;
int subscribed = 0;
extern int mqtt_connect_lost;

volatile MQTTAsync_token deliveredtoken;
char native_upload = 1;


void onSubscribeFailure(void* context, MQTTAsync_failureData* response)
{
    DBG("%s[%d]: Subscribe failed, rc %d\n", __FUNCTION__, __LINE__, response ? response->code : 0);
}

void onSubscribe(void* context, MQTTAsync_successData* response)
{
    subscribed++;
    // native_upload = 0;
    DBG("%s[%d]: Subscribe succeeded %d\n", __FUNCTION__, __LINE__, subscribed);
}

/*
 * MQTT 连接成功的回调函数
 * 将订阅 MQTT topic
 */
void onConnect(void* context, MQTTAsync_successData* response)
{
    MQTTAsync client = (MQTTAsync)context;
    MQTTAsync_responseOptions opts = MQTTAsync_responseOptions_initializer;
    int rc;
    int i = 0;

    DBG("%s[%d]: Successful connect MQTT service\n", __FUNCTION__, __LINE__);
    opts.onSuccess = onSubscribe;
    opts.onFailure = onSubscribeFailure;
    opts.context   = client;
    deliveredtoken = 0;

    /* 依次订阅 MQTT topic*/
    for (i = 0; i < TOPICNUM; i++) 
    {
        if ((rc = MQTTAsync_subscribe(client, TOPIC[i], QOS, &opts)) != MQTTASYNC_SUCCESS) 
            DBG("Failed to start subscribe topic=%s,\n return code %d\n", TOPIC[i], rc);
    }
}

/*
 * MQTT 连接失败的回调函数
 */
void onConnectFailure(void* context, MQTTAsync_failureData* response)
{
    native_upload = 1;
    DBG("%s[%d]: Connect failed, rc %d\n", __FUNCTION__, __LINE__, response ? response->code : 0);
}

/*
 * MQTT 连接断开的回调函数
 * 将对各个 topic 取消订阅
 */
void connlost(void *context, char *cause)
{
    MQTTAsync client = (MQTTAsync)context;
    MQTTAsync_responseOptions opts = MQTTAsync_responseOptions_initializer;
    int rc;
    int i = 0;
    
    DBG("%s[%d]: \nConnection lost\n", __FUNCTION__, __LINE__);
    DBG("[%d]: cause: %s\n", __LINE__, cause);
    DBG("[%d]: unsubscribe\n", __LINE__);
    
    mqtt_connect_lost = 1;

    /* 取消 MQTT topic 的订阅 */
    for (i = 0; i < TOPICNUM; i++) 
    {
        if ((rc = MQTTAsync_unsubscribe(client, TOPIC[i], &opts)) != MQTTASYNC_SUCCESS) 
            DBG("%s[%d]: Failed to start unsubscribe topic=%s,\n return code %d\n", __FUNCTION__, __LINE__, TOPIC[i], rc);
    }
}

/*
 * MQTT 断开连接的回调函数
 */
void onDisconnect(void* context, MQTTAsync_successData* response)
{
    DBG("%s[%d]: Successful disconnection\n", __FUNCTION__, __LINE__);
    disc_finish = 1;
}

void on_send(void* context, MQTTAsync_successData* response)
{
    // native_upload = 0;
    DBG("%s[%d]: onSend_Message with token value %d delivery confirmed\n", __FUNCTION__, __LINE__, response->token);
}


void on_send_delrec(void* context, MQTTAsync_successData* response)
{
    struct db_rec *db_rec = (struct db_rec*)context;
    
    DBG("%s[%d]: onSend_Message with token value %d delivery confirmed, then del record[%d]\n", 
        __FUNCTION__, __LINE__, response->token, db_rec->id);

    /* 从数据库中删除这条记录 */
//    sqlite_data_delete(NULL, db_rec->db_name, sqlite_delete_firstid);
}