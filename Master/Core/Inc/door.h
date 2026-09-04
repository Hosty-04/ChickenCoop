/**
  ******************************************************************************
  * @file    door.h
  * @brief   Automatic chicken door controller (sunrise/sunset based)
  ******************************************************************************
  */

#ifndef DOOR_H
#define DOOR_H

#include "main.h"

void Door_Init(void);
void Door_Process(void);
void Door_Setup(uint16_t year, uint8_t month, uint8_t day,
                uint8_t hour, uint8_t min, uint8_t sec,
                float latitude, float longitude);

/**
  * @brief  Synchronize internal date/time from SysTime
  */
void Door_SyncFromSysTime(void);

#endif /* DOOR_H */