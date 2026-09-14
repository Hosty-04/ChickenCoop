/**
  ******************************************************************************
  * @file    motor.h
  * @brief   Motor control and obstacle detection (INA226)
  ******************************************************************************
  */

#ifndef MOTOR_H
#define MOTOR_H

#include "main.h"

typedef enum {
  MOTOR_DIR_DOWN = 0,
  MOTOR_DIR_UP
} Motor_Dir_t;

typedef enum {
  MOTOR_OK = 0,
  MOTOR_ALREADY,
  MOTOR_NO_REFERENCE,
  MOTOR_OBSTACLE,
  MOTOR_STUCK,
  MOTOR_TIMEOUT,
  MOTOR_SENSOR_ERROR
} Motor_Result_t;

Motor_Result_t Motor_Move(Motor_Dir_t dir);

#endif /* MOTOR_H */