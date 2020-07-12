#include <stdio.h>
#include <sys/time.h>
#include <unistd.h>
#include <math.h>
#include "config.h"
#include "server_remote.h"
#include "message.h"
#include "bsp_tag.h"
#include "app.h"
#include "bsp_car_drive.h"
#include "bsp_tag_xy.h"
#include "bsp_imu.h"
#include "bsp_gps.h"
#define SERVER_DIS_RANGE       20          //单位是cm

extern control_info_t control_info;
extern carpos_t car_pos;
extern uwb_info_t uwb_info;
extern mqtt_user_info_t mqtt_user_info;
extern int demarcate_flag;
extern double demarcate_angle;
extern gps_info_t gps_info;
extern _imu_angle imu_angle;
extern int tag_flag;
extern int gps_flag;
int server_flag = 0;
CAR_MODE_t server_origin_mode;
// static void tag_server_goal_count(mqtt_user_info_t *p_userinfo,uwb_info_t *p_uwbinfo);
static void tag_server_goal_count(mqtt_user_info_t *p_userinfo,uwb_info_t *p_uwbinfo,carpos_t *p_carpos);
static void server_control(control_info_t *p_control_info,mqtt_user_info_t *p_userinfo,uwb_info_t *p_uwbinfo,const carpos_t *p_carpos);
/**
 * @brief
 * 服务器远程控制，到达某个目标点
 */
void server_remote(void)
{
    if(server_flag==1)
    {
        if (tag_flag>0)
        {
            int uwb_flag_num = tag_flag;
            tag_flag = 0;
            if (demarcate_flag == 0)
            {
                uwb_count_carpos(&uwb_info,&car_pos,&imu_angle);
                demarcate_angle = uwb_init_demarcate(&uwb_info,&car_pos,500);   //标定500cm
            }
            else if(demarcate_flag == 1)
            {
                //计算车的姿态
                uwb_count_carpos(&uwb_info,&car_pos,&imu_angle);
                //把坐标转换为中心点
                uwb_center_addr_count(&uwb_info,uwb_flag_num,&car_pos);
                //tag_server_goal_count(&mqtt_user_info,&uwb_info);
                server_control(&control_info,&mqtt_user_info,&uwb_info,&car_pos);
                //下发速度和角度命令
                remote_control(CARDRIVE,control_info.angle,control_info.speed);
            }
        }
        if (gps_flag==1)
        {
            gps_flag = 0;
            gps_count_carpos(&car_pos,&gps_info);
        }
    }
}
/**
 * @brief 根据服务器下发的距离算出目标点
 * @param  p_userinfo       My Param doc
 * @param  p_uwbinfo        My Param doc
 */
void tag_server_goal_count(mqtt_user_info_t *p_userinfo,uwb_info_t *p_uwbinfo,carpos_t *p_carpos)
{
    if (server_origin_mode == TAG_NAVIGATION)
    {
        float server_angle = atan2(p_userinfo->disX,p_userinfo->disY);
        float goal_angle = server_angle + p_carpos->yaw;
        if (goal_angle>=360)
        {
            goal_angle = 360 - goal_angle;
        }else if (goal_angle<0)
        {
            goal_angle = 360 + goal_angle;
        }
        float server_dis = sqrt(pow(p_userinfo->disX,2) + pow(p_userinfo->disY,2));
        if (goal_angle>=0&&goal_angle<=90)
        {
            p_userinfo->tag_x = p_uwbinfo->center.x + server_dis * cos(goal_angle);
            p_userinfo->tag_y = p_uwbinfo->center.y - server_dis * sin(goal_angle);
        }
        else if (goal_angle>90&&goal_angle<=180)
        {
            p_userinfo->tag_x = p_uwbinfo->center.x - server_dis * cos(180 - goal_angle);
            p_userinfo->tag_y = p_uwbinfo->center.y - server_dis * sin(180 - goal_angle);
        }
        else if (goal_angle>180&&goal_angle<=270)
        {
            p_userinfo->tag_x = p_uwbinfo->center.x - server_dis * cos(goal_angle - 180);
            p_userinfo->tag_y = p_uwbinfo->center.y + server_dis * sin(goal_angle - 180);
        }
        else if (goal_angle>270&&goal_angle<360)
        {
            p_userinfo->tag_x = p_uwbinfo->center.x + server_dis * cos(360 - goal_angle);
            p_userinfo->tag_y = p_uwbinfo->center.y + server_dis * sin(360 - goal_angle);
        }
    }
}
void gps_server_goal_count(mqtt_user_info_t *p_userinfo,gps_info_t *p_gpsinfo,carpos_t *p_carpos)
{
    if (server_origin_mode == GPS_NAVIGATION)
    {
        float server_angle = atan2(p_userinfo->disX,p_userinfo->disY);
        float goal_angle = server_angle + p_carpos->yaw;
        if (goal_angle>=360)
        {
            goal_angle = 360 - goal_angle;
        }else if (goal_angle<0)
        {
            goal_angle = 360 + goal_angle;
        }
        float server_dis = sqrt(pow(p_userinfo->disX,2) + pow(p_userinfo->disY,2));
        if (goal_angle>=0&&goal_angle<=90)
        {
            p_userinfo->tag_x = p_gpsinfo->addr.Lattitude + server_dis * cos(goal_angle);
            p_userinfo->tag_y = p_gpsinfo->addr.Longitude - server_dis * sin(goal_angle);
        }
        else if (goal_angle>90&&goal_angle<=180)
        {
            p_userinfo->tag_x = p_gpsinfo->addr.Lattitude - server_dis * cos(180 - goal_angle);
            p_userinfo->tag_y = p_gpsinfo->addr.Longitude - server_dis * sin(180 - goal_angle);
        }
        else if (goal_angle>180&&goal_angle<=270)
        {
            p_userinfo->tag_x = p_gpsinfo->addr.Lattitude - server_dis * cos(goal_angle - 180);
            p_userinfo->tag_y = p_gpsinfo->addr.Longitude + server_dis * sin(goal_angle - 180);
        }
        else if (goal_angle>270&&goal_angle<360)
        {
            p_userinfo->tag_x = p_gpsinfo->addr.Lattitude + server_dis * cos(360 - goal_angle);
            p_userinfo->tag_y = p_gpsinfo->addr.Longitude + server_dis * sin(360 - goal_angle);
        }
    }
}
/**
 * @brief 根据计算的目标点来算速度和转向角度
 * @param  p_control_info   My Param doc
 * @param  p_userinfo       My Param doc
 * @param  p_uwbinfo        My Param doc
 * @param  p_carpos         My Param doc
 */
void server_control(control_info_t *p_control_info,mqtt_user_info_t *p_userinfo,uwb_info_t *p_uwbinfo,const carpos_t *p_carpos)
{
    double self_angle = 0;
    double goal_angle = 0;
    double angle_diff = 0;
    goal_angle = tag_getAngle(p_uwbinfo->center.x,p_uwbinfo->center.y, p_userinfo->tag_x, p_userinfo->tag_y);
    self_angle = p_carpos->yaw;
    angle_diff = goal_angle - self_angle;  //大于0实际需要右转
    //由于偏差角度大于180,所以要朝另外一个方向转，即朝左，朝左值要为负
    if (angle_diff>180)  angle_diff = -360 + angle_diff;
    if (angle_diff<-180) angle_diff = 360 + angle_diff;
    if (angle_diff<-TAG_CAR_ANGLE)  angle_diff = -TAG_CAR_ANGLE;
    if (angle_diff>TAG_CAR_ANGLE)   angle_diff = TAG_CAR_ANGLE;
    p_control_info->angle = -angle_diff;     //此处角度差增大时应该是右转，右转角度要减小，所以为负
    printf("server_goal_angle: %f,server_self_angle: %f,server_angle_diff: %f\n",goal_angle,self_angle,-angle_diff);
    //printf("angle_diff: %f\n",-angle_diff);
    p_control_info->dis = tag_get_distance(p_uwbinfo->center.x,p_uwbinfo->center.y,p_userinfo->tag_x, p_userinfo->tag_y);
    p_control_info->speed = TAG_CAR_SPEED;
    if (p_control_info->dis < SERVER_DIS_RANGE)
    {
        printf("到达server_angle,距离: %f\n",p_control_info->dis);
        p_control_info->speed = 0;
        p_control_info->angle = 0;
    }
}






