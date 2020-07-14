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
#include "./led/bsp_led.h"  
#include "./lcd/bsp_lcd.h"
#include <stdlib.h>
#include "main.h"
#include "./beep/bsp_beep.h"
#include "./RTC/bsp_rtc.h"
#include "./beep/bsp_beep.h" 

/**
  * @brief  主函数
  * @param  无
  * @retval 无
  */
int main(void)
{
  
    /* HAL 初始化 */
    HAL_Init();
    /* 系统时钟初始化成216 MHz */
    SystemClock_Config();
    /* LED 端口初始化 */
    LED_GPIO_Config();
   /* 串口初始化 */
    DEBUG_USART_Config();
	  /* 蜂鸣器端口初始化 */	
	  BEEP_GPIO_Config();
    /* LCD 端口初始化 */ 
    LCD_Init();
    /* LCD 第一层初始化 */ 
    LCD_LayerInit(0, LCD_FB_START_ADDRESS,ARGB8888);
	/* LCD 第二层初始化 */ 
    LCD_LayerInit(1, LCD_FB_START_ADDRESS+(LCD_GetXSize()*LCD_GetYSize()*4),ARGB8888);
    /* 使能LCD，包括开背光 */ 
    LCD_DisplayOn(); 

    /* 选择LCD第一层 */
    LCD_SelectLayer(0);

    /* 第一层清屏，显示全黑 */ 
    LCD_Clear(LCD_COLOR_BLACK);  

    /* 选择LCD第二层 */
    LCD_SelectLayer(1);

    /* 第二层清屏，显示全黑 */ 
    LCD_Clear(LCD_COLOR_TRANSPARENT);

    /* 配置第一和第二层的透明度,最小值为0，最大值为255*/
    LCD_SetTransparency(0, 255);
    LCD_SetTransparency(1, 0);
	
	/* 选择LCD第一层 */
    LCD_SelectLayer(0);
    /*=========================液晶初始化结束===============================*/   
	/*
	 * 当我们配置过RTC时间之后就往备份寄存器0写入一个数据做标记
	 * 所以每次程序重新运行的时候就通过检测备份寄存器0的值来判断
	 * RTC 是否已经配置过，如果配置过那就继续运行，如果没有配置过
	 * 就初始化RTC，配置RTC的时间。
	 */
  
	/* RTC配置：选择时钟源，设置RTC_CLK的分频系数 */
	RTC_CLK_Config();
   
	if (HAL_RTCEx_BKUPRead(&Rtc_Handle,RTC_BKP_DRX) != 0X32F3)
	{		
		/* 闹钟设置 */
		RTC_AlarmSet();
		
		/* 设置时间和日期 */
		RTC_TimeAndDate_Set();
		

	}
	else
	{
		/* 检查是否电源复位 */
		if (__HAL_RCC_GET_FLAG(RCC_FLAG_PORRST) != RESET)
		{
		  printf("\r\n 发生电源复位....\r\n");
		}
		/* 检查是否外部复位 */
		else if (__HAL_RCC_GET_FLAG(RCC_FLAG_PINRST) != RESET)
		{
		  printf("\r\n 发生外部复位....\r\n");
		}

		printf("\r\n 不需要重新配置RTC....\r\n");

		/* 使能 PWR 时钟 */
		__HAL_RCC_PWR_CLK_ENABLE();
		/* PWR_CR:DBF置1，使能RTC、RTC备份寄存器和备份SRAM的访问 */
		HAL_PWR_EnableBkUpAccess();
		/* 等待 RTC APB 寄存器同步 */
		HAL_RTC_WaitForSynchro(&Rtc_Handle);

	} 
	
	/* 显示时间和日期 */
	RTC_TimeAndDate_Show();
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
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage 
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 15;
  RCC_OscInitStruct.PLL.PLLN = 216;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
  }
  /** Activate the Over-Drive mode 
  */
  if (HAL_PWREx_EnableOverDrive() != HAL_OK)
  {
  }
  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
  }
}


/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
