#ifndef SERIALSHELL_H_INCLUDED
#define SERIALSHELL_H_INCLUDED

#include "main.h"

void serialShellInit(void);
void serialShellGetDateTime(RTC_DateTypeDef *date, RTC_TimeTypeDef *time);
uint32_t serialShellCheckEnter(void);
void serialShellGetCountdown(uint32_t *val);
#endif /* SERIALSHELL_H_INCLUDED */
