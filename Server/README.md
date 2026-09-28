# Kurník — server

MQTT most do The Things Network, ukládání do InfluxDB a webový dashboard.

```
kurník → LoRaWAN → gateway → TTN ──MQTT──> tento server ──> InfluxDB
                                              │                 │
                                              └──── dashboard ──┘
```

Downlink jde opačně: tlačítko v dashboardu → server → TTN přes MQTT → gateway → kurník.

## Co je uvnitř

| Soubor | Role |
|---|---|
| `src/codec.js` | překlad payloadu — stejná pravidla jako `telemetry.c` ve firmwaru |
| `src/ttn.js` | MQTT klient: odběr uplinků, publikování downlinků |
| `src/influx.js` | zápis měření a dotazy na historii |
| `src/server.js` | HTTP API, statické soubory, WebSocket pro živé aktualizace |
| `public/` | dashboard (bez build kroku, čisté HTML/CSS/JS) |
| `test/` | testy kodéru proti payloadům z reálného zařízení |

## Co potřebuješ

- Docker a Docker Compose
- Node.js 20+ (jen pro vývojový režim a testy)
- API klíč z TTN

## 1. API klíč z TTN

V konzoli: **Applications → tvoje aplikace → API keys → Add API key**.

Práva zaškrtni tři:

- `Read application traffic (uplink and downlink)`
- `Write downlink application traffic`
- `View device info` (nepovinné, ale hodí se)

Klíč začíná `NNSXS.` a **zobrazí se jen jednou** — hned si ho zkopíruj.

## 2. Konfigurace

```bash
cp .env.example .env
```

Do `.env` doplň:

| Proměnná | Co tam patří |
|---|---|
| `TTN_APP_ID` | ID aplikace, u tebe `chicken-coop-thesis` |
| `TTN_DEVICE_ID` | ID zařízení, u tebe `lora-e5-mini` |
| `TTN_API_KEY` | klíč z kroku 1 |
| `INFLUX_TOKEN` | vymysli si dlouhý náhodný řetězec |
| `INFLUX_PASSWORD` | heslo do webu InfluxDB |

`INFLUX_TOKEN` a `INFLUX_PASSWORD` si vymýšlíš ty — InfluxDB se jimi při prvním startu založí.

Náhodný token vygeneruješ třeba takhle:

```bash
openssl rand -hex 32
```

## 3. Spuštění

```bash
docker compose up -d
```

Dashboard běží na <http://localhost:3000>, webové rozhraní InfluxDB na <http://localhost:8086>.

Logy:

```bash
docker compose logs -f app
```

Po připojení uvidíš `TTN connected` a `subscribed to v3/…/devices/+/up`. Při každé zprávě z kurníku přibude řádek s naměřenými hodnotami.

## Vývojový režim

Když chceš upravovat kód bez přebuildování kontejneru:

```bash
docker compose up -d influxdb
npm install
npm start
```

V `.env` musí být `INFLUX_URL=http://localhost:8086`.

Testy:

```bash
npm test
```

## Jak ověřit, že to jede

1. `docker compose ps` — obě služby `Up`, InfluxDB `healthy`
2. `curl localhost:3000/api/status` — `"ttnConnected": true` a `"influxOk": true`
3. Otevři dashboard a počkej na uplink (chodí po 10 minutách)
4. Zkus tlačítko **Zablokovat** a pak **Odblokovat** — stav dvířek se musí změnit

## API

| Metoda | Cesta | Co dělá |
|---|---|---|
| GET | `/api/status` | stav spojení a poslední měření |
| GET | `/api/history?hours=24` | historie napětí, 1 až 720 hodin |
| GET | `/api/commands` | seznam názvů příkazů |
| POST | `/api/command` | odešle downlink, tělo `{"command":"doorClose"}` |
| WS | `/ws` | živé zprávy `status`, `uplink`, `command` |

Názvy příkazů: `systemOn`, `systemOff`, `doorOpen`, `doorClose`, `block`, `unblock`.

## Když to nejede

**`ttnConnected` zůstává `false`** — špatný API klíč nebo `TTN_APP_ID`. Uživatelské jméno pro MQTT se skládá jako `<TTN_APP_ID>@<TTN_TENANT>`, u komunitní sítě je tenant `ttn`. Zkontroluj, že klíč má právo číst provoz aplikace.

**Dashboard je prázdný a historie hlásí chybu** — InfluxDB neběží, nebo `INFLUX_TOKEN` v `.env` nesouhlasí s tím, kterým se databáze zakládala. InfluxDB se inicializuje jen při **prvním** startu na prázdném svazku. Když jsi token měnil, smaž svazky a začni znovu:

```bash
docker compose down -v
docker compose up -d
```

**Příkaz projde, ale nic se nestane** — downlink čeká ve frontě, dokud kurník nepošle uplink. To je vlastnost LoRaWAN Class A, ne chyba. Počkej až 10 minut.

**Příkaz hlásí `cancel each other out`** — poslal jsi dvojici, která se vyruší (otevřít + zavřít). Firmware takovou kombinaci ignoruje, takže ji server odmítne dřív, než ji odešle.
