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
    PeriphClkInitStruct.RTCClockSelection = RCC_RTCCLKSOURCE_LSI;
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
rtc_t rtc = 
{
    .hrtc = &hrtc,
    .sem = &rtc_semHandle
};

uint8_t mday[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

static void rtc_tm_to_hal_st(RTC_TimeTypeDef *time, RTC_DateTypeDef *date, struct tm *__tm)
{
    date->Year = bcd_2_hex(__tm->tm_year - 100);
    date->Month = bcd_2_hex(__tm->tm_mon + 1);
    date->Date = bcd_2_hex(__tm->tm_mday);
    date->WeekDay = bcd_2_hex(__tm->tm_wday + 1);

    time->Hours = bcd_2_hex(__tm->tm_hour);
    time->Minutes = bcd_2_hex(__tm->tm_min);
    time->Seconds = bcd_2_hex(__tm->tm_sec % 60);
}

static void rtc_hal_st_to_tm(RTC_TimeTypeDef *time, RTC_DateTypeDef *date, struct tm *__tm)
{
    __tm->tm_year = hex_2_bcd(date->Year) + 100;
    __tm->tm_mon = hex_2_bcd(date->Month) - 1;
    __tm->tm_mday = hex_2_bcd(date->Date);
    __tm->tm_wday = hex_2_bcd(date->WeekDay - 1);

    __tm->tm_hour = hex_2_bcd(time->Hours);
    __tm->tm_min = hex_2_bcd(time->Minutes);
    __tm->tm_sec = hex_2_bcd(time->Seconds);
}

RTC_STATUS rtc_string_to_tm(char *str, struct tm *__tm)
{
    rtc_str_t rtc_str = {0, };
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

    if (strtok_r((char *)&rtc_str, "-: ", &ptr) == NULL)
    {
        return RTC_FORMAT_ERR;
    }

    time = atoi(rtc_str.year);
    if (time > 9999 || time < 2000)
    {
        return RTC_RANGE_ERR;
    }
    __tm->tm_year = time - 1900;

    time = atoi(rtc_str.month);
    if (time > 12 || time < 1)
    {
        return RTC_RANGE_ERR;
    }
    __tm->tm_mon = time - 1;

    time = atoi(rtc_str.day);
    if (time > mday[__tm->tm_mon] || time < 1)
    {
        return RTC_RANGE_ERR;
    }
    __tm->tm_mday = time;

    time = atoi(rtc_str.hour);
    if (time > 23 || time < 0)
    {
        return RTC_RANGE_ERR;
    }
    __tm->tm_hour = time;

    time = atoi(rtc_str.minute);
    if (time > 59 || time < 0)
    {
        return RTC_RANGE_ERR;
    }
    __tm->tm_min = time;

    time = atoi(rtc_str.second);
    if (time > 59 || time < 0)
    {
        return RTC_RANGE_ERR;
    }
    __tm->tm_sec = time;
    return RTC_OKAY;
}

RTC_STATUS rtc_set_time(struct tm *__tm)
{
    RTC_STATUS ret = RTC_OKAY;
    RTC_TimeTypeDef tim = {0, };
    RTC_DateTypeDef date = {0, };

    rtc_tm_to_hal_st(&tim, &date, __tm);
    if (hex_2_bcd(date.Year) > 99)
    {
        return RTC_RANGE_ERR;
    }
    if (hex_2_bcd(date.Month) < 1 || hex_2_bcd(date.Month) > 12)
    {
        return RTC_RANGE_ERR;
    }
    if (hex_2_bcd(date.Date) < 1 || hex_2_bcd(date.Date) > mday[hex_2_bcd(date.Month) - 1])
    {
        return RTC_RANGE_ERR;
    }
    if (hex_2_bcd(tim.Hours) > 23 || hex_2_bcd(tim.Hours) < 0)
    {
        return RTC_RANGE_ERR;
    }
    if (hex_2_bcd(tim.Minutes) > 59 || hex_2_bcd(tim.Minutes) < 0)
    {
        return RTC_RANGE_ERR;
    }
    if (hex_2_bcd(tim.Seconds) > 59 || hex_2_bcd(tim.Seconds) < 0)
    {
        return RTC_RANGE_ERR;
    }

    sem_wait(rtc.sem);
    if (HAL_RTC_SetDate(rtc.hrtc, &date, RTC_FORMAT_BCD) != HAL_OK)
    {
        ret = RTC_FUNC_ERR;
        goto sem_out;
    }
    if (HAL_RTC_SetTime(rtc.hrtc, &tim, RTC_FORMAT_BCD) != HAL_OK)
    {
        ret = RTC_FUNC_ERR;
        goto sem_out;
    }   
sem_out:
    sem_post(rtc.sem);
    return ret;
}

RTC_STATUS rtc_get_time(struct tm *__tm)
{
    RTC_STATUS ret = RTC_OKAY;
    RTC_TimeTypeDef tim = {0, };
    RTC_DateTypeDef date = {0, };

    sem_wait(rtc.sem);
    if (HAL_RTC_GetTime(rtc.hrtc, &tim, RTC_FORMAT_BCD) != HAL_OK)
    {
        ret = RTC_FUNC_ERR;
        goto sem_out;
    }
    if (HAL_RTC_GetDate(rtc.hrtc, &date, RTC_FORMAT_BCD) != HAL_OK)
    {
        ret = RTC_FUNC_ERR;
        goto sem_out;
    }
sem_out:
    sem_post(rtc.sem);
    rtc_hal_st_to_tm(&tim, &date, __tm);
    return ret;
}
/* USER CODE END 1 */
