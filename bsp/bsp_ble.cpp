/********************************************************
 *                蓝牙指令介绍   *是结束符
 *                A1,20,10*   遥控：1代表下发角度和速度，20是20度，10是0.1m/s
 *                A2,2000*    导航：2代表导航模式，2000是导航的目标坐标log索引值
 *                A3*         停止：3代表停止
 *********************************************************/
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include "bsp_ble.h"
#include "app.h"
#include "bsp_readfile.h"
#include "bsp_gps.h"

#define BLE_RECEIVE_LEN 4

extern long GPS_LOG_MAXNUM;

//send_uwb数据
extern uwb_info_t uwb_info;
extern uwblog_addr_t uwblog_addr[];
//send_gps数据
extern gps_info_t gps_info;
extern gpslog_addr_t gpslog_addr[];
//用户发送信息
extern user_order_t userorder_info;
double look_angle,look_filter_angle;
//蓝牙数据
ble_t ble_info;
//蓝牙接收数据处理
int handle_ble(unsigned char *ucData,int usLength)
{
    static int start_flag = 0;
    static CAR_MODE_t default_carmode = TAG_NAVIGATION;
	static unsigned char chrTemp[100];
	static unsigned short usRxLength = 0;
    int num = 0;
    memcpy(chrTemp+usRxLength,ucData,usLength);
    usRxLength += usLength;
    while (usRxLength >= BLE_RECEIVE_LEN)
    {
        if (chrTemp[0]!='A'||chrTemp[0]!='a'||chrTemp[1]!='0'||chrTemp[1]!='1'||chrTemp[1]!='2'||chrTemp[1]!='3'\
        ||chrTemp[1]!='4'||chrTemp[1]!='5'||chrTemp[1]!='6'||chrTemp[1]!='7')
        {
			usRxLength--;
			printf("ble head error\n");
			memcpy(&chrTemp[0],&chrTemp[1],usRxLength);                        
            continue;    //continue 退出这次循环执行下次
        }
        for (num = 3; num < usRxLength; num++)
        {
            if (chrTemp[num]=='*')
            {
                break;
            }
        }
         //搜索能找到*,说明可以采集信息
        if(num==usRxLength)
        {
			usRxLength--;
			printf("BLE check error\n");
			memcpy(&chrTemp[0],&chrTemp[1],usRxLength);  
			continue;
        }
        //接收数据 待处理
        switch (chrTemp[1])
        {
            case '7':                                           //开始
                ble_info.car_mode = default_carmode;            //默认是GPS导航
                userorder_info.car_mode = default_carmode;
                break;
            case '0':
                default_carmode = STOP_MODE;
                break;
            case '1':
                default_carmode = BLE_REMOTE;
                sscanf((const char*)(chrTemp+3),"%f,%f",&ble_info.ble_angle,&ble_info.ble_speed);
                break;
            case '2':
                default_carmode = SERVER_REMOTE;
                //sscanf((const char*)(chrTemp+3),"%ld",&ble_info.ble_goal);
                userorder_info.index_goal = ble_info.ble_goal;
                break;
            case '3':
                default_carmode = TAG_NAVIGATION;
                sscanf((const char*)(chrTemp+3),"%ld",&ble_info.ble_goal);
                userorder_info.index_goal = ble_info.ble_goal;
                break;
            case '4':
                default_carmode = GPS_NAVIGATION;
                break;
            case '5':
                default_carmode = FELLOW_MODE;
                break;
            case '6':
                default_carmode = ACCOMPANY_MODE;
                break;
            default:
                printf("\r\n模式选择错误: %c,请选择模式0-7\r\n",chrTemp[1]);
                break;
        }
        usRxLength -= num+1;
		memcpy(&chrTemp[0],&chrTemp[num+1],usRxLength);
    }
    return usRxLength;
}
//蓝牙发送uwb导航相关数据
int tag_send_ble(PORT_NAME name,const int angle,const int speed)
{
    unsigned char send_data[50]={0};
    int cnt = 0;
    unsigned char check = 0;
    send_data[cnt++] = 0xAA;
    send_data[cnt++] = 0xAA;
    send_data[cnt++] = 0xF1;
    send_data[cnt++] = 0;
    // send_data[cnt++] = (unsigned char)(angle>>24);
    // send_data[cnt++] = (unsigned char)(angle>>16);
    // send_data[cnt++] = (unsigned char)(angle>>8);
    // send_data[cnt++] = (unsigned)angle;
    // send_data[cnt++] = (unsigned char)(speed>>24);
    // send_data[cnt++] = (unsigned char)(speed>>16);
    // send_data[cnt++] = (unsigned char)(speed>>8);
    // send_data[cnt++] = (unsigned)speed;
    send_data[cnt++] = (unsigned char)((int)(uwb_info.center.x*100)>>8);
    send_data[cnt++] = (unsigned char)(uwb_info.center.x*100);
    send_data[cnt++] = (unsigned char)((int)(uwb_info.center.y*100)>>8);
    send_data[cnt++] = (unsigned char)(uwb_info.center.y*100);
    // send_data[cnt++] = (unsigned char)((int)(uwb_addr[0].z*100)>>8);
    // send_data[cnt++] = (unsigned char)(uwb_addr[0].z*100);
    // send_data[cnt++] = (unsigned char)((int)(uwb_addr[1].x*100)>>8);
    // send_data[cnt++] = (unsigned char)(uwb_addr[1].x*100);
    // send_data[cnt++] = (unsigned char)((int)(uwb_addr[1].y*100)>>8);
    // send_data[cnt++] = (unsigned char)(uwb_addr[1].y*100);
    // send_data[cnt++] = (unsigned char)((int)(uwb_addr[1].z*100)>>8);
    // send_data[cnt++] = (unsigned char)(uwb_addr[1].z*100);
    send_data[cnt++] = (unsigned char)((int)(look_angle*10)>>8);
    send_data[cnt++] = (unsigned char)((int)(look_angle*10));
    send_data[cnt++] = (unsigned char)((int)(look_filter_angle*10)>>8);
    send_data[cnt++] = (unsigned char)((int)(look_filter_angle*10));
    // send_data[cnt++] = (unsigned char)((int)(uwblog_addr[uwb_index_pos.self_index].x*100)>>8);
    // send_data[cnt++] = (unsigned char)(uwblog_addr[uwb_index_pos.self_index].x*100);
    // send_data[cnt++] = (unsigned char)((int)(uwblog_addr[uwb_index_pos.self_index].y*100)>>8);
    // send_data[cnt++] = (unsigned char)(uwblog_addr[uwb_index_pos.self_index].y*100);
    // send_data[cnt++] = (unsigned char)((int)(uwblog_addr[uwb_index_pos.self_index].z*100)>>8);
    // send_data[cnt++] = (unsigned char)(uwblog_addr[uwb_index_pos.self_index].z*100);
    send_data[3] = cnt - 4;
    for (int i = 0; i < cnt; i++)
    {
        check += send_data[i];
    }
    send_data[cnt++] = check; 
    return bsp_write_array(name,send_data,cnt);
}
//蓝牙发送gps导航相关数据
int gps_send_ble(PORT_NAME name,const int angle,const int speed)
{
    unsigned char send_data[50]={0};
    int cnt = 0;
    unsigned char check = 0;
    send_data[cnt++] = 0xAA;
    send_data[cnt++] = 0xAA;
    send_data[cnt++] = 0xF1;
    send_data[cnt++] = 0;
    // send_data[cnt++] = (unsigned char)(angle>>24);
    // send_data[cnt++] = (unsigned char)(angle>>16);
    // send_data[cnt++] = (unsigned char)(angle>>8);
    // send_data[cnt++] = (unsigned)angle;
    // send_data[cnt++] = (unsigned char)(speed>>24);
    // send_data[cnt++] = (unsigned char)(speed>>16);
    // send_data[cnt++] = (unsigned char)(speed>>8);
    // send_data[cnt++] = (unsigned)speed;
    send_data[cnt++] = (unsigned char)((int)gps_info.addr.Lattitude>>8);
    send_data[cnt++] = (unsigned char)gps_info.addr.Lattitude;
    send_data[cnt++] = (unsigned char)((int)gps_info.addr.Longitude>>8);
    send_data[cnt++] = (unsigned char)gps_info.addr.Longitude;
    send_data[cnt++] = (unsigned char)((int)gps_info.addr.Altitude>>8);
    send_data[cnt++] = (unsigned char)gps_info.addr.Altitude;
    send_data[cnt++] = (unsigned char)((int)gps_info.addr.Heading>>8);
    send_data[cnt++] = (unsigned char)gps_info.addr.Heading;
    send_data[cnt++] = (unsigned char)((int)gpslog_addr[uwb_info.index.self_index].Lattitude>>8);
    send_data[cnt++] = (unsigned char)gpslog_addr[uwb_info.index.self_index].Lattitude;
    send_data[cnt++] = (unsigned char)((int)gpslog_addr[uwb_info.index.self_index].Longitude>>8);
    send_data[cnt++] = (unsigned char)gpslog_addr[uwb_info.index.self_index].Longitude;
    send_data[cnt++] = (unsigned char)((int)gpslog_addr[uwb_info.index.self_index].Altitude>>8);
    send_data[cnt++] = (unsigned char)gpslog_addr[uwb_info.index.self_index].Altitude;
    send_data[cnt++] = (unsigned char)((int)gpslog_addr[uwb_info.index.self_index].Heading >>8);
    send_data[cnt++] = (unsigned char)gpslog_addr[uwb_info.index.self_index].Heading;
    send_data[3] = cnt - 4;
    for (int i = 0; i < cnt; i++)
    {
        check += send_data[i];
    }
    send_data[cnt++] = check; 
    return bsp_write_array(name,send_data,cnt);
}

