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
#include "main.h"
#include "stm32f4xx.h"
#include "./usart/bsp_debug_usart.h" 
#include "./sdram/bsp_sdram.h"
#include <stdlib.h>
#include "./led/bsp_led.h" 
#include "./lcd/bsp_lcd.h"
#include "./flash/bsp_spi_flash.h"
#include "./touch/gt9xx.h"
#include "./touch/palette.h"

void Delay(__IO uint32_t nCount); 

/**
  * @brief  主函数
  * @param  无
  * @retval 无
  */
int main(void)
{
    HAL_Init();
    
    /* 系统时钟初始化成216 MHz */
    SystemClock_Config();
    /* LED 端口初始化 */
    LED_GPIO_Config();
  
    DEBUG_USART_Config();	
    /* 初始化触摸屏 */
    GTP_Init_Panel();
 	
    /* LCD 端口初始化 */ 
    LCD_Init();
    /* LCD 第二层初始化 */ 
    LCD_LayerInit(1, LCD_FB_START_ADDRESS+(LCD_GetXSize()*LCD_GetYSize()*4),ARGB8888);
    /* 使能LCD，包括开背光 */ 
    LCD_DisplayOn(); 

    /* 选择LCD第二层 */
    LCD_SelectLayer(1);

    /* 第二层清屏，显示全黑 */ 
    LCD_Clear(LCD_COLOR_TRANSPARENT);

    /* 配置透明度,最小值为0，最大值为255*/

    LCD_SetTransparency(1, 255);
    printf("\r\n野火 STM3F429 触摸画板测试例程\r\n");
    
    /*调用画板函数*/
    Palette_Init();

    Delay(0xfff);
	
    while(1)
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

void Delay(__IO uint32_t nCount)	 //简单的延时函数
{
	for(; nCount != 0; nCount--);
}
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
