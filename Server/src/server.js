import { createServer } from 'node:http';
import { fileURLToPath } from 'node:url';
import { dirname, join } from 'node:path';
import express from 'express';
import { WebSocketServer } from 'ws';
import { config } from './config.js';
import { COMMANDS } from './codec.js';
import { TtnBridge } from './ttn.js';
import { writeReading, readHistory, readLatest, closeInflux } from './influx.js';

const here = dirname(fileURLToPath(import.meta.url));
const app = express();
const http = createServer(app);
const wss = new WebSocketServer({ server: http, path: '/ws' });
const ttn = new TtnBridge();

const state = {
  latest: null,
  ttnConnected: false,
  influxOk: null,
  influxError: null
};

app.use(express.json());
app.use(express.static(join(here, '..', 'public')));
app.use('/vendor', express.static(join(here, '..', 'node_modules', 'chart.js', 'dist')));

function broadcast(type, data) {
  const message = JSON.stringify({ type, data });
  for (const socket of wss.clients) {
    if (socket.readyState === socket.OPEN) socket.send(message);
  }
}

app.get('/api/status', (req, res) => {
  res.json({
    device: config.ttn.deviceId,
    ttnConnected: state.ttnConnected,
    influxOk: state.influxOk,
    influxError: state.influxError,
    latest: state.latest
  });
});

app.get('/api/history', async (req, res) => {
  const hours = Math.min(Math.max(Number(req.query.hours) || 24, 1), 720);
  try {
    res.json({ hours, points: await readHistory(config.ttn.deviceId, hours) });
  } catch (err) {
    res.status(502).json({ error: `InfluxDB query failed: ${err.message}` });
  }
});

app.get('/api/commands', (req, res) => res.json({ commands: Object.keys(COMMANDS) }));

app.post('/api/command', async (req, res) => {
  const names = Array.isArray(req.body?.commands)
    ? req.body.commands
    : (req.body?.command ? [req.body.command] : []);

  try {
    const sent = await ttn.sendCommand(names);
    broadcast('command', { ...sent, queuedAt: new Date().toISOString() });
    res.json({ ok: true, ...sent });
  } catch (err) {
    res.status(400).json({ ok: false, error: err.message });
  }
});

wss.on('connection', (socket) => {
  socket.send(JSON.stringify({ type: 'status', data: state }));
});

ttn.on('state', ({ connected }) => {
  state.ttnConnected = connected;
  broadcast('status', state);
  console.log(connected ? 'TTN connected' : 'TTN disconnected');
});

ttn.on('ready', (topic) => console.log(`subscribed to ${topic}`));
ttn.on('error', (err) => console.error('TTN:', err.message));

ttn.on('uplink', async (uplink) => {
  state.latest = uplink;
  broadcast('uplink', uplink);

  const { batteryMv, panelMv, door } = uplink.reading;
  console.log(`uplink fCnt=${uplink.fCnt} battery=${batteryMv ?? '-'} panel=${panelMv ?? '-'} door=${door}`);

  try {
    await writeReading(uplink.deviceId, uplink.reading, uplink.radio, new Date(uplink.receivedAt));
    state.influxOk = true;
    state.influxError = null;
  } catch (err) {
    state.influxOk = false;
    state.influxError = err.message;
    console.error('InfluxDB write failed:', err.message);
    broadcast('status', state);
  }
});

async function seedFromInflux() {
  try {
    const latest = await readLatest(config.ttn.deviceId);
    if (latest) state.latest = { deviceId: config.ttn.deviceId, fCnt: null, ...latest };
    state.influxOk = true;
  } catch (err) {
    state.influxOk = false;
    state.influxError = err.message;
    console.error('InfluxDB unreachable at startup:', err.message);
  }
}

async function shutdown() {
  console.log('shutting down');
  await ttn.stop();
  await closeInflux().catch(() => {});
  http.close(() => process.exit(0));
  setTimeout(() => process.exit(0), 3000).unref();
}

process.on('SIGINT', shutdown);
process.on('SIGTERM', shutdown);

await seedFromInflux();
ttn.start();
http.listen(config.port, () => {
  console.log(`dashboard on http://localhost:${config.port}`);
});
