#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h> // used for true/false
#include "xparameters.h" //contains hardware addresses and bit masks
#include "xil_cache.h" // cache drivers
#include "xintc.h" // interrupt drivers
#include "xtmrctr.h" // timer drivers
#include "xtmrctr_l.h" // low-level timer drivers
#include "xil_printf.h" // used for xil_printf()
#include "xgpio.h" // used for general purpose I/O
#include "sevenSeg_new.h"

/*
 * The following constant is used to determine which channel of the GPIO is
 * used for the LED if there are 2 channels supported.
 */
#define LED_CHANNEL 1
#define FREQUENCY 100000000 // default frequency 100MHz

void init_intc();
void init_tmrctr();
void init_btngpio();
void init_method();

void rst_tmr();
void tmrint_handler();
void start();
void stop();
void reset();
void btnint_handler();

/*
 * The following are declared globally so they are zeroed and so they are
 * easily accessible from a debugger
 */
XIntc sys_intc; /* The Instance of the Interrupt Controller Driver */
XTmrCtr sys_tmrctr; /* The Instance of the Timer Controller Driver */
XGpio led_gpio; /* The Instance of the GPIO Driver for LED */
XGpio btn_gpio; /* The Instance of the GPIO Driver for buttons */

unsigned int int_count = 0;
float start_time = 0, stop_time = 0;
bool active = false, up = true;

static const int digit_segs[10] =  {
		0b0111111,	//0 //GNU C extension allows binary constants
		0b0000110,	//1
		0b1011011,  //2
		0b1001111,  //3
		0b1100110,  //4
		0b1101101,  //5
		0b1111101,  //6
		0b0000111,  //7
		0b1111111,  //8
		0b1101111,  //9
};

int main () {
	Xil_ICacheInvalidate();
	Xil_ICacheEnable();
	Xil_DCacheInvalidate();
	Xil_DCacheEnable();

	xil_printf("---Entering main---\n\r");

	// Interrupt routine
	init_method();

	// Reset timer
	rst_tmr();

	//INITIALIZATION FOR AXI GPIO LED PORT
	XGpio_Initialize(&led_gpio, XPAR_AXI_GPIO_LED_DEVICE_ID);

	// Grand Loop
	while(true) {
		float display_time = stop_time;

		if(active) {
			if (up)
				display_time += ((float) 0xFFFFFFFF * int_count + XTmrCtr_GetTimerCounterReg(XPAR_TMRCTR_0_BASEADDR, 1) - start_time) / FREQUENCY;
			else {
				display_time -= ((float) 0xFFFFFFFF * int_count + XTmrCtr_GetTimerCounterReg(XPAR_TMRCTR_0_BASEADDR, 1) - start_time) / FREQUENCY;

				if (display_time <= 0) {
					stop();
					reset();
					up = true;
				}
			}
		}

		// Display time on 7-segment displays
		sevenseg_draw_digit(7, ((int)display_time % 10000) / 1000); // MSB
		sevenseg_draw_digit(6, ((int)display_time % 1000) / 100);
		sevenseg_draw_digit(5, ((int)display_time % 100) / 10);
		sevenseg_draw_digit(4, (int) display_time % 10);
		sevenseg_draw_digit(3, (int)(display_time * 10) % 10);
		sevenseg_draw_digit(2, (int)(display_time * 100) % 10);
		sevenseg_draw_digit(1, (int)(display_time * 1000) % 10);
		sevenseg_draw_digit(0, (int)(display_time * 10000) % 10); // LSB
	}

	return 0;
}

void sevenseg_draw_digit (int position, int value)
{
	int segs, segs_mask, digit_mask, combined_mask;

	segs  = digit_segs[value];
	segs_mask = 127^segs;
	digit_mask = 255 ^ (1<<position);
   	combined_mask = segs_mask | (digit_mask << 7);

   	Xil_Out32(XPAR_SEVENSEG_0_S00_AXI_BASEADDR, combined_mask);

   	return;
}

void init_intc() {
	/*
	 * Initialize the interrupt controller driver so that
	 * it is ready to use, specify the device ID that is generated in
	 * xparameters.h
	 */
	XIntc_Initialize(&sys_intc, XPAR_MICROBLAZE_0_AXI_INTC_DEVICE_ID);
	/*
	 * Connect the application handler that will be called when an interrupt
	 * for the timer occurs
	 */
	XIntc_Connect(&sys_intc, XPAR_MICROBLAZE_0_AXI_INTC_AXI_TIMER_0_INTERRUPT_INTR, (XInterruptHandler)tmrint_handler, &sys_tmrctr);
	XIntc_Connect(&sys_intc, XPAR_MICROBLAZE_0_AXI_INTC_AXI_GPIO_BTN_IP2INTC_IRPT_INTR, (XInterruptHandler)btnint_handler, &btn_gpio);
	/*
	 * Start the interrupt controller such that interrupts are enabled for
	 * all devices that cause interrupts.
	 */
	XIntc_Start(&sys_intc, XIN_REAL_MODE);
	/*
	 * Enable the interrupt for the timer counter
	 */
	XIntc_Enable(&sys_intc, XPAR_MICROBLAZE_0_AXI_INTC_AXI_TIMER_0_INTERRUPT_INTR);
	XIntc_Enable(&sys_intc, XPAR_MICROBLAZE_0_AXI_INTC_AXI_GPIO_BTN_IP2INTC_IRPT_INTR);

	return;
}

void init_tmrctr() {
	/*
	 * Initialize the timer counter so that it's ready to use,
	 * specify the device ID that is generated in xparameters.h
	 */
	XTmrCtr_Initialize(&sys_tmrctr, XPAR_MICROBLAZE_0_AXI_INTC_DEVICE_ID);
	/*
	 * Enable the interrupt of the timer counter so interrupts will occur
	 * and use auto reload mode such that the timer counter will reload
	 * itself automatically and continue repeatedly, without this option
	 * it would expire once only
	 */
	XTmrCtr_SetOptions(&sys_tmrctr, 0, XTC_INT_MODE_OPTION | XTC_AUTO_RELOAD_OPTION);
	/*
	 * Set a reset value for the timer counter such that it will expire
	 * earlier than letting it roll over from 0, the reset value is loaded
	 * into the timer counter when it is started
	 */
	XTmrCtr_SetResetValue(&sys_tmrctr, 0, 0);
	/*
	 * Start the timer counter such that it's incrementing by default,
	 * then wait for it to timeout a number of times
	 */
	XTmrCtr_Start(&sys_tmrctr, 0);

	return;
}

void init_btngpio() {
	/* Initialize the GPIO driver */
	XGpio_Initialize(&btn_gpio, XPAR_AXI_GPIO_BTN_DEVICE_ID);
	/* This function ensures interrupts are configured for this device then enables the interrupt. */
	XGpio_InterruptEnable(&btn_gpio, 1);
	/* This function allows for interrupt output signals to be passed through to the microblaze */
	XGpio_InterruptGlobalEnable(&btn_gpio);
	/* This function should be called after the software has serviced the interrupts that are pending. */
	XGpio_InterruptClear(&btn_gpio, XPAR_AXI_GPIO_BTN_DEVICE_ID);

	return;
}

void init_method() {
	/* Set up interrupt controller */
	init_intc();
	/* Set up timer interrupt */
	init_tmrctr();
	/* Set up GPIO button interrupt */
	init_btngpio();

	/*
	 * Register the intc device driver’s handler with the Standalone
	 * software platform’s interrupt table
	 */
	microblaze_register_handler((XInterruptHandler)XIntc_DeviceInterruptHandler, (void*)XPAR_MICROBLAZE_0_AXI_INTC_DEVICE_ID);
	/*
	 * Enable interrupts on MicroBlaze
	 */
	microblaze_enable_interrupts();

	return;
}

void rst_tmr() {
	/* Turn off the timer */
	XTmrCtr_SetControlStatusReg(XPAR_TMRCTR_0_BASEADDR, 1, 0);
	/* Put a 0 in the load register */
	XTmrCtr_SetLoadReg(XPAR_TMRCTR_0_BASEADDR, 1, 0);
	/* Copy the load register into the counter register */
	XTmrCtr_SetControlStatusReg(XPAR_TMRCTR_0_BASEADDR, 1, XTC_CSR_LOAD_MASK);
	/* Enable (start) the timer */
	XTmrCtr_SetControlStatusReg(XPAR_TMRCTR_0_BASEADDR, 1, XTC_CSR_ENABLE_TMR_MASK);

	return;
}

void tmrint_handler() {
	// increment interrupt count
	++int_count;
	// This is the interrupt handler function
	// Do not print inside of this function.
	uint32_t ControlStatusReg;
	/*
	 * Read the new Control/Status Register content.
	 */
	ControlStatusReg = XTimerCtr_ReadReg(sys_tmrctr.BaseAddress, 0, XTC_TCSR_OFFSET);
	/*
	 * Acknowledge the interrupt by clearing the interrupt
	 * bit in the timer control status register
	 */
	XTmrCtr_WriteReg(sys_tmrctr.BaseAddress, 0, XTC_TCSR_OFFSET, ControlStatusReg | XTC_CSR_INT_OCCURED_MASK);

	rst_tmr();

	return;
}

void start() {
	if (!active) {
		int_count = 0;
		/* Store the timer value before executing the operation being timed */
		start_time = XTmrCtr_GetTimerCounterReg(XPAR_TMRCTR_0_BASEADDR, 1);
		/* Enable (start) the timer */
		XTmrCtr_SetControlStatusReg(XPAR_TMRCTR_0_BASEADDR, 1, XTC_CSR_ENABLE_TMR_MASK);
		active = true;
	}

	return;
}

void stop() {
	if (active) {
		/* Turn off the timer */
		XTmrCtr_SetControlStatusReg(XPAR_TMRCTR_0_BASEADDR, 1, 0);
		active = false;

		if (up)
			stop_time += ((double) 0xFFFFFFFF * int_count + XTmrCtr_GetTimerCounterReg(XPAR_TMRCTR_0_BASEADDR, 1) - start_time) / FREQUENCY;
		else
			stop_time -= ((double) 0xFFFFFFFF * int_count + XTmrCtr_GetTimerCounterReg(XPAR_TMRCTR_0_BASEADDR, 1) - start_time) / FREQUENCY;

		/* Put stopped time in the load register */
		XTmrCtr_SetLoadReg(XPAR_TMRCTR_0_BASEADDR, 1, stop_time);
		/* Copy the load register into the counter register */
		XTmrCtr_SetControlStatusReg(XPAR_TMRCTR_0_BASEADDR, 1, XTC_CSR_LOAD_MASK);
	}

	return;
}

void reset() {
	int_count = 0;
	stop_time = 0;
	rst_tmr();
	/* Store the timer value before executing the operation being timed */
	start_time = XTmrCtr_GetTimerCounterReg(XPAR_TMRCTR_0_BASEADDR, 1);

	return;
}

void btnint_handler() {
	// Disable GPIO interrupts
	XGpio_InterruptDisable(&btn_gpio, XPAR_AXI_GPIO_BTN_DEVICE_ID);
	// Ignore additional button presses
	if ((XGpio_InterruptGetStatus(&btn_gpio) & XPAR_AXI_GPIO_BTN_DEVICE_ID) != XPAR_AXI_GPIO_BTN_DEVICE_ID)
		return;

	unsigned int btn_value = XGpio_DiscreteRead(&btn_gpio, 1); // gets button address
	switch (btn_value) {
		case 1:
			// count up (BTNU)
			if (!up) {
				if (active) {
					stop();
					up = true;
					start();
				} else {
					stop();
					up = true;
				}
			}
			break;
		case 2:
			// start (BTNL)
			start();
			break;
		case 4:
			// stop (BTNR)
			stop();
			break;
		case 8:
			// count down (BTND)
			if (up) {
				if (active) {
					stop();
					up = false;
					start();
				} else {
					stop();
					up = false;
				}
			}
			break;
		case 16:
			// reset (BTNC)
			reset(); // needs to be functional when active
			break;
	}

	XGpio_DiscreteWrite(&led_gpio, LED_CHANNEL, btn_value);
	XGpio_InterruptEnable(&btn_gpio, XPAR_AXI_GPIO_BTN_DEVICE_ID);

	return;
}
