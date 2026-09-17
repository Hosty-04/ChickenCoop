/**
  ******************************************************************************
  * @file    motor.c
  * @brief   Motor control and obstacle detection (INA226)
  ******************************************************************************
  */

#include "motor.h"
#include "endstop.h"
#include "ina226.h"
#include "power.h"
#include "timebase.h"
#include "tim.h"

#define MOTOR_PWM_TIMER          (&htim17)
#define MOTOR_PWM_CHANNEL        TIM_CHANNEL_1
#define MOTOR_PWM_MAX            49U
#define MOTOR_PWM_FREQ_HZ        20000UL

#define MOTOR_TRAVEL_TIMEOUT_MS  25000U
#define MOTOR_START_IGNORE_MS    250U
#define MOTOR_OVERCURRENT_MS     250U
#define MOTOR_REVERSE_PAUSE_MS   250U
#define MOTOR_SAMPLE_MS          40U
#define MOTOR_SENSE_MAX_FAILS    3U
#define MOTOR_TICK_STALL_MAX     20000UL

#define MOTOR_V_NOMINAL_V        6.0f
#define MOTOR_V_MARGIN_V         0.4f
#define MOTOR_V_MIN_VALID_V      0.5f
#define MOTOR_I_LIMIT_A          0.45f
#define MOTOR_BRIDGE_R_OHM       2.0f

typedef enum {
  STROKE_REACHED = 0,
  STROKE_TIMEOUT,
  STROKE_OVERCURRENT,
  STROKE_SENSOR
} Motor_Stroke_t;

static Endstop_Pos_t Motor_PosFor(Motor_Dir_t dir)
{
  return (dir == MOTOR_DIR_UP) ? ENDSTOP_POS_TOP : ENDSTOP_POS_BOTTOM;
}

static Motor_Dir_t Motor_Opposite(Motor_Dir_t dir)
{
  return (dir == MOTOR_DIR_UP) ? MOTOR_DIR_DOWN : MOTOR_DIR_UP;
}

static void Motor_SetPins(uint32_t mode)
{
  GPIO_InitTypeDef gpio = {0};

  __HAL_RCC_GPIOB_CLK_ENABLE();

  gpio.Pin   = PH_Pin | NSLEEP_Pin;
  gpio.Mode  = mode;
  gpio.Pull  = GPIO_NOPULL;
  gpio.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &gpio);
}

static void Motor_Duty(uint16_t duty)
{
  __HAL_TIM_SET_COMPARE(MOTOR_PWM_TIMER, MOTOR_PWM_CHANNEL, duty);
}

static void Motor_Begin(void)
{
  uint32_t div = (MOTOR_PWM_MAX + 1U) * MOTOR_PWM_FREQ_HZ;
  uint32_t psc = (HAL_RCC_GetPCLK2Freq() + (div / 2U)) / div;

  MX_TIM17_Init();

  __HAL_TIM_SET_PRESCALER(MOTOR_PWM_TIMER, (psc != 0U) ? (psc - 1U) : 0U);
  __HAL_TIM_SET_AUTORELOAD(MOTOR_PWM_TIMER, MOTOR_PWM_MAX);
  MOTOR_PWM_TIMER->Instance->EGR = TIM_EGR_UG;
  Motor_Duty(0U);

  HAL_GPIO_WritePin(PH_GPIO_Port, PH_Pin, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(NSLEEP_GPIO_Port, NSLEEP_Pin, GPIO_PIN_RESET);
  Motor_SetPins(GPIO_MODE_OUTPUT_PP);
  HAL_GPIO_WritePin(NSLEEP_GPIO_Port, NSLEEP_Pin, GPIO_PIN_SET);

  (void)HAL_TIM_PWM_Start(MOTOR_PWM_TIMER, MOTOR_PWM_CHANNEL);
}

static void Motor_End(void)
{
  Motor_Duty(0U);
  (void)HAL_TIM_PWM_Stop(MOTOR_PWM_TIMER, MOTOR_PWM_CHANNEL);

  HAL_GPIO_WritePin(NSLEEP_GPIO_Port, NSLEEP_Pin, GPIO_PIN_RESET);
  Motor_SetPins(GPIO_MODE_ANALOG);
}

static void Motor_Drive(Motor_Dir_t dir, uint16_t duty)
{
  HAL_GPIO_WritePin(PH_GPIO_Port, PH_Pin,
                    (dir == MOTOR_DIR_UP) ? GPIO_PIN_SET : GPIO_PIN_RESET);
  Motor_Duty(duty);
}

static void Motor_Regulate(float v_bat, float i_bat, uint16_t *duty, float *i_limit)
{
  float ratio;

  if (v_bat < MOTOR_V_MIN_VALID_V) {
    *duty    = (uint16_t)(MOTOR_PWM_MAX / 2U);
    *i_limit = MOTOR_I_LIMIT_A * 0.5f;
    return;
  }

  ratio = (MOTOR_V_NOMINAL_V + i_bat * MOTOR_BRIDGE_R_OHM + MOTOR_V_MARGIN_V) / v_bat;
  if (ratio > 1.0f)
    ratio = 1.0f;

  *duty    = (uint16_t)(ratio * (float)MOTOR_PWM_MAX + 0.5f);
  *i_limit = MOTOR_I_LIMIT_A * ratio;
}

static uint8_t Motor_AtTarget(Motor_Dir_t dir)
{
  return (dir == MOTOR_DIR_UP) ? Endstop_AtTop() : Endstop_AtBottom();
}

static Motor_Stroke_t Motor_Stroke(Motor_Dir_t dir)
{
  uint32_t t_start, t_sample, t_last, t_over = 0U, stall = 0U;
  uint8_t  over = 0U, fails = 0U;
  uint16_t duty;
  float    v, i, i_limit;

  HAL_Delay(MOTOR_SAMPLE_MS);

  if (INA226_Read(&v, &i) != HAL_OK)
    return STROKE_SENSOR;

  Motor_Regulate(v, i, &duty, &i_limit);
  Motor_Drive(dir, duty);

  t_start  = HAL_GetTick();
  t_sample = t_start;
  t_last   = t_start;

  while (1) {
    uint32_t now     = HAL_GetTick();
    uint32_t elapsed = TICKS_TO_MS(now - t_start);

    if (now != t_last) {
      t_last = now;
      stall  = 0U;
    } else if (++stall >= MOTOR_TICK_STALL_MAX) {
      return STROKE_TIMEOUT;
    }

    if (Motor_AtTarget(dir))
      return STROKE_REACHED;

    if (elapsed >= MOTOR_TRAVEL_TIMEOUT_MS)
      return STROKE_TIMEOUT;

    if (TICKS_TO_MS(now - t_sample) < MOTOR_SAMPLE_MS)
      continue;

    t_sample = now;

    if (INA226_Read(&v, &i) != HAL_OK) {
      if (++fails >= MOTOR_SENSE_MAX_FAILS)
        return STROKE_SENSOR;
      continue;
    }
    fails = 0U;

    Motor_Regulate(v, i, &duty, &i_limit);
    Motor_Drive(dir, duty);

    if ((elapsed < MOTOR_START_IGNORE_MS) || (i <= i_limit)) {
      over = 0U;
      continue;
    }

    if (!over) {
      over   = 1U;
      t_over = now;
    } else if (TICKS_TO_MS(now - t_over) >= MOTOR_OVERCURRENT_MS) {
      return STROKE_OVERCURRENT;
    }
  }
}

static Motor_Result_t Motor_Finish(Motor_Result_t result, Endstop_Pos_t expected)
{
  Motor_End();

  if ((Endstop_Sample() == ENDSTOP_POS_UNKNOWN) && (expected != ENDSTOP_POS_UNKNOWN))
    Endstop_Restore(expected);

  INA226_PowerDown();
  Power_SwitchToRunHSE48MHz();

  return result;
}

Motor_Result_t Motor_Move(Motor_Dir_t dir)
{
  Endstop_Pos_t  start  = Endstop_Sample();
  Endstop_Pos_t  target = Motor_PosFor(dir);
  Motor_Stroke_t stroke;

  if (start == target)
    return MOTOR_ALREADY;

  if (start == ENDSTOP_POS_UNKNOWN)
    return MOTOR_NO_REFERENCE;

  Power_SwitchToLPRunMSI1MHz();
  Endstop_Acquire();
  Motor_Begin();

  if ((INA226_PowerUp() != HAL_OK) || (INA226_ConfigFast() != HAL_OK))
    return Motor_Finish(MOTOR_SENSOR_ERROR, start);

  stroke = Motor_Stroke(dir);

  if (stroke == STROKE_REACHED) return Motor_Finish(MOTOR_OK, target);
  if (stroke == STROKE_TIMEOUT) return Motor_Finish(MOTOR_TIMEOUT, ENDSTOP_POS_UNKNOWN);
  if (stroke == STROKE_SENSOR)  return Motor_Finish(MOTOR_SENSOR_ERROR, ENDSTOP_POS_UNKNOWN);

  Motor_Duty(0U);
  HAL_Delay(MOTOR_REVERSE_PAUSE_MS);

  stroke = Motor_Stroke(Motor_Opposite(dir));

  if (stroke == STROKE_REACHED) return Motor_Finish(MOTOR_OBSTACLE, start);
  if (stroke == STROKE_SENSOR)  return Motor_Finish(MOTOR_SENSOR_ERROR, ENDSTOP_POS_UNKNOWN);

  return Motor_Finish(MOTOR_STUCK, ENDSTOP_POS_UNKNOWN);
}
