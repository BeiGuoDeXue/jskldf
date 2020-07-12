#include <stdio.h>
#include <sys/time.h>
#include "gps_navigation.h"
#include "config.h"
#include "app.h"
#include "bsp_car_drive.h"
#include "bsp_ble.h"
#include "bsp_gps.h"
#include "obstacle.h"
#include "bsp_hps3d.h"
//接收到数据标志位
extern int gps_flag;
//目标点位置
extern long GPS_LOG_MAXNUM;
//STM32信息
extern cardrive_info_t cardrive_info;
extern control_info_t  control_info;
//蓝牙信息
extern ble_t ble_info;
//gps定位信息
extern gps_info_t gps_info;
extern gpslog_addr_t gpslog_addr[];
//车的姿态信息
extern carpos_t     car_pos;
extern obs_hps_t    obs_hps;
extern int obs_flaging;
int obs_flag = 0;
void gps_navigation(void)
{
    struct timeval tv3;
    static int first_pos_addr = 1;
    if (gps_flag)
    {
        gps_flag = 0;
        if (first_pos_addr)
        {
            first_pos_addr = 0;
            //第一次开始定位自己的位置
            gps_info.index.self_index = gps_start_positin(&gps_info,gpslog_addr);
            printf("\r\n开始导航!\r\n");
        }
        if (gps_info.index.goal_index>=GPS_LOG_MAXNUM-GPS_GOAL_ARRAY_DIFF)
        {
            printf("到达目的\r\n");
            remote_control(CARDRIVE,0,0);
        }
        else
        {
            //找到自己和目标的位置
            gps_traversal_array(&gps_info,gpslog_addr);
            if(obs_flaging==0)
            {
                //计算下发的速度和角度
                gps_count_control(&gps_info,gpslog_addr,&control_info);
                cardrive_info.speed_wheel_L = Single_Dis*1000*cardrive_info.wheel_differ_L/cardrive_info.wheel_time_L;
                cardrive_info.speed_wheel_R = Single_Dis*1000*cardrive_info.wheel_differ_R/cardrive_info.wheel_time_R;
                gps_send_ble(BLE,cardrive_info.car_angle*10,(cardrive_info.speed_wheel_L+cardrive_info.speed_wheel_R)*100/2.0f);
                //下发速度和角度命令
                if (obs_hps.obs_hps_flag==HPS_WARNING_FLAG)
                    control_info.speed = OBS_SPEED;
                remote_control(CARDRIVE,control_info.angle,control_info.speed);
            }
        }
    }
    if (obs_flag>0)
    {
        int obs_flag1 = obs_flag;
        obs_flag = 0;
        obs_control(obs_flag1);
    }
}



