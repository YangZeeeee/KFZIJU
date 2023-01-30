#ifndef _htusart_H
#define _htusart_H

#define Rx_Length  20

struct htRxdata
{
	unsigned char Time;
	unsigned char Data[20];
	unsigned char Length;
	unsigned char Flag;
	unsigned int WaitTimer; //每个流程等待时间
	unsigned char WaitTimerFlag; //等待标志位
	unsigned char SendNum;
	unsigned char Check_Value;
};
extern struct htRxdata htRxdata_P;



void ht_usart_Init(void);
void htData_Tx(unsigned char *data,unsigned char length);
unsigned char htData_Analyze(void);
#endif
