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
Pro připojení solárního panelu a akumulátoru bude využito speciálního solárního kabelu typu licna o průřezu 4 mm² a hřebíkových příchytek o průměru 6 mm. Tyto kabely budou v K přes prodlužovací WAGO svorky přetransformovány na měděnou ohebnou licnu o průřezu 1,5 mm². Ta bude pomocí 4 pinové WAGO svorkovnice o rozteči 5,08 mm připojena k desce plošných spojů a odtamtud povede do akumulátoru. Mikrospínače budou připojeny k desce plošných spojů přes měděnou ohebnou licnu o průřezu 0,5 mm² pomocí další 4 pinové WAGO svorkovnice s roztečí 5,08 mm. U prototypu bude využíváno rovnou měděné ohebné licny o průřezu 1,5 mm² a drátků do nepájivého pole s průřezem 0,5 mm². Pro kladný pól bude použit červený vodič a pro záporný pól černý vodič.

Pro datovou komunikaci byl zvolen kabel UTP CAT5e typu licna, který bude ke stěně upevněn pomocí příchytek o průměru 5 mm a přes konektory RJ45 vložen do krabiček. Prodloužení vodičů tenzometru zajistí přes prodlužovací WAGO svorky stíněný kabel LiYCY 4 × 0,25 mm² typu licna. Jelikož jsou vodiče od tenzometru o průřezu 0,14 mm² pro WAGO svorky příliš tenké (povolený průřez je 0,2 - 4 mm²), tak se u odizolovaného konce přehnou napůl na průřez 0,28 mm². K desce plošných spojů bude potom tento kabel spolu se stíněním připojen pomocí 5 pinové WAGO svorkovnice o rozteči 2,5 mm.

### Napájení
Solární panel o parametrech 6 V / 10 Wp bude svisle připevněný na stěnu pod malou stříšku a orientovaný směrem na jih, případně na východ nebo západ (v mém případě bude panel směřovat na jihozápad). Tím bude zajištěno, že panel bude co nejlépe využívat dostupnou sluneční energii. Vertikální montáž omezuje usazování sněhu a nečistot.

Společně s bezúdržbovým olověným AGM akumulátorem o parametrech 6 V / 7 Ah, umístěným venku ve stínu asi 25 cm pod stříškou (ideálně na severní straně), budou zajišťovat napájení celého systému. Akumulátor LiFePO4 je sice v mnoha ohledech lepší, ale nesmí se nabíjet v zimě pod 0 °C a potřebuje složitější a energeticky náročnější solární regulátor napájení MPPT. Silová část bude pracovat s napětím 6 V, zatímco veškerá elektronika bude napájena napětím 3,3 V.

Snížení napětí z 6 V na 3,3 V bude realizováno pomocí nízkopříkonového LDO regulátoru MCP1700. Nízký klidový proud významně prodlužuje provoz z akumulátoru. Tento stabilizátor bude zapojený k baterii a na jeho vstupu bude připojen blokovací keramický kondenzátor o parametrech 1 µF / 50 V. Stejný kondenzátor bude připojen i na výstupu, ale v tomto případě z důvodu vyhlazení napětí. 

Použití spínaného buck měniče není vhodné z důvodu horší dostupnosti nízkopříkonových variant a velmi malého odběru systému po většinu dne. Jeho vyšší účinnost by se projevila pouze po dobu několika minut denně, zatímco po zbytek dne by kvůli vlastní spotřebě dosahoval nižší celkové účinnosti než jednoduchý lineární stabilizátor.


*Poznámka: Zimní bilance počítá s poklesem efektivity solárního panelu o 75 % a kapacity akumulátoru o 25 % oproti letním hodnotám*

### Řízení
Hlavní řídicí jednotkou systému bude mikrořadič STM32WLE5JC LoRa-E5 mini, taktovaný na 2 MHz. Ten bude disponovat integrovaným LoRa modulem, komunikujícím přes LoRaWAN stack (12 dBm, SF9, BW 125 kHz, CR 4 / 5, Class A neboli RX1 + RX2 a délka příjmového okna automaticky podle driftu krystalu). Pro komunikaci bude použit LDO režim namísto efektivnějšího SMPS režimu. Tato volba sice zdvojnásobí spotřebu při komunikaci, ale ta bude stále zanedbatelná. LDO režim je navíc jednodušší volbou. Kvůli zamezení rušení při komunikaci bude řadič umístěn na opačné straně než motor. LoRa modul umožní na rozdíl od Wi-Fi komunikaci na velké vzdálenosti při nízké spotřebě energie. NB-IoT má taktéž vyšší spotřebu. U každého snáškového hnízda bude umístěn další mikrořadič STM32 NUCLEO-L031K6 (2 MHz). Ten má integrovaný programátor, který bude využit i pro hlavní řadič.

Firmware bude vyvíjen v prostředí STM32CubeIDE. Součástí firmwaru hlavní řídicí jednotky budou astronomické hodiny, podle kterých se budou automaticky otevírat a zavírat dvířka. Použití časovače nebylo zvoleno z důvodu proměnlivé délky dne. Světelný senzor by mohl způsobovat chybné sepnutí při zatažené obloze (déšť nebo bouřka) nebo ve večerních hodinách vlivem pouličního osvětlení či světlometů automobilů. Roční období bude řadič schopen zjistit podle RTC.

Použité mikrořadiče mají režimy, které umožňují výrazně snížit spotřebu energie. Hlavní řídicí jednotka se bude společně s nezbytnými částmi systému probouzet každých 10 minut za účelem kontroly stavu solárního panelu a akumulátoru (stovky milisekund). Dále se bude společně s ostatními řídicími jednotkami a dalšími potřebnými částmi systému probouzet 24× denně, tedy každou hodinu, pro sekvenční kontrolu stavu vajec s postupným buzením každého hnízda (24 × 5 hnízd × 5 s na jedno hnízdo = cca 10 minut). Po sběru dat ze všech hnízd se všechno najednou odešle. Nakonec se bude probouzet ráno a večer, opět pouze s nezbytnými částmi systému, za účelem otevření a zavření dvířek (cca 2 × 20 s). Před zahájením pohybu dvířek budou piny koncových spínačů nastaveny jako digitální vstupy s interním pull-up rezistorem. Po dosažení koncové polohy se odešle informace o poloze dvířek a piny budou přepnuty na analogové vstupy, čímž bude eliminován jejich klidový odběr.

Pozor na plovoucí piny! Pokud piny nejsou používány, tak se musí kvůli spotřebě nastavit do analogového režimu nebo se u nich musí aktivovat interní pullup či pulldown rezistor. Kvůli spotřebě je také potřeba odpojit programátor ST-Link, před úsporným režimem nastavit externí RF switch na logickou nulu a použít LP rádio nejen softwarově, ale i LP radio výstup. Před odpojením napájení VCC různých částí systému je potřeba piny (SCK, DT, PH, EN, DI, DE, !RE, RO) přepsat na logickou nulu, pro zamezení napájení přes piny tomu neurčené (leakage current). Dále budou vypnuty nepoužívané periferie (I²C, UART, ADC), protože i když se přes ně nic neposílá, tak běží hodinový signál, který plýtvá energii. Po odpojení VCC je potom třeba nastavit všechny piny, včetně těch pro teď už vypnuté periferie, do analogového režimu bez pull rezistorů. LoRa anténa bude moci vysílat až po vypnutí všech systémů, kvůli jejímu vyššímu odběru proudu a ochraně před rušením. Po každém vysílání bude mít potom možnost příjmout data, což umožní uživatelské ovládání.

V domě bude umístěna LoRaWAN gateway, která bude přes WiFi plnit funkci internetové brány. Veškerá data přijatá přes gateway budou následně odeslána do cloudu (TTN) a potom přes MQTT na backend (NodeJS). Ten data uloží do databáze (InfluxDB) a zobrazí je na frontendu. Při odesílání dat z gateway je proces obrácený.

Data budou z kurníku odesílána pomocí 3+ bajtů. První bajt bude obsahovat 1 bit pro indikaci zapnutí / vypnutí systému, další pro oznámení kritického stavu (nízké napětí na akumulátoru) a 5 bitů ponese informaci o samotném napětí na akumulátoru (5,4 - 8 V, krok cca 0,1 V) Druhý bajt bude obsahovat 2 bity pro indikaci stavu dvířek (ON / OFF / zaseknuto) a 6 bitů ponese informaci o napětí solárního panelu (0 - 6 V, krok cca 0,1 V). Dalších x bajtů bude po 4 bitech alokováno pro přenos počtu vajec v x snáškových hnízdech (0-10 vajec na hnízdo). Kurník bude data přenášet každou hodinu po kontrole stavu hnízd a k tomu 2× denně, při změně stavu dvířek, odešle pouze první polovinu prvního bajtu a celý poslední bajt. Manuální ovládání bude využívat pouze 1 bajtu, jeden bit bude využit pro vypnutí / zapnutí systému, další bit pro vypnutí / zapnutí kritického režimu a poslední bit pro ovládání dvířek (ON / OFF).

Pro přenos dat mezi M a Mx bude využit protokol LPUART, který nevyužívá hodinový signál a vyznačuje se nízkou spotřebou energie. Přenosová rychlost bude 9600 Bd, kvůli minimalizaci odrazů. Na aplikační vrstvě bude použit komunikační protokol Modbus RTU a knihovna ModbusRTU-Slave. Protokol Modbus RTU vytvoří datový rámec obsahující adresu jednotky slave, přenášená data a kontrolní součet CRC pro detekci chyb při přenosu. LPUART, hardware v řadiči, následně převede jednotlivé bajty na sériový datový tok, doplní start a stop bity a zajistí jejich přenos po sběrnici. Na straně přijímače proběhne opačný proces.

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

### Elektronika
Prototyp bude sestaven z modulů umístěných na nepájivém poli pomocí kolíkových lišt a šroubovacích svorek, které budou obsahovat kovový plíšek pro ochranu licny. Finální verze bude obsahovat jednu hlavní desku plošných spojů a až 5 vedlejších desek pro jednotlivá snášková hnízda (v mém případě 2). Na všech deskách budou moduly nahrazeny čipy a nezbytnými externími SMD součástkami.

Po připojení solárního panelu do krabičky K bude využito dvou co nejblíže paralelně zapojených elektrolytických kondenzátorů 470 uF / 25 V a jednoho stejně zapojeného keramického kondenzátoru 100 nF / 50 V. Elektrolytické kondenzátory budou fungovat jako zásobárna energie a keramický bude filtrovat rychlé špičky.

V krabičce K bude k solárnímu panelu připojen vysokoimpedanční napěťový dělič tvořený rezistory o hodnotách 1 MΩ a 470 kΩ. Paralelně k rezistoru R2 (470 kΩ) bude připojen blokovací keramický kondenzátor o kapacitě 100 nF a jmenovitém napětí 16 V. Dělič bude sloužit ke snímání napětí solárního panelu, přičemž naměřené hodnoty budou odesílány do M přes ADC pin v režimu analog input. Díky vysoké impedanci bude proudový odběr děliče zanedbatelný.

K solárnímu panelu budou sériově připojeny dva P-MOS tranzistory s nízkým RDS(on), spojené back-to-back (drainy proti sobě), které budou tvořit výkonový spínač celého systému. Jejich řízení bude realizováno jedním budicím logic-level N-MOS tranzistorem, protože napětí 3,3 V z M není dostatečné pro jejich přímé sepnutí. N-MOS tranzistor bude řízen M. Na jeho gate bude sériově připojen rezistor o hodnotě 220 Ω pro omezení proudových špiček na výstupu M. Mezi gate a společnou zem bude paralelně připojen pulldown rezistor o hodnotě 10 kΩ, který zabrání vzniku nedefinovaného logického stavu. Drain bude připojen k akumulátoru, pull-up rezistoru 10 kΩ a na gate obou P-MOS tranzistorů. P-MOS tranzistory budou tedy spínané 6 V. Source bude připojen ke společné zemi. Tahle část obvodu bude sloužit jako jednoduchý a velmi úsporný solární regulátor napájení.

Důvodem použití dvou P-MOS tranzistorů namísto jednoho je přítomnost parazitní diody, která je nedílnou součástí jejich vnitřní struktury. Pokud by byl použit pouze jeden tranzistor, protékal by večer i při jeho rozepnutí proud z akumulátoru zpět do solárního panelu prostřednictvím této diody, což by způsobovalo samovolné vybíjení akumulátoru. Pokud nastane výpadek napájení, tak se P-MOS tranzistory automaticky zavřou.

Modul proudového a napěťového senzoru INA219 bude spolu s vyrovnávacím keramickým kondenzátorem o parametrech 100 nF / 50 V, co nejblíže paralelně zapojeným mezi piny Vin+ a GND, zapojen v krabičce K mezi akumulátor a vstup VM pro napájení motoru u H-bridge. Jednou z jeho funkcí bude snímat napětí akumulátoru.

Na základě údajů z modulu INA219 a děliče napětí bude M prostřednictvím sběrnice I²C, resp. portu GPIO vyhodnocovat stav akumulátoru a solárního panelu.

Pomocí PWM regulace bude M, přes hlavní spínač systému, přepínat mezi způsoby dobíjení akumulátoru. Stav 1: Bulk bude zvolen, když bude napětí akumulátoru pod limitem. V tomto stavu bude střída PWM 100 %. Stav 2: Absorbtion bude zvolen po dosažení limitu napětí akumulátoru, který je v létě 7,2 V, na jaře / podzim 7,3 V a v zimě 7,5 V. Zde bude střída PWM adaptivní, její úděl bude udržet napětí na akumulátoru konstantní. Stav 3: Float je posledním stavem, který nastane když proud klesne k nule (střida PWM kolem 5 %). V tomto stavu bude potřeba nastavit udržovací napětí, které je v létě 6,7 V, na jaře / podzim 6,8 V a v zimě 6,9 V.

Při kritickém vybití akumulátoru, kdy se jeho napětí přiblíží k hodnotě 5,4 V, M vypne všechny nepotřebné části systému, Mx uvede do StandBy režimu bez RTC a sám přejde do režimu Shutdown. Pro probuzení je potřeba mikrořadiče resetovat.

V noci, kdy panel nebude dodávat žádné napětí musí M zamezit přepólování, aneb ke vzniku zpětného proudu směrem do panelu. To udělá odpojením hlavního spínače systému.

Senzor INA219 bude současně využíván jako proudový snímač. M bude monitorovat proud odebíraný z akumulátoru. Náhlé zvýšení proudu během pohybu dvířek bude indikovat jejich zablokování nebo náraz do překážky, například slepice. V takovém případě se M zastaví na 250 ms, pokusí se obrátit směr otáčení motoru, dvířka vrátí zpět do původní polohy, uspí se a po 10 minutách to zkusí znovu. Pokud ani zpětný chod problém nevyřeší, systém se vypne. Bude nutné brát v potaz krátkodobé proudové špičky vznikající při rozběhu motoru.

Při pohybu dvířek budou postupně spínány (debounce 50 ms), respektive rozpínány dva mikrospínače. Kontakt COM bude připojen k akumulátoru a kontakt NO spolu se silným 1 kΩ externím pulldown rezistorem k M. Koncové spínače představují jednoduché a energeticky nenáročné řešení určování krajních poloh.

M bude dále napřímo (bez PWM modulace) řídit H-bridge Pololu DRV8838, který bude vybaven elektrolytickým kondenzátorem 47 µF / 25 V zapojeným co nejblíže mezi piny VM a GND, jenž bude potlačovat indukční napěťové špičky vznikající při vypínání motoru. Ten bude odrušen keramickým kondenzátorem 100 nF zapojeným přímo mezi jeho vývody a dvěma keramickými kondenzátory 47 nF zapojenými mezi jednotlivé vývody a kostru motoru (Faradayova klec). Všechny kondenzátory budou dimenzovány na napětí 50 V. Toto odrušení je nezbytné pro omezení jiskření kartáčků a potlačení vysokofrekvenčního elektromagnetického rušení. H-bridge i elektromotor budou umístěny v krabičce K.

Měření váhy snáškového hnízda bude zprostředkovávat tenzometr se zanedbatelnou nelinearitou a hysterezí. Stíněný kabel od tenzometru bude připojen k modulu AČ převodníku HX711 s nízkým klidovým proudem v řádu jednotek mikroampér, umístěnému v krabičce Kx. Modul bude použit z důvodu zesílení velmi nízkého výstupního napětí tenzometru, které se pohybuje v řádu jednotek milivoltů. Stínění kabelu bude připojeno ke společné zemi, na desce plošných spojů, za účelem odvodu šumu. Převodník bude připojen k Mx.

Mx bude pro komunikaci s M prostřednictvím datového kabelu typu UTP využívat sběrnici RS485. První kroucený pár bude sloužit k přenosu napájení pro Mx, přičemž oba vodiče budou zapojeny paralelně. Stejně tak druhý pár, který bude sloužit k přenosu napájení pro HX711 a MAX3485 přes tranzistorový spínač. Třetí pár bude stejným způsobem použit pro propojení společné země. Čtvrtý pár bude přenášet data prostřednictvím čipu MAX3485, který bude sloužit jako transceiver sběrnice RS485. Jeden čip bude před M a druhý před Mx.

Tento čip vytváří diferenciální signál na linkách A B, čímž zvyšuje odolnost komunikace proti elektromagnetickému rušení. Jelikož je to čip, tak je pro prototyp potřeba použít adaptér SOIC8 na DIP8 a kolíkové lišty.

Vzhledem k použití několika snáškových hnízd bude komunikace probíhat mezi jednou řídicí jednotkou M (master) a několika jednotkami Mx (slave), které budou propojeny sériově v topologii Daisy Chain. Vzhledem ke krátké délce vedení v řádu jednotek metrů nebude nutné na začátek ani konec sběrnice připojovat zakončovací rezistory o hodnotě 120 Ω pro impedanční přizpůsobení vedení. Jejich použití by pouze zvyšovalo proudový odběr systému.

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

V horní i dolní části hranolku budou ve svislé ose zapuštěny dva pákové koncové mikrospínače. Záhlubení široká 20 mm povedou skrz hranolek, kvůli vývodu pinů. Mikrospínače budou upevněny pomocí vrutů o průměru 2 mm. Je důležité otvory předvrtat, kvůli deformaci plastu při vrtání do dřeva.

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

**Konektory**  
https://www.hadex.cz/p/d626-zdirka-rj45-do-dps-8p8c (5 ks)  

**Klipy**  
Hornbach (cca 20 ks)  

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

**H-můstek**  
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
https://www.laskakit.cz/en/prevodnik-ttl-na-rs-485--max485/ (3 ks)  

**Adaptér**  
https://dratek.cz/arduino-platforma/1046-dps-adapter-sop8-so8-soic8-na-dip8.html (3ks)  

**Kondenzátory**  
https://www.gme.cz/v/1486151/hitano-ck-1u-50v-x7r-rm508-10-keramicky-kondenzator (4 ks)  
https://www.hadex.cz/p/i857-470u-25v-105-8x14x5mm-elektrolyt-kondenzator-radialni (4 ks)  

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
| HADEX         | 100 Kč      |
| HORNBACH      | 800 Kč      |
| DEXHAL        | 750 Kč      |   
| LEVNE-BATERKY | 350 Kč      |
| ECOM          | 100 Kč      |
| LASKAKIT      | 600 Kč      |
| DRATEK        | 300 Kč      |
| BOTLAND       | 1800 Kč     |
| ZENIT         | 100 Kč      |
| LIHNEME       | 1000 Kč     |
| REZERVA       | 1000 Kč     |
| **CELKEM**    | **8800 Kč** |

*Poznámka: Cena je orientační a je do ní započtena i doprava.*
