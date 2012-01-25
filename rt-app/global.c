/*
 * global.c
 *
 *  Created on: Dec 21, 2011
 *      Author: romain
 */

#include "global.h"
#include "hit_task.h"
#include "invaders_task.h"


difficulty_t difficulty = NORMAL;

/* Game over*/
uint8_t game_over = 0;

uint32_t game_points = 0;
uint32_t game_bullet_kill = 1;
uint32_t game_bullet_used = 1;

uint8_t game_break = 1;
uint8_t game_started = 0;

uint8_t screen_pressed = 0;
uint16_t screen_x = 0;
uint16_t screen_y = 0;

