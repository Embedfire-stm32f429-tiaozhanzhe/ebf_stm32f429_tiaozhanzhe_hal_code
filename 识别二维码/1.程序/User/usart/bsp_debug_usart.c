/**
  ******************************************************************************
  * @file    bsp_debug_usart.c
  * @author  fire
  * @version V1.0
  * @date    2016-xx-xx
  * @brief   使用串口1，重定向c库printf函数到usart端口，中断接收模式
  ******************************************************************************
  * @attention
  *
  * 实验平台:野火 STM32 F429 开发板  
  * 论坛    :http://www.firebbs.cn
  * 淘宝    :http://fire-stm32.taobao.com
  *
  ******************************************************************************
  */ 
  
#include "./usart/bsp_debug_usart.h"

unsigned int  uart_data_len = 0;     //串口待发送数据长度
unsigned int  uart_data_index = 0;   //串口已发送数据个数
unsigned char uart_send_state= 0; //串口状态，1表示正在发送，0表示空闲
unsigned char uart_tx_buf[UART_MAX_BUF_SIZE] = {0};//串口发送数据缓冲区

UART_HandleTypeDef UartHandle;
//extern uint8_t ucTemp;  
 /**
  * @brief  DEBUG_USART GPIO 配置,工作模式配置。115200 8-N-1
  * @param  无
  * @retval 无
  */  
void DEBUG_USART_Config(void)
{
  GPIO_InitTypeDef GPIO_InitStruct;
      
  DEBUG_USART_RX_GPIO_CLK_ENABLE();
  DEBUG_USART_TX_GPIO_CLK_ENABLE();
 
  /* 使能 UART 时钟 */
  DEBUG_USART_CLK_ENABLE();

  /**USART1 GPIO Configuration    
  PA9     ------> USART2_TX
  PA10    ------> USART2_RX 
  */
  /* 配置Tx引脚为复用功能  */
  GPIO_InitStruct.Pin = DEBUG_USART_TX_PIN;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
  GPIO_InitStruct.Alternate = DEBUG_USART_TX_AF;
  HAL_GPIO_Init(DEBUG_USART_TX_GPIO_PORT, &GPIO_InitStruct);
  
  /* 配置Rx引脚为复用功能 */
  GPIO_InitStruct.Pin = DEBUG_USART_RX_PIN;
  GPIO_InitStruct.Alternate = DEBUG_USART_RX_AF;
  HAL_GPIO_Init(DEBUG_USART_RX_GPIO_PORT, &GPIO_InitStruct); 
  
  UartHandle.Instance          = DEBUG_USART;
  
  UartHandle.Init.BaudRate     = DEBUG_USART_BAUDRATE;
  UartHandle.Init.WordLength   = UART_WORDLENGTH_8B;
  UartHandle.Init.StopBits     = UART_STOPBITS_1;
  UartHandle.Init.Parity       = UART_PARITY_NONE;
  UartHandle.Init.HwFlowCtl    = UART_HWCONTROL_NONE;
  UartHandle.Init.Mode         = UART_MODE_TX_RX;
  HAL_UART_Init(&UartHandle);

  /*串口1中断初始化 */
  HAL_NVIC_SetPriority(DEBUG_USART_IRQ, 0, 0);
  HAL_NVIC_EnableIRQ(DEBUG_USART_IRQ);
  /*配置串口接收中断 */
  __HAL_UART_ENABLE_IT(&UartHandle,UART_IT_RXNE);  
}

 /**
  * @brief  获取串口发送状态
  * @param  无
  * @retval 1表示正在发送，0表示空闲
  */
uint8_t get_send_sta()
{
	if(uart_send_state)
		return 1;
	return 0;
}
 /**
  * @brief  将数据写入USART1发送缓冲区
  * @param  dat数据指针，len数据长度
  * @retval 0表示写入成功，1表示写入失败
  */
uint8_t uart_send_buf(unsigned char *dat, unsigned int len)
{
	unsigned char addr = 0;
	
	if(uart_send_state)
		return 1;
	
	uart_data_len = len;
	uart_data_index = 0;
	uart_send_state = 1;
	
	for(; len > 0; len--)
		uart_tx_buf[addr++] = *(dat++);
	
	__HAL_UART_ENABLE_IT(&UartHandle,UART_IT_TXE);
	return 0;
}
 /**
  * @brief  USART1发送中断响应函数
  * @param  
  * @retval 
  */
void USART1_IRQHandler(void)
{
	//发送中断
    if (__HAL_UART_GET_FLAG(&UartHandle,USART_FLAG_TXE) != RESET)
    {
        if(uart_data_index < uart_data_len)
		{
			HAL_UART_Transmit(&UartHandle, (uint8_t *)&uart_tx_buf[uart_data_index++], 1, 1000);
		}
		else
		{
			uart_send_state = 0;
			__HAL_UART_DISABLE_IT(&UartHandle,UART_IT_TXE);
		}
				
		__HAL_UART_CLEAR_FLAG(&UartHandle, USART_FLAG_TXE);
    }
}
/*****************  发送字符串 **********************/
void Usart_SendString( USART_TypeDef * pUSARTx, uint8_t *str)
{
	unsigned int k=0;
  do 
  {
      HAL_UART_Transmit( &UartHandle,(uint8_t *)(str + k) ,1,1000);
      k++;
  } while(*(str + k)!='\0');
  
}
///重定向c库函数printf到串口DEBUG_USART，重定向后可使用printf函数
int fputc(int ch, FILE *f)
{
	/* 发送一个字节数据到串口DEBUG_USART */
	HAL_UART_Transmit(&UartHandle, (uint8_t *)&ch, 1, 1000);	
	
	return (ch);
}

///重定向c库函数scanf到串口DEBUG_USART，重写向后可使用scanf、getchar等函数
int fgetc(FILE *f)
{
		
	int ch;
	HAL_UART_Receive(&UartHandle, (uint8_t *)&ch, 1, 1000);	
	return (ch);
}
/*********************************************END OF FILE**********************/
