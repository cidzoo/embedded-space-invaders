/*
 * fichier ship_task.c
 * \brief fichier (corps) pour la gestion de la tache spaceship
 * \author Mohamed Regaya
 * \version 1.0
 * \date 12 janvier 2012
 * 
 * La tâche periodique space_ship est appelee toutes les 30 MS 
 * pour mettre a jour la position du vaisseau, sa vitesse et son acceleration
 *  
 */
#include <linux/types.h>
#include "ship_task.h"
#include "vga_lookup.h"
#include "xeno-ts.h"
#include "lcdlib.h"
#include "rt-app-m.h"


// le vaisseau a manipuler
spaceship_t ship;

/**
 * Variables privees: thread et mutex et variables (flags) 
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
 * contient le corps proprement dit de la tache vaisseau.
 * c'est a ce niveau qu'on effectue la mise a jour de l'etat 
 * du vaisseau (position, vitesse, acceleration)
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

/**
 *
 */
void ship_task_cleanup_task(){
	// Si tache cree
	if(ship_task_created){
		// On la nettoie
		printk("rt-app: Task SHIP cleanup task\n");
		ship_task_created = 0;
		rt_task_delete(&ship_task_handle);
	}
}

/**
 *
 */
void ship_task_cleanup_objects(){
	// Si mutex cree
	if(ship_task_mutex_created){
		//Mise a  jour du flag de creation et suppresion du mutex
		printk("rt-app: Task SHIP cleanup objects\n");
		ship_task_mutex_created = 0;
		rt_mutex_delete(&ship_task_mutex);
	}
}

/**
 *
 */
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

/**
 *
 */

static void ship_task(void *cookie){

	uint32_t cpt = 0;
	uint8_t rebond = 0;

	(void)cookie;

	// On definit la periode de la tache
	rt_task_set_periodic(NULL, TM_NOW, 30*MS);
	ship_lock();
	
	ship_task_init();

	ship_x = ship.hitbox.x;
	ship_unlock();

	for (;;) {
		rt_task_wait_period(NULL);

		// Lecture de l'etat du touchscreen
		xeno_ts_read(&sample, 1, O_NONBLOCK);
		
		// Tester une pression sur l'ecran
		if(sample.pressure > 0){
			if(cpt > 3){
				if(!rebond){
					rebond = 1;
					
					if(!screen_pressed){
						screen_pressed = 1;
						screen_x = sample.x;
						screen_y = sample.y;
					}
				}
			}else{
				cpt++;
			}
			// la pression se trouve dans la zone du vaisseau
			if(sample.y >= 200 && !game_break){
                
                // pression detecte a gauche du ship
				if(sample.x < (ship.hitbox.x + ship.hitbox.width/2 - 2)){
					if(ship_dir == -1){
						ship_acc += 1;
					}else{
						ship_acc /= 3;
					}
					ship_dir = -1;
					ship_x -= ship_acc;
                // pression detecte a droite du ship
				}else if(sample.x > (ship.hitbox.x + ship.hitbox.width/2 + 2)){
					if(ship_dir == 1){
						ship_acc += 1;
					}else{
						ship_acc /= 3;
					}
					ship_dir = 1;
					ship_x += ship_acc;
				}
                // correction lorsque ship atteint les bords 
				if(ship_x < 0){
					ship_x = 0;
					ship_acc = 0;
					ship_dir = 0;

				}else if(ship_x > LCD_MAX_X-1 - ship.hitbox.width){
					
					ship_x = LCD_MAX_X - 1 - ship.hitbox.width;
					ship_acc = 0;
					ship_dir = 0;
				}

				ship.hitbox.x = ship_x;
				ship_unlock();
			}
		}else{
			cpt = 0;
			rebond = 0;
		}
	}
}

/**
 * verouiller le mutex
 */
int ship_lock(){
	if(ship_task_mutex_created){
		return rt_mutex_lock(&ship_task_mutex, TM_INFINITE);
	}
	return -1;
}

/**
 * severouiller le mutex
 */
int ship_unlock(){
	if(ship_task_mutex_created){
		rt_mutex_unlock(&ship_task_mutex);
		return 0;
	}
	return -1;
}
