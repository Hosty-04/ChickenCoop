/**
  ******************************************************************************
  * @file    battery.h
  * @brief   Battery and panel monitoring (INA226 + divider) – MOSFET separator
  ******************************************************************************
  */

#ifndef BATTERY_H
#define BATTERY_H

#include "main.h"

void    Battery_Init(void);
void    Battery_Process(void);
uint8_t Battery_WorkPending(void);

uint8_t  Battery_IsCritical(void);
uint16_t Battery_GetVoltage_mV(void);
uint16_t Battery_GetPanelVoltage_mV(void);

#endif /* BATTERY_H */
