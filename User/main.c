#include "stm32f10x.h"                  // Device header
#include "Delay.h"
#include "OLED.h"
#include "LED.h"
#include "Timer.h"
#include "Key.h"
#include "MPU6050.h"
#include "Motor.h"
#include "Encoder.h"
#include "Serial.h"
#include "BlueSerial.h"
#include "PID.h"
#include <math.h>
#include <String.h>
#include <stdlib.h>

//所用变量
int16_t AX, AY, AZ, GX, GY, GZ;
uint8_t TimerErrorFlag;
uint16_t TimerCount;

int8_t KeyNum,RunFlag;

float AngleAcc;
float AngleGyro;
float Angle;

int16_t LeftPWM,RightPWM;
int16_t AvePWM,DifPWM;

float LeftSpeed,RightSpeed;
float AveSpeed,DifSpeed;

//各PID变量


//角度PID
PID_t AnglePID = {
	.Kp = 5.00,
	.Ki = 0.10,
	.Kd = 5.00,
	
	.OutOffset = 4,
	
	.OutMax = 100,
	.OutMin = -100,
	
	.ErrorIntMax = 600,
	.ErrorIntMin = -600,
	
};

//速度PID
PID_t SpeedPID = {
	.Kp = 2,
	.Ki = 0.05,
	.Kd = 0,
	
	.OutMax = 20,
	.OutMin = -20,
	
	.ErrorIntMax = 150,
	.ErrorIntMin = -150,
};

//转向PID
PID_t TurnPID = {
	.Kp = 7,
	.Ki = 3,
	.Kd = 0,
	
	.OutMax = 50,
	.OutMin = -50,

	.ErrorIntMax = 20,
	.ErrorIntMin = -20,	
};

int main(void)
{
	OLED_Init();
	MPU6050_Init();
	BlueSerial_Init();
	LED_Init();
	Key_Init();
	Motor_Init();
	Serial_Init();
	Encoder_Init();
	
	
	Timer_Init();
	
	while (1)
	{
//标志位（防止失控）		
		if(RunFlag){LED_ON();}else{LED_OFF();}
		KeyNum = Key_GetNum();
		if(KeyNum == 1)
		{
			//由RunFlag = !RunFlag;更改
			if(RunFlag == 1)
			{
//**				//初始化PID，防止倒地未断电的过度积分
				PID_Init(&AnglePID);
				PID_Init(&SpeedPID);
				PID_Init(&TurnPID);
				RunFlag = 0;
			}
			else
			{
				RunFlag = 1;
			}
		}
//OLED显示		
		OLED_Clear();
		OLED_Printf(0,0,OLED_6X8,"  Angle");
		OLED_Printf(0,8,OLED_6X8,"P:%05.2f",AnglePID.Kp);
		OLED_Printf(0,16,OLED_6X8,"I:%05.2f",AnglePID.Ki);
		OLED_Printf(0,24,OLED_6X8,"D:%05.2f",AnglePID.Kd);
		OLED_Printf(0,32,OLED_6X8,"T:%05.1f",AnglePID.Target);
		OLED_Printf(0,40,OLED_6X8,"A:%05.1f",Angle);
		OLED_Printf(0,48,OLED_6X8,"O:%05.1f",AnglePID.Out);
		OLED_Printf(0,56,OLED_6X8,"GY:%+06d",GY);
		OLED_Printf(56,56,OLED_6X8,"Offset:%02.0f",AnglePID.OutOffset);
		
		
		OLED_Printf(50,0,OLED_6X8,"  Speed");
		OLED_Printf(50,8,OLED_6X8,"%05.2f",SpeedPID.Kp);
		OLED_Printf(50,16,OLED_6X8,"%05.2f",SpeedPID.Ki);
		OLED_Printf(50,24,OLED_6X8,"%05.2f",SpeedPID.Kd);
		OLED_Printf(50,32,OLED_6X8,"%05.1f",SpeedPID.Target);
		OLED_Printf(50,40,OLED_6X8,"%05.1f",AveSpeed);
		OLED_Printf(50,48,OLED_6X8,"%05.1f",SpeedPID.Out);
		
		
		OLED_Printf(88,0,OLED_6X8,"  Turn");
		OLED_Printf(88,8,OLED_6X8,"%05.2f",TurnPID.Kp);
		OLED_Printf(88,16,OLED_6X8,"%05.2f",TurnPID.Ki);
		OLED_Printf(88,24,OLED_6X8,"%05.2f",TurnPID.Kd);
		OLED_Printf(88,32,OLED_6X8,"%05.1f",TurnPID.Target);
		OLED_Printf(88,40,OLED_6X8,"%05.1f",DifSpeed);
		OLED_Printf(88,48,OLED_6X8,"%05.1f",TurnPID.Out);
		
		
		OLED_Update();
		
//蓝牙：接收器传输到平衡车的数据解析（按键，滑杆，摇杆）		
		if (BlueSerial_RxFlag == 1)
		{
			
			char *Tag = strtok(BlueSerial_RxPacket, ",");
			if (strcmp(Tag, "key") == 0)//按键
			{
				char *Name = strtok(NULL, ",");
				char *Action = strtok(NULL, ",");
				if(strcmp(Name, "1") == 0 && strcmp(Action, "start") == 0)
				{
					RunFlag = 1;
				}
				else if(strcmp(Name, "2") == 0 && strcmp(Action, "shut") == 0)
				{
					RunFlag = 0;
				}

			}
			else if (strcmp(Tag, "slider") == 0)//滑杆
			{
				char *Name = strtok(NULL, ",");
				char *Value = strtok(NULL, ",");
				
				
				//角度调参
				if(strcmp(Name,"AngleKp") == 0)
				{
					AnglePID.Kp = atof(Value);
				}
				else if(strcmp(Name,"AngleKi") == 0)
				{
					AnglePID.Ki = atof(Value);
				}
				else if(strcmp(Name,"AngleKd") == 0)
				{
					AnglePID.Kd = atof(Value);
				}
				
				
				//速度调参
				else if(strcmp(Name,"SpeedKp") == 0)
				{
					SpeedPID.Kp = atof(Value);
				}
				else if(strcmp(Name,"SpeedKi") == 0)
				{
					SpeedPID.Ki = atof(Value);
				}
				else if(strcmp(Name,"SpeedKd") == 0)
				{
					SpeedPID.Kd = atof(Value);
				}
				
				
				
				//转向调参
				else if(strcmp(Name,"TurnKp") == 0)
				{
					TurnPID.Kp = atof(Value);
				}
				else if(strcmp(Name,"TurnKi") == 0)
				{
					TurnPID.Ki = atof(Value);
				}
				else if(strcmp(Name,"TurnKd") == 0)
				{
					TurnPID.Kd = atof(Value);
				}
				
				
				//输出偏移调参(角度PID)
				else if(strcmp(Name, "AngleOutOffset") == 0)
				{
					AnglePID.OutOffset = atof(Value);
				}
				
				
			}
			else if (strcmp(Tag, "joystick") == 0)//摇杆
			{
				int8_t LH = atoi(strtok(NULL, ","));
				int8_t LV = atoi(strtok(NULL, ","));
				int8_t RH = atoi(strtok(NULL, ","));
				int8_t RV = atoi(strtok(NULL, ","));
				
//				AnglePID.Target = LV / 10.0;
				SpeedPID.Target = LV / 25.0;
				TurnPID.Target = RH / 25.0;
			}
			
			BlueSerial_RxFlag = 0;
		}

		
//蓝牙传输到接受器的变量与打印形式（plot,绘图）		
//		BlueSerial_Printf("[plot,%f,%d]", Angle , GY);
//		BlueSerial_Printf("[plot,%f,%f]", AveSpeed , SpeedPID.Target);
//		BlueSerial_Printf("[plot,%f,%f]", DifSpeed , TurnPID.Target);
		BlueSerial_Printf("[plot,%f,%f,%f]", AnglePID.ErrorInt, SpeedPID.ErrorInt, TurnPID.ErrorInt);
			
		
	}//while结束
}//main结束



//中断函数
void TIM1_UP_IRQHandler(void)
{
	static uint16_t count0;//分频计数
	static uint16_t count1;
	if (TIM_GetITStatus(TIM1, TIM_IT_Update) == SET)
	{
		TIM_ClearITPendingBit(TIM1, TIM_IT_Update);
		
		Key_Tick();
//角度PID		
		count0++;
		if(count0 >= 10)
		{
			count0 = 0;
			
			
			MPU6050_GetData(&AX, &AY, &AZ, &GX, &GY, &GZ);
			
			GY -= 45;
			
			AngleAcc = -atan2(AX,AZ) / 3.14159 * 180;
			
			AngleAcc -=2.2;
			
			AngleGyro = Angle + GY / 32768.0 * 2000 * 0.01;
			
			//互补滤波
			float a = 0.01;
			Angle = AngleAcc * a + AngleGyro * (1 - a);
			
			if(Angle > 50 || Angle < -50)
			{
				RunFlag = 0;
			}
//安全执行			
			if(RunFlag)
			{
				AnglePID.Actual = Angle;    //实际值  来自传感器
				PID_Update(&AnglePID);      //目标值  主函数(串口传输)指定or外环输出值
				AvePWM = -AnglePID.Out;     //输出值  给到电机
				
				LeftPWM = AvePWM + DifPWM / 2;
				RightPWM = AvePWM - DifPWM / 2;
				
				if(LeftPWM>100){LeftPWM=100;}else if(LeftPWM<-100){LeftPWM=-100;}
				if(RightPWM>100){RightPWM=100;}else if(RightPWM<-100){RightPWM=-100;}
				
				Motor_SetPWM(1,LeftPWM);
				Motor_SetPWM(2,RightPWM);
			}
			else
			{
				Motor_SetPWM(1,0);
				Motor_SetPWM(2,0);
			}
		}
		
//速度PID		(速度周期与转向周期相近，故都写在速度PID里)
		count1++;
		if(count1 >= 50)
		{
			count1 = 0;
			
			LeftSpeed = Encoder_Get(1) / 44.0 / 0.05 / 19.0;
			RightSpeed = Encoder_Get(2) / 44.0 / 0.05 / 19.0;
			
			AveSpeed = (LeftSpeed + RightSpeed) / 2.0;
			DifSpeed = LeftSpeed - RightSpeed;
//安全执行
			if(RunFlag)
			{
				SpeedPID.Actual = AveSpeed;       //实际值  来自传感器
				PID_Update(&SpeedPID);            //目标值  主函数(串口传输)指定or外环输出值
				AnglePID.Target = SpeedPID.Out;   //输出值  给到电机
				
				
				TurnPID.Actual = DifSpeed;
				PID_Update(&TurnPID);
				DifPWM = TurnPID.Out;
				
				
				
			}
			
		}


		
		if (TIM_GetITStatus(TIM1, TIM_IT_Update) == SET)
		{
			TimerErrorFlag = 1;
			TIM_ClearITPendingBit(TIM1, TIM_IT_Update);
		}
		TimerCount = TIM_GetCounter(TIM1);
	}
}


//uint8_t KeyNum;
//int16_t PWML, PWMR;
//float SpeedL, SpeedR;

//int main(void)
//{
//	OLED_Init();
//	Key_Init();
//	Motor_Init();
//	Encoder_Init();
//	
//	Timer_Init();
//	
//	while (1)
//	{
//		KeyNum = Key_GetNum();
//		if (KeyNum == 1)
//		{
//			PWML += 10;
//		}
//		if (KeyNum == 2)
//		{
//			PWML -= 10;
//		}
//		if (KeyNum == 3)
//		{
//			PWMR += 10;
//		}
//		if (KeyNum == 4)
//		{
//			PWMR -= 10;
//		}
//		
//		Motor_SetPWM(1, PWML);
//		Motor_SetPWM(2, PWMR);
//		
//		OLED_Printf(0, 0, OLED_8X16, "PWML:%+04d", PWML);
//		OLED_Printf(0, 16, OLED_8X16, "PWMR:%+04d", PWMR);
//		OLED_Printf(0, 32, OLED_8X16, "SpdL:%+06.2f", SpeedL);
//		OLED_Printf(0, 48, OLED_8X16, "SpdR:%+06.2f", SpeedR);
//		OLED_Update();
//	}
//}

//void TIM1_UP_IRQHandler(void)
//{
//	static uint16_t Count;
//	if (TIM_GetITStatus(TIM1, TIM_IT_Update) == SET)
//	{
//		Key_Tick();
//		
//		Count ++;
//		if (Count >= 50)
//		{
//			Count = 0;
//			
//			SpeedL = Encoder_Get(1) / 44.0 / 0.05 / 19.0;
//			SpeedR = Encoder_Get(2) / 44.0 / 0.05 / 19.0;
//		}
//		
//		TIM_ClearITPendingBit(TIM1, TIM_IT_Update);
//	}
//}





