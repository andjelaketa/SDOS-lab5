/*********************************************************************************

 Copyright(c) 2012 Analog Devices, Inc. All Rights Reserved.

 This software is proprietary and confidential.  By using this software you agree
 to the terms of the associated Analog Devices License Agreement.

 *********************************************************************************/
/*This project is a talk-through example using the onboard AD1939 to acquire
 * and output an audio stream. The digital audio data is available for processing
 * in the file SPORT1_isr.c. The block size is 256 samples per audio channel.
 * The output on OUT2 is delayed and the user can expect to hear an echo effect.
 */

#include "ADDS_21479_EzKit.h"
#include "lab5_app.h"          /* LAB5: modifikacija 5 - overdrive */


volatile float * DelayLine;
volatile int Index  = 0;

void	main()
{
	volatile int count=0;

	/* Initialize managed drivers and/or services at the start of main(). */
	adi_initComponents();

	/* Initialize SHARC PLL*/
	initPLL();

	/* Initialize DDR2 SDRAM controller to access memory */
	initExternalMemory();

	/* Initialize DAI because the SPORT and SPI signals need to be routed */
    InitDAI();

    /* This function will configure the AD1939 codec on the 21469 EZ-KIT */
    init1939viaSPI();

    /* LAB5: inicijalizacija overdrive-a, tastera i LED dioda + benchmark
     * blokova (ispis BENCH linija). Mora biti POSLIJE InitDAI(), jer
     * clearDAIpins() resetuje LED pinove, a PRIJE ukljucivanja prekida. */
    lab5_init();
    lab5_benchmark();

    /* LAB5: tabela i bafer za eho premjesteni PRIJE instaliranja prekida -
     * u originalu je prekid mogao doci dok je DelayLine jos NULL. */
    SinTableInit();
    DelayLine =(float *) malloc(DELAYLINESIZE * sizeof(float));

    /* Turn on SPORT0 TX and SPORT1 RX for Multichannel Operation */
	enable_SPORT01_MCM_mode();


	/* Install and enable a handler for the SPORT1 Receiver interrupt.*/
	adi_int_InstallHandler(ADI_CID_P3I,process_AD1939_samples,0,true);


	/* Enable multichannel operation (SPORT mode and DMA in standby and ready) */
	*pSPMCTL0 |= MCEA;
	*pSPMCTL1 |= MCEA;

	/* LAB5: SinTableInit() i malloc za DelayLine su premjesteni gore */
//	DelayLine = (float *) 0x000C4000;

	for (;;)
	  {
		if(count==10)
			puts("Talkthrough is running successfully");
		count++;
    	asm("idle;");          /* ceka sljedeci prekid (odmjerak) */
    	lab5_background();     /* LAB5: tasteri + LOG ispis ciklusa */
      }
}

