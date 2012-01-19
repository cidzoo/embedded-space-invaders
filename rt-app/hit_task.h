/*
 * hit_task.h
 *
 *  Created on: 17 janv. 2012
 *      Author: redsuser
 */

#ifndef HIT_TASK_H_
#define HIT_TASK_H_

#include "global.h"


//List of the bullets
extern bullet_t bullets[NB_MAX_BULLETS];

//List of the bombs
extern bullet_t bombs[NB_MAX_BOMBS];

extern int hit_task_start(void);
extern void hit_task_cleanup_task(void);
extern void hit_task_cleanup_objects(void);

extern int hit_lock(void);
extern int hit_unlock(void);

extern void fire_weapon(weapontype_t w);

#endif /* HIT_TASK_H_ */
