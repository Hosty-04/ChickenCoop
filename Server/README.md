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
- **Raspberry Pi** a kartu microSD — stačí i to nejmenší, podrobnosti níž
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

> **AppKey musí přesně odpovídat tomu, který je v kurníku**, jinak se kurník k síti
> nepřipojí. Dostal jsi ho spolu se zařízením. Jestli si firmware překládáš sám,
> přečti si napřed následující odstavec.

Zapni kurník. Během pár minut se v **Live data** objeví `Accept join-request` a pak první zpráva.

### Vyměň si AppKey

AppKey je jediný klíč, ze kterého si kurník se sítí odvodí všechno ostatní. **Ten, který
je ve zdrojovém kódu tady na GitHubu, je veřejný** — přečte si ho kdokoliv.

Kdo ho má, dokáže odposlechnutý provoz rozšifrovat a hlavně **poslat kurníku vlastní
příkaz: otevřít dvířka, vypnout automatiku**. Musel by k tomu stát s vysílačkou v dosahu
kurníku, přes internet to nejde — ale souřadnice kurníku jsou v kódu taky. Než ho
pustíš naostro, vyměň klíč za vlastní:

1. V TTN u zařízení **AppKey → Generate** a klíč si zkopíruj. Je to 32 znaků.
2. V souboru `Master/LoRaWAN/App/se-identity.h` ho zapiš po dvojicích oddělených
   čárkami do `LORAWAN_GEN_APP_KEY` i `LORAWAN_APP_KEY` — obě mají stejnou hodnotu.
3. Přelož firmware a nahraj ho do kurníku.
4. **Upravený soubor už nikam nezveřejňuj.** Když si repozitář forkuješ, dej ho jako
   privátní.

Klíč, který si takhle vyrobíš, znáš jen ty a TTN. Vyměnit ho jde kdykoliv později,
jen po každé změně kurník znovu projde připojením k síti.

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
Musí běžet nepřetržitě — co zmešká, to je pryč.

### Jaké Pi

**Stačí i to nejlevnější.** Server si bere asi 90 MB paměti a data ukládá do jediného
souboru, takže se vejde na **Raspberry Pi Zero 2 W** s 512 MB. Spotřebuje kolem 1 W,
tedy asi 40 Kč elektřiny za rok. Silnější Pi 4 nebo 5 poslouží taky, jen stojí víc
a víc žerou; na Zero 2 W ale počítej s tím, že první sestavení serveru potrvá i deset
minut.

Systém může být 32bitový i 64bitový, na tom nezáleží. Potřebuješ jen kartu microSD,
8 GB bohatě stačí — měření za celý rok zabere asi 4 MB.

### Příprava karty

V **Raspberry Pi Imageru** vyber **Raspberry Pi OS Lite**. Pod ozubeným
kolečkem nastav:

- hostname `kurnik`
- zapnuté SSH
- uživatelské jméno a heslo
- síť Wi-Fi

Díky tomu nepotřebuješ monitor ani klávesnici.

### Docker

Přihlas se a nainstaluj:

```bash
ssh pi@kurnik.local
sudo apt update && sudo apt full-upgrade -y
curl -fsSL https://get.docker.com | sh
sudo usermod -aG docker $USER
```

**Odhlas se a přihlas znovu**, jinak bude Docker hlásit chybu oprávnění.

### Klíč pro přístup k síti

V konzoli TTN v aplikaci **API keys → Add API key**. Zaškrtni práva:

- Read application traffic (uplink and downlink)
- Write downlink application traffic

Klíč začíná `NNSXS.` a **zobrazí se jen jednou** — hned si ho zkopíruj.

### Nastavení

```bash
git clone https://github.com/Hosty-04/ChickenCoop.git
cd ChickenCoop/Server
cp .env.example .env
nano .env
```

Doplň čtyři hodnoty:

| Proměnná | Co tam patří |
|---|---|
| `AUTH_PASSWORD` | heslo, kterým se budeš přihlašovat ke stránce |
| `TTN_APP_ID` | ID aplikace z kroku 3 |
| `TTN_DEVICE_ID` | ID zařízení z kroku 4 |
| `TTN_API_KEY` | klíč, který jsi právě vytvořil |

Heslo si vymýšlíš ty. Dlouhé a náhodné vygeneruje `openssl rand -base64 18`.

> **Heslo ke stránce nepoužívej nikde jinde.** Je v souboru `.env` v čitelné podobě,
> stejně jako klíč k TTN. Kdo se dostane k tomu souboru, má stejně tak celý systém.

### Spuštění

```bash
docker compose up -d
```

Po restartu Pi se všechno spustí samo. Databáze se založí při prvním spuštění sama
a data přežijí i smazání a znovuvytvoření kontejnerů.

### Záloha

Celá historie měření je jeden soubor. Zkopíruješ si ho třeba do domovského adresáře:

```bash
docker compose cp app:/data/kurnik.db ~/kurnik-zaloha.db
```

Obnovíš ho opačným směrem, když server zastavíš (`docker compose down`), soubor
nakopíruješ zpět a server zase spustíš.

## 7. Přístup z mobilu

Na domácí Wi-Fi otevři **`http://kurnik.local:3000`**. Na iPhonu to funguje rovnou;
**na Androidu adresy s `.local` často nefungují** a musíš použít IP adresu. Zjistíš ji
na Pi příkazem `hostname -I`, vyjde něco jako `192.168.1.42`, takže zadáš
`http://192.168.1.42:3000`.

Aby se adresa neměnila, najdi v routeru **DHCP reservation** a přiřaď Raspberry Pi
napevno jednu adresu.

Přihlas se jménem `kurnik` a heslem z `.env`. Pak v prohlížeči dej **Přidat na plochu** —
vznikne ikona a stránka se otevře bez adresního řádku jako aplikace.

### Mimo domov

> **Nikdy neotevírej port 3000 do internetu.** I když je stránka chráněná heslem,
> vystavovat ji veřejně je zbytečné riziko — kdo se dostane dovnitř, otevře dvířka.

Použij **Tailscale**, který udělá šifrovaný tunel jen mezi tvými zařízeními:

```bash
curl -fsSL https://tailscale.com/install.sh | sh
sudo tailscale up
```

Nainstaluj Tailscale i do mobilu, přihlas se stejným účtem a v aplikaci uvidíš adresu
Pi. Tu pak zadáš s `:3000`. Funguje to z domova i z mobilních dat.

## Ověření

| Kde | Co má být vidět |
|---|---|
| Konzole, Gateways | brána **Connected** |
| Konzole, zařízení | zpráva každých 10 minut |
| Stránka | po přihlášení naměřená napětí a stav dvířek |

## Ovládání

Na stránce jsou tlačítka pro otevření a zavření dvířek, zablokování a vypnutí automatiky.

> **Příkaz se neprovede hned.** Kurník kvůli úspoře baterie poslouchá jen krátce po každé
> své zprávě, takže může trvat **až 10 minut**, než se dvířka pohnou. Není to porucha.
> Neklikej opakovaně — příkazy se řadí za sebe a provedou se všechny.

Pod tlačítky je řádek **Ve frontě** s příkazy, které ještě čekají na doručení. Tlačítkem
**Zrušit frontu** je smažeš — pokud to stihneš, než se kurník ozve, neprovede se nic.
Jakmile se příkaz doručí, stránka to napíše a z fronty zmizí.

> Frontu si server pamatuje jen dokud běží. Po jeho restartu se řádek ukáže prázdný, i když
> v síti něco čeká; **Zrušit frontu** ale vždy smaže vše, co v síti opravdu je, takže po
> restartu na něj klidně klikni, i když se nic nezobrazuje.

Za svítání a za soumraku se dvířka ovládají sama; ruční příkaz platí jen do nejbližší
takové změny.

## Když to nejede

**Brána není Connected.** Zkontroluj, že tvoje Wi-Fi vysílá na 2,4 GHz. Pokud ano, podrž
SETUP a nastav ji znovu. Ujisti se také, že jsi použil **Claim gateway**, ne Register gateway.

**Kurník se nepřipojí, v konzoli je `MIC mismatch`.** AppKey v konzoli nesouhlasí s tím
v zařízení. Přepiš ho a kurník vypni a zapni.

**Stránku nenajdeš.** Na Androidu zkus místo `kurnik.local` přímo IP adresu. Pokud prohlížeč
přepíná na HTTPS, vypni v něm „Vždy používat zabezpečené připojení".

**Přihlášení hlásí příliš mnoho pokusů.** Po pěti špatných heslech se přihlašování na 15 minut
zamkne. Buď počkej, nebo zámek zrušíš restartem: `docker compose restart app`.

**V konzoli jsou zprávy, ale stránka je prázdná.** Zkontroluj `TTN_APP_ID` a `TTN_API_KEY`
v souboru `.env`, pak `docker compose restart`.

**Databáze hlásí chybu.** Podívej se do výpisu `docker compose logs app`. Když je
soubor s daty poškozený (třeba po vytažení karty za běhu), obnov ho ze zálohy; když
žádnou nemáš, smaž ho a databáze se založí prázdná znovu:

```bash
docker compose down
docker volume rm server_coop-data
docker compose up -d
```

**Dvířka hlásí poruchu.** Něco jim překáží, nebo nedojela do koncové polohy. Odstraň
překážku a klikni na **Odblokovat**. Porucha se sama nezruší ani po vypnutí napájení.

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
