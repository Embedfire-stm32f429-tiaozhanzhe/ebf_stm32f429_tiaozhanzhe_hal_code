
/*
	使用寄存器的方法点亮LED灯
  */
#include "stm32f4xx_hal_gpio.h"

void Delay( uint32_t nCount); 

/**
  *   主函数，使用封装好的函数来控制LED灯
  */
int main(void)
{	
	GPIO_InitTypeDef InitStruct;
	
	/*开启 GPIOH 时钟，使用外设时都要先开启它的时钟*/
	RCC->AHB1ENR |= (1<<7);

	/* LED 端口初始化 */
	
	/*初始化PH10引脚*/
	/*选择要控制的GPIO引脚*/															   
	InitStruct.GPIO_Pin = GPIO_Pin_10;
	/*设置引脚模式为输出模式*/
	InitStruct.GPIO_Mode = GPIO_Mode_OUT;
	/*设置引脚的输出类型为推挽输出*/
	InitStruct.GPIO_OType = GPIO_OType_PP;
	/*设置引脚为上拉模式*/
	InitStruct.GPIO_PuPd = GPIO_PuPd_UP;
	/*设置引脚速率为2MHz */   
	InitStruct.GPIO_Speed = GPIO_Speed_2MHz;
	/*调用库函数，使用上面配置的GPIO_InitStructure初始化GPIO*/
	GPIO_Init(GPIOH, &InitStruct);	

	/*使引脚输出低电平,点亮LED1*/
	GPIO_ResetBits(GPIOH,GPIO_Pin_10);

	/*延时一段时间*/
	Delay(0xFFFFFF);	
	
	/*使引脚输出高电平，关闭LED1*/
	GPIO_SetBits(GPIOH,GPIO_Pin_10);
	
	/*初始化PH11引脚*/
	InitStruct.GPIO_Pin = GPIO_Pin_11;
	GPIO_Init(GPIOH,&InitStruct);
	
	/*使引脚输出低电平，点亮LED2*/
	GPIO_ResetBits(GPIOH,GPIO_Pin_11);

	while(1);

}

//简单的延时函数，让cpu执行无意义指令，消耗时间
//具体延时时间难以计算，以后我们可使用定时器精确延时
void Delay( uint32_t nCount)	 
{
	for(; nCount != 0; nCount--);
}

// 函数为空，目的是为了骗过编译器不报错
void SystemInit(void)
{	
}


///**
//  *   主函数,使用结构体指针，直接控制寄存器的方式来点灯
//			感兴趣可以解除注释来尝试一下
//  */
//int main(void)
//{	
//	/*开启 GPIOH 时钟，使用外设时都要先开启它的时钟*/
//	RCC->AHB1ENR |= (1<<7);

//	/* LED 端口初始化 */
//	
//	/*GPIOH MODER10清空*/
//	GPIOH->MODER  &= ~( 0x03<< (2*10));	
//	/*PH10 MODER10 = 01b 输出模式*/
//	GPIOH->MODER |= (1<<2*10);
//	
//	/*GPIOH OTYPER10清空*/
//	GPIOH->OTYPER &= ~(1<<1*10);
//	/*PH10 OTYPER10 = 0b 推挽模式*/
//	GPIOH->OTYPER |= (0<<1*10);
//	
//	/*GPIOH OSPEEDR10清空*/
//	GPIOH->OSPEEDR &= ~(0x03<<2*10);
//	/*PH10 OSPEEDR10 = 0b 速率2MHz*/
//	GPIOH->OSPEEDR |= (0<<2*10);
//	
//	/*GPIOH PUPDR10清空*/
//	GPIOH->PUPDR &= ~(0x03<<2*10);
//	/*PH10 PUPDR10 = 01b 上拉模式*/
//	GPIOH->PUPDR |= (1<<2*10);
//	
//	/*PH10 BSRR寄存器的 BR10置1，使引脚输出低电平*/
//	GPIOH->BSRRH |= (1<<10);
//	
//	/*PH10 BSRR寄存器的 BS10置1，使引脚输出高电平*/
////	GPIOH->BSRRL |= (1<<10); 

//	while(1);

//}




/*********************************************END OF FILE**********************/

