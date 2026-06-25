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
Systém bude obsahovat jednu krabičku pro akumulátor a druhou (K) pro mechaniku a elektroniku dvířek a hlavní mikrokontrolér (P). Dále bude pro každé snáškové hnízdo určena jedna krabička (Kx). Tyto krabičky budou vytisknuty na 3D tiskárně a budou disponovat otvory s gumovými kabelovými průchodkami. Krabička K bude mít zespodu ještě navíc 2cm trubičku pro lanko.

### Kabely
Pro silové rozvody (solární panel, akumulátor, H-můstek a motor) bude použita měděná ohebná licna o průřezu 1,5mm². Pro napájení elektroniky a signálová vedení bude použita měděná ohebná licna o průřezu 0,5mm². Pro kladnou větev bude použit červený vodič a pro zápornou větev černý vodič. Jako sdělovací kabel byl zvolen UTP CAT5E, který je zcela dostačující.

### Napájení
Solární panel (6V 10W) bude připevněn svisle na zeď, umístěn pod malou stříškou a orientován na jih (popř. východ, západ nebo při žádné jiné možnosti na sever). Tím se docílí toho, že nikdy nezapadne sněhem a využije co možná nejvíce sluneční energie.

Společně s olověným bezúdržbovým akumulátorem AGM (6V 7Ah) budou celý systém zásobovat elektřinou. Silová část bude dimenzována na 6V (motorek) a veškerá elektronika na 3,3V.

Snížení napětí z 6V na 3,3V bude realizováno pomocí LDO regulátoru s nízkým klidovým proudem (jednotky mikroampér).

Buck měniče nejsou vhodné kvůli horší dostupnosti modelů s nízkým Iq a malému odběru systému po většinu dne. Jejich vysoká účinnost by se uplatnila pouze po dobu přibližně 2 minut a po zbytek dne by měly paradoxně nižší účinnost než jednoduché LDO stabilizátory.

### Řízení
Hlavní řídicí jednotkou v kurníku bude mikrokontrolér STM32 s integrovaným LoRa modulem, komunikujícím na frekvenci 868MHz. LoRa modul umožní komunikaci na velké vzdálenosti při nízké spotřebě energie. U každého snáškového hnízda pak bude další mikrokontrolér STM32.

Programování bude probíhat v prostředí STM32CubeIDE. Součástí firmwaru hlavní jednotky budou astronomické hodiny, podle kterých se budou otevírat a zavírat dvířka. Časovač jsem se rozhodl nepoužít z důvodu proměnné délky dne a světelný senzor by se zase mohl spínat při zatažených dnech (deště, bouřky) nebo večer vlivem pouličního osvětlení či světlometů aut.

Tyto mikrokontroléry dokáží přejít do tzv. režimu hlubokého spánku (Standby), a tím ušetřit velké množství energie (klidový proud je v řádu jednotek mikroampér). Hlavní jednotka se bude spolu s dalšími potřebnými částmi systému probouzet každých 10 minut pro zkontrolování stavu panelu a baterie. Dále se bude společně s ostatními řídicími jednotkami a dalšími potřebnými částmi systému probouzet 24× denně, a to právě každou hodinu, pro zkontrolování stavu vajec. A nakonec se zapne ráno a večer, opět pouze s dalšími potřebnými částmi systému, pro otevření a zavření dvířek.

Pro vývoj budou použity vývojové desky stejného/podobného typu, jako budou čipy na finální desce. Finální deska bude mít vyvedenou čtyřkolíkovou lištu k připojení programátoru (z vývojové desky).

K hlavnímu čipu bude potřeba připojit anténu pracující na LoRa frekvenci ve formě ustřiženého měděného drátu dlouhého 8,2cm (čtvrtvlnný monopól), třeba z klasického UTP kabelu. Ten se připájí k plošnému spoji a může zůstat uvnitř krabičky. Anténa se ale musí nacházet co nejdále od baterie a motorku a 2cm kolem a pod ní nesmí být žádná měď ani součástky.

V domě bude potom vývojová deska ESP32 se zabudovaným LoRa modulem a anténou, zastávající roli internetové brány. Veškerá data, která tato deska přijme, odešle do cloudového úložiště.

### Mechanika
Hlavní část systému bude umístěna na venkovní stěně kurníku, která bude v souladu s požadavky pro solární panel, jež jsou uvedeny v sekci napájení. Tímto snížím komplexnost montáže a z velké části eliminuji vliv amoniaku ze slepičího trusu.

Pomocí vrutů budou na stěnu přišroubovány dřevěné hranolky, po jejichž bocích budou přivrtány plechové drážky z hliníku (opět pomocí vrutů), ve kterých se budou svisle pohybovat bílá pěněná PVC dvířka o rozměrech x a tloušťce 8mm.

Na přední straně dvířek nahoře bude umístěno závěsné očko, upevněné zapuštěnou podložkou a maticí. Tímto očkem bude prostrčeno lanko, zajištěné dračí smyčkou. Toto lanko povede nahoru do K, ve které bude špulka z PETG, vytištěná na 3D tiskárně, jejímž tělem bude uprostřed skrz otvor provlečeno a upevněno pomocí osmičkového uzlu. Špulka bude pomocí stavěcího šroubu (červíku) připevněna ke hřídeli tvaru D nízkootáčkového stejnosměrného motorku s kovovou převodovkou (6V).

V horní a dolní části hranolku budou zapuštěné 2 mechanické koncové mikrospínače (pákové). Jak dvířka pojedou nahoru nebo dolů, tak sepnou, resp. rozepnou tyto spínače, které budou pinem NO připojeny na společnou zem a pinem COM do P.

Před pohybem dvířek se piny koncových spínačů aktivují (digitální vstup s pull-up rezistorem). Po dosažení koncové polohy se piny deaktivují přepnutím do analogového režimu, čímž se eliminuje jejich klidový odběr.

### Elektronika
Prototyp bude složen z modulů, umístěných na nepájivé pole pomocí kolíkových lišt a svorek s kovovým plíškem pro ochranu licny. Finální verze bude mít pouze jednu hlavní DPS a x vedlejších DPS pro každé snáškové hnízdo. Jako rozhraní budou použity svorky a konektory RJ45.

Na panel bude připojen vysokoimpedanční dělič, složený z rezistorů o hodnotách 1MΩ a 470kΩ. Na rezistor R2, tedy 470kΩ, bude paralelně připojen keramický kondenzátor s parametry 100nF 16V pro správné měření při takto velké impedanci. Tento dělič bude snímat napětí na panelu, které bude odesílat do P. Vysoká impedance zajistí odběr v řádu jednotek mikroampér.

K panelu budou taktéž sériově (drainy k sobě) připojeny 2 P-MOS tranzistory, které budou plnit funkci výkonového spínače celého systému. Oba musí být ale spínány přes jeden N-MOS tranzistor, protože by je 3,3V mikrokontrolér sám sepnout nezvládl. Ten bude potom spínán přes P. Na gate N-MOSu bude sériově připojen 220Ω odpor z důvodu ochrany P proti proudovým špičkám. Paralelně mezi gate a společnou zem bude potom připojen 10kΩ pulldown rezistor z důvodu zamezení neurčitého stavu. Drain bude připojen na baterii, pullup rezistor 10kΩ a gate obou P-MOSů. Source potom na zem.

Důvodem použití 2 P-MOSů namísto jednoho je parazitní dioda, která je pevnou součástí křemíkového čipu. Kdyby byl P-MOS pouze jeden, tak by večer i přes jeho rozpojení přes tuto parazitní diodu protékal proud z akumulátoru zpátky do panelu, a tím by se baterie vybíjela.

Modul INA219 (resp. čip INA219 s externími součástkami) je proudové, ale i napěťové čidlo s nízkým klidovým proudem (jednotky mikroampér), které bude zapojeno mezi akumulátor a stabilizátor, kde bude snímat napětí na baterii.

Podle údaje o napětí od INA219 bude P přes sběrnici I2C informován o tom, jak moc je baterie nabitá, resp. vybitá. Mohou nastat 3 případy. Buď bude panel ve slunečný den přivádět napětí vyšší než 7,2V a baterie bude plně nabitá (přepětí), panel nebude přivádět žádné napětí (noc) a z baterie do něj začne proudit zpětný proud (přepólování), nebo bude baterie kriticky vybitá (blížit se 5,4V). P tudíž buď odpojí napájení, nebo v posledním uvedeném případě vše, co může, vypne a co nejhlouběji uspí.

INA219 je také, jak jsem již zmiňoval, senzorem proudu. Této vlastnosti bude využívat P, když se najednou proud dodávaný z baterie zvýší. To bude značit, že dvířka při zavírání narazila na slepici, a P okamžitě obrátí chod (pozor na falešné špičky způsobené rozběhem a vypnutím motorku).

Je tedy zřejmé, že P bude taktéž řídit H-můstek (resp. čip s externími součástkami) s nízkým klidovým proudem (jednotky mikroampér), a to přes sběrnici I2C. H-můstek s elektrolytem 47µF 25V (kvůli indukčním špičkám při vypínání motorku), zapojeným těsně paralelně k VM a GND, bude zase řídit elektromotorek, jenž bude odrušený 100nF keramickým kondenzátorem zapojeným těsně mezi kontakty a dvěma 47nF keramickými kondenzátory zapojenými těsně mezi kontakty a kostru (Faradayova klec). Všechny tyto kondenzátory budou dimenzované na napětí 50V. Odrušení je potřeba kvůli jiskření kartáčků a filtraci vysokofrekvenčního rušení.

K čipu P budou na finálních deskách paralelně těsně k pinům VDDA a VSSA připojeny keramické odrušovací kondenzátory s parametry 10µF 25V a 100nF 50V, kvůli filtraci VF šumu. DPS budou mít společnou zem ze záporného pólu panelu a akumulátoru ve formě souvislé vrstvy mědi.

## Prototyp
**Krabičky**  

**Kabely**  
https://www.gme.cz/v/1512357/elektrokabel-cya-1x15-cerny-h07v-k-izolovany-vodic-lanko  
https://www.gme.cz/v/1512360/elektrokabel-cya-1x05-cerny-h05v-k-izolovany-vodic-lanko  
https://www.gme.cz/v/1512358/elektrokabel-cya-1x15-cerveny-h07v-k-izolovany-vodic-lanko  
https://www.gme.cz/v/1512382/elektrokabel-cya-1x05-cerveny-h05v-k-izolovany-vodic-lanko  
  
https://www.alza.cz/alzapower-patch-cat5e-utp?dq=6592131  

**Svorkovnice (prototyp, DPS)**  
https://www.laskakit.cz/sroubovaci-svorkovnice-do-dps-kf128-2-54/  
https://www.laskakit.cz/sroubovaci-svorkovnice-do-dps-kf301-2p/  

**Konektory**
https://www.hadex.cz/p/d626-zdirka-rj45-do-dps-8p8c  

**Klipy  
5mm hřebíkové**  
Hobby ḿarket (Hornbach)  

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

**Proudový senzor INA219**  
https://dratek.cz/arduino-platforma/1437-ina219-proudovy-snimac-obousmerny.html  

**Motor**  
https://dratek.cz/arduino-platforma/3197-stejnosmerny-motor-s-prevodovkou-dc-6v-20-rpm.html  

**H-můstek**  
https://botland.cz/ovladace-stejnosmerneho-motoru/2695-drv8838-jednokanalovy-budic-motoru-11v-17a-pololu-2990-5903351244855.html  

**Koncák**  
https://www.hadex.cz/p/l405a-mikrospinac-kw11-3z-on-on-1pol-250v-5a-s-packou-13mm  

**Špulka**  
2cm šířka  
2,5cm vnější průměr  
3mm vnitřní průměr  
3mm díra  

**Červík  
M3 nerez A2**  
Železářství

**Lanko  
2mm syntetika (zednická stavební šňůra)**  
Hobby ḿarket (Hornbach)  

**Očko  
M5 (M6) nerez A2**  
Hobby ḿarket (Hornbach)  

**Podložka  
M5 (M6) nerez A2**  
Hobby ḿarket (Hornbach)  

**Matice  
M5 (M6) nerez A2**  
Hobby ḿarket (Hornbach)  

**Dveře**  
https://eshop.zenit.cz/desky/pvc-penene-desky/palight-print-8mm/variant/2030-3050/  

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
