/*
 * snake.h
 *
 *  Created on: Dec 2, 2021
 *      Author: Henry Chang, Ci-Chian Lu
 */

#ifndef SRC_SNAKE_H_
#define SRC_SNAKE_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "xil_printf.h"
#include "xtmrctr_l.h"
#include "microblaze_sleep.h" // used for MB_Sleep()

#include "lcd.h"
#include "sevenSeg_new.h"

#define GRIDSIZE 20
#define FRAMEWIDTH 240/GRIDSIZE
#define FRAMEHEIGHT 320/GRIDSIZE

typedef struct snake {
	u16 x;
	u16 y;
} Snake;

typedef enum dir {UP, LEFT, RIGHT, DOWN} Dir;
typedef enum state {STANDBY, SETUP, START, PAUSE, END} State;

Dir snakeDir;
State gameState;

/*** plot LCD functions ***/
void plotWelcomeScreen(void);
void drawSnake(void);
void drawFood(void);
void plotBackground(void);
void printScore(u16);
void printPlayPauseStop(State);
void printGameOver(void);

/*** snake metrics ***/
void snakeSpeed(u16);
void snakeMotion(void);
void spawnFood(void);
bool foodEaten(void);
bool collision(void);

/*** game functions ***/
void init_game(void);
void startGame(void);
void clearSevenSeg(void);
void pauseGame(void);
void gameOver(void);
void gameStandby(void);

#endif /* SRC_SNAKE_H_ */
