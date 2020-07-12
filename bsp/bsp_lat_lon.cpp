/**********************************************************************
*https://blog.csdn.net/gatieme/article/details/45599581
*纬度差一度就简单了，因为经线是大圆，和纬度无关 = πR/180=111.319491 km
*准确的角度距离计算，采用下面的github的
*https://github.com/TengMichael/ROS_Intell_Driving/blob/5a50da5e5aca6c5b9314edbeded2816869214143/src/navi_posi/include/global2local.h
**********************************************************************/
#include "stdio.h"
#include <math.h>
#include "bsp_pid.h"
#include "bsp_lat_lon.h"
#include "config.h"
extern gpslog_addr_t gpslog_addr[];
extern PID_IncTypeDef PID_Data;
/* 经纬度转为平面坐标xy ----------------------------------- */
//转换成弧度
double Rad(double d)
{
  return d * PI / 180.0;
}
double round(double d){
  return floor(d + 0.5);
}
/**  
 * @brief  经纬度解析角度,速度
 * @param  p_gps            My Param doc
 * @param  p_gpslog         My Param doc
 * @param  p_control_info   My Param doc
 * @return int 
 */
int lat_lon(const gps_info_t *p_gps,const gpslog_addr_t *p_gpslog,control_info_t *p_control_info)
{
    double goal_x=0,goal_y=0;
    //  goal_x = gpslog_addr[p_gps->index.goal_index].Lattitude;
    //  goal_y = gpslog_addr[p_gps->index.goal_index].Longitude;
    if (p_gps->index.goal_index >=1)
    {
        for (int i = 0; i < 3; i++)
        {
            goal_x += p_gpslog[p_gps->index.goal_index+i-1].Lattitude;
            goal_y += p_gpslog[p_gps->index.goal_index+i-1].Longitude;
        }
        goal_x = goal_x/3.f;
        goal_y = goal_y/3.f;
    }
    else
    {
      goal_x = p_gpslog[p_gps->index.goal_index].Lattitude;
      goal_y = p_gpslog[p_gps->index.goal_index].Longitude;
    }
    double s = gps_get_distance(p_gps->addr.Lattitude,p_gps->addr.Longitude,goal_x,goal_y);
    double angle = gps_getAngle(p_gps->addr.Lattitude,p_gps->addr.Longitude,goal_x,goal_y);
    double angle1 = p_gps->addr.Heading;
    p_control_info->dis = s;
    if (angle1>=360)
    {
      angle1 = angle1-360;
    }
    printf("两地计算偏角：%lf\n",angle);
    printf("航向计算偏角：%lf\n",angle1);
    double angle2 = angle - angle1;  //负值左转，正值右转 
    if (angle2>180)                  //由于偏差角度大于180,所以要朝另外一个方向转，即朝左，朝左值要为负
    {
        angle2 = -360+angle2;
    }
    if (angle2<-180)
    {
       angle2 = 360+angle2;
    }
    p_control_info->speed = s/120.f;  //60cm  0.5m/s
    //angle2 = PID_Inc(angle2, &PID_Data);
    if (angle2<-GPS_CAR_ANGLE)
    {
        angle2=-GPS_CAR_ANGLE;
    }
    if (angle2>GPS_CAR_ANGLE)
    {
        angle2=GPS_CAR_ANGLE;
    }
    p_control_info->angle = - angle2;//然而舵机是左转需要增加，所以前面要加负号
    printf("两地直线距离: %lf\n", p_control_info->dis);
    printf("两地的夹角:% lf\n",angle2);
}
//根据经纬度计算两点角度
double gps_getAngle(double lat_a, double lng_a, double lat_b, double lng_b) 
{
  lat_a = Rad(lat_a);
  lng_a = Rad(lng_a);
  lat_b = Rad(lat_b);
  lng_b = Rad(lng_b);
  double x = sin(lng_b-lng_a) * cos(lat_b);
  double y = cos(lat_a)*sin(lat_b) - sin(lat_a)*cos(lat_b)*cos(lng_b-lng_a);  
  // double bearing = atan2(y,x);  
  // bearing = bearing*180/PI;

  double bearing;
    if (x >=  0 && y >= 0) {
    bearing = atan2(x, y)*180/PI;
  }
  else if(x >=0 && y<=0){
    bearing = atan2(x, y)*180/PI;
  }
  else if (x <= 0 && y <= 0) {
    bearing = atan2(x, y)*180/PI+360;
  }
  else {
    bearing = atan2(x, y)*180/PI+180*2;
  }
  printf("x:%lf,y:%lf,atan2:%lf\n",x,y,atan2(x, y)*180/PI);
  // if (x >=  0 && y >= 0) {
  //   bearing = atan2(x, y)*180/PI;
  // }
  // else if(x >=0 && y<=0){
  //   bearing = atan2(x, y)*180/PI+90;
  // }
  // else if (x <= 0 && y <= 0) {
  //   bearing = atan2(x, y)*180/PI+180*2;
  // }
  // else {
  //   bearing = atan2(x, y)*180/PI+180*2;
  // }
  return bearing;  
}
//根据经纬度计算两点距离，返回值单位为cm
double gps_get_distance(double lat1, double lng1, double lat2, double lng2)
{
   double radLat1 = Rad(lat1);
   double radLat2 = Rad(lat2);
   double a = radLat1 - radLat2;
   double b = Rad(lng1) - Rad(lng2);
   double s = 2 * asin(sqrt(pow(sin(a/2),2) +cos(radLat1)*cos(radLat2)*pow(sin(b/2),2)));
   //s = round(s * 10000) / 10000.f;
   s = s * EARH_R*100000;   //单位cm
   return s;
}


