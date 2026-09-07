/**
  ******************************************************************************
  * @file    motor.c
  * @brief   DC motor driver (PWM + direction + limit switches)
  ******************************************************************************
  */

#include "motor.h"
#include "tim.h"
#include "gpio.h"

#define MOTOR_PWM_TIMER    &htim17
#define MOTOR_PWM_CHANNEL  TIM_CHANNEL_1
#define MOTOR_PWM_MAX      49U

#define MOTOR_PH_PORT      GPIOB
#define MOTOR_PH_PIN       GPIO_PIN_13
#define MOTOR_NSLEEP_PORT  GPIOB
#define MOTOR_NSLEEP_PIN   GPIO_PIN_14
#define MOTOR_EN_PORT      GPIOB
#define MOTOR_EN_PIN       GPIO_PIN_9

/* End-stop switches */
#define SWITCH_UP_PORT     GPIOA
#define SWITCH_UP_PIN      GPIO_PIN_9    /* upper = open position */
#define SWITCH_DN_PORT     GPIOB
#define SWITCH_DN_PIN      GPIO_PIN_10   /* lower = closed position */

#define MOTOR_TIMEOUT_MS   25000U

static uint8_t motor_active = 0;

static void Switch_Enable(void)
{
  GPIO_InitTypeDef gpio = {0};

  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  gpio.Mode  = GPIO_MODE_INPUT;
  gpio.Pull  = GPIO_PULLDOWN;
  gpio.Speed = GPIO_SPEED_FREQ_LOW;

  gpio.Pin = SWITCH_UP_PIN;
  HAL_GPIO_Init(SWITCH_UP_PORT, &gpio);

  gpio.Pin = SWITCH_DN_PIN;
  HAL_GPIO_Init(SWITCH_DN_PORT, &gpio);
}

static void Switch_Disable(void)
{
  GPIO_InitTypeDef gpio = {0};

  gpio.Mode = GPIO_MODE_ANALOG;
  gpio.Pull = GPIO_NOPULL;

  gpio.Pin = SWITCH_UP_PIN;
  HAL_GPIO_Init(SWITCH_UP_PORT, &gpio);

  gpio.Pin = SWITCH_DN_PIN;
  HAL_GPIO_Init(SWITCH_DN_PORT, &gpio);
}

static uint8_t Switch_UpPressed(void)
{
  return (HAL_GPIO_ReadPin(SWITCH_UP_PORT, SWITCH_UP_PIN) == GPIO_PIN_SET) ? 1U : 0U;
}

static uint8_t Switch_DnPressed(void)
{
  return (HAL_GPIO_ReadPin(SWITCH_DN_PORT, SWITCH_DN_PIN) == GPIO_PIN_SET) ? 1U : 0U;
}

void Motor_Begin(void)
{
  if (motor_active) return;

  GPIO_InitTypeDef gpio = {0};
  __HAL_RCC_GPIOB_CLK_ENABLE();

  gpio.Pin   = MOTOR_PH_PIN | MOTOR_NSLEEP_PIN;
  gpio.Mode  = GPIO_MODE_OUTPUT_PP;
  gpio.Pull  = GPIO_NOPULL;
  gpio.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &gpio);

  HAL_GPIO_WritePin(MOTOR_PH_PORT, MOTOR_PH_PIN, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(MOTOR_NSLEEP_PORT, MOTOR_NSLEEP_PIN, GPIO_PIN_SET);

  HAL_TIM_PWM_DeInit(MOTOR_PWM_TIMER);
  MX_TIM17_Init();
  HAL_TIM_PWM_Start(MOTOR_PWM_TIMER, MOTOR_PWM_CHANNEL);
  __HAL_TIM_SET_COMPARE(MOTOR_PWM_TIMER, MOTOR_PWM_CHANNEL, 0);
  motor_active = 1;
}

void Motor_End(void)
{
  if (!motor_active) return;

  __HAL_TIM_SET_COMPARE(MOTOR_PWM_TIMER, MOTOR_PWM_CHANNEL, 0);
  HAL_TIM_PWM_Stop(MOTOR_PWM_TIMER, MOTOR_PWM_CHANNEL);
  HAL_GPIO_WritePin(MOTOR_NSLEEP_PORT, MOTOR_NSLEEP_PIN, GPIO_PIN_RESET);

  GPIO_InitTypeDef gpio = {0};
  gpio.Pin  = MOTOR_EN_PIN | MOTOR_PH_PIN | MOTOR_NSLEEP_PIN;
  gpio.Mode = GPIO_MODE_ANALOG;
  gpio.Pull  = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOB, &gpio);

  motor_active = 0;
}

void Motor_Set(uint8_t up, uint16_t duty)
{
  if (!motor_active) Motor_Begin();
  if (duty > MOTOR_PWM_MAX) duty = MOTOR_PWM_MAX;

  HAL_GPIO_WritePin(MOTOR_PH_PORT, MOTOR_PH_PIN,
                    up ? GPIO_PIN_SET : GPIO_PIN_RESET);
  __HAL_TIM_SET_COMPARE(MOTOR_PWM_TIMER, MOTOR_PWM_CHANNEL, duty);
}

void Motor_Stop(void)
{
  if (motor_active)
    __HAL_TIM_SET_COMPARE(MOTOR_PWM_TIMER, MOTOR_PWM_CHANNEL, 0);
}

static void Motor_RunUntilSwitch(uint8_t up)
{
  uint32_t t_start;

  Switch_Enable();
  Motor_Begin();
  Motor_Set(up, MOTOR_PWM_MAX);

  t_start = HAL_GetTick();
  while (1)
  {
    if (up)
    {
      if (Switch_UpPressed()) break;
    }
    else
    {
      if (Switch_DnPressed()) break;
    }

    if ((HAL_GetTick() - t_start) >= MOTOR_TIMEOUT_MS)
      break;
  }

  Motor_Stop();
  Motor_End();
  Switch_Disable();
}

void Motor_Open(void)
{
  Motor_RunUntilSwitch(1);
}

void Motor_Close(void)
{
  Motor_RunUntilSwitch(0);
}