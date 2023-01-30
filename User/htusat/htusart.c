#include "htusart.h"
#include "stm32f10x_usart.h"
#include "stm32f10x_rcc.h"
#include "StepProcess.h"
#include "string.h"
#include "DCTFT.h"
struct htRxdata htRxdata_P;

void htUsart_NvicConfig(void)
{
	NVIC_InitTypeDef	NVIC_InitStruct;
  NVIC_InitStruct.NVIC_IRQChannel = USART3_IRQn;  //�����ж�Դ
  NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 1; //�ж����ȼ�
  NVIC_InitStruct.NVIC_IRQChannelSubPriority = 2;
  NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStruct);
}
/******************************************************
��Ӧͷ�������Ӷ˿�
����Ӳ����ʼ����������
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
	
	htUsart_NvicConfig();
	USART_ITConfig(USART3,USART_IT_RXNE,ENABLE);
	USART_Cmd(USART3,ENABLE);
}
/******************************************************
DCData_Tx����:
��MCU�������ݸ���Ӧͷ
*data  : ���͵����ݰ�
length : ���͵����ݳ���
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
Num_Add����:
���ܸ�Ӧͷ�����ݽ���У�� �ܺϡ�
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

/******************************************************
htData_Analyze����:
������Ӧͷ���ݲ���
���ز���ֵ:1.������ȷ 0.����У��ʧ�ܡ�
******************************************************/
unsigned char htData_Analyze(void)
{
	if((htRxdata_P.Flag==1) && (htRxdata_P.Length!=0))
	{
			if(Num_Add(htRxdata_P.Data,htRxdata_P.Length)) //������ɽ�������
			{
			//	memset(&htRxdata_P.Data,0,sizeof(htRxdata_P.Data));
				
				htRxdata_P.Length = 0;
				htRxdata_P.Flag = 0;
				return 1;
			}
			else
			{
				memset(&htRxdata_P.Data,0,sizeof(htRxdata_P.Data));
				htRxdata_P.Length = 0;
				htRxdata_P.Flag = 0;
				return 0;
			}
	}
	return 0;
}

/******************************************************
USART3_IRQHandler����:
���ո�Ӧͷ���������� 
10msû�����ݽ������жϽ������  
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
