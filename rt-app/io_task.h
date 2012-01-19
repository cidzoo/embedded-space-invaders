/*
 * io_task.h
 *
 *  Created on: 17 janv. 2012
 *      Author: redsuser
 */

#ifndef IO_TASK_H_
#define IO_TASK_H_

#include "global.h"

extern int io_task_start(void);
extern void io_task_cleanup_task(void);
extern void io_task_cleanup_objects(void);

#endif /* IO_TASK_H_ */
