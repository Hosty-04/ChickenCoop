/**
  ******************************************************************************
  * @file    hx711.h
  * @brief   HX711 load cell converter
  ******************************************************************************
  */

#ifndef HX711_H
#define HX711_H

#include "main.h"

#define HX711_MIN  (-8388608L)
#define HX711_MAX  8388607L

uint8_t HX711_Ready(void);
int32_t HX711_Read(void);
void    HX711_Off(void);

#endif /* HX711_H */
