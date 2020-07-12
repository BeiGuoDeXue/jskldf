#include <stdio.h>
#include <sys/time.h>
#include <unistd.h>
#include "config.h"
#include "app.h"
#include "bsp_car_drive.h"
#include "bsp_ble.h"
#include "ble_remote.h"

extern control_info_t control_info;
extern ble_t ble_info;
void ble_remote(void)
{
    //下发速度和角度命令
    control_info.angle = ble_info.ble_angle;
    control_info.speed = ble_info.ble_speed;
    remote_control(CARDRIVE,control_info.angle,control_info.speed);
    sleep(1);
}