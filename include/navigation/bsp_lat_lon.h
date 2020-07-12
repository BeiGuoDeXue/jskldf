#ifndef __BSP_LAT_LON_H__
#define __BSP_LAT_LON_H__

#include "bsp_gps.h"
#include "bsp_readfile.h"
#include "app.h"

double Rad(double d);
// int lat_lon(const _gps_pos *p_gps_pos,const gpslog_addr_t *p_gps_log_pos,control_info_t *p_control_info,const _gps_index_pos *p_gps_index_pos);
int lat_lon(const gps_info_t *p_gps,const gpslog_addr_t *p_gpslog,control_info_t *p_control_info);
double gps_getAngle(double lat_a, double lng_a, double lat_b, double lng_b);
double gps_get_distance(double lat1, double lng1, double lat2, double lng2);

#endif 

