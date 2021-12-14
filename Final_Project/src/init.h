/*
 * init.h
 *
 *  Created on: Dec 2, 2021
 *      Author: Henry Chang, Ci-Chian Lu
 */

#ifndef SRC_INIT_H_
#define SRC_INIT_H_

#include "xil_printf.h" // used for xil_printf()
#include "xparameters.h" //contains hardware addresses and bit masks
#include "xintc.h" // interrupt drivers
#include "xspi.h" // SPI drivers
#include "xspi_l.h" // low-level SPI drivers
#include "xtmrctr.h" // timer drivers
#include "xtmrctr_l.h" // low-level timer drivers
#include "xgpio.h" // used for general purpose I/O

#include "lcd.h"
#include "snake.h"

#define LED_CHANNEL 1
#define BTNU 0b00001
#define BTNL 0b00010
#define BTNR 0b00100
#define BTND 0b01000
#define BTNC 0b10000

void init_intc(void);
void init_tmrctr(void);
void init_btngpio(void);
void init_lcdspi(void);
void init_method(void);
void rst_tmr(void);
void tmrint_handler(void);
void btnint_handler(void);

#endif /* SRC_INIT_H_ */
