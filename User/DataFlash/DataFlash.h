#ifndef _DataFlash_H
#define _DataFlash_H


#define Flash_Size   			512
#define Flash_Base_Address    	0x08000000

struct Para
{

	unsigned int Power_Current_Max;//上电电流
	unsigned int Power_Current_Min;
	unsigned int Power_Current_ERRORNum;

	unsigned int Value_Current_Max;//阀门开启
	unsigned int Value_Current_Min;
	unsigned int Value_Current_ERRORNum;
	
	unsigned int Standby_Current_Max;//待机电流
	unsigned int Standby_Current_Min;
	unsigned int Standby_Current_ERRORNum;
	
	unsigned int Light_Current_Max;//指示灯电流  
	unsigned int Light_Current_Min;
	unsigned int Light_Current_ERRORNum;
	
	unsigned int Shoot_Power_Max;//发射功率
	unsigned int Shoot_Power_Min;
	unsigned int Shoot_Power_ERRORNum;
	
	unsigned int Power_ADC_Max; //电源ADC
	unsigned int Power_ADC_Min;
	unsigned int Power_ADC_ERRORNum;
	
	unsigned int HW_Learn_Max;   //红外学习
	unsigned int HW_Learn_Min;
	unsigned int HW_Learn_ERRORNum;
	
	unsigned int OPA0_Value_Max; //运放校准0
	unsigned int OPA0_Value_Min;
	unsigned int OPA0_Value_ERRORNum;

	unsigned int OPA1_Value_Max; //运放校准1
	unsigned int OPA1_Value_Min;
	unsigned int OPA1_Value_ERRORNum;
		
	unsigned int GanDistance_Max;  //感应距离
	unsigned int GanDistance_Min;  //感应距离
	unsigned int GanDistance_ERRORNum;
	
};

extern struct Para Para_Data;


unsigned  char System_Read_Flash(unsigned long ReadAddr,unsigned char *pBuffer,unsigned int NumToRead);
unsigned 	char System_Write_Flash(unsigned long WriteAddr,unsigned char *WriteToBuffer, unsigned int NumToWrite);
#endif

