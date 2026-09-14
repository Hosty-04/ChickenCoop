/**
  ******************************************************************************
  * @file    battery.c
  * @brief   Battery and panel monitoring (INA226 + divider) – MOSFET separator
  ******************************************************************************
  */

#include "battery.h"
#include "door.h"
#include "ina226.h"
#include "telemetry.h"
#include "timebase.h"
#include "power.h"
#include "stm32_timer.h"
#include "adc.h"
#include "gpio.h"

#define BATTERY_CHECK_S       (10UL * 60UL)
#define BATTERY_CONV_MS       75U

#define BATTERY_OV_HYST_V     0.1f
#define BATTERY_PANEL_HYST_V  0.05f

#define PANEL_DIV_R1_OHM      970000.0f
#define PANEL_DIV_R2_OHM      488500.0f
#define PANEL_DIV_RATIO       ((PANEL_DIV_R1_OHM + PANEL_DIV_R2_OHM) / PANEL_DIV_R2_OHM)

#define PANEL_ADC_CHANNEL     ADC_CHANNEL_2
#define ADC_FULL_SCALE        4095.0f

#define BATTERY_SEP_PORT      SEP_GPIO_Port
#define BATTERY_SEP_PIN       SEP_Pin

static UTIL_TIMER_Object_t battery_timer;
static volatile uint8_t    battery_pending = 0;

static uint8_t battery_enabled  = 1;
static uint8_t battery_critical = 0;
static uint8_t panel_ov_block   = 0;
static uint8_t panel_bf_block   = 0;

static uint16_t battery_mv = 0;
static uint16_t panel_mv   = 0;

static uint8_t adc_configured = 0;

static void Battery_SetPanel(uint8_t connected)
{
  HAL_GPIO_WritePin(BATTERY_SEP_PORT, BATTERY_SEP_PIN,
                    connected ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

static float Battery_SeasonLimit(uint8_t month)
{
  if (month == 6U || month == 7U || month == 8U)   return 7.2f;
  if (month == 12U || month == 1U || month == 2U)  return 7.5f;
  return 7.3f;
}

static float Battery_CriticalLimit(uint8_t month)
{
  if (month == 11U || month == 12U || month == 1U || month == 2U || month == 3U)
    return 6.15f;
  return 6.0f;
}

static const uint32_t battery_adc_channels[] = {
  ADC_CHANNEL_2, ADC_CHANNEL_VREFINT, ADC_CHANNEL_TEMPSENSOR
};

static HAL_StatusTypeDef Battery_AdcResume(void)
{
  if (!adc_configured)
    return HAL_ERROR;

  if (HAL_ADC_Init(&hadc) != HAL_OK)
    return HAL_ERROR;

  return HAL_ADCEx_Calibration_Start(&hadc);
}

static HAL_StatusTypeDef Battery_AdcSelect(uint32_t channel)
{
  ADC_ChannelConfTypeDef cfg = {0};
  uint32_t n;

  cfg.SamplingTime = ADC_SAMPLINGTIME_COMMON_1;

  for (n = 0; n < (sizeof(battery_adc_channels) / sizeof(battery_adc_channels[0])); n++) {
    cfg.Channel = battery_adc_channels[n];
    cfg.Rank    = (battery_adc_channels[n] == channel) ? ADC_RANK_CHANNEL_NUMBER
                                                       : ADC_RANK_NONE;
    if (HAL_ADC_ConfigChannel(&hadc, &cfg) != HAL_OK)
      return HAL_ERROR;
  }

  return HAL_OK;
}

static HAL_StatusTypeDef Battery_AdcRead(uint32_t channel, uint32_t *raw)
{
  if (Battery_AdcSelect(channel) != HAL_OK)
    return HAL_ERROR;

  if (HAL_ADC_Start(&hadc) != HAL_OK)
    return HAL_ERROR;

  if (HAL_ADC_PollForConversion(&hadc, 20U) != HAL_OK) {
    HAL_ADC_Stop(&hadc);
    return HAL_ERROR;
  }

  *raw = HAL_ADC_GetValue(&hadc);
  HAL_ADC_Stop(&hadc);

  return HAL_OK;
}

static HAL_StatusTypeDef Battery_ReadPanel(float *v_panel)
{
  uint32_t raw_panel, raw_vref, vdda_mv;
  HAL_StatusTypeDef st = HAL_ERROR;

  if (Battery_AdcResume() != HAL_OK)
    return HAL_ERROR;

  if (Battery_AdcRead(ADC_CHANNEL_VREFINT, &raw_vref)  == HAL_OK &&
      Battery_AdcRead(PANEL_ADC_CHANNEL,   &raw_panel) == HAL_OK &&
      raw_vref != 0U) {

    vdda_mv  = __LL_ADC_CALC_VREFANALOG_VOLTAGE(raw_vref, LL_ADC_RESOLUTION_12B);
    *v_panel = (float)raw_panel * ((float)vdda_mv * 0.001f / ADC_FULL_SCALE)
             * PANEL_DIV_RATIO;
    st = HAL_OK;
  }

  HAL_ADC_DeInit(&hadc);

  return st;
}

static void Battery_UpdateOverVoltage(float v_bat, uint8_t month)
{
  float limit = Battery_SeasonLimit(month);

  if (!panel_ov_block) {
    if (v_bat >= limit)
      panel_ov_block = 1;
  } else {
    if (v_bat <= (limit - BATTERY_OV_HYST_V))
      panel_ov_block = 0;
  }
}

static void Battery_UpdateCritical(float v_bat, uint8_t month)
{
  float limit = Battery_CriticalLimit(month);

  if (!battery_critical && v_bat <= limit) {
    battery_critical = 1;
    Door_Reschedule();
  } else if (battery_critical && v_bat > limit) {
    battery_critical = 0;
    Door_Reschedule();
    Door_Catchup();
  }
}

static void Battery_UpdateBackfeed(float v_bat, float v_panel)
{
  if (!panel_bf_block) {
    if (v_panel < (v_bat - BATTERY_PANEL_HYST_V))
      panel_bf_block = 1;
  } else {
    if (v_panel > (v_bat - BATTERY_PANEL_HYST_V))
      panel_bf_block = 0;
  }
}

static void Battery_OnTimer(void *ctx)
{
  UNUSED(ctx);
  battery_pending = 1;
}

static void Battery_Schedule(void)
{
  uint32_t now, next_mark, delay_s;

  if (!battery_enabled) {
    UTIL_TIMER_Stop(&battery_timer);
    return;
  }

  now       = Timebase_GetSecOfDay();
  next_mark = ((now / BATTERY_CHECK_S) + 1UL) * BATTERY_CHECK_S;

  if ((next_mark % SECS_PER_DAY) == 0UL)
    next_mark += BATTERY_CHECK_S;

  delay_s = next_mark - now;
  if (delay_s == 0UL)
    delay_s = 1UL;

  UTIL_TIMER_Stop(&battery_timer);
  UTIL_TIMER_SetPeriod(&battery_timer, delay_s * 1000UL);
  UTIL_TIMER_Start(&battery_timer);
}

void Battery_Init(void)
{
  Battery_SetPanel(1);

  MX_ADC_Init();
  HAL_ADC_DeInit(&hadc);
  adc_configured = 1;

  UTIL_TIMER_Create(&battery_timer, 0xFFFFFFFFU, UTIL_TIMER_ONESHOT,
                    Battery_OnTimer, NULL);
  Battery_Schedule();
}

void Battery_Enable(void)
{
  if (battery_enabled)
    return;

  battery_enabled = 1;
  Battery_Schedule();
  battery_pending = 1;
}

void Battery_Disable(void)
{
  battery_enabled = 0;
  battery_pending = 0;
  UTIL_TIMER_Stop(&battery_timer);
  Battery_SetPanel(0);
}

uint8_t Battery_IsEnabled(void)  { return battery_enabled; }
uint8_t Battery_IsCritical(void) { return battery_critical; }

uint8_t Battery_WorkPending(void)
{
  return battery_pending;
}

uint16_t Battery_GetVoltage_mV(void)      { return battery_mv; }
uint16_t Battery_GetPanelVoltage_mV(void) { return panel_mv; }

void Battery_Process(void)
{
  float v_bat = 0.0f, v_panel = 0.0f;
  HAL_StatusTypeDef st_bat, st_panel;

  if (!battery_pending)
    return;
  battery_pending = 0;

  Power_SwitchToLPRunMSI1MHz();

  INA226_PowerUp();
  INA226_ConfigBattery();
  HAL_Delay(BATTERY_CONV_MS);
  st_bat = INA226_Read(&v_bat, NULL);
  INA226_PowerDown();

  st_panel = Battery_ReadPanel(&v_panel);

  if (st_bat == HAL_OK) {
    uint8_t month = Timebase_GetMonth();

    battery_mv = (uint16_t)(v_bat * 1000.0f + 0.5f);

    Battery_UpdateOverVoltage(v_bat, month);
    Battery_UpdateCritical(v_bat, month);

    if (st_panel == HAL_OK) {
      panel_mv = (uint16_t)(v_panel * 1000.0f + 0.5f);
      Battery_UpdateBackfeed(v_bat, v_panel);
    }

    Battery_SetPanel(!(panel_ov_block || panel_bf_block));
  }

  Battery_Schedule();

  Power_SwitchToRunHSE48MHz();

  Telemetry_RequestStatus();
}