/**
  ******************************************************************************
  * @file    telemetry.h
  * @brief   LoRa uplink and downlink handler
  ******************************************************************************
  */

#ifndef TELEMETRY_H
#define TELEMETRY_H

#include "main.h"

#define TELEMETRY_NEST_COUNT     2U
#define TELEMETRY_EGGS_MAX       10U

#define TELEMETRY_LEN_STATUS     2U
#define TELEMETRY_LEN_FULL       5U
#define TELEMETRY_LEN_DOWNLINK   1U

void    Telemetry_RequestStatus(void);
uint8_t Telemetry_Pending(void);
void    Telemetry_Requeue(uint8_t length);
void    Telemetry_Clear(void);

uint8_t Telemetry_Build(uint8_t *buf, uint8_t buf_size);
void    Telemetry_HandleDownlink(const uint8_t *buf, uint8_t length);

#endif /* TELEMETRY_H */
