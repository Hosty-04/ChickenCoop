/**
  ******************************************************************************
  * @file    astro.h
  * @brief   Calculation of sunrise and sunset
  ******************************************************************************
  */

#ifndef ASTRO_H
#define ASTRO_H

#include <stdint.h>

typedef struct
{
  int16_t sunrise_min;
  int16_t sunset_min;
  float   timezone;
} Astro_Result_t;

void Astro_Calculate(uint16_t year, uint8_t month, uint8_t day,
                     float latitude, float longitude,
                     Astro_Result_t *result);

#endif /* ASTRO_H */