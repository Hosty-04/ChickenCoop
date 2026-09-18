/**
  ******************************************************************************
  * @file    ina226.c
  * @brief   INA226 voltage/current sensor
  ******************************************************************************
  */

#include "ina226.h"
#include "i2c.h"
#include <math.h>

#define INA226_I2C_ADDR         (0x40U << 1)
#define INA226_I2C_TIMEOUT_MS   50U
#define INA226_I2C_TIMING_DEF   0x10805D88UL
#define INA226_INIT_ATTEMPTS    3U

#define INA226_REG_CONFIG       0x00U
#define INA226_REG_BUS_V        0x02U
#define INA226_REG_CURRENT      0x04U
#define INA226_REG_CALIB        0x05U

#define INA226_CFG_RSVD         0x4000U
#define INA226_MODE_MASK        0x0007U
#define INA226_CONFIG_BATTERY   (INA226_CFG_RSVD | 0x0726U)
#define INA226_CONFIG_FAST      (INA226_CFG_RSVD | 0x0527U)

#define INA226_CALIB_VALUE      8389U
#define INA226_CURRENT_LSB_A    (2.0f / 32768.0f)
#define INA226_BUS_V_LSB_V      0.00125f

static uint16_t ina226_cfg = INA226_CONFIG_BATTERY;

static void INA226_BusDelay(void)
{
  volatile uint32_t n = (SystemCoreClock / 200000U) + 4U;

  while (n--)
    __NOP();
}

static void INA226_BusPins(uint32_t mode)
{
  GPIO_InitTypeDef gpio = {0};

  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  gpio.Mode  = mode;
  gpio.Pull  = GPIO_NOPULL;
  gpio.Speed = GPIO_SPEED_FREQ_LOW;

  gpio.Pin = I2C_SDA_Pin;
  HAL_GPIO_Init(I2C_SDA_GPIO_Port, &gpio);
  gpio.Pin = I2C_SCL_Pin;
  HAL_GPIO_Init(I2C_SCL_GPIO_Port, &gpio);
}

static uint8_t INA226_BusStuck(void)
{
  INA226_BusPins(GPIO_MODE_INPUT);
  INA226_BusDelay();

  return (uint8_t)((HAL_GPIO_ReadPin(I2C_SDA_GPIO_Port, I2C_SDA_Pin) == GPIO_PIN_RESET) ||
                   (HAL_GPIO_ReadPin(I2C_SCL_GPIO_Port, I2C_SCL_Pin) == GPIO_PIN_RESET));
}

static void INA226_BusRecover(void)
{
  uint8_t n;

  HAL_GPIO_WritePin(I2C_SDA_GPIO_Port, I2C_SDA_Pin, GPIO_PIN_SET);
  HAL_GPIO_WritePin(I2C_SCL_GPIO_Port, I2C_SCL_Pin, GPIO_PIN_SET);
  INA226_BusPins(GPIO_MODE_OUTPUT_OD);
  INA226_BusDelay();

  for (n = 0U; n < 9U; n++) {
    HAL_GPIO_WritePin(I2C_SCL_GPIO_Port, I2C_SCL_Pin, GPIO_PIN_RESET);
    INA226_BusDelay();
    HAL_GPIO_WritePin(I2C_SCL_GPIO_Port, I2C_SCL_Pin, GPIO_PIN_SET);
    INA226_BusDelay();

    if (HAL_GPIO_ReadPin(I2C_SDA_GPIO_Port, I2C_SDA_Pin) == GPIO_PIN_SET)
      break;
  }

  HAL_GPIO_WritePin(I2C_SDA_GPIO_Port, I2C_SDA_Pin, GPIO_PIN_RESET);
  INA226_BusDelay();
  HAL_GPIO_WritePin(I2C_SCL_GPIO_Port, I2C_SCL_Pin, GPIO_PIN_SET);
  INA226_BusDelay();
  HAL_GPIO_WritePin(I2C_SDA_GPIO_Port, I2C_SDA_Pin, GPIO_PIN_SET);
  INA226_BusDelay();
}

static uint32_t INA226_ComputeTiming(uint32_t i2cclk_hz)
{
  const float t_low   = 5.5e-6f;
  const float t_high  = 4.5e-6f;
  const float t_setup = 450e-9f;
  uint32_t presc;

  for (presc = 0U; presc <= 15U; presc++) {
    float   t_presc = (float)(presc + 1U) / (float)i2cclk_hz;
    int32_t scll    = (int32_t)(t_low   / t_presc + 0.5f) - 1;
    int32_t sclh    = (int32_t)(t_high  / t_presc + 0.5f) - 1;
    int32_t scldel  = (int32_t)(t_setup / t_presc + 0.5f) - 1;

    if (scldel < 0)
      scldel = 0;

    if ((scll >= scldel) && (scll <= 255) && (sclh >= 0) && (sclh <= 255) && (scldel <= 15))
      return (presc << 28) | ((uint32_t)scldel << 20) | ((uint32_t)sclh << 8) | (uint32_t)scll;
  }

  return INA226_I2C_TIMING_DEF;
}

static HAL_StatusTypeDef INA226_Write(uint8_t reg, uint16_t val)
{
  uint8_t buf[3] = { reg, (uint8_t)(val >> 8), (uint8_t)val };

  return HAL_I2C_Master_Transmit(&hi2c2, INA226_I2C_ADDR, buf, 3U, INA226_I2C_TIMEOUT_MS);
}

static void INA226_ClearError(void)
{
  __HAL_I2C_DISABLE(&hi2c2);
  hi2c2.ErrorCode = HAL_I2C_ERROR_NONE;
  hi2c2.State     = HAL_I2C_STATE_READY;
  hi2c2.Mode      = HAL_I2C_MODE_NONE;
  __HAL_I2C_ENABLE(&hi2c2);
}

static HAL_StatusTypeDef INA226_ReadReg(uint8_t reg, uint16_t *val)
{
  uint8_t buf[2];
  HAL_StatusTypeDef st;

  st = HAL_I2C_Master_Transmit(&hi2c2, INA226_I2C_ADDR, &reg, 1U, INA226_I2C_TIMEOUT_MS);
  if (st == HAL_OK)
    st = HAL_I2C_Master_Receive(&hi2c2, INA226_I2C_ADDR, buf, 2U, INA226_I2C_TIMEOUT_MS);

  if (st != HAL_OK) {
    INA226_ClearError();
    return st;
  }

  *val = (uint16_t)(((uint16_t)buf[0] << 8) | buf[1]);

  return HAL_OK;
}

static HAL_StatusTypeDef INA226_Configure(uint16_t cfg)
{
  HAL_StatusTypeDef st = INA226_Write(INA226_REG_CALIB, INA226_CALIB_VALUE);

  if (st == HAL_OK)
    st = INA226_Write(INA226_REG_CONFIG, cfg);

  if (st == HAL_OK)
    ina226_cfg = cfg;

  return st;
}

HAL_StatusTypeDef INA226_PowerUp(void)
{
  if (hi2c2.State != HAL_I2C_STATE_RESET)
    (void)HAL_I2C_DeInit(&hi2c2);

  if (INA226_BusStuck())
    INA226_BusRecover();

  hi2c2.Instance              = I2C2;
  hi2c2.Init.Timing           = INA226_ComputeTiming(HAL_RCC_GetPCLK1Freq());
  hi2c2.Init.OwnAddress1      = 0U;
  hi2c2.Init.AddressingMode   = I2C_ADDRESSINGMODE_7BIT;
  hi2c2.Init.DualAddressMode  = I2C_DUALADDRESS_DISABLE;
  hi2c2.Init.OwnAddress2      = 0U;
  hi2c2.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
  hi2c2.Init.GeneralCallMode  = I2C_GENERALCALL_DISABLE;
  hi2c2.Init.NoStretchMode    = I2C_NOSTRETCH_DISABLE;

  if ((HAL_I2C_Init(&hi2c2) != HAL_OK) ||
      (HAL_I2CEx_ConfigAnalogFilter(&hi2c2, I2C_ANALOGFILTER_ENABLE) != HAL_OK) ||
      (HAL_I2CEx_ConfigDigitalFilter(&hi2c2, 0U) != HAL_OK))
    return HAL_ERROR;

  return HAL_OK;
}

void INA226_PowerDown(void)
{
  if (hi2c2.State == HAL_I2C_STATE_READY)
    (void)INA226_Write(INA226_REG_CONFIG, (uint16_t)(ina226_cfg & ~INA226_MODE_MASK));

  (void)HAL_I2C_DeInit(&hi2c2);
}

HAL_StatusTypeDef INA226_Init(void)
{
  HAL_StatusTypeDef st = HAL_ERROR;
  uint8_t attempt;

  for (attempt = 0U; (attempt < INA226_INIT_ATTEMPTS) && (st != HAL_OK); attempt++) {
    st = INA226_Configure(INA226_CONFIG_BATTERY);
    if (st != HAL_OK)
      HAL_Delay(10U);
  }

  return st;
}

HAL_StatusTypeDef INA226_ConfigBattery(void)
{
  return INA226_Configure(INA226_CONFIG_BATTERY);
}

HAL_StatusTypeDef INA226_ConfigFast(void)
{
  return INA226_Configure(INA226_CONFIG_FAST);
}

HAL_StatusTypeDef INA226_Read(float *voltage, float *current)
{
  uint16_t raw_bus, raw_current;
  HAL_StatusTypeDef st;

  st = INA226_ReadReg(INA226_REG_BUS_V, &raw_bus);
  if (st == HAL_OK)
    st = INA226_ReadReg(INA226_REG_CURRENT, &raw_current);
  if (st != HAL_OK)
    return st;

  if (voltage != NULL)
    *voltage = (float)raw_bus * INA226_BUS_V_LSB_V;

  if (current != NULL)
    *current = fabsf((float)(int16_t)raw_current * INA226_CURRENT_LSB_A);

  return HAL_OK;
}
