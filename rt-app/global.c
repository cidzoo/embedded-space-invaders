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


/* Functions to manipulate the list of bullet */
int add_bullet(bullet_t b){
	int i=0;
	//find the first empty slot and place the bullet there
	//TODO : gérer le cas ou le tableau est plein
	for(i=0;i<NB_MAX_BULLETS;i++){
		if(bullets[i].weapon == NULL){
			bullets[i] = b;

			return 0;
		}
	}
	return -1;
}

void remove_bullet(int id){
	bullets[id].weapon = NULL;
}
