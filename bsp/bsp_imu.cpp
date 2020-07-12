/**
 * @file bsp_imu.cpp
 * @brief 
 * @author zhaokangxu (zhaokangxu@zhskg.cn)
 * @version 1.0
 * @date 2020-06-11
 * 
 * @copyright Copyright (c) 2020-2024  智慧式集团
 * 
 * @par 修改日志:
 * <table>
 * <tr><th>Date       <th>Version <th>Author  <th>Description
 * <tr><td>2020-06-11 <td>1.0     <td>zhaokangxu     <td>内容
 * </table>
 */

#include <stdio.h>
#include "bsp_imu.h"
#include "string.h"

extern _imu_angle imu_angle;
CJY901 ::CJY901 ()
{
}
void get_imu_angle(_imu_angle *p_imu_angle)
{
	if(JY901.stcAngle.Angle[2]<0)
		p_imu_angle->yaw = -JY901.stcAngle.Angle[2]/32768.f*180;
	else
		p_imu_angle->yaw = -JY901.stcAngle.Angle[2]/32768.f*180+360;
	p_imu_angle->roll 	= JY901.stcAngle.Angle[0]/32768.f*180;
	p_imu_angle->pitch 	= JY901.stcAngle.Angle[1]/32768.f*180;
	//printf("p_imu_angle: %f\n",p_imu_angle->yaw);
}
void CJY901 ::CopeSerialData(char ucData[],unsigned short usLength)
{
	static unsigned char chrTemp[2000];
	static unsigned char ucRxCnt = 0;	
	static unsigned short usRxLength = 0;
	unsigned char constsum=0;
	int count=0;
	if (usRxLength<=11)   //避免处理错误导致数组越界
	{
		memcpy(chrTemp+usRxLength,ucData,usLength);
		usRxLength += usLength;
	}
	else
	{
		memcpy(chrTemp,ucData,usLength);
		usRxLength += usLength;
		printf("imu handle receive error\n");
		perror("imu handle receive error\n");
	}
    while (usRxLength >= 11)
    {
        if (chrTemp[0] != 0x55)
        {
			usRxLength--;
			printf("imu head error\n");
			memcpy(&chrTemp[0],&chrTemp[1],usRxLength);                        
            continue;
        }
        for(int i=0;i<10;i++)
        {
        	constsum+=chrTemp[i];
        }
        if(constsum!=chrTemp[10])
        {
			usRxLength--;
			printf("check error\n");
			memcpy(&chrTemp[0],&chrTemp[1],usRxLength);  
			continue;
        }
		switch(chrTemp[1])
		{
			case 0x50:	memcpy(&stcTime,&chrTemp[2],8);count++;break;
			case 0x51:	
				stcAcc.a[0] = (chrTemp[2]|(unsigned short)chrTemp[3]<<8);
				stcAcc.a[1] = (chrTemp[4]|(unsigned short)chrTemp[5]<<8);
				stcAcc.a[2] = (chrTemp[6]|(unsigned short)chrTemp[7]<<8);
				stcAcc.T    = (chrTemp[8]|(unsigned short)chrTemp[9]<<8);
				count++;break;
			case 0x52:
				stcGyro.w[0] = (chrTemp[2]|(unsigned short)chrTemp[3]<<8);
				stcGyro.w[1] = (chrTemp[4]|(unsigned short)chrTemp[5]<<8);
				stcGyro.w[2] = (chrTemp[6]|(unsigned short)chrTemp[7]<<8);
				stcGyro.T    = (chrTemp[8]|(unsigned short)chrTemp[9]<<8);
				count++;break;
			case 0x53:	
				stcAngle.Angle[0] = (chrTemp[2]|(unsigned short)chrTemp[3]<<8);
				stcAngle.Angle[1] = (chrTemp[4]|(unsigned short)chrTemp[5]<<8);
				stcAngle.Angle[2] = (chrTemp[6]|(unsigned short)chrTemp[7]<<8);
				stcAngle.T        = (chrTemp[8]|(unsigned short)chrTemp[9]<<8);
				count++;break;
			case 0x54:	memcpy(&stcMag,&chrTemp[2],8);count++;break;
			case 0x55:	memcpy(&stcDStatus,&chrTemp[2],8);count++;break;
			case 0x56:	memcpy(&stcPress,&chrTemp[2],8);count++;break;
			case 0x57:	memcpy(&stcLonLat,&chrTemp[2],8);count++;break;
			case 0x58:	memcpy(&stcGPSV,&chrTemp[2],8);count++;break;
			case 0x59:
				stcQuaternion.Q[0] = (chrTemp[2]|(unsigned short)chrTemp[3]<<8);
				stcQuaternion.Q[1] = (chrTemp[4]|(unsigned short)chrTemp[5]<<8);
				stcQuaternion.Q[2] = (chrTemp[6]|(unsigned short)chrTemp[7]<<8);
				stcQuaternion.Q[3] = (chrTemp[8]|(unsigned short)chrTemp[9]<<8);
				count++;break;
			// case 0x50:	memcpy(&stcTime,&chrTemp[2],8);count++;break;
			// case 0x51:	memcpy(&stcAcc,&chrTemp[2],8);count++;break;
			// case 0x52:	memcpy(&stcGyro,&chrTemp[2],8);count++;break;
			// case 0x53:	memcpy(&stcAngle,&chrTemp[2],8);count++;break;
			// case 0x54:	memcpy(&stcMag,&chrTemp[2],8);count++;break;
			// case 0x55:	memcpy(&stcDStatus,&chrTemp[2],8);count++;break;
			// case 0x56:	memcpy(&stcPress,&chrTemp[2],8);count++;break;
			// case 0x57:	memcpy(&stcLonLat,&chrTemp[2],8);count++;break;
			// case 0x58:	memcpy(&stcGPSV,&chrTemp[2],8);count++;break;
			// case 0x59:	memcpy(&stcQuaternion,&chrTemp[2],8);count++;break;
		}
		usRxLength -= 11;
		constsum=0;
		memcpy(&chrTemp[0],&chrTemp[11],usRxLength);
    }
	//printf("angle : %f,count: %d\n",JY901.stcAngle.Angle[2]/32768.f*180,count);
}
CJY901 JY901 = CJY901();


        //     JY901.CopeSerialData(tmpdata,data_size);   //JY901 imu 库函数
            
    	// 	//四元数位姿,所有数据设为固定值，可以自己写代码获取ＩＭＵ的数据，然后进行传递
    	//  //     geometry_msgs::Quaternion q =\
    	// 	// tf::createQuaternionMsgFromRollPitchYaw(
    	// 	// 	(float)JY901.stcAngle.Angle[1]/32768*180,\
    	// 	//  	(float)JY901.stcAngle.Angle[2]/32768*180,\
    	// 	//  	(float)JY901.stcAngle.Angle[0]/32768*180);//返回四元数

    	//     if(constnum>=1)
        //     {
        //         constnum--;
        //         a[0] = 1-(float)JY901.stcQuaternion.Q[0]/32768;
        //         a[1] = (float)JY901.stcQuaternion.Q[1]/32768;
        //         a[2] = (float)JY901.stcQuaternion.Q[2]/32768;
        //         a[3] = (float)JY901.stcQuaternion.Q[3]/32768;
        //     }
    	//     imu.orientation.x = (float)JY901.stcQuaternion.Q[0]/32768+a[0];
     	//   	imu.orientation.y = (float)JY901.stcQuaternion.Q[1]/32768-a[1];
      	//  	imu.orientation.z = (float)JY901.stcQuaternion.Q[2]/32768-a[2];
     	// 	imu.orientation.w =(float)JY901.stcQuaternion.Q[3]/32768-a[3];
        //     imu.orientation_covariance=imu_fore_covariance;

        //    	//ROS_INFO("q0:%f", imu.orientation.x);
    	// 	// ROS_INFO("q1:%f", imu.orientation.y);
    	// 	// ROS_INFO("q2:%f", imu.orientation.z);
    	// 	// ROS_INFO("q3:%f", imu.orientation.w);


            
        //     //角速度
    	// 	imu.angular_velocity.x = (float)JY901.stcGyro.w[0]/32768*2000/180*3.1415926;
    	// 	imu.angular_velocity.y = (float)JY901.stcGyro.w[1]/32768*2000/180*3.1415926;
    	// 	imu.angular_velocity.z = (float)JY901.stcGyro.w[2]/32768*2000/180*3.1415926;
        //     imu.angular_velocity_covariance=imu_angle_covariance;

    	// 	//线加速度
    	// 	imu.linear_acceleration.x = (float)JY901.stcAcc.a[0]/32768*16*10;
    	// 	imu.linear_acceleration.y = (float)JY901.stcAcc.a[1]/32768*16*10;
        //     imu.linear_acceleration.z = (float)JY901.stcAcc.a[2]/32768*16*10;
        //     imu.linear_acceleration_covariance=imu_liner_covariance;
        //     /*
    	// 	if(b[0]!=imu.angular_velocity.x)
    	// 	  ROS_INFO("angle_x:%f,%d", imu.angular_velocity.x,JY901.stcGyro.w[0]);
        //     if(b[1]!=imu.angular_velocity.y)
        //         ROS_INFO("angle_y:%f,%d", imu.angular_velocity.y,JY901.stcGyro.w[1]);
        //     if(b[2]!=imu.angular_velocity.z)
        //         ROS_INFO("angle_z:%f,%d", imu.angular_velocity.z,JY901.stcGyro.w[2]);

