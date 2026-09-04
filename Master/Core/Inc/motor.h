/**
  ******************************************************************************
  * @file    motor.h
  * @brief   DC motor driver (PWM + direction)
  ******************************************************************************
  */

#ifndef MOTOR_H
#define MOTOR_H

#include "main.h"

void Motor_Begin(void);
void Motor_End(void);
void Motor_Set(uint8_t forward, uint16_t duty);
void Motor_Stop(void);
void Motor_Open(void);
void Motor_Close(void);

#endif /* MOTOR_H */