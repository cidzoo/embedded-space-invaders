/*
 * ship_task.c
 *
 *  Created on: 17 janv. 2012
 *      Author: redsuser
 */
#include <linux/types.h>
#include "ship_task.h"
#include "vga_lookup.h"
#include "xeno-ts.h"
#include "lcdlib.h"
#include "rt-app-m.h"


/**
 * Variables public
 */
spaceship_t ship;

/**
 * Variables privées
 */
RT_MUTEX ship_task_mutex;
static uint8_t ship_task_mutex_created = 0;

RT_TASK ship_task_handle;
static uint8_t ship_task_created = 0;

struct ts_sample sample;	// Utilise pour recuperer les information du TS

static int ship_acc;
static int ship_dir;
static int ship_x;

/**
 * Fonctions privées
 */
static void ship_task(void *cookie);

int ship_task_start(){
	int err;

	err = rt_mutex_create(&ship_task_mutex, "task_ship_mutex");
	if(err == 0){
		ship_task_mutex_created = 1;
		printk("rt-app: Task SHIP create mutex succeed\n");
	}else{
		printk("rt-app: Task SHIP create mutex failed\n");
		goto fail;
	}
	err = rt_task_create(&ship_task_handle,
	                     "task_ship",
	                      TASK_STKSZ,
	                      TASK_SHIP_PRIO,
	                      0);
	if (err == 0){
		err = rt_task_start(&ship_task_handle, &ship_task, NULL);
		ship_task_created = 1;
		if(err != 0){
			printk("rt-app: Task SHIP starting failed\n");
			goto fail;
		}else{
			printk("rt-app: Task SHIP starting succeed\n");
		}
	}else{
		printk("rt-app: Task SHIP creation failed\n");
		goto fail;
	}
	return 0;
fail:
	ship_task_cleanup_task();
	ship_task_cleanup_objects();
	return -1;
}

void ship_task_cleanup_task(){
	// Si tache créée
	if(ship_task_created){
		// On la nettoie
		printk("rt-app: Task SHIP cleanup task\n");
		ship_task_created = 0;
		rt_task_delete(&ship_task_handle);
	}
}

void ship_task_cleanup_objects(){
	// Si mutex créé
	if(ship_task_mutex_created){
		//Mise à jour du flag de création et suppresion du mutex
		printk("rt-app: Task SHIP cleanup objects\n");
		ship_task_mutex_created = 0;
		rt_mutex_delete(&ship_task_mutex);
	}
}

void ship_task_init(){
	ship.hp = LIFE_SHIP;
	ship.hitbox.height=32;
	ship.hitbox.width=32;
	ship.hitbox.x= 104;
	ship.hitbox.y= 280;
	ship.hitbox.bitmap = (uint16_t *)bmp_ship;

	ship_acc = 0;
	ship_dir = 0;
	ship_x = 104;
}

static void ship_task(void *cookie){

	uint32_t cpt = 0;
	uint8_t rebond = 0;

	(void)cookie;

	// On définit la période de la tache
	rt_task_set_periodic(NULL, TM_NOW, 30*MS);
	ship_lock();
	//ship_init();
	ship_task_init();
	//memcpy(&ship_loc, &ship, sizeof(ship_loc));
	//x = ship_loc.hitbox.x;
	ship_x = ship.hitbox.x;
	ship_unlock();



	for (;;) {
		rt_task_wait_period(NULL);

		// Lecture de l'etat du touchscreen
		xeno_ts_read(&sample, 1, O_NONBLOCK);

		//ship_lock();
		// Tester une pression sur l'ecran
		if(sample.pressure > 0){
			if(cpt > 3){
				if(!rebond){
					rebond = 1;
					/*if(sample.y < 100){
						game_break = !game_break;
						if(game_break){

						}
					}*/
					if(!screen_pressed){
						screen_pressed = 1;
						screen_x = sample.x;
						screen_y = sample.y;
					}
				}
			}else{
				cpt++;
			}
			if(sample.y >= 200 && !game_break){
				//if(sample.x < (ship_loc.hitbox.x + ship_loc.hitbox.width/2 - 2)){
				if(sample.x < (ship.hitbox.x + ship.hitbox.width/2 - 2)){
					if(ship_dir == -1){
						ship_acc += 1;
					}else{
						ship_acc /= 3;
					}
					ship_dir = -1;
					ship_x -= ship_acc;
				//}else if(sample.x > (ship_loc.hitbox.x + ship_loc.hitbox.width/2 + 2)){
				}else if(sample.x > (ship.hitbox.x + ship.hitbox.width/2 + 2)){
					if(ship_dir == 1){
						ship_acc += 1;
					}else{
						ship_acc /= 3;
					}
					ship_dir = 1;
					ship_x += ship_acc;
				}
				if(ship_x < 0){
					ship_x = 0;
					ship_acc = 0;
					ship_dir = 0;
				//}else if(x > LCD_MAX_X-1 - ship_loc.hitbox.width){
				}else if(ship_x > LCD_MAX_X-1 - ship.hitbox.width){
					//x = LCD_MAX_X - 1 - ship_loc.hitbox.width;
					ship_x = LCD_MAX_X - 1 - ship.hitbox.width;
					ship_acc = 0;
					ship_dir = 0;
				}
				//ship_unlock();

				//ship_loc.hitbox.x = x;
				ship.hitbox.x = ship_x;
				//memcpy(&ship, &ship_loc, sizeof(ship_loc));
				ship_unlock();
			}
		}else{
			cpt = 0;
			rebond = 0;
		}
	}
}

int ship_lock(){
	if(ship_task_mutex_created){
		return rt_mutex_lock(&ship_task_mutex, TM_INFINITE);
	}
	return -1;
}

int ship_unlock(){
	if(ship_task_mutex_created){
		rt_mutex_unlock(&ship_task_mutex);
		return 0;
	}
	return -1;
}
