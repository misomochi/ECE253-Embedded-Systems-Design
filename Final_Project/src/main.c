/*
 * main.c
 *
 *  Created on: Dec 2, 2021
 *      Author: Henry Chang, Ci-Chian Lu
 */

#include "xil_cache.h" // cache drivers
#include "xil_printf.h" // used for xil_printf()

#include "init.h"
#include "snake.h"
#include "lcd.h"

int main(void) {
	Xil_ICacheInvalidate();
	Xil_ICacheEnable();
	Xil_DCacheInvalidate();
	Xil_DCacheEnable();

	xil_printf("---Entering main---\n\r");

	init_method(); // initialize board and peripherals
	plotWelcomeScreen();

	/* Grand Loop */
	while (true) {
		switch (gameState) {
			case STANDBY: gameStandby(); break;
			case SETUP:   init_game();   break;
			case START:   startGame();   break;
			case PAUSE:   pauseGame();   break;
			case END: 	  gameOver(); 	 break;
		}
	}

	return 0;
}
