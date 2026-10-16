import { createServer } from 'node:http';
import { fileURLToPath } from 'node:url';
import { dirname, join } from 'node:path';
import express from 'express';
import { WebSocketServer } from 'ws';
import { config } from './config.js';
import { TtnBridge, QueuedCommandError } from './ttn.js';
import { EGGS_MAX } from './codec.js';
import {
  SESSION_COOKIE, SESSION_TTL_MS, readCookie, lockoutRemainingMs, checkCredentials, passwordMatches,
  openSession, closeSession, requestAuthenticated
} from './auth.js';
import {
  writeReading, readHistory, readLatest, readNests, readEggs, countReadings, clearReadings,
  readSetting, writeSetting, closeDb
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
  timeZone: Intl.DateTimeFormat().resolvedOptions().timeZone,
  automation: null,
  nests: null
};

const AUTOMATION_COMMANDS = { systemOn: true, systemOff: false };
const AUTOMATION_UNKNOWN = { enabled: null, at: null };

let unsavedAutomation = null;
let unsavedSequence = null;

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
    if (err instanceof QueuedCommandError) res.status(409).json({ ok: false, error: err.message, queued: err.queued });
    else res.status(400).json({ ok: false, error: err.message });
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

function storeAutomation(automation, reclaim = false) {
  try {
    writeSetting('automation', automation, reclaim);
    state.automation = automation;
    unsavedAutomation = null;
    return true;
  } catch (err) {
    state.automation = AUTOMATION_UNKNOWN;
    state.dbOk = false;
    state.dbError = err.message;
    unsavedAutomation = automation;
    console.error('automation state not saved:', err.message);
    return false;
  }
}

function storeSequence(sequence, reclaim = false) {
  try {
    writeSetting('commandSequence', sequence, reclaim);
    unsavedSequence = null;
    return true;
  } catch (err) {
    state.dbOk = false;
    state.dbError = err.message;
    unsavedSequence = sequence;
    console.error('command number not saved:', err.message);
    return false;
  }
}

ttn.on('sequence', (sequence) => {
  if (!storeSequence(sequence, true)) broadcast('status', state);
});

function trackAutomation(event, commands, at) {
  const command = commands?.find((name) => Object.hasOwn(AUTOMATION_COMMANDS, name));
  if (event !== 'ack' || !command) return;

  storeAutomation({ enabled: AUTOMATION_COMMANDS[command], at }, true);
  broadcast('status', state);
}

function commandText({ commands, nests }) {
  return `${commands.join(', ')}${nests ? ` nests ${nests.join(',')}` : ''}`;
}

ttn.on('downlink', ({ event, commands, nests }) => {
  const at = new Date().toISOString();
  broadcast('downlink', { event, commands, nests, at });
  console.log(`downlink ${event}${commands ? ` (${commandText({ commands, nests })})` : ''}`);
  trackAutomation(event, commands, at);
});

ttn.on('expired', (entries) => {
  broadcast('expired', entries);
  for (const entry of entries) console.log(`command expired (${commandText(entry)})`);
});

ttn.on('restarted', (entries) => {
  broadcast('restarted', entries);
  for (const entry of entries) console.log(`coop rejoined before confirming (${commandText(entry)})`);
});

ttn.on('superseded', ({ dropped, newer }) => {
  broadcast('superseded', { dropped, newer });
  console.log(`unconfirmed command dropped (${commandText(dropped)}), newer one queued (${commandText(newer)})`);
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
    if (unsavedAutomation) storeAutomation(unsavedAutomation);
    if (unsavedSequence) storeSequence(unsavedSequence);
  } catch (err) {
    state.dbOk = false;
    state.dbError = err.message;
    console.error('database write failed:', err.message);
  }

  broadcast('status', state);
});

function seedFromDb() {
  let automation = AUTOMATION_UNKNOWN;
  try {
    const latest = readLatest(config.ttn.deviceId);
    if (latest) state.latest = { deviceId: config.ttn.deviceId, fCnt: null, ...latest };
    state.readings = countReadings(config.ttn.deviceId);
    state.nests = readNests(config.ttn.deviceId, config.nestCount);
    automation = readSetting('automation');
    ttn.restoreSequence(readSetting('commandSequence'));
    state.dbOk = true;
  } catch (err) {
    state.dbOk = false;
    state.dbError = err.message;
    console.error('database unreadable at startup:', err.message);
  }
  if (!storeAutomation(automation)) unsavedAutomation = AUTOMATION_UNKNOWN;
}

let stopping = false;

async function shutdown() {
  if (stopping) return;
  stopping = true;
  console.log('shutting down');
  setTimeout(() => process.exit(0), 3000).unref();
  await ttn.stop();
  if (unsavedAutomation) storeAutomation(unsavedAutomation, true);
  if (unsavedSequence) storeSequence(unsavedSequence, true);
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
