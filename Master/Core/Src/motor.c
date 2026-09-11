/**
  ******************************************************************************
  * @file    motor.c
  * @brief   Motor driver + obstacle detection (INA226)
  ******************************************************************************
  */

#include "motor.h"
#include "ina226.h"
#include "power.h"
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

#define SWITCH_UP_PORT     GPIOA
#define SWITCH_UP_PIN      GPIO_PIN_9
#define SWITCH_DN_PORT     GPIOB
#define SWITCH_DN_PIN      GPIO_PIN_10

#define MOTOR_TIMEOUT_MS       25000U
#define MOTOR_START_IGNORE_MS  250U
#define MOTOR_OBSTACLE_MS      250U
#define MOTOR_PAUSE_MS         250U

#define MOTOR_V_NOMINAL  6.0f
#define MOTOR_V_MARGIN   0.4f
#define MOTOR_I_MAX      0.45f

#define BRIDGE_R_EF      2.0f

static uint8_t motor_active = 0;
static uint8_t motor_fault  = 0;

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

static uint8_t Switch_UpReleased(void)
{
  return HAL_GPIO_ReadPin(SWITCH_UP_PORT, SWITCH_UP_PIN) == GPIO_PIN_SET;
}

static uint8_t Switch_DnPressed(void)
{
  return HAL_GPIO_ReadPin(SWITCH_DN_PORT, SWITCH_DN_PIN) == GPIO_PIN_SET;
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
  HAL_Delay(1);

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
  gpio.Pin  = MOTOR_EN_PIN | MOTOR_PH_PIN;
  gpio.Mode = GPIO_MODE_ANALOG;
  gpio.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOB, &gpio);

  gpio.Pin   = MOTOR_NSLEEP_PIN;
  gpio.Mode  = GPIO_MODE_OUTPUT_PP;
  gpio.Pull  = GPIO_NOPULL;
  gpio.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &gpio);
  HAL_GPIO_WritePin(MOTOR_NSLEEP_PORT, MOTOR_NSLEEP_PIN, GPIO_PIN_RESET);

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

void Motor_ClearFault(void) { motor_fault = 0; }
void Motor_SetFault(void)   { motor_fault = 1; }
uint8_t Motor_IsFaulted(void) { return motor_fault; }

static uint16_t Motor_CalcDuty(float v_aku, float i_aku)
{
  float v_cmd = MOTOR_V_NOMINAL + i_aku * BRIDGE_R_EF + MOTOR_V_MARGIN;
  float duty_f;
  if (v_aku < 0.5f) return MOTOR_PWM_MAX;
  duty_f = (v_cmd / v_aku) * (float)MOTOR_PWM_MAX;
  if (duty_f > (float)MOTOR_PWM_MAX) duty_f = (float)MOTOR_PWM_MAX;
  if (duty_f < 0.0f) duty_f = 0.0f;
  return (uint16_t)(duty_f + 0.5f);
}

static float Motor_CalcIth(float v_aku, float i_aku)
{
  float v_cmd = MOTOR_V_NOMINAL + i_aku * BRIDGE_R_EF + MOTOR_V_MARGIN;
  float ith;
  if (v_aku < 0.5f) return MOTOR_I_MAX;
  ith = MOTOR_I_MAX * (v_cmd / v_aku);
  if (ith > MOTOR_I_MAX) ith = MOTOR_I_MAX;
  return ith;
}

static Motor_Result_t Motor_Run(uint8_t up)
{
  uint32_t t0, t_over = 0;
  float v = 6.5f, i = 0.0f;
  uint16_t duty;

  if (motor_fault)
    return MOTOR_FAULT;

  Switch_Enable();

  if (up) {
    if (!Switch_DnPressed()) {
      Switch_Disable();
      Motor_SetFault();
      return MOTOR_FAULT;
    }
  } else {
    if (!Switch_UpReleased()) {
      Switch_Disable();
      Motor_SetFault();
      return MOTOR_FAULT;
    }
  }

  Power_SwitchToLPRunMSI1MHz();
  Motor_Begin();
  INA226_PowerUp();
  INA226_ConfigFast();

  if (INA226_Read(&v, &i) != HAL_OK) { v = 6.5f; i = 0.0f; }
  duty = Motor_CalcDuty(v, i);
  Motor_Set(up, duty);
  t0 = HAL_GetTick();

  while (1) {
    uint32_t elapsed = HAL_GetTick() - t0;

    if (up) {
      if (Switch_UpReleased()) {
        Motor_Stop(); Motor_End(); Switch_Disable();
        INA226_PowerDown();
        Power_SwitchToRunHSE48MHz();
        return MOTOR_OK;
      }
    } else {
      if (Switch_DnPressed()) {
        Motor_Stop(); Motor_End(); Switch_Disable();
        INA226_PowerDown();
        Power_SwitchToRunHSE48MHz();
        return MOTOR_OK;
      }
    }

    if (elapsed >= MOTOR_TIMEOUT_MS) {
      Motor_Stop(); Motor_End(); Switch_Disable();
      INA226_PowerDown();
      Power_SwitchToRunHSE48MHz();
      return MOTOR_TIMEOUT;
    }

    if (INA226_Read(&v, &i) == HAL_OK) {
      duty = Motor_CalcDuty(v, i);
      Motor_Set(up, duty);
    }

    if (elapsed < MOTOR_START_IGNORE_MS)
      continue;

    {
      float ith = Motor_CalcIth(v, i);
      if (i > ith) {
        if (t_over == 0)
          t_over = HAL_GetTick();
        else if ((HAL_GetTick() - t_over) >= MOTOR_OBSTACLE_MS) {
          Motor_Stop();
          HAL_Delay(MOTOR_PAUSE_MS);
          uint8_t back = up ? 0U : 1U;
          uint32_t tr = HAL_GetTick();
          if (INA226_Read(&v, &i) != HAL_OK) { v = 6.5f; i = 0.0f; }
          duty = Motor_CalcDuty(v, i);
          Motor_Set(back, duty);
          uint8_t ok = 0;
          while ((HAL_GetTick() - tr) < MOTOR_TIMEOUT_MS) {
            if (back) { if (Switch_UpReleased()) { ok = 1; break; } }
            else      { if (Switch_DnPressed()) { ok = 1; break; } }
            if (INA226_Read(&v, &i) == HAL_OK) {
              duty = Motor_CalcDuty(v, i);
              Motor_Set(back, duty);
            }
          }
          Motor_Stop();

          if (!ok) {
            Motor_End();
            Switch_Disable();
            INA226_PowerDown();
            Power_SwitchToRunHSE48MHz();
            Motor_SetFault();
            return MOTOR_FAULT;
          }

          Motor_End();
          Switch_Disable();
          INA226_PowerDown();
          Power_SwitchToRunHSE48MHz();
          return MOTOR_OBSTACLE;
        }
      } else {
        t_over = 0;
      }
    }
  }
}

Motor_Result_t Motor_Open(void)  { return Motor_Run(1); }
Motor_Result_t Motor_Close(void) { return Motor_Run(0); }