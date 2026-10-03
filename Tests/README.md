# Testovací data

Databáze, kterými se dá nástěnka naplnit bez kurníku a brány. Každá ji uvede do jiného
stavu, takže jde vyzkoušet graf, tabulku i všechny hlášky, aniž by se čekalo, až stav
nastane doopravdy.

## Co je potřeba

- **Nastavený server** podle `Server/README.md` — stačí vyplněný `.env`, kurník ani brána ne.
- **`TTN_DEVICE_ID=lora-e5-mini`** v `.env`, protože pod tímhle zařízením jsou měření
  uložená. S jiným názvem zůstane nástěnka prázdná.
- **Dvě hnízda**, tedy `NEST_COUNT=2` nebo žádný řádek `NEST_COUNT` — data jsou pro dvě.
  Výjimkou je `kurnik-test-velky.db`, ta potřebuje `NEST_COUNT=15`. S jiným počtem se nic
  nerozbije: přebývající hnízda ukazují pomlčku bez poznámky, chybějící se nezobrazí.
- **Node.js 22** jen tehdy, když budete data přegenerovávat.

## Jak databázi nasadit

Server je k tomu potřeba zastavit, jinak může část posledních měření zůstat v pomocném
souboru. Nejdřív si odložte, co máte uložené, pak nakopírujte testovací databázi:

```bash
cd ~/ChickenCoop/Server
docker compose stop
docker compose cp app:/data/kurnik.db ~/kurnik-zaloha.db
docker compose cp ~/ChickenCoop/Tests/kurnik-test.db app:/data/kurnik.db
docker compose start
```

Kopíruje se celý soubor a uloží se pod jménem z cílové cesty, takže se z `kurnik-test.db`
uvnitř stane `kurnik.db` a původní databázi přepíše. Měření, která z kurníku dorazí potom,
se ukládají do té testovací.

Když server pouštíte přímo přes `node src/server.js`, je to prosté kopírování souboru, na
který ukazuje `DB_PATH` (ve výchozím nastavení `Server/data/kurnik.db`) — i tak server
nejdřív zastavte.

## Jak se vrátit ke svým datům

Stejný postup obráceně:

```bash
cd ~/ChickenCoop/Server
docker compose stop
docker compose cp ~/kurnik-zaloha.db app:/data/kurnik.db
docker compose start
```

Kdyby mezitím vypadl proud nebo se Pi vypnulo natvrdo, zůstanou v úložišti vedle databáze
ještě soubory `kurnik.db-wal` a `kurnik.db-shm` z testovací databáze. SQLite by je na
obnovenou zálohu přehrál a poškodil ji, proto se se zastaveným serverem nejdřív podívejte,
co v úložišti leží:

```bash
docker run --rm -v server_coop-data:/data alpine ls -l /data
```

Je tam jen `kurnik.db`? Kopírujte zálohu podle postupu výše. Je tam i `-wal` nebo `-shm`?
Nejdřív je smažte:

```bash
docker run --rm -v server_coop-data:/data alpine rm -f /data/kurnik.db-wal /data/kurnik.db-shm
```

Po běžném `docker compose stop` tam nic takového nebude. Celé zálohování je popsané
v `Server/README.md`.

## Co je co

| Soubor | Poslední měření | Na co se dívat |
|---|---|---|
| `kurnik-test.db` | běžný stav | dva roky měření, všechny události v grafech |
| `kurnik-test-poplach.db` | mrtvé čidlo baterie i panelu, neznámá dvířka, kvočna v hnízdě 1, hnízdo 2 neodpovídá | pět oranžových hlášek naráz; hnízdo 1 odpovídá, takže řetěz se přerušil až za ním |
| `kurnik-test-porucha.db` | kriticky vybitá baterie, dvířka v poruše, porucha váhy v hnízdě 1 | tři červené hlášky: „⚠ kriticky vybitá", „⚠ čeká na odblokování" a „⚠ porucha váhy"; hnízdo 2 počítá dál |
| `kurnik-test-meze.db` | baterie 8,00 V, panel 12,50 V, obě hnízda plná | „na horní mezi rozsahu" u obou dlaždic, „⚠ košík je plný" u obou hnízd |
| `kurnik-test-instalace.db` | první den po instalaci, hnízdo 1 zkalibrované, hnízdo 2 ještě ne | „⚠ váha není zkalibrovaná" a pomlčka místo počtu; Snáška jen od kalibrace hnízda 1 |
| `kurnik-test-velky.db` | 15 hnízd, poslední kontrola ve všech stavech (`NEST_COUNT=15`) | karta Hnízda se všemi poznámkami, okno výběru hnízd, Snáška a její tabulka s 15 sloupci, telefon |
| `kurnik-test-stara.db` | před pěti dny | 24 h hlásí „Zatím žádná data", 7 dní kreslí |
| `kurnik-test-jedno.db` | jediné měření | graf s jediným bodem |
| `kurnik-test-prazdna.db` | žádné | prázdné dlaždice, 0 záznamů, vypnuté tlačítko Smazat |

Krátké databáze mají týden historie a kolem 120 kB (`kurnik-test-velky.db` 200 kB),
`kurnik-test-instalace.db` začíná před dvaceti hodinami a `kurnik-test.db` má dva roky a 8,7 MB.

U `kurnik-test-porucha.db` je stav hnízd z poslední kontroly před vybitím baterie: s kriticky
vybitou baterií kurník hnízda nekontroluje, takže karta Hnízda zůstane u poslední kontroly.

V `kurnik-test-instalace.db` jsou obě váhy od instalace nezkalibrované. Za dvě hodiny se
hnízdo 1 vynuluje a o dvacet minut později zkalibruje; obojí přijde jako zpráva o hnízdech
mimo celou hodinu, tak jako ze skutečného kurníku. Hnízdo 2 zůstane nezkalibrované.

V `kurnik-test-velky.db` je při poslední kontrole kvočna v hnízdě 4, plný košík v hnízdě 6,
nezkalibrovaná váha v hnízdě 8, porucha váhy v hnízdě 11 a hnízda 14 a 15 neodpovídají;
ostatní počítají normálně.

## Co je v grafech

Platí pro `kurnik-test.db`; krátké databáze mají z tohohle seznamu první dva řádky.

| Rozsah | Události v Napájení |
|---|---|
| 6 h, 24 h | tři zprávy bez hodnot (v tabulce pomlčky), výpadek 40 minut, dvě hodiny s baterií na 8,00 V a panelem na 12,50 V |
| 7 dní | výpadek 6 hodin, 8 hodin kriticky vybitá baterie, den s odpojeným panelem |
| 30 dní | výpadek 2 dny, 3 dny odpojený panel, 4 dny baterie na horní mezi |
| 1 rok | výpadek 9 dní, 2 dny panel na horní mezi, 5 dní kriticky vybitá baterie, 10 dní odpojený panel |
| Vše | výpadek 21 dní, 6 dní kriticky vybitá baterie |

Hnízda se kontrolují každou celou hodinu. Slepice snáší hlavně dopoledne, víc v létě než
v zimě, a do prvního hnízda chodí raději než do druhého; vejce se sbírají v 17 hodin, jen
některé dny se na to zapomene. Během kriticky vybité baterie se hnízda nekontrolují vůbec.

| Rozsah | Události v hnízdech (karta Hnízda a Snáška) |
|---|---|
| 24 h | 4 hodiny kvočna v hnízdě 2 (v tabulce Snášky pomlčky ve sloupci Hnízdo 2) |
| 7 dní | 6 hodin hnízda neodpovídají, tři dny bez sběru — první hnízdo se zaplní |
| 30 dní | 3 dny kvočna v hnízdě 2 |
| 1 rok | den porucha váhy v hnízdě 1 |
| Vše | prvních 5 hodin po instalaci váha nezkalibrovaná |

Ke každému rozsahu patří jinak dlouhý výpadek proto, že se v grafu projeví, až když
vyprázdní aspoň dvě okna toho rozsahu — v 7 dnech jsou okna hodinová, ve 30 dnech
šestihodinová a v roce denní.

## Přegenerování

Měření sahají dva dny do budoucnosti a časy událostí se počítají od okamžiku vzniku
databáze, takže za pár dní data zestárnou a nástěnka začne hlásit, že poslední zpráva
přišla před několika dny. Nová se vytvoří takto:

```bash
node Tests/seed.mjs
node Tests/seed.mjs plny poplach
```

První příkaz přepíše všechny databáze, druhý jen vyjmenované. Na výběr jsou `plny`,
`poplach`, `porucha`, `meze`, `instalace`, `velky`, `stara`, `jedno` a `prazdna`. Generátor u každé databáze
vypíše seznam událostí i s časy, takže je jasné, kam se v grafu dívat.

Hodnoty sedí na mřížce protokolu — baterie po 50 mV v rozsahu 5–8 V, panel po 100 mV do
12,5 V, nejvýš 10 vajec v hnízdě — takže vypadají přesně jako to, co dorazí z kurníku. Napětí panelu je měřené
naprázdno, protože firmware při měření rozepne odpojovač; přes den je proto kolem 10 až
11 V a v noci nulové.
