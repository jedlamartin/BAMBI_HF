/*
 * entities.h
 *
 *  Created on: 2023. nov. 8.
 *      Author: Martin
 */

#ifndef SRC_ENTITIES_H_
#define SRC_ENTITIES_H_



#endif /* SRC_ENTITIES_H_ */

#include "segmentlcd_individual.h"
#include "segmentlcd.h"
#include "em_timer.h"
#include "em_adc.h"
#include "em_gpio.h"
#include <stdbool.h>

#define OBSTACLE_NUMBER 3
#define TIMER_DEFAULT 27343
volatile bool gameOver;
volatile bool idle;
long long int baudrate;




typedef enum Direction{
	Up, Down, Right
} Direction;

typedef struct Ship{
	Direction currDir;
	Direction prevDir;
	uint8_t dig;
	SegmentLCD_LowerCharSegments_TypeDef pos;
	int vel;
} Ship;

typedef struct Obstacle{
	uint8_t dig;
	SegmentLCD_LowerCharSegments_TypeDef pos;
	int posType;
} Obstacle;

int getBaudrate();

void initUSART1();
void initTIMER0();
void initGPIO();
void initTIMER1();
void initADC();
void enableIntForAll();

void randomObstacles();

void initShip(Ship* myShip);

void initObstacles(Obstacle* obstacles);

bool stepShip(Ship* myShip);

void display(Ship* myShip, Obstacle* obstacles, SegmentLCD_LowerCharSegments_TypeDef* lowerCharSegments);

bool checkCrash(Ship* myShip, Obstacle* obstacles);

void gameOverState();
