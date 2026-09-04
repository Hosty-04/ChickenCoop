/**
  ******************************************************************************
  * @file    motor.c
  * @brief   DC motor driver (PWM + direction control)
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
#define MOTOR_RUN_SECS     5U

static uint8_t motor_active = 0;

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
  gpio.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOB, &gpio);

  motor_active = 0;
  __HAL_RCC_GPIOB_CLK_DISABLE();
}

void Motor_Set(uint8_t forward, uint16_t duty)
{
  if (!motor_active) Motor_Begin();
  if (duty > MOTOR_PWM_MAX) duty = MOTOR_PWM_MAX;

  HAL_GPIO_WritePin(MOTOR_PH_PORT, MOTOR_PH_PIN,
                    forward ? GPIO_PIN_SET : GPIO_PIN_RESET);
  __HAL_TIM_SET_COMPARE(MOTOR_PWM_TIMER, MOTOR_PWM_CHANNEL, duty);
}

void Motor_Stop(void)
{
  if (motor_active)
    __HAL_TIM_SET_COMPARE(MOTOR_PWM_TIMER, MOTOR_PWM_CHANNEL, 0);
}

void Motor_Open(void)
{
  Motor_Begin();
  Motor_Set(1, MOTOR_PWM_MAX);
  HAL_Delay(MOTOR_RUN_SECS * 1000);
  Motor_Stop();
  Motor_End();
}

void Motor_Close(void)
{
  Motor_Begin();
  Motor_Set(0, MOTOR_PWM_MAX);
  HAL_Delay(MOTOR_RUN_SECS * 1000);
  Motor_Stop();
  Motor_End();
}