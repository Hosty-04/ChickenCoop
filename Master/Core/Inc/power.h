/**
  ******************************************************************************
  * @file    power.h
  * @brief   Power mode control
  ******************************************************************************
  */

#ifndef POWER_H
#define POWER_H

#include <stdint.h>

void Power_SwitchToRunHSE48MHz(void);

void Power_SwitchToLPRunMSI1MHz(void);
uint8_t Power_IsLowPowerRun(void);

void Power_DisableDebug(void);

#endif /* POWER_H */