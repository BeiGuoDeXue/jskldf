/**
 * @file carstat.cpp
 * @brief 
 * @author xiaozhong (xiaozhong@zhskg.cn)
 * @version 1.0
 * @date 2020-06-11
 * 
 * @copyright Copyright (c) 2020-2024  智慧式集团
 * 
 * @par 修改日志:
 * 2020-06-11 1.0     xiaozhong     
 */

/**
/////////////////////////////////////////////////////////////////////
该文件包含设备的状态
1.获取设备的内存信息
2.获取设备的设备的状态
3.获取设备的CPU信息
**/

#include <cstring>
#include <stdint.h>
#include <stdlib.h>

#include "device.h"
#include "MQTTAsync.h"
#include "dbg.h"

/*
 * cpuid, 通过 getDeviceId()获取
 */
char cpuid[50];


uint8_t get_MemInfo(int *FreeMem, int *UsedMem)
{
    char sStatBuf[256];
    FILE* fp;
    char* line;
    
    system("free -m | awk '{print $2,$3}' > mem");//获取内存信息保存到mem里面
    
    memset(sStatBuf, 0, 256);
    fp = fopen("mem", "rb");
    if(fp < 0)
        return -1;
    fread(sStatBuf, 1, sizeof(sStatBuf), fp);
    line = strstr(sStatBuf, "\n");
    *UsedMem = atoi(line);
    line = strstr(line, " ");
    *FreeMem = atoi(line);
    DBG("Used %dM\nFree %dM\n", UsedMem, FreeMem);
    
    return 0;
}
/*
 * 获取底层设备状态
 *
 */
int get_machine_stat(PT_SYS_DEVICE_MSG msg)
{
    int temp = 0;
    int vol = 0;
    int freemem = 0;
    int usedmem = 0;

    msg->temp = 0;      // get_machine_temp();
    msg->voltage = 220; //get_adc_voltage();
    get_MemInfo(&msg->freemem, &msg->usedmem);

    if (msg->temp > 80)
    {
        msg->stat = high_temp;
    }
    else if (msg->voltage < 1000)
    {
        msg->stat = low_vol;
    }
    else if (msg->freemem == 0)
    {
        msg->stat = mem_overflow;
    }
    else
    {
        msg->stat = normal;
    }

    return 0;
}
  
std::string getDeviceId(void)
{
    std::string str_buf;
    FILE *fp = NULL;
    char buf[50] = "";
    char tmp[50], tmp1[50];

    fp = popen("cat /proc/cpuinfo", "r");
    if (fp != NULL)
    {
        while (fgets(buf, sizeof(buf), fp) != 0)
        {
            if (strstr(buf, "Serial     :") != NULL)
            {     
#if 1
                memset(cpuid, 0, sizeof(cpuid));
                sscanf(buf, "%s %s %s", tmp, tmp1, cpuid);
                str_buf = cpuid;
#else
                sprintf(cpuid, "%s", "833b4d127bfc41bb9e328f3a0f8b3b85");
                str_buf = cpuid;
#endif

                memset(buf, 0x0, sizeof(buf));
                break;
            }

            memset(buf, 0x0, sizeof(buf));
        }
    }
    DBG("[%d]: cpuid is %s\n", __LINE__, cpuid);

    pclose(fp);
    return str_buf;
}