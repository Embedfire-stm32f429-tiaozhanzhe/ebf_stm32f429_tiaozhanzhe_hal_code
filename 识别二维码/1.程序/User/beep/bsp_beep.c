/**
  ******************************************************************************
  * @file    bsp_led.c
  * @author  fire
  * @version V1.0
  * @date    2016-xx-xx
  * @brief   蜂鸣器函数接口
  ******************************************************************************
  * @attention
  *
  * 实验平台:野火  STM32 F429 开发板  
  * 论坛    :http://www.firebbs.cn
  * 淘宝    :http://firestm32.taobao.com
  *
  ******************************************************************************
  */
  
#include "./beep/bsp_beep.h"   
#include "main.h"

 extern uint8_t beep_on_flag;
 /**
  * @brief  初始化控制蜂鸣器的IO
  * @param  无
  * @retval 无
  */
void BEEP_GPIO_Config(void)
{		
    /*定义一个GPIO_InitTypeDef类型的结构体*/
    GPIO_InitTypeDef GPIO_InitStructure;

    /*开启控制蜂鸣器的GPIO的端口时钟*/
    BEEP_GPIO_CLK_ENABLE();   

    /*选择要控制蜂鸣器的GPIO*/															   
    GPIO_InitStructure.Pin = BEEP_GPIO_PIN;	

    /*设置GPIO模式为通用推挽输出*/
    GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;   

    GPIO_InitStructure.Pull = GPIO_PULLDOWN;

    /*设置GPIO速率为50MHz */   
    GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_HIGH; 

    /*调用库函数，初始化控制蜂鸣器的GPIO*/
    HAL_GPIO_Init(BEEP_GPIO_PORT, &GPIO_InitStructure);			 

    /*  关闭蜂鸣器*/
    HAL_GPIO_WritePin(BEEP_GPIO_PORT, BEEP_GPIO_PIN,GPIO_PIN_RESET);	 
}
 /**
  * @brief  控制蜂鸣器响次数
  * @param  times 响的次数，可以选择1，2,3次
  * @retval 无
  */
void BEEP_Handler(uint8_t times)
{
	static uint8_t beepstep=1;
	//接收到蜂鸣器响的标志才响
	if(beep_on_flag && (!Task_Delay[1]))
	{
		//根据蜂鸣器响的预设次数来判断操作蜂鸣器响的步骤
		if(beepstep>2*times)
			beepstep =0;
		switch(beepstep)
		{
			case 1:  
					BEEP_ON;
					Task_Delay[1] = 50;
					beepstep = 2;
			break ;
			case 2:  
					BEEP_OFF;
					Task_Delay[1] = 30;
					beepstep = 3;
			break ;
			case 3:  
					BEEP_ON;
					Task_Delay[1] = 50;
					beepstep = 4;
			break ;
			case 4:  
					BEEP_OFF;
					Task_Delay[1] = 30;
					beepstep = 5;
			break ;
			case 5:  
					BEEP_ON;
					Task_Delay[1] = 50;
					beepstep = 6;
			break ;
			case 6:  
					BEEP_OFF;
					beepstep = 7;
			break ;			
			default :
					beepstep = 1;				
					beep_on_flag = 0;
		}	
	}
	
}

