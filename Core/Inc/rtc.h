/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    rtc.h
  * @brief   This file contains all the function prototypes for
  *          the rtc.c file
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
#ifndef __RTC_H__
#define __RTC_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* USER CODE BEGIN Includes */
#include <time.h>
#include "app_freertos.h"
/* USER CODE END Includes */

extern RTC_HandleTypeDef hrtc;

/* USER CODE BEGIN Private defines */
typedef enum
{
    RTC_RANGE_ERR = -1,
    RTC_FORMAT_ERR = -2,
    RTC_FUNC_ERR = -3,
    RTC_OKAY = 1,
}RTC_STATUS;


typedef struct
{
    char year[4];
    char dash1;
    char month[2];
    char dash2;
    char day[2];
    char space;
    char hour[2];
    char colon1;
    char minute[2];
    char colon2;
    char second[2];
}rtc_str_t;

typedef struct
{
    RTC_HandleTypeDef *hrtc;
    osSemaphoreId_t *sem;
}rtc_t;

/* USER CODE END Private defines */

void MX_RTC_Init(void);

/* USER CODE BEGIN Prototypes */
RTC_STATUS rtc_string_to_tm(char *str, struct tm *__tm);
RTC_STATUS rtc_set_time(struct tm *__tm);
RTC_STATUS rtc_get_time(struct tm *__tm);
/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif

#endif /* __RTC_H__ */

