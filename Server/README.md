# Kurník

Automatická dvířka kurníku ovládaná přes internet. Kurník posílá každých 10 minut stav
baterie, solárního panelu a dvířek; z webové stránky je lze kdykoli otevřít nebo zavřít.

```
kurník ──LoRa──> brána ──internet──> The Things Network ──> server ──> stránka
```

Příprava zabere asi půl hodiny. Kroky na sebe navazují, proto je vhodné dodržet jejich pořadí.

## Co je potřeba

- **Kurník** s namontovanou elektronikou
- **Brána** The Things Indoor Gateway a v domě Wi-Fi na 2,4 GHz
- **Raspberry Pi** a karta microSD — stačí i to nejmenší, podrobnosti níž
- **Tři údaje k zařízení**, které se dodávají spolu s kurníkem:

| Údaj | Vypadá jako |
|---|---|
| DevEUI | `0080E115XXXXXXXX` |
| JoinEUI | `0101010101010101` |
| AppKey | 32 znaků, například `2B7E1516…` |

Bez nich se kurník k síti nepřipojí. Pokud se údaje ztratily, v příloze je popsáno, jak DevEUI vyčíst ze zařízení.

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

Kontrolka chvíli bliká zeleně, pak střídavě zeleně a červeně. Za minutu až dvě je hotovo.

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
| JoinEUI, DevEUI, AppKey | tři údaje ze zařízení |

Zapněte kurník. Během pár minut se v **Live data** objeví `Accept join-request` a pak první zpráva.

## 5. Dekódování dat

Bez tohoto kroku jsou místo napětí vidět jen čísla v šestnáctkové soustavě.

V aplikaci **Payload formatters → Uplink** zvolte **Custom Javascript formatter** a vložte:

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

Uložte a počkejte na další zprávu. Pak už jsou vidět napětí a stav dvířek.

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
ssh pi@kurnik.local
sudo apt update && sudo apt full-upgrade -y
curl -fsSL https://get.docker.com | sh
sudo usermod -aG docker $USER
```

**Pak se odhlaste a přihlaste znovu**, jinak bude Docker hlásit chybu oprávnění.

### Klíč pro přístup k síti

V konzoli TTN v aplikaci **API keys → Add API key**. Zaškrtněte práva:

- Read application traffic (uplink and downlink)
- Write downlink application traffic

Klíč začíná `NNSXS.` a **zobrazí se jen jednou** — je potřeba ho hned zkopírovat.

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

Heslo si volí uživatel. Dlouhé a náhodné vygeneruje `openssl rand -base64 18`.

> **Heslo ke stránce nepoužívejte nikde jinde.** Je v souboru `.env` v čitelné podobě,
> stejně jako klíč k TTN. Kdo se dostane k tomu souboru, má stejně tak celý systém.

### Spuštění

```bash
docker compose up -d
```

Po restartu Pi se všechno spustí samo. Databáze se založí při prvním spuštění sama
a data přežijí i smazání a znovuvytvoření kontejnerů.

### Záloha

Celá historie měření je jeden soubor. Zkopírovat se dá třeba do domovského adresáře:

```bash
docker compose cp app:/data/kurnik.db ~/kurnik-zaloha.db
```

Obnova probíhá opačným směrem: zastavit server (`docker compose down`), nakopírovat
soubor zpět a server zase spustit.

## 7. Přístup ke stránce

Stránka je dostupná z čehokoliv v domácí síti — z počítače i z telefonu. Liší se jen
drobnosti, proto je dál každé zařízení zvlášť.

### Doma z počítače

Stačí otevřít **`http://kurnik.local:3000`**. Windows, macOS i běžné linuxové distribuce
jméno `.local` přeloží samy. Na Linuxu může chybět služba, která to umí; doinstaluje se
`sudo apt install avahi-daemon`.

Kdyby jméno nefungovalo, použijte místo něj IP adresu Pi — jak ji zjistit a zafixovat je
popsáno o kousek níž u Androidu, platí to stejně.

Pro rychlé spuštění si udělejte záložku nebo zástupce na ploše.

> Nabídka **Nainstalovat stránku jako aplikaci** se v Chrome ani Edge neobjeví. Prohlížeče
> ji nabízejí jen stránkám běžícím přes HTTPS a domácí adresa je obyčejné HTTP. Stránka
> funguje normálně, jen se otevírá v okně prohlížeče jako každá jiná.

### Doma z iPhonu

Stejně jako na počítači: **`http://kurnik.local:3000`**, jméno si telefon přeloží sám.
Po přihlášení zvolte **Sdílet → Přidat na plochu** a vznikne ikona, ze které se stránka
otevře bez adresního řádku jako aplikace.

### Doma z Androidu

**Jména `.local` tady nefungují.** Prohlížeč na Androidu je nepřekládá, takže je potřeba
zadat přímo IP adresu Pi:

1. **Zjistěte adresu.** Na Pi ji vypíše `hostname -I`, vyjde například `192.168.1.42`.
   Druhá možnost je podívat se v routeru do seznamu připojených zařízení a najít `kurnik`.
2. **Zafixujte ji.** V routeru najděte **DHCP reservation** (bývá pod Wi-Fi, LAN nebo DHCP)
   a přiřaďte tu adresu Raspberry Pi natrvalo. Potřebnou MAC adresu vypíše na Pi příkaz
   `ip link show wlan0`, je to řádek `link/ether`. Bez rezervace se adresa po výpadku proudu
   může změnit a uložená ikona přestane fungovat.
3. **Zadejte celou adresu včetně `http://`**, tedy `http://192.168.1.42:3000`. Bez toho ji
   Chrome pošle do vyhledávače.
4. **Kdyby prohlížeč přepnul na `https://`** a stránka nenaběhla, vypněte v Chrome
   **⋮ → Nastavení → Soukromí a zabezpečení → Vždy používat zabezpečená připojení**.
5. Přihlaste se jménem `kurnik` a heslem z `.env`, pak zvolte **⋮ → Přidat na plochu**.
   Vznikne ikona, která stránku otevře na jedno klepnutí.

> Ikona na Androidu otevře stránku v prohlížeči i s adresním řádkem. Aby se otevírala
> samostatně jako aplikace, musela by stránka běžet přes HTTPS — na domácím HTTP to Chrome
> nenabídne. Na funkci to nemá vliv.

Některé routery (OpenWrt, MikroTik, novější Asus) umí vlastní DNS záznam. Když se v takovém
routeru přiřadí jméno `kurnik` k adrese Pi, funguje pak i na Androidu.

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
Microsoft nebo e-mailem. Po přihlášení vypíše adresu Pi:

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

**Ikonu nebo aplikaci si udělejte právě z téhle adresy.** Funguje doma i z mobilních dat
a z cizí sítě, takže stačí jediná — na rozdíl od domácí adresy, která mimo domov neodpoví.

Aplikace drží připojení zapnuté; vypnout jde přepínačem, ale pak stránka mimo domov
nenaběhne. Tunelem prochází jen provoz na vlastní zařízení, běžné prohlížení internetu jde
mimo něj.

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
> Opakované klikání nepomůže — příkazy se řadí za sebe a provedou se všechny.

Pod tlačítky je řádek **Ve frontě** s příkazy, které ještě čekají na doručení. Tlačítko
**Zrušit** je smaže — pokud se to stihne dřív, než se kurník ozve, neprovede se nic.
Jakmile se příkaz doručí, stránka to oznámí a z fronty zmizí.

> Frontu si server pamatuje jen dokud běží. Po jeho restartu se řádek ukáže prázdný, i když
> v síti něco čeká; **Zrušit** ale vždy smaže vše, co v síti opravdu je, takže po
> restartu má smysl na něj kliknout, i když se nic nezobrazuje.

Za svítání a za soumraku se dvířka ovládají sama; ruční příkaz platí jen do nejbližší
takové změny.

## Když to nejede

**Brána není Connected.** Zkontrolujte, že domácí Wi-Fi vysílá na 2,4 GHz. Pokud ano, podržte
SETUP a nastavte ji znovu. Ujistěte se také, že byl použit **Claim gateway**, ne Register gateway.

**Kurník se nepřipojí, v konzoli je `MIC mismatch`.** AppKey v konzoli nesouhlasí s tím
v zařízení. Přepište ho a kurník vypněte a zapněte.

**Stránku nelze najít.** Na Androidu zkuste místo `kurnik.local` přímo IP adresu. Pokud prohlížeč
přepíná na HTTPS, vypněte v něm „Vždy používat zabezpečené připojení".

**Přihlášení hlásí příliš mnoho pokusů.** Po pěti špatných heslech se přihlašování na 15 minut
zamkne. Buď je potřeba počkat, nebo zámek zruší restart: `docker compose restart app`.

**V konzoli jsou zprávy, ale stránka je prázdná.** Zkontrolujte `TTN_APP_ID` a `TTN_API_KEY`
v souboru `.env`, pak `docker compose restart`.

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

## Příloha: jak zjistit DevEUI

Když DevEUI chybí, dá se vyčíst přímo z čipu. Potřeba je programátor ST-LINK a nástroj
STM32CubeProgrammer.

```bash
STM32_Programmer_CLI -c port=SWD -r32 0x1FFF7580 8
```

Výpis vypadá takto:

```
0x1FFF7580 : AABBCCDD 0080E115
```

DevEUI se složí tak, že **druhé číslo se dá před první**: `0080E115AABBCCDD`.
Druhé číslo musí být `0080E115` — podle toho se pozná, že jde o správné místo.
