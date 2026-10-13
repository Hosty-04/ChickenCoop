/**
  ******************************************************************************
  * @file    power.h
  * @brief   Power mode control and hand-over to the next nest
  ******************************************************************************
  */

#ifndef POWER_H
#define POWER_H

#include "main.h"

void Power_LowPowerRun(void);
void Power_MainRegulator(void);
void Power_LowRegulator(void);
void Power_Release(uint8_t next);

#endif /* POWER_H */
