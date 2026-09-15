/**
  ******************************************************************************
  * @file    door.c
  * @brief   Door control (sunrise/sunset based)
  ******************************************************************************
  */

#include "door.h"
#include "astro.h"
#include "motor.h"
#include "endstop.h"
#include "battery.h"
#include "telemetry.h"
#include "timebase.h"
#include "lora_app.h"              /* LoRaWAN_CanBlockFor(), LoRaWAN_RequestTime() */
#include "rtc.h"
#include "stm32_timer.h"
#include "stm32_systime.h"
#include "smtc_modem_api.h"        /* smtc_modem_suspend_radio_communications() */

#define DOOR_OPEN_OFFSET_MIN   (-60)
#define DOOR_CLOSE_OFFSET_MIN  ( 60)
#define DOOR_RETRY_S           (5UL * 60UL)

#define DOOR_RETRY_MAX         3U

/*
 * Horizont planovani. Timer nikdy nenarovname dal nez na hodinu, takze
 * prechod letni/zimni cas (skok sec_of_day o 3600 s) se projevi nejpozdeji
 * za hodinu a nezustane viset timer nastaveny podle stareho pasma.
 * Baterie se stejne budi po 10 minutach, takze hodinove probuzeni
 * nestoji nic navic.
 */
#define DOOR_RESYNC_S          3600UL

/*
 * Jak casto si rikat siti o cas. smtc_modem_trig_lorawan_mac_request()
 * je JEDNORAZOVY trigger, ne periodicka sluzba - bez opakovani by se cas
 * synchronizoval jen jednou po JOINu a dal by bezel na driftu LSE.
 * 12 h staci bohate: LSE ma i v nejhorsich podminkach max 3 min/mesic.
 */
#define DOOR_TIME_SYNC_S       (12UL * 3600UL)

/* nejhorsi pripad Motor_Move(): jizda 25 s + pauza + zpetny chod 25 s + rezerva */
#define DOOR_MOVE_BUDGET_MS    55000UL
#define DOOR_DEFER_S           10U
#define DOOR_DEFER_MAX         6U      /* po minute odkladu jedeme presto */

#define DOOR_BKP_REG    RTC_BKP_DR4
#define DOOR_BKP_MAGIC  0x00D0U

typedef enum {
  DOOR_EVT_NONE = 0,
  DOOR_EVT_RESYNC,
  DOOR_EVT_SUN,
  DOOR_EVT_RETRY,
  DOOR_EVT_DEFER
} Door_Event_t;

typedef enum {
  DOOR_REQ_NONE = 0,
  DOOR_REQ_OPEN,
  DOOR_REQ_CLOSE
} Door_Request_t;

static float   door_lat = 49.5170f;
static float   door_lon = 17.6181f;
static int16_t door_sunrise_min = 360;
static int16_t door_sunset_min  = 1080;
static uint8_t door_sun_day     = 0;     /* den, pro ktery plati sunrise/sunset */

static uint32_t door_time_sync_unix = 0; /* kdy naposledy dorazil cas ze site */

static UTIL_TIMER_Object_t     door_timer;
static volatile Door_Event_t   door_pending = DOOR_EVT_NONE;
static Door_Event_t            door_armed   = DOOR_EVT_NONE;

static volatile Door_Request_t door_request = DOOR_REQ_NONE;

/* odlozene kvuli radiu - zamerne mimo Door_WorkPending(), aby zarizeni
   mohlo mezitim spat */
static Door_Request_t door_deferred_req = DOOR_REQ_NONE;
static Door_Event_t   door_deferred_evt = DOOR_EVT_NONE;
static uint8_t        door_defer_count  = 0;

static uint8_t      door_enabled  = 1;
static uint8_t      door_fault    = 0;
static Door_State_t door_reported = DOOR_STATE_UNKNOWN;

static uint8_t      door_retry_pending  = 0;
static uint32_t     door_retry_deadline = 0;
static Motor_Dir_t  door_retry_dir      = MOTOR_DIR_UP;
static uint8_t      door_retry_count    = 0;

static void Door_Schedule(void);

/* --- persistence ---------------------------------------------------------- */

static void Door_StoreState(void)
{
  uint32_t word = ((uint32_t)DOOR_BKP_MAGIC << 16)
                | ((uint32_t)(door_fault ? 1U : 0U) << 8)
                |  (uint32_t)Endstop_Last();

  HAL_PWR_EnableBkUpAccess();
  HAL_RTCEx_BKUPWrite(&hrtc, DOOR_BKP_REG, word);
}

static void Door_LoadState(void)
{
  uint32_t word = HAL_RTCEx_BKUPRead(&hrtc, DOOR_BKP_REG);

  if (((word >> 16) & 0xFFFFU) != DOOR_BKP_MAGIC) {
    (void)Endstop_Sample();
    Door_StoreState();
  } else {
    door_fault = ((word >> 8) & 0x01U) ? 1U : 0U;
    Endstop_Restore((Endstop_Pos_t)(word & 0xFFU));
  }

  door_reported = Door_GetState();
}

static void Door_PublishState(void)
{
  Door_State_t state = Door_GetState();

  if (state != door_reported) {
    door_reported = state;
    Telemetry_RequestStatus();
  }
}

/* --- timer ---------------------------------------------------------------- */

static void Door_OnTimer(void *ctx)
{
  UNUSED(ctx);
  door_pending = door_armed;
}

static void Door_ArmTimer(uint32_t seconds, Door_Event_t evt)
{
  if (seconds == 0U)
    seconds = 1U;

  door_armed = evt;
  UTIL_TIMER_Stop(&door_timer);
  UTIL_TIMER_SetPeriod(&door_timer, seconds * 1000U);   /* UTIL_TIMER bere ms */
  UTIL_TIMER_Start(&door_timer);
}

static void Door_StopTimer(void)
{
  UTIL_TIMER_Stop(&door_timer);
  door_pending = DOOR_EVT_NONE;
  door_armed   = DOOR_EVT_NONE;
}

/* --- slunce --------------------------------------------------------------- */

static void Door_UpdateSun(void)
{
  Astro_Result_t res;
  uint8_t        day = Timebase_GetDay();

  Astro_Calculate(Timebase_GetYear(), Timebase_GetMonth(), day,
                  door_lat, door_lon, Timebase_GetTimezone(), &res);

  door_sunrise_min = res.sunrise_min;
  door_sunset_min  = res.sunset_min;
  door_sun_day     = day;
}

/* prepocita slunce, jen kdyz se zmenil den (nebo pasmo posunulo datum) */
static void Door_RefreshSun(void)
{
  if (Timebase_GetDay() != door_sun_day)
    Door_UpdateSun();
}

static uint32_t Door_MinuteToSec(int32_t minute_of_day)
{
  while (minute_of_day < 0)             minute_of_day += MINS_PER_DAY;
  while (minute_of_day >= MINS_PER_DAY) minute_of_day -= MINS_PER_DAY;

  return (uint32_t)minute_of_day * 60U;
}

static uint32_t Door_OpenTime(void)
{
  return Door_MinuteToSec((int32_t)door_sunrise_min + DOOR_OPEN_OFFSET_MIN);
}

static uint32_t Door_CloseTime(void)
{
  return Door_MinuteToSec((int32_t)door_sunset_min + DOOR_CLOSE_OFFSET_MIN);
}

static uint32_t Door_DelayTo(uint32_t now, uint32_t target)
{
  if (target <= now)
    target += SECS_PER_DAY;

  return target - now;
}

/* deadline je v RTC ticich, ne v ms */
static uint32_t Door_RetryDelay(void)
{
  int32_t remain = (int32_t)(door_retry_deadline - HAL_GetTick());

  return (remain <= 0) ? 0U : ((uint32_t)remain / TICKS_PER_SEC);
}

static Motor_Dir_t Door_DesiredDir(uint32_t now)
{
  uint32_t t_open  = Door_OpenTime();
  uint32_t t_close = Door_CloseTime();

  if (t_open <= t_close)
    return (now >= t_open && now < t_close) ? MOTOR_DIR_UP : MOTOR_DIR_DOWN;

  return (now >= t_open || now < t_close) ? MOTOR_DIR_UP : MOTOR_DIR_DOWN;
}

static uint8_t Door_PosMatchesDesired(void)
{
  Motor_Dir_t   want = Door_DesiredDir(Timebase_GetSecOfDay());
  Endstop_Pos_t pos  = Endstop_Last();

  return (want == MOTOR_DIR_UP) ? (pos == ENDSTOP_POS_TOP)
                                : (pos == ENDSTOP_POS_BOTTOM);
}

/* --- planovani ------------------------------------------------------------ */

static void Door_Schedule(void)
{
  uint32_t now, delay, best;
  Door_Event_t evt;

  if (!door_enabled) {
    Door_StopTimer();
    return;
  }

  Door_RefreshSun();

  now  = Timebase_GetSecOfDay();
  best = DOOR_RESYNC_S;                  /* horizont, viz komentar u define */
  evt  = DOOR_EVT_RESYNC;

  if (!Battery_IsCritical() && !door_fault) {
    delay = Door_DelayTo(now, Door_OpenTime());
    if (delay < best) { best = delay; evt = DOOR_EVT_SUN; }

    delay = Door_DelayTo(now, Door_CloseTime());
    if (delay < best) { best = delay; evt = DOOR_EVT_SUN; }
  }

  if (door_retry_pending && !door_fault) {
    delay = Door_RetryDelay();
    if (delay < best) { best = delay; evt = DOOR_EVT_RETRY; }
  }

  Door_ArmTimer(best, evt);
}

/* --- cas ze site ---------------------------------------------------------- */

static void Door_MaintainTimeSync(void)
{
  uint32_t now = Timebase_GetUnix();

  if (!Timebase_IsValid() ||
      ((now - door_time_sync_unix) >= DOOR_TIME_SYNC_S)) {
    LoRaWAN_RequestTime();
  }
}

/* --- radio ---------------------------------------------------------------- */

/*
 * Zdvorilostni kontrola: kdyz ma modem rozdelanou praci, radeji pohyb
 * odlozime, at ho nezdrzujeme. Na bezpecnosti uz ale nestoji - tu zajistuje
 * smtc_modem_suspend_radio_communications() primo v Door_Apply().
 */
static uint8_t Door_RadioBusy(void)
{
  if (LoRaWAN_CanBlockFor(DOOR_MOVE_BUDGET_MS)) {
    door_defer_count = 0;
    return 0U;
  }

  if (++door_defer_count >= DOOR_DEFER_MAX) {
    door_defer_count = 0;
    return 0U;
  }

  return 1U;
}

/* --- pohyb ---------------------------------------------------------------- */

static void Door_RaiseFault(void)
{
  door_fault         = 1;
  door_retry_pending = 0;
  door_retry_count   = 0;
}

static void Door_Apply(Motor_Dir_t dir)
{
  Motor_Result_t r;

  /*
   * Behem pohybu bezi jadro v LP Run na 1 MHz a hlavni smycka je blokovana
   * az 50 s. Provoz SubGHz radia v LP Run neni podporovany (LP regulator
   * neutahne ~35 mA odberu PA), takze modemu radio explicitne zakazeme.
   * Tim je kolize vyloucena strukturalne, ne jen statisticky.
   */
  (void)smtc_modem_suspend_radio_communications(true);

  r = Motor_Move(dir);

  (void)smtc_modem_suspend_radio_communications(false);

  switch (r) {
    case MOTOR_OK:
    case MOTOR_ALREADY:
      door_retry_pending = 0;
      door_retry_count   = 0;
      break;

    case MOTOR_OBSTACLE:
      if (++door_retry_count >= DOOR_RETRY_MAX) {
        Door_RaiseFault();
        break;
      }
      door_retry_pending  = 1;
      door_retry_dir      = dir;
      door_retry_deadline = HAL_GetTick() + MS_TO_TICKS(DOOR_RETRY_S * 1000UL);
      break;

    case MOTOR_NO_REFERENCE:
    case MOTOR_STUCK:
    case MOTOR_TIMEOUT:
    case MOTOR_SENSOR_ERROR:
    default:
      Door_RaiseFault();
      break;
  }

  Door_StoreState();
  Door_PublishState();
}

/* --- API ------------------------------------------------------------------ */

void Door_Init(void)
{
  UTIL_TIMER_Create(&door_timer, 0xFFFFFFFFU, UTIL_TIMER_ONESHOT,
                    Door_OnTimer, NULL);
  Door_LoadState();
}

void Door_Setup(uint16_t y, uint8_t mo, uint8_t d,
                uint8_t h, uint8_t mi, uint8_t s,
                float latitude, float longitude)
{
  door_lat = latitude;
  door_lon = longitude;

  /*
   * Zalozni cas. Parametry jsou v lokalnim case; pro prevod na UTC pouzijeme
   * CET, protoze zalozni datum lezi v lednu, kde letni cas neplati.
   * Timebase zustane oznaceny jako neplatny, dokud nedorazi cas ze site.
   */
  Timebase_SetFallback(Timebase_ToUnix(y, mo, d, h, mi, s, 1.0f));

  Door_UpdateSun();
  Door_Schedule();
}

void Door_SetUnixTime(uint32_t unix_sec)
{
  SysTime_t st;

  if (unix_sec < TIMEBASE_MIN_UNIX)
    return;                        /* zjevne neplatny cas, ignorujeme */

  /* SysTime je perzistentni uloziste (prezije reset pres BKP registry),
     Timebase je pracovni kopie */
  st.Seconds    = unix_sec;
  st.SubSeconds = 0;
  SysTimeSet(st);

  Timebase_SetUnix(unix_sec);
  door_time_sync_unix = unix_sec;

  Door_UpdateSun();
  Door_Schedule();
  Door_Catchup();                  /* dozene polohu, pokud se datum posunulo */
}

void Door_Reschedule(void)
{
  Door_Schedule();
}

/*
 * Resync z RTC. Srovna integraci po sekundach v Timebase proti presnemu
 * RTC casu a obnovi cas po resetu, pokud uz SysTime jednou nastaveny byl.
 */
void Door_SyncFromSysTime(void)
{
  SysTime_t t = SysTimeGet();

  /* puvodni test "> 0" neodhalil stav, kdy SysTimeGet() vraci pouhy uptime */
  if (t.Seconds >= TIMEBASE_MIN_UNIX) {
    Timebase_SetUnix(t.Seconds);
    Door_UpdateSun();
  }

  Door_Schedule();
}

void Door_Catchup(void)
{
  if (!door_enabled || door_fault || Battery_IsCritical())
    return;

  door_pending = DOOR_EVT_SUN;
}

void Door_RequestOpen(void)  { door_request = DOOR_REQ_OPEN;  }
void Door_RequestClose(void) { door_request = DOOR_REQ_CLOSE; }

void Door_SetFault(void)
{
  Door_RaiseFault();
  door_request      = DOOR_REQ_NONE;
  door_deferred_req = DOOR_REQ_NONE;
  door_deferred_evt = DOOR_EVT_NONE;
  Door_StoreState();
  Door_PublishState();
  Door_Schedule();
}

void Door_ClearFault(void)
{
  door_fault         = 0;
  door_retry_pending = 0;
  door_retry_count   = 0;
  (void)Endstop_Sample();
  Door_StoreState();
  Door_PublishState();
  Door_Schedule();
}

void Door_Enable(void)
{
  if (door_enabled)
    return;

  door_enabled = 1;
  Door_SyncFromSysTime();
  Door_Catchup();
}

void Door_Disable(void)
{
  door_enabled       = 0;
  door_request       = DOOR_REQ_NONE;
  door_deferred_req  = DOOR_REQ_NONE;
  door_deferred_evt  = DOOR_EVT_NONE;
  door_retry_pending = 0;
  door_retry_count   = 0;
  Door_StopTimer();
}

uint8_t Door_IsEnabled(void) { return door_enabled; }
uint8_t Door_IsFaulted(void) { return door_fault; }

uint8_t Door_WorkPending(void)
{
  return (uint8_t)((door_request != DOOR_REQ_NONE) ||
                   (door_pending != DOOR_EVT_NONE));
}

Door_State_t Door_GetState(void)
{
  if (door_fault)
    return DOOR_STATE_JAMMED;

  switch (Endstop_Last()) {
    case ENDSTOP_POS_TOP:    return DOOR_STATE_OPEN;
    case ENDSTOP_POS_BOTTOM: return DOOR_STATE_CLOSED;
    default:                 return DOOR_STATE_UNKNOWN;
  }
}

Door_State_t Door_RefreshState(void)
{
  (void)Endstop_Sample();
  return Door_GetState();
}

void Door_Process(void)
{
  Door_Request_t req;
  Door_Event_t   evt;

  req = door_request;
  door_request = DOOR_REQ_NONE;

  if (req != DOOR_REQ_NONE) {
    /* doplnena kontrola door_enabled - vypnuty system dvirky nehybe */
    if (door_fault || !door_enabled)
      return;

    if (Door_RadioBusy()) {
      door_deferred_req = req;
      Door_ArmTimer(DOOR_DEFER_S, DOOR_EVT_DEFER);
      return;
    }

    door_retry_pending = 0;
    door_retry_count   = 0;
    door_pending       = DOOR_EVT_NONE;

    Door_Apply((req == DOOR_REQ_OPEN) ? MOTOR_DIR_UP : MOTOR_DIR_DOWN);
    Door_Schedule();
    return;
  }

  evt = door_pending;
  door_pending = DOOR_EVT_NONE;

  if (evt == DOOR_EVT_NONE || !door_enabled)
    return;

  if (evt == DOOR_EVT_DEFER) {
    if (door_deferred_req != DOOR_REQ_NONE) {
      door_request      = door_deferred_req;
      door_deferred_req = DOOR_REQ_NONE;
    } else if (door_deferred_evt != DOOR_EVT_NONE) {
      door_pending      = door_deferred_evt;
      door_deferred_evt = DOOR_EVT_NONE;
    } else {
      Door_Schedule();
    }
    return;
  }

  if (evt == DOOR_EVT_RESYNC) {
    Door_SyncFromSysTime();
    Door_MaintainTimeSync();

    /* po zmene pasma nebo data muze poloha prestat odpovidat dennimu case */
    if (!door_fault && !Battery_IsCritical() && !Door_PosMatchesDesired())
      door_pending = DOOR_EVT_SUN;

    return;
  }

  if (door_fault) {
    Door_Schedule();
    return;
  }

  if (Door_RadioBusy()) {
    door_deferred_evt = evt;
    Door_ArmTimer(DOOR_DEFER_S, DOOR_EVT_DEFER);
    return;
  }

  if (evt == DOOR_EVT_RETRY) {
    if (!door_retry_pending) {
      Door_Schedule();
      return;
    }
    door_retry_pending = 0;
    Door_Apply(door_retry_dir);
  } else if (evt == DOOR_EVT_SUN) {
    Door_Apply(Door_DesiredDir(Timebase_GetSecOfDay()));
  } else {
    /* neznama udalost - nic nedelame, jen preplanujeme */
    Door_Schedule();
    return;
  }

  Door_Schedule();

  if (!door_fault && !door_retry_pending && !Battery_IsCritical() &&
      !Door_PosMatchesDesired())
    door_pending = DOOR_EVT_SUN;
}