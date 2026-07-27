# Kurník

Systém pro automatizaci kurníku s detekcí snesených vajec

&nbsp;

## Zadání

&nbsp;

- Prostudujte možnosti automatizace uzavírání kurníku, možné metody detekce snesených vajec a dostupné možnosti komunikace a připojení zařízení do IoT sítě (např. NB-IoT, WiFi, LoRaWAN).
- Navrhněte systém pro automatické otevírání a uzavírání dvířek kurníku s možností rozšíření o jednotky se senzory ve snáškových hnízdech (min. 5 hnízd). Prostudujte možnosti napájení z akumulátoru nebo fotovoltaického panelu. Proveďte analýzu možnosti přenosu dat do cloudu a jejich zobrazení uživateli včetně historie snášek a možnosti vzdáleného ovládání, například skrze aplikaci.
- Na základě analýzy navrhněte způsob vzdálené uživatelské interakce pro zobrazení stavu dvířek, manuální ovládání a počítání snesených vajec.
- Navrhněte schéma zařízení, které bude obsahovat mikrokontrolér, komunikační modul, senzory, řízení dvířek, napájecí/nabíjecí obvody. Dbejte na nízký příkon celého zařízení. Proveďte návrh desky plošných spojů.
- Zkonstruujte hardwarovou realizaci navrženého systému. Osaďte a zprovozněte řídicí elektroniku. Vytvořte ovládací firmware. Realizujte přenos dat ze zařízení k uživateli včetně ovládacího rozhraní a možnosti zobrazení historie.
- Ověřte funkčnost systému experimentálním měřením a vyhodnoťte spolehlivost detekce a ovládání.
- Zveřejněte veškeré výrobní podklady na vhodné platformě (např. GitHub).

&nbsp;

## Schéma

&nbsp;

<img src="https://github.com/Hosty-04/ChickenCoop/blob/main/schematics/block_schematic_white.png" alt="block_schematic" height="1080px">

&nbsp;

## Popis

&nbsp;

### Kabeláž
Pro připojení solárního panelu bude použita měděná ohebná licna o průřezu 1,5 mm², připojená přes 4pinovou pružinovou WAGO svorkovnici s roztečí 5,08 mm k desce plošných spojů a odtud dále k akumulátoru. Tento průřez poskytuje dostatečnou proudovou rezervu při minimálním úbytku napětí.

Stejným způsobem budou k desce plošných spojů připojeny mikrospínače, avšak s licnou o průřezu 0,5 mm². Spoje této licny s kontakty mikrospínače budou izolovány smršťovacími bužírkami s poměrem 2:1 a vnitřním průměrem 2 mm / 0,75 mm (před / po zahřátí), přečnívajícími asi centimetr přes izolaci.

Kabely povedou v klasické elektroinstalační PVC liště o rozměrech 15 × 10 mm, upevněné k betonové stěně pomocí vrutů 3 × 30 mm a hmoždinek o průměru 5 mm — dostatečně prostorné, a přitom minimalistické řešení. Speciální UV odolná lišta není potřeba, protože stěna kurníku, na které budou lišty umístěny, bude vystavena slunci pouze při jeho západu; životnost běžné lišty se odhaduje na 5-10 let.

Pro datovou komunikaci byl zvolen kabel UTP CAT5e typu licna, upevněný k betonové stěně pomocí 6mm šroubovacích příchytek, vrutů 4 × 20 mm a hmoždinek o průměru 6 mm. K deskám plošných spojů bude připojený přes konektory RJ45.

Prodloužení vodičů tenzometru zajistí stíněný kabel LiYCY 4 × 0,25 mm² typu licna, spojený přes prodlužovací pružinové WAGO svorky. Protože jsou vodiče od tenzometru (průřez 0,14 mm²) pro tyto svorky příliš tenké, přehnou se u odizolovaného konce napůl, čímž se jejich efektivní průřez zdvojnásobí na 0,28 mm². K desce plošných spojů bude tento kabel spolu se stíněním připojen přes 6pinovou pružinovou WAGO svorkovnici s roztečí 2,54 mm.

Oplet kabelu (pocínované měděné drátky) se izoluje smršťovací bužírkou 2:1 o vnitřním průměru 4 mm / 1,75 mm. Odmotaný a spletený oplet je potřeba oddělit od zbylých čtyř vodičů a bužírku navléknout až ke kořenu; přes celý kabel se pak přetáhne bužírka 2:1 o rozměru 7,5 mm / 3,5 mm, přečnívající asi centimetr přes hlavní izolaci.

U prototypu budou využity stejné kabely, svorkovnice a svorky. Dále budou použity drátky do nepájivého pole o průřezu 0,5 mm², který plně vyhovuje proudovému odběru systému.

&nbsp;

### Napájení
Výrobu energie zajistí fotovoltaický panel o parametrech 9 V / 10 Wp, svisle připevněný na stěnu mimo výběh pod malou stříšku a orientovaný na jih, případně na východ nebo západ (v tomto případě na jihozápad), aby co nejlépe využíval dostupnou sluneční energii. Vertikální montáž mimo výběh zároveň omezí usazování sněhu a nečistot. Tento panel byl zvolen proto, že při použití jednoduchého MOSFET odpojovače poskytuje vhodný poměr mezi napěťovou rezervou pro nabíjení 6V akumulátoru a dostupným nabíjecím proudem; panel je schopen reálně dodat maximálně kolem 1,2 A, tudíž je přesně na hranici nejvyššího povoleného nabíjecího proudu akumulátoru (1,2 A). Jeho vyšší výkon navíc zvyšuje energetickou rezervu systému v zimě, kdy je intenzita slunečního záření nízká. Účinnost panelu je přibližně 95 %, z důvodu teplotních ztrát a nepatrného úbytku napětí na krátké kabeláži.

Systém bude napájen z bezúdržbového olověného AGM akumulátoru 6 V / 4 Ah, umístěného venku ve stínu asi 25 cm pod stříškou. Jeho nabíjecí účinnost dosahuje přibližně 88 %, samovybíjení dosahuje 3 % měsíčně a v zimě ztrácí přibližně 30 % kapacity. Akumulátor typu LiFePO4 je sice v mnoha ohledech kvalitnější, nesmí se však nabíjet v zimě pod 0 °C a vyžaduje složitější nabíjecí systém. Vzhledem k volbě venkovního umístění a jednoduchého nabíjecího systému je pro celoroční provoz vhodnější olověný akumulátor. Je důležité mít na paměti životnost kolem 5 let a 15 % ztrátu kapacity ročně.

Před akumulátorem bude zapojen nízkopříkonový, mikrořadičem řízený MOSFET odpojovač fotovoltaického zdroje s ochranou akumulátoru. Od použití MPPT regulátoru se ustoupilo kvůli vyšší složitosti a vlastní spotřebě spínaného měniče — u systému s velmi nízkým denním odběrem by zlepšení účinnosti nabíjení, probíhajícího jen několik minut denně, nepřineslo oproti jednoduchému odpojovači s téměř nulovou klidovou spotřebou žádný významný energetický přínos. Účinnost pracovního bodu dosahuje přibližně 75 %, neboť akumulátor stahuje napětí panelu na svou úroveň (průměrně 6,8 V) a panel tak nepracuje v bodě maximálního výkonu, ale v oblasti konstantního proudu; účinnost MOSFET odpojovače dosahuje 97 %.

Silová část systému bude pracovat s napětím 6 V, veškerá elektronika pak s napětím 3,3 V. Snížení napětí zajistí nízkopříkonový LDO regulátor MCP1702 s přesnou stabilizací z 2,7-13,2 V na 3,3 V ± 2 %, dostačujícím výstupním proudem 250 mA a velmi nízkým klidovým proudem. Na jeho vstupu i výstupu bude připojen blokovací keramický kondenzátor 1 µF / 50 V — na vstupu jako filtrace, na výstupu pro stabilizaci napětí. Použití spínaného buck měniče není vhodné kvůli horší dostupnosti nízkopříkonových variant a velmi nízkému odběru systému po většinu dne. Jeho vyšší účinnost by se projevila jen po několik minut denně a kvůli vlastní spotřebě by paradoxně dosahoval nižší celkové účinnosti než jednoduchý lineární LDO regulátor.

&nbsp;

## Denní přehled (5 hnízd)

&nbsp;

### Klidová spotřeba (24 hod / 2-10 hod / 24 min / 16 min)

&nbsp;

| Komponenta | Proud (typ) | Proud (max) | Spotřeba (typ) | Spotřeba (max) |
|---|---|---|---|---|
| LDO (MCP1702) | 2 µA | 5 µA | 48 µAh | 120 µAh |
| INA216 (shutdown) | 0,6 µA | 2,5 µA | 14,4 µAh | 60 µAh |
| DRV8838 (shutdown) | 80 nA | 120 nA | 1,92 µAh | 2,88 µAh |
| Spínač (N) | 7,1 µA | 7,1 µA | 14,2 µAh | 71 µAh |
| Spínače (P,pu) | 33 µA | 33 µA | 13,2 µAh | 13,2 µAh |
| Spínače (leak) | 6 × 1,1 µA | 6 × 5,1 µA | 6 × 26,4 µAh | 6 × 122 µAh |
| Spínače (P,pd) | 33 µA | 33 µA | 8,8 µAh | 8,8 µAh |
| M (Stop2 s RTC) | 1 µA | 26 µA | 24 µAh | 624 µAh |
| Mx (Stop bez RTC) | 5 × 0,38 µA | 5 × 1,9 µA | 5 × 9,12 µAh | 5 × 45,6 µAh |
| **Celkem** | **85,3 µA** | **147 µA** | **329 µAh** | **1,86 mAh** |

&nbsp;

$$
I_N = \frac{U_{nap}}{R_{470} + R_G} + I_{GSS} = \frac{3,3\ \text{V}}{470\ \text{k}\Omega + 220\ \Omega} + 100\ \text{nA} \approx \mathbf{7,1\ \text{µA}}
$$

$$
I_P = \frac{U_{nap}}{R_{100} + R_G} = \frac{3,3\ \text{V}}{100\ \text{k}\Omega + 220\ \Omega} \approx \mathbf{33\ \text{µA}}
$$

$$
t_{P,pu} = 24 \cdot t_{P,pu,on} = 24 \cdot (20\ \text{s} + 16\ \text{s} + 12\ \text{s} + 8\ \text{s} + 4\ \text{s}) = \mathbf{24\ \text{min}}
$$

$$
I_{leak,min} = I_{DSS,min} + I_{GSS} = 1\ \mu\text{A} + 100\ \text{nA} = \mathbf{1,1\ \mu\text{A}}
$$

$$
I_{leak,max} = I_{DSS,max} + I_{GSS} = 5\ \mu\text{A} + 100\ \text{nA} = \mathbf{5,1\ \mu\text{A}}
$$

$$
t_{P,pd} = 24 \cdot t_{P,pd,off} = 24 \cdot (16\ \text{s} + 12\ \text{s} + 8\ \text{s} + 4\ \text{s}) = \mathbf{16\ \text{min}}
$$

&nbsp;

kde:
- $I_N$ ... proud tekoucí pull-down rezistorem spínačem s N-MOS tranzistorem
- $U_{nap}$ ... napájecí napětí
- $R_{470}$ ... pull-down rezistor o hodnotě 470 kΩ
- $I_P$ ... proud tekoucí pull-down/pull-up rezistorem spínači s P-MOS tranzistorem
- $R_{100}$ ... pull-down rezistor o hodnotě 100 kΩ
- $R_G$ ... rezistor připojený sériově za M
- $t_{P,pu}$ ... čas po který teče pull-up a ochrannými rezistory spínačů s P-MOS tranzistorem proud
- $t_{P,pu,on}$ ... čas po který jsou spínače s P-MOS tranzistorem s pull-up rezistorem sepnuty
- $t_{P,pd}$ ... čas po který teče pull-down a ochrannými rezistory spínačů s P-MOS tranzistorem proud
- $t_{P,pd,off}$ ... čas po který jsou spínače s P-MOS tranzistorem s pull-down rezistorem rozepnuty
- $I_{leak,min}$ ... minimální svodový proud spínačů s P-MOS tranzistorem
- $I_{leak,max}$ ... maximální svodový proud spínačů s P-MOS tranzistorem
- $I_{DSS,min}$ ... minimální svodový proud spínačů s P-MOS tranzistorem tekoucí přes drain
- $I_{DSS,max}$ ... maximální svodový proud spínačů s P-MOS tranzistorem tekoucí přes drain
- $I_{GSS}$ ... svodový proud tekoucí přes gate

&nbsp;

*Poznámka: U MOSFET oddělovače přispívá do spotřeby pouze pull-down rezistor při sepnutí N-MOS tranzistoru po 2-10 hodin denně, kvůli nabíjení akumulátoru, a svodový proud tekoucí do gate N-MOS tranzistoru po celý den. Ostatní části systému jsou odpojovány přes tranzistorové spínače — 6 spínačů s P-MOS tranzistorem a pull-up rezistorem a 5 s pull-down rezistorem. Spínače s P-MOS tranzistorem s pull-up rezistorem spotřebovávají energii pouze tehdy, když probíhá kontrola vajec a jsou sepnuty (každý z nich je sepnutý jinak dlouho); svodové proudy jim tečou přes gate celý den a přes drain při jejich rozepnutí (téměř celý den); ty tekoucí přes gate jsou při sepnutí zanedbatelné. Spínače s P-MOS tranzistorem s pull-down rezistorem spotřebovávají energii pouze když probíhá kontrola vajec a jsou rozepnuty (každý z nich je rozepnutý jinak dlouho); svodové proudy jimi tečou pouze při přívodu napájecího napětí — zanedbatelná doba. Přes ochranné rezistory teče proud pouze po velmi krátkou dobu při změně stavu spínače.*

&nbsp;

### Kontrola fotovoltaického panelu a akumulátoru (1 s a 11 s)

&nbsp;

| Komponenta | Proud (typ) | Proud (max) | Spotřeba (typ) | Spotřeba (max) |
|:---|:---:|:---:|:---:|:---:|
| INA226 | 330 µA | 420 µA | 1,01 µAh | 1,28 µAh |
| M (LPRun @ 1 MHz) | 120 µA | 390 µA | 100 nAh | 325 nAh |
| **Celkem** | **450 µA** | **810 µA** | **1,11 µAh** | **1,61 µAh** |

&nbsp;

$$
t_{p,c} = 144 \cdot t_{p,v} = 144 \cdot 5\ \text{ms} \approx \mathbf{1\ \text{s}}
$$

$$
t_{a,c} = 144 \cdot t_{a,v} = 144 \cdot 75\ \text{ms} \approx \mathbf{11\ \text{s}}
$$

&nbsp;

kde:
- $t_{p,c}$ ... celková doba měření napětí na panelu
- $t_{p,v}$ ... doba vzorkování napětí na panelu
- $t_{a,c}$ ... celková doba měření napětí na akumulátoru
- $t_{a,v}$ ... doba vzorkování napětí na akumulátoru

&nbsp;

### Pohyb dvířek (32-73 s)

&nbsp;

| Komponenta | Proud (typ) | Proud (max) | Spotřeba (typ) | Spotřeba (max) |
|:---|:---:|:---:|:---:|:---:|
| Motor | 100 mA | 250 mA | 0,889 mAh | 5,07 mAh |
| DRV8838 | 340 µA | 600 µA | 3,02 µAh | 12,2 µAh |
| INA226 | 330 µA | 420 µA | 2,93 µAh | 8,52 µAh |
| M (LPRun @ 1 MHz) | 120 µA | 390 µA | 1,07 µAh | 7,91 µAh |
| **Celkem** | **101 mA** | **251 mA** | **0,896 mAh** | **5,10 mAh** |

&nbsp;

$$
O_s = \pi \cdot d_s = \pi \cdot 25\ \text{mm} = 78,5\ \text{mm}
$$

$$
v_{min} = f_{min} \cdot O_s = \frac{15\ \text{rpm}} {60} \cdot 78,5\ \text{mm} = 19,6\ \text{mm/s}
$$

$$
v_{max} = f_{max} \cdot O_s = \frac{17\ \text{rpm}} {60} \cdot 78,5\ \text{mm} = 22,3\ \text{mm/s}
$$

$$
t_{min} = 2 \cdot \frac{h}{v_{max}} = 2 \cdot \frac{35\ \text{cm}}{22,3\ \text{mm/s}} = 2 \cdot 15,7\ \text{s} \approx \mathbf{32\ \text{s}}
$$

$$
t_z = \frac{h}{v_{min}} + 0,5\ \text{s} = \frac{35\ \text{cm}}{19,6\ \text{mm/s}} + 0,5\ \text{s} = 18,4\ \text{s}
$$

$$
t_{max} = 2 \cdot \frac{h}{v_{min}} + 2 \cdot t_z = 2 \cdot \frac{35\ \text{cm}}{19,6\ \text{mm/s}} + 2 \cdot 18,4\ \text{s} = 2 \cdot 17,9\ \text{s} + 2 \cdot 18,4\ \text{s} \approx \mathbf{73\ \text{s}}
$$

&nbsp;

kde:
- $t_{max}$ ... maximální čas potřebný pro otevření a zavření dvířek
- $t_z$ ... zpoždění při zaseknutí dvířek
- $t_{min}$ ... minimální čas potřebný pro otevření a zavření dvířek
- $v_{max}$ ... maximální rychlost otáčení špulky
- $v_{min}$ ... minimální rychlost otáčení špulky
- $f_{max}$ ... maximální frekvence otáčení špulky
- $f_{min}$ ... minimální frekvence otáčení špulky
- $O_s$ ... obvod špulky
- $d_s$ ... průměr špulky
- $h$ ... výška dvířek

&nbsp;

*Poznámka: Mikrospínače budou spotřebovávat málo energie po velmi krátkou dobu; výpočet astronomických hodin bude trvat pouze jednu milisekundu.*

&nbsp;

### Kontrola vajec (480-512 s)

&nbsp;

| Komponenta | Proud (typ) | Proud (max) | Spotřeba (typ) | Spotřeba (max) |
|:---|:---:|:---:|:---:|:---:|
| M (LPRun @ 1 MHz) | 120 µA | 390 µA | 16,0 µAh | 55,5 µAh |
| MAX3485 (M) | 1,1 mA | 2,2 mA | 147 µAh | 313 µAh |
| MAX3485 (Mx) | 1,1 mA | 2,2 mA | 147 µAh | 313 µAh |
| Mx (LPRun @ 131 kHz) | 32 µA | 37 µA | 4,27 µAh | 5,26 µAh |
| HX711 a tenzometr | 4,4 mA | 4,4 mA | 587 µAh | 626 µAh |
| **Celkem** | **6,75 mA** | **9,23 mA** | **0,9 mAh** | **1,31 mAh** |

&nbsp;

$$
t = t_i + t_v = 0,5\ \text{s} + \frac{32}{10} = 0,5\ \text{s} + 3,2\ \text{s} = 3,7\ \text{s} \approx \mathbf{4\ \text{s}}
$$

$$
t_{min} = 24 \cdot h \cdot t = 24 \cdot 5 \cdot 4\ \text{s} = \mathbf{480\ \text{s}}
$$

$$
t_r = 2 \cdot h \cdot t_v = 2 \cdot 5 \cdot 3,2\ \text{s} = 32\ \text{s}
$$

$$
t_{max} = 24 \cdot h \cdot t + t_r = 24 \cdot 5 \cdot 4\ \text{s} + 32\ \text{s} = \mathbf{512\ \text{s}}
$$

&nbsp;

kde:
- $t_{max}$ ... maximální doba každohodinové kontroly h hnízd
- $t_r$ ... čas navíc při aktualizaci referenční nulové hodnoty h tenzometrů
- $t_{min}$ ... minimální doba každohodinové kontroly h hnízd
- $t$ ... doba kontroly jednoho hnízda
- $t_i$ ... doba inicializace
- $t_v$ ... doba vzorkování

&nbsp;

*Poznámka: STM32 NUCLEO-L031K6, MAX3485, HX711 a tenzometr jsou přítomny v každé krabičce Kx, ale díky chytrému využití tranzistorových spínačů a režimů řadiče je zapnuté vždy jen to, co zrovna pracuje — to znamená několikrát nižší spotřebu. Bez použití spínačů by se v nejlepším možném případě a bez započtení klidového odběru spotřeba zvedla více jak 4× oproti nejhoršímu možnému případu architektury se spínači.*

&nbsp;

### Komunikace (4 s a 1-3 s)

&nbsp;

| Komponenta | Proud (typ) | Proud (max) | Spotřeba (typ) | Spotřeba (max) |
|:---|:---:|:---:|:---:|:---:|
| LoRa TX | 21 mA | 21 mA | 23,3 µAh | 23,3 µAh |
| LoRa RX | 4,8 mA | 4,8 mA | 1,33 µAh | 4 µAh |
| CPU (LPSleep) | 44 µA | 310 µA | 0,183 µAh | 1,29 µAh |
| **Celkem** | **25,8 mA** | **26,1 mA** | **24,8 µAh** | **28,6 µAh** |

&nbsp;

$$
t_v = 24 \cdot t_{5B} + 2 \cdot t_{2B} = 24 \cdot 150\ \text{ms} + 2 \cdot 130\ \text{ms} = 3,86\ \text{s} \approx \mathbf{4\ \text{s}}
$$

$$
t_{p,min} = 26 \cdot t_{o,min} = 26 \cdot 30\ \text{ms} = 0,78\ \text{s} \approx \mathbf{1\ \text{s}}
$$

$$
t_{p,max} = 2 \cdot 26 \cdot t_{o,max} = 2 \cdot 26 \cdot 50\ \text{ms} = 2,6\ \text{s} \approx \mathbf{3\ \text{s}}
$$

&nbsp;

kde:
- $t_v$ ... doba vysílání
- $t_{5B}$ ... airtime pro preambuli + 5B + zabezpečení
- $t_{2B}$ ... airtime pro preambuli + 2B + zabezpečení
- $t_{p,min}$ ... minimální doba přijmu
- $t_{p,max}$ ... maximální doba přijmu
- $t_{o,min}$ ... minimální doba příjmového okna
- $t_{o,max}$ ... maximální doba příjmového okna

&nbsp;

### Procentuální rozložení a celková denní spotřeba

&nbsp;

| Blok | Spotřeba (typ) | Podíl | Spotřeba (max) | Podíl |
|---|---|---|---|---|
| Kontrola vajec | 0,9 mAh | 41,8 % | 1,31 mAh | 15,8 % |
| Pohyb dvířek | 0,896 mAh | 41,7 % | 5,1 mAh | 61,4 % |
| Klidový režim | 329 µAh | 15,3 % | 1,86 mAh | 22,4 % |
| Komunikace | 24,8 µAh | 1,2 % | 28,6 µAh | 0,3 % |
| Kontrola panelu a baterie | 1,11 µAh | 0,1 % | 1,61 µAh | 0,0 % |
| **Celkem** | **2,150 mAh** | **100 %** | **8,300 mAh** | **100 %** |

&nbsp;

### Energie dodávaná do akumulátoru

&nbsp;

| Orientace | Léto (mAh/den) | Zima (mAh/den) |
|:---|:---:|:---:|
| Jih | 2662 | 1588 |
| Východ | 2547 | 453 |
| Západ | 2662 | 567 |
| Jihozápad | 2772 | 1248 |

&nbsp;

$$
P_{vst} = U_{aku} \cdot I_{max} = 6,8\ \text{V} \cdot 1,2\ \text{A} = 8,16\ \text{W}
$$

$$
P_{ztr} = I_{max}^2 \cdot 2 \cdot R_{DSon} = 1,2^2\ \text{A} \cdot 2 \cdot 100\ \text{mΩ} = 288\ \text{mW}
$$

$$
U_{ztr} = I_{max} \cdot 2 \cdot R_{DSon} = 1,2\ \text{A} \cdot 2 \cdot 100\ \text{mΩ} = 240\ \text{mV}
$$

$$
\eta_{bias} = \frac{P_{vst}}{P_{max}} = \frac{8,16\ \text{W}}{10\ \text{W}} = 0,816
$$

$$
\eta_{mos} = \frac{P_{vst} - P_{ztr}}{P_{vst}} = \frac{8,16\ \text{W} - 288\ \text{mW}}{8,16\ \text{W}} = 0,965
$$

&nbsp;

**Příklad výpočtu pro léto, jih**

&nbsp;

$$
E_{den} = \frac{E_{červen} + E_{červenec} + E_{srpen}}{\text{92 dní}} = \frac{0,7 + 0,8 + 0,9\ \text{kWh}}{92} = 26,1\ \text{Wh/den}
$$

$$
E_{aku} = E_{den} \cdot \eta_{bias} \cdot \eta_{mos} \cdot \eta_{aku} = 26,1\ \text{Wh/den} \cdot 0,816 \cdot 0,965 \cdot 0,88 = \mathbf{18,1\ \text{Wh/den}}
$$

$$
Q_{aku} = \frac{E_{aku}}{U_{aku}} = \frac{18,1\ \text{Wh/den}}{6,8\ \text{V}} = \mathbf{2662\ \text{mAh/den}}
$$

&nbsp;

kde:
- $E_{den}$ ... energie vyrobená za jeden den
- $E_{měsíc}$ ... energie vyrobená za daný měsíc
- $E_{aku}$ ... energie nabíjející akumulátor
- $\eta_{bias}$ ... účinnost pracovního bodu
- $\eta_{mos}$ ... účinnost MOSFET oddělovače
- $P_{vst}$ ... dosažitelný výkon panelu při plném osvitu v pracovním bodě daném akumulátorem
- $P_{ztr}$ ... ztrátový výkon MOSFET oddělovače
- $U_{ztr}$ ... úbytek napětí MOSFET oddělovače
- $P_{max}$ ... maximální výkon panelu při plném osvitu
- $I_{max}$ ... maximální proud panelu při plném osvitu
- $\eta_{aku}$ ... účinnost nabíjení akumulátoru
- $Q_{aku}$ ... náboj nabíjející akumulátor
- $U_{aku}$ ... průměrné napětí akumulátoru

&nbsp;

*Poznámka: Pro zjištění výkonu fotovoltaického panelu v lokalitě kurníku bylo využito nástroje PVGIS. Úbytek napětí MOSFET oddělovače nijak neovlivní účinnost pracovního bodu panelu, protože panel pracuje v oblasti konstantního proudu. Vliv pull-up rezistoru pro P-MOS tranzistor, svodového proudu tekoucího přes gate P-MOS tranzistoru a vniřního odporu N-MOS tranzistoru je na účinnnost MOSFET oddělovače a pracovního bodu panelu minimální. Tok svodového proudu přes drain P-MOS i N-MOS tranzistoru je vůči napájecímu proudu z panelu zanedbatelný a vyskytuje se pouze když jsou spínače rozepnuty.*

&nbsp;

### Energetická bilance

&nbsp;

| Orientace | Léto (mAh/den) | Zima (mAh/den) |
|:---|:---:|:---:|
| Jih | +2647 | +1573 |
| Východ | +2532 | +438 |
| Západ | +2647 | +552 |
| Jihozápad | +2757 | +1233 |

&nbsp;

$$
Q_{ztr} = Q_{aku} \cdot \frac{3\ \text{%}}{\text{30 dní}} = 4\ \text{Ah} \cdot \frac{3\ \text{%}}{30} = \mathbf{4\ \text{mAh/den}}
$$

&nbsp;

kde:
- $Q_{ztr}$ ... náboj ztracený samovybíjením akumulátoru
- $Q_{aku}$ ... náboj akumulátoru

&nbsp;

*Poznámka: Energetická bilance je rozdílem energie dodávané do akumulátoru a součtu maximální denní spotřeby a náboje ztraceného samovybíjením akumulátoru.*

&nbsp;

Systém nabízí spolehlivý celoroční provoz s obrovskou energetickou rezervou nehledě na orientaci fotovoltaického panelu. I se zohledněním zimního poklesu kapacity akumulátoru o 30 % představuje jeho rezerva několik stovek dní provozu — v praxi bude provozní dobu omezovat spíše několik týdnů nepříznivého počasí v kombinaci s přirozeným stárnutím akumulátoru než samotná spotřeba systému a samovybíjení.

&nbsp;

### Řízení
Hlavní řídicí jednotkou systému bude mikrořadič STM32WLE5JC LoRa-E5 mini (M) s integrovaným LoRa modulem, komunikujícím přes LoRaWAN stack. Technologie LoRaWAN umožní na rozdíl od Wi-Fi komunikaci na velké vzdálenosti při nízké spotřebě energie a na rozdíl od NB-IoT trvalé řešení s dobrým pokrytím. U každého snáškového hnízda bude umístěn další mikrořadič STM32 NUCLEO-L031K6 (Mx), který má integrovaný programátor využitelný i pro hlavní řadič.

Firmware bude vyvíjen v prostředí STM32CubeIDE. Součástí firmwaru hlavního řadiče budou astronomické hodiny, které každý den ve 12 hodin po kontrole solárního panelu a akumulátoru pomocí RTC obvodu spočítají čas východu a západu slunce; podle těchto údajů se pak budou automaticky otevírat a zavírat dvířka kurníku. Přes RTC hodiny bude řadič schopen zjistit i roční období. Použití prostého časovače nebylo zvoleno kvůli proměnlivé délce dne, a světelný senzor byl zavržen proto, že by mohl vyvolat chybné sepnutí motoru dvířek při zatažené obloze (déšť, bouřka) nebo vlivem pouličního osvětlení či světlometů automobilů. Konfiguraci periferií a hodin zajistí grafický nástroj STM32CubeMX.

Hlavní řídicí jednotka se bude společně s nezbytnými částmi systému probouzet každých 10 minut, aby zkontrolovala stav solárního panelu a akumulátoru. Dále se bude spolu s ostatními řídicími jednotkami a dalšími potřebnými částmi systému probouzet každou hodinu vždy pět minut před celou, kdy postupně provede u všech hnízd aktualizaci počtu vajec. Nakonec se bude probouzet opět pouze s nezbytnými částmi systému, a to ráno a večer mimo ostatní aktivity, kvůli otevření a zavření dvířek. Když dojde k odložení tohoto úkonu, tak bude zajištěno aby se nekřížil s žádnou jinou aktivitou. Po sběru dat ze všech hnízd nebo po změně stavu dvířek následuje komunikace.

LoRa anténa bude moci vysílat teprve po vypnutí všech ostatních systémů, a to kvůli jejímu vyššímu odběru proudu a ochraně proti rušení. Po každém vysílání bude mít možnost přijímat data, což umožní uživatelské ovládání. Upřednostňované parametry komunikace jsou: vysílací výkon 12 dBm, SF9, šířka pásma 125 kHz, kódovací poměr 4/5, LoRaWAN Class A - primární příjmové okno RX1 a záložní okno RX2. V domě bude umístěna LoRaWAN gateway, plnící funkci internetové brány. Veškerá přijatá data budou odeslána do cloudu (TTN) a odtud přes MQTT na backend (Node.js), který je uloží do databáze (InfluxDB) a zobrazí na frontendu. Při odesílání dat do kurníku probíhá proces obráceně.

Data budou z kurníku odesílána ve třech a více bajtech. První bajt ponese 1 bit pro indikaci zapnutí/vypnutí systému a 7 bitů pro napětí solárního panelu (rozsah 0–9 V, krok 70 mV). Druhý bajt bude obsahovat 6 bitů pro napětí akumulátoru (5,75–8 V, krok 35 mV) a 2 bity pro stav dvířek (otevřeno / zavřeno / porucha). Další bajty budou po čtyřech bitech alokovány pro počet vajec v jednotlivých snáškových hnízdech (0–10 vajec na hnízdo). Kurník bude data přenášet každou hodinu po kontrole stavu hnízd a dvakrát denně při změně stavu dvířek — v tom případě odešle pouze první dva bajty. Manuální ovládání využije jediný bajt: jeden bit pro vypnutí/zapnutí systému, druhý pro vypnutí/zapnutí kritického režimu (nízké napětí na akumulátoru) a poslední pro ovládání dvířek (zavřít/otevřít).

Pro přenos dat mezi hlavní řídicí jednotkou (master) a ostatními řídicími jednotkami (slave), propojenými sériově v topologii daisy chain, bude použit protokol LPUART, který nevyžaduje hodinový signál a vyznačuje se nízkou spotřebou energie. Vzhledem ke krátké délce vedení v řádu jednotek metrů nebude nutné na začátek ani konec sběrnice připojovat terminační rezistory 120 Ω pro impedanční přizpůsobení vedení — jejich použití by pouze zvyšovalo proudový odběr systému. Přenosová rychlost bude kvůli minimalizaci odrazů 9600 Bd. Na aplikační vrstvě poslouží protokol Modbus RTU spolu s knihovnou ModbusRTU-Slave. Modbus RTU vytvoří datový rámec obsahující adresu jednotky slave, přenášená data a kontrolní součet CRC pro detekci chyb při přenosu. Hardware LPUART v řadiči následně převede jednotlivé bajty na sériový datový tok, doplní start a stop bity a zajistí jejich přenos po sběrnici; na straně přijímače proběhne opačný proces.

Většinu dne bude hlavní řídicí jednotka v režimu Stop2 s RTC. Tento režim se vyznačuje velmi nízkou spotřebou a na rozdíl od režimu StandBy s RTC dokáže mimo jiné udržet logické úrovně a nastavení pinů. Řadič je taktován externím krystalem LSE, umístěným na LoRa-E5 mini, na 32 kHz. Jakmile ale RTC hodiny signalizují, že je čas na práci, přepne se řadič do režimu LP Run (Low-Power Run), taktovaného interním krystalem LSI na 1 MHz. Pro složitý výpočet astronomických hodin řadič zvolí strategii Race-to-Sleep. Ta spočívá v přepnutí do méně úsporného, ale rychlejšího režimu Run (MSI, 48 MHz) po velmi krátkou dobu jedné milisekundy. V průběhu přenosu dat (Radio TX/RX) se CPU přepne do režimu LP Sleep (LSI, 1 MHz). Kvůli nízké taktovací frekvenci je potřeba v souboru lorawan_conf.h zvýšit RADIO_WAKEUP_TIME z 2 na 5 ms; rádio poběží automaticky na 32 MHz a po skončení přenosu jej bude nutné nastavit do režimu LP Sleep. Tento řadič bude využívat SMPS režimu.

U ostatních řídicích jednotek to bude po většinu dne velmi podobné — ze stejných důvodů a protože je potřeba uchovat obsah paměti RAM. Tentokrát to ale bude režim Stop bez RTC. Řadiče budou postupně probouzeny a uspávány pomocí sběrnice LPUART přes hlavní řadič, díky čemuž nepotřebují vlastní RTC hodiny. Po probuzení se daný řadič přepne do režimu LP Run (LSI, 131 kHz) a ihned po vykonání úkolu se vrátí zpět do režimu Stop bez RTC. Přechod mikrořadičů mezi úspornými režimy (Stop2/Stop) a režimem LP Run / Run trvá řádově jednotky až desítky mikrosekund včetně obnovení systémových hodin. Ve srovnání s dobou měření senzorů (desítky milisekund až sekundy) je tato doba zanedbatelná.

Po připojení napájení VCC k jednotlivým částem systému nebo jejich probuzení je nutné počkat na jejich ustálení. U obvodu INA226 se použije čekací doba 200 µs, zahrnující náběh napájení, stabilizaci obvodu a nabití blokovacího keramického kondenzátoru 100 nF mezi VCC a GND. Při měření napětí s průměrováním 64 vzorků rychlostí 1,1 ms/vzorek trvá vytvoření hodnoty přibližně 75 ms, při měření proudu s průměrováním 16 vzorků rychlostí 1,1 ms/vzorek pak přibližně 20 ms. U obvodu MAX3485 se použije čekací doba 100 µs (náběh obvodu a nabití blokovacího kondenzátoru 100 nF mezi VCC a GND), u budiče DRV8838 pak 3 ms, což zahrnuje nabití elektrolytického kondenzátoru 47 µF mezi VM a GND, keramického 100 nF mezi VCC a GND a především ustálení interní nábojové pumpy. U obvodu HX711 bude po zapnutí napájení potřeba čekat přibližně 500 ms — dobu ustálení analogové části převodníku a dokončení prvního převodu. Po této době už lze odečítat stabilní hodnoty; při zvoleném režimu 10 SPS trvá jedna konverze přibližně 100 ms. Kromě posledního zmíněného obvodu nebude inicializační doba zahrnuta do výpočtu denní spotřeby systému.

Před odpojením napájení VCC od jednotlivých částí systému je kvůli snížení spotřeby a leakage nutné vypnout periferie (I²C, UART, ADC) i jejich hodinový signál, který plýtvá energií, i když periferie právě nic nepřenáší. Po odpojení VCC je nutné všechny nepoužívané piny, včetně těch pro právě vypnuté periferie, přepnout do analogového režimu bez pull rezistoru (SCL, SDA, SCK, DT, PH, EN, DI, DE, RO, /RE). Stejný postup se použije i u pinů pro koncové spínače: jakmile dvířka dosáhnou koncové polohy, přepnou se do analogového režimu bez pull rezistorů, čímž se eliminuje jejich klidový odběr. Řídicí piny všech tranzistorových spínačů musí být nastaveny v digitálním režimu, aby se předešlo zvýšení odběru proudu.

Kvůli nízkopříkonové povaze systému bude nutné odpájet červenou Power LED diodu a softwarově odpojit všechny zelené User LED diody. Programátor ST-Link musí být za provozu hardwarově odpojen, proto je potřeba odpájet pájecí jumpery SB9, SB14, SB2 a SB3. U LoRa-E5 mini je navíc nutné přepnout piny PA2 a PA3, určené k ladění, do analogového režimu bez pull rezistoru. Pro programování lze pak programátor k deskám připojovat přes klasické Dupont kabely a u LoRa-E5 mini vrátit piny PA2 a PA3 do původního stavu. Zvláštní pozornost je třeba věnovat plovoucím pinům — nepoužívané piny musí být vždy v analogovém režimu bez pull rezistoru. Při nepoužívání rádia je nutné nastavit externí RF switch na logickou nulu; u řadičů bez rádia je zase potřeba v registrech napájení (PWR) aktivovat ultra-low-power režim (bit ULP) a vypnout fast wakeup.

&nbsp;

**Stavový automat pro algoritmus detekce snesených vajec**

&nbsp;

- Probuzení mikrořadičů a připojení napájení k potřebným částem systému
- Čekání 500 ms na dokončení inicializace
- Odebrání 32 vzorků rychlostí 10 SPS (3,2 s)
- Výpočet mediánu
- Výběr 16 vzorků s nejmenší odchylkou od mediánu
- Výpočet aritmetického průměru, aktuální hmotnosti (odečet referenční nulové hodnoty) a směrodatné odchylky
- Pokud hmotnost překročí 1 kg (v hnízdu je slepice), měření se zahodí
- Pokud odchylka překročí stanovený práh (pohyb slepice, vibrace), měření se zahodí
- Je-li měření stabilní, aktuální hmotnost se porovná s uloženou hodnotou
- Odpovídá-li rozdíl hmotnosti přibližné váze jednoho (60 g) nebo více vajec, změna se aritmeticky přičte k uložené hodnotě a spočítá se počet vajec v hnízdě
- Při hmotnosti menší než 25 g proběhne nanejvíš jednou denně kontrola driftu — zaznamenají-li se tři hned po sobě jdoucí stabilní měření, aktualizuje se referenční nulová hodnota
- Odeslání informace o počtu vajec v jednotlivých hnízdech
- Uspání mikrořadičů a odpojení napájení od používaných částí systému

&nbsp;

### Elektronika
Prototyp bude sestaven z modulů umístěných na nepájivém poli pomocí kolíkových lišt. Finální verze bude obsahovat jednu hlavní desku plošných spojů a několik (v tomto případě dvě, obecně například pět) vedlejších desek pro jednotlivá snášková hnízda. Na všech deskách budou moduly nahrazeny čipy a nezbytnými externími SMD součástkami.

Za akumulátorem bude do napájecí větve zařazena rychlá trubičková pojistka v pouzdře o jmenovité hodnotě 1 A. Tato hodnota poskytuje dostatečnou rezervu vůči běžnému provoznímu odběru systému, který při pohybu dvířek dosahuje pouhých 250 mA a toleruje krátkodobé proudové špičky do 550 mA při rozběhu nebo zaseknutí motoru. Taktéž je ale nastavena dostatečně nízko na to, aby při poruchovém stavu (zkrat na desce plošných spojů nebo zkrat vinutí motoru) spolehlivě přerušila obvod dříve, než by proud mohl cokoliv poškodit.

Samostatná přepěťová ochrana ani ochrana proti přepólování nebude do systému zařazena. Napětí akumulátoru je již průběžně softwarově hlídáno hlavní řídicí jednotkou, která při překročení bezpečné meze odpojuje solární panel pomocí MOSFET odpojovače, a napětí panelu je navíc přirozeně omezeno jeho konstrukčními parametry (naprázdno nepřekročí bezpečnou hodnotu pro napájecí obvody); dedikovaná přepěťová ochrana by tak přinášela jen marginální přínos za cenu vyšší klidové spotřeby a složitosti obvodu. Ochrana proti přepólování byla rovněž vynechána, protože veškeré napájecí spoje (akumulátor, solární panel) jsou realizovány pevnými šroubovými WAGO svorkovnicemi zapojovanými jednorázově při montáži, čímž je riziko náhodného přepólování v provozu prakticky vyloučeno; přidání sériové ochranné diody by navíc znamenalo trvalý úbytek napětí a zbytečnou ztrátu energie v celé napájecí větvi systému.

MOSFET odpojovač bude tvořen dvěma P-MOS tranzistory AO3401A zapojenými back-to-back (drainy proti sobě). Toto zapojení umožňuje úplné odpojení kladného napájecího napětí při zachování společné země celého systému a zamezuje zpětnému toku proudu z akumulátoru do panelu, způsobenému parazitními diodami P-MOS tranzistorů. Tyto tranzistory bude M řídit přes budicí logic-level N-MOS tranzistor BSS138 (sepnutí odpojovače probíhá nastavením logické jedničky na gate N-MOS), protože napětí 3,3 V není při napájení z 9V solárního panelu pro jejich rozepnutí dostatečné. Za M bude sériově zapojen 220 Ω rezistor pro ochranu GPIO pinu před krátkodobou proudovou špičkou při nabíjení/vybíjení gate kapacity. Mezi gate a společnou zem N-MOS tranzistoru bude paralelně zapojen 470 kΩ pull-down rezistor zabraňující vzniku nedefinovaného logického stavu nebo falešnému sepnutí. Drain bude připojen na gate obou P-MOS tranzistorů a přes 100 kΩ pull-up rezistor k 9V panelu. Source bude připojen ke společné zemi. Na P-MOS tranzistorech bude napětí U<sub>GS</sub> při sepnutém N-MOS tranzistoru vždy nižší než −4,5 V a při rozepnutém nulové. Z toho vyplývá, že R<sub>DSon</sub> bude maximálně 50-100 mΩ. Na N-MOS tranzistorech bude napětí U<sub>GS</sub> při sepnutí vždy vyšší než 2,5 V — R<sub>DSon</sub> bude maximálně 5 Ω. Co nejblíže za MOSFET odpojovačem budou v krabičce K paralelně mezi výstupní napájecí větev a společnou zem zapojeny dva kondenzátory: elektrolytický 47 µF / 25 V jako zásobárna energie a blokovací keramický 100 nF / 50 V.

&nbsp;

<img src="https://github.com/Hosty-04/ChickenCoop/blob/main/flowcharts/main_separator_schematic_white.png" alt="main_separator_schematic" height="0px">

&nbsp;

**Ověření funkce MOSFET oddělovače**

&nbsp;

$$
U_{G,P} = U_{max} \cdot \frac{R_{DSon}}{R_{pullup} + R_{DSon}} = 9\ \text{V} \cdot \frac{5\ \\Omega}{100\ \text{k}\Omega + 5\ \\Omega} = \mathbf{450\ \text{µV} \approx 0\ \text{V}}
$$

$$
U_{G,N} = I_{leak} \cdot R_{pulldown} = 100\ \text{nA} \cdot 470\ \text{k}\Omega = \mathbf{47\ \text{mV} < 0,8\ \text{V}}
$$

$$
U_{G,N} = U_r \cdot \frac{R_{pulldown}}{R_G + R_{pulldown}} = 3,3\ \text{V} \cdot \frac{470\ \text{k}\Omega}{220\ \\Omega + 470\ \text{k}\Omega} = \mathbf{3,29\ \text{V}}
$$

&nbsp;

kde:
- $U_{G,P}$ ... napětí na gate P-MOS tranzistoru
- $U_{max}$ ... maximální napětí panelu
- $R_{DSon}$ ... vnitřní odpor sepnutého tranzistoru
- $R_{pullup}$ ... pull-up rezistor pro P-MOS tranzistor
- $U_{G,N}$ ... napětí na gate N-MOS tranzistoru
- $I_{leak}$ ... svodový proud tekoucí přes gate
- $U_r$ ... napětí řadiče
- $R_{pulldown}$ ... pull-down rezistor pro N-MOS tranzistor
- $R_G$ ... rezistor připojený sériově za M

&nbsp;

I při větším R<sub>DSon</sub> dokáže spínač s N-MOS tranzistorem spolehlivě stáhnout gate P-MOS tranzistoru k zemi a tím ho otevřít. Slabší pulldown dokáže i přes tok svodového proudu přes gate udržet spínač s N-MOS tranzistorem rozepnutý; U<sub>th</sub> je u N-MOS tranzistoru 0,8-1,5 V. Pokles napětí na gate N-MOS tranzistoru při jeho spínání, kvůli ochrannému rezistoru před M, je zanedbatelný.

&nbsp;

Pro dosažení nízké klidové spotřeby bude větev zodpovědná za kontrolu vajec napájena přes tranzistorové spínače, senzor INA226 bude využívat režimu shutdown stejně jako driver DRV8838 (z modulu Pololu je nutné odpájet nSLEEP pullup rezistor) — většina elektroniky totiž pracuje jen krátkodobě, při měření, komunikaci nebo pohybu dvířek, a trvalé napájení všech obvodů by zbytečně odebíralo energii z akumulátoru. Přes hlavní z těchto spínačů bude M řídit napájení k hlavnímu MAX3485 a zároveň ke všem krabičkám Kx (rozepnutí logickou jedničkou). V každé krabičce Kx budou pak dva další spínače: první, ve výchozím stavu sepnutý (logická nula), bude přes Mx napájet místní MAX3485 a HX711; druhý, ve výchozím stavu rozepnutý (logická jednička), bude řídit napájení další krabičky Kx v řadě. Tyto spínače bude tvořit pouze jeden přímo řízený P-MOS tranzistor AO3401A, jehož source bude připojen na lineární LDO regulátor. Za M bude, ze stejného důvodu jako u MOSFET oddělovače, sériově zapojen 220 Ω rezistor a mezi gate tranzistoru a lineární LDO regulátor bude zapojen 100 kΩ pull-up rezistor zabraňující vzniku nedefinovaného logického stavu nebo falešnému sepnutí. Ve výchozím stavu sepnuté spínače budou mít místo pull-up rezistoru pull-down o stejné hodnotě. Typ N-MOS (low-side spínání) nebyl zvolen, protože by u komponent v krabičkách Kx hrozilo uzemnění přes cesty, které k tomu nejsou určeny. Mezi source a společnou zem spínačů bude připojen kondenzátor o parametrech 1 µF / 50 V — buffer proti odběru při sepnutí, ochrana sdílené 3,3V větve před propadem.

&nbsp;

<img src="https://github.com/Hosty-04/ChickenCoop/blob/main/flowcharts/peripheral_separator_schematic_white.png" alt="peripheral_separators_schematic" height="0px">

&nbsp;

**Funkce a parametry spínačů**

&nbsp;

$$
U_G = U_{nap} \cdot \frac{R_G}{R_{pullup} + R_G} = 3,3\ \text{V} \cdot \frac{220\ \\Omega}{100\ \text{k}\Omega + 220\ \\Omega} = \mathbf{7,24\ \text{mV} \approx 0\ \text{V}}
$$

$$
U_{ztr} = I_{max} \cdot R_{DSon} = 33\ \text{mA} \cdot 150\ \text{m}\Omega \approx \mathbf{5\ \text{mV}}
$$

$$
I_G = \frac{U_G - U_{plateau}}{R_G} = \frac{3,3\ \text{V} - 1,4\ \text{V}}{220\ \\Omega} = \mathbf{8,64\ \text{mA}}
$$

$$
t_s = \frac{Q_G}{I_G} = \frac{9,4\ \text{nC}}{8,64\ \text{mA}} \approx \mathbf{1,5\ \text{µs}}
$$

$$
t_n = 5 \cdot \tau = 5 \cdot R_{DSon} \cdot C = 5 \cdot 150\ \text{mΩ} \cdot 1\ \text{µF} \approx \mathbf{1\ \text{µs}}
$$

&nbsp;

kde:
- $U_G$ ... napětí na gate
- $U_{nap}$ ... napájecí napětí
- $R_{pullup}$ ... pull-up rezistor
- $U_{ztr}$ ... maximální možný úbytek napětí na spínači
- $I_{max}$ ... maximální proud spínačem pro 5 hnízd
- $I_G$ ... proud nabíjející gate
- $U_{plateau}$ ... Millerova plošina
- $R_G$ ... rezistor připojený sériově za M
- $t_s$ ... čas sepnutí a běžného rozepnutí
- $Q_G$ ... náboj gate
- $t_n$ ... čas nabití/vybití kondenzátoru
- $\tau$ ... časová konstanta
- $R_{DSon}$ ... maximální vnitřní odpor sepnutého tranzistoru
- $C$ ... kapacita kondenzátoru

&nbsp;

I s ochranným rezistorem před M dokáže spínač spolehlivě stáhnout gate tranzistoru k zemi a tím ho otevřít. U spínačů s pull-down rezistorem platí, že pokles napětí na gate při jejich rozpínání, kvůli tomuto rezistoru, je zanedbatelný. Spínače s nejen pull-down rezistorem mají stejný svodový proud tekoucí přes gate a silnější pull-down/pull-up rezistor něž u dříve zmíněného spínače s N-MOS tranzistorem; U<sub>th</sub> je -1,3 až -0,5 V — pull rezistory udrží spínače rozepnuté. Napětí U<sub>GS</sub> bude vždy buď nižší než -2,5 V nebo téměr nulové, tudíž R<sub>DSon</sub> bude maximálně 80-150 mΩ — nejvyšší možný úbytek napětí na spínači je minimální. Náboj gate Q<sub>g</sub> bude maximálně 7-9,4 nC — běžná doba změny stavu tranzistoru, ke které byla přičtena rezerva kvůli odporu pinu a hradla — přibližně 25 Ω, je stejně jako doba nabití kondenzátoru zanedbatelná.

&nbsp;

K solárnímu panelu bude připojen vysokoimpedanční napěťový dělič tvořený metalizovanými rezistory 1 MΩ a 470 kΩ s tolerancí 1 %, přičemž paralelně k rezistoru R2 (470 kΩ) bude zapojen blokovací keramický kondenzátor 100 nF / 50 V. Ten slouží jako zásobárna energie, kvůli vysoké výstupní impedanci děliče přes kterou se nabíjí interní vzorkovací kondenzátor uvnitř M, jehož malá kapacita by se tak nabíjela příliš pomalu na spolehlivé vzorkování. Dělič bude sloužit k monitorování napětí panelu; naměřené hodnoty se do M přenesou přes ADC pin v analogovém režimu a pro zvýšení přesnosti bude provedena kalibrace, výsledek pak bude aritmetickým průměrem 16 vzorků. Vysoká impedance děliče a mizivý leakage do M zajišťuje zanedbatelný vliv na pracovní bod a účinnost panelu. Velmi úsporný modul proudového a napěťového senzoru INA226 bude v krabičce K zapojen mezi akumulátor a vstup VM pro napájení motoru přes H-bridge; jednou z jeho funkcí bude s 16bitovým rozlišením a průměrováním 64 vzorků (1,1 ms/vzorek) monitorovat napětí akumulátoru.

&nbsp;

**Napěťový rozsah děliče**

&nbsp;

$$
U_{r} = U_{max} \cdot \frac{R_2}{R_1 + R_2} = 9\ \text{V} \cdot \frac{470\ \text{k}\Omega}{1\ \text{M}\Omega + 470\ \text{k}\Omega} = \mathbf{2,88\ \text{V} < 3,3\ \text{V}}
$$

&nbsp;

kde:
- $U_r$ ... maximální napětí na řadiči
- $U_{max}$ ... maximální napětí panelu
- $R_1$ ... první rezistor děliče
- $R_2$ ... druhý rezistor děliče

&nbsp;

I při maximálním napětí na solárním panelu nepřesáhne napětí na ADC pinu napájecí napětí M. Napětí na ADC pinu se bude tudíž pohybovat v bezpečných mezích pro M.

&nbsp;

Na základě údajů z napěťového senzoru a napěťového děliče bude M přes sběrnici I²C, respektive přes ADC pin, vyhodnocovat stav akumulátoru a solárního panelu. Dostane-li se napětí akumulátoru nad limitní hodnotu (v létě 7,2 V, na jaře a na podzim 7,3 V, v zimě 7,5 V), M solární panel odpojí. Pokud napětí akumulátoru následně klesne o 250 mV po dobu 30 minut (tři po sobě jdoucí měření), M panel znovu připojí. Při kritickém vybití akumulátoru, kdy jeho napětí klesne na 5,75 V, přejde M do kritického režimu, ve kterém bude už jen kontrolovat napětí panelu a akumulátoru; k obnovení provozu dojde po dosažení 6,1 V. Během nedostatečného slunečního svitu nebo v noci, kdy je napětí panelu nižší než napětí akumulátoru, musí M zamezit vzniku zpětného proudu směrem do panelu jeho odpojením; kvůli úbytku napětí na MOSFET oddělovači (max 120 mV) a nepřesnosti měření bude hladina pro odpojení, respektive opětovné připojení panelu zvýšena o 250 mV.

&nbsp;

<img src="https://github.com/Hosty-04/ChickenCoop/blob/main/flowcharts/separator_flowchart_white.png" alt="separator_flowchart" height="1080px">

&nbsp;

Další funkce proudového senzoru bude s 16bitovým rozlišením a průměrováním 16 vzorků rychlostí 1,1 ms/vzorek (dostatečná přesnost pro detekci překročení prahové hodnoty) monitorovat proud při pohybu dvířek; zvýšení proudu nad mezní hodnotu po dobu 150 ms bude signalizovat překážku v cestě (typicky slepici) nebo zaseknutí dvířek. V takovém případě se M na 250 ms zastaví, pokusí se obrátit směr otáčení motoru a vrátit dvířka do původní polohy, poté se uspí a po 10 minutách pokus zopakuje. Nepomůže-li ani zpětný chod, systém odešle zprávu o poruše dvířek a do uživatelského pokynu s nimi nebude manipulovat. Zpráva o poruše bude odeslána i při nepřetržitém běhu motoru, po dobu vyšší než 25 s — potřebná doba pro změnu stavu dvířek + rezerva. Krátkodobou proudovou špičku při rozběhu motoru, trvající asi 250 ms, je nutné ignorovat.

&nbsp;

<img src="https://github.com/Hosty-04/ChickenCoop/blob/main/flowcharts/motor_flowchart_white.png" alt="motor_flowchart" height="0px">

&nbsp;

**Mezní proud motoru**

&nbsp;

$$
I_m = I_{m,p} \cdot \frac{U_m}{U_{aku}} = 450\ \text{mA} \cdot \frac{6\ \text{V}}{6,8\ \text{V}} = \mathbf{397\ \text{mA}}
$$

&nbsp;

kde:
- $I_m$ ... mezní proud
- $I_{m,p}$ ... mezní proud při přímém řízení bez PWM modulace
- $U_m$ ... napětí motoru
- $U_{aku}$ ... aktuální napětí akumulátoru (zde průměrné)

&nbsp;

Velmi úsporný modul H-bridge Pololu DRV8838 bude přes PWM modulaci s frekvencí 20 kHz regulovat napětí na motoru, aby efektivní hodnota odpovídala 6 V bez ohledu na aktuální napětí akumulátoru. Tato frekvence byla zvolena s ohledem na tři podmínky. Vůči časové konstantě vinutí motoru (u malých kartáčových motorů s převodovkou typicky v řádu stovek µs) je perioda PWM (50 µs) dostatečně krátká, aby proud vinutím zůstal v kontinuálním režimu a nestihl mezi jednotlivými pulzy poklesnout k nule — motor tak pracuje s vyhlazeným stejnosměrným napětím místo trhavých pulzů, což nezvyšuje jeho mechanické namáhání. Vůči měření proudu modulem INA226 (17,6 ms) proběhne při této frekvenci přes 350 period PWM, takže výsledek zůstává spolehlivě zprůměrován nezávisle na tom, v jaké fázi PWM cyklu zrovna vzorkování proběhlo. Vůči elektrolytickému kondenzátoru leží 20 kHz blízko horní hranice jeho rozsahu, kde má nejnižší ESR a snese nejvyšší ripple proud bez nadměrného zahřívání. Při 20 kHz je tento limit přibližně 152 mA — bezpečně pokrývá typický proud motoru (100 mA); krátkodobé špičky při zaseknutí (550 mA po dobu 150 ms) tento limit sice převyšují, ale díky tepelné setrvačnosti kondenzátoru a krátkému trvání nepředstavují riziko pro jeho životnost. Zvolená frekvence zároveň zůstává s velkou rezervou pod maximální PWM frekvencí driveru DRV8838 (250 kHz) i mimo slyšitelné pásmo.

H-bridge bude vybaven elektrolytickým kondenzátorem s nízkým ESR (47 µF / 25 V) zapojeným mezi piny VM a GND, který slouží jako zásobárna energie pro rychlé proudové nároky motoru a zároveň rychle potlačí indukční napěťové špičky vznikající při vypnutí motoru. Protože elektrolytický kondenzátor má kvůli své konstrukci nezanedbatelnou parazitní indukčnost (ESL) a nad určitou frekvencí (řádově stovky kHz a výš, tedy u vyšších harmonických PWM hran) přestává být účinným filtrem, bude napájecí větev motoru doplněna o π-článek (C-L-C) tvořený dvěma blokovacími keramickými kondenzátory 1 µF / 50 V a feritovou korálkou o impedanci 120 Ω při 100 MHz zapojenou mezi nimi v sérii do přívodu VM. Tato kombinace zajistí, že vysokofrekvenční složky PWM, které již neúčinně tlumí pomalý elektrolytický kondenzátor kvůli své ESL, budou lokálně svedeny do země na obou stranách korálky, zatímco korálka sama zabrání jejich šíření podél napájecího vedení směrem k citlivé analogové elektronice (INA226, HX711). Vzhledem k nízkému R<sub>DC</sub> korálky (30 mΩ) zůstane úbytek napětí na ní i při maximálním proudu motoru (550 mA) zanedbatelný (16,5 mV), a proudová rezerva korálky (3 A) zajišťuje, že feritové jádro nebude v žádném provozním stavu saturovat.

Co se týče samotného motoru, ten bude odrušen keramickým kondenzátorem 100 nF zapojeným přímo mezi jeho vývody a dvěma keramickými kondenzátory 47 nF mezi jednotlivými vývody a kostrou motoru (Faradayova klec); všechny kondenzátory budou dimenzovány na napětí 50 V. Toto odrušení je nezbytné pro omezení jiskření kartáčků a potlačení vysokofrekvenčního elektromagnetického rušení. H-bridge i motor budou v krabičce K umístěny co nejdále od ostatní elektroniky. U obou koncových spínačů sloužících k určení polohy dvířek bude kontakt COM připojen k lineárnímu LDO regulátoru a kontakt NO k M spolu s interním pull-down rezistorem 40 kΩ — ten je potřeba kvůli kabelům, které se chovají jako anténa. Tyto spínače budou umístěny tak, aby mohly jejich kabely vést co nejdále od silové části a cest krabičky K.

Měření hmotnosti snáškového hnízda bude zprostředkovávat tenzometr se zanedbatelnou nelinearitou (vůči rozeznávání slepičích vajec) a hysterezí, díky které zůstane kalibrace váhy dlouhodobě stabilní i při neustálém zatěžování. Kabel od tenzometru bude připojen k modulu A/D převodníku HX711 umístěnému v krabičce Kx. Modul zesílí velmi nízké výstupní napětí tenzometru, pohybující se v řádu jednotek milivoltů. Stínění kabelu bude na desce plošných spojů připojeno ke společné zemi za účelem odvodu šumu. Převodník bude připojen k Mx. Ten bude pro komunikaci s M přes datový kabel typu UTP využívat sběrnici RS485. První kroucený pár povede napájení pro Mx a spínače s P-MOS tranzistorem s pull-up rezistorem, oba vodiče zapojené paralelně; stejně tak druhý pár, který přes tranzistorový spínač povede napájení ke zbylé elektronice v krabičkách Kx. Třetí pár, opět přes dva paralelní vodiče, propojí společnou zem. Čtvrtý pár přenese data prostřednictvím čipu MAX3485, který slouží jako transceiver sběrnice RS485 — jeden čip bude před M, druhý před Mx. Tento čip je napájen 3,3 V a vytváří diferenciální signál na dvou linkách, čímž zvyšuje odolnost komunikace proti elektromagnetickému rušení. Protože jde o čip bez vývodů pro nepájivé pole, bude pro prototyp potřeba adaptér SO8 na DIP8 a kolíkové lišty. Paralelně k vývodům VCC a GND bude připojen blokovací keramický kondenzátor 100 nF / 50 V.

Na deskách plošných spojů musí být všechny součástky v jednotlivých krabičkách co nejblíže u sebe a kondenzátory musí být zapojeny co nejblíže k příslušným pinům, silové části a cesty však musí zůstat oddělené od ostatní elektroniky. Souvislou zemní plochu bude tvořit záporný pól solárního panelu a akumulátoru.

&nbsp;

### Mechanika
Hlavní část systému bude umístěna na vnější stěně kurníku, splňující požadavky na umístění solárního panelu popsané v kapitole Napájení. Toto řešení zjednoduší montáž a zároveň z velké části eliminuje vliv amoniaku ze slepičího trusu na elektroniku.

Solární panel bude uchycen do rámečku vytištěného z materiálu PETG, jehož vnější rozměr (360 × 240 mm) přesahuje rozměr panelu (340 × 220 mm) o 10 mm po každé straně. Kapsa pro panel bude hluboká 4 mm, tedy o 1 mm více než tloušťka panelu, aby po vložení mírně klesl pod úroveň okraje rámečku a nedocházelo k zadržování vody na jeho povrchu. Dno kapsy bude opatřeno výřezem o rozměru 320 × 200 mm, odpovídajícím aktivní ploše panelu, takže rámeček bude mít tvar pasparty a nebude stínit dopadající sluneční záření. Po vložení panelu do kapsy bude spára mezi jeho okrajem a stěnou rámečku zatmelena venkovním, UV odolným silikonovým tmelem, čímž vznikne vodotěsné a zároveň mechanicky pevné spojení bez nutnosti vrtat do samotného panelu. Rámeček bude přes čtyři otvory o průměru 5 mm v rozích, umístěné mimo aktivní plochu panelu, přišroubován vruty do zdiva 6 × 80 mm přes plastové hmoždinky 8 mm přímo ke stěně kurníku.

Konstrukce bude obsahovat jednu krabičku pro akumulátor o tloušťce stěny 2,4 mm a jednu krabičku (K) o tloušťce stěny 1,6 mm určenou pro mechaniku a elektroniku dvířek a hlavní mikrořadič. Pro každé snáškové hnízdo je dále vyhrazena samostatná krabička (Kx) o stejné tloušťce, obsahující potřebnou elektroniku a mikrořadič. Všechny krabičky budou vytištěny na 3D tiskárně z materiálu PETG, zvoleného pro jeho vysokou houževnatost a teplotní odolnost. Zadní stěna krabiček bude ve svislém směru prodloužena a bude mít tloušťku 3,5 mm; v každém rohu této stěny vzniknou otvory pro vruty 4 × 40 mm, které umožní upevnění krabiček k betonové stěně kurníku pomocí hmoždinek o průměru 6 mm. Víko bude vytištěno samostatně s přesahem 2 mm; uprostřed jeho levé hrany bude dutá trubička o průměru otvoru 2 mm. Stejná trubička bude u všech krabiček na obou koncích vnější horní hrany levé stěny. Po vytištění bude všemi třemi trubičkami protažen filament o průměru 1,75 mm, jehož konce se krátce nahřejí zapalovačem a roztepou do podoby hlaviček. Tím vznikne jednoduchý pant pro uchycení víka.

Na krabičkách K a Kx budou u víka vytvořeny nízké drážky ve tvaru písmene U s mírně zaoblenými rohy, sloužící k protažení kabelů. Pro kabel o průřezu 1,5 mm² budou mít šířku 4,5 mm, hloubku 3 mm a zaoblení dna i hran 1 mm; pro kabel o průřezu 0,5 mm² šířku 3,5 mm, hloubku 2 mm a zaoblení dna i hran 0,75 mm; pro datový kabel pak šířku 6 mm, hloubku 3,5 mm a stejné zaoblení jako u prvního kabelu. U krabičky pro akumulátor budou zespodu dva kruhové otvory o průměru 5 mm určené pro průchod napájecích kabelů a dva větrací otvory o průměru 4 mm, umístěné v horní a spodní části tak, aby mohl uvnitř cirkulovat vzduch. Krabička Kx bude mít zespodu jeden otvor o průměru 6 mm pro průchod kabelu od tenzometru, krabička K pak otvor o průměru 3 mm pro lanko.

Na betonovou stěnu budou pomocí vrutů a hmoždinek připevněny svislé dřevěné hranolky, k jejichž bočním stěnám budou vruty upevněny hliníkové U-profily s vnitřním rozměrem 10 mm. U-profily zajistí přesné vedení dvířek při minimálním tření. V těchto profilech se budou ve svislém směru pohybovat bílá pěněná PVC dvířka o rozměrech 24 × 35 cm a tloušťce 8 mm (pozor na výrobní tolerance). Hmotnost těchto dvířek je 350 g. Při vrtání otvorů do PVC je nutné použít nízké otáčky a nevyvíjet nadměrný přítlak. Pěněné PVC bylo zvoleno pro nízkou hmotnost, odolnost proti vlhkosti a snadnou obrobitelnost.

V horní i dolní části hranolku budou ve svislé ose zapuštěny dva pákové koncové mikrospínače sledující polohu dvířek. Záhlubení široká 6,5 mm a vysoká 20 mm povedou skrz hranolek a 10mm širokou hliníkovou drážku; sloužit budou k vývodu kontaktů. Spínače budou upevněny vruty o průměru 2 mm. Tohle provedení představuje jednoduché řešení určování krajních poloh dvířek.

Na horní části přední strany dvířek bude umístěno závěsné očko M5 z nerezové oceli A2, upevněné podložkou a samojistnou maticí stejného typu; očko bude procházet otvorem odpovídajícího průměru jako jeho metrický závit. Očkem bude protaženo syntetické lanko (zednická stavební šňůra) o průměru 2 mm, zajištěné dračí smyčkou zakápnutou vteřinovým lepidlem. Syntetická šňůra nekoroduje, má nízkou průtažnost a je odolná vůči vlhkosti.

Lanko povede kolmo vzhůru do krabičky K, kde bude navíjeno na špulku vytištěnou z materiálu PETG (šířka 2 cm, vnější průměr 2,5 cm). Otvorem o průměru 3 mm na okraji špulky, dále od motoru, bude lanko provlečeno a upevněno osmičkovým uzlem, rovněž zakápnutým vteřinovým lepidlem; uzel bude zapuštěn 8 mm hluboko do záhlubení o průměru 10 mm, s vývodem lanka na okraji záhlubení směrem k okraji špulky.

Ta bude opatřena otvorem s D-profilem (průměr 3 mm, hloubka 10 mm) a pomocí stavěcího šroubu (červíku) M5 z nerezové oceli A4 (délka 6 mm) a mosazné závitové vložky M5 (délka 5,8 mm, průměr 7,1 mm) bude upevněna na hřídeli nízkootáčkového (20 rpm) stejnosměrného motoru s kovovou převodovkou (6 V), ze strany, kde vstupuje lanko.

&nbsp;

**Tah motoru pro různé kroutící momenty**

&nbsp;

$$
M = F \cdot r
$$

$$
F_j = \frac{M_j}{r_s} = \frac{5\ \text{N·cm}}{12,5\ \text{mm}} = 4\ \text{N} \approx \mathbf{400\ \text{g tahu}}
$$

$$
F_d = \frac{M_d}{r_s} = \frac{4\ \text{N·cm}}{12,5\ \text{mm}} = 3,2\ \text{N} \approx \mathbf{320\ \text{g tahu}}
$$

$$
F_k = \frac{M_k}{r_s} = \frac{25\ \text{N·cm}}{12,5\ \text{mm}} = 20\ \text{N} \approx \mathbf{2\ \text{kg tahu}}
$$

&nbsp;

kde:

- $F_j$ ... síla při jmenovitém krouticím momentu
- $F_d$ ... síla při doporučeném dlouhodobém trvalém zatížení
- $F_k$ ... síla při maximálním krátkodobém zatížení
- $M_j$ ... jmenovitý krouticí moment
- $M_d$ ... krouticí moment při doporučeném dlouhodobém trvalém zatížení
- $M_k$ ... krouticí moment při maximálním krátkodobém zatížení
- $r_s$ ... poloměr špulky

&nbsp;

Dvířka o hmotnosti 350 g představují mírně vyšší zátěž, tudíž lze očekávat pokles otáček motoru na 15-17 za minutu. Je počítáno i s rezervou pro případ mírného drhnutí dvířek v drážkách. Nízkootáčkový motor spolu s větším vnějším průměrem špulky byl zvolen pro zvýšení síly motoru a snížení rizika přetrhnutí lanka nebo vykolejení dvířek.

Otvor pro mosaznou závitovou vložku bude mít průměr 8 mm a v posledních 6,5 mm směrem ke hřídeli se zúží na 6,5 mm. Vložka se do otvoru zavede pomocí 16mm šroubu M5; jakmile narazí na zúženou část otvoru, hlava šroubu se nahřeje mikropájkou, plast se v místě zúžení roztaví a umožní zalisování vložky. Po odejmutí pájky plast opět ztuhne a vložku pevně ukotví.

Vedlejší část systému budou tvořit spádovaná snášková hnízda s košíkem, orientovaná příčně ke stěně. Celá konstrukce musí být od stěny vzdálena natolik, aby se jí nikde nedotýkala a zároveň za ní zůstal prostor pro vedení kabelu; současně je nutné zabránit přístupu slepic a hlodavců do tohoto prostoru.

Pod hnízda bude vruty připevněna voděodolná OSB 3 deska o rozměrech 37 × 28 cm a tloušťce 12 mm. Další deska (35 × 26 cm, stejná tloušťka) bude tvořit základovou desku a bude připevněna k podlaze kurníku; třetí deska stejné tloušťky, o rozměrech 40 × 50 cm, poslouží jako zadní stěna. Mezi horní a spodní deskou bude u stěny umístěn tenzometr o tloušťce 12,7 mm, orientovaný kratší stranou s vodiči směrem ke stěně. Tenzometr váží s přesností 2 g, což pro vejce o hmotnosti 50–70 g plně postačuje, a je dimenzován na zatížení 20 kg — maximální předpokládané zatížení (dvě desky, hnízdo, dvě slepice a deset vajec) přitom činí přibližně 10 kg. Volný i pevný konec tenzometru bude upevněn dvěma zápustnými imbusovými šrouby M5 z nerezové oceli A2, procházejícími vážicí respektive základovou deskou skrz otvory o průměru 4,5 mm a zapuštěnými do záhlubení o průměru 10 mm a hloubce 3 mm; hlava šroubu bude u vážicí desky zalita akrylovým tmelem. Použité šrouby budou mít délku 25 mm. Mezi deskami a závity tenzometru budou umístěny distanční matice M6 z nerezové oceli A2 o tloušťce 4 mm.

Optická závora není pro počítání vajec v kurníku vhodná mimo jiné kvůli prachu, peří, podestýlce a zvědavým slepicím, jejichž přítomnost by mohla přerušit laserový paprsek a být chybně vyhodnocena jako propadnuté vejce. Z tohoto důvodu byl pro detekci vajec zvolen tenzometr.

Ze zbytku OSB desky budou vyrobeny ochranné lišty, přišroubované ke spodní straně vážicí desky. Lišty budou vysoké 15 mm, protože vzdálenost mezi deskami činí 20,7 mm a celá horní konstrukce musí být podepřena pouze tenzometrem — zároveň musí zůstat zachována vůle alespoň 3 mm. Do zadní lišty bude vyvrtán otvor, kterým se protáhne již prodloužený kabel od tenzometru; kabel nesmí být mechanicky namáhán tahem. Dále povede vzhůru po stěně do krabičky Kx, odkud bude z boku vyveden sdělovací kabel, vedoucí vysoko po stěně až do krabičky K. Uprostřed konstrukce, naproti košíku, bude vytvořen stavitelný doraz pomocí zápustného imbusového šroubu M5 z nerezové oceli A2 (délka 30 mm), procházejícího spodní deskou skrz otvor o průměru 4,5 mm a zapuštěného do záhlubení o průměru 10 mm a hloubce 6 mm. Doraz bude tvořit mosazná kloboučková matice M5 (délka 10 mm, závit 7,5 mm), zvolená kvůli omezení ulpívání nečistot. Optimální vůle dorazu je 0,8 mm, což odpovídá jednomu otočení šroubu M5 nebo přibližně tloušťce běžné platební karty.

&nbsp;

## Nákup

&nbsp;

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
https://www.gme.cz/v/1501395/wago-233-506-svorkovnice-6pol-roztec-254mm-6a-160v-vstup-45-pruzina  
https://www.gme.cz/v/1513875/wago-221-2411-svorka-bezsroubova-2pol-vodic-do-4mm2 (4 ks)  

**Konektor**  
https://www.hadex.cz/p/d626-zdirka-rj45-do-dps-8p8c (4 ks)  

**Fotovoltaický panel**  
https://www.dexhal.cz/fotovoltaicky-panel-9v-1110ma-10w-solarni-clanek_z2900/  

**N-MOS**  


**P-MOS**  


**Akumulátor**  
https://www.levne-baterky.cz/Green-Cell-AGM-Baterie-6V-4Ah-d5516.htm  

**Pojistka**  
https://www.gme.cz/v/1511166/schurter-spf-5x20-h-1a-250v-pojistka-trubickova-s-keramikou (5 ks)  
https://www.gme.cz/v/1512336/stelvio-chapponi-ptf15b-poistkove-puzdro-do-dps (2 ks)  

**Lineární LDO regulátor**  
https://www.elektro-hofman.cz/stabilizator-pevneho-napeti-mcp1702-3302et-3-3v-0-25a-low-drop-to92/ (2 ks)  

**Master**  
https://botland.cz/lora-lorawan/19267-lora-e5-mini-stm32wle5jc-modul-lorawan-868-915-mhz-seeedstudio-113990939-5904422369026.html  

**Slave**  
https://botland.cz/stm32-nucleo/18799-stm32-nucleo-l031k6-s-stm32l031k6-mcu-kompatibilni-s-arduino-nano-5904422364878.html (2 ks)  

**Header**  
https://www.laskakit.cz/dupont-40pin-2-54-mm-pinovy-pas/ (4 ks)  

**Proudový a napěťový senzor**  
https://www.laskakit.cz/en/laskakit-ina226-sensor-pro-mereni-napeti--proudu-a-vykonu/  

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
https://www.gme.cz/v/1486151/hitano-ck-1u-50v-x7r-rm508-10-keramicky-kondenzator (10 ks)  
https://www.gme.cz/v/1489496/hitano-ce-47u-25vit-hit-exr-5x11-rm2-bulk-elektrolyticky-kondenzator  

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

**Spojovací materiál**  
https://www.prumex.cz/sroub-zapustny-inbus-din-7991-m5x30-nerez-a2/ (4 ks)  
https://www.prumex.cz/sroub-zapustny-inbus-din-7991-m5x25-nerez-a2/ (10 ks)  
https://www.prumex.cz/sroub-sestihranny-cely-zavit-din-933-m5x16-nerez-a2-70/ (2 ks)  
https://www.prumex.cz/matice-samojistna-nizka-din-985-m5-nerez-a2/ (4 ks)  
https://www.prumex.cz/matice-din-934-m5-nerez-a2/ (10 ks)  
https://www.prumex.cz/podlozka-plocha-din-125a-m5-nerezova-ocel-a2-5-3x10x1/ (4 ks)  

&nbsp;

### Cenový přehled

&nbsp;

| Obchod | Cena |
|:---|:---:|
| GM electronic | 760 Kč |
| Alza | 150 Kč |
| Wilmann Elektro | 175 Kč |
| Hadex | 120 Kč |
| Hornbach | 710 Kč |
| Dexhal | 590 Kč |
| Levne-Baterky | 270 Kč |
| Hofman Elektro | 125 Kč |
| LaskaKit | 820 Kč |
| Dratek | 200 Kč |
| Botland | 1 880 Kč |
| Zenit | 120 Kč |
| Lihneme | 1 150 Kč |
| SOS elektro | 450 Kč |
| Prumex | 200 Kč |
| Rezerva | 500 Kč |
| **Celkem** | **8 220 Kč** |

&nbsp;

*Poznámka: Cena je orientační a je do ní započtena i doprava. Položky z Hornbachu budou zakoupeny osobně.*
