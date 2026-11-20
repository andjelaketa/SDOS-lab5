# Zapažanja sa testa slušanjem

**Datum:** 19.11.2026. · **Ploča:** ADSP-21489 EZ-Board · **Izvor:** interni sweep 1–8 kHz (10 s po prolazu), slušalice

Zapažanja su zabilježena tokom slušanja i ovdje su uređena radi preglednosti.
Izmjereni brojevi ciklusa nalaze se u `logovi/log_konzola_jedinstveno.txt` i u `TABELA_PREDVIDJANJA.md`.

## 1. fs = 48 kHz, G = 8

| Konfiguracija | Zapažanje |
|---|---|
| HARD, bez OS | Pored osnovnog tona koji raste čuju se i tonovi koji idu nadolje. Slabi su, ali čujni; najviše se čuju pri kraju sweep-a, ali i na početku. Osnovni ton „zviždi“, a pozadinski zvuci su kratki, kao prebrisavanje od niskog ka visokom tonu ili zvuk iz igrice. |
| TANH, bez OS | Blaže i „sporije“ od tvrdog odsijecanja. Aliasi se ne čuju na početku sweep-a, nego otprilike od sredine; nešto su tiši nego kod HARD. |
| POLY, bez OS | Zvuči sličnije tvrdom nego mekom odsijecanju. |
| BYPASS, bez OS | Jedan čist ton koji klizi naviše. U odnosu na HARD zvuk je mnogo tuplji i nešto tiši, „kao kroz oblak“. |
| HARD, 2× OS | Tonovi koji idu nadolje su tiši. Sama distorzija se ne mijenja, možda tek malo pri višim tonovima. Razlika u odnosu na rad bez OS najveća je pri nižim tonovima sweep-a. |
| TANH, 2× OS | Osnovni ton djeluje kao da je na višim frekvencijama; aliasi su tiši i javljaju se još kasnije nego bez OS. |
| POLY, 2× OS | Aliasi se čuju od sredine sweep-a; kraći su i tiši nego bez OS. |
| BYPASS, 2× OS | Zvuči isto kao BYPASS bez OS. |

## 2. Promjena pojačanja G (fs = 48 kHz, bez OS)

| G | Režim | Zapažanje |
|---|---|---|
| 48 | HARD | Mnoštvo aliasa u svim smjerovima. |
| 48 | TANH | Takođe mnogo aliasa, ali kao da „usporavaju“ kako frekvencija osnovnog tona raste. |
| 48–64 | POLY | Najviše aliasa i najbrži. |
| 1 | HARD | Aliasi se ne čuju; osnovni ton je tiši. |
| 1 | TANH | Aliasi se ne čuju; osnovni ton je tiši. |
| 1 | POLY | Aliasi se ne čuju. |
| 2 | HARD | Aliasi se ne čuju; pri kraju sweep-a utisak da ton malo „krene unazad“. |
| 2 | TANH | Aliasi se čuju vrlo tiho, samo pri kraju sweep-a. |
| 2 | POLY | Izobličenje se ne čuje; pri kraju sweep-a utisak da ton malo „krene unazad“. |

## 3. fs = 96 kHz, G = 8

| Konfiguracija | Zapažanje |
|---|---|
| HARD, bez OS | Aliasi se čuju, ali ih je manje i tiši su nego na 48 kHz; najviše pri kraju sweep-a. |
| HARD, 2× OS | Velika razlika u odnosu na rad bez OS: aliasi su tiši, kraći i javljaju se tek pri kraju sweep-a. |
| TANH, 2× OS | Aliasi se ne čuju. |

## Zaključak sa slušanja

Na 96 kHz aliasa ima manje nego na 48 kHz, a dodatno 2× nadodmjeravanje ih dalje smanjuje.
Polinom sa malim pojačanjem ne proizvodi čujne aliase.
