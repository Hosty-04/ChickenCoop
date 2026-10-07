/**
  ******************************************************************************
  * @file    nests.c
  * @brief   Nest units (Modbus RTU over RS485)
  ******************************************************************************
  */

#include "nests.h"
#include "battery.h"
#include "telemetry.h"
#include "timebase.h"
#include "power.h"
#include "usart.h"
#include "lora_app.h"
#include "stm32_timer.h"

#define NESTS_DEFER_S          10UL
#define NESTS_DEFER_MAX        4U
#define NESTS_BLOCK_MS         ((uint32_t)NESTS_COUNT * 15000UL)

#define NESTS_BOOT_MS          10U
#define NESTS_SAMPLE_MS        3700U
#define NESTS_POLL_MS          250U
#define NESTS_DONE_MS          10000UL
#define NESTS_REPLY_MS         200UL
#define NESTS_READ_MS          (NESTS_REPLY_MS + 20UL)
#define NESTS_GAP_MS           5UL
#define NESTS_TX_TIMEOUT_MS    100U
#define NESTS_ATTEMPTS         3U

#define NESTS_REG_COMMAND      0U
#define NESTS_REG_STATUS       2U
#define NESTS_STATUS_REGS      3U

#define NESTS_CMD_MEASURE      1U
#define NESTS_CMD_TARE         2U
#define NESTS_CMD_CALIBRATE    3U
#define NESTS_CMD_RELEASE      4U

#define NESTS_STATUS_DONE      2U

#define NESTS_FLAG_BROODY      0x0001U
#define NESTS_FLAG_UNCALIB     0x0002U
#define NESTS_FLAG_FAULT       0x0004U

#define MODBUS_READ_HOLDING    0x03U
#define MODBUS_WRITE_MULTIPLE  0x10U
#define MODBUS_FRAME_MAX       16U
#define MODBUS_CRC_INIT        0xFFFFU
#define MODBUS_CRC_POLY        0xA001U

typedef struct {
  Nests_State_t state;
  uint8_t       eggs;
  uint8_t       request;
} Nests_Data_t;

static UTIL_TIMER_Object_t nests_timer;
static volatile uint8_t    nests_pending = 0;
static uint32_t            nests_due     = 0;

static uint8_t      nests_enabled = 1;
static uint8_t      nests_defer   = 0;
static Nests_Data_t nests_data[NESTS_COUNT];

static uint8_t nests_tx[MODBUS_FRAME_MAX];
static uint8_t nests_rx[MODBUS_FRAME_MAX];

static void Nests_CtrlPin(uint32_t mode)
{
  GPIO_InitTypeDef gpio = {0};

  __HAL_RCC_GPIOA_CLK_ENABLE();

  gpio.Pin   = LPUART_CTRL_Pin;
  gpio.Mode  = mode;
  gpio.Pull  = GPIO_NOPULL;
  gpio.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LPUART_CTRL_GPIO_Port, &gpio);
}

static void Nests_RxPullUp(void)
{
  GPIO_InitTypeDef gpio = {0};

  gpio.Pin       = LPUART_RX_Pin;
  gpio.Mode      = GPIO_MODE_AF_PP;
  gpio.Pull      = GPIO_PULLUP;
  gpio.Speed     = GPIO_SPEED_FREQ_LOW;
  gpio.Alternate = GPIO_AF8_LPUART1;
  HAL_GPIO_Init(LPUART_RX_GPIO_Port, &gpio);
}

static HAL_StatusTypeDef Nests_PowerUp(void)
{
  HAL_GPIO_WritePin(LPUART_CTRL_GPIO_Port, LPUART_CTRL_Pin, GPIO_PIN_RESET);
  Nests_CtrlPin(GPIO_MODE_OUTPUT_PP);
  HAL_GPIO_WritePin(COM_GPIO_Port, COM_Pin, GPIO_PIN_RESET);
  HAL_Delay(NESTS_BOOT_MS);

  if ((HAL_UART_Init(&hlpuart1) != HAL_OK) ||
      (HAL_UARTEx_EnableFifoMode(&hlpuart1) != HAL_OK))
    return HAL_ERROR;

  Nests_RxPullUp();

  return HAL_OK;
}

static void Nests_PowerDown(void)
{
  (void)HAL_UART_DeInit(&hlpuart1);
  Nests_CtrlPin(GPIO_MODE_ANALOG);
  HAL_GPIO_WritePin(COM_GPIO_Port, COM_Pin, GPIO_PIN_SET);
}

static uint16_t Nests_Crc(const uint8_t *buf, uint8_t len)
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

static HAL_StatusTypeDef Nests_Send(uint8_t len)
{
  HAL_StatusTypeDef st;

  HAL_GPIO_WritePin(LPUART_CTRL_GPIO_Port, LPUART_CTRL_Pin, GPIO_PIN_SET);
  st = HAL_UART_Transmit(&hlpuart1, nests_tx, len, NESTS_TX_TIMEOUT_MS);
  HAL_GPIO_WritePin(LPUART_CTRL_GPIO_Port, LPUART_CTRL_Pin, GPIO_PIN_RESET);

  __HAL_UART_SEND_REQ(&hlpuart1, UART_RXDATA_FLUSH_REQUEST);
  __HAL_UART_CLEAR_FLAG(&hlpuart1, UART_CLEAR_OREF | UART_CLEAR_NEF | UART_CLEAR_FEF);

  return st;
}

static uint8_t Nests_FrameValid(uint8_t got, uint8_t len)
{
  uint16_t crc;

  if ((got != len) || (nests_rx[0] != nests_tx[0]) || (nests_rx[1] != nests_tx[1]))
    return 0U;

  crc = Nests_Crc(nests_rx, (uint8_t)(len - 2U));

  return (uint8_t)((nests_rx[len - 2U] == (uint8_t)crc) &&
                   (nests_rx[len - 1U] == (uint8_t)(crc >> 8)));
}

static HAL_StatusTypeDef Nests_Receive(uint8_t len)
{
  uint32_t start = HAL_GetTick();
  uint32_t last  = start;
  uint8_t  got   = 0U;

  while (TICKS_TO_MS(HAL_GetTick() - start) < NESTS_REPLY_MS) {
    if ((hlpuart1.Instance->ISR & (UART_FLAG_ORE | UART_FLAG_NE | UART_FLAG_FE)) != 0U)
      __HAL_UART_CLEAR_FLAG(&hlpuart1, UART_CLEAR_OREF | UART_CLEAR_NEF | UART_CLEAR_FEF);

    if (__HAL_UART_GET_FLAG(&hlpuart1, UART_FLAG_RXNE)) {
      uint8_t byte = (uint8_t)hlpuart1.Instance->RDR;

      if (got < MODBUS_FRAME_MAX)
        nests_rx[got] = byte;
      got++;
      last = HAL_GetTick();
      continue;
    }

    if ((got != 0U) && (TICKS_TO_MS(HAL_GetTick() - last) >= NESTS_GAP_MS)) {
      if (Nests_FrameValid(got, len))
        return HAL_OK;
      got = 0U;
    }
  }

  return Nests_FrameValid(got, len) ? HAL_OK : HAL_TIMEOUT;
}

static HAL_StatusTypeDef Nests_Transact(uint8_t tx_len, uint8_t rx_len, uint8_t attempts)
{
  uint16_t crc = Nests_Crc(nests_tx, tx_len);
  uint8_t  attempt;

  nests_tx[tx_len++] = (uint8_t)crc;
  nests_tx[tx_len++] = (uint8_t)(crc >> 8);

  for (attempt = 0U; attempt < attempts; attempt++) {
    if ((Nests_Send(tx_len) == HAL_OK) && (Nests_Receive(rx_len) == HAL_OK))
      return HAL_OK;
  }

  return HAL_TIMEOUT;
}

static HAL_StatusTypeDef Nests_Read(uint8_t addr, uint16_t reg, uint16_t *val, uint8_t count)
{
  uint8_t n;

  nests_tx[0] = addr;
  nests_tx[1] = MODBUS_READ_HOLDING;
  nests_tx[2] = (uint8_t)(reg >> 8);
  nests_tx[3] = (uint8_t)reg;
  nests_tx[4] = 0U;
  nests_tx[5] = count;

  if ((Nests_Transact(6U, (uint8_t)(5U + 2U * count), 1U) != HAL_OK) ||
      (nests_rx[2] != (uint8_t)(2U * count)))
    return HAL_ERROR;

  for (n = 0U; n < count; n++)
    val[n] = (uint16_t)(((uint16_t)nests_rx[3U + 2U * n] << 8) | nests_rx[4U + 2U * n]);

  return HAL_OK;
}

static HAL_StatusTypeDef Nests_Command(uint8_t addr, uint16_t cmd, uint16_t arg)
{
  nests_tx[0]  = addr;
  nests_tx[1]  = MODBUS_WRITE_MULTIPLE;
  nests_tx[2]  = 0U;
  nests_tx[3]  = NESTS_REG_COMMAND;
  nests_tx[4]  = 0U;
  nests_tx[5]  = 2U;
  nests_tx[6]  = 4U;
  nests_tx[7]  = (uint8_t)(cmd >> 8);
  nests_tx[8]  = (uint8_t)cmd;
  nests_tx[9]  = (uint8_t)(arg >> 8);
  nests_tx[10] = (uint8_t)arg;

  return Nests_Transact(11U, 8U, NESTS_ATTEMPTS);
}

static uint16_t Nests_Argument(uint16_t cmd)
{
  if (cmd == NESTS_CMD_MEASURE)   return (uint16_t)Timebase_GetDateKey();
  if (cmd == NESTS_CMD_CALIBRATE) return NESTS_CALIB_MASS_G;

  return 0U;
}

static Nests_State_t Nests_StateOf(uint16_t flags)
{
  if (flags & NESTS_FLAG_FAULT)   return NESTS_STATE_FAULT;
  if (flags & NESTS_FLAG_UNCALIB) return NESTS_STATE_UNCALIBRATED;
  if (flags & NESTS_FLAG_BROODY)  return NESTS_STATE_BROODY;

  return NESTS_STATE_OK;
}

static uint8_t Nests_Release(uint8_t addr)
{
  HAL_StatusTypeDef st = Nests_Command(addr, NESTS_CMD_RELEASE, 0U);

  HAL_Delay(NESTS_BOOT_MS);

  return (uint8_t)(st == HAL_OK);
}

static uint8_t Nests_Serve(uint8_t nest, uint8_t hourly)
{
  Nests_Data_t *n    = &nests_data[nest];
  uint8_t       addr = (uint8_t)(nest + 1U);
  uint16_t      cmd  = (n->request != 0U) ? n->request : (hourly ? NESTS_CMD_MEASURE : 0U);
  uint16_t      res[NESTS_STATUS_REGS];
  uint32_t      start;
  uint8_t       done;

  if (cmd == 0U)
    return Nests_Release(addr);

  n->state = NESTS_STATE_OFFLINE;

  if (Nests_Command(addr, cmd, Nests_Argument(cmd)) != HAL_OK)
    return 0U;

  n->request = 0U;
  HAL_Delay(NESTS_SAMPLE_MS);
  start = HAL_GetTick();

  while (1) {
    done = (uint8_t)((Nests_Read(addr, NESTS_REG_STATUS, res, NESTS_STATUS_REGS) == HAL_OK) &&
                     (res[0] == NESTS_STATUS_DONE));

    if (done || (TICKS_TO_MS(HAL_GetTick() - start) + NESTS_POLL_MS + NESTS_READ_MS > NESTS_DONE_MS))
      break;

    HAL_Delay(NESTS_POLL_MS);
  }

  if (done) {
    n->state = Nests_StateOf(res[2]);
    n->eggs  = (uint8_t)((res[1] > UINT8_MAX) ? UINT8_MAX : res[1]);
  }

  (void)Nests_Release(addr);

  return 1U;
}

static uint8_t Nests_Reach(uint8_t hourly)
{
  uint8_t nest, reach = 0U;

  if (hourly)
    return NESTS_COUNT;

  for (nest = 0U; nest < NESTS_COUNT; nest++) {
    if (nests_data[nest].request != 0U)
      reach = (uint8_t)(nest + 1U);
  }

  return reach;
}

static void Nests_OnTimer(void *ctx)
{
  UNUSED(ctx);
  nests_pending = 1U;
}

static void Nests_ArmTimer(uint32_t seconds)
{
  UTIL_TIMER_Stop(&nests_timer);
  UTIL_TIMER_SetPeriod(&nests_timer, ((seconds != 0UL) ? seconds : 1UL) * 1000UL);
  UTIL_TIMER_Start(&nests_timer);
}

static void Nests_Plan(void)
{
  uint32_t now = Timebase_GetSecOfDay();

  nests_due = Timebase_GetUnix() + (((now / NESTS_CHECK_S) + 1UL) * NESTS_CHECK_S - now);
}

static void Nests_Schedule(void)
{
  uint32_t now = Timebase_GetUnix();

  Nests_ArmTimer((nests_due > now) ? (nests_due - now) : 0UL);
}

static uint8_t Nests_Allowed(void)
{
  return (uint8_t)(nests_enabled && !Battery_IsCritical());
}

static uint8_t Nests_RadioBusy(void)
{
  if (LoRaWAN_IsIdleFor(NESTS_BLOCK_MS) || (++nests_defer >= NESTS_DEFER_MAX)) {
    nests_defer = 0U;
    return 0U;
  }

  return 1U;
}

static void Nests_Request(uint8_t nest, uint8_t cmd)
{
  if (!nests_enabled || (nest >= NESTS_COUNT))
    return;

  nests_data[nest].request = cmd;
  nests_pending            = 1U;
}

void Nests_Init(void)
{
  uint8_t nest;

  for (nest = 0U; nest < NESTS_COUNT; nest++)
    nests_data[nest].state = NESTS_STATE_OFFLINE;

  Nests_PowerDown();

  UTIL_TIMER_Create(&nests_timer, 0xFFFFFFFFU, UTIL_TIMER_ONESHOT, Nests_OnTimer, NULL);
  nests_pending = 1U;
}

void Nests_Process(void)
{
  uint8_t nest, reach, hourly, linked;

  if (!nests_pending)
    return;
  nests_pending = 0U;

  hourly = (uint8_t)(Timebase_GetUnix() >= nests_due);
  reach  = Nests_Reach(hourly);

  if ((reach == 0U) || !Nests_Allowed()) {
    nests_defer = 0U;
    if (hourly) {
      Nests_Plan();
      Battery_Request();
    }
    Nests_Schedule();
    return;
  }

  if (Nests_RadioBusy()) {
    Nests_ArmTimer(NESTS_DEFER_S);
    return;
  }

  Power_SwitchToLPRunMSI1MHz();

  linked = (uint8_t)(Nests_PowerUp() == HAL_OK);

  for (nest = 0U; nest < reach; nest++) {
    linked = (uint8_t)(linked && Nests_Serve(nest, hourly));
    if (!linked)
      nests_data[nest].state = NESTS_STATE_OFFLINE;
  }

  Nests_PowerDown();

  Power_SwitchToRunHSE48MHz();

  if (hourly) {
    Nests_Plan();
    Battery_Request();
  }

  Nests_Schedule();
  nests_pending = 0U;
  Telemetry_RequestFull();
}

void Nests_Reschedule(void)
{
  if (nests_due > (Timebase_GetUnix() + 2UL * NESTS_CHECK_S))
    Nests_Plan();

  Nests_Schedule();
}

uint8_t Nests_WorkPending(void)
{
  return nests_pending;
}

void Nests_Enable(void)
{
  nests_enabled = 1U;
}

void Nests_Disable(void)
{
  uint8_t nest;

  nests_enabled = 0U;

  for (nest = 0U; nest < NESTS_COUNT; nest++)
    nests_data[nest].request = 0U;
}

void Nests_RequestTare(uint8_t nest)
{
  Nests_Request(nest, NESTS_CMD_TARE);
}

void Nests_RequestCalibrate(uint8_t nest)
{
  Nests_Request(nest, NESTS_CMD_CALIBRATE);
}

Nests_State_t Nests_GetState(uint8_t nest)
{
  return (nest < NESTS_COUNT) ? nests_data[nest].state : NESTS_STATE_OFFLINE;
}

uint8_t Nests_GetEggs(uint8_t nest)
{
  return (nest < NESTS_COUNT) ? nests_data[nest].eggs : 0U;
}
