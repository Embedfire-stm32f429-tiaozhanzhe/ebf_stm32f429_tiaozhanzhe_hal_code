/**
  ******************************************************************************
  * @file    main.c
  * @author  fire
  * @version V1.0
  * @date    2017-xx-xx
  * @brief   GPIO输出--使用固件库点亮LED灯
  ******************************************************************************
  * @attention
  *
  * 实验平台:野火  STM32 F429 开发板 
  * 论坛    :http://www.firebbs.cn
  * 淘宝    :http://fire-stm32.taobao.com
  *
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/

#include "main.h"
#include "stm32f4xx.h"
#include "./led/bsp_led.h"
#include "./key/bsp_key.h" 
#include "./iwdg/bsp_iwdg.h"
static void SystemClock_Config(void);
/**
  * @brief  主函数
  * @param  无
  * @retval 无
  */
int main(void)
{
  /* 系统时钟初始化成216 MHz */
  SystemClock_Config();
	/* LED 端口初始化 */
	LED_GPIO_Config();	 
  
	HAL_Delay(1000);
	/* 检查是否为独立看门狗复位 */
	if (__HAL_RCC_GET_FLAG(RCC_FLAG_IWDGRST) != RESET)
	{
		/* 独立看门狗复位 */
		/*  亮红灯 */
		LED_RED;

		/* 清除标志 */
		__HAL_RCC_CLEAR_RESET_FLAGS();
		
		/*如果一直不喂狗，会一直复位，加上前面的延时，会看到红灯闪烁
		在1s 时间内喂狗的话，则会持续亮绿灯*/
	}
	else
	{
		/*不是独立看门狗复位(可能为上电复位或者手动按键复位之类的) */
		/* 亮蓝灯 */
		LED_BLUE;
	}		

	/*初始化按键*/
	Key_GPIO_Config();	
	
	// IWDG 1s 超时溢出
	IWDG_Config(IWDG_PRESCALER_64 ,625); 

 
	//while部分是我们在项目中具体需要写的代码，这部分的程序可以用独立看门狗来监控
	//如果我们知道这部分代码的执行时间，比如是500ms，那么我们可以设置独立看门狗的
	//溢出时间是600ms，比500ms多一点，如果要被监控的程序没有跑飞正常执行的话，那么
	//执行完毕之后就会执行喂狗的程序，如果程序跑飞了那程序就会超时，到达不了喂狗的
	//程序，此时就会产生系统复位。但是也不排除程序跑飞了又跑回来了，刚好喂狗了，
	//歪打正着。所以要想更精确的监控程序，可以使用窗口看门狗，窗口看门狗规定必须
	//在规定的窗口时间内喂狗。
	while(1)                            
	{	   
		if( Key_Scan(KEY1_GPIO_PORT,KEY1_PIN) == KEY_ON  )
		{
			// 喂狗，如果不喂狗，系统则会复位，复位后亮红灯，如果在1s
			// 时间内准时喂狗的话，则会亮绿灯
			IWDG_Feed();		
			//喂狗后亮绿灯
			LED_GREEN;
		}
		
	}
}	

/**
  * @brief  系统时钟配置 
  *            System Clock source            = PLL (HSE)
  *            SYSCLK(Hz)                     = 180000000
  *            HCLK(Hz)                       = 180000000
  *            AHB Prescaler                  = 1
  *            APB1 Prescaler                 = 4
  *            APB2 Prescaler                 = 2
  *            HSE Frequency(Hz)              = 12000000
  *            PLL_M                          = 25
  *            PLL_N                          = 360
  *            PLL_P                          = 2
  *            PLL_Q                          = 4
  *            VDD(V)                         = 3.3
  *            Main regulator output voltage  = Scale1 mode
  *            Flash Latency(WS)              = 5
  * @param  无
  * @retval 无
  */
static void SystemClock_Config(void)
{
  RCC_ClkInitTypeDef RCC_ClkInitStruct;
  RCC_OscInitTypeDef RCC_OscInitStruct;
  HAL_StatusTypeDef ret = HAL_OK;
  
   /* 使能HSE，配置HSE为PLL的时钟源，配置PLL的各种分频因子M N P Q 
	  * PLLCLK = HSE/M*N/P = 12M / 25 *360 / 2 = 180M
	  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 25;
  RCC_OscInitStruct.PLL.PLLN = 360;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  ret = HAL_RCC_OscConfig(&RCC_OscInitStruct);
  
  if( ret != HAL_OK)
  {
    while(1) {}
  }

  /* 激活 OverDrive 模式以达到180M频率 */
  ret =HAL_PWREx_EnableOverDrive();
   if( ret != HAL_OK)
  {
    while(1) {}
  }
 
  /* 选择PLLCLK作为SYSCLK，并配置 HCLK, PCLK1 and PCLK2 的时钟分频因子 
	 * SYSCLK = PLLCLK     = 216M
	 * HCLK   = SYSCLK / 1 = 216M
	 * PCLK2  = SYSCLK / 2 = 108M
	 * PCLK1  = SYSCLK / 4 = 54M
	 */
  RCC_ClkInitStruct.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2);
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;  
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;
  
  ret =HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5);
  
   if( ret != HAL_OK)
  {
    while(1) {}
  }
}




/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
