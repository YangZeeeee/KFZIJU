#include "LoadCode.h"
#include "Button.h"
#include "DCTFT.h"
#include "Delay.h"

struct LoadCodeS LoadCodeP;
unsigned  char DisplaySuccess[]= {0xEE,0xB1,0X03,0x00,0x01,0x00,0x03,0x01,0xff,0xfc,0xff,0xff};
unsigned  char DisplayFault[]= {0xEE,0xB1,0X03,0x00,0x01,0x00,0x04,0x01,0xff,0xfc,0xff,0xff};
unsigned  char YinCangSuccess[]= {0xEE,0xB1,0X03,0x00,0x01,0x00,0x03,0x00,0xff,0xfc,0xff,0xff};
unsigned  char YinCangFault[]= {0xEE,0xB1,0X03,0x00,0x01,0x00,0x04,0x00,0xff,0xfc,0xff,0xff};


void LoadCodeP_Init(void)
{
	LoadCodeP.LoadCodeResult = 0;
	LoadCodeP.LoadCodeTime = 0;
	LoadCodeP.LoadCodeTime_R = 0;
//	LoadCodeP.LoadCodeStep = 0;
	LoadCodeP.HintDisPlayFLag = 0;
	LoadCodeP.HintDisPlayTime = 0;
}


void LoadCode_Process(void)
{
	if(ButtonP.KEYCODE_FLAG == 1)
	{
		ButtonP.LoadCode_Flag = 1;
		ButtonP.KEYCODE_FLAG = 0;
		ButtonP.LoadCode_Timer  = 0;
		LoadCodeP.LoadCodeStep = 0;	 
	}
	if(ButtonP.LoadCode_Flag == 1)
	{
		switch(LoadCodeP.LoadCodeStep)
		{			
			case 0:  //继电器切换
			{
					DataSwitch_Relay_L;			//数据脚连接				
					Test_Relay_H;						//5V断开
					Load_Relay_L;						//
					Data_EXTG_Relay_L;			//短接
					LoadCodeP.LoadCodeStep = 3;	
			}break;				
			case 1:  //等待烧录结果
			{
					if(Read_EOP == 0)
					{
						if(Read_BIN1 == 0)
						{
							LoadCodeP.LoadCodeResult = 1;   //烧录成功
							LoadCodeP.LoadCodeStep = 2;										
						}
					}	
			}break;	
			case 2:
			{
					DataSwitch_Relay_H;			//数据脚连接
					Test_Relay_H;						//5V断开
					Load_Relay_H;						//
					LoadCodeP.LoadCodeStep = 3;	
					ButtonP.LoadCode_Timer  = 0;
					ButtonP.LoadCode_Flag=0;
			}break;
		}
	}

}

void HintDisplay_Send(void)
{
	if(	LoadCodeP.LoadCodeResult == 1)
	{
		DCData_Tx(DisplaySuccess,sizeof(DisplaySuccess));
		LoadCodeP.LoadCodeResult = 0;
		LoadCodeP.HintDisPlayFLag = 1;
	}
		
	else if(LoadCodeP.LoadCodeResult == 2)
	{
		DCData_Tx(DisplayFault,sizeof(DisplayFault));
		LoadCodeP.LoadCodeResult = 0;
		LoadCodeP.HintDisPlayFLag = 1;
	}
}

void HintYinCang_Send(void)
{
	DCData_Tx(YinCangSuccess,sizeof(YinCangSuccess));
	DCData_Tx(YinCangFault,sizeof(YinCangFault));
}
