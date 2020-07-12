#ifndef __BSP_BLE_H__
#define __BSP_BLE_H__
#include "config.h"

typedef struct
{
    CAR_MODE_t car_mode;
    float  speed;
    long index_goal;
    double lat;
    double lon;
}user_order_t;
typedef struct
{
    CAR_MODE_t car_mode;
    float ble_angle;
    float ble_speed;
    long  ble_goal;
    double lat;
    double lon;
}ble_t;
typedef struct
{
    CAR_MODE_t car_mode;
    float speed;
    float angle;
    int index_goal;
    double lat;
    double lon;
}server_order_t;

int handle_ble(unsigned char *ucData,int usLength);
//int uwb_send_ble(int fd_x,const int angle,const int speed);
//蓝牙发送tag导航相关数据
int tag_send_ble(PORT_NAME name,const int angle,const int speed);
//蓝牙发送gps导航相关数据
int gps_send_ble(PORT_NAME name,const int angle,const int speed);


#endif



