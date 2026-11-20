/*
 * overdrive.h
 * DSP jezgro modifikacije (nema zavisnosti od hardvera ploce:
 * tasteri, LED diode i kodek su u lab5_ui.c i lab5_app.c).
 */
#ifndef OVERDRIVE_H
#define OVERDRIVE_H

#include "lab5_config.h"

/* Rezimi nelinearnosti */
typedef enum {
    OD_BYPASS = 0,   /* y = x (bez obrade)                        */
    OD_HARD   = 1,   /* y = clip(Gx, -1, 1)                        */
    OD_TANH   = 2,   /* y = tanh(Gx)  (bibliotecka tanhf)          */
    OD_POLY   = 3,   /* u = clip(Gx,-1,1); y = 1.5(u - u^3/3)      */
    OD_NUM_MODES
} od_mode_t;

/* Izvor signala */
typedef enum {
    OD_SRC_SWEEP = 0,  /* interni sweep 1-8 kHz iz tabele sinusa   */
    OD_SRC_INPUT = 1   /* ulaz 1 kodeka (telefon/racunar)          */
} od_source_t;

#define OD_NUM_GAINS 16

/* Trenutno stanje - volatile da bi se moglo mijenjati i iz
 * Expressions prozora u CCES-u, i iz main petlje (tasteri). */
extern volatile int od_mode;        /* od_mode_t                    */
extern volatile int od_gain_idx;    /* 0..15  -> od_gain_table[]    */
extern volatile int od_os_enable;   /* 0 = bez OS, 1 = 2x OS        */
extern volatile int od_source;      /* od_source_t                  */

extern const float od_gain_table[OD_NUM_GAINS];

/* Koeficijenti half-band filtra (popunjava od_init) */
extern float od_hb_full[LAB5_HB_LEN];     /* cijeli impulsni odziv (za izvjestaj) */
extern float od_hb_side[2 * LAB5_HB_NH];  /* nenulti bocni koef. h[2j]            */

void  od_init(float fs);

/* Generator sweep-a: vraca sljedeci odmjerak, *f_inst = trenutna frekvencija */
float od_sweep_next(float *f_inst);
void  od_sweep_reset(void);

/* Nelinearnosti (pojedinacno - koristi ih i benchmark) */
float od_nl_hard(float x, float g);
float od_nl_tanh(float x, float g);
float od_nl_poly(float x, float g);
float od_nl(int mode, float x, float g);

/* Blokovi nadodmjeravanja */
void  od_interp2(float x, float *v0, float *v1);   /* 1 ulaz -> 2 izlaza na 2fs */
float od_decim2(float w0, float w1);               /* 2 ulaza na 2fs -> 1 izlaz */

/* Kompletna obrada jednog odmjerka (bira rezim, G i OS iz globala) */
float od_process(float x);

/* Brise stanja filtara (poziva se pri promjeni rezima) */
void  od_reset_state(void);

#endif /* OVERDRIVE_H */
