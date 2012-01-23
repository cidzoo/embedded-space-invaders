/*
 * task_fb.c
 *
 *  Created on: 17 janv. 2012
 *      Author: redsuser
 */
#include "fb_task.h"
#include "vga_lookup.h"
#include "lcdlib.h"

#include "invaders_task.h"
#include "ship_task.h"
#include "hit_task.h"
#include "rt-app-m.h"
#include "vga_lookup.h"
#include <linux/slab.h>
/**
 * Variables privées
 */
RT_TASK fb_task_handle;
static uint8_t fb_task_created = 0;


/**
 * Fonctions privées
 */
static void draw_bitmap(hitbox_t hb);
static void fb_task(void *cookie);

int fb_task_start(){
	int err;
	err = rt_task_create(&fb_task_handle,
	                     "task_fb",
	                      TASK_STKSZ,
	                      TASK_FB_PRIO,
	                      0);
	if (err == 0){
		err = rt_task_start(&fb_task_handle, &fb_task, NULL);
		fb_task_created = 1;
		if(err != 0){
			printk("rt-app: Task FB starting failed\n");
			goto fail;
		}else{
			printk("rt-app: Task FB starting succeed\n");
		}
	}else{
		printk("rt-app: Task FB creation failed\n");
		goto fail;
	}
	return 0;
fail:
	fb_task_cleanup_task();
	fb_task_cleanup_objects();
	return -1;
}

void fb_task_cleanup_task(){
	if(fb_task_created){
		printk("rt-app: Task FB cleanup task\n");
		fb_task_created = 0;
		rt_task_delete(&fb_task_handle);
	}
}

void fb_task_cleanup_objects(){
}

static void fb_task(void *cookie){

	int  i;
	invader_t invader_loc[NB_INVADERS];
	spaceship_t ship_loc;

	(void)cookie;


	rt_task_set_periodic(NULL, TM_NOW, 50*MS);

	for (;;) {
		rt_task_wait_period(NULL);

		invaders_lock();
		memcpy(invader_loc, invaders, sizeof(invader_loc));
		invaders_unlock();

		ship_lock();
		memcpy(&ship_loc, &ship, sizeof(ship_loc));
		ship_unlock();

		// On dessine le header
		fb_rect_fill(0, 10, 0, 239, LU_GREY);
		// On dessine le background
		fb_rect_fill(10, 319, 0, 239, LU_BLACK);

		// On print le texte pour la progress bar
		fb_print_string(LU_BLACK, LU_GREY, "life", 3, 3);
		// On print la progress bar
		fb_progress_bar(3, 7, 40, 100, LU_RED, ship_loc.hp, LIFE_SHIP);

		// On dessine les bullets
		hit_lock();
		for(i = 0; i < NB_MAX_BULLETS; i++){
			if (bullets[i].weapon != NULL){
				if( (bullets[i].weapon->weapon_type != RAIL) &&
					(bullets[i].hitbox.y-1 == 0) ){
					remove_bullet(i);
				}
				bullets[i].hitbox.y -= bullets[i].weapon->speed;

				draw_bitmap(bullets[i].hitbox);
			}
		}
		hit_unlock();

		// On dessine les invaders
		for(i = 0; i < NB_INVADERS; i++){
			if(invader_loc[i].hp > 0){
				draw_bitmap(invader_loc[i].hitbox);
			}
		}

		// On dessine le vaisseau
		draw_bitmap(ship_loc.hitbox);

		rt_task_set_priority(NULL, 90);
		fb_display();
		rt_task_set_priority(NULL, 50);
	}
}

static void draw_bitmap(hitbox_t hb){
	int i, j;
//fb_set_pixel(hb.y + hb.height-i, hb.x + j, (*((*(bmp_wave+i))+j)));
	uint16_t *bmp[hb.height];


	switch(hb.type){
		case G_SHIP:
			for(i = 0; i < hb.height; i++){
				bmp[i] = bmp_ship[i];
			}
			break;
		case G_INVADER:
			for(i = 0; i < hb.height; i++){
				bmp[i] = bmp_invader[i];
			}
			//bmp = bmp_invader;
			break;
		case G_BOMB:
			for(i = 0; i < hb.height; i++){
				bmp[i] = bmp_bomb[i];
			}
			//bmp = bmp_bomb;
			break;
		case G_GUN:
			for(i = 0; i < hb.height; i++){
				bmp[i] = bmp_gun[i];
			}
			//bmp = bmp_gun;
			break;
		case G_RAIL:
			for(i = 0; i < hb.height; i++){
				bmp[i] = bmp_rail[i];
			}
			//bmp = bmp_rail;
			break;
		case G_ROCKET:
			for(i = 0; i < hb.height; i++){
				bmp[i] = bmp_rocket[i];
			}
			//bmp = bmp_rocket;
			break;
		case G_WAVE:
			for(i = 0; i < hb.height; i++){
				bmp[i] = bmp_wave[i];
			}
			//bmp = bmp_wave;
			break;
	}


	//printk("%d %d %d %d\n", hb.x, hb.y, hb.width, hb.height);
	for(i = 0; i < hb.height; i++){
		for(j = 0; j < hb.width; j++){
			if((*((*(bmp+i))+j)) != LU_BLACK){
				//printk("%p %p\n", bmp, bmp_ship);
				//printk("%d\n", *((*(bmp+i))+j));
				fb_set_pixel(hb.y + hb.height-i, hb.x + j, (*((*(bmp+i))+j)));
			}
			//fb_set_pixel(hb.y + hb.height-i, hb.x + j, bmp[i][j]);
		}
	}
}

