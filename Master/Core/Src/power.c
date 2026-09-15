/**
  ******************************************************************************
  * @file    power.c
  * @brief   Power mode control
  ******************************************************************************
  */

#include "power.h"
#include "main.h"

extern void SystemClock_Config(void);

static uint8_t power_lprun = 0;

static void Power_ConfigMSI1MHz(void)
{
  RCC_OscInitTypeDef osc = {0};
  RCC_ClkInitTypeDef clk = {0};

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
}

void Power_SwitchToRunHSE48MHz(void)
{
  if (power_lprun) {
    (void)HAL_PWREx_DisableLowPowerRunMode();
    power_lprun = 0;
  }

  (void)HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1);

  SystemClock_Config();
}

void Power_SwitchToLPRunMSI1MHz(void)
{
  if (power_lprun)
    return;

  Power_ConfigMSI1MHz();

  (void)HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE2);
  HAL_PWREx_SMPS_SetMode(PWR_SMPS_STEP_DOWN);

  HAL_PWREx_EnableLowPowerRunMode();

  power_lprun = 1;
}

uint8_t Power_IsLowPowerRun(void)
{
  return power_lprun;
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