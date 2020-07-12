#ifndef __BSP_UART_H__
#define __BSP_UART_H__
#include "config.h"

//共20个串口   MAX_PORT = 20;
#define UART_PORT_CFG                           \
{                                               \
    TAG1_CFG,                                   \
    TAG2_CFG,                                   \
    GPS_CFG,                                    \
    IMU_CFG,                                    \
    CARDRIVE_CFG,                               \
    BLE_CFG,                                    \
    SINGLE_LIDAR1_CFG,                          \
    SINGLE_LIDAR2_CFG,                          \
    MULT_LIDAR_CFG,                             \
    HPS_3D160_CFG,                              \
    FRESH_DRIVE_CFG,                            \
    UWB1_CFG,                                   \
    UWB2_CFG,                                   \
    CAMERA1_CFG,                                \
    CAMERA2_CFG,                                \
    AUTO_CHARGE_CFG,                            \
    EC20_0_CFG,                                 \
    EC20_1_CFG,                                 \
    EC20_2_CFG,                                 \
    EC20_3_CFG                                  \
}
typedef struct
{
    const char *name; 
    int fd;
    int baudtate;
    int nBits;
    char nEvent;
    int nStop;
}uart_port_t;
#define TAG1_CFG                            \
{                                           \
    .name = "/dev/TAG1",                    \
    .fd = 0,                                \
    .baudtate = 115200,                     \
    .nBits = 8,                             \
    .nEvent = 'N',                          \
    .nStop = 1                              \
}
#define TAG2_CFG                            \
{                                           \
    .name = "/dev/TAG2",                    \
    .fd = 0,                                \
    .baudtate = 115200,                     \
    .nBits = 8,                             \
    .nEvent = 'N',                          \
    .nStop = 1                              \
}
#define GPS_CFG                             \
{                                           \
    .name = "/dev/GPS",                     \
    .fd = 0,                                \
    .baudtate = 115200,                     \
    .nBits = 8,                             \
    .nEvent = 'N',                          \
    .nStop = 1                              \
}
#define IMU_CFG                             \
{                                           \
    .name = "/dev/IMU",                     \
    .fd = 0,                                \
    .baudtate = 115200,                     \
    .nBits = 8,                             \
    .nEvent = 'N',                          \
    .nStop = 1                              \
}
#define CARDRIVE_CFG                        \
{                                           \
    .name = "/dev/CARDRIVE",                \
    .fd = 0,                                \
    .baudtate = 115200,                     \
    .nBits = 8,                             \
    .nEvent = 'N',                          \
    .nStop = 1                              \
}
#define BLE_CFG                             \
{                                           \
    .name = "/dev/BLE",                     \
    .fd = 0,                                \
    .baudtate = 115200,                     \
    .nBits = 8,                             \
    .nEvent = 'N',                          \
    .nStop = 1                              \
}
#define SINGLE_LIDAR1_CFG                   \
{                                           \
    .name = "/dev/SINGLE_LIDAR1",           \
    .fd = 0,                                \
    .baudtate = 115200,                     \
    .nBits = 8,                             \
    .nEvent = 'N',                          \
    .nStop = 1                              \
}
#define SINGLE_LIDAR2_CFG                   \
{                                           \
    .name = "/dev/SINGLE_LIDAR1",           \
    .fd = 0,                                \
    .baudtate = 115200,                     \
    .nBits = 8,                             \
    .nEvent = 'N',                          \
    .nStop = 1                              \
}
#define MULT_LIDAR_CFG                      \
{                                           \
    .name = "/dev/MULT_LIDAR",              \
    .fd = 0,                                \
    .baudtate = 115200,                     \
    .nBits = 8,                             \
    .nEvent = 'N',                          \
    .nStop = 1                              \
}
#define HPS_3D160_CFG                       \
{                                           \
    .name = "/dev/HPS_3D160",               \
    .fd = 0,                                \
    .baudtate = 115200,                     \
    .nBits = 8,                             \
    .nEvent = 'N',                          \
    .nStop = 1                              \
}
#define FRESH_DRIVE_CFG                     \
{                                           \
    .name = "/dev/FRESH_DRIVE",             \
    .fd = 0,                                \
    .baudtate = 115200,                     \
    .nBits = 8,                             \
    .nEvent = 'N',                          \
    .nStop = 1                              \
}
#define UWB1_CFG                            \
{                                           \
    .name = "/dev/UWB1",                    \
    .fd = 0,                                \
    .baudtate = 115200,                     \
    .nBits = 8,                             \
    .nEvent = 'N',                          \
    .nStop = 1                              \
}
#define UWB2_CFG                            \
{                                           \
    .name = "/dev/UWB2",                    \
    .fd = 0,                                \
    .baudtate = 115200,                     \
    .nBits = 8,                             \
    .nEvent = 'N',                          \
    .nStop = 1                              \
}
#define CAMERA1_CFG                         \
{                                           \
    .name = "/dev/CAMERA1",                 \
    .fd = 0,                                \
    .baudtate = 115200,                     \
    .nBits = 8,                             \
    .nEvent = 'N',                          \
    .nStop = 1                              \
}
#define CAMERA2_CFG                         \
{                                           \
    .name = "/dev/CAMERA2",                 \
    .fd = 0,                                \
    .baudtate = 115200,                     \
    .nBits = 8,                             \
    .nEvent = 'N',                          \
    .nStop = 1                              \
}
#define AUTO_CHARGE_CFG                     \
{                                           \
    .name = "/dev/AUTO_CHARGE",             \
    .fd = 0,                                \
    .baudtate = 115200,                     \
    .nBits = 8,                             \
    .nEvent = 'N',                          \
    .nStop = 1                              \
}
#define EC20_0_CFG                          \
{                                           \
    .name = "/dev/ttyUSB0",                 \
    .fd = 0,                                \
    .baudtate = 115200,                     \
    .nBits = 8,                             \
    .nEvent = 'N',                          \
    .nStop = 1                              \
}
#define EC20_1_CFG                          \
{                                           \
    .name = "/dev/ttyUSB1",                 \
    .fd = 0,                                \
    .baudtate = 115200,                     \
    .nBits = 8,                             \
    .nEvent = 'N',                          \
    .nStop = 1                              \
}
#define EC20_2_CFG                          \
{                                           \
    .name = "/dev/ttyUSB2",                 \
    .fd = 0,                                \
    .baudtate = 115200,                     \
    .nBits = 8,                             \
    .nEvent = 'N',                          \
    .nStop = 1                              \
}
#define EC20_3_CFG                          \
{                                           \
    .name = "/dev/ttyUSB3",                 \
    .fd = 0,                                \
    .baudtate = 115200,                     \
    .nBits = 8,                             \
    .nEvent = 'N',                          \
    .nStop = 1                              \
}
void *uart_selecte(void * arg);
int uart_init(void);
int bsp_write_array(PORT_NAME port_num,unsigned char *array,int lens);



#endif

