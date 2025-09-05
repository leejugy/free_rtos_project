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
#include "usart.h"
#include "cli.h"
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
/* Definitions for uart1_sem */
osSemaphoreId_t uart1_semHandle;
osStaticSemaphoreDef_t uart1_sem_blk;
const osSemaphoreAttr_t uart1_sem_attributes = {
  .name = "uart1_sem",
  .cb_mem = &uart1_sem_blk,
  .cb_size = sizeof(uart1_sem_blk),
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

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

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
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

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */

