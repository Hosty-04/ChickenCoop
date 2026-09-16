/**
  ******************************************************************************
  * @file    system.c
  * @brief   Execution order and on/off switch
  ******************************************************************************
  */

#include "system.h"
#include "door.h"
#include "battery.h"
#include "telemetry.h"

void System_Process(void)
{
  Battery_Process();
  Door_Process();
}

uint8_t System_WorkPending(void)
{
  return (uint8_t)(Door_WorkPending() || Battery_WorkPending());
}

void System_Enable(void)
{
  if (System_IsEnabled())
    return;

  Door_Enable();
  Telemetry_RequestStatus();
}

void System_Disable(void)
{
  if (!System_IsEnabled())
    return;

  Door_Disable();
  Telemetry_RequestStatus();
}

uint8_t System_IsEnabled(void)
{
  return Door_IsEnabled();
}
