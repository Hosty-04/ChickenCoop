/**
  ******************************************************************************
  * @file    ina226.c
  * @brief   INA226 voltage/current sensor
  ******************************************************************************
  */

#include "ina226.h"
#include "i2c.h"
#include "gpio.h"
#include <math.h>

extern I2C_HandleTypeDef hi2c2;

#define REG_CONFIG   0x00
#define REG_SHUNT_V  0x01
#define REG_BUS_V    0x02
#define REG_CURRENT  0x04
#define REG_CALIB    0x05

#define INA226_CFG_RSVD        0x4000U
#define INA226_MODE_MASK       0x0007U
#define INA226_MODE_SHUTDOWN   0x0000U

#define INA226_CONFIG_BATTERY  (INA226_CFG_RSVD | 0x0726U)
#define INA226_CONFIG_FAST     (INA226_CFG_RSVD | 0x0527U)

#define INA226_CURRENT_LSB   (2.0f / 32768.0f)
#define INA226_CALIB_VALUE   8389U

#define INA226_BUS_V_LSB     0.00125f

#define INA226_I2C_TIMEOUT_MS  50U

static uint16_t ina226_cfg = INA226_CONFIG_BATTERY;

static void INA226_BusDelay(void)
{
  volatile uint32_t n = (SystemCoreClock / 200000U) + 4U;

  while (n--) {
    __NOP();
  }
}

static uint8_t INA226_BusStuck(void)
{
  GPIO_InitTypeDef gpio = {0};
  uint8_t stuck;

  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  gpio.Mode  = GPIO_MODE_INPUT;
  gpio.Pull  = GPIO_NOPULL;
  gpio.Speed = GPIO_SPEED_FREQ_LOW;

  gpio.Pin = I2C_SDA_Pin;
  HAL_GPIO_Init(I2C_SDA_GPIO_Port, &gpio);
  gpio.Pin = I2C_SCL_Pin;
  HAL_GPIO_Init(I2C_SCL_GPIO_Port, &gpio);

  INA226_BusDelay();

  stuck = (HAL_GPIO_ReadPin(I2C_SDA_GPIO_Port, I2C_SDA_Pin) == GPIO_PIN_RESET) ||
          (HAL_GPIO_ReadPin(I2C_SCL_GPIO_Port, I2C_SCL_Pin) == GPIO_PIN_RESET);

  return stuck;
}

static void INA226_BusRecover(void)
{
  GPIO_InitTypeDef gpio = {0};
  uint8_t n;

  gpio.Mode  = GPIO_MODE_OUTPUT_OD;
  gpio.Pull  = GPIO_NOPULL;
  gpio.Speed = GPIO_SPEED_FREQ_LOW;

  gpio.Pin = I2C_SCL_Pin;
  HAL_GPIO_Init(I2C_SCL_GPIO_Port, &gpio);
  gpio.Pin = I2C_SDA_Pin;
  HAL_GPIO_Init(I2C_SDA_GPIO_Port, &gpio);

  HAL_GPIO_WritePin(I2C_SDA_GPIO_Port, I2C_SDA_Pin, GPIO_PIN_SET);
  HAL_GPIO_WritePin(I2C_SCL_GPIO_Port, I2C_SCL_Pin, GPIO_PIN_SET);
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

static HAL_StatusTypeDef Write(uint8_t reg, uint16_t val)
{
  uint8_t buf[3] = { reg, (uint8_t)(val >> 8), (uint8_t)(val & 0xFF) };

  return HAL_I2C_Master_Transmit(&hi2c2, INA226_I2C_ADDR, buf, 3,
                                 INA226_I2C_TIMEOUT_MS);
}

static void Read_ClearError(void)
{
  __HAL_I2C_DISABLE(&hi2c2);
  hi2c2.ErrorCode = HAL_I2C_ERROR_NONE;
  hi2c2.State     = HAL_I2C_STATE_READY;
  hi2c2.Mode      = HAL_I2C_MODE_NONE;
  __HAL_I2C_ENABLE(&hi2c2);
}

static HAL_StatusTypeDef Read(uint8_t reg, uint16_t *val)
{
  uint8_t buf[2];
  HAL_StatusTypeDef st;

  st = HAL_I2C_Master_Transmit(&hi2c2, INA226_I2C_ADDR, &reg, 1,
                               INA226_I2C_TIMEOUT_MS);
  if (st != HAL_OK) {
    Read_ClearError();
    return st;
  }

  st = HAL_I2C_Master_Receive(&hi2c2, INA226_I2C_ADDR, buf, 2,
                              INA226_I2C_TIMEOUT_MS);
  if (st != HAL_OK) {
    Read_ClearError();
    return st;
  }

  *val = ((uint16_t)buf[0] << 8) | buf[1];

  return HAL_OK;
}

static HAL_StatusTypeDef INA226_WriteConfig(uint16_t cfg)
{
  HAL_StatusTypeDef st = Write(REG_CONFIG, cfg);

  if (st == HAL_OK)
    ina226_cfg = cfg;

  return st;
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

  return 0x10805D88UL;
}

HAL_StatusTypeDef INA226_PowerUp(void)
{
  uint32_t timing;

  if (INA226_BusStuck())
    INA226_BusRecover();

  hi2c2.Instance              = I2C2;
  hi2c2.Init.Timing           = 0x10805D88;
  hi2c2.Init.OwnAddress1      = 0;
  hi2c2.Init.AddressingMode   = I2C_ADDRESSINGMODE_7BIT;
  hi2c2.Init.DualAddressMode  = I2C_DUALADDRESS_DISABLE;
  hi2c2.Init.OwnAddress2      = 0;
  hi2c2.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
  hi2c2.Init.GeneralCallMode  = I2C_GENERALCALL_DISABLE;
  hi2c2.Init.NoStretchMode    = I2C_NOSTRETCH_DISABLE;

  if (HAL_I2C_Init(&hi2c2) != HAL_OK)
    return HAL_ERROR;

  if (HAL_I2CEx_ConfigAnalogFilter(&hi2c2, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
    return HAL_ERROR;

  if (HAL_I2CEx_ConfigDigitalFilter(&hi2c2, 0) != HAL_OK)
    return HAL_ERROR;

  /* TIMINGR dopocitany podle aktualni PCLK1 (1 MHz v LP Run, 48 MHz v Run) */
  timing = I2C_ComputeTiming(HAL_RCC_GetPCLK1Freq());
  __HAL_I2C_DISABLE(&hi2c2);
  hi2c2.Instance->TIMINGR = timing;
  __HAL_I2C_ENABLE(&hi2c2);
  hi2c2.Init.Timing = timing;

  return HAL_OK;
}

HAL_StatusTypeDef INA226_Init(void)
{
  HAL_StatusTypeDef st = HAL_ERROR;

  for (uint8_t attempt = 0; attempt < 3; attempt++) {
    st = INA226_WriteConfig(INA226_CONFIG_BATTERY);
    if (st == HAL_OK) {
      st = Write(REG_CALIB, INA226_CALIB_VALUE);
      if (st == HAL_OK) return HAL_OK;
    }
    HAL_Delay(10);
  }

  return st;
}

HAL_StatusTypeDef INA226_Shutdown(void)
{
  uint16_t cfg = (uint16_t)((ina226_cfg & ~INA226_MODE_MASK) | INA226_MODE_SHUTDOWN);

  return Write(REG_CONFIG, cfg);
}

void INA226_PowerDown(void)
{
  GPIO_InitTypeDef gpio = {0};

  (void)INA226_Shutdown();

  HAL_I2C_DeInit(&hi2c2);
  __HAL_RCC_I2C2_CLK_DISABLE();

  gpio.Mode = GPIO_MODE_ANALOG;
  gpio.Pull = GPIO_NOPULL;

  gpio.Pin = I2C_SCL_Pin;
  HAL_GPIO_Init(I2C_SCL_GPIO_Port, &gpio);

  gpio.Pin = I2C_SDA_Pin;
  HAL_GPIO_Init(I2C_SDA_GPIO_Port, &gpio);
}

void INA226_ConfigFast(void)
{
  (void)INA226_WriteConfig(INA226_CONFIG_FAST);
}

void INA226_ConfigBattery(void)
{
  (void)INA226_WriteConfig(INA226_CONFIG_BATTERY);
}