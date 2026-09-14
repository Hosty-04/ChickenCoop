/**
  ******************************************************************************
  * @file    door.c
  * @brief   Door control (sunrise/sunset based)
  ******************************************************************************
  */

#include "door.h"
#include "astro.h"
#include "motor.h"
#include "endstop.h"
#include "battery.h"
#include "telemetry.h"
#include "timebase.h"
#include "rtc.h"
#include "stm32_timer.h"
#include "stm32_systime.h"

#define DOOR_OPEN_OFFSET_MIN   (-60)
#define DOOR_CLOSE_OFFSET_MIN  ( 60)
#define DOOR_RETRY_S           (5UL * 60UL)

#define DOOR_RETRY_MAX         3U

#define DOOR_BKP_REG    RTC_BKP_DR4
#define DOOR_BKP_MAGIC  0x00D0U

typedef enum {
  DOOR_EVT_NONE = 0,
  DOOR_EVT_MIDNIGHT,
  DOOR_EVT_SUN,
  DOOR_EVT_RETRY
} Door_Event_t;

typedef enum {
  DOOR_REQ_NONE = 0,
  DOOR_REQ_OPEN,
  DOOR_REQ_CLOSE
} Door_Request_t;

static float   door_lat = 49.5170f;
static float   door_lon = 17.6181f;
static int16_t door_sunrise_min = 360;
static int16_t door_sunset_min  = 1080;

static UTIL_TIMER_Object_t     door_timer;
static volatile Door_Event_t   door_pending = DOOR_EVT_NONE;
static Door_Event_t            door_armed   = DOOR_EVT_NONE;

static volatile Door_Request_t door_request = DOOR_REQ_NONE;

static uint8_t      door_enabled  = 1;
static uint8_t      door_fault    = 0;
static Door_State_t door_reported = DOOR_STATE_UNKNOWN;

static uint8_t      door_retry_pending  = 0;
static uint32_t     door_retry_deadline = 0;
static Motor_Dir_t  door_retry_dir      = MOTOR_DIR_UP;
static uint8_t      door_retry_count    = 0;

static void Door_Schedule(void);

static void Door_StoreState(void)
{
  uint32_t word = ((uint32_t)DOOR_BKP_MAGIC << 16)
                | ((uint32_t)(door_fault ? 1U : 0U) << 8)
                |  (uint32_t)Endstop_Last();

  HAL_PWR_EnableBkUpAccess();
  HAL_RTCEx_BKUPWrite(&hrtc, DOOR_BKP_REG, word);
}

static void Door_LoadState(void)
{
  uint32_t word = HAL_RTCEx_BKUPRead(&hrtc, DOOR_BKP_REG);

  if (((word >> 16) & 0xFFFFU) != DOOR_BKP_MAGIC) {
    (void)Endstop_Sample();
    Door_StoreState();
  } else {
    door_fault = ((word >> 8) & 0x01U) ? 1U : 0U;
    Endstop_Restore((Endstop_Pos_t)(word & 0xFFU));
  }

  door_reported = Door_GetState();
}

static void Door_PublishState(void)
{
  Door_State_t state = Door_GetState();

  if (state != door_reported) {
    door_reported = state;
    Telemetry_RequestStatus();
  }
}

static void Door_OnTimer(void *ctx)
{
  UNUSED(ctx);
  door_pending = door_armed;
}

static void Door_ArmTimer(uint32_t seconds, Door_Event_t evt)
{
  if (seconds == 0U)
    seconds = 1U;

  door_armed = evt;
  UTIL_TIMER_Stop(&door_timer);
  UTIL_TIMER_SetPeriod(&door_timer, seconds * 1000U);
  UTIL_TIMER_Start(&door_timer);
}

static void Door_StopTimer(void)
{
  UTIL_TIMER_Stop(&door_timer);
  door_pending = DOOR_EVT_NONE;
  door_armed   = DOOR_EVT_NONE;
}

static void Door_UpdateSun(void)
{
  Astro_Result_t res;

  Astro_Calculate(Timebase_GetYear(), Timebase_GetMonth(), Timebase_GetDay(),
                  door_lat, door_lon, &res);

  door_sunrise_min = res.sunrise_min;
  door_sunset_min  = res.sunset_min;
}

static uint32_t Door_MinuteToSec(int32_t minute_of_day)
{
  while (minute_of_day < 0)             minute_of_day += MINS_PER_DAY;
  while (minute_of_day >= MINS_PER_DAY) minute_of_day -= MINS_PER_DAY;

  return (uint32_t)minute_of_day * 60U;
}

static uint32_t Door_OpenTime(void)
{
  return Door_MinuteToSec((int32_t)door_sunrise_min + DOOR_OPEN_OFFSET_MIN);
}

static uint32_t Door_CloseTime(void)
{
  return Door_MinuteToSec((int32_t)door_sunset_min + DOOR_CLOSE_OFFSET_MIN);
}

static uint32_t Door_DelayTo(uint32_t now, uint32_t target)
{
  if (target <= now)
    target += SECS_PER_DAY;

  return target - now;
}

static uint32_t Door_RetryDelay(void)
{
  int32_t remain = (int32_t)(door_retry_deadline - HAL_GetTick());

  return (remain <= 0) ? 0U : ((uint32_t)remain / 1000U);
}

static Motor_Dir_t Door_DesiredDir(uint32_t now)
{
  uint32_t t_open  = Door_OpenTime();
  uint32_t t_close = Door_CloseTime();

  if (t_open <= t_close)
    return (now >= t_open && now < t_close) ? MOTOR_DIR_UP : MOTOR_DIR_DOWN;

  return (now >= t_open || now < t_close) ? MOTOR_DIR_UP : MOTOR_DIR_DOWN;
}

static uint8_t Door_PosMatchesDesired(void)
{
  Motor_Dir_t   want = Door_DesiredDir(Timebase_GetSecOfDay());
  Endstop_Pos_t pos  = Endstop_Last();

  return (want == MOTOR_DIR_UP) ? (pos == ENDSTOP_POS_TOP)
                                : (pos == ENDSTOP_POS_BOTTOM);
}

static void Door_Schedule(void)
{
  uint32_t now, delay, best;
  Door_Event_t evt;

  if (!door_enabled) {
    Door_StopTimer();
    return;
  }

  now  = Timebase_GetSecOfDay();
  best = SECS_PER_DAY - now;
  evt  = DOOR_EVT_MIDNIGHT;

  if (!Battery_IsCritical() && !door_fault) {
    delay = Door_DelayTo(now, Door_OpenTime());
    if (delay < best) { best = delay; evt = DOOR_EVT_SUN; }

    delay = Door_DelayTo(now, Door_CloseTime());
    if (delay < best) { best = delay; evt = DOOR_EVT_SUN; }
  }

  if (door_retry_pending && !door_fault) {
    delay = Door_RetryDelay();
    if (delay < best) { best = delay; evt = DOOR_EVT_RETRY; }
  }

  Door_ArmTimer(best, evt);
}

static void Door_RaiseFault(void)
{
  door_fault         = 1;
  door_retry_pending = 0;
  door_retry_count   = 0;
}

static void Door_Apply(Motor_Dir_t dir)
{
  Motor_Result_t r = Motor_Move(dir);

  switch (r) {
    case MOTOR_OK:
    case MOTOR_ALREADY:
      door_retry_pending = 0;
      door_retry_count   = 0;
      break;

    case MOTOR_OBSTACLE:
      if (++door_retry_count >= DOOR_RETRY_MAX) {
        Door_RaiseFault();
        break;
      }
      door_retry_pending  = 1;
      door_retry_dir      = dir;
      door_retry_deadline = HAL_GetTick() + (DOOR_RETRY_S * 1000UL);
      break;

    case MOTOR_NO_REFERENCE:
    case MOTOR_STUCK:
    case MOTOR_TIMEOUT:
    case MOTOR_SENSOR_ERROR:
    default:
      Door_RaiseFault();
      break;
  }

  Door_StoreState();
  Door_PublishState();
}

void Door_Init(void)
{
  UTIL_TIMER_Create(&door_timer, 0xFFFFFFFFU, UTIL_TIMER_ONESHOT,
                    Door_OnTimer, NULL);
  Door_LoadState();
}

void Door_Setup(uint16_t y, uint8_t mo, uint8_t d,
                uint8_t h, uint8_t mi, uint8_t s,
                float latitude, float longitude)
{
  Astro_Result_t res;
  SysTime_t st;

  door_lat = latitude;
  door_lon = longitude;

  Timebase_Set(y, mo, d, h, mi, s);

  Astro_Calculate(y, mo, d, door_lat, door_lon, &res);
  st.Seconds    = Timebase_ToUnix(y, mo, d, h, mi, s, res.timezone);
  st.SubSeconds = 0;
  SysTimeSet(st);

  Door_UpdateSun();
  Door_Schedule();
}

void Door_Reschedule(void)
{
  Door_Schedule();
}

void Door_SyncFromSysTime(void)
{
  SysTime_t t = SysTimeGet();

  if (t.Seconds > 0U) {
    Timebase_FromUnix(t.Seconds, door_lat, door_lon);
    Door_UpdateSun();
  }

  Door_Schedule();
}

void Door_Catchup(void)
{
  if (!door_enabled || door_fault || Battery_IsCritical())
    return;

  door_pending = DOOR_EVT_SUN;
}

void Door_RequestOpen(void)  { door_request = DOOR_REQ_OPEN;  }
void Door_RequestClose(void) { door_request = DOOR_REQ_CLOSE; }

void Door_SetFault(void)
{
  Door_RaiseFault();
  door_request = DOOR_REQ_NONE;
  Door_StoreState();
  Door_PublishState();
  Door_Schedule();
}

void Door_ClearFault(void)
{
  door_fault         = 0;
  door_retry_pending = 0;
  door_retry_count   = 0;
  (void)Endstop_Sample();
  Door_StoreState();
  Door_PublishState();
  Door_Schedule();
}

void Door_Enable(void)
{
  if (door_enabled)
    return;

  door_enabled = 1;
  Door_SyncFromSysTime();
  Door_Catchup();
}

void Door_Disable(void)
{
  door_enabled       = 0;
  door_request       = DOOR_REQ_NONE;
  door_retry_pending = 0;
  door_retry_count   = 0;
  Door_StopTimer();
}

uint8_t Door_IsEnabled(void) { return door_enabled; }
uint8_t Door_IsFaulted(void) { return door_fault; }

uint8_t Door_WorkPending(void)
{
  return (uint8_t)((door_request != DOOR_REQ_NONE) ||
                   (door_pending != DOOR_EVT_NONE));
}

Door_State_t Door_GetState(void)
{
  if (door_fault)
    return DOOR_STATE_JAMMED;

  switch (Endstop_Last()) {
    case ENDSTOP_POS_TOP:    return DOOR_STATE_OPEN;
    case ENDSTOP_POS_BOTTOM: return DOOR_STATE_CLOSED;
    default:                 return DOOR_STATE_UNKNOWN;
  }
}

Door_State_t Door_RefreshState(void)
{
  (void)Endstop_Sample();
  return Door_GetState();
}

void Door_Process(void)
{
  Door_Request_t req;
  Door_Event_t   evt;

  req = door_request;
  door_request = DOOR_REQ_NONE;

  if (req != DOOR_REQ_NONE) {
    if (door_fault)
      return;

    door_retry_pending = 0;
    door_retry_count   = 0;
    door_pending       = DOOR_EVT_NONE;

    Door_Apply((req == DOOR_REQ_OPEN) ? MOTOR_DIR_UP : MOTOR_DIR_DOWN);
    Door_Schedule();
    return;
  }

  evt = door_pending;
  door_pending = DOOR_EVT_NONE;

  if (evt == DOOR_EVT_NONE || !door_enabled)
    return;

  if (evt == DOOR_EVT_MIDNIGHT) {
    Door_SyncFromSysTime();
    return;
  }

  if (door_fault) {
    Door_Schedule();
    return;
  }

  if (evt == DOOR_EVT_RETRY) {
    if (!door_retry_pending) {
      Door_Schedule();
      return;
    }
    door_retry_pending = 0;
    Door_Apply(door_retry_dir);
  } else {
    Door_Apply(Door_DesiredDir(Timebase_GetSecOfDay()));
  }

  Door_Schedule();

  if (!door_fault && !door_retry_pending && !Battery_IsCritical() &&
      !Door_PosMatchesDesired())
    door_pending = DOOR_EVT_SUN;
}