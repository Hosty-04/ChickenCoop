/**
  ******************************************************************************
  * @file    door.c
  * @brief   Automatic chicken door controller (sunrise/sunset based)
  ******************************************************************************
  */

#include "door.h"
#include "astro.h"
#include "motor.h"
#include "battery.h"
#include "timebase.h"
#include "stm32_timer.h"
#include "stm32_systime.h"

#define DOOR_OPEN_OFFSET_MIN   (-60)
#define DOOR_CLOSE_OFFSET_MIN  ( 60)
#define DOOR_RETRY_MS          (10UL * 60UL * 1000UL)

static float    lat = 49.5170f;
static float    lon = 17.6181f;

static int16_t  sunrise_min = 360;
static int16_t  sunset_min  = 1080;

typedef enum {
  EVT_NONE = 0,
  EVT_MIDNIGHT,
  EVT_OPEN,
  EVT_CLOSE,
  EVT_RETRY
} DoorEvent_t;

static volatile DoorEvent_t pending_event   = EVT_NONE;
static DoorEvent_t          armed_event     = EVT_NONE;
static uint32_t             door_armed_s    = 0;
static UTIL_TIMER_Object_t  door_timer;

static uint8_t  retry_pending = 0;
static uint8_t  retry_is_open = 0;

static void Door_OnTimer(void *ctx)
{
  UNUSED(ctx);
  Timebase_AdvanceSeconds(door_armed_s);
  pending_event = armed_event;
}

static void Door_ArmTimer(uint32_t seconds, DoorEvent_t evt)
{
  if (seconds == 0U)
    seconds = 1U;
  armed_event  = evt;
  door_armed_s = seconds;
  UTIL_TIMER_Stop(&door_timer);
  UTIL_TIMER_SetPeriod(&door_timer, seconds * 1000U);
  UTIL_TIMER_Start(&door_timer);
}

static void Door_CreateTimer(void)
{
  UTIL_TIMER_Create(&door_timer, 0xFFFFFFFFU, UTIL_TIMER_ONESHOT,
                    Door_OnTimer, NULL);
}

static void Door_UpdateSun(void)
{
  Astro_Result_t res;
  Astro_Calculate(Timebase_GetYear(), Timebase_GetMonth(), Timebase_GetDay(),
                  lat, lon, &res);
  sunrise_min = res.sunrise_min;
  sunset_min  = res.sunset_min;
}

static int32_t Door_AvoidBatteryWindow(int32_t minute_of_day)
{
  switch (minute_of_day % 10) {
    case 9: minute_of_day += 2; break;
    case 0: minute_of_day += 1; break;
    default: break;
  }
  if (minute_of_day >= 1440)
    minute_of_day -= 1440;
  return minute_of_day;
}

static void Door_Schedule(void)
{
  uint32_t now = Timebase_GetSecOfDay();

  if (Battery_IsCritical()) {
    Door_ArmTimer(86400U - now, EVT_MIDNIGHT);
    return;
  }

  int32_t open_m  = (int32_t)sunrise_min + DOOR_OPEN_OFFSET_MIN;
  int32_t close_m = (int32_t)sunset_min  + DOOR_CLOSE_OFFSET_MIN;
  if (open_m  < 0)     open_m  += 1440;
  if (open_m  >= 1440) open_m  -= 1440;
  if (close_m < 0)     close_m += 1440;
  if (close_m >= 1440) close_m -= 1440;

  open_m  = Door_AvoidBatteryWindow(open_m);
  close_m = Door_AvoidBatteryWindow(close_m);

  uint32_t t_mid   = 86400U;
  uint32_t t_open  = (uint32_t)open_m  * 60U;
  uint32_t t_close = (uint32_t)close_m * 60U;
  if (now >= t_open)  t_open  += 86400U;
  if (now >= t_close) t_close += 86400U;

  uint32_t target = t_mid;
  DoorEvent_t evt = EVT_MIDNIGHT;
  if (t_open  < target) { target = t_open;  evt = EVT_OPEN;  }
  if (t_close < target) { target = t_close; evt = EVT_CLOSE; }

  Door_ArmTimer(target - now, evt);
}

static void Door_HandleMotion(uint8_t is_open)
{
  Motor_Result_t r;

  if (Motor_IsFaulted())
    return;

  r = is_open ? Motor_Open() : Motor_Close();

  if (r == MOTOR_OK || r == MOTOR_TIMEOUT) {
    retry_pending = 0;
    return;
  }

  if (r == MOTOR_OBSTACLE) {
    retry_pending = 1;
    retry_is_open = is_open;
    Door_ArmTimer(DOOR_RETRY_MS / 1000UL, EVT_RETRY);
    return;
  }

  if (r == MOTOR_FAULT) {
    retry_pending = 0;
    return;
  }
}

void Door_Init(void)
{
  Door_CreateTimer();
}

void Door_Setup(uint16_t y, uint8_t mo, uint8_t d,
                uint8_t h, uint8_t mi, uint8_t s,
                float latitude, float longitude)
{
  lat = latitude;
  lon = longitude;

  Timebase_Set(y, mo, d, h, mi, s);

  Astro_Result_t res;
  Astro_Calculate(y, mo, d, lat, lon, &res);
  uint32_t unix_sec = Timebase_ToUnix(y, mo, d, h, mi, s, res.timezone);
  SysTime_t st = { .Seconds = unix_sec, .SubSeconds = 0 };
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
    Timebase_FromUnix(t.Seconds, lat, lon);
    Door_UpdateSun();
    Door_Schedule();
  }
}

void Door_ClearFault(void)
{
  Motor_ClearFault();
  retry_pending = 0;
  Door_Schedule();
}

void Door_Process(void)
{
  if (pending_event == EVT_NONE)
    return;

  DoorEvent_t evt = pending_event;
  pending_event = EVT_NONE;

  if (evt == EVT_MIDNIGHT) {
    Door_SyncFromSysTime();
  }
  else if (evt == EVT_OPEN) {
    Door_HandleMotion(1);
    if (!retry_pending && !Motor_IsFaulted())
      Door_Schedule();
  }
  else if (evt == EVT_CLOSE) {
    Door_HandleMotion(0);
    if (!retry_pending && !Motor_IsFaulted())
      Door_Schedule();
  }
  else if (evt == EVT_RETRY) {
    Door_HandleMotion(retry_is_open);
    if (!retry_pending && !Motor_IsFaulted())
      Door_Schedule();
  }
}