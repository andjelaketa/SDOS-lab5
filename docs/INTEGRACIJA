# Integracija u projekat 5. vježbe (CCES)

Izmjene su **već urađene** u folderu `Lab05/`. Sva mjesta su označena komentarom `LAB5:`.

## Novi fajlovi (Lab05/src)
`lab5_config.h, overdrive.h, overdrive.c, lab5_ui.h, lab5_ui.c, lab5_app.h, lab5_app.c`

## Main.c
- `#include "lab5_app.h"`
- poslije `init1939viaSPI()`: `lab5_init(); lab5_benchmark();`
- `SinTableInit()` i `malloc` za `DelayLine` premješteni PRIJE instaliranja prekida
  (u originalu je prekid mogao doći dok je `DelayLine == NULL`)
- u `for(;;)` petlji poslije `asm("idle;")`: `lab5_background();`

## process_audio.c
- mjerenje ciklusa cijele `process_AD1939_samples` (`START/STOP_CYCLE_COUNT`)
- prije `Transmit_DAC_Samples()`: `y = lab5_process_sample(L_In1, R_In1)` → svih 8 izlaza
- originalno rutiranje/eho je ostavljeno (izlaze prepisuje overdrive), pa mjerenje
  cijelog prekida uključuje i njegovu cijenu

## Važno
- **Release** konfiguracija ima `-O`, **Debug** nema. Mjeriti u Release-u
  (ili uključiti Properties → C/C++ Build → Settings → Compiler → General → Enable optimization).
- **96 kHz:** u `init1939viaSPI.c` otkomentarisati `USE_96_KHZ_SAMPLE_RATE` (a 48 zakomentarisati)
  i u `lab5_config.h` postaviti `LAB5_FS 96000.0f`.
- CCLK = 400 MHz (`init_PLL_SDRAM.c`: CLKIN 25 MHz, PLLM=16, PLLD=2) → budžet 8333 / 4167 ciklusa.
- PB2 ne radi kao taster: `init_PLL_SDRAM.c` uključuje `MSEN` → FLAG2 je MS2.
