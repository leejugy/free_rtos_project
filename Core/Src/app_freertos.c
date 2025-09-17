/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : app_freertos.c
  * Description        : FreeRTOS applicative file
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
#include "app_freertos.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "eth.h"
#include "usart.h"
#include "cli.h"
#include "tcp_client.h"
#include "tcp_server.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
typedef StaticTask_t osStaticThreadDef_t;
typedef StaticSemaphore_t osStaticSemaphoreDef_t;
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */

/* USER CODE END Variables */
/* Definitions for console_thread */
osThreadId_t console_threadHandle;
uint32_t MyBufferTask01[ 1024 ];
osStaticThreadDef_t MycontrolBlocTask01;
const osThreadAttr_t console_thread_attributes = {
  .name = "console_thread",
  .stack_mem = &MyBufferTask01[0],
  .stack_size = sizeof(MyBufferTask01),
  .cb_mem = &MycontrolBlocTask01,
  .cb_size = sizeof(MycontrolBlocTask01),
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for tcp_cilent_thread */
osThreadId_t tcp_cilent_threadHandle;
uint32_t MyBufferTask02[ 1024 ];
osStaticThreadDef_t MycontrolBlocTask02;
const osThreadAttr_t tcp_cilent_thread_attributes = {
  .name = "tcp_cilent_thread",
  .stack_mem = &MyBufferTask02[0],
  .stack_size = sizeof(MyBufferTask02),
  .cb_mem = &MycontrolBlocTask02,
  .cb_size = sizeof(MycontrolBlocTask02),
  .priority = (osPriority_t) osPriorityLow,
};
/* Definitions for tcp_server_thread */
osThreadId_t tcp_server_threadHandle;
uint32_t MyBufferTask03[ 1024 ];
osStaticThreadDef_t MycontrolBlocTask03;
const osThreadAttr_t tcp_server_thread_attributes = {
  .name = "tcp_server_thread",
  .stack_mem = &MyBufferTask03[0],
  .stack_size = sizeof(MyBufferTask03),
  .cb_mem = &MycontrolBlocTask03,
  .cb_size = sizeof(MycontrolBlocTask03),
  .priority = (osPriority_t) osPriorityLow,
};
/* Definitions for uart1_sem */
osSemaphoreId_t uart1_semHandle;
osStaticSemaphoreDef_t uart1_sem_blk;
const osSemaphoreAttr_t uart1_sem_attributes = {
  .name = "uart1_sem",
  .cb_mem = &uart1_sem_blk,
  .cb_size = sizeof(uart1_sem_blk),
};
/* Definitions for rtc_sem */
osSemaphoreId_t rtc_semHandle;
osStaticSemaphoreDef_t rtc_sem_blk;
const osSemaphoreAttr_t rtc_sem_attributes = {
  .name = "rtc_sem",
  .cb_mem = &rtc_sem_blk,
  .cb_size = sizeof(rtc_sem_blk),
};
/* Definitions for status_sem */
osSemaphoreId_t status_semHandle;
osStaticSemaphoreDef_t status_sem_blk;
const osSemaphoreAttr_t status_sem_attributes = {
  .name = "status_sem",
  .cb_mem = &status_sem_blk,
  .cb_size = sizeof(status_sem_blk),
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */
void main_init()
{
    printu("\n\n\r\n");
    printu(",------. ,--------. ,-----.  ,---.   \r\n");
    printu("|  .--. ''--.  .--''  .-.  ''   .-'  \r\n");
    printu("|  '--'.'   |  |   |  | |  |`.  `-.  \r\n");
    printu("|  |\\  \\    |  |   '  '-'  '.-'    | \r\n");
    printu("`--' '--'   `--'    `-----' `-----'  \n\r\n");
}
/* USER CODE END FunctionPrototypes */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */
  /* creation of uart1_sem */
  uart1_semHandle = osSemaphoreNew(1, 1, &uart1_sem_attributes);

  /* creation of rtc_sem */
  rtc_semHandle = osSemaphoreNew(1, 1, &rtc_sem_attributes);

  /* creation of status_sem */
  status_semHandle = osSemaphoreNew(1, 1, &status_sem_attributes);

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */
  /* creation of console_thread */
  console_threadHandle = osThreadNew(console_thread, NULL, &console_thread_attributes);

  /* creation of tcp_cilent_thread */
  tcp_cilent_threadHandle = osThreadNew(tcp_cilent_thread, NULL, &tcp_cilent_thread_attributes);

  /* creation of tcp_server_thread */
  tcp_server_threadHandle = osThreadNew(tcp_server_thread, NULL, &tcp_server_thread_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  uart_init();
  status_init();
  eth_init();
  main_init();
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

}
/* USER CODE BEGIN Header_console_thread */
/**
* @brief Function implementing the console_thread thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_console_thread */
void console_thread(void *argument)
{
  /* USER CODE BEGIN console_thread */
    prints("~# ");
    /* Infinite loop */
    for(;;)
    {
        cli_proc();
        osDelay(10);
    }
  /* USER CODE END console_thread */
}

/* USER CODE BEGIN Header_tcp_cilent_thread */
/**
* @brief Function implementing the tcp_cilent_thread thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_tcp_cilent_thread */
void tcp_cilent_thread(void *argument)
{
  /* USER CODE BEGIN tcp_cilent_thread */
  /* Infinite loop */
  for(;;)
  {
    tcp_client_work();
    osDelay(1);
  }
  /* USER CODE END tcp_cilent_thread */
}

/* USER CODE BEGIN Header_tcp_server_thread */
/**
* @brief Function implementing the tcp_server_thread thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_tcp_server_thread */
void tcp_server_thread(void *argument)
{
  /* USER CODE BEGIN tcp_server_thread */
  /* Infinite loop */
  for(;;)
  {
    tcp_server_work();
    osDelay(1);
  }
  /* USER CODE END tcp_server_thread */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */

