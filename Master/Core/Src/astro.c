/**
  ******************************************************************************
  * @file    astro.c
  * @brief   Calculation of sunrise and sunset (NOAA algorithm approximation)
  ******************************************************************************
  */

#include "astro.h"
#include <math.h>

#define ASTRO_PI       3.14159265358979323846f
#define ASTRO_DEG2RAD  (ASTRO_PI / 180.0f)

static const uint8_t astro_days_in_month[13] = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

static uint8_t Astro_IsLeap(uint16_t year)
{
  return (uint8_t)((((year % 4U) == 0U) && ((year % 100U) != 0U)) || ((year % 400U) == 0U));
}

void Astro_Calculate(uint16_t year, uint8_t month, uint8_t day,
                     float latitude, float longitude, int16_t tz_min,
                     Astro_Result_t *result)
{
  float   lat, gamma, eqtime, decl, cos_ha, ha, sunrise, sunset;
  int32_t doy = day;
  uint8_t m;

  for (m = 1U; m < month; m++)
    doy += astro_days_in_month[m];
  if ((month > 2U) && Astro_IsLeap(year))
    doy++;

  lat   = latitude * ASTRO_DEG2RAD;
  gamma = 2.0f * ASTRO_PI / 365.0f * (float)(doy - 1);

  eqtime = 229.18f * (0.000075f + 0.001868f * cosf(gamma)
         - 0.032077f * sinf(gamma) - 0.014615f * cosf(2.0f * gamma)
         - 0.040849f * sinf(2.0f * gamma));

  decl = 0.006918f - 0.399912f * cosf(gamma) + 0.070257f * sinf(gamma)
       - 0.006758f * cosf(2.0f * gamma) + 0.000907f * sinf(2.0f * gamma)
       - 0.002697f * cosf(3.0f * gamma) + 0.00148f  * sinf(3.0f * gamma);

  cos_ha = cosf(90.833f * ASTRO_DEG2RAD) / (cosf(lat) * cosf(decl))
         - tanf(lat) * tanf(decl);
  if (cos_ha < -1.0f) cos_ha = -1.0f;
  if (cos_ha >  1.0f) cos_ha =  1.0f;

  ha = acosf(cos_ha) / ASTRO_DEG2RAD;

  sunrise = 720.0f - 4.0f * (longitude + ha) - eqtime + (float)tz_min;
  sunset  = 720.0f - 4.0f * (longitude - ha) - eqtime + (float)tz_min;

  if (sunrise < 0.0f)     sunrise += 1440.0f;
  if (sunset  < 0.0f)     sunset  += 1440.0f;
  if (sunrise >= 1440.0f) sunrise -= 1440.0f;
  if (sunset  >= 1440.0f) sunset  -= 1440.0f;

  result->sunrise_min = (int16_t)(sunrise + 0.5f);
  result->sunset_min  = (int16_t)(sunset  + 0.5f);
}
