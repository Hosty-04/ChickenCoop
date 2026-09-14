/**
  ******************************************************************************
  * @file    timebase.h
  * @brief   Timebase helper
  ******************************************************************************
  */

#ifndef TIMEBASE_H
#define TIMEBASE_H

#include <stdint.h>

#define SECS_PER_DAY  86400UL
#define MINS_PER_DAY  1440

void     Timebase_Set(uint16_t year, uint8_t month, uint8_t day,
                      uint8_t hour, uint8_t min, uint8_t sec);

uint32_t Timebase_GetSecOfDay(void);
uint8_t  Timebase_GetDay(void);
uint8_t  Timebase_GetMonth(void);
uint16_t Timebase_GetYear(void);

uint32_t Timebase_ToUnix(uint16_t y, uint8_t mo, uint8_t d,
                         uint8_t h, uint8_t mi, uint8_t s, float tz_hours);
void     Timebase_FromUnix(uint32_t unix_sec,
                           float latitude, float longitude);

#endif /* TIMEBASE_H */