/**
  ******************************************************************************
  * @file    ina226.c
  * @brief   INA226 voltage/current sensor
  ******************************************************************************
  */

#include "ina226.h"
#include "i2c.h"

extern I2C_HandleTypeDef hi2c2;

#define REG_CONFIG   0x00
#define REG_SHUNT_V  0x01
#define REG_BUS_V    0x02
#define REG_CURRENT  0x04
#define REG_CALIB    0x05

/* AVG=16, VBUSCT=1.1ms, VSHCT=1.1ms, continuous shunt+bus */
#define INA226_CONFIG_VALUE  0x0527U

/*
 * Rshunt = 0.010 Ohm (R010)
 * Max current ≈ 2 A → Current_LSB = 2/32768 ≈ 61.0 uA
 * CAL = 0.00512 / (Current_LSB * Rshunt) ≈ 8389
 * Resolution << 50 mA → OK
 */
#define INA226_CURRENT_LSB   (2.0f / 32768.0f)
#define INA226_CALIB_VALUE   8389U

/* Bus voltage LSB = 1.25 mV (celý 16bitový registr, žádný shift) */
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
        *current = (float)((int16_t)raw_current) * INA226_CURRENT_LSB;

    return HAL_OK;
}

HAL_StatusTypeDef INA226_Init(void)
{
    HAL_StatusTypeDef st = HAL_ERROR;

    for (uint8_t attempt = 0; attempt < 3; attempt++)
    {
        st = Write(REG_CONFIG, INA226_CONFIG_VALUE);
        if (st == HAL_OK)
        {
            st = Write(REG_CALIB, INA226_CALIB_VALUE);
            if (st == HAL_OK) return HAL_OK;
        }
        HAL_Delay(10);
    }

    return st;
}