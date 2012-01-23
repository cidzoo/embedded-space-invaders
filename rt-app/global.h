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
#include <native/heap.h>

#include "bitmaps.h"

#define NB_INVADERS 7
#define NB_MAX_BULLETS 500
#define NB_MAX_BOMBS 10

#define COOLDOWN_RAIL 10
#define COOLDOWN_ROCKET 20
#define COOLDOWN_WAVE 120

#define TASK_MODE  T_FPU|T_CPU(0)  /* Uses FPU, bound to CPU #0 */
#define TASK_STKSZ 8192            /* Stack size (in bytes) */

#define TASK_FB_PRIO  40
#define TASK_IO_PRIO  60
#define TASK_HIT_PRIO 20
#define TASK_INVADERS_PRIO	50
#define TASK_SHIP_PRIO 70

#define GAME_ZONE_X_MIN		0
#define GAME_ZONE_X_MAX		240
#define GAME_ZONE_Y_MIN		10
#define GAME_ZONE_Y_MAX		320

#define NB_WEAPONS			5

/* Difficulty */
typedef enum{EASY=1, NORMAL=2, HARD=3}difficulty_t;
extern difficulty_t difficulty;

/* for use of bitmap */
typedef enum{G_SHIP,G_INVADER,G_BOMB,G_GUN,G_RAIL,G_ROCKET,G_WAVE} graphics_t;

/* Coord */
typedef struct{
	uint16_t x,y;
}coord_t;

/* Hitbox */
typedef struct{
	uint16_t x,y;
	uint8_t width, height;
	graphics_t type;
}hitbox_t;

/* Invaders */
typedef struct invader_t{
	uint8_t hp;
	hitbox_t hitbox;
}invader_t;

/* Waves */
typedef struct{
	uint8_t level;
	uint16_t invader_speed;
}wave_t;

extern wave_t current_wave;

/* Weapons */
typedef enum{BOMB, GUN, RAIL, ROCKET, WAVE} weapontype_t;
typedef enum{ONE=1, TWO=2, THREE=3, MAX=10}damage_t;
typedef enum{SLOW=1, MEDIUM=5, FAST=10, INSTANT}speed_t;

typedef struct{
	weapontype_t weapon_type;
	uint16_t cooldown; 		//time to ready
	uint8_t temp;			//temperature in % if applicable
	uint8_t damage;
	speed_t speed;
	uint16_t charge_max;
	uint16_t charge_current;
	uint16_t charge_time_total;
	uint16_t charge_time_current;
	uint16_t charge_last;
	uint16_t led_charge_max;
	uint16_t led_charge_ratio;
	uint16_t led_charge_current;
}weapon_t;

//Array of all weapons
extern weapon_t weapons[5];


/* Bullets */
//TODO : si le temps supprimer le pointeur weapon et mettre un weapon_type à la place
typedef struct{
	weapon_t *weapon;
	hitbox_t hitbox;
}bullet_t;

//List of the bombs
extern bullet_t bombs[NB_MAX_BOMBS];

/* Prototypes */
extern int add_bullet(bullet_t b);
extern void remove_bullet(int id);
extern void level_up(void);

#endif /* __GLOBAL_H__ */
