/**
  ******************************************************************************
  * @file    system.h
  * @brief   Execution order and on/off switch
  ******************************************************************************
  */

#ifndef SYSTEM_H
#define SYSTEM_H

#include "main.h"

void System_Process(void);

void    System_Enable(void);
void    System_Disable(void);
uint8_t System_IsEnabled(void);

uint8_t System_WorkPending(void);

#endif /* SYSTEM_H */