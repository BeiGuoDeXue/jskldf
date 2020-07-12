#ifndef __BSP_TAG_H__
#define __BSP_TAG_H__
#include "config.h"
typedef struct
{
    float x;
    float y;
    float z;
    unsigned char quality;
}addr_t;
typedef struct
{
    int status[2];
    addr_t addr[2];
    addr_t center;
    index_t index;
}uwb_info_t;

int receive_tag(unsigned char *ucData,int usLength,int *usRxLength,long timer,int uwb_num);
int tag_send_lep(int fd_x);


#endif
