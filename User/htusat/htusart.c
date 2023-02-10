#include "htusart.h"
#include "stm32f10x_usart.h"
#include "stm32f10x_rcc.h"
#include "StepProcess.h"
#include "string.h"
#include "DCTFT.h"
struct htRxdata htRxdata_P;
	static unsigned char Length = 0;
void htUsart_NvicConfig(void)
{
	NVIC_InitTypeDef	NVIC_InitStruct;
  NVIC_InitStruct.NVIC_IRQChannel = USART3_IRQn;  //配置中断源
  NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 1; //中断优先级
  NVIC_InitStruct.NVIC_IRQChannelSubPriority = 2;
  NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStruct);
}
/******************************************************
感应头串口连接端口
串口硬件初始化参数配置
RX ->  PB10
TX ->  PB11
******************************************************/
void ht_usart_Init(void)
{
	USART_InitTypeDef USART_InitStruct;
	GPIO_InitTypeDef	GPIO_InitStruct;
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3,ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);
	
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_InitStruct.GPIO_Pin  = GPIO_Pin_11;
	GPIO_Init(GPIOB,&GPIO_InitStruct);
	
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_10;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(GPIOB,&GPIO_InitStruct);
	
	USART_InitStruct.USART_BaudRate = 9600;
	USART_InitStruct.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStruct.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_InitStruct.USART_Parity = USART_Parity_No;
	USART_InitStruct.USART_StopBits = USART_StopBits_1;
	USART_InitStruct.USART_WordLength = USART_WordLength_8b;
	USART_Init(USART3,&USART_InitStruct);
	htRxdata_P.htData_Deal = 0;
	htUsart_NvicConfig();
	USART_ITConfig(USART3,USART_IT_RXNE,ENABLE);
	USART_Cmd(USART3,ENABLE);
}
/******************************************************
DCData_Tx函数:
从MCU发送数据给感应头
*data  : 发送的数据包
length : 发送的数据长度
******************************************************/
void htData_Tx(unsigned char *data,unsigned char length)
{
	unsigned char len = 0;
	for(len = 0; len<length ;len ++)
	{
		USART_SendData(USART3, data[len]);
		while(USART_GetFlagStatus(USART3, USART_FLAG_TXE) == RESET);
	}
	while(USART_GetFlagStatus(USART3,USART_FLAG_TC)==RESET);
}


/******************************************************
Num_Add函数:
接受感应头的数据进行校验 总合。
******************************************************/
unsigned char  Num_Add(unsigned char *data,unsigned char len)
{
	unsigned char n,sum=0;
	for(n=0;n<len-1;n++)
	{
		sum+=data[n];
	}
	if(sum == data[len-1])
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

unsigned char htData_Use(void)
{

	if(htRxdata_P.Temp_Flag == 1 && htRxdata_P.htData_Deal == 0)
	{
		for(; Length < htRxdata_P.TempLenght;Length++)
		{
			if(htRxdata_P.TempData[Length] == 0xA5)
			{
				if(htRxdata_P.TempData[Length+1] == 0x10&&htRxdata_P.TempData[Length+2] == 0xB5 && TestStep.Test_Press == HW_Learn) //自学习结束
				{
					htRxdata_P.htData_Deal = 1;
					TestPara.HW_Learn_Flag = 1;
					GPIO_WriteBit(GPIOD,GPIO_Pin_0,Bit_RESET);
					return 1;
				}
				  
				 if(htRxdata_P.TempData[Length+1] == 0x11&&htRxdata_P.TempData[Length+2] == 0xB6) //指示灯电流
				{
					htRxdata_P.htData_Deal = 1;
					TestPara.IndicLigh_Current_Flag = 1;
					Length++;
					GPIO_WriteBit(GPIOD,GPIO_Pin_0,Bit_RESET);
					return 1;
				}
				else if(htRxdata_P.TempData[Length+1] == 0x12&&htRxdata_P.TempData[Length+2] == 0xB7) //驱动电流
				{
					htRxdata_P.htData_Deal = 1;
					TestPara.Power_Current_Flag = 1;
					Length++;
					GPIO_WriteBit(GPIOD,GPIO_Pin_0,Bit_RESET);
					return 1;
				}
				else if(htRxdata_P.TempData[Length+1] == 0x01&&htRxdata_P.TempData[Length+2] == 0x05)
				{
					htRxdata_P.htData_Deal = 1;
					TestPara.Ganying_Data_Flag = 1;
					TestPara.Power_ADC_Test = htRxdata_P.Data[Length+3];
					TestPara.Shoot_Power_Test =  htRxdata_P.Data[Length+4];
					TestPara.Learn_Test =  htRxdata_P.Data[Length+5];
					TestPara.OPA0_Test =htRxdata_P.Data[Length+6];
					TestPara.OPA1_Test = htRxdata_P.Data[Length+7];
					GPIO_WriteBit(GPIOD,GPIO_Pin_0,Bit_RESET);
					Length++;
					return 1;
				}
			}
		}
		htRxdata_P.Temp_Flag = 0;
		Length = 0;
	}
	return 0;
}
/******************************************************
htData_Analyze函数:
解析感应头数据参数
返回参数值:1.参数正确 0.参数校验失败。
******************************************************/
void htData_Analyze(void)
{
	if((htRxdata_P.Flag==1) && (htRxdata_P.Length!=0) && (htRxdata_P.Temp_Flag ==0))
	{	
		memset(htRxdata_P.TempData,0,sizeof(htRxdata_P.TempData));
		htRxdata_P.TempLenght = htRxdata_P.Length;
		htRxdata_P.Length = 0;
		htRxdata_P.Flag = 0;
		for(unsigned char i = 0 ; i<htRxdata_P.TempLenght; i++)
		{
			htRxdata_P.TempData[i] = htRxdata_P.Data[i];
		}
		htRxdata_P.Temp_Flag = 1;
		
	}
}

/******************************************************
USART3_IRQHandler函数:
接收感应头发来的数据 
10ms没有数据接受则判断接收完成  
******************************************************/
void USART3_IRQHandler(void)
{
	if(USART_GetITStatus(USART3,USART_IT_RXNE) != RESET)
	{
		USART_ClearITPendingBit(USART3,USART_IT_RXNE);
		htRxdata_P.Data[htRxdata_P.Length++%Rx_Length] = USART_ReceiveData(USART3);		
		htRxdata_P.Time = 0;
		htRxdata_P.Flag = 0;
	}
}
