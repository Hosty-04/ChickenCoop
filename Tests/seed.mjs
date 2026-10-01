import { rmSync } from 'node:fs';
import { join } from 'node:path';
import { fileURLToPath } from 'node:url';
import { DatabaseSync } from 'node:sqlite';

const DEVICE = 'lora-e5-mini';
const MIN = 60 * 1000;
const HOD = 60 * MIN;
const DEN = 24 * HOD;
const KROK = 10 * MIN;

const SLOZKA = fileURLToPath(new URL('.', import.meta.url));
const ted = Math.floor(Date.now() / KROK) * KROK;

const DATABAZE = {
  plny: { soubor: 'kurnik-test.db', popis: 'dva roky měření, všechny události v grafech' },
  poplach: { soubor: 'kurnik-test-poplach.db', popis: 'mrtvé čidlo baterie i panelu, neznámá dvířka' },
  porucha: { soubor: 'kurnik-test-porucha.db', popis: 'kriticky vybitá baterie, dvířka v poruše' },
  meze: { soubor: 'kurnik-test-meze.db', popis: 'baterie 8,00 V a panel 12,50 V na horní mezi rozsahu' },
  stara: { soubor: 'kurnik-test-stara.db', popis: 'poslední zpráva před pěti dny' },
  jedno: { soubor: 'kurnik-test-jedno.db', popis: 'jediné měření' },
  prazdna: { soubor: 'kurnik-test-prazdna.db', popis: 'prázdná databáze' }
};

let seminko = 20260101;

function nahoda() {
  seminko = (seminko * 1103515245 + 12345) % 2147483648;
  return seminko / 2147483648;
}

function hodina(t) {
  const d = new Date(t);
  return d.getHours() + d.getMinutes() / 60;
}

function denVRoce(t) {
  const d = new Date(t);
  return Math.floor((d - new Date(d.getFullYear(), 0, 0)) / DEN);
}

function delkaDne(t) {
  return 12 + 4 * Math.sin((2 * Math.PI * (denVRoce(t) - 80)) / 365);
}

function sezona(t) {
  return (delkaDne(t) - 8) / 8;
}

function pocasi(t) {
  const den = Math.floor(t / DEN);
  const x = Math.sin(den * 12.9898) * 43758.5453;
  return 0.35 + 0.65 * (x - Math.floor(x));
}

function voc(t) {
  return 11.5 - 1.3 * sezona(t);
}

function mezPrepeti(t) {
  const mesic = new Date(t).getMonth() + 1;
  if (mesic >= 6 && mesic <= 8) return 7.2;
  if (mesic === 12 || mesic <= 2) return 7.5;
  return 7.3;
}

function kritickaMez(t) {
  const mesic = new Date(t).getMonth() + 1;
  return mesic >= 11 || mesic <= 3 ? 6.15 : 6.0;
}

function slunce(t) {
  const delka = delkaDne(t);
  const h = hodina(t);
  const vychod = 13 - delka / 2;
  const zapad = 13 + delka / 2;
  if (h <= vychod || h >= zapad) return 0;
  return Math.sin((Math.PI * (h - vychod)) / delka);
}

function denniOkno(delka, odsazeni) {
  for (let t = ted - odsazeni; t > ted - 23 * HOD; t -= KROK) {
    const od = t - delka;
    if (new Date(od).toDateString() !== new Date(t).toDateString()) continue;
    if (hodina(od) >= 10 && hodina(t) <= 15) return [od, t];
  }
  return [ted - 7 * HOD, ted - 5 * HOD];
}

function kodBaterie(v) {
  if (v <= 5) return 0;
  return Math.min(60, Math.round((v * 1000 - 5000) / 50));
}

function kodPanelu(v) {
  return Math.min(125, Math.max(0, Math.round((v * 1000) / 100)));
}

function kratkeUdalosti() {
  const nasyceni = denniOkno(2 * HOD, 5 * HOD);
  return [
    { jmeno: 'výpadek 40 min', typ: 'vypadek', od: ted - 70 * MIN, do: ted - 30 * MIN, kde: '6 h, 24 h' },
    { jmeno: 'zpráva bez obou hodnot', typ: 'bezObojiho', od: ted - 2 * HOD, do: ted - 2 * HOD + KROK, kde: '6 h, 24 h' },
    { jmeno: 'zpráva bez napětí baterie', typ: 'bezBaterie', od: ted - 150 * MIN, do: ted - 150 * MIN + KROK, kde: '6 h, 24 h' },
    { jmeno: 'zpráva bez napětí panelu', typ: 'bezPanelu', od: ted - 3 * HOD, do: ted - 3 * HOD + KROK, kde: '6 h, 24 h' },
    { jmeno: 'baterie i panel na horní mezi', typ: 'nasyceno', od: nasyceni[0], do: nasyceni[1], kde: '24 h, 7 dní' },
    { jmeno: 'dvířka v poruše', typ: 'dvirkaPorucha', od: ted - 19 * HOD, do: ted - 17 * HOD, kde: 'jen v uložených datech' },
    { jmeno: 'dvířka neznámá', typ: 'dvirkaNeznama', od: ted - 22 * HOD, do: ted - 21 * HOD, kde: 'jen v uložených datech' },
    { jmeno: 'kriticky vybitá baterie (8 h)', typ: 'kriticka', od: ted - 2 * DEN, do: ted - 2 * DEN + 8 * HOD, kde: '7 dní' },
    { jmeno: 'výpadek 6 h', typ: 'vypadek', od: ted - 3 * DEN - 6 * HOD, do: ted - 3 * DEN, kde: '7 dní' },
    { jmeno: 'odpojený panel (1 den)', typ: 'panelOdpojen', od: ted - 5 * DEN, do: ted - 4 * DEN, kde: '7 dní, 30 dní' }
  ];
}

function dlouheUdalosti() {
  return [
    ...kratkeUdalosti(),
    { jmeno: 'výpadek 2 dny', typ: 'vypadek', od: ted - 12 * DEN, do: ted - 10 * DEN, kde: '30 dní' },
    { jmeno: 'odpojený panel (3 dny)', typ: 'panelOdpojen', od: ted - 18 * DEN, do: ted - 15 * DEN, kde: '30 dní' },
    { jmeno: 'baterie na horní mezi (4 dny)', typ: 'baterieNasycena', od: ted - 25 * DEN, do: ted - 21 * DEN, kde: '30 dní' },
    { jmeno: 'výpadek 9 dní', typ: 'vypadek', od: ted - 120 * DEN, do: ted - 111 * DEN, kde: '1 rok' },
    { jmeno: 'panel na horní mezi (2 dny)', typ: 'panelNasycen', od: ted - 150 * DEN, do: ted - 148 * DEN, kde: '1 rok' },
    { jmeno: 'kriticky vybitá baterie (5 dní)', typ: 'kriticka', od: ted - 200 * DEN, do: ted - 195 * DEN, kde: '1 rok' },
    { jmeno: 'odpojený panel (10 dní)', typ: 'panelOdpojen', od: ted - 300 * DEN, do: ted - 290 * DEN, kde: '1 rok, vše' },
    { jmeno: 'výpadek 21 dní', typ: 'vypadek', od: ted - 430 * DEN, do: ted - 409 * DEN, kde: 'vše' },
    { jmeno: 'kriticky vybitá baterie (6 dní)', typ: 'kriticka', od: ted - 620 * DEN, do: ted - 614 * DEN, kde: 'vše' }
  ];
}

function nastaveni(stav) {
  const konec = ted + 2 * DEN;
  const chvost = (typ, jmeno) => ({ jmeno, typ, od: ted - 20 * MIN, do: konec + KROK, kde: 'dlaždice' });

  if (stav === 'plny') return { zacatek: ted - 730 * DEN, konec, udalosti: dlouheUdalosti() };
  if (stav === 'poplach') return { zacatek: ted - 7 * DEN, konec, udalosti: [chvost('poplach', 'mrtvé čidlo baterie i panelu, neznámá dvířka'), ...kratkeUdalosti()] };
  if (stav === 'porucha') return { zacatek: ted - 7 * DEN, konec, udalosti: [chvost('porucha', 'kriticky vybitá baterie, dvířka v poruše'), ...kratkeUdalosti()] };
  if (stav === 'meze') return { zacatek: ted - 7 * DEN, konec, udalosti: [chvost('nasyceno', 'baterie 8,00 V a panel 12,50 V'), ...kratkeUdalosti()] };
  if (stav === 'stara') return { zacatek: ted - 12 * DEN, konec: ted - 5 * DEN, udalosti: [] };
  if (stav === 'jedno') return { zacatek: ted, konec: ted, udalosti: [] };
  return { zacatek: ted, konec: ted - KROK, udalosti: [] };
}

function vytvor(stav) {
  const { soubor, popis } = DATABAZE[stav];
  const cesta = join(SLOZKA, soubor);
  const { zacatek, konec, udalosti } = nastaveni(stav);

  for (const pripona of ['', '-wal', '-shm']) rmSync(cesta + pripona, { force: true });

  const db = new DatabaseSync(cesta);
  db.exec('PRAGMA journal_mode = WAL');
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

  const vloz = db.prepare(`
    INSERT OR REPLACE INTO readings
      (device, time, battery_mv, panel_mv, battery_critical, battery_saturated,
       panel_saturated, door, rssi, snr, sf, gateway)
    VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)
  `);

  let zapsano = 0;
  let drift = 0;

  db.exec('BEGIN');
  for (let t = zacatek; t <= konec; t += KROK) {
    const u = udalosti.find((e) => t >= e.od && t < e.do);
    if (u?.typ === 'vypadek') continue;

    const s = sezona(t);
    const sun = slunce(t);
    const mraky = pocasi(t);
    const osvit = sun * mraky;
    drift = Math.max(-0.12, Math.min(0.12, drift + (nahoda() - 0.5) * 0.02));

    let bat = Math.min(mezPrepeti(t), 6.85 + 0.35 * s + 0.45 * sun * (0.45 + 0.55 * mraky) - 0.1 * (1 - sun) + drift);
    let pan = osvit > 0.005 ? voc(t) * Math.max(0.6, 1 + 0.16 * Math.log10(osvit)) + (nahoda() - 0.5) * 0.1 : 0;
    let dvere = sun > 0 ? 1 : 0;
    let batNull = false;
    let panNull = false;

    if (u?.typ === 'bezObojiho') { batNull = true; panNull = true; }
    if (u?.typ === 'bezBaterie') batNull = true;
    if (u?.typ === 'bezPanelu') panNull = true;
    if (u?.typ === 'poplach') { batNull = true; panNull = true; dvere = 3; }
    if (u?.typ === 'porucha') { bat = 5.9; dvere = 2; }
    if (u?.typ === 'nasyceno') { bat = 8.2; pan = 12.8; }
    if (u?.typ === 'baterieNasycena') bat = 8.2;
    if (u?.typ === 'panelNasycen' && sun > 0) pan = 12.8;
    if (u?.typ === 'panelOdpojen') pan = 0;
    if (u?.typ === 'kriticka') bat = 5.9 + 0.1 * sun;
    if (u?.typ === 'dvirkaPorucha') dvere = 2;
    if (u?.typ === 'dvirkaNeznama') dvere = 3;

    const kodBat = kodBaterie(bat);
    const kodPan = kodPanelu(pan);

    vloz.run(
      DEVICE,
      t,
      batNull ? null : 5000 + kodBat * 50,
      panNull ? null : kodPan * 100,
      !batNull && bat <= kritickaMez(t) ? 1 : 0,
      !batNull && kodBat === 60 ? 1 : 0,
      !panNull && kodPan === 125 ? 1 : 0,
      dvere,
      -92 - Math.round(nahoda() * 20),
      Math.round((4 + nahoda() * 6) * 10) / 10,
      7,
      'eui-647fdafffe0122b1'
    );
    zapsano++;
  }
  db.exec('COMMIT');
  db.close();

  return { soubor, popis, zapsano, zacatek, konec, udalosti };
}

const cas = (t) => new Date(t).toLocaleString('cs-CZ', {
  day: '2-digit', month: '2-digit', year: 'numeric', hour: '2-digit', minute: '2-digit'
});

const zadane = process.argv.slice(2);
const neznamy = zadane.find((stav) => !(stav in DATABAZE));

if (neznamy) {
  console.error(`neznámý stav ${neznamy}, na výběr je: ${Object.keys(DATABAZE).join(', ')}`);
  process.exit(1);
}

for (const stav of zadane.length > 0 ? zadane : Object.keys(DATABAZE)) {
  const v = vytvor(stav);
  console.log(`\n${v.soubor} — ${v.popis}`);
  console.log(v.zapsano === 0
    ? '   bez měření'
    : `   ${v.zapsano} měření po 10 minutách, ${cas(v.zacatek)} – ${cas(v.konec)}`);
  for (const u of [...v.udalosti].sort((a, b) => a.od - b.od)) {
    console.log(`   ${u.jmeno.padEnd(50)} ${cas(u.od).padEnd(20)} ${u.kde}`);
  }
}
