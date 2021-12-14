#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "xparameters.h" //contains hardware addresses and bit masks
#include "xil_printf.h" // used for xil_printf()
#include "xil_cache.h" // cache drivers
#include "xintc.h" // interrupt drivers
#include "xtmrctr.h" // timer drivers
#include "xtmrctr_l.h" // low-level timer drivers
#include "xgpio.h" // used for general purpose I/O

/*
 * The following constant is used to determine which channel of the GPIO is
 * used for the LED if there are 2 channels supported.
 */
#define LED_CHANNEL 1
#define FREQUENCY 100000000 // default frequency 100MHz

void init_intc(void);
void init_tmrctr(void);
void init_encodergpio(void);
void init_method(void);

void rst_tmr(void);
void tmrint_handler(void);
void encoderint_handler(void);

/*
 * The following are declared globally so they are zeroed and so they are
 * easily accessible from a debugger
 */
XIntc sys_intc; /* The Instance of the Interrupt Controller Driver */
XTmrCtr sys_tmrctr; /* The Instance of the Timer Controller Driver */
XGpio led_gpio; /* The Instance of the GPIO Driver for LED */
XGpio rgbled_gpio; /* The Instance of the GPIO Driver for RGB LED */
XGpio encoder_gpio; /* The Instance of the GPIO Driver for encoder */

bool active = true;
unsigned int led_address = 1;

/*
 * CW00:  0b000
 * CW01:  0b001
 * CW11:  0b011
 * CW10:  0b010
 * CCW10: 0b110
 * CCW11: 0b111
 * CCW01: 0b101
 * CCW00: 0b100
 */
enum State {CW00, CW01, CW10, CW11, CCW00, CCW01, CCW10, CCW11};
enum State state = CW00; // reset state

int main()
{
	Xil_ICacheInvalidate();
	Xil_ICacheEnable();
	Xil_DCacheInvalidate();
	Xil_DCacheEnable();

	xil_printf("---Entering main---\n\r");

	/* Interrupt routine */
	init_method();

	/* Reset timer */
	rst_tmr();

	//INITIALIZATION FOR AXI GPIO LED PORT
	XGpio_Initialize(&led_gpio, XPAR_AXI_GPIO_LED_DEVICE_ID);

	//INITIALIZATION FOR AXI GPIO RGB LED PORT
	XGpio_Initialize(&rgbled_gpio, XPAR_AXI_GPIO_RGBLED_DEVICE_ID);

	// Grand loop
	while(true) {
		if (active) {
			XGpio_DiscreteWrite(&led_gpio, LED_CHANNEL, led_address);

			// RGB blinking as active indicator
			if ((XTmrCtr_GetTimerCounterReg(XPAR_TMRCTR_0_BASEADDR, 1) / FREQUENCY) % 2)
				XGpio_DiscreteWrite(&rgbled_gpio, LED_CHANNEL, 0b010); /* R: 0b100, G: 0b010, B: 0b001 */
			else
				XGpio_DiscreteWrite(&rgbled_gpio, LED_CHANNEL, 0b000);

		} else {
			XGpio_DiscreteWrite(&led_gpio, LED_CHANNEL, 0);
			XGpio_DiscreteWrite(&rgbled_gpio, LED_CHANNEL, 0b000);
		}
	}

    return 0;
}

void init_intc(void) {
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
	XIntc_Connect(&sys_intc, XPAR_MICROBLAZE_0_AXI_INTC_AXI_GPIO_ENCODER_IP2INTC_IRPT_INTR, (XInterruptHandler)encoderint_handler, &encoder_gpio);
	/*
	 * Start the interrupt controller such that interrupts are enabled for
	 * all devices that cause interrupts.
	 */
	XIntc_Start(&sys_intc, XIN_REAL_MODE);
	/*
	 * Enable the interrupt for the timer counter
	 */
	XIntc_Enable(&sys_intc, XPAR_MICROBLAZE_0_AXI_INTC_AXI_TIMER_0_INTERRUPT_INTR);
	XIntc_Enable(&sys_intc, XPAR_MICROBLAZE_0_AXI_INTC_AXI_GPIO_ENCODER_IP2INTC_IRPT_INTR);

	return;
}

void init_tmrctr(void) {
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

void init_encodergpio(void) {
	/* Initialize the GPIO driver */
	XGpio_Initialize(&encoder_gpio, XPAR_AXI_GPIO_ENCODER_DEVICE_ID);
	/* This function ensures interrupts are configured for this device then enables the interrupt. */
	XGpio_InterruptEnable(&encoder_gpio, 1);
	/* This function allows for interrupt output signals to be passed through to the microblaze */
	XGpio_InterruptGlobalEnable(&encoder_gpio);
	/* This function should be called after the software has serviced the interrupts that are pending. */
	XGpio_InterruptClear(&encoder_gpio, XPAR_AXI_GPIO_ENCODER_DEVICE_ID);

	return;
}

void init_method(void) {
	xil_printf("Start init_method\r\n");
	/* Set up interrupt controller */
	init_intc();
	/* Set up timer interrupt */
	init_tmrctr();
	/* Set up GPIO button interrupt */
	init_encodergpio();

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

void rst_tmr(void) {
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

void tmrint_handler(void) {
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

void encoderint_handler(void) {
	/*
	// Disable GPIO interrupts
	XGpio_InterruptDisable(&encoder_gpio, XPAR_AXI_GPIO_ENCODER_DEVICE_ID);
	// Ignore additional button presses
	if ((XGpio_InterruptGetStatus(&encoder_gpio) & XPAR_AXI_GPIO_ENCODER_DEVICE_ID) != XPAR_AXI_GPIO_ENCODER_DEVICE_ID)
		return;
	*/
	unsigned short encoder = XGpio_DiscreteRead(&encoder_gpio, 1); /* encoder = JD[2:0] = {BTN, B, A} */

	/* Check if BTN is pressed */
	if (encoder & 0b100) {
		active = !active;
		XTmrCtr_GetTimerCounterReg(XPAR_TMRCTR_0_BASEADDR, 1); /* Debouncing the push button requires a timer or timing loop. */
	}

	if (active) {
		switch ((state << 2) + (encoder & 0b11)) {
			case 0b00001: case 0b10001: state = CW01; break;
			case 0b00100: state = CW00; break;
			case 0b00111: state = CW11; break;
			case 0b01101: state = CW01; break;
			case 0b01110: state = CW10; break;
			case 0b01011: state = CW11; break;
			case 0b01000: state = CW00; led_address >>= 1; break;
			case 0b00010: case 0b10010: state = CCW10; break;
			case 0b11000: state = CCW00; break;
			case 0b11011: state = CCW11; break;
			case 0b11110: state = CCW10; break;
			case 0b11101: state = CCW01; break;
			case 0b10111: state = CCW11; break;
			case 0b10100: state = CCW00; led_address <<= 1; break;
		}

		if (led_address == 0) {
			/*  At the left-most position, wrap to the right-most position. */
			led_address = 0x8000; // 0b1000_0000_0000_0000
		} else if (led_address == 0x10000) { // 0b0001_0000_0000_0000_0000
			/* At the right-most position, wrap to the left-most position. */
			led_address = 1;
		}
	}

	XGpio_InterruptClear(&encoder_gpio, XPAR_MDM_1_INTERRUPT_MASK);
	//XGpio_InterruptEnable(&encoder_gpio, XPAR_AXI_GPIO_ENCODER_DEVICE_ID);

	return;
}
