#include "DCTFT.h"
#include "stm32f10x_usart.h"
#include "stm32f10x_rcc.h"
#include "string.h"
struct DC DCRXdata;
struct DCData DC_Analay;

/******************************************************
TFTUsart_NvicConfig����:
�����ж��������ú���
******************************************************/
void TFTUsart_NvicConfig(void)
{
	NVIC_InitTypeDef	NVIC_InitStruct;
  NVIC_InitStruct.NVIC_IRQChannel = DCTFT_USART_IRQ;  //�����ж�Դ
  NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 1; //�ж����ȼ�
  NVIC_InitStruct.NVIC_IRQChannelSubPriority = 1;
  NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStruct);
}

/******************************************************
TFTUsart_init����:
����Ӳ����ʼ����������
RX ->  PA9
TX ->  PA10
******************************************************/
void TFTUsart_init(void)
{
	GPIO_InitTypeDef 	GPIO_InitStruct;
	USART_InitTypeDef	USART_InitStruct;

	DCTFT_USART_GPIO_APBxClkCmd(DCTFT_USART_GPIO_CLK,ENABLE);
	DCTFT_USART_APBxClkCmd(DCTFT_USART_CLK,ENABLE);

	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStruct.GPIO_Pin  = DCTFT_USART_TX_GPIO_PIN;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(DCTFT_USART_TX_GPIO_PORT,&GPIO_InitStruct);

	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_InitStruct.GPIO_Pin  = DCTFT_USART_RX_GPIO_PIN;
	GPIO_Init(DCTFT_USART_RX_GPIO_PORT,&GPIO_InitStruct);	

	USART_InitStruct.USART_BaudRate = DCTFT_USART_BAUDRATE;
	USART_InitStruct.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_InitStruct.USART_StopBits = USART_StopBits_1;
	USART_InitStruct.USART_WordLength = USART_WordLength_8b;
	USART_InitStruct.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStruct.USART_Parity = USART_Parity_No;
	USART_Init(DCTFT_USARTx,&USART_InitStruct);

	TFTUsart_NvicConfig();  
	USART_ITConfig(DCTFT_USARTx, USART_IT_RXNE, ENABLE);	
	USART_Cmd(DCTFT_USARTx, ENABLE);	
}

/******************************************************
DCData_Tx����:
��MCU�������ݸ���ʴ�����
*data  : ���͵����ݰ�
length : ���͵����ݳ���
******************************************************/
void DCData_Tx(unsigned char *data,unsigned char length)
{
	unsigned char len = 0;
	for(len = 0; len<length ;len ++)
	{
		USART_SendData(DCTFT_USARTx, data[len]);
		while(USART_GetFlagStatus(DCTFT_USARTx, USART_FLAG_TXE) == RESET);
	}
	while(USART_GetFlagStatus(DCTFT_USARTx,USART_FLAG_TC)==RESET);
}

/******************************************************
Analysis_Rxdata����:
�����ڽ��յ������ݻ��浽��ʱ��������
�ж��Ƿ񻺴����������
1.���ݽ�����ɡ�
2.���µ����ݲ�����
3.��һ�εĻ������ݽ������
******************************************************/
void Rxdata_Cache(void)
{
	unsigned int len=0;
	//���ձ�־λflag = 0����û�������ڷ���,ͷ����β������ȴ�����������
	if(DCRXdata.Flag == 0 && (DCRXdata.Head != DCRXdata.Tail) && (DC_Analay.Complete == 0)) 
	{
		memset(&DC_Analay.DataTemp,0,sizeof(DC_Analay.DataTemp));
		DC_Analay.LengthTemp = DCRXdata.Length;
		DCRXdata.Length =0;
		for(len = 0 ; len < DC_Analay.LengthTemp;len++)
		{
			DC_Analay.DataTemp[len] = DCRXdata.Data[DCRXdata.Tail++%1024];
		}
		DC_Analay.Complete = 1;
		DC_Analay.Head = 0;
	}
}
/******************************************************
Rxdata_Extract����:
������ȡ ��ȡһ֡������
��ȡһ֡���ݵ�����
1.�ж�һ���ε������Ƿ������(�ж�����  ��  ����������ݳ���> �˴ν��յ����ݳ���)
2.����һ֡���ݰ�����ʼλ(0xee)
3.����һ֡���ݰ��Ľ���λ(0Xff) �ټ������ݳ����ж�(��δ��)
******************************************************/
void	Rxdata_Extract(void)
{
	unsigned char num = 0;
	if(DC_Analay.Head >= DC_Analay.LengthTemp)  //һ�������ݴ������
	{
		DC_Analay.Complete = 0;
	}
	if(DC_Analay.Complete ==1)
	{
		for (; DC_Analay.Head < DC_Analay.LengthTemp; DC_Analay.Head++)
		{
			if (DC_Analay.DataTemp[DC_Analay.Head] == 0XEE) // һ֡���ݵĿ�ʼ0XEE
			{
				DC_Analay.PackLength = 0;
				for(num = 0;num<DC_Analay.LengthTemp;num++)
				{
					DC_Analay.Data[num] = DC_Analay.DataTemp[DC_Analay.Head];				
					if((DC_Analay.DataTemp[DC_Analay.Head] == 0XFF&&(DC_Analay.DataTemp[DC_Analay.Head+1] == 0XFC)) )
					{
						DC_Analay.Data[num+1] = DC_Analay.DataTemp[DC_Analay.Head+1];
						DC_Analay.Data[num+2] = DC_Analay.DataTemp[DC_Analay.Head+2];
						DC_Analay.Data[num+3] = DC_Analay.DataTemp[DC_Analay.Head+3];
						//DCData_Tx(DC_Analay.Data,num+4);
						DC_Analay.Head = DC_Analay.Head+3;
						DC_Analay.PackLength = num+4;
						DC_Analay.packe=1;  // һ֡���ݽ������
						break;// ����һ֡���ݰ�ѭ��
					}
					DC_Analay.Head++;
				}
				if(DC_Analay.packe == 1)
				{
					break; //�ҵ�һ֡���ݰ�������ѭ����
				}
			}
		}		
	}
}
/******************************************************
DCTFT_USART_IRQHandler����:
���մ�ʴ�����������������
һ�����ݽ�������ж�Ϊ 50ms�����ݽ���
******************************************************/
void DCTFT_USART_IRQHandler(void)
{
	if(USART_GetITStatus(DCTFT_USARTx,USART_IT_RXNE) != RESET)
	{
		USART_ClearITPendingBit(DCTFT_USARTx,USART_IT_RXNE);
		DCRXdata.Data[DCRXdata.Head++%1024] = USART_ReceiveData(DCTFT_USARTx);		
		DCRXdata.Length++;
		DCRXdata.Flag = 1;
		DCRXdata.Time = 0;
	}
}
