#ifndef _Button_H
#define _Button_H
#include "stm32f10x_gpio.h"

#define KEY_START_PIN  				GPIO_Pin_3
#define KEY_CODELOAD_PIN			GPIO_Pin_2
#define MOTOR_DET						GPIO_Pin_2
#define GATE1_DET						GPIO_Pin_3
#define GATE2_DET						GPIO_Pin_4
#define CAP_SWTICH					GPIO_Pin_11


#define Test_Relay_H									GPIO_WriteBit(GPIOE,GPIO_Pin_5,Bit_SET)
#define Test_Relay_L									GPIO_WriteBit(GPIOE,GPIO_Pin_5,Bit_RESET)

#define Load_Relay_H									GPIO_WriteBit(GPIOE,GPIO_Pin_1,Bit_SET)
#define Load_Relay_L									GPIO_WriteBit(GPIOE,GPIO_Pin_1,Bit_RESET)

#define DataSwitch_Relay_H						GPIO_WriteBit(GPIOC,GPIO_Pin_2,Bit_SET)
#define DataSwitch_Relay_L						GPIO_WriteBit(GPIOC,GPIO_Pin_2,Bit_RESET)

#define Data_EXTG_Relay_H							GPIO_WriteBit(GPIOE,GPIO_Pin_11,Bit_SET)
#define Data_EXTG_Relay_L							GPIO_WriteBit(GPIOE,GPIO_Pin_11,Bit_RESET)

#define Read_BIN1											GPIO_ReadInputDataBit(GPIOG,GPIO_Pin_1)						
#define Read_EOP											GPIO_ReadInputDataBit(GPIOG,GPIO_Pin_0)

#define DCMotorZ_Relay_H							GPIO_WriteBit(GPIOF,GPIO_Pin_15,Bit_SET) 
#define DCMotorZ_Relay_L							GPIO_WriteBit(GPIOF,GPIO_Pin_15,Bit_RESET)

#define DCMotorF_Relay_H							GPIO_WriteBit(GPIOF,GPIO_Pin_13,Bit_SET)
#define DCMotorF_Relay_L							GPIO_WriteBit(GPIOF,GPIO_Pin_13,Bit_RESET)


#define CAP_SWITCH_H									GPIO_WriteBit(GPIOC,GPIO_Pin_11,Bit_SET)
#define CAP_SWITCH_L									GPIO_WriteBit(GPIOC,GPIO_Pin_11,Bit_RESET)
struct button
{
	unsigned char KEYSTART_FLAG;
	unsigned int  KEYSTART_TIMER;

	unsigned char KEYCODE_FLAG;
	unsigned char  KEYCODE_TIMER;
	unsigned int   LoadCode_Timer;
	unsigned char  LoadCode_Flag;

	unsigned char MOTOR_FLAG;
	unsigned int  MOTOR_TIMER;

	unsigned char GATE1_FLAG;
	unsigned int  GATE1_TIMER;

	unsigned char GATE2_FLAG;
	unsigned int  GATE2_TIMER;
	
	unsigned char KEY1_FLAG;
	unsigned int  KEY1_TIMER;
	
	unsigned char KEY0_FLAG;
	unsigned int  KEY0_TIMER;

};
extern struct button ButtonP;




void 	RelayGPIO_Init(void);
void  Button_Scan(void);
void 	Button_HardInit(void);
void 	Motor_Test(void);

#endif
