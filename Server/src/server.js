import { createServer } from 'node:http';
import { fileURLToPath } from 'node:url';
import { dirname, join } from 'node:path';
import express from 'express';
import { WebSocketServer } from 'ws';
import { config } from './config.js';
import { COMMANDS } from './codec.js';
import { TtnBridge } from './ttn.js';
import {
  SESSION_COOKIE, readCookie, lockoutRemainingMs, checkCredentials, passwordMatches,
  openSession, closeSession, sessionValid, requestAuthenticated
} from './auth.js';
import {
  writeReading, readHistory, readLatest, countReadings, clearReadings, closeDb
} from './db.js';

const here = dirname(fileURLToPath(import.meta.url));
const app = express();
const http = createServer(app);
const wss = new WebSocketServer({
  server: http,
  path: '/ws',
  verifyClient: ({ req }, done) => done(requestAuthenticated(req), 401, 'Unauthorized')
});
const ttn = new TtnBridge();

const state = {
  device: config.ttn.deviceId,
  latest: null,
  ttnConnected: false,
  dbOk: null,
  dbError: null,
  readings: 0,
  pending: []
};

const PUBLIC_PATHS = new Set([
  '/login.html', '/login.js', '/style.css', '/manifest.webmanifest',
  '/icon.svg', '/icon-192.png', '/icon-512.png', '/icon-maskable-512.png', '/apple-touch-icon.png'
]);

app.disable('x-powered-by');
if (config.trustProxy !== false) app.set('trust proxy', config.trustProxy);
app.use(express.json({ limit: '8kb' }));

app.post('/api/login', (req, res) => {
  const waitMs = lockoutRemainingMs(req.ip);
  if (waitMs > 0) {
    return res.status(429).json({
      ok: false,
      error: `Příliš mnoho pokusů. Zkus to za ${Math.ceil(waitMs / 60000)} min.`
    });
  }

  if (!checkCredentials(req.ip, req.body?.user, req.body?.password)) {
    return res.status(401).json({ ok: false, error: 'Nesprávné jméno nebo heslo.' });
  }

  res.cookie(SESSION_COOKIE, openSession(), {
    httpOnly: true,
    sameSite: 'lax',
    secure: req.secure,
    maxAge: 30 * 24 * 60 * 60 * 1000,
    path: '/'
  });
  res.json({ ok: true });
});

app.post('/api/logout', (req, res) => {
  closeSession(readCookie(req.headers.cookie, SESSION_COOKIE));
  res.clearCookie(SESSION_COOKIE, { path: '/' });
  res.json({ ok: true });
});

app.use((req, res, next) => {
  if (PUBLIC_PATHS.has(req.path)) return next();
  if (sessionValid(readCookie(req.headers.cookie, SESSION_COOKIE))) return next();
  if (req.path.startsWith('/api/')) return res.status(401).json({ error: 'nepřihlášen' });
  return res.redirect('/login.html');
});

app.use(express.static(join(here, '..', 'public')));
app.use('/vendor', express.static(join(here, '..', 'node_modules', 'chart.js', 'dist')));

function broadcast(type, data) {
  const message = JSON.stringify({ type, data });
  for (const socket of wss.clients) {
    if (socket.readyState === socket.OPEN) socket.send(message);
  }
}

app.get('/api/status', (req, res) => res.json(state));

app.get('/api/history', async (req, res) => {
  const all = req.query.hours === 'all';
  const hours = all ? null : Math.min(Math.max(Number(req.query.hours) || 24, 1), 8760);
  try {
    res.json({ hours: all ? 'all' : hours, points: await readHistory(config.ttn.deviceId, hours) });
  } catch (err) {
    res.status(502).json({ error: `dotaz do databáze selhal: ${err.message}` });
  }
});

app.get('/api/commands', (req, res) => res.json({ commands: Object.keys(COMMANDS) }));

app.post('/api/command', async (req, res) => {
  const names = Array.isArray(req.body?.commands)
    ? req.body.commands
    : (req.body?.command ? [req.body.command] : []);

  try {
    const sent = await ttn.sendCommand(names);
    broadcast('command', sent);
    res.json({ ok: true, ...sent });
  } catch (err) {
    res.status(400).json({ ok: false, error: err.message });
  }
});

app.post('/api/data/clear', async (req, res) => {
  if (!passwordMatches(req.body?.password)) {
    return res.status(403).json({ ok: false, error: 'Nesprávné heslo.' });
  }

  try {
    const removed = await clearReadings(config.ttn.deviceId);
    state.readings = 0;
    state.latest = null;
    state.dbOk = true;
    state.dbError = null;
    broadcast('status', state);
    console.log(`history cleared, ${removed} readings removed`);
    res.json({ ok: true, removed });
  } catch (err) {
    res.status(500).json({ ok: false, error: `mazání selhalo: ${err.message}` });
  }
});

app.post('/api/command/cancel', async (req, res) => {
  try {
    const { cleared } = await ttn.clearQueue();
    res.json({ ok: true, cleared });
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

ttn.on('ready', (topics) => console.log(`subscribed to ${topics}`));
ttn.on('error', (err) => console.error('TTN:', err.message));

ttn.on('pending', (pending) => {
  state.pending = pending;
  broadcast('pending', pending);
});

ttn.on('downlink', ({ event, commands }) => {
  broadcast('downlink', { event, commands, at: new Date().toISOString() });
  console.log(`downlink ${event}${commands ? ` (${commands.join(', ')})` : ''}`);
});

ttn.on('uplink', async (uplink) => {
  state.latest = uplink;
  broadcast('uplink', uplink);

  const { batteryMv, panelMv, door } = uplink.reading;
  console.log(`uplink fCnt=${uplink.fCnt} battery=${batteryMv ?? '-'} panel=${panelMv ?? '-'} door=${door}`);

  try {
    await writeReading(uplink.deviceId, uplink.reading, uplink.radio, new Date(uplink.receivedAt));
    state.readings = await countReadings(config.ttn.deviceId);
    state.dbOk = true;
    state.dbError = null;
  } catch (err) {
    state.dbOk = false;
    state.dbError = err.message;
    console.error('database write failed:', err.message);
    broadcast('status', state);
  }
});

async function seedFromDb() {
  try {
    const latest = await readLatest(config.ttn.deviceId);
    if (latest) state.latest = { deviceId: config.ttn.deviceId, fCnt: null, ...latest };
    state.readings = await countReadings(config.ttn.deviceId);
    state.dbOk = true;
  } catch (err) {
    state.dbOk = false;
    state.dbError = err.message;
    console.error('database unreadable at startup:', err.message);
  }
}

async function shutdown() {
  console.log('shutting down');
  await ttn.stop();
  await closeDb().catch(() => {});
  http.close(() => process.exit(0));
  setTimeout(() => process.exit(0), 3000).unref();
}

process.on('SIGINT', shutdown);
process.on('SIGTERM', shutdown);

await seedFromDb();
ttn.start();
http.listen(config.port, () => {
  console.log(`dashboard on http://localhost:${config.port}`);
});
