# Bakalářská práce
Systém pro automatizaci kurníku s detekcí snesených vajec

## Zadání
1. Prostudujte možnosti automatizace uzavírání kurníku, možné metody detekce snesených vajec a dostupné možnosti komunikace a připojení zařízení do IoT sítě (např. NB-IoT, WiFi, LoraWAN).
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
Systém bude obsahovat jednu krabičku pro akumulátor o rozměrech 250 × 175 × 150 mm a tloušťce stěny 2,4 mm, dále druhou krabičku (K) o rozměrech × a tloušťce stěny 1,6 mm, která bude určena pro mechaniku, elektroniku dvířek a hlavní mikrokontrolér (M). Pro každé snáškové hnízdo bude dále určena samostatná krabička (Kx) o rozměrech × a tloušťce stěny 1,6 mm. Každá tato krabička bude obsahovat vedlejší mikrokontrolér (Mx).

Všechny krabičky budou vytištěny na 3D tiskárně z materiálu PETG. Tento materiál byl zvolen pro vyšší houževnatost a teplotní odolnost oproti materiálu PLA. Jejich zadní stěna bude ve svislém směru prodloužena a bude mít tloušťku 3,5 mm. V každém rohu této stěny budou vytvořeny otvory pro vruty umožňující upevnění krabiček ke stěně. Zvětšená tloušťka zvyšuje tuhost při montáži pomocí vrutů. Krabičky budou mít ve stěnách naproti sobě 2 větrací otvory o průměru 1,5 cm.

Dále budou na všech krabičkách vytvořeny mírně zaoblené nízké drážky ve tvaru písmene U, směřující od horní hrany směrem dolů. Tyto drážky budou sloužit k protažení kabelů. Pro speciální solární kabel o průřezu budou mít šířku 6 mm a pro datový kabel 5,5 mm. Průměr 5,5 mm bude mít kruhový otvor umístěný na spodní straně krabičky Kx, určený pro průchod stíněného kabelu. V krabičce K bude zespodu otvor pro lanko o průměru 3 mm.

Víko bude vytištěno samostatně s přesahem 2 mm a bude obsahovat dutou trubičku uprostřed levé hrany. Na horní hraně levé stěny každé krabičky budou po stranách na malé plošince umístěny dvě trubičky s průměrem otvoru 2,1 mm. Po vytištění bude všemi třemi trubičkami protažen filament o průměru 1,75 mm, jehož konce budou krátce nahřáty zapalovačem a roztemovány tak, aby vytvořily hlavičky. Tím vznikne jednoduchý pant pro uchycení víka. Toto řešení minimalizuje počet součástek a umožňuje výrobu celé krabičky bez dalších kovových dílů.

### Mechanika
Hlavní část systému bude umístěna na vnější stěně kurníku, která bude splňovat požadavky na umístění solárního panelu uvedené v kapitole Napájení. Toto řešení zjednoduší montáž a zároveň z velké části eliminuje působení amoniaku ze slepičího trusu na elektroniku.

Na stěnu budou pomocí vrutů do dřeva připevněny dřevěné hranolky. K jejich bočním stranám budou následně pomocí vrutů upevněny hliníkové U-profily s vnitřním rozměrem 10 mm. U-profily zajišťují přesné vedení dvířek při minimálním tření.

V těchto profilech se budou ve svislém směru pohybovat bílá pěněná PVC dvířka o rozměrech 24 × 35 cm a tloušťce 8 mm (pozor na tolerance). Při vrtání otvorů do PVC bude nutné použít nízké otáčky a nevyvíjet nadměrný přítlak. Pěněné PVC bylo zvoleno díky nízké hmotnosti, odolnosti proti vlhkosti a snadné obrobitelnosti.

Na horní části přední strany dvířek bude umístěno očko, upevněné pomocí zapuštěné podložky a matice M5 z nerezové oceli A2. Očko bude procházet otvorem o stejném průměru, jako je jeho metrický závit. Podložka i matice budou zapuštěny do záhlubení o průměru 10 mm a hloubce 3 mm.

Očkem bude protaženo syntetické lanko (zednická stavební šňůra) o průměru 2 mm, které bude zajištěno dračí smyčkou zakápnutou vteřinovým lepidlem. Syntetická šňůra nekoroduje, má nízkou průtažnost a je odolná vůči vlhkosti.

Lanko povede kolmo vzhůru do krabičky K, ve které bude navinuto na špulku vytištěnou z materiálu PETG. Špulka bude mít šířku 2 cm a vnější průměr 2,5 cm. Otvorem o průměru 3 mm, umístěným mírně mimo osu špulky směrem od motoru, bude provlečeno skrz její tělo a upevněno osmičkovým uzlem, který bude rovněž zakápnut vteřinovým lepidlem.

Špulka bude opatřena otvorem s D-profilem o průměru 3 mm a pomocí stavěcího šroubu (červíku) M3 z nerezové oceli A2 a mosazné závitové vložky M3 (délka 4 mm, průměr 4,5 mm) bude upevněna na hřídeli nízkootáčkového stejnosměrného motoru s kovovou převodovkou (6 V).

Otvor pro mosaznou závitovou vložku bude mít průměr 5 mm a v posledních 4 mm směrem ke hřídeli se zúží na průměr 4,1 mm. Do otvoru bude zavedena vložka pomocí dlouhého šroubu M3. Jakmile vložka narazí na zúženou část otvoru, bude hlava šroubu nahřáta mikropájkou. Plast se v místě zúžení roztaví, což umožní zalisování vložky. Po odejmutí pájky plast opět ztuhne a vložku pevně ukotví.

V horní i dolní části hranolku budou zapuštěny dva pákové koncové mikrospínače. Při pohybu dvířek po svislé ose budou postupně spínány, respektive rozpínány. Kontakt NO bude připojen ke společné zemi a kontakt COM k M. Koncové spínače představují jednoduché a energeticky nenáročné řešení určování krajních poloh.

Vedlejší část systému bude umístěna u snáškových hnízd orientovaných příčně ke stěně.

Pod kukaně bude pomocí vrutů připevněna březová sítotisková překližka o rozměrech 37 × 28 cm a tloušťce 12 mm. Druhá překližka o rozměrech 35 × 26 cm bude tvořit základovou desku a bude připevněna k podlaze kurníku. Tyto desky budou vyrobeny z jedné nařezané překližky o rozměrech 765 × 317 mm (tolerance započteny). Třetí překližka o rozměrech 40 × 50 cm bude sloužit jako zadní stěna. Všechny překližky budou orientovány hladkou stranou směrem nahoru, respektive směrem ke hnízdu. Jejich řezné hrany budou opatřeny voděodolným lakem z důvodu ochrany proti vlhkosti.

Mezi horní a spodní překližkou bude u stěny umístěn tenzometr o tloušťce 12,7 mm, orientovaný kratší stranou s vodiči směrem ke stěně. Tenzometr váží s přesností 2 g, což je pro 50-70 g vejce zcela v pořádku. Je dimenzován na zatížení 20 kg a maximální předpokládané zatížení tvořené dvěma překližkami, hnízdem, dvěma slepicemi a deseti vejci činí přibližně 10 kg.

Optická závora není pro počítání vajec v kurníku vhodná mimo jiné z důvodu výskytu prachu, peří, podestýlky a zvědavých slepic. Všechny tyto objekty by mohly přerušit laserový paprsek a být chybně vyhodnoceny jako propadnuté vejce. Z tohoto důvodu byl pro detekci vajec zvolen tenzometr.

Volný konec tenzometru bude upevněn dvěma zápustnými inbusovými šrouby M4 z nerezové oceli A2. Šrouby budou procházet horní deskou skrz otvory o průměru 4,5 mm a budou zapuštěny do záhlubení o průměru 10 mm a hloubce 3 mm. Mezi hlavu šroubu a desku bude nanesena tenká vrstva PU tmelu.

Pevný konec tenzometru bude upevněn dvěma zápustnými inbusovými šrouby M5 z nerezové oceli A2. Ty budou procházet spodní deskou skrz otvory o průměru 5,5 mm a budou rovněž zapuštěny do záhlubení o průměru 10 mm a hloubce 3 mm.

Použité šrouby budou mít délku 25 mm. Mezi deskami a závity tenzometru budou umístěny distanční matice M5 a M6 z nerezové oceli A2, o tloušťce 4 mm.

Ze zbytku vážicí překližky o rozměru 38,5 × 29,5 cm, budou vyrobeny lišty. Ty budou pomocí vrutů připevněny ke spodní straně horní desky. Lišty budou vysoké 15 mm, protože vzdálenost mezi deskami činí 20,7 mm a celá horní konstrukce musí být podepřena pouze tenzometrem. Současně musí být zachována vůle alespoň 3 mm.

Do zadní lišty bude vyvrtán otvor, kterým bude protažen již prodloužený kabel od tenzometru. Kabel nesmí být mechanicky namáhán tahem. Dále bude veden vzhůru po stěně do krabičky Kx. Z boku krabičky bude vyveden sdělovací kabel, který povede vysoko po stěně až do krabičky K a bude upevněn pomocí hřebíkových příchytek.

Celá konstrukce musí být od stěny vzdálena natolik, aby se jí nikde nedotýkala a současně za ní zůstal prostor pro vedení kabelu. Zároveň je nutné zabránit přístupu slepic a hlodavců do tohoto prostoru.

Uprostřed konstrukce naproti košíku bude vytvořen stavitelný doraz pomocí zápustného inbusového šroubu M5 z nerezové oceli A2 o délce 30 mm. Šroub bude procházet spodní deskou skrz otvor o průměru 4,5 mm a bude zapuštěn do záhlubení o průměru 10 mm a hloubce 6 mm. Doraz bude tvořen mosaznou kloboučkovou maticí M5 (délka 10 mm, závit 7,5 mm) z důvodu omezení ulpívání nečistot. Optimální vůle dorazu je 0,8 mm, což odpovídá jednomu otočení šroubu M5 nebo přibližně tloušťce běžné platební karty.

### Kabeláž
Pro připojení solárního panelu a akumulátoru bude využito speciálního solárního kabelu typu licna o průřezu 4 mm² a hřebíkových přichytek o průměru 4 mm. Tyto kabely budou v K přes prodlužovací WAGO svorky přetransformovány na měděnou ohebnou licnu o průřezu 1.5mm². Ta bude pomocí WAGO svorek o rozteči 5,08 mm připojena k desce plošných spojů. U prototypu bude využíváno drátků do nepájivého pole s průřezem 0.5mm². Pro kladný pól bude použit červený vodič a pro záporný pól černý vodič.

Pro datovou komunikaci byl zvolen stíněný kabel a kabel UTP CAT5e typu licna, který bude ke stěně upevněn pomocí příchytek o průměru 5 mm a konektory RJ45. Prodloužení vodičů tenzometru zajistí přes prodlužovací WAGO svorky stíněný kabel LiYCY 4 × 0,25 mm² typu licna. Jelikož jsou vodiče od tenzometru pro WAGO svorky příliš tenké, tak se u odizolovaného konce přehnou napůl na průřez 0,28 mm². K desce plošných spojů bude potom tento kabel připojen pomocí WAGO svorek o rozteči 2,5 mm.

### Napájení
2 Solární panely o parametrech 6 V / 10 Wp budou paralelně spojeny, svisle připevněny na stěnu pod malou stříšku a orientovány směrem na jih, případně na východ nebo západ (v mém případě bude jeden panel směřovat na jihozápad a druhý na severozápad). Tím bude zajištěno, že panely budou co nejlépe využívat dostupnou sluneční energii. Vertikální montáž omezuje usazování sněhu a nečistot.

Společně s bezúdržbovým olověným AGM akumulátorem o parametrech 6 V / 7 Ah, jenž bude umístěný ve stínu pod stříškou, bude zajišťovat napájení celého systému. Akumulátor LiFePO4 je sice v mnoha ohledech lepší, ale nesmí se používat v zimě. Silová část bude pracovat s napětím 6 V, zatímco veškerá elektronika bude napájena napětím 3,3 V.

Snížení napětí z 6 V na 3,3 V bude realizováno pomocí nízkopříkonového LDO regulátoru. Nízký klidový proud významně prodlužuje provoz z akumulátoru. Tento stabilizátor bude zapojený k baterii a na jeho vstupu bude připojen blokovací keramický kondenzátor o kapacitě 1 µF / 50 V. Stejný kondenzátor bude připojen i na výstupu, ale v tomto případě z důvodu vyhlazení napětí. 

Použití spínaného buck měniče není vhodné z důvodu horší dostupnosti nízkopříkonových variant a velmi malého odběru systému po většinu dne. Jeho vyšší účinnost by se projevila pouze přibližně po dobu dvou minut denně, zatímco po zbytek dne by kvůli vlastní spotřebě dosahoval nižší celkové účinnosti než jednoduchý lineární stabilizátor.

Spotřeba proudu u systému s 5 snáškovými hnízdy bude v režimu spánku pod 50 µA, v aktivním režimu pod 50 mA a při pohybu dvířek kolem 500 mA. Denní spotřeba bude kolem 12 mAh, resp. 50 mWh. Dvířka (motor) se na denní spotřebě podílí ze 40 %, hnízda taktéž ze 40 % a řídicí elektronika a LoRa po 10 %.

|               |                          |                          |                                 |
| :---          | :---:                    | :---:                    | :---:                           |
| **Orientace** | **Letní výkon (Wh/den)** | **Zimní výkon (Wh/den)** | **Zimní bilance (mAh/den)**     |
| **Jih**       | 70–110                   | 16–30                    | +3800 až +7500 (extrémně dobrá) |
| **Východ**    | 50–80                    | 12–24                    | +2500 až +5000 (velmi dobrá)    |
| **Západ**     | 50–80                    | 12–24                    | +2500 až +5000 (velmi dobrá)    |
| **Sever**     | 16–30                    | 4–12                     | +300 až +1800 (přijatelná)      |

*Poznámka: Zimní bilance počítá s efektivitou 70 %.*

### Elektronika
Prototyp bude sestaven z modulů umístěných na nepájivém poli pomocí kolíkových lišt a svorek s roztečí 2,54 mm, které budou obsahovat kovový plíšek pro ochranu licny. Finální verze bude obsahovat jednu hlavní desku plošných spojů a až 5 vedlejších desek pro jednotlivá snášková hnízda. Na všech deskách budou moduly nahrazeny čipy a nezbytnými externími SMD součástkami.

V krabičce K bude k solárnímu panelu připojen vysokoimpedanční napěťový dělič tvořený rezistory o hodnotách 1 MΩ a 470 kΩ. Paralelně k rezistoru R2 (470 kΩ) bude připojen keramický kondenzátor o kapacitě 100 nF a jmenovitém napětí 16 V, který zajistí správnou funkci měření při takto vysoké impedanci. Dělič bude sloužit ke snímání napětí solárního panelu, přičemž naměřené hodnoty budou odesílány do M. Díky vysoké impedanci bude proudový odběr děliče pouze v řádu jednotek mikroampér.

K solárnímu panelu budou sériově připojeny dva P-MOS tranzistory s nízkým RDS(on), spojené back-to-back (drainy proti sobě), které budou tvořit výkonový spínač celého systému. Jejich řízení bude realizováno jedním budícím logic-level N-MOS tranzistorem, protože napětí 3,3 V z M není dostatečné pro jejich přímé sepnutí. N-MOS tranzistor bude řízen M. Na jeho gate bude sériově připojen rezistor o hodnotě 220 Ω pro omezení proudových špiček na výstupu M. Mezi gate a společnou zem bude paralelně připojen pulldown rezistor o hodnotě 10 kΩ, který zabrání vzniku nedefinovaného logického stavu. Drain bude připojen k akumulátoru, pull-up rezistoru 10 kΩ a na gate obou P-MOS tranzistorů. P-MOS tranzistory budou tedy spínané 6 V. Source bude připojen ke společné zemi

Důvodem použití dvou P-MOS tranzistorů namísto jednoho je přítomnost parazitní diody, která je nedílnou součástí jejich vnitřní struktury. Pokud by byl použit pouze jeden tranzistor, protékal by večer i při jeho rozepnutí proud z akumulátoru zpět do solárního panelu prostřednictvím této diody, což by způsobovalo samovolné vybíjení akumulátoru. Pokud nastane výpadek napájení, tak se P-MOS tranzistory automaticky zavřou.

Modul proudového a napěťového senzoru INA219 bude zapojen v krabičce K mezi akumulátor a vstup H-můstku pro napájení motoru. Jeho proud se v řežimu spánku pohybuje v řádu jednotek mikroampér a jednou z jeho funkcí bude snímat napětí akumulátoru.

Na základě údajů z modulu INA219 bude M prostřednictvím sběrnice I²C vyhodnocovat stav akumulátoru. Mohou nastat tři situace. První možností je slunečný den, kdy bude solární panel dodávat napětí vyšší než 7,2 V a akumulátor bude plně nabitý. Druhou možností je noc, kdy panel nebude dodávat žádné napětí a dojde ke vzniku zpětného proudu směrem do panelu. Třetí možností je kritické vybití akumulátoru při poklesu napětí přibližně k hodnotě 5,4 V. V závislosti na těchto stavech M odpojí napájení nebo v případě kritického vybití vypne všechny nepotřebné části systému a přejde do co nejhlubšího režimu spánku.

Senzor INA219 bude současně využíván jako proudový snímač. M bude monitorovat proud odebíraný z akumulátoru. Náhlé zvýšení proudu během pohybu dvířek bude indikovat jejich zablokování nebo náraz do překážky, například slepice. V takovém případě se M zastaví na 250 ms a pokusí se obrátit směr otáčení motoru. Pokud ani změna směru problém nevyřeší, systém se vypne a odešle zprávu o nefunkčnosti dvířek. Bude nutné brát v potaz krátkodobé proudové špičky vznikající při rozběhu motoru.

M bude dále řídit H-můstek DRV8838 s nízkým klidovým proudem. H-můstek bude vybaven elektrolytickým kondenzátorem 47 µF / 25 V zapojeným co nejblíže mezi piny VM a GND, který bude potlačovat indukční napěťové špičky vznikající při vypínání motoru. Ten bude odrušen keramickým kondenzátorem 100 nF zapojeným přímo mezi jeho vývody a dvěma keramickými kondenzátory 47 nF zapojenými mezi jednotlivé vývody a kostru motoru (Faradayova klec). Všechny kondenzátory budou dimenzovány na napětí 50 V. Toto odrušení je nezbytné pro omezení jiskření kartáčků a potlačení vysokofrekvenčního elektromagnetického rušení. H-můstek i elektromotor budou umístěny v krabičce K.

Měření váhy snáškového hnízda bude zprostředkovávat tenzometr se zanedbatelnou nelinearitou a hysterezí. Stíněný kabel od tenzometru bude připojen k modulu AČ převodníku HX711 s nízkým klidovým proudem v řádu jednotek mikroampér, umístěnému v krabičce Kx. Modul bude použit z důvodu zesílení velmi nízkého výstupního napětí tenzometru, které se pohybuje v řádu jednotek milivoltů. Stínění kabelu bude připojeno ke společné zemi, na desce plošných spojů, za účelem odvodu šumu. Převodník bude připojen k Mx, který bude schopen převodník uspat a tím současně odpojit napájení tenzometru.

Mx bude pro komunikaci s M prostřednictvím datového kabelu typu UTP využívat sběrnici RS485. První kroucený pár bude sloužit k přenosu napájení, přičemž oba vodiče budou zapojeny paralelně. Druhý pár bude stejným způsobem použit pro propojení společné země. Třetí pár bude přenášet data prostřednictvím čipu MAX3485 s keramickým kondenzátorem 100 nF / 16 V zapojeným co nejblíže mezi piny VCC a GND. Čip bude u prototypu připájen k adaptéru SOIC-8 na DIP-8 a bude sloužit jako transceiver sběrnice RS485. Jeden čip bude před M a druhý před Mx.

Na rozdíl od sběrnice I²C je RS485 vhodná pro komunikaci na delší vzdálenosti. Pro přenos dat bude využit protokol LPUART, který nevyužívá hodinový signál a vyznačuje se nízkou spotřebou energie. Čip MAX3485 vytváří diferenciální signál na linkách A B, čímž zvyšuje odolnost komunikace proti elektromagnetickému rušení. Současně jej bude možné prostřednictvím Mx uvést do režimu shutdown, ve kterém bude odebírat pouze proud v řádu jednotek nanoampér. Vše od tenzometru až po čip MAX3485 musí být co nejblíže k sobě.

Vzhledem k použití několika snáškových hnízd bude komunikace probíhat mezi jednou řídicí jednotkou M (master) a několika jednotkami Mx (slave), které budou propojeny sériově v topologii Daisy Chain. Vzhledem ke krátké délce vedení v řádu jednotek metrů nebude nutné na začátek ani konec sběrnice připojovat zakončovací rezistory o hodnotě 120 Ω pro impedanční přizpůsobení vedení. Jejich použití by pouze zvyšovalo proudový odběr systému.

Na aplikační vrstvě bude použit komunikační protokol Modbus RTU a knihovna ModbusRTU-Slave. Protokol Modbus RTU vytvoří datový rámec obsahující adresu jednotky slave, přenášená data a kontrolní součet CRC pro detekci chyb při přenosu. Hardware LPUART v mikrokontroléru následně převede jednotlivé bajty na sériový datový tok, doplní start a stop bity a zajistí jejich přenos po sběrnici. Na straně přijímače proběhne opačný proces.

K čipům M a Mx budou na finálních deskách plošných spojů přímo k pinům VDD a VSS, resp. VDDA a VSSA paralelně připojeny blokovací keramické kondenzátory o kapacitách 10 µF / 25 V a 100 nF / 50 V pro filtraci vysokofrekvenčního šumu. Desky plošných spojů budou obsahovat souvislou zemní plochu připojenou k zápornému pólu solárního panelu a akumulátoru.

### Řízení
Hlavní řídicí jednotkou systému bude mikrokontrolér STM32 s integrovaným LoRa modulem, komunikujícím na frekvenci 868 MHz. LoRa modul umožní na rozdíl od Wi-Fi komunikaci na velké vzdálenosti při nízké spotřebě energie. NB-IoT není pro tento typ projektu vhodnou technologií. U každého snáškového hnízda bude umístěn další mikrokontrolér STM32.

Firmware bude vyvíjen v prostředí STM32CubeIDE. Součástí firmwaru hlavní řídicí jednotky budou astronomické hodiny, podle kterých se budou automaticky otevírat a zavírat dvířka. Použití časovače nebylo zvoleno z důvodu proměnlivé délky dne. Světelný senzor by mohl způsobovat chybné sepnutí při zatažené obloze (déšť nebo bouřka) nebo ve večerních hodinách vlivem pouličního osvětlení či světlometů automobilů.

Použité čipy podporují režim hlubokého spánku (Standby), který umožňuje výrazně snížit spotřebu energie. Klidový proud se v tomto režimu pohybuje v řádu jednotek mikroampér. Hlavní řídicí jednotka se bude společně s nezbytnými částmi systému probouzet každých 10 minut za účelem kontroly stavu solárního panelu a akumulátoru (stovky milisekund). Dále se bude společně s ostatními řídicími jednotkami a dalšími potřebnými částmi systému probouzet 24× denně, tedy každou hodinu, pro sekvenční kontrolu stavu vajec u každého hnízda (24 × 5 hnízd × 5 s na jedno hnízdo = 10 minut). Nakonec se bude probouzet ráno a večer, opět pouze s nezbytnými částmi systému, za účelem otevření a zavření dvířek (cca 2 × 20 s). Před zahájením pohybu dvířek budou piny koncových spínačů nastaveny jako digitální vstupy s interním pull-up rezistorem. Po dosažení koncové polohy budou přepnuty do analogového režimu, čímž bude eliminován jejich klidový odběr.

Pro vývoj budou použity vývojové desky stejného nebo podobného typu, jako budou čipy použité na finální desce plošných spojů. Finální deska bude obsahovat čtyřpinovou lištu pro připojení programátoru z vývojové desky.

K hlavnímu čipu bude připojena anténa pro LoRa pásmo ve formě měděného vodiče o délce 8,2 cm (čtvrtvlnný monopól), například z běžného UTP kabelu. Anténa bude připájena přímo k desce plošných spojů a může zůstat uvnitř krabičky. Musí však být umístěna co nejdále od akumulátoru a motoru. V prostoru o poloměru 2 cm kolem antény ani pod ní se nesmí vyskytovat měď ani žádné elektronické součástky.

V domě bude umístěna vývojová deska ESP32 s integrovaným LoRa modulem a anténou, která bude plnit funkci internetové brány. Veškerá data přijatá touto deskou budou následně odesílána do cloudové databáze.

## Algoritmy a vývojové diagramy

### Stavový automat pro algoritmus detekce snesených vajec
Probuzení mikrokontroléru a převodníku HX711.  
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
Přechod mikrokontroléru i převodníku HX711 do režimu spánku.  

<img src="https://github.com/Hosty-04/Bachelors_Theses/blob/main/flowcharts/egg_detection_flowchart.png" alt="egg_detection_flowchart" height="100px">

## Nákup

**Kabeláž**  
https://www.gme.cz/v/1511630/solarkabel-xh-400mm2-cerveny (5 m)  
https://www.gme.cz/v/1511631/solarkabel-xh-400mm2-cerny (5 m)  
https://www.gme.cz/v/1512358/elektrokabel-cya-1x15-cerveny-h07v-k-izolovany-vodic-lanko (5 m)  
https://www.gme.cz/v/1512357/elektrokabel-cya-1x15-cerny-h07v-k-izolovany-vodic-lanko (5 m)  

https://www.alza.cz/alzapower-patch-cat5e-utp?dq=6592131 (5 m)  

Willmann Elektro Lipník (2m)  

**Svorkovnice**  
https://www.gme.cz/v/1501431/wago-256-402-svorkovnice-2pol-roztec-508mm-24a-320v-vstup-45-pruzina (2 ks)  
https://www.gme.cz/v/1502753/wago-250-405-svorkovnice-5pol-roztec-25mm-4a-160v-vstup-45-pruzina  
https://www.gme.cz/v/1513875/wago-221-2411-svorka-bezsroubova-2pol-vodic-do-4mm2 (8 ks)  

**Konektory**  
https://www.hadex.cz/p/d626-zdirka-rj45-do-dps-8p8c (2 ks)  

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
https://cz.mouser.com/ProductDetail/Microchip-Technology/MCP1700-3302E-TO?qs=h7tZ5KkzNMMPEB66r2rMQw%3D%3D (2 ks)  

**Brána**  
https://www.laskakit.cz/heltec-wifi-lora-32-v3-868mhz-0-96--wifi-modul/  

**Master (vývojová deska a čip)**  
https://cz.mouser.com/ProductDetail/STMicroelectronics/NUCLEO-WL55JC1?qs=zW32dvEIR3uxo98jZ7%2FN0g%3D%3D  
https://cz.mouser.com/ProductDetail/STMicroelectronics/STM32WLE5CCU6?qs=26oI9%252Bq63sLM90C6xjPI1g%3D%3D&utm_id=9873308831&utm_source=google&utm_medium=cpc&utm_marketing_tactic=emeacorp  

**Slave (vývojová deska a čip)**  
https://cz.mouser.com/ProductDetail/STMicroelectronics/NUCLEO-U031R8?qs=sGAEpiMZZMv0WPLDnYsI70d1YaGuDSdFrBEvuBfrX2oCPZoXXcsbiw%3D%3D  
https://cz.mouser.com/ProductDetail/STMicroelectronics/STM32G031G8U6?qs=uwxL4vQweFO0dXSA%2FIGJ2w%3D%3D  

**Headery**  
https://www.laskakit.cz/dupont-40pin-2-54-mm-pinovy-pas/ (2 ks)  
https://www.laskakit.cz/dupont-2x40pin-2-54-mm-pinovy-pas/  

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
https://www.laskakit.cz/vazici-senzor-20kg-adc-modul-hx711/  

**Převodník**  
https://www.laskakit.cz/ad-prevodnik-modul-24-bit-2-kanaly-hx711/  

**Transceiver**  
https://cz.mouser.com/ProductDetail/Analog-Devices-Maxim-Integrated/MAX3485EESA+?qs=1THa7WoU59HLsdsozrMCeQ%3D%3D (3 ks)  

**Adaptér**  
https://dratek.cz/arduino-platforma/1046-dps-adapter-sop8-so8-soic8-na-dip8.html (3 ks)  

**Kondenzátor**  
https://www.gme.cz/v/1486151/hitano-ck-1u-50v-x7r-rm508-10-keramicky-kondenzator (4 ks)  

**Překližka**  
Hornbach (2 ks)  

**Červík**  
Handy corp. Hranice / Přerov (2 ks)  

**Vložka**  
https://www.majkl3d.cz/p/zavitova-vlozka-do-plastu-mosaz-m3 (2 ks)  

**Lanko**  
Hornbach  

**Očko**  
Hornbach  

**Šrouby Matice Podložky Vruty**  
Hornbach (6, 6, 1, sada)  

**PU tmel**  
Hornbach  

|               |             |
| :---          | :---:       |
| **OBCHOD**    | **CENA**    |
| GME           | 700 Kč      |
| ALZA          | 200 Kč      |
| HADEX         | 100 Kč      |
| HORNBACH      | 600 Kč      |
| DEXHAL        | 750 Kč      |   
| LEVNE-BATERKY | 350 Kč      |
| MOUSER        | 2400 Kč     |
| LASKAKIT      | 1100 Kč     |
| DRATEK        | 300 Kč      |
| BOTLAND       | 200 Kč      |
| ZENIT         | 100 Kč      |
| LIHNEME       | 1000 Kč     |
| MAJKL3D       | 100 Kč      |
| REZERVA       | 1000 Kč     |
| **CELKEM**    | **8900 Kč** |

*Poznámka: Cena je orientační a je do ní započtena i doprava.*
