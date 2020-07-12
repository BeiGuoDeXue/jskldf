#ifndef __BSP_PID_H__
#define __BSP_PID_H__
typedef struct
{
	float Kp;                       //Proportional
	float Ki;                       //Integral
	float Kd;                       //Derivative

	float Ek;                       //e(k)
	float Ek1;                      //e(k-1)
	float Ek2;                      //e(k-2)
}PID_IncTypeDef;
void InitPID(void);
float PID_Inc(float SetValue, PID_IncTypeDef *PID);
#endif

