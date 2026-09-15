/**
  ******************************************************************************
  * @file    timebase.c
  * @brief   Timebase helper
  ******************************************************************************
  */

#include "timebase.h"
#include "astro.h"
#include "main.h"

/* --- stav ----------------------------------------------------------------- */

static volatile uint32_t unix_ref   = TIMEBASE_MIN_UNIX;  /* UTC sekundy */
static volatile uint32_t tick_ref   = 0;
static uint8_t           time_valid = 0;

/* cache rozkladu na lokalni cas - plati vzdy pro jednu UTC sekundu */
static uint32_t cache_unix  = 0xFFFFFFFFUL;
static uint16_t cache_year  = 2026;
static uint8_t  cache_month = 1;
static uint8_t  cache_day   = 1;
static uint32_t cache_sod   = 0;
static float    cache_tz    = 1.0f;

/* --- prevody -------------------------------------------------------------- */

/* Howard Hinnant: dny od 1970-01-01 -> obcansky kalendar */
static void Timebase_CivilFromDays(uint32_t days, uint16_t *y_out,
                                   uint8_t *m_out, uint8_t *d_out)
{
  uint32_t z   = days + 719468U;
  uint32_t era = z / 146097U;
  uint32_t doe = z - era * 146097U;
  uint32_t yoe = (doe - doe / 1460U + doe / 36524U - doe / 146096U) / 365U;
  uint32_t y   = yoe + era * 400U;
  uint32_t doy = doe - (365U * yoe + yoe / 4U - yoe / 100U);
  uint32_t mp  = (5U * doy + 2U) / 153U;
  uint32_t d   = doy - (153U * mp + 2U) / 5U + 1U;
  uint32_t m   = mp + ((mp < 10U) ? 3U : (uint32_t)(-9));

  y += (m <= 2U);

  *y_out = (uint16_t)y;
  *m_out = (uint8_t)m;
  *d_out = (uint8_t)d;
}

uint32_t Timebase_ToUnix(uint16_t y, uint8_t mo, uint8_t d,
                         uint8_t h, uint8_t mi, uint8_t s, float tz_hours)
{
  int32_t  y_adj = (int32_t)y - ((mo <= 2U) ? 1 : 0);
  int32_t  era   = ((y_adj >= 0) ? y_adj : (y_adj - 399)) / 400;
  uint32_t yoe   = (uint32_t)(y_adj - era * 400);
  uint32_t doy   = (uint32_t)((153U * ((uint32_t)mo + ((mo > 2U) ? (uint32_t)(-3) : 9U)) + 2U) / 5U
                 + (uint32_t)d - 1U);
  uint32_t doe   = yoe * 365U + yoe / 4U - yoe / 100U + doy;
  int32_t  days  = era * 146097 + (int32_t)doe - 719468;

  int32_t sod_local = (int32_t)h * 3600 + (int32_t)mi * 60 + (int32_t)s;
  int32_t tz_sec    = (int32_t)(tz_hours * 3600.0f);
  int32_t sod_utc   = sod_local - tz_sec;

  if (sod_utc < 0) {
    sod_utc += 86400;
    days--;
  } else if (sod_utc >= 86400) {
    sod_utc -= 86400;
    days++;
  }

  return (uint32_t)days * 86400UL + (uint32_t)sod_utc;
}

/* --- jadro ---------------------------------------------------------------- */

uint32_t Timebase_GetUnix(void)
{
  uint32_t now           = HAL_GetTick();
  uint32_t elapsed_ticks = now - tick_ref;       /* korektni i pres preteceni */
  uint32_t elapsed_s;

  if (elapsed_ticks >= TICKS_PER_SEC) {
    elapsed_s  = elapsed_ticks / TICKS_PER_SEC;
    unix_ref  += elapsed_s;
    tick_ref  += elapsed_s * TICKS_PER_SEC;      /* zbytek zustava v tick_ref */
  }

  return unix_ref;
}

/*
 * Pasmo se urcuje z UTC data a hodiny, takze prechod nastane presne
 * v 01:00 UTC. Vysledek cachujeme na jednu sekundu, aby opakovane
 * dotazy behem jedne udalosti nestaly nic navic.
 */
static void Timebase_Refresh(void)
{
  uint32_t utc = Timebase_GetUnix();
  uint16_t y_utc;
  uint8_t  m_utc, d_utc;
  uint32_t local;

  if (utc == cache_unix)
    return;

  cache_unix = utc;

  Timebase_CivilFromDays(utc / 86400U, &y_utc, &m_utc, &d_utc);
  cache_tz = Astro_Timezone(y_utc, m_utc, d_utc, (uint8_t)((utc % 86400U) / 3600U));

  local = (uint32_t)((int32_t)utc + (int32_t)(cache_tz * 3600.0f));

  Timebase_CivilFromDays(local / 86400U, &cache_year, &cache_month, &cache_day);
  cache_sod = local % 86400U;
}

void Timebase_SetUnix(uint32_t unix_sec)
{
  unix_ref   = unix_sec;
  tick_ref   = HAL_GetTick();
  time_valid = 1;
  cache_unix = 0xFFFFFFFFUL;                     /* invalidace cache */
}

void Timebase_SetFallback(uint32_t unix_sec)
{
  unix_ref   = unix_sec;
  tick_ref   = HAL_GetTick();
  time_valid = 0;
  cache_unix = 0xFFFFFFFFUL;
}

uint8_t Timebase_IsValid(void)
{
  return time_valid;
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
