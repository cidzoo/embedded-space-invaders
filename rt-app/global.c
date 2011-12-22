/*
 * global.c
 *
 *  Created on: Dec 21, 2011
 *      Author: romain
 */

#include "global.h"


//To call each time the current invaders wave is finished to init a new one
Wave* levelUp(){
	static uint8_t lvl = 0;

	currentWave->level = ++lvl;
	currentWave->invaderSpeed *= getDifficultyMultiplier();

	init_invaders(currentWave->invaders);
}

float getDifficultyMultiplier(){
	return 1+((float)difficulty/10);
}

/* Functions to manipulate the list of bullet */
int addBullet(Bullet *b){
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

int removeBullet(Bullet *b){
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

int main(void){

	levelUp();
}
