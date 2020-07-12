#ifndef __BSP_GPS_H__
#define __BSP_GPS_H__
#include "config.h"

typedef struct 
{
    int GPSWeek;
    float GPSTime;
    float Heading;
    float pitch;
    float roll;
    float gyro_x;
    float gyro_y;
    float gyro_z;
    float acc_x;
    float acc_y;
    float acc_z;
    double Lattitude;
    double Longitude;
    double Altitude;
    float Ve;
    float Vn;
    float Vu;
    float Baseline;
    int NSV1;
    int NSV2;
    int Status;
    int Age;
    char Warming;
    char Cs;
}gps_addr_t;
typedef struct 
{
    int status;
    gps_addr_t addr;
    index_t index;
}gps_info_t;

int handle_gps(unsigned char *ucData,int usLength);


#endif