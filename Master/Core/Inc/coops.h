/**
  ******************************************************************************
  * @file    coops.h
  * @brief   Nest units (Modbus RTU over RS485)
  ******************************************************************************
  */

#ifndef COOPS_H
#define COOPS_H

#include "main.h"

#define COOPS_NEST_COUNT     2U
#define COOPS_CALIB_MASS_G   1000U

#if (COOPS_NEST_COUNT < 1U) || (COOPS_NEST_COUNT > 15U)
#error "COOPS_NEST_COUNT must be between 1 and 15"
#endif

typedef enum {
  COOPS_NEST_OK = 0,
  COOPS_NEST_BROODY,
  COOPS_NEST_UNCALIBRATED,
  COOPS_NEST_FAULT,
  COOPS_NEST_OFFLINE
} Coops_State_t;

void    Coops_Init(void);
void    Coops_Process(void);
uint8_t Coops_WorkPending(void);

void    Coops_Enable(void);
void    Coops_Disable(void);

void    Coops_RequestTare(uint8_t nest);
void    Coops_RequestCalibrate(uint8_t nest);

Coops_State_t Coops_GetState(uint8_t nest);
uint8_t       Coops_GetEggs(uint8_t nest);

#endif /* COOPS_H */
