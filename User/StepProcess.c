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
#include "string.h"
unsigned char  Start_ScreenId = 0;
unsigned char  TestRead_ScreenID = 3;
unsigned char  TextControl_id[] = {0x08,0x0c,0x14,0x18,0x1C,0x20,0x23,0x27,0x2a};
unsigned char  TextColor_Control[] = {0xEE,0XB1,0x18,0x00,0x01,0x00,0x00,0x00,0x00,0xFF,0xFC,0xFF,0xFF};//文本背景色数据格式。
unsigned char  TextData_Control[] = {0xEE,0xB1,0x10,0x00,0x00,0x00,0xFF,0xFC,0xFF,0xFF};//
unsigned char  TextStored_Id[] = {0x06,0x07,0x0A,0x0B,0x12,0x13,0x16,0x17,0x1A,0x1B\
											,0x1E,0x1F,0x21,0x22,0x24,0x26,0x28,0x29};//页面1文本控件id 大小值。

unsigned char ReadTextControl_id[] = {0x06,0x07,0x0A,0x0B,0x12,0x13,0x16,0x17,0x1A,0x1B\
												,0x1E,0x1F,0x03,0x0d,0x0e,0x0f,0x21,0x22};  //读取参数  页面3的控件id

unsigned char  ProgressRate[] = {0xEE,0xB1,0x10,0x00,0x03,0x00,0x14,0x00,0x00,0x00,0x00,0xFF,0xFC,0xFF,0xFF};

unsigned char HWDIS[] = {0xA5,0x01,0x01,0x01,0xA8};
unsigned char HWREAD[] = {0xA5,0x02,0x01,0x02,0xAA};
unsigned char DGREAD[] = {0xA5,0x02,0x01,0x01,0xA9};
unsigned char METAL[] = {0XA5,0X01,0X01,0X02,0xA9};
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
处理数据格式上发给串口屏 例如 21 处理为 0x32  0x31
Data   	  : 发送的数据
Control_Id : 发送的控件Id
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
		if(Data>9999)// 万
		{
			TestData[Num++] = (Data/10000) + 0x30;
			TestData[Num++] = ((Data/1000)%10)+ 0x30;
			TestData[Num++] = ((Data/100)%10)+ 0x30;
			TestData[Num++] = ((Data/10)%10)+ 0x30;
			TestData[Num++] = (Data%10)+ 0x30;	
		}
		else if(Data<10000 && Data>999) //千
		{
			TestData[Num++] = (Data/1000)+ 0x30;
			TestData[Num++] = ((Data/100)%10)+ 0x30;
			TestData[Num++] = ((Data/10)%10)+ 0x30;
			TestData[Num++] = (Data%10)+ 0x30;	
		}
		else if(Data <1000 && Data>99) //百
		{
			TestData[Num++] = (Data/100)+ 0x30;
			TestData[Num++] = ((Data/10)%10)+ 0x30;
			TestData[Num++] = (Data%10)+ 0x30;
		}
		else if(Data <100 && Data>9) //十
		{
			TestData[Num++] = (Data/10)+ 0x30;
			TestData[Num++] = (Data%10)+ 0x30;
		}
		else if(Data <10) //个
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
设置文本背景色为红色
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
读取参数的指令(读取文本)
每次读取等待1s种 如果重复读取5次后没成功 则默认为读取失败
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
解析收到的数据 发送进度条显示数 
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
解析收到的数据 例如0X31 0x32 0x33  解析为 123
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
读取电流值 
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
				ComP.Com_time = 0;
				ComP.Com_Step = 1;
			}break;
			case 1:
			{
				if((ComP.Com_Num <= Send_Num)&&(ComP.Com_time == 0))
				{
					AMPara_Send();  //读取电流值。
					ComP.Com_Step = 2; 
					ComP.Com_time = 200;
				}
			}break;	
			case 2:
			{
				if(ComP.Com_time == 0)
				{
					AMRxdata_Cache();  //抓取数据。
					ComP.Com_Value = AMRXDATA_Extract();//获取电压值
					*Data = ComP.Com_Value;
					ComP.Com_Step = 3;
					ComP.Com_Flag = 0;  //感应头电流表一个流程读取结束
				}
			}break;
				
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
	
	TestPara.HW_Learn_Flag = 0;
	TestPara.IndicLigh_Current_Flag = 0;
	TestPara.Ganying_Data_Flag = 0;
	
	htRxdata_P.SendNum = 0;
	htRxdata_P.Rx_Error = 0;
}



/******************************************************
Start_Testing:
开始测试的流程并将数据上发到屏幕
测试数据在范围内的正常上发 超出相应范围的数据 背景色赋为红色

①发送数据让感应头进行红外学习。

******************************************************/
void Start_Testing(void)
{
	static unsigned char GanYing_Com = 0;
	if(TestStep.Test_Start == 1)
	{
		TestStep.Test_Flag = 1; //表示开始进入检测
		TextColor_Control[3] = 0x00;
		TextColor_Control[4] = Start_ScreenId;	
		switch (TestStep.Test_Press)
		{			
			case Test_Init:                     //马达归位
			{
				
				unsigned int StepP_Num = 0;
				//iwdg_feed(); //10S 马达归为 如果没有归为则表示检测不到马达感应位
				while((ButtonP.MOTOR_FLAG == 0 && TestStep.Motor_ReFlag == 0) ||(GPIO_ReadInputDataBit(GPIOE, MOTOR_DET) != 1))  
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
				htRxdata_P.WaitTimer = 0;
				TestStep.Test_Press++;
				TestStep.Motor_ReFlag = 0;
				TestStep.Motor_ReTime = 0;
			}break;
			
			case Starting_Up:
			{
				if(htRxdata_P.WaitTimerFlag == 0)  //上电等2S 发送自动学习指令
				{
					Test_Relay_L;
					DataSwitch_Relay_H;			//数据脚连接	
					htRxdata_P.WaitTimerFlag =1;
				}
				else if(htRxdata_P.WaitTimerFlag ==2)
				{
					TestStep.Test_Press++;
					htRxdata_P.WaitTimerFlag = 0;
				}
			}
			
			case HW_Learn:				 //  发送自动学习指令  等待5S 查看是否返回数据
			{				
				if(htRxdata_P.WaitTimerFlag  == 0)
						htRxdata_P.WaitTimerFlag  = 1;
				switch(GanYing_Com)
				{
					case 0:
					{
						htData_Tx(HWDIS,sizeof(HWDIS));
						GanYing_Com++;
					}break;
					case 1:
					{
						if(TestPara.HW_Learn_Flag == 1)
						{
							TestStep.Test_Press = Ganying_Data;	
							htRxdata_P.WaitTimerFlag = 0;
							ComP.Com_Flag = 1;//下一个读取开始
							ComP.Com_Step = 0;
							GanYing_Com = 0;
							TestPara.HW_Learn_Flag = 0;
							htRxdata_P.htData_Deal = 0;
							GPIO_WriteBit(GPIOD,GPIO_Pin_0,Bit_RESET);
						}
						else if(htRxdata_P.WaitTimerFlag == 2)   //自学习失败 .。。。还未修改
						{
							TestStep.Test_Press = Ganying_Data;	
							htRxdata_P.WaitTimerFlag = 0;
							ComP.Com_Flag = 1;//下一个读取开始
							ComP.Com_Step = 0;
							GanYing_Com = 0;
							TestPara.HW_Learn_Flag = 0;
							htRxdata_P.htData_Deal = 0;
						}
					}break;
				}
			}break;	
		
			case Ganying_Data:
			{
				if(htRxdata_P.WaitTimerFlag == 0)
				{
					htData_Tx(DGREAD,sizeof(DGREAD));
					htRxdata_P.WaitTimerFlag = 1;
				}		
				if(TestPara.IndicLigh_Current_Flag == 1)  //指示灯电流读取
				{
					Com_Send(&TestPara.Light_Current_Test);  //200ms自动退出
					if(ComP.Com_Flag ==0)
					{
						ComP.Com_Flag = 1;//下一个读取开始
						ComP.Com_Step = 0;
						TestPara.IndicLigh_Current_Flag = 0;
						htRxdata_P.htData_Deal = 0;
						htRxdata_P.SendNum ++;
					}
				}
				else if(TestPara.Power_Current_Flag == 1)  //驱动电流读取
				{
					Com_Send(&TestPara.Power_Current_Test);  //200ms自动退出
					if(ComP.Com_Flag ==0)
					{
						ComP.Com_Flag = 1;//下一个读取开始
						ComP.Com_Step = 0;
						htRxdata_P.htData_Deal = 0;
						TestPara.Power_Current_Flag = 0;
						htRxdata_P.SendNum ++;
					}					
				}
					
				else if(TestPara.Ganying_Data_Flag==1)   //感应数据读取
				{
					
						TestPara.Ganying_Data_Flag = 0;
						htRxdata_P.htData_Deal = 0;
						ComP.Com_Flag = 1;//下一个读取开始
						ComP.Com_Step = 0;
					//	TestStep.Test_Press = Standby_Current;
						htRxdata_P.WaitTimerFlag = 0;
						htRxdata_P.WaitTimer = 0;
						htRxdata_P.SendNum ++;
						if(htRxdata_P.SendNum ==  3)
						{
							TestStep.Test_Press = Standby_Current;
							htRxdata_P.SendNum = 0;
						}
						else
						{
							TestStep.Test_Press = Ganying_Data;
							htRxdata_P.SendNum = 0;
							htRxdata_P.Rx_Error ++;
							if(htRxdata_P.Rx_Error == 3)
							{
								htRxdata_P.Rx_Error = 0;
								TestStep.Test_Press = Standby_Current;
							}
						}
				}
		
				else if(htRxdata_P.WaitTimerFlag == 2)  //下一个检测
				{
						//TestStep.Test_Press = Standby_Current;
						htRxdata_P.WaitTimerFlag = 0;
						htRxdata_P.WaitTimer = 0;
						ComP.Com_Flag = 1;//下一个读取开始
						ComP.Com_Step = 0;
						htRxdata_P.SendNum ++;
						if(htRxdata_P.SendNum ==  3)
						{
							TestStep.Test_Press = Standby_Current;
							htRxdata_P.SendNum = 0;
						}
						else
						{
							TestStep.Test_Press = Ganying_Data;
							htRxdata_P.SendNum = 0;
							htRxdata_P.Rx_Error ++;
							if(htRxdata_P.Rx_Error == 3)
							{
								htRxdata_P.Rx_Error = 0;
								TestStep.Test_Press = Standby_Current;
							}
						}
				}		
			}break;
			
			case Standby_Current:
			{
				if(htRxdata_P.WaitTimerFlag == 0)
						htRxdata_P.WaitTimerFlag = 1;
				if(htRxdata_P.WaitTimerFlag == 2)
						Com_Send(&TestPara.Standby_Current_Test);  //200ms自动退出
				if(ComP.Com_Flag == 0)
				{
					TestStep.Test_Press = Display_Send;
					htRxdata_P.WaitTimerFlag = 0;
					htRxdata_P.WaitTimer = 0;
				}
			}break;
		
			case Display_Send:
			{
					//指示灯电流
					if((Para_Data.Light_Current_Max <=TestPara.Light_Current_Test)||(Para_Data.Light_Current_Min>=TestPara.Light_Current_Test))
					{					
						TextColor_Send(TextControl_id[0],Color_Red);
						Data_Deal_Send(TestPara.Light_Current_Test,0X0001,TextControl_id[0]);
					}		
					else
					{
						TextColor_Send(TextControl_id[0],0x07E0);
						Data_Deal_Send(TestPara.Light_Current_Test,0X0001,TextControl_id[0]);		
					}
					//驱动电路使能数据发送 = 上电电流
				  if((Para_Data.Power_Current_Max <= TestPara.Power_Current_Test)||(Para_Data.Power_Current_Min>=TestPara.Power_Current_Test))
					{
						TextColor_Send(TextControl_id[1],Color_Red);
						Data_Deal_Send(TestPara.Power_Current_Test,0X0001,TextControl_id[1]);
					}	
					else
					{
						TextColor_Send(TextControl_id[1],0x07E0);
						Data_Deal_Send(TestPara.Power_Current_Test,0X0001,TextControl_id[1]);		
					}
					//待机电流
					if((Para_Data.Standby_Current_Max <= TestPara.Standby_Current_Test)||(Para_Data.Standby_Current_Min>=TestPara.Standby_Current_Test))
					{
						TextColor_Send(TextControl_id[2],Color_Red);
						Data_Deal_Send(TestPara.Standby_Current_Test,0X0001,TextControl_id[2]);
					}
					else
					{
						TextColor_Send(TextControl_id[2],0x07E0);
						Data_Deal_Send(TestPara.Standby_Current_Test,0X0001,TextControl_id[2]);		
					}	
					// 电压ADC
					if((Para_Data.Power_ADC_Max < TestPara.Power_ADC_Test)||(Para_Data.Power_ADC_Min>TestPara.Power_ADC_Test))
					{
						TextColor_Send(TextControl_id[3],Color_Red);
						Data_Deal_Send(TestPara.Power_ADC_Test,0X0001,TextControl_id[3]);
					}
					else
					{
						TextColor_Send(TextControl_id[3],0x07E0);
						Data_Deal_Send(TestPara.Power_ADC_Test,0X0001,TextControl_id[3]);		
					}	
					//红外学习结果
					if((Para_Data.HW_Learn_Max <= TestPara.Learn_Test)||(Para_Data.HW_Learn_Min>=TestPara.Learn_Test))
					{
						TextColor_Send(TextControl_id[5],Color_Red);
						Data_Deal_Send(TestPara.Learn_Test,0X0001,TextControl_id[5]);
					}
					else
					{
						TextColor_Send(TextControl_id[5],0x07E0);
						Data_Deal_Send(TestPara.Learn_Test,0X0001,TextControl_id[5]);		
					}	
					//红外发射功率
					if((Para_Data.Shoot_Power_Max <TestPara.Shoot_Power_Test)||(Para_Data.Shoot_Power_Min>TestPara.Shoot_Power_Test))
					{
						TextColor_Send(TextControl_id[4],Color_Red);
						Data_Deal_Send(TestPara.Shoot_Power_Test,0X0001,TextControl_id[4]);
					}
					else
					{
						TextColor_Send(TextControl_id[4],0x07E0);
						Data_Deal_Send(TestPara.Shoot_Power_Test,0X0001,TextControl_id[4]);		
					}
					//运放校准0
					if((Para_Data.OPA0_Value_Max <TestPara.OPA0_Test)||(Para_Data.OPA0_Value_Min>TestPara.OPA0_Test))
					{
						TextColor_Send(TextControl_id[6],Color_Red);
						Data_Deal_Send(TestPara.OPA0_Test,0X0001,TextControl_id[6]);	
					}
					else
					{
						TextColor_Send(TextControl_id[6],0x07E0);
						Data_Deal_Send(TestPara.OPA0_Test,0X0001,TextControl_id[6]);	
					}
					//运放校准1
					if((Para_Data.OPA1_Value_Max < TestPara.OPA1_Test)||(Para_Data.OPA1_Value_Min)>TestPara.OPA1_Test)
					{
						TextColor_Send(TextControl_id[7],Color_Red);
						Data_Deal_Send(TestPara.OPA1_Test,0X0001,TextControl_id[7]);
					}
					else
					{
						TextColor_Send(TextControl_id[7],0x07E0);
						Data_Deal_Send(TestPara.OPA1_Test,0X0001,TextControl_id[7]);	
					}	
					TestStep.Motor_ReFlag = 0;
					TestStep.Motor_ReTime = 0;
					TestPara.Power_ADC_Test = 0;
					TestPara.Shoot_Power_Test =  0;
					TestPara.Learn_Test =  0;
					TestPara.OPA0_Test =0;
					TestPara.OPA1_Test = 0;
					TestStep.Test_Press = Ganyin_Distance;			
			}break;
			
			case Ganyin_Distance:  //感应距离
			{	
				unsigned int StepN_Num=0;
				unsigned char GATE_Flag=0;
				if(htRxdata_P.WaitTimerFlag == 0)
				{
						htRxdata_P.WaitTimerFlag = 1;
					htData_Tx(METAL,sizeof(METAL));					
				}
				if(htRxdata_P.WaitTimerFlag == 2)
				{
					switch(TestStep.Motor_Step)
					{
						case 0:
						{
							Motor_DirP;
							for(StepN_Num=0;StepN_Num<10270;StepN_Num++) // 790可更改为具体的参数值(从顶部到距离为6)
							{
								Motor_PulP;
								delay_us(50);
								Motor_PulN;
								delay_us(50);
								if(GPIO_ReadInputDataBit(GPIOE, GATE2_DET) == 0) //检测到 则开始检测上线值
								{
										GATE_Flag = 1;
										TestStep.Motor_Step = 1;	
								}
							}
							if(GATE_Flag== 0)					//跳出检测提示为低
							{
								TestStep.Test_Press =Restart_Motor;
								TextColor_Send(TextControl_id[8],Color_Red);
								Data_Deal_Send(0,0X0001,TextControl_id[8]);
							}
						}break;
						case 1:
						{
							Motor_DirN;
							for(StepN_Num=0;StepN_Num<1580;StepN_Num++) // 790可更改为具体的参数值(从距离为6->距离为8)
							{
								Motor_PulP;
								delay_us(50);
								Motor_PulN;
								delay_us(50);
									if(GPIO_ReadInputDataBit(GPIOE, GATE1_DET) == 0) //检测到 则开始检测上线值
									{
										GATE_Flag = 2;
									}
							}	
							if(GATE_Flag == 2) // 正常范围
							{
								TextColor_Send(TextControl_id[8],0x07E0 );
								Data_Deal_Send(1,0X0001,TextControl_id[8]);
							}
							else							//检测到提示距离过长
							{
								TextColor_Send(TextControl_id[8],0xFFE0); 
								Data_Deal_Send(2,0X0001,TextControl_id[8]);	
							}
							TestStep.Test_Press = Restart_Motor; //流程为0	
						}break;
					}
			}
			}break;	
			
			case Restart_Motor:
			{
				unsigned int Step_Num = 0;
				//(ButtonP.MOTOR_FLAG == 0) &&(TestStep.Motor_ReFlag == 0)
				while(GPIO_ReadInputDataBit(GPIOE, MOTOR_DET) != 1)
				{
					Motor_DirN;
					for(Step_Num=0;Step_Num<790;Step_Num++)
					{
						Motor_PulP;
						delay_us(50);
						Motor_PulN;
						delay_us(50);
						if(GPIO_ReadInputDataBit(GPIOE, MOTOR_DET) == 1)
							break;
					}	
				}
				Test_Relay_H;
				ButtonP.MOTOR_FLAG = 0;		
				TestStep.Test_Flag =  0; //检测完成。
				TestStep.Test_Start = 0; //停止检测
				TestStep.Test_Press = 0; //流程为0				
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
清除测试列文本的数据 
清楚文本颜色
未检测的数据直接赋为0
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
缓存的数据上发到串口屏
******************************************************/
void Stored_Data(void)
{
	Data_Deal_Send(Para_Data.Light_Current_Max,0X0001,TextStored_Id[0]);  //指示灯电流
	Data_Deal_Send(Para_Data.Light_Current_Min,0X0001,TextStored_Id[1]);  
	Data_Deal_Send(Para_Data.Power_Current_Max,0X0001,TextStored_Id[2]);  //检测阀驱动电流
	Data_Deal_Send(Para_Data.Power_Current_Min,0X0001,TextStored_Id[3]);
	//Data_Deal_Send(Para_Data.Value_Current_Max,0X0001,TextStored_Id[4]);
	//Data_Deal_Send(Para_Data.Value_Current_Min,0X0001,TextStored_Id[5]);
	Data_Deal_Send(Para_Data.Standby_Current_Max,0X0001,TextStored_Id[4]);  //待机电流
	Data_Deal_Send(Para_Data.Standby_Current_Min,0X0001,TextStored_Id[5]);
	Data_Deal_Send(Para_Data.Power_ADC_Max,0X0001,TextStored_Id[6]);				//电源ADC
	Data_Deal_Send(Para_Data.Power_ADC_Min,0X0001,TextStored_Id[7]);
	Data_Deal_Send(Para_Data.Shoot_Power_Max,0X0001,TextStored_Id[8]);			//红外发射功率
	Data_Deal_Send(Para_Data.Shoot_Power_Min,0X0001,TextStored_Id[9]);
	Data_Deal_Send(Para_Data.HW_Learn_Max,0X0001,TextStored_Id[10]);				//红外学习结果
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
	Data_Deal_Send(Para_Data.Light_Current_Max,0X0003,ReadTextControl_id[0]);  //指示灯电流
	Data_Deal_Send(Para_Data.Light_Current_Min,0X0003,ReadTextControl_id[1]);  
	Data_Deal_Send(Para_Data.Power_Current_Max,0X0003,ReadTextControl_id[2]);  //检测阀驱动电流
	Data_Deal_Send(Para_Data.Power_Current_Min,0X0003,ReadTextControl_id[3]);
	//Data_Deal_Send(Para_Data.Value_Current_Max,0X0001,TextStored_Id[4]);
	//Data_Deal_Send(Para_Data.Value_Current_Min,0X0001,TextStored_Id[5]);
	Data_Deal_Send(Para_Data.Standby_Current_Max,0X0003,ReadTextControl_id[4]);  //待机电流
	Data_Deal_Send(Para_Data.Standby_Current_Min,0X0003,ReadTextControl_id[5]);
	Data_Deal_Send(Para_Data.Power_ADC_Max,0X0003,ReadTextControl_id[6]);				//电源ADC
	Data_Deal_Send(Para_Data.Power_ADC_Min,0X0003,ReadTextControl_id[7]);
	Data_Deal_Send(Para_Data.HW_Learn_Max,0X0003,ReadTextControl_id[8]);			//红外发射功率
	Data_Deal_Send(Para_Data.HW_Learn_Min,0X0003,ReadTextControl_id[9]);
	Data_Deal_Send(Para_Data.Shoot_Power_Max ,0X0003,ReadTextControl_id[10]);				//红外学习结果
	Data_Deal_Send(Para_Data.Shoot_Power_Min ,0X0003,ReadTextControl_id[11]);

	Data_Deal_Send(Para_Data.OPA0_Value_Max,0X0003,ReadTextControl_id[12]);
	Data_Deal_Send(Para_Data.OPA0_Value_Min,0X0003,ReadTextControl_id[13]);
	Data_Deal_Send(Para_Data.OPA1_Value_Max,0X0003,ReadTextControl_id[14]);
	Data_Deal_Send(Para_Data.OPA1_Value_Min,0X0003,ReadTextControl_id[15]);
	Data_Deal_Send(Para_Data.GanDistance_Max,0X0003,ReadTextControl_id[16]);
	Data_Deal_Send(Para_Data.GanDistance_Min,0X0003,ReadTextControl_id[17]);
}


/******************************************************
Data_Analy:?
数据解析 解析
******************************************************/
void Data_Analy(void)
{
	if(DC_Analay.packe == 1)  //判断完一包数据了
	{	
		if(DC_Analay.Data[0] == 0xEE && DC_Analay.Data[1] == 0xB1) // 解析包头是否为0xee
		{
			
			if(DC_Analay.Data[2]== 0X11) //按键操作部分
			{
				switch(DC_Analay.Data[4])//判断再哪个页面操作
				{
				  //参数界面的按钮选项
					case 0x01:
					{
						if(DC_Analay.Data[9] == 0X01)
						{
							switch (DC_Analay.Data[6])
							{
								//跳过按钮
								case 0x34: 
								{
									
								}break;
								//终止按钮
								case 0x36:
								{
									TestStep.Test_Start = 0; //开始测试
								}break;
								//退出函数按钮
								case 0x2b:
								{
									TestStep.Test_Start = 0;
									TestStep.Test_Flag = 0;
									Clear_TestSample();
								}break;
								//烧录程序按钮							
								case 0x37:
								{
										ButtonP.KEYCODE_FLAG = 1;								
								}break;
								//开始测试按钮
								case 0x30:
								{
									TestStep.Test_Start = 1; //开始测试
									Test_Step_Init();
								}
							}
						}
					}break;
					//重新设置参数页面
					case 0x03:
					{
						unsigned char TestDatalen = 0;
						
						if(DC_Analay.Data[9] == 0X01)
						{
							switch(DC_Analay.Data[6])
							{
								case 0x0c:  // 保存参数按钮
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
										case 0x06:  //指示灯
										{
											Para_Data.Light_Current_Max=RecData_Deal(DC_Analay.Data,TestDatalen);
											ProgressData_Send(15);
										}break;
										case 0x07:
										{
											Para_Data.Light_Current_Min=RecData_Deal(DC_Analay.Data,TestDatalen);
											ProgressData_Send(20);
										}break;
										case 0x0A:  //驱动电路
										{
											Para_Data.Power_Current_Max=RecData_Deal(DC_Analay.Data,TestDatalen);
											ProgressData_Send(22);
										}break;
										case 0x0B:
										{			
											Para_Data.Power_Current_Min=RecData_Deal(DC_Analay.Data,TestDatalen);
											ProgressData_Send(25);
										}break;

										case 0x12: //待机电流
										{
											Para_Data.Standby_Current_Max=RecData_Deal(DC_Analay.Data,TestDatalen);
											ProgressData_Send(40);
										}break;
										case 0x13:
										{
											Para_Data.Standby_Current_Min=RecData_Deal(DC_Analay.Data,TestDatalen);
											ProgressData_Send(45);
										}break;
										case 0x16:  //电源ADC
										{
											Para_Data.Power_ADC_Max=RecData_Deal(DC_Analay.Data,TestDatalen);
											ProgressData_Send(50);
										}break;
										case 0x17:
										{
											Para_Data.Power_ADC_Min=RecData_Deal(DC_Analay.Data,TestDatalen);
											ProgressData_Send(55);
										}break;
										case 0x1A: //红外学习结果
										{
											Para_Data.HW_Learn_Max=RecData_Deal(DC_Analay.Data,TestDatalen);
											ProgressData_Send(60);
										}break;
										case 0x1B: //红外学习结果
										{
											Para_Data.HW_Learn_Min =RecData_Deal(DC_Analay.Data,TestDatalen);
											ProgressData_Send(65);
										}break;
										case 0x1E: //红外发射功率
										{
											Para_Data.Shoot_Power_Max =RecData_Deal(DC_Analay.Data,TestDatalen);
											ProgressData_Send(70);	
										}break;
										case 0x1F:
										{
											Para_Data.Shoot_Power_Min =RecData_Deal(DC_Analay.Data,TestDatalen);
											ProgressData_Send(74);
										}break;
										case 0x03: //运放校准值0
										{
											Para_Data.OPA0_Value_Max = RecData_Deal(DC_Analay.Data,TestDatalen);
											ProgressData_Send(82);
										}break;
										case 0x0D:
										{
											Para_Data.OPA0_Value_Min = RecData_Deal(DC_Analay.Data,TestDatalen);
											ProgressData_Send(85);
										}break;
										
										case 0x0E:  //运放校准值1
										{
											Para_Data.OPA1_Value_Max = RecData_Deal(DC_Analay.Data,TestDatalen);
											ProgressData_Send(89);
										}break;
										case 0x0F:
										{
											Para_Data.OPA1_Value_Min = RecData_Deal(DC_Analay.Data,TestDatalen);
											ProgressData_Send(92);
										}break;
										case 0x21: //感应距离
										{
											Para_Data.GanDistance_Max=RecData_Deal(DC_Analay.Data,TestDatalen);
											ProgressData_Send(97);
										}break;
										case 0x22:
										{
											Para_Data.GanDistance_Min=RecData_Deal(DC_Analay.Data,TestDatalen);
											ProgressData_Send(100);
											TestStep.Test_ReadFlag = 0;
											//发送数据储存 
											Storage_Data();
										}break;										
									}
								}
						}
					}break;
				}
			}
			else if(DC_Analay.Data[2] == 0x01)  //切换画面部分
			{
				if(DC_Analay.Data[4] == 0x01) //切换到参数显示的那个画面
				{
					HintYinCang_Send(); //隐藏组件
					Read_Storage_Send();					
				}
				else if(DC_Analay.Data[4] == 0X03)  //上传数据
				{
					Data3_Send();
					ProgressData_Send(0);  //进度条清0
				}
			}
		} 
		else if (DC_Analay.Data[0] == 0xEE && DC_Analay.Data[1] == 0x0B)  //数据存储
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
	//指示灯
	Para_Data.Light_Current_Max = 	buff[2]<<8|buff[3];
	Para_Data.Light_Current_Min = 	buff[4]<<8|buff[5];
	Para_Data.Light_Current_ERRORNum = buff[6]<<8|buff[7];
	//驱动电路使能
	Para_Data.Power_Current_Max	= buff[8]<<8|buff[9];
	Para_Data.Power_Current_Min = buff[10]<<8|buff[11];
	Para_Data.Power_Current_ERRORNum = buff[12]<<8|buff[13];
	//待机电流
	Para_Data.Standby_Current_Max	= buff[14]<<8|buff[15];
	Para_Data.Standby_Current_Min = buff[16]<<8|buff[17];
	Para_Data.Standby_Current_ERRORNum = buff[18]<<8|buff[19];
	//电压ADC
	Para_Data.Power_ADC_Max	= buff[20]<<8|buff[21];
	Para_Data.Power_ADC_Min = buff[22]<<8|buff[23];
	Para_Data.Power_ADC_ERRORNum = buff[24]<<8|buff[25];
	//红外发射功率	
	Para_Data.Shoot_Power_Max = buff[26]<<8|buff[27];
	Para_Data.Shoot_Power_Min = buff[28]<<8|buff[29];
	Para_Data.Shoot_Power_ERRORNum  = buff[30]<<8|buff[31];
	//红外学习结果
	Para_Data.HW_Learn_Max = buff[32]<<8|buff[33];
	Para_Data.HW_Learn_Min = buff[34]<<8|buff[35];
	Para_Data.HW_Learn_ERRORNum = buff[36]<<8|buff[37];
	//运放校准0
	Para_Data.OPA0_Value_Max	= buff[38]<<8|buff[39];
	Para_Data.OPA0_Value_Min = buff[40]<<8|buff[41];
	Para_Data.OPA0_Value_ERRORNum = buff[42]<<8|buff[43];	
	//运放校准1
	Para_Data.OPA1_Value_Max	= buff[44]<<8|buff[45];
	Para_Data.OPA1_Value_Min = buff[46]<<8|buff[47];
	Para_Data.OPA1_Value_ERRORNum = buff[48]<<8|buff[49];
	//感应距离
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
	//指示灯
	StorageBuffer[6] = Para_Data.Light_Current_Max >> 8;
	StorageBuffer[7] = Para_Data.Light_Current_Max&0x00ff;
	StorageBuffer[8] = Para_Data.Light_Current_Min >> 8;
	StorageBuffer[9] = Para_Data.Light_Current_Min&0x00ff;
	StorageBuffer[10] = Para_Data.Light_Current_ERRORNum >> 8;
	StorageBuffer[11] = Para_Data.Light_Current_ERRORNum&0x00ff;
	//驱动电路使能
	StorageBuffer[12] = Para_Data.Power_Current_Max >> 8;
	StorageBuffer[13] = Para_Data.Power_Current_Max&0x00ff;
	StorageBuffer[14] = Para_Data.Power_Current_Min >> 8;
	StorageBuffer[15] = Para_Data.Power_Current_Min&0x00ff;
	StorageBuffer[16] = Para_Data.Power_Current_ERRORNum >> 8;
	StorageBuffer[17] = Para_Data.Power_Current_ERRORNum&0x00ff;
	//待机电流
	StorageBuffer[18] = Para_Data.Standby_Current_Max >> 8;
	StorageBuffer[19] = Para_Data.Standby_Current_Max&0x00ff;
	StorageBuffer[20] = Para_Data.Standby_Current_Min >> 8;
	StorageBuffer[21] = Para_Data.Standby_Current_Min&0x00ff;
	StorageBuffer[22] = Para_Data.Standby_Current_ERRORNum >> 8;
	StorageBuffer[23] = Para_Data.Standby_Current_ERRORNum&0x00ff;
	//电压ADC
	StorageBuffer[24] = Para_Data.Power_ADC_Max >> 8;
	StorageBuffer[25] = Para_Data.Power_ADC_Max&0x00ff;
	StorageBuffer[26] = Para_Data.Power_ADC_Min >> 8;
	StorageBuffer[27] = Para_Data.Power_ADC_Min&0x00ff;
	StorageBuffer[28] = Para_Data.Power_ADC_ERRORNum >> 8;
	StorageBuffer[29] = Para_Data.Power_ADC_ERRORNum&0x00ff;

		//红外发射功率
	StorageBuffer[30] = Para_Data.Shoot_Power_Max >> 8;
	StorageBuffer[31] = Para_Data.Shoot_Power_Max &0x00ff;
	StorageBuffer[32] = Para_Data.Shoot_Power_Min >> 8;
	StorageBuffer[33] = Para_Data.Shoot_Power_Min &0x00ff;
	StorageBuffer[34] = Para_Data.Shoot_Power_ERRORNum  >> 8;
	StorageBuffer[35] = Para_Data.Shoot_Power_ERRORNum &0x00ff;
	//红外学习结果
	StorageBuffer[36] = Para_Data.HW_Learn_Max >> 8;
	StorageBuffer[37] = Para_Data.HW_Learn_Max&0x00ff;
	StorageBuffer[38] = Para_Data.HW_Learn_Min >> 8;
	StorageBuffer[39] = Para_Data.HW_Learn_Min&0x00ff;
	StorageBuffer[40] = Para_Data.HW_Learn_ERRORNum>> 8;
	StorageBuffer[41] = Para_Data.HW_Learn_ERRORNum&0x00ff;
	//运放校准0
	StorageBuffer[42] = Para_Data.OPA0_Value_Max >> 8;
	StorageBuffer[43] = Para_Data.OPA0_Value_Max&0x00ff;
	StorageBuffer[44] = Para_Data.OPA0_Value_Min >> 8;
	StorageBuffer[45] = Para_Data.OPA0_Value_Min&0x00ff;
	StorageBuffer[46] = Para_Data.OPA0_Value_ERRORNum >> 8;
	StorageBuffer[47] = Para_Data.OPA0_Value_ERRORNum&0x00ff;
	//运放校准1
	StorageBuffer[48] = Para_Data.OPA1_Value_Max >> 8;
	StorageBuffer[49] = Para_Data.OPA1_Value_Max&0x00ff;
	StorageBuffer[50] = Para_Data.OPA1_Value_Min >> 8;
	StorageBuffer[51] = Para_Data.OPA1_Value_Min&0x00ff;
	StorageBuffer[52] = Para_Data.OPA1_Value_ERRORNum >> 8;
	StorageBuffer[53] = Para_Data.OPA1_Value_ERRORNum&0x00ff;
	
	//感应距离
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

