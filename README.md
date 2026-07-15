# Bakalářská práce
Systém pro automatizaci kurníku s detekcí snesených vajec

## Zadání
- Prostudujte možnosti automatizace uzavírání kurníku, možné metody detekce snesených vajec a dostupné možnosti komunikace a připojení zařízení do IoT sítě (např. NB-IoT, WiFi, LoRaWAN).
- Navrhněte systém pro automatické otevírání a uzavírání dvířek kurníku s možností rozšíření o jednotky se senzory ve snáškových hnízdech (min. 5 hnízd). Prostudujte možnosti napájení z akumulátoru nebo fotovoltaického panelu. Proveďte analýzu možnosti přenosu dat do cloudu a jejich zobrazení uživateli včetně historie snášek a možnosti vzdáleného ovládání, například skrze aplikaci.
- Na základě analýzy navrhněte způsob vzdálené uživatelské interakce pro zobrazení stavu dvířek, manuální ovládání a počítání snesených vajec.
- Navrhněte schéma zařízení, které bude obsahovat mikrokontrolér, komunikační modul, senzory, řízení dvířek, napájecí/nabíjecí obvody. Dbejte na nízký příkon celého zařízení. Proveďte návrh desky plošných spojů.
- Zkonstruujte hardwarovou realizaci navrženého systému. Osaďte a zprovozněte řídicí elektroniku. Vytvořte ovládací firmware. Realizujte přenos dat ze zařízení k uživateli včetně ovládacího rozhraní a možnosti zobrazení historie.
- Ověřte funkčnost systému experimentálním měřením a vyhodnoťte spolehlivost detekce a ovládání.
- Zveřejněte veškeré výrobní podklady na vhodné platformě (např. GitHub).

## Schéma
<img src="https://github.com/Hosty-04/Bachelors_Theses/blob/main/schematics/block_schematic.png" alt="block_schematic">

## Popis

### Krabičky
Systém bude obsahovat jednu krabičku pro akumulátor o tloušťce stěny 2,4 mm a jednu krabičku (K) o tloušťce stěny 1,6 mm určenou pro mechaniku a elektroniku dvířek a hlavní mikrořadič. Pro každé snáškové hnízdo je dále vyhrazena samostatná krabička (Kx), rovněž s tloušťkou stěny 1,6 mm, obsahující potřebnou elektroniku a mikrořadič.

Všechny krabičky budou vytištěny na 3D tiskárně z materiálu PETG, zvoleného pro vysokou houževnatost a teplotní odolnost. Zadní stěna krabiček bude ve svislém směru prodloužena a bude mít tloušťku 3,5 mm; v každém rohu této stěny vzniknou otvory pro vruty, které umožní upevnění krabiček ke stěně kurníku.

Na krabičkách K a Kx budou dále u víka vytvořeny nízké drážky ve tvaru písmene U s mírně zaoblenými rohy, sloužící k protažení kabelů. Pro kabel o průřezu 1,5 mm² budou mít šířku 4,5 mm, hloubku 3 mm a zaoblení dna i hran 1 mm; pro kabel o průřezu 0,5 mm² šířku 3,5 mm, hloubku 2 mm a zaoblení dna i hran 0,75 mm; pro datový kabel pak šířku 6 mm, hloubku 3,5 mm a stejné zaoblení jako u prvního kabelu.

Krabička pro akumulátor bude mít ve spodní části dva kruhové otvory o průměru 5 mm určené pro průchod napájecích kabelů. Krabička Kx bude mít zespodu jeden otvor o průměru 6 mm pro průchod kabelu od tenzometru, krabička K pak otvor o průměru 3 mm pro lanko. U krabičky pro akumulátor budou navíc dva větrací otvory o průměru 4 mm, umístěné v horní a spodní části tak, aby uvnitř mohl cirkulovat vzduch.

Víko bude vytištěno samostatně s přesahem 2 mm; uprostřed jeho levé hrany bude dutá trubička o průměru otvoru 2 mm. Stejná trubička bude na obou koncích vnější horní hrany levé stěny u všech krabiček. Po vytištění bude všemi třemi trubičkami protažen filament o průměru 1,75 mm, jehož konce se krátce nahřejí zapalovačem a roztepou do podoby hlaviček. Tím vznikne jednoduchý pant pro uchycení víka — řešení, které minimalizuje počet součástek a umožňuje vyrobit celou krabičku bez dalších kovových dílů.

### Kabeláž
Pro připojení solárního panelu bude použita měděná ohebná licna o průřezu 1,5 mm², připojená přes 4pinovou pružinovou WAGO svorkovnici s roztečí 5,08 mm k desce plošných spojů a odtud dále k akumulátoru. Tento průřez poskytuje dostatečnou proudovou rezervu při minimálním úbytku napětí.

Stejným způsobem budou k desce plošných spojů připojeny mikrospínače, avšak s licnou o průřezu 0,5 mm². Spoje této licny s kontakty mikrospínače budou izolovány smršťovacími bužírkami s poměrem 2:1 a vnitřním průměrem 2 mm / 0,75 mm (před / po zahřátí), přečnívajícími asi centimetr přes izolaci. U prototypu se využijí drátky do nepájivého pole stejného průřezu, který plně vyhovuje proudovému odběru systému.

Kabely povedou v klasické elektroinstalační PVC liště o rozměrech 15 × 10 mm — dostatečně prostorné, a přitom minimalistické. Speciální UV odolná lišta není potřeba, protože stěna kurníku směřuje na severozápad; životnost běžné lišty se odhaduje na 5 až 10 let.

Pro datovou komunikaci byl zvolen kabel UTP CAT5e typu licna, upevněný ke stěně pomocí 6mm šroubovacích příchytek, vrutu 4 × 20 mm a 6mm hmoždinky, a k desce plošných spojů připojený přes konektory RJ45. Prodloužení vodičů tenzometru zajistí stíněný kabel LiYCY 4 × 0,25 mm² typu licna, spojený přes prodlužovací pružinové WAGO svorky. Protože jsou vodiče od tenzometru (průřez 0,14 mm²) pro tyto svorky příliš tenké, přehnou se u odizolovaného konce napůl, čímž se jejich efektivní průřez zdvojnásobí na 0,28 mm². K desce plošných spojů bude tento kabel spolu se stíněním připojen přes 5pinovou pružinovou WAGO svorkovnici s roztečí 2,5 mm.

Oplet kabelu (pocínované měděné drátky) se izoluje smršťovací bužírkou 2:1 o vnitřním průměru 4 mm / 1,75 mm. Odmotaný a spletený oplet je potřeba oddělit od zbylých čtyř vodičů a bužírku navléknout až ke kořenu; přes celý kabel se pak přetáhne bužírka 2:1 o rozměru 7,5 mm / 3,5 mm, přečnívající asi centimetr přes hlavní izolaci.

### Napájení
Výrobu energie zajistí fotovoltaický panel o parametrech 9 V / 10 Wp, svisle připevněný na stěnu pod malou stříšku a orientovaný na jih, případně na východ nebo západ (v tomto případě na jihozápad), aby co nejlépe využíval dostupnou sluneční energii. Vertikální montáž zároveň omezí usazování sněhu a nečistot. Tento panel byl zvolen proto, že při použití jednoduchého MOSFET odpojovače poskytuje vhodný poměr mezi napěťovou rezervou pro nabíjení 6V akumulátoru a dostupným nabíjecím proudem; jeho vyšší výkon navíc zvyšuje energetickou rezervu systému v zimě, kdy je intenzita slunečního záření nízká.

Systém bude napájen z bezúdržbového olověného AGM akumulátoru 6 V / 4 Ah, umístěného venku ve stínu asi 25 cm pod stříškou. Jeho nabíjecí účinnost se pohybuje kolem 88 %. I se zohledněním zimního poklesu kapacity o 30 % představuje jeho energetická rezerva několik stovek dní provozu — v praxi bude provozní dobu omezovat spíše několik týdnů nepříznivého počasí v kombinaci se samovybíjením 3 % za měsíc a přirozeným stárnutím než samotná spotřeba systému. Akumulátor typu LiFePO4 je sice v mnoha ohledech kvalitnější, nesmí se však nabíjet v zimě pod 0 °C a vyžaduje složitější nabíjecí systém. Vzhledem k volbé venkovního umístění a jednoduchého nabíjecího systému, je pro celoroční provoz vhodnější olověný akumulátor. Je důležité mít na paměti životnost kolem 5 let a 15 % ztrátu kapacity ročně.

Před akumulátorem bude zapojen nízkopříkonový, mikrořadičem řízený MOSFET odpojovač fotovoltaického zdroje s ochranou akumulátoru. Od použití MPPT regulátoru se ustoupilo kvůli vyšší složitosti a vlastní spotřebě spínaného měniče — u systému s velmi nízkým denním odběrem by zlepšení účinnosti nabíjení, probíhajícího jen několik minut denně, nepřineslo oproti jednoduchému odpojovači s téměř nulovou klidovou spotřebou žádný významný energetický přínos. Ztráty neidealního pracovního bodu jsou asi 15 % a ztráty použití jednoduchého algoritmu oproti maximálnímu využití dostupné energie jsou spolu se ztrátami na tranzistorech asi 10%.

Silová část systému bude pracovat s napětím 6 V, veškerá elektronika pak s napětím 3,3 V. Snížení napětí zajistí nízkopříkonový LDO regulátor MCP1702 s velmi nízkým klidovým proudem a dostačujícím výstupním proudem 250 mA. Na jeho vstupu i výstupu bude připojen blokovací keramický kondenzátor 1 µF / 50 V — na vstupu jako filtrace, na výstupu pro stabilizaci napětí.

Použití spínaného buck měniče není vhodné kvůli horší dostupnosti nízkopříkonových variant a velmi nízkému odběru systému po většinu dne. Jeho vyšší účinnost by se projevila jen po několik minut denně a kvůli vlastní spotřebě by paradoxně dosahoval nižší celkové účinnosti než jednoduchý lineární LDO regulátor.

## Denní přehled (5 hnízd)

### Klidová spotřeba (24 hod)

| Komponenta | Proud (typ) | Proud (max) | Spotřeba (typ) | Spotřeba (max) |
|:---|:---:|:---:|:---:|:---:|
| LDO (MCP1702) | 2 µA | 5 µA | 48 µAh | 120 µAh |
| M (Stop2 s RTC) | 1,0 µA | 26 µA | 24 µAh | 67,2 µAh |
| 5 × Mx (Stop bez RTC) | 1,9 µA | 9,5 µA | 45,6 µAh | 45,6 µAh |
| **Celkem** | **4,9 µA** | **9,7 µA** | **118 µAh** | **233 µAh** |

*Poznámka: Ostatní části systému jsou odpojovány přes tranzistorové spínače.*

### Kontrola panelu a akumulátoru (4 s a 3 s)

| Komponenta | Proud (typ) | Proud (max) | Spotřeba (typ) | Spotřeba (max) |
|:---|:---:|:---:|:---:|:---:|
| M (LPRun @ 1 MHz) | 120 µA | 390 µA | 0,244 µAh | 0,244 µAh |
| Dělič | | | | |
| INA219 aktivní | 0,7 mA | 1 mA | 0,583 µAh | 0,833 µAh |
| **Celkem** | **0,92 mA** | **1,22 mA** | **0,83 µAh** | **1,08 µAh** |

### Pohyb dvířek (28 s nebo 42 s)

| Komponenta | Proud (typ) | Proud (max) | Spotřeba (typ) | Spotřeba (max) |
|:---|:---:|:---:|:---:|:---:|
| Motor | 150 mA | 200 mA | 1,17 mAh | 2,33 mAh |
| DRV8838 | 340 µA | 600 µA | 2,64 µAh | 7,00 µAh |
| M (LPRun @ 1 MHz) | 120 µA | 390 µA | 1,71 µAh | 2,57 µAh |
| INA219 | 0,7 mA | 1 mA | 5,44 µAh | 11,7 µAh |
| **Celkem** | **151 mA** | **202 mA** | **1,18 mAh** | **2,35 mAh** |

*Poznámka: Možnost zaseknutí dvířek je brána v potaz.*

### Kontrola vajec (8 min)

| Komponenta | Proud (typ) | Proud (max) | Spotřeba (typ) | Spotřeba (max) |
|:---|:---:|:---:|:---:|:---:|
| M (LPRun @ 1 MHz) | 120 µA | 390 µA | 29,3 µAh | 29,3 µAh |
| MAX3485 (M) | 1,1 mA | 2,2 mA | 147 µAh | 293 µAh |
| MAX3485 (Mx) | 1,1 mA | 2,2 mA | 147 µAh | 293 µAh |
| Mx (LPRun @ 131 kHz) | 32 µA | 37 µA | 4,27 µAh | 4,93 µAh |
| HX711 a tenzometr | 4,4 mA | 4,4 mA | 0,587 mAh | 0,587 mAh |
| **Celkem** | **6,85 mA** | **9,06 mA** | **0,914 mAh** | **1,21 mAh** |

*Poznámka: STM32 NUCLEO-L031K6, MAX3485, HX711 a tenzometr jsou přítomny v každé krabičce Kx, ale díky chytrému využití tranzistorových spínačů a režimů řadiče je zapnuté vždy jen to, co zrovna pracuje — proudový odběr se tak chová, jako by v kurníku bylo jediné hnízdo, což znamená přibližně pětkrát nižší spotřebu.*

### LoRa TX (4 s)

| Komponenta | Proud (typ) | Proud (max) | Spotřeba (typ) | Spotřeba (max) |
|:---|:---:|:---:|:---:|:---:|
| Rádio TX (SMPS) | 21 mA | 21 mA | 23,3 µAh | 23,3 µAh |
| CPU (LPSleep) | 44 µA | 310 µA | 0,049 µAh | 0,077 µAh |
| **Celkem** | **21 mA** | **21,1 mA** | **23,4 µAh** | **23,4 µAh** |

### LoRa RX (11 s)

| Komponenta | Proud (typ) | Proud (max) | Spotřeba (typ) | Spotřeba (max) |
|:---|:---:|:---:|:---:|:---:|
| Rádio RX (SMPS) | 4,8 mA | 4,8 mA | 14,7 µAh | 14,7 µAh |
| CPU (LPSleep) | 44 µA | 310 µA | 0,134 µAh | 0,211 µAh |
| **Celkem** | **4,84 mA** | **4,87 mA** | **14,8 µAh** | **14,9 µAh** |

*Poznámka: Bezpečný odhad délky okna je 200 ms.*

### Procentuální rozložení a celková denní spotřeba

| Blok | Spotřeba (typ) | Podíl | Spotřeba (max) | Podíl |
|:---|:---:|:---:|:---:|:---:|
| Pohyb dvířek | 1,18 mAh | 52,3 % | 2,35 mAh | 61,4 % |
| Kontrola vajec | 914 µAh | 40,7 % | 1,21 mAh | 31,5 % |
| Klidový režim | 118 µAh | 5,3 % | 233 µAh | 6,1 % |
| LoRa TX | 23,4 µAh | 1,0 % | 23,4 µAh | 0,6 % |
| LoRa RX | 14,8 µAh | 0,7 % | 14,9 µAh | 0,4 % |
| Kontrola panelu / baterie | 0,83 µAh | 0,0 % | 1,08 0,38µAh | 0,0 % |
| **Celkem** | **2,25 mAh** | 100 % | **3,84 mAh** | 100 % |

### Výrobená energie

| Orientace | Léto | Zima |
|:---|:---:|:---:|
| Jih | | |
| Východ | | |
| Západ | | |
| Jihozápad | | |

*Poznámka: Energie dodávaná do akumulátoru přes oddělovač. Do výpočtů byly zahrnuty ztráty neideálního pracovního bodu fotovoltaického panelu, použití a ztráty jednoduchého regulátoru, přibližná nabíjecí účinnost akumulátoru, svislá poloha panelu a běžné klimatické podmínky v ČR*

### Energetická bilance

| Orientace panelu | Léto | Zima | Hodnocení |
|:---|:---:|:---:|:---|
| Jih | | | Velmi vysoká energetická rezerva |
| Východ | | | Spolehlivý celoroční provoz s dostatečnou rezervou |
| Západ | | | Spolehlivý celoroční provoz s dostatečnou rezervou |
| Jihozápad | | | Spolehlivý celoroční provoz |

*Poznámka: Počítáno je i se samovybíjením akumulátoru.*

### Řízení
Hlavní řídicí jednotkou systému bude mikrořadič STM32WLE5JC LoRa-E5 mini (M) s integrovaným LoRa modulem, komunikujícím přes LoRaWAN stack. Technologie LoRaWAN umožní na rozdíl od Wi-Fi komunikaci na velké vzdálenosti při nízké spotřebě energie a na rozdíl od NB-IoT trvalé řešení s dobrým pokrytím. U každého snáškového hnízda bude umístěn další mikrořadič STM32 NUCLEO-L031K6 (Mx), který má integrovaný programátor využitelný i pro hlavní řadič.

Firmware bude vyvíjen v prostředí STM32CubeIDE. Součástí firmwaru hlavního řadiče budou astronomické hodiny, které každý den pomocí RTC obvodu spočítají čas východu a západu slunce; podle těchto údajů se pak budou automaticky otevírat a zavírat dvířka kurníku. Přes RTC hodiny bude řadič schopen zjistit i roční období. Použití prostého časovače nebylo zvoleno kvůli proměnlivé délce dne, a světelný senzor byl zavržen proto, že by mohl vyvolat chybné sepnutí motoru dvířek při zatažené obloze (déšť, bouřka) nebo vlivem pouličního osvětlení či světlometů automobilů. Konfiguraci periferií a hodin zajistí grafický nástroj STM32CubeMX.

Hlavní řídicí jednotka se bude společně s nezbytnými částmi systému probouzet každých 10 minut, aby zkontrolovala stav solárního panelu a akumulátoru. Dále se bude spolu s ostatními řídicími jednotkami a dalšími potřebnými částmi systému probouzet každou hodinu, kdy postupně, hnízdo po hnízdu, provede kontrolu stavu vajec; po sběru dat ze všech hnízd následuje komunikace. Nakonec se bude probouzet ráno a večer — opět pouze s nezbytnými částmi systému — kvůli otevření a zavření dvířek. Před zahájením pohybu dvířek budou piny koncových spínačů nastaveny jako digitální vstupy; po dosažení koncové polohy proběhne komunikace a piny se přepnou do analogového režimu bez pull rezistorů, čímž se eliminuje jejich klidový odběr.

LoRa anténa bude moci vysílat teprve po vypnutí všech ostatních systémů, a to kvůli jejímu vyššímu odběru proudu a ochraně proti rušení. Po každém vysílání bude mít možnost přijímat data, což umožní uživatelské ovládání. Upřednostňované parametry komunikace jsou: vysílací výkon 12 dBm, SF9, šířka pásma 125 kHz, kódovací poměr 4/5, dvě příjmová okna RX1 a RX2, jejichž délka se automaticky přizpůsobuje driftu krystalu.

V domě bude umístěna LoRaWAN gateway, plnící funkci internetové brány přes Wi-Fi. Veškerá přijatá data budou odeslána do cloudu (TTN) a odtud přes MQTT na backend (Node.js), který je uloží do databáze (InfluxDB) a zobrazí na frontendu. Při odesílání dat z gateway do kurníku probíhá proces obráceně.

Data budou z kurníku odesílána ve třech a více bajtech. První bajt ponese 1 bit pro indikaci zapnutí/vypnutí systému a 7 bitů pro napětí solárního panelu (rozsah 0–9 V, krok cca 0,1 V). Druhý bajt bude obsahovat 5 bitů pro napětí akumulátoru (5,75–8 V, krok cca 0,1 V), 1 bit pro oznámení kritického stavu (nízké napětí akumulátoru) a 2 bity pro stav dvířek (otevřeno / zavřeno / zaseknuto). Další bajty budou po čtyřech bitech alokovány pro počet vajec v jednotlivých snáškových hnízdech (0–10 vajec na hnízdo). Kurník bude data přenášet každou hodinu po kontrole stavu hnízd a dvakrát denně při změně stavu dvířek — v tom případě odešle pouze první dva bajty. Manuální ovládání využije jediný bajt: jeden bit pro vypnutí/zapnutí systému, druhý pro vypnutí/zapnutí kritického režimu a poslední pro ovládání dvířek (otevřít/zavřít).

Pro přenos dat mezi hlavní řídicí jednotkou a ostatními řídicími jednotkami bude použit protokol LPUART, který nevyžaduje hodinový signál a vyznačuje se nízkou spotřebou energie. Přenosová rychlost bude 9600 Bd kvůli minimalizaci odrazů. Na aplikační vrstvě poslouží protokol Modbus RTU spolu s knihovnou ModbusRTU-Slave — Modbus RTU vytvoří datový rámec obsahující adresu jednotky slave, přenášená data a kontrolní součet CRC pro detekci chyb při přenosu. Hardware LPUART v řadiči následně převede jednotlivé bajty na sériový datový tok, doplní start a stop bity a zajistí jejich přenos po sběrnici; na straně přijímače proběhne opačný proces.

**Stavový automat pro algoritmus detekce snesených vajec**
- Probuzení
- Čekání 500 ms na ustálení měření
- Odebrání 32 vzorků rychlostí 10 SPS
- Výpočet mediánu
- Výběr 16 vzorků s nejmenší odchylkou od mediánu
- Výpočet aritmetického průměru, aktuální hmotnosti (odečet referenční nulové hodnoty) a směrodatné odchylky
- Pokud hmotnost překročí 1 kg (v hnízdu je slepice), měření se zahodí
- Pokud odchylka překročí stanovený práh (pohyb slepice, vibrace), měření se zahodí
- Je-li měření stabilní, aktuální hmotnost se porovná s uloženou hodnotou
- Odpovídá-li rozdíl hmotnosti přibližné hmotnosti jednoho (60 g) nebo více vajec, změna se aritmeticky přičte k uložené hodnotě
- Při hmotnosti menší než 25 g proběhne kontrola driftu — zaznamenají-li se tři po sobě jdoucí stabilní měření, aktualizuje se referenční nulová hodnota
- Uspání

Většinu dne bude hlavní řídicí jednotka v režimu Stop2 s RTC. Tento režim se vyznačuje velmi nízkou spotřebou a na rozdíl od režimu StandBy s RTC dokáže mimo jiné udržet logické úrovně a nastavení pinů. Řadič je taktován externím krystalem LSE, umístěným na LoRa-E5 mini, na 32 kHz. Jakmile ale RTC hodiny signalizují, že je čas na práci, přepne se řadič do režimu LP Run (Low-Power Run), taktovaného interním krystalem LSI na 1 MHz. V průběhu LoRa přenosu (Radio TX/RX) se CPU přepne do režimu LP Sleep (LSI, 1 MHz). Kvůli nízké taktovací frekvenci je potřeba v souboru `lorawan_conf.h` zvýšit `RADIO_WAKEUP_TIME` z 2 na 5 ms; rádio poběží automaticky na 32 MHz a po skončení přenosu bude nutné jej nastavit do režimu LP Sleep.

U ostatních řídicích jednotek to bude po většinu dne velmi podobné — ze stejných důvodů, a protože je potřeba uchovat obsah paměti RAM, se budou nacházet v úsporném režimu Stop bez RTC. Řadiče budou probouzeny postupně, jeden po druhém, přes hlavní řadič a sběrnici LPUART, díky čemuž nepotřebují vlastní RTC hodiny. Po probuzení se daný řadič přepne do režimu LP Run (LSI, 131 kHz) a ihned po vykonání úkolu se vrátí zpět do režimu Stop bez RTC.

Přechod mikrořadičů mezi úspornými režimy (Stop2/Stop) a režimem LP Run trvá řádově stovky mikrosekund včetně obnovení systémových hodin. Ve srovnání s dobou měření senzorů (desítky milisekund až sekundy) je tato doba zanedbatelná, a proto se v energetické bilanci neuvažuje.

Před odpojením napájení VCC jednotlivých částí systému je nutné přepsat výstupy (SCK, PH, EN, DI, DE, RE) na logickou nulu, aby se zamezilo napájení přes výstupy tomu neurčené (tzv. leakage current). Dále je třeba vypnout periferie (I²C, UART, ADC) i jejich hodinový signál, který plýtvá energií, i když právě nic nepřenáší. Po odpojení VCC je pak nutné nastavit všechny piny — včetně těch pro právě vypnuté periferie — do analogového režimu bez pull rezistoru.

Kvůli spotřebě bude nutné odpájet červenou Power LED diodu a softwarově odpojit všechny zelené User LED diody. Programátor ST-Link musí být za provozu hardwarově odpojen, proto je potřeba odpájet pájecí jumpery SB9, SB14, SB2 a SB3. U LoRa-E5 mini je navíc nutné přepnout piny PA2 a PA3, určené k ladění, do analogového režimu bez pull rezistoru. Pro programování lze pak programátor k deskám připojovat přes klasické Dupont kabely, a u LoRa-E5 mini vrátit piny PA2 a PA3 do původního stavu. Zvláštní pozornost je třeba věnovat plovoucím pinům — nepoužívané piny musí být vždy v analogovém režimu bez pull rezistoru. Při nepoužívání rádia je nutné nastavit externí RF switch na logickou nulu; u řadičů bez rádia je zase potřeba v registrech napájení (PWR) aktivovat ultra-low-power režim (bit ULP) a vypnout fast wakeup.

### Elektronika
Prototyp bude sestaven z modulů umístěných na nepájivém poli pomocí kolíkových lišt. Finální verze bude obsahovat jednu hlavní desku plošných spojů a několik (v tomto případě dvě, obecně například pět) vedlejších desek pro jednotlivá snášková hnízda. Na všech deskách budou moduly nahrazeny čipy a nezbytnými externími SMD součástkami.

MOSFET odpojovač bude tvořen dvěma P-MOS tranzistory zapojenými back-to-back (drainy proti sobě). Toto zapojení umožňuje úplné odpojení kladného napájecího napětí při zachování společné země celého systému a zamezuje zpětnému toku proudu z akumulátoru do panelu, způsobenému parazitními diodami P-MOS tranzistorů. Tyto tranzistory bude řídit budicí logic-level N-MOS tranzistor, protože napětí 3,3 V nemusí být vždy dostatečné pro ovládání P-MOS tranzistoru napájeného z 9V solárního panelu. Na gate N-MOS tranzistoru bude sériově zapojen ochranný rezistor 220 Ω a mezi gate a společnou zem paralelně pull-down rezistor, zabraňující vzniku nedefinovaného logického stavu. Drain bude připojen na gate obou P-MOS tranzistorů a přes pull-up rezistor k 9V panelu, source N-MOS tranzistoru pak ke společné zemi. Napětí U<sub>GS</sub> u P-MOS tranzistorů tak bude při rozepnutém N-MOS tranzistoru nulové, při sepnutém vždy nižší než −4,5 V.

Co nejblíže za MOSFET odpojovačem budou v krabičce K paralelně zapojeny dva kondenzátory: elektrolytický 47 µF/25 V jako zásobárna energie a blokovací keramický 100 nF/50 V pro filtraci vysokofrekvenčního elektromagnetického rušení.

Pro dosažení nízké klidové spotřeby budou jednotlivé části systému napájeny přes tranzistorové spínače — většina elektroniky totiž pracuje jen krátkodobě, při měření, komunikaci nebo pohybu dvířek, a trvalé napájení všech obvodů by způsobovalo zbytečný odběr energie z akumulátoru. Spínače budou konstruovány stejně jako MOSFET odpojovač, ale jen s jedním P-MOS tranzistorem a odpovídajícími pull-up a pull-down rezistory. Přes první z těchto spínačů bude M řídit napájení k děliči napětí, přes druhý k INA219, přes třetí k DRV8838 a čtvrtý bude napájet hlavní MAX3485 a zároveň všechny krabičky Kx. V každé krabičce Kx budou další dva spínače: první, ve výchozím stavu sepnutý, bude přes Mx napájet místní MAX3485 a HX711; druhý, ve výchozím stavu rozepnutý, bude napájet další krabičku Kx v řadě.

Po zapnutí napájení jednotlivých částí systému je nutné počkat na jejich ustálení. U obvodu INA219 se použije čekací doba 200 µs, zahrnující náběh napájení, stabilizaci obvodu a nabití blokovacího keramického kondenzátoru 100 nF mezi VCC a GND. Při 12bitovém měření s průměrováním 32 vzorků trvá vytvoření první hodnoty přibližně 17 ms (32 × 532 µs), při měření proudu s průměrováním 4 vzorků pak přibližně 2,1 ms. U obvodu MAX3485 se použije čekací doba 100 µs (náběh obvodu a nabití blokovacího kondenzátoru 100 nF mezi VCC a GND), u budiče DRV8838 pak 3 ms, což zahrnuje nabití elektrolytického kondenzátoru 47 µF a keramického 100 nF mezi VM a GND, a především ustálení interní nábojové pumpy. U obvodu HX711 bude po zapnutí napájení potřeba čekat přibližně 500 ms — dobu ustálení analogové části převodníku a dokončení prvního převodu — po níž už lze odečítat stabilní hodnoty; při zvoleném režimu 10 SPS trvá jedna konverze přibližně 100 ms.

K solárnímu panelu bude připojen vysokoimpedanční napěťový dělič tvořený rezistory 1 MΩ a 470 kΩ, přičemž paralelně k rezistoru R2 (470 kΩ) bude zapojen blokovací keramický kondenzátor 10 nF/50 V. Dělič bude sloužit ke snímání napětí panelu; naměřené hodnoty se do M přenesou přes ADC pin v analogovém režimu a pro zvýšení přesnosti bude provedena kalibrace, výsledek pak bude aritmetickým průměrem 16 vzorků. Vysoká impedance děliče zajišťuje jeho nízkou spotřebu. Běžně používaný modul proudového a napěťového senzoru INA219 bude v krabičce K zapojen mezi akumulátor a vstup VM pro napájení motoru přes H-bridge; jednou z jeho funkcí bude s 12bitovým rozlišením a průměrováním 32 vzorků snímat napětí akumulátoru.

Na základě údajů z tohoto modulu a z napěťového děliče bude M přes sběrnici I²C, respektive přes port GPIO, vyhodnocovat stav akumulátoru a solárního panelu. Dostane-li se napětí akumulátoru nad limitní hodnotu (v létě 7,2 V, na jaře a na podzim 7,3 V, v zimě 7,5 V), M solární panel odpojí. Pokud napětí akumulátoru následně klesne o  250 mV po dobu 30 minut (tři po sobě jdoucí měření), M panel znovu připojí. Při kritickém vybití akumulátoru, kdy jeho napětí klesne na 5,75 V, přejde M do kritického režimu, ve kterém bude už jen kontrolovat napětí panelu a akumulátoru; k obnovení provozu dojde po dosažení 6,1 V. Během nedostatečného slunečního svitu nebo v noci, kdy je napětí panelu nižší než napětí akumulátoru, musí M zamezit vzniku zpětného proudu směrem do panelu jeho odpojením; kvůli nepřesnosti měření bude hladina pro odpojení, respektive opětovné připojení panelu zvýšena o 250 mV.

Senzor INA219 bude zároveň sloužit jako proudový snímač s 12bitovým rozlišením a průměrováním 4 vzorků. Při pohybu dvířek bude M monitorovat proud odebíraný z akumulátoru; náhlé zvýšení proudu nad 225 mA u tří po sobě jdoucích měření bude signalizovat zablokování dvířek nebo překážku v cestě (typicky slepici). V takovém případě se M na 250 ms zastaví, pokusí se obrátit směr otáčení motoru a vrátit dvířka do původní polohy, poté se uspí a po 10 minutách pokus zopakuje. Nepomůže-li ani zpětný chod, systém odešle zprávu o zaseknutí dvířek a do uživatelského pokynu s nimi nebude manipulovat. Krátkodobé proudové špičky při rozběhu motoru, trvající asi 150 ms, je nutné ignorovat.

M bude dále přímo (bez PWM modulace) řídit běžný H-bridge Pololu DRV8838, vybavený elektrolytickým kondenzátorem s nízkým ESR (47 µF/25 V) zapojeným co nejblíže mezi piny VM a GND, který potlačí indukční napěťové špičky vznikající při vypínání motoru. Samotný motor bude odrušen keramickým kondenzátorem 100 nF zapojeným přímo mezi jeho vývody a dvěma keramickými kondenzátory 47 nF mezi jednotlivé vývody a kostru motoru (Faradayova klec); všechny kondenzátory budou dimenzovány na napětí 50 V. Toto odrušení je nezbytné pro omezení jiskření kartáčků a potlačení vysokofrekvenčního elektromagnetického rušení. H-bridge i elektromotor budou v krabičce K umístěny co nejdále od ostatní elektroniky.

Při pohybu dvířek budou postupně, přes 50ms debounce, spínány (respektive rozpínány) dva mikrospínače. Kontakt COM bude připojen k akumulátoru, kontakt NO spolu s pull-down rezistorem k M — rezistor je potřeba kvůli kabelům, které se chovají jako anténa. Koncové spínače představují jednoduché a energeticky nenáročné řešení určování krajních poloh dvířek.

Měření hmotnosti snáškového hnízda bude zprostředkovávat tenzometr se zanedbatelnou nelinearitou a hysterezí. Kabel od tenzometru bude připojen k modulu A/Č převodníku HX711 s nízkým klidovým proudem v řádu jednotek mikroampér, umístěnému v krabičce Kx. Modul zesílí velmi nízké výstupní napětí tenzometru, pohybující se v řádu jednotek milivoltů. Stínění kabelu bude na desce plošných spojů připojeno ke společné zemi za účelem odvodu šumu. Převodník bude připojen k Mx.

Mx bude pro komunikaci s M využívat sběrnici RS485 přes datový kabel typu UTP. První kroucený pár povede napájení pro Mx, oba vodiče zapojené paralelně; stejně tak druhý pár, který přes tranzistorový spínač povede napájení ke zbylé elektronice v krabičce Kx. Třetí pár, opět přes dva paralelní vodiče, propojí společnou zem. Čtvrtý pár přenese data prostřednictvím čipu MAX3485, který slouží jako transceiver sběrnice RS485 — jeden čip bude před M, druhý před Mx.

Tento čip je napájen 3,3 V a vytváří diferenciální signál na linkách A a B, čímž zvyšuje odolnost komunikace proti elektromagnetickému rušení. Protože jde o čip bez vývodů pro nepájivé pole, bude pro prototyp potřeba adaptér SO8 na DIP8 a kolíkové lišty. Paralelně k vývodům VCC a GND bude připojen blokovací keramický kondenzátor 100 nF/50 V.

Vzhledem k použití více snáškových hnízd bude komunikace probíhat mezi hlavní řídicí jednotkou M (master) a několika řídicími jednotkami Mx (slave), propojenými sériově v topologii daisy chain. Vzhledem ke krátké délce vedení v řádu jednotek metrů nebude nutné na začátek ani konec sběrnice připojovat terminační rezistory 120 Ω pro impedanční přizpůsobení vedení — jejich použití by pouze zvyšovalo proudový odběr systému.

Na deskách plošných spojů musí být všechny části systému v jednotlivých krabičkách co nejblíže u sebe, silové části a cesty však musí zůstat oddělené od ostatní elektroniky. Souvislou zemní plochu bude tvořit záporný pól solárního panelu a akumulátoru.

### Mechanika
Hlavní část systému bude umístěna na vnější stěně kurníku, splňující požadavky na umístění solárního panelu popsané v kapitole Napájení. Toto řešení zjednoduší montáž a zároveň z velké části eliminuje vliv amoniaku ze slepičího trusu na elektroniku.

Na stěnu budou pomocí vrutů do dřeva připevněny svislé dřevěné hranolky, k jejichž bočním stranám budou vruty upevněny hliníkové U-profily s vnitřním rozměrem 10 mm. U-profily zajistí přesné vedení dvířek při minimálním tření.

V těchto profilech se budou ve svislém směru pohybovat bílá pěněná PVC dvířka o rozměrech 24 × 35 cm a tloušťce 8 mm (pozor na výrobní tolerance). Při vrtání otvorů do PVC je nutné použít nízké otáčky a nevyvíjet nadměrný přítlak. Pěněné PVC bylo zvoleno pro nízkou hmotnost, odolnost proti vlhkosti a snadnou obrobitelnost.

Na horní části přední strany dvířek bude umístěno závěsné očko M5 z nerezové oceli A2, upevněné podložkou a samojistnou maticí stejného typu; očko bude procházet otvorem odpovídajícího průměru jako jeho metrický závit.

Očkem bude protaženo syntetické lanko (zednická stavební šňůra) o průměru 2 mm, zajištěné dračí smyčkou zakápnutou vteřinovým lepidlem. Syntetická šňůra nekoroduje, má nízkou průtažnost a je odolná vůči vlhkosti.

Lanko povede kolmo vzhůru do krabičky K, kde bude navinuto na špulku vytištěnou z materiálu PETG (šířka 2 cm, vnější průměr 2,5 cm). Otvorem o průměru 3 mm na okraji špulky, dále od motoru, bude lanko provlečeno a upevněno osmičkovým uzlem, rovněž zakápnutým vteřinovým lepidlem; uzel bude zapuštěn 8 mm hluboko do záhlubení o průměru 10 mm, s vývodem lanka na okraji záhlubení směrem k okraji špulky.

Špulka bude opatřena otvorem s D-profilem o průměru 3 mm a pomocí stavěcího šroubu (červíku) M5 z nerezové oceli A4 (délka 6 mm) a mosazné závitové vložky M5 (délka 5,8 mm, průměr 7,1 mm) bude upevněna na hřídeli nízkootáčkového stejnosměrného motoru s kovovou převodovkou (6 V), ze strany, kde vstupuje lanko.

Otvor pro mosaznou závitovou vložku bude mít průměr 8 mm a v posledních 6,5 mm směrem ke hřídeli se zúží na 6,5 mm. Vložka se do otvoru zavede pomocí 16mm šroubu M5; jakmile narazí na zúženou část otvoru, hlava šroubu se nahřeje mikropájkou, plast se v místě zúžení roztaví a umožní zalisování vložky. Po odejmutí pájky plast opět ztuhne a vložku pevně ukotví.

Nízkootáčkový motor spolu s větším vnějším průměrem špulky byly zvoleny pro zvýšení síly motoru a snížení rizika přetrhnutí lanka nebo vykolejení dvířek.

V horní i dolní části hranolku budou ve svislé ose zapuštěny dva pákové koncové mikrospínače. Záhlubení široká 6,5 mm a vysoká 20 mm povedou skrz hranolek a 10mm širokou hliníkovou drážku kvůli vývodu pinů. Mikrospínače budou upevněny vruty o průměru 2 mm; otvory je třeba předvrtat, aby se dřevo při vrtání nedeformovalo.

Vedlejší část systému bude umístěna u spádovaných snáškových hnízd s košíkem, orientovaných příčně ke stěně.

Pod hnízda bude vruty a hmoždinkami připevněna voděodolná OSB 3 deska o rozměrech 37 × 28 cm a tloušťce 12 mm. Druhá deska (35 × 26 cm, stejná tloušťka) bude tvořit základovou desku a bude připevněna k podlaze kurníku; třetí deska stejné tloušťky, o rozměrech 40 × 50 cm, poslouží jako zadní stěna.

Mezi horní a spodní deskou bude u stěny umístěn tenzometr o tloušťce 12,7 mm, orientovaný kratší stranou s vodiči směrem ke stěně. Tenzometr váží s přesností 2 g, což pro vejce o hmotnosti 50–70 g plně postačuje, a je dimenzován na zatížení 20 kg — maximální předpokládané zatížení (dvě desky, hnízdo, dvě slepice a deset vajec) přitom činí přibližně 10 kg.

Optická závora není pro počítání vajec v kurníku vhodná mimo jiné kvůli prachu, peří, podestýlce a zvědavým slepicím, jejichž přítomnost by mohla přerušit laserový paprsek a být chybně vyhodnocena jako propadnuté vejce. Z tohoto důvodu byl pro detekci vajec zvolen tenzometr.

Volný i pevný konec tenzometru bude upevněn dvěma zápustnými imbusovými šrouby M5 z nerezové oceli A2, procházejícími vážicí deskou skrz otvory o průměru 4,5 mm a zapuštěnými do záhlubení o průměru 10 mm a hloubce 3 mm; hlava šroubu bude zalita akrylovým tmelem.

Použité šrouby budou mít délku 25 mm. Mezi deskami a závity tenzometru budou umístěny distanční matice M5 a M6 z nerezové oceli A2 o tloušťce 4 mm.

Ze zbytku první desky (rozměr 38,5 × 29,5 cm) budou vyrobeny lišty, přišroubované ke spodní straně vážicí desky. Lišty budou vysoké 15 mm, protože vzdálenost mezi deskami činí 20,7 mm a celá horní konstrukce musí být podepřena pouze tenzometrem — zároveň musí zůstat zachována vůle alespoň 3 mm.

Do zadní lišty bude vyvrtán otvor, kterým se protáhne již prodloužený kabel od tenzometru; kabel nesmí být mechanicky namáhán tahem. Dále povede vzhůru po stěně do krabičky Kx, odkud bude z boku vyveden sdělovací kabel, vedoucí vysoko po stěně až do krabičky K.

Celá konstrukce musí být od stěny vzdálena natolik, aby se jí nikde nedotýkala a zároveň za ní zůstal prostor pro vedení kabelu; současně je nutné zabránit přístupu slepic a hlodavců do tohoto prostoru.

Uprostřed konstrukce, naproti košíku, bude vytvořen stavitelný doraz pomocí zápustného imbusového šroubu M5 z nerezové oceli A2 (délka 30 mm), procházejícího spodní deskou skrz otvor o průměru 4,5 mm a zapuštěného do záhlubení o průměru 10 mm a hloubce 6 mm. Doraz bude tvořit mosazná kloboučková matice M5 (délka 10 mm, závit 7,5 mm), zvolená kvůli omezení ulpívání nečistot. Optimální vůle dorazu je 0,8 mm, což odpovídá jednomu otočení šroubu M5 nebo přibližně tloušťce běžné platební karty.

## Nákup

**Kabeláž**  
https://www.gme.cz/v/1512358/elektrokabel-cya-1x15-cerveny-h07v-k-izolovany-vodic-lanko (5m)  
https://www.gme.cz/v/1512357/elektrokabel-cya-1x15-cerny-h07v-k-izolovany-vodic-lanko (5m)  
https://www.gme.cz/v/1512382/elektrokabel-cya-1x05-cerveny-h05v-k-izolovany-vodic-lanko (5m)  
https://www.gme.cz/v/1512360/elektrokabel-cya-1x05-cerny-h05v-k-izolovany-vodic-lanko (5m)  

https://www.alza.cz/alzapower-patch-cat5e-utp?dq=6592175 (3 m)  

Willmann Elektro Lipník (2 m)  

**Lišta**  
https://www.hornbach.cz/p/lista-na-kabely-malpro-15x10-mm-2m-bila/6647360/ (4 m)  

**Klipy**  
Willmann Elektro Lipník (10 ks)  

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


**P-MOS**  


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
https://eshop.zenit.cz/desky/pvc-penene-desky/palight-print-8mm/variant/2030-3050/ (2 ks)  

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
https://www.hornbach.cz/conf/osb-deska-kronospan-osb-3-f0-12-x-1250-x-2500-mm-nebrousena/3880511/  

**Červík**  
https://www.hornbach.cz/p/staveci-sroub-s-vnitrnim-sestihranem-a-hrotem-m5x6-mm-nerez-ocel-a4-1-ks/12120029/ (4 ks)  

**Lanko**  
https://www.hornbach.cz/p/stavebni-provazek-2-mm-50-m-cerveny/5040757/  

**Očko**  
https://www.hornbach.cz/p/oko-s-metrickym-zavitem-m5x20-mm-nerez-ocel-a2-1-ks/12120479/ (2 ks)  

**Klobouk**  
https://www.hornbach.cz/p/kloboukova-matice-din-1587-m5-mosaz-1-ks/3830903/ (2 ks)  

**Šrouby Matice Podložky**  
https://www.prumex.cz/sroub-zapustny-inbus-din-7991-m5x30-nerez-a2/ (4 ks)  
https://www.prumex.cz/sroub-zapustny-inbus-din-7991-m5x25-nerez-a2/ (10 ks)  
https://www.prumex.cz/sroub-sestihranny-cely-zavit-din-933-m5x16-nerez-a2-70/ (2 ks)  
https://www.prumex.cz/matice-samojistna-nizka-din-985-m5-nerez-a2/ (4 ks)  
https://www.prumex.cz/matice-din-934-m5-nerez-a2/ (10 ks)  
https://www.prumex.cz/podlozka-plocha-din-125a-m5-nerezova-ocel-a2-5-3x10x1/ (4 ks)  

| OBCHOD | CENA |
|:---|:---:|
| GME | 650 Kč |
| ALZA | 150 Kč |
| WILLMANN | 175 Kč |
| HADEX | 120 Kč |
| HORNBACH | 710 Kč |
| DEXHAL | 590 Kč |
| BATERKY | 270 Kč |
| HOFMAN | 125 Kč |
| LASKAKIT | 700 Kč |
| DRATEK | 300 Kč |
| BOTLAND | 1 880 Kč |
| ZENIT | 120 Kč |
| LIHNEME | 1 150 Kč |
| SOS | 450 Kč |
| PRUMEX | 200 Kč |
| REZERVA | 500 Kč |
| **CELKEM** | **8 090 Kč** |

*Poznámka: Cena je orientační a je do ní započtena i doprava. Položky z Hornbachu budou zakoupeny osobně.*
