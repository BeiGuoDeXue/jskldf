#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include "bsp_lidar.h"
#include <unistd.h>
#include <termios.h>
#include "config.h"
#define CIRCLE_COUNT 1450
struct _Lidar
{
    char cQuality;
    unsigned short iAngle;
    unsigned short iDistance;
};
struct _Lidar Lidar_Buff[CIRCLE_COUNT];
//lidar数据接收处理
int lidar_handle(unsigned char *buff,const int len, int *start)
{
    static int LidarCnt=0;
    int count=0;
    int old_len1 =0;
    static int old_len=0;
    static unsigned char buff1[10]={0};
    unsigned char buff_look[1000]={0};
    if (*start==0)
    {
        for (int i = 0; i < len-2; i++)
        {
            if (buff[i]==0xA5&&buff[i+1]==0x5A&&buff[i+2]==0x05)
            {
                *start = 1;
                printf("data receive starts: %d\r\n",len);
            }
        }
    }
    if (*start)
    {
        memcpy(buff_look,buff,len);
        old_len1=old_len;
        for (int i = 0; i < old_len; i++)
        {
        if (LidarCnt>=CIRCLE_COUNT)           LidarCnt = 0;
        switch (old_len1)
        {
            case 4:
                if ((((buff1[i]>>1)&0x01)==((~buff1[i])&0x01))&&(buff1[i+1]&0x01))
                {
                    Lidar_Buff[LidarCnt].cQuality = (buff1[i]>>2);
                    Lidar_Buff[LidarCnt].iAngle = (buff1[i+2]<<7) + (buff1[i+1]>>1);
                    Lidar_Buff[LidarCnt++].iDistance = (buff[0]<<8)+buff1[i+3];
                    old_len1 -= 5;
                }
                else   old_len1--;
                break;
            case 3:
                if ((((buff1[i]>>1)&0x01)==((~buff1[i])&0x01))&&(buff1[i+1]&0x01))
                {
                    Lidar_Buff[LidarCnt].cQuality = (buff1[i]>>2);
                    Lidar_Buff[LidarCnt].iAngle = (buff1[i+2]<<7) + (buff1[i+1]>>1);
                    Lidar_Buff[LidarCnt++].iDistance = (buff[1]<<8)+buff[0];
                    old_len1 -= 5;
                }
                else   old_len1--;
                break;
            case 2:
                if ((((buff1[i]>>1)&0x01)==((~buff1[i])&0x01))&&(buff1[i+1]&0x01))
                {
                    Lidar_Buff[LidarCnt].cQuality = (buff1[i]>>2);
                    Lidar_Buff[LidarCnt].iAngle = (buff[0]<<7) + (buff1[i+1]>>1);
                    Lidar_Buff[LidarCnt++].iDistance = (buff[2]<<8)+buff[1];
                    old_len1 -= 5;
                }
                else   old_len1--;
                break;
            case 1:
                if ((((buff1[i]>>1)&0x01)==((~buff1[i])&0x01))&&(buff[0]&0x01))
                {
                    Lidar_Buff[LidarCnt].cQuality = (buff1[0]>>2);
                    Lidar_Buff[LidarCnt].iAngle = (buff[1]<<7) + (buff[0]>>1);
                    Lidar_Buff[LidarCnt++].iDistance = (buff[3]<<8)+buff[2];
                    old_len1 -= 5;
                }
                else   old_len1--;
                break;
            default:
                break;
        }
        }
        for (count = abs(old_len1); count <= len-5;)
        //for (count =0; count <= len-5;)
        {
            // char a = (buff[count]>>1)&0x01;
            // char b = (~buff[count])&0x01;
            // char c = buff[count+1]&0x01;
            if ((((buff[count]>>1)&0x01)==((~buff[count])&0x01))&&(buff[count+1]&0x01))
            //if(a==b&&c)
            {
                if (LidarCnt>=CIRCLE_COUNT)           LidarCnt = 0;
                Lidar_Buff[LidarCnt].cQuality = (buff[count]>>2);
                Lidar_Buff[LidarCnt].iAngle = ((unsigned short)buff[count+2]<<7)|(buff[count+1]>>1);
                Lidar_Buff[LidarCnt].iDistance = ((unsigned short)buff[count+4]<<8)|buff[count+3];
                // printf("buff[count+4]: %02x,buff[count+3]: %02X\n",buff[count+4],buff[count+3]);
                // printf("Qua: %d,Angle: %d,Dis: %d,DisHex: %04X\n",Lidar_Buff[LidarCnt].cQuality,Lidar_Buff[LidarCnt].iAngle,Lidar_Buff[LidarCnt].iDistance,Lidar_Buff[LidarCnt].iDistance);
                count+=5;
                LidarCnt++;
            }
            else
            {
                count+=1;
            }
        }
        old_len = len-count;
        if (old_len>0)
        {
            memcpy(buff1,buff+count,old_len); 
        }
    }
}
//lidar send data，发送此命令，lidar开始工作
int lidar_send(PORT_NAME name)
{
    unsigned char send_data[2]={0};
    int cnt = 0;
    send_data[cnt++] = 0xA5;
    send_data[cnt++] = 0x20;
    return bsp_write_array(name,send_data,cnt);
}