/**
  ******************************************************************************
  * @file    ina226.c
  * @brief   INA226 voltage/current sensor
  ******************************************************************************
  */

#include "ina226.h"
#include "i2c.h"
#include <math.h>

extern I2C_HandleTypeDef hi2c2;

#define REG_CONFIG   0x00
#define REG_SHUNT_V  0x01
#define REG_BUS_V    0x02
#define REG_CURRENT  0x04
#define REG_CALIB    0x05

#define INA226_CONFIG_BATTERY  0x0726U   /* AVG=64, VBUSCT=1.1ms */
#define INA226_CONFIG_FAST     0x0527U   /* AVG=16, VBUSCT+VSHCT=1.1+1.1ms */

/*
 * Rshunt = 0.010 Ohm (R010)
 * Max current ≈ 2 A → Current_LSB = 2/32768 ≈ 61.0 uA
 * CAL = 0.00512 / (Current_LSB * Rshunt) ≈ 8389
 * Resolution << 50 mA → OK
 */
#define INA226_CURRENT_LSB   (2.0f / 32768.0f)
#define INA226_CALIB_VALUE   8389U

/* Bus voltage LSB = 1.25 mV */
#define INA226_BUS_V_LSB     0.00125f

static HAL_StatusTypeDef Write(uint8_t reg, uint16_t val)
{
  uint8_t buf[3] = { reg, (uint8_t)(val >> 8), (uint8_t)(val & 0xFF) };
  return HAL_I2C_Master_Transmit(&hi2c2, INA226_I2C_ADDR, buf, 3, 50);
}

static HAL_StatusTypeDef Read(uint8_t reg, uint16_t *val)
{
  uint8_t buf[2];
  HAL_StatusTypeDef st;

  st = HAL_I2C_Master_Transmit(&hi2c2, INA226_I2C_ADDR, &reg, 1, 50);
  if (st != HAL_OK) return st;

  st = HAL_I2C_Master_Receive(&hi2c2, INA226_I2C_ADDR, buf, 2, 50);
  if (st != HAL_OK) return st;

  *val = ((uint16_t)buf[0] << 8) | buf[1];
  return HAL_OK;
}

HAL_StatusTypeDef INA226_Read(float *voltage, float *current)
{
  uint16_t raw_bus, raw_current;
  HAL_StatusTypeDef st;

  st = Read(REG_BUS_V, &raw_bus);
  if (st != HAL_OK) return st;

  st = Read(REG_CURRENT, &raw_current);
  if (st != HAL_OK) return st;

  if (voltage)
    *voltage = (float)raw_bus * INA226_BUS_V_LSB;

  if (current)
    *current = fabsf((float)((int16_t)raw_current) * INA226_CURRENT_LSB);

  return HAL_OK;
}

/*
 * Simplified I2C_TIMINGR recalculation for the current PCLK1 frequency.
 * The original .ioc value (0x10805D88) is only for 48 MHz and causes wrong timings elsewhere.
 * Target: Standard-mode (SCL ~90 kHz, t_LOW/t_HIGH safely above 4.7/4.0 us).
 */

static uint32_t I2C_ComputeTiming(uint32_t i2cclk_hz)
{
  const float t_low  = 5.5e-6f;
  const float t_high = 4.5e-6f;
  const uint32_t scldel = 4U;
  const uint32_t sdadel = 0U;

  for (uint32_t presc = 0U; presc <= 15U; presc++) {
    float t_presc = (float)(presc + 1U) / (float)i2cclk_hz;
    int32_t scll = (int32_t)(t_low  / t_presc + 0.5f) - 1;
    int32_t sclh = (int32_t)(t_high / t_presc + 0.5f) - 1;

    if (scll >= 0 && scll <= 255 && sclh >= 0 && sclh <= 255) {
      return (presc << 28) | (scldel << 20) | (sdadel << 16)
           | ((uint32_t)sclh << 8) | (uint32_t)scll;
    }
  }

  return 0x10805D88UL;   /* fallback: original .ioc value for 48 MHz */
}

void INA226_PowerUp(void)
{
  MX_I2C2_Init();

  uint32_t timing = I2C_ComputeTiming(HAL_RCC_GetPCLK1Freq());
  __HAL_I2C_DISABLE(&hi2c2);
  hi2c2.Instance->TIMINGR = timing;
  __HAL_I2C_ENABLE(&hi2c2);
  hi2c2.Init.Timing = timing;
}

HAL_StatusTypeDef INA226_Init(void)
{
  HAL_StatusTypeDef st = HAL_ERROR;

  for (uint8_t attempt = 0; attempt < 3; attempt++) {
    st = Write(REG_CONFIG, INA226_CONFIG_BATTERY);
    if (st == HAL_OK) {
      st = Write(REG_CALIB, INA226_CALIB_VALUE);
      if (st == HAL_OK) return HAL_OK;
    }
    HAL_Delay(10);
  }

  return st;
}

void INA226_PowerDown(void)
{
  HAL_I2C_DeInit(&hi2c2);
  __HAL_RCC_I2C2_CLK_DISABLE();

  GPIO_InitTypeDef gpio = {0};
  gpio.Mode = GPIO_MODE_ANALOG;
  gpio.Pull = GPIO_NOPULL;

  gpio.Pin = I2C_SCL_Pin;
  HAL_GPIO_Init(I2C_SCL_GPIO_Port, &gpio);

  gpio.Pin = I2C_SDA_Pin;
  HAL_GPIO_Init(I2C_SDA_GPIO_Port, &gpio);
}

void INA226_ConfigFast(void)
{
  Write(REG_CONFIG, INA226_CONFIG_FAST);
}

void INA226_ConfigBattery(void)
{
  Write(REG_CONFIG, INA226_CONFIG_BATTERY);
}