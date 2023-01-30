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

// 测试引脚 PE5   载入程序引脚 PD1  数据切换引脚PC2(切换成HP烧录位置 或者说STM32数据位置
void RelayGPIO_Init(void)
{
	GPIO_InitTypeDef 	GPIO_InitStruct;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOE|RCC_APB2Periph_GPIOC|RCC_APB2Periph_GPIOD|RCC_APB2Periph_GPIOF|RCC_APB2Periph_GPIOG,ENABLE);
	
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_OD;  	//电压5V
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_5;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOE,&GPIO_InitStruct);
	
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_OD;  	//EXTG 光耦引脚 。
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_11;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOE,&GPIO_InitStruct);
	
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_OD;		//数据切换 将感应板的数据线和烧写器接一起
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_2;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOC,&GPIO_InitStruct);
	
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_OD;  //电压3.3V
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_1;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOE,&GPIO_InitStruct);	
	
	
																																															
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IPU;    //检测脚BIN1
	GPIO_InitStruct.GPIO_Pin  = GPIO_Pin_1;
	GPIO_Init(GPIOG,&GPIO_InitStruct);
	
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IPU;    //检测脚EOP
	GPIO_InitStruct.GPIO_Pin  = GPIO_Pin_0;
	GPIO_Init(GPIOG,&GPIO_InitStruct);


	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_OD;
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_13 | GPIO_Pin_15;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOF,&GPIO_InitStruct);	
	
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_OD;  //载入程序引脚 PD1
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
// 程序载入按键
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
	
// 开始测试按键    
	if(GPIO_ReadInputDataBit(GPIOD, KEY_START_PIN) == 0)
	{
		ButtonP.KEYSTART_TIMER ++;
		if(ButtonP.KEYSTART_TIMER == 2)
		{
			ButtonP.KEYSTART_TIMER = 0;
			if(ButtonP.KEYCODE_FLAG == 0)
			{
				ButtonP.KEYSTART_FLAG = 1;
			//	TestStep.Test_Start = 1; //开始测试
			}
		}
	}
	else
	{
		ButtonP.KEYSTART_TIMER = 0;
	}
	
// 马达检测  未检测到 三极管通->检测脚为0   检测到  三极管不导通->检测脚为1
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

//  阀门1检测
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

// 阀门2检测	
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



//DIR  1 是向上 。
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
