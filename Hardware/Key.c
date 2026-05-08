#include "stm32f10x.h"                  // Device header

uint8_t Key_Num;

void Key_Init(void)
{
	/*开启时钟*/
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);		//开启GPIOB的时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);		//开启GPIOB的时钟

	/*GPIO初始化*/
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4 | GPIO_Pin_5;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
}


uint8_t Key_GetNum(void)
{
	if(Key_Num)
	{
		uint8_t temp;
		temp = Key_Num;
		Key_Num = 0;
		return temp;
	}
	return 0;
}



uint8_t Key_GetState(void)
{
	if(GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_4)==0)
	{
		return 1;
	}
	if(GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_5)==0)
	{
		return 2;
	}
		if(GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_0)==0)
	{
		return 3;
	}
		if(GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_1)==0)
	{
		return 4;
	}
	return 0;
}




void Key_Tick(void)
{
	static uint8_t count;
	static uint8_t CurrStatic,PrevStatic;
	
	count++;
	
	if(count>=20)
	{
		count = 0;
		
		
		PrevStatic = CurrStatic;
		CurrStatic = Key_GetState();
		
		if(CurrStatic == 0 && PrevStatic!=0)
		{
			Key_Num = PrevStatic;
		}
	}	
}

