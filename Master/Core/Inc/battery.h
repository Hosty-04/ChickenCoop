/**
  ******************************************************************************
  * @file    battery.h
  * @brief   Battery and panel monitoring (INA226 + divider) – MOSFET separator
  ******************************************************************************
  */

#ifndef BATTERY_H
#define BATTERY_H

#include "main.h"

void Battery_Init(void);
void Battery_Enable(void);
void Battery_Disable(void);

uint8_t Battery_IsEnabled(void);
uint8_t Battery_IsCritical(void);

uint8_t Battery_WorkPending(void);

uint16_t Battery_GetVoltage_mV(void);
uint16_t Battery_GetPanelVoltage_mV(void);

void Battery_Process(void);

#endif /* BATTERY_H */