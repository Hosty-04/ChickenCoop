import { mkdirSync } from 'node:fs';
import { dirname } from 'node:path';
import { DatabaseSync } from 'node:sqlite';
import { config } from './config.js';
import { DOOR_STATES, NEST_STATES } from './codec.js';

mkdirSync(dirname(config.dbPath), { recursive: true });

const db = new DatabaseSync(config.dbPath);

db.exec('PRAGMA journal_mode = WAL');
db.exec('PRAGMA synchronous = NORMAL');
db.exec(`
  CREATE TABLE IF NOT EXISTS readings (
    device TEXT NOT NULL,
    time INTEGER NOT NULL,
    battery_mv INTEGER,
    panel_mv INTEGER,
    battery_critical INTEGER NOT NULL DEFAULT 0,
    battery_saturated INTEGER NOT NULL DEFAULT 0,
    panel_saturated INTEGER NOT NULL DEFAULT 0,
    door INTEGER NOT NULL DEFAULT 3,
    rssi REAL,
    snr REAL,
    sf INTEGER,
    gateway TEXT,
    PRIMARY KEY (device, time)
  ) WITHOUT ROWID
`);
db.exec(`
  CREATE TABLE IF NOT EXISTS nests (
    device TEXT NOT NULL,
    time INTEGER NOT NULL,
    nest INTEGER NOT NULL,
    eggs INTEGER,
    state INTEGER NOT NULL DEFAULT 4,
    PRIMARY KEY (device, time, nest)
  ) WITHOUT ROWID
`);

const HOUR_MS = 60 * 60 * 1000;
const DAY_MS = 24 * HOUR_MS;
const WEEK_MS = 7 * DAY_MS;
const MONDAY_OFFSET = 4 * DAY_MS;
const LAID_SLACK_MS = 10 * 60 * 1000;

const insertReading = db.prepare(`
  INSERT OR REPLACE INTO readings
    (device, time, battery_mv, panel_mv, battery_critical, battery_saturated,
     panel_saturated, door, rssi, snr, sf, gateway)
  VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)
`);

const insertNest = db.prepare(`
  INSERT OR REPLACE INTO nests (device, time, nest, eggs, state) VALUES (?, ?, ?, ?, ?)
`);

const selectLatest = db.prepare(`
  SELECT * FROM readings WHERE device = ? AND time <= ? ORDER BY time DESC LIMIT 1
`);

const selectOldest = db.prepare(`
  SELECT MIN(time) AS oldest FROM readings WHERE device = ?
`);

const selectCount = db.prepare(`
  SELECT COUNT(*) AS n FROM readings WHERE device = ?
`);

const selectNestsAt = db.prepare(`
  SELECT nest, eggs, state, time FROM nests
  WHERE device = ? AND time = (SELECT MAX(time) FROM nests WHERE device = ? AND time <= ?)
`);

const selectEggsBefore = db.prepare(`
  SELECT eggs FROM nests
  WHERE device = ? AND nest = ? AND eggs IS NOT NULL AND time < ?
  ORDER BY time DESC LIMIT 1
`);

const selectEggRows = db.prepare(`
  SELECT nest, time, eggs FROM nests
  WHERE device = ? AND eggs IS NOT NULL AND time >= ? AND time <= ?
  ORDER BY time
`);

const deleteReadings = db.prepare('DELETE FROM readings WHERE device = ?');
const deleteNests = db.prepare('DELETE FROM nests WHERE device = ?');

const historyStatements = new Map();

function historyStatement(bucket) {
  let statement = historyStatements.get(bucket);
  if (!statement) {
    statement = db.prepare(`
      SELECT (((strftime('%s', time / 1000, 'unixepoch', 'localtime') * 1000 - ${MONDAY_OFFSET}) / ${bucket}) * ${bucket}) + ${MONDAY_OFFSET} AS slot,
             AVG(battery_mv) AS battery_mv,
             COALESCE(AVG(NULLIF(panel_mv, 0)), AVG(panel_mv)) AS panel_mv
      FROM readings
      WHERE device = ? AND time >= ? AND time <= ?
      GROUP BY slot
      ORDER BY slot
    `);
    historyStatements.set(bucket, statement);
  }
  return statement;
}

function transaction(work) {
  db.exec('BEGIN');
  try {
    const result = work();
    db.exec('COMMIT');
    return result;
  } catch (err) {
    db.exec('ROLLBACK');
    throw err;
  }
}

function historyBucket(hours) {
  if (hours <= 24) return 10 * 60 * 1000;
  if (hours <= 168) return HOUR_MS;
  if (hours <= 720) return 6 * HOUR_MS;
  if (hours <= 8760) return DAY_MS;
  return WEEK_MS;
}

function eggBucket(hours) {
  if (hours <= 24) return HOUR_MS;
  if (hours <= 720) return DAY_MS;
  return WEEK_MS;
}

function toLocalClock(ms) {
  const d = new Date(ms);
  return Date.UTC(d.getFullYear(), d.getMonth(), d.getDate(), d.getHours(), d.getMinutes(), d.getSeconds());
}

function fromLocalClock(ms) {
  const d = new Date(ms);
  return new Date(d.getUTCFullYear(), d.getUTCMonth(), d.getUTCDate(), d.getUTCHours(), d.getUTCMinutes());
}

function slotOf(ms, bucket, align = Math.floor) {
  return align((toLocalClock(ms) - MONDAY_OFFSET) / bucket) * bucket + MONDAY_OFFSET;
}

function startOfToday(now) {
  return new Date(now).setHours(0, 0, 0, 0);
}

function rounded(value) {
  return value == null ? null : Math.round(value);
}

function eggsBefore(deviceId, nestCount, time) {
  return Array.from({ length: nestCount }, (_, index) =>
    selectEggsBefore.get(deviceId, index + 1, time)?.eggs ?? null);
}

function laidBetween(deviceId, nestCount, since, until) {
  const last = eggsBefore(deviceId, nestCount, since);

  return selectEggRows.all(deviceId, since, until)
    .filter((row) => row.nest >= 1 && row.nest <= nestCount)
    .map((row) => {
      const previous = last[row.nest - 1];
      last[row.nest - 1] = row.eggs;
      return { nest: row.nest, time: row.time, laid: previous === null ? 0 : Math.max(row.eggs - previous, 0) };
    });
}

export function writeReading(deviceId, reading, radio, at) {
  const time = at.getTime();
  transaction(() => {
    insertReading.run(
      deviceId,
      time,
      reading.batteryMv,
      reading.panelMv,
      reading.batteryCritical ? 1 : 0,
      reading.batterySaturated ? 1 : 0,
      reading.panelSaturated ? 1 : 0,
      DOOR_STATES.indexOf(reading.door),
      radio?.rssi ?? null,
      radio?.snr ?? null,
      radio?.spreadingFactor ?? null,
      radio?.gateway ?? null
    );
    reading.nests?.forEach((nest, index) => {
      insertNest.run(deviceId, time, index + 1, nest.eggs, NEST_STATES.indexOf(nest.state));
    });
  });
}

export function readHistory(deviceId, hours) {
  const now = Date.now();
  const oldest = selectOldest.get(deviceId)?.oldest;
  const span = hours ?? (oldest ? Math.max((now - oldest) / HOUR_MS, 1) : 1);
  const bucket = historyBucket(span);
  if (!oldest) return { bucketMs: bucket, points: [] };

  return {
    bucketMs: bucket,
    points: historyStatement(bucket).all(deviceId, now - span * HOUR_MS, now).map((row) => ({
      time: fromLocalClock(row.slot).toISOString(),
      batteryMv: rounded(row.battery_mv),
      panelMv: rounded(row.panel_mv)
    }))
  };
}

export function readLatest(deviceId) {
  const row = selectLatest.get(deviceId, Date.now());
  if (!row) return null;

  return {
    receivedAt: new Date(row.time).toISOString(),
    reading: {
      batteryMv: row.battery_mv,
      panelMv: row.panel_mv,
      batteryCritical: row.battery_critical === 1,
      batterySaturated: row.battery_saturated === 1,
      panelSaturated: row.panel_saturated === 1,
      door: DOOR_STATES[row.door] ?? 'unknown'
    },
    radio: {
      rssi: row.rssi,
      snr: row.snr,
      spreadingFactor: row.sf,
      gateway: row.gateway
    }
  };
}

export function readNests(deviceId, nestCount) {
  const now = Date.now();
  const latest = selectNestsAt.all(deviceId, deviceId, now);
  if (latest.length === 0) return null;

  const known = eggsBefore(deviceId, nestCount, now + 1);
  const laid = Array(nestCount).fill(0);
  for (const row of laidBetween(deviceId, nestCount, startOfToday(now) + LAID_SLACK_MS, now)) {
    laid[row.nest - 1] += row.laid;
  }
  const current = new Map(latest.map((row) => [row.nest, row]));

  const nests = Array.from({ length: nestCount }, (_, index) => {
    const row = current.get(index + 1);
    return {
      state: row ? (NEST_STATES[row.state] ?? 'offline') : null,
      eggs: row?.eggs ?? known[index],
      laidToday: laid[index]
    };
  });

  return {
    checkedAt: new Date(latest[0].time).toISOString(),
    laidToday: nests.reduce((sum, nest) => sum + nest.laidToday, 0),
    nests
  };
}

export function readEggs(deviceId, hours, nestCount) {
  const now = Date.now();
  const oldest = selectOldest.get(deviceId)?.oldest;
  if (!oldest) return { bucketMs: DAY_MS, points: [] };

  const span = hours ?? Math.max((now - oldest) / HOUR_MS, 1);
  const bucket = eggBucket(span);
  const reported = now - HOUR_MS;
  const first = hours === null ? slotOf(oldest, bucket) : slotOf(reported - span * HOUR_MS, bucket, Math.ceil);
  const last = slotOf(reported, bucket);
  if (last < first) return { bucketMs: bucket, points: [] };

  const rows = laidBetween(deviceId, nestCount, fromLocalClock(first).getTime() + LAID_SLACK_MS, now);
  if (rows.length === 0) return { bucketMs: bucket, points: [] };

  const laid = new Map();
  for (const row of rows) {
    const slot = slotOf(row.time - LAID_SLACK_MS, bucket);
    const perNest = laid.get(slot) ?? Array(nestCount).fill(null);
    perNest[row.nest - 1] = (perNest[row.nest - 1] ?? 0) + row.laid;
    laid.set(slot, perNest);
  }

  const points = [];
  for (let slot = first; slot <= last; slot += bucket) {
    const perNest = laid.get(slot) ?? Array(nestCount).fill(null);
    const known = perNest.filter((value) => value !== null);
    points.push({
      time: fromLocalClock(slot).toISOString(),
      total: known.length === 0 ? null : known.reduce((sum, value) => sum + value, 0),
      laid: perNest
    });
  }

  return {
    bucketMs: bucket,
    end: fromLocalClock(last + bucket).toISOString(),
    since: new Date(oldest).toISOString(),
    points
  };
}

export function countReadings(deviceId) {
  return Number(selectCount.get(deviceId).n);
}

export function clearReadings(deviceId) {
  const removed = transaction(() => {
    const { changes } = deleteReadings.run(deviceId);
    deleteNests.run(deviceId);
    return Number(changes);
  });
  db.exec('VACUUM');
  return removed;
}

export function closeDb() {
  db.close();
}
