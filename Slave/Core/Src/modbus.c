/**
  ******************************************************************************
  * @file    modbus.c
  * @brief   Modbus RTU slave over RS485
  ******************************************************************************
  */

#include "modbus.h"
#include "nest.h"
#include "usart.h"
#include <string.h>

#define MODBUS_FRAME_MAX       32U
#define MODBUS_FRAME_MIN       4U
#define MODBUS_REGS_MAX        ((MODBUS_FRAME_MAX - 5U) / 2U)
#define MODBUS_TX_TIMEOUT_MS   100U
#define MODBUS_CRC_INIT        0xFFFFU
#define MODBUS_CRC_POLY        0xA001U

#define MODBUS_READ_HOLDING    0x03U
#define MODBUS_WRITE_MULTIPLE  0x10U
#define MODBUS_EXCEPTION       0x80U
#define MODBUS_READ_LEN        8U
#define MODBUS_WRITE_HEAD      7U

static uint8_t           modbus_rx[MODBUS_FRAME_MAX];
static uint8_t           modbus_tx[MODBUS_FRAME_MAX];
static uint16_t          modbus_regs[MODBUS_REGS_MAX];
static volatile uint16_t modbus_rx_len   = 0;
static volatile uint8_t  modbus_rx_ready = 0;

static uint16_t Modbus_Crc(const uint8_t *buf, uint16_t len)
{
  uint16_t crc = MODBUS_CRC_INIT;
  uint16_t n;
  uint8_t  bit;

  for (n = 0U; n < len; n++) {
    crc ^= buf[n];
    for (bit = 0U; bit < 8U; bit++)
      crc = (crc & 0x0001U) ? (uint16_t)((crc >> 1) ^ MODBUS_CRC_POLY) : (uint16_t)(crc >> 1);
  }

  return crc;
}

static uint16_t Modbus_Word(const uint8_t *buf)
{
  return (uint16_t)(((uint16_t)buf[0] << 8) | buf[1]);
}

static void Modbus_Listen(void)
{
  modbus_rx_ready = 0U;

  if (HAL_UARTEx_ReceiveToIdle_DMA(&hlpuart1, modbus_rx, MODBUS_FRAME_MAX) == HAL_OK)
    __HAL_DMA_DISABLE_IT(hlpuart1.hdmarx, DMA_IT_HT);
}

static void Modbus_Send(uint16_t len)
{
  uint16_t crc = Modbus_Crc(modbus_tx, len);

  modbus_tx[len++] = (uint8_t)crc;
  modbus_tx[len++] = (uint8_t)(crc >> 8);

  (void)HAL_UART_Transmit(&hlpuart1, modbus_tx, len, MODBUS_TX_TIMEOUT_MS);
}

static uint16_t Modbus_Read(uint16_t start, uint16_t count, uint8_t *exc)
{
  uint16_t n;

  if ((count == 0U) || (count > MODBUS_REGS_MAX)) {
    *exc = MODBUS_EXC_VALUE;
    return 0U;
  }

  *exc = Nest_ReadRegisters(start, count, modbus_regs);
  if (*exc != MODBUS_OK)
    return 0U;

  modbus_tx[2] = (uint8_t)(2U * count);
  for (n = 0U; n < count; n++) {
    modbus_tx[3U + 2U * n] = (uint8_t)(modbus_regs[n] >> 8);
    modbus_tx[4U + 2U * n] = (uint8_t)modbus_regs[n];
  }

  return (uint16_t)(3U + 2U * count);
}

static uint16_t Modbus_Write(uint16_t start, uint16_t count, uint16_t len, uint8_t *exc)
{
  uint16_t n;

  if (len != (uint16_t)(MODBUS_WRITE_HEAD + modbus_rx[6] + 2U))
    return 0U;

  if ((count == 0U) || (count > MODBUS_REGS_MAX) || (modbus_rx[6] != 2U * count)) {
    *exc = MODBUS_EXC_VALUE;
    return 0U;
  }

  for (n = 0U; n < count; n++)
    modbus_regs[n] = Modbus_Word(&modbus_rx[MODBUS_WRITE_HEAD + 2U * n]);

  *exc = Nest_WriteRegisters(start, count, modbus_regs);
  if (*exc != MODBUS_OK)
    return 0U;

  return 6U;
}

static uint8_t Modbus_Handle(uint16_t len)
{
  uint16_t crc, start, count, reply = 0U;
  uint8_t  exc = MODBUS_OK;

  if ((len < MODBUS_FRAME_MIN) || (len > MODBUS_FRAME_MAX) || (modbus_rx[0] != NEST_ADDRESS))
    return 0U;

  crc = Modbus_Crc(modbus_rx, (uint16_t)(len - 2U));
  if ((modbus_rx[len - 2U] != (uint8_t)crc) || (modbus_rx[len - 1U] != (uint8_t)(crc >> 8)))
    return 0U;

  memcpy(modbus_tx, modbus_rx, 6U);
  start = Modbus_Word(&modbus_rx[2]);
  count = Modbus_Word(&modbus_rx[4]);

  switch (modbus_rx[1]) {
    case MODBUS_READ_HOLDING:
      if (len != MODBUS_READ_LEN)
        return 0U;
      reply = Modbus_Read(start, count, &exc);
      break;

    case MODBUS_WRITE_MULTIPLE:
      if (len < MODBUS_WRITE_HEAD + 2U)
        return 0U;
      reply = Modbus_Write(start, count, len, &exc);
      if ((reply == 0U) && (exc == MODBUS_OK))
        return 0U;
      break;

    default:
      exc = MODBUS_EXC_FUNCTION;
      break;
  }

  if (exc != MODBUS_OK) {
    modbus_tx[1] = (uint8_t)(modbus_rx[1] | MODBUS_EXCEPTION);
    modbus_tx[2] = exc;
    reply = 3U;
  }

  Modbus_Send(reply);

  return 1U;
}

void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t size)
{
  if (huart != &hlpuart1)
    return;

  modbus_rx_len   = size;
  modbus_rx_ready = 1U;
}

void Modbus_Start(void)
{
  Modbus_Listen();
}

uint8_t Modbus_Process(void)
{
  uint8_t answered;

  if (!modbus_rx_ready) {
    if (hlpuart1.RxState == HAL_UART_STATE_READY)
      Modbus_Listen();
    return 0U;
  }

  answered = Modbus_Handle(modbus_rx_len);
  Modbus_Listen();

  return answered;
}

void Modbus_Restart(void)
{
  (void)HAL_UART_AbortReceive(&hlpuart1);
  (void)HAL_UART_DeInit(&hlpuart1);
  MX_LPUART1_UART_Init();
  Modbus_Listen();
}

void Modbus_Stop(void)
{
  (void)HAL_UART_AbortReceive(&hlpuart1);
  (void)HAL_UART_DeInit(&hlpuart1);
}
