# Bakalářská práce
Systém pro automatizaci kurníku s detekcí snesených vajec

## Zadání
- Prostudujte možnosti automatizace uzavírání kurníku, možné metody detekce snesených vajec a dostupné možnosti komunikace a připojení zařízení do IoT sítě (např. NB-IoT, WiFi, LoRaWAN).
- Navrhněte systém pro automatické otevírání a uzavírání dvířek kurníku s možností rozšíření o jednotky se senzory ve snáškových hnízdech (min. 5 hnízd). Prostudujte možnosti napájení z akumulátoru nebo
   fotovoltaického panelu. Proveďte analýzu možnosti přenosu dat do cloudu a jejich zobrazení uživateli včetně historie snášek a možnosti vzdáleného ovládání, například skrze aplikaci.
- Na základě analýzy navrhněte způsob vzdálené uživatelské interakce pro zobrazení stavu dvířek, manuální ovládání a počítání snesených vajec.
- Navrhněte schéma zařízení, které bude obsahovat mikrokontrolér, komunikační modul, senzory, řízení dvířek, napájecí/nabíjecí obvody. Dbejte na nízký příkon celého zařízení. Proveďte návrh desky plošných
   spojů.
- Zkonstruujte hardwarovou realizaci navrženého systému. Osaďte a zprovozněte řídicí elektroniku. Vytvořte ovládací firmware. Realizujte přenos dat ze zařízení k uživateli včetně ovládacího rozhraní a
   možnosti zobrazení historie.
- Ověřte funkčnost systému experimentálním měřením a vyhodnoťte spolehlivost detekce a ovládání.
- Zveřejněte veškeré výrobní podklady na vhodné platformě (např. GitHub).

## Schéma
<img src="https://github.com/Hosty-04/Bachelors_Theses/blob/main/schematics/block_schematic.png" alt="block_schematic">

## Popis

### Krabičky
Systém bude obsahovat jednu krabičku pro akumulátor o tloušťce stěny 2,4 mm, dále druhou krabičku (K) o tloušťce stěny 1,6 mm, jenž bude určena pro mechaniku a elektroniku dvířek a hlavní mikrořadič. Pro každé snáškové hnízdo bude dále určena samostatná krabička (Kx) o tloušťce stěny 1,6 mm. Každá tato krabička bude obsahovat potřebnou elektroniku a mikrořadič.

Všechny krabičky budou vytištěny na 3D tiskárně z materiálu PETG. Tento materiál byl zvolen pro jeho vysokou houževnatost a teplotní odolnost. Jejich zadní stěna bude ve svislém směru prodloužena a bude mít tloušťku 3,5 mm. V každém rohu této stěny budou vytvořeny otvory pro vruty umožňující upevnění krabiček ke stěně.

Dále budou na krabičkách K a Kx u víka vytvořeny nízké drážky ve tvaru písmene U s mírně zaoblenými rohy. Tyto drážky budou sloužit k protažení kabelů. Pro kabel o průřezu 1,5 mm² budou mít šířku 4,5 mm, hloubku 3 mm a zaoblení dna i hran 1 mm, pro kabel o průřezu 0,5 mm² budou mít šířku 3,5 mm, hloubku 2 mm a zaoblení dna i hran 0,75 mm a pro datový kabel bude šířka 6 mm a hloubka 3,5 mm a zaoblení stejné jako u prvního kabelu.

Krabička pro akumulátor bude mít ze spodu 2 kruhové otvory o průměru 5 mm. Tyto otvory budou určené pro průchod napájecích kabelů. V krabičce Kx bude ze spodu jeden kruhový otvor o průměru 6 mm. Ten bude určený pro průchod kabelu od tenzometru. V krabičce K bude zespodu otvor o průměru 3 mm pro lanko. U Krabičky pro akumulátor budou 2 větrací otvory o průměru 4 mm. Tyto otvory budou umístěny v horní a spodní části, aby mohl v krabičce cirkulovat vzduch.

Víko bude vytištěno samostatně, s přesahem 2 mm a uprostřed levé hrany bude obsahovat dutou trubičku s průměrem otvoru 2 mm. U všech krabiček budou na každém konci vnější horní hrany levé stěny umístěna trubička se stejným průměrem jako ta na víku. Po vytištění bude všemi třemi trubičkami protažen filament o průměru 1,75 mm, jehož konce budou krátce nahřáty zapalovačem a roztepány tak, aby vytvořily hlavičky. Tím vznikne jednoduchý pant pro uchycení víka. Toto řešení minimalizuje počet součástek a umožňuje výrobu celé krabičky bez dalších kovových dílů.

### Kabeláž
Pro připojení solárního panelu bude použita měděná ohebná licna o průřezu 1,5 mm². Ta bude pomocí 4 pinové pružinové WAGO svorkovnice o rozteči 5,08 mm připojena k desce plošných spojů a odtamtud povede do akumulátoru. Tento průřez poskytuje dostatečnou proudovou rezervu při minimálním úbytku napětí.

Mikrospínače budou připojeny k desce plošných spojů stejným způsobem. Použita bude ale měděná ohebná licna o průřezu 0,5 mm². Spoje této licny s kontakty mikrospínače budou izolovány pomocí smršťovacích bužírek s poměrem 2:1 a vnitřním průměrem před / po zahřátí 2 mm / 0,75 mm. Bužírky budou přečnívat asi centimetr přes izolaci. U prototypu bude využíváno pouze kabelů o průřezu 0,5 mm² a drátků do nepájivého pole se stejným průřezem, který je zcela dostačující pro proudový odběr tohoto systému.

Kabely budou uloženy v klasické elektroinstalační PVC liště o rozměrech 15 × 10 mm. Je dostatečně prostorná a zároveň minimalistická. Speciální UV lišta není potřeba, protože stěna kurníku směřuje na severozápad. Životnost těchto lišt je odhadována na 5 - 10 let.

Pro datovou komunikaci byl zvolen kabel UTP CAT5e typu licna, který bude ke stěně upevněn pomocí příchytek s vrutem o průměru 6 mm a přes konektory RJ45 připojen k desce plošných spojů. Prodloužení vodičů tenzometru zajistí přes prodlužovací pružinové WAGO svorky stíněný kabel LiYCY 4 × 0,25 mm² typu licna. Jelikož jsou vodiče od tenzometru o průřezu 0,14 mm² pro svorky příliš tenké, tak se u odizolovaného konce přehnou napůl na průřez 0,28 mm². K desce plošných spojů bude potom tento kabel spolu se stíněním připojen pomocí 5 pinové pružinové WAGO svorkovnice o rozteči 2,5 mm.

Oplet tohoto kabelu, ve formě pocínovaných měděných drátků, bude izolován pomocí smršťovací bužírky s poměrem 2:1 a vnitřním průměrem před / po zahřátí 4 mm / 1,75 mm. Je potřeba oddělit odmotaný oplet, spletený do drátku, od zbylých 4 vodičů a bužírku navléknout až ke kořenu. Přes celý kabel bude potom přetáhnuta bužírka 2:1 7,5 mm / 3,5 mm, která bude přečnívat asi centimetr přes hlavní izolaci.

### Napájení
Výrobu energie bude zajišťovat solární panel o parametrech 9 V / 10 Wp, který bude svisle připevněný na stěnu a orientovaný směrem na jih, případně na východ nebo západ (v mém případě bude panel směřovat na jihozápad). Tím bude zajištěno, že panel bude co nejlépe využívat dostupnou sluneční energii. Vertikální montáž omezí usazování sněhu a nečistot. Tento solární panel byl zvolen, protože při použití jednoduchého MOSFET odpojovače poskytuje vhodný poměr mezi napěťovou rezervou pro nabíjení 6 V akumulátoru a dostupným nabíjecím proudem. Vyšší výkon panelu zároveň zvyšuje energetickou rezervu systému v zimním období při nízké intenzitě slunečního záření.

Systém bude napájen přes bezúdržbový olověný AGM akumulátor o parametrech 6 V / 4 Ah, umístěným venku ve stínu asi 25 cm pod stříškou. I při zohlednění poklesu kapacity v zimním období o 30 % představuje energetická rezerva akumulátoru několik stovek dní provozu. V praxi bude provozní dobu více než spotřeba systému omezovat několik týdnů špatného počasí zároveň se samovybíjením a přirozené stárnutí. Akumulátor LiFePO4 je sice v mnoha ohledech lepší než olověný, ale nesmí se nabíjet v zimě pod 0 °C a potřebuje složitější nabíjecí systém. Jelikož bude systém venku, kvůli amoniaku ze slepičího trusu, tak je olověný akumulátor lepší volbou pro celoroční provoz.

Před akumulátorem se bude nacházet nízkopříkonový jednoduchý mikrořadičem řízený MOSFET odpojovač fotovoltaického zdroje s ochranou akumulátoru. Použití MPPT regulátoru nebylo zvoleno z důvodu vyšší složitosti a vlastní spotřeby spínaného měniče. U systému s velmi nízkým denním odběrem by zlepšení účinnosti při nabíjení pouze několik minut denně nepřineslo významný energetický přínos oproti jednoduchému MOSFET odpojovači s téměř nulovou klidovou spotřebou.

Silová část bude pracovat s napětím 6 V, zatímco veškerá elektronika bude napájena napětím 3,3 V. Snížení napětí z 6 V na 3,3 V bude realizováno pomocí nízkopříkonového LDO regulátoru MCP1702 s velmi nízkým klidovým proudem a zcela dostačujícím výstupním proudem 250 mA. Tento stabilizátor bude zapojený k akumulátoru a na jeho vstupu bude připojen blokovací keramický kondenzátor o kapacitě 1 µF a jmenovitém napětí 50 V. Stejný kondenzátor bude připojen i na výstupu, ale v tomto případě z důvodu vyhlazení napětí. 

Použití spínaného buck měniče není vhodné z důvodu horší dostupnosti nízkopříkonových variant a velmi malého odběru systému po většinu dne. Jeho vyšší účinnost by se projevila pouze po dobu několika minut denně. Kvůli vlastní spotřebě by pak paradoxně dosahoval nižší celkové účinnosti než jednoduchý lineární LDO regulátor.

## Denní přehled

### Klidová spotřeba (24 hod)

| Komponenta | Proud (typ) | Proud (max) | Spotřeba (typ) | Spotřeba (max) |
|---|---|---|---|---|
| LDO (MCP1702) | 2 µA | 5 µA | 48 µAh | 120 µAh |
| M (Stop2 s RTC) | 1,0 µA | 2,8 µA | 24 µAh | 67,2 µAh |
| 5 × Mx (Stop bez RTC) | 1,9 µA | 1,9 µA | 45,6 µAh | 45,6 µAh |
| **Celkem** | **4,5 µA** | **8,7 µA** | **118 µAh** | **233 µAh** |

*Poznámka: Ostatní části systému jsou odpojovány přes tranzistorové spínače.*

### Kontrola panelu a akumulátoru (43 s)

| Komponenta | Proud (typ) | Proud (max) | Spotřeba (typ) | Spotřeba (max) |
|---|---|---|---|---|
| M (LPRun @ 1 MHz) | 220 µA | 220 µA | 2,64 µAh | 2,64 µAh |
| INA219 aktivní | 0,7 mA | 1 mA | 8,4 µAh | 12 µAh |
| **Celkem** | **0,92 mA** | **1,22 mA** | **11 µAh** | **14,6 µAh** |

### Pohyb dvířek (40 s / 60 s)

| Komponenta | Proud (typ) | Proud (max) | Spotřeba (typ) | Spotřeba (max) |
|---|---|---|---|---|
| Motor | 150 mA | 200 mA | 1,67 mAh | 3,33 mAh |
| DRV8838 | 340 µA | 600 µA | 3,78 µAh | 6,67 µAh |
| M (LPRun @ 1 MHz) | 220 µA | 220 µA | 2,44 µAh | 2,44 µAh |
| INA219 | 0,7 mA | 1 mA | 7,78 µAh | 11,1 µAh |
| **Celkem** | **151 mA** | **202 mA** | **1,68 mAh** | **3,35 mAh** |

*Poznámka: Možnost zaseknutí dvířek je brána v potaz.*

### Kontrola vajec (8 min, 5 hnízd)

| Komponenta | Proud (typ) | Proud (max) | Spotřeba (typ) | Spotřeba (max) |
|---|---|---|---|---|
| M (LPRun @ 1 MHz) | 220 µA | 220 µA | 29,3 µAh | 29,3 µAh |
| MAX3485 (M) | 1,1 mA | 2,2 mA | 0,147 mAh | 0,293 mAh |
| MAX3485 (Mx) | 1,1 mA | 2,2 mA | 0,147 mAh | 0,293 mAh |
| Mx (LPRun @ 131 kHz) | 32 µA | 37 µA | 4,27 µAh | 4,93 µAh |
| HX711 a tenzometr | 4,4 mA | 4,4 mA | 0,587 mAh | 0,587 mAh |
| **Celkem** | **6,85 mA** | **9,06 mA** | **0,914 mAh** | **1,21 mAh** |

*Poznámka: STM32 NUCLEO-L031K6, MAX3485, HX711 a tenzometr. Tyto obvody jsou v každé Kx, ale zásluhou chytrého používání tranzistorových spínačů a režimů řadiče se proud tváří, jako kdyby bylo v celém kurníku pouze jedno hnízdo. Trik je v tom, že zapnuté je pouze to, co zrovna pracuje. Výsledkem je 5 × nižší spotřeba*

### LoRa TX (3 s)

| Komponenta | Proud (typ) | Proud (max) | Spotřeba (typ) | Spotřeba (max) |
|---|---|---|---|---|
| Rádio TX (SMPS) | 21 mA | 21 mA | 17,5 µAh | 17,5 µAh |
| CPU (LPSleep) | 44 µA | 69 µA | 0,0367 µAh | 0,0575 µAh |
| **Celkem** | **21 mA** | **21,1 mA** | **17,5 µAh** | **17,6 µAh** |

### LoRa RX (10 s)

| Komponenta | Proud (typ) | Proud (max) | Spotřeba (typ) | Spotřeba (max) |
|---|---|---|---|---|
| Rádio RX (SMPS) | 4,8 mA | 4,8 mA | 13,3 µAh | 13,3 µAh |
| CPU (LPSleep) | 44 µA | 69 µA | 0,122 µAh | 0,192 µAh |
| **Celkem** | **4,84 mA** | **4,87 mA** | **13,4 µAh** | **13,5 µAh** |

*Poznámka: Odhad délky okna je 200 ms.*

### Procentuální rozložení a celková denní spotřeba

| Blok | Spotřeba (typ) | Podíl | Spotřeba (max) | Podíl |
|---|---|---|---|---|
| Pohyb dvířek | 1,68 mAh | 61,2 % | 3,35 mAh | 69,6 % |
| Kontrola vajec | 914 µAh | 33,3 % | 1,21 mAh | 25,1 % |
| Klidový režim | 118 µAh | 3,9 % | 233 µAh | 4,3 % |
| LoRa TX | 17,5 µAh | 0,6 % | 17,6 µAh | 0,4 % |
| LoRa RX | 13,4 µAh | 0,5 % | 13,5 µAh | 0,3 % |
| Kontrola panelu / baterie | 11 µAh | 0,4 % | 14,6 µAh | 0,3 % |
| **Celkem** | **2,75 mAh** | 100 % | **4,84 mAh** | 100 % |

### Výroba energie

| Orientace | Léto | Zima |
|---|---|---|
| Jih | 28 Wh | 5 Wh |
| Východ | 22 Wh | 4 Wh |
| Západ | 22 Wh | 4 Wh |
| Jihozápad | 25 Wh | 4,5 Wh |

### Energetická bilance

| Orientace panelu | Léto | Zima | Hodnocení |
|---|---|---|---|
| Jih | +4200 mAh | +730 mAh | Velmi vysoká energetická rezerva |
| Východ | +3200 mAh | +530 mAh | Spolehlivý celoroční provoz s dostatečnou rezervou |
| Západ | +3200 mAh | +530 mAh | Spolehlivý celoroční provoz s dostatečnou rezervou |
| Jihozápad | +3700 mAh | +630 mAh | Spolehlivý celoroční provoz, pouze mírně nižší výroba než při orientaci na jih |

*Poznámka: Do posledních 2 tabulek byly započteny ztráty neideálního pracovního bodu solárního panelu a jednoduchého regulátoru, přibližná nabíjecí účinnost akumulátoru, samovybíjení a běžné klimatické podmínky v ČR.*

### Řízení
Hlavní řídicí jednotkou systému bude mikrořadič STM32WLE5JC LoRa-E5 mini (M). Ten bude disponovat integrovaným LoRa modulem, komunikujícím přes LoRaWAN stack. Technologie LoRaWAN umožní na rozdíl od Wi-Fi komunikaci na velké vzdálenosti při nízké spotřebě energie a na rozdíl od NB-IoT trvalé řešení s dobrým pokrytím. U každého snáškového hnízda bude umístěn další mikrořadič STM32 NUCLEO-L031K6 (Mx). Ten má integrovaný programátor, který bude využit i pro hlavní řadič.

Firmware bude vyvíjen v prostředí STM32CubeIDE. Součástí firmwaru hlavního řadiče budou astronomické hodiny, které každý den, pomocí RTC hodin, spočítají čas východu a západu slunce. Podle těchto údajů se potom budou automaticky otevírat a zavírat dvířka od kurníku. Přes RTC hodiny bude řadič schopen zjistit také roční období. Použití časovače nebylo zvoleno z důvodu proměnlivé délky dne. Světelný senzor by mohl způsobovat chybné sepnutí motoru dvířek, kvůli zatažené obloze při dešťi a bouřce nebo vlivem pouličního osvětlení či světlometů automobilů. Konfiguraci zajistí STM32CubeMX.

Hlavní řídicí jednotka se bude společně s nezbytnými částmi systému probouzet každých 10 minut za účelem kontroly stavu solárního panelu a akumulátoru (cca 300 milisekund). Dále se bude společně s ostatními řídicími jednotkami a dalšími potřebnými částmi systému probouzet 24× denně, tedy každou hodinu, pro sekvenční kontrolu stavu vajec s postupným buzením každého hnízda (24 × 5 hnízd × 4 s na jedno hnízdo = cca 8 minut). Po sběru dat ze všech hnízd proběhne komunikace. Nakonec se bude probouzet ráno a večer, opět pouze s nezbytnými částmi systému, za účelem otevření a zavření dvířek (cca 2 × 20 s). Před zahájením pohybu dvířek budou piny koncových spínačů nastaveny jako digitální vstupy. Po dosažení koncové polohy proběhne komunikace a piny budou přepnuty do analogového režimu bez pull rezistorů, čímž bude eliminován jejich klidový odběr.

LoRa anténa bude moci vysílat až po vypnutí všech systémů, kvůli jejímu vyššímu odběru proudu a ochraně před rušením. Po každém vysílání bude mít potom možnost příjmout data, což umožní uživatelské ovládání. Upřednostňované parametry komunikace jsou: vysílací výkon 12 dBm, SF9, BW 125 kHz, CR 4 / 5, 2 příjmová okna RX1 + RX2 a délka příjmových oken automaticky podle driftu krystalu.

V domě bude umístěna LoRaWAN gateway, která bude přes WiFi plnit funkci internetové brány. Veškerá data přijatá přes gateway budou následně odeslána do cloudu (TTN) a potom přes MQTT na backend (NodeJS). Ten data uloží do databáze (InfluxDB) a zobrazí je na frontendu. Při odesílání dat z gateway je proces obrácený.

Data budou z kurníku odesílána pomocí 3+ bajtů. První bajt bude obsahovat 1 bit pro indikaci zapnutí / vypnutí systému a 7 bitů ponese informaci o napětí solárního panelu (0 - 9 V, krok cca 0,1 V). Druhý bajt bude obsahovat 5 bitů pro indikaci napětí na akumulátoru (5,75 V - 8 V, krok cca 0,1 V), 1 bit pro oznámení kritického stavu (nízké napětí na akumulátoru) a 2 bity ponesou informaci o stavu dvířek (otevřeno / zavřeno / zaseknuto). Dalších x bajtů bude po 4 bitech alokováno pro přenos počtu vajec v x snáškových hnízdech (0 - 10 vajec na hnízdo). Kurník bude data přenášet každou hodinu po kontrole stavu hnízd a k tomu 2× denně, při změně stavu dvířek. V druhém případě odešle pouze první 2 bajty. Manuální ovládání bude využívat pouze 1 bajtu. Jeden bit bude využit pro vypnutí / zapnutí systému, další bit pro vypnutí / zapnutí kritického režimu a poslední bit pro ovládání dvířek (otevřeno / zavřeno).

Pro přenos dat mezi hlavní řídicí jednotkou a ostatními řídicími jednotkami bude využit protokol LPUART, který nevyužívá hodinový signál a vyznačuje se nízkou spotřebou energie. Přenosová rychlost bude 9600 Bd, kvůli minimalizaci odrazů. Na aplikační vrstvě bude použit komunikační protokol Modbus RTU a knihovna ModbusRTU-Slave. Protokol Modbus RTU vytvoří datový rámec obsahující adresu jednotky slave, přenášená data a kontrolní součet CRC pro detekci chyb při přenosu. LPUART, hardware v řadiči, následně převede jednotlivé bajty na sériový datový tok, doplní start a stop bity a zajistí jejich přenos po sběrnici. Na straně přijímače proběhne opačný proces.

**Stavový automat pro algoritmus detekce snesených vajec**  
- Probuzení mikrořadiče  
- Čekání 500 ms na ustálení měření  
- Odebrání 32 vzorků rychlostí 10 SPS  
- Výpočet mediánu  
- Výběr 16 vzorků s nejmenší odchylkou od mediánu  
- Výpočet aritmetického průměru, aktuální hmotnosti (odečet referenční nulové hodnoty) a směrodatné odchylky  
- Pokud hmotnost překročí 1 kg (v hnízdu je slepice), měření se zahodí  
- Pokud odchylka překročí stanovený práh (pohyb slepice, vibrace), měření se zahodí  
- Je-li měření stabilní, aktuální hmotnost se porovná s uloženou hodnotou  
- Odpovídá-li rozdíl hmotnosti přibližné hmotnosti jednoho (60 g) nebo více vajec, je změna aritmeticky přičtena k uložené hodnotě  
- Při hmotnosti < 25 g je provedena kontrola driftu. Pokud jsou zaznamenána tři po sobě jdoucí stabilní měření, je aktualizována referenční nulová hodnota  
- Uspání mikrořadiče  

Většinu dne bude hlavní řídicí jednotka v režimu Stop2 s RTC. Tento režim se vyznačuje velmi nízkou spotřebou a na rozdíl od režimu StandBy s RTC má mimo jiné možnost udržet logické úrovně a nastavení pinů. Řadič je automaticky taktovaný externím krystalem LSE, umístěným na LoRa-E5 mini, na 32 kHz. Když ale RTC hodiny zavelí, že je čas na práci, tak se řadič přepne do režimu LPRun (Low Power Run). V tomto režimu je taktovaný interním krystalem LSI na 1 MHz. V průběhu LoRa přenosu (Radio TX / RX) se CPU přepne do LPSleep režimu (LSI, 1 MHz). Kvůli nízké taktovací frekvenci je potřeba v souboru lorawan_conf.h zvýšit RADIO_WAKEUP_TIME z 2 na 5 ms. Rádio poběží automaticky na 32 MHz.

U ostatních řídicích jednotek to bude po většinu dne velmi podobné. Budou se nacházet v úsporném režimu Stop bez RTC, ze stejných důvodů jako hlavní řídicí jednotka a protože je potřeba, aby si uchovaly paměť RAM. Řadiče budou postupně jeden za druhým probouzeny přes hlavní řadič, přesněji přes LPUART. Tudíž nepotřebují RTC hodiny. Po probuzení se daný řadič přepne do režimu LPRun (LSI, 131 kHz) a hned po vykonání úkolu se přepne zpět do režimu Stop bez RTC.

Před odpojením napájení VCC různých částí systému je potřeba piny (SCK, DT, PH, EN, DI, DE, RE, RO) přepsat na logickou nulu. Důvodem je zamezení napájení přes piny tomu neurčené (leakage current). Dále budou vypnuty periferie (I²C, UART, ADC) a jejich hodinový signál, který plýtvá energii, i když nic neposílají. Po odpojení VCC je potom třeba nastavit všechny piny, včetně těch pro teď už vypnuté periferie, do analogového režimu bez pull rezistoru.

Kvůli spotřebě je potřeba odpájet červenou Power LED diodu a softwarově odpojit všechny zelené User LED diody. Programátor ST-Link musí být při provozu hardwarově odpojen. Pájecí jumpery SB9, SB14, SB2 a SB3 je proto potřeba odpájet. U LoRa-E5 mini je ještě potřeba přepnout piny PA2 a PA3, zodpovědné za ladění, do analogového režimu bez pull rezistoru. Pro programování lze poté programátor připojovat k deskám přes klasické Dupont kabely a u LoRa-E5 mini vrátit piny PA2 a PA3 do původního stavu. Pozor na plovoucí piny! Nepoužívané piny musejí být uvedeny do analogového režimu bez pull rezistoru. Kvůli správné funkci úsporného režimu je u hlavní řídicí jednotky před uspáním nutné nastavit externí RF switch na logickou nulu a použít LP rádio (vstup i software). U ostatních řídicích jednotek je zase nutné v registrech napájení (PWR) aktivovat ultra-low-power režim (ULP bit) a vypnout fast wakeup.

### Elektronika
Prototyp bude sestaven z modulů umístěných na nepájivém poli pomocí kolíkových lišt a tranzistorových spínačů umístěných na univerzálních deskách plošných spojů. Finální verze bude obsahovat jednu hlavní desku plošných spojů a třeba 5 vedlejších desek pro jednotlivá snášková hnízda (v mém případě 2). Na všech deskách budou moduly nahrazeny čipy a nezbytnými externími SMD součástkami.

Po připojení solárního panelu do krabičky K bude využito elektrolytického kondenzátoru 47 µF / 25 V a keramického kondenzátoru 100 nF / 50 V, zapojených co nejblíže spínacím prvkům. Elektrolytický bude fungovat jako zásobárna energie a keramický bude filtrovat rychlé špičky.

K solárnímu panelu bude připojen vysokoimpedanční napěťový dělič tvořený rezistory o hodnotách 1 MΩ a 470 kΩ. Paralelně k rezistoru R2 (470 kΩ) bude připojen blokovací keramický kondenzátor o parametrech 100 nF / 50 V. Dělič bude sloužit ke snímání napětí panelu, přičemž naměřené hodnoty budou odesílány do M přes ADC pin v analogovém režimu. Díky vysoké impedanci bude proudový odběr děliče zanedbatelný.

Dále budou k solárnímu panelu sériově připojeny dva P-MOS tranzistory s nízkým RDS(on), spojené back-to-back (drainy proti sobě), které budou tvořit hlavní výkonový spínač celého systému. Jejich řízení bude realizováno jedním budicím logic-level N-MOS tranzistorem, protože napětí 3,3 V z M není dostatečné pro jejich přímé sepnutí. N-MOS bude řízen přes M. Na jeho gate bude sériově připojen rezistor o hodnotě 220 Ω. Mezi gate a společnou zem bude paralelně připojen 10 kΩ pull-down rezistor, který zabrání vzniku nedefinovaného logického stavu. Drain bude připojen k akumulátoru přes 10 kΩ pull-up rezistor a na gate obou P-MOS tranzistorů. P-MOS tranzistory budou tedy spínané 6 V. Source N-MOS tranzistoru bude připojen ke společné zemi.

Důvodem použití dvou P-MOS tranzistorů namísto jednoho je přítomnost parazitní diody, která je nedílnou součástí jejich vnitřní struktury. Pokud by byl použit pouze jeden tranzistor, protékal by večer i při jeho rozepnutí proud z akumulátoru zpět do solárního panelu, což by způsobovalo samovolné vybíjení akumulátoru. Pokud nastane výpadek napájení, tak se P-MOS tranzistory automaticky zavřou.

Modul proudového a napěťového senzoru INA219 bude spolu s vyrovnávacím keramickým kondenzátorem o parametrech 100 nF / 50 V, co nejblíže paralelně zapojeným mezi piny Vin+ a GND, zapojen v krabičce K mezi akumulátor a vstup VM pro napájení motoru u H-bridge. Jednou z jeho funkcí bude s 12bitovým rozlišením a průměrováním ze 128 vzorků snímat napětí akumulátoru.

Na základě údajů z tohoto modulu a děliče napětí bude M prostřednictvím sběrnice I²C, resp. portu GPIO vyhodnocovat stav akumulátoru a solárního panelu. M realizuje MOSFET odpojovač s hysterezí. Pokud bude napětí na akumulátoru limitní, v létě 7,2 V, na jaře / podzim 7,3 V a v zimě 7,5 V, tak odpojí solární panel. Pokud napětí akumulátoru následně klesne o 250 mV po dobu 30 minut (3 po sobě jdoucí měření), tak M solární panel znovu připojí. Při kritickém vybití akumulátoru, kdy jeho napětí klesne na hodnotu 5,75 V, přejde M do kritického režimu a bude pouze kontrolovat napětí na panelu a akumulátoru. K obnovení provozu dojde po dosažení 6,1 V. Během noci, kdy napětí panelu klesne pod 0,5 V, musí M zamezit vzniku zpětného proudu směrem do panelu. Toho docílí jeho odpojením.

Senzor INA219 bude současně využíván jako proudový snímač s 12bitovým rozlišením a průměrováním ze 4 vzorků. M bude při pohybu dvířek monitorovat proud odebíraný z akumulátoru. Náhlé zvýšení proudu nad 225 mA u 3 po sobě jdoucích měření bude indikovat zablokování dvířek nebo překážku v cestě, respektive slepici. V takovém případě se M zastaví na 250 ms, pokusí se obrátit směr otáčení motoru, dvířka vrátí zpět do původní polohy, uspí se a po 10 minutách to zkusí znovu. Pokud ani zpětný chod problém nevyřeší, systém se vypne. Bude nutné ignorovat krátkodobé proudové špičky vznikající při rozběhu motoru, trvající asi 150 ms.

Při pohybu dvířek budou postupně přes 50 ms debounce spínány, respektive rozpínány dva mikrospínače. Kontakt COM bude připojen k akumulátoru a kontakt NO spolu se silným 1 kΩ externím pulldown rezistorem k M. Koncové spínače představují jednoduché a energeticky nenáročné řešení určování krajních poloh.

M bude dále napřímo (bez PWM modulace) řídit H-bridge Pololu DRV8838, který bude vybaven elektrolytickým kondenzátorem o parametrech 47 µF / 25 V zapojeným co nejblíže mezi piny VM a GND, jenž bude potlačovat indukční napěťové špičky vznikající při vypínání motoru. Ten bude odrušen keramickým kondenzátorem 100 nF zapojeným přímo mezi jeho vývody a dvěma keramickými kondenzátory 47 nF zapojenými mezi jednotlivé vývody a kostru motoru (Faradayova klec). Všechny kondenzátory budou dimenzovány na napětí 50 V. Toto odrušení je nezbytné pro omezení jiskření kartáčků a potlačení vysokofrekvenčního elektromagnetického rušení. H-bridge i elektromotor budou umístěny v krabičce K.

Měření váhy snáškového hnízda bude zprostředkovávat tenzometr se zanedbatelnou nelinearitou a hysterezí. Kabel od tenzometru bude připojen k modulu AČ převodníku HX711 s nízkým klidovým proudem v řádu jednotek mikroampér, umístěnému v krabičce Kx. Modul bude použit z důvodu zesílení velmi nízkého výstupního napětí tenzometru, které se pohybuje v řádu jednotek milivoltů. Stínění kabelu bude na desce plošných spojů připojeno ke společné zemi, za účelem odvodu šumu. Převodník bude připojen k Mx.

Mx bude pro komunikaci s M prostřednictvím datového kabelu typu UTP využívat sběrnici RS485. První kroucený pár bude sloužit k přenosu napájení pro Mx, přičemž oba vodiče budou zapojeny paralelně. Stejně tak u druhého páru, který bude sloužit k přenosu napájení do zbylé elektroniky v krabičce Kx přes tranzistorový spínač. Třetí pár bude opět přes dva paralelní vodiče použit pro propojení společné země. Čtvrtý pár bude přenášet data prostřednictvím čipu MAX3485, jenž bude sloužit jako transceiver sběrnice RS485. Jeden čip bude před M a druhý před Mx.

Tento čip vytváří diferenciální signál na linkách A a B, čímž zvyšuje odolnost komunikace proti elektromagnetickému rušení. Jelikož je to čip, je pro prototyp potřeba použít adaptér SO8 na DIP8 a kolíkové lišty. Paralelně k vývodům VCC a GND bude připojen blokovací keramický kondenzátor o parametrech 100 nF / 50 V.

Vzhledem k použití několika snáškových hnízd bude komunikace probíhat mezi hlavní řídicí jednotkou M (master) a několika řídicími jednotkami Mx (slave), které budou propojeny sériově v topologii Daisy Chain. Vzhledem ke krátké délce vedení v řádu jednotek metrů nebude nutné na začátek ani konec sběrnice připojovat terminační rezistory o hodnotě 120 Ω pro impedanční přizpůsobení vedení. Jejich použití by pouze zvyšovalo proudový odběr systému.

Pro dosažení nízké spotřeby bude celý systém řízen několika tranzistorovými spínači. Tyto spínače budou konstruovány stejně jako hlavní spínač, ale jen s jedním PMOS tranzistorem. Přes první z těchto spínačů bude M řídit napájení k INA219. Přes druhý napájení k DRV8838 a třetí bude dodávat napětí k hlavnímu MAX3485 a zároveň do všech krabiček Kx. V každé krabičce Kx budou potom 2 spínače. Přes první z nich bude Mx řídit napájení ke svému MAX3485 a HX711. Ten bude ve výchozím stavu sepnutý. Druhý spínač bude Mx umožňovat poslat napětí do další krabičky Kx. Ten bude ve výchozím stavu rozepnutý. Použití těchto spínačů sníží spotřebu celého systému.

Po zapnutí napájení k daným částem systému bude potřeba chvíli počkat, než naběhnou. U INA219 to je 150 µs před odesláním konfigurace a 1,5 ms než je připraven s prvními naměřenými hodnotami. Pro DRV8838 je to 1,5 ms než se nabije nábojová pumpa a MAX3485 potřebuje pouhých 150 µs.

Všechny části systémů v jednotlivých krabičkách musí být co nejblíže u sebe. Desky plošných spojů budou obsahovat souvislou zemní plochu připojenou k zápornému pólu solárního panelu a akumulátoru.

### Mechanika
Hlavní část systému bude umístěna na vnější stěně kurníku, která bude splňovat požadavky na umístění solárního panelu uvedené v kapitole Napájení. Toto řešení zjednoduší montáž a zároveň z velké části eliminuje působení amoniaku ze slepičího trusu na elektroniku.

Na stěnu budou pomocí vrutů do dřeva připevněny svislé dřevěné hranolky. K jejich bočním stranám budou následně pomocí vrutů upevněny hliníkové U-profily s vnitřním rozměrem 10 mm. U-profily zajistí přesné vedení dvířek při minimálním tření.

V těchto profilech se budou ve svislém směru pohybovat bílá pěněná PVC dvířka o rozměrech 24 × 35 cm a tloušťce 8 mm (pozor na tolerance). Při vrtání otvorů do PVC bude nutné použít nízké otáčky a nevyvíjet nadměrný přítlak. Pěněné PVC bylo zvoleno díky nízké hmotnosti, odolnosti proti vlhkosti a snadné obrobitelnosti.

Na horní části přední strany dvířek bude umístěno závěsné očko M5 z nerezové oceli A2, upevněné pomocí zapuštěné podložky a matice stejného typu. Očko bude procházet otvorem o stejném průměru, jako je jeho metrický závit. Podložka i matice budou zapuštěny do záhlubení o průměru 10 mm a hloubce 3 mm.

Očkem bude protaženo syntetické lanko (zednická stavební šňůra) o průměru 2 mm, které bude zajištěno dračí smyčkou zakápnutou vteřinovým lepidlem. Syntetická šňůra nekoroduje, má nízkou průtažnost a je odolná vůči vlhkosti.

Lanko povede kolmo vzhůru do krabičky K, ve které bude navinuto na špulku vytištěnou z materiálu PETG. Špulka bude mít šířku 2 cm a vnější průměr 2,5 cm. Otvorem o průměru 3 mm, umístěným na okraji špulky dále od motoru, bude lanko provlečeno a upevněno osmičkovým uzlem, který bude rovněž zakápnut vteřinovým lepidlem. Tento uzel bude zapuštěn 8 mm hluboko do záhlubení s průměrem 10 mm. Vývod lanka se bude nacházet na okraji záhlubení směrem k okraji špulky.

Špulka bude opatřena otvorem s D-profilem o průměru 3 mm a pomocí stavěcího šroubu (červíku) M5 z nerezové oceli A2 o délce 5 mm a mosazné závitové vložky M5 (délka 5,8 mm, průměr 7,1 mm) bude upevněna na hřídeli nízkootáčkového stejnosměrného motoru s kovovou převodovkou (6 V), a to ze strany, kde bude vstupovat lanko.

Otvor pro mosaznou závitovou vložku bude mít průměr 8 mm a v posledních 6,5 mm směrem ke hřídeli se zúží na průměr 6,5 mm. Do otvoru bude zavedena vložka pomocí dlouhého 15 mm šroubu M5. Jakmile vložka narazí na zúženou část otvoru, bude hlava šroubu nahřáta mikropájkou. Plast se v místě zúžení roztaví, což umožní zalisování vložky. Po odejmutí pájky plast opět ztuhne a vložku pevně ukotví.

V horní i dolní části hranolku budou ve svislé ose zapuštěny dva pákové koncové mikrospínače. Záhlubení široká 6,5 mm a vysoká 20 mm povedou skrz hranolek a 10 mm širokou hliníkovou drážku, kvůli vývodu pinů. Mikrospínače budou upevněny pomocí vrutů o průměru 2 mm. Je důležité otvory předvrtat, kvůli deformaci plastu při vrtání do dřeva.

Vedlejší část systému bude umístěna u spádovaných snáškových hnízd s košíkem orientovaných příčně ke stěně.

Pod kukaně bude pomocí vrutů připevněna březová sítotisková překližka o rozměrech 37 × 28 cm a tloušťce 12 mm. Druhá překližka o rozměrech 35 × 26 cm a stejné tloušťce bude tvořit základovou desku a bude připevněna k podlaze kurníku. Tyto desky budou vyrobeny z jedné nařezané překližky o rozměrech 765 × 317 mm (tolerance započteny). Třetí překližka o rozměrech 40 × 50 cm a tloušťce 12 mm bude sloužit jako zadní stěna.

Všechny tyto velmi odolné vodovzdorné protiskluzové překližky budou orientovány hladkou stranou směrem nahoru, respektive směrem ke hnízdu. Jejich řezné hrany budou opatřeny voděodolným lakem z důvodu ochrany proti vlhkosti.

Mezi horní a spodní překližkou bude u stěny umístěn tenzometr o tloušťce 12,7 mm, orientovaný kratší stranou s vodiči směrem ke stěně. Tenzometr váží s přesností 2 g, což je pro 50-70 g vejce zcela v pořádku. Je dimenzován na zatížení 20 kg. Maximální předpokládané zatížení tvořené dvěma překližkami, hnízdem, dvěma slepicemi a deseti vejci činí přibližně 10 kg.

Optická závora není pro počítání vajec v kurníku vhodná mimo jiné z důvodu výskytu prachu, peří, podestýlky a zvědavých slepic. Všechny tyto objekty by mohly přerušit laserový paprsek a být chybně vyhodnoceny jako propadnuté vejce. Z tohoto důvodu byl pro detekci vajec zvolen tenzometr.

Volný konec tenzometru bude upevněn dvěma zápustnými inbusovými šrouby M4 z nerezové oceli A2. Šrouby budou procházet vážicí deskou skrz otvory o průměru 4,5 mm a budou zapuštěny do záhlubení o průměru 10 mm a hloubce 3 mm. Mezi hlavu šroubu a desku bude nanesena tenká vrstva PU tmelu.

Pevný konec tenzometru bude upevněn taktéž dvěma zápustnými inbusovými šrouby M5 z nerezové oceli A2. Ty budou procházet základovou deskou skrz otvory o průměru 5,5 mm a budou rovněž zapuštěny do záhlubení o průměru 10 mm a hloubce 3 mm.

Použité šrouby budou mít délku 25 mm. Mezi deskami a závity tenzometru budou umístěny distanční matice M5 a M6 z nerezové oceli A2, o tloušťce 4 mm.

Ze zbytku první překližky o rozměru 38,5 × 29,5 cm budou vyrobeny lišty. Ty budou pomocí vrutů připevněny ke spodní straně vážicí desky. Lišty budou vysoké 15 mm, protože vzdálenost mezi deskami činí 20,7 mm a celá horní konstrukce musí být podepřena pouze tenzometrem. Současně musí být zachována vůle alespoň 3 mm.

Do zadní lišty bude vyvrtán otvor, kterým bude protažen již prodloužený kabel od tenzometru. Kabel nesmí být mechanicky namáhán tahem. Dále bude veden vzhůru po stěně do krabičky Kx. Z boku krabičky bude vyveden sdělovací kabel, který povede vysoko po stěně až do krabičky K.

Celá konstrukce musí být od stěny vzdálena natolik, aby se jí nikde nedotýkala a současně za ní zůstal prostor pro vedení kabelu. Zároveň je nutné zabránit přístupu slepic a hlodavců do tohoto prostoru.

Uprostřed konstrukce naproti košíku bude vytvořen stavitelný doraz pomocí zápustného inbusového šroubu M5 z nerezové oceli A2 o délce 30 mm. Šroub bude procházet spodní deskou skrz otvor o průměru 4,5 mm a bude zapuštěn do záhlubení o průměru 10 mm a hloubce 6 mm. Doraz bude tvořen mosaznou kloboučkovou maticí M5 (délka 10 mm, závit 7,5 mm) z důvodu omezení ulpívání nečistot. Optimální vůle dorazu je 0,8 mm, což odpovídá jednomu otočení šroubu M5 nebo přibližně tloušťce běžné platební karty.

## Nákup

**Kabeláž**  
https://www.gme.cz/v/1512358/elektrokabel-cya-1x15-cerveny-h07v-k-izolovany-vodic-lanko (5m)  
https://www.gme.cz/v/1512357/elektrokabel-cya-1x15-cerny-h07v-k-izolovany-vodic-lanko (5m)  
https://www.gme.cz/v/1512382/elektrokabel-cya-1x05-cerveny-h05v-k-izolovany-vodic-lanko (5m)  
https://www.gme.cz/v/1512360/elektrokabel-cya-1x05-cerny-h05v-k-izolovany-vodic-lanko (5m)  

https://www.alza.cz/alzapower-patch-cat5e-utp?dq=6592131 (5 m)  

Willmann Elektro Lipník (5 m)  

**Lišta**  
https://www.hornbach.cz/p/lista-na-kabely-malpro-15x10-mm-2m-bila/6647360/ (4 m)  

**Klipy**  
https://www.hornbach.cz/p/kabelova-prichytka-6-mm-cerna-baleni-30-ks/5562029/  

**Bužírky**  
https://www.gme.cz/v/1483738/kss-vs-100bk-sada-smrstovacich-buzirek  

**Svorkovnice a svorka**  
https://www.gme.cz/v/1499112/wago-256-404-svorkovnice-4pol-roztec-508mm-24a-320v-vstup-45-pruzina (2 ks)  
https://www.gme.cz/v/1502753/wago-250-405-svorkovnice-5pol-roztec-25mm-4a-160v-vstup-45-pruzina  
https://www.gme.cz/v/1513875/wago-221-2411-svorka-bezsroubova-2pol-vodic-do-4mm2 (4 ks)  

**Konektor**  
https://www.hadex.cz/p/d626-zdirka-rj45-do-dps-8p8c (4 ks)  

**Solární panel**  
https://www.dexhal.cz/fotovoltaicky-panel-9v-1110ma-10w-solarni-clanek_z2900/  

**N-MOS**  
https://www.hadex.cz/p/b441-2n7000-mosfet-n-fet-60v-0-35a-to92 (10 ks)  

**P-MOS**  
https://www.gme.cz/v/1493257/infineon-irf4905pbf-unipolarni-tranzistor (10 ks)  

**Akumulátor**  
https://www.levne-baterky.cz/Green-Cell-AGM-Baterie-6V-4Ah-d5516.htm  

**Lineární LDO regulátor**  
https://www.elektro-hofman.cz/stabilizator-pevneho-napeti-mcp1702-3302et-3-3v-0-25a-low-drop-to92/ (2 ks)  

**Master**  
https://botland.cz/lora-lorawan/19267-lora-e5-mini-stm32wle5jc-modul-lorawan-868-915-mhz-seeedstudio-113990939-5904422369026.html  

**Slave**  
https://botland.cz/stm32-nucleo/18799-stm32-nucleo-l031k6-s-stm32l031k6-mcu-kompatibilni-s-arduino-nano-5904422364878.html (2 ks)  

**Header**  
https://www.laskakit.cz/dupont-40pin-2-54-mm-pinovy-pas/ (4 ks)  

**Proudový a napěťový senzor**  
https://dratek.cz/arduino-platforma/1437-ina219-proudovy-snimac-obousmerny.html  

**H-bridge**  
https://botland.cz/ovladace-stejnosmerneho-motoru/2695-drv8838-jednokanalovy-budic-motoru-11v-17a-pololu-2990-5903351244855.html  

**Motor**  
https://dratek.cz/arduino-platforma/3197-stejnosmerny-motor-s-prevodovkou-dc-6v-20-rpm.html  

**Mikrospínač**  
https://www.hadex.cz/p/l405a-mikrospinac-kw11-3z-on-on-1pol-250v-5a-s-packou-13mm (4 ks)  

**Dvířka**  
https://eshop.zenit.cz/desky/pvc-penene-desky/palight-print-8mm/variant/2030-3050/  

**Hnízdo**  
https://www.lihneme.cz/plastova-snaskova-hnizda/hf44x37x50-zelene/ (2 ks)  

**Tenzometr**  
https://www.laskakit.cz/vazici-senzor-20kg-adc-modul-hx711/ (2 ks)  

**Převodník**  
https://www.laskakit.cz/ad-prevodnik-modul-24-bit-2-kanaly-hx711/ (2 ks)  

**Transceiver**  
https://www.soselectronic.com/cz-cz/products/maxim/max3485esa-1-121397 (3 ks)  

**Adaptér**  
https://dratek.cz/arduino-platforma/1046-dps-adapter-sop8-so8-soic8-na-dip8.html (3ks)  

**Kondenzátor**  
https://www.gme.cz/v/1486151/hitano-ck-1u-50v-x7r-rm508-10-keramicky-kondenzator (4 ks)  

**Pole**  
https://www.laskakit.cz/velke-nepajive-kontaktni-pole-s-napajecimi-svorkami-2860-pinu/  

**Deska**  
https://dratek.cz/arduino-platforma/1047-univerzalni-plosny-spoj-30mm-x-70mm.html  

**Překližka**  
Hornbach (4 ks)  

**Červík**  
Hornbach (2 ks)  

**Lanko**  
Hornbach  

**Očko**  
Hornbach  

**Šrouby Matice Podložky**  
Hornbach (11, 11, 2)  

**PU tmel**  
Hornbach  

|               |             |
| :---          | :---:       |
| **OBCHOD**    | **CENA**    |
| GME           | 650 Kč      |
| ALZA          | 160 Kč      |
| HADEX         | 180 Kč      |
| HORNBACH      | 800 Kč      |
| DEXHAL        | 590 Kč      |   
| BATERKY       | 270 Kč      |
| HOFMAN        | 125 Kč      |
| LASKAKIT      | 700 Kč      |
| DRATEK        | 380 Kč      |
| BOTLAND       | 1880 Kč     |
| ZENIT         | 100 Kč      |
| LIHNEME       | 1150 Kč     |
| SOS           | 450 Kč      |
| REZERVA       | 500 Kč      |
| **CELKEM**    | **8825 Kč** |

*Poznámka: Cena je orientační a je do ní započtena i doprava.*
