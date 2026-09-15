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
#include "timebase.h"          /* TICKS_TO_MS */
#include "tim.h"
#include "gpio.h"

#define MOTOR_PH_PORT       PH_GPIO_Port
#define MOTOR_PH_PIN        PH_Pin
#define MOTOR_NSLEEP_PORT   NSLEEP_GPIO_Port
#define MOTOR_NSLEEP_PIN    NSLEEP_Pin

#define MOTOR_PWM_TIMER     (&htim17)
#define MOTOR_PWM_CHANNEL   TIM_CHANNEL_1

#define MOTOR_PWM_MAX       49U            /* ARR -> 50 kroku, rozliseni 2 % */
#define MOTOR_PWM_FREQ_HZ   20000UL

#define MOTOR_TRAVEL_TIMEOUT_MS  25000U
#define MOTOR_START_IGNORE_MS      250U
#define MOTOR_OVERCURRENT_MS       250U
#define MOTOR_REVERSE_PAUSE_MS     250U

/* CONFIG_FAST: AVG=16, VBUSCT+VSHCT = 1.1 + 1.1 ms -> novy vzorek po 35.2 ms */
#define MOTOR_SAMPLE_MS      36U

#define MOTOR_V_NOMINAL_V     6.0f
#define MOTOR_V_MARGIN_V      0.4f
#define MOTOR_I_LIMIT_A       0.45f
#define MOTOR_BRIDGE_R_OHM    2.0f

#define MOTOR_SENSE_MAX_FAILS 3U

typedef enum {
  STROKE_REACHED = 0,
  STROKE_TIMEOUT,
  STROKE_OVERCURRENT,
  STROKE_SENSOR
} Motor_Stroke_t;

static void Motor_SetPins(uint32_t mode)
{
  GPIO_InitTypeDef gpio = {0};

  __HAL_RCC_GPIOB_CLK_ENABLE();

  gpio.Pin   = MOTOR_PH_PIN | MOTOR_NSLEEP_PIN;
  gpio.Mode  = mode;
  gpio.Pull  = GPIO_NOPULL;
  gpio.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &gpio);
}

static void Motor_Duty(uint16_t duty)
{
  __HAL_TIM_SET_COMPARE(MOTOR_PWM_TIMER, MOTOR_PWM_CHANNEL, duty);
}

/*
 * Prescaler se dopocitava z aktualni frekvence APB2, aby PWM bylo 20 kHz
 * bez ohledu na to, v jakem hodinovem rezimu se motor spusti.
 * Pri PSC = 0 a 48 MHz by vyslo 960 kHz, tedy nad limitem DRV8838 (250 kHz).
 */
static void Motor_SetupPwm(void)
{
  uint32_t timclk = HAL_RCC_GetPCLK2Freq();        /* TIM17 sedi na APB2 */
  uint32_t div    = (MOTOR_PWM_MAX + 1U) * MOTOR_PWM_FREQ_HZ;
  uint32_t psc    = (timclk + (div / 2U)) / div;

  if (psc == 0U)
    psc = 1U;

  __HAL_TIM_SET_PRESCALER(MOTOR_PWM_TIMER, psc - 1U);
  __HAL_TIM_SET_AUTORELOAD(MOTOR_PWM_TIMER, MOTOR_PWM_MAX);

  /*
   * ARR ma zapnuty preload (AutoReloadPreload = ENABLE), takze se zapis
   * projevi az pri update eventu; PSC se nacita rovnez az pri update.
   * UG event oboji okamzite prenese do aktivnich registru.
   * MOTOR_PWM_TIMER je handle, registry jsou az v Instance.
   */
  MOTOR_PWM_TIMER->Instance->EGR = TIM_EGR_UG;
}

static void Motor_Begin(void)
{
  Motor_SetPins(GPIO_MODE_OUTPUT_PP);

  HAL_GPIO_WritePin(MOTOR_PH_PORT, MOTOR_PH_PIN, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(MOTOR_NSLEEP_PORT, MOTOR_NSLEEP_PIN, GPIO_PIN_SET);

  Motor_SetupPwm();
  Motor_Duty(0U);
  HAL_TIM_PWM_Start(MOTOR_PWM_TIMER, MOTOR_PWM_CHANNEL);
}

static void Motor_End(void)
{
  Motor_Duty(0U);
  HAL_TIM_PWM_Stop(MOTOR_PWM_TIMER, MOTOR_PWM_CHANNEL);

  HAL_GPIO_WritePin(MOTOR_NSLEEP_PORT, MOTOR_NSLEEP_PIN, GPIO_PIN_RESET);
  Motor_SetPins(GPIO_MODE_ANALOG);
}

static void Motor_Drive(Motor_Dir_t dir, uint16_t duty)
{
  HAL_GPIO_WritePin(MOTOR_PH_PORT, MOTOR_PH_PIN,
                    (dir == MOTOR_DIR_UP) ? GPIO_PIN_SET : GPIO_PIN_RESET);
  Motor_Duty(duty);
}

static void Motor_Regulate(float v_bat, float i_bat,
                           uint16_t *duty, float *i_limit)
{
  float v_cmd, ratio;

  if (v_bat < 0.5f) {
    /* nedoslo platne mereni - jedeme opatrne, ne plnou stridou */
    *duty    = (uint16_t)(MOTOR_PWM_MAX / 2U);
    *i_limit = MOTOR_I_LIMIT_A * 0.5f;
    return;
  }

  v_cmd = MOTOR_V_NOMINAL_V + i_bat * MOTOR_BRIDGE_R_OHM + MOTOR_V_MARGIN_V;
  ratio = v_cmd / v_bat;

  if (ratio > 1.0f)
    ratio = 1.0f;
  else if (ratio < 0.0f)
    ratio = 0.0f;

  *duty    = (uint16_t)(ratio * (float)MOTOR_PWM_MAX + 0.5f);
  *i_limit = MOTOR_I_LIMIT_A * ratio;
}

static uint8_t Motor_AtTarget(Motor_Dir_t dir)
{
  return (dir == MOTOR_DIR_UP) ? Endstop_AtTop() : Endstop_AtBottom();
}

static Motor_Stroke_t Motor_Stroke(Motor_Dir_t dir)
{
  uint32_t t_start, t_over = 0, t_sample;
  uint8_t  over = 0, fails = 0;
  uint16_t duty;
  float    v, i, i_limit;

  /* po probuzeni ze shutdownu musi dobehnout prvni konverze */
  HAL_Delay(MOTOR_SAMPLE_MS);

  if (INA226_Read(&v, &i) != HAL_OK)
    return STROKE_SENSOR;

  Motor_Regulate(v, i, &duty, &i_limit);
  Motor_Drive(dir, duty);

  t_start  = HAL_GetTick();
  t_sample = t_start;

  while (1) {
    uint32_t now     = HAL_GetTick();
    uint32_t elapsed = TICKS_TO_MS(now - t_start);

    if (Motor_AtTarget(dir))
      return STROKE_REACHED;

    if (elapsed >= MOTOR_TRAVEL_TIMEOUT_MS)
      return STROKE_TIMEOUT;

    /* koncaky testujeme v kazde otacce, INA226 jen jednou za konverzi */
    if (TICKS_TO_MS(now - t_sample) < MOTOR_SAMPLE_MS)
      continue;

    t_sample = now;

    if (INA226_Read(&v, &i) != HAL_OK) {
      if (++fails >= MOTOR_SENSE_MAX_FAILS)
        return STROKE_SENSOR;
      continue;
    }
    fails = 0;

    Motor_Regulate(v, i, &duty, &i_limit);
    Motor_Drive(dir, duty);

    if (elapsed < MOTOR_START_IGNORE_MS)
      continue;

    if (i <= i_limit) {
      over = 0;
      continue;
    }

    if (!over) {
      over   = 1;
      t_over = now;
    } else if (TICKS_TO_MS(now - t_over) >= MOTOR_OVERCURRENT_MS) {
      return STROKE_OVERCURRENT;
    }
  }
}

static Endstop_Pos_t Motor_PosFor(Motor_Dir_t dir)
{
  return (dir == MOTOR_DIR_UP) ? ENDSTOP_POS_TOP : ENDSTOP_POS_BOTTOM;
}

static Motor_Dir_t Motor_Opposite(Motor_Dir_t dir)
{
  return (dir == MOTOR_DIR_UP) ? MOTOR_DIR_DOWN : MOTOR_DIR_UP;
}

static Motor_Result_t Motor_Finish(Motor_Result_t result)
{
  Motor_End();
  (void)Endstop_Read();
  Endstop_Release();
  INA226_PowerDown();
  Power_SwitchToRunHSE48MHz();

  return result;
}

Motor_Result_t Motor_Move(Motor_Dir_t dir)
{
  Endstop_Pos_t  start;
  Motor_Stroke_t stroke;

  Endstop_Acquire();
  start = Endstop_Read();

  if (start == Motor_PosFor(dir)) {
    Endstop_Release();
    return MOTOR_ALREADY;
  }

  if (start != Motor_PosFor(Motor_Opposite(dir))) {
    Endstop_Release();
    return MOTOR_NO_REFERENCE;
  }

  Power_SwitchToLPRunMSI1MHz();
  Motor_Begin();

  if (INA226_PowerUp() != HAL_OK)
    return Motor_Finish(MOTOR_SENSOR_ERROR);

  INA226_ConfigFast();

  stroke = Motor_Stroke(dir);

  if (stroke == STROKE_REACHED) return Motor_Finish(MOTOR_OK);
  if (stroke == STROKE_TIMEOUT) return Motor_Finish(MOTOR_TIMEOUT);
  if (stroke == STROKE_SENSOR)  return Motor_Finish(MOTOR_SENSOR_ERROR);

  Motor_Duty(0U);
  HAL_Delay(MOTOR_REVERSE_PAUSE_MS);

  stroke = Motor_Stroke(Motor_Opposite(dir));

  if (stroke == STROKE_REACHED) return Motor_Finish(MOTOR_OBSTACLE);
  if (stroke == STROKE_SENSOR)  return Motor_Finish(MOTOR_SENSOR_ERROR);

  return Motor_Finish(MOTOR_STUCK);
}