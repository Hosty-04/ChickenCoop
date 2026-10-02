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

const insert = db.prepare(`
  INSERT OR REPLACE INTO readings
    (device, time, battery_mv, panel_mv, battery_critical, battery_saturated,
     panel_saturated, door, rssi, snr, sf, gateway)
  VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)
`);

const insertNest = db.prepare(`
  INSERT OR REPLACE INTO nests (device, time, nest, eggs, state) VALUES (?, ?, ?, ?, ?)
`);

const HOUR_MS = 60 * 60 * 1000;
const DAY_MS = 24 * HOUR_MS;
const MONDAY_OFFSET = 4 * DAY_MS;


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

const selectLatest = db.prepare(`
  SELECT * FROM readings WHERE device = ? AND time <= ? ORDER BY time DESC LIMIT 1
`);

const selectOldest = db.prepare(`
  SELECT MIN(time) AS oldest FROM readings WHERE device = ?
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

const selectOldestNest = db.prepare(`
  SELECT MIN(time) AS oldest FROM nests WHERE device = ?
`);

function bucketFor(hours) {
  if (hours <= 24) return 10 * 60 * 1000;
  if (hours <= 168) return 60 * 60 * 1000;
  if (hours <= 720) return 6 * 60 * 60 * 1000;
  if (hours <= 8760) return 24 * 60 * 60 * 1000;
  return 7 * 24 * 60 * 60 * 1000;
}

function eggBucketFor(hours) {
  if (hours <= 24) return HOUR_MS;
  if (hours <= 720) return DAY_MS;
  return 7 * DAY_MS;
}

function toLocalClock(ms) {
  const d = new Date(ms);
  return Date.UTC(
    d.getFullYear(), d.getMonth(), d.getDate(),
    d.getHours(), d.getMinutes(), d.getSeconds()
  );
}

function startOfToday(now) {
  const d = new Date(now);
  d.setHours(0, 0, 0, 0);
  return d.getTime();
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

function fromLocalClock(ms) {
  const parts = new Date(ms);
  return new Date(
    parts.getUTCFullYear(), parts.getUTCMonth(), parts.getUTCDate(),
    parts.getUTCHours(), parts.getUTCMinutes()
  );
}

function round(value) {
  return value === null || value === undefined ? null : Math.round(value);
}

export async function writeReading(deviceId, reading, radio, at) {
  db.exec('BEGIN');
  try {
    insertReading(deviceId, reading, radio, at);
    reading.nests?.forEach((nest, index) => {
      insertNest.run(deviceId, at.getTime(), index + 1, nest.eggs, NEST_STATES.indexOf(nest.state));
    });
    db.exec('COMMIT');
  } catch (err) {
    db.exec('ROLLBACK');
    throw err;
  }
}

function insertReading(deviceId, reading, radio, at) {
  insert.run(
    deviceId,
    at.getTime(),
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
}

export async function readHistory(deviceId, hours) {
  const now = Date.now();

  if (hours === null) {
    const oldest = selectOldest.get(deviceId)?.oldest;
    if (!oldest) return [];
    hours = Math.max((now - oldest) / (60 * 60 * 1000), 1);
  }

  const bucket = bucketFor(hours);
  const since = now - hours * 60 * 60 * 1000;

  return historyStatement(bucket).all(deviceId, since, now).map((row) => ({
    time: fromLocalClock(row.slot).toISOString(),
    batteryMv: round(row.battery_mv),
    panelMv: round(row.panel_mv)
  }));
}

export async function readLatest(deviceId) {
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

export async function readNests(deviceId, nestCount) {
  const now = Date.now();
  const latest = selectNestsAt.all(deviceId, deviceId, now);
  if (latest.length === 0) return null;

  const known = eggsBefore(deviceId, nestCount, now + 1);
  const laid = Array(nestCount).fill(0);
  for (const row of laidBetween(deviceId, nestCount, startOfToday(now), now)) laid[row.nest - 1] += row.laid;
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

export async function readEggs(deviceId, hours, nestCount) {
  const now = Date.now();
  const align = hours === null ? Math.floor : Math.ceil;

  if (hours === null) {
    const oldest = selectOldestNest.get(deviceId)?.oldest;
    if (!oldest) return { bucketMs: DAY_MS, points: [] };
    hours = Math.max((now - oldest) / HOUR_MS, 1);
  }

  const bucket = eggBucketFor(hours);
  const since = now - hours * HOUR_MS;
  const rows = laidBetween(deviceId, nestCount, since, now);
  if (rows.length === 0) return { bucketMs: bucket, points: [] };

  const laid = new Map();
  for (const row of rows) {
    const slot = Math.floor((toLocalClock(row.time) - MONDAY_OFFSET) / bucket) * bucket + MONDAY_OFFSET;
    if (!laid.has(slot)) laid.set(slot, Array(nestCount).fill(null));
    laid.get(slot)[row.nest - 1] = (laid.get(slot)[row.nest - 1] ?? 0) + row.laid;
  }

  const points = [];
  const first = align((toLocalClock(since) - MONDAY_OFFSET) / bucket) * bucket + MONDAY_OFFSET;
  for (let slot = first; slot <= toLocalClock(now); slot += bucket) {
    const perNest = laid.get(slot) ?? Array(nestCount).fill(null);
    const known = perNest.filter((value) => value !== null);
    points.push({
      time: fromLocalClock(slot).toISOString(),
      total: known.length === 0 ? null : known.reduce((sum, value) => sum + value, 0),
      laid: perNest
    });
  }

  return { bucketMs: bucket, points };
}

export async function countReadings(deviceId) {
  return Number(db.prepare('SELECT COUNT(*) AS n FROM readings WHERE device = ?').get(deviceId).n);
}

export async function clearReadings(deviceId) {
  const { changes } = db.prepare('DELETE FROM readings WHERE device = ?').run(deviceId);
  db.prepare('DELETE FROM nests WHERE device = ?').run(deviceId);
  db.exec('VACUUM');
  return Number(changes);
}

export async function closeDb() {
  db.close();
}
