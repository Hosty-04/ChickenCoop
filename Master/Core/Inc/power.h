/**
  ******************************************************************************
  * @file    power.h
  * @brief   Power mode control
  ******************************************************************************
  */

#ifndef POWER_H
#define POWER_H

#include <stdint.h>

void Power_Init(void);

void Power_SwitchToLPRunMSI2MHz(void);
void Power_SwitchToRunHSE48MHz(void);

uint8_t Power_LockMSI(void);
void    Power_UnlockMSI(void);

void Power_DisableDebug(void);

#endif /* POWER_H */
