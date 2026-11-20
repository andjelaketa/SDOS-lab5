/*
 * lab5_ui.h - tasteri i LED diode na ADSP-21489 EZ-Board
 *
 *   PB1 (FLAG1)    : G + (kruzno: 1, 1.5, 2, ... 256, pa opet 1)
 *   PB2 (FLAG2)    : NE KORISTI SE - u init_PLL_SDRAM.c je ukljucen MSEN,
 *                    pa je FLAG2 memorijski select MS2 (SDRAM/SRAM)
 *   PB3 (DAI_P19)  : rezim  BYPASS -> HARD -> TANH -> POLY -> ...
 *   PB4 (DAI_P20)  : OS / izvor:
 *                    sweep,bez OS -> sweep,2xOS -> ulaz,bez OS -> ulaz,2xOS
 *
 *   LED1, LED2 (DPI_P6, DPI_P13) : rezim (2 bita, LED1 = LSB)
 *   LED3       (DPI_P14)         : 2x OS ukljucen
 *   LED4       (DAI_P3)          : izvor = ulaz kodeka (ugasena = sweep)
 *   LED5..LED8 (DAI_P4,P15,P16,P17) : indeks pojacanja 0..15 (LED5 = LSB)
 */
#ifndef LAB5_UI_H
#define LAB5_UI_H

void lab5_ui_init(void);
/* poziva se iz main petlje; vraca 1 ako je pritisnut neki taster */
int  lab5_ui_poll(unsigned int sample_counter);
void lab5_ui_show(void);

#endif
