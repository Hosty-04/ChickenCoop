/**
  ******************************************************************************
  * @file    nests.h
  * @brief   Nest units (Modbus RTU over RS485)
  ******************************************************************************
  */

#ifndef NESTS_H
#define NESTS_H

#include "main.h"

#define NESTS_COUNT          2U
#define NESTS_CALIB_MASS_G   1000U

#if (NESTS_COUNT < 1U) || (NESTS_COUNT > 15U)
#error "NESTS_COUNT must be between 1 and 15"
#endif

typedef enum {
  NESTS_STATE_OK = 0,
  NESTS_STATE_BROODY,
  NESTS_STATE_UNCALIBRATED,
  NESTS_STATE_FAULT,
  NESTS_STATE_OFFLINE
} Nests_State_t;

void    Nests_Init(void);
void    Nests_Process(void);
uint8_t Nests_WorkPending(void);

void    Nests_Enable(void);
void    Nests_Disable(void);

void    Nests_RequestTare(uint8_t nest);
void    Nests_RequestCalibrate(uint8_t nest);

Nests_State_t Nests_GetState(uint8_t nest);
uint8_t       Nests_GetEggs(uint8_t nest);

#endif /* NESTS_H */
