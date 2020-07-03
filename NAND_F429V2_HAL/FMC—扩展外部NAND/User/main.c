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
  * 淘宝    :http://firestm32.taobao.com
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
#include "./malloc/malloc.h"
#include "./nand/ftl.h"
#include "./nand/bsp_nand.h"
#include "./nand/nandtest.h"
#include "./key/bsp_key.h" 

/**
  * @brief  主函数
  * @param  无
  * @retval 无
  */
uint8_t CNT = 0;
int main(void)
{
  uint8_t *buf;
	uint8_t *backbuf;	
  
  HAL_Init();

  /* 系统时钟初始化成180 MHz */
  SystemClock_Config();
  /* LED 端口初始化 */
  LED_GPIO_Config();	 
  /* 初始化串口 */
  DEBUG_USART_Config();

  printf("\r\n野火STM32F429 NAND FLASH 读写测试例程\r\n");
    
	/*初始化SDRAM模块*/
	SDRAM_Init();
	/*初始化外部内存池*/
	my_mem_init(SRAMEX);		
	/*蓝灯亮，表示正在读写 NAND FLASH 测试*/
	LED_BLUE;
  
	while(FTL_Init())	//检测NAND FLASH,并初始化FTL
	{
		printf("NAND Error!");
		HAL_Delay(500);				 
		printf("Please Check");
		HAL_Delay(500);				 
		LED1_TOGGLE;//红灯闪烁
	}
  backbuf=mymalloc(SRAMEX,NAND_ECC_SECTOR_SIZE);	//申请一个扇区的缓存
	buf=mymalloc(SRAMIN,NAND_ECC_SECTOR_SIZE);		//申请一个扇区的缓存
	sprintf((char*)buf,"NAND Size:%dMB",(nand_dev.block_totalnum/1024)*(nand_dev.page_mainsize/1024)*nand_dev.block_pagenum);
	printf((char*)buf);	//显示NAND容量  
	FTL_ReadSectors(backbuf,2,NAND_ECC_SECTOR_SIZE,1);//预先读取扇区0到备份区域,防止乱写导致文件系统损坏.
  
	while(1)
	{
    CNT++;
    if(CNT >= 255)
      CNT = 1;
    if(Key_Scan(KEY1_GPIO_PORT, KEY1_PIN) == KEY_ON)
    {
      /* 写之前需要擦除 */
      NAND_EraseBlock(0);
      test_writepage(32,0,255,CNT);
      printf("write: %d\n", CNT);
    }
    if(Key_Scan(KEY2_GPIO_PORT, KEY2_PIN) == KEY_ON)
    {
      test_readpage(32,0,255);//0x40
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

  /* 使能HSE，配置HSE为PLL的时钟源，配置PLL的各种分频因子M N P Q 
   * PLLCLK = HSE/M*N/P = 25M / 25 *360 / 2 = 180M
   */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 25;
  RCC_OscInitStruct.PLL.PLLN = 360;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    while(1);
  }
  /* 激活 OverDrive 模式 */
  if (HAL_PWREx_EnableOverDrive() != HAL_OK)
  {
    while(1);
  }
  /* 选择PLLCLK作为SYSCLK，并配置 HCLK, PCLK1 and PCLK2 的时钟分频因子 
	 * SYSCLK = PLLCLK     = 180M
	 * HCLK   = SYSCLK / 1 = 180M
	 * PCLK2  = SYSCLK / 2 = 90M
	 * PCLK1  = SYSCLK / 4 = 45M
	 */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    while(1);
  }
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
