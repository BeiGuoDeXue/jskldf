/**
 * @file message.cpp
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
    MQTT消息发送的处理函数
**/
#include <string>
#include <string.h>
#include "message.h"
#include "dbg.h"
#include "topic.h"
#include "ulitity.h"
#include "service.h"
#include "device.h"
#include "connect.h"
#include "zhs.h"
#include "rapidjson/document.h"
/******************************************************************/
#include "config.h"
#include "bsp_car_drive.h"
#include "bsp_gps.h"
#include "bsp_imu.h"
#include "bsp_tag.h"
#include "bsp_readfile.h"
#include "bsp_ble.h"
#include "bsp_lte.h"
#include "app.h"

using namespace rapidjson;

/***************************车子信息的全局变量***************************************/
extern cardrive_info_t cardrive_info;
extern uwb_info_t uwb_info;
extern gps_info_t gps_info;
extern user_order_t userorder_info;
extern carpos_t car_pos;
extern let_info_t let_info;

mqtt_user_info_t mqtt_user_info;
extern CAR_MODE_t server_origin_mode;
extern int server_flag;
/******************************************************************/
Subset_Status subset_status;
static char clientid[200];
static char msgarrvd_buf[500];
extern char net_id[50];
extern char TOPIC[TOPICNUM][MAX_TOPIC_LEN];
char strBuff[50][20] = {0};
/**********************************需要上发信息*************************************/
device_info Devinfo;
xyz_info Xyzinfo; //定位信息，参照属性
car_info Carinfo; //车辆状态信息
lte_info_mqtt Lteinfo; //4G模组相关信息
boxa_info Boxainfo;//水箱信息
boxb_info Boxbinfo; //冰箱信息
boxc_info Boxcinfo;//顶棚信息（最高层）
r_info Rinfo;//冰箱电池属性
/**********************************添加上发信息函数*************************************/
static void add_TimeJson(Document *Document,Value* CarRInfo);
static void add_CarRInfoJson(Document *document,Value* CarRInfo);
static void add_CarboxCJson(Document *document,Value* CarboxC);
static void add_CarboxBJson(Document *document,Value* CarboxB);
static void add_CarboxAJson(Document *document,Value* CarboxA);
static void add_Car4GInfoJson(Document *document,Value* Car4GInfo);
static void add_CarInfoJson(Document *document,Value* CarInfo);
static void add_CarXYZJson(Document *document,Value* CarXYZ);
static void add_MethodJson(Document *document);
static void add_CarParaJson(Document *document,Value* CarPara);
static void add_deviceJson(Document *document);
/**********************************赋值上发信息函数*************************************/
static void set_Deviceinfo(device_info *Deviceinfo);
static void set_XYZinfo(xyz_info *XYZinfo);
static void set_Carinfo(car_info *Carinfo);
static void set_Lteinfo(lte_info_mqtt *Lteinfo);
static void set_Boxa(boxa_info *Boxa);
static void set_Boxb(boxb_info *Boxb);
static void set_Boxc(boxc_info *Boxc);
static void set_Rinfo(r_info *Rinfo);

/*
 * 通过 MQTTAsync_sendMessage()向 MQTT 发送消息
 */
void ZhsMQTTAsync_sendMessage(MQTTAsync *client, MQTTAsync_message *pubmsg, 
    int ackTopic, MQTTAsync_responseOptions *opts)
{
    int ret;
    
    pubmsg->qos = QOS;
    pubmsg->retained = 0;
    
#if 1 
    //printf("topic %s\n", TOPIC[ackTopic]);  
    if ((ret = MQTTAsync_sendMessage(*client, TOPIC[ackTopic], pubmsg, opts)) != MQTTASYNC_SUCCESS)
    {
        ERR("%s[%d]: Failed to start sendMessage, return code %d\n", __FUNCTION__, __LINE__, ret);
    }
    else
        DBG("%s[%d]: start sendMessage\n", __FUNCTION__, __LINE__);
#endif
}

void PackMQTTMsg(char *loadbuf, void *context, char cmd, 
    MQTTAsync_message *pubmsg, MQTTAsync_responseOptions *opts, 
    MQTTAsync_onSuccess on_success)
{
    int times = get_ssecond();

    ServiceAck(loadbuf, cmd, times);
    pubmsg->payload = (char *)loadbuf;
    pubmsg->payloadlen = strlen(loadbuf);

    opts->onSuccess = on_success;
    opts->context   = context;
}


/* 发送推车心跳包 */
void ConcentratorHearting(MQTTAsync *client, uint32_t *timestamp)
{
    char loadbuf[3000];
    char *pbuf = loadbuf;
    int ackTopic = 0;
    uint32_t cur;
    /* 将发送的 MQTT 消息 */
    MQTTAsync_message pubmsg = MQTTAsync_message_initializer;
    /* MQTT 发送消息后的应答选项 */
    MQTTAsync_responseOptions opts = MQTTAsync_responseOptions_initializer;
    
    subset_status = hearting;

    /* 向 MQTT 服务器发送心跳包 */
    subset_status = idle;
    DBG("%s[%d]: HEATING___________________________ \n", __FUNCTION__, __LINE__);

    cur = get_ssecond();
    if (timestamp) 
        *timestamp = cur;

    memset(loadbuf, 0, sizeof(loadbuf));
    ServiceAck(loadbuf, HEATING, cur);
    
    pubmsg.payload = (char*)loadbuf;
    pubmsg.payloadlen = strlen(loadbuf);
    ackTopic = CAR_UPLOAD;

    opts.onSuccess = on_send;
    opts.context   = client;

    ZhsMQTTAsync_sendMessage(client, &pubmsg, ackTopic, &opts);
}


#if 0
int msgarrvd(void *context, char *topicName, int topicLen, MQTTAsync_message *message)
{
    int i;
    char* payloadptr;

    printf("Message arrived\n");
    printf("     topic: %s\n", topicName);
    printf("   message: ");

    payloadptr = (char*)message->payload;
    for(i=0; i<message->payloadlen; i++)
    {
        putchar(*payloadptr++);
    }
    putchar('\n');
    //MQTTClient_freeMessage(&message);
    //MQTTClient_free(topicName);
    return 1;
}
#else
/*
 * 接收到 MQTT 消息时的回调函数
 */
int msgarrvd(void *context, char *topicName, int topicLen, MQTTAsync_message *message)
{
    int targetTopic = 0;
    int ackTopic = 0;
    char needAck = 0, rc = 0;
    char buf[128];
    char arr[64];
    char tmp[64];
    char ptr[64];
    int cnt = 0;
    MQTTMSG mqttAck;
    MQTTAsync client = (MQTTAsync)context;
    MQTTAsync_responseOptions opts = MQTTAsync_responseOptions_initializer;
    MQTTAsync_message pubmsg = MQTTAsync_message_initializer;

    opts.onSuccess = on_send;
    opts.context   = client;

    DBG("%s[%d] topicName_topic:%s\n", __FUNCTION__, __LINE__, topicName);
    DBG("%s[%d] rev_message:%s\n", __FUNCTION__, __LINE__, message->payload);

    memset(buf, 0x00, sizeof(buf));
    memset(arr, 0x00, sizeof(arr));
    memset(tmp, 0x00, sizeof(tmp));
    memset(ptr, 0x00, sizeof(ptr));


    // if(strcmp(topicName,"/sys/acc_car/432944698949007777/thing/service/move/set") == 0)
    // {
    //     printf("/sys/acc_car/432944698949007777/thing/service/move/set");
    // }
    memset(msgarrvd_buf, 0, sizeof(msgarrvd_buf)); // message->payload 结尾带有非法字符
    /* TODO: 如果 message->payload 的数据长度大于 sizeof(msgarrvd_buf), 造成数据丢失怎么办??? */
    if (message->payloadlen <= sizeof(msgarrvd_buf))
        strncpy(msgarrvd_buf, (char *)message->payload, message->payloadlen);
    else
        strncpy(msgarrvd_buf, (char *)message->payload, sizeof(msgarrvd_buf));
    ////////////////////////////////数据收到的回调函数//////////////////////////////////
    /* 把 msgarrvd_buf 转换成 MQTTMSG */
    mqttAck = ServiceDataAnalysis(msgarrvd_buf, targetTopic - 1);
    ////////////////////////////////数据收到的回调函数//////////////////////////////////
    
    memset(net_id, 0, sizeof(net_id));
    strcpy(net_id, mqttAck.id.c_str());

    DBG("%s[%d]: msgType: %d\n", __FUNCTION__, __LINE__, mqttAck.msgtype);
#if 0
    switch (mqttAck.msgtype) {
        case CONTROL_SW: {
            DBG("%s[%d]: recved MQTT CONTROL_SW\n", __FUNCTION__, __LINE__);
            strcpy(arr, mqttAck.productId.c_str());

            /* 命令类型码, 1 byte */
            buf[cnt++] = Ctl_cmd;

            /* 子设备类型, 1 byte */
            buf[cnt++] = mqttAck.property[0].type;

            /* 子设备编码地址 id, 8 byte */
            buf[cnt++] = (char_to_hex(arr[0])<<4) + (char_to_hex(arr[1]));
            buf[cnt++] = (char_to_hex(arr[2])<<4) + (char_to_hex(arr[3]));
            buf[cnt++] = (char_to_hex(arr[4])<<4) + (char_to_hex(arr[5]));
            buf[cnt++] = (char_to_hex(arr[6])<<4) + (char_to_hex(arr[7]));
            buf[cnt++] = (char_to_hex(arr[8])<<4) + (char_to_hex(arr[9]));
            buf[cnt++] = (char_to_hex(arr[10])<<4) + (char_to_hex(arr[11]));
            buf[cnt++] = (char_to_hex(arr[12])<<4) + (char_to_hex(arr[13]));
            buf[cnt++] = (char_to_hex(arr[14])<<4) + (char_to_hex(arr[15]));

            /* 开关输出状态 */
            buf[cnt++] = 0x26;
            buf[cnt++] = 0x01;

            /* 控制开关 */
            buf[cnt++] = mqttAck.property[0].value[0];

            Uart1_Send_Data((uint8_t*)buf, cnt);
            break;
        }
        
        case SET_BIND_PARAM: {
            char sr_addr[28];
            char dst_addr[28];

            DBG("%s[%d]: recved MQTT SET_BIND_PARAM\n", __FUNCTION__, __LINE__);
            memset(sr_addr, 0, sizeof(sr_addr));
            memset(dst_addr, 0, sizeof(dst_addr));

            /* 命令类型码, 1 byte */
            buf[cnt++] = Pmtset_cmd;

            /* 子设备类型, 1 byte */
            buf[cnt++] = mqttAck.property[0].type;

            /* 子设备编码地址 id, 8 byte */
            strcpy(sr_addr, mqttAck.productId.c_str());
            memcpy(tmp, sr_addr, strlen(sr_addr));
            buf[cnt++] = (char_to_hex(sr_addr[0])<<4) + (char_to_hex(sr_addr[1]));
            buf[cnt++] = (char_to_hex(sr_addr[2])<<4) + (char_to_hex(sr_addr[3]));
            buf[cnt++] = (char_to_hex(sr_addr[4])<<4) + (char_to_hex(sr_addr[5]));
            buf[cnt++] = (char_to_hex(sr_addr[6])<<4) + (char_to_hex(sr_addr[7]));
            buf[cnt++] = (char_to_hex(sr_addr[8])<<4) + (char_to_hex(sr_addr[9]));
            buf[cnt++] = (char_to_hex(sr_addr[10])<<4) + (char_to_hex(sr_addr[11]));
            buf[cnt++] = (char_to_hex(sr_addr[12])<<4) + (char_to_hex(sr_addr[13]));
            buf[cnt++] = (char_to_hex(sr_addr[14])<<4) + (char_to_hex(sr_addr[15]));

            buf[cnt++] = 0x7A;  //pid
            buf[cnt++] = 0x02;
            buf[cnt++] = mqttAck.property[1].value[0]>>8;
            buf[cnt++] = mqttAck.property[1].value[1];

            buf[cnt++] = 0x7B;  //ch
            buf[cnt++] = 0x01;
            buf[cnt++] = mqttAck.property[2].value[0];

            buf[cnt++] = 0x7C;  //dt
            buf[cnt++] = 0x01;
            buf[cnt++] = mqttAck.property[0].value[0];

            buf[cnt++] = 0x7D;  //bd
            buf[cnt++] = 0x0C;
            buf[cnt++] = mqttAck.property[3].value[0];//绑定
            buf[cnt++] = mqttAck.property[5].value[0];//方向
            buf[cnt++] = 0;                           //被绑定设备的类型
            buf[cnt++] = mqttAck.property[4].value[0];

            strcpy(dst_addr, mqttAck.dstsid.c_str());
            memcpy(ptr, dst_addr, strlen(dst_addr));
            buf[cnt++] = (char_to_hex(dst_addr[0])<<4) + (char_to_hex(dst_addr[1]));  //被绑定设备地址id
            buf[cnt++] = (char_to_hex(dst_addr[2])<<4) + (char_to_hex(dst_addr[3]));
            buf[cnt++] = (char_to_hex(dst_addr[4])<<4) + (char_to_hex(dst_addr[5]));
            buf[cnt++] = (char_to_hex(dst_addr[6])<<4) + (char_to_hex(dst_addr[7]));
            buf[cnt++] = (char_to_hex(dst_addr[8])<<4) + (char_to_hex(dst_addr[9]));
            buf[cnt++] = (char_to_hex(dst_addr[10])<<4) + (char_to_hex(dst_addr[11]));
            buf[cnt++] = (char_to_hex(dst_addr[12])<<4) + (char_to_hex(dst_addr[13]));
            buf[cnt++] = (char_to_hex(dst_addr[14])<<4) + (char_to_hex(dst_addr[15]));

            if (mqttAck.property[0].type == 0x18) {
                buf[cnt++] = 0x7E;  // pic
                buf[cnt++] = 0x01;
                buf[cnt++] = mqttAck.property[6].value[0];
            }

            Uart1_Send_Data((uint8_t*)buf, cnt);

            /* 在 BIND_FILE 数据库中添加记录，只保存 mqttAck.productId */
            sqlite_data_insert(NULL, BIND_FILE, tmp, ptr, 0);
            break;
        }

        case SET_THRESHOLD: {
            DBG("%s[%d]: recved MQTT SET_THRESHOLD\n", __FUNCTION__, __LINE__);
            strcpy(arr, mqttAck.productId.c_str());

            /* 命令类型码, 1 byte */
            buf[cnt++] = Pmtset_cmd;

            /* 子设备类型, 1 byte */
            buf[cnt++] = mqttAck.property[0].type;

            /* 子设备编码地址 id, 8 byte */
            buf[cnt++] = (char_to_hex(arr[0])<<4) + (char_to_hex(arr[1]));
            buf[cnt++] = (char_to_hex(arr[2])<<4) + (char_to_hex(arr[3]));
            buf[cnt++] = (char_to_hex(arr[4])<<4) + (char_to_hex(arr[5]));
            buf[cnt++] = (char_to_hex(arr[6])<<4) + (char_to_hex(arr[7]));
            buf[cnt++] = (char_to_hex(arr[8])<<4) + (char_to_hex(arr[9]));
            buf[cnt++] = (char_to_hex(arr[10])<<4) + (char_to_hex(arr[11]));
            buf[cnt++] = (char_to_hex(arr[12])<<4) + (char_to_hex(arr[13]));
            buf[cnt++] = (char_to_hex(arr[14])<<4) + (char_to_hex(arr[15]));

            buf[cnt++] = 0x0C;  // 预警上限设置
            buf[cnt++] = 0x03;
            buf[cnt++] = mqttAck.property[1].value[0];     // 预警类型
            buf[cnt++] = mqttAck.property[2].value[0] >> 8;  // 预警值
            buf[cnt++] = mqttAck.property[2].value[1];

            Uart1_Send_Data((uint8_t*)buf, cnt);
            break;
        }

        /* TODO: 没有对这个消息进行回复 */
        case SET_SYS_PARAM: {
            int par_times = get_ssecond();

            DBG("%s[%d]: recved MQTT SET_SYS_PARAM\n", __FUNCTION__, __LINE__);

            strcpy(arr, mqttAck.dns.c_str());
            strcpy(buf, mqttAck.version.c_str());

            /* dns */
            memset(selectinfo.data, 0, sizeof(selectinfo.data));
            sqlite_data_select(NULL, SERVER_DNS, sqlite_select_hundred, NULL, selectinfo.data, NULL, NULL);
            if (strlen(selectinfo.data) == 0) {
                sqlite_data_insert(NULL, SERVER_DNS, arr, NULL, par_times);
                sqlite_data_select(NULL, SERVER_DNS, sqlite_select_hundred, NULL, selectinfo.data, NULL, NULL);
                DBG("selectinfo is dns_insert = %s\n", selectinfo.data);
            }
            else {
                sqlite_data_update(SERVER_DNS, arr, NULL, par_times);
                sqlite_data_select(NULL, SERVER_DNS, sqlite_select_hundred, NULL, selectinfo.data, NULL, NULL);
                DBG("selectinfo is dns_update = %s\n", selectinfo.data);
            }

            /* version */
            memset(selectinfo.data, 0, sizeof(selectinfo.data));
            sqlite_data_select(NULL, GATE_VERSION, sqlite_select_hundred, NULL, selectinfo.data, NULL, NULL);
            if (strlen(selectinfo.data) == 0) {
                sqlite_data_insert(NULL, GATE_VERSION, buf, NULL, par_times);
                sqlite_data_select(NULL, GATE_VERSION, sqlite_select_hundred, NULL, selectinfo.data, NULL, NULL);
                DBG("selectinfo is version_insert = %s\n", selectinfo.data);
            }
            else {
                sqlite_data_update(GATE_VERSION, buf, NULL, par_times);
                sqlite_data_select(NULL, GATE_VERSION, sqlite_select_hundred, NULL, selectinfo.data, NULL, NULL);
                DBG("selectinfo is version_update = %s\n", selectinfo.data);
            }
            break;
        }

        case SYS_PARAM_QUERY: {
            pthread_mutex_lock(&mqtt_mutex);
            subset_status = sys_paraming;
            sem_post(&paraming_sem);
            pthread_mutex_unlock(&mqtt_mutex);
            break;
        }

        case DEV_BIND_QUERY: {
            // DBG("%s[%d]\n", __FUNCTION__, __LINE__);
            pthread_mutex_lock(&mqtt_mutex);
            subset_status = bind_acking;
            sem_post(&bind_sem);
            pthread_mutex_unlock(&mqtt_mutex);
            break;
        }
        
        case DEV_UPGRADE: {
            DBG("%s[%d]: recved MQTT DEV_UPGRADE\n", __FUNCTION__, __LINE__);
            char ver[64] = {0};
            char cmd[128] = {0};
            char url[128] = {0};
            char names[128] = {0};

            if (mqttAck.property[0].type == 0x100) {
                strcpy(arr, mqttAck.productId.c_str());
                strcpy(ver, mqttAck.version.c_str());
                strcpy(url, mqttAck.url.c_str());

                // subset_status = upgrade_acking;
            }
            else {
                strcpy(arr, mqttAck.productId.c_str());
                strcpy(ver, mqttAck.version.c_str());
                strcpy(url, mqttAck.url.c_str());

                sprintf(cmd, "wget %s", url);
                sprintf(names, "%s", mqttAck.name.c_str());
                /* 通过 wget 下载文件 */
                system(cmd);
                rc = get_url(cmd, names);

                /* 命令类型码, 1 byte */
                buf[cnt++] = Upgrade_cmd;

                /* 子设备类型, 1 byte */
                buf[cnt++] = mqttAck.property[0].type;

                /* 子设备编码地址 id, 8 byte */
                buf[cnt++] = (char_to_hex(arr[0])<<4) + (char_to_hex(arr[1]));
                buf[cnt++] = (char_to_hex(arr[2])<<4) + (char_to_hex(arr[3]));
                buf[cnt++] = (char_to_hex(arr[4])<<4) + (char_to_hex(arr[5]));
                buf[cnt++] = (char_to_hex(arr[6])<<4) + (char_to_hex(arr[7]));
                buf[cnt++] = (char_to_hex(arr[8])<<4) + (char_to_hex(arr[9]));
                buf[cnt++] = (char_to_hex(arr[10])<<4) + (char_to_hex(arr[11]));
                buf[cnt++] = (char_to_hex(arr[12])<<4) + (char_to_hex(arr[13]));
                buf[cnt++] = (char_to_hex(arr[14])<<4) + (char_to_hex(arr[15]));

                buf[cnt++] = 0x05;  // version
                buf[cnt++] = 0x05;
                buf[cnt++] = ver[0];
                buf[cnt++] = ver[1];
                buf[cnt++] = ver[2];
                buf[cnt++] = ver[3];
                buf[cnt++] = ver[4];

                buf[cnt++] = 0x85;  // 版本类型
                buf[cnt++] = 0x01;
                buf[cnt++] = mqttAck.version_type;//0=A，1=B

                Uart1_Send_Data((uint8_t*)buf, cnt);
            }
            break;
        }

        case SYS_MACHINE_SELFCHECK: {
            pthread_mutex_lock(&mqtt_mutex);
            subset_status = sys_selfchecking;
            sem_post(&selfchecking_sem);
            pthread_mutex_unlock(&mqtt_mutex);
            break;
        }

        default :
            break;
    }

    MQTTAsync_freeMessage(&message);
    MQTTAsync_free(topicName);
#endif
    return 1;   
}
#endif


/*
 * 把 buf 转换成 MQTTMSG
 *
 * 用文件流把这个字符串送到一个 string 里，然后创建一个 Document 对象
 * 再把 string 转成 const char *类型以后送到 rapidjson 自带的函数 Parse 里处理就好了
 */
MQTTMSG ServiceDataAnalysis(const char *buf, int topicNum)
{
    Document document;
    MQTTMSG mqttmsg;
    int res_value = 0;
    // DBG("---------------%s-----%d------\r\n%s-----------------\r\n", __FUNCTION__, __LINE__, buf);
    // DBG("%s[%d]: mqtt recved:\r\n %s\r\n", __FUNCTION__, __LINE__, buf);
    
    if (document.Parse(buf).HasParseError()) {
        DBG("%s[%d]: mqtt HasParseError...\r\n", __FUNCTION__, __LINE__);
        return mqttmsg;
    }
    
    // 下行回复
    if (document.HasMember("code")) {
        rapidjson::Value &code = document["code"];
        mqttmsg.code = code.GetInt();
        if(document.HasMember("id"))
        {
            mqttmsg.id = document["id"].GetString();
        }
        mqttmsg.msgtype = ACK;
    }
    else { // 下行控制
        // id
        if(document.HasMember("id"))
        {
            mqttmsg.id = document["id"].GetString();
        }
        //ctp
        if(document.HasMember("ctp"))
        {
            mqttmsg.property[0].type = document["ctp"].GetInt();
        }
        //sid
        if(document.HasMember("sid"))
        {
            mqttmsg.productId = document["sid"].GetString();
        }

        //method
        if(document.HasMember("method"))
        {
            mqttmsg.method = document["method"].GetString();
            if(strstr(mqttmsg.method.c_str(), "thing.service.ssw") != NULL)
                mqttmsg.msgtype = CONTROL_SW;
            else if(strstr(mqttmsg.method.c_str(), "thing.service.move.set") != NULL)
                mqttmsg.msgtype = REMOTE_CONTROL_MOVE;
        }
        
        //params
        if(document.HasMember("params"))
        {
            switch(mqttmsg.msgtype)
            {
                case CONTROL_SW:
                {
                    mqttmsg.property[0].value[0] = document["params"]["sw"].GetInt();
                    DBG("Params_sw:%d\n",mqttmsg.property[0].value[0]);
                    break;
                }
                case SET_BIND_PARAM:
                {
                    mqttmsg.property[3].value[0] = document["params"]["bd"].GetInt();
                    DBG("Params_bd:%d\n",mqttmsg.property[3].value[0]);
                    
                    mqttmsg.property[4].value[0] = document["params"]["dstctp"].GetInt();
                    DBG("Params_dstctp:%d\n",mqttmsg.property[4].value[0]);
                    
                    mqttmsg.dstsid = document["params"]["dstsid"].GetString();
                    DBG("Params_dstsid:%s\n", mqttmsg.dstsid.c_str());
                    
                    if(document["params"].HasMember("dir"))
                    {
                        DBG("%s,%d\n",__FUNCTION__,__LINE__);
                        mqttmsg.dir = document["params"]["dir"].GetString();

                        DBG("%s[%d]: Params_dir:%d\n", 
                            __FUNCTION__, __LINE__, mqttmsg.property[5].value[0]);
                    }
                    else
                    {
                        mqttmsg.property[5].value[0] = 0xff;
                        DBG("Params_dir:%d\n",mqttmsg.property[5].value[0]);
                    }
                    
                    if(document["params"].HasMember("pic"))
                    {
                        mqttmsg.property[6].value[0] = document["params"]["pic"].GetInt();
                        DBG("Params_pic:%d\n",mqttmsg.property[6].value[0]);
                    }
                    break;
                }
                case SET_SYS_PARAM:
                {
                    mqttmsg.dns = document["params"]["dns"].GetString();
                    mqttmsg.version = document["params"]["ver"].GetString();
                    break;
                }
                case DEV_UPGRADE:
                {
                    mqttmsg.version = document["params"]["version"].GetString();
                    mqttmsg.version_type = document["params"]["vtype"].GetInt();
                    mqttmsg.url = document["params"]["name"].GetString();
                    mqttmsg.name = document["params"]["url"].GetString();
                    break;
                }
                case REMOTE_CONTROL_MOVE:
                {
                    server_origin_mode = userorder_info.car_mode;
                    server_flag = 1;
                    //userorder_info.car_mode = SERVER_REMOTE;
                    sscanf(document["params"]["move_x"].GetString(),"%f",&mqtt_user_info.disX);
                    sscanf(document["params"]["move_y"].GetString(),"%f",&mqtt_user_info.disY);
                    printf("disX = %f\n",mqtt_user_info.disX);
                    printf("disY = %f\n",mqtt_user_info.disY);
                    break;
                }
                default:
                    break;
            }
        }
        else
        {
            mqttmsg.params = "NULL";
        }
    }
    
    mqttmsg.topic = topicNum;
    
    return mqttmsg;
}

/***************************************赋值发送信息函数体*******************************************/
/**
 * @brief Set the Deviceinfo object
 * @param  Deviceinfo       My Param doc
 */
static void set_Deviceinfo(device_info *Deviceinfo)
{
    // Deviceinfo->str_id; //id String 消息ID号
    strcpy(Deviceinfo->version,"1.0.0"); //version String 协议版本号，目前协议版本1.0
    strcpy(Deviceinfo->business,"acc_car"); 
    strcpy(Deviceinfo->cid,"432944698949007777");//设备唯一标识
    strcpy(Deviceinfo->type,"241");//设备唯一标识
}
/**
 * @brief Set the XYZinfo object
 * @param  XYZinfo          My Param doc
 */
static void set_XYZinfo(xyz_info *XYZinfo)
{
    XYZinfo->indoor = 0; //表示定位方式( 0室内/1室外
    XYZinfo->longitude = gps_info.addr.Longitude; //经度
    XYZinfo->lattitude = gps_info.addr.Longitude; //纬度
    XYZinfo->floor = 1; //所在楼层（室内）
    strcpy(XYZinfo->area , "A"); //所在区域（室内）
    XYZinfo->area_posx = uwb_info.addr[UWB_FRONT].x; //所在区域内X方向的坐标
    XYZinfo->area_posy = uwb_info.addr[UWB_FRONT].y; //所在区域内Y方向的坐
}
/**
 * @brief Set the Carinfo object
 * @param  Carinfo          My Param doc
 */
static void set_Carinfo(car_info *Carinfo)
{
    Carinfo->b_lock = 1; //手环状态(0未使用/1已使用)
    Carinfo->ds = STOP_MODE; //行驶状态（0自动跟随|1手动控制|2送货模式|3载人模式|）（预留）
    Carinfo->bat = 100; //伴置车电池容量（百分比）
    Carinfo->km = 156; //伴置车里程数,单位km（累计）
    Carinfo->can_km = 26; //伴置车还可以行驶里程数，单位km
    Carinfo->user_km = 26; //伴置车用户行驶里程数，单位km
    Carinfo->soc = 0;//伴置车充电状态（0未充电|1充电中）
    Carinfo->ui = cardrive_info.battery_AH; //车电流（单位A）
    Carinfo->uv = cardrive_info.battery_Vdd; //车电流（单位A）
    Carinfo->rc = 14; //伴置车充电次数
    Carinfo->pitch = car_pos.pitch;//车辆俯仰角
    Carinfo->yaw = car_pos.yaw;//车厢航向角
    Carinfo->roll = car_pos.roll; //车厢航向角
    Carinfo->speed = car_pos.speed; //车辆行驶速度（单位m/
}
/**
 * @brief Set the Lteinfo object
 * @param  Lteinfo          My Param doc
 */
static void set_Lteinfo(lte_info_mqtt *Lteinfo)
{
    memcpy(Lteinfo->iccid,let_info.qccid.qccid,sizeof(let_info.qccid.qccid));
    //strcpy(Lteinfo->iccid,"1321313103131");//SIM卡号，由20位数字构成
    memcpy(Lteinfo->imei,let_info.qgsn.qgsn,sizeof(let_info.qgsn.qgsn));
    //strcpy(Lteinfo->imei,"213102");//4G模组设备身份证编号，由15位数字构成
    Lteinfo->sigquality = let_info.csq.rssi; //网络信息质量,范围0-31之间，数组越大表明信号质量越好。
}
/**
 * @brief Set the Boxa object
 * @param  Boxa             My Param doc
 */
static void set_Boxa(boxa_info *Boxa)
{
    Boxa->doora = 1; //仓门状态（0开|1关）
    Boxa->alev = 21; //水箱水位
    Boxa->aval = 1; //水箱放水阀门状态（0开|1关）
    Boxa->astr = 0; //水箱滤网状态（0平放|1提升）
    Boxa->at = 10; //温度，单位℃
    Boxa->aoxy = 1; //充氧状态（0未充氧|1充氧）
}
/**
 * @brief Set the Boxb object
 * @param  Boxb             My Param doc
 */
static void set_Boxb(boxb_info *Boxb)
{
    Boxb->doorb = 1;
    Boxb->bt = 1; //冰箱温度，单位℃
    Boxb->bref = 1; //冰箱制冷状态（0未制冷|1制冷）
}
/**
 * @brief Set the Boxc object
 * @param  Boxc             My Param doc
 */
static void set_Boxc(boxc_info *Boxc)
{
    Boxc->doorc = 1; //仓门状态（0开|1关）
}
/**
 * @brief Set the Rinfo object
 * @param  Rinfo            My Param doc
 */
static void set_Rinfo(r_info *Rinfo)
{
    Rinfo->rui = 1.2; //冰箱电池电流，单位A
    Rinfo->ruv = 20; //冰箱电池电压，单位V
    Rinfo->rbat = 100; //冰箱电池剩余电量
}
/*******************************************加入到json的函数体***********************************************/
void AttrDataPack(Document *document,CTL_DEVICE dev) 
{
    /*
     * 如果不使用这个方法, AddMember()会报错
     * 下面会遇到多次使用  allocator 的情况，采用这种方式避免多次调用 GetAllocator() 去获取 allocator
     */
    Document::AllocatorType& allocator = document->GetAllocator();
    document->SetObject();       

    Value mainmemb;

    add_deviceJson(document);
    // params
    //1.添加设备信息
    //5. params Object 请求参数
    //"params":{
    Value CarPara(kObjectType);
    CarPara.SetObject();
    add_CarParaJson(document,&CarPara);
    document->AddMember("params", CarPara, allocator);

    add_MethodJson(document);
}


static void add_deviceJson(Document *document)
{
    set_Deviceinfo(&Devinfo);
    Document::AllocatorType& allocator = document->GetAllocator();
    Value mainmemb;

    //1. id String 消息ID号，保留值
    //"id":"1",
    
    static uint32_t cnt_id = 0;
    mainmemb.SetInt(cnt_id/*(const char *)str_id, strlen(str_id)*/);
    document->AddMember("id", mainmemb, allocator);
    ++cnt_id;

    //2. version String 协议版本号，目前协议版本1.0
    // "version":"1.0.0",

    mainmemb.SetString((const char*)Devinfo.version,strlen(Devinfo.version));
    document->AddMember("version", mainmemb, allocator);

    //3 "business":"acc_car",
    mainmemb.SetString((const char*)Devinfo.business,strlen(Devinfo.business));
    document->AddMember("business", mainmemb, allocator);

    /*
     * 3. cid String 设备唯一标识
     * //"cid":"432944698949007788",
     * 
     */
    mainmemb.SetString((const char*)Devinfo.cid, strlen(Devinfo.cid));
    document->AddMember("cid", mainmemb, allocator);

    //4. type, String  设备类型 111(行李箱)|211(商场推车)|221(机场内推车)|222(机场外推车)|231(超市推车)|241(农贸市场推车)|311(婴儿推车) 
    //4. "type":"241",
    mainmemb.SetString((const char*)Devinfo.type, strlen(Devinfo.type));
    document->AddMember("type", mainmemb, allocator);
}
/**
 * @brief 
 * @param  document         My Param doc
 * @param  CarPara          My Param doc
 */
static void add_CarParaJson(Document *document,Value *CarPara)
{
   /*"params":{
       "user": 
       "xyz": 
       "car_info"
       "4g_info":
       "boxa":
       "boxb":
       "boxc":
       "r_info":
       "time"}
    */
    Document::AllocatorType& allocator = document->GetAllocator();
    Value mainmemb;

  
    //6. 所在区域的坐标
    //7. params Object 请求参数
    Value CarXYZ(kObjectType);
    CarXYZ.SetObject();
    add_CarXYZJson(document,&CarXYZ);
    CarPara->AddMember("xyz", CarXYZ, allocator);

    //车姿态
    Value CarInfo(kObjectType);
    CarInfo.SetObject();
    add_CarInfoJson(document,&CarInfo);
    CarPara->AddMember("car_info", CarInfo, allocator);


    //4G网络相关信息
    Value Car4GInfo(kObjectType);
    Car4GInfo.SetObject();
    add_Car4GInfoJson(document,&Car4GInfo);
    CarPara->AddMember("model_4g_info", Car4GInfo, allocator);

    //车姿态
    Value CarboxA(kObjectType);
    CarboxA.SetObject();
    add_CarboxAJson(document,&CarboxA);
    CarPara->AddMember("boxa", CarboxA, allocator);

    Value CarboxB(kObjectType);
    CarboxB.SetObject();
    add_CarboxBJson(document,&CarboxB);
    CarPara->AddMember("boxb", CarboxB, allocator);

    Value CarboxC(kObjectType);
    CarboxC.SetObject();
    add_CarboxCJson(document,&CarboxC);
    CarPara->AddMember("boxc", CarboxC, allocator);

    Value CarRInfo(kObjectType);
    CarRInfo.SetObject();
    add_CarRInfoJson(document,&CarRInfo);
    CarPara->AddMember("r_info", CarRInfo, allocator);

    add_TimeJson(document,&mainmemb);
    CarPara->AddMember("time", mainmemb, allocator);    

    //6. 正在使用的用户账号
    //"user":"15888888888",
    mainmemb.SetString("15888888888"); 
    CarPara->AddMember("user", mainmemb, allocator);

}

static void add_MethodJson(Document *document)
{
    Document::AllocatorType& allocator = document->GetAllocator();
    Value mainmemb;

    mainmemb.SetString("thing.event.property.post");
    document->AddMember("method", mainmemb, allocator);
}

static void add_UserJson()
{

}

static void add_CarXYZJson(Document *document,Value* CarXYZ)
{
    set_XYZinfo(&Xyzinfo);

    /* xyz":{"indoor":"0",
            "lon":"116.183097",
            "lat":"40.021531", 
            "floor":"1",
            "area":"A",
            "area-posx":"10",
            "area-posy":"10"},
    */
    Document::AllocatorType& allocator = document->GetAllocator();
    Value mainmemb;

    sprintf(strBuff[0],"%d", Xyzinfo.indoor);
    mainmemb.SetString((const char*)strBuff[0],strlen(strBuff[0])); 
    CarXYZ->AddMember("indoor", mainmemb, allocator);
    if (Xyzinfo.indoor==0)
    {
        memset(strBuff[3],0x00,sizeof(strBuff[3]));
        sprintf(strBuff[3],"%d", Xyzinfo.floor);
        mainmemb.SetString((const char*)strBuff[3],strlen(strBuff[3]));  
        CarXYZ->AddMember("floor", mainmemb, allocator);

        mainmemb.SetString((const char*)Xyzinfo.area,strlen(Xyzinfo.area)); 
        CarXYZ->AddMember("area", mainmemb, allocator);

        memset(strBuff[4],0x00,sizeof(strBuff[4]));
        sprintf(strBuff[4],"%0.2f", Xyzinfo.area_posx);
        mainmemb.SetString(strBuff[4],strlen(strBuff[4]));
        CarXYZ->AddMember("area_posx", mainmemb, allocator);

        sprintf(strBuff[5],"%0.2f",Xyzinfo.area_posy);
        mainmemb.SetString(strBuff[5],strlen(strBuff[5])); 
        CarXYZ->AddMember("area_posy", mainmemb, allocator);   
    }
    else
    {
        memset(strBuff[1],0x00,sizeof(strBuff[1]));
        sprintf(strBuff[1],"%0.8f", Xyzinfo.longitude);
        mainmemb.SetString((const char*)strBuff[1],strlen(strBuff[1])); 
        //mainmemb.SetString("116.183097"); 
        CarXYZ->AddMember("lon", mainmemb, allocator);

        sprintf(strBuff[2],"%0.8f", Xyzinfo.lattitude);
        mainmemb.SetString((const char*)strBuff[2],strlen(strBuff[2])); 
        //mainmemb.SetString("116.183097"); 
        CarXYZ->AddMember("lat", mainmemb, allocator);
    }
}

static void add_CarInfoJson(Document *document,Value* CarInfo)
{
    set_Carinfo(&Carinfo);

    /*"car_info":{
    "b_lock":"1",
    "ds":"0",
    "bat":"100",
    "km":"156",
    "can_km":"26",
    "user_km":"26",
    "soc":"0",
    "ui":"1.5",
    "uv":"24",
    "rc":"14",
    "pitch":"10",
    "yaw":"10",
    "roll":"1",
    "speed":"10"}, 
    */
    Document::AllocatorType& allocator = document->GetAllocator();
    Value mainmemb;

    sprintf(strBuff[6],"%d", Carinfo.b_lock);
    mainmemb.SetString(strBuff[6],strlen(strBuff[6]));
    CarInfo->AddMember("b_lock", mainmemb, allocator);

    sprintf(strBuff[7],"%d", Carinfo.ds);
    mainmemb.SetString(strBuff[7],strlen(strBuff[7]));
    CarInfo->AddMember("ds", mainmemb, allocator);

    sprintf(strBuff[8],"%d", (int)Carinfo.bat);
    mainmemb.SetString(strBuff[8],strlen(strBuff[8]));
    CarInfo->AddMember("bat", mainmemb, allocator);

    sprintf(strBuff[9],"%0.2f", Carinfo.km);
    mainmemb.SetString(strBuff[9],strlen(strBuff[9])); 
    CarInfo->AddMember("km", mainmemb, allocator);

    sprintf(strBuff[10],"%d", (int)Carinfo.can_km);
    mainmemb.SetString(strBuff[10],strlen(strBuff[10]));
    CarInfo->AddMember("can_km", mainmemb, allocator);

    sprintf(strBuff[11],"%d", (int)Carinfo.user_km);
    mainmemb.SetString(strBuff[11],strlen(strBuff[11]));
    CarInfo->AddMember("user_km", mainmemb, allocator);

    sprintf(strBuff[12],"%d", Carinfo.soc);
    mainmemb.SetString(strBuff[12],strlen(strBuff[12]));
    CarInfo->AddMember("soc", mainmemb, allocator);
////////////////////todo(xiaozhong) 添加 "ui":"1.5"/////////////////////////    
    
    sprintf(strBuff[13],"%0.3f", Carinfo.ui);
    mainmemb.SetString(strBuff[13],strlen(strBuff[13])); 
    CarInfo->AddMember("ui", mainmemb, allocator);

////////////////////todo(xiaozhong) 添加 "uv":"24"/////////////////////////    

    sprintf(strBuff[14],"%0.3f", Carinfo.uv);
    mainmemb.SetString(strBuff[14],strlen(strBuff[14]));
    CarInfo->AddMember("uv", mainmemb, allocator);    

    sprintf(strBuff[15],"%d", Carinfo.rc);
    mainmemb.SetString(strBuff[15],strlen(strBuff[15]));
    CarInfo->AddMember("rc", mainmemb, allocator);

    sprintf(strBuff[16],"%0.2f", Carinfo.pitch);
    mainmemb.SetString(strBuff[16],strlen(strBuff[16]));
    CarInfo->AddMember("pitch", mainmemb, allocator);

    sprintf(strBuff[17],"%0.2f", Carinfo.yaw);
    mainmemb.SetString(strBuff[17],strlen(strBuff[17])); 
    CarInfo->AddMember("yaw", mainmemb, allocator);

    sprintf(strBuff[18],"%0.2f", Carinfo.roll);
    mainmemb.SetString(strBuff[18],strlen(strBuff[18]));
    CarInfo->AddMember("roll", mainmemb, allocator);

    sprintf(strBuff[19],"%0.2f", Carinfo.speed);
    mainmemb.SetString(strBuff[19],strlen(strBuff[19]));
    CarInfo->AddMember("speed", mainmemb, allocator);
}


static void add_Car4GInfoJson(Document *document,Value* Car4GInfo)
{
    set_Lteinfo(&Lteinfo);

    /* "4g_info":{
        "iccid":"1321313103131",
        "imei":"213102",
        "sigquality":"25"},
    */
    Document::AllocatorType& allocator = document->GetAllocator();
    Value mainmemb;

    mainmemb.SetString((const char*)Lteinfo.iccid,strlen(Lteinfo.iccid)); 
    Car4GInfo->AddMember("iccid", mainmemb, allocator);

    mainmemb.SetString((const char*)Lteinfo.imei,strlen(Lteinfo.imei)); 
    Car4GInfo->AddMember("imei", mainmemb, allocator);

    sprintf(strBuff[20],"%d", Lteinfo.sigquality);
    mainmemb.SetString(strBuff[20],strlen(strBuff[20]));
    Car4GInfo->AddMember("sigquality", mainmemb, allocator); 
}

static void add_CarboxAJson(Document *document,Value* CarboxA)
{
    set_Boxa(&Boxainfo);

   /*"boxa":{
        "doora":"1",
        "alev":"21",
        "aval":"1",
        "astr":"0",
        "at":"10",
        "aoxy":"1"},
    */
    Document::AllocatorType& allocator = document->GetAllocator();
    Value mainmemb;

    sprintf(strBuff[21],"%d", Boxainfo.doora);
    mainmemb.SetString(strBuff[21],strlen(strBuff[21]));
    CarboxA->AddMember("doora", mainmemb, allocator);

    sprintf(strBuff[22],"%d", (int)Boxainfo.alev);
    mainmemb.SetString(strBuff[22],strlen(strBuff[22]));
    CarboxA->AddMember("alev", mainmemb, allocator);

    sprintf(strBuff[23],"%d", Boxainfo.aval);
    mainmemb.SetString(strBuff[23],strlen(strBuff[23]));
    CarboxA->AddMember("aval", mainmemb, allocator);

    sprintf(strBuff[24],"%d", Boxainfo.astr);
    mainmemb.SetString(strBuff[24],strlen(strBuff[24]));
    CarboxA->AddMember("astr", mainmemb, allocator);

    sprintf(strBuff[25],"%0.1f", Boxainfo.at);
    mainmemb.SetString(strBuff[25],strlen(strBuff[25])); 
    CarboxA->AddMember("at", mainmemb, allocator);

    sprintf(strBuff[26],"%d", Boxainfo.aoxy);
    mainmemb.SetString(strBuff[26],strlen(strBuff[26])); 
    CarboxA->AddMember("aoxy", mainmemb, allocator);
}


static void add_CarboxBJson(Document *document,Value* CarboxB)
{
    set_Boxb(&Boxbinfo);

    /*"boxb":
    {"doorb":"1",
    "bt":"1",
    "bref":"1"},
    */
    Document::AllocatorType& allocator = document->GetAllocator();
    Value mainmemb;

    sprintf(strBuff[27],"%d", Boxbinfo.doorb);
    mainmemb.SetString(strBuff[27],strlen(strBuff[27])); 
    CarboxB->AddMember("doorb", mainmemb, allocator);

    sprintf(strBuff[28],"%0.1f", Boxbinfo.bt);
    mainmemb.SetString(strBuff[28],strlen(strBuff[28]));
    CarboxB->AddMember("bt", mainmemb, allocator);

    sprintf(strBuff[29],"%d", Boxbinfo.bref);
    mainmemb.SetString(strBuff[29],strlen(strBuff[29]));
    CarboxB->AddMember("bref", mainmemb, allocator);
}


static void add_CarboxCJson(Document *document,Value* CarboxC)
{
    set_Boxc(&Boxcinfo);

    /*"boxc":{
        "doorc":"1"
        },
    */
    Document::AllocatorType& allocator = document->GetAllocator();
    Value mainmemb;

    sprintf(strBuff[30],"%d", Boxcinfo.doorc);
    mainmemb.SetString(strBuff[30],strlen(strBuff[30]));
    CarboxC->AddMember("doorc", mainmemb, allocator);    
}

static void add_CarRInfoJson(Document *document,Value* CarRInfo)
{
    set_Rinfo(&Rinfo);

    /*"r_info":{
        "rui":"1.2", 
        "ruv":"20",
        "rbat":"100"},
    */
    Document::AllocatorType& allocator = document->GetAllocator();
    Value mainmemb;

    sprintf(strBuff[31],"%0.3f", Rinfo.rui);
    mainmemb.SetString(strBuff[31],strlen(strBuff[31]));
    CarRInfo->AddMember("rui", mainmemb, allocator);

    sprintf(strBuff[32],"%0.3f", Rinfo.ruv);
    mainmemb.SetString(strBuff[32],strlen(strBuff[32]));
    CarRInfo->AddMember("ruv", mainmemb, allocator);

    sprintf(strBuff[33],"%d", (int)Rinfo.rbat);
    mainmemb.SetString(strBuff[33],strlen(strBuff[33]));
    CarRInfo->AddMember("rbat", mainmemb, allocator);
}

static void add_TimeJson(Document *document,Value* mainmemb)
{
    Document::AllocatorType& allocator = document->GetAllocator();
//    static char str_tim[50];
    static unsigned long timespor;  // 系统时间
    //timespor = get_ssecond();
    timespor = get_ms();
    sprintf(strBuff[34],"%ld", timespor);
    mainmemb->SetString((const char *)strBuff[34], strlen(strBuff[34]));
}

