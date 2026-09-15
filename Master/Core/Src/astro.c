/**
  ******************************************************************************
  * @file    astro.c
  * @brief   Calculation of sunrise and sunset (NOAA algorithm approximation)
  ******************************************************************************
  */

#include "astro.h"
#include <math.h>

#define ASTRO_PI  3.14159265358979323846f

static uint8_t GetLastSunday(uint16_t year, uint8_t month)
{
  static const uint8_t days[] = {0,31,28,31,30,31,30,31,31,30,31,30,31};
  uint8_t last_day = days[month];
  if (month == 2 && ((year % 4 == 0 && year % 100 != 0) || (year % 400 == 0)))
    last_day = 29;

  static const int t[] = {0,3,2,5,0,3,5,1,4,6,2,4};
  int y  = year - (month < 3);
  int wd = (y + y/4 - y/100 + y/400 + t[month-1] + 1) % 7;   /* 0 = nedele */

  return last_day - ((wd + last_day - 1) % 7);
}

float Astro_Timezone(uint16_t year, uint8_t month, uint8_t day, uint8_t hour_utc)
{
  uint8_t last_sun;

  if (month > 3U && month < 10U) return 2.0f;
  if (month < 3U || month > 10U) return 1.0f;

  if (month == 3U) {
    last_sun = GetLastSunday(year, 3);
    if (day > last_sun) return 2.0f;
    if (day < last_sun) return 1.0f;
    return (hour_utc >= 1U) ? 2.0f : 1.0f;
  }

  last_sun = GetLastSunday(year, 10);
  if (day > last_sun) return 1.0f;
  if (day < last_sun) return 2.0f;
  return (hour_utc >= 1U) ? 1.0f : 2.0f;
}

void Astro_Calculate(uint16_t year, uint8_t month, uint8_t day,
                     float latitude, float longitude, float tz_hours,
                     Astro_Result_t *result)
{
  static const uint8_t days_in_month[] = {0,31,28,31,30,31,30,31,31,30,31,30,31};

  int doy = day;
  for (int m = 1; m < month; m++)
    doy += days_in_month[m];
  if (month > 2 && ((year % 4 == 0 && year % 100 != 0) || (year % 400 == 0)))
    doy++;

  float lat   = latitude * ASTRO_PI / 180.0f;
  float lon   = longitude;
  float gamma = 2.0f * ASTRO_PI / 365.0f * (doy - 1 + 0.5f);

  float eqtime = 229.18f * (0.000075f + 0.001868f * cosf(gamma)
               - 0.032077f * sinf(gamma) - 0.014615f * cosf(2.0f * gamma)
               - 0.040849f * sinf(2.0f * gamma));

  float decl = 0.006918f - 0.399912f * cosf(gamma) + 0.070257f * sinf(gamma)
             - 0.006758f * cosf(2.0f * gamma) + 0.000907f * sinf(2.0f * gamma)
             - 0.002697f * cosf(3.0f * gamma) + 0.00148f  * sinf(3.0f * gamma);

  float cos_ha = cosf(90.833f * ASTRO_PI / 180.0f) / (cosf(lat) * cosf(decl))
               - tanf(lat) * tanf(decl);
  if (cos_ha < -1.0f) cos_ha = -1.0f;
  if (cos_ha >  1.0f) cos_ha =  1.0f;

  float ha = acosf(cos_ha) * 180.0f / ASTRO_PI;

  float sunrise = 720.0f - 4.0f * (lon + ha) - eqtime + tz_hours * 60.0f;
  float sunset  = 720.0f - 4.0f * (lon - ha) - eqtime + tz_hours * 60.0f;

  if (sunrise < 0.0f)     sunrise += 1440.0f;
  if (sunset  < 0.0f)     sunset  += 1440.0f;
  if (sunrise >= 1440.0f) sunrise -= 1440.0f;
  if (sunset  >= 1440.0f) sunset  -= 1440.0f;

  result->sunrise_min = (int16_t)(sunrise + 0.5f);
  result->sunset_min  = (int16_t)(sunset  + 0.5f);
  result->timezone    = tz_hours;
}