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
Celý systém bude umístěn pod stříškou, směrem na jih (popř. na východ nebo západ), na vnější straně kurníku. Tudíž panel nikdy nezasněží, využiji co možná nejvíce sluneční energie a z velké části eliminuji vliv amoniaku ze slepičího trusu.

### Napájení
Solární panel bude společně s akumulátorem, jež bude umístěn v první krabičče, celý systém zásobovat elektřinou. Krabička bude disponovat dírami s gumovými kabelovými průchodkami.

### Řízení
Hlavní řídicí jednotkou v kurníku bude mikrokontrolér STM32 se zabudovaným LoRa modulem. Programování bude probíhat v prostředí STM32CubeIDE. Tento mikroprocesor dokáže přejít do tzv. režimu hlubokého spánku (Stand By) a tím ušetřit velké množství energie (klidový proud je v řádu nízkých jednotek mikroampér). Je ale potřeba ho napájet lineárním LDO regulátorem s ultra nízkým klidovým proudem (nízké jednotky mikroampér), protože stepdown měniče mají vysoký klidový proud (jednotky miliampér), takže by byl režim hlubokého spánku k ničemu. LoRa modul umožní komunikaci na velké vzdálenosti při nízké spotřebě energie. Pro vývoj bude použita vývojová deska stejného typu jako bude čip na finální verzi. U čipu bude potřeba přimontovat anténu. u každého snáškového hnízda pak bude mikrokontrolér STM32.

### DPS
Deska plošných spojů s veškerou elektronikou se bude nacházet v druhé krabičce s anténní veží a 2 cm trubičkou pro lanko

### Mechanika
Na stěnu kurníku budou pomocí vrutů přišrubovány dřevěné hranolky, po jejichž bocích budou přivrtány plechové drážky z hliníku (opět pomocí vrutů), ve kterých budou jezdit bílá plastová dvířka. Na přední straně dvířek nahoře bude umístěno závěsné očko, kterým bude prostrčeno lanko, zajištěno dračí smyčkou. Tohle lanko povede do druhé krabičky, ve které bude špulka, jejímž tělem bude uprostřed skrz díru provlečeno a upevněno pomocí osmičkového uzlu. Špulka bude přes stavěcí šroub (červíka) přimontována ke hřídeli motorku, která povede k jejímu středu.

## Prototyp
**Krabičky  
3D tisk**  

**Solární panel  
6V 5W**  
https://www.laskakit.cz/solarni-panel-monokrystalicky-6v-5w-s-otvory/  

**Solární regulátor napájení  
6V/12V PWM s automatickou detekcí**  
https://allegro.cz/produkt/regulator-nabijeni-solarni-regulator-6v-12v-3a-6e53239d-c373-4be7-aee4-d630c05dd007?offerId=13463401801  

**Akumulátor  
6V 7Ah olověný AGM bezúdržbový**  
https://www.levne-baterky.cz/Green-Cell-AGM-Baterie-6V-7Ah-d5513.htm  

**Motor  
6V DC s kovovou převodovkou 20 RPM s hřídelí tvaru D**  
https://dratek.cz/arduino-platforma/3197-stejnosmerny-motor-s-prevodovkou-dc-6v-20-rpm.html  

**Master  
Iq = 2,1uA**  

**Slave**  

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

**Dveře  
pěněné PVC 8mm rozměry**  
https://eshop.zenit.cz/desky/pvc-penene-desky/palight-print-8mm/variant/2030-3050/  
