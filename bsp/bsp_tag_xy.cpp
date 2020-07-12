#include <stdio.h>
#include <math.h>
#include "bsp_tag_xy.h"
#include "config.h"

// double round(double d){
//   return floor(d + 0.5);
// }
//角度是与X轴正方像的夹角，从x轴正方向顺时针角度增大，此处为了和GPS角度统一
//atan2(y,x)  y>0时，角度从X正方向逆时针从0增大到180,y<0时，角度从X正方向顺时针从0增大到-180
double tag_getAngle(double x1, double y1, double x2, double y2) 
{
    double angle = 0;
    double x = x2-x1;
    double y = y2-y1;
    //printf("x:%lf,y:%lf,atan2:%lf\n",x,y,atan2(y,x)*180/PI);
    if (x > 0 && y > 0) 
    {                              //一象限
        //printf("x:%lf,y:%lf,atan2:%lf\n",x,y,atan2(y,x)*180/PI);
        angle = atan2(y,x)/PI*180;
        angle = 360-angle;
    }
    else if(x >0 && y<0){          //四象限
        //printf("x:%lf,y:%lf,atan2:%lf\n",x,y,atan2(y,x)*180/PI);
        angle = atan2(y,x)/PI*180;
        angle = -angle;
    }
    else if (x < 0 && y < 0) {    //三象限
        //printf("x:%lf,y:%lf,atan2:%lf\n",x,y,atan2(y,x)*180/PI);
        angle = atan2(y,x)/PI*180;
        angle = -angle;
    }
    else if (x < 0 && y > 0)      //二象限
    {
        //printf("x:%lf,y:%lf,atan2:%lf\n",x,y,atan2(y,x)*180/PI);
        angle = atan2(y,x)/PI*180;
        angle = 360-angle;
    }
    else if (x==0&&y>0)
    {
        angle = 270;
    }
    else if (x==0&&y<0)
    {
        angle = 90;
    }
    else if (x>0&&y==0)
    {
        angle = 0;
    }
    else if (x<0&&y==0)
    {
        angle = 180;
    }
    //printf("ANGLE: %f\n",angle);
    return angle;
}
// //角度是与X轴正方像的夹角，逆时针角度增大
// double tag_getAngle(double x1, double y1, double x2, double y2) 
// {
//     double angle= 0;
//     double x = x2-x1;
//     double y = y2-y1;
//     //printf("x:%lf,y:%lf,atan2:%lf\n",x,y,atan2(y,x)*180/PI);
//     if (x > 0 && y > 0) 
//     {                              //一
//         //printf("x:%lf,y:%lf,atan2:%lf\n",x,y,atan2(y,x)*180/PI);
//         angle = atan2(y,x)/PI*180;
//     }
//     else if(x >0 && y<0){          //四
//         //printf("x:%lf,y:%lf,atan2:%lf\n",x,y,atan2(y,x)*180/PI);
//         angle = atan2(y,x)/PI*180;
//         angle = angle + 180*2;
//     }
//     else if (x < 0 && y < 0) {    //三
//         //printf("x:%lf,y:%lf,atan2:%lf\n",x,y,atan2(y,x)*180/PI);
//         angle = atan2(y,x)/PI*180;
//         angle = angle + 180*2;
//     }
//     else if (x < 0 && y > 0)     //二
//     {
//         //printf("x:%lf,y:%lf,atan2:%lf\n",x,y,atan2(y,x)*180/PI);
//         angle = atan2(y,x)/PI*180;
//     }
//     else if (x==0&&y>0)
//     {
//         angle = 90;
//     }
//     else if (x==0&&y<0)
//     {
//         angle = 270;
//     }
//     else if (x>0&&y==0)
//     {
//         angle = 0;
//     }
//     else if (x<0&&y==0)
//     {
//         angle = 180;
//     }
//     return angle;
// }
//根据xy计算两点距离，返回值单位为cm
double tag_get_distance(double x1, double y1, double x2, double y2)
{
  double dis=0;
  dis = sqrt(pow(x2-x1,2)+pow(y2-y1,2));
  return dis*100;
}
