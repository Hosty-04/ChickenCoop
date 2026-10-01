const DOOR_LABELS = {
  closed: { text: 'Zavřeno', note: '', tone: '' },
  open: { text: 'Otevřeno', note: '', tone: '' },
  fault: { text: 'Porucha', note: '⚠ čeká na odblokování', tone: 'is-critical' },
  unknown: { text: 'Neznámý', note: '⚠ koncový spínač nehlásí polohu', tone: 'is-warning' }
};

const COMMAND_LABELS = {
  systemOn: 'zapnout automatiku',
  systemOff: 'vypnout automatiku',
  doorOpen: 'otevřít dvířka',
  doorClose: 'zavřít dvířka',
  block: 'zablokovat',
  unblock: 'odblokovat'
};

const DOWNLINK_EVENTS = {
  sent: { text: 'Kurník příkaz přijal', tone: 'is-ok' },
  ack: { text: 'Kurník příkaz potvrdil', tone: 'is-ok' },
  nack: { text: 'Kurník příkaz odmítl', tone: 'is-error' },
  failed: { text: 'Příkaz se nepodařilo doručit', tone: 'is-error' }
};

const el = (id) => document.getElementById(id);
const css = (name) => getComputedStyle(document.querySelector('.viz-root')).getPropertyValue(name).trim();

const TOAST_MS = 5000;
const REQUEST_MS = 8000;
const POINT_RADIUS = 4;
const POINT_GAP_PX = 14;
const TICK_LENGTH_PX = 6;
const TIME_LABEL_PX = 60;
const DATE_LABEL_PX = 100;
const MAX_TICKS = 8;
const MIN_TICK_GAP = 0.75;
const NOON_HOUR = 12;
const HOUR_MS = 60 * 60 * 1000;
const DAY_MS = 24 * HOUR_MS;

let chart = null;
let hours = 24;
let points = [];
let shownTicks = null;
const toastTimers = new Map();

function tickLimit() {
  const room = el('plot-wrap').clientWidth / (labelMode() === 'time' ? TIME_LABEL_PX : DATE_LABEL_PX);
  return Math.max(2, Math.min(MAX_TICKS, Math.floor(room)));
}

function snapUnit(times) {
  let bucketMs = Infinity;
  for (let i = 1; i < times.length; i++) bucketMs = Math.min(bucketMs, times[i] - times[i - 1]);

  const spanMs = times[times.length - 1] - times[0];
  if (spanMs >= 2 * DAY_MS && bucketMs < DAY_MS) return DAY_MS;
  if (spanMs >= 2 * HOUR_MS && bucketMs < HOUR_MS) return HOUR_MS;
  return 0;
}

function snapTarget(ideal, unit) {
  const target = new Date(ideal);

  if (unit === DAY_MS) target.setHours(NOON_HOUR, 0, 0, 0);
  else target.setMinutes(0, 0, 0);
  return target.getTime();
}

function nearestTick(times, target) {
  let best = 0;
  for (let i = 1; i < times.length; i++) {
    if (Math.abs(times[i] - target) < Math.abs(times[best] - target)) best = i;
  }
  return best;
}

function pickTicks(series) {
  const limit = tickLimit();
  const count = series.length;
  if (count <= limit) return null;

  const times = series.map((p) => new Date(p.time).getTime());
  const step = (count - 1) / (limit - 1);
  const unit = snapUnit(times);

  const room = step * MIN_TICK_GAP;
  const picked = [0];

  for (let i = 1; i < limit - 1; i++) {
    const ideal = Math.round(i * step);
    const index = unit ? nearestTick(times, snapTarget(times[ideal], unit)) : ideal;
    const fits = index - picked[picked.length - 1] >= room && count - 1 - index >= room;
    if (fits) picked.push(index);
  }
  picked.push(count - 1);
  return new Set(picked);
}

function showTick(index) {
  return shownTicks === null || shownTicks.has(index);
}

function pointRadius(ctx) {
  const area = ctx.chart.chartArea;
  const gaps = ctx.chart.data.labels.length - 1;
  if (!area || gaps < 1) return POINT_RADIUS;
  return area.width / gaps >= POINT_GAP_PX ? POINT_RADIUS : 0;
}

function volts(mv) {
  return mv === null || mv === undefined ? null : mv / 1000;
}

function formatVolts(mv) {
  return mv === null || mv === undefined ? '–' : `${(mv / 1000).toFixed(2)} V`;
}

function labelMode() {
  if (hours === 'all' || hours > 720) return 'date';
  return hours > 24 ? 'datetime' : 'time';
}

function formatTime(iso, mode) {
  const d = new Date(iso);
  if (mode === 'date') {
    return d.toLocaleDateString('cs-CZ', { day: 'numeric', month: 'numeric', year: 'numeric' });
  }
  const time = d.toLocaleTimeString('cs-CZ', { hour: '2-digit', minute: '2-digit' });
  if (mode !== 'datetime') return time;
  return `${d.toLocaleDateString('cs-CZ', { day: 'numeric', month: 'numeric' })} ${time}`;
}

function formatAgo(iso) {
  const seconds = Math.max(0, Math.round((Date.now() - new Date(iso).getTime()) / 1000));
  if (seconds < 60) return 'před chvílí';
  const minutes = Math.round(seconds / 60);
  if (minutes < 60) return `před ${minutes} min`;
  const h = Math.round(minutes / 60);
  return h < 48 ? `před ${h} h` : `před ${Math.round(h / 24)} dny`;
}

function setBadge(up, text) {
  const badge = el('link');
  badge.classList.toggle('is-up', up === true);
  badge.classList.toggle('is-down', up === false);
  el('link-text').textContent = text;
}

function renderStatus(status) {
  el('device').textContent = status.device ?? '';

  setBadge(status.ttnConnected, status.ttnConnected ? 'TTN připojeno' : 'TTN odpojeno');

  renderPending(status.pending);
  renderCount(status.readings ?? null);

  if (status.dbOk === false) {
    showToast('Databáze hlásí chybu, měření se nemusí ukládat.', 'is-error');
  }
}

function describe(commands) {
  return (commands ?? []).map((name) => COMMAND_LABELS[name] ?? name).join(' + ');
}

function pocet(n, jeden, dva, vice) {
  const cislo = n.toLocaleString('cs-CZ');
  if (n === 1) return `${cislo} ${jeden}`;
  return n >= 2 && n <= 4 ? `${cislo} ${dva}` : `${cislo} ${vice}`;
}

function countCommands(n) {
  return pocet(n, 'příkaz', 'příkazy', 'příkazů');
}

function renderPending(pending) {
  const list = pending ?? [];
  el('queue-value').textContent = list.length === 0
    ? 'nic nečeká'
    : list.map((entry) => describe(entry.commands)).join(' · ');
  el('queue').classList.toggle('is-waiting', list.length > 0);
}

function renderLatest(uplink) {
  if (!uplink) {
    for (const id of ['battery', 'panel', 'door', 'seen']) {
      el(`${id}-value`).textContent = '–';
      el(`${id}-note`).textContent = '';
      el(`${id}-note`).className = 'tile-note';
    }
    return;
  }

  const r = uplink.reading;

  el('battery-value').textContent = formatVolts(r.batteryMv);
  el('battery-note').textContent = r.batteryMv === null
    ? 'čidlo neodpovídá'
    : (r.batteryCritical ? '⚠ kriticky vybitá' : (r.batterySaturated ? 'na horní mezi rozsahu' : ''));
  el('battery-note').className = `tile-note ${r.batteryCritical ? 'is-critical' : (r.batteryMv === null ? 'is-warning' : '')}`;

  el('panel-value').textContent = formatVolts(r.panelMv);
  el('panel-note').textContent = r.panelMv === null
    ? 'čidlo neodpovídá'
    : (r.panelSaturated ? 'na horní mezi rozsahu' : '');
  el('panel-note').className = `tile-note ${r.panelMv === null ? 'is-warning' : ''}`;

  const door = DOOR_LABELS[r.door] ?? DOOR_LABELS.unknown;
  el('door-value').textContent = door.text;
  el('door-note').textContent = door.note;
  el('door-note').className = `tile-note ${door.tone}`;

  el('seen-value').textContent = formatTime(uplink.receivedAt, 'time');
  const ago = formatAgo(uplink.receivedAt);
  el('seen-note').textContent = uplink.radio?.rssi == null
    ? ago
    : `${ago} (${uplink.radio.rssi} dBm)`;
}

function renderLegend() {
  el('legend').innerHTML = [
    ['Baterie', css('--series-1')],
    ['Panel', css('--series-2')]
  ].map(([name, color]) =>
    `<span class="legend-item"><span class="legend-swatch" style="background:${color}"></span>${name}</span>`
  ).join('');
}

function renderTable() {
  const mode = labelMode();
  el('table').querySelector('tbody').innerHTML = points.slice().reverse().map((p) =>
    `<tr><td>${formatTime(p.time, mode)}</td><td>${formatVolts(p.batteryMv)}</td><td>${formatVolts(p.panelMv)}</td></tr>`
  ).join('');
}

function renderChart() {
  const empty = points.length === 0;
  const tableShown = !el('table-wrap').hidden;
  el('chart-empty').hidden = !empty;
  el('plot-wrap').hidden = empty || tableShown;
  el('legend').hidden = empty || tableShown;
  el('chart-hint').hidden = empty || !(hours === 'all' || hours > 24);

  if (empty) {
    if (chart) { chart.destroy(); chart = null; }
    return;
  }

  const mode = labelMode();
  const labels = points.map((p) => formatTime(p.time, mode));
  shownTicks = pickTicks(points);
  const datasets = [
    { label: 'Baterie', data: points.map((p) => volts(p.batteryMv)), borderColor: css('--series-1'), backgroundColor: css('--series-1') },
    { label: 'Panel', data: points.map((p) => volts(p.panelMv)), borderColor: css('--series-2'), backgroundColor: css('--series-2') }
  ].map((d) => ({
    ...d,
    borderWidth: 2,
    pointRadius,
    pointHoverRadius: 6,
    pointBorderWidth: 2,
    pointBorderColor: css('--surface-1'),
    tension: 0,
    spanGaps: true
  }));

  if (chart) {
    chart.data.labels = labels;
    chart.data.datasets.forEach((d, i) => {
      d.data = datasets[i].data;
      d.borderColor = datasets[i].borderColor;
      d.backgroundColor = datasets[i].backgroundColor;
      d.pointBorderColor = datasets[i].pointBorderColor;
    });
    chart.options.scales.x.ticks.color = css('--text-muted');
    chart.options.scales.y.ticks.color = css('--text-muted');
    chart.options.scales.x.grid.color = css('--grid');
    chart.options.scales.y.grid.color = css('--grid');
    chart.options.scales.x.border.color = css('--axis');
    chart.options.scales.y.border.color = css('--axis');
    chart.options.scales.x.grid.tickColor = css('--text-muted');
    chart.options.scales.y.grid.tickColor = css('--text-muted');
    chart.update();
    return;
  }

  chart = new Chart(el('chart'), {
    type: 'line',
    data: { labels, datasets },
    options: {
      responsive: true,
      maintainAspectRatio: false,
      interaction: { mode: 'index', intersect: false },
      plugins: {
        legend: { display: false },
        tooltip: {
          displayColors: true,
          callbacks: {
            label: (ctx) => `${ctx.dataset.label}: ${ctx.parsed.y === null ? '–' : ctx.parsed.y.toFixed(2)} V`
          }
        }
      },
      scales: {
        x: {
          grid: {
            color: css('--grid'),
            drawTicks: true,
            tickLength: TICK_LENGTH_PX,
            tickColor: css('--text-muted'),
            tickWidth: (ctx) => (showTick(ctx.index) ? 1 : 0),
            lineWidth: (ctx) => (showTick(ctx.index) ? 1 : 0)
          },
          border: { color: css('--axis') },
          ticks: {
            color: css('--text-muted'),
            autoSkip: false,
            maxRotation: 0,
            align: 'inner',
            callback(value, index) {
              return showTick(index) ? this.getLabelForValue(value) : '';
            }
          }
        },
        y: {
          grid: {
            color: css('--grid'),
            drawTicks: true,
            tickLength: TICK_LENGTH_PX,
            tickColor: css('--text-muted')
          },
          border: { color: css('--axis') },
          ticks: { color: css('--text-muted'), callback: (v) => `${v} V` },
          title: { display: false }
        }
      }
    }
  });
}

function reason(err) {
  const lost = err instanceof TypeError || err?.name === 'AbortError' || err?.name === 'TimeoutError';
  return lost ? 'server neodpovídá' : err.message;
}

async function request(url, options) {
  const controller = new AbortController();
  const timer = setTimeout(() => controller.abort(), REQUEST_MS);
  try {
    return await fetch(url, { ...options, signal: controller.signal });
  } finally {
    clearTimeout(timer);
  }
}

function showToast(text, tone, id = 'toast') {
  const toast = el(id);
  toast.textContent = text;
  toast.className = `toast ${tone ?? ''}`;
  toast.hidden = false;

  clearTimeout(toastTimers.get(id));
  if (tone === 'is-ok') {
    toastTimers.set(id, setTimeout(() => { toast.hidden = true; }, TOAST_MS));
  }
}

function requireSession(res) {
  if (res.status === 401) {
    location.replace('/login.html');
    return false;
  }
  return true;
}

async function loadHistory() {
  el('chart-sub').textContent = 'načítám…';
  try {
    const res = await request(`/api/history?hours=${hours}`);
    if (!requireSession(res)) return;
    const body = await res.json();
    if (!res.ok) throw new Error(body.error ?? res.statusText);
    points = body.points;
    el('chart-sub').textContent = `baterie a solární panel · ${points.length} měření`;
  } catch (err) {
    points = [];
    el('chart-sub').textContent = 'historii se nepodařilo načíst';
    showToast(`Historii se nepodařilo načíst: ${reason(err)}`, 'is-error');
  }
  renderChart();
  renderTable();
}

async function loadStatus() {
  try {
    const res = await request('/api/status');
    if (!requireSession(res)) return;
    const status = await res.json();
    renderStatus(status);
    renderLatest(status.latest);
  } catch (err) {
    setBadge(false, 'Server nedostupný');
    showToast('Server neodpovídá.', 'is-error');
  }
}

function connectSocket() {
  const proto = location.protocol === 'https:' ? 'wss' : 'ws';
  const socket = new WebSocket(`${proto}://${location.host}/ws`);

  socket.addEventListener('message', (event) => {
    const { type, data } = JSON.parse(event.data);
    if (type === 'status') { renderStatus(data); renderLatest(data.latest); }
    if (type === 'uplink') { renderLatest(data); loadHistory(); }
    if (type === 'command') showToast(`Zařazeno do fronty: ${describe(data.commands)} — čeká na další zprávu z kurníku`, 'is-ok');
    if (type === 'pending') renderPending(data);
    if (type === 'downlink') {
      const info = DOWNLINK_EVENTS[data.event] ?? { text: data.event, tone: '' };
      showToast(data.commands ? `${info.text}: ${describe(data.commands)}` : info.text, info.tone);
    }
  });

  socket.addEventListener('close', (event) => {
    setBadge(false, 'Server nedostupný');
    if (event.code === 1008 || event.code === 1006) {
      fetch('/api/status').then((r) => requireSession(r));
    }
    setTimeout(connectSocket, 3000);
  });
}

el('theme').addEventListener('click', () => {
  const dark = document.documentElement.getAttribute('data-theme') === 'dark';
  document.documentElement.setAttribute('data-theme', dark ? 'light' : 'dark');
  try { localStorage.setItem('theme', dark ? 'light' : 'dark'); } catch { void 0; }
  renderLegend();
  renderChart();
});

function renderCount(stored) {
  el('data-count').textContent = stored === null
    ? '–'
    : pocet(stored, 'záznam', 'záznamy', 'záznamů');
  el('wipe').disabled = stored === 0;
}

el('wipe').addEventListener('click', () => {
  el('wipe-password').value = '';
  el('wipe-error').hidden = true;
  el('wipe-dialog').showModal();
  el('wipe-password').focus();
});

el('wipe-cancel').addEventListener('click', () => el('wipe-dialog').close());

el('wipe-confirm').addEventListener('click', async () => {
  const button = el('wipe-confirm');
  button.disabled = true;
  try {
    const res = await request('/api/data/clear', {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify({ password: el('wipe-password').value })
    });
    if (!requireSession(res)) return;
    const body = await res.json();
    if (!res.ok) throw new Error(body.error ?? res.statusText);
    el('wipe-dialog').close();
    showToast(body.removed > 0
      ? `Historie smazána: ${pocet(body.removed, 'záznam', 'záznamy', 'záznamů')}`
      : 'Nebylo co mazat', 'is-ok', 'data-toast');
    await loadHistory();
  } catch (err) {
    el('wipe-error').textContent = reason(err);
    el('wipe-error').hidden = false;
  } finally {
    button.disabled = false;
  }
});

el('cancel').addEventListener('click', async () => {
  const button = el('cancel');
  button.disabled = true;
  try {
    const res = await request('/api/command/cancel', { method: 'POST' });
    if (!requireSession(res)) return;
    const body = await res.json();
    if (!res.ok) throw new Error(body.error ?? res.statusText);
    showToast(body.cleared > 0
      ? `Zrušeno: ${countCommands(body.cleared)}`
      : 'Fronta je prázdná', 'is-ok');
  } catch (err) {
    showToast(`Zrušení selhalo: ${reason(err)}`, 'is-error');
  } finally {
    button.disabled = false;
  }
});

el('logout').addEventListener('click', async () => {
  await request('/api/logout', { method: 'POST' }).catch(() => undefined);
  location.replace('/login.html');
});

el('toggle-table').addEventListener('click', (e) => {
  const showTable = el('table-wrap').hidden;
  el('table-wrap').hidden = !showTable;
  el('plot-wrap').hidden = showTable || points.length === 0;
  el('legend').hidden = showTable || points.length === 0;
  e.currentTarget.setAttribute('aria-pressed', String(showTable));
  e.currentTarget.textContent = showTable ? 'Graf' : 'Tabulka';
});

document.querySelectorAll('.filterbar button').forEach((button) => {
  button.addEventListener('click', () => {
    document.querySelectorAll('.filterbar button').forEach((b) => {
      b.classList.remove('is-selected');
      b.removeAttribute('aria-pressed');
    });
    button.classList.add('is-selected');
    button.setAttribute('aria-pressed', 'true');
    hours = button.dataset.hours === 'all' ? 'all' : Number(button.dataset.hours);
    loadHistory();
  });
});

document.querySelectorAll('.commands button').forEach((button) => {
  button.addEventListener('click', async () => {
    button.disabled = true;
    try {
      const res = await request('/api/command', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ commands: [button.dataset.command] })
      });
      if (!requireSession(res)) return;
      const body = await res.json();
      if (!res.ok) throw new Error(body.error ?? res.statusText);
    } catch (err) {
      showToast(`Příkaz selhal: ${reason(err)}`, 'is-error');
    } finally {
      button.disabled = false;
    }
  });
});

try {
  const stored = localStorage.getItem('theme');
  if (stored) document.documentElement.setAttribute('data-theme', stored);
} catch { void 0; }

renderLegend();
loadStatus();
loadHistory();
connectSocket();
setInterval(loadStatus, 60000);
