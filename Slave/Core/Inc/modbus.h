/**
  ******************************************************************************
  * @file    modbus.h
  * @brief   Modbus RTU slave over RS485
  ******************************************************************************
  */

#ifndef MODBUS_H
#define MODBUS_H

#include "main.h"

#define MODBUS_OK              0x00U
#define MODBUS_EXC_FUNCTION    0x01U
#define MODBUS_EXC_ADDRESS     0x02U
#define MODBUS_EXC_VALUE       0x03U

void    Modbus_Start(void);
uint8_t Modbus_Process(void);
void    Modbus_Restart(void);
void    Modbus_Stop(void);

#endif /* MODBUS_H */
