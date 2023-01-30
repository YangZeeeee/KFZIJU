#ifndef _htusart_H
#define _htusart_H

#define Rx_Length  20

struct htRxdata
{
	unsigned char Time;
	unsigned char Data[20];
	unsigned char Length;
	unsigned char Flag;
	unsigned int WaitTimer; //ÿ�����̵ȴ�ʱ��
	unsigned char WaitTimerFlag; //�ȴ���־λ
	unsigned char SendNum;
	unsigned char Check_Value;
};
extern struct htRxdata htRxdata_P;



void ht_usart_Init(void);
void htData_Tx(unsigned char *data,unsigned char length);
unsigned char htData_Analyze(void);
#endif
