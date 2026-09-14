/**
  ******************************************************************************
  * @file    endstop.c
  * @brief   Limit switches
  ******************************************************************************
  */

#include "endstop.h"
#include "gpio.h"

#define ENDSTOP_TOP_PORT    LIM_UP_GPIO_Port
#define ENDSTOP_TOP_PIN     LIM_UP_Pin
#define ENDSTOP_BOTTOM_PORT LIM_DN_GPIO_Port
#define ENDSTOP_BOTTOM_PIN  LIM_DN_Pin

#define ENDSTOP_SETTLE_MS   2U

static Endstop_Pos_t endstop_last = ENDSTOP_POS_UNKNOWN;

void Endstop_Acquire(void)
{
  GPIO_InitTypeDef gpio = {0};

  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  gpio.Mode  = GPIO_MODE_INPUT;
  gpio.Pull  = GPIO_PULLDOWN;
  gpio.Speed = GPIO_SPEED_FREQ_LOW;

  gpio.Pin = ENDSTOP_TOP_PIN;
  HAL_GPIO_Init(ENDSTOP_TOP_PORT, &gpio);
  gpio.Pin = ENDSTOP_BOTTOM_PIN;
  HAL_GPIO_Init(ENDSTOP_BOTTOM_PORT, &gpio);

  HAL_Delay(ENDSTOP_SETTLE_MS);
}

void Endstop_Release(void)
{
  GPIO_InitTypeDef gpio = {0};

  gpio.Mode = GPIO_MODE_ANALOG;
  gpio.Pull = GPIO_NOPULL;

  gpio.Pin = ENDSTOP_TOP_PIN;
  HAL_GPIO_Init(ENDSTOP_TOP_PORT, &gpio);
  gpio.Pin = ENDSTOP_BOTTOM_PIN;
  HAL_GPIO_Init(ENDSTOP_BOTTOM_PORT, &gpio);
}

uint8_t Endstop_AtTop(void)
{
  return (HAL_GPIO_ReadPin(ENDSTOP_TOP_PORT, ENDSTOP_TOP_PIN) == GPIO_PIN_SET);
}

uint8_t Endstop_AtBottom(void)
{
  return (HAL_GPIO_ReadPin(ENDSTOP_BOTTOM_PORT, ENDSTOP_BOTTOM_PIN) == GPIO_PIN_SET);
}

Endstop_Pos_t Endstop_Read(void)
{
  uint8_t top    = Endstop_AtTop();
  uint8_t bottom = Endstop_AtBottom();

  if (top && !bottom)      endstop_last = ENDSTOP_POS_TOP;
  else if (bottom && !top) endstop_last = ENDSTOP_POS_BOTTOM;
  else                     endstop_last = ENDSTOP_POS_UNKNOWN;

  return endstop_last;
}

Endstop_Pos_t Endstop_Sample(void)
{
  Endstop_Pos_t pos;

  Endstop_Acquire();
  pos = Endstop_Read();
  Endstop_Release();

  return pos;
}

Endstop_Pos_t Endstop_Last(void)
{
  return endstop_last;
}

void Endstop_Restore(Endstop_Pos_t pos)
{
  if (pos == ENDSTOP_POS_TOP || pos == ENDSTOP_POS_BOTTOM)
    endstop_last = pos;
  else
    endstop_last = ENDSTOP_POS_UNKNOWN;
}