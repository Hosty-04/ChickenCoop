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

const NEST_COMMAND_LABELS = {
  tare: 'vynulovat váhu',
  calibrate: 'zkalibrovat váhu'
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
const DATE_LABEL_PX = 115;
const MAX_TICKS = 8;
const MIN_TICK_GAP = 0.75;
const MAX_JOIN_BUCKETS = 2.5;
const NOON_HOUR = 12;
const HOUR_MS = 60 * 60 * 1000;
const DAY_MS = 24 * HOUR_MS;
const WEEK_MS = 7 * DAY_MS;

let chart = null;
let hours = 24;
let points = [];
let odchazim = false;
let socket = null;
let chartWindow = null;
let chartBucket = 0;
let eggChart = null;
let eggPoints = [];
let eggBucket = 0;
let eggEnd = 0;
let nestCount = 0;
const toastTimers = new Map();

function tickLimit() {
  const room = el('plot-wrap').clientWidth / (labelMode() === 'time' ? TIME_LABEL_PX : DATE_LABEL_PX);
  return Math.max(2, Math.min(MAX_TICKS, Math.floor(room)));
}

function snapUnit(spanMs, bucketMs) {
  if (spanMs >= 2 * DAY_MS && bucketMs < DAY_MS) return DAY_MS;
  if (spanMs >= 2 * HOUR_MS && bucketMs < HOUR_MS) return HOUR_MS;
  return 0;
}

function series(points, times, hodnota) {
  const limit = chartBucket * MAX_JOIN_BUCKETS;
  const out = [];

  for (let i = 0; i < points.length; i++) {
    if (i > 0 && times[i] - times[i - 1] > limit) {
      out.push({ x: (times[i - 1] + times[i]) / 2, y: null });
    }
    out.push({ x: times[i], y: hodnota(points[i]) });
  }
  return out;
}

function bucketOf(times) {
  let bucketMs = Infinity;
  for (let i = 1; i < times.length; i++) bucketMs = Math.min(bucketMs, times[i] - times[i - 1]);
  return Number.isFinite(bucketMs) ? bucketMs : 0;
}

function windowOf(times) {
  const to = Date.now();
  return { from: hours === 'all' ? Math.min(times[0], to) : to - hours * HOUR_MS, to };
}

function snapTarget(ideal, unit) {
  const target = new Date(unit === DAY_MS ? ideal : ideal + HOUR_MS / 2);

  if (unit === DAY_MS) target.setHours(NOON_HOUR, 0, 0, 0);
  else target.setMinutes(0, 0, 0);
  return target.getTime();
}

function pickTickTimes(first, last, bucketMs) {
  const limit = tickLimit();
  const spanMs = last - first;
  if (spanMs <= 0) return [first];

  const unit = snapUnit(spanMs, bucketMs > 0 ? bucketMs : spanMs);
  const room = (spanMs / (limit - 1)) * MIN_TICK_GAP;
  const picked = [first];
  const add = (at) => {
    if (at - picked[picked.length - 1] >= room && last - at >= room) picked.push(at);
  };

  if (unit) {
    const strideMs = Math.ceil(spanMs / (limit - 1) / unit) * unit;
    for (let at = first + strideMs; at < last; at += strideMs) add(snapTarget(at, unit));
  } else {
    const step = spanMs / (limit - 1);
    for (let i = 1; i < limit - 1; i++) add(first + Math.round(i * step));
  }

  picked.push(last);
  return picked;
}

function pointRadius(ctx) {
  const area = ctx.chart.chartArea;
  if (!area || !chartWindow || chartBucket <= 0) return POINT_RADIUS;
  const rozestup = (area.width * chartBucket) / (chartWindow.to - chartWindow.from);
  return rozestup >= POINT_GAP_PX ? POINT_RADIUS : 0;
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
  renderLatest(status.latest);
  renderNests(status);

  if (status.dbOk === false) {
    showToast('Databáze hlásí chybu, měření se nemusí ukládat.', 'is-error', 'data-toast', 'databaze');
  } else {
    hideToast('data-toast', 'databaze');
  }
}

function nestList(nests) {
  const list = nests ?? [];
  if (list.length === 1) return `hnízda ${list[0]}`;
  return `hnízd ${list.slice(0, -1).join(', ')} a ${list[list.length - 1]}`;
}

function describe(commands, nests) {
  return (commands ?? []).map((name) => (name in NEST_COMMAND_LABELS
    ? `${NEST_COMMAND_LABELS[name]} ${nestList(nests)}`
    : COMMAND_LABELS[name] ?? name)).join(' + ');
}

function pocet(n, jeden, dva, vice) {
  const cislo = n.toLocaleString('cs-CZ');
  if (n === 1) return `${cislo} ${jeden}`;
  return n >= 2 && n <= 4 ? `${cislo} ${dva}` : `${cislo} ${vice}`;
}

function countCommands(n) {
  return pocet(n, 'příkaz', 'příkazy', 'příkazů');
}

function countEggs(n) {
  return pocet(n, 'vejce', 'vejce', 'vajec');
}

function laidToday(n) {
  return n > 0 ? `dnes +${countEggs(n)}` : 'dnes zatím nic';
}

function renderPending(pending) {
  const list = pending ?? [];
  el('queue-value').textContent = list.length === 0
    ? 'nic nečeká'
    : list.map((entry) => describe(entry.commands, entry.nests)).join(' · ');
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
    ? '⚠ čidlo neodpovídá'
    : (r.batteryCritical ? '⚠ kriticky vybitá' : (r.batterySaturated ? 'na horní mezi rozsahu' : ''));
  el('battery-note').className = `tile-note ${r.batteryCritical ? 'is-critical' : (r.batteryMv === null ? 'is-warning' : '')}`;

  el('panel-value').textContent = formatVolts(r.panelMv);
  el('panel-note').textContent = r.panelMv === null
    ? '⚠ čidlo neodpovídá'
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
  renderPicks();
}

function nestBoxes() {
  return [...el('nest-picks').querySelectorAll('input[value]')];
}

function pickedNests() {
  return nestBoxes().filter((input) => input.checked).map((input) => Number(input.value));
}

function pickedText(picked) {
  if (picked.length === 0) return 'Vyberte hnízda';
  if (picked.length === 1) return `Hnízdo ${picked[0]}`;
  if (picked.length === nestCount) return 'Všechna hnízda';
  if (picked.length > nestCount / 2) {
    return `Všechna kromě ${nestBoxes().filter((input) => !input.checked).map((input) => input.value).join(', ')}`;
  }
  return `Hnízda ${picked.join(', ')}`;
}

function renderPicks() {
  const picked = pickedNests();
  const all = el('nest-all');

  if (all) {
    all.checked = picked.length === nestCount;
    all.indeterminate = picked.length > 0 && picked.length < nestCount;
  }
  el('nest-picker-text').textContent = pickedText(picked);
  document.querySelectorAll('.commands button[data-nest-command]').forEach((button) => {
    button.disabled = picked.length === 0;
  });
}

function renderNests(status) {
  const count = status.nestCount ?? 0;
  const eggsMax = status.eggsMax ?? 10;
  const snapshot = status.nests;

  if (count !== nestCount) {
    nestCount = count;
    buildNests(count, eggsMax);
  }

  const today = Boolean(snapshot) && new Date(snapshot.checkedAt).toDateString() === new Date().toDateString();

  if (!snapshot) {
    el('nests-sub').textContent = 'zatím žádná kontrola';
  } else if (today) {
    el('nests-sub').textContent = `poslední kontrola ${formatTime(snapshot.checkedAt, 'time')} · ${laidToday(snapshot.laidToday)}`;
  } else {
    el('nests-sub').textContent = `poslední kontrola ${formatTime(snapshot.checkedAt, 'datetime')}`;
  }

  for (let i = 0; i < count; i++) {
    const nest = snapshot?.nests?.[i] ?? { state: null, eggs: null, laidToday: 0 };
    const known = nest.eggs !== null && nest.state !== 'uncalibrated';
    const full = nest.state === 'ok' && nest.eggs >= eggsMax;
    const label = NEST_LABELS[nest.state];

    el(`nest-${i}-value`).textContent = known ? countEggs(nest.eggs) : '–';

    el(`nest-${i}-tray`).classList.toggle('is-stale', nest.state !== 'ok');
    el(`nest-${i}-tray`).querySelectorAll('.egg').forEach((egg, slot) => {
      egg.classList.toggle('is-laid', known && slot < nest.eggs);
    });

    let note = nest.state === 'ok' && today ? laidToday(nest.laidToday) : '';
    let tone = '';
    if (full) { note = '⚠ košík je plný'; tone = 'is-warning'; }
    if (label) { note = label.note; tone = label.tone; }

    el(`nest-${i}-note`).textContent = note;
    el(`nest-${i}-note`).className = `tile-note ${tone}`;
  }
}

function eggEdges() {
  return [...eggPoints.map((p) => new Date(p.time).getTime()), eggEnd];
}

function eggTickLimit() {
  const room = el('eggs-plot-wrap').clientWidth / (eggBucket < WEEK_MS ? TIME_LABEL_PX : DATE_LABEL_PX);
  return Math.max(2, Math.min(MAX_TICKS, Math.floor(room)));
}

function eggTicks() {
  const edges = eggEdges();
  const intervals = edges.length - 1;
  const stride = Math.max(1, Math.ceil(intervals / (eggTickLimit() - 1)));
  const picked = [];

  for (let i = 0; i < edges.length; i += stride) picked.push(edges[i]);
  if (intervals % stride !== 0) {
    if (picked.length > 1 && intervals % stride < stride * MIN_TICK_GAP) picked.pop();
    picked.push(edges[intervals]);
  }
  return picked;
}

function eggTick(ms) {
  const d = new Date(ms);
  if (eggBucket < DAY_MS) return d.toLocaleTimeString('cs-CZ', { hour: '2-digit', minute: '2-digit' });
  return d.toLocaleDateString('cs-CZ', eggBucket < WEEK_MS
    ? { day: 'numeric', month: 'numeric' }
    : { day: 'numeric', month: 'numeric', year: 'numeric' });
}

function eggPeriod(index) {
  const edges = eggEdges();
  const from = new Date(edges[index]);
  const to = new Date(edges[index + 1]);
  const time = (d) => d.toLocaleTimeString('cs-CZ', { hour: '2-digit', minute: '2-digit' });
  const date = (d, options) => d.toLocaleDateString('cs-CZ', options);

  if (eggBucket < DAY_MS) return `${date(from, { day: 'numeric', month: 'numeric' })} ${time(from)}–${time(to)}`;
  if (eggBucket < WEEK_MS) return date(from, { weekday: 'long', day: 'numeric', month: 'numeric', year: 'numeric' });
  return `týden od ${date(from, { day: 'numeric', month: 'numeric', year: 'numeric' })}`;
}

function formatEggs(n) {
  return n === null || n === undefined ? '–' : String(n);
}

function renderEggTable() {
  const nests = eggPoints[0]?.laid.length ?? 0;
  el('eggs-table').querySelector('thead').innerHTML = `<tr><th scope="col">Čas</th><th scope="col">Celkem</th>${
    Array.from({ length: nests }, (_, i) => `<th scope="col">Hnízdo ${i + 1}</th>`).join('')
  }</tr>`;
  el('eggs-table').querySelector('tbody').innerHTML = eggPoints.map((p, i) =>
    `<tr><td>${eggPeriod(i)}</td><td>${formatEggs(p.total)}</td>${
      p.laid.map((n) => `<td>${formatEggs(n)}</td>`).join('')
    }</tr>`
  ).reverse().join('');
}

function renderEggChart() {
  const empty = eggPoints.every((p) => p.total === null);
  const tableShown = !el('eggs-table-wrap').hidden;
  el('eggs-empty').hidden = !empty;
  el('eggs-plot-wrap').hidden = empty || tableShown;

  if (empty) {
    if (eggChart) { eggChart.destroy(); eggChart = null; }
    return;
  }

  const edges = eggEdges();
  const data = eggPoints.map((p, i) => ({ x: (edges[i] + edges[i + 1]) / 2, y: p.total }));

  if (eggChart) {
    eggChart.data.datasets[0].data = data;
    eggChart.data.datasets[0].backgroundColor = css('--series-4');
    eggChart.options.scales.x.min = edges[0];
    eggChart.options.scales.x.max = edges[edges.length - 1];
    eggChart.options.scales.x.ticks.color = css('--text-muted');
    eggChart.options.scales.y.ticks.color = css('--text-muted');
    eggChart.options.scales.x.grid.color = css('--grid');
    eggChart.options.scales.y.grid.color = css('--grid');
    eggChart.options.scales.x.border.color = css('--axis');
    eggChart.options.scales.y.border.color = css('--axis');
    eggChart.options.scales.x.grid.tickColor = css('--text-muted');
    eggChart.options.scales.y.grid.tickColor = css('--text-muted');
    eggChart.update();
    return;
  }

  eggChart = new Chart(el('eggs-chart'), {
    type: 'bar',
    data: {
      datasets: [{
        label: 'Snesená vejce',
        data,
        backgroundColor: css('--series-4'),
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
              const p = eggPoints[ctx.dataIndex];
              if (p.total === null) return 'bez dat';
              const missing = p.laid.flatMap((n, i) => (n === null ? [i + 1] : []));
              return [
                `Celkem: ${countEggs(p.total)}`,
                ...p.laid.flatMap((n, i) => (n > 0 ? [`Hnízdo ${i + 1}: ${n}`] : [])),
                ...(missing.length > 0 ? [`bez dat: ${nestList(missing)}`] : [])
              ];
            }
          }
        }
      },
      scales: {
        x: {
          type: 'linear',
          offset: false,
          min: edges[0],
          max: edges[edges.length - 1],
          afterBuildTicks: (scale) => {
            if (eggPoints.length > 0) scale.ticks = eggTicks().map((value) => ({ value }));
          },
          grid: {
            offset: false,
            color: css('--grid'),
            drawTicks: true,
            tickLength: TICK_LENGTH_PX,
            tickColor: css('--text-muted')
          },
          border: { color: css('--axis') },
          ticks: {
            color: css('--text-muted'),
            autoSkip: false,
            maxRotation: 0,
            align: 'inner',
            callback: (value) => eggTick(value)
          }
        },
        y: {
          beginAtZero: true,
          grid: {
            color: css('--grid'),
            drawTicks: true,
            tickLength: TICK_LENGTH_PX,
            tickColor: css('--text-muted')
          },
          border: { color: css('--axis') },
          ticks: { color: css('--text-muted'), precision: 0 }
        }
      }
    }
  });
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

  const times = points.map((p) => new Date(p.time).getTime());
  chartBucket = bucketOf(times);
  chartWindow = windowOf(times);
  const datasets = [
    { label: 'Baterie', data: series(points, times, (p) => volts(p.batteryMv)), borderColor: css('--series-1'), backgroundColor: css('--series-1') },
    { label: 'Panel', data: series(points, times, (p) => volts(p.panelMv)), borderColor: css('--series-2'), backgroundColor: css('--series-2') }
  ].map((d) => ({
    ...d,
    borderWidth: 2,
    pointRadius,
    pointHoverRadius: 6,
    pointBorderWidth: 2,
    pointBorderColor: css('--surface-1'),
    tension: 0,
    spanGaps: false
  }));

  if (chart) {
    chart.options.scales.x.min = chartWindow.from;
    chart.options.scales.x.max = chartWindow.to;
    chart.data.datasets.forEach((d, i) => {
      d.data = datasets[i].data;
      d.spanGaps = datasets[i].spanGaps;
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
    data: { datasets },
    options: {
      responsive: true,
      maintainAspectRatio: false,
      animation: false,
      interaction: { mode: 'index', intersect: false },
      plugins: {
        legend: { display: false },
        tooltip: {
          displayColors: true,
          callbacks: {
            title: (items) => formatTime(items[0].parsed.x, labelMode()),
            label: (ctx) => `${ctx.dataset.label}: ${ctx.parsed.y === null ? '–' : ctx.parsed.y.toFixed(2)} V`
          }
        }
      },
      scales: {
        x: {
          type: 'linear',
          min: chartWindow.from,
          max: chartWindow.to,
          afterBuildTicks: (scale) => {
            if (!chartWindow) return;
            scale.ticks = pickTickTimes(chartWindow.from, chartWindow.to, chartBucket).map((value) => ({ value }));
          },
          grid: {
            color: css('--grid'),
            drawTicks: true,
            tickLength: TICK_LENGTH_PX,
            tickColor: css('--text-muted')
          },
          border: { color: css('--axis') },
          ticks: {
            color: css('--text-muted'),
            autoSkip: false,
            maxRotation: 0,
            align: 'inner',
            callback: (value) => formatTime(value, labelMode())
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

function showToast(text, tone, id = 'toast', duvod = '') {
  const toast = el(id);
  toast.textContent = text;
  toast.className = `toast ${tone ?? ''}`;
  toast.dataset.duvod = duvod;
  toast.hidden = false;

  clearTimeout(toastTimers.get(id));
  if (tone === 'is-error') return;

  toastTimers.set(id, setTimeout(() => {
    toast.hidden = true;
  }, TOAST_MS));
}

function hideToast(id, duvod) {
  const toast = el(id);
  if (toast.dataset.duvod === duvod) toast.hidden = true;
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
    hideToast('chart-toast', 'historie');
  } catch (err) {
    points = [];
    el('chart-sub').textContent = 'historii se nepodařilo načíst';
    showToast(`Historii se nepodařilo načíst: ${reason(err)}`, 'is-error', 'chart-toast', 'historie');
  }
  renderChart();
  renderTable();
}

async function loadEggs() {
  el('eggs-sub').textContent = 'načítám…';
  try {
    const res = await request(`/api/eggs?hours=${hours}`);
    if (!requireSession(res)) return;
    const body = await res.json();
    if (!res.ok) throw new Error(body.error ?? res.statusText);
    eggPoints = body.points;
    eggBucket = body.bucketMs;
    eggEnd = body.end ? new Date(body.end).getTime() : 0;
    const total = eggPoints.reduce((sum, p) => sum + (p.total ?? 0), 0);
    el('eggs-sub').textContent = `histogram snesených vajec · celkem ${countEggs(total)}`;
    hideToast('eggs-toast', 'snaska');
  } catch (err) {
    eggPoints = [];
    el('eggs-sub').textContent = 'historii snášky se nepodařilo načíst';
    showToast(`Historii snášky se nepodařilo načíst: ${reason(err)}`, 'is-error', 'eggs-toast', 'snaska');
  }
  renderEggChart();
  renderEggTable();
}

async function loadStatus() {
  try {
    const res = await request('/api/status');
    if (!requireSession(res)) return;
    renderStatus(await res.json());
  } catch {
    if (!odchazim) setBadge(false, 'Server nedostupný');
  }
}

function connectSocket() {
  if (socket && (socket.readyState === WebSocket.CONNECTING || socket.readyState === WebSocket.OPEN)) return;

  const proto = location.protocol === 'https:' ? 'wss' : 'ws';
  socket = new WebSocket(`${proto}://${location.host}/ws`);

  socket.addEventListener('message', (event) => {
    const { type, data } = JSON.parse(event.data);
    if (type === 'status') renderStatus(data);
    if (type === 'uplink') {
      renderLatest(data);
      loadHistory();
      if (data.reading?.nests) loadEggs();
    }
    if (type === 'command') showToast(`Zařazeno do fronty: ${describe(data.commands, data.nests)} — čeká na další zprávu z kurníku`, 'is-ok');
    if (type === 'pending') renderPending(data);
    if (type === 'cancelled') {
      showToast(data.cleared > 0 ? `Zrušeno: ${countCommands(data.cleared)}` : 'Fronta je prázdná', 'is-ok');
    }
    if (type === 'cleared') {
      showToast(data.removed > 0
        ? `Historie smazána: ${pocet(data.removed, 'záznam', 'záznamy', 'záznamů')}`
        : 'Nebylo co mazat', 'is-ok', 'data-toast');
      loadHistory();
      loadEggs();
    }
    if (type === 'downlink') {
      const info = DOWNLINK_EVENTS[data.event] ?? { text: data.event, tone: '' };
      showToast(data.commands ? `${info.text}: ${describe(data.commands, data.nests)}` : info.text, info.tone);
    }
  });

  socket.addEventListener('close', (event) => {
    if (odchazim) return;
    setBadge(false, 'Server nedostupný');
    if (event.code === 1008 || event.code === 1006) {
      fetch('/api/status').then((r) => requireSession(r)).catch(() => undefined);
    }
    setTimeout(connectSocket, 3000);
  });
}

window.addEventListener('pagehide', () => { odchazim = true; });

window.addEventListener('pageshow', (event) => {
  odchazim = false;
  if (!event.persisted) return;
  connectSocket();
  loadStatus();
  loadHistory();
  loadEggs();
});

el('theme').addEventListener('click', () => {
  const dark = document.documentElement.getAttribute('data-theme') === 'dark';
  document.documentElement.setAttribute('data-theme', dark ? 'light' : 'dark');
  try { localStorage.setItem('theme', dark ? 'light' : 'dark'); } catch { void 0; }
  renderLegend();
  renderChart();
  renderEggChart();
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
  } catch (err) {
    showToast(`Zrušení selhalo: ${reason(err)}`, 'is-error');
  } finally {
    button.disabled = false;
  }
});

el('logout').addEventListener('click', async () => {
  odchazim = true;
  await request('/api/logout', { method: 'POST' }).catch(() => undefined);
  location.replace('/login.html');
});

el('toggle-eggs-table').addEventListener('click', (e) => {
  const showTable = el('eggs-table-wrap').hidden;
  const empty = eggPoints.every((p) => p.total === null);
  el('eggs-table-wrap').hidden = !showTable;
  el('eggs-plot-wrap').hidden = showTable || empty;
  e.currentTarget.setAttribute('aria-pressed', String(showTable));
  e.currentTarget.textContent = showTable ? 'Graf' : 'Tabulka';
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
    loadEggs();
  });
});

async function sendCommand(button, command) {
  button.disabled = true;
  try {
    const res = await request('/api/command', {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify(command)
    });
    if (!requireSession(res)) return;
    const body = await res.json();
    if (!res.ok) throw new Error(body.error ?? res.statusText);
  } catch (err) {
    showToast(`Příkaz selhal: ${reason(err)}`, 'is-error');
  } finally {
    button.disabled = false;
    renderPicks();
  }
}

document.querySelectorAll('.commands button[data-command]').forEach((button) => {
  button.addEventListener('click', () => sendCommand(button, { commands: [button.dataset.command] }));
});

document.querySelectorAll('.commands button[data-nest-command]').forEach((button) => {
  button.addEventListener('click', () => sendCommand(button, {
    commands: [button.dataset.nestCommand],
    nests: pickedNests()
  }));
});

el('nest-picks').addEventListener('change', (e) => {
  if (e.target.id === 'nest-all') nestBoxes().forEach((input) => { input.checked = e.target.checked; });
  renderPicks();
});

document.addEventListener('click', (e) => {
  if (!el('nest-picker').contains(e.target)) el('nest-picker').open = false;
});

document.addEventListener('keydown', (e) => {
  if (e.key === 'Escape' && el('nest-picker').open) {
    el('nest-picker').open = false;
    el('nest-picker').querySelector('summary').focus();
  }
});

try {
  const stored = localStorage.getItem('theme');
  if (stored) document.documentElement.setAttribute('data-theme', stored);
} catch { void 0; }

renderLegend();
loadStatus();
loadHistory();
loadEggs();
connectSocket();
setInterval(loadStatus, 60000);
