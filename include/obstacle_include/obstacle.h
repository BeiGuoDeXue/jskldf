#ifndef __BSP_OBSTACLE_H__
#define __BSP_OBSTACLE_H__

#define OBS_WIDTH               0.7f     //车宽0.5,两边各增加0.1m
#define OBS_AREA_WIDTH          9       //可以选做口字的区域

#define OBS_BACK_DIS            0.3f

/**
 * @brief 避障距离的类型
 */
typedef struct 
{
    float warning_dis;
    float obs_dis;
    float stop_dis;
}obs_dis_type_t;
/**
 * @brief 避障距离的方位
 */
typedef struct
{
    obs_dis_type_t obs_dis_front;
    obs_dis_type_t obs_dis_back;
    obs_dis_type_t obs_dis_left;
    obs_dis_type_t obs_dis_right;
}obs_dis_t;

/**
 * @brief 区域的相关数据
 */
typedef struct
{
    int num;                        //每个大有多少个没障碍物连续的小区域
    int start_area;                 //大区域的起始区域号
    int end_area;                   //大区域的结束区域号
    float dis[3];                      //大区域的障碍物平均距离
    float angle[3];                    //大区域的角度
    double lat[3];                     //大区域的纬度
    double lon[3];                     //大区域的经度
}obs_area_t;

void obs_init(void);
void obs_control(int obs_flag1);

#endif


