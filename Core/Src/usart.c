/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    usart.c
  * @brief   This file provides code for the configuration
  *          of the USART instances.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "usart.h"

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

UART_HandleTypeDef huart1;

/* USART1 init function */

void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  huart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart1.Init.ClockPrescaler = UART_PRESCALER_DIV1;
  huart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetTxFifoThreshold(&huart1, UART_TXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetRxFifoThreshold(&huart1, UART_RXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_DisableFifoMode(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}

void HAL_UART_MspInit(UART_HandleTypeDef* uartHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};
  if(uartHandle->Instance==USART1)
  {
  /* USER CODE BEGIN USART1_MspInit 0 */

  /* USER CODE END USART1_MspInit 0 */

  /** Initializes the peripherals clock
  */
    PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_USART1;
    PeriphClkInitStruct.Usart1ClockSelection = RCC_USART1CLKSOURCE_PCLK2;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
    {
      Error_Handler();
    }

    /* USART1 clock enable */
    __HAL_RCC_USART1_CLK_ENABLE();

    __HAL_RCC_GPIOA_CLK_ENABLE();
    /**USART1 GPIO Configuration
    PA9     ------> USART1_TX
    PA10     ------> USART1_RX
    */
    GPIO_InitStruct.Pin = GPIO_PIN_9|GPIO_PIN_10;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF7_USART1;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /* USART1 interrupt Init */
    HAL_NVIC_SetPriority(USART1_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(USART1_IRQn);
  /* USER CODE BEGIN USART1_MspInit 1 */

  /* USER CODE END USART1_MspInit 1 */
  }
}

void HAL_UART_MspDeInit(UART_HandleTypeDef* uartHandle)
{

  if(uartHandle->Instance==USART1)
  {
  /* USER CODE BEGIN USART1_MspDeInit 0 */

  /* USER CODE END USART1_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_USART1_CLK_DISABLE();

    /**USART1 GPIO Configuration
    PA9     ------> USART1_TX
    PA10     ------> USART1_RX
    */
    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_9|GPIO_PIN_10);

    /* USART1 interrupt Deinit */
    HAL_NVIC_DisableIRQ(USART1_IRQn);
  /* USER CODE BEGIN USART1_MspDeInit 1 */

  /* USER CODE END USART1_MspDeInit 1 */
  }
}

/* USER CODE BEGIN 1 */
uart_t uart[UART_MAX_IDX] = 
{
    /* uart1 */
    [UART1_IDX].handle = &huart1,
    [UART1_IDX].recv = {0, },
    [UART1_IDX].rx_status = UART_RX_STATUS_IDLE,
    [UART1_IDX].tx_status = UART_TX_STATUS_IDLE,
    [UART1_IDX].tx_type = UART_NORMAL,
    [UART1_IDX].rx_type = UART_INT,
    [UART1_IDX].sem = &uart1_semHandle,
    [UART1_IDX].idx = UART1_IDX,
    [UART1_IDX].rear = 0,
};

void __uart_recv_init(uart_t *ut)
{
    switch (ut->rx_type)
    {
    case UART_INT:
        if (HAL_UART_Receive_IT(ut->handle, ut->recv, sizeof(ut->recv)) != HAL_OK)
        {
            printfail("fail to start uart recv int : %d", ut->idx);
            Error_Handler();
        }
        break;
        
    case UART_DMA:
        if (HAL_UART_Receive_DMA(ut->handle, ut->recv, sizeof(ut->recv)) != HAL_OK)
        {
            printfail("fail to start uart recv dma : %d", ut->idx);
            Error_Handler();
        }
        break;

    case UART_NORMAL:
    default:
        break;
    }
}

void uart_init()
{    
    __uart_recv_init(&uart[UART1_IDX]);
    printok("uart recv init complete");
}

int uart_send(uart_t *ut, void *buf, size_t buf_size)
{
    int ret = 0;

    sem_wait(ut->sem);
    switch (ut->tx_type)
    {
    case UART_INT:
        if (ut->tx_status != UART_TX_STATUS_IDLE)
        {
            ret = -1;
            break;
        }

        if (HAL_UART_Transmit_IT(ut->handle, buf, buf_size) != HAL_OK)
        {
            ret = -1;
            break;
        }
        ut->tx_status = UART_TX_STATUS_BUSY;
        ret = buf_size;
        break;
    
    case UART_DMA:
        if (ut->tx_status != UART_TX_STATUS_IDLE)
        {
            ret = -1;
            break;
        }

        if (HAL_UART_Transmit_DMA(ut->handle, buf, buf_size) != HAL_OK)
        {
            ret = -1;
            break;
        }
        ut->tx_status = UART_TX_STATUS_BUSY;
        ret = buf_size;
        break;

    case UART_NORMAL:
        if (HAL_UART_Transmit(ut->handle, buf, buf_size, 100) != HAL_OK)
        {
            ret = -1;
            break;
        }
        ret = buf_size;
        break;
    
    default:
        break;
    }
    sem_post(ut->sem);
    return ret;
}

/**
 * @warning this is thread unsafe function
 */
int uart_read(uart_t *ut, void *buf, size_t buf_size)
{
    int ret = 0;
    int len = 0;
    int front = 0;

    sem_wait(ut->sem);
    switch (ut->rx_type)
    {
    case UART_NORMAL:
        if (HAL_UART_Receive(ut->handle, buf, buf_size, 100) != HAL_OK)
        {
            ret = -1;
            goto out;
        }
        break;
    
    case UART_INT:
    case UART_DMA:
    default:
        break;
    }

    switch (ut->rx_type)
    {
    case UART_INT:
    case UART_NORMAL:
        front = sizeof(ut->recv) - ut->handle->RxXferCount;
        len = (front - ut->rear) & (sizeof(ut->recv) - 1);
        break;

    case UART_DMA:
        front = sizeof(ut->recv) - __HAL_DMA_GET_COUNTER(ut->handle->hdmarx);
        len = (front - ut->rear) & (sizeof(ut->recv) - 1);
        break;
    default:
        break;
    }

    if(len == 0)
    {
        goto out;
    }

    len = (len > buf_size) ? buf_size: len;
    ret = len;
    memcpy(buf, &ut->recv[ut->rear], len);

    switch (ut->rx_type)
    {
    case UART_INT:
    case UART_DMA:
        ut->rear += len;
        ut->rear &= (sizeof(ut->recv) - 1);
        break;

    case UART_NORMAL:
    default:
        break;
    }
out:
    sem_post(ut->sem);
    return ret;
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    int idx = 0;
    for (idx = 0; idx < UART_MAX_IDX; idx++)
    {
        if (uart[idx].handle == huart)
        {
            __uart_recv_init(&uart[idx]);
        }
    }
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
    int idx = 0;
    for (idx = 0; idx < UART_MAX_IDX; idx++)
    {
        if (uart[idx].handle == huart)
        {
            uart[idx].tx_status = UART_TX_STATUS_IDLE;
        }
    }
}

/* thread safe printf */
void prints(char *fmt, ...)
{
    char pt_buf[UART_TRX_SIZE] = {0, };
    va_list va = {0, };

    va_start(va, fmt);
    vsnprintf(pt_buf, sizeof(pt_buf), fmt, va);
    va_end(va);

    while (uart[UART1_IDX].tx_status == UART_TX_STATUS_BUSY);
    uart_send(&uart[UART1_IDX], pt_buf, strlen(pt_buf));
}

/* thread unsafe printf */
void printu(char *fmt, ...)
{
    char pt_buf[UART_TRX_SIZE] = {0, };
    va_list va = {0, };

    va_start(va, fmt);
    vsnprintf(pt_buf, sizeof(pt_buf), fmt, va);
    va_end(va);

    HAL_UART_Transmit(uart[UART1_IDX].handle, (uint8_t *)pt_buf, strlen(pt_buf), 100);
}
/* USER CODE END 1 */
