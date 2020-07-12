#include<stdio.h>
#include<string.h>
#include <stdlib.h>
#include "bsp_readfile.h"

uwblog_addr_t uwblog_addr[100000];
gpslog_addr_t gpslog_addr[100000];

extern long UWB_LOG_MAXNUM ;
extern long GPS_LOG_MAXNUM ;
//读取UWB log数据
int readfile_uwb(void)
{
	FILE *fin;
	long i=0;
	fin = fopen("uwb_log/uwb_log7.txt","r");
	if (0==fin) {
	printf("Can not open file uwb_log.txt!\n");
	return 0;
    }
	for (i=0;!feof(fin);i++)
	{
		fscanf(fin,"POS,%f,%f,%f,%d\n",&uwblog_addr[i].x,&uwblog_addr[i].y,&uwblog_addr[i].z,&uwblog_addr[i].quality);
		//printf("%f,%f,%f,%d\n",uwblog_addr[i].x,uwblog_addr[i].y,uwblog_addr[i].z,uwblog_addr[i].quality);
	}
	UWB_LOG_MAXNUM = i;
	// for (int i=0;fscanf(fin,"POS,%f,%f,%f,%d\n",&log_pos[i].x,&log_pos[i].y,&log_pos[i].z,&log_pos[i].quality)!=0;i++)
	// {
	// 	printf("%f,%f,%f,%d\n",log_pos[i].x,log_pos[i].y,log_pos[i].z,log_pos[i].quality);
	// }
	fclose(fin);
	return 1;
}
//读取gps log数据，全部读取
int readfile_gps(void)
{
	FILE *fin;
	long i = 0;
	fin = fopen("gps_log/gps9.txt","r");
	if (0==fin) {
	printf("Can not open file gps.txt!\n");
	return 0;
    }
	 for (i = 0;!feof(fin); i++)   //读取到文件结束
	//for (i = 0;i<100000; i++)
	{
		// fscanf(fin,"$GPCHC,%d,%f,",&GPSWeek[i],&GPSTime[i]);
		// printf("%d,%f\n",GPSWeek[i],GPSTime[i]);
		fscanf(fin,"$GPCHC,%d,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%lf,%lf,%lf,%f,%f,%f,%f,%d,%d,%d,%d,%c*%x\n",\
		&gpslog_addr[i].GPSWeek,&gpslog_addr[i].GPSTime,&gpslog_addr[i].Heading,&gpslog_addr[i].pitch,\
		&gpslog_addr[i].roll,&gpslog_addr[i].gyro_x,&gpslog_addr[i].gyro_y,&gpslog_addr[i].gyro_z,\
		&gpslog_addr[i].acc_x,&gpslog_addr[i].acc_y,&gpslog_addr[i].acc_z,&gpslog_addr[i].Lattitude,\
		&gpslog_addr[i].Longitude,&gpslog_addr[i].Altitude,&gpslog_addr[i].Ve,&gpslog_addr[i].Vn,\
		&gpslog_addr[i].Vu,&gpslog_addr[i].Baseline,&gpslog_addr[i].NSV1,&gpslog_addr[i].NSV2,\
		&gpslog_addr[i].Status,&gpslog_addr[i].Age,&gpslog_addr[i].Warming,&gpslog_addr[i].Cs\
		);
		// printf("$GPCHC,%d,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%d,%d,%d,%d,%d*%x\n",\
		// gpslog_addr[i].GPSWeek,gpslog_addr[i].GPSTime,gpslog_addr[i].Heading,gpslog_addr[i].pitch,\
		// gpslog_addr[i].roll,gpslog_addr[i].gyro_x,gpslog_addr[i].gyro_y,gpslog_addr[i].gyro_z,\
		// gpslog_addr[i].acc_x,gpslog_addr[i].acc_y,gpslog_addr[i].acc_z,gpslog_addr[i].Lattitude,\
		// gpslog_addr[i].Longitude,gpslog_addr[i].Altitude,gpslog_addr[i].Ve,gpslog_addr[i].Vn,\
		// gpslog_addr[i].Vu,gpslog_addr[i].Baseline,gpslog_addr[i].NSV1,gpslog_addr[i].NSV2,\
		// gpslog_addr[i].Status,gpslog_addr[i].Age,gpslog_addr[i].Warming,gpslog_addr[i].Cs\
		// );
	}
	GPS_LOG_MAXNUM = i;
	// for (int i=0;fscanf(fin,"$GPCHC,%d,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%d,%d,%d,%d,*%d\n",\
	// &gpslog_addr[i].GPSWeek,&gpslog_addr[i].GPSTime,&gpslog_addr[i].Heading,&gpslog_addr[i].Pitch,\
	// &gpslog_addr[i].Roll,&gpslog_addr[i].gyro_x,&gpslog_addr[i].gyro_y,&gpslog_addr[i].gyro_z,\
	// &gpslog_addr[i].acc_x,&gpslog_addr[i].acc_y,&gpslog_addr[i].acc_z,&gpslog_addr[i].Lattitude,\
	// &gpslog_addr[i].Longitude,&gpslog_addr[i].Altitude,&gpslog_addr[i].Ve,&gpslog_addr[i].Vn,\
	// &gpslog_addr[i].Vu,&gpslog_addr[i].Baseline,&gpslog_addr[i].NSV1,&gpslog_addr[i].NSV2,\
	// &gpslog_addr[i].Status,&gpslog_addr[i].Age,&gpslog_addr[i].Warming,&gpslog_addr[i].Cs\
	// )!=0;i++)
	// {
	// 	printf("$GPCHC,%d,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%d,%d,%d,%d,*%d\n",\
	// 	&gpslog_addr[i].GPSWeek,&gpslog_addr[i].GPSTime,&gpslog_addr[i].Heading,&gpslog_addr[i].Pitch,\
	// 	&gpslog_addr[i].Roll,&gpslog_addr[i].gyro_x,&gpslog_addr[i].gyro_y,&gpslog_addr[i].gyro_z,\
	// 	&gpslog_addr[i].acc_x,&gpslog_addr[i].acc_y,&gpslog_addr[i].acc_z,&gpslog_addr[i].Lattitude,\
	// 	&gpslog_addr[i].Longitude,&gpslog_addr[i].Altitude,&gpslog_addr[i].Ve,&gpslog_addr[i].Vn,\
	// 	&gpslog_addr[i].Vu,&gpslog_addr[i].Baseline,&gpslog_addr[i].NSV1,&gpslog_addr[i].NSV2,\
	// 	&gpslog_addr[i].Status,&gpslog_addr[i].Age,&gpslog_addr[i].Warming,&gpslog_addr[i].Cs);
	// 	//printf("%f,%f,%f,%d\n",log_pos[i].x,log_pos[i].y,log_pos[i].z,log_pos[i].quality);
	// }
	fclose(fin);
	return 1;
}


