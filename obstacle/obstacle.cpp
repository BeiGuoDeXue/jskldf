#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <errno.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <stdlib.h>
#include <sys/time.h>
#include <pthread.h>
#include <math.h>
#include "bsp_uwb.h"
#include "bsp_timer.h"
#include "bsp_lidar.h"
#include "bsp_imu.h"
#include "bsp_car_drive.h"
#include "bsp_uart.h"
#include "app.h"
#include "bsp_pid.h"
#include "config.h"
#include "server_remote.h"
#include "bsp_hps3d.h"
#include "service.h"
#include "obstacle.h"
#include "bsp_lat_lon.h"
int obs_flaging = 0;
extern int gps_flag;
extern obs_hps_t obs_hps;
extern control_info_t control_info;
obs_dis_t obs_dis;
obs_area_t obs_area[AREA_NUM];
int obs_flaging1 = 0;
/*********************************GPS信息***************************************/
extern gps_info_t gps_info;
extern gpslog_addr_t gpslog_addr[];

static void obs_judge(int obs_flag1,obs_hps_t *p_obshps);
static int  obs_traverse_area(obs_hps_t *p_obshps,obs_area_t *p_obsarea);
static void obs_count_addr(obs_hps_t *p_obshps,obs_area_t *p_obsarea,int area_num);
static void count_control(obs_area_t *p_obsarea,gps_info_t *p_gpsinfo,control_info_t *p_control_info,double *p_gpsposition);
/**
 * @brief 避障初始化
 */
void obs_init(void)
{
    obs_dis.obs_dis_front.warning_dis = 1.5;
    obs_dis.obs_dis_front.obs_dis = 1;
    obs_dis.obs_dis_front.stop_dis = 0.3;
    obs_dis.obs_dis_back.warning_dis = 0.6;
    obs_dis.obs_dis_back.obs_dis = 0.3;
    obs_dis.obs_dis_back.stop_dis = 0.2;
    obs_dis.obs_dis_left.warning_dis = 0.6;
    obs_dis.obs_dis_left.obs_dis = 0.3;
    obs_dis.obs_dis_left.stop_dis = 0.2;
    obs_dis.obs_dis_right.warning_dis = 0.6;
    obs_dis.obs_dis_right.obs_dis = 0.3;
    obs_dis.obs_dis_right.stop_dis = 0.2;
    memset(obs_hps.threshold_state,0,AREA_NUM);
    memset(obs_hps.threshold_dist_avg,0,AREA_NUM);
}
/**
 * @brief 避障调用此处
 */
void obs_control(int obs_flag1)
{
    int area_num = 0;
    static double gps_position[3];                  //存储一个避障时GPS位置信息
    obs_judge(obs_flag1,&obs_hps);                  //判断避障模式
    if ((obs_hps.obs_hps_flag == HPS_WARNING_FLAG)&&(obs_flaging1 = 0))   //有障碍物在警报区域,车子减速,但是仍可导航
    {
        control_info.speed = OBS_SPEED;
        remote_control(CARDRIVE,control_info.angle,control_info.speed);
    }
    else if (obs_hps.obs_hps_flag == HPS_OBS_FLAG)
    {
        obs_flaging = 1;
        obs_flaging1 = 1;
        gps_position[0] = gps_info.addr.Lattitude;
        gps_position[1] = gps_info.addr.Longitude;
        gps_position[2] = gps_info.addr.Heading;
        area_num = obs_traverse_area(&obs_hps,obs_area);                //遍历出有几个可以走的区域
        if (area_num==0)                                                //36个区域没有可以前进的方向,还需后续处理
        {
            // obs_flaging = 1;
            // obs_flaging1 = 1;        //需要后退,或者遥控
        }
        obs_count_addr(&obs_hps,obs_area,area_num);
        count_control(obs_area,&gps_info,&control_info,gps_position);
    }
    else if (obs_hps.obs_hps_flag == HPS_STOP_FLAG)    //有障碍物在停止区域,车子停下,让后台遥控
    {
        obs_flaging = 1;
        obs_flaging1 = 0;
        control_info.speed = 0;
        control_info.angle = 0;
        remote_control(CARDRIVE,control_info.angle,control_info.speed);
    }
    else if (obs_hps.obs_hps_flag == HPS_NONE_FLAG)
    {
         //printf("障碍物不需要避障\n");
        if (obs_flaging1==1)
        {
            count_control(obs_area,&gps_info,&control_info,gps_position);
        }
        else
        {
            obs_flaging = 0;                            //结束避障模式
        }
    }
    else
    {
        printf("障碍物模式错误：%d\n",obs_hps.obs_hps_flag);
    }


    // switch (obs_hps.obs_hps_flag)
    // {
    // case HPS_WARNING_FLAG:                          //有障碍物在警报区域,车子减速,但是仍可导航
    //     control_info.speed = OBS_SPEED;
    //     remote_control(CARDRIVE,control_info.angle,control_info.speed);
    //     break;
    // case HPS_STOP_FLAG:                             //有障碍物在停止区域,车子停下,让后台遥控
    //     obs_flaging = 1;
    //     obs_flaging1 = 0;
    //     control_info.speed = 0;
    //     control_info.angle = 0;
    //     remote_control(CARDRIVE,control_info.angle,control_info.speed);
    //     break;
    // case HPS_OBS_FLAG:
    //     obs_flaging = 1;
    //     obs_flaging1 = 1;
    //     gps_position[0] = gps_info.addr.Lattitude;
    //     gps_position[1] = gps_info.addr.Longitude;
    //     gps_position[2] = gps_info.addr.Heading;
    //     area_num = obs_traverse_area(&obs_hps,obs_area);                //遍历出有几个可以走的区域
    //     if (area_num==0)                                                //36个区域没有可以前进的方向,还需后续处理
    //     {
    //         // obs_flaging = 1;
    //         // obs_flaging1 = 1;        //需要后退,或者遥控
    //         break;
    //     }
    //     obs_count_addr(&obs_hps,obs_area,area_num);
    //     count_control(obs_area,&gps_info,&control_info,gps_position);
    //     break;
    // case HPS_NONE_FLAG:
    //     if (obs_flaging1==1)
    //     {
    //         count_control(obs_area,&gps_info,&control_info,gps_position);
    //     }
    //     else
    //     {
    //         obs_flaging = 0;
    //     }
        
    //     //printf("障碍物不需要避障\n");
    //     break;
    // default:
    //     printf("障碍物模式错误：%d\n",obs_hps.obs_hps_flag);
    //     break;
    // }
}
/**
 * @brief 遍历前方障碍物留下的口子
 * 找出车子能过的口子，计算出坐标点保存到数组中
 */
int obs_traverse_area(obs_hps_t *p_obshps,obs_area_t *p_obsarea)
{
    int num = 0;
    int area_num = 0;                                      //代表口子的数量-1
    for (int i = 0; i < AREA_NUM; i++)
    {
        if (p_obshps->threshold_state[i]==0)               //为0,判断没有障碍物
        {
            num++;
            if (num>=OBS_AREA_WIDTH)
            {
                p_obsarea[area_num].start_area = i+1-num;
                p_obsarea[area_num].end_area = i;
                p_obsarea[area_num].num = num;
            }
        }
        else
        {
            if (num>=OBS_AREA_WIDTH)                    //如果有障碍物判断原来的区域是否大于可通行的大小,如果大于区域数加1
            {
                area_num++;
            }
            num = 0;
        }
    }
    if (num>=OBS_AREA_WIDTH)                            //避免遍历结束时最后一个区域被忽略
    {
        area_num++;
    }
    return area_num;
}

//需要添加，前行区域如果有障碍物距离小于30cm，就后退
//不同的距离所对应的可行走区域大小不同，后期处理
/**
 * @brief 避障计算下一个目标点位置
 * 通过x，y距离，方向角，计算出下一个点的经纬度，并且找到最优的点
 * p_obsarea[goal_area].angle[2] 角度为正负90度,向右为正,向左为负
 */
static void obs_count_addr(obs_hps_t *p_obshps,obs_area_t *p_obsarea,int area_num)
{
    int goal_area = 0;
    int number = 0;
    // for (int i = 0; i < AREA_NUM; i++)               //先不开发后退
    // {
    //     if (p_obshps->threshold_dist_avg[i]>0&&p_obshps->threshold_dist_avg[i]<OBS_BACK_DIS)
    //     {
            
    //     }
    // }
    // for (int i = 0; i < area_num; i++)
    // {
    //     p_obsarea[i].angle[0] = (p_obsarea[i].start_area)*4.75-4.75/2;       //区域边缘角度
    //     p_obsarea[i].dis[0] = 1;                                             //区域边缘距离
    //     p_obsarea[i].angle[1] = (p_obsarea[i].end_area)*4.75-4.75/2;
    //     p_obsarea[i].dis[1] = 1;
    // }
    for (int i = 0; i < area_num; i++)                                          //找到最大的区域,先不与路径最近的区域比较
    {
        if (number<p_obsarea[i].num)
        {
            goal_area = i;
            number = p_obsarea[i].num;
        }
    }
    if (goal_area==0)                                                       //这里代表车子的右侧,假设的
    {
        p_obsarea[goal_area].angle[2] = (p_obsarea[goal_area].end_area-int(OBS_AREA_WIDTH/2)+1)*4.75-4.75/2;         //区域中心角度
        p_obsarea[goal_area].dis[2] = obs_dis.obs_dis_front.obs_dis;    
    }
    else if (goal_area==area_num-1)                                           //这里代表车子的左侧,假设的
    {
        p_obsarea[goal_area].angle[2] = (p_obsarea[goal_area].start_area+int(OBS_AREA_WIDTH/2)+1)*4.75-4.75/2;       //区域中心角度
        p_obsarea[goal_area].dis[2] = obs_dis.obs_dis_front.obs_dis;                                                                    //区域中心距离
    }
    else if (goal_area>0&&goal_area<area_num-1)
    {
        p_obsarea[goal_area].angle[2] = (int)((p_obsarea[goal_area].start_area+p_obsarea[goal_area].end_area)/2)*4.75-4.75/2;      //区域中心角度
        p_obsarea[goal_area].dis[2] = obs_dis.obs_dis_front.obs_dis;                                                                    //区域中心距离
    }
    if (p_obsarea[goal_area].angle[2]>90)                                           //大于90度,为左转
    {
        p_obsarea[goal_area].angle[2] = -(p_obsarea[goal_area].angle[2] - 90);
    }
    else
    {
        p_obsarea[goal_area].angle[2] = 90 - p_obsarea[goal_area].angle[2] ;
    }
}
/**  
 * @brief 根据x，y计算出下点经纬度
 */
static void count_control(obs_area_t *p_obsarea,gps_info_t *p_gpsinfo,control_info_t *p_control_info,double *p_gpsposition)
{
    double angle = 0;
    static double goal_s = p_obsarea->dis[2],goal_angle = p_obsarea->angle[2];
    double dis = gps_get_distance(p_gpsinfo->addr.Lattitude,p_gpsinfo->addr.Longitude,p_gpsposition[0],p_gpsposition[1]);
    p_control_info->speed = OBS_SPEED;
    //如果距离小于20cm时,让角度为固定角度,避免角度摆动太大
    if (dis<=0.2)
    {
        angle = goal_angle/2;
    }
    else if (dis>0.2)                           //距离大于20cm时,让车子正常走到目标点
    {
        //解决加上的角度超过360,或者小于0
        goal_angle = p_gpsposition[2]+goal_angle;
        if (goal_angle>=360)
        {
            goal_angle = goal_angle-360;
        }
        else if(goal_angle<0)
        {
            goal_angle = goal_angle+360;
        }
        //计算要转的角度
        if (goal_angle-p_gpsinfo->addr.Heading>180)                 //左转,目标的角度大于180,新的角度小于180
        {
            angle = 360 - goal_angle + p_gpsinfo->addr.Heading;     //轮子左转为正
        }
        else if (p_gpsposition[2]-p_gpsinfo->addr.Heading<-180)      //右转,目标的角度小于180,新的角度大于180
        {
            angle = -(360-p_gpsinfo->addr.Heading + p_gpsinfo->addr.Heading);  //轮子右转为负值
        }
        else
        {
            angle = -(goal_angle-p_gpsinfo->addr.Heading);      //新的角度
        }
    }
    if (dis-obs_dis.obs_dis_front.obs_dis>0.2)  //超过目标点0.2m，且角度差小于20度认为到达目标点
    {
        // if (p_gpsinfo->addr.Heading)                         //先不处理平行的时候开始导航
        // {
            
        // }
        obs_flaging = 0;
        obs_flaging1 = 0;
    }
    // if ((dis-obs_dis.obs_dis_front.obs_dis>0.1)&&(fabs(angle)<20))  //超过目标点0.1m，且角度差小于20度认为到达目标点
    // {
    //     // if (p_gpsinfo->addr.Heading)                         //先不处理平行的时候开始导航
    //     // {
            
    //     // }
    //     obs_flaging = 0;
    //     obs_flaging1 = 0;
    // }
    // if (dis-obs_dis.obs_dis_front.obs_dis>1)                    //如果超过目标点1m,结束避障模式
    // {
    //     obs_flaging = 0;
    //     obs_flaging1 = 0;
    // }
    
}
/**
 * @brief 到达设定的点的判断
 */
void osb_arrive(void)
{

}
/**
 * @brief 判断到目标点时，转弯半径是否足够
 * 可等后期完善
 * 先采用小于30cm就后退
 */
void obs_turn_dis(void)
{

}
/**
 * @brief 判断有没有障碍物需要执行避障
 * @param  int obs_flag1        My Param doc
 * @param  obs_hps_t *p_obshps         My Param doc
 */
static void obs_judge(int obs_flag1,obs_hps_t *p_obshps)
{
    OBS_HPS_FLAG obs_hps_flag = HPS_NONE_FLAG;
	static int threshold_num = 0;
	if (obs_flag1==1)                                 //当等于1时说明第一组数据到来，可以判断减速或者停车
	{
		for (int i = 0; i < AREA_NUM/2; i++)
		{
			if (p_obshps->threshold_state[i*2]==1)
			{
				threshold_num ++;
                if (obs_hps_flag!=HPS_STOP_FLAG&&obs_hps_flag!=HPS_OBS_FLAG)
                {
                    obs_hps_flag = HPS_WARNING_FLAG;
                }
			}
			if (p_obshps->threshold_dist_avg[i*2]<obs_dis.obs_dis_front.warning_dis)
			{
                if (obs_hps_flag!=HPS_STOP_FLAG&&obs_hps_flag!=HPS_OBS_FLAG)
                {
                    obs_hps_flag = HPS_WARNING_FLAG;
                }
			}
			if (p_obshps->threshold_dist_avg[i*2]<obs_dis.obs_dis_front.stop_dis)
			{
				obs_hps_flag = HPS_STOP_FLAG;
			}
		}
	}
	else if (obs_flag1==2)
	{
		for (int i = 0; i < AREA_NUM/2; i++)
		{
			if (p_obshps->threshold_state[i*2+1]==1)
			{
				threshold_num ++;
			}
            if (p_obshps->threshold_dist_avg[i*2+1]<obs_dis.obs_dis_front.warning_dis)
            {
                if (obs_hps_flag!=HPS_STOP_FLAG&&obs_hps_flag!=HPS_OBS_FLAG)
                {
                    obs_hps_flag = HPS_WARNING_FLAG;
                }
            }
            if (p_obshps->threshold_dist_avg[i*2+1]<obs_dis.obs_dis_front.obs_dis)
            {
                if (obs_hps_flag!=HPS_STOP_FLAG)
                {
                    obs_hps_flag = HPS_OBS_FLAG;
                }
            }
            if (p_obshps->threshold_dist_avg[i*2+1]<obs_dis.obs_dis_front.stop_dis)
            {
                obs_hps_flag = HPS_STOP_FLAG;   
                break;                                                      //直接进入停车模式
            }
		}
        if (threshold_num>0)
        {
            threshold_num = 0;
            if (obs_hps_flag!=HPS_STOP_FLAG)
            {
                obs_hps_flag = HPS_OBS_FLAG;
            }
        }
        else
        {
            obs_hps_flag = HPS_NONE_FLAG;
            threshold_num = 0;
        }
	}
    p_obshps->obs_hps_flag = obs_hps_flag;
}



