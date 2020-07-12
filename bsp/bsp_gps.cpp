#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "bsp_gps.h"
int gps_flag;
gps_info_t gps_info;
#define GPS_RECEIVE_LINE  131   //最小也要有这么多数据
//串口接收GPS数据处理函数
int handle_gps(unsigned char *ucData,int usLength)
{
	static unsigned char chrTemp[1000];	
	static unsigned short usRxLength = 0;
    int num=0;
    memcpy(chrTemp+usRxLength,ucData,usLength);
    usRxLength += usLength;
    while (usRxLength >= GPS_RECEIVE_LINE)
    {
        if (chrTemp[0]!='$'||chrTemp[1]!='G'||chrTemp[2]!='P'||chrTemp[3]!='C'||chrTemp[4]!='H')
        {
			usRxLength--;
			printf("GPS head error\n");
			memcpy(&chrTemp[0],&chrTemp[1],usRxLength);
            continue;                      //continue 退出这次循环执行下次
        }
        for (num = 0; num < usRxLength; num++)  
        {
            if (chrTemp[num]=='*')
            {
                break;
            }
        }  //搜索能找到*,说明可以采集信息
        if(num==usRxLength)
        {
			usRxLength--;
			printf("check error\n");
			memcpy(&chrTemp[0],&chrTemp[1],usRxLength);  
			continue;
        }
        //接收数据 待处理
		sscanf((const char*)chrTemp,"$GPCHC,%d,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%lf,%lf,%lf,%f,%f,%f,%f,%d,%d,%d,%d,%c*%c\n",\
		&gps_info.addr.GPSWeek,&gps_info.addr.GPSTime,&gps_info.addr.Heading,&gps_info.addr.pitch,\
		&gps_info.addr.roll,&gps_info.addr.gyro_x,&gps_info.addr.gyro_y,&gps_info.addr.gyro_z,\
		&gps_info.addr.acc_x,&gps_info.addr.acc_y,&gps_info.addr.acc_z,&gps_info.addr.Lattitude,\
		&gps_info.addr.Longitude,&gps_info.addr.Altitude,&gps_info.addr.Ve,&gps_info.addr.Vn,\
		&gps_info.addr.Vu,&gps_info.addr.Baseline,&gps_info.addr.NSV1,&gps_info.addr.NSV2,\
		&gps_info.addr.Status,&gps_info.addr.Age,&gps_info.addr.Warming,&gps_info.addr.Cs\
		);
        gps_flag =1;
        if (usRxLength>=num+1+4)
        {
            usRxLength -= num+1+4;              //*后面还有两位，所以要加2
        }
		memcpy(&chrTemp[0],&chrTemp[num+1+4],usRxLength);
    }
    return usRxLength;
}