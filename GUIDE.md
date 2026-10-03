# Kurník

Automatická dvířka kurníku ovládaná přes internet. Kurník posílá každých 10 minut stav
baterie, solárního panelu a dvířek a každou hodinu počet vajec ve snáškových hnízdech;
z webové stránky lze dvířka kdykoli otevřít nebo zavřít.

```
kurník ──LoRa──> brána ──internet──> The Things Network ──> server na Pi ──> stránka
```

Příprava zabere asi hodinu. Kroky na sebe navazují, proto je vhodné dodržet jejich pořadí.

## Co je potřeba

- **Kurník** s namontovanou elektronikou
- **Brána** The Things Indoor Gateway a v domě Wi-Fi na 2,4 GHz
- **Raspberry Pi** a karta microSD — stačí i to nejmenší, podrobnosti níž
- **Tři údaje k zařízení**:

| Údaj | Vypadá jako | Odkud se bere |
|---|---|---|
| JoinEUI | `0101010101010101` | vždy tahle hodnota |
| DevEUI | `0080E115XXXXXXXX` | vyčte se z čipu, viz níž |
| AppKey | 32 znaků, například `2B7E1516…` | vygeneruje se, viz níž |

Bez nich se kurník k síti nepřipojí.

### DevEUI

Je to výrobní číslo čipu, nikde vytištěné není a musí se vyčíst přímo z něj. Potřeba je
programátor ST-LINK a nástroj STM32CubeProgrammer.

```bash
STM32_Programmer_CLI -c port=SWD -r32 0x1FFF7580 8
```

Výpis vypadá takto:

```
0x1FFF7580 : AABBCCDD 0080E115
```

DevEUI se složí tak, že **druhé číslo se dá před první**: `0080E115AABBCCDD`.
Druhé číslo musí být `0080E115` — podle toho se pozná, že jde o správné místo.

### AppKey

Tenhle klíč nikde vytištěný není a ani být nemůže: vymýšlí se a musí sedět na dvou místech
zároveň — v konzoli TTN a ve firmwaru kurníku. Když se liší, konzole hlásí `MIC mismatch`
a kurník se k síti nepřipojí.

Nejsnazší je nechat si ho vygenerovat. Při registraci zařízení (krok 4) je u pole **AppKey**
tlačítko **Generate**; objeví se 32 znaků, které si zkopírujte.

Do firmwaru patří do souboru `Master/LoRaWAN/App/se-identity.h`, po dvojicích oddělených
čárkami, a to do obou maker — mají stejnou hodnotu:

```c
#define LORAWAN_GEN_APP_KEY   2B,7E,15,16,28,AE,D2,A6,AB,F7,15,88,09,CF,4F,3C
#define LORAWAN_APP_KEY       2B,7E,15,16,28,AE,D2,A6,AB,F7,15,88,09,CF,4F,3C
```

Čárky doplní příkaz:

```bash
echo 2B7E151628AED2A6ABF7158809CF4F3C | sed 's/../&,/g; s/,$//'
```

Pak firmware přeložte, nahrajte do kurníku a kurník vypněte a zapněte.

> Klíč, který je ve zdrojovém kódu v tomhle repozitáři, si přečte kdokoliv. Pro ostré
> nasazení si vygenerujte vlastní a upravený soubor nezveřejňujte.

### Souřadnice kurníku

Dvířka se otevírají za svítání a zavírají za soumraku a kurník si oba časy počítá sám ze
zeměpisné polohy. Ve firmwaru je přednastavené místo z vývoje; vlastní souřadnice patří do
souboru `Master/Core/Inc/main.h`:

```c
#define COOP_LATITUDE   49.5170f
#define COOP_LONGITUDE  17.6181f
```

Kladná čísla znamenají sever a východ, což platí pro celou Českou republiku. Polohu zjistíte
v mapách kliknutím pravým tlačítkem na dané místo. Stačí čtyři desetinná místa, to je zhruba
deset metrů; chyba jednoho stupně směrem na východ nebo západ posune oba časy o čtyři minuty.

Po úpravě firmware znovu přeložte a nahrajte do kurníku.

> Kurník počítá se středoevropským časem včetně přechodu na letní čas. Mimo tohle pásmo by
> se musel upravit i soubor `Master/Core/Src/timebase.c`.

### Počet hnízd

Firmware je nastavený na dvě snášková hnízda. Jiný počet (1 až 15) patří do souboru
`Master/Core/Inc/nests.h`:

```c
#define NESTS_COUNT          2U
```

Stejné číslo pak dostane i server, viz `NEST_COUNT` v kapitole 6. Hnízda se číslují od
krabičky nejblíž hlavní krabičce: první má adresu 1, druhá 2 a tak dál, a tu samou adresu
musí mít nastavenou i řadič v dané krabičce.

## 1. Účet v The Things Network

Síť The Things Network je pro tohle využití zdarma.

1. Registrace na <https://www.thethingsnetwork.org/>
2. Přihlášení do konzole na <https://eu1.cloud.thethings.network/console/>

Je potřeba použít evropskou konzoli (`eu1`) — brána i kurník s ní počítají.

## 2. Brána

Na spodní straně brány je štítek se dvěma údaji: **Gateway EUI** a **WiFi PW**. Potřeba jsou oba.

### Wi-Fi

1. Zapojte bránu do zásuvky
2. Podržte tlačítko **SETUP** asi 5 vteřin, dokud kontrolka nezačne rychle blikat
3. Brána si vytvoří vlastní Wi-Fi síť **MINIHUB-xxxxxx**; připojte se k ní z mobilu, heslo je **WiFi PW** ze štítku
4. V prohlížeči otevřete **192.168.4.1**
5. U domácí sítě klikněte na **+** a zadejte k ní heslo
6. Klikněte na **Save and Reboot**

Kontrolka chvíli bliká zeleně, pak střídavě zeleně a červeně. To je v pořádku: brána už je
na Wi-Fi, ale ještě nemá, kam se přihlásit — účet jí dá až registrace v dalším kroku.

> **Brána umí jen 2,4 GHz.** Síť, která vysílá pouze na 5 GHz, se v seznamu vůbec neobjeví.
> Nefungují ani sítě s přihlašovací stránkou v prohlížeči a firemní sítě typu eduroam.

### Registrace v konzoli

V konzoli otevřete **Gateways** a zvolte **Claim gateway** — *ne* Register gateway. Claim bránu zaregistruje sám a pošle jí přihlašovací údaje; bez něj se k síti nepřipojí.

| Pole | Hodnota |
|---|---|
| Gateway EUI | ze štítku |
| Claim authentication code | **WiFi PW** ze štítku |
| Frequency plan | Europe 863-870 MHz (SF9 for RX2) |

Do minuty má brána nahoře svítit **Connected**.

## 3. Aplikace

V konzoli **Applications → Add application**. Vyplňte jen ID, například `kurnik`. Ostatní pole nechte být.

## 4. Zařízení

V aplikaci **End devices → Register end device** a zvolte **Enter end device specifics manually**.

| Pole | Hodnota |
|---|---|
| Frequency plan | Europe 863-870 MHz (SF9 for RX2) |
| LoRaWAN version | LoRaWAN Specification 1.0.4 |
| Regional Parameters version | RP002 Regional Parameters 1.0.4 |
| Activation mode | Over the air activation (OTAA) |
| Additional LoRaWAN class capabilities | None (class A only) |
| JoinEUI, DevEUI | dva údaje z úvodu |
| AppKey | tlačítkem **Generate**, pak ho zapište do firmwaru |

Zapněte kurník. Během pár minut se na kartě **Live data** (v konzoli u zařízení) objeví
`Accept join-request` a pak první zpráva.

## 5. Dekódování dat

Bez tohoto kroku jsou místo napětí vidět jen čísla v hexadecimální soustavě.

V aplikaci **Payload formatters → Uplink** zvolte **Custom Javascript formatter** a vložte:

```js
function decodeUplink(input) {
  var b = input.bytes;
  if (input.fPort !== 2 || b.length < 2) {
    return { data: {}, errors: ["neocekavany port nebo delka"] };
  }
  var panel = b[0] >> 1;
  var batt = b[1] >> 2;
  var data = {
    panel_mv: panel === 127 ? null : panel * 100,
    battery_mv: batt === 63 ? null : 5000 + batt * 50,
    battery_critical: (b[0] & 1) === 1,
    door: ["zavreno", "otevreno", "porucha", "neznamy"][b[1] & 3]
  };
  if (b.length > 2) {
    data.hnizda = [];
    for (var i = 2; i < b.length; i++) {
      data.hnizda.push(hnizdo(b[i] >> 4), hnizdo(b[i] & 15));
    }
  }
  return { data: data, warnings: [], errors: [] };
}

function hnizdo(kod) {
  if (kod <= 10) return kod;
  return { 11: "kvocna", 12: "nekalibrovano", 14: "porucha" }[kod] || null;
}
```

Uložte a počkejte na další zprávu — kurník se ozývá po deseti minutách. Pak už jsou
v **Live data** místo šestnáctkových čísel vidět napětí a stav dvířek. Jednou za hodinu
přibude seznam `hnizda` s počtem vajec v každém hnízdě; `null` znamená hnízdo, které se
neozvalo. Při lichém počtu hnízd je `null` vždycky i na konci seznamu — to jen dorovnává
poslední bajt zprávy.

## 6. Server

Server přebírá zprávy z The Things Network, ukládá je a zobrazuje na stránce.
Musí běžet nepřetržitě — co zmešká, to je pryč.

### Jaké Pi

**Stačí i to nejlevnější.** Server si bere asi 90 MB paměti a data ukládá do jediného
souboru, takže se vejde na **Raspberry Pi Zero 2 W** s 512 MB. Spotřebuje kolem 1 W,
tedy asi 40 Kč elektřiny za rok. Silnější Pi 4 nebo 5 poslouží také, jen stojí víc
a víc žerou; u Zero 2 W je naopak potřeba počítat s tím, že první sestavení serveru
potrvá i deset minut.

Systém může být 32bitový i 64bitový, na tom nezáleží. Kromě Pi stačí karta microSD,
8 GB bohatě vystačí — měření za celý rok zabere asi 4 MB.

### Příprava karty

V **Raspberry Pi Imageru** zvolte **Raspberry Pi OS Lite**. Pod ozubeným
kolečkem nastavte:

- hostname `kurnik`
- zapnuté SSH
- uživatelské jméno a heslo
- síť Wi-Fi

Díky tomu není potřeba monitor ani klávesnice.

### Docker

Přihlaste se a nainstalujte:

```bash
ssh uzivatel@kurnik.local
sudo apt update && sudo apt full-upgrade -y
sudo apt install -y git
curl -fsSL https://get.docker.com | sh
sudo usermod -aG docker $USER
```

Místo `uzivatel` patří jméno zadané v Imageru.

**Pak se odhlaste a přihlaste znovu**, jinak bude Docker hlásit chybu oprávnění.

### Klíč pro přístup k síti

V konzoli TTN v aplikaci **API keys → Add API key**. Klíč pojmenujte, zvolte **Grant
individual rights** a zaškrtněte:

- Read application traffic (uplink and downlink)
- Write downlink application traffic

Po uložení se klíč začínající `NNSXS.` **zobrazí jen jednou** — zkopírujte si ho hned.

### Nastavení

```bash
git clone https://github.com/Hosty-04/ChickenCoop.git
cd ChickenCoop/Server
cp .env.example .env
nano .env
```

Doplňte čtyři hodnoty:

| Proměnná | Co tam patří |
|---|---|
| `AUTH_PASSWORD` | heslo pro přihlášení ke stránce |
| `TTN_APP_ID` | ID aplikace z kroku 3 |
| `TTN_DEVICE_ID` | ID zařízení z kroku 4 |
| `TTN_API_KEY` | klíč vytvořený v předchozím kroku |

Heslo si zvolte sami; dlouhé a náhodné vygeneruje příkaz `openssl rand -base64 18`.

Kurník s jiným počtem hnízd než dvěma potřebuje ještě řádek `NEST_COUNT` se stejným číslem,
jaké je ve firmwaru (kapitola **Počet hnízd** v úvodu). Server podle něj ví, kolik hnízd
na stránce ukázat a jak rozložit čísla ve zprávě.

Řádek `TZ=Europe/Prague` nechte být, pokud kurník nestojí v jiném časovém pásmu. Podle něj
server dělí měření na dny, takže v grafu začíná den o půlnoci u vás, ne v Londýně.

> **Heslo ke stránce nepoužívejte nikde jinde.** Je v souboru `.env` v čitelné podobě,
> stejně jako klíč k TTN. Kdo se dostane k tomu souboru, má stejně tak celý systém.

### Spuštění

```bash
docker compose up -d
```

**Poprvé to chvíli trvá** — server se musí sestavit, na Zero 2 W klidně deset minut.
Že se rozběhl, ukáže výpis:

```bash
docker compose logs -f app
```

Mají se objevit řádky `dashboard on http://localhost:3000` a `TTN connected`. Sledování
ukončíte klávesami Ctrl+C, server běží dál.

Databáze se založí při prvním spuštění. Po restartu Pi se všechno spustí samo.

### Kde jsou data

Naměřené hodnoty neleží uvnitř kontejneru, ale v odděleném úložišti `server_coop-data`
na kartě Pi. Proto přežijí:

- restart serveru i celého Pi, včetně výpadku proudu
- `docker compose down` a nové spuštění
- aktualizaci a nové sestavení serveru
- smazání obrazu, třeba příkazem `docker image prune`

Přijít se o ně dá jen dvěma způsoby: příkazem `docker compose down -v` (to `-v` smaže
i úložiště) nebo `docker volume rm server_coop-data`. Oboje je nevratné, takže se hodí
mít zálohu.

> Při náhlém výpadku proudu může chybět poslední měření nebo dvě, která ještě nebyla
> zapsaná na kartu. Databáze jako taková zůstane v pořádku.

### Běžný provoz

Všechny příkazy se spouštějí ze složky `ChickenCoop/Server`.

| Co | Příkaz |
|---|---|
| Zjistit, jestli server běží | `docker compose ps` |
| Sledovat výpis | `docker compose logs -f app` |
| Zastavit | `docker compose down` |
| Spustit | `docker compose up -d` |
| Změnit heslo nebo klíče | upravit `.env`, pak `docker compose up -d` |

> Po úpravě `.env` nestačí `docker compose restart` — ten kontejner jen restartuje
> s původním nastavením. Nové hodnoty načte až `docker compose up -d`.

### Aktualizace

```bash
cd ~/ChickenCoop
git pull
cd Server
docker compose up -d --build
```

**Bez `--build`** by Docker spustil dřív sestavenou verzi a změny by se neprojevily.
Naměřená data zůstanou, ta jsou mimo kontejner.

### Záloha

Celá historie měření je jeden soubor. Server je při kopírování potřeba zastavit, jinak
může část posledních měření zůstat v pomocném souboru a do zálohy se nedostane:

```bash
docker compose stop
docker compose cp app:/data/kurnik.db ~/kurnik-zaloha.db
docker compose start
```

Obnova je totéž obráceně:

```bash
docker compose stop
docker compose cp ~/kurnik-zaloha.db app:/data/kurnik.db
docker compose start
```

Kopíruje se vždy celý soubor a uloží se pod jménem z cílové cesty: ze `kurnik-zaloha.db` se
uvnitř stane `kurnik.db` a původní databázi přepíše.

Zastavení trvá pár vteřin. Kdyby kurník zrovna v tu chvíli poslal zprávu, přijde se o ni —
další dorazí za deset minut.

### Obnova po výpadku proudu

Server si za běhu vedle `kurnik.db` drží ještě `kurnik.db-wal` a `kurnik.db-shm`. Při čistém
zastavení se jejich obsah zapíše do `kurnik.db` a samy zmizí, ale po výpadku proudu nebo po
vypnutí Pi natvrdo tam zůstanou. Kdyby je potom našla vedle sebe nově nakopírovaná databáze,
SQLite je na ni přehraje a poškodí ji. **Proto se v takovém případě před obnovou podívejte,
co v úložišti leží.**

Se zastaveným serverem vypište jeho obsah:

```bash
docker run --rm -v server_coop-data:/data alpine ls -l /data
```

Je tam jen `kurnik.db`? Kopírujte zálohu podle postupu výše. Je tam i `-wal` nebo `-shm`?
Nejdřív je smažte:

```bash
docker run --rm -v server_coop-data:/data alpine rm -f /data/kurnik.db-wal /data/kurnik.db-shm
```

Po běžném zastavení příkazem `docker compose stop` tam nic takového nebude a obnova se dá
spustit rovnou.

## 7. Přístup ke stránce

Stránka je dostupná z čehokoliv v domácí síti — z počítače i z telefonu. Nejdřív je
potřeba znát adresu Pi, zbytek se pak liší jen drobnostmi podle zařízení.

> **Kdo si nastaví Tailscale, vystačí si s ním i doma** a zbytek téhle kapitoly může
> přeskočit. Jeho adresa odpovídá na domácí Wi-Fi stejně jako z mobilních dat, takže stačí
> jediná a nemusí se nic přepínat. Domácí adresa má proti tomu tu výhodu, že nevyžaduje
> žádný účet ani aplikaci.

### Adresa Pi

Stránka běží na Pi, takže se na ni chodí přes jeho adresu. Napsat jde dvěma způsoby.

**Jméno `kurnik.local`** je pohodlnější, protože si není co pamatovat. Funguje na Windows,
macOS, běžných linuxových distribucích i na iPhonu; na Linuxu k tomu může chybět služba,
kterou doinstaluje `sudo apt install avahi-daemon`. Na Androidu je to nejisté — novější
verze systému jméno přeloží, starší ne.

**IP adresa** funguje všude. Vypíše ji na Pi příkaz `hostname -I`, vyjde například
`192.168.1.42`; druhá možnost je najít `kurnik` v seznamu připojených zařízení v routeru.
Po výpadku proudu se ale může změnit, a tím přestane fungovat i uložená ikona. Proto ji
zafixujte. Postup je u všech routerů stejný, liší se jen pojmenování:

1. Otevřete nastavení routeru. Jeho adresu vypíše na Pi příkaz `ip route | grep default`,
   bývá to `192.168.1.1` nebo `192.168.0.1`. Přihlašovací údaje jsou obvykle na štítku
   routeru.
2. Najděte oddíl **LAN** nebo **DHCP**. Hledaná položka se jmenuje **DHCP reservation**,
   **Address reservation**, **Static lease** nebo česky **rezervace adres**.
3. Ze seznamu připojených zařízení vyberte `kurnik`. Když seznam není, zadejte MAC adresu
   ručně — vypíše ji `ip link show` na Pi, řádek `link/ether` u `wlan0` pro Wi-Fi nebo
   u `eth0` pro kabel.
4. Uložte a Pi restartujte. Že se adresa opravdu ujala, ověří `hostname -I`.

**Když se do routeru nedostanete** — u krabic od poskytovatele bývá správcovský účet
zamčený — dá se adresa zafixovat přímo na Pi. Nejdřív si na něm zjistěte tři údaje:

```bash
hostname -I               # současná adresa Pi, třeba 192.168.1.42
ip route | grep default   # adresa routeru, je za slovem via
nmcli con show            # název připojení, na Raspberry Pi OS bývá preconfigured
```

Ty tři hodnoty se dosadí do následujícího příkazu. `preconfigured` je název připojení,
`192.168.1.42` adresa, kterou má Pi napevno dostat, a `192.168.1.1` router — ten slouží
zároveň jako DNS:

```bash
sudo nmcli con mod preconfigured ipv4.method manual \
  ipv4.addresses 192.168.1.42/24 \
  ipv4.gateway 192.168.1.1 \
  ipv4.dns 192.168.1.1
sudo reboot
```

`/24` na konci adresy říká, že první tři čísla jsou společná pro celou domácí síť. U sítí
začínajících `192.168.` to platí prakticky vždy.

> **Vybraná adresa musí být volná a volná i zůstat.** Router rozdává adresy z nějakého
> rozsahu, obvykle od `.100` do `.200`; kdyby ta vaše byla uvnitř, mohl by ji časem přidělit
> i jinému zařízení a obě by se o ni praly. Bezpečné je proto nízké číslo, třeba `.42`.
> Že je volné, ověříte z jiného počítače v síti — Pi zatím vypněte a zkuste
> `ping 192.168.1.42`. Nesmí odpovídat nikdo.

Některé routery umí k IP adrese přiřadit i vlastní jméno; bývá to hned vedle rezervace adres,
případně v oddílu **DNS**. Když se tam `kurnik` přiřadí k adrese Pi, funguje pak na všech
zařízeních v domácí síti adresa `http://kurnik:3000` a IP adresu si nikdo pamatovat nemusí.
Umí to ale jen některé routery; u většiny běžných tahle volba chybí.

### Doma z počítače

Otevřete **`http://kurnik.local:3000`**, nebo s IP adresou `http://192.168.1.42:3000`.
Pro rychlé spuštění si udělejte záložku nebo zástupce na ploše.

### Doma z iPhonu

Stejně jako na počítači: **`http://kurnik.local:3000`**, jméno si telefon přeloží sám.
Po přihlášení zvolte **Sdílet → Přidat na plochu** a vznikne ikona, ze které se stránka
otevře bez adresního řádku jako aplikace.

### Doma z Androidu

Protože jméno `kurnik.local` tu nemusí fungovat, počítejte s IP adresou:

1. **Zadejte celou adresu včetně `http://`**, tedy `http://192.168.1.42:3000`. Bez toho ji
   prohlížeč pošle do vyhledávače.
2. **Kdyby prohlížeč přepnul na `https://`** a stránka nenaběhla, vypněte vynucování:
   v Chrome **⋮ → Nastavení → Soukromí a zabezpečení → Vždy používat zabezpečená připojení**,
   ve Firefoxu **⋮ → Nastavení → Soukromí a zabezpečení → Režim pouze HTTPS**.
3. Přihlaste se jménem `kurnik` a heslem z `.env`, pak v nabídce zvolte **Přidat na plochu**.
   Tuhle volbu má Chrome i Firefox, jen ji každý řadí jinam. Vznikne ikona, která stránku
   otevře na jedno klepnutí.

> Ikona na Androidu otevře stránku v prohlížeči i s adresním řádkem. Aby se otevírala
> samostatně jako aplikace, musela by stránka běžet přes HTTPS — po domácím HTTP to žádný
> prohlížeč nenabídne. Na funkci to nemá vliv.

> **Dotaz na přístup k místní síti povolte.** Ptá se Chrome i Firefox, na Androidu 16
> a novějším i samotný systém. Je to ochrana proti stránkám z internetu, které by jinak
> mohly prohledávat domácí síť; tahle stránka běží přímo na Pi a nic dalšího nehledá.

### Mimo domov

> **Nikdy neotevírejte port 3000 do internetu.** I když je stránka chráněná heslem,
> vystavovat ji veřejně je zbytečné riziko — kdo se dostane dovnitř, otevře dvířka.
> Heslo navíc po holém HTTP cestuje nešifrovaně.

Řešením je **Tailscale**. Vytvoří privátní síť jen z vlastních zařízení: každé dostane
adresu `100.x.y.z`, kterou nikdo jiný nevidí, a provoz mezi nimi je šifrovaný. Do internetu
se nic neotevírá a není potřeba veřejná IP adresa, takže to funguje i na připojeních, kde
přesměrování portů vůbec nejde. Pro osobní použití je zdarma.

#### Na Raspberry Pi

```bash
curl -fsSL https://tailscale.com/install.sh | sh
sudo tailscale up
```

Druhý příkaz vypíše odkaz. Otevřete ho v prohlížeči a přihlaste se — účtem Google, GitHub,
Microsoft nebo e-mailem. Adresu, kterou Pi v téhle síti dostalo, pak vypíše:

```bash
tailscale ip -4
```

> **Vypněte vypršení klíče.** Tailscale po 180 dnech vyžaduje nové přihlášení a server by
> ze sítě tiše vypadl. Na <https://login.tailscale.com> u zařízení `kurnik` zvolte
> **Disable key expiry**. U serveru, který má běžet pořád, je to důležité.

#### Na ostatních zařízeních

Do telefonu nainstalujte aplikaci **Tailscale** z Obchodu Play nebo App Store, do počítače
program ze stránek <https://tailscale.com/download> (Windows, macOS i Linux). Všude se
přihlaste **stejným účtem** a zapněte připojení. V seznamu zařízení se objeví `kurnik`
i se svou adresou.

Stránku pak otevřete na `http://100.x.y.z:3000`. Když v konzoli Tailscale zapnete
**MagicDNS**, stačí kratší `http://kurnik:3000`.

**Ikonu si udělejte právě z téhle adresy.** Funguje doma i z mobilních dat
a z cizí sítě, takže stačí jediná — na rozdíl od domácí adresy, která mimo domov neodpoví.

Aplikace drží připojení zapnuté; vypnout jde přepínačem, ale pak stránka mimo domov
nenaběhne. Tunelem prochází jen provoz na vlastní zařízení, běžné prohlížení internetu jde
mimo něj.

## Ověření

| Kde | Co má být vidět |
|---|---|
| Konzole, Gateways | brána **Connected** |
| Konzole, zařízení | zpráva každých 10 minut |
| Pi, `docker compose logs app` | `TTN connected` a každých 10 minut řádek `uplink`, jednou za hodinu s `nests=` |
| Stránka | po přihlášení naměřená napětí, stav dvířek a vejce v hnízdech |

## Co je na stránce

Nahoře jsou čtyři dlaždice: napětí baterie, napětí solárního panelu, stav dvířek a čas
poslední zprávy. Pod nimi karta **Hnízda**, histogram **Snáška** a graf **Napájení** s napětím
baterie a panelu. Oba grafy se řídí volbou rozsahu nahoře — 6 hodin, 24 hodin, 7 dní,
30 dní, rok, nebo **Vše** od úplně prvního měření. Tlačítkem **Tabulka** se každý z nich
přepne na stejná data v číslech.

Čím delší rozsah, tím hrubší průměr: do dne po deseti minutách, do týdne po hodině, do
měsíce po šesti hodinách, do roku po dnech a dál po týdnech. Každý pohled tak má řádově
stovku bodů, ať pokrývá den nebo pět let.

U panelu se do průměru počítají jen hodnoty ze dne. V noci panel nedává nic a tyhle nuly by
průměr srazily na zlomek skutečnosti — u ročního pohledu, kde je jeden bod celý den, by graf
ukazoval napětí, jaké panel nikdy neměl. Kde ale do jednoho bodu padne celá noc a nic jiného,
zůstává nula, takže u krátkých rozsahů jsou noci v grafu dál vidět. Baterie se průměruje celá,
té napětí drží i v noci. Stránka na to pod grafem upozorňuje, a to u rozsahů od 7 dní — do
24 hodin je každý bod jedno desetiminutové měření a nic se neprůměruje.

Histogram **Snáška** ukazuje, kolik vajec ve všech hnízdech přibylo: do 24 hodin po hodinách,
do měsíce po dnech a dál po týdnech. Osa začíná a končí na celé hodině, dni nebo týdnu,
takže 24 hodin je třeba od 18:00 do 18:00. Kontrola v celou hodinu hlásí vejce snesená za
uplynulou hodinu, proto se kontrola v 10:00 započítá do sloupce 9:00–10:00. Právě běžící
hodina se tak neukazuje vůbec: v 15:30 končí 24 hodin v 15:00 a sloupec 15:00–16:00
přibude po kontrole v 16:00. Dnešek a tento týden jsou ve sloupcích po dnech a týdnech vidět
od první kontroly a doplňují se průběžně. Po najetí myší je
vidět, ve kterých hnízdech vejce přibyla. Počítá se jen přírůstek — když vejce sesbíráte,
počet v hnízdě klesne, ale snáška zůstane.

Pod ovládáním je karta **Data** s počtem uložených měření a tlačítkem pro smazání historie.

Vpravo nahoře svítí indikátor spojení serveru s The Things Network; vedle něj na počítači
a pod ním na telefonu je přepínač světlého a tmavého motivu a odhlášení. Když se stránka
se serverem přestane spojovat, ukáže indikátor nejdřív **připojuji…** a teprve po pěti
sekundách bez spojení **Server nedostupný**, takže krátké přerušení, třeba při znovunačtení
stránky, se neprojeví.

Nové hodnoty se doplňují samy, stránku není potřeba načítat znovu.

### Hnízda

Každé hnízdo má svůj zásobník s deseti místy, která se plní tak, jak přibývají vejce. Nad ním
je jejich počet, pod ním, kolik jich v hnízdě dnes přibylo. Kurník hnízda kontroluje každou
celou hodinu a hned po nich změří baterii a panel, takže obojí dorazí v jedné zprávě. Čas
poslední kontroly je v záhlaví karty i se součtem za celý den. Po vynulování nebo kalibraci
váhy přijde zpráva o hnízdech i mimo celou hodinu; hnízda, kterých se příkaz netýkal, v ní
mají stav z poslední kontroly.

Na telefonu je každé hnízdo jeden řádek: vlevo název a počet, vpravo zásobník a pod ním
poznámka. I patnáct hnízd se tak vejde zhruba na jednu obrazovku.

| Poznámka | Co znamená |
|---|---|
| **⚠ košík je plný** | v hnízdě je deset vajec, víc se jich do zprávy nevejde — je čas je vybrat |
| **⚠ sedí kvočna** | váha tři kontroly po sobě ukázala slepici; počet je z doby, kdy hnízdo bylo volné |
| **⚠ váha není zkalibrovaná** | váhu je potřeba vynulovat a zkalibrovat, viz **Ovládání** |
| **⚠ porucha váhy** | převodník u tenzometru neodpovídá nebo měří nesmysly |
| **⚠ hnízdo neodpovídá** | řadič v krabičce hnízda se neozval; počet je z poslední úspěšné kontroly |

Hnízda jsou napájená jedno od druhého, takže když se neozve první, neozvou se ani ta za ním.

Při kriticky vybité baterii a při vypnuté automatice kurník hnízda nekontroluje. Karta pak
ukazuje poslední kontrolu i s jejím datem.

### Když data chybí

Kurník se ozývá každých 10 minut. Když zpráva nedorazí, neuloží se nic: **v grafu vznikne
díra široká jako výpadek** a v tabulce prostě chybí řádky, takže se výpadek pozná skokem
v časovém sloupci. Jedna ztracená zpráva graf nepřeruší — čára se spojí, dokud mezera
nepřeroste dvě okna zvoleného rozsahu. V grafu je proto výpadek vidět zhruba od 20 minut
ve 24 hodinách, 2 hodin v 7 dnech, 12 hodin ve 30 dnech, 2 dnů v roce a 2 týdnů ve Vše.
V tabulce zmizí řádek, jakmile je celé okno bez měření — v rozsazích do 24 hodin tedy i po
jediné ztracené zprávě.

Zpráva může dorazit i bez některé hodnoty, když se čidlo neozve nebo naměří nesmysl
(baterie mimo rozsah 1–10 V, panel nad 12,5 V). Dlaždice pak ukáže pomlčku a oranžové
**⚠ čidlo neodpovídá**, v tabulce je pomlčka v tom sloupci a v grafu se přeruší jen ta jedna
čára — druhá pokračuje dál. V delších rozsazích taková hodnota zmizí beze stopy, protože se
do průměru okna nepočítá.

**Odpojený, zastíněný nebo rozbitý panel** čidlo hlásit umí: naměří 0,00 V. Denní hrb
v grafu proto klesne na nulu a v tabulce je `0.00 V`, ne pomlčka. Pomlčka u panelu znamená,
že selhalo samotné měření, ne že panel nic nedává.

**Odpojená baterie** vypadá jinak, protože z ní běží celý kurník: přestanou chodit zprávy
a na stránce je to obyčejný výpadek. Pomlčka u baterie znamená, že kurník běží dál, jen se
neozval senzor INA226; v takovém případě kurník pro jistotu odpojí i panel.

Poznámka **na horní mezi rozsahu** se objeví, když hodnota dojede na konec toho, co se vejde
do zprávy — 8,00 V u baterie a 12,50 V u panelu. Skutečné napětí může být vyšší, dlaždice,
graf i tabulka ukazují tuhle mez.

**Hnízda a Snáška.** Počty vajec chodí jen se zprávou po kontrole hnízd. Když nedorazí, karta
Hnízda zůstane u poslední kontroly a její čas je v záhlaví karty. V tabulce Snášky má takový
interval **pomlčku**, v grafu prázdné místo a po najetí myší popisek **bez dat**. Pomlčka
znamená, že za tu dobu kurník žádný počet nenahlásil; **0** znamená, že kontrola proběhla
a vejce nepřibyla. Ve sloupcích po dnech a týdnech je pomlčka, jen když za celý den nebo
týden nepřišel žádný počet.

Pomlčka v celém řádku nastane, když zprávy z kurníku nedorazí, a při kriticky vybité baterii
nebo vypnuté automatice, kdy se hnízda nekontrolují. Pomlčka jen u jednoho hnízda znamená, že
to hnízdo nemělo platný počet — sedí na něm kvočna, váha není zkalibrovaná, má poruchu nebo
neodpovídá. Sloupec **Celkem** pak sčítá jen hnízda, o kterých se ví, a v popisku grafu jsou
ostatní vyjmenovaná za **bez dat**.

Vejce snesená během výpadku se neztratí: první kontrola po něm porovná počet s posledním
známým a celý přírůstek připíše svému intervalu, takže po výpadku bývá jeden vyšší sloupec.
Stejně se projeví vejce, která přibyla, zatímco na hnízdě seděla kvočna nebo hnízdo
neodpovídalo. Přijde se jen o vejce, která se během výpadku stihla sesbírat, a o vejce nad
deset v plném košíku, protože víc se do zprávy nevejde.

## Ovládání

Úplně dole jsou tlačítka pro otevření a zavření dvířek, zablokování a vypnutí automatiky
a pro nastavení váhy v hnízdech. Vypnutá automatika zastaví dvířka i kontrolu hnízd.

> **Příkaz se neprovede hned.** Kurník kvůli úspoře baterie poslouchá jen krátce po každé
> své zprávě, takže může trvat **až 10 minut**, než se dvířka pohnou. Není to porucha.
> Opakované klikání nepomůže — stejný příkaz se do fronty zařadí jen jednou a stránka další
> kliknutí odmítne s upozorněním. Různé příkazy se řadí za sebe a provedou se všechny.

Pod tlačítky je řádek **Ve frontě** s příkazy, které ještě čekají na doručení. Tlačítko
**Zrušit** je smaže — pokud se to stihne dřív, než se kurník ozve, neprovede se nic.
Jakmile se příkaz doručí, stránka to oznámí a z fronty zmizí.

> Frontu si server pamatuje jen dokud běží. Po jeho restartu se řádek ukáže prázdný, i když
> v síti něco čeká, a stejný příkaz jde zařadit znovu; **Zrušit** ale vždy smaže vše, co v síti opravdu je, takže po
> restartu má smysl na něj kliknout, i když se nic nezobrazuje. Restart serveru také
> odhlásí všechna otevřená okna a stránka se sama vrátí na přihlášení.

Otevřených oken může být víc — třeba telefon a počítač zároveň. Co uděláte v jednom, se
hned ukáže i v ostatních: zařazený příkaz, zrušená fronta, nově naměřené hodnoty i smazání
historie.

Za svítání a za soumraku se dvířka ovládají sama; ruční příkaz platí jen do nejbližší
takové změny.

### Kalibrace váhy

Každé hnízdo váží vlastní tenzometr a ten je potřeba po montáži nastavit — dokud se to
nestane, hlásí hnízdo **⚠ váha není zkalibrovaná**. Stačí k tomu závaží o hmotnosti přesně
1 kg; jinou hmotnost by bylo potřeba změnit ve firmwaru (`NESTS_CALIB_MASS_G` v souboru
`Master/Core/Inc/nests.h`).

1. Z hnízd vyndejte všechna vejce a podestýlku nechte, jak bude normálně.
2. V **Ovládání** klikněte pod **Váha hnízd** na **Vyberte hnízda**. Otevře se okno se
   seznamem; zaškrtněte hnízda, která chcete nastavit, a klikněte na **Potvrdit**. Tlačítko
   pak ukazuje, co je vybrané (třeba **Hnízdo 2** nebo **Všechna hnízda**), a teprve teď
   jde kliknout na **Vynulovat**. Výběr zůstává, dokud ho nezměníte.
3. Počkejte, až stránka ohlásí **Kurník příkaz přijal** a karta Hnízda ukáže novou
   kontrolu. Kurník zaškrtnutá hnízda vynuluje hned po přijetí příkazu, nečeká na celou
   hodinu; ostatní hnízda přitom neměří.
4. Do každého z nich položte doprostřed závaží a klikněte na **Kalibrovat**. Kdo má jen
   jedno závaží, kalibruje hnízda po jednom.
5. Až se karta Hnízda znovu obnoví, závaží sundejte.

Každý z obou kroků může trvat až deset minut, protože příkaz čeká, až se kurník ozve.
Jedním příkazem jde nastavit libovolný počet hnízd; víc příkazů ve frontě se ale do kurníku
dostává po jednom, s každou jeho zprávou, tedy zhruba po deseti minutách.
Hnízdo během kalibrace nesmí obsadit slepice — nejlepší je kalibrovat večer, kdy jsou
slepice zavřené na hřadu.

### Smazání historie

Tlačítko **Smazat** na kartě Data vyhodí všechna uložená měření. Než se to stane,
stránka se zeptá na heslo — totéž, kterým se přihlašujete; samotné přihlášení k tomuhle
kroku nestačí. Když není co mazat, tlačítko je neaktivní.

> **Nedá se to vrátit.** Dlaždice se vyprázdní, graf zůstane bez dat a vrátit je jde jen
> ze zálohy. Zálohování je popsané v kapitole 6.

Kurníku se to nijak nedotkne: posílá dál a první zpráva po smazání se zase uloží.

## Když to nejede

**Brána není Connected.** Zkontrolujte, že domácí Wi-Fi vysílá na 2,4 GHz. Pokud ano, podržte
SETUP a nastavte ji znovu. Ujistěte se také, že byl použit **Claim gateway**, ne Register gateway.

**Kurník se nepřipojí, v konzoli je `MIC mismatch`.** AppKey v konzoli nesouhlasí s tím
v kurníku. Opravte ho u zařízení v **General settings → Join settings** a kurník vypněte a zapněte.

**Stránku nelze najít.** Zkuste místo `kurnik.local` přímo IP adresu Pi (kapitola 7,
**Adresa Pi**). Pokud prohlížeč přepíná na HTTPS, vypněte v něm „Vždy používat zabezpečené
připojení". A ověřte, že zařízení je na stejné Wi-Fi jako Pi — ze sítě pro hosty nebo
z mobilních dat se na něj nedostanete.

**Přihlášení hlásí příliš mnoho pokusů.** Po pěti špatných heslech se přihlašování na 15 minut
zamkne. Buď je potřeba počkat, nebo zámek zruší restart: `docker compose restart app`.

**V konzoli jsou zprávy, ale stránka je prázdná.** Zkontrolujte `TTN_APP_ID`, `TTN_DEVICE_ID`
a `TTN_API_KEY` v souboru `.env`, pak `docker compose up -d`. Ve výpisu `docker compose logs app`
musí být `TTN connected`.

**Databáze hlásí chybu.** Podívejte se do výpisu `docker compose logs app`. Když je
soubor s daty poškozený (třeba po vytažení karty za běhu), obnovte ho ze zálohy; pokud
žádná není, smažte ho a databáze se založí prázdná znovu:

```bash
docker compose down
docker volume rm server_coop-data
docker compose up -d
```

**Dvířka hlásí poruchu.** Něco jim překáží, nebo nedojela do koncové polohy. Odstraňte
překážku a klikněte na **Odblokovat**. Porucha se sama nezruší ani po vypnutí napájení.

**Hnízdo neodpovídá.** Zkontrolujte datový kabel do jeho krabičky a do krabiček před ním —
když se neozve jedno hnízdo, neozvou se ani všechna za ním. Pak ověřte, že `NEST_COUNT`
v `.env` sedí s `NESTS_COUNT` ve firmwaru a že řadič v krabičce má správnou adresu.
