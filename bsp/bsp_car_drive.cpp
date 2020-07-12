/***************************************
 * version V1.0
 * receive data of const lens 
 * buff[0] buff[1]  buff[2] buff[3] ->buff[10]  buff[11] 
 * head    order    lens                    data           check
 * 0xAA    0x01     30(0x1E)                                         和较验，只取低8位
 *
 * 
 * *************************************/
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "bsp_car_drive.h"
#include "config.h"

#define DATA_LEN  34
cardrive_info_t cardrive_info;
//接收STM32数据
int handle_stm32(unsigned char *ucData,int usLength)
{
    static long wheel_differ_count_l=0,wheel_differ_count_r=0;
	static unsigned char chrTemp[1000];
	static unsigned char ucRxCnt = 0;
	static unsigned short usRxLength = 0;
	unsigned char constsum=0;
	int count=0;
	if (usRxLength<=DATA_LEN)   //避免处理错误导致数组越界
	{
		memcpy(chrTemp+usRxLength,ucData,usLength);
		usRxLength += usLength;
	}
	else
	{
		memcpy(chrTemp,ucData,usLength);
		usRxLength += usLength;
		printf("stm32 handle receive error\n");
		perror("stm32 handle receive error\n");
	}
    while (usRxLength >= DATA_LEN)
    {
        if (chrTemp[0]!=0xAA&&chrTemp[1]!=0x01&&chrTemp[2]!=DATA_LEN-4)
        {
			usRxLength--;
			printf("car_drive head error\n");
			memcpy(&chrTemp[0],&chrTemp[1],usRxLength);                        
            continue;    //continue 退出这次循环执行下次
        }
        for(int i=0;i<DATA_LEN-1;i++)
        {
        	constsum += chrTemp[i];
        }
        //printf("cons: %d,chrTemp[25] is: %d\n",constsum,chrTemp[DATA_LEN-1]);
        if(constsum!=chrTemp[DATA_LEN-1])
        {
			usRxLength--;
			printf("check error\n");
			memcpy(&chrTemp[0],&chrTemp[1],usRxLength);  
			continue;
        }
        //接收数据
        cardrive_info.wheel_L = (chrTemp[3]|(unsigned int)chrTemp[4]<<8|(unsigned int)chrTemp[5]<<16|(unsigned int)chrTemp[6]<<24);
        cardrive_info.wheel_R = (chrTemp[7]|(unsigned int)chrTemp[8]<<8|(unsigned int)chrTemp[9]<<16|(unsigned int)chrTemp[10]<<24);
        cardrive_info.wheel_time_L = (chrTemp[11]|(unsigned short)chrTemp[12]<<8);
        cardrive_info.wheel_time_R = (chrTemp[13]|(unsigned short)chrTemp[14]<<8);
        cardrive_info.ultrasonic[0] = (chrTemp[15]|(unsigned short)chrTemp[16]<<8);
        cardrive_info.ultrasonic[1] = (chrTemp[17]|(unsigned short)chrTemp[18]<<8);
        cardrive_info.ultrasonic[2] = (chrTemp[19]|(unsigned short)chrTemp[20]<<8);
        cardrive_info.ultrasonic[3] = (chrTemp[21]|(unsigned short)chrTemp[22]<<8);
        cardrive_info.ultrasonic[4] = (chrTemp[23]|(unsigned short)chrTemp[24]<<8);
        cardrive_info.ultrasonic[5] = (chrTemp[25]|(unsigned short)chrTemp[26]<<8);
        cardrive_info.battery_Vdd = (chrTemp[27]|(unsigned short)chrTemp[28]<<8);
        cardrive_info.battery_AH = (chrTemp[29]|(unsigned short)chrTemp[30]<<8);
        cardrive_info.car_angle  = (chrTemp[31]|(unsigned short)chrTemp[32]<<8);
        if (wheel_differ_count_l!=0||wheel_differ_count_r!=0)
        {
            cardrive_info.wheel_differ_L = cardrive_info.wheel_L - wheel_differ_count_l;
            cardrive_info.wheel_differ_R = wheel_differ_count_r - cardrive_info.wheel_R;
        }
        wheel_differ_count_l = cardrive_info.wheel_L;
        wheel_differ_count_r = cardrive_info.wheel_R;
        //printf("head is: %d\n",chrTemp[0]);
		usRxLength -= DATA_LEN;
		constsum=0;
		memcpy(&chrTemp[0],&chrTemp[DATA_LEN],usRxLength);
    }
    return usRxLength;
}
int select_mode(int fd_x,char car_mode)
{
    //跟随，停止，伴行，手动
    const unsigned char send_data[4][4]={{0x55,0x01,0x00,0x56},\
                                   {0x55,0x02,0x00,0x57},\
                                   {0x55,0x03,0x00,0x58},\
                                   {0x55,0x04,0x00,0x59}};
    int len=0,n=0;
    unsigned char send_data1[4]={0};
    switch (car_mode)
    {
        case 0x01:
        memcpy(send_data1,send_data[0],sizeof(send_data[0]));  break;
        case 0x02:
        memcpy(send_data1,send_data[1],sizeof(send_data[1]));  break;
        case 0x03:
        memcpy(send_data1,send_data[2],sizeof(send_data[2]));  break;
        case 0x04:
        memcpy(send_data1,send_data[3],sizeof(send_data[3]));  break;
        default:
            printf("car_mode select error %d\n",car_mode);
            perror("car_mode select error \n");
            break;
    }
    while(1)
    {
        len += write(fd_x,send_data1+len,sizeof(send_data1)-len);
        n++; 
        if(sizeof(send_data1)==len)
        {
            break;
        }
        else if(n>=10)       //写10次长度还是不对，则报错
        {
            printf("write error len: %d,car_mode is: %d\n",len,car_mode);
            return -1;
        }
    }
    return len;
}
//遥控模式
int remote_control(PORT_NAME name,const float angle,const float speed)
{
    unsigned char send_data[50] = {0};
    unsigned char check = 0; 
    int angle1 = (int)(angle*10);
    int speed1 = (int)(speed*100);
    int cnt = 0;
    send_data[cnt++] = 0x55;
    send_data[cnt++] = 0x11;
    send_data[cnt++] = 0;
    send_data[cnt++] = (unsigned char)angle1;
    send_data[cnt++] = (unsigned char)(angle1>>8);
    send_data[cnt++] = (unsigned char)(angle1>>16);
    send_data[cnt++] = (unsigned char)(angle1>>24);
    send_data[cnt++] = (unsigned char)speed1;
    send_data[cnt++] = (unsigned char)(speed1>>8);
    send_data[cnt++] = (unsigned char)(speed1>>16);
    send_data[cnt++] = (unsigned char)(speed1>>24);
    send_data[2] = cnt-3;
    for (int i = 0; i < cnt; i++)
    {
        check += send_data[i];
    }
    send_data[cnt++] = check;
    return bsp_write_array(name,send_data,cnt);
}
