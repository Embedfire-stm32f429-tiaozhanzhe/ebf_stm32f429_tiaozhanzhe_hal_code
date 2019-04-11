/**
  ******************************************************************************
  * @file    main.c
  * @author  fire
  * @version V1.0
  * @date    2019-xx-xx
  * @brief   MP3 解码播放
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
#include "./led/bsp_led.h" 
#include "./usart/bsp_debug_usart.h"
#include "./key/bsp_key.h" 
/* FatFs includes component */
//#include "ff.h"
//#include "ff_gen_drv.h"
//#include "sd_diskio.h"

#include "bsp_wm8978.h"
/**
  ******************************************************************************
  *                              定义变量
  ******************************************************************************
  */
#include "main.h"
#include "stm32f4xx.h"
#include "./led/bsp_led.h" 
#include "./usart/bsp_debug_usart.h"
#include "./key/bsp_key.h" 
#include "./TouchPad/bsp_touchpad.h"
/* FatFs includes component */
#include "ff.h"
#include "ff_gen_drv.h"
#include "sd_diskio.h"
#include "recorder/Recorder.h"
#include "bsp_wm8978.h"
#include "bsp_SysTick.h"
/**
  ******************************************************************************
  *                              定义变量
  ******************************************************************************
  */
char SDPath[4]; /* SD逻辑驱动器路径 */
FATFS fs;													/* FatFs文件系统对象 */
FIL fnew;													/* 文件对象 */
FRESULT res_sd;                /* 文件操作结果 */
UINT fnum;            			  /* 文件成功读写数量 */
BYTE ReadBuffer[1024]={0};        /* 读缓冲区 */
BYTE WriteBuffer[] =              /* 写缓冲区*/
"欢迎使用野火STM32 F429开发板 今天是个好日子，新建文件系统测试文件\r\n";  

extern FATFS flash_fs;
extern Diskio_drvTypeDef  SD_Driver;

/**
	**************************************************************
	* Description : 初始化WiFi模块使能引脚，并禁用WiFi模块
	* Argument(s) : none.
	* Return(s)   : none.
	**************************************************************
	*/
static void WIFI_PDN_INIT(void)
{
	/*定义一个GPIO_InitTypeDef类型的结构体*/
	GPIO_InitTypeDef GPIO_InitStruct;
	/*使能引脚时钟*/	
	__HAL_RCC_GPIOB_CLK_ENABLE();
	/*选择要控制的GPIO引脚*/															   
	GPIO_InitStruct.Pin = GPIO_PIN_13;	
	/*设置引脚的输出类型为推挽输出*/
	GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;      
	/*设置引脚为上拉模式*/
	GPIO_InitStruct.Pull  = GPIO_PULLUP;
	/*设置引脚速率为高速 */   
	GPIO_InitStruct.Speed = GPIO_SPEED_FAST; 
	/*调用库函数，使用上面配置的GPIO_InitStructure初始化GPIO*/
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);	
	/*禁用WiFi模块*/
	HAL_GPIO_WritePin(GPIOB,GPIO_PIN_13,GPIO_PIN_RESET);  
}

/**
  * @brief  主函数
  * @param  无
  * @retval 无
  */
int main(void)
{	
  FRESULT res;                                          /* FatFs function common result code */
  
  /* 配置系统时钟为168 MHz */
  SystemClock_Config();

  SysTick_Init();
  
  TPAD_Init();
  
  /* 禁用WiFi */
  WIFI_PDN_INIT();
  
  /*初始化USART1*/
  DEBUG_USART_Config();
 
  printf("Music Player\n");
  
  FATFS_LinkDriver(&SD_Driver, SDPath);
  
  //在外部SD卡挂载文件系统，文件系统挂载时会对SD卡初始化
	res = f_mount(&fs,"0:",1);
//  res = f_mount(&SDFatFs, (TCHAR const*)SDPath, 0) ;
  if(res!=FR_OK)
	{
		printf("！！SD卡挂载文件系统失败。(%d)\r\n",res);
		printf("！！可能原因：SD卡初始化不成功。\r\n");
		while(1);
  }
  else
	{
		printf("SD卡挂载成功\r\n");
	}
	/* 检测WM8978芯片，此函数会自动配置CPU的GPIO */
	if (wm8978_Init()==0)
	{
		printf("检测不到WM8978芯片!!!\n");
		while (1);	/* 停机 */
	}
	printf("初始化WM8978成功\n");	
  
  while(1)
  {
    /* 录音与回放功能 */
    RecorderDemo();
  }
}


/**
  * @brief  System Clock Configuration
  *         The system Clock is configured as follow : 
  *            System Clock source            = PLL (HSE)
  *            SYSCLK(Hz)                     = 168000000
  *            HCLK(Hz)                       = 168000000
  *            AHB Prescaler                  = 1
  *            APB1 Prescaler                 = 4
  *            APB2 Prescaler                 = 2
  *            HSE Frequency(Hz)              = 8000000
  *            PLL_M                          = 25
  *            PLL_N                          = 336
  *            PLL_P                          = 2
  *            PLL_Q                          = 7
  *            VDD(V)                         = 3.3
  *            Main regulator output voltage  = Scale1 mode
  *            Flash Latency(WS)              = 5
  * @param  None
  * @retval None
  */
 void SystemClock_Config(void)
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
