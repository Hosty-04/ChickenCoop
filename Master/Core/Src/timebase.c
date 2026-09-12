/**
  ******************************************************************************
  * @file    timebase.c
  * @brief   Timebase helper
  ******************************************************************************
  */

#include "timebase.h"
#include "astro.h"
#include "stm32_systime.h"
#include "main.h"

static uint16_t year      = 2026;
static uint8_t  month     = 1;
static uint8_t  day       = 1;
static volatile uint32_t sec_of_day = 0;
static volatile uint32_t tick_ref   = 0;

static void Timebase_AdvanceDays(uint32_t days)
{
  static const uint8_t dim[] = {0,31,28,31,30,31,30,31,31,30,31,30,31};
  while (days--) {
    uint8_t dmax = dim[month];
    if (month == 2 && ((year % 4 == 0 && year % 100 != 0) || (year % 400 == 0)))
      dmax = 29;
    if (++day > dmax) {
      day = 1;
      if (++month > 12) {
        month = 1;
        year++;
      }
    }
  }
}

void Timebase_Set(uint16_t y, uint8_t mo, uint8_t d,
                  uint8_t h, uint8_t mi, uint8_t s)
{
  year  = y;
  month = mo;
  day   = d;
  sec_of_day = (uint32_t)h * 3600U + (uint32_t)mi * 60U + s;
  tick_ref   = HAL_GetTick();
}

void Timebase_AdvanceSeconds(uint32_t seconds)
{
  sec_of_day += seconds;
  if (sec_of_day >= 86400U) {
    uint32_t days = sec_of_day / 86400U;
    sec_of_day %= 86400U;
    Timebase_AdvanceDays(days);
  }
  tick_ref = HAL_GetTick();
}

static void Timebase_SyncFromTick(void)
{
  uint32_t now = HAL_GetTick();
  uint32_t elapsed_s = (now - tick_ref) / 1000U;
  if (elapsed_s == 0U)
    return;
  sec_of_day += elapsed_s;
  tick_ref   += elapsed_s * 1000U;
  if (sec_of_day >= 86400U) {
    uint32_t days = sec_of_day / 86400U;
    sec_of_day %= 86400U;
    Timebase_AdvanceDays(days);
  }
}

uint32_t Timebase_GetSecOfDay(void)
{
  Timebase_SyncFromTick();
  return sec_of_day;
}

uint8_t Timebase_GetDay(void)
{
  Timebase_SyncFromTick();
  return day;
}

uint8_t Timebase_GetMonth(void)
{
  Timebase_SyncFromTick();
  return month;
}

uint16_t Timebase_GetYear(void)
{
  Timebase_SyncFromTick();
  return year;
}

uint32_t Timebase_ToUnix(uint16_t y, uint8_t mo, uint8_t d,
                         uint8_t h, uint8_t mi, uint8_t s, float tz_hours)
{
  int16_t y_adj = (int16_t)y - (mo <= 2 ? 1 : 0);
  uint16_t era = (uint16_t)(y_adj >= 0 ? y_adj : y_adj - 399) / 400U;
  uint16_t yoe = (uint16_t)(y_adj - (int16_t)era * 400);
  uint16_t doy = (uint16_t)((153U * (mo + (mo > 2 ? -3 : 9)) + 2U) / 5U + d - 1U);

  uint32_t doe  = (uint32_t)yoe * 365U + yoe / 4U - yoe / 100U + doy;
  uint32_t days = era * 146097UL + doe - 719468UL;

  int32_t sod_local = (int32_t)h * 3600 + (int32_t)mi * 60 + s;
  int32_t tz_sec    = (int32_t)(tz_hours * 3600.0f);
  int32_t sod_utc   = sod_local - tz_sec;
  if (sod_utc < 0) {
    sod_utc += 86400;
    days--;
  } else if (sod_utc >= 86400) {
    sod_utc -= 86400;
    days++;
  }

  return days * 86400UL + (uint32_t)sod_utc;
}

void Timebase_FromUnix(uint32_t unix_sec, float latitude, float longitude)
{
  int32_t tz_sec = 3600;
  uint32_t local;
  Astro_Result_t res;

  for (int i = 0; i < 2; i++) {
    local = (uint32_t)((int32_t)unix_sec + tz_sec);
    uint32_t days = local / 86400U;
    uint32_t z    = days + 719468U;
    uint32_t era  = z / 146097U;
    uint32_t doe  = z - era * 146097U;
    uint32_t yoe  = (doe - doe/1460U + doe/36524U - doe/146096U) / 365U;
    uint32_t y    = yoe + era * 400U;
    uint32_t doy  = doe - (365U * yoe + yoe/4U - yoe/100U);
    uint32_t mp   = (5U * doy + 2U) / 153U;
    uint32_t d    = doy - (153U * mp + 2U) / 5U + 1U;
    uint32_t m    = mp + (mp < 10U ? 3U : -9U);
    y += (m <= 2U);

    year  = (uint16_t)y;
    month = (uint8_t)m;
    day   = (uint8_t)d;
    Astro_Calculate(year, month, day, latitude, longitude, &res);
    tz_sec = (int32_t)(res.timezone * 3600.0f);
  }

  sec_of_day = local % 86400U;
  tick_ref   = HAL_GetTick();
}