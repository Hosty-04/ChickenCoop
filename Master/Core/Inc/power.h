/**
  ******************************************************************************
  * @file    power.h
  * @brief   Power mode switching
  ******************************************************************************
  */

#ifndef POWER_H
#define POWER_H

void Power_SwitchToRunHSE48MHz(void);
void Power_SwitchToRunMSI16MHz(void);
void Power_SwitchToLPRunMSI1MHz(void);

#endif /* POWER_H */