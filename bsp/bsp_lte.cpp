#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "bsp_lte.h"

#define LTE_RECEIVE_LEN  21
let_info_t let_info;
/**
 * @brief 接收处理4G消息函数
 * @param  ucData           My Param doc
 * @param  usLength         My Param doc
 * @return int 
 */
int receive_lte(unsigned char *ucData,int usLength)
{
	static unsigned char chrTemp[100];
	static unsigned short usRxLength = 0;
    int num = 0;
    if (usRxLength+usLength<100)
    {
        memcpy(chrTemp+usRxLength,ucData,usLength);
        usRxLength += usLength;
    }else{
        memcpy(chrTemp,ucData,usLength);
        usRxLength += usLength;
    }
    while(usRxLength >= LTE_RECEIVE_LEN)
    {
            if (chrTemp[0]=='\r'&&chrTemp[1]=='\n'&&chrTemp[2]=='+'&&chrTemp[3]=='Q'&&chrTemp[4]=='G'&&chrTemp[5]=='S'&&chrTemp[6]=='N'&&chrTemp[7]==':'&&chrTemp[8]==' ')
            {
                if (usRxLength>=9+15+8)                      //    /r/n+QGSN：长9  ID长15  /r/n/r/nOK/r/n   长8
                {
                    if ((chrTemp[9+15]=='\r'&&chrTemp[9+15+1]=='\n')&&(chrTemp[9+15+4]=='O'&&chrTemp[9+15+5]=='K'))
                    {
                        memcpy(&let_info.qgsn.qgsn,&chrTemp[9],sizeof(let_info.qgsn.qgsn));
                        usRxLength -= 9+15+8;
                        memcpy(&chrTemp[0],&chrTemp[9+15+8],usRxLength);
                        printf("QGSN: %s\n",let_info.qgsn.qgsn);
                    }
                    else
                    {
                        usRxLength -= 9;             //如果尾不正确，把头去掉继续
                        memcpy(&chrTemp[0],&chrTemp[9],usRxLength);
                    }
                }
            }
            else if (chrTemp[0]=='\r'&&chrTemp[1]=='\n'&&chrTemp[2]=='+'&&chrTemp[3]=='Q'&&chrTemp[4]=='C'&&chrTemp[5]=='C'&&chrTemp[6]=='I'&&chrTemp[7]=='D'&&chrTemp[8]==':'&&chrTemp[9]==' ')
            {
                if (usRxLength>=10+20+8)                      //    /r/n+QCCID：长10  ID长20  /r/n/r/nOK/r/n   长8
                {
                    if ((chrTemp[20+10]=='\r'&&chrTemp[20+10+1]=='\n')&&(chrTemp[20+10+4]=='O'&&chrTemp[20+10+5]=='K'))
                    {
                        memcpy(&let_info.qccid.qccid,&chrTemp[10],sizeof(let_info.qccid.qccid));
                        usRxLength -= 38;
                        memcpy(&chrTemp[0],&chrTemp[38],usRxLength);
                        printf("qccid: %s\n",let_info.qccid.qccid);
                    }
                    else
                    {
                        usRxLength -= 10;             //如果尾不正确，把头去掉继续
                        memcpy(&chrTemp[0],&chrTemp[10],usRxLength);
                    }
                }
            }
            else if (chrTemp[0]=='\r'&&chrTemp[1]=='\n'&&chrTemp[2]=='+'&&chrTemp[3]=='C'&&chrTemp[4]=='S'&&chrTemp[5]=='Q'&&chrTemp[6]==':'&&chrTemp[7]==' ')
            {
                if (usRxLength>=8+5+8)                      //    /r/n+CSQ：长8  数据长5    /r/n/r/nOK/r/n   长8
                {
                    if ((chrTemp[8+5]=='\r'&&chrTemp[8+5+1]=='\n')&&(chrTemp[8+5+4]=='O'&&chrTemp[8+5+5]=='K'))
                    {
                        sscanf((const char*)(ucData+8),"%d,%d",&let_info.csq.rssi,&let_info.csq.ber);
                        usRxLength -= 8+5+8;
                        memcpy(&chrTemp[0],&chrTemp[8+5+8],usRxLength);
                        printf("let_info.csq.rssi: %d,let_info.csq.ber: %d\n",let_info.csq.rssi,let_info.csq.ber);
                    }
                    else
                    {
                        usRxLength -= 8;             //如果尾不正确，把头去掉继续
                        memcpy(&chrTemp[0],&chrTemp[8],usRxLength);
                    }
                }
            }
            else if (chrTemp[0]=='\r'&&chrTemp[1]=='\n'&&chrTemp[2]=='+'&&chrTemp[3]=='Q'&&chrTemp[4]=='S'&&chrTemp[5]=='P'&&chrTemp[6]=='N'&&chrTemp[7]==':'&&chrTemp[8]==' ')
            {
                if (usRxLength>=9+38+8)                 //    /r/n+CSQ：长9  数据长38    /r/n/r/nOK/r/n   长8
                {
                    if((chrTemp[9+38]=='\r'&&chrTemp[9+38+1]=='\n')&&(chrTemp[9+38+4]=='O'&&chrTemp[9+38+5]=='K'))
                    {
                        //memcpy(&let_info.qspn.business_name,&chrTemp[9+1],sizeof(let_info.qccid.qccid));
                        //sscanf((const char*)(ucData+9),"\" %s\",\"%s\",",let_info.qspn.business_name,let_info.qspn.net_plmn);
                        memcpy(&let_info.qspn.business_name,&chrTemp[9+1],12);
                        usRxLength -= 9+38+8;
                        memcpy(&chrTemp[0],&chrTemp[9+38+8],usRxLength);
                        printf("let_info.qspn.business_name: %s\n",let_info.qspn.business_name);
                    }
                    else
                    {
                        usRxLength -= 9;             //如果尾不正确，把头去掉继续
                        memcpy(&chrTemp[0],&chrTemp[9],usRxLength);
                    }
                }
            }
            else
            {
                usRxLength = usRxLength-1;
                memcpy(&chrTemp[0],&chrTemp[1],usRxLength);
                continue;                      //continue 退出这次循环执行下次
            }
    }
    return usRxLength;
}
/**
 * @brief 读取4G模块信息
 * @param  name             My Param doc
 * @return int 
 */
int read_lte_info(PORT_NAME name)
{
    unsigned char send_data[LET_MAX][32]={{"AT+QGSN\r\n"},{"AT+QCCID\r\n"},{"AT+CSQ\r\n"},{"AT+QSPN\r\n"}};
    int len = 0;
    static LTE_ENUM lte_enum = QGSN;
    switch (lte_enum)
    {
    case QGSN:
        len = bsp_write_array(name,send_data[QGSN],strlen((const char *)send_data[QGSN]));
        lte_enum = QCCID;
        break;
    case QCCID:
        len = bsp_write_array(name,send_data[QCCID],strlen((const char *)send_data[QCCID]));
        lte_enum = CSQ;
        break;
    case CSQ:
        len = bsp_write_array(name,send_data[CSQ],strlen((const char *)send_data[CSQ]));
        lte_enum = QSPN;
        break;
    case QSPN:
        len = bsp_write_array(name,send_data[QSPN],strlen((const char *)send_data[QSPN]));
        lte_enum = QGSN;
        break;
    default:
        printf("LET read_mode error: %d\n",lte_enum);
        break;
    }
    return len;
}


