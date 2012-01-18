/*
 * hit_task.c
 *
 *  Created on: 17 janv. 2012
 *      Author: redsuser
 */
#include "hit_task.h"
#include "invaders_task.h"

/**
 * Variables privées
 */
<<<<<<< HEAD
RT_TASK hit_task_handle;
=======
static RT_MUTEX invaders_task_mutex;
static uint8_t invaders_task_mutex_created = 0;

static RT_TASK hit_task_handle;
>>>>>>> 9867b11d598b99fc838c86310d11b5cd8e29a4f0
static uint8_t hit_task_created = 0;

/**
 * Fonctions privées
 */
static void hit_task(void *cookie);
static uint8_t hit_test(hitbox_t a, hitbox_t b);

int hit_task_start(){
	int err;

	err = rt_mutex_create(&invaders_task_mutex, "task_invader_mutex");
	if(err == 0){
		invaders_task_mutex_created = 1;
		printk("rt-app: Task INVADERS create mutex succeed\n");
	}else{
		printk("rt-app: Task INVADERS create mutex failed\n");
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
	if(invaders_task_mutex_created){
		invaders_task_mutex_created = 0;
		rt_mutex_delete(&invaders_task_mutex);
	}
}

void hit_task_cleanup_objects(){
}

void hit_task(void *cookie){

	invader_t *invader;
	bullet_t *bullet;
	int i, j, k;

	rt_task_set_periodic(NULL, TM_NOW, 10000000);

	for (;;) {
		rt_task_wait_period(NULL);

		//for each bullet
		for (i=0;i<NB_MAX_BULLETS;i++){
			//for each invader
			for (j=0;j<NB_INVADERS;j++){
				//current traited objects
				invader = &invaders[j];
				bullet = bullets[i];

				//control if the bullet it touching the invader
				if(invader != NULL && bullet != NULL){
					if(hit_test(invader->hitbox, bullet->hitbox) ){
						//if so : damage the invader
						for (k=0;k < bullet->weapon->damage;k++){
							invader->hp--;
							if(invader->hp == 0)
								break;
						}
						//for a rocket create the explosion as a new bullet
						if(bullet->weapon->weapon_type == ROCKET){
							bullet_t *new_bullet = NULL;
							new_bullet->weapon = &weapons[GUN];
							new_bullet->hitbox.x = bullet->hitbox.x;
							new_bullet->hitbox.y = bullet->hitbox.y;
							new_bullet->hitbox.width = 20;
							new_bullet->hitbox.height = 20;
							add_bullet(new_bullet);
						}
						remove_bullet(bullet);
					}//if positive hit test*/
				}
			}//for each invaders
			//control that the spaceship is not been touched by a invader's bomb
			if(ss != NULL && bullet != NULL){
				if( hit_test(ss->hitbox, bullet->hitbox) ){
					ss->hp--;
					remove_bullet(bullet);
				}
			}

		}//for each bullet
	}
	level_up();
}

static uint8_t hit_test(hitbox_t a, hitbox_t b){

	if (	(a.x + a.width >= b.x) &&
			(a.x <= b.x + b.width) &&
			(a.y <= b.y + b.height) &&
			(a.y + a.height >= b.y) ) {
		return 0;
	}
	return -1;

}//hit_test()

void hit_refresh(void){

}
