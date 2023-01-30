#include "Button.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include "StepProcess.h"
#include "Motor.h"
#include "Delay.h"
#include "DCTFT.h"
struct button ButtonP;

void  ButtonP_Init(void)
{
	ButtonP.KEYCODE_FLAG = 0;
	ButtonP.KEYCODE_TIMER = 0;
	ButtonP.KEYSTART_FLAG = 0;
	ButtonP.KEYSTART_TIMER = 0;
	ButtonP.GATE1_FLAG = 0;
	ButtonP.GATE1_TIMER = 0;
	ButtonP.GATE2_FLAG = 0;
	ButtonP.GATE2_TIMER = 0;
	ButtonP.LoadCode_Flag = 0;
	ButtonP.LoadCode_Timer = 0;
}

void Button_HardInit(void)
{
	GPIO_InitTypeDef 	GPIO_InitStruct;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD|RCC_APB2Periph_GPIOE,ENABLE);
	
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStruct.GPIO_Pin  = KEY_START_PIN | KEY_CODELOAD_PIN;
	GPIO_Init(GPIOD,&GPIO_InitStruct);

	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStruct.GPIO_Pin = GATE1_DET|GATE2_DET|MOTOR_DET;
	GPIO_Init(GPIOE,&GPIO_InitStruct);
	ButtonP_Init();
}

// �������� PE5   ����������� PD1  �����л�����PC2(�л���HP��¼λ�� ����˵STM32����λ��
void RelayGPIO_Init(void)
{
	GPIO_InitTypeDef 	GPIO_InitStruct;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOE|RCC_APB2Periph_GPIOC|RCC_APB2Periph_GPIOD|RCC_APB2Periph_GPIOF|RCC_APB2Periph_GPIOG,ENABLE);
	
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_OD;  	//��ѹ5V
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_5;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOE,&GPIO_InitStruct);
	
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_OD;  	//EXTG �������� ��
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_11;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOE,&GPIO_InitStruct);
	
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_OD;		//�����л� ����Ӧ��������ߺ���д����һ��
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_2;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOC,&GPIO_InitStruct);
	
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_OD;  //��ѹ3.3V
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_1;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOE,&GPIO_InitStruct);	
	
	
																																															
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IPU;    //����BIN1
	GPIO_InitStruct.GPIO_Pin  = GPIO_Pin_1;
	GPIO_Init(GPIOG,&GPIO_InitStruct);
	
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IPU;    //����EOP
	GPIO_InitStruct.GPIO_Pin  = GPIO_Pin_0;
	GPIO_Init(GPIOG,&GPIO_InitStruct);


	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_OD;
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_13 | GPIO_Pin_15;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOF,&GPIO_InitStruct);	
	
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_OD;  //����������� PD1
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_11;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOC,&GPIO_InitStruct);	
	
	Test_Relay_H;
	Load_Relay_H;;
	DataSwitch_Relay_H;
	DCMotorF_Relay_H;
	DCMotorZ_Relay_H;
	CAP_SWITCH_L;
	Data_EXTG_Relay_H;
}

void  Button_Scan(void)
{
// �������밴��
	if(GPIO_ReadInputDataBit(GPIOD, KEY_CODELOAD_PIN) == 0)
	{
		ButtonP.KEYCODE_TIMER ++;
		if(ButtonP.KEYCODE_TIMER == 2)
		{
			ButtonP.KEYCODE_TIMER = 0;
			if(ButtonP.KEYSTART_FLAG == 0)
			{
				ButtonP.KEYCODE_FLAG = 1;				
			}
		}
	}
	else
	{
		ButtonP.KEYCODE_TIMER =0;
	}
	
// ��ʼ���԰���    
	if(GPIO_ReadInputDataBit(GPIOD, KEY_START_PIN) == 0)
	{
		ButtonP.KEYSTART_TIMER ++;
		if(ButtonP.KEYSTART_TIMER == 2)
		{
			ButtonP.KEYSTART_TIMER = 0;
			if(ButtonP.KEYCODE_FLAG == 0)
			{
				ButtonP.KEYSTART_FLAG = 1;
			//	TestStep.Test_Start = 1; //��ʼ����
			}
		}
	}
	else
	{
		ButtonP.KEYSTART_TIMER = 0;
	}
	
// �����  δ��⵽ ������ͨ->����Ϊ0   ��⵽  �����ܲ���ͨ->����Ϊ1
	if(GPIO_ReadInputDataBit(GPIOE, MOTOR_DET) == 1)
	{
		ButtonP.MOTOR_TIMER ++;
		if(ButtonP.MOTOR_TIMER == 2)
		{
			ButtonP.MOTOR_TIMER = 0;
			ButtonP.MOTOR_FLAG = 1;
		}
	}
	else
	{
		ButtonP.MOTOR_FLAG = 0;
		ButtonP.MOTOR_TIMER = 0;
	}

//  ����1���
	if(GPIO_ReadInputDataBit(GPIOE, GATE1_DET) == 1)
	{
		ButtonP.GATE1_TIMER ++;
		if(ButtonP.GATE1_TIMER == 5)
		{
			ButtonP.GATE1_TIMER = 0;
			ButtonP.GATE1_FLAG = 1;
		}
	}
	else
	{
		ButtonP.GATE1_TIMER = 0;
	}

// ����2���	
	if(GPIO_ReadInputDataBit(GPIOE, GATE2_DET) == 1)
	{
		ButtonP.GATE2_TIMER ++;
		if(ButtonP.GATE2_TIMER == 5)
		{
			ButtonP.GATE2_TIMER = 0;
			ButtonP.GATE2_FLAG = 1;
		}
	}
	else
	{
		ButtonP.GATE2_TIMER = 0;
	}
	
}



//DIR  1 ������ ��
void Motor_Test(void)
{
	unsigned int StepP_Num=0,StepN_Num=0;
	while(ButtonP.MOTOR_FLAG == 0)
	{
		Motor_DirP;
		for(StepN_Num=0;StepN_Num<79;StepN_Num++)
		{
			Motor_PulP;
			delay_us(50);
			Motor_PulN;
			delay_us(50);
		}
	}
	ButtonP.MOTOR_FLAG = 0;
	
	Motor_DirN;
	for(StepP_Num=0;StepP_Num<7900;StepP_Num++)
	{
		Motor_PulP;
		delay_us(50);
		Motor_PulN;
		delay_us(50);
	}				
}
