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

#define NB_INVADERS_MAX 24
#define NB_MAX_BULLETS 50
#define NB_MAX_BOMBS 20


#define COOLDOWN_RAIL 10
#define COOLDOWN_ROCKET 20
#define COOLDOWN_WAVE 120

#define TASK_MODE  T_FPU|T_CPU(0)  /* Uses FPU, bound to CPU #0 */
#define TASK_STKSZ 8192            /* Stack size (in bytes) */

#define TASK_FB_PRIO  70 //40
#define TASK_IO_PRIO  60
#define TASK_HIT_PRIO 20
#define TASK_INVADERS_PRIO	50
#define TASK_SHIP_PRIO 70

#define GAME_ZONE_X_MIN		0
#define GAME_ZONE_X_MAX		240
#define GAME_ZONE_Y_MIN		35
#define GAME_ZONE_Y_MAX		320

#define LIFE_SHIP			3

#define NB_WEAPONS			5


/* General */
extern uint8_t game_over;
extern uint32_t game_points;
extern uint32_t game_bullet_kill;
extern uint32_t game_bullet_used;
extern uint8_t game_break;
extern uint8_t game_started;
extern uint8_t game_level_up;

extern uint8_t screen_pressed;
extern uint16_t screen_x;
extern uint16_t screen_y;

/* Difficulty */
typedef enum{EASY=1, NORMAL=2, HARD=3}difficulty_t;
extern difficulty_t difficulty;

/* Coord */
typedef struct{
	uint16_t x,y;
}coord_t;

/* Hitbox */
typedef struct{
	uint16_t x,y;
	uint16_t width, height;
	uint16_t *bitmap;
}hitbox_t;

/* Weapons */
typedef enum{BOMB, GUN, RAIL, ROCKET, WAVE} weapontype_t;
typedef enum{ONE=1, TWO=2, THREE=3, MAX=10}damage_t;
typedef enum{STATIC=0, SLOW=1, MEDIUM=3, FAST=7, SUPERFAST = 10}speed_t;

typedef struct{
	uint16_t max;
	uint16_t now;
	uint16_t last;
	uint16_t time_total;
	uint16_t time_current;
} weapon_timing_charge_t;

typedef struct{
	uint16_t max;
	uint16_t ratio;
	uint16_t now;
}weapon_timing_led_t;

typedef struct{
	weapontype_t weapon_type;
	uint8_t damage;
	speed_t speed;
	weapon_timing_charge_t timing_charge;
	weapon_timing_led_t timing_led;
}weapon_t;

typedef struct{
	weapon_t *weapon;
	hitbox_t hitbox;
}bullet_t;

#endif /* __GLOBAL_H__ */
