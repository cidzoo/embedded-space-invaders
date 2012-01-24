/*
 * ship_task.h
 *
 *  Created on: 17 janv. 2012
 *      Author: redsuser
 */

#ifndef SHIP_TASK_H_
#define SHIP_TASK_H_

#include "global.h"

/* Spaceship */
typedef struct{
	uint8_t hp;
	hitbox_t hitbox;
	int16_t current_speed_x; // -x : 0 : x
}spaceship_t;

//List of the invaders
extern spaceship_t ship;

extern int ship_task_start(void);
extern void ship_task_cleanup_task(void);
extern void ship_task_cleanup_objects(void);

extern int ship_lock(void);
extern int ship_unlock(void);

#endif /* SHIP_TASK_H_ */
