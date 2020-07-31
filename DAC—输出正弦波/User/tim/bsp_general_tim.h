#ifndef __GENERAL_TIM_H
#define	__GENERAL_TIM_H

#include "stm32f4xx.h"

#define GENERAL_TIM           		TIM2
#define GENERAL_TIM_CLK_ENABLE()  __HAL_RCC_TIM2_CLK_ENABLE()

#define GENERAL_TIM_IRQn		    	TIM2_IRQn
#define GENERAL_TIM_IRQHandler    TIM2_IRQHandler

extern TIM_HandleTypeDef TIM_TimeBaseStructure;

void TIMx_Configuration(void);

#endif /* __GENERAL_TIM_H */

