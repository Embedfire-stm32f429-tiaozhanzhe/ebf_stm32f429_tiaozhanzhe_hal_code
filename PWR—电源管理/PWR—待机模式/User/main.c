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
  * 淘宝    :http://firestm32.taobao.com
  *
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/

#include "main.h"
#include "stm32f4xx.h"
#include "./led/bsp_led.h" 
#include "./usart/bsp_debug_usart.h"
#include "./key/bsp_key.h" 

static uint8_t KEY2_LongPress(void);

/**
  * @brief  主函数
  * @param  无
  * @retval 无
  */
int main(void) 
{
	/* 初始化系统时钟为216MHz */
	SystemClock_Config();
	/* 初始化LED */
	LED_GPIO_Config();	
	/* 初始化调试串口，一般为串口1 */
	DEBUG_USART_Config();	
	/*初始化按键，不需要中断,仅初始化KEY2即可，只用于唤醒的PA0引脚不需要这样初始化*/
	Key_GPIO_Config(); 
	
	printf("\r\n 欢迎使用秉火  STM32 F767 开发板。\r\n");
	printf("\r\n 秉火F767 待机模式例程\r\n");
	
	printf("\r\n 实验说明：\r\n");

	printf("\r\n 1.本程序中，绿灯表示本次复位是上电或引脚复位，红灯表示即将进入待机状态，蓝灯表示本次是待机唤醒的复位\r\n");
	printf("\r\n 2.长按KEY2按键后，会进入待机模式\r\n");
	printf("\r\n 3.在待机模式下，按KEY1按键可唤醒，唤醒后系统会进行复位，程序从头开始执行\r\n");
	printf("\r\n 4.可通过检测WU标志位确定复位来源\r\n");
	
	printf("\r\n 5.在待机状态下，DAP下载器无法给STM32下载程序，需要唤醒后才能下载");

	//检测复位来源
	if(__HAL_PWR_GET_FLAG(PWR_FLAG_SB) == SET)
	{
		__HAL_PWR_CLEAR_FLAG(PWR_FLAG_SB);
		LED_BLUE;
		printf("\r\n 待机唤醒复位 \r\n");
	}
	else
	{
		LED_GREEN;
		printf("\r\n 非待机唤醒复位 \r\n");
	}
	
  while(1)
  {			
		// K2 按键长按进入待机模式
		if(KEY2_LongPress())
		{
			
			printf("\r\n 即将进入待机模式，进入待机模式后可按KEY1唤醒，唤醒后会进行复位，程序从头开始执行\r\n");
			LED_RED;	
			HAL_Delay(1000);
			
			/*清除WU状态位*/
			__HAL_PWR_CLEAR_FLAG(PWR_FLAG_WU);
			
			/* 使能WKUP引脚的唤醒功能 ，使能PA0*/
			HAL_PWR_EnableWakeUpPin( 0x00000100U);
			
			//暂停滴答时钟，防止通过滴答时钟中断唤醒
			HAL_SuspendTick();			
			/* 进入待机模式 */
			HAL_PWR_EnterSTANDBYMode();
		}

  }

}


/**
  * @brief  PWR PVD interrupt callback
  * @param  None 
  * @retval None
  */
void HAL_PWR_PVDCallback(void)
{
  /* 亮红灯，实际应用中应进入紧急状态处理 */
  LED_RED;
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

/**
  * @brief  用于检测按键是否被长时间按下
  * @param  无
  * @retval 1 ：按键被长时间按下  0 ：按键没有被长时间按下
  */
static uint8_t KEY2_LongPress(void)
{			
	uint8_t downCnt =0;	//记录按下的次数
	uint8_t upCnt =0;	//记录松开的次数			

	while(1)																										//死循环，由return结束
	{	
		HAL_Delay(20);	//延迟一段时间再检测

		if(HAL_GPIO_ReadPin( KEY2_GPIO_PORT, KEY2_PIN ) == SET)	//检测到按下按键
		{
			downCnt++;	//记录按下次数
			upCnt=0;	//清除按键释放记录

			if(downCnt>=100)	//按下时间足够
			{
				return 1; 		//检测到按键被时间长按下
			}
		}
		else 
		{
			upCnt++; 			//记录释放次数
			if(upCnt>5)			//连续检测到释放超过5次
			{
				return 0;		//按下时间太短，不是按键长按操作
			}
		}//	if(HAL_GPIO_ReadPin 
	}//while
}


/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
