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
unsigned char  TextColor_Control[] = {0xEE,0XB1,0x18,0x00,0x01,0x00,0x00,0x00,0x00,0xFF,0xFC,0xFF,0xFF};//�ı�����ɫ���ݸ�ʽ��
unsigned char  TextData_Control[] = {0xEE,0xB1,0x10,0x00,0x00,0x00,0xFF,0xFC,0xFF,0xFF};//
unsigned char  TextStored_Id[] = {0x06,0x07,0x0A,0x0B,0x12,0x13,0x16,0x17,0x1A,0x1B\
											,0x1E,0x1F,0x21,0x22,0x24,0x26,0x28,0x29};//ҳ��1�ı��ؼ�id ��Сֵ��

unsigned char ReadTextControl_id[] = {0x06,0x07,0x0A,0x0B,0x12,0x13,0x16,0x17,0x1A,0x1B\
												,0x1E,0x1F,0x03,0x0d,0x0e,0x0f,0x21,0x22};  //��ȡ����  ҳ��3�Ŀؼ�id

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
�������ݸ�ʽ�Ϸ��������� ���� 21 ����Ϊ 0x32  0x31
Data   	  : ���͵�����
Control_Id : ���͵Ŀؼ�Id
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
		if(Data>9999)// ��
		{
			TestData[Num++] = (Data/10000) + 0x30;
			TestData[Num++] = ((Data/1000)%10)+ 0x30;
			TestData[Num++] = ((Data/100)%10)+ 0x30;
			TestData[Num++] = ((Data/10)%10)+ 0x30;
			TestData[Num++] = (Data%10)+ 0x30;	
		}
		else if(Data<10000 && Data>999) //ǧ
		{
			TestData[Num++] = (Data/1000)+ 0x30;
			TestData[Num++] = ((Data/100)%10)+ 0x30;
			TestData[Num++] = ((Data/10)%10)+ 0x30;
			TestData[Num++] = (Data%10)+ 0x30;	
		}
		else if(Data <1000 && Data>99) //��
		{
			TestData[Num++] = (Data/100)+ 0x30;
			TestData[Num++] = ((Data/10)%10)+ 0x30;
			TestData[Num++] = (Data%10)+ 0x30;
		}
		else if(Data <100 && Data>9) //ʮ
		{
			TestData[Num++] = (Data/10)+ 0x30;
			TestData[Num++] = (Data%10)+ 0x30;
		}
		else if(Data <10) //��
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
�����ı�����ɫΪ��ɫ
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
��ȡ������ָ��(��ȡ�ı�)
ÿ�ζ�ȡ�ȴ�1s�� ����ظ���ȡ5�κ�û�ɹ� ��Ĭ��Ϊ��ȡʧ��
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
�����յ������� ���ͽ�������ʾ�� 
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
�����յ������� ����0X31 0x32 0x33  ����Ϊ 123
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
��ȡ����ֵ 
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
				//�Ϸ����ݵ���Ӧͷ
			}break;
			case 1:
			{
				if((ComP.Com_Num <= Send_Num)&&(ComP.Com_time == 0))
				{
					AMPara_Send();  //��ȡ����ֵ��
					//ComP.Com_Num ++;
				//	ComP.Com_time = Send_Wait_Time;
					ComP.Com_Step = 2; 
					ComP.Com_time = 1000;
				}
			//	else if(ComP.Com_Num >Send_Num)
			///	{
					
		//			//��ʾ��ȡʧ�ܡ��Ƿ�������
			//		ComP.Com_Flag = 0;
		///		}
				//�ȴ�10ms ץȡ����
			}break;	
			case 2:
			{
				if(ComP.Com_time == 0)
				{
					AMRxdata_Cache();  //ץȡ���ݡ�
					ComP.Com_Value = AMRXDATA_Extract();//��ȡ��ѹֵ
			//	if(ComP.Com_Value != 0)
			//	{	
					*Data = ComP.Com_Value;
					ComP.Com_Step = 3;
					ComP.Com_Flag = 0;  //��Ӧͷ������һ�����̶�ȡ����
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
��ʼ���Ե����̲��������Ϸ�����Ļ
���������ڷ�Χ�ڵ������Ϸ� ������Ӧ��Χ������ ����ɫ��Ϊ��ɫ

�ٷ��������ø�Ӧͷ���к���ѧϰ��

******************************************************/
void Start_Testing(void)
{
	if(TestStep.Test_Start == 1)
	{
		TestStep.Test_Flag = 1; //��ʾ��ʼ������
		TextColor_Control[3] = 0x00;
		TextColor_Control[4] = Start_ScreenId;	
		switch (TestStep.Test_Press)
		{			
			case Test_Init:                     //����λ
			{
				iwdg_feed(); //10S ����Ϊ ���û�й�Ϊ���ʾ��ⲻ������Ӧλ
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
					ComP.Com_Flag = 1;//��һ����ȡ��ʼ
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
					ComP.Com_Flag = 1;//��һ����ȡ��ʼ
					ComP.Com_Step = 0;
				}
			}break;
			
			case IndicLigh_Current://ָʾ�Ƶ���
			{
				Com_Send(&TestPara.Light_Current_Test);
				if(ComP.Com_Flag ==0&& htRxdata_P.WaitTimerFlag ==2) //�����ɼ������50ms���
				{
					ComP.Com_Flag = 1;//��һ����ȡ��ʼ
					ComP.Com_Step = 0;
					TestStep.Test_Press ++;					
				}
			}break;
			
			case Power_Current: //������·ʹ��
			{
				Com_Send(&TestPara.Power_Current_Test);
				if(ComP.Com_Flag ==0&& htRxdata_P.WaitTimerFlag ==2)//�����ɼ������50ms���
				{
					ComP.Com_Flag = 1;//��һ����ȡ��ʼ	
					ComP.Com_Step = 0;
					TestStep.Test_Press ++;						
				}

			}break;
			
			case Standby_Current://��������
			{
				Com_Send(&TestPara.Standby_Current_Test);
				if(ComP.Com_Flag ==0&& htRxdata_P.WaitTimerFlag ==2)
				{
					TestStep.Test_Press ++;		
					ComP.Com_Flag = 1;//��һ����ȡ����ʼ
					htRxdata_P.WaitTimerFlag =0;						
				}
			}break;
			
			case Trans_Power: //���书��
			{
				if(htRxdata_P.WaitTimerFlag == 0)
				{
					htRxdata_P.WaitTimerFlag = 1;
				}
				htRxdata_P.Check_Value = htData_Analyze();
				//У�����ݴ��� �ҵȴ�ʱ���� ��������һ�����̽����жϡ�һ���ظ�����(�ϱ�ͨ��ʧ��)
				if(htRxdata_P.Check_Value == 0 && htRxdata_P.WaitTimerFlag == 2)
				{
					TestStep.Test_Press = ReadData_Send;
					htRxdata_P.SendNum++;
					if(htRxdata_P.SendNum == 3)
					{
					//	htData_Tx("11234",5);	
							//�ϱ�ͨ�Ŵ���,�˳�����ģʽ��
						htRxdata_P.SendNum = 0;
					}
				}
				//У�����ݳɹ�
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
					//ָʾ�Ƶ���
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
					//������·ʹ�����ݷ��� = �ϵ����
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
					//��������
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
					// ��ѹADC
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
					//���ⷢ�书��
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
					//����ѧϰ���
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
					//�˷�У׼0
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
					//�˷�У׼1
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
			case Ganyin_Distance:  //��Ӧ����
			{	
				unsigned  int StepN_Num=0;
				switch(TestStep.Motor_Step)
				{
					case 0:
					{
						Motor_DirP;
						for(StepN_Num=0;StepN_Num<11060;StepN_Num++) // 790�ɸ���Ϊ����Ĳ���ֵ(�Ӷ���������Ϊ6)
						{
							Motor_PulP;
							delay_us(50);
							Motor_PulN;
							delay_us(50);
						}				
////						delay_us(150);							
						if(GPIO_ReadInputDataBit(GPIOE, GATE2_DET) == 0) //��⵽ ��ʼ�������ֵ
						{
							TestStep.Motor_Step = 1;
							
						}
						else					//���������ʾΪ��
						{
							TestStep.Test_Press =Restart_Motor;
							//TestStep.Test_Start = 0; //ֹͣ���
							//TestStep.Test_Flag =  0; //�����ɡ�
							TextColor_Send(TextControl_id[8],Color_Red);
							Data_Deal_Send(0,0X0001,TextControl_id[8]);
						}
					}break;
					case 1:
					{
						Motor_DirN;
						for(StepN_Num=0;StepN_Num<3160;StepN_Num++) // 790�ɸ���Ϊ����Ĳ���ֵ(�Ӿ���Ϊ6->����Ϊ8)
						{
							Motor_PulP;
							delay_us(50);
							Motor_PulN;
							delay_us(50);
						}	
////						delay_us(10);	
						if(GPIO_ReadInputDataBit(GPIOE, GATE1_DET) == 0) //��⵽��ʾ�������
						{
							TextColor_Send(TextControl_id[8],Color_Red);
							Data_Deal_Send(2,0X0001,TextControl_id[8]);
						}
						else							//�������  
						{
							TextColor_Send(TextControl_id[8],0xF980);
							Data_Deal_Send(1,0X0001,TextControl_id[8]);	
						}
					//	TestStep.Test_Flag =  0; //�����ɡ�
					//TestStep.Test_Start = 0; //ֹͣ���
						TestStep.Test_Press = Restart_Motor; //����Ϊ0	
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
				
				TestStep.Test_Flag =  0; //�����ɡ�
				TestStep.Test_Start = 0; //ֹͣ���
				TestStep.Test_Press = 0; //����Ϊ0	
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
����������ı������� 
����ı���ɫ
δ��������ֱ�Ӹ�Ϊ0
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
����������Ϸ���������
******************************************************/
void Stored_Data(void)
{
	Data_Deal_Send(Para_Data.Light_Current_Max,0X0001,TextStored_Id[0]);  //ָʾ�Ƶ���
	Data_Deal_Send(Para_Data.Light_Current_Min,0X0001,TextStored_Id[1]);  
	Data_Deal_Send(Para_Data.Power_Current_Max,0X0001,TextStored_Id[2]);  //��ⷧ��������
	Data_Deal_Send(Para_Data.Power_Current_Min,0X0001,TextStored_Id[3]);
	//Data_Deal_Send(Para_Data.Value_Current_Max,0X0001,TextStored_Id[4]);
	//Data_Deal_Send(Para_Data.Value_Current_Min,0X0001,TextStored_Id[5]);
	Data_Deal_Send(Para_Data.Standby_Current_Max,0X0001,TextStored_Id[4]);  //��������
	Data_Deal_Send(Para_Data.Standby_Current_Min,0X0001,TextStored_Id[5]);
	Data_Deal_Send(Para_Data.Power_ADC_Max,0X0001,TextStored_Id[6]);				//��ԴADC
	Data_Deal_Send(Para_Data.Power_ADC_Min,0X0001,TextStored_Id[7]);
	Data_Deal_Send(Para_Data.Shoot_Power_Max,0X0001,TextStored_Id[8]);			//���ⷢ�书��
	Data_Deal_Send(Para_Data.Shoot_Power_Min,0X0001,TextStored_Id[9]);
	Data_Deal_Send(Para_Data.HW_Learn_Max,0X0001,TextStored_Id[10]);				//����ѧϰ���
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
		Data_Deal_Send(Para_Data.Light_Current_Max,0X0003,ReadTextControl_id[0]);  //ָʾ�Ƶ���
	Data_Deal_Send(Para_Data.Light_Current_Min,0X0003,ReadTextControl_id[1]);  
	Data_Deal_Send(Para_Data.Power_Current_Max,0X0003,ReadTextControl_id[2]);  //��ⷧ��������
	Data_Deal_Send(Para_Data.Power_Current_Min,0X0003,ReadTextControl_id[3]);
	//Data_Deal_Send(Para_Data.Value_Current_Max,0X0001,TextStored_Id[4]);
	//Data_Deal_Send(Para_Data.Value_Current_Min,0X0001,TextStored_Id[5]);
	Data_Deal_Send(Para_Data.Standby_Current_Max,0X0003,ReadTextControl_id[4]);  //��������
	Data_Deal_Send(Para_Data.Standby_Current_Min,0X0003,ReadTextControl_id[5]);
	Data_Deal_Send(Para_Data.Power_ADC_Max,0X0003,ReadTextControl_id[6]);				//��ԴADC
	Data_Deal_Send(Para_Data.Power_ADC_Min,0X0003,ReadTextControl_id[7]);
	Data_Deal_Send(Para_Data.Shoot_Power_Max,0X0003,ReadTextControl_id[8]);			//���ⷢ�书��
	Data_Deal_Send(Para_Data.Shoot_Power_Min,0X0003,ReadTextControl_id[9]);
	Data_Deal_Send(Para_Data.HW_Learn_Max,0X0003,ReadTextControl_id[10]);				//����ѧϰ���
	Data_Deal_Send(Para_Data.HW_Learn_Min,0X0003,ReadTextControl_id[11]);

	Data_Deal_Send(Para_Data.OPA0_Value_Max,0X0003,ReadTextControl_id[12]);
	Data_Deal_Send(Para_Data.OPA0_Value_Min,0X0003,ReadTextControl_id[13]);
	Data_Deal_Send(Para_Data.OPA1_Value_Max,0X0003,ReadTextControl_id[14]);
	Data_Deal_Send(Para_Data.OPA1_Value_Min,0X0003,ReadTextControl_id[15]);
	Data_Deal_Send(Para_Data.GanDistance_Max,0X0003,ReadTextControl_id[16]);
	Data_Deal_Send(Para_Data.GanDistance_Min,0X0003,ReadTextControl_id[17]);
}


/******************************************************
Data_Analy:�
���ݽ��� ����
******************************************************/
void Data_Analy(void)
{
	if(DC_Analay.packe == 1)  //�ж���һ��������
	{	
		if(DC_Analay.Data[0] == 0xEE && DC_Analay.Data[1] == 0xB1) // ������ͷ�Ƿ�Ϊ0xee
		{
			
			if(DC_Analay.Data[2]== 0X11) //������������
			{
				switch(DC_Analay.Data[4])//�ж����ĸ�ҳ�����
				{
				  //��������İ�ťѡ��
					case 0x01:
					{
						if(DC_Analay.Data[9] == 0X01)
						{
							switch (DC_Analay.Data[6])
							{
								//������ť
								case 0x34: 
								{
									
								}break;
								//��ֹ��ť
								case 0x36:
								{
									TestStep.Test_Start = 0; //��ʼ����
								}break;
								//�˳�������ť
								case 0x2b:
								{
									TestStep.Test_Start = 0;
									TestStep.Test_Flag = 0;
									Clear_TestSample();
								}break;
								//��¼����ť							
								case 0x37:
								{
										ButtonP.KEYCODE_FLAG = 1;								
								}break;
								//��ʼ���԰�ť
								case 0x30:
								{
									TestStep.Test_Start = 1; //��ʼ����
									Test_Step_Init();
								}
							}
						}
					}break;
					//�������ò���ҳ��
					case 0x03:
					{
						unsigned char TestDatalen = 0;
						
						if(DC_Analay.Data[9] == 0X01)
						{
							switch(DC_Analay.Data[6])
							{
								case 0x0c:  // ���������ť
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
										case 0x06:  //ָʾ��
										{
											Para_Data.Light_Current_Max=RecData_Deal(DC_Analay.Data,TestDatalen);
											ProgressData_Send(15);
										}break;
										case 0x07:
										{
											Para_Data.Light_Current_Min=RecData_Deal(DC_Analay.Data,TestDatalen);
											ProgressData_Send(20);
										}break;
										case 0x0A:  //������·
										{
											Para_Data.Power_Current_Max=RecData_Deal(DC_Analay.Data,TestDatalen);
											ProgressData_Send(22);
										}break;
										case 0x0B:
										{			
											Para_Data.Power_Current_Min=RecData_Deal(DC_Analay.Data,TestDatalen);
											ProgressData_Send(25);
										}break;

										case 0x12: //��������
										{
											Para_Data.Standby_Current_Max=RecData_Deal(DC_Analay.Data,TestDatalen);
											ProgressData_Send(40);
										}break;
										case 0x13:
										{
											Para_Data.Standby_Current_Min=RecData_Deal(DC_Analay.Data,TestDatalen);
											ProgressData_Send(45);
										}break;
										case 0x16:  //��ԴADC
										{
											Para_Data.Power_ADC_Max=RecData_Deal(DC_Analay.Data,TestDatalen);
											ProgressData_Send(50);
										}break;
										case 0x17:
										{
											Para_Data.Power_ADC_Min=RecData_Deal(DC_Analay.Data,TestDatalen);
											ProgressData_Send(55);
										}break;
										case 0x1A: //����ѧϰ���
										{
											Para_Data.HW_Learn_Max=RecData_Deal(DC_Analay.Data,TestDatalen);
											ProgressData_Send(60);
										}break;
										case 0x1B: //����ѧϰ���
										{
											Para_Data.HW_Learn_Min=RecData_Deal(DC_Analay.Data,TestDatalen);
											ProgressData_Send(65);
										}break;
										case 0x1E: //���ⷢ�书��
										{
											Para_Data.Shoot_Power_Max=RecData_Deal(DC_Analay.Data,TestDatalen);
											ProgressData_Send(70);	
										}break;
										case 0x1F:
										{
											Para_Data.Shoot_Power_Min=RecData_Deal(DC_Analay.Data,TestDatalen);
											ProgressData_Send(74);
										}break;
										case 0x03: //�˷�У׼ֵ0
										{
											Para_Data.OPA0_Value_Max = RecData_Deal(DC_Analay.Data,TestDatalen);
											ProgressData_Send(82);
										}break;
										case 0x0D:
										{
											Para_Data.OPA0_Value_Min = RecData_Deal(DC_Analay.Data,TestDatalen);
											ProgressData_Send(85);
										}break;
										
										case 0x0E:  //�˷�У׼ֵ1
										{
											Para_Data.OPA1_Value_Max = RecData_Deal(DC_Analay.Data,TestDatalen);
											ProgressData_Send(89);
										}break;
										case 0x0F:
										{
											Para_Data.OPA1_Value_Min = RecData_Deal(DC_Analay.Data,TestDatalen);
											ProgressData_Send(92);
										}break;
										case 0x21: //��Ӧ����
										{
											Para_Data.GanDistance_Max=RecData_Deal(DC_Analay.Data,TestDatalen);
											ProgressData_Send(97);
										}break;
										case 0x22:
										{
											Para_Data.GanDistance_Min=RecData_Deal(DC_Analay.Data,TestDatalen);
											ProgressData_Send(100);
											TestStep.Test_ReadFlag = 0;
											//�������ݴ��� 
											Storage_Data();
										}break;										
									}
								}
						}
					}break;
				}
			}
			else if(DC_Analay.Data[2] == 0x01)  //�л����沿��
			{
				if(DC_Analay.Data[4] == 0x01) //�л���������ʾ���Ǹ�����
				{
					HintYinCang_Send(); //�������
					Read_Storage_Send();					
				}
				else if(DC_Analay.Data[4] == 0X03)  //�ϴ�����
				{
					Data3_Send();
					ProgressData_Send(0);  //��������0
				}
			}
		} 
		else if (DC_Analay.Data[0] == 0xEE && DC_Analay.Data[1] == 0x0B)  //���ݴ洢
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
	//ָʾ��
	Para_Data.Light_Current_Max = 	buff[2]<<8|buff[3];
	Para_Data.Light_Current_Min = 	buff[4]<<8|buff[5];
	Para_Data.Light_Current_ERRORNum = buff[6]<<8|buff[7];
	//������·ʹ��
	Para_Data.Power_Current_Max	= buff[8]<<8|buff[9];
	Para_Data.Power_Current_Min = buff[10]<<8|buff[11];
	Para_Data.Power_Current_ERRORNum = buff[12]<<8|buff[13];
	//��������
	Para_Data.Standby_Current_Max	= buff[14]<<8|buff[15];
	Para_Data.Standby_Current_Min = buff[16]<<8|buff[17];
	Para_Data.Standby_Current_ERRORNum = buff[18]<<8|buff[19];
	//��ѹADC
	Para_Data.Power_ADC_Max	= buff[20]<<8|buff[21];
	Para_Data.Power_ADC_Min = buff[22]<<8|buff[23];
	Para_Data.Power_ADC_ERRORNum = buff[24]<<8|buff[25];
	//����ѧϰ���
	Para_Data.HW_Learn_Max	= buff[26]<<8|buff[27];
	Para_Data.HW_Learn_Min = buff[28]<<8|buff[29];
	Para_Data.HW_Learn_ERRORNum = buff[30]<<8|buff[31];
	//���ⷢ�书��
	Para_Data.Shoot_Power_Max	= buff[32]<<8|buff[33];
	Para_Data.Shoot_Power_Min = buff[34]<<8|buff[35];
	Para_Data.Shoot_Power_ERRORNum = buff[36]<<8|buff[37];
	//�˷�У׼0
	Para_Data.OPA0_Value_Max	= buff[38]<<8|buff[39];
	Para_Data.OPA0_Value_Min = buff[40]<<8|buff[41];
	Para_Data.OPA0_Value_ERRORNum = buff[42]<<8|buff[43];	
	//�˷�У׼1
	Para_Data.OPA1_Value_Max	= buff[44]<<8|buff[45];
	Para_Data.OPA1_Value_Min = buff[46]<<8|buff[47];
	Para_Data.OPA1_Value_ERRORNum = buff[48]<<8|buff[49];
	//��Ӧ����
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
	//ָʾ��
	StorageBuffer[6] = Para_Data.Light_Current_Max >> 8;
	StorageBuffer[7] = Para_Data.Light_Current_Max&0x00ff;
	StorageBuffer[8] = Para_Data.Light_Current_Min >> 8;
	StorageBuffer[9] = Para_Data.Light_Current_Min&0x00ff;
	StorageBuffer[10] = Para_Data.Light_Current_ERRORNum >> 8;
	StorageBuffer[11] = Para_Data.Light_Current_ERRORNum&0x00ff;
	//������·ʹ��
	StorageBuffer[12] = Para_Data.Power_Current_Max >> 8;
	StorageBuffer[13] = Para_Data.Power_Current_Max&0x00ff;
	StorageBuffer[14] = Para_Data.Power_Current_Min >> 8;
	StorageBuffer[15] = Para_Data.Power_Current_Min&0x00ff;
	StorageBuffer[16] = Para_Data.Power_Current_ERRORNum >> 8;
	StorageBuffer[17] = Para_Data.Power_Current_ERRORNum&0x00ff;
	//��������
	StorageBuffer[18] = Para_Data.Standby_Current_Max >> 8;
	StorageBuffer[19] = Para_Data.Standby_Current_Max&0x00ff;
	StorageBuffer[20] = Para_Data.Standby_Current_Min >> 8;
	StorageBuffer[21] = Para_Data.Standby_Current_Min&0x00ff;
	StorageBuffer[22] = Para_Data.Standby_Current_ERRORNum >> 8;
	StorageBuffer[23] = Para_Data.Standby_Current_ERRORNum&0x00ff;
	//��ѹADC
	StorageBuffer[24] = Para_Data.Power_ADC_Max >> 8;
	StorageBuffer[25] = Para_Data.Power_ADC_Max&0x00ff;
	StorageBuffer[26] = Para_Data.Power_ADC_Min >> 8;
	StorageBuffer[27] = Para_Data.Power_ADC_Min&0x00ff;
	StorageBuffer[28] = Para_Data.Power_ADC_ERRORNum >> 8;
	StorageBuffer[29] = Para_Data.Power_ADC_ERRORNum&0x00ff;
	//����ѧϰ���
	StorageBuffer[30] = Para_Data.HW_Learn_Max >> 8;
	StorageBuffer[31] = Para_Data.HW_Learn_Max&0x00ff;
	StorageBuffer[32] = Para_Data.HW_Learn_Min >> 8;
	StorageBuffer[33] = Para_Data.HW_Learn_Min&0x00ff;
	StorageBuffer[34] = Para_Data.HW_Learn_ERRORNum >> 8;
	StorageBuffer[35] = Para_Data.HW_Learn_ERRORNum&0x00ff;
	//���ⷢ�书��
	StorageBuffer[36] = Para_Data.Shoot_Power_Max >> 8;
	StorageBuffer[37] = Para_Data.Shoot_Power_Max&0x00ff;
	StorageBuffer[38] = Para_Data.Shoot_Power_Min >> 8;
	StorageBuffer[39] = Para_Data.Shoot_Power_Min&0x00ff;
	StorageBuffer[40] = Para_Data.Shoot_Power_ERRORNum >> 8;
	StorageBuffer[41] = Para_Data.Shoot_Power_ERRORNum&0x00ff;
	//�˷�У׼0
	StorageBuffer[42] = Para_Data.OPA0_Value_Max >> 8;
	StorageBuffer[43] = Para_Data.OPA0_Value_Max&0x00ff;
	StorageBuffer[44] = Para_Data.OPA0_Value_Min >> 8;
	StorageBuffer[45] = Para_Data.OPA0_Value_Min&0x00ff;
	StorageBuffer[46] = Para_Data.OPA0_Value_ERRORNum >> 8;
	StorageBuffer[47] = Para_Data.OPA0_Value_ERRORNum&0x00ff;
	//�˷�У׼1
	StorageBuffer[48] = Para_Data.OPA1_Value_Max >> 8;
	StorageBuffer[49] = Para_Data.OPA1_Value_Max&0x00ff;
	StorageBuffer[50] = Para_Data.OPA1_Value_Min >> 8;
	StorageBuffer[51] = Para_Data.OPA1_Value_Min&0x00ff;
	StorageBuffer[52] = Para_Data.OPA1_Value_ERRORNum >> 8;
	StorageBuffer[53] = Para_Data.OPA1_Value_ERRORNum&0x00ff;
	
	//��Ӧ����
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

