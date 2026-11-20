/*
 * lab5_app.h - spoj modifikacije sa postojecim projektom
 *
 * Izmjene u postojecim fajlovima (vec uradjene u Main.c i process_audio.c):
 *   Main.c          : lab5_init(); lab5_benchmark();  (prije instaliranja prekida)
 *                     lab5_background();              (u for(;;) petlji, poslije idle)
 *   process_audio.c : y = lab5_process_sample(Left_Channel_In1, Right_Channel_In1);
 *                     y se upisuje u Left/Right_Channel_Out1..4;
 *                     cijeli prekid se mjeri i javlja sa lab5_note_isr_cycles()
 */
#ifndef LAB5_APP_H
#define LAB5_APP_H

extern volatile unsigned int lab5_sample_counter;

/* statistika ciklusa za posljednji zavrseni prozor (vidljivo u Expressions) */
typedef struct {
    unsigned int key;        /* kodirana konfiguracija              */
    unsigned int n;          /* broj odmjeraka                      */
    unsigned int min, max;   /* ciklusi po odmjerku                 */
    float        avg;
    unsigned int over;       /* broj prekida duzih od budzeta       */
    float        isr_avg;    /* cijela process_AD1939_samples       */
    unsigned int isr_max;
} lab5_stats_t;

extern volatile lab5_stats_t lab5_last_stats;

void  lab5_init(void);
float lab5_process_sample(float in_l, float in_r);   /* poziva se iz prekida */
void  lab5_background(void);                         /* poziva se iz main petlje */
void  lab5_benchmark(void);                          /* mikro-mjerenja, jednom na startu */
void  lab5_note_isr_cycles(unsigned int c);          /* ciklusi cijelog prekida (process_audio.c) */

#endif
