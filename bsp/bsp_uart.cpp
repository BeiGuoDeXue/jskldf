/**
 * @file bsp_uart.cpp
 * @brief 
 * @author zhaokangxu (zhaokangxu@zhskg.cn)
 * @version 1.0
 * @date 2020-06-11
 * 
 * @copyright Copyright (c) 2020-2024  智慧式集团
 * 
 * @par 修改日志:
 * <table>
 * <tr><th>Date       <th>Version <th>Author  <th>Description
 * <tr><td>2020-06-11 <td>1.0     <td>zhaokangxu     <td>内容
 * </table>
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
#include "bsp_gps.h"
#include "bsp_tag.h"
#include "bsp_lte.h"
#include "config.h"

/*****************************select从缓存读取字节数*****************************/
#define READ_LIDAR_LEN                  1450
#define READ_UWB_LEN                    32
#define READ_TAG_LEN                    32
#define READ_CAR_DRIVE_LEN              50
#define READ_FRESH_LEN                  50
#define READ_IMU_LEN                    44
#define READ_GPS_LEN                    100
#define READ_BLE_LEN                    20
#define READ_LET_LEN                    64
/*****************************select超时时间设置，单位s*****************************/
#define SELECT_TIMEOUT                  5 
/********************************************************************************/
int *p_start = (int*)malloc(sizeof(int));

static fd_set rd, wr;
static struct timeval tv,tv1;
static int maxfd = 0;
static uart_port_t  uart_port[MAX_PORT] = UART_PORT_CFG;
static int set_port(uart_port_t *p_uart_port,int name);
static int open_port(uart_port_t *p_uart_port,int name);

extern pthread_mutex_t mutex;           //互斥锁申请
//多路串口接收线程
void *uart_selecte(void * arg)
{
    pthread_mutex_lock(&mutex);      //加锁，若有线程获得锁，则会堵塞
    static int p_data_len[8] = {0};
    static int p_conect_len[8] = {0};
    static unsigned char uwb_buff[2][500];
    unsigned char buff[8][500] = {0};
    unsigned char lidar_buff[2000] = {0};
    int nwrite = 0, nread = 0;
    long uart_timer = 0;
    /*测试串口描述符是否准备就绪，并调用select函数对相关串口描述符做对应操作*/
    //while(FD_ISSET(TAG1, &rd) || FD_ISSET(TAG2, &rd) || FD_ISSET(GPS, &rd)||FD_ISSET(fd[3], &rd))   
    while (1)
    {
        FD_ZERO(&rd);
        for (int i = 0; i < MAX_PORT; i++)
        {
            if(PORT_NUM&(0x00000001<<i))
            {
                FD_SET(uart_port[i].fd, &rd);
            }
        }
        tv.tv_sec  = SELECT_TIMEOUT;    //超时时间5s
        tv.tv_usec = 0;
        if(select(maxfd + 1, &rd, NULL, NULL, &tv) < 0)
        {
            gettimeofday(&tv1, NULL);             //获取时间
            printf("----select <0 ms is = %ld\n",tv1.tv_sec * 1000 + tv1.tv_usec / 1000);
        }
        else if (select(maxfd + 1, &rd, NULL, NULL, &tv) == 0)
        {
            printf("-------------select = 0,all port timeout: %ds----------------\n",SELECT_TIMEOUT);
        }
        else
        {
            gettimeofday(&tv1, NULL);               //获取时间
            uart_timer = tv1.tv_sec * 1000 + tv1.tv_usec / 1000;
            // if(FD_ISSET(TAG1, &rd)==0)
            // {
            //     uwb_timer1 = uart_timer;
            // }
            // if(FD_ISSET(TAG2, &rd)==0)
            // {
                
            // }
            if(FD_ISSET(uart_port[TAG1].fd, &rd))                //uwb1
            {
                nread = read(uart_port[TAG1].fd, uwb_buff[0]+p_conect_len[0],READ_TAG_LEN);
                if (nread<=READ_TAG_LEN)
                {
                    p_conect_len[0] += nread;
                    //printf("p_conect_len:%d\n",p_conect_len[0]);
                }
                //printf("ms0=%ld  read_len0: %d\n",uart_timer,nread);
                if(nread>0)
                {
                    // for(int i=0;i< p_conect_len[0]+nread;i++)
                    // {
                    //     printf("%02x",buff[0][i]);
                    //     //printf("i:%d,%02x\n",i,buff[0][i]);
                    // }
                    // printf("\n");
                    //write(TAG2, buff[0],nread);
                    //receive_data(nread,buff[0],&p_conect_len[0],&p_data_len[0]);
                    
                    // for(int i=0;i< nread;i++)
                    // {
                    //     printf("%c",uwb_buff[0][i]);
                    // }
                    //printf("\n");
                    //printf("%s",uwb_buff[0]);
                    receive_tag(uwb_buff[0],nread,&p_conect_len[0],uart_timer,UWB_BACK);
                }
                else
                    printf("nread0<0\n");
            }
            // else
            // {
            //     //超时处理
            // }                      //uwb2
            if(FD_ISSET(uart_port[TAG2].fd,&rd))   //这里好像是添加了可读写的描述符进去，来消息就为真，如果没打开串口，这里也会一直为真**
            {
                // nread = read(TAG2, buff[1] + p_conect_len[1], 40);
                // //printf("ms1 = %ld  read_len1: %d\n",,nread);
                // if((nread > 0)&&(nread<100))
                // {
                //     // for(int i=0;i< p_conect_len[1]+nread;i++)
                //     // {
                //     //     printf("%02x",buff[1][i]);
                //     //     //printf("i:%d,%02x\n",i,buff[0][i]);
                //     // }
                //     // printf("\n");
                //     //write(TAG2, buff[1], nread);
                //     receive_data(nread,buff[1],&p_conect_len[1],&p_data_len[1]);
                // }
                // else
                // {
                //     printf("nread1<0\n");
                // }
                nread = read(uart_port[TAG2].fd, uwb_buff[1]+p_conect_len[1],32);
                if (nread<=32)
                {
                    p_conect_len[1] += nread;
                    //printf("p_conect_len1: %d\n",p_conect_len[1]);
                }
                //printf("ms1=%ld  read_len1: %d\n",uart_timer,nread);
                if((nread > 0)&&(nread<200))
                {
                    //printf("%s\n",buff[1]);
                    // for (int i = 0; i < nread; i++)
                    // {
                    //     printf("%c",uwb_buff[1][i]);
                    // }
                    //  printf("\n");
                    // for(int i=0;i< p_conect_len[1]+nread;i++)
                    // {
                    //     printf("%02x",buff[1][i]);
                    //     //printf("i:%d,%02x\n",i,buff[0][i]);
                    // }
                    // printf("\n");
                    //handle_gps(buff[1],nread);
                    receive_tag(uwb_buff[1],nread,&p_conect_len[1],uart_timer,UWB_FRONT);
                }
                else
                {
                    printf("nread1<0\n");
                }
            }
            if(FD_ISSET(uart_port[GPS].fd, &rd))                //lidar
            {
                nread = read(uart_port[GPS].fd, lidar_buff, READ_LIDAR_LEN);
                printf("ms2=%ld ,read_len2: %d\n ",uart_timer,nread);
                if((nread > 0)&&(nread<READ_LIDAR_LEN))
                {
                    lidar_handle(lidar_buff,nread,p_start);
                }
                else
                {
                    printf("GPS nread<0 or GPS nread>1500\n");
                }
            }
            if(FD_ISSET(uart_port[IMU].fd, &rd))               //imu
            {
                nread = read(uart_port[IMU].fd, buff[3], READ_IMU_LEN);
                //printf("ms3=%ld  read_len3: %d \n",uart_timer,nread);
                if((nread > 0)&&(nread<100))
                {
                    // for(int i=0;i< nread;i++)
                    // {
                    //     printf("%02x",buff[3][i]);
                    //     //printf("i:%d,%02x\n",i,buff[0][i]);
                    // }
                    // printf("\n");
                    JY901.CopeSerialData((char*)buff[3],(short)nread); 
                }
                else
                {
                    printf("nread3<0 or nread3>100\n");
                }
            }
            if(FD_ISSET(uart_port[CARDRIVE].fd, &rd))                  //stm32
            {
                nread = read(uart_port[CARDRIVE].fd,buff[4], READ_CAR_DRIVE_LEN);
                //printf("ms4=%ld  read_len4: %d \n",uart_timer,nread);
                if((nread > 0)&&(nread<100))
                {

                    // for(int i=0;i< nread;i++)
                    // {
                    //     printf("%02x",buff[4][i]);
                    //     //printf("i:%d,%02x\n",i,buff[0][i]);
                    // }
                    //printf("\n");
                    handle_stm32(buff[4],nread);
                }
                else
                {
                    printf("nread4<0 or nread4>100\n");
                }
            }
            if(FD_ISSET(uart_port[BLE].fd, &rd))    //ble
            {
                nread = read(uart_port[BLE].fd, buff[5], READ_BLE_LEN);
                printf("ms5=%ld  read_len5: %d \n",tv1.tv_sec * 1000 + tv1.tv_usec / 1000,nread);
                if((nread > 0)&&(nread<100))
                {
                    for(int i=0;i< nread;i++)
                    {
                        printf("%02x",buff[5][i]);
                        //printf("i:%d,%02x\n",i,buff[0][i]);
                    }
                    printf("\n");
                    handle_ble(buff[5],nread);
                }
                else
                {
                    printf("nread5<0 or nread5>100\n");
                }
            }
            if(FD_ISSET(uart_port[EC20_2].fd, &rd))    
            {
                nread = read(uart_port[EC20_2].fd, buff[6], READ_LET_LEN);
                printf("ms6=%ld  read_len6: %d \n",tv1.tv_sec * 1000 + tv1.tv_usec / 1000,nread);
                if((nread > 0)&&(nread<100))
                {
                    // for(int i=0;i< nread;i++)
                    // {
                    //     printf("%02x",buff[5][i]);
                    //     //printf("i:%d,%02x\n",i,buff[0][i]);
                    // }
                    for(int i=0;i< nread;i++)
                    {
                        printf("%c",buff[6][i]);
                    }
                    printf("\n");
                    if (nread>17)
                    {
                        receive_lte(buff[6],nread);
                    }
                    
                }
                else
                {
                    printf("nread6<0 or nread6>100\n");
                }
            }
        }
        pthread_mutex_unlock(&mutex);   //解锁
    }
}
//串口初始化
int uart_init(void)
{
    int a;
    for (int i = 0; i < MAX_PORT; i++)
    {
        if(PORT_NUM&(0x01<<i))
        {
             a= PORT_NUM&(0x01<<i);
            // printf("a: %x\n",a);
            // printf("PORT_NUM: %8x\n",PORT_NUM);
            // printf("i: %d\n",i);
            if((uart_port[i].fd = open_port(uart_port,i)) < 0)
            {
                perror("open_port error fd is\n");
                return 0;
            }
            if(set_port(uart_port,i) < 0)
            {
                perror("set_port0 error");
                return 0;
            }
        }
    }
    for (int i = 0; i < MAX_PORT; i++)
    {
        /* code */
        maxfd = maxfd > uart_port[i].fd ? maxfd : uart_port[i].fd;
    }
    printf("fd is %d\n",maxfd);
    /*初始化读集合rd,并在读集合中加入相应的描述符*/
    FD_ZERO(&rd);
    for (int i = 0; i < MAX_PORT; i++)
    {
        if(PORT_NUM&(0x00000001<<i))
        {
            FD_SET(uart_port[i].fd, &rd);
        }
    }
    tv.tv_sec = SELECT_TIMEOUT;
    tv.tv_usec = 0;
}
//设置串口参数
//int set_port(int fd, int nSpeed, int nBits, char nEvent, int nStop)
int set_port(uart_port_t *p_uart_port,int num)
{
        struct termios newtio, oldtio;
        /*保存测试现有串口参数设置，在这里如果串口号出错，会有相关的出错信息*/
        if(tcgetattr(p_uart_port[num].fd, &oldtio) != 0)
        {
            perror("SetupSerial 1");
            return -1;
        }
        bzero(&newtio, sizeof(newtio));
        /*步骤一，设置字符大小*/
        newtio.c_cflag |= CLOCAL | CREAD;
        newtio.c_cflag &= ~CSIZE;
        /*设置停止位*/
        switch(p_uart_port[num].nBits)
        {
        case 7:
            newtio.c_cflag |= CS7;
            break;
        case 8:
            newtio.c_cflag |= CS8;
            break;
        }
        /*设置奇偶校验位*/
        switch(p_uart_port[num].nEvent)
        {
        case 'O'://奇数
            newtio.c_cflag |= PARENB;
            newtio.c_cflag |= PARODD;
            newtio.c_iflag |= (INPCK | ISTRIP);
            break;
        case 'E'://偶数
            newtio.c_cflag |= PARENB;
            newtio.c_cflag &= ~PARODD;
            newtio.c_iflag |= (INPCK | ISTRIP);
            break;
        case 'N': //无奇偶校验位
            newtio.c_cflag &= ~PARENB;
            break;
        }
        /*设置波特率*/
        switch(p_uart_port[num].baudtate)
        {
        case 2400:
            cfsetispeed(&newtio, B2400);
            cfsetospeed(&newtio, B2400);
            break;
        case 4800:
            cfsetispeed(&newtio, B4800);
            cfsetospeed(&newtio, B4800);
            break;
        case 9600:
            cfsetispeed(&newtio, B9600);
            cfsetospeed(&newtio, B9600);
            break;
        case 115200:
            cfsetispeed(&newtio, B115200);
            cfsetospeed(&newtio, B115200);
            break;
        case 460800:
            cfsetispeed(&newtio, B460800);
            cfsetospeed(&newtio, B460800);
            break;
        default:
            cfsetispeed(&newtio, B115200);
            cfsetospeed(&newtio, B115200);
            break;
        }
        /*设置停止位*/
        if( p_uart_port[num].nStop == 1)
            newtio.c_cflag &= ~CSTOPB;
        else if( p_uart_port[num].nStop == 2)
            newtio.c_cflag |= CSTOPB;
        /*设置等待时间和最小接受字符*/
        newtio.c_cc[VTIME] = 0;
        newtio.c_cc[VMIN] = 0;
        /*处理未接收字符串*/
        tcflush(p_uart_port[num].fd, TCIFLUSH);
        /*激活新配置*/
        if(( tcsetattr(p_uart_port[num].fd, TCSANOW, &newtio)) != 0)
        {
            perror("com set error");
            return -1;
        }
        printf("set done!\n");  
        return 0;
}
//打开串口
int open_port(uart_port_t *p_uart_port,int num)
{
    // const char *dev[] = {"/dev/ttyUSB01", "/dev/ttyUSB02", "/dev/ttyUSB03","/dev/ttyUSB04",\
    // "/dev/ttyUSB05", "/dev/ttyUSB06", "/dev/ttyUSB07","/dev/ttyUSB08"};
    //long vdisable;
    int fd;
    fd = open(p_uart_port[num].name, O_RDWR|O_NOCTTY|O_NDELAY);
    if(-1 == fd)
    {
        printf("Can't Open Serial Port is:%s\n",p_uart_port[num].name);
        perror("Can't Open Serial Port");
        return -1;
    }
    /*恢复串口为阻塞状态*/
    if(fcntl(fd, F_SETFL, 0) < 0)
        printf("fcntl ailed!\n");
    else
        printf("fcntl = %d\n", fcntl(fd, F_SETFL, 0));
    /*测试是否为终端设备*/
    if(isatty(STDIN_FILENO) == 0)
        printf("standard input is not a terminal device\n");
    else
        printf("isatty success!\n");
    printf("fd-open = %d\n", fd);
    return fd;
}
int bsp_write_array(PORT_NAME name,unsigned char *array,int lens)
{
    //int fd_x = uart_port[name].fd;
    int len = 0,n = 0;
    while(1)
    {
        len += write(uart_port[name].fd,array+len,lens-len);
        n++;
        if(lens==len)
        {
            break;
        }
        else if(n>=10)   //写10次长度还是不对，则报错
        {
            printf("write erro: %s\n",uart_port[name].name);
            return -1;
            return FALSE;
        }
    }
    return len;
}

