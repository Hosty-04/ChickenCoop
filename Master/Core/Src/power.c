/**
  ******************************************************************************
  * @file    power.c
  * @brief   Power mode switching
  ******************************************************************************
  */

#include "power.h"
#include "main.h"

extern void SystemClock_Config(void);

static void Power_UpdateTimebase(void)
{
  HAL_SYSTICK_Config(SystemCoreClock / 1000U);
  HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);
}

static void Power_ConfigLowPower(void)
{
  HAL_PWREx_SMPS_SetMode(PWR_SMPS_STEP_DOWN);
  HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE2);
}

static void Power_ConfigMSI(uint32_t msi_range)
{
  RCC_OscInitTypeDef osc = {0};
  RCC_ClkInitTypeDef clk = {0};

  osc.OscillatorType = RCC_OSCILLATORTYPE_MSI;
  osc.MSIState = RCC_MSI_ON;
  osc.MSICalibrationValue = RCC_MSICALIBRATION_DEFAULT;
  osc.MSIClockRange = msi_range;
  osc.PLL.PLLState = RCC_PLL_NONE;
  HAL_RCC_OscConfig(&osc);

  clk.ClockType = RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK |
                  RCC_CLOCKTYPE_PCLK1  | RCC_CLOCKTYPE_PCLK2 |
                  RCC_CLOCKTYPE_HCLK3;
  clk.SYSCLKSource   = RCC_SYSCLKSOURCE_MSI;
  clk.AHBCLKDivider  = RCC_SYSCLK_DIV1;
  clk.APB1CLKDivider = RCC_HCLK_DIV1;
  clk.APB2CLKDivider = RCC_HCLK_DIV1;
  clk.AHBCLK3Divider = RCC_SYSCLK_DIV1;
  HAL_RCC_ClockConfig(&clk, FLASH_LATENCY_0);
}

void Power_SwitchToLPRunMSI1MHz(void)
{
  Power_ConfigMSI(RCC_MSIRANGE_4);

  Power_ConfigLowPower();
  Power_UpdateTimebase();
}

void Power_SwitchToRunHSE48MHz(void)
{
  HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1);
  SystemClock_Config();
  Power_UpdateTimebase();
}