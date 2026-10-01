import { mkdirSync } from 'node:fs';
import { dirname } from 'node:path';
import { DatabaseSync } from 'node:sqlite';
import { config } from './config.js';
import { DOOR_STATES } from './codec.js';

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

const insert = db.prepare(`
  INSERT OR REPLACE INTO readings
    (device, time, battery_mv, panel_mv, battery_critical, battery_saturated,
     panel_saturated, door, rssi, snr, sf, gateway)
  VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)
`);

const historyStatements = new Map();

function historyStatement(bucket) {
  let statement = historyStatements.get(bucket);
  if (!statement) {
    statement = db.prepare(`
      SELECT ((strftime('%s', time / 1000, 'unixepoch', 'localtime') * 1000) / ${bucket}) * ${bucket} AS slot,
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

function bucketFor(hours) {
  if (hours <= 24) return 10 * 60 * 1000;
  if (hours <= 168) return 60 * 60 * 1000;
  if (hours <= 720) return 6 * 60 * 60 * 1000;
  if (hours <= 8760) return 24 * 60 * 60 * 1000;
  return 7 * 24 * 60 * 60 * 1000;
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

export async function countReadings(deviceId) {
  return Number(db.prepare('SELECT COUNT(*) AS n FROM readings WHERE device = ?').get(deviceId).n);
}

export async function clearReadings(deviceId) {
  const { changes } = db.prepare('DELETE FROM readings WHERE device = ?').run(deviceId);
  db.exec('VACUUM');
  return Number(changes);
}

export async function closeDb() {
  db.close();
}
