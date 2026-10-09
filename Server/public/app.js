import { el, api, reason, HttpError } from '/common.js';

const DOOR_LABELS = {
  closed: { text: 'Zavřeno', note: '', tone: '' },
  open: { text: 'Otevřeno', note: '', tone: '' },
  fault: { text: 'Porucha', note: '⚠ čeká na odblokování', tone: 'is-critical' },
  unknown: { text: 'Neznámý', note: '⚠ koncový spínač nehlásí polohu', tone: 'is-warning' }
};

const NEST_LABELS = {
  broody: { note: '⚠ sedí kvočna', tone: 'is-warning' },
  uncalibrated: { note: '⚠ váha není zkalibrovaná', tone: 'is-warning' },
  fault: { note: '⚠ porucha váhy', tone: 'is-critical' },
  offline: { note: '⚠ hnízdo neodpovídá', tone: 'is-warning' }
};

const COMMAND_LABELS = {
  systemOn: 'zapnout automatiku',
  systemOff: 'vypnout automatiku',
  doorOpen: 'otevřít dvířka',
  doorClose: 'zavřít dvířka',
  block: 'zablokovat',
  unblock: 'odblokovat'
};

const NEST_COMMAND_LABELS = {
  tare: 'vynulovat váhu',
  calibrate: 'zkalibrovat váhu'
};

const DOWNLINK_EVENTS = {
  sent: { text: 'Příkaz odeslán do kurníku', tone: 'is-ok' },
  ack: { text: 'Kurník příkaz potvrdil', tone: 'is-ok' },
  nack: { text: 'Kurník příkaz odmítl', tone: 'is-error' },
  failed: { text: 'Příkaz se nepodařilo doručit', tone: 'is-error' }
};

const MINUTE_MS = 60 * 1000;
const HOUR_MS = 60 * MINUTE_MS;
const DAY_MS = 24 * HOUR_MS;
const WEEK_MS = 7 * DAY_MS;

const BUCKET_NAMES = {
  [10 * MINUTE_MS]: '10 minut',
  [HOUR_MS]: 'hodinu',
  [6 * HOUR_MS]: '6 hodin',
  [DAY_MS]: 'den',
  [WEEK_MS]: 'týden'
};

const TOAST_MS = 5000;
const OFFLINE_MS = 5000;
const STATUS_POLL_MS = 60 * 1000;
const RETRY_MS = 1000;
const RETRY_MAX_MS = 30 * 1000;
const POINT_RADIUS = 4;
const POINT_GAP_PX = 14;
const TICK_LENGTH_PX = 6;
const TIME_LABEL_PX = 60;
const DATE_LABEL_PX = 115;
const MAX_TICKS = 8;
const MIN_TICK_GAP = 0.75;
const MAX_JOIN_BUCKETS = 2.5;
const NOON_HOUR = 12;

const VOLTS = new Intl.NumberFormat('cs-CZ', { minimumFractionDigits: 2, maximumFractionDigits: 2 });
const NUMBER = new Intl.NumberFormat('cs-CZ');

const darkScheme = matchMedia('(prefers-color-scheme: dark)');
const rangeButtons = [...document.querySelectorAll('.filterbar button')];
const commandButtons = [...document.querySelectorAll('.commands button[data-command]')];
const nestButtons = [...document.querySelectorAll('.commands button[data-nest-command]')];
const toastTimers = new Map();
const busy = new Set();
const MESSAGES = new Map([
  ['status', renderStatus],
  ['pending', renderPending],
  ['command', (entry) => showToast(queuedText(entry), 'is-ok')],
  ['cancelled', ({ cleared }) => showToast(cancelledText(cleared), 'is-ok')],
  ['cleared', ({ removed }) => showToast(clearedText(removed), 'is-ok', 'data-toast')],
  ['downlink', ({ event, commands, nests }) => {
    const info = DOWNLINK_EVENTS[event];
    showToast(commands ? `${info.text}: ${describe(commands, nests)}` : info.text, info.tone);
  }]
]);

let range = 24;
let power = { range: null, bucketMs: 0, points: [] };
let powerWindow = null;
let eggs = { range: null, bucketMs: 0, points: [], edges: [], since: 0 };
let powerChart = null;
let eggChart = null;
let palette = readPalette();
let timeZone;
let formats = buildFormats();
let historySeq = 0;
let eggsSeq = 0;
let shownUplink;
let shownCheck;
let nestCount = 0;
let chosenNests = [];
let socket = null;
let retryMs = RETRY_MS;
let retryTimer = null;
let offlineTimer = null;
let serverDown = false;
let leaving = false;

function readPalette() {
  const style = getComputedStyle(document.documentElement);
  const token = (name) => style.getPropertyValue(name).trim();
  return {
    battery: token('--series-1'),
    panel: token('--series-2'),
    eggs: token('--series-4'),
    surface: token('--surface-1'),
    text: token('--text-muted'),
    grid: token('--grid'),
    axis: token('--axis')
  };
}

function buildFormats(zone) {
  const format = (options) => new Intl.DateTimeFormat('cs-CZ', { ...options, timeZone: zone });
  return {
    time: format({ hour: '2-digit', minute: '2-digit' }),
    day: format({ day: 'numeric', month: 'numeric' }),
    date: format({ day: 'numeric', month: 'numeric', year: 'numeric' }),
    weekday: format({ weekday: 'long', day: 'numeric', month: 'numeric', year: 'numeric' }),
    clock: format({ year: 'numeric', month: 'numeric', day: 'numeric', hour: 'numeric', minute: 'numeric', second: 'numeric', hourCycle: 'h23' })
  };
}

function zoneOffset(ms) {
  const part = Object.fromEntries(formats.clock.formatToParts(ms).map(({ type, value }) => [type, Number(value)]));
  return Date.UTC(part.year, part.month - 1, part.day, part.hour, part.minute, part.second) - Math.floor(ms / 1000) * 1000;
}

function plural(n, one, few, many) {
  return `${NUMBER.format(n)} ${n === 1 ? one : n >= 2 && n <= 4 ? few : many}`;
}

function countEggs(n) {
  return plural(n, 'vejce', 'vejce', 'vajec');
}

function countCommands(n) {
  return plural(n, 'příkaz', 'příkazy', 'příkazů');
}

function countRecords(n) {
  return plural(n, 'záznam', 'záznamy', 'záznamů');
}

function joinList(items) {
  return items.length === 1 ? String(items[0]) : `${items.slice(0, -1).join(', ')} a ${items.at(-1)}`;
}

function nestList(nests) {
  return `${nests.length === 1 ? 'hnízda' : 'hnízd'} ${joinList(nests)}`;
}

function nestNames(nests) {
  return `${nests.length === 1 ? 'hnízdo' : 'hnízda'} ${joinList(nests)}`;
}

function describe(commands, nests) {
  return commands.map((name) => (Object.hasOwn(NEST_COMMAND_LABELS, name)
    ? `${NEST_COMMAND_LABELS[name]} ${nestList(nests)}`
    : COMMAND_LABELS[name])).join(' + ');
}

function laidToday(n) {
  return n > 0 ? `dnes +${countEggs(n)}` : 'dnes zatím nic';
}

function queuedText(entry) {
  return `Zařazeno do fronty: ${describe(entry.commands, entry.nests)} — čeká na další zprávu z kurníku`;
}

function cancelledText(cleared) {
  return cleared > 0 ? `Zrušeno: ${countCommands(cleared)}` : 'Fronta je prázdná';
}

function clearedText(removed) {
  return removed > 0 ? `Historie smazána: ${countRecords(removed)}` : 'Nebylo co mazat';
}

function toVolts(mv) {
  return mv == null ? null : mv / 1000;
}

function formatVolts(volts) {
  return volts == null ? '–' : `${VOLTS.format(volts)} V`;
}

function formatEggs(n) {
  return n == null ? '–' : String(n);
}

function formatTime(value, mode) {
  const d = new Date(value);
  if (mode === 'date') return formats.date.format(d);
  if (mode === 'datetime') return `${formats.day.format(d)} ${formats.time.format(d)}`;
  return formats.time.format(d);
}

function formatAgo(iso) {
  const seconds = Math.max(0, Math.round((Date.now() - Date.parse(iso)) / 1000));
  if (seconds < 60) return 'před chvílí';
  const minutes = Math.round(seconds / 60);
  if (minutes < 60) return `před ${minutes} min`;
  const hours = Math.round(minutes / 60);
  return hours < 48 ? `před ${hours} h` : `před ${Math.round(hours / 24)} dny`;
}

function sameDay(a, b) {
  return formats.date.format(new Date(a)) === formats.date.format(new Date(b));
}

function labelMode() {
  if (power.bucketMs >= DAY_MS) return 'date';
  return power.bucketMs >= HOUR_MS ? 'datetime' : 'time';
}

function tickLimit(wrapId, wide) {
  const room = el(wrapId).clientWidth / (wide ? DATE_LABEL_PX : TIME_LABEL_PX);
  return Math.max(2, Math.min(MAX_TICKS, Math.floor(room)));
}

function snapUnit(spanMs, bucketMs) {
  if (spanMs >= 2 * DAY_MS && bucketMs < DAY_MS) return DAY_MS;
  if (spanMs >= 2 * HOUR_MS && bucketMs < HOUR_MS) return HOUR_MS;
  return 0;
}

function snapTarget(ideal, unit) {
  const at = unit === DAY_MS ? ideal : ideal + HOUR_MS / 2;
  const offset = zoneOffset(at);
  const wall = unit === DAY_MS
    ? Math.floor((at + offset) / DAY_MS) * DAY_MS + NOON_HOUR * HOUR_MS
    : Math.floor((at + offset) / HOUR_MS) * HOUR_MS;
  return wall - zoneOffset(wall - offset);
}

function powerTicks() {
  const { from: first, to: last } = powerWindow;
  const limit = tickLimit('plot-wrap', labelMode() !== 'time');
  const spanMs = last - first;
  if (spanMs < MINUTE_MS) return [first];

  const unit = snapUnit(spanMs, power.bucketMs);
  const room = (spanMs / (limit - 1)) * MIN_TICK_GAP;
  const picked = [first];
  const add = (at) => {
    if (at - picked.at(-1) >= room && last - at >= room) picked.push(at);
  };

  if (unit) {
    const strideMs = Math.ceil(spanMs / (limit - 1) / unit) * unit;
    for (let at = first + strideMs; at < last; at += strideMs) add(snapTarget(at, unit));
  } else {
    const step = Math.max(spanMs / (limit - 1), MINUTE_MS);
    for (let at = first + step; last - at >= MINUTE_MS; at += step) add(Math.round(at));
  }

  picked.push(last);
  return picked;
}

function eggTicks() {
  const { edges } = eggs;
  const intervals = edges.length - 1;
  const stride = Math.max(1, Math.ceil(intervals / (tickLimit('eggs-plot-wrap', eggs.bucketMs >= WEEK_MS) - 1)));
  const picked = [];

  for (let i = 0; i < edges.length; i += stride) picked.push(edges[i]);
  if (intervals % stride !== 0) {
    if (picked.length > 1 && intervals % stride < stride * MIN_TICK_GAP) picked.pop();
    picked.push(edges[intervals]);
  }
  return picked;
}

function eggTick(ms) {
  if (eggs.bucketMs < DAY_MS) return formats.time.format(ms);
  return (eggs.bucketMs < WEEK_MS ? formats.day : formats.date).format(ms);
}

function eggPeriod(index) {
  const from = eggs.edges[index];
  if (eggs.bucketMs < DAY_MS) {
    return `${formats.day.format(from)} ${formats.time.format(from)}–${formats.time.format(eggs.edges[index + 1])}`;
  }
  if (eggs.bucketMs < WEEK_MS) return formats.weekday.format(from);
  return `týden od ${formats.date.format(from)}`;
}

function pointRadius(ctx) {
  const area = ctx.chart.chartArea;
  if (!area) return POINT_RADIUS;
  const { data } = ctx.dataset;
  if (data[ctx.dataIndex - 1]?.y == null && data[ctx.dataIndex + 1]?.y == null) return POINT_RADIUS;
  const spacing = (area.width * power.bucketMs) / (powerWindow.to - powerWindow.from);
  return spacing >= POINT_GAP_PX ? POINT_RADIUS : 0;
}

function series(times, value) {
  const limit = power.bucketMs * MAX_JOIN_BUCKETS;
  const out = [];

  for (let i = 0; i < times.length; i++) {
    if (i > 0 && times[i] - times[i - 1] > limit) out.push({ x: (times[i - 1] + times[i]) / 2, y: null });
    out.push({ x: times[i], y: value(power.points[i]) });
  }
  return out;
}

function paintScales(scales) {
  for (const scale of Object.values(scales)) {
    scale.grid.color = palette.grid;
    scale.grid.tickColor = palette.text;
    scale.border.color = palette.axis;
    scale.ticks.color = palette.text;
  }
}

function paintPower(target) {
  const [battery, panel] = target.data.datasets;
  Object.assign(battery, { borderColor: palette.battery, backgroundColor: palette.battery, pointBorderColor: palette.surface });
  Object.assign(panel, { borderColor: palette.panel, backgroundColor: palette.panel, pointBorderColor: palette.surface });
  paintScales(target.options.scales);
  return target;
}

function paintEggs(target) {
  target.data.datasets[0].backgroundColor = palette.eggs;
  paintScales(target.options.scales);
  return target;
}

function repaint() {
  palette = readPalette();
  if (powerChart) paintPower(powerChart).update();
  if (eggChart) paintEggs(eggChart).update();
}

function timeAxis(ticks, callback) {
  return {
    type: 'linear',
    afterBuildTicks: (scale) => {
      scale.ticks = ticks().map((value) => ({ value }));
    },
    grid: { tickLength: TICK_LENGTH_PX },
    border: {},
    ticks: { autoSkip: false, maxRotation: 0, align: 'inner', callback }
  };
}

function valueAxis(ticks) {
  return { grid: { tickLength: TICK_LENGTH_PX }, border: {}, ticks };
}

function drawPower() {
  const times = power.points.map((p) => Date.parse(p.time));
  const now = Date.now();
  powerWindow = { from: power.range === 'all' ? Math.min(times[0], now) : now - power.range * HOUR_MS, to: now };
  const battery = series(times, (p) => toVolts(p.batteryMv));
  const panel = series(times, (p) => toVolts(p.panelMv));

  if (powerChart) {
    powerChart.data.datasets[0].data = battery;
    powerChart.data.datasets[1].data = panel;
    powerChart.options.scales.x.min = powerWindow.from;
    powerChart.options.scales.x.max = powerWindow.to;
    powerChart.update();
    return;
  }

  const line = { borderWidth: 2, pointRadius, pointHoverRadius: 6, pointBorderWidth: 2 };
  powerChart = new Chart(el('chart'), paintPower({
    type: 'line',
    data: {
      datasets: [
        { ...line, label: 'Baterie', data: battery },
        { ...line, label: 'Panel', data: panel }
      ]
    },
    options: {
      responsive: true,
      maintainAspectRatio: false,
      animation: false,
      interaction: { mode: 'index', intersect: false },
      plugins: {
        legend: { display: false },
        tooltip: {
          callbacks: {
            title: (items) => formatTime(items[0].parsed.x, labelMode()),
            label: (ctx) => `${ctx.dataset.label}: ${formatVolts(ctx.parsed.y)}`
          }
        }
      },
      scales: {
        x: { ...timeAxis(powerTicks, (value) => formatTime(value, labelMode())), min: powerWindow.from, max: powerWindow.to },
        y: valueAxis({ callback: (value) => `${NUMBER.format(value)} V` })
      }
    }
  }));
}

function drawEggs() {
  const { edges } = eggs;
  const data = eggs.points.map((p, i) => ({ x: (edges[i] + edges[i + 1]) / 2, y: p.total }));

  if (eggChart) {
    eggChart.data.datasets[0].data = data;
    eggChart.options.scales.x.min = edges[0];
    eggChart.options.scales.x.max = edges.at(-1);
    eggChart.update();
    return;
  }

  eggChart = new Chart(el('eggs-chart'), paintEggs({
    type: 'bar',
    data: {
      datasets: [{
        label: 'Snesená vejce',
        data,
        borderRadius: 4,
        borderSkipped: 'start',
        maxBarThickness: 24,
        categoryPercentage: 0.8,
        barPercentage: 0.9
      }]
    },
    options: {
      responsive: true,
      maintainAspectRatio: false,
      animation: false,
      interaction: { mode: 'nearest', axis: 'x', intersect: false },
      plugins: {
        legend: { display: false },
        tooltip: {
          displayColors: false,
          callbacks: {
            title: (items) => eggPeriod(items[0].dataIndex),
            label: (ctx) => {
              const point = eggs.points[ctx.dataIndex];
              if (point.total === null) return 'Bez dat';
              const missing = point.laid.flatMap((n, i) => (n === null ? [i + 1] : []));
              return [
                `Celkem: ${countEggs(point.total)}`,
                ...point.laid.flatMap((n, i) => (n > 0 ? [`Hnízdo ${i + 1}: ${n}`] : [])),
                ...(missing.length > 0 ? [`Bez dat: ${nestNames(missing)}`] : [])
              ];
            }
          }
        }
      },
      scales: {
        x: { ...timeAxis(eggTicks, eggTick), offset: false, min: edges[0], max: edges.at(-1), grid: { tickLength: TICK_LENGTH_PX, offset: false } },
        y: { ...valueAxis({ precision: 0 }), beginAtZero: true }
      }
    }
  }));
}

function renderPowerTable() {
  const mode = labelMode();
  el('table').tBodies[0].innerHTML = power.points.map((p) =>
    `<tr><td>${formatTime(p.time, mode)}</td><td>${formatVolts(toVolts(p.batteryMv))}</td><td>${formatVolts(toVolts(p.panelMv))}</td></tr>`
  ).reverse().join('');
}

function renderEggTable() {
  const nests = eggs.points[0]?.laid.length ?? nestCount;
  el('eggs-table').tHead.innerHTML = `<tr><th scope="col">Čas</th><th scope="col">Celkem</th>${
    Array.from({ length: nests }, (_, i) => `<th scope="col">Hnízdo ${i + 1}</th>`).join('')
  }</tr>`;
  el('eggs-table').tBodies[0].innerHTML = eggs.points.flatMap((p, i) => (eggs.edges[i + 1] <= eggs.since ? [] : [
    `<tr><td>${eggPeriod(i)}</td><td>${formatEggs(p.total)}</td>${p.laid.map((n) => `<td>${formatEggs(n)}</td>`).join('')}</tr>`
  ])).reverse().join('');
}

function renderPower() {
  const empty = power.points.length === 0;
  const tableShown = !el('table-wrap').hidden;
  el('chart-empty').hidden = !empty || power.range === null;
  el('plot-wrap').hidden = empty || tableShown;
  el('legend').hidden = empty || tableShown;
  el('chart-hint').hidden = empty || labelMode() === 'time';

  if (empty) {
    powerChart?.destroy();
    powerChart = null;
  }
  if (tableShown) renderPowerTable();
  else if (!empty) drawPower();
}

function renderEggs() {
  const empty = eggs.points.every((p) => p.total === null);
  const tableShown = !el('eggs-table-wrap').hidden;
  el('eggs-empty').hidden = !empty || eggs.range === null;
  el('eggs-plot-wrap').hidden = empty || tableShown;

  if (empty) {
    eggChart?.destroy();
    eggChart = null;
  }
  if (tableShown) renderEggTable();
  else if (!empty) drawEggs();
}

function redirected(err) {
  if (!leaving && err instanceof HttpError && err.status === 401) {
    leaving = true;
    location.replace('/login.html');
  }
  return leaving;
}

async function loadHistory() {
  const seq = ++historySeq;
  const requested = range;
  el('chart-sub').textContent = 'načítám…';
  try {
    const body = await api(`/api/history?hours=${requested}`);
    if (seq !== historySeq) return;
    power = { range: requested, bucketMs: body.bucketMs, points: body.points };
    el('chart-sub').textContent = power.points.length === 0
      ? 'baterie a solární panel'
      : `baterie a solární panel · průměr za ${BUCKET_NAMES[power.bucketMs]}`;
    hideToast('chart-toast', 'history');
  } catch (err) {
    if (redirected(err) || seq !== historySeq) return;
    if (power.range !== requested) power = { range: null, bucketMs: 0, points: [] };
    shownUplink = undefined;
    el('chart-sub').textContent = 'historii se nepodařilo načíst';
    showToast(`Historii se nepodařilo načíst: ${reason(err)}`, 'is-error', 'chart-toast', 'history');
  }
  renderPower();
}

async function loadEggs() {
  const seq = ++eggsSeq;
  const requested = range;
  el('eggs-sub').textContent = 'načítám…';
  try {
    const body = await api(`/api/eggs?hours=${requested}`);
    if (seq !== eggsSeq) return;
    eggs = {
      range: requested,
      bucketMs: body.bucketMs,
      points: body.points,
      edges: body.end ? [...body.points.map((p) => Date.parse(p.time)), Date.parse(body.end)] : [],
      since: body.since ? Date.parse(body.since) : 0
    };
    el('eggs-sub').textContent = `histogram snesených vajec · celkem ${countEggs(eggs.points.reduce((sum, p) => sum + (p.total ?? 0), 0))}`;
    hideToast('eggs-toast', 'eggs');
  } catch (err) {
    if (redirected(err) || seq !== eggsSeq) return;
    if (eggs.range !== requested) eggs = { range: null, bucketMs: 0, points: [], edges: [], since: 0 };
    shownCheck = undefined;
    el('eggs-sub').textContent = 'historii snášky se nepodařilo načíst';
    showToast(`Historii snášky se nepodařilo načíst: ${reason(err)}`, 'is-error', 'eggs-toast', 'eggs');
  }
  renderEggs();
}

async function loadStatus() {
  try {
    renderStatus(await api('/api/status'));
  } catch (err) {
    if (!redirected(err)) serverLost();
  }
}

function showToast(text, tone = '', id = 'toast', topic = '') {
  const toast = el(id);
  toast.textContent = text;
  toast.className = tone ? `toast ${tone}` : 'toast';
  toast.dataset.topic = topic;
  toast.hidden = false;

  clearTimeout(toastTimers.get(id));
  if (tone !== 'is-error') toastTimers.set(id, setTimeout(() => { toast.hidden = true; }, TOAST_MS));
}

function hideToast(id, topic) {
  const toast = el(id);
  if (toast.dataset.topic === topic) toast.hidden = true;
}

function setBadge(up, text) {
  const badge = el('link');
  badge.classList.toggle('is-up', up === true);
  badge.classList.toggle('is-down', up === false);
  el('link-text').textContent = text;
}

function serverLost() {
  if (leaving || serverDown || offlineTimer) return;
  setBadge(null, 'připojuji…');
  offlineTimer = setTimeout(() => {
    offlineTimer = null;
    if (leaving) return;
    serverDown = true;
    setBadge(false, 'Server nedostupný');
  }, OFFLINE_MS);
}

function setTile(id, value, note = '', tone = '') {
  el(`${id}-value`).textContent = value;
  const target = el(`${id}-note`);
  target.textContent = note;
  target.className = tone ? `tile-note ${tone}` : 'tile-note';
}

function batteryNote({ batteryMv, batteryCritical, batterySaturated }) {
  if (batteryCritical) return ['⚠ kriticky vybitá', 'is-critical'];
  if (batteryMv === null) return ['⚠ čidlo neodpovídá', 'is-warning'];
  return [batterySaturated ? 'na horní mezi rozsahu' : ''];
}

function panelNote({ panelMv, panelSaturated }) {
  if (panelMv === null) return ['⚠ čidlo neodpovídá', 'is-warning'];
  return [panelSaturated ? 'na horní mezi rozsahu' : ''];
}

function renderLatest(uplink) {
  if (!uplink) {
    for (const id of ['battery', 'panel', 'door', 'seen']) setTile(id, '–');
    return;
  }

  const { reading, radio, receivedAt } = uplink;
  const door = DOOR_LABELS[reading.door];
  const ago = formatAgo(receivedAt);
  setTile('battery', formatVolts(toVolts(reading.batteryMv)), ...batteryNote(reading));
  setTile('panel', formatVolts(toVolts(reading.panelMv)), ...panelNote(reading));
  setTile('door', door.text, door.note, door.tone);
  setTile('seen', formatTime(receivedAt, 'time'), radio.rssi == null ? ago : `${ago} (${radio.rssi} dBm)`);
}

function renderAutomation(automation) {
  if (automation === null) {
    setTile('automation', '–', 'zatím bez příkazu');
    return;
  }

  const since = `od ${formatTime(automation.at, sameDay(automation.at, Date.now()) ? 'time' : 'datetime')}`;
  if (automation.enabled) setTile('automation', 'Zapnutá', since);
  else setTile('automation', 'Vypnutá', `⚠ ${since}`, 'is-warning');
}

function renderPending(pending) {
  el('queue-value').textContent = pending.length === 0
    ? 'nic nečeká'
    : pending.map((entry) => describe(entry.commands, entry.nests)).join(' · ');
  el('queue').classList.toggle('is-waiting', pending.length > 0);
}

function renderCount(readings) {
  el('data-count').textContent = countRecords(readings);
  el('wipe').disabled = readings === 0;
}

function nestBoxes() {
  return [...el('nest-picks').querySelectorAll('input[value]')];
}

function tickedNests() {
  return nestBoxes().filter((input) => input.checked).map((input) => Number(input.value));
}

function pickedText(picked) {
  if (picked.length === 0) return 'Vyberte hnízda';
  if (picked.length === 1) return `Hnízdo ${picked[0]}`;
  if (picked.length === nestCount) return 'Všechna hnízda';
  if (picked.length > nestCount / 2) {
    const left = Array.from({ length: nestCount }, (_, i) => i + 1).filter((nest) => !picked.includes(nest));
    return `Všechna kromě ${left.join(', ')}`;
  }
  return `Hnízda ${picked.join(', ')}`;
}

function renderTicks() {
  const all = el('nest-all');
  if (!all) return;
  const ticked = tickedNests().length;
  all.checked = ticked === nestCount;
  all.indeterminate = ticked > 0 && ticked < nestCount;
}

function renderPicks() {
  el('nest-picker-text').textContent = pickedText(chosenNests);
  for (const button of nestButtons) button.disabled = busy.has(button) || chosenNests.length === 0;
}

function buildNests(count, eggsMax) {
  el('nests').innerHTML = Array.from({ length: count }, (_, i) => `
    <div class="nest">
      <span class="tile-label">Hnízdo ${i + 1}</span>
      <span class="tile-value" id="nest-${i}-value">–</span>
      <span class="tray" id="nest-${i}-tray" aria-hidden="true">${'<span class="egg"></span>'.repeat(eggsMax)}</span>
      <span class="tile-note" id="nest-${i}-note"></span>
    </div>`).join('');

  el('nest-picks').innerHTML = (count > 1
    ? '<label class="picker-item picker-all"><input type="checkbox" id="nest-all">Všechna hnízda</label>'
    : '') + Array.from({ length: count }, (_, i) =>
    `<label class="picker-item"><input type="checkbox" value="${i + 1}">Hnízdo ${i + 1}</label>`
  ).join('');
  chosenNests = [];
  renderPicks();
}

function renderNests({ nestCount: count, eggsMax, nests: snapshot }) {
  if (count !== nestCount) {
    nestCount = count;
    buildNests(count, eggsMax);
  }

  const today = snapshot !== null && sameDay(snapshot.checkedAt, Date.now());
  if (snapshot === null) el('nests-sub').textContent = 'zatím žádná kontrola';
  else if (today) el('nests-sub').textContent = `poslední kontrola ${formatTime(snapshot.checkedAt, 'time')} · ${laidToday(snapshot.laidToday)}`;
  else el('nests-sub').textContent = `poslední kontrola ${formatTime(snapshot.checkedAt, 'datetime')}`;

  for (let i = 0; i < count; i++) {
    const nest = snapshot?.nests[i] ?? { state: null, eggs: null, laidToday: 0 };
    const known = nest.eggs !== null && nest.state !== 'uncalibrated';
    const label = NEST_LABELS[nest.state];
    const tray = el(`nest-${i}-tray`);

    tray.classList.toggle('is-stale', nest.state !== 'ok');
    tray.querySelectorAll('.egg').forEach((egg, slot) => egg.classList.toggle('is-laid', known && slot < nest.eggs));

    if (label) setTile(`nest-${i}`, known ? countEggs(nest.eggs) : '–', label.note, label.tone);
    else if (nest.state === 'ok' && nest.eggs >= eggsMax) setTile(`nest-${i}`, countEggs(nest.eggs), '⚠ košík je plný', 'is-warning');
    else setTile(`nest-${i}`, known ? countEggs(nest.eggs) : '–', nest.state === 'ok' && today ? laidToday(nest.laidToday) : '');
  }
}

function syncCharts(status) {
  const uplinkAt = status.latest?.receivedAt ?? null;
  const checkedAt = status.nests?.checkedAt ?? null;
  if (uplinkAt !== shownUplink) {
    shownUplink = uplinkAt;
    loadHistory();
  }
  if (checkedAt !== shownCheck) {
    shownCheck = checkedAt;
    loadEggs();
  }
}

function syncTimeZone(zone) {
  if (zone === timeZone) return;
  timeZone = zone;
  formats = buildFormats(zone);
  renderPower();
  renderEggs();
}

function renderStatus(status) {
  clearTimeout(offlineTimer);
  offlineTimer = null;
  serverDown = false;
  syncTimeZone(status.timeZone);
  el('device').textContent = status.device;
  setBadge(status.ttnConnected, status.ttnConnected ? 'TTN připojeno' : 'TTN odpojeno');
  renderPending(status.pending);
  renderCount(status.readings);
  renderLatest(status.latest);
  renderAutomation(status.automation);
  renderNests(status);

  if (status.dbOk === false) {
    showToast(`Databáze hlásí chybu (${status.dbError}), měření se nemusí ukládat.`, 'is-error', 'data-toast', 'database');
  } else {
    hideToast('data-toast', 'database');
  }
  syncCharts(status);
}

function connectSocket() {
  if (socket && socket.readyState <= WebSocket.OPEN) return;

  socket = new WebSocket(`${location.protocol === 'https:' ? 'wss' : 'ws'}://${location.host}/ws`);
  socket.addEventListener('open', () => {
    retryMs = RETRY_MS;
  });
  socket.addEventListener('message', (event) => {
    const { type, data } = JSON.parse(event.data);
    MESSAGES.get(type)?.(data);
  });
  socket.addEventListener('close', () => {
    if (leaving) return;
    serverLost();
    loadStatus();
    clearTimeout(retryTimer);
    retryTimer = setTimeout(connectSocket, retryMs);
    retryMs = Math.min(retryMs * 2, RETRY_MAX_MS);
  });
}

function refresh() {
  shownUplink = undefined;
  shownCheck = undefined;
  connectSocket();
  loadStatus();
}

async function sendCommand(button, command) {
  busy.add(button);
  button.disabled = true;
  try {
    showToast(queuedText(await api('/api/command', command)), 'is-ok');
    loadStatus();
  } catch (err) {
    if (redirected(err)) return;
    if (err instanceof HttpError && err.status === 409) showToast('Stejný příkaz už ve frontě čeká, podruhé se nezařadil.');
    else showToast(`Příkaz selhal: ${reason(err)}`, 'is-error');
  } finally {
    busy.delete(button);
    button.disabled = false;
    renderPicks();
  }
}

function setWipeBusy(state) {
  el('wipe-confirm').disabled = state;
  el('wipe-cancel').disabled = state;
}

function bindToggle(buttonId, wrapId, render) {
  el(buttonId).addEventListener('click', (event) => {
    const wrap = el(wrapId);
    wrap.hidden = !wrap.hidden;
    event.currentTarget.textContent = wrap.hidden ? 'Tabulka' : 'Graf';
    render();
  });
}

for (const button of rangeButtons) {
  button.addEventListener('click', () => {
    for (const other of rangeButtons) other.setAttribute('aria-pressed', String(other === button));
    range = button.dataset.hours === 'all' ? 'all' : Number(button.dataset.hours);
    loadHistory();
    loadEggs();
  });
}

for (const button of commandButtons) {
  button.addEventListener('click', () => sendCommand(button, { commands: [button.dataset.command] }));
}

for (const button of nestButtons) {
  button.addEventListener('click', () => sendCommand(button, { commands: [button.dataset.nestCommand], nests: chosenNests }));
}

bindToggle('toggle-table', 'table-wrap', renderPower);
bindToggle('toggle-eggs-table', 'eggs-table-wrap', renderEggs);

el('cancel').addEventListener('click', async (event) => {
  const button = event.currentTarget;
  button.disabled = true;
  try {
    const { cleared } = await api('/api/command/cancel', {});
    showToast(cancelledText(cleared), 'is-ok');
    loadStatus();
  } catch (err) {
    if (!redirected(err)) showToast(`Zrušení selhalo: ${reason(err)}`, 'is-error');
  } finally {
    button.disabled = false;
  }
});

el('nest-picker').addEventListener('click', () => {
  for (const input of nestBoxes()) input.checked = chosenNests.includes(Number(input.value));
  renderTicks();
  el('nest-dialog').showModal();
});

el('nest-picks').addEventListener('change', (event) => {
  if (event.target.id === 'nest-all') for (const input of nestBoxes()) input.checked = event.target.checked;
  renderTicks();
});

el('nest-form').addEventListener('submit', (event) => {
  event.preventDefault();
  chosenNests = tickedNests();
  renderPicks();
  el('nest-dialog').close();
});

el('nest-cancel').addEventListener('click', () => el('nest-dialog').close());

el('wipe').addEventListener('click', () => {
  el('wipe-form').reset();
  el('wipe-error').hidden = true;
  el('wipe-dialog').showModal();
});

el('wipe-form').addEventListener('submit', async (event) => {
  event.preventDefault();
  el('wipe-error').hidden = true;
  setWipeBusy(true);
  try {
    const { removed } = await api('/api/data/clear', { password: el('wipe-password').value });
    el('wipe-dialog').close();
    showToast(clearedText(removed), 'is-ok', 'data-toast');
    loadStatus();
  } catch (err) {
    if (redirected(err)) return;
    el('wipe-error').textContent = `Mazání selhalo: ${reason(err)}`;
    el('wipe-error').hidden = false;
  } finally {
    setWipeBusy(false);
  }
});

el('wipe-cancel').addEventListener('click', () => el('wipe-dialog').close());

el('wipe-dialog').addEventListener('cancel', (event) => {
  if (el('wipe-confirm').disabled) event.preventDefault();
});

el('logout').addEventListener('click', async (event) => {
  const button = event.currentTarget;
  button.disabled = true;
  try {
    await api('/api/logout', {});
    leaving = true;
    location.replace('/login.html');
  } catch (err) {
    button.disabled = false;
    showToast(`Odhlášení selhalo: ${reason(err)}`, 'is-error');
  }
});

el('theme').addEventListener('click', () => {
  const current = document.documentElement.dataset.theme ?? (darkScheme.matches ? 'dark' : 'light');
  const next = current === 'dark' ? 'light' : 'dark';
  applyTheme(next);
  try {
    localStorage.setItem('theme', next);
  } catch {}
  repaint();
});

darkScheme.addEventListener('change', () => {
  if (!document.documentElement.dataset.theme) repaint();
});

window.addEventListener('pagehide', () => {
  leaving = true;
});

window.addEventListener('pageshow', (event) => {
  leaving = false;
  if (event.persisted) refresh();
});

refresh();
setInterval(loadStatus, STATUS_POLL_MS);
