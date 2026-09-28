const DOOR_LABELS = {
  closed: { text: 'Zavřeno', note: '', tone: '' },
  open: { text: 'Otevřeno', note: '', tone: '' },
  fault: { text: 'Porucha', note: '⚠ čeká na odblokování', tone: 'is-critical' },
  unknown: { text: 'Neznámý', note: '⚠ koncový spínač nehlásí polohu', tone: 'is-warning' }
};

const el = (id) => document.getElementById(id);
const css = (name) => getComputedStyle(document.querySelector('.viz-root')).getPropertyValue(name).trim();

let chart = null;
let hours = 24;
let points = [];

function volts(mv) {
  return mv === null || mv === undefined ? null : mv / 1000;
}

function formatVolts(mv) {
  return mv === null || mv === undefined ? '–' : `${(mv / 1000).toFixed(2)} V`;
}

function formatTime(iso, withDate) {
  const d = new Date(iso);
  const time = d.toLocaleTimeString('cs-CZ', { hour: '2-digit', minute: '2-digit' });
  return withDate ? `${d.toLocaleDateString('cs-CZ', { day: 'numeric', month: 'numeric' })} ${time}` : time;
}

function formatAgo(iso) {
  const seconds = Math.max(0, Math.round((Date.now() - new Date(iso).getTime()) / 1000));
  if (seconds < 60) return 'před chvílí';
  const minutes = Math.round(seconds / 60);
  if (minutes < 60) return `před ${minutes} min`;
  const h = Math.round(minutes / 60);
  return h < 48 ? `před ${h} h` : `před ${Math.round(h / 24)} dny`;
}

function renderStatus(status) {
  el('device').textContent = status.device ?? '';

  const badge = el('link');
  badge.classList.toggle('is-up', status.ttnConnected === true);
  badge.classList.toggle('is-down', status.ttnConnected === false);
  el('link-text').textContent = status.ttnConnected ? 'TTN připojeno' : 'TTN odpojeno';

  if (status.influxOk === false) {
    showToast(`InfluxDB nedostupná: ${status.influxError ?? 'neznámá chyba'}`, 'is-error');
  }
}

function renderLatest(uplink) {
  if (!uplink) return;
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

  el('seen-value').textContent = formatTime(uplink.receivedAt, false);
  el('seen-note').textContent = [
    formatAgo(uplink.receivedAt),
    uplink.radio?.rssi != null ? `${uplink.radio.rssi} dBm` : null
  ].filter(Boolean).join(' · ');
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
  const withDate = hours > 24;
  el('table').querySelector('tbody').innerHTML = points.slice().reverse().map((p) =>
    `<tr><td>${formatTime(p.time, withDate)}</td><td>${formatVolts(p.batteryMv)}</td><td>${formatVolts(p.panelMv)}</td></tr>`
  ).join('');
}

function renderChart() {
  const empty = points.length === 0;
  el('chart-empty').hidden = !empty;
  el('plot-wrap').hidden = empty;

  if (empty) {
    if (chart) { chart.destroy(); chart = null; }
    return;
  }

  const withDate = hours > 24;
  const labels = points.map((p) => formatTime(p.time, withDate));
  const datasets = [
    { label: 'Baterie', data: points.map((p) => volts(p.batteryMv)), borderColor: css('--series-1'), backgroundColor: css('--series-1') },
    { label: 'Panel', data: points.map((p) => volts(p.panelMv)), borderColor: css('--series-2'), backgroundColor: css('--series-2') }
  ].map((d) => ({
    ...d,
    borderWidth: 2,
    pointRadius: points.length > 120 ? 0 : 4,
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
      d.pointRadius = datasets[i].pointRadius;
      d.pointBorderColor = datasets[i].pointBorderColor;
    });
    chart.options.scales.x.ticks.color = css('--text-muted');
    chart.options.scales.y.ticks.color = css('--text-muted');
    chart.options.scales.x.grid.color = css('--grid');
    chart.options.scales.y.grid.color = css('--grid');
    chart.options.scales.x.border.color = css('--axis');
    chart.options.scales.y.border.color = css('--axis');
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
          grid: { color: css('--grid'), drawTicks: false },
          border: { color: css('--axis') },
          ticks: { color: css('--text-muted'), maxTicksLimit: 8, autoSkip: true, maxRotation: 0 }
        },
        y: {
          grid: { color: css('--grid'), drawTicks: false },
          border: { color: css('--axis') },
          ticks: { color: css('--text-muted'), callback: (v) => `${v} V` },
          title: { display: false }
        }
      }
    }
  });
}

function showToast(text, tone) {
  const toast = el('toast');
  toast.textContent = text;
  toast.className = `toast ${tone ?? ''}`;
  toast.hidden = false;
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
    const res = await fetch(`/api/history?hours=${hours}`);
    if (!requireSession(res)) return;
    const body = await res.json();
    if (!res.ok) throw new Error(body.error ?? res.statusText);
    points = body.points;
    el('chart-sub').textContent = `baterie a solární panel · ${points.length} měření`;
  } catch (err) {
    points = [];
    el('chart-sub').textContent = 'historii se nepodařilo načíst';
    showToast(`Historie: ${err.message}`, 'is-error');
  }
  renderChart();
  renderTable();
}

async function loadStatus() {
  try {
    const res = await fetch('/api/status');
    if (!requireSession(res)) return;
    const status = await res.json();
    renderStatus(status);
    renderLatest(status.latest);
  } catch (err) {
    showToast(`Server neodpovídá: ${err.message}`, 'is-error');
  }
}

function connectSocket() {
  const proto = location.protocol === 'https:' ? 'wss' : 'ws';
  const socket = new WebSocket(`${proto}://${location.host}/ws`);

  socket.addEventListener('message', (event) => {
    const { type, data } = JSON.parse(event.data);
    if (type === 'status') { renderStatus(data); renderLatest(data.latest); }
    if (type === 'uplink') { renderLatest(data); loadHistory(); }
    if (type === 'command') showToast(`Příkaz odeslán: 0x${data.byte.toString(16).padStart(2, '0').toUpperCase()} — čeká na další uplink`, 'is-ok');
  });

  socket.addEventListener('close', (event) => {
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

el('logout').addEventListener('click', async () => {
  await fetch('/api/logout', { method: 'POST' }).catch(() => undefined);
  location.replace('/login.html');
});

el('toggle-table').addEventListener('click', (e) => {
  const showTable = el('table-wrap').hidden;
  el('table-wrap').hidden = !showTable;
  el('plot-wrap').hidden = showTable || points.length === 0;
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
    hours = Number(button.dataset.hours);
    loadHistory();
  });
});

document.querySelectorAll('.commands button').forEach((button) => {
  button.addEventListener('click', async () => {
    button.disabled = true;
    try {
      const res = await fetch('/api/command', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ commands: [button.dataset.command] })
      });
      if (!requireSession(res)) return;
      const body = await res.json();
      if (!res.ok) throw new Error(body.error ?? res.statusText);
    } catch (err) {
      showToast(`Příkaz selhal: ${err.message}`, 'is-error');
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
