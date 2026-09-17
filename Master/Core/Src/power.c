/**
  ******************************************************************************
  * @file    power.c
  * @brief   Power mode control
  ******************************************************************************
  */

#include "power.h"
#include "main.h"
#include "stm32wlxx_ll_rcc.h"
#include "smtc_modem_api.h"

#define POWER_PLL_OFF_TIMEOUT  10U

extern void SystemClock_Config(void);

static uint8_t power_lprun = 0;

static void Power_StopHighSpeedClocks(void)
{
  uint32_t start;

  LL_RCC_PLL_Disable();

  start = HAL_GetTick();
  while (LL_RCC_PLL_IsReady() && ((HAL_GetTick() - start) < POWER_PLL_OFF_TIMEOUT))
    ;

  LL_RCC_HSE_Disable();
}

void Power_Init(void)
{
  HAL_PWREx_SMPS_SetMode(PWR_SMPS_STEP_DOWN);
}

void Power_SwitchToLPRunMSI1MHz(void)
{
  RCC_OscInitTypeDef osc = {0};
  RCC_ClkInitTypeDef clk = {0};

  if (power_lprun)
    return;

  (void)smtc_modem_suspend_radio_communications(true);

  osc.OscillatorType      = RCC_OSCILLATORTYPE_MSI;
  osc.MSIState            = RCC_MSI_ON;
  osc.MSICalibrationValue = RCC_MSICALIBRATION_DEFAULT;
  osc.MSIClockRange       = RCC_MSIRANGE_4;
  osc.PLL.PLLState        = RCC_PLL_NONE;
  (void)HAL_RCC_OscConfig(&osc);

  clk.ClockType      = RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK |
                       RCC_CLOCKTYPE_PCLK1  | RCC_CLOCKTYPE_PCLK2 |
                       RCC_CLOCKTYPE_HCLK3;
  clk.SYSCLKSource   = RCC_SYSCLKSOURCE_MSI;
  clk.AHBCLKDivider  = RCC_SYSCLK_DIV1;
  clk.APB1CLKDivider = RCC_HCLK_DIV1;
  clk.APB2CLKDivider = RCC_HCLK_DIV1;
  clk.AHBCLK3Divider = RCC_SYSCLK_DIV1;
  (void)HAL_RCC_ClockConfig(&clk, FLASH_LATENCY_0);

  Power_StopHighSpeedClocks();

  (void)HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE2);
  HAL_PWREx_EnableLowPowerRunMode();

  power_lprun = 1;
}

void Power_SwitchToRunHSE48MHz(void)
{
  uint8_t was_lprun = power_lprun;

  if (was_lprun) {
    (void)HAL_PWREx_DisableLowPowerRunMode();
    power_lprun = 0;
  }

  (void)HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1);
  SystemClock_Config();

  if (was_lprun)
    (void)smtc_modem_suspend_radio_communications(false);
}

void Power_DisableDebug(void)
{
  GPIO_InitTypeDef gpio = {0};

  HAL_DBGMCU_DisableDBGSleepMode();
  HAL_DBGMCU_DisableDBGStopMode();
  HAL_DBGMCU_DisableDBGStandbyMode();

  gpio.Mode = GPIO_MODE_ANALOG;
  gpio.Pull = GPIO_NOPULL;

  gpio.Pin = DEBUG_SWDIO_Pin;
  HAL_GPIO_Init(DEBUG_SWDIO_GPIO_Port, &gpio);

  gpio.Pin = DEBUG_SWCLK_Pin;
  HAL_GPIO_Init(DEBUG_SWCLK_GPIO_Port, &gpio);
}
