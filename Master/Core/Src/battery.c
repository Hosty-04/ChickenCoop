/**
  ******************************************************************************
  * @file    battery.c
  * @brief   Battery/panel monitoring (INA226 + divider)
  ******************************************************************************
  */

#include "door.h"
#include "battery.h"
#include "ina226.h"
#include "timebase.h"
#include "power.h"
#include "stm32_timer.h"
#include "adc.h"
#include "gpio.h"

#define BATTERY_CHECK_MS        (10UL * 60UL * 1000UL)

#define PANEL_V_HYST  0.05f

#define PANEL_DIV_R1  970000.0f
#define PANEL_DIV_R2  488500.0f
#define PANEL_DIVIDER_RATIO  ((PANEL_DIV_R1 + PANEL_DIV_R2) / PANEL_DIV_R2)

#define ADC_VREF        3.3f
#define ADC_FULL_SCALE  4095.0f

#define SEP_PORT  GPIOB
#define SEP_PIN   GPIO_PIN_7

static UTIL_TIMER_Object_t battery_timer;
static uint32_t            battery_armed_s = 0;
static volatile uint8_t    pending_check   = 0;

static uint8_t  ov_lockout      = 0;
static float    v_at_lockout    = 0.0f;

static uint8_t  backfeed_block  = 0;

static uint8_t  critical_mode   = 0;

static void Battery_SetPanel(uint8_t connected)
{
  HAL_GPIO_WritePin(SEP_PORT, SEP_PIN, connected ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

static float Battery_SeasonLimit(uint8_t month)
{
  if (month == 6 || month == 7 || month == 8)  return 7.2f;
  if (month == 12 || month == 1 || month == 2) return 7.5f;
  return 7.3f;
}

static float Battery_CriticalLimit(uint8_t month)
{
  if (month == 11 || month == 12 || month == 1 || month == 2 || month == 3)
    return 6.15f;
  return 6.0f;
}

static HAL_StatusTypeDef Battery_ReadPanelVoltage(float *v_panel)
{
  ADC_ChannelConfTypeDef sConfig = {0};

  MX_ADC_Init();

  if (HAL_ADCEx_Calibration_Start(&hadc) != HAL_OK) return HAL_ERROR;

  sConfig.Channel      = ADC_CHANNEL_2;
  sConfig.Rank         = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLINGTIME_COMMON_1;
  if (HAL_ADC_ConfigChannel(&hadc, &sConfig) != HAL_OK) return HAL_ERROR;

  if (HAL_ADC_Start(&hadc) != HAL_OK) return HAL_ERROR;
  if (HAL_ADC_PollForConversion(&hadc, 10) != HAL_OK) {
    HAL_ADC_Stop(&hadc);
    HAL_ADC_DeInit(&hadc);
    return HAL_ERROR;
  }

  uint32_t raw = HAL_ADC_GetValue(&hadc);
  HAL_ADC_Stop(&hadc);
  HAL_ADC_DeInit(&hadc);

  *v_panel = (float)raw * (ADC_VREF / ADC_FULL_SCALE) * PANEL_DIVIDER_RATIO;
  return HAL_OK;
}

static void Battery_UpdateOvLockout(float v_bat, uint8_t month)
{
  if (!ov_lockout) {
    if (v_bat >= Battery_SeasonLimit(month)) {
      ov_lockout      = 1;
      v_at_lockout    = v_bat;
    }
    return;
  }

  if (v_bat <= (v_at_lockout)) {
    ov_lockout      = 0;
  }
}

static void Battery_UpdateCritical(float v_bat, uint8_t month)
{
  float limit = Battery_CriticalLimit(month);

  if (!critical_mode && v_bat <= limit) {
    critical_mode = 1;
    Door_Reschedule();
  } else if (critical_mode && v_bat > limit) {
    critical_mode = 0;
    Door_Reschedule();
  }
}

static void Battery_UpdateBackfeed(float v_bat, float v_panel)
{
  if (!backfeed_block) {
    if (v_panel < v_bat - PANEL_V_HYST)
      backfeed_block = 1;
  } else {
    if (v_panel > v_bat - PANEL_V_HYST)
      backfeed_block = 0;
  }
}

static void Battery_OnTimer(void *ctx)
{
  UNUSED(ctx);
  Timebase_AdvanceSeconds(battery_armed_s);
  pending_check = 1;
}

static void Battery_Schedule(void)
{
  uint32_t now = Timebase_GetSecOfDay();
  uint32_t interval_s = BATTERY_CHECK_MS / 1000UL;
  uint32_t next_mark = ((now / interval_s) + 1UL) * interval_s;

  if ((next_mark % 86400UL) == 0UL)
    next_mark += interval_s;

  uint32_t delay_s = next_mark - now;
  if (delay_s == 0UL)
    delay_s = 1UL;

  battery_armed_s = delay_s;

  UTIL_TIMER_Stop(&battery_timer);
  UTIL_TIMER_SetPeriod(&battery_timer, delay_s * 1000UL);
  UTIL_TIMER_Start(&battery_timer);
}

void Battery_Init(void)
{
  Battery_SetPanel(1);

  UTIL_TIMER_Create(&battery_timer, 0xFFFFFFFFU, UTIL_TIMER_ONESHOT,
                    Battery_OnTimer, NULL);
  Battery_Schedule();
}

uint8_t Battery_IsCritical(void)
{
  return critical_mode;
}

void Battery_Process(void)
{
  if (!pending_check)
    return;
  pending_check = 0;

  Power_SwitchToLPRunMSI1MHz();

  float v_bat = 0.0f;
  INA226_PowerUp();
  INA226_ConfigBattery();
  HAL_Delay(75);   /* wait until averaging is done */
  HAL_StatusTypeDef st = INA226_Read(&v_bat, NULL);
  INA226_PowerDown();

  float v_panel = 0.0f;
  HAL_StatusTypeDef st_panel = Battery_ReadPanelVoltage(&v_panel);

  if (st == HAL_OK) {
    uint8_t month = Timebase_GetMonth();

    Battery_UpdateOvLockout(v_bat, month);
    Battery_UpdateCritical(v_bat, month);

    if (st_panel == HAL_OK)
      Battery_UpdateBackfeed(v_bat, v_panel);

    Battery_SetPanel(!(ov_lockout || backfeed_block));
  }

  Battery_Schedule();

  Power_SwitchToRunHSE48MHz();
}