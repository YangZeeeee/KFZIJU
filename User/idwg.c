#include "stm32f10x_iwdg.h"
#include "idwg.h"


void idwg_Init(unsigned char perp, unsigned int rlr)
{
	IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);  //IDWG_KR寄存器写入0x5555使能对寄存器IWDG_PR和IWDG_RLR的写保护  进行写操作
	
	IWDG_SetPrescaler(perp);  //设置IWDG预分频值:设置IWDG预分频值为64
	
	IWDG_SetReload(rlr);  //设置IWDG重装载值
	
	IWDG_ReloadCounter();  //按照IWDG重装载寄存器的值重装载IWDG计数器
	
	IWDG_Enable();  //使能IWDG

}

void iwdg_feed(void)
{
	IWDG_ReloadCounter();   //喂狗时寄存器写入0xAAAAA进行重装载
}