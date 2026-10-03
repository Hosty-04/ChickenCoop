/**
  ******************************************************************************
  * @file    system.c
  * @brief   Execution order and on/off switch
  ******************************************************************************
  */

#include "system.h"
#include "door.h"
#include "battery.h"
#include "nests.h"
#include "telemetry.h"

void System_Process(void)
{
  Nests_Process();
  Battery_Process();
  Door_Process();
}

uint8_t System_WorkPending(void)
{
  return (uint8_t)(Door_WorkPending() || Battery_WorkPending() || Nests_WorkPending());
}

void System_Enable(void)
{
  if (System_IsEnabled())
    return;

  Door_Enable();
  Nests_Enable();
  Telemetry_RequestStatus();
}

void System_Disable(void)
{
  if (!System_IsEnabled())
    return;

  Door_Disable();
  Nests_Disable();
  Telemetry_RequestStatus();
}

uint8_t System_IsEnabled(void)
{
  return Door_IsEnabled();
}
