/**
  ******************************************************************************
  * @file    telemetry.c
  * @brief   LoRa uplink and downlink handler
  ******************************************************************************
  */

#include "telemetry.h"
#include "door.h"
#include "battery.h"
#include "nests.h"
#include "system.h"
#include "timebase.h"

#define PANEL_STEP_MV       100U
#define PANEL_CODE_MAX      125U
#define PANEL_CODE_NODATA   127U
#define BATTERY_OFFSET_MV   5000U
#define BATTERY_STEP_MV     50U
#define BATTERY_CODE_MAX    60U
#define BATTERY_CODE_NODATA 63U
#define NEST_CODE_BROODY    11U
#define NEST_CODE_UNCALIB   12U
#define NEST_CODE_FAULT     14U
#define NEST_CODE_NODATA    15U

#define UL_CRITICAL_FLAG    0x01U
#define UL_DOOR_MASK        0x03U

#define DL_SYSTEM_ON        0x01U
#define DL_SYSTEM_OFF       0x02U
#define DL_DOOR_OPEN        0x04U
#define DL_DOOR_CLOSE       0x08U
#define DL_BLOCK            0x10U
#define DL_UNBLOCK          0x20U
#define DL_EXTENDED         0x40U
#define DL_EXTENDED_LEN     4U
#define DL_SEQUENCE_HISTORY 4U
#define DL_STAMP_MASK       0x00FFFFFFUL
#define DL_MAX_AGE_S        1800UL
#define DL_MAX_AHEAD_S      300UL
#define DL_NEST_TARE        0x01U
#define DL_NEST_CALIBRATE   0x02U
#define DL_NEST_PAIR        0x03U
#define DL_NESTS_PER_BYTE   4U

static uint8_t telemetry_len     = 0;
static uint8_t telemetry_dropped = 0;
static uint8_t telemetry_seq[DL_SEQUENCE_HISTORY];
static uint8_t telemetry_seq_pos = 0;

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

static uint8_t Telemetry_EncodeNest(uint8_t nest)
{
  uint8_t eggs;

  if (nest >= NESTS_COUNT)
    return NEST_CODE_NODATA;

  switch (Nests_GetState(nest)) {
    case NESTS_STATE_OK:           break;
    case NESTS_STATE_BROODY:       return NEST_CODE_BROODY;
    case NESTS_STATE_UNCALIBRATED: return NEST_CODE_UNCALIB;
    case NESTS_STATE_FAULT:        return NEST_CODE_FAULT;
    default:                       return NEST_CODE_NODATA;
  }

  eggs = Nests_GetEggs(nest);

  return (eggs > TELEMETRY_EGGS_MAX) ? TELEMETRY_EGGS_MAX : eggs;
}

static uint8_t Telemetry_Fresh(const uint8_t *stamp)
{
  uint32_t queued = ((uint32_t)stamp[0] << 16) | ((uint32_t)stamp[1] << 8) | stamp[2];
  uint32_t age;

  if (!Timebase_IsValid())
    return 1U;

  age = (Timebase_GetUnix() - queued) & DL_STAMP_MASK;

  return (uint8_t)((age <= DL_MAX_AGE_S) || (age >= (DL_STAMP_MASK + 1UL - DL_MAX_AHEAD_S)));
}

static uint8_t Telemetry_NewSequence(uint8_t seq)
{
  uint8_t n;

  for (n = 0U; n < DL_SEQUENCE_HISTORY; n++) {
    if (telemetry_seq[n] == seq)
      return 0U;
  }

  telemetry_seq[telemetry_seq_pos] = seq;
  telemetry_seq_pos = (uint8_t)((telemetry_seq_pos + 1U) % DL_SEQUENCE_HISTORY);

  return 1U;
}

static void Telemetry_HandleNests(const uint8_t *buf, uint8_t length)
{
  uint8_t nest, pair;

  for (nest = 0U; (nest < NESTS_COUNT) && ((nest / DL_NESTS_PER_BYTE) < length); nest++) {
    pair = (uint8_t)((buf[nest / DL_NESTS_PER_BYTE] >> (2U * (nest % DL_NESTS_PER_BYTE))) & DL_NEST_PAIR);

    switch (pair) {
      case DL_NEST_TARE:      Nests_RequestTare(nest);      break;
      case DL_NEST_CALIBRATE: Nests_RequestCalibrate(nest); break;
      default:                                              break;
    }
  }
}

void Telemetry_RequestStatus(void)
{
  if (telemetry_len < TELEMETRY_LEN_STATUS)
    telemetry_len = TELEMETRY_LEN_STATUS;
}

void Telemetry_RequestFull(void)
{
  telemetry_len = TELEMETRY_LEN_FULL;
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
  if (telemetry_len > telemetry_dropped)
    telemetry_dropped = telemetry_len;

  telemetry_len = 0U;
}

uint8_t Telemetry_Build(uint8_t *buf, uint8_t buf_size)
{
  uint8_t length, n, nest;

  if (buf == NULL)
    return 0U;

  length = (telemetry_len > telemetry_dropped) ? telemetry_len : telemetry_dropped;
  if (length > buf_size)
    length = buf_size;

  telemetry_len     = 0U;
  telemetry_dropped = 0U;

  if (length < TELEMETRY_LEN_STATUS)
    return 0U;

  buf[0] = (uint8_t)((Telemetry_EncodePanel() << 1)
         | (Battery_IsCritical() ? UL_CRITICAL_FLAG : 0U));

  buf[1] = (uint8_t)((Telemetry_EncodeBattery() << 2)
         | ((uint8_t)Door_GetState() & UL_DOOR_MASK));

  for (n = TELEMETRY_LEN_STATUS; n < length; n++) {
    nest   = (uint8_t)(2U * (n - TELEMETRY_LEN_STATUS));
    buf[n] = (uint8_t)((Telemetry_EncodeNest(nest) << 4) | Telemetry_EncodeNest(nest + 1U));
  }

  return length;
}

void Telemetry_HandleDownlink(const uint8_t *buf, uint8_t length)
{
  uint8_t cmd, head;

  if ((buf == NULL) || (length < TELEMETRY_LEN_DOWNLINK))
    return;

  cmd  = buf[0];
  head = (uint8_t)(TELEMETRY_LEN_DOWNLINK + ((cmd & DL_EXTENDED) ? DL_EXTENDED_LEN : 0U));

  if ((length < head) || (length > (uint8_t)(head + TELEMETRY_LEN_DL_MAX - TELEMETRY_LEN_DOWNLINK)))
    return;

  if ((cmd & DL_EXTENDED) &&
      (!Telemetry_Fresh(&buf[TELEMETRY_LEN_DOWNLINK + 1U]) || !Telemetry_NewSequence(buf[TELEMETRY_LEN_DOWNLINK])))
    return;

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

  Telemetry_HandleNests(&buf[head], (uint8_t)(length - head));
}
