#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include "bsp_tag.h"
#include "bsp_tag_xy.h"
#include "bsp_uart.h"
#define  DATA_LEN                   19          //UWB接收最小长度,单位字节
#define  UWB_DIFF_TIME              50          //uwb差分最小时间差，单位ms

int tag_flag;   
uwb_info_t uwb_info;

//新的标签数据接收
int receive_tag(unsigned char *ucData,int usLength,int *usRxLength,long timer,int uwb_num)
{
    static long timer1,timer2;
    static int timer1_flag=0,timer2_flag=0,timer_count=0;
	unsigned char look[1000]={0};
	memcpy(look,ucData,*usRxLength);
    int num = 0;
    if (usLength==9)    //接收到POS:N/A
    {
        *usRxLength = *usRxLength-9;
    }
    else if (*usRxLength<DATA_LEN)
    {
        printf("usRxLength:%d\n",*usRxLength);
        printf("tag receive error\n");
    }
    while (*usRxLength >= DATA_LEN)
    {
		if (ucData[0]!='P'||ucData[1]!='O'||ucData[2]!='S')
        {
			*usRxLength = *usRxLength-1;
			// printf("usRxLength:%d\n",*usRxLength);
			// printf("tag head error\n");
			memcpy(&ucData[0],&ucData[1],*usRxLength);
            continue;                      //continue 退出这次循环执行下次
        }
        for (num = 0; num < *usRxLength-1; num++)
        {
            if ((ucData[num] == 0x0D)&&(ucData[num+1] == 0x0A))
            {
                break;
            }
        }  //搜索能找到\r\n,说明可以采集信息
        if(num == *usRxLength-1)
        {
			*usRxLength = *usRxLength-1;
			printf("tag check error\n");
			memcpy(&ucData[0],&ucData[1],*usRxLength);
			continue;
        }
        //printf("POS quality:%d\n",p_uwb_addr[0].quality);
        if (uwb_num==UWB_BACK)
        {
            sscanf((const char*)ucData,"POS:%f,%f,%f,%c\r\n",&uwb_info.addr[UWB_BACK].x,&uwb_info.addr[UWB_BACK].y,&uwb_info.addr[UWB_BACK].z,&uwb_info.addr[UWB_BACK].quality);
            timer1 = timer;
            timer1_flag = 1;
            timer_count = 1;
            tag_flag = 1;           //后面的uwb来数据
        }
        else if (uwb_num==UWB_FRONT)
        {
            sscanf((const char*)ucData,"POS:%f,%f,%f,%c\r\n",&uwb_info.addr[UWB_FRONT].x,&uwb_info.addr[UWB_FRONT].y,&uwb_info.addr[UWB_FRONT].z,&uwb_info.addr[UWB_FRONT].quality);
            timer2 = timer;
            timer2_flag = 1;
            timer_count = 2;
            tag_flag = 2;    //前面的uwb来数据就分析
        }
        if (timer1_flag&&timer2_flag)
        {
            if (abs(timer1-timer2)<UWB_DIFF_TIME)
            {
                tag_flag = 3;    //两个uwb来数据就分析角度
                timer1_flag = 0;
                timer2_flag = 0;
            }
            else
            {
                if (timer_count==1)
                {
                    timer2_flag = 0;
                }
                else
                {
                    timer1_flag = 0;
                }
            }
        }
        //printf("UWB_BACK:%f\n",uwb_info.addr[UWB_FRONT].x);
        if (*usRxLength>=num+2)
        {
            *usRxLength -= num+2;              //*后面还有两位，所以要加2
        }
		memcpy(&ucData[0],&ucData[num+2],*usRxLength);
    }
    return *usRxLength;
}
//发送uwb命令相关数据
int tag_send_lep(int fd_x)
{
    unsigned char send_data[100]={0};
    int len=0;
    int cnt = 0,check=0;
    send_data[cnt++] = 'l';
    send_data[cnt++] = 'e';
    send_data[cnt++] = 'p';
    send_data[cnt++] = 0x0D;
    len = bsp_write_array(BLE,send_data,cnt);
    return len;
}
//发送0d0d命令相关数据
int tag_send_0D0D(int fd_x)
{
    unsigned char send_data[100]={0};
    int len = 0;
    int cnt = 0;
    send_data[cnt++] = 0x0D;
    send_data[cnt++] = 0x0D;
    len = bsp_write_array(TAG1,send_data,cnt);
    return len;
}

