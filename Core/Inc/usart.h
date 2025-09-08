/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    usart.h
  * @brief   This file contains all the function prototypes for
  *          the usart.c file
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
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __USART_H__
#define __USART_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* USER CODE BEGIN Includes */
#include "app_freertos.h"
/* USER CODE END Includes */

extern UART_HandleTypeDef huart1;

/* USER CODE BEGIN Private defines */
/* this value must be power of 2 */
#define UART_TRX_SIZE (1 << 9)
/* USER CODE END Private defines */

void MX_USART1_UART_Init(void);

/* USER CODE BEGIN Prototypes */

typedef enum
{
    UART_INT,
    UART_DMA,
    UART_NORMAL
}UART_COMMUNICATION_TYPE;

typedef enum
{
    UART_TX_STATUS_IDLE,
    UART_TX_STATUS_BUSY,
}UART_TX_STATUS;

typedef enum
{
    UART_RX_STATUS_IDLE,
    UART_RX_STATUS_BUSY,
}UART_RX_STATUS;

typedef enum
{
    UART1_IDX,
    UART_MAX_IDX,
}UART_INDEX;

typedef struct 
{
    uint8_t recv[UART_TRX_SIZE];
    int rear;
    UART_COMMUNICATION_TYPE rx_type;
    UART_COMMUNICATION_TYPE tx_type;
    UART_TX_STATUS tx_status;
    UART_RX_STATUS rx_status;
    UART_INDEX idx;
    osSemaphoreId_t *sem;
    UART_HandleTypeDef *handle;
}uart_t;

extern uart_t uart[UART_MAX_IDX];

#define print_dmesg(fmt, ...)                                                  \
do                                                                             \
{                                                                              \
    uint32_t tick = HAL_GetTick();                                             \
    prints("[%6d.%03d] " fmt "\r\n", tick / 1000, tick % 1000, ##__VA_ARGS__); \
} while (0)

void uart_init();
int uart_send(uart_t *ut, void *buf, size_t buf_size);
int uart_read(uart_t *ut, void *buf, size_t buf_size);
void prints(char *fmt, ...);
void printu(char *fmt, ...);

#define printok(fmt, ...) printu("[\x1b[32;1m  OK  \x1b[0m] "fmt"\r\n", ##__VA_ARGS__)
#define printfail(fmt, ...) printu("[\x1b[3\12;1m  FAIL  \x1b[0m] "fmt"\r\n", ##__VA_ARGS__)

#define printr(fmt, ...) prints("[\x1b[31;1mERR, %s\x1b[0m]"fmt"\r\n", __FUNCTION__, ##__VA_ARGS__)
#define printg(fmt, ...) prints("[\x1b[32;1mERR, %s\x1b[0m]"fmt"\r\n", __FUNCTION__, ##__VA_ARGS__)
#define printd(fmt, ...) prints("[\x1b[34;1mDBG, %s\x1b[0m]"fmt"\r\n", __FUNCTION__, ##__VA_ARGS__)
#define printy(fmt, ...) prints("[\x1b[32;1mWARN, %s\x1b[0m]"fmt"\r\n", __FUNCTION__, ##__VA_ARGS__)
/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif

#endif /* __USART_H__ */

