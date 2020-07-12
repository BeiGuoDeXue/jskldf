#ifndef __BSP_LIDAR_H__
#define __BSP_LIDAR_H__
#include "config.h"


int lidar_handle(unsigned char *buff,const int len, int *start);
int lidar_send(PORT_NAME name);



#endif 