/**
  ******************************************************************************
  * @file    nest.h
  * @brief   Egg counting nest unit on the RS485 chain
  ******************************************************************************
  */

#ifndef NEST_H
#define NEST_H

#include "main.h"

#ifndef NEST_ADDRESS
#define NEST_ADDRESS  1U
#endif

#if (NEST_ADDRESS < 1U) || (NEST_ADDRESS > 15U)
#error "NEST_ADDRESS must be between 1 and 15"
#endif

void    Nest_Init(void);
void    Nest_Process(void);
uint8_t Nest_ReadRegisters(uint16_t start, uint16_t count, uint16_t *values);
uint8_t Nest_WriteRegisters(uint16_t start, uint16_t count, const uint16_t *values);

#endif /* NEST_H */
