/*
 * global.c
 *
 *  Created on: Dec 21, 2011
 *      Author: romain
 */

#include "global.h"
#include "hit_task.h"

wave_t current_wave = {0,1};

difficulty_t difficulty = NORMAL;

/* Prototypes */

//To call each time the current invaders wave is finished to init a new one
void level_up(){
	//static uint8_t lvl = 0;

	//current_wave->level = ++lvl;
	current_wave.level++;
	current_wave.invader_speed += 2*difficulty;

	//init_invaders(current_wave->invaders);
}
