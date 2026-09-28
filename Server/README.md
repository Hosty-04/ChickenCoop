# Kurník

Automatická dvířka kurníku ovládaná přes internet. Kurník posílá každých 10 minut stav
baterie, solárního panelu a dvířek; z webové stránky je můžeš kdykoli otevřít nebo zavřít.

```
kurník ──LoRa──> brána ──internet──> The Things Network ──> tvůj server ──> stránka
```

Příprava zabere asi půl hodiny. Postupuj po krocích, každý navazuje na předchozí.

## Co budeš potřebovat

- **Kurník** s namontovanou elektronikou
- **Bránu** The Things Indoor Gateway a v domě Wi-Fi na 2,4 GHz
- **Počítač**, který běží nepřetržitě (stačí Raspberry Pi) s nainstalovaným Dockerem
- **Tři údaje k zařízení**, které dostaneš spolu s kurníkem:

| Údaj | Vypadá jako |
|---|---|
| DevEUI | `0080E115XXXXXXXX` |
| JoinEUI | `0101010101010101` |
| AppKey | 32 znaků, například `2B7E1516…` |

Bez nich se kurník k síti nepřipojí. Kdyby ses o ně připravil, najdeš v příloze, jak DevEUI vyčíst ze zařízení.

## 1. Účet v The Things Network

Síť The Things Network je pro tohle využití zdarma.

1. Zaregistruj se na <https://www.thethingsnetwork.org/>
2. Přihlas se do konzole na <https://eu1.cloud.thethings.network/console/>

Přihlas se do evropské konzole (`eu1`) — brána i kurník s ní počítají.

## 2. Brána

Na spodní straně brány je štítek se dvěma údaji: **Gateway EUI** a **WiFi PW**. Budeš potřebovat oba.

### Wi-Fi

1. Zapoj bránu do zásuvky
2. Podrž tlačítko **SETUP** asi 5 vteřin, dokud kontrolka nezačne rychle blikat
3. Brána si vytvoří vlastní Wi-Fi síť **MINIHUB-xxxxxx**; připoj se k ní z mobilu, heslo je **WiFi PW** ze štítku
4. V prohlížeči otevři **192.168.4.1**
5. U své domácí sítě klikni na **+**, zadej k ní heslo
6. Klikni na **Save and Reboot**

Kontrolka chvíli bliká zeleně, pak střídavě zeleně a červeně. Za minutu až dvě je hotovo.

> **Brána umí jen 2,4 GHz.** Síť, která vysílá pouze na 5 GHz, v seznamu vůbec neuvidí.
> Nefungují ani sítě s přihlašovací stránkou v prohlížeči a firemní sítě typu eduroam.

### Registrace v konzoli

V konzoli jdi na **Gateways** a zvol **Claim gateway** — *ne* Register gateway. Claim bránu zaregistruje sám a pošle jí přihlašovací údaje; bez něj se k síti nepřipojí.

| Pole | Hodnota |
|---|---|
| Gateway EUI | ze štítku |
| Claim authentication code | **WiFi PW** ze štítku |
| Frequency plan | Europe 863-870 MHz (SF9 for RX2) |

Do minuty má brána nahoře svítit **Connected**.

## 3. Aplikace

V konzoli **Applications → Add application**. Vyplň jen ID, například `kurnik`. Ostatní nech být.

## 4. Zařízení

V aplikaci **End devices → Register end device** a zvol **Enter end device specifics manually**.

| Pole | Hodnota |
|---|---|
| Frequency plan | Europe 863-870 MHz (SF9 for RX2) |
| LoRaWAN version | LoRaWAN Specification 1.0.4 |
| Regional Parameters version | RP002 Regional Parameters 1.0.4 |
| Activation mode | Over the air activation (OTAA) |
| Additional LoRaWAN class capabilities | None (class A only) |
| JoinEUI, DevEUI, AppKey | tvoje tři údaje |

> **AppKey si nenech vygenerovat** tlačítkem. Musí přesně odpovídat tomu, který dostaneš se zařízením, jinak kurník síť odmítne.

Zapni kurník. Během pár minut se v **Live data** objeví `Accept join-request` a pak první zpráva.

## 5. Dekódování dat

Bez tohoto kroku uvidíš místo napětí jen čísla v šestnáctkové soustavě.

V aplikaci **Payload formatters → Uplink**, zvol **Custom Javascript formatter** a vlož:

```js
function decodeUplink(input) {
  var b = input.bytes;
  if (input.fPort !== 2 || b.length < 2) {
    return { data: {}, errors: ["neocekavany port nebo delka"] };
  }
  var panel = b[0] >> 1;
  var batt = b[1] >> 2;
  return {
    data: {
      panel_mv: panel === 127 ? null : panel * 100,
      battery_mv: batt === 63 ? null : 5000 + batt * 50,
      battery_critical: (b[0] & 1) === 1,
      door: ["zavreno", "otevreno", "porucha", "neznamy"][b[1] & 3]
    },
    warnings: [], errors: []
  };
}
```

Ulož a počkej na další zprávu. Teď už uvidíš napětí a stav dvířek.

## 6. Server

Server přebírá zprávy z The Things Network, ukládá je a zobrazuje na stránce.

### Klíč pro přístup

V aplikaci **API keys → Add API key**. Zaškrtni práva:

- Read application traffic (uplink and downlink)
- Write downlink application traffic

Klíč začíná `NNSXS.` a **zobrazí se jen jednou** — hned si ho zkopíruj.

### Nastavení

```bash
cd Server
cp .env.example .env
```

Do souboru `.env` doplň pět hodnot:

| Proměnná | Co tam patří |
|---|---|
| `TTN_APP_ID` | ID aplikace z kroku 3 |
| `TTN_DEVICE_ID` | ID zařízení z kroku 4 |
| `TTN_API_KEY` | klíč, který jsi právě vytvořil |
| `INFLUX_TOKEN` | libovolný dlouhý náhodný řetězec, který si vymyslíš |
| `INFLUX_PASSWORD` | heslo do databáze, také si ho vymyslíš |

Náhodný řetězec ti vygeneruje:

```bash
openssl rand -hex 32
```

### Spuštění

```bash
docker compose up -d
```

Stránka běží na <http://localhost:3000>. Server se po restartu počítače spustí sám.

## Ověření

| Kde | Co má být vidět |
|---|---|
| Konzole, Gateways | brána **Connected** |
| Konzole, zařízení | zpráva každých 10 minut |
| `http://localhost:3000` | naměřená napětí a stav dvířek |

## Ovládání

Na stránce jsou tlačítka pro otevření a zavření dvířek, zablokování a vypnutí automatiky.

> **Příkaz se neprovede hned.** Kurník kvůli úspoře baterie poslouchá jen krátce po každé
> své zprávě, takže může trvat **až 10 minut**, než se dvířka pohnou. Není to porucha.
> Neklikej opakovaně — příkazy se řadí za sebe a provedou se všechny.

Za svítání a za soumraku se dvířka ovládají sama; ruční příkaz platí jen do nejbližší takové změny.

## Když to nejede

**Brána není Connected.** Zkontroluj, že tvoje Wi-Fi vysílá na 2,4 GHz. Pokud ano, podrž SETUP a nastav ji znovu. Ujisti se také, že jsi použil **Claim gateway**, ne Register gateway.

**Kurník se nepřipojí, v konzoli je `MIC mismatch`.** AppKey v konzoli nesouhlasí s tím v zařízení. Přepiš ho a kurník vypni a zapni.

**V konzoli jsou zprávy, ale stránka je prázdná.** Zkontroluj `TTN_APP_ID` a `TTN_API_KEY` v souboru `.env`, pak `docker compose restart`.

**Databáze hlásí chybu.** Pokud jsi po prvním spuštění měnil `INFLUX_TOKEN`, je potřeba databázi založit znovu:

```bash
docker compose down -v
docker compose up -d
```

**Dvířka hlásí poruchu.** Něco jim překáží, nebo nedojela do koncové polohy. Odstraň překážku a klikni na **Odblokovat**. Porucha se sama nezruší ani po vypnutí napájení.

## Příloha: jak zjistit DevEUI

Když DevEUI nemáš, dá se vyčíst přímo z čipu. Potřebuješ programátor ST-LINK a nástroj
STM32CubeProgrammer.

```bash
STM32_Programmer_CLI -c port=SWD -r32 0x1FFF7580 8
```

Výpis vypadá takto:

```
0x1FFF7580 : AABBCCDD 0080E115
```

DevEUI složíš tak, že **druhé číslo dáš před první**: `0080E115AABBCCDD`.
Druhé číslo musí být `0080E115` — podle toho poznáš, že čteš správné místo.
