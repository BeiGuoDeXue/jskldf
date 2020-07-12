#ifndef __BSP_READFILE_H__
#define __BSP_READFILE_H__

typedef struct 
{
    float x;
    float y;
    float z;
    int quality;
}uwblog_addr_t;
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
}gpslog_addr_t;

int readfile_uwb(void);
int readfile_gps(void);

#endif
