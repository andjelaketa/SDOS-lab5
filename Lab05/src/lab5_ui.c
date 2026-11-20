/*
 * lab5_ui.c - tasteri i LED diode (ADSP-21489 EZ-Board)
 * Autor: Andjela Laketa
 *
 * Tasteri se citaju POLLING-om iz main petlje (ne u prekidu), sa
 * softverskim debounce-om od ~20 ms. Vremenska baza je brojac odmjeraka
 * koji uvecava audio prekid.
 *
 * Aktivni nivo tastera se ne pretpostavlja: pri inicijalizaciji se
 * zapamti stanje "pusten", a pritisak je svaka promjena od tog stanja.
 *
 * PB2 (FLAG2) se NE koristi: init_PLL_SDRAM.c postavlja MSEN u SYSCTL,
 * cime FLAG2/FLAG3 postaju memorijski selecti MS2/MS3.
 *
 * Prekidaci SW1.3, SW1.4, SW2.7, SW2.8, SW7.1 (LED4-8), SW3.6, SW4.5,
 * SW4.6 (LED1-3), SW13.4 (PB1) i SW7.3, SW7.4 (PB3, PB4) moraju biti
 * ON (podrazumijevano jesu).
 *
 * Za DPI pinove se koristi makro SRU (kao u postojecem initSRU.c).
 */
#include <platform_include.h>   /* cdef21489.h / def21489.h */
#include <sru.h>
#include <sysreg.h>
#include "lab5_ui.h"
#include "overdrive.h"

#define NBTN        3     /* PB1, PB3, PB4 */
#define DEBOUNCE    ((unsigned int)(0.020f * LAB5_FS))   /* 20 ms */

static int          btn_idle[NBTN];
static int          btn_stable[NBTN];
static int          btn_last[NBTN];
static unsigned int btn_tchg[NBTN];

/* ---------------- citanje sirovog stanja tastera ---------------- */
static int read_raw(int i)
{
    switch (i) {
    case 0:  return (sysreg_read(sysreg_FLAGS) & FLG1) ? 1 : 0;   /* PB1 = FLAG1  */
    case 1:  return (*pDAI_PIN_STAT >> 18) & 1;                    /* PB3 = DAI_P19 */
    default: return (*pDAI_PIN_STAT >> 19) & 1;                    /* PB4 = DAI_P20 */
    }
}

/* ---------------- LED diode ---------------- */
static void led_set(int n, int on)
{
    /* SRU makro zahtijeva konstantu kao izvor, zato if/else */
    switch (n) {
    case 1: if (on) { SRU(HIGH, DPI_PB06_I); } else { SRU(LOW, DPI_PB06_I); } break;
    case 2: if (on) { SRU(HIGH, DPI_PB13_I); } else { SRU(LOW, DPI_PB13_I); } break;
    case 3: if (on) { SRU(HIGH, DPI_PB14_I); } else { SRU(LOW, DPI_PB14_I); } break;
    case 4: if (on) { SRU(HIGH, DAI_PB03_I);  } else { SRU(LOW, DAI_PB03_I);  } break;
    case 5: if (on) { SRU(HIGH, DAI_PB04_I);  } else { SRU(LOW, DAI_PB04_I);  } break;
    case 6: if (on) { SRU(HIGH, DAI_PB15_I);  } else { SRU(LOW, DAI_PB15_I);  } break;
    case 7: if (on) { SRU(HIGH, DAI_PB16_I);  } else { SRU(LOW, DAI_PB16_I);  } break;
    case 8: if (on) { SRU(HIGH, DAI_PB17_I);  } else { SRU(LOW, DAI_PB17_I);  } break;
    default: break;
    }
}

void lab5_ui_show(void)
{
    int m = od_mode, g = od_gain_idx;
    led_set(1, m & 1);
    led_set(2, (m >> 1) & 1);
    led_set(3, od_os_enable);
    led_set(4, od_source == OD_SRC_INPUT);
    led_set(5, g & 1);
    led_set(6, (g >> 1) & 1);
    led_set(7, (g >> 2) & 1);
    led_set(8, (g >> 3) & 1);
}

void lab5_ui_init(void)
{
    int i;

    /* LED pinovi kao izlazi (PBEN = HIGH) */
    SRU(HIGH, DPI_PBEN06_I);
    SRU(HIGH, DPI_PBEN13_I);
    SRU(HIGH, DPI_PBEN14_I);
    SRU(HIGH, PBEN03_I);
    SRU(HIGH, PBEN04_I);
    SRU(HIGH, PBEN15_I);
    SRU(HIGH, PBEN16_I);
    SRU(HIGH, PBEN17_I);

    /* PB3, PB4 (DAI_P19, DAI_P20) kao ulazi */
    SRU(LOW, DAI_PB19_I);
    SRU(LOW, DAI_PB20_I);
    SRU(LOW, PBEN19_I);
    SRU(LOW, PBEN20_I);

    /* PB1 (FLAG1) kao ulaz: obrisati FLG1O bit */
    sysreg_bit_clr(sysreg_FLAGS, FLG1O);

    for (i = 0; i < NBTN; i++) {
        btn_idle[i] = btn_stable[i] = btn_last[i] = read_raw(i);
        btn_tchg[i] = 0;
    }
    lab5_ui_show();
}

static void on_press(int i)
{
    switch (i) {
    case 0: od_gain_idx = (od_gain_idx + 1) % OD_NUM_GAINS;    break; /* PB1 */
    case 1: od_mode = (od_mode + 1) % OD_NUM_MODES;            break; /* PB3 */
    case 2: {                                                         /* PB4 */
        int s = ((od_source & 1) << 1) | (od_os_enable & 1);   /* 0..3 */
        s = (s + 1) & 3;
        od_os_enable = s & 1;
        od_source    = (s >> 1) & 1;
        break;
    }
    }
}

int lab5_ui_poll(unsigned int now)
{
    int i, any = 0;
    for (i = 0; i < NBTN; i++) {
        int raw = read_raw(i);
        if (raw != btn_last[i]) {
            btn_last[i] = raw;
            btn_tchg[i] = now;
        } else if ((now - btn_tchg[i]) > DEBOUNCE && raw != btn_stable[i]) {
            btn_stable[i] = raw;
            if (raw != btn_idle[i]) {       /* ivica "pritisnut" */
                on_press(i);
                any = 1;
            }
        }
    }
    if (any) lab5_ui_show();
    return any;
}
