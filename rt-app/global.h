/*
 * global.c
 *
 *  Created on: 20 déc. 2011
 *      Author: Romain
 */
#include <stdint.h>

/* Waves */
typedef struct{
	uint8_t level;
	Invader invaders[7];
	uint8_t invaderSpeed;
}Wave;

/* Invaders */
typedef struct{
	uint8_t hp;
	uint16_t x,y;
	uint8_t width, height;
}Invader;

/* Spaceship */
typedef struct{
	uint8_t hp;
	uint16_t x,y;
	uint8_t width, height;
	int16_t currentSpeedX; // -x : 0 : x
}SpaceShip;

/* Weapons */
typedef enum{GUN, RAIL, ROCKET, WAVE} WeaponType;
typedef enum{ONE=1, TWO=2, THREE=3, MAX=10};
typedef enum{SLOW=50, NORMAL=100, FAST=200, INSTANT}Speed;

typedef struct{
	WeaponType weaponType;
	uint16_t cooldown; 		//time to ready
	uint8_t temp;			//temperature in % if applicable
	uint8_t damage;
	Speed speed;
}Weapon;

//Array of weapons
Weapon weapons[4];
weapons[0] = {GUN, 0, 0, ONE, NORMAL};
weapons[1] = {RAIL, 10, 0, TWO, INSTANT};
weapons[2] = {ROCKET, 20, 0, THREE, SLOW};
weapons[3] = {WAVE, 120, 0, MAX, FAST};


/* Bullets */
typedef struct{
	Weapon *weapon;
	uint16_t x,y;
	uint8_t width, height;
}Bullet;

//List of the bullets
Bullet *bullets[NB_MAX_BULLETS] = {NULL};

//typedef struct{
//	Bullet *bullet;
//	ListBullets *next;
//}ListBullets;
//
//ListBullets *headListBullets = NULL;
