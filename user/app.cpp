/**
 * @file app.cpp
 * @brief 
 * @author zhaokangxu (zhaokangxu@zhskg.cn)
 * @version 1.0
 * @date 2020-06-11
 * 
 * @copyright Copyright (c) 2020-2024  智慧式集团
 * 
 * @par 修改日志:
 * <table>
 * <tr><th>Date       <th>Version <th>Author  <th>Description
 * <tr><td>2020-06-11 <td>1.0     <td>zhaokangxu     <td>内容
 * </table>
 */

#include <stdio.h>
#include <math.h>
#include <string.h>
#include "app.h"
#include "bsp_lat_lon.h"
#include "bsp_tag_xy.h"
#include "bsp_tag.h"
#include "bsp_filter.h"
#include "config.h"
#include "bsp_car_drive.h"

control_info_t control_info;
//读取路径长度
long UWB_LOG_MAXNUM = 0;
long GPS_LOG_MAXNUM = 0;

int demarcate_flag = 0;
double demarcate_imu = 0;
extern double demarcate_angle;
extern double look_angle,look_filter_angle;
//****************************UWB********************************************//
void uwb_count_carpos(const uwb_info_t *p_uwb,carpos_t *p_carpos,_imu_angle *p_imu_angle)
{
    double carpos_angle = 0,imu_ang = 0;
    //计算两个UWB得到的角度,车在标定之前可以使用这里判断车的姿态，暂时不开发
    // angle = tag_getAngle(p_uwb->addr[0].x,p_uwb->addr[0].y, p_uwb->addr[1].x,p_uwb->addr[1].y);
    // angle = uwb_filter_buff(angle);   //采用中值滤波看效果
    get_imu_angle(p_imu_angle);
    if (p_imu_angle->yaw < demarcate_imu)
    {
        imu_ang = p_imu_angle->yaw+360;
    }
    else imu_ang = p_imu_angle->yaw;
    carpos_angle = imu_ang + demarcate_angle;   //加上标定的差值
    if ((carpos_angle>=360)&&(carpos_angle<720))
    {
        carpos_angle = carpos_angle - 360;
    }
    else if (carpos_angle>=720)
    {
       carpos_angle = carpos_angle - 720;
    }
    p_carpos->yaw = carpos_angle;
    p_carpos->pitch = p_imu_angle->pitch;
    p_carpos->roll = p_imu_angle->roll;
    //printf("p_carpos: %f\n",p_carpos->yaw);
}
//计算车的中心点坐标
void uwb_center_addr_count(uwb_info_t *p_uwb,int num,carpos_t *p_carpos)
{
    double angle = p_carpos->yaw/180*PI;
    //printf("Heading: %f\n",p_carpos->yaw);
    switch (num)
    {
    case 1:                                 //接收到后面uwb数据
        p_uwb->center.quality = p_uwb->addr[UWB_BACK].quality;
        p_uwb->center.z = p_uwb->addr[UWB_BACK].z;
        if(angle<=PI/2)
        {
            p_uwb->center.x = p_uwb->addr[UWB_BACK].x + CENTER_BACK*cos(angle);
            p_uwb->center.y = p_uwb->addr[UWB_BACK].y - CENTER_BACK*sin(angle);
        }
        else if((angle<=PI)&&(angle>PI/2))
        {
            p_uwb->center.x = p_uwb->addr[UWB_BACK].x - CENTER_BACK*cos(PI-angle);
            p_uwb->center.y = p_uwb->addr[UWB_BACK].y - CENTER_BACK*sin(PI-angle);
        }
        else if((angle<=PI*2/3)&&(angle>PI))
        {
            p_uwb->center.x = p_uwb->addr[UWB_BACK].x - CENTER_BACK*cos(angle-PI);
            p_uwb->center.y = p_uwb->addr[UWB_BACK].y + CENTER_BACK*sin(angle-PI);
        }
        else if((angle<2*PI)&&(angle>PI*2/3))
        {
            p_uwb->center.x = p_uwb->addr[UWB_BACK].x + CENTER_BACK*cos(2*PI-angle);
            p_uwb->center.y = p_uwb->addr[UWB_BACK].y + CENTER_BACK*sin(2*PI-angle);
        }
        //printf("BACK: x1:%f,y1:%f,x:%f,y:%f\n",p_uwb->addr[UWB_BACK].x,p_uwb->addr[UWB_BACK].y,p_uwb->center.x,p_uwb->center.y);
        break;
    case 2:         //接收到前面uwb数据
        p_uwb->center.quality = p_uwb->addr[UWB_FRONT].quality;
        p_uwb->center.z = p_uwb->addr[UWB_FRONT].z;
        if(angle<=PI/2)
        {
            p_uwb->center.x = p_uwb->addr[UWB_FRONT].x - CENTER_FRONT*cos(angle);
            p_uwb->center.y = p_uwb->addr[UWB_FRONT].y + CENTER_FRONT*sin(angle);
        }
        else if((angle<=PI)&&(angle>PI/2))
        {
            p_uwb->center.x = p_uwb->addr[UWB_FRONT].x + CENTER_FRONT*cos(PI-angle);
            p_uwb->center.y = p_uwb->addr[UWB_FRONT].y + CENTER_FRONT*sin(PI-angle);
        }
        else if((angle<=PI*2/3)&&(angle>PI))
        {
            p_uwb->center.x = p_uwb->addr[UWB_FRONT].x + CENTER_FRONT*cos(angle-PI);
            p_uwb->center.y = p_uwb->addr[UWB_FRONT].y - CENTER_FRONT*sin(angle-PI);
        }
        else if((angle<2*PI)&&(angle>PI*2/3))
        {
            p_uwb->center.x = p_uwb->addr[UWB_FRONT].x - CENTER_FRONT*cos(2*PI-angle);
            p_uwb->center.y = p_uwb->addr[UWB_FRONT].y - CENTER_FRONT*sin(2*PI-angle);
        }
        //printf("FRONT: x2:%f,y2:%f,x:%f,y:%f\n",p_uwb->addr[UWB_FRONT].x,p_uwb->addr[UWB_FRONT].y,p_uwb->center.x,p_uwb->center.y);
        break;
    break;
        case 3:     //接收到两个uwb数据
        p_uwb->center.x = (p_uwb->addr[UWB_BACK].x + p_uwb->addr[UWB_FRONT].x)/2;  //取两点的中间值作为车的中心
        p_uwb->center.y = (p_uwb->addr[UWB_BACK].y + p_uwb->addr[UWB_FRONT].y)/2;
        p_uwb->center.z = (p_uwb->addr[UWB_BACK].z + p_uwb->addr[UWB_FRONT].z)/2;
        p_uwb->center.quality = (p_uwb->addr[UWB_BACK].quality + p_uwb->addr[UWB_FRONT].quality)/2;
        //printf("CENTER: x1:%f,y1:%f,x2:%f,y2:%f,x:%f,y:%f\n",p_uwb->addr[UWB_BACK].x,p_uwb->addr[UWB_BACK].y,p_uwb->addr[UWB_FRONT].x,p_uwb->addr[UWB_FRONT].y,p_uwb->center.x,p_uwb->center.y);
        break;
    default:
        //printf("num error value: %d\n",num);
        break;
    }
}
void uwb_count_control(const uwb_info_t *p_uwb,const uwblog_addr_t *p_logpos,control_info_t *p_control_info,const carpos_t *p_carpos)
{
    //double goal_x = 0,goal_y = 0;
    double self_angle = 0;
    double goal_angle = 0;
    double angle_diff = 0;
    int index = 0;
    index = p_uwb->index.goal_index;
    double filter_log[3];     //x,y,z
    //对log中的数据进行滤波处理
    uwblog_median_filter(p_logpos,p_uwb->index.goal_index,filter_log,UWB_LOG_MAXNUM);
    goal_angle = tag_getAngle(p_uwb->center.x,p_uwb->center.y, filter_log[0], filter_log[1]);
    self_angle = p_carpos->yaw;
    angle_diff = goal_angle - self_angle;  //大于0实际需要右转
    //由于偏差角度大于180,所以要朝另外一个方向转，即朝左，朝左值要为负
    if (angle_diff>180)  angle_diff = -360 + angle_diff;
    if (angle_diff<-180) angle_diff = 360 + angle_diff;
    if (angle_diff<-TAG_CAR_ANGLE)  angle_diff = -TAG_CAR_ANGLE;
    if (angle_diff>TAG_CAR_ANGLE)   angle_diff = TAG_CAR_ANGLE;
    p_control_info->angle = -angle_diff;     //此处角度差增大时应该是右转，右转角度要减小，所以为负
    printf("goal_angle: %f,self_angle: %f,angle_diff: %f\n",goal_angle,self_angle,-angle_diff);
    //printf("angle_diff: %f\n",-angle_diff);
    p_control_info->dis = tag_get_distance(p_uwb->center.x,p_uwb->center.y, filter_log[0], filter_log[1]);
    p_control_info->speed = TAG_CAR_SPEED;
}
//UWB计算速度角度
// void uwb_count_control(_uwb_addr *p_uwb_addr,const uwblog_addr_t *puwblog_addr_t,control_info_t *p_control_info,const _uwb_index_pos *p_index_pos)
// {
//     static double s_car_angle[10] = {0};
//     static int num = 0;
//     float goal_x = 0,goal_y = 0;
//     double self_angle = 0;
//     int index;
//     index = p_index_pos->goal_index;
//     if (p_index_pos->goal_index<1)
//     {
//         index = 1;
//     }
//     if (p_index_pos->goal_index>UWB_LOG_MAXNUM-1)
//     {
//         index = UWB_LOG_MAXNUM-1;
//     }
//     if (index>0)
//     {
//         for (int i = 0; i < 3; i++)
//         {
//             goal_x += puwblog_addr_t[index+i-1].x;
//             goal_y += puwblog_addr_t[index+i-1].y;
//         }
//         goal_x = goal_x/3.f;
//         goal_y = goal_y/3.f;
//     }
//     else
//     {
//         goal_x = puwblog_addr_t[index].x;
//         goal_y = puwblog_addr_t[index].y;
//     }
//     p_control_info->angle = tag_getAngle(p_uwb_addr[0].x, p_uwb_addr[0].y, goal_x, goal_y);
//     self_angle = p_control_info->angle - uwb_count_angle;   //此处和GPS导航正好相反
//     printf("angle: %f,self_angle: %f\n",p_control_info->angle,self_angle);
//     if (self_angle>180)                  //由于偏差角度大于180,所以要朝另外一个方向转，即朝左，朝左值要为负
//     {
//         self_angle = -360+self_angle;
//     }
//     if (self_angle<-180)
//     {
//        self_angle =360+self_angle;
//     }
//     if (self_angle<-20)
//     {
//         self_angle=-20;
//     }
//     if (self_angle>20)
//     {
//         self_angle=20;
//     }
//     // s_car_angle[num++] = p_control_info->angle;
//     // if (num>=5)
//     // {
//     //     num = 0;
//     // }
//     // double angle1 = 0;
//     // for (int i = 0; i < 5; i++)
//     // {
//     //     angle1 += s_car_angle[i];
//     // }
//     // angle1 = angle1/5.0f;
//     // p_control_info->angle = p_control_info->angle -angle1; 
//      p_control_info->angle = self_angle;
//     printf("self_angle: %f\n",-self_angle);
//     p_control_info->dis   = tag_get_distance(p_uwb_addr[0].x, p_uwb_addr[0].y, goal_x, goal_y);
//     p_control_info->speed = 0.5;
// }
//UWB导航中定位自己和目标在数组中的位置
void uwb_traversal_array(uwb_info_t *p_uwb,const uwblog_addr_t *p_logpos)
{
    double min_dis = 0;
    float a = 0,b = 0;
    int front_traversal = TAG_TRAVER_FRONT,back_traversal = TAG_TRAVER_BACK;//前后一共遍历600个数组
    if (UWB_LOG_MAXNUM-p_uwb->index.self_index<TAG_TRAVER_BACK)
    {
        back_traversal = UWB_LOG_MAXNUM - p_uwb->index.self_index;          //-2避免下面遍历时越界
    }
    else
    {
       back_traversal = TAG_TRAVER_BACK;
    }
    if(p_uwb->index.self_index<TAG_TRAVER_FRONT)
    {
       front_traversal = p_uwb->index.self_index-2;                         //-2避免下面遍历时越界
    }
    else
    {
        front_traversal = TAG_TRAVER_FRONT;
    }
    min_dis = tag_get_distance(p_uwb->center.x,p_uwb->center.y,p_logpos[p_uwb->index.self_index].x,p_logpos[p_uwb->index.self_index].y);
    for (int i = p_uwb->index.self_index+1; i < p_uwb->index.self_index + back_traversal; i++)
    {
        a = tag_get_distance(p_uwb->center.x,p_uwb->center.y,p_logpos[i].x,p_logpos[i].y);
        //a = sqrt(pow(p_uwb->center.x-p_logpos[i].x,2)+pow(p_uwb->center.y-p_logpos[i].y,2));
        if (a<min_dis)
        {
            min_dis = a;
            p_uwb->index.self_index = i;
        }
    }
    printf("self_index is %d,Dis is: %lf\n",p_uwb->index.self_index,min_dis);
    min_dis = tag_get_distance(p_logpos[p_uwb->index.self_index+1].x,p_logpos[p_uwb->index.self_index+1].y,\
    p_logpos[p_uwb->index.self_index].x,p_logpos[p_uwb->index.self_index].y)+CONST_GOAL_DIS;
    for (int i = p_uwb->index.self_index+2; i < p_uwb->index.self_index + back_traversal; i++)
    {
        a = tag_get_distance(p_logpos[i].x,p_logpos[i].y,\
        p_logpos[p_uwb->index.self_index].x,p_logpos[p_uwb->index.self_index].y);
        // a = sqrt(pow(p_logpos[i].x-p_logpos[p_uwb->index.self_index].x,2)+\
        // pow(p_logpos[i].y-p_logpos[p_uwb->index.self_index].y,2));
        if (fabs(a-CONST_GOAL_DIS)<min_dis)
        {
            min_dis = fabs(a-CONST_GOAL_DIS);
            p_uwb->index.goal_index = i;
        }
    }
    printf("goal_index is %d,Dis is: %f\n",p_uwb->index.goal_index,min_dis);
}
//UWB定位自己起始的位置,后期可以改为和哪个标签有通讯，只遍历这个标签附近的坐标
int uwb_start_positin(const uwb_info_t *p_uwb,const uwblog_addr_t *p_uwblog)
{
    double min_dis=0;
    float a = 0;
    int pos = 0;
    if (UWB_LOG_MAXNUM>0)
    {
        min_dis = tag_get_distance(p_uwb->center.x,p_uwb->center.y,p_uwblog[0].x,p_uwblog[0].y);
        for(int i = 1; i < UWB_LOG_MAXNUM; i++)
        {
            a = tag_get_distance(p_uwb->center.x,p_uwb->center.y,p_uwblog[i].x,p_uwblog[i].y);
            if (a!=0)
            {
                if (a<min_dis)
                {
                    min_dis = a;
                    pos = i;
                }
            }
        }
    }
    else
    {
        printf("UWB_LOG_MAXNUM is: %ld\n",UWB_LOG_MAXNUM);
    }
    return pos;
}

//****************************GPS********************************************//
/**
 * @brief  根据GPS传来的数据，给carpos赋值
 * @param  p_carpos         My Param doc
 * @param  p_gps_info       My Param doc
 */
void gps_count_carpos(carpos_t *p_carpos,gps_info_t *p_gps_info)
{
    p_carpos->yaw = p_gps_info->addr.Heading;
    p_carpos->pitch = p_gps_info->addr.pitch;
    p_carpos->roll = p_gps_info->addr.roll;
}
/**
 * @brief  gps根据两点计算速度和转向角度
 * @param  p_gps            My Param doc
 * @param  p_gpslog         My Param doc
 * @param  p_control_info   My Param doc
 */
void gps_count_control(gps_info_t *p_gps,const gpslog_addr_t *p_gpslog,control_info_t *p_control_info)
{
    float goal_x = 0,goal_y = 0;
    int index = 0;
    index = p_gps->index.goal_index;
    if (p_gps->index.goal_index<1)
    {
        index = 1;
    }
    if (p_gps->index.goal_index>GPS_LOG_MAXNUM-1)
    {
        index = GPS_LOG_MAXNUM-1;
    }
    lat_lon(p_gps,p_gpslog,p_control_info);
    p_control_info->speed = GPS_CAR_SPEED;
}
//导航中定位自己和目标在数组中的位置,自己的位置不一定找得到，还需要解决
void gps_traversal_array(gps_info_t *p_gps,const gpslog_addr_t *p_gpslog)
{
    double min_dis;
    double a = 0;
    int front_traversal = GPS_TRAVER_FRONT,back_traversal = GPS_TRAVER_BACK;   //前后一共遍历400个数组
    if (GPS_LOG_MAXNUM-p_gps->index.self_index<GPS_TRAVER_BACK)
    {
        back_traversal = GPS_LOG_MAXNUM - p_gps->index.self_index-1;
    }
    else
    {
       back_traversal = GPS_TRAVER_BACK;
    }
    if(p_gps->index.self_index<GPS_TRAVER_FRONT)
    {
       front_traversal = p_gps->index.self_index;
    }
    else
    {
        front_traversal = GPS_TRAVER_FRONT;
    }
    //先遍历出自己所在的位置
    min_dis = gps_get_distance(p_gps->addr.Lattitude,p_gps->addr.Longitude,p_gpslog[p_gps->index.self_index].Lattitude,p_gpslog[p_gps->index.self_index].Longitude);
    for (int i = p_gps->index.self_index+1; i < p_gps->index.self_index + back_traversal; i++)
    {
        a = gps_get_distance(p_gps->addr.Lattitude,p_gps->addr.Longitude,p_gpslog[i].Lattitude,p_gpslog[i].Longitude);
        if (a<min_dis)
        {
            min_dis = a;
            p_gps->index.self_index = i;
        }
    }
    printf("self_index is %d,Dis is: %lf\n",p_gps->index.self_index,min_dis);
    //遍历出目标所在的位置，目标是在log中后60cm来取的
    min_dis = gps_get_distance(p_gpslog[p_gps->index.self_index].Lattitude,p_gpslog[p_gps->index.self_index].Longitude,\
    p_gpslog[p_gps->index.self_index+1].Lattitude,p_gpslog[p_gps->index.self_index+1].Longitude)+CONST_GOAL_DIS;
    for (int i = p_gps->index.self_index+2; i < p_gps->index.self_index + back_traversal; i++)
    {
        a = gps_get_distance(p_gpslog[p_gps->index.self_index].Lattitude,\
        p_gpslog[p_gps->index.self_index].Longitude,p_gpslog[i].Lattitude,p_gpslog[i].Longitude);
        if (fabs(a-CONST_GOAL_DIS)<min_dis)
        {
            min_dis = fabs(a-CONST_GOAL_DIS);
            p_gps->index.goal_index = i;
        }
    }
    printf("goal_index is %d,Dis is: %f\n",p_gps->index.goal_index,min_dis);
}
//定位自己起始的位置,后期可以改为和哪个标签有通讯，只遍历这个标签附近的坐标
int gps_start_positin(const gps_info_t *p_gps,const gpslog_addr_t *p_gpslog)
{
    double min_dis;
    double a = 0;
    int pos = 0;
    if (GPS_LOG_MAXNUM>0)
    {
        min_dis = gps_get_distance(p_gps->addr.Lattitude,p_gps->addr.Longitude,\
        p_gpslog[0].Lattitude,p_gpslog[0].Longitude);
        for(int i = 0; i < GPS_LOG_MAXNUM; i++)
        {
            a = gps_get_distance(p_gps->addr.Lattitude,p_gps->addr.Longitude,\
            p_gpslog[i].Lattitude,p_gpslog[i].Longitude);
            if (a!=0)
            {
                if (a<min_dis)
                {
                    min_dis = a;
                    pos = i;
                }
            }
        }
    }
    else
    {
        printf("GPS_LOG_MAXNUM = 0 is: %ld\n",GPS_LOG_MAXNUM);
    }
    return pos;
}
//UWB标定算法
int uwb_init_demarcate(const uwb_info_t *p_uwb,carpos_t *p_carpos,const float demarcate_dis)
{
    static int remeber_start = 1;             //开始标定
    static double dem_pos[10000][3]={0},dem_start_end[10][3]={0};
    static int dem_index = 0,dem_index1 = 0;
    static int count =0;
    double angle[10] = {0};
    double angle_heading = 0;
    double angle_dem = 0;
    double b[10];//= {10000,10000,10000,10000,10000,10000,10000,10000,10000,10000};
    memset(b,10000,10);
    if (remeber_start)
    {
        remeber_start = 0;
        printf("\r\n开始标定航向,请走直线........................\n");
    }
    dem_pos[dem_index][0] = p_uwb->addr[UWB_BACK].x;
    dem_pos[dem_index][1] = p_uwb->addr[UWB_BACK].y;
    dem_pos[dem_index][2] = p_carpos->yaw;
    if (dem_index==10)
    {
        for (int i = 0; i < dem_index; i++)
        {
            dem_start_end[dem_index1][0] += dem_pos[i][0];
            dem_start_end[dem_index1][1] += dem_pos[i][1];
            dem_start_end[dem_index1][2] += dem_pos[i][2];
        }
        dem_start_end[0][0] = dem_start_end[dem_index1][0]/(dem_index);
        dem_start_end[0][1] = dem_start_end[dem_index1][1]/(dem_index);
        dem_start_end[0][2] = dem_start_end[dem_index1][2]/(dem_index);
    }
    else if (dem_index>10)
    {
        printf(">>");
        double dis1 = tag_get_distance(dem_pos[dem_index][0], dem_pos[dem_index][1],dem_start_end[0][0],dem_start_end[0][1]);
        //printf("dis1: %f\n",dis1);
        if (dis1<=500)
        {
            remote_control(CARDRIVE,0,TAG_DEM_SPEED);
        }
        else
        {
            count ++;
            remote_control(CARDRIVE,0,0);
        }
        if (count>=30)   //count计算接收30个后车才稳定，做最后处理
        {
            for (int i = 10; i < dem_index-10; i++)
            {
                dis1 = tag_get_distance(dem_start_end[0][0], dem_start_end[0][1],dem_pos[i][0], dem_pos[i][1]);
                if (fabs(dis1-50)<b[0])
                {
                    b[0] = fabs(dis1-50);
                    dem_start_end[1][0] = dem_pos[i][0];
                    dem_start_end[1][1] = dem_pos[i][1];
                    dem_start_end[1][2] = dem_pos[i][2];
                }
                if (fabs(dis1-100)<b[1])
                {
                    b[1] = fabs(dis1-100);
                    dem_start_end[2][0] = dem_pos[i][0];
                    dem_start_end[2][1] = dem_pos[i][1];
                    dem_start_end[2][2] = dem_pos[i][2];
                }
                if (fabs(dis1-150)<b[2])
                {
                    b[2] = fabs(dis1-150);
                    dem_start_end[3][0] = dem_pos[i][0];
                    dem_start_end[3][1] = dem_pos[i][1];
                    dem_start_end[3][2] = dem_pos[i][2];
                }
                if (fabs(dis1-200)<b[3])
                {
                    b[3] = fabs(dis1-200);
                    dem_start_end[4][0] = dem_pos[i][0];
                    dem_start_end[4][1] = dem_pos[i][1];
                    dem_start_end[4][2] = dem_pos[i][2];
                }
                if (fabs(dis1-demarcate_dis+150)<b[4])
                {
                    b[4] = fabs(dis1-demarcate_dis+150);
                    dem_start_end[5][0] = dem_pos[i][0];
                    dem_start_end[5][1] = dem_pos[i][1];
                    dem_start_end[5][2] = dem_pos[i][2];
                }
                if (fabs(dis1-demarcate_dis+100)<b[5])
                {
                    b[5] = fabs(dis1-demarcate_dis+100);
                    dem_start_end[6][0] = dem_pos[i][0];
                    dem_start_end[6][1] = dem_pos[i][1];
                    dem_start_end[6][2] = dem_pos[i][2];
                }
                if  (fabs(dis1-demarcate_dis+50)<b[6])
                {
                    b[6] = fabs(dis1-demarcate_dis+50);
                    dem_start_end[7][0] = dem_pos[i][0];
                    dem_start_end[7][1] = dem_pos[i][1];
                    dem_start_end[7][2] = dem_pos[i][2];
                }
            }
            for (int i = dem_index-10; i < dem_index; i++)
            {
                dem_start_end[8][0] += dem_pos[i][0];
                dem_start_end[8][1] += dem_pos[i][1];
                dem_start_end[8][2] += dem_pos[i][2];
            }
            dem_start_end[8][0] = dem_start_end[8][0]/10.f;
            dem_start_end[8][1] = dem_start_end[8][1]/10.f;
            dem_start_end[8][2] = dem_start_end[8][2]/10.f;
            angle[0]  = tag_getAngle(dem_start_end[0][0],dem_start_end[0][1],dem_start_end[8][0],dem_start_end[8][1]);
            angle_dem = angle[0];
            for (int i = 1; i < 5; i++)
            {
                angle[i] = tag_getAngle(dem_start_end[i][0],dem_start_end[i][1],dem_start_end[i+4][0],dem_start_end[i+4][1]);
                if (angle[i]-angle[0]>180)        //angle[0]<90，angle[i]>270,让他们值统一为-90到90，方便计算
                {
                    angle[i] = angle[i]-360;
                }
                else if(angle[i]-angle[0]<-180)  //angle[0]>270，angle[i]<90,让他们值统一为270到450，方便计算
                {
                    angle[i] = angle[i]+360;
                }
                angle_dem += angle[i];
            }
            for (int i = 0; i < 9; i++)
            {
                angle_heading += dem_start_end[i][2];
            }
            angle_dem = angle_dem/5.0f;
            if (fabs(angle_dem-angle[0])<=10)  //如果平均角度和最长的角度差值小于5度，认为标定正确
            {
                printf("\n\r恭喜标定完成！\n\r");
                demarcate_flag = 1;
            }
            else
            {
                printf("\n\r标定失败！\n\r");
                demarcate_flag = 0;
            }
            printf("标定质量：%f\n",angle_dem-angle[0]);
            if (angle_dem>360)
            {
                angle_dem = angle_dem - 360;
            }
            else if (angle_dem<0)
            {
                angle_dem = angle_dem + 360;
            }
            angle_heading = angle_heading/9.f;
            double angle_dem1 = angle_dem - angle_heading;
            demarcate_imu = angle_heading;
            if (angle_dem1<0)       //这里让航向角一直在经纬度角的后面，永远为正，方便计算
            {
                angle_dem1 = 360 - angle_heading + angle_dem;
            }
            printf("标定结果：%f\n",angle_dem1);
            
            //printf("angle_dem: %f,angle[0]: %f\n",angle_dem,angle[0]);
            return angle_dem1;
        }
    }
    dem_index++;
    if (dem_index>=1200)
    {
        printf("标定时间过长，错误：%d\n",dem_index);
        dem_index = 0;
        return 0;
    }
    return 0;
}


