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
#define INA226_I2C_MIN_HZ       2000000UL
#define INA226_SCL_LOW_PS       5500000UL
#define INA226_SCL_HIGH_PS      4500000UL
#define INA226_SCL_DELAY_PS     1250000UL
#define INA226_SDA_HOLD_PS      250000UL
#define INA226_SDA_VALID_PS     2190000UL
#define INA226_ATTEMPTS         3U
#define INA226_RETRY_MS         2U
#define INA226_V_MIN_VALID_V    1.0f
#define INA226_V_MAX_VALID_V    10.0f

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

static uint32_t INA226_Cycles(uint32_t ps, uint32_t period_ps)
{
  return (ps + period_ps - 1U) / period_ps;
}

static uint32_t INA226_ComputeTiming(uint32_t i2cclk_hz)
{
  uint32_t clk_ps, hold_ps, valid_ps, presc;

  if (i2cclk_hz < INA226_I2C_MIN_HZ)
    return 0U;

  clk_ps   = (uint32_t)(1000000000000ULL / i2cclk_hz);
  hold_ps  = (INA226_SDA_HOLD_PS > 3U * clk_ps) ? (INA226_SDA_HOLD_PS - 3U * clk_ps) : 0U;
  valid_ps = (INA226_SDA_VALID_PS > 4U * clk_ps) ? (INA226_SDA_VALID_PS - 4U * clk_ps) : 0U;

  for (presc = 0U; presc <= 15U; presc++) {
    uint32_t t_ps   = (presc + 1U) * clk_ps;
    uint32_t scll   = INA226_Cycles(INA226_SCL_LOW_PS, t_ps) - 1U;
    uint32_t sclh   = INA226_Cycles(INA226_SCL_HIGH_PS, t_ps) - 1U;
    uint32_t scldel = INA226_Cycles(INA226_SCL_DELAY_PS, t_ps) - 1U;
    uint32_t sdadel = INA226_Cycles(hold_ps, t_ps);

    if ((scll <= 255U) && (sclh <= 255U) && (scldel <= 15U) && (sdadel <= 15U) && ((sdadel * t_ps) <= valid_ps))
      return (presc << 28) | (scldel << 20) | (sdadel << 16) | (sclh << 8) | scll;
  }

  return 0U;
}

static void INA226_ClearError(void)
{
  __HAL_I2C_DISABLE(&hi2c2);
  hi2c2.ErrorCode = HAL_I2C_ERROR_NONE;
  hi2c2.State     = HAL_I2C_STATE_READY;
  hi2c2.Mode      = HAL_I2C_MODE_NONE;
  __HAL_I2C_ENABLE(&hi2c2);
}

static HAL_StatusTypeDef INA226_Transfer(uint8_t reg, uint16_t *val, uint8_t write)
{
  uint8_t buf[3] = { reg, 0U, 0U };
  HAL_StatusTypeDef st;

  if (write) {
    buf[1] = (uint8_t)(*val >> 8);
    buf[2] = (uint8_t)*val;
    st = HAL_I2C_Master_Transmit(&hi2c2, INA226_I2C_ADDR, buf, 3U, INA226_I2C_TIMEOUT_MS);
  } else {
    st = HAL_I2C_Master_Transmit(&hi2c2, INA226_I2C_ADDR, buf, 1U, INA226_I2C_TIMEOUT_MS);
    if (st == HAL_OK)
      st = HAL_I2C_Master_Receive(&hi2c2, INA226_I2C_ADDR, &buf[1], 2U, INA226_I2C_TIMEOUT_MS);
    if (st == HAL_OK)
      *val = (uint16_t)(((uint16_t)buf[1] << 8) | buf[2]);
  }

  if (st != HAL_OK)
    INA226_ClearError();

  return st;
}

static HAL_StatusTypeDef INA226_Write(uint8_t reg, uint16_t val)
{
  HAL_StatusTypeDef st = HAL_ERROR;
  uint8_t attempt;

  for (attempt = 0U; (attempt < INA226_ATTEMPTS) && (st != HAL_OK); attempt++) {
    if (attempt != 0U)
      HAL_Delay(INA226_RETRY_MS);
    st = INA226_Transfer(reg, &val, 1U);
  }

  return st;
}

static HAL_StatusTypeDef INA226_ReadReg(uint8_t reg, uint16_t *val)
{
  return INA226_Transfer(reg, val, 0U);
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

static HAL_StatusTypeDef INA226_Start(void)
{
  uint32_t timing = INA226_ComputeTiming(HAL_RCC_GetPCLK1Freq());

  if (hi2c2.State != HAL_I2C_STATE_RESET)
    (void)HAL_I2C_DeInit(&hi2c2);

  if (timing == 0U)
    return HAL_ERROR;

  if (INA226_BusStuck()) {
    INA226_BusRecover();
    if (INA226_BusStuck())
      return HAL_ERROR;
  }

  hi2c2.Instance              = I2C2;
  hi2c2.Init.Timing           = timing;
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

HAL_StatusTypeDef INA226_PowerUp(void)
{
  HAL_StatusTypeDef st = HAL_ERROR;
  uint8_t attempt;

  for (attempt = 0U; (attempt < INA226_ATTEMPTS) && (st != HAL_OK); attempt++) {
    if (attempt != 0U)
      HAL_Delay(INA226_RETRY_MS);
    st = INA226_Start();
  }

  return st;
}

void INA226_PowerDown(void)
{
  if (hi2c2.State == HAL_I2C_STATE_READY)
    (void)INA226_Write(INA226_REG_CONFIG, (uint16_t)(ina226_cfg & ~INA226_MODE_MASK));

  (void)HAL_I2C_DeInit(&hi2c2);
}

HAL_StatusTypeDef INA226_ConfigBattery(void)
{
  return INA226_Configure(INA226_CONFIG_BATTERY);
}

HAL_StatusTypeDef INA226_ConfigFast(void)
{
  return INA226_Configure(INA226_CONFIG_FAST);
}

uint8_t INA226_VoltagePlausible(float voltage)
{
  return (uint8_t)((voltage >= INA226_V_MIN_VALID_V) && (voltage <= INA226_V_MAX_VALID_V));
}

HAL_StatusTypeDef INA226_Read(float *voltage, float *current)
{
  uint16_t raw_bus = 0U, raw_current = 0U;
  HAL_StatusTypeDef st;

  st = INA226_ReadReg(INA226_REG_BUS_V, &raw_bus);
  if ((st == HAL_OK) && (current != NULL))
    st = INA226_ReadReg(INA226_REG_CURRENT, &raw_current);
  if (st != HAL_OK)
    return st;

  *voltage = (float)raw_bus * INA226_BUS_V_LSB_V;
  if (current != NULL)
    *current = fabsf((float)(int16_t)raw_current * INA226_CURRENT_LSB_A);

  return HAL_OK;
}
