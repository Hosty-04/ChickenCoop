/**
  ******************************************************************************
  * @file    ina226.h
  * @brief   INA226 voltage/current sensor
  ******************************************************************************
  */

#ifndef INA226_H
#define INA226_H

#include "main.h"

HAL_StatusTypeDef INA226_PowerUp(void);
void              INA226_PowerDown(void);

HAL_StatusTypeDef INA226_Init(void);
HAL_StatusTypeDef INA226_ConfigBattery(void);
HAL_StatusTypeDef INA226_ConfigFast(void);

HAL_StatusTypeDef INA226_Read(float *voltage, float *current);

#endif /* INA226_H */
