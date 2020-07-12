/**
 * @file main.cpp
 * @brief 
 * @author zhaokangxu (zhaokangxu@zhskg.cn)
 * @version 1.0
 * @date 2020-07-07
 * 
 * @copyright Copyright (c) 2020-2024  智慧式集团
 * 
 * @par 修改日志:
 * <table>
 * <tr><th>Date       <th>Version <th>Author  <th>Description
 * <tr><td>2020-07-07 <td>1.0     <td>zhaokangxu     <td>内容
 * </table>
 *  网址 https://bbs.csdn.net/topics/380203791
 *  fd句柄最大1024个
 */

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <errno.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <stdlib.h>
#include <sys/time.h>
#include <pthread.h>
#include "bsp_uwb.h"
#include "bsp_timer.h"
#include "bsp_lidar.h"
#include "bsp_imu.h"
#include "bsp_car_drive.h"
#include "bsp_ble.h"
#include "bsp_uart.h"
#include "app.h"
#include "bsp_pid.h"
#include "config.h"
#include "stop_mode.h"
#include "ble_remote.h"
#include "server_remote.h"
#include "gps_navigation.h"
#include "tag_navigation.h"
#include "fellow_mode.h"
#include "accompany_mode.h"
#include "bsp_hps3d.h"
#include "obstacle.h"
////////////////////////////////TODO(xiaozhong)///////////////////////////////////
#include "service.h"
void *navigation(void * arg);
extern long UWB_LOG_MAXNUM;
extern long GPS_LOG_MAXNUM;
extern int *p_start;
//用户下发信息
user_order_t userorder_info;
pthread_mutex_t mutex;                      //互斥锁申请
int main(void)
{
    userorder_info.car_mode = TAG_NAVIGATION;
    *p_start = -1;
    pthread_t t1,t2,t3;
    uart_init();                            //串口初始化
    InitPID();                              //初始化PID参数
    obs_init();                             //初始化避障的参数
    if(lidar_send(SINGLE_LIDAR1)>0)         //发送lidar启动命令
    {
        *p_start = 0;
        printf("lidar send success\n");
    }
    else printf("lidar send filed\n");
    printf("readfire timer1=%ld\n",get_ms());
    readfile_gps();
    readfile_uwb();
    printf("readfire timer2=%ld\n",get_ms());
    timer_init(timer_interupt,0,TIMER_INTERRUPT_NUM);         //打开定时器
    #if HPS3D_START
        hps3d();                                //启动面阵雷达
    #endif
    //创建两个线程1、select接收数据线程。2、导航线程。3、mqtt线程。4、面阵激光雷达自己的线程，回调函数
    int err = pthread_create(&t1,NULL,uart_selecte,NULL);
    if(err!=0)  printf("select thread_create Failed:%s\n",strerror(errno));
    else printf("select thread_create success\n");
    err = pthread_create(&t2,NULL,navigation,NULL);
    if(err!=0) printf("navigation thread_create Failed:%s\n",strerror(errno));
    else printf("navigation thread_create success\n");
    err = pthread_create(&t3,NULL,MqttAsyncServiceMain,NULL);
    if(err!=0) printf("mqtt thread_create Failed:%s\n",strerror(errno));
    else printf("mqtt thread_create success\n");
    int thread_a = pthread_join(t1,NULL);
    printf("thread_a:%d\n",thread_a);
    thread_a = pthread_join(t2, NULL);
    printf("thread_a:%d\n",thread_a);
    thread_a = pthread_join(t3, NULL);
    printf("thread_a:%d\n",thread_a);
    // while(1)
    // {
    // }
    //exit(0);
}
//导航功能线程
void *navigation(void * arg)
{
    pthread_mutex_lock(&mutex);      //加锁，若有线程获得锁，则会堵塞
    while (1)
    {
        switch (userorder_info.car_mode)
        {
            case STOP_MODE:                                         //停止模式
                stop_mode();
                break;
            case BLE_REMOTE:                                        //蓝牙遥控
                ble_remote();
                break;
            case SERVER_REMOTE:                                     //后台服务器遥控
                server_remote();
                break;
            case TAG_NAVIGATION:                                    //UWB导航
                tag_navigation();
                break;
            case GPS_NAVIGATION:                                    //GPS导航
                gps_navigation();
                break;
            case FELLOW_MODE:                                       //跟随模式
                fellow_mode();
                break;
            case ACCOMPANY_MODE:                                    //伴行模式
                accompany_mode();
                break;
            default:
                printf("\r\n模式选择错误:%d,请选择模式1至7\r\n",userorder_info.car_mode);
                break;
        }
        pthread_mutex_unlock(&mutex);   //解锁
    }
}


