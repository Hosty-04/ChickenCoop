/**
  ******************************************************************************
  * @file    hx711.c
  * @brief   HX711 load cell converter
  ******************************************************************************
  */

#include "hx711.h"

#define HX711_BITS    24U
#define HX711_SIGN    0x00800000UL
#define HX711_EXTEND  0xFF000000UL

uint8_t HX711_Ready(void)
{
  return (uint8_t)((DT_GPIO_Port->IDR & DT_Pin) == 0U);
}

int32_t HX711_Read(void)
{
  uint32_t primask = __get_PRIMASK();
  uint32_t value   = 0U;
  uint32_t bit;

  __disable_irq();

  for (bit = 0U; bit < HX711_BITS; bit++) {
    SCK_GPIO_Port->BSRR = SCK_Pin;
    SCK_GPIO_Port->BRR  = SCK_Pin;
    value = (value << 1) | (((DT_GPIO_Port->IDR & DT_Pin) != 0U) ? 1U : 0U);
  }

  SCK_GPIO_Port->BSRR = SCK_Pin;
  SCK_GPIO_Port->BRR  = SCK_Pin;

  __set_PRIMASK(primask);

  if (value & HX711_SIGN)
    value |= HX711_EXTEND;

  return (int32_t)value;
}

void HX711_Off(void)
{
  HAL_GPIO_DeInit(SCK_GPIO_Port, SCK_Pin);
  HAL_GPIO_DeInit(DT_GPIO_Port, DT_Pin);
}
