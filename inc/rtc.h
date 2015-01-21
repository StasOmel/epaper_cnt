#ifndef RTC_H_INCLUDED
#define RTC_H_INCLUDED

#include "main.h"

#define RTC_BACKUP_REGNUMBER_SIGNATURE  0
#define RTC_BACKUP_REGNUMBER_COUNTDOWN  1
#define RTC_BACKUP_REGNUMBER_COUNTUP    2


#define RTC_BACKUP_SIGNATURE_VALUE      0x3C5A

void rtcInit(void);
void rtcSetAlarm(void);
void rtcSetDateTime(RTC_DateTypeDef *date, RTC_TimeTypeDef *time);
void rtcGetDateTime(RTC_DateTypeDef *date, RTC_TimeTypeDef *time);
uint32_t rtcBackUpGetValue(uint32_t RegNumber);
void rtcBackUpSetValue(uint32_t RegNumber, uint32_t val);

#endif /* RTC_H_INCLUDED */
