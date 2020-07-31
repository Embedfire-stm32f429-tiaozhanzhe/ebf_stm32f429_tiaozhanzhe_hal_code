#include "./dac/bsp_dac.h"

/* 波形数据 ---------------------------------------------------------*/
const uint16_t Sine12bit[32] = {
	2048	, 2460	, 2856	, 3218	, 3532	, 3786	, 3969	, 4072	,
	4093	, 4031	, 3887	, 3668	, 3382	, 3042	,	2661	, 2255	, 
	1841	, 1435	, 1054	, 714		, 428		, 209		, 65		, 3			,
	24		, 127		, 310		, 564		, 878		, 1240	, 1636	, 2048

};

uint32_t DualSine12bit[32];


DAC_HandleTypeDef hdac;
DMA_HandleTypeDef hdma_dac1;

void DAC_4DMAmode_config()
{
	/* DMA 时钟使能 */
  __HAL_RCC_DMA1_CLK_ENABLE();
	/* DAC时钟使能 */
	__HAL_RCC_DAC_CLK_ENABLE();
  
	__HAL_RCC_GPIOA_CLK_ENABLE();
	
  DAC_ChannelConfTypeDef sConfig = {0};

  /* DAC 初始化 */
  hdac.Instance = DAC;
  if (HAL_DAC_Init(&hdac) != HAL_OK)
  {
    while(1);
  }
  /* DAC 通道1输出配置 */
  sConfig.DAC_Trigger = DAC_TRIGGER_T2_TRGO;
  sConfig.DAC_OutputBuffer = DAC_OUTPUTBUFFER_ENABLE;
  if (HAL_DAC_ConfigChannel(&hdac, &sConfig, DAC_CHANNEL_1) != HAL_OK)
  {
    while(1);
  }

  /* DMA 中断初始化 */
  HAL_NVIC_SetPriority(DMA1_Stream5_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Stream5_IRQn);

	
	/*=========数据处理========*/
	uint32_t Idx = 0;  

	/* 填充正弦波形数据，双通道右对齐*/
  for (Idx = 0; Idx < 32; Idx++)
  {
    DualSine12bit[Idx] = (Sine12bit[Idx] << 16) + (Sine12bit[Idx]);
  }
	
	if (HAL_DAC_Start_DMA(&hdac,DAC_CHANNEL_1,DualSine12bit,32,DAC_ALIGN_12B_R) != HAL_OK)
  {
    /* Start DMA Error */
    while(1);
  }
	
}

/**
* @brief DAC 底层硬件初始化,在外设初始化函数HAL_DAC_Init中调用。
* @param hdac: DAC句柄
* @retval None
*/
void HAL_DAC_MspInit(DAC_HandleTypeDef* hdac)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if(hdac->Instance==DAC)
  {
    /**DAC GPIO 配置    
    PA4     ------> DAC_OUT1 
    */
    GPIO_InitStruct.Pin = GPIO_PIN_4;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /* DAC DMA 初始化 */
    /* DAC1 初始化 */
    hdma_dac1.Instance = DMA1_Stream5;
    hdma_dac1.Init.Channel = DMA_CHANNEL_7;
    hdma_dac1.Init.Direction = DMA_MEMORY_TO_PERIPH;
    hdma_dac1.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_dac1.Init.MemInc = DMA_MINC_ENABLE;
    hdma_dac1.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
    hdma_dac1.Init.MemDataAlignment = DMA_MDATAALIGN_WORD;
    hdma_dac1.Init.Mode = DMA_CIRCULAR;
    hdma_dac1.Init.Priority = DMA_PRIORITY_HIGH;
    hdma_dac1.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
    if (HAL_DMA_Init(&hdma_dac1) != HAL_OK)
    {
    while(1);
    }

    __HAL_LINKDMA(hdac,DMA_Handle1,hdma_dac1);
		/* DMA controller clock enable */
		__HAL_RCC_DMA1_CLK_ENABLE();
    /* DAC 中断配置 */
    HAL_NVIC_SetPriority(DMA1_Stream5_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(DMA1_Stream5_IRQn);

  }

}

/**
* @brief DAC 底层硬件反初始化
* @param hdac: DAC句柄
* @retval None
*/
void HAL_DAC_MspDeInit(DAC_HandleTypeDef* hdac)
{
  if(hdac->Instance==DAC)
  {
    /* Peripheral clock disable */
    __HAL_RCC_DAC_CLK_DISABLE();
  
    /**DAC GPIO Configuration    
    PA4     ------> DAC_OUT1 
    */
    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_4);

    /* DAC DMA DeInit */
    HAL_DMA_DeInit(hdac->DMA_Handle1);

    /* DAC interrupt DeInit */
    HAL_NVIC_DisableIRQ(TIM6_DAC_IRQn);

  }

}
