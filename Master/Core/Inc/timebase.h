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
#include "main.h"

#define SECS_PER_DAY        86400UL
#define MINS_PER_DAY        1440

#define TICKS_PER_SEC       (1UL << RTC_N_PREDIV_S)
#define MS_TO_TICKS(ms)     ((uint32_t)(((uint64_t)(ms) * TICKS_PER_SEC) / 1000U))
#define TICKS_TO_MS(t)      ((uint32_t)(((uint64_t)(t) * 1000U) / TICKS_PER_SEC))

#define TIMEBASE_MIN_UNIX   1767222000UL
#define TIMEBASE_MAX_UNIX   4102444800UL

void     Timebase_SetUnix(uint32_t unix_sec);
void     Timebase_SetFallback(uint32_t unix_sec);
uint8_t  Timebase_Restore(void);
uint8_t  Timebase_IsValid(void);
uint32_t Timebase_GetUnix(void);

uint32_t Timebase_GetSecOfDay(void);
uint8_t  Timebase_GetDay(void);
uint8_t  Timebase_GetMonth(void);
uint16_t Timebase_GetYear(void);
uint32_t Timebase_GetDateKey(void);
int16_t  Timebase_GetTimezoneMin(void);

uint32_t Timebase_LocalToUnix(uint16_t year, uint8_t month, uint8_t day,
                              uint8_t hour, uint8_t min, uint8_t sec);

#endif /* TIMEBASE_H */
