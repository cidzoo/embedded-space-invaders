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

//List of the invaders
extern invader_t invaders[NB_INVADERS];


extern int invaders_task_start(void);
extern void invaders_task_cleanup_task(void);
extern void invaders_task_cleanup_objects(void);

extern int invaders_lock(void);
extern int invaders_unlock(void);

extern void invaders_refresh(void);

#endif /*ENEMIES_H_*/
