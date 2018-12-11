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
#include "./lcd/bsp_lcd.h"
#include "./flash/bsp_spi_flash.h"
#include "./i2c/bsp_i2c.h"
#include "./camera/bsp_ov5640.h"
#include "./camera/ov5640_AF.h"
#include "qr_decoder_user.h"
#include "./beep/bsp_beep.h"   

/*简单任务管理*/
uint32_t Task_Delay[NumOfTask]={0};
uint8_t dispBuf[100];
uint8_t fps=0;

uint8_t beep_on_flag = 0;//蜂鸣器状态，1表示蜂鸣器响，0表示蜂鸣器不响
/**
  * @brief  主函数
  * @param  无
  * @retval 无
  */
int main(void)
{
    char  qr_type_len=0;
    short qr_data_len=0;
    char qr_type_buf[10];
    char qr_data_buf[512];
    int addr=0;
    int i=0,j=0;
    char qr_num=0;
	uint32_t tickstart = 0;
	uint32_t tickstop = 0;
	
	OV5640_IDTypeDef OV5640_Camera_ID;
	
    /* 系统时钟初始化成216 MHz */
    SystemClock_Config();
    /* LED 端口初始化 */
    LED_GPIO_Config();	
	/* 蜂鸣器端口初始化 */
	BEEP_GPIO_Config();
	/*初始化USART1*/
	DEBUG_USART_Config(); 
    /* LCD 端口初始化 */ 
    LCD_Init();
    /* LCD 第一层初始化 */ 
    LCD_LayerInit(0, LCD_FB_START_ADDRESS,RGB565);
	/* LCD 第二层初始化 */ 
    LCD_LayerInit(1, LCD_FB_START_ADDRESS+(LCD_GetXSize()*LCD_GetYSize()*4),ARGB8888);

    /* 选择LCD第一层 */
    LCD_SelectLayer(0);

    /* 第一层清屏，显示蓝色 */ 
    LCD_Clear(LCD_COLOR_BLACK);  
	
	/* 选择LCD第二层 */
    LCD_SelectLayer(1);
    /* 第二层清屏，显示全黑 */ 
    LCD_Clear(LCD_COLOR_TRANSPARENT);
    /* 配置第一和第二层的透明度,最小值为0，最大值为255*/
    LCD_SetTransparency(0, 255);
    LCD_SetTransparency(1, 200);
		
    LCD_SetColors(LCD_COLOR_RED,LCD_COLOR_TRANSPARENT);
    LCD_DisplayStringLine_EN_CH(1,(uint8_t* )" 模式:UXGA 800x480");
    CAMERA_DEBUG("STM32F767 DCMI 驱动OV5640例程");

	OV5640_HW_Init();			
	//初始化 I2C
	I2CMaster_Init(); 
    /* 使能LCD，包括开背光 */ 
    LCD_DisplayOn(); 
	/* 读取摄像头芯片ID，确定摄像头正常连接 */
	OV5640_ReadID(&OV5640_Camera_ID);

	if(OV5640_Camera_ID.PIDH  == 0x56)
	{
		CAMERA_DEBUG("%x%x",OV5640_Camera_ID.PIDH ,OV5640_Camera_ID.PIDL);
	}
	else
	{
		LCD_SetColors(LCD_COLOR_RED,LCD_COLOR_TRANSPARENT);
		LCD_DisplayStringLine_EN_CH(8,(uint8_t*) "         没有检测到OV5640，请重新检查连接。");
		CAMERA_DEBUG("没有检测到OV5640摄像头，请重新检查连接。");
		while(1);  
	}
	LCD_Open_QR_Window();
    /* 配置摄像头输出像素格式 */
	OV5640_RGB565Config();	
	/* 初始化摄像头，捕获并显示图像 */
	OV5640_Init();
    OV5640_AUTO_FOCUS();
	//重置
    fps =0;
	Task_Delay[0]=1000;
	
	while(1)
	{
		//二维码识别，返回识别条码的个数
		qr_num = QR_decoder();
		if(qr_num)
		{
		   //识别成功，蜂鸣器响标志
		   beep_on_flag =1;	
       BEEP_Handler(1);      
		   //解码的数据是按照识别条码的个数封装好的二维数组，这些数据需要
		   //根据识别条码的个数，按组解包并通过串口发送到上位机串口终端
		   for(i=0;i < qr_num;i++)
		   {
			   qr_type_len = decoded_buf[i][addr++];//获取解码类型长度
			   
			   for(j=0;j < qr_type_len;j++)
				   qr_type_buf[j]=decoded_buf[i][addr++];//获取解码类型名称
			   
			   qr_data_len  = decoded_buf[i][addr++]<<8; //获取解码数据长度高8位
			   qr_data_len |= decoded_buf[i][addr++];    //获取解码数据长度低8位
			   
			   for(j=0;j < qr_data_len;j++)
				   qr_data_buf[j]=decoded_buf[i][addr++];//获取解码数据
			   
			   uart_send_buf((unsigned char *)qr_type_buf, qr_type_len);//串口发送解码类型
			   while(get_send_sta()); //等待串口发送完毕
			   uart_send_buf((unsigned char *)":", 1);    //串口发送分隔符
			   while(get_send_sta()); //等待串口发送完毕
			   uart_send_buf((unsigned char *)qr_data_buf, qr_data_len);//串口发送解码数据
			   while(get_send_sta()); //等待串口发送完毕
			   uart_send_buf((unsigned char *)"\r\n", 2); //串口发送分隔符
			   while(get_send_sta());//等待串口发送完毕
			   addr =0;//清零
		   }
			   
		} 
        if(Task_Delay[0]==0)
        {
			tickstop = HAL_GetTick() - tickstart;                
            sprintf((char*)dispBuf, " 帧率:%d FPS", fps*1000/tickstop);			
            /*输出帧率*/ 
			LCD_SelectLayer(1);
			LCD_SetColors(LCD_COLOR_RED,LCD_COLOR_TRANSPARENT);
			LCD_ClearLine(2);
            LCD_DisplayStringLine_EN_CH(2,dispBuf);
            //重置
            fps =0;

            Task_Delay[0]=1000; //此值每1ms会减1，减到0才可以重新进来这里
			tickstart = HAL_GetTick();            
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
