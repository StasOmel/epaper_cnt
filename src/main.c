/* Includes ------------------------------------------------------------------*/
#include "stm32l0xx_hal.h"
#include "main.h"
#include "bsp.h"
#include "stm32l0xx.h"
#include "rtc.h"
#include "serialShell.h"
#include <stdio.h>

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
RTC_DateTypeDef datestructure;
RTC_TimeTypeDef timestructure;
uint32_t countdown_days=0, countup_days=0;
/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
void DateTimeShow(void);
void ScreenUpdate(void);
void SystemPower_Config(void);
void SystemGPIO_Config(void);
/* Private functions ---------------------------------------------------------*/

/**
  * @brief  Main program
  * @param  None
  * @retval None
  */
int main(void)
{
    /* STM32L0xx HAL library initialization:
    - Configure the Flash prefetch, Flash preread and Buffer caches
    - Configure the Systick to generate an interrupt each 1 msec
    - Low Level Initialization
    */
    HAL_Init();

    /* Configure the System clock to have a frequency of 32 MHz */
    SystemClock_Config();

    /* Initialize the BSP features LEDs, Buttons, EPD and LTS */
    Bsp_Init();

    rtcInit();

    rtcSetAlarm();

    if (rtcBackUpGetValue(RTC_BACKUP_REGNUMBER_SIGNATURE) != RTC_BACKUP_SIGNATURE_VALUE)    /* Check cold or warm start up  */
    {
        serialShellInit();

        BSP_EPD_Clear(EPD_COLOR_WHITE);
        BSP_EPD_SetFont(&Font20);
        BSP_EPD_DisplayStringAt(1, 11, (uint8_t*)"Please set ", LEFT_MODE);
        BSP_EPD_DisplayStringAt(1, 6, (uint8_t*)"date / time", LEFT_MODE);
        BSP_EPD_DisplayStringAt(1, 1, (uint8_t*)"via terminal!", LEFT_MODE);
        BSP_EPD_RefreshDisplay();

        serialShellGetDateTime(&datestructure, &timestructure);
        rtcSetDateTime(&datestructure, &timestructure);

        serialShellGetCountdown(&countdown_days);

        rtcBackUpSetValue(RTC_BACKUP_REGNUMBER_COUNTDOWN, countdown_days);
        rtcBackUpSetValue(RTC_BACKUP_REGNUMBER_COUNTUP, countup_days);

        rtcBackUpSetValue(RTC_BACKUP_REGNUMBER_SIGNATURE, RTC_BACKUP_SIGNATURE_VALUE);
    }
    else
    {
        if(__HAL_PWR_GET_FLAG(PWR_FLAG_WU) == SET)          /* Check if was some wakeup event                               */
        {                                                   /* Wake up because of RTC alarm or wake up pin (User button)    */
            rtcGetDateTime(&datestructure, &timestructure);
                                                            /* If midnight right now -> RTC alarm                           */
#ifdef DEBUG
            if (timestructure.Seconds == 59)                /* wakeup every minute for debug    */
#else
            if (timestructure.Hours == 23 && timestructure.Minutes == 59 && timestructure.Seconds == 59)   /* every midnight */
#endif
            {
                countdown_days = rtcBackUpGetValue(RTC_BACKUP_REGNUMBER_COUNTDOWN);
                if (countdown_days != 0) countdown_days--;
                rtcBackUpSetValue(RTC_BACKUP_REGNUMBER_COUNTDOWN, countdown_days);

                countup_days = rtcBackUpGetValue(RTC_BACKUP_REGNUMBER_COUNTUP);
                countup_days++;
                rtcBackUpSetValue(RTC_BACKUP_REGNUMBER_COUNTUP, countup_days);
            }
            else                                            /* else -> User button  */
            {
                countup_days=0;
                rtcBackUpSetValue(RTC_BACKUP_REGNUMBER_COUNTUP, countup_days);
            }
        }
        else
        {                                                   /* reset for time display and/or reconfig     */
            DateTimeShow();
            HAL_Delay(5000);
            serialShellInit();
            if (serialShellCheckEnter())
            {
                serialShellGetDateTime(&datestructure, &timestructure);
                rtcSetDateTime(&datestructure, &timestructure);

                serialShellGetCountdown(&countdown_days);

                rtcBackUpSetValue(RTC_BACKUP_REGNUMBER_COUNTDOWN, countdown_days);
            }
        }
    }

    countdown_days = rtcBackUpGetValue(RTC_BACKUP_REGNUMBER_COUNTDOWN);
    countup_days = rtcBackUpGetValue(RTC_BACKUP_REGNUMBER_COUNTUP);
    ScreenUpdate();

    SystemPower_Config();

    /*The Following Wakeup sequence is highly recommended prior to each Standby mode entry
     mainly  when using more than one wakeup source this is to not miss any wakeup event.
       - Disable all used wakeup sources,
       - Clear all related wakeup flags,
       - Re-enable all used wakeup sources,
       - Enter the Standby mode.
     */
    /*Disable all used wakeup sources: Pin1(PA.0)*/
    HAL_PWR_DisableWakeUpPin(PWR_WAKEUP_PIN1);

    /*Clear all related wakeup flags*/
    __HAL_PWR_CLEAR_FLAG(PWR_FLAG_WU);

    /*Re-enable all used wakeup sources: Pin1(PA.0)*/
    HAL_PWR_EnableWakeUpPin(PWR_WAKEUP_PIN1);

    /*Enter the Standby mode*/
    HAL_PWR_EnterSTANDBYMode();

    while (1)
    {
    }
}

void DateTimeShow(void)
{
    uint8_t DateTimeStr[50] = {0};

    rtcGetDateTime(&datestructure, &timestructure);

    sprintf((char*)DateTimeStr,"%02d-%02d-%02d  %02d:%02d:%02d",datestructure.Date, datestructure.Month, datestructure.Year, timestructure.Hours, timestructure.Minutes, timestructure.Seconds);

    BSP_EPD_Clear(EPD_COLOR_WHITE);
    BSP_EPD_SetFont(&Font12);
    BSP_EPD_DisplayStringAt(2, 14, "Current date/time:", LEFT_MODE);
    BSP_EPD_DisplayStringAt(2, 10, DateTimeStr, LEFT_MODE);
    BSP_EPD_DisplayStringAt(2, 5, "Use terminal to adjust", LEFT_MODE);
    BSP_EPD_DisplayStringAt(2, 1, "date/time or counter.", LEFT_MODE);
    BSP_EPD_RefreshDisplay();
}

void ScreenUpdate(void)
{
    uint8_t countdown_days_str[15]= {0}, countup_days_str[20]= {0};

    sprintf((char*)countdown_days_str, "%3d days left", countdown_days);
    sprintf((char*)countup_days_str, "%3d days by now", countup_days);

    BSP_EPD_Clear(EPD_COLOR_WHITE);
    BSP_EPD_SetFont(&Font16);
    BSP_EPD_DisplayStringAt(1, 10, countdown_days_str, LEFT_MODE);
    BSP_EPD_DisplayStringAt(0, 2, countup_days_str, LEFT_MODE);
    BSP_EPD_RefreshDisplay();
}


/**
  * @brief  System Clock Configuration
  *         The system Clock is configured as follow :
  *            System Clock source            = PLL (HSI)
  *            SYSCLK(Hz)                     = 32000000
  *            HCLK(Hz)                       = 32000000
  *            AHB Prescaler                  = 1
  *            APB1 Prescaler                 = 1
  *            APB2 Prescaler                 = 1
  *            HSI Frequency(Hz)              = 16000000
  *            PLL_MUL                        = 4
  *            PLL_DIV                        = 2
  *            Flash Latency(WS)              = 1
  *            Main regulator output voltage  = Scale1 mode
  * @param  None
  * @retval None
  */
void SystemClock_Config(void)
{
    RCC_ClkInitTypeDef RCC_ClkInitStruct;
    RCC_OscInitTypeDef RCC_OscInitStruct;

    /* Enable Power Control clock */
    __PWR_CLK_ENABLE();

    /* The voltage scaling allows optimizing the power consumption when the device is
       clocked below the maximum system frequency, to update the voltage scaling value
       regarding system frequency refer to product datasheet.  */
    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

    /* Enable HSI Oscillator and activate PLL with HSI as source */
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
    RCC_OscInitStruct.HSEState = RCC_HSE_OFF;
    RCC_OscInitStruct.HSIState = RCC_HSI_ON;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
    RCC_OscInitStruct.PLL.PLLMUL = RCC_PLLMUL_4;
    RCC_OscInitStruct.PLL.PLLDIV = RCC_PLLDIV_2;
    RCC_OscInitStruct.HSICalibrationValue = 0x10;
    if(HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
    {
        Error_Handler();
    }

    /* Select PLL as system clock source and configure the HCLK, PCLK1 and PCLK2
       clocks dividers */
    RCC_ClkInitStruct.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2);
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
    if(HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
    {
        Error_Handler();
    }
}


void SystemPower_Config(void)
{
    /* Enable Ultra low power mode */
    HAL_PWREx_EnableUltraLowPower();

    /* Enable the fast wake up from Ultra low power mode */
    HAL_PWREx_EnableFastWakeUp();
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @param  None
  * @retval None
  */
void Error_Handler(void)
{
    BSP_LED_Init(LED4);
    while(1)
    {
        /* Turn LED3 on */
        BSP_LED_Toggle(LED4);
        /* Add a 100ms Delay */
        HAL_Delay(100);
    }
}
