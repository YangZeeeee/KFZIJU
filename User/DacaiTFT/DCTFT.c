#include "DCTFT.h"
#include "stm32f10x_usart.h"
#include "stm32f10x_rcc.h"
#include "string.h"
struct DC DCRXdata;
struct DCData DC_Analay;

/******************************************************
TFTUsart_NvicConfig函数:
串口中断优先配置函数
******************************************************/
void TFTUsart_NvicConfig(void)
{
	NVIC_InitTypeDef	NVIC_InitStruct;
  NVIC_InitStruct.NVIC_IRQChannel = DCTFT_USART_IRQ;  //配置中断源
  NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 1; //中断优先级
  NVIC_InitStruct.NVIC_IRQChannelSubPriority = 1;
  NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStruct);
}

/******************************************************
TFTUsart_init函数:
串口硬件初始化参数配置
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
DCData_Tx函数:
从MCU发送数据给大彩串口屏
*data  : 发送的数据包
length : 发送的数据长度
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
Analysis_Rxdata函数:
将串口接收到的数据缓存到临时数据区。
判断是否缓存的三个条件
1.数据接收完成。
2.有新的数据产生。
3.上一次的缓存数据解析完成
******************************************************/
void Rxdata_Cache(void)
{
	unsigned int len=0;
	//接收标志位flag = 0代表没有数据在发送,头部与尾部不相等代表有新数据
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
Rxdata_Extract函数:
数据提取 提取一帧的数据
提取一帧数据的流程
1.判断一整次的数据是否处理完成(判断依据  ：  处理包的数据长度> 此次接收的数据长度)
2.查找一帧数据包的起始位(0xee)
3.查找一帧数据包的结束位(0Xff) 再加上数据长度判断(还未加)
******************************************************/
void	Rxdata_Extract(void)
{
	unsigned char num = 0;
	if(DC_Analay.Head >= DC_Analay.LengthTemp)  //一整次数据处理完成
	{
		DC_Analay.Complete = 0;
	}
	if(DC_Analay.Complete ==1)
	{
		for (; DC_Analay.Head < DC_Analay.LengthTemp; DC_Analay.Head++)
		{
			if (DC_Analay.DataTemp[DC_Analay.Head] == 0XEE) // 一帧数据的开始0XEE
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
						DC_Analay.packe=1;  // 一帧数据接收完成
						break;// 跳出一帧数据包循环
					}
					DC_Analay.Head++;
				}
				if(DC_Analay.packe == 1)
				{
					break; //找到一帧数据包后跳出循环。
				}
			}
		}		
	}
}
/******************************************************
DCTFT_USART_IRQHandler函数:
接收大彩串口屏发过来的数据
一包数据接收完成判断为 50ms无数据接收
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
