/*
 * snake.c
 *
 *  Created on: Dec 2, 2021
 *      Author: Henry Chang, Ci-Chian Lu
 */

#include "snake.h"

Snake snake[FRAMEWIDTH * FRAMEHEIGHT]; // 768 = 32 * 24
Snake food;

u16 snakeSize;
u16 score, speed;

/*** plot LCD functions ***/
void plotWelcomeScreen(void) {
	xil_printf("Welcome to the snake game! Please press the center button to play.\n\r");
	gameState = STANDBY;

	// background canvas
	setColor(0, 54, 96);
	fillRect(0, 0, 240, 320);

	// project name
	setColor(254, 188, 17);
	setColorBg(0, 54, 96);
	setFont(BigFont);
	lcdPrint("NEXYSnake Game", 10, 10);

	// snake illustration
	setColor(0, 0, 0);
	fillRect(60, 40, 140, 60);
	fillRect(40, 60, 160, 100);
	fillRect(160, 80, 180, 120);
	fillRect(20, 100, 140, 140);
	fillRect(180, 120, 200, 140);
	fillRect(40, 140, 180, 160);
	fillRect(60, 160, 200, 180);
	fillRect(20, 180, 220, 200);
	fillRect(0, 200, 240, 240);
	fillRect(20, 240, 180, 260);
	fillRect(200, 240, 220, 260);

	setColor(146, 208, 80);
	fillRect(60, 60, 140, 100);
	fillRect(140, 80, 160, 100);
	fillRect(40, 100, 60, 140);
	fillRect(80, 100, 120, 140);
	fillRect(140, 100, 160, 140);
	fillRect(60, 180, 120, 200);
	fillRect(160, 180, 180, 200);
	fillRect(20, 200, 80, 220);
	fillRect(140, 200, 180, 220);

	setColor(83, 129, 53);
	fillRect(160, 120, 180, 140);
	fillRect(60, 140, 160, 160);
	fillRect(120, 180, 140, 200);
	fillRect(180, 180, 200, 200);
	fillRect(200, 200, 220, 240);
	fillRect(20, 220, 180, 240);

	setColor(237, 125, 49);
	fillRect(40, 120, 60, 140);
	fillRect(140, 120, 160, 140);

	// instructions
	setColor(254, 188, 17);
	setColorBg(0, 54, 96);
	setFont(SmallFont);
	lcdPrint("-Press BTNC to start-", 30, 270);

	// developer
	setColor(254, 188, 17);
	setColorBg(0, 54, 96);
	setFont(SmallFont);
	lcdPrint("UCSB ECE253 Final Project", 0, 295);
	lcdPrint("by Henry Chang & Ci-Chian Lu", 0, 305);

	return;
}

void drawSnake(void) {
	setColor(0, 54, 96);
	fillRect(snake[snakeSize].x + 1, snake[snakeSize].y + 1, snake[snakeSize].x + GRIDSIZE - 1, snake[snakeSize].y + GRIDSIZE - 1);

	// snake head
	setColor(0, 255, 0);
	fillRect(snake[0].x + 1, snake[0].y + 1, snake[0].x + GRIDSIZE - 1, snake[0].y + GRIDSIZE - 1);
	// snake eyes
	setColor(255, 255, 255);
	fillRect(snake[0].x + 5, snake[0].y + 5, snake[0].x + 14, snake[0].y + 14);
	setColor(0, 0, 0);
	fillRect(snake[0].x + 7, snake[0].y + 7, snake[0].x + 12, snake[0].y + 12);

	// snake body
	for (int i = 1; i < snakeSize; ++i) {
		(i % 2) ? setColor(146, 208, 80) : setColor(0, 255, 0);
		fillRect(snake[i].x + 1, snake[i].y + 1, snake[i].x + GRIDSIZE - 1, snake[i].y + GRIDSIZE - 1);
	}

	return;
}

void drawFood(void) {
	setColor(255, 0, 0);
	fillRect(food.x + 1, food.y + 8, food.x + GRIDSIZE - 1, food.y + GRIDSIZE - 1);

	setColor(128, 96, 0);
	fillRect(food.x + 10, food.y + 1, food.x + 11, food.y + 8);

	setColor(0, 255, 0);
	fillRect(food.x + 12, food.y + 3, food.x + 16, food.y + 4);

	setColor(0, 0, 0);
	fillRect(food.x + 8, food.y + 10, food.x + 12, food.y + 11);

	setColor(0, 54, 96);
	fillRect(food.x + 1, food.y + 8, food.x + 1, food.y + 8);
	fillRect(food.x + 1, food.y + GRIDSIZE - 1, food.x + 1, food.y + GRIDSIZE - 1);
	fillRect(food.x + GRIDSIZE - 1, food.y + 8, food.x + GRIDSIZE - 1, food.y + 8);
	fillRect(food.x + GRIDSIZE - 1, food.y + GRIDSIZE - 1, food.x + GRIDSIZE - 1, food.y + GRIDSIZE - 1);

	setColor(255, 255, 255);
	fillRect(food.x + 16, food.y + 10, food.x + 17, food.y + 14);

	return;
}

void plotBackground(void) {
	xil_printf("Plotting background.\n\r");
	// background
	setColor(0, 54, 96);
	fillRect(0, 0, 240, 320);

	// grid
	setColor(254, 188, 17);
	for (int i = GRIDSIZE; i < FRAMEWIDTH * GRIDSIZE; i += GRIDSIZE)
		fillRect(i, GRIDSIZE, i, FRAMEHEIGHT * GRIDSIZE);

	for (int j = GRIDSIZE; j < FRAMEHEIGHT * GRIDSIZE; j += GRIDSIZE)
		fillRect(0, j, FRAMEWIDTH * GRIDSIZE, j);

	return;
}

void printScore(u16 s) {
	char str[11];

	sprintf(str, "Score: %u", s);
	setColor(254, 188, 17);
	setColorBg(0, 54, 96);
	setFont(BigFont);
	lcdPrint(str, 0, 0);

	return;
}

void printPlayPauseStop(State s) {
	setColor(0, 54, 96);
	fillRect((FRAMEWIDTH - 1) * GRIDSIZE + 1, 1, FRAMEWIDTH * GRIDSIZE - 1, GRIDSIZE - 1);
	setColor(254, 188, 17);

	switch (s) {
		case START:
			for (int i = 0; i < 7; ++i)
				fillRect((FRAMEWIDTH - 1) * GRIDSIZE + 3 + i * 2, 3 + i, (FRAMEWIDTH - 1) * GRIDSIZE + 5 + i * 2, GRIDSIZE - 3 - i);
			break;
		case PAUSE:
			fillRect((FRAMEWIDTH - 1) * GRIDSIZE + 4, 4, (FRAMEWIDTH - 1) * GRIDSIZE + 8, GRIDSIZE - 4);
			fillRect((FRAMEWIDTH - 1) * GRIDSIZE + 12, 4, (FRAMEWIDTH - 1) * GRIDSIZE + 16, GRIDSIZE - 4);
			break;
		case END:
			fillRect((FRAMEWIDTH - 1) * GRIDSIZE + 4, 4, (FRAMEWIDTH - 1) * GRIDSIZE + 16, GRIDSIZE - 4);
			break;
		default:
			setColor(254, 188, 17);
			fillRect((FRAMEWIDTH - 1) * GRIDSIZE + 1, 1, FRAMEWIDTH * GRIDSIZE - 1, GRIDSIZE - 1);
	}

	return;
}

void printGameOver(void) {
	/* plot frame */
	setColor(0, 54, 96);
	fillRect(40, 120, 200, 120);
	fillRect(40, 120, 40, 220);
	fillRect(200, 120, 200, 220);
	fillRect(40, 220, 200, 220);

	/* plot text background */
	setColor(254, 188, 17);
	fillRect(41, 121, 199, 219);

	/* print text */
	setColor(0, 54, 96);
	setColorBg(254, 188, 17);

	setFont(BigFont);
	lcdPrint("GAME", 85, 140);
	lcdPrint("OVER!", 80, 160);

	setFont(SmallFont);
	lcdPrint("Press BTNC", 80, 190);
	lcdPrint("to continue", 75, 200);

	return;
}

/*** snake metrics ***/
void snakeSpeed(u16 s) {
	MB_Sleep(400 - s * 10); // millisecond

	return;
}

void snakeMotion(void) {
	// shift snake body coordinates
	for (int i = snakeSize; i > 0; --i)
		snake[i] = snake[i - 1];

	// update snake head coordinate
	switch (snakeDir) {
		case UP: 	snake[0].y -= GRIDSIZE; break;
		case LEFT:  snake[0].x -= GRIDSIZE; break;
		case RIGHT: snake[0].x += GRIDSIZE; break;
		case DOWN:  snake[0].y += GRIDSIZE; break;
	}

	return;
}

void spawnFood(void) {
	food.x = (rand() % FRAMEWIDTH) * GRIDSIZE;
	food.y = GRIDSIZE + (rand() % (FRAMEHEIGHT - 1)) * GRIDSIZE;

	// avoid overlapping with snake
	for (int i = 0; i < snakeSize; ++i) {
		if (food.x != snake[i].x || food.y != snake[i].y)
			continue;

		spawnFood();
		break;
	}

	return;
}

bool foodEaten(void) {
	if ((snake[0].x == food.x) && (snake[0].y == food.y))
		return true;

	return false;
}

bool collision(void) {
	// border collision
	switch (snakeDir) {
		case UP:
			if (snake[1].y == GRIDSIZE) return true;
			break;
		case LEFT:
			if (snake[1].x == 0) return true;
			break;
		case RIGHT:
			if (snake[0].x == 240) return true;
			break;
		case DOWN:
			if (snake[0].y == 320) return true;
			break;
	}

	// body collision
	for (int i = 1; i < snakeSize; ++i) {
		if ((snake[i].x == snake[0].x) && (snake[i].y == snake[0].y))
			return true;
	}

	return false;
}

/*** game functions ***/
void init_game(void) {
	xil_printf("---Entering game---\n\r");

	snakeSize = 5; // default snake size
	speed = 0; // initial speed;

	/*** generate initial snake position ***/
	srand(XTmrCtr_GetTimerCounterReg(XPAR_TMRCTR_0_BASEADDR, 1));
	snake[0].x = (rand() % (FRAMEWIDTH - snakeSize * 2) + snakeSize) * GRIDSIZE;
	snake[0].y = (rand() % (FRAMEHEIGHT - snakeSize * 2) + snakeSize) * GRIDSIZE;

	switch (rand() % 4) {
		case UP: // up
			for (int i = 1; i < snakeSize; ++i) {
				snake[i].x = snake[i - 1].x;
				snake[i].y = snake[i - 1].y - GRIDSIZE;
			}
			snakeDir = DOWN;
			break;
		case LEFT: // left
			for (int i = 1; i < snakeSize; ++i) {
				snake[i].x = snake[i - 1].x - GRIDSIZE;
				snake[i].y = snake[i - 1].y;
			}
			snakeDir = RIGHT;
			break;
		case RIGHT: // right
			for (int i = 1; i < snakeSize; ++i) {
				snake[i].x = snake[i - 1].x + GRIDSIZE;
				snake[i].y = snake[i - 1].y;
			}
			snakeDir = LEFT;
			break;
		case DOWN: // down
			for (int i = 1; i < snakeSize; ++i) {
				snake[i].x = snake[i - 1].x;
				snake[i].y = snake[i - 1].y + GRIDSIZE;
			}
			snakeDir = UP;
			break;
	}

	spawnFood(); // generate initial food

	plotBackground();
	printScore(score = 0); // initial score;
	drawSnake();
	drawFood();
	printPlayPauseStop(gameState = START); // set state

	xil_printf("Game start!\n\r");

	return;
}

void startGame(void) {
	snakeSpeed(speed);

	snakeMotion();
	drawSnake();

	if (collision()) {
		setColor(255, 0, 0);
		for (int i = 0; i < snakeSize; ++i) {
			fillRect(snake[i].x + 1, snake[i].y + 1, snake[i].x + GRIDSIZE - 1, snake[i].y + GRIDSIZE - 1);
			MB_Sleep(100);
		}

		// collision with north boundary
		if (snake[1].y == GRIDSIZE) {
			setColor(0, 54, 96);
			fillRect(snake[0].x + 1, snake[0].y + 1, snake[0].x + GRIDSIZE - 1, snake[0].y + GRIDSIZE - 1);
			MB_Sleep(100);
		}

		printPlayPauseStop(gameState = END);
		printScore(score);
		printGameOver();
	}

	if (foodEaten()) {
		++snakeSize;
		++speed;
		printScore(++score);

		spawnFood();
		drawFood();
	}

	return;
}

void clearSevenSeg(void) {
	sevenseg_draw_digit(7, 0b00000000);
	sevenseg_draw_digit(6, 0b00000000);
	sevenseg_draw_digit(5, 0b00000000);
	sevenseg_draw_digit(4, 0b00000000);
	sevenseg_draw_digit(3, 0b00000000);
	sevenseg_draw_digit(2, 0b00000000);
	sevenseg_draw_digit(1, 0b00000000);
	sevenseg_draw_digit(0, 0b00000000);

	return;
}

void pauseGame(void) {
	//sevenseg_draw_digit(7, 0b00000000);
	//sevenseg_draw_digit(6, 0b00000000);
	sevenseg_draw_digit(5, 0b01110011); //P
	sevenseg_draw_digit(4, 0b01110111); //A
	sevenseg_draw_digit(3, 0b00111110); //U
	sevenseg_draw_digit(2, 0b01101101); //S
	sevenseg_draw_digit(1, 0b01111001); //E
	sevenseg_draw_digit(0, 0b01011110); //D

	clearSevenSeg();

	return;
}

void gameOver(void) {
	sevenseg_draw_digit(7, 0b01101110); //Y
	sevenseg_draw_digit(6, 0b00111111); //O
	sevenseg_draw_digit(5, 0b00111110); //U
	//sevenseg_draw_digit(4, 0b00000000);
	sevenseg_draw_digit(3, 0b00111000); //L
	sevenseg_draw_digit(2, 0b00111111); //O
	sevenseg_draw_digit(1, 0b01101101); //S
	sevenseg_draw_digit(0, 0b01111001); //E

	clearSevenSeg();

	return;
}

void gameStandby(void) {
	sevenseg_draw_digit(7, 0b00111100); //V
	sevenseg_draw_digit(6, 0b00011110); //V
	sevenseg_draw_digit(5, 0b01111001); //E
	sevenseg_draw_digit(4, 0b00111000); //L
	sevenseg_draw_digit(3, 0b00111001); //C
	sevenseg_draw_digit(2, 0b00111111); //O
	sevenseg_draw_digit(1, 0b00110111); //M
	sevenseg_draw_digit(0, 0b01111001); //E

	clearSevenSeg();

	return;
}
