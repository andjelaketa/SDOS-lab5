/*
 * lab5_config.h
 * SDOS 2026 - Lab 5, modifikacija 5: Nelinearni efekti i aliasing (overdrive)
 * Autor: Andjela Laketa
 *
 * Svi parametri modifikacije na jednom mjestu.
 */
#ifndef LAB5_CONFIG_H
#define LAB5_CONFIG_H

/* ------------------------------------------------------------------
 * Frekvencija odmjeravanja.
 * MORA odgovarati izboru u init1939viaSPI.c:
 *   #define USE_48_KHZ_SAMPLE_RATE  ->  LAB5_FS = 48000.0f
 *   #define USE_96_KHZ_SAMPLE_RATE  ->  LAB5_FS = 96000.0f
 * ------------------------------------------------------------------ */
#ifndef LAB5_FS
#define LAB5_FS            48000.0f
#endif

/* Takt jezgra (CCLK) u Hz - init_PLL_SDRAM.c: CLKIN 25 MHz,
 * PLLM=16 -> fVCO = 2*16*25 = 800 MHz, PLLD=2 -> fCCLK = 400 MHz.
 * Koristi se samo za racunanje budzeta ciklusa N = fCCLK / fs. */
#ifndef LAB5_FCCLK
#define LAB5_FCCLK         400000000.0f
#endif

/* ------------------------------------------------------------------
 * Generator sweep-a (tabela sinusa)
 * ------------------------------------------------------------------ */
/* Koristi se POSTOJECA tabela projekta Sinetbl.dat:
 * 4000 odmjeraka jedne periode sinusa u formatu 1.31 (fiksni zarez). */
#define LAB5_SINE_SIZE     4000
#define LAB5_SWEEP_F0      1000.0f                  /* Hz            */
#define LAB5_SWEEP_F1      8000.0f                  /* Hz            */
#define LAB5_SWEEP_TIME    10.0f                    /* s, jedan prolaz 1->8 kHz */
#define LAB5_SWEEP_AMP     0.5f                     /* amplituda (-6 dBFS) */

/* ------------------------------------------------------------------
 * Half-band FIR za 2x nadodmjeravanje
 *   HB_NH  = broj nenultih koeficijenata sa JEDNE strane centra
 *   duzina filtra L = 4*HB_NH - 1  (npr. NH=12 -> L=47)
 *   HB_BETA = parametar Kaiserovog prozora
 * ------------------------------------------------------------------ */
#ifndef LAB5_HB_NH
#define LAB5_HB_NH         12
#endif
#define LAB5_HB_LEN        (4 * LAB5_HB_NH - 1)
#ifndef LAB5_HB_BETA
#define LAB5_HB_BETA       8.0f
#endif

/* Izlazna jacina (zastita usiju!) - hard clip daje +-1 = 0 dBFS */
#define LAB5_OUT_VOLUME    0.25f                    /* -12 dB        */

/* Broj odmjeraka (1 s) nakon kojeg se statistika ciklusa ispisuje */
#define LAB5_MEAS_WINDOW   ((unsigned int)LAB5_FS)

#endif /* LAB5_CONFIG_H */
