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
#define ASTRO_ZENITH   (90.833f * ASTRO_DEG2RAD)

static int32_t Astro_DaysSince2000(uint16_t year, uint8_t month, uint8_t day)
{
  int32_t y  = (int32_t)year - ((month <= 2U) ? 1 : 0);
  int32_t mp = (month > 2U) ? ((int32_t)month - 3) : ((int32_t)month + 9);

  return y * 365 + y / 4 - y / 100 + y / 400 + (153 * mp + 2) / 5 + (int32_t)day - 730426;
}

static float Astro_EventUtc(float n, float lat, float longitude, float sign)
{
  float l0, m, om, lam, eps, y, eqtime, decl, cos_ha;

  l0  = fmodf(280.46646f + 0.98564736f * n, 360.0f) * ASTRO_DEG2RAD;
  m   = fmodf(357.52911f + 0.98560028f * n, 360.0f) * ASTRO_DEG2RAD;
  om  = (125.04f - 0.05295377f * n) * ASTRO_DEG2RAD;
  lam = l0 + (1.914602f * sinf(m) + 0.019993f * sinf(2.0f * m)
            - 0.00569f - 0.00478f * sinf(om)) * ASTRO_DEG2RAD;
  eps = (23.439291f - 3.563e-7f * n + 0.00256f * cosf(om)) * ASTRO_DEG2RAD;
  y   = tanf(0.5f * eps);
  y  *= y;

  eqtime = 4.0f / ASTRO_DEG2RAD * (y * sinf(2.0f * l0) - 0.033417f * sinf(m)
         + 0.066835f * y * sinf(m) * cosf(2.0f * l0) - 0.5f * y * y * sinf(4.0f * l0)
         - 0.000349f * sinf(2.0f * m));
  decl   = asinf(sinf(eps) * sinf(lam));

  cos_ha = cosf(ASTRO_ZENITH) / (cosf(lat) * cosf(decl)) - tanf(lat) * tanf(decl);
  if (cos_ha < -1.0f) cos_ha = -1.0f;
  if (cos_ha >  1.0f) cos_ha =  1.0f;

  return 720.0f - 4.0f * (longitude + sign * acosf(cos_ha) / ASTRO_DEG2RAD) - eqtime;
}

void Astro_Calculate(uint16_t year, uint8_t month, uint8_t day,
                     float latitude, float longitude, int16_t tz_min,
                     Astro_Result_t *result)
{
  float n   = (float)Astro_DaysSince2000(year, month, day) - 0.5f;
  float lat = latitude * ASTRO_DEG2RAD;

  result->sunrise_min = (int16_t)floorf(Astro_EventUtc(n + 0.25f, lat, longitude,  1.0f) + (float)tz_min + 0.5f);
  result->sunset_min  = (int16_t)floorf(Astro_EventUtc(n + 0.75f, lat, longitude, -1.0f) + (float)tz_min + 0.5f);
}
