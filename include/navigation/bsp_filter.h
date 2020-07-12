#ifndef __BSP_FILTER_H__
#define __BSP_FILTER_H__

#include "bsp_readfile.h"



#define  FILTER_BUFF_LEN  5
#define  FILTER_USE_LEN  3
#define  UWBLOG_FILTER_BUFFLEN  5
#define  UWBLOG_FILTER_USELEN   5


void uwblog_median_filter(const uwblog_addr_t *p_uwblog,int index,double *filter_buff,long log_maxlen);
double GetMedianNum(double * bArray, int iFilterLen,int iUselens);
double uwb_filter_buff(double parameter);



#endif
