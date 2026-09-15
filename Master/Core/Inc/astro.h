/**
  ******************************************************************************
  * @file    astro.h
  * @brief   Calculation of sunrise and sunset (NOAA algorithm approximation)
  ******************************************************************************
  */

#ifndef ASTRO_H
#define ASTRO_H

#include <stdint.h>

typedef struct {
  int16_t sunrise_min;
  int16_t sunset_min;
  float   timezone;
} Astro_Result_t;

float Astro_Timezone(uint16_t year, uint8_t month, uint8_t day, uint8_t hour_utc);

void Astro_Calculate(uint16_t year, uint8_t month, uint8_t day,
                     float latitude, float longitude, float tz_hours,
                     Astro_Result_t *result);

#endif /* ASTRO_H */