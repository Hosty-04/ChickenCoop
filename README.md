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

## Koncept

### Krabičky
Systém bude obsahovat jednu krabičku pro akumulátor o rozměrech × a tloušťce stěny 2,4 mm a druhou krabičku (K) o rozměrech × a tloušťce stěny 1,6 mm, která bude určena pro mechaniku, elektroniku dvířek a hlavní mikrokontrolér (P). Pro každé snáškové hnízdo bude dále určena samostatná krabička (Kx) o rozměrech × a tloušťce stěny 1,6 mm.

Všechny krabičky budou vytištěny na 3D tiskárně z materiálu PETG. Jejich zadní stěna bude ve svislém směru prodloužena a bude mít tloušťku 3,5 mm. V každém rohu této stěny budou vytvořeny otvory pro vruty umožňující upevnění krabiček ke stěně.

Na krabičkách budou dále vytvořeny mírně zaoblené nízké drážky ve tvaru písmene U, směřující od horní hrany směrem dolů. Tyto drážky budou sloužit k protažení kabelů. Pro vodiče o průřezu 0,5 mm² budou mít šířku 2,5 mm, pro vodiče o průřezu 1,5 mm² šířku 3,5 mm a pro datový kabel šířku 5,5 mm. Stejný průměr bude mít i kruhový otvor umístěný na spodní straně krabičky Kx, určený pro průchod stíněného kabelu.

Víko bude vytištěno samostatně s přesahem 2 mm a bude obsahovat dutou trubičku uprostřed zadní hrany. Na horní hraně zadní stěny každé krabičky budou po stranách umístěny dvě trubičky s průměrem otvoru 2,1 mm. Po vytištění jimi bude protažen filament o průměru 1,75 mm, jehož konce budou krátce nahřáty zapalovačem a roztemovány tak, aby vytvořily hlavičky. Tím vznikne jednoduchý pant pro uchycení víka.

### Mechanika
Hlavní část systému bude umístěna na vnější stěně kurníku, která bude splňovat požadavky na umístění solárního panelu uvedené v kapitole Napájení. Toto řešení zjednoduší montáž a zároveň z velké části eliminuje působení amoniaku ze slepičího trusu na elektroniku.

Na stěnu budou pomocí vrutů do dřeva připevněny dřevěné hranolky. K jejich bočním stranám budou následně pomocí vrutů upevněny hliníkové U-profily s vnitřním rozměrem 10 mm. V těchto profilech se budou ve svislém směru pohybovat bílá pěněná PVC dvířka o rozměrech × a tloušťce 8 mm. Při vrtání otvorů do PVC bude nutné použít nízké otáčky a nevyvíjet nadměrný přítlak.

Na horní části přední strany dvířek bude umístěno očko, upevněné pomocí zapuštěné podložky a matice M5 z nerezové oceli A2. Očko bude procházet otvorem o stejném průměru, jako je jeho metrický závit. Podložka i matice budou zapuštěny do záhlubení o průměru 10 mm a hloubce 3 mm.

Očkem bude protaženo syntetické lanko (zednická stavební šňůra) o průměru 2 mm, které bude zajištěno dračí smyčkou zakápnutou vteřinovým lepidlem. Lanko povede kolmo vzhůru do krabičky K, ve které bude navinuto na špulku vytištěnou z materiálu PETG. Špulka bude mít šířku 2 cm a vnější průměr 2,5 cm.

Lanko bude otvorem o průměru 3 mm, umístěným mírně mimo osu špulky směrem od motoru, provlečeno skrz její tělo a upevněno osmičkovým uzlem, který bude rovněž zakápnut vteřinovým lepidlem.

Špulka bude opatřena otvorem s D-profilem o průměru 3 mm a pomocí stavěcího šroubu (červíku) M3 z nerezové oceli A2 a mosazné závitové vložky M3 (délka 4 mm, průměr 4,5 mm) bude upevněna na hřídeli nízkootáčkového stejnosměrného motoru s kovovou převodovkou (6 V).

Otvor pro mosaznou závitovou vložku bude mít průměr 5 mm a v posledních 4 mm směrem ke hřídeli se zúží na průměr 4,1 mm. Do otvoru bude zavedena vložka pomocí dlouhého šroubu M3. Jakmile vložka narazí na zúženou část otvoru, bude hlava šroubu nahřáta mikropájkou. Plast se v místě zúžení roztaví, což umožní zalisování vložky. Po odejmutí pájky plast opět ztuhne a vložku pevně ukotví.

V horní i dolní části hranolku budou zapuštěny dva pákové koncové mikrospínače. Při pohybu dvířek po svislé ose budou postupně spínány, respektive rozpínány. Kontakt NO bude připojen ke společné zemi a kontakt COM k P.

Před zahájením pohybu dvířek budou piny koncových spínačů nastaveny jako digitální vstupy s interním pull-up rezistorem. Po dosažení koncové polohy budou přepnuty do analogového režimu, čímž bude eliminován jejich klidový odběr.

Vedlejší část systému bude umístěna u snáškových hnízd orientovaných příčně ke stěně.

Pod kukaně bude pomocí vrutů připevněna březová sítotisková překližka o rozměrech 37 × 28 cm a tloušťce 12 mm. Druhá překližka o rozměrech 35 × 26 cm bude tvořit základovou desku a bude připevněna k podlaze kurníku. Třetí překližka o rozměrech 40 × 50 cm bude sloužit jako zadní stěna. Všechny překližky budou orientovány hladkou stranou směrem nahoru, respektive směrem ke hnízdu. Jejich řezné hrany budou opatřeny voděodolným lakem z důvodu ochrany proti vlhkosti.

Mezi horní a spodní překližkou bude u stěny umístěn tenzometr o tloušťce 12,7 mm, orientovaný kratší stranou s vodiči směrem ke stěně. Tenzometr je dimenzován na zatížení 20 kg. Maximální předpokládané zatížení tvořené dvěma překližkami, hnízdem, dvěma slepicemi a deseti vejci činí přibližně 10 kg.

Volný konec tenzometru bude upevněn dvěma zápustnými inbusovými šrouby M4 z nerezové oceli A2. Šrouby budou procházet horní deskou skrz otvory o průměru 4,5 mm a budou zapuštěny do záhlubení o průměru 10 mm a hloubce 3 mm. Mezi hlavu šroubu a desku bude nanesena tenká vrstva PU tmelu.

Pevný konec tenzometru bude upevněn dvěma zápustnými inbusovými šrouby M5 z nerezové oceli A2. Ty budou procházet spodní deskou skrz otvory o průměru 5,5 mm a budou rovněž zapuštěny do záhlubení o průměru 10 mm a hloubce 3 mm.

Použité šrouby budou mít délku 25 mm. Mezi deskami a závity tenzometru budou umístěny distanční matice M5 a M6 z nerezové oceli A2, o tloušťce 4 mm.

Ze zbytku vážicí překližky, která bude z obchodu nařezána na rozměr 38,5 × 29,5 cm, budou vyrobeny lišty. Ty budou pomocí vrutů připevněny ke spodní straně horní desky. Lišty budou vysoké 15 mm, protože vzdálenost mezi deskami činí 20,7 mm a celá horní konstrukce musí být podepřena pouze tenzometrem. Současně musí být zachována vůle alespoň 3 mm.

Krátké vodiče tenzometru budou prodlouženy stíněným lankovým kabelem. Jednotlivé vodiče budou odizolovány a bude na ně navlečena smršťovací bužírka. Následně budou spojeny pomocí pájecích smršťovacích spojek a zataveny zapalovačem. Poté bude smršťovací bužírka přetažena přes spoje a rovněž zahřáta.

Do zadní lišty bude vyvrtán otvor, kterým bude protažen již prodloužený kabel od tenzometru. Kabel nesmí být mechanicky namáhán tahem. Dále bude veden vzhůru po stěně do krabičky Kx. Z boku krabičky bude vyveden sdělovací kabel, který povede vysoko po stěně až do krabičky K a bude upevněn pomocí hřebíkových příchytek.

Celá konstrukce musí být od stěny vzdálena natolik, aby se jí nikde nedotýkala a současně za ní zůstal prostor pro vedení kabelu. Zároveň je nutné zabránit přístupu slepic a hlodavců do tohoto prostoru.

Uprostřed konstrukce naproti košíku bude vytvořen stavitelný doraz pomocí zápustného inbusového šroubu M5 z nerezové oceli A2 o délce 30 mm. Šroub bude procházet spodní deskou skrz otvor o průměru 4,5 mm a bude zapuštěn do záhlubení o průměru 10 mm a hloubce 6 mm. Doraz bude tvořen mosaznou kloboučkovou maticí M5 (délka 10 mm, závit 7,5 mm) z důvodu omezení ulpívání nečistot. Optimální vůle dorazu je 0,8 mm, což odpovídá jednomu otočení šroubu M5 nebo přibližně tloušťce běžné platební karty.

### Kabeláž
Pro silové rozvody, zahrnující připojení solárního panelu, akumulátoru, H-můstku a motoru, bude použita měděná ohebná licna o průřezu 1,5 mm². Pro napájení elektroniky a signálová vedení bude použita měděná ohebná licna o průřezu 0,5 mm².

Pro kladný pól bude použit červený vodič a pro záporný pól černý vodič. Pro datovou komunikaci byl zvolen stíněný kabel a kabel UTP CAT5e typu licna, který bude ke stěně upevněn pomocí hřebíkových příchytek o průměru 5 mm.

### Napájení
Solární panel o parametrech 6 V / 10 W bude připevněn svisle na stěnu pod malou stříšku a orientován směrem na jih, případně na východ nebo západ. Pokud nebude jiná možnost, bude orientován na sever. Tím bude zajištěno, že panel nebude zakrytý sněhem a zároveň bude co nejlépe využívat dostupnou sluneční energii.

Společně s bezúdržbovým olověným AGM akumulátorem o parametrech 6 V / 7 Ah bude zajišťovat napájení celého systému. Silová část bude pracovat s napětím 6 V, zatímco veškerá elektronika bude napájena napětím 3,3 V.

Snížení napětí z 6 V na 3,3 V bude realizováno pomocí LDO regulátoru s nízkým klidovým proudem v řádu jednotek mikroampér.

Použití spínaného buck měniče není vhodné z důvodu horší dostupnosti variant s nízkým klidovým proudem a velmi malého odběru systému po většinu dne. Jeho vyšší účinnost by se projevila pouze přibližně po dobu dvou minut denně, zatímco po zbytek dne by kvůli vlastní spotřebě dosahoval nižší celkové účinnosti než jednoduchý lineární stabilizátor.

### Elektronika
Prototyp bude sestaven z modulů umístěných na nepájivém poli pomocí kolíkových lišt a svorek s roztečí 2,54 mm, které budou obsahovat kovový plíšek pro ochranu licny. Finální verze bude obsahovat jednu hlavní desku plošných spojů a x vedlejších desek pro jednotlivá snášková hnízda. Na všech deskách budou moduly nahrazeny čipy a nezbytnými externími SMD součástkami. Jako rozhraní budou použity šroubovací svorkovnice s roztečí 5,08 mm a konektory RJ45.

V krabičce K bude k solárnímu panelu připojen vysokoimpedanční napěťový dělič tvořený rezistory o hodnotách 1 MΩ a 470 kΩ. Paralelně k rezistoru R2 (470 kΩ) bude připojen keramický kondenzátor o kapacitě 100 nF a jmenovitém napětí 16 V, který zajistí správnou funkci měření při takto vysoké impedanci. Dělič bude sloužit ke snímání napětí solárního panelu, přičemž naměřené hodnoty budou odesílány do P. Díky vysoké impedanci bude proudový odběr děliče pouze v řádu jednotek mikroampér.

K solárnímu panelu budou sériově připojeny dva P-MOS tranzistory (drainy proti sobě), které budou tvořit výkonový spínač celého systému. Jejich řízení bude realizováno jedním N-MOS tranzistorem, protože napětí 3,3 V z P není dostatečné pro jejich přímé sepnutí. N-MOS tranzistor bude řízen P. Na jeho gate bude sériově připojen rezistor o hodnotě 220 Ω pro omezení proudových špiček na výstupu P. Mezi gate a společnou zem bude paralelně připojen pulldown rezistor o hodnotě 10 kΩ, který zabrání vzniku nedefinovaného logického stavu. Drain bude připojen k akumulátoru, pull-up rezistoru 10 kΩ a na gate obou P-MOS tranzistorů. Source bude připojen ke společné zemi.

Důvodem použití dvou P-MOS tranzistorů namísto jednoho je přítomnost parazitní diody, která je nedílnou součástí jejich vnitřní struktury. Pokud by byl použit pouze jeden tranzistor, protékal by večer i při jeho rozepnutí proud z akumulátoru zpět do solárního panelu prostřednictvím této diody, což by způsobovalo samovolné vybíjení akumulátoru.

Modul INA219 bude zapojen v krabičce K mezi akumulátor a stabilizátor. Jedná se o proudový i napěťový senzor s klidovým proudem v řádu jednotek mikroampér, který bude snímat napětí akumulátoru.

Na základě údajů z modulu INA219 bude P prostřednictvím sběrnice I²C vyhodnocovat stav akumulátoru. Mohou nastat tři situace. První možností je slunečný den, kdy bude solární panel dodávat napětí vyšší než 7,2 V a akumulátor bude plně nabitý. Druhou možností je noc, kdy panel nebude dodávat žádné napětí a dojde ke vzniku zpětného proudu směrem do panelu. Třetí možností je kritické vybití akumulátoru při poklesu napětí přibližně k hodnotě 5,4 V. V závislosti na těchto stavech P odpojí napájení nebo v případě kritického vybití vypne všechny nepotřebné části systému a přejde do co nejhlubšího režimu spánku.

Modul INA219 bude současně využíván jako proudový senzor. P bude monitorovat proud odebíraný z akumulátoru. Náhlé zvýšení proudu během pohybu dvířek bude indikovat jejich zablokování nebo náraz do překážky, například slepice. V takovém případě se P zastaví na 250 ms a pokusí se obrátit směr otáčení motoru. Pokud ani změna směru problém nevyřeší, systém se vypne a odešle zprávu o nefunkčnosti dvířek. Bude nutné brát v potaz krátkodobé proudové špičky vznikající při rozběhu motoru.

P bude dále prostřednictvím sběrnice I²C řídit H-můstek s nízkým klidovým proudem. H-můstek bude vybaven elektrolytickým kondenzátorem 47 µF / 25 V zapojeným co nejblíže mezi piny VM a GND, který bude potlačovat indukční napěťové špičky vznikající při vypínání motoru. Ten bude odrušen keramickým kondenzátorem 100 nF zapojeným přímo mezi jeho vývody a dvěma keramickými kondenzátory 47 nF zapojenými mezi jednotlivé vývody a kostru motoru (Faradayova klec). Všechny kondenzátory budou dimenzovány na napětí 50 V. Toto odrušení je nezbytné pro omezení jiskření kartáčků a potlačení vysokofrekvenčního elektromagnetického rušení. H-můstek i elektromotor budou umístěny v krabičce K.

Optická závora není pro počítání vajec v kurníku vhodná mimo jiné z důvodu výskytu prachu, peří, podestýlky a zvědavých slepic. Všechny tyto objekty by mohly přerušit laserový paprsek a být chybně vyhodnoceny jako propadnuté vejce. Z tohoto důvodu bude pro detekci vajec využit tenzometr.

Stíněný kabel od tenzometru bude připojen k modulu AČ převodníku HX711 s nízkým klidovým proudem v řádu jednotek mikroampér, umístěnému v krabičce Kx. Převodník bude použit z důvodu velmi nízkého výstupního napětí tenzometru, které se pohybuje v řádu jednotek milivoltů. Stínění kabelu bude připojeno ke společné zemi za účelem odvodu šumu.

Modul HX711 bude připojen k Px, který bude schopen převodník uspat a tím současně odpojit napájení tenzometru.

Px bude pro komunikaci s P prostřednictvím datového kabelu typu UTP využívat sběrnici RS485. První kroucený pár bude sloužit k přenosu napájení, přičemž oba vodiče budou zapojeny paralelně. Druhý pár bude stejným způsobem použit pro propojení společné země. Třetí pár bude přenášet data prostřednictvím čipu MAX3485 s keramickým kondenzátorem 100 nF / 16 V zapojeným co nejblíže mezi piny VCC a GND. Čip bude u prototypu připájen k adaptéru SOIC-8 na DIP-8 a bude sloužit jako transceiver sběrnice RS485. Jeden čip bude před P a druhý před Px.

Na rozdíl od sběrnice I²C je RS485 vhodná pro komunikaci na delší vzdálenosti. Pro přenos dat bude využit protokol LPUART, který nevyužívá hodinový signál a vyznačuje se nízkou spotřebou energie. Čip MAX3485 vytváří diferenciální signál na linkách A B, čímž zvyšuje odolnost komunikace proti elektromagnetickému rušení. Současně jej bude možné prostřednictvím Px uvést do režimu spánku, ve kterém bude odebírat pouze proud v řádu jednotek mikroampér. Vše od tenzometru až po čip MAX3485 musí být co nejblíže k sobě.

Vzhledem k použití několika snáškových hnízd bude komunikace probíhat mezi jednou řídicí jednotkou P (master) a několika jednotkami Px (slave), které budou propojeny sériově v topologii Daisy Chain. Vzhledem ke krátké délce vedení v řádu jednotek metrů nebude nutné na začátek ani konec sběrnice připojovat zakončovací rezistory o hodnotě 120 Ω pro impedanční přizpůsobení vedení. Jejich použití by pouze zvyšovalo proudový odběr systému.

Na aplikační vrstvě bude použit komunikační protokol Modbus RTU a knihovna ModbusRTU-Slave. Protokol Modbus RTU vytvoří datový rámec obsahující adresu jednotky slave, přenášená data a kontrolní součet CRC pro detekci chyb při přenosu. Hardware LPUART v mikrokontroléru následně převede jednotlivé bajty na sériový datový tok, doplní start a stop bity a zajistí jejich přenos po sběrnici. Na straně přijímače proběhne opačný proces.

K čipům P a Px budou na finálních deskách plošných spojů přímo k pinům VDDA a VSSA paralelně připojeny odrušovací keramické kondenzátory o kapacitách 10 µF / 25 V a 100 nF / 50 V pro filtraci vysokofrekvenčního šumu. Desky plošných spojů budou obsahovat souvislou zemní plochu připojenou k zápornému pólu solárního panelu a akumulátoru.

### Řízení
Hlavní řídicí jednotkou systému bude mikrokontrolér STM32 s integrovaným LoRa modulem, komunikujícím na frekvenci 868 MHz. LoRa modul umožní komunikaci na velké vzdálenosti při nízké spotřebě energie. U každého snáškového hnízda bude umístěn další mikrokontrolér STM32.

Firmware bude vyvíjen v prostředí STM32CubeIDE. Součástí firmwaru hlavní řídicí jednotky budou astronomické hodiny, podle kterých se budou automaticky otevírat a zavírat dvířka. Použití časovače nebylo zvoleno z důvodu proměnlivé délky dne. Světelný senzor by mohl způsobovat chybné sepnutí při zatažené obloze (déšť nebo bouřka) nebo ve večerních hodinách vlivem pouličního osvětlení či světlometů automobilů.

Použité čipy podporují režim hlubokého spánku (Standby), který umožňuje výrazně snížit spotřebu energie. Klidový proud se v tomto režimu pohybuje v řádu jednotek mikroampér. Hlavní řídicí jednotka se bude společně s nezbytnými částmi systému probouzet každých 10 minut za účelem kontroly stavu solárního panelu a akumulátoru. Dále se bude společně s ostatními řídicími jednotkami a dalšími potřebnými částmi systému probouzet 24× denně, tedy každou hodinu, pro kontrolu stavu vajec. Nakonec se bude probouzet ráno a večer, opět pouze s nezbytnými částmi systému, za účelem otevření a zavření dvířek.

Pro vývoj budou použity vývojové desky stejného nebo podobného typu, jako budou čipy použité na finální desce plošných spojů. Finální deska bude obsahovat čtyřpinovou lištu pro připojení programátoru z vývojové desky.

K hlavnímu čipu bude připojena anténa pro LoRa pásmo ve formě měděného vodiče o délce 8,2 cm (čtvrtvlnný monopól), například z běžného UTP kabelu. Anténa bude připájena přímo k desce plošných spojů a může zůstat uvnitř krabičky. Musí však být umístěna co nejdále od akumulátoru a motoru. V prostoru o poloměru 2 cm kolem antény ani pod ní se nesmí vyskytovat měď ani žádné elektronické součástky.

V domě bude umístěna vývojová deska ESP32 s integrovaným LoRa modulem a anténou, která bude plnit funkci internetové brány. Veškerá data přijatá touto deskou budou následně odesílána do cloudové databáze.

## Nákup

**Kabeláž**  
https://www.gme.cz/v/1512357/elektrokabel-cya-1x15-cerny-h07v-k-izolovany-vodic-lanko  
https://www.gme.cz/v/1512360/elektrokabel-cya-1x05-cerny-h05v-k-izolovany-vodic-lanko  
https://www.gme.cz/v/1512358/elektrokabel-cya-1x15-cerveny-h07v-k-izolovany-vodic-lanko  
https://www.gme.cz/v/1512382/elektrokabel-cya-1x05-cerveny-h05v-k-izolovany-vodic-lanko  
  
https://konektor-kabel.cz/liycy-4x025-mm2-stineny-od-48-mm-sedy-helu  
  
https://www.alza.cz/alzapower-patch-cat5e-utp?dq=6592131  

**Bužírky**  
https://www.laskakit.cz/smrstovaci-pajeci-spojka--buzirka-s-cinem/ (bílé)  
  
Electric-Hranice s.r.o.  

**Svorkovnice**  
https://www.laskakit.cz/sroubovaci-svorkovnice-do-dps-kf128-2-54/  
https://www.laskakit.cz/sroubovaci-svorkovnice-do-dps-kf301-2p/  

**Klipy**  
Hobby market (Hornbach)  

**Konektory**  
https://www.hadex.cz/p/d626-zdirka-rj45-do-dps-8p8c  

**Solární panel**  
https://www.dexhal.cz/fotovoltaicky-panel-6v-1670ma-10w-solarni-clanek_z2899/  

**N-MOS**  
https://www.gme.cz/v/1494221/infineon-irfz44npbf-unipolarni-tranzistor

**P-MOS**  
https://www.gme.cz/v/1493257/infineon-irf4905pbf-unipolarni-tranzistor  

**Akumulátor**  
https://www.levne-baterky.cz/Green-Cell-AGM-Baterie-6V-7Ah-d5513.htm  

**Lineární LDO regulátor**  
https://www.laskakit.cz/modul-stabilizatoru-napeti-ams1117-3-3v/  

**Brána**  
https://www.hadex.cz/p/m378d-vyvojovy-modul-heltec-lora-32-wifi-bluetooth-v3-2  

**Master (vývojová deska a čip)**  
https://cz.mouser.com/ProductDetail/STMicroelectronics/NUCLEO-WL55JC1?qs=zW32dvEIR3uxo98jZ7%2FN0g%3D%3D  
https://cz.mouser.com/ProductDetail/STMicroelectronics/STM32WLE5CCU6?qs=26oI9%252Bq63sLM90C6xjPI1g%3D%3D&utm_id=9873308831&utm_source=google&utm_medium=cpc&utm_marketing_tactic=emeacorp  

**Slave (vývojová deska a čip)**  
https://cz.mouser.com/ProductDetail/STMicroelectronics/NUCLEO-U031R8?qs=sGAEpiMZZMv0WPLDnYsI70d1YaGuDSdFrBEvuBfrX2oCPZoXXcsbiw%3D%3D  
https://cz.mouser.com/ProductDetail/STMicroelectronics/STM32G031G8U6?qs=uwxL4vQweFO0dXSA%2FIGJ2w%3D%3D  

**Headery**  
https://www.laskakit.cz/dupont-40pin-2-54-mm-pinovy-pas/  
https://www.laskakit.cz/dupont-2x40pin-2-54-mm-pinovy-pas/  

**Proudový a napěťový senzor**  
https://dratek.cz/arduino-platforma/1437-ina219-proudovy-snimac-obousmerny.html  

**H-můstek**  
https://botland.cz/ovladace-stejnosmerneho-motoru/2695-drv8838-jednokanalovy-budic-motoru-11v-17a-pololu-2990-5903351244855.html  

**Motor**  
https://dratek.cz/arduino-platforma/3197-stejnosmerny-motor-s-prevodovkou-dc-6v-20-rpm.html  

**Mikrospínače**  
https://www.hadex.cz/p/l405a-mikrospinac-kw11-3z-on-on-1pol-250v-5a-s-packou-13mm  

**Dvířka**  
https://eshop.zenit.cz/desky/pvc-penene-desky/palight-print-8mm/variant/2030-3050/  

**Hnízdo**  
https://www.lihneme.cz/plastova-snaskova-hnizda/hf44x37x50-zelene/  

**Tenzometr**  
https://www.laskakit.cz/vazici-senzor-20kg-adc-modul-hx711/  

**Převodník**  
https://www.laskakit.cz/ad-prevodnik-modul-24-bit-2-kanaly-hx711/  

**Transceiver**  
https://cz.mouser.com/ProductDetail/Analog-Devices-Maxim-Integrated/MAX3485EESA+?qs=1THa7WoU59HLsdsozrMCeQ%3D%3D  

**Adaptér**  
https://dratek.cz/arduino-platforma/1046-dps-adapter-sop8-so8-soic8-na-dip8.html  

**Překližky**  
Hobby market (Hornbach)  

**Červík**  
Fabory Přerov  

**Vložka**  
https://www.majkl3d.cz/p/zavitova-vlozka-do-plastu-mosaz-m3  

**Lanko**  
Hobby market (Hornbach)  

**Očko Šrouby Podložka Matice Vruty**  
Hobby market (Hornbach)  

**PU tmel**  
Hobby market (Hornbach)  
