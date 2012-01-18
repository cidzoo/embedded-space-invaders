/*
 * ship_task.h
 *
 *  Created on: 17 janv. 2012
 *      Author: redsuser
 */

#ifndef SHIP_TASK_H_
#define SHIP_TASK_H_

#include "global.h"

extern int ship_task_start(void);
extern void ship_task_cleanup(void);

extern int ship_lock(void);
extern int ship_unlock(void);

#endif /* SHIP_TASK_H_ */
