#include <stdio.h>
#include <sys/time.h>
#include <unistd.h>
#include "config.h"
#include "stop_mode.h"
#include "bsp_car_drive.h"


void stop_mode(void)
{
    remote_control(CARDRIVE,0,0);
    sleep(1);
}