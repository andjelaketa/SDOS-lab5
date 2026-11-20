# Tabela predviđanja i mjerenja – modifikacija 5 (overdrive i aliasing)

Predviđanja (kolona „Predviđeno“) su napisana **prije** rada na ploči, 19.11.2026. ujutro.
Mjerenja su urađena istog dana na ADSP-21489 EZ-Board, CCES, **Release (-O)**.
Log konzole: docs/logovi/log_konzola_jedinstveno.txt; zapažanja sa slušanja: docs/ZAPAZANJA_SLUSANJA.md.

Pretpostavke: CCLK = 400 MHz (potvrđeno u `init_PLL_SDRAM.c`: CLKIN 25 MHz, PLLM=16, PLLD=2),
kod i podaci u internoj memoriji, float aritmetika.

## A. Budžet ciklusa: N = f_CCLK / f_s

| f_s | Budžet | Predviđeno opterećenje (OS + tanh) | Izmjereno (korigovano*) |
|---|---|---|---|
| 48 kHz | 8333 | < 10 % | 8,26 % |
| 96 kHz | 4166 | < 20 % (duplo) | 16,56 % |

\* „Cijeli prekid“ u prvom prozoru nakon promjene konfiguracije sadrži i odmjerke prethodne
konfiguracije (statistika prekida se ne resetuje pri promjeni). Korigovano = obrada + 242,
gdje je 242 fiksna cijena prekida iz stabilnih prozora; izmjereni maksimumi (508, 696, 698) to potvrđuju.

## B. Cijena blokova – benchmark (ciklusa po ulaznom odmjerku)

| Blok | Obrazloženje predviđanja | Predviđeno | Izmjereno | U opsegu |
|---|---|---:|---:|:---:|
| tvrdo odsijecanje (G=8) | poziv + 1 množenje + 2 poređenja | 8–15 | 12,0 | ✅ |
| polinom (G=8) | klip + 3 množenja + oduzimanje | 10–20 | 17,0 | ✅ |
| tanhf, G=1 | exp + dijeljenje (RECIPS + Newton) | 60–120 | 67,9 | ✅ |
| tanhf, G=8 | – | – | 103,7 | – |
| tanhf, G=64 (zasićenje) | biblioteka rano vraća ±1 | 20–60 | 55,5 | ✅ |
| interpolator 2× (NH=12) | ≈2 cik. po paru koeficijenata | 30–45 | 89,0 | ❌ |
| decimator 2× (NH=12) | isto + centralni koef. | 30–45 | 82,0 | ❌ |
| OS lanac bez nelinearnosti | interp + decim | 60–90 | 168,0 | ❌ |
| OS + tvrdo | interp + decim + 2·hard | 80–120 | 192,0 | ❌ |
| OS + tanhf | interp + decim + 2·tanh | 200–320 | 372,3 | ❌ |
| OS + polinom | interp + decim + 2·poly | 90–130 | 200,0 | ❌ |
| sweep generator | čitanje tabele + 2 sabiranja + poređenje | – | 48,0 | – |

## C. Mjerenje u prekidu (obrada = `od_process`, ciklusa po odmjerku)

| Režim | OS=1, 48 kHz | OS=2, 48 kHz | OS=1, 96 kHz | OS=2, 96 kHz | Cijena uklanjanja aliasa (48 kHz) |
|---|---:|---:|---:|---:|---:|
| BYPASS | 49 | 238 | – | – | 189 (samo filtri) |
| HARD | 63 | 266** | 63 | 268 | **203** |
| POLY | 69 | 278 | – | – | 209 |
| TANH (G=8) | 153 | 446 | – | 447,8 | 293 |

\** izmjereno pri G=2; cijena tvrdog odsijecanja ne zavisi od G (63 za G = 1, 2, 8, 48).

### tanh u zavisnosti od G (OS=1, 48 kHz)

| G | 1 | 1,5 | 2 | 8 | 48 |
|---|---:|---:|---:|---:|---:|
| obrada (avg) | 112,0 | 133,5 | 140,3 | 153,0 | 104,3 |
| samo tanh (− 49) | 63 | 85 | 91 | 104 | 55 |

HARD (63) i POLY (69) ne zavise od G – izmjereno za G = 1 … 256.

## D. Predviđanja za demonstraciju (na osnovu mjerenja)

- Filtri koštaju ≈ 6–7 ciklusa po paru koeficijenata (izmjereno, ne 2).
- NH 12 → 6 (L 47 → 23): OS lanac ≈ 2·6·6,5 ≈ **78 ciklusa manje** → HARD OS=2 ≈ 190.
- NH 12 → 24 (L 95): ≈ **156 ciklusa više** → HARD OS=2 ≈ 420.
- f_s 48 → 96 kHz: ciklusi isti (izmjereno 266 → 268), % opterećenja ×2.
- G: hard/poly isto; tanh 55–104 zavisno od G.
- Debug (-O0): očekivano 3–5× više ciklusa.

## E. Aliasi – ton f0 = 5 kHz, tvrdo odsijecanje (teorija, f_a = |k f0 − m f_s|)

| k | k·f0 [kHz] | f_s=48k → f_a [kHz] | f_s=96k → f_a [kHz] |
|---|---|---|---|
| 3 | 15 | 15 | 15 |
| 5 | 25 | 23 (alias) | 25 |
| 7 | 35 | 13 (alias) | 35 |
| 9 | 45 | 3 (alias) | 45 |
| 11 | 55 | 7 (alias) | 41 (alias) |
| 13 | 65 | 17 (alias) | 31 (alias) |
| 15 | 75 | 21 (alias) | 21 (alias) |
| 17 | 85 | 11 (alias) | 11 (alias) |
| 19 | 95 | 1 (alias) | 1 (alias) |

## F. Uslov za polinom: f_s > (P+1)·f_max
P = 3, f_max = 8 kHz → f_s > 32 kHz → na 48 kHz polinom (|Gx| ≤ 1) ne pravi aliase.
Potvrđeno slušanjem: POLY, G=1 – aliasi se ne čuju.
