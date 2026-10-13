/**
  ******************************************************************************
  * @file    power.c
  * @brief   Power mode control
  ******************************************************************************
  */

#include "power.h"
#include "main.h"
#include "timebase.h"
#include "stm32wlxx_ll_rcc.h"
#include "stm32wlxx_ll_bus.h"
#include "stm32wlxx_ll_tim.h"
#include "smtc_modem_api.h"

#define POWER_PLL_OFF_MS        10U

#define POWER_MSI_EDGES         64U
#define POWER_MSI_PERIODS       (POWER_MSI_EDGES * 8U)
#define POWER_MSI_TRIES         3U
#define POWER_MSI_TIMEOUT_MS    50U
#define POWER_MSI_TOLERANCE     5U
#define POWER_PERMILLE          1000U

extern void SystemClock_Config(void);

static uint8_t power_lprun = 0;

static void Power_StopHighSpeedClocks(void)
{
  uint32_t start;

  LL_RCC_PLL_Disable();

  start = HAL_GetTick();
  while (LL_RCC_PLL_IsReady() && (TICKS_TO_MS(HAL_GetTick() - start) < POWER_PLL_OFF_MS))
    ;

  LL_RCC_HSE_Disable();
}

void Power_Init(void)
{
  HAL_PWREx_SMPS_SetMode(PWR_SMPS_STEP_DOWN);
}

void Power_SwitchToLPRunMSI2MHz(void)
{
  RCC_OscInitTypeDef osc = {0};
  RCC_ClkInitTypeDef clk = {0};

  if (power_lprun)
    return;

  (void)smtc_modem_suspend_radio_communications(true);

  osc.OscillatorType      = RCC_OSCILLATORTYPE_MSI;
  osc.MSIState            = RCC_MSI_ON;
  osc.MSICalibrationValue = RCC_MSICALIBRATION_DEFAULT;
  osc.MSIClockRange       = RCC_MSIRANGE_5;
  osc.PLL.PLLState        = RCC_PLL_NONE;

  if (HAL_RCC_OscConfig(&osc) != HAL_OK) {
    (void)smtc_modem_suspend_radio_communications(false);
    return;
  }

  clk.ClockType      = RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK |
                       RCC_CLOCKTYPE_PCLK1  | RCC_CLOCKTYPE_PCLK2 |
                       RCC_CLOCKTYPE_HCLK3;
  clk.SYSCLKSource   = RCC_SYSCLKSOURCE_MSI;
  clk.AHBCLKDivider  = RCC_SYSCLK_DIV1;
  clk.APB1CLKDivider = RCC_HCLK_DIV1;
  clk.APB2CLKDivider = RCC_HCLK_DIV1;
  clk.AHBCLK3Divider = RCC_SYSCLK_DIV1;

  if (HAL_RCC_ClockConfig(&clk, FLASH_LATENCY_0) != HAL_OK) {
    (void)smtc_modem_suspend_radio_communications(false);
    return;
  }

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

static uint8_t Power_MeasureMSI(uint32_t expected)
{
  uint32_t start = HAL_GetTick();
  uint32_t cycles;
  uint16_t first = 0U;
  uint16_t last  = 0U;
  uint8_t  edges = 0U;

  LL_TIM_ClearFlag_CC1(TIM16);

  while (edges <= POWER_MSI_EDGES) {
    if (LL_TIM_IsActiveFlag_CC1(TIM16)) {
      last = (uint16_t)LL_TIM_IC_GetCaptureCH1(TIM16);
      if (edges == 0U) {
        first = last;
        LL_TIM_ClearFlag_CC1OVR(TIM16);
      }
      edges++;
    } else if (TICKS_TO_MS(HAL_GetTick() - start) > POWER_MSI_TIMEOUT_MS) {
      return 0U;
    }
  }

  if (LL_TIM_IsActiveFlag_CC1OVR(TIM16))
    return 0U;

  cycles = (uint16_t)(last - first);
  cycles = (cycles > expected) ? (cycles - expected) : (expected - cycles);

  return (uint8_t)((cycles * POWER_PERMILLE) <= (expected * POWER_MSI_TOLERANCE));
}

uint8_t Power_LockMSI(void)
{
  uint32_t expected = (uint32_t)(((uint64_t)HAL_RCC_GetPCLK2Freq() * POWER_MSI_PERIODS) / LSE_VALUE);
  uint8_t  locked   = 0U;
  uint8_t  attempt;

  if (!power_lprun || !LL_RCC_LSE_IsReady() || (expected > UINT16_MAX))
    return 0U;

  LL_RCC_MSI_EnablePLLMode();

  LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_TIM16);
  LL_TIM_SetRemap(TIM16, LL_TIM_TIM16_TI1_RMP_LSE);
  LL_TIM_IC_SetActiveInput(TIM16, LL_TIM_CHANNEL_CH1, LL_TIM_ACTIVEINPUT_DIRECTTI);
  LL_TIM_IC_SetPrescaler(TIM16, LL_TIM_CHANNEL_CH1, LL_TIM_ICPSC_DIV8);
  LL_TIM_CC_EnableChannel(TIM16, LL_TIM_CHANNEL_CH1);
  LL_TIM_EnableCounter(TIM16);

  for (attempt = 0U; (attempt < POWER_MSI_TRIES) && !locked; attempt++)
    locked = Power_MeasureMSI(expected);

  LL_TIM_DisableCounter(TIM16);
  LL_APB2_GRP1_ForceReset(LL_APB2_GRP1_PERIPH_TIM16);
  LL_APB2_GRP1_ReleaseReset(LL_APB2_GRP1_PERIPH_TIM16);
  LL_APB2_GRP1_DisableClock(LL_APB2_GRP1_PERIPH_TIM16);

  return locked;
}

void Power_UnlockMSI(void)
{
  LL_RCC_MSI_DisablePLLMode();
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
