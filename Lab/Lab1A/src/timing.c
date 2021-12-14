/*
 *timing.c: simple starter application for lab 1A and 1B
 *
 */

#include <stdio.h>		// Used for printf()
#include <stdlib.h>		// Used for rand()
#include "xparameters.h"	// Contains hardware addresses and bit masks
#include "xil_cache.h"		// Cache Drivers
#include "xintc.h"		// Interrupt Drivers
#include "xtmrctr.h"		// Timer Drivers
#include "xtmrctr_l.h" 		// Low-level timer drivers
#include "xil_printf.h" 	// Used for xil_printf()
#include "extra.h" 		// Provides a source of bus contention
#include "xgpio.h" 		// LED driver, used for General purpose I/i

#define NUMBER_OF_TRIALS 10000
#define NUMBER_OF_BINS 50
#define BUFFER_SIZE (1024*1024)
unsigned int buffer[BUFFER_SIZE]; //buffer for read/write operations to the DDR memory

/*
 * The following constant is used to determine which channel of the GPIO is
 * used for the LED if there are 2 channels supported.
 */
#define LED_CHANNEL 1


void histogram(void); // This function creates a histogram for the measured data

/*
 * The following are declared globally so they are zeroed and so they are
 * easily accessible from a debugger
 */
XGpio Gpio; /* The Instance of the GPIO Driver used for LED 0 */

/*
 * This globally declared array stores the
 * number of clock cycles for all the trials.
 * With global declaration, it is stored in the data segment of the
 * memory. Declaring this large array locally may cause a stack overflow.
 */
int numClockCycles[NUMBER_OF_TRIALS];

//Stores the number of samples in each bin
int histData[NUMBER_OF_BINS];


//BaseAddr points to the base (byte) address of the DDR2 Memory
u8 * BaseAddr = (u8 *) XPAR_MIG7SERIES_0_BASEADDR;


int main()
{
	Xil_ICacheInvalidate();
	Xil_ICacheEnable();
	Xil_DCacheInvalidate();
	Xil_DCacheEnable();
	print("---Entering main---\n\r");
	int i = 0;
	int timer_val_before; //Used to store the timer value before executing the operation being timed
	u32 Addr;
	volatile unsigned int Data;
	volatile float fData;


	// Extra Method contains an interrupt routine which is set to go off at timed intervals
    	extra_method();


 	//TIMER RESET CODE
		//Turn off the timer
		XTmrCtr_SetControlStatusReg(XPAR_TMRCTR_0_BASEADDR, 1, 0);
		//Put a zero in the load register
		XTmrCtr_SetLoadReg(XPAR_TMRCTR_0_BASEADDR, 1, 0);
		//Copy the load register into the counter register
		XTmrCtr_SetControlStatusReg(XPAR_TMRCTR_0_BASEADDR, 1, XTC_CSR_LOAD_MASK);
 		//Enable (start) the timer
 		XTmrCtr_SetControlStatusReg(XPAR_TMRCTR_0_BASEADDR, 1,	XTC_CSR_ENABLE_TMR_MASK);
 		//END TIMER RESET CODE

 	//INITIALIZATION FOR AXI GPIO LED PORT
 		XGpio_Initialize(&Gpio, XPAR_AXI_GPIO_LED_DEVICE_ID);

    for( i=0; i < NUMBER_OF_TRIALS; i++) {

    	Addr = rand()%BUFFER_SIZE; //Will be used to access a random buffer index

    	timer_val_before = XTmrCtr_GetTimerCounterReg(XPAR_TMRCTR_0_BASEADDR, 1); //Store the timer value before executing the operation being timed

 		// Enter the line of Code to time.

    	/*
    	// Section 5.1
    	Data = 0 + 1;
    	Data = 1 + 2;
    	Data = 3 + 3;
    	Data = 6 + 4;
    	Data = 10 + 5;
    	Data = 15 + 6;
    	Data = 21 + 7;
    	Data = 28 + 8;
    	Data = 36 + 9;
    	Data = 45 + 10;
    	Data = 55 + 11;
    	Data = 66 + 12;
    	Data = 78 + 13;
    	Data = 91 + 14;
    	Data = 105 + 15;
    	Data = 120 + 16;
    	Data = 136 + 17;
    	Data = 153 + 18;
    	Data = 171 + 19;
    	Data = 190 + 20;
    	Data = 210 + 21;
    	Data = 231 + 22;
    	Data = 253 + 23;
    	Data = 276 + 24;
    	Data = 300 + 25;
    	Data = 325 + 26;
    	Data = 351 + 27;
    	Data = 378 + 28;
    	Data = 406 + 29;
    	Data = 435 + 30;
    	Data = 465 + 31;
    	Data = 496 + 32;
    	Data = 528 + 33;
    	Data = 561 + 34;
    	Data = 595 + 35;
    	Data = 630 + 36;
    	Data = 666 + 37;
    	Data = 703 + 38;
    	Data = 741 + 39;
    	Data = 780 + 40;
    	*/
    	/*
    	// Section 5.2
    	fData = 0.301 + 0.4771;
    	fData = 0.301 + 0.4771;
    	fData = 0.301 + 0.4771;
    	fData = 0.301 + 0.4771;
    	fData = 0.301 + 0.4771; //5
    	fData = 0.301 + 0.4771;
    	fData = 0.301 + 0.4771;
    	fData = 0.301 + 0.4771;
    	fData = 0.301 + 0.4771;
    	fData = 0.301 + 0.4771;
    	fData = 0.301 + 0.4771;
    	fData = 0.301 + 0.4771;
    	fData = 0.301 + 0.4771;
    	fData = 0.301 + 0.4771;
    	fData = 0.301 + 0.4771; //15
    	fData = 0.301 + 0.4771;
    	fData = 0.301 + 0.4771;
    	fData = 0.301 + 0.4771;
    	fData = 0.301 + 0.4771;
    	fData = 0.301 + 0.4771;
    	fData = 0.301 + 0.4771;
    	fData = 0.301 + 0.4771;
    	fData = 0.301 + 0.4771;
    	fData = 0.301 + 0.4771;
    	fData = 0.301 + 0.4771; //25
    	fData = 0.301 + 0.4771;
    	fData = 0.301 + 0.4771;
    	fData = 0.301 + 0.4771;
    	fData = 0.301 + 0.4771;
    	fData = 0.301 + 0.4771;
    	fData = 0.301 + 0.4771;
    	fData = 0.301 + 0.4771;
    	fData = 0.301 + 0.4771;
    	fData = 0.301 + 0.4771;
    	fData = 0.301 + 0.4771;
    	fData = 0.301 + 0.4771;
    	fData = 0.301 + 0.4771;
    	fData = 0.301 + 0.4771;
    	fData = 0.301 + 0.4771;
    	fData = 0.301 + 0.4771; //40
		*/
    	/*
    	// Section 5.3
    	XGpio_DiscreteWrite(&Gpio, LED_CHANNEL, 0x1); //Turns on one LED
    	XGpio_DiscreteWrite(&Gpio, LED_CHANNEL, 0x1);
    	XGpio_DiscreteWrite(&Gpio, LED_CHANNEL, 0x1);
    	XGpio_DiscreteWrite(&Gpio, LED_CHANNEL, 0x1);
    	XGpio_DiscreteWrite(&Gpio, LED_CHANNEL, 0x1); //5
    	XGpio_DiscreteWrite(&Gpio, LED_CHANNEL, 0x1);
    	XGpio_DiscreteWrite(&Gpio, LED_CHANNEL, 0x1);
    	XGpio_DiscreteWrite(&Gpio, LED_CHANNEL, 0x1);
    	XGpio_DiscreteWrite(&Gpio, LED_CHANNEL, 0x1);
    	XGpio_DiscreteWrite(&Gpio, LED_CHANNEL, 0x1);
    	XGpio_DiscreteWrite(&Gpio, LED_CHANNEL, 0x1);
    	XGpio_DiscreteWrite(&Gpio, LED_CHANNEL, 0x1);
    	XGpio_DiscreteWrite(&Gpio, LED_CHANNEL, 0x1);
    	XGpio_DiscreteWrite(&Gpio, LED_CHANNEL, 0x1);
    	XGpio_DiscreteWrite(&Gpio, LED_CHANNEL, 0x1); //15
    	XGpio_DiscreteWrite(&Gpio, LED_CHANNEL, 0x1);
    	XGpio_DiscreteWrite(&Gpio, LED_CHANNEL, 0x1);
    	XGpio_DiscreteWrite(&Gpio, LED_CHANNEL, 0x1);
    	XGpio_DiscreteWrite(&Gpio, LED_CHANNEL, 0x1);
    	XGpio_DiscreteWrite(&Gpio, LED_CHANNEL, 0x1);
    	XGpio_DiscreteWrite(&Gpio, LED_CHANNEL, 0x1);
    	XGpio_DiscreteWrite(&Gpio, LED_CHANNEL, 0x1);
    	XGpio_DiscreteWrite(&Gpio, LED_CHANNEL, 0x1);
    	XGpio_DiscreteWrite(&Gpio, LED_CHANNEL, 0x1);
    	XGpio_DiscreteWrite(&Gpio, LED_CHANNEL, 0x1); //25
    	XGpio_DiscreteWrite(&Gpio, LED_CHANNEL, 0x1);
    	XGpio_DiscreteWrite(&Gpio, LED_CHANNEL, 0x1);
    	XGpio_DiscreteWrite(&Gpio, LED_CHANNEL, 0x1);
    	XGpio_DiscreteWrite(&Gpio, LED_CHANNEL, 0x1);
    	XGpio_DiscreteWrite(&Gpio, LED_CHANNEL, 0x1);
    	XGpio_DiscreteWrite(&Gpio, LED_CHANNEL, 0x1);
    	XGpio_DiscreteWrite(&Gpio, LED_CHANNEL, 0x1);
    	XGpio_DiscreteWrite(&Gpio, LED_CHANNEL, 0x1);
    	XGpio_DiscreteWrite(&Gpio, LED_CHANNEL, 0x1);
    	XGpio_DiscreteWrite(&Gpio, LED_CHANNEL, 0x1);
    	XGpio_DiscreteWrite(&Gpio, LED_CHANNEL, 0x1);
    	XGpio_DiscreteWrite(&Gpio, LED_CHANNEL, 0x1);
    	XGpio_DiscreteWrite(&Gpio, LED_CHANNEL, 0x1);
    	XGpio_DiscreteWrite(&Gpio, LED_CHANNEL, 0x1);
    	XGpio_DiscreteWrite(&Gpio, LED_CHANNEL, 0x1); //40
    	*/

    	// Section 5.4
    	Data = buffer[Addr];
    	Data = buffer[Addr];
    	Data = buffer[Addr];
    	Data = buffer[Addr];
    	Data = buffer[Addr]; //5
    	/*
    	Data = buffer[Addr];
    	Data = buffer[Addr];
    	Data = buffer[Addr];
    	Data = buffer[Addr];
    	Data = buffer[Addr];
    	Data = buffer[Addr];
    	Data = buffer[Addr];
    	Data = buffer[Addr];
    	Data = buffer[Addr];
    	Data = buffer[Addr]; //15
    	Data = buffer[Addr];
    	Data = buffer[Addr];
    	Data = buffer[Addr];
    	Data = buffer[Addr];
    	Data = buffer[Addr];
    	Data = buffer[Addr];
    	Data = buffer[Addr];
    	Data = buffer[Addr];
    	Data = buffer[Addr];
    	Data = buffer[Addr]; //25
    	Data = buffer[Addr];
    	Data = buffer[Addr];
    	Data = buffer[Addr];
    	Data = buffer[Addr];
    	Data = buffer[Addr];
    	Data = buffer[Addr];
    	Data = buffer[Addr];
    	Data = buffer[Addr];
    	Data = buffer[Addr];
    	Data = buffer[Addr];
    	Data = buffer[Addr];
    	Data = buffer[Addr];
    	Data = buffer[Addr];
    	Data = buffer[Addr];
    	Data = buffer[Addr]; //40
    	*/
    	/*
    	// Section 5.5.a
    	printf("%f\n\r", 0.1);
    	printf("%f\n\r", 0.2);
    	printf("%f\n\r", 0.3);
    	printf("%f\n\r", 0.4);
    	printf("%f\n\r", 0.5); //5
    	printf("%f\n\r", 0.1);
    	printf("%f\n\r", 0.2);
    	printf("%f\n\r", 0.3);
    	printf("%f\n\r", 0.4);
    	printf("%f\n\r", 0.5);
    	printf("%f\n\r", 0.1);
    	printf("%f\n\r", 0.2);
    	printf("%f\n\r", 0.3);
    	printf("%f\n\r", 0.4);
    	printf("%f\n\r", 0.5); //15
    	printf("%f\n\r", 0.1);
    	printf("%f\n\r", 0.2);
    	printf("%f\n\r", 0.3);
    	printf("%f\n\r", 0.4);
    	printf("%f\n\r", 0.5);
    	printf("%f\n\r", 0.1);
    	printf("%f\n\r", 0.2);
    	printf("%f\n\r", 0.3);
    	printf("%f\n\r", 0.4);
    	printf("%f\n\r", 0.5); //25
    	printf("%f\n\r", 0.1);
    	printf("%f\n\r", 0.2);
    	printf("%f\n\r", 0.3);
    	printf("%f\n\r", 0.4);
    	printf("%f\n\r", 0.5);
    	printf("%f\n\r", 0.1);
    	printf("%f\n\r", 0.2);
    	printf("%f\n\r", 0.3);
    	printf("%f\n\r", 0.4);
    	printf("%f\n\r", 0.5);
    	printf("%f\n\r", 0.1);
    	printf("%f\n\r", 0.2);
    	printf("%f\n\r", 0.3);
    	printf("%f\n\r", 0.4);
    	printf("%f\n\r", 0.5); //40
    	*/
    	/*
    	// Section 5.5.b
    	xil_printf("HelloWorld\n\r");
    	xil_printf("HelloWorld\n\r");
    	xil_printf("HelloWorld\n\r");
    	xil_printf("HelloWorld\n\r");
    	xil_printf("HelloWorld\n\r"); //5
    	xil_printf("HelloWorld\n\r");
    	xil_printf("HelloWorld\n\r");
    	xil_printf("HelloWorld\n\r");
    	xil_printf("HelloWorld\n\r");
    	xil_printf("HelloWorld\n\r");
    	xil_printf("HelloWorld\n\r");
    	xil_printf("HelloWorld\n\r");
    	xil_printf("HelloWorld\n\r");
    	xil_printf("HelloWorld\n\r");
    	xil_printf("HelloWorld\n\r"); //15
    	xil_printf("HelloWorld\n\r");
    	xil_printf("HelloWorld\n\r");
    	xil_printf("HelloWorld\n\r");
    	xil_printf("HelloWorld\n\r");
    	xil_printf("HelloWorld\n\r");
    	xil_printf("HelloWorld\n\r");
    	xil_printf("HelloWorld\n\r");
    	xil_printf("HelloWorld\n\r");
    	xil_printf("HelloWorld\n\r");
    	xil_printf("HelloWorld\n\r"); //25
    	xil_printf("HelloWorld\n\r");
    	xil_printf("HelloWorld\n\r");
    	xil_printf("HelloWorld\n\r");
    	xil_printf("HelloWorld\n\r");
    	xil_printf("HelloWorld\n\r");
    	xil_printf("HelloWorld\n\r");
    	xil_printf("HelloWorld\n\r");
    	xil_printf("HelloWorld\n\r");
    	xil_printf("HelloWorld\n\r");
    	xil_printf("HelloWorld\n\r");
    	xil_printf("HelloWorld\n\r");
    	xil_printf("HelloWorld\n\r");
    	xil_printf("HelloWorld\n\r");
    	xil_printf("HelloWorld\n\r");
    	xil_printf("HelloWorld\n\r"); //40
    	*/

 		numClockCycles[i] = XTmrCtr_GetTimerCounterReg(XPAR_TMRCTR_0_BASEADDR, 1) - timer_val_before; //Stores the time to execute the operation

     }

    //Prints the collected data
    for (i=0; i < NUMBER_OF_TRIALS; i++ ) {
 		xil_printf("%d,%d\n\r", i,numClockCycles[i]);
    }

    histogram(); //Creates a histogram for the measured data

}


void histogram(void){

	int min, max, binSize, binIndex;

	int i;

	//min and max initialized
	min = numClockCycles[0];
	max = numClockCycles[0];

	//find the min and max values
	for (i=0; i<NUMBER_OF_TRIALS; i++)
	{
		if (numClockCycles[i] < min) min = numClockCycles[i];
		if (numClockCycles[i] > max) max = numClockCycles[i];
	}

	binSize = (max - min)/NUMBER_OF_BINS;

	//Bin number for each data element is found here
	for (i=0; i<NUMBER_OF_TRIALS; i++)
	{
		binIndex = 0;
		if (binSize > 0) {
			binIndex = (numClockCycles[i] - min)/binSize;
			if (binIndex >= NUMBER_OF_BINS) binIndex = NUMBER_OF_BINS - 1 ;
			histData[binIndex]++;
		}
		else {
			//if there is no variance in the data all values are assigned to bin 0
			histData[0] = NUMBER_OF_TRIALS;
			break;
		}
	}
	//Prints the number of elements in each bin
	for (i=0; i<NUMBER_OF_BINS; i++)
	{
		xil_printf("Bin %d: %d\n\r",i,histData[i]);
	}

	xil_printf("Done!\n\r");


}