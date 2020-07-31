/**
  ******************************************************************************
  * @file    bsp_GENERAL_tim.c
  * @author  STMicroelectronics
  * @version V1.0
  * @date    2015-xx-xx
  * @brief   高级控制定时器定时范例
  ******************************************************************************
  * @attention
  *
  * 实验平台:野火  STM32 F429 开发板  
  * 论坛    :http://www.firebbs.cn
  * 淘宝    :http://firestm32.taobao.com
  *
  ******************************************************************************
  */
  
#include "./tim/bsp_general_tim.h"
TIM_HandleTypeDef TIM_TimeBaseStructure;
/*
 * 注意：TIM_TimeBaseInitTypeDef结构体里面有5个成员，TIM6和TIM7的寄存器里面只有
 * TIM_Prescaler和TIM_Period，所以使用TIM6和TIM7的时候只需初始化这两个成员即可，
 * 另外三个成员是通用定时器和高级定时器才有.
 *-----------------------------------------------------------------------------
 * TIM_Prescaler         都有
 * TIM_CounterMode			 TIMx,x[6,7]没有，其他都有（基本定时器）
 * TIM_Period            都有
 * TIM_ClockDivision     TIMx,x[6,7]没有，其他都有(基本定时器)
 * TIM_RepetitionCounter TIMx,x[1,8]才有(高级定时器)
 *-----------------------------------------------------------------------------
 */
static void TIM_Mode_Config(void)
{
	TIM_MasterConfigTypeDef sMasterConfig;

	GENERAL_TIM_CLK_ENABLE(); 
	
	TIM_TimeBaseStructure.Instance = GENERAL_TIM;
	/* 累计 TIM_Period个后产生一个更新或者中断*/		
	//当定时器从0计数到4999，即为5000次，为一个定时周期
	TIM_TimeBaseStructure.Init.Period =10000-1;       
	// 高级控制定时器时钟源TIMxCLK = HCLK=180MHz 
	// 设定定时器频率为=TIMxCLK/(TIM_Prescaler+1)=10000Hz
	TIM_TimeBaseStructure.Init.Prescaler = 180;	
	// 采样时钟分频
	TIM_TimeBaseStructure.Init.ClockDivision=TIM_CLOCKDIVISION_DIV1;
	// 计数方式
	TIM_TimeBaseStructure.Init.CounterMode=TIM_COUNTERMODE_UP;
	// 重复计数器:重复0次，不生成中断
	TIM_TimeBaseStructure.Init.RepetitionCounter=0;
	
  HAL_TIM_Base_Init(&TIM_TimeBaseStructure);

	sMasterConfig.MasterOutputTrigger = TIM_TRGO_UPDATE;
	sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
	
	HAL_TIMEx_MasterConfigSynchronization(&TIM_TimeBaseStructure, &sMasterConfig);
	
	HAL_TIM_Base_Start(&TIM_TimeBaseStructure);

}

/**
  * @brief  初始化高级控制定时器定时，1s产生一次中断
  * @param  无
  * @retval 无
  */
void TIMx_Configuration(void)
{
	TIM_Mode_Config();	
}

/*********************************************END OF FILE**********************/
