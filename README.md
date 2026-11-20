# SDOS 2026 – Lab 5, modifikacija 5: Nelinearni efekti i aliasing (overdrive)

**Student:** Anđela Laketa · ETF Banja Luka · Sistemi za digitalnu obradu signala
**Platforma:** ADSP-21489 EZ-Board (AD1939), CrossCore Embedded Studio

## Šta je implementirano
- Tvrdo odsijecanje `clip(Gx,-1,1)`, meko `tanh(Gx)` (bibliotečka `tanhf`) i polinom `1.5(u - u³/3)`
- Pojačanje G (16 nivoa, 1–256) tasterima, stanje na LED diodama
- Interni sweep 1–8 kHz iz postojeće tabele `Sinetbl.dat` (4000 tačaka, 1.31 → float, 10 s)
- 2× nadodmjeravanje sa half-band FIR filtrom (L = 47, Kaiser β=8, polifazno, simetrija)
- Mjerenje ciklusa: obrada (min/avg/max) i cijela prekidna rutina, po konfiguraciji + benchmark svakog bloka

## Rezultati (izmjereno na ploči, 19.11.2026.)
| Režim | obrada OS=1 | obrada OS=2 | cijena uklanjanja aliasa |
|---|---:|---:|---:|
| BYPASS | 49 | 238 | 189 |
| HARD | 63 | 266 | 203 |
| POLY | 69 | 278 | 209 |
| TANH (G=8) | 153 | 446 | 293 |

- Budžet: 8333 ciklusa (48 kHz) / 4166 (96 kHz); najveće opterećenje 16,6 % (TANH + OS, 96 kHz); bez prekoračenja.
- Ciklusi po odmjerku ne zavise od fs (HARD OS=2: 266 / 268).
- Slušanjem: 2× OS i 96 kHz primjetno smanjuju aliase; polinom sa G ≤ 2 bez aliasa.

Detalji: `docs/TABELA_PREDVIDJANJA.md`, logovi: `docs/logovi/`, izvještaj: `docs/SDOS_Lab5_Laketa_Andjela.pdf`.

## Struktura
```
Lab05/       CCES projekat 5. vježbe sa ugrađenom modifikacijom (otvara se direktno)
  src/overdrive.c/.h   DSP jezgro (sweep, nelinearnosti, half-band 2x OS)
  src/lab5_ui.c/.h     tasteri i LED diode
  src/lab5_app.c/.h    spoj sa projektom, mjerenje ciklusa, benchmark
  src/lab5_config.h    parametri (fs, CCLK, half-band, sweep)
  src/Main.c, src/process_audio.c   izmijenjeni (oznaka "LAB5:")
docs/        izvještaj, tabela predviđanja i mjerenja, logovi, AI log, grafici (+ skripta)
```

## Kontrole
| Taster | Funkcija |
|---|---|
| PB1 | G + (kružno: 1 → 256 → 1) |
| PB2 | ne koristi se (FLAG2 je MS2 jer `init_PLL_SDRAM.c` uključuje MSEN) |
| PB3 | režim: BYPASS → HARD → TANH → POLY |
| PB4 | sweep/bez OS → sweep/2×OS → ulaz/bez OS → ulaz/2×OS |

| LED | Značenje |
|---|---|
| LED1–2 | režim (binarno, LED1 = LSB) |
| LED3 | 2× OS uključen |
| LED4 | izvor = ulaz kodeka |
| LED5–8 | indeks pojačanja 0–15 (LED5 = LSB) |

## Proširenja koja nisu tražena zadatkom
| Proširenje | Zašto |
|---|---|
| izbor ulaza kodeka kao izvora (PB4) | provjera efekta na stvarnom muzičkom signalu |
| prikaz stanja na LED diodama | ploča nema displej; bez LED se ne zna aktivni režim/G/OS |
| polinom kao režim za slušanje | provjera uslova fs > (P+1)·fmax na sluh |
| mjerenje cijene u zavisnosti od G | provjera da tanhf zavisi od argumenta, a hard/poly ne |
| mjerenje cijelog prekida i opterećenja | koliki dio budžeta je stvarno iskorišten |
| ispravka redoslijeda inicijalizacije `DelayLine` u `Main.c` | prekid je mogao pristupiti nultom pokazivaču |

## Grafici
`docs/grafici/plot_mjerenja.py` crta teorijski prikaz aliasa i grafike iz izmjerenih vrijednosti:
```
cd docs/grafici
python3 plot_mjerenja.py
```
