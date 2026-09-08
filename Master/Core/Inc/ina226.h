/**
  ******************************************************************************
  * @file    ina226.h
  * @brief   INA226 voltage/current sensor
  ******************************************************************************
  */

#ifndef INA226_H
#define INA226_H

#include "main.h"

#define INA226_I2C_ADDR   (0x40U << 1)  /* 7-bit 0x40 → HAL 8-bit */

HAL_StatusTypeDef INA226_Init(void);
HAL_StatusTypeDef INA226_Read(float *vbus_V, float *current_A);

#endif /* INA226_H */