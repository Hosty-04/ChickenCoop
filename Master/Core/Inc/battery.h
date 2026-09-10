/**
  ******************************************************************************
  * @file    battery.h
  * @brief   Battery/panel monitoring (INA226 + divider)
  ******************************************************************************
  */

#ifndef BATTERY_H
#define BATTERY_H

#include <stdint.h>

void Battery_Init(void);
void Battery_Process(void);
uint8_t Battery_IsCritical(void);

#endif /* BATTERY_H */