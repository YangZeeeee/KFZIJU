#include "stm32f10x_iwdg.h"
#include "idwg.h"


void idwg_Init(unsigned char perp, unsigned int rlr)
{
	IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);  //IDWG_KR�Ĵ���д��0x5555ʹ�ܶԼĴ���IWDG_PR��IWDG_RLR��д����  ����д����
	
	IWDG_SetPrescaler(perp);  //����IWDGԤ��Ƶֵ:����IWDGԤ��ƵֵΪ64
	
	IWDG_SetReload(rlr);  //����IWDG��װ��ֵ
	
	IWDG_ReloadCounter();  //����IWDG��װ�ؼĴ�����ֵ��װ��IWDG������
	
	IWDG_Enable();  //ʹ��IWDG

}

void iwdg_feed(void)
{
	IWDG_ReloadCounter();   //ι��ʱ�Ĵ���д��0xAAAAA������װ��
}