/*
 * hit_task.h
 *
 *  Created on: 17 janv. 2012
 *      Author: redsuser
 */

#ifndef HIT_TASK_H_
#define HIT_TASK_H_

#include "global.h"

extern int hit_task_start(void);
extern void hit_task_cleanup_task(void);
extern void hit_task_cleanup_objects(void);

extern void hit_refresh(void);

#endif /* HIT_TASK_H_ */
