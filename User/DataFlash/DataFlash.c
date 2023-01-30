#include "DataFlash.h"
#include "stm32f10x_spi.h"
#include "stm32f10x_flash.h"
#include "DCTFT.h"
struct Para Para_Data;

static   unsigned char sectorbufferTemp[1024];  

/*** **************************************************************************
*������ : System_Read_Flash
*��  �� : ��ȡоƬ�ڲ�FLASH , ��ȡָ���ĳ��ȡ�
*��  �� : unsigned long ReadAddr   ���ݵ�ַ
					unsigned char *pBuffer   ��ȡ����λ��
					unsigned int  NumToRead   ��ȡ���ֽ���
*����ֵ : 0 �ɹ�  1 ʧ��  ��ַС��FLASH��ַ  2ʧ�� ��ַ����Flash��ַ��
 ******************************************************************************/ 
unsigned  char System_Read_Flash(unsigned long ReadAddr,unsigned char *pBuffer,unsigned int NumToRead)
{
		unsigned int flashSize = *(uint16_t*)(0x1FFFF7E0); 
		if(ReadAddr < Flash_Base_Address)  return 1;
		if(ReadAddr > (Flash_Base_Address + (flashSize * 2048))) return 2;
		while(NumToRead--)
		{
				*pBuffer = *(__IO unsigned char *)ReadAddr;
				pBuffer ++;
				ReadAddr++;
		}
		return 0;
}
/*** **************************************************************************
*������ : WaitForFlashBSY
*��  �� : �ж��ڲ�FLASH�Ƿ�æµ
*��  �� : unsigned long timeout  �ȴ�ʱ��
*����ֵ : 0 �ɹ�  1 ʧ��  ��ʱ
 ******************************************************************************/ 
static unsigned char WaitForFlashBSY(unsigned long timeout)
{
	while((FLASH->SR & 0x01)&&(timeout-- != 0))
		if(timeout ==0)
			return 1;
	return  0;
}
/*** **************************************************************************
*������ : System_Write_Flash
*��  �� : д��ָ�����ȵ�����
*��  �� : unsigned long WriteAddr д��ĵ�ַ ��ַ������ż��
					unsigned char *WriteToBuffer 
					unsinged int   NumToWrite
*����ֵ : 0 �ɹ�  1 ʧ��  ��ʱ
 ******************************************************************************/ 
unsigned char System_Write_Flash(unsigned long WriteAddr,unsigned char *WriteToBuffer, unsigned int NumToWrite)
{
	unsigned int FlashSize = *(unsigned int *)(0x1ffff7e0);  //��ȡоƬ��С
	
	unsigned long addrOff = WriteAddr - Flash_Base_Address;  //ȥ��0x08000000���ʵ��ƫ�Ƶ�ַ
	unsigned long SecPos  = addrOff / 2048;									 //��ʼ��ַ������λ��	
	unsigned int  SecOff  = addrOff % 2048;									 // ������������ƫ��λ��
	unsigned int  secRemain  = 2048 - SecOff; 							 //����������Ҫд����ֽ����������ж��Ƿ񹻿ռ䴢�档
	

	if(WriteAddr < Flash_Base_Address) return 1;						 //��ַС�ڻ�����ַ
	if(WriteAddr > (Flash_Base_Address + (FlashSize * 2048))) return 2;  //��ַ���ڴ����ַ��
	
	FLASH->KEYR = ((unsigned long) 0x45670123);
	FLASH->KEYR = ((unsigned long) 0xCDEF89AB);  // ����flash
	
	if(NumToWrite <= secRemain) secRemain = NumToWrite;
	while(1)
	{
		//��ȡ��ǰ����������
		if(WaitForFlashBSY(0x00888888)) return 2;
		System_Read_Flash(SecPos * 2048 +Flash_Base_Address,sectorbufferTemp,2048);
		
		if(WaitForFlashBSY(0x00888888)) return 2;
		FLASH->CR|= 1<<1;																				//ѡ��ҳ����
		FLASH->AR = Flash_Base_Address + SecPos* 2048;					//������ҳ��ַ
		FLASH->CR|= 0x40;
		if(WaitForFlashBSY(0x00888888)) return 2;
		FLASH->CR &= ((unsigned long) 0x00001FFD);
		for(unsigned int i=0; i<secRemain;i++)
		{
			sectorbufferTemp[SecOff+i] = WriteToBuffer[i];
		}
		
		for(unsigned int i=0; i<2048/ 2;i++)
		{
			if(WaitForFlashBSY(0x00888888)) return 2;
			FLASH->CR |= 0X01<<0;
			*(unsigned int*)(Flash_Base_Address + SecPos*1+i*2)= (sectorbufferTemp[i*2+1]<<8) | sectorbufferTemp[i*2];
			
		}
		if(WaitForFlashBSY(0x00888888))   return 2;                     
            FLASH->CR &= ((uint32_t)0x00001FFE);      

		if(secRemain == NumToWrite)
			break;
		else
		{
			WriteToBuffer += secRemain;
			SecPos++;
			SecOff=0;
			NumToWrite -=secRemain;
			secRemain = (NumToWrite>2048)?(2048):NumToWrite;
		}
	}
	FLASH->CR |= 1<<7;
	return 0;
}
