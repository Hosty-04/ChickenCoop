/**
  ******************************************************************************
  * @file    telemetry.h
  * @brief   LoRa uplink and downlink handeler
  ******************************************************************************
  */

#ifndef TELEMETRY_H
#define TELEMETRY_H

#include "main.h"

#define TELEMETRY_NEST_COUNT     2U
#define TELEMETRY_EGGS_MAX       10U

#define TELEMETRY_LEN_STATUS     2U
#define TELEMETRY_LEN_FULL       5U

void Telemetry_RequestStatus(void);
void Telemetry_RequestFull(void);
uint8_t Telemetry_Pending(void);

void Telemetry_Requeue(uint8_t length);

uint8_t Telemetry_Build(uint8_t *buf);

void Telemetry_HandleDownlink(const uint8_t *buf, uint8_t length);

#endif /* TELEMETRY_H */