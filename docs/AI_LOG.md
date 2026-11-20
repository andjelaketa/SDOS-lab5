# Log korišćenja AI alata

**Student:** Anđela Laketa · **Datum:** 19.11.2026.

## AI infrastruktura

| Element | Opis |
|---|---|
| Model / agent | Claude Opus 5.5 (Anthropic), claude.ai, agentski režim – jedan agent, bez pod-agenata |
| Alati (harness) | čitanje i pisanje fajlova, Linux terminal u oblaku (gcc, Python: numpy, scipy, matplotlib), web pretraga |
| Skilovi | ugrađeni skilovi za čitanje PDF-a i za pravljenje Word dokumenta (izvještaj) |
| Ulazi | PDF 5. vježbe, PDF individualnih modifikacija, poruke asistenta (slike), originalni CCES projekat `Lab05.zip`, log mjerenja sa ploče |
| Provjera AI rezultata | kod provjereno preveden na PC-u (gcc); na ploči prevođen u CCES-u i pokrenut; sva mjerenja i slušanje urađeni samostalno |

## Upiti i namjena

| # | Upit (sažetak) | Za šta je AI korišten |
|---|---|---|
| 1 | Poslani PDF vježbe, PDF modifikacija i poruke asistenta; traženo: sve za vježbu, objašnjenje koda/postupka/rezultata, uputstvo korak po korak i koraci za izvještaj | generisanje C koda (`overdrive.c`, `lab5_ui.c`, `lab5_app.c`, `lab5_config.h`), teorijske tabele aliasa, predloga tabele predviđanja. AI je generisao i PC simulaciju spektra, koja **nije uključena** u konačan rad |
| 2 | Poslan originalni projekat `Lab05.zip` | prilagođavanje originalnom kodu: sweep iz postojeće `Sinetbl.dat` (1.31), PB2 se ne koristi (FLAG2 = MS2 zbog MSEN), mjerenje cijelog prekida, ugradnja u `Main.c` i `process_audio.c`, ispravka redoslijeda inicijalizacije `DelayLine` |
| 3 | Traženo konačno uputstvo i objašnjenje koda, zadatka i rješenja | objašnjenje teorije i koda, plan rada u laboratoriji |
| 4 | Greška „Unable To Launch“ u CCES-u | uputstvo za pravljenje debug (launch) konfiguracije |
| 5 | Objašnjenje vježbe „običnim jezikom“; tabele i pitanja za slušanje | priprema testa slušanjem (48 kHz, 96 kHz, OS, promjena G) |
| 6 | Provjera kompletnosti loga mjerenja | AI je uočio da nedostaju HARD OS=2 i mjerenja na 96 kHz – dopunjeno u laboratoriji |
| 7 | Generisanje dokumenata za predaju na osnovu loga | grafici izmjerenih vrijednosti, popunjena tabela predviđanja, nacrt izvještaja |
| 8 | Izbaciti PC simulaciju i linearni model; naglasiti šta nije traženo; urediti literaturu | izmjena izvještaja i repozitorijuma |
| 9 | Dodati poglavlje o osnovnoj vježbi | analiza originalnog koda (SRU veze, kodek, SPORT/DMA, rutiranje, eho) i nacrt poglavlja 2 izvještaja |

## Šta je urađeno samostalno
- prevođenje i pokretanje projekta na ploči ADSP-21489, podešavanje debug konfiguracije
- sva mjerenja ciklusa (BENCH i LOG, 48 kHz i 96 kHz)
- svi testovi slušanjem i opisi zvuka
- pregled i provjera izvještaja, odbrana
