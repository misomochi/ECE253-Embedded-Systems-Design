/*
 * init.c
 *
 *  Created on: Dec 2, 2021
 *      Author: Henry Chang, Ci-Chian Lu
 */

#include "init.h"

// Create ONE interrupt controllers XIntc
XIntc sys_intc;
// Create ONE timer controllers XTmrCtr
XTmrCtr sys_tmrctr;
// Create static XGpio variables
static XGpio led_gpio;
static XGpio btn_gpio;
// Create static XSpi variables
static XSpi lcd_spi;
XSpi_Config *configPtr; /* Pointer to Configuration data */

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
	XIntc_Connect(&sys_intc, XPAR_MICROBLAZE_0_AXI_INTC_AXI_GPIO_BTN_IP2INTC_IRPT_INTR, (XInterruptHandler)btnint_handler, &btn_gpio);
	//XIntc_Connect(&sys_intc, XPAR_MICROBLAZE_0_AXI_INTC_AXI_GPIO_ENCODER_IP2INTC_IRPT_INTR, (XInterruptHandler)encoderint_handler, &encoder_gpio);

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
	//XIntc_Enable(&sys_intc, XPAR_MICROBLAZE_0_AXI_INTC_AXI_GPIO_ENCODER_IP2INTC_IRPT_INTR);

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
	xil_printf("Timer start!\n\r");

	return;
}

void init_btngpio() {
	/* Initialize the GPIO driver */
	XGpio_Initialize(&btn_gpio, XPAR_AXI_GPIO_BTN_DEVICE_ID);
	/* This function ensures interrupts are configured for this device then enables the interrupt. */
	XGpio_InterruptEnable(&btn_gpio, XPAR_MDM_1_INTERRUPT_MASK);
	/* This function allows for interrupt output signals to be passed through to the microblaze */
	XGpio_InterruptGlobalEnable(&btn_gpio);
	/* This function should be called after the software has serviced the interrupts that are pending. */
	XGpio_InterruptClear(&btn_gpio, XPAR_MDM_1_INTERRUPT_MASK);

	return;
}

void init_lcdspi(void) {
	/*
	 * Initialize the SPI driver so that it is ready to use.
	 */
	configPtr = XSpi_LookupConfig(XPAR_SPI_DEVICE_ID);
	if (configPtr == NULL) {
		xil_printf("Can't find SPI device!\n\r");
		return XST_DEVICE_NOT_FOUND;
	}

	int status = XSpi_CfgInitialize(&lcd_spi, configPtr, configPtr -> BaseAddress);
	if (status != XST_SUCCESS) {
		xil_printf("Initialize SPI fail!\n\r");
		return XST_FAILURE;
	}

	/*
	 * Reset the SPI device to leave it in a known good state.
	 */
	XSpi_Reset(&lcd_spi);

	/*
	 * Setup the control register to enable master mode
	 */
	u32 controlReg = XSpi_GetControlReg(&lcd_spi);
	XSpi_SetControlReg(&lcd_spi, (controlReg | XSP_CR_ENABLE_MASK | XSP_CR_MASTER_MODE_MASK) & (~XSP_CR_TRANS_INHIBIT_MASK));

	// Select 1st slave device
	XSpi_SetSlaveSelectReg(&lcd_spi, ~0x01);

	return;
}

void init_method(void) {
/* Setup LED's, etc */
/* Setup interrupts and reference to interrupt handler function(s)  */
	xil_printf("Starting init_method\n\r");
	/*
	 * Initialize the interrupt controller driver so that it's ready to use.
	 * specify the device ID that was generated in xparameters.h
	 *
	 * Initialize GPIO and connect the interrupt controller to the GPIO.
	 *
	 */
	xil_printf("Starting interrupt initialization\n\r");
	init_intc();
	init_tmrctr();
	init_btngpio();
	//init_encodergpio();
	xil_printf("Finishing interrupt initialization\n\r");

	/*
	 * Register the intc device driver’s handler with the Standalone
	 * software platform’s interrupt table
	 */
	microblaze_register_handler((XInterruptHandler)XIntc_DeviceInterruptHandler, (void*)XPAR_MICROBLAZE_0_AXI_INTC_DEVICE_ID);
	/*
	 * Enable interrupts on MicroBlaze
	 */
	microblaze_enable_interrupts();

	rst_tmr();

	//INITIALIZATION FOR AXI GPIO LED PORT
	XGpio_Initialize(&led_gpio, XPAR_AXI_GPIO_LED_DEVICE_ID);

	// INITIALIZATION FOR SPI DRIVER
	init_lcdspi();
	initLCD();
	clrScr();

	xil_printf("Finishing init_method\n\r");

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

void btnint_handler() {
	unsigned int btn_value = XGpio_DiscreteRead(&btn_gpio, 1); // gets button address

	// Disable GPIO interrupts
	XGpio_InterruptDisable(&btn_gpio, XPAR_AXI_GPIO_BTN_DEVICE_ID);
	// Ignore additional button presses
	if ((XGpio_InterruptGetStatus(&btn_gpio) & XPAR_AXI_GPIO_BTN_DEVICE_ID) != XPAR_AXI_GPIO_BTN_DEVICE_ID)
		return;

	u32 start_time = XTmrCtr_GetTimerCounterReg(XPAR_TMRCTR_0_BASEADDR, 1);

	switch (btn_value) {
		case BTNU:
			if (snakeDir != UP && snakeDir != DOWN && gameState == START) snakeDir = UP;
			break;
		case BTNL:
			if (snakeDir != LEFT && snakeDir != RIGHT && gameState == START) snakeDir = LEFT;
			break;
		case BTNR:
			if (snakeDir != RIGHT && snakeDir != LEFT && gameState == START) snakeDir = RIGHT;
			break;
		case BTND:
			if (snakeDir != DOWN && snakeDir != UP && gameState == START) snakeDir = DOWN;
			break;
		case BTNC:
			// set state
			switch (gameState) {
				case STANDBY:
					gameState = SETUP;
					//xil_printf("State: SETUP\n\r");
					break;
				case START:
					printPlayPauseStop(gameState = PAUSE);
					//xil_printf("State: PAUSE\n\r");
					break;
				case PAUSE:
					printPlayPauseStop(gameState = START);
					//xil_printf("State: START\n\r");
					break;
				case END:
					gameState = STANDBY;
					//xil_printf("State: STANDBY\n\r");
					plotWelcomeScreen();
					break;
			}
			break;
	}

	XGpio_DiscreteWrite(&led_gpio, LED_CHANNEL, btn_value); // debug session

	while (XTmrCtr_GetTimerCounterReg(XPAR_TMRCTR_0_BASEADDR, 1) < start_time + 20000000)
		continue;

	XGpio_InterruptEnable(&btn_gpio, XPAR_AXI_GPIO_BTN_DEVICE_ID);
	XGpio_InterruptClear(&btn_gpio, XPAR_AXI_GPIO_BTN_DEVICE_ID);

	return;
}
