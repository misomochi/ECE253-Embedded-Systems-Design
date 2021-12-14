/*****************************************************************************
* lab2a.c for Lab2A of ECE 153a at UCSB
* Date of the Last Update:  October 23,2014
*****************************************************************************/

#define AO_LAB2A

#include <stdbool.h>
#include "qpn_port.h"
#include "bsp.h"
#include "lab2a.h"
#include "lcd.h"

//Lab2A State machine
typedef struct Lab2ATag  {
	QActive super;
	/* extended state variables */
	unsigned short volume; // ranges from 0 to 100
	unsigned int time;
} Lab2A;

/* Setup state machines */
/**********************************************************************/
static QState Lab2A_initial (Lab2A *me); /* initial pseudostate-handler */
static QState Lab2A_on      (Lab2A *me); /* state-handler */
static QState Lab2A_stateA  (Lab2A *me); /* state-handler */
/**********************************************************************/

Lab2A AO_Lab2A; /* the sole instance of the Lab2A AO */
bool idle = false;

void Lab2A_ctor(void)  {
	Lab2A *me = &AO_Lab2A;
	QActive_ctor(&me->super, (QStateHandler)&Lab2A_initial);
	AO_Lab2A.volume = 0;
	AO_Lab2A.time = 0;
}

QState Lab2A_initial(Lab2A *me) {
	xil_printf("\n\rInitialization");
	initLCD(); // Initialize LCD controller
	clrScr(); // Clear display
    return Q_TRAN(&Lab2A_on);
}

QState Lab2A_on(Lab2A *me) {
	switch (Q_SIG(me)) {
		case Q_ENTRY_SIG: {
			xil_printf("On\r\n");
			}
			
		case Q_INIT_SIG: {
			return Q_TRAN(&Lab2A_stateA);
			}
	}
	
	return Q_SUPER(&QHsm_top);
}


/* Create Lab2A_on state and do any initialization code if needed */
/******************************************************************/
QState Lab2A_stateA(Lab2A *me) {
	switch (Q_SIG(me)) {
		case Q_ENTRY_SIG: {
			xil_printf("Startup State A\r\n");
			plotBackground();
			return Q_HANDLED();
		}
		case ENCODER_UP: {
			xil_printf("Encoder Up from State A\r\n");
			if (AO_Lab2A.time >= 2) {
				setColor(108, 54, 85);
				fillRect(10, 310 - 3 * AO_Lab2A.volume, 98, 310);
			}
			if (AO_Lab2A.volume < 100) {
				++AO_Lab2A.volume;
				setColor(108, 54, 85);
				fillRect(10, 310 - 3 * AO_Lab2A.volume, 98, 310 - 3 * (AO_Lab2A.volume - 1));
			}
			AO_Lab2A.time = 0;
			idle = false;
			return Q_HANDLED();
		}
		case ENCODER_DOWN: {
			xil_printf("Encoder Down from State A\r\n");
			if (AO_Lab2A.time >= 2) {
				setColor(108, 54, 85);
				fillRect(10, 310 - 3 * AO_Lab2A.volume, 98, 310);
			}
			if (AO_Lab2A.volume > 0) {
				--AO_Lab2A.volume;
				// cover volume bar with background
				setColor(25, 137, 100);
				fillRect(10, 310 - 3 * (AO_Lab2A.volume + 1), 98, 310 - 3 * AO_Lab2A.volume);
			}
			AO_Lab2A.time = 0;
			idle = false;
			return Q_HANDLED();
		}
		case ENCODER_CLICK: {
			xil_printf("Changing State\r\n");
			setColor(25, 137, 100);
			fillRect(10, 310 - 3 * AO_Lab2A.volume, 98, 310);
			AO_Lab2A.volume = 0;
			AO_Lab2A.time = 0;
			idle = false;
			return Q_HANDLED();
		}
		case BUTTON_UP: {
			xil_printf("Button Up from State A\r\n");
			setColor(255, 255, 255);
			setColorBg(0, 0, 0);
			lcdPrint("MODE: 1", 150, 30);
			AO_Lab2A.time = 0;
			idle = false;
			return Q_HANDLED();
		}
		case BUTTON_LEFT: {
			xil_printf("Button Left from State A\r\n");
			setColor(255, 255, 255);
			setColorBg(0, 0, 0);
			lcdPrint("MODE: 2", 150, 30);
			AO_Lab2A.time = 0;
			return Q_HANDLED();
		}
		case BUTTON_RIGHT: {
			xil_printf("Button Right from State A\r\n");
			setColor(255, 255, 255);
			setColorBg(0, 0, 0);
			lcdPrint("MODE: 3", 150, 30);
			AO_Lab2A.time = 0;
			idle = false;
			return Q_HANDLED();
		}
		case BUTTON_DOWN: {
			xil_printf("Button Down from State A\r\n");
			setColor(255, 255, 255);
			setColorBg(0, 0, 0);
			lcdPrint("MODE: 4", 150, 30);
			AO_Lab2A.time = 0;
			idle = false;
			return Q_HANDLED();
		}
		case BUTTON_CENTER: {
			xil_printf("Button Center from State A\r\n");
			setColor(255, 255, 255);
			setColorBg(0, 0, 0);
			lcdPrint("MODE: 5", 150, 30);
			AO_Lab2A.time = 0;
			idle = false;
			return Q_HANDLED();
		}
		case DEFAULT: {
			++AO_Lab2A.time;
			if (AO_Lab2A.time >= 2 && !idle) {
				setColor(25, 137, 100);
				fillRect(10, 10, 98, 310);
				fillRect(132, 0, 240, 188);
				idle = true;
			}
		}
	}

	return Q_SUPER(&Lab2A_on);
}

void plotBackground(void) {
	setColor(25, 137, 100);
	fillRect(0, 0, 240, 320);
	setColor(255, 255, 255);
	fillRect(108, 0, 132, 320);
	fillRect(108, 188, 240, 212);
	setColor(128, 128, 128);
	fillRect(118, 158, 122, 162);

	return;
}
