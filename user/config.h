#ifndef __CONFIG_H__
#define __CONFIG_H__

/*****************************小车底盘参数*****************************/
#define PI                              3.1415926
#define Radius                          0.0762f         //轮子半径
#define Wheel_Num                       90.0f           //一圈多少个脉冲
#define Roller                          0.22f           //后轮间距
#define car_length                      0.246f          //后轮到前轮距离
#define Single_Dis                      0.00532f        //单个脉冲走的长度
//车轮左转下发正值
/*****************************车速，最大转弯角度选择*****************************/
#define GPS_CAR_SPEED                   1               //GPS导航速度
#define TAG_CAR_SPEED                   0//.5f
#define GPS_CAR_ANGLE                   30
#define TAG_CAR_ANGLE                   20
#define TAG_DEM_SPEED                   0//.5f          //标签标定时速度
#define OBS_SPEED                       0.3f            //避障时速度
#define TURN_PROPORTION                 0.5f            //转弯减速比例
#define OBS_BACK_SPEED                  0.2f
/*****************************地球半径*****************************/
//#define                               EARH_R 6371.004 
#define EARH_R                          6378.137f       //地球半径
/*****************************两个UWB到车的中心的距离*****************************/
#define CENTER_BACK                     0.3f            //车中心到后面UWB的距离,单位为m
#define CENTER_FRONT                    0.3f            //车中心到前面UWB的距离,单位为 
/*****************************导航时目标点的设置距离*****************************/
#define CONST_GOAL_DIS                  100             //和目标点的固定距离，单位cm
/*****************************遍历自己位置时的遍历范围*****************************/
#define TAG_TRAVER_FRONT                40              //UWB向前遍历数组个数
#define TAG_TRAVER_BACK                 60              //UWB向后遍历数组个
#define GPS_TRAVER_FRONT                100             //UWB向前遍历数组个数
#define GPS_TRAVER_BACK                 200             //UWB向后遍历数组个
/*****************************导航时到达目标点的容忍度*****************************/
#define UWB_GOAL_ARRAY_DIFF             10              //距离目标差20个数组
#define GPS_GOAL_ARRAY_DIFF             100             //距离目标差20个数组
/*****************************定时器中断触发时间*****************************/
#define TIMER_INTERRUPT_NUM             2               //单位是s
/*****************************状态值**************************************/
typedef enum 
{
    FALSE = -1,
    SUCCESS = 1,
    INIT = 0
}STAT;
/*****************************在数组中的索引*****************************/
typedef struct
{
    int self_index;
    int goal_index;
}index_t;
/*****************************车模式选择*****************************/
typedef enum
{
    STOP_MODE = 0,                                      //停止模式
    BLE_REMOTE = 1,                                     //BLE遥控
    SERVER_REMOTE = 2,                                  //后台服务器遥控
    TAG_NAVIGATION = 3,                                 //UWB导航
    GPS_NAVIGATION =4,                                  //GPS导航
    FELLOW_MODE = 5,                                    //跟随模式
    ACCOMPANY_MODE = 6                                  //伴行模式
}CAR_MODE_t;
/*****************************TAG位置索引*****************************/
typedef enum 
{
    UWB_BACK,
    UWB_FRONT
}UWB_POS;
/*****************************串口名字*****************************/
typedef enum
{
    TAG1 = 0,                               \
    TAG2,                                   \
    GPS,                                    \
    IMU,                                    \
    CARDRIVE,                               \
    BLE,                                    \
    SINGLE_LIDAR1,                          \
    SINGLE_LIDAR2,                          \
    MULT_LIDAR,                             \
    HPS_3D160,                              \
    FRESH_DRIVE,                            \
    UWB1,                                   \
    UWB2,                                   \
    CAMERA1,                                \
    CAMERA2,                                \
    AUTO_CHARGE,                            \
    EC20_0,                                 \
    EC20_1,                                 \
    EC20_2,                                 \
    EC20_3,                                 \
    MAX_PORT
}PORT_NAME;

#define FD(p)       (0x00000001<<(p))
#define PORT_NUM    (FD(IMU)|FD(BLE)|FD(EC20_2))
#define HPS3D_START 0    //打开HPS3D_160
//#define  PORT_NUM   (FD(EC20_2)|FD(CARDRIVE))
int bsp_write_array(PORT_NAME port_num,unsigned char *array,int lens);

extern unsigned long get_ms(void);

#endif


