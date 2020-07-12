#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "bsp_uwb.h"

AOA_report_t AOA_report;
UWB_ReportTag_t ReportTag_t;
UWB_ReportTag_t *p_ReportTag_t = &ReportTag_t;

UWB_DelTag_Report_t DelTag_Report_t;
UWB_DelTag_Report_t *p_DelTag_Report_t = &DelTag_Report_t;

UWB_AddTag_Report_t AddTag_Report_t;
UWB_AddTag_Report_t *p_AddTag_Report_t = &AddTag_Report_t;

//ReportTag_t.tag_addr64 = 0x82261444de34a00;
//简单的数据处理，并且封装成函数，可以使用的
// void receive_data(int nread1,unsigned char *buff1,int *conect_len1,int *data_len1)
// {
//     if((nread1 > 0)&&(nread1<100))
//     {
//         if((buff1[*conect_len1]==0x55)&&(buff1[*conect_len1+1]==nread1-4)&&\
//         (buff1[*conect_len1+2]==0x36||buff1[*conect_len1+2]==0x37||buff1[*conect_len1+2]==0x20))     //如果接收到完整一帧，其他的都抛弃
//         {
//             uart_handle(buff1+*conect_len1,buff1[*conect_len1+1]+4);
//             *conect_len1 = 0;
//             //printf("onec receive all data/n");
//         }
//         else if((buff1[*conect_len1]==0x55)&&(buff1[*conect_len1+1]>nread1-4)&&\
//         (buff1[*conect_len1+2]==0x36||buff1[*conect_len1+2]==0x37||buff1[*conect_len1+2]==0x20)) //如果接收到头长度不够，等下次接收
//         {
//             *data_len1 = buff1[*conect_len1+1];
//             if(*data_len1>35||*data_len1<=0)
//             {
//                 perror("*data_len1>35||*data_len1<=0");
//                 //printf("第一帧长度数据大于35:%d\n",*data_len1);
//             }
//             else
//             {
//                 *conect_len1 = nread1;
//             }
//             //printf("第一帧长度不够\n");
//         }
//         else if(*conect_len1>0)  //接着上次接收，处理数据和*conect_len1长度
//         {
//             if(*conect_len1+nread1==*data_len1+4)          //4是头和尾
//             {
//                 uart_handle(buff1,*data_len1+4);
//                 //printf("*conect_len1 is:%d,nread1 is:%d,*data_len1 is:%d\n",*conect_len1,nread1,*data_len1+4);
//             }
//             else
//             {
//                 printf("twice receive error!\n");
//             }
//             *conect_len1 = 0;
//             *data_len1 = 0;
//         }
//         else
//         {
//             printf("----------------handle data error-----------------\n");
//             //printf("-----------handle data error----------\n");
//         }
//     }
//     else
//         perror("read");
// }

//复杂的处理封装成函数
void receive_data(int nread1,unsigned char *buff1,int *conect_len1,int *data_len1)
{
    // if((nread1 > 0)&&(nread1<100))
    // {
        if((buff1[*conect_len1]==0x55)&&(buff1[*conect_len1+1]==nread1-4)&&(buff1[*conect_len1+2]==0x36||\
        buff1[*conect_len1+2]==0x37||buff1[*conect_len1+2]==0x20||buff1[*conect_len1+2]==0x21))     //如果接收到完整一帧，其他的都抛弃
        {
            //*data_len1 = buff1[1];
            uart_handle(buff1+*conect_len1,buff1[*conect_len1+1]+4);
            *conect_len1 = 0;
            //printf("onec receive all data/n");
        }
        else if((buff1[*conect_len1]==0x55)&&(buff1[*conect_len1+1]>nread1-4)&&(buff1[*conect_len1+2]==0x36||\
        buff1[*conect_len1+2]==0x37||buff1[*conect_len1+2]==0x20||buff1[*conect_len1+2]==0x21)) //如果接收到头长度不够，等下次接收
        {
            *data_len1 = buff1[*conect_len1+1];
            if(*data_len1>35||*data_len1<=0)
            {
                printf("一帧长度数据大于35:%d\n",*data_len1);
                perror("*data_len1>35||*data_len1<=0");
            }
            else
            {
                *conect_len1 = nread1;
            }
            //printf("第一帧长度不够\n");
        }
        else if((buff1[*conect_len1]==0x55)&&(buff1[*conect_len1+1]<nread1-4)&&(buff1[*conect_len1+2]==0x36||\
        buff1[*conect_len1+2]==0x37||buff1[*conect_len1+2]==0x20||buff1[*conect_len1+2]==0x21)) //如果新的一帧接收多了
        {
            uart_handle(buff1+*conect_len1,buff1[*conect_len1+1]+4);
            if(buff1[buff1[*conect_len1+1]+4]==0x55)
            {
                memcpy(buff1,buff1+buff1[*conect_len1+1]+4,*conect_len1+nread1-(buff1[*conect_len1+1]+4));
                *conect_len1 = *conect_len1+nread1-(*data_len1+4);
                //如果只接收到一个头，不知道datalen如何处理
            }
            else
            {
                *conect_len1 = 0;
                *data_len1 = 0;
            }
        }
        else if(*conect_len1>0)  //接着上次接收，处理数据和*conect_len1长度
        {
            if(*conect_len1+nread1==*data_len1+4)          //4是头和尾
            {
                uart_handle(buff1,*data_len1+4);
                //printf("*conect_len1 is:%d,nread1 is:%d,*data_len1 is:%d\n",*conect_len1,nread1,*data_len1+4);
                *conect_len1 = 0;
                *data_len1 = 0;
            }
            else if(*conect_len1+nread1<*data_len1+4)      //如果数据还未够，继续接
            {
                if(*conect_len1+nread1<100-32)           //-32避免越界，nread1最大接收32个
                {
                    *conect_len1 += nread1;
                    printf("第三帧长度还不够%d\n",*conect_len1);
                }
                else
                {
                    *data_len1 = 0;
                    *conect_len1 =0;
                    printf("receive data length >100-32\n");
                    perror("receive data length >100-32\n");
                }
            }
            else if(*conect_len1+nread1>*data_len1+4)  //如果接收的长度多了，且接收到头，则留给下一组
            {
                uart_handle(buff1,*data_len1+4);
                if(buff1[*data_len1+4]==0x55)       
                {
                    memcpy(buff1,buff1+*data_len1+4,*conect_len1+nread1-(*data_len1+4));
                    *conect_len1 = *conect_len1+nread1-(*data_len1+4);
                    //printf("-------------------data_len1 receive out-------------------");
                    //如果只接收到一个头，不知道datalen如何处理
                }
                else
                {
                    *conect_len1 = 0;
                    *data_len1 = 0;
                }
            }
        }
        else
        {
            printf("-----------handle data error----------\n");
        }
    // }
}

//协议数据解析出结果
int uart_handle(unsigned char *UWB_RX_BUF,int len) 
{
    unsigned char UWB_RX_CHECKSUM=0;
    if(UWB_RX_BUF[0]==0x55)
    {
        if(UWB_RX_BUF[1]==0x23&&UWB_RX_BUF[2]==0x37&&UWB_RX_BUF[1]==len-4)  //接收距离角度
        {
            for(int i=0;i<len-1;i++)
            {
                UWB_RX_CHECKSUM += UWB_RX_BUF[i];
            }
            if (UWB_RX_BUF[len-1]==UWB_RX_CHECKSUM)
            {
                 //printf("--------------------data is%ld\n",((unsigned int*)(UWB_RX_BUF+10)));
                 //printf("--------------------data is%ld\n",0xDEADBEEF);
                // return 1;
                // if (((unsigned int*)(UWB_RX_BUF+10) == 0xDEADBEEF) || ((unsigned int*)(UWB_RX_BUF+18) == 0xDEADBEEF) || ((unsigned int*)(UWB_RX_BUF+22) == 0xDEADBEEF))
                // {
                //     printf("--------------------data is DEAD-----------------");
                //     perror("com set error");
                //     // if (UWB_FreshCnt == 0) i = 5;
                //     // else i= UWB_FreshCnt - 1;
                //     // g_Angle[UWB_FreshCnt] = g_Angle[i];
                //     // g_iDist[UWB_FreshCnt++] = g_iDist[i];
                // }
                // else if((unsigned int*)(UWB_RX_BUF+10) == 0xDEADFEEB || (unsigned int*)(UWB_RX_BUF+14) == 0xDEADFEEB || (unsigned int*)(UWB_RX_BUF+26) == 0xDEADFEEB)
                // {
                //     perror("com set error");
                //     printf("--------------------data is DAED-----------------");
                // }
                // else 
                // {
                 AOA_report.title.start = UWB_RX_BUF[0];
                 AOA_report.title.len = UWB_RX_BUF[1];
                 AOA_report.title.order = UWB_RX_BUF[2];
                 AOA_report.addr16 = (unsigned short)UWB_RX_BUF[3]|(((unsigned short)UWB_RX_BUF[4])<<8);
                 AOA_report.rangeNum = UWB_RX_BUF[5];
                 AOA_report.resTime_us = (UWB_RX_BUF[6]&0x000000ff)|((UWB_RX_BUF[7]&0x000000ff)<<8)|((UWB_RX_BUF[8]&0x000000ff)<<16)|((UWB_RX_BUF[9]&0x000000ff)<<24);
                 AOA_report.dist  = (UWB_RX_BUF[10]&0x000000ff)|((UWB_RX_BUF[11]&0x000000ff)<<8)|((UWB_RX_BUF[12]&0x000000ff)<<16)|((UWB_RX_BUF[13]&0x000000ff)<<24);
                 AOA_report.angle = (UWB_RX_BUF[14]&0x000000ff)|((UWB_RX_BUF[15]&0x000000ff)<<8)|((UWB_RX_BUF[16]&0x000000ff)<<16)|((UWB_RX_BUF[17]&0x000000ff)<<24);
                 AOA_report.x_cm  = (UWB_RX_BUF[18]&0x000000ff)|((UWB_RX_BUF[19]&0x000000ff)<<8)|((UWB_RX_BUF[20]&0x000000ff)<<16)|((UWB_RX_BUF[21]&0x000000ff)<<24);
                 AOA_report.y_cm  = (UWB_RX_BUF[22]&0x000000ff)|((UWB_RX_BUF[23]&0x000000ff)<<8)|((UWB_RX_BUF[24]&0x000000ff)<<16)|((UWB_RX_BUF[25]&0x000000ff)<<24);
                 AOA_report.clockOffset_pphm = (UWB_RX_BUF[26]&0x000000ff)|((UWB_RX_BUF[27]&0x000000ff)<<8)|((UWB_RX_BUF[28]&0x000000ff)<<16)|((UWB_RX_BUF[29]&0x000000ff)<<24);
                 AOA_report.flag  = (unsigned short)UWB_RX_BUF[30]|(((unsigned short)UWB_RX_BUF[31])<<8);
                 AOA_report.acc_x = (unsigned short)UWB_RX_BUF[32]|(((unsigned short)UWB_RX_BUF[33])<<8);
                 AOA_report.acc_y = (unsigned short)UWB_RX_BUF[34]|(((unsigned short)UWB_RX_BUF[35])<<8);
                 AOA_report.acc_z = (unsigned short)UWB_RX_BUF[36]|(((unsigned short)UWB_RX_BUF[37])<<8);
                 AOA_report.end.end = UWB_RX_BUF[38];
                //printf("dist：%d，angle: %d\n",AOA_report.dist,AOA_report.angle);

                // printf("--------------------x_cm is %d\n",AOA_report.x_cm);
                // printf("--------------------y_cm is %d\n",AOA_report.y_cm);
                if((AOA_report.dist == 0xEFBEADDE) || (AOA_report.x_cm == 0xDEADBEEF) || (AOA_report.y_cm == 0xDEADBEEF))
                {
                    printf("--------------------data is 0xEFBEADDE-----------------");
                }
                if ((AOA_report.dist == 0xDEADBEEF) || (AOA_report.x_cm == 0xDEADFEEB) || (AOA_report.y_cm == 0xDEADFEEB))
                {
                    printf("--------------------data is 0xDEADBEEF-----------------");
                }
                 return 1;
                 // }
            }
            else
            {
                printf("AOA_report check error\n");
                return -1;
            }
        }    //发现新标签
        else if(UWB_RX_BUF[1]==0x08&&UWB_RX_BUF[2]==0x36&&UWB_RX_BUF[1]==len-4)
        {
            for (int i = 0; i < len-1; i++)
            {
                UWB_RX_CHECKSUM  += UWB_RX_BUF[i];
                /* code */
            }
            if (UWB_RX_BUF[len-1]==UWB_RX_CHECKSUM)
            {
                //memcpy(p_ReportTag_t,UWB_RX_BUF,len);
                p_ReportTag_t = (UWB_ReportTag_t *)UWB_RX_BUF;
                p_ReportTag_t->tag_addr64 = ((unsigned long)UWB_RX_BUF[3]<<56|((unsigned long)UWB_RX_BUF[4]<<48)|((unsigned long)UWB_RX_BUF[5]<<40)|((unsigned long)UWB_RX_BUF[6]<<32)|\
                ((unsigned long)UWB_RX_BUF[7]<<24)|((unsigned long)UWB_RX_BUF[8]<<16)|((unsigned long)UWB_RX_BUF[9]<<8)|((unsigned long)UWB_RX_BUF[10]));
                // printf("tig_title is :%2X,title_len:%2X,title_end%2X\n",p_ReportTag_t->title.start,p_ReportTag_t->title.len,p_ReportTag_t->end.end);
                // printf("tag_addr64 is :%16X\n",p_ReportTag_t->tag_addr64);
                // printf("tag_addr64 is :%ld\n",p_ReportTag_t->tag_addr64);
                return 1;
            }
            else
            {
                printf("UWB_ReportTag check error\n");
                return -1;
                /* code */
            }
        }      //添加标签后 返回消息：(上报)
        else if(UWB_RX_BUF[1]==0x11&&UWB_RX_BUF[2]==0x20&&UWB_RX_BUF[1]==len-4)
        {
            for (int i = 0; i < len-1; i++)
            {
                UWB_RX_CHECKSUM  += UWB_RX_BUF[i];
                /* code */
            }
            if (UWB_RX_BUF[len-1]==UWB_RX_CHECKSUM)
            {
                AddTag_Report_t.slot = (unsigned short)UWB_RX_BUF[3]<<8|UWB_RX_BUF[4];
                AddTag_Report_t.tag_addr64 = ((unsigned long)UWB_RX_BUF[5]<<56|((unsigned long)UWB_RX_BUF[6]<<48)|((unsigned long)UWB_RX_BUF[7]<<40)|((unsigned long)UWB_RX_BUF[8]<<32)|\
                ((unsigned long)UWB_RX_BUF[9]<<24)|((unsigned long)UWB_RX_BUF[10]<<16)|((unsigned long)UWB_RX_BUF[11]<<8)|((unsigned long)UWB_RX_BUF[12]));
                AddTag_Report_t.tag_addr16 = (unsigned short)UWB_RX_BUF[13]<<8|UWB_RX_BUF[14];
                AddTag_Report_t.multFast = (unsigned short)UWB_RX_BUF[15]<<8|UWB_RX_BUF[16];
                AddTag_Report_t.multSlow = (unsigned short)UWB_RX_BUF[17]<<8|UWB_RX_BUF[18];
                AddTag_Report_t.mode = UWB_RX_BUF[19];
                return 1;
            }
            else
            {
                printf("AddTag_Report check error\n");
                return -1;
                /* code */
            }
        }       //删除标签后 返回消息：(上报)
        else if(UWB_RX_BUF[1]==0x08&&UWB_RX_BUF[2]==0x21&&UWB_RX_BUF[1]==len-4)
        {
            for (int i = 0; i < len-1; i++)
            {
                UWB_RX_CHECKSUM  += UWB_RX_BUF[i];
                /* code */
            }
            if (UWB_RX_BUF[len-1]==UWB_RX_CHECKSUM)
            {
                
                DelTag_Report_t.tag_addr64 = ((unsigned long)UWB_RX_BUF[3]<<56|((unsigned long)UWB_RX_BUF[4]<<48)|((unsigned long)UWB_RX_BUF[5]<<40)|((unsigned long)UWB_RX_BUF[6]<<32)|\
                ((unsigned long)UWB_RX_BUF[7]<<24)|((unsigned long)UWB_RX_BUF[8]<<16)|((unsigned long)UWB_RX_BUF[9]<<8)|((unsigned long)UWB_RX_BUF[10]));
                // printf("DelTag_Report  success %16x\n", DelTag_Report_t.tag_addr64);
                // printf("DelTag_Report  success %ld\n", DelTag_Report_t.tag_addr64);
                return 1;
            }
            else
            {
                printf("DelTag_Report check error\n");
                return -1;
                /* code */
            }
        }
    }
    return 0;
}
//删除标签
int delTag(PORT_NAME name,unsigned long tag_addr)
{
    unsigned char send_data[12]={0};
    unsigned char check = 0;
    int cnt = 0;
    send_data[cnt++] = 0xAA;
    send_data[cnt++] = 0x08;
    send_data[cnt++] = 0x21;
    send_data[cnt++] = (char)(tag_addr>>56);
    send_data[cnt++] = (char)(tag_addr>>48);
    send_data[cnt++] = (char)(tag_addr>>40);
    send_data[cnt++] = (char)(tag_addr>>32);
    send_data[cnt++] = (char)(tag_addr>>24);
    send_data[cnt++] = (char)(tag_addr>>16);
    send_data[cnt++] = (char)(tag_addr>>8);
    send_data[cnt++] = (char)tag_addr;
    for(int i=3; i<cnt; i++)
    {
      check += send_data[i];
    }
    send_data[cnt++] = check;   //D3=0xaa+8+0x21
    return bsp_write_array(name,send_data,cnt);
}
// int handle_tag(unsigned char *ucData,int usLength)
// {
// 	static unsigned char chrTemp[100];
// 	static unsigned char ucRxCnt = 0;	
// 	static unsigned short usRxLength = 0;
// 	unsigned char constsum=0;
// 	int count=0;
//     // for (int i = 0; i < usLength; i++)
//     // {
// 	// 	if((ucData[i]>='A')&&(ucData[i]<='Z'))
// 	// 	{
// 	// 		ucData[i]=ucData[i]-'A'+10;
// 	// 	}
// 	// 	else if((ucData[i]>='a')&&(ucData[i]<='z'))
// 	// 	{
// 	// 		ucData[i]=ucData[i]-'a'+10;
// 	// 	}
// 	// 	else if((ucData[i]>='0')&&(ucData[i]<='9'))
// 	// 	{
// 	// 		ucData[i]=ucData[i]-'0';
// 	// 	}
// 	// 	if((i+1)%2==0)
// 	// 	{
// 	// 		ucData[(i-1)/2]=(ucData[i-1]<<4)|(ucData[i]);
// 	// 	}
//     // }
// 	if (usRxLength< UWB_RECEIVE_LEN)   //避免处理错误导致数组越界
// 	{
// 		memcpy(chrTemp+usRxLength,ucData,usLength);
// 		usRxLength += usLength;
// 	}
// 	else
// 	{
// 		memcpy(chrTemp,ucData,usLength);
// 		usRxLength += usLength;
// 		printf("uwb handle receive error\n");
// 		perror("uwb handle receive error\n");
// 	}
//     while (usRxLength >= UWB_RECEIVE_LEN)
//     {
//         if (chrTemp[0]!=0xAA&&chrTemp[1]!=)
//         {
// 			usRxLength--;
// 			printf("head error\n");
// 			memcpy(&chrTemp[0],&chrTemp[1],usRxLength);                        
//             continue;    //continue 退出这次循环执行下次
//         }
//         //接收数据 待处理


// 		usRxLength -= UWB_RECEIVE_LEN;
// 		constsum=0;
// 		memcpy(&chrTemp[0],&chrTemp[UWB_RECEIVE_LEN],usRxLength);
//     }
//     return usRxLength;
// }
//添加标签
// int AddTag(int fd_x,unsigned long tag_addr)
// {
//     unsigned char addr[12]={0};
//     int len=0,n=0;
//     addr[0] = 0xAA;
//     addr[1] = 0x0E;
//     addr[2] = 0x20;
//     addr[3] = (char)(tag_addr>>56);
//     addr[4] = (char)(tag_addr>>48);
//     addr[5] = (char)(tag_addr>>40);
//     addr[6] = (char)(tag_addr>>32);
//     addr[7] = (char)(tag_addr>>24);
//     addr[8] = (char)(tag_addr>>16);
//     addr[9] = (char)(tag_addr>>8);
//     addr[10] =(char)tag_addr;
    
//     addr[11] = 0xD3;   //D3=0xaa+8+0x21
//     for(int i=3; i<11; i++)
//     {
//       addr[11] += addr[i];
//     }
//     while(1)
//     {
//         len += write(fd_x,addr+len,sizeof(addr)-len);
//         n++;
//         if(sizeof(addr)==len)
//         {
//             break;
//         }
//         else if(n>=10)       //写10次长度还是不对，则报错
//         {
//             printf("write error len:%d\n",len);
//             return -1;
//         }
//     }
//     return len;
// }

//单个字节接收
// int USART1_IRQHandler(unsigned char Res)                    
// {
//     static unsigned char UWB_RX_BUF[40];
//     static unsigned char UWB_RX_CON = 0;
//     unsigned char UWB_RX_CHECKSUM=0;
//     static unsigned char UWB_RX_LEN = 0;
//     //unsigned char Res=0;
//     unsigned char i,j;
//     if(UWB_RX_CON < 3)  
//     {
//         switch(UWB_RX_CON)
//         {
//             case 0x00:
//                 if(Res == 0x55)
//                 {
//                     UWB_RX_BUF[0] = Res;
//                     UWB_RX_CON = 1;
//                 }
//                 break;
//             case 0x01:
//                 if(Res==0x23)//长度是35
//                 {
//                     UWB_RX_BUF[1] = Res;
//                     UWB_RX_LEN = Res;
//                     UWB_RX_CON = 2;
//                 }
//                 else if (Res == 0x55)
//                 {
//                     UWB_RX_BUF[0] = Res;
//                     UWB_RX_CON = 1;
//                 }
//                 else
//                     UWB_RX_CON = 0;
//                 break;
//             case 0x02:
//                 if(Res == 0x37)
//                 {
//                     UWB_RX_BUF[2] = Res;
//                     UWB_RX_CON = 3;
//                 }
//                 else if (Res == 0x55)
//                 {
//                     UWB_RX_BUF[0] = Res;
//                     UWB_RX_CON = 1;
//                 }
//                 else
//                     UWB_RX_CON = 0;
//                 break;
//         }
//     }
//     else  
//     {
//         if (UWB_RX_CON < UWB_RX_LEN+3)              
//         {
//             UWB_RX_BUF[UWB_RX_CON++] = Res;
//         }
//         else if (UWB_RX_CON == UWB_RX_LEN+3)
//         {
//             UWB_RX_BUF[UWB_RX_CON]=Res;
//             //UWB_RX_CHECKSUM = UWB_RX_BUF[UWB_RX_CON];
//             for(unsigned char i=0;i<UWB_RX_LEN+3;i++)
//             {
//                 UWB_RX_CHECKSUM += UWB_RX_BUF[i];
//             }
//             if (UWB_RX_BUF[UWB_RX_CON]==UWB_RX_CHECKSUM)
//             {
//                  //printf("--------------------data is%ld\n",((unsigned int*)(UWB_RX_BUF+10)));
//                  //printf("--------------------data is%ld\n",0xDEADBEEF);
//                 // return 1;
//                 // if (((unsigned int*)(UWB_RX_BUF+10) == 0xDEADBEEF) || ((unsigned int*)(UWB_RX_BUF+18) == 0xDEADBEEF) || ((unsigned int*)(UWB_RX_BUF+22) == 0xDEADBEEF))
//                 // {
//                 //     printf("--------------------data is DEAD-----------------");
//                 //     perror("com set error");
//                 //     // if (UWB_FreshCnt == 0) i = 5;
//                 //     // else i= UWB_FreshCnt - 1;
//                 //     // g_Angle[UWB_FreshCnt] = g_Angle[i];
//                 //     // g_iDist[UWB_FreshCnt++] = g_iDist[i];
//                 // }
//                 // else if((unsigned int*)(UWB_RX_BUF+10) == 0xDEADFEEB || (unsigned int*)(UWB_RX_BUF+14) == 0xDEADFEEB || (unsigned int*)(UWB_RX_BUF+26) == 0xDEADFEEB)
//                 // {
//                 //     perror("com set error");
//                 //     printf("--------------------data is DAED-----------------");
//                 // }
//                 // else 
//                 // {
//                  AOA_report.title.start = UWB_RX_BUF[0];
//                  AOA_report.title.len = UWB_RX_BUF[1];
//                  AOA_report.type = UWB_RX_BUF[2];
//                  AOA_report.addr16 = (unsigned short)UWB_RX_BUF[3]|(((unsigned short)UWB_RX_BUF[4])<<8);
//                  AOA_report.rangeNum = UWB_RX_BUF[5];
//                  AOA_report.resTime_us = (UWB_RX_BUF[6]&0x000000ff)|((UWB_RX_BUF[7]&0x000000ff)<<8)|((UWB_RX_BUF[8]&0x000000ff)<<16)|((UWB_RX_BUF[9]&0x000000ff)<<24);
//                  AOA_report.dist  = (UWB_RX_BUF[10]&0x000000ff)|((UWB_RX_BUF[11]&0x000000ff)<<8)|((UWB_RX_BUF[12]&0x000000ff)<<16)|((UWB_RX_BUF[13]&0x000000ff)<<24);
//                  AOA_report.angle = (UWB_RX_BUF[14]&0x000000ff)|((UWB_RX_BUF[15]&0x000000ff)<<8)|((UWB_RX_BUF[16]&0x000000ff)<<16)|((UWB_RX_BUF[17]&0x000000ff)<<24);
//                  AOA_report.x_cm  = (UWB_RX_BUF[18]&0x000000ff)|((UWB_RX_BUF[19]&0x000000ff)<<8)|((UWB_RX_BUF[20]&0x000000ff)<<16)|((UWB_RX_BUF[21]&0x000000ff)<<24);
//                  AOA_report.y_cm  = (UWB_RX_BUF[22]&0x000000ff)|((UWB_RX_BUF[23]&0x000000ff)<<8)|((UWB_RX_BUF[24]&0x000000ff)<<16)|((UWB_RX_BUF[25]&0x000000ff)<<24);
//                  AOA_report.clockOffset_pphm = (UWB_RX_BUF[26]&0x000000ff)|((UWB_RX_BUF[27]&0x000000ff)<<8)|((UWB_RX_BUF[28]&0x000000ff)<<16)|((UWB_RX_BUF[29]&0x000000ff)<<24);
//                  AOA_report.flag  = (unsigned short)UWB_RX_BUF[30]|(((unsigned short)UWB_RX_BUF[31])<<8);
//                  AOA_report.acc_x = (unsigned short)UWB_RX_BUF[32]|(((unsigned short)UWB_RX_BUF[33])<<8);
//                  AOA_report.acc_y = (unsigned short)UWB_RX_BUF[34]|(((unsigned short)UWB_RX_BUF[35])<<8);
//                  AOA_report.acc_z = (unsigned short)UWB_RX_BUF[36]|(((unsigned short)UWB_RX_BUF[37])<<8);
//                  AOA_report.end.end = UWB_RX_BUF[38];
//                 printf("--------------------dist is %ld\n",AOA_report.dist);
//                 printf("--------------------x_cm is %d\n",AOA_report.x_cm);
//                 printf("--------------------y_cm is %d\n",AOA_report.y_cm);
//                 if ((AOA_report.dist == 0xEFBEADDE) || (AOA_report.x_cm == 0xDEADBEEF) || (AOA_report.y_cm == 0xDEADBEEF))
//                 {
//                     printf("--------------------data is 0xEFBEADDE-----------------");
//                 }

//                 if ((AOA_report.dist == 0xDEADBEEF) || (AOA_report.x_cm == 0xDEADFEEB) || (AOA_report.y_cm == 0xDEADFEEB))
//                 {
//                     printf("--------------------data is 0xDEADBEEF-----------------");
//                 }
//                  //if(UWB_FreshCnt==3)      UWB_FreshCnt=0;
//                  // UWB_FreshCnt = 1;
//                  UWB_RX_CON = 0;
//                  return 1;
//                  // }
//             }
//             else
//             {
//                return -1;
//                UWB_RX_CON = 0;
//             }
//         }
//     }
//     return 0;
// }