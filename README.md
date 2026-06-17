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

V rámci semestrálního projektu řešte body 1. – 4.  
Pozn. Předpokládá se, že student má možnost otestovat zařízení v reálném nasazení.

## Koncept

## Krabičky
Systém bude obsahovat 2 krabičky. Jednu pro akumulátor a druhou (K1) pro mechaniku a elektroniku dvířek a hlavní procesor (P1). Tyto krabičky budou disponovat dírami s gumovými kabelovými průchodkami a KE bude mít zespodu ještě navíc 2 cm trubičku pro lanko.

### Napájení
Solární panel (6V 10W) bude připevněný svisle na zeď, umístěný pod malou stříškou a orientovaný na jih (popř. východ, západ nebo při žádné jiné možnosti na sever). Tím se docílí toho, že nikdy nezasněží a využije co možná nejvíce sluneční energie. Společně s olověným bezúdržbovým akumulátorem AGM (6V 7Ah) budou celý systém zásobovat elektřinou. Snížení napětí z 6 na 3,3V bude realizováno pomocí LDO regulátoru s nízkým klidovým proudem (jednotky mikroampér), ke kterému bude paralelně připojen vstupní a výstupní elektrolytický filtrační kondenzátor s parametry 10uF a minimálně 16V. Buck měniče nejsou vhodné kvůli horší dostupnosti modelu s nízkým Iq a malému odběru systému po většinu dne. Jejich vysoká učinnost by se uplatnila pouze po dobu přibližně 2 minut a po zbytek dne by měli paradoxně nižší účinnost než jednoduché LDO stabilizátory.  

### Řízení
Hlavní řídicí jednotkou v kurníku bude mikrokontrolér STM32 se zabudovaným LoRa modulem. LoRa modul umožní komunikaci na velké vzdálenosti při nízké spotřebě energie. U každého snáškového hnízda pak bude další mikrokontrolér STM32. Programování bude probíhat v prostředí STM32CubeIDE. Částí firmwaru hlavní jednotky budou astronomické hodiny, podle kterých se budou otevírat a zavírat dvířka. Tyto mikroprocesory dokáží přejít do tzv. režimu hlubokého spánku (Stand By) a tím ušetřit velké množství energie (klidový proud je v řádu jednotek mikroampér). Pro vývoj budou použity vývojové desky stejného typu jako budou čipy na finálních desce. Finální deska bude mít vyvedenou čtyř kolíkovou lištu k připojení programátoru (z vývojové desky). K hlavnímu čipu bude potřeba připojit LoRa anténu ve formě ustřiženého měděného drátu (8.2mm) z klasického UTP kabelu. Ten se připájí k plošnému spoji a může zůstat uvnitř krabičky. Anténa se ale musí nacházet co nejdále od baterie a motorku a 2cm kolem a pod ní nesmí být žádná měď a součástky.

### Elektronika
Prototyp bude složen z modulů, ale finální verze bude mít pouze jednu DPS. Silová část bude dimenzována na 6V a elektronika na 3,3V. Ze solárního panelu povede kabel do vysoko-impedačního děliče složeného z rezistorů o hodnotách 1Mohm a 470kohm. Tento dělič bude snímat napětí na panelu, které bude odesílat P1. Vysoká impedance zajistí odběr v řádu jednotek mikroampér. Z děliče se půjde na P-MOS tranzistor, který bude plnit funkci spínače celého systému. Tento tranzistor musí být ale spínaný N-MOS tranzistorem, protože by ho 3,3V procesor sepnout nezvládl. Při slunečných dnech, kdy bude panel vyrábět přebytek energie a baterie bude nabitá je potřeba ho odpojit (přepěťová ochrana). Zároveň je potřeba panel odpojit i přes noc, kvůli zpětnému proudu z baterie (Přepólování). Tohle bude vyhodnocovat P1, na základě dat z děliče a z modulu INA219 (resp. příslušné části finální DPS). Toto proudové, ale i napěťové čidlo s nízkým klidovým proudem (jednotky mikroampér) bude připojeno

### Mechanika
Hlavní část sytému bude umístěna na venkovní stěně kurníku, která bude v souladu s požadavky pro solární panel, již jsou uvedeny v sekci napájení. Tímto snížím komplexnost montáže a z velké části eliminuji vliv amoniaku ze slepičího trusu. Pomocí vrutů budou na stěnu přišrubovány dřevěné hranolky, po jejichž bocích budou přivrtány plechové drážky z hliníku (opět pomocí vrutů), ve kterých budou jezdit bílá pěněná PVC dvířka o rozměrech x a tloušťce 8mm. Na přední straně dvířek nahoře bude umístěno závěsné očko, kterým bude prostrčeno lanko, zajištěno dračí smyčkou. Tohle lanko povede do K1, ve které bude špulka, jejímž tělem bude uprostřed skrz díru provlečeno a upevněno pomocí osmičkového uzlu. Špulka bude přes stavěcí šroub (červíka) přimontována ke hřídeli DC nízkootáčkového motorku s kovovou převodovkou a hřídelí tvaru D (6V).

## Prototyp
**Krabičky  
3D tisk**  

**Solární panel**  
https://www.dexhal.cz/fotovoltaicky-panel-6v-1670ma-10w-solarni-clanek_z2899/  

**Akumulátor**  
https://www.levne-baterky.cz/Green-Cell-AGM-Baterie-6V-7Ah-d5513.htm  

**Lineární LDO regulátor**  
https://www.laskakit.cz/holtek-ht7833-3-3v-0-5a-stabilizator--sot-89/?utm_source=google&utm_medium=cpc&utm_campaign=2_SEA_DSA_%5BCZ%5D_tROAS_400%2F500&utm_id=1371577095  

**Proudový senzor INA219**  
https://www.laskakit.cz/laskakit-ina219-proudovy-senzor/  

**Svorkovnice**  
https://www.laskakit.cz/sroubovaci-svorkovnice-do-dps-kf128-2-54/?variantId=8868  

**Motor**  
https://dratek.cz/arduino-platforma/3197-stejnosmerny-motor-s-prevodovkou-dc-6v-20-rpm.html  

**Master (vývojová deska a čip)**  

**Slave (vývojová deska a čip)**  

**Špulka  
3D tisk (PETG ASA ABS nylon)**  
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
