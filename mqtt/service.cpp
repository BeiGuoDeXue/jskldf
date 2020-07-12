/**
 * @file service.cpp
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
#include <stdio.h>
#include <string>
#include <cstring>
#include <Thread.h>

#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"
#include "rapidjson/document.h"
#include "rapidjson/encodings.h"

#include "service.h"
#include "dbg.h"
#include "ulitity.h"
#include "MQTTAsync.h"
#include "connect.h"
#include "zhs.h"
#include "message.h"
#include "device.h"
#include "unistd.h"
#include "mqtt.h"
#include "topic.h"
#include "connect.h"
#include "zhssqlite.h"
using namespace rapidjson;

extern pthread_mutex_t mutex;           //互斥锁申请
/*
 * MQTT Topic
 *
 * 字符串中的 %s 将由 cpuid 替换
 *
 * refr: 
 * MqttInit()
 * onConnect(), 订阅 MQTT topic
 * connlost(), 取消订阅 MQTT topic
 */
char TOPIC[TOPICNUM][MAX_TOPIC_LEN] = {
    // 推车属性上报的话题 Topic: 0
    "/sys/acc_car/%s/thing/event/property/post"   
    ,
    // 推车属性订阅的话题 Topic: 1
    "/sys/acc_car/%s/thing/event/property/post_reply"
    ,
    // 推车故障上报的话题 Topic: 2
    "/sys/acc_car/%s/thing/event/fd/post"
    ,
    // 推车故障订阅的话题 Topic: 3    
    "/sys/acc_car/%s/thing/event/fd/post_reply"
    ,
    // 后台下发数据 Topic: 4
    "/sys/acc_car/%s/thing/service/move/set"
};


/*
 * 对 MQTT 相关信息进行初始化
 *
 * 包括: 
 * x. 初始化 MQTT 的 TOPIC
 * x. 初始化 clientid
 * x. 初始化 net_addr
 */

/*
 * refr: 
 * MqttInit(), 对其初始化
 */
static char clientid[200];
static char msgarrvd_buf[500];
char net_addr[50];
char net_id[50];
bool mqtt_running = false;      // MQTT 是否在运行
pthread_t mqtt_pthid;      // MqttAsyncServiceMain()线程 id
pthread_mutex_t mqtt_mutex;
pthread_cond_t mqtt_pending;
/*
 * MQTT 的连接是否断开
 *
 * refr: 
 * connlost(), 连接断开
 */
int mqtt_connect_lost = 0;
SQLITE_SELECTINFO selectinfo;
//bool mqtt_running = false; // MQTT 是否在运行
/*
 * MQTT 消息结构
 *
 * refr: 
 * msgarrvd(), 接收到 MQTT 消息时的回调函数
 */
#define PROPERTY_NUM 10

/*
 * 根据 cmd/tims, 产生 MQTT 的响应数据，保存在 data 中
 * 最终, data 将通过 MQTT 发送
 */
void ServiceAck(char *data, char cmd, int tims)
{
    Document document;
    /*
     * 如果不使用这个方法, AddMember()将会报错
     * 下面会遇到多次使用 allocator 的情况，采用这种方式避免多次调用 GetAllocator() 去获取 allocator
     */
    document.SetObject();
    Document::AllocatorType& allocator = document.GetAllocator();
    Value mainmemb;

    std::string str_id;
    static int heart_id = 1;
    char *bufid;
    char str_tim[50];
    char str_heat[50];
    char str_dns[50];
    char str_version[50];
    char str_ptr[50];

    str_id = getDeviceId();
    bufid = (char*)str_id.c_str();

    // id
    //mainmemb.SetString((const char*)net_id, strlen(net_id));
    //document.AddMember("id", mainmemb, allocator);

    switch (cmd) 
    {
        StringBuffer jsonBuffer;
        Writer<StringBuffer> writer(jsonBuffer);
        document.Accept(writer);
        const char* str = jsonBuffer.GetString();
        sprintf(data, "%s", str);
        DBG("%s[%d]: %s\n", __FUNCTION__, __LINE__, str);
    }
}




/*
 * 对 struct CTL_DEVICE 通过 rapidjson 进行整合，以便能传递给后台
 *
 * data, 通过 rapidjson 整合后的数据，它将传递给后台
 * dev, 受控设备及其上传的数据
 * load_subset_data(), 将车底层的接收到的数据'device_info'解析为 struct CTL_DEVICE，保存至 ctledDevice[0]
 */
pthread_mutex_t cnt_mutex;
void DevToServiceDataCreat(char *data, CTL_DEVICE dev)
{
    Document document;
    pthread_mutex_t cnt_mutex;
    AttrDataPack(&document,dev);
    pthread_mutex_unlock(&cnt_mutex);

    StringBuffer jsonBuffer;
    Writer<StringBuffer> writer(jsonBuffer);
    document.Accept(writer);

    const char *strb = jsonBuffer.GetString();
    sprintf(data, "%s", strb);
    DBG("%s[%d]: buf = %s\n", __FUNCTION__, __LINE__, strb);
}


/*
 * MQTT 工作线程
 *
 * 将: 
 * x. 创建 MQTT 客户端
 * x. 设置连接选项，并连接 MQTT
 */
void* MqttAsyncServiceMain(void *buf)
{
    /* MQTT 客户端 */
    MQTTAsync client;
    /* MQTT 连接选项 */
    MQTTAsync_connectOptions    conn_opts   = MQTTAsync_connectOptions_initializer;
    /* MQTT 发送消息后的应答选项 */
    MQTTAsync_responseOptions   opts        = MQTTAsync_responseOptions_initializer;
    /* MQTT 断开连接的选项 */
    MQTTAsync_disconnectOptions disc_opts   = MQTTAsync_disconnectOptions_initializer;
    /* 将发送的 MQTT 消息 */
    MQTTAsync_message           pubmsg      = MQTTAsync_message_initializer;
    int rc;
    int needAck = 0;
    int ackTopic = 0;
    int i = 0;
    char loadbuf[3000];
    static uint32_t timestamp = 0;
    struct timespec time_wait;

    /* 设置线程是可以取消的 */
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    
    /* 设置线程的取消类型，异步取消，线程接到取消信号后，立即退出 */
    pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);

    MqttInit();
    /* 创建 MQTT 客户端 */
    MQTTAsync_create(&client, net_addr, clientid, MQTTCLIENT_PERSISTENCE_NONE, NULL);
    /* 设置 MQTT 回调函数 */
    MQTTAsync_setCallbacks(client, NULL, connlost, msgarrvd, NULL);

    /* 设置 MQTT 连接选项 */
    conn_opts.keepAliveInterval = KEEPALIVEINTERVAL;
    conn_opts.cleansession = 0;
    conn_opts.username  = MQTTACCOUNT_name;
    conn_opts.password  = MQTTACCOUNT_pass;
    conn_opts.onSuccess = onConnect;
    conn_opts.onFailure = onConnectFailure;
    conn_opts.context   = client;
    conn_opts.automaticReconnect = 100;
    conn_opts.connectTimeout = 100;
    conn_opts.minRetryInterval = 1;
    conn_opts.maxRetryInterval = 100;

    /* 连接 MQTT */
    if ((rc = MQTTAsync_connect(client, &conn_opts)) != MQTTASYNC_SUCCESS) 
    {
        DBG("%s[%d]: Failed to start connect, return code %d\n", __FUNCTION__, __LINE__, rc);

        MQTTAsync_destroy(&client);
        mqtt_connect_lost = 1;

        return NULL;
    }

    //ConcentratorHearting(&client, &timestamp);

    while (!mqtt_connect_lost) 
    {
       
        pthread_mutex_lock(&mutex);      //加锁，若有线程获得锁，则会堵塞
        /* 本地传感器数据主动上传 */
        
        if (get_ssecond() >= timestamp + UPLOADINTERVAL) 
        {
//            ConcentratorHearting(&client, &timestamp);
        }
////////////////////////TODO(xiaozhong)属性上报///////////////////////////
        /* 阻塞等待信号量，时间 1 秒 */
        time_wait.tv_sec = 1;       
//      if (0 == sem_timedwait(&sqlite_updata_sem, &time_wait)) 
        {
            /* 将 UPDATA_FILE 表中的新记录发送至 MQTT, 发送成功将从数据库中删除此记录 */
//           DBG("%s[%d]: sem_timedwait sqlite_updata_sem\n", __FUNCTION__, __LINE__);
            //sqlite_data_select(NULL, UPDATA_FILE, sqlite_select_hundred, selectinfo.id, selectinfo.data, NULL, NULL);
            if (selectinfo.data != NULL)
            {
                CTL_DEVICE dev;
//              DBG("%s[%d]: select data: %s\r\n", __FUNCTION__, __LINE__, selectinfo.data);
                char loadbuf[300];
                printf("\r\n----------------xiaozhong--------------\r\n");
                DevToServiceDataCreat(loadbuf,dev);
                pubmsg.payload = loadbuf;//selectinfo.data;
                pubmsg.payloadlen = strlen(loadbuf/*selectinfo.data*/);
                needAck = 1;
                ackTopic = CAR_UPLOAD;
//                sscanf(selectinfo.id, "%d", &updata_db_rec.id);
//                opts.onSuccess = on_send_delrec;
//                opts.context   = &updata_db_rec;
                ZhsMQTTAsync_sendMessage(&client, &pubmsg, ackTopic, &opts);
                
            }
        }
////////////////////////TODO(xiaozhong)属性上报///////////////////////////

////////////////////////TODO(xiaozhong)故障上传///////////////////////////

#if 0         
        if (0 == sem_timedwait(&control_ack_sem, &time_wait)) 
        {
            DBG("%s[%d]: sem_timedwait control_ack_sem\n", __FUNCTION__, __LINE__);

            memset(loadbuf, 0, sizeof(loadbuf));

            /* TODO: 确定这里应该传输 client 还是 &client */
            PackMQTTMsg(loadbuf, client, CONTROL_REPLY, &pubmsg, &opts, on_send);
            subset_status = idle;
            ackTopic = DEVICE_CONTROL_ACK;

            MQTTAsync_sendMessage(&client, &pubmsg, ackTopic, &opts);
        }

        if (0 == sem_timedwait(&upgrade_sem, &time_wait)) 
        {
            DBG("%s[%d]: sem_timedwait upgrade_sem\n", __FUNCTION__, __LINE__);

            memset(loadbuf, 0, sizeof(loadbuf));

            /* TODO: 确定这里应该传输 client 还是 &client */
            PackMQTTMsg(loadbuf, client, UPGRADE_REPLY, &pubmsg, &opts, on_send);
            subset_status = idle;
            ackTopic = DEVICE_UPGRADE_ACK;

            MQTTAsync_sendMessage(&client, &pubmsg, ackTopic, &opts);
        }

        if (0 == sem_timedwait(&bind_sem, &time_wait)) 
        {
            DBG("%s[%d]: sem_timedwait bind_sem\n", __FUNCTION__, __LINE__);

            memset(loadbuf, 0, sizeof(loadbuf));

            /* TODO: 确定这里应该传输 client 还是 &client */
            PackMQTTMsg(loadbuf, client, BINDING_QUERY, &pubmsg, &opts, on_send);
            subset_status = idle;
            ackTopic = DEVICE_CONTROL_ACK;

            MQTTAsync_sendMessage(&client, &pubmsg, ackTopic, &opts);
        }

        if (0 == sem_timedwait(&paraming_sem, &time_wait)) 
        {
            DBG("%s[%d]: sem_timedwait paraming_sem\n", __FUNCTION__, __LINE__);

            memset(loadbuf, 0, sizeof(loadbuf));

            /* TODO: 确定这里应该传输 client 还是 &client */
            PackMQTTMsg(loadbuf, client, PARAMING_QUERY, &pubmsg, &opts, on_send);
            subset_status = idle;
            ackTopic = DEVICE_CONTROL_ACK;

            MQTTAsync_sendMessage(&client, &pubmsg, ackTopic, &opts);
        }

        if (0 == sem_timedwait(&selfchecking_sem, &time_wait)) 
        {
            DBG("%s[%d]: sem_timedwait selfchecking_sem\n", __FUNCTION__, __LINE__);

            memset(loadbuf, 0, sizeof(loadbuf));

            /* TODO: 确定这里应该传输 client 还是 &client */
            PackMQTTMsg(loadbuf, client, SLEFCHECKING, &pubmsg, &opts, on_send);
            subset_status = idle;
            ackTopic = DEVICE_CONTROL_ACK;

            MQTTAsync_sendMessage(&client, &pubmsg, ackTopic, &opts);
        }
#endif
        pthread_mutex_unlock(&mutex);   //解锁
        usleep(10000000L);
    }

    /* MQTT 断开连接 */
    disc_opts.onSuccess = onDisconnect;
    if ((rc = MQTTAsync_disconnect(client, &disc_opts)) != MQTTASYNC_SUCCESS) 
        DBG("%s[%d]: Failed to start disconnect, return code %d\n", __FUNCTION__, __LINE__, rc);
    MQTTAsync_destroy(&client);
    return NULL;
}



void MqttServic_stop(void)
{
    mqtt_running = false;
    mqtt_connect_lost = 1;
    pthread_cancel(mqtt_pthid);
}


/*
 * 将创建 MqttAsyncServiceMain() MQTT 主线程
 * 而后通过 pthread_join()阻塞等待该线程结束，并尝试重新创建它
 */
int MqttServiceMain()
{
    int ret = -1;
    int retry = 1;
    int netpingtime;

    mqtt_running = true;
    mqtt_connect_lost = 0;

    pthread_mutex_init(&mqtt_mutex, NULL);
    pthread_cond_init(&mqtt_pending, NULL);

    while (mqtt_running) 
    {
        /* 创建 MQTT 主线程 */
        ret = pthread_create(&mqtt_pthid, NULL, MqttAsyncServiceMain, NULL);
        if (ret != 0) 
        {
            DBG("[%d]: Creat pthread error:%d\n", __LINE__, ret);
            sleep(3);
            continue;
        }

        /*
         * 调用 pthread_join() 以阻塞等待指定的线程结束
         */
        ret = pthread_join(mqtt_pthid, NULL);
        DBG("\n\n[%d]: MqttAsyncServiceMain thread dead time: %d \n\n", __LINE__, retry); 
    }
    return ret;
}


void MqttInit()
{
    int i = 0;
    char *buf;
    std::string str;
    char topicTmp[100];
    
    str = getDeviceId();
    if (str.empty())
        str = DEVICE_ID;

    buf = (char*)str.c_str();
    DBG("[%d]: init deviceid=%s\n", __LINE__, buf);

    /* 初始化 MQTT 的 TOPIC */
    for (i = 0; i < TOPICNUM; i++) {
        memset(topicTmp, 0, strlen(topicTmp));
        strcat(topicTmp, TOPIC[i]);

        memset(TOPIC[i], 0, strlen(TOPIC[i]));
        sprintf(TOPIC[i], topicTmp, buf);
        DBG("%s[%d]: topic %d=%s\n", __FUNCTION__, __LINE__, i, TOPIC[i]);
    }

    /* 初始化 clientid */
    memset(clientid, 0, strlen(clientid));
    sprintf(clientid, "241_zhaokangxu_%s", buf);
    DBG("[%d]: clientid=%s\n", __LINE__, clientid);
    
    mqtt_connect_lost = 0;

    /* 初始化 net_addr */
    memset(selectinfo.data, 0, sizeof(selectinfo.data));
//    sqlite_data_select(NULL, SERVER_DNS, sqlite_select_hundred, NULL, selectinfo.data, NULL, NULL);
    if (strlen(selectinfo.data) == 0) {
        memset(net_addr, 0, strlen(net_addr));
        sprintf(net_addr, "tcp://182.150.115.28:2883");
    }
    else {
        memset(net_addr, 0, strlen(net_addr));
        strcpy(net_addr, selectinfo.data);
    }
    DBG("[%d]: net_addr=%s\n", __LINE__, net_addr);
}

