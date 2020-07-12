#include "bsp_pid.h"        

PID_IncTypeDef PID_Data;

void InitPID(void)  //OK2
{
//	PID_Data.Kp = 0.26;
	PID_Data.Kp = 0.4;//0.35;
	PID_Data.Ki = 0;       //0.008;
	PID_Data.Kd = 0.5;//0.2;     //0.5;
	PID_Data.Ek = 0;
	PID_Data.Ek1 = 0;
	PID_Data.Ek2 = 0;
}
float PID_Inc(float SetValue, PID_IncTypeDef *PID)
{
	float PIDInc=0;
	static float PIDADD_count;
	PID->Ek = SetValue;
	if((PIDADD_count<=600)&&(PIDADD_count>=-600))
	{
		PIDADD_count+=SetValue;
	}
//	PIDInc = PID->Kp * (PID->Ek-PID->Ek1) + PID->Ki * PID->Ek + PID->Kd *(PID->Ek-2*PID->Ek1+PID->Ek2); //增量式
//		PIDInc = PID->Kp * PID->Ek + PID->Ki * (PID->Ek+PID->Ek1+PID->Ek2) + PID->Kd *(PID->Ek-PID->Ek1);   //weizhishi
	PIDInc = PID->Kp * PID->Ek + PID->Ki *PIDADD_count + PID->Kd *(PID->Ek-PID->Ek1);        //位置式带积分
	// PIDInc=25.34f*PIDInc;     //(Turn_Right-Turn_Mid)/38°=25.34
	// PWM_Timer2=Turn_Mid+PIDInc;
	if(PIDInc>30)  PIDInc =  30;
	if(PIDInc<-30) PIDInc = -30;
	PID->Ek2 = PID->Ek1;
	PID->Ek1 = PID->Ek;
	return PIDInc;
}