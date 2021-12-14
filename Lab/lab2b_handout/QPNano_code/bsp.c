/*****************************************************************************
* bsp.c for Lab2A of ECE 153a at UCSB
* Date of the Last Update:  October 27,2019
*****************************************************************************/

/**/
#include "qpn_port.h"
#include "bsp.h"
#include "lab2a.h"
#include "lcd.h"
#include "xil_exception.h"
#include "xparameters.h" //contains hardware addresses and bit masks
#include "xil_printf.h" // used for xil_printf()
#include "xintc.h" // interrupt drivers
#include "xspi.h" // SPI drivers
#include "xspi_l.h" // low-level SPI drivers
#include "xtmrctr.h" // timer drivers
#include "xtmrctr_l.h" // low-level timer drivers
#include "xgpio.h" // used for general purpose I/O

/*****************************/

/* Define all variables and Gpio objects here  */

#define LED_CHANNEL 1
#define BTNU 0b00001
#define BTNL 0b00010
#define BTNR 0b00100
#define BTND 0b01000
#define BTNC 0b10000

// Create ONE interrupt controllers XIntc
XIntc sys_intc;
// Create ONE timer controllers XTmrCtr
XTmrCtr sys_tmrctr;
// Create static XGpio variables
static XGpio led_gpio;
static XGpio btn_gpio;
static XGpio encoder_gpio;
// Create static XSpi variables
static XSpi lcd_spi;
XSpi_Config *configPtr; /* Pointer to Configuration data */
// Suggest Creating two int's to use for determining the direction of twist

enum State {CW00, CW01, CW10, CW11, CCW00, CCW01, CCW10, CCW11};
enum State state = CW00; // reset state

/*..........................................................................*/
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
	XIntc_Enable(&sys_intc, XPAR_MICROBLAZE_0_AXI_INTC_AXI_GPIO_BTN_IP2INTC_IRPT_INTR);
	XIntc_Enable(&sys_intc, XPAR_MICROBLAZE_0_AXI_INTC_AXI_GPIO_ENCODER_IP2INTC_IRPT_INTR);

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
	XTmrCtr_SetResetValue(&sys_tmrctr, 0, 0xFFFFFFFF - 200000000); // the timer will expire approximately every 1 second
	/*
	 * Start the timer counter such that it's incrementing by default,
	 * then wait for it to timeout a number of times
	 */
	XTmrCtr_Start(&sys_tmrctr, 0);
	xil_printf("Timer start!\n");

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

void init_encodergpio(void) {
	/* Initialize the GPIO driver */
	XGpio_Initialize(&encoder_gpio, XPAR_AXI_GPIO_ENCODER_DEVICE_ID);
	/* This function ensures interrupts are configured for this device then enables the interrupt. */
	XGpio_InterruptEnable(&encoder_gpio, XPAR_MDM_1_INTERRUPT_MASK);
	/* This function allows for interrupt output signals to be passed through to the microblaze */
	XGpio_InterruptGlobalEnable(&encoder_gpio);
	/* This function should be called after the software has serviced the interrupts that are pending. */
	XGpio_InterruptClear(&encoder_gpio, XPAR_MDM_1_INTERRUPT_MASK);

	return;
}

void init_lcdspi(void) {
	/*
	 * Initialize the SPI driver so that it is  ready to use.
	 */
	configPtr = XSpi_LookupConfig(XPAR_SPI_DEVICE_ID);
	if (configPtr == NULL) {
		xil_printf("Can't find spi device!\n");
		return XST_DEVICE_NOT_FOUND;
	}

	int status = XSpi_CfgInitialize(&lcd_spi, configPtr, configPtr -> BaseAddress);
	if (status != XST_SUCCESS) {
		xil_printf("Initialize spi fail!\n");
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

void BSP_init(void) {
/* Setup LED's, etc */
/* Setup interrupts and reference to interrupt handler function(s)  */
	xil_printf("Starting BSP_init\r\n");
	/*
	 * Initialize the interrupt controller driver so that it's ready to use.
	 * specify the device ID that was generated in xparameters.h
	 *
	 * Initialize GPIO and connect the interrupt controller to the GPIO.
	 *
	 */
	xil_printf("Starting interrupt initialization\r\n");
	init_intc();
	init_tmrctr();
	init_btngpio();
	init_encodergpio();
	xil_printf("Finishing interrupt initialization\r\n");

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

	xil_printf("Finishing BSP_init\r\n");
		
}
/*..........................................................................*/
void QF_onStartup(void) {                 /* entered with interrupts locked */

/* Enable interrupts */
	xil_printf("\n\rQF_onStartup\n"); // Comment out once you are in your complete program

	// Press Knob
	// Enable interrupt controller
	// Start interupt controller
	// register handler with Microblaze
	// Global enable of interrupt
	// Enable interrupt on the GPIO

	// Twist Knob

	// General
	// Initialize Exceptions
	// Press Knob
	// Register Exception
	// Twist Knob
	// Register Exception
	// General
	// Enable Exception

	// Variables for reading Microblaze registers to debug your interrupts.
//	{
//		u32 axi_ISR =  Xil_In32(intcPress.BaseAddress + XIN_ISR_OFFSET);
//		u32 axi_IPR =  Xil_In32(intcPress.BaseAddress + XIN_IPR_OFFSET);
//		u32 axi_IER =  Xil_In32(intcPress.BaseAddress + XIN_IER_OFFSET);
//		u32 axi_IAR =  Xil_In32(intcPress.BaseAddress + XIN_IAR_OFFSET);
//		u32 axi_SIE =  Xil_In32(intcPress.BaseAddress + XIN_SIE_OFFSET);
//		u32 axi_CIE =  Xil_In32(intcPress.BaseAddress + XIN_CIE_OFFSET);
//		u32 axi_IVR =  Xil_In32(intcPress.BaseAddress + XIN_IVR_OFFSET);
//		u32 axi_MER =  Xil_In32(intcPress.BaseAddress + XIN_MER_OFFSET);
//		u32 axi_IMR =  Xil_In32(intcPress.BaseAddress + XIN_IMR_OFFSET);
//		u32 axi_ILR =  Xil_In32(intcPress.BaseAddress + XIN_ILR_OFFSET) ;
//		u32 axi_IVAR = Xil_In32(intcPress.BaseAddress + XIN_IVAR_OFFSET);
//		u32 gpioTestIER  = Xil_In32(sw_Gpio.BaseAddress + XGPIO_IER_OFFSET);
//		u32 gpioTestISR  = Xil_In32(sw_Gpio.BaseAddress  + XGPIO_ISR_OFFSET ) & 0x00000003; // & 0xMASK
//		u32 gpioTestGIER = Xil_In32(sw_Gpio.BaseAddress  + XGPIO_GIE_OFFSET ) & 0x80000000; // & 0xMASK
//	}
}


void QF_onIdle(void) {        /* entered with interrupts locked */

    QF_INT_UNLOCK();                       /* unlock interrupts */

    {
    	// Write code to increment your interrupt counter here.
    	// QActive_postISR((QActive *)&AO_Lab2A, ENCODER_DOWN); is used to post an event to your FSM



// 			Useful for Debugging, and understanding your Microblaze registers.
//    		u32 axi_ISR =  Xil_In32(intcPress.BaseAddress + XIN_ISR_OFFSET);
//    	    u32 axi_IPR =  Xil_In32(intcPress.BaseAddress + XIN_IPR_OFFSET);
//    	    u32 axi_IER =  Xil_In32(intcPress.BaseAddress + XIN_IER_OFFSET);
//
//    	    u32 axi_IAR =  Xil_In32(intcPress.BaseAddress + XIN_IAR_OFFSET);
//    	    u32 axi_SIE =  Xil_In32(intcPress.BaseAddress + XIN_SIE_OFFSET);
//    	    u32 axi_CIE =  Xil_In32(intcPress.BaseAddress + XIN_CIE_OFFSET);
//    	    u32 axi_IVR =  Xil_In32(intcPress.BaseAddress + XIN_IVR_OFFSET);
//    	    u32 axi_MER =  Xil_In32(intcPress.BaseAddress + XIN_MER_OFFSET);
//    	    u32 axi_IMR =  Xil_In32(intcPress.BaseAddress + XIN_IMR_OFFSET);
//    	    u32 axi_ILR =  Xil_In32(intcPress.BaseAddress + XIN_ILR_OFFSET) ;
//    	    u32 axi_IVAR = Xil_In32(intcPress.BaseAddress + XIN_IVAR_OFFSET);
//
//    	    // Expect to see 0x00000001
//    	    u32 gpioTestIER  = Xil_In32(sw_Gpio.BaseAddress + XGPIO_IER_OFFSET);
//    	    // Expect to see 0x00000001
//    	    u32 gpioTestISR  = Xil_In32(sw_Gpio.BaseAddress  + XGPIO_ISR_OFFSET ) & 0x00000003;
//
//    	    // Expect to see 0x80000000 in GIER
//    		u32 gpioTestGIER = Xil_In32(sw_Gpio.BaseAddress  + XGPIO_GIE_OFFSET ) & 0x80000000;


    }
}

/* Do not touch Q_onAssert */
/*..........................................................................*/
void Q_onAssert(char const Q_ROM * const Q_ROM_VAR file, int line) {
    (void)file;                                   /* avoid compiler warning */
    (void)line;                                   /* avoid compiler warning */
    QF_INT_LOCK();
    for (;;) {
    }
}

/* Interrupt handler functions here.  Do not forget to include them in lab2a.h!
To post an event from an ISR, use this template:
QActive_postISR((QActive *)&AO_Lab2A, SIGNALHERE);
Where the Signals are defined in lab2a.h  */

/******************************************************************************
*
* This is the interrupt handler routine for the GPIO for this example.
*
******************************************************************************/
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
	QActive_postISR((QActive *)&AO_Lab2A, DEFAULT);
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

	XGpio_DiscreteWrite(&led_gpio, LED_CHANNEL, btn_value); // debug session

	switch (btn_value) {
		case BTNU:
			QActive_postISR((QActive *)&AO_Lab2A, BUTTON_UP);
			break;
		case BTNL:
			QActive_postISR((QActive *)&AO_Lab2A, BUTTON_LEFT);
			break;
		case BTNR:
			QActive_postISR((QActive *)&AO_Lab2A, BUTTON_RIGHT);
			break;
		case BTND:
			QActive_postISR((QActive *)&AO_Lab2A, BUTTON_DOWN);
			break;
		case BTNC:
			QActive_postISR((QActive *)&AO_Lab2A, BUTTON_CENTER);
			break;
	}

	XGpio_InterruptClear(&btn_gpio, XPAR_MDM_1_INTERRUPT_MASK);

	return;
}

void encoderint_handler(void) {
	unsigned short encoder = XGpio_DiscreteRead(&encoder_gpio, 1); /* encoder = JD[2:0] = {BTN, B, A} */

	/* Check if BTN is pressed */
	if (encoder & 0b100) {
		QActive_postISR((QActive *)&AO_Lab2A, ENCODER_CLICK);
	}

	switch ((state << 2) + (encoder & 0b11)) {
		case 0b00001: case 0b10001: state = CW01; break;
		case 0b00100: state = CW00; break;
		case 0b00111: state = CW11; break;
		case 0b01101: state = CW01; break;
		case 0b01110: state = CW10; break;
		case 0b01011: state = CW11; break;
		case 0b01000: state = CW00; QActive_postISR((QActive *)&AO_Lab2A, ENCODER_DOWN); break;
		case 0b00010: case 0b10010: state = CCW10; break;
		case 0b11000: state = CCW00; break;
		case 0b11011: state = CCW11; break;
		case 0b11110: state = CCW10; break;
		case 0b11101: state = CCW01; break;
		case 0b10111: state = CCW11; break;
		case 0b10100: state = CCW00; QActive_postISR((QActive *)&AO_Lab2A, ENCODER_UP); break;
	}

	XGpio_InterruptClear(&encoder_gpio, XPAR_MDM_1_INTERRUPT_MASK);

	return;
}
