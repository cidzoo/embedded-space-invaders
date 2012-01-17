/*
 * chip_task.c
 *
 *  Created on: 17 janv. 2012
 *      Author: redsuser
 */
#include <linux/types.h>
#include "chip_task.h"

/**
 * Variables privées
 */
static RT_TASK chip_task_handle;
static uint8_t chip_task_created = 0;

/**
 * Fonctions privées
 */
static void chip_task(void *cookie);
