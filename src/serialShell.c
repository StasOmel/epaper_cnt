#include "serialShell.h"
#include <stdio.h>

UART_HandleTypeDef UartHandle;
uint32_t USART_Scanf(uint32_t numbers_cnt, uint32_t min, uint32_t max);

void serialShellInit(void)
{
  /* Put the USART peripheral in the Asynchronous mode (UART Mode) */
  /* UART1 configured as follow:
      - Word Length = 8 Bits
      - Stop Bit = One Stop bit
      - Parity = None
      - BaudRate = 9600 baud
      - Hardware flow control disabled (RTS and CTS signals) */
  UartHandle.Instance        = USARTx;
  UartHandle.Init.BaudRate   = 9600;
  UartHandle.Init.WordLength = UART_WORDLENGTH_8B;
  UartHandle.Init.StopBits   = UART_STOPBITS_1;
  UartHandle.Init.Parity     = UART_PARITY_NONE;
  UartHandle.Init.HwFlowCtl  = UART_HWCONTROL_NONE;
  UartHandle.Init.Mode       = UART_MODE_TX_RX;

  if(HAL_UART_Init(&UartHandle) != HAL_OK)
  {
    Error_Handler();
  }

}

void serialShellGetDateTime(RTC_DateTypeDef *date, RTC_TimeTypeDef *time)
{
  uint32_t year = 0xFFF, month = 0xFFF, day = 0xFFF, hour = 0xFFF, minute = 0xFFF, second = 0xFFF;

  printf("\r\n==============Date Settings=====================================");
  printf("\r\n  Please Set Year (00..99)");
  printf("\r\n");
  while (year == 0xFFF)
  {
    year = USART_Scanf(2, 0, 99);
  }
  printf(":  %d", year);

  printf("\r\n  Please Set Month (01..12)");
  printf("\r\n");
  while (month == 0xFFF)
  {
    month = USART_Scanf(2, 1, 12);
  }
  printf(":  %d", month);

  printf("\r\n  Please Set Day (01..31)");
  printf("\r\n");
  while (day == 0xFFF)
  {
    day = USART_Scanf(2, 1, 31);
  }
  printf(":  %d", day);
  printf("\r\n");

  date->Year = year;
  date->Month = month;
  date->Date = day;

  printf("\r\n==============Time Settings=====================================");
  printf("\r\n  Please Set Hours");
  printf("\r\n");
  while (hour == 0xFFF)
  {
    hour = USART_Scanf(2, 0, 23);
  }
  printf(":  %d", hour);

  printf("\r\n  Please Set Minutes");
  printf("\r\n");
  while (minute == 0xFFF)
  {
    minute = USART_Scanf(2, 0, 59);
  }
  printf(":  %d", minute);

  printf("\r\n  Please Set Seconds");
  printf("\r\n");

  while (second == 0xFFF)
  {
    second = USART_Scanf(2, 0, 59);
  }
  printf(":  %d", second);
  printf("\r\n");

  time->Hours = hour;
  time->Minutes = minute;
  time->Seconds = second;
  time->TimeFormat = RTC_HOURFORMAT12_AM;
  time->DayLightSaving = RTC_DAYLIGHTSAVING_NONE ;
  time->StoreOperation = RTC_STOREOPERATION_RESET;
}

void serialShellGetCountdown(uint32_t *val)
{
  uint32_t days = 0xFFF;
  printf("\r\n==============Countdown Settings================================");
  printf("\r\n  Please set number of days to countdown (000..999)");
  printf("\r\n");
  while (days == 0xFFF)
  {
    days = USART_Scanf(3, 0, 999);
  }
  printf(":  %d", days);
  printf("\r\n");
  *val = days;
}

int _write(int file, char *ptr, int len)
	{
        (void)file;
	    HAL_UART_Transmit(&UartHandle, ptr, len, 10000);
	    return len;
	}

/**
  * @brief  Gets numeric values from the hyperterminal.
  * @param  None
  * @retval None
  */
uint32_t USART_Scanf(uint32_t numbers_cnt, uint32_t min, uint32_t max)
{
  uint32_t index = 0;
  uint8_t tmp[3] = {0x30, 0x30, 0x30};

  while (index < numbers_cnt)
  {
    while (HAL_UART_Receive(&UartHandle, &tmp[index], 1, 1000000) != HAL_OK) {};

    index++;
    if ((tmp[index - 1] < 0x30) || (tmp[index - 1] > 0x39))
    {
      printf("\n\rPlease enter valid number between 0 and 9");
      printf("\r\n");
      index--;
    }
  }
  /* Calculate the Corresponding value */
  if (numbers_cnt == 1) index = (tmp[0] - 0x30);
  else if (numbers_cnt == 2) index = (tmp[1] - 0x30) + ((tmp[0] - 0x30) * 10);
  else if (numbers_cnt == 3) index = (tmp[2] - 0x30) + ((tmp[1] - 0x30) * 10) + ((tmp[0] - 0x30) * 100);
  /* Checks */
  if (index < min || index > max)
  {
    printf("\n\rPlease enter valid number between %d and %d", min, max);
    printf("\r\n");
    return 0xFFF;
  }
  return index;
}

uint32_t serialShellCheckEnter(void)
{
  HAL_StatusTypeDef result;
  uint8_t tmp = 0xFF;
  printf("\r\n  Please <Enter> within 5 seconds to adjust date/time and counter");
  printf("\r\n");

  while (1) {
    result = HAL_UART_Receive(&UartHandle, &tmp, 1, 5000);
    if (result == HAL_OK || result == HAL_TIMEOUT) break;
  }

  if (tmp == 0x0D) return 1;
  else return 0;
}
