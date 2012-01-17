/*
 * enemies.h
 *
 *  Created on: Dec 22, 2011
 *      Author: michael
 */

#ifndef INVADERS_H_
#define INVADERS_H_

#include "global.h"

#define WIDTH_INVADER 	35
#define HEIGT_INVADER	35
#define HP_INVADER		10
#define SPACE_BETWEEN_INVADER	20

//List of the invaders
extern invader_t invaders[NB_INVADERS];

extern int invaders_task_start(void);
extern void invaders_task_cleanup(void);

extern int invaders_lock(void);
extern int invaders_unlock(void);

#endif /*ENEMIES_H_*/
