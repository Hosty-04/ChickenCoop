import { createServer } from 'node:http';
import { fileURLToPath } from 'node:url';
import { dirname, join } from 'node:path';
import express from 'express';
import { WebSocketServer } from 'ws';
import { config } from './config.js';
import { TtnBridge, DuplicateCommandError } from './ttn.js';
import { EGGS_MAX } from './codec.js';
import {
  SESSION_COOKIE, SESSION_TTL_MS, readCookie, lockoutRemainingMs, checkCredentials, passwordMatches,
  openSession, closeSession, requestAuthenticated
} from './auth.js';
import {
  writeReading, readHistory, readLatest, readNests, readEggs, countReadings, clearReadings, closeDb
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
  pending: [],
  nestCount: config.nestCount,
  eggsMax: EGGS_MAX,
  nests: null
};

const PUBLIC_PATHS = new Set([
  '/login.html', '/login.js', '/common.js', '/theme.js', '/style.css', '/manifest.webmanifest',
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
      error: `Příliš mnoho pokusů. Zkuste to za ${Math.ceil(waitMs / 60000)} min.`
    });
  }

  if (!checkCredentials(req.ip, req.body?.user, req.body?.password)) {
    return res.status(401).json({ ok: false, error: 'Nesprávné jméno nebo heslo.' });
  }

  res.cookie(SESSION_COOKIE, openSession(), {
    httpOnly: true,
    sameSite: 'lax',
    secure: req.secure,
    maxAge: SESSION_TTL_MS,
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
  if (requestAuthenticated(req)) return next();
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

function refreshNests() {
  try {
    state.nests = readNests(config.ttn.deviceId, config.nestCount);
  } catch (err) {
    console.error('nest state unreadable:', err.message);
  }
}

function hoursFrom(query) {
  return query.hours === 'all' ? null : Math.min(Math.max(Number(query.hours) || 24, 1), 8760);
}

function sendRange(req, res, read) {
  const hours = hoursFrom(req.query);
  try {
    res.json({ hours: hours ?? 'all', ...read(hours) });
  } catch (err) {
    res.status(500).json({ error: err.message });
  }
}

app.get('/api/status', (req, res) => {
  refreshNests();
  res.json(state);
});

app.get('/api/history', (req, res) => {
  sendRange(req, res, (hours) => readHistory(config.ttn.deviceId, hours));
});

app.get('/api/eggs', (req, res) => {
  sendRange(req, res, (hours) => readEggs(config.ttn.deviceId, hours, config.nestCount));
});

app.post('/api/command', async (req, res) => {
  try {
    const sent = await ttn.sendCommand(req.body?.commands ?? [], req.body?.nests);
    broadcast('command', sent);
    res.json({ ok: true, ...sent });
  } catch (err) {
    res.status(err instanceof DuplicateCommandError ? 409 : 400).json({ ok: false, error: err.message });
  }
});

app.post('/api/command/cancel', async (req, res) => {
  try {
    const { cleared } = await ttn.clearQueue();
    broadcast('cancelled', { cleared });
    res.json({ ok: true, cleared });
  } catch (err) {
    res.status(400).json({ ok: false, error: err.message });
  }
});

app.post('/api/data/clear', (req, res) => {
  if (!passwordMatches(req.body?.password)) {
    return res.status(403).json({ ok: false, error: 'Nesprávné heslo.' });
  }

  try {
    const removed = clearReadings(config.ttn.deviceId);
    state.readings = 0;
    state.latest = null;
    state.nests = null;
    state.dbOk = true;
    state.dbError = null;
    broadcast('status', state);
    broadcast('cleared', { removed });
    console.log(`history cleared, ${removed} readings removed`);
    res.json({ ok: true, removed });
  } catch (err) {
    res.status(500).json({ ok: false, error: err.message });
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

ttn.on('downlink', ({ event, commands, nests }) => {
  broadcast('downlink', { event, commands, nests, at: new Date().toISOString() });
  console.log(`downlink ${event}${commands ? ` (${commands.join(', ')}${nests ? ` nests ${nests.join(',')}` : ''})` : ''}`);
});

ttn.on('uplink', (uplink) => {
  state.latest = uplink;

  const { batteryMv, panelMv, door, nests } = uplink.reading;
  const eggs = nests ? ` nests=${nests.map((nest) => (nest.state === 'ok' ? nest.eggs : nest.state)).join(',')}` : '';
  console.log(`uplink fCnt=${uplink.fCnt} battery=${batteryMv ?? '-'} panel=${panelMv ?? '-'} door=${door}${eggs}`);

  try {
    writeReading(uplink.deviceId, uplink.reading, uplink.radio, new Date(uplink.receivedAt));
    state.readings = countReadings(config.ttn.deviceId);
    state.dbOk = true;
    state.dbError = null;
    if (nests) refreshNests();
  } catch (err) {
    state.dbOk = false;
    state.dbError = err.message;
    console.error('database write failed:', err.message);
  }

  broadcast('status', state);
});

function seedFromDb() {
  try {
    const latest = readLatest(config.ttn.deviceId);
    if (latest) state.latest = { deviceId: config.ttn.deviceId, fCnt: null, ...latest };
    state.readings = countReadings(config.ttn.deviceId);
    state.nests = readNests(config.ttn.deviceId, config.nestCount);
    state.dbOk = true;
  } catch (err) {
    state.dbOk = false;
    state.dbError = err.message;
    console.error('database unreadable at startup:', err.message);
  }
}

let stopping = false;

async function shutdown() {
  if (stopping) return;
  stopping = true;
  console.log('shutting down');
  setTimeout(() => process.exit(0), 3000).unref();
  await ttn.stop();
  try {
    closeDb();
  } catch {}
  for (const socket of wss.clients) socket.terminate();
  http.close(() => process.exit(0));
}

process.on('SIGINT', shutdown);
process.on('SIGTERM', shutdown);

seedFromDb();
ttn.start();
http.listen(config.port, () => {
  console.log(`dashboard on http://localhost:${config.port}`);
});
