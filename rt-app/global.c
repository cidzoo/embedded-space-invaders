/*
 * global.c
 *
 *  Created on: Dec 21, 2011
 *      Author: romain
 */

#include "global.h"

wave_t *current_wave;
spaceship_t *ss;

//List of the bullets
bullet_t *bullets[NB_MAX_BULLETS] = {NULL};

//List of the bombs
bullet_t *bombs[NB_MAX_BOMBS] = {NULL};

//Array of all weapons
weapon_t weapons[5] = {
	{BOMB, 0, 0, ONE, MEDIUM},
	{GUN, 0, 0, ONE, MEDIUM},
	{RAIL, 10, 0, TWO, INSTANT},
	{ROCKET, 20, 0, THREE, SLOW},
	{WAVE, 120, 0, MAX, FAST}
};

difficulty_t difficulty = NORMAL;

/* Prototypes */

//To call each time the current invaders wave is finished to init a new one
void level_up(){
	static uint8_t lvl = 0;

	current_wave->level = ++lvl;
	current_wave->invader_speed += 2*difficulty;

	//init_invaders(current_wave->invaders);
}

/* Functions to manipulate the list of bullet */
int add_bullet(bullet_t *b){
	int i=0;
	//find the first empty slot and place the bullet there
	for(i=0;i<NB_MAX_BULLETS;i++){
		if(bullets[i] == NULL){
			bullets[i] = b;
			return 0;
		}
	}
	return -1;
}

int remove_bullet(bullet_t *b){
	int i=0;
	//find the bullet to delete
	for(i=0;i<NB_MAX_BULLETS;i++){
		if(bullets[i] == b){
			bullets[i] = NULL;
			return 0;
		}
	}
	return -1;
}
