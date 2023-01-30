#include "StepProcess.h"
#include "DCTFT.h"
#include "DataFlash.h"
#include "button.h"
#include "Ampere.h"
#include "htusart.h"
#include "motor.h"
#include "Delay.h"
#include "LoadCode.h"
#include "idwg.h"



unsigned char  Start_ScreenId = 0;
unsigned char  TestRead_ScreenID = 3;
unsigned char  TextControl_id[] = {0x08,0x0c,0x14,0x18,0x1C,0x20,0x23,0x27,0x2a};
unsigned char  TextColor_Control[] = {0xEE,0XB1,0x18,0x00,0x01,0x00,0x00,0x00,0x00,0xFF,0xFC,0xFF,0xFF};//ÎÄ±¾±³¾°É«Êý¾Ý¸ñÊ½¡£
unsigned char  TextData_Control[] = {0xEE,0xB1,0x10,0x00,0x00,0x00,0xFF,0xFC,0xFF,0xFF};//
unsigned char  TextStored_Id[] = {0x06,0x07,0x0A,0x0B,0x12,0x13,0x16,0x17,0x1A,0x1B\
											,0x1E,0x1F,0x21,0x22,0x24,0x26,0x28,0x29};//Ò³Ãæ1ÎÄ±¾¿Ø¼þid ´óÐ¡Öµ¡£

unsigned char ReadTextControl_id[] = {0x06,0x07,0x0A,0x0B,0x12,0x13,0x16,0x17,0x1A,0x1B\
												,0x1E,0x1F,0x03,0x0d,0x0e,0x0f,0x21,0x22};  //¶ÁÈ¡²ÎÊý  Ò³Ãæ3µÄ¿Ø¼þid

unsigned char  ProgressRate[] = {0xEE,0xB1,0x10,0x00,0x03,0x00,0x14,0x00,0x00,0x00,0x00,0xFF,0xFC,0xFF,0xFF};

unsigned char HWDIS[] = {0xA5,0x01,0x01,0x01,0xA8};
unsigned char HWREAD[] = {0xA5,0x02,0x01,0x02,0xAA};
unsigned char DGREAD[] = {0xA5,0x02,0x01,0x01,0xA9};
unsigned char StorageBuffer[66];
struct Step  TestStep;
struct ParaData TestPara;
struct Com_Para ComP;

void Clear_TestSample(void);
void Storage_Data(void);
void Read_Storage_Data(unsigned char *buff,unsigned int length);
void Read_Storage_Send(void);
/******************************************************
Data_Deal:
´¦ÀíÊý¾Ý¸ñÊ½ÉÏ·¢¸ø´®¿ÚÆÁ ÀýÈç 21 ´¦ÀíÎª 0x32  0x31
Data   	  : ·¢ËÍµÄÊý¾Ý
Control_Id : ·¢ËÍµÄ¿Ø¼þId
******************************************************/
void Data_Deal_Send(unsigned int Data,unsigned int Screen_Id,unsigned int Control_Id)
{
		unsigned char TestData[15],Num = 0;
		TestData[Num++] = 0xEE;
		TestData[Num++] = 0xB1;
		TestData[Num++] = 0x10;
		if(Screen_Id < 255)
		{
			TestData[Num++] = 0;
			TestData[Num++] = Screen_Id;
		}
		else
		{
			TestData[Num++] = (Screen_Id & 0xff00)>>8;
			TestData[Num++] = Screen_Id & 0x00ff;
		}
		if(Control_Id < 255)
		{
			TestData[Num++] = 0;
			TestData[Num++] = Control_Id;
		}
		else
		{
			TestData[Num++] = (Control_Id & 0xff00)>>8;
			TestData[Num++] = Control_Id & 0x00ff;
		}
		if(Data>9999)// Íò
		{
			TestData[Num++] = (Data/10000) + 0x30;
			TestData[Num++] = ((Data/1000)%10)+ 0x30;
			TestData[Num++] = ((Data/100)%10)+ 0x30;
			TestData[Num++] = ((Data/10)%10)+ 0x30;
			TestData[Num++] = (Data%10)+ 0x30;	
		}
		else if(Data<10000 && Data>999) //Ç§
		{
			TestData[Num++] = (Data/1000)+ 0x30;
			TestData[Num++] = ((Data/100)%10)+ 0x30;
			TestData[Num++] = ((Data/10)%10)+ 0x30;
			TestData[Num++] = (Data%10)+ 0x30;	
		}
		else if(Data <1000 && Data>99) //°Ù
		{
			TestData[Num++] = (Data/100)+ 0x30;
			TestData[Num++] = ((Data/10)%10)+ 0x30;
			TestData[Num++] = (Data%10)+ 0x30;
		}
		else if(Data <100 && Data>9) //Ê®
		{
			TestData[Num++] = (Data/10)+ 0x30;
			TestData[Num++] = (Data%10)+ 0x30;
		}
		else if(Data <10) //¸ö
		{
			TestData[Num++] = Data+0x30;
		}
		TestData[Num++] = 0xFF;
		TestData[Num++] = 0xFC;
		TestData[Num++] = 0xFF;
		TestData[Num++] = 0xFF;
		DCData_Tx(TestData, Num);
}

/******************************************************
TextColor_Send:
ÉèÖÃÎÄ±¾±³¾°É«ÎªºìÉ«
******************************************************/
void TextColor_Send(unsigned char testcolorId,unsigned int color)
{
		TextColor_Control[4] = 0x01;
		TextColor_Control[5] = 0x00;
		TextColor_Control[6] = testcolorId;;
		TextColor_Control[7] = (color&0Xff00)>>8;
		TextColor_Control[8] = (color&0X00ff);
		DCData_Tx(TextColor_Control,13);
}

/******************************************************
Read_TextPara_Send:
¶ÁÈ¡²ÎÊýµÄÖ¸Áî(¶ÁÈ¡ÎÄ±¾)
Ã¿´Î¶ÁÈ¡µÈ´ý1sÖÖ Èç¹ûÖØ¸´¶ÁÈ¡5´ÎºóÃ»³É¹¦ ÔòÄ¬ÈÏÎª¶ÁÈ¡Ê§°Ü
******************************************************/

void Read_TextPara_Send(unsigned int  Screen_Id,unsigned int Control_Id)
{
	unsigned char TestData[] = {0xEE, 0xB1, 0x11, 0x00, 0x03, 0x00, 0x03, 0xFF, 0xFC, 0xFF, 0xFF};
	if(Screen_Id > 0x00ff)	
	{
		TestData[3] = (Screen_Id & 0xff00)>>8;
		TestData[4] = (Screen_Id & 0x00ff);
	}
	else
	{
		TestData[3] = 0;
		TestData[4] = (Screen_Id & 0x00ff);
	}

	if(Control_Id > 0x00ff)
	{
		TestData[5] = (Control_Id & 0xff00) >>8;
		TestData[6] = (Control_Id & 0x00ff);
	}
	else
	{
		TestData[5] = 0;
		TestData[6] = (Control_Id & 0x00ff);
	}
	DCData_Tx(TestData,11);
}


/******************************************************
ProgressData_Send:
½âÎöÊÕµ½µÄÊý¾Ý ·¢ËÍ½ø¶ÈÌõÏÔÊ¾Êý 
******************************************************/
void ProgressData_Send(unsigned long Num)
{
	if(Num > 0x00ffffff)
	{
		ProgressRate[7] = (Num&0xff000000)>>24;
		ProgressRate[8] = (Num&0x00ff0000)>>16;
		ProgressRate[9] = (Num&0x0000ff00)>>8;
		ProgressRate[10] = (Num&0x000000ff);
		
	}
	else if(Num <=0x00ffffff && Num > 0x0000ffff)
	{
		ProgressRate[7] = 0;
		ProgressRate[8] = (Num&0x00ff0000)>>16;
		ProgressRate[9] = (Num&0x0000ff00)>>8;
		ProgressRate[10] = (Num&0x000000ff);
	}
	else if(Num <=0x0000ffff && Num > 0x000000ff)
	{
		ProgressRate[7] = 0;
		ProgressRate[8] = 0;
		ProgressRate[9] = (Num&0x0000ff00)>>8;
		ProgressRate[10] = (Num&0x000000ff);
	}
	else if(Num <=0x0000ff && Num > 0x00000000)
	{
		ProgressRate[7] = 0;
		ProgressRate[8] = 0;
		ProgressRate[9] = 0;
		ProgressRate[10] = (Num&0x000000ff);
	}
	else if(Num == 0)
	{
		ProgressRate[7] = 0;
		ProgressRate[8] = 0;
		ProgressRate[9] = 0;
		ProgressRate[10] = 0;
	}
	DCData_Tx(ProgressRate,15);
}


/******************************************************
RecData_Deal:
½âÎöÊÕµ½µÄÊý¾Ý ÀýÈç0X31 0x32 0x33  ½âÎöÎª 123
******************************************************/
unsigned int RecData_Deal(unsigned char *Data,unsigned char Num)
{
	unsigned int Value = 0,i=0;
	for(i=0;i<Num;i++)
	{
		Value = Value*10 + (Data[8+i]-0x30);
	}
	return Value;
}
/******************************************************
Com_Send:
¶ÁÈ¡µçÁ÷Öµ 
******************************************************/
void Com_Send(unsigned long *Data)
{
	if(ComP.Com_Flag == 1)
	{
		switch(ComP.Com_Step)
		{
			case 0:
			{
				htRxdata_P.WaitTimerFlag =1;
				ComP.Com_time = 10;
				ComP.Com_Num = 0;
				ComP.Com_Step = 1;
				//ÉÏ·¢Êý¾Ýµ½¸ÐÓ¦Í·
			}break;
			case 1:
			{
				if((ComP.Com_Num <= Send_Num)&&(ComP.Com_time == 0))
				{
					AMPara_Send();  //¶ÁÈ¡µçÁ÷Öµ¡£
					//ComP.Com_Num ++;
				//	ComP.Com_time = Send_Wait_Time;
					ComP.Com_Step = 2; 
					ComP.Com_time = 1000;
				}
			//	else if(ComP.Com_Num >Send_Num)
			///	{
					
		//			//ÌáÊ¾¶ÁÈ¡Ê§°Ü¡£ÊÇ·ñÌø¹ý¡£
			//		ComP.Com_Flag = 0;
		///		}
				//µÈ´ý10ms ×¥È¡Êý¾Ý
			}break;	
			case 2:
			{
				if(ComP.Com_time == 0)
				{
					AMRxdata_Cache();  //×¥È¡Êý¾Ý¡£
					ComP.Com_Value = AMRXDATA_Extract();//»ñÈ¡µçÑ¹Öµ
			//	if(ComP.Com_Value != 0)
			//	{	
					*Data = ComP.Com_Value;
					ComP.Com_Step = 3;
					ComP.Com_Flag = 0;  //¸ÐÓ¦Í·µçÁ÷±íÒ»¸öÁ÷³Ì¶ÁÈ¡½áÊø
				}
			}break;
				
			//	}break;
		}		
	}

}

void Test_Step_Init(void)
{
	Clear_TestSample();
	TestStep.Test_Press  = 0;
	htRxdata_P.WaitTimerFlag = 0;
	htRxdata_P.WaitTimer = 0;
	htRxdata_P.SendNum = 0;
	TestStep.Motor_Step = 0;
}



/******************************************************
Start_Testing:
¿ªÊ¼²âÊÔµÄÁ÷³Ì²¢½«Êý¾ÝÉÏ·¢µ½ÆÁÄ»
²âÊÔÊý¾ÝÔÚ·¶Î§ÄÚµÄÕý³£ÉÏ·¢ ³¬³öÏàÓ¦·¶Î§µÄÊý¾Ý ±³¾°É«¸³ÎªºìÉ«

¢Ù·¢ËÍÊý¾ÝÈÃ¸ÐÓ¦Í·½øÐÐºìÍâÑ§Ï°¡£

******************************************************/
void Start_Testing(void)
{
	if(TestStep.Test_Start == 1)
	{
		TestStep.Test_Flag = 1; //±íÊ¾¿ªÊ¼½øÈë¼ì²â
		TextColor_Control[3] = 0x00;
		TextColor_Control[4] = Start_ScreenId;	
		switch (TestStep.Test_Press)
		{			
			case Test_Init:                     //Âí´ï¹éÎ»
			{
				iwdg_feed(); //10S Âí´ï¹éÎª Èç¹ûÃ»ÓÐ¹éÎªÔò±íÊ¾¼ì²â²»µ½Âí´ï¸ÐÓ¦Î»
				unsigned int StepP_Num = 0;
			
				while(ButtonP.MOTOR_FLAG == 0 && TestStep.Motor_ReFlag == 0)
				{
					Motor_DirN;
					for(StepP_Num=0;StepP_Num<790;StepP_Num++)
					{
						Motor_PulP;
						delay_us(50);
						Motor_PulN;
						delay_us(50);
					}	
				}
				ButtonP.MOTOR_FLAG = 0;
				htRxdata_P.WaitTimerFlag  = 0;
				TestStep.Test_Press++;
				TestStep.Motor_ReFlag = 0;
				TestStep.Motor_ReTime = 0;
			}break;
			
			case HW_Learn:
			{
				Test_Relay_L;
				if(htRxdata_P.WaitTimerFlag == 0)
				{
					htData_Tx(HWDIS,sizeof(HWDIS));
					htRxdata_P.WaitTimerFlag =1;
				}
				else if(htRxdata_P.WaitTimerFlag ==2)
				{
					TestStep.Test_Press++;
					htRxdata_P.WaitTimerFlag = 0;
					ComP.Com_Flag = 1;//ÏÂÒ»¸ö¶ÁÈ¡¿ªÊ¼
				}
			}break;	
			
			case ReadData_Send:
			{
				if(htRxdata_P.WaitTimerFlag == 0)
				{
					htData_Tx(DGREAD,sizeof(DGREAD));
					htRxdata_P.WaitTimerFlag =1;
				}
				else if(htRxdata_P.WaitTimerFlag ==2)
				{
					TestStep.Test_Press++;
					htRxdata_P.WaitTimerFlag = 0;
					ComP.Com_Flag = 1;//ÏÂÒ»¸ö¶ÁÈ¡¿ªÊ¼
					ComP.Com_Step = 0;
				}
			}break;
			
			case IndicLigh_Current://Ö¸Ê¾µÆµçÁ÷
			{
				Com_Send(&TestPara.Light_Current_Test);
				if(ComP.Com_Flag ==0&& htRxdata_P.WaitTimerFlag ==2) //µçÁ÷²É¼¯Íê³ÉÇÒ50msÍê³É
				{
					ComP.Com_Flag = 1;//ÏÂÒ»¸ö¶ÁÈ¡¿ªÊ¼
					ComP.Com_Step = 0;
					TestStep.Test_Press ++;					
				}
			}break;
			
			case Power_Current: //Çý¶¯µçÂ·Ê¹ÄÜ
			{
				Com_Send(&TestPara.Power_Current_Test);
				if(ComP.Com_Flag ==0&& htRxdata_P.WaitTimerFlag ==2)//µçÁ÷²É¼¯Íê³ÉÇÒ50msÍê³É
				{
					ComP.Com_Flag = 1;//ÏÂÒ»¸ö¶ÁÈ¡¿ªÊ¼	
					ComP.Com_Step = 0;
					TestStep.Test_Press ++;						
				}

			}break;
			
			case Standby_Current://´ý»úµçÁ÷
			{
				Com_Send(&TestPara.Standby_Current_Test);
				if(ComP.Com_Flag ==0&& htRxdata_P.WaitTimerFlag ==2)
				{
					TestStep.Test_Press ++;		
					ComP.Com_Flag = 1;//ÏÂÒ»¸ö¶ÁÈ¡¿ª¿ªÊ¼
					htRxdata_P.WaitTimerFlag =0;						
				}
			}break;
			
			case Trans_Power: //·¢Éä¹¦ÂÊ
			{
				if(htRxdata_P.WaitTimerFlag == 0)
				{
					htRxdata_P.WaitTimerFlag = 1;
				}
				htRxdata_P.Check_Value = htData_Analyze();
				//Ð£ÑéÊý¾Ý´íÎó ÇÒµÈ´ýÊ±³¤µ½ ÔòÖØÐÂ×ßÒ»±éÁ÷³Ì½øÐÐÅÐ¶Ï¡£Ò»¹²ÖØ¸´Èý´Î(ÉÏ±¨Í¨ÐÅÊ§°Ü)
				if(htRxdata_P.Check_Value == 0 && htRxdata_P.WaitTimerFlag == 2)
				{
					TestStep.Test_Press = ReadData_Send;
					htRxdata_P.SendNum++;
					if(htRxdata_P.SendNum == 3)
					{
					//	htData_Tx("11234",5);	
							//ÉÏ±¨Í¨ÐÅ´íÎó,ÍË³ö²âÊÔÄ£Ê½¡£
						htRxdata_P.SendNum = 0;
					}
				}
				//Ð£ÑéÊý¾Ý³É¹¦
			//	else if(htRxdata_P.Check_Value == 1)
				//{
					//htData_Tx("Check_Success",15);	
					TestPara.Power_ADC_Test = htRxdata_P.Data[3];
					TestPara.Shoot_Power_Test =  htRxdata_P.Data[4];
					TestPara.Learn_Test =  htRxdata_P.Data[5];
					TestPara.OPA0_Test =htRxdata_P.Data[6];
					TestPara.OPA1_Test = htRxdata_P.Data[7];
					TestStep.Test_Press ++;
					htRxdata_P.Check_Value =0;
			//	}
				
			}break;
			
			case Display_Send:
			{
					//Ö¸Ê¾µÆµçÁ÷
					if((Para_Data.Light_Current_Max <=TestPara.Light_Current_Test)||(Para_Data.Light_Current_Min>=TestPara.Light_Current_Test))
					{
					
						TextColor_Send(TextControl_id[0],Color_Red);
						Data_Deal_Send(TestPara.Light_Current_Test,0X0001,TextControl_id[0]);
					}		
					else
					{
						TextColor_Send(TextControl_id[0],0xF980);
						Data_Deal_Send(TestPara.Light_Current_Test,0X0001,TextControl_id[0]);		
					}
					//Çý¶¯µçÂ·Ê¹ÄÜÊý¾Ý·¢ËÍ = ÉÏµçµçÁ÷
				  if((Para_Data.Power_Current_Max <= TestPara.Power_Current_Test)||(Para_Data.Power_Current_Min>=TestPara.Power_Current_Test))
					{
						TextColor_Send(TextControl_id[1],Color_Red);
						Data_Deal_Send(TestPara.Power_Current_Test,0X0001,TextControl_id[1]);
					}	
					else
					{
						TextColor_Send(TextControl_id[1],0xF980);
						Data_Deal_Send(TestPara.Power_Current_Test,0X0001,TextControl_id[1]);		
					}
					//´ý»úµçÁ÷
					if((Para_Data.Standby_Current_Max <= TestPara.Standby_Current_Test)||(Para_Data.Standby_Current_Min>=TestPara.Standby_Current_Test))
					{
						TextColor_Send(TextControl_id[2],Color_Red);
						Data_Deal_Send(TestPara.Standby_Current_Test,0X0001,TextControl_id[2]);
					}
					else
					{
						TextColor_Send(TextControl_id[2],0xF980);
						Data_Deal_Send(TestPara.Standby_Current_Test,0X0001,TextControl_id[2]);		
					}	
					// µçÑ¹ADC
					if((Para_Data.Power_ADC_Max <= TestPara.Power_ADC_Test)||(Para_Data.Power_ADC_Min>=TestPara.Power_ADC_Test))
					{
						TextColor_Send(TextControl_id[3],Color_Red);
						Data_Deal_Send(TestPara.Power_ADC_Test,0X0001,TextControl_id[3]);
					}
					else
					{
						TextColor_Send(TextControl_id[3],0xF980);
						Data_Deal_Send(TestPara.Power_ADC_Test,0X0001,TextControl_id[3]);		
					}	
					//ºìÍâ·¢Éä¹¦ÂÊ
					if((Para_Data.HW_Learn_Max <= TestPara.Learn_Test)||(Para_Data.HW_Learn_Min>=TestPara.Learn_Test))
					{
						TextColor_Send(TextControl_id[4],Color_Red);
						Data_Deal_Send(TestPara.Learn_Test,0X0001,TextControl_id[4]);
					}
					else
					{
						TextColor_Send(TextControl_id[4],0xF980);
						Data_Deal_Send(TestPara.Learn_Test,0X0001,TextControl_id[4]);		
					}	
					//ºìÍâÑ§Ï°½á¹û
					if((Para_Data.Shoot_Power_Max <= TestPara.Shoot_Power_Test)||(Para_Data.Light_Current_Min>=TestPara.Shoot_Power_Test))
					{
						TextColor_Send(TextControl_id[5],Color_Red);
						Data_Deal_Send(TestPara.Shoot_Power_Test,0X0001,TextControl_id[5]);
					}
					else
					{
						TextColor_Send(TextControl_id[5],0xF980);
						Data_Deal_Send(TestPara.Shoot_Power_Test,0X0001,TextControl_id[5]);		
					}
					//ÔË·ÅÐ£×¼0
					if((Para_Data.OPA0_Value_Max <=TestPara.OPA0_Test)||(Para_Data.OPA0_Value_Min>=TestPara.OPA0_Test))
					{
						TextColor_Send(TextControl_id[6],Color_Red);
						Data_Deal_Send(TestPara.Shoot_Power_Test,0X0001,TextControl_id[6]);	
					}
					else
					{
						TextColor_Send(TextControl_id[6],0xF980);
						Data_Deal_Send(TestPara.Shoot_Power_Test,0X0001,TextControl_id[6]);	
					}
					//ÔË·ÅÐ£×¼1
					if((Para_Data.OPA1_Value_Max <= TestPara.OPA1_Test)||(Para_Data.OPA1_Value_Min)>=TestPara.OPA1_Test)
					{
						TextColor_Send(TextControl_id[7],Color_Red);
						Data_Deal_Send(TestPara.Shoot_Power_Test,0X0001,TextControl_id[7]);
					}
					else
					{
						TextColor_Send(TextControl_id[7],0xF980);
						Data_Deal_Send(TestPara.Shoot_Power_Test,0X0001,TextControl_id[7]);	
					}
					TestStep.Test_Press ++;				
			}break;
			case Ganyin_Distance:  //¸ÐÓ¦¾àÀë
			{	
				unsigned  int StepN_Num=0;
				switch(TestStep.Motor_Step)
				{
					case 0:
					{
						Motor_DirP;
						for(StepN_Num=0;StepN_Num<11060;StepN_Num++) // 790¿É¸ü¸ÄÎª¾ßÌåµÄ²ÎÊýÖµ(´Ó¶¥²¿µ½¾àÀëÎª6)
						{
							Motor_PulP;
							delay_us(50);
							Motor_PulN;
							delay_us(50);
						}				
////						delay_us(150);							
						if(GPIO_ReadInputDataBit(GPIOE, GATE2_DET) == 0) //¼ì²âµ½ Ôò¿ªÊ¼¼ì²âÉÏÏßÖµ
						{
							TestStep.Motor_Step = 1;
							
						}
						else					//Ìø³ö¼ì²âÌáÊ¾ÎªµÍ
						{
							TestStep.Test_Press =Restart_Motor;
							//TestStep.Test_Start = 0; //Í£Ö¹¼ì²â
							//TestStep.Test_Flag =  0; //¼ì²âÍê³É¡£
							TextColor_Send(TextControl_id[8],Color_Red);
							Data_Deal_Send(0,0X0001,TextControl_id[8]);
						}
					}break;
					case 1:
					{
						Motor_DirN;
						for(StepN_Num=0;StepN_Num<3160;StepN_Num++) // 790¿É¸ü¸ÄÎª¾ßÌåµÄ²ÎÊýÖµ(´Ó¾àÀëÎª6->¾àÀëÎª8)
						{
							Motor_PulP;
							delay_us(50);
							Motor_PulN;
							delay_us(50);
						}	
////						delay_us(10);	
						if(GPIO_ReadInputDataBit(GPIOE, GATE1_DET) == 0) //¼ì²âµ½ÌáÊ¾¾àÀë¹ý³¤
						{
							TextColor_Send(TextControl_id[8],Color_Red);
							Data_Deal_Send(2,0X0001,TextControl_id[8]);
						}
						else							//Ìø³ö¼ì²â  
						{
							TextColor_Send(TextControl_id[8],0xF980);
							Data_Deal_Send(1,0X0001,TextControl_id[8]);	
						}
					//	TestStep.Test_Flag =  0; //¼ì²âÍê³É¡£
					//TestStep.Test_Start = 0; //Í£Ö¹¼ì²â
						TestStep.Test_Press = Restart_Motor; //Á÷³ÌÎª0	
					}break;
				}
			}break;	
			
			case Restart_Motor:
			{
				unsigned int Step_Num = 0;
				
				while((ButtonP.MOTOR_FLAG == 0) &&(TestStep.Motor_ReFlag == 0))
				{
					Motor_DirN;
					for(Step_Num=0;Step_Num<790;Step_Num++)
					{
						Motor_PulP;
						delay_us(50);
						Motor_PulN;
						delay_us(50);
					}	
				}
				TestStep.Motor_ReFlag = 0;
				TestStep.Motor_ReTime = 0;
////				TestPara.Power_ADC_Test = 0;
////				TestPara.Shoot_Power_Test =  0;
////				TestPara.Learn_Test =  0;
////				TestPara.OPA0_Test =0;
////				TestPara.OPA1_Test = 0;
				Test_Relay_H;
				
				ButtonP.MOTOR_FLAG = 0;
				
				TestStep.Test_Flag =  0; //¼ì²âÍê³É¡£
				TestStep.Test_Start = 0; //Í£Ö¹¼ì²â
				TestStep.Test_Press = 0; //Á÷³ÌÎª0	
			}break;
		}
	}
	if(LoadCodeP.HintDisPlayFLag == 2)
	{
		HintYinCang_Send();
		LoadCodeP.HintDisPlayFLag = 0;
	}
}	 

/******************************************************
Clear_TestSample:
Çå³ý²âÊÔÁÐÎÄ±¾µÄÊý¾Ý 
Çå³þÎÄ±¾ÑÕÉ«
Î´¼ì²âµÄÊý¾ÝÖ±½Ó¸³Îª0
******************************************************/
void Clear_TestSample(void)
{
	unsigned char Clear_Code[11] = {0xEE,0xB1,0x10,0x00,0x00,0x00,0x00\
											,0xFF,0xfC,0xff,0xff};
	unsigned char i = 0;
	Clear_Code[4] = 0x01;
	Clear_Code[5] = 0x00;
	for(i=0;i<11;i++)
	{
		Clear_Code[6] = TextControl_id[i];
		DCData_Tx(Clear_Code,11);		
		TextColor_Send(TextControl_id[i],Color_White);		
	}
}

/******************************************************
Stored_Data:
»º´æµÄÊý¾ÝÉÏ·¢µ½´®¿ÚÆÁ
******************************************************/
void Stored_Data(void)
{
	Data_Deal_Send(Para_Data.Light_Current_Max,0X0001,TextStored_Id[0]);  //Ö¸Ê¾µÆµçÁ÷
	Data_Deal_Send(Para_Data.Light_Current_Min,0X0001,TextStored_Id[1]);  
	Data_Deal_Send(Para_Data.Power_Current_Max,0X0001,TextStored_Id[2]);  //¼ì²â·§Çý¶¯µçÁ÷
	Data_Deal_Send(Para_Data.Power_Current_Min,0X0001,TextStored_Id[3]);
	//Data_Deal_Send(Para_Data.Value_Current_Max,0X0001,TextStored_Id[4]);
	//Data_Deal_Send(Para_Data.Value_Current_Min,0X0001,TextStored_Id[5]);
	Data_Deal_Send(Para_Data.Standby_Current_Max,0X0001,TextStored_Id[4]);  //´ý»úµçÁ÷
	Data_Deal_Send(Para_Data.Standby_Current_Min,0X0001,TextStored_Id[5]);
	Data_Deal_Send(Para_Data.Power_ADC_Max,0X0001,TextStored_Id[6]);				//µçÔ´ADC
	Data_Deal_Send(Para_Data.Power_ADC_Min,0X0001,TextStored_Id[7]);
	Data_Deal_Send(Para_Data.Shoot_Power_Max,0X0001,TextStored_Id[8]);			//ºìÍâ·¢Éä¹¦ÂÊ
	Data_Deal_Send(Para_Data.Shoot_Power_Min,0X0001,TextStored_Id[9]);
	Data_Deal_Send(Para_Data.HW_Learn_Max,0X0001,TextStored_Id[10]);				//ºìÍâÑ§Ï°½á¹û
	Data_Deal_Send(Para_Data.HW_Learn_Min,0X0001,TextStored_Id[11]);

	Data_Deal_Send(Para_Data.OPA0_Value_Max,0X0001,TextStored_Id[12]);
	Data_Deal_Send(Para_Data.OPA0_Value_Min,0X0001,TextStored_Id[13]);
	Data_Deal_Send(Para_Data.OPA1_Value_Max,0X0001,TextStored_Id[14]);
	Data_Deal_Send(Para_Data.OPA1_Value_Min,0X0001,TextStored_Id[15]);
	Data_Deal_Send(Para_Data.GanDistance_Max,0X0001,TextStored_Id[16]);
	Data_Deal_Send(Para_Data.GanDistance_Min,0X0001,TextStored_Id[17]);

}





void Data3_Send(void)
{
		Data_Deal_Send(Para_Data.Light_Current_Max,0X0003,ReadTextControl_id[0]);  //Ö¸Ê¾µÆµçÁ÷
	Data_Deal_Send(Para_Data.Light_Current_Min,0X0003,ReadTextControl_id[1]);  
	Data_Deal_Send(Para_Data.Power_Current_Max,0X0003,ReadTextControl_id[2]);  //¼ì²â·§Çý¶¯µçÁ÷
	Data_Deal_Send(Para_Data.Power_Current_Min,0X0003,ReadTextControl_id[3]);
	//Data_Deal_Send(Para_Data.Value_Current_Max,0X0001,TextStored_Id[4]);
	//Data_Deal_Send(Para_Data.Value_Current_Min,0X0001,TextStored_Id[5]);
	Data_Deal_Send(Para_Data.Standby_Current_Max,0X0003,ReadTextControl_id[4]);  //´ý»úµçÁ÷
	Data_Deal_Send(Para_Data.Standby_Current_Min,0X0003,ReadTextControl_id[5]);
	Data_Deal_Send(Para_Data.Power_ADC_Max,0X0003,ReadTextControl_id[6]);				//µçÔ´ADC
	Data_Deal_Send(Para_Data.Power_ADC_Min,0X0003,ReadTextControl_id[7]);
	Data_Deal_Send(Para_Data.Shoot_Power_Max,0X0003,ReadTextControl_id[8]);			//ºìÍâ·¢Éä¹¦ÂÊ
	Data_Deal_Send(Para_Data.Shoot_Power_Min,0X0003,ReadTextControl_id[9]);
	Data_Deal_Send(Para_Data.HW_Learn_Max,0X0003,ReadTextControl_id[10]);				//ºìÍâÑ§Ï°½á¹û
	Data_Deal_Send(Para_Data.HW_Learn_Min,0X0003,ReadTextControl_id[11]);

	Data_Deal_Send(Para_Data.OPA0_Value_Max,0X0003,ReadTextControl_id[12]);
	Data_Deal_Send(Para_Data.OPA0_Value_Min,0X0003,ReadTextControl_id[13]);
	Data_Deal_Send(Para_Data.OPA1_Value_Max,0X0003,ReadTextControl_id[14]);
	Data_Deal_Send(Para_Data.OPA1_Value_Min,0X0003,ReadTextControl_id[15]);
	Data_Deal_Send(Para_Data.GanDistance_Max,0X0003,ReadTextControl_id[16]);
	Data_Deal_Send(Para_Data.GanDistance_Min,0X0003,ReadTextControl_id[17]);
}


/******************************************************
Data_Analy:¡
Êý¾Ý½âÎö ½âÎö
******************************************************/
void Data_Analy(void)
{
	if(DC_Analay.packe == 1)  //ÅÐ¶ÏÍêÒ»°üÊý¾ÝÁË
	{	
		if(DC_Analay.Data[0] == 0xEE && DC_Analay.Data[1] == 0xB1) // ½âÎö°üÍ·ÊÇ·ñÎª0xee
		{
			
			if(DC_Analay.Data[2]== 0X11) //°´¼ü²Ù×÷²¿·Ö
			{
				switch(DC_Analay.Data[4])//ÅÐ¶ÏÔÙÄÄ¸öÒ³Ãæ²Ù×÷
				{
				  //²ÎÊý½çÃæµÄ°´Å¥Ñ¡Ïî
					case 0x01:
					{
						if(DC_Analay.Data[9] == 0X01)
						{
							switch (DC_Analay.Data[6])
							{
								//Ìø¹ý°´Å¥
								case 0x34: 
								{
									
								}break;
								//ÖÕÖ¹°´Å¥
								case 0x36:
								{
									TestStep.Test_Start = 0; //¿ªÊ¼²âÊÔ
								}break;
								//ÍË³öº¯Êý°´Å¥
								case 0x2b:
								{
									TestStep.Test_Start = 0;
									TestStep.Test_Flag = 0;
									Clear_TestSample();
								}break;
								//ÉÕÂ¼³ÌÐò°´Å¥							
								case 0x37:
								{
										ButtonP.KEYCODE_FLAG = 1;								
								}break;
								//¿ªÊ¼²âÊÔ°´Å¥
								case 0x30:
								{
									TestStep.Test_Start = 1; //¿ªÊ¼²âÊÔ
									Test_Step_Init();
								}
							}
						}
					}break;
					//ÖØÐÂÉèÖÃ²ÎÊýÒ³Ãæ
					case 0x03:
					{
						unsigned char TestDatalen = 0;
						
						if(DC_Analay.Data[9] == 0X01)
						{
							switch(DC_Analay.Data[6])
							{
								case 0x0c:  // ±£´æ²ÎÊý°´Å¥
								{
									TestStep.Test_ReadFlag = 1;
									TestStep.Test_ReadFlag_Press = 0;
									TestStep.Test_ReadRec_Flag = 0;
									ProgressData_Send(0);
									//Read_TextPara();
								}break;							
							}
						}
						else if(DC_Analay.Data[7] == 0x11)
						{
							TestDatalen = DC_Analay.PackLength  - 13;
							if(TestStep.Test_ReadFlag == 1)
							{
								switch(DC_Analay.Data[6])
								{
										case 0x06:  //Ö¸Ê¾µÆ
										{
											Para_Data.Light_Current_Max=RecData_Deal(DC_Analay.Data,TestDatalen);
											ProgressData_Send(15);
										}break;
										case 0x07:
										{
											Para_Data.Light_Current_Min=RecData_Deal(DC_Analay.Data,TestDatalen);
											ProgressData_Send(20);
										}break;
										case 0x0A:  //Çý¶¯µçÂ·
										{
											Para_Data.Power_Current_Max=RecData_Deal(DC_Analay.Data,TestDatalen);
											ProgressData_Send(22);
										}break;
										case 0x0B:
										{			
											Para_Data.Power_Current_Min=RecData_Deal(DC_Analay.Data,TestDatalen);
											ProgressData_Send(25);
										}break;

										case 0x12: //´ý»úµçÁ÷
										{
											Para_Data.Standby_Current_Max=RecData_Deal(DC_Analay.Data,TestDatalen);
											ProgressData_Send(40);
										}break;
										case 0x13:
										{
											Para_Data.Standby_Current_Min=RecData_Deal(DC_Analay.Data,TestDatalen);
											ProgressData_Send(45);
										}break;
										case 0x16:  //µçÔ´ADC
										{
											Para_Data.Power_ADC_Max=RecData_Deal(DC_Analay.Data,TestDatalen);
											ProgressData_Send(50);
										}break;
										case 0x17:
										{
											Para_Data.Power_ADC_Min=RecData_Deal(DC_Analay.Data,TestDatalen);
											ProgressData_Send(55);
										}break;
										case 0x1A: //ºìÍâÑ§Ï°½á¹û
										{
											Para_Data.HW_Learn_Max=RecData_Deal(DC_Analay.Data,TestDatalen);
											ProgressData_Send(60);
										}break;
										case 0x1B: //ºìÍâÑ§Ï°½á¹û
										{
											Para_Data.HW_Learn_Min=RecData_Deal(DC_Analay.Data,TestDatalen);
											ProgressData_Send(65);
										}break;
										case 0x1E: //ºìÍâ·¢Éä¹¦ÂÊ
										{
											Para_Data.Shoot_Power_Max=RecData_Deal(DC_Analay.Data,TestDatalen);
											ProgressData_Send(70);	
										}break;
										case 0x1F:
										{
											Para_Data.Shoot_Power_Min=RecData_Deal(DC_Analay.Data,TestDatalen);
											ProgressData_Send(74);
										}break;
										case 0x03: //ÔË·ÅÐ£×¼Öµ0
										{
											Para_Data.OPA0_Value_Max = RecData_Deal(DC_Analay.Data,TestDatalen);
											ProgressData_Send(82);
										}break;
										case 0x0D:
										{
											Para_Data.OPA0_Value_Min = RecData_Deal(DC_Analay.Data,TestDatalen);
											ProgressData_Send(85);
										}break;
										
										case 0x0E:  //ÔË·ÅÐ£×¼Öµ1
										{
											Para_Data.OPA1_Value_Max = RecData_Deal(DC_Analay.Data,TestDatalen);
											ProgressData_Send(89);
										}break;
										case 0x0F:
										{
											Para_Data.OPA1_Value_Min = RecData_Deal(DC_Analay.Data,TestDatalen);
											ProgressData_Send(92);
										}break;
										case 0x21: //¸ÐÓ¦¾àÀë
										{
											Para_Data.GanDistance_Max=RecData_Deal(DC_Analay.Data,TestDatalen);
											ProgressData_Send(97);
										}break;
										case 0x22:
										{
											Para_Data.GanDistance_Min=RecData_Deal(DC_Analay.Data,TestDatalen);
											ProgressData_Send(100);
											TestStep.Test_ReadFlag = 0;
											//·¢ËÍÊý¾Ý´¢´æ 
											Storage_Data();
										}break;										
									}
								}
						}
					}break;
				}
			}
			else if(DC_Analay.Data[2] == 0x01)  //ÇÐ»»»­Ãæ²¿·Ö
			{
				if(DC_Analay.Data[4] == 0x01) //ÇÐ»»µ½²ÎÊýÏÔÊ¾µÄÄÇ¸ö»­Ãæ
				{
					HintYinCang_Send(); //Òþ²Ø×é¼þ
					Read_Storage_Send();					
				}
				else if(DC_Analay.Data[4] == 0X03)  //ÉÏ´«Êý¾Ý
				{
					Data3_Send();
					ProgressData_Send(0);  //½ø¶ÈÌõÇå0
				}
			}
		} 
		else if (DC_Analay.Data[0] == 0xEE && DC_Analay.Data[1] == 0x0B)  //Êý¾Ý´æ´¢
		{
			Read_Storage_Data(DC_Analay.Data,DC_Analay.PackLength);
		//	Storage_Data();
		}
		DC_Analay.packe = 0;
	}
}

void Read_Storage_Send(void)
{
	unsigned char buf[12] = {0xEE,0x88,0x00,0x00,0x00,0x00,0x00,0x36,0xFF,0xFC,0xFF,0xFF};
	DCData_Tx(buf,12);
}

void Read_Storage_Data(unsigned char *buff,unsigned int length)
{
	//Ö¸Ê¾µÆ
	Para_Data.Light_Current_Max = 	buff[2]<<8|buff[3];
	Para_Data.Light_Current_Min = 	buff[4]<<8|buff[5];
	Para_Data.Light_Current_ERRORNum = buff[6]<<8|buff[7];
	//Çý¶¯µçÂ·Ê¹ÄÜ
	Para_Data.Power_Current_Max	= buff[8]<<8|buff[9];
	Para_Data.Power_Current_Min = buff[10]<<8|buff[11];
	Para_Data.Power_Current_ERRORNum = buff[12]<<8|buff[13];
	//´ý»úµçÁ÷
	Para_Data.Standby_Current_Max	= buff[14]<<8|buff[15];
	Para_Data.Standby_Current_Min = buff[16]<<8|buff[17];
	Para_Data.Standby_Current_ERRORNum = buff[18]<<8|buff[19];
	//µçÑ¹ADC
	Para_Data.Power_ADC_Max	= buff[20]<<8|buff[21];
	Para_Data.Power_ADC_Min = buff[22]<<8|buff[23];
	Para_Data.Power_ADC_ERRORNum = buff[24]<<8|buff[25];
	//ºìÍâÑ§Ï°½á¹û
	Para_Data.HW_Learn_Max	= buff[26]<<8|buff[27];
	Para_Data.HW_Learn_Min = buff[28]<<8|buff[29];
	Para_Data.HW_Learn_ERRORNum = buff[30]<<8|buff[31];
	//ºìÍâ·¢Éä¹¦ÂÊ
	Para_Data.Shoot_Power_Max	= buff[32]<<8|buff[33];
	Para_Data.Shoot_Power_Min = buff[34]<<8|buff[35];
	Para_Data.Shoot_Power_ERRORNum = buff[36]<<8|buff[37];
	//ÔË·ÅÐ£×¼0
	Para_Data.OPA0_Value_Max	= buff[38]<<8|buff[39];
	Para_Data.OPA0_Value_Min = buff[40]<<8|buff[41];
	Para_Data.OPA0_Value_ERRORNum = buff[42]<<8|buff[43];	
	//ÔË·ÅÐ£×¼1
	Para_Data.OPA1_Value_Max	= buff[44]<<8|buff[45];
	Para_Data.OPA1_Value_Min = buff[46]<<8|buff[47];
	Para_Data.OPA1_Value_ERRORNum = buff[48]<<8|buff[49];
	//¸ÐÓ¦¾àÀë
	Para_Data.GanDistance_Max	= buff[50]<<8|buff[51];
	Para_Data.GanDistance_Min = buff[52]<<8|buff[53];
	Para_Data.Power_ADC_ERRORNum = buff[54]<<8|buff[55];
	Stored_Data();
}
void Storage_Data(void)
{
	StorageBuffer[0] = 0xEE;
	StorageBuffer[1] = 0x87;
	StorageBuffer[2] = 0x00;
	StorageBuffer[3] = 0x00;
	StorageBuffer[4] = 0x00;
	StorageBuffer[5] = 0x00;
	//Ö¸Ê¾µÆ
	StorageBuffer[6] = Para_Data.Light_Current_Max >> 8;
	StorageBuffer[7] = Para_Data.Light_Current_Max&0x00ff;
	StorageBuffer[8] = Para_Data.Light_Current_Min >> 8;
	StorageBuffer[9] = Para_Data.Light_Current_Min&0x00ff;
	StorageBuffer[10] = Para_Data.Light_Current_ERRORNum >> 8;
	StorageBuffer[11] = Para_Data.Light_Current_ERRORNum&0x00ff;
	//Çý¶¯µçÂ·Ê¹ÄÜ
	StorageBuffer[12] = Para_Data.Power_Current_Max >> 8;
	StorageBuffer[13] = Para_Data.Power_Current_Max&0x00ff;
	StorageBuffer[14] = Para_Data.Power_Current_Min >> 8;
	StorageBuffer[15] = Para_Data.Power_Current_Min&0x00ff;
	StorageBuffer[16] = Para_Data.Power_Current_ERRORNum >> 8;
	StorageBuffer[17] = Para_Data.Power_Current_ERRORNum&0x00ff;
	//´ý»úµçÁ÷
	StorageBuffer[18] = Para_Data.Standby_Current_Max >> 8;
	StorageBuffer[19] = Para_Data.Standby_Current_Max&0x00ff;
	StorageBuffer[20] = Para_Data.Standby_Current_Min >> 8;
	StorageBuffer[21] = Para_Data.Standby_Current_Min&0x00ff;
	StorageBuffer[22] = Para_Data.Standby_Current_ERRORNum >> 8;
	StorageBuffer[23] = Para_Data.Standby_Current_ERRORNum&0x00ff;
	//µçÑ¹ADC
	StorageBuffer[24] = Para_Data.Power_ADC_Max >> 8;
	StorageBuffer[25] = Para_Data.Power_ADC_Max&0x00ff;
	StorageBuffer[26] = Para_Data.Power_ADC_Min >> 8;
	StorageBuffer[27] = Para_Data.Power_ADC_Min&0x00ff;
	StorageBuffer[28] = Para_Data.Power_ADC_ERRORNum >> 8;
	StorageBuffer[29] = Para_Data.Power_ADC_ERRORNum&0x00ff;
	//ºìÍâÑ§Ï°½á¹û
	StorageBuffer[30] = Para_Data.HW_Learn_Max >> 8;
	StorageBuffer[31] = Para_Data.HW_Learn_Max&0x00ff;
	StorageBuffer[32] = Para_Data.HW_Learn_Min >> 8;
	StorageBuffer[33] = Para_Data.HW_Learn_Min&0x00ff;
	StorageBuffer[34] = Para_Data.HW_Learn_ERRORNum >> 8;
	StorageBuffer[35] = Para_Data.HW_Learn_ERRORNum&0x00ff;
	//ºìÍâ·¢Éä¹¦ÂÊ
	StorageBuffer[36] = Para_Data.Shoot_Power_Max >> 8;
	StorageBuffer[37] = Para_Data.Shoot_Power_Max&0x00ff;
	StorageBuffer[38] = Para_Data.Shoot_Power_Min >> 8;
	StorageBuffer[39] = Para_Data.Shoot_Power_Min&0x00ff;
	StorageBuffer[40] = Para_Data.Shoot_Power_ERRORNum >> 8;
	StorageBuffer[41] = Para_Data.Shoot_Power_ERRORNum&0x00ff;
	//ÔË·ÅÐ£×¼0
	StorageBuffer[42] = Para_Data.OPA0_Value_Max >> 8;
	StorageBuffer[43] = Para_Data.OPA0_Value_Max&0x00ff;
	StorageBuffer[44] = Para_Data.OPA0_Value_Min >> 8;
	StorageBuffer[45] = Para_Data.OPA0_Value_Min&0x00ff;
	StorageBuffer[46] = Para_Data.OPA0_Value_ERRORNum >> 8;
	StorageBuffer[47] = Para_Data.OPA0_Value_ERRORNum&0x00ff;
	//ÔË·ÅÐ£×¼1
	StorageBuffer[48] = Para_Data.OPA1_Value_Max >> 8;
	StorageBuffer[49] = Para_Data.OPA1_Value_Max&0x00ff;
	StorageBuffer[50] = Para_Data.OPA1_Value_Min >> 8;
	StorageBuffer[51] = Para_Data.OPA1_Value_Min&0x00ff;
	StorageBuffer[52] = Para_Data.OPA1_Value_ERRORNum >> 8;
	StorageBuffer[53] = Para_Data.OPA1_Value_ERRORNum&0x00ff;
	
	//¸ÐÓ¦¾àÀë
	StorageBuffer[54] = Para_Data.GanDistance_Max>> 8;
	StorageBuffer[55] = Para_Data.GanDistance_Max&0x00ff;
	StorageBuffer[56] = Para_Data.GanDistance_Min>> 8;
	StorageBuffer[57] = Para_Data.GanDistance_Min&0x00ff;
	StorageBuffer[58] = Para_Data.GanDistance_ERRORNum >> 8;
	StorageBuffer[59] = Para_Data.GanDistance_ERRORNum&0x00ff;
	
	StorageBuffer[60] = 0xff;
	StorageBuffer[61] = 0xfc;
	StorageBuffer[62] = 0xff;
	StorageBuffer[63] = 0xff;
	
	DCData_Tx(StorageBuffer,sizeof(StorageBuffer)/sizeof(char));	
}

