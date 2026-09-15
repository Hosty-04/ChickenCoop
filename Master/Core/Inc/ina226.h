/**
  ******************************************************************************
  * @file    ina226.h
  * @brief   INA226 voltage/current sensor
  ******************************************************************************
  */

#ifndef INA226_H
#define INA226_H

#include "main.h"

#define INA226_I2C_ADDR   (0x40U << 1)

#define INA226_FAST_CONV_MS     40U
#define INA226_BATTERY_CONV_MS  75U

HAL_StatusTypeDef INA226_Read(float *voltage, float *current);

HAL_StatusTypeDef INA226_PowerUp(void);
HAL_StatusTypeDef INA226_Init(void);
HAL_StatusTypeDef INA226_Shutdown(void);
void              INA226_PowerDown(void);

void INA226_ConfigBattery(void);
void INA226_ConfigFast(void);

#endif /* INA226_H */
