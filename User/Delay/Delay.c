#include "Delay.h"
#include "stm32f10x.h"

static unsigned char  fac_us=0; //us
static unsigned int  fac_ms=0; //ms
 
 
void delay_init(void)
{
    SysTick_CLKSourceConfig(SysTick_CLKSource_HCLK_Div8);    //选择外部时钟 HCLK/8
    fac_us = SystemCoreClock/8000000;                        //为时钟的1/8
    fac_ms = (u16)fac_us*1000;                               //每一个ms需要systick时间数
}
 
void delay_us(unsigned long nus)
{
    u32 temp;
    SysTick->LOAD = nus*fac_us;                 //时间加载
    SysTick->VAL = 0x00;                        //清空计数器
    SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk ;  //开始倒数
    do
    {
        temp = SysTick->CTRL;
    }while((temp&0x01)&&!(temp&(1<<16)));       //等待时间到达
    SysTick->CTRL &= ~SysTick_CTRL_ENABLE_Msk;  //关闭计数器
    SysTick->VAL = 0x00;                        //清空
}
 
void delay_ms(unsigned int nms)
{
    u32 temp;
    SysTick->LOAD = (u32)nms*fac_ms;            ////时间加载
    SysTick->VAL = 0x00;                        //清空计数器
    SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk ;  //开始倒数
    do
    {
        temp=SysTick->CTRL;
    }while((temp&0x01)&&!(temp&(1<<16)));       //等待时间到达
    SysTick->CTRL &= ~SysTick_CTRL_ENABLE_Msk;  //关闭计数器
    SysTick->VAL = 0x00;                        //清空
}
