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
#include "lora_app.h"
#include "rtc.h"
#include "stm32_timer.h"
#include "utilities_conf.h"

#define DOOR_OPEN_OFFSET_MIN   (-60)
#define DOOR_CLOSE_OFFSET_MIN  60
#define DOOR_RETRY_S           (5UL * 60UL)
#define DOOR_RETRY_MAX         3U
#define DOOR_RESYNC_S          3600UL
#define DOOR_RESYNC_OFFSET_S   5UL
#define DOOR_TIME_SYNC_S       (12UL * 3600UL)
#define DOOR_MOVE_BUDGET_MS    55000UL
#define DOOR_DEFER_S           10UL
#define DOOR_DEFER_MAX         6U

#define DOOR_BKP_REG           RTC_BKP_DR4
#define DOOR_BKP_MAGIC         0x00D0UL

typedef enum {
  DOOR_EVT_NONE = 0,
  DOOR_EVT_RESYNC,
  DOOR_EVT_SUN,
  DOOR_EVT_RETRY,
  DOOR_EVT_DEFER
} Door_Event_t;

typedef enum {
  DOOR_REQ_NONE = 0,
  DOOR_REQ_OPEN,
  DOOR_REQ_CLOSE
} Door_Request_t;

static UTIL_TIMER_Object_t   door_timer;
static volatile Door_Event_t door_pending = DOOR_EVT_NONE;
static Door_Event_t          door_armed   = DOOR_EVT_NONE;
static Door_Request_t        door_request = DOOR_REQ_NONE;

static Door_Request_t door_deferred_req = DOOR_REQ_NONE;
static Door_Event_t   door_deferred_evt = DOOR_EVT_NONE;
static uint8_t        door_defer_count  = 0;

static uint8_t      door_enabled  = 1;
static uint8_t      door_fault    = 0;
static Door_State_t door_reported = DOOR_STATE_UNKNOWN;

static uint8_t     door_manual      = 0;
static Motor_Dir_t door_manual_want = MOTOR_DIR_UP;

static uint8_t     door_retry_pending = 0;
static uint8_t     door_retry_count   = 0;
static Motor_Dir_t door_retry_dir     = MOTOR_DIR_UP;
static uint32_t    door_retry_tick    = 0;

static float    door_lat            = 49.5170f;
static float    door_lon            = 17.6181f;
static int16_t  door_sunrise_min    = 360;
static int16_t  door_sunset_min     = 1080;
static uint32_t door_sun_key        = 0;
static int16_t  door_sun_tz_min     = 0;
static uint32_t door_time_sync_unix = 0;

static void Door_StoreState(void)
{
  HAL_PWR_EnableBkUpAccess();
  HAL_RTCEx_BKUPWrite(&hrtc, DOOR_BKP_REG,
                      (DOOR_BKP_MAGIC << 16) | ((uint32_t)door_fault << 8));
}

static void Door_LoadState(void)
{
  uint32_t word = HAL_RTCEx_BKUPRead(&hrtc, DOOR_BKP_REG);

  if ((word >> 16) == DOOR_BKP_MAGIC)
    door_fault = (uint8_t)((word >> 8) & 0x01U);

  (void)Endstop_Sample();
  Door_StoreState();

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
  UTIL_TIMER_Stop(&door_timer);
  door_armed = evt;
  UTIL_TIMER_SetPeriod(&door_timer, ((seconds != 0UL) ? seconds : 1UL) * 1000UL);
  UTIL_TIMER_Start(&door_timer);
}

static void Door_StopTimer(void)
{
  UTIL_TIMER_Stop(&door_timer);
  door_armed   = DOOR_EVT_NONE;
  door_pending = DOOR_EVT_NONE;
}

static Door_Event_t Door_TakePending(void)
{
  Door_Event_t evt;

  UTILS_ENTER_CRITICAL_SECTION();
  evt          = door_pending;
  door_pending = DOOR_EVT_NONE;
  UTILS_EXIT_CRITICAL_SECTION();

  return evt;
}

static void Door_UpdateSun(void)
{
  Astro_Result_t res;

  door_sun_key    = Timebase_GetDateKey();
  door_sun_tz_min = Timebase_GetTimezoneMin();

  Astro_Calculate(Timebase_GetYear(), Timebase_GetMonth(), Timebase_GetDay(),
                  door_lat, door_lon, door_sun_tz_min, &res);

  door_sunrise_min = res.sunrise_min;
  door_sunset_min  = res.sunset_min;
}

static void Door_RefreshSun(void)
{
  if ((Timebase_GetDateKey() != door_sun_key) ||
      (Timebase_GetTimezoneMin() != door_sun_tz_min))
    Door_UpdateSun();
}

static uint32_t Door_MinuteToSec(int32_t minute)
{
  minute %= MINS_PER_DAY;
  if (minute < 0)
    minute += MINS_PER_DAY;

  return (uint32_t)minute * 60UL;
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
  return (target > now) ? (target - now) : (target + SECS_PER_DAY - now);
}

static Motor_Dir_t Door_DesiredDir(uint32_t now)
{
  uint32_t t_open  = Door_OpenTime();
  uint32_t t_close = Door_CloseTime();
  uint8_t  day;

  if (t_open <= t_close)
    day = (uint8_t)((now >= t_open) && (now < t_close));
  else
    day = (uint8_t)((now >= t_open) || (now < t_close));

  return day ? MOTOR_DIR_UP : MOTOR_DIR_DOWN;
}

static uint8_t Door_AutoAllowed(void)
{
  return (uint8_t)(door_enabled && !door_fault && !Battery_IsCritical() && Timebase_IsValid());
}

static uint8_t Door_ManualActive(uint32_t now)
{
  if (door_manual && (Door_DesiredDir(now) != door_manual_want))
    door_manual = 0U;

  return door_manual;
}

static uint8_t Door_NeedsCatchup(void)
{
  uint32_t      now;
  Endstop_Pos_t want;

  if (!Door_AutoAllowed() || door_retry_pending)
    return 0U;

  now = Timebase_GetSecOfDay();
  if (Door_ManualActive(now))
    return 0U;

  want = (Door_DesiredDir(now) == MOTOR_DIR_UP) ? ENDSTOP_POS_TOP : ENDSTOP_POS_BOTTOM;

  return (uint8_t)(Endstop_Last() != want);
}

static uint32_t Door_RetryDelay(void)
{
  uint32_t elapsed = (HAL_GetTick() - door_retry_tick) / TICKS_PER_SEC;

  return (elapsed >= DOOR_RETRY_S) ? 0UL : (DOOR_RETRY_S - elapsed);
}

static void Door_Schedule(void)
{
  uint32_t     now, delay, best;
  Door_Event_t evt = DOOR_EVT_RESYNC;

  Door_RefreshSun();
  now  = Timebase_GetSecOfDay();
  best = (DOOR_RESYNC_S + DOOR_RESYNC_OFFSET_S - (now % DOOR_RESYNC_S)) % DOOR_RESYNC_S;
  if (best == 0UL)
    best = DOOR_RESYNC_S;

  if (Door_AutoAllowed()) {
    delay = Door_DelayTo(now, Door_OpenTime());
    if (delay < best) { best = delay; evt = DOOR_EVT_SUN; }

    delay = Door_DelayTo(now, Door_CloseTime());
    if (delay < best) { best = delay; evt = DOOR_EVT_SUN; }
  }

  if (door_retry_pending && (door_deferred_evt != DOOR_EVT_RETRY)) {
    delay = Door_RetryDelay();
    if (delay < best) { best = delay; evt = DOOR_EVT_RETRY; }
  }

  if (((door_deferred_req != DOOR_REQ_NONE) || (door_deferred_evt != DOOR_EVT_NONE)) &&
      (DOOR_DEFER_S < best)) {
    best = DOOR_DEFER_S;
    evt  = DOOR_EVT_DEFER;
  }

  Door_ArmTimer(best, evt);
}

static void Door_MaintainTimeSync(void)
{
  if (!Timebase_IsValid() || ((Timebase_GetUnix() - door_time_sync_unix) >= DOOR_TIME_SYNC_S))
    LoRaWAN_RequestTime();
}

static uint8_t Door_RadioBusy(void)
{
  if (LoRaWAN_CanBlockFor(DOOR_MOVE_BUDGET_MS) || (++door_defer_count >= DOOR_DEFER_MAX)) {
    door_defer_count = 0U;
    return 0U;
  }

  return 1U;
}

static void Door_RaiseFault(void)
{
  door_fault         = 1U;
  door_manual        = 0U;
  door_retry_pending = 0U;
  door_retry_count   = 0U;
  door_deferred_req  = DOOR_REQ_NONE;
  door_deferred_evt  = DOOR_EVT_NONE;
}

static void Door_Apply(Motor_Dir_t dir)
{
  if (dir != door_retry_dir) {
    door_retry_dir   = dir;
    door_retry_count = 0U;
  }

  switch (Motor_Move(dir)) {
    case MOTOR_OK:
    case MOTOR_ALREADY:
      door_retry_pending = 0U;
      door_retry_count   = 0U;
      break;

    case MOTOR_OBSTACLE:
      if (++door_retry_count >= DOOR_RETRY_MAX) {
        Door_RaiseFault();
      } else {
        door_retry_pending = 1U;
        door_retry_tick    = HAL_GetTick();
      }
      break;

    default:
      Door_RaiseFault();
      break;
  }

  Door_StoreState();
  Door_PublishState();
}

void Door_Init(void)
{
  UTIL_TIMER_Create(&door_timer, 0xFFFFFFFFU, UTIL_TIMER_ONESHOT, Door_OnTimer, NULL);
  Door_LoadState();
}

void Door_Setup(uint16_t year, uint8_t month, uint8_t day,
                uint8_t hour, uint8_t min, uint8_t sec,
                float latitude, float longitude)
{
  uint32_t fallback = Timebase_LocalToUnix(year, month, day, hour, min, sec);
  uint8_t  restored = Timebase_Restore();

  door_lat     = latitude;
  door_lon     = longitude;
  door_sun_key = 0UL;

#if DOOR_TRUST_FALLBACK_TIME
  if (!restored || (fallback > Timebase_GetUnix()))
    Timebase_SetUnix(fallback);
#else
  if (!restored)
    Timebase_SetFallback(fallback);
#endif

  Door_Schedule();
  Door_Catchup();
}

void Door_Process(void)
{
  Door_Request_t req = door_request;
  Door_Event_t   evt = Door_TakePending();
  uint32_t       now;

  door_request = DOOR_REQ_NONE;

  if ((req == DOOR_REQ_NONE) && (evt == DOOR_EVT_NONE))
    return;

  if (!door_enabled) {
    if (evt == DOOR_EVT_RESYNC) {
      Door_Schedule();
      Door_MaintainTimeSync();
    }
    return;
  }

  if (evt == DOOR_EVT_DEFER) {
    if (req == DOOR_REQ_NONE)
      req = door_deferred_req;
    evt = door_deferred_evt;
    door_deferred_req = DOOR_REQ_NONE;
    door_deferred_evt = DOOR_EVT_NONE;
  }

  if (evt == DOOR_EVT_RESYNC) {
    Door_Schedule();
    Door_MaintainTimeSync();
    evt = Door_NeedsCatchup() ? DOOR_EVT_SUN : DOOR_EVT_NONE;
  }

  now = Timebase_GetSecOfDay();

  if (door_fault)
    req = DOOR_REQ_NONE;
  if ((evt == DOOR_EVT_RETRY) && !door_retry_pending)
    evt = DOOR_EVT_NONE;
  if ((evt == DOOR_EVT_SUN) && (!Door_AutoAllowed() || Door_ManualActive(now)))
    evt = DOOR_EVT_NONE;

  if ((req == DOOR_REQ_NONE) && (evt == DOOR_EVT_NONE)) {
    Door_Schedule();
    return;
  }

  if (Door_RadioBusy()) {
    if (req != DOOR_REQ_NONE)
      door_deferred_req = req;
    if (evt != DOOR_EVT_NONE)
      door_deferred_evt = evt;
    Door_Schedule();
    return;
  }

  if (req != DOOR_REQ_NONE) {
    door_manual        = 1U;
    door_manual_want   = Door_DesiredDir(now);
    door_retry_pending = 0U;
    door_retry_count   = 0U;
    door_deferred_req  = DOOR_REQ_NONE;
    door_deferred_evt  = DOOR_EVT_NONE;
    Door_Apply((req == DOOR_REQ_OPEN) ? MOTOR_DIR_UP : MOTOR_DIR_DOWN);
  } else if (evt == DOOR_EVT_RETRY) {
    door_retry_pending = 0U;
    Door_Apply(door_retry_dir);
  } else {
    Door_Apply(Door_DesiredDir(now));
  }

  Door_Schedule();

  if (Door_NeedsCatchup())
    door_pending = DOOR_EVT_SUN;
}

void Door_Enable(void)
{
  if (door_enabled)
    return;

  door_enabled = 1U;
  Door_Schedule();
  Door_Catchup();
}

void Door_Disable(void)
{
  door_enabled       = 0U;
  door_request       = DOOR_REQ_NONE;
  door_manual        = 0U;
  door_retry_pending = 0U;
  door_retry_count   = 0U;
  door_deferred_req  = DOOR_REQ_NONE;
  door_deferred_evt  = DOOR_EVT_NONE;
  Door_StopTimer();
  Door_Schedule();
}

uint8_t Door_IsEnabled(void)
{
  return door_enabled;
}

uint8_t Door_WorkPending(void)
{
  return (uint8_t)((door_request != DOOR_REQ_NONE) || (door_pending != DOOR_EVT_NONE));
}

void Door_SetUnixTime(uint32_t unix_sec)
{
  if (unix_sec < TIMEBASE_MIN_UNIX)
    return;

  Timebase_SetUnix(unix_sec);
  door_time_sync_unix = unix_sec;

  Door_Schedule();
  Door_Catchup();
}

void Door_Reschedule(void)
{
  Door_Schedule();
}

void Door_Catchup(void)
{
  if (Door_NeedsCatchup())
    door_pending = DOOR_EVT_SUN;
}

void Door_RequestOpen(void)
{
  if (door_enabled)
    door_request = DOOR_REQ_OPEN;
}

void Door_RequestClose(void)
{
  if (door_enabled)
    door_request = DOOR_REQ_CLOSE;
}

void Door_SetFault(void)
{
  if (door_fault)
    return;

  Door_RaiseFault();
  door_request = DOOR_REQ_NONE;
  Door_StoreState();
  Door_PublishState();
  Door_Schedule();
}

void Door_ClearFault(void)
{
  if (!door_fault)
    return;

  door_fault         = 0U;
  door_retry_pending = 0U;
  door_retry_count   = 0U;
  (void)Endstop_Sample();
  Door_StoreState();
  Door_PublishState();
  Door_Schedule();
  Door_Catchup();
}

Door_State_t Door_GetState(void)
{
  if (door_fault)
    return DOOR_STATE_FAULTED;

  switch (Endstop_Last()) {
    case ENDSTOP_POS_TOP:    return DOOR_STATE_OPEN;
    case ENDSTOP_POS_BOTTOM: return DOOR_STATE_CLOSED;
    default:                 return DOOR_STATE_UNKNOWN;
  }
}
