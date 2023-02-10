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
��ʱ���ж�Դ��ʼ����������
******************************************************/
void TIM_NVIC_Config(void)
{
	NVIC_InitTypeDef NVIC_InitStructure; 
   NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);// �����ж���		
   NVIC_InitStructure.NVIC_IRQChannel =TIM2_IRQn;	//�ж���Դ
   NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;// �����ȼ� 
   NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;// ��ռ���ȼ�	
   NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
   NVIC_Init(&NVIC_InitStructure);
}

/******************************************************
Timer_Init����:
��ʱ����ʼ����������
Period �� ��ʱʱ��  ��׼ʱ��1us            period= 1000   ��ʱʱ��Ϊ1ms
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
	TIM_NVIC_Config();	//�ж����ȼ�����
	TIM_ClearFlag(TIM2,TIM_FLAG_Update);
	TIM_ITConfig(TIM2,TIM_IT_Update,ENABLE);
	TIM_Cmd(TIM2, ENABLE);		
}


/******************************************************
TIM2_IRQHandler �жϺ���
1ms����һ���ж�
******************************************************/
void TIM2_IRQHandler(void) 
{	
	if(TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET)
	{
		TIM_ClearFlag(TIM2,TIM_FLAG_Update);
		Button_Scan();
		//�ж�50ms��û�����ݷ��� �����һ�����ݴ�����ɡ�
		if(DCRXdata.Flag == 1)
		{
			DCRXdata.Time++;
			if (DCRXdata.Time == 50)
			{
				DCRXdata.Flag = 0;
			}			
		}

		if(htRxdata_P.Flag == 0)				//��Ӧͷͨ��һ���������
		{
			htRxdata_P.Time ++;
			if(htRxdata_P.Time == 20)
			{
				htRxdata_P.Flag = 1;
			}			
		}	
		if(htRxdata_P.WaitTimerFlag == 1) //��Ӧͷͨ�ŵȴ�ʱ�����
		{
			htRxdata_P.WaitTimer++;
			if(TestStep.Test_Press == HW_Learn)
			{
				if(htRxdata_P.WaitTimer == 7000) //�ȴ�5S �鿴�Ƿ������
				{
					htRxdata_P.WaitTimerFlag = 2;
					htRxdata_P.WaitTimer = 0;
				}
			}			
			else if(TestStep.Test_Press == Starting_Up)  //�ϵ��2S �����Զ�ѧϰָ��
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
		if(AmpereData.FLAG == 1)  //������ͨ��һ���������
		{
			AmpereData.TIME++;
			if(AmpereData.TIME == 100)
			{
				AmpereData.FLAG=0;
			}			
		}	
		if(ComP.Com_time > 0)  //��ȡ�����ȴ�ʱ��
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
			
		
		// ��ʾʱ��Ϊ5S��  ������ʾ��
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
				LoadCodeP.LoadCodeResult = 2;  //��¼ʧ��
				ButtonP.LoadCode_Timer  = 0;
				ButtonP.LoadCode_Flag = 0;
				LoadCodeP.LoadCodeStep = 2;
			}
		}
	}
}
