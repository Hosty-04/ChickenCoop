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

/**
  * @brief  Sestavi uplink do buf.
  * @param  buf_size  velikost ciloveho bufferu - nikdy ho neprekrocime
  * @retval pocet zapsanych bajtu, 0 kdyz neni co poslat
  */
uint8_t Telemetry_Build(uint8_t *buf, uint8_t buf_size);

void    Telemetry_HandleDownlink(const uint8_t *buf, uint8_t length);

/* Telemetry_RequestFull() zamerne odstranena - byla deklarovana
   bez definice. Vrat ji, az budou implementovana hnizda. */

#endif /* TELEMETRY_H */
