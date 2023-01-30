#ifndef _Ampere_H
#define _Ampere_H


#define AMpere_TX 			GPIO_Pin_2
#define AMpere_RX  			GPIO_Pin_3
#define AMpere_Usartx		USART2	
#define Ampere_IRQHandler  USART2_IRQHandler
#define AMRXLEN	128
struct AmpereP
{
	unsigned char DATA[AMRXLEN];
	unsigned char DATATEMP[AMRXLEN];
	unsigned char HEAD;
	unsigned int  TIME;
	unsigned char LENGTH;
	unsigned char FLAG;
	unsigned char TAIL;
	unsigned char LENGTHTEMP;
	unsigned char COMPLETE;
	unsigned char FAILFLAG;
};

extern struct AmpereP  AmpereData; 
void Ampere_usart485_init(void);
void AMRxdata_Cache(void);
void AMPara_Send(void);
unsigned long	AMRXDATA_Extract(void);

#endif

