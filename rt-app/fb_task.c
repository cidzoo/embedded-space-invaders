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
	// Variable locale pour les invaders
	invader_t invaders_loc[NB_INVADERS];
	// Variable locale pour les bullets
	bullet_t bullets_loc[NB_MAX_BULLETS];
	spaceship_t ship_loc;

	(void)cookie;

	// On d�finit la p�riode de la tache
	rt_task_set_periodic(NULL, TM_NOW, 50*MS);

	for (;;) {
		rt_task_wait_period(NULL);

		// On copie les invaders en local
		invaders_lock();
		memcpy(invaders_loc, invaders, sizeof(invaders_loc));
		invaders_unlock();

		// On copie le vaisseau en local
		ship_lock();
		memcpy(&ship_loc, &ship, sizeof(ship_loc));
		ship_unlock();

		// On copie les bullets en local
		hit_lock();
		memcpy(bullets_loc, bullets, sizeof(bullets_loc));
		hit_unlock();

		// On dessine le background
		fb_rect_fill(10, 319, 0, 239, LU_BLACK);

		// On dessine les bullets
		for(i = 0; i < NB_MAX_BULLETS; i++){
			if (bullets_loc[i].weapon != NULL){
				/*if( (bullets[i].weapon->weapon_type != RAIL) &&
					(bullets[i].hitbox.y-1 == 0) ){
					remove_bullet(i);
				}
				bullets[i].hitbox.y -= bullets[i].weapon->speed;*/

				draw_bitmap(bullets_loc[i].hitbox);
			}
		}

		// On dessine les invaders
		for(i = 0; i < NB_INVADERS; i++){
			if(invaders_loc[i].hp > 0){
				draw_bitmap(invaders_loc[i].hitbox);
			}
		}

		// On dessine le vaisseau
		draw_bitmap(ship_loc.hitbox);

		// On dessine le header
		fb_rect_fill(0, 20, 0, 239, LU_GREY);

		// On print le texte pour la progress bar
		fb_print_string(LU_BLACK, LU_GREY, "life", 3, 3);
		// On print la progress bar
		fb_progress_bar(3, 10, 40, 200, LU_RED, ship_loc.hp, LIFE_SHIP);

		rt_task_set_priority(NULL, 90);
		fb_display();
		rt_task_set_priority(NULL, 50);
	}
}

static void draw_bitmap(hitbox_t hb){
	int i, j;
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
			break;
		case G_BOMB:
			for(i = 0; i < hb.height; i++){
				bmp[i] = bmp_bomb[i];
			}
			break;
		case G_GUN:
			for(i = 0; i < hb.height; i++){
				bmp[i] = bmp_gun[i];
			}
			break;
		case G_RAIL:
			for(i = 0; i < hb.height; i++){
				bmp[i] = bmp_rail[i];
			}
			break;
		case G_ROCKET:
			for(i = 0; i < hb.height; i++){
				bmp[i] = bmp_rocket[i];
			}
			break;
		case G_WAVE:
			for(i = 0; i < hb.height; i++){
				bmp[i] = bmp_wave[i];
			}
			break;
	}

	for(i = 0; i < hb.height; i++){
		for(j = 0; j < hb.width; j++){
			if((*((*(bmp+i))+j)) != LU_BLACK){
				fb_set_pixel(hb.y + hb.height-i, hb.x + j, (*((*(bmp+i))+j)));
			}
		}
	}
}

