#include <stdio.h>
#include <sys/time.h>
#include <unistd.h>
#include "config.h"
#include "accompany_mode.h"
#include "bsp_car_drive.h"

void accompany_mode(void)
{
    printf("敬请期待！\n");
    remote_control(CARDRIVE,0,0);
    sleep(1);
}
