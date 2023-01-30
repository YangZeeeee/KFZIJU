#include "Delay.h"
#include "stm32f10x.h"

static unsigned char  fac_us=0; //us
static unsigned int  fac_ms=0; //ms
 
 
void delay_init(void)
{
    SysTick_CLKSourceConfig(SysTick_CLKSource_HCLK_Div8);    //ѡ���ⲿʱ�� HCLK/8
    fac_us = SystemCoreClock/8000000;                        //Ϊʱ�ӵ�1/8
    fac_ms = (u16)fac_us*1000;                               //ÿһ��ms��Ҫsystickʱ����
}
 
void delay_us(unsigned long nus)
{
    u32 temp;
    SysTick->LOAD = nus*fac_us;                 //ʱ�����
    SysTick->VAL = 0x00;                        //��ռ�����
    SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk ;  //��ʼ����
    do
    {
        temp = SysTick->CTRL;
    }while((temp&0x01)&&!(temp&(1<<16)));       //�ȴ�ʱ�䵽��
    SysTick->CTRL &= ~SysTick_CTRL_ENABLE_Msk;  //�رռ�����
    SysTick->VAL = 0x00;                        //���
}
 
void delay_ms(unsigned int nms)
{
    u32 temp;
    SysTick->LOAD = (u32)nms*fac_ms;            ////ʱ�����
    SysTick->VAL = 0x00;                        //��ռ�����
    SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk ;  //��ʼ����
    do
    {
        temp=SysTick->CTRL;
    }while((temp&0x01)&&!(temp&(1<<16)));       //�ȴ�ʱ�䵽��
    SysTick->CTRL &= ~SysTick_CTRL_ENABLE_Msk;  //�رռ�����
    SysTick->VAL = 0x00;                        //���
}
