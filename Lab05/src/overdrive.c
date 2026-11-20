/*
 * overdrive.c
 * SDOS 2026 - Lab 5, modifikacija 5: Nelinearni efekti i aliasing
 * Autor: Andjela Laketa
 *
 * Sadrzaj:
 *   1) generator sweep-a 1-8 kHz iz postojece tabele sinusa (Sinetbl.dat)
 *   2) tri nelinearnosti: tvrdo odsijecanje, tanh (bibliotecka), polinom
 *   3) 2x nadodmjeravanje sa half-band FIR filtrom (polifazna realizacija)
 *
 * Kod je cist C bez zavisnosti od hardvera.
 */
#include <math.h>
#include <stdint.h>
#include "overdrive.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

/* ================================================================
 * Globalno stanje (mijenja se tasterima / iz debagera)
 * ================================================================ */
volatile int od_mode      = OD_HARD;
volatile int od_gain_idx  = 6;          /* G = 8 */
volatile int od_os_enable = 0;
volatile int od_source    = OD_SRC_SWEEP;

const float od_gain_table[OD_NUM_GAINS] = {
    1.0f, 1.5f, 2.0f, 3.0f, 4.0f, 6.0f, 8.0f, 12.0f,
    16.0f, 24.0f, 32.0f, 48.0f, 64.0f, 96.0f, 128.0f, 256.0f
};

/* ================================================================
 * 1) Sweep generator iz postojece tabele Sinetbl.dat
 *
 *   Tabela ima 4000 odmjeraka jedne periode u formatu 1.31.
 *   Pri inicijalizaciji se pretvara u float (x / 2^31).
 *   Faza je realni indeks u tabeli p u [0, 4000):
 *       p[n+1] = p[n] + f[n] * 4000 / fs   (mod 4000)
 *   a frekvencija linearno raste od F0 do F1 za LAB5_SWEEP_TIME s.
 * ================================================================ */
static const uint32_t sine_fix[LAB5_SINE_SIZE] = {
#include "Sinetbl.dat"
};
static float sine_tab[LAB5_SINE_SIZE];
static float sw_pos;               /* faza = realni indeks u tabeli  */
static float sw_freq;              /* trenutna frekvencija [Hz]      */
static float sw_df;                /* prirast frekvencije po odmjerku */
static float sw_hz_to_step;        /* 4000 / fs                      */

void od_sweep_reset(void)
{
    sw_pos  = 0.0f;
    sw_freq = LAB5_SWEEP_F0;
}

float od_sweep_next(float *f_inst)
{
    float s = sine_tab[(int)sw_pos];

    if (f_inst) *f_inst = sw_freq;

    sw_pos += sw_freq * sw_hz_to_step;
    if (sw_pos >= (float)LAB5_SINE_SIZE)
        sw_pos -= (float)LAB5_SINE_SIZE;

    /* linearni sweep F0 -> F1, pa ponovo od F0 */
    sw_freq += sw_df;
    if (sw_freq >= LAB5_SWEEP_F1)
        sw_freq = LAB5_SWEEP_F0;

    return LAB5_SWEEP_AMP * s;
}

/* ================================================================
 * 2) Nelinearnosti
 * ================================================================ */
float od_nl_hard(float x, float g)
{
    float y = g * x;
    if (y >  1.0f) y =  1.0f;
    if (y < -1.0f) y = -1.0f;
    return y;
}

float od_nl_tanh(float x, float g)
{
    return tanhf(g * x);
}

/* Kubni polinom y = x - x^3/3 vazi za |x|<=1; skaliran sa 1.5 tako da
 * je izlaz u zasicenju tacno +-1 (isto kao kod tvrdog odsijecanja).
 * Generise samo 3. harmonik (dok je ulaz u opsegu |Gx|<=1).        */
float od_nl_poly(float x, float g)
{
    float u = g * x;
    if (u >  1.0f) u =  1.0f;
    if (u < -1.0f) u = -1.0f;
    return u * (1.5f - 0.5f * u * u);
}

float od_nl(int mode, float x, float g)
{
    switch (mode) {
    case OD_HARD: return od_nl_hard(x, g);
    case OD_TANH: return od_nl_tanh(x, g);
    case OD_POLY: return od_nl_poly(x, g);
    default:      return x;            /* OD_BYPASS */
    }
}

/* ================================================================
 * 3) Half-band FIR i 2x nadodmjeravanje
 *
 *   Half-band filtar duzine L = 4*NH-1, centar c = 2NH-1:
 *     h[c] = 0.5,  h[c +- 2m] = 0 (m != 0),  h[c +- (2m-1)] != 0.
 *   Nenulti bocni koeficijenti su na PARNIM indeksima k = 2j:
 *     od_hb_side[j] = h[2j],  j = 0 .. 2NH-1, simetricni.
 *
 *   Interpolator (umetanje nule + filtar, pojacanje 2):
 *     v[2n]   = 2 * sum_j h[2j] x[n-j]          (NH MAC uz simetriju)
 *     v[2n+1] = x[n-NH+1]                        (samo centralni koef.)
 *
 *   Decimator (filtar + uzimanje svakog drugog):
 *     y[n] = sum_j h[2j] w[2n-2j] + 0.5 w[2(n-NH)+1]
 *
 *   => po ulaznom odmjerku: ~NH MAC za interpolaciju + ~NH MAC za
 *      decimaciju, umjesto 2*L za direktnu realizaciju.
 * ================================================================ */
#define NH   LAB5_HB_NH
#define NS   (2 * NH)           /* broj bocnih koeficijenata */

float od_hb_full[LAB5_HB_LEN];
float od_hb_side[NS];
static float hb_pair[NH];       /* h[2j] (za decimator)              */
static float hb_pair2[NH];      /* 2*h[2j] (za interpolator, G = 2)  */

/* Linije kasnjenja - "dupli bafer" da bi se izbjegao modulo:
 * podatak se upisuje na idx i idx+NS, pa je buf[idx..idx+NS-1]
 * uvijek kontinualna istorija x[n], x[n-1], ... */
static float ix_buf[2 * NS];  static int ix_idx;     /* ulaz interpolatora     */
static float de_buf[2 * NS];  static int de_idx;     /* parni odmjerci w[2n]   */
static float do_buf[2 * (NH + 1)]; static int do_idx;/* neparni odmjerci w[2n+1] */

static double bessel_i0(double x)
{
    double sum = 1.0, term = 1.0, k;
    for (k = 1.0; k < 50.0; k += 1.0) {
        term *= (x / (2.0 * k)) * (x / (2.0 * k));
        sum  += term;
        if (term < 1e-12 * sum) break;
    }
    return sum;
}

static void hb_design(void)
{
    int k, j;
    const int L = LAB5_HB_LEN;
    const int c = (L - 1) / 2;
    double side_sum = 0.0;
    double i0b = bessel_i0(LAB5_HB_BETA);

    for (k = 0; k < L; k++) {
        int d = k - c;
        double h, r, w;
        if (d == 0)            h = 0.5;
        else if ((d & 1) == 0) h = 0.0;                 /* parni pomaci = 0 */
        else                   h = sin(M_PI * d / 2.0) / (M_PI * d);
        r = 2.0 * k / (L - 1) - 1.0;                    /* -1 .. 1 */
        w = bessel_i0(LAB5_HB_BETA * sqrt(1.0 - r * r)) / i0b;
        od_hb_full[k] = (float)(h * w);
    }
    od_hb_full[c] = 0.5f;

    /* normalizacija: suma bocnih = 0.5 -> DC pojacanje tacno 1 */
    for (j = 0; j < NS; j++) side_sum += od_hb_full[2 * j];
    for (j = 0; j < NS; j++) {
        od_hb_full[2 * j] = (float)(od_hb_full[2 * j] * 0.5 / side_sum);
        od_hb_side[j]     = od_hb_full[2 * j];
    }
    for (j = 0; j < NH; j++) {
        hb_pair[j]  = od_hb_side[j];
        hb_pair2[j] = 2.0f * od_hb_side[j];
    }
}

void od_reset_state(void)
{
    int i;
    for (i = 0; i < 2 * NS; i++) { ix_buf[i] = 0.0f; de_buf[i] = 0.0f; }
    for (i = 0; i < 2 * (NH + 1); i++) do_buf[i] = 0.0f;
    ix_idx = 0; de_idx = 0; do_idx = 0;
}

void od_interp2(float x, float *v0, float *v1)
{
    int j;
    float acc = 0.0f;
    float *p;

    if (--ix_idx < 0) ix_idx = NS - 1;
    ix_buf[ix_idx] = x;
    ix_buf[ix_idx + NS] = x;
    p = &ix_buf[ix_idx];                /* p[0]=x[n], p[1]=x[n-1], ... */

    for (j = 0; j < NH; j++)            /* simetrija: h[2j] = h[2(NS-1-j)] */
        acc += hb_pair2[j] * (p[j] + p[NS - 1 - j]);

    *v0 = acc;                          /* v[2n]   */
    *v1 = p[NH - 1];                    /* v[2n+1] = x[n-NH+1] */
}

float od_decim2(float w0, float w1)
{
    int j;
    float acc;
    float *pe, *po;

    if (--de_idx < 0) de_idx = NS - 1;
    de_buf[de_idx] = w0;
    de_buf[de_idx + NS] = w0;
    pe = &de_buf[de_idx];

    if (--do_idx < 0) do_idx = NH;      /* bafer duzine NH+1 */
    do_buf[do_idx] = w1;
    do_buf[do_idx + NH + 1] = w1;
    po = &do_buf[do_idx];               /* po[NH] = w[2(n-NH)+1] */

    acc = 0.5f * po[NH];
    for (j = 0; j < NH; j++)
        acc += hb_pair[j] * (pe[j] + pe[NS - 1 - j]);
    return acc;
}

/* ================================================================
 * Kompletna obrada
 * ================================================================ */
float od_process(float x)
{
    const int   mode = od_mode;
    const float g    = od_gain_table[od_gain_idx & (OD_NUM_GAINS - 1)];

    if (!od_os_enable)
        return od_nl(mode, x, g);

    {   /* 2x: interpolacija -> nelinearnost na 2fs -> decimacija */
        float v0, v1;
        od_interp2(x, &v0, &v1);
        v0 = od_nl(mode, v0, g);
        v1 = od_nl(mode, v1, g);
        return od_decim2(v0, v1);
    }
}

void od_init(float fs)
{
    unsigned int i;
    for (i = 0; i < LAB5_SINE_SIZE; i++)          /* 1.31 -> float */
        sine_tab[i] = (float)(int32_t)sine_fix[i] * (1.0f / 2147483648.0f);

    sw_hz_to_step = (float)LAB5_SINE_SIZE / fs;
    sw_df = (LAB5_SWEEP_F1 - LAB5_SWEEP_F0) / (LAB5_SWEEP_TIME * fs);
    od_sweep_reset();

    hb_design();
    od_reset_state();
}
