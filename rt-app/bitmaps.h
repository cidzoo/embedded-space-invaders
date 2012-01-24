/*
 * bitmaps.h
 *
 *  Created on: 21 janv. 2012
 *      Author: redsuser
 */

#ifndef BITMAPS_H_
#define BITMAPS_H_

#include <linux/types.h>
#include "lcdlib.h"

/* Bitmap sizes */

#define BOMB_WIDTH 8
#define BOMB_HEIGHT 8

#define GUN_WIDTH 4
#define GUN_HEIGHT 5

#define RAIL_WIDTH 6
#define RAIL_HEIGHT LCD_MAX_Y

#define ROCKET_WIDTH 10
#define ROCKET_HEIGHT 14

#define WAVE_WIDTH LCD_MAX_X
#define WAVE_HEIGHT 4

#define INVADER_WIDTH 16
#define INVADER_HEIGHT 16

#define SHIP_WIDTH 32
#define SHIP_HEIGHT 32

extern uint16_t bmp_bomb[BOMB_HEIGHT][BOMB_WIDTH];
extern uint16_t bmp_gun[GUN_HEIGHT][GUN_WIDTH];
extern uint16_t bmp_rail[RAIL_HEIGHT][RAIL_WIDTH];
extern uint16_t bmp_rocket[ROCKET_HEIGHT][ROCKET_WIDTH];
extern uint16_t bmp_wave[WAVE_HEIGHT][WAVE_WIDTH];
extern uint16_t bmp_invader[INVADER_HEIGHT][INVADER_WIDTH];
extern uint16_t bmp_ship[SHIP_HEIGHT][SHIP_WIDTH];

#endif /* BITMAPS_H_ */
