import { InfluxDB, Point } from '@influxdata/influxdb-client';
import { config } from './config.js';
import { DOOR_STATES } from './codec.js';

const MEASUREMENT = 'coop';

const client = new InfluxDB({ url: config.influx.url, token: config.influx.token });
const writeApi = client.getWriteApi(config.influx.org, config.influx.bucket, 'ms');
const queryApi = client.getQueryApi(config.influx.org);

function windowFor(hours) {
  if (hours <= 24) return '10m';
  if (hours <= 168) return '1h';
  return '6h';
}

export async function writeReading(deviceId, reading, radio, at) {
  const point = new Point(MEASUREMENT)
    .tag('device', deviceId)
    .intField('door', DOOR_STATES.indexOf(reading.door))
    .booleanField('battery_critical', reading.batteryCritical)
    .timestamp(at);

  if (reading.batteryMv !== null) point.intField('battery_mv', reading.batteryMv);
  if (reading.panelMv !== null) point.intField('panel_mv', reading.panelMv);
  if (radio?.rssi !== undefined) point.floatField('rssi', radio.rssi);
  if (radio?.snr !== undefined) point.floatField('snr', radio.snr);
  if (radio?.spreadingFactor !== undefined) point.intField('sf', radio.spreadingFactor);

  writeApi.writePoint(point);
  await writeApi.flush();
}

export async function readHistory(deviceId, hours) {
  const flux = `
    from(bucket: "${config.influx.bucket}")
      |> range(start: -${hours}h)
      |> filter(fn: (r) => r._measurement == "${MEASUREMENT}")
      |> filter(fn: (r) => r.device == "${deviceId}")
      |> filter(fn: (r) => r._field == "battery_mv" or r._field == "panel_mv")
      |> aggregateWindow(every: ${windowFor(hours)}, fn: mean, createEmpty: false)
      |> pivot(rowKey: ["_time"], columnKey: ["_field"], valueColumn: "_value")
      |> sort(columns: ["_time"])
  `;

  const rows = [];
  for await (const { values, tableMeta } of queryApi.iterateRows(flux)) {
    const row = tableMeta.toObject(values);
    rows.push({
      time: row._time,
      batteryMv: row.battery_mv === undefined ? null : Math.round(row.battery_mv),
      panelMv: row.panel_mv === undefined ? null : Math.round(row.panel_mv)
    });
  }
  return rows;
}

export async function readLatest(deviceId) {
  const flux = `
    from(bucket: "${config.influx.bucket}")
      |> range(start: -30d)
      |> filter(fn: (r) => r._measurement == "${MEASUREMENT}")
      |> filter(fn: (r) => r.device == "${deviceId}")
      |> pivot(rowKey: ["_time"], columnKey: ["_field"], valueColumn: "_value")
      |> sort(columns: ["_time"], desc: true)
      |> limit(n: 1)
  `;

  for await (const { values, tableMeta } of queryApi.iterateRows(flux)) {
    const row = tableMeta.toObject(values);
    return {
      receivedAt: row._time,
      reading: {
        batteryMv: row.battery_mv ?? null,
        panelMv: row.panel_mv ?? null,
        batteryCritical: row.battery_critical === true || row.battery_critical === 'true',
        door: DOOR_STATES[row.door] ?? 'unknown',
        panelSaturated: false,
        batterySaturated: false
      },
      radio: { rssi: row.rssi ?? null, snr: row.snr ?? null, spreadingFactor: row.sf ?? null }
    };
  }
  return null;
}

export async function closeInflux() {
  await writeApi.close();
}
