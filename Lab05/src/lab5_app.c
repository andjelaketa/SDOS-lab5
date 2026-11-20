/*
 * lab5_app.c - integracija modifikacije u projekat 5. vjezbe
 * Autor: Andjela Laketa
 *
 *   - lab5_process_sample(): poziva se iz prekida za svaki odmjerak,
 *     bira izvor (sweep / ulaz), radi overdrive i MJERI broj ciklusa
 *   - lab5_background():     poziva se iz main petlje, cita tastere i
 *     ispisuje statistiku ciklusa u Console (log mjerenja)
 *   - lab5_benchmark():      mjeri cijenu svakog bloka posebno
 *                            (hard, tanh, poly, interpolator, decimator, ...)
 *
 * Mjerenje ciklusa koristi CCES "cycle_count.h" (registar EMUCLK jezgra).
 */
#define DO_CYCLE_COUNTS
#include <cycle_count.h>
#include <stdio.h>
#include "lab5_app.h"
#include "lab5_ui.h"
#include "overdrive.h"

#define BUDGET   ((unsigned int)(LAB5_FCCLK / LAB5_FS))

volatile unsigned int  lab5_sample_counter = 0;
volatile lab5_stats_t  lab5_last_stats;
static volatile int    stats_ready = 0;

/* tekuca statistika (pise je samo prekid) */
static unsigned int cur_key = 0xFFFFFFFFu, cur_n, cur_min, cur_max;
static float        cur_sum;
static unsigned int isr_n, isr_max, isr_over;     /* cijeli prekid */
static float        isr_sum;

void lab5_note_isr_cycles(unsigned int c)
{
    isr_sum += (float)c;
    isr_n++;
    if (c > isr_max) isr_max = c;
    if (c > BUDGET)  isr_over++;
}

static unsigned int config_key(void)
{
    return  (unsigned int)od_mode
         | ((unsigned int)od_os_enable << 2)
         | ((unsigned int)od_source    << 3)
         | ((unsigned int)od_gain_idx  << 4);
}

static const char *mode_name(int m)
{
    static const char *names[] = { "BYPASS", "HARD", "TANH", "POLY" };
    return (m >= 0 && m < OD_NUM_MODES) ? names[m] : "?";
}

void lab5_init(void)
{
    od_init(LAB5_FS);
    lab5_ui_init();
    printf("\n=== SDOS Lab5 / modifikacija 5 - overdrive i aliasing ===\n");
    printf("fs = %.0f Hz, fCCLK = %.0f MHz, budzet = %u ciklusa/odmjerak\n",
           LAB5_FS, LAB5_FCCLK / 1e6f, BUDGET);
    printf("half-band: L = %d (NH = %d), beta = %.1f\n",
           LAB5_HB_LEN, LAB5_HB_NH, LAB5_HB_BETA);
}

/* ---------------------------------------------------------------
 * Poziva se iz prekidne rutine (process_AD1939_samples)
 * --------------------------------------------------------------- */
float lab5_process_sample(float in_l, float in_r)
{
    cycle_t t0, dt;
    unsigned int c, key;
    float x, y;

    lab5_sample_counter++;

    x = (od_source == OD_SRC_SWEEP) ? od_sweep_next(0)
                                    : 0.5f * (in_l + in_r);

    START_CYCLE_COUNT(t0);
    y = od_process(x);                   /* <-- mjereni dio */
    STOP_CYCLE_COUNT(dt, t0);
    c = (unsigned int)dt;

    /* statistika po konfiguraciji; reset pri promjeni */
    key = config_key();
    if (key != cur_key) {
        cur_key = key; cur_n = 0; cur_sum = 0.0f;
        cur_min = 0xFFFFFFFFu; cur_max = 0;
    }
    if (c < cur_min) cur_min = c;
    if (c > cur_max) cur_max = c;
    cur_sum += (float)c;
    if (++cur_n >= LAB5_MEAS_WINDOW) {
        lab5_last_stats.key  = cur_key;
        lab5_last_stats.n    = cur_n;
        lab5_last_stats.min  = cur_min;
        lab5_last_stats.max  = cur_max;
        lab5_last_stats.avg  = cur_sum / (float)cur_n;
        lab5_last_stats.over = isr_over;
        lab5_last_stats.isr_avg = isr_n ? isr_sum / (float)isr_n : 0.0f;
        lab5_last_stats.isr_max = isr_max;
        isr_n = 0; isr_sum = 0.0f; isr_max = 0; isr_over = 0;
        stats_ready = 1;
        cur_key = 0xFFFFFFFFu;           /* novi prozor */
    }

    return LAB5_OUT_VOLUME * y;
}

/* ---------------------------------------------------------------
 * Main petlja: tasteri + log (ispis samo jednom po konfiguraciji,
 * jer printf preko JTAG-a zaustavlja jezgro i izaziva "klik")
 * --------------------------------------------------------------- */
void lab5_background(void)
{
    static unsigned int printed_key = 0xFFFFFFFFu;

    lab5_ui_poll(lab5_sample_counter);

    if (stats_ready) {
        lab5_stats_t s;
        stats_ready = 0;
        s.key = lab5_last_stats.key;  s.n   = lab5_last_stats.n;
        s.min = lab5_last_stats.min;  s.max = lab5_last_stats.max;
        s.avg = lab5_last_stats.avg;  s.over = lab5_last_stats.over;
        s.isr_avg = lab5_last_stats.isr_avg;  s.isr_max = lab5_last_stats.isr_max;
        if (s.key != printed_key) {
            int m  = s.key & 3, os = (s.key >> 2) & 1;
            int src = (s.key >> 3) & 1, gi = (s.key >> 4) & 15;
            printf("LOG fs=%.0f mode=%-6s OS=%d src=%s G=%-5.1f | "
                   "obrada: min=%u avg=%.1f max=%u | cijeli prekid: avg=%.1f max=%u | "
                   "budzet=%u (%.2f%%) over=%u\n",
                   LAB5_FS, mode_name(m), os ? 2 : 1, src ? "IN" : "SWEEP",
                   od_gain_table[gi], s.min, s.avg, s.max, s.isr_avg, s.isr_max,
                   BUDGET, 100.0f * s.isr_avg / BUDGET, s.over);
            printed_key = s.key;
        }
    }
}

/* ---------------------------------------------------------------
 * Benchmark: svaki blok N puta nad nizom ulaza, rezultat u
 * ciklusima po odmjerku (oduzeta je cijena prazne petlje).
 * --------------------------------------------------------------- */
#define BN 256
static float bench_in[BN];
static volatile float bench_sink;

typedef enum { B_EMPTY, B_HARD, B_TANH, B_POLY, B_INTERP, B_DECIM,
               B_OS_BYPASS, B_OS_HARD, B_OS_TANH, B_OS_POLY, B_SWEEP } bench_t;

static unsigned int bench_run(bench_t t, float g)
{
    cycle_t t0, dt;
    int i;
    float acc = 0.0f, v0, v1;

    START_CYCLE_COUNT(t0);
    for (i = 0; i < BN; i++) {
        float x = bench_in[i];
        switch (t) {
        case B_EMPTY:  acc += x;                         break;
        case B_HARD:   acc += od_nl_hard(x, g);          break;
        case B_TANH:   acc += od_nl_tanh(x, g);          break;
        case B_POLY:   acc += od_nl_poly(x, g);          break;
        case B_INTERP: od_interp2(x, &v0, &v1); acc += v0 + v1; break;
        case B_DECIM:  acc += od_decim2(x, x);           break;
        case B_OS_BYPASS: od_interp2(x,&v0,&v1); acc += od_decim2(v0, v1); break;
        case B_OS_HARD:   od_interp2(x,&v0,&v1);
                          acc += od_decim2(od_nl_hard(v0,g), od_nl_hard(v1,g)); break;
        case B_OS_TANH:   od_interp2(x,&v0,&v1);
                          acc += od_decim2(od_nl_tanh(v0,g), od_nl_tanh(v1,g)); break;
        case B_OS_POLY:   od_interp2(x,&v0,&v1);
                          acc += od_decim2(od_nl_poly(v0,g), od_nl_poly(v1,g)); break;
        case B_SWEEP:  acc += od_sweep_next(0);          break;
        }
    }
    STOP_CYCLE_COUNT(dt, t0);
    bench_sink = acc;
    return (unsigned int)dt;
}

void lab5_benchmark(void)
{
    int i;
    unsigned int e;
    for (i = 0; i < BN; i++)               /* ulazi u opsegu [-0.6, 0.6] */
        bench_in[i] = 0.6f * (2.0f * i / (BN - 1) - 1.0f);

    e = bench_run(B_EMPTY, 1.0f);
#define REP(name, t, g) do { unsigned int c = bench_run(t, g); \
        printf("BENCH %-28s : %7.1f ciklusa/odmjerak\n", name, \
               (float)((int)c - (int)e) / BN); } while (0)

    printf("\n--- BENCHMARK (N=%d, prazna petlja = %.1f cik/odm oduzeta) ---\n",
           BN, (float)e / BN);
    REP("tvrdo odsijecanje (G=8)",        B_HARD, 8.0f);
    REP("tanhf, G=1 (mali argument)",     B_TANH, 1.0f);
    REP("tanhf, G=8",                     B_TANH, 8.0f);
    REP("tanhf, G=64 (zasicenje)",        B_TANH, 64.0f);
    REP("polinom (G=8)",                  B_POLY, 8.0f);
    REP("interpolator 2x (half-band)",    B_INTERP, 1.0f);
    REP("decimator 2x (half-band)",       B_DECIM, 1.0f);
    REP("OS lanac bez nelinearnosti",     B_OS_BYPASS, 1.0f);
    REP("OS + tvrdo (G=8)",               B_OS_HARD, 8.0f);
    REP("OS + tanhf (G=8)",               B_OS_TANH, 8.0f);
    REP("OS + polinom (G=8)",             B_OS_POLY, 8.0f);
    REP("sweep generator",                B_SWEEP, 1.0f);
    printf("--- kraj benchmarka ---\n\n");
#undef REP

    od_reset_state();
    od_sweep_reset();
}
