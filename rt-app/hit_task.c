/*
 * hit_task.c
 *
 *  Created on: 17 janv. 2012
 *      Author: redsuser
 */
#include "hit_task.h"
#include "invaders_task.h"
#include "ship_task.h"

#include "lcdlib.h"
#include "vga_lookup.h"


//List of the bullets
bullet_t bullets[NB_MAX_BULLETS];

//List of the bombs
bullet_t bombs[NB_MAX_BOMBS];

/**
 * Variables privées
 */
RT_MUTEX hit_task_mutex;
static uint8_t hit_task_mutex_created = 0;

RT_TASK hit_task_handle;
static uint8_t hit_task_created = 0;

/**
 * Fonctions privées
 */
static void hit_task(void *cookie);
static int hit_test(hitbox_t a, hitbox_t b);

int hit_task_start(){
	int err;

	err = rt_mutex_create(&hit_task_mutex, "task_hit_mutex");
	if(err == 0){
		hit_task_mutex_created = 1;
		printk("rt-app: Task HIT create mutex succeed\n");
	}else{
		printk("rt-app: Task HIT create mutex failed\n");
		goto fail;
	}

	err = rt_task_create(&hit_task_handle,
	                     "task_hit",
	                      TASK_STKSZ,
	                      TASK_HIT_PRIO,
	                      0);
	if (err == 0){
		err = rt_task_start(&hit_task_handle, &hit_task, NULL);
		hit_task_created = 1;
		if(err != 0){
			printk("rt-app: Task HIT starting failed\n");
			goto fail;
		}else{
			printk("rt-app: Task HIT starting succeed\n");
		}
	}else{
		printk("rt-app: Task HIT creation failed\n");
		goto fail;
	}
	return 0;
fail:
	hit_task_cleanup_task();
	hit_task_cleanup_objects();
	return -1;
}

void hit_task_cleanup_task(){
	if(hit_task_created){
		hit_task_created = 0;
		rt_task_delete(&hit_task_handle);
	}
}

void hit_task_cleanup_objects(){
	if(hit_task_mutex_created){
		hit_task_mutex_created = 0;
		rt_mutex_delete(&hit_task_mutex);
	}
}

void hit_task(void *cookie){

	invader_t *invader;
	bullet_t *bullet;
	int i, j, k, tempo =0;

	spaceship_t ship_loc;

	rt_task_set_periodic(NULL, TM_NOW, 10000000);

	ship_lock();
	memcpy(&ship_loc, &ship, sizeof(ship_loc));
	ship_unlock();

	ship_loc.hitbox.height = 20;
	ship_loc.hitbox.width = 20;
	ship_loc.hitbox.x = 100;
	ship_loc.hitbox.y = 290;
	ship_loc.hp = 3;


	for (;;) {
		rt_task_wait_period(NULL);
		if(tempo == 100){
			fire_weapon(GUN);
			tempo = 0;
		}
		tempo++;

		//for each bullet
		for (i=0;i<NB_MAX_BULLETS;i++){
			//for each invader
			for (j=0;j<NB_INVADERS;j++){
				//current traited objects
				invader = &invaders[j];
				bullet = &bullets[i];

				//control if the bullet it touching the invader
				if(invader->hp>0 && bullet->weapon != NULL){

					//TODO : test normalement : == 0, controler la fct hit_test car
					//renvoie toujours 0
					if(hit_test(invader->hitbox, bullet->hitbox) == 0){

						//if so : damage the invader
						for (k=0;k < bullet->weapon->damage;k++){
							invader->hp--;
							if(invader->hp == 0)
								break;
						}
//						//for a rocket create the explosion as a new bullet
//						if(bullet->weapon->weapon_type == ROCKET){
//							bullet_t new_bullet;
//							new_bullet.weapon = &weapons[GUN];
//							new_bullet.hitbox.x = bullet->hitbox.x;
//							new_bullet.hitbox.y = bullet->hitbox.y;
//							new_bullet.hitbox.width = 20;
//							new_bullet.hitbox.height = 20;
//							add_bullet(new_bullet);
//						}
						remove_bullet(i);
					}//if positive hit test*/
				}
			}//for each invaders
			//control that the spaceship is not been touched by a invader's bomb
//			if(bullet->weapon != NULL){
//				if( hit_test(ship.hitbox, bullet->hitbox) ){
//					ship.hp--;
//					remove_bullet(*bullet);
//				}
//			}

		}//for each bullet
	}
	//level_up();
}

void fire_weapon(weapontype_t w){
	uint16_t start_x=0, start_y=0;
	bullet_t b;

	//Control that the selected weapon CAN be used
	if(weapons[w].cooldown == 0){

		switch(w){
		case BOMB:
			//TODO get global invaders hitbox
			break;
		default:
			//Grab the position of the spaceship's gun's middle top
			start_x = ship.hitbox.x + ship.hitbox.width/2;
			start_y = ship.hitbox.y-1;
			break;
		}


		//Fire the weapon
		switch(w){
		case BOMB:
			b.weapon = &weapons[BOMB];
			b.hitbox.x = start_x-2/2; // width/2
			b.hitbox.y = start_y-4; //-height
			b.hitbox.width = 2;
			b.hitbox.height = 4;
			break;
		case GUN:
			b.weapon = &weapons[GUN];
			b.hitbox.x = start_x-2/2; // width/2
			b.hitbox.y = start_y-4; //-height
			b.hitbox.width = 2;
			b.hitbox.height = 4;
			break;
		case RAIL:
			b.weapon = &weapons[RAIL];
			b.hitbox.x = start_x-2/2; // width/2
			b.hitbox.y = 0;
			b.hitbox.width = 2;
			b.hitbox.height = LCD_MAX_Y-start_y;
			break;
		case ROCKET:
			b.weapon = &weapons[ROCKET];
			b.hitbox.x = start_x-4/2; // width/2
			b.hitbox.y = start_y-8; //-height
			b.hitbox.width = 4;
			b.hitbox.height = 8;
			break;
		case WAVE:
			b.weapon = &weapons[WAVE];
			b.hitbox.x = 0; // width/2
			b.hitbox.y = start_y-2;
			b.hitbox.width = LCD_MAX_X;
			b.hitbox.height = 2;
			break;
		}

		//add it to the list of current bullets
		add_bullet(b);

	}//end ready

}//fire_weapon()

static int hit_test(hitbox_t a, hitbox_t b){

	if (	(a.x + a.width >= b.x) &&
			(a.x <= b.x + b.width) &&
			(a.y <= b.y + b.height) &&
			(a.y + a.height >= b.y) ) {
		return 0;
	}
	return -1;

}//hit_test()
