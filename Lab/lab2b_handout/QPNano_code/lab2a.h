/*****************************************************************************
* lab2a.h for Lab2A of ECE 153a at UCSB
* Date of the Last Update:  October 23,2014
*****************************************************************************/

#ifndef lab2a_h
#define lab2a_h

enum Lab2ASignals {
	ENCODER_UP = Q_USER_SIG,
	ENCODER_DOWN,
	ENCODER_CLICK,
	BUTTON_UP,
	BUTTON_LEFT,
	BUTTON_RIGHT,
	BUTTON_DOWN,
	BUTTON_CENTER,
	DEFAULT
};


extern struct Lab2ATag AO_Lab2A;


void Lab2A_ctor(void); /* the ctor */
void plotBackground(void);

#endif  
