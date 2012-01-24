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
#include "rt-app-m.h"

//Array of all weapons
weapon_t weapons[NB_WEAPONS] = {
	{BOMB, ONE, MEDIUM,
		{0, 0, 0, 0, 0},
		{0, 0, 0},
	},
	{GUN, ONE, MEDIUM,
		{15, 15, 0, 4, 0},
		{10, 0, 0},
	},
	{RAIL, TWO, STATIC,
		{1, 0, 0, 50, 0},
		{10, 0, 0},
	},
	{ROCKET, THREE, SLOW,
		{1, 0, 0, 100, 0},
		{10, 0, 0},
	},
	{WAVE, MAX, FAST,
		{1, 0, 0, 250, 0},
		{10, 0, 0},
	}
};

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
static int hit_test(hitbox_t a, hitbox_t b);
static void hit_task(void *cookie);

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
	int i, j, k;
	int16_t y;

	(void)cookie;
	// On définit la période de la tache
	rt_task_set_periodic(NULL, TM_NOW, 50*MS);


	for (;;) {
		rt_task_wait_period(NULL);

		// On verrouille les bullets
		hit_lock();
		// On verrouille le vaisseau
		ship_lock();
		// On verrouille les invaders
		invaders_lock();
		//for each bullet.
		for (i=0;i<NB_MAX_BULLETS;i++){
			if(bullets[i].weapon != NULL){
				bullet = &bullets[i];

				// On déplace le bullets
				//bullet->hitbox.y -= bullet->weapon->speed;
				bullet->hitbox.y -= bullet->weapon->speed;
				/*y = bullet->hitbox.y-1;
				if( (bullet->weapon->weapon_type != RAIL) &&
					(y <= 0) ){
					remove_bullet(i);
				}*/

				//for each invader
				for (j=0;j<NB_INVADERS;j++){
					//current traited objects
					invader = &invaders[j];

					//test if applicable
					if(invader->hp > 0){
						//control if the bullet it touching the invader
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
							if(!(bullet->weapon->weapon_type == WAVE))
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
			}

		}//for each bullet
		// On deverrouille les invaders
		invaders_unlock();
		// On deverrouille le vaisseau
		ship_unlock();
		// On deverrouille les bullets
		hit_unlock();
	}
	//level_up();
}

void fire_weapon(weapontype_t w){
	uint16_t start_x=0, start_y=0;
	bullet_t b;

	//Control that the selected weapon CAN be used


//		if(w == BOMB){
//			//TODO:Grab the position of the spaceship's gun's
//		}
//		else{
			//Grab the position of the spaceship's gun's
			start_x = ship.hitbox.x + ship.hitbox.width/2;
			start_y = ship.hitbox.y-1;
//		}

	//Fire the weapon
	switch(w){
	case BOMB:
		b.weapon = &weapons[BOMB];
		b.hitbox.x = start_x-BOMB_WIDTH/2;
		b.hitbox.y = start_y-BOMB_HEIGHT;
		b.hitbox.width = BOMB_WIDTH;
		b.hitbox.height = BOMB_HEIGHT;
		b.hitbox.type = G_BOMB;
		break;
	case GUN:
		b.weapon = &weapons[GUN];
		b.hitbox.x = start_x-GUN_WIDTH/2;
		b.hitbox.y = start_y-GUN_HEIGHT;
		b.hitbox.width = GUN_WIDTH;
		b.hitbox.height = GUN_HEIGHT;
		b.hitbox.type = G_GUN;
		break;
	case RAIL:
		b.weapon = &weapons[RAIL];
		b.hitbox.x = start_x-RAIL_WIDTH/2;
		b.hitbox.y = GAME_ZONE_Y_MIN;
		b.hitbox.width = RAIL_WIDTH;
		b.hitbox.height = start_y - GAME_ZONE_Y_MIN;
		b.hitbox.type = G_RAIL;
		break;
	case ROCKET:
		b.weapon = &weapons[ROCKET];
		b.hitbox.x = start_x-ROCKET_WIDTH/2;
		b.hitbox.y = start_y-ROCKET_HEIGHT;
		b.hitbox.width = ROCKET_WIDTH;
		b.hitbox.height = ROCKET_HEIGHT;
		b.hitbox.type = G_ROCKET;
		break;
	case WAVE:
		b.weapon = &weapons[WAVE];
		b.hitbox.x = 0;
		b.hitbox.y = start_y-WAVE_HEIGHT;
		b.hitbox.width = WAVE_WIDTH;
		b.hitbox.height = WAVE_HEIGHT;
		b.hitbox.type = G_WAVE;
		break;
	}

	//add it to the list of current bullets
	add_bullet(b);

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

int hit_lock(){
	if(hit_task_mutex_created){
		return rt_mutex_lock(&hit_task_mutex, TM_INFINITE);
	}
	return -1;
}

int hit_unlock(){
	if(hit_task_mutex_created){
		return rt_mutex_unlock(&hit_task_mutex);
	}
	return -1;
}
