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
  * 实验平台:野火 STM32 F429 开发板 
  * 论坛    :http://www.firebbs.cn
  * 淘宝    :http://fire-stm32.taobao.com
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
/*	
【 ！！】操作步骤：

1.连接DAP下载器，点击debug按钮运行使程序在RAM中运行，然后耐心等待板子LED灯由蓝转绿
 （整个过程在2分钟左右，等待至5分钟还不转绿说明恢复失败）
2.LED灯变为绿灯后，打开其它工程下载程序到STM32的FLASH(如普通的流水灯例程)
3.若还是无法下载，请重试1-2步骤。
4.复位开发板，观察新的程序是否正常(如流水灯例程时，LED是否正常闪烁)
5.完成。	
*/	  
#include "stm32f4xx.h"
#include "main.h"
#include "./led/bsp_led.h"
#include "./usart/bsp_debug_usart.h"
#include "./internalFlash/internalFlash_reset.h" 

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
	/* 串口初始化 */
	DEBUG_USART_Config();
	LED_BLUE;
	
	FLASH_INFO("本程序将会被下载到STM32的内部SRAM运行。");

	FLASH_INFO("\r\n");
	FLASH_INFO("----这是一个STM32芯片内部FLASH解锁程序----"); 
	FLASH_INFO("程序会把芯片的内部FLASH选项字节恢复为默认值"); 
	
	
	#if 0  //工程调试、演示时使用，正常解除时不需要运行此函数
	WriteProtect_Test(); //修改写保护位，仿真芯片扇区被设置成写保护的的环境
	#endif

	OptionByte_Info();
	
	/*恢复选项字节到默认值，解除保护*/
	if(InternalFlash_Reset()==HAL_OK)
	{		
		FLASH_INFO("选项字节恢复成功");
		FLASH_INFO("然后随便找一个普通的程序，下载程序到芯片的内部FLASH进行测试"); 
		LED_GREEN;
	}	
	else
	{			
		FLASH_INFO("选项字节恢复成功失败，请复位重试");
		LED_RED;
	}

	OptionByte_Info();	

	while (1)
	{

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
  *            HSE Frequency(Hz)              = 25000000
  *            PLL_M                          = 25
  *            PLL_N                          = 360
  *            PLL_P                          = 2
  *            PLL_Q                          = 7
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

  /* Enable Power Control clock */
  __HAL_RCC_PWR_CLK_ENABLE();
  
  /* 使能HSE，配置HSE为PLL的时钟源，配置PLL的各种分频因子M N P Q 
	 * PLLCLK = HSE/M*N/P = 25M / 25 *432 / 2 = 216M
	 */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
  
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 25;
  RCC_OscInitStruct.PLL.PLLN = 360;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 7;
  if(HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
   while(1) {}
  }

  /* 激活 OverDrive 模式 */
  HAL_PWREx_EnableOverDrive();
 
  /* 选择PLLCLK作为SYSCLK，并配置 HCLK, PCLK1 and PCLK2 的时钟分频因子 
	 * SYSCLK = PLLCLK     = 180M
	 * HCLK   = SYSCLK / 1 = 180M
	 * PCLK2  = SYSCLK / 2 = 90M
	 * PCLK1  = SYSCLK / 4 = 45M
	 */
  RCC_ClkInitStruct.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2);
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;  
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;  
  if(HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    while(1) {}
  }
}


/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
