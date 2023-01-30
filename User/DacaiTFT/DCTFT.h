#ifndef _DCTFT_H
#define _DCTFT_H
#include "stm32f10x.h"
//usart 1 

#define  DCTFT_USARTx                   USART1
#define  DCTFT_USART_CLK                RCC_APB2Periph_USART1
#define  DCTFT_USART_APBxClkCmd         RCC_APB2PeriphClockCmd
#define  DCTFT_USART_BAUDRATE           9600

#define  DCTFT_USART_GPIO_CLK           (RCC_APB2Periph_GPIOA)
#define  DCTFT_USART_GPIO_APBxClkCmd    RCC_APB2PeriphClockCmd
    
#define  DCTFT_USART_TX_GPIO_PORT       GPIOA   
#define  DCTFT_USART_TX_GPIO_PIN        GPIO_Pin_9
#define  DCTFT_USART_RX_GPIO_PORT       GPIOA
#define  DCTFT_USART_RX_GPIO_PIN        GPIO_Pin_10

#define  DCTFT_USART_IRQ                USART1_IRQn
#define  DCTFT_USART_IRQHandler         USART1_IRQHandler


#define DCRx_Len 1024
struct DC
{
	unsigned char Data[DCRx_Len];  //接收数据buff
	unsigned int Head;				//每次接受的头地址 判断是否有新数据
	unsigned int Tail;				//接收数据的尾部 判断头尾是否相等
	unsigned int Length;				//每次接收长度
	unsigned int  Time;				//用于判断串口是否停止时间
	unsigned char Flag;				//代表接收完毕
};
extern struct DC DCRXdata;


#define DCAnalay_Len 1024

struct DCData
{
	unsigned char DataTemp[DCRx_Len];
	unsigned int LengthTemp;
	unsigned char Data[DCAnalay_Len];
	unsigned int Head;
	unsigned int Complete;  //接收的数据全部处理完成
	unsigned int packe;     //一包的数据处理提取完成  
	unsigned int PackLength;  //包长。
};
extern struct DCData DC_Analay;

void TFTUsart_init(void);
void	Rxdata_Extract(void);
void Rxdata_Cache(void);
void DCData_Tx(unsigned char *data,unsigned char length);


#endif
