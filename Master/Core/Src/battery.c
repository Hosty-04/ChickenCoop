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
#include "adc.h"
#include "lora_app.h"
#include "stm32_timer.h"

#define BATTERY_CHECK_S        (10UL * 60UL)
#define BATTERY_CONV_MS        80U
#define BATTERY_BLOCK_MS       500UL
#define BATTERY_DEFER_S        10UL
#define BATTERY_DEFER_MAX      6U
#define BATTERY_MIN_VALID_V    1.0f
#define BATTERY_MAX_VALID_V    12.0f

#define PANEL_SETTLE_MS        200U
#define PANEL_DIV_R1_OHM       970000.0f
#define PANEL_DIV_R2_OHM       330000.0f
#define PANEL_DIV_RATIO        ((PANEL_DIV_R1_OHM + PANEL_DIV_R2_OHM) / PANEL_DIV_R2_OHM)
#define PANEL_ADC_CHANNEL      ADC_CHANNEL_2

#define ADC_FULL_SCALE         4095.0f
#define ADC_TIMEOUT_MS         20U

static const uint32_t battery_adc_channels[] = {
  ADC_CHANNEL_2, ADC_CHANNEL_VREFINT, ADC_CHANNEL_TEMPSENSOR
};

static UTIL_TIMER_Object_t battery_timer;
static volatile uint8_t    battery_pending = 0;

static uint8_t  battery_critical = 0;
static uint8_t  battery_defer    = 0;
static uint8_t  panel_connected  = 0;
static uint8_t  panel_ov_block   = 0;
static uint8_t  panel_bf_block   = 0;
static uint16_t battery_mv       = 0;
static uint16_t panel_mv         = 0;
static uint8_t  battery_mv_valid = 0;
static uint8_t  panel_mv_valid   = 0;

static void Battery_SetPanel(uint8_t connected)
{
  panel_connected = connected ? 1U : 0U;
  HAL_GPIO_WritePin(SEP_GPIO_Port, SEP_Pin, panel_connected ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

static float Battery_OverVoltageLimit(uint8_t month)
{
  if ((month == 0U) || ((month >= 6U) && (month <= 8U))) return 7.2f;
  if ((month == 12U) || (month <= 2U))                     return 7.5f;
  return 7.3f;
}

static uint8_t Battery_Plausible(float v_bat)
{
  return (uint8_t)((v_bat >= BATTERY_MIN_VALID_V) && (v_bat <= BATTERY_MAX_VALID_V));
}

static float Battery_CriticalLimit(uint8_t month)
{
  return ((month >= 11U) || (month <= 3U)) ? 6.15f : 6.0f;
}

static HAL_StatusTypeDef Battery_AdcRead(uint32_t channel, uint32_t *raw)
{
  ADC_ChannelConfTypeDef cfg = {0};
  uint32_t n;

  cfg.SamplingTime = ADC_SAMPLINGTIME_COMMON_1;

  for (n = 0U; n < (sizeof(battery_adc_channels) / sizeof(battery_adc_channels[0])); n++) {
    cfg.Channel = battery_adc_channels[n];
    cfg.Rank    = (cfg.Channel == channel) ? ADC_RANK_CHANNEL_NUMBER : ADC_RANK_NONE;
    if (HAL_ADC_ConfigChannel(&hadc, &cfg) != HAL_OK)
      return HAL_ERROR;
  }

  if (HAL_ADC_Start(&hadc) != HAL_OK)
    return HAL_ERROR;

  if (HAL_ADC_PollForConversion(&hadc, ADC_TIMEOUT_MS) != HAL_OK) {
    (void)HAL_ADC_Stop(&hadc);
    return HAL_ERROR;
  }

  *raw = HAL_ADC_GetValue(&hadc);
  (void)HAL_ADC_Stop(&hadc);

  return HAL_OK;
}

static HAL_StatusTypeDef Battery_ReadPanel(float *v_panel)
{
  uint32_t raw_panel = 0U, raw_vref = 0U, vdda_mv;
  HAL_StatusTypeDef st = HAL_ERROR;

  if ((HAL_ADC_Init(&hadc) == HAL_OK) &&
      (HAL_ADCEx_Calibration_Start(&hadc) == HAL_OK) &&
      (Battery_AdcRead(ADC_CHANNEL_VREFINT, &raw_vref) == HAL_OK) &&
      (Battery_AdcRead(PANEL_ADC_CHANNEL, &raw_panel) == HAL_OK) &&
      (raw_vref != 0U)) {
    vdda_mv  = __LL_ADC_CALC_VREFANALOG_VOLTAGE(raw_vref, LL_ADC_RESOLUTION_12B);
    *v_panel = (float)raw_panel * ((float)vdda_mv * 0.001f / ADC_FULL_SCALE) * PANEL_DIV_RATIO;
    st = HAL_OK;
  }

  (void)HAL_ADC_DeInit(&hadc);

  return st;
}

static void Battery_UpdateOverVoltage(float v_bat, uint8_t month)
{
  panel_ov_block = (uint8_t)(v_bat >= Battery_OverVoltageLimit(month));
}

static void Battery_UpdateCritical(float v_bat, uint8_t month)
{
  uint8_t critical = (uint8_t)(v_bat <= Battery_CriticalLimit(month));

  if (critical == battery_critical)
    return;

  battery_critical = critical;
  Door_Reschedule();

  if (!critical)
    Door_Catchup();
}

static void Battery_UpdateSensor(uint8_t ok)
{
  if (ok == battery_mv_valid)
    return;

  battery_mv_valid = ok;
  Door_Reschedule();

  if (ok)
    Door_Catchup();
}

static void Battery_UpdateBackfeed(float v_bat, float v_panel)
{
  panel_bf_block = (uint8_t)(v_panel <= v_bat);
}

static void Battery_OnTimer(void *ctx)
{
  UNUSED(ctx);
  battery_pending = 1U;
}

static void Battery_ArmTimer(uint32_t seconds)
{
  UTIL_TIMER_Stop(&battery_timer);
  UTIL_TIMER_SetPeriod(&battery_timer, ((seconds != 0UL) ? seconds : 1UL) * 1000UL);
  UTIL_TIMER_Start(&battery_timer);
}

static void Battery_Schedule(void)
{
  uint32_t now  = Timebase_GetSecOfDay();
  uint32_t next = ((now / BATTERY_CHECK_S) + 1UL) * BATTERY_CHECK_S;

  Battery_ArmTimer(next - now);
}

static uint8_t Battery_RadioBusy(void)
{
  if (LoRaWAN_CanBlockFor(BATTERY_BLOCK_MS) || (++battery_defer >= BATTERY_DEFER_MAX)) {
    battery_defer = 0U;
    return 0U;
  }

  return 1U;
}

void Battery_Init(void)
{
  Battery_SetPanel(1U);

  MX_ADC_Init();
  (void)HAL_ADC_DeInit(&hadc);

  UTIL_TIMER_Create(&battery_timer, 0xFFFFFFFFU, UTIL_TIMER_ONESHOT, Battery_OnTimer, NULL);
  battery_pending = 1U;
}

void Battery_Process(void)
{
  float    v_bat = 0.0f, v_panel = 0.0f;
  uint8_t  was_connected = panel_connected;
  uint8_t  month, bat_ok;
  HAL_StatusTypeDef st_bat, st_panel;

  if (!battery_pending)
    return;
  battery_pending = 0U;

  if (Battery_RadioBusy()) {
    Battery_ArmTimer(BATTERY_DEFER_S);
    return;
  }

  Power_SwitchToLPRunMSI1MHz();

  st_bat = INA226_PowerUp();
  if (st_bat == HAL_OK)
    st_bat = INA226_ConfigBattery();
  if (st_bat == HAL_OK) {
    HAL_Delay(BATTERY_CONV_MS);
    st_bat = INA226_Read(&v_bat, NULL);
  }
  INA226_PowerDown();

  if (panel_connected) {
    Battery_SetPanel(0U);
    HAL_Delay(PANEL_SETTLE_MS);
  }
  st_panel = Battery_ReadPanel(&v_panel);

  bat_ok         = (uint8_t)((st_bat == HAL_OK) && Battery_Plausible(v_bat));
  panel_mv_valid = (uint8_t)(st_panel == HAL_OK);

  if (st_panel == HAL_OK)
    panel_mv = (uint16_t)(v_panel * 1000.0f + 0.5f);

  if (bat_ok) {
    month      = Timebase_IsValid() ? Timebase_GetMonth() : 0U;
    battery_mv = (uint16_t)(v_bat * 1000.0f + 0.5f);

    Battery_UpdateOverVoltage(v_bat, month);
    Battery_UpdateCritical(v_bat, month);

    if (st_panel == HAL_OK)
      Battery_UpdateBackfeed(v_bat, v_panel);

    Battery_SetPanel((uint8_t)!(panel_ov_block || panel_bf_block));
  } else {
    Battery_SetPanel(was_connected);
  }

  Battery_UpdateSensor(bat_ok);

  Power_SwitchToRunHSE48MHz();

  Battery_Schedule();
  Telemetry_RequestStatus();
}

uint8_t Battery_WorkPending(void)
{
  return battery_pending;
}

uint8_t Battery_IsCritical(void)
{
  return battery_critical;
}

uint16_t Battery_GetVoltage_mV(void)
{
  return battery_mv;
}

uint16_t Battery_GetPanelVoltage_mV(void)
{
  return panel_mv;
}

uint8_t Battery_IsVoltageValid(void)
{
  return battery_mv_valid;
}

uint8_t Battery_IsPanelValid(void)
{
  return panel_mv_valid;
}
