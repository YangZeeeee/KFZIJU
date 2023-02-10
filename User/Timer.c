#include "Timer.h"
#include "stm32f10x_tim.h"
#include "stm32f10x_rcc.h"
#include "DCTFT.h"
#include "StepProcess.h"
#include "Button.h"
#include "Ampere.h"
#include "htusart.h"
#include "LoadCode.h"
unsigned int led_test_time;

/******************************************************
TIM_NVIC_Config:
定时器中断源初始化参数配置
******************************************************/
void TIM_NVIC_Config(void)
{
	NVIC_InitTypeDef NVIC_InitStructure; 
   NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);// 设置中断组		
   NVIC_InitStructure.NVIC_IRQChannel =TIM2_IRQn;	//中断来源
   NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;// 主优先级 
   NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;// 抢占优先级	
   NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
   NVIC_Init(&NVIC_InitStructure);
}

/******************************************************
Timer_Init函数:
定时器初始化参数配置
Period ： 定时时间  基准时间1us            period= 1000   则定时时长为1ms
******************************************************/
void Timer_Init(unsigned int Period)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStruct;
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
	TIM_TimeBaseStruct.TIM_Period = Period - 1;
	TIM_TimeBaseStruct.TIM_Prescaler = 71;
	TIM_TimeBaseStruct.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseStruct.TIM_CounterMode  = TIM_CounterMode_Up;
	TIM_TimeBaseStruct.TIM_RepetitionCounter = 0;
	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStruct);
	TIM_NVIC_Config();	//中断优先级配置
	TIM_ClearFlag(TIM2,TIM_FLAG_Update);
	TIM_ITConfig(TIM2,TIM_IT_Update,ENABLE);
	TIM_Cmd(TIM2, ENABLE);		
}


/******************************************************
TIM2_IRQHandler 中断函数
1ms进入一次中断
******************************************************/
void TIM2_IRQHandler(void) 
{	
	if(TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET)
	{
		TIM_ClearFlag(TIM2,TIM_FLAG_Update);
		Button_Scan();
		//判断50ms内没有数据发送 则代表一次数据传输完成。
		if(DCRXdata.Flag == 1)
		{
			DCRXdata.Time++;
			if (DCRXdata.Time == 50)
			{
				DCRXdata.Flag = 0;
			}			
		}

		if(htRxdata_P.Flag == 0)				//感应头通信一包数据完成
		{
			htRxdata_P.Time ++;
			if(htRxdata_P.Time == 20)
			{
				htRxdata_P.Flag = 1;
			}			
		}	
		if(htRxdata_P.WaitTimerFlag == 1) //感应头通信等待时长完成
		{
			htRxdata_P.WaitTimer++;
			if(TestStep.Test_Press == HW_Learn)
			{
				if(htRxdata_P.WaitTimer == 7000) //等待5S 查看是否回数据
				{
					htRxdata_P.WaitTimerFlag = 2;
					htRxdata_P.WaitTimer = 0;
				}
			}			
			else if(TestStep.Test_Press == Starting_Up)  //上电等2S 发送自动学习指令
			{
				if(htRxdata_P.WaitTimer == 2000)
				{
					htRxdata_P.WaitTimerFlag = 2;
					htRxdata_P.WaitTimer = 0;
				}	
			}
			else if(TestStep.Test_Press == ReadData_Send)
			{
				if(htRxdata_P.WaitTimer == 15) //50ms
				{
						htRxdata_P.WaitTimerFlag = 2;
						htRxdata_P.WaitTimer = 0;
				}		
			}
			else if(TestStep.Test_Press == Ganying_Data)
			{
				if(htRxdata_P.WaitTimer == 4000) //5s
				{
						htRxdata_P.WaitTimerFlag = 2;
						htRxdata_P.WaitTimer = 0;
				}
			}
			else if(TestStep.Test_Press==Ganyin_Distance)
			{
				
				if(htRxdata_P.WaitTimer == 1000) //1s
				{
						htRxdata_P.WaitTimerFlag = 2;
						htRxdata_P.WaitTimer = 0;
				}
			}
			else if(TestStep.Test_Press==Standby_Current)
			{
				if(htRxdata_P.WaitTimer == 1000) //1s
				{
						htRxdata_P.WaitTimerFlag = 2;
						htRxdata_P.WaitTimer = 0;
				}
			}
			else 
			{
					if(htRxdata_P.WaitTimer == 250) 
					{
						htRxdata_P.WaitTimerFlag = 2;
						htRxdata_P.WaitTimer = 0;
					}				
			}
		}
		led_test_time++;
		if(AmpereData.FLAG == 1)  //电流表通信一包数据完成
		{
			AmpereData.TIME++;
			if(AmpereData.TIME == 100)
			{
				AmpereData.FLAG=0;
			}			
		}	
		if(ComP.Com_time > 0)  //读取电流等待时间
		{
			ComP.Com_time--;
		}
		
		if((TestStep.Test_Press == Test_Init || TestStep.Test_Press == Restart_Motor) && (TestStep.Motor_ReTime <= 5000))
		{
			TestStep.Motor_ReTime++;
			if(TestStep.Motor_ReTime == 5000)
			{
				TestStep.Motor_ReFlag = 1;
			}
		}
			
		
		// 显示时长为5S后  隐藏显示。
		if(LoadCodeP.HintDisPlayFLag == 1)
		{
			if(LoadCodeP.HintDisPlayTime < 5000)
			{
				LoadCodeP.HintDisPlayTime++;
			}
			else if(LoadCodeP.HintDisPlayTime == 5000)
			{
				LoadCodeP.HintDisPlayFLag = 2;
				LoadCodeP.HintDisPlayTime = 0;
			}
		}
		
		if(ButtonP.LoadCode_Flag==1)
		{
			ButtonP.LoadCode_Timer++;
			if(ButtonP.LoadCode_Timer == 100)
			{
				Data_EXTG_Relay_H;		
				LoadCodeP.LoadCodeStep =1;
			}
			else if(ButtonP.LoadCode_Timer == 5000 && LoadCodeP.LoadCodeStep==1)
			{
				LoadCodeP.LoadCodeResult = 2;  //烧录失败
				ButtonP.LoadCode_Timer  = 0;
				ButtonP.LoadCode_Flag = 0;
				LoadCodeP.LoadCodeStep = 2;
			}
		}
	}
}
