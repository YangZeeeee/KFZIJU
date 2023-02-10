#ifndef _htusart_H
#define _htusart_H

#define Rx_Length  20

struct htRxdata
{
	unsigned char Time;
	unsigned char Data[15];
	unsigned char TempData[15];
	unsigned char TempLenght;
	unsigned char Length;
	unsigned char Flag;
	unsigned char Temp_Flag;
	unsigned int 	WaitTimer; //ÿ�����̵ȴ�ʱ��
	unsigned char WaitTimerFlag; //�ȴ���־λ
	unsigned char SendNum;
	unsigned char Check_Value;
	unsigned char htData_Deal;  //�ȴ�������ݴ�����֮��,�ſ�ִ���¸����ݶδ���
	unsigned char Rx_Error;
};
extern struct htRxdata htRxdata_P;



void ht_usart_Init(void);
void htData_Tx(unsigned char *data,unsigned char length);
void htData_Analyze(void);
unsigned char htData_Use(void);
#endif
