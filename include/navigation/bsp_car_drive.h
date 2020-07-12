#ifndef __BSP_CAR_DRIVE_H__
#define __BSP_CAR_DRIVE_H__
#include "config.h"

typedef struct 
{
    int wheel_L;
    int wheel_R;
    int wheel_differ_L;
    int wheel_differ_R;
    unsigned short wheel_time_L;
    unsigned short wheel_time_R;
    unsigned short ultrasonic[6];
    unsigned short battery_Vdd;
    unsigned short battery_AH;
    short car_angle;
    float speed_wheel_L;
    float speed_wheel_R;
}cardrive_info_t;



int handle_stm32(unsigned char *ucData,int usLength);
int select_mode(int fd_x,char car_mode);
int remote_control(PORT_NAME name,const float angle,const float speed);

#endif



