#include <stdio.h>
#include <sys/time.h>
#include "tag_navigation.h"
#include "config.h"
#include "app.h"
#include "bsp_car_drive.h"
#include "bsp_ble.h"
#include "bsp_tag.h"
extern control_info_t control_info;
//蓝牙信息
extern ble_t ble_info;
//uwb定位信息
extern uwb_info_t uwb_info;
extern uwblog_addr_t uwblog_addr[];
extern int tag_flag;
//标定标志位
extern int demarcate_flag;
double demarcate_angle = 0;
//IMU数据
_imu_angle imu_angle;
//车的姿态信息
carpos_t car_pos;
extern long UWB_LOG_MAXNUM;
//车子状态信息
extern cardrive_info_t cardrive_info;
void tag_navigation(void)
{
    static int first_pos_addr = 1;
    //标定
    if (tag_flag>0)
    {
        int uwb_flag_num = tag_flag;
        tag_flag = 0;
        //tag_send_ble(TAG1,0,0);
        if (demarcate_flag == 0)
        {
            uwb_count_carpos(&uwb_info,&car_pos,&imu_angle);
            demarcate_angle = uwb_init_demarcate(&uwb_info,&car_pos,500);   //标定500cm
        }
        else if(demarcate_flag == 1)
        {
            if (first_pos_addr)
            {
                //计算车的姿态
                uwb_count_carpos(&uwb_info,&car_pos,&imu_angle);
                //把坐标转换为中心点
                uwb_center_addr_count(&uwb_info,uwb_flag_num,&car_pos);
                //第一次开始定位自己的位置
                uwb_info.index.self_index = uwb_start_positin(&uwb_info,uwblog_addr);
                printf("\r\n开始导航!\r\n");
                first_pos_addr=0;
            }
            else
            {
                if (uwb_info.index.self_index>=UWB_LOG_MAXNUM-UWB_GOAL_ARRAY_DIFF)
                {
                    printf("到达目的\n");
                    remote_control(CARDRIVE,0,0);
                }
                else
                {
                    //计算车的姿态
                    uwb_count_carpos(&uwb_info,&car_pos,&imu_angle);
                    //把坐标转换为中心点
                    uwb_center_addr_count(&uwb_info,uwb_flag_num,&car_pos);
                    
                    //找到自己和目标的位置
                    uwb_traversal_array(&uwb_info,uwblog_addr);
                    //计算下发的速度和角度
                    uwb_count_control(&uwb_info,uwblog_addr,&control_info,&car_pos);
                    cardrive_info.speed_wheel_L = Single_Dis*1000*cardrive_info.wheel_differ_L/cardrive_info.wheel_time_L;
                    cardrive_info.speed_wheel_R = Single_Dis*1000*cardrive_info.wheel_differ_R/cardrive_info.wheel_time_R;
                    car_pos.speed = (cardrive_info.speed_wheel_L + cardrive_info.speed_wheel_R)/2.0f;
                    //tag_send_ble(BLE,0,0);
                    //下发速度和角度命令
                    remote_control(CARDRIVE,control_info.angle,control_info.speed);
                }
            }
        }
    }
}


