/**
  ******************************************************************************
  * @file    door.h
  * @brief   Door control (sunrise/sunset based)
  ******************************************************************************
  */

#ifndef DOOR_H
#define DOOR_H

#include "main.h"

typedef enum {
  DOOR_STATE_CLOSED  = 0,
  DOOR_STATE_OPEN    = 1,
  DOOR_STATE_JAMMED  = 2,
  DOOR_STATE_UNKNOWN = 3
} Door_State_t;

void Door_Init(void);
void Door_Setup(uint16_t year, uint8_t month, uint8_t day,
                uint8_t hour, uint8_t min, uint8_t sec,
                float latitude, float longitude);

void Door_Reschedule(void);

void Door_SyncFromSysTime(void);

void Door_Catchup(void);

/* --- Manual control; execute in Door_Process() -------------------- */
void Door_RequestOpen(void);
void Door_RequestClose(void);
void Door_SetFault(void);
void Door_ClearFault(void);

void    Door_Enable(void);
void    Door_Disable(void);
uint8_t Door_IsEnabled(void);
uint8_t Door_IsFaulted(void);

uint8_t Door_WorkPending(void);

Door_State_t Door_GetState(void);
Door_State_t Door_RefreshState(void);

void Door_Process(void);

#endif /* DOOR_H */