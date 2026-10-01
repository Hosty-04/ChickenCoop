# Testovací data

Databáze, kterými se dá nástěnka naplnit bez kurníku a brány. Každá ji uvede do jiného
stavu, takže jde vyzkoušet graf, tabulku i všechny hlášky, aniž by se čekalo, až stav
nastane doopravdy.

## Co je potřeba

- **Nastavený server** podle `Server/README.md` — stačí vyplněný `.env`, kurník ani brána ne.
- **`TTN_DEVICE_ID=lora-e5-mini`** v `.env`, protože pod tímhle zařízením jsou měření
  uložená. S jiným názvem zůstane nástěnka prázdná.
- **Node.js 22** jen tehdy, když budete data přegenerovávat.

## Jak databázi nasadit

Server čte soubor, na který ukazuje `DB_PATH`, ve výchozím nastavení `Server/data/kurnik.db`.
Kopírováním se přepíšou uložená měření, proto si je nejdřív odložte:

```bash
cp Server/data/kurnik.db Server/data/kurnik-zaloha.db
cp Tests/kurnik-test.db Server/data/kurnik.db
```

Pak server restartujte. Měření, která z kurníku dorazí potom, se ukládají do nasazené
databáze dál.

## Co je co

| Soubor | Poslední měření | Na co se dívat |
|---|---|---|
| `kurnik-test.db` | běžný stav | dva roky měření, všechny události v grafech |
| `kurnik-test-poplach.db` | kriticky vybitá baterie, mrtvé čidlo panelu, neznámá dvířka | červená hláška a dvě oranžové naráz |
| `kurnik-test-porucha.db` | mrtvé čidlo baterie, dvířka v poruše | „Porucha" a „⚠ čeká na odblokování" |
| `kurnik-test-meze.db` | baterie 8,00 V, panel 12,50 V | „na horní mezi rozsahu" u obou dlaždic |
| `kurnik-test-stara.db` | před pěti dny | 24 h hlásí „Zatím žádná data", 7 dní kreslí |
| `kurnik-test-jedno.db` | jediné měření | graf s jediným bodem |
| `kurnik-test-prazdna.db` | žádné | prázdné dlaždice, 0 záznamů, vypnuté tlačítko Smazat |

Krátké databáze mají týden historie a kolem 100 kB, `kurnik-test.db` dva roky a 7,6 MB.

## Co je v grafech

Platí pro `kurnik-test.db`; krátké databáze mají z tohohle seznamu první dva řádky.

| Rozsah | Události |
|---|---|
| 6 h, 24 h | tři zprávy bez hodnot (v tabulce pomlčky), výpadek 40 minut, dvě hodiny s baterií na 8,00 V a panelem na 12,50 V |
| 7 dní | výpadek 6 hodin, 8 hodin kriticky vybitá baterie, den s odpojeným panelem |
| 30 dní | výpadek 2 dny, 3 dny odpojený panel, 4 dny baterie na horní mezi |
| 1 rok | výpadek 9 dní, 2 dny panel na horní mezi, 5 dní kriticky vybitá baterie, 10 dní odpojený panel |
| Vše | výpadek 21 dní, 6 dní kriticky vybitá baterie |

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
`poplach`, `porucha`, `meze`, `stara`, `jedno` a `prazdna`. Generátor u každé databáze
vypíše seznam událostí i s časy, takže je jasné, kam se v grafu dívat.

Hodnoty sedí na mřížce protokolu — baterie po 50 mV v rozsahu 5–8 V, panel po 100 mV do
12,5 V — takže vypadají přesně jako to, co dorazí z kurníku. Napětí panelu je měřené
naprázdno, protože firmware při měření rozepne odpojovač; přes den je proto kolem 10 až
11 V a v noci nulové.
