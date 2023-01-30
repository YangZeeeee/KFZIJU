#include "stm32f10x.h"
#include "DCTFT.h"
#include "string.h"
#include "Timer.h"
#include "StepProcess.h"
#include "Button.h"
#include "Ampere.h"
#include "htusart.h"
#include "DataFlash.h"
#include "Delay.h"
#include "Motor.h"
#include "LoadCode.h"
#include "stm32f10x_iwdg.h"
#include "idwg.h"


unsigned char Ampere_ControL1[8] = {0x01,0x03,0x00,0x01,0x00,0x02,0x95,0xcb}; //读取电流寄存器
unsigned char writeString[10]={0xA5,0x01,0x01,0x01,0x08}; 
unsigned char readString[10];
	
unsigned char HWDIS1[] = {0xA5,0x02,0x01,0x01,0xA9};
unsigned char HWREAD1[] = {0xA5,0x02,0x01,0x02,0xAA};
unsigned char DGREAD1[] = {0xA5,0x01,0x01,0x02,0xA9};

unsigned char loadTestFlag = 0;

void test(void)
{
	GPIO_InitTypeDef 	GPIO_InitStruct;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOE|RCC_APB2Periph_GPIOD,ENABLE);
		
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_OD;
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_0;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOD,&GPIO_InitStruct);	
	GPIO_WriteBit(GPIOD,GPIO_Pin_0,1);
}
void  structInit_Test(void)
{
		Para_Data.GanDistance_Max = 1234;
		Para_Data.GanDistance_Min = 123;

		Para_Data.HW_Learn_Max = 204;
		Para_Data.HW_Learn_Min = 24;
	
		Para_Data.Light_Current_Max = 4567;
		Para_Data.Light_Current_Min = 456;
		
		Para_Data.Power_ADC_Max = 5678;
		Para_Data.Power_ADC_Min = 678;
	
		Para_Data.Power_Current_Max = 6789;
		Para_Data.Power_Current_Min = 679;
	
		Para_Data.Shoot_Power_Max = 7890;
		Para_Data.Shoot_Power_Min= 890;
		
		Para_Data.Standby_Current_Max = 8901;
		Para_Data.Standby_Current_Min = 9023;
		
		Para_Data.Value_Current_Max = 6514;
		Para_Data.Value_Current_Min = 514;
	
}


int main(void)
{
	unsigned  char level = 0;
	memset(&DCRXdata,0,sizeof(DCRXdata));
	delay_init();
	RelayGPIO_Init();
	test();
	LoadCodeP_Init();
	TFTUsart_init();
	Motor_GPIO_Init();
	Ampere_usart485_init(); //电流表初始化
	Button_HardInit();
	ht_usart_Init();
	Motor_EnP;
	Timer_Init(1000);
	idwg_Init(4,6250);  //10S一次喂狗
	
	while(1)		
	{
		Rxdata_Cache();	
		Rxdata_Extract();
		Data_Analy();	
		LoadCode_Process();
		HintDisplay_Send();
		Start_Testing();	
//		htData_Analyze();
////		Motor_Test();
////		AMRxdata_Cache();  //抓取数据。
////		AMRXDATA_Extract();
		iwdg_feed();
		if(led_test_time>=7000)	
		{
			led_test_time = 0;
			level = 1-level;
			GPIO_WriteBit(GPIOD,GPIO_Pin_0,level);	
		//	DCData_Tx(HWDIS1,5);
			htData_Tx(HWDIS1,5);
			
		}
	}
}

