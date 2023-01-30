#include "Motor.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
/******************************************************
Motor_GPIO_Init函数:
电机引脚参数配置初始化     
******************************************************/
void Motor_GPIO_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOF,ENABLE);
	
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_1|GPIO_Pin_2|GPIO_Pin_0;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(Motor_GPIO,&GPIO_InitStruct);
//	GPIO_SetBits(Motor_GPIO,Motor_Pul_Pin);
}
