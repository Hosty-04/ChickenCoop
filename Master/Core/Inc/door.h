/**
  ******************************************************************************
  * @file    door.h
  * @brief   Door control (sunrise/sunset based)
  ******************************************************************************
  */

#ifndef DOOR_H
#define DOOR_H

#include "main.h"

#ifndef DOOR_TRUST_FALLBACK_TIME
#define DOOR_TRUST_FALLBACK_TIME  0
#endif

typedef enum {
  DOOR_STATE_CLOSED  = 0,
  DOOR_STATE_OPEN    = 1,
  DOOR_STATE_FAULTED = 2,
  DOOR_STATE_UNKNOWN = 3
} Door_State_t;

void Door_Init(void);
void Door_Setup(uint16_t year, uint8_t month, uint8_t day,
                uint8_t hour, uint8_t min, uint8_t sec,
                float latitude, float longitude);
void Door_Process(void);

void    Door_Enable(void);
void    Door_Disable(void);
uint8_t Door_IsEnabled(void);
uint8_t Door_WorkPending(void);

void Door_SetUnixTime(uint32_t unix_sec);
void Door_Reschedule(void);
void Door_Catchup(void);

void Door_RequestOpen(void);
void Door_RequestClose(void);
void Door_SetFault(void);
void Door_ClearFault(void);

Door_State_t Door_GetState(void);

#endif /* DOOR_H */
