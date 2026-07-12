# Bakalářská práce
Systém pro automatizaci kurníku s detekcí snesených vajec

## Zadání
1. Prostudujte možnosti automatizace uzavírání kurníku, možné metody detekce snesených vajec a dostupné možnosti komunikace a připojení zařízení do IoT sítě (např. NB-IoT, WiFi, LoRaWAN).
2. Navrhněte systém pro automatické otevírání a uzavírání dvířek kurníku s možností rozšíření o jednotky se senzory ve snáškových hnízdech (min. 5 hnízd). Prostudujte možnosti napájení z akumulátoru nebo
   fotovoltaického panelu. Proveďte analýzu možnosti přenosu dat do cloudu a jejich zobrazení uživateli včetně historie snášek a možnosti vzdáleného ovládání, například skrze aplikaci.
3. Na základě analýzy navrhněte způsob vzdálené uživatelské interakce pro zobrazení stavu dvířek, manuální ovládání a počítání snesených vajec.
4. Navrhněte schéma zařízení, které bude obsahovat mikrokontrolér, komunikační modul, senzory, řízení dvířek, napájecí/nabíjecí obvody. Dbejte na nízký příkon celého zařízení. Proveďte návrh desky plošných
   spojů.
5. Zkonstruujte hardwarovou realizaci navrženého systému. Osaďte a zprovozněte řídicí elektroniku. Vytvořte ovládací firmware. Realizujte přenos dat ze zařízení k uživateli včetně ovládacího rozhraní a
   možnosti zobrazení historie.
6. Ověřte funkčnost systému experimentálním měřením a vyhodnoťte spolehlivost detekce a ovládání.
7. Zveřejněte veškeré výrobní podklady na vhodné platformě (např. GitHub).

## Schéma
<img src="https://github.com/Hosty-04/Bachelors_Theses/blob/main/schematics/block_schematic.png" alt="block_schematic">

## Popis

### Krabičky
Systém bude obsahovat jednu krabičku pro akumulátor o rozměrech 250 × 175 × 150 mm a tloušťce stěny 2,4 mm, dále druhou krabičku (K) o rozměrech × a tloušťce stěny 1,6 mm, která bude určena pro mechaniku a elektroniku dvířek a hlavní mikrokontrolér (M). Pro každé snáškové hnízdo bude dále určena samostatná krabička (Kx) o rozměrech × a tloušťce stěny 1,6 mm. Každá tato krabička bude obsahovat potřebnou elektroniku a vedlejší mikrokontrolér (Mx).

Všechny krabičky budou vytištěny na 3D tiskárně z materiálu PETG. Tento materiál byl zvolen pro vyšší houževnatost a teplotní odolnost oproti materiálu PLA. Jejich zadní stěna bude ve svislém směru prodloužena a bude mít tloušťku 3,5 mm. V každém rohu této stěny budou vytvořeny otvory pro vruty umožňující upevnění krabiček ke stěně. Zvětšená tloušťka zvyšuje tuhost při montáži. Krabičky budou mít ve stěnách naproti sobě 2 větrací otvory o průměru 1,5 cm.

Dále budou na všech krabičkách vytvořeny mírně zaoblené nízké drážky ve tvaru písmene U, směřující od horní hrany směrem dolů. Tyto drážky budou sloužit k protažení kabelů. Pro speciální solární kabel budou mít šířku 6 mm a pro datový kabel 5,5 mm. Kruhový otvor umístěný na spodní straně krabičky Kx bude mít průměr taktéž 5,5 mm, určený pro průchod stíněného kabelu. V krabičce K bude zespodu otvor pro lanko o průměru 3 mm.

Víko bude vytištěno samostatně s přesahem 2 mm a bude obsahovat dutou trubičku uprostřed levé hrany. Na horní hraně levé stěny každé krabičky budou po stranách na malé plošince umístěny dvě trubičky s průměrem otvoru 2,1 mm. Po vytištění bude všemi třemi trubičkami protažen filament o průměru 1,75 mm, jehož konce budou krátce nahřáty zapalovačem a roztemovány tak, aby vytvořily hlavičky. Tím vznikne jednoduchý pant pro uchycení víka. Toto řešení minimalizuje počet součástek a umožňuje výrobu celé krabičky bez dalších kovových dílů.

### Kabeláž
Pro připojení solárního panelu a akumulátoru bude využito speciálního solárního kabelu typu licna o průřezu 4 mm² a hřebíkových příchytek o průměru 6 mm. Tyto kabely budou v K přes prodlužovací WAGO svorky přetransformovány na měděnou ohebnou licnu o průřezu 1,5 mm². Ta bude pomocí 4 pinové WAGO svorkovnice o rozteči 5,08 mm připojena k desce plošných spojů a odtamtud povede do akumulátoru.

Mikrospínače budou připojeny k desce plošných spojů přes měděnou ohebnou licnu o průřezu 0,5 mm² pomocí další 4 pinové WAGO svorkovnice s roztečí 5,08 mm. Spoje kontaktů mikrospínačů s touto licnou budou izolovány pomocí smršťovacích bužírek s poměrem 2:1 a vnitřním průměrem před / po zahřátí 2 mm / 0,75 mm, které budou přečnívat asi centimetr přes izolaci.

U prototypu bude využíváno rovnou měděné ohebné licny o průřezu 1,5 mm² a drátků do nepájivého pole s průřezem 0,5 mm². Pro kladný pól bude použit červený vodič a pro záporný pól černý vodič.

Pro datovou komunikaci byl zvolen kabel UTP CAT5e typu licna, který bude ke stěně upevněn pomocí příchytek o průměru 5 mm a přes konektory RJ45 vložen do krabiček.

Prodloužení vodičů tenzometru zajistí přes prodlužovací WAGO svorky stíněný kabel LiYCY 4 × 0,25 mm² typu licna. Jelikož jsou vodiče od tenzometru o průřezu 0,14 mm² pro WAGO svorky příliš tenké (povolený průřez je 0,2 - 4 mm²), tak se u odizolovaného konce přehnou napůl na průřez 0,28 mm². K desce plošných spojů bude potom tento kabel spolu se stíněním připojen pomocí 5 pinové WAGO svorkovnice o rozteči 2,5 mm.

Oplet, ve formě pocínovaných měděných drátků, bude izolován pomocí smršťovací bužírky s poměrem 2:1 a vnitřním průměrem před / po zahřátí 4 mm / 1,75 mm. Je potřeba rozdělit odmotaný oplet, spletený do drátku a zbylé 4 vodiče a bužírku navléknout až ke kořenu. Přes celý kabel bude potom přetáhnuta bužírka 2:1 7,5 mm / 3,5 mm, která bude přečnívat asi centimetr přes hlavní izolaci.

### Napájení
Solární panel o parametrech 6 V / 10 Wp bude svisle připevněný na stěnu pod malou stříšku a orientovaný směrem na jih, případně na východ nebo západ (v mém případě bude panel směřovat na jihozápad). Tím bude zajištěno, že panel bude co nejlépe využívat dostupnou sluneční energii. Vertikální montáž omezuje usazování sněhu a nečistot.

Společně s bezúdržbovým olověným AGM akumulátorem o parametrech 6 V / 7 Ah, umístěným venku ve stínu asi 25 cm pod stříškou (ideálně na severní straně), budou zajišťovat napájení celého systému. Akumulátor LiFePO4 je sice v mnoha ohledech lepší, ale nesmí se nabíjet v zimě pod 0 °C a potřebuje složitější a energeticky náročnější solární regulátor napájení MPPT. Silová část bude pracovat s napětím 6 V, zatímco veškerá elektronika bude napájena napětím 3,3 V.

Snížení napětí z 6 V na 3,3 V bude realizováno pomocí nízkopříkonového LDO regulátoru MCP1700. Nízký klidový proud významně prodlužuje provoz z akumulátoru. Tento stabilizátor bude zapojený k baterii a na jeho vstupu bude připojen blokovací keramický kondenzátor o parametrech 1 µF / 50 V. Stejný kondenzátor bude připojen i na výstupu, ale v tomto případě z důvodu vyhlazení napětí. 

Použití spínaného buck měniče není vhodné z důvodu horší dostupnosti nízkopříkonových variant a velmi malého odběru systému po většinu dne. Jeho vyšší účinnost by se projevila pouze po dobu několika minut denně, zatímco po zbytek dne by kvůli vlastní spotřebě dosahoval nižší celkové účinnosti než jednoduchý lineární stabilizátor.

#### Denní přehled

### Klidová spotřeba (24 hod)

| Komponenta | Proud (typ) | Proud (max) | Spotřeba (typ) | Spotřeba (max) |
|---|---|---|---|---|
| LDO (MCP1700) | 1,6 µA | 4 µA | 38,4 µAh | 96 µAh |
| M (Stop2 s RTC) | 1,0 µA | 2,8 µA | 24 µAh | 67,2 µAh |
| 5 × Mx (Stop bez RTC) | 1,9 µA | 1,9 µA | 45,6 µAh | 45,6 µAh |
| **Celkem** | **4,5 µA** | **8,7 µA** | **108 µAh** | **209 µAh** |

*Poznámka: Ostatní části systému jsou odpojovány přes tranzistorové spínače.*

### Kontrola panelu a baterie (43 s)

| Komponenta | Proud (typ) | Proud (max) | Spotřeba (typ) | Spotřeba (max) |
|---|---|---|---|---|
| M (LPRun @ 1 MHz) | 220 µA | 220 µA | 2,64 µAh | 2,64 µAh |
| INA219 aktivní | 0,7 mA | 1 mA | 8,4 µAh | 12 µAh |
| **Celkem** | **0,92 mA** | **1,22 mA** | **11 µAh** | **14,6 µAh** |

*Poznámka: Kontrola probíhá každých 10 minut po dobu 300 ms.*

### Pohyb dvířek (40 / 60 s)

| Komponenta | Proud (typ) | Proud (max) | Spotřeba (typ) | Spotřeba (max) |
|---|---|---|---|---|
| Motor | 150 mA | 200 mA | 1,67 mAh / den | 3,33 mAh / den |
| DRV8838 | 340 µA | 600 µA | 3,78 µAh / den | 6,67 µAh / den |
| M (LPRun @ 1 MHz) | 220 µA | 220 µA | 2,44 µAh / den | 2,44 µAh / den |
| INA219 | 0,7 mA | 1 mA | 7,78 µAh / den | 11,1 µAh / den |
| **Celkem** | **151 mA** | **202 mA** | **1,68 mAh / den** | **3,35 mAh / den** |

*Poznámka: Možnost zaseknutí dvířek je brána v potaz.*

### Kontrola vajec (8 min, 5 hnízd)

| Komponenta | Proud (typ) | Proud (max) | Spotřeba (typ) | Spotřeba (max) |
|---|---|---|---|---|
| M (LPRun @ 1 MHz) | 220 µA | 220 µA | 29,3 µAh / den | 29,3 µAh / den |
| MAX3485 (M) | 1,1 mA | 2,2 mA | 0,147 mAh / den | 0,293 mAh / den |
| MAX3485 (Mx) | 1,1 mA | 2,2 mA | 0,147 mAh / den | 0,293 mAh / den |
| Mx (LPRun @ 131 kHz) | 32 µA | 37 µA | 4,27 µAh / den | 4,93 µAh / den |
| HX711 a tenzometr | 4,4 mA | 4,4 mA | 0,587 mAh / den | 0,587 mAh / den |
| **Celkem** | **6,85 mA** | **9,06 mA** | **0,914 mAh / den** | **1,21 mAh / den** |

*Poznámka: STM32 NUCLEO-L031K6, MAX3485, HX711 a tenzometr. Tyto obvody jsou v každé Kx, ale zásluhou chytrého používání tranzistorových spínačů a režimů řadiče se proud tváří, jako kdyby bylo v celém kurníku pouze jedno hnízdo. Trik je v tom že zapnuté je pouze to, co zrovna pracuje. Výsledkem je 5 × nižší spotřeba.*

### LoRa TX (3 s)

| Komponenta | Proud (typ) | Proud (max) | Spotřeba (typ) | Spotřeba (max) |
|---|---|---|---|---|
| Rádio TX (SMPS) | 21 mA | 21 mA | 17,5 µAh / den | 17,5 µAh / den |
| CPU (LPSleep) | 44 µA | 69 µA | 0,0367 µAh / den | 0,0575 µAh / den |
| **Celkem** | **21 mA** | **21,1 mA** | **17,5 µAh / den** | **17,6 µAh / den** |

### LoRa RX (10 s)

| Komponenta | Proud (typ) | Proud (max) | Spotřeba (typ) | Spotřeba (max) |
|---|---|---|---|---|
| Rádio RX (SMPS) | 4,8 mA | 4,8 mA | 13,3 µAh / den | 13,3 µAh / den |
| CPU (LPSleep) | 44 µA | 69 µA | 0,122 µAh / den | 0,192 µAh / den |
| **Celkem** | **4,84 mA** | **4,87 mA** | **13,4 µAh / den** | **13,5 µAh / den** |

*Poznámka: Odhad délky okna je 200 ms*

### Procentuální rozložení a celková denní spotřeba

| Blok | Spotřeba (typ) | Podíl | Spotřeba (max) | Podíl |
|---|---|---|---|---|
| Pohyb dvířek | 1,68 mAh / den | 61,2 % | 3,35 mAh / den | 69,6 % |
| Kontrola vajec | 0,914 mAh / den | 33,3 % | 1,21 mAh / den | 25,1 % |
| Klidový režim | 0,108 mAh / den | 3,9 % | 0,209 mAh / den | 4,3 % |
| LoRa TX | 17,5 µAh / den | 0,6 % | 17,6 µAh / den | 0,4 % |
| LoRa RX | 13,4 µAh / den | 0,5 % | 13,5 µAh / den | 0,3 % |
| Kontrola panelu / baterie | 11 µAh / den | 0,4 % | 14,6 µAh / den | 0,3 % |
| **Celkem** | **2,75 mAh / den** | 100 % | **4,81 mAh / den** | 100 % |

### Tabulka 7 – Výroba energie panelu (1 panel, dle světových stran)

| Orientace | Léto (Wh/den) | Zima (Wh/den) |
|---|---|---|
| Jih | 30–45 | 7,5–11,25 |
| Východ | 22–32 | 5,5–8,0 |
| Západ | 22–32 | 5,5–8,0 |
| Sever | 8–14 | 2,0–3,5 |
| JZ (odhad interpolací Jih/Západ) | ~26–38 | ~6,5–9,5 |

### Tabulka 8 – Kapacita baterie (25 % derating v zimě)

| | Léto | Zima |
|---|---|---|
| AGM 6V/7Ah | 7 Ah / 42 Wh | 5,25 Ah / 31,5 Wh |

### Tabulka 9 – Energetická bilance (spotřeba: 2,605–3,657 mAh/den = 0,0156–0,0219 Wh/den)

| Orientace | Bilance léto (mAh/den) | Bilance zima (mAh/den) |
|---|---|---|
| Jih | +4 997–7 497 | +1 246–1 871 |
| Východ | +3 663–5 330 | +913–1 330 |
| Západ | +3 663–5 330 | +913–1 330 |
| Sever | +1 330–2 330 | +330–580 |
| JZ | +4 330–6 330 | +1 080–1 580 |

### Denní harmonogram power módů – M (LoRa-E5, 1 MHz)

| Čas/událost | Mód | Proud (pokoj/max) |
|---|---|---|
| Mezi probuzeními | Stop2 mode + RTC | 1,0 / 2,8 µA |
| Kontrola panelu/baterie (144×) | LPRun @ 1 MHz | 190/245 µA + INA219 aktivní |
| Koordinace vajec (24×, 20 s/hod) | LPRun @ 1 MHz | 190/245 µA + MAX3485_M aktivní |
| LoRa TX (SMPS) | LPSleep (CPU) + rádio TX | 35,5/60,0 µA + 21 mA |
| LoRa RX (SMPS) | LPSleep (CPU) + rádio RX | 35,5/60,0 µA + 4,8 mA |
| Pohyb dvířek | LPRun @ 1 MHz | 190/245 µA + INA219 + DRV8838 logika |

### Denní harmonogram power módů – Mx (STM32L031, 131 kHz)

| Čas/událost | Mód | Proud (pokoj/max) |
|---|---|---|
| Mezi vlastními cykly (většina hodiny) | Stop mode bez RTC (RAM zachována) | 0,38 / 0,99 µA |
| Vlastní aktivní okno (4 s/hodinu) | LPRun @ 131 kHz | 32/36 µA + tenzometr+HX711 + MAX3485_Mx aktivní |

### Poznámky k ověření hodnot

Ověřeno z datasheetů: proud M ve Stop2+RTC (STM32WLE5, Table 43), proud M v LPSleep (STM32WLE5, Table 42), proud Mx v LPRun @131kHz (STM32L031, Table 29), proud Mx ve Stop bez RTC (STM32L031, Table 31), klidový proud LDO MCP1700, aktivní/shutdown proud INA219, klidový proud DRV8838 nSLEEP (0,34/0,6 µA – bezpředmětný díky spínači 2).

Neověřeno, zůstává jako zadaná/odhadnutá hodnota: proud motoru DRV8838 VM (150/200 mA), proud M v LPRun @1MHz (190/245 µA), proud MAX3485 (1,1/2,2 mA), proud HX711 samotného (1,5 mA) a tenzometr+HX711 dohromady (3,3 mA), airtime LoRa (SF9/BW125/CR4:5, vypočteno teoreticky, neměřeno), doba aktivního okna 4 s/hnízdo (odhad rozpadu, neměřeno).

*Poznámka: Zimní bilance počítá s poklesem efektivity solárního panelu o 75 % a kapacity akumulátoru o 25 % oproti letním hodnotám*

### Řízení
Hlavní řídicí jednotkou systému bude mikrořadič STM32WLE5JC LoRa-E5 mini. Ten bude disponovat integrovaným LoRa modulem, komunikujícím přes LoRaWAN stack. Kvůli zamezení rušení při komunikaci bude řadič umístěn na opačné straně než motor. LoRa modul umožní na rozdíl od Wi-Fi komunikaci na velké vzdálenosti při nízké spotřebě energie. NB-IoT má taktéž vyšší spotřebu. U každého snáškového hnízda bude umístěn další mikrořadič STM32 NUCLEO-L031K6. Ten má integrovaný programátor, který bude využit i pro hlavní řadič.

Firmware bude vyvíjen v prostředí STM32CubeIDE. Součástí firmwaru hlavní řídicí jednotky budou astronomické hodiny, které každý den, pomocí RTC hodin, spočítají čas východu a západu slunce. Podle těchto údajů se potom budou automaticky otevírat a zavírat dvířka od kurníku. Přes RTC hodiny bude řadič schopen zjistit také roční období. Použití časovače nebylo zvoleno z důvodu proměnlivé délky dne. Světelný senzor by mohl způsobovat chybné sepnutí při zatažené obloze (déšť nebo bouřka) nebo ve večerních hodinách vlivem pouličního osvětlení či světlometů automobilů. Konfiguraci zajistí STM32CubeMX.

Hlavní řídicí jednotka se bude společně s nezbytnými částmi systému probouzet každých 10 minut za účelem kontroly stavu solárního panelu a akumulátoru (stovky milisekund). Dále se bude společně s ostatními řídicími jednotkami a dalšími potřebnými částmi systému probouzet 24× denně, tedy každou hodinu, pro sekvenční kontrolu stavu vajec s postupným buzením každého hnízda (24 × 5 hnízd × 4 s na jedno hnízdo = cca 8 minut). Po sběru dat ze všech hnízd se všechno najednou odešle. Nakonec se bude probouzet ráno a večer, opět pouze s nezbytnými částmi systému, za účelem otevření a zavření dvířek (cca 2 × 20 s). Před zahájením pohybu dvířek budou piny koncových spínačů nastaveny jako digitální vstupy s interním pull-up rezistorem. Po dosažení koncové polohy se odešle informace o poloze dvířek a piny budou přepnuty do analogového režimu bez pull rezistorů, čímž bude eliminován jejich klidový odběr.

LoRa anténa bude moci vysílat až po vypnutí všech systémů, kvůli jejímu vyššímu odběru proudu a ochraně před rušením. Po každém vysílání bude mít potom možnost příjmout data, což umožní uživatelské ovládání. Upřednostňované parametry komunikace jsou: vysílací výkon 12 dBm, SF9, BW 125 kHz, CR 4 / 5, 2 příjmová okna RX1 + RX2 a délka příjmových oken automaticky podle driftu krystalu.

V domě bude umístěna LoRaWAN gateway, která bude přes WiFi plnit funkci internetové brány. Veškerá data přijatá přes gateway budou následně odeslána do cloudu (TTN) a potom přes MQTT na backend (NodeJS). Ten data uloží do databáze (InfluxDB) a zobrazí je na frontendu. Při odesílání dat z gateway je proces obrácený.

Data budou z kurníku odesílána pomocí 3+ bajtů. První bajt bude obsahovat 1 bit pro indikaci zapnutí / vypnutí systému, další pro oznámení kritického stavu (nízké napětí na akumulátoru) a 5 bitů ponese informaci o samotném napětí na akumulátoru (5,75 V - 8 V, krok cca 0,1 V) Druhý bajt bude obsahovat 2 bity pro indikaci stavu dvířek (ON / OFF / zaseknuto) a 6 bitů ponese informaci o napětí solárního panelu (0 - 6 V, krok cca 0,1 V). Dalších x bajtů bude po 4 bitech alokováno pro přenos počtu vajec v x snáškových hnízdech (0 - 10 vajec na hnízdo). Kurník bude data přenášet každou hodinu po kontrole stavu hnízd a k tomu 2× denně, při změně stavu dvířek, odešle pouze první 2 bajty. Manuální ovládání bude využívat pouze 1 bajtu, jeden bit bude využit pro vypnutí / zapnutí systému, další bit pro vypnutí / zapnutí kritického režimu a poslední bit pro ovládání dvířek (ON / OFF).

Pro přenos dat mezi hlavní řídicí jednotkou a ostatními řídicími jednotkami bude využit protokol LPUART, který nevyužívá hodinový signál a vyznačuje se nízkou spotřebou energie. Přenosová rychlost bude 9600 Bd, kvůli minimalizaci odrazů. Na aplikační vrstvě bude použit komunikační protokol Modbus RTU a knihovna ModbusRTU-Slave. Protokol Modbus RTU vytvoří datový rámec obsahující adresu jednotky slave, přenášená data a kontrolní součet CRC pro detekci chyb při přenosu. LPUART, hardware v řadiči, následně převede jednotlivé bajty na sériový datový tok, doplní start a stop bity a zajistí jejich přenos po sběrnici. Na straně přijímače proběhne opačný proces.

**Stavový automat pro algoritmus detekce snesených vajec**  
Probuzení mikrořadiče a převodníku HX711.  
Čekání 500 ms na ustálení měření.  
Odebrání 32 vzorků rychlostí 10 SPS.  
Výpočet mediánu.  
Výběr 16 vzorků s nejmenší odchylkou od mediánu.  
Výpočet aritmetického průměru, aktuální hmotnosti (odečet referenční nulové hodnoty) a směrodatné odchylky.  
Pokud hmotnost překročí 1 kg (v hnízdu je slepice), měření se zahodí.  
Pokud odchylka překročí stanovený práh (pohyb slepice, vibrace), měření se zahodí.  
Je-li měření stabilní, aktuální hmotnost se porovná s uloženou hodnotou.  
Odpovídá-li rozdíl hmotnosti přibližné hmotnosti jednoho (60 g) nebo více vajec, je změna aritmeticky přičtena k uložené hodnotě.  
Při hmotnosti < 25 g je provedena kontrola driftu. Pokud jsou zaznamenána tři po sobě jdoucí stabilní měření, je aktualizována referenční nulová hodnota.  
Přechod mikrořadiče i převodníku HX711 do režimu spánku.  

Většinu dne bude hlavní řídicí jednotka v režimu Stop2 s RTC. Tento režim se vyznačuje velmi nízkou spotřebou a narozdíl od režimu StandBy s RTC má mimo jiné možnost udržet logické úrovně a nastavení pinů. Řadič je automaticky taktovaný externím krystalem LSE, umístěným na LoRa-E5 mini, na 32 kHz. Když ale RTC hodiny zavelí že je čas na práci, tak se řadič přepne do režimu LPRun (Low Power Run). V tomto režimu je taktovaný interním krystalem LSI na 1 MHz. V průběhu LoRa přenosu (Radio TX / RX) se CPU přepne do LPSleep režimu (LSI, 1MHz). Kvůli nízké taktovací frekvenci je potřeba v souboru lorawan_conf.h zvýšit RADIO_WAKEUP_TIME z 2 na 5 ms. Rádio poběží automaticky na 32 MHz.

U ostatních řídicích jednotek to bude po většinu dne velmi podobné. Budou se nacházet v úsporném režimu Stop bez RTC, ze stejných důvodů jako hlavní řídicí jednotka a protože je potřeba, aby si uchovali paměť RAM. Řadiče budou postupně jeden za druhým probouzeny přes hlavní řadič, přesněji přes LPUART. Tudíž nepotřebují RTC hodiny. Po probuzení se daný řadič přepne do režimu LPRun (LSI, 131 kHz) a hned po vykonání úkolu se přepne zpět do režimu Stop bez RTC.

Před odpojením napájení VCC různých částí systému je potřeba piny (SCK, DT, PH, EN, DI, DE, RE, RO) přepsat na logickou nulu, pro zamezení napájení přes piny tomu neurčené (leakage current). Dále budou vypnuty periferie (I²C, UART, ADC) a jejich hodinový signál, který plýtvá energii, i když nic neposílají. Po odpojení VCC je potom třeba nastavit všechny piny, včetně těch pro teď už vypnuté periferie, do analogového režimu bez pull rezistoru.

Kvůli spotřebě je potřeba odpájet červenou Power LED diodu a softwarově odpojit všechny zelené User LED diody. Programátor ST-Link musí být při provozu hardwarově odpojen. Pájecí jumpery SB9, SB14, SB2 a SB3 je potřeba odpájet. U LoRa-E5 mini je ještě potřeba přepnout piny PA2 a PA3, zodpovědné za ladění, do analogového režimu bez pull rezistoru. Pro programování lze poté programátor připojovat k deskám přes klasické Dupont kabely a u LoRa-E5 mini vrátit piny PA2 a PA3 do původního stavu. Pozor na plovoucí piny! Nepoužívané piny musejí být uvedeny do analogového režimu bez pull rezistoru. Kvůli správné funkci úsporného režimu je u hlavní řídicí jednotky před uspáním nutné nastavit externí RF switch na logickou nulu a použít LP rádio (vstup i software). U ostatních řídicích jednotek je zase nutné v registrech napájení (PWR) aktivovat ultra-low-power režim (ULP bit) a vypnout fast wakeup.

### Elektronika
Prototyp bude sestaven z modulů umístěných na nepájivém poli pomocí kolíkových lišt a šroubovacích svorek, které budou obsahovat kovový plíšek pro ochranu licny. Finální verze bude obsahovat jednu hlavní desku plošných spojů a až 5 vedlejších desek pro jednotlivá snášková hnízda (v mém případě 2). Na všech deskách budou moduly nahrazeny čipy a nezbytnými externími SMD součástkami.

Po připojení solárního panelu do krabičky K bude využito elektrolytického kondenzátoru 47 µF / 25 V a keramického kondenzátoru 100 nF / 50 V, zapojeného co nejblíže spínacím prvkům. Elektrolytický kondenzátor bude fungovat jako zásobárna energie a keramický bude filtrovat rychlé špičky.

K solárnímu panelu bude připojen vysokoimpedanční napěťový dělič tvořený rezistory o hodnotách 1 MΩ a 470 kΩ. Paralelně k rezistoru R2 (470 kΩ) bude připojen blokovací keramický kondenzátor o kapacitě 100 nF a jmenovitém napětí 16 V. Dělič bude sloužit ke snímání napětí solárního panelu, přičemž naměřené hodnoty budou odesílány do M přes ADC pin v režimu analog input. Díky vysoké impedanci bude proudový odběr děliče zanedbatelný.

K solárnímu panelu budou sériově připojeny dva P-MOS tranzistory s nízkým RDS(on), spojené back-to-back (drainy proti sobě), které budou tvořit hlavní výkonový spínač celého systému. Jejich řízení bude realizováno jedním budicím logic-level N-MOS tranzistorem, protože napětí 3,3 V z M není dostatečné pro jejich přímé sepnutí. N-MOS tranzistor bude řízen M. Na jeho gate bude sériově připojen rezistor o hodnotě 220 Ω pro omezení proudových špiček na výstupu M. Mezi gate a společnou zem bude paralelně připojen pulldown rezistor o hodnotě 10 kΩ, který zabrání vzniku nedefinovaného logického stavu. Drain bude připojen k akumulátoru, pull-up rezistoru 10 kΩ a na gate obou P-MOS tranzistorů. P-MOS tranzistory budou tedy spínané 6 V. Source bude připojen ke společné zemi. Tahle část obvodu bude sloužit jako jednoduchý a velmi úsporný solární regulátor napájení.

Důvodem použití dvou P-MOS tranzistorů namísto jednoho je přítomnost parazitní diody, která je nedílnou součástí jejich vnitřní struktury. Pokud by byl použit pouze jeden tranzistor, protékal by večer i při jeho rozepnutí proud z akumulátoru zpět do solárního panelu prostřednictvím této diody, což by způsobovalo samovolné vybíjení akumulátoru. Pokud nastane výpadek napájení, tak se P-MOS tranzistory automaticky zavřou.

Modul proudového a napěťového senzoru INA219 bude spolu s vyrovnávacím keramickým kondenzátorem o parametrech 100 nF / 50 V, co nejblíže paralelně zapojeným mezi piny Vin+ a GND, zapojen v krabičce K mezi akumulátor a vstup VM pro napájení motoru u H-bridge. Jednou z jeho funkcí bude s 12-bit rozlišením a průměrováním ze 128 vzorků snímat napětí akumulátoru.

Na základě údajů z modulu INA219 a děliče napětí bude M prostřednictvím sběrnice I²C, resp. portu GPIO vyhodnocovat stav akumulátoru a solárního panelu. Ochranu proti přepětí M zajistí přes regulaci s hysterezí. Pokud bude napětí na akumulátoru limitní, tedy v létě 7,2 V, na jaře / podzim 7,3 V a v zimě 7,5 V, M odpojí solární panel. Jestli napětí akumulátoru klesne o 0,25 V, tak M solární panel znovu připojí. Při kritickém vybití akumulátoru, kdy se jeho napětí dosáhne hodnoty 5,75 V, M vypne všechny části systému, Mx uvede do StandBy režimu bez RTC a sám přejde do režimu Shutdown. Pro probuzení je potřeba mikrořadiče resetovat. V noci, kdy panel nebude dodávat žádné napětí musí M zamezit přepólování, aneb ke vzniku zpětného proudu směrem do panelu. To udělá odpojením solárního panelu.

Senzor INA219 bude současně využíván jako proudový snímač s 12-bit rozlišením a průměrováním ze 4 vzorků. M bude monitorovat proud odebíraný z akumulátoru. Náhlé zvýšení proudu během pohybu dvířek nad 225 mA u 3 vzorků po sobě bude indikovat jejich zablokování nebo náraz do překážky, například slepice. V takovém případě se M zastaví na 250 ms, pokusí se obrátit směr otáčení motoru, dvířka vrátí zpět do původní polohy, uspí se a po 10 minutách to zkusí znovu. Pokud ani zpětný chod problém nevyřeší, systém se vypne. Bude nutné ignorovat krátkodobé proudové špičky vznikající při rozběhu motoru, trvající asi 150 ms. Takto bude zabráněno poškození systému.

Při pohybu dvířek budou postupně spínány (debounce 50 ms), respektive rozpínány dva mikrospínače. Kontakt COM bude připojen k akumulátoru a kontakt NO spolu se silným 1 kΩ externím pulldown rezistorem k M. Koncové spínače představují jednoduché a energeticky nenáročné řešení určování krajních poloh.

M bude dále napřímo (bez PWM modulace) řídit H-bridge Pololu DRV8838, který bude vybaven elektrolytickým kondenzátorem 47 µF / 25 V zapojeným co nejblíže mezi piny VM a GND, jenž bude potlačovat indukční napěťové špičky vznikající při vypínání motoru. Ten bude odrušen keramickým kondenzátorem 100 nF zapojeným přímo mezi jeho vývody a dvěma keramickými kondenzátory 47 nF zapojenými mezi jednotlivé vývody a kostru motoru (Faradayova klec). Všechny kondenzátory budou dimenzovány na napětí 50 V. Toto odrušení je nezbytné pro omezení jiskření kartáčků a potlačení vysokofrekvenčního elektromagnetického rušení. H-bridge i elektromotor budou umístěny v krabičce K.

Měření váhy snáškového hnízda bude zprostředkovávat tenzometr se zanedbatelnou nelinearitou a hysterezí. Stíněný kabel od tenzometru bude připojen k modulu AČ převodníku HX711 s nízkým klidovým proudem v řádu jednotek mikroampér, umístěnému v krabičce Kx. Modul bude použit z důvodu zesílení velmi nízkého výstupního napětí tenzometru, které se pohybuje v řádu jednotek milivoltů. Stínění kabelu bude připojeno ke společné zemi, na desce plošných spojů, za účelem odvodu šumu. Převodník bude připojen k Mx.

Mx bude pro komunikaci s M prostřednictvím datového kabelu typu UTP využívat sběrnici RS485. První kroucený pár bude sloužit k přenosu napájení pro Mx, přičemž oba vodiče budou zapojeny paralelně. Stejně tak druhý pár, který bude sloužit k přenosu napájení do zbylé elektroniky v krabičce Kx, přes tranzistorový spínač. Třetí pár bude stejným způsobem použit pro propojení společné země. Čtvrtý pár bude přenášet data prostřednictvím čipu MAX3485, který bude sloužit jako transceiver sběrnice RS485. Jeden čip bude před M a druhý před Mx.

Tento čip vytváří diferenciální signál na linkách A B, čímž zvyšuje odolnost komunikace proti elektromagnetickému rušení. Jelikož je to čip, tak je pro prototyp potřeba použít adaptér SO8 na DIP8 a kolíkové lišty.

Vzhledem k použití několika snáškových hnízd bude komunikace probíhat mezi jednou řídicí jednotkou M (master) a několika jednotkami Mx (slave), které budou propojeny sériově v topologii Daisy Chain. Vzhledem ke krátké délce vedení v řádu jednotek metrů nebude nutné na začátek ani konec sběrnice připojovat zakončovací rezistory o hodnotě 120 Ω pro impedanční přizpůsobení vedení. Jejich použití by pouze zvyšovalo proudový odběr systému.

Pro dosažení nízké spotřeby bude celý systém řízen několika tranzistorovými spínači. Tyto spínače budou konstruovány stejně jako hlavní spínač, ale jen s jedním PMOS tranzistorem. Přes první z těchto spínačů bude M řídit napájení k INA219. Přes druhý napájení k DRV8838 a třetí bude dodávat napětí k hlavnímu MAX3485 a zároveň do všech krabiček Kx. V každé krabičce Kx budou 2 spínače. Přes první z nich bude Mx řídit napájení ke svému MAX3485 a HX711. Ten bude ve výchozím stavu sepnutý. Druhý spínač bude Mx umožňovat poslat napětí do další krabičky Kx. Ten bude ve výchozím stavu rozepnutý. Použití těchto spínačů sníží spotřebu celého systému na zanedbatelnou po většinu dne.

Po zapnutí napájení k daným částem systému bude potřeba chvíli počkat něž naběhnou. U INA219 to je 150 µs před odesláním konfigurace a 1,5 ms než je připraven s prvními naměřenými hodnotami. Pro DRV8838 je to 1,5 ms než se nabije nábojová pumpa a MAX3485 potřebuje pouhých 150 µs.

Všechny části systémů v jednotlivých krabičkách musí být co nejblíže u sebe. Desky plošných spojů budou obsahovat souvislou zemní plochu připojenou k zápornému pólu solárního panelu a akumulátoru.

### Mechanika
Hlavní část systému bude umístěna na vnější stěně kurníku, která bude splňovat požadavky na umístění solárního panelu uvedené v kapitole Napájení. Toto řešení zjednoduší montáž a zároveň z velké části eliminuje působení amoniaku ze slepičího trusu na elektroniku.

Na stěnu budou pomocí vrutů do dřeva připevněny svislé dřevěné hranolky. K jejich bočním stranám budou následně pomocí vrutů upevněny hliníkové U-profily s vnitřním rozměrem 10 mm. U-profily zajišťují přesné vedení dvířek při minimálním tření.

V těchto profilech se budou ve svislém směru pohybovat bílá pěněná PVC dvířka o rozměrech 24 × 35 cm a tloušťce 8 mm (pozor na tolerance). Při vrtání otvorů do PVC bude nutné použít nízké otáčky a nevyvíjet nadměrný přítlak. Pěněné PVC bylo zvoleno díky nízké hmotnosti, odolnosti proti vlhkosti a snadné obrobitelnosti.

Na horní části přední strany dvířek bude umístěno závěsné očko M5 z nerezové oceli A2, upevněné pomocí zapuštěné podložky a matice stejného typu. Očko bude procházet otvorem o stejném průměru, jako je jeho metrický závit. Podložka i matice budou zapuštěny do záhlubení o průměru 10 mm a hloubce 3 mm.

Očkem bude protaženo syntetické lanko (zednická stavební šňůra) o průměru 2 mm, které bude zajištěno dračí smyčkou zakápnutou vteřinovým lepidlem. Syntetická šňůra nekoroduje, má nízkou průtažnost a je odolná vůči vlhkosti.

Lanko povede kolmo vzhůru do krabičky K, ve které bude navinuto na špulku vytištěnou z materiálu PETG. Špulka bude mít šířku 2 cm a vnější průměr 2,5 cm. Otvorem o průměru 3 mm, umístěným na okraji špulky směrem od motoru, bude provlečeno skrz a upevněno osmičkovým uzlem, který bude rovněž zakápnut vteřinovým lepidlem. Tento uzel bude zapuštěn 8 mm hluboko do záhlubení s průměrem 10 mm. Vývod lanka se bude nacházet na okraji záhlubení směrem k okraji špulky.

Špulka bude opatřena otvorem s D-profilem o průměru 3 mm a pomocí stavěcího šroubu (červíku) M5 z nerezové oceli A2 o délce 5 mm a mosazné závitové vložky M5 (délka 5,8 mm, průměr 7,1 mm) bude upevněna na hřídeli nízkootáčkového stejnosměrného motoru s kovovou převodovkou (6 V). A to ze strany kde bude vstupovat lanko.

Otvor pro mosaznou závitovou vložku bude mít průměr 8 mm a v posledních 6,5 mm směrem ke hřídeli se zúží na průměr 6,5 mm. Do otvoru bude zavedena vložka pomocí dlouhého 15 mm šroubu M5. Jakmile vložka narazí na zúženou část otvoru, bude hlava šroubu nahřáta mikropájkou. Plast se v místě zúžení roztaví, což umožní zalisování vložky. Po odejmutí pájky plast opět ztuhne a vložku pevně ukotví.

V horní i dolní části hranolku budou ve svislé ose zapuštěny dva pákové koncové mikrospínače. Záhlubení široká 6,5 mm a vysoká 20 mm povedou skrz hranolek a 10 mm hliníkovou drážku, kvůli vývodu pinů. Mikrospínače budou upevněny pomocí vrutů o průměru 2 mm. Je důležité otvory předvrtat, kvůli deformaci plastu při vrtání do dřeva.

Vedlejší část systému bude umístěna u spádovaných snáškových hnízd s košíkem orientovaných příčně ke stěně.

Pod kukaně bude pomocí vrutů připevněna březová sítotisková překližka o rozměrech 37 × 28 cm a tloušťce 12 mm. Druhá překližka o rozměrech 35 × 26 cm bude tvořit základovou desku a bude připevněna k podlaze kurníku. Tyto desky budou vyrobeny z jedné nařezané překližky o rozměrech 765 × 317 mm (tolerance započteny). Třetí překližka o rozměrech 40 × 50 cm bude sloužit jako zadní stěna.

Všechny tyto velmi odolné překližky budou orientovány hladkou stranou směrem nahoru, respektive směrem ke hnízdu. Jejich řezné hrany budou opatřeny voděodolným lakem z důvodu ochrany proti vlhkosti.

Mezi horní a spodní překližkou bude u stěny umístěn tenzometr o tloušťce 12,7 mm, orientovaný kratší stranou s vodiči směrem ke stěně. Tenzometr váží s přesností 2 g, což je pro 50-70 g vejce zcela v pořádku. Je dimenzován na zatížení 20 kg a maximální předpokládané zatížení tvořené dvěma překližkami, hnízdem, dvěma slepicemi a deseti vejci činí přibližně 10 kg.

Optická závora není pro počítání vajec v kurníku vhodná mimo jiné z důvodu výskytu prachu, peří, podestýlky a zvědavých slepic. Všechny tyto objekty by mohly přerušit laserový paprsek a být chybně vyhodnoceny jako propadnuté vejce. Z tohoto důvodu byl pro detekci vajec zvolen tenzometr.

Volný konec tenzometru bude upevněn dvěma zápustnými inbusovými šrouby M4 z nerezové oceli A2. Šrouby budou procházet vážicí deskou skrz otvory o průměru 4,5 mm a budou zapuštěny do záhlubení o průměru 10 mm a hloubce 3 mm. Mezi hlavu šroubu a desku bude nanesena tenká vrstva PU tmelu.

Pevný konec tenzometru bude upevněn dvěma zápustnými inbusovými šrouby M5 z nerezové oceli A2. Ty budou procházet základovou deskou skrz otvory o průměru 5,5 mm a budou rovněž zapuštěny do záhlubení o průměru 10 mm a hloubce 3 mm.

Použité šrouby budou mít délku 25 mm. Mezi deskami a závity tenzometru budou umístěny distanční matice M5 a M6 z nerezové oceli A2, o tloušťce 4 mm.

Ze zbytku první překližky o rozměru 38,5 × 29,5 cm, budou vyrobeny lišty. Ty budou pomocí vrutů připevněny ke spodní straně vážicí desky. Lišty budou vysoké 15 mm, protože vzdálenost mezi deskami činí 20,7 mm a celá horní konstrukce musí být podepřena pouze tenzometrem. Současně musí být zachována vůle alespoň 3 mm.

Do zadní lišty bude vyvrtán otvor, kterým bude protažen již prodloužený kabel od tenzometru. Kabel nesmí být mechanicky namáhán tahem. Dále bude veden vzhůru po stěně do krabičky Kx. Z boku krabičky bude vyveden sdělovací kabel, který povede vysoko po stěně až do krabičky K a bude upevněn pomocí hřebíkových příchytek.

Celá konstrukce musí být od stěny vzdálena natolik, aby se jí nikde nedotýkala a současně za ní zůstal prostor pro vedení kabelu. Zároveň je nutné zabránit přístupu slepic a hlodavců do tohoto prostoru.

Uprostřed konstrukce naproti košíku bude vytvořen stavitelný doraz pomocí zápustného inbusového šroubu M5 z nerezové oceli A2 o délce 30 mm. Šroub bude procházet spodní deskou skrz otvor o průměru 4,5 mm a bude zapuštěn do záhlubení o průměru 10 mm a hloubce 6 mm. Doraz bude tvořen mosaznou kloboučkovou maticí M5 (délka 10 mm, závit 7,5 mm) z důvodu omezení ulpívání nečistot. Optimální vůle dorazu je 0,8 mm, což odpovídá jednomu otočení šroubu M5 nebo přibližně tloušťce běžné platební karty.

## Nákup

**Kabeláž**  
https://www.gme.cz/v/1511630/solarkabel-xh-400mm2-cerveny (5 m)  
https://www.gme.cz/v/1511631/solarkabel-xh-400mm2-cerny (5 m)  
https://www.gme.cz/v/1512358/elektrokabel-cya-1x15-cerveny-h07v-k-izolovany-vodic-lanko (5 m)  
https://www.gme.cz/v/1512357/elektrokabel-cya-1x15-cerny-h07v-k-izolovany-vodic-lanko (5 m)  
https://www.gme.cz/v/1512382/elektrokabel-cya-1x05-cerveny-h05v-k-izolovany-vodic-lanko (5m)  
https://www.gme.cz/v/1512360/elektrokabel-cya-1x05-cerny-h05v-k-izolovany-vodic-lanko (5m)  

https://www.alza.cz/alzapower-patch-cat5e-utp?dq=6592131 (5 m)  

Willmann Elektro Lipník (5 m)  

**Svorkovnice a svorky**  
https://www.gme.cz/v/1499112/wago-256-404-svorkovnice-4pol-roztec-508mm-24a-320v-vstup-45-pruzina (2 ks)  
https://www.gme.cz/v/1502753/wago-250-405-svorkovnice-5pol-roztec-25mm-4a-160v-vstup-45-pruzina  
https://www.gme.cz/v/1513875/wago-221-2411-svorka-bezsroubova-2pol-vodic-do-4mm2 (8 ks)  

**Konektor**  
https://www.hadex.cz/p/d626-zdirka-rj45-do-dps-8p8c (5 ks)  

**Klipy**  
Hornbach (cca 20 ks)  

**Bužírky**  
https://www.gme.cz/v/1483738/kss-vs-100bk-sada-smrstovacich-buzirek  

**Solární panel**  
https://www.dexhal.cz/fotovoltaicky-panel-6v-1670ma-10w-solarni-clanek_z2899/ (2 ks)  

**N-MOS**  
https://www.hadex.cz/p/b441-2n7000-mosfet-n-fet-60v-0-35a-to92 (2 ks)  

**P-MOS**  
https://www.gme.cz/v/1493257/infineon-irf4905pbf-unipolarni-tranzistor (4 ks)  

**Akumulátor**  
https://www.levne-baterky.cz/Green-Cell-AGM-Baterie-6V-7Ah-d5513.htm  

**Lineární LDO regulátor**  
https://ecom.cz/eshop/detail/37332 (2 ks)  

**Master**  
https://botland.cz/lora-lorawan/19267-lora-e5-mini-stm32wle5jc-modul-lorawan-868-915-mhz-seeedstudio-113990939-5904422369026.html  

**Slave**  
https://botland.cz/stm32-nucleo/18799-stm32-nucleo-l031k6-s-stm32l031k6-mcu-kompatibilni-s-arduino-nano-5904422364878.html (2 ks)  

**Headery**  
https://www.laskakit.cz/dupont-40pin-2-54-mm-pinovy-pas/  

**Proudový a napěťový senzor**  
https://dratek.cz/arduino-platforma/1437-ina219-proudovy-snimac-obousmerny.html  

**H-bridge**  
https://botland.cz/ovladace-stejnosmerneho-motoru/2695-drv8838-jednokanalovy-budic-motoru-11v-17a-pololu-2990-5903351244855.html  

**Motor**  
https://dratek.cz/arduino-platforma/3197-stejnosmerny-motor-s-prevodovkou-dc-6v-20-rpm.html  

**Mikrospínač**  
https://www.hadex.cz/p/l405a-mikrospinac-kw11-3z-on-on-1pol-250v-5a-s-packou-13mm (2 ks)  

**Dvířka**  
https://eshop.zenit.cz/desky/pvc-penene-desky/palight-print-8mm/variant/2030-3050/  

**Hnízdo**  
https://www.lihneme.cz/plastova-snaskova-hnizda/hf44x37x50-zelene/ (2 ks)  

**Tenzometr**  
https://www.laskakit.cz/vazici-senzor-20kg-adc-modul-hx711/ (2 ks)  

**Převodník**  
https://www.laskakit.cz/ad-prevodnik-modul-24-bit-2-kanaly-hx711/ (2 ks)  

**Transceiver**  
https://www.soselectronic.com/cz-cz/products/maxim/max3485esa-1-121397?query=max3485 (3 ks)  

**Adaptér**  
https://dratek.cz/arduino-platforma/1046-dps-adapter-sop8-so8-soic8-na-dip8.html (3ks)  

**Kondenzátor**  
https://www.gme.cz/v/1486151/hitano-ck-1u-50v-x7r-rm508-10-keramicky-kondenzator (4 ks)  

**Překližka**  
Hornbach (4 ks)  

**Červík**  
Hornbach (2 ks)  

**Lanko**  
Hornbach  

**Očko**  
Hornbach  

**Šrouby Matice Podložky Vruty**  
Hornbach (11, 11, 2, sada)  

**PU tmel**  
Hornbach  

|               |             |
| :---          | :---:       |
| **OBCHOD**    | **CENA**    |
| GME           | 850 Kč      |
| ALZA          | 200 Kč      |
| HADEX         | 200 Kč      |
| HORNBACH      | 800 Kč      |
| DEXHAL        | 750 Kč      |   
| BATERKY       | 350 Kč      |
| ECOM          | 100 Kč      |
| LASKAKIT      | 600 Kč      |
| DRATEK        | 300 Kč      |
| BOTLAND       | 1800 Kč     |
| ZENIT         | 100 Kč      |
| LIHNEME       | 1000 Kč     |
| SOS           | 450 Kč      |
| REZERVA       | 500 Kč      |
| **CELKEM**    | **8950 Kč** |

*Poznámka: Cena je orientační a je do ní započtena i doprava.*
