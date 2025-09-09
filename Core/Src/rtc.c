/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    rtc.c
  * @brief   This file provides code for the configuration
  *          of the RTC instances.
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
#include "rtc.h"

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

RTC_HandleTypeDef hrtc;

/* RTC init function */
void MX_RTC_Init(void)
{

  /* USER CODE BEGIN RTC_Init 0 */

  /* USER CODE END RTC_Init 0 */

  RTC_PrivilegeStateTypeDef privilegeState = {0};

  /* USER CODE BEGIN RTC_Init 1 */

  /* USER CODE END RTC_Init 1 */

  /** Initialize RTC Only
  */
  hrtc.Instance = RTC;
  hrtc.Init.HourFormat = RTC_HOURFORMAT_24;
  hrtc.Init.AsynchPrediv = 127;
  hrtc.Init.SynchPrediv = 255;
  hrtc.Init.OutPut = RTC_OUTPUT_DISABLE;
  hrtc.Init.OutPutRemap = RTC_OUTPUT_REMAP_NONE;
  hrtc.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
  hrtc.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;
  hrtc.Init.OutPutPullUp = RTC_OUTPUT_PULLUP_NONE;
  hrtc.Init.BinMode = RTC_BINARY_NONE;
  if (HAL_RTC_Init(&hrtc) != HAL_OK)
  {
    Error_Handler();
  }
  privilegeState.rtcPrivilegeFull = RTC_PRIVILEGE_FULL_NO;
  privilegeState.backupRegisterPrivZone = RTC_PRIVILEGE_BKUP_ZONE_NONE;
  privilegeState.backupRegisterStartZone2 = RTC_BKP_DR0;
  privilegeState.backupRegisterStartZone3 = RTC_BKP_DR0;
  if (HAL_RTCEx_PrivilegeModeSet(&hrtc, &privilegeState) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN RTC_Init 2 */

  /* USER CODE END RTC_Init 2 */

}

void HAL_RTC_MspInit(RTC_HandleTypeDef* rtcHandle)
{

  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};
  if(rtcHandle->Instance==RTC)
  {
  /* USER CODE BEGIN RTC_MspInit 0 */

  /* USER CODE END RTC_MspInit 0 */

  /** Initializes the peripherals clock
  */
    PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_RTC;
    PeriphClkInitStruct.RTCClockSelection = RCC_RTCCLKSOURCE_LSE;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
    {
      Error_Handler();
    }

    /* RTC clock enable */
    __HAL_RCC_RTC_ENABLE();
    __HAL_RCC_RTCAPB_CLK_ENABLE();
  /* USER CODE BEGIN RTC_MspInit 1 */

  /* USER CODE END RTC_MspInit 1 */
  }
}

void HAL_RTC_MspDeInit(RTC_HandleTypeDef* rtcHandle)
{

  if(rtcHandle->Instance==RTC)
  {
  /* USER CODE BEGIN RTC_MspDeInit 0 */

  /* USER CODE END RTC_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_RTC_DISABLE();
    __HAL_RCC_RTCAPB_CLK_DISABLE();
  /* USER CODE BEGIN RTC_MspDeInit 1 */

  /* USER CODE END RTC_MspDeInit 1 */
  }
}

/* USER CODE BEGIN 1 */
static void rtc_tm_to_hal_st(RTC_TimeTypeDef *time, RTC_DateTypeDef *date, struct tm *__tm)
{
    char str[16] = {0, };
    char *end_ptr = NULL;

    sprintf(str, "%d", __tm->tm_year - 1900);
    date->Year = strtoul(str, &end_ptr, 16);
    memset(str, 0, sizeof(str));

    sprintf(str, "%d", __tm->tm_mon + 1);
    date->Month = strtoul(str, &end_ptr, 16);
    memset(str, 0, sizeof(str));

    sprintf(str, "%d", __tm->tm_mday);
    date->Date = strtoul(str, &end_ptr, 16);
    memset(str, 0, sizeof(str));

    date->WeekDay = __tm->tm_wday + 1;

    time->Hours = __tm->tm_hour;
    time->Minutes = __tm->tm_min;
    time->Seconds = __tm->tm_sec % 60;
}

RTC_STATUS rtc_chk_valid_str(char *str, rtc_data_t *rtc_data)
{
    rtc_str_t rtc_str = {0, };
    uint8_t mday[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    int time = 0;
    char *ptr = NULL;

    memcpy(&rtc_str, str, strlen(str));
    if (rtc_str.colon1 != ':' || rtc_str.colon2 != ':')
    {
        return RTC_FORMAT_ERR;
    }

    if (rtc_str.dash1 != '-' || rtc_str.dash2 != '-')
    {
        return RTC_FORMAT_ERR;
    }

    if (rtc_str.space != ' ')
    {
        return RTC_FORMAT_ERR;
    }

    if (strtok_r(&rtc_str, "-: ", &ptr) == NULL)
    {
        return RTC_FORMAT_ERR;
    }

    time = atoi(rtc_str.year);
    if (time > 9999 || time < 2000)
    {
        return RTC_RANGE_ERR;
    }
    rtc_data->tm.tm_year = time - 1900;

    time = atoi(rtc_str.month);
    if (time > 12 || time < 1)
    {
        return RTC_RANGE_ERR;
    }
    rtc_data->tm.tm_mon = time - 1;

    time = atoi(rtc_str.day);
    if (time > mday[rtc_data->date.Month - 1] || time < 1)
    {
        return RTC_RANGE_ERR;
    }
    rtc_data->tm.tm_mday = time;

    time = atoi(rtc_str.hour);
    if (time > 23 || time < 0)
    {
        return RTC_RANGE_ERR;
    }
    rtc_data->tm.tm_hour = time;

    time = atoi(rtc_str.minute);
    if (time > 59 || time < 0)
    {
        return RTC_RANGE_ERR;
    }
    rtc_data->tm.tm_min = time;

    time = atoi(rtc_str.second);
    if (time > 59 || time < 0)
    {
        return RTC_RANGE_ERR;
    }
    rtc_data->tm.tm_sec = time;

    rtc_tm_to_hal_st(&rtc_data->time, &rtc_data->date, &rtc_data->tm);
    return 1;
}
/* USER CODE END 1 */
