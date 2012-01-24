/*
 * enemies.h
 *
 *  Created on: Dec 22, 2011
 *      Author: michael
 */

#ifndef INVADERS_H_
#define INVADERS_H_

#include "global.h"

#define WIDTH_INVADER 	16
#define HEIGT_INVADER	16
#define HP_INVADER		10
#define SPACE_BETWEEN_INVADER	20
#define MARGE 10

/* Invaders */
typedef struct invader_t{
	uint8_t hp;
	hitbox_t hitbox;
}invader_t;

/* Waves */
typedef struct{
	uint8_t level;
	uint16_t invader_speed;
	invader_t invaders[NB_INVADERS_MAX];
	uint8_t invaders_count;
}wave_t;

extern wave_t wave;

extern int invaders_task_start(void);
extern void invaders_task_cleanup_task(void);
extern void invaders_task_cleanup_objects(void);

extern int invaders_lock(void);
extern int invaders_unlock(void);

extern void level_up(void);

#endif /*ENEMIES_H_*/
