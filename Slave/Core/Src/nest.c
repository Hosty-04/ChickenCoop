/**
  ******************************************************************************
  * @file    nest.c
  * @brief   Egg counting nest unit on the RS485 chain
  ******************************************************************************
  */

#include "nest.h"
#include "hx711.h"
#include "modbus.h"
#include "power.h"

#define NEST_INIT_MS          500U
#define NEST_SAMPLE_MS        300U
#define NEST_IDLE_MS          30000U
#define NEST_SAMPLES          32U
#define NEST_KEPT             16U
#define NEST_ATTEMPTS         3U
#define NEST_DRIFT_ROUNDS     3U
#define NEST_BROODY_CHECKS    3U

#define NEST_EGG_G            60L
#define NEST_CHANGE_G         30L
#define NEST_HEN_G            1250L
#define NEST_DRIFT_G          25L
#define NEST_STABLE_G         5L
#define NEST_EGGS_MAX         255L
#define NEST_NOMINAL_PER_KG   100000L
#define NEST_MIN_PER_KG       10000L
#define NEST_G_PER_KG         1000L

#define NEST_REG_COMMAND      0U
#define NEST_REG_ARGUMENT     1U
#define NEST_REG_STATUS       2U
#define NEST_REG_EGGS         3U
#define NEST_REG_FLAGS        4U
#define NEST_REGS             5U
#define NEST_REGS_WRITABLE    2U

#define NEST_CMD_MEASURE      1U
#define NEST_CMD_TARE         2U
#define NEST_CMD_CALIBRATE    3U
#define NEST_CMD_RELEASE      4U

#define NEST_STATUS_IDLE      0U
#define NEST_STATUS_BUSY      1U
#define NEST_STATUS_DONE      2U

#define NEST_FLAG_BROODY      0x0001U
#define NEST_FLAG_UNCALIB     0x0002U
#define NEST_FLAG_FAULT       0x0004U

#define NEST_EEPROM_CALIB     DATA_EEPROM_BASE
#define NEST_EEPROM_COUNT     (DATA_EEPROM_BASE + 16UL)
#define NEST_MAGIC_CALIB      0x4E455A31UL
#define NEST_MAGIC_COUNT      0x4E455331UL
#define NEST_RECORD_WORDS     4U
#define NEST_HEAVY_MASK       0x000000FFUL
#define NEST_DAY_SET          0x00000100UL
#define NEST_DAY_SHIFT        16U

typedef struct {
  uint8_t valid;
  int32_t zero;
  int32_t per_kg;
} Nest_Calib_t;

typedef struct {
  int32_t  stored;
  uint8_t  heavy;
  uint8_t  day_set;
  uint16_t day;
} Nest_Count_t;

typedef struct {
  uint8_t  valid;
  int32_t  mean;
  uint32_t deviation;
} Nest_Reading_t;

static Nest_Calib_t nest_calib;
static Nest_Count_t nest_count;
static Nest_Count_t nest_base;

static uint16_t nest_command   = 0;
static uint16_t nest_argument  = 0;
static uint8_t  nest_status    = NEST_STATUS_IDLE;
static uint8_t  nest_fault     = 0;
static uint8_t  nest_release   = 0;
static uint8_t  nest_slow      = 0;
static uint8_t  nest_round     = 0;
static uint8_t  nest_taken     = 0;
static int64_t  nest_drift_sum = 0;
static uint32_t nest_boot_tick;
static uint32_t nest_wait_tick;
static uint32_t nest_seen_tick;
static int32_t  nest_samples[NEST_SAMPLES];

static uint32_t Nest_Check(const uint32_t *words)
{
  return ~(words[0] ^ words[1] ^ words[2]);
}

static uint8_t Nest_Load(uint32_t addr, uint32_t magic, uint32_t *words)
{
  const volatile uint32_t *record = (const volatile uint32_t *)addr;
  uint32_t n;

  for (n = 0U; n < NEST_RECORD_WORDS; n++)
    words[n] = record[n];

  return (uint8_t)((words[0] == magic) && (words[3] == Nest_Check(words)));
}

static void Nest_Store(uint32_t addr, uint32_t *words)
{
  const volatile uint32_t *record = (const volatile uint32_t *)addr;
  uint32_t n;

  words[3] = Nest_Check(words);

  Power_MainRegulator();
  if (HAL_FLASHEx_DATAEEPROM_Unlock() == HAL_OK) {
    for (n = 0U; n < NEST_RECORD_WORDS; n++) {
      if (record[n] != words[n])
        (void)HAL_FLASHEx_DATAEEPROM_Program(FLASH_TYPEPROGRAMDATA_WORD, addr + 4UL * n, words[n]);
    }
    (void)HAL_FLASHEx_DATAEEPROM_Lock();
  }
  Power_LowRegulator();
}

static void Nest_LoadState(void)
{
  uint32_t words[NEST_RECORD_WORDS];

  nest_calib.valid  = Nest_Load(NEST_EEPROM_CALIB, NEST_MAGIC_CALIB, words);
  nest_calib.zero   = nest_calib.valid ? (int32_t)words[1] : 0;
  nest_calib.per_kg = nest_calib.valid ? (int32_t)words[2] : 0;

  if (Nest_Load(NEST_EEPROM_COUNT, NEST_MAGIC_COUNT, words)) {
    nest_count.stored  = (int32_t)words[1];
    nest_count.heavy   = (uint8_t)(words[2] & NEST_HEAVY_MASK);
    nest_count.day_set = (uint8_t)((words[2] & NEST_DAY_SET) != 0U);
    nest_count.day     = (uint16_t)(words[2] >> NEST_DAY_SHIFT);
  }

  nest_base = nest_count;
}

static void Nest_SaveCalib(void)
{
  uint32_t words[NEST_RECORD_WORDS] = { NEST_MAGIC_CALIB, (uint32_t)nest_calib.zero, (uint32_t)nest_calib.per_kg, 0U };

  nest_calib.valid = 1U;
  Nest_Store(NEST_EEPROM_CALIB, words);
}

static void Nest_SaveCount(void)
{
  uint32_t words[NEST_RECORD_WORDS] = {
    NEST_MAGIC_COUNT,
    (uint32_t)nest_count.stored,
    ((uint32_t)nest_count.day << NEST_DAY_SHIFT) | (nest_count.day_set ? NEST_DAY_SET : 0U) | nest_count.heavy,
    0U
  };

  Nest_Store(NEST_EEPROM_COUNT, words);
}

static uint8_t Nest_Calibrated(void)
{
  return (uint8_t)(nest_calib.valid && (nest_calib.per_kg != 0));
}

static int32_t Nest_Grams(int32_t counts)
{
  return (int32_t)(((int64_t)(counts - nest_calib.zero) * NEST_G_PER_KG) / nest_calib.per_kg);
}

static uint16_t Nest_Eggs(void)
{
  int32_t eggs = (nest_count.stored > 0) ? ((nest_count.stored + NEST_EGG_G / 2L) / NEST_EGG_G) : 0;

  return (uint16_t)((eggs > NEST_EGGS_MAX) ? NEST_EGGS_MAX : eggs);
}

static uint16_t Nest_Flags(void)
{
  uint16_t flags = 0U;

  if (nest_count.heavy >= NEST_BROODY_CHECKS) flags |= NEST_FLAG_BROODY;
  if (!Nest_Calibrated())                     flags |= NEST_FLAG_UNCALIB;
  if (nest_fault)                             flags |= NEST_FLAG_FAULT;

  return flags;
}

static uint32_t Nest_Sqrt(uint64_t value)
{
  uint64_t bit  = 1ULL << 62;
  uint64_t root = 0U;

  while (bit > value)
    bit >>= 2;

  while (bit != 0U) {
    if (value >= root + bit) {
      value -= root + bit;
      root   = (root >> 1) + bit;
    } else {
      root >>= 1;
    }
    bit >>= 2;
  }

  return (uint32_t)root;
}

static void Nest_Sort(int32_t *values, uint8_t count)
{
  uint8_t i, j;
  int32_t v;

  for (i = 1U; i < count; i++) {
    v = values[i];
    for (j = i; (j > 0U) && (values[j - 1U] > v); j--)
      values[j] = values[j - 1U];
    values[j] = v;
  }
}

static Nest_Reading_t Nest_Filter(void)
{
  Nest_Reading_t reading = {0};
  int32_t       *v       = nest_samples;
  int8_t         lo      = (int8_t)(NEST_SAMPLES / 2U) - 1;
  int8_t         hi      = (int8_t)(NEST_SAMPLES / 2U);
  int64_t        sum     = 0;
  uint64_t       squares = 0U;
  int32_t        median, d;
  uint8_t        kept;

  Nest_Sort(v, NEST_SAMPLES);
  median = (v[lo] + v[hi]) / 2;

  if ((v[0] == v[NEST_SAMPLES - 1U]) || (median <= HX711_MIN) || (median >= HX711_MAX))
    return reading;

  for (kept = 0U; kept < NEST_KEPT; kept++) {
    if ((hi >= (int8_t)NEST_SAMPLES) || ((lo >= 0) && ((median - v[lo]) <= (v[hi] - median))))
      sum += v[lo--];
    else
      sum += v[hi++];
  }

  reading.mean = (int32_t)(sum / (int64_t)NEST_KEPT);

  for (lo++; lo < hi; lo++) {
    d        = v[lo] - reading.mean;
    squares += (uint64_t)((int64_t)d * d);
  }

  reading.deviation = Nest_Sqrt(squares / NEST_KEPT);
  reading.valid     = 1U;

  return reading;
}

static uint8_t Nest_Stable(const Nest_Reading_t *reading)
{
  int32_t per_kg = Nest_Calibrated() ? nest_calib.per_kg : NEST_NOMINAL_PER_KG;

  if (per_kg < 0)
    per_kg = -per_kg;

  return (uint8_t)(((int64_t)reading->deviation * NEST_G_PER_KG) <= ((int64_t)NEST_STABLE_G * per_kg));
}

static void Nest_Finish(void)
{
  nest_status = NEST_STATUS_DONE;
}

static void Nest_Fail(void)
{
  nest_fault = 1U;
  Nest_Finish();
}

static void Nest_Retry(void)
{
  if (nest_round >= NEST_ATTEMPTS)
    Nest_Fail();
}

static void Nest_Drift(const Nest_Reading_t *reading, int32_t grams)
{
  if (!Nest_Stable(reading) || (grams >= NEST_DRIFT_G) || (grams <= -NEST_DRIFT_G)) {
    Nest_Finish();
    return;
  }

  nest_drift_sum += reading->mean;
  if (nest_round < NEST_DRIFT_ROUNDS)
    return;

  nest_calib.zero   = (int32_t)(nest_drift_sum / (int64_t)NEST_DRIFT_ROUNDS);
  nest_count.stored = 0;
  Nest_SaveCalib();
  Nest_SaveCount();
  nest_base = nest_count;
  Nest_Finish();
}

static void Nest_Measured(const Nest_Reading_t *reading)
{
  int32_t grams;

  if (!Nest_Calibrated()) {
    Nest_Finish();
    return;
  }

  grams = Nest_Grams(reading->mean);

  if (nest_round > 1U) {
    Nest_Drift(reading, grams);
    return;
  }

  nest_count = nest_base;

  if (grams > NEST_HEN_G) {
    if (nest_count.heavy < UINT8_MAX)
      nest_count.heavy++;
    Nest_SaveCount();
    Nest_Finish();
    return;
  }

  if (!Nest_Stable(reading)) {
    Nest_Finish();
    return;
  }

  nest_count.heavy = 0U;
  if ((grams - nest_count.stored >= NEST_CHANGE_G) || (nest_count.stored - grams >= NEST_CHANGE_G))
    nest_count.stored = grams;

  if ((grams < NEST_DRIFT_G) && (grams > -NEST_DRIFT_G) &&
      (!nest_count.day_set || (nest_count.day != nest_argument))) {
    nest_count.day_set = 1U;
    nest_count.day     = nest_argument;
    nest_drift_sum     = reading->mean;
    Nest_SaveCount();
    return;
  }

  Nest_SaveCount();
  Nest_Finish();
}

static void Nest_Tared(const Nest_Reading_t *reading)
{
  if (!Nest_Stable(reading)) {
    Nest_Retry();
    return;
  }

  nest_calib.zero   = reading->mean;
  nest_count.stored = 0;
  nest_count.heavy  = 0U;
  Nest_SaveCalib();
  Nest_SaveCount();
  nest_base = nest_count;
  Nest_Finish();
}

static void Nest_Weighed(const Nest_Reading_t *reading)
{
  int64_t counts = (int64_t)reading->mean - nest_calib.zero;
  int64_t per_kg;

  if (!Nest_Stable(reading)) {
    Nest_Retry();
    return;
  }

  per_kg = (counts * NEST_G_PER_KG) / (int64_t)nest_argument;
  if ((per_kg < NEST_MIN_PER_KG) && (per_kg > -NEST_MIN_PER_KG)) {
    Nest_Fail();
    return;
  }

  nest_calib.per_kg = (int32_t)per_kg;
  nest_count.stored = 0;
  nest_count.heavy  = 0U;
  Nest_SaveCalib();
  Nest_SaveCount();
  nest_base = nest_count;
  Nest_Finish();
}

static void Nest_Evaluate(void)
{
  Nest_Reading_t reading = Nest_Filter();

  nest_taken = 0U;
  nest_round++;

  if (!reading.valid) {
    Nest_Fail();
    return;
  }

  switch (nest_command) {
    case NEST_CMD_MEASURE:   Nest_Measured(&reading); break;
    case NEST_CMD_TARE:      Nest_Tared(&reading);    break;
    case NEST_CMD_CALIBRATE: Nest_Weighed(&reading);  break;
    default:                 Nest_Finish();           break;
  }
}

static void Nest_Start(uint16_t command)
{
  if (command == NEST_CMD_RELEASE) {
    nest_release = 1U;
    return;
  }

  nest_command   = command;
  nest_fault     = 0U;
  nest_round     = 0U;
  nest_taken     = 0U;
  nest_status    = NEST_STATUS_BUSY;
  nest_slow      = 1U;
  nest_wait_tick = HAL_GetTick();

  if ((command == NEST_CMD_CALIBRATE) && (!nest_calib.valid || (nest_argument == 0U)))
    Nest_Fail();
}

static void Nest_Shutdown(uint8_t next)
{
  Modbus_Stop();
  HX711_Off();
  Power_Release(next);
}

static void Nest_Sample(uint32_t now)
{
  if ((now - nest_boot_tick) < NEST_INIT_MS) {
    nest_wait_tick = now;
    return;
  }

  if (HX711_Ready()) {
    nest_samples[nest_taken++] = HX711_Read();
    nest_wait_tick = now;
    if (nest_taken == NEST_SAMPLES)
      Nest_Evaluate();
    return;
  }

  if ((now - nest_wait_tick) >= NEST_SAMPLE_MS)
    Nest_Fail();
}

void Nest_Init(void)
{
  nest_boot_tick = HAL_GetTick();
  nest_seen_tick = nest_boot_tick;

  Nest_LoadState();
  Modbus_Start();
}

void Nest_Process(void)
{
  uint32_t now;

  if (Modbus_Process())
    nest_seen_tick = HAL_GetTick();

  if (nest_release)
    Nest_Shutdown(1U);

  if (nest_slow) {
    nest_slow = 0U;
    Power_LowPowerRun();
    Modbus_Restart();
  }

  now = HAL_GetTick();

  if ((now - nest_seen_tick) >= NEST_IDLE_MS)
    Nest_Shutdown(0U);

  if (nest_status == NEST_STATUS_BUSY)
    Nest_Sample(now);
}

uint8_t Nest_ReadRegisters(uint16_t start, uint16_t count, uint16_t *values)
{
  uint16_t n;

  if ((start >= NEST_REGS) || (count > NEST_REGS - start))
    return MODBUS_EXC_ADDRESS;

  for (n = 0U; n < count; n++) {
    switch (start + n) {
      case NEST_REG_COMMAND:  values[n] = nest_command;  break;
      case NEST_REG_ARGUMENT: values[n] = nest_argument; break;
      case NEST_REG_STATUS:   values[n] = nest_status;   break;
      case NEST_REG_EGGS:     values[n] = Nest_Eggs();   break;
      default:                values[n] = Nest_Flags();  break;
    }
  }

  return MODBUS_OK;
}

uint8_t Nest_WriteRegisters(uint16_t start, uint16_t count, const uint16_t *values)
{
  uint16_t command  = nest_command;
  uint16_t argument = nest_argument;
  uint8_t  issued   = 0U;
  uint16_t n;

  if ((start >= NEST_REGS_WRITABLE) || (count > NEST_REGS_WRITABLE - start))
    return MODBUS_EXC_ADDRESS;

  for (n = 0U; n < count; n++) {
    if ((start + n) == NEST_REG_COMMAND) {
      command = values[n];
      issued  = 1U;
    } else {
      argument = values[n];
    }
  }

  if (issued && ((command < NEST_CMD_MEASURE) || (command > NEST_CMD_RELEASE)))
    return MODBUS_EXC_VALUE;

  if (issued && (command != NEST_CMD_RELEASE) && (nest_status != NEST_STATUS_IDLE) &&
      (command == nest_command) && (argument == nest_argument))
    return MODBUS_OK;

  nest_argument = argument;
  if (issued)
    Nest_Start(command);

  return MODBUS_OK;
}
