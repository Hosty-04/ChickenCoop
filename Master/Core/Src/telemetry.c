/**
  ******************************************************************************
  * @file    telemetry.c
  * @brief   LoRa uplink and downlink handeler
  ******************************************************************************
  */

#include "telemetry.h"
#include "door.h"
#include "battery.h"
#include "system.h"

#define PANEL_STEP_MV       100U
#define PANEL_CODE_MAX      110U

#define BATTERY_OFFSET_MV   5000U
#define BATTERY_STEP_MV     50U
#define BATTERY_CODE_MAX    60U

#define DL_SYSTEM_MASK   0x01U   /* 0 turn system off, 1 turn system on */
#define DL_DOOR_MASK     0x02U   /* 0 close door,      1 open door      */
#define DL_BLOCK_MASK    0x04U   /* 0 block door,      1 unblock door   */

static volatile uint8_t telemetry_len = 0;

static uint8_t Telemetry_EncodePanel(uint16_t mv)
{
  uint32_t code = ((uint32_t)mv + (PANEL_STEP_MV / 2U)) / PANEL_STEP_MV;

  return (code > PANEL_CODE_MAX) ? (uint8_t)PANEL_CODE_MAX : (uint8_t)code;
}

static uint8_t Telemetry_EncodeBattery(uint16_t mv)
{
  uint32_t code;

  if (mv <= BATTERY_OFFSET_MV)
    return 0U;

  code = ((uint32_t)(mv - BATTERY_OFFSET_MV) + (BATTERY_STEP_MV / 2U))
       / BATTERY_STEP_MV;

  return (code > BATTERY_CODE_MAX) ? (uint8_t)BATTERY_CODE_MAX : (uint8_t)code;
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

uint8_t Telemetry_Build(uint8_t *buf)
{
  uint8_t length = telemetry_len;

  if (buf == NULL || length == 0U)
    return 0U;

  telemetry_len = 0;

  buf[0] = (uint8_t)(Battery_IsCritical() ? 0x80U : 0x00U)
         | Telemetry_EncodePanel(Battery_GetPanelVoltage_mV());

  buf[1] = (uint8_t)((Telemetry_EncodeBattery(Battery_GetVoltage_mV()) << 2)
         | ((uint8_t)Door_GetState() & 0x03U));

  return length;
}

void Telemetry_HandleDownlink(const uint8_t *buf, uint8_t length)
{
  uint8_t cmd;

  if (buf == NULL || length == 0U)
    return;

  cmd = buf[0];

  if (cmd & DL_SYSTEM_MASK)
    System_Enable();
  else
    System_Disable();

  if (cmd & DL_BLOCK_MASK)
    Door_ClearFault();
  else
    Door_SetFault();

  if (cmd & DL_DOOR_MASK)
    Door_RequestOpen();
  else
    Door_RequestClose();
}