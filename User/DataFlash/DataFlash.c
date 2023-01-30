#include "DataFlash.h"
#include "stm32f10x_spi.h"
#include "stm32f10x_flash.h"
#include "DCTFT.h"
struct Para Para_Data;

static   unsigned char sectorbufferTemp[1024];  

/*** **************************************************************************
*函数名 : System_Read_Flash
*功  能 : 读取芯片内部FLASH , 读取指定的长度。
*参  数 : unsigned long ReadAddr   数据地址
					unsigned char *pBuffer   读取后存放位置
					unsigned int  NumToRead   读取的字节数
*返回值 : 0 成功  1 失败  地址小于FLASH基址  2失败 地址大于Flash地址。
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
*函数名 : WaitForFlashBSY
*功  能 : 判断内部FLASH是否忙碌
*参  数 : unsigned long timeout  等待时长
*返回值 : 0 成功  1 失败  超时
 ******************************************************************************/ 
static unsigned char WaitForFlashBSY(unsigned long timeout)
{
	while((FLASH->SR & 0x01)&&(timeout-- != 0))
		if(timeout ==0)
			return 1;
	return  0;
}
/*** **************************************************************************
*函数名 : System_Write_Flash
*功  能 : 写入指定长度的数据
*参  数 : unsigned long WriteAddr 写入的地址 地址必须是偶数
					unsigned char *WriteToBuffer 
					unsinged int   NumToWrite
*返回值 : 0 成功  1 失败  超时
 ******************************************************************************/ 
unsigned char System_Write_Flash(unsigned long WriteAddr,unsigned char *WriteToBuffer, unsigned int NumToWrite)
{
	unsigned int FlashSize = *(unsigned int *)(0x1ffff7e0);  //读取芯片大小
	
	unsigned long addrOff = WriteAddr - Flash_Base_Address;  //去掉0x08000000后的实际偏移地址
	unsigned long SecPos  = addrOff / 2048;									 //起始地址的扇区位置	
	unsigned int  SecOff  = addrOff % 2048;									 // 数据在扇区的偏移位置
	unsigned int  secRemain  = 2048 - SecOff; 							 //本个扇区需要写入的字节数。用于判断是否够空间储存。
	

	if(WriteAddr < Flash_Base_Address) return 1;						 //地址小于基础地址
	if(WriteAddr > (Flash_Base_Address + (FlashSize * 2048))) return 2;  //地址大于储存地址。
	
	FLASH->KEYR = ((unsigned long) 0x45670123);
	FLASH->KEYR = ((unsigned long) 0xCDEF89AB);  // 解锁flash
	
	if(NumToWrite <= secRemain) secRemain = NumToWrite;
	while(1)
	{
		//读取当前扇区的数据
		if(WaitForFlashBSY(0x00888888)) return 2;
		System_Read_Flash(SecPos * 2048 +Flash_Base_Address,sectorbufferTemp,2048);
		
		if(WaitForFlashBSY(0x00888888)) return 2;
		FLASH->CR|= 1<<1;																				//选择页擦除
		FLASH->AR = Flash_Base_Address + SecPos* 2048;					//擦除的页地址
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
