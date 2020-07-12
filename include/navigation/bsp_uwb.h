#ifndef __BSP_UWB_H__
#define __BSP_UWB_H__
#include "config.h"

#pragma pack(1)
typedef struct
{
    unsigned char end;
}frame_end_t;

typedef struct
{
    unsigned char  start;
    unsigned char  len;
    unsigned char  order;
}frame_title_t;
typedef struct
{
    frame_title_t title;
    unsigned short addr16;//0~255
    unsigned char rangeNum;//
    unsigned int  resTime_us;//
    unsigned int  dist;//cm
    int  angle;//1000
    int  x_cm;//
    int  y_cm;//
	unsigned int  clockOffset_pphm;//
	unsigned short  flag;//
	short  acc_x;//
	short  acc_y;//
	short  acc_z;//
    frame_end_t  end;//
}AOA_report_t;
//发现新标签
typedef struct
{
    frame_title_t title;
    unsigned long tag_addr64;
    frame_end_t  end;
}UWB_ReportTag_t;
//删除后上报消息
typedef struct
{
    frame_title_t title;
    unsigned long tag_addr64;
    frame_end_t  end;
}UWB_DelTag_Report_t;

//添加后上报消息
typedef struct
{
    frame_title_t title;
    unsigned short slot;
    unsigned long tag_addr64;
    unsigned short tag_addr16;
    unsigned short multFast;
    unsigned short multSlow;
    unsigned char  mode;
    frame_end_t  end;
}UWB_AddTag_Report_t;


#pragma pack(0)

int uart_handle(unsigned char *UWB_RX_BUF,int len);
void receive_data(int nread1,unsigned char *buff1,int *conect_len1,int *data_len1);
//删除标签
int delTag(PORT_NAME name,unsigned long tag_addr);
#endif 

//这是本方案的最简单的处理方法，还未封装成函数
// if(FD_ISSET(fd0, &rd))
// {
//     nread = read(fd0, buff[0]+conect_len, 40);

//     printf("ms=%ld  read_len: %d  ",tv1.tv_sec * 1000 + tv1.tv_usec / 1000,nread);
//     for(int i=0;i<conect_len+nread;i++)
//     {
//         printf("%02x",buff[0][i]);
//         //printf("i:%d,%02x\n",i,buff[0][i]);
//     }
//     printf("\n");

//     if((nread > 0)&&(nread<100))
//     {
//         if((buff[0][conect_len]==0x55)&&(buff[0][conect_len+1]==nread-4))     //如果接收到完整一帧，其他的都抛弃
//         {
//             uart_handle(buff[0]+conect_len,buff[0][conect_len+1]+4);
//             conect_len = 0;
//             //printf("onec receive all data/n");
//         }
//         else if((buff[0][conect_len]==0x55)&&(buff[0][conect_len+1]>nread-4)) //如果接收到头长度不够，等下次接收
//         {
//             data_len = buff[0][conect_len+1];
//             if(data_len>35||data_len<=0)
//             {
//                 perror("data_len>35||data_len<=0");
//                 //printf("第一帧长度数据大于35:%d\n",data_len);
//             }
//             else
//             {
//                 conect_len = nread;
//             }
//             //printf("第一帧长度不够\n");
//         }
//         else if(conect_len>0)  //接着上次接收，处理数据和conect_len长度
//         {
//             if(conect_len+nread==data_len+4)          //4是头和尾
//             {
//                 uart_handle(buff[0],data_len+4);
//                 //printf("conect_len is:%d,nread is:%d,data_len is:%d\n",conect_len,nread,data_len+4);
//             }
//             else
//             {

//                 printf("twice receive error!\n");
//             }
//             conect_len = 0;
//             data_len = 0;
//         }
//         else
//         {
//             printf("-----------handle data error----------\n");
//             printf("-----------handle data error----------\n");
//         }
//     }
//     else
//         perror("read");
// }


//下面是接收数据处理复杂情况的方法，可能会有bug，先不用
// if((nread > 0)&&(nread<100))
// {
//     if((buff[0][conect_len]==0x55)&&(buff[0][conect_len+1]==nread-4))     //如果接收到完整一帧，其他的都抛弃
//     {
//         //data_len = buff[0][1];
//         uart_handle(buff[0]+conect_len,buff[0][conect_len+1]+4);
//         conect_len = 0;
//         printf("onec receive all data/n");
//     }
//     else if((buff[0][conect_len]==0x55)&&(buff[0][conect_len+1]>nread-4)) //如果接收到头长度不够，等下次接收
//     {
//         data_len = buff[0][conect_len+1];
//         if(data_len>35||data_len<=0)
//         {
//             perror("data_len>35||data_len<=0");
//             //printf("第一帧长度数据大于35:%d\n",data_len);
//         }
//         else
//         {
//             conect_len = nread;
//         }
//         //printf("第一帧长度不够\n");
//     }
//     else if((buff[0][conect_len]==0x55)&&(buff[0][conect_len+1]<nread-4)) //如果新的一帧接收多了
//     {
//         uart_handle(buff[0]+conect_len,buff[0][conect_len+1]+4);
//         if(buff[0][buff[0][conect_len+1]+4]==0x55)
//         {
//             memcpy(buff[0],buff[0]+buff[0][conect_len+1]+4,conect_len+nread-(buff[0][conect_len+1]+4));
//             conect_len = conect_len+nread-(data_len+4);
//             //如果只接收到一个头，不知道datalen如何处理
//         }
//         else
//         {
//             conect_len = 0;
//             data_len = 0;
//         }
//     }
//     else if(conect_len>0)  //接着上次接收，处理数据和conect_len长度
//     {
//         if(conect_len+nread==data_len+4)          //4是头和尾
//         {
//             uart_handle(buff[0],data_len+4);
//             printf("conect_len is:%d,nread is:%d,data_len is:%d\n",conect_len,nread,data_len+4);
//             conect_len = 0;
//             data_len = 0;
//         }
//         else if(conect_len+nread<data_len+4)      //如果数据还未够，继续接
//         {
//             if(conect_len+nread<100-32)          //-32避免越界，nread最大接收32个
//             {
//                 conect_len +=nread;
//                 printf("第三帧长度还不够%d\n",conect_len);
//             }
//             else
//             {
//                 data_len = 0;
//                 conect_len =0;
//                 perror("receive data length >100-32\n");
//             }
//         }
//         else if(conect_len+nread>data_len+4)  //如果接收的长度多了，且接收到头，则留给下一组
//         {
//             uart_handle(buff[0],data_len+4);
//             if(buff[0][data_len+4]==0x55)       
//             {
//                 memcpy(buff[0],buff[0]+data_len+4,conect_len+nread-(data_len+4));
//                 conect_len = conect_len+nread-(data_len+4);
//                 //如果只接收到一个头，不知道datalen如何处理
//             }
//             else
//             {
//                 conect_len = 0;
//                 data_len = 0;
//             }
//         }
//     }
//     else
//     {
//         printf("-----------handle data error----------\n");
//         printf("-----------handle data error----------\n");
//     }