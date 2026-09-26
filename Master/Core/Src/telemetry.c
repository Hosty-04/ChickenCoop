/**
  ******************************************************************************
  * @file    telemetry.c
  * @brief   LoRa uplink and downlink handler
  ******************************************************************************
  */

#include "telemetry.h"
#include "door.h"
#include "battery.h"
#include "system.h"

#define PANEL_STEP_MV       100U
#define PANEL_CODE_MAX      125U
#define PANEL_CODE_NODATA   127U
#define BATTERY_OFFSET_MV   5000U
#define BATTERY_STEP_MV     50U
#define BATTERY_CODE_MAX    60U
#define BATTERY_CODE_NODATA 63U

#define UL_CRITICAL_FLAG    0x01U
#define UL_DOOR_MASK        0x03U

#define DL_SYSTEM_ON        0x01U
#define DL_SYSTEM_OFF       0x02U
#define DL_DOOR_OPEN        0x04U
#define DL_DOOR_CLOSE       0x08U
#define DL_BLOCK            0x10U
#define DL_UNBLOCK          0x20U

static uint8_t telemetry_len = 0;

static uint8_t Telemetry_EncodePanel(void)
{
  uint32_t code;

  if (!Battery_IsPanelValid())
    return PANEL_CODE_NODATA;

  code = ((uint32_t)Battery_GetPanelVoltage_mV() + (PANEL_STEP_MV / 2U)) / PANEL_STEP_MV;

  return (uint8_t)((code > PANEL_CODE_MAX) ? PANEL_CODE_MAX : code);
}

static uint8_t Telemetry_EncodeBattery(void)
{
  uint16_t mv;
  uint32_t code;

  if (!Battery_IsVoltageValid())
    return BATTERY_CODE_NODATA;

  mv = Battery_GetVoltage_mV();
  if (mv <= BATTERY_OFFSET_MV)
    return 0U;

  code = ((uint32_t)(mv - BATTERY_OFFSET_MV) + (BATTERY_STEP_MV / 2U)) / BATTERY_STEP_MV;

  return (uint8_t)((code > BATTERY_CODE_MAX) ? BATTERY_CODE_MAX : code);
}

void Telemetry_RequestStatus(void)
{
  telemetry_len = TELEMETRY_LEN_STATUS;
}

uint8_t Telemetry_Pending(void)
{
  return telemetry_len;
}

void Telemetry_Requeue(uint8_t length)
{
  if (length > telemetry_len)
    telemetry_len = length;
}

void Telemetry_Clear(void)
{
  telemetry_len = 0U;
}

uint8_t Telemetry_Build(uint8_t *buf, uint8_t buf_size)
{
  uint8_t length, n;

  if (buf == NULL)
    return 0U;

  length        = (telemetry_len < buf_size) ? telemetry_len : buf_size;
  telemetry_len = 0U;

  if (length < TELEMETRY_LEN_STATUS)
    return 0U;

  buf[0] = (uint8_t)((Telemetry_EncodePanel() << 1)
         | (Battery_IsCritical() ? UL_CRITICAL_FLAG : 0U));

  buf[1] = (uint8_t)((Telemetry_EncodeBattery() << 2)
         | ((uint8_t)Door_GetState() & UL_DOOR_MASK));

  for (n = TELEMETRY_LEN_STATUS; n < length; n++)
    buf[n] = 0U;

  return length;
}

void Telemetry_HandleDownlink(const uint8_t *buf, uint8_t length)
{
  uint8_t cmd;

  if ((buf == NULL) || (length != TELEMETRY_LEN_DOWNLINK))
    return;

  cmd = buf[0];

  switch (cmd & (DL_BLOCK | DL_UNBLOCK)) {
    case DL_BLOCK:   Door_SetFault();   break;
    case DL_UNBLOCK: Door_ClearFault(); break;
    default:                            break;
  }

  switch (cmd & (DL_SYSTEM_ON | DL_SYSTEM_OFF)) {
    case DL_SYSTEM_ON:  System_Enable();  break;
    case DL_SYSTEM_OFF: System_Disable(); break;
    default:                              break;
  }

  switch (cmd & (DL_DOOR_OPEN | DL_DOOR_CLOSE)) {
    case DL_DOOR_OPEN:  Door_RequestOpen();  break;
    case DL_DOOR_CLOSE: Door_RequestClose(); break;
    default:                                 break;
  }
}
