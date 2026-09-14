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

static uint8_t system_busy = 0;

void System_Process(void)
{
  if (system_busy)
    return;

  system_busy = 1;
  Battery_Process();
  Door_Process();
  system_busy = 0;
}

void System_Enable(void)
{
  if (System_IsEnabled())
    return;

  Battery_Enable();
  Door_Enable();
  Telemetry_RequestStatus();
}

void System_Disable(void)
{
  if (!System_IsEnabled())
    return;

  Door_Disable();
  Battery_Disable();
  Telemetry_RequestStatus();
}

uint8_t System_IsEnabled(void)
{
  return (uint8_t)(Door_IsEnabled() && Battery_IsEnabled());
}

uint8_t System_WorkPending(void)
{
  return (uint8_t)(Door_WorkPending() || Battery_WorkPending());
}