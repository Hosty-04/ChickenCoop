/**
  ******************************************************************************
  * @file    timebase.c
  * @brief   Timebase helper
  ******************************************************************************
  */

#include "timebase.h"
#include "astro.h"
#include "rtc.h"

#define TIMEBASE_BKP_UNIX   RTC_BKP_DR5
#define TIMEBASE_BKP_TICK   RTC_BKP_DR6
#define TIMEBASE_SAVE_S     3600UL

static uint32_t unix_ref   = TIMEBASE_MIN_UNIX;
static uint32_t tick_ref   = 0;
static uint32_t saved_unix = 0;
static uint8_t  time_valid = 0;

static uint32_t cache_unix  = 0xFFFFFFFFUL;
static uint32_t cache_sod   = 0;
static uint16_t cache_year  = 2026;
static uint8_t  cache_month = 1;
static uint8_t  cache_day   = 1;
static float    cache_tz    = 1.0f;

static void Timebase_CivilFromDays(uint32_t days, uint16_t *y_out,
                                   uint8_t *m_out, uint8_t *d_out)
{
  uint32_t z   = days + 719468U;
  uint32_t era = z / 146097U;
  uint32_t doe = z - era * 146097U;
  uint32_t yoe = (doe - doe / 1460U + doe / 36524U - doe / 146096U) / 365U;
  uint32_t doy = doe - (365U * yoe + yoe / 4U - yoe / 100U);
  uint32_t mp  = (5U * doy + 2U) / 153U;
  uint32_t d   = doy - (153U * mp + 2U) / 5U + 1U;
  uint32_t m   = (mp < 10U) ? (mp + 3U) : (mp - 9U);
  uint32_t y   = yoe + era * 400U + ((m <= 2U) ? 1U : 0U);

  *y_out = (uint16_t)y;
  *m_out = (uint8_t)m;
  *d_out = (uint8_t)d;
}

static uint32_t Timebase_ToUnix(uint16_t y, uint8_t mo, uint8_t d,
                                uint8_t h, uint8_t mi, uint8_t s, float tz_hours)
{
  int32_t yy   = (int32_t)y - ((mo <= 2U) ? 1 : 0);
  int32_t era  = yy / 400;
  int32_t yoe  = yy - era * 400;
  int32_t mp   = (mo > 2U) ? ((int32_t)mo - 3) : ((int32_t)mo + 9);
  int32_t doy  = (153 * mp + 2) / 5 + (int32_t)d - 1;
  int32_t doe  = yoe * 365 + yoe / 4 - yoe / 100 + doy;
  int32_t days = era * 146097 + doe - 719468;
  int32_t sec  = (int32_t)h * 3600 + (int32_t)mi * 60 + (int32_t)s
               - (int32_t)(tz_hours * 3600.0f);

  return (uint32_t)((int64_t)days * (int64_t)SECS_PER_DAY + sec);
}

static float Timebase_ZoneAt(uint32_t utc)
{
  uint16_t y;
  uint8_t  m, d;

  Timebase_CivilFromDays(utc / SECS_PER_DAY, &y, &m, &d);

  return Astro_Timezone(y, m, d, (uint8_t)((utc % SECS_PER_DAY) / 3600UL));
}

static void Timebase_Save(void)
{
  HAL_PWR_EnableBkUpAccess();
  HAL_RTCEx_BKUPWrite(&hrtc, TIMEBASE_BKP_UNIX, unix_ref);
  HAL_RTCEx_BKUPWrite(&hrtc, TIMEBASE_BKP_TICK, tick_ref);
  saved_unix = unix_ref;
}

static void Timebase_Refresh(void)
{
  uint32_t utc = Timebase_GetUnix();
  uint32_t local;

  if (utc == cache_unix)
    return;

  cache_unix = utc;
  cache_tz   = Timebase_ZoneAt(utc);
  local      = utc + (uint32_t)(int32_t)(cache_tz * 3600.0f);

  Timebase_CivilFromDays(local / SECS_PER_DAY, &cache_year, &cache_month, &cache_day);
  cache_sod = local % SECS_PER_DAY;
}

void Timebase_SetUnix(uint32_t unix_sec)
{
  unix_ref   = unix_sec;
  tick_ref   = HAL_GetTick();
  time_valid = 1U;
  cache_unix = 0xFFFFFFFFUL;
  Timebase_Save();
}

void Timebase_SetFallback(uint32_t unix_sec)
{
  unix_ref   = unix_sec;
  tick_ref   = HAL_GetTick();
  time_valid = 0U;
  cache_unix = 0xFFFFFFFFUL;
}

uint8_t Timebase_Restore(void)
{
  uint32_t unix_sec = HAL_RTCEx_BKUPRead(&hrtc, TIMEBASE_BKP_UNIX);

  if (unix_sec < TIMEBASE_MIN_UNIX)
    return 0U;

  unix_ref   = unix_sec;
  tick_ref   = HAL_RTCEx_BKUPRead(&hrtc, TIMEBASE_BKP_TICK);
  saved_unix = unix_sec;
  time_valid = 1U;
  cache_unix = 0xFFFFFFFFUL;

  return 1U;
}

uint8_t Timebase_IsValid(void)
{
  return time_valid;
}

uint32_t Timebase_GetUnix(void)
{
  uint32_t elapsed_s = (HAL_GetTick() - tick_ref) / TICKS_PER_SEC;

  unix_ref += elapsed_s;
  tick_ref += elapsed_s * TICKS_PER_SEC;

  if (time_valid && ((unix_ref - saved_unix) >= TIMEBASE_SAVE_S))
    Timebase_Save();

  return unix_ref;
}

uint32_t Timebase_GetSecOfDay(void)
{
  Timebase_Refresh();
  return cache_sod;
}

uint8_t Timebase_GetDay(void)
{
  Timebase_Refresh();
  return cache_day;
}

uint8_t Timebase_GetMonth(void)
{
  Timebase_Refresh();
  return cache_month;
}

uint16_t Timebase_GetYear(void)
{
  Timebase_Refresh();
  return cache_year;
}

float Timebase_GetTimezone(void)
{
  Timebase_Refresh();
  return cache_tz;
}

uint32_t Timebase_LocalToUnix(uint16_t y, uint8_t mo, uint8_t d,
                              uint8_t h, uint8_t mi, uint8_t s)
{
  uint32_t guess = Timebase_ToUnix(y, mo, d, h, mi, s, 1.0f);

  return Timebase_ToUnix(y, mo, d, h, mi, s, Timebase_ZoneAt(guess));
}
