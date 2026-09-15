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
#define ENDSTOP_DEBOUNCE_MS 3U
#define ENDSTOP_SAMPLES     5U

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

static Endstop_Pos_t Endstop_ReadRaw(void)
{
  uint8_t top    = Endstop_AtTop();
  uint8_t bottom = Endstop_AtBottom();

  if (top && !bottom) return ENDSTOP_POS_TOP;
  if (bottom && !top) return ENDSTOP_POS_BOTTOM;

  return ENDSTOP_POS_UNKNOWN;
}

/* Rychle, nefiltrovane cteni - pouziva Motor_Stroke() pro detekci dojezdu,
   kde je prvni kontakt spravna odpoved. */
Endstop_Pos_t Endstop_Read(void)
{
  endstop_last = Endstop_ReadRaw();

  return endstop_last;
}

/*
 * Filtrovane cteni. Vola se po resetu (Door_LoadState) a pri Door_ClearFault(),
 * tedy prave tam, kde zakmit pakoveho mikrospinace (5-10 ms) vedl k falesnemu
 * UNKNOWN a nasledne k MOTOR_NO_REFERENCE a trvale porouse.
 */
Endstop_Pos_t Endstop_Sample(void)
{
  Endstop_Pos_t first, pos;
  uint8_t n;

  Endstop_Acquire();

  first = Endstop_ReadRaw();

  for (n = 1U; n < ENDSTOP_SAMPLES; n++) {
    HAL_Delay(ENDSTOP_DEBOUNCE_MS);
    pos = Endstop_ReadRaw();

    if (pos != first) {
      first = ENDSTOP_POS_UNKNOWN;    /* zakmit nebo skutecna mezipoloha */
      break;
    }
  }

  Endstop_Release();

  endstop_last = first;

  return endstop_last;
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
