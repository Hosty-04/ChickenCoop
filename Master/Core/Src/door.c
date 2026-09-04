/**
  ******************************************************************************
  * @file    door.c
  * @brief   Automatic chicken door – schedule open/close around sunrise/sunset
  ******************************************************************************
  */

#include "door.h"
#include "astro.h"
#include "motor.h"
#include "power.h"
#include "stm32_timer.h"
#include "stm32_systime.h"

/* Open this many minutes before sunrise / after sunset */
#define DOOR_OPEN_OFFSET_MIN   (-60)
#define DOOR_CLOSE_OFFSET_MIN  ( 60)

/* Location (default: Týn nad Bečvou, Czech Republic) */
static float    lat = 49.5170f;
static float    lon = 17.6181f;

/* Soft real-time clock (kept in sync with SysTime) */
static uint16_t year  = 2026;
static uint8_t  month = 1;
static uint8_t  day   = 1;
static uint32_t sec_of_day = 0;
static uint32_t tick_ref   = 0;

/* Cached sun times (minutes after midnight) */
static int16_t  sunrise_min = 360;
static int16_t  sunset_min  = 1080;

typedef enum {
  EVT_NONE = 0,
  EVT_MIDNIGHT,
  EVT_OPEN,
  EVT_CLOSE
} DoorEvent_t;

static volatile DoorEvent_t pending_event    = EVT_NONE;
static DoorEvent_t          armed_event      = EVT_NONE;
static uint32_t             armed_seconds    = 0;

static UTIL_TIMER_Object_t  door_timer;

/* ---- calendar helpers --------------------------------------------------- */

static void Time_AdvanceDays(uint32_t days)
{
  static const uint8_t dim[] = {0,31,28,31,30,31,30,31,31,30,31,30,31};

  while (days--)
  {
    uint8_t dmax = dim[month];
    if (month == 2 && ((year % 4 == 0 && year % 100 != 0) || (year % 400 == 0)))
      dmax = 29;

    if (++day > dmax)
    {
      day = 1;
      if (++month > 12)
      {
        month = 1;
        year++;
      }
    }
  }
}

static void Time_SyncFromTick(void)
{
  uint32_t now = HAL_GetTick();
  uint32_t elapsed_s = (now - tick_ref) / 1000U;

  if (elapsed_s == 0U)
    return;

  sec_of_day += elapsed_s;
  tick_ref   += elapsed_s * 1000U;

  if (sec_of_day >= 86400U)
  {
    uint32_t days = sec_of_day / 86400U;
    sec_of_day %= 86400U;
    Time_AdvanceDays(days);
  }
}

static void Time_Set(uint16_t y, uint8_t mo, uint8_t d,
                     uint8_t h, uint8_t mi, uint8_t s)
{
  year       = y;
  month      = mo;
  day        = d;
  sec_of_day = (uint32_t)h * 3600U + (uint32_t)mi * 60U + s;
  tick_ref   = HAL_GetTick();
}

static uint32_t Time_SecOfDay(void)
{
  Time_SyncFromTick();
  return sec_of_day;
}

/** Convert calendar (local) to Unix UTC seconds */
static uint32_t Time_ToUnix(uint16_t y, uint8_t mo, uint8_t d,
                            uint8_t h, uint8_t mi, uint8_t s, float tz_hours)
{
  int16_t y_adj = (int16_t)y - (mo <= 2 ? 1 : 0);
  uint16_t era = (uint16_t)(y_adj >= 0 ? y_adj : y_adj - 399) / 400U;
  uint16_t yoe = (uint16_t)(y_adj - (int16_t)era * 400);
  uint16_t doy = (uint16_t)((153U * (mo + (mo > 2 ? -3 : 9)) + 2U) / 5U + d - 1U);
  uint32_t doe = (uint32_t)yoe * 365U + yoe / 4U - yoe / 100U + doy;
  uint32_t days = era * 146097UL + doe - 719468UL;

  int32_t sod_local = (int32_t)h * 3600 + (int32_t)mi * 60 + s;
  int32_t tz_sec = (int32_t)(tz_hours * 3600.0f);
  int32_t sod_utc = sod_local - tz_sec;

  if (sod_utc < 0)
  {
    sod_utc += 86400;
    days--;
  }
  else if (sod_utc >= 86400)
  {
    sod_utc -= 86400;
    days++;
  }

  return days * 86400UL + (uint32_t)sod_utc;
}

/** Convert Unix UTC to local calendar using CET/CEST */
static void Time_FromUnix(uint32_t unix_sec)
{
  uint32_t local = unix_sec + 3600U;
  uint32_t days = local / 86400U;

  uint32_t z = days + 719468U;
  uint32_t era = z / 146097U;
  uint32_t doe = z - era * 146097U;
  uint32_t yoe = (doe - doe/1460U + doe/36524U - doe/146096U) / 365U;
  uint32_t y = yoe + era * 400U;
  uint32_t doy = doe - (365U * yoe + yoe/4U - yoe/100U);
  uint32_t mp = (5U * doy + 2U) / 153U;
  uint32_t d = doy - (153U * mp + 2U) / 5U + 1U;
  uint32_t m = mp + (mp < 10U ? 3U : -9U);
  y += (m <= 2U);

  year  = (uint16_t)y;
  month = (uint8_t)m;
  day   = (uint8_t)d;

  Astro_Result_t res;
  Astro_Calculate(year, month, day, lat, lon, &res);
  int32_t tz_sec = (int32_t)(res.timezone * 3600.0f);

  local = (uint32_t)((int32_t)unix_sec + tz_sec);
  sec_of_day = local % 86400U;
  tick_ref   = HAL_GetTick();
}

/* ---- timer / schedule --------------------------------------------------- */

static void Door_OnTimer(void *ctx)
{
  UNUSED(ctx);

  sec_of_day += armed_seconds;
  if (sec_of_day >= 86400U)
  {
    uint32_t days = sec_of_day / 86400U;
    sec_of_day %= 86400U;
    Time_AdvanceDays(days);
  }
  tick_ref = HAL_GetTick();

  pending_event = armed_event;
}

static void Door_ArmTimer(uint32_t seconds, DoorEvent_t evt)
{
  if (seconds == 0U)
    seconds = 1U;

  armed_event   = evt;
  armed_seconds = seconds;

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
  Time_SyncFromTick();

  Astro_Result_t res;
  Astro_Calculate(year, month, day, lat, lon, &res);
  sunrise_min = res.sunrise_min;
  sunset_min  = res.sunset_min;
}

static void Door_Schedule(void)
{
  uint32_t now = Time_SecOfDay();

  int32_t open_m  = (int32_t)sunrise_min + DOOR_OPEN_OFFSET_MIN;
  int32_t close_m = (int32_t)sunset_min  + DOOR_CLOSE_OFFSET_MIN;

  if (open_m  < 0)     open_m  += 1440;
  if (close_m >= 1440) close_m -= 1440;
  if (close_m < 0)     close_m += 1440;

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

/* ---- public API --------------------------------------------------------- */

void Door_Init(void)
{
  Power_SwitchToRun48MHz();
  Door_CreateTimer();
}

void Door_Setup(uint16_t y, uint8_t mo, uint8_t d,
                uint8_t h, uint8_t mi, uint8_t s,
                float latitude, float longitude)
{
  lat = latitude;
  lon = longitude;

  Time_Set(y, mo, d, h, mi, s);

  /* Share the same absolute time with LoRaWAN stack */
  Astro_Result_t res;
  Astro_Calculate(y, mo, d, lat, lon, &res);
  uint32_t unix = Time_ToUnix(y, mo, d, h, mi, s, res.timezone);

  SysTime_t st = { .Seconds = unix, .SubSeconds = 0 };
  SysTimeSet(st);

  Door_UpdateSun();
  Door_Schedule();
}

void Door_SyncFromSysTime(void)
{
  SysTime_t t = SysTimeGet();
  if (t.Seconds > 0U)
  {
    Time_FromUnix(t.Seconds);
    Door_UpdateSun();
    Door_Schedule();
  }
}

void Door_Process(void)
{
  if (pending_event == EVT_NONE)
    return;

  DoorEvent_t evt = pending_event;
  pending_event = EVT_NONE;

  if (evt == EVT_MIDNIGHT)
  {
    Power_SwitchToRun48MHz();
    /* limit long-term drift: re-read SysTime every midnight */
    Door_SyncFromSysTime();
  }
  else if (evt == EVT_OPEN)
  {
    Power_SwitchToLPRunMSI1MHz();
    Motor_Open();
    Power_SwitchToRun48MHz();
    Door_Schedule();
  }
  else if (evt == EVT_CLOSE)
  {
    Power_SwitchToLPRunMSI1MHz();
    Motor_Close();
    Power_SwitchToRun48MHz();
    Door_Schedule();
  }
}