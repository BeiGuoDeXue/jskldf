/**
 * @file message.h
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
#ifndef MESSAGE_H
#define MESSAGE_H
#include <stdint.h>
#include <string>
#include "MQTTAsync.h"
#include "topic.h"
#include "rapidjson/document.h"
#include "config.h"

using namespace std;



#define MSGMAXNUM 15

// CMD list
// 协议相关
#define PROTOCOL_HEAD 0xfe
#define PROTOCOL_END 0xed

// error
#define ERROR_HEADER -1
#define ERROR_CRC8 -2
#define ERROR_LEN -3

// device message type
#define DEVICE_CODE 0x01        // 设备代码
#define VERDOR_CODE 0x02        // 厂商代码
#define PRODUCT_BATCH_NUM 0x03  // 生产批号
#define SOFTWARE_VERSION 0x05   // 软件版本
#define WORK_MODE 0x06          // 工作模式
#define DEVICE_ADDR 0x07        // 设备地址
#define TIME_NOW 0x08           // 实时时间
#define ACCUMULATE_FLOW 0x0c    // 累计流量
#define INSTANTANEOUS_FLOW 0x0d // 瞬时流量 Instantaneous

const uint8_t cCrcTable[256] =
{
    0x00, 0x5e, 0xbc, 0xe2, 0x61, 0x3f, 0xdd, 0x83,
    0xc2, 0x9c, 0x7e, 0x20, 0xa3, 0xfd, 0x1f, 0x41,
    0x9d, 0xc3, 0x21, 0x7f, 0xfc, 0xa2, 0x40, 0x1e,
    0x5f, 0x01, 0xe3, 0xbd, 0x3e, 0x60, 0x82, 0xdc,
    0x23, 0x7d, 0x9f, 0xc1, 0x42, 0x1c, 0xfe, 0xa0,
    0xe1, 0xbf, 0x5d, 0x03, 0x80, 0xde, 0x3c, 0x62,
    0xbe, 0xe0, 0x02, 0x5c, 0xdf, 0x81, 0x63, 0x3d,
    0x7c, 0x22, 0xc0, 0x9e, 0x1d, 0x43, 0xa1, 0xff,
    0x46, 0x18, 0xfa, 0xa4, 0x27, 0x79, 0x9b, 0xc5,
    0x84, 0xda, 0x38, 0x66, 0xe5, 0xbb, 0x59, 0x07,
    0xdb, 0x85, 0x67, 0x39, 0xba, 0xe4, 0x06, 0x58,
    0x19, 0x47, 0xa5, 0xfb, 0x78, 0x26, 0xc4, 0x9a,
    0x65, 0x3b, 0xd9, 0x87, 0x04, 0x5a, 0xb8, 0xe6,
    0xa7, 0xf9, 0x1b, 0x45, 0xc6, 0x98, 0x7a, 0x24,
    0xf8, 0xa6, 0x44, 0x1a, 0x99, 0xc7, 0x25, 0x7b,
    0x3a, 0x64, 0x86, 0xd8, 0x5b, 0x05, 0xe7, 0xb9,
    0x8c, 0xd2, 0x30, 0x6e, 0xed, 0xb3, 0x51, 0x0f,
    0x4e, 0x10, 0xf2, 0xac, 0x2f, 0x71, 0x93, 0xcd,
    0x11, 0x4f, 0xad, 0xf3, 0x70, 0x2e, 0xcc, 0x92,
    0xd3, 0x8d, 0x6f, 0x31, 0xb2, 0xec, 0x0e, 0x50,
    0xaf, 0xf1, 0x13, 0x4d, 0xce, 0x90, 0x72, 0x2c,
    0x6d, 0x33, 0xd1, 0x8f, 0x0c, 0x52, 0xb0, 0xee,
    0x32, 0x6c, 0x8e, 0xd0, 0x53, 0x0d, 0xef, 0xb1,
    0xf0, 0xae, 0x4c, 0x12, 0x91, 0xcf, 0x2d, 0x73,
    0xca, 0x94, 0x76, 0x28, 0xab, 0xf5, 0x17, 0x49,
    0x08, 0x56, 0xb4, 0xea, 0x69, 0x37, 0xd5, 0x8b,
    0x57, 0x09, 0xeb, 0xb5, 0x36, 0x68, 0x8a, 0xd4,
    0x95, 0xcb, 0x29, 0x77, 0xf4, 0xaa, 0x48, 0x16,
    0xe9, 0xb7, 0x55, 0x0b, 0x88, 0xd6, 0x34, 0x6a,
    0x2b, 0x75, 0x97, 0xc9, 0x4a, 0x14, 0xf6, 0xa8,
    0x74, 0x2a, 0xc8, 0x96, 0x15, 0x4b, 0xa9, 0xf7,
    0xb6, 0xe8, 0x0a, 0x54, 0xd7, 0x89, 0x6b, 0x35,
};




/*
 *上报的数据中的数据单元的参数类型码
 *
 * refr: 
 * enum DEVLIST, 进行区分
 * struct _MSG_DATA, 它的成员
 * DevToServiceDataCreat()
 */
typedef enum
{
    user_code = 1,       //正在使用的用户账号
    Voltage = 2,         // 电压, vol|uint
    Standardization = 3, // 出厂标定, 0: 未曾出厂标定; 1: 标定OK; 2: 标定失败, std|uint
    Battery_stat = 4,    // 电池状态, 0: 电池电量正常(比如: 7000mv); 1: 电量过低(比如: 5000mv); 2: 预警(比如: 6500mv)，单位: mv, bs|uint
    Battery_level = 5,   // 电池电量，单位毫伏, bt|uint
    Open_angle = 6,      // 开门角度, ag|int
    Longitude = 7,       // 经度, gel(东经)|gwl(西经)|float
    Latitude = 8         // 纬度, gnl(北纬)|gsl(南纬)|float
} PMTLIST;

/*
 * MQTT 消息的类型
 */
typedef enum {
	ACK = 0,
	ACQUIRE,
	REQUEST,
	CONTROL_SW,				// 控制命令, 将发送至设备
	SET_BIND_PARAM,			// 设置绑定参数, 将发送至设备
	SET_THRESHOLD,			// 设置子设备预警参数, 将发送至设备
	SET_SYS_PARAM,			// 设置系统参数: 版本号，服务器域名，将存储在 SERVER_DNS/GATE_VERSION 数据库中
	DEV_BIND_QUERY,			// 查询绑定信息
	SYS_PARAM_QUERY,		// 查询系统参数：版本号，服务器域名
	DEV_UPGRADE,			// 设备升级命令，将发送至设备
	SYS_MACHINE_SELFCHECK,	// 设备自检命令
    REMOTE_CONTROL_MOVE
} MQTT_MSG_TYPE;

/*
 * 设备携带的消息，即数据单元
 * struct Device_Info:pload 中可以包含多个 struct MSG_DATA
 *
 * refr: 
 * struct Device_Info, 它的 pload
 * struct CTL_DEVICE, 它的成员
 * load_subset_data(), 获取
 * DevToServiceDataCreat(), 解析
 */
typedef struct _MSG_DATA
{
    // unsigned char devtype;
    PMTLIST devtype;

    unsigned char devlen;

    unsigned char value[64];
} MSG_DATA;

typedef struct {
	/*
	 * 可以表示子设备类型
	 */
    char type;
    char address[8];
    char value[1950];
    long int time;
} DEV_PROPERTY;


/*
 * 设备的状态
 *
 * refr: 
 * struct _CTL_DEVICE, 它的成员
 */
typedef enum
{
    Normal,
    Fault,
    Needack,
    CtlMsg,
} DEVSTA;

/*
 * MQTT 消息结构
 *
 * refr: 
 * msgarrvd(), 接收到 MQTT 消息时的回调函数
 */
#define PROPERTY_NUM 10

typedef struct 
{
    CAR_MODE_t car_mode;
    float disX;
    float disY;
    float  speed;
    long index_goal;
    double lat;
    double lon;
    float tag_x;
    float tag_y;
    double time;
}mqtt_user_info_t;

typedef struct _MQTTMSG {
	string id;			// 将保存至 net_id
	int categoryId;
	string productId;	// 子设备编码地址
	string version;	// 版本号，将保存到数据库 GATE_VERSION
	int version_type;
	string params;
	string method;
	string dstsid;
	string url;
	string name;
	string dir;
	string dns;		// 域名，将保存到数据库 SERVER_DNS
	int code;
	string data;
	int topic;
	MQTT_MSG_TYPE msgtype;
	int propertyNum;
	DEV_PROPERTY property[PROPERTY_NUM];
}MQTTMSG;

/*
 * 设备类型列表
 * 协议文档中称为设备地址
 *
 * refr: 
 * enum PMTLIST, 进行区分
 * struct _CTL_DEVICE, 它的成员
 * struct Device_Info_Head::dev_type, 为它赋值
 */
typedef enum
{

} DEVLIST;




/*
 * 推车上传的消息
 *
 * 将由 load_subset_data()转换为 struct CTL_DEVICE
 *
 * refr: 
 * struct Device_Info, 推车实际上报的数据格式
 * load_subset_data(), 将 UART1 推车收到的数据'device_info'解析为 struct CTL_DEVICE, 保存至 ctledDevice[0]
 * DevToServiceDataCreat(), 对 struct CTL_DEVICE 通过 rapidjson 进行整合，以便能传递给后台
 */
typedef struct _CTL_DEVICE
{
    DEVLIST devtype; // 设备类型

    DEVSTA devStatus; // 设备状态

    unsigned char addr[8]; // 设备的物理编码地址

    /*
     * 设备上报的命令类型码
     *
     * refr: 
     * #define Updata_cmd
     */
    unsigned char cmd;

    unsigned char msgNum; // 设备携带消息总量

    MSG_DATA msg[MSGMAXNUM]; // 设备携带的消息

    unsigned char msgFlag; // MQTT 处理消息依据

    int socketFd; // tcp 处理文件

    unsigned char isAlive; // 存储时间片资源，时间片消耗尽释放资源，心跳包收到后更新
} CTL_DEVICE;


/////////////////////////回复的消息/////////////////////////
// #define ... add more too.
typedef struct {
	uint64_t id;
	int     code;
	char *  data;
} zhsAckMsg;

typedef struct {
	char topic[100];
	zhsAckMsg msgAck; 
	bool ackSuccessFlag;
	bool msgHanded;
	// bool
	/*
	 * 每一个是否有消息需要发送，
	 */
} zhsTopicMsg[TOPICNUM];

typedef struct _MSG_Send {
	int topicNum;
	int sendWaitTime;
	int resendTime;
	bool successFlag;
} mqttMsgsed;

/********************************************上报消息**************************************/
typedef struct
{
    uint32_t str_id;//id String 消息ID号
    char version[20];//version String 协议版本号，目前协议版本1.0
    char business[20]; 
    char cid[20];  //cid String 设备唯一标识
    char type[20];      //设备类型
}device_info;

typedef struct 
{
    bool indoor; //表示定位方式( 0室内/1室外
    double longitude; //经度
    double lattitude; //纬度
    int floor; //所在楼层（室内）
    char area[10]; //所在区域（室内）
    double area_posx; //所在区域内X方向的坐标
    double area_posy; //所在区域内Y方向的坐标
}xyz_info; //定位信息，参照属性

typedef struct
{
    bool b_lock; //手环状态(0未使用/1已使用)
    CAR_MODE_t ds; //行驶状态（0自动跟随|1手动控制|2送货模式|3载人模式|）（预留）
    double bat; //伴置车电池容量（百分比）
    double km; //伴置车里程数,单位km（累计）
    double can_km; //伴置车还可以行驶里程数，单位km
    double user_km; //伴置车用户行驶里程数，单位km
    bool soc;//伴置车充电状态（0未充电|1充电中）
    double ui; //车电流（单位A）
    double uv; //车电压（单位V）
    int rc; //伴置车充电次数
    double pitch;//车辆俯仰角
    double yaw;//车厢航向角
    double roll; //车厢航向角
    double speed; //车辆行驶速度（单位m/s）
}car_info; //车辆状态信息


typedef struct
{
    char iccid[25];//SIM卡号，由20位数字构成
    char imei[20];//4G模组设备身份证编号，由15位数字构成
    int sigquality; //网络信息质量,范围0-31之间，数组越大表明信号质量越好。
    char rev[];
}lte_info_mqtt; //4G模组相关信息

typedef struct 
{
    bool doora; //仓门状态（0开|1关）
    double alev; //水箱水位
    bool aval; //水箱放水阀门状态（0开|1关）
    bool astr; //水箱滤网状态（0平放|1提升）
    double at; //温度，单位℃
    bool aoxy; //充氧状态（0未充氧|1充氧）
}boxa_info;//水箱信息

typedef struct
{
    bool doorb;
    double bt; //冰箱温度，单位℃
    bool bref; //冰箱制冷状态（0未制冷|1制冷）
}boxb_info; //冰箱信息

typedef struct
{
    bool doorc; //仓门状态（0开|1关）
}boxc_info;//顶棚信息（最高层）

typedef struct 
{
    double rui; //冰箱电池电流，单位A
    double ruv; //冰箱电池电压，单位V
    double rbat; //冰箱电池剩余电量
}r_info;//冰箱电池属性



#ifdef __cplusplus
extern "C"
{
#endif

void ZhsMQTTAsync_sendMessage(MQTTAsync *client, MQTTAsync_message *pubmsg, 
     int ackTopic, MQTTAsync_responseOptions *opts);
void PackMQTTMsg(char *loadbuf, void *context, char cmd, 
    MQTTAsync_message *pubmsg, MQTTAsync_responseOptions *opts, 
    MQTTAsync_onSuccess on_success);
/* 发送推车心跳包 */
void ConcentratorHearting(MQTTAsync *client, uint32_t *timestamp);
int msgarrvd(void *context, char *topicName, int topicLen, MQTTAsync_message *message);
MQTTMSG ServiceDataAnalysis(const char *buf, int topicNum);
void AttrDataPack(rapidjson::Document *document,CTL_DEVICE dev);
#ifdef __cplusplus
}
#endif

#endif //MESSAGE_H