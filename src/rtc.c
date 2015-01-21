#include "rtc.h"


/* RTC handler declaration */
RTC_HandleTypeDef RtcHandle;

void rtcInit(void)
{
  /* Configure RTC prescaler and RTC data registers */
  /* RTC configured as follow:
      - Hour Format    = Format 24
      - Asynch Prediv  = Value according to source clock
      - Synch Prediv   = Value according to source clock
      - OutPut         = Output Disable
      - OutPutPolarity = High Polarity
      - OutPutType     = Open Drain */

  RtcHandle.Instance = RTC;
  RtcHandle.Init.HourFormat = RTC_HOURFORMAT_24;
  RtcHandle.Init.AsynchPrediv = RTC_ASYNCH_PREDIV;
  RtcHandle.Init.SynchPrediv = RTC_SYNCH_PREDIV;
  RtcHandle.Init.OutPut = RTC_OUTPUT_DISABLE;
  RtcHandle.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
  RtcHandle.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;

  if(HAL_RTC_Init(&RtcHandle) != HAL_OK)
  {
    /* Error */
    Error_Handler();
  }
}

void rtcSetDateTime(RTC_DateTypeDef *date, RTC_TimeTypeDef *time)
{
  if(HAL_RTC_SetDate(&RtcHandle,date,FORMAT_BIN) != HAL_OK)
  {
    /* Initialization Error */
    Error_Handler();
  }

  if(HAL_RTC_SetTime(&RtcHandle,time,FORMAT_BIN) != HAL_OK)
  {
    /* Error */
    Error_Handler();
  }
}

void rtcGetDateTime(RTC_DateTypeDef *date, RTC_TimeTypeDef *time)
{
  /* Get the RTC current Time */
  HAL_RTC_GetTime(&RtcHandle, time, FORMAT_BIN);
  /* Get the RTC current Date */
  HAL_RTC_GetDate(&RtcHandle, date, FORMAT_BIN);
}


void rtcSetAlarm(void)
{
  RTC_AlarmTypeDef salarmstructure;
  /* Set Alarm to 02:20:30
  RTC Alarm Generation: Alarm on Hours, Minutes and Seconds */
  salarmstructure.Alarm = RTC_ALARM_A;
  salarmstructure.AlarmDateWeekDay = RTC_WEEKDAY_WEDNESDAY;
  salarmstructure.AlarmDateWeekDaySel = RTC_ALARMDATEWEEKDAYSEL_WEEKDAY;
#ifdef DEBUG
  salarmstructure.AlarmMask = RTC_ALARMMASK_DATEWEEKDAY | RTC_ALARMMASK_HOURS | RTC_ALARMMASK_MINUTES;    /* Alarm every minute   */
#else
  salarmstructure.AlarmMask = RTC_ALARMMASK_DATEWEEKDAY;                                                  /* Alarm every day      */
#endif

  salarmstructure.AlarmSubSecondMask = RTC_ALARMSUBSECONDMASK_None;
  salarmstructure.AlarmTime.TimeFormat = RTC_HOURFORMAT12_AM;
  salarmstructure.AlarmTime.Hours = 23;
  salarmstructure.AlarmTime.Minutes = 59;
  salarmstructure.AlarmTime.Seconds = 59;
  salarmstructure.AlarmTime.SubSeconds = 56;

  if(HAL_RTC_SetAlarm_IT(&RtcHandle,&salarmstructure,FORMAT_BIN) != HAL_OK)
  {
    /* Initialization Error */
    Error_Handler();
  }
}

uint32_t rtcBackUpGetValue(uint32_t RegNumber)
{
  return HAL_RTCEx_BKUPRead(&RtcHandle, RegNumber);
}

void rtcBackUpSetValue(uint32_t RegNumber, uint32_t val)
{
  HAL_RTCEx_BKUPWrite(&RtcHandle, RegNumber, val);
}
