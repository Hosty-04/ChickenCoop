/**
  ******************************************************************************
  * @file    endstop.h
  * @brief   Limit switches
  ******************************************************************************
  */

#ifndef ENDSTOP_H
#define ENDSTOP_H

#include "main.h"

typedef enum {
  ENDSTOP_POS_UNKNOWN = 0,
  ENDSTOP_POS_BOTTOM,
  ENDSTOP_POS_TOP
} Endstop_Pos_t;

void Endstop_Acquire(void);
void Endstop_Release(void);

uint8_t       Endstop_AtTop(void);
uint8_t       Endstop_AtBottom(void);

Endstop_Pos_t Endstop_Read(void);
Endstop_Pos_t Endstop_Sample(void);
Endstop_Pos_t Endstop_Last(void);

void Endstop_Restore(Endstop_Pos_t pos);

#endif /* ENDSTOP_H */