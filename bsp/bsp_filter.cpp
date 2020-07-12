#include <stdio.h>
#include <math.h>
#include "bsp_filter.h"

//中值滤波,请确保长度为奇数时，有效数据取奇数个，为偶数有效数据为偶数个
double GetMedianNum(double * bArray, int iFilterLen,int iUselens)
{
	int i,j;// 循环变量
	double bTemp = 0;
	double add_num = 0;
	//保证数组大于0,且取中间值不大于数组长度
    if ((iFilterLen>=iUselens)&&(iFilterLen>0)&&(iUselens>0))
    {
        // 用冒泡法对数组进行排序   从小到大排序
        for (j = 0; j < iFilterLen - 1; j ++)
        {
            for (i = 0; i < iFilterLen - j - 1; i ++)
            {
                if (bArray[i] > bArray[i + 1])
                {
                    // 互换
                    bTemp = bArray[i];
                    bArray[i] = bArray[i + 1];
                    bArray[i + 1] = bTemp;
                }
            }
        }
        // 计算中值
        if (((iFilterLen & 1) > 0)&&((iUselens & 1) > 0))
        {
            // 数组有奇数个元素，有效数据也为奇数，返回中间有用数据均值
            for (int i = (iFilterLen-iUselens)/2; i <=(iFilterLen+iUselens-2)/2; i++)
            {
                add_num += bArray[i];
            }
            add_num = add_num/iUselens;
        }
        else if (((iFilterLen & 1) == 0)&&((iUselens & 1) == 0))
        {
            // 数组有偶数个元素，有效数据也为偶数，返回中间几个个元素平均值
            for (int i = (iFilterLen-iUselens)/2; i <=(iFilterLen+iUselens)/2-1; i++)
            {
                add_num += bArray[i];
            }
            add_num = add_num/iUselens;
        }
        else 
        {
            // 数组有奇数个元素，有效数据为偶数，返回中间几个个元素平均值
            // 或者数组有偶数个元素，有效数据为奇数，返回中间几个个元素平均值，不太具有中值滤波意义
            printf("请输入合适的有效数据iFilterLen：%d,iUselens: %d\n",iFilterLen,iUselens);
            return -1;
        }
    }
    else
    {
        printf("数组长度错误iFilterLen：%d,iUselens: %d\n",iFilterLen,iUselens);
        return -1;
    }
	return add_num;
}
double uwb_filter_buff(double parameter)
{
    static double buff[FILTER_BUFF_LEN];
    static int first_filter = FILTER_BUFF_LEN;
    int buff_len;
    static int num = 0;
    double angle = 0;
    if (num>=FILTER_BUFF_LEN)
    {
        num = 0;
    }
    buff[num++] = parameter;
    if (first_filter>0)    //如果数据不足滤波的个数，先取均值滤波
    {
        first_filter--;
        for (int i = 0; i < num; i++)
        {
            angle += buff[i];
        }
        angle = angle/num;
        return angle;
    }
    else                   //开始中值滤波
    {
        angle = GetMedianNum(buff,FILTER_BUFF_LEN,FILTER_USE_LEN);
    }
    return angle;
}
void uwblog_median_filter(const uwblog_addr_t *p_uwblog,int index,double *filter_buff,long log_maxlen)
{
    int num = 0;
    num = index;
    double x_buff[UWBLOG_FILTER_BUFFLEN];
    double y_buff[UWBLOG_FILTER_BUFFLEN];
    double z_buff[UWBLOG_FILTER_BUFFLEN];
    if (UWBLOG_FILTER_BUFFLEN>=3)
    {
        if ((UWBLOG_FILTER_BUFFLEN&1)>0)       //奇数
        {
            if ((num >=(UWBLOG_FILTER_BUFFLEN-1)/2)&&(num<(log_maxlen-UWBLOG_FILTER_BUFFLEN/2)))
            {
                num = num - (UWBLOG_FILTER_BUFFLEN-1)/2;
                for (int i = 0; i <UWBLOG_FILTER_BUFFLEN;i++)
                {
                    x_buff[i] = p_uwblog[num].x;
                    y_buff[i] = p_uwblog[num].y;
                    z_buff[i] = p_uwblog[num].z;
                    num++;
                }
                filter_buff[0] = GetMedianNum(x_buff,UWBLOG_FILTER_BUFFLEN,UWBLOG_FILTER_USELEN);
                filter_buff[1] = GetMedianNum(y_buff,UWBLOG_FILTER_BUFFLEN,UWBLOG_FILTER_USELEN);
                filter_buff[2] = GetMedianNum(z_buff,UWBLOG_FILTER_BUFFLEN,UWBLOG_FILTER_USELEN);
            }
            else if((num<(UWBLOG_FILTER_BUFFLEN-1)/2))   //没有滤波窗口大,特殊处理
            {
                for (int i = 0; i < num; i++)
                {
                    x_buff[i] += p_uwblog[i].x;
                    y_buff[i] += p_uwblog[i].y;
                    z_buff[i] += p_uwblog[i].z;
                }
                filter_buff[0] = GetMedianNum(x_buff,num,(num&1>0 ? (num+1)/2-1 : num/2));
                filter_buff[1] = GetMedianNum(y_buff,num,(num&1>0 ? (num+1)/2-1 : num/2));
                filter_buff[2] = GetMedianNum(z_buff,num,(num&1>0 ? (num+1)/2-1 : num/2));
            }
            else if(num>=(log_maxlen-UWBLOG_FILTER_BUFFLEN/2)) //剩余数据不足窗口大小,直接返回原始值
            {
                filter_buff[0] = p_uwblog[num].x;
                filter_buff[1] = p_uwblog[num].y;
                filter_buff[2] = p_uwblog[num].z;
            }
        }
        else if((UWBLOG_FILTER_BUFFLEN&1)==0)              //偶数
        {
            printf("\r\n窗口为偶数不合适，请重新选取为奇数: %d\r\n",UWBLOG_FILTER_BUFFLEN);
        }
    }
    else if((UWBLOG_FILTER_BUFFLEN>0)&&(UWBLOG_FILTER_BUFFLEN<3))    //如果窗口小于三，滤波没意义
    {
        filter_buff[0] = p_uwblog[num].x;
        filter_buff[1] = p_uwblog[num].y;
        filter_buff[2] = p_uwblog[num].z;
    }
    else
    {
        printf("\r\n滤波窗口大小设置错误: %d\r\n",UWBLOG_FILTER_BUFFLEN);
    }
}

