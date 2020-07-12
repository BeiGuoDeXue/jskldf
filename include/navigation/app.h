#ifndef __App_H__
#define __App_H__
#include "bsp_readfile.h"
#include "bsp_gps.h"
#include "bsp_tag.h"
#include "bsp_imu.h"

typedef struct
{
    float angle;   //单位角度
    float dis;     //单位cm
    float speed;
}control_info_t;

//车的姿态
typedef struct
{
    double yaw;     //航向角
    double pitch;   //俯仰角
    double roll;    //翻滚角
    double speed;   //车速
}carpos_t;


//TAG
void uwb_count_carpos(const uwb_info_t *p_uwb,carpos_t *p_carpos,_imu_angle *p_imu_angle);
void uwb_center_addr_count(uwb_info_t *p_uwb,int num,carpos_t *p_carpos);
void uwb_count_control(const uwb_info_t *p_uwb,const uwblog_addr_t *p_logpos,control_info_t *p_control_info,const carpos_t *p_carpos);
void uwb_traversal_array(uwb_info_t *p_uwb,const uwblog_addr_t *p_logpos);
int uwb_start_positin(const uwb_info_t *p_uwb,const uwblog_addr_t *p_uwblog);
int uwb_init_demarcate(const uwb_info_t *p_uwb,carpos_t *p_carpos,const float demarcate_dis);
//GPS
void gps_count_carpos(carpos_t *p_carpos,gps_info_t *p_gps_info);
void gps_count_control(gps_info_t *p_gps,const gpslog_addr_t *p_gpslog,control_info_t *p_control_info);
void gps_traversal_array(gps_info_t *p_gps,const gpslog_addr_t *p_gpslog);
int gps_start_positin(const gps_info_t *p_gps,const gpslog_addr_t *p_gpslog);




#endif

