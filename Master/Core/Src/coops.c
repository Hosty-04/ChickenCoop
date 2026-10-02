/**
  ******************************************************************************
  * @file    coops.c
  * @brief   Nest units (Modbus RTU over RS485)
  ******************************************************************************
  */

#include "coops.h"
#include "battery.h"
#include "telemetry.h"
#include "timebase.h"
#include "power.h"
#include "usart.h"
#include "lora_app.h"
#include "stm32_timer.h"

#define COOPS_CHECK_S          3600UL
#define COOPS_AHEAD_S          2UL
#define COOPS_DEFER_S          10UL
#define COOPS_DEFER_MAX        6U
#define COOPS_BLOCK_MS         ((uint32_t)COOPS_NEST_COUNT * 14000UL)

#define COOPS_BOOT_MS          10U
#define COOPS_SAMPLE_MS        3700U
#define COOPS_POLL_MS          250U
#define COOPS_DONE_MS          9000UL
#define COOPS_REPLY_MS         200UL
#define COOPS_GAP_MS           5UL
#define COOPS_TX_TIMEOUT_MS    100U
#define COOPS_ATTEMPTS         3U

#define COOPS_REG_COMMAND      0U
#define COOPS_REG_STATUS       2U
#define COOPS_STATUS_REGS      3U

#define COOPS_CMD_MEASURE      1U
#define COOPS_CMD_TARE         2U
#define COOPS_CMD_CALIBRATE    3U
#define COOPS_CMD_RELEASE      4U

#define COOPS_STATUS_DONE      2U

#define COOPS_FLAG_BROODY      0x0001U
#define COOPS_FLAG_UNCALIB     0x0002U
#define COOPS_FLAG_FAULT       0x0004U

#define MODBUS_READ_HOLDING    0x03U
#define MODBUS_WRITE_MULTIPLE  0x10U
#define MODBUS_FRAME_MAX       16U
#define MODBUS_CRC_INIT        0xFFFFU
#define MODBUS_CRC_POLY        0xA001U

typedef struct {
  Coops_State_t state;
  uint8_t       eggs;
  uint8_t       request;
} Coops_Nest_t;

static UTIL_TIMER_Object_t coops_timer;
static volatile uint8_t    coops_pending = 0;

static uint8_t      coops_enabled = 1;
static uint8_t      coops_defer   = 0;
static Coops_Nest_t coops_nest[COOPS_NEST_COUNT];

static uint8_t coops_tx[MODBUS_FRAME_MAX];
static uint8_t coops_rx[MODBUS_FRAME_MAX];

static void Coops_CtrlPin(uint32_t mode)
{
  GPIO_InitTypeDef gpio = {0};

  __HAL_RCC_GPIOA_CLK_ENABLE();

  gpio.Pin   = LPUART_CTRL_Pin;
  gpio.Mode  = mode;
  gpio.Pull  = GPIO_NOPULL;
  gpio.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LPUART_CTRL_GPIO_Port, &gpio);
}

static void Coops_RxPullUp(void)
{
  GPIO_InitTypeDef gpio = {0};

  gpio.Pin       = LPUART_RX_Pin;
  gpio.Mode      = GPIO_MODE_AF_PP;
  gpio.Pull      = GPIO_PULLUP;
  gpio.Speed     = GPIO_SPEED_FREQ_LOW;
  gpio.Alternate = GPIO_AF8_LPUART1;
  HAL_GPIO_Init(LPUART_RX_GPIO_Port, &gpio);
}

static HAL_StatusTypeDef Coops_PowerUp(void)
{
  HAL_GPIO_WritePin(LPUART_CTRL_GPIO_Port, LPUART_CTRL_Pin, GPIO_PIN_RESET);
  Coops_CtrlPin(GPIO_MODE_OUTPUT_PP);
  HAL_GPIO_WritePin(COM_GPIO_Port, COM_Pin, GPIO_PIN_RESET);
  HAL_Delay(COOPS_BOOT_MS);

  if (HAL_UART_Init(&hlpuart1) != HAL_OK)
    return HAL_ERROR;

  Coops_RxPullUp();

  return HAL_OK;
}

static void Coops_PowerDown(void)
{
  (void)HAL_UART_DeInit(&hlpuart1);
  Coops_CtrlPin(GPIO_MODE_ANALOG);
  HAL_GPIO_WritePin(COM_GPIO_Port, COM_Pin, GPIO_PIN_SET);
}

static uint16_t Coops_Crc(const uint8_t *buf, uint8_t len)
{
  uint16_t crc = MODBUS_CRC_INIT;
  uint8_t  n, bit;

  for (n = 0U; n < len; n++) {
    crc ^= buf[n];
    for (bit = 0U; bit < 8U; bit++)
      crc = (crc & 0x0001U) ? (uint16_t)((crc >> 1) ^ MODBUS_CRC_POLY) : (uint16_t)(crc >> 1);
  }

  return crc;
}

static HAL_StatusTypeDef Coops_Send(uint8_t len)
{
  HAL_StatusTypeDef st;

  HAL_GPIO_WritePin(LPUART_CTRL_GPIO_Port, LPUART_CTRL_Pin, GPIO_PIN_SET);
  st = HAL_UART_Transmit(&hlpuart1, coops_tx, len, COOPS_TX_TIMEOUT_MS);
  HAL_GPIO_WritePin(LPUART_CTRL_GPIO_Port, LPUART_CTRL_Pin, GPIO_PIN_RESET);

  __HAL_UART_SEND_REQ(&hlpuart1, UART_RXDATA_FLUSH_REQUEST);
  __HAL_UART_CLEAR_FLAG(&hlpuart1, UART_CLEAR_OREF | UART_CLEAR_NEF | UART_CLEAR_FEF);

  return st;
}

static uint8_t Coops_FrameValid(uint8_t got, uint8_t len)
{
  uint16_t crc;

  if ((got != len) || (coops_rx[0] != coops_tx[0]) || (coops_rx[1] != coops_tx[1]))
    return 0U;

  crc = Coops_Crc(coops_rx, (uint8_t)(len - 2U));

  return (uint8_t)((coops_rx[len - 2U] == (uint8_t)crc) &&
                   (coops_rx[len - 1U] == (uint8_t)(crc >> 8)));
}

static HAL_StatusTypeDef Coops_Receive(uint8_t len)
{
  uint32_t start = HAL_GetTick();
  uint32_t last  = start;
  uint8_t  got   = 0U;

  while (TICKS_TO_MS(HAL_GetTick() - start) < COOPS_REPLY_MS) {
    if ((hlpuart1.Instance->ISR & (UART_FLAG_ORE | UART_FLAG_NE | UART_FLAG_FE)) != 0U)
      __HAL_UART_CLEAR_FLAG(&hlpuart1, UART_CLEAR_OREF | UART_CLEAR_NEF | UART_CLEAR_FEF);

    if (__HAL_UART_GET_FLAG(&hlpuart1, UART_FLAG_RXNE)) {
      uint8_t byte = (uint8_t)hlpuart1.Instance->RDR;

      if (got < MODBUS_FRAME_MAX)
        coops_rx[got] = byte;
      got++;
      last = HAL_GetTick();
      continue;
    }

    if ((got != 0U) && (TICKS_TO_MS(HAL_GetTick() - last) >= COOPS_GAP_MS)) {
      if (Coops_FrameValid(got, len))
        return HAL_OK;
      got = 0U;
    }
  }

  return Coops_FrameValid(got, len) ? HAL_OK : HAL_TIMEOUT;
}

static HAL_StatusTypeDef Coops_Transact(uint8_t tx_len, uint8_t rx_len, uint8_t attempts)
{
  uint16_t crc = Coops_Crc(coops_tx, tx_len);
  uint8_t  attempt;

  coops_tx[tx_len++] = (uint8_t)crc;
  coops_tx[tx_len++] = (uint8_t)(crc >> 8);

  for (attempt = 0U; attempt < attempts; attempt++) {
    if ((Coops_Send(tx_len) == HAL_OK) && (Coops_Receive(rx_len) == HAL_OK))
      return HAL_OK;
  }

  return HAL_TIMEOUT;
}

static HAL_StatusTypeDef Coops_Read(uint8_t addr, uint16_t reg, uint16_t *val, uint8_t count)
{
  uint8_t n;

  coops_tx[0] = addr;
  coops_tx[1] = MODBUS_READ_HOLDING;
  coops_tx[2] = (uint8_t)(reg >> 8);
  coops_tx[3] = (uint8_t)reg;
  coops_tx[4] = 0U;
  coops_tx[5] = count;

  if ((Coops_Transact(6U, (uint8_t)(5U + 2U * count), 1U) != HAL_OK) ||
      (coops_rx[2] != (uint8_t)(2U * count)))
    return HAL_ERROR;

  for (n = 0U; n < count; n++)
    val[n] = (uint16_t)(((uint16_t)coops_rx[3U + 2U * n] << 8) | coops_rx[4U + 2U * n]);

  return HAL_OK;
}

static HAL_StatusTypeDef Coops_Command(uint8_t addr, uint16_t cmd, uint16_t arg)
{
  coops_tx[0]  = addr;
  coops_tx[1]  = MODBUS_WRITE_MULTIPLE;
  coops_tx[2]  = 0U;
  coops_tx[3]  = COOPS_REG_COMMAND;
  coops_tx[4]  = 0U;
  coops_tx[5]  = 2U;
  coops_tx[6]  = 4U;
  coops_tx[7]  = (uint8_t)(cmd >> 8);
  coops_tx[8]  = (uint8_t)cmd;
  coops_tx[9]  = (uint8_t)(arg >> 8);
  coops_tx[10] = (uint8_t)arg;

  return Coops_Transact(11U, 8U, COOPS_ATTEMPTS);
}

static uint16_t Coops_Argument(uint16_t cmd)
{
  if (cmd == COOPS_CMD_MEASURE)   return (uint16_t)Timebase_GetDateKey();
  if (cmd == COOPS_CMD_CALIBRATE) return COOPS_CALIB_MASS_G;

  return 0U;
}

static Coops_State_t Coops_StateOf(uint16_t flags)
{
  if (flags & COOPS_FLAG_FAULT)   return COOPS_NEST_FAULT;
  if (flags & COOPS_FLAG_UNCALIB) return COOPS_NEST_UNCALIBRATED;
  if (flags & COOPS_FLAG_BROODY)  return COOPS_NEST_BROODY;

  return COOPS_NEST_OK;
}

static uint8_t Coops_Serve(uint8_t nest)
{
  Coops_Nest_t *n    = &coops_nest[nest];
  uint8_t       addr = (uint8_t)(nest + 1U);
  uint16_t      cmd  = (n->request != 0U) ? n->request : COOPS_CMD_MEASURE;
  uint16_t      res[COOPS_STATUS_REGS];
  uint32_t      start;
  uint8_t       done;

  n->state = COOPS_NEST_OFFLINE;

  if (Coops_Command(addr, cmd, Coops_Argument(cmd)) != HAL_OK)
    return 0U;

  n->request = 0U;
  HAL_Delay(COOPS_SAMPLE_MS);
  start = HAL_GetTick();

  while (1) {
    done = (uint8_t)((Coops_Read(addr, COOPS_REG_STATUS, res, COOPS_STATUS_REGS) == HAL_OK) &&
                     (res[0] == COOPS_STATUS_DONE));

    if (done || (TICKS_TO_MS(HAL_GetTick() - start) >= COOPS_DONE_MS))
      break;

    HAL_Delay(COOPS_POLL_MS);
  }

  if (done) {
    n->state = Coops_StateOf(res[2]);
    n->eggs  = (uint8_t)((res[1] > UINT8_MAX) ? UINT8_MAX : res[1]);
  }

  (void)Coops_Command(addr, COOPS_CMD_RELEASE, 0U);
  HAL_Delay(COOPS_BOOT_MS);

  return 1U;
}

static void Coops_OnTimer(void *ctx)
{
  UNUSED(ctx);
  coops_pending = 1U;
}

static void Coops_ArmTimer(uint32_t seconds)
{
  UTIL_TIMER_Stop(&coops_timer);
  UTIL_TIMER_SetPeriod(&coops_timer, ((seconds != 0UL) ? seconds : 1UL) * 1000UL);
  UTIL_TIMER_Start(&coops_timer);
}

static void Coops_Schedule(void)
{
  uint32_t now  = Timebase_GetSecOfDay();
  uint32_t next = (((now + COOPS_AHEAD_S) / COOPS_CHECK_S) + 1UL) * COOPS_CHECK_S - COOPS_AHEAD_S;

  Coops_ArmTimer(next - now);
}

static uint8_t Coops_Allowed(void)
{
  return (uint8_t)(coops_enabled && !Battery_IsCritical());
}

static uint8_t Coops_RadioBusy(void)
{
  if (LoRaWAN_IsIdleFor(COOPS_BLOCK_MS) || (++coops_defer >= COOPS_DEFER_MAX)) {
    coops_defer = 0U;
    return 0U;
  }

  return 1U;
}

static void Coops_Request(uint8_t nest, uint8_t cmd)
{
  if (!coops_enabled || (nest >= COOPS_NEST_COUNT))
    return;

  coops_nest[nest].request = cmd;
  coops_pending            = 1U;
}

void Coops_Init(void)
{
  uint8_t nest;

  for (nest = 0U; nest < COOPS_NEST_COUNT; nest++)
    coops_nest[nest].state = COOPS_NEST_OFFLINE;

  Coops_PowerDown();

  UTIL_TIMER_Create(&coops_timer, 0xFFFFFFFFU, UTIL_TIMER_ONESHOT, Coops_OnTimer, NULL);
  coops_pending = 1U;
}

void Coops_Process(void)
{
  uint8_t nest, linked;

  if (!coops_pending)
    return;
  coops_pending = 0U;

  if (!Coops_Allowed()) {
    Coops_Schedule();
    return;
  }

  if (Coops_RadioBusy()) {
    Coops_ArmTimer(COOPS_DEFER_S);
    return;
  }

  Power_SwitchToLPRunMSI1MHz();

  linked = (uint8_t)(Coops_PowerUp() == HAL_OK);

  for (nest = 0U; nest < COOPS_NEST_COUNT; nest++) {
    linked = (uint8_t)(linked && Coops_Serve(nest));
    if (!linked)
      coops_nest[nest].state = COOPS_NEST_OFFLINE;
  }

  Coops_PowerDown();

  Power_SwitchToRunHSE48MHz();

  Coops_Schedule();
  coops_pending = 0U;
  Telemetry_RequestFull();
}

uint8_t Coops_WorkPending(void)
{
  return coops_pending;
}

void Coops_Enable(void)
{
  coops_enabled = 1U;
}

void Coops_Disable(void)
{
  uint8_t nest;

  coops_enabled = 0U;

  for (nest = 0U; nest < COOPS_NEST_COUNT; nest++)
    coops_nest[nest].request = 0U;
}

void Coops_RequestTare(uint8_t nest)
{
  Coops_Request(nest, COOPS_CMD_TARE);
}

void Coops_RequestCalibrate(uint8_t nest)
{
  Coops_Request(nest, COOPS_CMD_CALIBRATE);
}

Coops_State_t Coops_GetState(uint8_t nest)
{
  return (nest < COOPS_NEST_COUNT) ? coops_nest[nest].state : COOPS_NEST_OFFLINE;
}

uint8_t Coops_GetEggs(uint8_t nest)
{
  return (nest < COOPS_NEST_COUNT) ? coops_nest[nest].eggs : 0U;
}
