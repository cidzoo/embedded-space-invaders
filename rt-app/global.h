#ifndef __GLOBAL_H__
#define __GLOBAL_H__
/*
 * global.c
 *
 *  Created on: 20 déc. 2011
 *      Author: Romain
 */

#include <linux/types.h>
#include <native/task.h>
#include <native/intr.h>
#include <native/event.h>
#include <native/alarm.h>
#include <native/timer.h>
#include <native/mutex.h>

#define NB_INVADERS 7
#define NB_MAX_BULLETS 500
#define NB_MAX_BOMBS 10

#define COOLDOWN_RAIL 10
#define COOLDOWN_ROCKET 20
#define COOLDOWN_WAVE 120

#define TASK_MODE  T_FPU|T_CPU(0)  /* Uses FPU, bound to CPU #0 */
#define TASK_STKSZ 8192            /* Stack size (in bytes) */

#define TASK_FB_PRIO  40
#define TASK_IO_PRIO  50
#define TASK_HIT_PRIO 40
#define TASK_INVADERS_PRIO	60

/* Difficulty */
typedef enum{EASY=1, NORMAL=2, HARD=3}difficulty_t;
extern difficulty_t difficulty;

/* Hitbox */
typedef struct{
	uint16_t x,y;
}coord_t;

/* Hitbox */
typedef struct{
	uint16_t x,y;
	uint8_t width, height;
}hitbox_t;

/* Invaders */
typedef struct{
	uint8_t hp;
	hitbox_t hitbox;
}invader_t;

/* Waves */
typedef struct{
	uint8_t level;
	invader_t invaders[NB_INVADERS];
	uint16_t invader_speed;
}wave_t;

extern wave_t *current_wave;

/* Spaceship */
typedef struct{
	uint8_t hp;
	hitbox_t hitbox;
	int16_t current_speed_x; // -x : 0 : x
}spaceship_t;

extern spaceship_t *ss;

/* Weapons */
typedef enum{BOMB, GUN, RAIL, ROCKET, WAVE} weapontype_t;
typedef enum{ONE=1, TWO=2, THREE=3, MAX=10}damage_t;
typedef enum{SLOW=50, MEDIUM=100, FAST=200, INSTANT}speed_t;

typedef struct{
	weapontype_t weapon_type;
	uint16_t cooldown; 		//time to ready
	uint8_t temp;			//temperature in % if applicable
	uint8_t damage;
	speed_t speed;
}weapon_t;

//Array of all weapons
extern weapon_t weapons[5];


/* Bullets */
typedef struct{
	weapon_t *weapon;
	hitbox_t hitbox;
}bullet_t;



//List of the bullets
extern bullet_t *bullets[NB_MAX_BULLETS];

//List of the bombs
extern bullet_t *bombs[NB_MAX_BOMBS];

/* Prototypes */
extern int add_bullet(bullet_t *b);
extern int remove_bullet(bullet_t *b);
extern void level_up(void);

#endif /* __GLOBAL_H__ */
