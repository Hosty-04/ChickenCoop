/**
  ******************************************************************************
  * @file    motor.h
  * @brief   Motor driver + obstacle detection (INA226)
  ******************************************************************************
  */

#ifndef MOTOR_H
#define MOTOR_H

#include "main.h"

typedef enum {
  MOTOR_OK = 0,
  MOTOR_OBSTACLE,
  MOTOR_TIMEOUT,
  MOTOR_FAULT
} Motor_Result_t;

void Motor_Begin(void);
void Motor_End(void);
void Motor_Set(uint8_t up, uint16_t duty);
void Motor_Stop(void);

Motor_Result_t Motor_Open(void);
Motor_Result_t Motor_Close(void);

void Motor_ClearFault(void);
void Motor_SetFault(void);
uint8_t Motor_IsFaulted(void);

#endif /* MOTOR_H */