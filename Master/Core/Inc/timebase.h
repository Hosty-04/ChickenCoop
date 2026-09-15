/**
  ******************************************************************************
  * @file    timebase.h
  * @brief   Timebase helper
  *
  * Vnitrni reprezentace casu je UTC (unixove sekundy). Lokalni datum, cas
  * a casove pasmo se odvozuji az pri dotazu, takze prechod letni/zimni cas
  * nastane presne v 01:00 UTC a nezavisi na tom, kdy naposledy probehla
  * synchronizace.
  ******************************************************************************
  */

#ifndef TIMEBASE_H
#define TIMEBASE_H

#include <stdint.h>
#include "main.h"                     /* RTC_N_PREDIV_S */

#define SECS_PER_DAY  86400UL
#define MINS_PER_DAY  1440

/*
 * POZOR: HAL_GetTick() je v tomto projektu prepsana v sys_app.c na
 * TIMER_IF_GetTimerValue(), coz jsou RTC ticky, NE milisekundy.
 * Pri RTC_N_PREDIV_S = 10 bezi na 1024 Hz.
 */
#define TICKS_PER_SEC       (1UL << RTC_N_PREDIV_S)
#define MS_TO_TICKS(ms)     ((uint32_t)(((uint64_t)(ms) * TICKS_PER_SEC) / 1000U))
#define TICKS_TO_MS(t)      ((uint32_t)(((uint64_t)(t) * 1000U) / TICKS_PER_SEC))

/* 2026-01-01 00:00:00 UTC - pod timto datem povazujeme cas za nenastaveny */
#define TIMEBASE_MIN_UNIX   1767225600UL

/**
  * @brief  Nastaveni overeneho casu (ze site). Oznaci timebase jako platny.
  */
void     Timebase_SetUnix(uint32_t unix_sec);

/**
  * @brief  Nastaveni zalozniho casu pri startu. Timebase zustane oznaceny
  *         jako neplatny, dokud nedorazi cas ze site.
  */
void     Timebase_SetFallback(uint32_t unix_sec);

/**
  * @brief  Dorazil uz cas ze site?
  */
uint8_t  Timebase_IsValid(void);

uint32_t Timebase_GetUnix(void);

/* Lokalni cas - pasmo se vyhodnocuje pri kazdem dotazu */
uint32_t Timebase_GetSecOfDay(void);
uint8_t  Timebase_GetDay(void);
uint8_t  Timebase_GetMonth(void);
uint16_t Timebase_GetYear(void);
float    Timebase_GetTimezone(void);

uint32_t Timebase_ToUnix(uint16_t y, uint8_t mo, uint8_t d,
                         uint8_t h, uint8_t mi, uint8_t s, float tz_hours);

#endif /* TIMEBASE_H */
