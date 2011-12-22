/*
 * global.c
 *
 *  Created on: 20 déc. 2011
 *      Author: Romain
 */
#include <stdint.h>

#define NB_INVADERS 7
#define NB_MAX_BULLETS 500

#define COOLDOWN_RAIL 10
#define COOLDOWN_ROCKET 20
#define COOLDOWN_WAVE 120

typedef enum{EASY=1, NORMAL=2, HARD=3}Difficulty;
Difficulty difficulty = NORMAL;

/* Waves */
typedef struct{
	uint8_t level;
	Invader invaders[NB_INVADERS];
	uint8_t invaderSpeed;
}Wave;
Wave *currentWave;

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
typedef enum{BOMB, GUN, RAIL, ROCKET, WAVE} WeaponType;
typedef enum{ONE=1, TWO=2, THREE=3, MAX=10};
typedef enum{SLOW=50, NORMAL=100, FAST=200, INSTANT}Speed;

typedef struct{
	WeaponType weaponType;
	uint16_t cooldown; 		//time to ready
	uint8_t temp;			//temperature in % if applicable
	uint8_t damage;
	Speed speed;
}Weapon;

//Array of all weapons
Weapon weapons[4];
weapons[0] = {BOMB, 0, 0, ONE, NORMAL};
weapons[1] = {GUN, 0, 0, ONE, NORMAL};
weapons[2] = {RAIL, 10, 0, TWO, INSTANT};
weapons[3] = {ROCKET, 20, 0, THREE, SLOW};
weapons[4] = {WAVE, 120, 0, MAX, FAST};


/* Bullets */
typedef struct{
	Weapon *weapon;
	uint16_t x,y;
	uint8_t width, height;
}Bullet;

//List of the bullets
Bullet *bullets[NB_MAX_BULLETS] = {NULL};

//List of the bombs
BUllet *bombs[NB_MAX_BOMBS] = {NULL};

