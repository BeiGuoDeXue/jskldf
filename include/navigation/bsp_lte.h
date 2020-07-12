/**
 * @file bsp_lte.h
 * @brief 
 * @author zhaokangxu (zhaokangxu@zhskg.cn)
 * @version 1.0
 * @date 2020-07-01
 * 
 * @copyright Copyright (c) 2020-2024  智慧式集团
 * 
 * @par 修改日志:
 * <table>
 * <tr><th>Date       <th>Version <th>Author  <th>Description
 * <tr><td>2020-07-01 <td>1.0     <td>zhaokangxu     <td>内容
 * </table>
 */
#ifndef __BSP_LTE_H__
#define __BSP_LTE_H__
#include "config.h"
/**
 * send:AT+CSQ
 * receive:+CSQ:<rssi>,<ber>
 * rssi:    0               小于等于-113dB
 *          1               -111dBm
 *          2..30           -109,,, -53dBm
 *          31              大于等于-51dBm
 *          99              未知或不可测
 * ber:     (百分比格式)
 *          0..7            RXQUA值
 *          99              未知或不可测
 */
typedef enum
{
    QGSN,                   //TA上报ME设备的IMEI号（国际移动台设备识别码）
    QCCID,                  //SIM卡的CCID
    CSQ,                    //信号质量<rssi> <ber> 
    QSPN,                   //读取网络运营商名称
    LET_MAX
}LTE_ENUM;
typedef enum 
{
    SEND,                   //发送完成
    RE_SUCCESS,             //接收正确
    RE_FAIL,                //接收失败
    Equipment_damage        //设备损坏
}LTE_STAT_T;

typedef struct 
{
    char qgsn[15];           //长度固定为15
    LTE_STAT_T qgsn_stat;
}qgsn_t;
typedef struct 
{
    char qccid[20];         //长度固定为20
    LTE_STAT_T qccid_stat;
}qccid_t;
typedef struct 
{
    int rssi;
    int ber;
    LTE_STAT_T csq_stat;
}csq_t;
typedef struct 
{
    char business_name[50];
    char net_plmn[50];
    char net_speed[50];
    LTE_STAT_T qspn_stat;
}qspn_t;
typedef struct 
{
    qgsn_t qgsn;
    qccid_t qccid;
    csq_t csq;
    qspn_t qspn;
}let_info_t;

int receive_lte(unsigned char *ucData,int usLength);
int read_lte_info(PORT_NAME name);
#endif


