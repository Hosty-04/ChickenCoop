/**
  ******************************************************************************
  * @file    power.c
  * @brief   Power mode control and hand-over to the next nest
  ******************************************************************************
  */

#include "power.h"

#define POWER_SWD_PORT  GPIOA
#define POWER_SWD_PINS  (GPIO_PIN_13 | GPIO_PIN_14)

static uint8_t power_lprun = 0;

void Power_LowPowerRun(void)
{
  RCC_OscInitTypeDef osc = {0};

  if (power_lprun)
    return;

  (void)HAL_SetTickFreq(HAL_TICK_FREQ_100HZ);

  osc.OscillatorType      = RCC_OSCILLATORTYPE_MSI;
  osc.MSIState            = RCC_MSI_ON;
  osc.MSICalibrationValue = RCC_MSICALIBRATION_DEFAULT;
  osc.MSIClockRange       = RCC_MSIRANGE_1;
  osc.PLL.PLLState        = RCC_PLL_NONE;

  if (HAL_RCC_OscConfig(&osc) != HAL_OK)
    return;

  HAL_PWREx_EnableLowPowerRunMode();
  power_lprun = 1U;
}

void Power_MainRegulator(void)
{
  if (power_lprun)
    (void)HAL_PWREx_DisableLowPowerRunMode();
}

void Power_LowRegulator(void)
{
  if (power_lprun)
    HAL_PWREx_EnableLowPowerRunMode();
}

void Power_Release(uint8_t next)
{
  GPIO_InitTypeDef gpio = {0};

  HAL_GPIO_WritePin(LOCAL_GPIO_Port, LOCAL_Pin, GPIO_PIN_SET);
  if (next)
    HAL_GPIO_WritePin(NEXT_GPIO_Port, NEXT_Pin, GPIO_PIN_RESET);

  gpio.Pin  = POWER_SWD_PINS;
  gpio.Mode = GPIO_MODE_ANALOG;
  gpio.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(POWER_SWD_PORT, &gpio);

  Power_MainRegulator();
  HAL_SuspendTick();
  HAL_PWREx_EnableUltraLowPower();
  HAL_PWREx_DisableFastWakeUp();

  while (1)
    HAL_PWR_EnterSTOPMode(PWR_LOWPOWERREGULATOR_ON, PWR_STOPENTRY_WFI);
}
