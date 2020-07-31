#ifndef __BSP_DAC_H
#define __BSP_DAC_H

#include "stm32f4xx.h"

#define DAC_CLK_ENABLE	

extern DAC_HandleTypeDef hdac;
extern DMA_HandleTypeDef hdma_dac1;

void DAC_4DMAmode_config(void);
#endif
